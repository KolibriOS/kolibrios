/*
 * Copyright 2008 Advanced Micro Devices, Inc.
 * Copyright 2008 Red Hat Inc.
 * Copyright 2009 Jerome Glisse.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Dave Airlie
 *          Alex Deucher
 *          Jerome Glisse
 */
//#include <linux/console.h>

#include <drmP.h>
//#include <drm/drm_crtc_helper.h>
#include "radeon_drm.h"
#include "radeon_reg.h"
#include "radeon.h"
#include "radeon_asic.h"
#include "atom.h"

#include <syscall.h>

int radeon_modeset = -1;
int radeon_dynclks = -1;
int radeon_r4xx_atom = 0;
int radeon_agpmode = 0;
int radeon_vram_limit = 0;
int radeon_gart_size = 512; /* default gart size */
int radeon_benchmarking = 0;
int radeon_connector_table = 0;


/*
 * Clear GPU surface registers.
 */
static void radeon_surface_init(struct radeon_device *rdev)
{
    dbgprintf("%s\n",__FUNCTION__);

    /* FIXME: check this out */
    if (rdev->family < CHIP_R600) {
        int i;

        for (i = 0; i < 8; i++) {
            WREG32(RADEON_SURFACE0_INFO +
                   i * (RADEON_SURFACE1_INFO - RADEON_SURFACE0_INFO),
                   0);
        }
    }
}

/*
 * GPU scratch registers helpers function.
 */
static void radeon_scratch_init(struct radeon_device *rdev)
{
    int i;

    /* FIXME: check this out */
    if (rdev->family < CHIP_R300) {
        rdev->scratch.num_reg = 5;
    } else {
        rdev->scratch.num_reg = 7;
    }
    for (i = 0; i < rdev->scratch.num_reg; i++) {
        rdev->scratch.free[i] = true;
        rdev->scratch.reg[i] = RADEON_SCRATCH_REG0 + (i * 4);
    }
}

int radeon_scratch_get(struct radeon_device *rdev, uint32_t *reg)
{
	int i;

	for (i = 0; i < rdev->scratch.num_reg; i++) {
		if (rdev->scratch.free[i]) {
			rdev->scratch.free[i] = false;
			*reg = rdev->scratch.reg[i];
			return 0;
		}
	}
	return -EINVAL;
}

void radeon_scratch_free(struct radeon_device *rdev, uint32_t reg)
{
	int i;

	for (i = 0; i < rdev->scratch.num_reg; i++) {
		if (rdev->scratch.reg[i] == reg) {
			rdev->scratch.free[i] = true;
			return;
		}
	}
}

/*
 * MC common functions
 */
int radeon_mc_setup(struct radeon_device *rdev)
{
	uint32_t tmp;

	/* Some chips have an "issue" with the memory controller, the
	 * location must be aligned to the size. We just align it down,
	 * too bad if we walk over the top of system memory, we don't
	 * use DMA without a remapped anyway.
	 * Affected chips are rv280, all r3xx, and all r4xx, but not IGP
	 */
	/* FGLRX seems to setup like this, VRAM a 0, then GART.
	 */
/*
	 * Note: from R6xx the address space is 40bits but here we only
	 * use 32bits (still have to see a card which would exhaust 4G
	 * address space).
	 */
	if (rdev->mc.vram_location != 0xFFFFFFFFUL) {
		/* vram location was already setup try to put gtt after
		 * if it fits */
		tmp = rdev->mc.vram_location + rdev->mc.vram_size;
		tmp = (tmp + rdev->mc.gtt_size - 1) & ~(rdev->mc.gtt_size - 1);
		if ((0xFFFFFFFFUL - tmp) >= rdev->mc.gtt_size) {
			rdev->mc.gtt_location = tmp;
		} else {
			if (rdev->mc.gtt_size >= rdev->mc.vram_location) {
				printk(KERN_ERR "[drm] GTT too big to fit "
				       "before or after vram location.\n");
				return -EINVAL;
			}
			rdev->mc.gtt_location = 0;
		}
	} else if (rdev->mc.gtt_location != 0xFFFFFFFFUL) {
		/* gtt location was already setup try to put vram before
		 * if it fits */
		if (rdev->mc.vram_size < rdev->mc.gtt_location) {
			rdev->mc.vram_location = 0;
		} else {
			tmp = rdev->mc.gtt_location + rdev->mc.gtt_size;
			tmp += (rdev->mc.vram_size - 1);
			tmp &= ~(rdev->mc.vram_size - 1);
			if ((0xFFFFFFFFUL - tmp) >= rdev->mc.vram_size) {
				rdev->mc.vram_location = tmp;
			} else {
				printk(KERN_ERR "[drm] vram too big to fit "
				       "before or after GTT location.\n");
				return -EINVAL;
			}
		}
	} else {
		rdev->mc.vram_location = 0;
		rdev->mc.gtt_location = rdev->mc.vram_size;
	}
	DRM_INFO("radeon: VRAM %uM\n", rdev->mc.vram_size >> 20);
	DRM_INFO("radeon: VRAM from 0x%08X to 0x%08X\n",
		 rdev->mc.vram_location,
		 rdev->mc.vram_location + rdev->mc.vram_size - 1);
	DRM_INFO("radeon: GTT %uM\n", rdev->mc.gtt_size >> 20);
	DRM_INFO("radeon: GTT from 0x%08X to 0x%08X\n",
		 rdev->mc.gtt_location,
		 rdev->mc.gtt_location + rdev->mc.gtt_size - 1);
	return 0;
}


