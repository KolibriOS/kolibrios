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
#include "volume9.h"
#include "basetexture9.h" /* for marking dirty */
#include "nine_helpers.h"
#include "nine_pipe.h"
#include "nine_dump.h"

#include "util/u_hash_table.h"
#include "util/u_format.h"
#include "util/u_surface.h"
#include "nine_pdata.h"

#define DBG_CHANNEL DBG_VOLUME


static HRESULT
NineVolume9_AllocateData( struct NineVolume9 *This )
{
    unsigned size = This->layer_stride * This->desc.Depth;

    DBG("(%p(This=%p),level=%u) Allocating 0x%x bytes of system memory.\n",
        This->base.container, This, This->level, size);

    This->data = (uint8_t *)MALLOC(size);
    if (!This->data)
        return E_OUTOFMEMORY;
    return D3D_OK;
}

static HRESULT
NineVolume9_ctor( struct NineVolume9 *This,
                  struct NineUnknownParams *pParams,
                  struct NineUnknown *pContainer,
                  struct pipe_resource *pResource,
                  unsigned Level,
                  D3DVOLUME_DESC *pDesc )
{
    HRESULT hr;

    assert(pContainer); /* stand-alone volumes can't be created */

    DBG("This=%p pContainer=%p pDevice=%p pResource=%p Level=%u pDesc=%p\n",
        This, pContainer, pParams->device, pResource, Level, pDesc);

    /* Mark this as a special surface held by another internal resource. */
    pParams->container = pContainer;

    user_assert(!(pDesc->Usage & D3DUSAGE_DYNAMIC) ||
                (pDesc->Pool != D3DPOOL_MANAGED), D3DERR_INVALIDCALL);

    assert(pResource || pDesc->Pool != D3DPOOL_DEFAULT);

    hr = NineUnknown_ctor(&This->base, pParams);
    if (FAILED(hr))
        return hr;

    This->pdata = util_hash_table_create(ht_guid_hash, ht_guid_compare);
    if (!This->pdata)
        return E_OUTOFMEMORY;

    pipe_resource_reference(&This->resource, pResource);

    This->pipe = pParams->device->pipe;
    This->transfer = NULL;
    This->lock_count = 0;

    This->level = Level;
    This->level_actual = Level;
    This->desc = *pDesc;

    This->info.screen = pParams->device->screen;
    This->info.target = PIPE_TEXTURE_3D;
    This->info.width0 = pDesc->Width;
    This->info.height0 = pDesc->Height;
    This->info.depth0 = pDesc->Depth;
    This->info.last_level = 0;
    This->info.array_size = 1;
    This->info.nr_samples = 0;
    This->info.usage = PIPE_USAGE_DEFAULT;
    This->info.bind = PIPE_BIND_SAMPLER_VIEW;
    This->info.flags = 0;
    This->info.format = d3d9_to_pipe_format_checked(This->info.screen,
                                                    pDesc->Format,
                                                    This->info.target,
                                                    This->info.nr_samples,
                                                    This->info.bind, FALSE);

    if (This->info.format == PIPE_FORMAT_NONE)
        return D3DERR_DRIVERINTERNALERROR;

    This->stride = util_format_get_stride(This->info.format, pDesc->Width);
    This->stride = align(This->stride, 4);
    This->layer_stride = util_format_get_2d_size(This->info.format,
                                                 This->stride, pDesc->Height);

    if (pDesc->Pool == D3DPOOL_SYSTEMMEM)
        This->info.usage = PIPE_USAGE_STAGING;

    if (!This->resource) {
        hr = NineVolume9_AllocateData(This);
        if (FAILED(hr))
            return hr;
    }
    return D3D_OK;
}

static void
NineVolume9_dtor( struct NineVolume9 *This )
{
    DBG("This=%p\n", This);

    if (This->transfer)
        NineVolume9_UnlockBox(This);

    if (This->data)
           FREE(This->data);

    pipe_resource_reference(&This->resource, NULL);

    NineUnknown_dtor(&This->base);
}

HRESULT WINAPI
NineVolume9_GetContainer( struct NineVolume9 *This,
                          REFIID riid,
                          void **ppContainer )
{
    if (!NineUnknown(This)->container)
        return E_NOINTERFACE;
    return NineUnknown_QueryInterface(NineUnknown(This)->container, riid, ppContainer);
}

