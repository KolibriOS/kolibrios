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
 * Deals with the Primary TMDS device (TMDSA) of R500s, R600s.
 * Gets replaced by DDIA on RS690 and DIG/UNIPHY on RV620.
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
#endif

struct rhdTMDSPrivate {
    Bool RunsDualLink;
    DisplayModePtr Mode;
    Bool Coherent;
    int PowerState;

    struct rhdHdmi *Hdmi;

    Bool Stored;

    CARD32 StoreControl;
    CARD32 StoreSource;
    CARD32 StoreFormat;
    CARD32 StoreForce;
    CARD32 StoreReduction;
    CARD32 StoreDCBalancer;
    CARD32 StoreDataSynchro;
    CARD32 StoreTXEnable;
    CARD32 StoreMacro;
    CARD32 StoreTXControl;
    CARD32 StoreTXAdjust;
};

/*
 * We cannot sense for dual link here at all, plus, we need a bit more work
 * for enabling the transmitter for sensing to happen on most R5xx cards.
 * RV570 (0x7280) and R600 and above seem ok.
 */
static enum rhdSensedOutput
TMDSASense(struct rhdOutput *Output, struct rhdConnector *Connector)
{
    RHDPtr rhdPtr = RHDPTRI(Output);
    CARD32 Enable, Control, Detect;
    enum rhdConnectorType Type = Connector->Type;
    Bool ret;

    RHDFUNC(Output);

    if ((Type != RHD_CONNECTOR_DVI) && (Type != RHD_CONNECTOR_DVI_SINGLE)) {
	xf86DrvMsg(Output->scrnIndex, X_WARNING,
		   "%s: connector type %d is not supported.\n",
		   __func__, Type);
	return RHD_SENSED_NONE;
    }

    Enable = RHDRegRead(Output, TMDSA_TRANSMITTER_ENABLE);
    Control = RHDRegRead(Output, TMDSA_TRANSMITTER_CONTROL);
    Detect = RHDRegRead(Output, TMDSA_LOAD_DETECT);

    if (rhdPtr->ChipSet < RHD_R600) {
	RHDRegMask(Output, TMDSA_TRANSMITTER_ENABLE, 0x00000003, 0x00000003);
	RHDRegMask(Output, TMDSA_TRANSMITTER_CONTROL, 0x00000001, 0x00000003);
    }

    RHDRegMask(Output, TMDSA_LOAD_DETECT, 0x00000001, 0x00000001);
    usleep(1);
    ret = RHDRegRead(Output, TMDSA_LOAD_DETECT) & 0x00000010;

    RHDRegMask(Output, TMDSA_LOAD_DETECT, Detect, 0x00000001);

    if (rhdPtr->ChipSet < RHD_R600) {
	RHDRegWrite(Output, TMDSA_TRANSMITTER_ENABLE, Enable);
	RHDRegWrite(Output, TMDSA_TRANSMITTER_CONTROL, Control);
    }

    RHDDebug(Output->scrnIndex, "%s: %s\n", __func__,
	     ret ? "Attached" : "Disconnected");

    if (ret)
	return RHD_SENSED_DVI;
    else
	return RHD_SENSED_NONE;
}

/*
 *
 */
static ModeStatus
TMDSAModeValid(struct rhdOutput *Output, DisplayModePtr Mode)
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
 * This information is not provided in an atombios data table.
 */
static struct R5xxTMDSAMacro {
    CARD16 Device;
    CARD32 Macro;
} R5xxTMDSAMacro[] = {
    { 0x7104, 0x00C00414 }, /* R520  */
    { 0x7142, 0x00A00415 }, /* RV515 */
    { 0x7145, 0x00A00416 }, /* M54   */
    { 0x7146, 0x00C0041F }, /* RV515 */
    { 0x7147, 0x00C00418 }, /* RV505 */
    { 0x7149, 0x00800416 }, /* M56   */
    { 0x7152, 0x00A00415 }, /* RV515 */
    { 0x7183, 0x00600412 }, /* RV530 */
    { 0x71C1, 0x00C0041F }, /* RV535 */
    { 0x71C2, 0x00A00416 }, /* RV530 */
    { 0x71C4, 0x00A00416 }, /* M56   */
    { 0x71C5, 0x00A00416 }, /* M56   */
    { 0x71C6, 0x00A00513 }, /* RV530 */
    { 0x71D2, 0x00A00513 }, /* RV530 */
    { 0x71D5, 0x00A00513 }, /* M66   */
    { 0x7249, 0x00A00513 }, /* R580  */
    { 0x724B, 0x00A00513 }, /* R580  */
    { 0x7280, 0x00C0041F }, /* RV570 */
    { 0x7288, 0x00C0041F }, /* RV570 */
    { 0x9400, 0x00910419 }, /* R600: */
    { 0, 0} /* End marker */
};

