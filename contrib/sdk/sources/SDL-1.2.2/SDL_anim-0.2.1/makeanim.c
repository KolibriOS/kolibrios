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
#include <png.h>

/* deal with MSVC++ crappiness */
#ifdef WIN32UNDEFINED
	#define snprintf _snprintf
	#endif

typedef struct inputstruct {
	FILE *file;
	char *name;
	png_structp read_ptr;
	png_infop read_info_ptr;
	};

int numfiles;
struct inputstruct *input;

int main( int argc, char *argv[] ) {
	int f, rowbytes;
	char buf[256];
	static FILE *fpout;  /* "static" prevents setjmp corruption */
	png_structp write_ptr;
	png_infop write_info_ptr, end_info_ptr;
	png_bytep row_buf, here;
	png_uint_32 y;
	png_textp text_ptr, new_text_ptr;
	int num_text;

	int interlace_type, compression_type, filter_type, bit_depth, color_type;
	int it, ct, ft, bd, clrt;
	png_uint_32 width, height, w, h;

	int duration;

	if( argc < 4 ) {
		printf( "makeanim v0.2\nusage: makeanim <duration in milliseconds> <input files ...> <output file>\n" );
		printf( "example: makeanim 1500 a00.png a01.png a02.png a03.png a04.png a.anim\n" );
		return 1;
		}

	duration = atoi( argv[1] );
	if( duration < 1 ) {
		printf( "duration is incorrect\n" );
		return 1;
		}

	numfiles = argc - 3;
	input = (struct inputstruct *)malloc( sizeof( struct inputstruct ) * numfiles );
	if( !input ) return 1;

	for( f = 0; f < numfiles; f++ ) {
		input[f].name = argv[f + 2];
		printf( "opening file %d, \"%s\"\n", f, input[f].name );

		/* open the file handle */
		input[f].file = fopen( input[f].name, "rb" );
		if( input[f].file == NULL ) {
			printf( "fopen() failed\n" );
			return 1;
			}

		/* check if it's PNG */
		if( fread( buf, 1, 8, input[f].file ) != 8 ) {
			printf( "fread() failed for file \"%s\"\n", input[f].name );
			return 1;
			}
		if( png_sig_cmp( buf, (png_size_t)0, 8 ) ) {
			printf( "not a PNG file\n" );
			return 1;
			}
		fseek( input[f].file, 0, SEEK_SET );

		/* allocate read structure */
		input[f].read_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, (png_voidp)NULL, (png_error_ptr)NULL, (png_error_ptr)NULL );
		if( input[f].read_ptr == NULL ) {
			printf( "png_create_read_struct() failed\n" );
			return 1;
			}
		
		/* allocate read info structure */
		input[f].read_info_ptr = png_create_info_struct( input[f].read_ptr );
		if( input[f].read_info_ptr == NULL ) {
			printf( "png_create_info_struct() failed\n" );
			return 1;
			}


		/* set error handler code */
		if( setjmp( input[f].read_ptr->jmpbuf ) ) {
			printf( "libpng read error\n" );
			return 1;
			}

		/* initialize stream */
		png_init_io( input[f].read_ptr, input[f].file );
		png_set_read_status_fn( input[f].read_ptr, NULL );

		/* read png info struct */
		png_read_info( input[f].read_ptr, input[f].read_info_ptr );

		/* get the info */
		if( !png_get_IHDR( input[f].read_ptr, input[f].read_info_ptr, &w, &h, &bd, &clrt, &it, &ct, &ft ) ) {
			printf( "png_get_IHDR() failed\n" );
			return 1;
			}

		/* save the info of the first frame */
		if( f == 0 ) {
			width = w;
			height = h;
			bit_depth = bd;
			color_type = clrt;
			interlace_type = it;
			compression_type = ct;
			filter_type = ft;
			}
		/* compare all other frames to first frame */
		else if( (w != width) ||
				(h != height) ||
				(bd != bit_depth) ||
				(clrt != color_type) ||
				(it != interlace_type) ||
				(ct != compression_type) ||
				(ft != filter_type) ) {
			if( w != width ) printf( "width is different\n" );
			if( h != height ) printf( "height  is different\n" );
			if( bd != bit_depth ) printf( "bit depth is different\n" );
			if( clrt != color_type ) printf( "color type is different\n" );
			if( it != interlace_type ) printf( "interlace type is different\n" );
			if( ct != compression_type ) printf( "compression type is different\n" );
			if( ft != filter_type ) printf( "filter type is different\n" );
			return 1;
			}
		}
	
	row_buf = (png_bytep)NULL;
	
	/* open output file */
	printf( "opening file \"%s\"\n", argv[numfiles + 2] );
	fpout = fopen( argv[numfiles + 2], "wb" );
	if( fpout == NULL ) {
		printf( "fopen() failed\n" );
		return 1;
		}

	/* allocate write structure */
	write_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING, (png_voidp)NULL, (png_error_ptr)NULL, (png_error_ptr)NULL );

	/* allocate info structures */
	write_info_ptr = png_create_info_struct( write_ptr );
	end_info_ptr = png_create_info_struct( write_ptr );

	/* error handling */
	if( setjmp( write_ptr->jmpbuf ) ) {
		printf( "libpng write error\n" );
		return 1;
		}

	/* initialize output stream */
	png_init_io( write_ptr, fpout );
	png_set_write_status_fn( write_ptr, NULL );

	/* set info */
	png_set_IHDR( write_ptr, write_info_ptr, width * numfiles, height, bit_depth, color_type, PNG_INTERLACE_NONE, compression_type, filter_type);

	/* image characteristics */
	{
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
		png_colorp palette;
		int num_palette;
		png_color_8p sig_bit;
		png_bytep trans;
		int num_trans;
		png_color_16p trans_values;

		/* background color */
		if( png_get_bKGD( input[0].read_ptr, input[0].read_info_ptr, &background ) ) {
			png_set_bKGD( write_ptr, write_info_ptr, background );
			}

		if( png_get_cHRM( input[0].read_ptr, input[0].read_info_ptr, &white_x, &white_y, &red_x, &red_y, &green_x, &green_y, &blue_x, &blue_y ) ) {
			png_set_cHRM( write_ptr, write_info_ptr, white_x, white_y, red_x, red_y, green_x, green_y, blue_x, blue_y );
			}

		/* gamma */
		if( png_get_gAMA( input[0].read_ptr, input[0].read_info_ptr, &gamma ) ) {
			png_set_gAMA( write_ptr, write_info_ptr, gamma );
			}

		/* rendering intent */
		if( png_get_sRGB( input[0].read_ptr, input[0].read_info_ptr, &intent ) ) {
			png_set_sRGB( write_ptr, write_info_ptr, intent );
			}

		/* Histogram */
		if( png_get_hIST( input[0].read_ptr, input[0].read_info_ptr, &hist ) ) {
			png_set_hIST( write_ptr, write_info_ptr, hist );
			}

		/* offsets */
		if( png_get_oFFs( input[0].read_ptr, input[0].read_info_ptr, &offset_x, &offset_y, &unit_type ) ) {
			png_set_oFFs( write_ptr, write_info_ptr, offset_x, offset_y, unit_type );
			}

		if( png_get_pCAL( input[0].read_ptr, input[0].read_info_ptr, &purpose, &X0, &X1, &type, &nparams, &units, &params ) ) {
			png_set_pCAL( write_ptr, write_info_ptr, purpose, X0, X1, type, nparams, units, params );
			}

		/* pixel density */
		if( png_get_pHYs( input[0].read_ptr, input[0].read_info_ptr, &res_x, &res_y, &unit_type ) ) {
			png_set_pHYs( write_ptr, write_info_ptr, res_x, res_y, unit_type );
			}

		/* text chunks */
/*		if( png_get_text( input[0].read_ptr, input[0].read_info_ptr, &text_ptr, &num_text ) > 0 ) {
			printf( "Handling %d tEXt/zTXt chunks\n", num_text );
			png_set_text( write_ptr, write_info_ptr, text_ptr, num_text );
			}
*/
		/* palette */
		if( png_get_PLTE( input[0].read_ptr, input[0].read_info_ptr, &palette, &num_palette ) ) {
			png_set_PLTE( write_ptr, write_info_ptr, palette, num_palette );
			}

		/* significant bits */
		if( png_get_sBIT( input[0].read_ptr, input[0].read_info_ptr, &sig_bit ) ) {
			png_set_sBIT( write_ptr, write_info_ptr, sig_bit );
			}

		/* transparency */
		if( png_get_tRNS( input[0].read_ptr, input[0].read_info_ptr, &trans, &num_trans, &trans_values ) ) {
			png_set_tRNS( write_ptr, write_info_ptr, trans, num_trans, trans_values );
			}
		}

	/* text chunks */
	num_text = 0;
	if( !png_get_text( input[0].read_ptr, input[0].read_info_ptr, &text_ptr, &num_text ) ) num_text = 0;
	new_text_ptr = (struct png_text_struct *)malloc( sizeof( struct png_text_struct ) * num_text + 1 );
	if( !new_text_ptr ) {
		printf( "malloc() failed\n" );
		return 1;
		}
	
	memcpy( new_text_ptr, text_ptr, sizeof( struct png_text_struct ) * num_text );

	snprintf( buf, 255, "SDL_anim %d %d %d", duration, width, numfiles );
	buf[255] = 0;
	new_text_ptr[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
	new_text_ptr[num_text].key = "format";
	new_text_ptr[num_text].text = buf;
	new_text_ptr[num_text].text_length = strlen( buf );
	num_text++;
	png_set_text( write_ptr, write_info_ptr, new_text_ptr, num_text );

	/* write info */
	png_write_info( write_ptr, write_info_ptr );

	/* allocate buffer */
	rowbytes = png_get_rowbytes( input[0].read_ptr, input[0].read_info_ptr );
	row_buf = (png_bytep)png_malloc( write_ptr, rowbytes * numfiles );
	if( row_buf == NULL ) {
		printf( "png_malloc() failed\n" );
		return 1;
		}

	/* copy raw data */
	for( y = 0; y < height; y++ ) {
		/* grab a scanline from each file */
		here = row_buf;
		for( f = 0; f < numfiles; f++ ) {
			png_read_rows( input[f].read_ptr, (png_bytepp)&here, (png_bytepp)NULL, 1 );
			here += rowbytes;
			}
		/* write the long scanline */
		png_write_rows( write_ptr, (png_bytepp)&row_buf, 1 );
		}

	/* end io */
	for( f = 0; f < numfiles; f++ ) png_read_end( input[f].read_ptr, end_info_ptr );
	png_write_end( write_ptr, end_info_ptr );

	/* cleanup */
	png_free( write_ptr, row_buf );
	for( f = 0; f < numfiles; f++ ) {
		png_destroy_read_struct( &input[f].read_ptr, &input[f].read_info_ptr, &end_info_ptr);
		fclose( input[f].file );
		}
	png_destroy_write_struct( &write_ptr, &write_info_ptr );
	fclose( fpout );

	return 0;
	}
