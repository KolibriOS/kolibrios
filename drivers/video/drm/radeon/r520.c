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
//#include "drmP.h"
#include "radeon_reg.h"
#include "radeon.h"

/* r520,rv530,rv560,rv570,r580 depends on : */
void r100_hdp_reset(struct radeon_device *rdev);
int rv370_pcie_gart_enable(struct radeon_device *rdev);
void rv370_pcie_gart_disable(struct radeon_device *rdev);
void r420_pipes_init(struct radeon_device *rdev);
void rs600_mc_disable_clients(struct radeon_device *rdev);
void rs600_disable_vga(struct radeon_device *rdev);
int rv515_debugfs_pipes_info_init(struct radeon_device *rdev);
int rv515_debugfs_ga_info_init(struct radeon_device *rdev);

/* This files gather functions specifics to:
 * r520,rv530,rv560,rv570,r580
 *
 * Some of these functions might be used by newer ASICs.
 */
void r520_gpu_init(struct radeon_device *rdev);
int r520_mc_wait_for_idle(struct radeon_device *rdev);

/*
 * MC
 */
int r520_mc_init(struct radeon_device *rdev)
{
	uint32_t tmp;
	int r;

    dbgprintf("%s\n",__FUNCTION__);

//   if (r100_debugfs_rbbm_init(rdev)) {
//       DRM_ERROR("Failed to register debugfs file for RBBM !\n");
//   }
//   if (rv515_debugfs_pipes_info_init(rdev)) {
//       DRM_ERROR("Failed to register debugfs file for pipes !\n");
//   }
//   if (rv515_debugfs_ga_info_init(rdev)) {
//       DRM_ERROR("Failed to register debugfs file for pipes !\n");
//   }

	r520_gpu_init(rdev);
	rv370_pcie_gart_disable(rdev);

	/* Setup GPU memory space */
	rdev->mc.vram_location = 0xFFFFFFFFUL;
	rdev->mc.gtt_location = 0xFFFFFFFFUL;
	if (rdev->flags & RADEON_IS_AGP) {
		r = radeon_agp_init(rdev);
		if (r) {
			printk(KERN_WARNING "[drm] Disabling AGP\n");
			rdev->flags &= ~RADEON_IS_AGP;
			rdev->mc.gtt_size = radeon_gart_size * 1024 * 1024;
		} else {
			rdev->mc.gtt_location = rdev->mc.agp_base;
		}
	}
	r = radeon_mc_setup(rdev);
	if (r) {
		return r;
	}

	/* Program GPU memory space */
    rs600_mc_disable_clients(rdev);
    if (r520_mc_wait_for_idle(rdev)) {
       printk(KERN_WARNING "Failed to wait MC idle while "
		       "programming pipes. Bad things might happen.\n");
	}
	/* Write VRAM size in case we are limiting it */
	WREG32(RADEON_CONFIG_MEMSIZE, rdev->mc.vram_size);
	tmp = rdev->mc.vram_location + rdev->mc.vram_size - 1;
	tmp = REG_SET(R520_MC_FB_TOP, tmp >> 16);
	tmp |= REG_SET(R520_MC_FB_START, rdev->mc.vram_location >> 16);
	WREG32_MC(R520_MC_FB_LOCATION, tmp);
	WREG32(RS690_HDP_FB_LOCATION, rdev->mc.vram_location >> 16);
	WREG32(0x310, rdev->mc.vram_location);
	if (rdev->flags & RADEON_IS_AGP) {
		tmp = rdev->mc.gtt_location + rdev->mc.gtt_size - 1;
		tmp = REG_SET(R520_MC_AGP_TOP, tmp >> 16);
		tmp |= REG_SET(R520_MC_AGP_START, rdev->mc.gtt_location >> 16);
		WREG32_MC(R520_MC_AGP_LOCATION, tmp);
		WREG32_MC(R520_MC_AGP_BASE, rdev->mc.agp_base);
		WREG32_MC(R520_MC_AGP_BASE_2, 0);
	} else {
		WREG32_MC(R520_MC_AGP_LOCATION, 0x0FFFFFFF);
		WREG32_MC(R520_MC_AGP_BASE, 0);
		WREG32_MC(R520_MC_AGP_BASE_2, 0);
	}

    dbgprintf("done: %s\n",__FUNCTION__);

	return 0;
}

