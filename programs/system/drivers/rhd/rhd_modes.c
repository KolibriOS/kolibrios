/*
 * Copyright 2004-2008  Luc Verhaegen <lverhaegen@novell.com>
 * Copyright 2007, 2008 Matthias Hopf <mhopf@novell.com>
 * Copyright 2007, 2008 Egbert Eich   <eich@novell.com>
 * Copyright 2007, 2008 Advanced Micro Devices, Inc.
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
# include <stdio.h>
# include <string.h>
#endif


#include "rhd.h"

#include "edid.h"
#include "xf86DDC.h"

#include "rhd_crtc.h"
#include "rhd_pll.h"
#include "rhd_connector.h"
#include "rhd_output.h"
#include "rhd_modes.h"
#include "rhd_monitor.h"

/* For Acceleration FB validation */
//#include "r5xx_accel.h"

/*
 * Define a set of own mode errors.
 */
#define RHD_MODE_STATUS 0x51B00
#ifndef MONREC_HAS_REDUCED
#define MODE_NO_REDUCED     0x01 + RHD_MODE_STATUS
#endif
#define MODE_MEM_BW         0x02 + RHD_MODE_STATUS
#define MODE_OUTPUT_UNDEF   0x03 + RHD_MODE_STATUS
#define MODE_NOT_PAL        0x04 + RHD_MODE_STATUS
#define MODE_NOT_NTSC       0x05 + RHD_MODE_STATUS
#define MODE_HTOTAL_WIDE    0x06 + RHD_MODE_STATUS
#define MODE_HDISPLAY_WIDE  0x07 + RHD_MODE_STATUS
#define MODE_HSYNC_RANGE    0x08 + RHD_MODE_STATUS
#define MODE_HBLANK_RANGE   0x09 + RHD_MODE_STATUS
#define MODE_VTOTAL_WIDE    0x0A + RHD_MODE_STATUS
#define MODE_VDISPLAY_WIDE  0x0B + RHD_MODE_STATUS
#define MODE_VSYNC_RANGE    0x0C + RHD_MODE_STATUS
#define MODE_VBLANK_RANGE   0x0D + RHD_MODE_STATUS
#define MODE_PITCH          0x0E + RHD_MODE_STATUS
#define MODE_OFFSET         0x0F + RHD_MODE_STATUS
#define MODE_MINHEIGHT      0x10 + RHD_MODE_STATUS
#define MODE_FIXED          0x11 + RHD_MODE_STATUS
#define MODE_SCALE          0x12 + RHD_MODE_STATUS

/*
 * Don't bother with checking whether X offers this. Just use the internal one
 * I'm the author of the X side one anyway.
 */

/*
 * Generate a CVT standard mode from HDisplay, VDisplay and VRefresh.
 *
 * These calculations are stolen from the CVT calculation spreadsheet written
 * by Graham Loveridge. He seems to be claiming no copyright and there seems to
 * be no license attached to this. He apparently just wants to see his name
 * mentioned.
 *
 * This file can be found at http://www.vesa.org/Public/CVT/CVTd6r1.xls
 *
 * Comments and structure corresponds to the comments and structure of the xls.
 * This should ease importing of future changes to the standard (not very
 * likely though).
 *
 * About margins; i'm sure that they are to be the bit between HDisplay and
 * HBlankStart, HBlankEnd and HTotal, VDisplay and VBlankStart, VBlankEnd and
 * VTotal, where the overscan colour is shown. FB seems to call _all_ blanking
 * outside sync "margin" for some reason. Since we prefer seeing proper
 * blanking instead of the overscan colour, and since the Crtc* values will
 * probably get altered after us, we will disable margins altogether. With
 * these calculations, Margins will plainly expand H/VDisplay, and we don't
 * want that. -- libv
 *
 */
DisplayModePtr
RHDCVTMode(int HDisplay, int VDisplay, float VRefresh, Bool Reduced,
           Bool Interlaced)
{
    DisplayModeRec  *Mode = xnfalloc(sizeof(DisplayModeRec));

    /* 1) top/bottom margin size (% of height) - default: 1.8 */
#define CVT_MARGIN_PERCENTAGE 1.8

    /* 2) character cell horizontal granularity (pixels) - default 8 */
#define CVT_H_GRANULARITY 1

    /* 4) Minimum vertical porch (lines) - default 3 */
#define CVT_MIN_V_PORCH 3

    /* 4) Minimum number of vertical back porch lines - default 6 */
#define CVT_MIN_V_BPORCH 6

    /* Pixel Clock step (kHz) */
#define CVT_CLOCK_STEP 250

    Bool Margins = FALSE;
    float  VFieldRate, HPeriod;
    int  HDisplayRnd, HMargin;
    int  VDisplayRnd, VMargin, VSync;
    float  Interlace; /* Please rename this */

    memset(Mode, 0, sizeof(DisplayModeRec));

    /* CVT default is 60.0Hz */
    if (!VRefresh)
        VRefresh = 60.0;

    /* 1. Required field rate */
    if (Interlaced)
        VFieldRate = VRefresh * 2;
    else
        VFieldRate = VRefresh;

    /* 2. Horizontal pixels */
    HDisplayRnd = HDisplay - (HDisplay % CVT_H_GRANULARITY);

    /* 3. Determine left and right borders */
    if (Margins) {
        /* right margin is actually exactly the same as left */
        HMargin = (((float) HDisplayRnd) * CVT_MARGIN_PERCENTAGE / 100.0);
        HMargin -= HMargin % CVT_H_GRANULARITY;
    } else
        HMargin = 0;

    /* 4. Find total active pixels */
    Mode->HDisplay = HDisplayRnd + 2*HMargin;

    /* 5. Find number of lines per field */
    if (Interlaced)
        VDisplayRnd = VDisplay / 2;
    else
        VDisplayRnd = VDisplay;

    /* 6. Find top and bottom margins */
    /* nope. */
    if (Margins)
        /* top and bottom margins are equal again. */
        VMargin = (((float) VDisplayRnd) * CVT_MARGIN_PERCENTAGE / 100.0);
    else
        VMargin = 0;

    Mode->VDisplay = VDisplay + 2*VMargin;

    /* 7. Interlace */
    if (Interlaced)
        Interlace = 0.5;
    else
        Interlace = 0.0;

    /* Determine VSync Width from aspect ratio */
    if (!(VDisplay % 3) && ((VDisplay * 4 / 3) == HDisplay))
        VSync = 4;
    else if (!(VDisplay % 9) && ((VDisplay * 16 / 9) == HDisplay))
        VSync = 5;
    else if (!(VDisplay % 10) && ((VDisplay * 16 / 10) == HDisplay))
        VSync = 6;
    else if (!(VDisplay % 4) && ((VDisplay * 5 / 4) == HDisplay))
        VSync = 7;
    else if (!(VDisplay % 9) && ((VDisplay * 15 / 9) == HDisplay))
        VSync = 7;
    else /* Custom */
        VSync = 10;

    if (!Reduced) { /* simplified GTF calculation */

        /* 4) Minimum time of vertical sync + back porch interval (µs)
         * default 550.0 */
#define CVT_MIN_VSYNC_BP 550.0

        /* 3) Nominal HSync width (% of line period) - default 8 */
#define CVT_HSYNC_PERCENTAGE 8

        float  HBlankPercentage;
        int  VSyncAndBackPorch, VBackPorch;
        int  HBlank;

        /* 8. Estimated Horizontal period */
        HPeriod = ((float) (1000000.0 / VFieldRate - CVT_MIN_VSYNC_BP)) /
            (VDisplayRnd + 2 * VMargin + CVT_MIN_V_PORCH + Interlace);

        /* 9. Find number of lines in sync + backporch */
        if (((int)(CVT_MIN_VSYNC_BP / HPeriod) + 1) < (VSync + CVT_MIN_V_PORCH))
            VSyncAndBackPorch = VSync + CVT_MIN_V_PORCH;
        else
            VSyncAndBackPorch = (int)(CVT_MIN_VSYNC_BP / HPeriod) + 1;

        /* 10. Find number of lines in back porch */
        VBackPorch = VSyncAndBackPorch - VSync;

        /* 11. Find total number of lines in vertical field */
        Mode->VTotal = VDisplayRnd + 2 * VMargin + VSyncAndBackPorch + Interlace
            + CVT_MIN_V_PORCH;

        /* 5) Definition of Horizontal blanking time limitation */
        /* Gradient (%/kHz) - default 600 */
#define CVT_M_FACTOR 600

        /* Offset (%) - default 40 */
#define CVT_C_FACTOR 40

        /* Blanking time scaling factor - default 128 */
#define CVT_K_FACTOR 128

        /* Scaling factor weighting - default 20 */
#define CVT_J_FACTOR 20

#define CVT_M_PRIME CVT_M_FACTOR * CVT_K_FACTOR / 256
#define CVT_C_PRIME (CVT_C_FACTOR - CVT_J_FACTOR) * CVT_K_FACTOR / 256 + \
        CVT_J_FACTOR

        /* 12. Find ideal blanking duty cycle from formula */
        HBlankPercentage = CVT_C_PRIME - CVT_M_PRIME * HPeriod/1000.0;

        /* 13. Blanking time */
        if (HBlankPercentage < 20)
            HBlankPercentage = 20;

        HBlank = Mode->HDisplay * HBlankPercentage/(100.0 - HBlankPercentage);
        HBlank -= HBlank % (2*CVT_H_GRANULARITY);

        /* 14. Find total number of pixels in a line. */
        Mode->HTotal = Mode->HDisplay + HBlank;

        /* Fill in HSync values */
        Mode->HSyncEnd = Mode->HDisplay + HBlank / 2;

        Mode->HSyncStart = Mode->HSyncEnd -
            (Mode->HTotal * CVT_HSYNC_PERCENTAGE) / 100;
        Mode->HSyncStart += CVT_H_GRANULARITY -
            Mode->HSyncStart % CVT_H_GRANULARITY;

        /* Fill in VSync values */
        Mode->VSyncStart = Mode->VDisplay + CVT_MIN_V_PORCH;
        Mode->VSyncEnd = Mode->VSyncStart + VSync;

    } else { /* Reduced blanking */
        /* Minimum vertical blanking interval time (µs) - default 460 */
#define CVT_RB_MIN_VBLANK 460.0

        /* Fixed number of clocks for horizontal sync */
#define CVT_RB_H_SYNC 32.0

        /* Fixed number of clocks for horizontal blanking */
#define CVT_RB_H_BLANK 160.0

        /* Fixed number of lines for vertical front porch - default 3 */
#define CVT_RB_VFPORCH 3

        int  VBILines;

        /* 8. Estimate Horizontal period. */
        HPeriod = ((float) (1000000.0 / VFieldRate - CVT_RB_MIN_VBLANK)) /
            (VDisplayRnd + 2*VMargin);

        /* 9. Find number of lines in vertical blanking */
        VBILines = ((float) CVT_RB_MIN_VBLANK) / HPeriod + 1;

        /* 10. Check if vertical blanking is sufficient */
        if (VBILines < (CVT_RB_VFPORCH + VSync + CVT_MIN_V_BPORCH))
            VBILines = CVT_RB_VFPORCH + VSync + CVT_MIN_V_BPORCH;

        /* 11. Find total number of lines in vertical field */
        Mode->VTotal = VDisplayRnd + 2 * VMargin + Interlace + VBILines;

        /* 12. Find total number of pixels in a line */
        Mode->HTotal = Mode->HDisplay + CVT_RB_H_BLANK;

        /* Fill in HSync values */
        Mode->HSyncEnd = Mode->HDisplay + CVT_RB_H_BLANK / 2;
        Mode->HSyncStart = Mode->HSyncEnd - CVT_RB_H_SYNC;

        /* Fill in VSync values */
        Mode->VSyncStart = Mode->VDisplay + CVT_RB_VFPORCH;
        Mode->VSyncEnd = Mode->VSyncStart + VSync;
    }

    /* 15/13. Find pixel clock frequency (kHz for xf86) */
    Mode->Clock = Mode->HTotal * 1000.0 / HPeriod;
    Mode->Clock -= Mode->Clock % CVT_CLOCK_STEP;

    /* 16/14. Find actual Horizontal Frequency (kHz) */
    Mode->HSync = ((float) Mode->Clock) / ((float) Mode->HTotal);

    /* 17/15. Find actual Field rate */
    Mode->VRefresh = (1000.0 * ((float) Mode->Clock)) /
        ((float) (Mode->HTotal * Mode->VTotal));

    /* 18/16. Find actual vertical frame frequency */
    /* ignore - just set the mode flag for interlaced */
    if (Interlaced)
        Mode->VTotal *= 2;

    {
        char  Name[256];
        Name[0] = 0;

        snprintf(Name, 256, "%dx%d", HDisplay, VDisplay);
        Mode->name = strdup(Name);
    }

    if (Reduced)
        Mode->Flags |= V_PHSYNC | V_NVSYNC;
    else
        Mode->Flags |= V_NHSYNC | V_PVSYNC;

    if (Interlaced)
        Mode->Flags |= V_INTERLACE;

    return Mode;
}

