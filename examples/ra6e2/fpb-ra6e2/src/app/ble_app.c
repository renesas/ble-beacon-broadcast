#include "ble_thread.h"            /* g_ble_abs0_ctrl / g_ble_abs0_cfg, FreeRTOS, FSP */
#include "r_ble_api.h"             /* R_BLE_Execute, GAP events                        */
#include "event_groups.h"          /* xEventGroupCreate                               */
#include "beacon_set.h"            /* the app: payload setup + (dual) runtime alternation */
#include "ble_app.h"               /* ble_app_run() — the shared, board-agnostic body  */
#include "msg_print.h"             /* MSG_PRINT                                        */
#include <stdbool.h>

/* Defined in ble_app_glue.c; the GTL driver uses it, so create it before open. */
extern EventGroupHandle_t g_ble_event_group_handle;

/* Bits the GTL driver raises on g_ble_event_group_handle when it has queued RX
 * work for R_BLE_Execute(). Must match BLE_EVENT_PATTERN in r_ble_gtl.c — the
 * driver does not export it, so the application defines it (as the QE-generated
 * app_main does). The BLE task blocks on this instead of polling. */
#define BLE_EVENT_PATTERN   0x0A0AU

/* Set true once the BLE stack has finished coming up. The FSP only reports stack
 * status through the GAP callback, so this one flag is unavoidable. */
static volatile bool g_ble_stack_ready = false;

/* GAP callback (name + signature per ra_gen/ble_thread.h). It does ONE thing:
 * record that the stack is up. Advertising is started once GAP-ready, below. */
void gap_cb(uint16_t type, ble_status_t result, st_ble_evt_data_t * p_data)
{
    FSP_PARAMETER_NOT_USED(result);
    FSP_PARAMETER_NOT_USED(p_data);
    if (BLE_GAP_EVENT_STACK_ON == type) {
        g_ble_stack_ready = true;
    }
}

/* Vendor-specific callback (unused for a beacon). */
void vs_cb(uint16_t type, ble_status_t result, st_ble_vs_evt_data_t * p_data)
{
    FSP_PARAMETER_NOT_USED(type);
    FSP_PARAMETER_NOT_USED(result);
    FSP_PARAMETER_NOT_USED(p_data);
}

/* Brings the BLE stack up: GTL open (DA14531 boot), STACK_ON wait. Needs the
 * BLE event group to exist already.
 * Halts at __BKPT on any failure, so it only returns with the stack ready. */
static void ble_init(void)
{
    /* 1. Open the GTL stack. Blocks until the DA14531 is reset, booted over
     *    UART, ready and configured; STACK_ON is only queued, and is
     *    delivered to gap_cb by R_BLE_Execute() in step 2. */
    fsp_err_t err = RM_BLE_ABS_Open(&g_ble_abs0_ctrl, &g_ble_abs0_cfg);
    if (FSP_SUCCESS != err) {
        MSG_PRINT("ERR: ble_init: RM_BLE_ABS_Open failed, err=0x%X\n", (unsigned)err);
        __BKPT(0);   /* DA14531 UART boot or device-ready failed */
    }

    /* 2. Wait until the stack is actually up. R_BLE_Execute() dispatches queued
     *    BLE events (incl. STACK_ON) to the callbacks; then block on the BLE
     *    event group until the GTL driver signals more RX work (no polling).
     *    The driver sets the bit before we wait, so no wakeup is ever missed. */
    while (!g_ble_stack_ready) {
        R_BLE_Execute();
        (void)xEventGroupWaitBits(g_ble_event_group_handle, BLE_EVENT_PATTERN,
                                  pdTRUE, pdFALSE, portMAX_DELAY);
    }
}

/* Board-agnostic BLE application body. Every board's FSP-owned thread entry
 * (src/ble_thread_entry.c) is a one-line shim that calls this, so all of the
 * task logic below — stack bring-up (ble_init), payload build, advertising
 * start, and (dual builds) the format-switch service loop — is shared across
 * boards. */
void ble_app_run(void * pvParameters)
{
    FSP_PARAMETER_NOT_USED(pvParameters);

    /* 0. Create the BLE event group the GTL driver uses. Must exist before the stack opens. */
    g_ble_event_group_handle = xEventGroupCreate();
    if (NULL == g_ble_event_group_handle) {
        MSG_PRINT("ERR: ble_app_run: xEventGroupCreate failed\n");
        __BKPT(0);   /* FreeRTOS heap exhausted: raise configTOTAL_HEAP_SIZE */
    }

    /* 1. BLE INIT: stack open, wait for STACK_ON. */
    ble_init();

    /* 2. Stack ready -> build the enabled payload(s) (iBeacon and/or Eddystone-
     *    URL, app-owned structs), start advertising, and — in dual builds —
     *    start the alternation timer. All format knowledge and the timer live
     *    in the app (beacon_set.c) — the RS is only ever handed opaque bytes. */
    ble_status_t init_status = beacon_set_init();
    if (BLE_SUCCESS != init_status) {
        MSG_PRINT("ERR: ble_app_run: beacon_set_init failed, status=0x%04X\n", (unsigned)init_status);
        __BKPT(0);   /* advertising or alternation timer could not start */
    }
    MSG_PRINT("INF: ble_app_run: BLE stack up, advertising started\n");

    /* 3. Event-driven service loop (no polling). Drain ALL queued GTL events
     *    (Execute() frees one callback event per call and the wake bit is not a
     *    count, so a single call per wake lets events pile up), then block until
     *    EITHER the driver signals RX work (BLE_EVENT_PATTERN) OR (dual builds)
     *    the alternation timer signals a due switch (BEACON_SET_EVT) — both ride
     *    the same event group, so one wait covers both. The switch (and hence
     *    rs_ble_broadcast_Stop()/rs_ble_broadcast_Start()) runs here, in the
     *    BLE task, so GTL/UART traffic stays serialised with R_BLE_Execute(). */
    while (1) {
        do {
            (void)R_BLE_Execute();
        } while (0U == R_BLE_IsTaskFree());

        EventBits_t bits = xEventGroupWaitBits(
            g_ble_event_group_handle,
            BLE_EVENT_PATTERN            /* GTL RX work */
#if BEACON_DUAL
            | BEACON_SET_EVT             /* format-switch due (dual only) */
#endif
            ,
            pdTRUE,                      /* clear on exit  */
            pdFALSE,                     /* wake on ANY bit */
            portMAX_DELAY);
#if BEACON_DUAL
        if ((bits & BEACON_SET_EVT) != 0U) {
            beacon_set_switch();
        }
#else
        (void)bits;                      /* single format: only ever woken for GTL work */
#endif
    }
}
