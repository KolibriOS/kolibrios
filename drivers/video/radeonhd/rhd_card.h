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

#ifndef _RHD_CARD_H
#define _RHD_CARD_H

/* Four bytes in TYPE/DDC layout: see rhd_connector.h */
struct rhdConnectorInfo {
    rhdConnectorType Type;
    char *Name;
    rhdDDC DDC;
    rhdHPD HPD;
    rhdOutputType Output[MAX_OUTPUTS_PER_CONNECTOR];
};

/* Some card specific flags, where and when needed */
enum rhdCardFlag {
    RHD_CARD_FLAG_NONE  = 0,
    RHD_CARD_FLAG_DMS59 = 1,    /* DMS59 connector is only reported as two DVI-I */
    RHD_CARD_FLAG_HPDSWAP = 2,  /* some cards have broken connector tables */
    RHD_CARD_FLAG_HPDOFF = 4    /* some have *very* broken connector tables */
};

struct rhdCard {
    CARD16 device;
    CARD16 card_vendor;
    CARD16 card_device;
    char *name;
    enum rhdCardFlag flags;

    struct rhdConnectorInfo ConnectorInfo[RHD_CONNECTORS_MAX];
#ifdef ATOM_BIOS
    enum atomDevice DeviceInfo[RHD_CONNECTORS_MAX][MAX_OUTPUTS_PER_CONNECTOR];
#endif
};

void RhdPrintConnectorInfo(RHDPtr rhdPtr, struct rhdConnectorInfo *cp);

#endif /* _RHD_CARD_H */