void r520_mc_fini(struct radeon_device *rdev)
{
	rv370_pcie_gart_disable(rdev);
	radeon_gart_table_vram_free(rdev);
	radeon_gart_fini(rdev);
}


/*
 * Global GPU functions
 */
void r520_errata(struct radeon_device *rdev)
{
	rdev->pll_errata = 0;
}

int r520_mc_wait_for_idle(struct radeon_device *rdev)
{
	unsigned i;
	uint32_t tmp;

	for (i = 0; i < rdev->usec_timeout; i++) {
		/* read MC_STATUS */
		tmp = RREG32_MC(R520_MC_STATUS);
		if (tmp & R520_MC_STATUS_IDLE) {
			return 0;
		}
		DRM_UDELAY(1);
	}
	return -1;
}

void r520_gpu_init(struct radeon_device *rdev)
{
	unsigned pipe_select_current, gb_pipe_select, tmp;
    dbgprintf("%s\n\r",__FUNCTION__);

	r100_hdp_reset(rdev);
	rs600_disable_vga(rdev);
	/*
	 * DST_PIPE_CONFIG		0x170C
	 * GB_TILE_CONFIG		0x4018
	 * GB_FIFO_SIZE			0x4024
	 * GB_PIPE_SELECT		0x402C
	 * GB_PIPE_SELECT2              0x4124
	 *	Z_PIPE_SHIFT			0
	 *	Z_PIPE_MASK			0x000000003
	 * GB_FIFO_SIZE2                0x4128
	 *	SC_SFIFO_SIZE_SHIFT		0
	 *	SC_SFIFO_SIZE_MASK		0x000000003
	 *	SC_MFIFO_SIZE_SHIFT		2
	 *	SC_MFIFO_SIZE_MASK		0x00000000C
	 *	FG_SFIFO_SIZE_SHIFT		4
	 *	FG_SFIFO_SIZE_MASK		0x000000030
	 *	ZB_MFIFO_SIZE_SHIFT		6
	 *	ZB_MFIFO_SIZE_MASK		0x0000000C0
	 * GA_ENHANCE			0x4274
	 * SU_REG_DEST			0x42C8
	 */
	/* workaround for RV530 */
	if (rdev->family == CHIP_RV530) {
		WREG32(0x4124, 1);
		WREG32(0x4128, 0xFF);
	}
	r420_pipes_init(rdev);
	gb_pipe_select = RREG32(0x402C);
	tmp = RREG32(0x170C);
	pipe_select_current = (tmp >> 2) & 3;
	tmp = (1 << pipe_select_current) |
	      (((gb_pipe_select >> 8) & 0xF) << 4);
	WREG32_PLL(0x000D, tmp);
	if (r520_mc_wait_for_idle(rdev)) {
		printk(KERN_WARNING "Failed to wait MC idle while "
		       "programming pipes. Bad things might happen.\n");
	}
}


/*
 * VRAM info
 */
static void r520_vram_get_type(struct radeon_device *rdev)
{
	uint32_t tmp;
    dbgprintf("%s\n\r",__FUNCTION__);

	rdev->mc.vram_width = 128;
	rdev->mc.vram_is_ddr = true;
	tmp = RREG32_MC(R520_MC_CNTL0);
	switch ((tmp & R520_MEM_NUM_CHANNELS_MASK) >> R520_MEM_NUM_CHANNELS_SHIFT) {
	case 0:
		rdev->mc.vram_width = 32;
		break;
	case 1:
		rdev->mc.vram_width = 64;
		break;
	case 2:
		rdev->mc.vram_width = 128;
		break;
	case 3:
		rdev->mc.vram_width = 256;
		break;
	default:
		rdev->mc.vram_width = 128;
		break;
	}
	if (tmp & R520_MC_CHANNEL_SIZE)
		rdev->mc.vram_width *= 2;
}

void r520_vram_info(struct radeon_device *rdev)
{
	r520_vram_get_type(rdev);
	rdev->mc.vram_size = RREG32(RADEON_CONFIG_MEMSIZE);

	rdev->mc.aper_base = drm_get_resource_start(rdev->ddev, 0);
	rdev->mc.aper_size = drm_get_resource_len(rdev->ddev, 0);
}

