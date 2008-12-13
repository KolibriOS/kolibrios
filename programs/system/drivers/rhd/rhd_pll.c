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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xf86.h"

/* for usleep */
#if HAVE_XF86_ANSIC_H
# include "xf86_ansic.h"
#else
# include <unistd.h>
#endif

#include "rhd.h"
#include "rhd_crtc.h"
#include "rhd_pll.h"
#include "rhd_regs.h"
#include "rhd_atombios.h"


#define PLL_CALIBRATE_WAIT 0x100000

/*
 * Get gain, charge pump, loop filter and current bias.
 * For R500, this is done in atombios by ASIC_RegistersInit
 * Some data table in atom should've provided this information.
 */

struct PLL_Control {
    CARD16 FeedbackDivider; /* 0xFFFF/-1 is the endmarker here */
    CARD32 Control;
};

/* From hardcoded values. */
static struct PLL_Control RV610PLLControl[] =
{
    { 0x0049, 0x159F8704 },
    { 0x006C, 0x159B8704 },
    { 0xFFFF, 0x159EC704 }
};

/* Some tables are provided by atombios,
 * it's just that they are hidden away deliberately and not exposed */
static struct PLL_Control RV670PLLControl[] =
{
    { 0x004A, 0x159FC704 },
    { 0x0067, 0x159BC704 },
    { 0x00C4, 0x159EC704 },
    { 0x00F4, 0x1593A704 },
    { 0x0136, 0x1595A704 },
    { 0x01A4, 0x1596A704 },
    { 0x022C, 0x159CE504 },
    { 0xFFFF, 0x1591E404 }
};

/*
 * Used by PLLElectrical() for r5xx+ and by rv620/35 code.
 */
static CARD32
PLLControlTableRetrieve(struct PLL_Control *Table, CARD16 FeedbackDivider)
{
    int i;

    for (i = 0; Table[i].FeedbackDivider < 0xFFFF ; i++)
	if (Table[i].FeedbackDivider >= FeedbackDivider)
	    break;

    return Table[i].Control;
}

/*
 * Not used by rv620/35 code.
 */
static CARD32
PLLElectrical(RHDPtr rhdPtr, CARD16 FeedbackDivider)
{
    switch (rhdPtr->ChipSet) {
    case RHD_RV515:
	if (rhdPtr->PciDeviceID == 0x7146)
	    return 0x00120704;
	else
	    return 0;
    case RHD_RV535:
	if (rhdPtr->PciDeviceID == 0x71C1)
	    return 0x00230704;
	else
	    return 0;
    case RHD_RS600:
    case RHD_RS690:
    case RHD_RS740:
	/* depending on MiscInfo also 0x00120004 */
	return 0x00120704;
    case RHD_R600:
	return 0x01130704;
    case RHD_RV610:
    case RHD_RV630:
    case RHD_M72:
    case RHD_M74:
    case RHD_M76:
	return PLLControlTableRetrieve(RV610PLLControl, FeedbackDivider);
    case RHD_RV670:
    case RHD_R680:
	return PLLControlTableRetrieve(RV670PLLControl, FeedbackDivider);
    default:
	return 0;
    }
}

/*
 * All R500s, RS6x0, R600, RV610 and RV630.
 */

/*
 *
 */
static void
PLL1Calibrate(struct rhdPLL *PLL)
{
    int i;

    RHDFUNC(PLL);

    RHDRegMask(PLL, P1PLL_CNTL, 1, 0x01); /* Reset */
    usleep(2);
    RHDRegMask(PLL, P1PLL_CNTL, 0, 0x01); /* Set */
    for (i = 0; i < PLL_CALIBRATE_WAIT; i++)
	if (((RHDRegRead(PLL, P1PLL_CNTL) >> 20) & 0x03) == 0x03)
	    break;

    if (i == PLL_CALIBRATE_WAIT) {
	if (RHDRegRead(PLL, P1PLL_CNTL) & 0x00100000) /* Calibration done? */
	    xf86DrvMsg(PLL->scrnIndex, X_ERROR,
		       "%s: Calibration failed.\n", __func__);
	if (RHDRegRead(PLL, P1PLL_CNTL) & 0x00200000) /* PLL locked? */
	    xf86DrvMsg(PLL->scrnIndex, X_ERROR,
		       "%s: Locking failed.\n", __func__);
    } else
	RHDDebug(PLL->scrnIndex, "%s: lock in %d loops\n", __func__, i);
}

/*
 *
 */
static void
PLL2Calibrate(struct rhdPLL *PLL)
{
    int i;

    RHDFUNC(PLL);

    RHDRegMask(PLL, P2PLL_CNTL, 1, 0x01); /* Reset */
    usleep(2);
    RHDRegMask(PLL, P2PLL_CNTL, 0, 0x01); /* Set */

    for (i = 0; i < PLL_CALIBRATE_WAIT; i++)
	if (((RHDRegRead(PLL, P2PLL_CNTL) >> 20) & 0x03) == 0x03)
	    break;

    if (i == PLL_CALIBRATE_WAIT) {
	if (RHDRegRead(PLL, P2PLL_CNTL) & 0x00100000) /* Calibration done? */
	    xf86DrvMsg(PLL->scrnIndex, X_ERROR,
		       "%s: Calibration failed.\n", __func__);
	if (RHDRegRead(PLL, P2PLL_CNTL) & 0x00200000) /* PLL locked? */
	    xf86DrvMsg(PLL->scrnIndex, X_ERROR,
		       "%s: Locking failed.\n", __func__);
    } else
	RHDDebug(PLL->scrnIndex, "%s: lock in %d loops\n", __func__, i);
}

/*
 *
 */
static void
R500PLL1Power(struct rhdPLL *PLL, int Power)
{
    RHDFUNC(PLL);

    switch (Power) {
    case RHD_POWER_ON:
	RHDRegMask(PLL, P1PLL_CNTL, 0, 0x02); /* Powah */
	usleep(2);

	PLL1Calibrate(PLL);

	return;
    case RHD_POWER_RESET:
	RHDRegMask(PLL, P1PLL_CNTL, 0x01, 0x01); /* Reset */
	usleep(2);

	RHDRegMask(PLL, P1PLL_CNTL, 0, 0x02); /* Powah */
	usleep(2);

	return;
    case RHD_POWER_SHUTDOWN:
    default:
	RHDRegMask(PLL, P1PLL_CNTL, 0x01, 0x01); /* Reset */
	usleep(2);

	RHDRegMask(PLL, P1PLL_CNTL, 0x02, 0x02); /* Power down */
	usleep(200);

	return;
    }
}

/*
 *
 */
static void
R500PLL2Power(struct rhdPLL *PLL, int Power)
{
    RHDFUNC(PLL);

    switch (Power) {
    case RHD_POWER_ON:
	RHDRegMask(PLL, P2PLL_CNTL, 0, 0x02); /* Powah */
	usleep(2);

	PLL2Calibrate(PLL);

	return;
    case RHD_POWER_RESET:
	RHDRegMask(PLL, P2PLL_CNTL, 0x01, 0x01); /* Reset */
	usleep(2);

	RHDRegMask(PLL, P2PLL_CNTL, 0, 0x02); /* Powah */
	usleep(2);

	return;
    case RHD_POWER_SHUTDOWN:
    default:
	RHDRegMask(PLL, P2PLL_CNTL, 0x01, 0x01); /* Reset */
	usleep(2);

	RHDRegMask(PLL, P2PLL_CNTL, 0x02, 0x02); /* Power down */
	usleep(200);

	return;
    }
}

/*
 *
 */
