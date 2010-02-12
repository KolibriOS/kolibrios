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
# include <string.h>
# include <stdio.h>
#endif

#include "rhd.h"
#include "edid.h"
#include "rhd_connector.h"
#include "rhd_output.h"
#include "rhd_crtc.h"
#include "rhd_atombios.h"
#include "rhd_atomout.h"
#include "rhd_biosscratch.h"
#include "rhd_hdmi.h"

#if defined (ATOM_BIOS) && defined (ATOM_BIOS_PARSER)
struct rhdAtomOutputPrivate {
    Bool Stored;

    struct atomCodeTableVersion EncoderVersion;
    struct atomCodeTableVersion CrtcSourceVersion;
    struct atomEncoderConfig EncoderConfig;
    enum atomEncoder EncoderId;

    struct atomTransmitterConfig TransmitterConfig;
    enum atomTransmitter TransmitterId;

    enum atomOutput OutputControlId;

    Bool   RunDualLink;
    int    PixelClock;

    void  *Save;

    CARD16 PowerDigToDE;
    CARD16 PowerDEToBL;
    CARD16 OffDelay;
    Bool   TemporalDither;
    Bool   SpatialDither;
    int    GreyLevel;
    Bool   DualLink;
    Bool   LVDS24Bit;
    Bool   FPDI;

    Bool   Coherent;
    DisplayModePtr Mode;
    struct rhdHdmi *Hdmi;

    int    BlLevel;
};

#define ERROR_MSG(x) 	xf86DrvMsg(Output->scrnIndex, X_ERROR, "%s: %s failed.\n", __func__, x)

/*
 *
 */
static inline void
rhdSetEncoderTransmitterConfig(struct rhdOutput *Output, int PixelClock)
{
    RHDPtr rhdPtr = RHDPTRI(Output);
    struct rhdAtomOutputPrivate *Private = (struct rhdAtomOutputPrivate *) Output->Private;
    struct atomEncoderConfig *EncoderConfig = &Private->EncoderConfig;
    struct atomTransmitterConfig *TransmitterConfig = &Private->TransmitterConfig;

    RHDFUNC(Output);

    EncoderConfig->PixelClock = TransmitterConfig->PixelClock = PixelClock;

    switch (Output->Id) {
	case RHD_OUTPUT_NONE:
	    break;
	case RHD_OUTPUT_DVO:

	    EncoderConfig->u.dvo.DvoDeviceType = Output->OutputDriverPrivate->Device;
	    switch (EncoderConfig->u.dvo.DvoDeviceType) {
		case atomCRT1:
		case atomCRT2:
		    EncoderConfig->u.dvo.digital = FALSE;
		break;
		case atomTV1:
		case atomTV2:
		case atomCV:
		    EncoderConfig->u.dvo.digital = FALSE;
		    EncoderConfig->u.dvo.u.TVMode = rhdPtr->tvMode;
		break;
		case atomLCD1:
		case atomDFP1:
		case atomDFP2:
		case atomLCD2:
		case atomDFP3:
		case atomDFP4:
		case atomDFP5:
		    EncoderConfig->u.dvo.digital = TRUE;
		    /* @@@ no digital attributes, yet */
		break;
		case atomNone:
		    break;
	    }
	    break;
	case RHD_OUTPUT_DACA:
	case RHD_OUTPUT_DACB:
	    switch (Output->SensedType) {
		case RHD_SENSED_VGA:
		    EncoderConfig->u.dac.DacStandard = atomDAC_VGA;
		    break;
		case RHD_SENSED_TV_COMPONENT:
		    EncoderConfig->u.dac.DacStandard = atomDAC_CV;
		    break;
		case RHD_SENSED_TV_SVIDEO:
		case RHD_SENSED_TV_COMPOSITE:
		    switch (rhdPtr->tvMode) {
			case RHD_TV_NTSC:
			case RHD_TV_NTSCJ:
			    EncoderConfig->u.dac.DacStandard = atomDAC_NTSC;
			    /* NTSC */
			    break;
			case RHD_TV_PAL:
			case RHD_TV_PALN:
			case RHD_TV_PALCN:
			case RHD_TV_PAL60:
			default:
			    EncoderConfig->u.dac.DacStandard = atomDAC_PAL;
			    /* PAL */
			    break;
		    }
		    break;
		case RHD_SENSED_NONE:
		    EncoderConfig->u.dac.DacStandard = atomDAC_VGA;
		    break;
		default:
		    xf86DrvMsg(Output->scrnIndex, X_ERROR, "Sensed incompatible output for DAC\n");
		    EncoderConfig->u.dac.DacStandard = atomDAC_VGA;
		    break;
	    }
	    break;

	case RHD_OUTPUT_TMDSA:
	case RHD_OUTPUT_LVTMA:
	    if (Output->Connector && PixelClock > 0) {
		if (Output->Connector->Type == RHD_CONNECTOR_DVI
#if 0
		    || Output->Connector->Type == RHD_CONNECTOR_HDMI_B
#endif
		    )
		    Private->RunDualLink = (PixelClock > 165000) ? TRUE : FALSE;
		else
		    Private->RunDualLink = FALSE;
	    } else
		/* only get here for power down: thus power down both channels to be save */
		Private->RunDualLink = TRUE;

	    switch (Private->EncoderVersion.cref) {
		case 1:
		    if (Private->RunDualLink)
			EncoderConfig->u.lvds.LinkCnt = atomDualLink;
		    else
			EncoderConfig->u.lvds.LinkCnt = atomSingleLink;
		    break;
		case 2:
		case 3:
		    if (Private->RunDualLink)
			EncoderConfig->u.lvds2.LinkCnt = atomDualLink;
		    else
			EncoderConfig->u.lvds2.LinkCnt = atomSingleLink;
		    if (Private->Coherent)
			EncoderConfig->u.lvds2.Coherent = TRUE;
		    else
			EncoderConfig->u.lvds2.Coherent = FALSE;
		    break;
	    }
	    break;

	case RHD_OUTPUT_KLDSKP_LVTMA:
	case RHD_OUTPUT_UNIPHYA:
	case RHD_OUTPUT_UNIPHYB:
	case RHD_OUTPUT_UNIPHYC:
	case RHD_OUTPUT_UNIPHYD:
	case RHD_OUTPUT_UNIPHYE:
	case RHD_OUTPUT_UNIPHYF:
	    if (Output->Connector && PixelClock > 0) {
		if (Output->Connector->Type == RHD_CONNECTOR_DVI
#if 0
		    || Output->Connector->Type == RHD_CONNECTOR_DP_DUAL
		    || Output->Connector->Type == RHD_CONNECTOR_HDMI_B
#endif
		    )
		    Private->RunDualLink = (PixelClock > 165000) ? TRUE : FALSE;
		else
		    Private->RunDualLink = FALSE;
	    } else
		/* only get here for power down: thus power down both channels to be save */
		    Private->RunDualLink = TRUE;

	    if (Private->RunDualLink) {
		TransmitterConfig->LinkCnt = EncoderConfig->u.dig.LinkCnt = atomDualLink;
		if (TransmitterConfig->Link == atomTransLinkA)
		    TransmitterConfig->Link = atomTransLinkAB;
		else if (TransmitterConfig->Link == atomTransLinkB)
		    TransmitterConfig->Link = atomTransLinkBA;
	    } else {
		TransmitterConfig->LinkCnt = EncoderConfig->u.dig.LinkCnt = atomSingleLink;
		if (TransmitterConfig->Link == atomTransLinkAB)
		    TransmitterConfig->Link = atomTransLinkA;
		else if (TransmitterConfig->Link == atomTransLinkBA)
		    TransmitterConfig->Link = atomTransLinkB;
	    }
 	    TransmitterConfig->Coherent = Private->Coherent;
	    break;
    }
}

