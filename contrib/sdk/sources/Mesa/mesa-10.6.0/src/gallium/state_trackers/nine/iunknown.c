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

#include "iunknown.h"
#include "util/u_atomic.h"
#include "nine_helpers.h"

#define DBG_CHANNEL DBG_UNKNOWN

HRESULT
NineUnknown_ctor( struct NineUnknown *This,
                  struct NineUnknownParams *pParams )
{
    This->refs = pParams->container ? 0 : 1;
    This->bind = 0;
    This->forward = !This->refs;
    This->container = pParams->container;
    This->device = pParams->device;
    if (This->refs && This->device)
        NineUnknown_AddRef(NineUnknown(This->device));

    This->vtable = pParams->vtable;
    This->guids = pParams->guids;
    This->dtor = pParams->dtor;

    return D3D_OK;
}

void
NineUnknown_dtor( struct NineUnknown *This )
{
    FREE(This);
}

HRESULT WINAPI
NineUnknown_QueryInterface( struct NineUnknown *This,
                            REFIID riid,
                            void **ppvObject )
{
    unsigned i = 0;

    DBG("This=%p riid=%p ppvObject=%p\n", This, riid, ppvObject);

    if (!ppvObject) return E_POINTER;

    do {
        if (GUID_equal(This->guids[i], riid)) {
            *ppvObject = This;
            assert(This->refs);
            NineUnknown_AddRef(This);
            return S_OK;
        }
    } while (This->guids[++i]);

    *ppvObject = NULL;
    return E_NOINTERFACE;
}

ULONG WINAPI
NineUnknown_AddRef( struct NineUnknown *This )
{
    ULONG r;
    if (This->forward)
        return NineUnknown_AddRef(This->container);
    else
        r = p_atomic_inc_return(&This->refs);

    if (r == 1) {
        if (This->device)
            NineUnknown_AddRef(NineUnknown(This->device));
        /* This shouldn't be necessary:
        if (This->container)
            NineUnknown_Bind(NineUnknown(This->container)); */
    }
    return r;
}

ULONG WINAPI
NineUnknown_Release( struct NineUnknown *This )
{
    if (This->forward)
        return NineUnknown_Release(This->container);

    ULONG r = p_atomic_dec_return(&This->refs);

    if (r == 0) {
        if (This->device) {
            if (NineUnknown_Release(NineUnknown(This->device)) == 0)
                return r; /* everything's gone */
        }
        if (This->container) {
            /* NineUnknown_Unbind(NineUnknown(This->container)); */
        } else
        if (This->bind == 0) {
            This->dtor(This);
        }
    }
    return r;
}

HRESULT WINAPI
NineUnknown_GetDevice( struct NineUnknown *This,
                       IDirect3DDevice9 **ppDevice )
{
    user_assert(ppDevice, E_POINTER);
    NineUnknown_AddRef(NineUnknown(This->device));
    *ppDevice = (IDirect3DDevice9 *)This->device;
    return D3D_OK;
}
