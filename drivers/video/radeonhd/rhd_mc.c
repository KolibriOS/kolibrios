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

/*
 * MC idling:
 *
 * For SetupFBLocation and Restore, we require a fully idle MC as we might lock up
 * otherwise. Both calls now check whether the MC is Idle before attempting
 * to set up the MC, and complain loudly when this fails.
 *
 * Likely suspect registers for when the Idle fails:
 *   DxVGA_CONTROL & D1VGA_MODE_ENABLE (run RHDVGADisable beforehand)
 *   DxCRTC_CONTROL & 0x1 (run DxCRTCDisable beforehand)
 *   (... Add more here...)
 *
 *
 * MC addressing:
 *
 * On R600 and up the MC can use a larger than 32bit card internal address for
 * its framebuffer. This is why the Address used inside the MC code is a
 * CARD64.
 *
 * rhdPtr->FbIntAddress is kept as a CARD32 for the time being. This is still
 * valid, as this makes the R500 code simpler, and since we pick FbIntAddress
 * from a 32bit register anyway on R600. FbIntAddress will also correctly cast
 * to a CARD64 when passed to the likes of the SetupFBLocation callback.
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
#include "rhd_crtc.h" /* for definition of Crtc->Id */

struct rhdMC {
    int scrnIndex;

    CARD32 FbLocation;
    CARD32 HdpFbAddress;
    CARD32 MiscLatencyTimer;
    Bool Stored;
    void (*Save)(struct rhdMC *MC);
    void (*Restore)(struct rhdMC *MC);
    Bool (*Idle)(struct rhdMC *MC);
    CARD64 (*GetFBLocation)(struct rhdMC *MC, CARD32 *size);
    void (*SetupFBLocation)(struct rhdMC *MC, CARD64 Address, CARD32 Size);
    void (*TuneAccessForDisplay)(struct rhdMC *MC, int crtc,
				   DisplayModePtr Mode, DisplayModePtr ScaledToMode);
};

/*
 * Some common FB location calculations.
 */
/*
 * Applicable for all R5xx and RS600, RS690, RS740
 */
static CARD64
R5xxMCGetFBLocation(CARD32 Value, CARD32 *Size)
{
    *Size = (Value & 0xFFFF0000) - ((Value & 0xFFFF) << 16);
    return  (Value & 0xFFFF) << 16;
}

#define R5XX_FB_LOCATION(address, size) \
    ((((address) + (size)) & 0xFFFF0000) | (((address) >> 16) & 0xFFFF))
#define R5XX_HDP_LOCATION(address) \
    (((address) >> 16) & 0xFFFF)

/*
 * Applicable for all R6xx and R7xx, and RS780/RS790
 */
static CARD64
R6xxMCGetFBLocation(CARD32 Value, CARD32 *Size)
{
    *Size = (((Value & 0xFFFF0000) - ((Value & 0xFFFF) << 16))) << 8;
    return (Value & 0xFFFF) << 24;
}

#define R6XX_FB_LOCATION(address, size) \
    (((((address) + (size)) >> 8) & 0xFFFF0000) | (((address) >> 24) & 0xFFFF))
#define R6XX_HDP_LOCATION(address) \
    ((((address) >> 8) & 0x00FF0000))

/*
 *
 */
static void
RV515MCSave(struct rhdMC *MC)
{
    MC->FbLocation = RHDReadMC(MC, MC_IND_ALL | RV515_MC_FB_LOCATION);
    MC->MiscLatencyTimer = RHDReadMC(MC, MC_IND_ALL | RV515_MC_MISC_LAT_TIMER);
    MC->HdpFbAddress = RHDRegRead(MC, HDP_FB_LOCATION);
}

/*
 *
 */
static void
RV515MCRestore(struct rhdMC *MC)
{
    RHDWriteMC(MC, MC_IND_ALL | RV515_MC_FB_LOCATION, MC->FbLocation);
    RHDWriteMC(MC, MC_IND_ALL | RV515_MC_MISC_LAT_TIMER, MC->MiscLatencyTimer);
    RHDRegWrite(MC, HDP_FB_LOCATION, MC->HdpFbAddress);
}

