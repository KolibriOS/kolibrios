
#include <assert.h>
#include <stdlib.h>
#include <memory.h>

#include <intel_bufmgr.h>
//#include "xf86.h"
#include "uxa/intel.h"
#include "i830_reg.h"
#include "i965_reg.h"

/* bring in brw structs */
#include "brw_defines.h"
#include "brw_structs.h"

#include "i915_pciids.h"
#include <pixlib2.h>
#include <kos32sys.h>

#define PictOpClear             0
#define PictOpSrc               1
#define PictOpDst               2
#define PictOpOver              3
#define PictOpOverReverse       4
#define PictOpIn                5
#define PictOpInReverse         6
#define PictOpOut               7
#define PictOpOutReverse        8
#define PictOpAtop              9
#define PictOpAtopReverse       10
#define PictOpXor               11
#define PictOpAdd               12
#define PictOpSaturate          13
#define PictOpMaximum           13

static int    tls_mask;

intel_screen_private *driverPrivate;
__LOCK_INIT_RECURSIVE(, __uxa_lock);

#define DBG printf

typedef struct
{
    struct list     entry;
    uint32_t        width;
    uint32_t        height;
    void           *data;
    uint32_t        pitch;
    drm_intel_bo   *bo;
    uint32_t        bo_size;
    uint32_t        flags;
}surface_t;

#define to_surface(x) (surface_t*)((x)->handle)

struct _Pixmap fb_pixmap;

struct list sf_list;

int uxa_update_fb(struct intel_screen_private *intel);

static void i830_done_composite(PixmapPtr dest)
{
	intel_screen_private *intel = intel_get_screen_private();

	if (intel->vertex_flush)
		intel->vertex_flush(intel);

//	intel_debug_flush(scrn);
}

int sna_bitmap_from_handle(bitmap_t *bitmap, uint32_t handle)
{
	struct intel_screen_private *intel = intel_get_screen_private();
	drm_intel_bo *bo;
    surface_t    *sf;
    unsigned int size;

    bitmap->handle = 0;

    __lock_acquire_recursive(__uxa_lock);
    list_for_each_entry(sf, &sf_list, entry)
    {
        if (sf->bo->handle == handle)
        {
            bitmap->handle = (uint32_t)sf;
            break;
        }
    }
    __lock_release_recursive(__uxa_lock);

    if(bitmap->handle)
        return 0;

    sf = malloc(sizeof(*sf));
    if(sf == NULL)
        goto err_1;

    size = bitmap->pitch * bitmap->height;

    bo = bo_create_from_gem_handle(intel->bufmgr, size, handle);

    sf->width   = bitmap->width;
    sf->height  = bitmap->height;
    sf->data    = NULL;
    sf->pitch   = bitmap->pitch;
    sf->bo      = bo;
    sf->bo_size = size;
    sf->flags   = bitmap->flags;

    bitmap->handle = (uint32_t)sf;

    return 0;

err_1:

    return -1;
};

void sna_set_bo_handle(bitmap_t *bitmap, int handle)
{
    sna_bitmap_from_handle(bitmap, handle);
};


int sna_blit_tex(bitmap_t *bitmap, bool scale, int dst_x, int dst_y,
                  int w, int h, int src_x, int src_y)
{
//    DBG("%s\n", __FUNCTION__);

    struct _Pixmap pixSrc, pixMask;
    struct intel_pixmap privSrc;
    struct _Picture pictSrc, pictDst;
	struct intel_screen_private *intel = intel_get_screen_private();

    surface_t *sf = to_surface(bitmap);

    int winx, winy;

    char proc_info[1024];
    get_proc_info(proc_info);
    winx = *(uint32_t*)(proc_info+34);
    winy = *(uint32_t*)(proc_info+38);

    memset(&pixSrc,  0, sizeof(pixSrc));
    memset(&pixMask, 0, sizeof(pixMask));
    memset(&privSrc, 0, sizeof(pixSrc));

    memset(&pictSrc, 0, sizeof(pictSrc));
    memset(&pictDst, 0, sizeof(pictDst));

    pixSrc.drawable.bitsPerPixel = 32;
    pixSrc.drawable.width        = sf->width;
    pixSrc.drawable.height       = sf->height;
    pixSrc.devKind               = sf->pitch;
    pixSrc.private               = &privSrc;

    list_init(&privSrc.batch);
    privSrc.bo = sf->bo;
    privSrc.stride = sf->pitch;
    privSrc.tiling = I915_TILING_X;

    pictSrc.format     = PICT_x8r8g8b8;
    pictSrc.filter     = PictFilterNearest;
    pictSrc.repeatType = RepeatNone;

    pictDst.format     = PICT_a8r8g8b8;
    pictDst.filter = PictFilterNearest;
    pictDst.repeatType = RepeatNone;

    uxa_update_fb(intel);

    i965_prepare_composite(PictOpSrc, &pictSrc, NULL, &pictDst,
                           &pixSrc, NULL, &fb_pixmap);

    i965_composite(&fb_pixmap, src_x, src_y, 0, 0,
                    dst_x+winx, dst_y+winy, w, h);

    i830_done_composite(&fb_pixmap);

	intel_batch_submit();

//    DBG("%s done\n", __FUNCTION__);

    return 0;
};