static struct Rv6xxTMDSAMacro {
    CARD16 Device;
    CARD32 PLL;
    CARD32 TX;
} Rv6xxTMDSAMacro[] = {
    { 0x94C1, 0x00010416, 0x00010308 }, /* RV610 */
    { 0x94C3, 0x00010416, 0x00010308 }, /* RV610 */
    { 0x9501, 0x00010416, 0x00010308 }, /* RV670: != atombios */
    { 0x9505, 0x00010416, 0x00010308 }, /* RV670: != atombios */
    { 0x950F, 0x00010416, 0x00010308 }, /* R680 : != atombios */
    { 0x9581, 0x00030410, 0x00301044 }, /* M76 */
    { 0x9587, 0x00010416, 0x00010308 }, /* RV630 */
    { 0x9588, 0x00010416, 0x00010388 }, /* RV630 */
    { 0x9589, 0x00010416, 0x00010388 }, /* RV630 */
    { 0, 0, 0} /* End marker */
};

static void
TMDSAVoltageControl(struct rhdOutput *Output)
{
    RHDPtr rhdPtr = RHDPTRI(Output);
    int i;

    if (rhdPtr->ChipSet < RHD_RV610) {
	for (i = 0; R5xxTMDSAMacro[i].Device; i++)
	    if (R5xxTMDSAMacro[i].Device == rhdPtr->PciDeviceID) {
		RHDRegWrite(Output, TMDSA_MACRO_CONTROL, R5xxTMDSAMacro[i].Macro);
		return;
	    }

	xf86DrvMsg(Output->scrnIndex, X_ERROR, "%s: unhandled chipset: 0x%04X.\n",
		   __func__, rhdPtr->PciDeviceID);
	xf86DrvMsg(Output->scrnIndex, X_INFO, "TMDSA_MACRO_CONTROL: 0x%08X\n",
		   (unsigned int) RHDRegRead(Output, TMDSA_MACRO_CONTROL));
    } else {
	for (i = 0; Rv6xxTMDSAMacro[i].Device; i++)
	    if (Rv6xxTMDSAMacro[i].Device == rhdPtr->PciDeviceID) {
		RHDRegWrite(Output, TMDSA_PLL_ADJUST, Rv6xxTMDSAMacro[i].PLL);
		RHDRegWrite(Output, TMDSA_TRANSMITTER_ADJUST, Rv6xxTMDSAMacro[i].TX);
		return;
	    }
	xf86DrvMsg(Output->scrnIndex, X_ERROR, "%s: unhandled chipset: 0x%04X.\n",
		   __func__, rhdPtr->PciDeviceID);
	xf86DrvMsg(Output->scrnIndex, X_INFO, "TMDSA_PLL_ADJUST: 0x%08X\n",
		   (unsigned int) RHDRegRead(Output, TMDSA_PLL_ADJUST));
	xf86DrvMsg(Output->scrnIndex, X_INFO, "TMDSA_TRANSMITTER_ADJUST: 0x%08X\n",
		   (unsigned int) RHDRegRead(Output, TMDSA_TRANSMITTER_ADJUST));
    }
}

/*
 *
 */
