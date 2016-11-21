;
; Options
;

PNG_RELEASE_BUILD equ 1

;---
; Various modes of operation.  Note that after an init, mode is set to
; zero automatically when the structure is created.  Three of these
; are defined in png.inc because they need to be visible to applications
; that call png_set_unknown_chunk().

;PNG_HAVE_IHDR            0x01 (defined in png.inc)
;PNG_HAVE_PLTE            0x02 (defined in png.inc)
PNG_HAVE_IDAT              equ 0x04
;PNG_AFTER_IDAT           0x08 (defined in png.inc)
PNG_HAVE_IEND              equ 0x10
	;0x20 (unused)
	;0x40 (unused)
	;0x80 (unused)
PNG_HAVE_CHUNK_HEADER     equ 0x100
PNG_WROTE_tIME            equ 0x200
PNG_WROTE_INFO_BEFORE_PLTE equ 0x400
PNG_BACKGROUND_IS_GRAY    equ 0x800
PNG_HAVE_PNG_SIGNATURE   equ 0x1000
PNG_HAVE_CHUNK_AFTER_IDAT equ 0x2000 ;Have another chunk after IDAT
	;0x4000 (unused)
PNG_IS_READ_STRUCT       equ 0x8000 ;Else is a write struct

; Flags for the transformations the PNG library does on the image data
PNG_BGR                equ 0x0001
PNG_INTERLACE          equ 0x0002
PNG_PACK               equ 0x0004
PNG_SHIFT              equ 0x0008
PNG_SWAP_BYTES         equ 0x0010
PNG_INVERT_MONO        equ 0x0020
PNG_QUANTIZE           equ 0x0040
PNG_COMPOSE            equ 0x0080 ;Was PNG_BACKGROUND
PNG_BACKGROUND_EXPAND  equ 0x0100
PNG_EXPAND_16          equ 0x0200 ;Added to libpng 1.5.2
PNG_16_TO_8            equ 0x0400 ;Becomes 'chop' in 1.5.4
PNG_RGBA               equ 0x0800
PNG_EXPAND             equ 0x1000
PNG_GAMMA              equ 0x2000
PNG_GRAY_TO_RGB        equ 0x4000
PNG_FILLER             equ 0x8000
PNG_PACKSWAP          equ 0x10000
PNG_SWAP_ALPHA        equ 0x20000
PNG_STRIP_ALPHA       equ 0x40000
PNG_INVERT_ALPHA      equ 0x80000
PNG_USER_TRANSFORM   equ 0x100000
PNG_RGB_TO_GRAY_ERR  equ 0x200000
PNG_RGB_TO_GRAY_WARN equ 0x400000
PNG_RGB_TO_GRAY      equ 0x600000 ;two bits, RGB_TO_GRAY_ERR|WARN
PNG_ENCODE_ALPHA     equ 0x800000 ;Added to libpng-1.5.4
PNG_ADD_ALPHA       equ 0x1000000 ;Added to libpng-1.2.7
PNG_EXPAND_tRNS     equ 0x2000000 ;Added to libpng-1.2.9
PNG_SCALE_16_TO_8   equ 0x4000000 ;Added to libpng-1.5.4
	;0x8000000 unused
	;0x10000000 unused
	;0x20000000 unused
	;0x40000000 unused
; Flags for png_create_struct
PNG_STRUCT_PNG  equ 0x0001
PNG_STRUCT_INFO equ 0x0002

; Flags for the png_ptr->flags rather than declaring a byte for each one
PNG_FLAG_ZLIB_CUSTOM_STRATEGY    equ 0x0001
PNG_FLAG_ZSTREAM_INITIALIZED     equ 0x0002 ;Added to libpng-1.6.0
	;0x0004    unused
PNG_FLAG_ZSTREAM_ENDED           equ 0x0008 ;Added to libpng-1.6.0
	;0x0010    unused
	;0x0020    unused