static void
R500PLL1SetLow(struct rhdPLL *PLL, CARD32 RefDiv, CARD32 FBDiv, CARD32 PostDiv,
	       CARD32 Control)
{
    RHDFUNC(PLL);
    RHDRegWrite(PLL, EXT1_PPLL_REF_DIV_SRC, 0x01); /* XTAL */
    RHDRegWrite(PLL, EXT1_PPLL_POST_DIV_SRC, 0x00); /* source = reference */

    RHDRegWrite(PLL, EXT1_PPLL_UPDATE_LOCK, 0x01); /* lock */

    RHDRegWrite(PLL, EXT1_PPLL_REF_DIV, RefDiv);
    RHDRegWrite(PLL, EXT1_PPLL_FB_DIV, FBDiv);
    RHDRegWrite(PLL, EXT1_PPLL_POST_DIV, PostDiv);
    RHDRegWrite(PLL, EXT1_PPLL_CNTL, Control);

    RHDRegMask(PLL, EXT1_PPLL_UPDATE_CNTL, 0x00010000, 0x00010000); /* no autoreset */
    RHDRegMask(PLL, P1PLL_CNTL, 0, 0x04); /* don't bypass calibration */

    /* We need to reset the anti glitch logic */
    RHDRegMask(PLL, P1PLL_CNTL, 0, 0x00000002); /* power up */

    /* reset anti glitch logic */
    RHDRegMask(PLL, P1PLL_CNTL, 0x00002000, 0x00002000);
    usleep(2);
    RHDRegMask(PLL, P1PLL_CNTL, 0, 0x00002000);

    /* powerdown and reset */
    RHDRegMask(PLL, P1PLL_CNTL, 0x00000003, 0x00000003);
    usleep(2);

    RHDRegWrite(PLL, EXT1_PPLL_UPDATE_LOCK, 0); /* unlock */
    RHDRegMask(PLL, EXT1_PPLL_UPDATE_CNTL, 0, 0x01); /* we're done updating! */

    RHDRegMask(PLL, P1PLL_CNTL, 0, 0x02); /* Powah */
    usleep(2);

    PLL1Calibrate(PLL);

    RHDRegWrite(PLL, EXT1_PPLL_POST_DIV_SRC, 0x01); /* source is PLL itself */
}

/*
 *
 */
static void
R500PLL2SetLow(struct rhdPLL *PLL, CARD32 RefDiv, CARD32 FBDiv, CARD32 PostDiv,
	       CARD32 Control)
{
    RHDRegWrite(PLL, EXT2_PPLL_REF_DIV_SRC, 0x01); /* XTAL */
    RHDRegWrite(PLL, EXT2_PPLL_POST_DIV_SRC, 0x00); /* source = reference */

    RHDRegWrite(PLL, EXT2_PPLL_UPDATE_LOCK, 0x01); /* lock */

    RHDRegWrite(PLL, EXT2_PPLL_REF_DIV, RefDiv);
    RHDRegWrite(PLL, EXT2_PPLL_FB_DIV, FBDiv);
    RHDRegWrite(PLL, EXT2_PPLL_POST_DIV, PostDiv);
    RHDRegWrite(PLL, EXT2_PPLL_CNTL, Control);

    RHDRegMask(PLL, EXT2_PPLL_UPDATE_CNTL, 0x00010000, 0x00010000); /* no autoreset */
    RHDRegMask(PLL, P2PLL_CNTL, 0, 0x04); /* don't bypass calibration */

    /* We need to reset the anti glitch logic */
    RHDRegMask(PLL, P2PLL_CNTL, 0, 0x00000002); /* power up */

    /* reset anti glitch logic */
    RHDRegMask(PLL, P2PLL_CNTL, 0x00002000, 0x00002000);
    usleep(2);
    RHDRegMask(PLL, P2PLL_CNTL, 0, 0x00002000);

    /* powerdown and reset */
    RHDRegMask(PLL, P2PLL_CNTL, 0x00000003, 0x00000003);
    usleep(2);

    RHDRegWrite(PLL, EXT2_PPLL_UPDATE_LOCK, 0); /* unlock */
    RHDRegMask(PLL, EXT2_PPLL_UPDATE_CNTL, 0, 0x01); /* we're done updating! */

    RHDRegMask(PLL, P2PLL_CNTL, 0, 0x02); /* Powah */
    usleep(2);

    PLL2Calibrate(PLL);

    RHDRegWrite(PLL, EXT2_PPLL_POST_DIV_SRC, 0x01); /* source is PLL itself */
}

/*
 * The CRTC ownership of each PLL is multiplexed on the PLL blocks, and the
 * ownership can only be switched when the currently referenced PLL is active.
 * This makes handling a slight bit more complex.
 */
static void
R500PLLCRTCGrab(struct rhdPLL *PLL, Bool Crtc2)
{
    CARD32 Stored;
    Bool PLL2IsCurrent;

    if (!Crtc2) {
	PLL2IsCurrent = RHDRegRead(PLL, PCLK_CRTC1_CNTL) & 0x00010000;

	if (PLL->Id == PLL_ID_PLL1)
	    RHDRegMask(PLL, PCLK_CRTC1_CNTL, 0, 0x00010000);
	else
	    RHDRegMask(PLL, PCLK_CRTC1_CNTL, 0x00010000, 0x00010000);
    } else {
	PLL2IsCurrent = RHDRegRead(PLL, PCLK_CRTC2_CNTL) & 0x00010000;

	if (PLL->Id == PLL_ID_PLL1)
	    RHDRegMask(PLL, PCLK_CRTC2_CNTL, 0, 0x00010000);
	else
	    RHDRegMask(PLL, PCLK_CRTC2_CNTL, 0x00010000, 0x00010000);
    }

    /* if the current pll is not active, then poke it just enough to flip
     * owners */
    if (!PLL2IsCurrent) {
	Stored = RHDRegRead(PLL, P1PLL_CNTL);

	if (Stored & 0x03) {
	    RHDRegMask(PLL, P1PLL_CNTL, 0, 0x03);
	    usleep(10);
	    RHDRegMask(PLL, P1PLL_CNTL, Stored, 0x03);
	}
    } else {
	Stored = RHDRegRead(PLL, P2PLL_CNTL);

	if (Stored & 0x03) {
	    RHDRegMask(PLL, P2PLL_CNTL, 0, 0x03);
	    usleep(10);
	    RHDRegMask(PLL, P2PLL_CNTL, Stored, 0x03);
	}
    }
}

/*
 *
 */
static void
R500PLL1Set(struct rhdPLL *PLL, int PixelClock, CARD16 ReferenceDivider,
	    CARD16 FeedbackDivider, CARD8 PostDivider)
{
    RHDPtr rhdPtr = RHDPTRI(PLL);
    CARD32 RefDiv, FBDiv, PostDiv, Control;

    RHDFUNC(PLL);

    RefDiv = ReferenceDivider;

    FBDiv = FeedbackDivider << 16;

    if (rhdPtr->ChipSet > RHD_R600) { /* set up Feedbackdivider slip */
	if (FeedbackDivider <= 0x24)
	    FBDiv |= 0x00000030;
	else if (FeedbackDivider <= 0x3F)
	    FBDiv |= 0x00000020;
    } else if (rhdPtr->ChipSet >= RHD_RS600) /* RS600, RS690, R600 */
	FBDiv |= 0x00000030;
    else
	FBDiv |= RHDRegRead(PLL, EXT1_PPLL_FB_DIV) & 0x00000030;

    PostDiv = RHDRegRead(PLL, EXT1_PPLL_POST_DIV) & ~0x0000007F;
    PostDiv |= PostDivider & 0x0000007F;

    Control = PLLElectrical(rhdPtr, FeedbackDivider);
    if (!Control)
	Control = RHDRegRead(PLL, EXT1_PPLL_CNTL);

    /* Disable Spread Spectrum */
    RHDRegMask(PLL, P1PLL_INT_SS_CNTL, 0, 0x00000001);

    R500PLL1SetLow(PLL, RefDiv, FBDiv, PostDiv, Control);

    if (rhdPtr->Crtc[0]->PLL == PLL)
	R500PLLCRTCGrab(PLL, FALSE);
    if (rhdPtr->Crtc[1]->PLL == PLL)
	R500PLLCRTCGrab(PLL, TRUE);
}

