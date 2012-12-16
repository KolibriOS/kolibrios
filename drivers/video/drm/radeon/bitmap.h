
typedef struct tag_object  kobj_t;
typedef struct tag_display display_t;


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
    char   *uaddr;

    u32     pitch;
    u32     gaddr;

    u32     width;
    u32     height;
    u32     max_width;
    u32     max_height;
    u32     page_count;
    u32     max_count;

    u32     format;
    struct radeon_bo  *obj;
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
    u32     handle;
    void   *data;
    u32     pitch;
};

struct  io_call_14         /*     SRV_RESIZE_SURFACE    */
{
    u32     handle;
    void   *data;
    u32     new_width;
    u32     new_height;
    u32     pitch;
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
struct context
{
    kobj_t   header;

    struct radeon_ib ib;

    u32       cmd_buffer;
    u32       cmd_offset;

    bitmap_t *mask;
    u32       seqno;
    int       slot;

};

int get_driver_caps(hwcaps_t *caps);
int create_surface(struct drm_device *dev, struct io_call_10 *pbitmap);
int lock_surface(struct io_call_12 *pbitmap);
int resize_surface(struct io_call_14 *pbitmap);

struct context *get_context(struct drm_device *dev);

int init_bitmaps();

