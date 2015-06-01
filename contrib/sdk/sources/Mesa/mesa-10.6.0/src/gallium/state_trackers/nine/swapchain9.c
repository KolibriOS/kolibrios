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

#include "swapchain9.h"
#include "surface9.h"
#include "device9.h"

#include "nine_helpers.h"
#include "nine_pipe.h"
#include "nine_dump.h"

#include "util/u_inlines.h"
#include "util/u_surface.h"
#include "hud/hud_context.h"
#include "state_tracker/drm_driver.h"

#include "threadpool.h"

#define DBG_CHANNEL DBG_SWAPCHAIN

#define UNTESTED(n) DBG("UNTESTED point %d. Please tell if it worked\n", n)

HRESULT
NineSwapChain9_ctor( struct NineSwapChain9 *This,
                     struct NineUnknownParams *pParams,
                     BOOL implicit,
                     ID3DPresent *pPresent,
                     D3DPRESENT_PARAMETERS *pPresentationParameters,
                     struct d3dadapter9_context *pCTX,
                     HWND hFocusWindow,
                     D3DDISPLAYMODEEX *mode )
{
    HRESULT hr;

    DBG("This=%p pDevice=%p pPresent=%p pCTX=%p hFocusWindow=%p\n",
        This, pParams->device, pPresent, pCTX, hFocusWindow);

    hr = NineUnknown_ctor(&This->base, pParams);
    if (FAILED(hr))
        return hr;

    This->screen = NineDevice9_GetScreen(This->base.device);
    This->pipe = NineDevice9_GetPipe(This->base.device);
    This->cso = NineDevice9_GetCSO(This->base.device);
    This->implicit = implicit;
    This->actx = pCTX;
    This->present = pPresent;
    This->mode = NULL;

    ID3DPresent_AddRef(pPresent);

    if (!pPresentationParameters->hDeviceWindow)
        pPresentationParameters->hDeviceWindow = hFocusWindow;

    This->rendering_done = FALSE;
    This->pool = NULL;
    return NineSwapChain9_Resize(This, pPresentationParameters, mode);
}

static D3DWindowBuffer *
D3DWindowBuffer_create(struct NineSwapChain9 *This,
                       struct pipe_resource *resource,
                       int depth)
{
    D3DWindowBuffer *ret;
    struct winsys_handle whandle;
    int stride, dmaBufFd;

    memset(&whandle, 0, sizeof(whandle));
    whandle.type = DRM_API_HANDLE_TYPE_FD;
    This->screen->resource_get_handle(This->screen, resource, &whandle);
    stride = whandle.stride;
    dmaBufFd = whandle.handle;
    ID3DPresent_NewD3DWindowBufferFromDmaBuf(This->present,
                                             dmaBufFd,
                                             resource->width0,
                                             resource->height0,
                                             stride,
                                             depth,
                                             32,
                                             &ret);
    return ret;
}

