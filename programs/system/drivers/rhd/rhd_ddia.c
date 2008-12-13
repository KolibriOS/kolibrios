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
#ifdef ATOM_BIOS
#include "rhd_atombios.h"
#endif

struct DDIAPrivate
{
    Bool RunDualLink;
    CARD32 PcieCfgReg7;
    CARD32 CapabilityFlag;

    Bool Stored;

    CARD32 DdiaPathControl;
    CARD32 DdiaCntl;
    CARD32 DdiaDcbalancerControl;
    CARD32 DdiaPcieLinkControl2;
    CARD32 DdiaBitDepthControl;
};

/*
 *
 */
static ModeStatus
DDIAModeValid(struct rhdOutput *Output, DisplayModePtr Mode)
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
DDIAMode(struct rhdOutput *Output, DisplayModePtr Mode)
{
    struct DDIAPrivate *Private = (struct DDIAPrivate *)Output->Private;
    CARD32 mux0, mux1, mux2, mux3;
    Bool LaneReversal;
    RHDPtr rhdPtr = RHDPTRI(Output);

    RHDFUNC(Output);

    if (Mode->SynthClock >= 165000)
	Private->RunDualLink = TRUE;
    else
	Private->RunDualLink = FALSE;

    /* reset on - will be enabled at POWER_ON */
    RHDRegMask(Output, RS69_DDIA_PATH_CONTROL, RS69_DDIA_PIXVLD_RESET, RS69_DDIA_PIXVLD_RESET);
    /* RGB 4:4:4 */
    RHDRegMask(Output, RS69_DDIA_CNTL, 0, RS69_DDIA_PIXEL_ENCODING);
    /* TMDS_AC */
    RHDRegMask(Output, RS69_DDIA_PATH_CONTROL,
	       2 << RS69_DDIA_PATH_SELECT_SHIFT,
	       0x3 << RS69_DDIA_PATH_SELECT_SHIFT);
    /* dual link */
    RHDRegMask(Output, RS69_DDIA_CNTL, Private->RunDualLink ?
	       RS69_DDIA_DUAL_LINK_ENABLE : 0, RS69_DDIA_DUAL_LINK_ENABLE);
    RHDRegMask(Output, RS69_DDIA_DCBALANCER_CONTROL,
	       RS69_DDIA_DCBALANCER_EN,
	       RS69_DDIA_SYNC_DCBAL_EN_MASK | RS69_DDIA_DCBALANCER_EN);

    RHDRegMask(Output,  RS69_DDIA_PCIE_PHY_CONTROL2, 0x0, 0x80);
    RHDRegMask(Output,  RS69_DDIA_PCIE_PHY_CONTROL2, 0x0, 0x100);

    mux0 = Private->PcieCfgReg7 & 0x3;
    mux1 = (Private->PcieCfgReg7 >> 2) & 0x3;
    mux2 = (Private->PcieCfgReg7 >> 4) & 0x3;
    mux3 = (Private->PcieCfgReg7 >> 6) & 0x3;

    RHDRegMask(Output, RS69_DDIA_PCIE_LINK_CONTROL2,
	       (mux0 << RS69_DDIA_PCIE_OUTPUT_MUX_SEL0)
	       | (mux1 << RS69_DDIA_PCIE_OUTPUT_MUX_SEL1)
	       | (mux2 << RS69_DDIA_PCIE_OUTPUT_MUX_SEL2)
	       | (mux3 << RS69_DDIA_PCIE_OUTPUT_MUX_SEL3),
	       (3 << RS69_DDIA_PCIE_OUTPUT_MUX_SEL0)
	       | (3 << RS69_DDIA_PCIE_OUTPUT_MUX_SEL1)
	       | (3 << RS69_DDIA_PCIE_OUTPUT_MUX_SEL2)
	       | (3 << RS69_DDIA_PCIE_OUTPUT_MUX_SEL3)
	);
    LaneReversal = Private->PcieCfgReg7 & (0x1 << 10);
    RHDRegMask(Output,  RS69_DDIA_PCIE_PHY_CONTROL2, 0x0, 0x3);
    RHDRegMask(Output,  RS69_DDIA_PCIE_PHY_CONTROL2, 0x2, 0x2);

    RHDRegMask(Output, RS69_DDIA_PCIE_LINK_CONTROL3,
	       LaneReversal ? RS69_DDIA_PCIE_MIRROR_EN : 0,
	       RS69_DDIA_PCIE_MIRROR_EN);

    RHDRegMask(Output,  RS69_DDIA_PCIE_PHY_CONTROL2, 0x70, 0x70);

    RHDRegMask(Output,  RS69_DDIA_PCIE_PHY_CONTROL1, 0, 0x10);
    RHDRegMask(Output,  RS69_DDIA_PCIE_PHY_CONTROL1, 0, 0x60);
    RHDRegMask(Output,  RS69_DDIA_PCIE_PHY_CONTROL1, 0, 0x4000000);

    switch (rhdPtr->PciDeviceID) {
	case 0x791E:
	    if (Mode->SynthClock <= 25000) {
		RHDRegMask(Output,  RS69_DDIA_PCIE_PHY_CONTROL1, 0x2780, 0x3f80);
		RHDRegMask(Output,  RS69_DDIA_PCIE_PHY_CONTROL1, 0x0, 0xc000);
		RHDRegMask(Output,  RS69_DDIA_PCIE_PHY_CONTROL1, 0x039f0000, 0x03000000 | 0x039f0000);
	    } else if (Mode->SynthClock <= 60000) {
		RHDRegMask(Output,  RS69_DDIA_PCIE_PHY_CONTROL1, 0x2780, 0x3f80);
		RHDRegMask(Output,  RS69_DDIA_PCIE_PHY_CONTROL1, 0x0, 0xc000);
		RHDRegMask(Output,  RS69_DDIA_PCIE_PHY_CONTROL1, 0x024f0000, 0x03000000 | 0x024f0000);
	    } else {
		RHDRegMask(Output,  RS69_DDIA_PCIE_PHY_CONTROL1, 0x0980, 0x3f80);
		RHDRegMask(Output,  RS69_DDIA_PCIE_PHY_CONTROL1, 0x0, 0xc000);
		RHDRegMask(Output,  RS69_DDIA_PCIE_PHY_CONTROL1, 0x01270000, 0x03000000 | 0x01270000);
	    }
	    break;
	case 0x791F:
		RHDRegMask(Output,  RS69_DDIA_PCIE_PHY_CONTROL1, 0x0980, 0x3f80);
		RHDRegMask(Output,  RS69_DDIA_PCIE_PHY_CONTROL1, 0x4000, 0xc000);
		RHDRegMask(Output,  RS69_DDIA_PCIE_PHY_CONTROL1, 0x00ac0000, 0x03000000 | 0x00ac0000);
		if (Private->CapabilityFlag & 0x10) {
		    RHDRegMask(Output,  RS69_DDIA_PCIE_PHY_CONTROL1, 0x0, 0xc000);
		    if (Mode->SynthClock <= 6500)
			RHDRegMask(Output,  RS69_DDIA_PCIE_PHY_CONTROL1, 0x01ac0000, 0x03ff0000);
		    else
			RHDRegMaskD(Output,  RS69_DDIA_PCIE_PHY_CONTROL1, 0x01110000, 0x03ff0000);
		}
	    break;
    }
    usleep (1);
    RHDRegMask(Output, RS69_DDIA_PCIE_PHY_CONTROL1, 0x04000000, 0x04000000);
    RHDRegMask(Output, RS69_DDIA_PCIE_PHY_CONTROL1, 0x60, 0x60);
    usleep(30);
    RHDRegMask(Output, RS69_DDIA_PCIE_PHY_CONTROL1, 0x01, 0x01);
    usleep(1);
    RHDRegMask(Output, RS69_DDIA_PCIE_PHY_CONTROL1, 0x02, 0x02);
    usleep(1);
    RHDRegMask(Output, RS69_DDIA_PCIE_PHY_CONTROL1, 0x04, 0x04);
    usleep(1);
    RHDRegMask(Output, RS69_DDIA_PCIE_PHY_CONTROL1, 0x08, 0x08);
    usleep(1);
    RHDRegMask(Output, RS69_DDIA_PCIE_PHY_CONTROL1, 0x10, 0x10);
    RHDRegMask(Output, RS69_DDIA_PCIE_PHY_CONTROL1, 0x0, 0xf);

    RHDRegMask(Output, RS69_DDIA_PCIE_PHY_CONTROL2, 0x0180, 0x0180);
    RHDRegMask(Output, RS69_DDIA_PCIE_PHY_CONTROL2, 0x600, 0x600);
    usleep(5);
    RHDRegMask(Output, RS69_DDIA_PCIE_PHY_CONTROL2, 0x0, 0x600);

    /* hw reset will be turned off at POWER_ON */

    /* select crtc source, sync_a, no stereosync */
    RHDRegMask(Output, RS69_DDIA_SOURCE_SELECT, Output->Crtc->Id,
	       RS69_DDIA_SOURCE_SELECT_BIT
	       | RS69_DDIA_SYNC_SELECT
	       | RS69_DDIA_STEREOSYNC_SELECT);
}

