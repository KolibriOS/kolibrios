/*
 * Copyright 2007, 2008  Egbert Eich   <eich@novell.com>
 * Copyright 2007, 2008  Luc Verhaegen <lverhaegen@novell.com>
 * Copyright 2007, 2008  Matthias Hopf <mhopf@novell.com>
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
# include "config.h"
#endif

#ifdef ATOM_BIOS
# include "xf86.h"
#include "rhd.h"

# include "edid.h"

# include "xf86DDC.h"

# if HAVE_XF86_ANSIC_H
#  include "xf86_ansic.h"
# else
#  include <unistd.h>
#  include <string.h>
#  include <stdio.h>
# endif



# include "rhd_atombios.h"

# include "rhd_connector.h"
# include "rhd_output.h"
# include "rhd_biosscratch.h"
# include "rhd_crtc.h"
# include "rhd_card.h"

# ifdef ATOM_BIOS_PARSER
#  define INT8 INT8
#  define INT16 INT16
#  define INT32 INT32
#  include "CD_Common_Types.h"
# else
#  ifndef ULONG
typedef unsigned int ULONG;
#   define ULONG ULONG
#  endif
#  ifndef UCHAR
typedef unsigned char UCHAR;
#   define UCHAR UCHAR
#  endif
#  ifndef USHORT
typedef unsigned short USHORT;
#   define USHORT USHORT
#  endif
# endif

# include "atombios.h"

struct rhdOutputDevices {
    enum atomDevice DeviceId;
    enum rhdConnectorType ConnectorType;
};

#if defined (ATOM_BIOS_PARSER)
/*
 *
 */
static enum rhdSensedOutput
rhdAtomBIOSScratchDACSenseResults(struct rhdOutput *Output, enum atomDAC DAC, enum atomDevice Device)
{
    RHDPtr rhdPtr = RHDPTRI(Output);
    CARD32 BIOS_0;
    Bool TV = FALSE;

    RHDFUNC(Output);

    if (rhdPtr->ChipSet < RHD_R600)
	BIOS_0 = RHDRegRead(Output, 0x10);
    else
	BIOS_0 = RHDRegRead(Output, 0x1724);

    switch (Device) {
    	case atomNone:
	case atomCRT2:
    	case atomCRT1:
	case atomLCD1:
	case atomLCD2:
	case atomDFP1:
	case atomDFP2:
	case atomDFP3:
	case atomDFP4:
	case atomDFP5:
	    TV = FALSE;
	    break;
	case atomTV1:
	case atomTV2:
	case atomCV:
	    TV = TRUE;
	    break;
    }

    RHDDebug(Output->scrnIndex, "BIOSScratch_0: 0x%4.4x\n",BIOS_0);

    switch (DAC) {
	case atomDACA:
	    break;
	case atomDACB:
	    BIOS_0 >>= 8;
	    break;
	case atomDACExt:
	    return RHD_SENSED_NONE;
    }

    if (!TV) {
	if (BIOS_0 & ATOM_S0_CRT1_MASK) {
	    RHDDebug(Output->scrnIndex, "%s sensed RHD_SENSED_VGA\n",__func__);
	    return RHD_SENSED_VGA;
	}
    } else {
	if (BIOS_0 & ATOM_S0_TV1_COMPOSITE_A) {
	    RHDDebug(Output->scrnIndex, "%s: RHD_SENSED_TV_COMPOSITE\n",__func__);
	    return RHD_SENSED_TV_COMPOSITE;
	} else if (BIOS_0 & ATOM_S0_TV1_SVIDEO_A) {
	    RHDDebug(Output->scrnIndex, "%s: RHD_SENSED_TV_SVIDE\n",__func__);
	    return RHD_SENSED_TV_SVIDEO;
	} else if (BIOS_0 & ATOM_S0_CV_MASK_A) {
	    RHDDebug(Output->scrnIndex, "%s: RHD_SENSED_TV_COMPONENT\n",__func__);
	    return RHD_SENSED_TV_COMPONENT;
	}
    }

    RHDDebug(Output->scrnIndex, "%s: RHD_SENSED_NONE\n",__func__);
    return RHD_SENSED_NONE;
}

