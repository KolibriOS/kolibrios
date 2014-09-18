/*
	SDL_anim:  an animation library for SDL
	Copyright (C) 2001, 2002  Michael Leonhard

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the Free
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
	
	Michael Leonhard
	mike@tamale.net
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <png.h>
#include "SDL_anim.h"

/* deal with MSVC++ crappiness */
#ifdef WIN32UNDEFINED
	#define strcasecmp _strcmpi
	#endif

void Anim_Free( SDL_Animation *anim ) {
	SDL_FreeSurface( anim->surface );
	free( anim );
	}

int Anim_GetFrameNum( SDL_Animation *anim, Uint32 start, Uint32 now ) {
	int mspf, ms, frame;
	if( now < start ) return 0;

	mspf = anim->duration / anim->frames;
	ms = now - start;
	if( mspf == 0 ) frame = 0;
	else frame = ms / mspf;

	return frame;
	}

void Anim_GetFrameRect( SDL_Animation *anim, int frame, SDL_Rect *rect ) {
	rect->x = anim->w * (frame % anim->frames);
	rect->y = 0;
	rect->w = anim->w;
	rect->h = anim->h;
	}

int Anim_BlitFrame( SDL_Animation *anim, Uint32 start, Uint32 now, SDL_Surface *dest, SDL_Rect *dr ) {
	int frame;
	frame = Anim_GetFrameNum( anim, start, now );
	return Anim_BlitFrameNum( anim, frame, dest, dr );
	}

int Anim_BlitFrameNum( SDL_Animation *anim, int frame, SDL_Surface *dest, SDL_Rect *dr ) {
	SDL_Rect rect;
	Anim_GetFrameRect( anim, frame, &rect );
	return SDL_BlitSurface( anim->surface, &rect, dest, dr );
	}

int Anim_DisplayFormat( SDL_Animation *anim ) {
	struct SDL_Surface *newsurface;
	if( SDL_WasInit( SDL_INIT_VIDEO ) == 0 ) return 0;/*"Video is not initialized.\n"*/
	newsurface = SDL_DisplayFormatAlpha( anim->surface );
	if( !newsurface ) return 0;
	anim->surface = newsurface;
	return 1;	
	}

int DoAnimFormat( char *text, int *duration, int *framewidth, int *numframes ) {
	char *tok;
	SDL_printf( "file is \"%s\"\n", text );
	
	/* SDL_anim */
	tok = strtok( text, " " );
	if( !tok ) return 0;
	if( strcasecmp( tok, "SDL_anim" ) != 0 ) {
		SDL_printf( "no SDL_anim\n" );
		return 0;
		}
	
	/* duration */
	tok = strtok( NULL, " " );
	if( !tok ) return 0;
	*duration = atoi( tok );
	if( *duration < 1 ) {
		SDL_printf( "no duration\n" );
		return 0;
		}

	/* framewidth */
	tok = strtok( NULL, " " );
	if( !tok ) return 0;
	*framewidth = atoi( tok );
	if( *framewidth < 1 ) {
		SDL_printf( "no framewidth\n" );
		return 0;
		}

	/* numframes */
	tok = strtok( NULL, " " );
	if( !tok ) return 0;
	*numframes = atoi( tok );
	if( *numframes < 1 ) {
		SDL_printf( "no numframes\n" );
		return 0;
		}

	return 1;
	}

struct SDL_Animation *Anim_Load( const char *file ) {
	int ckey = -1, i;
	char buf[8];
	static FILE *fp;  /* "static" prevents setjmp corruption */
	png_structp read_ptr;
	png_infop read_info_ptr, end_info_ptr;
	png_bytep *row_pointers;
	png_textp text_ptr;
	int num_text, t;
	int interlace_type, compression_type, filter_type, bit_depth, color_type;
	png_uint_32 width, height, row;
	int duration, framewidth, numframes;

