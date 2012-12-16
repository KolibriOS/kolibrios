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
#include <linux/slab.h>
#include <drm/drmP.h>
#include <drm/drm_crtc_helper.h>
#include <drm/radeon_drm.h>
#include "radeon_reg.h"
#include "radeon.h"
#include "atom.h"

#include "bitmap.h"
#include "display.h"


#include <drm/drm_pciids.h>


int radeon_no_wb   =  1;
int radeon_modeset = -1;
int radeon_dynclks = -1;
int radeon_r4xx_atom = 0;
int radeon_agpmode = 0;
int radeon_vram_limit = 0;
int radeon_gart_size = 512; /* default gart size */
int radeon_benchmarking = 0;
int radeon_testing = 0;
int radeon_connector_table = 0;
int radeon_tv = 1;
int radeon_new_pll = -1;
int radeon_dynpm = -1;
int radeon_audio = 1;
int radeon_hw_i2c = 0;
int radeon_pcie_gen2 = 0;
int radeon_disp_priority = 0;
int radeon_lockup_timeout = 10000;


int irq_override = 0;


extern display_t *rdisplay;
struct drm_device *main_drm_device;


void parse_cmdline(char *cmdline, videomode_t *mode, char *log, int *kms);
int init_display(struct radeon_device *rdev, videomode_t *mode);
int init_display_kms(struct radeon_device *rdev, videomode_t *mode);

int get_modes(videomode_t *mode, u32_t *count);
int set_user_mode(videomode_t *mode);
int r100_2D_test(struct radeon_device *rdev);


 /* Legacy VGA regions */
#define VGA_RSRC_NONE          0x00
#define VGA_RSRC_LEGACY_IO     0x01
#define VGA_RSRC_LEGACY_MEM    0x02
#define VGA_RSRC_LEGACY_MASK   (VGA_RSRC_LEGACY_IO | VGA_RSRC_LEGACY_MEM)
/* Non-legacy access */
#define VGA_RSRC_NORMAL_IO     0x04
#define VGA_RSRC_NORMAL_MEM    0x08


static const char radeon_family_name[][16] = {
	"R100",
	"RV100",
	"RS100",
	"RV200",
	"RS200",
	"R200",
	"RV250",
	"RS300",
	"RV280",
	"R300",
	"R350",
	"RV350",
	"RV380",
	"R420",
	"R423",
	"RV410",
	"RS400",
	"RS480",
	"RS600",
	"RS690",
	"RS740",
	"RV515",
	"R520",
	"RV530",
	"RV560",
	"RV570",
	"R580",
	"R600",
	"RV610",
	"RV630",
	"RV670",
	"RV620",
	"RV635",
	"RS780",
	"RS880",
	"RV770",
	"RV730",
	"RV710",
	"RV740",
	"CEDAR",
	"REDWOOD",
	"JUNIPER",
	"CYPRESS",
	"HEMLOCK",
	"PALM",
	"SUMO",
	"SUMO2",
	"BARTS",
	"TURKS",
	"CAICOS",
	"CAYMAN",
	"ARUBA",
	"TAHITI",
	"PITCAIRN",
	"VERDE",
	"LAST",
};

/**
 * radeon_surface_init - Clear GPU surface registers.
 *
 * @rdev: radeon_device pointer
 *
 * Clear GPU surface registers (r1xx-r5xx).
 */
void radeon_surface_init(struct radeon_device *rdev)
{
    /* FIXME: check this out */
    if (rdev->family < CHIP_R600) {
        int i;

		for (i = 0; i < RADEON_GEM_MAX_SURFACES; i++) {
           radeon_clear_surface_reg(rdev, i);
        }
		/* enable surfaces */
		WREG32(RADEON_SURFACE_CNTL, 0);
    }
}

/*
 * GPU scratch registers helpers function.
 */
/**
 * radeon_scratch_init - Init scratch register driver information.
 *
 * @rdev: radeon_device pointer
 *
 * Init CP scratch register driver information (r1xx-r5xx)
 */
void radeon_scratch_init(struct radeon_device *rdev)
{
    int i;

    /* FIXME: check this out */
    if (rdev->family < CHIP_R300) {
        rdev->scratch.num_reg = 5;
    } else {
        rdev->scratch.num_reg = 7;
    }
	rdev->scratch.reg_base = RADEON_SCRATCH_REG0;
    for (i = 0; i < rdev->scratch.num_reg; i++) {
        rdev->scratch.free[i] = true;
		rdev->scratch.reg[i] = rdev->scratch.reg_base + (i * 4);
    }
}

/**
 * radeon_scratch_get - Allocate a scratch register
 *
 * @rdev: radeon_device pointer
 * @reg: scratch register mmio offset
 *
 * Allocate a CP scratch register for use by the driver (all asics).
 * Returns 0 on success or -EINVAL on failure.
 */
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

/**
 * radeon_scratch_free - Free a scratch register
 *
 * @rdev: radeon_device pointer
 * @reg: scratch register mmio offset
 *
 * Free a CP scratch register allocated for use by the driver (all asics)
 */
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
 * radeon_wb_*()
 * Writeback is the the method by which the the GPU updates special pages
 * in memory with the status of certain GPU events (fences, ring pointers,
 * etc.).
 */

/**
 * radeon_wb_disable - Disable Writeback
 *
 * @rdev: radeon_device pointer
 *
 * Disables Writeback (all asics).  Used for suspend.
 */
