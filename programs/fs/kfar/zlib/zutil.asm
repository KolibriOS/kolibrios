; zutil.asm -- target dependent utility functions for the compression library
; Copyright (C) 1995-2005, 2010, 2011, 2012 Jean-loup Gailly.
; For conditions of distribution and use, see copyright notice in zlib.inc

align 4
z_errmsg dd ze0,ze1,ze2,ze3,ze4,ze5,ze6,ze7,ze8,ze9
ze0 db 'need dictionary',0 ;Z_NEED_DICT       2
ze1 db 'stream end',0      ;Z_STREAM_END      1
ze2 db '',0                ;Z_OK              0
ze3 db 'file error',0      ;Z_ERRNO         (-1)
ze4 db 'stream error',0    ;Z_STREAM_ERROR  (-2)
ze5 db 'data error',0      ;Z_DATA_ERROR    (-3)
ze6 db 'insufficient memory',0  ;Z_MEM_ERROR     (-4)
ze7 db 'buffer error',0         ;Z_BUF_ERROR     (-5)
ze8 db 'incompatible version',0 ;Z_VERSION_ERROR (-6)
ze9 db '',0

;const char * ()
align 4
proc zlibVersion
	mov eax,ZLIB_VERSION;
	ret
endp

;uLong ()
align 4
proc zlibCompileFlags
;    uLong flags;

;    flags = 0;
;    switch ((int)(sizeof(uInt))) {
;    case 2:     break;
;    case 4:     flags += 1;     break;
;    case 8:     flags += 2;     break;
;    default:    flags += 3;
;    }
;    switch ((int)(sizeof(uLong))) {
;    case 2:     break;
;    case 4:     flags += 1 << 2;        break;
;    case 8:     flags += 2 << 2;        break;
;    default:    flags += 3 << 2;
;    }
;    switch ((int)(sizeof(voidpf))) {
;    case 2:     break;
;    case 4:     flags += 1 << 4;        break;
;    case 8:     flags += 2 << 4;        break;
;    default:    flags += 3 << 4;
;    }
;    switch ((int)(sizeof(z_off_t))) {
;    case 2:     break;
;    case 4:     flags += 1 << 6;        break;
;    case 8:     flags += 2 << 6;        break;
;    default:    flags += 3 << 6;
;    }
;if DEBUG
;    flags += 1 << 8;
;end if
;#if defined(ASMV) || defined(ASMINF)
;    flags += 1 << 9;
;end if
if ZLIB_WINAPI eq 1
;    flags += 1 << 10;
end if
if BUILDFIXED eq 1
;    flags += 1 << 12;
end if
if DYNAMIC_CRC_TABLE eq 1
;    flags += 1 << 13;
end if
if NO_GZCOMPRESS eq 1
;    flags += 1L << 16;
end if
if NO_GZIP eq 1
;    flags += 1L << 17;
end if
if PKZIP_BUG_WORKAROUND eq 1
;    flags += 1L << 20;
end if
if FASTEST eq 1
;    flags += 1L << 21;
end if
;#if defined(STDC) || defined(Z_HAVE_STDARG_H)
;#  ifdef NO_vsnprintf
;    flags += 1L << 25;
;#    ifdef HAS_vsprintf_void
;    flags += 1L << 26;
;#    endif
;#  else
;#    ifdef HAS_vsnprintf_void
;    flags += 1L << 26;
;#    endif
;#  endif
;#else
;    flags += 1L << 24;
;#  ifdef NO_snprintf
;    flags += 1L << 25;
;#    ifdef HAS_sprintf_void
;    flags += 1L << 26;
;#    endif
;#  else
;#    ifdef HAS_snprintf_void
;    flags += 1L << 26;
;#    endif
;#  endif
;end if
;    return flags;
	ret
endp

;void (char *m)
align 4
proc z_error, m:dword
;    fprintf(stderr, "%s\n", m);
;    exit(1);
	ret
endp

; exported to allow conversion of error code to string for compress() and
; uncompress()

;const char * (int err)
align 4
proc zError uses ecx, err:dword
	ERR_MSG [err]
	mov eax,ecx
	ret
endp

;#ifndef HAVE_MEMCPY

;void (dest, source, len)
;    Bytef* dest
;    const Bytef* source
;    uInt  len
align 4
proc zmemcpy uses ecx edi esi, dest:dword, source:dword, len:dword
	mov ecx,[len]
	test ecx,ecx
	jz .end0
		mov edi,[dest]
		mov esi,[source]
		bt ecx,0 ;кратно 2 ?
		jnc @f
			rep movsb
			jmp .end0
		@@:
		bt ecx,1 ;кратно 4 ?
		jnc @f
			shr ecx,1
			rep movsw
			jmp .end0
		@@:
		shr ecx,2
		rep movsd
	.end0:
	ret
endp

;int (s1, s2, len)
;    const Bytef* s1
;    const Bytef* s2
;    uInt  len
align 4
proc zmemcmp, s1:dword, s2:dword, len:dword
;    uInt j;

;    for (j = 0; j < len; j++) {
;        if (s1[j] != s2[j]) return 2*(s1[j] > s2[j])-1;
;    }
;    return 0;
	ret
endp

;void (Bytef* dest, uInt len)
align 4
proc zmemzero uses eax ecx edi, dest:dword, len:dword
	mov ecx,[len]
	test ecx,ecx
	jz .end0
		xor eax,eax
		mov edi,[dest]
		bt ecx,0 ;кратно 2 ?
		jnc @f
			rep stosb
			jmp .end0
		@@:
		bt ecx,1 ;кратно 4 ?
		jnc @f
			shr ecx,1
			rep stosw
			jmp .end0
		@@:
		shr ecx,2
		rep stosd
	.end0:
	ret
endp
;end if

;voidpf (voidpf opaque, unsigned items, unsigned size)
align 4
proc zcalloc uses ebx ecx, opaque:dword, items:dword, size:dword
	mov ecx,[size]
	imul ecx,[items]
	mcall SF_SYS_MISC, SSF_MEM_ALLOC
	ret
endp

;void (voidpf opaque, voidpf ptr)
align 4
proc zcfree uses eax ebx ecx, opaque:dword, p2ptr:dword
	mcall SF_SYS_MISC, SSF_MEM_FREE, [p2ptr]
	ret
endp