/*
 * Temporary.
 */
static void
add(char **p, char *new)
{
//  char *tmp = kmalloc(strlen(*p) + strlen(new) + 2);

  *p = (char*)realloc(*p, strlen(*p) + strlen(new) + 2);
  strcat(*p, " ");
  strcat(*p, new);
}

/*
 *
 */
Bool
rhdModesEqual(DisplayModePtr mode1, DisplayModePtr mode2)
{
    if (mode1->Clock == mode2->Clock
	&& mode1->HDisplay == mode2->HDisplay
	&& mode1->HSyncStart == mode2->HSyncStart
	&& mode1->HSyncEnd == mode2->HSyncEnd
	&& mode1->HTotal == mode2->HTotal
	&& mode1->HSkew == mode2->HSkew
	&& mode1->VDisplay == mode2->VDisplay
	&& mode1->VSyncStart == mode2->VSyncStart
	&& mode1->VSyncEnd == mode2->VSyncEnd
	&& mode1->VTotal == mode2->VTotal
	&& mode1->VScan == mode2->VScan
	&& mode1->Flags == mode2->Flags)
	return TRUE;

    return FALSE;
}

/*
 *
 */
void
RHDPrintModeline(DisplayModePtr mode)
{
    char tmp[256];
    char *flags = xnfcalloc(1, 1);

    if (mode->HSkew) {
	snprintf(tmp, 256, "hskew %i", mode->HSkew);
	add(&flags, tmp);
    }
    if (mode->VScan) {
	snprintf(tmp, 256, "vscan %i", mode->VScan);
	add(&flags, tmp);
    }
    if (mode->Flags & V_INTERLACE) add(&flags, "interlace");
    if (mode->Flags & V_CSYNC) add(&flags, "composite");
    if (mode->Flags & V_DBLSCAN) add(&flags, "doublescan");
    if (mode->Flags & V_BCAST) add(&flags, "bcast");
    if (mode->Flags & V_PHSYNC) add(&flags, "+hsync");
    if (mode->Flags & V_NHSYNC) add(&flags, "-hsync");
    if (mode->Flags & V_PVSYNC) add(&flags, "+vsync");
    if (mode->Flags & V_NVSYNC) add(&flags, "-vsync");
    if (mode->Flags & V_PCSYNC) add(&flags, "+csync");
    if (mode->Flags & V_NCSYNC) add(&flags, "-csync");
#if 0
    if (mode->Flags & V_CLKDIV2) add(&flags, "vclk/2");
#endif
    xf86Msg(X_NONE, "Modeline \"%s\"  %6.2f  %i %i %i %i  %i %i %i %i%s\n",
	    mode->name, mode->Clock/1000.,
	    mode->HDisplay, mode->HSyncStart, mode->HSyncEnd, mode->HTotal,
	    mode->VDisplay, mode->VSyncStart, mode->VSyncEnd, mode->VTotal,
	    flags);
    xfree(flags);
}

/*
 * xf86Mode.c should have a some more DisplayModePtr list handling.
 */
DisplayModePtr
RHDModesAdd(DisplayModePtr Modes, DisplayModePtr Additions)
{
    if (!Modes) {
        if (Additions)
            return Additions;
        else
            return NULL;
    }

    if (Additions) {
        DisplayModePtr Mode = Modes;

        while (Mode->next)
            Mode = Mode->next;

        Mode->next = Additions;
        Additions->prev = Mode;
    }

    return Modes;
}

/*
 *
 */
static DisplayModePtr
rhdModeDelete(DisplayModePtr Modes, DisplayModePtr Delete)
{
    DisplayModePtr Next, Previous;

    if (!Delete)
	return Modes;

    if (Modes == Delete)
	Modes = NULL;

    if (Delete->next == Delete)
	Delete->next = NULL;

    if (Delete->prev == Delete)
	Delete->next = NULL;

    Next = Delete->next;
    Previous = Delete->prev;

    if (Next)
	Next->prev = Previous;

    if (Previous)
	Previous->next = Next;

    xfree(Delete->name);
    xfree(Delete);

    if (Modes)
	return Modes;

    if (Next)
	return Next;

    if (Previous)
	while (Previous->prev)
	    Previous = Previous->prev;

    return Previous;
}

