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
#ifndef __RADEON_H__
#define __RADEON_H__

//#include "radeon_object.h"

/* TODO: Here are things that needs to be done :
 *	- surface allocator & initializer : (bit like scratch reg) should
 *	  initialize HDP_ stuff on RS600, R600, R700 hw, well anythings
 *	  related to surface
 *	- WB : write back stuff (do it bit like scratch reg things)
 *	- Vblank : look at Jesse's rework and what we should do
 *	- r600/r700: gart & cp
 *	- cs : clean cs ioctl use bitmap & things like that.
 *	- power management stuff
 *	- Barrier in gart code
 *	- Unmappabled vram ?
 *	- TESTING, TESTING, TESTING
 */

#include <types.h>
#include <list.h>

#include <pci.h>

#include <errno-base.h>
#include "drm_edid.h"
#include "radeon_mode.h"
#include "radeon_reg.h"
#include "r300.h"

#include <syscall.h>

extern int radeon_modeset;
extern int radeon_dynclks;
extern int radeon_r4xx_atom;
extern int radeon_gart_size;
extern int radeon_connector_table;

/*
 * Copy from radeon_drv.h so we don't have to include both and have conflicting
 * symbol;
 */
#define RADEON_MAX_USEC_TIMEOUT         100000  /* 100 ms */
#define RADEON_IB_POOL_SIZE             16
#define RADEON_DEBUGFS_MAX_NUM_FILES	32
#define RADEONFB_CONN_LIMIT             4

enum radeon_family {
    CHIP_R100,
    CHIP_RV100,
    CHIP_RS100,
    CHIP_RV200,
    CHIP_RS200,
    CHIP_R200,
    CHIP_RV250,
    CHIP_RS300,
    CHIP_RV280,
    CHIP_R300,
    CHIP_R350,
    CHIP_RV350,
    CHIP_RV380,
    CHIP_R420,
    CHIP_R423,
    CHIP_RV410,
    CHIP_RS400,
    CHIP_RS480,
    CHIP_RS600,
    CHIP_RS690,
    CHIP_RS740,
    CHIP_RV515,
    CHIP_R520,
    CHIP_RV530,
    CHIP_RV560,
    CHIP_RV570,
    CHIP_R580,
    CHIP_R600,
    CHIP_RV610,
    CHIP_RV630,
    CHIP_RV620,
    CHIP_RV635,
    CHIP_RV670,
    CHIP_RS780,
    CHIP_RV770,
    CHIP_RV730,
    CHIP_RV710,
    CHIP_LAST,
};

enum radeon_chip_flags {
    RADEON_FAMILY_MASK = 0x0000ffffUL,
    RADEON_FLAGS_MASK = 0xffff0000UL,
    RADEON_IS_MOBILITY = 0x00010000UL,
    RADEON_IS_IGP = 0x00020000UL,
    RADEON_SINGLE_CRTC = 0x00040000UL,
    RADEON_IS_AGP = 0x00080000UL,
    RADEON_HAS_HIERZ = 0x00100000UL,
    RADEON_IS_PCIE = 0x00200000UL,
    RADEON_NEW_MEMMAP = 0x00400000UL,
    RADEON_IS_PCI = 0x00800000UL,
    RADEON_IS_IGPGART = 0x01000000UL,
};


/*
 * Errata workarounds.
 */
enum radeon_pll_errata {
    CHIP_ERRATA_R300_CG             = 0x00000001,
    CHIP_ERRATA_PLL_DUMMYREADS      = 0x00000002,
    CHIP_ERRATA_PLL_DELAY           = 0x00000004
};


struct radeon_device;


/*
 * BIOS.
 */
bool radeon_get_bios(struct radeon_device *rdev);

/*
 * Clocks
 */

struct radeon_clock {
	struct radeon_pll p1pll;
	struct radeon_pll p2pll;
	struct radeon_pll spll;
	struct radeon_pll mpll;
	/* 10 Khz units */
	uint32_t default_mclk;
	uint32_t default_sclk;
};

/*
 * Fences.
 */
struct radeon_fence_driver {
	uint32_t			scratch_reg;
//	atomic_t			seq;
	uint32_t			last_seq;
	unsigned long			count_timeout;
//	wait_queue_head_t		queue;
//	rwlock_t			lock;
	struct list_head		created;
	struct list_head		emited;
	struct list_head		signaled;
};

struct radeon_fence {
	struct radeon_device		*rdev;
//	struct kref			kref;
	struct list_head		list;
	/* protected by radeon_fence.lock */
	uint32_t			seq;
	unsigned long			timeout;
	bool				emited;
	bool				signaled;
};

int radeon_fence_driver_init(struct radeon_device *rdev);
void radeon_fence_driver_fini(struct radeon_device *rdev);
int radeon_fence_create(struct radeon_device *rdev, struct radeon_fence **fence);
int radeon_fence_emit(struct radeon_device *rdev, struct radeon_fence *fence);
void radeon_fence_process(struct radeon_device *rdev);
bool radeon_fence_signaled(struct radeon_fence *fence);
int radeon_fence_wait(struct radeon_fence *fence, bool interruptible);
int radeon_fence_wait_next(struct radeon_device *rdev);
int radeon_fence_wait_last(struct radeon_device *rdev);
struct radeon_fence *radeon_fence_ref(struct radeon_fence *fence);
void radeon_fence_unref(struct radeon_fence **fence);


/*
 * Radeon buffer.
 */
struct radeon_object;

