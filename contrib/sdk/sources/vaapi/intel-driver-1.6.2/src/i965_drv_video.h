/*
 * Copyright ?2009 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Xiang Haihao <haihao.xiang@intel.com>
 *    Zou Nan hai <nanhai.zou@intel.com>
 *
 */

#ifndef _I965_DRV_VIDEO_H_
#define _I965_DRV_VIDEO_H_

#include <va/va.h>
#include <va/va_enc_h264.h>
#include <va/va_enc_mpeg2.h>
#include <va/va_enc_hevc.h>
#include <va/va_enc_jpeg.h>
#include <va/va_enc_vp8.h>
#include <va/va_vpp.h>
#include <va/va_backend.h>
#include <va/va_backend_vpp.h>

#include "i965_mutext.h"
#include "object_heap.h"
#include "intel_driver.h"
#include "i965_fourcc.h"

#define I965_MAX_PROFILES                       20
#define I965_MAX_ENTRYPOINTS                    5
#define I965_MAX_CONFIG_ATTRIBUTES              10
#define I965_MAX_IMAGE_FORMATS                  10
#define I965_MAX_SUBPIC_FORMATS                 6
#define I965_MAX_SUBPIC_SUM                     4
#define I965_MAX_SURFACE_ATTRIBUTES             16

#define INTEL_STR_DRIVER_VENDOR                 "Intel"
#define INTEL_STR_DRIVER_NAME                   "i965"

#define I965_SURFACE_TYPE_IMAGE                 0
#define I965_SURFACE_TYPE_SURFACE               1

#define I965_SURFACE_FLAG_FRAME                 0x00000000
#define I965_SURFACE_FLAG_TOP_FIELD_FIRST       0x00000001
#define I965_SURFACE_FLAG_BOTTOME_FIELD_FIRST   0x00000002

#define DEFAULT_BRIGHTNESS      0
#define DEFAULT_CONTRAST        50
#define DEFAULT_HUE             0
#define DEFAULT_SATURATION      50

#define ENCODER_QUALITY_RANGE     2
#define ENCODER_DEFAULT_QUALITY   1
#define ENCODER_HIGH_QUALITY      ENCODER_DEFAULT_QUALITY
#define ENCODER_LOW_QUALITY       2

struct i965_surface
{
    struct object_base *base;
    int type;
    int flags;
};

struct i965_kernel 
{
    char *name;
    int interface;
    const uint32_t (*bin)[4];
    int size;
    dri_bo *bo;
    unsigned int kernel_offset;
};

struct buffer_store
{
    unsigned char *buffer;
    dri_bo *bo;
    int ref_count;
    int num_elements;
};
    
struct object_config 
{
    struct object_base base;
    VAProfile profile;
    VAEntrypoint entrypoint;
    VAConfigAttrib attrib_list[I965_MAX_CONFIG_ATTRIBUTES];
    int num_attribs;

    VAGenericID wrapper_config;
};

#define NUM_SLICES     10

struct codec_state_base {
    uint32_t chroma_formats;
};

struct decode_state
{
    struct codec_state_base base;
    struct buffer_store *pic_param;
    struct buffer_store **slice_params;
    struct buffer_store *iq_matrix;
    struct buffer_store *bit_plane;
    struct buffer_store *huffman_table;
    struct buffer_store **slice_datas;
    struct buffer_store *probability_data;
    VASurfaceID current_render_target;
    int max_slice_params;
    int max_slice_datas;
    int num_slice_params;
    int num_slice_datas;

    struct object_surface *render_object;
    struct object_surface *reference_objects[16]; /* Up to 2 reference surfaces are valid for MPEG-2,*/
};

#define SLICE_PACKED_DATA_INDEX_TYPE    0x80000000
#define SLICE_PACKED_DATA_INDEX_MASK    0x00FFFFFF

struct encode_state
{
    struct codec_state_base base;
    struct buffer_store *seq_param;
    struct buffer_store *pic_param;
    struct buffer_store *pic_control;
    struct buffer_store *iq_matrix;
    struct buffer_store *q_matrix;
    struct buffer_store **slice_params;
    struct buffer_store *huffman_table;
    int max_slice_params;
    int num_slice_params;

