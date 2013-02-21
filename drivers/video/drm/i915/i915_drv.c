/* i915_drv.c -- i830,i845,i855,i865,i915 driver -*- linux-c -*-
 */
/*
 *
 * Copyright 2003 Tungsten Graphics, Inc., Cedar Park, Texas.
 * All Rights Reserved.
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
 * IN NO EVENT SHALL TUNGSTEN GRAPHICS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

//#include <linux/device.h>
#include <drm/drmP.h>
#include <drm/i915_drm.h>
#include "i915_drv.h"
#include "intel_drv.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <errno-base.h>
#include <linux/pci.h>

#include <drm/drm_crtc_helper.h>

#include <syscall.h>

#define __read_mostly

int init_display_kms(struct drm_device *dev);

struct drm_device *main_device;

struct drm_file *drm_file_handlers[256];

static int i915_modeset __read_mostly = 1;
MODULE_PARM_DESC(modeset,
		"Use kernel modesetting [KMS] (0=DRM_I915_KMS from .config, "
		"1=on, -1=force vga console preference [default])");


int i915_panel_ignore_lid __read_mostly         =  0;
MODULE_PARM_DESC(panel_ignore_lid,
		"Override lid status (0=autodetect [default], 1=lid open, "
		"-1=lid closed)");

unsigned int i915_powersave  __read_mostly      =  0;
MODULE_PARM_DESC(powersave,
		"Enable powersavings, fbc, downclocking, etc. (default: true)");

int i915_semaphores __read_mostly = -1;

MODULE_PARM_DESC(semaphores,
		"Use semaphores for inter-ring sync (default: -1 (use per-chip defaults))");

int i915_enable_rc6 __read_mostly      = 0;
MODULE_PARM_DESC(i915_enable_rc6,
		"Enable power-saving render C-state 6. "
		"Different stages can be selected via bitmask values "
		"(0 = disable; 1 = enable rc6; 2 = enable deep rc6; 4 = enable deepest rc6). "
		"For example, 3 would enable rc6 and deep rc6, and 7 would enable everything. "
		"default: -1 (use per-chip default)");

int i915_enable_fbc __read_mostly      =  0;
MODULE_PARM_DESC(i915_enable_fbc,
		"Enable frame buffer compression for power savings "
		"(default: -1 (use per-chip default))");

unsigned int i915_lvds_downclock  __read_mostly =  0;
MODULE_PARM_DESC(lvds_downclock,
		"Use panel (LVDS/eDP) downclocking for power savings "
		"(default: false)");

int i915_lvds_channel_mode __read_mostly;
MODULE_PARM_DESC(lvds_channel_mode,
		 "Specify LVDS channel mode "
		 "(0=probe BIOS [default], 1=single-channel, 2=dual-channel)");

int i915_panel_use_ssc __read_mostly = -1;
MODULE_PARM_DESC(lvds_use_ssc,
		"Use Spread Spectrum Clock with panels [LVDS/eDP] "
		"(default: auto from VBT)");

int i915_vbt_sdvo_panel_type __read_mostly      = -1;
MODULE_PARM_DESC(vbt_sdvo_panel_type,
		"Override/Ignore selection of SDVO panel mode in the VBT "
		"(-2=ignore, -1=auto [default], index in VBT BIOS table)");

static bool i915_try_reset __read_mostly = true;
MODULE_PARM_DESC(reset, "Attempt GPU resets (default: true)");

bool i915_enable_hangcheck __read_mostly = false;
MODULE_PARM_DESC(enable_hangcheck,
		"Periodically check GPU activity for detecting hangs. "
		"WARNING: Disabling this can cause system wide hangs. "
		"(default: true)");

int i915_enable_ppgtt __read_mostly = false;
MODULE_PARM_DESC(i915_enable_ppgtt,
		"Enable PPGTT (default: true)");

unsigned int i915_preliminary_hw_support __read_mostly = true;
MODULE_PARM_DESC(preliminary_hw_support,
		"Enable preliminary hardware support. "
		"Enable Haswell and ValleyView Support. "
		"(default: false)");


#define PCI_VENDOR_ID_INTEL        0x8086

#define INTEL_VGA_DEVICE(id, info) {        \
	.class = PCI_BASE_CLASS_DISPLAY << 16,	\
    .class_mask = 0xff0000,                 \
    .vendor = 0x8086,                       \
    .device = id,                           \
    .subvendor = PCI_ANY_ID,                \
    .subdevice = PCI_ANY_ID,                \
    .driver_data = (unsigned long) info }


static const struct intel_device_info intel_i915g_info = {
	.gen = 3, .is_i915g = 1, .cursor_needs_physical = 1,
	.has_overlay = 1, .overlay_needs_physical = 1,
};
static const struct intel_device_info intel_i915gm_info = {
	.gen = 3, .is_mobile = 1,
	.cursor_needs_physical = 1,
	.has_overlay = 1, .overlay_needs_physical = 1,
	.supports_tv = 1,
};
static const struct intel_device_info intel_i945g_info = {
	.gen = 3, .has_hotplug = 1, .cursor_needs_physical = 1,
	.has_overlay = 1, .overlay_needs_physical = 1,
};
static const struct intel_device_info intel_i945gm_info = {
	.gen = 3, .is_i945gm = 1, .is_mobile = 1,
	.has_hotplug = 1, .cursor_needs_physical = 1,
	.has_overlay = 1, .overlay_needs_physical = 1,
	.supports_tv = 1,
};

static const struct intel_device_info intel_i965g_info = {
	.gen = 4, .is_broadwater = 1,
	.has_hotplug = 1,
	.has_overlay = 1,
};

static const struct intel_device_info intel_i965gm_info = {
	.gen = 4, .is_crestline = 1,
	.is_mobile = 1, .has_fbc = 1, .has_hotplug = 1,
	.has_overlay = 1,
	.supports_tv = 1,
};

static const struct intel_device_info intel_g33_info = {
	.gen = 3, .is_g33 = 1,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.has_overlay = 1,
};

static const struct intel_device_info intel_g45_info = {
	.gen = 4, .is_g4x = 1, .need_gfx_hws = 1,
	.has_pipe_cxsr = 1, .has_hotplug = 1,
	.has_bsd_ring = 1,
};

static const struct intel_device_info intel_gm45_info = {
	.gen = 4, .is_g4x = 1,
	.is_mobile = 1, .need_gfx_hws = 1, .has_fbc = 1,
	.has_pipe_cxsr = 1, .has_hotplug = 1,
	.supports_tv = 1,
	.has_bsd_ring = 1,
};

static const struct intel_device_info intel_pineview_info = {
	.gen = 3, .is_g33 = 1, .is_pineview = 1, .is_mobile = 1,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.has_overlay = 1,
};

static const struct intel_device_info intel_ironlake_d_info = {
	.gen = 5,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.has_bsd_ring = 1,
};

static const struct intel_device_info intel_ironlake_m_info = {
	.gen = 5, .is_mobile = 1,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.has_fbc = 1,
	.has_bsd_ring = 1,
};

static const struct intel_device_info intel_sandybridge_d_info = {
    .gen = 6,
	.need_gfx_hws = 1, .has_hotplug = 1,
    .has_bsd_ring = 1,
    .has_blt_ring = 1,
	.has_llc = 1,
	.has_force_wake = 1,
};

static const struct intel_device_info intel_sandybridge_m_info = {
	.gen = 6, .is_mobile = 1,
	.need_gfx_hws = 1, .has_hotplug = 1,
    .has_fbc      = 1,
    .has_bsd_ring = 1,
    .has_blt_ring = 1,
	.has_llc = 1,
	.has_force_wake = 1,
};

static const struct intel_device_info intel_ivybridge_d_info = {
	.is_ivybridge = 1, .gen = 7,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.has_bsd_ring = 1,
	.has_blt_ring = 1,
	.has_llc = 1,
	.has_force_wake = 1,
};

static const struct intel_device_info intel_ivybridge_m_info = {
	.is_ivybridge = 1, .gen = 7, .is_mobile = 1,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.has_fbc = 0,	/* FBC is not enabled on Ivybridge mobile yet */
	.has_bsd_ring = 1,
	.has_blt_ring = 1,
	.has_llc = 1,
	.has_force_wake = 1,
};

