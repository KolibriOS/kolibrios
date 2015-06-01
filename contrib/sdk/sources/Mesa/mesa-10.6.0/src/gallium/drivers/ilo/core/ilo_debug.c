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

#include "ilo_debug.h"

static const struct debug_named_value ilo_debug_flags[] = {
   { "batch",     ILO_DEBUG_BATCH,    "Dump batch/dynamic/surface/instruction buffers" },
   { "vs",        ILO_DEBUG_VS,       "Dump vertex shaders" },
   { "gs",        ILO_DEBUG_GS,       "Dump geometry shaders" },
   { "fs",        ILO_DEBUG_FS,       "Dump fragment shaders" },
   { "cs",        ILO_DEBUG_CS,       "Dump compute shaders" },
   { "draw",      ILO_DEBUG_DRAW,     "Show draw information" },
   { "submit",    ILO_DEBUG_SUBMIT,   "Show batch buffer submissions" },
   { "hang",      ILO_DEBUG_HANG,     "Detect GPU hangs" },
   { "nohw",      ILO_DEBUG_NOHW,     "Do not send commands to HW" },
   { "nocache",   ILO_DEBUG_NOCACHE,  "Always invalidate HW caches" },
   { "nohiz",     ILO_DEBUG_NOHIZ,    "Disable HiZ" },
   DEBUG_NAMED_VALUE_END
};

int ilo_debug;

void
ilo_debug_init(const char *name)
{
   ilo_debug = debug_get_flags_option(name, ilo_debug_flags, 0);
}