    /* for ext */
    struct buffer_store *seq_param_ext;
    struct buffer_store *pic_param_ext;
    struct buffer_store *packed_header_param[5];
    struct buffer_store *packed_header_data[5];
    struct buffer_store **slice_params_ext;
    int max_slice_params_ext;
    int num_slice_params_ext;

    /* Check the user-configurable packed_header attribute.
     * Currently it is mainly used to check whether the packed slice_header data
     * is provided by user or the driver.
     * TBD: It will check for the packed SPS/PPS/MISC/RAWDATA and so on.
     */
    unsigned int packed_header_flag;
    /* For the packed data that needs to be inserted into video clip */
    /* currently it is mainly to track packed raw data and packed slice_header data. */
    struct buffer_store **packed_header_params_ext;
    int max_packed_header_params_ext;
    int num_packed_header_params_ext;
    struct buffer_store **packed_header_data_ext;
    int max_packed_header_data_ext;
    int num_packed_header_data_ext;

    /* the index of current vps and sps ,special for HEVC*/
    int vps_sps_seq_index;
    /* the index of current slice */
    int slice_index;
    /* the array is determined by max_slice_params_ext */
    int max_slice_num;
    /* This is to store the first index of packed data for one slice */
    int *slice_rawdata_index;
    /* This is to store the number of packed data for one slice.
     * Both packed rawdata and slice_header data are tracked by this
     * this variable. That is to say: When one packed slice_header is parsed,
     * this variable will also be increased.
     */
    int *slice_rawdata_count;

    /* This is to store the index of packed slice header for one slice */
    int *slice_header_index;

    int last_packed_header_type;

    struct buffer_store *misc_param[16];

    VASurfaceID current_render_target;
    struct object_surface *input_yuv_object;
    struct object_surface *reconstructed_object;
    struct object_buffer *coded_buf_object;
    struct object_surface *reference_objects[16]; /* Up to 2 reference surfaces are valid for MPEG-2,*/
};

struct proc_state
{
    struct codec_state_base base;
    struct buffer_store *pipeline_param;

    VASurfaceID current_render_target;
};

#define CODEC_DEC       0
#define CODEC_ENC       1
#define CODEC_PROC      2

union codec_state
{
    struct codec_state_base base;
    struct decode_state decode;
    struct encode_state encode;
    struct proc_state proc;
};

struct hw_context
{
    VAStatus (*run)(VADriverContextP ctx, 
                    VAProfile profile, 
                    union codec_state *codec_state,
                    struct hw_context *hw_context);
    void (*destroy)(void *);
    struct intel_batchbuffer *batch;
};

struct object_context 
{
    struct object_base base;
    VAContextID context_id;
    struct object_config *obj_config;
    VASurfaceID *render_targets;		//input->encode, output->decode
    int num_render_targets;
    int picture_width;
    int picture_height;
    int flags;
    int codec_type;
    union codec_state codec_state;
    struct hw_context *hw_context;

    VAGenericID       wrapper_context;
};

#define SURFACE_REFERENCED      (1 << 0)
#define SURFACE_DERIVED         (1 << 2)
#define SURFACE_ALL_MASK        ((SURFACE_REFERENCED) | \
                                 (SURFACE_DERIVED))

struct object_surface 
{
    struct object_base base;
    VASurfaceStatus status;
    VASubpictureID subpic[I965_MAX_SUBPIC_SUM];
    struct object_subpic *obj_subpic[I965_MAX_SUBPIC_SUM];
    unsigned int subpic_render_idx;

    int width;          /* the pitch of plane 0 in bytes in horizontal direction */
    int height;         /* the pitch of plane 0 in bytes in vertical direction */
    int size;
    int orig_width;     /* the width of plane 0 in pixels */
    int orig_height;    /* the height of plane 0 in pixels */
    int flags;
    unsigned int fourcc;    
    dri_bo *bo;
    unsigned int expected_format;
    VAImageID locked_image_id;
    VAImageID derived_image_id;
    void (*free_private_data)(void **data);
    void *private_data;
    unsigned int subsampling;
    int x_cb_offset;
    int y_cb_offset;
    int x_cr_offset;
    int y_cr_offset;
    int cb_cr_width;
    int cb_cr_height;
    int cb_cr_pitch;
    /* user specified attributes see: VASurfaceAttribExternalBuffers/VA_SURFACE_ATTRIB_MEM_TYPE_VA */
    uint32_t user_disable_tiling : 1;
    uint32_t user_h_stride_set   : 1;
    uint32_t user_v_stride_set   : 1;

