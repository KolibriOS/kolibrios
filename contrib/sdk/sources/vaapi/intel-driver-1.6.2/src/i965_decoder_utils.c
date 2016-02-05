/*
 * Copyright (C) 2006-2012 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "sysdeps.h"
#include <limits.h>
#include <alloca.h>

#include "intel_batchbuffer.h"
#include "intel_media.h"
#include "i965_drv_video.h"
#include "i965_decoder_utils.h"
#include "i965_defines.h"

/* Set reference surface if backing store exists */
static inline int
set_ref_frame(
    struct i965_driver_data *i965,
    GenFrameStore           *ref_frame,
    VASurfaceID              va_surface,
    struct object_surface   *obj_surface
)
{
    if (va_surface == VA_INVALID_ID)
        return 0;

    if (!obj_surface || !obj_surface->bo)
        return 0;

    ref_frame->surface_id = va_surface;
    ref_frame->obj_surface = obj_surface;
    return 1;
}

/* Check wether codec layer incorrectly fills in slice_vertical_position */
int
mpeg2_wa_slice_vertical_position(
    struct decode_state           *decode_state,
    VAPictureParameterBufferMPEG2 *pic_param
)
{
    unsigned int i, j, mb_height, vpos, last_vpos = 0;

    /* Assume progressive sequence if we got a progressive frame */
    if (pic_param->picture_coding_extension.bits.progressive_frame)
        return 0;

    /* Wait for a field coded picture */
    if (pic_param->picture_coding_extension.bits.picture_structure == MPEG_FRAME)
        return -1;

    assert(decode_state && decode_state->slice_params);

    mb_height = (pic_param->vertical_size + 31) / 32;

    for (j = 0; j < decode_state->num_slice_params; j++) {
        struct buffer_store * const buffer_store =
            decode_state->slice_params[j];

        for (i = 0; i < buffer_store->num_elements; i++) {
            VASliceParameterBufferMPEG2 * const slice_param =
                ((VASliceParameterBufferMPEG2 *)buffer_store->buffer) + i;

            vpos = slice_param->slice_vertical_position;
            if (vpos >= mb_height || vpos == last_vpos + 2) {
                WARN_ONCE("codec layer incorrectly fills in MPEG-2 slice_vertical_position. Workaround applied\n");
                return 1;
            }
            last_vpos = vpos;
        }
    }
    return 0;
}

/* Build MPEG-2 reference frames array */
void
mpeg2_set_reference_surfaces(
    VADriverContextP               ctx,
    GenFrameStore                  ref_frames[MAX_GEN_REFERENCE_FRAMES],
    struct decode_state           *decode_state,
    VAPictureParameterBufferMPEG2 *pic_param
)
{
    struct i965_driver_data * const i965 = i965_driver_data(ctx);
    VASurfaceID va_surface;
    unsigned pic_structure, is_second_field, n = 0;
    struct object_surface *obj_surface;

    pic_structure = pic_param->picture_coding_extension.bits.picture_structure;
    is_second_field = pic_structure != MPEG_FRAME &&
        !pic_param->picture_coding_extension.bits.is_first_field;

    ref_frames[0].surface_id = VA_INVALID_ID;
    ref_frames[0].obj_surface = NULL;

    /* Reference frames are indexed by frame store ID  (0:top, 1:bottom) */
    switch (pic_param->picture_coding_type) {
    case MPEG_P_PICTURE:
        if (is_second_field && pic_structure == MPEG_BOTTOM_FIELD) {
            va_surface = decode_state->current_render_target;
            obj_surface = decode_state->render_object;
            n += set_ref_frame(i965, &ref_frames[n], va_surface, obj_surface);
        }
        va_surface = pic_param->forward_reference_picture;
        obj_surface = decode_state->reference_objects[0];
        n += set_ref_frame(i965, &ref_frames[n], va_surface, obj_surface);
        break;

    case MPEG_B_PICTURE:
        va_surface = pic_param->forward_reference_picture;
        obj_surface = decode_state->reference_objects[0];
        n += set_ref_frame(i965, &ref_frames[n], va_surface, obj_surface);
        va_surface = pic_param->backward_reference_picture;
        obj_surface = decode_state->reference_objects[1];
        n += set_ref_frame(i965, &ref_frames[n], va_surface, obj_surface);
        break;
    }

    while (n != 2) {
        ref_frames[n].obj_surface = ref_frames[0].obj_surface;
        ref_frames[n++].surface_id = ref_frames[0].surface_id;
    }

    if (pic_param->picture_coding_extension.bits.frame_pred_frame_dct)
        return;

    ref_frames[2].surface_id = VA_INVALID_ID;
    ref_frames[2].obj_surface = NULL;

    /* Bottom field pictures used as reference */
    switch (pic_param->picture_coding_type) {
    case MPEG_P_PICTURE:
        if (is_second_field && pic_structure == MPEG_TOP_FIELD) {
            va_surface = decode_state->current_render_target;
            obj_surface = decode_state->render_object;
            n += set_ref_frame(i965, &ref_frames[n], va_surface, obj_surface);
        }
        va_surface = pic_param->forward_reference_picture;
        obj_surface = decode_state->reference_objects[0];
        n += set_ref_frame(i965, &ref_frames[n], va_surface, obj_surface);
        break;

    case MPEG_B_PICTURE:
        va_surface = pic_param->forward_reference_picture;
        obj_surface = decode_state->reference_objects[0];
        n += set_ref_frame(i965, &ref_frames[n], va_surface, obj_surface);
        va_surface = pic_param->backward_reference_picture;
        obj_surface = decode_state->reference_objects[1];
        n += set_ref_frame(i965, &ref_frames[n], va_surface, obj_surface);
        break;
    }

    while (n != 4) {
        ref_frames[n].obj_surface = ref_frames[2].obj_surface;
        ref_frames[n++].surface_id = ref_frames[2].surface_id;
    }
}

/* Ensure the supplied VA surface has valid storage for decoding the
   current picture */
