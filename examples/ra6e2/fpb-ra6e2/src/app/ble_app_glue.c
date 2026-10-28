/**
 * ble_app_glue.c  —  app-provided BLE globals the FSP GTL stack/driver expect.
 *
 * In Renesas's QE-generated "app_main.c" template these globals live alongside a
 * full GATT server/client application. A non-connectable iBeacon needs none of
 * that, so we provide only the minimum the generated g_ble_abs0_cfg and the GTL
 * driver reference:
 *   - empty GATT server/client callback sets (no GATT services for a beacon), and
 *   - the BLE event group the GTL driver signals on (r_ble_gtl.c).
 *
 * The BLE app logic (gap_cb / vs_cb, open, start-once, event loop) is in
 * ble_app.c — deliberately NOT here, so there is no app_main.c and no
 * duplicate-callback clash.
 */

#include <stddef.h>              /* NULL                                     */
#include "r_ble_api.h"           /* callback function pointer types          */
#include "rm_ble_abs_api.h"      /* ble_abs_gatt_server/client_callback_set_t */
#include "FreeRTOS.h"
#include "event_groups.h"        /* EventGroupHandle_t                       */

/* Used by the GTL driver (declared extern inside r_ble_gtl.c). It is CREATED in
 * ble_app.c (ble_app_run) before RM_BLE_ABS_Open(). */
EventGroupHandle_t g_ble_event_group_handle;

/* GATT server/client callback lists referenced by g_ble_abs0_cfg
 * (.p_gatt_server_callback_list / .p_gatt_client_callback_list, count = 2).
 * A beacon serves no GATT, so both entries are NULL. */
ble_abs_gatt_server_callback_set_t gs_abs_gatts_cb_param[] =
{
    { .gatt_server_callback_function = NULL },
    { .gatt_server_callback_function = NULL },
};

ble_abs_gatt_client_callback_set_t gs_abs_gattc_cb_param[] =
{
    { .gatt_client_callback_function = NULL },
    { .gatt_client_callback_function = NULL },
};
