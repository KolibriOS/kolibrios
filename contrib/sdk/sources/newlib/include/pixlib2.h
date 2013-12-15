#ifndef __PIXLIB2_H__
#define __PIXLIB2_H__

#include <stdint.h>

#define HW_BIT_BLIT         (1<<0)      /* BGRX blitter             */
#define HW_TEX_BLIT         (1<<1)      /* stretch blit             */
#define HW_VID_BLIT         (1<<2)      /* planar and packed video  */

typedef struct
{
    uint32_t  handle;
    uint8_t  *data;

    uint32_t  width;
    uint32_t  height;
    uint32_t  pitch;

    uint32_t  max_width;
    uint32_t  max_height;
    uint32_t  flags;
}bitmap_t;

uint32_t init_pixlib(uint32_t flags);
void done_pixlib();

int create_bitmap(bitmap_t *bitmap);
int destroy_bitmap(bitmap_t *bitmap);

int lock_bitmap(bitmap_t *bitmap);
int resize_bitmap(bitmap_t *bitmap);

int blit_bitmap(bitmap_t * bitmap, int dst_x, int dst_y,
                int w, int h, int src_x, int src_y);



#endif

