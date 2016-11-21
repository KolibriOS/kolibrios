
; png.inc - header file for PNG reference library

; libpng version 1.6.25, September 1, 2016

; Copyright (c) 1998-2002,2004,2006-2016 Glenn Randers-Pehrson
; (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
; (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)

; This code is released under the libpng license (See LICENSE, below)

; Authors and maintainers:
;   libpng versions 0.71, May 1995, through 0.88, January 1996: Guy Schalnat
;   libpng versions 0.89, June 1996, through 0.96, May 1997: Andreas Dilger
;   libpng versions 0.97, January 1998, through 1.6.25, September 1, 2016:
;     Glenn Randers-Pehrson.
;   See also "Contributing Authors", below.



; COPYRIGHT NOTICE, DISCLAIMER, and LICENSE:

; If you modify libpng you may insert additional notices immediately following
; this sentence.

; This code is released under the libpng license.

; Some files in the "contrib" directory and some configure-generated
; files that are distributed with libpng have other copyright owners and
; are released under other open source licenses.

; libpng versions 1.0.7, July 1, 2000 through 1.6.25, September 1, 2016 are
; Copyright (c) 2000-2002, 2004, 2006-2016 Glenn Randers-Pehrson, are
; derived from libpng-1.0.6, and are distributed according to the same
; disclaimer and license as libpng-1.0.6 with the following individuals
; added to the list of Contributing Authors:

;    Simon-Pierre Cadieux
;    Eric S. Raymond
;    Mans Rullgard
;    Cosmin Truta
;    Gilles Vollant
;    James Yu
;    Mandar Sahastrabuddhe

; and with the following additions to the disclaimer:

;    There is no warranty against interference with your enjoyment of the
;    library or against infringement.  There is no warranty that our
;    efforts or the library will fulfill any of your particular purposes
;    or needs.  This library is provided with all faults, and the entire
;    risk of satisfactory quality, performance, accuracy, and effort is with
;    the user.

; Some files in the "contrib" directory have other copyright owners and
; are released under other open source licenses.


; libpng versions 0.97, January 1998, through 1.0.6, March 20, 2000, are
; Copyright (c) 1998-2000 Glenn Randers-Pehrson, are derived from
; libpng-0.96, and are distributed according to the same disclaimer and
; license as libpng-0.96, with the following individuals added to the list
; of Contributing Authors:

;    Tom Lane
;    Glenn Randers-Pehrson
;    Willem van Schaik

; Some files in the "scripts" directory have different copyright owners
; but are also released under this license.

; libpng versions 0.89, June 1996, through 0.96, May 1997, are
; Copyright (c) 1996-1997 Andreas Dilger, are derived from libpng-0.88,
; and are distributed according to the same disclaimer and license as
; libpng-0.88, with the following individuals added to the list of
; Contributing Authors:

;    John Bowler
;    Kevin Bracey
;    Sam Bushell
;    Magnus Holmgren
;    Greg Roelofs
;    Tom Tanner

; Some files in the "scripts" directory have other copyright owners
; but are released under this license.

; libpng versions 0.5, May 1995, through 0.88, January 1996, are
; Copyright (c) 1995-1996 Guy Eric Schalnat, Group 42, Inc.

; For the purposes of this copyright and license, "Contributing Authors"
; is defined as the following set of individuals:

;    Andreas Dilger
;    Dave Martindale
;    Guy Eric Schalnat
;    Paul Schmidt
;    Tim Wegner

; The PNG Reference Library is supplied "AS IS".  The Contributing Authors
; and Group 42, Inc. disclaim all warranties, expressed or implied,
; including, without limitation, the warranties of merchantability and of
; fitness for any purpose.  The Contributing Authors and Group 42, Inc.
; assume no liability for direct, indirect, incidental, special, exemplary,
; or consequential damages, which may result from the use of the PNG
; Reference Library, even if advised of the possibility of such damage.

; Permission is hereby granted to use, copy, modify, and distribute this
; source code, or portions hereof, for any purpose, without fee, subject
; to the following restrictions:

;   1. The origin of this source code must not be misrepresented.

;   2. Altered versions must be plainly marked as such and must not
;      be misrepresented as being the original source.

;   3. This Copyright notice may not be removed or altered from any
;      source or altered source distribution.

; The Contributing Authors and Group 42, Inc. specifically permit, without
; fee, and encourage the use of this source code as a component to
; supporting the PNG file format in commercial products.  If you use this
; source code in a product, acknowledgment is not required but would be
; appreciated.

; END OF COPYRIGHT NOTICE, DISCLAIMER, and LICENSE.

; TRADEMARK:

; The name "libpng" has not been registered by the Copyright owner
; as a trademark in any jurisdiction.  However, because libpng has
; been distributed and maintained world-wide, continually since 1995,
; the Copyright owner claims "common-law trademark protection" in any
; jurisdiction where common-law trademark is recognized.

; OSI CERTIFICATION:

; Libpng is OSI Certified Open Source Software.  OSI Certified Open Source is
; a certification mark of the Open Source Initiative. OSI has not addressed
; the additional disclaimers inserted at version 1.0.7.

; EXPORT CONTROL:

; The Copyright owner believes that the Export Control Classification
; Number (ECCN) for libpng is EAR99, which means not subject to export
; controls or International Traffic in Arms Regulations (ITAR) because
; it is open source, publicly available software, that does not contain
; any encryption software.  See the EAR, paragraphs 734.3(b)(3) and
; 734.7(b).



; A "png_get_copyright" function is available, for convenient use in "about"
; boxes and the like:

;    printf("%s", png_get_copyright(NULL));

; Also, the PNG logo (in PNG format, of course) is supplied in the
; files "pngbar.png" and "pngbar.jpg (88x31) and "pngnow.png" (98x31).



; The contributing authors would like to thank all those who helped
; with testing, bug fixes, and patience.  This wouldn't have been
; possible without all of you.

; Thanks to Frank J. T. Wojcik for helping with the documentation.

;
; Note about libpng version numbers:
;
;    Due to various miscommunications, unforeseen code incompatibilities
;    and occasional factors outside the authors' control, version numbering
;    on the library has not always been consistent and straightforward.
;    The following table summarizes matters since version 0.89c, which was
;    the first widely used release:
;
;    source                 png.h  png.h  shared-lib
;    version                string   int  version
;    -------                ------ -----  ----------
;    ...
;    1.2.56                  13    10256  12.so.0.56[.0]
;    ...
;    1.5.27                  15    10527  15.so.15.27[.0]
;    ...
;    1.6.25                  16    10625  16.so.16.25[.0]

;    Henceforth the source version will match the shared-library major
;    and minor numbers; the shared-library major version number will be
;    used for changes in backward compatibility, as it is intended.  The
;    PNG_LIBPNG_VER macro, which is not used within libpng but is available
;    for applications, is an unsigned integer of the form xyyzz corresponding
;    to the source version x.y.z (leading zeros in y and z).  Beta versions
;    were given the previous public release number plus a letter, until
;    version 1.0.6j; from then on they were given the upcoming public
;    release number plus "betaNN" or "rcNN".

;    Binary incompatibility exists only when applications make direct access
;    to the info_ptr or png_ptr members through png.h, and the compiled
;    application is loaded with a different version of the library.

;    DLLNUM will change each time there are forward or backward changes
;    in binary compatibility (e.g., when a new feature is added).

; See libpng.txt or libpng.3 for more information.  The PNG specification
; is available as a W3C Recommendation and as an ISO Specification,
; <http://www.w3.org/TR/2003/REC-PNG-20031110/



; Y2K compliance in libpng:
; =========================

;    September 1, 2016

;    Since the PNG Development group is an ad-hoc body, we can't make
;    an official declaration.

;    This is your unofficial assurance that libpng from version 0.71 and
;    upward through 1.6.25 are Y2K compliant.  It is my belief that
;    earlier versions were also Y2K compliant.

;    Libpng only has two year fields.  One is a 2-byte unsigned integer
;    that will hold years up to 65535.  The other, which is deprecated,
;    holds the date in text format, and will hold years up to 9999.

;    The integer is
;        "uint_16 year" in png_time_struct.

;    The string is
;        "char time_buffer[29]" in png_struct.  This is no longer used
;    in libpng-1.6.x and will be removed from libpng-1.7.0.

;    There are seven time-related functions:
;        png.asm: png_convert_to_rfc_1123_buffer() in png.asm
;          (formerly png_convert_to_rfc_1123() prior to libpng-1.5.x and
;          png_convert_to_rfc_1152() in error prior to libpng-0.98)
;        png_convert_from_struct_tm() in pngwrite.asm, called in pngwrite.asm
;        png_convert_from_time_t() in pngwrite.asm
;        png_get_tIME() in pngget.asm
;        png_handle_tIME() in pngrutil.asm, called in pngread.asm
;        png_set_tIME() in pngset.asm
;        png_write_tIME() in pngwutil.asm, called in pngwrite.asm

;    All handle dates properly in a Y2K environment.  The
;    png_convert_from_time_t() function calls gmtime() to convert from system
;    clock time, which returns (year - 1900), which we properly convert to
;    the full 4-digit year.  There is a possibility that libpng applications
;    are not passing 4-digit years into the png_convert_to_rfc_1123_buffer()
;    function, or that they are incorrectly passing only a 2-digit year
;    instead of "year - 1900" into the png_convert_from_struct_tm() function,
;    but this is not under our control.  The libpng documentation has always
;    stated that it works with 4-digit years, and the APIs have been
;    documented as such.

;    The tIME chunk itself is also Y2K compliant.  It uses a 2-byte unsigned
;    integer to hold the year, and can hold years as large as 65535.

;    zlib, upon which libpng depends, is also Y2K compliant.  It contains
;    no date-related code.

;       Glenn Randers-Pehrson
;       libpng maintainer
;       PNG Development Group


; This is not the place to learn how to use libpng. The file libpng-manual.txt
; describes how to use libpng, and the file example.c summarizes it
; with some code on which to build.  This file is useful for looking
; at the actual function definitions and structure components.  If that
; file has been stripped from your copy of libpng, you can find it at
; <http://www.libpng.org/pub/png/libpng-manual.txt>

; If you just need to read a PNG file and don't want to read the documentation
; skip to the end of this file and read the section entitled 'simplified API'.


; Version information for png.h - this should match the version in png.asm
PNG_LIBPNG_VER_STRING db '1.6.25',0
PNG_HEADER_VERSION_STRING db ' libpng version 1.6.25 - September 1, 2016',13,10,0

PNG_LIBPNG_VER_SONUM  equ 16
PNG_LIBPNG_VER_DLLNUM equ 16

; These should match the first 3 components of PNG_LIBPNG_VER_STRING:
PNG_LIBPNG_VER_MAJOR  equ 1
PNG_LIBPNG_VER_MINOR  equ 6
PNG_LIBPNG_VER_RELEASE equ 25

; This should match the numeric part of the final component of
; PNG_LIBPNG_VER_STRING, omitting any leading zero:


PNG_LIBPNG_VER_BUILD equ 0

; Release Status
PNG_LIBPNG_BUILD_ALPHA   equ 1
PNG_LIBPNG_BUILD_BETA    equ 2
PNG_LIBPNG_BUILD_RC      equ 3
PNG_LIBPNG_BUILD_STABLE  equ 4
PNG_LIBPNG_BUILD_RELEASE_STATUS_MASK equ 7

; Release-Specific Flags
PNG_LIBPNG_BUILD_PATCH   equ  8 ;Can be OR'ed with PNG_LIBPNG_BUILD_STABLE only
PNG_LIBPNG_BUILD_PRIVATE equ 16 ;Cannot be OR'ed with PNG_LIBPNG_BUILD_SPECIAL
PNG_LIBPNG_BUILD_SPECIAL equ 32 ;Cannot be OR'ed with PNG_LIBPNG_BUILD_PRIVATE

PNG_LIBPNG_BUILD_BASE_TYPE equ PNG_LIBPNG_BUILD_STABLE

; Careful here.  At one time, Guy wanted to use 082, but that would be octal.
; We must not include leading zeros.
; Versions 0.7 through 1.0.0 were in the range 0 to 100 here (only
; version 1.0.0 was mis-numbered 100 instead of 10000).  From
; version 1.0.1 it's    xxyyzz, where x=major, y=minor, z=release

PNG_LIBPNG_VER equ 10625 ;1.6.25

; Library configuration: these options cannot be changed after
; the library has been built.


; Added at libpng-1.2.8

; Ref MSDN: Private as priority over Special
; VS_FF_PRIVATEBUILD File *was not* built using standard release
; procedures. If this value is given, the StringFileInfo block must
; contain a PrivateBuild string.

; VS_FF_SPECIALBUILD File *was* built by the original company using
; standard release procedures but is a variation of the standard
; file of the same version number. If this value is given, the
; StringFileInfo block must contain a SpecialBuild string.

;if PNG_USER_PRIVATEBUILD /* From pnglibconf.h */
;#  define PNG_LIBPNG_BUILD_TYPE \
;       (PNG_LIBPNG_BUILD_BASE_TYPE | PNG_LIBPNG_BUILD_PRIVATE)
;#else
;#  ifdef PNG_LIBPNG_SPECIALBUILD
;#    define PNG_LIBPNG_BUILD_TYPE \
;         (PNG_LIBPNG_BUILD_BASE_TYPE | PNG_LIBPNG_BUILD_SPECIAL)
;#  else
;#    define PNG_LIBPNG_BUILD_TYPE (PNG_LIBPNG_BUILD_BASE_TYPE)
;#  endif
;end if

;#ifndef PNG_VERSION_INFO_ONLY

; Version information for C files, stored in png.asm.  This had better match
; the version above.

;#define png_libpng_ver png_get_header_ver(NULL)

; This file is arranged in several sections:

; 1. [omitted]
; 2. Any configuration options that can be specified by for the application
;    code when it is built.  (Build time configuration is in pnglibconf.h)
; 3. Type definitions (base types are defined in pngconf.h), structure
;    definitions.
; 4. Exported library functions.
; 5. Simplified API.
; 6. Implementation options.

; The library source code has additional files (principally pngpriv.h) that
; allow configuration of the library.


; Section 1: [omitted]

; Section 2: run time configuration
; See pnglibconf.h for build time configuration

; Run time configuration allows the application to choose between
; implementations of certain arithmetic APIs.  The default is set
; at build time and recorded in pnglibconf.h, but it is safe to
; override these (and only these) settings.  Note that this won't
; change what the library does, only application code, and the
; settings can (and probably should) be made on a per-file basis
; by setting the #defines before including png.h

; Use macros to read integers from PNG data or use the exported
; functions?
;   PNG_USE_READ_MACROS: use the macros (see below)  Note that
;     the macros evaluate their argument multiple times.
;   PNG_NO_USE_READ_MACROS: call the relevant library function.

; Use the alternative algorithm for compositing alpha samples that
; does not use division?
;   PNG_READ_COMPOSITE_NODIV_SUPPORTED: use the 'no division'
;      algorithm.
;   PNG_NO_READ_COMPOSITE_NODIV: use the 'division' algorithm.

; How to handle benign errors if PNG_ALLOW_BENIGN_ERRORS is
; false?
;   PNG_ALLOW_BENIGN_ERRORS: map calls to the benign error
;      APIs to png_warning.
; Otherwise the calls are mapped to png_error.


; Section 3: type definitions, including structures and compile time
; constants.
; See pngconf.h for base types that vary by machine/system


; This triggers a compiler error in png.c, if png.c and png.h
; do not agree upon the version number.

;typedef char* png_libpng_version_1_6_25;

; Basic control structions.  Read libpng-manual.txt or libpng.3 for more info.

; png_struct is the cache of information used while reading or writing a single
; PNG file.  One of these is always required, although the simplified API
; (below) hides the creation and destruction of it.

; png_info contains information read from or to be written to a PNG file.  One
; or more of these must exist while reading or creating a PNG file.  The
; information is not used by libpng during read but is used to control what
; gets written when a PNG file is created.  "png_get_" function calls read
; information during read and "png_set_" functions calls write information
; when creating a PNG.
; been moved into a separate header file that is not accessible to
; applications.  Read libpng-manual.txt or libpng.3 for more info.

; Types with names ending 'p' are pointer types.  The corresponding types with
; names ending 'rp' are identical pointer types except that the pointer is
; marked 'restrict', which means that it is the only pointer to the object
; passed to the function.  Applications should not use the 'restrict' types;
; it is always valid to pass 'p' to a pointer with a function argument of the
; corresponding 'rp' type.  Different compilers have different rules with
; regard to type matching in the presence of 'restrict'.  For backward
; compatibility libpng callbacks never have 'restrict' in their parameters and,
; consequentially, writing portable application code is extremely difficult if
; an attempt is made to use 'restrict'.

; Three color definitions.  The order of the red, green, and blue, (and the
; exact size) is not important, although the size of the fields need to
; be byte or uint_16 (as defined below).

struct png_color
	red   db ? ;byte
	green db ? ;byte
	blue  db ? ;byte
ends

struct png_color_16
	index db ? ;byte ;used for palette files
	red   dw ? ;uint_16 ;for use in red green blue files
	green dw ? ;uint_16
	blue  dw ? ;uint_16
	gray  dw ? ;uint_16 ;for use in grayscale files
ends

struct png_color_8
	red   db ? ;byte ;for use in red green blue files
	green db ? ;byte
	blue  db ? ;byte
	gray  db ? ;byte ;for use in grayscale files
	alpha db ? ;byte ;for alpha channel files
ends


; The following two structures are used for the in-core representation
; of sPLT chunks.

struct png_sPLT_entry
	red   dw ? ;uint_16
	green dw ? ;uint_16
	blue  dw ? ;uint_16
	alpha dw ? ;uint_16
	frequency dw ? ;uint_16
ends

; When the depth of the sPLT palette is 8 bits, the color and alpha samples
; occupy the LSB of their respective members, and the MSB of each member
; is zero-filled.  The frequency member always occupies the full 16 bits.


struct png_sPLT_t
	name    dd ? ;charp ;palette name
	depth   db ? ;byte ;depth of palette samples
	entries dd ? ;png_sPLT_entryp ;palette entries
	nentries dd ? ;int_32 ;number of palette entries
ends

if PNG_TEXT_SUPPORTED eq 1
; png_text holds the contents of a text/ztxt/itxt chunk in a PNG file,
; and whether that contents is compressed or not.  The "key" field
; points to a regular zero-terminated C string.  The "text" fields can be a
; regular C string, an empty string, or a NULL pointer.
; However, the structure returned by png_get_text() will always contain
; the "text" field as a regular zero-terminated C string (possibly
; empty), never a NULL pointer, so it can be safely used in printf() and
; other string-handling functions.  Note that the "itxt_length", "lang", and
; "lang_key" members of the structure only exist when the library is built
; with iTXt chunk support.  Prior to libpng-1.4.0 the library was built by
; default without iTXt support. Also note that when iTXt *is* supported,
; the "lang" and "lang_key" fields contain NULL pointers when the
; "compression" field contains * PNG_TEXT_COMPRESSION_NONE or
; PNG_TEXT_COMPRESSION_zTXt. Note that the "compression value" is not the
; same as what appears in the PNG tEXt/zTXt/iTXt chunk's "compression flag"
; which is always 0 or 1, or its "compression method" which is always 0.

struct png_text
	compression dd ? ;int ;compression value:
			;-1: tEXt, none
			; 0: zTXt, deflate
			; 1: iTXt, none
			; 2: iTXt, deflate
	key  dd ? ;charp ;keyword, 1-79 character description of "text"
	text dd ? ;charp ;comment, may be an empty string (ie "")
			; or a NULL pointer
	text_length dd ? ;png_size_t ;length of the text string
	itxt_length dd ? ;png_size_t ;length of the itxt string
	lang dd ? ;charp ;language code, 0-79 characters
			; or a NULL pointer
	lang_key dd ? ;charp ;keyword translated UTF-8 string, 0 or more
			; chars or a NULL pointer
ends
end if

; Supported compression types for text in PNG files (tEXt, and zTXt).
; The values of the PNG_TEXT_COMPRESSION_ defines should NOT be changed.
PNG_TEXT_COMPRESSION_NONE_WR equ -3
PNG_TEXT_COMPRESSION_zTXt_WR equ -2
PNG_TEXT_COMPRESSION_NONE    equ -1
PNG_TEXT_COMPRESSION_zTXt    equ 0
PNG_ITXT_COMPRESSION_NONE    equ 1
PNG_ITXT_COMPRESSION_zTXt    equ 2
PNG_TEXT_COMPRESSION_LAST    equ 3 ;Not a valid value

; png_time is a way to hold the time in an machine independent way.
; Two conversions are provided, both from time_t and struct tm.  There
; is no portable way to convert to either of these structures, as far
; as I know.  If you know of a portable way, send it to me.  As a side
; note - PNG has always been Year 2000 compliant!

struct png_time
	year  dw ? ;uint_16 ;full year, as in, 1995
	month db ? ;byte ;month of year, 1 - 12
	day   db ? ;byte ;day of month, 1 - 31
	hour  db ? ;byte ;hour of day, 0 - 23
	minute db ? ;byte ;minute of hour, 0 - 59
	second db ? ;byte ;second of minute, 0 - 60 (for leap seconds)
ends

if (PNG_STORE_UNKNOWN_CHUNKS_SUPPORTED eq 1) | (PNG_USER_CHUNKS_SUPPORTED eq 1)
; png_unknown_chunk is a structure to hold queued chunks for which there is
; no specific support.  The idea is that we can use this to queue
; up private chunks for output even though the library doesn't actually
; know about their semantics.

; The data in the structure is set by libpng on read and used on write.

struct png_unknown_chunk
	name rb 5 ;byte[5] ;Textual chunk name with '\0' terminator
	podata dd ? ;byte* ;Data, should not be modified on read!
	size dd ? ;png_size_t

	; On write 'location' must be set using the flag values listed below.
	; Notice that on read it is set by libpng however the values stored have
	; more bits set than are listed below.  Always treat the value as a
	; bitmask.  On write set only one bit - setting multiple bits may cause the
	; chunk to be written in multiple places.

	location db ? ;byte ;mode of operation at read time
ends
end if

; Flag values for the unknown chunk location byte.
PNG_HAVE_IHDR equ  0x01
PNG_HAVE_PLTE equ  0x02
PNG_AFTER_IDAT equ 0x08

; Maximum positive integer used in PNG is (2^31)-1
PNG_UINT_31_MAX equ 0x7fffffff ;uint_32
PNG_UINT_32_MAX equ -1 ;uint_32
PNG_SIZE_MAX    equ 0x60000000 ;1.5 Gb

; These are constants for fixed point values encoded in the
; PNG specification manner (x100000)

PNG_FP_1   equ 100000
PNG_FP_HALF equ 50000
PNG_FP_MAX equ ((png_fixed_point)0x7fffffffL)
PNG_FP_MIN equ (-PNG_FP_MAX)

; These describe the color_type field in png_info.
; color type masks
PNG_COLOR_MASK_PALETTE equ 1
PNG_COLOR_MASK_COLOR   equ 2
PNG_COLOR_MASK_ALPHA   equ 4

; color types.  Note that not all combinations are legal
PNG_COLOR_TYPE_GRAY equ 0
PNG_COLOR_TYPE_PALETTE equ (PNG_COLOR_MASK_COLOR or PNG_COLOR_MASK_PALETTE)
PNG_COLOR_TYPE_RGB      equ (PNG_COLOR_MASK_COLOR)
PNG_COLOR_TYPE_RGB_ALPHA equ (PNG_COLOR_MASK_COLOR or PNG_COLOR_MASK_ALPHA)
PNG_COLOR_TYPE_GRAY_ALPHA equ (PNG_COLOR_MASK_ALPHA)
; aliases
PNG_COLOR_TYPE_RGBA equ PNG_COLOR_TYPE_RGB_ALPHA
PNG_COLOR_TYPE_GA equ PNG_COLOR_TYPE_GRAY_ALPHA

; This is for compression type. PNG 1.0-1.2 only define the single type.
PNG_COMPRESSION_TYPE_BASE equ 0 ;Deflate method 8, 32K window
PNG_COMPRESSION_TYPE_DEFAULT equ PNG_COMPRESSION_TYPE_BASE

; This is for filter type. PNG 1.0-1.2 only define the single type.
PNG_FILTER_TYPE_BASE     equ 0 ;Single row per-byte filtering
PNG_INTRAPIXEL_DIFFERENCING equ 64 ;Used only in MNG datastreams
PNG_FILTER_TYPE_DEFAULT  equ PNG_FILTER_TYPE_BASE

; These are for the interlacing type.  These values should NOT be changed.
PNG_INTERLACE_NONE       equ 0 ;Non-interlaced image
PNG_INTERLACE_ADAM7      equ 1 ;Adam7 interlacing
PNG_INTERLACE_LAST       equ 2 ;Not a valid value

; These are for the oFFs chunk.  These values should NOT be changed.
PNG_OFFSET_PIXEL         equ 0 ;Offset in pixels
PNG_OFFSET_MICROMETER    equ 1 ;Offset in micrometers (1/10^6 meter)
PNG_OFFSET_LAST          equ 2 ;Not a valid value

; These are for the pCAL chunk.  These values should NOT be changed.
PNG_EQUATION_LINEAR      equ 0 ;Linear transformation
PNG_EQUATION_BASE_E      equ 1 ;Exponential base e transform
PNG_EQUATION_ARBITRARY   equ 2 ;Arbitrary base exponential transform
PNG_EQUATION_HYPERBOLIC  equ 3 ;Hyperbolic sine transformation
PNG_EQUATION_LAST        equ 4 ;Not a valid value

; These are for the sCAL chunk.  These values should NOT be changed.
PNG_SCALE_UNKNOWN        equ 0 ;unknown unit (image scale)
PNG_SCALE_METER          equ 1 ;meters per pixel
PNG_SCALE_RADIAN         equ 2 ;radians per pixel
PNG_SCALE_LAST           equ 3 ;Not a valid value

; These are for the pHYs chunk.  These values should NOT be changed.
PNG_RESOLUTION_UNKNOWN   equ 0 ;pixels/unknown unit (aspect ratio)
PNG_RESOLUTION_METER     equ 1 ;pixels/meter
PNG_RESOLUTION_LAST      equ 2 ;Not a valid value

; These are for the sRGB chunk.  These values should NOT be changed.
PNG_sRGB_INTENT_PERCEPTUAL equ 0
PNG_sRGB_INTENT_RELATIVE   equ 1
PNG_sRGB_INTENT_SATURATION equ 2
PNG_sRGB_INTENT_ABSOLUTE   equ 3
PNG_sRGB_INTENT_LAST       equ 4 ;Not a valid value

; This is for text chunks
PNG_KEYWORD_MAX_LENGTH equ 79

; Maximum number of entries in PLTE/sPLT/tRNS arrays
PNG_MAX_PALETTE_LENGTH equ 256

; These determine if an ancillary chunk's data has been successfully read
; from the PNG header, or if the application has filled in the corresponding
; data in the info_struct to be written into the output file.  The values
; of the PNG_INFO_<chunk> defines should NOT be changed.

PNG_INFO_gAMA equ 0x0001
PNG_INFO_sBIT equ 0x0002
PNG_INFO_cHRM equ 0x0004
PNG_INFO_PLTE equ 0x0008
PNG_INFO_tRNS equ 0x0010
PNG_INFO_bKGD equ 0x0020
PNG_INFO_hIST equ 0x0040
PNG_INFO_pHYs equ 0x0080
PNG_INFO_oFFs equ 0x0100
PNG_INFO_tIME equ 0x0200
PNG_INFO_pCAL equ 0x0400
PNG_INFO_sRGB equ 0x0800 ; GR-P, 0.96a
PNG_INFO_iCCP equ 0x1000 ; ESR, 1.0.6
PNG_INFO_sPLT equ 0x2000 ; ESR, 1.0.6
PNG_INFO_sCAL equ 0x4000 ; ESR, 1.0.6
PNG_INFO_IDAT equ 0x8000 ; ESR, 1.0.6

; This is used for the transformation routines, as some of them
; change these values for the row.  It also should enable using
; the routines for other purposes.

struct png_row_info ;png_row_info_struct
	width      dd ? ;uint_32 ;width of row
	rowbytes   dd ? ;png_size_t ;number of bytes in row
	color_type db ? ;byte ;color type of row
	bit_depth  db ? ;byte ;bit depth of row
	channels   db ? ;byte ;number of channels (1, 2, 3, or 4)
	pixel_depth db ? ;byte ;bits per pixel (depth * channels)
ends

if PNG_SETJMP_SUPPORTED eq 1
; This must match the function definition in <setjmp.h>, and the application
; must include this before png.h to obtain the definition of jmp_buf.  The
; function is required to be PNG_NORETURN, but this is not checked.  If the
; function does return the application will crash via an abort() or similar
; system level call.

; If you get a warning here while building the library you may need to make
; changes to ensure that pnglibconf.h records the calling convention used by
; your compiler.  This may be very difficult - try using a different compiler
; to build the library!

;PNG_FUNCTION(void, (PNGCAPI *png_longjmp_ptr), PNGARG((jmp_buf, int)), typedef);
end if

; Transform masks for the high-level interface
PNG_TRANSFORM_IDENTITY      equ 0x0000 ; read and write
PNG_TRANSFORM_STRIP_16      equ 0x0001 ; read only
PNG_TRANSFORM_STRIP_ALPHA   equ 0x0002 ; read only
PNG_TRANSFORM_PACKING       equ 0x0004 ; read and write
PNG_TRANSFORM_PACKSWAP      equ 0x0008 ; read and write
PNG_TRANSFORM_EXPAND        equ 0x0010 ; read only
PNG_TRANSFORM_INVERT_MONO   equ 0x0020 ; read and write
PNG_TRANSFORM_SHIFT         equ 0x0040 ; read and write
PNG_TRANSFORM_BGR           equ 0x0080 ; read and write
PNG_TRANSFORM_SWAP_ALPHA    equ 0x0100 ; read and write
PNG_TRANSFORM_SWAP_ENDIAN   equ 0x0200 ; read and write
PNG_TRANSFORM_INVERT_ALPHA  equ 0x0400 ; read and write
PNG_TRANSFORM_STRIP_FILLER  equ 0x0800 ; write only
; Added to libpng-1.2.34
PNG_TRANSFORM_STRIP_FILLER_BEFORE equ PNG_TRANSFORM_STRIP_FILLER
PNG_TRANSFORM_STRIP_FILLER_AFTER equ 0x1000 ; write only
; Added to libpng-1.4.0
PNG_TRANSFORM_GRAY_TO_RGB  equ 0x2000 ; read only
; Added to libpng-1.5.4
PNG_TRANSFORM_EXPAND_16    equ 0x4000 ;read only
;if INT_MAX >= 0x8000 ;else this might break
PNG_TRANSFORM_SCALE_16     equ 0x8000 ;read only
;end if

; Flags for MNG supported features
PNG_FLAG_MNG_EMPTY_PLTE    equ 0x01
PNG_FLAG_MNG_FILTER_64     equ 0x04
PNG_ALL_MNG_FEATURES       equ 0x05

; NOTE: prior to 1.5 these functions had no 'API' style declaration,
; this allowed the zlib default functions to be used on Windows
; platforms.  In 1.5 the zlib default malloc (which just calls malloc and
; ignores the first argument) should be completely compatible with the
; following.

; Section 4: exported functions
; Here are the function definitions most commonly used.  This is not
; the place to find out how to use libpng.  See libpng-manual.txt for the
; full explanation, see example.c for the summary.  This just provides
; a simple one line description of the use of each function.

; The PNG_EXPORT() and PNG_EXPORTA() macros used below are defined in
; pngconf.h and in the *.dfn files in the scripts directory.

;   PNG_EXPORT(ordinal, type, name, (args));

;       ordinal:    ordinal that is used while building
;                   *.def files. The ordinal value is only
;                   relevant when preprocessing png.h with
;                   the *.dfn files for building symbol table
;                   entries, and are removed by pngconf.h.
;       type:       return type of the function
;       name:       function name
;       args:       function arguments, with types

; When we wish to append attributes to a function prototype we use
; the PNG_EXPORTA() macro instead.

;   PNG_EXPORTA(ordinal, type, name, (args), attributes);

;       ordinal, type, name, and args: same as in PNG_EXPORT().
;       attributes: function attributes

macro PNG_EXPORT ordinal, typ, nam, arg
{
align 4
nam:
local .end_t
local .m_txt
jmp .end_t
	.m_txt db `nam,13,10,0
.end_t:
	stdcall dbg_print,txt_zv,.m_txt
ret
}

; Simple signature checking function.  This is the same as calling
; png_check_sig(sig, n) := !png_sig_cmp(sig, 0, n).

;#define png_check_sig(sig, n) !png_sig_cmp((sig), 0, (n))

macro png_setup_abs sum
{
local .end0
	and eax,0xff
if PNG_USE_ABS eq 1
	add sum,128
	sub al,128 ;v - 128
	cmp al,128
	jl @f
		neg al
		inc al ;abs(v - 128)
	@@:
	sub sum,eax
else
	cmp eax,128
	jl @f
		add sum,256
		sub sum,eax
		jmp .end0
	@@:
		add sum,eax
	.end0:
end if
}

; Allocate and initialize png_ptr struct for reading, and any other memory.
;PNG_EXPORTA(4, png_structp, png_create_read_struct,
;    (charp user_png_ver, voidp error_ptr,
;    png_error_ptr error_fn, png_error_ptr warn_fn),
;    PNG_ALLOCATED);

; Moved from pngconf.h in 1.4.0 and modified to ensure setjmp/longjmp
; match up.

; Read the information before the actual image data.
PNG_EXPORT 22, void, png_read_info, '(png_structrp png_ptr, png_inforp info_ptr)'

; Convert to a US string format: there is no localization support in this
; routine.  The original implementation used a 29 character buffer in
; png_struct, this will be removed in future versions.

;#if PNG_LIBPNG_VER < 10700
; To do: remove this from libpng17 (and from libpng17/png.c and pngstruct.h)
;PNG_EXPORTA(23, charp, png_convert_to_rfc1123, (png_structrp png_ptr,
;    png_const_timep ptime),PNG_DEPRECATED);
;end if

; Expand data to 24-bit RGB, or 8-bit grayscale, with alpha if available.
PNG_EXPORT 26, void, png_set_expand, '(png_structrp png_ptr)'
PNG_EXPORT 27, void, png_set_expand_gray_1_2_4_to_8, '(png_structrp png_ptr)'
PNG_EXPORT 28, void, png_set_palette_to_rgb, '(png_structrp png_ptr)'
PNG_EXPORT 29, void, png_set_tRNS_to_alpha, '(png_structrp png_ptr)'

; Expand to 16-bit channels, forces conversion of palette to RGB and expansion
; of a tRNS chunk if present.
PNG_EXPORT 221, void, png_set_expand_16, '(png_structrp png_ptr)'

; Expand the grayscale to 24-bit RGB if necessary.
PNG_EXPORT 31, void, png_set_gray_to_rgb, '(png_structrp png_ptr)'

; Reduce RGB to grayscale.
PNG_ERROR_ACTION_NONE equ 1
PNG_ERROR_ACTION_WARN equ 2
PNG_ERROR_ACTION_ERROR equ 3
PNG_RGB_TO_GRAY_DEFAULT equ (-1) ;for red/green coefficients

PNG_EXPORT 32, void, png_set_rgb_to_gray, '(png_structrp png_ptr, int error_action, double red, double green)'
PNG_EXPORT 33, void, png_set_rgb_to_gray_fixed, '(png_structrp png_ptr, int error_action, png_fixed_point red, png_fixed_point green)'

PNG_EXPORT 34, byte, png_get_rgb_to_gray_status, '(png_const_structrp png_ptr)'

PNG_EXPORT 35, void, png_build_grayscale_palette, '(int bit_depth, png_colorp palette)'

; How the alpha channel is interpreted - this affects how the color channels
; of a PNG file are returned to the calling application when an alpha channel,
; or a tRNS chunk in a palette file, is present.

; This has no effect on the way pixels are written into a PNG output
; datastream. The color samples in a PNG datastream are never premultiplied
; with the alpha samples.

; The default is to return data according to the PNG specification: the alpha
; channel is a linear measure of the contribution of the pixel to the
; corresponding composited pixel, and the color channels are unassociated
; (not premultiplied).  The gamma encoded color channels must be scaled
; according to the contribution and to do this it is necessary to undo
; the encoding, scale the color values, perform the composition and reencode
; the values.  This is the 'PNG' mode.

; The alternative is to 'associate' the alpha with the color information by
; storing color channel values that have been scaled by the alpha.
; image.  These are the 'STANDARD', 'ASSOCIATED' or 'PREMULTIPLIED' modes
; (the latter being the two common names for associated alpha color channels).

; For the 'OPTIMIZED' mode, a pixel is treated as opaque only if the alpha
; value is equal to the maximum value.

; The final choice is to gamma encode the alpha channel as well.  This is
; broken because, in practice, no implementation that uses this choice
; correctly undoes the encoding before handling alpha composition.  Use this
; choice only if other serious errors in the software or hardware you use
; mandate it; the typical serious error is for dark halos to appear around
; opaque areas of the composited PNG image because of arithmetic overflow.

; The API function png_set_alpha_mode specifies which of these choices to use
; with an enumerated 'mode' value and the gamma of the required output:

PNG_ALPHA_PNG          equ 0 ;according to the PNG standard
PNG_ALPHA_STANDARD     equ 1 ;according to Porter/Duff
PNG_ALPHA_ASSOCIATED   equ 1 ;as above; this is the normal practice
PNG_ALPHA_PREMULTIPLIED equ 1 ;as above
PNG_ALPHA_OPTIMIZED    equ 2 ;'PNG' for opaque pixels, else 'STANDARD'
PNG_ALPHA_BROKEN       equ 3 ;the alpha channel is gamma encoded

PNG_EXPORT 227, void, png_set_alpha_mode, '(png_structrp png_ptr, int mode, double output_gamma)'
PNG_EXPORT 228, void, png_set_alpha_mode_fixed, '(png_structrp png_ptr, int mode, png_fixed_point output_gamma)'

if (PNG_GAMMA_SUPPORTED eq 1) | (PNG_READ_ALPHA_MODE_SUPPORTED eq 1)
; The output_gamma value is a screen gamma in libpng terminology: it expresses
; how to decode the output values, not how they are encoded.

PNG_DEFAULT_sRGB equ -1       ;sRGB gamma and color space
PNG_GAMMA_MAC_18 equ -2       ;Old Mac '1.8' gamma and color space
PNG_GAMMA_sRGB   equ 220000   ;Television standards--matches sRGB gamma
PNG_GAMMA_LINEAR equ PNG_FP_1 ;Linear
end if

; The following are examples of calls to png_set_alpha_mode to achieve the
; required overall gamma correction and, where necessary, alpha
; premultiplication.

; png_set_alpha_mode(pp, PNG_ALPHA_PNG, PNG_DEFAULT_sRGB);
;    This is the default libpng handling of the alpha channel - it is not
;    pre-multiplied into the color components.  In addition the call states
;    that the output is for a sRGB system and causes all PNG files without gAMA
;    chunks to be assumed to be encoded using sRGB.

; png_set_alpha_mode(pp, PNG_ALPHA_PNG, PNG_GAMMA_MAC);
;    In this case the output is assumed to be something like an sRGB conformant
;    display preceeded by a power-law lookup table of power 1.45.  This is how
;    early Mac systems behaved.

; png_set_alpha_mode(pp, PNG_ALPHA_STANDARD, PNG_GAMMA_LINEAR);
;    This is the classic Jim Blinn approach and will work in academic
;    environments where everything is done by the book.  It has the shortcoming
;    of assuming that input PNG data with no gamma information is linear - this
;    is unlikely to be correct unless the PNG files where generated locally.
;    Most of the time the output precision will be so low as to show
;    significant banding in dark areas of the image.

; png_set_expand_16(pp);
; png_set_alpha_mode(pp, PNG_ALPHA_STANDARD, PNG_DEFAULT_sRGB);
;    This is a somewhat more realistic Jim Blinn inspired approach.  PNG files
;    are assumed to have the sRGB encoding if not marked with a gamma value and
;    the output is always 16 bits per component.  This permits accurate scaling
;    and processing of the data.  If you know that your input PNG files were
;    generated locally you might need to replace PNG_DEFAULT_sRGB with the
;    correct value for your system.

; png_set_alpha_mode(pp, PNG_ALPHA_OPTIMIZED, PNG_DEFAULT_sRGB);
;    If you just need to composite the PNG image onto an existing background
;    and if you control the code that does this you can use the optimization
;    setting.  In this case you just copy completely opaque pixels to the
;    output.  For pixels that are not completely transparent (you just skip
;    those) you do the composition math using png_composite or png_composite_16
;    below then encode the resultant 8-bit or 16-bit values to match the output
;    encoding.

; Other cases
;    If neither the PNG nor the standard linear encoding work for you because
;    of the software or hardware you use then you have a big problem.  The PNG
;    case will probably result in halos around the image.  The linear encoding
;    will probably result in a washed out, too bright, image (it's actually too
;    contrasty.)  Try the ALPHA_OPTIMIZED mode above - this will probably
;    substantially reduce the halos.  Alternatively try:

; png_set_alpha_mode(pp, PNG_ALPHA_BROKEN, PNG_DEFAULT_sRGB);
;    This option will also reduce the halos, but there will be slight dark
;    halos round the opaque parts of the image where the background is light.
;    In the OPTIMIZED mode the halos will be light halos where the background
;    is dark.  Take your pick - the halos are unavoidable unless you can get
;    your hardware/software fixed!  (The OPTIMIZED approach is slightly
;    faster.)

; When the default gamma of PNG files doesn't match the output gamma.
;    If you have PNG files with no gamma information png_set_alpha_mode allows
;    you to provide a default gamma, but it also sets the ouput gamma to the
;    matching value.  If you know your PNG files have a gamma that doesn't
;    match the output you can take advantage of the fact that
;    png_set_alpha_mode always sets the output gamma but only sets the PNG
;    default if it is not already set:

; png_set_alpha_mode(pp, PNG_ALPHA_PNG, PNG_DEFAULT_sRGB);
; png_set_alpha_mode(pp, PNG_ALPHA_PNG, PNG_GAMMA_MAC);
;    The first call sets both the default and the output gamma values, the
;    second call overrides the output gamma without changing the default.  This
;    is easier than achieving the same effect with png_set_gamma.  You must use
;    PNG_ALPHA_PNG for the first call - internal checking in png_set_alpha will
;    fire if more than one call to png_set_alpha_mode and png_set_background is
;    made in the same read operation, however multiple calls with PNG_ALPHA_PNG
;    are ignored.

PNG_EXPORT 36, void, png_set_strip_alpha, '(png_structrp png_ptr)'

; The values of the PNG_FILLER_ defines should NOT be changed
PNG_FILLER_BEFORE equ 0
PNG_FILLER_AFTER equ 1

;#if defined(PNG_READ_INVERT_SUPPORTED) || defined(PNG_WRITE_INVERT_SUPPORTED)

; Handle alpha and tRNS by replacing with a background color.  Prior to
; libpng-1.5.4 this API must not be called before the PNG file header has been
; read.  Doing so will result in unexpected behavior and possible warnings or
; errors if the PNG file contains a bKGD chunk.

PNG_EXPORT 47, void, png_set_background, '(png_structrp png_ptr, png_const_color_16p background_color, int background_gamma_code, int need_expand, double background_gamma)'
PNG_EXPORT 215, void, png_set_background_fixed, '(png_structrp png_ptr, png_const_color_16p background_color, int background_gamma_code, int need_expand, png_fixed_point background_gamma)'

if PNG_READ_BACKGROUND_SUPPORTED eq 1
PNG_BACKGROUND_GAMMA_UNKNOWN equ 0
PNG_BACKGROUND_GAMMA_SCREEN equ 1
PNG_BACKGROUND_GAMMA_FILE   equ 2
PNG_BACKGROUND_GAMMA_UNIQUE equ 3
end if

; Scale a 16-bit depth file down to 8-bit, accurately.
PNG_EXPORT 229, void, png_set_scale_16, '(png_structrp png_ptr)'

;#define PNG_READ_16_TO_8_SUPPORTED /* Name prior to 1.5.4 */
; Strip the second byte of information from a 16-bit depth file.
PNG_EXPORT 48, void, png_set_strip_16, '(png_structrp png_ptr)'

; Turn on quantizing, and reduce the palette to the number of colors
; available.

PNG_EXPORT 49, void, png_set_quantize, '(png_structrp png_ptr, png_colorp palette, int num_palette, int maximum_colors, png_const_uint_16p histogram, int full_quantize)'

; The threshold on gamma processing is configurable but hard-wired into the
; library.  The following is the floating point variant.

;#define PNG_GAMMA_THRESHOLD (PNG_GAMMA_THRESHOLD_FIXED*.00001)

; Handle gamma correction. Screen_gamma=(display_exponent).
; NOTE: this API simply sets the screen and file gamma values. It will
; therefore override the value for gamma in a PNG file if it is called after
; the file header has been read - use with care  - call before reading the PNG
; file for best results!

; These routines accept the same gamma values as png_set_alpha_mode (described
; above).  The PNG_GAMMA_ defines and PNG_DEFAULT_sRGB can be passed to either
; API (floating point or fixed.)  Notice, however, that the 'file_gamma' value
; is the inverse of a 'screen gamma' value.

PNG_EXPORT 50, void, png_set_gamma, '(png_structrp png_ptr, double screen_gamma, double override_file_gamma)'
PNG_EXPORT 208, void, png_set_gamma_fixed, '(png_structrp png_ptr, png_fixed_point screen_gamma, png_fixed_point override_file_gamma)'

; Optional update palette with requested transformations
PNG_EXPORT 53, void, png_start_read_image, '(png_structrp png_ptr)'

; Optional call to update the users info structure
PNG_EXPORT 54, void, png_read_update_info, '(png_structrp png_ptr, png_inforp info_ptr)'

; Read one or more rows of image data.
PNG_EXPORT 55, void, png_read_rows, '(png_structrp png_ptr, bytepp row, bytepp display_row, uint_32 num_rows)'

; Read a row of data.
PNG_EXPORT 56, void, png_read_row, '(png_structrp png_ptr, bytep row, bytep display_row)'

; Read the whole image into memory at once.
PNG_EXPORT 57, void, png_read_image, '(png_structrp png_ptr, bytepp image)'

; Read the end of the PNG file.
PNG_EXPORT 62, void, png_read_end, '(png_structrp png_ptr, png_inforp info_ptr)'

; Free any memory associated with the png_struct and the png_info_structs
PNG_EXPORT 64, void, png_destroy_read_struct, '(png_structpp png_ptr_ptr, png_infopp info_ptr_ptr, png_infopp end_info_ptr_ptr)'

; Set the libpng method of handling chunk CRC errors
PNG_EXPORT 66, void, png_set_crc_action, '(png_structrp png_ptr, int crit_action, int ancil_action)'

; Values for png_set_crc_action() say how to handle CRC errors in
; ancillary and critical chunks, and whether to use the data contained
; therein.  Note that it is impossible to "discard" data in a critical
; chunk.  For versions prior to 0.90, the action was always error/quit,
; whereas in version 0.90 and later, the action for CRC errors in ancillary
; chunks is warn/discard.  These values should NOT be changed.

;      value                       action:critical     action:ancillary

PNG_CRC_DEFAULT      equ 0 ;error/quit          warn/discard data
PNG_CRC_ERROR_QUIT   equ 1 ;error/quit          error/quit
PNG_CRC_WARN_DISCARD equ 2 ;(INVALID)           warn/discard data
PNG_CRC_WARN_USE     equ 3 ;warn/use data       warn/use data
PNG_CRC_QUIET_USE    equ 4 ;quiet/use data      quiet/use data
PNG_CRC_NO_CHANGE    equ 5 ;use current value   use current value

; Flags for png_set_filter() to say which filters to use.  The flags
; are chosen so that they don't conflict with real filter types
; below, in case they are supplied instead of the #defined constants.
; These values should NOT be changed.

PNG_NO_FILTERS   equ 0x00
PNG_FILTER_NONE  equ 0x08
PNG_FILTER_SUB   equ 0x10
PNG_FILTER_UP    equ 0x20
PNG_FILTER_AVG   equ 0x40
PNG_FILTER_PAETH equ 0x80
PNG_FAST_FILTERS equ (PNG_FILTER_NONE or PNG_FILTER_SUB or PNG_FILTER_UP)
PNG_ALL_FILTERS  equ (PNG_FAST_FILTERS or PNG_FILTER_AVG or PNG_FILTER_PAETH)

; Filter values (not flags) - used in pngwrite.c, pngwutil.c for now.
; These defines should NOT be changed.

PNG_FILTER_VALUE_NONE equ 0
PNG_FILTER_VALUE_SUB  equ 1
PNG_FILTER_VALUE_UP   equ 2
PNG_FILTER_VALUE_AVG  equ 3
PNG_FILTER_VALUE_PAETH equ 4
PNG_FILTER_VALUE_LAST equ 5

; The following are no longer used and will be removed from libpng-1.7:
PNG_FILTER_HEURISTIC_DEFAULT   equ 0 ;Currently "UNWEIGHTED"
PNG_FILTER_HEURISTIC_UNWEIGHTED equ 1 ;Used by libpng < 0.95
PNG_FILTER_HEURISTIC_WEIGHTED  equ 2 ;Experimental feature
PNG_FILTER_HEURISTIC_LAST      equ 3 ;Not a valid value

; These next functions are called for input/output, memory, and error
; handling.  They are in the file pngrio.c, pngwio.c, and pngerror.c,
; and call standard C I/O routines such as fread(), fwrite(), and
; fprintf().  These functions can be made to use other I/O routines
; at run time for those applications that need to handle I/O in a
; different manner by calling png_set_???_fn().  See libpng-manual.txt for
; more information.


; Replace the default data input function with a user supplied one.
PNG_EXPORT 78, void, png_set_read_fn, '(png_structrp png_ptr, voidp io_ptr, png_rw_ptr read_data_fn)'

PNG_EXPORT 80, void, png_set_read_status_fn, '(png_structrp png_ptr, png_read_status_ptr read_row_fn)'

PNG_EXPORT 84, void, png_set_read_user_transform_fn, '(png_structrp png_ptr, png_user_transform_ptr read_user_transform_fn)'

if PNG_PROGRESSIVE_READ_SUPPORTED eq 1
; Sets the function callbacks for the push reader, and a pointer to a
; user-defined structure available to the callback functions.

PNG_EXPORT 90, void, png_set_progressive_read_fn, '(png_structrp png_ptr, voidp progressive_ptr, png_progressive_info_ptr info_fn, png_progressive_row_ptr row_fn, png_progressive_end_ptr end_fn)'

; Returns the user pointer associated with the push read functions
PNG_EXPORT 91, voidp, png_get_progressive_ptr, '(png_const_structrp png_ptr)'

; Function to be called when data becomes available
PNG_EXPORT 92, void, png_process_data, '(png_structrp png_ptr, png_inforp info_ptr, bytep buffer, png_size_t buffer_size)'

; A function which may be called *only* within png_process_data to stop the
; processing of any more data.  The function returns the number of bytes
; remaining, excluding any that libpng has cached internally.  A subsequent
; call to png_process_data must supply these bytes again.  If the argument
; 'save' is set to true the routine will first save all the pending data and
; will always return 0.

PNG_EXPORT 219, png_size_t, png_process_data_pause, '(png_structrp, int save)'

; A function which may be called *only* outside (after) a call to
; png_process_data.  It returns the number of bytes of data to skip in the
; input.  Normally it will return 0, but if it returns a non-zero value the
; application must skip than number of bytes of input data and pass the
; following data to the next call to png_process_data.

PNG_EXPORT 220, uint_32, png_process_data_skip, '(png_structrp)'

; Function that combines rows.  'new_row' is a flag that should come from
; the callback and be non-NULL if anything needs to be done; the library
; stores its own version of the new data internally and ignores the passed
; in value.

PNG_EXPORT 93, void, png_progressive_combine_row, '(png_const_structrp png_ptr, bytep old_row, bytep new_row)'
end if ;PROGRESSIVE_READ

; Reassign responsibility for freeing existing data, whether allocated
; by libpng or by the application; this works on the png_info structure passed
; in, it does not change the state for other png_info structures.

; It is unlikely that this function works correctly as of 1.6.0 and using it
; may result either in memory leaks or double free of allocated data.

; Assignments for png_data_freer
PNG_DESTROY_WILL_FREE_DATA equ 1
PNG_SET_WILL_FREE_DATA equ 1
PNG_USER_WILL_FREE_DATA equ 2
; Flags for png_ptr->free_me and info_ptr->free_me
PNG_FREE_HIST equ 0x0008
PNG_FREE_ICCP equ 0x0010
PNG_FREE_SPLT equ 0x0020
PNG_FREE_ROWS equ 0x0040
PNG_FREE_PCAL equ 0x0080
PNG_FREE_SCAL equ 0x0100
if PNG_STORE_UNKNOWN_CHUNKS_SUPPORTED eq 1
	PNG_FREE_UNKN equ 0x0200
end if
PNG_FREE_LIST equ 0x0400 ;removed in 1.6.0 because it is ignored
PNG_FREE_PLTE equ 0x1000
PNG_FREE_TRNS equ 0x2000
PNG_FREE_TEXT equ 0x4000
PNG_FREE_ALL  equ 0x7fff
PNG_FREE_MUL  equ 0x4220 ;PNG_FREE_SPLT|PNG_FREE_TEXT|PNG_FREE_UNKN

;if PNG_ERROR_TEXT_SUPPORTED
; The same, but the chunk name is prepended to the error string.
;PNG_EXPORTA(103, void, png_chunk_error, (png_const_structrp png_ptr,
;    charp error_message), PNG_NORETURN);

;#else
; Fatal error in PNG image of libpng - can't continue
;PNG_EXPORTA(104, void, png_err, (png_const_structrp png_ptr), PNG_NORETURN);
;#  define png_chunk_error(s1,s2) png_err(s1)
;end if

; Non-fatal error in libpng, chunk name is prepended to message.
PNG_EXPORT 106, void, png_chunk_warning, '(png_const_structrp png_ptr, charp warning_message)'
;#else
;#  define png_warning(s1,s2) ((void)(s1))
;#  define png_chunk_warning(s1,s2) ((void)(s1))

; Benign error in libpng.  Can continue, but may have a problem.
; User can choose whether to handle as a fatal error or as a warning.
PNG_EXPORT 107, void, png_benign_error, '(png_const_structrp png_ptr, charp warning_message)'

;if PNG_READ_SUPPORTED
; Same, chunk name is prepended to message (only during read)
PNG_EXPORT 108, void, png_chunk_benign_error, '(png_const_structrp png_ptr, charp warning_message)'

;#else
;#  ifdef PNG_ALLOW_BENIGN_ERRORS
macro png_benign_error h,txt
{
	png_warning h,txt
}
;#    define png_chunk_benign_error png_chunk_warning
;#  else
;#    define png_benign_error png_error
;#    define png_chunk_benign_error png_chunk_error
;#  endif
;end if

; Returns number of color channels in image.
PNG_EXPORT 114, byte, png_get_channels, '(png_const_structrp png_ptr, png_const_inforp info_ptr)'

; Returns pixel aspect ratio, computed from pHYs chunk data.
PNG_EXPORT 125, float, png_get_pixel_aspect_ratio, '(png_const_structrp png_ptr, png_const_inforp info_ptr)'
PNG_EXPORT 210, png_fixed_point, png_get_pixel_aspect_ratio_fixed, '(png_const_structrp png_ptr, png_const_inforp info_ptr)'

; Returns image x, y offset in pixels or microns, from oFFs chunk data.
PNG_EXPORT 126, int_32, png_get_x_offset_pixels, '(png_const_structrp png_ptr, png_const_inforp info_ptr)'
PNG_EXPORT 127, int_32, png_get_y_offset_pixels, '(png_const_structrp png_ptr, png_const_inforp info_ptr)'
PNG_EXPORT 128, int_32, png_get_x_offset_microns, '(png_const_structrp png_ptr, png_const_inforp info_ptr)'
PNG_EXPORT 129, int_32, png_get_y_offset_microns, '(png_const_structrp png_ptr, png_const_inforp info_ptr)'

; Returns pointer to signature string read from PNG header
PNG_EXPORT 130, bytep, png_get_signature, '(png_const_structrp png_ptr, png_const_inforp info_ptr)'

PNG_EXPORT 131, uint_32, png_get_bKGD, '(png_const_structrp png_ptr, png_inforp info_ptr, png_color_16p *background)'

PNG_EXPORT 133, uint_32, png_get_cHRM, '(png_const_structrp png_ptr, png_const_inforp info_ptr, double *white_x, double *white_y, double *red_x, double *red_y, double *green_x, double *green_y, double *blue_x, double *blue_y)'
PNG_EXPORT 230, uint_32, png_get_cHRM_XYZ, '(png_const_structrp png_ptr, png_const_inforp info_ptr, double *red_X, double *red_Y, double *red_Z, double *green_X, double *green_Y, double *green_Z, double *blue_X, double *blue_Y, double *blue_Z)'
PNG_EXPORT 134, uint_32, png_get_cHRM_fixed, '(png_const_structrp png_ptr, png_const_inforp info_ptr, png_fixed_point *int_white_x, png_fixed_point *int_white_y, png_fixed_point *int_red_x, png_fixed_point *int_red_y, png_fixed_point *int_green_x, png_fixed_point *int_green_y, png_fixed_point *int_blue_x, png_fixed_point *int_blue_y)'

PNG_EXPORT 137, uint_32, png_get_gAMA, '(png_const_structrp png_ptr, png_const_inforp info_ptr, double *file_gamma)'
PNG_EXPORT 138, uint_32, png_get_gAMA_fixed, '(png_const_structrp png_ptr, png_const_inforp info_ptr, png_fixed_point *int_file_gamma)'

PNG_EXPORT 145, uint_32, png_get_oFFs, '(png_const_structrp png_ptr, png_const_inforp info_ptr, int_32 *offset_x, int_32 *offset_y, int *unit_type)'

PNG_EXPORT 147, uint_32, png_get_pCAL, '(png_const_structrp png_ptr, png_inforp info_ptr, charp *purpose, int_32 *X0, int_32 *X1, int *type, int *nparams, charp *units, charpp *params)'

PNG_EXPORT 149, uint_32, png_get_pHYs, '(png_const_structrp png_ptr, png_const_inforp info_ptr, uint_32 *res_x, uint_32 *res_y, int *unit_type)'

PNG_EXPORT 151, uint_32, png_get_PLTE, '(png_const_structrp png_ptr, png_inforp info_ptr, png_colorp *palette, int *num_palette)'

PNG_EXPORT 153, uint_32, png_get_sBIT, '(png_const_structrp png_ptr, png_inforp info_ptr, png_color_8p *sig_bit)'

PNG_EXPORT 155, uint_32, png_get_sRGB, '(png_const_structrp png_ptr, png_const_inforp info_ptr, int *file_srgb_intent)'

; png_get_text also returns the number of text chunks in *num_text
PNG_EXPORT 162, int, png_get_text, '(png_const_structrp png_ptr, png_inforp info_ptr, png_textp *text_ptr, int *num_text)'

PNG_EXPORT 164, uint_32, png_get_tIME, '(png_const_structrp png_ptr, png_inforp info_ptr, png_timep *mod_time)'

PNG_EXPORT 166, uint_32, png_get_tRNS, '(png_const_structrp png_ptr, png_inforp info_ptr, bytep *trans_alpha, int *num_trans, png_color_16p *trans_color)'

PNG_EXPORT 168, uint_32, png_get_sCAL, '(png_const_structrp png_ptr, png_const_inforp info_ptr, int *unit, double *width, double *height)'

; NOTE: this API is currently implemented using floating point arithmetic,
; consequently it can only be used on systems with floating point support.
; In any case the range of values supported by png_fixed_point is small and it
; is highly recommended that png_get_sCAL_s be used instead.

PNG_EXPORT 214, uint_32, png_get_sCAL_fixed, '(png_const_structrp png_ptr, png_const_inforp info_ptr, int *unit, png_fixed_point *width, png_fixed_point *height)'

PNG_EXPORT 170, void, png_set_sCAL, '(png_const_structrp png_ptr, png_inforp info_ptr, int unit, double width, double height)'
PNG_EXPORT 171, void, png_set_sCAL_s, '(png_const_structrp png_ptr, png_inforp info_ptr, int unit, charp swidth, charp sheight)'

; Provide the default handling for all unknown chunks or, optionally, for
; specific unknown chunks.

; NOTE: prior to 1.6.0 the handling specified for particular chunks on read was
; ignored and the default was used, the per-chunk setting only had an effect on
; write.  If you wish to have chunk-specific handling on read in code that must
; work on earlier versions you must use a user chunk callback to specify the
; desired handling (keep or discard.)

; The 'keep' parameter is a PNG_HANDLE_CHUNK_ value as listed below.  The
; parameter is interpreted as follows:

; READ:
;    PNG_HANDLE_CHUNK_AS_DEFAULT:
;       Known chunks: do normal libpng processing, do not keep the chunk (but
;          see the comments below about PNG_HANDLE_AS_UNKNOWN_SUPPORTED)
;       Unknown chunks: for a specific chunk use the global default, when used
;          as the default discard the chunk data.
;    PNG_HANDLE_CHUNK_NEVER:
;       Discard the chunk data.
;    PNG_HANDLE_CHUNK_IF_SAFE:
;       Keep the chunk data if the chunk is not critical else raise a chunk
;       error.
;    PNG_HANDLE_CHUNK_ALWAYS:
;       Keep the chunk data.

; If the chunk data is saved it can be retrieved using png_get_unknown_chunks,
; below.  Notice that specifying "AS_DEFAULT" as a global default is equivalent
; to specifying "NEVER", however when "AS_DEFAULT" is used for specific chunks
; it simply resets the behavior to the libpng default.

; INTERACTION WTIH USER CHUNK CALLBACKS:
; The per-chunk handling is always used when there is a png_user_chunk_ptr
; callback and the callback returns 0; the chunk is then always stored *unless*
; it is critical and the per-chunk setting is other than ALWAYS.  Notice that
; the global default is *not* used in this case.  (In effect the per-chunk
; value is incremented to at least IF_SAFE.)

; IMPORTANT NOTE: this behavior will change in libpng 1.7 - the global and
; per-chunk defaults will be honored.  If you want to preserve the current
; behavior when your callback returns 0 you must set PNG_HANDLE_CHUNK_IF_SAFE
; as the default - if you don't do this libpng 1.6 will issue a warning.

; If you want unhandled unknown chunks to be discarded in libpng 1.6 and
; earlier simply return '1' (handled).

; PNG_HANDLE_AS_UNKNOWN_SUPPORTED:
;    If this is *not* set known chunks will always be handled by libpng and
;    will never be stored in the unknown chunk list.  Known chunks listed to
;    png_set_keep_unknown_chunks will have no effect.  If it is set then known
;    chunks listed with a keep other than AS_DEFAULT will *never* be processed
;    by libpng, in addition critical chunks must either be processed by the
;    callback or saved.

;    The IHDR and IEND chunks must not be listed.  Because this turns off the
;    default handling for chunks that would otherwise be recognized the
;    behavior of libpng transformations may well become incorrect!

; WRITE:
;    When writing chunks the options only apply to the chunks specified by
;    png_set_unknown_chunks (below), libpng will *always* write known chunks
;    required by png_set_ calls and will always write the core critical chunks
;    (as required for PLTE).

;    Each chunk in the png_set_unknown_chunks list is looked up in the
;    png_set_keep_unknown_chunks list to find the keep setting, this is then
;    interpreted as follows:

;    PNG_HANDLE_CHUNK_AS_DEFAULT:
;       Write safe-to-copy chunks and write other chunks if the global
;       default is set to _ALWAYS, otherwise don't write this chunk.
;    PNG_HANDLE_CHUNK_NEVER:
;       Do not write the chunk.
;    PNG_HANDLE_CHUNK_IF_SAFE:
;       Write the chunk if it is safe-to-copy, otherwise do not write it.
;    PNG_HANDLE_CHUNK_ALWAYS:
;       Write the chunk.

; Note that the default behavior is effectively the opposite of the read case -
; in read unknown chunks are not stored by default, in write they are written
; by default.  Also the behavior of PNG_HANDLE_CHUNK_IF_SAFE is very different
; - on write the safe-to-copy bit is checked, on read the critical bit is
; checked and on read if the chunk is critical an error will be raised.

; num_chunks:
; ===========
;    If num_chunks is positive, then the "keep" parameter specifies the manner
;    for handling only those chunks appearing in the chunk_list array,
;    otherwise the chunk list array is ignored.

;    If num_chunks is 0 the "keep" parameter specifies the default behavior for
;    unknown chunks, as described above.

;    If num_chunks is negative, then the "keep" parameter specifies the manner
;    for handling all unknown chunks plus all chunks recognized by libpng
;    except for the IHDR, PLTE, tRNS, IDAT, and IEND chunks (which continue to
;    be processed by libpng.


; The "params" pointer is currently not used and is for future expansion.
PNG_EXPORT 178, void, png_read_png, '(png_structrp png_ptr, png_inforp info_ptr, int transforms, voidp params)'

; For use in png_set_keep_unknown, added to version 1.2.6
PNG_HANDLE_CHUNK_AS_DEFAULT  equ 0
PNG_HANDLE_CHUNK_NEVER       equ 1
PNG_HANDLE_CHUNK_IF_SAFE     equ 2
PNG_HANDLE_CHUNK_ALWAYS      equ 3
PNG_HANDLE_CHUNK_LAST        equ 4

; Removed from libpng 1.6; use png_get_io_chunk_type.
;PNG_REMOVED(200, bytep, png_get_io_chunk_name, (png_structrp png_ptr),
;    PNG_DEPRECATED)

; The flags returned by png_get_io_state() are the following:
PNG_IO_NONE        equ 0x0000 ; no I/O at this moment
PNG_IO_READING     equ 0x0001 ; currently reading
PNG_IO_WRITING     equ 0x0002 ; currently writing
PNG_IO_SIGNATURE   equ 0x0010 ; currently at the file signature
PNG_IO_CHUNK_HDR   equ 0x0020 ; currently at the chunk header
PNG_IO_CHUNK_DATA  equ 0x0040 ; currently at the chunk data
PNG_IO_CHUNK_CRC   equ 0x0080 ; currently at the chunk crc
PNG_IO_MASK_OP     equ 0x000f ; current operation: reading/writing
PNG_IO_MASK_LOC    equ 0x00f0 ; current location: sig/hdr/data/crc
;end if /* IO_STATE */

; Interlace support.  The following macros are always defined so that if
; libpng interlace handling is turned off the macros may be used to handle
; interlaced images within the application.

;#define PNG_INTERLACE_ADAM7_PASSES 7

; Two macros to return the first row and first column of the original,
; full, image which appears in a given pass.  'pass' is in the range 0
; to 6 and the result is in the range 0 to 7.

macro PNG_PASS_START_ROW pass
{
	push ebx ecx
	mov eax,pass
	not eax
	mov ebx,pass
	and eax,1
	shr ebx,1
	mov ecx,3
	sub ecx,ebx
	shl eax,cl
	and eax,7
	pop ecx ebx
}
macro PNG_PASS_START_COL pass
{
	push ebx ecx
	mov eax,pass
	mov ebx,pass
	and eax,1
	inc ebx
	shr ebx,1
	mov ecx,3
	sub ecx,ebx
	shl eax,cl
	and eax,7
	pop ecx ebx
}

; A macro to return the offset between pixels in the output row for a pair of
; pixels in the input - effectively the inverse of the 'COL_SHIFT' macro that
; follows.  Note that ROW_OFFSET is the offset from one row to the next whereas
; COL_OFFSET is from one column to the next, within a row.

;#define PNG_PASS_ROW_OFFSET(pass) ((pass)>2?(8>>(((pass)-1)>>1)):8)
;#define PNG_PASS_COL_OFFSET(pass) (1<<((7-(pass))>>1))

; Two macros to help evaluate the number of rows or columns in each
; pass.  This is expressed as a shift - effectively log2 of the number or
; rows or columns in each 8x8 tile of the original image.

macro PNG_PASS_ROW_SHIFT pass
{
local .end0
	mov eax,3
	cmp pass,2
	jle .end0
		mov eax,8
		sub eax,pass
		shr eax,1
	.end0:
}
macro PNG_PASS_COL_SHIFT pass
{
local .end0
	mov eax,3
	cmp pass,1
	jle .end0
		mov eax,7
		sub eax,pass
		shr eax,1
	.end0:
}

; Hence two macros to determine the number of rows or columns in a given
; pass of an image given its height or width.  In fact these macros may
; return non-zero even though the sub-image is empty, because the other
; dimension may be empty for a small image.

macro PNG_PASS_ROWS height, pass
{
push ecx
push ebx
	PNG_PASS_START_ROW pass 
	mov ebx,eax
	PNG_PASS_ROW_SHIFT pass
	mov ecx,eax
	xor eax,eax
	inc eax
	shl eax,cl
	dec eax
	sub eax,ebx
pop ebx
	add eax,height
	shr eax,cl
pop ecx
}
macro PNG_PASS_COLS width, pass
{
push ecx
push ebx
	PNG_PASS_START_COL pass 
	mov ebx,eax
	PNG_PASS_COL_SHIFT pass
	mov ecx,eax
	xor eax,eax
	inc eax
	shl eax,cl
	dec eax
	sub eax,ebx
pop ebx
	add eax,width
	shr eax,cl
pop ecx
}

; For the reader row callbacks (both progressive and sequential) it is
; necessary to find the row in the output image given a row in an interlaced
; image, so two more macros:

;#define PNG_ROW_FROM_PASS_ROW(y_in, pass) \
;   (((y_in)<<PNG_PASS_ROW_SHIFT(pass))+PNG_PASS_START_ROW(pass))
;#define PNG_COL_FROM_PASS_COL(x_in, pass) \
;   (((x_in)<<PNG_PASS_COL_SHIFT(pass))+PNG_PASS_START_COL(pass))

; Two macros which return a boolean (0 or 1) saying whether the given row
; or column is in a particular pass.  These use a common utility macro that
; returns a mask for a given pass - the offset 'off' selects the row or
; column version.  The mask has the appropriate bit set for each column in
; the tile.

;#define PNG_PASS_MASK(pass,off) ( \
;   ((0x110145AF>>(((7-(off))-(pass))<<2)) & 0xF) | \
;   ((0x01145AF0>>(((7-(off))-(pass))<<2)) & 0xF0))

;#define PNG_ROW_IN_INTERLACE_PASS(y, pass) \
;   ((PNG_PASS_MASK(pass,0) >> ((y)&7)) & 1)
;#define PNG_COL_IN_INTERLACE_PASS(x, pass) \
;   ((PNG_PASS_MASK(pass,1) >> ((x)&7)) & 1)

;if PNG_READ_COMPOSITE_NODIV_SUPPORTED
; With these routines we avoid an integer divide, which will be slower on
; most machines.  However, it does take more operations than the corresponding
; divide method, so it may be slower on a few RISC systems.  There are two
; shifts (by 8 or 16 bits) and an addition, versus a single integer divide.

; Note that the rounding factors are NOT supposed to be the same!  128 and
; 32768 are correct for the NODIV code; 127 and 32767 are correct for the
; standard method.

; [Optimized code by Greg Roelofs and Mark Adler...blame us for bugs. :-) ]


; fg and bg should be in `gamma 1.0' space; alpha is the opacity

;#  define png_composite(composite, fg, alpha, bg)        \
;   {                                                     \
;      uint_16 temp = (uint_16)((uint_16)(fg) \
;          * (uint_16)(alpha)                         \
;          + (uint_16)(bg)*(uint_16)(255          \
;          - (uint_16)(alpha)) + 128);                \
;      (composite) = (byte)(((temp + (temp >> 8)) >> 8) & 0xff); \
;   }

;#  define png_composite_16(composite, fg, alpha, bg)     \
;   {                                                     \
;      uint_32 temp = (uint_32)((uint_32)(fg) \
;          * (uint_32)(alpha)                         \
;          + (uint_32)(bg)*(65535                     \
;          - (uint_32)(alpha)) + 32768);              \
;      (composite) = (uint_16)(0xffff & ((temp + (temp >> 16)) >> 16)); \
;   }

;#else  /* Standard method using integer division */

;#  define png_composite(composite, fg, alpha, bg)                      \
;   (composite) =                                                       \
;       (byte)(0xff & (((uint_16)(fg) * (uint_16)(alpha) +  \
;       (uint_16)(bg) * (uint_16)(255 - (uint_16)(alpha)) + \
;       127) / 255))

;#  define png_composite_16(composite, fg, alpha, bg)                       \
;   (composite) =                                                           \
;       (uint_16)(0xffff & (((uint_32)(fg) * (uint_32)(alpha) + \
;       (uint_32)(bg)*(uint_32)(65535 - (uint_32)(alpha)) +     \
;       32767) / 65535))
;end if /* READ_COMPOSITE_NODIV */

PNG_EXPORT 201, uint_32, png_get_uint_32, '(bytep buf)'
PNG_EXPORT 202, uint_16, png_get_uint_16, '(bytep buf)'
PNG_EXPORT 203, int_32, png_get_int_32, '(bytep buf)'

PNG_EXPORT 204, uint_32, png_get_uint_31, '(png_const_structrp png_ptr, bytep buf)'
; No png_get_int_16 -- may be added if there's a real need for it.

;if PNG_USE_READ_MACROS
; Inline macros to do direct reads of bytes from the input buffer.
; The png_get_int_32() routine assumes we are using two's complement
; format for negative values, which is almost certainly true.

;#  define PNG_get_uint_32(buf) \
;   (((uint_32)(*(buf)) << 24) + \
;    ((uint_32)(*((buf) + 1)) << 16) + \
;    ((uint_32)(*((buf) + 2)) << 8) + \
;    ((uint_32)(*((buf) + 3))))

	; From libpng-1.4.0 until 1.4.4, the png_get_uint_16 macro (but not the
	; function) incorrectly returned a value of type uint_32.

;#  define PNG_get_uint_16(buf) \
;   ((uint_16) \
;    (((unsigned int)(*(buf)) << 8) + \
;    ((unsigned int)(*((buf) + 1)))))

;#  define PNG_get_int_32(buf) \
;   ((int_32)((*(buf) & 0x80) \
;    ? -((int_32)(((png_get_uint_32(buf)^0xffffffffU)+1U)&0x7fffffffU)) \
;    : (int_32)png_get_uint_32(buf)))

; If PNG_PREFIX is defined the same thing as below happens in pnglibconf.h,
; but defining a macro name prefixed with PNG_PREFIX.

;#  ifndef PNG_PREFIX
;#    define png_get_uint_32(buf) PNG_get_uint_32(buf)
;#    define png_get_uint_16(buf) PNG_get_uint_16(buf)
;#    define png_get_int_32(buf)  PNG_get_int_32(buf)
;#  endif
;#else
;#  ifdef PNG_PREFIX
	; No macros; revert to the (redefined) function
;#    define PNG_get_uint_32 (png_get_uint_32)
;#    define PNG_get_uint_16 (png_get_uint_16)
;#    define PNG_get_int_32  (png_get_int_32)
;#  endif
;end if

;/*******************************************************************************
; Section 5: SIMPLIFIED API
; *******************************************************************************

; Please read the documentation in libpng-manual.txt (TODO: write said
; documentation) if you don't understand what follows.

; The simplified API hides the details of both libpng and the PNG file format
; itself.  It allows PNG files to be read into a very limited number of
; in-memory bitmap formats or to be written from the same formats.  If these
; formats do not accomodate your needs then you can, and should, use the more
; sophisticated APIs above - these support a wide variety of in-memory formats
; and a wide variety of sophisticated transformations to those formats as well
; as a wide variety of APIs to manipulate ancillary information.

; To read a PNG file using the simplified API:

; 1) Declare a 'png_image' structure (see below) on the stack, set the
;    version field to PNG_IMAGE_VERSION and the 'opaque' pointer to NULL
;    (this is REQUIRED, your program may crash if you don't do it.)
; 2) Call the appropriate png_image_begin_read... function.
; 3) Set the png_image 'format' member to the required sample format.
; 4) Allocate a buffer for the image and, if required, the color-map.
; 5) Call png_image_finish_read to read the image and, if required, the
;    color-map into your buffers.

; There are no restrictions on the format of the PNG input itself; all valid
; color types, bit depths, and interlace methods are acceptable, and the
; input image is transformed as necessary to the requested in-memory format
; during the png_image_finish_read() step.  The only caveat is that if you
; request a color-mapped image from a PNG that is full-color or makes
; complex use of an alpha channel the transformation is extremely lossy and the
; result may look terrible.

; To write a PNG file using the simplified API:

; 1) Declare a 'png_image' structure on the stack and memset() it to all zero.
; 2) Initialize the members of the structure that describe the image, setting
;    the 'format' member to the format of the image samples.
; 3) Call the appropriate png_image_write... function with a pointer to the
;    image and, if necessary, the color-map to write the PNG data.

; png_image is a structure that describes the in-memory format of an image
; when it is being read or defines the in-memory format of an image that you
; need to write:

;#if defined(PNG_SIMPLIFIED_READ_SUPPORTED) || \
;    defined(PNG_SIMPLIFIED_WRITE_SUPPORTED)

PNG_IMAGE_VERSION equ 1

struct png_image
	opaque  dd ? ;png_controlp ;Initialize to NULL, free with png_image_free
	version dd ? ;uint_32  ;Set to PNG_IMAGE_VERSION
	width   dd ? ;uint_32  ;Image width in pixels (columns)
	height  dd ? ;uint_32  ;Image height in pixels (rows)
	format  dd ? ;uint_32  ;Image format as defined below
	flags   dd ? ;uint_32  ;A bit mask containing informational flags
	colormap_entries dd ? ;uint_32 ;Number of entries in the color-map

	; In the event of an error or warning the following field will be set to a
	; non-zero value and the 'message' field will contain a '\0' terminated
	; string with the libpng error or warning message.  If both warnings and
	; an error were encountered, only the error is recorded.  If there
	; are multiple warnings, only the first one is recorded.

	; The upper 30 bits of this value are reserved, the low two bits contain
	; a value as follows:

PNG_IMAGE_WARNING equ 1
PNG_IMAGE_ERROR equ 2

; The result is a two-bit code such that a value more than 1 indicates
; a failure in the API just called:

;    0 - no warning or error
;    1 - warning
;    2 - error
;    3 - error preceded by warning

;#  define PNG_IMAGE_FAILED(png_cntrl) ((((png_cntrl).warning_or_error)&0x03)>1)

	warning_or_error dd ? ;uint_32
	message rb 64 ;char[64]
ends

; The samples of the image have one to four channels whose components have
; original values in the range 0 to 1.0:

; 1: A single gray or luminance channel (G).
; 2: A gray/luminance channel and an alpha channel (GA).
; 3: Three red, green, blue color channels (RGB).
; 4: Three color channels and an alpha channel (RGBA).

; The components are encoded in one of two ways:

; a) As a small integer, value 0..255, contained in a single byte.  For the
; alpha channel the original value is simply value/255.  For the color or
; luminance channels the value is encoded according to the sRGB specification
; and matches the 8-bit format expected by typical display devices.

; The color/gray channels are not scaled (pre-multiplied) by the alpha
; channel and are suitable for passing to color management software.

; b) As a value in the range 0..65535, contained in a 2-byte integer.  All
; channels can be converted to the original value by dividing by 65535; all
; channels are linear.  Color channels use the RGB encoding (RGB end-points) of
; the sRGB specification.  This encoding is identified by the
; PNG_FORMAT_FLAG_LINEAR flag below.

; When the simplified API needs to convert between sRGB and linear colorspaces,
; the actual sRGB transfer curve defined in the sRGB specification (see the
; article at http://en.wikipedia.org/wiki/SRGB) is used, not the gamma=1/2.2
; approximation used elsewhere in libpng.

; When an alpha channel is present it is expected to denote pixel coverage
; of the color or luminance channels and is returned as an associated alpha
; channel: the color/gray channels are scaled (pre-multiplied) by the alpha
; value.

; The samples are either contained directly in the image data, between 1 and 8
; bytes per pixel according to the encoding, or are held in a color-map indexed
; by bytes in the image data.  In the case of a color-map the color-map entries
; are individual samples, encoded as above, and the image data has one byte per
; pixel to select the relevant sample from the color-map.


; PNG_FORMAT_*

; #defines to be used in png_image::format.  Each #define identifies a
; particular layout of sample data and, if present, alpha values.  There are
; separate defines for each of the two component encodings.

; A format is built up using single bit flag values.  All combinations are
; valid.  Formats can be built up from the flag values or you can use one of
; the predefined values below.  When testing formats always use the FORMAT_FLAG
; macros to test for individual features - future versions of the library may
; add new flags.

; When reading or writing color-mapped images the format should be set to the
; format of the entries in the color-map then png_image_{read,write}_colormap
; called to read or write the color-map and set the format correctly for the
; image data.  Do not set the PNG_FORMAT_FLAG_COLORMAP bit directly!

; NOTE: libpng can be built with particular features disabled. If you see
; compiler errors because the definition of one of the following flags has been
; compiled out it is because libpng does not have the required support.  It is
; possible, however, for the libpng configuration to enable the format on just
; read or just write; in that case you may see an error at run time.  You can
; guard against this by checking for the definition of the appropriate
; "_SUPPORTED" macro, one of:

;    PNG_SIMPLIFIED_{READ,WRITE}_{BGR,AFIRST}_SUPPORTED

PNG_FORMAT_FLAG_ALPHA    equ 0x01 ;format with an alpha channel
PNG_FORMAT_FLAG_COLOR    equ 0x02 ;color format: otherwise grayscale
PNG_FORMAT_FLAG_LINEAR   equ 0x04 ;2-byte channels else 1-byte
PNG_FORMAT_FLAG_COLORMAP equ 0x08 ;image data is color-mapped

;if PNG_FORMAT_BGR_SUPPORTED
PNG_FORMAT_FLAG_BGR   equ 0x10 ;BGR colors, else order is RGB
;end if

;if PNG_FORMAT_AFIRST_SUPPORTED
PNG_FORMAT_FLAG_AFIRST equ 0x20 ;alpha channel comes first
;end if

; Commonly used formats have predefined macros.

; First the single byte (sRGB) formats:

PNG_FORMAT_GRAY equ 0
PNG_FORMAT_GA  equ PNG_FORMAT_FLAG_ALPHA
PNG_FORMAT_AG  equ (PNG_FORMAT_GA|PNG_FORMAT_FLAG_AFIRST)
PNG_FORMAT_RGB equ PNG_FORMAT_FLAG_COLOR
PNG_FORMAT_BGR equ (PNG_FORMAT_FLAG_COLOR|PNG_FORMAT_FLAG_BGR)
PNG_FORMAT_RGBA equ (PNG_FORMAT_RGB|PNG_FORMAT_FLAG_ALPHA)
PNG_FORMAT_ARGB equ (PNG_FORMAT_RGBA|PNG_FORMAT_FLAG_AFIRST)
PNG_FORMAT_BGRA equ (PNG_FORMAT_BGR|PNG_FORMAT_FLAG_ALPHA)
PNG_FORMAT_ABGR equ (PNG_FORMAT_BGRA|PNG_FORMAT_FLAG_AFIRST)

; Then the linear 2-byte formats.  When naming these "Y" is used to
; indicate a luminance (gray) channel.

PNG_FORMAT_LINEAR_Y equ PNG_FORMAT_FLAG_LINEAR
PNG_FORMAT_LINEAR_Y_ALPHA equ (PNG_FORMAT_FLAG_LINEAR|PNG_FORMAT_FLAG_ALPHA)
PNG_FORMAT_LINEAR_RGB equ (PNG_FORMAT_FLAG_LINEAR|PNG_FORMAT_FLAG_COLOR)
PNG_FORMAT_LINEAR_RGB_ALPHA equ\
	(PNG_FORMAT_FLAG_LINEAR|PNG_FORMAT_FLAG_COLOR|PNG_FORMAT_FLAG_ALPHA)

; With color-mapped formats the image data is one byte for each pixel, the byte
; is an index into the color-map which is formatted as above.  To obtain a
; color-mapped format it is sufficient just to add the PNG_FOMAT_FLAG_COLORMAP
; to one of the above definitions, or you can use one of the definitions below.

PNG_FORMAT_RGB_COLORMAP equ (PNG_FORMAT_RGB|PNG_FORMAT_FLAG_COLORMAP)
PNG_FORMAT_BGR_COLORMAP equ (PNG_FORMAT_BGR|PNG_FORMAT_FLAG_COLORMAP)
PNG_FORMAT_RGBA_COLORMAP equ (PNG_FORMAT_RGBA|PNG_FORMAT_FLAG_COLORMAP)
PNG_FORMAT_ARGB_COLORMAP equ (PNG_FORMAT_ARGB|PNG_FORMAT_FLAG_COLORMAP)
PNG_FORMAT_BGRA_COLORMAP equ (PNG_FORMAT_BGRA|PNG_FORMAT_FLAG_COLORMAP)
PNG_FORMAT_ABGR_COLORMAP equ (PNG_FORMAT_ABGR|PNG_FORMAT_FLAG_COLORMAP)

; PNG_IMAGE macros

; These are convenience macros to derive information from a png_image
; structure.  The PNG_IMAGE_SAMPLE_ macros return values appropriate to the
; actual image sample values - either the entries in the color-map or the
; pixels in the image.  The PNG_IMAGE_PIXEL_ macros return corresponding values
; for the pixels and will always return 1 for color-mapped formats.  The
; remaining macros return information about the rows in the image and the
; complete image.

; NOTE: All the macros that take a png_image::format parameter are compile time
; constants if the format parameter is, itself, a constant.  Therefore these
; macros can be used in array declarations and case labels where required.
; Similarly the macros are also pre-processor constants (sizeof is not used) so
; they can be used in #if tests.

; First the information about the samples.

macro PNG_IMAGE_SAMPLE_CHANNELS fmt
{
	mov eax,fmt
	and eax,PNG_FORMAT_FLAG_COLOR or PNG_FORMAT_FLAG_ALPHA
	inc eax
}
; Return the total number of channels in a given format: 1..4

;#define PNG_IMAGE_SAMPLE_COMPONENT_SIZE(fmt)\
;   ((((fmt) & PNG_FORMAT_FLAG_LINEAR) >> 2)+1)
;   /* Return the size in bytes of a single component of a pixel or color-map
; entry (as appropriate) in the image: 1 or 2.


;#define PNG_IMAGE_SAMPLE_SIZE(fmt)\
;   (PNG_IMAGE_SAMPLE_CHANNELS(fmt) * PNG_IMAGE_SAMPLE_COMPONENT_SIZE(fmt))
;   /* This is the size of the sample data for one sample.  If the image is
; color-mapped it is the size of one color-map entry (and image pixels are
; one byte in size), otherwise it is the size of one image pixel.


;#define PNG_IMAGE_MAXIMUM_COLORMAP_COMPONENTS(fmt)\
;   (PNG_IMAGE_SAMPLE_CHANNELS(fmt) * 256)
;   /* The maximum size of the color-map required by the format expressed in a
; count of components.  This can be used to compile-time allocate a
; color-map:

; uint_16 colormap[PNG_IMAGE_MAXIMUM_COLORMAP_COMPONENTS(linear_fmt)];

; byte colormap[PNG_IMAGE_MAXIMUM_COLORMAP_COMPONENTS(sRGB_fmt)];

; Alternatively use the PNG_IMAGE_COLORMAP_SIZE macro below to use the
; information from one of the png_image_begin_read_ APIs and dynamically
; allocate the required memory.


; Corresponding information about the pixels
macro PNG_IMAGE_PIXEL_ p1,fmt
{
local .end0
local .end1
	mov eax,fmt
	and eax,PNG_FORMAT_FLAG_COLORMAP
	cmp eax,0
	je .end0
		xor eax,eax
		inc eax
		jmp .end1
	.end0:
		p1 fmt
	.end1:
}

macro PNG_IMAGE_PIXEL_CHANNELS fmt
{
	PNG_IMAGE_PIXEL_ PNG_IMAGE_SAMPLE_CHANNELS,fmt 
}
; The number of separate channels (components) in a pixel; 1 for a
; color-mapped image.


;#define PNG_IMAGE_PIXEL_COMPONENT_SIZE(fmt)\
;   PNG_IMAGE_PIXEL_(PNG_IMAGE_SAMPLE_COMPONENT_SIZE,fmt)
;   /* The size, in bytes, of each component in a pixel; 1 for a color-mapped
; image.


;#define PNG_IMAGE_PIXEL_SIZE(fmt) PNG_IMAGE_PIXEL_(PNG_IMAGE_SAMPLE_SIZE,fmt)
;   /* The size, in bytes, of a complete pixel; 1 for a color-mapped image. */

; Information about the whole row, or whole image
;#define PNG_IMAGE_ROW_STRIDE(image)\
;   (PNG_IMAGE_PIXEL_CHANNELS((image).format) * (image).width)
; Return the total number of components in a single row of the image; this
; is the minimum 'row stride', the minimum count of components between each
; row.  For a color-mapped image this is the minimum number of bytes in a
; row.

; WARNING: this macro overflows for some images with more than one component
; and very large image widths.  libpng will refuse to process an image where
; this macro would overflow.


;#define PNG_IMAGE_BUFFER_SIZE(image, row_stride)\
;   (PNG_IMAGE_PIXEL_COMPONENT_SIZE((image).format)*(image).height*(row_stride))
; Return the size, in bytes, of an image buffer given a png_image and a row
; stride - the number of components to leave space for in each row.

; WARNING: this macro overflows a 32-bit integer for some large PNG images,
; libpng will refuse to process an image where such an overflow would occur.


;#define PNG_IMAGE_SIZE(image)\
;   PNG_IMAGE_BUFFER_SIZE(image, PNG_IMAGE_ROW_STRIDE(image))
; Return the size, in bytes, of the image in memory given just a png_image;
; the row stride is the minimum stride required for the image.


;#define PNG_IMAGE_COLORMAP_SIZE(image)\
;   (PNG_IMAGE_SAMPLE_SIZE((image).format) * (image).colormap_entries)
; Return the size, in bytes, of the color-map of this image.  If the image
; format is not a color-map format this will return a size sufficient for
; 256 entries in the given format; check PNG_FORMAT_FLAG_COLORMAP if
; you don't want to allocate a color-map in this case.


; PNG_IMAGE_FLAG_*

; Flags containing additional information about the image are held in the
; 'flags' field of png_image.

PNG_IMAGE_FLAG_COLORSPACE_NOT_sRGB equ 0x01
; This indicates the the RGB values of the in-memory bitmap do not
; correspond to the red, green and blue end-points defined by sRGB.

PNG_IMAGE_FLAG_FAST equ 0x02
; On write emphasise speed over compression; the resultant PNG file will be
; larger but will be produced significantly faster, particular for large
; images.  Do not use this option for images which will be distributed, only
; used it when producing intermediate files that will be read back in
; repeatedly.  For a typical 24-bit image the option will double the read
; speed at the cost of increasing the image size by 25%, however for many
; more compressible images the PNG file can be 10 times larger with only a
; slight speed gain.

PNG_IMAGE_FLAG_16BIT_sRGB equ 0x04
; On read if the image is a 16-bit per component image and there is no gAMA
; or sRGB chunk assume that the components are sRGB encoded.  Notice that
; images output by the simplified API always have gamma information; setting
; this flag only affects the interpretation of 16-bit images from an
; external source.  It is recommended that the application expose this flag
; to the user; the user can normally easily recognize the difference between
; linear and sRGB encoding.  This flag has no effect on write - the data
; passed to the write APIs must have the correct encoding (as defined
; above.)

; If the flag is not set (the default) input 16-bit per component data is
; assumed to be linear.

; NOTE: the flag can only be set after the png_image_begin_read_ call,
; because that call initializes the 'flags' field.


;if PNG_SIMPLIFIED_READ_SUPPORTED
; READ APIs
; ---------

; The png_image passed to the read APIs must have been initialized by setting
; the png_controlp field 'opaque' to NULL (or, safer, memset the whole thing.)

if PNG_STDIO_SUPPORTED eq 1
PNG_EXPORT 234, int, png_image_begin_read_from_file, '(png_imagep image, const char *file_name)'
; The named file is opened for read and the image header is filled in
; from the PNG header in the file.

PNG_EXPORT 235, int, png_image_begin_read_from_stdio, '(png_imagep image, FILE* file)'
; The PNG header is read from the stdio FILE object.
end if ;STDIO

PNG_EXPORT 236, int, png_image_begin_read_from_memory, '(png_imagep image, png_const_voidp memory, png_size_t size)'
; The PNG header is read from the given memory buffer.

PNG_EXPORT 237, int, png_image_finish_read, '(png_imagep image, png_const_colorp background, void *buffer, int_32 row_stride, void *colormap)'
; Finish reading the image into the supplied buffer and clean up the
; png_image structure.

; row_stride is the step, in byte or 2-byte units as appropriate,
; between adjacent rows.  A positive stride indicates that the top-most row
; is first in the buffer - the normal top-down arrangement.  A negative
; stride indicates that the bottom-most row is first in the buffer.

; background need only be supplied if an alpha channel must be removed from
; a byte format and the removal is to be done by compositing on a solid
; color; otherwise it may be NULL and any composition will be done directly
; onto the buffer.  The value is an sRGB color to use for the background,
; for grayscale output the green channel is used.

; background must be supplied when an alpha channel must be removed from a
; single byte color-mapped output format, in other words if:

; 1) The original format from png_image_begin_read_from_* had
;    PNG_FORMAT_FLAG_ALPHA set.
; 2) The format set by the application does not.
; 3) The format set by the application has PNG_FORMAT_FLAG_COLORMAP set and
;    PNG_FORMAT_FLAG_LINEAR *not* set.