struct radeon_object_list {
	struct list_head	list;
	struct radeon_object	*robj;
	uint64_t		gpu_offset;
	unsigned		rdomain;
	unsigned		wdomain;
};

int radeon_object_init(struct radeon_device *rdev);
void radeon_object_fini(struct radeon_device *rdev);
int radeon_object_create(struct radeon_device *rdev,
			 struct drm_gem_object *gobj,
			 unsigned long size,
			 bool kernel,
			 uint32_t domain,
			 bool interruptible,
			 struct radeon_object **robj_ptr);


/*
 * GEM objects.
 */
struct radeon_gem {
	struct list_head	objects;
};

int radeon_gem_init(struct radeon_device *rdev);
void radeon_gem_fini(struct radeon_device *rdev);
int radeon_gem_object_create(struct radeon_device *rdev, int size,
			     int alignment, int initial_domain,
			     bool discardable, bool kernel,
			     bool interruptible,
			     struct drm_gem_object **obj);
int radeon_gem_object_pin(struct drm_gem_object *obj, uint32_t pin_domain,
			  uint64_t *gpu_addr);
void radeon_gem_object_unpin(struct drm_gem_object *obj);


/*
 * GART structures, functions & helpers
 */
struct radeon_mc;

struct radeon_gart_table_ram {
    volatile uint32_t       *ptr;
};

struct radeon_gart_table_vram {
    struct radeon_object        *robj;
    volatile uint32_t       *ptr;
};

union radeon_gart_table {
    struct radeon_gart_table_ram    ram;
    struct radeon_gart_table_vram   vram;
};

struct radeon_gart {
    dma_addr_t          table_addr;
    unsigned            num_gpu_pages;
    unsigned            num_cpu_pages;
    unsigned            table_size;
    union radeon_gart_table     table;
    struct page         **pages;
    dma_addr_t          *pages_addr;
    bool                ready;
};

int radeon_gart_table_ram_alloc(struct radeon_device *rdev);
void radeon_gart_table_ram_free(struct radeon_device *rdev);
int radeon_gart_table_vram_alloc(struct radeon_device *rdev);
void radeon_gart_table_vram_free(struct radeon_device *rdev);
int radeon_gart_init(struct radeon_device *rdev);
void radeon_gart_fini(struct radeon_device *rdev);
void radeon_gart_unbind(struct radeon_device *rdev, unsigned offset,
			int pages);
int radeon_gart_bind(struct radeon_device *rdev, unsigned offset,
            int pages, u32_t *pagelist);


/*
 * GPU MC structures, functions & helpers
 */
struct radeon_mc {
    resource_size_t     aper_size;
    resource_size_t     aper_base;
    resource_size_t     agp_base;
    unsigned            gtt_location;
    unsigned            gtt_size;
    unsigned            vram_location;
    unsigned            vram_size;
    unsigned            vram_width;
    int                 vram_mtrr;
    bool                vram_is_ddr;
};

int radeon_mc_setup(struct radeon_device *rdev);


/*
 * GPU scratch registers structures, functions & helpers
 */
struct radeon_scratch {
    unsigned        num_reg;
    bool            free[32];
    uint32_t        reg[32];
};

int radeon_scratch_get(struct radeon_device *rdev, uint32_t *reg);
void radeon_scratch_free(struct radeon_device *rdev, uint32_t reg);


/*
 * IRQS.
 */
struct radeon_irq {
	bool		installed;
	bool		sw_int;
	/* FIXME: use a define max crtc rather than hardcode it */
	bool		crtc_vblank_int[2];
};

int radeon_irq_kms_init(struct radeon_device *rdev);
void radeon_irq_kms_fini(struct radeon_device *rdev);


/*
 * CP & ring.
 */
struct radeon_ib {
	struct list_head	list;
	unsigned long		idx;
	uint64_t		gpu_addr;
	struct radeon_fence	*fence;
	volatile uint32_t	*ptr;
	uint32_t		length_dw;
};

struct radeon_ib_pool {
//	struct mutex		mutex;
	struct radeon_object	*robj;
	struct list_head	scheduled_ibs;
	struct radeon_ib	ibs[RADEON_IB_POOL_SIZE];
	bool			ready;
	DECLARE_BITMAP(alloc_bm, RADEON_IB_POOL_SIZE);
};

struct radeon_cp {
	struct radeon_object	*ring_obj;
	volatile uint32_t	*ring;
	unsigned		rptr;
	unsigned		wptr;
	unsigned		wptr_old;
	unsigned		ring_size;
	unsigned		ring_free_dw;
	int			count_dw;
	uint64_t		gpu_addr;
	uint32_t		align_mask;
	uint32_t		ptr_mask;
//	struct mutex		mutex;
	bool			ready;
};

int radeon_ib_get(struct radeon_device *rdev, struct radeon_ib **ib);
void radeon_ib_free(struct radeon_device *rdev, struct radeon_ib **ib);
int radeon_ib_schedule(struct radeon_device *rdev, struct radeon_ib *ib);
int radeon_ib_pool_init(struct radeon_device *rdev);
void radeon_ib_pool_fini(struct radeon_device *rdev);
int radeon_ib_test(struct radeon_device *rdev);
/* Ring access between begin & end cannot sleep */
void radeon_ring_free_size(struct radeon_device *rdev);
int radeon_ring_lock(struct radeon_device *rdev, unsigned ndw);
void radeon_ring_unlock_commit(struct radeon_device *rdev);
void radeon_ring_unlock_undo(struct radeon_device *rdev);
int radeon_ring_test(struct radeon_device *rdev);
int radeon_ring_init(struct radeon_device *rdev, unsigned ring_size);
void radeon_ring_fini(struct radeon_device *rdev);


