
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <freetype/freetype.h>

#include "SDL.h"
#include "SDL_ttf.h"

/* Macro to convert a character to a Unicode value -- assume already Unicode */
#define UNICODE(c)	c

/* Round a float up to the nearest integeter and return that integer */
static int round(float x)
{
	int value;

	value = (int)x;
	if ( x > value ) {
		value = value + 1;
	} else
	if ( x < value ) {
		value = value - 1;
	}
	return value;
}

/* The structure used to hold glyph information (cached) */
struct glyph {
	int cached;
	TT_Raster_Map bitmap;
	TT_Raster_Map pixmap;
	int minx;
	int maxx;
	int miny;
	int maxy;
	int advance;
};

/* The structure used to hold internal font information */
struct _TTF_Font {
	TT_Face face;
	TT_Instance inst;
	TT_Glyph glyph;
	TT_CharMap map;

	/* Font metrics */
	int pointsize;
	int height;  /* ascent - descent */
	float ascent;
	float descent;
	float lineskip;

	/* The font style */
	int style;

	/* Extra width in glyph bounds for text styles */
	int glyph_overhang;
	float glyph_italics;

	/* For now, support Latin-1 character set caching */
	struct glyph *current;
	struct glyph cache[256];
	struct glyph scratch;
};

/* The FreeType font engine */
static TT_Engine engine;

int TTF_Init(void)
{
	int error;

	error = TT_Init_FreeType(&engine);
	if ( error ) {
		SDL_SetError("Couldn't init FreeType engine");
		return(-1);
	}
	return(0);
}

TTF_Font *TTF_OpenFont(const char *file, int ptsize)
{
	TTF_Font *font;
	TT_Face_Properties properties;
	TT_Instance_Metrics imetrics;
	int i, n;
	TT_UShort platform, encoding;
	TT_Error error;

	font = (TTF_Font *)malloc(sizeof(*font));
	if ( font == NULL ) {
		SDL_SetError("Out of memory");
		return(NULL);
	}
	memset(font, 0, sizeof(*font));

	/* Open the font and create ancillary data */
	error = TT_Open_Face(engine, file, &font->face);
	if ( error ) {
		SDL_SetError("Couldn't load font file");
		free(font);
		return(NULL);
	}
	error = TT_New_Glyph(font->face, &font->glyph);
	if ( error ) {
		SDL_SetError("Couldn't create glyph container");
		TTF_CloseFont(font);
		return(NULL);
	}
	error = TT_New_Instance(font->face, &font->inst);
	if ( error ) {
		SDL_SetError("Couldn't create font instance");
		TTF_CloseFont(font);
		return(NULL);
	}

	/* Set the display resolution */
	error = TT_Set_Instance_Resolutions(font->inst, 72, 72);
	if ( error ) {
		SDL_SetError("Couldn't set font resolution");
		TTF_CloseFont(font);
		return(NULL);
	}
	error = TT_Set_Instance_CharSize(font->inst, ptsize*64);
	if ( error ) {
		SDL_SetError("Couldn't set font size");
		TTF_CloseFont(font);
		return(NULL);
	}

	/* Get a Unicode mapping for this font */
	n = TT_Get_CharMap_Count(font->face);
	for ( i=0; i<n; ++i ) {
		TT_Get_CharMap_ID(font->face, i, &platform, &encoding);
		if ( ((platform == TT_PLATFORM_MICROSOFT) &&
		                  (encoding == TT_MS_ID_UNICODE_CS)) ||
		     ((platform == TT_PLATFORM_APPLE_UNICODE) &&
		                  (encoding == TT_APPLE_ID_DEFAULT)) ) {
			TT_Get_CharMap(font->face, i, &font->map);
			break;
		}
	}
	if ( i == n ) {
		SDL_SetError("Font doesn't have a Unicode mapping");
		TTF_CloseFont(font);
		return(NULL);
	}

	/* Get the font metrics for this font */
	TT_Get_Face_Properties(font->face, &properties );
	TT_Get_Instance_Metrics(font->inst, &imetrics);
	font->pointsize = imetrics.y_ppem;
	font->ascent = (float)properties.horizontal->Ascender /
	                properties.header->Units_Per_EM;
	font->ascent *= font->pointsize;
	font->descent = (float)properties.horizontal->Descender /
	                 properties.header->Units_Per_EM;
	font->descent *= font->pointsize;
	font->lineskip = (float)properties.horizontal->Line_Gap /
	                  properties.header->Units_Per_EM;
	font->lineskip *= font->pointsize;
	font->height = round(font->ascent - font->descent);

	/* Set the default font style */
	font->style = TTF_STYLE_NORMAL;
	font->glyph_overhang = font->pointsize/10;
	/* x offset = cos(((90.0-12)/360)*2*M_PI), or 12 degree angle */
	font->glyph_italics = 0.207;
	font->glyph_italics *= font->height;

	return(font);
}