/*
 *
 */
static void
DDIAPower(struct rhdOutput *Output, int Power)
{
    RHDDebug(Output->scrnIndex, "%s(%s,%s)\n",__func__,Output->Name,
	     rhdPowerString[Power]);

    switch (Power) {
	case RHD_POWER_ON:
	    RHDRegMask(Output, RS69_DDIA_PATH_CONTROL, RS69_DDIA_PIXVLD_RESET,
		       RS69_DDIA_PIXVLD_RESET);
	    RHDRegWrite(Output, RS69_DDIA_BIT_DEPTH_CONTROL, 0);
	    RHDRegMask(Output, RS69_DDIA_BIT_DEPTH_CONTROL,
		       RS69_DDIA_TEMPORAL_DITHER_RESET, RS69_DDIA_TEMPORAL_DITHER_RESET);
	    RHDRegMask(Output, RS69_DDIA_BIT_DEPTH_CONTROL,
		       0, RS69_DDIA_TEMPORAL_DITHER_RESET);
	    RHDRegMask(Output, RS69_DDIA_CNTL, RS69_DDIA_ENABLE, RS69_DDIA_ENABLE);
	    RHDRegMask(Output, RS69_DDIA_PATH_CONTROL, 0, RS69_DDIA_PIXVLD_RESET);
	    return;
	case RHD_POWER_RESET:
	    RHDRegMask(Output, RS69_DDIA_CNTL, 0, RS69_DDIA_ENABLE);
	    return;
	case RHD_POWER_SHUTDOWN:
	    RHDRegMask(Output, RS69_DDIA_BIT_DEPTH_CONTROL,
		       RS69_DDIA_TEMPORAL_DITHER_RESET, RS69_DDIA_TEMPORAL_DITHER_RESET);
	    RHDRegMask(Output, RS69_DDIA_BIT_DEPTH_CONTROL,
		       0, RS69_DDIA_TEMPORAL_DITHER_RESET);
	    RHDRegMask(Output, RS69_DDIA_BIT_DEPTH_CONTROL,
		       0,
		       RS69_DDIA_TRUNCATE_EN
		       | RS69_DDIA_TRUNCATE_DEPTH
		       | RS69_DDIA_SPATIAL_DITHER_EN
		       | RS69_DDIA_SPATIAL_DITHER_DEPTH);
	    RHDRegMask(Output, RS69_DDIA_BIT_DEPTH_CONTROL,
		       0,
		       RS69_DDIA_TEMPORAL_DITHER_EN
		       | RS69_DDIA_TEMPORAL_DITHER_EN
		       | RS69_DDIA_TEMPORAL_DITHER_DEPTH
		       | RS69_DDIA_TEMPORAL_LEVEL);
	    RHDRegMask(Output, RS69_DDIA_CNTL, 0, RS69_DDIA_ENABLE);
	    return;
	default:
	    return;
    }
}

