#ifndef __PIXDRIVER_H__
#define __PIXDRIVER_H__

struct pix_driver
{
    uint32_t  driver_caps;
    bitmap_t *(*create_bitmap)(uint32_t width, uint32_t height);
    int       (*destroy_bitmap)(bitmap_t *bitmap);
    void     *(*lock_bitmap)(bitmap_t *bitmap, uint32_t *pitch);
    int       (*resize_bitmap)(bitmap_t *bitmap, uint32_t width, uint32_t height);
    int       (*blit_bitmap)(bitmap_t * bitmap, int dst_x, int dst_y,
                uint32_t w, uint32_t h, int src_x, int src_y);
    int       (*create_client)(int x, int y, uint32_t width, uint32_t height);
    int       (*resize_client)(int x, int y, uint32_t width, uint32_t height);
    void      (*fini)(void);
};

#endif
