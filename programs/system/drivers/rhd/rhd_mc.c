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

#if HAVE_XF86_ANSIC_H
# include "xf86_ansic.h"
#else
# include <unistd.h>
#endif

#include "xf86.h"

#include "rhd.h"
#include "rhd_regs.h"

Bool RHDMCIdle(RHDPtr rhdPtr, CARD32 count);

Bool RHDMCIdle(RHDPtr rhdPtr, CARD32 count);

struct rhdMC {
    CARD32 FbLocation;
    CARD32 HdpFbBase;
    CARD32 MiscLatencyTimer;
    Bool Stored;
    void (*SaveMC)(RHDPtr rhdPtr);
    void (*RestoreMC)(RHDPtr rhdPtr);
    void (*SetupMC)(RHDPtr rhdPtr);
    Bool (*MCIdle)(RHDPtr rhdPtr);
    CARD32 (*GetFBLocation)(RHDPtr rhdPtr, CARD32 *size);
    void (*TuneMCAccessForDisplay)(RHDPtr rhdPtr, int crtc,
				   DisplayModePtr Mode, DisplayModePtr ScaledToMode);
    Bool RV515Variant;
};

/*
 * Save MC_VM state.
 */
static void
rs600SaveMC(RHDPtr rhdPtr)
{
    struct rhdMC *MC = rhdPtr->MC;

    RHDFUNC(rhdPtr);

    MC->FbLocation = RHDReadMC(rhdPtr, RS60_NB_FB_LOCATION);
    MC->HdpFbBase = RHDRegRead(rhdPtr, HDP_FB_LOCATION);
}

/*
 *
 */
static void
rs690SaveMC(RHDPtr rhdPtr)
{
    struct rhdMC *MC = rhdPtr->MC;

    RHDFUNC(rhdPtr);

    MC->FbLocation = RHDReadMC(rhdPtr, RS69_MCCFG_FB_LOCATION);
    MC->HdpFbBase = RHDRegRead(rhdPtr, HDP_FB_LOCATION);
    MC->MiscLatencyTimer = RHDReadMC(rhdPtr, RS69_MC_INIT_MISC_LAT_TIMER);
}

/*
 *
 */
static void
r6xxSaveMC(RHDPtr rhdPtr)
{
    struct rhdMC *MC = rhdPtr->MC;

    RHDFUNC(rhdPtr);

    MC->FbLocation = RHDRegRead(rhdPtr, R6XX_MC_VM_FB_LOCATION);
    MC->HdpFbBase = RHDRegRead(rhdPtr, R6XX_HDP_NONSURFACE_BASE);
}

/*
 *
 */
#ifdef NOTYET
static void
rs780SaveMC(RHDPtr rhdPtr)
{
    struct rhdMC *MC = rhdPtr->MC;

    RHDFUNC(rhdPtr);

    MC->FbLocation = RHDReadMC(rhdPtr, RS78_MC_FB_LOCATION);
    /* RS780 uses the same register as R6xx */
    MC->HdpFbBase = RHDRegRead(rhdPtr, R6XX_HDP_NONSURFACE_BASE);
}
#endif

/*
 *
 */
static void
r7xxSaveMC(RHDPtr rhdPtr)
{
    struct rhdMC *MC = rhdPtr->MC;

    RHDFUNC(rhdPtr);

    MC->FbLocation = RHDRegRead(rhdPtr, R7XX_MC_VM_FB_LOCATION);
    MC->HdpFbBase = RHDRegRead(rhdPtr, R6XX_HDP_NONSURFACE_BASE);
}

/*
 *
 */
static void
r5xxRestoreMC(RHDPtr rhdPtr)
{
    struct rhdMC *MC = rhdPtr->MC;

    RHDFUNC(rhdPtr);

    if (MC->RV515Variant) {
	RHDWriteMC(rhdPtr, MC_IND_ALL |  RV515_MC_FB_LOCATION,
		   MC->FbLocation);
	RHDWriteMC(rhdPtr, MC_IND_ALL |  RV515_MC_MISC_LAT_TIMER,
		   MC->MiscLatencyTimer);
    } else
	RHDWriteMC(rhdPtr, MC_IND_ALL | R5XX_MC_FB_LOCATION,
		   MC->FbLocation);
    RHDRegWrite(rhdPtr, HDP_FB_LOCATION, MC->HdpFbBase);
}