VAStatus
avc_ensure_surface_bo(
    VADriverContextP                    ctx,
    struct decode_state                *decode_state,
    struct object_surface              *obj_surface,
    const VAPictureParameterBufferH264 *pic_param
)
{
    VAStatus va_status;
    uint32_t hw_fourcc, fourcc, subsample, chroma_format;

    /* Validate chroma format */
    switch (pic_param->seq_fields.bits.chroma_format_idc) {
    case 0: // Grayscale
        fourcc = VA_FOURCC_Y800;
        subsample = SUBSAMPLE_YUV400;
        chroma_format = VA_RT_FORMAT_YUV400;
        break;
    case 1: // YUV 4:2:0
        fourcc = VA_FOURCC_NV12;
        subsample = SUBSAMPLE_YUV420;
        chroma_format = VA_RT_FORMAT_YUV420;
        break;
    default:
        return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
    }

    /* Determine the HW surface format, bound to VA config needs */
    if ((decode_state->base.chroma_formats & chroma_format) == chroma_format)
        hw_fourcc = fourcc;
    else {
        hw_fourcc = 0;
        switch (fourcc) {
        case VA_FOURCC_Y800: // Implement with an NV12 surface
            if (decode_state->base.chroma_formats & VA_RT_FORMAT_YUV420) {
                hw_fourcc = VA_FOURCC_NV12;
                subsample = SUBSAMPLE_YUV420;
            }
            break;
        }
    }
    if (!hw_fourcc)
        return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;

    /* (Re-)allocate the underlying surface buffer store, if necessary */
    if (!obj_surface->bo || obj_surface->fourcc != hw_fourcc) {
        struct i965_driver_data * const i965 = i965_driver_data(ctx);

        i965_destroy_surface_storage(obj_surface);
        va_status = i965_check_alloc_surface_bo(ctx, obj_surface,
            i965->codec_info->has_tiled_surface, hw_fourcc, subsample);
        if (va_status != VA_STATUS_SUCCESS)
            return va_status;
    }

    /* Fake chroma components if grayscale is implemented on top of NV12 */
    if (fourcc == VA_FOURCC_Y800 && hw_fourcc == VA_FOURCC_NV12) {
        const uint32_t uv_offset = obj_surface->width * obj_surface->height;
        const uint32_t uv_size   = obj_surface->width * obj_surface->height / 2;

        drm_intel_gem_bo_map_gtt(obj_surface->bo);
        memset(obj_surface->bo->virtual + uv_offset, 0x80, uv_size);
        drm_intel_gem_bo_unmap_gtt(obj_surface->bo);
    }
    return VA_STATUS_SUCCESS;
}

/* Generate flat scaling matrices for H.264 decoding */
void
avc_gen_default_iq_matrix(VAIQMatrixBufferH264 *iq_matrix)
{
    /* Flat_4x4_16 */
    memset(&iq_matrix->ScalingList4x4, 16, sizeof(iq_matrix->ScalingList4x4));

    /* Flat_8x8_16 */
    memset(&iq_matrix->ScalingList8x8, 16, sizeof(iq_matrix->ScalingList8x8));
}

/* Returns the POC of the supplied VA picture */
static int
avc_get_picture_poc(const VAPictureH264 *va_pic)
{
    int structure, field_poc[2];

    structure = va_pic->flags &
        (VA_PICTURE_H264_TOP_FIELD | VA_PICTURE_H264_BOTTOM_FIELD);
    field_poc[0] = structure != VA_PICTURE_H264_BOTTOM_FIELD ?
        va_pic->TopFieldOrderCnt : INT_MAX;
    field_poc[1] = structure != VA_PICTURE_H264_TOP_FIELD ?
        va_pic->BottomFieldOrderCnt : INT_MAX;
    return MIN(field_poc[0], field_poc[1]);
}

/* Returns a unique picture ID that represents the supplied VA surface object */
int
avc_get_picture_id(struct object_surface *obj_surface)
{
    int pic_id;

    /* This highly depends on how the internal VA objects are organized.

       Theory of operations:
       The VA objects are maintained in heaps so that any released VA
       surface will become free again for future allocation. This means
       that holes in there are filled in for subsequent allocations.
       So, this ultimately means that we could just use the Heap ID of
       the VA surface as the resulting picture ID (16 bits) */
    pic_id = 1 + (obj_surface->base.id & OBJECT_HEAP_ID_MASK);
    return (pic_id <= 0xffff) ? pic_id : -1;
}

/* Finds the VA/H264 picture associated with the specified VA surface id */
VAPictureH264 *
avc_find_picture(VASurfaceID id, VAPictureH264 *pic_list, int pic_list_count)
{
    int i;

    if (id != VA_INVALID_ID) {
        for (i = 0; i < pic_list_count; i++) {
            VAPictureH264 * const va_pic = &pic_list[i];
            if (va_pic->picture_id == id &&
                !(va_pic->flags & VA_PICTURE_H264_INVALID))
                return va_pic;
        }
    }
    return NULL;
}

/* Get first macroblock bit offset for BSD, minus EPB count (AVC) */
/* XXX: slice_data_bit_offset does not account for EPB */
unsigned int
avc_get_first_mb_bit_offset(
    dri_bo                     *slice_data_bo,
    VASliceParameterBufferH264 *slice_param,
    unsigned int                mode_flag
)
{
    unsigned int slice_data_bit_offset = slice_param->slice_data_bit_offset;

    if (mode_flag == ENTROPY_CABAC)
        slice_data_bit_offset = ALIGN(slice_data_bit_offset, 0x8);
    return slice_data_bit_offset;
}

/* Get first macroblock bit offset for BSD, with EPB count (AVC) */
/* XXX: slice_data_bit_offset does not account for EPB */
unsigned int
avc_get_first_mb_bit_offset_with_epb(
    dri_bo                     *slice_data_bo,
    VASliceParameterBufferH264 *slice_param,
    unsigned int                mode_flag
)
{
    unsigned int in_slice_data_bit_offset = slice_param->slice_data_bit_offset;
    unsigned int out_slice_data_bit_offset;
    unsigned int i, j, n, buf_size, data_size, header_size;
    uint8_t *buf;
    int ret;

    header_size = slice_param->slice_data_bit_offset / 8;
    data_size   = slice_param->slice_data_size - slice_param->slice_data_offset;
    buf_size    = (header_size * 3 + 1) / 2; // Max possible header size (x1.5)

    if (buf_size > data_size)
        buf_size = data_size;

    buf = alloca(buf_size);
    ret = dri_bo_get_subdata(
        slice_data_bo, slice_param->slice_data_offset,
        buf_size, buf
    );
    assert(ret == 0);

    for (i = 2, j = 2, n = 0; i < buf_size && j < header_size; i++, j++) {
        if (buf[i] == 0x03 && buf[i - 1] == 0x00 && buf[i - 2] == 0x00)
            i += 2, j++, n++;
    }

    out_slice_data_bit_offset = in_slice_data_bit_offset + n * 8;

    if (mode_flag == ENTROPY_CABAC)
        out_slice_data_bit_offset = ALIGN(out_slice_data_bit_offset, 0x8);
    return out_slice_data_bit_offset;
}