PNG_FLAG_ROW_INIT                equ 0x0040
PNG_FLAG_FILLER_AFTER            equ 0x0080
PNG_FLAG_CRC_ANCILLARY_USE       equ 0x0100
PNG_FLAG_CRC_ANCILLARY_NOWARN    equ 0x0200
PNG_FLAG_CRC_CRITICAL_USE        equ 0x0400
PNG_FLAG_CRC_CRITICAL_IGNORE     equ 0x0800
PNG_FLAG_ASSUME_sRGB             equ 0x1000 ;Added to libpng-1.5.4
PNG_FLAG_OPTIMIZE_ALPHA          equ 0x2000 ;Added to libpng-1.5.4
PNG_FLAG_DETECT_UNINITIALIZED    equ 0x4000 ;Added to libpng-1.5.4
;PNG_FLAG_KEEP_UNKNOWN_CHUNKS      0x8000
;PNG_FLAG_KEEP_UNSAFE_CHUNKS      0x10000
PNG_FLAG_LIBRARY_MISMATCH       equ 0x20000
PNG_FLAG_STRIP_ERROR_NUMBERS    equ 0x40000
PNG_FLAG_STRIP_ERROR_TEXT       equ 0x80000
PNG_FLAG_BENIGN_ERRORS_WARN    equ 0x100000 ;Added to libpng-1.4.0
PNG_FLAG_APP_WARNINGS_WARN     equ 0x200000 ;Added to libpng-1.6.0
PNG_FLAG_APP_ERRORS_WARN       equ 0x400000 ;Added to libpng-1.6.0

; Gamma values (new at libpng-1.5.4):
PNG_GAMMA_MAC_OLD equ 151724  ;Assume '1.8' is really 2.2/1.45!
PNG_GAMMA_MAC_INVERSE equ 65909
PNG_GAMMA_sRGB_INVERSE equ 45455

macro PNG_ROWBYTES pixel_bits, width
{
local .end0
if pixel_bits eq eax
else
	mov eax,pixel_bits
end if
	cmp eax,8
	jge .end0
		add eax,7
	.end0:
	shr eax,3
	imul eax,width
}

; In 1.7.0 the definitions will be made public in png.inc to avoid having to
; duplicate the same definitions in application code.

png_IDAT equ 'IDAT'
png_IEND equ 'IEND'
png_IHDR equ 'IHDR'
png_PLTE equ 'PLTE'
png_bKGD equ 'bKGD'
png_cHRM equ 'cHRM'
png_fRAc equ 'fRAc' ;registered, not defined
png_gAMA equ 'gAMA'
png_gIFg equ 'gIFg'
png_gIFt equ 'gIFt' ;deprecated
png_gIFx equ 'gIFx'
png_hIST equ 'hIST'
png_iCCP equ 'iCCP'
png_iTXt equ 'iTXt'
png_oFFs equ 'oFFs'
png_pCAL equ 'pCAL'
png_pHYs equ 'pHYs'
png_sBIT equ 'sBIT'
png_sCAL equ 'sCAL'
png_sPLT equ 'sPLT'
png_sRGB equ 'sRGB'
png_sTER equ 'sTER'
png_tEXt equ 'tEXt'
png_tIME equ 'tIME'
png_tRNS equ 'tRNS'
png_zTXt equ 'zTXt'

;Test on flag values as defined in the spec (section 5.4):
macro PNG_CHUNK_ANCILLARY c
{
	mov eax,c
	shr eax,29
	and eax,1
}
macro PNG_CHUNK_CRITICAL c
{
	PNG_CHUNK_ANCILLARY c
	xor eax,1
}
macro PNG_CHUNK_PRIVATE c
{
	mov eax,c
	shr eax,21
	and eax,1
}
macro PNG_CHUNK_RESERVED c
{
	mov eax,c
	shr eax,13
	and eax,1
}
macro PNG_CHUNK_SAFE_TO_COPY c
{
	mov eax,c
	shr eax,5
	and eax,1
}

PNG_FLAG_CRC_ANCILLARY_MASK equ (PNG_FLAG_CRC_ANCILLARY_USE or PNG_FLAG_CRC_ANCILLARY_NOWARN)
PNG_FLAG_CRC_CRITICAL_MASK  equ (PNG_FLAG_CRC_CRITICAL_USE or PNG_FLAG_CRC_CRITICAL_IGNORE)
PNG_FLAG_CRC_MASK           equ (PNG_FLAG_CRC_ANCILLARY_MASK or PNG_FLAG_CRC_CRITICAL_MASK)

macro PNG_sRGB_FROM_LINEAR linear
{
	mov eax,linear
	shr eax,15
	shl eax,1
	add eax,png_sRGB_base
	movzx eax,word[eax]
push ebx ecx
	mov ebx,linear
	shr ebx,15
	add ebx,png_sRGB_delta
	mov ecx,linear
	and ecx,0x7fff
	imul ecx,ebx
	shr ecx,12
	add eax,ecx
pop ecx ebx
	shr eax,8
	;;;and eax,0xff
}
; Given a value 'linear' in the range 0..255*65535 calculate the 8-bit sRGB
; encoded value with maximum error 0.646365.  Note that the input is not a
; 16-bit value; it has been multiplied by 255!

