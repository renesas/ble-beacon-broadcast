/**
 * rs_ble_broadcast.h  —  RS: BLE Beacon Broadcasting (over DA14531)
 *
 * PUBLIC API (customer-facing). Fully DATA-TYPE-AGNOSTIC:
 *   - The RS does NOT know iBeacon, Eddystone, or any beacon format.
 *   - The APP builds the advertising bytes itself (in its own format struct)
 *     and hands this RS a ready adv-data buffer + length.
 *   - This RS only issues the non-connectable GAP advertising sequence
 *     (start) and halts it (stop), via direct FSP r_ble calls.
 *   - Non-connectable is enforced at the controller/link-layer level
 *     (ADV_NONCONN_IND rejects connection attempts before they reach the
 *     application). Neither the RS nor the APP needs, or has, any
 *     application-level connection-rejection logic.
 *
 * Two functions — start + stop. Stop+start is also how the APP alternates
 * between formats at runtime (it re-starts with a different byte buffer); the
 * RS neither schedules nor is aware of that — it has no timer and no state.
 *
 * PRECONDITION — BLE init is NOT this RS's job:
 *   The APP (the project that includes this RS) must already have brought the
 *   BLE stack up (DA14531 GTL firmware loaded, UART open, rm_ble_abs_gtl open,
 *   R_BLE_GAP_Init done, GAP-ready) as part of its normal init — like bus/clock/
 *   platform init. These functions only start/stop advertising.
 *
 * Copy-paste: drop rs_ble_broadcast.{c,h} into a project's src/. Depends only
 * on FSP (r_ble_api.h). Returns the FSP-native ble_status_t (BLE_SUCCESS == 0).
 */

#ifndef RS_BLE_BROADCAST_H
#define RS_BLE_BROADCAST_H

#include <stdint.h>              /* uint8_t / uint16_t                                */
#include "r_ble_api.h"          /* FSP: ble_status_t, R_BLE_GAP_*, BLE_VS_TX_POWER_*, BLE_GAP_ADV_CH_* */

#ifdef __cplusplus
extern "C" {
#endif

/* Max legacy advertising payload (bytes). The RS rejects anything larger. */
#define RS_BLE_BROADCAST_ADV_MAX   31U

/* ── Advertising parameters (non-payload config) ────────────────────────────
 * tx_power / channel_map take the FSP values directly — no re-aliased enums:
 *   tx_power    : BLE_VS_TX_POWER_HIGH | _MID | _LOW   (r_ble_api.h)
 *                 NOTE: HIGH is MAXIMUM radio power. This is the RADIO level,
 *                 not the calibration byte the APP puts inside the payload.
 *   channel_map : bitwise OR of BLE_GAP_ADV_CH_37/38/39, or BLE_GAP_ADV_CH_ALL
 */
typedef struct {
    uint16_t adv_interval;   /* 0.625 ms units; 160 = 100 ms. Min 0x20.        */
    uint8_t  tx_power;        /* BLE_VS_TX_POWER_HIGH / _MID / _LOW              */
    uint8_t  channel_map;     /* BLE_GAP_ADV_CH_ALL (37/38/39)                   */
} rs_ble_broadcast_adv_params_t;

/**
 * START — advertise a caller-supplied, fully-formed advertising payload.
 *
 * Issues, in order (non-connectable legacy advertising, GAP Broadcaster):
 *   R_BLE_GAP_SetAdvParam (ADV_NONCONN_IND) → R_BLE_VS_SetTxPower →
 *   R_BLE_GAP_SetAdvSresData (opaque bytes) → R_BLE_GAP_StartAdv.
 *
 * The payload is treated as opaque bytes — the RS never parses it.
 * PRECONDITION: BLE stack GAP-ready (see file header). Call from the BLE task.
 *
 * @param adv_data [in] fully-formed AD payload (built by the APP). Non-NULL.
 * @param adv_len  [in] payload length, 1..RS_BLE_BROADCAST_ADV_MAX (31).
 * @param params   [in] advertising parameters. Non-NULL.
 * @return BLE_SUCCESS on success; BLE_ERR_INVALID_ARG for a bad argument;
 *         otherwise the failing R_BLE_GAP / R_BLE_VS status, unchanged.
 */
ble_status_t rs_ble_broadcast_Start(const uint8_t                  *adv_data,
                                    uint8_t                         adv_len,
                                    const rs_ble_broadcast_adv_params_t *params);

/**
 * STOP — halt advertising (R_BLE_GAP_StopAdv on the fixed advertising handle).
 *
 * No arguments (the handle is a compile-time constant), so stop holds no state.
 * Safe to call when not advertising — the FSP status is returned unchanged.
 * After stop, rs_ble_broadcast_Start() may be called again (this is how the APP
 * switches formats at runtime).
 *
 * @return BLE_SUCCESS on success; otherwise the R_BLE_GAP_StopAdv status.
 */
ble_status_t rs_ble_broadcast_Stop(void);

#ifdef __cplusplus
}
#endif

#endif /* RS_BLE_BROADCAST_H */