static void Flush_Glyph(struct glyph *glyph)
{
	if ( glyph->bitmap.bitmap ) {
		free(glyph->bitmap.bitmap);
		glyph->bitmap.bitmap = 0;
	}
	if ( glyph->pixmap.bitmap ) {
		free(glyph->pixmap.bitmap);
		glyph->pixmap.bitmap = 0;
	}
	glyph->cached = 0;
}
	
static void Flush_Cache(TTF_Font *font)
{
	int i;

	for ( i=0; i<(sizeof font->cache)/(sizeof font->cache[0]); ++i ) {
		if ( font->cache[i].cached ) {
			Flush_Glyph(&font->cache[i]);
		}
	}
	if ( font->scratch.cached ) {
		Flush_Glyph(&font->scratch);
	}
}

static TT_Error Load_Glyph(TTF_Font *font, Uint16 ch, struct glyph *glyph)
{
	TT_UShort index;
	TT_Glyph_Metrics metrics;
	TT_Outline outline;
	int x_offset;
	int y_offset;
	TT_Error error;

	/* Load the glyph */
	index = TT_Char_Index(font->map, UNICODE(ch));
	error = TT_Load_Glyph(font->inst, font->glyph, index, TTLOAD_DEFAULT);
	if ( error ) return error;

	/* Get the bounding box */
	TT_Get_Glyph_Metrics(font->glyph, &metrics);
	glyph->minx = (metrics.bbox.xMin & -64) / 64;
	glyph->maxx = ((metrics.bbox.xMax + 63) & -64) / 64;
	glyph->miny = (metrics.bbox.yMin & -64) / 64;
	glyph->maxy = ((metrics.bbox.yMax + 63) & -64) / 64;
	glyph->advance = (metrics.advance & -64) / 64;

	/* Adjust for bold and italic text */
	if ( font->style & TTF_STYLE_BOLD ) {
		glyph->maxx += font->glyph_overhang;
	}
	if ( font->style & TTF_STYLE_ITALIC ) {
		glyph->maxx += round(font->glyph_italics);
	}

	/* Get the bitmap memory */
	glyph->bitmap.width = ((glyph->maxx - glyph->minx) + 7) & ~7;
	glyph->bitmap.rows = font->height;
	glyph->bitmap.cols = glyph->bitmap.width/8;
	glyph->bitmap.flow = TT_Flow_Down;
	glyph->bitmap.size = (glyph->bitmap.rows * glyph->bitmap.cols);
	if ( glyph->bitmap.size ) {
		glyph->bitmap.bitmap = malloc(glyph->bitmap.size);
		if ( ! glyph->bitmap.bitmap ) {
			error = TT_Err_Out_Of_Memory;
			goto was_error;
		}
		memset(glyph->bitmap.bitmap, 0, glyph->bitmap.size);
	} else {
		glyph->bitmap.bitmap = 0;
	}

	/* Get the pixmap memory */
	glyph->pixmap.width = ((glyph->maxx - glyph->minx) + 3) & ~3;
	glyph->pixmap.rows = font->height;
	glyph->pixmap.cols = glyph->pixmap.width;
	glyph->pixmap.flow = TT_Flow_Down;
	glyph->pixmap.size = (glyph->pixmap.rows * glyph->pixmap.cols);
	if ( glyph->pixmap.size ) {
		glyph->pixmap.bitmap = malloc(glyph->pixmap.size);
		if ( ! glyph->pixmap.bitmap ) {
			error = TT_Err_Out_Of_Memory;
			goto was_error;
		}
		memset(glyph->pixmap.bitmap, 0, glyph->pixmap.size);
	} else {
		glyph->pixmap.bitmap = 0;
	}

	/* Render the glyph into the bitmap and pixmap */
	error = TT_Get_Glyph_Outline(font->glyph, &outline);
	/* Handle the italic style */
	if ( font->style & TTF_STYLE_ITALIC ) {
		TT_Matrix shear;

		shear.xx = 1<<16;
		shear.xy = (int)(font->glyph_italics*(1<<16))/font->height;
		shear.yx = 0;
		shear.yy = 1<<16;
		TT_Transform_Outline(&outline, &shear);
	}
	x_offset = -glyph->minx * 64;
	y_offset = -round(font->descent) * 64;
	TT_Translate_Outline(&outline, x_offset, y_offset);
	error += TT_Get_Outline_Bitmap(engine, &outline, &glyph->bitmap);
	error += TT_Get_Outline_Pixmap(engine, &outline, &glyph->pixmap);
	/* Handle the bold style */
	if ( font->style & TTF_STYLE_BOLD ) {
		int row, col;
		int offset;
		int pixel;
		Uint8 *pixmap;

		/* The bitmap is easy, just render another copy */
		for ( offset=0; offset < font->glyph_overhang; ++offset ) {
			TT_Translate_Outline(&outline, 64, 0);
			error += TT_Get_Outline_Bitmap(engine,
			                               &outline,&glyph->bitmap);
		}
		x_offset += font->glyph_overhang*64;

		/* The pixmap is a little harder, we have to add and clamp */
		for ( row=glyph->pixmap.rows-1; row >= 0; --row ) {
			pixmap = (Uint8 *)glyph->pixmap.bitmap +
			                  row*glyph->pixmap.cols;
			for (offset=1; offset<=font->glyph_overhang; ++offset) {
				for (col=glyph->pixmap.cols-1; col > 0; --col) {
					pixel=(pixmap[col]+pixmap[col-1]);
					if ( pixel > 4 ) {
						pixel = 4;
					}
					pixmap[col] = (Uint8)pixel;
				}
			}
		}
	}
	TT_Translate_Outline(&outline, -x_offset, -y_offset);
was_error:
	if ( error ) {
		if ( glyph->bitmap.bitmap ) {
			free(glyph->bitmap.bitmap);
			glyph->bitmap.bitmap = 0;
		}
		if ( glyph->pixmap.bitmap ) {
			free(glyph->pixmap.bitmap);
			glyph->pixmap.bitmap = 0;
		}
		return error;
	}

	/* We're done, mark this glyph cached */
	glyph->cached = ch;
	return TT_Err_Ok;
}

