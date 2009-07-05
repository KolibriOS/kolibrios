/*
 * Copyright © 2007 David Airlie
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
 *
 * Authors:
 *     David Airlie
 */
    /*
     *  Modularization
     */

//#include <linux/module.h>
//#include <linux/kernel.h>
//#include <linux/errno.h>
//#include <linux/string.h>
//#include <linux/mm.h>
//#include <linux/tty.h>
//#include <linux/slab.h>
//#include <linux/delay.h>
//#include <linux/fb.h>
//#include <linux/init.h>

#include "drmP.h"
#include "drm.h"
#include "drm_crtc.h"
#include "drm_crtc_helper.h"
#include "radeon_drm.h"
#include "radeon.h"

#include <drm_mm.h>
#include "radeon_object.h"


#define FB_TYPE_PACKED_PIXELS       0   /* Packed Pixels    */
#define FB_VISUAL_TRUECOLOR     2   /* True color   */

struct fb_fix_screeninfo {
    char id[16];            /* identification string eg "TT Builtin" */
    unsigned long smem_start;   /* Start of frame buffer mem */
                    /* (physical address) */
    __u32 smem_len;         /* Length of frame buffer mem */
    __u32 type;         /* see FB_TYPE_*        */
    __u32 type_aux;         /* Interleave for interleaved Planes */
    __u32 visual;           /* see FB_VISUAL_*      */
    __u16 xpanstep;         /* zero if no hardware panning  */
    __u16 ypanstep;         /* zero if no hardware panning  */
    __u16 ywrapstep;        /* zero if no hardware ywrap    */
    __u32 line_length;      /* length of a line in bytes    */
    unsigned long mmio_start;   /* Start of Memory Mapped I/O   */
                    /* (physical address) */
    __u32 mmio_len;         /* Length of Memory Mapped I/O  */
    __u32 accel;            /* Indicate to driver which */
                    /*  specific chip/card we have  */
    __u16 reserved[3];      /* Reserved for future compatibility */
};




struct fb_bitfield {
    __u32 offset;           /* beginning of bitfield    */
    __u32 length;           /* length of bitfield       */
    __u32 msb_right;        /* != 0 : Most significant bit is */
                    /* right */
};


struct fb_var_screeninfo {
    __u32 xres;         /* visible resolution       */
    __u32 yres;
    __u32 xres_virtual;     /* virtual resolution       */
    __u32 yres_virtual;
    __u32 xoffset;          /* offset from virtual to visible */
    __u32 yoffset;          /* resolution           */

    __u32 bits_per_pixel;       /* guess what           */
    __u32 grayscale;        /* != 0 Graylevels instead of colors */

    struct fb_bitfield red;     /* bitfield in fb mem if true color, */
    struct fb_bitfield green;   /* else only length is significant */
    struct fb_bitfield blue;
    struct fb_bitfield transp;  /* transparency         */

    __u32 nonstd;           /* != 0 Non standard pixel format */

    __u32 activate;         /* see FB_ACTIVATE_*        */

    __u32 height;           /* height of picture in mm    */
    __u32 width;            /* width of picture in mm     */

    __u32 accel_flags;      /* (OBSOLETE) see fb_info.flags */

    /* Timing: All values in pixclocks, except pixclock (of course) */
    __u32 pixclock;         /* pixel clock in ps (pico seconds) */
    __u32 left_margin;      /* time from sync to picture    */
    __u32 right_margin;     /* time from picture to sync    */
    __u32 upper_margin;     /* time from sync to picture    */
    __u32 lower_margin;
    __u32 hsync_len;        /* length of horizontal sync    */
    __u32 vsync_len;        /* length of vertical sync  */
    __u32 sync;         /* see FB_SYNC_*        */
    __u32 vmode;            /* see FB_VMODE_*       */
    __u32 rotate;           /* angle we rotate counter clockwise */
    __u32 reserved[5];      /* Reserved for future compatibility */
};



struct fb_chroma {
    __u32 redx; /* in fraction of 1024 */
    __u32 greenx;
    __u32 bluex;
    __u32 whitex;
    __u32 redy;
    __u32 greeny;
    __u32 bluey;
    __u32 whitey;
};

struct fb_videomode {
    const char *name;   /* optional */
    u32 refresh;        /* optional */
    u32 xres;
    u32 yres;
    u32 pixclock;
    u32 left_margin;
    u32 right_margin;
    u32 upper_margin;
    u32 lower_margin;
    u32 hsync_len;
    u32 vsync_len;
    u32 sync;
    u32 vmode;
    u32 flag;
};


