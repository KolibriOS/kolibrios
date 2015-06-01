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

#include "stateblock9.h"
#include "device9.h"
#include "basetexture9.h"
#include "nine_helpers.h"

#define DBG_CHANNEL DBG_STATEBLOCK

/* XXX TODO: handling of lights is broken */

HRESULT
NineStateBlock9_ctor( struct NineStateBlock9 *This,
                      struct NineUnknownParams *pParams,
                      enum nine_stateblock_type type )
{
    HRESULT hr = NineUnknown_ctor(&This->base, pParams);

    DBG("This=%p pParams=%p type=%d\n", This, pParams, type);

    if (FAILED(hr))
        return hr;

    This->type = type;

    This->state.vs_const_f = MALLOC(This->base.device->vs_const_size);
    This->state.ps_const_f = MALLOC(This->base.device->ps_const_size);
    if (!This->state.vs_const_f || !This->state.ps_const_f)
        return E_OUTOFMEMORY;

    return D3D_OK;
}

void
NineStateBlock9_dtor( struct NineStateBlock9 *This )
{
    struct nine_state *state = &This->state;
    struct nine_range *r;
    struct nine_range_pool *pool = &This->base.device->range_pool;

    nine_state_clear(state, FALSE);

    FREE(state->vs_const_f);
    FREE(state->ps_const_f);

    FREE(state->ff.light);

    FREE(state->ff.transform);

    if (This->state.changed.ps_const_f) {
        for (r = This->state.changed.ps_const_f; r->next; r = r->next);
        nine_range_pool_put_chain(pool, This->state.changed.ps_const_f, r);
    }
    if (This->state.changed.vs_const_f) {
        for (r = This->state.changed.vs_const_f; r->next; r = r->next);
        nine_range_pool_put_chain(pool, This->state.changed.vs_const_f, r);
    }

    NineUnknown_dtor(&This->base);
}

/* Copy state marked changed in @mask from @src to @dst.
 * If @apply is false, updating dst->changed can be omitted.
 * TODO: compare ?
 */
static void
nine_state_copy_common(struct nine_state *dst,
                       const struct nine_state *src,
                       struct nine_state *mask, /* aliases either src or dst */
                       const boolean apply,
                       struct nine_range_pool *pool)
{
    unsigned i, s;

    if (apply)
       dst->changed.group |= mask->changed.group;

    if (mask->changed.group & NINE_STATE_VIEWPORT)
        dst->viewport = src->viewport;
    if (mask->changed.group & NINE_STATE_SCISSOR)
        dst->scissor = src->scissor;

    if (mask->changed.group & NINE_STATE_VS)
        nine_bind(&dst->vs, src->vs);
    if (mask->changed.group & NINE_STATE_PS)
        nine_bind(&dst->ps, src->ps);

    /* Vertex constants.
     *
     * Various possibilities for optimization here, like creating a per-SB
     * constant buffer, or memcmp'ing for changes.
     * Will do that later depending on what works best for specific apps.
     */
    if (mask->changed.group & NINE_STATE_VS_CONST) {
        struct nine_range *r;
        for (r = mask->changed.vs_const_f; r; r = r->next) {
            memcpy(&dst->vs_const_f[r->bgn * 4],
                   &src->vs_const_f[r->bgn * 4],
                   (r->end - r->bgn) * 4 * sizeof(float));
            if (apply)
                nine_ranges_insert(&dst->changed.vs_const_f, r->bgn, r->end,
                                   pool);
        }
        if (mask->changed.vs_const_i) {
            uint16_t m = mask->changed.vs_const_i;
            for (i = ffs(m) - 1, m >>= i; m; ++i, m >>= 1)
                if (m & 1)
                    memcpy(dst->vs_const_i[i], src->vs_const_i[i], 4 * sizeof(int));
            if (apply)
                dst->changed.vs_const_i |= mask->changed.vs_const_i;
        }
        if (mask->changed.vs_const_b) {
            uint16_t m = mask->changed.vs_const_b;
            for (i = ffs(m) - 1, m >>= i; m; ++i, m >>= 1)
                if (m & 1)
                    dst->vs_const_b[i] = src->vs_const_b[i];
            if (apply)
                dst->changed.vs_const_b |= mask->changed.vs_const_b;
        }
    }

