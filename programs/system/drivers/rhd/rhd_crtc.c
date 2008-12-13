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
#include "rhd_pll.h"
#include "rhd_lut.h"
#include "rhd_regs.h"
#include "rhd_modes.h"
#include "rhd_mc.h"
#ifdef ATOM_BIOS
# include "rhd_atombios.h"
#endif

#define D1_REG_OFFSET 0x0000
#define D2_REG_OFFSET 0x0800
#define FMT1_REG_OFFSET 0x0000
#define FMT2_REG_OFFSET 0x800

struct rhdCrtcFMTPrivate {
    CARD32 StoreControl;
    CARD32 StoreBitDepthControl;
    CARD32 StoreClampCntl;
};

struct rhdCrtcFBPrivate {
    CARD32 StoreGrphEnable;
    CARD32 StoreGrphControl;
    CARD32 StoreGrphXStart;
    CARD32 StoreGrphYStart;
    CARD32 StoreGrphXEnd;
    CARD32 StoreGrphYEnd;
    CARD32 StoreGrphSwap;
    CARD32 StoreGrphPrimarySurfaceAddress;
    CARD32 StoreGrphSurfaceOffsetX;
    CARD32 StoreGrphSurfaceOffsetY;
    CARD32 StoreGrphPitch;
    CARD32 StoreModeDesktopHeight;
};

struct rhdCrtcLUTPrivate {
    CARD32 StoreGrphLutSel;
};

struct rhdCrtcScalePrivate {
    CARD32 StoreModeViewPortSize;

    CARD32 StoreModeOverScanH;
    CARD32 StoreModeOverScanV;

    CARD32 StoreModeViewPortStart;
    CARD32 StoreScaleEnable;
    CARD32 StoreScaleTapControl;
    CARD32 StoreModeCenter;
    CARD32 StoreScaleHV;
    CARD32 StoreScaleHFilter;
    CARD32 StoreScaleVFilter;
    CARD32 StoreScaleDither;
};

struct rhdCrtcModePrivate {
    CARD32 StoreCrtcControl;

    CARD32 StoreCrtcHTotal;
    CARD32 StoreCrtcHBlankStartEnd;
    CARD32 StoreCrtcHSyncA;
    CARD32 StoreCrtcHSyncACntl;
    CARD32 StoreCrtcHSyncB;
    CARD32 StoreCrtcHSyncBCntl;

    CARD32 StoreCrtcVTotal;
    CARD32 StoreCrtcVBlankStartEnd;
    CARD32 StoreCrtcVSyncA;
    CARD32 StoreCrtcVSyncACntl;
    CARD32 StoreCrtcVSyncB;
    CARD32 StoreCrtcVSyncBCntl;
    CARD32 StoreCrtcCountControl;

    CARD32 StoreModeDataFormat;
    CARD32 StoreCrtcInterlaceControl;

    CARD32 StoreCrtcBlackColor;
    CARD32 StoreCrtcBlankControl;
};

/*
 * Checks whether Width, Height are within boundaries.
 * If MODE_OK is returned and pPitch is not NULL, it is set.
 */
static ModeStatus
DxFBValid(struct rhdCrtc *Crtc, CARD16 Width, CARD16 Height, int bpp,
	  CARD32 Offset, CARD32 Size, CARD32 *pPitch)
{
    RHDPtr rhdPtr = RHDPTRI(Crtc);
    ScrnInfoPtr pScrn = rhdPtr->pScrn;

    CARD16 Pitch;
    unsigned int BytesPerPixel;
    CARD8 PitchMask = 0xFF;

    RHDDebug(Crtc->scrnIndex, "FUNCTION: %s: %s\n", __func__, Crtc->Name);

    /* If we hit this, then the memory claimed so far is not properly aligned */
    if (Offset & 0xFFF) {
	xf86DrvMsg(Crtc->scrnIndex, X_ERROR, "%s: Offset (0x%08X) is invalid!\n",
		   __func__, (int) Offset);
      return MODE_ERROR;
    }

    switch (pScrn->bitsPerPixel) {
      case 8:
        BytesPerPixel = 1;
        break;
      case 15:
      case 16:
        BytesPerPixel = 2;
        PitchMask /= BytesPerPixel;
        break;
     case 24:
     case 32:
       BytesPerPixel = 4;
       PitchMask /= BytesPerPixel;
       break;
     default:
	xf86DrvMsg(pScrn->scrnIndex, X_WARNING, "%s: %dbpp is not implemented!\n",
		   __func__, pScrn->bitsPerPixel);
       return MODE_BAD;
    }

    if((Width==720)&&(Height==400))    //skip textmode
      return MODE_BAD;

     /* Be reasonable */
    if (Width < 640)
      return MODE_H_ILLEGAL;
    if (Height < 480)
      return MODE_V_ILLEGAL;

    /* D1GRPH_X_START is 14bits while D1_MODE_VIEWPORT_X_START is only 13 bits.
     * Since it is reasonable to assume that modes will be at least 1x1
     * limit at 13bits + 1 */
    if (Width > 0x2000)
      return MODE_VIRTUAL_X;

    /* D1GRPH_Y_START is 14bits while D1_MODE_VIEWPORT_Y_START is only 13 bits.
     * Since it is reasonable to assume that modes will be at least 1x1
     * limit at 13bits + 1 */
    if (Height > 0x2000)
      return MODE_VIRTUAL_Y;

    Pitch = (Width + PitchMask) & ~PitchMask;
    /* D1_PITCH limit: should never happen after clamping Width to 0x2000 */
    if (Pitch >= 0x4000)
      return MODE_VIRTUAL_X;

    if ((Pitch * BytesPerPixel * Height) > Size)
      return MODE_MEM_VIRT;

    if (pPitch)
      *pPitch = Pitch;
    return MODE_OK;
}

/*
 *
 */
static void
DxFBSet(struct rhdCrtc *Crtc, CARD16 Pitch, CARD16 Width, CARD16 Height,
	int bpp, CARD32 Offset)
{
    RHDPtr rhdPtr = RHDPTRI(Crtc);
    CARD16 RegOff;

    RHDDebug(Crtc->scrnIndex, "FUNCTION: %s: %s (%i[%i]x%i@%ibpp)  +0x%x )\n",
	     __func__, Crtc->Name, Width, Pitch, Height, bpp, Offset);

    if (Crtc->Id == RHD_CRTC_1)
      RegOff = D1_REG_OFFSET;
    else
      RegOff = D2_REG_OFFSET;

    RHDRegMask(Crtc, RegOff + D1GRPH_ENABLE, 1, 0x00000001);

    /* disable R/B swap, disable tiling, disable 16bit alpha, etc. */
    RHDRegWrite(Crtc, RegOff + D1GRPH_CONTROL, 0);

    switch (bpp) {
      case 8:
        RHDRegMask(Crtc, RegOff + D1GRPH_CONTROL, 0, 0x00000703);
        break;
      case 15:
        RHDRegMask(Crtc, RegOff + D1GRPH_CONTROL, 0x000001, 0x00000703);
        break;
      case 16:
        RHDRegMask(Crtc, RegOff + D1GRPH_CONTROL, 0x000101, 0x00000703);
        break;
      case 24:
      case 32:
        default:
        RHDRegMask(Crtc, RegOff + D1GRPH_CONTROL, 0x000002, 0x00000703);
        break;
    /* TODO: 64bpp ;p */
    }

    /* Make sure that we are not swapping colours around */
    if (rhdPtr->ChipSet > RHD_R600)
      RHDRegWrite(Crtc, RegOff + D1GRPH_SWAP_CNTL, 0);
    /* R5xx - RS690 case is GRPH_CONTROL bit 16 */

    RHDRegWrite(Crtc, RegOff + D1GRPH_PRIMARY_SURFACE_ADDRESS,
                rhdPtr->FbIntAddress + Offset);
    RHDRegWrite(Crtc, RegOff + D1GRPH_PITCH, Pitch);
    RHDRegWrite(Crtc, RegOff + D1GRPH_SURFACE_OFFSET_X, 0);
    RHDRegWrite(Crtc, RegOff + D1GRPH_SURFACE_OFFSET_Y, 0);
    RHDRegWrite(Crtc, RegOff + D1GRPH_X_START, 0);
    RHDRegWrite(Crtc, RegOff + D1GRPH_Y_START, 0);
    RHDRegWrite(Crtc, RegOff + D1GRPH_X_END, Width);
    RHDRegWrite(Crtc, RegOff + D1GRPH_Y_END, Height);

    /* D1Mode registers */
    RHDRegWrite(Crtc, RegOff + D1MODE_DESKTOP_HEIGHT, Height);

    Crtc->Pitch = Pitch;
    Crtc->Width = Width;
    Crtc->Height = Height;
    Crtc->bpp = bpp;
    Crtc->Offset = Offset;
}