/*
 *
 */
static void
atomSetBacklightFromBIOSScratch(struct rhdOutput *Output)
{
    RHDPtr rhdPtr = RHDPTRI(Output);
    struct rhdAtomOutputPrivate *Private = (struct rhdAtomOutputPrivate *) Output->Private;

    RHDFUNC(Output);

    switch (Output->Id) {
	case RHD_OUTPUT_KLDSKP_LVTMA:
	case RHD_OUTPUT_UNIPHYA:
	case RHD_OUTPUT_UNIPHYB:
	case RHD_OUTPUT_UNIPHYC:
	case RHD_OUTPUT_UNIPHYD:
	case RHD_OUTPUT_UNIPHYE:
	case RHD_OUTPUT_UNIPHYF:
	    rhdSetEncoderTransmitterConfig(Output, Private->PixelClock);
	    if (!rhdAtomDigTransmitterControl(rhdPtr->atomBIOS, Private->TransmitterId,
					      atomTransLcdBlBrightness, &Private->TransmitterConfig))
		ERROR_MSG("rhdAtomDigTransmitterControl(atomTransEnable)");
	    break;
	default:
	    if (!rhdAtomOutputControl(rhdPtr->atomBIOS, Private->OutputControlId, atomOutputLcdBrightnessControl))
		ERROR_MSG("rhdAtomOutputControl(atomOutputLcdBrightnessControl)");
	    break;
    }
}

/*
 *
 */
static void
atomSetBacklight(struct rhdOutput *Output, int value)
{
    RHDPtr rhdPtr = RHDPTRI(Output);

    RHDFUNC(Output);

    RHDAtomBIOSScratchBlLevel(rhdPtr, rhdBIOSScratchBlSet, &value);

    atomSetBacklightFromBIOSScratch(Output);
}

/*
 *
 */
static inline void
rhdAtomOutputSet(struct rhdOutput *Output, DisplayModePtr Mode)
{
    RHDPtr rhdPtr = RHDPTRI(Output);
    struct rhdAtomOutputPrivate *Private = (struct rhdAtomOutputPrivate *) Output->Private;
    struct atomEncoderConfig *EncoderConfig = &Private->EncoderConfig;
    struct atomCrtcSourceConfig CrtcSourceConfig;
    union AtomBiosArg data;

    RHDFUNC(Output);

    Private->Mode = Mode;

    data.Address = &Private->Save;
    RHDAtomBiosFunc(Output->scrnIndex, rhdPtr->atomBIOS, ATOM_SET_REGISTER_LIST_LOCATION, &data);

    Private->PixelClock = Mode->SynthClock;
    rhdSetEncoderTransmitterConfig(Output, Private->PixelClock);

    switch ( Private->CrtcSourceVersion.cref){
	case 1:
	    CrtcSourceConfig.u.Device = Output->OutputDriverPrivate->Device;
	    break;
	case 2:
	    CrtcSourceConfig.u.crtc2.Encoder = Private->EncoderId;
	    CrtcSourceConfig.u.crtc2.Mode = EncoderConfig->u.dig.EncoderMode;
	    break;
	default:
	    xf86DrvMsg(Output->scrnIndex, X_ERROR,
		       "Unknown version of SelectCrtcSource code table: %i\n",Private->CrtcSourceVersion.cref);
	    return;
    }
    switch (Output->Id) {
	case RHD_OUTPUT_UNIPHYA:
	case RHD_OUTPUT_UNIPHYB:
 	case RHD_OUTPUT_UNIPHYC:
 	case RHD_OUTPUT_UNIPHYD:
 	case RHD_OUTPUT_UNIPHYE:
 	case RHD_OUTPUT_UNIPHYF:
#if 1
	    rhdAtomDigTransmitterControl(rhdPtr->atomBIOS, Private->TransmitterId, atomTransInit,
					 &Private->TransmitterConfig);
#endif
	case RHD_OUTPUT_KLDSKP_LVTMA:
	    rhdAtomDigTransmitterControl(rhdPtr->atomBIOS, Private->TransmitterId, atomTransSetup,
					 &Private->TransmitterConfig);
	    break;
	default:
	    break;
    }

    rhdAtomSelectCrtcSource(rhdPtr->atomBIOS, Output->Crtc->Id ? atomCrtc2 : atomCrtc1, &CrtcSourceConfig);
    data.Address = NULL;
    RHDAtomBiosFunc(Output->scrnIndex, rhdPtr->atomBIOS, ATOM_SET_REGISTER_LIST_LOCATION, &data);
    RHDHdmiSetMode(Private->Hdmi, Mode);
}

