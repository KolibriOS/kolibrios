
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

    u32       handle;
    u32       width;
    u32       height;
    u32       pitch;
    u32       gaddr;
    void     *uaddr;
    struct drm_i915_gem_object *obj;
}bitmap_t;


struct ubitmap
{
    u32    width;
    u32    height;
    u32    pitch;
    u32    handle;
    void  *data;
};

int create_bitmap(struct ubitmap *pbitmap, int width, int height);

