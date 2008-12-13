/*
 * Copyright 2007-2008  Luc Verhaegen <lverhaegen@novell.com>
 * Copyright 2007-2008  Matthias Hopf <mhopf@novell.com>
 * Copyright 2007-2008  Egbert Eich   <eich@novell.com>
 * Copyright 2007-2008  Advanced Micro Devices, Inc.
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

/*
 * Deals with the Shared LVDS/TMDS encoder.
 *
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

#ifdef ATOM_BIOS
#include "rhd_atombios.h"
#include "rhd_atomout.h"
#endif

#include "rhd_connector.h"
#include "rhd_output.h"
#include "rhd_regs.h"
#include "rhd_card.h"

/*
 * First of all, make it more managable to code for both R500 and R600, as
 * there was a 1 register shift, right in the middle of the register block.
 * There are of course much nicer ways to do the workaround i am doing here,
 * but speed is not an issue here.
 */
static inline CARD16
LVTMAChipGenerationSelect(int ChipSet, CARD32 R500, CARD32 R600)
{
    if (ChipSet >= RHD_RS600)
	return R600;
    else
	return R500;
}

#define LVTMAGENSEL(r500, r600) LVTMAChipGenerationSelect(rhdPtr->ChipSet, (r500), (r600))
#define LVTMA_DATA_SYNCHRONIZATION \
    LVTMAGENSEL(LVTMA_R500_DATA_SYNCHRONIZATION, LVTMA_R600_DATA_SYNCHRONIZATION)
#define LVTMA_PWRSEQ_REF_DIV \
    LVTMAGENSEL(LVTMA_R500_PWRSEQ_REF_DIV, LVTMA_R600_PWRSEQ_REF_DIV)
#define LVTMA_PWRSEQ_DELAY1 \
    LVTMAGENSEL(LVTMA_R500_PWRSEQ_DELAY1, LVTMA_R600_PWRSEQ_DELAY1)
#define LVTMA_PWRSEQ_DELAY2 \
    LVTMAGENSEL(LVTMA_R500_PWRSEQ_DELAY2, LVTMA_R600_PWRSEQ_DELAY2)
#define LVTMA_PWRSEQ_CNTL \
    LVTMAGENSEL(LVTMA_R500_PWRSEQ_CNTL, LVTMA_R600_PWRSEQ_CNTL)
#define LVTMA_PWRSEQ_STATE \
    LVTMAGENSEL(LVTMA_R500_PWRSEQ_STATE, LVTMA_R600_PWRSEQ_STATE)
#define LVTMA_LVDS_DATA_CNTL \
    LVTMAGENSEL(LVTMA_R500_LVDS_DATA_CNTL, LVTMA_R600_LVDS_DATA_CNTL)
#define LVTMA_MODE LVTMAGENSEL(LVTMA_R500_MODE, LVTMA_R600_MODE)
#define LVTMA_TRANSMITTER_ENABLE \
    LVTMAGENSEL(LVTMA_R500_TRANSMITTER_ENABLE, LVTMA_R600_TRANSMITTER_ENABLE)
#define LVTMA_MACRO_CONTROL \
    LVTMAGENSEL(LVTMA_R500_MACRO_CONTROL, LVTMA_R600_MACRO_CONTROL)
#define LVTMA_TRANSMITTER_CONTROL \
    LVTMAGENSEL(LVTMA_R500_TRANSMITTER_CONTROL, LVTMA_R600_TRANSMITTER_CONTROL)
#define LVTMA_REG_TEST_OUTPUT \
    LVTMAGENSEL(LVTMA_R500_REG_TEST_OUTPUT, LVTMA_R600_REG_TEST_OUTPUT)
#define LVTMA_BL_MOD_CNTL \
    LVTMAGENSEL(LVTMA_R500_BL_MOD_CNTL, LVTMA_R600_BL_MOD_CNTL)

#define LVTMA_DITHER_RESET_BIT LVTMAGENSEL(0x04000000, 0x02000000)

/*
 *
 * Handling for LVTMA block as LVDS.
 *
 */

struct LVDSPrivate {
    Bool DualLink;
    Bool LVDS24Bit;
    Bool FPDI; /* LDI otherwise */
    CARD16 TXClockPattern;
    int BlLevel;
    CARD32 MacroControl;

    /* Power timing for LVDS */
    CARD16 PowerRefDiv;
    CARD16 BlonRefDiv;
    CARD16 PowerDigToDE;
    CARD16 PowerDEToBL;
    CARD16 OffDelay;
    Bool   TemporalDither;
    Bool   SpatialDither;
    int    GreyLevel;

    Bool Stored;

    CARD32 StoreControl;
    CARD32 StoreSourceSelect;
    CARD32 StoreBitDepthControl;
    CARD32 StoreDataSynchronisation;
    CARD32 StorePWRSEQRefDiv;
    CARD32 StorePWRSEQDelay1;
    CARD32 StorePWRSEQDelay2;
    CARD32 StorePWRSEQControl;
    CARD32 StorePWRSEQState;
    CARD32 StoreLVDSDataControl;
    CARD32 StoreMode;
    CARD32 StoreTxEnable;
    CARD32 StoreMacroControl;
    CARD32 StoreTXControl;
    CARD32 StoreBlModCntl;
#ifdef NOT_YET
    /* to hook in AtomBIOS property callback */
    Bool (*WrappedPropertyCallback) (struct rhdOutput *Output,
		      enum rhdPropertyAction Action, enum rhdOutputProperty Property, union rhdPropertyData *val);
    void *PropertyPrivate;
#endif
};

/*
 *
 */
static ModeStatus
LVDSModeValid(struct rhdOutput *Output, DisplayModePtr Mode)
{
    RHDFUNC(Output);

    if (Mode->Flags & V_INTERLACE)
        return MODE_NO_INTERLACE;

    return MODE_OK;
}

/*
 *
 */
static void
LVDSDebugBacklight(struct rhdOutput *Output)
{
    RHDPtr rhdPtr = RHDPTRI(Output);
    CARD32 tmp;
    Bool Blon, BlonOvrd, BlonPol, BlModEn;
    int BlModLevel, BlModRes = 0;

    if (rhdPtr->verbosity < 7)
	return;

    tmp = (RHDRegRead(Output, LVTMA_PWRSEQ_STATE) >> 3) & 0x01;
    RHDDebug(rhdPtr->scrnIndex, "%s: PWRSEQ BLON State: %s\n",
	    __func__, tmp ? "on" : "off");
    tmp = RHDRegRead(rhdPtr, LVTMA_PWRSEQ_CNTL);
    Blon = (tmp >> 24) & 0x1;
    BlonOvrd = (tmp >> 25) & 0x1;
    BlonPol = (tmp >> 26) & 0x1;

    RHDDebug(rhdPtr->scrnIndex, "%s: BLON: %s BLON_OVRD: %s BLON_POL: %s\n",
	    __func__, Blon ? "on" : "off",
	    BlonOvrd ? "enabled" : "disabled",
	    BlonPol ? "invert" : "non-invert");

    tmp = RHDRegRead(rhdPtr, LVTMA_BL_MOD_CNTL);
    BlModEn = tmp & 0x1;
    BlModLevel = (tmp >> 8) & 0xFF;
    if (rhdPtr->ChipSet >= RHD_RS600)
	BlModRes = (tmp >> 16) & 0xFF;

    xf86DrvMsgVerb(rhdPtr->scrnIndex, X_INFO, 3,
		   "%s: BL_MOD: %s BL_MOD_LEVEL: %d BL_MOD_RES: %d\n",
		   __func__, BlModEn ? "enable" : "disable",
	    BlModLevel, BlModRes);
}

/*
 *
 */