HRESULT
NineSwapChain9_Resize( struct NineSwapChain9 *This,
                       D3DPRESENT_PARAMETERS *pParams,
                       D3DDISPLAYMODEEX *mode )
{
    struct NineDevice9 *pDevice = This->base.device;
    struct NineSurface9 **bufs;
    D3DSURFACE_DESC desc;
    HRESULT hr;
    struct pipe_resource *resource, tmplt;
    enum pipe_format pf;
    BOOL has_present_buffers = FALSE;
    int depth;
    unsigned i, oldBufferCount, newBufferCount;

    DBG("This=%p pParams=%p\n", This, pParams);
    user_assert(pParams != NULL, E_POINTER);

    DBG("pParams(%p):\n"
        "BackBufferWidth: %u\n"
        "BackBufferHeight: %u\n"
        "BackBufferFormat: %s\n"
        "BackBufferCount: %u\n"
        "MultiSampleType: %u\n"
        "MultiSampleQuality: %u\n"
        "SwapEffect: %u\n"
        "hDeviceWindow: %p\n"
        "Windowed: %i\n"
        "EnableAutoDepthStencil: %i\n"
        "AutoDepthStencilFormat: %s\n"
        "Flags: %s\n"
        "FullScreen_RefreshRateInHz: %u\n"
        "PresentationInterval: %x\n", pParams,
        pParams->BackBufferWidth, pParams->BackBufferHeight,
        d3dformat_to_string(pParams->BackBufferFormat),
        pParams->BackBufferCount,
        pParams->MultiSampleType, pParams->MultiSampleQuality,
        pParams->SwapEffect, pParams->hDeviceWindow, pParams->Windowed,
        pParams->EnableAutoDepthStencil,
        d3dformat_to_string(pParams->AutoDepthStencilFormat),
        nine_D3DPRESENTFLAG_to_str(pParams->Flags),
        pParams->FullScreen_RefreshRateInHz,
        pParams->PresentationInterval);

    if (pParams->SwapEffect == D3DSWAPEFFECT_COPY &&
        pParams->BackBufferCount > 1) {
        pParams->BackBufferCount = 1;
    }

    if (pParams->BackBufferCount > 3) {
        pParams->BackBufferCount = 3;
    }

    if (pParams->BackBufferCount == 0) {
        pParams->BackBufferCount = 1;
    }

    if (pParams->BackBufferFormat == D3DFMT_UNKNOWN) {
        pParams->BackBufferFormat = D3DFMT_A8R8G8B8;
    }

    This->desired_fences = This->actx->throttling ? This->actx->throttling_value + 1 : 0;
    /* +1 because we add the fence of the current buffer before popping an old one */
    if (This->desired_fences > DRI_SWAP_FENCES_MAX)
        This->desired_fences = DRI_SWAP_FENCES_MAX;

    if (This->actx->vblank_mode == 0)
        pParams->PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    else if (This->actx->vblank_mode == 3)
        pParams->PresentationInterval = D3DPRESENT_INTERVAL_ONE;

    if (mode && This->mode) {
        *(This->mode) = *mode;
    } else if (mode) {
        This->mode = malloc(sizeof(D3DDISPLAYMODEEX));
        memcpy(This->mode, mode, sizeof(D3DDISPLAYMODEEX));
    } else {
        free(This->mode);
        This->mode = NULL;
    }

    /* Note: It is the role of the backend to fill if necessary
     * BackBufferWidth and BackBufferHeight */
    ID3DPresent_SetPresentParameters(This->present, pParams, This->mode);

    /* When we have flip behaviour, d3d9 expects we get back the screen buffer when we flip.
     * Here we don't get back the initial content of the screen. To emulate the behaviour
     * we allocate an additional buffer */
    oldBufferCount = This->params.BackBufferCount ?
                     (This->params.BackBufferCount +
                      (This->params.SwapEffect != D3DSWAPEFFECT_COPY)) : 0;
    newBufferCount = pParams->BackBufferCount +
                     (pParams->SwapEffect != D3DSWAPEFFECT_COPY);

    pf = d3d9_to_pipe_format_checked(This->screen, pParams->BackBufferFormat,
                                     PIPE_TEXTURE_2D, pParams->MultiSampleType,
                                     PIPE_BIND_RENDER_TARGET, FALSE);

    if (This->actx->linear_framebuffer ||
        (pf != PIPE_FORMAT_B8G8R8X8_UNORM &&
        pf != PIPE_FORMAT_B8G8R8A8_UNORM) ||
        pParams->SwapEffect != D3DSWAPEFFECT_DISCARD ||
        pParams->MultiSampleType >= 2 ||
        (This->actx->ref && This->actx->ref == This->screen))
        has_present_buffers = TRUE;

    /* Note: the buffer depth has to match the window depth.
     * In practice, ARGB buffers can be used with windows
     * of depth 24. Windows of depth 32 are extremely rare.
     * So even if the buffer is ARGB, say it is depth 24.
     * It is common practice, for example that's how
     * glamor implements depth 24.
     * TODO: handle windows with other depths. Not possible in the short term.
     * For example 16 bits.*/
    depth = 24;

    tmplt.target = PIPE_TEXTURE_2D;
    tmplt.width0 = pParams->BackBufferWidth;
    tmplt.height0 = pParams->BackBufferHeight;
    tmplt.depth0 = 1;
    tmplt.last_level = 0;
    tmplt.array_size = 1;
    tmplt.usage = PIPE_USAGE_DEFAULT;
    tmplt.flags = 0;

    desc.Type = D3DRTYPE_SURFACE;
    desc.Pool = D3DPOOL_DEFAULT;
    desc.MultiSampleType = pParams->MultiSampleType;
    desc.MultiSampleQuality = 0;
    desc.Width = pParams->BackBufferWidth;
    desc.Height = pParams->BackBufferHeight;

    if (This->pool) {
        _mesa_threadpool_destroy(This->pool);
        This->pool = NULL;
    }
    This->enable_threadpool = This->actx->thread_submit && (pParams->SwapEffect != D3DSWAPEFFECT_COPY);
    if (This->enable_threadpool)
        This->pool = _mesa_threadpool_create();
    if (!This->pool)
        This->enable_threadpool = FALSE;

    This->tasks = REALLOC(This->tasks,
                          oldBufferCount * sizeof(struct threadpool_task *),
                          newBufferCount * sizeof(struct threadpool_task *));
    memset(This->tasks, 0, newBufferCount * sizeof(struct threadpool_task *));

    for (i = 0; i < oldBufferCount; i++) {
        ID3DPresent_DestroyD3DWindowBuffer(This->present, This->present_handles[i]);
        This->present_handles[i] = NULL;
        if (This->present_buffers)
            pipe_resource_reference(&(This->present_buffers[i]), NULL);
    }

    if (!has_present_buffers && This->present_buffers) {
        FREE(This->present_buffers);
        This->present_buffers = NULL;
    }

    if (newBufferCount != oldBufferCount) {
        for (i = newBufferCount; i < oldBufferCount;
             ++i)
            NineUnknown_Detach(NineUnknown(This->buffers[i]));

        bufs = REALLOC(This->buffers,
                       oldBufferCount * sizeof(This->buffers[0]),
                       newBufferCount * sizeof(This->buffers[0]));
        if (!bufs)
            return E_OUTOFMEMORY;
        This->buffers = bufs;
        This->present_handles = REALLOC(This->present_handles,
                                        oldBufferCount * sizeof(D3DWindowBuffer *),
                                        newBufferCount * sizeof(D3DWindowBuffer *));
        for (i = oldBufferCount; i < newBufferCount; ++i) {
            This->buffers[i] = NULL;
            This->present_handles[i] = NULL;
        }
    }

    if (has_present_buffers &&
        (newBufferCount != oldBufferCount || !This->present_buffers)) {
        This->present_buffers = REALLOC(This->present_buffers,
                                        This->present_buffers == NULL ? 0 :
                                        oldBufferCount * sizeof(struct pipe_resource *),
                                        newBufferCount * sizeof(struct pipe_resource *));
        memset(This->present_buffers, 0, newBufferCount * sizeof(struct pipe_resource *));
    }

    for (i = 0; i < newBufferCount; ++i) {
        tmplt.bind = PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_TRANSFER_READ |
                     PIPE_BIND_TRANSFER_WRITE | PIPE_BIND_RENDER_TARGET;
        tmplt.nr_samples = pParams->MultiSampleType;
        if (!has_present_buffers)
            tmplt.bind |= PIPE_BIND_SHARED | PIPE_BIND_SCANOUT | PIPE_BIND_DISPLAY_TARGET;
        tmplt.format = d3d9_to_pipe_format_checked(This->screen,
                                                   pParams->BackBufferFormat,
                                                   PIPE_TEXTURE_2D,
                                                   tmplt.nr_samples,
                                                   tmplt.bind, FALSE);
        if (tmplt.format == PIPE_FORMAT_NONE)
            return D3DERR_INVALIDCALL;
        resource = This->screen->resource_create(This->screen, &tmplt);
        if (!resource) {
            DBG("Failed to create pipe_resource.\n");
            return D3DERR_OUTOFVIDEOMEMORY;
        }
        if (pParams->Flags & D3DPRESENTFLAG_LOCKABLE_BACKBUFFER)
            resource->flags |= NINE_RESOURCE_FLAG_LOCKABLE;
        if (This->buffers[i]) {
            NineSurface9_SetResourceResize(This->buffers[i], resource);
            if (has_present_buffers)
                pipe_resource_reference(&resource, NULL);
        } else {
            desc.Format = pParams->BackBufferFormat;
            desc.Usage = D3DUSAGE_RENDERTARGET;
            hr = NineSurface9_new(pDevice, NineUnknown(This), resource, NULL, 0,
                                  0, 0, &desc, &This->buffers[i]);
            if (has_present_buffers)
                pipe_resource_reference(&resource, NULL);
            if (FAILED(hr)) {
                DBG("Failed to create RT surface.\n");
                return hr;
            }
            This->buffers[i]->base.base.forward = FALSE;
        }
        if (has_present_buffers) {
            tmplt.format = PIPE_FORMAT_B8G8R8X8_UNORM;
            tmplt.bind = PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_SHARED | PIPE_BIND_SCANOUT | PIPE_BIND_DISPLAY_TARGET;
            tmplt.nr_samples = 0;
            if (This->actx->linear_framebuffer)
                tmplt.bind |= PIPE_BIND_LINEAR;
            if (pParams->SwapEffect != D3DSWAPEFFECT_DISCARD)
                tmplt.bind |= PIPE_BIND_RENDER_TARGET;
            resource = This->screen->resource_create(This->screen, &tmplt);
            pipe_resource_reference(&(This->present_buffers[i]), resource);
        }
        This->present_handles[i] = D3DWindowBuffer_create(This, resource, depth);
        pipe_resource_reference(&resource, NULL);
    }
    if (pParams->EnableAutoDepthStencil) {
        tmplt.bind = d3d9_get_pipe_depth_format_bindings(pParams->AutoDepthStencilFormat);
        /* Checking the d3d9 depth format for texture support indicates the app if it can use
         * the format for shadow mapping or texturing. If the check returns true, then the app
         * is allowed to use this functionnality, so try first to create the buffer
         * with PIPE_BIND_SAMPLER_VIEW. If the format can't be created with it, try without.
         * If it fails with PIPE_BIND_SAMPLER_VIEW, then the app check for texture support
         * would fail too, so we are fine. */
        tmplt.bind |= PIPE_BIND_SAMPLER_VIEW;
        tmplt.nr_samples = pParams->MultiSampleType;
        tmplt.format = d3d9_to_pipe_format_checked(This->screen,
                                                   pParams->AutoDepthStencilFormat,
                                                   PIPE_TEXTURE_2D,
                                                   tmplt.nr_samples,
                                                   tmplt.bind,
                                                   FALSE);
        if (tmplt.format == PIPE_FORMAT_NONE) {
            tmplt.bind &= ~PIPE_BIND_SAMPLER_VIEW;
            tmplt.format = d3d9_to_pipe_format_checked(This->screen,
                                                       pParams->AutoDepthStencilFormat,
                                                       PIPE_TEXTURE_2D,
                                                       tmplt.nr_samples,
                                                       tmplt.bind,
                                                       FALSE);
        }

        if (tmplt.format == PIPE_FORMAT_NONE)
            return D3DERR_INVALIDCALL;

        resource = This->screen->resource_create(This->screen, &tmplt);
        if (!resource) {
            DBG("Failed to create pipe_resource for depth buffer.\n");
            return D3DERR_OUTOFVIDEOMEMORY;
        }
        if (This->zsbuf) {
            NineSurface9_SetResourceResize(This->zsbuf, resource);
            pipe_resource_reference(&resource, NULL);
        } else {
            /* XXX wine thinks the container of this should be the device */
            desc.Format = pParams->AutoDepthStencilFormat;
            desc.Usage = D3DUSAGE_DEPTHSTENCIL;
            hr = NineSurface9_new(pDevice, NineUnknown(pDevice), resource, NULL, 0,
                                  0, 0, &desc, &This->zsbuf);
            pipe_resource_reference(&resource, NULL);
            if (FAILED(hr)) {
                DBG("Failed to create ZS surface.\n");
                return hr;
            }
            This->zsbuf->base.base.forward = FALSE;
        }
    }

    This->params = *pParams;

    return D3D_OK;
}

