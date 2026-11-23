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