static void
LVDSSetBacklight(struct rhdOutput *Output, int level)
{
    struct LVDSPrivate *Private = (struct LVDSPrivate *) Output->Private;
    RHDPtr rhdPtr = RHDPTRI(Output);

    Private->BlLevel = level;

    xf86DrvMsg(rhdPtr->scrnIndex, X_INFO,
	       "%s: trying to set BL_MOD_LEVEL to: %d\n",
	       __func__, level);

    if (rhdPtr->ChipSet >= RHD_RS600)
  _RHDRegMask(rhdPtr, LVTMA_BL_MOD_CNTL,
		   0xFF << 16 | (level << 8) | 0x1,
		   0xFFFF01);
    else
  _RHDRegMask(rhdPtr, LVTMA_BL_MOD_CNTL,
		   (level << 8) | 0x1,
		   0xFF01);

    /*
     * Poor man's debug
     */
    LVDSDebugBacklight(Output);
}

/*
 *
 */
static Bool
LVDSPropertyControl(struct rhdOutput *Output, enum rhdPropertyAction Action,
		    enum rhdOutputProperty Property, union rhdPropertyData *val)
{
    struct LVDSPrivate *Private = (struct LVDSPrivate *) Output->Private;

    switch (Action) {
	case rhdPropertyCheck:
	    switch (Property) {
		if (Private->BlLevel < 0)
		    return FALSE;
		case RHD_OUTPUT_BACKLIGHT:
		    return TRUE;
		default:
		    return FALSE;
	    }
	case rhdPropertyGet:
	    switch (Property) {
		case RHD_OUTPUT_BACKLIGHT:
		    if (Private->BlLevel < 0)
			return FALSE;
		    val->integer = Private->BlLevel;
		    break;
		default:
		    return FALSE;
	    }
	    break;
	case rhdPropertySet:
	    switch (Property) {
		case RHD_OUTPUT_BACKLIGHT:
		    if (Private->BlLevel < 0)
			return FALSE;
		    LVDSSetBacklight(Output, val->integer);
		    break;
		default:
		    return FALSE;
	    }
	    break;
    }
    return TRUE;
}

/*
 *
 */
static void
LVDSSet(struct rhdOutput *Output, DisplayModePtr Mode)
{
    struct LVDSPrivate *Private = (struct LVDSPrivate *) Output->Private;
    RHDPtr rhdPtr = RHDPTRI(Output);

    RHDFUNC(Output);

    RHDRegMask(Output, LVTMA_CNTL, 0x00000001, 0x00000001); /* enable */
    usleep(20);

    RHDRegWrite(Output, LVTMA_MODE, 0); /* set to LVDS */

    /* Select CRTC, select syncA, no stereosync */
    RHDRegMask(Output, LVTMA_SOURCE_SELECT, Output->Crtc->Id, 0x00010101);

    if (Private->LVDS24Bit) { /* 24bits */
	RHDRegMask(Output, LVTMA_LVDS_DATA_CNTL, 0x00000001, 0x00000001); /* enable 24bits */
	RHDRegMask(Output, LVTMA_BIT_DEPTH_CONTROL, 0x00101010, 0x00101010); /* dithering bit depth = 24 */

	if (Private->FPDI) /* FPDI? */
	    RHDRegMask(Output, LVTMA_LVDS_DATA_CNTL, 0x00000010, 0x00000010); /* 24 bit format: FPDI or LDI? */
	else
	    RHDRegMask(Output, LVTMA_LVDS_DATA_CNTL, 0, 0x00000010);
    } else {
	RHDRegMask(Output, LVTMA_LVDS_DATA_CNTL, 0, 0x00000001); /* disable 24bits */
	RHDRegMask(Output, LVTMA_BIT_DEPTH_CONTROL, 0, 0x00101010); /* dithering bit depth != 24 */
    }

    /* enable temporal dithering, disable spatial dithering and disable truncation */
    RHDRegMask(Output, LVTMA_BIT_DEPTH_CONTROL,
	       Private->TemporalDither ? 1 << 16 : 0
	       | Private->SpatialDither ? 1 << 8 : 0
	       | (Private->GreyLevel > 2) ? 1 << 24 : 0,
	       0x01010101);

    /* reset the temporal dithering */
    RHDRegMask(Output, LVTMA_BIT_DEPTH_CONTROL, LVTMA_DITHER_RESET_BIT, LVTMA_DITHER_RESET_BIT);
    RHDRegMask(Output, LVTMA_BIT_DEPTH_CONTROL, 0, LVTMA_DITHER_RESET_BIT);

    /* go for RGB 4:4:4 RGB/YCbCr  */
    RHDRegMask(Output, LVTMA_CNTL, 0, 0x00010000);

    if (Private->DualLink)
	RHDRegMask(Output, LVTMA_CNTL, 0x01000000, 0x01000000);
    else
	RHDRegMask(Output, LVTMA_CNTL, 0, 0x01000000);

    /* PLL and TX voltages */
    RHDRegWrite(Output, LVTMA_MACRO_CONTROL, Private->MacroControl);

    RHDRegMask(Output, LVTMA_TRANSMITTER_CONTROL, 0x00000010, 0x00000010); /* use pclk_lvtma_direct */
    RHDRegMask(Output, LVTMA_TRANSMITTER_CONTROL, 0, 0xCC000000);
    RHDRegMask(Output, LVTMA_TRANSMITTER_CONTROL, Private->TXClockPattern << 16, 0x03FF0000);
    RHDRegMask(Output, LVTMA_TRANSMITTER_CONTROL, 0x00000001, 0x00000001); /* enable PLL */
    usleep(20);

    /* reset transmitter */
    RHDRegMask(Output, LVTMA_TRANSMITTER_CONTROL, 0x00000002, 0x00000002);
    usleep(2);
    RHDRegMask(Output, LVTMA_TRANSMITTER_CONTROL, 0, 0x00000002);
    usleep(20);

    /* start data synchronisation */
    RHDRegMask(Output, LVTMA_DATA_SYNCHRONIZATION, 0x00000001, 0x00000001);
    RHDRegMask(Output, LVTMA_DATA_SYNCHRONIZATION, 0x00000100, 0x00000100); /* reset */
    usleep(2);
    RHDRegMask(Output, LVTMA_DATA_SYNCHRONIZATION, 0, 0x00000100);
}

/*
 *
 */
static void
LVDSPWRSEQInit(struct rhdOutput *Output)
{
    struct LVDSPrivate *Private = (struct LVDSPrivate *) Output->Private;
    RHDPtr rhdPtr = RHDPTRI(Output);

    CARD32 tmp = 0;

    tmp = Private->PowerDigToDE >> 2;
    RHDRegMask(Output, LVTMA_PWRSEQ_DELAY1, tmp, 0x000000FF);
    RHDRegMask(Output, LVTMA_PWRSEQ_DELAY1, tmp << 24, 0xFF000000);

    tmp = Private->PowerDEToBL >> 2;
    RHDRegMask(Output, LVTMA_PWRSEQ_DELAY1, tmp << 8, 0x0000FF00);
    RHDRegMask(Output, LVTMA_PWRSEQ_DELAY1, tmp << 16, 0x00FF0000);

    RHDRegWrite(Output, LVTMA_PWRSEQ_DELAY2, Private->OffDelay >> 2);
    RHDRegWrite(Output, LVTMA_PWRSEQ_REF_DIV,
		Private->PowerRefDiv | (Private->BlonRefDiv << 16));

    /* Enable power sequencer and allow it to override everything */
    RHDRegMask(Output, LVTMA_PWRSEQ_CNTL, 0x0000000D, 0x0000000D);

    /* give full control to the sequencer */
    RHDRegMask(Output, LVTMA_PWRSEQ_CNTL, 0, 0x02020200);
}

/*
 *
 */
