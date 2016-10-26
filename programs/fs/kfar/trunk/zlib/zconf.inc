; zconf.inc -- configuration of the zlib compression library
; Copyright (C) 1995-2013 Jean-loup Gailly.
; For conditions of distribution and use, see copyright notice in zlib.inc

; Compile with -DMAXSEG_64K if the alloc function cannot allocate more
; than 64k bytes at a time (needed on systems with 16-bit int).

;if MSDOS
;#  define UNALIGNED_OK
;end if

; Maximum value for memLevel in deflateInit2
MAX_MEM_LEVEL equ 9

; Maximum value for windowBits in deflateInit2 and inflateInit2.
; WARNING: reducing MAX_WBITS makes minigzip unable to extract .gz files
; created by gzip. (Files created by minigzip can still be extracted by
; gzip.)

MAX_WBITS  equ 15 ;32K LZ77 window

; The memory requirements for deflate are (in bytes):
;            (1 << (windowBits+2)) +  (1 << (memLevel+9))
; that is: 128K for windowBits=15  +  128K for memLevel = 8  (default values)
; plus a few kilobytes for small objects. For example, if you want to reduce
; the default memory requirements from 256K to 128K, compile with
;     make CFLAGS="-O -DMAX_WBITS=14 -DMAX_MEM_LEVEL=7"
; Of course this will generally degrade compression (there's no free lunch).

;   The memory requirements for inflate are (in bytes) 1 << windowBits
; that is, 32K for windowBits=15 (default value) plus a few kilobytes
; for small objects.

;                        /* Type declarations */

;#ifndef OF /* function prototypes */
;#  ifdef STDC
;#    define OF(args)  args
;#  else
;#    define OF(args)  ()
;#  endif
;end if

;#ifndef Z_ARG /* function prototypes for stdarg */
;#  if defined(STDC) || defined(Z_HAVE_STDARG_H)
;#    define Z_ARG(args)  args
;#  else
;#    define Z_ARG(args)  ()
;#  endif
;end if

; The following definitions for FAR are needed only for MSDOS mixed
; model programming (small or medium model with some far allocations).
; This was tested only with MSC; for other MSDOS compilers you may have
; to define NO_MEMCPY in zutil.h.  If you don't need the mixed model,
; just define FAR to be empty.

;#if defined(WINDOWS) || defined(WIN32)
	; If building or using zlib as a DLL, define ZLIB_DLL.
	; This is not mandatory, but it offers a little performance increase.

;#  ifdef ZLIB_DLL
;#    if defined(WIN32) && (!defined(__BORLANDC__) || (__BORLANDC__ >= 0x500))
;#      ifdef ZLIB_INTERNAL
;#        define ZEXTERN extern __declspec(dllexport)
;#      else
;#        define ZEXTERN extern __declspec(dllimport)
;#      endif
;#    endif
;#  endif  /* ZLIB_DLL */
	; If building or using zlib with the WINAPI/WINAPIV calling convention,
	; define ZLIB_WINAPI.
	; Caution: the standard ZLIB1.DLL is NOT compiled using ZLIB_WINAPI.

;#if !defined(Z_U4) && !defined(Z_SOLO) && defined(STDC)
;#  include <limits.h>
;#  if (UINT_MAX == 0xffffffffUL)
;#    define Z_U4 unsigned
;#  elif (ULONG_MAX == 0xffffffffUL)
;#    define Z_U4 unsigned long
;#  elif (USHRT_MAX == 0xffffffffUL)
;#    define Z_U4 unsigned short
;#  endif
;end if

;if Z_U4
;   typedef Z_U4 z_crc_t;
;else
;   typedef unsigned long z_crc_t;
;end if

;if HAVE_UNISTD_H    /* may be set to #if 1 by ./configure */
;#  define Z_HAVE_UNISTD_H
;end if

;if HAVE_STDARG_H    /* may be set to #if 1 by ./configure */
;#  define Z_HAVE_STDARG_H
;end if

;if STDC
;#  ifndef Z_SOLO
;#    include <sys/types.h>      /* for off_t */
;#  endif
;end if

;#if defined(STDC) || defined(Z_HAVE_STDARG_H)
;#  ifndef Z_SOLO
;#    include <stdarg.h>         /* for va_list */
;#  endif
;end if

; a little trick to accommodate both "#define _LARGEFILE64_SOURCE" and
; "#define _LARGEFILE64_SOURCE 1" as requesting 64-bit operations, (even
; though the former does not conform to the LFS document), but considering
; both "#undef _LARGEFILE64_SOURCE" and "#define _LARGEFILE64_SOURCE 0" as
; equivalently requesting no 64-bit operations

;#if defined(_LARGEFILE64_SOURCE) && -_LARGEFILE64_SOURCE - -1 == 1
;#  undef _LARGEFILE64_SOURCE
;end if

;#if defined(__WATCOMC__) && !defined(Z_HAVE_UNISTD_H)
;#  define Z_HAVE_UNISTD_H
;end if
;#ifndef Z_SOLO
;#  if defined(Z_HAVE_UNISTD_H) || defined(_LARGEFILE64_SOURCE)
;#    include <unistd.h>         /* for SEEK_*, off_t, and _LFS64_LARGEFILE */
;#    ifdef VMS
;#      include <unixio.h>       /* for off_t */
;#    endif
;#    ifndef z_off_t
;#      define z_off_t off_t
;#    endif
;#  endif
;end if

;#if defined(_LFS64_LARGEFILE) && _LFS64_LARGEFILE-0
;#  define Z_LFS64
;end if

;#if defined(_LARGEFILE64_SOURCE) && defined(Z_LFS64)
;#  define Z_LARGE64
;end if

;#if defined(_FILE_OFFSET_BITS) && _FILE_OFFSET_BITS-0 == 64 && defined(Z_LFS64)
;#  define Z_WANT64
;end if

;#if !defined(SEEK_SET) && !defined(Z_SOLO)
;#  define SEEK_SET        0       /* Seek from beginning of file.  */
;#  define SEEK_CUR        1       /* Seek from current position.  */
;#  define SEEK_END        2       /* Set file pointer to EOF plus "offset" */
;end if

;#  define z_off_t long

;#if !defined(_WIN32) && defined(Z_LARGE64)
;#  define z_off64_t off64_t
;else
;#  if defined(_WIN32) && !defined(__GNUC__) && !defined(Z_SOLO)
;#    define z_off64_t __int64
;#  else
;#    define z_off64_t z_off_t
;#  endif
;end if
