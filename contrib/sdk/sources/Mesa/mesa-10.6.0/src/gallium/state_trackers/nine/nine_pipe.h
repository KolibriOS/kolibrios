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

#ifndef _NINE_PIPE_H_
#define _NINE_PIPE_H_

#include "d3d9.h"
#include "pipe/p_format.h"
#include "pipe/p_screen.h"
#include "pipe/p_state.h" /* pipe_box */
#include "util/u_rect.h"
#include "util/u_format.h"
#include "nine_helpers.h"

struct cso_context;

extern const enum pipe_format nine_d3d9_to_pipe_format_map[120];
extern const D3DFORMAT nine_pipe_to_d3d9_format_map[PIPE_FORMAT_COUNT];

void nine_convert_dsa_state(struct cso_context *, const DWORD *);
void nine_convert_rasterizer_state(struct cso_context *, const DWORD *);
void nine_convert_blend_state(struct cso_context *, const DWORD *);
void nine_convert_sampler_state(struct cso_context *, int idx, const DWORD *);

void nine_pipe_context_clear(struct NineDevice9 *);

static INLINE unsigned d3dlock_buffer_to_pipe_transfer_usage(DWORD Flags)
{
    unsigned usage;

    if (Flags & D3DLOCK_DISCARD)
        usage = PIPE_TRANSFER_WRITE | PIPE_TRANSFER_DISCARD_WHOLE_RESOURCE;
    else
    if (Flags & D3DLOCK_READONLY)
        usage = PIPE_TRANSFER_READ;
    else
        usage = PIPE_TRANSFER_READ_WRITE;

    if (Flags & D3DLOCK_NOOVERWRITE)
        usage = (PIPE_TRANSFER_UNSYNCHRONIZED |
                 PIPE_TRANSFER_DISCARD_RANGE | usage) & ~PIPE_TRANSFER_READ;
    else
    if (Flags & D3DLOCK_DONOTWAIT)
        usage |= PIPE_TRANSFER_DONTBLOCK;

    /*
    if (Flags & D3DLOCK_NO_DIRTY_UPDATE)
        usage |= PIPE_TRANSFER_FLUSH_EXPLICIT;
    */

    return usage;
}

static INLINE void
rect_to_pipe_box(struct pipe_box *dst, const RECT *src)
{
    dst->x = src->left;
    dst->y = src->top;
    dst->z = 0;
    dst->width = src->right - src->left;
    dst->height = src->bottom - src->top;
    dst->depth = 1;
}

static INLINE boolean
rect_to_pipe_box_clamp(struct pipe_box *dst, const RECT *src)
{
    rect_to_pipe_box(dst, src);

    if (dst->width <= 0 || dst->height <= 0) {
        DBG_FLAG(DBG_UNKNOWN, "Warning: NULL box");
        dst->width = MAX2(dst->width, 0);
        dst->height = MAX2(dst->height, 0);
        return TRUE;
    }
    return FALSE;
}

static INLINE boolean
rect_to_pipe_box_flip(struct pipe_box *dst, const RECT *src)
{
    rect_to_pipe_box(dst, src);

    if (dst->width >= 0 && dst->height >= 0)
        return FALSE;
    if (dst->width < 0) dst->width = -dst->width;
    if (dst->height < 0) dst->height = -dst->height;
    return TRUE;
}

static INLINE void
rect_to_pipe_box_xy_only(struct pipe_box *dst, const RECT *src)
{
    user_warn(src->left > src->right || src->top > src->bottom);

    dst->x = src->left;
    dst->y = src->top;
    dst->width = src->right - src->left;
    dst->height = src->bottom - src->top;
}

static INLINE boolean
rect_to_pipe_box_xy_only_clamp(struct pipe_box *dst, const RECT *src)
{
    rect_to_pipe_box_xy_only(dst, src);

    if (dst->width <= 0 || dst->height <= 0) {
        DBG_FLAG(DBG_UNKNOWN, "Warning: NULL box");
        dst->width = MAX2(dst->width, 0);
        dst->height = MAX2(dst->height, 0);
        return TRUE;
    }
    return FALSE;
}

