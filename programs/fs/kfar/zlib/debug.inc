
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

;for debug
tz1 db 'next_in',13,10,0
tz2 db 'avail_in',13,10,0
tz3 db 'total_in',13,10,0
tz4 db 'next_out',13,10,0
tz5 db 'avail_out',13,10,0
tz6 db 'total_out',13,10,0
tz7 db 'msg',13,10,0
tz8 db 'state',13,10,0
tz9 db 'zalloc',13,10,0
tz10 db 'zfree',13,10,0
tz11 db 'opaque',13,10,0
tz12 db 'data_type',13,10,0
tz13 db 'adler',13,10,0
tz14 db 'reserved',13,10,0

sv_2:
dd z_stream.next_in,4,tz1
dd z_stream.avail_in,4,tz2
dd z_stream.total_in,4,tz3
dd z_stream.next_out,4,tz4
dd z_stream.avail_out,4,tz5
dd z_stream.total_out,4,tz6
dd z_stream.msg,4,tz7
dd z_stream.state,4,tz8
dd z_stream.zalloc,4,tz9
dd z_stream.zfree,4,tz10
dd z_stream.opaque,4,tz11
dd z_stream.data_type,4,tz12
dd z_stream.adler,4,tz13
dd z_stream.reserved,4,tz14
dd 0,0

ta1 db 'strm',13,10,0
ta2 db 'status',13,10,0
ta3 db 'pending_buf',13,10,0
ta4 db 'pending_buf_size',13,10,0
ta5 db 'pending_out',13,10,0
ta6 db 'pending',13,10,0
ta7 db 'wrap',13,10,0
ta8 db 'gzhead',13,10,0
ta9 db 'gzindex',13,10,0
ta10 db 'method',13,10,0
ta11 db 'last_flush',13,10,0
ta12 db 'w_size',13,10,0
ta13 db 'w_bits',13,10,0
ta14 db 'w_mask',13,10,0
ta15 db 'window',13,10,0
ta16 db 'window_size',13,10,0
ta17 db 'prev',13,10,0
ta18 db 'head',13,10,0
ta19 db 'ins_h',13,10,0
ta20 db 'hash_size',13,10,0
ta21 db 'hash_bits',13,10,0
ta22 db 'hash_mask',13,10,0
ta23 db 'hash_shift',13,10,0
ta24 db 'block_start',13,10,0
ta25 db 'match_length',13,10,0
ta26 db 'prev_match',13,10,0
ta27 db 'match_available',13,10,0
ta28 db 'strstart',13,10,0
ta29 db 'match_start',13,10,0
ta30 db 'lookahead',13,10,0
ta31 db 'prev_length',13,10,0
ta32 db 'max_chain_length',13,10,0
ta33 db 'max_lazy_match',13,10,0
ta34 db 'level',13,10,0
ta35 db 'strategy',13,10,0
ta36 db 'good_match',13,10,0
ta37 db 'nice_match',13,10,0
ta38 db 'dyn_ltree',13,10,0
ta39 db 'dyn_dtree',13,10,0
ta40 db 'bl_tree',13,10,0
ta41 db 'l_desc',13,10,0
ta42 db 'd_desc',13,10,0
ta43 db 'bl_desc',13,10,0
ta44 db 'bl_count',13,10,0
ta45 db 'heap',13,10,0
ta46 db 'heap_len',13,10,0
ta47 db 'heap_max',13,10,0
ta48 db 'depth',13,10,0
ta49 db 'l_buf',13,10,0
ta50 db 'lit_bufsize',13,10,0
ta51 db 'last_lit',13,10,0
ta52 db 'd_buf',13,10,0
ta53 db 'opt_len',13,10,0
ta54 db 'static_len',13,10,0
ta55 db 'matches',13,10,0
ta56 db 'insert',13,10,0
; db 'compressed_len',13,10,0
; db 'bits_sent',13,10,0
ta59 db 'bi_buf',13,10,0
ta60 db 'bi_valid',13,10,0
ta61 db 'high_water',13,10,0