	png_color_16p background;
	double white_x, white_y, red_x, red_y, green_x, green_y, blue_x, blue_y;
	double gamma;
	int intent;
	png_uint_16p hist;
	png_uint_32 offset_x, offset_y;
	int unit_type;
	png_charp purpose, units;
	png_charpp params;
	png_int_32 X0, X1;
	int type, nparams;
	png_uint_32 res_x, res_y;
/*	png_colorp palette;
	int num_palette;
*/	png_color_8p sig_bit;
	png_bytep trans;
	int num_trans;
	png_color_16p trans_values;

	Uint32 Rmask;
	Uint32 Gmask;
	Uint32 Bmask;
	Uint32 Amask;

	SDL_Animation *anim;
	SDL_Surface *surface;
	SDL_Palette *palette;

	if( !file ) return NULL;

/*	printf( "opening file \"%s\"\n", file );
*/
	/* open the file handle */
	fp = fopen( file, "rb" );
	if( fp == NULL ) {
		SDL_printf( "fopen() failed\n" );
		return NULL;
		}

	/* check if it's PNG */
	if( fread( buf, 1, 8, fp ) != 8 ) {
		SDL_printf( "fread() failed\n" );
		return NULL;
		}
	if( png_sig_cmp( buf, (png_size_t)0, 8 ) ) {
		SDL_printf( "not a PNG file\n" );
		return NULL;
		}
	fseek( fp, 0, SEEK_SET );

	/* allocate read structure */
	read_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, (png_voidp)NULL, (png_error_ptr)NULL, (png_error_ptr)NULL );
	if( read_ptr == NULL ) {
		SDL_printf( "png_create_read_struct() failed\n" );
		return NULL;
		}
	
	/* allocate read info structure */
	read_info_ptr = png_create_info_struct( read_ptr );
	if( read_info_ptr == NULL ) {
		SDL_printf( "png_create_info_struct() failed\n" );
		return NULL;
		}
	end_info_ptr = png_create_info_struct( read_ptr );
	if( end_info_ptr == NULL ) {
		SDL_printf( "png_create_info_struct() failed\n" );
		return NULL;
		}

	/* set error handler code */
	if( setjmp( read_ptr->jmpbuf ) ) {
		SDL_printf( "libpng read error\n" );
		return NULL;
		}

	/* initialize stream */
	png_init_io( read_ptr, fp );
	png_set_read_status_fn( read_ptr, NULL );

	/* read png info struct */
	png_read_info( read_ptr, read_info_ptr );

	/* get the info */
	if( !png_get_IHDR( read_ptr, read_info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_type ) ) {
		SDL_printf( "png_get_IHDR() failed\n" );
		return NULL;
		}

	/* background color */
	png_get_bKGD( read_ptr, read_info_ptr, &background );

	png_get_cHRM( read_ptr, read_info_ptr, &white_x, &white_y, &red_x, &red_y, &green_x, &green_y, &blue_x, &blue_y );

	/* gamma */
	png_get_gAMA( read_ptr, read_info_ptr, &gamma );

	/* rendering intent */
	png_get_sRGB( read_ptr, read_info_ptr, &intent );

	/* Histogram */
	png_get_hIST( read_ptr, read_info_ptr, &hist );

	/* offsets */
	png_get_oFFs( read_ptr, read_info_ptr, &offset_x, &offset_y, &unit_type );
	
	png_get_pCAL( read_ptr, read_info_ptr, &purpose, &X0, &X1, &type, &nparams, &units, &params );

	/* pixel density */
	png_get_pHYs( read_ptr, read_info_ptr, &res_x, &res_y, &unit_type );