/*
 *
 */
DisplayModePtr
RHDModeCopy(DisplayModePtr Mode)
{
    DisplayModePtr New;

    if (!Mode)
        return NULL;

    New = xnfalloc(sizeof(DisplayModeRec));
    memcpy(New, Mode, sizeof(DisplayModeRec)); /* re-use private */
    New->name = strdup(Mode->name);
    New->prev = NULL;
    New->next = NULL;
    New->Private = Mode->Private;
    New->PrivSize = Mode->PrivSize;

    return New;
}

/*
 *
 */
static void
rhdModesDestroy(DisplayModePtr Modes)
{
    DisplayModePtr mode = Modes, next;

    while (mode) {
        next = mode->next;
        xfree(mode->name);
        xfree(mode);
        mode = next;
    }
}

/*
 * Basic sanity checks.
 */
static int
rhdModeSanity(RHDPtr rhdPtr, DisplayModePtr Mode)
{
    /* do we need to bother at all? */
    if (Mode->status != MODE_OK)
        return Mode->status;

    if (!Mode->name) {
	xf86DrvMsg(rhdPtr->scrnIndex, X_ERROR,
		   "Validation found mode without name.\n");
        return MODE_ERROR;
    }

    if (Mode->Clock <= 0)
        return MODE_NOCLOCK;

    if ((Mode->HDisplay <= 0) || (Mode->HSyncStart <= 0) ||
        (Mode->HSyncEnd <= 0) || (Mode->HTotal <= 0))
        return MODE_H_ILLEGAL;

    if ((Mode->HTotal <= Mode->HSyncEnd) ||
        (Mode->HSyncEnd <= Mode->HSyncStart) ||
        (Mode->HSyncStart < Mode->HDisplay))
        return MODE_H_ILLEGAL;

    /* HSkew? */

    if ((Mode->VDisplay <= 0) || (Mode->VSyncStart <= 0) ||
        (Mode->VSyncEnd <= 0) || (Mode->VTotal <= 0))
        return MODE_V_ILLEGAL;

    if ((Mode->VTotal <= Mode->VSyncEnd) ||
        (Mode->VSyncEnd <= Mode->VSyncStart) ||
        (Mode->VSyncStart < Mode->VDisplay))
        return MODE_V_ILLEGAL;

    if ((Mode->VScan != 0) && (Mode->VScan != 1))
        return MODE_NO_VSCAN;

    if (Mode->Flags & V_DBLSCAN)
        return MODE_NO_DBLESCAN;

    /* Flags */
    return MODE_OK;
}

/*
 * After we passed the initial sanity check, we need to fill out the CRTC
 * values.
 */
static void
rhdModeFillOutCrtcValues(DisplayModePtr Mode)
{
    /* do we need to bother at all? */
    if (Mode->status != MODE_OK)
        return;

    Mode->ClockIndex = -1; /* Always! direct non-programmable support must die. */

    if (!Mode->SynthClock)
        Mode->SynthClock = Mode->Clock;

    if (!Mode->CrtcHDisplay)
        Mode->CrtcHDisplay = Mode->HDisplay;

    if (!Mode->CrtcHBlankStart)
        Mode->CrtcHBlankStart = Mode->HDisplay;

    if (!Mode->CrtcHSyncStart)
        Mode->CrtcHSyncStart = Mode->HSyncStart;

    if (!Mode->CrtcHSyncEnd)
        Mode->CrtcHSyncEnd = Mode->HSyncEnd;

    if (!Mode->CrtcHBlankEnd)
        Mode->CrtcHBlankEnd = Mode->HTotal;

    if (!Mode->CrtcHTotal)
        Mode->CrtcHTotal = Mode->HTotal;

    if (!Mode->CrtcHSkew)
        Mode->CrtcHSkew = Mode->HSkew;

    if (!Mode->CrtcVDisplay)
        Mode->CrtcVDisplay = Mode->VDisplay;

    if (!Mode->CrtcVBlankStart)
        Mode->CrtcVBlankStart = Mode->VDisplay;

    if (!Mode->CrtcVSyncStart)
        Mode->CrtcVSyncStart = Mode->VSyncStart;

    if (!Mode->CrtcVSyncEnd)
        Mode->CrtcVSyncEnd = Mode->VSyncEnd;

    if (!Mode->CrtcVBlankEnd)
        Mode->CrtcVBlankEnd = Mode->VTotal;

    if (!Mode->CrtcVTotal)
        Mode->CrtcVTotal = Mode->VTotal;

    /* Always change these */
    Mode->HSync = ((float) Mode->SynthClock) / Mode->CrtcHTotal;
    Mode->VRefresh = (Mode->SynthClock * 1000.0) /
        (Mode->CrtcHTotal * Mode->CrtcVTotal);
    if (Mode->Flags & V_INTERLACE)
	Mode->VRefresh *= 2.0;
    if (Mode->Flags & V_DBLSCAN)
	Mode->VRefresh /= 2.0;

    /* We're usually first in the chain, right after rhdModeSanity. */
    Mode->CrtcHAdjusted = FALSE;
    Mode->CrtcVAdjusted = FALSE;

    /* Steer clear of PrivSize, Private and PrivFlags */
}

/*
 * Basic sanity checks.
 */
static int
rhdModeCrtcSanity(DisplayModePtr Mode)
{
    if (Mode->SynthClock <= 0)
        return MODE_NOCLOCK;

    if ((Mode->CrtcHDisplay <= 0) || (Mode->CrtcHBlankStart <= 0) ||
        (Mode->CrtcHSyncStart <= 0) || (Mode->CrtcHSyncEnd <= 0) ||
        (Mode->CrtcHBlankEnd <= 0) || (Mode->CrtcHTotal <= 0))
        return MODE_H_ILLEGAL;

    /* there seem to be no alignment constraints on horizontal timing on our
       hardware here */

    if ((Mode->CrtcHTotal < Mode->CrtcHBlankEnd) ||
        (Mode->CrtcHBlankEnd <= Mode->CrtcHSyncEnd) ||
        (Mode->CrtcHSyncEnd <= Mode->CrtcHSyncStart) ||
        (Mode->CrtcHSyncStart < Mode->CrtcHBlankStart) ||
        (Mode->CrtcHBlankStart < Mode->CrtcHDisplay))
        return MODE_H_ILLEGAL;

    /* CrtcHSkew? */

    if ((Mode->CrtcVDisplay <= 0) || (Mode->CrtcVBlankStart <= 0) ||
        (Mode->CrtcVSyncStart <= 0) || (Mode->CrtcVSyncEnd <= 0) ||
        (Mode->CrtcVBlankEnd <= 0) || (Mode->CrtcVTotal <= 0))
        return MODE_V_ILLEGAL;

    if ((Mode->CrtcVTotal < Mode->CrtcVBlankEnd) ||
        (Mode->CrtcVBlankEnd <= Mode->CrtcVSyncEnd) ||
        (Mode->CrtcVSyncEnd <= Mode->CrtcVSyncStart) ||
        (Mode->CrtcVSyncStart < Mode->CrtcVBlankStart) ||
        (Mode->CrtcVBlankStart < Mode->CrtcVDisplay))
        return MODE_V_ILLEGAL;

    return MODE_OK;
}

/*
 *
 */
static Bool
rhdMonitorFixedValid(struct rhdMonitor *Monitor, DisplayModePtr Mode)
{
    DisplayModePtr Fixed;

    for (Fixed = Monitor->Modes; Fixed; Fixed = Fixed->next) {
	if ((Mode->Flags != Fixed->Flags) ||
	    (Mode->Clock != Fixed->Clock) ||
	    (Mode->SynthClock != Fixed->Clock))
	    continue;

	if ((Mode->HDisplay > Fixed->HDisplay) ||
	    (Mode->VDisplay > Fixed->VDisplay))
	    continue;

	if ((Mode->HSyncStart != Fixed->HSyncStart) ||
	    (Mode->HSyncEnd != Fixed->HSyncEnd))
	    continue;

	if ((Mode->VSyncStart != Fixed->VSyncStart) ||
	    (Mode->VSyncEnd != Fixed->VSyncEnd))
	    continue;

	if ((Mode->CrtcHDisplay > Fixed->HDisplay) ||
	    (Mode->CrtcVDisplay > Fixed->VDisplay))
	    continue;

	if ((Mode->CrtcHBlankStart != Fixed->HDisplay) ||
	    (Mode->CrtcHSyncStart != Fixed->HSyncStart) ||
	    (Mode->CrtcHSyncEnd != Fixed->HSyncEnd) ||
	    (Mode->CrtcHBlankEnd != Fixed->HTotal))
	    continue;

	if ((Mode->CrtcVBlankStart != Fixed->VDisplay) ||
	    (Mode->CrtcVSyncStart != Fixed->VSyncStart) ||
	    (Mode->CrtcVSyncEnd != Fixed->VSyncEnd) ||
	    (Mode->CrtcVBlankEnd != Fixed->VTotal))
	    continue;

	return TRUE;
    }

    return FALSE;
}
/*
 * TODO: review fixed modes when doing different modes on both crtcs.
 */
