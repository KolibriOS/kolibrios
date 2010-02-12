/*
 * Copyright 2004-2008  Luc Verhaegen <lverhaegen@novell.com>
 * Copyright 2007, 2008 Matthias Hopf <mhopf@novell.com>
 * Copyright 2007, 2008 Egbert Eich   <eich@novell.com>
 * Copyright 2007, 2008 Advanced Micro Devices, Inc.
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

#ifndef _RHD_OUTPUT_H
#define _RHD_OUTPUT_H

/* Also needed for connector -> output mapping */
typedef enum rhdOutputType {
    RHD_OUTPUT_NONE  = 0,
    RHD_OUTPUT_DAC_EXTERNAL = RHD_OUTPUT_NONE,
    RHD_OUTPUT_DACA,
    RHD_OUTPUT_DACB,
    RHD_OUTPUT_TMDSA,
    RHD_OUTPUT_LVTMA,
    RHD_OUTPUT_DVO,
    RHD_OUTPUT_KLDSKP_LVTMA,
    RHD_OUTPUT_UNIPHYA,
    RHD_OUTPUT_UNIPHYB,
    RHD_OUTPUT_UNIPHYC,
    RHD_OUTPUT_UNIPHYD,
    RHD_OUTPUT_UNIPHYE,
    RHD_OUTPUT_UNIPHYF,
    RHD_OUTPUT_TMDSB = RHD_OUTPUT_NONE,
    RHD_OUTPUT_LVDS = RHD_OUTPUT_NONE,
    RHD_OUTPUT_LVTMB = RHD_OUTPUT_NONE
} rhdOutputType;

typedef enum rhdSensedOutput {
    RHD_SENSED_NONE = 0,
    RHD_SENSED_VGA,
    RHD_SENSED_DVI,
    RHD_SENSED_TV_SVIDEO,
    RHD_SENSED_TV_COMPOSITE,
    RHD_SENSED_TV_COMPONENT
} rhdSensedOutput;

enum rhdOutputProperty {
    RHD_OUTPUT_BACKLIGHT,
    RHD_OUTPUT_COHERENT
};

enum rhdOutputAllocation {
    RHD_OUTPUT_ALLOC,
    RHD_OUTPUT_FREE
};

char *rhdPowerString[4];

/*
 *
 * This structure should deal with everything output related.
 *
 */
struct rhdOutput {
    struct rhdOutput *Next;

    int scrnIndex;
    RHDPtr rhdPtr;

    char *Name;
    enum rhdOutputType Id;

    Bool Active;

    struct rhdCrtc *Crtc;
    struct rhdConnector *Connector;

    enum rhdSensedOutput SensedType;

    enum rhdSensedOutput (*Sense) (struct rhdOutput *Output,
				   struct rhdConnector *Connector);
    ModeStatus (*ModeValid) (struct rhdOutput *Output, DisplayModePtr Mode);
    void (*Mode) (struct rhdOutput *Output, DisplayModePtr Mode);
    void (*Power) (struct rhdOutput *Output, int Power);
    void (*Save) (struct rhdOutput *Output);
    void (*Restore) (struct rhdOutput *Output);
    void (*Destroy) (struct rhdOutput *Output);
    Bool (*Property) (struct rhdOutput *Output,
		      enum rhdPropertyAction Action, enum rhdOutputProperty Property, union rhdPropertyData *val);
    Bool (*AllocFree) (struct rhdOutput *Output, enum rhdOutputAllocation Alloc);
    /* Driver Private data */
    rhdOutputDriverPrivate *OutputDriverPrivate;
    /* Output Private data */
    void *Private;
};

void RHDOutputAdd(RHDPtr rhdPtr, struct rhdOutput *Output);
void RHDOutputsMode(RHDPtr rhdPtr, struct rhdCrtc *Crtc, DisplayModePtr Mode);
void RHDOutputsPower(RHDPtr rhdPtr, int Power);
void RHDOutputsShutdownInactive(RHDPtr rhdPtr);
void RHDOutputsSave(RHDPtr rhdPtr);
void RHDOutputsRestore(RHDPtr rhdPtr);
void RHDOutputsDestroy(RHDPtr rhdPtr);
void RHDOutputPrintSensedType(struct rhdOutput *Output);

/* output local functions. */
struct rhdOutput *RHDDACAInit(RHDPtr rhdPtr);
struct rhdOutput *RHDDACBInit(RHDPtr rhdPtr);
struct rhdOutput *RHDTMDSAInit(RHDPtr rhdPtr);
struct rhdOutput *RHDLVTMAInit(RHDPtr rhdPtr, CARD8 Type);
struct rhdOutput *RHDDIGInit(RHDPtr rhdPtr,  enum rhdOutputType outputType, CARD8 ConnectorType);
struct rhdOutput *RHDDDIAInit(RHDPtr rhdPtr);
struct rhdOutput *RHDAtomOutputInit(RHDPtr rhdPtr, rhdConnectorType ConnectorType, rhdOutputType OutputType);

#endif /* _RHD_OUTPUT_H */
