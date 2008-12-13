/*
 * Copyright 2007, 2008  Luc Verhaegen <lverhaegen@novell.com>
 * Copyright 2007, 2008  Matthias Hopf <mhopf@novell.com>
 * Copyright 2007, 2008  Egbert Eich   <eich@novell.com>
 * Copyright 2007, 2008  Advanced Micro Devices, Inc.
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
#include "rhd_connector.h"
#include "rhd_output.h"
#include "rhd_regs.h"
#include "rhd_hdmi.h"
#ifdef ATOM_BIOS
#include "rhd_atombios.h"
#include "rhd_atomout.h"
#endif

#define FMT2_OFFSET 0x800
#define DIG1_OFFSET 0x000
#define DIG2_OFFSET 0x400

/*
 * Transmitter
 */
struct transmitter {
    enum rhdSensedOutput (*Sense) (struct rhdOutput *Output,
				   enum rhdConnectorType Type);
    ModeStatus (*ModeValid) (struct rhdOutput *Output, DisplayModePtr Mode);
    void (*Mode) (struct rhdOutput *Output, struct rhdCrtc *Crtc, DisplayModePtr Mode);
    void (*Power) (struct rhdOutput *Output, int Power);
    void (*Save) (struct rhdOutput *Output);
    void (*Restore) (struct rhdOutput *Output);
    void (*Destroy) (struct rhdOutput *Output);
    Bool (*Property) (struct rhdOutput *Output,
		      enum rhdPropertyAction Action, enum rhdOutputProperty Property, union rhdPropertyData *val);
#ifdef NOT_YET
    Bool (*WrappedPropertyCallback) (struct rhdOutput *Output,
		      enum rhdPropertyAction Action, enum rhdOutputProperty Property, union rhdPropertyData *val);
    void *PropertyPrivate;
#endif
    void *Private;
};

/*
 * Encoder
 */
struct encoder {
    ModeStatus (*ModeValid) (struct rhdOutput *Output, DisplayModePtr Mode);
    void (*Mode) (struct rhdOutput *Output, struct rhdCrtc *Crtc, DisplayModePtr Mode);
    void (*Power) (struct rhdOutput *Output, int Power);
    void (*Save) (struct rhdOutput *Output);
    void (*Restore) (struct rhdOutput *Output);
    void (*Destroy) (struct rhdOutput *Output);
    void *Private;
};

/*
 *
 */
enum encoderMode {
    DISPLAYPORT = 0,
    LVDS = 1,
    TMDS_DVI = 2,
    TMDS_HDMI = 3,
    SDVO = 4
};

enum encoderID {
    ENCODER_NONE,
    ENCODER_DIG1,
    ENCODER_DIG2
};

struct DIGPrivate
{
    struct encoder Encoder;
    struct transmitter Transmitter;
    enum encoderID EncoderID;
    enum encoderMode EncoderMode;
    Bool Coherent;
    Bool RunDualLink;
    DisplayModePtr Mode;
    struct rhdHdmi *Hdmi;

    /* LVDS */
    Bool FPDI;
    CARD32 PowerSequenceDe2Bl;
    CARD32 PowerSequenceDig2De;
    CARD32 OffDelay;
    struct rhdFMTDither FMTDither;
    int BlLevel;
};

/*
 * LVTMA Transmitter
 */

struct LVTMATransmitterPrivate
{
    Bool Stored;

    CARD32 StoredTransmitterControl;
    CARD32 StoredTransmitterAdjust;
    CARD32 StoredPreemphasisControl;
    CARD32 StoredMacroControl;
    CARD32 StoredLVTMADataSynchronization;
    CARD32 StoredTransmiterEnable;
    CARD32 StoredPwrSeqCntl;
    CARD32 StoredPwrSeqRevDiv;
    CARD32 StoredPwrSeqDelay1;
    CARD32 StoredPwrSeqDelay2;
};

/*
 *
 */
static ModeStatus
LVTMATransmitterModeValid(struct rhdOutput *Output, DisplayModePtr Mode)
{
    RHDFUNC(Output);

    if (Mode->Flags & V_INTERLACE)
        return MODE_NO_INTERLACE;

    if (Output->Connector->Type == RHD_CONNECTOR_DVI_SINGLE
	&& Mode->SynthClock > 165000)
	return MODE_CLOCK_HIGH;

    return MODE_OK;
}

static void
LVDSSetBacklight(struct rhdOutput *Output, int level)
{
    struct DIGPrivate *Private = (struct DIGPrivate *) Output->Private;

    RHDFUNC(Output);

    Private->BlLevel = level;

    RHDRegMask(Output, RV620_LVTMA_PWRSEQ_REF_DIV,
	       0x144 << LVTMA_BL_MOD_REF_DI_SHIFT,
	       0x7ff << LVTMA_BL_MOD_REF_DI_SHIFT);
    RHDRegWrite(Output, RV620_LVTMA_BL_MOD_CNTL,
		0xff << LVTMA_BL_MOD_RES_SHIFT
		| level << LVTMA_BL_MOD_LEVEL_SHIFT
		| LVTMA_BL_MOD_EN);
}

/*
 *
 */
static Bool
LVDSTransmitterPropertyControl(struct rhdOutput *Output,
	     enum rhdPropertyAction Action, enum rhdOutputProperty Property, union rhdPropertyData *val)
{
    struct DIGPrivate *Private = (struct DIGPrivate *) Output->Private;

    RHDFUNC(Output);
    switch (Action) {
	case rhdPropertyCheck:
	    if (Private->BlLevel < 0)
		return FALSE;
	switch (Property) {
	    case RHD_OUTPUT_BACKLIGHT:
		    return TRUE;
	    default:
		return FALSE;
	}
	case rhdPropertyGet:
	    if (Private->BlLevel < 0)
		return FALSE;
	    switch (Property) {
		case RHD_OUTPUT_BACKLIGHT:
		    val->integer = Private->BlLevel;
		    return TRUE;
		default:
		    return FALSE;
	    }
	    break;
	case rhdPropertySet:
	    if (Private->BlLevel < 0)
		return FALSE;
	    switch (Property) {
		case RHD_OUTPUT_BACKLIGHT:
		    LVDSSetBacklight(Output, val->integer);
		    return TRUE;
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
static Bool
TMDSTransmitterPropertyControl(struct rhdOutput *Output,
	     enum rhdPropertyAction Action, enum rhdOutputProperty Property, union rhdPropertyData *val)
{
    struct DIGPrivate *Private = (struct DIGPrivate *) Output->Private;

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
		    val->Bool =  Private->Coherent;
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
LVTMATransmitterSet(struct rhdOutput *Output, struct rhdCrtc *Crtc, DisplayModePtr Mode)
{
    struct DIGPrivate *Private = (struct DIGPrivate *)Output->Private;
    CARD32 value = 0;
#ifdef ATOM_BIOS
    AtomBiosArgRec data;
#endif
    RHDPtr rhdPtr = RHDPTRI(Output);
    Bool doCoherent = Private->Coherent;
    RHDFUNC(Output);

    /* set coherent / not coherent mode; whatever that is */
    if (Output->Connector->Type != RHD_CONNECTOR_PANEL)
	RHDRegMask(Output, RV620_LVTMA_TRANSMITTER_CONTROL,
		   doCoherent ? 0 : RV62_LVTMA_BYPASS_PLL, RV62_LVTMA_BYPASS_PLL);

    Private->Mode = Mode;
#ifdef ATOM_BIOS
    RHDDebug(Output->scrnIndex, "%s: SynthClock: %i Hex: %x EncoderMode: %x\n",__func__,
	     (Mode->SynthClock),(Mode->SynthClock / 10), Private->EncoderMode);

    /* Set up magic value that's used for list lookup */
    value = ((Mode->SynthClock / 10 / ((Private->RunDualLink) ? 2 : 1)) & 0xffff)
	| (Private->EncoderMode << 16)
	| ((doCoherent ? 0x2 : 0) << 24);

    RHDDebug(Output->scrnIndex, "%s: GetConditionalGoldenSettings for: %x\n", __func__, value);

    /* Get data from DIG2TransmitterControl table */
    data.val = 0x4d;
    if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS, ATOMBIOS_GET_CODE_DATA_TABLE,
			&data) == ATOM_SUCCESS) {
	AtomBiosArgRec data1;
	CARD32 *d_p = NULL;

	data1.GoldenSettings.BIOSPtr = data.CommandDataTable.loc;
	data1.GoldenSettings.End = data1.GoldenSettings.BIOSPtr + data.CommandDataTable.size;
	data1.GoldenSettings.value = value;

	/* now find pointer */
    if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			    ATOM_GET_CONDITIONAL_GOLDEN_SETTINGS, &data1) == ATOM_SUCCESS) {
	    d_p = (CARD32*)data1.GoldenSettings.BIOSPtr;
	} else {
	    /* nothing found, now try toggling the coherent setting */
	    doCoherent = !doCoherent;
	    value = (value & ~(0x2 << 24)) | ((doCoherent ? 0x2 : 0) << 24);
	    data1.GoldenSettings.value = value;

        if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			    ATOM_GET_CONDITIONAL_GOLDEN_SETTINGS, &data1) == ATOM_SUCCESS) {
		d_p = (CARD32*)data1.GoldenSettings.BIOSPtr;
		/* set coherent / not coherent mode; whatever that is */
		xf86DrvMsg(Output->scrnIndex, X_INFO, "%s: %soherent Mode not supported, switching to %soherent.\n",
			   __func__, doCoherent ? "Inc" : "C", doCoherent ? "C" : "Inc");
		if (Output->Connector->Type != RHD_CONNECTOR_PANEL)
		    RHDRegMask(Output, RV620_LVTMA_TRANSMITTER_CONTROL,
			       doCoherent ? 0 : RV62_LVTMA_BYPASS_PLL, RV62_LVTMA_BYPASS_PLL);
	    } else
		doCoherent = Private->Coherent; /* reset old value if nothing found either */
	}
	if (d_p) {
	    RHDDebug(Output->scrnIndex, "TransmitterAdjust: 0x%8.8x\n",d_p[0]);
	    RHDRegWrite(Output, RV620_LVTMA_TRANSMITTER_ADJUST, d_p[0]);

	    RHDDebug(Output->scrnIndex, "PreemphasisControl: 0x%8.8x\n",d_p[1]);
	    RHDRegWrite(Output, RV620_LVTMA_PREEMPHASIS_CONTROL, d_p[1]);

	    RHDDebug(Output->scrnIndex, "MacroControl: 0x%8.8x\n",d_p[2]);
	    RHDRegWrite(Output, RV620_LVTMA_MACRO_CONTROL, d_p[2]);
	} else
	    xf86DrvMsg(Output->scrnIndex, X_WARNING, "%s: cannot get golden settings\n",__func__);
    } else