static inline uint8_t
get_ref_idx_state_1(const VAPictureH264 *va_pic, unsigned int frame_store_id)
{
    /* The H.264 standard, and the VA-API specification, allows for at
       least 3 states for a picture: "used for short-term reference",
       "used for long-term reference", or considered as not used for
       reference.

       The latter is used in the MVC inter prediction and inter-view
       prediction process (H.8.4). This has an incidence on the
       colZeroFlag variable, as defined in 8.4.1.2.

       Since it is not possible to directly program that flag, let's
       make the hardware derive this value by assimilating "considered
       as not used for reference" to a "not used for short-term
       reference", and subsequently making it "used for long-term
       reference" to fit the definition of Bit6 here */
    const unsigned int ref_flags = VA_PICTURE_H264_SHORT_TERM_REFERENCE |
        VA_PICTURE_H264_LONG_TERM_REFERENCE;
    const unsigned int is_long_term =
        ((va_pic->flags & ref_flags) != VA_PICTURE_H264_SHORT_TERM_REFERENCE);
    const unsigned int is_top_field =
        !!(va_pic->flags & VA_PICTURE_H264_TOP_FIELD);
    const unsigned int is_bottom_field =
        !!(va_pic->flags & VA_PICTURE_H264_BOTTOM_FIELD);

    return ((is_long_term                         << 6) |
            ((is_top_field ^ is_bottom_field ^ 1) << 5) |
            (frame_store_id                       << 1) |
            ((is_top_field ^ 1) & is_bottom_field));
}

/* Fill in Reference List Entries (Gen5+: ILK, SNB, IVB) */
void
gen5_fill_avc_ref_idx_state(
    uint8_t             state[32],
    const VAPictureH264 ref_list[32],
    unsigned int        ref_list_count,
    const GenFrameStore frame_store[MAX_GEN_REFERENCE_FRAMES]
)
{
    int i, j;

    for (i = 0; i < ref_list_count; i++) {
        const VAPictureH264 * const va_pic = &ref_list[i];

        if ((va_pic->flags & VA_PICTURE_H264_INVALID) ||
            va_pic->picture_id == VA_INVALID_ID) {
            state[i] = 0xff;
            continue;
        }

        for (j = 0; j < MAX_GEN_REFERENCE_FRAMES; j++) {
            if (frame_store[j].surface_id == va_pic->picture_id)
                break;
        }

        if (j != MAX_GEN_REFERENCE_FRAMES) { // Found picture in the Frame Store
            const GenFrameStore * const fs = &frame_store[j];
            assert(fs->frame_store_id == j); // Current architecture/assumption
            state[i] = get_ref_idx_state_1(va_pic, fs->frame_store_id);
        }
        else {
            WARN_ONCE("Invalid RefPicListX[] entry!!! It is not included in DPB\n");
            state[i] = get_ref_idx_state_1(va_pic, 0) | 0x80;
        }
    }

    for (; i < 32; i++)
        state[i] = 0xff;
}

/* Emit Reference List Entries (Gen6+: SNB, IVB) */
static void
gen6_send_avc_ref_idx_state_1(
    struct intel_batchbuffer         *batch,
    unsigned int                      list,
    const VAPictureH264              *ref_list,
    unsigned int                      ref_list_count,
    const GenFrameStore               frame_store[MAX_GEN_REFERENCE_FRAMES]
)
{
    uint8_t ref_idx_state[32];

    BEGIN_BCS_BATCH(batch, 10);
    OUT_BCS_BATCH(batch, MFX_AVC_REF_IDX_STATE | (10 - 2));
    OUT_BCS_BATCH(batch, list);
    gen5_fill_avc_ref_idx_state(
        ref_idx_state,
        ref_list, ref_list_count,
        frame_store
    );
    intel_batchbuffer_data(batch, ref_idx_state, sizeof(ref_idx_state));
    ADVANCE_BCS_BATCH(batch);
}

void
gen6_send_avc_ref_idx_state(
    struct intel_batchbuffer         *batch,
    const VASliceParameterBufferH264 *slice_param,
    const GenFrameStore               frame_store[MAX_GEN_REFERENCE_FRAMES]
)
{
    if (slice_param->slice_type == SLICE_TYPE_I ||
        slice_param->slice_type == SLICE_TYPE_SI)
        return;

    /* RefPicList0 */
    gen6_send_avc_ref_idx_state_1(
        batch, 0,
        slice_param->RefPicList0, slice_param->num_ref_idx_l0_active_minus1 + 1,
        frame_store
    );

    if (slice_param->slice_type != SLICE_TYPE_B)
        return;

    /* RefPicList1 */
    gen6_send_avc_ref_idx_state_1(
        batch, 1,
        slice_param->RefPicList1, slice_param->num_ref_idx_l1_active_minus1 + 1,
        frame_store
    );
}

static void
gen6_mfd_avc_phantom_slice_state(VADriverContextP ctx,
                                 VAPictureParameterBufferH264 *pic_param,
                                 VASliceParameterBufferH264 *next_slice_param,
                                 struct intel_batchbuffer *batch)
{
    int width_in_mbs = pic_param->picture_width_in_mbs_minus1 + 1;
    int height_in_mbs = pic_param->picture_height_in_mbs_minus1 + 1; /* frame height */
    int slice_hor_pos, slice_ver_pos, slice_start_mb_num, next_slice_hor_pos, next_slice_ver_pos;
    int mbaff_picture = (!pic_param->pic_fields.bits.field_pic_flag &&
                         pic_param->seq_fields.bits.mb_adaptive_frame_field_flag);

    if (next_slice_param) {
        int first_mb_in_next_slice;

        slice_hor_pos = 0;
        slice_ver_pos = 0;
        slice_start_mb_num = 0;
        first_mb_in_next_slice = next_slice_param->first_mb_in_slice << mbaff_picture;
        next_slice_hor_pos = first_mb_in_next_slice % width_in_mbs;
        next_slice_ver_pos = first_mb_in_next_slice / width_in_mbs;
    } else {
        slice_hor_pos = 0;
        slice_ver_pos = height_in_mbs;
        slice_start_mb_num = width_in_mbs * height_in_mbs / (1 + !!pic_param->pic_fields.bits.field_pic_flag);
        next_slice_hor_pos = 0;
        next_slice_ver_pos = 0;
    }

    BEGIN_BCS_BATCH(batch, 11);
    OUT_BCS_BATCH(batch, MFX_AVC_SLICE_STATE | (11 - 2));
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch,
                  slice_ver_pos << 24 |
                  slice_hor_pos << 16 |
                  slice_start_mb_num << 0);
    OUT_BCS_BATCH(batch,
                  next_slice_ver_pos << 16 |
                  next_slice_hor_pos << 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    ADVANCE_BCS_BATCH(batch);
}

static void
gen6_mfd_avc_phantom_slice_bsd_object(VADriverContextP ctx,
                                      VAPictureParameterBufferH264 *pic_param,
                                      struct intel_batchbuffer *batch)
{

    BEGIN_BCS_BATCH(batch, 6);
    OUT_BCS_BATCH(batch, MFD_AVC_BSD_OBJECT | (6 - 2));
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    ADVANCE_BCS_BATCH(batch);
}

void
gen6_mfd_avc_phantom_slice(VADriverContextP ctx,
                           VAPictureParameterBufferH264 *pic_param,
                           VASliceParameterBufferH264 *next_slice_param,
                           struct intel_batchbuffer *batch)
{
    gen6_mfd_avc_phantom_slice_state(ctx, pic_param, next_slice_param, batch);
    gen6_mfd_avc_phantom_slice_bsd_object(ctx, pic_param, batch);
}

