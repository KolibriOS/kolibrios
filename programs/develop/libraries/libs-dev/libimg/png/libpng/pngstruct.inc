
; pngstruct.inc - header file for PNG reference library

; Last changed in libpng 1.6.24 [August 4, 2016]
; Copyright (c) 1998-2002,2004,2006-2016 Glenn Randers-Pehrson
; (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
; (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)

; This code is released under the libpng license.
; For conditions of distribution and use, see the disclaimer
; and license in png.inc


; The structure that holds the information to read and write PNG files.
; The only people who need to care about what is inside of this are the
; people who will be modifying the library for their own special needs.
; It should NOT be accessed directly by an application.


; zlib.inc defines the structure z_stream, an instance of which is included
; in this structure and is required for decompressing the LZ compressed
; data in PNG files.

include '../../../../../../fs/kfar/trunk/zlib/zlib.inc'

; zlib.inc declares a magic type 'uInt' that limits the amount of data that zlib
; can handle at once.  This type need be no larger than 16 bits (so maximum of
; 65535), this define allows us to discover how big it is, but limited by the
; maximuum for png_size_t.  The value can be overriden in a library build
; (pngusr.h, or set it in CPPFLAGS) and it works to set it to a considerably
; lower value (e.g. 255 works).  A lower value may help memory usage (slightly)
; and may even improve performance on some systems (and degrade it on others.)


ZLIB_IO_MAX equ 0xffff ;-1 ;uInt


; The type of a compression buffer list used by the write code.
struct png_compression_buffer
	next dd ? ;struct png_compression_buffer *
	output db ? ;byte[1] ;actually zbuf_size
ends

macro PNG_COMPRESSION_BUFFER_SIZE pp
{
	mov eax,png_compression_buffer.output
	add eax,[pp+png_struct.zbuffer_size]
}

; Colorspace support; structures used in png_struct, png_info and in internal
; functions to hold and communicate information about the color space.

; PNG_COLORSPACE_SUPPORTED is only required if the application will perform
; colorspace corrections, otherwise all the colorspace information can be
; skipped and the size of libpng can be reduced (significantly) by compiling
; out the colorspace support.

if PNG_COLORSPACE_SUPPORTED eq 1
; The chromaticities of the red, green and blue colorants and the chromaticity
; of the corresponding white point (i.e. of rgb(1.0,1.0,1.0)).

struct png_xy
	redx   dd ? ;png_fixed_point
	redy   dd ?
	greenx dd ?
	greeny dd ?
	bluex  dd ?
	bluey  dd ?
	whitex dd ?
	whitey dd ?
ends

; The same data as above but encoded as CIE XYZ values.  When this data comes
; from chromaticities the sum of the Y values is assumed to be 1.0

struct png_XYZ
	red_X dd ? ;png_fixed_point 
	red_Y dd ?
	red_Z dd ?
	green_X dd ?
	green_Y dd ?
	green_Z dd ?
	blue_X dd ?
	blue_Y dd ?
	blue_Z dd ?
ends
end if ;COLORSPACE

if (PNG_COLORSPACE_SUPPORTED eq 1) | (PNG_GAMMA_SUPPORTED eq 1)
; A colorspace is all the above plus, potentially, profile information;
; however at present libpng does not use the profile internally so it is only
; stored in the png_info struct (if iCCP is supported.)  The rendering intent
; is retained here and is checked.

; The file gamma encoding information is also stored here and gamma correction
; is done by libpng, whereas color correction must currently be done by the
; application.

struct png_colorspace
if PNG_GAMMA_SUPPORTED eq 1
	gamma dd ? ;png_fixed_point ;File gamma
end if

if PNG_COLORSPACE_SUPPORTED eq 1
	end_points_xy png_xy ;End points as chromaticities
	end_points_XYZ png_XYZ ;End points as CIE XYZ colorant values
	rendering_intent dw ? ;uint_16 ;Rendering intent of a profile
end if

	; Flags are always defined to simplify the code.
	flags dw ? ;uint_16 ;As defined below
ends

; General flags for the 'flags' field
PNG_COLORSPACE_HAVE_GAMMA     equ 0x0001
PNG_COLORSPACE_HAVE_ENDPOINTS equ 0x0002
PNG_COLORSPACE_HAVE_INTENT    equ 0x0004
PNG_COLORSPACE_FROM_gAMA      equ 0x0008
PNG_COLORSPACE_FROM_cHRM      equ 0x0010
PNG_COLORSPACE_FROM_sRGB      equ 0x0020
PNG_COLORSPACE_ENDPOINTS_MATCH_sRGB equ 0x0040
PNG_COLORSPACE_MATCHES_sRGB   equ 0x0080 ;exact match on profile
PNG_COLORSPACE_INVALID        equ 0x8000
macro PNG_COLORSPACE_CANCEL flags {(0xffff xor (flags))}
end if ;COLORSPACE || GAMMA

struct png_struct
if PNG_SETJMP_SUPPORTED eq 1
	jmp_buf_local rb 64 ;jmp_buf ;New name in 1.6.0 for jmp_buf in png_struct
	longjmp_fn   dd ? ;png_longjmp_ptr ;setjmp non-local goto function.
	jmp_buf_ptr  dd ? ;jmp_buf * ;passed to longjmp_fn
	jmp_buf_size dd ? ;size_t ;size of the above, if allocated
end if
	error_fn dd ? ;png_error_ptr ;function for printing errors and aborting
if PNG_WARNINGS_SUPPORTED eq 1
	warning_fn dd ? ;png_error_ptr ;function for printing warnings
end if
	error_ptr dd ? ;voidp ;user supplied struct for error functions
	write_data_fn dd ? ;png_rw_ptr ;function for writing output data
	read_data_fn dd ? ;png_rw_ptr ;function for reading input data
	io_ptr dd ? ;voidp ;ptr to application struct for I/O functions

if PNG_READ_USER_TRANSFORM_SUPPORTED eq 1
	read_user_transform_fn dd ? ;png_user_transform_ptr ;user read transform
end if

if PNG_WRITE_USER_TRANSFORM_SUPPORTED eq 1
	write_user_transform_fn dd ? ;png_user_transform_ptr ; user write transform
end if

; These were added in libpng-1.0.2
if PNG_USER_TRANSFORM_PTR_SUPPORTED eq 1
if (PNG_READ_USER_TRANSFORM_SUPPORTED eq 1) | (PNG_WRITE_USER_TRANSFORM_SUPPORTED eq 1)
	user_transform_ptr dd ? ;voidp ;user supplied struct for user transform
	user_transform_depth db ? ;byte ;bit depth of user transformed pixels
	user_transform_channels db ? ;byte ;channels in user transformed pixels
	rb 2 ;align
end if
end if

	mode  dd ? ;uint_32 ;tells us where we are in the PNG file
	flags dd ? ;uint_32 ;flags indicating various things to libpng
	transformations dd ? ;uint_32 ;which transformations to perform

	zowner dd ? ;uint_32 ;ID (chunk type) of zstream owner, 0 if none
	zstream z_stream ;decompression structure

	zbuffer_list dd ? ;png_compression_bufferp ;Created on demand during write
	zbuffer_size dd ? ;uInt ;size of the actual buffer

	zlib_level  dd ? ;int ;holds zlib compression level
	zlib_method dd ? ;int ;holds zlib compression method
	zlib_window_bits dd ? ;int ;holds zlib compression window bits
	zlib_mem_level dd ? ;int ;holds zlib compression memory level
	zlib_strategy  dd ? ;int ;holds zlib compression strategy
; Added at libpng 1.5.4
if PNG_WRITE_CUSTOMIZE_ZTXT_COMPRESSION_SUPPORTED eq 1
	zlib_text_level  dd ? ;int ;holds zlib compression level
	zlib_text_method dd ? ;int ;holds zlib compression method
	zlib_text_window_bits dd ? ;int ;holds zlib compression window bits
	zlib_text_mem_level dd ? ;int ;holds zlib compression memory level
	zlib_text_strategy  dd ? ;int ;holds zlib compression strategy
end if
;End of material added at libpng 1.5.4
;Added at libpng 1.6.0
	zlib_set_level  dd ? ;int ;Actual values set into the zstream on write
	zlib_set_method dd ? ;int
	zlib_set_window_bits dd ? ;int
	zlib_set_mem_level dd ? ;int
	zlib_set_strategy  dd ? ;int

	width    dd ? ;uint_32 ;width of image in pixels
	height   dd ? ;uint_32 ;height of image in pixels
	num_rows dd ? ;uint_32 ;number of rows in current pass
	usr_width dd ? ;uint_32 ;width of row at start of write
	rowbytes  dd ? ;png_size_t ;size of row in bytes
	iwidth     dd ? ;uint_32 ;width of current interlaced row in pixels
	row_number dd ? ;uint_32 ;current row in interlace pass
	chunk_name dd ? ;uint_32 ;PNG_CHUNK() id of current chunk
	prev_row dd ? ;bytep ;buffer to save previous (unfiltered) row.
			;While reading this is a pointer into
			;big_prev_row; while writing it is separately
			;allocated if needed.

	row_buf dd ? ;bytep ;buffer to save current (unfiltered) row.
			;While reading, this is a pointer into
			;big_row_buf; while writing it is separately
			;allocated.

if PNG_WRITE_FILTER_SUPPORTED eq 1
	try_row dd ? ;bytep ;buffer to save trial row when filtering
	tst_row dd ? ;bytep ;buffer to save best trial row when filtering
end if
	info_rowbytes dd ? ;png_size_t ;Added in 1.5.4: cache of updated row bytes

	idat_size dd ? ;uint_32 ;current IDAT size for read
	crc dd ? ;uint_32 ;current chunk CRC value
	palette dd ? ;png_colorp ;palette from the input file
	num_palette dw ? ;uint_16 ;number of color entries in palette
	rb 2 ;align
; Added at libpng-1.5.10
if PNG_CHECK_FOR_INVALID_INDEX_SUPPORTED eq 1
	num_palette_max dd ? ;int ;maximum palette index found in IDAT
end if

	num_trans   dw ? ;uint_16 ;number of transparency values
	compression db ? ;byte ;file compression type (always 0)
	filter      db ? ;byte ;file filter type (always 0)
	interlaced  db ? ;byte ;PNG_INTERLACE_NONE, PNG_INTERLACE_ADAM7
	pass        db ? ;byte ;current interlace pass (0 - 6)
	do_filter   db ? ;byte ;row filter flags (see PNG_FILTER_ in png.inc)
	color_type   db ? ;byte ;color type of file
	bit_depth    db ? ;byte ;bit depth of file
	usr_bit_depth db ? ;byte ;bit depth of users row: write only
	pixel_depth  db ? ;byte ;number of bits per pixel
	channels     db ? ;byte ;number of channels in file
	usr_channels db ? ;byte ;channels at start of write: write only
	sig_bytes    db ? ;byte ;magic bytes read/written from start of file
	maximum_pixel_depth db ? ;byte ;pixel depth used for the row buffers
	transformed_pixel_depth db ? ;byte ;pixel depth after read/write transforms
;if PNG_ZLIB_VERNUM >= 0x1240
	;zstream_start db 1 ;byte ;at start of an input zlib stream
;end if ;Zlib >= 1.2.4
if (PNG_READ_FILLER_SUPPORTED eq 1) | (PNG_WRITE_FILLER_SUPPORTED eq 1)
	filler dw ? ;uint_16 ;filler bytes for pixel expansion
end if

if (PNG_bKGD_SUPPORTED eq 1) | (PNG_READ_BACKGROUND_SUPPORTED eq 1) | \
	(PNG_READ_ALPHA_MODE_SUPPORTED eq 1)
	background_gamma_type db ? ;byte
	rb 1 ;align
	background_gamma dd ? ;png_fixed_point
	background png_color_16 ;background color in screen gamma space
	rb 1 ;align
if PNG_READ_GAMMA_SUPPORTED eq 1
	background_1 png_color_16 ;background normalized to gamma 1.0
	rb 1 ;align
end if
end if ;bKGD

if PNG_WRITE_FLUSH_SUPPORTED eq 1
	output_flush_fn dd ? ;png_flush_ptr ;Function for flushing output
	flush_dist dd ? ;uint_32 ;how many rows apart to flush, 0 - no flush
	flush_rows dd ? ;uint_32 ;number of rows written since last flush
end if

if PNG_READ_GAMMA_SUPPORTED eq 1
	gamma_shift  dd ? ;int ;number of "insignificant" bits in 16-bit gamma
	screen_gamma dd ? ;png_fixed_point ;screen gamma value (display_exponent)

	gamma_table  dd ? ;bytep ;gamma table for 8-bit depth files
	gamma_16_table dd ? ;uint_16pp ;gamma table for 16-bit depth files
if (PNG_READ_BACKGROUND_SUPPORTED eq 1) | \
	(PNG_READ_ALPHA_MODE_SUPPORTED eq 1) | \
	(PNG_READ_RGB_TO_GRAY_SUPPORTED eq 1)
	gamma_from_1  dd ? ;bytep ;converts from 1.0 to screen
	gamma_to_1    dd ? ;bytep ;converts from file to 1.0
	gamma_16_from_1 dd ? ;uint_16pp ;converts from 1.0 to screen
	gamma_16_to_1   dd ? ;uint_16pp ;converts from file to 1.0
end if ;READ_BACKGROUND || READ_ALPHA_MODE || RGB_TO_GRAY
end if

if (PNG_READ_GAMMA_SUPPORTED eq 1) | (PNG_sBIT_SUPPORTED eq 1)
	sig_bit png_color_8 ;significant bits in each available channel
end if

if (PNG_READ_SHIFT_SUPPORTED eq 1) | (PNG_WRITE_SHIFT_SUPPORTED eq 1)
	shift   png_color_8 ;shift for significant bit tranformation
	rb 2 ;align
end if

if (PNG_tRNS_SUPPORTED eq 1) | (PNG_READ_BACKGROUND_SUPPORTED eq 1) \
	| (PNG_READ_EXPAND_SUPPORTED eq 1) | (PNG_READ_BACKGROUND_SUPPORTED eq 1)
	trans_alpha dd ? ;bytep ;alpha values for paletted files
	trans_color png_color_16 ;transparent color for non-paletted files
	rb 3 ;align
end if

	read_row_fn  dd ? ;png_read_status_ptr ;called after each row is decoded
	write_row_fn dd ? ;png_write_status_ptr ;called after each row is encoded
if PNG_PROGRESSIVE_READ_SUPPORTED eq 1
	info_fn      dd ? ;png_progressive_info_ptr ;called after header data fully read
	row_fn        dd ? ;png_progressive_row_ptr ;called after a prog. row is decoded
	end_fn         dd ? ;png_progressive_end_ptr ;called after image is complete
	save_buffer_ptr dd ? ;bytep ;current location in save_buffer
	save_buffer      dd ? ;bytep ;buffer for previously read data
	current_buffer_ptr dd ? ;bytep ;current location in current_buffer
	current_buffer   dd ? ;bytep ;buffer for recently used data
	push_length      dd ? ;uint_32 ;size of current input chunk
	skip_length      dd ? ;uint_32 ;bytes to skip in input data
	save_buffer_size dd ? ;png_size_t ;amount of data now in save_buffer
	save_buffer_max  dd ? ;png_size_t ;total size of save_buffer
	buffer_size      dd ? ;png_size_t ;total amount of available input data
	current_buffer_size dd ? ;png_size_t ;amount of data now in current_buffer
	process_mode     dd ? ;int ;what push library is currently doing
	cur_palette      dd ? ;int ;current push library palette index

end if ;PROGRESSIVE_READ

if PNG_READ_QUANTIZE_SUPPORTED eq 1
	palette_lookup dd ? ;bytep ;lookup table for quantizing
	quantize_index dd ? ;bytep ;index translation for palette files
end if

; Options
if PNG_SET_OPTION_SUPPORTED eq 1
	options db ? ;byte ;On/off state (up to 4 options)
end if

;#if PNG_LIBPNG_VER < 10700
; To do: remove this from libpng-1.7
if PNG_TIME_RFC1123_SUPPORTED eq 1
	time_buffer rb 29 ;char[29] ;String to hold RFC 1123 time text
	rb 2 ;align
end if
;end if

; New members added in libpng-1.0.6

	free_me dd ? ;uint_32 ;flags items libpng is responsible for freeing

if PNG_USER_CHUNKS_SUPPORTED eq 1
	user_chunk_ptr dd ? ;voidp
if PNG_READ_USER_CHUNKS_SUPPORTED eq 1
	read_user_chunk_fn dd ? ;png_user_chunk_ptr ;user read chunk handler
end if
end if

if PNG_SET_UNKNOWN_CHUNKS_SUPPORTED eq 1
	unknown_default dd ? ;int          ; As PNG_HANDLE_*
	num_chunk_list  dd ? ;unsigned int ; Number of entries in the list
	chunk_list      dd ? ;bytep        ; List of byte[5]; the textual chunk name
			; followed by a PNG_HANDLE_* byte
end if

; New members added in libpng-1.0.3
if PNG_READ_RGB_TO_GRAY_SUPPORTED eq 1
	rgb_to_gray_status db ? ;byte
	; Added in libpng 1.5.5 to record setting of coefficients:
	rgb_to_gray_coefficients_set db ? ;byte
	; These were changed from byte in libpng-1.0.6
	rgb_to_gray_red_coeff   dw ? ;uint_16
	rgb_to_gray_green_coeff dw ? ;uint_16
	; deleted in 1.5.5: rgb_to_gray_blue_coeff;
	rb 2 ;align
end if

if PNG_MNG_FEATURES_SUPPORTED eq 1
; New member added in libpng-1.0.4 (renamed in 1.0.9)
; Changed from byte to uint_32 at version 1.2.0
	mng_features_permitted dd ? ;uint_32

; New member added in libpng-1.0.9, ifdef'ed out in 1.0.12, enabled in 1.2.0
	filter_type db ? ;byte
	rb 3 ;align
end if

; New members added in libpng-1.2.0

; New members added in libpng-1.0.2 but first enabled by default in 1.2.0
if PNG_USER_MEM_SUPPORTED eq 1
	mem_ptr   dd ? ;voidp      ;user supplied struct for mem functions
	malloc_fn dd ? ;malloc_ptr ;function for allocating memory
	free_fn   dd ? ;free_ptr   ;function for freeing memory
end if

; New member added in libpng-1.0.13 and 1.2.0
	big_row_buf dd ? ;bytep ;buffer to save current (unfiltered) row

if PNG_READ_QUANTIZE_SUPPORTED eq 1
; The following three members were added at version 1.0.14 and 1.2.4
	quantize_sort    dd ? ;bytep ;working sort array
	index_to_palette dd ? ;bytep ;where the original index currently is in the palette
	palette_to_index dd ? ;bytep ;which original index points to this palette color
end if

; New members added in libpng-1.0.16 and 1.2.6
	compression_type db ? ;byte
	rb 3 ;align

if PNG_USER_LIMITS_SUPPORTED eq 1
	user_width_max dd ? ;uint_32
	user_height_max dd ? ;uint_32

	; Added in libpng-1.4.0: Total number of sPLT, text, and unknown
	; chunks that can be stored (0 means unlimited).

	user_chunk_cache_max dd ? ;uint_32

	; Total memory that a zTXt, sPLT, iTXt, iCCP, or unknown chunk
	; can occupy when decompressed.  0 means unlimited.

	user_chunk_malloc_max dd ? ;png_alloc_size_t
end if

; New member added in libpng-1.0.25 and 1.2.17
if PNG_READ_UNKNOWN_CHUNKS_SUPPORTED eq 1
	; Temporary storage for unknown chunk that the library doesn't recognize,
	; used while reading the chunk.

	unknown_chunk png_unknown_chunk
	rb 3 ;align
end if

; New member added in libpng-1.2.26
	old_big_row_buf_size dd ? ;png_size_t

if PNG_READ_SUPPORTED eq 1
; New member added in libpng-1.2.30
	read_buffer dd ? ;bytep ;buffer for reading chunk data
	read_buffer_size dd ? ;png_alloc_size_t ;current size of the buffer
end if
if PNG_SEQUENTIAL_READ_SUPPORTED eq 1
	IDAT_read_size dd ? ;uInt ;limit on read buffer size for IDAT
end if

if PNG_IO_STATE_SUPPORTED eq 1
; New member added in libpng-1.4.0
	io_state dd ? ;uint_32
end if

	; New member added in libpng-1.5.6
	big_prev_row dd ? ;bytep

	; New member added in libpng-1.5.7
	read_filter rd PNG_FILTER_VALUE_LAST-1

if PNG_READ_SUPPORTED eq 1
if (PNG_COLORSPACE_SUPPORTED eq 1) | (PNG_GAMMA_SUPPORTED eq 1)
	colorspace png_colorspace
end if
end if
ends