static TT_Error Find_Glyph(TTF_Font *font, Uint16 ch)
{
	int retval;

	retval = 0;
	if ( ch < 256 ) {
		font->current = &font->cache[ch];
	} else {
		if ( font->scratch.cached != ch ) {
			Flush_Glyph(&font->scratch);
		}
		font->current = &font->scratch;
	}
	if ( ! font->current->cached ) {
		retval = Load_Glyph(font, ch, font->current);
	}
	return retval;
}

void TTF_CloseFont(TTF_Font *font)
{
	Flush_Cache(font);
	TT_Close_Face(font->face);
	free(font);
}

static Uint16 *ASCII_to_UNICODE(Uint16 *unicode, const char *text, int len)
{
	int i;

	for ( i=0; i < len; ++i ) {
		unicode[i] = ((const unsigned char *)text)[i];
	}
	unicode[i] = 0;

	return unicode;
}

static Uint16 *UTF8_to_UNICODE(Uint16 *unicode, const char *utf8, int len)
{
	int i, j;
	Uint16 ch;

	for ( i=0, j=0; i < len; ++i, ++j ) {
		ch = ((const unsigned char *)utf8)[i];
		if ( ch >= 0xF0 ) {
			ch  =  (Uint16)(utf8[i]&0x07) << 18;
			ch |=  (Uint16)(utf8[++i]&0x3F) << 12;
			ch |=  (Uint16)(utf8[++i]&0x3F) << 6;
			ch |=  (Uint16)(utf8[++i]&0x3F);
		} else
		if ( ch >= 0xE0 ) {
			ch  =  (Uint16)(utf8[i]&0x3F) << 12;
			ch |=  (Uint16)(utf8[++i]&0x3F) << 6;
			ch |=  (Uint16)(utf8[++i]&0x3F);
		} else
		if ( ch >= 0xC0 ) {
			ch  =  (Uint16)(utf8[i]&0x3F) << 6;
			ch |=  (Uint16)(utf8[++i]&0x3F);
		}
		unicode[j] = ch;
	}
	unicode[j] = 0;

	return unicode;
}

int TTF_FontHeight(TTF_Font *font)
{
	return(font->height);
}

int TTF_FontAscent(TTF_Font *font)
{
       return(round(font->ascent));
}

int TTF_FontDescent(TTF_Font *font)
{
	return(round(font->descent));
}

int TTF_FontLineSkip(TTF_Font *font)
{
	return(round(font->lineskip));
}

int TTF_GlyphMetrics(TTF_Font *font, Uint16 ch,
                     int* minx, int* maxx, int* miny, int* maxy, int* advance)
{
	TT_Error error;

	error = Find_Glyph(font, ch);

	if ( error ) {
		return -1;
	}

	if ( minx ) {
		*minx = font->current->minx;
	}
	if ( maxx ) {
		*maxx = font->current->maxx;
	}
	if ( miny ) {
		*miny = font->current->miny;
	}
	if ( maxy ) {
		*maxy = font->current->maxy;
	}
	if ( advance ) {
		*advance = font->current->advance;
	}
	return 0;
}

int TTF_SizeText(TTF_Font *font, const char *text, int *w, int *h)
{
	Uint16 *unicode_text;
	int unicode_len;
	int status;

	/* Copy the Latin-1 text to a UNICODE text buffer */
	unicode_len = strlen(text);
	unicode_text = (Uint16 *)malloc((unicode_len+1)*(sizeof *unicode_text));
	if ( unicode_text == NULL ) {
		SDL_SetError("Out of memory");
		return -1;
	}
	ASCII_to_UNICODE(unicode_text, text, unicode_len);

	/* Render the new text */
	status = TTF_SizeUNICODE(font, unicode_text, w, h);

	/* Free the text buffer and return */
	free(unicode_text);
	return status;
}