int uxa_init_fb(struct intel_screen_private *intel)
{
    struct drm_i915_fb_info fb;
    static struct intel_pixmap ipix;
    int ret;

    memset(&fb, 0, sizeof(fb));

    ret = drmIoctl(intel->scrn, SRV_FBINFO, &fb);
	if( ret != 0 )
	    return ret;

    intel->front_buffer = intel_bo_gem_create_from_name(intel->bufmgr,"frontbuffer", fb.name);
    if(intel->front_buffer == NULL)
        return -1;

    ipix.bo = intel->front_buffer;
    list_init(&ipix.batch);
    ipix.stride = fb.pitch;
    ipix.tiling = fb.tiling;
    ipix.pinned = PIN_SCANOUT;

    printf("create frontbuffer name %d bo %x\n", fb.name, ipix.bo);
    printf("size %d, offset %d handle %d\n",ipix.bo->size, ipix.bo->offset, ipix.bo->handle);

    fb_pixmap.drawable.bitsPerPixel = 32;
    fb_pixmap.drawable.width  = fb.width;
    fb_pixmap.drawable.height = fb.height;
    fb_pixmap.devKind = fb.pitch;
    fb_pixmap.private = &ipix;

    return 0;
}

int uxa_update_fb(struct intel_screen_private *intel)
{
    struct drm_i915_fb_info fb;
    struct intel_pixmap *ipix;
    size_t size;
    int ret;

//    DBG("%s\n", __FUNCTION__);

    ret = drmIoctl(intel->scrn, SRV_FBINFO, &fb);
	if( ret != 0 )
	    return ret;

    ipix = (struct intel_pixmap*)fb_pixmap.private;

    list_init(&ipix->batch);
    ipix->stride = fb.pitch;
    ipix->tiling = fb.tiling;

    fb_pixmap.drawable.width  = fb.width;
    fb_pixmap.drawable.height = fb.height;
    fb_pixmap.devKind = fb.pitch;

    return 0;
};

int uxa_init(uint32_t service)
{
    static struct pci_device device;
	struct intel_screen_private *intel = intel_get_screen_private();

    ioctl_t   io;
    int caps = 0;

    DBG("%s\n", __FUNCTION__);

    __lock_acquire_recursive(__uxa_lock);

    if(intel)
        goto done;

    io.handle   = service;
    io.io_code  = SRV_GET_PCI_INFO;
    io.input    = &device;
    io.inp_size = sizeof(device);
    io.output   = NULL;
    io.out_size = 0;

    if (call_service(&io)!=0)
        goto err1;

    intel = (intel_screen_private*)malloc(sizeof(*intel));
    if (intel == NULL)
        goto err1;

    list_init(&sf_list);

    driverPrivate = intel;
    memset(intel, 0, sizeof(*intel));

//    sna->cpu_features = sna_cpu_detect();

    intel->PciInfo = &device;
  	intel->info = intel_detect_chipset(intel->PciInfo);
    intel->scrn = service;

    intel->bufmgr = intel_bufmgr_gem_init(service, 8192);
    if(intel->bufmgr == NULL)
    {
		printf("Memory manager initialization failed\n");
		goto err1;
    };

	list_init(&intel->batch_pixmaps);

	if ((INTEL_INFO(intel)->gen == 060)) {
		intel->wa_scratch_bo =
			drm_intel_bo_alloc(intel->bufmgr, "wa scratch",
					   4096, 4096);
	}

    if( uxa_init_fb(intel) != 0)
        goto err1;

	intel_batch_init();

	if (INTEL_INFO(intel)->gen >= 040)
		gen4_render_state_init();

	if (!intel_uxa_init()) {
		printf("Hardware acceleration initialization failed\n");
		goto err1;
	}

    tls_mask = tls_alloc();

//    printf("tls mask %x\n", tls_mask);

done:
//    caps = sna_device->render.caps;

err1:
    __lock_release_recursive(__uxa_lock);

    LEAVE();
    return caps;
}



