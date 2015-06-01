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

#ifndef _NINE_INDEXBUFFER9_H_
#define _NINE_INDEXBUFFER9_H_

#include "resource9.h"

#include "pipe/p_state.h"

struct pipe_screen;
struct pipe_context;
struct pipe_index_buffer;
struct pipe_transfer;
struct NineDevice9;

struct NineIndexBuffer9
{
    struct NineResource9 base;

    /* g3d stuff */
    struct pipe_context *pipe;
    struct pipe_index_buffer buffer;
    struct pipe_transfer *transfer;
    UINT map_count;

    D3DINDEXBUFFER_DESC desc;
};
static INLINE struct NineIndexBuffer9 *
NineIndexBuffer9( void *data )
{
    return (struct NineIndexBuffer9 *)data;
}

HRESULT
NineIndexBuffer9_new( struct NineDevice9 *pDevice,
                      D3DINDEXBUFFER_DESC *pDesc,
                      struct NineIndexBuffer9 **ppOut );

HRESULT
NineIndexBuffer9_ctor( struct NineIndexBuffer9 *This,
                       struct NineUnknownParams *pParams,
                       D3DINDEXBUFFER_DESC *pDesc );

void
NineIndexBuffer9_dtor( struct NineIndexBuffer9 *This );

/*** Nine private ***/

const struct pipe_index_buffer *
NineIndexBuffer9_GetBuffer( struct NineIndexBuffer9 *This );

/*** Direct3D public ***/

HRESULT WINAPI
NineIndexBuffer9_Lock( struct NineIndexBuffer9 *This,
                       UINT OffsetToLock,
                       UINT SizeToLock,
                       void **ppbData,
                       DWORD Flags );

HRESULT WINAPI
NineIndexBuffer9_Unlock( struct NineIndexBuffer9 *This );

HRESULT WINAPI
NineIndexBuffer9_GetDesc( struct NineIndexBuffer9 *This,
                          D3DINDEXBUFFER_DESC *pDesc );

#endif /* _NINE_INDEXBUFFER9_H_ */