/* 	png_get_PLTE( read_ptr, read_info_ptr, &palette, &num_palette );
*/
	/* significant bits */
	png_get_sBIT( read_ptr, read_info_ptr, &sig_bit );

	/* transparency */
	if( png_get_tRNS( read_ptr, read_info_ptr, &trans, &num_trans, &trans_values ) ) {
		if( color_type == PNG_COLOR_TYPE_PALETTE ) {
			if( num_trans == 1 ) ckey = trans[0];
			else png_set_expand( read_ptr );
			}
		else ckey = 0; /* actual value will be set later */
		}

	/* text chunks */
	num_text = 0;
	if( !png_get_text( read_ptr, read_info_ptr, &text_ptr, &num_text ) ) {
		SDL_printf( "file has no text chunks\n" );
		return NULL;
		}
	for( t = 0; t < num_text; t++ ) {
		if( strcasecmp( text_ptr[t].key, "format" ) == 0 ) {
			if( DoAnimFormat( text_ptr[t].text, &duration, &framewidth, &numframes ) ) break;
			}
		}
	if( t == num_text ) {
		SDL_printf( "file is not an SDL_anim PNG\n" );
		return NULL;
		}

	png_set_strip_16( read_ptr );
	png_set_packing( read_ptr );
	if(color_type == PNG_COLOR_TYPE_GRAY)
		png_set_expand( read_ptr );

	/* Allocate the SDL surface to hold the image */
	Rmask = Gmask = Bmask = Amask = 0;
	if( color_type != PNG_COLOR_TYPE_PALETTE ) {
		if( SDL_BYTEORDER == SDL_LIL_ENDIAN ) {
			Rmask = 0x000000FF;
			Gmask = 0x0000FF00;
			Bmask = 0x00FF0000;
			Amask = (read_info_ptr->channels == 4)? 0xFF000000 : 0;
			}
		else {
			int s = (read_info_ptr->channels == 4) ? 0 : 8;
			Rmask = 0xFF000000 >> s;
			Gmask = 0x00FF0000 >> s;
			Bmask = 0x0000FF00 >> s;
			Amask = 0x000000FF >> s;
			}
		}

	surface = SDL_AllocSurface( SDL_SWSURFACE, width, height, bit_depth * read_info_ptr->channels, Rmask, Gmask, Bmask, Amask );
	if( surface == NULL ) {
		Anim_SetError("Out of memory");
		return NULL;
		}

	if(ckey != -1) {
		if( color_type != PNG_COLOR_TYPE_PALETTE ) ckey = SDL_MapRGB( surface->format, (Uint8)trans_values->red, (Uint8)trans_values->green, (Uint8)trans_values->blue );
		SDL_SetColorKey( surface, SDL_SRCCOLORKEY, ckey );
		}

	/* allocate row pointers */
	row_pointers = (png_bytep *)malloc( sizeof( png_bytep ) * height );
	if( row_pointers == NULL ) {
		SDL_printf( "malloc() failed\n" );
		return NULL;
		}
	for( row = 0; row < height; row++ ) {
		row_pointers[row] = (Uint8 *)surface->pixels + row * surface->pitch;
		}

	png_read_image( read_ptr, row_pointers );

	/* end io */
/*	printf( "done\n" );
*/	png_read_end( read_ptr, end_info_ptr );

	/* cleanup */
	png_destroy_read_struct( &read_ptr, &read_info_ptr, &end_info_ptr);
	fclose( fp );

	/* Load the palette, if any */
	palette = surface->format->palette;
	if( palette ) {
		if(color_type == PNG_COLOR_TYPE_GRAY) {
			palette->ncolors = 256;
			for( i = 0; i < 256; i++ ) {
				palette->colors[i].r = i;
				palette->colors[i].g = i;
				palette->colors[i].b = i;
				}
			}
		else if( read_info_ptr->num_palette > 0 ) {
			palette->ncolors = read_info_ptr->num_palette; 
			for( i = 0; i < read_info_ptr->num_palette; ++i ) {
				palette->colors[i].b = read_info_ptr->palette[i].blue;
				palette->colors[i].g = read_info_ptr->palette[i].green;
				palette->colors[i].r = read_info_ptr->palette[i].red;
				}
			}
		}
	
	anim = (struct SDL_Animation *)malloc( sizeof( struct SDL_Animation ) );
	if( !anim ) {
		SDL_printf( "malloc() failed\n" );
		return NULL;
		}

	anim->surface = surface;
	anim->w = framewidth;
	anim->h = height;
	anim->frames = numframes;
	anim->duration = duration;

	return anim;
	}