    VAGenericID wrapper_surface;

    int exported_primefd;
};

struct object_buffer 
{
    struct object_base base;
    struct buffer_store *buffer_store;
    int max_num_elements;
    int num_elements;
    int size_element;
    VABufferType type;

    /* Export state */
    unsigned int export_refcount;
    VABufferInfo export_state;

    VAGenericID wrapper_buffer;
};

struct object_image 
{
    struct object_base base;
    VAImage image;
    dri_bo *bo;
    unsigned int *palette;
    VASurfaceID derived_surface;
};

struct object_subpic 
{
    struct object_base base;
    VAImageID image;
    struct object_image *obj_image;
    VARectangle src_rect;
    VARectangle dst_rect;
    unsigned int format;
    int width;
    int height;
    int pitch;
    float global_alpha;
    dri_bo *bo;
    unsigned int flags;
};

#define I965_RING_NULL  0
#define I965_RING_BSD   1
#define I965_RING_BLT   2
#define I965_RING_VEBOX 3

struct i965_filter
{
    VAProcFilterType type;
    int ring;
};

struct hw_codec_info
{
    struct hw_context *(*dec_hw_context_init)(VADriverContextP, struct object_config *);
    struct hw_context *(*enc_hw_context_init)(VADriverContextP, struct object_config *);
    struct hw_context *(*proc_hw_context_init)(VADriverContextP, struct object_config *);
    bool (*render_init)(VADriverContextP);
    void (*post_processing_context_init)(VADriverContextP, void *, struct intel_batchbuffer *);
    void (*preinit_hw_codec)(VADriverContextP, struct hw_codec_info *);

    int max_width;
    int max_height;
    int min_linear_wpitch;
    int min_linear_hpitch;

    unsigned int h264_mvc_dec_profiles;
    unsigned int h264_dec_chroma_formats;
    unsigned int jpeg_dec_chroma_formats;
    unsigned int jpeg_enc_chroma_formats;
    unsigned int hevc_dec_chroma_formats;

    unsigned int has_mpeg2_decoding:1;
    unsigned int has_mpeg2_encoding:1;
    unsigned int has_h264_decoding:1;
    unsigned int has_h264_encoding:1;
    unsigned int has_vc1_decoding:1;
    unsigned int has_vc1_encoding:1;
    unsigned int has_jpeg_decoding:1;
    unsigned int has_jpeg_encoding:1;
    unsigned int has_vpp:1;
    unsigned int has_accelerated_getimage:1;
    unsigned int has_accelerated_putimage:1;
    unsigned int has_tiled_surface:1;
    unsigned int has_di_motion_adptive:1;
    unsigned int has_di_motion_compensated:1;
    unsigned int has_vp8_decoding:1;
    unsigned int has_vp8_encoding:1;
    unsigned int has_h264_mvc_encoding:1;
    unsigned int has_hevc_decoding:1;
    unsigned int has_hevc_encoding:1;
    unsigned int has_hevc10_decoding:1;
    unsigned int has_vp9_decoding:1;

    unsigned int num_filters;
    struct i965_filter filters[VAProcFilterCount];
};


#include "i965_render.h"

struct i965_driver_data 
{
    struct intel_driver_data intel;
    struct object_heap config_heap;
    struct object_heap context_heap;
    struct object_heap surface_heap;
    struct object_heap buffer_heap;
    struct object_heap image_heap;
    struct object_heap subpic_heap;
    struct hw_codec_info *codec_info;

    _I965Mutex render_mutex;
    _I965Mutex pp_mutex;
    struct intel_batchbuffer *batch;
    struct intel_batchbuffer *pp_batch;
    struct i965_render_state render_state;
    void *pp_context;
    char va_vendor[256];
 
    VADisplayAttribute *display_attributes;
    unsigned int num_display_attributes;
    VADisplayAttribute *rotation_attrib;
    VADisplayAttribute *brightness_attrib;
    VADisplayAttribute *contrast_attrib;
    VADisplayAttribute *hue_attrib;
    VADisplayAttribute *saturation_attrib;
    VAContextID current_context_id;