struct fb_monspecs {
    struct fb_chroma chroma;
    struct fb_videomode *modedb;    /* mode database */
    __u8  manufacturer[4];      /* Manufacturer */
    __u8  monitor[14];      /* Monitor String */
    __u8  serial_no[14];        /* Serial Number */
    __u8  ascii[14];        /* ? */
    __u32 modedb_len;       /* mode database length */
    __u32 model;            /* Monitor Model */
    __u32 serial;           /* Serial Number - Integer */
    __u32 year;         /* Year manufactured */
    __u32 week;         /* Week Manufactured */
    __u32 hfmin;            /* hfreq lower limit (Hz) */
    __u32 hfmax;            /* hfreq upper limit (Hz) */
    __u32 dclkmin;          /* pixelclock lower limit (Hz) */
    __u32 dclkmax;          /* pixelclock upper limit (Hz) */
    __u16 input;            /* display type - see FB_DISP_* */
    __u16 dpms;         /* DPMS support - see FB_DPMS_ */
    __u16 signal;           /* Signal Type - see FB_SIGNAL_* */
    __u16 vfmin;            /* vfreq lower limit (Hz) */
    __u16 vfmax;            /* vfreq upper limit (Hz) */
    __u16 gamma;            /* Gamma - in fractions of 100 */
    __u16 gtf   : 1;        /* supports GTF */
    __u16 misc;         /* Misc flags - see FB_MISC_* */
    __u8  version;          /* EDID version... */
    __u8  revision;         /* ...and revision */
    __u8  max_x;            /* Maximum horizontal size (cm) */
    __u8  max_y;            /* Maximum vertical size (cm) */
};


struct fb_info {
    int node;
    int flags;
//    struct mutex lock;      /* Lock for open/release/ioctl funcs */
//    struct mutex mm_lock;       /* Lock for fb_mmap and smem_* fields */
    struct fb_var_screeninfo var;   /* Current var */
    struct fb_fix_screeninfo fix;   /* Current fix */
    struct fb_monspecs monspecs;    /* Current Monitor specs */
//    struct work_struct queue;   /* Framebuffer event queue */
//    struct fb_pixmap pixmap;    /* Image hardware mapper */
//    struct fb_pixmap sprite;    /* Cursor hardware mapper */
//    struct fb_cmap cmap;        /* Current cmap */
    struct list_head modelist;      /* mode list */
    struct fb_videomode *mode;  /* current mode */

#ifdef CONFIG_FB_BACKLIGHT
    /* assigned backlight device */
    /* set before framebuffer registration,
       remove after unregister */
    struct backlight_device *bl_dev;

    /* Backlight level curve */
    struct mutex bl_curve_mutex;
    u8 bl_curve[FB_BACKLIGHT_LEVELS];
#endif
#ifdef CONFIG_FB_DEFERRED_IO
    struct delayed_work deferred_work;
    struct fb_deferred_io *fbdefio;
#endif

    struct fb_ops *fbops;
//    struct device *device;      /* This is the parent */
//   struct device *dev;     /* This is this fb device */
    int class_flag;                    /* private sysfs flags */
#ifdef CONFIG_FB_TILEBLITTING
    struct fb_tile_ops *tileops;    /* Tile Blitting */
#endif
    char __iomem *screen_base;  /* Virtual address */
    unsigned long screen_size;  /* Amount of ioremapped VRAM or 0 */
    void *pseudo_palette;       /* Fake palette of 16 colors */
#define FBINFO_STATE_RUNNING    0
#define FBINFO_STATE_SUSPENDED  1
    u32 state;          /* Hardware state i.e suspend */
    void *fbcon_par;                /* fbcon use-only private area */
    /* From here on everything is device dependent */
    void *par;
    /* we need the PCI or similiar aperture base/size not
       smem_start/size as smem_start may just be an object
       allocated inside the aperture so may not actually overlap */
    resource_size_t aperture_base;
    resource_size_t aperture_size;
};



struct radeon_fb_device {
	struct radeon_device		*rdev;
	struct drm_display_mode		*mode;
	struct radeon_framebuffer	*rfb;
    int                         crtc_count;
	/* crtc currently bound to this */
	uint32_t			crtc_ids[2];
};

int radeon_gem_fb_object_create(struct radeon_device *rdev, int size,
                 int alignment, int initial_domain,
                 bool discardable, bool kernel,
                 bool interruptible,
                 struct drm_gem_object **obj);

struct fb_info *framebuffer_alloc(size_t size);

#if 0
static int radeonfb_setcolreg(unsigned regno,
			      unsigned red,
			      unsigned green,
			      unsigned blue,
			      unsigned transp,
			      struct fb_info *info)
{
	struct radeon_fb_device *rfbdev = info->par;
	struct drm_device *dev = rfbdev->rdev->ddev;
	struct drm_crtc *crtc;
	int i;

	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		struct radeon_crtc *radeon_crtc = to_radeon_crtc(crtc);
		struct drm_mode_set *modeset = &radeon_crtc->mode_set;
		struct drm_framebuffer *fb = modeset->fb;