/*
 *
 */
static inline void
rhdAtomOutputPower(struct rhdOutput *Output, int Power)
{
    RHDPtr rhdPtr = RHDPTRI(Output);
    struct rhdAtomOutputPrivate *Private = (struct rhdAtomOutputPrivate *) Output->Private;
    struct atomEncoderConfig *EncoderConfig = &Private->EncoderConfig;
    union AtomBiosArg data;
    Bool enableHDMI = FALSE;

    RHDFUNC(Output);

    if(Output->Connector != NULL) {
	enableHDMI = RHDConnectorEnableHDMI(Output->Connector);
	switch(Output->Id) {
	    case RHD_OUTPUT_TMDSA:
	    case RHD_OUTPUT_LVTMA:
		if(enableHDMI && !Private->EncoderConfig.u.lvds2.Hdmi)
		    Private->EncoderConfig.u.lvds2.Hdmi = TRUE;
		else if(!enableHDMI && Private->EncoderConfig.u.lvds2.Hdmi)
		    Private->EncoderConfig.u.lvds2.Hdmi = FALSE;
		break;

	    case RHD_OUTPUT_UNIPHYA:
	    case RHD_OUTPUT_UNIPHYB:
	    case RHD_OUTPUT_KLDSKP_LVTMA:
		if(enableHDMI && Private->TransmitterConfig.Mode == atomDVI) {
		    Private->TransmitterConfig.Mode = atomHDMI;
		    Private->EncoderConfig.u.dig.EncoderMode = atomHDMI;

		} else if(!enableHDMI && Private->TransmitterConfig.Mode == atomHDMI) {
		    Private->TransmitterConfig.Mode = atomDVI;
		    Private->EncoderConfig.u.dig.EncoderMode = atomDVI;
		}
		break;

	    default:
		break;
	}
    }

    data.Address = &Private->Save;
    RHDAtomBiosFunc(Output->scrnIndex, rhdPtr->atomBIOS, ATOM_SET_REGISTER_LIST_LOCATION, &data);

    rhdSetEncoderTransmitterConfig(Output, Private->PixelClock);

    switch (Power) {
	case RHD_POWER_ON:
	    RHDDebug(Output->scrnIndex, "RHD_POWER_ON\n");
	    rhdAtomEncoderControl(rhdPtr->atomBIOS,  Private->EncoderId, atomEncoderOn, EncoderConfig);
	    switch (Output->Id) {
		case RHD_OUTPUT_KLDSKP_LVTMA:
		case RHD_OUTPUT_UNIPHYA:
		case RHD_OUTPUT_UNIPHYB:
		case RHD_OUTPUT_UNIPHYC:
		case RHD_OUTPUT_UNIPHYD:
		case RHD_OUTPUT_UNIPHYE:
		case RHD_OUTPUT_UNIPHYF:
		    if (!rhdAtomDigTransmitterControl(rhdPtr->atomBIOS, Private->TransmitterId,
						      atomTransEnable, &Private->TransmitterConfig)) {
			ERROR_MSG("rhdAtomDigTransmitterControl(atomTransEnable)");
			break;
		    }
		    if (!rhdAtomDigTransmitterControl(rhdPtr->atomBIOS, Private->TransmitterId,
						      atomTransEnableOutput, &Private->TransmitterConfig))
			ERROR_MSG("rhdAtomDigTransmitterControl(atomTransEnableOutput)");
		    break;
		default:
		    if (!rhdAtomOutputControl(rhdPtr->atomBIOS, Private->OutputControlId, atomOutputEnable))
			ERROR_MSG("rhdAtomOutputControl(atomOutputEnable)");
		    break;
	    }
	    RHDHdmiEnable(Private->Hdmi, enableHDMI);
	    break;
	case RHD_POWER_RESET:
	    RHDDebug(Output->scrnIndex, "RHD_POWER_RESET\n");
	    switch (Output->Id) {
		case RHD_OUTPUT_KLDSKP_LVTMA:
		case RHD_OUTPUT_UNIPHYA:
		case RHD_OUTPUT_UNIPHYB:
		case RHD_OUTPUT_UNIPHYC:
		case RHD_OUTPUT_UNIPHYD:
		case RHD_OUTPUT_UNIPHYE:
		case RHD_OUTPUT_UNIPHYF:
		    if (!rhdAtomDigTransmitterControl(rhdPtr->atomBIOS, Private->TransmitterId,
						      atomTransDisableOutput, &Private->TransmitterConfig))
			ERROR_MSG("rhdAtomDigTransmitterControl(atomTransDisableOutput)");
		    break;
		default:
		    if (!rhdAtomOutputControl(rhdPtr->atomBIOS, Private->OutputControlId, atomOutputDisable))
			ERROR_MSG("rhdAtomOutputControl(atomOutputDisable)");
		    break;
	    }
	    break;
	case RHD_POWER_SHUTDOWN:
	    RHDDebug(Output->scrnIndex, "RHD_POWER_SHUTDOWN\n");
	    switch (Output->Id) {
		case RHD_OUTPUT_KLDSKP_LVTMA:
		case RHD_OUTPUT_UNIPHYA:
		case RHD_OUTPUT_UNIPHYB:
		case RHD_OUTPUT_UNIPHYC:
		case RHD_OUTPUT_UNIPHYD:
		case RHD_OUTPUT_UNIPHYE:
		case RHD_OUTPUT_UNIPHYF:
		    if (Private->EncoderId == atomEncoderNone)
			break;
		    if (!rhdAtomDigTransmitterControl(rhdPtr->atomBIOS, Private->TransmitterId,
						      atomTransDisableOutput, &Private->TransmitterConfig)) {
			ERROR_MSG("rhdAtomDigTransmitterControl(atomTransDisableOutput)");
			break;
		    }
		    if (!rhdAtomDigTransmitterControl(rhdPtr->atomBIOS, Private->TransmitterId,
						      atomTransDisable, &Private->TransmitterConfig))
			ERROR_MSG("rhdAtomDigTransmitterControl(atomTransDisable)");
		    break;
		default:
		    if (!rhdAtomOutputControl(rhdPtr->atomBIOS, Private->OutputControlId, atomOutputDisable))
			ERROR_MSG("rhdAtomOutputControl(atomOutputDisable)");
		    break;
	    }
	    if (Private->EncoderId != atomEncoderNone)
	    if (!rhdAtomEncoderControl(rhdPtr->atomBIOS, Private->EncoderId, atomEncoderOff, &Private->EncoderConfig))
		ERROR_MSG("rhdAtomEncoderControl(atomEncoderOff)");
	    RHDHdmiEnable(Private->Hdmi, FALSE);
	    break;
    }

    data.Address = NULL;
    RHDAtomBiosFunc(Output->scrnIndex, rhdPtr->atomBIOS, ATOM_SET_REGISTER_LIST_LOCATION, &data);
}