/* Throttling: code adapted from the dri state tracker */

/**
 * swap_fences_pop_front - pull a fence from the throttle queue
 *
 * If the throttle queue is filled to the desired number of fences,
 * pull fences off the queue until the number is less than the desired
 * number of fences, and return the last fence pulled.
 */
static struct pipe_fence_handle *
swap_fences_pop_front(struct NineSwapChain9 *This)
{
    struct pipe_screen *screen = This->screen;
    struct pipe_fence_handle *fence = NULL;

    if (This->desired_fences == 0)
        return NULL;

    if (This->cur_fences >= This->desired_fences) {
        screen->fence_reference(screen, &fence, This->swap_fences[This->tail]);
        screen->fence_reference(screen, &This->swap_fences[This->tail++], NULL);
        This->tail &= DRI_SWAP_FENCES_MASK;
        --This->cur_fences;
    }
    return fence;
}


/**
 * swap_fences_see_front - same than swap_fences_pop_front without
 * pulling
 *
 */

static struct pipe_fence_handle *
swap_fences_see_front(struct NineSwapChain9 *This)
{
    struct pipe_screen *screen = This->screen;
    struct pipe_fence_handle *fence = NULL;

    if (This->desired_fences == 0)
        return NULL;

    if (This->cur_fences >= This->desired_fences) {
        screen->fence_reference(screen, &fence, This->swap_fences[This->tail]);
    }
    return fence;
}


