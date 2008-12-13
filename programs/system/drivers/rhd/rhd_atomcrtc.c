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

#if HAVE_XF86_ANSIC_H
# include "xf86_ansic.h"
#else
# include <unistd.h>
#endif

#include "rhd.h"
#include "rhd_crtc.h"
#include "rhd_pll.h"
#include "rhd_lut.h"
#include "rhd_regs.h"
#include "rhd_modes.h"
#include "rhd_mc.h"
#if defined (ATOM_BIOS) && defined (ATOM_BIOS_PARSER)
# include "rhd_atombios.h"

# define D1_REG_OFFSET 0x0000
# define D2_REG_OFFSET 0x0800

struct rhdCrtcScalePrivate {
    void *RegList;
    CARD32 StoreViewportSize;
    CARD32 StoreViewportStart;
};

/*
 *
 */
static void
rhdAtomCrtcRestore(struct rhdCrtc *Crtc, void *Store)
{
    RHDPtr rhdPtr = RHDPTRI(Crtc);
    union AtomBiosArg data;

    RHDFUNC(rhdPtr);

    data.Address = Store;
    RHDAtomBiosFunc(Crtc->scrnIndex, rhdPtr->atomBIOS, ATOM_RESTORE_REGISTERS, &data);
}

/*
 *
 */
static void
rhdAtomScaleSet(struct rhdCrtc *Crtc, enum rhdCrtcScaleType Type,
	   DisplayModePtr Mode, DisplayModePtr ScaledToMode)
{
    RHDPtr rhdPtr = RHDPTRI(Crtc);
    struct rhdScalerOverscan Overscan;
    struct atomCrtcOverscan AtomOverscan;
    enum atomCrtc AtomCrtc = RHD_CRTC_1;
    enum atomScaler Scaler  = 0;
    enum atomScaleMode ScaleMode = 0;
    union AtomBiosArg data;
    CARD32 RegOff = 0;

    RHDDebug(Crtc->scrnIndex, "FUNCTION: %s: %s viewport: %ix%i\n", __func__, Crtc->Name,
	     Mode->CrtcHDisplay, Mode->CrtcVDisplay);

    /* D1Mode registers */
    if (Crtc->Id == RHD_CRTC_1)
	RegOff = D1_REG_OFFSET;
    else
	RegOff = D2_REG_OFFSET;

    RHDRegWrite(Crtc, RegOff + D1MODE_VIEWPORT_SIZE,
		Mode->CrtcVDisplay | (Mode->CrtcHDisplay << 16));
    RHDRegWrite(Crtc, RegOff + D1MODE_VIEWPORT_START, 0);

    Overscan = rhdCalculateOverscan(Mode, ScaledToMode, Type);
    Type = Overscan.Type;

    ASSERT(Crtc->ScalePriv);
    data.Address = &((Crtc->ScalePriv)->RegList);
    RHDAtomBiosFunc(rhdPtr->scrnIndex, rhdPtr->atomBIOS, ATOM_SET_REGISTER_LIST_LOCATION, &data);

    AtomOverscan.ovscnLeft = Overscan.OverscanLeft;
    AtomOverscan.ovscnRight = Overscan.OverscanRight;
    AtomOverscan.ovscnTop = Overscan.OverscanTop;
    AtomOverscan.ovscnBottom = Overscan.OverscanBottom;

    switch (Crtc->Id) {
	case  RHD_CRTC_1:
	    Scaler = atomScaler1;
	    AtomCrtc = atomCrtc1;
	    break;
	case RHD_CRTC_2:
	    Scaler = atomScaler2;
	    AtomCrtc = atomCrtc2;
	    break;
    }

    rhdAtomSetCRTCOverscan(rhdPtr->atomBIOS, AtomCrtc, &AtomOverscan);

    switch (Type) {
	case RHD_CRTC_SCALE_TYPE_NONE:
	    ScaleMode = atomScaleDisable;
	    break;
	case RHD_CRTC_SCALE_TYPE_CENTER:
	    ScaleMode = atomScaleCenter;
	    break;
	case RHD_CRTC_SCALE_TYPE_SCALE:
	case RHD_CRTC_SCALE_TYPE_SCALE_KEEP_ASPECT_RATIO: /* scaled to fullscreen */
	    ScaleMode = atomScaleExpand;
	    break;
    }
    rhdAtomSetScaler(rhdPtr->atomBIOS, Scaler, ScaleMode);

    data.Address = NULL;
    RHDAtomBiosFunc(rhdPtr->scrnIndex, rhdPtr->atomBIOS, ATOM_SET_REGISTER_LIST_LOCATION, &data);

    RHDTuneMCAccessForDisplay(rhdPtr, Crtc->Id, Mode,
			      ScaledToMode ? ScaledToMode : Mode);
}