/* Comparison function for sorting out the array of free frame store entries */
static int
compare_avc_ref_store_func(const void *p1, const void *p2)
{
    const GenFrameStore * const fs1 = *((GenFrameStore **)p1);
    const GenFrameStore * const fs2 = *((GenFrameStore **)p2);

    return fs1->ref_age - fs2->ref_age;
}

static void
intel_update_codec_frame_store_index(
    VADriverContextP              ctx,
    struct decode_state          *decode_state,
    int poc,
    GenFrameStore                 frame_store[],
    int num_elements,
    GenFrameStoreContext         *fs_ctx
)
{
    GenFrameStore **free_refs = calloc(num_elements, sizeof(GenFrameStore *));
    uint32_t used_refs = 0, add_refs = 0;
    uint64_t age;
    int i, n, num_free_refs;

    if (!free_refs)
        return;

    /* Detect changes of access unit */
    if (fs_ctx->age == 0 || fs_ctx->prev_poc != poc)
        fs_ctx->age++;
    fs_ctx->prev_poc = poc;
    age = fs_ctx->age;

    /* Tag entries that are still available in our Frame Store */
    for (i = 0; i < ARRAY_ELEMS(decode_state->reference_objects); i++) {
        struct object_surface * const obj_surface =
            decode_state->reference_objects[i];
        if (!obj_surface)
            continue;

        GenCodecSurface * const codec_surface = obj_surface->private_data;
        if (!codec_surface)
            continue;
        if (codec_surface->frame_store_id >= 0) {
            GenFrameStore * const fs =
                &frame_store[codec_surface->frame_store_id];
            if (fs->surface_id == obj_surface->base.id) {
                fs->obj_surface = obj_surface;
                fs->ref_age = age;
                used_refs |= 1 << fs->frame_store_id;
                continue;
            }
        }
        add_refs |= 1 << i;
    }

    /* Build and sort out the list of retired candidates. The resulting
       list is ordered by increasing age when they were last used */
    for (i = 0, n = 0; i < num_elements; i++) {
        if (!(used_refs & (1 << i))) {
            GenFrameStore * const fs = &frame_store[i];
            fs->obj_surface = NULL;
            free_refs[n++] = fs;
        }
    }
    num_free_refs = n;
    qsort(&free_refs[0], n, sizeof(free_refs[0]), compare_avc_ref_store_func);

    /* Append the new reference frames */
    for (i = 0, n = 0; i < ARRAY_ELEMS(decode_state->reference_objects); i++) {
        struct object_surface * const obj_surface =
            decode_state->reference_objects[i];
        if (!obj_surface || !(add_refs & (1 << i)))
            continue;

        GenCodecSurface * const codec_surface = obj_surface->private_data;
        if (!codec_surface)
            continue;
        if (n < num_free_refs) {
            GenFrameStore * const fs = free_refs[n++];
            fs->surface_id = obj_surface->base.id;
            fs->obj_surface = obj_surface;
            fs->frame_store_id = fs - frame_store;
            fs->ref_age = age;
            codec_surface->frame_store_id = fs->frame_store_id;
            continue;
        }
        WARN_ONCE("No free slot found for DPB reference list!!!\n");
    }

    free(free_refs);
}

void
intel_update_avc_frame_store_index(
    VADriverContextP              ctx,
    struct decode_state          *decode_state,
    VAPictureParameterBufferH264 *pic_param,
    GenFrameStore                 frame_store[MAX_GEN_REFERENCE_FRAMES],
    GenFrameStoreContext         *fs_ctx
)
{
    intel_update_codec_frame_store_index(ctx,
                                         decode_state,
                                         avc_get_picture_poc(&pic_param->CurrPic),
                                         frame_store,
                                         MAX_GEN_REFERENCE_FRAMES,
                                         fs_ctx);
}

void
intel_update_hevc_frame_store_index(
    VADriverContextP              ctx,
    struct decode_state          *decode_state,
    VAPictureParameterBufferHEVC *pic_param,
    GenFrameStore                 frame_store[MAX_GEN_HCP_REFERENCE_FRAMES],
    GenFrameStoreContext         *fs_ctx
    )
{
    int i, n = 0;

    for (i = 0; i < ARRAY_ELEMS(decode_state->reference_objects); i++) {
        struct object_surface * const obj_surface = decode_state->reference_objects[i];

        if (!obj_surface)
            continue;

        GenFrameStore * const fs = &frame_store[n];
        fs->surface_id = obj_surface->base.id;
        fs->obj_surface = obj_surface;
        fs->frame_store_id = n++;

        if (n == MAX_GEN_HCP_REFERENCE_FRAMES)
            break;
    }

    for (; n < MAX_GEN_HCP_REFERENCE_FRAMES; n++) {
        GenFrameStore * const fs = &frame_store[n];

        fs->surface_id = VA_INVALID_ID;
        fs->obj_surface = NULL;
        fs->frame_store_id = -1;
    }
}

void
gen75_update_avc_frame_store_index(
    VADriverContextP              ctx,
    struct decode_state          *decode_state,
    VAPictureParameterBufferH264 *pic_param,
    GenFrameStore                 frame_store[MAX_GEN_REFERENCE_FRAMES]
)
{
    int i, n;

    /* Construct the Frame Store array, in compact form. i.e. empty or
       invalid entries are discarded. */
    for (i = 0, n = 0; i < ARRAY_ELEMS(decode_state->reference_objects); i++) {
        struct object_surface * const obj_surface =
            decode_state->reference_objects[i];
        if (!obj_surface)
            continue;

        GenFrameStore * const fs = &frame_store[n];
        fs->surface_id = obj_surface->base.id;
        fs->obj_surface = obj_surface;
        fs->frame_store_id = n++;
    }

    /* Any remaining entry is marked as invalid */
    for (; n < MAX_GEN_REFERENCE_FRAMES; n++) {
        GenFrameStore * const fs = &frame_store[n];
        fs->surface_id = VA_INVALID_ID;
        fs->obj_surface = NULL;
        fs->frame_store_id = -1;
    }
}

bool
gen75_fill_avc_picid_list(
    uint16_t                    pic_ids[16],
    GenFrameStore               frame_store[MAX_GEN_REFERENCE_FRAMES]
)
{
    int i, pic_id;

    /* Fill in with known picture IDs. The Frame Store array is in
       compact form, i.e. empty entries are only to be found at the
       end of the array: there are no holes in the set of active
       reference frames */
    for (i = 0; i < MAX_GEN_REFERENCE_FRAMES; i++) {
        GenFrameStore * const fs = &frame_store[i];
        if (!fs->obj_surface)
            break;
        pic_id = avc_get_picture_id(fs->obj_surface);
        if (pic_id < 0)
            return false;
        pic_ids[i] = pic_id;
    }

    /* When an element of the list is not relevant the value of the
       picture ID shall be set to 0 */
    for (; i < MAX_GEN_REFERENCE_FRAMES; i++)
        pic_ids[i] = 0;
    return true;
}