static const struct intel_device_info intel_valleyview_m_info = {
	.gen = 7, .is_mobile = 1,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.has_fbc = 0,
	.has_bsd_ring = 1,
	.has_blt_ring = 1,
	.is_valleyview = 1,
};

static const struct intel_device_info intel_valleyview_d_info = {
	.gen = 7,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.has_fbc = 0,
	.has_bsd_ring = 1,
	.has_blt_ring = 1,
	.is_valleyview = 1,
};

static const struct intel_device_info intel_haswell_d_info = {
	.is_haswell = 1, .gen = 7,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.has_bsd_ring = 1,
	.has_blt_ring = 1,
	.has_llc = 1,
	.has_force_wake = 1,
};

static const struct intel_device_info intel_haswell_m_info = {
	.is_haswell = 1, .gen = 7, .is_mobile = 1,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.has_bsd_ring = 1,
	.has_blt_ring = 1,
	.has_llc = 1,
	.has_force_wake = 1,
};

static const struct pci_device_id pciidlist[] = {       /* aka */
	INTEL_VGA_DEVICE(0x2582, &intel_i915g_info),		/* I915_G */
	INTEL_VGA_DEVICE(0x258a, &intel_i915g_info),		/* E7221_G */
	INTEL_VGA_DEVICE(0x2592, &intel_i915gm_info),		/* I915_GM */
	INTEL_VGA_DEVICE(0x2772, &intel_i945g_info),		/* I945_G */
	INTEL_VGA_DEVICE(0x27a2, &intel_i945gm_info),		/* I945_GM */
	INTEL_VGA_DEVICE(0x27ae, &intel_i945gm_info),		/* I945_GME */
	INTEL_VGA_DEVICE(0x2972, &intel_i965g_info),		/* I946_GZ */
	INTEL_VGA_DEVICE(0x2982, &intel_i965g_info),		/* G35_G */
	INTEL_VGA_DEVICE(0x2992, &intel_i965g_info),		/* I965_Q */
	INTEL_VGA_DEVICE(0x29a2, &intel_i965g_info),		/* I965_G */
	INTEL_VGA_DEVICE(0x29b2, &intel_g33_info),		/* Q35_G */
	INTEL_VGA_DEVICE(0x29c2, &intel_g33_info),		/* G33_G */
	INTEL_VGA_DEVICE(0x29d2, &intel_g33_info),		/* Q33_G */
	INTEL_VGA_DEVICE(0x2a02, &intel_i965gm_info),		/* I965_GM */
	INTEL_VGA_DEVICE(0x2a12, &intel_i965gm_info),		/* I965_GME */
	INTEL_VGA_DEVICE(0x2a42, &intel_gm45_info),		/* GM45_G */
	INTEL_VGA_DEVICE(0x2e02, &intel_g45_info),		/* IGD_E_G */
	INTEL_VGA_DEVICE(0x2e12, &intel_g45_info),		/* Q45_G */
	INTEL_VGA_DEVICE(0x2e22, &intel_g45_info),		/* G45_G */
	INTEL_VGA_DEVICE(0x2e32, &intel_g45_info),		/* G41_G */
	INTEL_VGA_DEVICE(0x2e42, &intel_g45_info),		/* B43_G */
	INTEL_VGA_DEVICE(0x2e92, &intel_g45_info),		/* B43_G.1 */
	INTEL_VGA_DEVICE(0xa001, &intel_pineview_info),
	INTEL_VGA_DEVICE(0xa011, &intel_pineview_info),
	INTEL_VGA_DEVICE(0x0042, &intel_ironlake_d_info),
	INTEL_VGA_DEVICE(0x0046, &intel_ironlake_m_info),
    INTEL_VGA_DEVICE(0x0102, &intel_sandybridge_d_info),
    INTEL_VGA_DEVICE(0x0112, &intel_sandybridge_d_info),
    INTEL_VGA_DEVICE(0x0122, &intel_sandybridge_d_info),
    INTEL_VGA_DEVICE(0x0106, &intel_sandybridge_m_info),
    INTEL_VGA_DEVICE(0x0116, &intel_sandybridge_m_info),
    INTEL_VGA_DEVICE(0x0126, &intel_sandybridge_m_info),
    INTEL_VGA_DEVICE(0x010A, &intel_sandybridge_d_info),
	INTEL_VGA_DEVICE(0x0156, &intel_ivybridge_m_info), /* GT1 mobile */
	INTEL_VGA_DEVICE(0x0166, &intel_ivybridge_m_info), /* GT2 mobile */
	INTEL_VGA_DEVICE(0x0152, &intel_ivybridge_d_info), /* GT1 desktop */
	INTEL_VGA_DEVICE(0x0162, &intel_ivybridge_d_info), /* GT2 desktop */
	INTEL_VGA_DEVICE(0x015a, &intel_ivybridge_d_info), /* GT1 server */
	INTEL_VGA_DEVICE(0x016a, &intel_ivybridge_d_info), /* GT2 server */
	INTEL_VGA_DEVICE(0x0402, &intel_haswell_d_info), /* GT1 desktop */
	INTEL_VGA_DEVICE(0x0412, &intel_haswell_d_info), /* GT2 desktop */
	INTEL_VGA_DEVICE(0x0422, &intel_haswell_d_info), /* GT2 desktop */
	INTEL_VGA_DEVICE(0x040a, &intel_haswell_d_info), /* GT1 server */
	INTEL_VGA_DEVICE(0x041a, &intel_haswell_d_info), /* GT2 server */
	INTEL_VGA_DEVICE(0x042a, &intel_haswell_d_info), /* GT2 server */
	INTEL_VGA_DEVICE(0x0406, &intel_haswell_m_info), /* GT1 mobile */
	INTEL_VGA_DEVICE(0x0416, &intel_haswell_m_info), /* GT2 mobile */
	INTEL_VGA_DEVICE(0x0426, &intel_haswell_m_info), /* GT2 mobile */
	INTEL_VGA_DEVICE(0x0C02, &intel_haswell_d_info), /* SDV GT1 desktop */
	INTEL_VGA_DEVICE(0x0C12, &intel_haswell_d_info), /* SDV GT2 desktop */
	INTEL_VGA_DEVICE(0x0C22, &intel_haswell_d_info), /* SDV GT2 desktop */
	INTEL_VGA_DEVICE(0x0C0A, &intel_haswell_d_info), /* SDV GT1 server */
	INTEL_VGA_DEVICE(0x0C1A, &intel_haswell_d_info), /* SDV GT2 server */
	INTEL_VGA_DEVICE(0x0C2A, &intel_haswell_d_info), /* SDV GT2 server */
	INTEL_VGA_DEVICE(0x0C06, &intel_haswell_m_info), /* SDV GT1 mobile */
	INTEL_VGA_DEVICE(0x0C16, &intel_haswell_m_info), /* SDV GT2 mobile */
	INTEL_VGA_DEVICE(0x0C26, &intel_haswell_m_info), /* SDV GT2 mobile */
	INTEL_VGA_DEVICE(0x0A02, &intel_haswell_d_info), /* ULT GT1 desktop */
	INTEL_VGA_DEVICE(0x0A12, &intel_haswell_d_info), /* ULT GT2 desktop */
	INTEL_VGA_DEVICE(0x0A22, &intel_haswell_d_info), /* ULT GT2 desktop */
	INTEL_VGA_DEVICE(0x0A0A, &intel_haswell_d_info), /* ULT GT1 server */
	INTEL_VGA_DEVICE(0x0A1A, &intel_haswell_d_info), /* ULT GT2 server */
	INTEL_VGA_DEVICE(0x0A2A, &intel_haswell_d_info), /* ULT GT2 server */
	INTEL_VGA_DEVICE(0x0A06, &intel_haswell_m_info), /* ULT GT1 mobile */
	INTEL_VGA_DEVICE(0x0A16, &intel_haswell_m_info), /* ULT GT2 mobile */
	INTEL_VGA_DEVICE(0x0A26, &intel_haswell_m_info), /* ULT GT2 mobile */
	INTEL_VGA_DEVICE(0x0D12, &intel_haswell_d_info), /* CRW GT1 desktop */
	INTEL_VGA_DEVICE(0x0D22, &intel_haswell_d_info), /* CRW GT2 desktop */
	INTEL_VGA_DEVICE(0x0D32, &intel_haswell_d_info), /* CRW GT2 desktop */
	INTEL_VGA_DEVICE(0x0D1A, &intel_haswell_d_info), /* CRW GT1 server */
	INTEL_VGA_DEVICE(0x0D2A, &intel_haswell_d_info), /* CRW GT2 server */
	INTEL_VGA_DEVICE(0x0D3A, &intel_haswell_d_info), /* CRW GT2 server */
	INTEL_VGA_DEVICE(0x0D16, &intel_haswell_m_info), /* CRW GT1 mobile */
	INTEL_VGA_DEVICE(0x0D26, &intel_haswell_m_info), /* CRW GT2 mobile */
	INTEL_VGA_DEVICE(0x0D36, &intel_haswell_m_info), /* CRW GT2 mobile */
	INTEL_VGA_DEVICE(0x0f30, &intel_valleyview_m_info),
	INTEL_VGA_DEVICE(0x0157, &intel_valleyview_m_info),
	INTEL_VGA_DEVICE(0x0155, &intel_valleyview_d_info),
    {0, 0, 0}
};