/*
 * CS.
 */
struct radeon_cs_reloc {
//	struct drm_gem_object		*gobj;
	struct radeon_object		*robj;
	struct radeon_object_list	lobj;
	uint32_t			handle;
	uint32_t			flags;
};

struct radeon_cs_chunk {
	uint32_t		chunk_id;
	uint32_t		length_dw;
	uint32_t		*kdata;
};

struct radeon_cs_parser {
	struct radeon_device	*rdev;
//	struct drm_file		*filp;
	/* chunks */
	unsigned		nchunks;
	struct radeon_cs_chunk	*chunks;
	uint64_t		*chunks_array;
	/* IB */
	unsigned		idx;
	/* relocations */
	unsigned		nrelocs;
	struct radeon_cs_reloc	*relocs;
	struct radeon_cs_reloc	**relocs_ptr;
	struct list_head	validated;
	/* indices of various chunks */
	int			chunk_ib_idx;
	int			chunk_relocs_idx;
	struct radeon_ib	*ib;
	void			*track;
};

struct radeon_cs_packet {
	unsigned	idx;
	unsigned	type;
	unsigned	reg;
	unsigned	opcode;
	int		count;
	unsigned	one_reg_wr;
};

typedef int (*radeon_packet0_check_t)(struct radeon_cs_parser *p,
				      struct radeon_cs_packet *pkt,
				      unsigned idx, unsigned reg);
typedef int (*radeon_packet3_check_t)(struct radeon_cs_parser *p,
				      struct radeon_cs_packet *pkt);


/*
 * AGP
 */
int radeon_agp_init(struct radeon_device *rdev);
void radeon_agp_fini(struct radeon_device *rdev);


/*
 * Writeback
 */
struct radeon_wb {
	struct radeon_object	*wb_obj;
	volatile uint32_t	*wb;
	uint64_t		gpu_addr;
};


/*
 * ASIC specific functions.
 */
struct radeon_asic {
	int (*init)(struct radeon_device *rdev);
	void (*errata)(struct radeon_device *rdev);
	void (*vram_info)(struct radeon_device *rdev);
	int (*gpu_reset)(struct radeon_device *rdev);
	int (*mc_init)(struct radeon_device *rdev);
	void (*mc_fini)(struct radeon_device *rdev);
	int (*wb_init)(struct radeon_device *rdev);
	void (*wb_fini)(struct radeon_device *rdev);
	int (*gart_enable)(struct radeon_device *rdev);
	void (*gart_disable)(struct radeon_device *rdev);
	void (*gart_tlb_flush)(struct radeon_device *rdev);
	int (*gart_set_page)(struct radeon_device *rdev, int i, uint64_t addr);
	int (*cp_init)(struct radeon_device *rdev, unsigned ring_size);
	void (*cp_fini)(struct radeon_device *rdev);
	void (*cp_disable)(struct radeon_device *rdev);
	void (*ring_start)(struct radeon_device *rdev);
	int (*irq_set)(struct radeon_device *rdev);
	int (*irq_process)(struct radeon_device *rdev);
	void (*fence_ring_emit)(struct radeon_device *rdev, struct radeon_fence *fence);
	int (*cs_parse)(struct radeon_cs_parser *p);
	int (*copy_blit)(struct radeon_device *rdev,
			 uint64_t src_offset,
			 uint64_t dst_offset,
			 unsigned num_pages,
			 struct radeon_fence *fence);
	int (*copy_dma)(struct radeon_device *rdev,
			uint64_t src_offset,
			uint64_t dst_offset,
			unsigned num_pages,
			struct radeon_fence *fence);
	int (*copy)(struct radeon_device *rdev,
		    uint64_t src_offset,
		    uint64_t dst_offset,
		    unsigned num_pages,
		    struct radeon_fence *fence);
	void (*set_engine_clock)(struct radeon_device *rdev, uint32_t eng_clock);
	void (*set_memory_clock)(struct radeon_device *rdev, uint32_t mem_clock);
	void (*set_pcie_lanes)(struct radeon_device *rdev, int lanes);
	void (*set_clock_gating)(struct radeon_device *rdev, int enable);
};

union radeon_asic_config {
	struct r300_asic	r300;
};


/*
/*
 * Core structure, functions and helpers.
 */
typedef uint32_t (*radeon_rreg_t)(struct radeon_device*, uint32_t);
typedef void (*radeon_wreg_t)(struct radeon_device*, uint32_t, uint32_t);

struct radeon_device {
    struct drm_device          *ddev;
    struct pci_dev             *pdev;
    /* ASIC */
    union radeon_asic_config    config;
    enum radeon_family          family;
    unsigned long               flags;
    int                         usec_timeout;
    enum radeon_pll_errata      pll_errata;
    int                         num_gb_pipes;
    int                         disp_priority;
    /* BIOS */
    uint8_t                     *bios;
    bool                        is_atom_bios;
    uint16_t                    bios_header_start;

//    struct radeon_object        *stollen_vga_memory;
    struct fb_info              *fbdev_info;
    struct radeon_object        *fbdev_robj;
    struct radeon_framebuffer   *fbdev_rfb;

    /* Register mmio */
    unsigned long               rmmio_base;
    unsigned long               rmmio_size;
    void                       *rmmio;

