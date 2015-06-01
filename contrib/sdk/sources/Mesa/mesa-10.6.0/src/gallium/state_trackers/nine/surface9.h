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

#ifndef _NINE_SURFACE9_H_
#define _NINE_SURFACE9_H_

#include "resource9.h"

#include "pipe/p_state.h"
#include "util/list.h"
#include "util/u_rect.h"
#include "util/u_inlines.h"

struct NineSurface9
{
    struct NineResource9 base;

    /* G3D state */
    struct pipe_context *pipe;
    struct pipe_transfer *transfer;
    struct pipe_surface *surface[2]; /* created on-demand (linear, sRGB) */
    int lock_count;
    uint8_t texture; /* rtype of container BaseTex or 0 */

    /* resource description */
    unsigned level;        /* refers to the pipe_resource (SetLOD !) */
    unsigned level_actual; /* refers to the NineTexture */
    unsigned layer;
    D3DSURFACE_DESC desc;

    uint8_t *data; /* system memory backing */
    unsigned stride; /* for system memory backing */
};
static INLINE struct NineSurface9 *
NineSurface9( void *data )
{
    return (struct NineSurface9 *)data;
}

HRESULT
NineSurface9_new( struct NineDevice9 *pDevice,
                  struct NineUnknown *pContainer,
                  struct pipe_resource *pResource,
                  void *user_buffer,
                  uint8_t TextureType, /* 0 if pContainer isn't BaseTexure9 */
                  unsigned Level,
                  unsigned Layer,
                  D3DSURFACE_DESC *pDesc,
                  struct NineSurface9 **ppOut );

HRESULT
NineSurface9_ctor( struct NineSurface9 *This,
                   struct NineUnknownParams *pParams,
                   struct NineUnknown *pContainer,
                   struct pipe_resource *pResource,
                   void *user_buffer,
                   uint8_t TextureType,
                   unsigned Level,
                   unsigned Layer,
                   D3DSURFACE_DESC *pDesc );

void
NineSurface9_dtor( struct NineSurface9 *This );

/*** Nine private ***/

void
NineSurface9_MarkContainerDirty( struct NineSurface9 *This );

struct pipe_surface *
NineSurface9_CreatePipeSurface( struct NineSurface9 *This, const int sRGB );

static INLINE struct pipe_surface *
NineSurface9_GetSurface( struct NineSurface9 *This, int sRGB )
{
    if (This->surface[sRGB])
        return This->surface[sRGB];
    return NineSurface9_CreatePipeSurface(This, sRGB);
}

static INLINE struct pipe_resource *
NineSurface9_GetResource( struct NineSurface9 *This )
{
    return This->base.resource;
}

static INLINE void
NineSurface9_SetResource( struct NineSurface9 *This,
                          struct pipe_resource *resource, unsigned level )
{
    This->level = level;
    pipe_resource_reference(&This->base.resource, resource);
    pipe_surface_reference(&This->surface[0], NULL);
    pipe_surface_reference(&This->surface[1], NULL);
}

void
NineSurface9_SetResourceResize( struct NineSurface9 *This,
                                struct pipe_resource *resource );

void
NineSurface9_AddDirtyRect( struct NineSurface9 *This,
                           const struct pipe_box *box );

HRESULT
NineSurface9_UploadSelf( struct NineSurface9 *This,
                         const struct pipe_box *damaged );

HRESULT
NineSurface9_CopySurface( struct NineSurface9 *This,
                          struct NineSurface9 *From,
                          const POINT *pDestPoint,
                          const RECT *pSourceRect );

static INLINE boolean
NineSurface9_IsOffscreenPlain (struct NineSurface9 *This )
{
    return This->base.usage == 0 && !This->texture;
}

#ifdef DEBUG
void
NineSurface9_Dump( struct NineSurface9 *This );
#else
static INLINE void
NineSurface9_Dump( struct NineSurface9 *This ) { }
#endif

/*** Direct3D public ***/

HRESULT WINAPI
NineSurface9_GetContainer( struct NineSurface9 *This,
                           REFIID riid,
                           void **ppContainer );

HRESULT WINAPI
NineSurface9_GetDesc( struct NineSurface9 *This,
                      D3DSURFACE_DESC *pDesc );

HRESULT WINAPI
NineSurface9_LockRect( struct NineSurface9 *This,
                       D3DLOCKED_RECT *pLockedRect,
                       const RECT *pRect,
                       DWORD Flags );

HRESULT WINAPI
NineSurface9_UnlockRect( struct NineSurface9 *This );

HRESULT WINAPI
NineSurface9_GetDC( struct NineSurface9 *This,
                    HDC *phdc );

HRESULT WINAPI
NineSurface9_ReleaseDC( struct NineSurface9 *This,
                        HDC hdc );

#endif /* _NINE_SURFACE9_H_ */
