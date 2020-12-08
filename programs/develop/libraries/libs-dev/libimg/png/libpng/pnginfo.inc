
; pnginfo.inc - header file for PNG reference library

; Last changed in libpng 1.6.1 [March 28, 2013]
; Copyright (c) 1998-2002,2004,2006-2013 Glenn Randers-Pehrson
; (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
; (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)

; This code is released under the libpng license.
; For conditions of distribution and use, see the disclaimer
; and license in png.inc

; png_info is a structure that holds the information in a PNG file so
; that the application can find out the characteristics of the image.
; If you are reading the file, this structure will tell you what is
; in the PNG file.  If you are writing the file, fill in the information
; you want to put into the PNG file, using png_set_*() functions, then
; call png_write_info().

; The names chosen should be very close to the PNG specification, so
; consult that document for information about the meaning of each field.

; With libpng < 0.95, it was only possible to directly set and read the
; the values in the png_info_struct, which meant that the contents and
; order of the values had to remain fixed.  With libpng 0.95 and later,
; however, there are now functions that abstract the contents of
; png_info_struct from the application, so this makes it easier to use
; libpng with dynamic libraries, and even makes it possible to use
; libraries that don't have all of the libpng ancillary chunk-handing
; functionality.  In libpng-1.5.0 this was moved into a separate private
; file that is not visible to applications.

; The following members may have allocated storage attached that should be
; cleaned up before the structure is discarded: palette, trans, text,
; pcal_purpose, pcal_units, pcal_params, hist, iccp_name, iccp_profile,
; splt_palettes, scal_unit, row_pointers, and unknowns.   By default, these
; are automatically freed when the info structure is deallocated, if they were
; allocated internally by libpng.  This behavior can be changed by means
; of the png_data_freer() function.

; More allocation details: all the chunk-reading functions that
; change these members go through the corresponding png_set_*
; functions.  A function to clear these members is available: see
; png_free_data().  The png_set_* functions do not depend on being
; able to point info structure members to any of the storage they are
; passed (they make their own copies), EXCEPT that the png_set_text
; functions use the same storage passed to them in the text_ptr or
; itxt_ptr structure argument, and the png_set_rows and png_set_unknowns
; functions do not make their own copies.


struct png_info_def
	; The following are necessary for every PNG file
	width  dd ? ;uint_32 ;width of image in pixels (from IHDR)
	height dd ? ;uint_32 ;height of image in pixels (from IHDR)
	valid  dd ? ;uint_32 ;valid chunk data (see PNG_INFO_ below)
	rowbytes dd ? ;png_size_t ;bytes needed to hold an untransformed row
	palette  dd ? ;png_colorp ; array of color values (valid & PNG_INFO_PLTE)
	num_palette dw ? ;uint_16 ;number of color entries in "palette" (PLTE)
	num_trans   dw ? ;uint_16 ;number of transparent palette color (tRNS)
	bit_depth   db ? ;byte ;1, 2, 4, 8, or 16 bits/channel (from IHDR)
	color_type  db ? ;byte ;see PNG_COLOR_TYPE_ below (from IHDR)
	; The following three should have been named *_method not *_type
	compression_type db ? ;byte ;must be PNG_COMPRESSION_TYPE_BASE (IHDR)
	filter_type    db ? ;byte ;must be PNG_FILTER_TYPE_BASE (from IHDR)
	interlace_type db ? ;byte ;One of PNG_INTERLACE_NONE, PNG_INTERLACE_ADAM7

	; The following are set by png_set_IHDR, called from the application on
	; write, but the are never actually used by the write code.

	channels    db ? ;byte ;number of data channels per pixel (1, 2, 3, 4)
	pixel_depth db ? ;byte ;number of bits per pixel
	spare_byte  db ? ;byte ;to align the data, and for future use

if PNG_READ_SUPPORTED eq 1
	; This is never set during write
	signature db 8 ;byte[8] ;magic bytes read by libpng from start of file
end if

	; The rest of the data is optional.  If you are reading, check the
	; valid field to see if the information in these are valid.  If you
	; are writing, set the valid field to those chunks you want written,
	; and initialize the appropriate fields below.


if (PNG_COLORSPACE_SUPPORTED eq 1) | (PNG_GAMMA_SUPPORTED eq 1)
	; png_colorspace only contains 'flags' if neither GAMMA or COLORSPACE are
	; defined.  When COLORSPACE is switched on all the colorspace-defining
	; chunks should be enabled, when GAMMA is switched on all the gamma-defining
	; chunks should be enabled.  If this is not done it becomes possible to read
	; inconsistent PNG files and assign a probably incorrect interpretation to
	; the information.  (In other words, by carefully choosing which chunks to
	; recognize the system configuration can select an interpretation for PNG
	; files containing ambiguous data and this will result in inconsistent
	; behavior between different libpng builds!)

	colorspace png_colorspace
end if

if PNG_iCCP_SUPPORTED eq 1
	; iCCP chunk data.
	iccp_name dd ? ;charp ;profile name
	iccp_profile dd ? ;bytep ;International Color Consortium profile data
	iccp_proflen dd ? ;uint_32 ;ICC profile data length
end if

if PNG_TEXT_SUPPORTED eq 1
	; The tEXt, and zTXt chunks contain human-readable textual data in
	; uncompressed, compressed, and optionally compressed forms, respectively.
	; The data in "text" is an array of pointers to uncompressed,
	; null-terminated C strings. Each chunk has a keyword that describes the
	; textual data contained in that chunk.  Keywords are not required to be
	; unique, and the text string may be empty.  Any number of text chunks may
	; be in an image.

	num_text dd ? ;int ;number of comments read or comments to write
	max_text dd ? ;int ;current size of text array
	text dd ? ;png_textp ;array of comments read or comments to write
end if ;TEXT

if PNG_tIME_SUPPORTED eq 1
	; The tIME chunk holds the last time the displayed image data was
	; modified.  See the png_time struct for the contents of this struct.

	mod_time png_time
end if

if PNG_sBIT_SUPPORTED eq 1
	; The sBIT chunk specifies the number of significant high-order bits
	; in the pixel data.  Values are in the range [1, bit_depth], and are
	; only specified for the channels in the pixel data.  The contents of
	; the low-order bits is not specified.  Data is valid if
	; (valid & PNG_INFO_sBIT) is non-zero.

	sig_bit png_color_8 ; significant bits in color channels
end if

if (PNG_tRNS_SUPPORTED eq 1) | (PNG_READ_EXPAND_SUPPORTED eq 1) | (PNG_READ_BACKGROUND_SUPPORTED eq 1)
	; The tRNS chunk supplies transparency data for paletted images and
	; other image types that don't need a full alpha channel.  There are
	; "num_trans" transparency values for a paletted image, stored in the
	; same order as the palette colors, starting from index 0.  Values
	; for the data are in the range [0, 255], ranging from fully transparent
	; to fully opaque, respectively.  For non-paletted images, there is a
	; single color specified that should be treated as fully transparent.
	; Data is valid if (valid & PNG_INFO_tRNS) is non-zero.

	trans_alpha dd ? ;bytep ; alpha values for paletted image
	trans_color png_color_16 ;transparent color for non-palette image
end if

if (PNG_bKGD_SUPPORTED eq 1) | (PNG_READ_BACKGROUND_SUPPORTED eq 1)
	; The bKGD chunk gives the suggested image background color if the
	; display program does not have its own background color and the image
	; is needs to composited onto a background before display.  The colors
	; in "background" are normally in the same color space/depth as the
	; pixel data.  Data is valid if (valid & PNG_INFO_bKGD) is non-zero.

	background png_color_16
end if

if PNG_oFFs_SUPPORTED eq 1
	; The oFFs chunk gives the offset in "offset_unit_type" units rightwards
	; and downwards from the top-left corner of the display, page, or other
	; application-specific co-ordinate space.  See the PNG_OFFSET_ defines
	; below for the unit types.  Valid if (valid & PNG_INFO_oFFs) non-zero.

	x_offset dd ? ;int_32 ;x offset on page
	y_offset dd ? ;int_32 ;y offset on page
	offset_unit_type db ? ;byte ;offset units type
end if

if PNG_pHYs_SUPPORTED eq 1
	; The pHYs chunk gives the physical pixel density of the image for
	; display or printing in "phys_unit_type" units (see PNG_RESOLUTION_
	; defines below).  Data is valid if (valid & PNG_INFO_pHYs) is non-zero.

	x_pixels_per_unit dd ? ;uint_32 ;horizontal pixel density
	y_pixels_per_unit dd ? ;uint_32 ;vertical pixel density
	phys_unit_type db ? ;byte ;resolution type (see PNG_RESOLUTION_ below)
end if

if PNG_hIST_SUPPORTED eq 1
	; The hIST chunk contains the relative frequency or importance of the
	; various palette entries, so that a viewer can intelligently select a
	; reduced-color palette, if required.  Data is an array of "num_palette"
	; values in the range [0,65535]. Data valid if (valid & PNG_INFO_hIST)
	; is non-zero.

	hist dd ? ;uint_16p
end if

if PNG_pCAL_SUPPORTED eq 1
	; The pCAL chunk describes a transformation between the stored pixel
	; values and original physical data values used to create the image.
	; The integer range [0, 2^bit_depth - 1] maps to the floating-point
	; range given by [pcal_X0, pcal_X1], and are further transformed by a
	; (possibly non-linear) transformation function given by "pcal_type"
	; and "pcal_params" into "pcal_units".  Please see the PNG_EQUATION_
	; defines below, and the PNG-Group's PNG extensions document for a
	; complete description of the transformations and how they should be
	; implemented, and for a description of the ASCII parameter strings.
	; Data values are valid if (valid & PNG_INFO_pCAL) non-zero.

	pcal_purpose dd ? ;charp ;pCAL chunk description string
	pcal_X0 dd ? ;int_32 ;minimum value
	pcal_X1 dd ? ;int_32 ;maximum value
	pcal_units   dd ? ;charp ;Latin-1 string giving physical units
	pcal_params  dd ? ;charpp ;ASCII strings containing parameter values
	pcal_type    db ? ;byte ;equation type (see PNG_EQUATION_ below)
	pcal_nparams db ? ;byte ;number of parameters given in pcal_params 
end if

; New members added in libpng-1.0.6
	free_me dd ? ;uint_32 ;flags items libpng is responsible for freeing

if PNG_STORE_UNKNOWN_CHUNKS_SUPPORTED eq 1
	; Storage for unknown chunks that the library doesn't recognize.
	unknown_chunks dd ? ;png_unknown_chunkp

	; The type of this field is limited by the type of
	; png_struct::user_chunk_cache_max, else overflow can occur.

	unknown_chunks_num dd ? ;int
end if

if PNG_sPLT_SUPPORTED eq 1
	; Data on sPLT chunks (there may be more than one).
	splt_palettes dd ? ;png_sPLT_tp
	splt_palettes_num dd ? ;int ;Match type returned by png_get API
end if

if PNG_sCAL_SUPPORTED eq 1
	; The sCAL chunk describes the actual physical dimensions of the
	; subject matter of the graphic.  The chunk contains a unit specification
	; a byte value, and two ASCII strings representing floating-point
	; values.  The values are width and height corresponsing to one pixel
	; in the image.  Data values are valid if (valid & PNG_INFO_sCAL) is
	; non-zero.

	scal_unit db ? ;byte ;unit of physical scale
	scal_s_width dd ? ;charp ;string containing height
	scal_s_height dd ? ;charp ;string containing width
end if

if PNG_INFO_IMAGE_SUPPORTED eq 1
	; Memory has been allocated if (valid & PNG_ALLOCATED_INFO_ROWS)
	; non-zero
	; Data valid if (valid & PNG_INFO_IDAT) non-zero
	row_pointers dd ? ;bytepp ;the image bits
end if

ends