    radeon_rreg_t               mm_rreg;
    radeon_wreg_t               mm_wreg;
    radeon_rreg_t               mc_rreg;
    radeon_wreg_t               mc_wreg;
    radeon_rreg_t               pll_rreg;
    radeon_wreg_t               pll_wreg;
    radeon_rreg_t               pcie_rreg;
    radeon_wreg_t               pcie_wreg;
    radeon_rreg_t               pciep_rreg;
    radeon_wreg_t               pciep_wreg;
    struct radeon_clock         clock;
    struct radeon_mc            mc;
    struct radeon_gart          gart;
	struct radeon_mode_info		mode_info;
    struct radeon_scratch       scratch;
//    struct radeon_mman          mman;
	struct radeon_fence_driver	fence_drv;
    struct radeon_cp            cp;
    struct radeon_ib_pool       ib_pool;
//    struct radeon_irq       irq;
    struct radeon_asic         *asic;
    struct radeon_gem       gem;
//    struct mutex            cs_mutex;
    struct radeon_wb        wb;
    bool                gpu_lockup;
    bool                shutdown;
    bool                suspend;
};

int radeon_device_init(struct radeon_device *rdev,
		       struct drm_device *ddev,
		       struct pci_dev *pdev,
		       uint32_t flags);
void radeon_device_fini(struct radeon_device *rdev);
int radeon_gpu_wait_for_idle(struct radeon_device *rdev);

#define __iomem
#define __force



static inline uint8_t __raw_readb(const volatile void __iomem *addr)
{
    return *(const volatile uint8_t __force *) addr;
}

static inline uint16_t __raw_readw(const volatile void __iomem *addr)
{
    return *(const volatile uint16_t __force *) addr;
}

static inline uint32_t __raw_readl(const volatile void __iomem *addr)
{
    return *(const volatile uint32_t __force *) addr;
}

#define readb __raw_readb
#define readw __raw_readw
#define readl __raw_readl



static inline void __raw_writeb(uint8_t b, volatile void __iomem *addr)
{
    *(volatile uint8_t __force *) addr = b;
}

static inline void __raw_writew(uint16_t b, volatile void __iomem *addr)
{
    *(volatile uint16_t __force *) addr = b;
}

static inline void __raw_writel(uint32_t b, volatile void __iomem *addr)
{
    *(volatile uint32_t __force *) addr = b;
}

#define writeb __raw_writeb
#define writew __raw_writew
#define writel __raw_writel

//#define writeb(b,addr) *(volatile uint8_t* ) addr = (uint8_t)b
//#define writew(b,addr) *(volatile uint16_t*) addr = (uint16_t)b
//#define writel(b,addr) *(volatile uint32_t*) addr = (uint32_t)b



/*
 * Registers read & write functions.
 */
#define RREG8(reg) readb(((void __iomem *)rdev->rmmio) + (reg))
#define WREG8(reg, v) writeb(v, ((void __iomem *)rdev->rmmio) + (reg))
#define RREG32(reg) rdev->mm_rreg(rdev, (reg))
#define WREG32(reg, v) rdev->mm_wreg(rdev, (reg), (v))
#define REG_SET(FIELD, v) (((v) << FIELD##_SHIFT) & FIELD##_MASK)
#define REG_GET(FIELD, v) (((v) << FIELD##_SHIFT) & FIELD##_MASK)
#define RREG32_PLL(reg) rdev->pll_rreg(rdev, (reg))
#define WREG32_PLL(reg, v) rdev->pll_wreg(rdev, (reg), (v))
#define RREG32_MC(reg) rdev->mc_rreg(rdev, (reg))
#define WREG32_MC(reg, v) rdev->mc_wreg(rdev, (reg), (v))
#define RREG32_PCIE(reg) rdev->pcie_rreg(rdev, (reg))
#define WREG32_PCIE(reg, v) rdev->pcie_wreg(rdev, (reg), (v))
#define WREG32_P(reg, val, mask)				\
	do {							\
		uint32_t tmp_ = RREG32(reg);			\
		tmp_ &= (mask);					\
		tmp_ |= ((val) & ~(mask));			\
		WREG32(reg, tmp_);				\
	} while (0)
#define WREG32_PLL_P(reg, val, mask)				\
	do {							\
		uint32_t tmp_ = RREG32_PLL(reg);		\
		tmp_ &= (mask);					\
		tmp_ |= ((val) & ~(mask));			\
		WREG32_PLL(reg, tmp_);				\
	} while (0)


