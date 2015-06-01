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

#include "device9.h"
#include "stateblock9.h"
#include "surface9.h"
#include "swapchain9.h"
#include "swapchain9ex.h"
#include "indexbuffer9.h"
#include "vertexbuffer9.h"
#include "vertexdeclaration9.h"
#include "vertexshader9.h"
#include "pixelshader9.h"
#include "query9.h"
#include "texture9.h"
#include "cubetexture9.h"
#include "volumetexture9.h"
#include "nine_helpers.h"
#include "nine_pipe.h"
#include "nine_ff.h"
#include "nine_dump.h"

#include "pipe/p_screen.h"
#include "pipe/p_context.h"
#include "pipe/p_config.h"
#include "util/u_math.h"
#include "util/u_inlines.h"
#include "util/u_hash_table.h"
#include "util/u_format.h"
#include "util/u_surface.h"
#include "util/u_upload_mgr.h"
#include "hud/hud_context.h"

#include "cso_cache/cso_context.h"

#define DBG_CHANNEL DBG_DEVICE

#if defined(PIPE_CC_GCC) && (defined(PIPE_ARCH_X86) || defined(PIPE_ARCH_X86_64))

#include <fpu_control.h>

static void nine_setup_fpu()
{
    fpu_control_t c;

    _FPU_GETCW(c);
    /* clear the control word */
    c &= _FPU_RESERVED;
    /* d3d9 doc/wine tests: mask all exceptions, use single-precision
     * and round to nearest */
    c |= _FPU_MASK_IM | _FPU_MASK_DM | _FPU_MASK_ZM | _FPU_MASK_OM |
         _FPU_MASK_UM | _FPU_MASK_PM | _FPU_SINGLE | _FPU_RC_NEAREST;
    _FPU_SETCW(c);
}

#else

static void nine_setup_fpu(void)
{
    WARN_ONCE("FPU setup not supported on non-x86 platforms\n");
}

#endif

static void
NineDevice9_SetDefaultState( struct NineDevice9 *This, boolean is_reset )
{
    struct NineSurface9 *refSurf = NULL;

    DBG("This=%p is_reset=%d\n", This, (int) is_reset);

    assert(!This->is_recording);

    nine_state_set_defaults(This, &This->caps, is_reset);

    This->state.viewport.X = 0;
    This->state.viewport.Y = 0;
    This->state.viewport.Width = 0;
    This->state.viewport.Height = 0;

    This->state.scissor.minx = 0;
    This->state.scissor.miny = 0;
    This->state.scissor.maxx = 0xffff;
    This->state.scissor.maxy = 0xffff;

    if (This->nswapchains && This->swapchains[0]->params.BackBufferCount)
        refSurf = This->swapchains[0]->buffers[0];

    if (refSurf) {
        This->state.viewport.Width = refSurf->desc.Width;
        This->state.viewport.Height = refSurf->desc.Height;
        This->state.scissor.maxx = refSurf->desc.Width;
        This->state.scissor.maxy = refSurf->desc.Height;
    }

    if (This->nswapchains && This->swapchains[0]->params.EnableAutoDepthStencil)
        This->state.rs[D3DRS_ZENABLE] = TRUE;
    if (This->state.rs[D3DRS_ZENABLE])
        NineDevice9_SetDepthStencilSurface(
            This, (IDirect3DSurface9 *)This->swapchains[0]->zsbuf);
}

void
NineDevice9_RestoreNonCSOState( struct NineDevice9 *This, unsigned mask )
{
    struct pipe_context *pipe = This->pipe;

    DBG("This=%p mask=%u\n", This, mask);

    if (mask & 0x1) {
        struct pipe_constant_buffer cb;
        cb.buffer_offset = 0;

        if (This->prefer_user_constbuf) {
            cb.buffer = NULL;
            cb.user_buffer = This->state.vs_const_f;
        } else {
            cb.buffer = This->constbuf_vs;
            cb.user_buffer = NULL;
        }
        cb.buffer_size = This->vs_const_size;
        pipe->set_constant_buffer(pipe, PIPE_SHADER_VERTEX, 0, &cb);

        if (This->prefer_user_constbuf) {
            cb.user_buffer = This->state.ps_const_f;
        } else {
            cb.buffer = This->constbuf_ps;
        }
        cb.buffer_size = This->ps_const_size;
        pipe->set_constant_buffer(pipe, PIPE_SHADER_FRAGMENT, 0, &cb);
    }

    if (mask & 0x2) {
        struct pipe_poly_stipple stipple;
        memset(&stipple, ~0, sizeof(stipple));
        pipe->set_polygon_stipple(pipe, &stipple);
    }

    This->state.changed.group = NINE_STATE_ALL;
    This->state.changed.vtxbuf = (1ULL << This->caps.MaxStreams) - 1;
    This->state.changed.ucp = (1 << PIPE_MAX_CLIP_PLANES) - 1;
    This->state.changed.texture = NINE_PS_SAMPLERS_MASK | NINE_VS_SAMPLERS_MASK;
}

#define GET_PCAP(n) pScreen->get_param(pScreen, PIPE_CAP_##n)
HRESULT
NineDevice9_ctor( struct NineDevice9 *This,
                  struct NineUnknownParams *pParams,
                  struct pipe_screen *pScreen,
                  D3DDEVICE_CREATION_PARAMETERS *pCreationParameters,
                  D3DCAPS9 *pCaps,
                  D3DPRESENT_PARAMETERS *pPresentationParameters,
                  IDirect3D9 *pD3D9,
                  ID3DPresentGroup *pPresentationGroup,
                  struct d3dadapter9_context *pCTX,
                  boolean ex,
                  D3DDISPLAYMODEEX *pFullscreenDisplayMode )
{
    unsigned i;
    HRESULT hr = NineUnknown_ctor(&This->base, pParams);

    DBG("This=%p pParams=%p pScreen=%p pCreationParameters=%p pCaps=%p pPresentationParameters=%p "
        "pD3D9=%p pPresentationGroup=%p pCTX=%p ex=%d pFullscreenDisplayMode=%p\n",
        This, pParams, pScreen, pCreationParameters, pCaps, pPresentationParameters, pD3D9,
        pPresentationGroup, pCTX, (int) ex, pFullscreenDisplayMode);

    if (FAILED(hr)) { return hr; }

    list_inithead(&This->update_textures);

    This->screen = pScreen;
    This->caps = *pCaps;
    This->d3d9 = pD3D9;
    This->params = *pCreationParameters;
    This->ex = ex;
    This->present = pPresentationGroup;
    IDirect3D9_AddRef(This->d3d9);
    ID3DPresentGroup_AddRef(This->present);

    if (!(This->params.BehaviorFlags & D3DCREATE_FPU_PRESERVE))
        nine_setup_fpu();

    if (This->params.BehaviorFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING)
        DBG("Application asked full Software Vertex Processing. Ignoring.\n");
    if (This->params.BehaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING)
        DBG("Application asked mixed Software Vertex Processing. Ignoring.\n");

    This->pipe = This->screen->context_create(This->screen, NULL);
    if (!This->pipe) { return E_OUTOFMEMORY; } /* guess */

    This->cso = cso_create_context(This->pipe);
    if (!This->cso) { return E_OUTOFMEMORY; } /* also a guess */

    /* Create first, it messes up our state. */
    This->hud = hud_create(This->pipe, This->cso); /* NULL result is fine */

    /* create implicit swapchains */
    This->nswapchains = ID3DPresentGroup_GetMultiheadCount(This->present);
    This->swapchains = CALLOC(This->nswapchains,
                              sizeof(struct NineSwapChain9 *));
    if (!This->swapchains) { return E_OUTOFMEMORY; }

    for (i = 0; i < This->nswapchains; ++i) {
        ID3DPresent *present;

        hr = ID3DPresentGroup_GetPresent(This->present, i, &present);
        if (FAILED(hr))
            return hr;

        if (ex) {
            D3DDISPLAYMODEEX *mode = NULL;
            struct NineSwapChain9Ex **ret =
                (struct NineSwapChain9Ex **)&This->swapchains[i];

            if (pFullscreenDisplayMode) mode = &(pFullscreenDisplayMode[i]);
            /* when this is a Device9Ex, it should create SwapChain9Exs */
            hr = NineSwapChain9Ex_new(This, TRUE, present,
                                      &pPresentationParameters[i], pCTX,
                                      This->params.hFocusWindow, mode, ret);
        } else {
            hr = NineSwapChain9_new(This, TRUE, present,
                                    &pPresentationParameters[i], pCTX,
                                    This->params.hFocusWindow,
                                    &This->swapchains[i]);
        }

        ID3DPresent_Release(present);
        if (FAILED(hr))
            return hr;
        NineUnknown_ConvertRefToBind(NineUnknown(This->swapchains[i]));

        hr = NineSwapChain9_GetBackBuffer(This->swapchains[i], 0,
                                          D3DBACKBUFFER_TYPE_MONO,
                                          (IDirect3DSurface9 **)
                                          &This->state.rt[i]);
        if (FAILED(hr))
            return hr;
        NineUnknown_ConvertRefToBind(NineUnknown(This->state.rt[i]));
    }

    /* Initialize a dummy VBO to be used when a a vertex declaration does not
     * specify all the inputs needed by vertex shader, on win default behavior
     * is to pass 0,0,0,0 to the shader */
    {
        struct pipe_transfer *transfer;
        struct pipe_resource tmpl;
        struct pipe_box box;
        unsigned char *data;

        tmpl.target = PIPE_BUFFER;
        tmpl.format = PIPE_FORMAT_R8_UNORM;
        tmpl.width0 = 16; /* 4 floats */
        tmpl.height0 = 1;
        tmpl.depth0 = 1;
        tmpl.array_size = 1;
        tmpl.last_level = 0;
        tmpl.nr_samples = 0;
        tmpl.usage = PIPE_USAGE_DEFAULT;
        tmpl.bind = PIPE_BIND_VERTEX_BUFFER | PIPE_BIND_TRANSFER_WRITE;
        tmpl.flags = 0;
        This->dummy_vbo = pScreen->resource_create(pScreen, &tmpl);

        if (!This->dummy_vbo)
            return D3DERR_OUTOFVIDEOMEMORY;

        u_box_1d(0, 16, &box);
        data = This->pipe->transfer_map(This->pipe, This->dummy_vbo, 0,
                                        PIPE_TRANSFER_WRITE |
                                        PIPE_TRANSFER_DISCARD_WHOLE_RESOURCE,
                                        &box, &transfer);
        assert(data);
        assert(transfer);
        memset(data, 0, 16);
        This->pipe->transfer_unmap(This->pipe, transfer);
    }

    This->cursor.software = FALSE;
    This->cursor.hotspot.x = -1;
    This->cursor.hotspot.y = -1;
    {
        struct pipe_resource tmpl;
        tmpl.target = PIPE_TEXTURE_2D;
        tmpl.format = PIPE_FORMAT_R8G8B8A8_UNORM;
        tmpl.width0 = 64;
        tmpl.height0 = 64;
        tmpl.depth0 = 1;
        tmpl.array_size = 1;
        tmpl.last_level = 0;
        tmpl.nr_samples = 0;
        tmpl.usage = PIPE_USAGE_DEFAULT;
        tmpl.bind = PIPE_BIND_CURSOR | PIPE_BIND_SAMPLER_VIEW;
        tmpl.flags = 0;

        This->cursor.image = pScreen->resource_create(pScreen, &tmpl);
        if (!This->cursor.image)
            return D3DERR_OUTOFVIDEOMEMORY;
    }

    /* Create constant buffers. */
    {
        struct pipe_resource tmpl;
        unsigned max_const_vs, max_const_ps;

        /* vs 3.0: >= 256 float constants, but for cards with exactly 256 slots,
         * we have to take in some more slots for int and bool*/
        max_const_vs = _min(pScreen->get_shader_param(pScreen, PIPE_SHADER_VERTEX,
                                PIPE_SHADER_CAP_MAX_CONST_BUFFER_SIZE) /
                                sizeof(float[4]),
                            NINE_MAX_CONST_ALL);
        /* ps 3.0: 224 float constants. All cards supported support at least
         * 256 constants for ps */
        max_const_ps = NINE_MAX_CONST_F_PS3 + (NINE_MAX_CONST_I + NINE_MAX_CONST_B / 4);

        This->max_vs_const_f = max_const_vs -
                               (NINE_MAX_CONST_I + NINE_MAX_CONST_B / 4);
        This->max_ps_const_f = max_const_ps -
                               (NINE_MAX_CONST_I + NINE_MAX_CONST_B / 4);

        This->vs_const_size = max_const_vs * sizeof(float[4]);
        This->ps_const_size = max_const_ps * sizeof(float[4]);
        /* Include space for I,B constants for user constbuf. */
        This->state.vs_const_f = CALLOC(This->vs_const_size, 1);
        This->state.ps_const_f = CALLOC(This->ps_const_size, 1);
        This->state.vs_lconstf_temp = CALLOC(This->vs_const_size,1);
        if (!This->state.vs_const_f || !This->state.ps_const_f ||
            !This->state.vs_lconstf_temp)
            return E_OUTOFMEMORY;

        if (strstr(pScreen->get_name(pScreen), "AMD") ||
            strstr(pScreen->get_name(pScreen), "ATI")) {
            This->prefer_user_constbuf = TRUE;
            This->driver_bugs.buggy_barycentrics = TRUE;
        }

        tmpl.target = PIPE_BUFFER;
        tmpl.format = PIPE_FORMAT_R8_UNORM;
        tmpl.height0 = 1;
        tmpl.depth0 = 1;
        tmpl.array_size = 1;
        tmpl.last_level = 0;
        tmpl.nr_samples = 0;
        tmpl.usage = PIPE_USAGE_DYNAMIC;
        tmpl.bind = PIPE_BIND_CONSTANT_BUFFER;
        tmpl.flags = 0;

        tmpl.width0 = This->vs_const_size;
        This->constbuf_vs = pScreen->resource_create(pScreen, &tmpl);

        tmpl.width0 = This->ps_const_size;
        This->constbuf_ps = pScreen->resource_create(pScreen, &tmpl);

        if (!This->constbuf_vs || !This->constbuf_ps)
            return E_OUTOFMEMORY;
    }

    /* allocate dummy texture/sampler for when there are missing ones bound */
    {
        struct pipe_resource tmplt;
        struct pipe_sampler_view templ;

        tmplt.target = PIPE_TEXTURE_2D;
        tmplt.width0 = 1;
        tmplt.height0 = 1;
        tmplt.depth0 = 1;
        tmplt.last_level = 0;
        tmplt.array_size = 1;
        tmplt.usage = PIPE_USAGE_DEFAULT;
        tmplt.flags = 0;
        tmplt.format = PIPE_FORMAT_B8G8R8A8_UNORM;
        tmplt.bind = PIPE_BIND_SAMPLER_VIEW;
        tmplt.nr_samples = 0;

        This->dummy_texture = This->screen->resource_create(This->screen, &tmplt);
        if (!This->dummy_texture)
            return D3DERR_DRIVERINTERNALERROR;

        templ.format = PIPE_FORMAT_B8G8R8A8_UNORM;
        templ.u.tex.first_layer = 0;
        templ.u.tex.last_layer = 0;
        templ.u.tex.first_level = 0;
        templ.u.tex.last_level = 0;
        templ.swizzle_r = PIPE_SWIZZLE_ZERO;
        templ.swizzle_g = PIPE_SWIZZLE_ZERO;
        templ.swizzle_b = PIPE_SWIZZLE_ZERO;
        templ.swizzle_a = PIPE_SWIZZLE_ONE;
        templ.target = This->dummy_texture->target;

        This->dummy_sampler = This->pipe->create_sampler_view(This->pipe, This->dummy_texture, &templ);
        if (!This->dummy_sampler)
            return D3DERR_DRIVERINTERNALERROR;
    }

    /* Allocate upload helper for drivers that suck (from st pov ;). */
    {
        unsigned bind = 0;

        This->driver_caps.user_vbufs = GET_PCAP(USER_VERTEX_BUFFERS);
        This->driver_caps.user_ibufs = GET_PCAP(USER_INDEX_BUFFERS);

        if (!This->driver_caps.user_vbufs) bind |= PIPE_BIND_VERTEX_BUFFER;
        if (!This->driver_caps.user_ibufs) bind |= PIPE_BIND_INDEX_BUFFER;
        if (bind)
            This->upload = u_upload_create(This->pipe, 1 << 20, 4, bind);
    }

    This->driver_caps.window_space_position_support = GET_PCAP(TGSI_VS_WINDOW_SPACE_POSITION);
    This->driver_caps.vs_integer = pScreen->get_shader_param(pScreen, PIPE_SHADER_VERTEX, PIPE_SHADER_CAP_INTEGERS);
    This->driver_caps.ps_integer = pScreen->get_shader_param(pScreen, PIPE_SHADER_FRAGMENT, PIPE_SHADER_CAP_INTEGERS);

    nine_ff_init(This); /* initialize fixed function code */

    NineDevice9_SetDefaultState(This, FALSE);
    NineDevice9_RestoreNonCSOState(This, ~0);

    This->update = &This->state;
    nine_update_state(This, ~0);

    ID3DPresentGroup_Release(This->present);

    return D3D_OK;
}
#undef GET_PCAP

void
NineDevice9_dtor( struct NineDevice9 *This )
{
    unsigned i;

    DBG("This=%p\n", This);

    if (This->pipe && This->cso)
        nine_pipe_context_clear(This);
    nine_ff_fini(This);
    nine_state_clear(&This->state, TRUE);

    if (This->upload)
        u_upload_destroy(This->upload);

    nine_bind(&This->record, NULL);

    pipe_sampler_view_reference(&This->dummy_sampler, NULL);
    pipe_resource_reference(&This->dummy_texture, NULL);
    pipe_resource_reference(&This->constbuf_vs, NULL);
    pipe_resource_reference(&This->constbuf_ps, NULL);
    pipe_resource_reference(&This->dummy_vbo, NULL);
    FREE(This->state.vs_const_f);
    FREE(This->state.ps_const_f);
    FREE(This->state.vs_lconstf_temp);

    if (This->swapchains) {
        for (i = 0; i < This->nswapchains; ++i)
            NineUnknown_Unbind(NineUnknown(This->swapchains[i]));
        FREE(This->swapchains);
    }

    /* state stuff */
    if (This->pipe) {
        if (This->cso) {
            cso_destroy_context(This->cso);
        }
        if (This->pipe->destroy) { This->pipe->destroy(This->pipe); }
    }

    if (This->present) { ID3DPresentGroup_Release(This->present); }
    if (This->d3d9) { IDirect3D9_Release(This->d3d9); }

    NineUnknown_dtor(&This->base);
}

