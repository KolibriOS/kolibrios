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

#ifndef _RHD_PLL_H
#define _RHD_PLL_H

struct rhdPLL {
    int scrnIndex;

#define PLL_NAME_PLL1 "PLL 1"
#define PLL_NAME_PLL2 "PLL 2"
    char *Name;

/* also used as an index to rhdPtr->PLLs */
#define PLL_ID_PLL1  0
#define PLL_ID_PLL2  1
#define PLL_ID_NONE -1
    int Id;

    CARD32 CurrentClock;
    Bool Active;

    /* from defaults or from atom */
    CARD32 RefClock;
    CARD32 IntMin;
    CARD32 IntMax;
    CARD32 PixMin;
    CARD32 PixMax;

    ModeStatus (*Valid) (struct rhdPLL *PLL, CARD32 Clock);
    void (*Set) (struct rhdPLL *PLL, int PixelClock, CARD16 ReferenceDivider,
		 CARD16 FeedbackDivider, CARD8 PostDivider);
    void (*Power) (struct rhdPLL *PLL, int Power);
    void (*Save) (struct rhdPLL *PLL);
    void (*Restore) (struct rhdPLL *PLL);

    /* For save/restore: Move to a Private */
    Bool Stored;

    void *Private;

    Bool StoreActive;
    Bool StoreCrtc1Owner;
    Bool StoreCrtc2Owner;
    CARD32 StoreRefDiv;
    CARD32 StoreFBDiv;
    CARD32 StorePostDiv;
    CARD32 StoreControl;
    CARD32 StoreSpreadSpectrum;

    /* RV620/RV635/RS780 */
    Bool StoreDCCGCLKOwner;
    CARD32 StoreDCCGCLK;
    CARD8 StoreScalerPostDiv;
    CARD8 StoreSymPostDiv;
    CARD32 StorePostDivSrc;
    Bool StoreGlitchReset;
};

Bool RHDPLLsInit(RHDPtr rhdPtr);
ModeStatus RHDPLLValid(struct rhdPLL *PLL, CARD32 Clock);
void RHDPLLSet(struct rhdPLL *PLL, CARD32 Clock);
void RHDPLLPower(struct rhdPLL *PLL, int Power);
void RHDPLLsPowerAll(RHDPtr rhdPtr, int Power);
void RHDPLLsShutdownInactive(RHDPtr rhdPtr);
void RHDPLLsSave(RHDPtr rhdPtr);
void RHDPLLsRestore(RHDPtr rhdPtr);
void RHDPLLsDestroy(RHDPtr rhdPtr);

void RHDSetupLimits(RHDPtr rhdPtr, CARD32 *RefClock,
		    CARD32 *IntMin, CARD32 *IntMax,
		    CARD32 *PixMin, CARD32 *PixMax);
Bool RHDAtomPLLsInit(RHDPtr rhdPtr);

#endif /* _RHD_PLL_H */