#define radeon_PCI_IDS \
    {0x1002, 0x3150, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV380|RADEON_IS_MOBILITY}, \
    {0x1002, 0x3152, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV380|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x3154, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV380|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x3E50, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV380|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x3E54, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV380|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x4136, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS100|RADEON_IS_IGP}, \
    {0x1002, 0x4137, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS200|RADEON_IS_IGP}, \
    {0x1002, 0x4144, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R300}, \
    {0x1002, 0x4145, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R300}, \
    {0x1002, 0x4146, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R300}, \
    {0x1002, 0x4147, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R300}, \
    {0x1002, 0x4148, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R350}, \
    {0x1002, 0x4149, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R350}, \
    {0x1002, 0x414A, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R350}, \
    {0x1002, 0x414B, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R350}, \
    {0x1002, 0x4150, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV350}, \
    {0x1002, 0x4151, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV350}, \
    {0x1002, 0x4152, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV350}, \
    {0x1002, 0x4153, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV350}, \
    {0x1002, 0x4154, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV350}, \
    {0x1002, 0x4155, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV350}, \
    {0x1002, 0x4156, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV350}, \
    {0x1002, 0x4237, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS200|RADEON_IS_IGP}, \
    {0x1002, 0x4242, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R200}, \
    {0x1002, 0x4243, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R200}, \
    {0x1002, 0x4336, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS100|RADEON_IS_IGP|RADEON_IS_MOBILITY}, \
    {0x1002, 0x4337, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS200|RADEON_IS_IGP|RADEON_IS_MOBILITY}, \
    {0x1002, 0x4437, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS200|RADEON_IS_IGP|RADEON_IS_MOBILITY}, \
    {0x1002, 0x4966, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV250}, \
    {0x1002, 0x4967, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV250}, \
    {0x1002, 0x4A48, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R420|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x4A49, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R420|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x4A4A, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R420|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x4A4B, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R420|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x4A4C, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R420|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x4A4D, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R420|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x4A4E, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R420|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x4A4F, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R420|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x4A50, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R420|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x4A54, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R420|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x4B49, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R420|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x4B4A, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R420|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x4B4B, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R420|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x4B4C, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R420|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x4C57, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV200|RADEON_IS_MOBILITY}, \
    {0x1002, 0x4C58, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV200|RADEON_IS_MOBILITY}, \
    {0x1002, 0x4C59, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV100|RADEON_IS_MOBILITY}, \
    {0x1002, 0x4C5A, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV100|RADEON_IS_MOBILITY}, \
    {0x1002, 0x4C64, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV250|RADEON_IS_MOBILITY}, \
    {0x1002, 0x4C66, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV250|RADEON_IS_MOBILITY}, \
    {0x1002, 0x4C67, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV250|RADEON_IS_MOBILITY}, \
    {0x1002, 0x4E44, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R300}, \
    {0x1002, 0x4E45, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R300}, \
    {0x1002, 0x4E46, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R300}, \
    {0x1002, 0x4E47, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R300}, \
    {0x1002, 0x4E48, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R350}, \
    {0x1002, 0x4E49, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R350}, \
    {0x1002, 0x4E4A, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R350}, \
    {0x1002, 0x4E4B, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R350}, \
    {0x1002, 0x4E50, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV350|RADEON_IS_MOBILITY}, \
    {0x1002, 0x4E51, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV350|RADEON_IS_MOBILITY}, \
    {0x1002, 0x4E52, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV350|RADEON_IS_MOBILITY}, \
    {0x1002, 0x4E53, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV350|RADEON_IS_MOBILITY}, \
    {0x1002, 0x4E54, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV350|RADEON_IS_MOBILITY}, \
    {0x1002, 0x4E56, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV350|RADEON_IS_MOBILITY}, \
    {0x1002, 0x5144, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R100|RADEON_SINGLE_CRTC}, \
    {0x1002, 0x5145, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R100|RADEON_SINGLE_CRTC}, \
    {0x1002, 0x5146, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R100|RADEON_SINGLE_CRTC}, \
    {0x1002, 0x5147, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R100|RADEON_SINGLE_CRTC}, \
    {0x1002, 0x5148, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R200}, \
    {0x1002, 0x514C, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R200}, \
    {0x1002, 0x514D, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R200}, \
    {0x1002, 0x5157, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV200}, \
    {0x1002, 0x5158, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV200}, \
    {0x1002, 0x5159, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV100}, \
    {0x1002, 0x515A, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV100}, \
    {0x1002, 0x515E, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV100}, \
    {0x1002, 0x5460, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV380|RADEON_IS_MOBILITY}, \
    {0x1002, 0x5462, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV380|RADEON_IS_MOBILITY}, \
    {0x1002, 0x5464, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV380|RADEON_IS_MOBILITY}, \
    {0x1002, 0x5657, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV380|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5548, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R423|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5549, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R423|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x554A, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R423|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x554B, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R423|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x554C, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R423|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x554D, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R423|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x554E, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R423|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x554F, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R423|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5550, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R423|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5551, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R423|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5552, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R423|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5554, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R423|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x564A, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV410|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x564B, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV410|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x564F, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV410|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5652, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV410|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5653, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV410|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5834, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS300|RADEON_IS_IGP}, \
    {0x1002, 0x5835, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS300|RADEON_IS_IGP|RADEON_IS_MOBILITY}, \
    {0x1002, 0x5954, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS480|RADEON_IS_IGP|RADEON_IS_MOBILITY|RADEON_IS_IGPGART}, \
    {0x1002, 0x5955, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS480|RADEON_IS_IGP|RADEON_IS_MOBILITY|RADEON_IS_IGPGART}, \
    {0x1002, 0x5974, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS480|RADEON_IS_IGP|RADEON_IS_MOBILITY|RADEON_IS_IGPGART}, \
    {0x1002, 0x5975, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS480|RADEON_IS_IGP|RADEON_IS_MOBILITY|RADEON_IS_IGPGART}, \
    {0x1002, 0x5960, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV280}, \
    {0x1002, 0x5961, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV280}, \
    {0x1002, 0x5962, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV280}, \
    {0x1002, 0x5964, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV280}, \
    {0x1002, 0x5965, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV280}, \
    {0x1002, 0x5969, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV100}, \
    {0x1002, 0x5a41, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS400|RADEON_IS_IGP|RADEON_IS_IGPGART}, \
    {0x1002, 0x5a42, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS400|RADEON_IS_IGP|RADEON_IS_MOBILITY|RADEON_IS_IGPGART}, \
    {0x1002, 0x5a61, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS400|RADEON_IS_IGP|RADEON_IS_IGPGART}, \
    {0x1002, 0x5a62, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS400|RADEON_IS_IGP|RADEON_IS_MOBILITY|RADEON_IS_IGPGART}, \
    {0x1002, 0x5b60, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV380|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5b62, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV380|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5b63, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV380|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5b64, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV380|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5b65, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV380|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5c61, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV280|RADEON_IS_MOBILITY}, \
    {0x1002, 0x5c63, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV280|RADEON_IS_MOBILITY}, \
    {0x1002, 0x5d48, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R423|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5d49, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R423|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5d4a, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R423|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5d4c, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R423|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5d4d, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R423|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5d4e, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R423|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5d4f, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R423|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5d50, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R423|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5d52, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R423|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5d57, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R423|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5e48, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV410|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5e4a, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV410|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5e4b, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV410|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5e4c, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV410|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5e4d, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV410|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x5e4f, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV410|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7100, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R520|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7101, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R520|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7102, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R520|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7103, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R520|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7104, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R520|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7105, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R520|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7106, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R520|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7108, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R520|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7109, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R520|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x710A, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R520|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x710B, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R520|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x710C, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R520|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x710E, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R520|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x710F, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R520|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7140, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7141, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7142, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7143, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7144, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7145, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7146, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7147, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7149, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x714A, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x714B, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x714C, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x714D, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x714E, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x714F, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7151, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7152, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7153, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x715E, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x715F, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7180, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7181, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7183, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7186, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7187, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7188, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x718A, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x718B, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x718C, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x718D, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x718F, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7193, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7196, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x719B, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x719F, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x71C0, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV530|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x71C1, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV530|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x71C2, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV530|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x71C3, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV530|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x71C4, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV530|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x71C5, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV530|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x71C6, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV530|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x71C7, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV530|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x71CD, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV530|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x71CE, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV530|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x71D2, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV530|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x71D4, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV530|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x71D5, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV530|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x71D6, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV530|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x71DA, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV530|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x71DE, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV530|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7200, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7210, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7211, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7240, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R580|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7243, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R580|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7244, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R580|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7245, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R580|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7246, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R580|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7247, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R580|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7248, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R580|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7249, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R580|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x724A, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R580|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x724B, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R580|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x724C, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R580|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x724D, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R580|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x724E, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R580|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x724F, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R580|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7280, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV570|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7281, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV560|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7283, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV560|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7284, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R580|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7287, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV560|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7288, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV570|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7289, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV570|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x728B, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV570|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x728C, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV570|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7290, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV560|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7291, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV560|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7293, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV560|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7297, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV560|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7834, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS300|RADEON_IS_IGP|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7835, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS300|RADEON_IS_IGP|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x791e, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS690|RADEON_IS_IGP|RADEON_NEW_MEMMAP|RADEON_IS_IGPGART}, \
    {0x1002, 0x791f, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS690|RADEON_IS_IGP|RADEON_NEW_MEMMAP|RADEON_IS_IGPGART}, \
    {0x1002, 0x793f, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS600|RADEON_IS_IGP|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7941, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS600|RADEON_IS_IGP|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x7942, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS600|RADEON_IS_IGP|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x796c, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS740|RADEON_IS_IGP|RADEON_NEW_MEMMAP|RADEON_IS_IGPGART}, \
    {0x1002, 0x796d, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS740|RADEON_IS_IGP|RADEON_NEW_MEMMAP|RADEON_IS_IGPGART}, \
    {0x1002, 0x796e, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS740|RADEON_IS_IGP|RADEON_NEW_MEMMAP|RADEON_IS_IGPGART}, \
    {0x1002, 0x796f, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS740|RADEON_IS_IGP|RADEON_NEW_MEMMAP|RADEON_IS_IGPGART}, \
    {0x1002, 0x9400, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R600|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9401, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R600|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9402, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R600|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9403, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R600|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9405, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R600|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x940A, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R600|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x940B, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R600|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x940F, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_R600|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9440, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV770|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9441, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV770|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9442, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV770|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9444, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV770|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9446, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV770|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x944A, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV770|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x944B, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV770|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x944C, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV770|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x944E, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV770|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9450, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV770|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9452, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV770|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9456, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV770|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x945A, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV770|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x945B, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV770|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9460, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV770|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9462, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV770|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x946A, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV770|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x946B, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV770|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x947A, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV770|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x947B, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV770|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9480, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV730|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9487, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV730|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9488, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV730|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9489, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV730|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x948F, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV730|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9490, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV730|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9491, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV730|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9498, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV730|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x949C, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV730|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x949E, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV730|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x949F, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV730|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x94C0, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV610|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x94C1, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV610|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x94C3, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV610|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x94C4, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV610|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x94C5, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV610|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x94C6, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV610|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x94C7, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV610|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x94C8, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV610|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x94C9, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV610|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x94CB, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV610|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x94CC, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV610|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x94CD, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV610|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9500, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV670|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9501, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV670|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9504, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV670|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9505, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV670|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9506, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV670|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9507, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV670|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9508, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV670|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9509, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV670|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x950F, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV670|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9511, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV670|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9515, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV670|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9517, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV670|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9519, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV670|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9540, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV710|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9541, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV710|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9542, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV710|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x954E, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV710|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x954F, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV710|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9552, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV710|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9553, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV710|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9555, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV710|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9580, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV630|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9581, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV630|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9583, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV630|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9586, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV630|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9587, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV630|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9588, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV630|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9589, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV630|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x958A, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV630|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x958B, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV630|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x958C, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV630|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x958D, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV630|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x958E, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV630|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x958F, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV630|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9590, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV635|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9591, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV635|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9593, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV635|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9595, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV635|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9596, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV635|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9597, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV635|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9598, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV635|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9599, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV635|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x959B, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV635|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x95C0, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV620|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x95C5, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV620|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x95C6, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV620|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x95C7, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV620|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x95C9, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV620|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x95C2, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV620|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x95C4, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV620|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x95CC, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV620|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x95CD, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV620|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x95CE, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV620|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x95CF, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RV620|RADEON_NEW_MEMMAP}, \
    {0x1002, 0x9610, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS780|RADEON_NEW_MEMMAP|RADEON_IS_IGP}, \
    {0x1002, 0x9611, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS780|RADEON_NEW_MEMMAP|RADEON_IS_IGP}, \
    {0x1002, 0x9612, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS780|RADEON_NEW_MEMMAP|RADEON_IS_IGP}, \
    {0x1002, 0x9613, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS780|RADEON_NEW_MEMMAP|RADEON_IS_IGP}, \
    {0x1002, 0x9614, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS780|RADEON_NEW_MEMMAP|RADEON_IS_IGP}, \
    {0x1002, 0x9615, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS780|RADEON_NEW_MEMMAP|RADEON_IS_IGP}, \
    {0x1002, 0x9616, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_RS780|RADEON_NEW_MEMMAP|RADEON_IS_IGP}, \
    {0, 0, 0}


