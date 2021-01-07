/*
 * gfx.c
 * Copyright (C) 1998 Brainchild Design - http://brainchilddesign.com/
 * 
 * Copyright (C) 2001 Chuck Mason <cemason@users.sourceforge.net>
 *
 * Copyright (C) 2002 Florian Schulze <crow@icculus.org>
 *
 * This file is part of Jump'n'Bump.
 *
 * Jump'n'Bump is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Jump'n'Bump is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "globals.h"
#include "SDL_endian.h"
#include "SDL.h"
#include "filter.h"


#include "jumpnbump64.xpm" 

SDL_Surface *icon;

int screen_width=400;
int screen_height=256;
int screen_pitch=400;
int scale_up=0;
int dirty_block_shift=4;

static SDL_Surface *jnb_surface;
static int fullscreen = 0;
static int vinited = 0;
static void *screen_buffer[2];
static int drawing_enable = 0;
static void *background = NULL;
static int background_drawn;
static void *mask = NULL;
static int dirty_blocks[2][25*16*2];

static SDL_Surface *load_xpm_from_array(char **xpm)
{
#define NEXT_TOKEN { \
	while ((*p != ' ') && (*p != '\t')) p++; \
	while ((*p == ' ') || (*p == '\t')) p++; }

	SDL_Surface *surface;
	char *p;
	int width;
	int height;
	int colors;
	int images;
	int color;
	int pal[256];
	int x,y;

	p = *xpm++;

	width = atoi(p);
	if (width <= 0)
		return NULL;
	NEXT_TOKEN;

	height = atoi(p);
	if (height <= 0)
		return NULL;
	NEXT_TOKEN;

	colors = atoi(p);
	if (colors <= 0)
		return NULL;
	NEXT_TOKEN;

	images = atoi(p);
	if (images <= 0)
		return NULL;

	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
	if (!surface)
		return NULL;

	SDL_SetColorKey(surface, SDL_SRCCOLORKEY, SDL_MapRGBA(surface->format, 0, 0, 0, 0));
	while (colors--) {
		p = *xpm++;

		color = *p++;
		NEXT_TOKEN;

		if (*p++ != 'c') {
			SDL_FreeSurface(surface);
			return NULL;
		}
		NEXT_TOKEN;

		if (*p == '#')
			pal[color] = strtoul(++p, NULL, 16) | 0xff000000;
		else
			pal[color] = 0;
	}

	y = 0;
	while (y < height) {
		int *pixels;

		p = *xpm++;

		pixels = (int *)&((char *)surface->pixels)[y++ * surface->pitch];
		x = 0;
		while (x < width) {
			Uint8 r,g,b,a;

			if (*p == '\0') {
				SDL_FreeSurface(surface);
				return NULL;
			}
			r = (pal[(int)*p] >> 16) & 0xff;
			b = (pal[(int)*p] & 0xff);
			g = (pal[(int)*p] >> 8) & 0xff;
			a = (pal[(int)*p] >> 24) & 0xff;
			pixels[x] = SDL_MapRGBA(surface->format, r, g, b, a);
			x++;
			p++;
		}
	}

	return surface;
}

unsigned char *get_vgaptr(int page, int x, int y)
{
	assert(drawing_enable==1);

	return (unsigned char *)screen_buffer[page] + (y*screen_pitch)+(x);
}


void set_scaling(int scale)
{
	if (scale==1) {
		screen_width=800;
		screen_height=512;
		scale_up=1;
		dirty_block_shift=5;
		screen_pitch=screen_width;
	} else {
		screen_width=400;
		screen_height=256;
		scale_up=0;
		dirty_block_shift=4;
		screen_pitch=screen_width;
	}
}

void open_screen(void)
{
	int lval = 0;
	int flags;

	lval = SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO );
	if (lval < 0) {
		fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
		//exit(EXIT_FAILURE);
	}

	flags = SDL_SWSURFACE;
	if (fullscreen)
		flags |= SDL_FULLSCREEN;
	jnb_surface = SDL_SetVideoMode(screen_width, screen_height, 8, flags);

	if (!jnb_surface) {
		fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	if(fullscreen)
		SDL_ShowCursor(0);
	else
		SDL_ShowCursor(1);

	SDL_WM_SetCaption("Jump'n'Bump","");

	icon=load_xpm_from_array(jumpnbump_xpm);
	if (icon==NULL) {
	    printf("Couldn't load icon\n");
	} else {
	    SDL_WM_SetIcon(icon,NULL);
	}

	vinited = 1;

	memset(dirty_blocks, 0, sizeof(dirty_blocks));

	screen_buffer[0]=malloc(screen_width*screen_height);
	screen_buffer[1]=malloc(screen_width*screen_height);

/*
	dirty_blocks[0]=malloc(sizeof(int)*25*16+1000);
	dirty_blocks[1]=malloc(sizeof(int)*25*16+1000);
*/

	return;
}


