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

#ifndef _NINE_VERTEXSHADER9_H_
#define _NINE_VERTEXSHADER9_H_

#include "iunknown.h"
#include "nine_shader.h"

struct NineVertexShader9
{
    struct NineUnknown base;
    struct nine_shader_variant variant;

    struct {
        uint16_t ndecl; /* NINE_DECLUSAGE_x */
    } input_map[PIPE_MAX_ATTRIBS];
    unsigned num_inputs;

    struct {
        const DWORD *tokens;
        DWORD size;
        uint8_t version; /* (major << 4) | minor */
    } byte_code;

    uint8_t sampler_mask;
    uint8_t sampler_mask_shadow;

    boolean position_t; /* if true, disable vport transform */
    boolean point_size; /* if true, set rasterizer.point_size_per_vertex to 1 */

    unsigned const_used_size; /* in bytes */

    struct nine_lconstf lconstf;

    const struct pipe_stream_output_info *so;

    uint64_t ff_key[2];
};
static INLINE struct NineVertexShader9 *
NineVertexShader9( void *data )
{
    return (struct NineVertexShader9 *)data;
}

void *
NineVertexShader9_GetVariant( struct NineVertexShader9 *vs,
                              uint32_t key );

/*** public ***/

HRESULT
NineVertexShader9_new( struct NineDevice9 *pDevice,
                       struct NineVertexShader9 **ppOut,
                       const DWORD *pFunction, void *cso );

HRESULT
NineVertexShader9_ctor( struct NineVertexShader9 *,
                        struct NineUnknownParams *pParams,
                        const DWORD *pFunction, void *cso );

void
NineVertexShader9_dtor( struct NineVertexShader9 * );

HRESULT WINAPI
NineVertexShader9_GetFunction( struct NineVertexShader9 *This,
                               void *pData,
                               UINT *pSizeOfData );

#endif /* _NINE_VERTEXSHADER9_H_ */