static int
rhdMonitorValid(struct rhdMonitor *Monitor, DisplayModePtr Mode)
{
    int i;
    Bool isNative = FALSE;

    if (Monitor->NativeMode && rhdModesEqual(Mode, Monitor->NativeMode))
	isNative = TRUE;

    for (i = 0; i < Monitor->numHSync; i++)
        if ((Mode->HSync >= (Monitor->HSync[i].lo * (1.0 - SYNC_TOLERANCE))) &&
            (Mode->HSync <= (Monitor->HSync[i].hi * (1.0 + SYNC_TOLERANCE))))
            break;
    if (Monitor->numHSync && (i == Monitor->numHSync))
        return MODE_HSYNC;

    for (i = 0; i < Monitor->numVRefresh; i++)
        if ((Mode->VRefresh >= (Monitor->VRefresh[i].lo * (1.0 - SYNC_TOLERANCE))) &&
            (Mode->VRefresh <= (Monitor->VRefresh[i].hi * (1.0 + SYNC_TOLERANCE))))
            break;
    if (Monitor->numVRefresh && (i == Monitor->numVRefresh))
        return MODE_VSYNC;

    if (Monitor->Bandwidth &&
	(Mode->SynthClock > (Monitor->Bandwidth * (1 + SYNC_TOLERANCE))))
        return MODE_CLOCK_HIGH;

    if (isNative) { /* if it's this monitor's native mode be less strict on validation */
    if (Monitor->ReducedAllowed) {
	    if ((Mode->CrtcHDisplay * 101) > (Mode->CrtcHTotal * 100)) /* 1% */
	    return MODE_HBLANK_NARROW;
    } else { /* no reduced blanking */
	if ((Mode->CrtcHDisplay * 23) > (Mode->CrtcHTotal * 20)) /* 15% */
	    return MODE_HBLANK_NARROW;
    }
    } else {
	if (((Mode->CrtcHDisplay * 5 / 4) & ~0x07) > Mode->CrtcHTotal) {
	    /* is this a cvt -r Mode, and only a cvt -r Mode? */
	    if (((Mode->CrtcHTotal - Mode->CrtcHDisplay) == 160) &&
		((Mode->CrtcHSyncEnd - Mode->CrtcHDisplay) == 80) &&
		((Mode->CrtcHSyncEnd - Mode->CrtcHSyncStart) == 32) &&
		((Mode->CrtcVSyncStart - Mode->CrtcVDisplay) == 3)) {
		if (!Monitor->ReducedAllowed)
		    return MODE_NO_REDUCED;
	    } else if ((Mode->CrtcHDisplay * 11) > (Mode->CrtcHTotal * 10))
		return MODE_HSYNC_NARROW;
	}
    }

    if (Monitor->UseFixedModes && !rhdMonitorFixedValid(Monitor, Mode))
	return MODE_FIXED;

    return MODE_OK;
}

#define RHD_MODE_VALIDATION_LOOPS 10

enum ValidationKind {
    VALIDATE_SCALE_NONE,
    VALIDATE_SCALE_FROM,
    VALIDATE_SCALE_TO
};

/*
 *
 */
static int
rhdModeValidateCrtc(struct rhdCrtc *Crtc, DisplayModePtr Mode, enum ValidationKind ValidateScaleModeKind)
{
    RHDPtr rhdPtr = RHDPTRI(Crtc);
    ScrnInfoPtr pScrn = rhdPtr->pScrn;
    int Status, i;

    RHDFUNC(Crtc);

    Status = rhdModeSanity(rhdPtr, Mode);
    if (Status != MODE_OK)
        return Status;

    rhdModeFillOutCrtcValues(Mode);

    /* We don't want to loop around this forever */
    for (i = 0; i < RHD_MODE_VALIDATION_LOOPS; i++) {
        struct rhdOutput *Output;

        Mode->CrtcHAdjusted = FALSE;
        Mode->CrtcVAdjusted = FALSE;

        Status = rhdModeCrtcSanity(Mode);
        if (Status != MODE_OK)
            return Status;
        if (Mode->CrtcHAdjusted || Mode->CrtcVAdjusted)
            continue;

	if (ValidateScaleModeKind != VALIDATE_SCALE_TO) {

	Status = Crtc->FBValid(Crtc, Mode->CrtcHDisplay, Mode->CrtcVDisplay,
			       pScrn->bitsPerPixel, rhdPtr->FbScanoutStart,
			       rhdPtr->FbScanoutSize, NULL);
        if (Status != MODE_OK)
            return Status;

	    if (Crtc->ScaleValid) {
		if (ValidateScaleModeKind == VALIDATE_SCALE_NONE)
		    Status = Crtc->ScaleValid(Crtc, RHD_CRTC_SCALE_TYPE_NONE, Mode, NULL);
		else
		    Status = Crtc->ScaleValid(Crtc, Crtc->ScaleType, Mode, Crtc->ScaledToMode);
        if (Status != MODE_OK)
            return Status;
        if (Mode->CrtcHAdjusted || Mode->CrtcVAdjusted)
            continue;
	    }
	}

	if (ValidateScaleModeKind != VALIDATE_SCALE_FROM) {
	    Status = Crtc->ModeValid(Crtc, Mode);
	    if (Status != MODE_OK)
		return Status;
	    if (Mode->CrtcHAdjusted || Mode->CrtcVAdjusted)
		continue;

	    if (Crtc->PLL && Crtc->PLL->Valid) { /* RandR may not have PLL filled out. oh well... */
	    Status = Crtc->PLL->Valid(Crtc->PLL, Mode->Clock);
	    if (Status != MODE_OK)
		return Status;
	    if (Mode->CrtcHAdjusted || Mode->CrtcVAdjusted)
		continue;
	}

        for (Output = rhdPtr->Outputs; Output; Output = Output->Next)
            if (Output->Active && (Output->Crtc == Crtc)) {
		/* Check the output */
                Status = Output->ModeValid(Output, Mode);
                if (Status != MODE_OK)
                    return Status;
                if (Mode->CrtcHAdjusted || Mode->CrtcVAdjusted)
                    break; /* restart. */

		/* Check the monitor attached to this output */
		if (Output->Connector && Output->Connector->Monitor)
		    Status = rhdMonitorValid(Output->Connector->Monitor, Mode);
		if (Status != MODE_OK)
                    return Status;
                if (Mode->CrtcHAdjusted || Mode->CrtcVAdjusted)
                    break; /* restart. */
            }

	    if (Output) /* We're done. This must be a good mode. */
		continue;
	}

            return MODE_OK;
    }

    /* Mode has been bouncing around for ages, on adjustments */
    xf86DrvMsg(Crtc->scrnIndex, X_ERROR, "%s: Mode \"%s\" (%dx%d:%3.1fMhz) was"
               " thrown around for too long.\n", __func__, Mode->name,
               Mode->HDisplay, Mode->VDisplay, Mode->Clock/1000.0);
    return MODE_ERROR;
}

/*
 *
 */
int
RHDValidateScaledToMode(struct rhdCrtc *Crtc, DisplayModePtr Mode)
{
    RHDPtr rhdPtr = RHDPTRI(Crtc);
    int Status;

    RHDFUNC(Crtc);

    Status = rhdModeSanity(rhdPtr, Mode);
    if (Status != MODE_OK)
        return Status;

    rhdModeFillOutCrtcValues(Mode);

    Status = rhdModeValidateCrtc(Crtc, Mode, VALIDATE_SCALE_TO);
    if (Status != MODE_OK)
	return Status;

    /* Do we want to also validate against a configured monitor? */
    if (rhdPtr->ConfigMonitor) {
	Status = rhdMonitorValid(rhdPtr->ConfigMonitor, Mode);
	if (Status != MODE_OK)
	    return Status;
    }

    return MODE_OK;
}

/*
 *
 */