/*
 *
 */
static void
DxFBSave(struct rhdCrtc *Crtc)
{
    struct rhdCrtcFBPrivate *FBPriv;
    CARD32 RegOff;

    if (!Crtc->FBPriv)
	FBPriv = xnfcalloc(1, sizeof(struct rhdCrtcFBPrivate));
    else
	FBPriv = Crtc->FBPriv;

    if (Crtc->Id == RHD_CRTC_1)
	RegOff = D1_REG_OFFSET;
    else
	RegOff = D2_REG_OFFSET;

    FBPriv->StoreGrphEnable = RHDRegRead(Crtc, RegOff + D1GRPH_ENABLE);
    FBPriv->StoreGrphControl = RHDRegRead(Crtc, RegOff + D1GRPH_CONTROL);
    FBPriv->StoreGrphXStart = RHDRegRead(Crtc, RegOff + D1GRPH_X_START);
    FBPriv->StoreGrphYStart = RHDRegRead(Crtc, RegOff + D1GRPH_Y_START);
    FBPriv->StoreGrphXEnd = RHDRegRead(Crtc, RegOff + D1GRPH_X_END);
    FBPriv->StoreGrphYEnd = RHDRegRead(Crtc, RegOff + D1GRPH_Y_END);
    if (RHDPTRI(Crtc)->ChipSet >= RHD_R600)
	FBPriv->StoreGrphSwap = RHDRegRead(Crtc, RegOff + D1GRPH_SWAP_CNTL);
    FBPriv->StoreGrphPrimarySurfaceAddress =
	RHDRegRead(Crtc, RegOff + D1GRPH_PRIMARY_SURFACE_ADDRESS);
    FBPriv->StoreGrphSurfaceOffsetX =
	RHDRegRead(Crtc, RegOff + D1GRPH_SURFACE_OFFSET_X);
    FBPriv->StoreGrphSurfaceOffsetY =
	RHDRegRead(Crtc, RegOff + D1GRPH_SURFACE_OFFSET_Y);
    FBPriv->StoreGrphPitch = RHDRegRead(Crtc, RegOff + D1GRPH_PITCH);
    FBPriv->StoreModeDesktopHeight = RHDRegRead(Crtc, RegOff + D1MODE_DESKTOP_HEIGHT);

    Crtc->FBPriv = FBPriv;
}

/*
 *
 */
static void
DxFBRestore(struct rhdCrtc *Crtc)
{
    struct rhdCrtcFBPrivate *FBPriv = Crtc->FBPriv;
    CARD32 RegOff;

    if (!FBPriv) {
	xf86DrvMsg(Crtc->scrnIndex, X_ERROR, "%s: no registers stored!\n",
		   __func__);
	return;
    }

    if (Crtc->Id == RHD_CRTC_1)
	RegOff = D1_REG_OFFSET;
    else
	RegOff = D2_REG_OFFSET;

    /* FBSet */
    RHDRegWrite(Crtc, RegOff + D1GRPH_CONTROL, FBPriv->StoreGrphControl);
    RHDRegWrite(Crtc, RegOff + D1GRPH_X_START, FBPriv->StoreGrphXStart);
    RHDRegWrite(Crtc, RegOff + D1GRPH_Y_START, FBPriv->StoreGrphYStart);
    RHDRegWrite(Crtc, RegOff + D1GRPH_X_END, FBPriv->StoreGrphXEnd);
    RHDRegWrite(Crtc, RegOff + D1GRPH_Y_END, FBPriv->StoreGrphYEnd);
    if (RHDPTRI(Crtc)->ChipSet >= RHD_R600)
	RHDRegWrite(Crtc, RegOff + D1GRPH_SWAP_CNTL, FBPriv->StoreGrphSwap);

    /* disable read requests */
    RHDRegMask(Crtc, RegOff + D1CRTC_CONTROL, 0x01000000, 0x01000000);
    RHDRegMask(Crtc, RegOff + D1GRPH_ENABLE, 0, 0x00000001);
    usleep (10);

    RHDRegWrite(Crtc, RegOff + D1GRPH_PRIMARY_SURFACE_ADDRESS,
		FBPriv->StoreGrphPrimarySurfaceAddress);
    usleep(10);

    RHDRegWrite(Crtc, RegOff + D1GRPH_ENABLE, FBPriv->StoreGrphEnable);

    RHDRegWrite(Crtc, RegOff + D1GRPH_SURFACE_OFFSET_X,
		FBPriv->StoreGrphSurfaceOffsetX);
    RHDRegWrite(Crtc, RegOff + D1GRPH_SURFACE_OFFSET_Y,
		FBPriv->StoreGrphSurfaceOffsetY);

    RHDRegWrite(Crtc, RegOff + D1GRPH_PITCH, FBPriv->StoreGrphPitch);
    RHDRegWrite(Crtc, RegOff + D1MODE_DESKTOP_HEIGHT, FBPriv->StoreModeDesktopHeight);
}

/*
 *
 */
static void
DxFBDestroy(struct rhdCrtc *Crtc)
{
    if (Crtc->FBPriv)
	xfree(Crtc->FBPriv);
    Crtc->FBPriv = NULL;
}

/*
 *
 */
static ModeStatus
DxModeValid(struct rhdCrtc *Crtc, DisplayModePtr Mode)
{
  CARD32 tmp;

    RHDDebug(Crtc->scrnIndex, "%s: %s\n", __func__, Crtc->Name);

    /* Work around HW bug: need at least 2 lines of front porch
       for interlaced mode */
    if ((Mode->Flags & V_INTERLACE)
	&& (Mode->CrtcVSyncStart < (Mode->CrtcVDisplay + 2))) {
	Mode->CrtcVSyncStart = Mode->CrtcVDisplay + 2;
	Mode->CrtcVAdjusted = TRUE;
    }

    /* D1CRTC_H_TOTAL - 1 : 13bits */
  if (Mode->CrtcHTotal > 0x2000)
    return MODE_BAD_HVALUE;

  tmp = Mode->CrtcHTotal + Mode->CrtcHBlankStart - Mode->CrtcHSyncStart;
    /* D1CRTC_H_BLANK_START: 13bits */
  if (tmp >= 0x2000)
    return MODE_BAD_HVALUE;

  tmp = Mode->CrtcHBlankEnd - Mode->CrtcHSyncStart;
    /* D1CRTC_H_BLANK_END: 13bits */
  if (tmp >= 0x2000)
    return MODE_BAD_HVALUE;

  tmp = Mode->CrtcHSyncEnd - Mode->CrtcHSyncStart;
    /* D1CRTC_H_SYNC_A_END: 13bits */
  if (tmp >= 0x2000)
    return MODE_HSYNC_WIDE;

    /* D1CRTC_V_TOTAL - 1 : 13bits */
  if (Mode->CrtcVTotal > 0x2000)
    return MODE_BAD_VVALUE;

  tmp = Mode->CrtcVTotal + Mode->CrtcVBlankStart - Mode->CrtcVSyncStart;
    /* D1CRTC_V_BLANK_START: 13bits */
  if (tmp >= 0x2000)
    return MODE_BAD_VVALUE;

  tmp = Mode->CrtcVBlankEnd - Mode->CrtcVSyncStart;
    /* D1CRTC_V_BLANK_END: 13bits */
  if (tmp >= 0x2000)
    return MODE_BAD_VVALUE;

  tmp = Mode->CrtcVSyncEnd - Mode->CrtcVSyncStart;
    /* D1CRTC_V_SYNC_A_END: 13bits */
  if (tmp >= 0x2000)
    return MODE_VSYNC_WIDE;

  return MODE_OK;
}