static void
LVDSEnable(struct rhdOutput *Output)
{
    struct LVDSPrivate *Private = (struct LVDSPrivate *) Output->Private;
    RHDPtr rhdPtr = RHDPTRI(Output);
    CARD32 tmp = 0;
    int i;

    RHDFUNC(Output);

    LVDSPWRSEQInit(Output);

    /* set up the transmitter */
    RHDRegMask(Output, LVTMA_TRANSMITTER_ENABLE, 0x0000001E, 0x0000001E);
    if (Private->LVDS24Bit) /* 24bit ? */
	RHDRegMask(Output, LVTMA_TRANSMITTER_ENABLE, 0x00000020, 0x00000020);

    if (Private->DualLink) {
	RHDRegMask(Output, LVTMA_TRANSMITTER_ENABLE, 0x00001E00, 0x00001E00);

	if (Private->LVDS24Bit)
	    RHDRegMask(Output, LVTMA_TRANSMITTER_ENABLE, 0x00002000, 0x00002000);
    }

    RHDRegMask(Output, LVTMA_PWRSEQ_CNTL, 0x00000010, 0x00000010);

    for (i = 0; i <= Private->OffDelay; i++) {
	usleep(1000);

	tmp = (RHDRegRead(Output, LVTMA_PWRSEQ_STATE) >> 8) & 0x0F;
	if (tmp == 4)
	    break;
    }

    if (i == Private->OffDelay) {
	xf86DrvMsg(Output->scrnIndex, X_ERROR, "%s: failed to reach "
             "POWERUP_DONE state after %d loops (%d)\n",
             __func__, i, (int) tmp);
  }
    if (Private->BlLevel >= 0) {
	union rhdPropertyData data;
	data.integer = Private->BlLevel;
	Output->Property(Output, rhdPropertySet, RHD_OUTPUT_BACKLIGHT,
			 &data);
    }
}

/*
 *
 */
static void
LVDSDisable(struct rhdOutput *Output)
{
    struct LVDSPrivate *Private = (struct LVDSPrivate *) Output->Private;
    RHDPtr rhdPtr = RHDPTRI(Output);
    CARD32 tmp = 0;
    int i;

    RHDFUNC(Output);

    if (!(RHDRegRead(Output, LVTMA_PWRSEQ_CNTL) & 0x00000010))
	return;

    LVDSPWRSEQInit(Output);

    RHDRegMask(Output, LVTMA_PWRSEQ_CNTL, 0, 0x00000010);

    for (i = 0; i <= Private->OffDelay; i++) {
	usleep(1000);

	tmp = (RHDRegRead(Output, LVTMA_PWRSEQ_STATE) >> 8) & 0x0F;
	if (tmp == 9)
	    break;
    }

    if (i == Private->OffDelay) {
	xf86DrvMsg(Output->scrnIndex, X_ERROR, "%s: failed to reach "
		   "POWERDOWN_DONE state after %d loops (%d)\n",
		   __func__, i, (int) tmp);
    }

    RHDRegMask(Output, LVTMA_TRANSMITTER_ENABLE, 0, 0x0000FFFF);
}

#if 0
/*
 *
 */
static void
LVDSShutdown(struct rhdOutput *Output)
{
    RHDPtr rhdPtr = RHDPTRI(Output);
    RHDFUNC(Output);

    RHDRegMask(Output, LVTMA_TRANSMITTER_CONTROL, 0x00000002, 0x00000002); /* PLL in reset */
    RHDRegMask(Output, LVTMA_TRANSMITTER_CONTROL, 0, 0x00000001); /* disable LVDS */
    RHDRegMask(Output, LVTMA_DATA_SYNCHRONIZATION, 0, 0x00000001);
    RHDRegMask(Output, LVTMA_BIT_DEPTH_CONTROL, LVTMA_DITHER_RESET_BIT, LVTMA_DITHER_RESET_BIT); /* reset temp dithering */
    RHDRegMask(Output, LVTMA_BIT_DEPTH_CONTROL, 0, 0x00111111); /* disable all dithering */
    RHDRegWrite(Output, LVTMA_CNTL, 0); /* disable */
}
#endif

/*
 *
 */
static void
LVDSPower(struct rhdOutput *Output, int Power)
{
    RHDDebug(Output->scrnIndex, "%s(%s,%s)\n",__func__,Output->Name,
	     rhdPowerString[Power]);

    switch (Power) {
    case RHD_POWER_ON:
	LVDSEnable(Output);
	break;
    case RHD_POWER_RESET:
	/*	LVDSDisable(Output);
		break;*/
    case RHD_POWER_SHUTDOWN:
    default:
	LVDSDisable(Output);
	/* LVDSShutdown(Output); */
	break;
    }
    return;
}

/*
 *
 */
static void
LVDSSave(struct rhdOutput *Output)
{
    struct LVDSPrivate *Private = (struct LVDSPrivate *) Output->Private;
    RHDPtr rhdPtr = RHDPTRI(Output);

    RHDFUNC(Output);

    Private->StoreControl = RHDRegRead(Output, LVTMA_CNTL);
    Private->StoreSourceSelect = RHDRegRead(Output,  LVTMA_SOURCE_SELECT);
    Private->StoreBitDepthControl = RHDRegRead(Output, LVTMA_BIT_DEPTH_CONTROL);
    Private->StoreDataSynchronisation = RHDRegRead(Output, LVTMA_DATA_SYNCHRONIZATION);
    Private->StorePWRSEQRefDiv = RHDRegRead(Output, LVTMA_PWRSEQ_REF_DIV);
    Private->StorePWRSEQDelay1 = RHDRegRead(Output, LVTMA_PWRSEQ_DELAY1);
    Private->StorePWRSEQDelay2 = RHDRegRead(Output, LVTMA_PWRSEQ_DELAY2);
    Private->StorePWRSEQControl = RHDRegRead(Output, LVTMA_PWRSEQ_CNTL);
    Private->StorePWRSEQState = RHDRegRead(Output, LVTMA_PWRSEQ_STATE);
    Private->StoreLVDSDataControl = RHDRegRead(Output, LVTMA_LVDS_DATA_CNTL);
    Private->StoreMode = RHDRegRead(Output, LVTMA_MODE);
    Private->StoreTxEnable = RHDRegRead(Output, LVTMA_TRANSMITTER_ENABLE);
    Private->StoreMacroControl = RHDRegRead(Output, LVTMA_MACRO_CONTROL);
    Private->StoreTXControl = RHDRegRead(Output, LVTMA_TRANSMITTER_CONTROL);
    Private->StoreBlModCntl = RHDRegRead(Output, LVTMA_BL_MOD_CNTL);

    Private->Stored = TRUE;
}

/*
 * This needs to reset things like the temporal dithering and the TX appropriately.
 * Currently it's a dumb register dump.
 */
