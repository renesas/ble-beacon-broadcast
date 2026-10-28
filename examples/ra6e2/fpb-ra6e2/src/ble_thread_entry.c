/*
 * ble_thread_entry.c — FSP-owned RTOS thread entry (per board project).
 *
 * FSP generates/expects this file at the project's src/ folder for the "ble_thread"
 * thread. It is intentionally a THIN SHIM: all the board-agnostic BLE application
 * logic lives in the shared app folder (ble_app.c / ble_app_run), so it is
 * single-sourced across every board variant. Because this file exists, "Generate
 * Project Content" will not overwrite it.
 */

#include "ble_app.h"   /* ble_app_run() — shared, board-agnostic task body */

void ble_thread_entry(void * pvParameters)
{
    ble_app_run(pvParameters);
}
