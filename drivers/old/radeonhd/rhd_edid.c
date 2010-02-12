/*
 * Copyright 2006-2007 Luc Verhaegen <lverhaegen@novell.com>.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#ifdef HAVE_XORG_CONFIG_H
# include <xorg-config.h>
#endif

#include "xf86.h"
#include "rhd.h"
#include "edid.h"
#include "xf86DDC.h"

#include "rhd_modes.h"
#include "rhd_monitor.h"

/*
 * TODO:
 *  - for those with access to the VESA DMT standard; review please.
 *  - swap M_T_DEFAULT for M_T_EDID_...
 */
#define MODEPREFIX(name) NULL, NULL, name, 0,M_T_DRIVER
#define MODESUFFIX   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,FALSE,FALSE,0,NULL,0,0.0,0.0

static DisplayModeRec EDIDEstablishedModes[17] = {
    { MODEPREFIX("800x600"),    40000,  800,  840,  968, 1056, 0,  600,  601,  605,  628, 0, V_PHSYNC | V_PVSYNC, MODESUFFIX }, /* 800x600@60Hz */
    { MODEPREFIX("800x600"),    36000,  800,  824,  896, 1024, 0,  600,  601,  603,  625, 0, V_PHSYNC | V_PVSYNC, MODESUFFIX }, /* 800x600@56Hz */
    { MODEPREFIX("640x480"),    31500,  640,  656,  720,  840, 0,  480,  481,  484,  500, 0, V_NHSYNC | V_NVSYNC, MODESUFFIX }, /* 640x480@75Hz */
    { MODEPREFIX("640x480"),    31500,  640,  664,  704,  832, 0,  480,  489,  491,  520, 0, V_NHSYNC | V_NVSYNC, MODESUFFIX }, /* 640x480@72Hz */
    { MODEPREFIX("640x480"),    30240,  640,  704,  768,  864, 0,  480,  483,  486,  525, 0, V_NHSYNC | V_NVSYNC, MODESUFFIX }, /* 640x480@67Hz */
    { MODEPREFIX("640x480"),    25200,  640,  656,  752,  800, 0,  480,  490,  492,  525, 0, V_NHSYNC | V_NVSYNC, MODESUFFIX }, /* 640x480@60Hz */
    { MODEPREFIX("720x400"),    35500,  720,  738,  846,  900, 0,  400,  421,  423,  449, 0, V_NHSYNC | V_NVSYNC, MODESUFFIX }, /* 720x400@88Hz */
    { MODEPREFIX("720x400"),    28320,  720,  738,  846,  900, 0,  400,  412,  414,  449, 0, V_NHSYNC | V_PVSYNC, MODESUFFIX }, /* 720x400@70Hz */
    { MODEPREFIX("1280x1024"), 135000, 1280, 1296, 1440, 1688, 0, 1024, 1025, 1028, 1066, 0, V_PHSYNC | V_PVSYNC, MODESUFFIX }, /* 1280x1024@75Hz */
    { MODEPREFIX("1024x768"),   78800, 1024, 1040, 1136, 1312, 0,  768,  769,  772,  800, 0, V_PHSYNC | V_PVSYNC, MODESUFFIX }, /* 1024x768@75Hz */
    { MODEPREFIX("1024x768"),   75000, 1024, 1048, 1184, 1328, 0,  768,  771,  777,  806, 0, V_NHSYNC | V_NVSYNC, MODESUFFIX }, /* 1024x768@70Hz */
    { MODEPREFIX("1024x768"),   65000, 1024, 1048, 1184, 1344, 0,  768,  771,  777,  806, 0, V_NHSYNC | V_NVSYNC, MODESUFFIX }, /* 1024x768@60Hz */
    { MODEPREFIX("1024x768"),   44900, 1024, 1032, 1208, 1264, 0,  768,  768,  776,  817, 0, V_PHSYNC | V_PVSYNC | V_INTERLACE, MODESUFFIX }, /* 1024x768@43Hz */
    { MODEPREFIX("832x624"),    57284,  832,  864,  928, 1152, 0,  624,  625,  628,  667, 0, V_NHSYNC | V_NVSYNC, MODESUFFIX }, /* 832x624@75Hz */
    { MODEPREFIX("800x600"),    49500,  800,  816,  896, 1056, 0,  600,  601,  604,  625, 0, V_PHSYNC | V_PVSYNC, MODESUFFIX }, /* 800x600@75Hz */
    { MODEPREFIX("800x600"),    50000,  800,  856,  976, 1040, 0,  600,  637,  643,  666, 0, V_PHSYNC | V_PVSYNC, MODESUFFIX }, /* 800x600@72Hz */
    { MODEPREFIX("1152x864"),  108000, 1152, 1216, 1344, 1600, 0,  864,  865,  868,  900, 0, V_PHSYNC | V_PVSYNC, MODESUFFIX }, /* 1152x864@75Hz */
};