#endif
    {
	xf86DrvMsg(Output->scrnIndex, X_WARNING, "%s: No AtomBIOS supplied "
		   "electrical parameters available\n", __func__);
    }
}

/*
 *
 */
static void
LVTMATransmitterSave(struct rhdOutput *Output)
{
    struct DIGPrivate *digPrivate = (struct DIGPrivate *)Output->Private;
    struct LVTMATransmitterPrivate *Private = (struct LVTMATransmitterPrivate*)digPrivate->Transmitter.Private;

    Private->StoredTransmitterControl       = RHDRegRead(Output, RV620_LVTMA_TRANSMITTER_CONTROL);
    Private->StoredTransmitterAdjust        = RHDRegRead(Output, RV620_LVTMA_TRANSMITTER_ADJUST);
    Private->StoredPreemphasisControl       = RHDRegRead(Output, RV620_LVTMA_PREEMPHASIS_CONTROL);
    Private->StoredMacroControl             = RHDRegRead(Output, RV620_LVTMA_MACRO_CONTROL);
    Private->StoredLVTMADataSynchronization = RHDRegRead(Output, RV620_LVTMA_DATA_SYNCHRONIZATION);
    Private->StoredTransmiterEnable         = RHDRegRead(Output, RV620_LVTMA_TRANSMITTER_ENABLE);
}

/*
 *
 */
static void
LVTMATransmitterRestore(struct rhdOutput *Output)
{
    struct DIGPrivate *digPrivate = (struct DIGPrivate *)Output->Private;
    struct LVTMATransmitterPrivate *Private = (struct LVTMATransmitterPrivate*)digPrivate->Transmitter.Private;

    RHDFUNC(Output);

    /* write control values back */
    RHDRegWrite(Output, RV620_LVTMA_TRANSMITTER_CONTROL,Private->StoredTransmitterControl);
    usleep (14);
    /* reset PLL */
    RHDRegWrite(Output, RV620_LVTMA_TRANSMITTER_CONTROL,Private->StoredTransmitterControl
		| RV62_LVTMA_PLL_RESET);
    usleep (10);
    /* unreset PLL */
    RHDRegWrite(Output, RV620_LVTMA_TRANSMITTER_CONTROL,Private->StoredTransmitterControl);
    usleep(1000);
    RHDRegWrite(Output, RV620_LVTMA_TRANSMITTER_ADJUST, Private->StoredTransmitterAdjust);
    RHDRegWrite(Output, RV620_LVTMA_PREEMPHASIS_CONTROL, Private->StoredPreemphasisControl);
    RHDRegWrite(Output, RV620_LVTMA_MACRO_CONTROL, Private->StoredMacroControl);
    /* start data synchronization */
    RHDRegWrite(Output, RV620_LVTMA_DATA_SYNCHRONIZATION, (Private->StoredLVTMADataSynchronization
							   & ~(CARD32)RV62_LVTMA_DSYNSEL)
		| RV62_LVTMA_PFREQCHG);
    usleep (1);
    RHDRegWrite(Output, RV620_LVTMA_DATA_SYNCHRONIZATION, Private->StoredLVTMADataSynchronization);
    usleep(10);
    RHDRegWrite(Output, RV620_LVTMA_DATA_SYNCHRONIZATION, Private->StoredLVTMADataSynchronization);
    RHDRegWrite(Output, RV620_LVTMA_TRANSMITTER_ENABLE, Private->StoredTransmiterEnable);
}

/*
 *
 */
static void
LVTMA_TMDSTransmitterSet(struct rhdOutput *Output, struct rhdCrtc *Crtc, DisplayModePtr Mode)
{
    RHDFUNC(Output);

    /* TMDS Mode */
    RHDRegMask(Output, RV620_LVTMA_TRANSMITTER_CONTROL,
	       RV62_LVTMA_USE_CLK_DATA, RV62_LVTMA_USE_CLK_DATA);

    LVTMATransmitterSet(Output, Crtc, Mode);

    /* use differential post divider input */
    RHDRegMask(Output, RV620_LVTMA_TRANSMITTER_CONTROL,
	       RV62_LVTMA_IDSCKSEL, RV62_LVTMA_IDSCKSEL);
}

/*
 *
 */
static void
LVTMA_TMDSTransmitterPower(struct rhdOutput *Output, int Power)
{
    struct DIGPrivate *Private = (struct DIGPrivate *)Output->Private;

    RHDFUNC(Output);

    switch (Power) {
	case RHD_POWER_ON:
	    /* enable PLL */
	    RHDRegMask(Output, RV620_LVTMA_TRANSMITTER_CONTROL,
		       RV62_LVTMA_PLL_ENABLE, RV62_LVTMA_PLL_ENABLE);
	    usleep(14);
	    /* PLL reset on */
	    RHDRegMask(Output, RV620_LVTMA_TRANSMITTER_CONTROL,
		       RV62_LVTMA_PLL_RESET, RV62_LVTMA_PLL_RESET);
	    usleep(10);
	    /* PLL reset off */
	    RHDRegMask(Output, RV620_LVTMA_TRANSMITTER_CONTROL,
		       0, RV62_LVTMA_PLL_RESET);
	    usleep(1000);
	    /* start data synchronization */
	    RHDRegMask(Output, RV620_LVTMA_DATA_SYNCHRONIZATION,
		       RV62_LVTMA_PFREQCHG, RV62_LVTMA_PFREQCHG);
	    usleep(1);
	    /* restart write address logic */
	    RHDRegMask(Output, RV620_LVTMA_DATA_SYNCHRONIZATION,
		       RV62_LVTMA_DSYNSEL, RV62_LVTMA_DSYNSEL);
#if 1
	    /* TMDS Mode ?? */
	    RHDRegMask(Output, RV620_LVTMA_TRANSMITTER_CONTROL,
		       RV62_LVTMA_MODE, RV62_LVTMA_MODE);
#endif
	    /* enable lower link */
	    RHDRegMask(Output, RV620_LVTMA_TRANSMITTER_ENABLE,
		       RV62_LVTMA_LNKL,
		       RV62_LVTMA_LNK_ALL);
	    if (Private->RunDualLink) {
		usleep (28);
		/* enable upper link */
		RHDRegMask(Output, RV620_LVTMA_TRANSMITTER_ENABLE,
			   RV62_LVTMA_LNKU,
			   RV62_LVTMA_LNKU);
	    }
	    return;
	case RHD_POWER_RESET:
	    /* disable all links */
	    RHDRegMask(Output, RV620_LVTMA_TRANSMITTER_ENABLE,
		       0, RV62_LVTMA_LNK_ALL);
	    return;
	case RHD_POWER_SHUTDOWN:
	default:
	    /* disable transmitter */
	    RHDRegMask(Output, RV620_LVTMA_TRANSMITTER_ENABLE,
		       0, RV62_LVTMA_LNK_ALL);
	    /* PLL reset */
	    RHDRegMask(Output, RV620_LVTMA_TRANSMITTER_CONTROL,
		       RV62_LVTMA_PLL_RESET, RV62_LVTMA_PLL_RESET);
	    usleep(10);
	    /* end PLL reset */
	    RHDRegMask(Output, RV620_LVTMA_TRANSMITTER_CONTROL,
		       0, RV62_LVTMA_PLL_RESET);
	    /* disable data synchronization */
	    RHDRegMask(Output, RV620_LVTMA_DATA_SYNCHRONIZATION,
		       0, RV62_LVTMA_DSYNSEL);
	    /* reset macro control */
	    RHDRegWrite(Output, RV620_LVTMA_TRANSMITTER_ADJUST, 0);

	    return;
    }
}

/*
 *
 */