#define INTEL_PCH_DEVICE_ID_MASK        0xff00
#define INTEL_PCH_IBX_DEVICE_ID_TYPE    0x3b00
#define INTEL_PCH_CPT_DEVICE_ID_TYPE    0x1c00
#define INTEL_PCH_PPT_DEVICE_ID_TYPE    0x1e00
#define INTEL_PCH_LPT_DEVICE_ID_TYPE	0x8c00

void intel_detect_pch(struct drm_device *dev)
{
    struct drm_i915_private *dev_priv = dev->dev_private;
    struct pci_dev *pch;

    /*
     * The reason to probe ISA bridge instead of Dev31:Fun0 is to
     * make graphics device passthrough work easy for VMM, that only
     * need to expose ISA bridge to let driver know the real hardware
     * underneath. This is a requirement from virtualization team.
     */
    pch = pci_get_class(PCI_CLASS_BRIDGE_ISA << 8, NULL);
    if (pch) {
        if (pch->vendor == PCI_VENDOR_ID_INTEL) {
			unsigned short id;
            id = pch->device & INTEL_PCH_DEVICE_ID_MASK;
			dev_priv->pch_id = id;

            if (id == INTEL_PCH_IBX_DEVICE_ID_TYPE) {
                dev_priv->pch_type = PCH_IBX;
				dev_priv->num_pch_pll = 2;
                DRM_DEBUG_KMS("Found Ibex Peak PCH\n");
				WARN_ON(!IS_GEN5(dev));
            } else if (id == INTEL_PCH_CPT_DEVICE_ID_TYPE) {
                dev_priv->pch_type = PCH_CPT;
				dev_priv->num_pch_pll = 2;
                DRM_DEBUG_KMS("Found CougarPoint PCH\n");
				WARN_ON(!(IS_GEN6(dev) || IS_IVYBRIDGE(dev)));
            } else if (id == INTEL_PCH_PPT_DEVICE_ID_TYPE) {
                /* PantherPoint is CPT compatible */
                dev_priv->pch_type = PCH_CPT;
				dev_priv->num_pch_pll = 2;
                DRM_DEBUG_KMS("Found PatherPoint PCH\n");
				WARN_ON(!(IS_GEN6(dev) || IS_IVYBRIDGE(dev)));
			} else if (id == INTEL_PCH_LPT_DEVICE_ID_TYPE) {
				dev_priv->pch_type = PCH_LPT;
				dev_priv->num_pch_pll = 0;
				DRM_DEBUG_KMS("Found LynxPoint PCH\n");
				WARN_ON(!IS_HASWELL(dev));
			} else if (id == INTEL_PCH_LPT_LP_DEVICE_ID_TYPE) {
				dev_priv->pch_type = PCH_LPT;
				dev_priv->num_pch_pll = 0;
				DRM_DEBUG_KMS("Found LynxPoint LP PCH\n");
				WARN_ON(!IS_HASWELL(dev));
            }
			BUG_ON(dev_priv->num_pch_pll > I915_NUM_PLLS);
        }
    }
}