/**
 * swap_fences_push_back - push a fence onto the throttle queue at the back
 *
 * push a fence onto the throttle queue and pull fences of the queue
 * so that the desired number of fences are on the queue.
 */
static void
swap_fences_push_back(struct NineSwapChain9 *This,
                      struct pipe_fence_handle *fence)
{
    struct pipe_screen *screen = This->screen;

    if (!fence || This->desired_fences == 0)
        return;

    while(This->cur_fences == This->desired_fences)
        swap_fences_pop_front(This);

    This->cur_fences++;
    screen->fence_reference(screen, &This->swap_fences[This->head++],
                            fence);
    This->head &= DRI_SWAP_FENCES_MASK;
}


/**
 * swap_fences_unref - empty the throttle queue
 *
 * pulls fences of the throttle queue until it is empty.
 */
static void
swap_fences_unref(struct NineSwapChain9 *This)
{
    struct pipe_screen *screen = This->screen;

    while(This->cur_fences) {
        screen->fence_reference(screen, &This->swap_fences[This->tail++], NULL);
        This->tail &= DRI_SWAP_FENCES_MASK;
        --This->cur_fences;
    }
}

void
NineSwapChain9_dtor( struct NineSwapChain9 *This )
{
    unsigned i;

    DBG("This=%p\n", This);

    if (This->pool)
        _mesa_threadpool_destroy(This->pool);

    if (This->buffers) {
        for (i = 0; i < This->params.BackBufferCount; i++) {
            NineUnknown_Release(NineUnknown(This->buffers[i]));
            ID3DPresent_DestroyD3DWindowBuffer(This->present, This->present_handles[i]);
            if (This->present_buffers)
                pipe_resource_reference(&(This->present_buffers[i]), NULL);
        }
        FREE(This->buffers);
        FREE(This->present_buffers);
    }
    if (This->zsbuf)
        NineUnknown_Destroy(NineUnknown(This->zsbuf));

    if (This->present)
        ID3DPresent_Release(This->present);

    swap_fences_unref(This);
    NineUnknown_dtor(&This->base);
}

