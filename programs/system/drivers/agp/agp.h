
/* Intel registers */
#define INTEL_APSIZE	0xb4
#define INTEL_ATTBASE	0xb8
#define INTEL_AGPCTRL	0xb0
#define INTEL_NBXCFG	0x50
#define INTEL_ERRSTS	0x91

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

    aper_size_t    *previous_size;
    aper_size_t    *current_size;
    int             aperture_size_idx;

    u32_t volatile *gatt_table;
    addr_t          gatt_dma;

    addr_t          apbase_config;
    addr_t          gart_addr;

//    void   *aperture_sizes;
//    int     num_aperture_sizes;
//   enum    aper_size_type size_type;
//    int     cant_use_aperture;
//    int     needs_scratch_page;
//    struct gatt_mask *masks;
    int     (*fetch_size)(void *);
    int     (*configure)(void *);
//    void    (*agp_enable)(struct agp_bridge_data *, u32);
//    void    (*cleanup)(void);
    void    (*tlb_flush)(void *);
//    u32_t   (*mask_memory)(struct agp_bridge_data *,u32_t, int);
//    void    (*cache_flush)(void);
//    int     (*create_gatt_table)(struct agp_bridge_data *);
//    int     (*free_gatt_table)(struct agp_bridge_data *);
//    int     (*insert_memory)(struct agp_memory *, off_t, int);
//    int     (*remove_memory)(struct agp_memory *, off_t, int);
//    struct  agp_memory *(*alloc_by_type) (size_t, int);
//    void    (*free_by_type)(struct agp_memory *);
//    void   *(*agp_alloc_page)(struct agp_bridge_data *);
//    void    (*agp_destroy_page)(void *);
}agp_t;