static void
LVTMA_TMDSTransmitterSave(struct rhdOutput *Output)
{
    struct DIGPrivate *digPrivate = (struct DIGPrivate *)Output->Private;
    struct LVTMATransmitterPrivate *Private = (struct LVTMATransmitterPrivate*)digPrivate->Transmitter.Private;

    RHDFUNC(Output);

    LVTMATransmitterSave(Output);

    Private->Stored = TRUE;
}

/*
 *
 */
static void
LVTMA_TMDSTransmitterRestore(struct rhdOutput *Output)
{
    struct DIGPrivate *digPrivate = (struct DIGPrivate *)Output->Private;
    struct LVTMATransmitterPrivate *Private = (struct LVTMATransmitterPrivate*)digPrivate->Transmitter.Private;

    RHDFUNC(Output);

    if (!Private->Stored) {
	xf86DrvMsg(Output->scrnIndex, X_ERROR,
		   "%s: No registers stored.\n", __func__);
	return;
    }

    LVTMATransmitterRestore(Output);
}

/*
 *
 */
static void
LVTMA_LVDSTransmitterSet(struct rhdOutput *Output, struct rhdCrtc *Crtc, DisplayModePtr Mode)
{
    RHDFUNC(Output);

    /* LVDS Mode */
    RHDRegMask(Output, RV620_LVTMA_TRANSMITTER_CONTROL,
	       0, RV62_LVTMA_USE_CLK_DATA);

    LVTMATransmitterSet(Output, Crtc, Mode);

    /* use IDCLK */
    RHDRegMask(Output, RV620_LVTMA_TRANSMITTER_CONTROL, RV62_LVTMA_IDSCKSEL, RV62_LVTMA_IDSCKSEL);
    /* enable pwrseq, pwrseq overwrite PPL enable, reset */
    RHDRegMask(Output,  RV620_LVTMA_PWRSEQ_CNTL,
	       RV62_LVTMA_PWRSEQ_EN
	       | RV62_LVTMA_PLL_ENABLE_PWRSEQ_MASK
	       | RV62_LVTMA_PLL_RESET_PWRSEQ_MASK,
	       RV62_LVTMA_PWRSEQ_EN
	       | RV62_LVTMA_PLL_ENABLE_PWRSEQ_MASK
	       | RV62_LVTMA_PLL_RESET_PWRSEQ_MASK
	);

}

/*
 *
 */
static void
LVTMA_LVDSTransmitterPower(struct rhdOutput *Output, int Power)
{
    struct DIGPrivate *Private = (struct DIGPrivate *)Output->Private;
    CARD32 tmp, tmp1;
    int i;

    RHDFUNC(Output);

    switch (Power) {
	case RHD_POWER_ON:
	    /* enable PLL */
	    RHDRegMask(Output, RV620_LVTMA_TRANSMITTER_CONTROL,
		       RV62_LVTMA_PLL_ENABLE, RV62_LVTMA_PLL_ENABLE);
	    usleep(14);
	    /* PLL reset on */
	    RHDRegMask(Output, RV620_LVTMA_TRANSMITTER_CONTROL,
		       RV62_LVTMA_PLL_RESET, RV62_LVTMA_PLL_RESET);
	    usleep(10);
	    /* PLL reset off */
	    RHDRegMask(Output, RV620_LVTMA_TRANSMITTER_CONTROL,
		       0, RV62_LVTMA_PLL_RESET);
	    usleep(1000);
	    /* start data synchronization */
	    RHDRegMask(Output, RV620_LVTMA_DATA_SYNCHRONIZATION,
		       RV62_LVTMA_PFREQCHG, RV62_LVTMA_PFREQCHG);
	    usleep(1);
	    /* restart write address logic */
	    RHDRegMask(Output, RV620_LVTMA_DATA_SYNCHRONIZATION,
		       RV62_LVTMA_DSYNSEL, RV62_LVTMA_DSYNSEL);
	    /* SYNCEN disables pwrseq ?? */
	    RHDRegMask(Output, RV620_LVTMA_PWRSEQ_CNTL,
		       RV62_LVTMA_PWRSEQ_DISABLE_SYNCEN_CONTROL_OF_TX_EN,
		       RV62_LVTMA_PWRSEQ_DISABLE_SYNCEN_CONTROL_OF_TX_EN);
	    /* LVDS Mode ?? */
	    RHDRegMask(Output, RV620_LVTMA_TRANSMITTER_CONTROL,
		       0, RV62_LVTMA_MODE);
	    /* enable links */
	    if (Private->RunDualLink) {
		if (Private->FMTDither.LVDS24Bit)
		    RHDRegMask(Output, RV620_LVTMA_TRANSMITTER_ENABLE, 0x3ff, 0x3ff);
		else
		    RHDRegMask(Output, RV620_LVTMA_TRANSMITTER_ENABLE, 0x1ef, 0x3ff);
		    } else {
		if (Private->FMTDither.LVDS24Bit)
		    RHDRegMask(Output, RV620_LVTMA_TRANSMITTER_ENABLE, 0x1f, 0x3ff);
		else
		    RHDRegMask(Output, RV620_LVTMA_TRANSMITTER_ENABLE, 0x0f, 0x3ff);
	    }
	    RHDRegMask(Output, RV620_LVTMA_PWRSEQ_CNTL, 0,
		       RV62_LVTMA_DIGON_OVRD | RV62_LVTMA_BLON_OVRD);
	    RHDRegMask(Output, RV620_LVTMA_PWRSEQ_REF_DIV, 3999, 0xffff); /* 4000 - 1 */
	    tmp = Private->PowerSequenceDe2Bl * 10 / 4;
	    tmp1 = Private->PowerSequenceDig2De * 10 / 4;
	    /* power sequencing delay for on / off between DIGON and SYNCEN, and SYNCEN and BLON */
	    RHDRegWrite(Output, RV620_LVTMA_PWRSEQ_DELAY1, (tmp1 << 24) | tmp1 | (tmp << 8) | (tmp << 16));
	    RHDRegWrite(Output, RV620_LVTMA_PWRSEQ_DELAY2, Private->OffDelay / 4);
	    RHDRegMask(Output, RV620_LVTMA_PWRSEQ_CNTL, 0, RV62_LVTMA_PWRSEQ_DISABLE_SYNCEN_CONTROL_OF_TX_EN);
	    for (i = 0; i < 500; i++) {
		CARD32 tmp;

		usleep(1000);
		tmp = RHDRegRead(Output, RV620_LVTMA_PWRSEQ_STATE);
		tmp >>= RV62_LVTMA_PWRSEQ_STATE_SHIFT;
		tmp &= 0xff;
		if (tmp <= RV62_POWERUP_DONE)
		    break;
		if (tmp >= RV62_POWERDOWN_DONE)
		    break;
	    }
	    /* LCD on */
	    RHDRegMask(Output, RV620_LVTMA_PWRSEQ_CNTL, RV62_LVTMA_PWRSEQ_TARGET_STATE,
		       RV62_LVTMA_PWRSEQ_TARGET_STATE);
	    return;

	case RHD_POWER_RESET:
	    /* Disable LCD and BL */
	    RHDRegMask(Output, RV620_LVTMA_PWRSEQ_CNTL, 0,
		       RV62_LVTMA_PWRSEQ_TARGET_STATE
		       | RV62_LVTMA_DIGON_OVRD
		       | RV62_LVTMA_BLON_OVRD);
	    for (i = 0; i < 500; i++) {
		CARD32 tmp;

		usleep(1000);
		tmp = RHDRegRead(Output, RV620_LVTMA_PWRSEQ_STATE);
		tmp >>= RV62_LVTMA_PWRSEQ_STATE_SHIFT;
		tmp &= 0xff;
		if (tmp >= RV62_POWERDOWN_DONE)
		    break;
	    }
	    return;
	case RHD_POWER_SHUTDOWN:
	    LVTMA_LVDSTransmitterPower(Output, RHD_POWER_RESET);
	    /* op-amp down, bias current for output driver down, shunt resistor down */
	    RHDRegWrite(Output, RV620_LVTMA_TRANSMITTER_ADJUST, 0x00e00000);
	    /* set macro control */
	    RHDRegWrite(Output, RV620_LVTMA_MACRO_CONTROL, 0x07430408);
	default:
	    return;
    }
}

/*
 *
 */
static void
LVTMA_LVDSTransmitterSave(struct rhdOutput *Output)
{
    struct DIGPrivate *digPrivate = (struct DIGPrivate *)Output->Private;
    struct LVTMATransmitterPrivate *Private = (struct LVTMATransmitterPrivate*)digPrivate->Transmitter.Private;

    RHDFUNC(Output);

    LVTMATransmitterSave(Output);

    Private->StoredPwrSeqCntl               = RHDRegRead(Output, RV620_LVTMA_PWRSEQ_CNTL);
    Private->StoredPwrSeqRevDiv             = RHDRegRead(Output, RV620_LVTMA_PWRSEQ_REF_DIV);
    Private->StoredPwrSeqDelay1             = RHDRegRead(Output, RV620_LVTMA_PWRSEQ_DELAY1);
    Private->StoredPwrSeqDelay2             = RHDRegRead(Output, RV620_LVTMA_PWRSEQ_DELAY2);

    Private->Stored = TRUE;
}

/*
 *
 */