static DisplayModePtr
EDIDModesFromEstablished(int scrnIndex, struct established_timings *timing)
{
    DisplayModePtr Modes = NULL, Mode = NULL;
    CARD32 bits = (timing->t1) | (timing->t2 << 8) |
        ((timing->t_manu & 0x80) << 9);
    int i;

    for (i = 0; i < 17; i++)
        if (bits & (0x01 << i)) {
            Mode = RHDModeCopy(&(EDIDEstablishedModes[i]));
            Modes = RHDModesAdd(Modes, Mode);
        }

    return Modes;
}

/*
 *
 */
static DisplayModePtr
EDIDModesFromStandardTiming(int scrnIndex, struct std_timings *timing)
{
    DisplayModePtr Modes = NULL, Mode = NULL;
    int i;

    for (i = 0; i < STD_TIMINGS; i++)
        if (timing[i].hsize && timing[i].vsize && timing[i].refresh) {
            Mode =  RHDCVTMode(timing[i].hsize, timing[i].vsize,
                                timing[i].refresh, FALSE, FALSE);
            Mode->type = M_T_DRIVER;
            Modes = RHDModesAdd(Modes, Mode);
        }

    return Modes;
}

/*
 *
 */
static DisplayModePtr
EDIDModeFromDetailedTiming(int scrnIndex, struct detailed_timings *timing)
{
    DisplayModePtr Mode;

    /* We don't do stereo */
    if (timing->stereo) {
        xf86DrvMsg(scrnIndex, X_INFO, "%s: Ignoring: We don't handle stereo.\n",
                   __func__);
        return NULL;
    }

    /* We only do separate sync currently */
    if (timing->sync != 0x03) {
         xf86DrvMsg(scrnIndex, X_INFO, "%s: Ignoring: We only handle separate"
                    " sync.\n", __func__);
         return NULL;
    }

    Mode = xnfalloc(sizeof(DisplayModeRec));
    memset(Mode, 0, sizeof(DisplayModeRec));

    Mode->name = xnfalloc(10); /* "1234x1234" */
    snprintf(Mode->name, 20, "%dx%d", timing->h_active, timing->v_active);

    Mode->type = M_T_DRIVER;

    Mode->Clock = timing->clock / 1000.0;

    Mode->HDisplay = timing->h_active;
    Mode->HSyncStart = timing->h_active + timing->h_sync_off;
    Mode->HSyncEnd = Mode->HSyncStart + timing->h_sync_width;
    Mode->HTotal = timing->h_active + timing->h_blanking;

    Mode->VDisplay = timing->v_active;
    Mode->VSyncStart = timing->v_active + timing->v_sync_off;
    Mode->VSyncEnd = Mode->VSyncStart + timing->v_sync_width;
    Mode->VTotal = timing->v_active + timing->v_blanking;

    /* We ignore h/v_size and h/v_border for now. */

    if (timing->interlaced)
        Mode->Flags |= V_INTERLACE;

    if (timing->misc & 0x02)
        Mode->Flags |= V_PVSYNC;
    else
        Mode->Flags |= V_NVSYNC;

    if (timing->misc & 0x01)
        Mode->Flags |= V_PHSYNC;
    else
        Mode->Flags |= V_NHSYNC;

    return Mode;
}

