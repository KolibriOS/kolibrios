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

#ifndef _RHD_LUT_H
#define _RHD_LUT_H

struct rhdLUT {
    int scrnIndex;

    char *Name;
#define RHD_LUT_A 0
#define RHD_LUT_B 1
    int Id;

    void (*Save) (struct rhdLUT *LUT);
    void (*Restore) (struct rhdLUT *LUT);
    void (*Set) (struct rhdLUT *LUT, int numColors, int *indices, LOCO *colors);

    /* because RandR does not specifically initialise a gamma ramp when
       setting up a CRTC */
    Bool Initialised;

    Bool Stored;

    CARD32 StoreControl;

    CARD32 StoreBlackRed;
    CARD32 StoreBlackGreen;
    CARD32 StoreBlackBlue;

    CARD32 StoreWhiteRed;
    CARD32 StoreWhiteGreen;
    CARD32 StoreWhiteBlue;

    CARD16 StoreEntry[0x300];
};

void RHDLUTsInit(RHDPtr rhdPtr);
void RHDLUTsSave(RHDPtr rhdPtr);
void RHDLUTsRestore(RHDPtr rhdPtr);
void RHDLUTsDestroy(RHDPtr rhdPtr);

/* For missing RandR functionality */
void RHDLUTCopyForRR(struct rhdLUT *LUT);

#endif /* _RHD_LUT_H */