static int
rhdModeValidate(ScrnInfoPtr pScrn, DisplayModePtr Mode)
{
    RHDPtr rhdPtr = RHDPTR(pScrn);
    struct rhdCrtc *Crtc;
    int Status;
    int i;

    Status = rhdModeSanity(rhdPtr, Mode);
    if (Status != MODE_OK)
        return Status;

    rhdModeFillOutCrtcValues(Mode);

    /* now let our modesetting tree have its say */
    for (i = 0; i < 2; i++) {
        Crtc = rhdPtr->Crtc[i];
        if (!Crtc->Active)
            continue;

	if (!Crtc->ScaledToMode) {

	    Status = rhdModeValidateCrtc(Crtc, Mode, VALIDATE_SCALE_NONE);
	if (Status != MODE_OK)
	    return Status;

	} else {
	    Status = rhdModeValidateCrtc(Crtc, Mode, VALIDATE_SCALE_FROM);
	    if (Status != MODE_OK)
		return Status;
    }
    }

    /* throw them at the configured monitor, so that the inadequate
     * conf file at least has some influence. */
    if (rhdPtr->ConfigMonitor) {
	Status = rhdMonitorValid(rhdPtr->ConfigMonitor, Mode);
	if (Status != MODE_OK)
	    return Status;
    }

    /* Did we set up virtual resolution already? */
    if ((pScrn->virtualX > 0) && (pScrn->virtualY > 0)) {
        if (pScrn->virtualX < Mode->CrtcHDisplay)
            return MODE_VIRTUAL_X;
        if (pScrn->virtualY < Mode->CrtcVDisplay)
            return MODE_VIRTUAL_Y;
    }

    return MODE_OK;
}

/*
 * Wrap the limited xf86 Mode statusses with our own message.
 */
struct {
    int Status;
    char *Message;
} rhdModeStatusMessages[] = {
    { MODE_NO_REDUCED,    "Reduced blanking is not supported."},
    { MODE_MEM_BW,        "Memory bandwidth exceeded."},
    { MODE_OUTPUT_UNDEF,  "Mode not defined by output device."},
    { MODE_NOT_PAL,       "This is not a PAL TV mode."},
    { MODE_NOT_NTSC,      "This is not an NTSC TV mode."},
    { MODE_HTOTAL_WIDE,   "Horizontal Total is out of range."},
    { MODE_HDISPLAY_WIDE, "Mode is too wide."},
    { MODE_HSYNC_RANGE,   "Horizontal Sync Start is out of range."},
    { MODE_HBLANK_RANGE,  "Horizontal Blanking Start is out of range."},
    { MODE_VTOTAL_WIDE,   "Vertical Total is out of range.\n"},
    { MODE_VDISPLAY_WIDE, "Mode is too high."},
    { MODE_VSYNC_RANGE,   "Vertical Sync Start is out of range.\n"},
    { MODE_VBLANK_RANGE,  "Vertical Blanking Start is out of range."},
    { MODE_PITCH,         "Scanout buffer Pitch too wide."},
    { MODE_OFFSET,        "Scanout buffer offset too high in FB."},
    { MODE_MINHEIGHT,     "Height too low."},
    { MODE_FIXED,         "Mode not compatible with fixed mode."},
    { MODE_SCALE,         "Mode cannot be scaled to fixed mode."},
    { 0, NULL}
};

const char *
xf86ModeStatusToString(ModeStatus status)
{
    switch (status) {
    case MODE_OK:
	return "Mode OK";
    case MODE_HSYNC:
	return "hsync out of range";
    case MODE_VSYNC:
	return "vrefresh out of range";
    case MODE_H_ILLEGAL:
	return "illegal horizontal timings";
    case MODE_V_ILLEGAL:
	return "illegal vertical timings";
    case MODE_BAD_WIDTH:
	return "width requires unsupported line pitch";
    case MODE_NOMODE:
	return "no mode of this name";
    case MODE_NO_INTERLACE:
	return "interlace mode not supported";
    case MODE_NO_DBLESCAN:
	return "doublescan mode not supported";
    case MODE_NO_VSCAN:
	return "multiscan mode not supported";
    case MODE_MEM:
	return "insufficient memory for mode";
    case MODE_VIRTUAL_X:
	return "width too large for virtual size";
    case MODE_VIRTUAL_Y:
	return "height too large for virtual size";
    case MODE_MEM_VIRT:
	return "insufficient memory given virtual size";
    case MODE_NOCLOCK:
	return "no clock available for mode";
    case MODE_CLOCK_HIGH:
	return "mode clock too high";
    case MODE_CLOCK_LOW:
	return "mode clock too low";
    case MODE_CLOCK_RANGE:
	return "bad mode clock/interlace/doublescan";
    case MODE_BAD_HVALUE:
	return "horizontal timing out of range";
    case MODE_BAD_VVALUE:
	return "vertical timing out of range";
    case MODE_BAD_VSCAN:
	return "VScan value out of range";
    case MODE_HSYNC_NARROW:
	return "horizontal sync too narrow";
    case MODE_HSYNC_WIDE:
	return "horizontal sync too wide";
    case MODE_HBLANK_NARROW:
	return "horizontal blanking too narrow";
    case MODE_HBLANK_WIDE:
	return "horizontal blanking too wide";
    case MODE_VSYNC_NARROW:
	return "vertical sync too narrow";
    case MODE_VSYNC_WIDE:
	return "vertical sync too wide";
    case MODE_VBLANK_NARROW:
	return "vertical blanking too narrow";
    case MODE_VBLANK_WIDE:
	return "vertical blanking too wide";
    case MODE_PANEL:
	return "exceeds panel dimensions";
    case MODE_INTERLACE_WIDTH:
	return "width too large for interlaced mode";
    case MODE_ONE_WIDTH:
        return "all modes must have the same width";
    case MODE_ONE_HEIGHT:
        return "all modes must have the same height";
    case MODE_ONE_SIZE:
        return "all modes must have the same resolution";
    case MODE_BAD:
	return "unknown reason";
    case MODE_ERROR:
	return "internal error";
    default:
	return "unknown";
    }
}

const char *RHDModeStatusToString(int Status)
{
    if ((Status & 0xFFF00) == RHD_MODE_STATUS) {
        int i;

        for (i = 0; rhdModeStatusMessages[i].Message; i++)
            if (rhdModeStatusMessages[i].Status == Status)
                return rhdModeStatusMessages[i].Message;
        ErrorF("%s: unhandled Status type: 0x%X\n", __func__, Status);
        return "Unknown status.";

    } else
        return xf86ModeStatusToString(Status);
}

/*
 *
 */
static DisplayModePtr
rhdModesGrabOnNameAll(DisplayModePtr *Modes, char *name)
{
    DisplayModePtr Mode, Matched = NULL, Temp;

    for (Mode = *Modes; Mode; ) {
        if (!strcmp(Mode->name, name)) {
            Temp = Mode;
            Mode = Mode->next;

            if (Temp->prev)
                Temp->prev->next = Mode;
            else
                *Modes = Mode;

            if (Mode)
                Mode->prev = Temp->prev;

            Temp->prev = NULL;
            Temp->next = Matched;
            if (Matched)
                Matched->prev = Temp;
            Matched = Temp;
        } else
            Mode = Mode->next;
    }

    return Matched;
}

/*
 *
 */
static DisplayModePtr
rhdModesGrabOnTypeAll(DisplayModePtr *Modes, int Type, int Mask)
{
    DisplayModePtr Mode, Matched = NULL, Temp;

    for (Mode = *Modes; Mode; ) {
        if ((Mode->type & Mask) == (Type & Mask)) {
            Temp = Mode;
            Mode = Mode->next;

            if (Temp->prev)
                Temp->prev->next = Mode;
            else
                *Modes = Mode;

            if (Mode)
                Mode->prev = Temp->prev;

            Temp->next = Matched;
            if (Matched)
                Matched->prev = Temp;
            Temp->prev = NULL;
            Matched = Temp;
        } else
            Mode = Mode->next;
    }

    return Matched;
}

/*
 *
 */
static DisplayModePtr
rhdModesGrabBestRefresh(DisplayModePtr *Modes)
{
    DisplayModePtr Mode, Best = NULL;

    if (!*Modes)
        return NULL;

    Best = *Modes;

    for (Mode = Best->next; Mode; Mode = Mode->next)
        if (Best->VRefresh < Mode->VRefresh)
            Best = Mode;
        else if (Best->VRefresh == Mode->VRefresh) {
            /* Same name != same resolution */
            if ((Best->HDisplay * Best->VDisplay) <
                (Mode->HDisplay * Mode->VDisplay))
                Best = Mode;
            else if ((Best->HDisplay * Best->VDisplay) ==
                     (Mode->HDisplay * Mode->VDisplay)) {
                /* Lower bandwidth == better! */
                if (Best->Clock > Mode->Clock)
                    Best = Mode;
            }
        }

    if (Best->next)
        Best->next->prev = Best->prev;
    if (Best->prev)
        Best->prev->next = Best->next;
    if (Best == *Modes)
        *Modes = (*Modes)->next;

    Best->next = NULL;
    Best->prev = NULL;

    return Best;
}