static void
create_present_buffer( struct NineSwapChain9 *This,
                       unsigned int width, unsigned int height,
                       struct pipe_resource **resource,
                       D3DWindowBuffer **present_handle)
{
    struct pipe_resource tmplt;

    tmplt.target = PIPE_TEXTURE_2D;
    tmplt.width0 = width;
    tmplt.height0 = height;
    tmplt.depth0 = 1;
    tmplt.last_level = 0;
    tmplt.array_size = 1;
    tmplt.usage = PIPE_USAGE_DEFAULT;
    tmplt.flags = 0;
    tmplt.format = PIPE_FORMAT_B8G8R8X8_UNORM;
    tmplt.bind = PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_TRANSFER_READ |
                 PIPE_BIND_TRANSFER_WRITE | PIPE_BIND_RENDER_TARGET |
                 PIPE_BIND_SHARED | PIPE_BIND_SCANOUT | PIPE_BIND_DISPLAY_TARGET;
    tmplt.nr_samples = 0;
    if (This->actx->linear_framebuffer)
        tmplt.bind |= PIPE_BIND_LINEAR;
    *resource = This->screen->resource_create(This->screen, &tmplt);

    *present_handle = D3DWindowBuffer_create(This, *resource, 24);
}

static void
handle_draw_cursor_and_hud( struct NineSwapChain9 *This, struct pipe_resource *resource)
{
    struct NineDevice9 *device = This->base.device;
    struct pipe_blit_info blit;

    if (device->cursor.software && device->cursor.visible && device->cursor.w) {
        memset(&blit, 0, sizeof(blit));
        blit.src.resource = device->cursor.image;
        blit.src.level = 0;
        blit.src.format = device->cursor.image->format;
        blit.src.box.x = 0;
        blit.src.box.y = 0;
        blit.src.box.z = 0;
        blit.src.box.depth = 1;
        blit.src.box.width = device->cursor.w;
        blit.src.box.height = device->cursor.h;

        blit.dst.resource = resource;
        blit.dst.level = 0;
        blit.dst.format = resource->format;
        blit.dst.box.z = 0;
        blit.dst.box.depth = 1;

        blit.mask = PIPE_MASK_RGBA;
        blit.filter = PIPE_TEX_FILTER_NEAREST;
        blit.scissor_enable = FALSE;

        ID3DPresent_GetCursorPos(This->present, &device->cursor.pos);

        /* NOTE: blit messes up when box.x + box.width < 0, fix driver */
        blit.dst.box.x = MAX2(device->cursor.pos.x, 0) - device->cursor.hotspot.x;
        blit.dst.box.y = MAX2(device->cursor.pos.y, 0) - device->cursor.hotspot.y;
        blit.dst.box.width = blit.src.box.width;
        blit.dst.box.height = blit.src.box.height;

        DBG("Blitting cursor(%ux%u) to (%i,%i).\n",
            blit.src.box.width, blit.src.box.height,
            blit.dst.box.x, blit.dst.box.y);

        This->pipe->blit(This->pipe, &blit);
    }

    if (device->hud && resource) {
        hud_draw(device->hud, resource); /* XXX: no offset */
        /* HUD doesn't clobber stipple */
        NineDevice9_RestoreNonCSOState(device, ~0x2);
    }
}

struct end_present_struct {
    struct pipe_screen *screen;
    struct pipe_fence_handle *fence_to_wait;
    ID3DPresent *present;
    D3DWindowBuffer *present_handle;
    HWND hDestWindowOverride;
};

static void work_present(void *data)
{
    struct end_present_struct *work = data;
    if (work->fence_to_wait) {
        (void) work->screen->fence_finish(work->screen, work->fence_to_wait, PIPE_TIMEOUT_INFINITE);
        work->screen->fence_reference(work->screen, &(work->fence_to_wait), NULL);
    }
    ID3DPresent_PresentBuffer(work->present, work->present_handle, work->hDestWindowOverride, NULL, NULL, NULL, 0);
    free(work);
}

