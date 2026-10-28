/**
 * rs_ble_broadcast.c  —  BLE Beacon advertising driver (over DA14531)
 *
 * Direct FSP calls, no abstraction layers, and NO beacon-format knowledge.
 * The caller (APP) hands in a fully-formed advertising payload (opaque bytes);
 * this file only runs the non-connectable GAP advertising sequence and halts
 * it. Meant to be readable and copy-pasteable into any FSP BLE project.
 *
 * rm_ble_abs_gtl provides no non-connectable helper, so the low-level
 * R_BLE_GAP / R_BLE_VS API is used (the supported path on the GTL/DA14531
 * transport).
 *
 * PRECONDITION: the BLE stack is already up (BLE init done by the APP). This
 * file does NOT open the UART, load DA14531 firmware, or open rm_ble_abs_gtl.
 */

#include "rs_ble_broadcast.h"   /* this module: adv params + start/stop API   */
#include "r_ble_api.h"             /* FSP r_ble API: R_BLE_GAP_*, R_BLE_VS_*, types */

/* Fixed advertising handle, shared by start and stop. The DA14531 supports a
 * single advertising set, so this is always 0; being constant, stop needs no
 * stored state. */
#define RS_BLE_BROADCAST_ADV_HANDLE   0x00U

/* Fill the non-connectable legacy advertising parameters. Small, clearly-named
 * helper so the start sequence reads top-to-bottom. */
static void rs_ble_broadcast_fill_adv_param(const rs_ble_broadcast_adv_params_t *p,
                                            st_ble_gap_adv_param_t *param)
{
    param->adv_hdl       = RS_BLE_BROADCAST_ADV_HANDLE;
    /* Always use the symbolic macro here, never a hard-coded numeric value:
     * BLE_GAP_LEGACY_PROP_ADV_NONCONN_IND resolves to a different number
     * (0x0010 vs 0x0003) depending on BLE_CFG_LIBRARY_TYPE, and the GTL driver
     * compares against the same macro — a literal would drift out of sync. */
    param->adv_prop_type = BLE_GAP_LEGACY_PROP_ADV_NONCONN_IND;  /* non-connectable */
    param->adv_intv_min  = p->adv_interval;
    param->adv_intv_max  = p->adv_interval;
    param->adv_ch_map    = p->channel_map;                       /* 37/38/39 */
    /* No o_addr_type: the DA14531 GTL driver fixes the advertiser address to
     * PUBLIC at init and never reads this field on the adv-param path. */
    param->adv_phy       = BLE_GAP_ADV_PHY_1M;
    param->filter_policy = BLE_GAP_ADV_ALLOW_SCAN_ANY_CONN_ANY;
}

ble_status_t rs_ble_broadcast_Start(const uint8_t                  *adv_data,
                                    uint8_t                         adv_len,
                                    const rs_ble_broadcast_adv_params_t *params)
{
    /* Validate everything before touching FSP (fail fast, actionable errors). */
    if ((adv_data == NULL) || (params == NULL)) {
        return BLE_ERR_INVALID_ARG;
    }
    if ((adv_len == 0U) || (adv_len > RS_BLE_BROADCAST_ADV_MAX)) {
        return BLE_ERR_INVALID_ARG;    /* 1..31 bytes; never truncate */
    }
    /* FSP min advertising interval is 0x00000020 (see r_ble_api.h). */
    if (params->adv_interval < 0x20U) {
        return BLE_ERR_INVALID_ARG;
    }
    /* Only bits 0x01/0x02/0x04 (channels 37/38/39) are legal (BLE_GAP_ADV_CH_ALL
     * = 0x07); reject 0 or any other bit so a typo'd bitmask fails fast. */
    if ((params->channel_map == 0U) ||
        ((params->channel_map & (uint8_t)~BLE_GAP_ADV_CH_ALL) != 0U)) {
        return BLE_ERR_INVALID_ARG;
    }
    /* R_BLE_VS_SetTxPower accepts only these three levels. */
    if ((params->tx_power != BLE_VS_TX_POWER_HIGH) &&
        (params->tx_power != BLE_VS_TX_POWER_MID)  &&
        (params->tx_power != BLE_VS_TX_POWER_LOW)) {
        return BLE_ERR_INVALID_ARG;
    }

    ble_status_t status;

    /* 1. Set non-connectable advertising parameters. */
    st_ble_gap_adv_param_t adv_param = {0};
    rs_ble_broadcast_fill_adv_param(params, &adv_param);
    status = R_BLE_GAP_SetAdvParam(&adv_param);
    if (status != BLE_SUCCESS) {
        return status;
    }

    /* 2. Set Tx power for the non-connected (advertising) state. Per the FSP
     *    contract, conn_hdl must be BLE_GAP_INIT_CONN_HDL (0xFFFF) here — NOT
     *    the advertising handle — because this call is scoped to the
     *    not-connected state, not to a specific connection. */
    status = R_BLE_VS_SetTxPower(BLE_GAP_INIT_CONN_HDL, params->tx_power);
    if (status != BLE_SUCCESS) {
        return status;
    }

    /* 3. Load the advertising payload (opaque bytes — format-agnostic). */
    st_ble_gap_adv_data_t adv_data_param = {0};
    adv_data_param.adv_hdl     = RS_BLE_BROADCAST_ADV_HANDLE;
    adv_data_param.data_type   = BLE_GAP_ADV_DATA_MODE;
    adv_data_param.data_length = adv_len;
    adv_data_param.p_data      = (uint8_t *)adv_data;   /* FSP takes non-const; not modified */
    status = R_BLE_GAP_SetAdvSresData(&adv_data_param);
    if (status != BLE_SUCCESS) {
        return status;
    }

    /* 4. Start advertising continuously (duration 0, no event limit). */
    return R_BLE_GAP_StartAdv(RS_BLE_BROADCAST_ADV_HANDLE, 0x0000, 0x00);
}

ble_status_t rs_ble_broadcast_Stop(void)
{
    /* Halt advertising on the same fixed handle used by start. Nothing else to
     * tear down (no state, no allocation). Safe to call when not advertising —
     * the FSP status is returned unchanged. This is also the first half of a
     * runtime format switch (the APP then calls start again with new bytes). */
    return R_BLE_GAP_StopAdv(RS_BLE_BROADCAST_ADV_HANDLE);
}