void fs_toggle()
{
	if (!vinited) {
		fullscreen ^= 1;
		return;
	}
	if (SDL_WM_ToggleFullScreen(jnb_surface))
		fullscreen ^= 1;
}


void wait_vrt(int mix)
{
	return;
}


void clear_page(int page, int color)
{
	int i,j;
	unsigned char *buf = get_vgaptr(page, 0, 0);

	assert(drawing_enable==1);

	for (i=0; i<(25*16); i++)
		dirty_blocks[page][i] = 1;

	for (i=0; i<screen_height; i++)
		for (j=0; j<screen_width; j++)
			*buf++ = color;
}


void clear_lines(int page, int y, int count, int color)
{
	int i,j;

	assert(drawing_enable==1);

	if (scale_up) {
		count *= 2;
		y *= 2;
	}

	for (i=0; i<count; i++) {
		if ((i+y)<screen_height) {
			unsigned char *buf = get_vgaptr(page, 0, i+y);
			for (j=0; j<screen_width; j++)
				*buf++ = color;
		}
	}
	count = ((y+count)>>dirty_block_shift) - (y>>dirty_block_shift) + 1;
	y >>= dirty_block_shift;
	for (i=0; i<count; i++)
		for (j=0; j<25; j++)
			dirty_blocks[page][(y+i)*25+j] = 1;
}


int get_color(int color, char pal[768])
{
	assert(color<256);
	assert(pal);
	return SDL_MapRGB(jnb_surface->format, (Uint8)(pal[color*3+0]<<2), (Uint8)(pal[color*3+1]<<2), (Uint8)(pal[color*3+2]<<2));
}


int get_pixel(int page, int x, int y)
{
	assert(drawing_enable==1);

	if (scale_up) {
		x *= 2;
		y *= 2;
	}

	assert(x<screen_width);
	assert(y<screen_height);

	return *(unsigned char *)get_vgaptr(page, x, y);
}


void set_pixel(int page, int x, int y, int color)
{
	assert(drawing_enable==1);

	if (scale_up) {
		x *= 2;
		y *= 2;
	}

	assert(x<screen_width);
	assert(y<screen_height);

	dirty_blocks[page][(y>>dirty_block_shift)*25+(x>>dirty_block_shift)] = 1;

	*(unsigned char *)get_vgaptr(page, x, y) = color;
}


void flippage(int page)
{
	int x,y;
	unsigned char *src;
	unsigned char *dest;

	assert(drawing_enable==0);

	SDL_LockSurface(jnb_surface);
	if (!jnb_surface->pixels) {
		
		for (x=0; x<(25*16); x++) {
			dirty_blocks[0][x] = 1;
			dirty_blocks[1][x] = 1;
		}

		return;
	}
	dest=(unsigned char *)jnb_surface->pixels;
	src=screen_buffer[page];
	for (y=0; y<screen_height; y++) {
		for (x=0; x<25; x++) {
			int count;
			int test_x;

			count=0;
			test_x=x;
			while ( (test_x<25) && (dirty_blocks[page][(y>>dirty_block_shift)*25+test_x]) ) {
				count++;
				test_x++;
			}
			if (count) {
				memcpy(	&dest[y*jnb_surface->pitch+(x<<dirty_block_shift)],
					&src[y*screen_pitch+((x<<dirty_block_shift))],
					((16<<dirty_block_shift)>>4)*count);
			}
			x = test_x;
		}
	}
	memset(&dirty_blocks[page], 0, sizeof(int)*25*16);
        SDL_UnlockSurface(jnb_surface);
	SDL_Flip(jnb_surface);
}


void draw_begin(void)
{
	assert(drawing_enable==0);

	drawing_enable = 1;
	if (background_drawn == 0) {
		if (background) {
			put_block(0, 0, 0, JNB_WIDTH, JNB_HEIGHT, background);
			put_block(1, 0, 0, JNB_WIDTH, JNB_HEIGHT, background);
		} else {
			clear_page(0, 0);
			clear_page(1, 0);
		}
		background_drawn = 1;
	}
}


