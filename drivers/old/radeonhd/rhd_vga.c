/*
 * Copyright 2007-2008  Luc Verhaegen <lverhaegen@novell.com>
 * Copyright 2007-2008  Matthias Hopf <mhopf@novell.com>
 * Copyright 2007-2008  Egbert Eich   <eich@novell.com>
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
#endif

#include "rhd.h"
#include "rhd_vga.h"
#include "rhd_regs.h"
#include "rhd_mc.h"

struct rhdVGA {
    Bool Stored;

    CARD32 FBOffset;
    CARD8 *FB;
    int FBSize; /* 256kB */

    CARD32 Render_Control;
    CARD32 Mode_Control;
    CARD32 HDP_Control;
    CARD32 D1_Control;
    CARD32 D2_Control;
};

/*
 *
 */
void
RHDVGAInit(RHDPtr rhdPtr)
{
    struct rhdVGA *VGA;

    RHDFUNC(rhdPtr);

  /* Check whether one of our VGA bits is set */
  if (!(_RHDRegRead(rhdPtr, VGA_RENDER_CONTROL) & 0x00030000) &&
       (_RHDRegRead(rhdPtr, VGA_HDP_CONTROL) & 0x00000010) &&
      !(_RHDRegRead(rhdPtr, D1VGA_CONTROL) & 0x00000001) &&
      !(_RHDRegRead(rhdPtr, D2VGA_CONTROL) & 0x00000001))
	return;

    xf86DrvMsg(rhdPtr->scrnIndex, X_INFO, "Detected VGA mode.\n");

    VGA = xnfcalloc(sizeof(struct rhdVGA), 1);
    VGA->Stored = FALSE;

    rhdPtr->VGA = VGA;
}

#if 0
/*
 *
 */
static CARD32
rhdVGAFBOffsetGet(RHDPtr rhdPtr)
{
    CARD32 FBSize, VGAFBOffset, VGAFBSize = 256 * 1024;
    CARD64 FBAddress = RHDMCGetFBLocation(rhdPtr, &FBSize);
    CARD64 VGAFBAddress = RHDRegRead(rhdPtr, VGA_MEMORY_BASE_ADDRESS);

    if (VGAFBAddress < FBAddress)
	return 0xFFFFFFFF;

    if ((VGAFBAddress + VGAFBSize) > (FBAddress + FBSize))
	return 0xFFFFFFFF;

    VGAFBOffset = VGAFBAddress - FBAddress; /* < FBSize, so 32bit */

    if ((VGAFBOffset + VGAFBSize) >= rhdPtr->FbMapSize)
	return 0xFFFFFFFF;

    return VGAFBOffset;
}

/*
 * This is (usually) ok, as VGASave is called after the memory has been mapped,
 * but before the MC is set up. So the use of RHDMCGetFBLocation is correct in
 * rhdVGAFBOffsetGet.
 */
static void
rhdVGASaveFB(RHDPtr rhdPtr)
{
    struct rhdVGA *VGA = rhdPtr->VGA;

    ASSERT(rhdPtr->FbBase);

    RHDFUNC(rhdPtr);

    VGA->FBOffset = rhdVGAFBOffsetGet(rhdPtr);

    if (VGA->FBOffset == 0xFFFFFFFF) {
	xf86DrvMsg(rhdPtr->scrnIndex, X_WARNING, "%s: Unable to access the VGA "
		   "framebuffer (0x%08X)\n", __func__,
		   (unsigned int) RHDRegRead(rhdPtr, VGA_MEMORY_BASE_ADDRESS));
	if (VGA->FB)
	    xfree(VGA->FB);
	VGA->FB = NULL;
	VGA->FBSize = 0;
	return;
    }

    VGA->FBSize = 256 * 1024;

    RHDDebug(rhdPtr->scrnIndex, "%s: VGA FB Offset 0x%08X [0x%08X]\n",
	     __func__, VGA->FBOffset, VGA->FBSize);

    if (!VGA->FB)
	VGA->FB = xcalloc(VGA->FBSize, 1);

	if (VGA->FB)
	    memcpy(VGA->FB, ((CARD8 *) rhdPtr->FbBase) + VGA->FBOffset,
		   VGA->FBSize);
	else {
	xf86DrvMsg(rhdPtr->scrnIndex, X_WARNING, "%s: Failed to allocate"
                " space for storing the VGA framebuffer.\n", __func__);
	VGA->FBOffset = 0xFFFFFFFF;
	VGA->FBSize = 0;
    }
}

/*
 *
 */
void
RHDVGASave(RHDPtr rhdPtr)
{
    struct rhdVGA *VGA = rhdPtr->VGA;

    RHDFUNC(rhdPtr);

    if (!VGA)
	return; /* We don't need to warn , this is intended use */

    VGA->Render_Control = RHDRegRead(rhdPtr, VGA_RENDER_CONTROL);
    VGA->Mode_Control = RHDRegRead(rhdPtr, VGA_MODE_CONTROL);
    VGA->HDP_Control = RHDRegRead(rhdPtr, VGA_HDP_CONTROL);
    VGA->D1_Control = RHDRegRead(rhdPtr, D1VGA_CONTROL);
    VGA->D2_Control = RHDRegRead(rhdPtr, D2VGA_CONTROL);

    rhdVGASaveFB(rhdPtr);
    VGA->Stored = TRUE;
}

/*
 *
 */
void
RHDVGARestore(RHDPtr rhdPtr)
{
    struct rhdVGA *VGA = rhdPtr->VGA;

    RHDFUNC(rhdPtr);

    if (!VGA)
	return; /* We don't need to warn , this is intended use */

    if (!VGA->Stored) {
	xf86DrvMsg(rhdPtr->scrnIndex, X_ERROR,
		   "%s: trying to restore uninitialized values.\n", __func__);
	return;
    }

    if (VGA->FB)
	memcpy(((CARD8 *) rhdPtr->FbBase) + VGA->FBOffset, VGA->FB,
	       VGA->FBSize);

    RHDRegWrite(rhdPtr, VGA_RENDER_CONTROL, VGA->Render_Control);
    RHDRegWrite(rhdPtr, VGA_MODE_CONTROL, VGA->Mode_Control);
    RHDRegWrite(rhdPtr, VGA_HDP_CONTROL, VGA->HDP_Control);
    RHDRegWrite(rhdPtr, D1VGA_CONTROL, VGA->D1_Control);
    RHDRegWrite(rhdPtr, D2VGA_CONTROL, VGA->D2_Control);
    RHD_UNSETDEBUGFLAG(rhdPtr, VGA_SETUP);
}
#endif
/*
 *
 */
void
RHDVGADisable(RHDPtr rhdPtr)
{
    RHDFUNC(rhdPtr);

    _RHDRegMask(rhdPtr, VGA_RENDER_CONTROL, 0, 0x00030000);
    _RHDRegMask(rhdPtr, VGA_MODE_CONTROL, 0, 0x00000030);
    _RHDRegMask(rhdPtr, VGA_HDP_CONTROL, 0x00010010, 0x00010010);
    RHDRegMask(rhdPtr, D1VGA_CONTROL, 0, D1VGA_MODE_ENABLE);
    RHDRegMask(rhdPtr, D2VGA_CONTROL, 0, D2VGA_MODE_ENABLE);
}

/*
 *
 */
void
RHDVGADestroy(RHDPtr rhdPtr)
{
    struct rhdVGA *VGA = rhdPtr->VGA;

    RHDFUNC(rhdPtr);

    if (!VGA)
      return; /* We don't need to warn , this is intended use */

    if (VGA->FB)
	xfree(VGA->FB);
    xfree(VGA);
}