static INLINE void
rect_to_g3d_u_rect(struct u_rect *dst, const RECT *src)
{
    user_warn(src->left > src->right || src->top > src->bottom);

    dst->x0 = src->left;
    dst->x1 = src->right;
    dst->y0 = src->top;
    dst->y1 = src->bottom;
}

static INLINE void
d3dbox_to_pipe_box(struct pipe_box *dst, const D3DBOX *src)
{
    user_warn(src->Left > src->Right);
    user_warn(src->Top > src->Bottom);
    user_warn(src->Front > src->Back);

    dst->x = src->Left;
    dst->y = src->Top;
    dst->z = src->Front;
    dst->width = src->Right - src->Left;
    dst->height = src->Bottom - src->Top;
    dst->depth = src->Back - src->Front;
}

static INLINE D3DFORMAT
pipe_to_d3d9_format(enum pipe_format format)
{
    return nine_pipe_to_d3d9_format_map[format];
}

static INLINE boolean
depth_stencil_format( D3DFORMAT fmt )
{
    static D3DFORMAT allowed[] = {
        D3DFMT_D16_LOCKABLE,
        D3DFMT_D32,
        D3DFMT_D15S1,
        D3DFMT_D24S8,
        D3DFMT_D24X8,
        D3DFMT_D24X4S4,
        D3DFMT_D16,
        D3DFMT_D32F_LOCKABLE,
        D3DFMT_D24FS8,
        D3DFMT_D32_LOCKABLE,
        D3DFMT_DF16,
        D3DFMT_DF24,
        D3DFMT_INTZ
    };
    unsigned i;

    for (i = 0; i < sizeof(allowed)/sizeof(D3DFORMAT); i++) {
        if (fmt == allowed[i]) { return TRUE; }
    }
    return FALSE;
}

static INLINE unsigned
d3d9_get_pipe_depth_format_bindings(D3DFORMAT format)
{
    switch (format) {
    case D3DFMT_D32:
    case D3DFMT_D15S1:
    case D3DFMT_D24S8:
    case D3DFMT_D24X8:
    case D3DFMT_D24X4S4:
    case D3DFMT_D16:
    case D3DFMT_D24FS8:
        return PIPE_BIND_DEPTH_STENCIL;
    case D3DFMT_D32F_LOCKABLE:
    case D3DFMT_D16_LOCKABLE:
    case D3DFMT_D32_LOCKABLE:
        return PIPE_BIND_DEPTH_STENCIL | PIPE_BIND_TRANSFER_READ |
               PIPE_BIND_TRANSFER_WRITE;
    case D3DFMT_DF16:
    case D3DFMT_DF24:
    case D3DFMT_INTZ:
        return PIPE_BIND_DEPTH_STENCIL | PIPE_BIND_SAMPLER_VIEW;
    default: unreachable("Unexpected format");
    }
}

static INLINE enum pipe_format
d3d9_to_pipe_format_internal(D3DFORMAT format)
{
    if (format <= D3DFMT_A2B10G10R10_XR_BIAS)
        return nine_d3d9_to_pipe_format_map[format];
    switch (format) {
    case D3DFMT_INTZ: return PIPE_FORMAT_S8_UINT_Z24_UNORM;
    case D3DFMT_DF16: return PIPE_FORMAT_Z16_UNORM;
    case D3DFMT_DXT1: return PIPE_FORMAT_DXT1_RGBA;
    case D3DFMT_DXT2: return PIPE_FORMAT_DXT3_RGBA; /* XXX */
    case D3DFMT_DXT3: return PIPE_FORMAT_DXT3_RGBA;
    case D3DFMT_DXT4: return PIPE_FORMAT_DXT5_RGBA; /* XXX */
    case D3DFMT_DXT5: return PIPE_FORMAT_DXT5_RGBA;
    case D3DFMT_ATI1: return PIPE_FORMAT_RGTC1_UNORM;
    case D3DFMT_ATI2: return PIPE_FORMAT_RGTC2_UNORM;
    case D3DFMT_UYVY: return PIPE_FORMAT_UYVY;
    case D3DFMT_YUY2: return PIPE_FORMAT_YUYV; /* XXX check */
    case D3DFMT_NV12: return PIPE_FORMAT_NV12;
    case D3DFMT_G8R8_G8B8: return PIPE_FORMAT_G8R8_G8B8_UNORM; /* XXX order ? */
    case D3DFMT_R8G8_B8G8: return PIPE_FORMAT_R8G8_B8G8_UNORM; /* XXX order ? */
    case D3DFMT_BINARYBUFFER: return PIPE_FORMAT_NONE; /* not a format */
    case D3DFMT_MULTI2_ARGB8: return PIPE_FORMAT_NONE; /* not supported */
    case D3DFMT_Y210: /* XXX */
    case D3DFMT_Y216:
    case D3DFMT_NV11:
    case D3DFMT_DF24: /* Similar to D3DFMT_DF16 but for 24-bits.
        We don't advertise it because when it is supported, Fetch-4 is
        supposed to be supported, which we don't support yet. */
    case D3DFMT_NULL: /* special cased, only for surfaces */
        return PIPE_FORMAT_NONE;
    default:
        DBG_FLAG(DBG_UNKNOWN, "unknown D3DFORMAT: 0x%x/%c%c%c%c\n",
                 format, (char)format, (char)(format >> 8),
                 (char)(format >> 16), (char)(format >> 24));
        return PIPE_FORMAT_NONE;
    }
}