static void
LVDSRestore(struct rhdOutput *Output)
{
    struct LVDSPrivate *Private = (struct LVDSPrivate *) Output->Private;
    RHDPtr rhdPtr = RHDPTRI(Output);

    RHDFUNC(Output);

    if (!Private->Stored) {
	xf86DrvMsg(Output->scrnIndex, X_ERROR,
		   "%s: No registers stored.\n", __func__);
	return;
    }

    RHDRegWrite(Output, LVTMA_CNTL, Private->StoreControl);
    RHDRegWrite(Output, LVTMA_SOURCE_SELECT, Private->StoreSourceSelect);
    RHDRegWrite(Output, LVTMA_BIT_DEPTH_CONTROL,  Private->StoreBitDepthControl);
    RHDRegWrite(Output, LVTMA_DATA_SYNCHRONIZATION, Private->StoreDataSynchronisation);
    RHDRegWrite(Output, LVTMA_PWRSEQ_REF_DIV, Private->StorePWRSEQRefDiv);
    RHDRegWrite(Output, LVTMA_PWRSEQ_DELAY1, Private->StorePWRSEQDelay1);
    RHDRegWrite(Output, LVTMA_PWRSEQ_DELAY2,  Private->StorePWRSEQDelay2);
    RHDRegWrite(Output, LVTMA_PWRSEQ_CNTL, Private->StorePWRSEQControl);
    RHDRegWrite(Output, LVTMA_PWRSEQ_STATE, Private->StorePWRSEQState);
    RHDRegWrite(Output, LVTMA_LVDS_DATA_CNTL, Private->StoreLVDSDataControl);
    RHDRegWrite(Output, LVTMA_MODE, Private->StoreMode);
    RHDRegWrite(Output, LVTMA_TRANSMITTER_ENABLE, Private->StoreTxEnable);
    RHDRegWrite(Output, LVTMA_MACRO_CONTROL, Private->StoreMacroControl);
    RHDRegWrite(Output, LVTMA_TRANSMITTER_CONTROL,  Private->StoreTXControl);
    RHDRegWrite(Output, LVTMA_BL_MOD_CNTL, Private->StoreBlModCntl);

    /*
     * Poor man's debug
     */
    LVDSDebugBacklight(Output);
}

/*
 * Here we pretty much assume that ATOM has either initialised the panel already
 * or that we can find information from ATOM BIOS data tables. We know that the
 * latter assumption is false for some values, but there is no getting around
 * ATI clinging desperately to a broken concept.
 */
static struct LVDSPrivate *
LVDSInfoRetrieve(RHDPtr rhdPtr)
{
    struct LVDSPrivate *Private = xnfcalloc(sizeof(struct LVDSPrivate), 1);
    CARD32 tmp;

    /* These values are not available from atombios data tables at all. */
    Private->MacroControl = RHDRegRead(rhdPtr, LVTMA_MACRO_CONTROL);
    Private->TXClockPattern =
  (_RHDRegRead(rhdPtr, LVTMA_TRANSMITTER_CONTROL) >> 16) & 0x3FF;

    /* For these values, we try to retrieve them from register space first,
       and later override with atombios data table information */
    Private->PowerDigToDE =
  (_RHDRegRead(rhdPtr, LVTMA_PWRSEQ_DELAY1) & 0x000000FF) << 2;

    Private->PowerDEToBL =
  (_RHDRegRead(rhdPtr, LVTMA_PWRSEQ_DELAY1) & 0x0000FF00) >> 6;

    Private->OffDelay = (_RHDRegRead(rhdPtr, LVTMA_PWRSEQ_DELAY2) & 0xFF) << 2;

    tmp = _RHDRegRead(rhdPtr, LVTMA_PWRSEQ_REF_DIV);
    Private->PowerRefDiv = tmp & 0x0FFF;
    Private->BlonRefDiv = (tmp >> 16) & 0x0FFF;
    tmp = _RHDRegRead(rhdPtr, LVTMA_BL_MOD_CNTL);
    if (tmp & 0x1)
	Private->BlLevel = (tmp >> 8) & 0xff;
    else
	Private->BlLevel = -1; /* Backlight control seems to be done some other way */

    Private->DualLink = (_RHDRegRead(rhdPtr, LVTMA_CNTL) >> 24) & 0x00000001;
    Private->LVDS24Bit = _RHDRegRead(rhdPtr, LVTMA_LVDS_DATA_CNTL) & 0x00000001;
    Private->FPDI = _RHDRegRead(rhdPtr, LVTMA_LVDS_DATA_CNTL) & 0x00000010;

    tmp = _RHDRegRead(rhdPtr, LVTMA_BIT_DEPTH_CONTROL);
    Private->TemporalDither =  ((tmp & (1 << 16)) != 0);
    Private->SpatialDither = ((tmp & (1 << 8)) != 0);
    Private->GreyLevel = (tmp & (1 << 24)) ? 4 : 2;

#ifdef ATOM_BIOS
    {
	AtomBiosArgRec data;

  if(RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			   ATOM_LVDS_SEQ_DIG_ONTO_DE, &data) == ATOM_SUCCESS)
	    Private->PowerDigToDE = data.val;

  if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			    ATOM_LVDS_SEQ_DE_TO_BL, &data) == ATOM_SUCCESS)
	    Private->PowerDEToBL = data.val;

  if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			    ATOM_LVDS_OFF_DELAY, &data) == ATOM_SUCCESS)
	    Private->OffDelay = data.val;

  if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			    ATOM_LVDS_DUALLINK, &data) == ATOM_SUCCESS)
	    Private->DualLink = data.val;

  if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			    ATOM_LVDS_24BIT, &data) == ATOM_SUCCESS)
	    Private->LVDS24Bit = data.val;

  if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
				 ATOM_LVDS_FPDI, &data) == ATOM_SUCCESS)
	    Private->FPDI = data.val;

  if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			    ATOM_LVDS_TEMPORAL_DITHER, &data) == ATOM_SUCCESS)
	    Private->TemporalDither = data.val;

  if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			    ATOM_LVDS_SPATIAL_DITHER, &data) == ATOM_SUCCESS)
	    Private->SpatialDither = data.val;

  if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			    ATOM_LVDS_GREYLVL, &data) == ATOM_SUCCESS) {
	    Private->GreyLevel = data.val;
	    xf86DrvMsg(rhdPtr->scrnIndex, X_INFO, "AtomBIOS returned %i Grey Levels\n",
		       Private->GreyLevel);
    }
    }
#endif

    if (Private->LVDS24Bit)
	xf86DrvMsg(rhdPtr->scrnIndex, X_PROBED,
		   "Detected a 24bit %s, %s link panel.\n",
            Private->DualLink ? "dual" : "single",
            Private->FPDI ? "FPDI": "LDI");
    else
	xf86DrvMsg(rhdPtr->scrnIndex, X_PROBED,
		   "Detected a 18bit %s link panel.\n",
             Private->DualLink ? "dual" : "single");

    /* extra noise */
    RHDDebug(rhdPtr->scrnIndex, "Printing LVDS paramaters:\n");
    xf86MsgVerb(X_NONE, LOG_DEBUG, "\tMacroControl: 0x%08X\n",
		(unsigned int) Private->MacroControl);
    xf86MsgVerb(X_NONE, LOG_DEBUG, "\tTXClockPattern: 0x%04X\n",
		Private->TXClockPattern);
    xf86MsgVerb(X_NONE, LOG_DEBUG, "\tPowerDigToDE: 0x%04X\n",
		Private->PowerDigToDE);
    xf86MsgVerb(X_NONE, LOG_DEBUG, "\tPowerDEToBL: 0x%04X\n",
		Private->PowerDEToBL);
    xf86MsgVerb(X_NONE, LOG_DEBUG, "\tOffDelay: 0x%04X\n",
		Private->OffDelay);
    xf86MsgVerb(X_NONE, LOG_DEBUG, "\tPowerRefDiv: 0x%04X\n",
		Private->PowerRefDiv);
    xf86MsgVerb(X_NONE, LOG_DEBUG, "\tBlonRefDiv: 0x%04X\n",
		Private->BlonRefDiv);

    return Private;
}

/*
 *
 */
static void
LVDSDestroy(struct rhdOutput *Output)
{

    struct LVDSPrivate *Private = (struct LVDSPrivate *) Output->Private;

    RHDFUNC(Output);

    if (!Private)
	return;

#ifdef NOT_YET
    if (Private->PropertyPrivate)
	RhdAtomDestroyBacklightControlProperty(Output, Private->PropertyPrivate);
#endif
    xfree(Private);
    Output->Private = NULL;
}

/*
 *
 * Handling for LVTMA block as TMDS.
 *
 */
struct rhdTMDSBPrivate {
    Bool RunsDualLink;
    Bool Coherent;
    DisplayModePtr Mode;

    struct rhdHdmi *Hdmi;