/*
 * Global GPU functions
 */
void rs600_disable_vga(struct radeon_device *rdev)
{
    unsigned tmp;
    dbgprintf("%s\n\r",__FUNCTION__);

    WREG32(0x330, 0);
    WREG32(0x338, 0);
    tmp = RREG32(0x300);
    tmp &= ~(3 << 16);
    WREG32(0x300, tmp);
    WREG32(0x308, (1 << 8));
    WREG32(0x310, rdev->mc.vram_location);
    WREG32(0x594, 0);
}


void r420_pipes_init(struct radeon_device *rdev)
{
    unsigned tmp;
    unsigned gb_pipe_select;
    unsigned num_pipes;

    dbgprintf("%s\n\r",__FUNCTION__);

    /* GA_ENHANCE workaround TCL deadlock issue */
    WREG32(0x4274, (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3));
    /* get max number of pipes */
    gb_pipe_select = RREG32(0x402C);
    num_pipes = ((gb_pipe_select >> 12) & 3) + 1;
    rdev->num_gb_pipes = num_pipes;
    tmp = 0;
    switch (num_pipes) {
    default:
        /* force to 1 pipe */
        num_pipes = 1;
    case 1:
        tmp = (0 << 1);
        break;
    case 2:
        tmp = (3 << 1);
        break;
    case 3:
        tmp = (6 << 1);
        break;
    case 4:
        tmp = (7 << 1);
        break;
    }
    WREG32(0x42C8, (1 << num_pipes) - 1);
    /* Sub pixel 1/12 so we can have 4K rendering according to doc */
    tmp |= (1 << 4) | (1 << 0);
    WREG32(0x4018, tmp);
    if (r100_gui_wait_for_idle(rdev)) {
        printk(KERN_WARNING "Failed to wait GUI idle while "
               "programming pipes. Bad things might happen.\n");
    }

    tmp = RREG32(0x170C);
    WREG32(0x170C, tmp | (1 << 31));

    WREG32(R300_RB2D_DSTCACHE_MODE,
           RREG32(R300_RB2D_DSTCACHE_MODE) |
           R300_DC_AUTOFLUSH_ENABLE |
           R300_DC_DC_DISABLE_IGNORE_PE);

    if (r100_gui_wait_for_idle(rdev)) {
        printk(KERN_WARNING "Failed to wait GUI idle while "
               "programming pipes. Bad things might happen.\n");
    }
    DRM_INFO("radeon: %d pipes initialized.\n", rdev->num_gb_pipes);
}

void rv370_pcie_gart_disable(struct radeon_device *rdev)
{
    uint32_t tmp;
    dbgprintf("%s\n\r",__FUNCTION__);

    tmp = RREG32_PCIE(RADEON_PCIE_TX_GART_CNTL);
    tmp |= RADEON_PCIE_TX_GART_UNMAPPED_ACCESS_DISCARD;
    WREG32_PCIE(RADEON_PCIE_TX_GART_CNTL, tmp & ~RADEON_PCIE_TX_GART_EN);
    if (rdev->gart.table.vram.robj) {
//        radeon_object_kunmap(rdev->gart.table.vram.robj);
//        radeon_object_unpin(rdev->gart.table.vram.robj);
    }
}

void radeon_gart_table_vram_free(struct radeon_device *rdev)
{
    if (rdev->gart.table.vram.robj == NULL) {
        return;
    }
//    radeon_object_kunmap(rdev->gart.table.vram.robj);
//    radeon_object_unpin(rdev->gart.table.vram.robj);
//    radeon_object_unref(&rdev->gart.table.vram.robj);
}

/*
 * Common gart functions.
 */
