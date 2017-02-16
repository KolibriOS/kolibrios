#ifndef __PXDRAW_H__
#define __PXDRAW_H__

#include <stdint.h>

#if defined __cplusplus
extern "C" {
#endif

typedef unsigned int color_t;
typedef struct context ctx_t;

typedef struct
{
	int  l;
	int  t;
	int  r;
	int  b;
}rect_t;

typedef struct
{
	int num_rects;
	rect_t *rects;
	rect_t extents;
}rgn_t;

rgn_t* create_round_rect_rgn(int left, int top, int right, int bottom,
                             int ellipse_width, int ellipse_height);
void   destroy_region(rgn_t *rgn);

ctx_t* create_context(int x, int y, int width, int height);
int    resize_context(ctx_t *ctx, int width, int height);
void   clear_context(ctx_t *ctx, color_t color);
void   show_context(ctx_t *ctx);
void   scroll_context(ctx_t *ctx, int dst_y, int  src_y, int rows);

int    px_hline(ctx_t*ctx, int x, int y, int width, color_t color);
void   px_vline(ctx_t*ctx, int x, int y, int height, color_t color);
void   px_fill_rect(ctx_t *ctx, const rect_t *src, color_t color);
void   px_draw_glyph(ctx_t *ctx, const void *buffer, int pitch, const rect_t *rc, color_t color);

#if defined __cplusplus
}
#endif
#endif /* __PXDRAW_H__ */