    Bool Stored;

    CARD32 StoreControl;
    CARD32 StoreSource;
    CARD32 StoreFormat;
    CARD32 StoreForce;
    CARD32 StoreReduction;
    CARD32 StoreDCBalancer;
    CARD32 StoreDataSynchro;
    CARD32 StoreMode;
    CARD32 StoreTXEnable;
    CARD32 StoreMacro;
    CARD32 StoreTXControl;
    CARD32 StoreTXAdjust;
    CARD32 StoreTestOutput;

    CARD32 StoreRs690Unknown;
    CARD32 StoreRv600TXAdjust;
    CARD32 StoreRv600PreEmphasis;
};

/*
 *
 */
static ModeStatus
TMDSBModeValid(struct rhdOutput *Output, DisplayModePtr Mode)
{
    RHDFUNC(Output);

    if (Mode->Flags & V_INTERLACE)
        return MODE_NO_INTERLACE;

    if (Mode->Clock < 25000)
	return MODE_CLOCK_LOW;

    if (Output->Connector->Type == RHD_CONNECTOR_DVI_SINGLE) {
    if (Mode->Clock > 165000)
	return MODE_CLOCK_HIGH;
    } else if (Output->Connector->Type == RHD_CONNECTOR_DVI) {
	if (Mode->Clock > 330000) /* could go higher still */
	    return MODE_CLOCK_HIGH;
    }

    return MODE_OK;
}

/*
 *
 */
static void
RS600VoltageControl(struct rhdOutput *Output, DisplayModePtr Mode)
{
    struct rhdTMDSBPrivate *Private = (struct rhdTMDSBPrivate *) Output->Private;

    RHDFUNC(Output);
#ifdef NOTYET
    if (Output->Connector == RHD_CONNECTOR_HDMI || Output->Connector == RHD_CONNECTOR_HDMI_DUAL) {
	int clock = Mode->SynthClock;

	if (Private->RunsDualLink)
	    clock >>= 1;
	if (clock <= 75000) {
	    RHDRegWrite(Output, LVTMA_R600_MACRO_CONTROL, 0x00010213);
	    RHDRegWrite(Output, LVTMA_R600_REG_TEST_OUTPUT, 0x000a0000);
	} else {
	    RHDRegWrite(Output, LVTMA_R600_MACRO_CONTROL, 0x00000213);
	    RHDRegWrite(Output, LVTMA_R600_REG_TEST_OUTPUT, 0x00100000);
	}
    } else
#endif
    {
	if (Private->RunsDualLink) {
	    RHDRegWrite(Output, LVTMA_R600_MACRO_CONTROL, 0x0000020f);
	    RHDRegWrite(Output, LVTMA_R600_REG_TEST_OUTPUT, 0x00100000);
	} else {
	    if (Mode->SynthClock < 39000)
		RHDRegWrite(Output, LVTMA_R600_MACRO_CONTROL, 0x0002020f);
	    else
		RHDRegWrite(Output, LVTMA_R600_MACRO_CONTROL, 0x0000020f);
	    RHDRegWrite(Output, LVTMA_R600_REG_TEST_OUTPUT, 0x00100000);
	}
    }
}

/*
 *
 */
static void
RS690VoltageControl(struct rhdOutput *Output, DisplayModePtr Mode)
{
    RHDPtr rhdPtr = RHDPTRI(Output);
    struct rhdTMDSBPrivate *Private = (struct rhdTMDSBPrivate *) Output->Private;
    CARD32 rev = (RHDRegRead(Output, CONFIG_CNTL) && RS69_CFG_ATI_REV_ID_MASK) >> RS69_CFG_ATI_REV_ID_SHIFT;

    if (rev < 3) {
#ifdef NOTYET
	if (Output->Connector == RHD_CONNECTOR_HDMI || Output->Connector == RHD_CONNECTOR_HDMI_DUAL) {
	    if (Mode->SynthClock > 75000) {
		RHDRegWrite(Output, LVTMA_R600_MACRO_CONTROL, 0xa001632f);
		RHDRegWrite(Output, LVTMA_R600_REG_TEST_OUTPUT, 0x05120000);
		RHDRegMask(Output,  LVTMA_R600_TRANSMITTER_CONTROL, 0x10000000, 0x10000000);
	    } else if (Mode->SynthClock > 41000) {
		RHDRegWrite(Output, LVTMA_R600_MACRO_CONTROL, 0x0000632f);
		RHDRegWrite(Output, LVTMA_R600_REG_TEST_OUTPUT, 0x05120000);
		RHDRegMask(Output,  LVTMA_R600_TRANSMITTER_CONTROL, 0x10000000, 0x10000000);
	    } else {
		RHDRegWrite(Output, LVTMA_R600_MACRO_CONTROL, 0x0003632f);
		RHDRegWrite(Output, LVTMA_R600_REG_TEST_OUTPUT, 0x050b000);
		RHDRegMask(Output,  LVTMA_R600_TRANSMITTER_CONTROL, 0x0, 0x10000000);
	    }
	} else
#endif
	{
	    int clock = Mode->SynthClock;

	    if (Private->RunsDualLink)
		clock >>= 1;

	    RHDRegWrite(Output, LVTMA_R600_REG_TEST_OUTPUT, 0x05120000);

	    if (clock > 75000) {
		RHDRegWrite(Output, LVTMA_R600_MACRO_CONTROL, 0xa001631f);
		RHDRegMask(Output,  LVTMA_R600_TRANSMITTER_CONTROL, 0x10000000, 0x10000000);
	    } else if (clock > 41000) {
		RHDRegWrite(Output, LVTMA_R600_MACRO_CONTROL, 0x0000631f);
		RHDRegMask(Output,  LVTMA_R600_TRANSMITTER_CONTROL, 0x10000000, 0x10000000);
	    } else {
		RHDRegWrite(Output, LVTMA_R600_MACRO_CONTROL, 0x0003631f);
		RHDRegMask(Output,  LVTMA_R600_TRANSMITTER_CONTROL, 0x0, 0x10000000);
	    }
	}
    } else {
#ifdef NOTYET
	if (Output->Connector == RHD_CONNECTOR_HDMI || Output->Connector == RHD_CONNECTOR_HDMI_DUAL) {
	    if (Mode->SynthClock <= 75000) {
		RHDRegWrite(Output, LVTMA_R600_MACRO_CONTROL, 0x0002612f);
		RHDRegWrite(Output, LVTMA_R600_REG_TEST_OUTPUT, 0x010b0000);
		RHDRegMask(Output,  LVTMA_R600_TRANSMITTER_CONTROL, 0x0, 0x10000000);
	    } else {
		RHDRegWrite(Output, LVTMA_R600_MACRO_CONTROL, 0x0000642f);
		RHDRegWrite(Output, LVTMA_R600_REG_TEST_OUTPUT, 0x01120000);
		RHDRegMask(Output,  LVTMA_R600_TRANSMITTER_CONTROL, 0x10000000, 0x10000000);
	    }
	} else
#endif
	{
	    int clock = Mode->SynthClock;

	    if (Private->RunsDualLink)
		clock >>= 1;

	    RHDRegWrite(Output, LVTMA_R600_REG_TEST_OUTPUT, 0x01120000);
	    RHDRegMask(Output,  LVTMA_R600_TRANSMITTER_CONTROL, 0x10000000, 0x10000000);

	    if (Mode->SynthClock > 75000) {
		RHDRegWrite(Output, LVTMA_R600_MACRO_CONTROL, 0x00016318);
	    } else {
		{
#ifdef ATOM_BIOS
		    AtomBiosArgRec data;

        if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
					ATOM_GET_CAPABILITY_FLAG, &data) == ATOM_SUCCESS) {
			if (((data.val & 0x60) == 0x20 || (data.val & 0x80))) {
			    RHDRegWrite(Output, LVTMA_R600_MACRO_CONTROL, 0x00016318);
			} else {
			    RHDRegWrite(Output, LVTMA_R600_MACRO_CONTROL, 0x00006318);
			}
		    } else
#endif
		    {
			RHDRegWrite(Output, LVTMA_R600_MACRO_CONTROL, 0x00006318);
		    }
		}
	    }
	}
    }
}

