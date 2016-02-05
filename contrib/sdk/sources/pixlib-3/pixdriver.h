#ifndef __PIXDRIVER_H__
#define __PIXDRIVER_H__

struct pix_driver
{
    uint32_t  driver_caps;
    bitmap_t *(*create_bitmap)(uint32_t width, uint32_t height);
    int       (*destroy_bitmap)(bitmap_t *bitmap);
    void     *(*lock_bitmap)(bitmap_t *bitmap, uint32_t *pitch);
    int       (*resize_bitmap)(bitmap_t *bitmap, uint32_t width, uint32_t height);
    int       (*blit_bitmap)(bitmap_t *bitmap, int dst_x, int dst_y,
                             uint32_t w, uint32_t h, int src_x, int src_y);
    int       (*create_client)(int x, int y, uint32_t width, uint32_t height);
    int       (*resize_client)(int x, int y, uint32_t width, uint32_t height);
    void      (*fini)(void);
    planar_t *(*create_planar)(int name, int format,
                            uint32_t width, uint32_t height,
                            uint32_t offset0, uint32_t pitch0,
                            uint32_t offset1, uint32_t pitch1,
                            uint32_t offset2, uint32_t pitch2);
    int       (*destroy_planar)(planar_t *planar);
    int       (*blit_planar)(planar_t *planar, int dst_x, int dst_y,
                             uint32_t w, uint32_t h, int src_x, int src_y);
};

#endif