/*
 *
 */
static void
EDIDGuessRangesFromModes(struct rhdMonitor *Monitor, DisplayModePtr Modes)
{
    DisplayModePtr Mode = Modes;

    if (!Monitor || !Modes)
        return;

    for (Mode = Modes; Mode; Mode = Mode->next) {
	if (!Mode->HSync)
            Mode->HSync = ((float) Mode->Clock ) / ((float) Mode->HTotal);

        if (!Mode->VRefresh) {
            Mode->VRefresh = (1000.0 * ((float) Mode->Clock)) /
                ((float) (Mode->HTotal * Mode->VTotal));
	    if (Mode->Flags & V_INTERLACE)
		Mode->VRefresh *= 2.0;
	    if (Mode->Flags & V_DBLSCAN)
		Mode->VRefresh /= 2.0;
	}
    }

    if (!Monitor->numHSync) {
	/* set up the ranges for scanning through the modes */
	Monitor->numHSync = 1;
	Monitor->HSync[0].lo = 1024.0;
	Monitor->HSync[0].hi = 0.0;

	for (Mode = Modes; Mode; Mode = Mode->next) {
	    if (Mode->HSync < Monitor->HSync[0].lo)
		Monitor->HSync[0].lo = Mode->HSync;

	    if (Mode->HSync > Monitor->HSync[0].hi)
		Monitor->HSync[0].hi = Mode->HSync;
	}
    }


    if (!Monitor->numVRefresh) {
	Monitor->numVRefresh = 1;
	Monitor->VRefresh[0].lo = 1024.0;
	Monitor->VRefresh[0].hi = 0.0;

	for (Mode = Modes; Mode; Mode = Mode->next) {
	    if (Mode->VRefresh < Monitor->VRefresh[0].lo)
		Monitor->VRefresh[0].lo = Mode->VRefresh;

	    if (Mode->VRefresh > Monitor->VRefresh[0].hi)
		Monitor->VRefresh[0].hi = Mode->VRefresh;
	}
    }

    if (!Monitor->Bandwidth)
	for (Mode = Modes; Mode; Mode = Mode->next)
	    if (Mode->Clock > Monitor->Bandwidth)
		Monitor->Bandwidth = Mode->Clock;
}

/*
 * Determine whether this monitor does allow reduced blanking.
 * Do not set it to false, to allow the user to specify this too.
 */
static void
EDIDReducedAllowed(struct rhdMonitor *Monitor, DisplayModePtr Modes)
{
    DisplayModePtr Mode;

    for (Mode = Modes; Mode; Mode = Mode->next)
	if (((Mode->HTotal - Mode->HDisplay) == 160) &&
	    ((Mode->HSyncEnd - Mode->HDisplay) == 80) &&
            ((Mode->HSyncEnd - Mode->HSyncStart) == 32) &&
            ((Mode->VSyncStart - Mode->VDisplay) == 3))
	    Monitor->ReducedAllowed = TRUE;
}

/*
 * Fill out rhdMonitor with xf86MonPtr information.
 */