/*
 *
 */
static void
DDIASave(struct rhdOutput *Output)
{
    struct DDIAPrivate *Private = (struct DDIAPrivate *)Output->Private;

    RHDFUNC(Output);

    Private->DdiaPathControl = RHDRegRead(Output, RS69_DDIA_PATH_CONTROL);
    Private->DdiaCntl = RHDRegRead(Output, RS69_DDIA_CNTL);
    Private->DdiaDcbalancerControl = RHDRegRead(Output, RS69_DDIA_DCBALANCER_CONTROL);
    Private->DdiaPcieLinkControl2 = RHDRegRead(Output, RS69_DDIA_PCIE_LINK_CONTROL2);
    Private->DdiaBitDepthControl = RHDRegRead(Output, RS69_DDIA_BIT_DEPTH_CONTROL);

    Private->Stored = TRUE;
}

/*
 *
 */
static void
DDIARestore(struct rhdOutput *Output)
{
    struct DDIAPrivate *Private = (struct DDIAPrivate *)Output->Private;
    RHDFUNC(Output);

    if (!Private->Stored)
	return;

    /* disalbe */
    RHDRegMask(Output, RS69_DDIA_CNTL, 0, RS69_DDIA_ENABLE);
    /* reset on */
    RHDRegMask(Output, RS69_DDIA_PATH_CONTROL, RS69_DDIA_PIXVLD_RESET, RS69_DDIA_PIXVLD_RESET);
    RHDRegWrite(Output, RS69_DDIA_PATH_CONTROL, Private->DdiaPathControl | RS69_DDIA_PIXVLD_RESET);

    RHDRegWrite(Output, RS69_DDIA_BIT_DEPTH_CONTROL, Private->DdiaBitDepthControl);
    /* temporal dither reset on */
    RHDRegWrite(Output, RS69_DDIA_BIT_DEPTH_CONTROL, Private->DdiaBitDepthControl
	       | RS69_DDIA_TEMPORAL_DITHER_RESET);
    /* temporal dither reset off */
    RHDRegWrite(Output, RS69_DDIA_BIT_DEPTH_CONTROL, Private->DdiaBitDepthControl);

    RHDRegWrite(Output, RS69_DDIA_DCBALANCER_CONTROL, Private->DdiaDcbalancerControl);
    RHDRegWrite(Output, RS69_DDIA_PCIE_LINK_CONTROL2, Private->DdiaPcieLinkControl2);
    /* enable if enabled at startup */
    RHDRegWrite(Output, RS69_DDIA_CNTL, Private->DdiaCntl);
    /* reset off */
    RHDRegWrite(Output, RS69_DDIA_PATH_CONTROL, Private->DdiaPathControl);
}