void radeon_gart_unbind(struct radeon_device *rdev, unsigned offset,
            int pages)
{
    unsigned t;
    unsigned p;
    int i, j;
    dbgprintf("%s\n\r",__FUNCTION__);

    if (!rdev->gart.ready) {
        dbgprintf("trying to unbind memory to unitialized GART !\n");
        return;
    }
    t = offset / 4096;
    p = t / (PAGE_SIZE / 4096);
    for (i = 0; i < pages; i++, p++) {
        if (rdev->gart.pages[p]) {
//            pci_unmap_page(rdev->pdev, rdev->gart.pages_addr[p],
//                       PAGE_SIZE, PCI_DMA_BIDIRECTIONAL);
            rdev->gart.pages[p] = NULL;
            rdev->gart.pages_addr[p] = 0;
            for (j = 0; j < (PAGE_SIZE / 4096); j++, t++) {
                radeon_gart_set_page(rdev, t, 0);
            }
        }
    }
    mb();
    radeon_gart_tlb_flush(rdev);
}



void radeon_gart_fini(struct radeon_device *rdev)
{
    if (rdev->gart.pages && rdev->gart.pages_addr && rdev->gart.ready) {
        /* unbind pages */
        radeon_gart_unbind(rdev, 0, rdev->gart.num_cpu_pages);
    }
    rdev->gart.ready = false;
//    kfree(rdev->gart.pages);
//    kfree(rdev->gart.pages_addr);
    rdev->gart.pages = NULL;
    rdev->gart.pages_addr = NULL;
}



int radeon_agp_init(struct radeon_device *rdev)
{

    dbgprintf("%s\n\r",__FUNCTION__);

#if __OS_HAS_AGP
    struct radeon_agpmode_quirk *p = radeon_agpmode_quirk_list;
    struct drm_agp_mode mode;
    struct drm_agp_info info;
    uint32_t agp_status;
    int default_mode;
    bool is_v3;
    int ret;

    /* Acquire AGP. */
    if (!rdev->ddev->agp->acquired) {
        ret = drm_agp_acquire(rdev->ddev);
        if (ret) {
            DRM_ERROR("Unable to acquire AGP: %d\n", ret);
            return ret;
        }
    }

    ret = drm_agp_info(rdev->ddev, &info);
    if (ret) {
        DRM_ERROR("Unable to get AGP info: %d\n", ret);
        return ret;
    }
    mode.mode = info.mode;
    agp_status = (RREG32(RADEON_AGP_STATUS) | RADEON_AGPv3_MODE) & mode.mode;
    is_v3 = !!(agp_status & RADEON_AGPv3_MODE);

    if (is_v3) {
        default_mode = (agp_status & RADEON_AGPv3_8X_MODE) ? 8 : 4;
    } else {
        if (agp_status & RADEON_AGP_4X_MODE) {
            default_mode = 4;
        } else if (agp_status & RADEON_AGP_2X_MODE) {
            default_mode = 2;
        } else {
            default_mode = 1;
        }
    }

    /* Apply AGPMode Quirks */
    while (p && p->chip_device != 0) {
        if (info.id_vendor == p->hostbridge_vendor &&
            info.id_device == p->hostbridge_device &&
            rdev->pdev->vendor == p->chip_vendor &&
            rdev->pdev->device == p->chip_device &&
            rdev->pdev->subsystem_vendor == p->subsys_vendor &&
            rdev->pdev->subsystem_device == p->subsys_device) {
            default_mode = p->default_mode;
        }
        ++p;
    }

    if (radeon_agpmode > 0) {
        if ((radeon_agpmode < (is_v3 ? 4 : 1)) ||
            (radeon_agpmode > (is_v3 ? 8 : 4)) ||
            (radeon_agpmode & (radeon_agpmode - 1))) {
            DRM_ERROR("Illegal AGP Mode: %d (valid %s), leaving at %d\n",
                  radeon_agpmode, is_v3 ? "4, 8" : "1, 2, 4",
                  default_mode);
            radeon_agpmode = default_mode;
        } else {
            DRM_INFO("AGP mode requested: %d\n", radeon_agpmode);
        }
    } else {
        radeon_agpmode = default_mode;
    }

    mode.mode &= ~RADEON_AGP_MODE_MASK;
    if (is_v3) {
        switch (radeon_agpmode) {
        case 8:
            mode.mode |= RADEON_AGPv3_8X_MODE;
            break;
        case 4:
        default:
            mode.mode |= RADEON_AGPv3_4X_MODE;
            break;
        }
    } else {
        switch (radeon_agpmode) {
        case 4:
            mode.mode |= RADEON_AGP_4X_MODE;
            break;
        case 2:
            mode.mode |= RADEON_AGP_2X_MODE;
            break;
        case 1:
        default:
            mode.mode |= RADEON_AGP_1X_MODE;
            break;
        }
    }

    mode.mode &= ~RADEON_AGP_FW_MODE; /* disable fw */
    ret = drm_agp_enable(rdev->ddev, mode);
    if (ret) {
        DRM_ERROR("Unable to enable AGP (mode = 0x%lx)\n", mode.mode);
        return ret;
    }

    rdev->mc.agp_base = rdev->ddev->agp->agp_info.aper_base;
    rdev->mc.gtt_size = rdev->ddev->agp->agp_info.aper_size << 20;

    /* workaround some hw issues */
    if (rdev->family < CHIP_R200) {
        WREG32(RADEON_AGP_CNTL, RREG32(RADEON_AGP_CNTL) | 0x000e0000);
    }
    return 0;
#else
    return 0;
#endif
}