PNG_UNEXPECTED_ZLIB_RETURN equ (-7)

;...

; Suggested size for a number buffer (enough for 64 bits and a sign!)
PNG_NUMBER_BUFFER_SIZE equ 24

; These are the integer formats currently supported, the name is formed from
; the standard printf(3) format string.

PNG_NUMBER_FORMAT_u    equ 1 ;chose unsigned API!
PNG_NUMBER_FORMAT_02u  equ 2
PNG_NUMBER_FORMAT_d    equ 1 ;chose signed API!
PNG_NUMBER_FORMAT_02d  equ 2
PNG_NUMBER_FORMAT_x    equ 3
PNG_NUMBER_FORMAT_02x  equ 4
PNG_NUMBER_FORMAT_fixed equ 5 ;choose the signed API

; New defines and members adding in libpng-1.5.4
PNG_WARNING_PARAMETER_SIZE equ 32
PNG_WARNING_PARAMETER_COUNT equ 8 ;Maximum 9; see pngerror.asm

PNG_CHUNK_WARNING     equ 0 ;never an error
PNG_CHUNK_WRITE_ERROR equ 1 ;an error only on write
PNG_CHUNK_ERROR       equ 2 ;always an error

; ASCII to FP interfaces, currently only implemented if sCAL
; support is required.

; MAX_DIGITS is actually the maximum number of characters in an sCAL
; width or height, derived from the precision (number of significant
; digits - a build time settable option) and assumptions about the
; maximum ridiculous exponent.

PNG_sCAL_MAX_DIGITS equ PNG_sCAL_PRECISION+1+1+10 ;. E exponent

; An internal API to validate the format of a floating point number.
; The result is the index of the next character.  If the number is
; not valid it will be the index of a character in the supposed number.

; The format of a number is defined in the PNG extensions specification
; and this API is strictly conformant to that spec, not anyone elses!

; The format as a regular expression is:

; [+-]?[0-9]+.?([Ee][+-]?[0-9]+)?

; or:

; [+-]?.[0-9]+(.[0-9]+)?([Ee][+-]?[0-9]+)?

; The complexity is that either integer or fraction must be present and the
; fraction is permitted to have no digits only if the integer is present.

; NOTE: The dangling E problem.
;   There is a PNG valid floating point number in the following:

;       PNG floating point numbers are not greedy.

;   Working this out requires *TWO* character lookahead (because of the
;   sign), the parser does not do this - it will fail at the 'r' - this
;   doesn't matter for PNG sCAL chunk values, but it requires more care
;   if the value were ever to be embedded in something more complex.  Use
;   ANSI-C strtod if you need the lookahead.

; State table for the parser.
PNG_FP_INTEGER   equ 0 ;before or in integer
PNG_FP_FRACTION  equ 1 ;before or in fraction
PNG_FP_EXPONENT  equ 2 ;before or in exponent
PNG_FP_STATE     equ 3 ;mask for the above
PNG_FP_SAW_SIGN  equ 4 ;Saw +/- in current state
PNG_FP_SAW_DIGIT equ 8 ;Saw a digit in current state
PNG_FP_SAW_DOT  equ 16 ;Saw a dot in current state
PNG_FP_SAW_E    equ 32 ;Saw an E (or e) in current state
PNG_FP_SAW_ANY  equ 60 ;Saw any of the above 4

; These three values don't affect the parser.  They are set but not used.

PNG_FP_WAS_VALID equ 64 ;Preceding substring is a valid fp number
PNG_FP_NEGATIVE equ 128 ;A negative number, including "-0"
PNG_FP_NONZERO  equ 256 ;A non-zero value
PNG_FP_STICKY   equ 448 ;The above three flags

; This is available for the caller to store in 'state' if required.  Do not
; call the parser after setting it (the parser sometimes clears it.)

PNG_FP_INVALID equ 512 ;Available for callers as a distinct value

; Result codes for the parser (boolean - true meants ok, false means
; not ok yet.)

PNG_FP_MAYBE     equ 0 ;The number may be valid in the future
PNG_FP_OK        equ 1 ;The number is valid

; The internal structure that png_image::opaque points to.
struct png_control
	png_ptr dd ? ;png_structp
	info_ptr dd ? ;png_infop
	error_buf dd ? ;voidp ;Always a jmp_buf at present.

	memory dd ? ;bytep ;Memory buffer.
	size dd ? ;png_size_t ;Size of the memory buffer.

	for_write dd ? ;uint ;:1;Otherwise it is a read structure
	owned_file dd ? ;uint ;:1;We own the file in io_ptr
ends