/*
 *
 */
static void
DxModeSet(struct rhdCrtc *Crtc, DisplayModePtr Mode)
{
    RHDPtr rhdPtr = RHDPTRI(Crtc);
  CARD16 BlankStart, BlankEnd;
  CARD16 RegOff;

    RHDDebug(Crtc->scrnIndex, "FUNCTION: %s: %s\n", __func__, Crtc->Name);

    if (rhdPtr->verbosity > 6) {
	xf86DrvMsg(Crtc->scrnIndex, X_INFO, "%s: Setting ",__func__);
	RHDPrintModeline(Mode);
    }

  if (Crtc->Id == RHD_CRTC_1)
    RegOff = D1_REG_OFFSET;
  else
    RegOff = D2_REG_OFFSET;

    /* enable read requests */
  RHDRegMask(Crtc, RegOff + D1CRTC_CONTROL, 0, 0x01000000);

    /* Horizontal */
  RHDRegWrite(Crtc, RegOff + D1CRTC_H_TOTAL, Mode->CrtcHTotal - 1);

  BlankStart = Mode->CrtcHTotal + Mode->CrtcHBlankStart - Mode->CrtcHSyncStart;
  BlankEnd = Mode->CrtcHBlankEnd - Mode->CrtcHSyncStart;
  RHDRegWrite(Crtc, RegOff + D1CRTC_H_BLANK_START_END,
  BlankStart | (BlankEnd << 16));

  RHDRegWrite(Crtc, RegOff + D1CRTC_H_SYNC_A,
             (Mode->CrtcHSyncEnd - Mode->CrtcHSyncStart) << 16);
  RHDRegWrite(Crtc, RegOff + D1CRTC_H_SYNC_A_CNTL, Mode->Flags & V_NHSYNC);

    /* Vertical */
  RHDRegWrite(Crtc, RegOff + D1CRTC_V_TOTAL, Mode->CrtcVTotal - 1);

  BlankStart = Mode->CrtcVTotal + Mode->CrtcVBlankStart - Mode->CrtcVSyncStart;
  BlankEnd = Mode->CrtcVBlankEnd - Mode->CrtcVSyncStart;
  RHDRegWrite(Crtc, RegOff + D1CRTC_V_BLANK_START_END,
              BlankStart | (BlankEnd << 16));

    /* set interlaced */
    if (Mode->Flags & V_INTERLACE) {
	RHDRegWrite(Crtc, RegOff + D1CRTC_INTERLACE_CONTROL, 0x1);
	RHDRegWrite(Crtc, RegOff + D1MODE_DATA_FORMAT, 0x1);
    } else {
	RHDRegWrite(Crtc, RegOff + D1CRTC_INTERLACE_CONTROL, 0x0);
	RHDRegWrite(Crtc, RegOff + D1MODE_DATA_FORMAT, 0x0);
    }

  RHDRegWrite(Crtc, RegOff + D1CRTC_V_SYNC_A,
             (Mode->CrtcVSyncEnd - Mode->CrtcVSyncStart) << 16);
  RHDRegWrite(Crtc, RegOff + D1CRTC_V_SYNC_A_CNTL, Mode->Flags & V_NVSYNC);

    /* set D1CRTC_HORZ_COUNT_BY2_EN to 0; should only be set to 1 on 30bpp DVI modes */
    RHDRegMask(Crtc, RegOff + D1CRTC_COUNT_CONTROL, 0x0, 0x1);

  Crtc->CurrentMode = Mode;
}

/*
 *
 */
static void
DxModeSave(struct rhdCrtc *Crtc)
{
    struct rhdCrtcModePrivate *ModePriv;
    CARD32 RegOff;

    if (!Crtc->ModePriv)
	ModePriv = xnfcalloc(1, sizeof(struct rhdCrtcModePrivate));
    else
	ModePriv = Crtc->ModePriv;

    if (Crtc->Id == RHD_CRTC_1)
	RegOff = D1_REG_OFFSET;
    else
	RegOff = D2_REG_OFFSET;

    ModePriv->StoreCrtcControl = RHDRegRead(Crtc, RegOff + D1CRTC_CONTROL);

    ModePriv->StoreCrtcHTotal = RHDRegRead(Crtc, RegOff + D1CRTC_H_TOTAL);
    ModePriv->StoreCrtcHBlankStartEnd =
	RHDRegRead(Crtc, RegOff + D1CRTC_H_BLANK_START_END);
    ModePriv->StoreCrtcHSyncA = RHDRegRead(Crtc, RegOff + D1CRTC_H_SYNC_A);
    ModePriv->StoreCrtcHSyncACntl = RHDRegRead(Crtc, RegOff + D1CRTC_H_SYNC_A_CNTL);
    ModePriv->StoreCrtcHSyncB = RHDRegRead(Crtc, RegOff + D1CRTC_H_SYNC_B);
    ModePriv->StoreCrtcHSyncBCntl = RHDRegRead(Crtc, RegOff + D1CRTC_H_SYNC_B_CNTL);

    ModePriv->StoreModeDataFormat = RHDRegRead(Crtc, RegOff + D1MODE_DATA_FORMAT);
    ModePriv->StoreCrtcInterlaceControl = RHDRegRead(Crtc, RegOff + D1CRTC_INTERLACE_CONTROL);

    ModePriv->StoreCrtcVTotal = RHDRegRead(Crtc, RegOff + D1CRTC_V_TOTAL);
    ModePriv->StoreCrtcVBlankStartEnd =
	RHDRegRead(Crtc, RegOff + D1CRTC_V_BLANK_START_END);
    ModePriv->StoreCrtcVSyncA = RHDRegRead(Crtc, RegOff + D1CRTC_V_SYNC_A);
    ModePriv->StoreCrtcVSyncACntl = RHDRegRead(Crtc, RegOff + D1CRTC_V_SYNC_A_CNTL);
    ModePriv->StoreCrtcVSyncB = RHDRegRead(Crtc, RegOff + D1CRTC_V_SYNC_B);
    ModePriv->StoreCrtcVSyncBCntl = RHDRegRead(Crtc, RegOff + D1CRTC_V_SYNC_B_CNTL);

    ModePriv->StoreCrtcBlackColor = RHDRegRead(Crtc, RegOff + D1CRTC_BLACK_COLOR);
    ModePriv->StoreCrtcBlankControl = RHDRegRead(Crtc, RegOff + D1CRTC_BLANK_CONTROL);

    ModePriv->StoreCrtcCountControl = RHDRegRead(Crtc, RegOff + D1CRTC_COUNT_CONTROL);
    RHDDebug(Crtc->scrnIndex, "Saved CrtcCountControl[%i] = 0x%8.8x\n",
	     Crtc->Id,ModePriv->StoreCrtcCountControl);

    Crtc->ModePriv = ModePriv;
}

/*
 *
 */
