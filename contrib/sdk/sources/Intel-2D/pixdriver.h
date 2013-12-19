#ifndef __PIXDRIVER_H__
#define __PIXDRIVER_H__

struct pix_driver
{
    char *name;

    int (*create_bitmap)(bitmap_t * bitmap);
    int (*destroy_bitmap)(bitmap_t * bitmap);
    int (*lock_bitmap)(bitmap_t * bitmap);
    int (*blit)(bitmap_t * bitmap, int scale, int vsync,
                int dst_x, int dst_y, int w, int h, int src_x, int src_y);
    int (*resize_bitmap)(bitmap_t * bitmap);
    void (*fini)(void);
};

#endif