/*
 * GPU helpers function.
 */
static bool radeon_card_posted(struct radeon_device *rdev)
{
	uint32_t reg;

    dbgprintf("%s\n",__FUNCTION__);

	/* first check CRTCs */
	if (ASIC_IS_AVIVO(rdev)) {
		reg = RREG32(AVIVO_D1CRTC_CONTROL) |
		      RREG32(AVIVO_D2CRTC_CONTROL);
		if (reg & AVIVO_CRTC_EN) {
			return true;
		}
	} else {
		reg = RREG32(RADEON_CRTC_GEN_CNTL) |
		      RREG32(RADEON_CRTC2_GEN_CNTL);
		if (reg & RADEON_CRTC_EN) {
			return true;
		}
	}

	/* then check MEM_SIZE, in case the crtcs are off */
	if (rdev->family >= CHIP_R600)
		reg = RREG32(R600_CONFIG_MEMSIZE);
	else
		reg = RREG32(RADEON_CONFIG_MEMSIZE);

	if (reg)
		return true;

	return false;

}


/*
 * Registers accessors functions.
 */
uint32_t radeon_invalid_rreg(struct radeon_device *rdev, uint32_t reg)
{
    DRM_ERROR("Invalid callback to read register 0x%04X\n", reg);
    BUG_ON(1);
    return 0;
}

void radeon_invalid_wreg(struct radeon_device *rdev, uint32_t reg, uint32_t v)
{
    DRM_ERROR("Invalid callback to write register 0x%04X with 0x%08X\n",
          reg, v);
    BUG_ON(1);
}

