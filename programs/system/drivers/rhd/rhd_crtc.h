/*
 * Copyright 2004-2008  Luc Verhaegen <lverhaegen@novell.com>
 * Copyright 2007, 2008 Matthias Hopf <mhopf@novell.com>
 * Copyright 2007, 2008 Egbert Eich   <eich@novell.com>
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

#ifndef _RHD_CRTC_H
# define _RHD_CRTC_H

struct rhdFMTDither {
    Bool LVDS24Bit;
    Bool LVDSSpatialDither;
    Bool LVDSTemporalDither;
    int LVDSGreyLevel;
};

enum rhdCrtcScaleType {
    RHD_CRTC_SCALE_TYPE_NONE,                     /* top left */
    RHD_CRTC_SCALE_TYPE_CENTER,                   /* center of the actual mode */
    RHD_CRTC_SCALE_TYPE_SCALE,                    /* scaled to fullscreen */
    RHD_CRTC_SCALE_TYPE_SCALE_KEEP_ASPECT_RATIO   /* scaled to fullscreen */
};

#define RHD_CRTC_SCALE_TYPE_DEFAULT RHD_CRTC_SCALE_TYPE_SCALE_KEEP_ASPECT_RATIO


struct rhdCrtc {
    int scrnIndex;

    char *Name;
#define RHD_CRTC_1 0
#define RHD_CRTC_2 1
    int Id; /* for others to hook onto */

    Bool Active;

    int Offset; /* Current offset */
    int bpp;
    int Pitch;
    int Width;
    int Height;
    int X, Y; /* Current frame */
    int MinX, MinY, MaxX, MaxY; /* Panning Area: Max != 0 if used */
    enum rhdCrtcScaleType ScaleType;
    struct rhdPLL *PLL; /* Currently attached PLL: move to private */
    struct rhdLUT *LUT; /* Currently attached LUT: move to private */
    struct rhdCursor *Cursor; /* Fixed to the MODE engine */

    DisplayModePtr CurrentMode;
    DisplayModePtr Modes; /* Validated ones: Cycle through these */

    DisplayModePtr ScaledToMode; /* usually a fixed mode from one of the monitors */

    struct rhdCrtcFMTPrivate *FMTPriv;  /* each CRTC subsystem may define this independently */
    void (*FMTModeSet)(struct rhdCrtc *Crtc, struct rhdFMTDither *FMTDither);
    void (*FMTSave)(struct rhdCrtc *Crtc);
    void (*FMTRestore)(struct rhdCrtc *Crtc);
    void (*FMTDestroy) (struct rhdCrtc *Crtc);

    struct rhdCrtcFBPrivate *FBPriv;  /* each CRTC subsystem may define this independently */
    ModeStatus (*FBValid) (struct rhdCrtc *Crtc, CARD16 Width, CARD16 Height,
			   int bpp, CARD32 Offset, CARD32 Size, CARD32 *pPitch);
    void (*FBSet) (struct rhdCrtc *Crtc, CARD16 Pitch, CARD16 Width,
		   CARD16 Height, int bpp, CARD32 Offset);
    void (*FBSave) (struct rhdCrtc *Crtc);
    void (*FBRestore) (struct rhdCrtc *Crtc);
    void (*FBDestroy) (struct rhdCrtc *Crtc);

    struct rhdCrtcModePrivate *ModePriv;  /* each CRTC subsystem may define this independently */
    ModeStatus (*ModeValid) (struct rhdCrtc *Crtc, DisplayModePtr Mode);
    void (*ModeSet) (struct rhdCrtc *Crtc, DisplayModePtr Mode);
    void (*ModeSave) (struct rhdCrtc *Crtc);
    void (*ModeRestore) (struct rhdCrtc *Crtc);
    void (*ModeDestroy) (struct rhdCrtc *Crtc);

    struct rhdCrtcScalePrivate *ScalePriv;  /* each CRTC subsystem may define this independently */
    ModeStatus (*ScaleValid) (struct rhdCrtc *Crtc, enum rhdCrtcScaleType Type, DisplayModePtr Mode, DisplayModePtr ScaledToMode);
    void (*ScaleSet) (struct rhdCrtc *Crtc, enum rhdCrtcScaleType Type, DisplayModePtr Mode, DisplayModePtr ScaledToMode);
    void (*ScaleSave) (struct rhdCrtc *Crtc);
    void (*ScaleRestore) (struct rhdCrtc *Crtc);
    void (*ScaleDestroy) (struct rhdCrtc *Crtc);

    void (*FrameSet) (struct rhdCrtc *Crtc, CARD16 X, CARD16 Y);

    /* callback for pll setting lives here */
    /* callback for lut setting lives here */
    struct rhdCrtcLUTPrivate *LUTPriv;  /* each CRTC subsystem may define this independently */
    void (*LUTSelect) (struct rhdCrtc *Crtc, struct rhdLUT *LUT);
    void (*LUTSave) (struct rhdCrtc *Crtc);
    void (*LUTRestore) (struct rhdCrtc *Crtc);
    void (*LUTDestroy) (struct rhdCrtc *Crtc);

    void (*Power) (struct rhdCrtc *Crtc, int Power);
    void (*Blank) (struct rhdCrtc *Crtc, Bool Blank);
};

Bool RHDCrtcsInit(RHDPtr rhdPtr);
void RHDAtomCrtcsInit(RHDPtr rhdPtr);
void RHDCrtcsDestroy(RHDPtr rhdPtr);
void RHDCrtcSave(struct rhdCrtc *Crtc);
void RHDCrtcRestore(struct rhdCrtc *Crtc);

/*
 * Calculate overscan values for scaler.
 */
struct rhdScalerOverscan
{
    int OverscanTop;
    int OverscanBottom;
    int OverscanLeft;
    int OverscanRight;
    enum rhdCrtcScaleType Type;
};

extern struct rhdScalerOverscan
rhdCalculateOverscan(DisplayModePtr Mode,
		     DisplayModePtr ScaledToMode,
		     enum rhdCrtcScaleType Type);


#endif /* _RHD_CRTC_H */