bool i915_semaphore_is_enabled(struct drm_device *dev)
{
	if (INTEL_INFO(dev)->gen < 6)
		return 0;

	if (i915_semaphores >= 0)
		return i915_semaphores;

#ifdef CONFIG_INTEL_IOMMU
	/* Enable semaphores on SNB when IO remapping is off */
	if (INTEL_INFO(dev)->gen == 6 && intel_iommu_gfx_mapped)
		return false;
#endif

	return 1;
}





int drm_get_dev(struct pci_dev *pdev, const struct pci_device_id *ent);

int i915_init(void)
{
    static pci_dev_t device;
    const struct pci_device_id  *ent;
    int  err;

    ent = find_pci_device(&device, pciidlist);
    if( unlikely(ent == NULL) )
    {
        dbgprintf("device not found\n");
        return 0;
    };

    struct intel_device_info *intel_info =
        (struct intel_device_info *) ent->driver_data;

	if (intel_info->is_valleyview)
        if(!i915_preliminary_hw_support) {
            DRM_ERROR("Preliminary hardware support disabled\n");
            return -ENODEV;
        }

    DRM_INFO("device %x:%x\n", device.pci_dev.vendor,
                                device.pci_dev.device);

    if (intel_info->gen != 3) {

    } else if (init_agp() != 0) {
        DRM_ERROR("drm/i915 can't work without intel_agp module!\n");
        return -ENODEV;
    }

    err = drm_get_dev(&device.pci_dev, ent);

    return err;
}



