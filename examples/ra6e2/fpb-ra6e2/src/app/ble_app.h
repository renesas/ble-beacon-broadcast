#ifndef BLE_APP_H
#define BLE_APP_H

/*
 * ble_app.h — entry point for the portable BLE beacon application body.
 *
 * FSP owns the RTOS thread entry symbol `ble_thread_entry` and generates a stub
 * for it into each board project's own (FSP-owned) src/ folder. To keep the
 * actual task logic (stack-ready wait, payload build, advertising start, and the
 * dual-format switch service loop) SINGLE-SOURCED across every board variant,
 * that logic lives here in the shared app folder as ble_app_run(); the per-board
 * src/ble_thread_entry.c is a one-line shim:
 *
 *     #include "ble_app.h"
 *     void ble_thread_entry(void * pv) { ble_app_run(pv); }
 */

void ble_app_run(void * pvParameters);

#endif /* BLE_APP_H */