/*
 *
 */
static void
rhdAtomScaleSave(struct rhdCrtc *Crtc)
{
    struct rhdCrtcScalePrivate* ScalePriv;
    CARD32 RegOff = 0;

    RHDFUNC(Crtc);

    if (!Crtc->ScalePriv) {
	if(!(ScalePriv = (struct rhdCrtcScalePrivate*)xnfcalloc(1, sizeof(struct rhdCrtcScalePrivate))))
	    return;
	Crtc->ScalePriv = ScalePriv;
    } else
	ScalePriv = Crtc->ScalePriv;

    if (Crtc->Id == RHD_CRTC_1)
	RegOff = D1_REG_OFFSET;
    else
	RegOff = D2_REG_OFFSET;

    ScalePriv->StoreViewportSize  = RHDRegRead(Crtc, RegOff + D1MODE_VIEWPORT_SIZE);
    ScalePriv->StoreViewportStart = RHDRegRead(Crtc, RegOff + D1MODE_VIEWPORT_START);
    ScalePriv->RegList = NULL;
}

/*
 *
 */
static void
rhdAtomCrtcScaleRestore(struct rhdCrtc *Crtc)
{
    struct rhdCrtcScalePrivate* ScalePriv;
    CARD32 RegOff = 0;

    RHDFUNC(Crtc);

    rhdAtomCrtcRestore(Crtc, &(((struct rhdCrtcScalePrivate*)Crtc->ScalePriv)->RegList));

    if (Crtc->Id == RHD_CRTC_1)
	RegOff = D1_REG_OFFSET;
    else
	RegOff = D2_REG_OFFSET;

    ScalePriv = (struct rhdCrtcScalePrivate*)Crtc->ScalePriv;
    RHDRegWrite(Crtc, RegOff + D1MODE_VIEWPORT_SIZE, ScalePriv->StoreViewportSize);
    RHDRegWrite(Crtc, RegOff + D1MODE_VIEWPORT_START, ScalePriv->StoreViewportStart);
}

/*
 *
 */
static void
rhdAtomCrtcScaleDestroy(struct rhdCrtc *Crtc)
{
    RHDFUNC(Crtc);

    if (Crtc->ScalePriv) {
	xfree(Crtc->ScalePriv->RegList);
	xfree(Crtc->ScalePriv);
	Crtc->ScalePriv = NULL;
    }
}

/*
 *
 */
struct rhdCrtcModePrivate {
    void *RegList;
    CARD32 StoreModeDataFormat;
};