void rs600_mc_disable_clients(struct radeon_device *rdev)
{
    unsigned tmp;
    dbgprintf("%s\n",__FUNCTION__);

    if (r100_gui_wait_for_idle(rdev)) {
        printk(KERN_WARNING "Failed to wait GUI idle while "
               "programming pipes. Bad things might happen.\n");
    }

    tmp = RREG32(AVIVO_D1VGA_CONTROL);
    WREG32(AVIVO_D1VGA_CONTROL, tmp & ~AVIVO_DVGA_CONTROL_MODE_ENABLE);
    tmp = RREG32(AVIVO_D2VGA_CONTROL);
    WREG32(AVIVO_D2VGA_CONTROL, tmp & ~AVIVO_DVGA_CONTROL_MODE_ENABLE);

    tmp = RREG32(AVIVO_D1CRTC_CONTROL);
    WREG32(AVIVO_D1CRTC_CONTROL, tmp & ~AVIVO_CRTC_EN);
    tmp = RREG32(AVIVO_D2CRTC_CONTROL);
    WREG32(AVIVO_D2CRTC_CONTROL, tmp & ~AVIVO_CRTC_EN);

    /* make sure all previous write got through */
    tmp = RREG32(AVIVO_D2CRTC_CONTROL);

    mdelay(1);

    dbgprintf("done\n");

}

int rv370_pcie_gart_set_page(struct radeon_device *rdev, int i, uint64_t addr)
{
    void __iomem *ptr = (void *)rdev->gart.table.vram.ptr;

    if (i < 0 || i > rdev->gart.num_gpu_pages) {
        return -EINVAL;
    }
    addr = (((u32_t)addr) >> 8) | ((upper_32_bits(addr) & 0xff) << 4) | 0xC;
    writel(cpu_to_le32(addr), ((void __iomem *)ptr) + (i * 4));
    return 0;
}


int radeon_gart_init(struct radeon_device *rdev)
{

    dbgprintf("%s\n",__FUNCTION__);

    if (rdev->gart.pages) {
        return 0;
    }
    /* We need PAGE_SIZE >= 4096 */
    if (PAGE_SIZE < 4096) {
        DRM_ERROR("Page size is smaller than GPU page size!\n");
        return -EINVAL;
    }
    /* Compute table size */
    rdev->gart.num_cpu_pages = rdev->mc.gtt_size / PAGE_SIZE;
    rdev->gart.num_gpu_pages = rdev->mc.gtt_size / 4096;
    DRM_INFO("GART: num cpu pages %u, num gpu pages %u\n",
         rdev->gart.num_cpu_pages, rdev->gart.num_gpu_pages);
    /* Allocate pages table */
    rdev->gart.pages = kzalloc(sizeof(void *) * rdev->gart.num_cpu_pages,
                   GFP_KERNEL);
    if (rdev->gart.pages == NULL) {
//        radeon_gart_fini(rdev);
        return -ENOMEM;
    }
    rdev->gart.pages_addr = kzalloc(sizeof(u32_t) *
                    rdev->gart.num_cpu_pages, GFP_KERNEL);
    if (rdev->gart.pages_addr == NULL) {
//        radeon_gart_fini(rdev);
        return -ENOMEM;
    }
    return 0;
}