static void
LVTMA_LVDSTransmitterRestore(struct rhdOutput *Output)
{
    struct DIGPrivate *digPrivate = (struct DIGPrivate *)Output->Private;
    struct LVTMATransmitterPrivate *Private = (struct LVTMATransmitterPrivate*)digPrivate->Transmitter.Private;

    RHDFUNC(Output);

    if (!Private->Stored) {
	xf86DrvMsg(Output->scrnIndex, X_ERROR,
		   "%s: No registers stored.\n", __func__);
	return;
    }

    LVTMATransmitterRestore(Output);

    RHDRegWrite(Output, RV620_LVTMA_PWRSEQ_REF_DIV, Private->StoredPwrSeqRevDiv);
    RHDRegWrite(Output, RV620_LVTMA_PWRSEQ_DELAY1, Private->StoredPwrSeqDelay1);
    RHDRegWrite(Output, RV620_LVTMA_PWRSEQ_DELAY2, Private->StoredPwrSeqDelay2);
    RHDRegWrite(Output, RV620_LVTMA_PWRSEQ_CNTL, Private->StoredPwrSeqCntl);
}

/*
 *
 */
static void
LVTMATransmitterDestroy(struct rhdOutput *Output)
{
    struct DIGPrivate *digPrivate = (struct DIGPrivate *)Output->Private;

    RHDFUNC(Output);

    if (!digPrivate)
	return;

    xfree(digPrivate->Transmitter.Private);
}

#if defined(ATOM_BIOS) && defined(ATOM_BIOS_PARSER)

struct ATOMTransmitterPrivate
{
    struct atomTransmitterConfig atomTransmitterConfig;
    enum atomTransmitter atomTransmitterID;
};

/*
 *
 */
static ModeStatus
ATOMTransmitterModeValid(struct rhdOutput *Output, DisplayModePtr Mode)
{

    RHDFUNC(Output);

    if (Output->Connector->Type == RHD_CONNECTOR_DVI_SINGLE
	&& Mode->SynthClock > 165000)
	return MODE_CLOCK_HIGH;

    return MODE_OK;
}

/*
 *
 */
void
rhdPrintDigDebug(RHDPtr rhdPtr, const char *name)
{
    xf86DrvMsgVerb(rhdPtr->scrnIndex, X_INFO, 7, "%s: DIGn_CNTL: n=1: 0x%x n=2: 0x%x\n",
	   name, RHDRegRead(rhdPtr, RV620_DIG1_CNTL),
	   RHDRegRead(rhdPtr, DIG2_OFFSET + RV620_DIG1_CNTL));
}

/*
 *
 */
static void
ATOMTransmitterSet(struct rhdOutput *Output, struct rhdCrtc *Crtc, DisplayModePtr Mode)
{
    RHDPtr rhdPtr = RHDPTRI(Output);
    struct DIGPrivate *Private = (struct DIGPrivate *)Output->Private;
    struct ATOMTransmitterPrivate *transPrivate
	= (struct ATOMTransmitterPrivate*) Private->Transmitter.Private;
    struct atomTransmitterConfig *atc = &transPrivate->atomTransmitterConfig;

    RHDFUNC(Output);

    atc->Coherent = Private->Coherent;
    atc->PixelClock = Mode->SynthClock;

    rhdPrintDigDebug(rhdPtr,__func__);

    if (Private->RunDualLink) {
	atc->Mode = atomDualLink;

	if (atc->Link == atomTransLinkA)
	    atc->Link = atomTransLinkAB;
	else if (atc->Link == atomTransLinkB)
	    atc->Link = atomTransLinkBA;

    } else {
	atc->Mode = atomSingleLink;

	if (atc->Link == atomTransLinkAB)
	    atc->Link = atomTransLinkA;
	else if (atc->Link == atomTransLinkBA)
	    atc->Link = atomTransLinkB;

    }

    atc->PixelClock = Mode->SynthClock;

    rhdAtomDigTransmitterControl(rhdPtr->atomBIOS, transPrivate->atomTransmitterID,
				 atomTransSetup, atc);
    rhdPrintDigDebug(rhdPtr,__func__);
}

/*
 *
 */
static CARD32
digProbeEncoder(struct rhdOutput *Output)
{
    if (Output->Id == RHD_OUTPUT_KLDSKP_LVTMA) {
	return ENCODER_DIG2;
    } else {
	Bool swap = (RHDRegRead(Output, RV620_DCIO_LINK_STEER_CNTL)
		     & RV62_LINK_STEER_SWAP) == RV62_LINK_STEER_SWAP;

	switch (Output->Id) {
	    case RHD_OUTPUT_UNIPHYA:
		if (swap) {
		    RHDDebug(Output->scrnIndex, "%s: detected ENCODER_DIG2 for UNIPHYA\n",__func__);
		    return ENCODER_DIG2;
		} else {
		    RHDDebug(Output->scrnIndex, "%s: detected ENCODER_DIG1 for UNIPHYA\n",__func__);
		    return ENCODER_DIG1;
		}
		break;
	    case RHD_OUTPUT_UNIPHYB:
		if (swap) {
		    RHDDebug(Output->scrnIndex, "%s: detected ENCODER_DIG1 for UNIPHYB\n",__func__);
		    return ENCODER_DIG1;
		} else {
		    RHDDebug(Output->scrnIndex, "%s: detected ENCODER_DIG2 for UNIPHYB\n",__func__);
		    return ENCODER_DIG2;
		}
		break;
	    default:
		return ENCODER_NONE; /* should not get here */
	}
    }
    return ENCODER_NONE;
}

/*
 *
 */
static void
ATOMTransmitterPower(struct rhdOutput *Output, int Power)
{
    RHDPtr rhdPtr = RHDPTRI(Output);
    struct DIGPrivate *Private = (struct DIGPrivate *)Output->Private;
    struct ATOMTransmitterPrivate *transPrivate
	= (struct ATOMTransmitterPrivate*) Private->Transmitter.Private;
    struct atomTransmitterConfig *atc = &transPrivate->atomTransmitterConfig;

    RHDFUNC(Output);

    rhdPrintDigDebug(rhdPtr,__func__);

    if (Private->RunDualLink)
	atc->LinkCnt = atomDualLink;
    else
	atc->LinkCnt = atomSingleLink;

    atc->Coherent = Private->Coherent;

    if (atc->Encoder == atomEncoderNone) {
	switch (digProbeEncoder(Output)) {
	    case ENCODER_DIG1:
		if (rhdPtr->DigEncoderOutput[0]) {
		    RHDDebug(Output->scrnIndex,"%s: DIG1 for %s already taken\n",__func__,Output->Name);
		    return;
		}
		atc->Encoder = atomEncoderDIG1;
		break;
	    case ENCODER_DIG2:
		if (rhdPtr->DigEncoderOutput[1]) {
		    RHDDebug(Output->scrnIndex,"%s: DIG2 for %s already taken\n",__func__,Output->Name);
		    return;
		}
		atc->Encoder = atomEncoderDIG2;
		break;
	    default:
		return;
	}
    }

    switch (Power) {
	case RHD_POWER_ON:
	    rhdAtomDigTransmitterControl(rhdPtr->atomBIOS, transPrivate->atomTransmitterID,
					 atomTransEnable, atc);
	    rhdAtomDigTransmitterControl(rhdPtr->atomBIOS, transPrivate->atomTransmitterID,
					 atomTransEnableOutput, atc);
	    break;
	case RHD_POWER_RESET:
	    rhdAtomDigTransmitterControl(rhdPtr->atomBIOS, transPrivate->atomTransmitterID,
					 atomTransDisableOutput, atc);
	    break;
	case RHD_POWER_SHUTDOWN:
	    if (!Output->Connector || Output->Connector->Type == RHD_CONNECTOR_DVI)
		atc->Mode = atomDVI;

	    rhdAtomDigTransmitterControl(rhdPtr->atomBIOS, transPrivate->atomTransmitterID,
					 atomTransDisableOutput, atc);
	    rhdAtomDigTransmitterControl(rhdPtr->atomBIOS, transPrivate->atomTransmitterID,
					 atomTransDisable, atc);
	    break;
    }
    rhdPrintDigDebug(rhdPtr,__func__);
}

/*
 *
 */
static void
ATOMTransmitterSave(struct rhdOutput *Output)
{
    RHDFUNC(Output);
}

/*
 *
 */
static void
ATOMTransmitterRestore(struct rhdOutput *Output)
{
    RHDFUNC(Output);
}

/*
 *
 */
static void
ATOMTransmitterDestroy(struct rhdOutput *Output)
{
    struct DIGPrivate *digPrivate = (struct DIGPrivate *)Output->Private;

    RHDFUNC(Output);

    if (!digPrivate)
	return;

    xfree(digPrivate->Transmitter.Private);
}

#endif /* ATOM_BIOS && ATOM_BIOS_PASER */

/*
 *  Encoder
 */

struct DIGEncoder
{
    Bool Stored;

    CARD32 StoredOff;

    CARD32 StoredRegExt1DiffPostDivCntl;
    CARD32 StoredRegExt2DiffPostDivCntl;
    CARD32 StoredDIGClockPattern;
    CARD32 StoredLVDSDataCntl;
    CARD32 StoredTMDSPixelEncoding;
    CARD32 StoredTMDSCntl;
    CARD32 StoredDIGCntl;
    CARD32 StoredDIGMisc1;
    CARD32 StoredDIGMisc2;
    CARD32 StoredDIGMisc3;
    CARD32 StoredDCCGPclkDigCntl;
    CARD32 StoredDCCGSymclkCntl;
    CARD32 StoredDCIOLinkSteerCntl;
    CARD32 StoredBlModCntl;
};