#define format_check_internal(pipe_format) \
    screen->is_format_supported(screen, pipe_format, target, \
                                sample_count, bindings)

static INLINE enum pipe_format
d3d9_to_pipe_format_checked(struct pipe_screen *screen,
                            D3DFORMAT format,
                            enum pipe_texture_target target,
                            unsigned sample_count,
                            unsigned bindings,
                            boolean srgb)
{
    enum pipe_format result;

    result = d3d9_to_pipe_format_internal(format);
    if (result == PIPE_FORMAT_NONE)
        return PIPE_FORMAT_NONE;

    if (srgb)
        result = util_format_srgb(result);

    if (format_check_internal(result))
        return result;

    /* fallback to another format for formats
     * that match several pipe_format */
    switch(format) {
        /* depth buffer formats are not lockable (except those for which it
         * is precised in the name), so it is ok to match to another similar
         * format. In all cases, if the app reads the texture with a shader,
         * it gets depth on r and doesn't get stencil.*/
        case D3DFMT_INTZ:
        case D3DFMT_D24S8:
            if (format_check_internal(PIPE_FORMAT_Z24_UNORM_S8_UINT))
                return PIPE_FORMAT_Z24_UNORM_S8_UINT;
            break;
        case D3DFMT_D24X8:
            if (format_check_internal(PIPE_FORMAT_Z24X8_UNORM))
                return PIPE_FORMAT_Z24X8_UNORM;
        default:
            break;
    }
    return PIPE_FORMAT_NONE;
}

