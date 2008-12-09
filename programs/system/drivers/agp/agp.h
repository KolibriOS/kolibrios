
/* Chipset independant registers (from AGP Spec) */
#define AGP_APBASE	0x10

#define AGPSTAT		0x4
#define AGPCMD		0x8
#define AGPNISTAT	0xc
#define AGPCTRL		0x10
#define AGPAPSIZE	0x14
#define AGPNEPG		0x16
#define AGPGARTLO	0x18
#define AGPGARTHI	0x1c
#define AGPNICMD	0x20


#define AGP_MAJOR_VERSION_SHIFT (20)
#define AGP_MINOR_VERSION_SHIFT	(16)

#define AGPSTAT_RQ_DEPTH    (0xff000000)
#define AGPSTAT_RQ_DEPTH_SHIFT	24

#define AGPSTAT_CAL_MASK	(1<<12|1<<11|1<<10)
#define AGPSTAT_ARQSZ		(1<<15|1<<14|1<<13)
#define AGPSTAT_ARQSZ_SHIFT	13

#define AGPSTAT_SBA         (1<<9)
#define AGPSTAT_AGP_ENABLE	(1<<8)
#define AGPSTAT_FW          (1<<4)
#define AGPSTAT_MODE_3_0	(1<<3)

#define AGPSTAT2_1X         (1<<0)
#define AGPSTAT2_2X         (1<<1)
#define AGPSTAT2_4X         (1<<2)

#define AGPSTAT3_RSVD		(1<<2)
#define AGPSTAT3_8X         (1<<1)
#define AGPSTAT3_4X         (1)

#define AGPCTRL_APERENB		(1<<8)
#define AGPCTRL_GTLBEN		(1<<7)

#define AGP2_RESERVED_MASK 0x00fffcc8
#define AGP3_RESERVED_MASK 0x00ff00c4

#define AGP_ERRATA_FASTWRITES 1<<0
#define AGP_ERRATA_SBA	 1<<1
#define AGP_ERRATA_1X 1<<2



/* Intel registers */
#define INTEL_APSIZE        0xb4
#define INTEL_ATTBASE       0xb8
#define INTEL_AGPCTRL       0xb0
#define INTEL_NBXCFG        0x50
#define INTEL_ERRSTS        0x91

/* Intel i845 registers */
#define INTEL_I845_AGPM		0x51
#define INTEL_I845_ERRSTS	0xc8

/* Chipset independant registers (from AGP Spec) */
#define AGP_APBASE	0x10

typedef struct
{
    size_t    size;
    count_t   num_entries;
    count_t   pages_count;
    u32_t     size_value;
}aper_size_t;

typedef struct
{
    PCITAG    PciTag;

    aper_size_t    *aperture_sizes;
    aper_size_t    *current_size;
    aper_size_t    *previous_size;
    int             aperture_size_idx;

    u32_t volatile *gatt_table;
    addr_t          gatt_dma;

    addr_t          apbase_config;
    addr_t          gart_addr;

    u32_t           flags;
    u32_t           mode;

    int             capndx;

    char            major_version;
    char            minor_version;

//    int     num_aperture_sizes;
//   enum    aper_size_type size_type;
//    int     cant_use_aperture;
//    int     needs_scratch_page;
//    struct gatt_mask *masks;
    int     (*fetch_size)();
    int     (*configure)();
//    void    (*agp_enable)(struct agp_bridge_data *, u32);
//    void    (*cleanup)(void);
    void    (*tlb_flush)();
//    u32_t   (*mask_memory)(struct agp_bridge_data *,u32_t, int);
//    void    (*cache_flush)(void);
    int     (*create_gatt_table)();
//    int     (*free_gatt_table)(struct agp_bridge_data *);
//    int     (*insert_memory)(struct agp_memory *, off_t, int);
//    int     (*remove_memory)(struct agp_memory *, off_t, int);
//    struct  agp_memory *(*alloc_by_type) (size_t, int);
//    void    (*free_by_type)(struct agp_memory *);
//    void   *(*agp_alloc_page)(struct agp_bridge_data *);
//    void    (*agp_destroy_page)(void *);
}agp_t;