bool
gen75_send_avc_picid_state(
    struct intel_batchbuffer   *batch,
    GenFrameStore               frame_store[MAX_GEN_REFERENCE_FRAMES]
)
{
    uint16_t pic_ids[16];

    if (!gen75_fill_avc_picid_list(pic_ids, frame_store))
        return false;

    BEGIN_BCS_BATCH(batch, 10);
    OUT_BCS_BATCH(batch, MFD_AVC_PICID_STATE | (10 - 2));
    OUT_BCS_BATCH(batch, 0); // enable Picture ID Remapping
    intel_batchbuffer_data(batch, pic_ids, sizeof(pic_ids));
    ADVANCE_BCS_BATCH(batch);
    return true;
}

void
intel_update_vc1_frame_store_index(VADriverContextP ctx,
                                   struct decode_state *decode_state,
                                   VAPictureParameterBufferVC1 *pic_param,
                                   GenFrameStore frame_store[MAX_GEN_REFERENCE_FRAMES])
{
    struct object_surface *obj_surface;
    int i;

    obj_surface = decode_state->reference_objects[0];

    if (pic_param->forward_reference_picture == VA_INVALID_ID ||
        !obj_surface || 
        !obj_surface->bo) {
        frame_store[0].surface_id = VA_INVALID_ID;
        frame_store[0].obj_surface = NULL;
    } else {
        frame_store[0].surface_id = pic_param->forward_reference_picture;
        frame_store[0].obj_surface = obj_surface;
    }

    obj_surface = decode_state->reference_objects[1];

    if (pic_param->backward_reference_picture == VA_INVALID_ID ||
        !obj_surface || 
        !obj_surface->bo) {
        frame_store[1].surface_id = frame_store[0].surface_id;
        frame_store[1].obj_surface = frame_store[0].obj_surface;
    } else {
        frame_store[1].surface_id = pic_param->backward_reference_picture;
        frame_store[1].obj_surface = obj_surface;
    }
    for (i = 2; i < MAX_GEN_REFERENCE_FRAMES; i++) {
        frame_store[i].surface_id = frame_store[i % 2].surface_id;
        frame_store[i].obj_surface = frame_store[i % 2].obj_surface;
    }

}

void
intel_update_vp8_frame_store_index(VADriverContextP ctx,
                                   struct decode_state *decode_state,
                                   VAPictureParameterBufferVP8 *pic_param,
                                   GenFrameStore frame_store[MAX_GEN_REFERENCE_FRAMES])
{
    struct object_surface *obj_surface;
    int i;

    obj_surface = decode_state->reference_objects[0];

    if (pic_param->last_ref_frame == VA_INVALID_ID ||
        !obj_surface ||
        !obj_surface->bo) {
        frame_store[0].surface_id = VA_INVALID_ID;
        frame_store[0].obj_surface = NULL;
    } else {
        frame_store[0].surface_id = pic_param->last_ref_frame;
        frame_store[0].obj_surface = obj_surface;
    }

    obj_surface = decode_state->reference_objects[1];

    if (pic_param->golden_ref_frame == VA_INVALID_ID ||
        !obj_surface ||
        !obj_surface->bo) {
        frame_store[1].surface_id = frame_store[0].surface_id;
        frame_store[1].obj_surface = frame_store[0].obj_surface;
    } else {
        frame_store[1].surface_id = pic_param->golden_ref_frame;
        frame_store[1].obj_surface = obj_surface;
    }

    obj_surface = decode_state->reference_objects[2];

    if (pic_param->alt_ref_frame == VA_INVALID_ID ||
        !obj_surface ||
        !obj_surface->bo) {
        frame_store[2].surface_id = frame_store[0].surface_id;
        frame_store[2].obj_surface = frame_store[0].obj_surface;
    } else {
        frame_store[2].surface_id = pic_param->alt_ref_frame;
        frame_store[2].obj_surface = obj_surface;
    }

    for (i = 3; i < MAX_GEN_REFERENCE_FRAMES; i++) {
        frame_store[i].surface_id = frame_store[i % 2].surface_id;
        frame_store[i].obj_surface = frame_store[i % 2].obj_surface;
    }

}

//Obtain the reference frames from the decode state and store them in frame store.
void
intel_update_vp9_frame_store_index(VADriverContextP ctx,
                                   struct decode_state *decode_state,
                                   VADecPictureParameterBufferVP9 *pic_param,
                                   GenFrameStore frame_store[MAX_GEN_REFERENCE_FRAMES])
{
    struct object_surface *obj_surface;
    int i=0, index=0;

    //Check for the validity of the last reference frame
    obj_surface = decode_state->reference_objects[0];

    index = pic_param->pic_fields.bits.last_ref_frame;
    if (pic_param->reference_frames[index] == VA_INVALID_ID ||
        !obj_surface ||
        !obj_surface->bo) {
        frame_store[0].surface_id = VA_INVALID_ID;
        frame_store[0].obj_surface = NULL;
    } else {
        frame_store[0].surface_id = pic_param->reference_frames[index];
        frame_store[0].obj_surface = obj_surface;
    }

    //Check for the validity of the golden reference frame
    obj_surface = decode_state->reference_objects[1];

    index = pic_param->pic_fields.bits.golden_ref_frame;
    if (pic_param->reference_frames[index] == VA_INVALID_ID ||
        !obj_surface ||
        !obj_surface->bo) {
        frame_store[1].surface_id = frame_store[0].surface_id;
        frame_store[1].obj_surface = frame_store[0].obj_surface;
    } else {
        frame_store[1].surface_id = pic_param->reference_frames[index];
        frame_store[1].obj_surface = obj_surface;
    }

    //Check for the validity of the altref reference frame
    obj_surface = decode_state->reference_objects[2];

    index = pic_param->pic_fields.bits.alt_ref_frame;
    if (pic_param->reference_frames[index] == VA_INVALID_ID ||
        !obj_surface ||
        !obj_surface->bo) {
        frame_store[2].surface_id = frame_store[0].surface_id;
        frame_store[2].obj_surface = frame_store[0].obj_surface;
    } else {
        frame_store[2].surface_id = pic_param->reference_frames[index];
        frame_store[2].obj_surface = obj_surface;
    }

    //Set the remaining framestores to either last/golden/altref
    for (i = 3; i < MAX_GEN_REFERENCE_FRAMES; i++) {
        frame_store[i].surface_id = frame_store[i % 2].surface_id;
        frame_store[i].obj_surface = frame_store[i % 2].obj_surface;
    }

}