sv_3:
dd deflate_state.strm,4,ta1
dd deflate_state.status,4,ta2
dd deflate_state.pending_buf,4,ta3
dd deflate_state.pending_buf_size,4,ta4
dd deflate_state.pending_out,4,ta5
dd deflate_state.pending,4,ta6
dd deflate_state.wrap,4,ta7
dd deflate_state.gzhead,4,ta8
dd deflate_state.gzindex,4,ta9
dd deflate_state.method,1,ta10
dd deflate_state.last_flush,4,ta11
dd deflate_state.w_size,4,ta12
dd deflate_state.w_bits,4,ta13
dd deflate_state.w_mask,4,ta14
dd deflate_state.window,4,ta15
dd deflate_state.window_size,4,ta16
dd deflate_state.prev,4,ta17
dd deflate_state.head,4,ta18
dd deflate_state.ins_h,4,ta19
dd deflate_state.hash_size,4,ta20
dd deflate_state.hash_bits,4,ta21
dd deflate_state.hash_mask,4,ta22
dd deflate_state.hash_shift,4,ta23
dd deflate_state.block_start,4,ta24
dd deflate_state.match_length,4,ta25
dd deflate_state.prev_match,4,ta26
dd deflate_state.match_available,4,ta27
dd deflate_state.strstart,4,ta28
dd deflate_state.match_start,4,ta29
dd deflate_state.lookahead,4,ta30
dd deflate_state.prev_length,4,ta31
dd deflate_state.max_chain_length,4,ta32
dd deflate_state.max_lazy_match,4,ta33
dd deflate_state.level,2,ta34
dd deflate_state.strategy,2,ta35
dd deflate_state.good_match,4,ta36
dd deflate_state.nice_match,4,ta37
dd deflate_state.dyn_ltree,((2*HEAP_SIZE) shl 16)+2,ta38
dd deflate_state.dyn_dtree,((2*(2*D_CODES+1)) shl 16)+2,ta39
dd deflate_state.bl_tree,((2*(2*BL_CODES+1)) shl 16)+2,ta40
dd deflate_state.l_desc,(3 shl 16)+4,ta41
dd deflate_state.d_desc,(3 shl 16)+4,ta42
dd deflate_state.bl_desc,(3 shl 16)+4,ta43
dd deflate_state.bl_count,((MAX_BITS+1) shl 16)+2,ta44
dd deflate_state.heap,((2*L_CODES+1) shl 16)+2,ta45
dd deflate_state.heap_len,4,ta46
dd deflate_state.heap_max,4,ta47
dd deflate_state.depth,((2*L_CODES+1) shl 16)+1,ta48
dd deflate_state.l_buf,4,ta49
dd deflate_state.lit_bufsize,4,ta50
dd deflate_state.last_lit,4,ta51
dd deflate_state.d_buf,4,ta52
dd deflate_state.opt_len,4,ta53
dd deflate_state.static_len,4,ta54
dd deflate_state.matches,4,ta55
dd deflate_state.insert,4,ta56
;if DEBUG eq 1
;dd deflate_state.compressed_len
;dd deflate_state.bits_sent
;end if
dd deflate_state.bi_buf,2,ta59
dd deflate_state.bi_valid,4,ta60
dd deflate_state.high_water,4,ta61
dd 0,0

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

align 4
proc debug_fields, saddr:dword, form:dword
locals
	nl_array dd ?
endl
pushad
	mov edi,[saddr]
	cmp edi,0
	je .end_f
	mcall SF_BOARD,SSF_DEBUG_WRITE,13
	mcall ,,10
	mov eax,[form]
align 4
	.cycle0:
		mov ebx,[eax+4]
		mov ecx,ebx
		and ebx,0xffff
		cmp ebx,0
		je .end_f
		mov esi,ebx
		shl ebx,1
		shr ecx,16
		cmp ecx,0
		je .end0
			;if array
			stdcall dbg_print,0,[eax+8]
			mov edx,61 ;size text line
			mov dword[nl_array],0
			.cycle2:
				inc dword[nl_array]
				sub edx,ebx
				sub edx,2 ;': '
				cmp edx,3
				jg .cycle2
			mov edx,edi
			add edx,[eax]
			push eax
			.nl_i:
			mov eax,[nl_array]
			mov byte[ebx+txt_buf],0 ;конец числа
			.cycle1:
				stdcall hex_in_str,txt_buf,[edx],ebx
				add edx,esi ;move next value
				
				push edi
				mov edi,txt_buf
				cmp byte[edi],'0'
				jne @f
					inc edi
				cmp byte[edi],'0'
				jne @f
					inc edi
				cmp byte[edi],'0'
				jne @f
					inc edi
				cmp byte[edi],'0'
				jne @f
					inc edi
				@@:
				cmp byte[edi],0
				jne @f
					dec edi
				@@:
				stdcall dbg_print,edi,0
				pop edi
				
				;stdcall dbg_print,txt_buf,0
				dec eax
				jz .nl
				loop .cycle1
			.nl:
			push ebx ecx
			mcall SF_BOARD,SSF_DEBUG_WRITE,13
			mcall ,,10
			pop ecx ebx
			dec ecx
			cmp ecx,0
			jg .nl_i
			pop eax
			add eax,12
			jmp .cycle0
		.end0:
		mov edx,edi
		add edx,[eax]
		stdcall hex_in_str,txt_buf,[edx],ebx
		mov byte[ebx+txt_buf],0 ;конец числа
		stdcall dbg_print,txt_buf,[eax+8]
		add eax,12
		jmp .cycle0
.end_f:
	mcall SF_BOARD,SSF_DEBUG_WRITE,13
	mcall ,,10
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

