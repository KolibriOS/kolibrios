/*
 * Copyright 2007  Luc Verhaegen <lverhaegen@novell.com>
 * Copyright 2007  Matthias Hopf <mhopf@novell.com>
 * Copyright 2007  Egbert Eich   <eich@novell.com>
 * Copyright 2007  Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _RHD_MONITOR_H
#define _RHD_MONITOR_H

struct rhdMonitor {
    int scrnIndex;

    char *Name;

    int xDpi;
    int yDpi;

    int numHSync;                  /* default: 0 */
    range HSync[MAX_HSYNC];
    int numVRefresh;               /* default: 0 */
    range VRefresh[MAX_VREFRESH];
    int Bandwidth;                 /* default 0 */

    Bool ReducedAllowed;

    Bool UseFixedModes;
    DisplayModePtr Modes; /* default: NULL */
    DisplayModePtr NativeMode;

    xf86MonPtr EDID;
};


void RHDConfigMonitorSet(RHDPtr rhdPtr, Bool UseConfig);

#ifdef _RHD_CONNECTOR_H
struct rhdMonitor *RHDMonitorInit(struct rhdConnector *Connector);
#endif

void RHDMonitorDestroy(struct rhdMonitor *Monitor);
void RHDMonitorPrint(struct rhdMonitor *Monitor);

#endif /* _RHD_MONITOR_H */