static struct drm_driver driver = {
    /* Don't use MTRRs here; the Xserver or userspace app should
     * deal with them for Intel hardware.
     */
//    .driver_features =
//        DRIVER_USE_AGP | DRIVER_REQUIRE_AGP | /* DRIVER_USE_MTRR |*/
//        DRIVER_HAVE_IRQ | DRIVER_IRQ_SHARED | DRIVER_GEM | DRIVER_PRIME,
//    .load = i915_driver_load,
//    .unload = i915_driver_unload,
//    .open = i915_driver_open,
//    .lastclose = i915_driver_lastclose,
//    .preclose = i915_driver_preclose,
//    .postclose = i915_driver_postclose,

    /* Used in place of i915_pm_ops for non-DRIVER_MODESET */
//    .suspend = i915_suspend,
//    .resume = i915_resume,

//    .device_is_agp = i915_driver_device_is_agp,
//    .master_create = i915_master_create,
//    .master_destroy = i915_master_destroy,
    .gem_init_object = i915_gem_init_object,
    .gem_free_object = i915_gem_free_object,
//    .gem_vm_ops = &i915_gem_vm_ops,

//    .prime_handle_to_fd = drm_gem_prime_handle_to_fd,
//    .prime_fd_to_handle = drm_gem_prime_fd_to_handle,
//    .gem_prime_export = i915_gem_prime_export,
//    .gem_prime_import = i915_gem_prime_import,

//    .dumb_create = i915_gem_dumb_create,
//    .dumb_map_offset = i915_gem_mmap_gtt,
//    .dumb_destroy = i915_gem_dumb_destroy,
//    .ioctls = i915_ioctls,
//    .fops = &i915_driver_fops,
//    .name = DRIVER_NAME,
//    .desc = DRIVER_DESC,
//    .date = DRIVER_DATE,
//    .major = DRIVER_MAJOR,
//    .minor = DRIVER_MINOR,
//    .patchlevel = DRIVER_PATCHLEVEL,
};