/*
 *
 */
enum rhdSensedOutput
RHDBIOSScratchDACSense(struct rhdOutput *Output, struct rhdConnector *Connector)
{
    RHDPtr rhdPtr = RHDPTRI(Output);
    enum atomDAC DAC;
    Bool ret;
    Bool TV;
    enum atomDevice Device;
    enum rhdSensedOutput retVal;
    int i = 0;

    RHDFUNC(Output);

    if (!Output->OutputDriverPrivate)
	return RHD_SENSED_NONE;

    switch (Output->Id) {
	case RHD_OUTPUT_DACA:
	    RHDDebug(Output->scrnIndex, "Sensing DACA on Output %s\n",Output->Name);
	    DAC = atomDACA;
	    break;
	case RHD_OUTPUT_DACB:
	    RHDDebug(Output->scrnIndex, "Sensing DACB on Output %s\n",Output->Name);
	    DAC = atomDACB;
	    break;
	default:
	    return FALSE;
    }

    switch (Connector->Type) {
	case RHD_CONNECTOR_DVI:
	case RHD_CONNECTOR_DVI_SINGLE:
	case RHD_CONNECTOR_VGA:
	    TV = FALSE;
	    break;
	default:
	    TV = TRUE;
    }

    while ((Device = Output->OutputDriverPrivate->OutputDevices[i++].DeviceId) != atomNone) {
	switch (Device) {
	    case atomCRT1:
	    case atomCRT2:
		if (TV)
		    continue;
		break;
	    case atomTV1:
	    case atomTV2:
	    case atomCV:
		if (!TV)
		    continue;
		break;
	    default: /* should not get here */
		return RHD_SENSED_NONE;
	}

	ret = AtomDACLoadDetection(rhdPtr->atomBIOS, Device, DAC);

	if (!ret)
	    continue;

	if ((retVal =  rhdAtomBIOSScratchDACSenseResults(Output, DAC, Device)) != RHD_SENSED_NONE)
	    return retVal;
    }
    return RHD_SENSED_NONE;
}
# endif /* ATOM_BIOS_PARSER */
/*
 *
 */
static void
rhdAtomBIOSScratchUpdateAttachedState(RHDPtr rhdPtr, enum atomDevice dev, Bool attached)
{
    CARD32 BIOS_0;
    CARD32 Addr;
    CARD32 Mask;

    RHDFUNC(rhdPtr);

    if (rhdPtr->ChipSet < RHD_R600)
	Addr = 0x10;
    else
	Addr = 0x1724;

    BIOS_0 = RHDRegRead(rhdPtr, Addr);

    switch (dev) {
	case atomDFP1:
	    Mask = ATOM_S0_DFP1;
	    break;
	case atomDFP2:
	    Mask = ATOM_S0_DFP2;
	    break;
	case atomLCD1:
	    Mask = ATOM_S0_LCD1;
	    break;
	case atomLCD2:
	    Mask = ATOM_S0_LCD2;
	    break;
	case atomTV2:
	    Mask = ATOM_S0_TV2;
	    break;
	case atomDFP3:
	    Mask = ATOM_S0_DFP3;
	    break;
	case atomDFP4:
	    Mask = ATOM_S0_DFP4;
	    break;
	case atomDFP5:
	    Mask = ATOM_S0_DFP5;
	    break;
	default:
	    return;
    }
    if (attached)
	BIOS_0 |= Mask;
    else
	BIOS_0 &= ~Mask;

    RHDRegWrite(rhdPtr, Addr, BIOS_0);
}

/*
 *
 */
