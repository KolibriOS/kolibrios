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

#include "c99_alloca.h"

#include "device9.h"
#include "surface9.h"
#include "texture9.h"
#include "nine_helpers.h"
#include "nine_pipe.h"
#include "nine_dump.h"

#include "pipe/p_state.h"
#include "pipe/p_context.h"
#include "pipe/p_screen.h"
#include "util/u_inlines.h"
#include "util/u_resource.h"

#define DBG_CHANNEL DBG_TEXTURE

static HRESULT
NineTexture9_ctor( struct NineTexture9 *This,
                   struct NineUnknownParams *pParams,
                   UINT Width, UINT Height, UINT Levels,
                   DWORD Usage,
                   D3DFORMAT Format,
                   D3DPOOL Pool,
                   HANDLE *pSharedHandle )
{
    struct pipe_screen *screen = pParams->device->screen;
    struct pipe_resource *info = &This->base.base.info;
    struct pipe_resource *resource;
    enum pipe_format pf;
    unsigned *level_offsets;
    unsigned l;
    D3DSURFACE_DESC sfdesc;
    HRESULT hr;
    void *user_buffer = NULL, *user_buffer_for_level;

    DBG("(%p) Width=%u Height=%u Levels=%u Usage=%s Format=%s Pool=%s "
        "pSharedHandle=%p\n", This, Width, Height, Levels,
        nine_D3DUSAGE_to_str(Usage),
        d3dformat_to_string(Format), nine_D3DPOOL_to_str(Pool), pSharedHandle);

    user_assert(!(Usage & D3DUSAGE_AUTOGENMIPMAP) ||
                (Pool != D3DPOOL_SYSTEMMEM && Levels <= 1), D3DERR_INVALIDCALL);

    /* TODO: implement buffer sharing (should work with cross process too)
     *
     * Gem names may have fit but they're depreciated and won't work on render-nodes.
     * One solution is to use shm buffers. We would use a /dev/shm file, fill the first
     * values to tell it is a nine buffer, the size, which function created it, etc,
     * and then it would contain the data. The handle would be a number, corresponding to
     * the file to read (/dev/shm/nine-share-4 for example would be 4).
     *
     * Wine just ignores the argument, which works only if the app creates the handle
     * and won't use it. Instead of failing, we support that situation by putting an
     * invalid handle, that we would fail to import. Please note that we don't advertise
     * the flag indicating the support for that feature, but apps seem to not care.
     */
    user_assert(!pSharedHandle ||
                Pool == D3DPOOL_SYSTEMMEM ||
                Pool == D3DPOOL_DEFAULT, D3DERR_INVALIDCALL);

    if (pSharedHandle && Pool == D3DPOOL_DEFAULT) {
        if (!*pSharedHandle) {
            DBG("Creating Texture with invalid handle. Importing will fail\n.");
            *pSharedHandle = (HANDLE)1; /* Wine would keep it NULL */
            pSharedHandle = NULL;
        } else {
            ERR("Application tries to use cross-process sharing feature. Nine "
                "doesn't support it");
            return D3DERR_INVALIDCALL;
        }
    }

    if (Usage & D3DUSAGE_AUTOGENMIPMAP)
        Levels = 0;

    pf = d3d9_to_pipe_format_checked(screen, Format, PIPE_TEXTURE_2D, 0,
                                     PIPE_BIND_SAMPLER_VIEW, FALSE);
    if (Format != D3DFMT_NULL && pf == PIPE_FORMAT_NONE)
        return D3DERR_INVALIDCALL;

    info->screen = screen;
    info->target = PIPE_TEXTURE_2D;
    info->format = pf;
    info->width0 = Width;
    info->height0 = Height;
    info->depth0 = 1;
    if (Levels)
        info->last_level = Levels - 1;
    else
        info->last_level = util_logbase2(MAX2(Width, Height));
    info->array_size = 1;
    info->nr_samples = 0;
    info->bind = PIPE_BIND_SAMPLER_VIEW;
    info->usage = PIPE_USAGE_DEFAULT;
    info->flags = 0;

    if (Usage & D3DUSAGE_RENDERTARGET)
        info->bind |= PIPE_BIND_RENDER_TARGET;
    if (Usage & D3DUSAGE_DEPTHSTENCIL)
        info->bind |= PIPE_BIND_DEPTH_STENCIL;

    if (Usage & D3DUSAGE_DYNAMIC) {
        info->usage = PIPE_USAGE_DYNAMIC;
        info->bind |=
            PIPE_BIND_TRANSFER_READ |
            PIPE_BIND_TRANSFER_WRITE;
    }

    if (Usage & D3DUSAGE_SOFTWAREPROCESSING)
        DBG("Application asked for Software Vertex Processing, "
            "but this is unimplemented\n");

    if (pSharedHandle)
        info->bind |= PIPE_BIND_SHARED;

    if (Pool == D3DPOOL_SYSTEMMEM)
        info->usage = PIPE_USAGE_STAGING;

    if (pSharedHandle && *pSharedHandle) { /* Pool == D3DPOOL_SYSTEMMEM */
        user_buffer = (void *)*pSharedHandle;
        level_offsets = alloca(sizeof(unsigned) * (info->last_level + 1));
        (void) nine_format_get_size_and_offsets(pf, level_offsets,
                                                Width, Height,
                                                info->last_level);
    } else if (Pool != D3DPOOL_DEFAULT) {
        /* TODO: For D3DUSAGE_AUTOGENMIPMAP, it is likely we only have to
         * allocate only for the first level, since it is the only lockable
         * level. Check apps don't crash if we allocate smaller buffer (some
         * apps access sublevels of texture even if they locked only first
         * level) */
        level_offsets = alloca(sizeof(unsigned) * (info->last_level + 1));
        user_buffer = MALLOC(
            nine_format_get_size_and_offsets(pf, level_offsets,
                                             Width, Height,
                                             info->last_level));
        This->managed_buffer = user_buffer;
        if (!This->managed_buffer)
            return E_OUTOFMEMORY;
    }

    This->surfaces = CALLOC(info->last_level + 1, sizeof(*This->surfaces));
    if (!This->surfaces)
        return E_OUTOFMEMORY;

    hr = NineBaseTexture9_ctor(&This->base, pParams, NULL, D3DRTYPE_TEXTURE, Format, Pool, Usage);
    if (FAILED(hr))
        return hr;
    This->base.pstype = (Height == 1) ? 1 : 0;

    /* Create all the surfaces right away.
     * They manage backing storage, and transfers (LockRect) are deferred
     * to them.
     */
    sfdesc.Format = Format;
    sfdesc.Type = D3DRTYPE_SURFACE;
    sfdesc.Usage = Usage;
    sfdesc.Pool = Pool;
    sfdesc.MultiSampleType = D3DMULTISAMPLE_NONE;
    sfdesc.MultiSampleQuality = 0;

    if (Pool == D3DPOOL_SYSTEMMEM)
        resource = NULL;
    else
        resource = This->base.base.resource;

    for (l = 0; l <= info->last_level; ++l) {
        sfdesc.Width = u_minify(Width, l);
        sfdesc.Height = u_minify(Height, l);
        /* Some apps expect the memory to be allocated in
         * continous blocks */
        user_buffer_for_level = user_buffer ? user_buffer +
            level_offsets[l] : NULL;

        hr = NineSurface9_new(This->base.base.base.device, NineUnknown(This),
                              resource, user_buffer_for_level,
                              D3DRTYPE_TEXTURE, l, 0,
                              &sfdesc, &This->surfaces[l]);
        if (FAILED(hr))
            return hr;
    }

    This->dirty_rect.depth = 1; /* widht == 0 means empty, depth stays 1 */

    if (pSharedHandle && !*pSharedHandle) {/* Pool == D3DPOOL_SYSTEMMEM */
        *pSharedHandle = This->surfaces[0]->data;
    }

    return D3D_OK;
}