static VAStatus
intel_decoder_check_avc_parameter(VADriverContextP ctx,
                                  VAProfile h264_profile,
                                  struct decode_state *decode_state)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    VAPictureParameterBufferH264 *pic_param = (VAPictureParameterBufferH264 *)decode_state->pic_param->buffer;
    VAStatus va_status;
    struct object_surface *obj_surface;	
    int i;
    VASliceParameterBufferH264 *slice_param, *next_slice_param, *next_slice_group_param;
    int j;

    assert(!(pic_param->CurrPic.flags & VA_PICTURE_H264_INVALID));
    assert(pic_param->CurrPic.picture_id != VA_INVALID_SURFACE);

    if (pic_param->CurrPic.flags & VA_PICTURE_H264_INVALID ||
        pic_param->CurrPic.picture_id == VA_INVALID_SURFACE)
        goto error;

    assert(pic_param->CurrPic.picture_id == decode_state->current_render_target);

    if (pic_param->CurrPic.picture_id != decode_state->current_render_target)
        goto error;

    if ((h264_profile != VAProfileH264Baseline)) {
       if (pic_param->num_slice_groups_minus1 ||
           pic_param->pic_fields.bits.redundant_pic_cnt_present_flag) {
           WARN_ONCE("Unsupported the FMO/ASO constraints!!!\n");
           goto error;
       }
    }

    /* Fill in the reference objects array with the actual VA surface
       objects with 1:1 correspondance with any entry in ReferenceFrames[],
       i.e. including "holes" for invalid entries, that are expanded
       to NULL in the reference_objects[] array */
    for (i = 0; i < ARRAY_ELEMS(pic_param->ReferenceFrames); i++) {
        const VAPictureH264 * const va_pic = &pic_param->ReferenceFrames[i];

        obj_surface = NULL;
        if (!(va_pic->flags & VA_PICTURE_H264_INVALID) &&
            va_pic->picture_id != VA_INVALID_ID) {
            obj_surface = SURFACE(pic_param->ReferenceFrames[i].picture_id);
            if (!obj_surface)
                return VA_STATUS_ERROR_INVALID_SURFACE;

            /*
             * Sometimes a dummy frame comes from the upper layer
             * library, call i965_check_alloc_surface_bo() to make
             * sure the store buffer is allocated for this reference
             * frame
             */
            va_status = avc_ensure_surface_bo(ctx, decode_state, obj_surface,
                pic_param);
            if (va_status != VA_STATUS_SUCCESS)
                return va_status;
        }
        decode_state->reference_objects[i] = obj_surface;
    }

    for (j = 0; j < decode_state->num_slice_params; j++) {
        assert(decode_state->slice_params && decode_state->slice_params[j]->buffer);
        slice_param = (VASliceParameterBufferH264 *)decode_state->slice_params[j]->buffer;

        if (j == decode_state->num_slice_params - 1)
            next_slice_group_param = NULL;
        else
            next_slice_group_param = (VASliceParameterBufferH264 *)decode_state->slice_params[j + 1]->buffer;

        for (i = 0; i < decode_state->slice_params[j]->num_elements; i++) {

            if (i < decode_state->slice_params[j]->num_elements - 1)
                next_slice_param = slice_param + 1;
            else
                next_slice_param = next_slice_group_param;

            if (next_slice_param != NULL) {
                /* If the mb position of next_slice is less than or equal to the current slice,
                 * discard the current frame.
                 */
                if (next_slice_param->first_mb_in_slice <= slice_param->first_mb_in_slice) {
                    next_slice_param = NULL;
                    WARN_ONCE("!!!incorrect slice_param. The first_mb_in_slice of next_slice is less"
                               " than or equal to that in current slice\n");
                    goto error;
                }
            }
        }
    }

    return VA_STATUS_SUCCESS;

error:
    return VA_STATUS_ERROR_INVALID_PARAMETER;
}

static VAStatus
intel_decoder_check_mpeg2_parameter(VADriverContextP ctx,
                                    struct decode_state *decode_state)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    VAPictureParameterBufferMPEG2 *pic_param = (VAPictureParameterBufferMPEG2 *)decode_state->pic_param->buffer;
    struct object_surface *obj_surface;	
    int i = 0;
    
    if (pic_param->picture_coding_type == MPEG_I_PICTURE) {
    } else if (pic_param->picture_coding_type == MPEG_P_PICTURE) {
        obj_surface = SURFACE(pic_param->forward_reference_picture);

        if (!obj_surface || !obj_surface->bo)
            decode_state->reference_objects[i++] = NULL;
        else
            decode_state->reference_objects[i++] = obj_surface;
    } else if (pic_param->picture_coding_type == MPEG_B_PICTURE) {
        obj_surface = SURFACE(pic_param->forward_reference_picture);

        if (!obj_surface || !obj_surface->bo)
            decode_state->reference_objects[i++] = NULL;
        else
            decode_state->reference_objects[i++] = obj_surface;

        obj_surface = SURFACE(pic_param->backward_reference_picture);

        if (!obj_surface || !obj_surface->bo)
            decode_state->reference_objects[i++] = NULL;
        else
            decode_state->reference_objects[i++] = obj_surface;
    } else
        goto error;

    for ( ; i < 16; i++)
        decode_state->reference_objects[i] = NULL;

    return VA_STATUS_SUCCESS;

error:
    return VA_STATUS_ERROR_INVALID_PARAMETER;
}

static VAStatus
intel_decoder_check_vc1_parameter(VADriverContextP ctx,
                                  struct decode_state *decode_state)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    VAPictureParameterBufferVC1 *pic_param = (VAPictureParameterBufferVC1 *)decode_state->pic_param->buffer;
    struct object_surface *obj_surface;	
    int i = 0;

    if (pic_param->sequence_fields.bits.interlace == 1 &&
        pic_param->picture_fields.bits.frame_coding_mode != 0) { /* frame-interlace or field-interlace */
        return VA_STATUS_ERROR_DECODING_ERROR;
    }

    if (pic_param->picture_fields.bits.picture_type == 0 ||
        pic_param->picture_fields.bits.picture_type == 3) {
    } else if (pic_param->picture_fields.bits.picture_type == 1 ||
               pic_param->picture_fields.bits.picture_type == 4) {
        obj_surface = SURFACE(pic_param->forward_reference_picture);

        if (!obj_surface || !obj_surface->bo)
            decode_state->reference_objects[i++] = NULL;
        else
            decode_state->reference_objects[i++] = obj_surface;
    } else if (pic_param->picture_fields.bits.picture_type == 2) {
        obj_surface = SURFACE(pic_param->forward_reference_picture);

        if (!obj_surface || !obj_surface->bo)
            decode_state->reference_objects[i++] = NULL;
        else
            decode_state->reference_objects[i++] = obj_surface;

        obj_surface = SURFACE(pic_param->backward_reference_picture);

        if (!obj_surface || !obj_surface->bo)
            decode_state->reference_objects[i++] = NULL;
        else
            decode_state->reference_objects[i++] = obj_surface;
    } else 
        goto error;

    for ( ; i < 16; i++)
        decode_state->reference_objects[i] = NULL;

    return VA_STATUS_SUCCESS;

error:
    return VA_STATUS_ERROR_INVALID_PARAMETER;
}