void draw_end(void)
{
	assert(drawing_enable==1);

	drawing_enable = 0;
}


void setpalette(int index, int count, char *palette)
{
	SDL_Color colors[256];
	int i;

	assert(drawing_enable==0);

	for (i = 0; i < count; i++) {
		colors[i+index].r = palette[i * 3 + 0] << 2;
		colors[i+index].g = palette[i * 3 + 1] << 2;
		colors[i+index].b = palette[i * 3 + 2] << 2;
	}
	SDL_SetColors(jnb_surface, &colors[index], index, count);
}


void fillpalette(int red, int green, int blue)
{
	SDL_Color colors[256];
	int i;

	assert(drawing_enable==0);

	for (i = 0; i < 256; i++) {
		colors[i].r = red << 2;
		colors[i].g = green << 2;
		colors[i].b = blue << 2;
	}
	SDL_SetColors(jnb_surface, colors, 0, 256);
}


void get_block(int page, int x, int y, int width, int height, void *buffer)
{
	unsigned char *buffer_ptr, *vga_ptr;
	int h;

	assert(drawing_enable==1);

	if (scale_up) {
		x *= 2;
		y *= 2;
		width *= 2;
		height *= 2;
	}

	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;
	if (y + height >= screen_height)
		height = screen_height - y;
	if (x + width >= screen_width)
		width = screen_width - x;
	if (width<=0)
		return;
	if(height<=0)
		return;

	vga_ptr = get_vgaptr(page, x, y);
	buffer_ptr = buffer;
	for (h = 0; h < height; h++) {
		memcpy(buffer_ptr, vga_ptr, width);
		vga_ptr += screen_pitch;
		buffer_ptr += width;
	}

}


void put_block(int page, int x, int y, int width, int height, void *buffer)
{
	int h;
	unsigned char *vga_ptr, *buffer_ptr;

	assert(drawing_enable==1);

	if (scale_up) {
		x *= 2;
		y *= 2;
		width *= 2;
		height *= 2;
	}

	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;
	if (y + height >= screen_height)
		height = screen_height - y;
	if (x + width >= screen_width)
		width = screen_width - x;
	if (width<=0)
		return;
	if(height<=0)
		return;

	vga_ptr = get_vgaptr(page, x, y);
	buffer_ptr = buffer;
	for (h = 0; h < height; h++) {
		memcpy(vga_ptr, buffer_ptr, width);
		vga_ptr += screen_pitch;
		buffer_ptr += width;
	}
	width = ((x+width)>>dirty_block_shift) - (x>>dirty_block_shift) + 1;
	height = ((y+height)>>dirty_block_shift) - (y>>dirty_block_shift) + 1;
	x >>= dirty_block_shift;
	y >>= dirty_block_shift;
	while (width--)
		for (h=0; h<height; h++)
			dirty_blocks[page][(y+h)*25+(x+width)] = 1;
}


void put_text(int page, int x, int y, char *text, int align)
{
	int c1;
	int t1;
	int width;
	int cur_x;
	int image;

	assert(drawing_enable==1);

	if (text == NULL || strlen(text) == 0)
		return;
	if (font_gobs.num_images == 0)
		return;

	width = 0;
	c1 = 0;
	while (text[c1] != 0) {
		t1 = text[c1];
		c1++;
		if (t1 == ' ') {
			width += 5;
			continue;
		}
		if (t1 >= 33 && t1 <= 34)
			image = t1 - 33;

		else if (t1 >= 39 && t1 <= 41)
			image = t1 - 37;

		else if (t1 >= 44 && t1 <= 59)
			image = t1 - 39;

		else if (t1 >= 64 && t1 <= 90)
			image = t1 - 43;

		else if (t1 >= 97 && t1 <= 122)
			image = t1 - 49;

		else if (t1 == '~')
			image = 74;

		else if (t1 == 0x84)
			image = 75;

		else if (t1 == 0x86)
			image = 76;

		else if (t1 == 0x8e)
			image = 77;

		else if (t1 == 0x8f)
			image = 78;

		else if (t1 == 0x94)
			image = 79;

		else if (t1 == 0x99)
			image = 80;

		else
			continue;
		width += pob_width(image, &font_gobs) + 1;
	}

	switch (align) {
	case 0:
		cur_x = x;
		break;
	case 1:
		cur_x = x - width;
		break;
	case 2:
		cur_x = x - width / 2;
		break;
	default:
		cur_x = 0;	/* this should cause error? -Chuck */
		break;
	}
	c1 = 0;

	while (text[c1] != 0) {
		t1 = text[c1];
		c1++;
		if (t1 == ' ') {
			cur_x += 5;
			continue;
		}
		if (t1 >= 33 && t1 <= 34)
			image = t1 - 33;

		else if (t1 >= 39 && t1 <= 41)
			image = t1 - 37;

		else if (t1 >= 44 && t1 <= 59)
			image = t1 - 39;

		else if (t1 >= 64 && t1 <= 90)
			image = t1 - 43;

		else if (t1 >= 97 && t1 <= 122)
			image = t1 - 49;

		else if (t1 == '~')
			image = 74;

		else if (t1 == 0x84)
			image = 75;

		else if (t1 == 0x86)
			image = 76;

		else if (t1 == 0x8e)
			image = 77;

		else if (t1 == 0x8f)
			image = 78;

		else if (t1 == 0x94)
			image = 79;

		else if (t1 == 0x99)
			image = 80;

		else
			continue;
		put_pob(page, cur_x, y, image, &font_gobs, 1, mask_pic);
		cur_x += pob_width(image, &font_gobs) + 1;
	}
}