int TTF_SizeUTF8(TTF_Font *font, const char *text, int *w, int *h)
{
	Uint16 *unicode_text;
	int unicode_len;
	int status;

	/* Copy the UTF-8 text to a UNICODE text buffer */
	unicode_len = strlen(text);
	unicode_text = (Uint16 *)malloc((unicode_len+1)*(sizeof *unicode_text));
	if ( unicode_text == NULL ) {
		SDL_SetError("Out of memory");
		return -1;
	}
	UTF8_to_UNICODE(unicode_text, text, unicode_len);

	/* Render the new text */
	status = TTF_SizeUNICODE(font, unicode_text, w, h);

	/* Free the text buffer and return */
	free(unicode_text);
	return status;
}

int TTF_SizeUNICODE(TTF_Font *font, const Uint16 *text, int *w, int *h)
{
	int status;
	const Uint16 *ch;
	int x, z, minx, maxx;
	TT_Error error;

	/* Initialize everything to 0 */
	status = 0;
	minx = maxx = 0;

	/* Load each character and sum it's bounding box */
	x= 0;
	for ( ch=text; *ch; ++ch ) {
		error = Find_Glyph(font, *ch);
		if ( ! error ) {
			z = x + font->current->minx;
			if ( minx > z ) {
				minx = z;
			}
			if ( font->style & TTF_STYLE_BOLD ) {
				x += font->glyph_overhang;
			}
			if ( font->current->advance > font->current->maxx ) {
				z = x + font->current->advance;
			} else {
				z = x + font->current->maxx;
			}
			if ( maxx < z ) {
				maxx = z;
			}
			x += font->current->advance;
		}
	}

	/* Fill the bounds rectangle */
	if ( w ) {
		*w = (maxx - minx);
	}
	if ( h ) {
		*h = font->height;
	}
	return status;
}

/* Convert the Latin-1 text to UNICODE and render it
*/
SDL_Surface *TTF_RenderText_Solid(TTF_Font *font,
				const char *text, SDL_Color fg)
{
	SDL_Surface *textbuf;
	Uint16 *unicode_text;
	int unicode_len;

	/* Copy the Latin-1 text to a UNICODE text buffer */
	unicode_len = strlen(text);
	unicode_text = (Uint16 *)malloc((unicode_len+1)*(sizeof *unicode_text));
	if ( unicode_text == NULL ) {
		SDL_SetError("Out of memory");
		return(NULL);
	}
	ASCII_to_UNICODE(unicode_text, text, unicode_len);

	/* Render the new text */
	textbuf = TTF_RenderUNICODE_Solid(font, unicode_text, fg);

	/* Free the text buffer and return */
	free(unicode_text);
	return(textbuf);
}

/* Convert the UTF-8 text to UNICODE and render it
*/
SDL_Surface *TTF_RenderUTF8_Solid(TTF_Font *font,
				const char *text, SDL_Color fg)
{
	SDL_Surface *textbuf;
	Uint16 *unicode_text;
	int unicode_len;

	/* Copy the UTF-8 text to a UNICODE text buffer */
	unicode_len = strlen(text);
	unicode_text = (Uint16 *)malloc((unicode_len+1)*(sizeof *unicode_text));
	if ( unicode_text == NULL ) {
		SDL_SetError("Out of memory");
		return(NULL);
	}
	UTF8_to_UNICODE(unicode_text, text, unicode_len);

	/* Render the new text */
	textbuf = TTF_RenderUNICODE_Solid(font, unicode_text, fg);

	/* Free the text buffer and return */
	free(unicode_text);
	return(textbuf);
}