/*
 *
 */
static Bool
RV515MCWaitIdle(struct rhdMC *MC)
{
    if (RHDReadMC(MC, MC_IND_ALL | RV515_MC_STATUS) & RV515_MC_IDLE)
	return TRUE;
    return FALSE;
}

/*
 *
 */
static CARD64
RV515MCGetFBLocation(struct rhdMC *MC, CARD32 *Size)
{
    return R5xxMCGetFBLocation(RHDReadMC(MC, RV515_MC_FB_LOCATION | MC_IND_ALL), Size);
}

/*
 *
 */
static void
RV515MCSetupFBLocation(struct rhdMC *MC, CARD64 Address, CARD32 Size)
{
    RHDWriteMC(MC, RV515_MC_FB_LOCATION | MC_IND_ALL,
	       R5XX_FB_LOCATION(Address, Size));
    RHDRegWrite(MC, HDP_FB_LOCATION, R5XX_HDP_LOCATION(Address));
}

/*
 *
 */
static void
RV515MCTuneMCAccessForDisplay(struct rhdMC *MC, int Crtc,
		    DisplayModePtr Mode, DisplayModePtr ScaledToMode)
{
    CARD32 value, setting = 0x1;

    value = RHDReadMC(MC, RV515_MC_MISC_LAT_TIMER);

    if (Crtc == RHD_CRTC_1) {
	value &= ~(0x0F << MC_DISP0R_INIT_LAT_SHIFT);
	value |= setting << MC_DISP0R_INIT_LAT_SHIFT;
    } else { /* RHD_CRTC_2 */
	value &= ~(0x0F << MC_DISP1R_INIT_LAT_SHIFT);
	value |= setting << MC_DISP1R_INIT_LAT_SHIFT;
    }

    RHDWriteMC(MC, RV515_MC_MISC_LAT_TIMER, value);
}

/*
 *
 */
static void
R500MCSave(struct rhdMC *MC)
{
    MC->FbLocation = RHDReadMC(MC, MC_IND_ALL | R5XX_MC_FB_LOCATION);
    MC->HdpFbAddress = RHDRegRead(MC, HDP_FB_LOCATION);
}

/*
 *
 */
static void
R500MCRestore(struct rhdMC *MC)
{
    RHDWriteMC(MC, MC_IND_ALL | R5XX_MC_FB_LOCATION, MC->FbLocation);
    RHDRegWrite(MC, HDP_FB_LOCATION, MC->HdpFbAddress);
}

/*
 *
 */
static Bool
R500MCWaitIdle(struct rhdMC *MC)
{
    if (RHDReadMC(MC, MC_IND_ALL | R5XX_MC_STATUS) & R5XX_MC_IDLE)
	return TRUE;
    return FALSE;
}

/*
 *
 */
static CARD64
R500MCGetFBLocation(struct rhdMC *MC, CARD32 *Size)
{
    return R5xxMCGetFBLocation(RHDReadMC(MC, R5XX_MC_FB_LOCATION | MC_IND_ALL), Size);
}

/*
 *
 */
static void
R500MCSetupFBLocation(struct rhdMC *MC, CARD64 Address, CARD32 Size)
{
    RHDWriteMC(MC, R5XX_MC_FB_LOCATION | MC_IND_ALL,
	       R5XX_FB_LOCATION(Address, Size));
    RHDRegWrite(MC, HDP_FB_LOCATION, R5XX_HDP_LOCATION(Address));
}

/*
 *
 */
static void
RS600MCSave(struct rhdMC *MC)
{
    MC->FbLocation = RHDReadMC(MC, RS60_NB_FB_LOCATION);
    MC->HdpFbAddress = RHDRegRead(MC, HDP_FB_LOCATION);
}

/*
 *
 */
static void
RS600MCRestore(struct rhdMC *MC)
{
    RHDWriteMC(MC, RS60_NB_FB_LOCATION, MC->FbLocation);
    RHDRegWrite(MC, HDP_FB_LOCATION, MC->HdpFbAddress);
}