int radeon_gart_table_vram_alloc(struct radeon_device *rdev)
{
    uint32_t gpu_addr;
    int r;

//    if (rdev->gart.table.vram.robj == NULL) {
//        r = radeon_object_create(rdev, NULL,
//                     rdev->gart.table_size,
//                     true,
//                     RADEON_GEM_DOMAIN_VRAM,
//                     false, &rdev->gart.table.vram.robj);
//        if (r) {
//            return r;
//        }
//    }
//    r = radeon_object_pin(rdev->gart.table.vram.robj,
//                  RADEON_GEM_DOMAIN_VRAM, &gpu_addr);
//    if (r) {
//        radeon_object_unref(&rdev->gart.table.vram.robj);
//        return r;
//    }
//    r = radeon_object_kmap(rdev->gart.table.vram.robj,
//                   (void **)&rdev->gart.table.vram.ptr);
//    if (r) {
//        radeon_object_unpin(rdev->gart.table.vram.robj);
//        radeon_object_unref(&rdev->gart.table.vram.robj);
//        DRM_ERROR("radeon: failed to map gart vram table.\n");
//        return r;
//    }

    gpu_addr = 0x800000;

    u32_t pci_addr = rdev->mc.aper_base + gpu_addr;

    rdev->gart.table.vram.ptr = (void*)MapIoMem(pci_addr, rdev->gart.table_size, PG_SW);

    rdev->gart.table_addr = gpu_addr;

    dbgprintf("alloc gart vram:\n  gpu_base %x pci_base %x lin_addr %x",
               gpu_addr, pci_addr, rdev->gart.table.vram.ptr);

    return 0;
}

void rv370_pcie_gart_tlb_flush(struct radeon_device *rdev);

int rv370_pcie_gart_enable(struct radeon_device *rdev)
{
    uint32_t table_addr;
    uint32_t tmp;
    int r;

    dbgprintf("%s\n",__FUNCTION__);

    /* Initialize common gart structure */
    r = radeon_gart_init(rdev);
    if (r) {
        return r;
    }
 //   r = rv370_debugfs_pcie_gart_info_init(rdev);
 //   if (r) {
 //       DRM_ERROR("Failed to register debugfs file for PCIE gart !\n");
 //   }
    rdev->gart.table_size = rdev->gart.num_gpu_pages * 4;
    r = radeon_gart_table_vram_alloc(rdev);
    if (r) {
        return r;
    }
    /* discard memory request outside of configured range */
    tmp = RADEON_PCIE_TX_GART_UNMAPPED_ACCESS_DISCARD;
    WREG32_PCIE(RADEON_PCIE_TX_GART_CNTL, tmp);
    WREG32_PCIE(RADEON_PCIE_TX_GART_START_LO, rdev->mc.gtt_location);
    tmp = rdev->mc.gtt_location + rdev->mc.gtt_size - 4096;
    WREG32_PCIE(RADEON_PCIE_TX_GART_END_LO, tmp);
    WREG32_PCIE(RADEON_PCIE_TX_GART_START_HI, 0);
    WREG32_PCIE(RADEON_PCIE_TX_GART_END_HI, 0);
    table_addr = rdev->gart.table_addr;
    WREG32_PCIE(RADEON_PCIE_TX_GART_BASE, table_addr);
    /* FIXME: setup default page */
    WREG32_PCIE(RADEON_PCIE_TX_DISCARD_RD_ADDR_LO, rdev->mc.vram_location);
    WREG32_PCIE(RADEON_PCIE_TX_DISCARD_RD_ADDR_HI, 0);
    /* Clear error */
    WREG32_PCIE(0x18, 0);
    tmp = RREG32_PCIE(RADEON_PCIE_TX_GART_CNTL);
    tmp |= RADEON_PCIE_TX_GART_EN;
    tmp |= RADEON_PCIE_TX_GART_UNMAPPED_ACCESS_DISCARD;
    WREG32_PCIE(RADEON_PCIE_TX_GART_CNTL, tmp);
    rv370_pcie_gart_tlb_flush(rdev);
    DRM_INFO("PCIE GART of %uM enabled (table at 0x%08X).\n",
         rdev->mc.gtt_size >> 20, table_addr);
    rdev->gart.ready = true;
    return 0;
}

