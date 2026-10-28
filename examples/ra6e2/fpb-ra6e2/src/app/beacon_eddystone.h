/**
 * beacon_eddystone.h  —  Eddystone-URL format struct (app-owned, not the RS)
 *
 * The RS core is fully data-type-agnostic (see rs_ble_broadcast.h);
 * ALL beacon-format knowledge lives here, in the app. This header is pure C,
 * no FSP dependency, so its byte layout can be unit-tested on host.
 *
 * The struct is packed and cast DIRECTLY into the RS adv-data argument — its
 * bytes ARE the on-air AD payload (Eddystone spec: Service Data AD, UUID
 * 0xFEAA, frame type 0x10). Because the URL is variable-length, the struct is
 * sized to the maximum and the caller passes EDDYSTONE_URL_ADV_LEN(url_len)
 * as adv_len — the trailing unused url[] bytes are never transmitted:
 *
 *   eddystone_url_t ed = EDDYSTONE_URL_INIT(tx, scheme, url_bytes, n);
 *   rs_ble_broadcast_Start((const uint8_t *)&ed, EDDYSTONE_URL_ADV_LEN(n), &params);
 */

#ifndef BEACON_EDDYSTONE_H
#define BEACON_EDDYSTONE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Eddystone URL scheme prefix (encoded first byte of the URL body). */
typedef enum {
    EDDYSTONE_SCHEME_HTTP_WWW  = 0x00,  /* http://www.  */
    EDDYSTONE_SCHEME_HTTPS_WWW = 0x01,  /* https://www. */
    EDDYSTONE_SCHEME_HTTP      = 0x02,  /* http://      */
    EDDYSTONE_SCHEME_HTTPS     = 0x03,  /* https://     */
} eddystone_scheme_t;

/* Max encoded URL bytes so the total legacy adv frame stays <= 31 bytes
 * (14-byte fixed prefix + up to 17 bytes of URL = 31). */
#define EDDYSTONE_URL_MAX   17

/* ── Recognizable default identity (override before including) ───────────── */
#ifndef EDDYSTONE_CFG_TX_POWER
#define EDDYSTONE_CFG_TX_POWER    (-22)          /* ranging data @ 0 m, dBm */
#endif
#ifndef EDDYSTONE_CFG_SCHEME
#define EDDYSTONE_CFG_SCHEME      EDDYSTONE_SCHEME_HTTP_WWW
#endif
#ifndef EDDYSTONE_CFG_URL
#define EDDYSTONE_CFG_URL         { 'r','e','n','e','s','a','s','.','c','o','m' }
#endif
#ifndef EDDYSTONE_CFG_URL_LEN
#define EDDYSTONE_CFG_URL_LEN     11U            /* "renesas.com" */
#endif

/* ── Eddystone-URL wire-layout struct (packed; url[] is NUL/zero-padded —
 * only url_len bytes of it are ever transmitted, per EDDYSTONE_URL_ADV_LEN). */
typedef struct __attribute__((packed)) {
    uint8_t flags_len, flags_type, flags_val;   /* 02 01 06   Flags AD                     */
    uint8_t uuid_len, uuid_type;                /* 03 03      Complete 16-bit UUID list    */
    uint8_t uuid_list[2];                        /* AA FE      Eddystone, little-endian     */
    uint8_t svc_len;                             /* 06 + url_len   Service Data AD length    */
    uint8_t svc_type;                            /* 16         Service Data AD type          */
    uint8_t svc_uuid[2];                         /* AA FE                                    */
    uint8_t frame_type;                          /* 10         Eddystone-URL                 */
    int8_t  tx_power;                            /* ranging data, dBm @ 0 m                  */
    uint8_t url_scheme;                          /* eddystone_scheme_t                   */
    uint8_t url[EDDYSTONE_URL_MAX];          /* encoded URL; only url_len bytes valid    */
} eddystone_url_t;

/* On-air length for a given URL length: fixed 14-byte prefix + URL bytes (≤31). */
#define EDDYSTONE_URL_ADV_LEN(url_len)   ((uint8_t)(14u + (url_len)))

/* Build an eddystone_url_t initializer. svc_len is derived from url_len_.
 *
 * SAFE USAGE IS COMPILE-TIME LITERALS ONLY: url_bytes_ must be a
 * brace-enclosed byte list (like EDDYSTONE_CFG_URL) no longer than
 * EDDYSTONE_URL_MAX bytes — the compiler rejects an oversize literal
 * ("excess elements in array initializer"). There is no runtime bounds
 * check on url[]; if a caller ever populates it dynamically (e.g. memcpy
 * from a runtime string) instead of via this macro, exceeding
 * EDDYSTONE_URL_MAX bytes is a buffer overflow the APP must prevent itself. */
#define EDDYSTONE_URL_INIT(tx_, scheme_, url_bytes_, url_len_) {        \
    0x02,0x01,0x06, 0x03,0x03, {0xAA,0xFE},                                 \
    (uint8_t)(0x06u + (url_len_)), 0x16, {0xAA,0xFE}, 0x10,                 \
    (int8_t)(tx_), (uint8_t)(scheme_), url_bytes_ }

/* Recognizable default instance ("renesas.com", HTTP_WWW scheme), built from
 * the CFG defaults above. Pair with EDDYSTONE_URL_ADV_LEN(EDDYSTONE_CFG_URL_LEN). */
#define EDDYSTONE_URL_DEFAULT   \
    EDDYSTONE_URL_INIT(EDDYSTONE_CFG_TX_POWER, EDDYSTONE_CFG_SCHEME, \
                            EDDYSTONE_CFG_URL, EDDYSTONE_CFG_URL_LEN)

#ifdef __cplusplus
}
#endif

#endif /* BEACON_EDDYSTONE_H */