void radeon_wb_disable(struct radeon_device *rdev)
{
	int r;

	if (rdev->wb.wb_obj) {
		r = radeon_bo_reserve(rdev->wb.wb_obj, false);
		if (unlikely(r != 0))
			return;
		radeon_bo_kunmap(rdev->wb.wb_obj);
		radeon_bo_unpin(rdev->wb.wb_obj);
		radeon_bo_unreserve(rdev->wb.wb_obj);
	}
	rdev->wb.enabled = false;
}

/**
 * radeon_wb_fini - Disable Writeback and free memory
 *
 * @rdev: radeon_device pointer
 *
 * Disables Writeback and frees the Writeback memory (all asics).
 * Used at driver shutdown.
 */
void radeon_wb_fini(struct radeon_device *rdev)
{
	radeon_wb_disable(rdev);
	if (rdev->wb.wb_obj) {
		radeon_bo_unref(&rdev->wb.wb_obj);
		rdev->wb.wb = NULL;
		rdev->wb.wb_obj = NULL;
	}
}

/**
 * radeon_wb_init- Init Writeback driver info and allocate memory
 *
 * @rdev: radeon_device pointer
 *
 * Disables Writeback and frees the Writeback memory (all asics).
 * Used at driver startup.
 * Returns 0 on success or an -error on failure.
 */
int radeon_wb_init(struct radeon_device *rdev)
{
	int r;

	if (rdev->wb.wb_obj == NULL) {
		r = radeon_bo_create(rdev, RADEON_GPU_PAGE_SIZE, PAGE_SIZE, true,
				     RADEON_GEM_DOMAIN_GTT, NULL, &rdev->wb.wb_obj);
		if (r) {
			dev_warn(rdev->dev, "(%d) create WB bo failed\n", r);
			return r;
		}
	}
	r = radeon_bo_reserve(rdev->wb.wb_obj, false);
	if (unlikely(r != 0)) {
		radeon_wb_fini(rdev);
		return r;
	}
	r = radeon_bo_pin(rdev->wb.wb_obj, RADEON_GEM_DOMAIN_GTT,
			  &rdev->wb.gpu_addr);
	if (r) {
		radeon_bo_unreserve(rdev->wb.wb_obj);
		dev_warn(rdev->dev, "(%d) pin WB bo failed\n", r);
		radeon_wb_fini(rdev);
		return r;
	}
	r = radeon_bo_kmap(rdev->wb.wb_obj, (void **)&rdev->wb.wb);
	radeon_bo_unreserve(rdev->wb.wb_obj);
	if (r) {
		dev_warn(rdev->dev, "(%d) map WB bo failed\n", r);
		radeon_wb_fini(rdev);
		return r;
	}

	/* clear wb memory */
	memset((char *)rdev->wb.wb, 0, RADEON_GPU_PAGE_SIZE);
	/* disable event_write fences */
	rdev->wb.use_event = false;
	/* disabled via module param */
	if (radeon_no_wb == 1) {
		rdev->wb.enabled = false;
	} else {
		if (rdev->flags & RADEON_IS_AGP) {
		/* often unreliable on AGP */
			rdev->wb.enabled = false;
		} else if (rdev->family < CHIP_R300) {
			/* often unreliable on pre-r300 */
			rdev->wb.enabled = false;
		} else {
			rdev->wb.enabled = true;
			/* event_write fences are only available on r600+ */
			if (rdev->family >= CHIP_R600) {
				rdev->wb.use_event = true;
	}
		}
	}
	/* always use writeback/events on NI, APUs */
	if (rdev->family >= CHIP_PALM) {
		rdev->wb.enabled = true;
		rdev->wb.use_event = true;
	}

	dev_info(rdev->dev, "WB %sabled\n", rdev->wb.enabled ? "en" : "dis");

	return 0;
}

/**
 * radeon_vram_location - try to find VRAM location
 * @rdev: radeon device structure holding all necessary informations
 * @mc: memory controller structure holding memory informations
 * @base: base address at which to put VRAM
 *
 * Function will place try to place VRAM at base address provided
 * as parameter (which is so far either PCI aperture address or
 * for IGP TOM base address).
 *
 * If there is not enough space to fit the unvisible VRAM in the 32bits
 * address space then we limit the VRAM size to the aperture.
 *
 * If we are using AGP and if the AGP aperture doesn't allow us to have
 * room for all the VRAM than we restrict the VRAM to the PCI aperture
 * size and print a warning.
 *
 * This function will never fails, worst case are limiting VRAM.
 *
 * Note: GTT start, end, size should be initialized before calling this
 * function on AGP platform.
 *
 * Note: We don't explicitly enforce VRAM start to be aligned on VRAM size,
 * this shouldn't be a problem as we are using the PCI aperture as a reference.
 * Otherwise this would be needed for rv280, all r3xx, and all r4xx, but
 * not IGP.
 *
 * Note: we use mc_vram_size as on some board we need to program the mc to
 * cover the whole aperture even if VRAM size is inferior to aperture size
 * Novell bug 204882 + along with lots of ubuntu ones
 *
 * Note: when limiting vram it's safe to overwritte real_vram_size because
 * we are not in case where real_vram_size is inferior to mc_vram_size (ie
 * note afected by bogus hw of Novell bug 204882 + along with lots of ubuntu
 * ones)
 *
 * Note: IGP TOM addr should be the same as the aperture addr, we don't
 * explicitly check for that thought.
 *
 * FIXME: when reducing VRAM size align new size on power of 2.
 */