static void
rhdAtomBIOSScratchUpdateOnState(RHDPtr rhdPtr, enum atomDevice dev, Bool on)
{
    CARD32 BIOS_3;
    CARD32 Addr;
    CARD32 Mask = 0;

    RHDFUNC(rhdPtr);

    if (rhdPtr->ChipSet < RHD_R600)
	Addr = 0x1C;
    else
	Addr = 0x1730;

    BIOS_3 = RHDRegRead(rhdPtr, Addr);

    switch (dev) {
	case atomCRT1:
	    Mask = ATOM_S3_CRT1_ACTIVE;
	    break;
	case atomLCD1:
	    Mask = ATOM_S3_LCD1_ACTIVE;
	    break;
	case atomTV1:
	    Mask = ATOM_S3_TV1_ACTIVE;
	    break;
	case atomDFP1:
	    Mask =  ATOM_S3_DFP1_ACTIVE;
	    break;
	case atomCRT2:
	    Mask = ATOM_S3_CRT2_ACTIVE;
	    break;
	case atomLCD2:
	    Mask = ATOM_S3_LCD2_ACTIVE;
	    break;
	case atomTV2:
	    Mask = ATOM_S3_TV2_ACTIVE;
	    break;
	case atomDFP2:
	    Mask = ATOM_S3_DFP2_ACTIVE;
	    break;
	case  atomCV:
	    Mask = ATOM_S3_CV_ACTIVE;
	    break;
	case atomDFP3:
	    Mask = ATOM_S3_DFP3_ACTIVE;
	    break;
	case atomDFP4:
	    Mask = ATOM_S3_DFP4_ACTIVE;
	    break;
	case atomDFP5:
	    Mask = ATOM_S3_DFP5_ACTIVE;
	    break;
	case atomNone:
	    return;
    }
    if (on)
	BIOS_3 |= Mask;
    else
	BIOS_3 &= ~Mask;

    RHDRegWrite(rhdPtr, Addr, BIOS_3);
}

/*
 *
 */
void
RHDAtomBIOSScratchSetAccelratorMode(RHDPtr rhdPtr, Bool on)
{
    CARD32 Addr;
    CARD32 Mask = ATOM_S6_ACC_MODE | ATOM_S6_ACC_BLOCK_DISPLAY_SWITCH;

    if (rhdPtr->ChipSet < RHD_R600)
	Addr = 0x10 + (6 << 2);
    else
	Addr = 0x1724 + (6 << 2);

    RHDRegMask(rhdPtr, Addr, on ? Mask : 0, Mask);
}

/*
 *
 */
static void
rhdAtomBIOSScratchSetAcceleratorModeForDevice(RHDPtr rhdPtr,
					      enum atomDevice Device, Bool on)
{
    CARD32 Addr;
    CARD32 Mask = 0;

    if (rhdPtr->ChipSet < RHD_R600)
	Addr = 0x10 + (6 << 2);
    else
	Addr = 0x1724 + (6 << 2);

    switch (Device) {
	case atomCRT1:
	    Mask = ATOM_S6_ACC_REQ_CRT1;
	    break;
	case atomLCD1:
	    Mask = ATOM_S6_ACC_REQ_LCD1;
	    break;
	case atomTV1:
	    Mask = ATOM_S6_ACC_REQ_TV1;
	    break;
	case atomDFP1:
	    Mask = ATOM_S6_ACC_REQ_DFP1;
	    break;
	case atomCRT2:
	    Mask = ATOM_S6_ACC_REQ_CRT2;
	    break;
	case atomLCD2:
	    Mask = ATOM_S6_ACC_REQ_LCD2;
	    break;
	case atomTV2:
	    Mask = ATOM_S6_ACC_REQ_TV2;
	    break;
	case atomDFP2:
	    Mask = ATOM_S6_ACC_REQ_DFP2;
	    break;
	case  atomCV:
	    Mask = ATOM_S6_ACC_REQ_CV;
	    break;
	case atomDFP3:
	    Mask = ATOM_S6_ACC_REQ_DFP3;
	    break;
	case atomDFP4:
	    Mask = ATOM_S6_ACC_REQ_DFP4;
	    break;
	case atomDFP5:
	    Mask = ATOM_S6_ACC_REQ_DFP5;
	    break;
	case atomNone:
	    return;
    }
    RHDRegMask(rhdPtr, Addr, on ? Mask : 0, Mask);
}

/*
 *
 */
