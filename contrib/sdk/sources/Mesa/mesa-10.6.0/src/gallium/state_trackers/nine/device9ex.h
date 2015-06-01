/*
 * Copyright 2011 Joakim Sindholt <opensource@zhasha.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

#ifndef _NINE_DEVICE9EX_H_
#define _NINE_DEVICE9EX_H_

#include "device9.h"

struct NineDevice9Ex
{
    struct NineDevice9 base;
};
static INLINE struct NineDevice9Ex *
NineDevice9Ex( void *data )
{
    return (struct NineDevice9Ex *)data;
}

HRESULT
NineDevice9Ex_new( struct pipe_screen *pScreen,
                   D3DDEVICE_CREATION_PARAMETERS *pCreationParameters,
                   D3DCAPS9 *pCaps,
                   D3DPRESENT_PARAMETERS *pPresentationParameters,
                   D3DDISPLAYMODEEX *pFullscreenDisplayMode,
                   IDirect3D9Ex *pD3D9Ex,
                   ID3DPresentGroup *pPresentationGroup,
                   struct d3dadapter9_context *pCTX,
                   struct NineDevice9Ex **ppOut );

HRESULT WINAPI
NineDevice9Ex_SetConvolutionMonoKernel( struct NineDevice9Ex *This,
                                        UINT width,
                                        UINT height,
                                        float *rows,
                                        float *columns );

HRESULT WINAPI
NineDevice9Ex_ComposeRects( struct NineDevice9Ex *This,
                            IDirect3DSurface9 *pSrc,
                            IDirect3DSurface9 *pDst,
                            IDirect3DVertexBuffer9 *pSrcRectDescs,
                            UINT NumRects,
                            IDirect3DVertexBuffer9 *pDstRectDescs,
                            D3DCOMPOSERECTSOP Operation,
                            int Xoffset,
                            int Yoffset );

HRESULT WINAPI
NineDevice9Ex_PresentEx( struct NineDevice9Ex *This,
                         const RECT *pSourceRect,
                         const RECT *pDestRect,
                         HWND hDestWindowOverride,
                         const RGNDATA *pDirtyRegion,
                         DWORD dwFlags );

HRESULT WINAPI
NineDevice9Ex_GetGPUThreadPriority( struct NineDevice9Ex *This,
                                    INT *pPriority );

HRESULT WINAPI
NineDevice9Ex_SetGPUThreadPriority( struct NineDevice9Ex *This,
                                    INT Priority );

HRESULT WINAPI
NineDevice9Ex_WaitForVBlank( struct NineDevice9Ex *This,
                             UINT iSwapChain );

HRESULT WINAPI
NineDevice9Ex_CheckResourceResidency( struct NineDevice9Ex *This,
                                      IDirect3DResource9 **pResourceArray,
                                      UINT32 NumResources );

HRESULT WINAPI
NineDevice9Ex_SetMaximumFrameLatency( struct NineDevice9Ex *This,
                                      UINT MaxLatency );

HRESULT WINAPI
NineDevice9Ex_GetMaximumFrameLatency( struct NineDevice9Ex *This,
                                      UINT *pMaxLatency );

HRESULT WINAPI
NineDevice9Ex_CheckDeviceState( struct NineDevice9Ex *This,
                                HWND hDestinationWindow );

HRESULT WINAPI
NineDevice9Ex_CreateRenderTargetEx( struct NineDevice9Ex *This,
                                    UINT Width,
                                    UINT Height,
                                    D3DFORMAT Format,
                                    D3DMULTISAMPLE_TYPE MultiSample,
                                    DWORD MultisampleQuality,
                                    BOOL Lockable,
                                    IDirect3DSurface9 **ppSurface,
                                    HANDLE *pSharedHandle,
                                    DWORD Usage );

HRESULT WINAPI
NineDevice9Ex_CreateOffscreenPlainSurfaceEx( struct NineDevice9Ex *This,
                                             UINT Width,
                                             UINT Height,
                                             D3DFORMAT Format,
                                             D3DPOOL Pool,
                                             IDirect3DSurface9 **ppSurface,
                                             HANDLE *pSharedHandle,
                                             DWORD Usage );

HRESULT WINAPI
NineDevice9Ex_CreateDepthStencilSurfaceEx( struct NineDevice9Ex *This,
                                           UINT Width,
                                           UINT Height,
                                           D3DFORMAT Format,
                                           D3DMULTISAMPLE_TYPE MultiSample,
                                           DWORD MultisampleQuality,
                                           BOOL Discard,
                                           IDirect3DSurface9 **ppSurface,
                                           HANDLE *pSharedHandle,
                                           DWORD Usage );

HRESULT WINAPI
NineDevice9Ex_ResetEx( struct NineDevice9Ex *This,
                       D3DPRESENT_PARAMETERS *pPresentationParameters,
                       D3DDISPLAYMODEEX *pFullscreenDisplayMode );

HRESULT WINAPI
NineDevice9Ex_GetDisplayModeEx( struct NineDevice9Ex *This,
                                UINT iSwapChain,
                                D3DDISPLAYMODEEX *pMode,
                                D3DDISPLAYROTATION *pRotation );

#endif /* _NINE_DEVICE9EX_H_ */