void put_pob(int page, int x, int y, int image, gob_t *gob, int use_mask, void *mask_pic)
{
	int c1, c2;
	int pob_x, pob_y;
	int width, height;
	int draw_width, draw_height;
	int colour;
	unsigned char *vga_ptr;
	unsigned char *pob_ptr;
	unsigned char *mask_ptr;

	assert(drawing_enable==1);
	assert(gob);
	assert(image>=0);
	assert(image<gob->num_images);

	if (scale_up) {
		x *= 2;
		y *= 2;
		width = draw_width = gob->width[image]*2;
		height = draw_height = gob->height[image]*2;
		x -= gob->hs_x[image]*2;
		y -= gob->hs_y[image]*2;
	} else {
		width = draw_width = gob->width[image];
		height = draw_height = gob->height[image];
		x -= gob->hs_x[image];
		y -= gob->hs_y[image];
	}

	if ((x + width) <= 0 || x >= screen_width)
		return;
	if ((y + height) <= 0 || y >= screen_height)
		return;

	pob_x = 0;
	pob_y = 0;
	if (x < 0) {
		pob_x -= x;
		draw_width += x;
		x = 0;
	}
	if ((x + width) > screen_width)
		draw_width -= x + width - screen_width;
	if (y < 0) {
		pob_y -= y;
		draw_height += y;
		y = 0;
	}
	if ((y + height) > screen_height)
		draw_height -= y + height - screen_height;

	vga_ptr = get_vgaptr(page, x, y);
	pob_ptr = ((unsigned char *)gob->data[image]) + ((pob_y * width) + pob_x);
	mask_ptr = ((unsigned char *)mask) + ((y * screen_pitch) + (x));
	for (c1 = 0; c1 < draw_height; c1++) {
		for (c2 = 0; c2 < draw_width; c2++) {
			colour = *mask_ptr;
			if (use_mask == 0 || (use_mask == 1 && colour == 0)) {
				colour = *pob_ptr;
				if (colour != 0) {
					*vga_ptr = colour;
				}
			}
			vga_ptr++;
			pob_ptr++;
			mask_ptr++;
		}
		pob_ptr += width - c2;
		vga_ptr += (screen_width - c2);
		mask_ptr += (screen_width - c2);
	}
	draw_width = ((x+draw_width)>>dirty_block_shift) - (x>>dirty_block_shift) + 1;
	draw_height = ((y+draw_height)>>dirty_block_shift) - (y>>dirty_block_shift) + 1;
	x >>= dirty_block_shift;
	y >>= dirty_block_shift;
	while (draw_width--)
		for (c1=0; c1<draw_height; c1++)
			dirty_blocks[page][(y+c1)*25+(x+draw_width)] = 1;
}


int pob_width(int image, gob_t *gob)
{
	assert(gob);
	assert(image>=0);
	assert(image<gob->num_images);
	return gob->width[image];
}


int pob_height(int image, gob_t *gob)
{
	assert(gob);
	assert(image>=0);
	assert(image<gob->num_images);
	return gob->height[image];
}