static void
NineTexture9_dtor( struct NineTexture9 *This )
{
    unsigned l;

    if (This->surfaces) {
        /* The surfaces should have 0 references and be unbound now. */
        for (l = 0; l <= This->base.base.info.last_level; ++l)
            NineUnknown_Destroy(&This->surfaces[l]->base.base);
        FREE(This->surfaces);
    }

    if (This->managed_buffer)
        FREE(This->managed_buffer);

    NineBaseTexture9_dtor(&This->base);
}

HRESULT WINAPI
NineTexture9_GetLevelDesc( struct NineTexture9 *This,
                           UINT Level,
                           D3DSURFACE_DESC *pDesc )
{
    user_assert(Level <= This->base.base.info.last_level, D3DERR_INVALIDCALL);
    user_assert(Level == 0 || !(This->base.base.usage & D3DUSAGE_AUTOGENMIPMAP),
                D3DERR_INVALIDCALL);

    *pDesc = This->surfaces[Level]->desc;

    return D3D_OK;
}

HRESULT WINAPI
NineTexture9_GetSurfaceLevel( struct NineTexture9 *This,
                              UINT Level,
                              IDirect3DSurface9 **ppSurfaceLevel )
{
    user_assert(Level <= This->base.base.info.last_level, D3DERR_INVALIDCALL);
    user_assert(Level == 0 || !(This->base.base.usage & D3DUSAGE_AUTOGENMIPMAP),
                D3DERR_INVALIDCALL);

    NineUnknown_AddRef(NineUnknown(This->surfaces[Level]));
    *ppSurfaceLevel = (IDirect3DSurface9 *)This->surfaces[Level];

    return D3D_OK;
}