void radeon_register_accessor_init(struct radeon_device *rdev)
{

    dbgprintf("%s\n",__FUNCTION__);

    rdev->mm_rreg = &r100_mm_rreg;
    rdev->mm_wreg = &r100_mm_wreg;
    rdev->mc_rreg = &radeon_invalid_rreg;
    rdev->mc_wreg = &radeon_invalid_wreg;
    rdev->pll_rreg = &radeon_invalid_rreg;
    rdev->pll_wreg = &radeon_invalid_wreg;
    rdev->pcie_rreg = &radeon_invalid_rreg;
    rdev->pcie_wreg = &radeon_invalid_wreg;
    rdev->pciep_rreg = &radeon_invalid_rreg;
    rdev->pciep_wreg = &radeon_invalid_wreg;

    /* Don't change order as we are overridding accessor. */
    if (rdev->family < CHIP_RV515) {
//        rdev->pcie_rreg = &rv370_pcie_rreg;
//        rdev->pcie_wreg = &rv370_pcie_wreg;
    }
    if (rdev->family >= CHIP_RV515) {
        rdev->pcie_rreg = &rv515_pcie_rreg;
        rdev->pcie_wreg = &rv515_pcie_wreg;
    }
    /* FIXME: not sure here */
    if (rdev->family <= CHIP_R580) {
        rdev->pll_rreg = &r100_pll_rreg;
        rdev->pll_wreg = &r100_pll_wreg;
    }
    if (rdev->family >= CHIP_RV515) {
        rdev->mc_rreg = &rv515_mc_rreg;
        rdev->mc_wreg = &rv515_mc_wreg;
    }
    if (rdev->family == CHIP_RS400 || rdev->family == CHIP_RS480) {
//        rdev->mc_rreg = &rs400_mc_rreg;
//        rdev->mc_wreg = &rs400_mc_wreg;
    }
    if (rdev->family == CHIP_RS690 || rdev->family == CHIP_RS740) {
//        rdev->mc_rreg = &rs690_mc_rreg;
//        rdev->mc_wreg = &rs690_mc_wreg;
    }
    if (rdev->family == CHIP_RS600) {
//        rdev->mc_rreg = &rs600_mc_rreg;
//        rdev->mc_wreg = &rs600_mc_wreg;
    }
    if (rdev->family >= CHIP_R600) {
//        rdev->pciep_rreg = &r600_pciep_rreg;
//        rdev->pciep_wreg = &r600_pciep_wreg;
    }
}



/*
 * ASIC
 */
int radeon_asic_init(struct radeon_device *rdev)
{

    dbgprintf("%s\n",__FUNCTION__);

    radeon_register_accessor_init(rdev);
	switch (rdev->family) {
	case CHIP_R100:
	case CHIP_RV100:
	case CHIP_RS100:
	case CHIP_RV200:
	case CHIP_RS200:
	case CHIP_R200:
	case CHIP_RV250:
	case CHIP_RS300:
	case CHIP_RV280:
//       rdev->asic = &r100_asic;
		break;
	case CHIP_R300:
	case CHIP_R350:
	case CHIP_RV350:
	case CHIP_RV380:
//       rdev->asic = &r300_asic;
		break;
	case CHIP_R420:
	case CHIP_R423:
	case CHIP_RV410:
//       rdev->asic = &r420_asic;
		break;
	case CHIP_RS400:
	case CHIP_RS480:
//       rdev->asic = &rs400_asic;
		break;
	case CHIP_RS600:
//       rdev->asic = &rs600_asic;
		break;
	case CHIP_RS690:
	case CHIP_RS740:
//       rdev->asic = &rs690_asic;
		break;
	case CHIP_RV515:
//       rdev->asic = &rv515_asic;
		break;
	case CHIP_R520:
	case CHIP_RV530:
	case CHIP_RV560:
	case CHIP_RV570:
	case CHIP_R580:
        rdev->asic = &r520_asic;
		break;
	case CHIP_R600:
	case CHIP_RV610:
	case CHIP_RV630:
	case CHIP_RV620:
	case CHIP_RV635:
	case CHIP_RV670:
	case CHIP_RS780:
	case CHIP_RV770:
	case CHIP_RV730:
	case CHIP_RV710:
	default:
		/* FIXME: not supported yet */
		return -EINVAL;
	}
	return 0;
}


/*
 * Wrapper around modesetting bits.
 */
int radeon_clocks_init(struct radeon_device *rdev)
{
	int r;

    dbgprintf("%s\n",__FUNCTION__);

    radeon_get_clock_info(rdev->ddev);
    r = radeon_static_clocks_init(rdev->ddev);
	if (r) {
		return r;
	}
	DRM_INFO("Clocks initialized !\n");
	return 0;
}

void radeon_clocks_fini(struct radeon_device *rdev)
{
}

/* ATOM accessor methods */
static uint32_t cail_pll_read(struct card_info *info, uint32_t reg)
{
    struct radeon_device *rdev = info->dev->dev_private;
    uint32_t r;

    r = rdev->pll_rreg(rdev, reg);
    return r;
}

static void cail_pll_write(struct card_info *info, uint32_t reg, uint32_t val)
{
    struct radeon_device *rdev = info->dev->dev_private;

    rdev->pll_wreg(rdev, reg, val);
}

