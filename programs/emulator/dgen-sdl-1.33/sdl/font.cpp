/**
 * DGen's font renderer.
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "font.h"	/* The interface functions */

#ifndef FONT_VISIBLE
#define FONT_VISIBLE 24
#endif

extern const short *dgen_font_8x13[0x80];
extern const short *dgen_font_16x26[0x80];
extern const short *dgen_font_7x5[0x80];

const struct dgen_font dgen_font[] = {
	{ 16, 26, &dgen_font_16x26 },
	{ 8, 13, &dgen_font_8x13 },
	{ 7, 5, &dgen_font_7x5 },
	{ 0, 0, NULL }
};

const struct dgen_font *font_select(unsigned int max_width,
				    unsigned int max_height,
				    enum font_type type)
{
	const struct dgen_font *font = dgen_font;

	assert(type <= FONT_TYPE_AUTO);
	if (type != FONT_TYPE_AUTO) {
		// Make sure we're able to display at least one character.
		font = &font[type];
		if ((max_width < font->w) || (max_height < font->h))
			return NULL;
		return font;
	}
	// Chose a font able to display at least FONT_VISIBLE characters.
	while ((font->data != NULL) &&
	       ((font->h > max_height) ||
		((max_width / font->w) < FONT_VISIBLE))) {
		++font;
		continue;
	}
	if (font->data == NULL)
		return NULL;
	return font;
}

static void font_mark(uint8_t *buf,
		      unsigned int max_width, unsigned int max_height,
		      unsigned int bytes_per_pixel, unsigned int pitch,
		      unsigned int mark_x,
		      unsigned int font_w, unsigned int font_h)
{
	unsigned int y;

	if (((mark_x + font_w) > max_width) || (max_height < font_h))
		return;
	buf += (mark_x * bytes_per_pixel);
	for (y = 0; (y != font_h); ++y) {
		unsigned int x;
		unsigned int len = (bytes_per_pixel * font_w);

		for (x = 0; (x < len); ++x)
			buf[x] ^= 0xff;
		buf += pitch;
	}
}

size_t font_text_width(const char *msg, size_t len,
		       unsigned int max_width, unsigned int max_height,
		       enum font_type type)
{
	const struct dgen_font *font =
		font_select(max_width, max_height, type);
	size_t width = 0;

	if (font == NULL)
		return 0;
	while ((*msg != '\0') && (len)) {
		width += font->w;
		++msg;
		--len;
	}
	return width;
}

size_t font_text_max_len(unsigned int max_width, unsigned int max_height,
			 enum font_type type)
{
	const struct dgen_font *font =
		font_select(max_width, max_height, type);

	if (font == NULL)
		return ~(size_t)0;
	return (max_width / font->w);
}

size_t font_text(uint8_t *buf,
		 unsigned int max_width, unsigned int max_height,
		 unsigned int bytes_per_pixel, unsigned int pitch,
		 const char *msg, size_t len, unsigned int mark,
		 enum font_type type)
{
	const struct dgen_font *font;
	uint8_t *p_max;
	size_t r;
	unsigned int x;

	if (len == 0)
		return 0;
	if ((font = font_select(max_width, max_height, type)) == NULL) {
		printf("info: %.*s\n", (unsigned int)len, msg);
		if (mark <= len) {
			printf("      ");
			for (x = 0; (x != mark); ++x)
				putchar(' ');
			puts("^");
		}
		return len;
	}
	p_max = (buf + (pitch * max_height));
	for (x = 0, r = 0;
	     ((msg[r] != '\0') && (r != len) && ((x + font->w) <= max_width));
	     ++r, x += font->w) {
		const short *glyph = (*font->data)[(msg[r] & 0x7f)];
		uint8_t *p = (buf + (x * bytes_per_pixel));
		unsigned int n = 0;
		short g;

		if (glyph == NULL)
			continue;
		while ((g = *glyph) != -1) {
			unsigned int i;

			p += (((n += g) / font->w) * pitch);
			n %= font->w;
			for (i = 0; (i < bytes_per_pixel); ++i) {
				uint8_t *tmp = &p[((n * bytes_per_pixel) + i)];

				if (tmp < p_max)
					*tmp = 0xff;
			}
			++glyph;
		}
		if (r == mark)
			font_mark(buf, max_width, max_height, bytes_per_pixel,
				  pitch, x, font->w, font->h);
	}
	if (r == mark)
		font_mark(buf, max_width, max_height, bytes_per_pixel, pitch,
			  x, font->w, font->h);
	return r;
}