void rv370_pcie_gart_tlb_flush(struct radeon_device *rdev)
{
    uint32_t tmp;
    int i;

    /* Workaround HW bug do flush 2 times */
    for (i = 0; i < 2; i++) {
        tmp = RREG32_PCIE(RADEON_PCIE_TX_GART_CNTL);
        WREG32_PCIE(RADEON_PCIE_TX_GART_CNTL, tmp | RADEON_PCIE_TX_GART_INVALIDATE_TLB);
        (void)RREG32_PCIE(RADEON_PCIE_TX_GART_CNTL);
        WREG32_PCIE(RADEON_PCIE_TX_GART_CNTL, tmp);
        mb();
    }
}

int r300_gart_enable(struct radeon_device *rdev)
{
#if __OS_HAS_AGP
    if (rdev->flags & RADEON_IS_AGP) {
        if (rdev->family > CHIP_RV350) {
            rv370_pcie_gart_disable(rdev);
        } else {
            r100_pci_gart_disable(rdev);
        }
        return 0;
    }
#endif
    if (rdev->flags & RADEON_IS_PCIE) {
        rdev->asic->gart_disable = &rv370_pcie_gart_disable;
        rdev->asic->gart_tlb_flush = &rv370_pcie_gart_tlb_flush;
        rdev->asic->gart_set_page = &rv370_pcie_gart_set_page;
        return rv370_pcie_gart_enable(rdev);
    }
 //   return r100_pci_gart_enable(rdev);
}



int radeon_fence_driver_init(struct radeon_device *rdev)
{
    unsigned long irq_flags;
    int r;

//    write_lock_irqsave(&rdev->fence_drv.lock, irq_flags);
    r = radeon_scratch_get(rdev, &rdev->fence_drv.scratch_reg);
    if (r) {
        DRM_ERROR("Fence failed to get a scratch register.");
//        write_unlock_irqrestore(&rdev->fence_drv.lock, irq_flags);
        return r;
    }
    WREG32(rdev->fence_drv.scratch_reg, 0);
//    atomic_set(&rdev->fence_drv.seq, 0);
//    INIT_LIST_HEAD(&rdev->fence_drv.created);
//    INIT_LIST_HEAD(&rdev->fence_drv.emited);
//    INIT_LIST_HEAD(&rdev->fence_drv.signaled);
    rdev->fence_drv.count_timeout = 0;
//    init_waitqueue_head(&rdev->fence_drv.queue);
//    write_unlock_irqrestore(&rdev->fence_drv.lock, irq_flags);
//    if (radeon_debugfs_fence_init(rdev)) {
//        DRM_ERROR("Failed to register debugfs file for fence !\n");
//    }
    return 0;
}


int radeon_gart_bind(struct radeon_device *rdev, unsigned offset,
             int pages, u32_t *pagelist)
{
    unsigned t;
    unsigned p;
    uint64_t page_base;
    int i, j;

    dbgprintf("%s\n\r",__FUNCTION__);


    if (!rdev->gart.ready) {
        DRM_ERROR("trying to bind memory to unitialized GART !\n");
        return -EINVAL;
    }
    t = offset / 4096;
    p = t / (PAGE_SIZE / 4096);

    for (i = 0; i < pages; i++, p++) {
        /* we need to support large memory configurations */
        /* assume that unbind have already been call on the range */

        rdev->gart.pages_addr[p] = pagelist[i] & ~4095;

        //if (pci_dma_mapping_error(rdev->pdev, rdev->gart.pages_addr[p])) {
        //    /* FIXME: failed to map page (return -ENOMEM?) */
        //    radeon_gart_unbind(rdev, offset, pages);
        //    return -ENOMEM;
        //}
        rdev->gart.pages[p] = pagelist[i];
        page_base = (uint32_t)rdev->gart.pages_addr[p];
        for (j = 0; j < (PAGE_SIZE / 4096); j++, t++) {
            radeon_gart_set_page(rdev, t, page_base);
            page_base += 4096;
        }
    }
    mb();
    radeon_gart_tlb_flush(rdev);

    dbgprintf("done %s\n",__FUNCTION__);

    return 0;
}