static uint32_t cail_mc_read(struct card_info *info, uint32_t reg)
{
    struct radeon_device *rdev = info->dev->dev_private;
    uint32_t r;

    r = rdev->mc_rreg(rdev, reg);
    return r;
}

static void cail_mc_write(struct card_info *info, uint32_t reg, uint32_t val)
{
    struct radeon_device *rdev = info->dev->dev_private;

    rdev->mc_wreg(rdev, reg, val);
}

static void cail_reg_write(struct card_info *info, uint32_t reg, uint32_t val)
{
    struct radeon_device *rdev = info->dev->dev_private;

    WREG32(reg*4, val);
}

static uint32_t cail_reg_read(struct card_info *info, uint32_t reg)
{
    struct radeon_device *rdev = info->dev->dev_private;
    uint32_t r;

    r = RREG32(reg*4);
    return r;
}

static struct card_info atom_card_info = {
    .dev = NULL,
    .reg_read = cail_reg_read,
    .reg_write = cail_reg_write,
    .mc_read = cail_mc_read,
    .mc_write = cail_mc_write,
    .pll_read = cail_pll_read,
    .pll_write = cail_pll_write,
};

int radeon_atombios_init(struct radeon_device *rdev)
{
    dbgprintf("%s\n",__FUNCTION__);

    atom_card_info.dev = rdev->ddev;
    rdev->mode_info.atom_context = atom_parse(&atom_card_info, rdev->bios);
    radeon_atom_initialize_bios_scratch_regs(rdev->ddev);
    return 0;
}

void radeon_atombios_fini(struct radeon_device *rdev)
{
	kfree(rdev->mode_info.atom_context);
}

int radeon_combios_init(struct radeon_device *rdev)
{
//	radeon_combios_initialize_bios_scratch_regs(rdev->ddev);
	return 0;
}

void radeon_combios_fini(struct radeon_device *rdev)
{
}

int radeon_modeset_init(struct radeon_device *rdev);
void radeon_modeset_fini(struct radeon_device *rdev);

/*
 * Radeon device.
 */