/*
 *
 */
static ModeStatus
EncoderModeValid(struct rhdOutput *Output, DisplayModePtr Mode)
{
    RHDFUNC(Output);

    return MODE_OK;
}

/*
 *
 */
static void
LVDSEncoder(struct rhdOutput *Output)
{
    struct DIGPrivate *Private = (struct DIGPrivate *)Output->Private;
    CARD32 off;

    RHDFUNC(Output);


    off = (Private->EncoderID == ENCODER_DIG2) ? DIG2_OFFSET : DIG1_OFFSET;
    /* Clock pattern ? */
    RHDRegMask(Output, off + RV620_DIG1_CLOCK_PATTERN, 0x0063, 0xFFFF);
    /* set panel type: 18/24 bit mode */
    RHDRegMask(Output, off + RV620_LVDS1_DATA_CNTL,
	       (Private->FMTDither.LVDS24Bit ? RV62_LVDS_24BIT_ENABLE : 0)
	       | (Private->FPDI ? RV62_LVDS_24BIT_FORMAT : 0),
	       RV62_LVDS_24BIT_ENABLE | RV62_LVDS_24BIT_FORMAT);

    Output->Crtc->FMTModeSet(Output->Crtc, &Private->FMTDither);
}

/*
 *
 */
static void
TMDSEncoder(struct rhdOutput *Output)
{
    struct DIGPrivate *Private = (struct DIGPrivate *)Output->Private;
    CARD32 off;

    RHDFUNC(Output);

    off = (Private->EncoderID == ENCODER_DIG2) ? DIG2_OFFSET : DIG1_OFFSET;
    /* clock pattern ? */
    RHDRegMask(Output, off + RV620_DIG1_CLOCK_PATTERN, 0x001F, 0xFFFF);
    /* color format RGB - normal color format 24bpp, Twin-Single 30bpp or Dual 48bpp*/
    RHDRegMask(Output, off + RV620_TMDS1_CNTL, 0x0,
	       RV62_TMDS_PIXEL_ENCODING | RV62_TMDS_COLOR_FORMAT);
    /* no dithering */
    Output->Crtc->FMTModeSet(Output->Crtc, NULL);
}

/*
 *
 */
static void
EncoderSet(struct rhdOutput *Output, struct rhdCrtc *Crtc, DisplayModePtr Mode)
{
    struct DIGPrivate *Private = (struct DIGPrivate *)Output->Private;
    RHDPtr rhdPtr = RHDPTRI(Output);
    CARD32 off;

    RHDFUNC(Output);

    off = (Private->EncoderID == ENCODER_DIG2) ? DIG2_OFFSET : DIG1_OFFSET;

    rhdPrintDigDebug(rhdPtr,__func__);

    RHDRegMask(Output, off + RV620_DIG1_CNTL, Output->Crtc->Id,
	       RV62_DIG_SOURCE_SELECT);

    if (Output->Id == RHD_OUTPUT_UNIPHYA) {
	/* select LinkA ?? */
	RHDRegMask(Output, RV620_DCIO_LINK_STEER_CNTL,
		   ((Private->EncoderID == ENCODER_DIG2)
		    ? RV62_LINK_STEER_SWAP
		    : 0), RV62_LINK_STEER_SWAP); /* swap if DIG2 */
	if (!Private->RunDualLink) {
	    RHDRegMask(Output, off + RV620_DIG1_CNTL,
		       0,
		       RV62_DIG_SWAP |RV62_DIG_DUAL_LINK_ENABLE);
	} else {
	    RHDRegMask(Output, off + RV620_DIG1_CNTL,
		       RV62_DIG_DUAL_LINK_ENABLE,
		       RV62_DIG_SWAP | RV62_DIG_DUAL_LINK_ENABLE);
	}
    } else if (Output->Id == RHD_OUTPUT_UNIPHYB) {
	/* select LinkB ?? */
	RHDRegMask(Output, RV620_DCIO_LINK_STEER_CNTL,
		   ((Private->EncoderID == ENCODER_DIG2)
		    ? 0
		    : RV62_LINK_STEER_SWAP), RV62_LINK_STEER_SWAP);
	if (!Private->RunDualLink)
	    RHDRegMask(Output, off + RV620_DIG1_CNTL,
		       0,
		       RV62_DIG_SWAP |  RV62_DIG_DUAL_LINK_ENABLE);
	 else
	    RHDRegMask(Output, off + RV620_DIG1_CNTL,
		       RV62_DIG_SWAP | RV62_DIG_DUAL_LINK_ENABLE,
		       RV62_DIG_SWAP | RV62_DIG_DUAL_LINK_ENABLE);
    } else { /* LVTMA */
	RHDRegMask(Output, RV620_EXT2_DIFF_POST_DIV_CNTL, 0, RV62_EXT2_DIFF_DRIVER_ENABLE);
    }

    if (Private->EncoderMode == LVDS)
	LVDSEncoder(Output);
    else if (Private->EncoderMode == DISPLAYPORT)
      dbgprintf("No displayport support yet!",__FILE__, __LINE__, __func__);  /* bugger ! */
    else
	TMDSEncoder(Output);

    /* Start DIG, set links, disable stereo sync, select FMT source */
    RHDRegMask(Output, off + RV620_DIG1_CNTL,
	       (Private->EncoderMode & 0x7) << 8
	       | RV62_DIG_START
	       | (Private->RunDualLink ? RV62_DIG_DUAL_LINK_ENABLE : 0)
	       | Output->Crtc->Id,
	       RV62_DIG_MODE
	       | RV62_DIG_START
	       | RV62_DIG_DUAL_LINK_ENABLE
	       | RV62_DIG_STEREOSYNC_SELECT
	       | RV62_DIG_SOURCE_SELECT);
    rhdPrintDigDebug(rhdPtr,__func__);
}

/*
 *
 */
static void
EncoderPower(struct rhdOutput *Output, int Power)
{
    struct DIGPrivate *Private = (struct DIGPrivate *)Output->Private;
    CARD32 off;
    enum encoderID EncoderID = Private->EncoderID;
    RHDPtr rhdPtr = Output->rhdPtr;

    RHDFUNC(Output);

    if (EncoderID == ENCODER_NONE) {
	EncoderID = digProbeEncoder(Output);
	switch (EncoderID) {
	    case ENCODER_DIG1:
		if (rhdPtr->DigEncoderOutput[0]) {
		    RHDDebug(Output->scrnIndex,"%s: DIG1 for %s already taken\n",__func__,Output->Name);
		    return;
		}
		break;
	    case ENCODER_DIG2:
		if (rhdPtr->DigEncoderOutput[1]) {
		    RHDDebug(Output->scrnIndex,"%s: DIG2 for %s already taken\n",__func__,Output->Name);
		    return;
		}
		break;
	    default:
		return;
	}
    }

    off = (EncoderID == ENCODER_DIG2) ? DIG2_OFFSET : DIG1_OFFSET;

    /* clock src is pixel PLL */
    RHDRegMask(Output, RV620_DCCG_SYMCLK_CNTL, 0x0,
	       0x3 << ((EncoderID == ENCODER_DIG2)
		       ? RV62_SYMCLKB_SRC_SHIFT
		       : RV62_SYMCLKA_SRC_SHIFT));

    rhdPrintDigDebug(rhdPtr,__func__);
    switch (Power) {
	case RHD_POWER_ON:
	    RHDDebug(Output->scrnIndex,"%s(RHD_POWER_ON, %i)\n",__func__,
		     EncoderID);
	    /* enable DIG */
	    RHDRegMask(Output, off + RV620_DIG1_CNTL, 0x10, 0x10);
	    RHDRegMask(Output, (EncoderID == ENCODER_DIG2)
		       ? RV620_DCCG_PCLK_DIGB_CNTL
		       : RV620_DCCG_PCLK_DIGA_CNTL,
		       RV62_PCLK_DIGA_ON, RV62_PCLK_DIGA_ON); /* @@@ */
	    rhdPrintDigDebug(rhdPtr,__func__);
	    return;
	case RHD_POWER_RESET:
	case RHD_POWER_SHUTDOWN:
	default:
	    RHDDebug(Output->scrnIndex,"%s(RHD_POWER_SHUTDOWN, %i)\n",__func__,
		     EncoderID);
	    /* disable differential clock driver */
	    if (EncoderID == ENCODER_DIG1)
		RHDRegMask(Output, RV620_EXT1_DIFF_POST_DIV_CNTL,
			   0,
			   RV62_EXT1_DIFF_DRIVER_ENABLE);
	    else
		RHDRegMask(Output, RV620_EXT2_DIFF_POST_DIV_CNTL,
			   0,
			   RV62_EXT2_DIFF_DRIVER_ENABLE);
	    /* disable DIG */
	    RHDRegMask(Output, off + RV620_DIG1_CNTL, 0x0, 0x10);
	    RHDRegMask(Output, (EncoderID == ENCODER_DIG2)
		       ? RV620_DCCG_PCLK_DIGB_CNTL
		       : RV620_DCCG_PCLK_DIGA_CNTL,
		       0, RV62_PCLK_DIGA_ON); /* @@@ */
	    rhdPrintDigDebug(rhdPtr,__func__);
	    return;
    }
}