/*
 *
 */
static void
R500PLL2Set(struct rhdPLL *PLL, int PixelClock, CARD16 ReferenceDivider,
	    CARD16 FeedbackDivider, CARD8 PostDivider)
{
    RHDPtr rhdPtr = RHDPTRI(PLL);
    CARD32 RefDiv, FBDiv, PostDiv, Control;

    RHDFUNC(PLL);

    RefDiv = ReferenceDivider;

    FBDiv = FeedbackDivider << 16;

    if (rhdPtr->ChipSet > RHD_R600) { /* set up Feedbackdivider slip */
	if (FeedbackDivider <= 0x24)
	    FBDiv |= 0x00000030;
	else if (FeedbackDivider <= 0x3F)
	    FBDiv |= 0x00000020;
    } else if (rhdPtr->ChipSet >= RHD_RS600) /* RS600, RS690, R600 */
	FBDiv |= 0x00000030;
    else
	FBDiv |= RHDRegRead(PLL, EXT2_PPLL_FB_DIV) & 0x00000030;

    PostDiv = RHDRegRead(PLL, EXT2_PPLL_POST_DIV) & ~0x0000007F;
    PostDiv |= PostDivider & 0x0000007F;

    Control = PLLElectrical(rhdPtr, FeedbackDivider);
    if (!Control)
	Control = RHDRegRead(PLL, EXT2_PPLL_CNTL);

    /* Disable Spread Spectrum */
    RHDRegMask(PLL, P2PLL_INT_SS_CNTL, 0, 0x00000001);

    R500PLL2SetLow(PLL, RefDiv, FBDiv, PostDiv, Control);

    if (rhdPtr->Crtc[0]->PLL == PLL)
	R500PLLCRTCGrab(PLL, FALSE);
    if (rhdPtr->Crtc[1]->PLL == PLL)
	R500PLLCRTCGrab(PLL, TRUE);
}

/*
 *
 */
static void
R500PLL1Save(struct rhdPLL *PLL)
{
    RHDFUNC(PLL);

    PLL->StoreActive = !(RHDRegRead(PLL, P1PLL_CNTL) & 0x03);
    PLL->StoreRefDiv = RHDRegRead(PLL, EXT1_PPLL_REF_DIV);
    PLL->StoreFBDiv = RHDRegRead(PLL, EXT1_PPLL_FB_DIV);
    PLL->StorePostDiv = RHDRegRead(PLL, EXT1_PPLL_POST_DIV);
    PLL->StoreControl = RHDRegRead(PLL, EXT1_PPLL_CNTL);
    PLL->StoreSpreadSpectrum = RHDRegRead(PLL, P1PLL_INT_SS_CNTL);
    PLL->StoreCrtc1Owner = !(RHDRegRead(PLL, PCLK_CRTC1_CNTL) & 0x00010000);
    PLL->StoreCrtc2Owner = !(RHDRegRead(PLL, PCLK_CRTC2_CNTL) & 0x00010000);

    PLL->Stored = TRUE;
}

/*
 *
 */
static void
R500PLL2Save(struct rhdPLL *PLL)
{
    RHDFUNC(PLL);

    PLL->StoreActive = !(RHDRegRead(PLL, P2PLL_CNTL) & 0x03);
    PLL->StoreRefDiv = RHDRegRead(PLL, EXT2_PPLL_REF_DIV);
    PLL->StoreFBDiv = RHDRegRead(PLL, EXT2_PPLL_FB_DIV);
    PLL->StorePostDiv = RHDRegRead(PLL, EXT2_PPLL_POST_DIV);
    PLL->StoreControl = RHDRegRead(PLL, EXT2_PPLL_CNTL);
    PLL->StoreSpreadSpectrum = RHDRegRead(PLL, P2PLL_INT_SS_CNTL);
    PLL->StoreCrtc1Owner = RHDRegRead(PLL, PCLK_CRTC1_CNTL) & 0x00010000;
    PLL->StoreCrtc2Owner = RHDRegRead(PLL, PCLK_CRTC2_CNTL) & 0x00010000;

    PLL->Stored = TRUE;
}

/*
 *
 */
static void
R500PLL1Restore(struct rhdPLL *PLL)
{
    RHDFUNC(PLL);

    if (!PLL->Stored) {
	xf86DrvMsg(PLL->scrnIndex, X_ERROR, "%s: %s: trying to restore "
		   "uninitialized values.\n", __func__, PLL->Name);
	return;
    }

    if (PLL->StoreActive) {
	R500PLL1SetLow(PLL, PLL->StoreRefDiv, PLL->StoreFBDiv,
		       PLL->StorePostDiv, PLL->StoreControl);

	/* HotFix: always keep spread spectrum disabled on restore */
	if (0 && RHDPTRI(PLL)->ChipSet != RHD_M54)
	    RHDRegMask(PLL, P1PLL_INT_SS_CNTL,
		       PLL->StoreSpreadSpectrum, 0x00000001);
    } else {
	PLL->Power(PLL, RHD_POWER_SHUTDOWN);

	/* lame attempt at at least restoring the old values */
	RHDRegWrite(PLL, EXT1_PPLL_REF_DIV, PLL->StoreRefDiv);
	RHDRegWrite(PLL, EXT1_PPLL_FB_DIV, PLL->StoreFBDiv);
	RHDRegWrite(PLL, EXT1_PPLL_POST_DIV, PLL->StorePostDiv);
	RHDRegWrite(PLL, EXT1_PPLL_CNTL, PLL->StoreControl);
	RHDRegWrite(PLL, P1PLL_INT_SS_CNTL, PLL->StoreSpreadSpectrum);
    }

    if (PLL->StoreCrtc1Owner)
	R500PLLCRTCGrab(PLL, FALSE);
    if (PLL->StoreCrtc2Owner)
	R500PLLCRTCGrab(PLL, TRUE);
}

/*
 *
 */
static void
R500PLL2Restore(struct rhdPLL *PLL)
{
    RHDFUNC(PLL);

    if (!PLL->Stored) {
	xf86DrvMsg(PLL->scrnIndex, X_ERROR, "%s: %s: trying to restore "
		   "uninitialized values.\n", __func__, PLL->Name);
	return;
    }

    if (PLL->StoreActive) {
	R500PLL2SetLow(PLL, PLL->StoreRefDiv, PLL->StoreFBDiv,
		       PLL->StorePostDiv, PLL->StoreControl);

	if (RHDPTRI(PLL)->ChipSet != RHD_M54)
	    RHDRegMask(PLL, P2PLL_INT_SS_CNTL,
		       PLL->StoreSpreadSpectrum, 0x00000001);
    } else {
	PLL->Power(PLL, RHD_POWER_SHUTDOWN);

	/* lame attempt at at least restoring the old values */
	RHDRegWrite(PLL, EXT2_PPLL_REF_DIV, PLL->StoreRefDiv);
	RHDRegWrite(PLL, EXT2_PPLL_FB_DIV, PLL->StoreFBDiv);
	RHDRegWrite(PLL, EXT2_PPLL_POST_DIV, PLL->StorePostDiv);
	RHDRegWrite(PLL, EXT2_PPLL_CNTL, PLL->StoreControl);
	RHDRegWrite(PLL, P2PLL_INT_SS_CNTL, PLL->StoreSpreadSpectrum);
    }

    if (PLL->StoreCrtc1Owner)
	R500PLLCRTCGrab(PLL, FALSE);
    if (PLL->StoreCrtc2Owner)
	R500PLLCRTCGrab(PLL, TRUE);
}