; For linear output removing the alpha channel is always done by compositing
; on black and background is ignored.

; colormap must be supplied when PNG_FORMAT_FLAG_COLORMAP is set.  It must
; be at least the size (in bytes) returned by PNG_IMAGE_COLORMAP_SIZE.
; image->colormap_entries will be updated to the actual number of entries
; written to the colormap; this may be less than the original value.


;end if /* SIMPLIFIED_READ */

;if PNG_SIMPLIFIED_WRITE_SUPPORTED
;/* WRITE APIS
; ----------
; For write you must initialize a png_image structure to describe the image to
; be written.  To do this use memset to set the whole structure to 0 then
; initialize fields describing your image.

; version: must be set to PNG_IMAGE_VERSION
; opaque: must be initialized to NULL
; width: image width in pixels
; height: image height in rows
; format: the format of the data (image and color-map) you wish to write
; flags: set to 0 unless one of the defined flags applies; set
;    PNG_IMAGE_FLAG_COLORSPACE_NOT_sRGB for color format images where the RGB
;    values do not correspond to the colors in sRGB.
; colormap_entries: set to the number of entries in the color-map (0 to 256)

; With all write APIs if image is in one of the linear formats with 16-bit
; data then setting convert_to_8_bit will cause the output to be an 8-bit PNG
; gamma encoded according to the sRGB specification, otherwise a 16-bit linear
; encoded PNG file is written.

; With color-mapped data formats the colormap parameter point to a color-map
; with at least image->colormap_entries encoded in the specified format.  If
; the format is linear the written PNG color-map will be converted to sRGB
; regardless of the convert_to_8_bit flag.

; With all APIs row_stride is handled as in the read APIs - it is the spacing
; from one row to the next in component sized units (1 or 2 bytes) and if
; negative indicates a bottom-up row layout in the buffer.  If row_stride is
; zero, libpng will calculate it for you from the image width and number of
; channels.

; Note that the write API does not support interlacing, sub-8-bit pixels or
; most ancillary chunks.  If you need to write text chunks (e.g. for copyright
; notices) you need to use one of the other APIs.


;#define png_image_write_get_memory_size(image, size, convert_to_8_bit, buffer,\
;   row_stride, colormap)\
;   png_image_write_to_memory(&(image), 0, &(size), convert_to_8_bit, buffer,\
;         row_stride, colormap)
; Return the amount of memory in 'size' required to compress this image.
; The png_image structure 'image' must be filled in as in the above
; function and must not be changed before the actual write call, the buffer
; and all other parameters must also be identical to that in the final
; write call.  The 'size' variable need not be initialized.

; NOTE: the macro returns true/false, if false is returned 'size' will be
; set to zero and the write failed and probably will fail if tried again.

; You can pre-allocate the buffer by making sure it is of sufficient size
; regardless of the amount of compression achieved.  The buffer size will
; always be bigger than the original image and it will never be filled.  The
; following macros are provided to assist in allocating the buffer.

;#define PNG_IMAGE_DATA_SIZE(image) (PNG_IMAGE_SIZE(image)+(image).height)
; The number of uncompressed bytes in the PNG byte encoding of the image;
; uncompressing the PNG IDAT data will give this number of bytes.

; NOTE: while PNG_IMAGE_SIZE cannot overflow for an image in memory this
; macro can because of the extra bytes used in the PNG byte encoding.  You
; need to avoid this macro if your image size approaches 2^30 in width or
; height.  The same goes for the remainder of these macros; they all produce
; bigger numbers than the actual in-memory image size.

;#ifndef PNG_ZLIB_MAX_SIZE
;#  define PNG_ZLIB_MAX_SIZE(b) ((b)+(((b)+7U)>>3)+(((b)+63U)>>6)+11U)
; An upper bound on the number of compressed bytes given 'b' uncompressed
; bytes.  This is based on deflateBounds() in zlib; different
; implementations of zlib compression may conceivably produce more data so
; if your zlib implementation is not zlib itself redefine this macro
; appropriately.

;end if

;#define PNG_IMAGE_COMPRESSED_SIZE_MAX(image)\
;   PNG_ZLIB_MAX_SIZE((png_alloc_size_t)PNG_IMAGE_DATA_SIZE(image))
;   /* An upper bound on the size of the data in the PNG IDAT chunks. */
;
;#define PNG_IMAGE_PNG_SIZE_MAX_(image, image_size)\
;   ((8U/*sig*/+25U/*IHDR*/+16U/*gAMA*/+44U/*cHRM*/+12U/*IEND*/+\
;    (((image).format&PNG_FORMAT_FLAG_COLORMAP)?/*colormap: PLTE, tRNS*/\
;    12U+3U*(image).colormap_entries/*PLTE data*/+\
;    (((image).format&PNG_FORMAT_FLAG_ALPHA)?\
;    12U/*tRNS*/+(image).colormap_entries:0U):0U)+\
;    12U)+(12U*((image_size)/PNG_ZBUF_SIZE))/*IDAT*/+(image_size))
;   /* A helper for the following macro; if your compiler cannot handle the
; following macro use this one with the result of
; PNG_IMAGE_COMPRESSED_SIZE_MAX(image) as the second argument (most
; compilers should handle this just fine.)


;#define PNG_IMAGE_PNG_SIZE_MAX(image)\
;   PNG_IMAGE_PNG_SIZE_MAX_(image, PNG_IMAGE_COMPRESSED_SIZE_MAX(image))
; An upper bound on the total length of the PNG data stream for 'image'.
; The result is of type png_alloc_size_t, on 32-bit systems this may
; overflow even though PNG_IMAGE_DATA_SIZE does not overflow; the write will
; run out of buffer space but return a corrected size which should work.

;end if /* SIMPLIFIED_WRITE */
;/*******************************************************************************
;  END OF SIMPLIFIED API
; ******************************************************************************/
;end if /* SIMPLIFIED_{READ|WRITE} */