    /* Pixel constants. */
    if (mask->changed.group & NINE_STATE_PS_CONST) {
        struct nine_range *r;
        for (r = mask->changed.ps_const_f; r; r = r->next) {
            memcpy(&dst->ps_const_f[r->bgn * 4],
                   &src->ps_const_f[r->bgn * 4],
                   (r->end - r->bgn) * 4 * sizeof(float));
            if (apply)
                nine_ranges_insert(&dst->changed.ps_const_f, r->bgn, r->end,
                                   pool);
        }
        if (mask->changed.ps_const_i) {
            uint16_t m = mask->changed.ps_const_i;
            for (i = ffs(m) - 1, m >>= i; m; ++i, m >>= 1)
                if (m & 1)
                    memcpy(dst->ps_const_i[i], src->ps_const_i[i], 4 * sizeof(int));
            if (apply)
                dst->changed.ps_const_i |= mask->changed.ps_const_i;
        }
        if (mask->changed.ps_const_b) {
            uint16_t m = mask->changed.ps_const_b;
            for (i = ffs(m) - 1, m >>= i; m; ++i, m >>= 1)
                if (m & 1)
                    dst->ps_const_b[i] = src->ps_const_b[i];
            if (apply)
                dst->changed.ps_const_b |= mask->changed.ps_const_b;
        }
    }

    /* Render states.
     * TODO: Maybe build a list ?
     */
    for (i = 0; i < Elements(dst->changed.rs); ++i) {
        uint32_t m = mask->changed.rs[i];
        if (apply)
            dst->changed.rs[i] |= m;
        while (m) {
            const int r = ffs(m) - 1;
            m &= ~(1 << r);
            dst->rs[i * 32 + r] = src->rs[i * 32 + r];
        }
    }


    /* Clip planes. */
    if (mask->changed.ucp) {
        for (i = 0; i < PIPE_MAX_CLIP_PLANES; ++i)
            if (mask->changed.ucp & (1 << i))
                memcpy(dst->clip.ucp[i],
                       src->clip.ucp[i], sizeof(src->clip.ucp[0]));
        if (apply)
           dst->changed.ucp |= mask->changed.ucp;
    }

    /* Sampler state. */
    if (mask->changed.group & NINE_STATE_SAMPLER) {
        for (s = 0; s < NINE_MAX_SAMPLERS; ++s) {
            if (mask->changed.sampler[s] == 0x3ffe) {
                memcpy(&dst->samp[s], &src->samp[s], sizeof(dst->samp[s]));
            } else {
                uint32_t m = mask->changed.sampler[s];
                while (m) {
                    const int i = ffs(m) - 1;
                    m &= ~(1 << i);
                    dst->samp[s][i] = src->samp[s][i];
                }
            }
            if (apply)
                dst->changed.sampler[s] |= mask->changed.sampler[s];
        }
    }

    /* Index buffer. */
    if (mask->changed.group & NINE_STATE_IDXBUF)
        nine_bind(&dst->idxbuf, src->idxbuf);

    /* Vertex streams. */
    if (mask->changed.vtxbuf | mask->changed.stream_freq) {
        uint32_t m = mask->changed.vtxbuf | mask->changed.stream_freq;
        for (i = 0; m; ++i, m >>= 1) {
            if (mask->changed.vtxbuf & (1 << i)) {
                nine_bind(&dst->stream[i], src->stream[i]);
                if (src->stream[i]) {
                    dst->vtxbuf[i].buffer_offset = src->vtxbuf[i].buffer_offset;
                    dst->vtxbuf[i].buffer = src->vtxbuf[i].buffer;
                    dst->vtxbuf[i].stride = src->vtxbuf[i].stride;
                }
            }
            if (mask->changed.stream_freq & (1 << i))
                dst->stream_freq[i] = src->stream_freq[i];
        }
        dst->stream_instancedata_mask &= ~mask->changed.stream_freq;
        dst->stream_instancedata_mask |=
            src->stream_instancedata_mask & mask->changed.stream_freq;
        if (apply) {
            dst->changed.vtxbuf |= mask->changed.vtxbuf;
            dst->changed.stream_freq |= mask->changed.stream_freq;
        }
    }