int radeon_device_init(struct radeon_device *rdev,
               struct drm_device *ddev,
               struct pci_dev *pdev,
               uint32_t flags)
{
    int r, ret = -1;

    dbgprintf("%s\n",__FUNCTION__);

    DRM_INFO("radeon: Initializing kernel modesetting.\n");
    rdev->shutdown = false;
    rdev->ddev = ddev;
    rdev->pdev = pdev;
    rdev->flags = flags;
    rdev->family = flags & RADEON_FAMILY_MASK;
    rdev->is_atom_bios = false;
    rdev->usec_timeout = RADEON_MAX_USEC_TIMEOUT;
    rdev->mc.gtt_size = radeon_gart_size * 1024 * 1024;
    rdev->gpu_lockup = false;
    /* mutex initialization are all done here so we
     * can recall function without having locking issues */
 //   mutex_init(&rdev->cs_mutex);
 //   mutex_init(&rdev->ib_pool.mutex);
 //   mutex_init(&rdev->cp.mutex);
 //   rwlock_init(&rdev->fence_drv.lock);


    if (radeon_agpmode == -1) {
        rdev->flags &= ~RADEON_IS_AGP;
        if (rdev->family > CHIP_RV515 ||
            rdev->family == CHIP_RV380 ||
            rdev->family == CHIP_RV410 ||
            rdev->family == CHIP_R423) {
            DRM_INFO("Forcing AGP to PCIE mode\n");
            rdev->flags |= RADEON_IS_PCIE;
        } else {
            DRM_INFO("Forcing AGP to PCI mode\n");
            rdev->flags |= RADEON_IS_PCI;
        }
    }

    /* Set asic functions */
    r = radeon_asic_init(rdev);
    if (r) {
        return r;
    }
//    r = radeon_init(rdev);

    r = rdev->asic->init(rdev);

    if (r) {
        return r;
    }

    /* Report DMA addressing limitation */
    r = pci_set_dma_mask(rdev->pdev, DMA_BIT_MASK(32));
    if (r) {
        printk(KERN_WARNING "radeon: No suitable DMA available.\n");
    }

    /* Registers mapping */
    /* TODO: block userspace mapping of io register */
    rdev->rmmio_base = pci_resource_start(rdev->pdev, 2);

    rdev->rmmio_size = pci_resource_len(rdev->pdev, 2);

    rdev->rmmio =  (void*)MapIoMem(rdev->rmmio_base, rdev->rmmio_size,
                                   PG_SW+PG_NOCACHE);

    if (rdev->rmmio == NULL) {
        return -ENOMEM;
    }
    DRM_INFO("register mmio base: 0x%08X\n", (uint32_t)rdev->rmmio_base);
    DRM_INFO("register mmio size: %u\n", (unsigned)rdev->rmmio_size);

    /* Setup errata flags */
    radeon_errata(rdev);
    /* Initialize scratch registers */
    radeon_scratch_init(rdev);
	/* Initialize surface registers */
    radeon_surface_init(rdev);

    /* TODO: disable VGA need to use VGA request */
    /* BIOS*/
    if (!radeon_get_bios(rdev)) {
        if (ASIC_IS_AVIVO(rdev))
            return -EINVAL;
    }
    if (rdev->is_atom_bios) {
        r = radeon_atombios_init(rdev);
        if (r) {
            return r;
        }
    } else {
        r = radeon_combios_init(rdev);
        if (r) {
            return r;
        }
    }
    /* Reset gpu before posting otherwise ATOM will enter infinite loop */
    if (radeon_gpu_reset(rdev)) {
        /* FIXME: what do we want to do here ? */
    }
    /* check if cards are posted or not */
    if (!radeon_card_posted(rdev) && rdev->bios) {
        DRM_INFO("GPU not posted. posting now...\n");
        if (rdev->is_atom_bios) {
            atom_asic_init(rdev->mode_info.atom_context);
        } else {
    //        radeon_combios_asic_init(rdev->ddev);
        }
    }

    /* Get vram informations */
    radeon_vram_info(rdev);
    /* Device is severly broken if aper size > vram size.
     * for RN50/M6/M7 - Novell bug 204882 ?
     */
    if (rdev->mc.vram_size < rdev->mc.aper_size) {
        rdev->mc.aper_size = rdev->mc.vram_size;
    }
    /* Add an MTRR for the VRAM */
//    rdev->mc.vram_mtrr = mtrr_add(rdev->mc.aper_base, rdev->mc.aper_size,
//                      MTRR_TYPE_WRCOMB, 1);
    DRM_INFO("Detected VRAM RAM=%uM, BAR=%uM\n",
         rdev->mc.vram_size >> 20,
         (unsigned)rdev->mc.aper_size >> 20);
    DRM_INFO("RAM width %dbits %cDR\n",
         rdev->mc.vram_width, rdev->mc.vram_is_ddr ? 'D' : 'S');

    /* Initialize clocks */
    r = radeon_clocks_init(rdev);
    if (r) {
        return r;
    }

    /* Initialize memory controller (also test AGP) */
    r = radeon_mc_init(rdev);
    if (r) {
        return r;
    };


    /* Fence driver */
//    r = radeon_fence_driver_init(rdev);
//    if (r) {
//        return r;
//    }
//    r = radeon_irq_kms_init(rdev);
//    if (r) {
//        return r;
//    }
    /* Memory manager */
    r = radeon_object_init(rdev);
    if (r) {
        return r;
    }
    /* Initialize GART (initialize after TTM so we can allocate
     * memory through TTM but finalize after TTM) */
    r = radeon_gart_enable(rdev);
//    if (!r) {
//        r = radeon_gem_init(rdev);
//    }

    /* 1M ring buffer */
    if (!r) {
        r = radeon_cp_init(rdev, 1024 * 1024);
    }
    if (!r) {
        r = radeon_wb_init(rdev);
        if (r) {
            DRM_ERROR("radeon: failled initializing WB (%d).\n", r);
            return r;
        }
    }

    if (!r) {
        r = radeon_ib_pool_init(rdev);
        if (r) {
            DRM_ERROR("radeon: failled initializing IB pool (%d).\n", r);
            return r;
        }
    }
#if 0

    if (!r) {
        r = radeon_ib_test(rdev);
        if (r) {
            DRM_ERROR("radeon: failled testing IB (%d).\n", r);
            return r;
        }
    }
    ret = r;
    r = radeon_modeset_init(rdev);
    if (r) {
        return r;
    }
    if (rdev->fbdev_rfb && rdev->fbdev_rfb->obj) {
        rdev->fbdev_robj = rdev->fbdev_rfb->obj->driver_private;
    }
    if (!ret) {
        DRM_INFO("radeon: kernel modesetting successfully initialized.\n");
    }
//    if (radeon_benchmarking) {
//        radeon_benchmark(rdev);
//    }

#endif

    return ret;
}