static INLINE void
NineVolume9_MarkContainerDirty( struct NineVolume9 *This )
{
    struct NineBaseTexture9 *tex;
#ifdef DEBUG
    /* This is always contained by a NineVolumeTexture9. */
    GUID id = IID_IDirect3DVolumeTexture9;
    REFIID ref = &id;
    assert(NineUnknown_QueryInterface(This->base.container, ref, (void **)&tex)
           == S_OK);
    assert(NineUnknown_Release(NineUnknown(tex)) != 0);
#endif

    tex = NineBaseTexture9(This->base.container);
    assert(tex);
    if (This->desc.Pool == D3DPOOL_MANAGED)
        tex->managed.dirty = TRUE;

    BASETEX_REGISTER_UPDATE(tex);
}

HRESULT WINAPI
NineVolume9_GetDesc( struct NineVolume9 *This,
                     D3DVOLUME_DESC *pDesc )
{
    user_assert(pDesc != NULL, E_POINTER);
    *pDesc = This->desc;
    return D3D_OK;
}

static INLINE boolean
NineVolume9_IsDirty(struct NineVolume9 *This)
{
    return This->dirty_box[0].width != 0;
}

INLINE void
NineVolume9_AddDirtyRegion( struct NineVolume9 *This,
                            const struct pipe_box *box )
{
    struct pipe_box cover_a, cover_b;
    float vol[2];

    if (!box) {
        u_box_3d(0, 0, 0, This->desc.Width, This->desc.Height,
                 This->desc.Depth, &This->dirty_box[0]);
        memset(&This->dirty_box[1], 0, sizeof(This->dirty_box[1]));
        return;
    }
    if (!This->dirty_box[0].width) {
        This->dirty_box[0] = *box;
        return;
    }

    u_box_union_3d(&cover_a, &This->dirty_box[0], box);
    vol[0] = u_box_volume_3d(&cover_a);

    if (This->dirty_box[1].width == 0) {
        vol[1] = u_box_volume_3d(&This->dirty_box[0]);
        if (vol[0] > (vol[1] * 1.5f))
            This->dirty_box[1] = *box;
        else
            This->dirty_box[0] = cover_a;
    } else {
        u_box_union_3d(&cover_b, &This->dirty_box[1], box);
        vol[1] = u_box_volume_3d(&cover_b);

        if (vol[0] > vol[1])
            This->dirty_box[1] = cover_b;
        else
            This->dirty_box[0] = cover_a;
    }
}

static INLINE uint8_t *
NineVolume9_GetSystemMemPointer(struct NineVolume9 *This, int x, int y, int z)
{
    unsigned x_offset = util_format_get_stride(This->info.format, x);

    y = util_format_get_nblocksy(This->info.format, y);

    assert(This->data);
    return This->data + (z * This->layer_stride + y * This->stride + x_offset);
}