struct pipe_screen *
NineDevice9_GetScreen( struct NineDevice9 *This )
{
    return This->screen;
}

struct pipe_context *
NineDevice9_GetPipe( struct NineDevice9 *This )
{
    return This->pipe;
}

struct cso_context *
NineDevice9_GetCSO( struct NineDevice9 *This )
{
    return This->cso;
}

const D3DCAPS9 *
NineDevice9_GetCaps( struct NineDevice9 *This )
{
    return &This->caps;
}

static INLINE void
NineDevice9_PauseRecording( struct NineDevice9 *This )
{
    if (This->record) {
        This->update = &This->state;
        This->is_recording = FALSE;
    }
}

static INLINE void
NineDevice9_ResumeRecording( struct NineDevice9 *This )
{
    if (This->record) {
        This->update = &This->record->state;
        This->is_recording = TRUE;
    }
}

HRESULT WINAPI
NineDevice9_TestCooperativeLevel( struct NineDevice9 *This )
{
    return D3D_OK; /* TODO */
}

UINT WINAPI
NineDevice9_GetAvailableTextureMem( struct NineDevice9 *This )
{
   const unsigned mem = This->screen->get_param(This->screen, PIPE_CAP_VIDEO_MEMORY);
   if (mem < 4096)
      return mem << 20;
   else
      return UINT_MAX;
}

HRESULT WINAPI
NineDevice9_EvictManagedResources( struct NineDevice9 *This )
{
    /* We don't really need to do anything here, but might want to free up
     * the GPU virtual address space by killing pipe_resources.
     */
    STUB(D3D_OK);
}