enum chipset_type {
    NOT_SUPPORTED,
    SUPPORTED,
};

struct agp_version {
    u16_t major;
    u16_t minor;
};

struct agp_bridge_data;

struct agp_kern_info {
    struct agp_version version;
    struct pci_dev *device;
    enum chipset_type chipset;
    unsigned long mode;
    unsigned long aper_base;
    size_t aper_size;
    int max_memory;     /* In pages */
    int current_memory;
    bool cant_use_aperture;
    unsigned long page_mask;
//    struct vm_operations_struct *vm_ops;
};


/**
 * AGP data.
 *
 * \sa drm_agp_init() and drm_device::agp.
 */
struct drm_agp_head {
    struct agp_kern_info agp_info;      /**< AGP device information */
//    struct list_head memory;
    unsigned long mode;     /**< AGP mode */
    struct agp_bridge_data *bridge;
    int enabled;            /**< whether the AGP bus as been enabled */
    int acquired;           /**< whether the AGP device has been acquired */
    unsigned long base;
    int agp_mtrr;
    int cant_use_aperture;
    unsigned long page_mask;
};


#define radeon_errata(rdev) (rdev)->asic->errata((rdev))


/*
 * ASICs helpers.
 */
#define ASIC_IS_RV100(rdev) ((rdev->family == CHIP_RV100) || \
        (rdev->family == CHIP_RV200) || \
        (rdev->family == CHIP_RS100) || \
        (rdev->family == CHIP_RS200) || \
        (rdev->family == CHIP_RV250) || \
        (rdev->family == CHIP_RV280) || \
        (rdev->family == CHIP_RS300))