HRESULT WINAPI
NineVolume9_LockBox( struct NineVolume9 *This,
                     D3DLOCKED_BOX *pLockedVolume,
                     const D3DBOX *pBox,
                     DWORD Flags )
{
    struct pipe_resource *resource = This->resource;
    struct pipe_box box;
    unsigned usage;

    DBG("This=%p(%p) pLockedVolume=%p pBox=%p[%u..%u,%u..%u,%u..%u] Flags=%s\n",
        This, This->base.container, pLockedVolume, pBox,
        pBox ? pBox->Left : 0, pBox ? pBox->Right : 0,
        pBox ? pBox->Top : 0, pBox ? pBox->Bottom : 0,
        pBox ? pBox->Front : 0, pBox ? pBox->Back : 0,
        nine_D3DLOCK_to_str(Flags));

    user_assert(This->desc.Pool != D3DPOOL_DEFAULT ||
                (This->desc.Usage & D3DUSAGE_DYNAMIC), D3DERR_INVALIDCALL);

    user_assert(!((Flags & D3DLOCK_DISCARD) && (Flags & D3DLOCK_READONLY)),
                D3DERR_INVALIDCALL);

    user_assert(This->lock_count == 0, D3DERR_INVALIDCALL);
    user_assert(pLockedVolume, E_POINTER);

    if (pBox && This->desc.Pool == D3DPOOL_DEFAULT &&
        util_format_is_compressed(This->info.format)) {
        const unsigned w = util_format_get_blockwidth(This->info.format);
        const unsigned h = util_format_get_blockheight(This->info.format);
        user_assert(!(pBox->Left % w) && !(pBox->Right % w) &&
                    !(pBox->Top % h) && !(pBox->Bottom % h),
                    D3DERR_INVALIDCALL);
    }

    if (Flags & D3DLOCK_DISCARD) {
        usage = PIPE_TRANSFER_WRITE | PIPE_TRANSFER_DISCARD_RANGE;
    } else {
        usage = (Flags & D3DLOCK_READONLY) ?
            PIPE_TRANSFER_READ : PIPE_TRANSFER_READ_WRITE;
    }
    if (Flags & D3DLOCK_DONOTWAIT)
        usage |= PIPE_TRANSFER_DONTBLOCK;

    if (pBox) {
        d3dbox_to_pipe_box(&box, pBox);
        if (u_box_clip_2d(&box, &box, This->desc.Width, This->desc.Height) < 0) {
            DBG("Locked volume intersection empty.\n");
            return D3DERR_INVALIDCALL;
        }
    } else {
        u_box_3d(0, 0, 0, This->desc.Width, This->desc.Height, This->desc.Depth,
                 &box);
    }

    if (This->data) {
        pLockedVolume->RowPitch = This->stride;
        pLockedVolume->SlicePitch = This->layer_stride;
        pLockedVolume->pBits =
            NineVolume9_GetSystemMemPointer(This, box.x, box.y, box.z);
    } else {
        pLockedVolume->pBits =
            This->pipe->transfer_map(This->pipe, resource, This->level, usage,
                                     &box, &This->transfer);
        if (!This->transfer) {
            if (Flags & D3DLOCK_DONOTWAIT)
                return D3DERR_WASSTILLDRAWING;
            return D3DERR_DRIVERINTERNALERROR;
        }
        pLockedVolume->RowPitch = This->transfer->stride;
        pLockedVolume->SlicePitch = This->transfer->layer_stride;
    }

    if (!(Flags & (D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_READONLY))) {
        NineVolume9_MarkContainerDirty(This);
        if (This->desc.Pool == D3DPOOL_MANAGED)
            NineVolume9_AddDirtyRegion(This, &box);
    }

    ++This->lock_count;
    return D3D_OK;
}

HRESULT WINAPI
NineVolume9_UnlockBox( struct NineVolume9 *This )
{
    DBG("This=%p lock_count=%u\n", This, This->lock_count);
    user_assert(This->lock_count, D3DERR_INVALIDCALL);
    if (This->transfer) {
        This->pipe->transfer_unmap(This->pipe, This->transfer);
        This->transfer = NULL;
    }
    --This->lock_count;
    return D3D_OK;
}


HRESULT
NineVolume9_CopyVolume( struct NineVolume9 *This,
                        struct NineVolume9 *From,
                        unsigned dstx, unsigned dsty, unsigned dstz,
                        struct pipe_box *pSrcBox )
{
    struct pipe_context *pipe = This->pipe;
    struct pipe_resource *r_dst = This->resource;
    struct pipe_resource *r_src = From->resource;
    struct pipe_transfer *transfer;
    struct pipe_box src_box;
    struct pipe_box dst_box;
    uint8_t *p_dst;
    const uint8_t *p_src;

    DBG("This=%p From=%p dstx=%u dsty=%u dstz=%u pSrcBox=%p\n",
        This, From, dstx, dsty, dstz, pSrcBox);

    assert(This->desc.Pool != D3DPOOL_MANAGED &&
           From->desc.Pool != D3DPOOL_MANAGED);
    user_assert(This->desc.Format == From->desc.Format, D3DERR_INVALIDCALL);

    dst_box.x = dstx;
    dst_box.y = dsty;
    dst_box.z = dstz;