/*
 *
 */
static void
EncoderSave(struct rhdOutput *Output)
{
    struct DIGPrivate *digPrivate = (struct DIGPrivate *)Output->Private;
    struct DIGEncoder *Private = (struct DIGEncoder *)(digPrivate->Encoder.Private);
    CARD32 off;
    enum encoderID EncoderID;

    RHDFUNC(Output);

    EncoderID = digProbeEncoder(Output);
    off = (EncoderID == ENCODER_DIG2) ? DIG2_OFFSET : DIG1_OFFSET;
    Private->StoredOff = off;

    Private->StoredRegExt1DiffPostDivCntl          = RHDRegRead(Output, off + RV620_EXT1_DIFF_POST_DIV_CNTL);
    Private->StoredRegExt2DiffPostDivCntl          = RHDRegRead(Output, off + RV620_EXT2_DIFF_POST_DIV_CNTL);
    Private->StoredDIGClockPattern = RHDRegRead(Output, off + RV620_DIG1_CLOCK_PATTERN);
    Private->StoredLVDSDataCntl    = RHDRegRead(Output, off + RV620_LVDS1_DATA_CNTL);
    Private->StoredDIGCntl         = RHDRegRead(Output, off + RV620_DIG1_CNTL);
    Private->StoredTMDSCntl        = RHDRegRead(Output, off + RV620_TMDS1_CNTL);
    Private->StoredDCIOLinkSteerCntl = RHDRegRead(Output, RV620_DCIO_LINK_STEER_CNTL);
    Private->StoredDCCGPclkDigCntl    = RHDRegRead(Output,
						   (off == DIG2_OFFSET)
						   ? RV620_DCCG_PCLK_DIGB_CNTL
						   : RV620_DCCG_PCLK_DIGA_CNTL);
    Private->StoredDCCGSymclkCntl     = RHDRegRead(Output, RV620_DCCG_SYMCLK_CNTL);
    Private->StoredBlModCntl          = RHDRegRead(Output, RV620_LVTMA_BL_MOD_CNTL);

    Private->Stored = TRUE;
}

/*
 *
 */
static void
EncoderRestore(struct rhdOutput *Output)
{
    struct DIGPrivate *digPrivate = (struct DIGPrivate *)Output->Private;
    struct DIGEncoder *Private = (struct DIGEncoder *)(digPrivate->Encoder.Private);
    CARD32 off;

    RHDFUNC(Output);

    if (!Private->Stored) {
	xf86DrvMsg(Output->scrnIndex, X_ERROR,
		   "%s: No registers stored.\n", __func__);
	return;
    }
    off = Private->StoredOff;

    RHDRegWrite(Output, off + RV620_EXT1_DIFF_POST_DIV_CNTL, Private->StoredRegExt1DiffPostDivCntl);
    RHDRegWrite(Output, off + RV620_EXT2_DIFF_POST_DIV_CNTL, Private->StoredRegExt2DiffPostDivCntl);
    /* reprogram all values but don't start the encoder, yet */
    RHDRegWrite(Output, off + RV620_DIG1_CNTL, Private->StoredDIGCntl & ~(CARD32)RV62_DIG_START);
    RHDRegWrite(Output, RV620_DCIO_LINK_STEER_CNTL, Private->StoredDCIOLinkSteerCntl);
    RHDRegWrite(Output, off + RV620_DIG1_CLOCK_PATTERN, Private->StoredDIGClockPattern);
    RHDRegWrite(Output, off + RV620_LVDS1_DATA_CNTL, Private->StoredLVDSDataCntl);
    RHDRegWrite(Output, off + RV620_TMDS1_CNTL, Private->StoredTMDSCntl);
    RHDRegWrite(Output, (off == DIG2_OFFSET)
		? RV620_DCCG_PCLK_DIGB_CNTL
		: RV620_DCCG_PCLK_DIGA_CNTL,
		Private->StoredDCCGPclkDigCntl);
    /* now enable the encoder */
    RHDRegWrite(Output, off + RV620_DIG1_CNTL, Private->StoredDIGCntl);
    RHDRegWrite(Output, RV620_DCCG_SYMCLK_CNTL, Private->StoredDCCGSymclkCntl);
    RHDRegWrite(Output, RV620_LVTMA_BL_MOD_CNTL, Private->StoredBlModCntl);
}

/*
 *
 */
static void
EncoderDestroy(struct rhdOutput *Output)
{
    struct DIGPrivate *digPrivate = (struct DIGPrivate *)Output->Private;

    RHDFUNC(Output);

    if (!digPrivate || !digPrivate->Encoder.Private)
	return;

    xfree(digPrivate->Encoder.Private);
}

/*
 * Housekeeping
 */
void
GetLVDSInfo(RHDPtr rhdPtr, struct DIGPrivate *Private)
{
    CARD32 off = (Private->EncoderID == ENCODER_DIG2) ? DIG2_OFFSET : DIG1_OFFSET;
    CARD32 tmp;

    RHDFUNC(rhdPtr);

    Private->FPDI = ((RHDRegRead(rhdPtr, off + RV620_LVDS1_DATA_CNTL)
				 & RV62_LVDS_24BIT_FORMAT) != 0);
    Private->RunDualLink = ((RHDRegRead(rhdPtr, off + RV620_DIG1_CNTL)
				 & RV62_DIG_DUAL_LINK_ENABLE) != 0);
    Private->FMTDither.LVDS24Bit = ((RHDRegRead(rhdPtr, off  + RV620_LVDS1_DATA_CNTL)
			   & RV62_LVDS_24BIT_ENABLE) != 0);

    tmp = RHDRegRead(rhdPtr, RV620_LVTMA_BL_MOD_CNTL);
    if (tmp & 0x1)
	Private->BlLevel = ( tmp >> LVTMA_BL_MOD_LEVEL_SHIFT )  & 0xff;
    else
	Private->BlLevel = -1;

    tmp = RHDRegRead(rhdPtr, RV620_LVTMA_PWRSEQ_REF_DIV);
    tmp &= 0xffff;
    tmp += 1;
    tmp /= 1000;
    Private->PowerSequenceDig2De = Private->PowerSequenceDe2Bl =
	RHDRegRead(rhdPtr, RV620_LVTMA_PWRSEQ_REF_DIV);
    Private->PowerSequenceDig2De = ((Private->PowerSequenceDig2De & 0xff) * tmp) / 10;
    Private->PowerSequenceDe2Bl = (((Private->PowerSequenceDe2Bl >> 8) & 0xff) * tmp) / 10;
    Private->OffDelay = RHDRegRead(rhdPtr, RV620_LVTMA_PWRSEQ_DELAY2);
    Private->OffDelay *= tmp;

    /* This is really ugly! */
    {
	CARD32 fmt_offset;

	tmp = RHDRegRead(rhdPtr, off + RV620_DIG1_CNTL);
	fmt_offset = (tmp & RV62_DIG_SOURCE_SELECT) ? FMT2_OFFSET :0;
	tmp = RHDRegRead(rhdPtr, fmt_offset + RV620_FMT1_BIT_DEPTH_CONTROL);
	Private->FMTDither.LVDSSpatialDither = ((tmp & 0x100) != 0);
	Private->FMTDither.LVDSGreyLevel = ((tmp & 0x10000) != 0);
	Private->FMTDither.LVDSTemporalDither
	    = (Private->FMTDither.LVDSGreyLevel > 0) || ((tmp & 0x1000000) != 0);
    }

#ifdef ATOM_BIOS
    {
	AtomBiosArgRec data;

    if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
				 ATOM_LVDS_FPDI, &data) == ATOM_SUCCESS)
	    Private->FPDI = data.val;

    if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			    ATOM_LVDS_DUALLINK, &data) == ATOM_SUCCESS)
	    Private->RunDualLink = data.val;

    if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			    ATOM_LVDS_GREYLVL, &data) == ATOM_SUCCESS)
	    Private->FMTDither.LVDSGreyLevel = data.val;

    if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			    ATOM_LVDS_SEQ_DIG_ONTO_DE, &data) == ATOM_SUCCESS)
	    Private->PowerSequenceDig2De = data.val;

    if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			    ATOM_LVDS_SEQ_DE_TO_BL, &data) == ATOM_SUCCESS)
	    Private->PowerSequenceDe2Bl = data.val;

    if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			    ATOM_LVDS_OFF_DELAY, &data) == ATOM_SUCCESS)
	    Private->OffDelay = data.val;

    if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			    ATOM_LVDS_24BIT, &data) == ATOM_SUCCESS)
	    Private->FMTDither.LVDS24Bit = data.val;

    if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			    ATOM_LVDS_SPATIAL_DITHER, &data) == ATOM_SUCCESS)
	    Private->FMTDither.LVDSSpatialDither = data.val;

    if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			    ATOM_LVDS_TEMPORAL_DITHER, &data) == ATOM_SUCCESS)
	    Private->FMTDither.LVDSTemporalDither = data.val;

	Private->PowerSequenceDe2Bl = data.val;

    }
#endif

}

/*
 * Infrastructure
 */

