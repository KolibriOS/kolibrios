#ifndef __PIXLIB3_H__
#define __PIXLIB3_H__

#include <stdint.h>

#define HW_BIT_BLIT         (1<<0)      /* BGRX blitter             */
#define HW_TEX_BLIT         (1<<1)      /* stretch blit             */
#define HW_VID_BLIT         (1<<2)      /* planar and packed video  */


struct bitmap;
typedef struct bitmap bitmap_t;

uint32_t pxInit(int hw);
void pxFini();

bitmap_t *pxCreateBitmap(uint32_t width, uint32_t height);

int pxDestroyBitmap(bitmap_t *bitmap);

void *pxLockBitmap(bitmap_t *bitmap, uint32_t *pitch);

int pxResizeBitmap(bitmap_t *bitmap, uint32_t width, uint32_t height);

int pxBlitBitmap(bitmap_t *bitmap, int dst_x, int dst_y,
                 uint32_t w, uint32_t h, int src_x, int src_y);

int pxCreateClient(int x, int y, uint32_t width, uint32_t height);

int pxResizeClient(int x, int y, uint32_t width, uint32_t height);

#endif