		for (i = 0; i < rfbdev->crtc_count; i++) {
			if (crtc->base.id == rfbdev->crtc_ids[i]) {
				break;
			}
		}
		if (i == rfbdev->crtc_count) {
			continue;
		}
		if (regno > 255) {
			return 1;
		}
		if (fb->depth == 8) {
			radeon_crtc_fb_gamma_set(crtc, red, green, blue, regno);
			return 0;
		}

		if (regno < 16) {
			switch (fb->depth) {
			case 15:
				fb->pseudo_palette[regno] = ((red & 0xf800) >> 1) |
					((green & 0xf800) >>  6) |
					((blue & 0xf800) >> 11);
				break;
			case 16:
				fb->pseudo_palette[regno] = (red & 0xf800) |
					((green & 0xfc00) >>  5) |
					((blue  & 0xf800) >> 11);
				break;
			case 24:
			case 32:
				fb->pseudo_palette[regno] = ((red & 0xff00) << 8) |
					(green & 0xff00) |
					((blue  & 0xff00) >> 8);
				break;
			}
		}
	}
	return 0;
}

static int radeonfb_check_var(struct fb_var_screeninfo *var,
			      struct fb_info *info)
{
	struct radeon_fb_device *rfbdev = info->par;
	struct radeon_framebuffer *rfb = rfbdev->rfb;
	struct drm_framebuffer *fb = &rfb->base;
	int depth;

	if (var->pixclock == -1 || !var->pixclock) {
		return -EINVAL;
	}
	/* Need to resize the fb object !!! */
	if (var->xres > fb->width || var->yres > fb->height) {
		DRM_ERROR("Requested width/height is greater than current fb "
			   "object %dx%d > %dx%d\n", var->xres, var->yres,
			   fb->width, fb->height);
		DRM_ERROR("Need resizing code.\n");
		return -EINVAL;
	}

	switch (var->bits_per_pixel) {
	case 16:
		depth = (var->green.length == 6) ? 16 : 15;
		break;
	case 32:
		depth = (var->transp.length > 0) ? 32 : 24;
		break;
	default:
		depth = var->bits_per_pixel;
		break;
	}

	switch (depth) {
	case 8:
		var->red.offset = 0;
		var->green.offset = 0;
		var->blue.offset = 0;
		var->red.length = 8;
		var->green.length = 8;
		var->blue.length = 8;
		var->transp.length = 0;
		var->transp.offset = 0;
		break;
	case 15:
		var->red.offset = 10;
		var->green.offset = 5;
		var->blue.offset = 0;
		var->red.length = 5;
		var->green.length = 5;
		var->blue.length = 5;
		var->transp.length = 1;
		var->transp.offset = 15;
		break;
	case 16:
		var->red.offset = 11;
		var->green.offset = 5;
		var->blue.offset = 0;
		var->red.length = 5;
		var->green.length = 6;
		var->blue.length = 5;
		var->transp.length = 0;
		var->transp.offset = 0;
		break;
	case 24:
		var->red.offset = 16;
		var->green.offset = 8;
		var->blue.offset = 0;
		var->red.length = 8;
		var->green.length = 8;
		var->blue.length = 8;
		var->transp.length = 0;
		var->transp.offset = 0;
		break;
	case 32:
		var->red.offset = 16;
		var->green.offset = 8;
		var->blue.offset = 0;
		var->red.length = 8;
		var->green.length = 8;
		var->blue.length = 8;
		var->transp.length = 8;
		var->transp.offset = 24;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

#endif


/* this will let fbcon do the mode init */
static int radeonfb_set_par(struct fb_info *info)
{
	struct radeon_fb_device *rfbdev = info->par;
	struct drm_device *dev = rfbdev->rdev->ddev;
	struct fb_var_screeninfo *var = &info->var;
	struct drm_crtc *crtc;
	int ret;
	int i;

	if (var->pixclock != -1) {
		DRM_ERROR("PIXEL CLCOK SET\n");
		return -EINVAL;
	}

	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		struct radeon_crtc *radeon_crtc = to_radeon_crtc(crtc);

		for (i = 0; i < rfbdev->crtc_count; i++) {
			if (crtc->base.id == rfbdev->crtc_ids[i]) {
				break;
			}
		}
		if (i == rfbdev->crtc_count) {
			continue;
		}
		if (crtc->fb == radeon_crtc->mode_set.fb) {
//           mutex_lock(&dev->mode_config.mutex);
			ret = crtc->funcs->set_config(&radeon_crtc->mode_set);
//           mutex_unlock(&dev->mode_config.mutex);
			if (ret) {
				return ret;
			}
		}
	}
	return 0;
}

#if 0

static int radeonfb_pan_display(struct fb_var_screeninfo *var,
				struct fb_info *info)
{
	struct radeon_fb_device *rfbdev = info->par;
	struct drm_device *dev = rfbdev->rdev->ddev;
	struct drm_mode_set *modeset;
	struct drm_crtc *crtc;
	struct radeon_crtc *radeon_crtc;
	int ret = 0;
	int i;

	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		for (i = 0; i < rfbdev->crtc_count; i++) {
			if (crtc->base.id == rfbdev->crtc_ids[i]) {
				break;
			}
		}

