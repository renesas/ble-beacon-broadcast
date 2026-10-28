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