static INLINE const char *
d3dformat_to_string(D3DFORMAT fmt)
{
    switch (fmt) {
    case D3DFMT_UNKNOWN: return "D3DFMT_UNKNOWN";
    case D3DFMT_R8G8B8: return "D3DFMT_R8G8B8";
    case D3DFMT_A8R8G8B8: return "D3DFMT_A8R8G8B8";
    case D3DFMT_X8R8G8B8: return "D3DFMT_X8R8G8B8";
    case D3DFMT_R5G6B5: return "D3DFMT_R5G6B5";
    case D3DFMT_X1R5G5B5: return "D3DFMT_X1R5G5B5";
    case D3DFMT_A1R5G5B5: return "D3DFMT_A1R5G5B5";
    case D3DFMT_A4R4G4B4: return "D3DFMT_A4R4G4B4";
    case D3DFMT_R3G3B2: return "D3DFMT_R3G3B2";
    case D3DFMT_A8: return "D3DFMT_A8";
    case D3DFMT_A8R3G3B2: return "D3DFMT_A8R3G3B2";
    case D3DFMT_X4R4G4B4: return "D3DFMT_X4R4G4B4";
    case D3DFMT_A2B10G10R10: return "D3DFMT_A2B10G10R10";
    case D3DFMT_A8B8G8R8: return "D3DFMT_A8B8G8R8";
    case D3DFMT_X8B8G8R8: return "D3DFMT_X8B8G8R8";
    case D3DFMT_G16R16: return "D3DFMT_G16R16";
    case D3DFMT_A2R10G10B10: return "D3DFMT_A2R10G10B10";
    case D3DFMT_A16B16G16R16: return "D3DFMT_A16B16G16R16";
    case D3DFMT_A8P8: return "D3DFMT_A8P8";
    case D3DFMT_P8: return "D3DFMT_P8";
    case D3DFMT_L8: return "D3DFMT_L8";
    case D3DFMT_A8L8: return "D3DFMT_A8L8";
    case D3DFMT_A4L4: return "D3DFMT_A4L4";
    case D3DFMT_V8U8: return "D3DFMT_V8U8";
    case D3DFMT_L6V5U5: return "D3DFMT_L6V5U5";
    case D3DFMT_X8L8V8U8: return "D3DFMT_X8L8V8U8";
    case D3DFMT_Q8W8V8U8: return "D3DFMT_Q8W8V8U8";
    case D3DFMT_V16U16: return "D3DFMT_V16U16";
    case D3DFMT_A2W10V10U10: return "D3DFMT_A2W10V10U10";
    case D3DFMT_UYVY: return "D3DFMT_UYVY";
    case D3DFMT_R8G8_B8G8: return "D3DFMT_R8G8_B8G8";
    case D3DFMT_YUY2: return "D3DFMT_YUY2";
    case D3DFMT_G8R8_G8B8: return "D3DFMT_G8R8_G8B8";
    case D3DFMT_DXT1: return "D3DFMT_DXT1";
    case D3DFMT_DXT2: return "D3DFMT_DXT2";
    case D3DFMT_DXT3: return "D3DFMT_DXT3";
    case D3DFMT_DXT4: return "D3DFMT_DXT4";
    case D3DFMT_DXT5: return "D3DFMT_DXT5";
    case D3DFMT_ATI1: return "D3DFMT_ATI1";
    case D3DFMT_ATI2: return "D3DFMT_ATI2";
    case D3DFMT_D16_LOCKABLE: return "D3DFMT_D16_LOCKABLE";
    case D3DFMT_D32: return "D3DFMT_D32";
    case D3DFMT_D15S1: return "D3DFMT_D15S1";
    case D3DFMT_D24S8: return "D3DFMT_D24S8";
    case D3DFMT_D24X8: return "D3DFMT_D24X8";
    case D3DFMT_D24X4S4: return "D3DFMT_D24X4S4";
    case D3DFMT_D16: return "D3DFMT_D16";
    case D3DFMT_D32F_LOCKABLE: return "D3DFMT_D32F_LOCKABLE";
    case D3DFMT_D24FS8: return "D3DFMT_D24FS8";
    case D3DFMT_D32_LOCKABLE: return "D3DFMT_D32_LOCKABLE";
    case D3DFMT_S8_LOCKABLE: return "D3DFMT_S8_LOCKABLE";
    case D3DFMT_L16: return "D3DFMT_L16";
    case D3DFMT_VERTEXDATA: return "D3DFMT_VERTEXDATA";
    case D3DFMT_INDEX16: return "D3DFMT_INDEX16";
    case D3DFMT_INDEX32: return "D3DFMT_INDEX32";
    case D3DFMT_Q16W16V16U16: return "D3DFMT_Q16W16V16U16";
    case D3DFMT_MULTI2_ARGB8: return "D3DFMT_MULTI2_ARGB8";
    case D3DFMT_R16F: return "D3DFMT_R16F";
    case D3DFMT_G16R16F: return "D3DFMT_G16R16F";
    case D3DFMT_A16B16G16R16F: return "D3DFMT_A16B16G16R16F";
    case D3DFMT_R32F: return "D3DFMT_R32F";
    case D3DFMT_G32R32F: return "D3DFMT_G32R32F";
    case D3DFMT_A32B32G32R32F: return "D3DFMT_A32B32G32R32F";
    case D3DFMT_CxV8U8: return "D3DFMT_CxV8U8";
    case D3DFMT_A1: return "D3DFMT_A1";
    case D3DFMT_A2B10G10R10_XR_BIAS: return "D3DFMT_A2B10G10R10_XR_BIAS";
    case D3DFMT_BINARYBUFFER: return "D3DFMT_BINARYBUFFER";
    case D3DFMT_DF16: return "D3DFMT_DF16";
    case D3DFMT_DF24: return "D3DFMT_DF24";
    case D3DFMT_INTZ: return "D3DFMT_INTZ";
    case D3DFMT_NVDB: return "D3DFMT_NVDB";
    case D3DFMT_RESZ: return "D3DFMT_RESZ";
    case D3DFMT_NULL: return "D3DFMT_NULL";
    default:
        break;
    }
    return "Unknown";
}