static VAStatus
intel_decoder_check_vp8_parameter(VADriverContextP ctx,
                                  struct decode_state *decode_state)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    VAPictureParameterBufferVP8 *pic_param = (VAPictureParameterBufferVP8 *)decode_state->pic_param->buffer;
    struct object_surface *obj_surface;	
    int i = 0;

    if (pic_param->last_ref_frame != VA_INVALID_SURFACE) {
        obj_surface = SURFACE(pic_param->last_ref_frame);

        if (obj_surface && obj_surface->bo)
            decode_state->reference_objects[i++] = obj_surface;
        else
            decode_state->reference_objects[i++] = NULL;
    }

    if (pic_param->golden_ref_frame != VA_INVALID_SURFACE) {
        obj_surface = SURFACE(pic_param->golden_ref_frame);

        if (obj_surface && obj_surface->bo)
            decode_state->reference_objects[i++] = obj_surface;
        else
            decode_state->reference_objects[i++] = NULL;
    }

    if (pic_param->alt_ref_frame != VA_INVALID_SURFACE) {
        obj_surface = SURFACE(pic_param->alt_ref_frame);

        if (obj_surface && obj_surface->bo)
            decode_state->reference_objects[i++] = obj_surface;
        else
            decode_state->reference_objects[i++] = NULL;
    }

    for ( ; i < 16; i++)
        decode_state->reference_objects[i] = NULL;

    return VA_STATUS_SUCCESS;
}

VAStatus
hevc_ensure_surface_bo(
    VADriverContextP                    ctx,
    struct decode_state                *decode_state,
    struct object_surface              *obj_surface,
    const VAPictureParameterBufferHEVC *pic_param
)
{
    VAStatus va_status = VA_STATUS_SUCCESS;
    int update = 0;
    unsigned int fourcc = VA_FOURCC_NV12;

    if((pic_param->bit_depth_luma_minus8 > 0)
        || (pic_param->bit_depth_chroma_minus8 > 0))
    {
        if(obj_surface->fourcc != VA_FOURCC_P010)
        {
            update = 1;
            fourcc = VA_FOURCC_P010;
        }
    }
    else if(obj_surface->fourcc != VA_FOURCC_NV12)
    {
        update = 1;
        fourcc = VA_FOURCC_NV12;
    }

    /* (Re-)allocate the underlying surface buffer store, if necessary */
    if (!obj_surface->bo || update) {
        struct i965_driver_data * const i965 = i965_driver_data(ctx);

        i965_destroy_surface_storage(obj_surface);

        va_status = i965_check_alloc_surface_bo(ctx,
                                                obj_surface,
                                                i965->codec_info->has_tiled_surface,
                                                fourcc,
                                                SUBSAMPLE_YUV420);
    }

    return va_status;
}

//Ensure there is a tiled render surface in NV12 format. If not, create one.
VAStatus
vp9_ensure_surface_bo(
    VADriverContextP                    ctx,
    struct decode_state                *decode_state,
    struct object_surface              *obj_surface,
    const VADecPictureParameterBufferVP9 *pic_param
)
{
    VAStatus va_status = VA_STATUS_SUCCESS;

    /* (Re-)allocate the underlying surface buffer store, if necessary */
    if (!obj_surface->bo || obj_surface->fourcc != VA_FOURCC_NV12) {
        struct i965_driver_data * const i965 = i965_driver_data(ctx);

        i965_destroy_surface_storage(obj_surface);

        va_status = i965_check_alloc_surface_bo(ctx,
                                                obj_surface,
                                                i965->codec_info->has_tiled_surface,
                                                VA_FOURCC_NV12,
                                                SUBSAMPLE_YUV420);
    }

    return va_status;
}

static VAStatus
intel_decoder_check_hevc_parameter(VADriverContextP ctx,
                                   struct decode_state *decode_state)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    VAPictureParameterBufferHEVC *pic_param = (VAPictureParameterBufferHEVC *)decode_state->pic_param->buffer;
    VAStatus va_status = VA_STATUS_ERROR_INVALID_PARAMETER;
    struct object_surface *obj_surface;
    int i;
    int min_cb_size;

    if (pic_param->CurrPic.flags & VA_PICTURE_HEVC_INVALID ||
        pic_param->CurrPic.picture_id == VA_INVALID_SURFACE)
        goto error;

    if (pic_param->CurrPic.picture_id != decode_state->current_render_target)
        goto error;

    min_cb_size = (1 << (pic_param->log2_min_luma_coding_block_size_minus3 + 3));

    if (pic_param->pic_width_in_luma_samples % min_cb_size ||
        pic_param->pic_height_in_luma_samples % min_cb_size)
        goto error;

    /* Fill in the reference objects array with the actual VA surface
       objects with 1:1 correspondance with any entry in ReferenceFrames[],
       i.e. including "holes" for invalid entries, that are expanded
       to NULL in the reference_objects[] array */
    for (i = 0; i < ARRAY_ELEMS(pic_param->ReferenceFrames); i++) {
        const VAPictureHEVC * const va_pic = &pic_param->ReferenceFrames[i];

        obj_surface = NULL;

        /*
         * Only the index with (VA_PICTURE_HEVC_RPS_ST_CURR_BEFORE |
         * VA_PICTURE_HEVC_RPS_ST_CURR_AFTER | VA_PICTURE_HEVC_RPS_LT_CURR)
         * is valid
         */
        if (!(va_pic->flags & VA_PICTURE_HEVC_INVALID) &&
            (va_pic->picture_id != VA_INVALID_ID) &&
            (va_pic->flags & (VA_PICTURE_HEVC_RPS_ST_CURR_BEFORE |
                              VA_PICTURE_HEVC_RPS_ST_CURR_AFTER |
                              VA_PICTURE_HEVC_RPS_LT_CURR))) {

            obj_surface = SURFACE(pic_param->ReferenceFrames[i].picture_id);

            if (!obj_surface) {
                va_status = VA_STATUS_ERROR_INVALID_SURFACE;
                goto error;
            }

            va_status = hevc_ensure_surface_bo(ctx, decode_state, obj_surface,
                                               pic_param);

            if (va_status != VA_STATUS_SUCCESS)
                goto error;
        }

        decode_state->reference_objects[i] = obj_surface;
    }

    va_status = VA_STATUS_SUCCESS;

error:
    return va_status;
}