/*
 *
 */
static DisplayModePtr
rhdModesGrabOnHighestType(DisplayModePtr *Modes)
{
    DisplayModePtr Mode;

    /* User provided, but can also have another source. */
    Mode = rhdModesGrabOnTypeAll(Modes, M_T_USERDEF, 0xF0);
    if (Mode)
        return Mode;

    /* Often EDID provided, but can also have another source. */
    Mode = rhdModesGrabOnTypeAll(Modes, M_T_DRIVER, 0xF0);
    if (Mode)
        return Mode;

    /* No reason why we should treat built-in and vesa separately */
    Mode = *Modes;
    *Modes = NULL;
    return Mode;
}

/*
 *
 */
static DisplayModePtr
rhdModesSortOnSize(DisplayModePtr Modes)
{
    DisplayModePtr Sorted, Mode, Temp, Next;

    if (!Modes)
        return NULL;

    Sorted = Modes;
    Modes = Modes->next;

    Sorted->next = NULL;
    Sorted->prev = NULL;

    for (Next = Modes; Next; ) {
        /* since we're taking modelines from in between */
        Mode = Next;
        Next = Next->next;

        for (Temp = Sorted; Temp; Temp = Temp->next) {
            /* nasty ! */
            if (((Temp->CrtcHDisplay * Temp->CrtcVDisplay) <
                (Mode->CrtcHDisplay * Mode->CrtcVDisplay)) ||
                (((Temp->CrtcHDisplay * Temp->CrtcVDisplay) ==
                  (Mode->CrtcHDisplay * Mode->CrtcVDisplay)) &&
                 ((Temp->VRefresh < Mode->VRefresh)  ||
                  ((Temp->VRefresh < Mode->VRefresh) &&
                   (Temp->SynthClock < Mode->SynthClock))))) {
                Mode->next = Temp;
                Mode->prev = Temp->prev;
                Temp->prev = Mode;
                if (Mode->prev)
                    Mode->prev->next = Mode;
                else
                    Sorted = Mode;
                break;
            }

            if (!Temp->next) {
                Temp->next = Mode;
                Mode->prev = Temp;
                Mode->next = NULL;
                break;
            }
        }
    }

    return Sorted;
}

#if 0
/*
 * take a modename, try to parse it, if that works, generate the CVT modeline.
 */
static DisplayModePtr
rhdModeCreateFromName(ScrnInfoPtr pScrn, char *name, Bool Silent)
{
    DisplayModePtr Mode;
    int HDisplay = 0, VDisplay = 0, tmp;
    float VRefresh = 0;
    Bool Reduced;
    int Status;

    sscanf(name, "%dx%d@%f", &HDisplay, &VDisplay, &VRefresh);
    if (!HDisplay || !VDisplay) {
        if (!Silent)
            xf86DrvMsg(pScrn->scrnIndex, X_INFO, "%s: Unable to generate "
                       "Modeline for Mode \"%s\"\n", __func__, name);
        return NULL;
    }

    tmp = strlen(name) - 1;
    if ((name[tmp] == 'r') || (name[tmp] == 'R'))
        Reduced = TRUE;
    else
        Reduced = FALSE;

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	       "Generating Modeline for \"%s\"\n", name);

    /* First, try a plain CVT mode */
    Mode = RHDCVTMode(HDisplay, VDisplay, VRefresh, Reduced, FALSE);
    xfree(Mode->name);
    Mode->name = xnfstrdup(name);
    Mode->type = M_T_USERDEF;

    Status = rhdModeValidate(pScrn, Mode);
    if (Status == MODE_OK)
        return Mode;
    rhdModesDestroy(Mode);

#if 0 /* noscale mode */
    /* Now see if we have fixed modes */
    for (i = 0; i < 2; i++) {
        Crtc = rhdPtr->Crtc[i];

        if (!Crtc->Active || !Crtc->FixedMode)
            continue;

        Mode = RHDModeCopy(Crtc->FixedMode);
        xfree(Mode->name);
        Mode->name = xnfstrdup(name);
        Mode->type = M_T_USERDEF;

        Mode->HDisplay = HDisplay;
        Mode->CrtcHDisplay = 0; /* set by validation code */
        Mode->VDisplay = VDisplay;
        Mode->CrtcVDisplay = 0;

	Status = rhdModeValidate(pScrn, Mode);
        if (Status == MODE_OK)
            return Mode;
        rhdModesDestroy(Mode);
    }
#endif

    if (!Silent)
	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Rejected mode \"%s\" "
		   "(%dx%d):\n\t %s\n", name, HDisplay, VDisplay,
		   RHDModeStatusToString(Status));
    return NULL;
}
#endif

/*
 *
 */
static DisplayModePtr
rhdModesListValidateAndCopy(ScrnInfoPtr pScrn, DisplayModePtr Modes, Bool Silent)
{
  DisplayModePtr Keepers = NULL, Check, Mode;
  int Status;

    for (Check = Modes; Check; Check = Check->next) {
    Mode = RHDModeCopy(Check);

    Status = rhdModeValidate(pScrn, Mode);
    if (Status == MODE_OK)
	    Keepers = RHDModesAdd(Keepers, Mode);
	else {
	    if (!Silent)
		xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Rejected mode \"%s\" "
			   "(%dx%d:%3.1fMhz): %s\n", Mode->name,
			   Mode->HDisplay, Mode->VDisplay,
			   Mode->Clock / 1000.0, RHDModeStatusToString(Status));
	    xfree(Mode->name);
	    xfree(Mode);
    }
  }

  return Keepers;
}

/*
 * Create the list of all modes that are currently valid
 */
static DisplayModePtr
rhdCreateModesListAndValidate(ScrnInfoPtr pScrn, Bool Silent)
{
    RHDPtr rhdPtr = RHDPTR(pScrn);
  DisplayModePtr Keepers = NULL, Modes;
  struct rhdCrtc *Crtc;
  struct rhdOutput *Output;
  int i;

    RHDFUNC(pScrn);

    /* Cycle through our monitors list, and find a fixed mode one */
    for (i = 0; i < 2; i++) {
    Crtc = rhdPtr->Crtc[i];
	for (Output = rhdPtr->Outputs; Output; Output = Output->Next) {
	    if (Output->Active && (Output->Crtc == Crtc)) {
        if (Output->Connector && Output->Connector->Monitor
		    && Output->Connector->Monitor->UseFixedModes
		    && !Crtc->ScaledToMode) {
          Modes = Output->Connector->Monitor->Modes;
          if (!Silent && Modes)
			xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Validating Fixed"
                      " Modes from Monitor \"%s\"\n\t on Connector"
                      " \"%s\"\n", Output->Connector->Monitor->Name,
				   Output->Connector->Name);

          Modes = rhdModesListValidateAndCopy(pScrn, Modes, Silent);
          Keepers = RHDModesAdd(Keepers, Modes);
          return Keepers;
        }
	    }
    }
  }


    /* Cycle through our actual monitors list */
    for (i = 0; i < 2; i++) {
    Crtc = rhdPtr->Crtc[i];

	for (Output = rhdPtr->Outputs; Output; Output = Output->Next) {
	    if (Output->Active && (Output->Crtc == Crtc)) {
		if (Output->Connector && Output->Connector->Monitor) {
          Modes = Output->Connector->Monitor->Modes;
          if (!Silent && Modes)
			xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Validating Modes "
                      "from Monitor \"%s\" on \"%s\"\n",
                      Output->Connector->Monitor->Name,
				   Output->Connector->Name);

          Modes = rhdModesListValidateAndCopy(pScrn, Modes, Silent);
          Keepers = RHDModesAdd(Keepers, Modes);
        }
	    }
    }
  }

  return Keepers;
}

/*
 *
 */
