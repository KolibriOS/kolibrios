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

#ifndef _RHD_MODES_H
#define _RHD_MODES_H

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
#define MODE_NO_ENCODER     0x13 + RHD_MODE_STATUS

/*
 * In case this isn't in xf86str.h yet.
 */

#define M_T_BUILTIN 0x01        /* built-in mode */

#ifndef M_T_PREFERRED
#define M_T_PREFERRED 0x08
#endif
#ifndef M_T_DRIVER
#define M_T_DRIVER 0x40
#endif

DisplayModePtr RHDCVTMode(int HDisplay, int VDisplay, float VRefresh,
			  Bool Reduced, Bool Interlaced);
void RHDPrintModeline(DisplayModePtr mode);
DisplayModePtr RHDModesAdd(DisplayModePtr Modes, DisplayModePtr Additions);
const char *RHDModeStatusToString(int Status);

DisplayModePtr RHDModesPoolCreate(ScrnInfoPtr pScrn, Bool Silent);
void RHDModesAttach(ScrnInfoPtr pScrn, DisplayModePtr Modes);
DisplayModePtr RHDModeCopy(DisplayModePtr Mode);

Bool RHDGetVirtualFromConfig(ScrnInfoPtr pScrn);
void RHDGetVirtualFromModesAndFilter(ScrnInfoPtr pScrn, DisplayModePtr Modes, Bool Silent);

int RHDRRModeFixup(ScrnInfoPtr pScrn, DisplayModePtr Mode, struct rhdCrtc *Crtc,
       struct rhdConnector *Connector, struct rhdOutput *Output,
		   struct rhdMonitor *Monitor, Bool ScaledMode);
int RHDValidateScaledToMode(struct rhdCrtc *Crtc, DisplayModePtr Mode);
int RHDRRValidateScaledToMode(struct rhdOutput *Output, DisplayModePtr Mode);
void RHDSynthModes(int scrnIndex, DisplayModePtr Mode);

#endif /* _RHD_MODES_H */