/*
 *
 */
static inline void
rhdAtomOutputSave(struct rhdOutput *Output)
{
     struct rhdAtomOutputPrivate *Private = (struct rhdAtomOutputPrivate *) Output->Private;
     RHDHdmiSave(Private->Hdmi);
}

/*
 *
 */
static void
rhdAtomOutputRestore(struct rhdOutput *Output)
{
     struct rhdAtomOutputPrivate *Private = (struct rhdAtomOutputPrivate *) Output->Private;
     RHDPtr rhdPtr = RHDPTRI(Output);
     union AtomBiosArg data;

     data.Address = &Private->Save;
     RHDAtomBiosFunc(Output->scrnIndex, rhdPtr->atomBIOS, ATOM_RESTORE_REGISTERS, &data);
     if (Output->Connector && Output->Connector->Type == RHD_CONNECTOR_PANEL)
	 atomSetBacklightFromBIOSScratch(Output);
     RHDHdmiRestore(Private->Hdmi);
}

/*
 *
 */
static ModeStatus
rhdAtomOutputModeValid(struct rhdOutput *Output, DisplayModePtr Mode)
{
    RHDFUNC(Output);

    if (Mode->Flags & V_INTERLACE)
	return MODE_NO_INTERLACE;

    if (Mode->Clock < 25000)
	return MODE_CLOCK_LOW;


    if (Output->Connector->Type == RHD_CONNECTOR_DVI_SINGLE
#if 0
		|| Output->Connector->Type == RHD_CONNECTOR_DP_DUAL
		|| Output->Connector->Type == RHD_CONNECTOR_HDMI_B
#endif
	) {
	if (Mode->Clock > 165000)
	    return MODE_CLOCK_HIGH;
    }
    else if (Output->Connector->Type == RHD_CONNECTOR_DVI) {
	if (Mode->Clock > 330000) /* could go higher still */
	    return MODE_CLOCK_HIGH;
    }

    return MODE_OK;
}


/*
 *
 */
