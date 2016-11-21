
txt_zv db '*',0
txt_sp db ' ',0
txt_buf db '1234',0
	rd 1

buf_param rb 80

macro cStr dest,txt
{
local .end_t
local .m_txt
jmp .end_t
align 4
	.m_txt db txt,0
align 4
.end_t:
if dest eq
	mov eax,.m_txt
else
	mov dest,.m_txt
end if
}

align 4
proc f_png_warning, h:dword, m_txt:dword
	stdcall hex_in_str,txt_buf,[h],5
	mov byte[txt_buf+5],0
	stdcall dbg_print,txt_buf,[m_txt]
	ret
endp

align 4
proc f_png_error, h:dword, m_txt:dword
	stdcall hex_in_str,txt_buf,[h],5
	mov byte[txt_buf+5],0
	stdcall dbg_print,txt_buf,[m_txt]
	ret
endp

align 4
proc f_png_debug, n:dword, m_txt:dword
	stdcall dbg_print,txt_sp,[m_txt]
	ret
endp

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

;input:
; zif - 1...8
align 4
proc hex_in_str, buf:dword,val:dword,zif:dword
pushad
	mov edi,dword[buf]
	mov ecx,dword[zif]
	add edi,ecx
	dec edi
	mov ebx,dword[val]

	.cycle:
		mov al,bl
		and al,0xf
		cmp al,10
		jl @f
			add al,'a'-'0'-10
		@@:
		add al,'0'
		mov byte[edi],al
		dec edi
		shr ebx,4
	loop .cycle
popad
	ret
endp

;---

macro png_warning h,txt
{
if txt eqtype ''
	local .end_t
	local .m_txt
	jmp .end_t
	.m_txt db txt,13,10,0
	.end_t:
	stdcall f_png_warning,h,.m_txt
else
	stdcall f_png_warning,h,txt
	push eax ebx ecx
	mcall SF_BOARD,SSF_DEBUG_WRITE,13
	mcall ,,10
	pop ecx ebx eax
end if
}

macro png_app_warning h,txt
{
	png_warning h,<txt>
}

macro png_error h,txt
{
if txt eqtype ''
	local .end_t
	local .m_txt
	jmp .end_t
	.m_txt db txt,13,10,0
	.end_t:
	stdcall f_png_error,h,.m_txt
else
	stdcall f_png_error,h,txt
	push eax ebx ecx
	mcall SF_BOARD,SSF_DEBUG_WRITE,13
	mcall ,,10
	pop ecx ebx eax
end if
}

macro png_debug n,txt
{
if DEBUG eq 1
local .end_t
local .m_txt
jmp .end_t
	.m_txt db txt,13,10,0
align 4
.end_t:
stdcall f_png_debug,n,.m_txt
end if
}

macro png_debug1 n,fmt,p1
{
if DEBUG eq 1
local .end_t

if p1 eqtype ''
local .m_txt1
local .m_txt2
jmp .end_t
	.m_txt1 db fmt,0
	.m_txt2 db p1,13,10,0
align 4
.end_t:
stdcall dbg_print,.m_txt1,.m_txt2

else
local .m_fmt
jmp .end_t
	.m_fmt db fmt,13,10,0
align 4
.end_t:
	stdcall str_format_dbg, buf_param,.m_fmt,p1
	
end if
end if
}

;output:
; eax = strlen
align 4
proc strlen, str1:dword
	mov eax,[str1]
	@@:
		cmp byte[eax],0
		je @f
		inc eax
		jmp @b
	@@:
	sub eax,[str1]
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
	stdcall dbg_print,txt_sp,[buf]
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

macro std_png_image_error n,txt
{
local .end_t
local .m_txt
jmp .end_t
	.m_txt db txt,13,10,0
align 4
.end_t:
stdcall png_image_error,n,.m_txt
}

