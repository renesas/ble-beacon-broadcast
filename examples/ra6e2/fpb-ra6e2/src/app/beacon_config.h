/**
 * beacon_config.h  —  App compile-time beacon selection & timing (customer-editable)
 *
 * Choose which beacon format(s) this build broadcasts, and the advertising
 * interval. This is an app concern only — the RS (rs_ble_broadcast.{c,h}) is
 * format-agnostic and never sees these.
 *
 *   - Enable IBEACON only, EDDYSTONE only, or BOTH.
 *   - BOTH  → the app alternates them on a timer (see beacon_set.c); each
 *             format is on-air about half of every advertising interval.
 *   - ONE   → no timer and no switch code is compiled in at all (no false
 *             switches, no wasted RAM/CPU) — the single format just advertises.
 */

#ifndef BEACON_CONFIG_H
#define BEACON_CONFIG_H

/* ── Format selection — set either to 0 to exclude, or both to 1 for both. ── */
#ifndef BEACON_ENABLE_IBEACON
#define BEACON_ENABLE_IBEACON     1
#endif
#ifndef BEACON_ENABLE_EDDYSTONE
#define BEACON_ENABLE_EDDYSTONE   1
#endif

#if !BEACON_ENABLE_IBEACON && !BEACON_ENABLE_EDDYSTONE
#  error "beacon_config.h: enable at least one of BEACON_ENABLE_IBEACON / BEACON_ENABLE_EDDYSTONE"
#endif

/* True only when BOTH formats are active → alternate them on a timer. When
 * false (exactly one format), the beacon_set module builds no timer and no switch path. */
#define BEACON_DUAL   (BEACON_ENABLE_IBEACON && BEACON_ENABLE_EDDYSTONE)

/* ── Advertising interval, in 0.625 ms units. 1600 = 1000 ms (~1 s). ────────
 * In dual mode the format switch runs at interval/2 so, over each advertising
 * interval, each of the two formats gets roughly one turn on air (switch
 * latency is assumed negligible next to a ~1 s interval). */
#ifndef BEACON_ADV_INTERVAL_UNITS
#define BEACON_ADV_INTERVAL_UNITS   1600u
#endif

#endif /* BEACON_CONFIG_H */