/*
 *
 */
static void
rs600RestoreMC(RHDPtr rhdPtr)
{
    struct rhdMC *MC = rhdPtr->MC;

    RHDFUNC(rhdPtr);

    RHDWriteMC(rhdPtr, RS60_NB_FB_LOCATION, MC->FbLocation);
    RHDRegWrite(rhdPtr, HDP_FB_LOCATION, MC->HdpFbBase);
}

/*
 *
 */
static void
rs690RestoreMC(RHDPtr rhdPtr)
{
    struct rhdMC *MC = rhdPtr->MC;

    RHDFUNC(rhdPtr);

    RHDWriteMC(rhdPtr,  RS69_MCCFG_FB_LOCATION, MC->FbLocation);
    RHDRegWrite(rhdPtr, HDP_FB_LOCATION, MC->HdpFbBase);
    RHDWriteMC(rhdPtr,  RS69_MC_INIT_MISC_LAT_TIMER, MC->MiscLatencyTimer);
}

/*
 *
 */
static void
r6xxRestoreMC(RHDPtr rhdPtr)
{
    struct rhdMC *MC = rhdPtr->MC;

    RHDFUNC(rhdPtr);

    RHDRegWrite(rhdPtr, R6XX_MC_VM_FB_LOCATION, MC->FbLocation);
    RHDRegWrite(rhdPtr, R6XX_HDP_NONSURFACE_BASE, MC->HdpFbBase);
}

/*
 *
 */
#ifdef NOTYET
static void
rs780RestoreMC(RHDPtr rhdPtr)
{
    struct rhdMC *MC = rhdPtr->MC;

    RHDFUNC(rhdPtr);

    RHDWriteMC(rhdPtr, RS78_MC_FB_LOCATION, MC->FbLocation);
    /* RS780 uses the same register as R6xx */
    RHDRegWrite(rhdPtr, R6XX_HDP_NONSURFACE_BASE, MC->HdpFbBase);
}
#endif

/*
 *
 */
static void
r7xxRestoreMC(RHDPtr rhdPtr)
{
    struct rhdMC *MC = rhdPtr->MC;

    RHDFUNC(rhdPtr);

    RHDRegWrite(rhdPtr, R7XX_MC_VM_FB_LOCATION, MC->FbLocation);
    RHDRegWrite(rhdPtr, R6XX_HDP_NONSURFACE_BASE, MC->HdpFbBase);
}

/*
 * Setup the MC
 */

/*
 *
 */
static void
r5xxSetupMC(RHDPtr rhdPtr)
{
    struct rhdMC *MC = rhdPtr->MC;
    CARD32 fb_location, fb_location_tmp;
    CARD16 fb_size;
    unsigned int reg;

    RHDFUNC(rhdPtr);


    if (MC->RV515Variant)
	reg = RV515_MC_FB_LOCATION | MC_IND_ALL;
    else
	reg = R5XX_MC_FB_LOCATION | MC_IND_ALL;

    fb_location = RHDReadMC(rhdPtr, reg);
    fb_size = (fb_location >> 16) - (fb_location & 0xFFFF);
    fb_location_tmp = rhdPtr->FbIntAddress >> 16;
    fb_location_tmp |= (fb_location_tmp + fb_size) << 16;

    RHDDebug(rhdPtr->scrnIndex, "%s: fb_location: 0x%08X "
	     "[fb_size: 0x%04X] -> fb_location: 0x%08X\n",
	     __func__, (unsigned int)fb_location,
	     fb_size,(unsigned int)fb_location_tmp);
    RHDWriteMC(rhdPtr, reg, fb_location_tmp);
    RHDRegWrite(rhdPtr, HDP_FB_LOCATION, fb_location_tmp & 0xFFFF);
}

/*
 *
 */
static void
rs600SetupMC(RHDPtr rhdPtr)
{
    CARD32 fb_location, fb_location_tmp;
    CARD16 fb_size;

    RHDFUNC(rhdPtr);

    fb_location = RHDReadMC(rhdPtr, RS60_NB_FB_LOCATION);
    fb_size = (fb_location >> 16) - (fb_location & 0xFFFF);
    fb_location_tmp = rhdPtr->FbIntAddress >> 16;
    fb_location_tmp |= (fb_location_tmp + fb_size) << 16;

    RHDDebug(rhdPtr->scrnIndex, "%s: fb_location: 0x%08X "
	     "[fb_size: 0x%04X] -> fb_location: 0x%08X\n",
	     __func__, (unsigned int)fb_location,
	     fb_size,(unsigned int)fb_location_tmp);
    RHDWriteMC(rhdPtr, RS60_NB_FB_LOCATION, fb_location_tmp);
    RHDRegWrite(rhdPtr, HDP_FB_LOCATION, fb_location_tmp & 0xFFFF); /* same ;) */
}