void radeon_vram_location(struct radeon_device *rdev, struct radeon_mc *mc, u64 base)
{
	uint64_t limit = (uint64_t)radeon_vram_limit << 20;

	mc->vram_start = base;
	if (mc->mc_vram_size > (0xFFFFFFFF - base + 1)) {
		dev_warn(rdev->dev, "limiting VRAM to PCI aperture size\n");
		mc->real_vram_size = mc->aper_size;
		mc->mc_vram_size = mc->aper_size;
	}
	mc->vram_end = mc->vram_start + mc->mc_vram_size - 1;
	if (rdev->flags & RADEON_IS_AGP && mc->vram_end > mc->gtt_start && mc->vram_start <= mc->gtt_end) {
		dev_warn(rdev->dev, "limiting VRAM to PCI aperture size\n");
		mc->real_vram_size = mc->aper_size;
		mc->mc_vram_size = mc->aper_size;
		}
	mc->vram_end = mc->vram_start + mc->mc_vram_size - 1;
	if (limit && limit < mc->real_vram_size)
		mc->real_vram_size = limit;
	dev_info(rdev->dev, "VRAM: %lluM 0x%016llX - 0x%016llX (%lluM used)\n",
			mc->mc_vram_size >> 20, mc->vram_start,
			mc->vram_end, mc->real_vram_size >> 20);
}

/**
 * radeon_gtt_location - try to find GTT location
 * @rdev: radeon device structure holding all necessary informations
 * @mc: memory controller structure holding memory informations
 *
 * Function will place try to place GTT before or after VRAM.
 *
 * If GTT size is bigger than space left then we ajust GTT size.
 * Thus function will never fails.
 *
 * FIXME: when reducing GTT size align new size on power of 2.
 */
void radeon_gtt_location(struct radeon_device *rdev, struct radeon_mc *mc)
{
	u64 size_af, size_bf;

	size_af = ((0xFFFFFFFF - mc->vram_end) + mc->gtt_base_align) & ~mc->gtt_base_align;
	size_bf = mc->vram_start & ~mc->gtt_base_align;
	if (size_bf > size_af) {
		if (mc->gtt_size > size_bf) {
			dev_warn(rdev->dev, "limiting GTT\n");
			mc->gtt_size = size_bf;
		}
		mc->gtt_start = (mc->vram_start & ~mc->gtt_base_align) - mc->gtt_size;
	} else {
		if (mc->gtt_size > size_af) {
			dev_warn(rdev->dev, "limiting GTT\n");
			mc->gtt_size = size_af;
		}
		mc->gtt_start = (mc->vram_end + 1 + mc->gtt_base_align) & ~mc->gtt_base_align;
	}
	mc->gtt_end = mc->gtt_start + mc->gtt_size - 1;
	dev_info(rdev->dev, "GTT: %lluM 0x%016llX - 0x%016llX\n",
			mc->gtt_size >> 20, mc->gtt_start, mc->gtt_end);
}

/*
 * GPU helpers function.
 */
/**
 * radeon_card_posted - check if the hw has already been initialized
 *
 * @rdev: radeon_device pointer
 *
 * Check if the asic has been initialized (all asics).
 * Used at driver startup.
 * Returns true if initialized or false if not.
 */