		if (i == rfbdev->crtc_count) {
			continue;
		}

		radeon_crtc = to_radeon_crtc(crtc);
		modeset = &radeon_crtc->mode_set;

		modeset->x = var->xoffset;
		modeset->y = var->yoffset;

		if (modeset->num_connectors) {
			mutex_lock(&dev->mode_config.mutex);
			ret = crtc->funcs->set_config(modeset);
			mutex_unlock(&dev->mode_config.mutex);
			if (!ret) {
				info->var.xoffset = var->xoffset;
				info->var.yoffset = var->yoffset;
			}
		}
	}
	return ret;
}

static void radeonfb_on(struct fb_info *info)
{
	struct radeon_fb_device *rfbdev = info->par;
	struct drm_device *dev = rfbdev->rdev->ddev;
	struct drm_crtc *crtc;
	struct drm_encoder *encoder;
	int i;

	/*
	 * For each CRTC in this fb, find all associated encoders
	 * and turn them off, then turn off the CRTC.
	 */
	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		struct drm_crtc_helper_funcs *crtc_funcs = crtc->helper_private;

		for (i = 0; i < rfbdev->crtc_count; i++) {
			if (crtc->base.id == rfbdev->crtc_ids[i]) {
				break;
			}
		}

		mutex_lock(&dev->mode_config.mutex);
		crtc_funcs->dpms(crtc, DRM_MODE_DPMS_ON);
		mutex_unlock(&dev->mode_config.mutex);

		/* Found a CRTC on this fb, now find encoders */
		list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {
			if (encoder->crtc == crtc) {
				struct drm_encoder_helper_funcs *encoder_funcs;

				encoder_funcs = encoder->helper_private;
				mutex_lock(&dev->mode_config.mutex);
				encoder_funcs->dpms(encoder, DRM_MODE_DPMS_ON);
				mutex_unlock(&dev->mode_config.mutex);
			}
		}
	}
}

static void radeonfb_off(struct fb_info *info, int dpms_mode)
{
	struct radeon_fb_device *rfbdev = info->par;
	struct drm_device *dev = rfbdev->rdev->ddev;
	struct drm_crtc *crtc;
	struct drm_encoder *encoder;
	int i;

	/*
	 * For each CRTC in this fb, find all associated encoders
	 * and turn them off, then turn off the CRTC.
	 */
	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		struct drm_crtc_helper_funcs *crtc_funcs = crtc->helper_private;

		for (i = 0; i < rfbdev->crtc_count; i++) {
			if (crtc->base.id == rfbdev->crtc_ids[i]) {
				break;
			}
		}

		/* Found a CRTC on this fb, now find encoders */
		list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {
			if (encoder->crtc == crtc) {
				struct drm_encoder_helper_funcs *encoder_funcs;

				encoder_funcs = encoder->helper_private;
				mutex_lock(&dev->mode_config.mutex);
				encoder_funcs->dpms(encoder, dpms_mode);
				mutex_unlock(&dev->mode_config.mutex);
			}
		}
		if (dpms_mode == DRM_MODE_DPMS_OFF) {
			mutex_lock(&dev->mode_config.mutex);
			crtc_funcs->dpms(crtc, dpms_mode);
			mutex_unlock(&dev->mode_config.mutex);
		}
	}
}

int radeonfb_blank(int blank, struct fb_info *info)
{
	switch (blank) {
	case FB_BLANK_UNBLANK:
		radeonfb_on(info);
		break;
	case FB_BLANK_NORMAL:
		radeonfb_off(info, DRM_MODE_DPMS_STANDBY);
		break;
	case FB_BLANK_HSYNC_SUSPEND:
		radeonfb_off(info, DRM_MODE_DPMS_STANDBY);
		break;
	case FB_BLANK_VSYNC_SUSPEND:
		radeonfb_off(info, DRM_MODE_DPMS_SUSPEND);
		break;
	case FB_BLANK_POWERDOWN:
		radeonfb_off(info, DRM_MODE_DPMS_OFF);
		break;
	}
	return 0;
}

static struct fb_ops radeonfb_ops = {
	.owner = THIS_MODULE,
	.fb_check_var = radeonfb_check_var,
	.fb_set_par = radeonfb_set_par,
	.fb_setcolreg = radeonfb_setcolreg,
	.fb_fillrect = cfb_fillrect,
	.fb_copyarea = cfb_copyarea,
	.fb_imageblit = cfb_imageblit,
	.fb_pan_display = radeonfb_pan_display,
	.fb_blank = radeonfb_blank,
};

