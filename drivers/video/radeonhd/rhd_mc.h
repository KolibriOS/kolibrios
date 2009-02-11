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
#ifndef RHD_MC_H
# define RHD_MC_H

extern void RHDMCInit(RHDPtr rhdPtr);
extern void RHDMCDestroy(RHDPtr rhdPtr);
extern void RHDMCSave(RHDPtr rhdPtr);
extern void RHDMCRestore(RHDPtr rhdPtr);
extern Bool RHDMCSetupFBLocation(RHDPtr rhdPtr, CARD64 Address, CARD32 Size);
extern Bool RHDMCIdleWait(RHDPtr rhdPtr, CARD32 count);
extern void RHDMCTuneAccessForDisplay(RHDPtr rhdPtr, int Crtc, DisplayModePtr Mode,
				DisplayModePtr ScaledToMode);
extern CARD64 RHDMCGetFBLocation(RHDPtr rhdPtr, CARD32 *size);

extern Bool RHD_MC_IGP_SideportMemoryPresent(RHDPtr rhdPtr);

#endif /* RHD_MC_H */