static Bool
LVDSInfoRetrieve(RHDPtr rhdPtr, struct rhdAtomOutputPrivate *Private)
{
    AtomBiosArgRec data;

    if (RHDAtomBiosFunc(rhdPtr->scrnIndex, rhdPtr->atomBIOS,
			ATOM_LVDS_SEQ_DIG_ONTO_DE, &data) != ATOM_SUCCESS)
	return FALSE;
    Private->PowerDigToDE = data.val;

    if (RHDAtomBiosFunc(rhdPtr->scrnIndex, rhdPtr->atomBIOS,
			ATOM_LVDS_SEQ_DE_TO_BL, &data) != ATOM_SUCCESS)
	return FALSE;
    Private->PowerDEToBL = data.val;

    if (RHDAtomBiosFunc(rhdPtr->scrnIndex, rhdPtr->atomBIOS,
			ATOM_LVDS_OFF_DELAY, &data) != ATOM_SUCCESS)
	return FALSE;
    Private->OffDelay = data.val;

    if (RHDAtomBiosFunc(rhdPtr->scrnIndex, rhdPtr->atomBIOS,
			ATOM_LVDS_DUALLINK, &data) != ATOM_SUCCESS)
	return FALSE;
    Private->DualLink = data.val;

    if (RHDAtomBiosFunc(rhdPtr->scrnIndex, rhdPtr->atomBIOS,
			ATOM_LVDS_24BIT, &data) != ATOM_SUCCESS)
	return FALSE;
    Private->LVDS24Bit = data.val;

    if (RHDAtomBiosFunc(rhdPtr->scrnIndex, rhdPtr->atomBIOS,
			ATOM_LVDS_FPDI, &data) != ATOM_SUCCESS)
	return FALSE;
    Private->FPDI = data.val;

    if (RHDAtomBiosFunc(rhdPtr->scrnIndex, rhdPtr->atomBIOS,
			ATOM_LVDS_TEMPORAL_DITHER, &data) != ATOM_SUCCESS)
	return FALSE;
    Private->TemporalDither = data.val;

    if (RHDAtomBiosFunc(rhdPtr->scrnIndex, rhdPtr->atomBIOS,
			ATOM_LVDS_SPATIAL_DITHER, &data) != ATOM_SUCCESS)
	return FALSE;
    Private->SpatialDither = data.val;

    if (RHDAtomBiosFunc(rhdPtr->scrnIndex, rhdPtr->atomBIOS,
			ATOM_LVDS_GREYLVL, &data) != ATOM_SUCCESS)
	return FALSE;
    {
	Private->GreyLevel = data.val;
	xf86DrvMsg(rhdPtr->scrnIndex, X_ERROR, "AtomBIOS returned %i Grey Levels\n",
		   Private->GreyLevel);
    }
    Private->Coherent = FALSE;

    RHDAtomBIOSScratchBlLevel(rhdPtr, rhdBIOSScratchBlGet, &Private->BlLevel);

    return TRUE;
}

/*
 * TMDSInfoRetrieve() - interface to set TMDS (DVI) parameters.
 */
static Bool
TMDSInfoRetrieve(RHDPtr rhdPtr, struct rhdAtomOutputPrivate *Private)
{
    Private->FPDI = FALSE;
    Private->TemporalDither = FALSE;
    Private->SpatialDither = FALSE;
    Private->GreyLevel = 0;
    Private->BlLevel = -1;

    return TRUE;
}

/*
 *
 */