SDL_Surface *TTF_RenderUNICODE_Solid(TTF_Font *font,
				const Uint16 *text, SDL_Color fg)
{
	int xstart, width;
	int w, h;
	SDL_Surface *textbuf;
	SDL_Palette *palette;
	const Uint16 *ch;
	Uint8 *src, *dst;
	int row, col;
	TT_Error error;

	/* Get the dimensions of the text surface */
	if ( (TTF_SizeUNICODE(font, text, &w, &h) < 0) || !w ) {
		TTF_SetError("Text has zero width");
		return(NULL);
	}

	/* Create the target surface */
	width = w;
	w = (w+7)&~7;
	textbuf = SDL_AllocSurface(SDL_SWSURFACE, w, h, 8, 0, 0, 0, 0);
	if ( textbuf == NULL ) {
		return(NULL);
	}

	/* Fill the palette with the foreground color */
	palette = textbuf->format->palette;
	palette->colors[0].r = 255-fg.r;
	palette->colors[0].g = 255-fg.g;
	palette->colors[0].b = 255-fg.b;
	palette->colors[1].r = fg.r;
	palette->colors[1].g = fg.g;
	palette->colors[1].b = fg.b;
	SDL_SetColorKey(textbuf, SDL_SRCCOLORKEY, 0);

	/* Load and render each character */
	xstart = 0;
	for ( ch=text; *ch; ++ch ) {
		error = Find_Glyph(font, *ch);
		if ( ! error ) {
			w = font->current->bitmap.width;
			src = (Uint8 *)font->current->bitmap.bitmap;
			for ( row = 0; row < h; ++row ) {
				dst = (Uint8 *)textbuf->pixels +
				               row * textbuf->pitch +
				               xstart + font->current->minx;
				for ( col = 0; col < w; col += 8 ) {
					Uint8 c = *src++;
					*dst++ |= (c&0x80)>>7;
					c <<= 1;
					*dst++ |= (c&0x80)>>7;
					c <<= 1;
					*dst++ |= (c&0x80)>>7;
					c <<= 1;
					*dst++ |= (c&0x80)>>7;
					c <<= 1;
					*dst++ |= (c&0x80)>>7;
					c <<= 1;
					*dst++ |= (c&0x80)>>7;
					c <<= 1;
					*dst++ |= (c&0x80)>>7;
					c <<= 1;
					*dst++ |= (c&0x80)>>7;
				}
			}
			xstart += font->current->advance;
			if ( font->style & TTF_STYLE_BOLD ) {
				xstart += font->glyph_overhang;
			}
		}
	}
	/* Handle the underline style */
	if ( font->style & TTF_STYLE_UNDERLINE ) {
		int row_offset;

		row_offset = round(font->ascent) + 1;
		if ( row_offset > font->height ) {
			row_offset = font->height-1;
		}
		memset((Uint8 *)textbuf->pixels+row_offset*textbuf->pitch,
								1, width);
	}
	return(textbuf);
}

SDL_Surface *TTF_RenderGlyph_Solid(TTF_Font *font, Uint16 ch, SDL_Color fg)
{
	SDL_Surface *textbuf;
	SDL_Palette *palette;
	Uint8 *src, *dst;
	int row, col;
	TT_Error error;
	struct glyph *glyph;

	/* Get the glyph itself */
	error = Find_Glyph(font, ch);
	if ( error ) {
		return(NULL);
	}
	glyph = font->current;

	/* Create the target surface */
	textbuf = SDL_CreateRGBSurface(SDL_SWSURFACE,
	              glyph->bitmap.width, glyph->bitmap.rows, 8, 0, 0, 0, 0);
	if ( ! textbuf ) {
		return(NULL);
	}

	/* Fill the palette with the foreground color */
	palette = textbuf->format->palette;
	palette->colors[0].r = 255-fg.r;
	palette->colors[0].g = 255-fg.g;
	palette->colors[0].b = 255-fg.b;
	palette->colors[1].r = fg.r;
	palette->colors[1].g = fg.g;
	palette->colors[1].b = fg.b;
	SDL_SetColorKey(textbuf, SDL_SRCCOLORKEY, 0);

	/* Load and render each character */
	src = (Uint8 *)font->current->bitmap.bitmap;
	for ( row = 0; row < textbuf->h; ++row ) {
		dst = (Uint8 *)textbuf->pixels + row * textbuf->pitch;
		for ( col = 0; col < textbuf->w; col += 8 ) {
			Uint8 c = *src++;
			*dst++ |= (c&0x80)>>7;
			c <<= 1;
			*dst++ |= (c&0x80)>>7;
			c <<= 1;
			*dst++ |= (c&0x80)>>7;
			c <<= 1;
			*dst++ |= (c&0x80)>>7;
			c <<= 1;
			*dst++ |= (c&0x80)>>7;
			c <<= 1;
			*dst++ |= (c&0x80)>>7;
			c <<= 1;
			*dst++ |= (c&0x80)>>7;
			c <<= 1;
			*dst++ |= (c&0x80)>>7;
		}
	}

	/* Handle the underline style */
	if ( font->style & TTF_STYLE_UNDERLINE ) {
		int row_offset;

		row_offset = round(font->ascent) + 1;
		if ( row_offset > font->height ) {
			row_offset = font->height-1;
		}
		memset((Uint8 *)textbuf->pixels+row_offset*textbuf->pitch,
							1, textbuf->w);
	}
	return(textbuf);
}