static INLINE unsigned
nine_fvf_stride( DWORD fvf )
{
    unsigned texcount, i, size = 0;

    switch (fvf & D3DFVF_POSITION_MASK) {
    case D3DFVF_XYZ:    size += 3*4; break;
    case D3DFVF_XYZRHW: size += 4*4; break;
    case D3DFVF_XYZB1:  size += 4*4; break;
    case D3DFVF_XYZB2:  size += 5*4; break;
    case D3DFVF_XYZB3:  size += 6*4; break;
    case D3DFVF_XYZB4:  size += 7*4; break;
    case D3DFVF_XYZB5:  size += 8*4; break;
    case D3DFVF_XYZW:   size += 4*4; break;
    default:
        user_warn("Position doesn't match any known combination.");
        break;
    }

    if (fvf & D3DFVF_NORMAL)   { size += 3*4; }
    if (fvf & D3DFVF_PSIZE)    { size += 1*4; }
    if (fvf & D3DFVF_DIFFUSE)  { size += 1*4; }
    if (fvf & D3DFVF_SPECULAR) { size += 1*4; }

    texcount = (fvf >> D3DFVF_TEXCOUNT_SHIFT) & D3DFVF_TEXCOUNT_MASK;
    if (user_error(texcount <= 8))
        texcount = 8;

    for (i = 0; i < texcount; ++i) {
        unsigned texformat = (fvf>>(16+i*2))&0x3;
        /* texformats are defined having been shifted around so 1=3,2=0,3=1,4=2
         * meaning we can just do this instead of the switch below */
        size += (((texformat+1)&0x3)+1)*4;

        /*
        switch (texformat) {
        case D3DFVF_TEXTUREFORMAT1: size += 1*4;
        case D3DFVF_TEXTUREFORMAT2: size += 2*4;
        case D3DFVF_TEXTUREFORMAT3: size += 3*4;
        case D3DFVF_TEXTUREFORMAT4: size += 4*4;
        }
        */
    }

    return size;
}

static INLINE void
d3dcolor_to_rgba(float *rgba, D3DCOLOR color)
{
    rgba[0] = (float)((color >> 16) & 0xFF) / 0xFF;
    rgba[1] = (float)((color >>  8) & 0xFF) / 0xFF;
    rgba[2] = (float)((color >>  0) & 0xFF) / 0xFF;
    rgba[3] = (float)((color >> 24) & 0xFF) / 0xFF;
}

static INLINE void
d3dcolor_to_pipe_color_union(union pipe_color_union *rgba, D3DCOLOR color)
{
    d3dcolor_to_rgba(&rgba->f[0], color);
}

static INLINE unsigned
d3dprimitivetype_to_pipe_prim(D3DPRIMITIVETYPE prim)
{
    switch (prim) {
    case D3DPT_POINTLIST:     return PIPE_PRIM_POINTS;
    case D3DPT_LINELIST:      return PIPE_PRIM_LINES;
    case D3DPT_LINESTRIP:     return PIPE_PRIM_LINE_STRIP;
    case D3DPT_TRIANGLELIST:  return PIPE_PRIM_TRIANGLES;
    case D3DPT_TRIANGLESTRIP: return PIPE_PRIM_TRIANGLE_STRIP;
    case D3DPT_TRIANGLEFAN:   return PIPE_PRIM_TRIANGLE_FAN;
    default:
        assert(0);
        return PIPE_PRIM_POINTS;
    }
}

static INLINE unsigned
prim_count_to_vertex_count(D3DPRIMITIVETYPE prim, UINT count)
{
    switch (prim) {
    case D3DPT_POINTLIST:     return count;
    case D3DPT_LINELIST:      return count * 2;
    case D3DPT_LINESTRIP:     return count + 1;
    case D3DPT_TRIANGLELIST:  return count * 3;
    case D3DPT_TRIANGLESTRIP: return count + 2;
    case D3DPT_TRIANGLEFAN:   return count + 2;
    default:
        assert(0);
        return 0;
    }
}