/*
 *
 */
static Bool
RS600MCWaitIdle(struct rhdMC *MC)
{
    if (RHDReadMC(MC, RS60_MC_SYSTEM_STATUS) & RS6X_MC_SEQUENCER_IDLE)
	return TRUE;
    return FALSE;
}

/*
 *
 */
static CARD64
RS600MCGetFBLocation(struct rhdMC *MC, CARD32 *Size)
{
    return R5xxMCGetFBLocation(RHDReadMC(MC, RS60_NB_FB_LOCATION), Size);
}

/*
 *
 */
static void
RS600MCSetupFBLocation(struct rhdMC *MC, CARD64 Address, CARD32 Size)
{
    RHDWriteMC(MC, RS60_NB_FB_LOCATION, R5XX_FB_LOCATION(Address, Size));
    RHDRegWrite(MC, HDP_FB_LOCATION, R5XX_HDP_LOCATION(Address));
}

/*
 *
 */
static void
RS690MCSave(struct rhdMC *MC)
{
    MC->FbLocation = RHDReadMC(MC, RS69_MCCFG_FB_LOCATION);
    MC->HdpFbAddress = RHDRegRead(MC, HDP_FB_LOCATION);
    MC->MiscLatencyTimer = RHDReadMC(MC, RS69_MC_INIT_MISC_LAT_TIMER);

}

/*
 *
 */
static void
RS690MCRestore(struct rhdMC *MC)
{
    RHDWriteMC(MC, RS69_MCCFG_FB_LOCATION, MC->FbLocation);
    RHDRegWrite(MC, HDP_FB_LOCATION, MC->HdpFbAddress);
    RHDWriteMC(MC, RS69_MC_INIT_MISC_LAT_TIMER, MC->MiscLatencyTimer);
}

/*
 *
 */
static Bool
RS690MCWaitIdle(struct rhdMC *MC)
{
    if (RHDReadMC(MC, RS69_MC_SYSTEM_STATUS) & RS6X_MC_SEQUENCER_IDLE)
	return TRUE;
    return FALSE;
}

/*
 *
 */
static CARD64
RS690MCGetFBLocation(struct rhdMC *MC, CARD32 *Size)
{
    return R5xxMCGetFBLocation(RHDReadMC(MC, RS69_MCCFG_FB_LOCATION), Size);
}

/*
 *
 */
static void
RS690MCSetupFBLocation(struct rhdMC *MC, CARD64 Address, CARD32 Size)
{
    RHDWriteMC(MC, RS69_MCCFG_FB_LOCATION, R5XX_FB_LOCATION(Address, Size));
    RHDRegWrite(MC, HDP_FB_LOCATION, R5XX_HDP_LOCATION(Address));
}

/*
 *
 */
static void
RS690MCTuneMCAccessForDisplay(struct rhdMC *MC, int Crtc,
		      DisplayModePtr Mode, DisplayModePtr ScaledToMode)
{
    CARD32 value, setting = 0x1;

    value = RHDReadMC(MC, RS69_MC_INIT_MISC_LAT_TIMER);

    if (Crtc == RHD_CRTC_1) {
	value &= ~(0x0F << MC_DISP0R_INIT_LAT_SHIFT);
	value |= setting << MC_DISP0R_INIT_LAT_SHIFT;
    } else { /* RHD_CRTC_2 */
	value &= ~(0x0F << MC_DISP1R_INIT_LAT_SHIFT);
	value |= setting << MC_DISP1R_INIT_LAT_SHIFT;
    }

    RHDWriteMC(MC, RS69_MC_INIT_MISC_LAT_TIMER, value);
}

/*
 *
 */
static void
R600MCSave(struct rhdMC *MC)
{
    MC->FbLocation = RHDRegRead(MC, R6XX_MC_VM_FB_LOCATION);
    MC->HdpFbAddress = RHDRegRead(MC, R6XX_HDP_NONSURFACE_BASE);
}

/*
 *
 */