/*
 *
 */
static void
rs690SetupMC(RHDPtr rhdPtr)
{
    CARD32 fb_location, fb_location_tmp;
    CARD16 fb_size;

    RHDFUNC(rhdPtr);

    fb_location = RHDReadMC(rhdPtr, RS69_MCCFG_FB_LOCATION);
    fb_size = (fb_location >> 16) - (fb_location & 0xFFFF);
    fb_location_tmp = rhdPtr->FbIntAddress >> 16;
    fb_location_tmp |= (fb_location_tmp + fb_size) << 16;

    RHDDebug(rhdPtr->scrnIndex, "%s: fb_location: 0x%08X "
	     "[fb_size: 0x%04X] -> fb_location: 0x%08X\n",
	     __func__, (unsigned int)fb_location,
	     fb_size,(unsigned int)fb_location_tmp);
    RHDWriteMC(rhdPtr, RS69_MCCFG_FB_LOCATION, fb_location_tmp);
    RHDRegWrite(rhdPtr, HDP_FB_LOCATION, fb_location_tmp & 0xFFFF);
}

/*
 *
 */
static void
r6xxSetupMC(RHDPtr rhdPtr)
{
    CARD32 fb_location, fb_location_tmp, hdp_fbbase_tmp;
    CARD16 fb_size;

    RHDFUNC(rhdPtr);

    fb_location = RHDRegRead(rhdPtr, R6XX_MC_VM_FB_LOCATION);
    fb_size = (fb_location >> 16) - (fb_location & 0xFFFF);
    fb_location_tmp = rhdPtr->FbIntAddress >> 24;
    fb_location_tmp |= (fb_location_tmp + fb_size) << 16;
    hdp_fbbase_tmp = (rhdPtr->FbIntAddress >> 8) & 0xff0000;

    RHDDebug(rhdPtr->scrnIndex, "%s: fb_location: 0x%08X "
	     "fb_offset: 0x%08X [fb_size: 0x%04X] -> fb_location: 0x%08X "
	     "fb_offset: 0x%08X\n",
	     __func__, (unsigned int)fb_location,
	     RHDRegRead(rhdPtr,R6XX_HDP_NONSURFACE_BASE), fb_size,
	     (unsigned int)fb_location_tmp, (unsigned int)hdp_fbbase_tmp);

    RHDRegWrite(rhdPtr, R6XX_MC_VM_FB_LOCATION, fb_location_tmp);
    RHDRegWrite(rhdPtr, R6XX_HDP_NONSURFACE_BASE, hdp_fbbase_tmp);
}

/*
 *
 */
#ifdef NOTYET
static void
rs780SetupMC(RHDPtr rhdPtr)
{
    CARD32 fb_location, fb_location_tmp, hdp_fbbase_tmp;
    CARD16 fb_size;

    RHDFUNC(rhdPtr);

    fb_location = RHDReadMC(rhdPtr, RS78_MC_FB_LOCATION);
    fb_size = (fb_location >> 16) - (fb_location & 0xFFFF);
    fb_location_tmp = rhdPtr->FbIntAddress >> 16;
    fb_location_tmp |= (fb_location_tmp + fb_size) << 16;
    hdp_fbbase_tmp = (rhdPtr->FbIntAddress >> 8) & 0xff0000;

    RHDDebug(rhdPtr->scrnIndex, "%s: fb_location: 0x%08X "
	     "[fb_size: 0x%04X] -> fb_location: 0x%08X\n",
	     __func__, (unsigned int)fb_location,
	     fb_size,(unsigned int)fb_location_tmp);
    RHDWriteMC(rhdPtr, RS78_MC_FB_LOCATION, fb_location_tmp);
    /* RS780 uses the same register as R6xx */
    RHDRegWrite(rhdPtr, R6XX_HDP_NONSURFACE_BASE, hdp_fbbase_tmp);
}
#endif

/*
 *
 */