;/*******************************************************************************
; Section 6: IMPLEMENTATION OPTIONS
; *******************************************************************************

; Support for arbitrary implementation-specific optimizations.  The API allows
; particular options to be turned on or off.  'Option' is the number of the
; option and 'onoff' is 0 (off) or non-0 (on).  The value returned is given
; by the PNG_OPTION_ defines below.

; HARDWARE: normally hardware capabilites, such as the Intel SSE instructions,
;           are detected at run time, however sometimes it may be impossible
;           to do this in user mode, in which case it is necessary to discover
;           the capabilities in an OS specific way.  Such capabilities are
;           listed here when libpng has support for them and must be turned
;           ON by the application if present.

; SOFTWARE: sometimes software optimizations actually result in performance
;           decrease on some architectures or systems, or with some sets of
;           PNG images.  'Software' options allow such optimizations to be
;           selected at run time.

if PNG_SET_OPTION_SUPPORTED eq 1
if PNG_ARM_NEON_API_SUPPORTED eq 1
	PNG_ARM_NEON  equ 0 ;HARDWARE: ARM Neon SIMD instructions supported
end if
PNG_MAXIMUM_INFLATE_WINDOW equ 2 ;SOFTWARE: force maximum window
PNG_SKIP_sRGB_CHECK_PROFILE equ 4 ;SOFTWARE: Check ICC profile for sRGB
if PNG_MIPS_MSA_API_SUPPORTED eq 1
	PNG_MIPS_MSA  equ 6 ;HARDWARE: MIPS Msa SIMD instructions supported
end if
PNG_OPTION_NEXT equ 8 ;Next option - numbers must be even

; Return values: NOTE: there are four values and 'off' is *not* zero
PNG_OPTION_UNSET  equ 0 ;Unset - defaults to off
PNG_OPTION_INVALID equ 1 ;Option number out of range
PNG_OPTION_OFF    equ 2
PNG_OPTION_ON     equ 3
end if ;SET_OPTION

;/*******************************************************************************
;  END OF HARDWARE AND SOFTWARE OPTIONS
; ******************************************************************************/

; Maintainer: Put new public prototypes here ^, in libpng.3, in project
; defs, and in scripts/symbols.def.


; The last ordinal number (this is the *last* one already used; the next
; one to use is one more than this.)

;end if /* PNG_VERSION_INFO_ONLY */
; Do not put anything past this line