/* Convert the Latin-1 text to UNICODE and render it
*/
SDL_Surface *TTF_RenderText_Shaded(TTF_Font *font,
				const char *text, SDL_Color fg, SDL_Color bg)
{
	SDL_Surface *textbuf;
	Uint16 *unicode_text;
	int unicode_len;

	/* Copy the Latin-1 text to a UNICODE text buffer */
	unicode_len = strlen(text);
	unicode_text = (Uint16 *)malloc((unicode_len+1)*(sizeof *unicode_text));
	if ( unicode_text == NULL ) {
		SDL_SetError("Out of memory");
		return(NULL);
	}
	ASCII_to_UNICODE(unicode_text, text, unicode_len);

	/* Render the new text */
	textbuf = TTF_RenderUNICODE_Shaded(font, unicode_text, fg, bg);

	/* Free the text buffer and return */
	free(unicode_text);
	return(textbuf);
}

/* Convert the UTF-8 text to UNICODE and render it
*/
SDL_Surface *TTF_RenderUTF8_Shaded(TTF_Font *font,
				const char *text, SDL_Color fg, SDL_Color bg)
{
	SDL_Surface *textbuf;
	Uint16 *unicode_text;
	int unicode_len;

	/* Copy the UTF-8 text to a UNICODE text buffer */
	unicode_len = strlen(text);
	unicode_text = (Uint16 *)malloc((unicode_len+1)*(sizeof *unicode_text));
	if ( unicode_text == NULL ) {
		SDL_SetError("Out of memory");
		return(NULL);
	}
	UTF8_to_UNICODE(unicode_text, text, unicode_len);

	/* Render the new text */
	textbuf = TTF_RenderUNICODE_Shaded(font, unicode_text, fg, bg);

	/* Free the text buffer and return */
	free(unicode_text);
	return(textbuf);
}

SDL_Surface *TTF_RenderUNICODE_Shaded(TTF_Font *font,
				const Uint16 *text, SDL_Color fg, SDL_Color bg)
{
	int xstart, width;
	int w, h;
	SDL_Surface *textbuf;
	SDL_Palette *palette;
	int index;
	int rdiff, gdiff, bdiff;
	const Uint16 *ch;
	Uint8 *src, *dst;
	int row, col;
	TT_Error error;

	/* Get the dimensions of the text surface */
	if ( (TTF_SizeUNICODE(font, text, &w, &h) < 0) || !w ) {
		TTF_SetError("Text has zero width");
		return(NULL);
	}

	/* Create the target surface */
	width = w;
	w = (w+3)&~3;
	textbuf = SDL_AllocSurface(SDL_SWSURFACE, w, h, 8, 0, 0, 0, 0);
	if ( textbuf == NULL ) {
		return(NULL);
	}

	/* Fill the palette with 5 levels of shading from bg to fg */
	palette = textbuf->format->palette;
	rdiff = fg.r - bg.r;
	gdiff = fg.g - bg.g;
	bdiff = fg.b - bg.b;
	for ( index=0; index<5; ++index ) {
		palette->colors[index].r = bg.r + (index*rdiff)/4;
		palette->colors[index].g = bg.g + (index*gdiff)/4;
		palette->colors[index].b = bg.b + (index*bdiff)/4;
	}
	/* The other 3 levels are used as overflow when ORing pixels */
	for ( ; index<8; ++index ) {
		palette->colors[index] = palette->colors[4];
	}

	/* Load and render each character */
	xstart = 0;
	for ( ch=text; *ch; ++ch ) {
		error = Find_Glyph(font, *ch);
		if ( ! error ) {
			w = font->current->pixmap.width;
			src = (Uint8 *)font->current->pixmap.bitmap;
			for ( row = 0; row < h; ++row ) {
				dst = (Uint8 *)textbuf->pixels +
				               row * textbuf->pitch +
				               xstart + font->current->minx;
				for ( col=w; col>0; col -= 4 ) {
					*dst++ |= *src++;
					*dst++ |= *src++;
					*dst++ |= *src++;
					*dst++ |= *src++;
				}
			}
			xstart += font->current->advance;
			if ( font->style & TTF_STYLE_BOLD ) {
				xstart += font->glyph_overhang;
			}
		}
	}
	/* Handle the underline style */
	if ( font->style & TTF_STYLE_UNDERLINE ) {
		int row_offset;

		row_offset = round(font->ascent) + 1;
		if ( row_offset > font->height ) {
			row_offset = font->height-1;
		}
		memset((Uint8 *)textbuf->pixels+row_offset*textbuf->pitch,
								4, width);
	}
	return(textbuf);
}