static void
r7xxSetupMC(RHDPtr rhdPtr)
{
    CARD32 fb_location, fb_location_tmp, hdp_fbbase_tmp;
    CARD16 fb_size;

    RHDFUNC(rhdPtr);

    fb_location = RHDRegRead(rhdPtr, R7XX_MC_VM_FB_LOCATION);
    fb_size = (fb_location >> 16) - (fb_location & 0xFFFF);
    fb_location_tmp = rhdPtr->FbIntAddress >> 24;
    fb_location_tmp |= (fb_location_tmp + fb_size) << 16;
    hdp_fbbase_tmp = (rhdPtr->FbIntAddress >> 8) & 0xff0000;

    RHDDebug(rhdPtr->scrnIndex, "%s: fb_location: 0x%08X "
	     "fb_offset: 0x%08X [fb_size: 0x%04X] -> fb_location: 0x%08X "
	     "fb_offset: 0x%08X\n",
	     __func__, (unsigned int)fb_location,
	     RHDRegRead(rhdPtr,R6XX_HDP_NONSURFACE_BASE), fb_size,
	     (unsigned int)fb_location_tmp, (unsigned int)hdp_fbbase_tmp);

    RHDRegWrite(rhdPtr, R7XX_MC_VM_FB_LOCATION, fb_location_tmp);
    RHDRegWrite(rhdPtr, R6XX_HDP_NONSURFACE_BASE, hdp_fbbase_tmp);
}

/*
 *
 */
void
RHDMCSetup(RHDPtr rhdPtr)
{
    struct rhdMC *MC = rhdPtr->MC;
    RHDFUNC(rhdPtr);

    if (!MC)
	return;
    /*
     * make sure the hw is in a state such that we can update
     * the MC - ie no subsystem is currently accessing memory.
     */
    ASSERT((RHDRegRead(rhdPtr, D1VGA_CONTROL) & D1VGA_MODE_ENABLE) != D1VGA_MODE_ENABLE);
    ASSERT((RHDRegRead(rhdPtr, D2VGA_CONTROL) & D2VGA_MODE_ENABLE) != D2VGA_MODE_ENABLE);
    ASSERT((RHDRegRead(rhdPtr, D1CRTC_CONTROL) & 0x1) != 0x1);
    ASSERT((RHDRegRead(rhdPtr, D2CRTC_CONTROL) & 0x1) != 0x1);
    ASSERT(RHDMCIdle(rhdPtr, 1));

    MC->SetupMC(rhdPtr);
}

/*
 * Get FB location and size.
 */
static CARD32
r5xxGetFBLocation(RHDPtr rhdPtr, CARD32 *size)
{
    struct rhdMC *MC = rhdPtr->MC;
    CARD32 val;
    CARD32 reg;

    if (MC->RV515Variant)
	reg = RV515_MC_FB_LOCATION | MC_IND_ALL;
    else
	reg = R5XX_MC_FB_LOCATION | MC_IND_ALL;

	val = RHDReadMC(rhdPtr, reg);

    if (size) *size = ((val >> 16) - (val & 0xFFFF)) << 16;

    return (val & 0xFFFF) << 16;
}

/*
 *
 */
static CARD32
rs600GetFBLocation(RHDPtr rhdPtr, CARD32 *size)
{
    CARD32 val = RHDReadMC(rhdPtr, RS60_NB_FB_LOCATION);

    if (size) *size = ((val >> 16) - (val & 0xFFFF)) << 16;

    return (val & 0xFFFF) << 16;
}

/*
 *
 */
static CARD32
rs690GetFBLocation(RHDPtr rhdPtr, CARD32 *size)
{
    CARD32 val = RHDReadMC(rhdPtr, RS69_MCCFG_FB_LOCATION);

    if (size) *size = ((val >> 16) - (val & 0xFFFF)) << 16;

    return (val & 0xFFFF) << 16;
}

/*
 *
 */
static CARD32
r6xxGetFBLocation(RHDPtr rhdPtr, CARD32 *size)
{
    CARD32 val = RHDRegRead(rhdPtr, R6XX_MC_VM_FB_LOCATION);

    if (size) *size = ((val >> 16) - (val & 0xFFFF)) << 24;

    return (val & 0xFFFF) << 24;
}

/*
 *
 */