static Bool
atomLVDSPropertyControl(struct rhdOutput *Output,
	     enum rhdPropertyAction Action, enum rhdOutputProperty Property, union rhdPropertyData *val)
{
    struct rhdAtomOutputPrivate *Private = (struct rhdAtomOutputPrivate *) Output->Private;

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
		    atomSetBacklight(Output, val->integer);
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
atomTMDSPropertyControl(struct rhdOutput *Output,
	     enum rhdPropertyAction Action, enum rhdOutputProperty Property, union rhdPropertyData *val)
{
    struct rhdAtomOutputPrivate *Private = (struct rhdAtomOutputPrivate *) Output->Private;

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
rhdAtomOutputDestroy(struct rhdOutput *Output)
{
    struct rhdAtomOutputPrivate *Private = (struct rhdAtomOutputPrivate *) Output->Private;
    RHDFUNC(Output);
    if (Private->Save)
	xfree(Private->Save);
    RHDHdmiDestroy(Private->Hdmi);

    if (Private)
	xfree(Private);
    Output->Private = NULL;
    xfree(Output->Name);
}

/*
 *
 */
static Bool
RHDAtomOutputAllocFree(struct rhdOutput *Output, enum rhdOutputAllocation Alloc)
{
    struct rhdAtomOutputPrivate *Private = (struct rhdAtomOutputPrivate *) Output->Private;
    struct atomTransmitterConfig *TransmitterConfig = &Private->TransmitterConfig;
    RHDPtr rhdPtr = RHDPTRI(Output);
    char *TransmitterName;

    RHDFUNC(rhdPtr);

    switch (Output->Id) {
	case RHD_OUTPUT_KLDSKP_LVTMA:
	    TransmitterName = "KLDSKP_LVTMA";
	    break;
	case RHD_OUTPUT_UNIPHYA:
	    TransmitterName = "KLDSKP_UNIPHYA";
	    break;
	case RHD_OUTPUT_UNIPHYB:
	    TransmitterName = "KLDSKP_UNIPHYB";
	    break;
	case RHD_OUTPUT_UNIPHYC:
	    TransmitterName = "KLDSKP_UNIPHYC";
	    break;
	case RHD_OUTPUT_UNIPHYD:
	    TransmitterName = "KLDSKP_UNIPHYD";
	    break;
	case RHD_OUTPUT_UNIPHYE:
	    TransmitterName = "KLDSKP_UNIPHYE";
	    break;
	case RHD_OUTPUT_UNIPHYF:
	    TransmitterName = "KLDSKP_UNIPHYF";
	    break;
	default:
	    return TRUE;
    }

    switch (Alloc) {
	case RHD_OUTPUT_ALLOC:
	    /*
	     * LVTMA can only use DIG2. Thus exclude
	     * DIG1 for LVTMA and prefer it for the
	     * UNIPHYs.
	     */
	    if (Private->EncoderId != atomEncoderNone)
		return TRUE;
	    if (Output->Id != RHD_OUTPUT_KLDSKP_LVTMA
		&& !rhdPtr->DigEncoderOutput[0]) {
		rhdPtr->DigEncoderOutput[0] = Output;
		TransmitterConfig->Encoder = Private->EncoderId = atomEncoderDIG1;
		xf86DrvMsg(Output->scrnIndex, X_INFO, "Mapping DIG1 encoder to %s\n",TransmitterName);
		return TRUE;
	    } else if (!rhdPtr->DigEncoderOutput[1]) {
		rhdPtr->DigEncoderOutput[1] = Output;
		TransmitterConfig->Encoder = Private->EncoderId = atomEncoderDIG2;
		xf86DrvMsg(Output->scrnIndex, X_INFO, "Mapping DIG2 encoder to %s\n",TransmitterName);
		return TRUE;
	    } else
		return FALSE;
	case RHD_OUTPUT_FREE:
		TransmitterConfig->Encoder = Private->EncoderId = atomEncoderNone;
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
struct rhdOutput *
RHDAtomOutputInit(RHDPtr rhdPtr, rhdConnectorType ConnectorType,
		  rhdOutputType OutputType)
{
    struct rhdOutput *Output;
    struct rhdAtomOutputPrivate *Private;
    struct atomEncoderConfig *EncoderConfig;
    struct atomTransmitterConfig *TransmitterConfig;
    char *OutputName = NULL;

    RHDFUNC(rhdPtr);

    switch (OutputType) {
	case RHD_OUTPUT_NONE:
	    return NULL;
	case  RHD_OUTPUT_DACA:
	    OutputName = "DACA";
	    break;
	case RHD_OUTPUT_DACB:
	    OutputName = "DACB";
	    break;
	case RHD_OUTPUT_TMDSA:
	    OutputName = "TMDSA";
	    break;
	case RHD_OUTPUT_LVTMA:
	    OutputName = "LVTMA";
	    break;
	case RHD_OUTPUT_DVO:
	    OutputName = "DVO";
	    break;
	case RHD_OUTPUT_KLDSKP_LVTMA:
	    OutputName = "KldskpLvtma";
	    break;
	case RHD_OUTPUT_UNIPHYA:
	    OutputName = "UniphyA";
	    break;
	case RHD_OUTPUT_UNIPHYB:
	    OutputName = "UniphyB";
	    break;
	case RHD_OUTPUT_UNIPHYC:
	    OutputName = "UniphyC";
	    break;
	case RHD_OUTPUT_UNIPHYD:
	    OutputName = "UniphyD";
	    break;
	case RHD_OUTPUT_UNIPHYE:
	    OutputName = "UniphyE";
	    break;
	case RHD_OUTPUT_UNIPHYF:
	    OutputName = "UniphyF";
	    break;
    }

    Output = xnfcalloc(sizeof(struct rhdOutput), 1);
    Output->scrnIndex = rhdPtr->scrnIndex;

    Output->Name = RhdAppendString(NULL, "AtomOutput");
    Output->Name = RhdAppendString(Output->Name, OutputName);

    Output->Id = OutputType;
    Output->Sense = NULL;
    Private = xnfcalloc(sizeof(struct rhdAtomOutputPrivate), 1);
    Output->Private = Private;
    Output->OutputDriverPrivate = NULL;

    EncoderConfig = &Private->EncoderConfig;
    Private->PixelClock = 0;

    switch (OutputType) {
        case RHD_OUTPUT_NONE:
	    xfree(Output);
	    xfree(Private);
	    return NULL;
	case RHD_OUTPUT_DACA:
	    Output->Sense = RHDBIOSScratchDACSense;
	    Private->EncoderId = atomEncoderDACA;
	    Private->OutputControlId = atomDAC1Output;
	    Private->Hdmi = NULL;
	    break;
	case RHD_OUTPUT_DACB:
	    Output->Sense = RHDBIOSScratchDACSense;
	    Private->EncoderId = atomEncoderDACB;
	    Private->OutputControlId = atomDAC2Output;
	    Private->Hdmi = NULL;
	    break;
	case RHD_OUTPUT_TMDSA:
	case RHD_OUTPUT_LVTMA:
	    if (OutputType == RHD_OUTPUT_LVTMA) {
		if (ConnectorType == RHD_CONNECTOR_PANEL) {
		    Private->OutputControlId = atomLCDOutput;
		    LVDSInfoRetrieve(rhdPtr, Private);
		    Private->RunDualLink = Private->DualLink;
		    Private->EncoderId = atomEncoderLVDS;
		} else {
		    TMDSInfoRetrieve(rhdPtr, Private);
		    Private->OutputControlId = atomLVTMAOutput;
		    Private->EncoderId = atomEncoderTMDS2;
		}
	    } else {
		TMDSInfoRetrieve(rhdPtr, Private);
		Private->OutputControlId = atomTMDSAOutput;
		Private->EncoderId = atomEncoderTMDS1;
	    }

	    if (OutputType == RHD_CONNECTOR_DVI)
		Private->DualLink = TRUE;
	    else
		Private->DualLink = FALSE;

	    if (ConnectorType != RHD_CONNECTOR_PANEL)
		Private->Hdmi = RHDHdmiInit(rhdPtr, Output);
	    else
		Private->Hdmi = NULL;

	    Private->EncoderVersion = rhdAtomEncoderControlVersion(rhdPtr->atomBIOS, Private->EncoderId);
	    switch (Private->EncoderVersion.cref) {
		case 1:
		    EncoderConfig->u.lvds.Is24bit = Private->LVDS24Bit;
		    break;
		case 2:
		case 3:
		    EncoderConfig->u.lvds2.Is24bit = Private->LVDS24Bit;
		    EncoderConfig->u.lvds2.SpatialDither = Private->SpatialDither;
		    EncoderConfig->u.lvds2.LinkB = 0; /* @@@ */
		    EncoderConfig->u.lvds2.Hdmi = FALSE;
#if 0
		    if (ConnectorType == RHD_CONNECTOR_HDMI_B
			|| ConnectorType == RHD_CONNECTOR_HDMI_A)
			EncoderConfig->u.lvds2.hdmi = TRUE;
#endif
		    switch (Private->GreyLevel) {
			case 2:
			    EncoderConfig->u.lvds2.TemporalGrey = atomTemporalDither2;
			    break;
			case 4:
			    EncoderConfig->u.lvds2.TemporalGrey = atomTemporalDither4;
			    break;
			case 0:
			default:
			    EncoderConfig->u.lvds2.TemporalGrey = atomTemporalDither0;
		    }
		    if (Private->SpatialDither)
			EncoderConfig->u.lvds2.SpatialDither = TRUE;
		    else
			EncoderConfig->u.lvds2.SpatialDither = FALSE;
		    EncoderConfig->u.lvds2.Coherent = Private->Coherent;
		    break;
	    }
	    break;
	case RHD_OUTPUT_DVO:
	    Private->EncoderId = atomEncoderDVO;
	    Private->EncoderVersion = rhdAtomEncoderControlVersion(rhdPtr->atomBIOS,
								   Private->EncoderId);
	    switch (Private->EncoderVersion.cref) {
		case 1:
		case 2:
		    /* Output->OutputDriverPrivate->Device not set yet. */
		    break;
		case 3:  /* @@@ still to be handled */
		    xfree(Output);
		    xfree(Private);
		    return NULL;
	    }
	    {
		struct atomCodeTableVersion version = rhdAtomOutputControlVersion(rhdPtr->atomBIOS, atomDVOOutput);
		switch (version.cref) {
		    case 1:
		    case 2:
			Private->OutputControlId = atomDVOOutput;
			break;
		    case 3:
#if 0
			Private->TransmitterId = atomTransmitterDVO;    /* @@@ check how to handle this one */
			break;
#else
			xfree(Output);
			xfree(Private);
			return NULL;
#endif
		}
	    }
	    break;
	case RHD_OUTPUT_KLDSKP_LVTMA:
	    Private->EncoderVersion = rhdAtomEncoderControlVersion(rhdPtr->atomBIOS,
								   Private->EncoderId);
	    Output->AllocFree = RHDAtomOutputAllocFree;
	    EncoderConfig->u.dig.Link = atomTransLinkA;
	    EncoderConfig->u.dig.Transmitter = Private->TransmitterId = atomTransmitterLVTMA;

	    TransmitterConfig = &Private->TransmitterConfig;
	    TransmitterConfig->Link = atomTransLinkA;
	    TransmitterConfig->Encoder =  Private->TransmitterId;

	    if (ConnectorType == RHD_CONNECTOR_PANEL) {
		TransmitterConfig->Mode = EncoderConfig->u.dig.EncoderMode = atomLVDS;
		LVDSInfoRetrieve(rhdPtr, Private);
		Private->Hdmi = NULL;
	    } else {
		TransmitterConfig->Mode = EncoderConfig->u.dig.EncoderMode = atomDVI;
		TMDSInfoRetrieve(rhdPtr, Private);
			Private->Coherent = FALSE;
		Private->Hdmi = RHDHdmiInit(rhdPtr, Output);
	    }
	    break;

	case RHD_OUTPUT_UNIPHYA:
	case RHD_OUTPUT_UNIPHYB:
	case RHD_OUTPUT_UNIPHYC:
	case RHD_OUTPUT_UNIPHYD:
	case RHD_OUTPUT_UNIPHYE:
	case RHD_OUTPUT_UNIPHYF:
	    Output->AllocFree = RHDAtomOutputAllocFree;
	    if (RHDIsIGP(rhdPtr->ChipSet))
		EncoderConfig->u.dig.Transmitter = Private->TransmitterId = atomTransmitterPCIEPHY;
	    else {
		switch (OutputType) {
		    case RHD_OUTPUT_UNIPHYA:
		    case RHD_OUTPUT_UNIPHYB:
		EncoderConfig->u.dig.Transmitter = Private->TransmitterId = atomTransmitterUNIPHY;
			break;
		    case RHD_OUTPUT_UNIPHYC:
		    case RHD_OUTPUT_UNIPHYD:
			EncoderConfig->u.dig.Transmitter = Private->TransmitterId = atomTransmitterUNIPHY1;
			break;
		    case RHD_OUTPUT_UNIPHYE:
		    case RHD_OUTPUT_UNIPHYF:
			EncoderConfig->u.dig.Transmitter = Private->TransmitterId = atomTransmitterUNIPHY2;
			break;
		    default:
		    xfree(Private);
		    xfree(Output);
		    return NULL;
		}
	    }

	    TransmitterConfig = &Private->TransmitterConfig;
	    TransmitterConfig->Encoder =  Private->EncoderId = atomEncoderNone;
	    switch (OutputType) {
		case RHD_OUTPUT_UNIPHYA:
		case RHD_OUTPUT_UNIPHYC:
		case RHD_OUTPUT_UNIPHYE:
		    TransmitterConfig->Link = EncoderConfig->u.dig.Link = atomTransLinkA;
		    break;
		case RHD_OUTPUT_UNIPHYB:
		case RHD_OUTPUT_UNIPHYD:
		case RHD_OUTPUT_UNIPHYF:
		    TransmitterConfig->Link = EncoderConfig->u.dig.Link = atomTransLinkB;
		    break;
		default:
		    xfree(Private);
		    xfree(Output);
		    return NULL;
	    }

	    if (RHDIsIGP(rhdPtr->ChipSet)) {
		AtomBiosArgRec data;
		data.val = 1;
		if (RHDAtomBiosFunc(rhdPtr->scrnIndex, rhdPtr->atomBIOS, ATOM_GET_PCIE_LANES,
				    &data) == ATOM_SUCCESS)
		    TransmitterConfig->Lanes = data.pcieLanes.Chassis;
		/* only do 'chassis' for now */
		else {
		    xfree(Private);
		    xfree(Output);
		    return NULL;
		}
	    }

	    if (ConnectorType == RHD_CONNECTOR_PANEL)
		LVDSInfoRetrieve(rhdPtr, Private);
	    else
		TMDSInfoRetrieve(rhdPtr, Private);

	    switch (ConnectorType) {
		case RHD_CONNECTOR_DVI:
		case RHD_CONNECTOR_DVI_SINGLE:
		    TransmitterConfig->Mode = EncoderConfig->u.dig.EncoderMode = atomDVI;
		    Private->Hdmi = RHDHdmiInit(rhdPtr, Output);
		    break;
		case RHD_CONNECTOR_PANEL:
		    TransmitterConfig->Mode = EncoderConfig->u.dig.EncoderMode = atomLVDS;
		    break;
#if 0
		case RHD_CONNECTOR_DP:
		case RHD_CONNECTOR_DP_DUAL:
		    TransmitterConfig->Mode = EncoderConfig->u.dig.EncoderMode = atomDP;
		    break;
		case RHD_CONNECTOR_HDMI_A:
		case RHD_CONNECTOR_HDMI_B:
		    TransmitterConfig->Mode = EncoderConfig->u.dig.EncoderMode = atomHDMI;
		    break;
#endif
		default:
		    xf86DrvMsg(rhdPtr->scrnIndex, X_ERROR, "%s: Unknown connector type\n",__func__);
		    xfree(Output);
		    xfree(Private);
		    return NULL;
	    }
	    break;
	default:
	    xf86DrvMsg(rhdPtr->scrnIndex, X_ERROR, "Unknown output type\n");
	    xfree(Output);
	    xfree(Private);
	    return NULL;
    }
    if (ConnectorType == RHD_CONNECTOR_PANEL) {
	Output->Property = atomLVDSPropertyControl;
	LVDSInfoRetrieve(rhdPtr, Private);
    } else {
	Output->Property = atomTMDSPropertyControl;
	TMDSInfoRetrieve(rhdPtr, Private);
    }


    Output->Mode = rhdAtomOutputSet;
    Output->Power = rhdAtomOutputPower;
    Output->Save = rhdAtomOutputSave;
    Output->Restore = rhdAtomOutputRestore;
    Output->ModeValid = rhdAtomOutputModeValid;
    Output->Destroy = rhdAtomOutputDestroy;
    Private->CrtcSourceVersion = rhdAtomSelectCrtcSourceVersion(rhdPtr->atomBIOS);

    return Output;
}

/*
 * This sets up AtomBIOS based BL control if we need to use a non-standard method to control BL.
 */
int
RhdAtomSetupBacklightControlProperty(struct rhdOutput *Output,
				     Bool (**PropertyFunc)(struct rhdOutput *Output,
							   enum rhdPropertyAction Action,
							   enum rhdOutputProperty Property,
							   union rhdPropertyData *val), void **PrivatePtr)
{
    RHDPtr rhdPtr = RHDPTRI(Output);
    int BlLevel;
    struct rhdAtomOutputPrivate *Private;
    struct atomTransmitterConfig *TransmitterConfig;

    RHDFUNC(Output);

    Private = xnfcalloc(sizeof(struct rhdAtomOutputPrivate), 1);

    switch (Output->Id) {
	case RHD_OUTPUT_KLDSKP_LVTMA:
	case RHD_OUTPUT_UNIPHYE:
	case RHD_OUTPUT_UNIPHYF:
	    /* We set up a those parameters although they may never be needed for BL control */
	    TransmitterConfig = &Private->TransmitterConfig;
	    switch (Output->Id) {
		case RHD_OUTPUT_KLDSKP_LVTMA:
		    Private->TransmitterId = atomTransmitterLVTMA;
		    break;
		case RHD_OUTPUT_UNIPHYE:
		    Private->TransmitterId = atomTransmitterUNIPHY2;
		    TransmitterConfig->Link = atomTransLinkA;
		    break;
		case RHD_OUTPUT_UNIPHYF:
		    Private->TransmitterId = atomTransmitterUNIPHY2;
		    TransmitterConfig->Link = atomTransLinkB;
		    break;
		default:
		    return 0;  /* never get here */
	    }
	    TransmitterConfig = &Private->TransmitterConfig;
	    TransmitterConfig->Mode = atomLVDS;
	    if (rhdPtr->DigEncoderOutput[0] == Output)
		TransmitterConfig->Encoder =  Private->EncoderId = atomEncoderDIG1;
	    else if (rhdPtr->DigEncoderOutput[1] == Output)
		TransmitterConfig->Encoder =  Private->EncoderId = atomEncoderDIG2;
	    else
		TransmitterConfig->Encoder =  Private->EncoderId = atomEncoderNone;
	    LVDSInfoRetrieve(rhdPtr, Private);
	    Private->PixelClock = 0;
	    Private->Hdmi = NULL;
	    break;
	case RHD_OUTPUT_LVTMA:
	    Private->OutputControlId = atomLCDOutput;
	    break;
	default:
	    xfree(Private);
	    return 0;
    }
    *PropertyFunc = atomLVDSPropertyControl;
    *PrivatePtr = Private;
    RHDAtomBIOSScratchBlLevel(rhdPtr, rhdBIOSScratchBlGet, &BlLevel);

    return BlLevel;
}

void
RhdAtomDestroyBacklightControlProperty(struct rhdOutput *Output, void *PrivatePtr)
{
    if (PrivatePtr)
	xfree(PrivatePtr);
}

#endif /* ATOM_BIOS && ATOM_BIOS_PARSER */