DisplayModePtr
RHDModesPoolCreate(ScrnInfoPtr pScrn, Bool Silent)
{
    DisplayModePtr Pool = NULL, List, TempList, Temp;
    char **ModeNames = NULL; //pScrn->display->modes;
    int i;

    RHDFUNC(pScrn);

    List = rhdCreateModesListAndValidate(pScrn, Silent);
    if (!List)
        return List;

    /* Reduce our list */
    if (ModeNames && ModeNames[0]) { /* Find the best matching mode for each name */
        for (i = 0; ModeNames[i]; i++) {
        TempList = rhdModesGrabOnNameAll(&List, ModeNames[i]);
            if (TempList) {
          Temp = rhdModesGrabOnHighestType(&TempList);
          rhdModesDestroy(TempList);

          TempList = Temp;
          Temp = rhdModesGrabOnTypeAll(&TempList, M_T_PREFERRED, M_T_PREFERRED);
                if (Temp) {
            rhdModesDestroy(TempList);
            TempList = Temp;
          }

          Temp = rhdModesGrabBestRefresh(&TempList);

          rhdModesDestroy(TempList);
        }
      //  else /* No matching modes found, generate */
      //    Temp = rhdModeCreateFromName(pScrn, ModeNames[i], Silent);

        if (Temp)
          Pool = RHDModesAdd(Pool, Temp);
      }
      rhdModesDestroy(List);
    } else { /* No names, just work the list directly */
      Temp = rhdModesGrabOnHighestType(&List);
      rhdModesDestroy(List);
      List = Temp;

        while (List) {
        TempList = rhdModesGrabOnNameAll(&List, List->name);

        Temp = rhdModesGrabOnTypeAll(&TempList, M_T_PREFERRED, M_T_PREFERRED);
            if (Temp) {
           rhdModesDestroy(TempList);
           TempList = Temp;
        }

        Temp = rhdModesGrabBestRefresh(&TempList);
        rhdModesDestroy(TempList);

        Pool = RHDModesAdd(Pool, Temp);
      }

        /* Sort our list */
      TempList = Pool;

        /* Sort higher priority modes separately */
      Pool = rhdModesGrabOnTypeAll(&TempList, M_T_PREFERRED, M_T_PREFERRED);
      Pool = rhdModesSortOnSize(Pool);

      TempList = rhdModesSortOnSize(TempList);

      Pool = RHDModesAdd(Pool, TempList);
    }

    return Pool;
}

/*
 *
 */
void
RHDModesAttach(ScrnInfoPtr pScrn, DisplayModePtr Modes)
{
    DisplayModePtr Mode = Modes;

    pScrn->modes = Modes;
    pScrn->currentMode = Modes;

    while (Mode->next) {
        Mode->type = M_T_USERDEF; /* satisfy xf86ZoomViewport */
        Mode = Mode->next;
    }

    Mode->type = M_T_USERDEF;

    /* Make our list circular */
    Mode->next = pScrn->modes;
    pScrn->modes->prev = Mode;
}

/*
 *
 */
#if 0
Bool
RHDGetVirtualFromConfig(ScrnInfoPtr pScrn)
{
    RHDPtr rhdPtr = RHDPTR(pScrn);
    struct rhdCrtc *Crtc1 = rhdPtr->Crtc[0], *Crtc2 = rhdPtr->Crtc[1];
    CARD32 VirtualX = pScrn->display->virtualX;
    CARD32 VirtualY = pScrn->display->virtualY;
    CARD32 Pitch1, Pitch2;
    float Ratio = (float) pScrn->display->virtualY / pScrn->display->virtualX;
    int ret = FALSE;

    RHDFUNC(pScrn);

    while (VirtualX && VirtualY) {
	ret = Crtc1->FBValid(Crtc1, VirtualX, VirtualY, pScrn->bitsPerPixel,
			     rhdPtr->FbScanoutStart, rhdPtr->FbScanoutSize, &Pitch1);
	if (ret != MODE_OK)
	    goto shrink;
	ret = Crtc2->FBValid(Crtc2, VirtualX, VirtualY, pScrn->bitsPerPixel,
			     rhdPtr->FbScanoutStart, rhdPtr->FbScanoutSize, &Pitch2);
	if (ret != MODE_OK)
	    goto shrink;

	if (Pitch1 != Pitch2)
	    goto shrink;
#if 0
	/* let 2d acceleration have a say as well */
	if (rhdPtr->AccelMethod >= RHD_ACCEL_XAA)
	    if (rhdPtr->ChipSet < RHD_R600) /* badly abstracted, i know */
		if (!R5xx2DFBValid(rhdPtr, VirtualX, VirtualY, pScrn->bitsPerPixel,
				   rhdPtr->FbScanoutStart, rhdPtr->FbScanoutSize, Pitch1))
		    goto shrink;
#endif
	break; /* must be good then. */
    shrink:
	VirtualX--;
	VirtualY = Ratio * VirtualX;
    }

    if (VirtualX && VirtualY) {
	pScrn->virtualX = VirtualX;
	pScrn->virtualY = VirtualY;
	pScrn->displayWidth = Pitch1;
	return TRUE;
    } else
	return FALSE;
}

/*
 *
 */
void
RHDGetVirtualFromModesAndFilter(ScrnInfoPtr pScrn, DisplayModePtr Modes, Bool Silent)
{
    RHDPtr rhdPtr = RHDPTR(pScrn);
    struct rhdCrtc *Crtc1 = rhdPtr->Crtc[0], *Crtc2 = rhdPtr->Crtc[1];
    DisplayModePtr Mode, Next;
    CARD32 VirtualX = 0;
    CARD32 VirtualY = 0;
    CARD32 Pitch1, Pitch2;
    int ret = FALSE;

    RHDFUNC(pScrn);

    /* assert */
    if (!Modes)
	return;

    Mode = Modes;

    while (Mode) {
	if ((Mode->CrtcHDisplay > pScrn->virtualX) ||
            (Mode->CrtcVDisplay > pScrn->virtualY)) {
            if (Mode->CrtcHDisplay > pScrn->virtualX)
                VirtualX = Mode->CrtcHDisplay;
	    else
		VirtualX = pScrn->virtualX;

            if (Mode->CrtcVDisplay > pScrn->virtualY)
                VirtualY = Mode->CrtcVDisplay;
	    else
		VirtualY = pScrn->virtualY;

	    /* Check what Crtc1 thinks this should be. */
	    ret = Crtc1->FBValid(Crtc1, VirtualX, VirtualY, pScrn->bitsPerPixel,
				 rhdPtr->FbScanoutStart, rhdPtr->FbScanoutSize, &Pitch1);
	    if (ret != MODE_OK) {
		xf86DrvMsg(pScrn->scrnIndex, X_INFO, "%s rejected mode \"%s\" "
			   "(%dx%d): %s\n", Crtc1->Name, Mode->name,
			   Mode->HDisplay, Mode->VDisplay,
			   RHDModeStatusToString(ret));
		goto rejected;
	    }

	    /* Check what Crtc2 thinks this should be. */
	    ret = Crtc2->FBValid(Crtc2, VirtualX, VirtualY, pScrn->bitsPerPixel,
				 rhdPtr->FbScanoutStart, rhdPtr->FbScanoutSize, &Pitch2);
	    if (ret != MODE_OK) {
		xf86DrvMsg(pScrn->scrnIndex, X_INFO, "%s rejected mode \"%s\" "
			   "(%dx%d): %s\n", Crtc2->Name, Mode->name,
			   Mode->HDisplay, Mode->VDisplay,
			   RHDModeStatusToString(ret));
		goto rejected;
	    }

	    /* when needed, check whether this matches our 2D engine as well. */
	    if (rhdPtr->AccelMethod >= RHD_ACCEL_XAA)
		if (rhdPtr->ChipSet < RHD_R600) /* badly abstracted, i know */
#if 0
		    if (!R5xx2DFBValid(rhdPtr, VirtualX, VirtualY,
				       pScrn->bitsPerPixel, rhdPtr->FbScanoutStart,
				       rhdPtr->FbScanoutSize, Pitch1)) {
			xf86DrvMsg(pScrn->scrnIndex, X_INFO, "2D acceleration "
				   "rejected mode \"%s\" (%dx%d).\n",
				   Mode->name, Mode->HDisplay, Mode->VDisplay);
			goto rejected;
		    }
#endif
	    /* mode is perfectly valid FB wise */
	    Mode = Mode->next;
	    pScrn->virtualX = VirtualX;
	    pScrn->virtualY = VirtualY;
	    pScrn->displayWidth = Pitch1;
	    continue;

	rejected:
	    Next = Mode->next;
	    Modes = rhdModeDelete(Modes, Mode);
	    Mode = Next;
	} else
	    Mode = Mode->next;
    }
}

/*
 * RandR entry point: fixup per Crtc and Output (in RandR speech)
 * Due to misconceptions we might end up fixing *everything* here.
 */
int
RHDRRModeFixup(ScrnInfoPtr pScrn, DisplayModePtr Mode, struct rhdCrtc *Crtc,
	       struct rhdConnector *Connector, struct rhdOutput *Output,
	       struct rhdMonitor *Monitor, Bool ScaledMode)
{
    RHDPtr rhdPtr = RHDPTR(pScrn);
    int i, Status;

    ASSERT(Connector);
    ASSERT(Output);
    RHDFUNC(Output);

    Status = rhdModeSanity(rhdPtr, Mode);
    if (Status != MODE_OK)
        return Status;