#ifdef NOTYET
static CARD32
rs780GetFBLocation(RHDPtr rhdPtr, CARD32 *size)
{
    CARD32 val = RHDReadMC(rhdPtr, RS78_MC_FB_LOCATION);
    if (size) *size = ((val >> 16) - (val & 0xFFFF)) << 16;

    return (val & 0xFFFF) << 16;
}
#endif

/*
 *
 */
static CARD32
r7xxGetFBLocation(RHDPtr rhdPtr, CARD32 *size)
{
    CARD32 val = RHDRegRead(rhdPtr, R7XX_MC_VM_FB_LOCATION);

    if (size) *size = ((val >> 16) - (val & 0xFFFF)) << 24;

    return (val & 0xFFFF) << 24;
}

/*
 *
 */
CARD32
RHDGetFBLocation(RHDPtr rhdPtr, CARD32 *size)
{
    struct rhdMC *MC = rhdPtr->MC;
    RHDFUNC(rhdPtr);

    if (!MC) {
	if (size) *size = 0;
	return 0;
    }

    return MC->GetFBLocation(rhdPtr, size);
}

/*
 *
 */
static Bool
rv515MCIdle(RHDPtr rhdPtr)
{
    RHDFUNC(rhdPtr);

    if (RHDReadMC(rhdPtr, MC_IND_ALL | RV515_MC_STATUS) & RV515_MC_IDLE)
	return TRUE;
    return FALSE;
}


/*
 *
 */
static Bool
r5xxMCIdle(RHDPtr rhdPtr)
{
    RHDFUNC(rhdPtr);

    if (RHDReadMC(rhdPtr, MC_IND_ALL | R5XX_MC_STATUS) & R5XX_MC_IDLE)
	return TRUE;
    return FALSE;
}

/*
 *
 */
static Bool
rs600MCIdle(RHDPtr rhdPtr)
{
    RHDFUNC(rhdPtr);

    if (RHDReadMC(rhdPtr, RS60_MC_SYSTEM_STATUS) & RS6X_MC_SEQUENCER_IDLE)
	return TRUE;
    return FALSE;
}

/*
 *
 */
static Bool
rs690MCIdle(RHDPtr rhdPtr)
{
    RHDFUNC(rhdPtr);

    if (RHDReadMC(rhdPtr, RS69_MC_SYSTEM_STATUS) & RS6X_MC_SEQUENCER_IDLE)
	return TRUE;
    return FALSE;
}

/*
 *
 */
static Bool
r6xxMCIdle(RHDPtr rhdPtr)
{
    RHDFUNC(rhdPtr);

    if (!(RHDRegRead(rhdPtr, SRBM_STATUS) & 0x3f00))
	return TRUE;
    return FALSE;
}

/*
 *
 */
#ifdef NOTYET
static Bool
rs780MCIdle(RHDPtr rhdPtr)
{
    RHDFUNC(rhdPtr);

    if (RHDReadMC(rhdPtr, RS78_MC_SYSTEM_STATUS) & RS78_MC_SEQUENCER_IDLE)
	return TRUE;
    return FALSE;
}
#endif

/*
 *
 */
Bool
RHDMCIdle(RHDPtr rhdPtr, CARD32 count)
{
    struct rhdMC *MC = rhdPtr->MC;
    RHDFUNC(rhdPtr);

    if (!MC)
	return TRUE;

    do {
	if (MC->MCIdle(rhdPtr))
	    return TRUE;
	usleep(10);
    } while (count--);

    RHDDebug(rhdPtr->scrnIndex, "%s: MC not idle\n",__func__);

    return FALSE;
}

/*
 *
 */
void
RHDSaveMC(RHDPtr rhdPtr)
{
    struct rhdMC *MC = rhdPtr->MC;
    RHDFUNC(rhdPtr);

    if (!MC)
	return;

    MC->SaveMC(rhdPtr);

    MC->Stored = TRUE;
}

/*
 * Restore MC VM state.
 */