static ModeStatus
DigModeValid(struct rhdOutput *Output, DisplayModePtr Mode)
{
    struct DIGPrivate *Private = (struct DIGPrivate *)Output->Private;
    struct transmitter *Transmitter = &Private->Transmitter;
    struct encoder *Encoder = &Private->Encoder;
    ModeStatus Status;

    RHDFUNC(Output);

    if ((Status = Transmitter->ModeValid(Output, Mode)) == MODE_OK)
	return ((Encoder->ModeValid(Output, Mode)));
    else
	return Status;
}

/*
 *
 */
static void
DigPower(struct rhdOutput *Output, int Power)
{
    struct DIGPrivate *Private = (struct DIGPrivate *)Output->Private;
    struct transmitter *Transmitter = &Private->Transmitter;
    struct encoder *Encoder = &Private->Encoder;
    Bool enableHDMI;

    RHDDebug(Output->scrnIndex, "%s(%s,%s)\n",__func__,Output->Name,
	     rhdPowerString[Power]);

    if(Output->Connector != NULL) {
	/* check if attached monitor supports HDMI */
	enableHDMI = RHDConnectorEnableHDMI(Output->Connector);
	if (enableHDMI && Private->EncoderMode == TMDS_DVI)
	    Private->EncoderMode = TMDS_HDMI;
	else if (!enableHDMI && Private->EncoderMode == TMDS_HDMI)
	    Private->EncoderMode = TMDS_DVI;
    }

    switch (Power) {
	case RHD_POWER_ON:
	    Encoder->Power(Output, Power);
	    Transmitter->Power(Output, Power);
	    RHDHdmiEnable(Private->Hdmi, Private->EncoderMode == TMDS_HDMI);
	    return;
	case RHD_POWER_RESET:
	    Transmitter->Power(Output, Power);
	    Encoder->Power(Output, Power);
	    return;
	case RHD_POWER_SHUTDOWN:
	default:
	    Transmitter->Power(Output, Power);
	    Encoder->Power(Output, Power);
	    RHDHdmiEnable(Private->Hdmi, FALSE);
	    return;
    }
}

/*
 *
 */
static Bool
DigPropertyControl(struct rhdOutput *Output,
			      enum rhdPropertyAction Action, enum rhdOutputProperty Property, union rhdPropertyData *val)
{
    struct DIGPrivate *Private = (struct DIGPrivate *)Output->Private;

    RHDFUNC(Output);

    switch(Property) {
	case RHD_OUTPUT_COHERENT:
	case RHD_OUTPUT_BACKLIGHT:
	{
	    if (!Private->Transmitter.Property)
		return FALSE;
	    Private->Transmitter.Property(Output, Action, Property, val);
	    break;
	}
	default:
	    return FALSE;
    }
    return TRUE;
}


/*
 *
 */
static void
DigMode(struct rhdOutput *Output, DisplayModePtr Mode)
{
    struct DIGPrivate *Private = (struct DIGPrivate *)Output->Private;
    struct transmitter *Transmitter = &Private->Transmitter;
    struct encoder *Encoder = &Private->Encoder;
    struct rhdCrtc *Crtc = Output->Crtc;

    RHDFUNC(Output);

    /*
     * For dual link capable DVI we need to decide from the pix clock if we are dual link.
     * Do it here as it is convenient.
     */
    if (Output->Connector->Type == RHD_CONNECTOR_DVI)
	Private->RunDualLink = (Mode->SynthClock > 165000) ? TRUE : FALSE;

    Encoder->Mode(Output, Crtc, Mode);
    Transmitter->Mode(Output, Crtc, Mode);
    RHDHdmiSetMode(Private->Hdmi, Mode);
}

/*
 *
 */
static void
DigSave(struct rhdOutput *Output)
{
    struct DIGPrivate *Private = (struct DIGPrivate *)Output->Private;
    struct transmitter *Transmitter = &Private->Transmitter;
    struct encoder *Encoder = &Private->Encoder;

    RHDFUNC(Output);

    Encoder->Save(Output);
    Transmitter->Save(Output);
    RHDHdmiSave(Private->Hdmi);
}

/*
 *
 */
static void
DigRestore(struct rhdOutput *Output)
{
    struct DIGPrivate *Private = (struct DIGPrivate *)Output->Private;
    struct transmitter *Transmitter = &Private->Transmitter;
    struct encoder *Encoder = &Private->Encoder;

    RHDFUNC(Output);

    Encoder->Restore(Output);
    Transmitter->Restore(Output);
    RHDHdmiRestore(Private->Hdmi);
}

/*
 *
 */
static void
DigDestroy(struct rhdOutput *Output)
{
    struct DIGPrivate *Private = (struct DIGPrivate *)Output->Private;
    struct transmitter *Transmitter = &Private->Transmitter;
    struct encoder *Encoder = &Private->Encoder;

    RHDFUNC(Output);

    Encoder->Destroy(Output);
    Transmitter->Destroy(Output);
    RHDHdmiDestroy(Private->Hdmi);
#ifdef NOT_YET
    if (Transmitter->PropertyPrivate)
	RhdAtomDestroyBacklightControlProperty(Output, Transmitter->PropertyPrivate);
#endif
    xfree(Private);
    Output->Private = NULL;
}

/*
 *
 */
static Bool
DigAllocFree(struct rhdOutput *Output, enum rhdOutputAllocation Alloc)
{
    struct DIGPrivate *Private = (struct DIGPrivate *)Output->Private;
    RHDPtr rhdPtr = RHDPTRI(Output);
    char *TransmitterName;

    RHDFUNC(rhdPtr);

    switch (Output->Id) {
	case RHD_OUTPUT_KLDSKP_LVTMA:
	    TransmitterName = "KLDSKP_LVTMA";
	    break;
	case RHD_OUTPUT_UNIPHYA:
	    TransmitterName = "UNIPHYA";
	    break;
	case RHD_OUTPUT_UNIPHYB:
	    TransmitterName = "UNIPHYB";
	    break;
	default:
	    return FALSE;
    }
    switch (Alloc) {
	case RHD_OUTPUT_ALLOC:

	    if (Private->EncoderID != ENCODER_NONE)
		return TRUE;

	    /*
	     * LVTMA can only use DIG2. Thus exclude
	     * DIG1 for LVTMA and prefer it for the
	     * UNIPHYs.
	     */
	    if (Output->Id == RHD_OUTPUT_KLDSKP_LVTMA) {
		if (!rhdPtr->DigEncoderOutput[1]) {
		    rhdPtr->DigEncoderOutput[1] = Output;
		    Private->EncoderID = ENCODER_DIG2;
		    xf86DrvMsg(Output->scrnIndex, X_INFO,
			       "Mapping DIG2 encoder to %s\n",TransmitterName);
		return TRUE;
		} else
		    return FALSE;
	    } else {
		struct ATOMTransmitterPrivate *transPrivate =
		    (struct ATOMTransmitterPrivate *)Private->Transmitter.Private;
		struct atomTransmitterConfig *atc = &transPrivate->atomTransmitterConfig;
		if (!rhdPtr->DigEncoderOutput[0]) {
		    rhdPtr->DigEncoderOutput[0] = Output;
		    Private->EncoderID = ENCODER_DIG1;
		    atc->Encoder = atomEncoderDIG1;
		    xf86DrvMsg(Output->scrnIndex, X_INFO,
			       "Mapping DIG1 encoder to %s\n",TransmitterName);
		    return TRUE;
		} else if (!rhdPtr->DigEncoderOutput[1]) {
		    rhdPtr->DigEncoderOutput[1] = Output;
		    Private->EncoderID = ENCODER_DIG2;
		    atc->Encoder = atomEncoderDIG2;
		    xf86DrvMsg(Output->scrnIndex, X_INFO,
			       "Mapping DIG2 encoder to %s\n",TransmitterName);
		    return TRUE;
		} else
		    return FALSE;
	    }

	case RHD_OUTPUT_FREE:
		Private->EncoderID = ENCODER_NONE;
	    if (rhdPtr->DigEncoderOutput[0] == Output) {
		rhdPtr->DigEncoderOutput[0] = NULL;
		return TRUE;
	    } else if (rhdPtr->DigEncoderOutput[1] == Output) {
		rhdPtr->DigEncoderOutput[1] = NULL;
		return TRUE;
	    } else
		return FALSE;
	    break;
	default:
	    return FALSE;
    }
}

/*
 *
 */
static Bool
rhdDIGSetCoherent(RHDPtr rhdPtr,struct rhdOutput *Output)
{
    Bool coherent = FALSE;
//    int  from = X_CONFIG;

//    switch (RhdParseBooleanOption(&rhdPtr->coherent, Output->Name)) {
//    case RHD_OPTION_NOT_SET:
//    case RHD_OPTION_DEFAULT:
//        from = X_DEFAULT;
//        coherent = FALSE;
//        break;
//    case RHD_OPTION_ON:
//        coherent = TRUE;
//        break;
//    case RHD_OPTION_OFF:
//        coherent = FALSE;
//        break;
//    }
//    xf86DrvMsg(rhdPtr->scrnIndex,from,"Setting %s to %scoherent\n",
//           Output->Name,coherent ? "" : "in");

    return coherent;
}

/*
 *
 */
#ifdef NOT_YET
static Bool
digTransmitterPropertyWrapper(struct rhdOutput *Output,
			      enum rhdPropertyAction Action,
			      enum rhdOutputProperty Property,
			      union rhdPropertyData *val)
{
    struct DIGPrivate *Private = (struct DIGPrivate *)Output->Private;
    void *storePrivate = Output->Private;
    Bool (*func)(struct rhdOutput *,enum rhdPropertyAction, enum rhdOutputProperty,
		  union rhdPropertyData *) = Private->Transmitter.WrappedPropertyCallback;
    Bool ret;