/*
 * This information is not provided in an atombios data table.
 */
static struct R5xxTMDSBMacro {
    CARD16 Device;
    CARD32 MacroSingle;
    CARD32 MacroDual;
} R5xxTMDSBMacro[] = {
    /*
     * this list isn't complete yet.
     *  Some more values for dual need to be dug up
     */
    { 0x7104, 0x00F20616, 0x00F20616 }, /* R520  */
    { 0x7142, 0x00F2061C, 0x00F2061C }, /* RV515 */
    { 0x7145, 0x00F1061D, 0x00F2061D }, /**/
    { 0x7146, 0x00F1061D, 0x00F1061D }, /* RV515 */
    { 0x7147, 0x0082041D, 0x0082041D }, /* RV505 */
    { 0x7149, 0x00F1061D, 0x00D2061D }, /**/
    { 0x7152, 0x00F2061C, 0x00F2061C }, /* RV515 */
    { 0x7183, 0x00B2050C, 0x00B2050C }, /* RV530 */
    { 0x71C0, 0x00F1061F, 0x00f2061D }, /**/
    { 0x71C1, 0x0062041D, 0x0062041D }, /* RV535 *//**/
    { 0x71C2, 0x00F1061D, 0x00F2061D }, /* RV530 *//**/
    { 0x71C5, 0x00D1061D, 0x00D2061D }, /**/
    { 0x71C6, 0x00F2061D, 0x00F2061D }, /* RV530 */
    { 0x71D2, 0x00F10610, 0x00F20610 }, /* RV530: atombios uses 0x00F1061D *//**/
    { 0x7249, 0x00F1061D, 0x00F1061D }, /* R580  */
    { 0x724B, 0x00F10610, 0x00F10610 }, /* R580: atombios uses 0x00F1061D */
    { 0x7280, 0x0042041F, 0x0042041F }, /* RV570 *//**/
    { 0x7288, 0x0042041F, 0x0042041F }, /* RV570 */
    { 0x791E, 0x0001642F, 0x0001642F }, /* RS690 */
    { 0x791F, 0x0001642F, 0x0001642F }, /* RS690 */
    { 0x9400, 0x00020213, 0x00020213 }, /* R600  */
    { 0x9401, 0x00020213, 0x00020213 }, /* R600  */
    { 0x9402, 0x00020213, 0x00020213 }, /* R600  */
    { 0x9403, 0x00020213, 0x00020213 }, /* R600  */
    { 0x9405, 0x00020213, 0x00020213 }, /* R600  */
    { 0x940A, 0x00020213, 0x00020213 }, /* R600  */
    { 0x940B, 0x00020213, 0x00020213 }, /* R600  */
    { 0x940F, 0x00020213, 0x00020213 }, /* R600  */
    { 0, 0, 0 } /* End marker */
};

static struct RV6xxTMDSBMacro {
    CARD16 Device;
    CARD32 Macro;
    CARD32 TX;
    CARD32 PreEmphasis;
} RV6xxTMDSBMacro[] = {
    { 0x94C1, 0x01030311, 0x10001A00, 0x01801015}, /* RV610 */
    { 0x94C3, 0x01030311, 0x10001A00, 0x01801015}, /* RV610 */
    { 0x9501, 0x0533041A, 0x020010A0, 0x41002045}, /* RV670 */
    { 0x9505, 0x0533041A, 0x020010A0, 0x41002045}, /* RV670 */
    { 0x950F, 0x0533041A, 0x020010A0, 0x41002045}, /* R680  */
    { 0x9587, 0x01030311, 0x10001C00, 0x01C01011}, /* RV630 */
    { 0x9588, 0x01030311, 0x10001C00, 0x01C01011}, /* RV630 */
    { 0x9589, 0x01030311, 0x10001C00, 0x01C01011}, /* RV630 */
    { 0, 0, 0, 0} /* End marker */
};

static void
TMDSBVoltageControl(struct rhdOutput *Output, DisplayModePtr Mode)
{
    struct rhdTMDSBPrivate *Private = (struct rhdTMDSBPrivate *) Output->Private;
    RHDPtr rhdPtr = RHDPTRI(Output);
  int i;

    /* IGP chipsets are rather special */
    if (rhdPtr->ChipSet == RHD_RS690) {
	RS690VoltageControl(Output, Mode);
	return;
    } else if (rhdPtr->ChipSet == RHD_RS600) {
	RS600VoltageControl(Output, Mode);
	return;
    }

    /* TEST_OUTPUT register - IGPs are handled above */
    if (rhdPtr->ChipSet < RHD_RS600) /* r5xx */
	RHDRegMask(Output, LVTMA_REG_TEST_OUTPUT, 0x00200000, 0x00200000);
    else if (rhdPtr->ChipSet < RHD_RV670)
	RHDRegMask(Output, LVTMA_REG_TEST_OUTPUT, 0x00100000, 0x00100000);

    /* macro control values */
    if (rhdPtr->ChipSet < RHD_RV610) { /* R5xx and R600 */
    for (i = 0; R5xxTMDSBMacro[i].Device; i++)
	    if (R5xxTMDSBMacro[i].Device == rhdPtr->PciDeviceID) {
		if (!Private->RunsDualLink)
		    RHDRegWrite(Output, LVTMA_MACRO_CONTROL, R5xxTMDSBMacro[i].MacroSingle);
		else
		    RHDRegWrite(Output, LVTMA_MACRO_CONTROL, R5xxTMDSBMacro[i].MacroDual);
        return;
	    }

	xf86DrvMsg(Output->scrnIndex, X_ERROR, "%s: unhandled chipset: 0x%04X.\n",
		   __func__, rhdPtr->PciDeviceID);
	xf86DrvMsg(Output->scrnIndex, X_INFO, "LVTMA_MACRO_CONTROL: 0x%08X\n",
		   (unsigned int) RHDRegRead(Output, LVTMA_MACRO_CONTROL));
    } else { /* RV6x0 and up */
    for (i = 0; RV6xxTMDSBMacro[i].Device; i++)
	    if (RV6xxTMDSBMacro[i].Device == rhdPtr->PciDeviceID) {
        RHDRegWrite(Output, LVTMA_MACRO_CONTROL, RV6xxTMDSBMacro[i].Macro);
        RHDRegWrite(Output, LVTMA_TRANSMITTER_ADJUST, RV6xxTMDSBMacro[i].TX);
        RHDRegWrite(Output, LVTMA_PREEMPHASIS_CONTROL, RV6xxTMDSBMacro[i].PreEmphasis);
        return;
	    }

	xf86DrvMsg(Output->scrnIndex, X_ERROR, "%s: unhandled chipset: 0x%04X.\n",
		   __func__, rhdPtr->PciDeviceID);
	xf86DrvMsg(Output->scrnIndex, X_INFO, "LVTMA_MACRO_CONTROL: 0x%08X\n",
            (unsigned int) RHDRegRead(Output, LVTMA_MACRO_CONTROL));
	xf86DrvMsg(Output->scrnIndex, X_INFO, "LVTMA_TRANSMITTER_ADJUST: 0x%08X\n",
            (unsigned int) RHDRegRead(Output, LVTMA_TRANSMITTER_ADJUST));
	xf86DrvMsg(Output->scrnIndex, X_INFO, "LVTMA_PREEMPHASIS_CONTROL: 0x%08X\n",
            (unsigned int) RHDRegRead(Output, LVTMA_PREEMPHASIS_CONTROL));
    }
}