/**
 * Curretly it is assumed that the old framebuffer is reused.
 *
 * LOCKING
 * caller should hold the mode config lock.
 *
 */
int radeonfb_resize(struct drm_device *dev, struct drm_crtc *crtc)
{
	struct fb_info *info;
	struct drm_framebuffer *fb;
	struct drm_display_mode *mode = crtc->desired_mode;

	fb = crtc->fb;
	if (fb == NULL) {
		return 1;
	}
	info = fb->fbdev;
	if (info == NULL) {
		return 1;
	}
	if (mode == NULL) {
		return 1;
	}
	info->var.xres = mode->hdisplay;
	info->var.right_margin = mode->hsync_start - mode->hdisplay;
	info->var.hsync_len = mode->hsync_end - mode->hsync_start;
	info->var.left_margin = mode->htotal - mode->hsync_end;
	info->var.yres = mode->vdisplay;
	info->var.lower_margin = mode->vsync_start - mode->vdisplay;
	info->var.vsync_len = mode->vsync_end - mode->vsync_start;
	info->var.upper_margin = mode->vtotal - mode->vsync_end;
	info->var.pixclock = 10000000 / mode->htotal * 1000 / mode->vtotal * 100;
	/* avoid overflow */
	info->var.pixclock = info->var.pixclock * 1000 / mode->vrefresh;

	return 0;
}
EXPORT_SYMBOL(radeonfb_resize);

static struct drm_mode_set panic_mode;

int radeonfb_panic(struct notifier_block *n, unsigned long ununsed,
		  void *panic_str)
{
	DRM_ERROR("panic occurred, switching back to text console\n");
	drm_crtc_helper_set_config(&panic_mode);
	return 0;
}
EXPORT_SYMBOL(radeonfb_panic);

static struct notifier_block paniced = {
	.notifier_call = radeonfb_panic,
};
#endif

static int radeon_align_pitch(struct radeon_device *rdev, int width, int bpp)
{
	int aligned = width;
	int align_large = (ASIC_IS_AVIVO(rdev));
	int pitch_mask = 0;

	switch (bpp / 8) {
	case 1:
		pitch_mask = align_large ? 255 : 127;
		break;
	case 2:
		pitch_mask = align_large ? 127 : 31;
		break;
	case 3:
	case 4:
		pitch_mask = align_large ? 63 : 15;
		break;
	}

	aligned += pitch_mask;
	aligned &= ~pitch_mask;
	return aligned;
}

int radeonfb_create(struct radeon_device *rdev,
		    uint32_t fb_width, uint32_t fb_height,
		    uint32_t surface_width, uint32_t surface_height,
		    struct radeon_framebuffer **rfb_p)
{
	struct fb_info *info;
	struct radeon_fb_device *rfbdev;
	struct drm_framebuffer *fb = NULL;
	struct radeon_framebuffer *rfb;
	struct drm_mode_fb_cmd mode_cmd;
	struct drm_gem_object *gobj = NULL;
	struct radeon_object *robj = NULL;
//   struct device *device = &rdev->pdev->dev;
	int size, aligned_size, ret;
	u64 fb_gpuaddr;
	void *fbptr = NULL;
	unsigned long tmp;

    ENTRY();

	mode_cmd.width = surface_width;
	mode_cmd.height = surface_height;
	mode_cmd.bpp = 32;
	/* need to align pitch with crtc limits */
	mode_cmd.pitch = radeon_align_pitch(rdev, mode_cmd.width, mode_cmd.bpp) * ((mode_cmd.bpp + 1) / 8);
    mode_cmd.depth = 32;

	size = mode_cmd.pitch * mode_cmd.height;
	aligned_size = ALIGN(size, PAGE_SIZE);

    ret = radeon_gem_fb_object_create(rdev, aligned_size, 0,
			RADEON_GEM_DOMAIN_VRAM,
            false, 0,
			false, &gobj);


	if (ret) {
		printk(KERN_ERR "failed to allocate framebuffer (%d %d)\n",
		       surface_width, surface_height);
		ret = -ENOMEM;
		goto out;
	}
	robj = gobj->driver_private;

//   mutex_lock(&rdev->ddev->struct_mutex);
	fb = radeon_framebuffer_create(rdev->ddev, &mode_cmd, gobj);
	if (fb == NULL) {
		DRM_ERROR("failed to allocate fb.\n");
		ret = -ENOMEM;
		goto out_unref;
	}
	ret = radeon_object_pin(robj, RADEON_GEM_DOMAIN_VRAM, &fb_gpuaddr);
	if (ret) {
		printk(KERN_ERR "failed to pin framebuffer\n");
		ret = -ENOMEM;
		goto out_unref;
	}

	list_add(&fb->filp_head, &rdev->ddev->mode_config.fb_kernel_list);

	rfb = to_radeon_framebuffer(fb);
	*rfb_p = rfb;
	rdev->fbdev_rfb = rfb;
	rdev->fbdev_robj = robj;