static void pend_present(struct NineSwapChain9 *This,
                         HWND hDestWindowOverride)
{
    struct end_present_struct *work = calloc(1, sizeof(struct end_present_struct));

    work->screen = This->screen;
    work->fence_to_wait = swap_fences_pop_front(This);
    work->present = This->present;
    work->present_handle = This->present_handles[0];
    work->hDestWindowOverride = hDestWindowOverride;
    This->tasks[0] = _mesa_threadpool_queue_task(This->pool, work_present, work);

    return;
}

static INLINE HRESULT
present( struct NineSwapChain9 *This,
         const RECT *pSourceRect,
         const RECT *pDestRect,
         HWND hDestWindowOverride,
         const RGNDATA *pDirtyRegion,
         DWORD dwFlags )
{
    struct pipe_resource *resource;
    struct pipe_fence_handle *fence;
    HRESULT hr;
    struct pipe_blit_info blit;

    DBG("present: This=%p pSourceRect=%p pDestRect=%p "
        "pDirtyRegion=%p hDestWindowOverride=%p"
        "dwFlags=%d resource=%p\n",
        This, pSourceRect, pDestRect, pDirtyRegion,
        hDestWindowOverride, (int)dwFlags, This->buffers[0]->base.resource);

    if (pSourceRect)
        DBG("pSourceRect = (%u..%u)x(%u..%u)\n",
            pSourceRect->left, pSourceRect->right,
            pSourceRect->top, pSourceRect->bottom);
    if (pDestRect)
        DBG("pDestRect = (%u..%u)x(%u..%u)\n",
            pDestRect->left, pDestRect->right,
            pDestRect->top, pDestRect->bottom);

    /* TODO: in the case the source and destination rect have different size:
     * We need to allocate a new buffer, and do a blit to it to resize.
     * We can't use the present_buffer for that since when we created it,
     * we couldn't guess which size would have been needed.
     * If pDestRect or pSourceRect is null, we have to check the sizes
     * from the source size, and the destination window size.
     * In this case, either resize rngdata, or pass NULL instead
     */
    /* Note: This->buffers[0]->level should always be 0 */

    if (This->rendering_done)
        goto bypass_rendering;

    resource = This->buffers[0]->base.resource;

    if (This->params.SwapEffect == D3DSWAPEFFECT_DISCARD)
        handle_draw_cursor_and_hud(This, resource);

    if (This->present_buffers) {
        memset(&blit, 0, sizeof(blit));
        blit.src.resource = resource;
        blit.src.level = 0;
        blit.src.format = resource->format;
        blit.src.box.z = 0;
        blit.src.box.depth = 1;
        blit.src.box.x = 0;
        blit.src.box.y = 0;
        blit.src.box.width = resource->width0;
        blit.src.box.height = resource->height0;

        resource = This->present_buffers[0];

        blit.dst.resource = resource;
        blit.dst.level = 0;
        blit.dst.format = resource->format;
        blit.dst.box.z = 0;
        blit.dst.box.depth = 1;
        blit.dst.box.x = 0;
        blit.dst.box.y = 0;
        blit.dst.box.width = resource->width0;
        blit.dst.box.height = resource->height0;

        blit.mask = PIPE_MASK_RGBA;
        blit.filter = PIPE_TEX_FILTER_NEAREST;
        blit.scissor_enable = FALSE;

        This->pipe->blit(This->pipe, &blit);
    }

    if (This->params.SwapEffect != D3DSWAPEFFECT_DISCARD)
        handle_draw_cursor_and_hud(This, resource);

    fence = NULL;
    This->pipe->flush(This->pipe, &fence, PIPE_FLUSH_END_OF_FRAME);
    if (fence) {
        swap_fences_push_back(This, fence);
        This->screen->fence_reference(This->screen, &fence, NULL);
    }

    This->rendering_done = TRUE;
bypass_rendering:

    if (dwFlags & D3DPRESENT_DONOTWAIT) {
        UNTESTED(2);
        BOOL still_draw = FALSE;
        fence = swap_fences_see_front(This);
        if (fence) {
            still_draw = !This->screen->fence_signalled(This->screen, fence);
            This->screen->fence_reference(This->screen, &fence, NULL);
        }
        if (still_draw)
            return D3DERR_WASSTILLDRAWING;
    }

    if (This->present_buffers)
        resource = This->present_buffers[0];
    else
        resource = This->buffers[0]->base.resource;
    This->pipe->flush_resource(This->pipe, resource);

    if (!This->enable_threadpool) {
        This->tasks[0]=NULL;
        fence = swap_fences_pop_front(This);
        if (fence) {
            (void) This->screen->fence_finish(This->screen, fence, PIPE_TIMEOUT_INFINITE);
            This->screen->fence_reference(This->screen, &fence, NULL);
        }

        hr = ID3DPresent_PresentBuffer(This->present, This->present_handles[0], hDestWindowOverride, pSourceRect, pDestRect, pDirtyRegion, dwFlags);

        if (FAILED(hr)) { UNTESTED(3);return hr; }
    } else {
        pend_present(This, hDestWindowOverride);
    }
    This->rendering_done = FALSE;

    return D3D_OK;
}