    if (!(mask->changed.group & NINE_STATE_FF))
        return;
    WARN_ONCE("Fixed function state not handled properly by StateBlocks.\n");

    /* Fixed function state. */
    if (apply)
        dst->ff.changed.group |= src->ff.changed.group;

    if (mask->changed.group & NINE_STATE_FF_MATERIAL)
        dst->ff.material = src->ff.material;

    if (mask->changed.group & NINE_STATE_FF_PSSTAGES) {
        for (s = 0; s < NINE_MAX_SAMPLERS; ++s) {
            for (i = 0; i < NINED3DTSS_COUNT; ++i)
                if (mask->ff.changed.tex_stage[s][i / 32] & (1 << (i % 32)))
                    dst->ff.tex_stage[s][i] = src->ff.tex_stage[s][i];
            if (apply) {
                /* TODO: it's 32 exactly, just offset by 1 as 0 is unused */
                dst->ff.changed.tex_stage[s][0] |=
                    mask->ff.changed.tex_stage[s][0];
                dst->ff.changed.tex_stage[s][1] |=
                    mask->ff.changed.tex_stage[s][1];
            }
        }
    }
    if (mask->changed.group & NINE_STATE_FF_LIGHTING) {
        if (dst->ff.num_lights < mask->ff.num_lights) {
            dst->ff.light = REALLOC(dst->ff.light,
                                    dst->ff.num_lights * sizeof(D3DLIGHT9),
                                    mask->ff.num_lights * sizeof(D3DLIGHT9));
            dst->ff.num_lights = mask->ff.num_lights;
        }
        for (i = 0; i < mask->ff.num_lights; ++i)
            if (mask->ff.light[i].Type != NINED3DLIGHT_INVALID)
                dst->ff.light[i] = src->ff.light[i];

        memcpy(dst->ff.active_light, src->ff.active_light, sizeof(src->ff.active_light) );
        dst->ff.num_lights_active = src->ff.num_lights_active;
    }
    if (mask->changed.group & NINE_STATE_FF_VSTRANSF) {
        for (i = 0; i < Elements(mask->ff.changed.transform); ++i) {
            if (!mask->ff.changed.transform[i])
                continue;
            for (s = i * 32; s < (i * 32 + 32); ++s) {
                if (!(mask->ff.changed.transform[i] & (1 << (s % 32))))
                    continue;
                *nine_state_access_transform(dst, s, TRUE) =
                    *nine_state_access_transform( /* const because !alloc */
                        (struct nine_state *)src, s, FALSE);
            }
            if (apply)
                dst->ff.changed.transform[i] |= mask->ff.changed.transform[i];
        }
    }
}