int drm_get_dev(struct pci_dev *pdev, const struct pci_device_id *ent)
{
    static struct drm_device drm_dev;
    static struct drm_file   drm_file;

    struct drm_device *dev;
    struct drm_file   *priv;

    int ret;

    dev  = &drm_dev;
    priv = &drm_file;

    drm_file_handlers[0] = priv;

 //   ret = pci_enable_device(pdev);
 //   if (ret)
 //       goto err_g1;

    pci_set_master(pdev);

 //   if ((ret = drm_fill_in_dev(dev, pdev, ent, driver))) {
 //       printk(KERN_ERR "DRM: Fill_in_dev failed.\n");
 //       goto err_g2;
 //   }

    dev->pdev = pdev;
    dev->pci_device = pdev->device;
    dev->pci_vendor = pdev->vendor;

    INIT_LIST_HEAD(&dev->filelist);
    INIT_LIST_HEAD(&dev->ctxlist);
    INIT_LIST_HEAD(&dev->vmalist);
    INIT_LIST_HEAD(&dev->maplist);

    spin_lock_init(&dev->count_lock);
    mutex_init(&dev->struct_mutex);
    mutex_init(&dev->ctxlist_mutex);

    INIT_LIST_HEAD(&priv->lhead);
    INIT_LIST_HEAD(&priv->fbs);
    INIT_LIST_HEAD(&priv->event_list);
    init_waitqueue_head(&priv->event_wait);
    priv->event_space = 4096; /* set aside 4k for event buffer */

    idr_init(&priv->object_idr);
    spin_lock_init(&priv->table_lock);

    dev->driver = &driver;

    ret = i915_driver_load(dev, ent->driver_data );

    if (ret)
        goto err_g4;

    ret = init_display_kms(dev);

    if (ret)
        goto err_g4;

    return 0;

err_g4:
//err_g3:
//    if (drm_core_check_feature(dev, DRIVER_MODESET))
//        drm_put_minor(&dev->control);
//err_g2:
//    pci_disable_device(pdev);
//err_g1:

    return ret;
}