/*
 * RV620 and up
 */

/*
 *
 */
#define RV620_DCCGCLK_RESET   0
#define RV620_DCCGCLK_GRAB    1
#define RV620_DCCGCLK_RELEASE 2

/*
 * I still have no idea what DCCG stands for and why it needs to hook off some
 * pixelclock...
 */
static void
RV620DCCGCLKSet(struct rhdPLL *PLL, int set)
{
    CARD32 tmp;

    RHDFUNC(PLL);

    switch(set) {
    case RV620_DCCGCLK_GRAB:
	if (PLL->Id == PLL_ID_PLL1)
	    RHDRegMask(PLL, DCCG_DISP_CLK_SRCSEL, 0, 0x00000003);
	else if (PLL->Id == PLL_ID_PLL2)
	    RHDRegMask(PLL, DCCG_DISP_CLK_SRCSEL, 1, 0x00000003);
	else
	    RHDRegMask(PLL, DCCG_DISP_CLK_SRCSEL, 3, 0x00000003);
	break;
    case RV620_DCCGCLK_RELEASE:
	tmp = RHDRegRead(PLL, DCCG_DISP_CLK_SRCSEL) & 0x03;

	if ((PLL->Id == PLL_ID_PLL1) && (tmp == 0)) {
	    /* set to other PLL or external */
	    tmp = RHDRegRead(PLL, P2PLL_CNTL);
	    if (!(tmp & 0x03) && /* powered and not in reset */
		((tmp & 0x00300000) == 0x00300000)) /* calibrated and locked */
		RHDRegMask(PLL, DCCG_DISP_CLK_SRCSEL, 1, 0x00000003);
	    else
		RHDRegMask(PLL, DCCG_DISP_CLK_SRCSEL, 3, 0x00000003);
	} else if ((PLL->Id == PLL_ID_PLL2) && (tmp == 1)) {
	    /* set to other PLL or external */
	    tmp = RHDRegRead(PLL, P1PLL_CNTL);
	    if (!(tmp & 0x03) && /* powered and not in reset */
		((tmp & 0x00300000) == 0x00300000)) /* calibrated and locked */
		RHDRegMask(PLL, DCCG_DISP_CLK_SRCSEL, 0, 0x00000003);
	    else
		RHDRegMask(PLL, DCCG_DISP_CLK_SRCSEL, 3, 0x00000003);

	} /* no other action needs to be taken */
	break;
    case RV620_DCCGCLK_RESET:
	tmp = RHDRegRead(PLL, DCCG_DISP_CLK_SRCSEL) & 0x03;

	if (((PLL->Id == PLL_ID_PLL1) && (tmp == 0)) ||
	    ((PLL->Id == PLL_ID_PLL2) && (tmp == 1)))
	    RHDRegMask(PLL, DCCG_DISP_CLK_SRCSEL, 3, 0x00000003);
	break;
    default:
	break;
    }
}

/*
 *
 */
static Bool
RV620DCCGCLKAvailable(struct rhdPLL *PLL)
{
    CARD32 Dccg = RHDRegRead(PLL, DCCG_DISP_CLK_SRCSEL) & 0x03;

    RHDFUNC(PLL);

    if (Dccg & 0x02)
	return TRUE;

    if ((PLL->Id == PLL_ID_PLL1) && (Dccg == 0))
	return TRUE;
    if ((PLL->Id == PLL_ID_PLL2) && (Dccg == 1))
	return TRUE;

    return FALSE;
}

/*
 *
 */
static void
RV620PLL1Power(struct rhdPLL *PLL, int Power)
{
    RHDFUNC(PLL);

    switch (Power) {
    case RHD_POWER_ON:
    {
	Bool HasDccg = RV620DCCGCLKAvailable(PLL);

	if (HasDccg)
	    RV620DCCGCLKSet(PLL, RV620_DCCGCLK_RESET);

	RHDRegMask(PLL, P1PLL_CNTL, 0, 0x02); /* Powah */
	usleep(2);

	PLL1Calibrate(PLL);

	if (HasDccg)
	    RV620DCCGCLKSet(PLL, RV620_DCCGCLK_GRAB);
	return;
    }
    case RHD_POWER_RESET:
	RV620DCCGCLKSet(PLL, RV620_DCCGCLK_RELEASE);

	RHDRegMask(PLL, P1PLL_CNTL, 0x01, 0x01); /* Reset */
	usleep(2);

	RHDRegMask(PLL, P1PLL_CNTL, 0, 0x02); /* Powah */
	usleep(2);

	return;
    case RHD_POWER_SHUTDOWN:
    default:
	RV620DCCGCLKSet(PLL, RV620_DCCGCLK_RELEASE);

	RHDRegMask(PLL, P1PLL_CNTL, 0x01, 0x01); /* Reset */
	usleep(2);

	RHDRegMask(PLL, P1PLL_CNTL, 0x02, 0x02); /* Power down */
	usleep(200);

	return;
    }
}

/*
 *
 */
static void
RV620PLL2Power(struct rhdPLL *PLL, int Power)
{
    RHDFUNC(PLL);

    switch (Power) {
    case RHD_POWER_ON:
    {
	Bool HasDccg = RV620DCCGCLKAvailable(PLL);

	if (HasDccg)
	    RV620DCCGCLKSet(PLL, RV620_DCCGCLK_RESET);

	RHDRegMask(PLL, P2PLL_CNTL, 0, 0x02); /* Powah */
	usleep(2);

	PLL2Calibrate(PLL);

	if (HasDccg)
	    RV620DCCGCLKSet(PLL, RV620_DCCGCLK_GRAB);
	return;
    }
    case RHD_POWER_RESET:
	RV620DCCGCLKSet(PLL, RV620_DCCGCLK_RELEASE);

	RHDRegMask(PLL, P2PLL_CNTL, 0x01, 0x01); /* Reset */
	usleep(2);

	RHDRegMask(PLL, P2PLL_CNTL, 0, 0x02); /* Powah */
	usleep(2);

	return;
    case RHD_POWER_SHUTDOWN:
    default:
	RV620DCCGCLKSet(PLL, RV620_DCCGCLK_RELEASE);

	RHDRegMask(PLL, P2PLL_CNTL, 0x01, 0x01); /* Reset */
	usleep(2);

	RHDRegMask(PLL, P2PLL_CNTL, 0x02, 0x02); /* Power down */
	usleep(200);

	return;
    }
}

/*
 *
 */