bool radeon_card_posted(struct radeon_device *rdev)
{
	uint32_t reg;

	/* first check CRTCs */
	if (ASIC_IS_DCE41(rdev)) {
		reg = RREG32(EVERGREEN_CRTC_CONTROL + EVERGREEN_CRTC0_REGISTER_OFFSET) |
			RREG32(EVERGREEN_CRTC_CONTROL + EVERGREEN_CRTC1_REGISTER_OFFSET);
		if (reg & EVERGREEN_CRTC_MASTER_EN)
			return true;
	} else if (ASIC_IS_DCE4(rdev)) {
		reg = RREG32(EVERGREEN_CRTC_CONTROL + EVERGREEN_CRTC0_REGISTER_OFFSET) |
			RREG32(EVERGREEN_CRTC_CONTROL + EVERGREEN_CRTC1_REGISTER_OFFSET) |
			RREG32(EVERGREEN_CRTC_CONTROL + EVERGREEN_CRTC2_REGISTER_OFFSET) |
			RREG32(EVERGREEN_CRTC_CONTROL + EVERGREEN_CRTC3_REGISTER_OFFSET) |
			RREG32(EVERGREEN_CRTC_CONTROL + EVERGREEN_CRTC4_REGISTER_OFFSET) |
			RREG32(EVERGREEN_CRTC_CONTROL + EVERGREEN_CRTC5_REGISTER_OFFSET);
		if (reg & EVERGREEN_CRTC_MASTER_EN)
			return true;
	} else if (ASIC_IS_AVIVO(rdev)) {
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

/**
 * radeon_update_bandwidth_info - update display bandwidth params
 *
 * @rdev: radeon_device pointer
 *
 * Used when sclk/mclk are switched or display modes are set.
 * params are used to calculate display watermarks (all asics)
 */
void radeon_update_bandwidth_info(struct radeon_device *rdev)
{
	fixed20_12 a;
	u32 sclk = rdev->pm.current_sclk;
	u32 mclk = rdev->pm.current_mclk;

	/* sclk/mclk in Mhz */
		a.full = dfixed_const(100);
		rdev->pm.sclk.full = dfixed_const(sclk);
		rdev->pm.sclk.full = dfixed_div(rdev->pm.sclk, a);
		rdev->pm.mclk.full = dfixed_const(mclk);
		rdev->pm.mclk.full = dfixed_div(rdev->pm.mclk, a);

	if (rdev->flags & RADEON_IS_IGP) {
		a.full = dfixed_const(16);
		/* core_bandwidth = sclk(Mhz) * 16 */
		rdev->pm.core_bandwidth.full = dfixed_div(rdev->pm.sclk, a);
	}
}

/**
 * radeon_boot_test_post_card - check and possibly initialize the hw
 *
 * @rdev: radeon_device pointer
 *
 * Check if the asic is initialized and if not, attempt to initialize
 * it (all asics).
 * Returns true if initialized or false if not.
 */
bool radeon_boot_test_post_card(struct radeon_device *rdev)
{
	if (radeon_card_posted(rdev))
		return true;

	if (rdev->bios) {
		DRM_INFO("GPU not posted. posting now...\n");
		if (rdev->is_atom_bios)
			atom_asic_init(rdev->mode_info.atom_context);
		else
			radeon_combios_asic_init(rdev->ddev);
		return true;
	} else {
		dev_err(rdev->dev, "Card not posted and no BIOS - ignoring\n");
		return false;
	}
}

/**
 * radeon_dummy_page_init - init dummy page used by the driver
 *
 * @rdev: radeon_device pointer
 *
 * Allocate the dummy page used by the driver (all asics).
 * This dummy page is used by the driver as a filler for gart entries
 * when pages are taken out of the GART
 * Returns 0 on sucess, -ENOMEM on failure.
 */
int radeon_dummy_page_init(struct radeon_device *rdev)
{
	if (rdev->dummy_page.page)
		return 0;
    rdev->dummy_page.page = (void*)AllocPage();
	if (rdev->dummy_page.page == NULL)
		return -ENOMEM;
    rdev->dummy_page.addr = MapIoMem((addr_t)rdev->dummy_page.page, 4096, 3);
	if (!rdev->dummy_page.addr) {
//       __free_page(rdev->dummy_page.page);
		rdev->dummy_page.page = NULL;
		return -ENOMEM;
	}
	return 0;
}

/**
 * radeon_dummy_page_fini - free dummy page used by the driver
 *
 * @rdev: radeon_device pointer
 *
 * Frees the dummy page used by the driver (all asics).
 */
void radeon_dummy_page_fini(struct radeon_device *rdev)
{
	if (rdev->dummy_page.page == NULL)
		return;
    KernelFree((void*)rdev->dummy_page.addr);
	rdev->dummy_page.page = NULL;
}


/* ATOM accessor methods */
/*
 * ATOM is an interpreted byte code stored in tables in the vbios.  The
 * driver registers callbacks to access registers and the interpreter
 * in the driver parses the tables and executes then to program specific
 * actions (set display modes, asic init, etc.).  See radeon_atombios.c,
 * atombios.h, and atom.c
 */

/**
 * cail_pll_read - read PLL register
 *
 * @info: atom card_info pointer
 * @reg: PLL register offset
 *
 * Provides a PLL register accessor for the atom interpreter (r4xx+).
 * Returns the value of the PLL register.
 */
static uint32_t cail_pll_read(struct card_info *info, uint32_t reg)
{
    struct radeon_device *rdev = info->dev->dev_private;
    uint32_t r;

    r = rdev->pll_rreg(rdev, reg);
    return r;
}

/**
 * cail_pll_write - write PLL register
 *
 * @info: atom card_info pointer
 * @reg: PLL register offset
 * @val: value to write to the pll register
 *
 * Provides a PLL register accessor for the atom interpreter (r4xx+).
 */
static void cail_pll_write(struct card_info *info, uint32_t reg, uint32_t val)
{
    struct radeon_device *rdev = info->dev->dev_private;

    rdev->pll_wreg(rdev, reg, val);
}

/**
 * cail_mc_read - read MC (Memory Controller) register
 *
 * @info: atom card_info pointer
 * @reg: MC register offset
 *
 * Provides an MC register accessor for the atom interpreter (r4xx+).
 * Returns the value of the MC register.
 */
static uint32_t cail_mc_read(struct card_info *info, uint32_t reg)
{
    struct radeon_device *rdev = info->dev->dev_private;
    uint32_t r;

    r = rdev->mc_rreg(rdev, reg);
    return r;
}

/**
 * cail_mc_write - write MC (Memory Controller) register
 *
 * @info: atom card_info pointer
 * @reg: MC register offset
 * @val: value to write to the pll register
 *
 * Provides a MC register accessor for the atom interpreter (r4xx+).
 */
static void cail_mc_write(struct card_info *info, uint32_t reg, uint32_t val)
{
    struct radeon_device *rdev = info->dev->dev_private;

    rdev->mc_wreg(rdev, reg, val);
}

/**
 * cail_reg_write - write MMIO register
 *
 * @info: atom card_info pointer
 * @reg: MMIO register offset
 * @val: value to write to the pll register
 *
 * Provides a MMIO register accessor for the atom interpreter (r4xx+).
 */
static void cail_reg_write(struct card_info *info, uint32_t reg, uint32_t val)
{
    struct radeon_device *rdev = info->dev->dev_private;

    WREG32(reg*4, val);
}

/**
 * cail_reg_read - read MMIO register
 *
 * @info: atom card_info pointer
 * @reg: MMIO register offset
 *
 * Provides an MMIO register accessor for the atom interpreter (r4xx+).
 * Returns the value of the MMIO register.
 */
static uint32_t cail_reg_read(struct card_info *info, uint32_t reg)
{
    struct radeon_device *rdev = info->dev->dev_private;
    uint32_t r;

    r = RREG32(reg*4);
    return r;
}

/**
 * cail_ioreg_write - write IO register
 *
 * @info: atom card_info pointer
 * @reg: IO register offset
 * @val: value to write to the pll register
 *
 * Provides a IO register accessor for the atom interpreter (r4xx+).
 */
static void cail_ioreg_write(struct card_info *info, uint32_t reg, uint32_t val)
{
	struct radeon_device *rdev = info->dev->dev_private;

	WREG32_IO(reg*4, val);
}

/**
 * cail_ioreg_read - read IO register
 *
 * @info: atom card_info pointer
 * @reg: IO register offset
 *
 * Provides an IO register accessor for the atom interpreter (r4xx+).
 * Returns the value of the IO register.
 */
static uint32_t cail_ioreg_read(struct card_info *info, uint32_t reg)
{
	struct radeon_device *rdev = info->dev->dev_private;
	uint32_t r;

	r = RREG32_IO(reg*4);
	return r;
}

/**
 * radeon_atombios_init - init the driver info and callbacks for atombios
 *
 * @rdev: radeon_device pointer
 *
 * Initializes the driver info and register access callbacks for the
 * ATOM interpreter (r4xx+).
 * Returns 0 on sucess, -ENOMEM on failure.
 * Called at driver startup.
 */
int radeon_atombios_init(struct radeon_device *rdev)
{
	struct card_info *atom_card_info =
	    kzalloc(sizeof(struct card_info), GFP_KERNEL);

	if (!atom_card_info)
		return -ENOMEM;

	rdev->mode_info.atom_card_info = atom_card_info;
	atom_card_info->dev = rdev->ddev;
	atom_card_info->reg_read = cail_reg_read;
	atom_card_info->reg_write = cail_reg_write;
	/* needed for iio ops */
	if (rdev->rio_mem) {
		atom_card_info->ioreg_read = cail_ioreg_read;
		atom_card_info->ioreg_write = cail_ioreg_write;
	} else {
		DRM_ERROR("Unable to find PCI I/O BAR; using MMIO for ATOM IIO\n");
		atom_card_info->ioreg_read = cail_reg_read;
		atom_card_info->ioreg_write = cail_reg_write;
	}
	atom_card_info->mc_read = cail_mc_read;
	atom_card_info->mc_write = cail_mc_write;
	atom_card_info->pll_read = cail_pll_read;
	atom_card_info->pll_write = cail_pll_write;

	rdev->mode_info.atom_context = atom_parse(atom_card_info, rdev->bios);
	mutex_init(&rdev->mode_info.atom_context->mutex);
    radeon_atom_initialize_bios_scratch_regs(rdev->ddev);
	atom_allocate_fb_scratch(rdev->mode_info.atom_context);
    return 0;
}

/**
 * radeon_atombios_fini - free the driver info and callbacks for atombios
 *
 * @rdev: radeon_device pointer
 *
 * Frees the driver info and register access callbacks for the ATOM
 * interpreter (r4xx+).
 * Called at driver shutdown.
 */
void radeon_atombios_fini(struct radeon_device *rdev)
{
	if (rdev->mode_info.atom_context) {
		kfree(rdev->mode_info.atom_context->scratch);
	kfree(rdev->mode_info.atom_context);
	}
	kfree(rdev->mode_info.atom_card_info);
}

/* COMBIOS */
/*
 * COMBIOS is the bios format prior to ATOM. It provides
 * command tables similar to ATOM, but doesn't have a unified
 * parser.  See radeon_combios.c
 */

/**
 * radeon_combios_init - init the driver info for combios
 *
 * @rdev: radeon_device pointer
 *
 * Initializes the driver info for combios (r1xx-r3xx).
 * Returns 0 on sucess.
 * Called at driver startup.
 */
int radeon_combios_init(struct radeon_device *rdev)
{
	radeon_combios_initialize_bios_scratch_regs(rdev->ddev);
	return 0;
}

/**
 * radeon_combios_fini - free the driver info for combios
 *
 * @rdev: radeon_device pointer
 *
 * Frees the driver info for combios (r1xx-r3xx).
 * Called at driver shutdown.
 */
void radeon_combios_fini(struct radeon_device *rdev)
{
}

/* if we get transitioned to only one device, take VGA back */
/**
 * radeon_vga_set_decode - enable/disable vga decode
 *
 * @cookie: radeon_device pointer
 * @state: enable/disable vga decode
 *
 * Enable/disable vga decode (all asics).
 * Returns VGA resource flags.
 */
static unsigned int radeon_vga_set_decode(void *cookie, bool state)
{
	struct radeon_device *rdev = cookie;
	radeon_vga_set_state(rdev, state);
	if (state)
		return VGA_RSRC_LEGACY_IO | VGA_RSRC_LEGACY_MEM |
		       VGA_RSRC_NORMAL_IO | VGA_RSRC_NORMAL_MEM;
	else
		return VGA_RSRC_NORMAL_IO | VGA_RSRC_NORMAL_MEM;
}

/**
 * radeon_check_pot_argument - check that argument is a power of two
 *
 * @arg: value to check
 *
 * Validates that a certain argument is a power of two (all asics).
 * Returns true if argument is valid.
 */
static bool radeon_check_pot_argument(int arg)
{
	return (arg & (arg - 1)) == 0;
}

/**
 * radeon_check_arguments - validate module params
 *
 * @rdev: radeon_device pointer
 *
 * Validates certain module parameters and updates
 * the associated values used by the driver (all asics).
 */
static void radeon_check_arguments(struct radeon_device *rdev)
{
	/* vramlimit must be a power of two */
	if (!radeon_check_pot_argument(radeon_vram_limit)) {
		dev_warn(rdev->dev, "vram limit (%d) must be a power of 2\n",
				radeon_vram_limit);
		radeon_vram_limit = 0;
	}

	/* gtt size must be power of two and greater or equal to 32M */
	if (radeon_gart_size < 32) {
		dev_warn(rdev->dev, "gart size (%d) too small forcing to 512M\n",
				radeon_gart_size);
		radeon_gart_size = 512;

	} else if (!radeon_check_pot_argument(radeon_gart_size)) {
		dev_warn(rdev->dev, "gart size (%d) must be a power of 2\n",
				radeon_gart_size);
		radeon_gart_size = 512;
	}
	rdev->mc.gtt_size = (uint64_t)radeon_gart_size << 20;

	/* AGP mode can only be -1, 1, 2, 4, 8 */
	switch (radeon_agpmode) {
	case -1:
	case 0:
	case 1:
	case 2:
	case 4:
	case 8:
		break;
	default:
		dev_warn(rdev->dev, "invalid AGP mode %d (valid mode: "
				"-1, 0, 1, 2, 4, 8)\n", radeon_agpmode);
		radeon_agpmode = 0;
		break;
	}
}

int radeon_device_init(struct radeon_device *rdev,
               struct drm_device *ddev,
               struct pci_dev *pdev,
               uint32_t flags)
{
	int r, i;
	int dma_bits;

    rdev->shutdown = false;
    rdev->ddev = ddev;
    rdev->pdev = pdev;
    rdev->flags = flags;
    rdev->family = flags & RADEON_FAMILY_MASK;
    rdev->is_atom_bios = false;
    rdev->usec_timeout = RADEON_MAX_USEC_TIMEOUT;
    rdev->mc.gtt_size = radeon_gart_size * 1024 * 1024;
	rdev->accel_working = false;
	/* set up ring ids */
	for (i = 0; i < RADEON_NUM_RINGS; i++) {
		rdev->ring[i].idx = i;
	}

	DRM_INFO("initializing kernel modesetting (%s 0x%04X:0x%04X 0x%04X:0x%04X).\n",
		radeon_family_name[rdev->family], pdev->vendor, pdev->device,
		pdev->subsystem_vendor, pdev->subsystem_device);

    /* mutex initialization are all done here so we
     * can recall function without having locking issues */
	mutex_init(&rdev->ring_lock);
	mutex_init(&rdev->dc_hw_i2c_mutex);
	atomic_set(&rdev->ih.lock, 0);
	mutex_init(&rdev->gem.mutex);
	mutex_init(&rdev->pm.mutex);
	mutex_init(&rdev->gpu_clock_mutex);
	init_rwsem(&rdev->pm.mclk_lock);
	init_rwsem(&rdev->exclusive_lock);
	init_waitqueue_head(&rdev->irq.vblank_queue);
	r = radeon_gem_init(rdev);
	if (r)
		return r;
	/* initialize vm here */
	mutex_init(&rdev->vm_manager.lock);
	/* Adjust VM size here.
	 * Currently set to 4GB ((1 << 20) 4k pages).
	 * Max GPUVM size for cayman and SI is 40 bits.
	 */
	rdev->vm_manager.max_pfn = 1 << 20;
	INIT_LIST_HEAD(&rdev->vm_manager.lru_vm);

	/* Set asic functions */
	r = radeon_asic_init(rdev);
	if (r)
		return r;
	radeon_check_arguments(rdev);

	/* all of the newer IGP chips have an internal gart
	 * However some rs4xx report as AGP, so remove that here.
	 */
	if ((rdev->family >= CHIP_RS400) &&
	    (rdev->flags & RADEON_IS_IGP)) {
		rdev->flags &= ~RADEON_IS_AGP;
	}

	if (rdev->flags & RADEON_IS_AGP && radeon_agpmode == -1) {
		radeon_agp_disable(rdev);
    }

	/* set DMA mask + need_dma32 flags.
	 * PCIE - can handle 40-bits.
	 * IGP - can handle 40-bits
	 * AGP - generally dma32 is safest
	 * PCI - dma32 for legacy pci gart, 40 bits on newer asics
	 */
	rdev->need_dma32 = false;
	if (rdev->flags & RADEON_IS_AGP)
		rdev->need_dma32 = true;
	if ((rdev->flags & RADEON_IS_PCI) &&
	    (rdev->family <= CHIP_RS740))
		rdev->need_dma32 = true;

	dma_bits = rdev->need_dma32 ? 32 : 40;
	r = pci_set_dma_mask(rdev->pdev, DMA_BIT_MASK(dma_bits));
    if (r) {
		rdev->need_dma32 = true;
		dma_bits = 32;
        printk(KERN_WARNING "radeon: No suitable DMA available.\n");
    }

    /* Registers mapping */
    /* TODO: block userspace mapping of io register */
    rdev->rmmio_base = pci_resource_start(rdev->pdev, 2);
    rdev->rmmio_size = pci_resource_len(rdev->pdev, 2);
	rdev->rmmio = ioremap(rdev->rmmio_base, rdev->rmmio_size);
    if (rdev->rmmio == NULL) {
        return -ENOMEM;
    }
    DRM_INFO("register mmio base: 0x%08X\n", (uint32_t)rdev->rmmio_base);
    DRM_INFO("register mmio size: %u\n", (unsigned)rdev->rmmio_size);

	/* io port mapping */
	for (i = 0; i < DEVICE_COUNT_RESOURCE; i++) {
		if (pci_resource_flags(rdev->pdev, i) & IORESOURCE_IO) {
			rdev->rio_mem_size = pci_resource_len(rdev->pdev, i);
			rdev->rio_mem = pci_iomap(rdev->pdev, i, rdev->rio_mem_size);
			break;
		}
	}
	if (rdev->rio_mem == NULL)
		DRM_ERROR("Unable to find PCI I/O BAR\n");


	r = radeon_init(rdev);
	if (r)
        return r;

	if (rdev->flags & RADEON_IS_AGP && !rdev->accel_working) {
		/* Acceleration not working on AGP card try again
		 * with fallback to PCI or PCIE GART
		 */
		radeon_asic_reset(rdev);
		radeon_fini(rdev);
		radeon_agp_disable(rdev);
		r = radeon_init(rdev);
		if (r)
		return r;
	}
//	if (radeon_testing) {
//		radeon_test_moves(rdev);
//    }
//	if ((radeon_testing & 2)) {
//		radeon_test_syncing(rdev);
//	}
   if (radeon_benchmarking) {
		radeon_benchmark(rdev, radeon_benchmarking);
    }
	return 0;
}

/**
 * radeon_gpu_reset - reset the asic
 *
 * @rdev: radeon device pointer
 *
 * Attempt the reset the GPU if it has hung (all asics).
 * Returns 0 for success or an error on failure.
 */
int radeon_gpu_reset(struct radeon_device *rdev)
{
    unsigned ring_sizes[RADEON_NUM_RINGS];
    uint32_t *ring_data[RADEON_NUM_RINGS];

    bool saved = false;

    int i, r;
    int resched;

//    down_write(&rdev->exclusive_lock);
    radeon_save_bios_scratch_regs(rdev);
    /* block TTM */
//    resched = ttm_bo_lock_delayed_workqueue(&rdev->mman.bdev);
    radeon_suspend(rdev);

    for (i = 0; i < RADEON_NUM_RINGS; ++i) {
        ring_sizes[i] = radeon_ring_backup(rdev, &rdev->ring[i],
                           &ring_data[i]);
        if (ring_sizes[i]) {
            saved = true;
            dev_info(rdev->dev, "Saved %d dwords of commands "
                 "on ring %d.\n", ring_sizes[i], i);
        }
    }

retry:
    r = radeon_asic_reset(rdev);
    if (!r) {
        dev_info(rdev->dev, "GPU reset succeeded, trying to resume\n");
        radeon_resume(rdev);
    }

    radeon_restore_bios_scratch_regs(rdev);
    drm_helper_resume_force_mode(rdev->ddev);

    if (!r) {
        for (i = 0; i < RADEON_NUM_RINGS; ++i) {
            radeon_ring_restore(rdev, &rdev->ring[i],
                        ring_sizes[i], ring_data[i]);
            ring_sizes[i] = 0;
            ring_data[i] = NULL;
        }

        r = radeon_ib_ring_tests(rdev);
        if (r) {
            dev_err(rdev->dev, "ib ring test failed (%d).\n", r);
            if (saved) {
                saved = false;
                radeon_suspend(rdev);
                goto retry;
            }
        }
    } else {
        for (i = 0; i < RADEON_NUM_RINGS; ++i) {
            kfree(ring_data[i]);
        }
    }

//    ttm_bo_unlock_delayed_workqueue(&rdev->mman.bdev, resched);
    if (r) {
        /* bad news, how to tell it to userspace ? */
        dev_info(rdev->dev, "GPU reset failed\n");
    }

//    up_write(&rdev->exclusive_lock);
    return r;
}



/*
 * Driver load/unload
 */
int radeon_driver_load_kms(struct drm_device *dev, unsigned long flags)
{
    struct radeon_device *rdev;
    int r;

    ENTER();

    rdev = kzalloc(sizeof(struct radeon_device), GFP_KERNEL);
    if (rdev == NULL) {
        return -ENOMEM;
    };

    dev->dev_private = (void *)rdev;

    /* update BUS flag */
    if (drm_device_is_agp(dev)) {
        flags |= RADEON_IS_AGP;
    } else if (drm_device_is_pcie(dev)) {
        flags |= RADEON_IS_PCIE;
    } else {
        flags |= RADEON_IS_PCI;
    }

    /* radeon_device_init should report only fatal error
     * like memory allocation failure or iomapping failure,
     * or memory manager initialization failure, it must
     * properly initialize the GPU MC controller and permit
     * VRAM allocation
     */
    r = radeon_device_init(rdev, dev, dev->pdev, flags);
    if (r) {
        DRM_ERROR("Fatal error while trying to initialize radeon.\n");
        return r;
    }
    /* Again modeset_init should fail only on fatal error
     * otherwise it should provide enough functionalities
     * for shadowfb to run
     */
    if( radeon_modeset )
    {
        r = radeon_modeset_init(rdev);
        if (r) {
            return r;
        }
    };
    return 0;
}

videomode_t usermode;


int drm_get_dev(struct pci_dev *pdev, const struct pci_device_id *ent)
{
    static struct drm_device *dev;
    int ret;

    ENTER();

    dev = kzalloc(sizeof(*dev), 0);
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

    INIT_LIST_HEAD(&dev->filelist);
    INIT_LIST_HEAD(&dev->ctxlist);
    INIT_LIST_HEAD(&dev->vmalist);
    INIT_LIST_HEAD(&dev->maplist);

    spin_lock_init(&dev->count_lock);
    mutex_init(&dev->struct_mutex);
    mutex_init(&dev->ctxlist_mutex);


    ret = radeon_driver_load_kms(dev, ent->driver_data );
    if (ret)
        goto err_g4;

    main_drm_device = dev;

    if( radeon_modeset )
        init_display_kms(dev->dev_private, &usermode);
    else
        init_display(dev->dev_private, &usermode);


    LEAVE();

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

    LEAVE();

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


static struct pci_device_id pciidlist[] = {
    radeon_PCI_IDS
};


#define CURRENT_API     0x0200      /*      2.00     */
#define COMPATIBLE_API  0x0100      /*      1.00     */

#define API_VERSION     (COMPATIBLE_API << 16) | CURRENT_API

#define SRV_GETVERSION      0
#define SRV_ENUM_MODES      1
#define SRV_SET_MODE        2
#define SRV_GET_CAPS            3

#define SRV_CREATE_SURFACE      10
#define SRV_DESTROY_SURFACE     11
#define SRV_LOCK_SURFACE        12
#define SRV_UNLOCK_SURFACE      13
#define SRV_RESIZE_SURFACE      14
#define SRV_BLIT_BITMAP         15
#define SRV_BLIT_TEXTURE        16
#define SRV_BLIT_VIDEO          17



int r600_video_blit(uint64_t src_offset, int  x, int y,
                    int w, int h, int pitch);

#define check_input(size) \
    if( unlikely((inp==NULL)||(io->inp_size != (size))) )   \
        break;

#define check_output(size) \
    if( unlikely((outp==NULL)||(io->out_size != (size))) )   \
        break;

int _stdcall display_handler(ioctl_t *io)
{
    int    retval = -1;
    u32_t *inp;
    u32_t *outp;

    inp = io->input;
    outp = io->output;

    switch(io->io_code)
    {
        case SRV_GETVERSION:
            check_output(4);
            *outp  = API_VERSION;
            retval = 0;
            break;

        case SRV_ENUM_MODES:
            dbgprintf("SRV_ENUM_MODES inp %x inp_size %x out_size %x\n",
                       inp, io->inp_size, io->out_size );
            check_output(4);
            if( radeon_modeset)
                retval = get_modes((videomode_t*)inp, outp);
            break;

        case SRV_SET_MODE:
            dbgprintf("SRV_SET_MODE inp %x inp_size %x\n",
                       inp, io->inp_size);
            check_input(sizeof(videomode_t));
            if( radeon_modeset )
                retval = set_user_mode((videomode_t*)inp);
            break;

        case SRV_GET_CAPS:
            retval = get_driver_caps((hwcaps_t*)inp);
            break;

        case SRV_CREATE_SURFACE:
//            check_input(8);
            retval = create_surface(main_drm_device, (struct io_call_10*)inp);
            break;

        case SRV_LOCK_SURFACE:
            retval = lock_surface((struct io_call_12*)inp);
            break;

        case SRV_BLIT_BITMAP:
            srv_blit_bitmap( inp[0], inp[1], inp[2],
                        inp[3], inp[4], inp[5], inp[6]);

    };

    return retval;
}

static char  log[256];
static pci_dev_t device;

u32_t drvEntry(int action, char *cmdline)
{
    struct radeon_device *rdev = NULL;

    const struct pci_device_id  *ent;

    int     err;
    u32_t   retval = 0;

    if(action != 1)
        return 0;

    if( GetService("DISPLAY") != 0 )
        return 0;

    if( cmdline && *cmdline )
        parse_cmdline(cmdline, &usermode, log, &radeon_modeset);

    if(!dbg_open(log))
    {
        strcpy(log, "/RD/1/DRIVERS/atikms.log");

        if(!dbg_open(log))
        {
            printf("Can't open %s\nExit\n", log);
            return 0;
        };
    }
    dbgprintf("Radeon RC12 preview 1 cmdline %s\n", cmdline);

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

    rdev = rdisplay->ddev->dev_private;

    err = RegService("DISPLAY", display_handler);

    if( err != 0)
        dbgprintf("Set DISPLAY handler\n");

    return err;
};

#define PCI_CLASS_REVISION      0x08
#define PCI_CLASS_DISPLAY_VGA   0x0300

int pci_scan_filter(u32_t id, u32_t busnr, u32_t devfn)
{
    u16_t vendor, device;
    u32_t class;
    int   ret = 0;

    vendor   = id & 0xffff;
    device   = (id >> 16) & 0xffff;

    if(vendor == 0x1002)
    {
        class = PciRead32(busnr, devfn, PCI_CLASS_REVISION);
        class >>= 16;

        if( class == PCI_CLASS_DISPLAY_VGA)
            ret = 1;
    }
    return ret;
}