static void
DxModeRestore(struct rhdCrtc *Crtc)
{
    struct rhdCrtcModePrivate *ModePriv = Crtc->ModePriv;
    CARD32 RegOff;

    if (!ModePriv) {
	xf86DrvMsg(Crtc->scrnIndex, X_ERROR, "%s: no registers stored!\n",
		   __func__);
	return;
    }

    if (Crtc->Id == RHD_CRTC_1)
	RegOff = D1_REG_OFFSET;
    else
	RegOff = D2_REG_OFFSET;

    /* ModeSet */
    RHDRegWrite(Crtc, RegOff + D1CRTC_CONTROL, ModePriv->StoreCrtcControl);

    RHDRegWrite(Crtc, RegOff + D1CRTC_H_TOTAL, ModePriv->StoreCrtcHTotal);
    RHDRegWrite(Crtc, RegOff + D1CRTC_H_BLANK_START_END,
		ModePriv->StoreCrtcHBlankStartEnd);
    RHDRegWrite(Crtc, RegOff + D1CRTC_H_SYNC_A, ModePriv->StoreCrtcHSyncA);
    RHDRegWrite(Crtc, RegOff + D1CRTC_H_SYNC_A_CNTL, ModePriv->StoreCrtcHSyncACntl);
    RHDRegWrite(Crtc, RegOff + D1CRTC_H_SYNC_B, ModePriv->StoreCrtcHSyncB);
    RHDRegWrite(Crtc, RegOff + D1CRTC_H_SYNC_B_CNTL, ModePriv->StoreCrtcHSyncBCntl);

    RHDRegWrite(Crtc, RegOff + D1MODE_DATA_FORMAT, ModePriv->StoreModeDataFormat);
    RHDRegWrite(Crtc, RegOff + D1CRTC_INTERLACE_CONTROL, ModePriv->StoreCrtcInterlaceControl);

    RHDRegWrite(Crtc, RegOff + D1CRTC_V_TOTAL, ModePriv->StoreCrtcVTotal);
    RHDRegWrite(Crtc, RegOff + D1CRTC_V_BLANK_START_END,
		ModePriv->StoreCrtcVBlankStartEnd);
    RHDRegWrite(Crtc, RegOff + D1CRTC_V_SYNC_A, ModePriv->StoreCrtcVSyncA);
    RHDRegWrite(Crtc, RegOff + D1CRTC_V_SYNC_A_CNTL, ModePriv->StoreCrtcVSyncACntl);
    RHDRegWrite(Crtc, RegOff + D1CRTC_V_SYNC_B, ModePriv->StoreCrtcVSyncB);
    RHDRegWrite(Crtc, RegOff + D1CRTC_V_SYNC_B_CNTL, ModePriv->StoreCrtcVSyncBCntl);

    RHDRegWrite(Crtc, RegOff + D1CRTC_COUNT_CONTROL, ModePriv->StoreCrtcCountControl);

    /* Blank */
    RHDRegWrite(Crtc, RegOff + D1CRTC_BLACK_COLOR, ModePriv->StoreCrtcBlackColor);
    RHDRegWrite(Crtc, RegOff + D1CRTC_BLANK_CONTROL, ModePriv->StoreCrtcBlankControl);

    /* When VGA is enabled, it imposes its timing on us, so our CRTC SYNC
     * timing can be set to 0. This doesn't always restore properly...
     * Workaround is to set a valid sync length for a bit so VGA can
     * latch in. */
    if (!ModePriv->StoreCrtcVSyncA && (ModePriv->StoreCrtcControl & 0x00000001)) {
	RHDRegWrite(Crtc, RegOff + D1CRTC_V_SYNC_A, 0x00040000);
	usleep(300000); /* seems a reliable timeout here */
	RHDRegWrite(Crtc, RegOff + D1CRTC_V_SYNC_A, ModePriv->StoreCrtcVSyncA);
    }
}

/*
 *
 */
static void
DxModeDestroy(struct rhdCrtc *Crtc)
{
    RHDFUNC(Crtc);

    if (Crtc->ModePriv)
	xfree(Crtc->ModePriv);
    Crtc->ModePriv = NULL;
}

/*
 *
 */
struct rhdScalerOverscan
rhdCalculateOverscan(DisplayModePtr Mode, DisplayModePtr ScaledToMode, enum rhdCrtcScaleType Type)
{
    struct rhdScalerOverscan Overscan;
    int tmp;

    Overscan.OverscanTop = Overscan.OverscanBottom = Overscan.OverscanLeft = Overscan.OverscanRight = 0;
    Overscan.Type = Type;

    if (ScaledToMode) {
	Overscan.OverscanTop = ScaledToMode->CrtcVDisplay - Mode->CrtcVDisplay;
	Overscan.OverscanLeft = ScaledToMode->CrtcHDisplay - Mode->CrtcHDisplay;

	if (!Overscan.OverscanTop && !Overscan.OverscanLeft)
	    Overscan.Type = RHD_CRTC_SCALE_TYPE_NONE;

	/* handle down scaling */
	if (Overscan.OverscanTop < 0) {
	    Overscan.Type = RHD_CRTC_SCALE_TYPE_SCALE;
	    Overscan.OverscanTop = 0;
	}
	if (Overscan.OverscanLeft < 0) {
	    Overscan.Type = RHD_CRTC_SCALE_TYPE_SCALE;
	    Overscan.OverscanLeft = 0;
	}
    }

    switch (Type) {
	case RHD_CRTC_SCALE_TYPE_NONE:
	    break;

	case RHD_CRTC_SCALE_TYPE_CENTER:
	    tmp = Overscan.OverscanTop;
	    Overscan.OverscanTop >>= 1;
	    Overscan.OverscanBottom = tmp - Overscan.OverscanTop;
	    tmp = Overscan.OverscanLeft;
	    Overscan.OverscanLeft >>= 1;
	    Overscan.OverscanRight = tmp - Overscan.OverscanLeft;
	    break;

	case RHD_CRTC_SCALE_TYPE_SCALE:
	    Overscan.OverscanLeft = Overscan.OverscanRight = Overscan.OverscanTop = Overscan.OverscanBottom = 0;
	    break;
	case RHD_CRTC_SCALE_TYPE_SCALE_KEEP_ASPECT_RATIO:
	{
	    int p1, p2, tmp;
	    Overscan.OverscanLeft = Overscan.OverscanRight = Overscan.OverscanTop = Overscan.OverscanBottom = 0;
	    p1 = Mode->CrtcVDisplay * ScaledToMode->CrtcHDisplay;
	    p2 = ScaledToMode->CrtcVDisplay * Mode->CrtcHDisplay;
	    if (p1 == p2) {
		Overscan.Type = RHD_CRTC_SCALE_TYPE_SCALE;
	    } else if (p1 > p2) {
		tmp = (p2 / Mode->CrtcVDisplay);
		tmp = ScaledToMode->CrtcHDisplay - tmp;
		Overscan.OverscanLeft = tmp >> 1;
		Overscan.OverscanRight = tmp - Overscan.OverscanLeft;
		ErrorF("HScale %i %i\n", Overscan.OverscanLeft, Overscan.OverscanRight);
	    } else {
		tmp = (p1 / Mode->CrtcHDisplay);
		tmp = ScaledToMode->CrtcVDisplay - tmp;
		Overscan.OverscanTop = tmp >> 1;
		Overscan.OverscanBottom = tmp - Overscan.OverscanTop;
		ErrorF("VScale %i %i\n", Overscan.OverscanTop, Overscan.OverscanBottom);
	    }
	    break;
	}
    }

    return Overscan;
}

/*
 *
 */
static ModeStatus
DxScaleValid(struct rhdCrtc *Crtc, enum rhdCrtcScaleType Type,
	     DisplayModePtr Mode, DisplayModePtr ScaledToMode)
{
    struct rhdScalerOverscan Overscan;

    /* D1_MODE_VIEWPORT_WIDTH: 14bits */
    if (Mode->CrtcHDisplay >= 0x4000)
	return MODE_BAD_HVALUE;

    /* D1_MODE_VIEWPORT_HEIGHT: 14bits */
    if (Mode->CrtcVDisplay >= 0x4000)
	return MODE_BAD_VVALUE;

    Overscan = rhdCalculateOverscan(Mode, ScaledToMode, Type);

    if (Overscan.OverscanLeft >= 4096 || Overscan.OverscanRight >= 4096)
	return MODE_HBLANK_WIDE;

    if (Overscan.OverscanTop >= 4096 || Overscan.OverscanBottom >= 4096)
	return MODE_VBLANK_WIDE;

    if ((Type == RHD_CRTC_SCALE_TYPE_SCALE
	 || Type == RHD_CRTC_SCALE_TYPE_SCALE_KEEP_ASPECT_RATIO)
	&& (Mode->Flags & V_INTERLACE))
	return MODE_NO_INTERLACE;

    /* should we also fail of Type != Overscan.Type? */

    return MODE_OK;
}

