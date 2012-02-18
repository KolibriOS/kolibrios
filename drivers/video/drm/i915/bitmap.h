
typedef struct tag_object  kobj_t;
typedef struct tag_display display_t;

struct hman
{
    u32  *table;
    u32   next;
    u32   avail;
    u32   count;
};

extern struct hman bm_man;

int init_hman(struct hman *man, u32 count);
u32  alloc_handle(struct hman *man);
int free_handle(struct hman *man, u32 handle);

#define hman_get_data(man, handle)                  \
        ((man)->table[(handle)-1])

#define hman_set_data(man, handle, val)             \
        ((man)->table[(handle)-1]) = (u32)(val)


struct tag_object
{
    uint32_t   magic;
    void      *destroy;
    kobj_t    *fd;
    kobj_t    *bk;
    uint32_t   pid;
};

typedef struct
{
    kobj_t   header;

    u32     handle;
    void    *uaddr;

    u32     pitch;
    u32     gaddr;

    u32     width;
    u32     height;
    u32     max_width;
    u32     max_height;

    u32     format;
    struct drm_i915_gem_object *obj;
}bitmap_t;


struct  io_call_10         /*     SRV_CREATE_SURFACE    */
{
    u32     handle;       // ignored
    void   *data;         // ignored

    u32     width;
    u32     height;
    u32     pitch;        // ignored

    u32     max_width;
    u32     max_height;
    u32     format;       // reserved mbz
};

struct  io_call_12         /*     SRV_LOCK_SURFACE    */
{
    u32     handle;       // ignored
    void   *data;         // ignored

    u32     width;
    u32     height;
    u32     pitch;        // ignored
};


typedef struct
{
    uint32_t  idx;
    union
    {
        uint32_t opt[2];
        struct {
            uint32_t max_tex_width;
            uint32_t max_tex_height;
        }cap1;
    };
}hwcaps_t;

#define HW_BIT_BLIT         (1<<0)      /* BGRX blitter             */
#define HW_TEX_BLIT         (1<<1)      /* stretch blit             */
#define HW_VID_BLIT         (1<<2)      /* planar and packed video  */
                                        /*  3 - 63 reserved         */

int get_driver_caps(hwcaps_t *caps);
int create_surface(struct io_call_10 *pbitmap);
int lock_surface(struct io_call_12 *pbitmap);

int init_bitmaps();

