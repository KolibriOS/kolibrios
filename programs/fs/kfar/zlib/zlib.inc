; zlib.inc -- interface of the 'zlib' general purpose compression library
;  version 1.2.8, April 28th, 2013

;  Copyright (C) 1995-2013 Jean-loup Gailly and Mark Adler

;  This software is provided 'as-is', without any express or implied
;  warranty.  In no event will the authors be held liable for any damages
;  arising from the use of this software.

;  Permission is granted to anyone to use this software for any purpose,
;  including commercial applications, and to alter it and redistribute it
;  freely, subject to the following restrictions:

;  1. The origin of this software must not be misrepresented; you must not
;     claim that you wrote the original software. If you use this software
;     in a product, an acknowledgment in the product documentation would be
;     appreciated but is not required.
;  2. Altered source versions must be plainly marked as such, and must not be
;     misrepresented as being the original software.
;  3. This notice may not be removed or altered from any source distribution.

;  Jean-loup Gailly        Mark Adler
;  jloup@gzip.org          madler@alumni.caltech.edu


;  The data format used by the zlib library is described by RFCs (Request for
;  Comments) 1950 to 1952 in the files http://tools.ietf.org/html/rfc1950
;  (zlib format), rfc1951 (deflate format) and rfc1952 (gzip format).


include 'zconf.inc'

align 4
ZLIB_VERSION db '1.2.8',0
ZLIB_VERNUM equ 0x1280
ZLIB_VER_MAJOR equ 1
ZLIB_VER_MINOR equ 2
ZLIB_VER_REVISION equ 8
ZLIB_VER_SUBREVISION equ 0


;    The 'zlib' compression library provides in-memory compression and
;  decompression functions, including integrity checks of the uncompressed data.
;  This version of the library supports only one compression method (deflation)
;  but other algorithms will be added later and will have the same stream
;  interface.

;    Compression can be done in a single step if the buffers are large enough,
;  or can be done by repeated calls of the compression function.  In the latter
;  case, the application must provide more input and/or consume the output
;  (providing more output space) before each call.

;    The compressed data format used by default by the in-memory functions is
;  the zlib format, which is a zlib wrapper documented in RFC 1950, wrapped
;  around a deflate stream, which is itself documented in RFC 1951.

;    The library also supports reading and writing files in gzip (.gz) format
;  with an interface similar to that of stdio using the functions that start
;  with "gz".  The gzip format is different from the zlib format.  gzip is a
;  gzip wrapper, documented in RFC 1952, wrapped around a deflate stream.

;    This library can optionally read and write gzip streams in memory as well.

;    The zlib format was designed to be compact and fast for use in memory
;  and on communications channels.  The gzip format was designed for single-
;  file compression on file systems, has a larger header than zlib to maintain
;  directory information, and uses a different, slower check method than zlib.

;    The library does not install any signal handler.  The decoder checks
;  the consistency of the compressed data, so the library should never crash
;  even in case of corrupted input.

struct z_stream ;z_stream_s
	next_in dd ? ;z_const Bytef * ;next input byte
	avail_in dd ? ;uInt  ;number of bytes available at next_in
	total_in dd ? ;uLong ;total number of input bytes read so far

	next_out dd ? ;Bytef * ;next output byte should be put there
	avail_out dd ? ;uInt  ;remaining free space at next_out
	total_out dd ? ;uLong ;total number of bytes output so far

	msg dd ? ;z_const char * ;last error message, NULL if no error
	state dd ? ;deflate_state* ;not visible by applications

	zalloc dd ? ;alloc_func ;used to allocate the internal state
	zfree  dd ? ;free_func  ;used to free the internal state
	opaque dd ? ;voidpf     ;private data object passed to zalloc and zfree

	data_type dd ? ;int   ;best guess about the data type: binary or text
	adler     dd ? ;uLong ;adler32 value of the uncompressed data
	reserved  dd ? ;uLong ;reserved for future use
ends

;     gzip header information passed to and from zlib routines.  See RFC 1952
;  for more details on the meanings of these fields.

struct gz_header ;_s
	text   dd ? ;int    ;true if compressed data believed to be text
	time   dd ? ;uLong  ;modification time
	xflags dd ? ;int    ;extra flags (not used when writing a gzip file)
	os     dd ? ;int    ;operating system
	extra  dd ? ;Bytef* ;pointer to extra field or Z_NULL if none
	extra_len dd ? ;uInt  ;extra field length (valid if extra != Z_NULL)
	extra_max dd ? ;uInt  ;space at extra (only when reading header)
	name     dd ? ;Bytef* ;pointer to zero-terminated file name or Z_NULL
	name_max dd ? ;uInt   ;space at name (only when reading header)
	comment  dd ? ;Bytef* ;pointer to zero-terminated comment or Z_NULL
	comm_max dd ? ;uInt   ;space at comment (only when reading header)
	hcrc dd ? ;int ;true if there was or will be a header crc
	done dd ? ;int ;true when done reading gzip header (not used
		;when writing a gzip file)