void
RHDMonitorEDIDSet(struct rhdMonitor *Monitor, xf86MonPtr EDID)
{
    DisplayModePtr Modes = NULL, Mode;
    int i, preferred;

    if (!Monitor || !EDID)
        return;

    /* We don't parse the detailed name yet, so use ABC-0123 */
    Monitor->Name = xnfcalloc(9, 1);
    snprintf(Monitor->Name, 9, "%s-%04X", EDID->vendor.name,
             EDID->vendor.prod_id);

    /* Add established timings */
    Mode = EDIDModesFromEstablished(Monitor->scrnIndex, &EDID->timings1);
    Modes = RHDModesAdd(Modes, Mode);

    /* Add standard timings */
    Mode = EDIDModesFromStandardTiming(Monitor->scrnIndex, EDID->timings2);
    Modes = RHDModesAdd(Modes, Mode);

    /* First DT timing preferred? */
    preferred = PREFERRED_TIMING_MODE(EDID->features.msc);

    /* Go through the detailed monitor sections */
    for (i = 0; i < DET_TIMINGS; i++)
        switch (EDID->det_mon[i].type) {
        case DS_RANGES:
            if (!Monitor->numHSync) {
                Monitor->numHSync = 1;
                Monitor->HSync[0].lo = EDID->det_mon[i].section.ranges.min_h;
                Monitor->HSync[0].hi = EDID->det_mon[i].section.ranges.max_h;
            } else
                xf86DrvMsg(Monitor->scrnIndex, X_INFO,
                           "\"%s\": keeping configured HSync.\n",
                           Monitor->Name);

            if (!Monitor->numVRefresh) {
                Monitor->numVRefresh = 1;
                Monitor->VRefresh[0].lo = EDID->det_mon[i].section.ranges.min_v;
                Monitor->VRefresh[0].hi = EDID->det_mon[i].section.ranges.max_v;
            } else
                xf86DrvMsg(Monitor->scrnIndex, X_INFO,
                           "\"%s\": keeping configured VRefresh.\n",
                           Monitor->Name);

	    if (!Monitor->Bandwidth)
		Monitor->Bandwidth =
		    EDID->det_mon[i].section.ranges.max_clock * 1000;

            break;
        case DT:
            Mode = EDIDModeFromDetailedTiming(Monitor->scrnIndex,
                   &EDID->det_mon[i].section.d_timings);
            if (Mode) {
                if (preferred) {
                    Mode->type |= M_T_PREFERRED;

		    /* also grab the DPI while we are at it */
		    Monitor->xDpi = (Mode->HDisplay * 25.4) /
			((float) EDID->det_mon[i].section.d_timings.h_size) + 0.5;
		    Monitor->yDpi = (Mode->VDisplay * 25.4) /
			((float) EDID->det_mon[i].section.d_timings.v_size) + 0.5;

		    Monitor->NativeMode = Mode;
		}
		preferred = FALSE;

                Modes = RHDModesAdd(Modes, Mode);
            }
            break;
        case DS_STD_TIMINGS:
            Mode = EDIDModesFromStandardTiming(Monitor->scrnIndex,
                                               EDID->det_mon[i].section.std_t);
            Modes = RHDModesAdd(Modes, Mode);
            break;
        case DS_NAME:
            xfree(Monitor->Name);
            Monitor->Name = xnfcalloc(13, 1);
            memcpy(Monitor->Name, EDID->det_mon[i].section.name, 13);
            break;
        default:
            break;
        }

    if (Modes) {
	EDIDGuessRangesFromModes(Monitor, Modes);
	EDIDReducedAllowed(Monitor, Modes);
        Monitor->Modes = RHDModesAdd(Monitor->Modes, Modes);
    }

    /* Calculate DPI when we still don't have this */
    if (!Monitor->xDpi || !Monitor->yDpi) {
	int HDisplay = 0, VDisplay = 0;

	for (Mode = Monitor->Modes; Mode; Mode = Mode->next) {
	    if (Mode->HDisplay > HDisplay)
		HDisplay = Mode->HDisplay;
	    if (Mode->VDisplay > VDisplay)
		VDisplay = Mode->VDisplay;
	}

	if (HDisplay && EDID->features.hsize)
	    Monitor->xDpi = (HDisplay * 2.54) / ((float) EDID->features.hsize) + 0.5;
	if (VDisplay && EDID->features.vsize)
	    Monitor->yDpi = (VDisplay * 2.54) / ((float) EDID->features.vsize) + 0.5;

	if (!Monitor->xDpi && Monitor->yDpi)
	    Monitor->xDpi = Monitor->yDpi;
	if (!Monitor->yDpi && Monitor->xDpi)
	    Monitor->yDpi = Monitor->xDpi;
    }
}
