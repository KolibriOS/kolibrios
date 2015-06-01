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

#ifndef _NINE_PIXELSHADER9_H_
#define _NINE_PIXELSHADER9_H_

#include "iunknown.h"
#include "nine_shader.h"

struct nine_lconstf;

struct NinePixelShader9
{
    struct NineUnknown base;
    struct nine_shader_variant variant;

    struct {
        const DWORD *tokens;
        DWORD size;
        uint8_t version; /* (major << 4) | minor */
    } byte_code;

    unsigned const_used_size; /* in bytes */

    uint16_t sampler_mask;
    uint16_t sampler_mask_shadow;
    uint8_t rt_mask;

    uint64_t ff_key[6];
};
static INLINE struct NinePixelShader9 *
NinePixelShader9( void *data )
{
    return (struct NinePixelShader9 *)data;
}

void *
NinePixelShader9_GetVariant( struct NinePixelShader9 *vs,
                             uint32_t key );

/*** public ***/

HRESULT
NinePixelShader9_new( struct NineDevice9 *pDevice,
                      struct NinePixelShader9 **ppOut,
                      const DWORD *pFunction, void *cso );

HRESULT
NinePixelShader9_ctor( struct NinePixelShader9 *,
                       struct NineUnknownParams *pParams,
                       const DWORD *pFunction, void *cso );

void
NinePixelShader9_dtor( struct NinePixelShader9 * );

HRESULT WINAPI
NinePixelShader9_GetFunction( struct NinePixelShader9 *This,
                              void *pData,
                              UINT *pSizeOfData );

#endif /* _NINE_PIXELSHADER9_H_ */