/*
 *
 */
static Bool
TMDSBPropertyControl(struct rhdOutput *Output,
	     enum rhdPropertyAction Action, enum rhdOutputProperty Property, union rhdPropertyData *val)
{
    struct rhdTMDSBPrivate *Private = (struct rhdTMDSBPrivate *) Output->Private;

    RHDFUNC(Output);
    switch (Action) {
	case rhdPropertyCheck:
	    switch (Property) {
		case RHD_OUTPUT_COHERENT:
		    return TRUE;
		default:
		    return FALSE;
	    }
	case rhdPropertyGet:
	    switch (Property) {
		case RHD_OUTPUT_COHERENT:
		    val->Bool = Private->Coherent;
		    return TRUE;
		default:
		    return FALSE;
	    }
	    break;
	case rhdPropertySet:
	    switch (Property) {
		case RHD_OUTPUT_COHERENT:
		    Private->Coherent = val->Bool;
		    Output->Mode(Output, Private->Mode);
		    Output->Power(Output, RHD_POWER_ON);
		    break;
		default:
		    return FALSE;
	    }
	    break;
    }
    return TRUE;
}

/*
 *
 */
static void
TMDSBSet(struct rhdOutput *Output, DisplayModePtr Mode)
{
    RHDPtr rhdPtr = RHDPTRI(Output);
    struct rhdTMDSBPrivate *Private = (struct rhdTMDSBPrivate *) Output->Private;

    RHDFUNC(Output);

    RHDRegMask(Output, LVTMA_MODE, 0x00000001, 0x00000001); /* select TMDS */

    /* Clear out some HPD events first: this should be under driver control. */
    RHDRegMask(Output, LVTMA_TRANSMITTER_CONTROL, 0, 0x0000000C);
    RHDRegMask(Output, LVTMA_TRANSMITTER_ENABLE, 0, 0x00070000);
    RHDRegMask(Output, LVTMA_CNTL, 0, 0x00000010);

    /* Disable the transmitter */
	RHDRegMask(Output, LVTMA_TRANSMITTER_ENABLE, 0, 0x00003E3E);

    /* Disable bit reduction and reset temporal dither */
    RHDRegMask(Output, LVTMA_BIT_DEPTH_CONTROL, 0, 0x00010101);
    RHDRegMask(Output, LVTMA_BIT_DEPTH_CONTROL, LVTMA_DITHER_RESET_BIT, LVTMA_DITHER_RESET_BIT);
    usleep(2);
    RHDRegMask(Output, LVTMA_BIT_DEPTH_CONTROL, 0, LVTMA_DITHER_RESET_BIT);
    RHDRegMask(Output, LVTMA_BIT_DEPTH_CONTROL, 0, 0xF0000000); /* not documented */

    /* reset phase on vsync and use RGB */
    RHDRegMask(Output, LVTMA_CNTL, 0x00001000, 0x00011000);

    /* Select CRTC, select syncA, no stereosync */
    RHDRegMask(Output, LVTMA_SOURCE_SELECT, Output->Crtc->Id, 0x00010101);

    RHDRegWrite(Output, LVTMA_COLOR_FORMAT, 0);

    Private->Mode = Mode;
    if (Mode->SynthClock > 165000) {
	RHDRegMask(Output, LVTMA_CNTL, 0x01000000, 0x01000000);
	Private->RunsDualLink = TRUE; /* for TRANSMITTER_ENABLE in TMDSBPower */
    } else {
    RHDRegMask(Output, LVTMA_CNTL, 0, 0x01000000);
	Private->RunsDualLink = FALSE;
    }

    if (rhdPtr->ChipSet > RHD_R600) /* Rv6xx: disable split mode */
	RHDRegMask(Output, LVTMA_CNTL, 0, 0x20000000);

    /* Disable force data */
    RHDRegMask(Output, LVTMA_FORCE_OUTPUT_CNTL, 0, 0x00000001);

    /* DC balancer enable */
    RHDRegMask(Output, LVTMA_DCBALANCER_CONTROL, 0x00000001, 0x00000001);

    TMDSBVoltageControl(Output, Mode);

    /* use IDCLK */
    RHDRegMask(Output, LVTMA_TRANSMITTER_CONTROL, 0x00000010, 0x00000010);
    /* LVTMA only: use clock selected by next write */
    RHDRegMask(Output, LVTMA_TRANSMITTER_CONTROL, 0x20000000, 0x20000000);
    /* coherent mode */
    RHDRegMask(Output, LVTMA_TRANSMITTER_CONTROL,
	       Private->Coherent ? 0 : 0x10000000, 0x10000000);
    /* clear LVDS clock pattern */
    RHDRegMask(Output, LVTMA_TRANSMITTER_CONTROL, 0, 0x03FF0000);

    /* reset transmitter pll */
    RHDRegMask(Output, LVTMA_TRANSMITTER_CONTROL, 0x00000002, 0x00000002);
    usleep(2);
    RHDRegMask(Output, LVTMA_TRANSMITTER_CONTROL, 0, 0x00000002);
    usleep(20);

    /* restart data synchronisation */
    RHDRegMask(Output, LVTMA_DATA_SYNCHRONIZATION, 0x00000001, 0x00000001);
    RHDRegMask(Output, LVTMA_DATA_SYNCHRONIZATION, 0x00000100, 0x00000100);
    usleep(2);
    RHDRegMask(Output, LVTMA_DATA_SYNCHRONIZATION, 0, 0x00000001);

    RHDHdmiSetMode(Private->Hdmi, Mode);
}

/*
 *
 */
static void
TMDSBPower(struct rhdOutput *Output, int Power)
{
    RHDPtr rhdPtr = RHDPTRI(Output);
    struct rhdTMDSBPrivate *Private = (struct rhdTMDSBPrivate *) Output->Private;

    RHDDebug(Output->scrnIndex, "%s(%s,%s)\n",__func__,Output->Name,
	     rhdPowerString[Power]);

    RHDRegMask(Output, LVTMA_MODE, 0x00000001, 0x00000001); /* select TMDS */

    switch (Power) {
    case RHD_POWER_ON:
	RHDRegMask(Output, LVTMA_CNTL, 0x1, 0x00000001);

	if (Private->RunsDualLink)
	    RHDRegMask(Output, LVTMA_TRANSMITTER_ENABLE, 0x00003E3E,0x00003E3E);
	else
	    RHDRegMask(Output, LVTMA_TRANSMITTER_ENABLE, 0x0000003E, 0x00003E3E);

	RHDRegMask(Output, LVTMA_TRANSMITTER_CONTROL, 0x00000001, 0x00000001);
	usleep(2);
	RHDRegMask(Output, LVTMA_TRANSMITTER_CONTROL, 0, 0x00000002);
	if(Output->Connector != NULL && RHDConnectorEnableHDMI(Output->Connector))
	    RHDHdmiEnable(Private->Hdmi, TRUE);
	else
	    RHDHdmiEnable(Private->Hdmi, FALSE);
	return;
    case RHD_POWER_RESET:
	RHDRegMask(Output, LVTMA_TRANSMITTER_ENABLE, 0, 0x00003E3E);
	return;
    case RHD_POWER_SHUTDOWN:
    default:
	RHDRegMask(Output, LVTMA_TRANSMITTER_CONTROL, 0x00000002, 0x00000002);
	usleep(2);
	RHDRegMask(Output, LVTMA_TRANSMITTER_CONTROL, 0, 0x00000001);
	RHDRegMask(Output, LVTMA_TRANSMITTER_ENABLE, 0, 0x00003E3E);
	RHDRegMask(Output, LVTMA_CNTL, 0, 0x00000001);
	RHDHdmiEnable(Private->Hdmi, FALSE);
	return;
    }
}

/*
 *
 */