static void
rhdAtomBIOSScratchSetCrtcState(RHDPtr rhdPtr, enum atomDevice dev, enum atomCrtc Crtc)
{
    CARD32 BIOS_3;
    CARD32 Addr;
    CARD32 Mask = 0;

    RHDFUNC(rhdPtr);

    if (rhdPtr->ChipSet < RHD_R600)
	Addr = 0x1C;
    else
	Addr = 0x1730;

    BIOS_3 = RHDRegRead(rhdPtr, Addr);

    switch (dev) {
	case atomCRT1:
	    Mask = ATOM_S3_CRT1_CRTC_ACTIVE;
	    break;
	case atomLCD1:
	    Mask = ATOM_S3_LCD1_CRTC_ACTIVE;
	    break;
	case atomTV1:
	    Mask = ATOM_S3_TV1_CRTC_ACTIVE;
	    break;
	case atomDFP1:
	    Mask =  ATOM_S3_DFP1_CRTC_ACTIVE;
	    break;
	case atomCRT2:
	    Mask = ATOM_S3_CRT2_CRTC_ACTIVE;
	    break;
	case atomLCD2:
	    Mask = ATOM_S3_LCD2_CRTC_ACTIVE;
	    break;
	case atomTV2:
	    Mask = ATOM_S3_TV2_CRTC_ACTIVE;
	    break;
	case atomDFP2:
	    Mask = ATOM_S3_DFP2_CRTC_ACTIVE;
	    break;
	case  atomCV:
	    Mask = ATOM_S3_CV_CRTC_ACTIVE;
	    break;
	case atomDFP3:
	    Mask = ATOM_S3_DFP3_CRTC_ACTIVE;
	    break;
	case atomDFP4:
	    Mask = ATOM_S3_DFP4_CRTC_ACTIVE;
	    break;
	case atomDFP5:
	    Mask = ATOM_S3_DFP5_CRTC_ACTIVE;
	    break;
	case atomNone:
	    return;
    }
    if (Crtc == atomCrtc2)
	BIOS_3 |= Mask;
    else
	BIOS_3 &= ~Mask;

    RHDRegWrite(rhdPtr, Addr, BIOS_3);
}

/*
 *
 */
void
RHDAtomBIOSScratchPMState(RHDPtr rhdPtr, struct rhdOutput *Output, int PowerManagementMode)
{
    CARD32 Addr;
    CARD32 Mask = 0, Mask1;
    enum atomDevice Device = Output->OutputDriverPrivate->Device;

    if (rhdPtr->ChipSet < RHD_R600)
	Addr = 0x10 + (2 << 2);
    else
	Addr = 0x1724 + (2 << 2);

    switch (Device) {
	case atomCRT1:
	    Mask = ATOM_S2_CRT1_DPMS_STATE;
	    break;
	case atomLCD1:
	    Mask = ATOM_S2_LCD1_DPMS_STATE;
	    break;
	case atomTV1:
	    Mask = ATOM_S2_TV1_DPMS_STATE;
	    break;
	case atomDFP1:
	    Mask = ATOM_S2_DFP1_DPMS_STATE;
	    break;
	case atomCRT2:
	    Mask = ATOM_S2_CRT2_DPMS_STATE;
	    break;
	case atomLCD2:
	    Mask = ATOM_S2_LCD2_DPMS_STATE;
	    break;
	case atomTV2:
	    Mask = ATOM_S2_TV2_DPMS_STATE;
	    break;
	case atomDFP2:
	    Mask = ATOM_S2_DFP2_DPMS_STATE;
	    break;
	case  atomCV:
	    Mask = ATOM_S2_CV_DPMS_STATE;
	    break;
	case atomDFP3:
	    Mask = ATOM_S2_DFP3_DPMS_STATE;
	    break;
	case atomDFP4:
	    Mask = ATOM_S2_DFP4_DPMS_STATE;
	    break;
	case atomDFP5:
	    Mask = ATOM_S2_DFP5_DPMS_STATE;
	    break;
	case atomNone:
	    return;
    }
    switch (PowerManagementMode) {
	case DPMSModeOn:
	    Mask1 = 0;
	    break;
	case DPMSModeStandby:
	case DPMSModeSuspend:
	case DPMSModeOff:
	default:
	    Mask1 = Mask;
	    break;
    }

    RHDRegMask(rhdPtr, Addr, Mask1, Mask);
}