static struct pci_device_id pciidlist[] = {
    radeon_PCI_IDS
};


u32_t __stdcall drvEntry(int action)
{
    struct pci_device_id  *ent;

    dev_t   device;
    int     err;
    u32_t   retval = 0;

    if(action != 1)
        return 0;

    if(!dbg_open("/hd0/2/atikms.log"))
    {
        printf("Can't open /hd0/2/atikms.log\nExit\n");
        return 0;
    }

    enum_pci_devices();

    ent = find_pci_device(&device, pciidlist);

    if( unlikely(ent == NULL) )
    {
        dbgprintf("device not found\n");
        return 0;
    };

    dbgprintf("device %x:%x\n", device.pci_dev.vendor,
                                device.pci_dev.device);

    err = drm_get_dev(&device.pci_dev, ent);

    return retval;
};

/*
static struct drm_driver kms_driver = {
    .driver_features =
        DRIVER_USE_AGP | DRIVER_USE_MTRR | DRIVER_PCI_DMA | DRIVER_SG |
        DRIVER_HAVE_IRQ | DRIVER_HAVE_DMA | DRIVER_IRQ_SHARED | DRIVER_GEM,
    .dev_priv_size = 0,
    .load = radeon_driver_load_kms,
    .firstopen = radeon_driver_firstopen_kms,
    .open = radeon_driver_open_kms,
    .preclose = radeon_driver_preclose_kms,
    .postclose = radeon_driver_postclose_kms,
    .lastclose = radeon_driver_lastclose_kms,
    .unload = radeon_driver_unload_kms,
    .suspend = radeon_suspend_kms,
    .resume = radeon_resume_kms,
    .get_vblank_counter = radeon_get_vblank_counter_kms,
    .enable_vblank = radeon_enable_vblank_kms,
    .disable_vblank = radeon_disable_vblank_kms,
    .master_create = radeon_master_create_kms,
    .master_destroy = radeon_master_destroy_kms,
#if defined(CONFIG_DEBUG_FS)
    .debugfs_init = radeon_debugfs_init,
    .debugfs_cleanup = radeon_debugfs_cleanup,
#endif
    .irq_preinstall = radeon_driver_irq_preinstall_kms,
    .irq_postinstall = radeon_driver_irq_postinstall_kms,
    .irq_uninstall = radeon_driver_irq_uninstall_kms,
    .irq_handler = radeon_driver_irq_handler_kms,
    .reclaim_buffers = drm_core_reclaim_buffers,
    .get_map_ofs = drm_core_get_map_ofs,
    .get_reg_ofs = drm_core_get_reg_ofs,
    .ioctls = radeon_ioctls_kms,
    .gem_init_object = radeon_gem_object_init,
    .gem_free_object = radeon_gem_object_free,
    .dma_ioctl = radeon_dma_ioctl_kms,
    .fops = {
         .owner = THIS_MODULE,
         .open = drm_open,
         .release = drm_release,
         .ioctl = drm_ioctl,
         .mmap = radeon_mmap,
         .poll = drm_poll,
         .fasync = drm_fasync,
#ifdef CONFIG_COMPAT
         .compat_ioctl = NULL,
#endif
    },

    .pci_driver = {
         .name = DRIVER_NAME,
         .id_table = pciidlist,
         .probe = radeon_pci_probe,
         .remove = radeon_pci_remove,
         .suspend = radeon_pci_suspend,
         .resume = radeon_pci_resume,
    },

    .name = DRIVER_NAME,
    .desc = DRIVER_DESC,
    .date = DRIVER_DATE,
    .major = KMS_DRIVER_MAJOR,
    .minor = KMS_DRIVER_MINOR,
    .patchlevel = KMS_DRIVER_PATCHLEVEL,
};
*/