static void
RV620PLL1SetLow(struct rhdPLL *PLL, CARD32 RefDiv, CARD32 FBDiv, CARD32 PostDiv,
		CARD8 ScalerDiv, CARD8 SymPostDiv, CARD32 Control)
{
    RHDFUNC(PLL);

    /* switch to external */
    RHDRegWrite(PLL, EXT1_PPLL_POST_DIV_SRC, 0);
    RHDRegMask(PLL, P1PLL_DISP_CLK_CNTL, 0x00000200, 0x00000300);
    RHDRegMask(PLL, EXT1_SYM_PPLL_POST_DIV, 0, 0x00000100);

    RHDRegMask(PLL, P1PLL_CNTL, 0x00000001, 0x00000001); /* reset */
    usleep(2);
    RHDRegMask(PLL, P1PLL_CNTL, 0x00000002, 0x00000002); /* power down */
    usleep(10);
    RHDRegMask(PLL, P1PLL_CNTL, 0x00002000, 0x00002000); /* reset anti-glitch */

    RHDRegWrite(PLL, EXT1_PPLL_CNTL, Control);

    RHDRegMask(PLL, P1PLL_DISP_CLK_CNTL, ScalerDiv, 0x0000003F);

    RHDRegWrite(PLL, EXT1_PPLL_UPDATE_LOCK, 1); /* lock */

    RHDRegWrite(PLL, EXT1_PPLL_POST_DIV_SRC, 0x00000001);

    RHDRegWrite(PLL, EXT1_PPLL_REF_DIV, RefDiv);
    RHDRegWrite(PLL, EXT1_PPLL_FB_DIV, FBDiv);
    RHDRegMask(PLL, EXT1_PPLL_POST_DIV, PostDiv, 0x0000007F);
    RHDRegMask(PLL, EXT1_SYM_PPLL_POST_DIV, SymPostDiv, 0x0000007F);

    usleep(10);
    RHDRegWrite(PLL, EXT1_PPLL_UPDATE_LOCK, 0); /* unlock */

    RHDRegMask(PLL, P1PLL_CNTL, 0, 0x00000002); /* power up */
    usleep(10);
    RHDRegMask(PLL, P1PLL_CNTL, 0, 0x00002000); /* undo reset anti-glitch */

    PLL1Calibrate(PLL);

    /* switch back to the pll */
    RHDRegMask(PLL, P1PLL_DISP_CLK_CNTL, 0, 0x00000300);
    RHDRegMask(PLL, EXT1_SYM_PPLL_POST_DIV, 0x00000100, 0x00000100);
    RHDRegWrite(PLL, EXT1_PPLL_POST_DIV_SRC, 0x00000001);

    RHDRegMask(PLL, P1PLL_CNTL, 0, 0x80000000); /* new and undocumented */
}

/*
 *
 */
static void
RV620PLL2SetLow(struct rhdPLL *PLL, CARD32 RefDiv, CARD32 FBDiv, CARD32 PostDiv,
		CARD8 ScalerDiv, CARD8 SymPostDiv, CARD32 Control)
{
    RHDFUNC(PLL);

    /* switch to external */
    RHDRegWrite(PLL, EXT2_PPLL_POST_DIV_SRC, 0);
    RHDRegMask(PLL, P2PLL_DISP_CLK_CNTL, 0x00000200, 0x00000300);
    RHDRegMask(PLL, EXT2_SYM_PPLL_POST_DIV, 0, 0x00000100);

    RHDRegMask(PLL, P2PLL_CNTL, 0x00000001, 0x00000001); /* reset */
    usleep(2);
    RHDRegMask(PLL, P2PLL_CNTL, 0x00000002, 0x00000002); /* power down */
    usleep(10);
    RHDRegMask(PLL, P2PLL_CNTL, 0x00002000, 0x00002000); /* reset anti-glitch */

    RHDRegWrite(PLL, EXT2_PPLL_CNTL, Control);

    RHDRegMask(PLL, P2PLL_DISP_CLK_CNTL, ScalerDiv, 0x0000003F);

    RHDRegWrite(PLL, EXT2_PPLL_UPDATE_LOCK, 1); /* lock */

    RHDRegWrite(PLL, EXT2_PPLL_POST_DIV_SRC, 0x00000001);

    RHDRegWrite(PLL, EXT2_PPLL_REF_DIV, RefDiv);
    RHDRegWrite(PLL, EXT2_PPLL_FB_DIV, FBDiv);
    RHDRegMask(PLL, EXT2_PPLL_POST_DIV, PostDiv, 0x0000007F);
    RHDRegMask(PLL, EXT2_SYM_PPLL_POST_DIV, SymPostDiv, 0x0000007F);

    usleep(10);
    RHDRegWrite(PLL, EXT2_PPLL_UPDATE_LOCK, 0); /* unlock */

    RHDRegMask(PLL, P2PLL_CNTL, 0, 0x00000002); /* power up */
    usleep(10);
    RHDRegMask(PLL, P2PLL_CNTL, 0, 0x00002000); /* undo reset anti-glitch */

    PLL2Calibrate(PLL);

    /* switch back to the pll */
    RHDRegMask(PLL, P2PLL_DISP_CLK_CNTL, 0, 0x00000300);
    RHDRegMask(PLL, EXT2_SYM_PPLL_POST_DIV, 0x00000100, 0x00000100);
    RHDRegWrite(PLL, EXT2_PPLL_POST_DIV_SRC, 0x00000001);

    RHDRegMask(PLL, P2PLL_CNTL, 0, 0x80000000); /* new and undocumented */
}

/*
 *
 */
static void
RV620PLL1Set(struct rhdPLL *PLL, int PixelClock, CARD16 ReferenceDivider,
	     CARD16 FeedbackDivider, CARD8 PostDivider)
{
    RHDPtr rhdPtr = RHDPTRI(PLL);
    Bool HasDccg = RV620DCCGCLKAvailable(PLL);
    CARD32 RefDiv, FBDiv, PostDiv, Control;
    CARD8 ScalerDiv, SymPostDiv;

    RHDFUNC(PLL);

    if (HasDccg)
	RV620DCCGCLKSet(PLL, RV620_DCCGCLK_RESET);

    /* Disable Spread Spectrum */
    RHDRegMask(PLL, P1PLL_INT_SS_CNTL, 0, 0x00000001);

    RefDiv = ReferenceDivider;

    FBDiv = RHDRegRead(PLL, EXT1_PPLL_FB_DIV) & ~0x07FF003F;
    FBDiv |= ((FeedbackDivider << 16) | 0x0030) & 0x07FF003F;

    PostDiv = RHDRegRead(PLL, EXT1_PPLL_POST_DIV) & ~0x0000007F;
    PostDiv |= PostDivider & 0x0000007F;

    /* introduce flags for this, like on unichrome */
    ScalerDiv = 2; /* scaler post divider, 4 for UPDP */

    SymPostDiv = PostDivider & 0x0000007F;

    Control = PLLControlTableRetrieve(RV670PLLControl, FeedbackDivider);

    RV620PLL1SetLow(PLL, RefDiv, FBDiv, PostDiv, ScalerDiv, SymPostDiv,
		    Control);

    if (rhdPtr->Crtc[0]->PLL == PLL)
	R500PLLCRTCGrab(PLL, FALSE);
    if (rhdPtr->Crtc[1]->PLL == PLL)
	R500PLLCRTCGrab(PLL, TRUE);

    if (HasDccg)
	RV620DCCGCLKSet(PLL, RV620_DCCGCLK_GRAB);
}

/*
 *
 */
static void
RV620PLL2Set(struct rhdPLL *PLL, int PixelClock, CARD16 ReferenceDivider,
	     CARD16 FeedbackDivider, CARD8 PostDivider)
{
    RHDPtr rhdPtr = RHDPTRI(PLL);
    Bool HasDccg = RV620DCCGCLKAvailable(PLL);
    CARD32 RefDiv, FBDiv, PostDiv, Control;
    CARD8 ScalerDiv, SymPostDiv;

    RHDFUNC(PLL);

    if (HasDccg)
	RV620DCCGCLKSet(PLL, RV620_DCCGCLK_RESET);

    /* Disable Spread Spectrum */
    RHDRegMask(PLL, P2PLL_INT_SS_CNTL, 0, 0x00000001);

    RefDiv = ReferenceDivider;

    FBDiv = RHDRegRead(PLL, EXT2_PPLL_FB_DIV) & ~0x07FF003F;
    FBDiv |= ((FeedbackDivider << 16) | 0x0030) & 0x07FF003F;

    PostDiv = RHDRegRead(PLL, EXT2_PPLL_POST_DIV) & ~0x0000007F;
    PostDiv |= PostDivider & 0x0000007F;

    /* introduce flags for this, like on unichrome */
    ScalerDiv = 2; /* scaler post divider, 4 for UPDP */

    SymPostDiv = PostDivider & 0x0000007F;

    Control = PLLControlTableRetrieve(RV670PLLControl, FeedbackDivider);

    RV620PLL2SetLow(PLL, RefDiv, FBDiv, PostDiv, ScalerDiv, SymPostDiv,
		    Control);

    if (rhdPtr->Crtc[0]->PLL == PLL)
	R500PLLCRTCGrab(PLL, FALSE);
    if (rhdPtr->Crtc[1]->PLL == PLL)
	R500PLLCRTCGrab(PLL, TRUE);

    if (HasDccg)
	RV620DCCGCLKSet(PLL, RV620_DCCGCLK_GRAB);
}