SDL_Surface *TTF_RenderGlyph_Shaded(TTF_Font *font,
				Uint16 ch, SDL_Color fg, SDL_Color bg)
{
	SDL_Surface *textbuf;
	SDL_Palette *palette;
	int index;
	int rdiff, gdiff, bdiff;
	Uint8 *src, *dst;
	int row, col;
	TT_Error error;
	struct glyph *glyph;

	/* Get the glyph itself */
	error = Find_Glyph(font, ch);
	if ( error ) {
		return(NULL);
	}
	glyph = font->current;

	/* Create the target surface */
	textbuf = SDL_CreateRGBSurface(SDL_SWSURFACE,
	              glyph->pixmap.width, glyph->pixmap.rows, 8, 0, 0, 0, 0);
	if ( ! textbuf ) {
		return(NULL);
	}

	/* Fill the palette with 5 levels of shading from bg to fg */
	palette = textbuf->format->palette;
	rdiff = fg.r - bg.r;
	gdiff = fg.g - bg.g;
	bdiff = fg.b - bg.b;
	for ( index=0; index<5; ++index ) {
		palette->colors[index].r = bg.r + (index*rdiff)/4;
		palette->colors[index].g = bg.g + (index*gdiff)/4;
		palette->colors[index].b = bg.b + (index*bdiff)/4;
	}

	/* Copy the character from the pixmap */
	for ( row=0; row<textbuf->h; ++row ) {
		src = glyph->pixmap.bitmap + row * glyph->pixmap.cols;
		dst = (Uint8 *)textbuf->pixels + row * textbuf->pitch;
		memcpy(dst, src, glyph->pixmap.cols);
	}

	/* Handle the underline style */
	if ( font->style & TTF_STYLE_UNDERLINE ) {
		int row_offset;

		row_offset = round(font->ascent) + 1;
		if ( row_offset > font->height ) {
			row_offset = font->height-1;
		}
		memset((Uint8 *)textbuf->pixels+row_offset*textbuf->pitch,
							4, glyph->pixmap.cols);
	}
	return(textbuf);
}

/* Convert the Latin-1 text to UNICODE and render it
*/
SDL_Surface *TTF_RenderText_Blended(TTF_Font *font,
				const char *text, SDL_Color fg)
{
	SDL_Surface *textbuf;
	Uint16 *unicode_text;
	int unicode_len;

	/* Copy the Latin-1 text to a UNICODE text buffer */
	unicode_len = strlen(text);
	unicode_text = (Uint16 *)malloc((unicode_len+1)*(sizeof *unicode_text));
	if ( unicode_text == NULL ) {
		SDL_SetError("Out of memory");
		return(NULL);
	}
	ASCII_to_UNICODE(unicode_text, text, unicode_len);

	/* Render the new text */
	textbuf = TTF_RenderUNICODE_Blended(font, unicode_text, fg);

	/* Free the text buffer and return */
	free(unicode_text);
	return(textbuf);
}

/* Convert the UTF-8 text to UNICODE and render it
*/
SDL_Surface *TTF_RenderUTF8_Blended(TTF_Font *font,
				const char *text, SDL_Color fg)
{
	SDL_Surface *textbuf;
	Uint16 *unicode_text;
	int unicode_len;

	/* Copy the UTF-8 text to a UNICODE text buffer */
	unicode_len = strlen(text);
	unicode_text = (Uint16 *)malloc((unicode_len+1)*(sizeof *unicode_text));
	if ( unicode_text == NULL ) {
		SDL_SetError("Out of memory");
		return(NULL);
	}
	UTF8_to_UNICODE(unicode_text, text, unicode_len);

	/* Render the new text */
	textbuf = TTF_RenderUNICODE_Blended(font, unicode_text, fg);

	/* Free the text buffer and return */
	free(unicode_text);
	return(textbuf);
}

SDL_Surface *TTF_RenderUNICODE_Blended(TTF_Font *font,
				const Uint16 *text, SDL_Color fg)
{
	int xstart, width;
	int w, h;
	SDL_Surface *textbuf;
#if SDL_VERSIONNUM(SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL) >= \
    SDL_VERSIONNUM(1, 1, 5)			/* The great alpha flip */
	Uint32 alpha_table[] = {
		(0)<<24,      (255-128)<<24, (255-64)<<24, (255-32)<<24,
		(255)<<24,    (255)<<24,     (255)<<24,    (255)<<24
	};
#else
	Uint32 alpha_table[] = {
		(255<<24), (128<<24), (64<<24), (32<<24), 0, 0, 0, 0
	};
#endif
	Uint32 pixel;
	const Uint16 *ch;
	Uint8 *src;
	Uint32 *dst;
	int row, col;
	TT_Error error;

	/* Get the dimensions of the text surface */
	if ( (TTF_SizeUNICODE(font, text, &w, &h) < 0) || !w ) {
		TTF_SetError("Text has zero width");
		return(NULL);
	}

	/* Create the target surface, 32-bit ARGB format */
	width = w;
	w = (w+3)&~3;
	textbuf = SDL_AllocSurface(SDL_SWSURFACE, w, h, 32,
	                           0x00FF0000, 0x0000FF00, 0x000000FF,
	                           0xFF000000);
	if ( textbuf == NULL ) {
		return(NULL);
	}

	/* Load and render each character */
	xstart = 0;
	for ( ch=text; *ch; ++ch ) {
		error = Find_Glyph(font, *ch);
		if ( ! error ) {
			w = font->current->pixmap.width;
			src = (Uint8 *)font->current->pixmap.bitmap;
			for ( row = 0; row < h; ++row ) {
				dst = (Uint32 *)textbuf->pixels +
				               row * textbuf->pitch/4 +
				               xstart + font->current->minx;
				for ( col=w; col>0; col -= 4 ) {
					*dst++ |= *src++;
					*dst++ |= *src++;
					*dst++ |= *src++;
					*dst++ |= *src++;
				}
			}
			xstart += font->current->advance;
			if ( font->style & TTF_STYLE_BOLD ) {
				xstart += font->glyph_overhang;
			}
		}
	}

	/* Handle the underline style */
	if ( font->style & TTF_STYLE_UNDERLINE ) {
		int row_offset;

		row_offset = round(font->ascent) + 1;
		if ( row_offset > font->height ) {
			row_offset = font->height-1;
		}
		dst = (Uint32 *)textbuf->pixels+row_offset*textbuf->pitch/4;
		for ( col=width; col > 0; ++col ) {
			*dst++ = 4;
		}
	}

	/* Build the alpha table */
	pixel = (fg.r<<16)|(fg.g<<8)|fg.b;
	for ( xstart = 0; xstart < 8; ++xstart ) {
		alpha_table[xstart] |= pixel;
	}

	/* Transform the alpha values */
	for ( row = 0; row < textbuf->h; ++row ) {
		dst = (Uint32 *)textbuf->pixels + row * textbuf->pitch/4;
		for ( col=textbuf->w; col>0; col -= 4 ) {
			*dst = alpha_table[*dst];
			++dst;
			*dst = alpha_table[*dst];
			++dst;
			*dst = alpha_table[*dst];
			++dst;
			*dst = alpha_table[*dst];
			++dst;
		}
	}
	return(textbuf);
}