/*
 *
 */
static void
DxScaleSet(struct rhdCrtc *Crtc, enum rhdCrtcScaleType Type,
	   DisplayModePtr Mode, DisplayModePtr ScaledToMode)
{
    RHDPtr rhdPtr = RHDPTRI(Crtc);
    CARD16 RegOff;
    struct rhdScalerOverscan Overscan;

    RHDDebug(Crtc->scrnIndex, "FUNCTION: %s: %s viewport: %ix%i\n", __func__, Crtc->Name,
	     Mode->CrtcHDisplay, Mode->CrtcVDisplay);

    if (Crtc->Id == RHD_CRTC_1)
	RegOff = D1_REG_OFFSET;
    else
	RegOff = D2_REG_OFFSET;

    Overscan = rhdCalculateOverscan(Mode, ScaledToMode, Type);
    Type = Overscan.Type;

    RHDDebug(Crtc->scrnIndex, "FUNCTION: %s: %s viewport: %ix%i - OverScan: T: %i B: %i R: %i L: %i\n",
	     __func__, Crtc->Name, Mode->CrtcHDisplay, Mode->CrtcVDisplay,
	     Overscan.OverscanTop, Overscan.OverscanBottom,
	     Overscan.OverscanLeft, Overscan.OverscanRight);

    /* D1Mode registers */
    RHDRegWrite(Crtc, RegOff + D1MODE_VIEWPORT_SIZE,
		Mode->CrtcVDisplay | (Mode->CrtcHDisplay << 16));
    RHDRegWrite(Crtc, RegOff + D1MODE_VIEWPORT_START, 0);

    RHDRegWrite(Crtc, RegOff + D1MODE_EXT_OVERSCAN_LEFT_RIGHT,
		(Overscan.OverscanLeft << 16) | Overscan.OverscanRight);
    RHDRegWrite(Crtc, RegOff + D1MODE_EXT_OVERSCAN_TOP_BOTTOM,
		(Overscan.OverscanTop << 16) | Overscan.OverscanBottom);

    switch (Type) {
	case RHD_CRTC_SCALE_TYPE_NONE:  /* No scaling whatsoever */
	    ErrorF("None\n");
	    RHDRegWrite(Crtc, RegOff + D1SCL_ENABLE, 0);
	    RHDRegWrite(Crtc, RegOff + D1SCL_TAP_CONTROL, 0);
	    RHDRegWrite(Crtc, RegOff + D1MODE_CENTER, 0);
	    break;
	case RHD_CRTC_SCALE_TYPE_CENTER: /* center of the actual mode */
	    ErrorF("Center\n");
	    RHDRegWrite(Crtc, RegOff + D1SCL_ENABLE, 0);
	    RHDRegWrite(Crtc, RegOff + D1SCL_TAP_CONTROL, 0);
	    RHDRegWrite(Crtc, RegOff + D1MODE_CENTER, 1);
	    break;
	case RHD_CRTC_SCALE_TYPE_SCALE_KEEP_ASPECT_RATIO: /* scaled to fullscreen */
	case RHD_CRTC_SCALE_TYPE_SCALE: /* scaled to fullscreen */
	    ErrorF("Full\n");
	    if (Type == RHD_CRTC_SCALE_TYPE_SCALE_KEEP_ASPECT_RATIO)
		RHDRegWrite(Crtc, RegOff + D1MODE_CENTER, 1);
	    else
		RHDRegWrite(Crtc, RegOff + D1MODE_CENTER, 0);

	    RHDRegWrite(Crtc, RegOff + D1SCL_UPDATE, 0);
	    RHDRegWrite(Crtc, RegOff + D1SCL_DITHER, 0);

	    RHDRegWrite(Crtc, RegOff + D1SCL_ENABLE, 1);
	    RHDRegWrite(Crtc, RegOff + D1SCL_HVSCALE, 0x00010001); /* both h/v */

	    RHDRegWrite(Crtc, RegOff + D1SCL_TAP_CONTROL, 0x00000101);

	    RHDRegWrite(Crtc, RegOff + D1SCL_HFILTER, 0x00030100);
	    RHDRegWrite(Crtc, RegOff + D1SCL_VFILTER, 0x00030100);

	    RHDRegWrite(Crtc, RegOff + D1SCL_DITHER, 0x00001010);
	    break;
    }
    RHDTuneMCAccessForDisplay(rhdPtr, Crtc->Id, Mode,
			      ScaledToMode ? ScaledToMode : Mode);
}

/*
 *
 */
static void
DxScaleSave(struct rhdCrtc *Crtc)
{
    struct rhdCrtcScalePrivate *ScalePriv;
    CARD32 RegOff;

    if (!Crtc->ScalePriv)
	ScalePriv =  xnfcalloc(1, sizeof(struct rhdCrtcScalePrivate));
    else
	ScalePriv = Crtc->ScalePriv;

    if (Crtc->Id == RHD_CRTC_1)
	RegOff = D1_REG_OFFSET;
    else
	RegOff = D2_REG_OFFSET;

    ScalePriv->StoreModeViewPortSize = RHDRegRead(Crtc, RegOff + D1MODE_VIEWPORT_SIZE);
    ScalePriv->StoreModeViewPortStart = RHDRegRead(Crtc, RegOff + D1MODE_VIEWPORT_START);
    ScalePriv->StoreModeOverScanH =
	RHDRegRead(Crtc, RegOff + D1MODE_EXT_OVERSCAN_LEFT_RIGHT);
    ScalePriv->StoreModeOverScanV =
	RHDRegRead(Crtc, RegOff + D1MODE_EXT_OVERSCAN_TOP_BOTTOM);

    ScalePriv->StoreScaleEnable = RHDRegRead(Crtc, RegOff + D1SCL_ENABLE);
    ScalePriv->StoreScaleTapControl = RHDRegRead(Crtc, RegOff + D1SCL_TAP_CONTROL);
    ScalePriv->StoreModeCenter = RHDRegRead(Crtc, RegOff + D1MODE_CENTER);
    ScalePriv->StoreScaleHV = RHDRegRead(Crtc, RegOff + D1SCL_HVSCALE);
    ScalePriv->StoreScaleHFilter = RHDRegRead(Crtc, RegOff + D1SCL_HFILTER);
    ScalePriv->StoreScaleVFilter = RHDRegRead(Crtc, RegOff + D1SCL_VFILTER);
    ScalePriv->StoreScaleDither = RHDRegRead(Crtc, RegOff + D1SCL_DITHER);

    Crtc->ScalePriv = ScalePriv;
}

/*
 *
 */
static void
DxScaleRestore(struct rhdCrtc *Crtc)
{
    struct rhdCrtcScalePrivate *ScalePriv = Crtc->ScalePriv;
    CARD32 RegOff;

    if (!ScalePriv) {
	xf86DrvMsg(Crtc->scrnIndex, X_ERROR, "%s: no registers stored!\n",
		   __func__);
	return;
    }

    if (Crtc->Id == RHD_CRTC_1)
	RegOff = D1_REG_OFFSET;
    else
	RegOff = D2_REG_OFFSET;

    /* ScaleSet */
    RHDRegWrite(Crtc, RegOff + D1MODE_VIEWPORT_SIZE, ScalePriv->StoreModeViewPortSize);

    /* ScaleSet/ViewPortStart */
    RHDRegWrite(Crtc, RegOff + D1MODE_VIEWPORT_START, ScalePriv->StoreModeViewPortStart);

    /* ScaleSet */
    RHDRegWrite(Crtc, RegOff + D1MODE_EXT_OVERSCAN_LEFT_RIGHT,
		ScalePriv->StoreModeOverScanH);
    RHDRegWrite(Crtc, RegOff + D1MODE_EXT_OVERSCAN_TOP_BOTTOM,
		ScalePriv->StoreModeOverScanV);

    RHDRegWrite(Crtc, RegOff + D1SCL_ENABLE, ScalePriv->StoreScaleEnable);
    RHDRegWrite(Crtc, RegOff + D1SCL_TAP_CONTROL, ScalePriv->StoreScaleTapControl);
    RHDRegWrite(Crtc, RegOff + D1MODE_CENTER, ScalePriv->StoreModeCenter);
    RHDRegWrite(Crtc, RegOff + D1SCL_HVSCALE, ScalePriv->StoreScaleHV);
    RHDRegWrite(Crtc, RegOff + D1SCL_HFILTER, ScalePriv->StoreScaleHFilter);
    RHDRegWrite(Crtc, RegOff + D1SCL_VFILTER, ScalePriv->StoreScaleVFilter);
    RHDRegWrite(Crtc, RegOff + D1SCL_DITHER, ScalePriv->StoreScaleDither);
}

