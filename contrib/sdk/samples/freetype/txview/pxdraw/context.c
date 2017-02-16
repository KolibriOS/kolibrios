#include <stdlib.h>
#include <kos32sys.h>
#include "pxdraw.h"
#include "internal.h"

ctx_t* create_context(int x, int y, int width, int height)
{
	ctx_t *ctx;

	ctx = malloc(sizeof(ctx_t));
	if (ctx == NULL)
        goto err_0;

    ctx->pitch = ALIGN(width * sizeof(color_t), 16);
    ctx->size = ALIGN(ctx->pitch * height, 4096);

    ctx->buffer = user_alloc(ctx->size+4096);
    if (ctx->buffer == NULL)
        goto err_1;

	ctx->x = x;
	ctx->y = y;
	ctx->width = width;
	ctx->height = height;

	ctx->rc.l = 0;
	ctx->rc.t = 0;
	ctx->rc.r = width;
	ctx->rc.b = height;

	ctx->rcu.l = 0;
	ctx->rcu.t = 0;
	ctx->rcu.r = ctx->width;
	ctx->rcu.b = ctx->height;
    ctx->dirty = 1;

    __builtin_cpu_init ();
    if (__builtin_cpu_supports ("sse2"))
        ctx->px_rect_simd = px_rect_xmm;
    else if (__builtin_cpu_supports ("mmx"))
        ctx->px_rect_simd = px_rect_mmx;
    else
        ctx->px_rect_simd = px_rect_alu;

    if (__builtin_cpu_supports ("sse2"))
		ctx->px_glyph = px_glyph_sse;
	else
        ctx->px_glyph = px_glyph_alu;

	return ctx;

err_1:
    free(ctx);
err_0:
    return NULL;
};

int resize_context(ctx_t *ctx, int width, int height)
{
    int size;
    int pitch;

    pitch = ALIGN(width * sizeof(color_t), 16);
    size = ALIGN(pitch * height, 4096);

    if (size > ctx->size)
    {
        ctx->buffer = user_realloc(ctx->buffer, size);    /* grow buffer */
        if (ctx->buffer == NULL)
            return -1;

        ctx->size = size;
    }
    else if (size < ctx->size)
        user_unmap(ctx->buffer, size, ctx->size - size); /* unmap unused pages */

    ctx->width  = width;
    ctx->height = height;
    ctx->pitch  = pitch;

    ctx->rc.l   = 0;
    ctx->rc.t   = 0;
    ctx->rc.r   = width;
    ctx->rc.b   = height;

    ctx->rcu.l  = ctx->rcu.t = 0;
    ctx->rcu.r  = ctx->rcu.b = 0;

    return 0;
};

void clear_context(ctx_t *ctx, color_t color)
{
	size_t size;

	size = ctx->pitch * ctx->height;

	if (size >= 1024)
		ctx->px_rect_simd(ctx->buffer, ctx->pitch, ctx->width, ctx->height, color);
	else
		px_rect_alu(ctx->buffer, ctx->pitch, ctx->width, ctx->height, color);

	ctx->rcu.l = 0;
	ctx->rcu.t = 0;
	ctx->rcu.r = ctx->width;
	ctx->rcu.b = ctx->height;
    ctx->dirty = 1;
};

void show_context(ctx_t *ctx)
{
    struct blit_call bc;
    int ret;

    bc.dstx   = ctx->x;
    bc.dsty   = ctx->y;
    bc.w      = ctx->width;
    bc.h      = ctx->height;
    bc.srcx   = 0;
    bc.srcy   = 0;
    bc.srcw   = ctx->width;
    bc.srch   = ctx->height;
    bc.stride = ctx->pitch;
    bc.bitmap = ctx->buffer;

    __asm__ __volatile__(
    "int $0x40":"=a"(ret):"a"(73), "b"(0x00),
    "c"(&bc):"memory");

    ctx->dirty = 0;
};

void scroll_context(ctx_t *ctx, int dst_y, int  src_y, int rows)
{
    char *dst;
    char *src;

    dst = ctx->buffer + dst_y * ctx->pitch;
    src = ctx->buffer + src_y * ctx->pitch;

    __builtin_memmove(dst, src, rows * ctx->pitch);
    ctx->dirty = 1;
}

static int clip_rect(const rect_t *clip, rect_t *rc)
{
	if (rc->l > rc->r)
		return 1;
	if (rc->t > rc->b)
		return 1;

	if (rc->l < clip->l)
		rc->l = clip->l;
	else if (rc->l >= clip->r)
		return 1;

	if (rc->t < clip->t)
		rc->t = clip->t;
	else if (rc->t >= clip->b)
		return 1;

	if (rc->r < clip->l)
		return 1;
	else if (rc->r > clip->r)
		rc->r = clip->r;

	if (rc->b < clip->t)
		return 1;
	else if (rc->b > clip->b)
		rc->b = clip->b;

	if ((rc->l == rc->r) ||
		(rc->t == rc->b))
		return 1;
	return 0;
}

