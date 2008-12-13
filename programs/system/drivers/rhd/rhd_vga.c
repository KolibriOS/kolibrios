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

#include "xf86.h"
#if HAVE_XF86_ANSIC_H
# include "xf86_ansic.h"
#endif

#include "rhd.h"
#include "rhd_vga.h"
#include "rhd_regs.h"
#include "rhd_mc.h"

/*
 *
 */
void
RHDVGAInit(RHDPtr rhdPtr)
{
  static struct rhdVGA VGA;

  /* Check whether one of our VGA bits is set */
  if (!(_RHDRegRead(rhdPtr, VGA_RENDER_CONTROL) & 0x00030000) &&
       (_RHDRegRead(rhdPtr, VGA_HDP_CONTROL) & 0x00000010) &&
      !(_RHDRegRead(rhdPtr, D1VGA_CONTROL) & 0x00000001) &&
      !(_RHDRegRead(rhdPtr, D2VGA_CONTROL) & 0x00000001))
	return;

  DBG(dbgprintf("Detected VGA mode.\n"));

  VGA.Stored = FALSE;
  rhdPtr->VGA = &VGA;
}

#if 0
/*
 *
 */
void
RHDVGASave(RHDPtr rhdPtr)
{
  struct rhdVGA *VGA = &rhdPtr->VGA;

  VGA->Render_Control = _RHDRegRead(rhdPtr, VGA_RENDER_CONTROL);
  VGA->Mode_Control = _RHDRegRead(rhdPtr, VGA_MODE_CONTROL);
  VGA->HDP_Control = _RHDRegRead(rhdPtr, VGA_HDP_CONTROL);
  VGA->D1_Control = _RHDRegRead(rhdPtr, D1VGA_CONTROL);
  VGA->D2_Control = _RHDRegRead(rhdPtr, D2VGA_CONTROL);

    /* Store our VGA FB */
  VGA->FBOffset =
    _RHDRegRead(rhdPtr, VGA_MEMORY_BASE_ADDRESS) - rhdPtr->FbIntAddress;

  /* Could be that the VGA internal address no longer is pointing to what
     we know as our FB memory, in which case we should give up cleanly. */
  if (VGA->FBOffset < (unsigned) (rhdPtr->videoRam * 1024)) {
    VGA->FBSize = 256 * 1024;
    VGA->FB = calloc(VGA->FBSize, 1);
	if (VGA->FB)
	    memcpy(VGA->FB, ((CARD8 *) rhdPtr->FbBase) + VGA->FBOffset,
		   VGA->FBSize);
	else {
      dbgprintf("%s: Failed to allocate"
                " space for storing the VGA framebuffer.\n", __func__);
	    VGA->FBSize = 0;
	    VGA->FB = NULL;
	}
    } else {
  dbgprintf("%s: VGA FB Offset (0x%08X) is "
            "out of range of the Cards Internal FB Address (0x%08X)\n",
            __func__, (int) RHDRegRead(rhdPtr, VGA_MEMORY_BASE_ADDRESS),
		   rhdPtr->FbIntAddress);
	VGA->FBOffset = 0xFFFFFFFF;
	VGA->FBSize = 0;
	VGA->FB = NULL;
    }

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
  dbgprintf("%s: trying to restore uninitialized values.\n", __func__);
	return;
    }

    if (VGA->FB)
	memcpy(((CARD8 *) rhdPtr->FbBase) + VGA->FBOffset,
	       VGA->FB, VGA->FBSize);

    RHDRegWrite(rhdPtr, VGA_RENDER_CONTROL, VGA->Render_Control);
    RHDRegWrite(rhdPtr, VGA_MODE_CONTROL, VGA->Mode_Control);
    RHDRegWrite(rhdPtr, VGA_HDP_CONTROL, VGA->HDP_Control);
    RHDRegWrite(rhdPtr, D1VGA_CONTROL, VGA->D1_Control);
    RHDRegWrite(rhdPtr, D2VGA_CONTROL, VGA->D2_Control);
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
    _RHDRegMask(rhdPtr, D1VGA_CONTROL, 0, 0x00000001);
    _RHDRegMask(rhdPtr, D2VGA_CONTROL, 0, 0x00000001);
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
      free(VGA->FB);
    rhdPtr->VGA=NULL;
}