/*
 *
 */
static void
DDIADestroy(struct rhdOutput *Output)
{
    struct DDIAPrivate *Private = (struct DDIAPrivate *)Output->Private;

    RHDFUNC(Output);

    xfree(Private);
    Output->Private = NULL;
}

/*
 *
 */
struct rhdOutput *
RHDDDIAInit(RHDPtr rhdPtr)
{
#ifdef ATOM_BIOS
    struct rhdOutput *Output;
    struct DDIAPrivate *Private;
    AtomBiosArgRec data;

    RHDFUNC(rhdPtr);

    /*
     * This needs to be handled separately
     * for now we only deal with it here.
     */
    if (rhdPtr->ChipSet < RHD_RS600 || rhdPtr->ChipSet >= RHD_RS740)
	return FALSE;

    Output = xnfcalloc(sizeof(struct rhdOutput), 1);

    Output->Name = "DDIA";

    Output->scrnIndex = rhdPtr->scrnIndex;
    Output->Id = RHD_OUTPUT_DVO;

    Output->Sense = NULL;
    Output->ModeValid = DDIAModeValid;
    Output->Mode = DDIAMode;
    Output->Power = DDIAPower;
    Output->Save = DDIASave;
    Output->Restore = DDIARestore;
    Output->Destroy = DDIADestroy;

    Private = xnfcalloc(1, sizeof(struct DDIAPrivate));
    Output->Private = Private;
    Private->Stored = FALSE;

    if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			ATOM_GET_PCIENB_CFG_REG7, &data) == ATOM_SUCCESS) {
	Private->PcieCfgReg7 = data.val;
    } else {
	xf86DrvMsg(Output->scrnIndex, X_ERROR, "Retrieval of PCIE MUX values failed. "
		   "no DDIA block support available\n");
	goto error;
    }
    if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			 ATOM_GET_CAPABILITY_FLAG, &data) == ATOM_SUCCESS) {
	Private->CapabilityFlag = data.val;
    } else {
	xf86DrvMsg(Output->scrnIndex, X_ERROR, "Retrieval of Capability flag failed. "
		   "no DDIA block support available\n");
	goto error;
    }

    return Output;
error:
    xfree(Private);
    return NULL;

#else
    return NULL;
#endif
}