static INLINE unsigned
d3dcmpfunc_to_pipe_func(D3DCMPFUNC func)
{
    switch (func) {
    case D3DCMP_NEVER:        return PIPE_FUNC_NEVER;
    case D3DCMP_LESS:         return PIPE_FUNC_LESS;
    case D3DCMP_EQUAL:        return PIPE_FUNC_EQUAL;
    case D3DCMP_LESSEQUAL:    return PIPE_FUNC_LEQUAL;
    case D3DCMP_GREATER:      return PIPE_FUNC_GREATER;
    case D3DCMP_NOTEQUAL:     return PIPE_FUNC_NOTEQUAL;
    case D3DCMP_GREATEREQUAL: return PIPE_FUNC_GEQUAL;
    case D3DCMP_ALWAYS:       return PIPE_FUNC_ALWAYS;
    case D3DCMP_NEVER_ZERO:   return PIPE_FUNC_NEVER; // Tested on windows + ATI HD5770
    default:
        assert(0);
        return PIPE_FUNC_NEVER;
    }
}

static INLINE unsigned
d3dstencilop_to_pipe_stencil_op(D3DSTENCILOP op)
{
    switch (op) {
    case D3DSTENCILOP_KEEP:    return PIPE_STENCIL_OP_KEEP;
    case D3DSTENCILOP_ZERO:    return PIPE_STENCIL_OP_ZERO;
    case D3DSTENCILOP_REPLACE: return PIPE_STENCIL_OP_REPLACE;
    case D3DSTENCILOP_INCRSAT: return PIPE_STENCIL_OP_INCR;
    case D3DSTENCILOP_DECRSAT: return PIPE_STENCIL_OP_DECR;
    case D3DSTENCILOP_INVERT:  return PIPE_STENCIL_OP_INVERT;
    case D3DSTENCILOP_INCR:    return PIPE_STENCIL_OP_INCR_WRAP;
    case D3DSTENCILOP_DECR:    return PIPE_STENCIL_OP_DECR_WRAP;
    default:
        return PIPE_STENCIL_OP_ZERO;
    }
}

static INLINE unsigned
d3dcull_to_pipe_face(D3DCULL cull)
{
    switch (cull) {
    case D3DCULL_NONE: return PIPE_FACE_NONE;
    case D3DCULL_CW:   return PIPE_FACE_FRONT;
    case D3DCULL_CCW:  return PIPE_FACE_BACK;
    default:
        assert(0);
        return PIPE_FACE_NONE;
    }
}

static INLINE unsigned
d3dfillmode_to_pipe_polygon_mode(D3DFILLMODE mode)
{
    switch (mode) {
    case D3DFILL_POINT:     return PIPE_POLYGON_MODE_POINT;
    case D3DFILL_WIREFRAME: return PIPE_POLYGON_MODE_LINE;
    case D3DFILL_SOLID:     return PIPE_POLYGON_MODE_FILL;
    case D3DFILL_SOLID_ZERO:return PIPE_POLYGON_MODE_FILL;
    default:
        assert(0);
        return PIPE_POLYGON_MODE_FILL;
    }
}

static INLINE unsigned
d3dblendop_to_pipe_blend(D3DBLENDOP op)
{
    switch (op) {
    case D3DBLENDOP_ADD:         return PIPE_BLEND_ADD;
    case D3DBLENDOP_SUBTRACT:    return PIPE_BLEND_SUBTRACT;
    case D3DBLENDOP_REVSUBTRACT: return PIPE_BLEND_REVERSE_SUBTRACT;
    case D3DBLENDOP_MIN:         return PIPE_BLEND_MIN;
    case D3DBLENDOP_MAX:         return PIPE_BLEND_MAX;
    default:
        assert(0);
        return PIPE_BLEND_ADD;
    }
}

/* NOTE: The COLOR factors for are equal to the ALPHA ones for alpha.
 * Drivers may check RGB and ALPHA factors for equality so we should not
 * simply substitute the ALPHA variants.
 */