    rhdModeFillOutCrtcValues(Mode);

    if (!ScaledMode) {
    /* We don't want to loop around this forever */
	for (i = 0; i < RHD_MODE_VALIDATION_LOOPS; i++) {
        Mode->CrtcHAdjusted = FALSE;
        Mode->CrtcVAdjusted = FALSE;

	/* Sanitize */
        Status = rhdModeCrtcSanity(Mode);
        if (Status != MODE_OK)
            return Status;
        if (Mode->CrtcHAdjusted || Mode->CrtcVAdjusted)
            continue;

	if (Crtc) {
	    /* Check FB */
	    Status = Crtc->FBValid(Crtc, Mode->CrtcHDisplay, Mode->CrtcVDisplay,
				   pScrn->bitsPerPixel, rhdPtr->FbScanoutStart,
				   rhdPtr->FbScanoutSize, NULL);
	    if (Status != MODE_OK)
		return Status;

		if (Crtc->ScaleValid) {
		    Status = Crtc->ScaleValid(Crtc, RHD_CRTC_SCALE_TYPE_NONE, Mode, NULL);
	    if (Status != MODE_OK)
		return Status;
	    if (Mode->CrtcHAdjusted || Mode->CrtcVAdjusted)
		continue;
		}

		/* Check Crtc */
		Status = Crtc->ModeValid(Crtc, Mode);
		if (Status != MODE_OK)
		    return Status;
		if (Mode->CrtcHAdjusted || Mode->CrtcVAdjusted)
		    continue;

	    /* Check PLL */
	    if (Crtc->PLL->Valid) {
		Status = Crtc->PLL->Valid(Crtc->PLL, Mode->Clock);
		if (Status != MODE_OK)
		    return Status;
		if (Mode->CrtcHAdjusted || Mode->CrtcVAdjusted)
		    continue;
	    }
	}

	/* Check Output */
	Status = Output->ModeValid(Output, Mode);
	if (Status != MODE_OK)
	    return Status;
	if (Mode->CrtcHAdjusted || Mode->CrtcVAdjusted)
	    continue;

	/* Check the monitor attached to this output */
	if (Connector->Monitor)
	    Status = rhdMonitorValid(Connector->Monitor, Mode);
	if (Status != MODE_OK)
	    return Status;
	if (Mode->CrtcHAdjusted || Mode->CrtcVAdjusted)
	    continue;

	/* Seems to be good */
	break;
    }

	if (i == RHD_MODE_VALIDATION_LOOPS) {
	/* Mode has been bouncing around for ages, on adjustments */
	xf86DrvMsg(Output->scrnIndex, X_ERROR,
		   "%s: Mode \"%s\" (%dx%d:%3.1fMhz) was thrown around"
		   " for too long.\n", __func__, Mode->name,
		   Mode->HDisplay, Mode->VDisplay, Mode->Clock/1000.0);
	return MODE_ERROR;
    }

    /* throw them at the configured monitor */
    if (Monitor) {
	Status = rhdMonitorValid(Monitor, Mode);
	if (Status != MODE_OK)
	    return Status;
    }

    } else {
	if (Crtc) {
	    Status = rhdModeValidateCrtc(Crtc, Mode, VALIDATE_SCALE_FROM);
	    if (Status != MODE_OK)
		return Status;
	}
    }

    /* Did we set up virtual resolution already? */
    if ((pScrn->virtualX > 0) && (pScrn->virtualY > 0)) {
        if (pScrn->virtualX < Mode->CrtcHDisplay)
            return MODE_VIRTUAL_X;
        if (pScrn->virtualY < Mode->CrtcVDisplay)
            return MODE_VIRTUAL_Y;
    }

    return MODE_OK;
}
#endif

/*
 * RHDRRValidateScaledToMode(): like RHDValidateScaledMode() - but we cannot validate against a CRTC
 * as this isn't known when this function is called. So at least validate against the 'output' here.
 */
int
RHDRRValidateScaledToMode(struct rhdOutput *Output, DisplayModePtr Mode)
{
    RHDPtr rhdPtr = RHDPTRI(Output);
    int Status;
    int i;

    RHDFUNC(Output);

    Status = rhdModeSanity(rhdPtr, Mode);
    if (Status != MODE_OK)
        return Status;

    rhdModeFillOutCrtcValues(Mode);

    for (i = 0; i < RHD_MODE_VALIDATION_LOOPS; i++) {

        Mode->CrtcHAdjusted = FALSE;
        Mode->CrtcVAdjusted = FALSE;

        Status = rhdModeCrtcSanity(Mode);
        if (Status != MODE_OK)
            return Status;
        if (Mode->CrtcHAdjusted || Mode->CrtcVAdjusted)
            continue;

	/* Check the output */
	Status = Output->ModeValid(Output, Mode);
	if (Status != MODE_OK)
	    return Status;
	if (Mode->CrtcHAdjusted || Mode->CrtcVAdjusted)
	    continue; /* restart. */

	/* Check the monitor attached to this output */
	if (Output->Connector && Output->Connector->Monitor)
	    Status = rhdMonitorValid(Output->Connector->Monitor, Mode);
	if (Status != MODE_OK)
	    return Status;
	if (Mode->CrtcHAdjusted || Mode->CrtcVAdjusted)
	    continue;

	break;
    }

    if (i == RHD_MODE_VALIDATION_LOOPS) {
	/* Mode has been bouncing around for ages, on adjustments */
	xf86DrvMsg(Output->scrnIndex, X_ERROR,
		   "%s: Mode \"%s\" (%dx%d:%3.1fMhz) was thrown around"
		   " for too long.\n", __func__, Mode->name,
		   Mode->HDisplay, Mode->VDisplay, Mode->Clock/1000.0);
	return MODE_ERROR;
    }

    /* Do we want to also validate against a configured monitor? */
    if (rhdPtr->ConfigMonitor) {
	Status = rhdMonitorValid(rhdPtr->ConfigMonitor, Mode);
	if (Status != MODE_OK)
	    return Status;
    }

    return MODE_OK;
}

/*
 * RHDSynthModes(): synthesize CVT modes for well known resolutions.
 * For now we assume we want reduced modes only.
 */
void
RHDSynthModes(int scrnIndex, DisplayModePtr Mode)
{
    RHDPtr rhdPtr = (RHDPtr)(scrnIndex);
    DisplayModePtr Tmp;
    unsigned int i;

    struct resolution{
	int x;
	int y;
    } resolution_list[] = {
	{  320,  200 },  /* CGA */
	{  320,  240 },  /* QVGA */
	{  640,  480 },  /* VGA */
	{  720,  480 },  /* NTSC */
	{  854,  480 },  /* WVGA */
	{  768,  576 },  /* PAL */
	{  800,  600 },  /* SVGA */
	{ 1024,  768 },  /* XGA */
	{ 1152,  768 },
	{ 1280,  720 },  /* HD720 */
	{ 1280,  960 },
	{ 1280,  854 },
	{ 1280,  960 },
	{ 1280, 1024 },  /* SXGA */
	{ 1440,  960 },
 	{ 1400, 1050 },  /* SXGA+ */
	{ 1680, 1050 },  /* WSXGA+ */
	{ 1600, 1200 },  /* UXGA */
	{ 1920, 1080 },  /* HD1080 */
	{ 1920, 1200 },  /* WUXGA */
	{ 2048, 1536 },  /* QXGA */
	{ 2560, 1600 },  /* WQXGA */
	{ 2560, 2048 }   /* QSXGA */
    };

    RHDFUNC(pScrn);

    for (i = 0; i < (sizeof(resolution_list) / sizeof(struct resolution)); i++) {
	/*
	 *  chances are that the native mode of a display is a CVT mode with 60 Hz.
	 *  This will make RandR share the CRTC which is undesireable for scaling.
	 *  This we 'tweak' the frequency to be slightly higher.
	 *  Don't tell me it's ugly - I know this already.
	 */
	Tmp = RHDCVTMode(resolution_list[i].x, resolution_list[i].y, 60.5, TRUE, FALSE);
	Tmp->status = MODE_OK;
	rhdModeFillOutCrtcValues(Tmp);
	xfree(Tmp->name);
	Tmp->name = xnfalloc(20);
	snprintf(Tmp->name, 20, "%ix%iScaled",resolution_list[i].x,resolution_list[i].y);
	Tmp->type = M_T_BUILTIN;
  //  if (rhdPtr->verbosity > 6) {
  //      xf86DrvMsg(scrnIndex, X_INFO, "%s: Adding Modeline ",__func__);
  //      RHDPrintModeline(Tmp);
  //  }
	RHDModesAdd(Mode, Tmp);
    }
}
