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
    kobj_t     header;

    uint32_t  *data;
    uint32_t   hot_x;
    uint32_t   hot_y;

    struct list_head   list;
    void      *cobj;
}cursor_t;

#define KMS_CURSOR_WIDTH 64
#define KMS_CURSOR_HEIGHT 64

struct kos_framebuffer
{
    struct list_head list;
    uint32_t    magic;
    uint32_t    handle;
    void        *destructor;

    uint32_t    width;
    uint32_t    height;
    uint32_t    pitch;
    uint32_t    format;
    void        *private;
    uint32_t    pde[8];
};

int fake_framebuffer_create();
void set_fake_framebuffer();
int kolibri_framebuffer_init(void *param);
void kolibri_framebuffer_update(struct drm_device *dev, struct kos_framebuffer *kfb);

struct tag_display
{
    u32   x;
    u32   y;
    u32   width;
    u32   height;
    u32   bpp;
    u32   vrefresh;
    struct kos_framebuffer *current_lfb;
    u32   lfb_pitch;

    struct rw_semaphore win_map_lock;
    void *win_map;
    u32   win_map_pitch;
    u32   win_map_size;

    u32   supported_modes;
    struct drm_device    *ddev;
    struct drm_connector *connector;
    struct drm_crtc      *crtc;

    struct list_head   cursors;

    cursor_t   *cursor;
    int       (*init_cursor)(cursor_t*);
    cursor_t* (__stdcall *select_cursor)(cursor_t*);
    void      (*show_cursor)(int show);
    void      (__stdcall *move_cursor)(cursor_t *cursor, int x, int y);
    void      (__stdcall *restore_cursor)(int x, int y);
    void      (*disable_mouse)(void);
    u32  mask_seqno;
    u32  check_mouse;
    u32  check_m_pixel;

    u32  bytes_per_pixel;
};

extern display_t *os_display;