    info = framebuffer_alloc(sizeof(struct radeon_fb_device));
	if (info == NULL) {
		ret = -ENOMEM;
		goto out_unref;
	}
	rfbdev = info->par;

//   ret = radeon_object_kmap(robj, &fbptr);
//   if (ret) {
//       goto out_unref;
//   }

    fbptr = (void*)0xFE000000; // LFB_BASE


	strcpy(info->fix.id, "radeondrmfb");
	info->fix.type = FB_TYPE_PACKED_PIXELS;
	info->fix.visual = FB_VISUAL_TRUECOLOR;
	info->fix.type_aux = 0;
	info->fix.xpanstep = 1; /* doing it in hw */
	info->fix.ypanstep = 1; /* doing it in hw */
	info->fix.ywrapstep = 0;
//   info->fix.accel = FB_ACCEL_NONE;
	info->fix.type_aux = 0;
//   info->flags = FBINFO_DEFAULT;
//   info->fbops = &radeonfb_ops;
	info->fix.line_length = fb->pitch;
	tmp = fb_gpuaddr - rdev->mc.vram_location;
	info->fix.smem_start = rdev->mc.aper_base + tmp;
	info->fix.smem_len = size;
	info->screen_base = fbptr;
	info->screen_size = size;
	info->pseudo_palette = fb->pseudo_palette;
	info->var.xres_virtual = fb->width;
	info->var.yres_virtual = fb->height;
	info->var.bits_per_pixel = fb->bits_per_pixel;
	info->var.xoffset = 0;
	info->var.yoffset = 0;
//   info->var.activate = FB_ACTIVATE_NOW;
	info->var.height = -1;
	info->var.width = -1;
	info->var.xres = fb_width;
	info->var.yres = fb_height;
	info->fix.mmio_start = 0;
	info->fix.mmio_len = 0;
//   info->pixmap.size = 64*1024;
//   info->pixmap.buf_align = 8;
//   info->pixmap.access_align = 32;
//   info->pixmap.flags = FB_PIXMAP_SYSTEM;
//   info->pixmap.scan_align = 1;
	if (info->screen_base == NULL) {
		ret = -ENOSPC;
		goto out_unref;
	}
	DRM_INFO("fb mappable at 0x%lX\n",  info->fix.smem_start);
	DRM_INFO("vram apper at 0x%lX\n",  (unsigned long)rdev->mc.aper_base);
	DRM_INFO("size %lu\n", (unsigned long)size);
	DRM_INFO("fb depth is %d\n", fb->depth);
	DRM_INFO("   pitch is %d\n", fb->pitch);

	switch (fb->depth) {
	case 8:
		info->var.red.offset = 0;
		info->var.green.offset = 0;
		info->var.blue.offset = 0;
		info->var.red.length = 8; /* 8bit DAC */
		info->var.green.length = 8;
		info->var.blue.length = 8;
		info->var.transp.offset = 0;
		info->var.transp.length = 0;
		break;
	case 15:
		info->var.red.offset = 10;
		info->var.green.offset = 5;
		info->var.blue.offset = 0;
		info->var.red.length = 5;
		info->var.green.length = 5;
		info->var.blue.length = 5;
		info->var.transp.offset = 15;
		info->var.transp.length = 1;
		break;
	case 16:
		info->var.red.offset = 11;
		info->var.green.offset = 5;
		info->var.blue.offset = 0;
		info->var.red.length = 5;
		info->var.green.length = 6;
		info->var.blue.length = 5;
		info->var.transp.offset = 0;
		break;
	case 24:
		info->var.red.offset = 16;
		info->var.green.offset = 8;
		info->var.blue.offset = 0;
		info->var.red.length = 8;
		info->var.green.length = 8;
		info->var.blue.length = 8;
		info->var.transp.offset = 0;
		info->var.transp.length = 0;
		break;
	case 32:
		info->var.red.offset = 16;
		info->var.green.offset = 8;
		info->var.blue.offset = 0;
		info->var.red.length = 8;
		info->var.green.length = 8;
		info->var.blue.length = 8;
		info->var.transp.offset = 24;
		info->var.transp.length = 8;
		break;
	default:
		break;
	}

    dbgprintf("fb = %x\n", fb);

	fb->fbdev = info;
	rfbdev->rfb = rfb;
	rfbdev->rdev = rdev;

//   mutex_unlock(&rdev->ddev->struct_mutex);
	return 0;

out_unref:
	if (robj) {
//       radeon_object_kunmap(robj);
	}
	if (fb && ret) {
		list_del(&fb->filp_head);
 //      drm_gem_object_unreference(gobj);
//       drm_framebuffer_cleanup(fb);
		kfree(fb);
	}
//   drm_gem_object_unreference(gobj);
//   mutex_unlock(&rdev->ddev->struct_mutex);
out:
	return ret;
}