static void
rhdAtomModeSet(struct rhdCrtc *Crtc, DisplayModePtr Mode)
{
    RHDPtr rhdPtr = RHDPTRI(Crtc);
    union AtomBiosArg data;
    CARD32 RegOff = 0;

    RHDFUNC(rhdPtr);

    ASSERT(Crtc->ModePriv);
    data.Address = &((Crtc->ModePriv)->RegList);
    RHDAtomBiosFunc(rhdPtr->scrnIndex, rhdPtr->atomBIOS, ATOM_SET_REGISTER_LIST_LOCATION, &data);

    if (!rhdAtomSetCRTCTimings(rhdPtr->atomBIOS,
			      Crtc->Id == RHD_CRTC_1 ? atomCrtc1 : atomCrtc2,
                  Mode, 32))
	xf86DrvMsg(rhdPtr->scrnIndex, X_ERROR, "%s: failed to set mode.\n",__func__);

    /* set interlaced - AtomBIOS never sets the data format - never tested? */
    if (Crtc->Id == RHD_CRTC_1)
	RegOff = D1_REG_OFFSET;
    else
	RegOff = D2_REG_OFFSET;

    if (Mode->Flags & V_INTERLACE)
	RHDRegWrite(Crtc, RegOff + D1MODE_DATA_FORMAT, 0x1);
    else
	RHDRegWrite(Crtc, RegOff + D1MODE_DATA_FORMAT, 0x0);

    data.Address = NULL;
    RHDAtomBiosFunc(Crtc->scrnIndex, rhdPtr->atomBIOS, ATOM_SET_REGISTER_LIST_LOCATION, &data);
}

/*
 *
 */
static void
rhdAtomCrtcPower(struct rhdCrtc *Crtc, int Power)
{
    RHDPtr rhdPtr = RHDPTRI(Crtc);
    enum atomCrtc AtomCrtc = atomCrtc1;
    union AtomBiosArg data;

    RHDFUNC(Crtc);

    switch (Crtc->Id) {
	case RHD_CRTC_1:
	    AtomCrtc = atomCrtc1;
	    break;
	case RHD_CRTC_2:
	    AtomCrtc = atomCrtc2;
	    break;
    }
    data.Address = &((Crtc->ModePriv)->RegList);
    RHDAtomBiosFunc(rhdPtr->scrnIndex, rhdPtr->atomBIOS, ATOM_SET_REGISTER_LIST_LOCATION, &data);

    /*
     * We call  rhdAtomEnableCrtcMemReq blindly as this table seemed to have existed in all
     * versions of AtomBIOS on the hardware we support
     */
    switch (Power) {
	case RHD_POWER_ON:
	    rhdAtomEnableCrtcMemReq(rhdPtr->atomBIOS, AtomCrtc, atomCrtcEnable);
	    rhdAtomEnableCrtc(rhdPtr->atomBIOS, AtomCrtc, atomCrtcEnable);
	    break;
	case RHD_POWER_RESET:
	case RHD_POWER_SHUTDOWN:
	default:
	    rhdAtomEnableCrtc(rhdPtr->atomBIOS, AtomCrtc, atomCrtcDisable);
	    rhdAtomEnableCrtcMemReq(rhdPtr->atomBIOS, AtomCrtc, atomCrtcDisable);
	    break;
    }
    data.Address = NULL;
    RHDAtomBiosFunc(Crtc->scrnIndex, rhdPtr->atomBIOS, ATOM_SET_REGISTER_LIST_LOCATION, &data);
}

/*
 *
 */
static void
rhdAtomCrtcBlank(struct rhdCrtc *Crtc, Bool Blank)
{
    RHDPtr rhdPtr = RHDPTRI(Crtc);
    enum atomCrtc AtomCrtc = atomCrtc1;
    struct atomCrtcBlank Config;
    union AtomBiosArg data;

    RHDFUNC(Crtc);

    switch (Crtc->Id) {
	case RHD_CRTC_1:
	    AtomCrtc = atomCrtc1;
	    break;
	case RHD_CRTC_2:
	    AtomCrtc = atomCrtc2;
	    break;
    }
    if (Blank)
	Config.Action = atomBlankOn;
    else
	Config.Action = atomBlankOff;

    Config.r = Config.g = Config.b = 0;

    data.Address = &((Crtc->ModePriv)->RegList);
    RHDAtomBiosFunc(rhdPtr->scrnIndex, rhdPtr->atomBIOS, ATOM_SET_REGISTER_LIST_LOCATION, &data);

    rhdAtomBlankCRTC(rhdPtr->atomBIOS, AtomCrtc , &Config);

    data.Address = NULL;
    RHDAtomBiosFunc(Crtc->scrnIndex, rhdPtr->atomBIOS, ATOM_SET_REGISTER_LIST_LOCATION, &data);
}