static void
TMDSBSave(struct rhdOutput *Output)
{
    struct rhdTMDSBPrivate *Private = (struct rhdTMDSBPrivate *) Output->Private;
    RHDPtr rhdPtr = RHDPTRI(Output);

    RHDFUNC(Output);

    Private->StoreControl = RHDRegRead(Output, LVTMA_CNTL);
    Private->StoreSource = RHDRegRead(Output, LVTMA_SOURCE_SELECT);
    Private->StoreFormat = RHDRegRead(Output, LVTMA_COLOR_FORMAT);
    Private->StoreForce = RHDRegRead(Output, LVTMA_FORCE_OUTPUT_CNTL);
    Private->StoreReduction = RHDRegRead(Output, LVTMA_BIT_DEPTH_CONTROL);
    Private->StoreDCBalancer = RHDRegRead(Output, LVTMA_DCBALANCER_CONTROL);

    Private->StoreDataSynchro = RHDRegRead(Output, LVTMA_DATA_SYNCHRONIZATION);
    Private->StoreMode = RHDRegRead(Output, LVTMA_MODE);
    Private->StoreTXEnable = RHDRegRead(Output, LVTMA_TRANSMITTER_ENABLE);
    Private->StoreMacro = RHDRegRead(Output, LVTMA_MACRO_CONTROL);
    Private->StoreTXControl = RHDRegRead(Output, LVTMA_TRANSMITTER_CONTROL);
    Private->StoreTestOutput = RHDRegRead(Output, LVTMA_REG_TEST_OUTPUT);

    if (rhdPtr->ChipSet > RHD_R600) { /* Rv6x0 */
       Private->StoreRv600TXAdjust = RHDRegRead(Output, LVTMA_TRANSMITTER_ADJUST);
       Private->StoreRv600PreEmphasis = RHDRegRead(Output, LVTMA_PREEMPHASIS_CONTROL);
    }

    RHDHdmiSave(Private->Hdmi);

    Private->Stored = TRUE;
}

/*
 *
 */
static void
TMDSBRestore(struct rhdOutput *Output)
{
    struct rhdTMDSBPrivate *Private = (struct rhdTMDSBPrivate *) Output->Private;
    RHDPtr rhdPtr = RHDPTRI(Output);

    RHDFUNC(Output);

    if (!Private->Stored) {
	xf86DrvMsg(Output->scrnIndex, X_ERROR,
		   "%s: No registers stored.\n", __func__);
	return;
    }

    RHDRegWrite(Output, LVTMA_CNTL, Private->StoreControl);
    RHDRegWrite(Output, LVTMA_SOURCE_SELECT, Private->StoreSource);
    RHDRegWrite(Output, LVTMA_COLOR_FORMAT, Private->StoreFormat);
    RHDRegWrite(Output, LVTMA_FORCE_OUTPUT_CNTL, Private->StoreForce);
    RHDRegWrite(Output, LVTMA_BIT_DEPTH_CONTROL, Private->StoreReduction);
    RHDRegWrite(Output, LVTMA_DCBALANCER_CONTROL, Private->StoreDCBalancer);

    RHDRegWrite(Output, LVTMA_DATA_SYNCHRONIZATION, Private->StoreDataSynchro);
    RHDRegWrite(Output, LVTMA_MODE, Private->StoreMode);
    RHDRegWrite(Output, LVTMA_TRANSMITTER_ENABLE, Private->StoreTXEnable);
    RHDRegWrite(Output, LVTMA_MACRO_CONTROL, Private->StoreMacro);
    RHDRegWrite(Output, LVTMA_TRANSMITTER_CONTROL, Private->StoreTXControl);
    RHDRegWrite(Output, LVTMA_REG_TEST_OUTPUT, Private->StoreTestOutput);

    if (rhdPtr->ChipSet > RHD_R600) { /* Rv6x0 */
	RHDRegWrite(Output, LVTMA_TRANSMITTER_ADJUST, Private->StoreRv600TXAdjust);
	RHDRegWrite(Output, LVTMA_PREEMPHASIS_CONTROL, Private->StoreRv600PreEmphasis);
    }

    RHDHdmiRestore(Private->Hdmi);
}


/*
 *
 */
static void
TMDSBDestroy(struct rhdOutput *Output)
{
    struct rhdTMDSBPrivate *Private = (struct rhdTMDSBPrivate *) Output->Private;
    RHDFUNC(Output);

    if (!Private)
	return;

    RHDHdmiDestroy(Private->Hdmi);

    xfree(Private);
    Output->Private = NULL;
}

#ifdef NOT_YET
static Bool
LVDSPropertyWrapper(struct rhdOutput *Output,
		    enum rhdPropertyAction Action,
		    enum rhdOutputProperty Property,
		    union rhdPropertyData *val)
{
    struct LVDSPrivate *Private = (struct LVDSPrivate *) Output->Private;
    void *storePrivate = Output->Private;
    Bool (*func)(struct rhdOutput *,enum rhdPropertyAction, enum rhdOutputProperty,
		  union rhdPropertyData *) = Private->WrappedPropertyCallback;
    Bool ret;

    Output->Private = Private->PropertyPrivate;
    ret = func(Output, Action, Property, val);
    Output->Private = storePrivate;

    return ret;
}
#endif

/*
 *
 */
struct rhdOutput *
RHDLVTMAInit(RHDPtr rhdPtr, CARD8 Type)
{
    struct rhdOutput *Output;

    RHDFUNC(rhdPtr);

    /* Stop weird connector types */
    if ((Type != RHD_CONNECTOR_PANEL)
	&& (Type != RHD_CONNECTOR_DVI)
	&& (Type != RHD_CONNECTOR_DVI_SINGLE)) {
	xf86DrvMsg(rhdPtr->scrnIndex, X_ERROR, "%s: unhandled connector type:"
		   " %d\n", __func__, Type);
	return NULL;
    }

    Output = xnfcalloc(sizeof(struct rhdOutput), 1);

    Output->scrnIndex = rhdPtr->scrnIndex;
    Output->Id = RHD_OUTPUT_LVTMA;

    Output->Sense = NULL; /* not implemented in hw */

    if (Type == RHD_CONNECTOR_PANEL) {
	struct LVDSPrivate *Private;

	Output->Name = "LVDS";

	Output->ModeValid = LVDSModeValid;
	Output->Mode = LVDSSet;
	Output->Power = LVDSPower;
	Output->Save = LVDSSave;
	Output->Restore = LVDSRestore;
	Output->Property = LVDSPropertyControl;
	Output->Destroy = LVDSDestroy;
	Output->Private = Private =  LVDSInfoRetrieve(rhdPtr);
#ifdef NOT_YET
	if (Private->BlLevel < 0) {
	    Private->BlLevel = RhdAtomSetupBacklightControlProperty(Output, &Private->WrappedPropertyCallback,
								    &Private->PropertyPrivate);
	    if (Private->PropertyPrivate)
		Output->Property = LVDSPropertyWrapper;
	} else
#else
	if (Private->BlLevel >= 0)
#endif
	    LVDSDebugBacklight(Output);

    } else {
	struct rhdTMDSBPrivate *Private = xnfcalloc(sizeof(struct rhdTMDSBPrivate), 1);

	Output->Name = "TMDS B";

	Output->ModeValid = TMDSBModeValid;
	Output->Mode = TMDSBSet;
	Output->Power = TMDSBPower;
	Output->Save = TMDSBSave;
	Output->Restore = TMDSBRestore;
	Output->Property = TMDSBPropertyControl;
	Output->Destroy = TMDSBDestroy;

	Private->Hdmi = RHDHdmiInit(rhdPtr, Output);
	Output->Private = Private;

	Private->RunsDualLink = FALSE;
	Private->Coherent = FALSE;
    }

    return Output;
}