int pob_hs_x(int image, gob_t *gob)
{
	assert(gob);
	assert(image>=0);
	assert(image<gob->num_images);
	return gob->hs_x[image];
}


int pob_hs_y(int image, gob_t *gob)
{
	assert(gob);
	assert(image>=0);
	assert(image<gob->num_images);
	return gob->hs_y[image];
}


int read_pcx(unsigned char * handle, void *buf, int buf_len, char *pal)
{
	unsigned char *buffer=buf;
	short c1;
	short a, b;
	long ofs1;
	if (buffer != 0) {
		handle += 128;
		ofs1 = 0;
		while (ofs1 < buf_len) {
			a = *(handle++);
			if ((a & 0xc0) == 0xc0) {
				b = *(handle++);
				a &= 0x3f;
				for (c1 = 0; c1 < a && ofs1 < buf_len; c1++)
					buffer[ofs1++] = (char) b;
			} else
				buffer[ofs1++] = (char) a;
		}
		if (pal != 0) {
			handle++;
			for (c1 = 0; c1 < 768; c1++)
				pal[c1] = *(handle++) /*fgetc(handle)*/ >> 2;
		}
	}
	return 0;
}


void register_background(char *pixels, char pal[768])
{
	if (background) {
		free(background);
		background = NULL;
	}
	background_drawn = 0;
	if (!pixels)
		return;
	assert(pal);
	if (scale_up) {
		background = malloc(screen_pitch*screen_height);
		assert(background);
		do_scale2x((unsigned char *)pixels, JNB_WIDTH, JNB_HEIGHT, (unsigned char *)background);
	} else {
		background = malloc(JNB_WIDTH*JNB_HEIGHT);
		assert(background);
		memcpy(background, pixels, JNB_WIDTH*JNB_HEIGHT);
	}
}

int register_gob(unsigned char *handle, gob_t *gob, int len)
{
	unsigned char *gob_data;
	int i;

	gob_data = malloc(len);
	memcpy(gob_data, handle, len);

	gob->num_images = (short)((gob_data[0]) + (gob_data[1] << 8));

	gob->width = malloc(gob->num_images*sizeof(int));
	gob->height = malloc(gob->num_images*sizeof(int));
	gob->hs_x = malloc(gob->num_images*sizeof(int));
	gob->hs_y = malloc(gob->num_images*sizeof(int));
	gob->data = malloc(gob->num_images*sizeof(void *));
	gob->orig_data = malloc(gob->num_images*sizeof(void *));
	for (i=0; i<gob->num_images; i++) {
		int image_size;
		int offset;

		offset = (gob_data[i*4+2]) + (gob_data[i*4+3] << 8) + (gob_data[i*4+4] << 16) + (gob_data[i*4+5] << 24);

		gob->width[i]  = (short)((gob_data[offset]) + (gob_data[offset+1] << 8)); offset += 2;
		gob->height[i] = (short)((gob_data[offset]) + (gob_data[offset+1] << 8)); offset += 2;
		gob->hs_x[i]   = (short)((gob_data[offset]) + (gob_data[offset+1] << 8)); offset += 2;
		gob->hs_y[i]   = (short)((gob_data[offset]) + (gob_data[offset+1] << 8)); offset += 2;

		image_size = gob->width[i] * gob->height[i];
		gob->orig_data[i] = malloc(image_size);
		memcpy(gob->orig_data[i], &gob_data[offset], image_size);
		if (scale_up) {
			image_size = gob->width[i] * gob->height[i] * 4;
			gob->data[i] = malloc(image_size);
			do_scale2x((unsigned char *)gob->orig_data[i], gob->width[i], gob->height[i], (unsigned char *)gob->data[i]);
		} else {
			gob->data[i] = (unsigned short *)gob->orig_data[i];
		}
	}
	free(gob_data);
	return 0;
}


void recalculate_gob(gob_t *gob, char pal[768])
{
}

void register_mask(void *pixels)
{
	if (mask) {
		free(mask);
		mask = NULL;
	}
	assert(pixels);
	if (scale_up) {
		mask = malloc(screen_pitch*screen_height);
		assert(mask);
		do_scale2x((unsigned char *)pixels, JNB_WIDTH, JNB_HEIGHT, (unsigned char *)mask);
	} else {
		mask = malloc(JNB_WIDTH*JNB_HEIGHT);
		assert(mask);
		memcpy(mask, pixels, JNB_WIDTH*JNB_HEIGHT);
	}
}