static void
nine_state_copy_common_all(struct nine_state *dst,
                           const struct nine_state *src,
                           struct nine_state *help,
                           const boolean apply,
                           struct nine_range_pool *pool,
                           const int MaxStreams)
{
    unsigned i;

    if (apply)
       dst->changed.group |= src->changed.group;

    dst->viewport = src->viewport;
    dst->scissor = src->scissor;

    nine_bind(&dst->vs, src->vs);
    nine_bind(&dst->ps, src->ps);

    /* Vertex constants.
     *
     * Various possibilities for optimization here, like creating a per-SB
     * constant buffer, or memcmp'ing for changes.
     * Will do that later depending on what works best for specific apps.
     */
    if (1) {
        struct nine_range *r = help->changed.vs_const_f;
        memcpy(&dst->vs_const_f[0],
               &src->vs_const_f[0], (r->end - r->bgn) * 4 * sizeof(float));
        if (apply)
            nine_ranges_insert(&dst->changed.vs_const_f, r->bgn, r->end, pool);

        memcpy(dst->vs_const_i, src->vs_const_i, sizeof(dst->vs_const_i));
        memcpy(dst->vs_const_b, src->vs_const_b, sizeof(dst->vs_const_b));
        if (apply) {
            dst->changed.vs_const_i |= src->changed.vs_const_i;
            dst->changed.vs_const_b |= src->changed.vs_const_b;
        }
    }

    /* Pixel constants. */
    if (1) {
        struct nine_range *r = help->changed.ps_const_f;
        memcpy(&dst->ps_const_f[0],
               &src->ps_const_f[0], (r->end - r->bgn) * 4 * sizeof(float));
        if (apply)
            nine_ranges_insert(&dst->changed.ps_const_f, r->bgn, r->end, pool);

        memcpy(dst->ps_const_i, src->ps_const_i, sizeof(dst->ps_const_i));
        memcpy(dst->ps_const_b, src->ps_const_b, sizeof(dst->ps_const_b));
        if (apply) {
            dst->changed.ps_const_i |= src->changed.ps_const_i;
            dst->changed.ps_const_b |= src->changed.ps_const_b;
        }
    }

    /* Render states. */
    memcpy(dst->rs, src->rs, sizeof(dst->rs));
    if (apply)
        memcpy(dst->changed.rs, src->changed.rs, sizeof(dst->changed.rs));


    /* Clip planes. */
    memcpy(&dst->clip, &src->clip, sizeof(dst->clip));
    if (apply)
        dst->changed.ucp = src->changed.ucp;

    /* Sampler state. */
    memcpy(dst->samp, src->samp, sizeof(dst->samp));
    if (apply)
        memcpy(dst->changed.sampler,
               src->changed.sampler, sizeof(dst->changed.sampler));

    /* Index buffer. */
    nine_bind(&dst->idxbuf, src->idxbuf);

    /* Vertex streams. */
    if (1) {
        for (i = 0; i < Elements(dst->stream); ++i) {
            nine_bind(&dst->stream[i], src->stream[i]);
            if (src->stream[i]) {
                dst->vtxbuf[i].buffer_offset = src->vtxbuf[i].buffer_offset;
                dst->vtxbuf[i].buffer = src->vtxbuf[i].buffer;
                dst->vtxbuf[i].stride = src->vtxbuf[i].stride;
            }
            dst->stream_freq[i] = src->stream_freq[i];
        }
        dst->stream_instancedata_mask = src->stream_instancedata_mask;
        if (apply) {
            dst->changed.vtxbuf = (1ULL << MaxStreams) - 1;
            dst->changed.stream_freq = (1ULL << MaxStreams) - 1;
        }
    }

    /* keep this check in case we want to disable FF */
    if (!(help->changed.group & NINE_STATE_FF))
        return;
    WARN_ONCE("Fixed function state not handled properly by StateBlocks.\n");

    /* Fixed function state. */
    if (apply)
        dst->ff.changed.group = src->ff.changed.group;

    dst->ff.material = src->ff.material;

    memcpy(dst->ff.tex_stage, src->ff.tex_stage, sizeof(dst->ff.tex_stage));
    if (apply) /* TODO: memset */
        memcpy(dst->ff.changed.tex_stage,
               src->ff.changed.tex_stage, sizeof(dst->ff.changed.tex_stage));

    /* Lights. */
    if (1) {
        if (dst->ff.num_lights < src->ff.num_lights) {
            dst->ff.light = REALLOC(dst->ff.light,
                                    dst->ff.num_lights * sizeof(D3DLIGHT9),
                                    src->ff.num_lights * sizeof(D3DLIGHT9));
            dst->ff.num_lights = src->ff.num_lights;
        }
        memcpy(dst->ff.light,
               src->ff.light, src->ff.num_lights * sizeof(dst->ff.light[0]));

        memcpy(dst->ff.active_light, src->ff.active_light, sizeof(src->ff.active_light) );
        dst->ff.num_lights_active = src->ff.num_lights_active;
    }

    /* Transforms. */
    if (1) {
        if (dst->ff.num_transforms < src->ff.num_transforms) {
            dst->ff.transform = REALLOC(dst->ff.transform,
                dst->ff.num_transforms * sizeof(dst->ff.transform[0]),
                src->ff.num_transforms * sizeof(src->ff.transform[0]));
            dst->ff.num_transforms = src->ff.num_transforms;
        }
        memcpy(dst->ff.transform,
               src->ff.transform, src->ff.num_transforms * sizeof(D3DMATRIX));
        if (apply) /* TODO: memset */
            memcpy(dst->ff.changed.transform,
                   src->ff.changed.transform, sizeof(dst->ff.changed.transform));
    }
}