static int radeonfb_single_fb_probe(struct radeon_device *rdev)
{
	struct drm_crtc *crtc;
	struct drm_connector *connector;
	unsigned int fb_width = (unsigned)-1, fb_height = (unsigned)-1;
	unsigned int surface_width = 0, surface_height = 0;
	int new_fb = 0;
	int crtc_count = 0;
	int ret, i, conn_count = 0;
	struct radeon_framebuffer *rfb;
	struct fb_info *info;
	struct radeon_fb_device *rfbdev;
	struct drm_mode_set *modeset = NULL;

    ENTRY();

	/* first up get a count of crtcs now in use and new min/maxes width/heights */
	list_for_each_entry(crtc, &rdev->ddev->mode_config.crtc_list, head) {
		if (drm_helper_crtc_in_use(crtc)) {
			if (crtc->desired_mode) {
				if (crtc->desired_mode->hdisplay < fb_width)
					fb_width = crtc->desired_mode->hdisplay;

				if (crtc->desired_mode->vdisplay < fb_height)
					fb_height = crtc->desired_mode->vdisplay;

				if (crtc->desired_mode->hdisplay > surface_width)
					surface_width = crtc->desired_mode->hdisplay;

				if (crtc->desired_mode->vdisplay > surface_height)
					surface_height = crtc->desired_mode->vdisplay;
			}
			crtc_count++;
		}
	}

	if (crtc_count == 0 || fb_width == -1 || fb_height == -1) {
		/* hmm everyone went away - assume VGA cable just fell out
		   and will come back later. */

        dbgprintf("crtc count %x width %x height %x\n",
                   crtc_count, fb_width, fb_height);
		return 0;
	}

	/* do we have an fb already? */
	if (list_empty(&rdev->ddev->mode_config.fb_kernel_list)) {
		/* create an fb if we don't have one */
		ret = radeonfb_create(rdev, fb_width, fb_height, surface_width, surface_height, &rfb);
		if (ret) {
			return -EINVAL;
		}
		new_fb = 1;
	} else {
		struct drm_framebuffer *fb;
		fb = list_first_entry(&rdev->ddev->mode_config.fb_kernel_list, struct drm_framebuffer, filp_head);
		rfb = to_radeon_framebuffer(fb);

		/* if someone hotplugs something bigger than we have already allocated, we are pwned.
		   As really we can't resize an fbdev that is in the wild currently due to fbdev
		   not really being designed for the lower layers moving stuff around under it.
		   - so in the grand style of things - punt. */
		if ((fb->width < surface_width) || (fb->height < surface_height)) {
			DRM_ERROR("Framebuffer not large enough to scale console onto.\n");
			return -EINVAL;
		}
	}

	info = rfb->base.fbdev;
	rdev->fbdev_info = info;
	rfbdev = info->par;

	crtc_count = 0;
	/* okay we need to setup new connector sets in the crtcs */
	list_for_each_entry(crtc, &rdev->ddev->mode_config.crtc_list, head) {
		struct radeon_crtc *radeon_crtc = to_radeon_crtc(crtc);
		modeset = &radeon_crtc->mode_set;
		modeset->fb = &rfb->base;
		conn_count = 0;
		list_for_each_entry(connector, &rdev->ddev->mode_config.connector_list, head) {
			if (connector->encoder)
				if (connector->encoder->crtc == modeset->crtc) {
					modeset->connectors[conn_count] = connector;
					conn_count++;
					if (conn_count > RADEONFB_CONN_LIMIT)
						BUG();
				}
		}

		for (i = conn_count; i < RADEONFB_CONN_LIMIT; i++)
			modeset->connectors[i] = NULL;


		rfbdev->crtc_ids[crtc_count++] = crtc->base.id;

		modeset->num_connectors = conn_count;
		if (modeset->crtc->desired_mode) {
			if (modeset->mode) {
				drm_mode_destroy(rdev->ddev, modeset->mode);
			}
			modeset->mode = drm_mode_duplicate(rdev->ddev,
							   modeset->crtc->desired_mode);
		}
	}
	rfbdev->crtc_count = crtc_count;

	if (new_fb) {
		info->var.pixclock = -1;
//       if (register_framebuffer(info) < 0)
//           return -EINVAL;
	} else {
		radeonfb_set_par(info);
	}
	printk(KERN_INFO "fb%d: %s frame buffer device\n", info->node,
	       info->fix.id);

	/* Switch back to kernel console on panic */
//   panic_mode = *modeset;
//   atomic_notifier_chain_register(&panic_notifier_list, &paniced);
//   printk(KERN_INFO "registered panic notifier\n");
    LEAVE();

	return 0;
}