/*
 *
 */
static void
DxScaleDestroy(struct rhdCrtc *Crtc)
{
    RHDFUNC(Crtc);

    if (Crtc->ScalePriv)
	xfree(Crtc->ScalePriv);
    Crtc->ScalePriv = NULL;
}

/*
 *
 */
static void
D1LUTSelect(struct rhdCrtc *Crtc, struct rhdLUT *LUT)
{
    RHDFUNC(Crtc);

    RHDRegWrite(Crtc, D1GRPH_LUT_SEL, LUT->Id & 1);
    Crtc->LUT = LUT;
}

/*
 *
 */
static void
D2LUTSelect(struct rhdCrtc *Crtc, struct rhdLUT *LUT)
{
    RHDFUNC(Crtc);

    RHDRegWrite(Crtc, D2GRPH_LUT_SEL, LUT->Id & 1);
    Crtc->LUT = LUT;
}

/*
 *
 */
static void
DxLUTSave(struct rhdCrtc *Crtc)
{
    struct rhdCrtcLUTPrivate *LUTPriv;
    CARD32 RegOff;

    if (!Crtc->LUTPriv)
	LUTPriv =  xnfcalloc(1, sizeof(struct rhdCrtcLUTPrivate));
    else
	LUTPriv = Crtc->LUTPriv;

    if (Crtc->Id == RHD_CRTC_1)
	RegOff = D1_REG_OFFSET;
    else
	RegOff = D2_REG_OFFSET;

    LUTPriv->StoreGrphLutSel = RHDRegRead(Crtc, RegOff + D1GRPH_LUT_SEL);

    Crtc->LUTPriv = LUTPriv;
}

/*
 *
 */
static void
DxLUTRestore(struct rhdCrtc *Crtc)
{
    struct rhdCrtcLUTPrivate *LUTPriv = Crtc->LUTPriv;
    CARD32 RegOff;

    if (!LUTPriv) {
	xf86DrvMsg(Crtc->scrnIndex, X_ERROR, "%s: no registers stored!\n",
		   __func__);
	return;
    }

    if (Crtc->Id == RHD_CRTC_1)
	RegOff = D1_REG_OFFSET;
    else
	RegOff = D2_REG_OFFSET;

    /* LUTSelect */
    RHDRegWrite(Crtc, RegOff + D1GRPH_LUT_SEL, LUTPriv->StoreGrphLutSel);
}

/*
 *
 */
static void
DxLUTDestroy(struct rhdCrtc *Crtc)
{
    RHDFUNC(Crtc);

    if (Crtc->LUTPriv)
	xfree(Crtc->LUTPriv);
    Crtc->LUTPriv = NULL;
}

/*
 *
 */
static void
D1ViewPortStart(struct rhdCrtc *Crtc, CARD16 X, CARD16 Y)
{
    RHDFUNC(Crtc);

    /* not as granular as docs make it seem to be.
     * if the lower two bits are set the line buffer might screw up, requiring
     * a power cycle. */
    X = (X + 0x02) & ~0x03;
    Y &= ~0x01;

    RHDRegMask(Crtc, D1SCL_UPDATE, 0x00010000, 0x0001000);
    RHDRegWrite(Crtc, D1MODE_VIEWPORT_START, (X << 16) | Y);
    RHDRegMask(Crtc, D1SCL_UPDATE, 0, 0x0001000);

    Crtc->X = X;
    Crtc->Y = Y;
}

/*
 *
 */
static void
D2ViewPortStart(struct rhdCrtc *Crtc, CARD16 X, CARD16 Y)
{
    RHDFUNC(Crtc);

    /* not as granular as docs make it seem to be. */
    X = (X + 0x02) & ~0x03;
    Y &= ~0x01;

    RHDRegMask(Crtc, D2SCL_UPDATE, 0x00010000, 0x0001000);
    RHDRegWrite(Crtc, D2MODE_VIEWPORT_START, (X << 16) | Y);
    RHDRegMask(Crtc, D2SCL_UPDATE, 0, 0x0001000);

    Crtc->X = X;
    Crtc->Y = Y;
}

#define CRTC_SYNC_WAIT 0x100000
/*
 *
 */
static void
D1CRTCDisable(struct rhdCrtc *Crtc)
{
    if (RHDRegRead(Crtc, D1CRTC_CONTROL) & 0x00000001) {
    CARD32 Control = RHDRegRead(Crtc, D1CRTC_CONTROL);
    int i;

	RHDRegMask(Crtc, D1CRTC_CONTROL, 0, 0x00000301);
	(void)RHDRegRead(Crtc, D1CRTC_CONTROL);

    for (i = 0; i < CRTC_SYNC_WAIT; i++)
	    if (!(RHDRegRead(Crtc, D1CRTC_CONTROL) & 0x00010000)) {
		RHDDebug(Crtc->scrnIndex, "%s: %d loops\n", __func__, i);
		RHDRegMask(Crtc, D1CRTC_CONTROL, Control, 0x00000300);
        return;
	    }
	xf86DrvMsg(Crtc->scrnIndex, X_ERROR,
		   "%s: Failed to Unsync %s\n", __func__, Crtc->Name);
	RHDRegMask(Crtc, D1CRTC_CONTROL, Control, 0x00000300);
    }
}

/*
 *
 */
static void
D2CRTCDisable(struct rhdCrtc *Crtc)
{
    if (RHDRegRead(Crtc, D2CRTC_CONTROL) & 0x00000001) {
    CARD32 Control = RHDRegRead(Crtc, D2CRTC_CONTROL);
    int i;

	RHDRegMask(Crtc, D2CRTC_CONTROL, 0, 0x00000301);
	(void)RHDRegRead(Crtc, D2CRTC_CONTROL);

    for (i = 0; i < CRTC_SYNC_WAIT; i++)
	    if (!(RHDRegRead(Crtc, D2CRTC_CONTROL) & 0x00010000)) {
		RHDDebug(Crtc->scrnIndex, "%s: %d loops\n", __func__, i);
		RHDRegMask(Crtc, D2CRTC_CONTROL, Control, 0x00000300);
        return;
	    }
	xf86DrvMsg(Crtc->scrnIndex, X_ERROR,
		   "%s: Failed to Unsync %s\n", __func__, Crtc->Name);
	RHDRegMask(Crtc, D2CRTC_CONTROL, Control, 0x00000300);
    }
}

/*
 *
 */
static void
D1Power(struct rhdCrtc *Crtc, int Power)
{
    RHDFUNC(Crtc);

    switch (Power) {
    case RHD_POWER_ON:
	RHDRegMask(Crtc, D1GRPH_ENABLE, 0x00000001, 0x00000001);
	usleep(2);
	RHDRegMask(Crtc, D1CRTC_CONTROL, 0, 0x01000000); /* enable read requests */
	RHDRegMask(Crtc, D1CRTC_CONTROL, 1, 1);
	return;
    case RHD_POWER_RESET:
	RHDRegMask(Crtc, D1CRTC_CONTROL, 0x01000000, 0x01000000); /* disable read requests */
	D1CRTCDisable(Crtc);
	return;
    case RHD_POWER_SHUTDOWN:
    default:
	RHDRegMask(Crtc, D1CRTC_CONTROL, 0x01000000, 0x01000000); /* disable read requests */
	D1CRTCDisable(Crtc);
	RHDRegMask(Crtc, D1GRPH_ENABLE, 0, 0x00000001);
	return;
    }
}