/*
 *
 */
void
RHDAtomBIOSScratchBlLevel(RHDPtr rhdPtr, enum rhdBIOSScratchBlAction action, int *val)
{
    CARD32 Addr;

    RHDFUNC(rhdPtr);

    if (rhdPtr->ChipSet < RHD_R600)
	Addr = 0x18;
    else
	Addr = 0x172C;

    switch (action) {
	case rhdBIOSScratchBlGet:
	    *val = (RHDRegRead(rhdPtr, Addr) >> 8) & 0xFF;
	    RHDDebug(rhdPtr->scrnIndex, "Get BL level: 0x%x\n",*val);
	    break;
	case rhdBIOSScratchBlSet:
	    RHDDebug(rhdPtr->scrnIndex, "Set BL level: 0x%x\n",*val);
	    RHDRegMask(rhdPtr, Addr, (*val) << 8, 0xFF00);
	    break;
    }
}

/*
 * This function finds the AtomBIOS device ID of the device that we currently
 * want to drive with a specific output. It contains a logic to deal with CRTC vs. TV
 * on DACs.
 * This function preferrably gets called from within the function that also updates
 * the BIOS scratch registers.
 */
static enum atomDevice
rhdBIOSScratchSetDeviceForOutput(struct rhdOutput *Output)
{
    int i = 0;

    RHDFUNC(Output);

    if (!Output->Connector) {
	RHDDebug(Output->scrnIndex,"%s: No connector assigned to output %s\n",__func__,Output->Name);
	return atomNone;
    }

    if (!Output->OutputDriverPrivate) {
	RHDDebug(Output->scrnIndex,"%s: Output %s has no DriverPrivate\n",__func__,Output->Name);
	return atomNone;
    }

    while (Output->OutputDriverPrivate->OutputDevices[i].DeviceId != atomNone) {
	if (Output->OutputDriverPrivate->OutputDevices[i].ConnectorType == Output->Connector->Type){

	    switch (Output->OutputDriverPrivate->OutputDevices[i].DeviceId) {
		case atomCrtc1:
		case atomCrtc2:
		    if (Output->SensedType == RHD_SENSED_VGA
			|| Output->SensedType == RHD_SENSED_NONE) /* if nothing was sensed default to VGA */
			break;
		    i++;
		    continue;
		case atomTV1:
		case atomTV2:
		    if (Output->SensedType == RHD_SENSED_TV_SVIDEO
			|| Output->SensedType == RHD_SENSED_TV_COMPOSITE)
			break;
		    i++;
		    continue;
		case atomCV:
		    if (Output->SensedType == RHD_SENSED_TV_COMPONENT)
			break;
		    i++;
		    continue;
		default:
		    break;
	    }
	    Output->OutputDriverPrivate->Device = Output->OutputDriverPrivate->OutputDevices[i].DeviceId;

	    return Output->OutputDriverPrivate->Device;
	}
	i++;
    }
    RHDDebugVerb(Output->scrnIndex,1,"%s: No device found: ConnectorType: %2.2x SensedType: %2.2x\n",
	     __func__, Output->Connector->Type, Output->SensedType);
    return atomNone;
}

/*
 * This function is public as it is used from within other outputs, too.
 */