ends


;     The application must update next_in and avail_in when avail_in has dropped
;   to zero.  It must update next_out and avail_out when avail_out has dropped
;   to zero.  The application must initialize zalloc, zfree and opaque before
;   calling the init function.  All other fields are set by the compression
;   library and must not be updated by the application.

;     The opaque value provided by the application will be passed as the first
;   parameter for calls of zalloc and zfree.  This can be useful for custom
;   memory management.  The compression library attaches no meaning to the
;   opaque value.

;     zalloc must return Z_NULL if there is not enough memory for the object.
;   If zlib is used in a multi-threaded application, zalloc and zfree must be
;   thread safe.

;     On 16-bit systems, the functions zalloc and zfree must be able to allocate
;   exactly 65536 bytes, but will not be required to allocate more than this if
;   the symbol MAXSEG_64K is defined (see zconf.h).  WARNING: On MSDOS, pointers
;   returned by zalloc for objects of exactly 65536 bytes *must* have their
;   offset normalized to zero.  The default allocation function provided by this
;   library ensures this (see zutil.c).  To reduce memory requirements and avoid
;   any allocation of 64K objects, at the expense of compression ratio, compile
;   the library with -DMAX_WBITS=14 (see zconf.h).

;     The fields total_in and total_out can be used for statistics or progress
;   reports.  After compression, total_in holds the total size of the
;   uncompressed data and may be saved for use in the decompressor (particularly
;   if the decompressor wants to decompress everything in a single step).


; constants

Z_NO_FLUSH      equ 0
Z_PARTIAL_FLUSH equ 1
Z_SYNC_FLUSH   equ 2
Z_FULL_FLUSH   equ 3
Z_FINISH       equ 4
Z_BLOCK        equ 5
Z_TREES        equ 6
; Allowed flush values; see deflate() and inflate() below for details

Z_OK           equ 0
Z_STREAM_END   equ 1
Z_NEED_DICT    equ 2
Z_ERRNO       equ (-1)
Z_STREAM_ERROR equ (-2)
Z_DATA_ERROR  equ (-3)
Z_MEM_ERROR   equ (-4)
Z_BUF_ERROR   equ (-5)
Z_VERSION_ERROR equ (-6)
; Return codes for the compression/decompression functions. Negative values
; are errors, positive values are used for special but normal events.


Z_NO_COMPRESSION        equ 0
Z_BEST_SPEED            equ 1
Z_BEST_COMPRESSION      equ 9
Z_DEFAULT_COMPRESSION equ (-1)
; compression levels

Z_FILTERED           equ 1
Z_HUFFMAN_ONLY       equ 2
Z_RLE                equ 3
Z_FIXED              equ 4
Z_DEFAULT_STRATEGY   equ 0
; compression strategy; see deflateInit2() below for details

Z_BINARY  equ 0
Z_TEXT    equ 1
Z_ASCII   equ Z_TEXT ;for compatibility with 1.2.2 and earlier
Z_UNKNOWN equ 2
; Possible values of the data_type field (though see inflate())

Z_DEFLATED  equ 8
; The deflate compression method (the only one supported in this version)

Z_NULL equ 0  ;for initializing zalloc, zfree, opaque

zlib_version equ zlibVersion
; for compatibility with versions < 1.0.2

; various hacks, don't look :)

; deflateInit and inflateInit are macros to allow checking the zlib version
; and the compiler's view of z_stream:

;int inflateBackInit_ OF((z_streamp strm, int windowBits,
;                                         unsigned char FAR *window,
;                                         const char *version,
;                                         int stream_size));
;#define inflateInit(strm) \
;        inflateInit_((strm), ZLIB_VERSION, (int)sizeof(z_stream))

;#define inflateInit2(strm, windowBits) \
;        inflateInit2_((strm), (windowBits), ZLIB_VERSION, \
;                      (int)sizeof(z_stream))
;#define inflateBackInit(strm, windowBits, window) \
;        inflateBackInit_((strm), (windowBits), (window), \
;                      ZLIB_VERSION, (int)sizeof(z_stream))

if Z_SOLO eq 0

; gzgetc() macro and its supporting function and exposed data structure.  Note
; that the real internal state is much larger than the exposed structure.
; This abbreviated structure exposes just enough for the gzgetc() macro.  The
; user should not mess with these exposed elements, since their names or
; behavior could change in the future, perhaps even capriciously.  They can
; only be used by the gzgetc() macro.  You have been warned.

;struct gzFile_s {
;    unsigned have;
;    unsigned char *next;
;    z_off64_t pos;
;};

;#  define gzgetc(g) \
;          ((g)->have ? ((g)->have--, (g)->pos++, *((g)->next)++) : gzgetc(g))

end if