/* Capture those bits of current device state that have been changed between
 * BeginStateBlock and EndStateBlock.
 */
HRESULT WINAPI
NineStateBlock9_Capture( struct NineStateBlock9 *This )
{
    struct nine_state *dst = &This->state;
    struct nine_state *src = &This->base.device->state;
    const int MaxStreams = This->base.device->caps.MaxStreams;
    unsigned s;

    DBG("This=%p\n", This);

    if (This->type == NINESBT_ALL)
        nine_state_copy_common_all(dst, src, dst, FALSE, NULL, MaxStreams);
    else
        nine_state_copy_common(dst, src, dst, FALSE, NULL);

    if (dst->changed.group & NINE_STATE_VDECL)
        nine_bind(&dst->vdecl, src->vdecl);

    /* Textures */
    if (dst->changed.texture) {
        uint32_t m = dst->changed.texture;
        for (s = 0; m; ++s, m >>= 1)
            if (m & 1)
                nine_bind(&dst->texture[s], src->texture[s]);
    }

    return D3D_OK;
}

/* Set state managed by this StateBlock as current device state. */
HRESULT WINAPI
NineStateBlock9_Apply( struct NineStateBlock9 *This )
{
    struct nine_state *dst = &This->base.device->state;
    struct nine_state *src = &This->state;
    struct nine_range_pool *pool = &This->base.device->range_pool;
    const int MaxStreams = This->base.device->caps.MaxStreams;
    unsigned s;

    DBG("This=%p\n", This);

    if (This->type == NINESBT_ALL)
        nine_state_copy_common_all(dst, src, src, TRUE, pool, MaxStreams);
    else
        nine_state_copy_common(dst, src, src, TRUE, pool);

    if ((src->changed.group & NINE_STATE_VDECL) && src->vdecl)
        nine_bind(&dst->vdecl, src->vdecl);

    /* Textures */
    if (src->changed.texture) {
        uint32_t m = src->changed.texture;
        dst->changed.texture |= m;

        dst->samplers_shadow &= ~m;

        for (s = 0; m; ++s, m >>= 1) {
            struct NineBaseTexture9 *tex = src->texture[s];
            if (!(m & 1))
                continue;
            if (tex) {
                tex->bind_count++;
                if ((tex->managed.dirty | tex->dirty_mip) && LIST_IS_EMPTY(&tex->list))
                    list_add(&tex->list, &This->base.device->update_textures);
                dst->samplers_shadow |= tex->shadow << s;
            }
            if (src->texture[s])
                src->texture[s]->bind_count--;
            nine_bind(&dst->texture[s], src->texture[s]);
        }
    }

    return D3D_OK;
}

IDirect3DStateBlock9Vtbl NineStateBlock9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_Release,
    (void *)NineUnknown_GetDevice, /* actually part of StateBlock9 iface */
    (void *)NineStateBlock9_Capture,
    (void *)NineStateBlock9_Apply
};

static const GUID *NineStateBlock9_IIDs[] = {
    &IID_IDirect3DStateBlock9,
    &IID_IUnknown,
    NULL
};

HRESULT
NineStateBlock9_new( struct NineDevice9 *pDevice,
                     struct NineStateBlock9 **ppOut,
                     enum nine_stateblock_type type)
{
    NINE_DEVICE_CHILD_NEW(StateBlock9, ppOut, pDevice, type);
}