int radeonfb_probe(struct drm_device *dev)
{
	int ret;

	/* something has changed in the lower levels of hell - deal with it
	   here */

	/* two modes : a) 1 fb to rule all crtcs.
	               b) one fb per crtc.
	   two actions 1) new connected device
	               2) device removed.
	   case a/1 : if the fb surface isn't big enough - resize the surface fb.
	              if the fb size isn't big enough - resize fb into surface.
		      if everything big enough configure the new crtc/etc.
	   case a/2 : undo the configuration
	              possibly resize down the fb to fit the new configuration.
           case b/1 : see if it is on a new crtc - setup a new fb and add it.
	   case b/2 : teardown the new fb.
	*/
	ret = radeonfb_single_fb_probe(dev->dev_private);
	return ret;
}
EXPORT_SYMBOL(radeonfb_probe);

int radeonfb_remove(struct drm_device *dev, struct drm_framebuffer *fb)
{
	struct fb_info *info;
	struct radeon_framebuffer *rfb = to_radeon_framebuffer(fb);
	struct radeon_object *robj;

	if (!fb) {
		return -EINVAL;
	}
	info = fb->fbdev;
	if (info) {
		robj = rfb->obj->driver_private;
//       unregister_framebuffer(info);
//       radeon_object_kunmap(robj);
//       radeon_object_unpin(robj);
//       framebuffer_release(info);
	}

	printk(KERN_INFO "unregistered panic notifier\n");
//   atomic_notifier_chain_unregister(&panic_notifier_list, &paniced);
//   memset(&panic_mode, 0, sizeof(struct drm_mode_set));
	return 0;
}
EXPORT_SYMBOL(radeonfb_remove);


/**
 * Allocate a GEM object of the specified size with shmfs backing store
 */
struct drm_gem_object *
drm_gem_object_alloc(struct drm_device *dev, size_t size)
{
    struct drm_gem_object *obj;

    BUG_ON((size & (PAGE_SIZE - 1)) != 0);

    obj = kzalloc(sizeof(*obj), GFP_KERNEL);

    obj->dev = dev;
//    obj->filp = shmem_file_setup("drm mm object", size, VM_NORESERVE);
//    if (IS_ERR(obj->filp)) {
//        kfree(obj);
//        return NULL;
//    }

//    kref_init(&obj->refcount);
//    kref_init(&obj->handlecount);
    obj->size = size;

//    if (dev->driver->gem_init_object != NULL &&
//        dev->driver->gem_init_object(obj) != 0) {
//        fput(obj->filp);
//        kfree(obj);
//        return NULL;
//    }
//    atomic_inc(&dev->object_count);
//    atomic_add(obj->size, &dev->object_memory);
    return obj;
}


int radeon_gem_fb_object_create(struct radeon_device *rdev, int size,
                 int alignment, int initial_domain,
                 bool discardable, bool kernel,
                 bool interruptible,
                 struct drm_gem_object **obj)
{
    struct drm_gem_object *gobj;
    struct radeon_object *robj;

    *obj = NULL;
    gobj = drm_gem_object_alloc(rdev->ddev, size);
    if (!gobj) {
        return -ENOMEM;
    }
    /* At least align on page size */
    if (alignment < PAGE_SIZE) {
        alignment = PAGE_SIZE;
    }

    robj = kzalloc(sizeof(struct radeon_object), GFP_KERNEL);
    if (!robj) {
        DRM_ERROR("Failed to allocate GEM object (%d, %d, %u)\n",
              size, initial_domain, alignment);
//       mutex_lock(&rdev->ddev->struct_mutex);
//       drm_gem_object_unreference(gobj);
//       mutex_unlock(&rdev->ddev->struct_mutex);
        return -ENOMEM;;
    }
    robj->rdev = rdev;
    robj->gobj = gobj;
    INIT_LIST_HEAD(&robj->list);

    robj->flags = TTM_PL_FLAG_VRAM;

    struct drm_mm_node *vm_node;

    vm_node = kzalloc(sizeof(*vm_node),0);

    vm_node->free = 0;
    vm_node->size = 0x800000 >> 12;
    vm_node->start = 0;
    vm_node->mm = NULL;

    robj->mm_node = vm_node;

    robj->vm_addr = ((uint32_t)robj->mm_node->start);

    gobj->driver_private = robj;
    *obj = gobj;
    return 0;
}


struct fb_info *framebuffer_alloc(size_t size)
{
#define BYTES_PER_LONG (BITS_PER_LONG/8)
#define PADDING (BYTES_PER_LONG - (sizeof(struct fb_info) % BYTES_PER_LONG))
        int fb_info_size = sizeof(struct fb_info);
        struct fb_info *info;
        char *p;

        if (size)
                fb_info_size += PADDING;

        p = kzalloc(fb_info_size + size, GFP_KERNEL);

        if (!p)
                return NULL;

        info = (struct fb_info *) p;

        if (size)
                info->par = p + fb_info_size;

        return info;
#undef PADDING
#undef BYTES_PER_LONG
}