static INLINE unsigned
d3dblend_alpha_to_pipe_blendfactor(D3DBLEND b)
{
    switch (b) {
    case D3DBLEND_ZERO:            return PIPE_BLENDFACTOR_ZERO;
    case D3DBLEND_ONE:             return PIPE_BLENDFACTOR_ONE;
    case D3DBLEND_SRCCOLOR:        return PIPE_BLENDFACTOR_SRC_COLOR/*ALPHA*/;
    case D3DBLEND_INVSRCCOLOR:     return PIPE_BLENDFACTOR_INV_SRC_COLOR/*ALPHA*/;
    case D3DBLEND_SRCALPHA:        return PIPE_BLENDFACTOR_SRC_ALPHA;
    case D3DBLEND_INVSRCALPHA:     return PIPE_BLENDFACTOR_INV_SRC_ALPHA;
    case D3DBLEND_DESTALPHA:       return PIPE_BLENDFACTOR_DST_ALPHA;
    case D3DBLEND_INVDESTALPHA:    return PIPE_BLENDFACTOR_INV_DST_ALPHA;
    case D3DBLEND_DESTCOLOR:       return PIPE_BLENDFACTOR_DST_COLOR/*ALPHA*/;
    case D3DBLEND_INVDESTCOLOR:    return PIPE_BLENDFACTOR_INV_DST_COLOR/*ALPHA*/;
    case D3DBLEND_SRCALPHASAT:     return PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE;
    case D3DBLEND_BOTHSRCALPHA:    return PIPE_BLENDFACTOR_SRC_ALPHA;
    case D3DBLEND_BOTHINVSRCALPHA: return PIPE_BLENDFACTOR_INV_SRC_ALPHA;
    case D3DBLEND_BLENDFACTOR:     return PIPE_BLENDFACTOR_CONST_COLOR/*ALPHA*/;
    case D3DBLEND_INVBLENDFACTOR:  return PIPE_BLENDFACTOR_INV_CONST_COLOR/*ALPHA*/;
    case D3DBLEND_SRCCOLOR2:       return PIPE_BLENDFACTOR_ONE; /* XXX */
    case D3DBLEND_INVSRCCOLOR2:    return PIPE_BLENDFACTOR_ZERO; /* XXX */
    default:
       DBG_FLAG(DBG_UNKNOWN, "Unhandled blend factor %d\n", b);
       return PIPE_BLENDFACTOR_ZERO;
    }
}

static INLINE unsigned
d3dblend_color_to_pipe_blendfactor(D3DBLEND b)
{
    switch (b) {
    case D3DBLEND_ZERO:            return PIPE_BLENDFACTOR_ZERO;
    case D3DBLEND_ONE:             return PIPE_BLENDFACTOR_ONE;
    case D3DBLEND_SRCCOLOR:        return PIPE_BLENDFACTOR_SRC_COLOR;
    case D3DBLEND_INVSRCCOLOR:     return PIPE_BLENDFACTOR_INV_SRC_COLOR;
    case D3DBLEND_SRCALPHA:        return PIPE_BLENDFACTOR_SRC_ALPHA;
    case D3DBLEND_INVSRCALPHA:     return PIPE_BLENDFACTOR_INV_SRC_ALPHA;
    case D3DBLEND_DESTALPHA:       return PIPE_BLENDFACTOR_DST_ALPHA;
    case D3DBLEND_INVDESTALPHA:    return PIPE_BLENDFACTOR_INV_DST_ALPHA;
    case D3DBLEND_DESTCOLOR:       return PIPE_BLENDFACTOR_DST_COLOR;
    case D3DBLEND_INVDESTCOLOR:    return PIPE_BLENDFACTOR_INV_DST_COLOR;
    case D3DBLEND_SRCALPHASAT:     return PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE;
    case D3DBLEND_BOTHSRCALPHA:    return PIPE_BLENDFACTOR_SRC_ALPHA;
    case D3DBLEND_BOTHINVSRCALPHA: return PIPE_BLENDFACTOR_INV_SRC_ALPHA;
    case D3DBLEND_BLENDFACTOR:     return PIPE_BLENDFACTOR_CONST_COLOR;
    case D3DBLEND_INVBLENDFACTOR:  return PIPE_BLENDFACTOR_INV_CONST_COLOR;
    case D3DBLEND_SRCCOLOR2:       return PIPE_BLENDFACTOR_SRC1_COLOR;
    case D3DBLEND_INVSRCCOLOR2:    return PIPE_BLENDFACTOR_INV_SRC1_COLOR;
    default:
       DBG_FLAG(DBG_UNKNOWN, "Unhandled blend factor %d\n", b);
       return PIPE_BLENDFACTOR_ZERO;
    }
}

