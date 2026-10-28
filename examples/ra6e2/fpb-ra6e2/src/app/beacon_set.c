/**
 * beacon_set.c  —  Beacon setup + (optional) runtime alternation (app)
 *
 * See beacon_set.h / beacon_config.h for the selection and threading contract.
 * All beacon-format knowledge and scheduling live here in the app; the RS is
 * only ever handed opaque bytes.
 *
 * SINGLE format (only one of BEACON_ENABLE_*): beacon_set_init() builds that one
 * payload and advertises it. No timer, no switch path is compiled.
 *
 * BOTH formats (BEACON_DUAL): both payloads are resident at distinct static
 * addresses; a FreeRTOS software timer (auto-reload, period = adv_interval/2)
 * raises BEACON_SET_EVT on g_ble_event_group_handle each half-interval, and the
 * BLE task calls beacon_set_switch() to stop()+start() the other one — so each
 * format gets ~one turn on air per advertising interval. The timer runs in the
 * timer-service task and only sets the bit; the RS is touched only from the BLE
 * task, keeping GTL/UART serialised with R_BLE_Execute().
 */

#include "beacon_set.h"          /* includes beacon_config.h */
#include "rs_ble_broadcast.h"   /* the RS: rs_ble_broadcast_Start/rs_ble_broadcast_Stop */
#include "msg_print.h"          /* MSG_PRINT */
#if BEACON_ENABLE_IBEACON
#include "beacon_ibeacon.h"      /* ibeacon_t + IBEACON_DEFAULT (app-owned)  */
#endif
#if BEACON_ENABLE_EDDYSTONE
#include "beacon_eddystone.h"    /* eddystone_url_t + EDDYSTONE_URL_DEFAULT  */
#endif
#if BEACON_DUAL
#include "FreeRTOS.h"            /* required before timers.h / event_groups.h        */
#include "timers.h"
#include "event_groups.h"
/* Created in ble_app.c (ble_app_run) and signalled by the GTL driver for RX work. */
extern EventGroupHandle_t g_ble_event_group_handle;
#endif

/* Advertising parameters shared by every format (moderate/MID Tx power). */
static const rs_ble_broadcast_adv_params_t g_params = {
    .adv_interval = BEACON_ADV_INTERVAL_UNITS,   /* ~1 s (beacon_config.h)      */
    .tx_power     = BLE_VS_TX_POWER_MID,
    .channel_map  = (uint8_t)BLE_GAP_ADV_CH_ALL,
};

/* Resident payload(s) — only the enabled ones exist. */
#if BEACON_ENABLE_IBEACON
static ibeacon_t       g_ib;
#endif
#if BEACON_ENABLE_EDDYSTONE
static eddystone_url_t g_ed;
#endif

#if BEACON_DUAL
/* One agnostic descriptor per format: opaque bytes + length. */
typedef struct { const uint8_t *bytes; uint8_t len; } beacon_slot_t;
static beacon_slot_t      g_slots[2];
static uint8_t            g_active_slot;   /* 0 = iBeacon, 1 = Eddystone-URL */
static TimerHandle_t      g_set_timer;

/* Timer-service-task context ONLY: signal the BLE task, never call the RS. */
static void beacon_set_timer_cb(TimerHandle_t timer)
{
    (void)timer;
    (void)xEventGroupSetBits(g_ble_event_group_handle, BEACON_SET_EVT);
}

/* adv_interval units are 0.625 ms; convert to whole milliseconds. */
static uint32_t adv_interval_to_ms(uint16_t adv_interval_units)
{
    return ((uint32_t)adv_interval_units * 625u) / 1000u;
}
#endif /* BEACON_DUAL */

ble_status_t beacon_set_init(void)
{
#if BEACON_ENABLE_IBEACON
    { ibeacon_t       ib = IBEACON_DEFAULT;       g_ib = ib; }
#endif
#if BEACON_ENABLE_EDDYSTONE
    { eddystone_url_t ed = EDDYSTONE_URL_DEFAULT; g_ed = ed; }
#endif

#if BEACON_DUAL
    /* Both formats: set up the two slots and alternate on a timer. */
    g_slots[0].bytes = (const uint8_t *)&g_ib;
    g_slots[0].len   = IBEACON_ADV_LEN;
    g_slots[1].bytes = (const uint8_t *)&g_ed;
    g_slots[1].len   = EDDYSTONE_URL_ADV_LEN(EDDYSTONE_CFG_URL_LEN);
    g_active_slot    = 0U;   /* start on iBeacon */

    /* Switch at adv_interval/2 → each format ~one turn per advertising interval
     * (switch latency assumed negligible vs the ~1 s interval). */
    uint32_t period_ms = adv_interval_to_ms(BEACON_ADV_INTERVAL_UNITS) / 2U;
    g_set_timer = xTimerCreate("beacon_set",
                                pdMS_TO_TICKS(period_ms),
                                pdTRUE,                     /* auto-reload */
                                NULL,
                                beacon_set_timer_cb);
    if (NULL == g_set_timer) {
        MSG_PRINT("ERR: beacon_set_init: xTimerCreate failed\n");
        return BLE_ERR_MEM_ALLOC_FAILED;   /* FreeRTOS heap exhausted */
    }
    if (pdPASS != xTimerStart(g_set_timer, 0)) {
        MSG_PRINT("ERR: beacon_set_init: xTimerStart failed\n");
        return BLE_ERR_CONTEXT_FULL;       /* timer command queue full */
    }

    return rs_ble_broadcast_Start(g_slots[g_active_slot].bytes,
                                  g_slots[g_active_slot].len, &g_params);

#elif BEACON_ENABLE_IBEACON
    /* iBeacon only — no timer, no switching. */
    return rs_ble_broadcast_Start((const uint8_t *)&g_ib, IBEACON_ADV_LEN, &g_params);

#else /* BEACON_ENABLE_EDDYSTONE only */
    /* Eddystone-URL only — no timer, no switching. */
    return rs_ble_broadcast_Start((const uint8_t *)&g_ed,
                                  EDDYSTONE_URL_ADV_LEN(EDDYSTONE_CFG_URL_LEN),
                                  &g_params);
#endif
}

#if BEACON_DUAL
void beacon_set_switch(void)
{
    /* DA14531 supports one advertising set: a format switch is inherently
     * stop -> re-start with the other opaque payload. */
    ble_status_t status = rs_ble_broadcast_Stop();
    if (BLE_SUCCESS != status) {
        MSG_PRINT("ERR: beacon_set_switch: stop failed, status=0x%04X\n", (unsigned)status);
        __BKPT(0);
    }

    g_active_slot ^= 1U;   /* iBeacon <-> Eddystone-URL */

    status = rs_ble_broadcast_Start(g_slots[g_active_slot].bytes,
                                    g_slots[g_active_slot].len, &g_params);
    if (BLE_SUCCESS != status) {
        MSG_PRINT("ERR: beacon_set_switch: start failed, status=0x%04X\n", (unsigned)status);
        __BKPT(0);
    }
}
#endif /* BEACON_DUAL */
