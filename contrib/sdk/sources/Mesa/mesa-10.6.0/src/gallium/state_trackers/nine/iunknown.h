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

#ifndef _NINE_IUNKNOWN_H_
#define _NINE_IUNKNOWN_H_

#include "pipe/p_compiler.h"

#include "util/u_memory.h"

#include "guid.h"
#include "nine_debug.h"
#include "nine_quirk.h"

#include "d3d9.h"

struct Nine9;
struct NineDevice9;

struct NineUnknown
{
    /* pointer to vtable  */
    void *vtable;

    int32_t refs; /* external reference count */
    int32_t bind; /* internal bind count */
    boolean forward; /* whether to forward references to the container */

    struct NineUnknown *container; /* referenced if (refs | bind) */
    struct NineDevice9 *device;    /* referenced if (refs) */

    const GUID **guids; /* for QueryInterface */

    void (*dtor)(void *data); /* top-level dtor */
};
static INLINE struct NineUnknown *
NineUnknown( void *data )
{
    return (struct NineUnknown *)data;
}

/* Use this instead of a shitload of arguments: */
struct NineUnknownParams
{
    void *vtable;
    const GUID **guids;
    void (*dtor)(void *data);
    struct NineUnknown *container;
    struct NineDevice9 *device;
};

HRESULT
NineUnknown_ctor( struct NineUnknown *This,
                  struct NineUnknownParams *pParams );

void
NineUnknown_dtor( struct NineUnknown *This );

/*** Direct3D public methods ***/

HRESULT WINAPI
NineUnknown_QueryInterface( struct NineUnknown *This,
                            REFIID riid,
                            void **ppvObject );

ULONG WINAPI
NineUnknown_AddRef( struct NineUnknown *This );

ULONG WINAPI
NineUnknown_Release( struct NineUnknown *This );

HRESULT WINAPI
NineUnknown_GetDevice( struct NineUnknown *This,
                       IDirect3DDevice9 **ppDevice );

/*** Nine private methods ***/

static INLINE void
NineUnknown_Destroy( struct NineUnknown *This )
{
    assert(!(This->refs | This->bind));
    This->dtor(This);
}

static INLINE UINT
NineUnknown_Bind( struct NineUnknown *This )
{
    UINT b = ++This->bind;
    assert(b);
    if (b == 1 && This->container) {
        if (This->container != NineUnknown(This->device))
            NineUnknown_Bind(This->container);
    }
    return b;
}

static INLINE UINT
NineUnknown_Unbind( struct NineUnknown *This )
{
    UINT b = --This->bind;
    if (!b) {
        if (This->container) {
            if (This->container != NineUnknown(This->device))
                NineUnknown_Unbind(This->container);
        } else
        if (This->refs == 0) {
            This->dtor(This);
        }
    }
    return b;
}

static INLINE void
NineUnknown_ConvertRefToBind( struct NineUnknown *This )
{
    NineUnknown_Bind(This);
    NineUnknown_Release(This);
}

/* Detach from container. */
static INLINE void
NineUnknown_Detach( struct NineUnknown *This )
{
    assert(This->container && !This->forward);
    if (This->refs)
        NineUnknown_Unbind(This->container);
    if (This->bind)
        NineUnknown_Unbind(This->container);
    This->container = NULL;
    if (!(This->refs | This->bind))
        This->dtor(This);
}

#endif /* _NINE_IUNKNOWN_H_ */