static void
gen6_context_switch(intel_screen_private *intel,
		    int new_mode)
{
	intel_batch_submit(intel->scrn);
}

static void
gen5_context_switch(intel_screen_private *intel,
		    int new_mode)
{
	/* Ironlake has a limitation that a 3D or Media command can't
	 * be the first command after a BLT, unless it's
	 * non-pipelined.  Instead of trying to track it and emit a
	 * command at the right time, we just emit a dummy
	 * non-pipelined 3D instruction after each blit.
	 */

	if (new_mode == I915_EXEC_BLT) {
		OUT_BATCH(MI_FLUSH |
			  MI_STATE_INSTRUCTION_CACHE_FLUSH |
			  MI_INHIBIT_RENDER_CACHE_FLUSH);
	} else {
		OUT_BATCH(CMD_POLY_STIPPLE_OFFSET << 16);
		OUT_BATCH(0);
	}
}

static void
gen4_context_switch(intel_screen_private *intel,
		    int new_mode)
{
	if (new_mode == I915_EXEC_BLT) {
		OUT_BATCH(MI_FLUSH |
			  MI_STATE_INSTRUCTION_CACHE_FLUSH |
			  MI_INHIBIT_RENDER_CACHE_FLUSH);
	}
}

static void
intel_limits_init(intel_screen_private *intel)
{
	/* Limits are described in the BLT engine chapter under Graphics Data Size
	 * Limitations, and the descriptions of SURFACE_STATE, 3DSTATE_BUFFER_INFO,
	 * 3DSTATE_DRAWING_RECTANGLE, 3DSTATE_MAP_INFO, and 3DSTATE_MAP_INFO.
	 *
	 * i845 through i965 limits 2D rendering to 65536 lines and pitch of 32768.
	 *
	 * i965 limits 3D surface to (2*element size)-aligned offset if un-tiled.
	 * i965 limits 3D surface to 4kB-aligned offset if tiled.
	 * i965 limits 3D surfaces to w,h of ?,8192.
	 * i965 limits 3D surface to pitch of 1B - 128kB.
	 * i965 limits 3D surface pitch alignment to 1 or 2 times the element size.
	 * i965 limits 3D surface pitch alignment to 512B if tiled.
	 * i965 limits 3D destination drawing rect to w,h of 8192,8192.
	 *
	 * i915 limits 3D textures to 4B-aligned offset if un-tiled.
	 * i915 limits 3D textures to ~4kB-aligned offset if tiled.
	 * i915 limits 3D textures to width,height of 2048,2048.
	 * i915 limits 3D textures to pitch of 16B - 8kB, in dwords.
	 * i915 limits 3D destination to ~4kB-aligned offset if tiled.
	 * i915 limits 3D destination to pitch of 16B - 8kB, in dwords, if un-tiled.
	 * i915 limits 3D destination to pitch 64B-aligned if used with depth.
	 * i915 limits 3D destination to pitch of 512B - 8kB, in tiles, if tiled.
	 * i915 limits 3D destination to POT aligned pitch if tiled.
	 * i915 limits 3D destination drawing rect to w,h of 2048,2048.
	 *
	 * i845 limits 3D textures to 4B-aligned offset if un-tiled.
	 * i845 limits 3D textures to ~4kB-aligned offset if tiled.
	 * i845 limits 3D textures to width,height of 2048,2048.
	 * i845 limits 3D textures to pitch of 4B - 8kB, in dwords.
	 * i845 limits 3D destination to 4B-aligned offset if un-tiled.
	 * i845 limits 3D destination to ~4kB-aligned offset if tiled.
	 * i845 limits 3D destination to pitch of 8B - 8kB, in dwords.
	 * i845 limits 3D destination drawing rect to w,h of 2048,2048.
	 *
	 * For the tiled issues, the only tiled buffer we draw to should be
	 * the front, which will have an appropriate pitch/offset already set up,
	 * so UXA doesn't need to worry.
	 */
	if (INTEL_INFO(intel)->gen >= 040) {
		intel->accel_pixmap_offset_alignment = 4 * 2;
		intel->accel_max_x = 8192;
		intel->accel_max_y = 8192;
	} else {
		intel->accel_pixmap_offset_alignment = 4;
		intel->accel_max_x = 2048;
		intel->accel_max_y = 2048;
	}
}