/*
 *
 */
static void
D2Power(struct rhdCrtc *Crtc, int Power)
{
    RHDFUNC(Crtc);

    switch (Power) {
    case RHD_POWER_ON:
	RHDRegMask(Crtc, D2GRPH_ENABLE, 0x00000001, 0x00000001);
	usleep(2);
	RHDRegMask(Crtc, D2CRTC_CONTROL, 0, 0x01000000); /* enable read requests */
	RHDRegMask(Crtc, D2CRTC_CONTROL, 1, 1);
	return;
    case RHD_POWER_RESET:
	RHDRegMask(Crtc, D2CRTC_CONTROL, 0x01000000, 0x01000000); /* disable read requests */
	D2CRTCDisable(Crtc);
	return;
    case RHD_POWER_SHUTDOWN:
    default:
	RHDRegMask(Crtc, D2CRTC_CONTROL, 0x01000000, 0x01000000); /* disable read requests */
	D2CRTCDisable(Crtc);
	RHDRegMask(Crtc, D2GRPH_ENABLE, 0, 0x00000001);
	return;
    }
}

/*
 * This is quite different from Power. Power disables and enables things,
 * this here makes the hw send out black, and can switch back and forth
 * immediately. Useful for covering up a framebuffer that is not filled
 * in yet.
 */
static void
D1Blank(struct rhdCrtc *Crtc, Bool Blank)
{
    RHDFUNC(Crtc);

    RHDRegWrite(Crtc, D1CRTC_BLACK_COLOR, 0);
    if (Blank)
	RHDRegMask(Crtc, D1CRTC_BLANK_CONTROL, 0x00000100, 0x00000100);
    else
	RHDRegMask(Crtc, D1CRTC_BLANK_CONTROL, 0, 0x00000100);
}

/*
 *
 */
static void
D2Blank(struct rhdCrtc *Crtc, Bool Blank)
{
    RHDFUNC(Crtc);

    RHDRegWrite(Crtc, D2CRTC_BLACK_COLOR, 0);
    if (Blank)
	RHDRegMask(Crtc, D2CRTC_BLANK_CONTROL, 0x00000100, 0x00000100);
    else
	RHDRegMask(Crtc, D2CRTC_BLANK_CONTROL, 0, 0x00000100);
}

/*
 *
 */
static void
DxFMTSet(struct rhdCrtc *Crtc, struct rhdFMTDither *FMTDither)
{
    CARD32 RegOff;
    CARD32 fmt_cntl = 0;

    RHDFUNC(Crtc);

    if (Crtc->Id == RHD_CRTC_1)
	RegOff = FMT1_REG_OFFSET;
    else
	RegOff = FMT2_REG_OFFSET;

    if (FMTDither) {

	/* set dither depth to 18/24 */
	fmt_cntl = FMTDither->LVDS24Bit
	    ? (RV62_FMT_SPATIAL_DITHER_DEPTH | RV62_FMT_TEMPORAL_DITHER_DEPTH)
	    : 0;
	RHDRegMask(Crtc, RegOff + RV620_FMT1_BIT_DEPTH_CONTROL, fmt_cntl,
	       RV62_FMT_SPATIAL_DITHER_DEPTH | RV62_FMT_TEMPORAL_DITHER_DEPTH);

	/* set temporal dither */
	if (FMTDither->LVDSTemporalDither) {
	    fmt_cntl = FMTDither->LVDSGreyLevel ? RV62_FMT_TEMPORAL_LEVEL : 0x0;
	    /* grey level */
	    RHDRegMask(Crtc, RegOff + RV620_FMT1_BIT_DEPTH_CONTROL,
		       fmt_cntl, RV62_FMT_TEMPORAL_LEVEL);
	    /* turn on temporal dither and reset */
	    RHDRegMask(Crtc, RegOff + RV620_FMT1_BIT_DEPTH_CONTROL,
		       RV62_FMT_TEMPORAL_DITHER_EN | RV62_FMT_TEMPORAL_DITHER_RESET,
		       RV62_FMT_TEMPORAL_DITHER_EN | RV62_FMT_TEMPORAL_DITHER_RESET);
	    usleep(20);
	    /* turn off reset */
	    RHDRegMask(Crtc, RegOff + RV620_FMT1_BIT_DEPTH_CONTROL, 0x0,
		       RV62_FMT_TEMPORAL_DITHER_RESET);
	}
	/* spatial dither */
	RHDRegMask(Crtc, RegOff + RV620_FMT1_BIT_DEPTH_CONTROL,
		   FMTDither->LVDSSpatialDither ? RV62_FMT_SPATIAL_DITHER_EN : 0,
		   RV62_FMT_SPATIAL_DITHER_EN);
    } else
	RHDRegWrite(Crtc, RegOff + RV620_FMT1_BIT_DEPTH_CONTROL, 0);

    /* 4:4:4 encoding */
    RHDRegMask(Crtc,  RegOff + RV620_FMT1_CONTROL, 0, RV62_FMT_PIXEL_ENCODING);
    /* disable color clamping */
    RHDRegWrite(Crtc, RegOff + RV620_FMT1_CLAMP_CNTL, 0);
}

/*
 *
 */
static void
DxFMTSave(struct rhdCrtc *Crtc)
{
    struct rhdCrtcFMTPrivate *FMTPrivate;
    CARD32 RegOff;

    RHDFUNC(Crtc);

    if (!Crtc->FMTPriv)
	FMTPrivate = xnfcalloc(sizeof (struct rhdCrtcFMTPrivate),1);
    else
	FMTPrivate = Crtc->FMTPriv;

    if (Crtc->Id == RHD_CRTC_1)
	RegOff = FMT1_REG_OFFSET;
    else
	RegOff = FMT2_REG_OFFSET;

    FMTPrivate->StoreControl         = RHDRegRead(Crtc, RegOff + RV620_FMT1_CONTROL);
    FMTPrivate->StoreBitDepthControl = RHDRegRead(Crtc, RegOff + RV620_FMT1_BIT_DEPTH_CONTROL);
    FMTPrivate->StoreClampCntl       = RHDRegRead(Crtc, RegOff + RV620_FMT1_CLAMP_CNTL);

    Crtc->FMTPriv = FMTPrivate;
}

/*
 *
 */
static void
DxFMTRestore(struct rhdCrtc *Crtc)
{
    struct rhdCrtcFMTPrivate *FMTPrivate = Crtc->FMTPriv;
    CARD32 RegOff;

    RHDFUNC(Crtc);

    if (!FMTPrivate)
	return;

    if (Crtc->Id == RHD_CRTC_1)
	RegOff = FMT1_REG_OFFSET;
    else
	RegOff = FMT2_REG_OFFSET;

    RHDRegWrite(Crtc, RegOff + RV620_FMT1_CONTROL, FMTPrivate->StoreControl);
    RHDRegWrite(Crtc, RegOff + RV620_FMT1_BIT_DEPTH_CONTROL, FMTPrivate->StoreBitDepthControl);
    RHDRegWrite(Crtc, RegOff + RV620_FMT1_CLAMP_CNTL, FMTPrivate->StoreClampCntl);
}

/*
 *
 */
static void
DxFMTDestroy(struct rhdCrtc *Crtc)
{
    RHDFUNC(Crtc);

    if (Crtc->FMTPriv)
	xfree(Crtc->FMTPriv);
    Crtc->FMTPriv = NULL;
}

/*
 *
 */