static enum atomDevice
rhdBIOSScratchUpdateBIOSScratchForOutput(struct rhdOutput *Output)
{
    RHDPtr rhdPtr = RHDPTRI(Output);
    struct rhdOutputDevices *devList;
    enum atomDevice Device;
    int i = 0;

    RHDFUNC(Output);

    if (!Output->OutputDriverPrivate) {
	RHDDebug(Output->scrnIndex,"%s: no output driver private present\n",__func__);
	return atomNone;
    }
    devList = Output->OutputDriverPrivate->OutputDevices;

    if (Output->Connector) {
	/* connected - enable */
	Device = rhdBIOSScratchSetDeviceForOutput(Output);

    if (Device == atomNone && rhdPtr->Card->ConnectorInfo[0].Type != RHD_CONNECTOR_NONE) {
        xf86DrvMsg(Output->scrnIndex, X_WARNING, "%s: AtomBIOS DeviceID unknown\n",__func__);
        return Device;
    }

	ASSERT(Device != atomNone);

	if (Output->Crtc)
	    rhdAtomBIOSScratchSetCrtcState(rhdPtr, Device,
					   Output->Crtc->Id == 1 ? atomCrtc2 : atomCrtc1);
	rhdAtomBIOSScratchUpdateOnState(rhdPtr, Device, Output->Active);
	rhdAtomBIOSScratchSetAcceleratorModeForDevice(rhdPtr, Device, Output->Active);
	rhdAtomBIOSScratchUpdateAttachedState(rhdPtr, Device, TRUE);

	while (devList[i].DeviceId != atomNone) {
	    if (devList[i].DeviceId != Device)
		rhdAtomBIOSScratchUpdateOnState(rhdPtr, devList[i].DeviceId, FALSE);
	        i++;
	}

    } else {
	/* not connected - just disable everything */
	Device = atomNone;
	Output->OutputDriverPrivate->Device = Device;

	while (devList[i].DeviceId != atomNone) {
	    rhdAtomBIOSScratchUpdateOnState(rhdPtr, devList[i].DeviceId, FALSE);
	    rhdAtomBIOSScratchSetAcceleratorModeForDevice(rhdPtr,
							  devList[i].DeviceId, FALSE);
	    rhdAtomBIOSScratchUpdateAttachedState(rhdPtr, devList[i].DeviceId, FALSE);
	    i++;
	}
    }

    return Device;
}

/*
 *
 */
static void
rhdBIOSScratchPower(struct rhdOutput *Output, int Power)
{
    rhdBIOSScratchUpdateBIOSScratchForOutput(Output);
    Output->OutputDriverPrivate->Power(Output, Power);
}

/*
 *
 */
static void
rhdBIOSScratchMode(struct rhdOutput *Output, DisplayModePtr Mode)
{
    rhdBIOSScratchUpdateBIOSScratchForOutput(Output);
    Output->OutputDriverPrivate->Mode(Output, Mode);
}

/*
 * This destroys the privates again. It is implemented as an output destroy wrapper.
 */
static void
rhdBIOSScratchDestroyOutputDriverPrivate(struct rhdOutput *Output)
{
    RHDFUNC(Output);

    if (Output->OutputDriverPrivate) {
	void (*Destroy) (struct rhdOutput *Output) = Output->OutputDriverPrivate->Destroy;

	xfree(Output->OutputDriverPrivate->OutputDevices);
	xfree(Output->OutputDriverPrivate);
	Output->OutputDriverPrivate = NULL;
	if (Destroy)
	    Destroy(Output);
    }
}

/*
 * This sets up the AtomBIOS driver output private.
 * It allocates the data structure and sets up the list of devices
 * including the connector they are associated with.
 */