static Bool
TMDSAPropertyControl(struct rhdOutput *Output,
	     enum rhdPropertyAction Action, enum rhdOutputProperty Property, union rhdPropertyData *val)
{
    struct rhdTMDSPrivate *Private = (struct rhdTMDSPrivate *) Output->Private;

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
		    break;
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
TMDSASet(struct rhdOutput *Output, DisplayModePtr Mode)
{
    RHDPtr rhdPtr = RHDPTRI(Output);
    struct rhdTMDSPrivate *Private = (struct rhdTMDSPrivate *) Output->Private;

    RHDFUNC(Output);

    /* Clear out some HPD events first: this should be under driver control. */
    RHDRegMask(Output, TMDSA_TRANSMITTER_CONTROL, 0, 0x0000000C);
    RHDRegMask(Output, TMDSA_TRANSMITTER_ENABLE, 0, 0x00070000);
    RHDRegMask(Output, TMDSA_CNTL, 0, 0x00000010);

    /* Disable the transmitter */
    RHDRegMask(Output, TMDSA_TRANSMITTER_ENABLE, 0, 0x00001D1F);

    /* Disable bit reduction and reset temporal dither */
    RHDRegMask(Output, TMDSA_BIT_DEPTH_CONTROL, 0, 0x00010101);
    if (rhdPtr->ChipSet < RHD_R600) {
	RHDRegMask(Output, TMDSA_BIT_DEPTH_CONTROL, 0x04000000, 0x04000000);
	usleep(2);
	RHDRegMask(Output, TMDSA_BIT_DEPTH_CONTROL, 0, 0x04000000);
    } else {
	RHDRegMask(Output, TMDSA_BIT_DEPTH_CONTROL, 0x02000000, 0x02000000);
	usleep(2);
	RHDRegMask(Output, TMDSA_BIT_DEPTH_CONTROL, 0, 0x02000000);
    }

    /* reset phase on vsync and use RGB */
    RHDRegMask(Output, TMDSA_CNTL, 0x00001000, 0x00011000);

    /* Select CRTC, select syncA, no stereosync */
    RHDRegMask(Output, TMDSA_SOURCE_SELECT, Output->Crtc->Id, 0x00010101);

    /* Single link, for now */
    RHDRegWrite(Output, TMDSA_COLOR_FORMAT, 0);

    /* store this for TRANSMITTER_ENABLE in TMDSAPower */
    Private->Mode = Mode;
    if (Mode->SynthClock > 165000) {
	RHDRegMask(Output, TMDSA_CNTL, 0x01000000, 0x01000000);
	Private->RunsDualLink = TRUE; /* for TRANSMITTER_ENABLE in TMDSAPower */
    } else {
	RHDRegMask(Output, TMDSA_CNTL, 0, 0x01000000);
	Private->RunsDualLink = FALSE;
    }

    /* Disable force data */
    RHDRegMask(Output, TMDSA_FORCE_OUTPUT_CNTL, 0, 0x00000001);

    /* DC balancer enable */
    RHDRegMask(Output, TMDSA_DCBALANCER_CONTROL, 0x00000001, 0x00000001);

    TMDSAVoltageControl(Output);

    /* use IDCLK */
    RHDRegMask(Output, TMDSA_TRANSMITTER_CONTROL, 0x00000010, 0x00000010);

    if (Private->Coherent)
	RHDRegMask(Output, TMDSA_TRANSMITTER_CONTROL, 0x00000000, 0x10000000);
    else
	RHDRegMask(Output, TMDSA_TRANSMITTER_CONTROL, 0x10000000, 0x10000000);

    RHDHdmiSetMode(Private->Hdmi, Mode);
}

/*
 *
 */
static void
TMDSAPower(struct rhdOutput *Output, int Power)
{
    RHDPtr rhdPtr = RHDPTRI(Output);
    struct rhdTMDSPrivate *Private = (struct rhdTMDSPrivate *) Output->Private;

    RHDDebug(Output->scrnIndex, "%s(%s,%s)\n",__func__,Output->Name,
	     rhdPowerString[Power]);

    switch (Power) {
    case RHD_POWER_ON:
	if (Private->PowerState == RHD_POWER_SHUTDOWN
	    || Private->PowerState == RHD_POWER_UNKNOWN) {
	    RHDRegMask(Output, TMDSA_CNTL, 0x1, 0x00000001);

	RHDRegMask(Output, TMDSA_TRANSMITTER_CONTROL, 0x00000001, 0x00000001);
	usleep(20);

	/* reset transmitter PLL */
	RHDRegMask(Output, TMDSA_TRANSMITTER_CONTROL, 0x00000002, 0x00000002);
	usleep(2);
	RHDRegMask(Output, TMDSA_TRANSMITTER_CONTROL, 0, 0x00000002);

	usleep(30);

	/* restart data synchronisation */
  if (rhdPtr->ChipSet < RHD_R600) {
	    RHDRegMask(Output, TMDSA_DATA_SYNCHRONIZATION_R500, 0x00000001, 0x00000001);
	    usleep(2);
	    RHDRegMask(Output, TMDSA_DATA_SYNCHRONIZATION_R500, 0x00000100, 0x00000100);
	    RHDRegMask(Output, TMDSA_DATA_SYNCHRONIZATION_R500, 0, 0x00000001);
	} else {
	    RHDRegMask(Output, TMDSA_DATA_SYNCHRONIZATION_R600, 0x00000001, 0x00000001);
	    usleep(2);
	    RHDRegMask(Output, TMDSA_DATA_SYNCHRONIZATION_R600, 0x00000100, 0x00000100);
	    RHDRegMask(Output, TMDSA_DATA_SYNCHRONIZATION_R600, 0, 0x00000001);
	}
	}

	if (Private->RunsDualLink) {
	    /* bit 9 is not known by anything below RV610, but is ignored by
	       the hardware anyway */
	    RHDRegMask(Output, TMDSA_TRANSMITTER_ENABLE, 0x00001F1F, 0x00001F1F);
	} else
	    RHDRegMask(Output, TMDSA_TRANSMITTER_ENABLE, 0x0000001F, 0x00001F1F);

	if(Output->Connector != NULL && RHDConnectorEnableHDMI(Output->Connector))
	    RHDHdmiEnable(Private->Hdmi, TRUE);
	else
	    RHDHdmiEnable(Private->Hdmi, FALSE);
	Private->PowerState = RHD_POWER_ON;
	return;

    case RHD_POWER_RESET:
	RHDRegMask(Output, TMDSA_TRANSMITTER_ENABLE, 0, 0x00001F1F);
	/* if we do a RESET after a SHUTDOWN don't raise the power level,
	 * and similarly, don't raise from UNKNOWN state. */
	if (Private->PowerState == RHD_POWER_ON)
	    Private->PowerState = RHD_POWER_RESET;
	return;

    case RHD_POWER_SHUTDOWN:
    default:
	RHDRegMask(Output, TMDSA_TRANSMITTER_CONTROL, 0x00000002, 0x00000002);
	usleep(2);
	RHDRegMask(Output, TMDSA_TRANSMITTER_CONTROL, 0, 0x00000001);
	RHDRegMask(Output, TMDSA_TRANSMITTER_ENABLE, 0, 0x00001F1F);
	RHDRegMask(Output, TMDSA_CNTL, 0, 0x00000001);
	RHDHdmiEnable(Private->Hdmi, FALSE);
	Private->PowerState = RHD_POWER_SHUTDOWN;
	return;
    }
}

/*
 *
 */
static void
TMDSASave(struct rhdOutput *Output)
{
    int ChipSet = RHDPTRI(Output)->ChipSet;
    struct rhdTMDSPrivate *Private = (struct rhdTMDSPrivate *) Output->Private;

    RHDFUNC(Output);

    Private->StoreControl = RHDRegRead(Output, TMDSA_CNTL);
    Private->StoreSource = RHDRegRead(Output, TMDSA_SOURCE_SELECT);
    Private->StoreFormat = RHDRegRead(Output, TMDSA_COLOR_FORMAT);
    Private->StoreForce = RHDRegRead(Output, TMDSA_FORCE_OUTPUT_CNTL);
    Private->StoreReduction = RHDRegRead(Output, TMDSA_BIT_DEPTH_CONTROL);
    Private->StoreDCBalancer = RHDRegRead(Output, TMDSA_DCBALANCER_CONTROL);

    if (ChipSet < RHD_R600)
	Private->StoreDataSynchro = RHDRegRead(Output, TMDSA_DATA_SYNCHRONIZATION_R500);
    else
	Private->StoreDataSynchro = RHDRegRead(Output, TMDSA_DATA_SYNCHRONIZATION_R600);

    Private->StoreTXEnable = RHDRegRead(Output, TMDSA_TRANSMITTER_ENABLE);
    Private->StoreMacro = RHDRegRead(Output, TMDSA_MACRO_CONTROL);
    Private->StoreTXControl = RHDRegRead(Output, TMDSA_TRANSMITTER_CONTROL);

    if (ChipSet >= RHD_RV610)
	Private->StoreTXAdjust = RHDRegRead(Output, TMDSA_TRANSMITTER_ADJUST);

    RHDHdmiSave(Private->Hdmi);

    Private->Stored = TRUE;
}

/*
 *
 */
static void
TMDSARestore(struct rhdOutput *Output)
{
    int ChipSet = RHDPTRI(Output)->ChipSet;
    struct rhdTMDSPrivate *Private = (struct rhdTMDSPrivate *) Output->Private;

    RHDFUNC(Output);

    if (!Private->Stored) {
	xf86DrvMsg(Output->scrnIndex, X_ERROR,
		   "%s: No registers stored.\n", __func__);
	return;
    }

    RHDRegWrite(Output, TMDSA_CNTL, Private->StoreControl);
    RHDRegWrite(Output, TMDSA_SOURCE_SELECT, Private->StoreSource);
    RHDRegWrite(Output, TMDSA_COLOR_FORMAT, Private->StoreFormat);
    RHDRegWrite(Output, TMDSA_FORCE_OUTPUT_CNTL, Private->StoreForce);
    RHDRegWrite(Output, TMDSA_BIT_DEPTH_CONTROL, Private->StoreReduction);
    RHDRegWrite(Output, TMDSA_DCBALANCER_CONTROL, Private->StoreDCBalancer);

    if (ChipSet < RHD_R600)
	RHDRegWrite(Output, TMDSA_DATA_SYNCHRONIZATION_R500, Private->StoreDataSynchro);
    else
	RHDRegWrite(Output, TMDSA_DATA_SYNCHRONIZATION_R600, Private->StoreDataSynchro);

    RHDRegWrite(Output, TMDSA_TRANSMITTER_ENABLE, Private->StoreTXEnable);
    RHDRegWrite(Output, TMDSA_MACRO_CONTROL, Private->StoreMacro);
    RHDRegWrite(Output, TMDSA_TRANSMITTER_CONTROL, Private->StoreTXControl);

    if (ChipSet >= RHD_RV610)
	RHDRegWrite(Output, TMDSA_TRANSMITTER_ADJUST, Private->StoreTXAdjust);

    RHDHdmiRestore(Private->Hdmi);
}

/*
 *
 */
static void
TMDSADestroy(struct rhdOutput *Output)
{
    struct rhdTMDSPrivate *Private = (struct rhdTMDSPrivate *) Output->Private;
    RHDFUNC(Output);

    if (!Private)
	return;

    RHDHdmiDestroy(Private->Hdmi);

    xfree(Private);
    Output->Private = NULL;
}

/*
 *
 */
struct rhdOutput *
RHDTMDSAInit(RHDPtr rhdPtr)
{
    struct rhdOutput *Output;
    struct rhdTMDSPrivate *Private;

    RHDFUNC(rhdPtr);

    Output = xnfcalloc(sizeof(struct rhdOutput), 1);

    Output->scrnIndex = rhdPtr->scrnIndex;
    Output->Name = "TMDS A";
    Output->Id = RHD_OUTPUT_TMDSA;

    Output->Sense = TMDSASense;
    Output->ModeValid = TMDSAModeValid;
    Output->Mode = TMDSASet;
    Output->Power = TMDSAPower;
    Output->Save = TMDSASave;
    Output->Restore = TMDSARestore;
    Output->Destroy = TMDSADestroy;
    Output->Property = TMDSAPropertyControl;

    Private = xnfcalloc(sizeof(struct rhdTMDSPrivate), 1);
    Private->RunsDualLink = FALSE;
    Private->Coherent = FALSE;
    Private->PowerState = RHD_POWER_UNKNOWN;

    Output->Private = Private;

    return Output;
}