/*
 *
 */
static void
RV620PLL1Save(struct rhdPLL *PLL)
{
    RHDFUNC(PLL);

    PLL->StoreActive = !(RHDRegRead(PLL, P1PLL_CNTL) & 0x03);
    PLL->StoreRefDiv = RHDRegRead(PLL, EXT1_PPLL_REF_DIV);
    PLL->StoreFBDiv = RHDRegRead(PLL, EXT1_PPLL_FB_DIV);
    PLL->StorePostDiv = RHDRegRead(PLL, EXT1_PPLL_POST_DIV);
    PLL->StorePostDivSrc = RHDRegRead(PLL, EXT1_PPLL_POST_DIV_SRC);
    PLL->StoreControl = RHDRegRead(PLL, EXT1_PPLL_CNTL);
    PLL->StoreSpreadSpectrum = RHDRegRead(PLL, P1PLL_INT_SS_CNTL);

    PLL->StoreGlitchReset = RHDRegRead(PLL, P1PLL_CNTL) & 0x00002000;

    PLL->StoreScalerPostDiv = RHDRegRead(PLL, P1PLL_DISP_CLK_CNTL) & 0x003F;
    PLL->StoreSymPostDiv = RHDRegRead(PLL, EXT1_SYM_PPLL_POST_DIV) & 0x007F;

    PLL->StoreCrtc1Owner = !(RHDRegRead(PLL, PCLK_CRTC1_CNTL) & 0x00010000);
    PLL->StoreCrtc2Owner = !(RHDRegRead(PLL, PCLK_CRTC2_CNTL) & 0x00010000);

    PLL->StoreDCCGCLKOwner = RV620DCCGCLKAvailable(PLL);
    if (PLL->StoreDCCGCLKOwner)
	PLL->StoreDCCGCLK = RHDRegRead(PLL, DCCG_DISP_CLK_SRCSEL);
    else
	PLL->StoreDCCGCLK = 0;

    PLL->Stored = TRUE;
}

/*
 *
 */
static void
RV620PLL2Save(struct rhdPLL *PLL)
{
    RHDFUNC(PLL);

    PLL->StoreActive = !(RHDRegRead(PLL, P2PLL_CNTL) & 0x03);
    PLL->StoreRefDiv = RHDRegRead(PLL, EXT2_PPLL_REF_DIV);
    PLL->StoreFBDiv = RHDRegRead(PLL, EXT2_PPLL_FB_DIV);
    PLL->StorePostDiv = RHDRegRead(PLL, EXT2_PPLL_POST_DIV);
    PLL->StorePostDivSrc = RHDRegRead(PLL, EXT2_PPLL_POST_DIV_SRC);
    PLL->StoreControl = RHDRegRead(PLL, EXT2_PPLL_CNTL);
    PLL->StoreSpreadSpectrum = RHDRegRead(PLL, P2PLL_INT_SS_CNTL);

    PLL->StoreGlitchReset = RHDRegRead(PLL, P2PLL_CNTL) & 0x00002000;

    PLL->StoreScalerPostDiv = RHDRegRead(PLL, P2PLL_DISP_CLK_CNTL) & 0x003F;
    PLL->StoreSymPostDiv = RHDRegRead(PLL, EXT2_SYM_PPLL_POST_DIV) & 0x007F;

    PLL->StoreCrtc1Owner = RHDRegRead(PLL, PCLK_CRTC1_CNTL) & 0x00010000;
    PLL->StoreCrtc2Owner = RHDRegRead(PLL, PCLK_CRTC2_CNTL) & 0x00010000;

    PLL->StoreDCCGCLKOwner = RV620DCCGCLKAvailable(PLL);
    if (PLL->StoreDCCGCLKOwner)
	PLL->StoreDCCGCLK = RHDRegRead(PLL, DCCG_DISP_CLK_SRCSEL);
    else
	PLL->StoreDCCGCLK = 0;

    PLL->Stored = TRUE;
}

/*
 * Notice how we handle the DCCG ownership here. There is a difference between
 * currently holding the DCCG and what was held when in the VT. With the
 * solution here we no longer hardlock, but we do have the danger of keeping
 * the DCCG in external mode for too long a time, if both PLL restores are
 * too far apart. This is currently not an issue as VT restoration goes over
 * the whole device in one go anyway; no partial restoration going on
 */
static void
RV620PLL1Restore(struct rhdPLL *PLL)
{
    RHDFUNC(PLL);

    if (RV620DCCGCLKAvailable(PLL))
	RHDRegMask(PLL, DCCG_DISP_CLK_SRCSEL, 0x03, 0x00000003);

    if (PLL->StoreActive) {
	RV620PLL1SetLow(PLL, PLL->StoreRefDiv, PLL->StoreFBDiv,
			PLL->StorePostDiv, PLL->StoreScalerPostDiv,
			PLL->StoreSymPostDiv, PLL->StoreControl);
	RHDRegMask(PLL, P1PLL_INT_SS_CNTL,
		   PLL->StoreSpreadSpectrum, 0x00000001);

	if (PLL->StoreDCCGCLKOwner)
	    RHDRegWrite(PLL, DCCG_DISP_CLK_SRCSEL, PLL->StoreDCCGCLK);

    } else {
	PLL->Power(PLL, RHD_POWER_SHUTDOWN);

	/* lame attempt at at least restoring the old values */
	RHDRegWrite(PLL, EXT1_PPLL_REF_DIV, PLL->StoreRefDiv);
	RHDRegWrite(PLL, EXT1_PPLL_FB_DIV, PLL->StoreFBDiv);
	RHDRegWrite(PLL, EXT1_PPLL_POST_DIV, PLL->StorePostDiv);
	RHDRegWrite(PLL, EXT1_PPLL_POST_DIV_SRC, PLL->StorePostDivSrc);
	RHDRegWrite(PLL, EXT1_PPLL_CNTL, PLL->StoreControl);
	RHDRegMask(PLL, P1PLL_DISP_CLK_CNTL, PLL->StoreScalerPostDiv, 0x003F);
	RHDRegMask(PLL, EXT1_SYM_PPLL_POST_DIV, PLL->StoreSymPostDiv, 0x007F);
	RHDRegWrite(PLL, P1PLL_INT_SS_CNTL, PLL->StoreSpreadSpectrum);

	if (PLL->StoreGlitchReset)
	    RHDRegMask(PLL, P1PLL_CNTL, 0x00002000, 0x00002000);
	else
	    RHDRegMask(PLL, P1PLL_CNTL, 0, 0x00002000);
    }

    if (PLL->StoreCrtc1Owner)
	R500PLLCRTCGrab(PLL, FALSE);
    if (PLL->StoreCrtc2Owner)
	R500PLLCRTCGrab(PLL, TRUE);

    if (PLL->StoreDCCGCLKOwner)
	RHDRegWrite(PLL, DCCG_DISP_CLK_SRCSEL, PLL->StoreDCCGCLK);
}

/*
 *
 */