HRESULT WINAPI
NineDevice9_GetDirect3D( struct NineDevice9 *This,
                         IDirect3D9 **ppD3D9 )
{
    user_assert(ppD3D9 != NULL, E_POINTER);
    IDirect3D9_AddRef(This->d3d9);
    *ppD3D9 = This->d3d9;
    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetDeviceCaps( struct NineDevice9 *This,
                           D3DCAPS9 *pCaps )
{
    user_assert(pCaps != NULL, D3DERR_INVALIDCALL);
    *pCaps = This->caps;
    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetDisplayMode( struct NineDevice9 *This,
                            UINT iSwapChain,
                            D3DDISPLAYMODE *pMode )
{
    DBG("This=%p iSwapChain=%u pMode=%p\n", This, iSwapChain, pMode);

    user_assert(iSwapChain < This->nswapchains, D3DERR_INVALIDCALL);

    return NineSwapChain9_GetDisplayMode(This->swapchains[iSwapChain], pMode);
}

HRESULT WINAPI
NineDevice9_GetCreationParameters( struct NineDevice9 *This,
                                   D3DDEVICE_CREATION_PARAMETERS *pParameters )
{
    user_assert(pParameters != NULL, D3DERR_INVALIDCALL);
    *pParameters = This->params;
    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_SetCursorProperties( struct NineDevice9 *This,
                                 UINT XHotSpot,
                                 UINT YHotSpot,
                                 IDirect3DSurface9 *pCursorBitmap )
{
    /* TODO: hardware cursor */
    struct NineSurface9 *surf = NineSurface9(pCursorBitmap);
    struct pipe_context *pipe = This->pipe;
    struct pipe_box box;
    struct pipe_transfer *transfer;
    void *ptr;

    DBG_FLAG(DBG_SWAPCHAIN, "This=%p XHotSpot=%u YHotSpot=%u "
             "pCursorBitmap=%p\n", This, XHotSpot, YHotSpot, pCursorBitmap);

    user_assert(pCursorBitmap, D3DERR_INVALIDCALL);

    This->cursor.w = MIN2(surf->desc.Width, This->cursor.image->width0);
    This->cursor.h = MIN2(surf->desc.Height, This->cursor.image->height0);

    u_box_origin_2d(This->cursor.w, This->cursor.h, &box);

    ptr = pipe->transfer_map(pipe, This->cursor.image, 0,
                             PIPE_TRANSFER_WRITE |
                             PIPE_TRANSFER_DISCARD_WHOLE_RESOURCE,
                             &box, &transfer);
    if (!ptr)
        ret_err("Failed to update cursor image.\n", D3DERR_DRIVERINTERNALERROR);

    This->cursor.hotspot.x = XHotSpot;
    This->cursor.hotspot.y = YHotSpot;

    /* Copy cursor image to internal storage. */
    {
        D3DLOCKED_RECT lock;
        HRESULT hr;
        const struct util_format_description *sfmt =
            util_format_description(surf->base.info.format);
        assert(sfmt);

        hr = NineSurface9_LockRect(surf, &lock, NULL, D3DLOCK_READONLY);
        if (FAILED(hr))
            ret_err("Failed to map cursor source image.\n",
                    D3DERR_DRIVERINTERNALERROR);

        sfmt->unpack_rgba_8unorm(ptr, transfer->stride,
                                 lock.pBits, lock.Pitch,
                                 This->cursor.w, This->cursor.h);

        if (!This->cursor.software &&
            This->cursor.w == 32 && This->cursor.h == 32)
            ID3DPresent_SetCursor(This->swapchains[0]->present,
                                  lock.pBits, &This->cursor.hotspot,
                                  This->cursor.visible);

        NineSurface9_UnlockRect(surf);
    }
    pipe->transfer_unmap(pipe, transfer);

    return D3D_OK;
}

void WINAPI
NineDevice9_SetCursorPosition( struct NineDevice9 *This,
                               int X,
                               int Y,
                               DWORD Flags )
{
    struct NineSwapChain9 *swap = This->swapchains[0];

    DBG("This=%p X=%d Y=%d Flags=%d\n", This, X, Y, Flags);

    This->cursor.pos.x = X;
    This->cursor.pos.y = Y;

    if (!This->cursor.software)
        ID3DPresent_SetCursorPos(swap->present, &This->cursor.pos);
}

BOOL WINAPI
NineDevice9_ShowCursor( struct NineDevice9 *This,
                        BOOL bShow )
{
    BOOL old = This->cursor.visible;

    DBG("This=%p bShow=%d\n", This, (int) bShow);

    This->cursor.visible = bShow && (This->cursor.hotspot.x != -1);
    if (!This->cursor.software)
        ID3DPresent_SetCursor(This->swapchains[0]->present, NULL, NULL, bShow);

    return old;
}

HRESULT WINAPI
NineDevice9_CreateAdditionalSwapChain( struct NineDevice9 *This,
                                       D3DPRESENT_PARAMETERS *pPresentationParameters,
                                       IDirect3DSwapChain9 **pSwapChain )
{
    struct NineSwapChain9 *swapchain, *tmplt = This->swapchains[0];
    ID3DPresent *present;
    HRESULT hr;

    DBG("This=%p pPresentationParameters=%p pSwapChain=%p\n",
        This, pPresentationParameters, pSwapChain);

    user_assert(pPresentationParameters, D3DERR_INVALIDCALL);

    hr = ID3DPresentGroup_CreateAdditionalPresent(This->present, pPresentationParameters, &present);

    if (FAILED(hr))
        return hr;

    hr = NineSwapChain9_new(This, FALSE, present, pPresentationParameters,
                            tmplt->actx,
                            tmplt->params.hDeviceWindow,
                            &swapchain);
    if (FAILED(hr))
        return hr;

    *pSwapChain = (IDirect3DSwapChain9 *)swapchain;
    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetSwapChain( struct NineDevice9 *This,
                          UINT iSwapChain,
                          IDirect3DSwapChain9 **pSwapChain )
{
    user_assert(pSwapChain != NULL, D3DERR_INVALIDCALL);

    *pSwapChain = NULL;
    user_assert(iSwapChain < This->nswapchains, D3DERR_INVALIDCALL);

    NineUnknown_AddRef(NineUnknown(This->swapchains[iSwapChain]));
    *pSwapChain = (IDirect3DSwapChain9 *)This->swapchains[iSwapChain];

    return D3D_OK;
}

UINT WINAPI
NineDevice9_GetNumberOfSwapChains( struct NineDevice9 *This )
{
    return This->nswapchains;
}

HRESULT WINAPI
NineDevice9_Reset( struct NineDevice9 *This,
                   D3DPRESENT_PARAMETERS *pPresentationParameters )
{
    HRESULT hr = D3D_OK;
    unsigned i;

    DBG("This=%p pPresentationParameters=%p\n", This, pPresentationParameters);

    for (i = 0; i < This->nswapchains; ++i) {
        D3DPRESENT_PARAMETERS *params = &pPresentationParameters[i];
        hr = NineSwapChain9_Resize(This->swapchains[i], params, NULL);
        if (FAILED(hr))
            return (hr == D3DERR_OUTOFVIDEOMEMORY) ? hr : D3DERR_DEVICELOST;
    }

    nine_pipe_context_clear(This);
    nine_state_clear(&This->state, TRUE);

    NineDevice9_SetDefaultState(This, TRUE);
    NineDevice9_SetRenderTarget(
        This, 0, (IDirect3DSurface9 *)This->swapchains[0]->buffers[0]);
    /* XXX: better use GetBackBuffer here ? */

    return hr;
}

HRESULT WINAPI
NineDevice9_Present( struct NineDevice9 *This,
                     const RECT *pSourceRect,
                     const RECT *pDestRect,
                     HWND hDestWindowOverride,
                     const RGNDATA *pDirtyRegion )
{
    unsigned i;
    HRESULT hr;

    DBG("This=%p pSourceRect=%p pDestRect=%p hDestWindowOverride=%p pDirtyRegion=%p\n",
        This, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);

    /* XXX is this right? */
    for (i = 0; i < This->nswapchains; ++i) {
        hr = NineSwapChain9_Present(This->swapchains[i], pSourceRect, pDestRect,
                                    hDestWindowOverride, pDirtyRegion, 0);
        if (FAILED(hr)) { return hr; }
    }

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetBackBuffer( struct NineDevice9 *This,
                           UINT iSwapChain,
                           UINT iBackBuffer,
                           D3DBACKBUFFER_TYPE Type,
                           IDirect3DSurface9 **ppBackBuffer )
{
    user_assert(ppBackBuffer != NULL, D3DERR_INVALIDCALL);
    user_assert(iSwapChain < This->nswapchains, D3DERR_INVALIDCALL);

    return NineSwapChain9_GetBackBuffer(This->swapchains[iSwapChain],
                                        iBackBuffer, Type, ppBackBuffer);
}

HRESULT WINAPI
NineDevice9_GetRasterStatus( struct NineDevice9 *This,
                             UINT iSwapChain,
                             D3DRASTER_STATUS *pRasterStatus )
{
    user_assert(pRasterStatus != NULL, D3DERR_INVALIDCALL);
    user_assert(iSwapChain < This->nswapchains, D3DERR_INVALIDCALL);

    return NineSwapChain9_GetRasterStatus(This->swapchains[iSwapChain],
                                          pRasterStatus);
}

HRESULT WINAPI
NineDevice9_SetDialogBoxMode( struct NineDevice9 *This,
                              BOOL bEnableDialogs )
{
    STUB(D3DERR_INVALIDCALL);
}

void WINAPI
NineDevice9_SetGammaRamp( struct NineDevice9 *This,
                          UINT iSwapChain,
                          DWORD Flags,
                          const D3DGAMMARAMP *pRamp )
{
    DBG("This=%p iSwapChain=%u Flags=%x pRamp=%p\n", This,
        iSwapChain, Flags, pRamp);

    user_warn(iSwapChain >= This->nswapchains);
    user_warn(!pRamp);

    if (pRamp && (iSwapChain < This->nswapchains)) {
        struct NineSwapChain9 *swap = This->swapchains[iSwapChain];
        swap->gamma = *pRamp;
        ID3DPresent_SetGammaRamp(swap->present, pRamp, swap->params.hDeviceWindow);
    }
}

void WINAPI
NineDevice9_GetGammaRamp( struct NineDevice9 *This,
                          UINT iSwapChain,
                          D3DGAMMARAMP *pRamp )
{
    DBG("This=%p iSwapChain=%u pRamp=%p\n", This, iSwapChain, pRamp);

    user_warn(iSwapChain >= This->nswapchains);
    user_warn(!pRamp);

    if (pRamp && (iSwapChain < This->nswapchains))
        *pRamp = This->swapchains[iSwapChain]->gamma;
}

HRESULT WINAPI
NineDevice9_CreateTexture( struct NineDevice9 *This,
                           UINT Width,
                           UINT Height,
                           UINT Levels,
                           DWORD Usage,
                           D3DFORMAT Format,
                           D3DPOOL Pool,
                           IDirect3DTexture9 **ppTexture,
                           HANDLE *pSharedHandle )
{
    struct NineTexture9 *tex;
    HRESULT hr;

    DBG("This=%p Width=%u Height=%u Levels=%u Usage=%s Format=%s Pool=%s "
        "ppOut=%p pSharedHandle=%p\n", This, Width, Height, Levels,
        nine_D3DUSAGE_to_str(Usage), d3dformat_to_string(Format),
        nine_D3DPOOL_to_str(Pool), ppTexture, pSharedHandle);

    Usage &= D3DUSAGE_AUTOGENMIPMAP | D3DUSAGE_DEPTHSTENCIL | D3DUSAGE_DMAP |
             D3DUSAGE_DYNAMIC | D3DUSAGE_NONSECURE | D3DUSAGE_RENDERTARGET |
             D3DUSAGE_SOFTWAREPROCESSING | D3DUSAGE_TEXTAPI;

    *ppTexture = NULL;
    user_assert(Width && Height, D3DERR_INVALIDCALL);
    user_assert(!pSharedHandle || This->ex, D3DERR_INVALIDCALL);
    /* When is used shared handle, Pool must be
     * SYSTEMMEM with Levels 1 or DEFAULT with any Levels */
    user_assert(!pSharedHandle || Pool != D3DPOOL_SYSTEMMEM || Levels == 1,
                D3DERR_INVALIDCALL);
    user_assert(!pSharedHandle || Pool == D3DPOOL_SYSTEMMEM || Pool == D3DPOOL_DEFAULT,
                D3DERR_INVALIDCALL);
    user_assert((Usage != D3DUSAGE_AUTOGENMIPMAP || Levels <= 1), D3DERR_INVALIDCALL);

    hr = NineTexture9_new(This, Width, Height, Levels, Usage, Format, Pool,
                          &tex, pSharedHandle);
    if (SUCCEEDED(hr))
        *ppTexture = (IDirect3DTexture9 *)tex;

    return hr;
}

HRESULT WINAPI
NineDevice9_CreateVolumeTexture( struct NineDevice9 *This,
                                 UINT Width,
                                 UINT Height,
                                 UINT Depth,
                                 UINT Levels,
                                 DWORD Usage,
                                 D3DFORMAT Format,
                                 D3DPOOL Pool,
                                 IDirect3DVolumeTexture9 **ppVolumeTexture,
                                 HANDLE *pSharedHandle )
{
    struct NineVolumeTexture9 *tex;
    HRESULT hr;

    DBG("This=%p Width=%u Height=%u Depth=%u Levels=%u Usage=%s Format=%s Pool=%s "
        "ppOut=%p pSharedHandle=%p\n", This, Width, Height, Depth, Levels,
        nine_D3DUSAGE_to_str(Usage), d3dformat_to_string(Format),
        nine_D3DPOOL_to_str(Pool), ppVolumeTexture, pSharedHandle);

    Usage &= D3DUSAGE_DYNAMIC | D3DUSAGE_NONSECURE |
             D3DUSAGE_SOFTWAREPROCESSING;

    *ppVolumeTexture = NULL;
    user_assert(Width && Height && Depth, D3DERR_INVALIDCALL);
    user_assert(!pSharedHandle || Pool == D3DPOOL_DEFAULT, D3DERR_INVALIDCALL);

    hr = NineVolumeTexture9_new(This, Width, Height, Depth, Levels,
                                Usage, Format, Pool, &tex, pSharedHandle);
    if (SUCCEEDED(hr))
        *ppVolumeTexture = (IDirect3DVolumeTexture9 *)tex;

    return hr;
}

HRESULT WINAPI
NineDevice9_CreateCubeTexture( struct NineDevice9 *This,
                               UINT EdgeLength,
                               UINT Levels,
                               DWORD Usage,
                               D3DFORMAT Format,
                               D3DPOOL Pool,
                               IDirect3DCubeTexture9 **ppCubeTexture,
                               HANDLE *pSharedHandle )
{
    struct NineCubeTexture9 *tex;
    HRESULT hr;

    DBG("This=%p EdgeLength=%u Levels=%u Usage=%s Format=%s Pool=%s ppOut=%p "
        "pSharedHandle=%p\n", This, EdgeLength, Levels,
        nine_D3DUSAGE_to_str(Usage), d3dformat_to_string(Format),
        nine_D3DPOOL_to_str(Pool), ppCubeTexture, pSharedHandle);

    Usage &= D3DUSAGE_AUTOGENMIPMAP | D3DUSAGE_DEPTHSTENCIL | D3DUSAGE_DYNAMIC |
             D3DUSAGE_NONSECURE | D3DUSAGE_RENDERTARGET |
             D3DUSAGE_SOFTWAREPROCESSING;

    *ppCubeTexture = NULL;
    user_assert(EdgeLength, D3DERR_INVALIDCALL);
    user_assert(!pSharedHandle || Pool == D3DPOOL_DEFAULT, D3DERR_INVALIDCALL);

    hr = NineCubeTexture9_new(This, EdgeLength, Levels, Usage, Format, Pool,
                              &tex, pSharedHandle);
    if (SUCCEEDED(hr))
        *ppCubeTexture = (IDirect3DCubeTexture9 *)tex;

    return hr;
}

HRESULT WINAPI
NineDevice9_CreateVertexBuffer( struct NineDevice9 *This,
                                UINT Length,
                                DWORD Usage,
                                DWORD FVF,
                                D3DPOOL Pool,
                                IDirect3DVertexBuffer9 **ppVertexBuffer,
                                HANDLE *pSharedHandle )
{
    struct NineVertexBuffer9 *buf;
    HRESULT hr;
    D3DVERTEXBUFFER_DESC desc;

    DBG("This=%p Length=%u Usage=%x FVF=%x Pool=%u ppOut=%p pSharedHandle=%p\n",
        This, Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);

    user_assert(!pSharedHandle || Pool == D3DPOOL_DEFAULT, D3DERR_NOTAVAILABLE);

    desc.Format = D3DFMT_VERTEXDATA;
    desc.Type = D3DRTYPE_VERTEXBUFFER;
    desc.Usage = Usage &
        (D3DUSAGE_DONOTCLIP | D3DUSAGE_DYNAMIC | D3DUSAGE_NONSECURE |
         D3DUSAGE_NPATCHES | D3DUSAGE_POINTS | D3DUSAGE_RTPATCHES |
         D3DUSAGE_SOFTWAREPROCESSING | D3DUSAGE_TEXTAPI |
         D3DUSAGE_WRITEONLY);
    desc.Pool = Pool;
    desc.Size = Length;
    desc.FVF = FVF;

    user_assert(!pSharedHandle || Pool == D3DPOOL_DEFAULT, D3DERR_INVALIDCALL);
    user_assert(desc.Usage == Usage, D3DERR_INVALIDCALL);

    hr = NineVertexBuffer9_new(This, &desc, &buf);
    if (SUCCEEDED(hr))
        *ppVertexBuffer = (IDirect3DVertexBuffer9 *)buf;
    return hr;
}

HRESULT WINAPI
NineDevice9_CreateIndexBuffer( struct NineDevice9 *This,
                               UINT Length,
                               DWORD Usage,
                               D3DFORMAT Format,
                               D3DPOOL Pool,
                               IDirect3DIndexBuffer9 **ppIndexBuffer,
                               HANDLE *pSharedHandle )
{
    struct NineIndexBuffer9 *buf;
    HRESULT hr;
    D3DINDEXBUFFER_DESC desc;

    DBG("This=%p Length=%u Usage=%x Format=%s Pool=%u ppOut=%p "
        "pSharedHandle=%p\n", This, Length, Usage,
        d3dformat_to_string(Format), Pool, ppIndexBuffer, pSharedHandle);

    user_assert(!pSharedHandle || Pool == D3DPOOL_DEFAULT, D3DERR_NOTAVAILABLE);

    desc.Format = Format;
    desc.Type = D3DRTYPE_INDEXBUFFER;
    desc.Usage = Usage &
        (D3DUSAGE_DONOTCLIP | D3DUSAGE_DYNAMIC | D3DUSAGE_NONSECURE |
         D3DUSAGE_NPATCHES | D3DUSAGE_POINTS | D3DUSAGE_RTPATCHES |
         D3DUSAGE_SOFTWAREPROCESSING | D3DUSAGE_WRITEONLY);
    desc.Pool = Pool;
    desc.Size = Length;

    user_assert(!pSharedHandle || Pool == D3DPOOL_DEFAULT, D3DERR_INVALIDCALL);
    user_assert(desc.Usage == Usage, D3DERR_INVALIDCALL);

    hr = NineIndexBuffer9_new(This, &desc, &buf);
    if (SUCCEEDED(hr))
        *ppIndexBuffer = (IDirect3DIndexBuffer9 *)buf;
    return hr;
}

static HRESULT
create_zs_or_rt_surface(struct NineDevice9 *This,
                        unsigned type, /* 0 = RT, 1 = ZS, 2 = plain */
                        D3DPOOL Pool,
                        UINT Width, UINT Height,
                        D3DFORMAT Format,
                        D3DMULTISAMPLE_TYPE MultiSample,
                        DWORD MultisampleQuality,
                        BOOL Discard_or_Lockable,
                        IDirect3DSurface9 **ppSurface,
                        HANDLE *pSharedHandle)
{
    struct NineSurface9 *surface;
    struct pipe_screen *screen = This->screen;
    struct pipe_resource *resource = NULL;
    HRESULT hr;
    D3DSURFACE_DESC desc;
    struct pipe_resource templ;

    DBG("This=%p type=%u Pool=%s Width=%u Height=%u Format=%s MS=%u Quality=%u "
        "Discard_or_Lockable=%i ppSurface=%p pSharedHandle=%p\n",
        This, type, nine_D3DPOOL_to_str(Pool), Width, Height,
        d3dformat_to_string(Format), MultiSample, MultisampleQuality,
        Discard_or_Lockable, ppSurface, pSharedHandle);

    if (pSharedHandle)
      DBG("FIXME Used shared handle! This option isn't probably handled correctly!\n");

    user_assert(Width && Height, D3DERR_INVALIDCALL);
    user_assert(Pool != D3DPOOL_MANAGED, D3DERR_INVALIDCALL);

    templ.target = PIPE_TEXTURE_2D;
    templ.width0 = Width;
    templ.height0 = Height;
    templ.depth0 = 1;
    templ.array_size = 1;
    templ.last_level = 0;
    templ.nr_samples = (unsigned)MultiSample;
    templ.usage = PIPE_USAGE_DEFAULT;
    templ.flags = 0;
    templ.bind = PIPE_BIND_SAMPLER_VIEW; /* StretchRect */
    switch (type) {
    case 0: templ.bind |= PIPE_BIND_RENDER_TARGET; break;
    case 1: templ.bind = d3d9_get_pipe_depth_format_bindings(Format); break;
    default:
        assert(type == 2);
        break;
    }
    templ.format = d3d9_to_pipe_format_checked(screen, Format, templ.target,
                                               templ.nr_samples, templ.bind,
                                               FALSE);

    desc.Format = Format;
    desc.Type = D3DRTYPE_SURFACE;
    desc.Usage = 0;
    desc.Pool = Pool;
    desc.MultiSampleType = MultiSample;
    desc.MultiSampleQuality = MultisampleQuality;
    desc.Width = Width;
    desc.Height = Height;
    switch (type) {
    case 0: desc.Usage = D3DUSAGE_RENDERTARGET; break;
    case 1: desc.Usage = D3DUSAGE_DEPTHSTENCIL; break;
    default: break;
    }

    if (Pool == D3DPOOL_DEFAULT && Format != D3DFMT_NULL) {
        /* resource_create doesn't return an error code, so check format here */
        user_assert(templ.format != PIPE_FORMAT_NONE, D3DERR_INVALIDCALL);
        resource = screen->resource_create(screen, &templ);
        user_assert(resource, D3DERR_OUTOFVIDEOMEMORY);
        if (Discard_or_Lockable && (desc.Usage & D3DUSAGE_RENDERTARGET))
            resource->flags |= NINE_RESOURCE_FLAG_LOCKABLE;
    } else {
        resource = NULL;
    }
    hr = NineSurface9_new(This, NULL, resource, NULL, 0, 0, 0, &desc, &surface);
    pipe_resource_reference(&resource, NULL);

    if (SUCCEEDED(hr))
        *ppSurface = (IDirect3DSurface9 *)surface;
    return hr;
}

HRESULT WINAPI
NineDevice9_CreateRenderTarget( struct NineDevice9 *This,
                                UINT Width,
                                UINT Height,
                                D3DFORMAT Format,
                                D3DMULTISAMPLE_TYPE MultiSample,
                                DWORD MultisampleQuality,
                                BOOL Lockable,
                                IDirect3DSurface9 **ppSurface,
                                HANDLE *pSharedHandle )
{
    *ppSurface = NULL;
    return create_zs_or_rt_surface(This, 0, D3DPOOL_DEFAULT,
                                   Width, Height, Format,
                                   MultiSample, MultisampleQuality,
                                   Lockable, ppSurface, pSharedHandle);
}

HRESULT WINAPI
NineDevice9_CreateDepthStencilSurface( struct NineDevice9 *This,
                                       UINT Width,
                                       UINT Height,
                                       D3DFORMAT Format,
                                       D3DMULTISAMPLE_TYPE MultiSample,
                                       DWORD MultisampleQuality,
                                       BOOL Discard,
                                       IDirect3DSurface9 **ppSurface,
                                       HANDLE *pSharedHandle )
{
    *ppSurface = NULL;
    if (!depth_stencil_format(Format))
        return D3DERR_NOTAVAILABLE;
    return create_zs_or_rt_surface(This, 1, D3DPOOL_DEFAULT,
                                   Width, Height, Format,
                                   MultiSample, MultisampleQuality,
                                   Discard, ppSurface, pSharedHandle);
}

HRESULT WINAPI
NineDevice9_UpdateSurface( struct NineDevice9 *This,
                           IDirect3DSurface9 *pSourceSurface,
                           const RECT *pSourceRect,
                           IDirect3DSurface9 *pDestinationSurface,
                           const POINT *pDestPoint )
{
    struct NineSurface9 *dst = NineSurface9(pDestinationSurface);
    struct NineSurface9 *src = NineSurface9(pSourceSurface);

    DBG("This=%p pSourceSurface=%p pDestinationSurface=%p "
        "pSourceRect=%p pDestPoint=%p\n", This,
        pSourceSurface, pDestinationSurface, pSourceRect, pDestPoint);
    if (pSourceRect)
        DBG("pSourceRect = (%u,%u)-(%u,%u)\n",
            pSourceRect->left, pSourceRect->top,
            pSourceRect->right, pSourceRect->bottom);
    if (pDestPoint)
        DBG("pDestPoint = (%u,%u)\n", pDestPoint->x, pDestPoint->y);

    user_assert(dst->base.pool == D3DPOOL_DEFAULT, D3DERR_INVALIDCALL);
    user_assert(src->base.pool == D3DPOOL_SYSTEMMEM, D3DERR_INVALIDCALL);

    user_assert(dst->desc.MultiSampleType == D3DMULTISAMPLE_NONE, D3DERR_INVALIDCALL);
    user_assert(src->desc.MultiSampleType == D3DMULTISAMPLE_NONE, D3DERR_INVALIDCALL);

    return NineSurface9_CopySurface(dst, src, pDestPoint, pSourceRect);
}

HRESULT WINAPI
NineDevice9_UpdateTexture( struct NineDevice9 *This,
                           IDirect3DBaseTexture9 *pSourceTexture,
                           IDirect3DBaseTexture9 *pDestinationTexture )
{
    struct NineBaseTexture9 *dstb = NineBaseTexture9(pDestinationTexture);
    struct NineBaseTexture9 *srcb = NineBaseTexture9(pSourceTexture);
    unsigned l, m;
    unsigned last_level = dstb->base.info.last_level;

    DBG("This=%p pSourceTexture=%p pDestinationTexture=%p\n", This,
        pSourceTexture, pDestinationTexture);

    user_assert(pSourceTexture != pDestinationTexture, D3DERR_INVALIDCALL);

    user_assert(dstb->base.pool == D3DPOOL_DEFAULT, D3DERR_INVALIDCALL);
    user_assert(srcb->base.pool == D3DPOOL_SYSTEMMEM, D3DERR_INVALIDCALL);

    if (dstb->base.usage & D3DUSAGE_AUTOGENMIPMAP) {
        /* Only the first level is updated, the others regenerated. */
        last_level = 0;
        /* if the source has D3DUSAGE_AUTOGENMIPMAP, we have to ignore
         * the sublevels, thus level 0 has to match */
        user_assert(!(srcb->base.usage & D3DUSAGE_AUTOGENMIPMAP) ||
                    (srcb->base.info.width0 == dstb->base.info.width0 &&
                     srcb->base.info.height0 == dstb->base.info.height0 &&
                     srcb->base.info.depth0 == dstb->base.info.depth0),
                    D3DERR_INVALIDCALL);
    } else {
        user_assert(!(srcb->base.usage & D3DUSAGE_AUTOGENMIPMAP), D3DERR_INVALIDCALL);
    }

    user_assert(dstb->base.type == srcb->base.type, D3DERR_INVALIDCALL);

    /* TODO: We can restrict the update to the dirty portions of the source.
     * Yes, this seems silly, but it's what MSDN says ...
     */

    /* Find src level that matches dst level 0: */
    user_assert(srcb->base.info.width0 >= dstb->base.info.width0 &&
                srcb->base.info.height0 >= dstb->base.info.height0 &&
                srcb->base.info.depth0 >= dstb->base.info.depth0,
                D3DERR_INVALIDCALL);
    for (m = 0; m <= srcb->base.info.last_level; ++m) {
        unsigned w = u_minify(srcb->base.info.width0, m);
        unsigned h = u_minify(srcb->base.info.height0, m);
        unsigned d = u_minify(srcb->base.info.depth0, m);

        if (w == dstb->base.info.width0 &&
            h == dstb->base.info.height0 &&
            d == dstb->base.info.depth0)
            break;
    }
    user_assert(m <= srcb->base.info.last_level, D3DERR_INVALIDCALL);

    last_level = MIN2(last_level, srcb->base.info.last_level - m);

    if (dstb->base.type == D3DRTYPE_TEXTURE) {
        struct NineTexture9 *dst = NineTexture9(dstb);
        struct NineTexture9 *src = NineTexture9(srcb);

        for (l = 0; l <= last_level; ++l, ++m)
            NineSurface9_CopySurface(dst->surfaces[l],
                                     src->surfaces[m], NULL, NULL);
    } else
    if (dstb->base.type == D3DRTYPE_CUBETEXTURE) {
        struct NineCubeTexture9 *dst = NineCubeTexture9(dstb);
        struct NineCubeTexture9 *src = NineCubeTexture9(srcb);
        unsigned z;

        /* GPUs usually have them stored as arrays of mip-mapped 2D textures. */
        for (z = 0; z < 6; ++z) {
            for (l = 0; l <= last_level; ++l, ++m) {
                NineSurface9_CopySurface(dst->surfaces[l * 6 + z],
                                         src->surfaces[m * 6 + z], NULL, NULL);
            }
            m -= l;
        }
    } else
    if (dstb->base.type == D3DRTYPE_VOLUMETEXTURE) {
        struct NineVolumeTexture9 *dst = NineVolumeTexture9(dstb);
        struct NineVolumeTexture9 *src = NineVolumeTexture9(srcb);

        for (l = 0; l <= last_level; ++l, ++m)
            NineVolume9_CopyVolume(dst->volumes[l],
                                   src->volumes[m], 0, 0, 0, NULL);
    } else{
        assert(!"invalid texture type");
    }

    if (dstb->base.usage & D3DUSAGE_AUTOGENMIPMAP) {
        dstb->dirty_mip = TRUE;
        NineBaseTexture9_GenerateMipSubLevels(dstb);
    }

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetRenderTargetData( struct NineDevice9 *This,
                                 IDirect3DSurface9 *pRenderTarget,
                                 IDirect3DSurface9 *pDestSurface )
{
    struct NineSurface9 *dst = NineSurface9(pDestSurface);
    struct NineSurface9 *src = NineSurface9(pRenderTarget);

    DBG("This=%p pRenderTarget=%p pDestSurface=%p\n",
        This, pRenderTarget, pDestSurface);

    user_assert(dst->desc.Pool == D3DPOOL_SYSTEMMEM, D3DERR_INVALIDCALL);
    user_assert(src->desc.Pool == D3DPOOL_DEFAULT, D3DERR_INVALIDCALL);

    user_assert(dst->desc.MultiSampleType < 2, D3DERR_INVALIDCALL);
    user_assert(src->desc.MultiSampleType < 2, D3DERR_INVALIDCALL);

    return NineSurface9_CopySurface(dst, src, NULL, NULL);
}

HRESULT WINAPI
NineDevice9_GetFrontBufferData( struct NineDevice9 *This,
                                UINT iSwapChain,
                                IDirect3DSurface9 *pDestSurface )
{
    DBG("This=%p iSwapChain=%u pDestSurface=%p\n", This,
        iSwapChain, pDestSurface);

    user_assert(pDestSurface != NULL, D3DERR_INVALIDCALL);
    user_assert(iSwapChain < This->nswapchains, D3DERR_INVALIDCALL);

    return NineSwapChain9_GetFrontBufferData(This->swapchains[iSwapChain],
                                             pDestSurface);
}

HRESULT WINAPI
NineDevice9_StretchRect( struct NineDevice9 *This,
                         IDirect3DSurface9 *pSourceSurface,
                         const RECT *pSourceRect,
                         IDirect3DSurface9 *pDestSurface,
                         const RECT *pDestRect,
                         D3DTEXTUREFILTERTYPE Filter )
{
    struct pipe_screen *screen = This->screen;
    struct pipe_context *pipe = This->pipe;
    struct NineSurface9 *dst = NineSurface9(pDestSurface);
    struct NineSurface9 *src = NineSurface9(pSourceSurface);
    struct pipe_resource *dst_res = NineSurface9_GetResource(dst);
    struct pipe_resource *src_res = NineSurface9_GetResource(src);
    const boolean zs = util_format_is_depth_or_stencil(dst_res->format);
    struct pipe_blit_info blit;
    boolean scaled, clamped, ms, flip_x = FALSE, flip_y = FALSE;

    DBG("This=%p pSourceSurface=%p pSourceRect=%p pDestSurface=%p "
        "pDestRect=%p Filter=%u\n",
        This, pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
    if (pSourceRect)
        DBG("pSourceRect=(%u,%u)-(%u,%u)\n",
            pSourceRect->left, pSourceRect->top,
            pSourceRect->right, pSourceRect->bottom);
    if (pDestRect)
        DBG("pDestRect=(%u,%u)-(%u,%u)\n", pDestRect->left, pDestRect->top,
            pDestRect->right, pDestRect->bottom);

    user_assert(!zs || !This->in_scene, D3DERR_INVALIDCALL);
    user_assert(!zs || !pSourceRect ||
                (pSourceRect->left == 0 &&
                 pSourceRect->top == 0 &&
                 pSourceRect->right == src->desc.Width &&
                 pSourceRect->bottom == src->desc.Height), D3DERR_INVALIDCALL);
    user_assert(!zs || !pDestRect ||
                (pDestRect->left == 0 &&
                 pDestRect->top == 0 &&
                 pDestRect->right == dst->desc.Width &&
                 pDestRect->bottom == dst->desc.Height), D3DERR_INVALIDCALL);
    user_assert(!zs ||
                (dst->desc.Width == src->desc.Width &&
                 dst->desc.Height == src->desc.Height), D3DERR_INVALIDCALL);
    user_assert(zs || !util_format_is_depth_or_stencil(src_res->format),
                D3DERR_INVALIDCALL);
    user_assert(!zs || dst->desc.Format == src->desc.Format,
                D3DERR_INVALIDCALL);
    user_assert(screen->is_format_supported(screen, src_res->format,
                                            src_res->target,
                                            src_res->nr_samples,
                                            PIPE_BIND_SAMPLER_VIEW),
                D3DERR_INVALIDCALL);
    user_assert(dst->base.pool == D3DPOOL_DEFAULT &&
                src->base.pool == D3DPOOL_DEFAULT, D3DERR_INVALIDCALL);

    /* We might want to permit these, but wine thinks we shouldn't. */
    user_assert(!pDestRect ||
                (pDestRect->left <= pDestRect->right &&
                 pDestRect->top <= pDestRect->bottom), D3DERR_INVALIDCALL);
    user_assert(!pSourceRect ||
                (pSourceRect->left <= pSourceRect->right &&
                 pSourceRect->top <= pSourceRect->bottom), D3DERR_INVALIDCALL);

    memset(&blit, 0, sizeof(blit));
    blit.dst.resource = dst_res;
    blit.dst.level = dst->level;
    blit.dst.box.z = dst->layer;
    blit.dst.box.depth = 1;
    blit.dst.format = dst_res->format;
    if (pDestRect) {
        flip_x = pDestRect->left > pDestRect->right;
        if (flip_x) {
            blit.dst.box.x = pDestRect->right;
            blit.dst.box.width = pDestRect->left - pDestRect->right;
        } else {
            blit.dst.box.x = pDestRect->left;
            blit.dst.box.width = pDestRect->right - pDestRect->left;
        }
        flip_y = pDestRect->top > pDestRect->bottom;
        if (flip_y) {
            blit.dst.box.y = pDestRect->bottom;
            blit.dst.box.height = pDestRect->top - pDestRect->bottom;
        } else {
            blit.dst.box.y = pDestRect->top;
            blit.dst.box.height = pDestRect->bottom - pDestRect->top;
        }
    } else {
        blit.dst.box.x = 0;
        blit.dst.box.y = 0;
        blit.dst.box.width = dst->desc.Width;
        blit.dst.box.height = dst->desc.Height;
    }
    blit.src.resource = src_res;
    blit.src.level = src->level;
    blit.src.box.z = src->layer;
    blit.src.box.depth = 1;
    blit.src.format = src_res->format;
    if (pSourceRect) {
        if (flip_x ^ (pSourceRect->left > pSourceRect->right)) {
            blit.src.box.x = pSourceRect->right;
            blit.src.box.width = pSourceRect->left - pSourceRect->right;
        } else {
            blit.src.box.x = pSourceRect->left;
            blit.src.box.width = pSourceRect->right - pSourceRect->left;
        }
        if (flip_y ^ (pSourceRect->top > pSourceRect->bottom)) {
            blit.src.box.y = pSourceRect->bottom;
            blit.src.box.height = pSourceRect->top - pSourceRect->bottom;
        } else {
            blit.src.box.y = pSourceRect->top;
            blit.src.box.height = pSourceRect->bottom - pSourceRect->top;
        }
    } else {
        blit.src.box.x = flip_x ? src->desc.Width : 0;
        blit.src.box.y = flip_y ? src->desc.Height : 0;
        blit.src.box.width = flip_x ? -src->desc.Width : src->desc.Width;
        blit.src.box.height = flip_y ? -src->desc.Height : src->desc.Height;
    }
    blit.mask = zs ? PIPE_MASK_ZS : PIPE_MASK_RGBA;
    blit.filter = Filter == D3DTEXF_LINEAR ?
       PIPE_TEX_FILTER_LINEAR : PIPE_TEX_FILTER_NEAREST;
    blit.scissor_enable = FALSE;

    /* If both of a src and dst dimension are negative, flip them. */
    if (blit.dst.box.width < 0 && blit.src.box.width < 0) {
        blit.dst.box.width = -blit.dst.box.width;
        blit.src.box.width = -blit.src.box.width;
    }
    if (blit.dst.box.height < 0 && blit.src.box.height < 0) {
        blit.dst.box.height = -blit.dst.box.height;
        blit.src.box.height = -blit.src.box.height;
    }
    scaled =
        blit.dst.box.width != blit.src.box.width ||
        blit.dst.box.height != blit.src.box.height;

    user_assert(!scaled || dst != src, D3DERR_INVALIDCALL);
    user_assert(!scaled ||
                !NineSurface9_IsOffscreenPlain(dst) ||
                NineSurface9_IsOffscreenPlain(src), D3DERR_INVALIDCALL);
    user_assert(!scaled ||
                (!util_format_is_compressed(dst->base.info.format) &&
                 !util_format_is_compressed(src->base.info.format)),
                D3DERR_INVALIDCALL);

    user_warn(src == dst &&
              u_box_test_intersection_2d(&blit.src.box, &blit.dst.box));

    /* Check for clipping/clamping: */
    {
        struct pipe_box box;
        int xy;

        xy = u_box_clip_2d(&box, &blit.dst.box,
                           dst->desc.Width, dst->desc.Height);
        if (xy < 0)
            return D3D_OK;
        if (xy == 0)
            xy = u_box_clip_2d(&box, &blit.src.box,
                               src->desc.Width, src->desc.Height);
        clamped = !!xy;
    }

    ms = (dst->desc.MultiSampleType | 1) != (src->desc.MultiSampleType | 1);

    if (clamped || scaled || (blit.dst.format != blit.src.format) || ms) {
        DBG("using pipe->blit()\n");
        /* TODO: software scaling */
        user_assert(screen->is_format_supported(screen, dst_res->format,
                                                dst_res->target,
                                                dst_res->nr_samples,
                                                zs ? PIPE_BIND_DEPTH_STENCIL :
                                                PIPE_BIND_RENDER_TARGET),
                    D3DERR_INVALIDCALL);

        pipe->blit(pipe, &blit);
    } else {
        assert(blit.dst.box.x >= 0 && blit.dst.box.y >= 0 &&
               blit.src.box.x >= 0 && blit.src.box.y >= 0 &&
               blit.dst.box.x + blit.dst.box.width <= dst->desc.Width &&
               blit.src.box.x + blit.src.box.width <= src->desc.Width &&
               blit.dst.box.y + blit.dst.box.height <= dst->desc.Height &&
               blit.src.box.y + blit.src.box.height <= src->desc.Height);
        /* Or drivers might crash ... */
        DBG("Using resource_copy_region.\n");
        pipe->resource_copy_region(pipe,
            blit.dst.resource, blit.dst.level,
            blit.dst.box.x, blit.dst.box.y, blit.dst.box.z,
            blit.src.resource, blit.src.level,
            &blit.src.box);
    }

    /* Communicate the container it needs to update sublevels - if apply */
    NineSurface9_MarkContainerDirty(dst);

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_ColorFill( struct NineDevice9 *This,
                       IDirect3DSurface9 *pSurface,
                       const RECT *pRect,
                       D3DCOLOR color )
{
    struct pipe_context *pipe = This->pipe;
    struct NineSurface9 *surf = NineSurface9(pSurface);
    struct pipe_surface *psurf;
    unsigned x, y, w, h;
    union pipe_color_union rgba;
    boolean fallback;

    DBG("This=%p pSurface=%p pRect=%p color=%08x\n", This,
        pSurface, pRect, color);
    if (pRect)
        DBG("pRect=(%u,%u)-(%u,%u)\n", pRect->left, pRect->top,
            pRect->right, pRect->bottom);

    user_assert(surf->base.pool == D3DPOOL_DEFAULT, D3DERR_INVALIDCALL);

    user_assert((surf->base.usage & D3DUSAGE_RENDERTARGET) ||
                NineSurface9_IsOffscreenPlain(surf), D3DERR_INVALIDCALL);

    if (pRect) {
        x = pRect->left;
        y = pRect->top;
        w = pRect->right - pRect->left;
        h = pRect->bottom - pRect->top;
    } else{
        x = 0;
        y = 0;
        w = surf->desc.Width;
        h = surf->desc.Height;
    }
    d3dcolor_to_pipe_color_union(&rgba, color);

    fallback =
        !This->screen->is_format_supported(This->screen, surf->base.info.format,
                                           surf->base.info.target,
                                           surf->base.info.nr_samples,
                                           PIPE_BIND_RENDER_TARGET);
    if (!fallback) {
        psurf = NineSurface9_GetSurface(surf, 0);
        if (!psurf)
            fallback = TRUE;
    }

    if (!fallback) {
        pipe->clear_render_target(pipe, psurf, &rgba, x, y, w, h);
    } else {
        D3DLOCKED_RECT lock;
        union util_color uc;
        HRESULT hr;
        /* XXX: lock pRect and fix util_fill_rect */
        hr = NineSurface9_LockRect(surf, &lock, NULL, 0);
        if (FAILED(hr))
            return hr;
        util_pack_color_ub(color >> 16, color >> 8, color >> 0, color >> 24,
                           surf->base.info.format, &uc);
        util_fill_rect(lock.pBits, surf->base.info.format,lock.Pitch,
                       x, y, w, h, &uc);
        NineSurface9_UnlockRect(surf);
    }

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_CreateOffscreenPlainSurface( struct NineDevice9 *This,
                                         UINT Width,
                                         UINT Height,
                                         D3DFORMAT Format,
                                         D3DPOOL Pool,
                                         IDirect3DSurface9 **ppSurface,
                                         HANDLE *pSharedHandle )
{
    HRESULT hr;

    DBG("This=%p Width=%u Height=%u Format=%s(0x%x) Pool=%u "
        "ppSurface=%p pSharedHandle=%p\n", This,
        Width, Height, d3dformat_to_string(Format), Format, Pool,
        ppSurface, pSharedHandle);

    *ppSurface = NULL;
    user_assert(!pSharedHandle || Pool == D3DPOOL_DEFAULT
                               || Pool == D3DPOOL_SYSTEMMEM, D3DERR_INVALIDCALL);
    user_assert(Pool != D3DPOOL_MANAGED, D3DERR_INVALIDCALL);

    /* Can be used with StretchRect and ColorFill. It's also always lockable.
     */
    hr = create_zs_or_rt_surface(This, 2, Pool, Width, Height,
                                 Format,
                                 D3DMULTISAMPLE_NONE, 0,
                                 TRUE,
                                 ppSurface, pSharedHandle);
    if (FAILED(hr))
        DBG("Failed to create surface.\n");
    return hr;
}

HRESULT WINAPI
NineDevice9_SetRenderTarget( struct NineDevice9 *This,
                             DWORD RenderTargetIndex,
                             IDirect3DSurface9 *pRenderTarget )
{
    struct NineSurface9 *rt = NineSurface9(pRenderTarget);
    const unsigned i = RenderTargetIndex;

    DBG("This=%p RenderTargetIndex=%u pRenderTarget=%p\n", This,
        RenderTargetIndex, pRenderTarget);

    user_assert(i < This->caps.NumSimultaneousRTs, D3DERR_INVALIDCALL);
    user_assert(i != 0 || pRenderTarget, D3DERR_INVALIDCALL);
    user_assert(!pRenderTarget ||
                rt->desc.Usage & D3DUSAGE_RENDERTARGET, D3DERR_INVALIDCALL);

    if (i == 0) {
        This->state.viewport.X = 0;
        This->state.viewport.Y = 0;
        This->state.viewport.Width = rt->desc.Width;
        This->state.viewport.Height = rt->desc.Height;
        This->state.viewport.MinZ = 0.0f;
        This->state.viewport.MaxZ = 1.0f;

        This->state.scissor.minx = 0;
        This->state.scissor.miny = 0;
        This->state.scissor.maxx = rt->desc.Width;
        This->state.scissor.maxy = rt->desc.Height;

        This->state.changed.group |= NINE_STATE_VIEWPORT | NINE_STATE_SCISSOR;
    }

    if (This->state.rt[i] != NineSurface9(pRenderTarget)) {
       nine_bind(&This->state.rt[i], pRenderTarget);
       This->state.changed.group |= NINE_STATE_FB;
    }
    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetRenderTarget( struct NineDevice9 *This,
                             DWORD RenderTargetIndex,
                             IDirect3DSurface9 **ppRenderTarget )
{
    const unsigned i = RenderTargetIndex;

    user_assert(i < This->caps.NumSimultaneousRTs, D3DERR_INVALIDCALL);
    user_assert(ppRenderTarget, D3DERR_INVALIDCALL);

    *ppRenderTarget = (IDirect3DSurface9 *)This->state.rt[i];
    if (!This->state.rt[i])
        return D3DERR_NOTFOUND;

    NineUnknown_AddRef(NineUnknown(This->state.rt[i]));
    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_SetDepthStencilSurface( struct NineDevice9 *This,
                                    IDirect3DSurface9 *pNewZStencil )
{
    DBG("This=%p pNewZStencil=%p\n", This, pNewZStencil);

    if (This->state.ds != NineSurface9(pNewZStencil)) {
        nine_bind(&This->state.ds, pNewZStencil);
        This->state.changed.group |= NINE_STATE_FB;
    }
    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetDepthStencilSurface( struct NineDevice9 *This,
                                    IDirect3DSurface9 **ppZStencilSurface )
{
    user_assert(ppZStencilSurface, D3DERR_INVALIDCALL);

    *ppZStencilSurface = (IDirect3DSurface9 *)This->state.ds;
    if (!This->state.ds)
        return D3DERR_NOTFOUND;

    NineUnknown_AddRef(NineUnknown(This->state.ds));
    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_BeginScene( struct NineDevice9 *This )
{
    DBG("This=%p\n", This);
    user_assert(!This->in_scene, D3DERR_INVALIDCALL);
    This->in_scene = TRUE;
    /* Do we want to do anything else here ? */
    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_EndScene( struct NineDevice9 *This )
{
    DBG("This=%p\n", This);
    user_assert(This->in_scene, D3DERR_INVALIDCALL);
    This->in_scene = FALSE;
    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_Clear( struct NineDevice9 *This,
                   DWORD Count,
                   const D3DRECT *pRects,
                   DWORD Flags,
                   D3DCOLOR Color,
                   float Z,
                   DWORD Stencil )
{
    const int sRGB = This->state.rs[D3DRS_SRGBWRITEENABLE] ? 1 : 0;
    struct pipe_surface *cbuf, *zsbuf;
    struct pipe_context *pipe = This->pipe;
    struct NineSurface9 *zsbuf_surf = This->state.ds;
    struct NineSurface9 *rt;
    unsigned bufs = 0;
    unsigned r, i;
    union pipe_color_union rgba;
    unsigned rt_mask = 0;
    D3DRECT rect;

    DBG("This=%p Count=%u pRects=%p Flags=%x Color=%08x Z=%f Stencil=%x\n",
        This, Count, pRects, Flags, Color, Z, Stencil);

    user_assert(This->state.ds || !(Flags & NINED3DCLEAR_DEPTHSTENCIL),
                D3DERR_INVALIDCALL);
    user_assert(!(Flags & D3DCLEAR_STENCIL) ||
                (zsbuf_surf &&
                 util_format_is_depth_and_stencil(zsbuf_surf->base.info.format)),
                D3DERR_INVALIDCALL);
#ifdef NINE_STRICT
    user_assert((Count && pRects) || (!Count && !pRects), D3DERR_INVALIDCALL);
#else
    user_warn((pRects && !Count) || (!pRects && Count));
    if (pRects && !Count)
        return D3D_OK;
    if (!pRects)
        Count = 0;
#endif

    if (Flags & D3DCLEAR_TARGET) bufs |= PIPE_CLEAR_COLOR;
    if (Flags & D3DCLEAR_ZBUFFER) bufs |= PIPE_CLEAR_DEPTH;
    if (Flags & D3DCLEAR_STENCIL) bufs |= PIPE_CLEAR_STENCIL;
    if (!bufs)
        return D3D_OK;
    d3dcolor_to_pipe_color_union(&rgba, Color);

    nine_update_state(This, NINE_STATE_FB);

    rect.x1 = This->state.viewport.X;
    rect.y1 = This->state.viewport.Y;
    rect.x2 = This->state.viewport.Width + rect.x1;
    rect.y2 = This->state.viewport.Height + rect.y1;

    /* Both rectangles apply, which is weird, but that's D3D9. */
    if (This->state.rs[D3DRS_SCISSORTESTENABLE]) {
        rect.x1 = MAX2(rect.x1, This->state.scissor.minx);
        rect.y1 = MAX2(rect.y1, This->state.scissor.miny);
        rect.x2 = MIN2(rect.x2, This->state.scissor.maxx);
        rect.y2 = MIN2(rect.y2, This->state.scissor.maxy);
    }

    if (Count) {
        /* Maybe apps like to specify a large rect ? */
        if (pRects[0].x1 <= rect.x1 && pRects[0].x2 >= rect.x2 &&
            pRects[0].y1 <= rect.y1 && pRects[0].y2 >= rect.y2) {
            DBG("First rect covers viewport.\n");
            Count = 0;
            pRects = NULL;
        }
    }

    if (rect.x1 >= This->state.fb.width || rect.y1 >= This->state.fb.height)
        return D3D_OK;

    for (i = 0; i < This->caps.NumSimultaneousRTs; ++i) {
        if (This->state.rt[i] && This->state.rt[i]->desc.Format != D3DFMT_NULL)
            rt_mask |= 1 << i;
    }

    /* fast path, clears everything at once */
    if (!Count &&
        (!(bufs & PIPE_CLEAR_COLOR) || (rt_mask == This->state.rt_mask)) &&
        rect.x1 == 0 && rect.y1 == 0 &&
        /* Case we clear only render target. Check clear region vs rt. */
        ((!(bufs & (PIPE_CLEAR_DEPTH | PIPE_CLEAR_STENCIL)) &&
         rect.x2 >= This->state.fb.width &&
         rect.y2 >= This->state.fb.height) ||
        /* Case we clear depth buffer (and eventually rt too).
         * depth buffer size is always >= rt size. Compare to clear region */
        ((bufs & (PIPE_CLEAR_DEPTH | PIPE_CLEAR_STENCIL)) &&
         This->state.fb.zsbuf != NULL &&
         rect.x2 >= zsbuf_surf->desc.Width &&
         rect.y2 >= zsbuf_surf->desc.Height))) {
        DBG("Clear fast path\n");
        pipe->clear(pipe, bufs, &rgba, Z, Stencil);
        return D3D_OK;
    }

    if (!Count) {
        Count = 1;
        pRects = &rect;
    }

    for (i = 0; i < This->caps.NumSimultaneousRTs; ++i) {
        rt = This->state.rt[i];
        if (!rt || rt->desc.Format == D3DFMT_NULL ||
            !(Flags & D3DCLEAR_TARGET))
            continue; /* save space, compiler should hoist this */
        cbuf = NineSurface9_GetSurface(rt, sRGB);
        for (r = 0; r < Count; ++r) {
            /* Don't trust users to pass these in the right order. */
            unsigned x1 = MIN2(pRects[r].x1, pRects[r].x2);
            unsigned y1 = MIN2(pRects[r].y1, pRects[r].y2);
            unsigned x2 = MAX2(pRects[r].x1, pRects[r].x2);
            unsigned y2 = MAX2(pRects[r].y1, pRects[r].y2);
#ifndef NINE_LAX
            /* Drop negative rectangles (like wine expects). */
            if (pRects[r].x1 > pRects[r].x2) continue;
            if (pRects[r].y1 > pRects[r].y2) continue;
#endif

            x1 = MAX2(x1, rect.x1);
            y1 = MAX2(y1, rect.y1);
            x2 = MIN3(x2, rect.x2, rt->desc.Width);
            y2 = MIN3(y2, rect.y2, rt->desc.Height);

            DBG("Clearing (%u..%u)x(%u..%u)\n", x1, x2, y1, y2);
            pipe->clear_render_target(pipe, cbuf, &rgba,
                                      x1, y1, x2 - x1, y2 - y1);
        }
    }
    if (!(Flags & NINED3DCLEAR_DEPTHSTENCIL))
        return D3D_OK;

    bufs &= PIPE_CLEAR_DEPTHSTENCIL;

    for (r = 0; r < Count; ++r) {
        unsigned x1 = MIN2(pRects[r].x1, pRects[r].x2);
        unsigned y1 = MIN2(pRects[r].y1, pRects[r].y2);
        unsigned x2 = MAX2(pRects[r].x1, pRects[r].x2);
        unsigned y2 = MAX2(pRects[r].y1, pRects[r].y2);
#ifndef NINE_LAX
        /* Drop negative rectangles. */
        if (pRects[r].x1 > pRects[r].x2) continue;
        if (pRects[r].y1 > pRects[r].y2) continue;
#endif

        x1 = MIN2(x1, rect.x1);
        y1 = MIN2(y1, rect.y1);
        x2 = MIN3(x2, rect.x2, zsbuf_surf->desc.Width);
        y2 = MIN3(y2, rect.y2, zsbuf_surf->desc.Height);

        zsbuf = NineSurface9_GetSurface(zsbuf_surf, 0);
        assert(zsbuf);
        pipe->clear_depth_stencil(pipe, zsbuf, bufs, Z, Stencil,
                                  x1, y1, x2 - x1, y2 - y1);
    }
    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_SetTransform( struct NineDevice9 *This,
                          D3DTRANSFORMSTATETYPE State,
                          const D3DMATRIX *pMatrix )
{
    struct nine_state *state = This->update;
    D3DMATRIX *M = nine_state_access_transform(state, State, TRUE);

    DBG("This=%p State=%d pMatrix=%p\n", This, State, pMatrix);

    user_assert(M, D3DERR_INVALIDCALL);

    *M = *pMatrix;
    state->ff.changed.transform[State / 32] |= 1 << (State % 32);
    state->changed.group |= NINE_STATE_FF;

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetTransform( struct NineDevice9 *This,
                          D3DTRANSFORMSTATETYPE State,
                          D3DMATRIX *pMatrix )
{
    D3DMATRIX *M = nine_state_access_transform(&This->state, State, FALSE);
    user_assert(M, D3DERR_INVALIDCALL);
    *pMatrix = *M;
    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_MultiplyTransform( struct NineDevice9 *This,
                               D3DTRANSFORMSTATETYPE State,
                               const D3DMATRIX *pMatrix )
{
    struct nine_state *state = This->update;
    D3DMATRIX T;
    D3DMATRIX *M = nine_state_access_transform(state, State, TRUE);

    DBG("This=%p State=%d pMatrix=%p\n", This, State, pMatrix);

    user_assert(M, D3DERR_INVALIDCALL);

    nine_d3d_matrix_matrix_mul(&T, pMatrix, M);
    return NineDevice9_SetTransform(This, State, &T);
}

HRESULT WINAPI
NineDevice9_SetViewport( struct NineDevice9 *This,
                         const D3DVIEWPORT9 *pViewport )
{
    struct nine_state *state = This->update;

    DBG("X=%u Y=%u W=%u H=%u MinZ=%f MaxZ=%f\n",
        pViewport->X, pViewport->Y, pViewport->Width, pViewport->Height,
        pViewport->MinZ, pViewport->MaxZ);

    state->viewport = *pViewport;
    state->changed.group |= NINE_STATE_VIEWPORT;

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetViewport( struct NineDevice9 *This,
                         D3DVIEWPORT9 *pViewport )
{
    *pViewport = This->state.viewport;
    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_SetMaterial( struct NineDevice9 *This,
                         const D3DMATERIAL9 *pMaterial )
{
    struct nine_state *state = This->update;

    DBG("This=%p pMaterial=%p\n", This, pMaterial);
    if (pMaterial)
        nine_dump_D3DMATERIAL9(DBG_FF, pMaterial);

    user_assert(pMaterial, E_POINTER);

    state->ff.material = *pMaterial;
    state->changed.group |= NINE_STATE_FF_MATERIAL;

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetMaterial( struct NineDevice9 *This,
                         D3DMATERIAL9 *pMaterial )
{
    user_assert(pMaterial, E_POINTER);
    *pMaterial = This->state.ff.material;
    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_SetLight( struct NineDevice9 *This,
                      DWORD Index,
                      const D3DLIGHT9 *pLight )
{
    struct nine_state *state = This->update;

    DBG("This=%p Index=%u pLight=%p\n", This, Index, pLight);
    if (pLight)
        nine_dump_D3DLIGHT9(DBG_FF, pLight);

    user_assert(pLight, D3DERR_INVALIDCALL);
    user_assert(pLight->Type < NINED3DLIGHT_INVALID, D3DERR_INVALIDCALL);

    user_assert(Index < NINE_MAX_LIGHTS, D3DERR_INVALIDCALL); /* sanity */

    if (Index >= state->ff.num_lights) {
        unsigned n = state->ff.num_lights;
        unsigned N = Index + 1;

        state->ff.light = REALLOC(state->ff.light, n * sizeof(D3DLIGHT9),
                                                   N * sizeof(D3DLIGHT9));
        if (!state->ff.light)
            return E_OUTOFMEMORY;
        state->ff.num_lights = N;

        for (; n < Index; ++n)
            state->ff.light[n].Type = (D3DLIGHTTYPE)NINED3DLIGHT_INVALID;
    }
    state->ff.light[Index] = *pLight;

    if (pLight->Type == D3DLIGHT_SPOT && pLight->Theta >= pLight->Phi) {
        DBG("Warning: clamping D3DLIGHT9.Theta\n");
        state->ff.light[Index].Theta = state->ff.light[Index].Phi;
    }
    if (pLight->Type != D3DLIGHT_DIRECTIONAL &&
        pLight->Attenuation0 == 0.0f &&
        pLight->Attenuation1 == 0.0f &&
        pLight->Attenuation2 == 0.0f) {
        DBG("Warning: all D3DLIGHT9.Attenuation[i] are 0\n");
    }

    state->changed.group |= NINE_STATE_FF_LIGHTING;

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetLight( struct NineDevice9 *This,
                      DWORD Index,
                      D3DLIGHT9 *pLight )
{
    const struct nine_state *state = &This->state;

    user_assert(pLight, D3DERR_INVALIDCALL);
    user_assert(Index < state->ff.num_lights, D3DERR_INVALIDCALL);
    user_assert(state->ff.light[Index].Type < NINED3DLIGHT_INVALID,
                D3DERR_INVALIDCALL);

    *pLight = state->ff.light[Index];

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_LightEnable( struct NineDevice9 *This,
                         DWORD Index,
                         BOOL Enable )
{
    struct nine_state *state = This->update;
    unsigned i;

    DBG("This=%p Index=%u Enable=%i\n", This, Index, Enable);

    if (Index >= state->ff.num_lights ||
        state->ff.light[Index].Type == NINED3DLIGHT_INVALID) {
        /* This should create a default light. */
        D3DLIGHT9 light;
        memset(&light, 0, sizeof(light));
        light.Type = D3DLIGHT_DIRECTIONAL;
        light.Diffuse.r = 1.0f;
        light.Diffuse.g = 1.0f;
        light.Diffuse.b = 1.0f;
        light.Direction.z = 1.0f;
        NineDevice9_SetLight(This, Index, &light);
    }
    user_assert(Index < state->ff.num_lights, D3DERR_INVALIDCALL);

    for (i = 0; i < state->ff.num_lights_active; ++i) {
        if (state->ff.active_light[i] == Index)
            break;
    }

    if (Enable) {
        if (i < state->ff.num_lights_active)
            return D3D_OK;
        /* XXX wine thinks this should still succeed:
         */
        user_assert(i < NINE_MAX_LIGHTS_ACTIVE, D3DERR_INVALIDCALL);

        state->ff.active_light[i] = Index;
        state->ff.num_lights_active++;
    } else {
        if (i == state->ff.num_lights_active)
            return D3D_OK;
        --state->ff.num_lights_active;
        for (; i < state->ff.num_lights_active; ++i)
            state->ff.active_light[i] = state->ff.active_light[i + 1];
    }
    state->changed.group |= NINE_STATE_FF_LIGHTING;

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetLightEnable( struct NineDevice9 *This,
                            DWORD Index,
                            BOOL *pEnable )
{
    const struct nine_state *state = &This->state;
    unsigned i;

    user_assert(Index < state->ff.num_lights, D3DERR_INVALIDCALL);
    user_assert(state->ff.light[Index].Type < NINED3DLIGHT_INVALID,
                D3DERR_INVALIDCALL);

    for (i = 0; i < state->ff.num_lights_active; ++i)
        if (state->ff.active_light[i] == Index)
            break;

    *pEnable = i != state->ff.num_lights_active ? 128 : 0; // Taken from wine

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_SetClipPlane( struct NineDevice9 *This,
                          DWORD Index,
                          const float *pPlane )
{
    struct nine_state *state = This->update;

    user_assert(pPlane, D3DERR_INVALIDCALL);

    DBG("This=%p Index=%u pPlane=%f %f %f %f\n", This, Index,
        pPlane[0], pPlane[1],
        pPlane[2], pPlane[3]);

    user_assert(Index < PIPE_MAX_CLIP_PLANES, D3DERR_INVALIDCALL);

    memcpy(&state->clip.ucp[Index][0], pPlane, sizeof(state->clip.ucp[0]));
    state->changed.ucp |= 1 << Index;

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetClipPlane( struct NineDevice9 *This,
                          DWORD Index,
                          float *pPlane )
{
    const struct nine_state *state = &This->state;

    user_assert(Index < PIPE_MAX_CLIP_PLANES, D3DERR_INVALIDCALL);

    memcpy(pPlane, &state->clip.ucp[Index][0], sizeof(state->clip.ucp[0]));
    return D3D_OK;
}

#define RESZ_CODE 0x7fa05000

static HRESULT
NineDevice9_ResolveZ( struct NineDevice9 *This )
{
    struct nine_state *state = &This->state;
    const struct util_format_description *desc;
    struct NineSurface9 *source = state->ds;
    struct NineBaseTexture9 *destination = state->texture[0];
    struct pipe_resource *src, *dst;
    struct pipe_blit_info blit;

    DBG("RESZ resolve\n");

    user_assert(source && destination &&
                destination->base.type == D3DRTYPE_TEXTURE, D3DERR_INVALIDCALL);

    src = source->base.resource;
    dst = destination->base.resource;

    user_assert(src && dst, D3DERR_INVALIDCALL);

    /* check dst is depth format. we know already for src */
    desc = util_format_description(dst->format);
    user_assert(desc->colorspace == UTIL_FORMAT_COLORSPACE_ZS, D3DERR_INVALIDCALL);

    memset(&blit, 0, sizeof(blit));
    blit.src.resource = src;
    blit.src.level = 0;
    blit.src.format = src->format;
    blit.src.box.z = 0;
    blit.src.box.depth = 1;
    blit.src.box.x = 0;
    blit.src.box.y = 0;
    blit.src.box.width = src->width0;
    blit.src.box.height = src->height0;

    blit.dst.resource = dst;
    blit.dst.level = 0;
    blit.dst.format = dst->format;
    blit.dst.box.z = 0;
    blit.dst.box.depth = 1;
    blit.dst.box.x = 0;
    blit.dst.box.y = 0;
    blit.dst.box.width = dst->width0;
    blit.dst.box.height = dst->height0;

    blit.mask = PIPE_MASK_ZS;
    blit.filter = PIPE_TEX_FILTER_NEAREST;
    blit.scissor_enable = FALSE;

    This->pipe->blit(This->pipe, &blit);
    return D3D_OK;
}

#define ALPHA_TO_COVERAGE_ENABLE   MAKEFOURCC('A', '2', 'M', '1')
#define ALPHA_TO_COVERAGE_DISABLE  MAKEFOURCC('A', '2', 'M', '0')

HRESULT WINAPI
NineDevice9_SetRenderState( struct NineDevice9 *This,
                            D3DRENDERSTATETYPE State,
                            DWORD Value )
{
    struct nine_state *state = This->update;

    DBG("This=%p State=%u(%s) Value=%08x\n", This,
        State, nine_d3drs_to_string(State), Value);

    /* Amd hacks (equivalent to GL extensions) */
    if (State == D3DRS_POINTSIZE) {
        if (Value == RESZ_CODE)
            return NineDevice9_ResolveZ(This);

        if (Value == ALPHA_TO_COVERAGE_ENABLE ||
            Value == ALPHA_TO_COVERAGE_DISABLE) {
            state->rs[NINED3DRS_ALPHACOVERAGE] = (Value == ALPHA_TO_COVERAGE_ENABLE);
            state->changed.group |= NINE_STATE_BLEND;
            return D3D_OK;
        }
    }

    /* NV hack */
    if (State == D3DRS_ADAPTIVETESS_Y &&
        (Value == D3DFMT_ATOC || (Value == D3DFMT_UNKNOWN && state->rs[NINED3DRS_ALPHACOVERAGE]))) {
            state->rs[NINED3DRS_ALPHACOVERAGE] = (Value == D3DFMT_ATOC);
            state->changed.group |= NINE_STATE_BLEND;
            return D3D_OK;
    }

    user_assert(State < Elements(state->rs), D3DERR_INVALIDCALL);

    if (likely(state->rs[State] != Value) || unlikely(This->is_recording)) {
        state->rs[State] = Value;
        state->changed.rs[State / 32] |= 1 << (State % 32);
        state->changed.group |= nine_render_state_group[State];
    }

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetRenderState( struct NineDevice9 *This,
                            D3DRENDERSTATETYPE State,
                            DWORD *pValue )
{
    user_assert(State < Elements(This->state.rs), D3DERR_INVALIDCALL);

    *pValue = This->state.rs[State];
    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_CreateStateBlock( struct NineDevice9 *This,
                              D3DSTATEBLOCKTYPE Type,
                              IDirect3DStateBlock9 **ppSB )
{
    struct NineStateBlock9 *nsb;
    struct nine_state *dst;
    HRESULT hr;
    enum nine_stateblock_type type;
    unsigned s;

    DBG("This=%p Type=%u ppSB=%p\n", This, Type, ppSB);

    user_assert(Type == D3DSBT_ALL ||
                Type == D3DSBT_VERTEXSTATE ||
                Type == D3DSBT_PIXELSTATE, D3DERR_INVALIDCALL);

    switch (Type) {
    case D3DSBT_VERTEXSTATE: type = NINESBT_VERTEXSTATE; break;
    case D3DSBT_PIXELSTATE:  type = NINESBT_PIXELSTATE; break;
    default:
       type = NINESBT_ALL;
       break;
    }

    hr = NineStateBlock9_new(This, &nsb, type);
    if (FAILED(hr))
       return hr;
    *ppSB = (IDirect3DStateBlock9 *)nsb;
    dst = &nsb->state;

    dst->changed.group =
       NINE_STATE_TEXTURE |
       NINE_STATE_SAMPLER;

    if (Type == D3DSBT_ALL || Type == D3DSBT_VERTEXSTATE) {
       dst->changed.group |=
           NINE_STATE_FF_LIGHTING |
           NINE_STATE_VS | NINE_STATE_VS_CONST |
           NINE_STATE_VDECL;
       /* TODO: texture/sampler state */
       memcpy(dst->changed.rs,
              nine_render_states_vertex, sizeof(dst->changed.rs));
       nine_ranges_insert(&dst->changed.vs_const_f, 0, This->max_vs_const_f,
                          &This->range_pool);
       dst->changed.vs_const_i = 0xffff;
       dst->changed.vs_const_b = 0xffff;
       for (s = 0; s < NINE_MAX_SAMPLERS; ++s)
           dst->changed.sampler[s] |= 1 << D3DSAMP_DMAPOFFSET;
       if (This->state.ff.num_lights) {
           dst->ff.num_lights = This->state.ff.num_lights;
           /* zero'd -> light type won't be NINED3DLIGHT_INVALID, so
            * all currently existing lights will be captured
            */
           dst->ff.light = CALLOC(This->state.ff.num_lights,
                                  sizeof(D3DLIGHT9));
           if (!dst->ff.light) {
               nine_bind(ppSB, NULL);
               return E_OUTOFMEMORY;
           }
       }
    }
    if (Type == D3DSBT_ALL || Type == D3DSBT_PIXELSTATE) {
       dst->changed.group |=
          NINE_STATE_PS | NINE_STATE_PS_CONST;
       /* TODO: texture/sampler state */
       memcpy(dst->changed.rs,
              nine_render_states_pixel, sizeof(dst->changed.rs));
       nine_ranges_insert(&dst->changed.ps_const_f, 0, This->max_ps_const_f,
                          &This->range_pool);
       dst->changed.ps_const_i = 0xffff;
       dst->changed.ps_const_b = 0xffff;
       for (s = 0; s < NINE_MAX_SAMPLERS; ++s)
           dst->changed.sampler[s] |= 0x1ffe;
    }
    if (Type == D3DSBT_ALL) {
       dst->changed.group |=
          NINE_STATE_VIEWPORT |
          NINE_STATE_SCISSOR |
          NINE_STATE_RASTERIZER |
          NINE_STATE_BLEND |
          NINE_STATE_DSA |
          NINE_STATE_IDXBUF |
          NINE_STATE_MATERIAL |
          NINE_STATE_BLEND_COLOR |
          NINE_STATE_SAMPLE_MASK;
       memset(dst->changed.rs, ~0, (D3DRS_COUNT / 32) * sizeof(uint32_t));
       dst->changed.rs[D3DRS_LAST / 32] |= (1 << (D3DRS_COUNT % 32)) - 1;
       dst->changed.vtxbuf = (1ULL << This->caps.MaxStreams) - 1;
       dst->changed.stream_freq = dst->changed.vtxbuf;
       dst->changed.ucp = (1 << PIPE_MAX_CLIP_PLANES) - 1;
       dst->changed.texture = (1 << NINE_MAX_SAMPLERS) - 1;
    }
    NineStateBlock9_Capture(NineStateBlock9(*ppSB));

    /* TODO: fixed function state */

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_BeginStateBlock( struct NineDevice9 *This )
{
    HRESULT hr;

    DBG("This=%p\n", This);

    user_assert(!This->record, D3DERR_INVALIDCALL);

    hr = NineStateBlock9_new(This, &This->record, NINESBT_CUSTOM);
    if (FAILED(hr))
        return hr;
    NineUnknown_ConvertRefToBind(NineUnknown(This->record));

    This->update = &This->record->state;
    This->is_recording = TRUE;

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_EndStateBlock( struct NineDevice9 *This,
                           IDirect3DStateBlock9 **ppSB )
{
    DBG("This=%p ppSB=%p\n", This, ppSB);

    user_assert(This->record, D3DERR_INVALIDCALL);

    This->update = &This->state;
    This->is_recording = FALSE;

    NineUnknown_AddRef(NineUnknown(This->record));
    *ppSB = (IDirect3DStateBlock9 *)This->record;
    NineUnknown_Unbind(NineUnknown(This->record));
    This->record = NULL;

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_SetClipStatus( struct NineDevice9 *This,
                           const D3DCLIPSTATUS9 *pClipStatus )
{
    STUB(D3DERR_INVALIDCALL);
}

HRESULT WINAPI
NineDevice9_GetClipStatus( struct NineDevice9 *This,
                           D3DCLIPSTATUS9 *pClipStatus )
{
    STUB(D3DERR_INVALIDCALL);
}

HRESULT WINAPI
NineDevice9_GetTexture( struct NineDevice9 *This,
                        DWORD Stage,
                        IDirect3DBaseTexture9 **ppTexture )
{
    user_assert(Stage < This->caps.MaxSimultaneousTextures ||
                Stage == D3DDMAPSAMPLER ||
                (Stage >= D3DVERTEXTEXTURESAMPLER0 &&
                 Stage <= D3DVERTEXTEXTURESAMPLER3), D3DERR_INVALIDCALL);
    user_assert(ppTexture, D3DERR_INVALIDCALL);

    if (Stage >= D3DDMAPSAMPLER)
        Stage = Stage - D3DDMAPSAMPLER + NINE_MAX_SAMPLERS_PS;

    *ppTexture = (IDirect3DBaseTexture9 *)This->state.texture[Stage];

    if (This->state.texture[Stage])
        NineUnknown_AddRef(NineUnknown(This->state.texture[Stage]));
    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_SetTexture( struct NineDevice9 *This,
                        DWORD Stage,
                        IDirect3DBaseTexture9 *pTexture )
{
    struct nine_state *state = This->update;
    struct NineBaseTexture9 *tex = NineBaseTexture9(pTexture);

    DBG("This=%p Stage=%u pTexture=%p\n", This, Stage, pTexture);

    user_assert(Stage < This->caps.MaxSimultaneousTextures ||
                Stage == D3DDMAPSAMPLER ||
                (Stage >= D3DVERTEXTEXTURESAMPLER0 &&
                 Stage <= D3DVERTEXTEXTURESAMPLER3), D3DERR_INVALIDCALL);
    user_assert(!tex || (tex->base.pool != D3DPOOL_SCRATCH &&
                tex->base.pool != D3DPOOL_SYSTEMMEM), D3DERR_INVALIDCALL);

    if (Stage >= D3DDMAPSAMPLER)
        Stage = Stage - D3DDMAPSAMPLER + NINE_MAX_SAMPLERS_PS;

    if (!This->is_recording) {
        struct NineBaseTexture9 *old = state->texture[Stage];
        if (old == tex)
            return D3D_OK;

        state->samplers_shadow &= ~(1 << Stage);
        if (tex) {
            state->samplers_shadow |= tex->shadow << Stage;

            if ((tex->managed.dirty | tex->dirty_mip) && LIST_IS_EMPTY(&tex->list))
                list_add(&tex->list, &This->update_textures);

            tex->bind_count++;
        }
        if (old)
            old->bind_count--;
    }
    nine_bind(&state->texture[Stage], pTexture);

    state->changed.texture |= 1 << Stage;
    state->changed.group |= NINE_STATE_TEXTURE;

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetTextureStageState( struct NineDevice9 *This,
                                  DWORD Stage,
                                  D3DTEXTURESTAGESTATETYPE Type,
                                  DWORD *pValue )
{
    const struct nine_state *state = &This->state;

    user_assert(Stage < Elements(state->ff.tex_stage), D3DERR_INVALIDCALL);
    user_assert(Type < Elements(state->ff.tex_stage[0]), D3DERR_INVALIDCALL);

    *pValue = state->ff.tex_stage[Stage][Type];

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_SetTextureStageState( struct NineDevice9 *This,
                                  DWORD Stage,
                                  D3DTEXTURESTAGESTATETYPE Type,
                                  DWORD Value )
{
    struct nine_state *state = This->update;

    DBG("Stage=%u Type=%u Value=%08x\n", Stage, Type, Value);
    nine_dump_D3DTSS_value(DBG_FF, Type, Value);

    user_assert(Stage < Elements(state->ff.tex_stage), D3DERR_INVALIDCALL);
    user_assert(Type < Elements(state->ff.tex_stage[0]), D3DERR_INVALIDCALL);

    state->ff.tex_stage[Stage][Type] = Value;

    state->changed.group |= NINE_STATE_FF_PSSTAGES;
    state->ff.changed.tex_stage[Stage][Type / 32] |= 1 << (Type % 32);

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetSamplerState( struct NineDevice9 *This,
                             DWORD Sampler,
                             D3DSAMPLERSTATETYPE Type,
                             DWORD *pValue )
{
    user_assert(Sampler < This->caps.MaxSimultaneousTextures ||
                Sampler == D3DDMAPSAMPLER ||
                (Sampler >= D3DVERTEXTEXTURESAMPLER0 &&
                 Sampler <= D3DVERTEXTEXTURESAMPLER3), D3DERR_INVALIDCALL);

    if (Sampler >= D3DDMAPSAMPLER)
        Sampler = Sampler - D3DDMAPSAMPLER + NINE_MAX_SAMPLERS_PS;

    *pValue = This->state.samp[Sampler][Type];
    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_SetSamplerState( struct NineDevice9 *This,
                             DWORD Sampler,
                             D3DSAMPLERSTATETYPE Type,
                             DWORD Value )
{
    struct nine_state *state = This->update;

    DBG("This=%p Sampler=%u Type=%s Value=%08x\n", This,
        Sampler, nine_D3DSAMP_to_str(Type), Value);

    user_assert(Sampler < This->caps.MaxSimultaneousTextures ||
                Sampler == D3DDMAPSAMPLER ||
                (Sampler >= D3DVERTEXTEXTURESAMPLER0 &&
                 Sampler <= D3DVERTEXTEXTURESAMPLER3), D3DERR_INVALIDCALL);

    if (Sampler >= D3DDMAPSAMPLER)
        Sampler = Sampler - D3DDMAPSAMPLER + NINE_MAX_SAMPLERS_PS;

    state->samp[Sampler][Type] = Value;
    state->changed.group |= NINE_STATE_SAMPLER;
    state->changed.sampler[Sampler] |= 1 << Type;

    if (Type == D3DSAMP_SRGBTEXTURE)
        state->changed.srgb = TRUE;

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_ValidateDevice( struct NineDevice9 *This,
                            DWORD *pNumPasses )
{
    const struct nine_state *state = &This->state;
    unsigned i;
    unsigned w = 0, h = 0;

    DBG("This=%p pNumPasses=%p\n", This, pNumPasses);

    for (i = 0; i < Elements(state->samp); ++i) {
        if (state->samp[i][D3DSAMP_MINFILTER] == D3DTEXF_NONE ||
            state->samp[i][D3DSAMP_MAGFILTER] == D3DTEXF_NONE)
            return D3DERR_UNSUPPORTEDTEXTUREFILTER;
    }

    for (i = 0; i < This->caps.NumSimultaneousRTs; ++i) {
        if (!state->rt[i])
            continue;
        if (w == 0) {
            w = state->rt[i]->desc.Width;
            h = state->rt[i]->desc.Height;
        } else
        if (state->rt[i]->desc.Width != w || state->rt[i]->desc.Height != h) {
            return D3DERR_CONFLICTINGRENDERSTATE;
        }
    }
    if (state->ds &&
        (state->rs[D3DRS_ZENABLE] || state->rs[D3DRS_STENCILENABLE])) {
        if (w != 0 &&
            (state->ds->desc.Width != w || state->ds->desc.Height != h))
            return D3DERR_CONFLICTINGRENDERSTATE;
    }

    if (pNumPasses)
        *pNumPasses = 1;

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_SetPaletteEntries( struct NineDevice9 *This,
                               UINT PaletteNumber,
                               const PALETTEENTRY *pEntries )
{
    STUB(D3D_OK); /* like wine */
}

HRESULT WINAPI
NineDevice9_GetPaletteEntries( struct NineDevice9 *This,
                               UINT PaletteNumber,
                               PALETTEENTRY *pEntries )
{
    STUB(D3DERR_INVALIDCALL);
}

HRESULT WINAPI
NineDevice9_SetCurrentTexturePalette( struct NineDevice9 *This,
                                      UINT PaletteNumber )
{
    STUB(D3D_OK); /* like wine */
}

HRESULT WINAPI
NineDevice9_GetCurrentTexturePalette( struct NineDevice9 *This,
                                      UINT *PaletteNumber )
{
    STUB(D3DERR_INVALIDCALL);
}

HRESULT WINAPI
NineDevice9_SetScissorRect( struct NineDevice9 *This,
                            const RECT *pRect )
{
    struct nine_state *state = This->update;

    DBG("x=(%u..%u) y=(%u..%u)\n",
        pRect->left, pRect->top, pRect->right, pRect->bottom);

    state->scissor.minx = pRect->left;
    state->scissor.miny = pRect->top;
    state->scissor.maxx = pRect->right;
    state->scissor.maxy = pRect->bottom;

    state->changed.group |= NINE_STATE_SCISSOR;

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetScissorRect( struct NineDevice9 *This,
                            RECT *pRect )
{
    pRect->left   = This->state.scissor.minx;
    pRect->top    = This->state.scissor.miny;
    pRect->right  = This->state.scissor.maxx;
    pRect->bottom = This->state.scissor.maxy;

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_SetSoftwareVertexProcessing( struct NineDevice9 *This,
                                         BOOL bSoftware )
{
    STUB(D3DERR_INVALIDCALL);
}

BOOL WINAPI
NineDevice9_GetSoftwareVertexProcessing( struct NineDevice9 *This )
{
    return !!(This->params.BehaviorFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING);
}

HRESULT WINAPI
NineDevice9_SetNPatchMode( struct NineDevice9 *This,
                           float nSegments )
{
    STUB(D3DERR_INVALIDCALL);
}

float WINAPI
NineDevice9_GetNPatchMode( struct NineDevice9 *This )
{
    STUB(0);
}

static INLINE void
init_draw_info(struct pipe_draw_info *info,
               struct NineDevice9 *dev, D3DPRIMITIVETYPE type, UINT count)
{
    info->mode = d3dprimitivetype_to_pipe_prim(type);
    info->count = prim_count_to_vertex_count(type, count);
    info->start_instance = 0;
    info->instance_count = 1;
    if (dev->state.stream_instancedata_mask & dev->state.stream_usage_mask)
        info->instance_count = MAX2(dev->state.stream_freq[0] & 0x7FFFFF, 1);
    info->primitive_restart = FALSE;
    info->restart_index = 0;
    info->count_from_stream_output = NULL;
    info->indirect = NULL;
}

HRESULT WINAPI
NineDevice9_DrawPrimitive( struct NineDevice9 *This,
                           D3DPRIMITIVETYPE PrimitiveType,
                           UINT StartVertex,
                           UINT PrimitiveCount )
{
    struct pipe_draw_info info;

    DBG("iface %p, PrimitiveType %u, StartVertex %u, PrimitiveCount %u\n",
        This, PrimitiveType, StartVertex, PrimitiveCount);

    nine_update_state(This, ~0);

    init_draw_info(&info, This, PrimitiveType, PrimitiveCount);
    info.indexed = FALSE;
    info.start = StartVertex;
    info.index_bias = 0;
    info.min_index = info.start;
    info.max_index = info.count - 1;

    This->pipe->draw_vbo(This->pipe, &info);

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_DrawIndexedPrimitive( struct NineDevice9 *This,
                                  D3DPRIMITIVETYPE PrimitiveType,
                                  INT BaseVertexIndex,
                                  UINT MinVertexIndex,
                                  UINT NumVertices,
                                  UINT StartIndex,
                                  UINT PrimitiveCount )
{
    struct pipe_draw_info info;

    DBG("iface %p, PrimitiveType %u, BaseVertexIndex %u, MinVertexIndex %u "
        "NumVertices %u, StartIndex %u, PrimitiveCount %u\n",
        This, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices,
        StartIndex, PrimitiveCount);

    user_assert(This->state.idxbuf, D3DERR_INVALIDCALL);
    user_assert(This->state.vdecl, D3DERR_INVALIDCALL);

    nine_update_state(This, ~0);

    init_draw_info(&info, This, PrimitiveType, PrimitiveCount);
    info.indexed = TRUE;
    info.start = StartIndex;
    info.index_bias = BaseVertexIndex;
    /* These don't include index bias: */
    info.min_index = MinVertexIndex;
    info.max_index = MinVertexIndex + NumVertices - 1;

    This->pipe->draw_vbo(This->pipe, &info);

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_DrawPrimitiveUP( struct NineDevice9 *This,
                             D3DPRIMITIVETYPE PrimitiveType,
                             UINT PrimitiveCount,
                             const void *pVertexStreamZeroData,
                             UINT VertexStreamZeroStride )
{
    struct pipe_vertex_buffer vtxbuf;
    struct pipe_draw_info info;

    DBG("iface %p, PrimitiveType %u, PrimitiveCount %u, data %p, stride %u\n",
        This, PrimitiveType, PrimitiveCount,
        pVertexStreamZeroData, VertexStreamZeroStride);

    user_assert(pVertexStreamZeroData && VertexStreamZeroStride,
                D3DERR_INVALIDCALL);

    nine_update_state(This, ~0);

    init_draw_info(&info, This, PrimitiveType, PrimitiveCount);
    info.indexed = FALSE;
    info.start = 0;
    info.index_bias = 0;
    info.min_index = 0;
    info.max_index = info.count - 1;

    vtxbuf.stride = VertexStreamZeroStride;
    vtxbuf.buffer_offset = 0;
    vtxbuf.buffer = NULL;
    vtxbuf.user_buffer = pVertexStreamZeroData;

    if (!This->driver_caps.user_vbufs)
        u_upload_data(This->upload,
                      0,
                      (info.max_index + 1) * VertexStreamZeroStride, /* XXX */
                      vtxbuf.user_buffer,
                      &vtxbuf.buffer_offset,
                      &vtxbuf.buffer);

    This->pipe->set_vertex_buffers(This->pipe, 0, 1, &vtxbuf);

    This->pipe->draw_vbo(This->pipe, &info);

    NineDevice9_PauseRecording(This);
    NineDevice9_SetStreamSource(This, 0, NULL, 0, 0);
    NineDevice9_ResumeRecording(This);

    pipe_resource_reference(&vtxbuf.buffer, NULL);

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_DrawIndexedPrimitiveUP( struct NineDevice9 *This,
                                    D3DPRIMITIVETYPE PrimitiveType,
                                    UINT MinVertexIndex,
                                    UINT NumVertices,
                                    UINT PrimitiveCount,
                                    const void *pIndexData,
                                    D3DFORMAT IndexDataFormat,
                                    const void *pVertexStreamZeroData,
                                    UINT VertexStreamZeroStride )
{
    struct pipe_draw_info info;
    struct pipe_vertex_buffer vbuf;
    struct pipe_index_buffer ibuf;

    DBG("iface %p, PrimitiveType %u, MinVertexIndex %u, NumVertices %u "
        "PrimitiveCount %u, pIndexData %p, IndexDataFormat %u "
        "pVertexStreamZeroData %p, VertexStreamZeroStride %u\n",
        This, PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount,
        pIndexData, IndexDataFormat,
        pVertexStreamZeroData, VertexStreamZeroStride);

    user_assert(pIndexData && pVertexStreamZeroData, D3DERR_INVALIDCALL);
    user_assert(VertexStreamZeroStride, D3DERR_INVALIDCALL);
    user_assert(IndexDataFormat == D3DFMT_INDEX16 ||
                IndexDataFormat == D3DFMT_INDEX32, D3DERR_INVALIDCALL);

    nine_update_state(This, ~0);

    init_draw_info(&info, This, PrimitiveType, PrimitiveCount);
    info.indexed = TRUE;
    info.start = 0;
    info.index_bias = 0;
    info.min_index = MinVertexIndex;
    info.max_index = MinVertexIndex + NumVertices - 1;

    vbuf.stride = VertexStreamZeroStride;
    vbuf.buffer_offset = 0;
    vbuf.buffer = NULL;
    vbuf.user_buffer = pVertexStreamZeroData;

    ibuf.index_size = (IndexDataFormat == D3DFMT_INDEX16) ? 2 : 4;
    ibuf.offset = 0;
    ibuf.buffer = NULL;
    ibuf.user_buffer = pIndexData;

    if (!This->driver_caps.user_vbufs) {
        const unsigned base = info.min_index * VertexStreamZeroStride;
        u_upload_data(This->upload,
                      base,
                      (info.max_index -
                       info.min_index + 1) * VertexStreamZeroStride, /* XXX */
                      (const uint8_t *)vbuf.user_buffer + base,
                      &vbuf.buffer_offset,
                      &vbuf.buffer);
        /* Won't be used: */
        vbuf.buffer_offset -= base;
    }
    if (!This->driver_caps.user_ibufs)
        u_upload_data(This->upload,
                      0,
                      info.count * ibuf.index_size,
                      ibuf.user_buffer,
                      &ibuf.offset,
                      &ibuf.buffer);

    This->pipe->set_vertex_buffers(This->pipe, 0, 1, &vbuf);
    This->pipe->set_index_buffer(This->pipe, &ibuf);

    This->pipe->draw_vbo(This->pipe, &info);

    pipe_resource_reference(&vbuf.buffer, NULL);
    pipe_resource_reference(&ibuf.buffer, NULL);

    NineDevice9_PauseRecording(This);
    NineDevice9_SetIndices(This, NULL);
    NineDevice9_SetStreamSource(This, 0, NULL, 0, 0);
    NineDevice9_ResumeRecording(This);

    return D3D_OK;
}

/* TODO: Write to pDestBuffer directly if vertex declaration contains
 * only f32 formats.
 */
HRESULT WINAPI
NineDevice9_ProcessVertices( struct NineDevice9 *This,
                             UINT SrcStartIndex,
                             UINT DestIndex,
                             UINT VertexCount,
                             IDirect3DVertexBuffer9 *pDestBuffer,
                             IDirect3DVertexDeclaration9 *pVertexDecl,
                             DWORD Flags )
{
    struct pipe_screen *screen = This->screen;
    struct NineVertexDeclaration9 *vdecl = NineVertexDeclaration9(pVertexDecl);
    struct NineVertexShader9 *vs;
    struct pipe_resource *resource;
    struct pipe_stream_output_target *target;
    struct pipe_draw_info draw;
    HRESULT hr;
    unsigned buffer_offset, buffer_size;

    DBG("This=%p SrcStartIndex=%u DestIndex=%u VertexCount=%u "
        "pDestBuffer=%p pVertexDecl=%p Flags=%d\n",
        This, SrcStartIndex, DestIndex, VertexCount, pDestBuffer,
        pVertexDecl, Flags);

    if (!screen->get_param(screen, PIPE_CAP_MAX_STREAM_OUTPUT_BUFFERS))
        STUB(D3DERR_INVALIDCALL);

    nine_update_state(This, ~0);

    /* TODO: Create shader with stream output. */
    STUB(D3DERR_INVALIDCALL);
    struct NineVertexBuffer9 *dst = NineVertexBuffer9(pDestBuffer);

    vs = This->state.vs ? This->state.vs : This->ff.vs;

    buffer_size = VertexCount * vs->so->stride[0];
    if (1) {
        struct pipe_resource templ;

        templ.target = PIPE_BUFFER;
        templ.format = PIPE_FORMAT_R8_UNORM;
        templ.width0 = buffer_size;
        templ.flags = 0;
        templ.bind = PIPE_BIND_STREAM_OUTPUT;
        templ.usage = PIPE_USAGE_STREAM;
        templ.height0 = templ.depth0 = templ.array_size = 1;
        templ.last_level = templ.nr_samples = 0;

        resource = This->screen->resource_create(This->screen, &templ);
        if (!resource)
            return E_OUTOFMEMORY;
        buffer_offset = 0;
    } else {
        /* SO matches vertex declaration */
        resource = dst->base.resource;
        buffer_offset = DestIndex * vs->so->stride[0];
    }
    target = This->pipe->create_stream_output_target(This->pipe, resource,
                                                     buffer_offset,
                                                     buffer_size);
    if (!target) {
        pipe_resource_reference(&resource, NULL);
        return D3DERR_DRIVERINTERNALERROR;
    }

    if (!vdecl) {
        hr = NineVertexDeclaration9_new_from_fvf(This, dst->desc.FVF, &vdecl);
        if (FAILED(hr))
            goto out;
    }

    init_draw_info(&draw, This, D3DPT_POINTLIST, VertexCount);
    draw.instance_count = 1;
    draw.indexed = FALSE;
    draw.start = SrcStartIndex;
    draw.index_bias = 0;
    draw.min_index = SrcStartIndex;
    draw.max_index = SrcStartIndex + VertexCount - 1;

    This->pipe->set_stream_output_targets(This->pipe, 1, &target, 0);
    This->pipe->draw_vbo(This->pipe, &draw);
    This->pipe->set_stream_output_targets(This->pipe, 0, NULL, 0);
    This->pipe->stream_output_target_destroy(This->pipe, target);

    hr = NineVertexDeclaration9_ConvertStreamOutput(vdecl,
                                                    dst, DestIndex, VertexCount,
                                                    resource, vs->so);
out:
    pipe_resource_reference(&resource, NULL);
    if (!pVertexDecl)
        NineUnknown_Release(NineUnknown(vdecl));
    return hr;
}

HRESULT WINAPI
NineDevice9_CreateVertexDeclaration( struct NineDevice9 *This,
                                     const D3DVERTEXELEMENT9 *pVertexElements,
                                     IDirect3DVertexDeclaration9 **ppDecl )
{
    struct NineVertexDeclaration9 *vdecl;

    DBG("This=%p pVertexElements=%p ppDecl=%p\n",
        This, pVertexElements, ppDecl);

    HRESULT hr = NineVertexDeclaration9_new(This, pVertexElements, &vdecl);
    if (SUCCEEDED(hr))
        *ppDecl = (IDirect3DVertexDeclaration9 *)vdecl;

    return hr;
}

HRESULT WINAPI
NineDevice9_SetVertexDeclaration( struct NineDevice9 *This,
                                  IDirect3DVertexDeclaration9 *pDecl )
{
    struct nine_state *state = This->update;

    DBG("This=%p pDecl=%p\n", This, pDecl);

    if (likely(!This->is_recording) && state->vdecl == NineVertexDeclaration9(pDecl))
        return D3D_OK;
    nine_bind(&state->vdecl, pDecl);

    state->changed.group |= NINE_STATE_VDECL;

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetVertexDeclaration( struct NineDevice9 *This,
                                  IDirect3DVertexDeclaration9 **ppDecl )
{
    user_assert(ppDecl, D3DERR_INVALIDCALL);

    *ppDecl = (IDirect3DVertexDeclaration9 *)This->state.vdecl;
    if (*ppDecl)
        NineUnknown_AddRef(NineUnknown(*ppDecl));
    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_SetFVF( struct NineDevice9 *This,
                    DWORD FVF )
{
    struct NineVertexDeclaration9 *vdecl;
    HRESULT hr;

    DBG("FVF = %08x\n", FVF);
    if (!FVF)
        return D3D_OK; /* like wine */

    vdecl = util_hash_table_get(This->ff.ht_fvf, &FVF);
    if (!vdecl) {
        hr = NineVertexDeclaration9_new_from_fvf(This, FVF, &vdecl);
        if (FAILED(hr))
            return hr;
        vdecl->fvf = FVF;
        util_hash_table_set(This->ff.ht_fvf, &vdecl->fvf, vdecl);
        NineUnknown_ConvertRefToBind(NineUnknown(vdecl));
    }
    return NineDevice9_SetVertexDeclaration(
        This, (IDirect3DVertexDeclaration9 *)vdecl);
}

HRESULT WINAPI
NineDevice9_GetFVF( struct NineDevice9 *This,
                    DWORD *pFVF )
{
    *pFVF = This->state.vdecl ? This->state.vdecl->fvf : 0;
    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_CreateVertexShader( struct NineDevice9 *This,
                                const DWORD *pFunction,
                                IDirect3DVertexShader9 **ppShader )
{
    struct NineVertexShader9 *vs;
    HRESULT hr;

    DBG("This=%p pFunction=%p ppShader=%p\n", This, pFunction, ppShader);

    hr = NineVertexShader9_new(This, &vs, pFunction, NULL);
    if (FAILED(hr))
        return hr;
    *ppShader = (IDirect3DVertexShader9 *)vs;
    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_SetVertexShader( struct NineDevice9 *This,
                             IDirect3DVertexShader9 *pShader )
{
    struct nine_state *state = This->update;

    DBG("This=%p pShader=%p\n", This, pShader);

    nine_bind(&state->vs, pShader);

    state->changed.group |= NINE_STATE_VS;

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetVertexShader( struct NineDevice9 *This,
                             IDirect3DVertexShader9 **ppShader )
{
    user_assert(ppShader, D3DERR_INVALIDCALL);
    nine_reference_set(ppShader, This->state.vs);
    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_SetVertexShaderConstantF( struct NineDevice9 *This,
                                      UINT StartRegister,
                                      const float *pConstantData,
                                      UINT Vector4fCount )
{
    struct nine_state *state = This->update;

    DBG("This=%p StartRegister=%u pConstantData=%p Vector4fCount=%u\n",
        This, StartRegister, pConstantData, Vector4fCount);

    user_assert(StartRegister                  < This->caps.MaxVertexShaderConst, D3DERR_INVALIDCALL);
    user_assert(StartRegister + Vector4fCount <= This->caps.MaxVertexShaderConst, D3DERR_INVALIDCALL);

    if (!Vector4fCount)
       return D3D_OK;
    user_assert(pConstantData, D3DERR_INVALIDCALL);

    memcpy(&state->vs_const_f[StartRegister * 4],
           pConstantData,
           Vector4fCount * 4 * sizeof(state->vs_const_f[0]));

    nine_ranges_insert(&state->changed.vs_const_f,
                       StartRegister, StartRegister + Vector4fCount,
                       &This->range_pool);

    state->changed.group |= NINE_STATE_VS_CONST;

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetVertexShaderConstantF( struct NineDevice9 *This,
                                      UINT StartRegister,
                                      float *pConstantData,
                                      UINT Vector4fCount )
{
    const struct nine_state *state = &This->state;

    user_assert(StartRegister                  < This->caps.MaxVertexShaderConst, D3DERR_INVALIDCALL);
    user_assert(StartRegister + Vector4fCount <= This->caps.MaxVertexShaderConst, D3DERR_INVALIDCALL);
    user_assert(pConstantData, D3DERR_INVALIDCALL);

    memcpy(pConstantData,
           &state->vs_const_f[StartRegister * 4],
           Vector4fCount * 4 * sizeof(state->vs_const_f[0]));

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_SetVertexShaderConstantI( struct NineDevice9 *This,
                                      UINT StartRegister,
                                      const int *pConstantData,
                                      UINT Vector4iCount )
{
    struct nine_state *state = This->update;
    int i;

    DBG("This=%p StartRegister=%u pConstantData=%p Vector4iCount=%u\n",
        This, StartRegister, pConstantData, Vector4iCount);

    user_assert(StartRegister                  < NINE_MAX_CONST_I, D3DERR_INVALIDCALL);
    user_assert(StartRegister + Vector4iCount <= NINE_MAX_CONST_I, D3DERR_INVALIDCALL);
    user_assert(pConstantData, D3DERR_INVALIDCALL);

    if (This->driver_caps.vs_integer) {
        memcpy(&state->vs_const_i[StartRegister][0],
               pConstantData,
               Vector4iCount * sizeof(state->vs_const_i[0]));
    } else {
        for (i = 0; i < Vector4iCount; i++) {
            state->vs_const_i[StartRegister+i][0] = fui((float)(pConstantData[4*i]));
            state->vs_const_i[StartRegister+i][1] = fui((float)(pConstantData[4*i+1]));
            state->vs_const_i[StartRegister+i][2] = fui((float)(pConstantData[4*i+2]));
            state->vs_const_i[StartRegister+i][3] = fui((float)(pConstantData[4*i+3]));
        }
    }

    state->changed.vs_const_i |= ((1 << Vector4iCount) - 1) << StartRegister;
    state->changed.group |= NINE_STATE_VS_CONST;

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetVertexShaderConstantI( struct NineDevice9 *This,
                                      UINT StartRegister,
                                      int *pConstantData,
                                      UINT Vector4iCount )
{
    const struct nine_state *state = &This->state;
    int i;

    user_assert(StartRegister                  < NINE_MAX_CONST_I, D3DERR_INVALIDCALL);
    user_assert(StartRegister + Vector4iCount <= NINE_MAX_CONST_I, D3DERR_INVALIDCALL);
    user_assert(pConstantData, D3DERR_INVALIDCALL);

    if (This->driver_caps.vs_integer) {
        memcpy(pConstantData,
               &state->vs_const_i[StartRegister][0],
               Vector4iCount * sizeof(state->vs_const_i[0]));
    } else {
        for (i = 0; i < Vector4iCount; i++) {
            pConstantData[4*i] = (int32_t) uif(state->vs_const_i[StartRegister+i][0]);
            pConstantData[4*i+1] = (int32_t) uif(state->vs_const_i[StartRegister+i][1]);
            pConstantData[4*i+2] = (int32_t) uif(state->vs_const_i[StartRegister+i][2]);
            pConstantData[4*i+3] = (int32_t) uif(state->vs_const_i[StartRegister+i][3]);
        }
    }

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_SetVertexShaderConstantB( struct NineDevice9 *This,
                                      UINT StartRegister,
                                      const BOOL *pConstantData,
                                      UINT BoolCount )
{
    struct nine_state *state = This->update;
    int i;
    uint32_t bool_true = This->driver_caps.vs_integer ? 0xFFFFFFFF : fui(1.0f);

    DBG("This=%p StartRegister=%u pConstantData=%p BoolCount=%u\n",
        This, StartRegister, pConstantData, BoolCount);

    user_assert(StartRegister              < NINE_MAX_CONST_B, D3DERR_INVALIDCALL);
    user_assert(StartRegister + BoolCount <= NINE_MAX_CONST_B, D3DERR_INVALIDCALL);
    user_assert(pConstantData, D3DERR_INVALIDCALL);

    for (i = 0; i < BoolCount; i++)
        state->vs_const_b[StartRegister + i] = pConstantData[i] ? bool_true : 0;

    state->changed.vs_const_b |= ((1 << BoolCount) - 1) << StartRegister;
    state->changed.group |= NINE_STATE_VS_CONST;

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetVertexShaderConstantB( struct NineDevice9 *This,
                                      UINT StartRegister,
                                      BOOL *pConstantData,
                                      UINT BoolCount )
{
    const struct nine_state *state = &This->state;
    int i;

    user_assert(StartRegister              < NINE_MAX_CONST_B, D3DERR_INVALIDCALL);
    user_assert(StartRegister + BoolCount <= NINE_MAX_CONST_B, D3DERR_INVALIDCALL);
    user_assert(pConstantData, D3DERR_INVALIDCALL);

    for (i = 0; i < BoolCount; i++)
        pConstantData[i] = state->vs_const_b[StartRegister + i] != 0 ? TRUE : FALSE;

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_SetStreamSource( struct NineDevice9 *This,
                             UINT StreamNumber,
                             IDirect3DVertexBuffer9 *pStreamData,
                             UINT OffsetInBytes,
                             UINT Stride )
{
    struct nine_state *state = This->update;
    struct NineVertexBuffer9 *pVBuf9 = NineVertexBuffer9(pStreamData);
    const unsigned i = StreamNumber;

    DBG("This=%p StreamNumber=%u pStreamData=%p OffsetInBytes=%u Stride=%u\n",
        This, StreamNumber, pStreamData, OffsetInBytes, Stride);

    user_assert(StreamNumber < This->caps.MaxStreams, D3DERR_INVALIDCALL);
    user_assert(Stride <= This->caps.MaxStreamStride, D3DERR_INVALIDCALL);

    if (likely(!This->is_recording)) {
        if (state->stream[i] == NineVertexBuffer9(pStreamData) &&
            state->vtxbuf[i].stride == Stride &&
            state->vtxbuf[i].buffer_offset == OffsetInBytes)
            return D3D_OK;
    }
    nine_bind(&state->stream[i], pStreamData);

    state->changed.vtxbuf |= 1 << StreamNumber;

    if (pStreamData) {
        state->vtxbuf[i].stride = Stride;
        state->vtxbuf[i].buffer_offset = OffsetInBytes;
    }
    state->vtxbuf[i].buffer = pStreamData ? pVBuf9->base.resource : NULL;

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetStreamSource( struct NineDevice9 *This,
                             UINT StreamNumber,
                             IDirect3DVertexBuffer9 **ppStreamData,
                             UINT *pOffsetInBytes,
                             UINT *pStride )
{
    const struct nine_state *state = &This->state;
    const unsigned i = StreamNumber;

    user_assert(StreamNumber < This->caps.MaxStreams, D3DERR_INVALIDCALL);
    user_assert(ppStreamData, D3DERR_INVALIDCALL);

    nine_reference_set(ppStreamData, state->stream[i]);
    *pStride = state->vtxbuf[i].stride;
    *pOffsetInBytes = state->vtxbuf[i].buffer_offset;

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_SetStreamSourceFreq( struct NineDevice9 *This,
                                 UINT StreamNumber,
                                 UINT Setting )
{
    struct nine_state *state = This->update;
    /* const UINT freq = Setting & 0x7FFFFF; */

    DBG("This=%p StreamNumber=%u FrequencyParameter=0x%x\n", This,
        StreamNumber, Setting);

    user_assert(StreamNumber < This->caps.MaxStreams, D3DERR_INVALIDCALL);
    user_assert(StreamNumber != 0 || !(Setting & D3DSTREAMSOURCE_INSTANCEDATA),
                D3DERR_INVALIDCALL);
    user_assert(!((Setting & D3DSTREAMSOURCE_INSTANCEDATA) &&
                  (Setting & D3DSTREAMSOURCE_INDEXEDDATA)), D3DERR_INVALIDCALL);
    user_assert(Setting, D3DERR_INVALIDCALL);

    state->stream_freq[StreamNumber] = Setting;

    if (Setting & D3DSTREAMSOURCE_INSTANCEDATA)
        state->stream_instancedata_mask |= 1 << StreamNumber;
    else
        state->stream_instancedata_mask &= ~(1 << StreamNumber);

    state->changed.stream_freq |= 1 << StreamNumber;
    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetStreamSourceFreq( struct NineDevice9 *This,
                                 UINT StreamNumber,
                                 UINT *pSetting )
{
    user_assert(StreamNumber < This->caps.MaxStreams, D3DERR_INVALIDCALL);
    *pSetting = This->state.stream_freq[StreamNumber];
    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_SetIndices( struct NineDevice9 *This,
                        IDirect3DIndexBuffer9 *pIndexData )
{
    struct nine_state *state = This->update;

    DBG("This=%p pIndexData=%p\n", This, pIndexData);

    if (likely(!This->is_recording))
        if (state->idxbuf == NineIndexBuffer9(pIndexData))
            return D3D_OK;
    nine_bind(&state->idxbuf, pIndexData);

    state->changed.group |= NINE_STATE_IDXBUF;

    return D3D_OK;
}

/* XXX: wine/d3d9 doesn't have pBaseVertexIndex, and it doesn't make sense
 * here because it's an argument passed to the Draw calls.
 */
HRESULT WINAPI
NineDevice9_GetIndices( struct NineDevice9 *This,
                        IDirect3DIndexBuffer9 **ppIndexData /*,
                        UINT *pBaseVertexIndex */ )
{
    user_assert(ppIndexData, D3DERR_INVALIDCALL);
    nine_reference_set(ppIndexData, This->state.idxbuf);
    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_CreatePixelShader( struct NineDevice9 *This,
                               const DWORD *pFunction,
                               IDirect3DPixelShader9 **ppShader )
{
    struct NinePixelShader9 *ps;
    HRESULT hr;

    DBG("This=%p pFunction=%p ppShader=%p\n", This, pFunction, ppShader);

    hr = NinePixelShader9_new(This, &ps, pFunction, NULL);
    if (FAILED(hr))
        return hr;
    *ppShader = (IDirect3DPixelShader9 *)ps;
    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_SetPixelShader( struct NineDevice9 *This,
                            IDirect3DPixelShader9 *pShader )
{
    struct nine_state *state = This->update;
    unsigned old_mask = state->ps ? state->ps->rt_mask : 1;
    unsigned mask;

    DBG("This=%p pShader=%p\n", This, pShader);

    nine_bind(&state->ps, pShader);

    state->changed.group |= NINE_STATE_PS;

    mask = state->ps ? state->ps->rt_mask : 1;
    /* We need to update cbufs if the pixel shader would
     * write to different render targets */
    if (mask != old_mask)
        state->changed.group |= NINE_STATE_FB;

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetPixelShader( struct NineDevice9 *This,
                            IDirect3DPixelShader9 **ppShader )
{
    user_assert(ppShader, D3DERR_INVALIDCALL);
    nine_reference_set(ppShader, This->state.ps);
    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_SetPixelShaderConstantF( struct NineDevice9 *This,
                                     UINT StartRegister,
                                     const float *pConstantData,
                                     UINT Vector4fCount )
{
    struct nine_state *state = This->update;

    DBG("This=%p StartRegister=%u pConstantData=%p Vector4fCount=%u\n",
        This, StartRegister, pConstantData, Vector4fCount);

    user_assert(StartRegister                  < NINE_MAX_CONST_F_PS3, D3DERR_INVALIDCALL);
    user_assert(StartRegister + Vector4fCount <= NINE_MAX_CONST_F_PS3, D3DERR_INVALIDCALL);

    if (!Vector4fCount)
       return D3D_OK;
    user_assert(pConstantData, D3DERR_INVALIDCALL);

    memcpy(&state->ps_const_f[StartRegister * 4],
           pConstantData,
           Vector4fCount * 4 * sizeof(state->ps_const_f[0]));

    nine_ranges_insert(&state->changed.ps_const_f,
                       StartRegister, StartRegister + Vector4fCount,
                       &This->range_pool);

    state->changed.group |= NINE_STATE_PS_CONST;

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetPixelShaderConstantF( struct NineDevice9 *This,
                                     UINT StartRegister,
                                     float *pConstantData,
                                     UINT Vector4fCount )
{
    const struct nine_state *state = &This->state;

    user_assert(StartRegister                  < NINE_MAX_CONST_F_PS3, D3DERR_INVALIDCALL);
    user_assert(StartRegister + Vector4fCount <= NINE_MAX_CONST_F_PS3, D3DERR_INVALIDCALL);
    user_assert(pConstantData, D3DERR_INVALIDCALL);

    memcpy(pConstantData,
           &state->ps_const_f[StartRegister * 4],
           Vector4fCount * 4 * sizeof(state->ps_const_f[0]));

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_SetPixelShaderConstantI( struct NineDevice9 *This,
                                     UINT StartRegister,
                                     const int *pConstantData,
                                     UINT Vector4iCount )
{
    struct nine_state *state = This->update;
    int i;

    DBG("This=%p StartRegister=%u pConstantData=%p Vector4iCount=%u\n",
        This, StartRegister, pConstantData, Vector4iCount);

    user_assert(StartRegister                  < NINE_MAX_CONST_I, D3DERR_INVALIDCALL);
    user_assert(StartRegister + Vector4iCount <= NINE_MAX_CONST_I, D3DERR_INVALIDCALL);
    user_assert(pConstantData, D3DERR_INVALIDCALL);

    if (This->driver_caps.ps_integer) {
        memcpy(&state->ps_const_i[StartRegister][0],
               pConstantData,
               Vector4iCount * sizeof(state->ps_const_i[0]));
    } else {
        for (i = 0; i < Vector4iCount; i++) {
            state->ps_const_i[StartRegister+i][0] = fui((float)(pConstantData[4*i]));
            state->ps_const_i[StartRegister+i][1] = fui((float)(pConstantData[4*i+1]));
            state->ps_const_i[StartRegister+i][2] = fui((float)(pConstantData[4*i+2]));
            state->ps_const_i[StartRegister+i][3] = fui((float)(pConstantData[4*i+3]));
        }
    }
    state->changed.ps_const_i |= ((1 << Vector4iCount) - 1) << StartRegister;
    state->changed.group |= NINE_STATE_PS_CONST;

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetPixelShaderConstantI( struct NineDevice9 *This,
                                     UINT StartRegister,
                                     int *pConstantData,
                                     UINT Vector4iCount )
{
    const struct nine_state *state = &This->state;
    int i;

    user_assert(StartRegister                  < NINE_MAX_CONST_I, D3DERR_INVALIDCALL);
    user_assert(StartRegister + Vector4iCount <= NINE_MAX_CONST_I, D3DERR_INVALIDCALL);
    user_assert(pConstantData, D3DERR_INVALIDCALL);

    if (This->driver_caps.ps_integer) {
        memcpy(pConstantData,
               &state->ps_const_i[StartRegister][0],
               Vector4iCount * sizeof(state->ps_const_i[0]));
    } else {
        for (i = 0; i < Vector4iCount; i++) {
            pConstantData[4*i] = (int32_t) uif(state->ps_const_i[StartRegister+i][0]);
            pConstantData[4*i+1] = (int32_t) uif(state->ps_const_i[StartRegister+i][1]);
            pConstantData[4*i+2] = (int32_t) uif(state->ps_const_i[StartRegister+i][2]);
            pConstantData[4*i+3] = (int32_t) uif(state->ps_const_i[StartRegister+i][3]);
        }
    }

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_SetPixelShaderConstantB( struct NineDevice9 *This,
                                     UINT StartRegister,
                                     const BOOL *pConstantData,
                                     UINT BoolCount )
{
    struct nine_state *state = This->update;
    int i;
    uint32_t bool_true = This->driver_caps.ps_integer ? 0xFFFFFFFF : fui(1.0f);

    DBG("This=%p StartRegister=%u pConstantData=%p BoolCount=%u\n",
        This, StartRegister, pConstantData, BoolCount);

    user_assert(StartRegister              < NINE_MAX_CONST_B, D3DERR_INVALIDCALL);
    user_assert(StartRegister + BoolCount <= NINE_MAX_CONST_B, D3DERR_INVALIDCALL);
    user_assert(pConstantData, D3DERR_INVALIDCALL);

    for (i = 0; i < BoolCount; i++)
        state->ps_const_b[StartRegister + i] = pConstantData[i] ? bool_true : 0;

    state->changed.ps_const_b |= ((1 << BoolCount) - 1) << StartRegister;
    state->changed.group |= NINE_STATE_PS_CONST;

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_GetPixelShaderConstantB( struct NineDevice9 *This,
                                     UINT StartRegister,
                                     BOOL *pConstantData,
                                     UINT BoolCount )
{
    const struct nine_state *state = &This->state;
    int i;

    user_assert(StartRegister              < NINE_MAX_CONST_B, D3DERR_INVALIDCALL);
    user_assert(StartRegister + BoolCount <= NINE_MAX_CONST_B, D3DERR_INVALIDCALL);
    user_assert(pConstantData, D3DERR_INVALIDCALL);

    for (i = 0; i < BoolCount; i++)
        pConstantData[i] = state->ps_const_b[StartRegister + i] ? TRUE : FALSE;

    return D3D_OK;
}

HRESULT WINAPI
NineDevice9_DrawRectPatch( struct NineDevice9 *This,
                           UINT Handle,
                           const float *pNumSegs,
                           const D3DRECTPATCH_INFO *pRectPatchInfo )
{
    STUB(D3DERR_INVALIDCALL);
}

HRESULT WINAPI
NineDevice9_DrawTriPatch( struct NineDevice9 *This,
                          UINT Handle,
                          const float *pNumSegs,
                          const D3DTRIPATCH_INFO *pTriPatchInfo )
{
    STUB(D3DERR_INVALIDCALL);
}

HRESULT WINAPI
NineDevice9_DeletePatch( struct NineDevice9 *This,
                         UINT Handle )
{
    STUB(D3DERR_INVALIDCALL);
}

HRESULT WINAPI
NineDevice9_CreateQuery( struct NineDevice9 *This,
                         D3DQUERYTYPE Type,
                         IDirect3DQuery9 **ppQuery )
{
    struct NineQuery9 *query;
    HRESULT hr;

    DBG("This=%p Type=%d ppQuery=%p\n", This, Type, ppQuery);

    hr = nine_is_query_supported(This->screen, Type);
    if (!ppQuery || hr != D3D_OK)
        return hr;

    hr = NineQuery9_new(This, &query, Type);
    if (FAILED(hr))
        return hr;
    *ppQuery = (IDirect3DQuery9 *)query;
    return D3D_OK;
}

IDirect3DDevice9Vtbl NineDevice9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_Release,
    (void *)NineDevice9_TestCooperativeLevel,
    (void *)NineDevice9_GetAvailableTextureMem,
    (void *)NineDevice9_EvictManagedResources,
    (void *)NineDevice9_GetDirect3D,
    (void *)NineDevice9_GetDeviceCaps,
    (void *)NineDevice9_GetDisplayMode,
    (void *)NineDevice9_GetCreationParameters,
    (void *)NineDevice9_SetCursorProperties,
    (void *)NineDevice9_SetCursorPosition,
    (void *)NineDevice9_ShowCursor,
    (void *)NineDevice9_CreateAdditionalSwapChain,
    (void *)NineDevice9_GetSwapChain,
    (void *)NineDevice9_GetNumberOfSwapChains,
    (void *)NineDevice9_Reset,
    (void *)NineDevice9_Present,
    (void *)NineDevice9_GetBackBuffer,
    (void *)NineDevice9_GetRasterStatus,
    (void *)NineDevice9_SetDialogBoxMode,
    (void *)NineDevice9_SetGammaRamp,
    (void *)NineDevice9_GetGammaRamp,
    (void *)NineDevice9_CreateTexture,
    (void *)NineDevice9_CreateVolumeTexture,
    (void *)NineDevice9_CreateCubeTexture,
    (void *)NineDevice9_CreateVertexBuffer,
    (void *)NineDevice9_CreateIndexBuffer,
    (void *)NineDevice9_CreateRenderTarget,
    (void *)NineDevice9_CreateDepthStencilSurface,
    (void *)NineDevice9_UpdateSurface,
    (void *)NineDevice9_UpdateTexture,
    (void *)NineDevice9_GetRenderTargetData,
    (void *)NineDevice9_GetFrontBufferData,
    (void *)NineDevice9_StretchRect,
    (void *)NineDevice9_ColorFill,
    (void *)NineDevice9_CreateOffscreenPlainSurface,
    (void *)NineDevice9_SetRenderTarget,
    (void *)NineDevice9_GetRenderTarget,
    (void *)NineDevice9_SetDepthStencilSurface,
    (void *)NineDevice9_GetDepthStencilSurface,
    (void *)NineDevice9_BeginScene,
    (void *)NineDevice9_EndScene,
    (void *)NineDevice9_Clear,
    (void *)NineDevice9_SetTransform,
    (void *)NineDevice9_GetTransform,
    (void *)NineDevice9_MultiplyTransform,
    (void *)NineDevice9_SetViewport,
    (void *)NineDevice9_GetViewport,
    (void *)NineDevice9_SetMaterial,
    (void *)NineDevice9_GetMaterial,
    (void *)NineDevice9_SetLight,
    (void *)NineDevice9_GetLight,
    (void *)NineDevice9_LightEnable,
    (void *)NineDevice9_GetLightEnable,
    (void *)NineDevice9_SetClipPlane,
    (void *)NineDevice9_GetClipPlane,
    (void *)NineDevice9_SetRenderState,
    (void *)NineDevice9_GetRenderState,
    (void *)NineDevice9_CreateStateBlock,
    (void *)NineDevice9_BeginStateBlock,
    (void *)NineDevice9_EndStateBlock,
    (void *)NineDevice9_SetClipStatus,
    (void *)NineDevice9_GetClipStatus,
    (void *)NineDevice9_GetTexture,
    (void *)NineDevice9_SetTexture,
    (void *)NineDevice9_GetTextureStageState,
    (void *)NineDevice9_SetTextureStageState,
    (void *)NineDevice9_GetSamplerState,
    (void *)NineDevice9_SetSamplerState,
    (void *)NineDevice9_ValidateDevice,
    (void *)NineDevice9_SetPaletteEntries,
    (void *)NineDevice9_GetPaletteEntries,
    (void *)NineDevice9_SetCurrentTexturePalette,
    (void *)NineDevice9_GetCurrentTexturePalette,
    (void *)NineDevice9_SetScissorRect,
    (void *)NineDevice9_GetScissorRect,
    (void *)NineDevice9_SetSoftwareVertexProcessing,
    (void *)NineDevice9_GetSoftwareVertexProcessing,
    (void *)NineDevice9_SetNPatchMode,
    (void *)NineDevice9_GetNPatchMode,
    (void *)NineDevice9_DrawPrimitive,
    (void *)NineDevice9_DrawIndexedPrimitive,
    (void *)NineDevice9_DrawPrimitiveUP,
    (void *)NineDevice9_DrawIndexedPrimitiveUP,
    (void *)NineDevice9_ProcessVertices,
    (void *)NineDevice9_CreateVertexDeclaration,
    (void *)NineDevice9_SetVertexDeclaration,
    (void *)NineDevice9_GetVertexDeclaration,
    (void *)NineDevice9_SetFVF,
    (void *)NineDevice9_GetFVF,
    (void *)NineDevice9_CreateVertexShader,
    (void *)NineDevice9_SetVertexShader,
    (void *)NineDevice9_GetVertexShader,
    (void *)NineDevice9_SetVertexShaderConstantF,
    (void *)NineDevice9_GetVertexShaderConstantF,
    (void *)NineDevice9_SetVertexShaderConstantI,
    (void *)NineDevice9_GetVertexShaderConstantI,
    (void *)NineDevice9_SetVertexShaderConstantB,
    (void *)NineDevice9_GetVertexShaderConstantB,
    (void *)NineDevice9_SetStreamSource,
    (void *)NineDevice9_GetStreamSource,
    (void *)NineDevice9_SetStreamSourceFreq,
    (void *)NineDevice9_GetStreamSourceFreq,
    (void *)NineDevice9_SetIndices,
    (void *)NineDevice9_GetIndices,
    (void *)NineDevice9_CreatePixelShader,
    (void *)NineDevice9_SetPixelShader,
    (void *)NineDevice9_GetPixelShader,
    (void *)NineDevice9_SetPixelShaderConstantF,
    (void *)NineDevice9_GetPixelShaderConstantF,
    (void *)NineDevice9_SetPixelShaderConstantI,
    (void *)NineDevice9_GetPixelShaderConstantI,
    (void *)NineDevice9_SetPixelShaderConstantB,
    (void *)NineDevice9_GetPixelShaderConstantB,
    (void *)NineDevice9_DrawRectPatch,
    (void *)NineDevice9_DrawTriPatch,
    (void *)NineDevice9_DeletePatch,
    (void *)NineDevice9_CreateQuery
};

static const GUID *NineDevice9_IIDs[] = {
    &IID_IDirect3DDevice9,
    &IID_IUnknown,
    NULL
};

HRESULT
NineDevice9_new( struct pipe_screen *pScreen,
                 D3DDEVICE_CREATION_PARAMETERS *pCreationParameters,
                 D3DCAPS9 *pCaps,
                 D3DPRESENT_PARAMETERS *pPresentationParameters,
                 IDirect3D9 *pD3D9,
                 ID3DPresentGroup *pPresentationGroup,
                 struct d3dadapter9_context *pCTX,
                 boolean ex,
                 D3DDISPLAYMODEEX *pFullscreenDisplayMode,
                 struct NineDevice9 **ppOut )
{
    BOOL lock;
    lock = !!(pCreationParameters->BehaviorFlags & D3DCREATE_MULTITHREADED);

    NINE_NEW(Device9, ppOut, lock, /* args */
             pScreen, pCreationParameters, pCaps,
             pPresentationParameters, pD3D9, pPresentationGroup, pCTX,
             ex, pFullscreenDisplayMode);
}
