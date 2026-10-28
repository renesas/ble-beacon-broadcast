/**
 * beacon_ibeacon.h  —  iBeacon format struct (app-owned, not part of the RS)
 *
 * The RS core is fully data-type-agnostic (see rs_ble_broadcast.h);
 * ALL beacon-format knowledge lives here, in the app. This header is pure C,
 * no FSP dependency, so its byte layout can be unit-tested on host.
 *
 * The struct is packed and cast DIRECTLY into the RS adv-data argument — its
 * bytes ARE the on-air AD payload, no serialization/builder step:
 *
 *   ibeacon_t ib = IBEACON_INIT(uuid, major, minor, power);
 *   rs_ble_broadcast_Start((const uint8_t *)&ib, IBEACON_ADV_LEN, &params);
 */

#ifndef BEACON_IBEACON_H
#define BEACON_IBEACON_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Recognizable DA14531 identity defaults (override before including) ────
 * major = 0xDA14, minor = 0x0531  →  reads "DA14 0531" = "DA14531"
 * UUID  = ASCII "RENESAS DA14531!"                                          */
#ifndef IBEACON_CFG_UUID
#define IBEACON_CFG_UUID   { 0x52,0x45,0x4E,0x45,0x53,0x41,0x53,0x20, \
                                  0x44,0x41,0x31,0x34,0x35,0x33,0x31,0x21 }
#endif
#ifndef IBEACON_CFG_MAJOR
#define IBEACON_CFG_MAJOR            0xDA14U
#endif
#ifndef IBEACON_CFG_MINOR
#define IBEACON_CFG_MINOR            0x0531U
#endif
/* Measured RSSI @ 1 m, dBm — MUST be calibrated per board/antenna. */
#ifndef IBEACON_CFG_MEASURED_POWER
#define IBEACON_CFG_MEASURED_POWER   (-59)
#endif

/* ── iBeacon wire-layout struct (packed, 30 bytes ≤ 31-byte legacy advertising
 * limit)
 * Flags AD stays FIRST on purpose: the DA14531 GTL driver detects a leading
 * Flags AD, strips it, and the DA14531 firmware re-adds its own. If this
 * struct were reordered, that strip would not trigger. */
typedef struct __attribute__((packed)) {
    uint8_t flags_len, flags_type, flags_val;   /* 02 01 06  Flags AD           */
    uint8_t mfr_len, mfr_type;                   /* 1A FF     Mfr-Specific AD    */
    uint8_t company_id[2];                       /* 4C 00     Apple, little-end. */
    uint8_t beacon_type, beacon_len;             /* 02 15     iBeacon subtype    */
    uint8_t uuid[16];                            /* proximity UUID              */
    uint8_t major[2];                            /* big-endian on air           */
    uint8_t minor[2];                            /* big-endian on air           */
    int8_t  measured_power;                      /* RSSI @ 1 m, dBm (calibrate) */
} ibeacon_t;                                 /* sizeof == 30 (≤ 31-byte limit) */

/* On-air length: fixed, the struct has no variable part. */
#define IBEACON_ADV_LEN   ((uint8_t)sizeof(ibeacon_t))   /* 30 */

/* Build an ibeacon_t initializer. major_/minor_ are big-endian on air. */
#define IBEACON_INIT(uuid16_, major_, minor_, power_) {                 \
    0x02,0x01,0x06, 0x1A,0xFF, {0x4C,0x00}, 0x02,0x15,                      \
    uuid16_,                                                                \
    {(uint8_t)((major_) >> 8), (uint8_t)(major_)},                         \
    {(uint8_t)((minor_) >> 8), (uint8_t)(minor_)},                         \
    (int8_t)(power_) }

/* Recognizable DA14531 default instance, built from the CFG defaults above. */
#define IBEACON_DEFAULT   \
    IBEACON_INIT(IBEACON_CFG_UUID, IBEACON_CFG_MAJOR, \
                      IBEACON_CFG_MINOR, IBEACON_CFG_MEASURED_POWER)

#ifdef __cplusplus
}
#endif

#endif /* BEACON_IBEACON_H */