int px_hline(ctx_t*ctx, int x, int y, int width, color_t color)
{
    char *dst_addr;

    int xr = x + width;

    if(y < ctx->rc.t)
        return 0;
    else if(y >= ctx->rc.b)
        return 0;

    if(x < ctx->rc.l)
        x = ctx->rc.l;
    else if(x >= ctx->rc.r)
        return 0;

    if(xr <= ctx->rc.l)
        return 0;
    else if(xr > ctx->rc.r)
        xr = ctx->rc.r;

    dst_addr = ctx->buffer;
    dst_addr+= ctx->pitch * y + x * sizeof(color_t);

    __asm__ __volatile__
    (" cld; rep stosl\n\t"
      :: "D" (dst_addr),"c" (xr-x), "a" (color)
      : "flags");
};

void px_vline(ctx_t*ctx, int x, int y, int height, color_t color)
{
    char *dst_addr;

    int yb = y + height;

    if(x < ctx->rc.l)
        return;
    else if(x >= ctx->rc.r)
        return;

    if(y < ctx->rc.t)
        y = ctx->rc.t;
    else if(y >= ctx->rc.b)
        return;

    if(yb <= ctx->rc.t)
        return;
    else if(yb > ctx->rc.b)
        yb = ctx->rc.b;

    dst_addr = ctx->buffer;
    dst_addr+= ctx->pitch * y + x * sizeof(color_t);

    while(y < yb)
    {
        color_t *t = (color_t*)dst_addr;
        *t = color;
        y++;
        dst_addr+= ctx->pitch;
    };
};

static int do_fill_rect(ctx_t *ctx, rect_t *rc, color_t color)
{
	if (!clip_rect(&ctx->rc, rc))
	{
		int w, h;
		char *dst_addr;

		w = rc->r - rc->l;
		h = rc->b - rc->t;

		dst_addr = ctx->buffer;
		dst_addr += ctx->pitch * rc->t + rc->l * sizeof(color_t);
		if (w * h >= 256)
			ctx->px_rect_simd(dst_addr, ctx->pitch, w, h, color);
		else
			px_rect_alu(dst_addr, ctx->pitch, w, h, color);
		return 1;
	};
	return 0;
};

void px_fill_rect(ctx_t *ctx, const rect_t *src, color_t color)
{
	rect_t rc = *src;
	int update;

	update = do_fill_rect(ctx, &rc, color);

	if(update)
	{
		if (rc.l < ctx->rcu.l)
			ctx->rcu.l = rc.l;
		if (rc.t < ctx->rcu.t)
			ctx->rcu.t = rc.t;
		if (rc.r > ctx->rcu.r)
			ctx->rcu.r = rc.r;
		if (rc.b > ctx->rcu.b)
			ctx->rcu.b = rc.b;
        ctx->dirty = 1;
	};
}

void px_fill_region(ctx_t *ctx, const rgn_t *rgn, color_t color)
{
	int update = 0;

	for (int i = 0; i < rgn->num_rects; i++)
	{
		rect_t rc = rgn->rects[i];
		update |= do_fill_rect(ctx, &rc, color);
	}

	if (update)
	{
		if (rgn->extents.l < ctx->rcu.l)
			ctx->rcu.l = rgn->extents.l;
		if (rgn->extents.t < ctx->rcu.t)
			ctx->rcu.t = rgn->extents.t;
		if (rgn->extents.r > ctx->rcu.r)
			ctx->rcu.r = rgn->extents.r;
		if (rgn->extents.b > ctx->rcu.b)
			ctx->rcu.b = rgn->extents.b;
        ctx->dirty = 1;
	};
}

void px_draw_glyph(ctx_t *ctx, const void *buffer, int pitch, const rect_t *rc, color_t color)
{
	rect_t rc_dst = *rc;
	int srcx, srcy;

	if (!clip_rect(&ctx->rc, &rc_dst))
	{
		int width;
		int height;
		unsigned char *dst = ctx->buffer;
		const unsigned char *src = buffer;

		width = rc_dst.r - rc_dst.l;
		height = rc_dst.b - rc_dst.t;

		srcx = rc_dst.l - rc->l;
		srcy = rc_dst.t - rc->t;
		dst += ctx->pitch * rc_dst.t + rc_dst.l * sizeof(color_t);
		src += pitch * srcy + srcx;
    	ctx->px_glyph(dst, ctx->pitch, src, pitch, width, height, color);

		if (rc_dst.l < ctx->rcu.l)
			ctx->rcu.l = rc_dst.l;
		if (rc_dst.t < ctx->rcu.t)
			ctx->rcu.t = rc_dst.t;
		if (rc_dst.r > ctx->rcu.r)
			ctx->rcu.r = rc_dst.r;
		if (rc_dst.b > ctx->rcu.b)
			ctx->rcu.b = rc_dst.b;

        ctx->dirty = 1;
	};
};
