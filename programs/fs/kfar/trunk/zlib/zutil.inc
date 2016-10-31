; zutil.inc -- internal interface and configuration of the compression library
; Copyright (C) 1995-2013 Jean-loup Gailly.
; For conditions of distribution and use, see copyright notice in zlib.inc

; WARNING: this file should *not* be used by applications. It is
; part of the implementation of the compression library and is
; subject to change. Applications should only use zlib.inc.


macro ERR_MSG err
{
	mov ecx,Z_NEED_DICT
	sub ecx,err
	mov ecx,[4*ecx+z_errmsg]
}

macro ERR_RETURN strm,err
{
	ERR_MSG err
	mov [strm+z_stream.msg],ecx
	mov eax,err
}
; To be used only when the state is known to be valid

;        /* common constants */

;#ifndef DEF_WBITS
;#  define DEF_WBITS MAX_WBITS
;end if
; default windowBits for decompression. MAX_WBITS is for compression only

;#if MAX_MEM_LEVEL >= 8
DEF_MEM_LEVEL equ 8
;#else
;#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
;end if
; default memLevel

STORED_BLOCK equ 0
STATIC_TREES equ 1
DYN_TREES   equ 2
; The three kinds of block type

MIN_MATCH equ 3
MAX_MATCH equ 258
; The minimum and maximum match lengths

PRESET_DICT equ 0x20 ;preset dictionary flag in zlib header

;        /* common defaults */

OS_CODE equ 0x03 ;assume Unix

;         /* functions */

; Diagnostic functions
;if DEBUG eq 1
;#  define Trace(x) {if (z_verbose>=0) fprintf x ;}
;#  define Tracev(x) {if (z_verbose>0) fprintf x ;}
macro Tracevv mes1, mes2
{
	;zlib_debug 'Tracevv = %d', mes1
}
;#  define Tracec(c,x) {if (z_verbose>0 && (c)) fprintf x ;}
;#  define Tracecv(c,x) {if (z_verbose>1 && (c)) fprintf x ;}
;end if

macro ZALLOC strm, items, size
{
	stdcall dword[strm+z_stream.zalloc], [strm+z_stream.opaque], items, size
}
macro ZFREE strm, p2addr
{
	stdcall dword[strm+z_stream.zfree], dword[strm+z_stream.opaque], p2addr
}
macro TRY_FREE s, p
{
local .end0
	cmp p,0
	je .end0
	ZFREE s, p
	.end0:
}

; Reverse the bytes in a 32-bit value
macro ZSWAP32 q
{
	bswap q
}