void
RHDRestoreMC(RHDPtr rhdPtr)
{
    struct rhdMC *MC = rhdPtr->MC;
    RHDFUNC(rhdPtr);

    if (!MC)
	return;

    if (!MC->Stored) {
	xf86DrvMsg(rhdPtr->scrnIndex, X_ERROR,
		   "%s: trying to restore uninitialized values.\n",__func__);
	return;
    }
    /*
     * make sure the hw is in a state such that we can update
     * the MC - ie no subsystem is currently accessing memory.
     */
    ASSERT((RHDRegRead(rhdPtr, D1VGA_CONTROL) & D1VGA_MODE_ENABLE) != D1VGA_MODE_ENABLE);
    ASSERT((RHDRegRead(rhdPtr, D2VGA_CONTROL) & D2VGA_MODE_ENABLE) != D2VGA_MODE_ENABLE);
    ASSERT((RHDRegRead(rhdPtr, D1CRTC_CONTROL) & 0x1) != 0x1);
    ASSERT((RHDRegRead(rhdPtr, D2CRTC_CONTROL) & 0x1) != 0x1);
    ASSERT(RHDMCIdle(rhdPtr, 1));

    MC->RestoreMC(rhdPtr);
}

/*
 *
 */
static void
r5xxSaveMC(RHDPtr rhdPtr)
{
    struct rhdMC *MC = rhdPtr->MC;

    RHDFUNC(rhdPtr);

    if (MC->RV515Variant) {
	MC->FbLocation = RHDReadMC(rhdPtr, MC_IND_ALL | RV515_MC_FB_LOCATION);
	MC->MiscLatencyTimer = RHDReadMC(rhdPtr, MC_IND_ALL | RV515_MC_MISC_LAT_TIMER);
    } else
	MC->FbLocation = RHDReadMC(rhdPtr, MC_IND_ALL | R5XX_MC_FB_LOCATION);
    MC->HdpFbBase = RHDRegRead(rhdPtr, HDP_FB_LOCATION);
}

/*
 *
 */
static void
rv515TuneMCAccessForDisplay(RHDPtr rhdPtr, int crtc,
				   DisplayModePtr Mode, DisplayModePtr ScaledToMode)
{
    CARD32 value, setting = 0x1;

    RHDFUNC(rhdPtr);

    value = RHDReadMC(rhdPtr,  RV515_MC_MISC_LAT_TIMER);

    value |= (setting << (crtc ? MC_DISP1R_INIT_LAT_SHIFT : MC_DISP0R_INIT_LAT_SHIFT));
    RHDWriteMC(rhdPtr,  RV515_MC_MISC_LAT_TIMER, value);
}

/*
 *
 */
static void
rs690TuneMCAccessForDisplay(RHDPtr rhdPtr, int crtc,
				   DisplayModePtr Mode, DisplayModePtr ScaledToMode)
{
    CARD32 value, setting = 0x1;

    RHDFUNC(rhdPtr);

    value = RHDReadMC(rhdPtr,  RS69_MC_INIT_MISC_LAT_TIMER);
    value |= setting << (crtc ? MC_DISP1R_INIT_LAT_SHIFT : MC_DISP0R_INIT_LAT_SHIFT);
    RHDWriteMC(rhdPtr,  RS69_MC_INIT_MISC_LAT_TIMER, value);
}

/*
 *
 */
void
RHDTuneMCAccessForDisplay(RHDPtr rhdPtr, int crtc,
				   DisplayModePtr Mode, DisplayModePtr ScaledToMode)
{
    struct rhdMC *MC = rhdPtr->MC;

    RHDFUNC(rhdPtr);

    if (MC->TuneMCAccessForDisplay)
	MC->TuneMCAccessForDisplay(rhdPtr, crtc, Mode, ScaledToMode);
}

/*
 *
 */
