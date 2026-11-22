/********************************************************************************
* BSD 3-Clause License
*
* Copyright (c) 2026, Renesas Electronics
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
*    list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its
*    contributors may be used to endorse or promote products derived from
*    this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************/

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