static void
RV620PLL2Restore(struct rhdPLL *PLL)
{
    RHDFUNC(PLL);

    if (RV620DCCGCLKAvailable(PLL))
	RHDRegMask(PLL, DCCG_DISP_CLK_SRCSEL, 0x03, 0x00000003);

    if (PLL->StoreActive) {
	RV620PLL2SetLow(PLL, PLL->StoreRefDiv, PLL->StoreFBDiv,
			PLL->StorePostDiv, PLL->StoreScalerPostDiv,
			PLL->StoreSymPostDiv, PLL->StoreControl);
	RHDRegMask(PLL, P2PLL_INT_SS_CNTL,
		   PLL->StoreSpreadSpectrum, 0x00000001);
    } else {
	PLL->Power(PLL, RHD_POWER_SHUTDOWN);

	/* lame attempt at at least restoring the old values */
	RHDRegWrite(PLL, EXT2_PPLL_REF_DIV, PLL->StoreRefDiv);
	RHDRegWrite(PLL, EXT2_PPLL_FB_DIV, PLL->StoreFBDiv);
	RHDRegWrite(PLL, EXT2_PPLL_POST_DIV, PLL->StorePostDiv);
	RHDRegWrite(PLL, EXT2_PPLL_POST_DIV_SRC, PLL->StorePostDivSrc);
	RHDRegWrite(PLL, EXT2_PPLL_CNTL, PLL->StoreControl);
	RHDRegMask(PLL, P2PLL_DISP_CLK_CNTL, PLL->StoreScalerPostDiv, 0x003F);
	RHDRegMask(PLL, EXT2_SYM_PPLL_POST_DIV, PLL->StoreSymPostDiv, 0x007F);
	RHDRegWrite(PLL, P2PLL_INT_SS_CNTL, PLL->StoreSpreadSpectrum);

	if (PLL->StoreGlitchReset)
	    RHDRegMask(PLL, P2PLL_CNTL, 0x00002000, 0x00002000);
	else
	    RHDRegMask(PLL, P2PLL_CNTL, 0, 0x00002000);
    }

    if (PLL->StoreCrtc1Owner)
	R500PLLCRTCGrab(PLL, FALSE);
    if (PLL->StoreCrtc2Owner)
	R500PLLCRTCGrab(PLL, TRUE);

    if (PLL->StoreDCCGCLKOwner)
	RHDRegWrite(PLL, DCCG_DISP_CLK_SRCSEL, PLL->StoreDCCGCLK);
}

/* Some defaults for when we don't have this info */
/* XTAL is visible on the cards */
#define RHD_PLL_REFERENCE_DEFAULT            27000
/* these required quite some testing */
#define RHD_R500_PLL_INTERNAL_MIN_DEFAULT   648000
#define RHD_RV620_PLL_INTERNAL_MIN_DEFAULT  702000
/* Lowest value seen so far */
#define RHD_PLL_INTERNAL_MAX_DEFAULT       1100000
#define RHD_PLL_MIN_DEFAULT                  16000 /* guess */
#define RHD_PLL_MAX_DEFAULT                 400000 /* 400Mhz modes... hrm */

enum pllComp {
    PLL_NONE,
    PLL_MIN,
    PLL_MAX
};

/*
 *
 */
#ifdef ATOM_BIOS
static Bool
getPLLValuesFromAtomBIOS(RHDPtr rhdPtr,
			 AtomBiosRequestID func, char *msg, CARD32 *val, enum pllComp comp)
{
    AtomBiosArgRec arg;
    AtomBiosResult ret;

    if (rhdPtr->atomBIOS) {
	ret = RHDAtomBiosFunc(rhdPtr->scrnIndex, rhdPtr->atomBIOS,
			      func, &arg);
	if (ret == ATOM_SUCCESS) {
	    if (arg.val) {
		switch (comp) {
		    case PLL_MAX:
			if (arg.val < *val)
			    xf86DrvMsg(rhdPtr->scrnIndex, X_WARNING,
				       "Lower %s detected than the default: %lu %lu.\n"
				       "Please contact the authors ASAP.\n", msg,
				       (unsigned long)*val, (unsigned long)arg.val * 10);
			break;
		    case PLL_MIN:
			if (arg.val > *val)
			    xf86DrvMsg(rhdPtr->scrnIndex, X_WARNING,
				       "Higher %s detected than the default: %lu %lu.\n"
				       "Please contact the authors ASAP.\n", msg,
				       (unsigned long)*val, (unsigned long)arg.val * 10);
			break;
		    default:
			break;
		}
		*val = arg.val;
	    }
	}
	return TRUE;
    } else
	xf86DrvMsg(rhdPtr->scrnIndex, X_ERROR, "Failed to retrieve the %s"
		   " clock from ATOM.\n",msg);
    return FALSE;
}
#endif

/*
 *
 */
void
RHDSetupLimits(RHDPtr rhdPtr, CARD32 *RefClock,
	       CARD32 *IntMin, CARD32 *IntMax,
	       CARD32 *PixMin, CARD32 *PixMax)
{
    /* Retrieve the internal PLL frequency limits*/
    *RefClock = RHD_PLL_REFERENCE_DEFAULT;
    if (rhdPtr->ChipSet < RHD_RV620)
	*IntMin = RHD_R500_PLL_INTERNAL_MIN_DEFAULT;
    else
	*IntMin = RHD_RV620_PLL_INTERNAL_MIN_DEFAULT;

    *IntMax = RHD_PLL_INTERNAL_MAX_DEFAULT;

    /* keep the defaults */
    *PixMin = RHD_PLL_MIN_DEFAULT;
    *PixMax = RHD_PLL_MAX_DEFAULT;

#ifdef ATOM_BIOS
    getPLLValuesFromAtomBIOS(rhdPtr, GET_MIN_PIXEL_CLOCK_PLL_OUTPUT, "minimum PLL output",
			     IntMin,  PLL_MIN);
    getPLLValuesFromAtomBIOS(rhdPtr, GET_MAX_PIXEL_CLOCK_PLL_OUTPUT, "maximum PLL output",
			     IntMax, PLL_MAX);
    getPLLValuesFromAtomBIOS(rhdPtr, GET_MAX_PIXEL_CLK, "Pixel Clock",
			     PixMax, PLL_MAX);
    getPLLValuesFromAtomBIOS(rhdPtr, GET_REF_CLOCK, "reference clock",
			     RefClock, PLL_NONE);
    if (*IntMax == 0) {
	if (rhdPtr->ChipSet < RHD_RV620)
	    *IntMax = RHD_R500_PLL_INTERNAL_MIN_DEFAULT;
	else
	    *IntMax = RHD_RV620_PLL_INTERNAL_MIN_DEFAULT;

	xf86DrvMsg(rhdPtr->scrnIndex, X_WARNING, "AtomBIOS reports maximum VCO freq 0. "
		   "Using %lu instead\n",(unsigned long)*IntMax);
    }
#endif
}

/*
 *
 */
Bool
RHDPLLsInit(RHDPtr rhdPtr)
{
    struct rhdPLL *PLL;
    CARD32 RefClock, IntMin, IntMax, PixMin, PixMax;

    RHDFUNC(rhdPtr);

    if (RHDUseAtom(rhdPtr, NULL, atomUsagePLL))
	return FALSE;

    RHDSetupLimits(rhdPtr, &RefClock, &IntMin, &IntMax, &PixMin, &PixMax);

    /* PLL1 */
    PLL = (struct rhdPLL *) xnfcalloc(sizeof(struct rhdPLL), 1);

    PLL->scrnIndex = rhdPtr->scrnIndex;
    PLL->Name = PLL_NAME_PLL1;
    PLL->Id = PLL_ID_PLL1;

    PLL->RefClock = RefClock;
    PLL->IntMin = IntMin;
    PLL->IntMax = IntMax;
    PLL->PixMin = PixMin;
    PLL->PixMax = PixMax;

    PLL->Valid = NULL;
    if (rhdPtr->ChipSet < RHD_RV620) {
	PLL->Set = R500PLL1Set;
	PLL->Power = R500PLL1Power;
	PLL->Save = R500PLL1Save;
	PLL->Restore = R500PLL1Restore;
    } else {
	PLL->Set = RV620PLL1Set;
	PLL->Power = RV620PLL1Power;
	PLL->Save = RV620PLL1Save;
	PLL->Restore = RV620PLL1Restore;
    }

    rhdPtr->PLLs[0] = PLL;

    /* PLL2 */
    PLL = (struct rhdPLL *) xnfcalloc(sizeof(struct rhdPLL), 1);

    PLL->scrnIndex = rhdPtr->scrnIndex;
    PLL->Name = PLL_NAME_PLL2;
    PLL->Id = PLL_ID_PLL2;

    PLL->RefClock = RefClock;
    PLL->IntMin = IntMin;
    PLL->IntMax = IntMax;
    PLL->PixMin = PixMin;
    PLL->PixMax = PixMax;

    PLL->Valid = NULL;
    if (rhdPtr->ChipSet < RHD_RV620) {
	PLL->Set = R500PLL2Set;
	PLL->Power = R500PLL2Power;
	PLL->Save = R500PLL2Save;
	PLL->Restore = R500PLL2Restore;
    } else {
	PLL->Set = RV620PLL2Set;
	PLL->Power = RV620PLL2Power;
	PLL->Save = RV620PLL2Save;
	PLL->Restore = RV620PLL2Restore;
    }

    rhdPtr->PLLs[1] = PLL;

    return TRUE;
}