#define ASIC_IS_R300(rdev) ((rdev->family == CHIP_R300)  || \
        (rdev->family == CHIP_RV350) ||         \
        (rdev->family == CHIP_R350)  ||         \
        (rdev->family == CHIP_RV380) ||         \
        (rdev->family == CHIP_R420)  ||         \
        (rdev->family == CHIP_R423)  ||         \
        (rdev->family == CHIP_RV410) ||         \
        (rdev->family == CHIP_RS400) ||         \
        (rdev->family == CHIP_RS480))
#define ASIC_IS_AVIVO(rdev) ((rdev->family >= CHIP_RS600))
#define ASIC_IS_DCE3(rdev) ((rdev->family >= CHIP_RV620))
#define ASIC_IS_DCE32(rdev) ((rdev->family >= CHIP_RV730))


/*
 * BIOS helpers.
 */
#define RBIOS8(i) (rdev->bios[i])
#define RBIOS16(i) (RBIOS8(i) | (RBIOS8((i)+1) << 8))
#define RBIOS32(i) ((RBIOS16(i)) | (RBIOS16((i)+2) << 16))

int radeon_combios_init(struct radeon_device *rdev);
void radeon_combios_fini(struct radeon_device *rdev);
int radeon_atombios_init(struct radeon_device *rdev);
void radeon_atombios_fini(struct radeon_device *rdev);


/*
 * RING helpers.
 */