static void
R600MCRestore(struct rhdMC *MC)
{
    RHDRegWrite(MC, R6XX_MC_VM_FB_LOCATION, MC->FbLocation);
    RHDRegWrite(MC, R6XX_HDP_NONSURFACE_BASE, MC->HdpFbAddress);
}

/*
 *
 */
static Bool
R600MCWaitIdle(struct rhdMC *MC)
{
    if (!(RHDRegRead(MC, SRBM_STATUS) & 0x3f00))
	return TRUE;
    return FALSE;
}


/*
 *
 */
static CARD64
R600MCGetFBLocation(struct rhdMC *MC, CARD32 *Size)
{
    return R6xxMCGetFBLocation(RHDRegRead(MC, R6XX_MC_VM_FB_LOCATION), Size);
}

/*
 *
 */
static void
R600MCSetupFBLocation(struct rhdMC *MC, CARD64 Address, CARD32 Size)
{
    RHDRegWrite(MC, R6XX_MC_VM_FB_LOCATION, R6XX_FB_LOCATION(Address, Size));
    RHDRegWrite(MC, R6XX_HDP_NONSURFACE_BASE, R6XX_HDP_LOCATION(Address));
}

/*
 *
 */
#ifdef NOTYET

/*
 *
 */
static void
RS780MCSave(struct rhdMC *MC)
{
    MC->FbLocation = RHDReadMC(MC, RS78_MC_FB_LOCATION);
    MC->HdpFbAddress = RHDRegRead(MC, R6XX_HDP_NONSURFACE_BASE);
}

/*
 *
 */
static void
RS780MCRestore(struct rhdMC *MC)
{
    RHDWriteMC(MC, RS78_MC_FB_LOCATION, MC->FbLocation);
    RHDRegWrite(MC, R6XX_HDP_NONSURFACE_BASE, MC->HdpFbAddress);
}

/*
 *
 */
static Bool
RS780MCWaitIdle(struct rhdMC *MC)
{
    if (RHDReadMC(MC, RS78_MC_SYSTEM_STATUS) & RS78_MC_SEQUENCER_IDLE)
	return TRUE;
    return FALSE;
}

/*
 *
 */
static CARD64
RS780MCGetFBLocation(struct rhdMC *MC, CARD32 *Size)
{
    /* is this correct? */

    return R5xxMCGetFBLocation(RHDReadMC(MC, RS78_MC_FB_LOCATION), Size);
}

/*
 *
 */
static void
RS780MCSetupFBLocation(struct rhdMC *MC, CARD64 Address, CARD32 Size)
{
    /* is this correct? */
    RHDWriteMC(MC, RS78_MC_FB_LOCATION, R5XX_FB_LOCATION(Address, Size));
    RHDRegWrite(MC, R6XX_HDP_NONSURFACE_BASE, R6XX_HDP_LOCATION(Address));
}
#endif /* NOTYET */

/*
 *
 */
static void
R700MCSave(struct rhdMC *MC)
{
    MC->FbLocation = RHDRegRead(MC, R7XX_MC_VM_FB_LOCATION);
    MC->HdpFbAddress = RHDRegRead(MC, R6XX_HDP_NONSURFACE_BASE);
}

/*
 *
 */
static void
R700MCRestore(struct rhdMC *MC)
{
    RHDFUNC(MC);

    RHDRegWrite(MC, R7XX_MC_VM_FB_LOCATION, MC->FbLocation);
    RHDRegWrite(MC, R6XX_HDP_NONSURFACE_BASE, MC->HdpFbAddress);
}

/*
 * Idle is the R600 one...
 */

/*
 *
 */
static CARD64
R700MCGetFBLocation(struct rhdMC *MC, CARD32 *Size)
{
    return R6xxMCGetFBLocation(RHDRegRead(MC, R7XX_MC_VM_FB_LOCATION), Size);
}

/*
 *
 */
