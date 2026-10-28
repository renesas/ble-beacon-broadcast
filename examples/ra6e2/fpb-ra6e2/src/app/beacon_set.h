/**
 * beacon_set.h  —  Beacon broadcast setup + (optional) runtime alternation
 *
 * The app owns all format knowledge and scheduling; the RS only ever sees
 * opaque bytes via rs_ble_broadcast_Start()/rs_ble_broadcast_Stop().
 *
 * Which format(s) are built is chosen in beacon_config.h:
 *   - Exactly ONE format  → beacon_set_init() just advertises it. NO timer,
 *     NO switch code is compiled in (BEACON_SET_EVT / beacon_set_switch() do
 *     not exist), so there are no false switches.
 *   - BOTH formats (BEACON_DUAL) → beacon_set_init() also starts a FreeRTOS
 *     software timer that fires at adv_interval/2 and raises BEACON_SET_EVT on
 *     the BLE event group. The BLE task, woken by that bit, calls
 *     beacon_set_switch() to stop()+start() the other format.
 *
 * Threading: the timer callback runs in the FreeRTOS timer-service task and only
 * sets the event bit — never the RS. The stop()/start() runs in the BLE task so
 * GTL/UART traffic stays serialised with R_BLE_Execute().
 */

#ifndef BEACON_SET_H
#define BEACON_SET_H

#include "beacon_config.h"
#include "r_ble_api.h"           /* ble_status_t */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Build the enabled payload(s), start advertising the first one, and — only when
 * BOTH formats are enabled — create and start the alternation timer. Call once,
 * after the BLE stack is GAP-ready, from the BLE task. Returns the first
 * failure (timer create/start or initial advertising start), else BLE_SUCCESS.
 */
ble_status_t beacon_set_init(void);

#if BEACON_DUAL
/* Event-group bit the alternation timer raises on g_ble_event_group_handle.
 * Chosen clear of the GTL driver's BLE_EVENT_PATTERN (0x0A0A) so they never
 * collide; the BLE task ORs this into its xEventGroupWaitBits() mask. Plain
 * literal so this header needs no FreeRTOS include; widens to EventBits_t at use. */
#define BEACON_SET_EVT   (1u << 16)

/**
 * Perform one format switch: toggle the active format and run
 * rs_ble_broadcast_Stop() + rs_ble_broadcast_Start() with the other payload. Call from
 * the BLE task ONLY, when xEventGroupWaitBits() reports BEACON_SET_EVT set.
 * (Only exists in dual-format builds.)
 */
void beacon_set_switch(void);
#endif /* BEACON_DUAL */

#ifdef __cplusplus
}
#endif

#endif /* BEACON_SET_H */