static enum rhdCrtcScaleType
rhdInitScaleType(RHDPtr rhdPtr)
{
    RHDFUNC(rhdPtr);
/*
    if (rhdPtr->scaleTypeOpt.set) {
	if (!strcasecmp(rhdPtr->scaleTypeOpt.val.string, "none"))
	    return RHD_CRTC_SCALE_TYPE_NONE;
	else if (!strcasecmp(rhdPtr->scaleTypeOpt.val.string, "center"))
	    return RHD_CRTC_SCALE_TYPE_CENTER;
	else if (!strcasecmp(rhdPtr->scaleTypeOpt.val.string, "scale"))
	    return RHD_CRTC_SCALE_TYPE_SCALE;
	else if (!strcasecmp(rhdPtr->scaleTypeOpt.val.string, "scale_keep_aspect_ratio"))
	    return RHD_CRTC_SCALE_TYPE_SCALE_KEEP_ASPECT_RATIO;
	else if (!strcasecmp(rhdPtr->scaleTypeOpt.val.string, "default"))
	    return RHD_CRTC_SCALE_TYPE_DEFAULT;
	else {
	    xf86DrvMsgVerb(rhdPtr->scrnIndex, X_ERROR, 0,
			   "Unknown scale type: %s\n", rhdPtr->scaleTypeOpt.val.string);
	    return RHD_CRTC_SCALE_TYPE_DEFAULT;
	}
    } else  */
  return RHD_CRTC_SCALE_TYPE_SCALE;
}

/*
 *
 */
Bool
RHDCrtcsInit(RHDPtr rhdPtr)
{
    struct rhdCrtc *Crtc;
    enum rhdCrtcScaleType ScaleType;
    Bool useAtom;

    RHDFUNC(rhdPtr);

    useAtom = RHDUseAtom(rhdPtr, NULL, atomUsageCrtc);

    ScaleType = rhdInitScaleType(rhdPtr);

    Crtc = xnfcalloc(sizeof(struct rhdCrtc), 1);
    Crtc->scrnIndex = rhdPtr->scrnIndex;
    Crtc->Name = "CRTC 1";
    Crtc->Id = RHD_CRTC_1;

    Crtc->ScaleType = ScaleType;

    if (rhdPtr->ChipSet >= RHD_RV620) {
	Crtc->FMTDestroy = DxFMTDestroy;
	Crtc->FMTSave = DxFMTSave;
	Crtc->FMTRestore = DxFMTRestore;
	Crtc->FMTModeSet = DxFMTSet;
    }
    Crtc->FMTPriv = NULL;

    Crtc->FBValid   = DxFBValid;
    Crtc->FBSet     = DxFBSet;
    Crtc->FBSave = DxFBSave;
    Crtc->FBRestore = DxFBRestore;
    Crtc->FBDestroy = DxFBDestroy;

    Crtc->ModeValid = DxModeValid;
    Crtc->ModeSet   = DxModeSet;
    Crtc->ModeSave = DxModeSave;
    Crtc->ModeRestore = DxModeRestore;
    Crtc->ModeDestroy = DxModeDestroy;
    Crtc->ModePriv = NULL;

    Crtc->ScaleValid = DxScaleValid;
    Crtc->ScaleSet = DxScaleSet;
    Crtc->ScaleSave = DxScaleSave;
    Crtc->ScaleRestore = DxScaleRestore;
    Crtc->ScaleDestroy = DxScaleDestroy;
    Crtc->ScalePriv = NULL;

    Crtc->LUTSelect = D1LUTSelect;
    Crtc->LUTSave = DxLUTSave;
    Crtc->LUTRestore = DxLUTRestore;
    Crtc->LUTDestroy = DxLUTDestroy;
    Crtc->LUTPriv = NULL;

    Crtc->FrameSet  = D1ViewPortStart;

    Crtc->Power     = D1Power;
    Crtc->Blank     = D1Blank;

    rhdPtr->Crtc[0] = Crtc;

    Crtc = xnfcalloc(sizeof(struct rhdCrtc), 1);
    Crtc->scrnIndex = rhdPtr->scrnIndex;
    Crtc->Name      = "CRTC 2";
    Crtc->Id        = RHD_CRTC_2;

    Crtc->ScaleType = ScaleType;

    if (rhdPtr->ChipSet >= RHD_RV620) {
	Crtc->FMTDestroy = DxFMTDestroy;
	Crtc->FMTSave = DxFMTSave;
	Crtc->FMTRestore = DxFMTRestore;
	Crtc->FMTModeSet = DxFMTSet;
    }
    Crtc->FMTPriv = NULL;

    Crtc->FBValid   = DxFBValid;
    Crtc->FBSet     = DxFBSet;
    Crtc->FBSave = DxFBSave;
    Crtc->FBRestore = DxFBRestore;
    Crtc->FBDestroy = DxFBDestroy;

    Crtc->ModeValid = DxModeValid;
    Crtc->ModeSet   = DxModeSet;
    Crtc->ModeSave = DxModeSave;
    Crtc->ModeRestore = DxModeRestore;
    Crtc->ModeDestroy = DxModeDestroy;
    Crtc->ModePriv = NULL;

    Crtc->ScaleValid = DxScaleValid;
    Crtc->ScaleSet = DxScaleSet;
    Crtc->ScaleSave = DxScaleSave;
    Crtc->ScaleRestore = DxScaleRestore;
    Crtc->ScaleDestroy = DxScaleDestroy;
    Crtc->ScalePriv = NULL;

    Crtc->LUTSelect = D2LUTSelect;
    Crtc->LUTSave = DxLUTSave;
    Crtc->LUTRestore = DxLUTRestore;
    Crtc->LUTDestroy = DxLUTDestroy;
    Crtc->LUTPriv = NULL;

    Crtc->FrameSet  = D2ViewPortStart;

    Crtc->Power     = D2Power;
    Crtc->Blank     = D2Blank;

    rhdPtr->Crtc[1] = Crtc;

    return !useAtom;
}

/*
 *
 */
void
RHDCrtcsDestroy(RHDPtr rhdPtr)
{
    struct rhdCrtc *Crtc;
    int i;

    RHDFUNC(rhdPtr);

    for (i = 0; i < 2; i++) {
	Crtc = rhdPtr->Crtc[i];
    if (Crtc) {
	    if (Crtc->FMTDestroy)
		Crtc->FMTDestroy(Crtc);

	    if (Crtc->LUTDestroy)
		Crtc->LUTDestroy(Crtc);

	    if (Crtc->FBDestroy)
		Crtc->FBDestroy(Crtc);

	    if (Crtc->ScaleDestroy)
		Crtc->ScaleDestroy(Crtc);

	    if (Crtc->ModeDestroy)
		Crtc->ModeDestroy(Crtc);

	    xfree(Crtc);
	    rhdPtr->Crtc[i] = NULL;
	}
    }
}


/*
 *
 */
void
RHDCrtcSave(struct rhdCrtc *Crtc)
{
    RHDDebug(Crtc->scrnIndex, "%s: %s\n", __func__, Crtc->Name);

    if (Crtc->FMTSave)
	Crtc->FMTSave(Crtc);

    if (Crtc->FBSave)
	Crtc->FBSave(Crtc);

    if (Crtc->LUTSave)
	Crtc->LUTSave(Crtc);

    if (Crtc->ScaleSave)
	Crtc->ScaleSave(Crtc);

    if (Crtc->ModeSave)
	Crtc->ModeSave(Crtc);
}

/*
 *
 */
void
RHDCrtcRestore(struct rhdCrtc *Crtc)
{

    RHDDebug(Crtc->scrnIndex, "%s: %s\n", __func__, Crtc->Name);

    if (Crtc->FMTRestore)
	Crtc->FMTRestore(Crtc);

    if (Crtc->FBRestore)
	Crtc->FBRestore(Crtc);

    if (Crtc->LUTRestore)
	Crtc->LUTRestore(Crtc);

    if (Crtc->ScaleRestore)
	Crtc->ScaleRestore(Crtc);

    if (Crtc->ModeRestore)
	Crtc->ModeRestore(Crtc);
}