HRESULT WINAPI
NineTexture9_LockRect( struct NineTexture9 *This,
                       UINT Level,
                       D3DLOCKED_RECT *pLockedRect,
                       const RECT *pRect,
                       DWORD Flags )
{
    DBG("This=%p Level=%u pLockedRect=%p pRect=%p Flags=%d\n",
        This, Level, pLockedRect, pRect, Flags);

    user_assert(Level <= This->base.base.info.last_level, D3DERR_INVALIDCALL);
    user_assert(Level == 0 || !(This->base.base.usage & D3DUSAGE_AUTOGENMIPMAP),
                D3DERR_INVALIDCALL);

    return NineSurface9_LockRect(This->surfaces[Level], pLockedRect,
                                 pRect, Flags);
}

HRESULT WINAPI
NineTexture9_UnlockRect( struct NineTexture9 *This,
                         UINT Level )
{
    DBG("This=%p Level=%u\n", This, Level);

    user_assert(Level <= This->base.base.info.last_level, D3DERR_INVALIDCALL);

    return NineSurface9_UnlockRect(This->surfaces[Level]);
}

HRESULT WINAPI
NineTexture9_AddDirtyRect( struct NineTexture9 *This,
                           const RECT *pDirtyRect )
{
    DBG("This=%p pDirtyRect=%p[(%u,%u)-(%u,%u)]\n", This, pDirtyRect,
        pDirtyRect ? pDirtyRect->left : 0, pDirtyRect ? pDirtyRect->top : 0,
        pDirtyRect ? pDirtyRect->right : 0, pDirtyRect ? pDirtyRect->bottom : 0);

    /* Tracking dirty regions on DEFAULT or SYSTEMMEM resources is pointless,
     * because we always write to the final storage. Just marked it dirty in
     * case we need to generate mip maps.
     */
    if (This->base.base.pool != D3DPOOL_MANAGED) {
        if (This->base.base.usage & D3DUSAGE_AUTOGENMIPMAP)
            This->base.dirty_mip = TRUE;
        return D3D_OK;
    }
    This->base.managed.dirty = TRUE;

    BASETEX_REGISTER_UPDATE(&This->base);

    if (!pDirtyRect) {
        u_box_origin_2d(This->base.base.info.width0,
                        This->base.base.info.height0, &This->dirty_rect);
    } else {
        struct pipe_box box;
        rect_to_pipe_box_clamp(&box, pDirtyRect);
        u_box_union_2d(&This->dirty_rect, &This->dirty_rect, &box);
        (void) u_box_clip_2d(&This->dirty_rect, &This->dirty_rect,
                             This->base.base.info.width0,
                             This->base.base.info.height0);
    }
    return D3D_OK;
}

IDirect3DTexture9Vtbl NineTexture9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_Release,
    (void *)NineUnknown_GetDevice, /* actually part of Resource9 iface */
    (void *)NineResource9_SetPrivateData,
    (void *)NineResource9_GetPrivateData,
    (void *)NineResource9_FreePrivateData,
    (void *)NineResource9_SetPriority,
    (void *)NineResource9_GetPriority,
    (void *)NineBaseTexture9_PreLoad,
    (void *)NineResource9_GetType,
    (void *)NineBaseTexture9_SetLOD,
    (void *)NineBaseTexture9_GetLOD,
    (void *)NineBaseTexture9_GetLevelCount,
    (void *)NineBaseTexture9_SetAutoGenFilterType,
    (void *)NineBaseTexture9_GetAutoGenFilterType,
    (void *)NineBaseTexture9_GenerateMipSubLevels,
    (void *)NineTexture9_GetLevelDesc,
    (void *)NineTexture9_GetSurfaceLevel,
    (void *)NineTexture9_LockRect,
    (void *)NineTexture9_UnlockRect,
    (void *)NineTexture9_AddDirtyRect
};

static const GUID *NineTexture9_IIDs[] = {
    &IID_IDirect3DTexture9,
    &IID_IDirect3DBaseTexture9,
    &IID_IDirect3DResource9,
    &IID_IUnknown,
    NULL
};

HRESULT
NineTexture9_new( struct NineDevice9 *pDevice,
                  UINT Width, UINT Height, UINT Levels,
                  DWORD Usage,
                  D3DFORMAT Format,
                  D3DPOOL Pool,
                  struct NineTexture9 **ppOut,
                  HANDLE *pSharedHandle )
{
    NINE_DEVICE_CHILD_NEW(Texture9, ppOut, pDevice,
                          Width, Height, Levels,
                          Usage, Format, Pool, pSharedHandle);
}