    Output->Private = Private->Transmitter.PropertyPrivate;
    ret = func(Output, Action, Property, val);
    Output->Private = storePrivate;

    return ret;
}
#endif

/*
 *
 */
struct rhdOutput *
RHDDIGInit(RHDPtr rhdPtr,  enum rhdOutputType outputType, CARD8 ConnectorType)
{
    struct rhdOutput *Output;
    struct DIGPrivate *Private;
    struct DIGEncoder *Encoder;

    RHDFUNC(rhdPtr);

    Output = xnfcalloc(sizeof(struct rhdOutput), 1);

    Output->scrnIndex = rhdPtr->scrnIndex;
    Output->Id = outputType;

    Output->Sense = NULL;
    Output->ModeValid = DigModeValid;
    Output->Mode = DigMode;
    Output->Power = DigPower;
    Output->Save = DigSave;
    Output->Restore = DigRestore;
    Output->Destroy = DigDestroy;
    Output->Property = DigPropertyControl;
    Output->AllocFree = DigAllocFree;

    Private = xnfcalloc(sizeof(struct DIGPrivate), 1);
    Output->Private = Private;

    Private->EncoderID = ENCODER_NONE;

    switch (outputType) {
	case RHD_OUTPUT_UNIPHYA:
#if defined (ATOM_BIOS) && defined (ATOM_BIOS_PARSER)
	    Output->Name = "UNIPHY_A";
	    Private->Transmitter.Private =
		(struct ATOMTransmitterPrivate *)xnfcalloc(sizeof (struct ATOMTransmitterPrivate), 1);

	    Private->Transmitter.Sense = NULL;
	    Private->Transmitter.ModeValid = ATOMTransmitterModeValid;
	    Private->Transmitter.Mode = ATOMTransmitterSet;
	    Private->Transmitter.Power = ATOMTransmitterPower;
	    Private->Transmitter.Save = ATOMTransmitterSave;
	    Private->Transmitter.Restore = ATOMTransmitterRestore;
	    Private->Transmitter.Destroy = ATOMTransmitterDestroy;
	    Private->Transmitter.Property = TMDSTransmitterPropertyControl;
	    {
		struct ATOMTransmitterPrivate *transPrivate =
		    (struct ATOMTransmitterPrivate *)Private->Transmitter.Private;
		struct atomTransmitterConfig *atc = &transPrivate->atomTransmitterConfig;
		atc->Coherent = Private->Coherent = rhdDIGSetCoherent(rhdPtr, Output);
		atc->Link = atomTransLinkA;
		atc->Encoder = atomEncoderNone;
		if (RHDIsIGP(rhdPtr->ChipSet)) {
		    AtomBiosArgRec data;
		    data.val = 1;
            if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS, ATOM_GET_PCIE_LANES,
					&data) == ATOM_SUCCESS)
			atc->Lanes = data.pcieLanes.Chassis; /* only do 'chassis' for now */
		    else {
			xfree(Private);
			xfree(Output);
			return NULL;
		    }
		}
		if (RHDIsIGP(rhdPtr->ChipSet))
		    transPrivate->atomTransmitterID = atomTransmitterPCIEPHY;
		else
		    transPrivate->atomTransmitterID = atomTransmitterUNIPHY;
	    }
	    break;
#else
	    xfree(Private);
	    xfree(Output);
	    return NULL;
#endif /* ATOM_BIOS && ATOM_BIOS_PARSER */

	case RHD_OUTPUT_UNIPHYB:
#if defined (ATOM_BIOS) && defined (ATOM_BIOS_PARSER)
	    Output->Name = "UNIPHY_B";
	    Private->Transmitter.Private =
		(struct atomTransmitterPrivate *)xnfcalloc(sizeof (struct ATOMTransmitterPrivate), 1);

	    Private->Transmitter.Sense = NULL;
	    Private->Transmitter.ModeValid = ATOMTransmitterModeValid;
	    Private->Transmitter.Mode = ATOMTransmitterSet;
	    Private->Transmitter.Power = ATOMTransmitterPower;
	    Private->Transmitter.Save = ATOMTransmitterSave;
	    Private->Transmitter.Restore = ATOMTransmitterRestore;
	    Private->Transmitter.Destroy = ATOMTransmitterDestroy;
	    Private->Transmitter.Property = TMDSTransmitterPropertyControl;
	    {
		struct ATOMTransmitterPrivate *transPrivate =
		    (struct ATOMTransmitterPrivate *)Private->Transmitter.Private;
		struct atomTransmitterConfig *atc = &transPrivate->atomTransmitterConfig;
		atc->Coherent = Private->Coherent = rhdDIGSetCoherent(rhdPtr, Output);
		atc->Link = atomTransLinkB;
		atc->Encoder = atomEncoderNone;
		if (RHDIsIGP(rhdPtr->ChipSet)) {
		    AtomBiosArgRec data;
		    data.val = 2;
		    if (RHDAtomBiosFunc(rhdPtr->scrnIndex, rhdPtr->atomBIOS, ATOM_GET_PCIE_LANES,
					&data) == ATOM_SUCCESS)
			atc->Lanes = data.pcieLanes.Chassis; /* only do 'chassis' for now */
		    else {
			xfree(Private);
			xfree(Output);
			return NULL;
		    }
		}
		if (RHDIsIGP(rhdPtr->ChipSet))
		    transPrivate->atomTransmitterID = atomTransmitterPCIEPHY;
		else
		    transPrivate->atomTransmitterID = atomTransmitterUNIPHY;
	    }
	    break;
#else
	    xfree(Private);
	    xfree(Output);
	    return NULL;
#endif /* ATOM_BIOS && ATOM_BIOS_PARSER */

	case RHD_OUTPUT_KLDSKP_LVTMA:
	    Output->Name = "UNIPHY_KLDSKP_LVTMA";
	    Private->Coherent = rhdDIGSetCoherent(rhdPtr, Output);
	    Private->Transmitter.Private =
		(struct LVTMATransmitterPrivate *)xnfcalloc(sizeof (struct LVTMATransmitterPrivate), 1);

	    Private->Transmitter.Sense = NULL;
	    Private->Transmitter.ModeValid = LVTMATransmitterModeValid;
	    if (ConnectorType != RHD_CONNECTOR_PANEL) {
		Private->Transmitter.Mode = LVTMA_TMDSTransmitterSet;
		Private->Transmitter.Power = LVTMA_TMDSTransmitterPower;
		Private->Transmitter.Save = LVTMA_TMDSTransmitterSave;
		Private->Transmitter.Restore = LVTMA_TMDSTransmitterRestore;
	    } else {
		Private->Transmitter.Mode = LVTMA_LVDSTransmitterSet;
		Private->Transmitter.Power = LVTMA_LVDSTransmitterPower;
		Private->Transmitter.Save = LVTMA_LVDSTransmitterSave;
		Private->Transmitter.Restore = LVTMA_LVDSTransmitterRestore;
	    }
	    Private->Transmitter.Destroy = LVTMATransmitterDestroy;
	    if (ConnectorType == RHD_CONNECTOR_PANEL)
		Private->Transmitter.Property = LVDSTransmitterPropertyControl;
	    else
		Private->Transmitter.Property = TMDSTransmitterPropertyControl;
	    break;

	default:
	    xfree(Private);
	    xfree(Output);
	    return NULL;
    }

    Encoder = (struct DIGEncoder *)(xnfcalloc(sizeof (struct DIGEncoder),1));
    Private->Encoder.Private = Encoder;
    Private->Encoder.ModeValid = EncoderModeValid;
    Private->Encoder.Mode = EncoderSet;
    Private->Encoder.Power = EncoderPower;
    Private->Encoder.Save = EncoderSave;
    Private->Encoder.Restore = EncoderRestore;
    Private->Encoder.Destroy = EncoderDestroy;

    switch (ConnectorType) {
	case RHD_CONNECTOR_PANEL:
	    Private->EncoderMode = LVDS;
	    GetLVDSInfo(rhdPtr, Private);
#ifdef ATOM_BIOS
#ifdef NOT_YET
	    if (Private->BlLevel < 0) {
		Private->BlLevel = RhdAtomSetupBacklightControlProperty(Output,
									&Private->Transmitter.WrappedPropertyCallback,
									&Private->Transmitter.PropertyPrivate);
		if (Private->Transmitter.PropertyPrivate)
		    Private->Transmitter.Property = digTransmitterPropertyWrapper;
	    }
#endif
#endif
	    Private->Hdmi = NULL;
	    break;
	case RHD_CONNECTOR_DVI:
	    Private->RunDualLink = FALSE; /* will be set later acc to pxclk */
	    Private->EncoderMode = TMDS_DVI;
	    Private->Hdmi = RHDHdmiInit(rhdPtr, Output);
	    break;
	case RHD_CONNECTOR_DVI_SINGLE:
	    Private->RunDualLink = FALSE;
	    Private->EncoderMode = TMDS_DVI; /* changed later to HDMI if aplicateable */
	    Private->Hdmi = RHDHdmiInit(rhdPtr, Output);
	    break;
    }

    return Output;
}