/*
 *
 */
static void
rhdAtomModeSave(struct rhdCrtc *Crtc)
{
    struct rhdCrtcModePrivate* ModePriv;
    CARD32 RegOff = 0;

    if (!Crtc->ModePriv) {
	if(!(ModePriv = (struct rhdCrtcModePrivate*)xnfcalloc(1, sizeof(struct rhdCrtcModePrivate))))
	    return;
	Crtc->ModePriv = ModePriv;
    } else
	ModePriv = Crtc->ModePriv;

    if (Crtc->Id == RHD_CRTC_1)
	RegOff = D1_REG_OFFSET;
    else
	RegOff = D2_REG_OFFSET;

    ModePriv->StoreModeDataFormat  = RHDRegRead(Crtc, RegOff + D1MODE_DATA_FORMAT);
    ModePriv->RegList = NULL;
}

/*
 *
 */
static void
rhdAtomModeRestore(struct rhdCrtc *Crtc)
{
    struct rhdCrtcModePrivate* ModePriv;
    CARD32 RegOff = 0;

    if (Crtc->Id == RHD_CRTC_1)
	RegOff = D1_REG_OFFSET;
    else
	RegOff = D2_REG_OFFSET;

    ModePriv = Crtc->ModePriv;

    rhdAtomCrtcRestore(Crtc, &ModePriv->RegList);

   RHDRegWrite(Crtc, RegOff + D1MODE_DATA_FORMAT, ModePriv->StoreModeDataFormat);
}

/*
 *
 */
static void
rhdAtomModeDestroy(struct rhdCrtc *Crtc)
{
    RHDFUNC(Crtc);

    if (Crtc->ModePriv) {
	xfree(Crtc->ModePriv->RegList);
	xfree(Crtc->ModePriv);
	Crtc->ModePriv = NULL;
    }
}

/*
 *
 */
void
RHDAtomCrtcsInit(RHDPtr rhdPtr)
{
    struct rhdCrtc *Crtc;
    int i;

    RHDFUNC(rhdPtr);

    if (rhdPtr->Crtc[0] == NULL || rhdPtr->Crtc[1] == NULL) {
	xf86DrvMsg(rhdPtr->scrnIndex, X_ERROR, "%s: CRTCs not initialized\n",__func__);
	return;
    }

    for (i = 0; i < 2; i++) {

	Crtc = rhdPtr->Crtc[i];

	if (i == 0) {
	    Crtc->Name = "ATOM CRTC 1";
	    Crtc->Id = RHD_CRTC_1;
	} else {
	    Crtc->Name = "ATOM CRTC 2";
	    Crtc->Id = RHD_CRTC_2;
	}

	/* EnableGraphSurfaces is only a BIOS internal table. So use the hardcoded path.
	Crtc->FBValid = atomFBValid;
	Crtc->FBSet = atomFBSet;
	Crtc->FBSave = atomSave;
	Crtc->FBRestore = atomRestore;
	*/

	/* There is no separate function to set up the LUT thru AtomBIOS */

	/* Crtc->ScaleValid: From rhd_crtc.c */
	Crtc->ScaleSet = rhdAtomScaleSet;
	Crtc->ScaleSave = rhdAtomScaleSave;
	Crtc->ScaleRestore = rhdAtomCrtcScaleRestore;
	Crtc->ScaleDestroy = rhdAtomCrtcScaleDestroy;

	/* No such AtomBIOS table */
	/* Crtc->FrameSet = atomViewPortStart; */

	/* Crtc->ModeValid: From rhd_crtc.c */
	Crtc->ModeSet = rhdAtomModeSet;
	Crtc->ModeSave = rhdAtomModeSave;
	Crtc->ModeRestore = rhdAtomModeRestore;
	Crtc->ModeDestroy = rhdAtomModeDestroy;

	Crtc->Power = rhdAtomCrtcPower;
	Crtc->Blank = rhdAtomCrtcBlank;
    }
}

#endif /* ATOM_BIOS && ATOM_BIOS_PARSER */
