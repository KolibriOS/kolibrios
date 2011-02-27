#include <stdio.h>
#include <stdlib.h>
#include <config.h>
#include "pixman-private.h"
#include "pixman.h"

#define FALSE 0
#define TRUE 1

/* Randomly decide between 32 and 16 bit
 *
 * Allocate bits with random width, stride and height
 *
 * Then make up some random offset (dx, dy)
 *
 * Then make an image with those values.
 *
 * Do this for both source and destination
 *
 * Composite them together using OVER.
 *
 * The bits in the source and the destination should have
 * recognizable colors so that the result can be verified.
 *
 * Ie., walk the bits and verify that they have been composited.
 */

static int
get_rand (int bound)
{
    return rand () % bound;
}

static pixman_image_t *
make_image (int width, int height, pixman_bool_t src, int *rx, int *ry)
{
    pixman_format_code_t format;
    pixman_image_t *image;
    pixman_region32_t region;
    uint8_t *bits;
    int stride;
    int bpp;
    int dx, dy;
    int i, j;

    if (src)
	format = PIXMAN_a8r8g8b8;
    else
	format = PIXMAN_r5g6b5;

    bpp = PIXMAN_FORMAT_BPP (format) / 8;

    stride = width + get_rand (width);
    stride += (stride & 1);		/* Make it an even number */

    bits = malloc (height * stride * bpp);

    for (j = 0; j < height; ++j)
    {
	for (i = 0; i < width; ++i)
	{
	    uint8_t *pixel = bits + (stride * j + i) * bpp;

	    if (src)
		*(uint32_t *)pixel = 0x7f00007f;
	    else
		*(uint16_t *)pixel = 0xf100;
	}
    }

    dx = dy = 0;

    dx = get_rand (500);
    dy = get_rand (500);

    if (!src)
    {
	/* Now simulate the bogus X server translations */
	bits -= (dy * stride + dx) * bpp;
    }

    image = pixman_image_create_bits (
	format, width, height, (uint32_t *)bits, stride * bpp);

    if (!src)
    {
	/* And add the bogus clip region */
	pixman_region32_init_rect (&region, dx, dy, dx + width, dy + height);

	pixman_image_set_clip_region32 (image, &region);
    }

    pixman_image_set_source_clipping (image, TRUE);

    if (src)
    {
	pixman_transform_t trans;

	pixman_transform_init_identity (&trans);

	pixman_transform_translate (&trans,
				    NULL,
				    - pixman_int_to_fixed (width / 2),
				    - pixman_int_to_fixed (height / 2));

	pixman_transform_scale (&trans,
				NULL,
				pixman_double_to_fixed (0.5),
				pixman_double_to_fixed (0.5));

	pixman_transform_translate (&trans,
				    NULL,
				    pixman_int_to_fixed (width / 2),
				    pixman_int_to_fixed (height / 2));

	pixman_image_set_transform (image, &trans);
	pixman_image_set_filter (image, PIXMAN_FILTER_BILINEAR, NULL, 0);
	pixman_image_set_repeat (image, PIXMAN_REPEAT_PAD);
    }

    if (!src)
    {
	*rx = dx;
	*ry = dy;
    }
    else
    {
	*rx = *ry = 0;
    }

    return image;
}

int
main ()
{
    pixman_image_t *src, *dest;
    int src_x, src_y, dest_x, dest_y;
    int i, j;
    int width = get_rand (499) + 1;
    int height = get_rand (499) + 1;

    src = make_image (width, height, TRUE, &src_x, &src_y);
    dest = make_image (width, height, FALSE, &dest_x, &dest_y);

    pixman_image_composite (
	PIXMAN_OP_OVER, src, NULL, dest,
	src_x, src_y,
	-1, -1,
	dest_x, dest_y,
	width, height);

    for (i = 0; i < height; ++i)
    {
	for (j = 0; j < width; ++j)
	{
	    uint8_t *bits = (uint8_t *)dest->bits.bits;
	    int bpp = PIXMAN_FORMAT_BPP (dest->bits.format) / 8;
	    int stride = dest->bits.rowstride * 4;

	    uint8_t *pixel =
		bits + (i + dest_y) * stride + (j + dest_x) * bpp;

	    if (*(uint16_t *)pixel != 0x788f)
	    {
		printf ("bad pixel %x\n", *(uint16_t *)pixel);
		assert (*(uint16_t *)pixel == 0x788f);
	    }
	}
    }

    return 0;
}