    if (pSrcBox) {
        /* make sure it doesn't range outside the source volume */
        user_assert(pSrcBox->x >= 0 &&
                    (pSrcBox->width - pSrcBox->x) <= From->desc.Width &&
                    pSrcBox->y >= 0 &&
                    (pSrcBox->height - pSrcBox->y) <= From->desc.Height &&
                    pSrcBox->z >= 0 &&
                    (pSrcBox->depth - pSrcBox->z) <= From->desc.Depth,
                    D3DERR_INVALIDCALL);
        src_box = *pSrcBox;
    } else {
        src_box.x = 0;
        src_box.y = 0;
        src_box.z = 0;
        src_box.width = From->desc.Width;
        src_box.height = From->desc.Height;
        src_box.depth = From->desc.Depth;
    }
    /* limits */
    dst_box.width = This->desc.Width - dst_box.x;
    dst_box.height = This->desc.Height - dst_box.y;
    dst_box.depth = This->desc.Depth - dst_box.z;

    user_assert(src_box.width <= dst_box.width &&
                src_box.height <= dst_box.height &&
                src_box.depth <= dst_box.depth, D3DERR_INVALIDCALL);

    dst_box.width = src_box.width;
    dst_box.height = src_box.height;
    dst_box.depth = src_box.depth;

    if (r_dst && r_src) {
        pipe->resource_copy_region(pipe,
                                   r_dst, This->level,
                                   dst_box.x, dst_box.y, dst_box.z,
                                   r_src, From->level,
                                   &src_box);
    } else
    if (r_dst) {
        p_src = NineVolume9_GetSystemMemPointer(From,
            src_box.x, src_box.y, src_box.z);

        pipe->transfer_inline_write(pipe, r_dst, This->level,
                                    0, /* WRITE|DISCARD are implicit */
                                    &dst_box, p_src,
                                    From->stride, From->layer_stride);
    } else
    if (r_src) {
        p_dst = NineVolume9_GetSystemMemPointer(This, 0, 0, 0);
        p_src = pipe->transfer_map(pipe, r_src, From->level,
                                   PIPE_TRANSFER_READ,
                                   &src_box, &transfer);
        if (!p_src)
            return D3DERR_DRIVERINTERNALERROR;

        util_copy_box(p_dst, This->info.format,
                      This->stride, This->layer_stride,
                      dst_box.x, dst_box.y, dst_box.z,
                      dst_box.width, dst_box.height, dst_box.depth,
                      p_src,
                      transfer->stride, transfer->layer_stride,
                      src_box.x, src_box.y, src_box.z);

        pipe->transfer_unmap(pipe, transfer);
    } else {
        p_dst = NineVolume9_GetSystemMemPointer(This, 0, 0, 0);
        p_src = NineVolume9_GetSystemMemPointer(From, 0, 0, 0);

        util_copy_box(p_dst, This->info.format,
                      This->stride, This->layer_stride,
                      dst_box.x, dst_box.y, dst_box.z,
                      dst_box.width, dst_box.height, dst_box.depth,
                      p_src,
                      From->stride, From->layer_stride,
                      src_box.x, src_box.y, src_box.z);
    }

    if (This->desc.Pool == D3DPOOL_DEFAULT)
        NineVolume9_MarkContainerDirty(This);
    if (!r_dst && This->resource)
        NineVolume9_AddDirtyRegion(This, &dst_box);

    return D3D_OK;
}

HRESULT
NineVolume9_UploadSelf( struct NineVolume9 *This )
{
    struct pipe_context *pipe = This->pipe;
    struct pipe_resource *res = This->resource;
    uint8_t *ptr;
    unsigned i;

    DBG("This=%p dirty=%i data=%p res=%p\n", This, NineVolume9_IsDirty(This),
        This->data, res);

    assert(This->desc.Pool == D3DPOOL_MANAGED);

    if (!NineVolume9_IsDirty(This))
        return D3D_OK;
    assert(res);

    for (i = 0; i < Elements(This->dirty_box); ++i) {
        const struct pipe_box *box = &This->dirty_box[i];
        if (box->width == 0)
            break;
        ptr = NineVolume9_GetSystemMemPointer(This, box->x, box->y, box->z);

        pipe->transfer_inline_write(pipe, res, This->level,
                                    0,
                                    box, ptr, This->stride, This->layer_stride);
    }
    NineVolume9_ClearDirtyRegion(This);

    return D3D_OK;
}