/* We give fast paths for the really cool registers */
#define NEEDS_FORCE_WAKE(dev_priv, reg) \
	((HAS_FORCE_WAKE((dev_priv)->dev)) && \
	 ((reg) < 0x40000) &&            \
	 ((reg) != FORCEWAKE))

static bool IS_DISPLAYREG(u32 reg)
{
	/*
	 * This should make it easier to transition modules over to the
	 * new register block scheme, since we can do it incrementally.
	 */
	if (reg >= VLV_DISPLAY_BASE)
		return false;

	if (reg >= RENDER_RING_BASE &&
	    reg < RENDER_RING_BASE + 0xff)
		return false;
	if (reg >= GEN6_BSD_RING_BASE &&
	    reg < GEN6_BSD_RING_BASE + 0xff)
		return false;
	if (reg >= BLT_RING_BASE &&
	    reg < BLT_RING_BASE + 0xff)
		return false;

	if (reg == PGTBL_ER)
		return false;

	if (reg >= IPEIR_I965 &&
	    reg < HWSTAM)
		return false;

	if (reg == MI_MODE)
		return false;

	if (reg == GFX_MODE_GEN7)
		return false;

	if (reg == RENDER_HWS_PGA_GEN7 ||
	    reg == BSD_HWS_PGA_GEN7 ||
	    reg == BLT_HWS_PGA_GEN7)
		return false;

	if (reg == GEN6_BSD_SLEEP_PSMI_CONTROL ||
	    reg == GEN6_BSD_RNCID)
		return false;

	if (reg == GEN6_BLITTER_ECOSKPD)
		return false;

	if (reg >= 0x4000c &&
	    reg <= 0x4002c)
		return false;

	if (reg >= 0x4f000 &&
	    reg <= 0x4f08f)
		return false;

	if (reg >= 0x4f100 &&
	    reg <= 0x4f11f)
		return false;

	if (reg >= VLV_MASTER_IER &&
	    reg <= GEN6_PMIER)
		return false;

	if (reg >= FENCE_REG_SANDYBRIDGE_0 &&
	    reg < (FENCE_REG_SANDYBRIDGE_0 + (16*8)))
		return false;

	if (reg >= VLV_IIR_RW &&
	    reg <= VLV_ISR)
		return false;

	if (reg == FORCEWAKE_VLV ||
	    reg == FORCEWAKE_ACK_VLV)
		return false;

	if (reg == GEN6_GDRST)
		return false;

	switch (reg) {
	case _3D_CHICKEN3:
	case IVB_CHICKEN3:
	case GEN7_COMMON_SLICE_CHICKEN1:
	case GEN7_L3CNTLREG1:
	case GEN7_L3_CHICKEN_MODE_REGISTER:
	case GEN7_ROW_CHICKEN2:
	case GEN7_L3SQCREG4:
	case GEN7_SQ_CHICKEN_MBCUNIT_CONFIG:
	case GEN7_HALF_SLICE_CHICKEN1:
	case GEN6_MBCTL:
	case GEN6_UCGCTL2:
		return false;
	default:
		break;
	}

	return true;
}

