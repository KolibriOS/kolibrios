#ifndef __FONT_H__
#define __FONT_H__

#include <stddef.h>
#include <stdint.h>

enum font_type {
	FONT_TYPE_16X26,
	FONT_TYPE_8X13,
	FONT_TYPE_7X5,
	FONT_TYPE_AUTO
};

struct dgen_font {
	unsigned int w;
	unsigned int h;
	const short *(*data)[0x80];
};

extern const struct dgen_font dgen_font[];

extern const struct dgen_font *font_select(unsigned int max_width,
					   unsigned int max_height,
					   enum font_type type);
extern size_t font_text_width(const char *msg, size_t len,
			      unsigned int max_width, unsigned int max_height,
			      enum font_type type);
extern size_t font_text_max_len(unsigned int max_width,
				unsigned int max_height,
				enum font_type type);
extern size_t font_text(uint8_t *buf,
			unsigned int max_width, unsigned int max_height,
			unsigned int bytes_per_pixel, unsigned int pitch,
			const char *msg, size_t len, unsigned int mark,
			enum font_type type);

#endif /* __FONT_H__ */