static void
R700MCSetupFBLocation(struct rhdMC *MC, CARD64 Address, CARD32 Size)
{
    RHDRegWrite(MC, R7XX_MC_VM_FB_LOCATION, R6XX_FB_LOCATION(Address, Size));
    RHDRegWrite(MC, R6XX_HDP_NONSURFACE_BASE, R6XX_HDP_LOCATION(Address));
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
     * sure that it is made consistent in MCSetupFBLocation and the various MC
     * accessing subsystems.
     */

    RHDDebug(rhdPtr->scrnIndex, "MC FB Address: 0x%08X.\n",
	     rhdPtr->FbIntAddress);

    MC = xnfcalloc(1, sizeof(struct rhdMC));
    MC->scrnIndex = rhdPtr->scrnIndex;

    if (rhdPtr->ChipSet < RHD_RS600) {
	switch(rhdPtr->ChipSet) {
	case RHD_RV515:
	case RHD_RV505:
	case RHD_RV516:
	case RHD_RV550:
	case RHD_M52:
	case RHD_M54:
	case RHD_M62:
	case RHD_M64:
	case RHD_M71:
	    MC->Save = RV515MCSave;
	    MC->Restore = RV515MCRestore;
	    MC->SetupFBLocation = RV515MCSetupFBLocation;
	    MC->GetFBLocation = RV515MCGetFBLocation;
	    MC->Idle = RV515MCWaitIdle;
	    MC->TuneAccessForDisplay = RV515MCTuneMCAccessForDisplay;
	    break;
	default:
	    MC->Save = R500MCSave;
	    MC->Restore = R500MCRestore;
	    MC->SetupFBLocation = R500MCSetupFBLocation;
	    MC->GetFBLocation = R500MCGetFBLocation;
	    MC->Idle = R500MCWaitIdle;
	    break;
	}

    } else if (rhdPtr->ChipSet == RHD_RS600) {
	MC->Save = RS600MCSave;
	MC->Restore = RS600MCRestore;
	MC->SetupFBLocation = RS600MCSetupFBLocation;
	MC->Idle = RS600MCWaitIdle;
	MC->GetFBLocation = RS600MCGetFBLocation;
    } else if (rhdPtr->ChipSet < RHD_R600) {
	MC->Save = RS690MCSave;
	MC->Restore = RS690MCRestore;
	MC->SetupFBLocation = RS690MCSetupFBLocation;
	MC->Idle = RS690MCWaitIdle;
	MC->GetFBLocation = RS690MCGetFBLocation;
	MC->TuneAccessForDisplay = RS690MCTuneMCAccessForDisplay;
    } else if (rhdPtr->ChipSet <= RHD_RS780) {
	MC->Save = R600MCSave;
	MC->Restore = R600MCRestore;
	MC->SetupFBLocation = R600MCSetupFBLocation;
	MC->Idle = R600MCWaitIdle;
	MC->GetFBLocation = R600MCGetFBLocation;
    }
#ifdef NOTYET
    else if (rhdPtr->ChipSet == RHD_RS780) {
	MC->Save = RS780MCSave;
	MC->Restore = RS780MCRestore;
	MC->SetupFBLocation = RS780MCSetupFBLocation;
	MC->Idle = RS780MCWaitIdle;
	MC->GetFBLocation = RS780MCGetFBLocation;
    }
#endif /* NOTYET */
    else if (rhdPtr->ChipSet >= RHD_RV770) {
	MC->Save = R700MCSave;
	MC->Restore = R700MCRestore;
	MC->SetupFBLocation = R700MCSetupFBLocation;
	MC->Idle = R600MCWaitIdle;
	MC->GetFBLocation = R700MCGetFBLocation;
    } else {
	xf86DrvMsg(rhdPtr->scrnIndex, X_ERROR, "I don't know anything about MC on this chipset\n");
	xfree(MC);
	return;
    }
    if (rhdPtr->ChipSet < RHD_R600)
	rhdPtr->FbIntAddress = RHDRegRead(rhdPtr, HDP_FB_LOCATION) << 16;
    else
	rhdPtr->FbIntAddress = RHDRegRead(rhdPtr, R6XX_CONFIG_FB_BASE);
    MC->GetFBLocation(MC, &rhdPtr->FbIntSize);

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
void
RHDMCSave(RHDPtr rhdPtr)
{
    struct rhdMC *MC = rhdPtr->MC;

    ASSERT(MC);

    RHDFUNC(rhdPtr);

    MC->Save(MC);

    MC->Stored = TRUE;
}

/*
 * Make sure that nothing is accessing memory anymore before calling this.
 */
void
RHDMCRestore(RHDPtr rhdPtr)
{
    struct rhdMC *MC = rhdPtr->MC;

 //   ASSERT(MC);
 //   RHD_UNSETDEBUGFLAG(rhdPtr, MC_SETUP);

    RHDFUNC(rhdPtr);

    if (!MC->Stored) {
	xf86DrvMsg(rhdPtr->scrnIndex, X_ERROR,
		   "%s: trying to restore uninitialized values.\n",__func__);
	return;
    }

    if (MC->Idle(MC))
	MC->Restore(MC);
    else
	xf86DrvMsg(rhdPtr->scrnIndex, X_ERROR,
		   "%s: MC is still not idle!!!\n", __func__);
}

/*
 *
 */
Bool
RHDMCIdleWait(RHDPtr rhdPtr, CARD32 count)
{
    struct rhdMC *MC = rhdPtr->MC;

    RHDFUNC(rhdPtr);

    ASSERT(MC);

    do {
	if (MC->Idle(MC))
	    return TRUE;
    usleep(100);
    } while (count--);

    RHDDebug(rhdPtr->scrnIndex, "%s: MC not idle\n",__func__);

    return FALSE;
}

/*
 * Get FB location and size.
 */
CARD64
RHDMCGetFBLocation(RHDPtr rhdPtr, CARD32 *size)
{
    struct rhdMC *MC = rhdPtr->MC;

 //   ASSERT(MC);
 //   ASSERT(size);

    RHDFUNC(rhdPtr);

    return MC->GetFBLocation(MC, size);
}

/*
 * Make sure that nothing is accessing memory anymore before calling this.
 */
Bool
RHDMCSetupFBLocation(RHDPtr rhdPtr, CARD64 Address, CARD32 Size)
{
    struct rhdMC *MC = rhdPtr->MC;
    CARD64 OldAddress;
    CARD32 OldSize;

 //   ASSERT(MC);
 //   RHD_SETDEBUGFLAG(rhdPtr, MC_SETUP);

    RHDFUNC(rhdPtr);

    if (!MC->Idle(MC)) {
	xf86DrvMsg(rhdPtr->scrnIndex, X_ERROR,
		   "%s: Cannot setup MC: not idle!!!\n", __func__);
	return FALSE;
    }

    OldAddress = MC->GetFBLocation(MC, &OldSize);
    if (OldAddress == Address && OldSize == Size)
	return TRUE;

    /* If this ever occurs, we might have issues */
    if (OldAddress >> 32)
	xf86DrvMsg(rhdPtr->scrnIndex, X_WARNING, "%s: Board claims to use a "
		   "higher than 32bit address for its FB\n", __func__);

    RHDDebug(rhdPtr->scrnIndex,
	     "Setting MC from 0x%08X to 0x%08X [Size 0x%08X]\n",
	     OldAddress, rhdPtr->FbIntAddress, Size);

    MC->SetupFBLocation(MC, Address, Size);

    return TRUE;
}

/*
 *
 */
void
RHDMCTuneAccessForDisplay(RHDPtr rhdPtr, int Crtc,
		    DisplayModePtr Mode, DisplayModePtr ScaledToMode)
{
    struct rhdMC *MC = rhdPtr->MC;

    ASSERT(MC);

    RHDFUNC(rhdPtr);

    if (MC->TuneAccessForDisplay)
	MC->TuneAccessForDisplay(MC, Crtc, Mode, ScaledToMode);
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
    xf86DrvMsg(rhdPtr->scrnIndex, X_INFO, "IGP sideport memory %s present.\n", Present ? "" : "not");

    return Present;
}