#define CP_PACKET0			0x00000000
#define		PACKET0_BASE_INDEX_SHIFT	0
#define		PACKET0_BASE_INDEX_MASK		(0x1ffff << 0)
#define		PACKET0_COUNT_SHIFT		16
#define		PACKET0_COUNT_MASK		(0x3fff << 16)
#define CP_PACKET1			0x40000000
#define CP_PACKET2			0x80000000
#define		PACKET2_PAD_SHIFT		0
#define		PACKET2_PAD_MASK		(0x3fffffff << 0)
#define CP_PACKET3			0xC0000000
#define		PACKET3_IT_OPCODE_SHIFT		8
#define		PACKET3_IT_OPCODE_MASK		(0xff << 8)
#define		PACKET3_COUNT_SHIFT		16
#define		PACKET3_COUNT_MASK		(0x3fff << 16)
/* PACKET3 op code */
#define		PACKET3_NOP			0x10
#define		PACKET3_3D_DRAW_VBUF		0x28
#define		PACKET3_3D_DRAW_IMMD		0x29
#define		PACKET3_3D_DRAW_INDX		0x2A
#define		PACKET3_3D_LOAD_VBPNTR		0x2F
#define		PACKET3_INDX_BUFFER		0x33
#define		PACKET3_3D_DRAW_VBUF_2		0x34
#define		PACKET3_3D_DRAW_IMMD_2		0x35
#define		PACKET3_3D_DRAW_INDX_2		0x36
#define		PACKET3_BITBLT_MULTI		0x9B

#define PACKET0(reg, n)	(CP_PACKET0 |					\
			 REG_SET(PACKET0_BASE_INDEX, (reg) >> 2) |	\
			 REG_SET(PACKET0_COUNT, (n)))
#define PACKET2(v)	(CP_PACKET2 | REG_SET(PACKET2_PAD, (v)))
#define PACKET3(op, n)	(CP_PACKET3 |					\
			 REG_SET(PACKET3_IT_OPCODE, (op)) |		\
			 REG_SET(PACKET3_COUNT, (n)))

#define	PACKET_TYPE0	0
#define	PACKET_TYPE1	1
#define	PACKET_TYPE2	2
#define	PACKET_TYPE3	3

#define CP_PACKET_GET_TYPE(h) (((h) >> 30) & 3)
#define CP_PACKET_GET_COUNT(h) (((h) >> 16) & 0x3FFF)
#define CP_PACKET0_GET_REG(h) (((h) & 0x1FFF) << 2)
#define CP_PACKET0_GET_ONE_REG_WR(h) (((h) >> 15) & 1)
#define CP_PACKET3_GET_OPCODE(h) (((h) >> 8) & 0xFF)

static inline void radeon_ring_write(struct radeon_device *rdev, uint32_t v)
{
#if DRM_DEBUG_CODE
	if (rdev->cp.count_dw <= 0) {
		DRM_ERROR("radeon: writting more dword to ring than expected !\n");
	}
#endif
	rdev->cp.ring[rdev->cp.wptr++] = v;
	rdev->cp.wptr &= rdev->cp.ptr_mask;
	rdev->cp.count_dw--;
	rdev->cp.ring_free_dw--;
}


/*
 * ASICs macro.
 */
#define radeon_init(rdev) (rdev)->asic->init((rdev))
#define radeon_cs_parse(p) rdev->asic->cs_parse((p))
#define radeon_errata(rdev) (rdev)->asic->errata((rdev))
#define radeon_vram_info(rdev) (rdev)->asic->vram_info((rdev))
#define radeon_gpu_reset(rdev) (rdev)->asic->gpu_reset((rdev))
#define radeon_mc_init(rdev) (rdev)->asic->mc_init((rdev))
#define radeon_mc_fini(rdev) (rdev)->asic->mc_fini((rdev))
#define radeon_wb_init(rdev) (rdev)->asic->wb_init((rdev))
#define radeon_wb_fini(rdev) (rdev)->asic->wb_fini((rdev))
#define radeon_gart_enable(rdev) (rdev)->asic->gart_enable((rdev))
#define radeon_gart_disable(rdev) (rdev)->asic->gart_disable((rdev))
#define radeon_gart_tlb_flush(rdev) (rdev)->asic->gart_tlb_flush((rdev))
#define radeon_gart_set_page(rdev, i, p) (rdev)->asic->gart_set_page((rdev), (i), (p))
#define radeon_cp_init(rdev,rsize) (rdev)->asic->cp_init((rdev), (rsize))
#define radeon_cp_fini(rdev) (rdev)->asic->cp_fini((rdev))
#define radeon_cp_disable(rdev) (rdev)->asic->cp_disable((rdev))
#define radeon_ring_start(rdev) (rdev)->asic->ring_start((rdev))
#define radeon_irq_set(rdev) (rdev)->asic->irq_set((rdev))
#define radeon_irq_process(rdev) (rdev)->asic->irq_process((rdev))
#define radeon_fence_ring_emit(rdev, fence) (rdev)->asic->fence_ring_emit((rdev), (fence))
#define radeon_copy_blit(rdev, s, d, np, f) (rdev)->asic->copy_blit((rdev), (s), (d), (np), (f))
#define radeon_copy_dma(rdev, s, d, np, f) (rdev)->asic->copy_dma((rdev), (s), (d), (np), (f))
#define radeon_copy(rdev, s, d, np, f) (rdev)->asic->copy((rdev), (s), (d), (np), (f))
#define radeon_set_engine_clock(rdev, e) (rdev)->asic->set_engine_clock((rdev), (e))
#define radeon_set_memory_clock(rdev, e) (rdev)->asic->set_engine_clock((rdev), (e))
#define radeon_set_pcie_lanes(rdev, l) (rdev)->asic->set_pcie_lanes((rdev), (l))
#define radeon_set_clock_gating(rdev, e) (rdev)->asic->set_clock_gating((rdev), (e))


#define DRM_UDELAY(d)           udelay(d)

resource_size_t
drm_get_resource_start(struct drm_device *dev, unsigned int resource);
resource_size_t
drm_get_resource_len(struct drm_device *dev, unsigned int resource);


#endif