Bool
RHDAtomSetupOutputDriverPrivate(struct rhdAtomOutputDeviceList *Devices, struct rhdOutput *Output)
{
    struct rhdOutputDevices *od = NULL;
    struct BIOSScratchOutputPrivate *OutputDriverPrivate;
    int i = 0, cnt = 0;

    RHDFUNC(Output);

    if (!Devices) {
	RHDDebug(Output->scrnIndex, "%s: Device list doesn't exist.\n");
	return FALSE;
    }

    RHDDebugVerb(Output->scrnIndex, 1, " Output: %s[0x%2.2x] - adding devices:\n", Output->Name, Output->Id);

    while (Devices[i].DeviceId != atomNone) {
	RHDDebugVerb(Output->scrnIndex,1," Looking at DeviceID: 0x%2.2x OutputType: 0x%2.2x ConnectorType: 0x%2.2x\n",
		     Devices[i].DeviceId,Devices[i].OutputType,Devices[i].ConnectorType);
	if (Devices[i].OutputType == Output->Id) {
	    if (!(od = (struct rhdOutputDevices *)xrealloc(od, sizeof(struct rhdOutputDevices) * (cnt + 1))))
		return FALSE;
	    RHDDebugVerb(Output->scrnIndex,1,"  >> 0x%2.2x\n", Devices[i].DeviceId);
	    od[cnt].DeviceId = Devices[i].DeviceId;
	    od[cnt].ConnectorType = Devices[i].ConnectorType;
	    cnt++;
	}
	i++;
    }
    if (!(od = (struct rhdOutputDevices *)xrealloc(od, sizeof(struct rhdOutputDevices) * (cnt + 1))))
	return FALSE;
    od[cnt].DeviceId = atomNone;

    if (!(OutputDriverPrivate = (struct BIOSScratchOutputPrivate *)xalloc(sizeof(struct BIOSScratchOutputPrivate)))) {
	xfree(od);
	return FALSE;
    }
    OutputDriverPrivate->OutputDevices = od;
    OutputDriverPrivate->Destroy = Output->Destroy;
    Output->Destroy = rhdBIOSScratchDestroyOutputDriverPrivate;
    OutputDriverPrivate->Power = Output->Power;
    Output->Power = rhdBIOSScratchPower;
    OutputDriverPrivate->Mode = Output->Mode;
    Output->Mode = rhdBIOSScratchMode;
    Output->OutputDriverPrivate = OutputDriverPrivate;

    return TRUE;
}

/*
 * Find the connector and output type for a specific atom device.
 * This information is kept in the output lists.
 */
Bool
RHDFindConnectorAndOutputTypesForDevice(RHDPtr rhdPtr, enum atomDevice Device, enum rhdOutputType *ot, enum rhdConnectorType *ct)
{
    struct rhdOutput *Output;

    *ot = RHD_OUTPUT_NONE;
    *ct = RHD_CONNECTOR_NONE;

    for (Output = rhdPtr->Outputs; Output; Output = Output->Next) {
	struct rhdOutputDevices *DeviceList;
	int i = 0;

	if (!Output->OutputDriverPrivate)
	    continue;

	DeviceList = Output->OutputDriverPrivate->OutputDevices;
	while (DeviceList[i].DeviceId != atomNone) {
	    if (DeviceList[i].DeviceId == Device) {
		*ot = Output->Id;
		*ct = DeviceList[i].ConnectorType;
		return TRUE;
	    }
	    i++;
	}
    }

    return FALSE;
}

/*
 *
 */