Bool intel_uxa_init()
{
	intel_screen_private *intel = intel_get_screen_private();

	intel_limits_init(intel);

	intel->prim_offset = 0;
	intel->vertex_count = 0;
	intel->vertex_offset = 0;
	intel->vertex_used = 0;
	intel->floats_per_vertex = 0;
	intel->last_floats_per_vertex = 0;
	intel->vertex_bo = NULL;
	intel->surface_used = 0;
	intel->surface_reloc = 0;

/*
	intel->uxa_driver->check_composite = i965_check_composite;
	intel->uxa_driver->check_composite_texture = i965_check_composite_texture;
	intel->uxa_driver->prepare_composite = i965_prepare_composite;
	intel->uxa_driver->composite = i965_composite;
	intel->uxa_driver->done_composite = i830_done_composite;
*/
	intel->vertex_flush = i965_vertex_flush;
	intel->batch_flush = i965_batch_flush;
	intel->batch_commit_notify = i965_batch_commit_notify;

	if (IS_GEN4(intel)) {
		intel->context_switch = gen4_context_switch;
	} else if (IS_GEN5(intel)) {
		intel->context_switch = gen5_context_switch;
	} else {
		intel->context_switch = gen6_context_switch;
	}

	return TRUE;
}


static const struct intel_device_info intel_generic_info = {
	.gen = -1,
};

static const struct intel_device_info intel_i915_info = {
	.gen = 030,
};
static const struct intel_device_info intel_i945_info = {
	.gen = 031,
};

static const struct intel_device_info intel_g33_info = {
	.gen = 033,
};

static const struct intel_device_info intel_i965_info = {
	.gen = 040,
};

static const struct intel_device_info intel_g4x_info = {
	.gen = 045,
};

static const struct intel_device_info intel_ironlake_info = {
	.gen = 050,
};

static const struct intel_device_info intel_sandybridge_info = {
	.gen = 060,
};

static const struct intel_device_info intel_ivybridge_info = {
	.gen = 070,
};

static const struct intel_device_info intel_valleyview_info = {
	.gen = 071,
};

static const struct intel_device_info intel_haswell_info = {
	.gen = 075,
};

#define INTEL_DEVICE_MATCH(d,i) \
    { 0x8086, (d), PCI_MATCH_ANY, PCI_MATCH_ANY, 0x3 << 16, 0xff << 16, (intptr_t)(i) }


static const struct pci_id_match intel_device_match[] = {

	INTEL_I915G_IDS(&intel_i915_info),
	INTEL_I915GM_IDS(&intel_i915_info),
	INTEL_I945G_IDS(&intel_i945_info),
	INTEL_I945GM_IDS(&intel_i945_info),

	INTEL_G33_IDS(&intel_g33_info),
	INTEL_PINEVIEW_IDS(&intel_g33_info),

	INTEL_I965G_IDS(&intel_i965_info),
	INTEL_I965GM_IDS(&intel_i965_info),

	INTEL_G45_IDS(&intel_g4x_info),
	INTEL_GM45_IDS(&intel_g4x_info),

	INTEL_IRONLAKE_D_IDS(&intel_ironlake_info),
	INTEL_IRONLAKE_M_IDS(&intel_ironlake_info),

	INTEL_SNB_D_IDS(&intel_sandybridge_info),
	INTEL_SNB_M_IDS(&intel_sandybridge_info),

	INTEL_IVB_D_IDS(&intel_ivybridge_info),
	INTEL_IVB_M_IDS(&intel_ivybridge_info),

	INTEL_HSW_D_IDS(&intel_haswell_info),
	INTEL_HSW_M_IDS(&intel_haswell_info),

	INTEL_VLV_D_IDS(&intel_valleyview_info),
	INTEL_VLV_M_IDS(&intel_valleyview_info),

	INTEL_VGA_DEVICE(PCI_MATCH_ANY, &intel_generic_info),

	{ 0, 0, 0 },
};

const struct pci_id_match *PciDevMatch(uint16_t dev,const struct pci_id_match *list)
{
    while(list->device_id)
    {
        if(dev==list->device_id)
            return list;
        list++;
    }
    return NULL;
}


const struct intel_device_info *
intel_detect_chipset(struct pci_device *pci)
{
    const struct pci_id_match *ent = NULL;

    ent = PciDevMatch(pci->device_id, intel_device_match);

    if(ent != NULL)
        return (const struct intel_device_info*)ent->match_data;
    else
        return &intel_generic_info;
}