/*
 *
 */
ModeStatus
RHDPLLValid(struct rhdPLL *PLL, CARD32 Clock)
{
    RHDFUNC(PLL);

    if (Clock < PLL->PixMin)
	return MODE_CLOCK_LOW;
    if (Clock > PLL->PixMax)
	return MODE_CLOCK_HIGH;

    if (PLL->Valid)
	return PLL->Valid(PLL, Clock);
    else
	return MODE_OK;
}


/*
 * Calculate the PLL parameters for a given dotclock.
 *
 * This calculation uses a linear approximation of an experimentally found
 * curve that delimits reference versus feedback dividers on rv610. This curve
 * can be shifted towards higher feedback divider through increasing the gain
 * control, but the effect of this is rather limited.
 *
 * Since this upper limit still provides a wide enough range with enough
 * granularity, we use it for all r5xx and r6xx devices.
 */
static Bool
PLLCalculate(struct rhdPLL *PLL, CARD32 PixelClock,
	     CARD16 *RefDivider, CARD16 *FBDivider, CARD8 *PostDivider)
{
/* limited by the number of bits available */
#define FB_DIV_LIMIT 2048
#define REF_DIV_LIMIT 1024
#define POST_DIV_LIMIT 128

    CARD32 FBDiv, RefDiv, PostDiv, BestDiff = 0xFFFFFFFF;
    float Ratio;

    Ratio = ((float) PixelClock) / ((float) PLL->RefClock);

    for (PostDiv = 2; PostDiv < POST_DIV_LIMIT; PostDiv++) {
	CARD32 VCOOut = PixelClock * PostDiv;

	/* we are conservative and avoid the limits */
	if (VCOOut <= PLL->IntMin)
	    continue;
	if (VCOOut >= PLL->IntMax)
	    break;

        for (RefDiv = 1; RefDiv <= REF_DIV_LIMIT; RefDiv++) {
	    CARD32 Diff;

	    FBDiv = (CARD32) ((Ratio * PostDiv * RefDiv) + 0.5);

	    if (FBDiv >= FB_DIV_LIMIT)
		break;
	    if (FBDiv > (500 + (13 * RefDiv))) /* rv6x0 limit */
		break;

	    Diff = abs( PixelClock - (FBDiv * PLL->RefClock) / (PostDiv * RefDiv) );

	    if (Diff < BestDiff) {
		*FBDivider = FBDiv;
		*RefDivider = RefDiv;
		*PostDivider = PostDiv;
		BestDiff = Diff;
	    }

	    if (BestDiff == 0)
		break;
	}
	if (BestDiff == 0)
	    break;
    }

    if (BestDiff != 0xFFFFFFFF) {
	RHDDebug(PLL->scrnIndex, "PLL Calculation: %dkHz = "
		   "(((%i / 0x%X) * 0x%X) / 0x%X) (%dkHz off)\n",
		   (int) PixelClock, (unsigned int) PLL->RefClock, *RefDivider,
		   *FBDivider, *PostDivider, (int) BestDiff);
	return TRUE;
    } else { /* Should never happen */
	xf86DrvMsg(PLL->scrnIndex, X_ERROR,
		   "%s: Failed to get a valid PLL setting for %dkHz\n",
		   __func__, (int) PixelClock);
	return FALSE;
    }
}

/*
 *
 */
void
RHDPLLSet(struct rhdPLL *PLL, CARD32 Clock)
{
    CARD16 RefDivider = 0, FBDivider = 0;
    CARD8 PostDivider = 0;

    RHDDebug(PLL->scrnIndex, "%s: Setting %s to %dkHz\n", __func__,
	     PLL->Name, Clock);

    if (PLLCalculate(PLL, Clock, &RefDivider, &FBDivider, &PostDivider)) {
	PLL->Set(PLL, Clock, RefDivider, FBDivider, PostDivider);

	PLL->CurrentClock = Clock;
	PLL->Active = TRUE;
    } else
	xf86DrvMsg(PLL->scrnIndex, X_WARNING,
		   "%s: Not altering any settings.\n", __func__);
}

/*
 *
 */
void
RHDPLLPower(struct rhdPLL *PLL, int Power)
{
    RHDFUNC(PLL);

    if (PLL->Power)
	PLL->Power(PLL, Power);
}

/*
 *
 */
void
RHDPLLsPowerAll(RHDPtr rhdPtr, int Power)
{
    struct rhdPLL *PLL;

    RHDFUNC(rhdPtr);

    PLL = rhdPtr->PLLs[0];
    if (PLL->Power)
	PLL->Power(PLL, Power);

    PLL = rhdPtr->PLLs[1];
    if (PLL->Power)
	PLL->Power(PLL, Power);
}

/*
 *
 */
void
RHDPLLsShutdownInactive(RHDPtr rhdPtr)
{
    struct rhdPLL *PLL;

    RHDFUNC(rhdPtr);

    PLL = rhdPtr->PLLs[0];
    if (PLL->Power && !PLL->Active)
	PLL->Power(PLL, RHD_POWER_SHUTDOWN);

    PLL = rhdPtr->PLLs[1];
    if (PLL->Power && !PLL->Active)
	PLL->Power(PLL, RHD_POWER_SHUTDOWN);
}

/*
 *
 */
void
RHDPLLsSave(RHDPtr rhdPtr)
{
    struct rhdPLL *PLL;

    RHDFUNC(rhdPtr);

    PLL = rhdPtr->PLLs[0];
    if (PLL->Save)
	PLL->Save(PLL);

    PLL = rhdPtr->PLLs[1];
    if (PLL->Save)
	PLL->Save(PLL);
}

/*
 *
 */
void
RHDPLLsRestore(RHDPtr rhdPtr)
{
    struct rhdPLL *PLL;

    RHDFUNC(rhdPtr);

    PLL = rhdPtr->PLLs[0];
    if (PLL->Restore)
	PLL->Restore(PLL);

    PLL = rhdPtr->PLLs[1];
    if (PLL->Restore)
	PLL->Restore(PLL);
}

/*
 *
 */
void
RHDPLLsDestroy(RHDPtr rhdPtr)
{
    RHDFUNC(rhdPtr);

    if (rhdPtr->PLLs[0] && rhdPtr->PLLs[0]->Private)
	xfree(rhdPtr->PLLs[0]->Private);
    xfree(rhdPtr->PLLs[0]);
    if (rhdPtr->PLLs[1] && rhdPtr->PLLs[1]->Private)
	xfree(rhdPtr->PLLs[1]->Private);
    xfree(rhdPtr->PLLs[1]);
}