void
RHDMCInit(RHDPtr rhdPtr)
{
    struct rhdMC *MC;

    RHDFUNC(rhdPtr);

    /* These devices have an internal address reference, which some other
     * address registers in there also use. This can be different from the
     * address in the BAR.
     *
     * We read out the address here from some known location. This address
     * is as good a guess as any, we just need to pick one, but then make
     * sure that it is made consistent in MCSetup and the various MC
     * accessing subsystems.
     */
    if (rhdPtr->ChipSet < RHD_R600)
	rhdPtr->FbIntAddress = RHDRegRead(rhdPtr, HDP_FB_LOCATION) << 16;
    else
	rhdPtr->FbIntAddress = RHDRegRead(rhdPtr, R6XX_CONFIG_FB_BASE);

    RHDDebug(rhdPtr->scrnIndex, "MC FB Address: 0x%08X.\n",
	     rhdPtr->FbIntAddress);

    MC = xnfcalloc(1, sizeof(struct rhdMC));
    MC->Stored = FALSE;

    if (rhdPtr->ChipSet < RHD_RS600) {
	MC->SaveMC = r5xxSaveMC;
	MC->RestoreMC = r5xxRestoreMC;
	MC->SetupMC = r5xxSetupMC;
	MC->GetFBLocation = r5xxGetFBLocation;

	if (rhdPtr->ChipSet == RHD_RV515
	    || rhdPtr->ChipSet == RHD_RV505
	    || rhdPtr->ChipSet == RHD_RV516
	    || rhdPtr->ChipSet == RHD_RV550
	    || rhdPtr->ChipSet == RHD_M52
	    || rhdPtr->ChipSet == RHD_M54
	    || rhdPtr->ChipSet == RHD_M62
	    || rhdPtr->ChipSet == RHD_M64
	    || rhdPtr->ChipSet == RHD_M71) {

	    MC->RV515Variant = TRUE;
	    MC->MCIdle = rv515MCIdle;
	    MC->TuneMCAccessForDisplay = rv515TuneMCAccessForDisplay;
	} else {

	    MC->RV515Variant = FALSE;
	    MC->MCIdle = r5xxMCIdle;

	}

    } else if (rhdPtr->ChipSet == RHD_RS600) {
	MC->SaveMC = rs600SaveMC;
	MC->RestoreMC = rs600RestoreMC;
	MC->SetupMC = rs600SetupMC;
	MC->MCIdle = rs600MCIdle;
	MC->GetFBLocation = rs600GetFBLocation;
    } else if (rhdPtr->ChipSet < RHD_R600) {
	MC->SaveMC = rs690SaveMC;
	MC->RestoreMC = rs690RestoreMC;
	MC->SetupMC = rs690SetupMC;
	MC->MCIdle = rs690MCIdle;
	MC->GetFBLocation = rs690GetFBLocation;
	MC->TuneMCAccessForDisplay = rs690TuneMCAccessForDisplay;
    } else if (rhdPtr->ChipSet <= RHD_RS780) {
	MC->SaveMC = r6xxSaveMC;
	MC->RestoreMC = r6xxRestoreMC;
	MC->SetupMC = r6xxSetupMC;
	MC->MCIdle = r6xxMCIdle;
	MC->GetFBLocation = r6xxGetFBLocation;
    }
#if 0
    else if (rhdPtr->ChipSet == RHD_RS780) {
	MC->SaveMC = rs780SaveMC;
	MC->RestoreMC = rs780RestoreMC;
	MC->SetupMC = rs780SetupMC;
	MC->MCIdle = rs780MCIdle;
	MC->GetFBLocation = rs780GetFBLocation;
    }
#endif
    else if (rhdPtr->ChipSet >= RHD_RV770) {
	MC->SaveMC = r7xxSaveMC;
	MC->RestoreMC = r7xxRestoreMC;
	MC->SetupMC = r7xxSetupMC;
	MC->MCIdle = r6xxMCIdle;
	MC->GetFBLocation = r7xxGetFBLocation;
    } else {
	xf86DrvMsg(rhdPtr->scrnIndex, X_ERROR, "I don't know anything about MC on this chipset\n");
	xfree(MC);
	return;
    }
    rhdPtr->MC = MC;

}

/*
 * Free structure.
 */
void
RHDMCDestroy(RHDPtr rhdPtr)
{
    RHDFUNC(rhdPtr);

    if (!rhdPtr->MC)
	return;

    xfree(rhdPtr->MC);
    rhdPtr->MC = NULL;
}

/*
 *
 */
Bool
RHD_MC_IGP_SideportMemoryPresent(RHDPtr rhdPtr)
{
    Bool Present = FALSE;

    RHDFUNC(rhdPtr);

    switch (rhdPtr->ChipSet) {
	case RHD_RS690:
	case RHD_RS740:
	    Present = (RHDReadMC(rhdPtr, RS69_MC_MISC_UMA_CNTL) & RS69_SIDE_PORT_PRESENT_R) != 0;
	    break;
	case RHD_RS780:
	    Present = (RHDReadMC(rhdPtr, RS78_MC_MISC_UMA_CNTL) & RS78_SIDE_PORT_PRESENT_R) != 0;
	    break;
	default:
	    break;
    }
    xf86DrvMsg(rhdPtr->scrnIndex, X_INFO, "IPG sideport memory %s present.\n", Present ? "" : "not");

    return Present;
}