    /* VA/DRI (X11) specific data */
    struct va_dri_output *dri_output;

    /* VA/Wayland specific data */
    struct va_wl_output *wl_output;

    VADriverContextP wrapper_pdrvctx;
};

#define NEW_CONFIG_ID() object_heap_allocate(&i965->config_heap);
#define NEW_CONTEXT_ID() object_heap_allocate(&i965->context_heap);
#define NEW_SURFACE_ID() object_heap_allocate(&i965->surface_heap);
#define NEW_BUFFER_ID() object_heap_allocate(&i965->buffer_heap);
#define NEW_IMAGE_ID() object_heap_allocate(&i965->image_heap);
#define NEW_SUBPIC_ID() object_heap_allocate(&i965->subpic_heap);

#define CONFIG(id) ((struct object_config *)object_heap_lookup(&i965->config_heap, id))
#define CONTEXT(id) ((struct object_context *)object_heap_lookup(&i965->context_heap, id))
#define SURFACE(id) ((struct object_surface *)object_heap_lookup(&i965->surface_heap, id))
#define BUFFER(id) ((struct object_buffer *)object_heap_lookup(&i965->buffer_heap, id))
#define IMAGE(id) ((struct object_image *)object_heap_lookup(&i965->image_heap, id))
#define SUBPIC(id) ((struct object_subpic *)object_heap_lookup(&i965->subpic_heap, id))

#define FOURCC_IA44 0x34344149
#define FOURCC_AI44 0x34344941

#define STRIDE(w)               (((w) + 0xf) & ~0xf)
#define SIZE_YUV420(w, h)       (h * (STRIDE(w) + STRIDE(w >> 1)))

static INLINE struct i965_driver_data *
i965_driver_data(VADriverContextP ctx)
{
    return (struct i965_driver_data *)(ctx->pDriverData);
}

VAStatus
i965_check_alloc_surface_bo(VADriverContextP ctx,
                            struct object_surface *obj_surface,
                            int tiled,
                            unsigned int fourcc,
                            unsigned int subsampling);

int
va_enc_packed_type_to_idx(int packed_type);

/* reserve 2 byte for internal using */
#define CODEC_H264      0
#define CODEC_MPEG2     1
#define CODEC_H264_MVC  2
#define CODEC_JPEG      3
#define CODEC_VP8       4
#define CODEC_HEVC      5

#define H264_DELIMITER0 0x00
#define H264_DELIMITER1 0x00
#define H264_DELIMITER2 0x00
#define H264_DELIMITER3 0x00
#define H264_DELIMITER4 0x00

#define MPEG2_DELIMITER0        0x00
#define MPEG2_DELIMITER1        0x00
#define MPEG2_DELIMITER2        0x00
#define MPEG2_DELIMITER3        0x00
#define MPEG2_DELIMITER4        0xb0

#define HEVC_DELIMITER0 0x00
#define HEVC_DELIMITER1 0x00
#define HEVC_DELIMITER2 0x00
#define HEVC_DELIMITER3 0x00
#define HEVC_DELIMITER4 0x00

struct i965_coded_buffer_segment
{
    VACodedBufferSegment base;
    unsigned char mapped;
    unsigned char codec;
};

#define I965_CODEDBUFFER_HEADER_SIZE   ALIGN(sizeof(struct i965_coded_buffer_segment), 0x1000)

extern VAStatus i965_MapBuffer(VADriverContextP ctx,
		VABufferID buf_id,       /* in */
		void **pbuf);            /* out */

extern VAStatus i965_UnmapBuffer(VADriverContextP ctx, VABufferID buf_id);

extern VAStatus i965_DestroySurfaces(VADriverContextP ctx,
                     VASurfaceID *surface_list,
                     int num_surfaces);

extern VAStatus i965_CreateSurfaces(VADriverContextP ctx,
                    int width,
                    int height,
                    int format,
                    int num_surfaces,
                    VASurfaceID *surfaces);

#define I965_SURFACE_MEM_NATIVE             0
#define I965_SURFACE_MEM_GEM_FLINK          1
#define I965_SURFACE_MEM_DRM_PRIME          2

void
i965_destroy_surface_storage(struct object_surface *obj_surface);

#endif /* _I965_DRV_VIDEO_H_ */