IDirect3DVolume9Vtbl NineVolume9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_Release,
    (void *)NineUnknown_GetDevice, /* actually part of Volume9 iface */
    (void *)NineVolume9_SetPrivateData,
    (void *)NineVolume9_GetPrivateData,
    (void *)NineVolume9_FreePrivateData,
    (void *)NineVolume9_GetContainer,
    (void *)NineVolume9_GetDesc,
    (void *)NineVolume9_LockBox,
    (void *)NineVolume9_UnlockBox
};

static const GUID *NineVolume9_IIDs[] = {
    &IID_IDirect3DVolume9,
    &IID_IUnknown,
    NULL
};

HRESULT
NineVolume9_new( struct NineDevice9 *pDevice,
                 struct NineUnknown *pContainer,
                 struct pipe_resource *pResource,
                 unsigned Level,
                 D3DVOLUME_DESC *pDesc,
                 struct NineVolume9 **ppOut )
{
    NINE_DEVICE_CHILD_NEW(Volume9, ppOut, pDevice, /* args */
                          pContainer, pResource, Level, pDesc);
}


/*** The boring stuff. TODO: Unify with Resource. ***/

HRESULT WINAPI
NineVolume9_SetPrivateData( struct NineVolume9 *This,
                            REFGUID refguid,
                            const void *pData,
                            DWORD SizeOfData,
                            DWORD Flags )
{
    enum pipe_error err;
    struct pheader *header;
    const void *user_data = pData;

    DBG("This=%p refguid=%p pData=%p SizeOfData=%d Flags=%d\n",
        This, refguid, pData, SizeOfData, Flags);

    if (Flags & D3DSPD_IUNKNOWN)
        user_assert(SizeOfData == sizeof(IUnknown *), D3DERR_INVALIDCALL);

    /* data consists of a header and the actual data. avoiding 2 mallocs */
    header = CALLOC_VARIANT_LENGTH_STRUCT(pheader, SizeOfData-1);
    if (!header) { return E_OUTOFMEMORY; }
    header->unknown = (Flags & D3DSPD_IUNKNOWN) ? TRUE : FALSE;

    /* if the refguid already exists, delete it */
    NineVolume9_FreePrivateData(This, refguid);

    /* IUnknown special case */
    if (header->unknown) {
        /* here the pointer doesn't point to the data we want, so point at the
         * pointer making what we eventually copy is the pointer itself */
        user_data = &pData;
    }

    header->size = SizeOfData;
    memcpy(header->data, user_data, header->size);

    err = util_hash_table_set(This->pdata, refguid, header);
    if (err == PIPE_OK) {
        if (header->unknown) { IUnknown_AddRef(*(IUnknown **)header->data); }
        return D3D_OK;
    }

    FREE(header);
    if (err == PIPE_ERROR_OUT_OF_MEMORY) { return E_OUTOFMEMORY; }

    return D3DERR_DRIVERINTERNALERROR;
}

HRESULT WINAPI
NineVolume9_GetPrivateData( struct NineVolume9 *This,
                            REFGUID refguid,
                            void *pData,
                            DWORD *pSizeOfData )
{
    struct pheader *header;

    user_assert(pSizeOfData, E_POINTER);

    header = util_hash_table_get(This->pdata, refguid);
    if (!header) { return D3DERR_NOTFOUND; }

    if (!pData) {
        *pSizeOfData = header->size;
        return D3D_OK;
    }
    if (*pSizeOfData < header->size) {
        return D3DERR_MOREDATA;
    }

    if (header->unknown) { IUnknown_AddRef(*(IUnknown **)header->data); }
    memcpy(pData, header->data, header->size);

    return D3D_OK;
}

HRESULT WINAPI
NineVolume9_FreePrivateData( struct NineVolume9 *This,
                             REFGUID refguid )
{
    struct pheader *header;

    DBG("This=%p refguid=%p\n", This, refguid);

    header = util_hash_table_get(This->pdata, refguid);
    if (!header) { return D3DERR_NOTFOUND; }

    ht_guid_delete(NULL, header, NULL);
    util_hash_table_remove(This->pdata, refguid);

    return D3D_OK;
}