/*
 * Driver load/unload
 */
int radeon_driver_load_kms(struct drm_device *dev, unsigned long flags)
{
    struct radeon_device *rdev;
    int r;

    dbgprintf("%s\n",__FUNCTION__);

    rdev = kzalloc(sizeof(struct radeon_device), GFP_KERNEL);
    if (rdev == NULL) {
        return -ENOMEM;
    };

    dev->dev_private = (void *)rdev;

    /* update BUS flag */
//    if (drm_device_is_agp(dev)) {
        flags |= RADEON_IS_AGP;
//    } else if (drm_device_is_pcie(dev)) {
//        flags |= RADEON_IS_PCIE;
//    } else {
//        flags |= RADEON_IS_PCI;
//    }

    r = radeon_device_init(rdev, dev, dev->pdev, flags);
    if (r) {
        dbgprintf("Failed to initialize Radeon, disabling IOCTL\n");
//        radeon_device_fini(rdev);
        return r;
    }
    return 0;
}

int drm_get_dev(struct pci_dev *pdev, const struct pci_device_id *ent)
{
    struct drm_device *dev;
    int ret;

    dbgprintf("%s\n",__FUNCTION__);

    dev = malloc(sizeof(*dev));
    if (!dev)
        return -ENOMEM;

 //   ret = pci_enable_device(pdev);
 //   if (ret)
 //       goto err_g1;

 //   pci_set_master(pdev);

 //   if ((ret = drm_fill_in_dev(dev, pdev, ent, driver))) {
 //       printk(KERN_ERR "DRM: Fill_in_dev failed.\n");
 //       goto err_g2;
 //   }

    dev->pdev = pdev;
    dev->pci_device = pdev->device;
    dev->pci_vendor = pdev->vendor;

 //   if (drm_core_check_feature(dev, DRIVER_MODESET)) {
 //       pci_set_drvdata(pdev, dev);
 //       ret = drm_get_minor(dev, &dev->control, DRM_MINOR_CONTROL);
 //       if (ret)
 //           goto err_g2;
 //   }

 //   if ((ret = drm_get_minor(dev, &dev->primary, DRM_MINOR_LEGACY)))
 //       goto err_g3;

 //   if (dev->driver->load) {
 //       ret = dev->driver->load(dev, ent->driver_data);
 //       if (ret)
 //           goto err_g4;
 //   }

      ret = radeon_driver_load_kms(dev, ent->driver_data );
      if (ret)
        goto err_g4;

 //   list_add_tail(&dev->driver_item, &driver->device_list);

 //   DRM_INFO("Initialized %s %d.%d.%d %s for %s on minor %d\n",
 //        driver->name, driver->major, driver->minor, driver->patchlevel,
 //        driver->date, pci_name(pdev), dev->primary->index);

    return 0;

err_g4:
//    drm_put_minor(&dev->primary);
//err_g3:
//    if (drm_core_check_feature(dev, DRIVER_MODESET))
//        drm_put_minor(&dev->control);
//err_g2:
//    pci_disable_device(pdev);
//err_g1:
    free(dev);

    return ret;
}

resource_size_t drm_get_resource_start(struct drm_device *dev, unsigned int resource)
{
    return pci_resource_start(dev->pdev, resource);
}

resource_size_t drm_get_resource_len(struct drm_device *dev, unsigned int resource)
{
    return pci_resource_len(dev->pdev, resource);
}


uint32_t __div64_32(uint64_t *n, uint32_t base)
{
        uint64_t rem = *n;
        uint64_t b = base;
        uint64_t res, d = 1;
        uint32_t high = rem >> 32;

        /* Reduce the thing a bit first */
        res = 0;
        if (high >= base) {
                high /= base;
                res = (uint64_t) high << 32;
                rem -= (uint64_t) (high*base) << 32;
        }

        while ((int64_t)b > 0 && b < rem) {
                b = b+b;
                d = d+d;
        }

        do {
                if (rem >= b) {
                        rem -= b;
                        res += d;
                }
                b >>= 1;
                d >>= 1;
        } while (d);

        *n = res;
        return rem;
}