enum atomDevice
RHDGetDeviceOnCrtc(RHDPtr rhdPtr, enum atomCrtc Crtc)
{
    CARD32 BIOS_3;
    CARD32 Addr;
    CARD32 Mask = 0;

    RHDFUNC(rhdPtr);

    if (rhdPtr->ChipSet < RHD_R600)
	Addr = 0x1C;
    else
	Addr = 0x1730;

    if (Crtc == atomCrtc1)
	Mask = ~Mask;

    BIOS_3 = RHDRegRead(rhdPtr, Addr);
    RHDDebug(rhdPtr->scrnIndex, "%s: BIOS_3 = 0x%x\n",__func__,BIOS_3);

    if (BIOS_3 & ATOM_S3_CRT1_ACTIVE
	&& ((BIOS_3 ^ Mask) & ATOM_S3_CRT1_CRTC_ACTIVE))
	return atomCRT1;
    else if (BIOS_3 & ATOM_S3_LCD1_ACTIVE
	     && ((BIOS_3 ^ Mask) & ATOM_S3_LCD1_CRTC_ACTIVE))
	return atomLCD1;
    else if (BIOS_3 & ATOM_S3_DFP1_ACTIVE
	     && ((BIOS_3 ^ Mask) & ATOM_S3_DFP1_CRTC_ACTIVE))
	return atomDFP1;
    else if (BIOS_3 & ATOM_S3_CRT2_ACTIVE
	     && ((BIOS_3 ^ Mask) & ATOM_S3_CRT2_CRTC_ACTIVE))
	return atomCRT2;
    else if (BIOS_3 & ATOM_S3_LCD2_ACTIVE
	     && ((BIOS_3 ^ Mask) & ATOM_S3_LCD2_CRTC_ACTIVE))
	return atomLCD2;
    else if (BIOS_3 & ATOM_S3_TV2_ACTIVE
	     && ((BIOS_3 ^ Mask) & ATOM_S3_TV2_CRTC_ACTIVE))
	return atomTV2;
    else if (BIOS_3 & ATOM_S3_DFP2_ACTIVE
	     && ((BIOS_3 ^ Mask) & ATOM_S3_DFP2_CRTC_ACTIVE))
	return atomDFP2;
    else if (BIOS_3 & ATOM_S3_CV_ACTIVE
	     && ((BIOS_3 ^ Mask) & ATOM_S3_CV_CRTC_ACTIVE))
	return atomCV;
    else if (BIOS_3 & ATOM_S3_DFP3_ACTIVE
	     && ((BIOS_3 ^ Mask) & ATOM_S3_DFP3_CRTC_ACTIVE))
	return atomDFP3;
    else if (BIOS_3 & ATOM_S3_DFP4_ACTIVE
	     && ((BIOS_3 ^ Mask) & ATOM_S3_DFP4_CRTC_ACTIVE))
	return atomDFP4;
    else if (BIOS_3 & ATOM_S3_DFP5_ACTIVE
	     && ((BIOS_3 ^ Mask) & ATOM_S3_DFP5_CRTC_ACTIVE))
	return atomDFP5;
    else
	return atomNone;
}

struct rhdBiosScratchRegisters {
    CARD32 Scratch0;
    CARD32 Scratch2;
    CARD32 Scratch3;
    CARD32 Scratch6;
};

struct rhdBiosScratchRegisters *
RHDSaveBiosScratchRegisters(RHDPtr rhdPtr)
{
    struct rhdBiosScratchRegisters *regs;
    CARD32 S0Addr, S2Addr, S3Addr, S6Addr;

    RHDFUNC(rhdPtr);

    if (!(regs = (struct rhdBiosScratchRegisters *)xalloc(sizeof(struct rhdBiosScratchRegisters))))
	return NULL;

    if (rhdPtr->ChipSet < RHD_R600) {
	S0Addr = 0x10;
	S2Addr = 0x18;
	S3Addr = 0x1C;
	S6Addr = 0x10 + (6 << 2);
    } else {
	S0Addr = 0x1724;
	S2Addr = 0x172C;
	S3Addr = 0x1730;
	S6Addr = 0x1724 + (6 << 2);
    }
    regs->Scratch0 = RHDRegRead(rhdPtr, S0Addr);
    regs->Scratch2 = RHDRegRead(rhdPtr, S2Addr);
    regs->Scratch3 = RHDRegRead(rhdPtr, S3Addr);
    regs->Scratch6 = RHDRegRead(rhdPtr, S6Addr);

    return regs;
}

void
RHDRestoreBiosScratchRegisters(RHDPtr rhdPtr, struct rhdBiosScratchRegisters *regs)
{
    CARD32 S0Addr, S2Addr, S3Addr, S6Addr;

    RHDFUNC(rhdPtr);

    if (!regs)
	return;

    if (rhdPtr->ChipSet < RHD_R600) {
	S0Addr = 0x10;
	S2Addr = 0x18;
	S3Addr = 0x1C;
	S6Addr = 0x10 + (6 << 2);
    } else {
	S0Addr = 0x1724;
	S2Addr = 0x172C;
	S3Addr = 0x1730;
	S6Addr = 0x1724 + (6 << 2);
    }
    RHDRegWrite(rhdPtr, S0Addr, regs->Scratch0);
    RHDRegWrite(rhdPtr, S2Addr, regs->Scratch2);
    RHDRegWrite(rhdPtr, S3Addr, regs->Scratch3);
    RHDRegWrite(rhdPtr, S6Addr, regs->Scratch6);

    xfree(regs);
}

#endif /* ATOM_BIOS */