HRESULT WINAPI
NineSwapChain9_Present( struct NineSwapChain9 *This,
                        const RECT *pSourceRect,
                        const RECT *pDestRect,
                        HWND hDestWindowOverride,
                        const RGNDATA *pDirtyRegion,
                        DWORD dwFlags )
{
    struct pipe_resource *res = NULL;
    D3DWindowBuffer *handle_temp;
    struct threadpool_task *task_temp;
    int i;
    HRESULT hr = present(This, pSourceRect, pDestRect,
                         hDestWindowOverride, pDirtyRegion, dwFlags);

    DBG("This=%p pSourceRect=%p pDestRect=%p hDestWindowOverride=%p "
        "pDirtyRegion=%p dwFlags=%d\n",
        This, pSourceRect, pDestRect, hDestWindowOverride,
        pDirtyRegion,dwFlags);

    if (hr == D3DERR_WASSTILLDRAWING)
        return hr;

    switch (This->params.SwapEffect) {
        case D3DSWAPEFFECT_FLIP:
            UNTESTED(4);
        case D3DSWAPEFFECT_DISCARD:
            /* rotate the queue */;
            pipe_resource_reference(&res, This->buffers[0]->base.resource);
            for (i = 1; i <= This->params.BackBufferCount; i++) {
                NineSurface9_SetResourceResize(This->buffers[i - 1],
                                               This->buffers[i]->base.resource);
            }
            NineSurface9_SetResourceResize(
                This->buffers[This->params.BackBufferCount], res);
            pipe_resource_reference(&res, NULL);

            if (This->present_buffers) {
                pipe_resource_reference(&res, This->present_buffers[0]);
                for (i = 1; i <= This->params.BackBufferCount; i++)
                    pipe_resource_reference(&(This->present_buffers[i-1]), This->present_buffers[i]);
                pipe_resource_reference(&(This->present_buffers[This->params.BackBufferCount]), res);
                pipe_resource_reference(&res, NULL);
            }

            handle_temp = This->present_handles[0];
            for (i = 1; i <= This->params.BackBufferCount; i++) {
                This->present_handles[i-1] = This->present_handles[i];
            }
            This->present_handles[This->params.BackBufferCount] = handle_temp;
            task_temp = This->tasks[0];
            for (i = 1; i <= This->params.BackBufferCount; i++) {
                This->tasks[i-1] = This->tasks[i];
            }
            This->tasks[This->params.BackBufferCount] = task_temp;
            break;

        case D3DSWAPEFFECT_COPY:
            UNTESTED(5);
            /* do nothing */
            break;

        case D3DSWAPEFFECT_OVERLAY:
            /* XXX not implemented */
            break;

        case D3DSWAPEFFECT_FLIPEX:
            /* XXX not implemented */
            break;
    }

    if (This->tasks[0])
        _mesa_threadpool_wait_for_task(This->pool, &(This->tasks[0]));

    ID3DPresent_WaitBufferReleased(This->present, This->present_handles[0]);

    This->base.device->state.changed.group |= NINE_STATE_FB;
    nine_update_state(This->base.device, NINE_STATE_FB);

    return hr;
}

HRESULT WINAPI
NineSwapChain9_GetFrontBufferData( struct NineSwapChain9 *This,
                                   IDirect3DSurface9 *pDestSurface )
{
    struct NineSurface9 *dest_surface = NineSurface9(pDestSurface);
    struct NineDevice9 *pDevice = This->base.device;
    unsigned int width, height;
    struct pipe_resource *temp_resource;
    struct NineSurface9 *temp_surface;
    D3DWindowBuffer *temp_handle;
    D3DSURFACE_DESC desc;
    HRESULT hr;

    DBG("GetFrontBufferData: This=%p pDestSurface=%p\n",
        This, pDestSurface);

    width = dest_surface->desc.Width;
    height = dest_surface->desc.Height;

    /* Note: front window size and destination size are supposed
     * to match. However it's not very clear what should get taken in Windowed
     * mode. It may need a fix */
    create_present_buffer(This, width, height, &temp_resource, &temp_handle);

    desc.Type = D3DRTYPE_SURFACE;
    desc.Pool = D3DPOOL_DEFAULT;
    desc.MultiSampleType = D3DMULTISAMPLE_NONE;
    desc.MultiSampleQuality = 0;
    desc.Width = width;
    desc.Height = height;
    /* NineSurface9_CopySurface needs same format. */
    desc.Format = dest_surface->desc.Format;
    desc.Usage = D3DUSAGE_RENDERTARGET;
    hr = NineSurface9_new(pDevice, NineUnknown(This), temp_resource, NULL, 0,
                          0, 0, &desc, &temp_surface);
    pipe_resource_reference(&temp_resource, NULL);
    if (FAILED(hr)) {
        DBG("Failed to create temp FrontBuffer surface.\n");
        return hr;
    }

    ID3DPresent_FrontBufferCopy(This->present, temp_handle);

    NineSurface9_CopySurface(dest_surface, temp_surface, NULL, NULL);

    ID3DPresent_DestroyD3DWindowBuffer(This->present, temp_handle);
    NineUnknown_Destroy(NineUnknown(temp_surface));

    return D3D_OK;
}

