

typedef struct
{
    u32_t   width;
    u32_t   height;
    u32_t   bpp;

    u32_t   lfb;
    u32_t   pci_fb;
    u32_t   gpu_fb;
    u32_t   fb_object;

    struct drm_display_mode *mode;
    cursor_t   *cursor;

    int     (*set_cursor)();
    int     (*show_cursor)();
    int     (*hide_cursor)();
    int     (*move_cursor)();

    int     (*copy)();
    int     (*blit)();

}display_t;