static void
ilk_dummy_write(struct drm_i915_private *dev_priv)
{
	/* WaIssueDummyWriteToWakeupFromRC6: Issue a dummy write to wake up the
	 * chip from rc6 before touching it for real. MI_MODE is masked, hence
	 * harmless to write 0 into. */
	I915_WRITE_NOTRACE(MI_MODE, 0);
}

#define __i915_read(x, y) \
u##x i915_read##x(struct drm_i915_private *dev_priv, u32 reg) { \
	u##x val = 0; \
	if (IS_GEN5(dev_priv->dev)) \
		ilk_dummy_write(dev_priv); \
	if (NEEDS_FORCE_WAKE((dev_priv), (reg))) { \
		unsigned long irqflags; \
		spin_lock_irqsave(&dev_priv->gt_lock, irqflags); \
		if (dev_priv->forcewake_count == 0) \
			dev_priv->gt.force_wake_get(dev_priv); \
		val = read##y(dev_priv->regs + reg); \
		if (dev_priv->forcewake_count == 0) \
			dev_priv->gt.force_wake_put(dev_priv); \
		spin_unlock_irqrestore(&dev_priv->gt_lock, irqflags); \
	} else if (IS_VALLEYVIEW(dev_priv->dev) && IS_DISPLAYREG(reg)) { \
		val = read##y(dev_priv->regs + reg + 0x180000);		\
	} else { \
		val = read##y(dev_priv->regs + reg); \
	} \
	return val; \
}

__i915_read(8, b)
__i915_read(16, w)
__i915_read(32, l)
__i915_read(64, q)
#undef __i915_read

#define __i915_write(x, y) \
void i915_write##x(struct drm_i915_private *dev_priv, u32 reg, u##x val) { \
	u32 __fifo_ret = 0; \
	trace_i915_reg_rw(true, reg, val, sizeof(val)); \
	if (NEEDS_FORCE_WAKE((dev_priv), (reg))) { \
		__fifo_ret = __gen6_gt_wait_for_fifo(dev_priv); \
	} \
	if (IS_GEN5(dev_priv->dev)) \
		ilk_dummy_write(dev_priv); \
	if (IS_HASWELL(dev_priv->dev) && (I915_READ_NOTRACE(GEN7_ERR_INT) & ERR_INT_MMIO_UNCLAIMED)) { \
		DRM_ERROR("Unknown unclaimed register before writing to %x\n", reg); \
		I915_WRITE_NOTRACE(GEN7_ERR_INT, ERR_INT_MMIO_UNCLAIMED); \
	} \
	if (IS_VALLEYVIEW(dev_priv->dev) && IS_DISPLAYREG(reg)) { \
		write##y(val, dev_priv->regs + reg + 0x180000);		\
	} else {							\
	write##y(val, dev_priv->regs + reg); \
	}								\
	if (unlikely(__fifo_ret)) { \
		gen6_gt_check_fifodbg(dev_priv); \
	} \
	if (IS_HASWELL(dev_priv->dev) && (I915_READ_NOTRACE(GEN7_ERR_INT) & ERR_INT_MMIO_UNCLAIMED)) { \
		DRM_ERROR("Unclaimed write to %x\n", reg); \
		writel(ERR_INT_MMIO_UNCLAIMED, dev_priv->regs + GEN7_ERR_INT);	\
	} \
}
__i915_write(8, b)
__i915_write(16, w)
__i915_write(32, l)
__i915_write(64, q)
#undef __i915_write