//Obtains reference frames from the picture parameter and
//then sets the reference frames in the decode_state
static VAStatus
intel_decoder_check_vp9_parameter(VADriverContextP ctx,
                                   struct decode_state *decode_state)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    VADecPictureParameterBufferVP9 *pic_param = (VADecPictureParameterBufferVP9 *)decode_state->pic_param->buffer;
    VAStatus va_status = VA_STATUS_ERROR_INVALID_PARAMETER;
    struct object_surface *obj_surface;
    int i=0, index=0;

    //Max support upto 4k for BXT
    if ((pic_param->frame_width-1 < 7) || (pic_param->frame_width-1 > 4095))
        return va_status;

    if ((pic_param->frame_height-1 < 7) || (pic_param->frame_height-1 > 4095))
        return va_status;

    //Set the reference object in decode state for last reference
    index = pic_param->pic_fields.bits.last_ref_frame;
    if (pic_param->reference_frames[index] != VA_INVALID_SURFACE) {
        obj_surface = SURFACE(pic_param->reference_frames[index]);

        if (obj_surface && obj_surface->bo)
            decode_state->reference_objects[i++] = obj_surface;
        else
            decode_state->reference_objects[i++] = NULL;
    }

    //Set the reference object in decode state for golden reference
    index = pic_param->pic_fields.bits.golden_ref_frame;
    if (pic_param->reference_frames[index] != VA_INVALID_SURFACE) {
        obj_surface = SURFACE(pic_param->reference_frames[index]);

        if (obj_surface && obj_surface->bo)
            decode_state->reference_objects[i++] = obj_surface;
        else
            decode_state->reference_objects[i++] = NULL;
    }

    //Set the reference object in decode state for altref reference
    index = pic_param->pic_fields.bits.alt_ref_frame;
    if (pic_param->reference_frames[index] != VA_INVALID_SURFACE) {
        obj_surface = SURFACE(pic_param->reference_frames[index]);

        if (obj_surface && obj_surface->bo)
            decode_state->reference_objects[i++] = obj_surface;
        else
            decode_state->reference_objects[i++] = NULL;
    }

    for ( ; i < 16; i++)
        decode_state->reference_objects[i] = NULL;

    return VA_STATUS_SUCCESS;
}

VAStatus
intel_decoder_sanity_check_input(VADriverContextP ctx,
                                 VAProfile profile,
                                 struct decode_state *decode_state)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_surface *obj_surface;
    VAStatus vaStatus = VA_STATUS_ERROR_INVALID_PARAMETER;

    if (decode_state->current_render_target == VA_INVALID_SURFACE)
        goto out;
        
    obj_surface = SURFACE(decode_state->current_render_target);

    if (!obj_surface)
        goto out;

    decode_state->render_object = obj_surface;

    switch (profile) {
    case VAProfileMPEG2Simple:
    case VAProfileMPEG2Main:
        vaStatus = intel_decoder_check_mpeg2_parameter(ctx, decode_state);
        break;
        
    case VAProfileH264ConstrainedBaseline:
    case VAProfileH264Main:
    case VAProfileH264High:
    case VAProfileH264StereoHigh:
    case VAProfileH264MultiviewHigh:
        vaStatus = intel_decoder_check_avc_parameter(ctx, profile, decode_state);
        break;

    case VAProfileVC1Simple:
    case VAProfileVC1Main:
    case VAProfileVC1Advanced:
        vaStatus = intel_decoder_check_vc1_parameter(ctx, decode_state);
        break;

    case VAProfileJPEGBaseline:
        vaStatus = VA_STATUS_SUCCESS;
        break;

    case VAProfileVP8Version0_3:
        vaStatus = intel_decoder_check_vp8_parameter(ctx, decode_state);
        break;

    case VAProfileHEVCMain:
    case VAProfileHEVCMain10:
        vaStatus = intel_decoder_check_hevc_parameter(ctx, decode_state);
        break;

    case VAProfileVP9Profile0:
        vaStatus = intel_decoder_check_vp9_parameter(ctx, decode_state);
        break;

    default:
        vaStatus = VA_STATUS_ERROR_INVALID_PARAMETER;
        break;
    }

out:
    return vaStatus;
}

/*
 * Return the next slice paramter
 *
 * Input:
 *      slice_param: the current slice
 *      *group_idx & *element_idx the current slice position in slice groups
 * Output:
 *      Return the next slice parameter
 *      *group_idx & *element_idx the next slice position in slice groups,
 *      if the next slice is NULL, *group_idx & *element_idx will be ignored
 */
VASliceParameterBufferMPEG2 *
intel_mpeg2_find_next_slice(struct decode_state *decode_state,
                            VAPictureParameterBufferMPEG2 *pic_param,
                            VASliceParameterBufferMPEG2 *slice_param,
                            int *group_idx,
                            int *element_idx)
{
    VASliceParameterBufferMPEG2 *next_slice_param;
    unsigned int width_in_mbs = ALIGN(pic_param->horizontal_size, 16) / 16;
    int j = *group_idx, i = *element_idx + 1;

    for (; j < decode_state->num_slice_params; j++) {
        for (; i < decode_state->slice_params[j]->num_elements; i++) {
            next_slice_param = ((VASliceParameterBufferMPEG2 *)decode_state->slice_params[j]->buffer) + i;

            if ((next_slice_param->slice_vertical_position * width_in_mbs + next_slice_param->slice_horizontal_position) >=
                (slice_param->slice_vertical_position * width_in_mbs + slice_param->slice_horizontal_position)) {
                *group_idx = j;
                *element_idx = i;

                return next_slice_param;
            }
        }

        i = 0;
    }

    return NULL;
}

/* Ensure the segmentation buffer is large enough for the supplied
   number of MBs, or re-allocate it */
bool
intel_ensure_vp8_segmentation_buffer(VADriverContextP ctx, GenBuffer *buf,
    unsigned int mb_width, unsigned int mb_height)
{
    struct i965_driver_data * const i965 = i965_driver_data(ctx);
    /* The segmentation map is a 64-byte aligned linear buffer, with
       each cache line holding only 8 bits for 4 continuous MBs */
    const unsigned int buf_size = ((mb_width + 3) / 4) * 64 * mb_height;

    if (buf->valid) {
        if (buf->bo && buf->bo->size >= buf_size)
            return true;
        drm_intel_bo_unreference(buf->bo);
        buf->valid = false;
    }

    buf->bo = drm_intel_bo_alloc(i965->intel.bufmgr, "segmentation map",
        buf_size, 0x1000);
    buf->valid = buf->bo != NULL;
    return buf->valid;
}

void
hevc_gen_default_iq_matrix(VAIQMatrixBufferHEVC *iq_matrix)
{
    /* Flat_4x4_16 */
    memset(&iq_matrix->ScalingList4x4, 16, sizeof(iq_matrix->ScalingList4x4));

    /* Flat_8x8_16 */
    memset(&iq_matrix->ScalingList8x8, 16, sizeof(iq_matrix->ScalingList8x8));

    /* Flat_16x16_16 */
    memset(&iq_matrix->ScalingList16x16, 16, sizeof(iq_matrix->ScalingList16x16));

    /* Flat_32x32_16 */
    memset(&iq_matrix->ScalingList32x32, 16, sizeof(iq_matrix->ScalingList32x32));

    /* Flat_16x16_dc_16 */
    memset(&iq_matrix->ScalingListDC16x16, 16, sizeof(iq_matrix->ScalingListDC16x16));

    /* Flat_32x32_dc_16 */
    memset(&iq_matrix->ScalingListDC32x32, 16, sizeof(iq_matrix->ScalingListDC32x32));
}