HRESULT WINAPI
NineSwapChain9_GetBackBuffer( struct NineSwapChain9 *This,
                              UINT iBackBuffer,
                              D3DBACKBUFFER_TYPE Type,
                              IDirect3DSurface9 **ppBackBuffer )
{
    DBG("GetBackBuffer: This=%p iBackBuffer=%d Type=%d ppBackBuffer=%p\n",
        This, iBackBuffer, Type, ppBackBuffer);
    (void)user_error(Type == D3DBACKBUFFER_TYPE_MONO);
    user_assert(iBackBuffer < This->params.BackBufferCount, D3DERR_INVALIDCALL);
    user_assert(ppBackBuffer != NULL, E_POINTER);

    NineUnknown_AddRef(NineUnknown(This->buffers[iBackBuffer]));
    *ppBackBuffer = (IDirect3DSurface9 *)This->buffers[iBackBuffer];
    return D3D_OK;
}

HRESULT WINAPI
NineSwapChain9_GetRasterStatus( struct NineSwapChain9 *This,
                                D3DRASTER_STATUS *pRasterStatus )
{
    DBG("GetRasterStatus: This=%p pRasterStatus=%p\n",
        This, pRasterStatus);
    user_assert(pRasterStatus != NULL, E_POINTER);
    return ID3DPresent_GetRasterStatus(This->present, pRasterStatus);
}

HRESULT WINAPI
NineSwapChain9_GetDisplayMode( struct NineSwapChain9 *This,
                               D3DDISPLAYMODE *pMode )
{
    D3DDISPLAYMODEEX mode;
    D3DDISPLAYROTATION rot;
    HRESULT hr;

    DBG("GetDisplayMode: This=%p pMode=%p\n",
        This, pMode);
    user_assert(pMode != NULL, E_POINTER);

    hr = ID3DPresent_GetDisplayMode(This->present, &mode, &rot);
    if (SUCCEEDED(hr)) {
        pMode->Width = mode.Width;
        pMode->Height = mode.Height;
        pMode->RefreshRate = mode.RefreshRate;
        pMode->Format = mode.Format;
    }
    return hr;
}

HRESULT WINAPI
NineSwapChain9_GetPresentParameters( struct NineSwapChain9 *This,
                                     D3DPRESENT_PARAMETERS *pPresentationParameters )
{
    DBG("GetPresentParameters: This=%p pPresentationParameters=%p\n",
        This, pPresentationParameters);
    user_assert(pPresentationParameters != NULL, E_POINTER);
    *pPresentationParameters = This->params;
    return D3D_OK;
}

IDirect3DSwapChain9Vtbl NineSwapChain9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_Release,
    (void *)NineSwapChain9_Present,
    (void *)NineSwapChain9_GetFrontBufferData,
    (void *)NineSwapChain9_GetBackBuffer,
    (void *)NineSwapChain9_GetRasterStatus,
    (void *)NineSwapChain9_GetDisplayMode,
    (void *)NineUnknown_GetDevice, /* actually part of SwapChain9 iface */
    (void *)NineSwapChain9_GetPresentParameters
};

static const GUID *NineSwapChain9_IIDs[] = {
    &IID_IDirect3DSwapChain9,
    &IID_IUnknown,
    NULL
};

HRESULT
NineSwapChain9_new( struct NineDevice9 *pDevice,
                    BOOL implicit,
                    ID3DPresent *pPresent,
                    D3DPRESENT_PARAMETERS *pPresentationParameters,
                    struct d3dadapter9_context *pCTX,
                    HWND hFocusWindow,
                    struct NineSwapChain9 **ppOut )
{
    NINE_DEVICE_CHILD_NEW(SwapChain9, ppOut, pDevice, /* args */
                          implicit, pPresent, pPresentationParameters,
                          pCTX, hFocusWindow, NULL);
}
