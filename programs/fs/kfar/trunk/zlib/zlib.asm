
include '../../../../proc32.inc'
include '../../../../macros.inc'
include '../../../../KOSfuncs.inc'

FASTEST equ 0
GEN_TREES_H equ 0
DEBUG equ 0
DYNAMIC_CRC_TABLE equ 1
Z_SOLO equ 0

; define NO_GZIP when compiling if you want to disable gzip header and
; trailer creation by deflate().  NO_GZIP would be used to avoid linking in
; the crc code when it is not needed.  For shared libraries, gzip encoding
; should be left enabled.
GZIP equ 1

macro zlib_debug fmt,p1
{
if DEBUG eq 1
	zlib_assert fmt,p1
end if
}

macro zlib_assert fmt,p1
{
	local .end_t
	local .m_fmt
pushf
	jmp .end_t
	.m_fmt db fmt,13,10,0
align 4
	.end_t:
if p1 eq
	stdcall dbg_print,0,.m_fmt
else
	stdcall str_format_dbg, buf_param,.m_fmt,p1
end if
popf
}

include 'zlib.inc'
include 'deflate.inc'
include 'zutil.asm'
;include '../kfar_arc/crc.inc'
include 'crc32.asm'
include 'adler32.asm'
include 'trees.asm'
include 'deflate.asm'

align 4
buf_param rb 80

align 4
proc dbg_print, fun:dword, mes:dword
pushad
	mov eax,SF_BOARD
	mov ebx,SSF_DEBUG_WRITE

	mov esi,[fun]
	cmp esi,0
	je .end0
	@@:
		mov cl,byte[esi]
		int 0x40
		inc esi
		cmp byte[esi],0
		jne @b
	mov cl,':'
	int 0x40
	mov cl,' '
	int 0x40
	.end0:
	mov esi,[mes]
	cmp esi,0
	je .end_f
	@@:
		mov cl,byte[esi]
		cmp cl,0
		je .end_f
		int 0x40
		inc esi
		jmp @b
	.end_f:
popad
	ret
endp

align 4
proc str_format_dbg, buf:dword, fmt:dword, p1:dword
pushad
	mov esi,[fmt]
	mov edi,[buf]
	mov ecx,80-1
	.cycle0:
		lodsb
		cmp al,'%'
		jne .no_param
			lodsb
			dec ecx
			cmp al,0
			je .cycle0end
			cmp al,'d'
			je @f
			cmp al,'u'
			je @f
			cmp al,'l'
			je .end1
				jmp .end0
			.end1: ;%lu %lx
				lodsb
				dec ecx
				cmp al,'u'
				jne .end0
			@@:
				mov eax,[p1]
				stdcall convert_int_to_str,ecx
				xor al,al
				repne scasb
				dec edi
			.end0:
			loop .cycle0
		.no_param:
		stosb
		cmp al,0
		je .cycle0end
		loop .cycle0
	.cycle0end:
	xor al,al
	stosb
	stdcall dbg_print,0,[buf]
popad
	ret
endp

;input:
; eax - число
; edi - буфер для строки
; len - длинна буфера
;output:
align 4
proc convert_int_to_str, len:dword
pushad
	mov esi,[len]
	add esi,edi
	dec esi
	call .str
popad
	ret
endp

align 4
.str:
	mov ecx,0x0a
	cmp eax,ecx
	jb @f
		xor edx,edx
		div ecx
		push edx
		call .str
		pop eax
	@@:
	cmp edi,esi
	jge @f
		or al,0x30
		stosb
		mov byte[edi],0
	@@:
	ret