SDL_Surface *TTF_RenderGlyph_Blended(TTF_Font *font, Uint16 ch, SDL_Color fg)
{
	SDL_Surface *textbuf;
#if SDL_VERSIONNUM(SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL) >= \
    SDL_VERSIONNUM(1, 1, 5)			/* The great alpha flip */
	Uint32 alpha_table[] = {
		(0)<<24,      (255-128)<<24, (255-64)<<24, (255-32)<<24,
		(255)<<24,    (255)<<24,     (255)<<24,    (255)<<24
	};
#else
	Uint32 alpha_table[] = {
		(255<<24), (128<<24), (64<<24), (32<<24), 0, 0, 0, 0
	};
#endif
	Uint32 pixel;
	Uint8 *src;
	Uint32 *dst;
	int row, col;
	TT_Error error;
	struct glyph *glyph;

	/* Get the glyph itself */
	error = Find_Glyph(font, ch);
	if ( error ) {
		return(NULL);
	}
	glyph = font->current;

	/* Create the target surface, 32-bit ARGB format */
	textbuf = SDL_CreateRGBSurface(SDL_SWSURFACE,
	              glyph->pixmap.width, glyph->pixmap.rows, 32,
	              0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	if ( ! textbuf ) {
		return(NULL);
	}

	/* Copy the character from the pixmap */
	for ( row=0; row<textbuf->h; ++row ) {
		src = glyph->pixmap.bitmap + row * glyph->pixmap.cols;
		dst = (Uint32 *)textbuf->pixels + row * textbuf->pitch/4;
		for ( col=0; col<glyph->pixmap.cols; ++col ) {
			*dst++ = *src++;
		}
	}

	/* Handle the underline style */
	if ( font->style & TTF_STYLE_UNDERLINE ) {
		int row_offset;

		row_offset = round(font->ascent) + 1;
		if ( row_offset > font->height ) {
			row_offset = font->height-1;
		}
		dst = (Uint32 *)textbuf->pixels+row_offset*textbuf->pitch/4;
		for ( col=glyph->pixmap.cols; col > 0; ++col ) {
			*dst++ = 4;
		}
	}

	/* Build the alpha table */
	pixel = (fg.r<<16)|(fg.g<<8)|fg.b;
	for ( col = 0; col < 8; ++col ) {
		alpha_table[col] |= pixel;
	}

	/* Transform the alpha values */
	for ( row = 0; row < textbuf->h; ++row ) {
		dst = (Uint32 *)textbuf->pixels + row * textbuf->pitch/4;
		for ( col=textbuf->w; col>0; col -= 4 ) {
			*dst = alpha_table[*dst];
			++dst;
			*dst = alpha_table[*dst];
			++dst;
			*dst = alpha_table[*dst];
			++dst;
			*dst = alpha_table[*dst];
			++dst;
		}
	}
	return(textbuf);
}

void TTF_SetFontStyle(TTF_Font *font, int style)
{
	font->style = style;
	Flush_Cache(font);
}

int TTF_GetFontStyle(TTF_Font *font)
{
	return(font->style);
}

void TTF_Quit(void)
{
	TT_Done_FreeType(engine);
}
