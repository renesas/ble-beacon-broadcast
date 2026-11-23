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
 * msg_print.h  —  Debug print hook for the app
 *
 * MSG_PRINT(fmt, ...) prints over SEGGER RTT (J-Link) when RTT_DEBUG_ON is
 * defined (set in CMakeLists.txt); otherwise it compiles to nothing. To route
 * it to your own debug print instead, define MSG_PRINT before this header is
 * included, e.g. -DMSG_PRINT=my_printf.
 */

#ifndef MSG_PRINT_H
#define MSG_PRINT_H

#ifndef MSG_PRINT
#ifdef RTT_DEBUG_ON
#include "SEGGER_RTT/SEGGER_RTT.h"
/* RTT format subset: %c %d %u %x %X %s %p (no %f, no 64-bit). */
#define MSG_PRINT(...)   ((void)SEGGER_RTT_printf(0U, __VA_ARGS__))
#else
#define MSG_PRINT(...)   ((void)0)
#endif
#endif

#endif /* MSG_PRINT_H */
