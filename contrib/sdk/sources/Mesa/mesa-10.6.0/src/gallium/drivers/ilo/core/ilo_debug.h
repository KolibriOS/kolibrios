/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2012-2013 LunarG, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Chia-I Wu <olv@lunarg.com>
 */

#ifndef ILO_DEBUG_H
#define ILO_DEBUG_H

#include "ilo_core.h"

/* enable debug flags affecting hot pathes only with debug builds */
#ifdef DEBUG
#define ILO_DEBUG_HOT 1
#else
#define ILO_DEBUG_HOT 0
#endif

enum ilo_debug {
   ILO_DEBUG_BATCH     = 1 << 0,
   ILO_DEBUG_VS        = 1 << 1,
   ILO_DEBUG_GS        = 1 << 2,
   ILO_DEBUG_FS        = 1 << 3,
   ILO_DEBUG_CS        = 1 << 4,
   ILO_DEBUG_DRAW      = ILO_DEBUG_HOT << 5,
   ILO_DEBUG_SUBMIT    = 1 << 6,
   ILO_DEBUG_HANG      = 1 << 7,

   /* flags that affect the behaviors of the driver */
   ILO_DEBUG_NOHW      = 1 << 20,
   ILO_DEBUG_NOCACHE   = 1 << 21,
   ILO_DEBUG_NOHIZ     = 1 << 22,
};

extern int ilo_debug;

void
ilo_debug_init(const char *name);

/**
 * Print a message, for dumping or debugging.
 */
static inline void _util_printf_format(1, 2)
ilo_printf(const char *format, ...)
{
   va_list ap;

   va_start(ap, format);
   _debug_vprintf(format, ap);
   va_end(ap);
}

/**
 * Print a critical error.
 */
static inline void _util_printf_format(1, 2)
ilo_err(const char *format, ...)
{
   va_list ap;

   va_start(ap, format);
   _debug_vprintf(format, ap);
   va_end(ap);
}

/**
 * Print a warning, silenced for release builds.
 */
static inline void _util_printf_format(1, 2)
ilo_warn(const char *format, ...)
{
#ifdef DEBUG
   va_list ap;

   va_start(ap, format);
   _debug_vprintf(format, ap);
   va_end(ap);
#else
#endif
}

#endif /* ILO_DEBUG_H */
