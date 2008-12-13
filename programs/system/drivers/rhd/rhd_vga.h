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

#ifndef _RHD_VGA_H
#define _RHD_VGA_H

struct rhdVGA {
    Bool Stored;

    CARD32 FBBase;
    CARD8 *FB;
    int FBSize; /* most cases, 256kB */

    CARD32 Render_Control;
    CARD32 Mode_Control;
    CARD32 HDP_Control;
    CARD32 D1_Control;
    CARD32 D2_Control;
};

void RHDVGAInit(RHDPtr rhdPtr);
void RHDVGASave(RHDPtr rhdPtr);
void RHDVGARestore(RHDPtr rhdPtr);
void RHDVGADisable(RHDPtr rhdPtr);
void RHDVGADestroy(RHDPtr rhdPtr);

#endif /* _RHD_PLL_H */
