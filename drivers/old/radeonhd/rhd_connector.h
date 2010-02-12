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

#ifndef _RHD_CONNECTOR_H
#define _RHD_CONNECTOR_H

/* so that we can map which is which */
typedef enum rhdConnectorType {
    RHD_CONNECTOR_NONE  = 0,
    RHD_CONNECTOR_VGA,
    RHD_CONNECTOR_DVI,
    RHD_CONNECTOR_DVI_SINGLE,
    RHD_CONNECTOR_PANEL,
    RHD_CONNECTOR_TV,
    RHD_CONNECTOR_PCIE
} rhdConnectorType;
/* add whatever */

/* map which DDC bus is where */
typedef enum _rhdDDC {
    RHD_DDC_0 = 0,
    RHD_DDC_1,
    RHD_DDC_2,
    RHD_DDC_3,
    RHD_DDC_4,
    RHD_DDC_MAX,
    RHD_DDC_NONE  = 0xFF,
    RHD_DDC_GPIO = RHD_DDC_NONE
} rhdDDC;

/* map which HPD plug is used where */
typedef enum _rhdHPD {
    RHD_HPD_NONE  = 0,
    RHD_HPD_0,
    RHD_HPD_1,
    RHD_HPD_2,
    RHD_HPD_3
} rhdHPD;

#define MAX_OUTPUTS_PER_CONNECTOR 2

struct rhdConnector {
    int scrnIndex;

    CARD8 Type;
    char *Name;

    struct _I2CBusRec *DDC;

    /* HPD handling here */
    int  HPDMask;
    Bool HPDAttached;
    Bool (*HPDCheck) (struct rhdConnector *Connector);

    /* Add rhdMonitor pointer here. */
    /* This is created either from default, config or from EDID */
    struct rhdMonitor *Monitor;

    /* Point back to our Outputs, so we can handle sensing better */
    struct rhdOutput *Output[MAX_OUTPUTS_PER_CONNECTOR];
};

Bool RHDConnectorsInit(RHDPtr rhdPtr, struct rhdCard *Card);
void RHDHPDSave(RHDPtr rhdPtr);
void RHDHPDRestore(RHDPtr rhdPtr);
void RHDConnectorsDestroy(RHDPtr rhdPtr);
Bool RHDConnectorEnableHDMI(struct rhdConnector *Connector);

#endif /* _RHD_CONNECTOR_H */