static INLINE unsigned
d3dtextureaddress_to_pipe_tex_wrap(D3DTEXTUREADDRESS addr)
{
    switch (addr) {
    case D3DTADDRESS_WRAP:       return PIPE_TEX_WRAP_REPEAT;
    case D3DTADDRESS_MIRROR:     return PIPE_TEX_WRAP_MIRROR_REPEAT;
    case D3DTADDRESS_CLAMP:      return PIPE_TEX_WRAP_CLAMP_TO_EDGE;
    case D3DTADDRESS_BORDER:     return PIPE_TEX_WRAP_CLAMP_TO_BORDER;
    case D3DTADDRESS_MIRRORONCE: return PIPE_TEX_WRAP_MIRROR_CLAMP_TO_EDGE;
    default:
        assert(0);
        return PIPE_TEX_WRAP_CLAMP_TO_EDGE;
    }
}

static INLINE unsigned
d3dtexturefiltertype_to_pipe_tex_filter(D3DTEXTUREFILTERTYPE filter)
{
    switch (filter) {
    case D3DTEXF_POINT:       return PIPE_TEX_FILTER_NEAREST;
    case D3DTEXF_LINEAR:      return PIPE_TEX_FILTER_LINEAR;
    case D3DTEXF_ANISOTROPIC: return PIPE_TEX_FILTER_LINEAR;

    case D3DTEXF_NONE:
    case D3DTEXF_PYRAMIDALQUAD:
    case D3DTEXF_GAUSSIANQUAD:
    case D3DTEXF_CONVOLUTIONMONO:
    default:
        assert(0);
        return PIPE_TEX_FILTER_NEAREST;
    }
}

static INLINE unsigned
d3dtexturefiltertype_to_pipe_tex_mipfilter(D3DTEXTUREFILTERTYPE filter)
{
    switch (filter) {
    case D3DTEXF_NONE:        return PIPE_TEX_MIPFILTER_NONE;
    case D3DTEXF_POINT:       return PIPE_TEX_FILTER_NEAREST;
    case D3DTEXF_LINEAR:      return PIPE_TEX_FILTER_LINEAR;
    case D3DTEXF_ANISOTROPIC: return PIPE_TEX_FILTER_LINEAR;

    case D3DTEXF_PYRAMIDALQUAD:
    case D3DTEXF_GAUSSIANQUAD:
    case D3DTEXF_CONVOLUTIONMONO:
    default:
        assert(0);
        return PIPE_TEX_MIPFILTER_NONE;
    }
}

static INLINE unsigned nine_format_get_stride(enum pipe_format format,
                                              unsigned width)
{
    unsigned stride = util_format_get_stride(format, width);

    return align(stride, 4);
}

static INLINE unsigned nine_format_get_level_alloc_size(enum pipe_format format,
                                                        unsigned width,
                                                        unsigned height,
                                                        unsigned level)
{
    unsigned w, h, size;

    w = u_minify(width, level);
    h = u_minify(height, level);
    size = nine_format_get_stride(format, w) *
        util_format_get_nblocksy(format, h);
    return size;
}

static INLINE unsigned nine_format_get_size_and_offsets(enum pipe_format format,
                                                        unsigned *offsets,
                                                        unsigned width,
                                                        unsigned height,
                                                        unsigned last_level)
{
    unsigned l, w, h, size = 0;

    for (l = 0; l <= last_level; ++l) {
        w = u_minify(width, l);
        h = u_minify(height, l);
        offsets[l] = size;
        size += nine_format_get_stride(format, w) *
            util_format_get_nblocksy(format, h);
    }

    return size;
}

#endif /* _NINE_PIPE_H_ */
