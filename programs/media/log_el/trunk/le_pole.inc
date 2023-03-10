;
; ?㭪?? ??? ᮧ????? ? ।????஢???? ?஢????
;

;--------------------------------------
struct Cell
	x dd ? ;+0
	y dd ? ;+4
	liv db ? ;+8
	napr db ? ;+9
ends

offs_cell_x equ 0
offs_cell_y equ 4
offs_cell_liv equ 8
offs_cell_napr equ 9

;????????? ??? ᮧ????? ????
align 4
pole:
.index dd 0
cell dd 0 ;㪠??⥫? ?? ?????? ?? ???????ࠬ? ?祥?
.max_cell dd 90000
.b_sort dd 0 ;?࠭??? ??? ?????஢????? ?祥?

pole_index    equ dword[edi]
pole_data     equ dword[edi +4] ;㪠??⥫? ?? ?????? ?? ???????ࠬ? ?祥?
pole_max_cell equ dword[edi +8]
pole_b_sort   equ dword[edi+12] ;?࠭??? ??? ?????஢????? ?祥?
offs_pole_b_sort equ 12

macro get_cell_offset reg,ind
{
	mov reg,ind
	imul reg,sizeof.Cell
	add reg,dword[cell]
}

er_oom db 0 ;?? ???砩 ????௠??? ??????
Cor_x dd 0
Cor_y dd 0
zoom db 3 ;????⠡ ????
txt_zoom db '????⠡:',0
txt_osob db '??祪:',0
txt_info: db '??????: '
.size: rb 16
txt_mull db '*',0
txt_space db ' ',0
txt_nl db 13,10,0
txt_buf rb 32


align 4
proc pole_init uses eax ebx edi, pole:dword
	mov edi,dword[pole]

	;*** ??? ? ????? ???????? ? ?????? ***
	mov ebx,4
	add ebx,sizeof.Cell
	imul ebx,pole_max_cell
	stdcall mem.Alloc,ebx
	mov pole_index,eax

	mov ebx,pole_max_cell
	shl ebx,2
	add eax,ebx
	mov pole_data,eax

	stdcall pole_clear, edi
	stdcall pole_paint, edi ;??ᮢ???? ???? ? ?????? (?? ?? ??࠭?)
	ret
endp

align 4
proc pole_delete uses edi, pole:dword
	mov edi,dword[pole]
	stdcall mem.Free,pole_index
	ret
endp

;???⪠ ?஢???? ?? ?奬?
align 4
proc pole_clear uses eax ecx edi, pole:dword
	mov edi,dword[pole]

	xor eax,eax
	mov pole_b_sort,eax
	mov byte[er_oom],al
	cld
	mov ecx,pole_max_cell
	imul ecx,sizeof.Cell
	mov edi,pole_data
	repne stosb ;memset(cell,0,sizeof(Cell)*pole_max_cell);

	mov edi,dword[pole]
	mov ecx,pole_max_cell
	mov edi,pole_index
	@@:
		stosd ;for(i=0;i<pole_max_cell;i++) pole_index[i]=i;
		inc eax
		loop @b
	ret
endp

align 4
proc pole_cell_creat, pole:dword, x:dword, y:dword, li:dword
	pushad
	mov edi,dword[pole]
	mov esi,pole_index

	; *** ?᫨ ???⪠ 㦥 ?뫠 ᮧ????
	stdcall pole_cell_find, [pole], [x],[y]
	cmp eax,0
	je @f
		get_cell_offset ebx,eax
		jmp .change
	@@:

	; *** ᮧ????? ????? ?祩??
	; ??室?? ????? ᢮?????? ?祩?? (i) ??? ?????????? ?????
	;mov esi,pole_index
	inc dword[esi]
	mov ebx,pole_max_cell
	cmp dword[esi],ebx
	jne @f
		dec dword[esi]
		;... need call message: "eror out of memory" ...
		;... ?뢮? ᮮ?饭?? ??९??????? ???? ????????  ...
		mov byte[er_oom],0
		jmp .fun_e ;return;
	@@:
	mov eax,dword[esi] ;eax - ????? ??? ??᫥???? ?祩??
	shl eax,2
	add eax,pole_index ;eax - 㪠??⥫? ?? ??????塞?? ?祩?? (? ????? ???ᨢ?)
	get_cell_offset ebx,dword[eax]
	mov ecx,dword[x]
	mov dword[ebx],ecx ;+0 = .x
	mov edx,dword[y]
	mov dword[ebx+4],edx ;+4 = .y
	.change:
		mov ecx,[li]
		mov byte[ebx+offs_cell_liv],cl
	.fun_e:
	popad
	ret
endp

;㤠????? ?祩??
align 4
proc pole_cell_delete, pole:dword, x:dword, y:dword
	pushad
	mov edi,dword[pole]
	mov ebx,edi
	add ebx,offs_pole_b_sort
	mov esi,pole_index

	mov ecx,[esi]
	cmp ecx,1
	jl .fun_e

	stdcall pole_cell_find, [pole], [x],[y]
	cmp eax,0
	je .fun_e ;?᫨ ???⪠ ?? ?뫠 ᮧ????

	dec dword[esi]

	mov edi,esi
	add edi,4
	mov edx,ecx
	cld
	repnz scasd ;?????
	sub edi,4

	cmp dword[ebx],1 ;[ebx]=pole_b_sort
	jl @f
	mov eax,edi
	sub eax,esi ;esi=pole_index
	shr eax,2
	dec eax
	cmp [ebx],eax ;eax - ??????? 㪠??⥫? 㤠?塞?? ?祩??
	jle @f ;?뫮 jl @f
		dec dword[ebx]
	@@:

	shl edx,2
	add edx,esi ;???????? ??????? ???ᨢ?
	sub edx,edi
	shr edx,2
	mov ecx,edx

	bt ecx,31
	jc .fun_e
		mov esi,edi
		add esi,4
		mov edx,[edi] ;???࠭???? ⥪?饣? 㪠??⥫?
		cld
		rep movsd
		mov [edi],edx ;????⠭??????? ⥪?饣? 㪠??⥫? (? ????? ???ᨢ?)
	.fun_e:
	popad
	ret
endp

if debug
align 4
proc but_test_pole, pole:dword
	pushad
	stdcall [buf2d_clear], buf_0, [buf_0.color]

	mov edi,dword[pole]
	stdcall pole_paint,edi
	mov ebx,5

	mov esi,pole_index
	mov ecx,[esi]

	mov eax,pole_b_sort
	mov edi,open_file_lif
	stdcall convert_int_to_str
	stdcall [buf2d_draw_text], buf_0, buf_font,edi,5,ebx,[shem_colors] ;???㥬 b_sort
	add ebx,18

	mov eax,[esi]
	add esi,4
	stdcall convert_int_to_str
	stdcall [buf2d_draw_text], buf_0, buf_font,edi,5,ebx,[shem_colors] ;???㥬 ??᫮ ??祪
	add ebx,9

	cmp ecx,1
	jl .end_dr
	cld
	@@:
		mov eax,[esi]
		add esi,4
		stdcall convert_int_to_str
		stdcall [buf2d_draw_text], buf_0, buf_font,edi,5,ebx,[color_captions] ;???㥬 㪠??⥫? ?? ???ᨢ? ??祪
		add ebx,9
		loop @b
	.end_dr:
	mov ecx,4
	cld
	@@:
		mov eax,[esi]
		add esi,4
		stdcall convert_int_to_str
		stdcall [buf2d_draw_text], buf_0, buf_font,edi,5,ebx,[color_border] ;???㥬 4 ??ப? 㪠??⥫??
		add ebx,9
		loop @b

	stdcall [buf2d_draw], buf_0

	;call redraw_pole
	popad
	ret
endp
end if

;output:
; eax - index
align 4
proc pole_cell_find uses edi, pole:dword, x:dword, y:dword
	mov edi,dword[pole]

	mov eax,pole_index
	cmp dword[eax],0
	jne @f
		xor eax,eax ;if(!fristC) return 0;
		jmp .fun_e
	@@:

	xor eax,eax ;fnd=0;
	cmp pole_b_sort,0
	je @f
		stdcall pole_bin_find, pole_index, [x],[y], pole_b_sort ;i=BinFind(pole_index, x,y, pole_b_sort);
		cmp eax,0
		je @f
			shl eax,2
			add eax,pole_index
			mov eax,dword[eax] ;if(i) fnd=pole_index[i];
			jmp .fun_e
	@@:

	cmp eax,0
	jne @f ;????? ?祩?? ?? ???????? ??ॢ??
		push ebx ecx edx esi
		;ebx -> i
		;ecx -> firstC
		;edx -> &pole_index[i]
		;esi -> cell[pole_index[i]]
		mov ecx,pole_index
		mov ebx,pole_b_sort
		mov edx,ebx
		shl edx,2
		add edx,ecx
		mov ecx,dword[ecx]
		.cycle_b: ;for(i=pole_b_sort+1;i<=fristC;i++)
			inc ebx
			cmp ebx,ecx
			jg .not_found
			add edx,4
			get_cell_offset esi,dword[edx]
			mov eax,dword[x]
			cmp dword[esi],eax ;+0 = .x
			jne .if_e
			mov eax,dword[y]
			cmp dword[esi+4],eax ;+4 = .y
			jne .if_e
				;if(cell[pole_index[i]].x==x && cell[pole_index[i]].y==y){
				mov eax,dword[edx] ;fnd=pole_index[i];
				jmp .cycle_e ;break;
			.if_e:
			jmp .cycle_b
			.not_found:
		xor eax,eax ;????⠭???????? ?㫥??? ???祭?? ?᫨ ?? ??諨 ?祩?? (? 横?? eax ???????? ??? ?஢?થ ???न???)
		.cycle_e:
		pop esi edx ecx ebx
	@@:
	.fun_e:
	ret
endp

;output:
; eax - index
align 4
proc pole_bin_find uses ebx ecx edx edi, mas:dword, fx:dword, fy:dword, k:dword
	xor eax,eax
	mov ebx,1 ;ebx - ???ᨬ?????? ???冷? ??? ??ॢ?
	@@:
	cmp dword[k],ebx
	jle @f ;while(k>por)
		shl ebx,1 ;por<<=1;
		jmp @b
	@@:
	cmp dword[k],ebx
	jge @f ;if(k<por)
		shr ebx,1 ;por>>=1;
	@@:
	mov ecx,ebx ;i=por;

	;ecx -> i
	;edi -> mas[i]
	.cycle_b: ;do{
		shr ebx,1 ;por>>=1;

		mov edi,ecx
		shl edi,2
		add edi,dword[mas]
		;if(compare_cells_mb(mas[i],fx,fy)){
		stdcall pole_compare_cells_mb_coords, dword[edi],[fx],[fy]
		cmp dl,0
		je .if_u0_e
			@@: ;while(i+por>k)
			mov edx,ecx
			add edx,ebx
			cmp edx,dword[k] ;i+por>k
			jle @f
				shr ebx,1 ;por>>=1;
				jmp @b
			@@:
			add ecx,ebx ;i+=por;
			jmp .if_e
		.if_u0_e:
		;else if(compare_cells_bm(mas[i],fx,fy))i-=por;
		stdcall pole_compare_cells_bm_coords, dword[edi],[fx],[fy]
		cmp dl,0
		je .if_u1_e
			sub ecx,ebx
			jmp .if_e
		.if_u1_e:
		;else { m=i; por=0; }
			mov eax,ecx
			xor ebx,ebx
		.if_e:
	cmp ebx,0
	jne .cycle_b ;}while(por);

	ret
endp

;ᤢ?? ???? ?祥? (? ??ꥪ⮢)
align 4
proc pole_move_all, pole:dword, m_d_x:dword, m_d_y:dword
pushad
	mov edi,dword[pole]
	mov edx,[m_d_x]
	mov esi,[m_d_y]

	mov eax,pole_index
	cmp dword[eax],0
	je .end_0 ;?᫨ ??? ?祥? (?஢????) ?? ??室

	mov ecx,dword[eax]
	cld
	@@: ;横? ?? ?ᥬ ?祩???
		add eax,4
		mov ebx,[eax]
		imul ebx,sizeof.Cell
		add ebx,pole_data

		add dword[ebx+offs_cell_x],edx
		add dword[ebx+offs_cell_y],esi
		loop @b
	.end_0:

	;横? ?? ??????᪨? ??????⠬ ? ????????
	stdcall dword[tl_node_poi_get_info], tree1,0
	@@:
		cmp eax,0
		je .end_1
		cmp word[eax],el_icon_elems ;????祭?? ??१ eax ⨯ ??????
		je .mov_1
		cmp word[eax],el_icon_captions
		je .mov_1
			jmp .end_mov_1
		.mov_1:
			mov ecx,eax
			stdcall [tl_node_poi_get_data], tree1,eax
			add [eax],edx ;coord x
			add [eax+4],esi ;coord y
			mov eax,ecx
		.end_mov_1:
		stdcall dword[tl_node_poi_get_next_info], tree1,eax ;????室?? ? ᫥??饬? 㧫?
		jmp @b
	.end_1:

popad
	ret
endp

;output:
; dl
align 4
proc pole_compare_cells_bm_coords uses eax ebx ecx, i0:dword, fx:dword, fy:dword
	get_cell_offset eax,[i0]
	;eax -> cell[i0]
	mov ebx,dword[fx]
	cmp dword[eax],ebx
	jle @f
		mov dl,1
		jmp .fun_e
	@@:
	mov ecx,dword[fy]
	cmp dword[eax+4],ecx
	jle @f
	cmp dword[eax],ebx
	jne @f
		mov dl,1
		jmp .fun_e
	@@:
	xor dl,dl
	.fun_e:
	ret
endp

;output:
; dl
align 4
proc pole_compare_cells_mb_coords uses eax ebx ecx, i0:dword, fx:dword, fy:dword
	get_cell_offset eax,[i0]
	;eax -> cell[i0]
	mov ebx,dword[fx]
	cmp dword[eax],ebx
	jge @f
		mov dl,1
		jmp .fun_e
	@@:
	mov ecx,dword[fy]
	cmp dword[eax+4],ecx
	jge @f
	cmp dword[eax],ebx
	jne @f
		mov dl,1
		jmp .fun_e
	@@:
	xor dl,dl
	.fun_e:
	ret
endp

;output:
; dl
align 4
proc pole_compare_cells_bm, i0:dword, i1:dword
	push eax ebx ecx
	get_cell_offset eax,[i0] ;eax -> cell[i0]
	get_cell_offset ebx,[i1] ;ebx -> cell[i1]
	mov ecx,dword[ebx] ;+0 = .x
	cmp dword[eax],ecx
	jle @f ;x0>x1
		mov dl,1
		jmp .fun_e
	@@:
	jne @f ;x0==x1
	mov ecx,dword[ebx+4] ;+4 = .y
	cmp dword[eax+4],ecx
	jle @f ;y0>y1
		mov dl,1
		jmp .fun_e
	@@:
	xor dl,dl
	.fun_e:
	pop ecx ebx eax
	ret
endp

;description:
; ???⪠ ?祥? (?஢????), ??⠭???? ?? ???? ?஢???? 0-?? ᨣ????
; ?㦭? ???뢠?? ??? ??ନ஢???? ??? ??। ????᪮? ?奬?
align 4
proc pole_reset_cells uses eax ebx ecx edi, pole:dword
	mov edi,dword[pole]
	mov eax,pole_index
	cmp dword[eax],0
	je .fun_e ;?᫨ ??? ?祥? (?஢????) ?? ??室

	mov ecx,dword[eax]
	cld
	@@: ;横? ?? ?ᥬ ?祩???
		add eax,4
		mov ebx,[eax]
		imul ebx,sizeof.Cell
		add ebx,pole_data
		;and byte[ebx+offs_cell_liv],0xfe ;????? ????襣? ????
		cmp byte[ebx+offs_cell_liv],2
		je .no_clear
			mov byte[ebx+offs_cell_liv],0
		.no_clear:
		loop @b
	.fun_e:
	ret
endp

align 4
proc p_paint_elems uses eax esi
	stdcall dword[tl_node_poi_get_info], tree1,0
	@@:
		cmp eax,0
		je @f
		mov esi,eax
		cmp word[esi],el_icon_elems ;????祭?? ??१ esi ⨯ ??????
		jne .end_element
			stdcall [tl_node_poi_get_data], tree1,esi
			stdcall el_draw, eax
		.end_element:
		cmp word[esi],el_icon_captions ;????祭?? ??१ esi ⨯ ??????
		jne .end_caption
			stdcall [tl_node_poi_get_data], tree1,esi
			stdcall capt_draw, eax
		.end_caption:
		stdcall dword[tl_node_poi_get_next_info], tree1,esi
		jmp @b
	@@:
	ret
endp

;description:
; ?㭪??? ??ᮢ???? ???????? ?? ????
align 4
proc el_draw, h_elem:dword
	pushad

	mov edi,[h_elem]
	mov eax,[edi] ;coord x
	mov ebx,[edi+4] ;coord y

	movzx edi,byte[edi+sp_offs_el_type]
	imul edi,size_el_opt
	add edi,el_opt_beg ;edi - 㪠??⥫? ?? ????????? ?? ᢮??⢠?? ????????

	movzx ecx,byte[edi+el_offs_box_x]
	movzx edx,byte[edi+el_offs_box_y]
	dec ecx
	dec edx

	push eax ebx
		mov esi,[h_elem]
		movzx esi,byte[esi+8]
		push dword[edi+el_offs_col]
		push ebx
		push eax
		stdcall move_rotate_n90, ecx,edx,esi
		stdcall draw_scaled_rect, eax,ebx ;??ᮢ??? ??????? ????????
	pop ebx eax

	;*** ???????? ??ᮢ???? ??? ***
	movzx esi,byte[zoom]
	cmp esi,1
	jne .end_m1
		;*** ??ᮢ???? ??? ??? 1-? ????⠡? ***
		;?室??? ????
		mov esi,[h_elem]
		stdcall el_get_leg_coords,esi,0 ;??⠭???? ??ࠬ??஢ 0-? ????
		add eax,[Cor_x]
		add ebx,[Cor_y]
		movzx esi,byte[esi+8]
		stdcall move_rotate_n90, 1,0,esi
		mov edx,1
		@@:
			stdcall [buf2d_set_pixel], buf_0, eax,ebx,dword[edi+el_offs_col]
			mov ecx,[edi+el_offs_legs_inp]
			movzx ecx,byte[ecx+edx]
			cmp ecx,0
			je @f
			stdcall move_rotate_n90, 0,ecx,esi
			inc edx
			jmp @b
		@@:

		;??室??? ????
		mov esi,[h_elem]
		stdcall el_get_leg_coords,esi,(1 shl 16) ;??⠭???? ??ࠬ??஢ 0-? ????
		add eax,[Cor_x] ;??? ࠡ??? ? buf2d_line
		add ebx,[Cor_y] ;??? ࠡ??? ? buf2d_line
		movzx esi,byte[esi+8]
		stdcall move_rotate_n90, -2,0,esi
		mov edx,el_offs_legs_out
		inc edx
		@@:
			push dword[edi+el_offs_col]
			stdcall move_rotate_n90, 1,0,esi
			push ebx
			push eax
			stdcall move_rotate_n90, -1,0,esi
			;stdcall draw_scaled_rect, eax,ebx
			stdcall [buf2d_line], buf_0, eax,ebx
			movzx ecx,byte[edi+edx]
			cmp ecx,0
			je @f
			stdcall move_rotate_n90, 0,ecx,esi
			inc edx
			jmp @b
		@@:

		jmp .end_mn
	.end_m1:
		;*** ??ᮢ???? ??? ??? n-? ????⠡? ***
		;?室??? ????
		xor edx,edx
		@@:
			stdcall el_get_leg_coords,[h_elem],edx
			mov ecx,eax
			or ecx,ebx
			jz @f
			mov ecx,[h_elem]
			movzx ecx,byte[ecx+8]
			stdcall move_rotate_n90, 1,0,ecx
			add eax,[Cor_x]
			add ebx,[Cor_y]
			imul eax,esi
			imul ebx,esi
			stdcall [buf2d_filled_rect_by_size], buf_0, eax,ebx,esi,esi, dword[edi+el_offs_col]
			inc edx
			jmp @b
		@@:

		;??室??? ????
		mov edx,(1 shl 16)
		@@:
			stdcall el_get_leg_coords,[h_elem],edx
			mov ecx,eax
			or ecx,ebx
			jz @f
			mov ecx,[h_elem]
			movzx ecx,byte[ecx+8]

			push dword[edi+el_offs_col]
			stdcall move_rotate_n90, -2,0,ecx
			push ebx
			push eax
			stdcall move_rotate_n90, 1,0,ecx
			stdcall draw_scaled_rect, eax,ebx

			inc edx
			jmp @b
		@@:		
	.end_mn:
	popad
	ret
endp

;description:
;??ᮢ???? ?????ᥩ
align 4
proc capt_draw uses eax ebx edi esi, h_capt:dword
	mov edi,[h_capt]
	mov eax,[edi] ;coord x
	mov ebx,[edi+4] ;coord y

	movzx esi,byte[zoom]
	cmp esi,3
	jl @f
		;??ᮢ???? ࠬ??, ?????? ????⮩ ??窨
		stdcall draw_point_border, eax,ebx, [color_captions]
	@@:

	add eax,[Cor_x]
	add ebx,[Cor_y]

	cmp esi,1
	jle @f
		imul eax,esi
		imul ebx,esi
	@@:

	cmp esi,3
	jl @f
		;ᤢ?? ??????? ? ???⮬ ࠬ??
		add eax,esi
		inc eax
	@@:

	add edi,capt_offs ;edi - 㪠??⥫? ?? ?????? ??????? (? ???न??⠬?)
	call str_next_val
	call str_next_val
	stdcall [buf2d_draw_text], buf_0, buf_font,edi,eax,ebx,[color_captions] ;???㥬 ??ப? ? ⥪?⮬
	ret
endp

;description:
; ?㭪??? ??? ?뤥????? ???????? ??ꥪ⮢ ?? ??㯭?? ????⠡??
; ?????? ?㭪??? ?祭? ??宦? ?? draw_signal_rect
align 4
proc draw_point_border uses eax ebx edi, x0:dword,y0:dword, color:dword
	movzx edi,byte[zoom]
	mov ebx,[y0]
	mov eax,[x0]

	add ebx,[Cor_y]
	imul ebx,edi
	add eax,[Cor_x]
	imul eax,edi

	stdcall [buf2d_rect_by_size], buf_0, eax,ebx,edi,edi, dword[color]
	ret
endp

;description:
; ????㭪??? ??? ??ᮢ???? 㢥??祭??? ??אַ㣮?쭨??? ?? ?奬?
align 4
proc draw_scaled_rect uses eax ebx ecx edx edi, x0:dword,y0:dword,x1:dword,y1:dword, color:dword
	movzx edi,byte[zoom]
	mov edx,[y1]
	mov ecx,[x1]
	mov ebx,[y0]
	mov eax,[x0]

	cmp eax,ecx
	jle @f
		xchg eax,ecx
	@@:
	sub ecx,eax
	cmp ebx,edx
	jle @f
		xchg ebx,edx
	@@:
	sub edx,ebx

	inc ecx
	inc edx

	imul edx,edi
	imul ecx,edi
	add ebx,[Cor_y]
	imul ebx,edi
	add eax,[Cor_x]
	imul eax,edi

	stdcall [buf2d_filled_rect_by_size], buf_0, eax,ebx,ecx,edx, dword[color]
	ret
endp

align 4
proc pole_paint, pole:dword
	pushad

	;*** நᮢ???? ࠬ??
	mov eax,[Cor_x]
	mov ebx,[Cor_y]
	mov ecx,[shem_w]
	mov edx,[shem_h]
	movzx esi,byte[zoom]
	cmp esi,1
	jle @f
		imul eax,esi
		imul ebx,esi
		imul ecx,esi
		imul edx,esi
	@@:
	dec eax
	dec ebx
	add ecx,2
	add edx,2
	stdcall [buf2d_rect_by_size], buf_0, eax,ebx, ecx,edx, [color_border]

	;eax -> firstC
	;ebx -> i
	;ecx -> cell[pole_index[i]]
	;edx -> color

	mov edi,dword[pole]
	mov eax,pole_index
	cmp dword[eax],0
	je .no_draw

	mov eax,dword[eax]
	mov ebx,1

;---
	@@: ;while(i<pole_b_sort && Cor_x+cell[pole_index[i]].x<0)
		cmp ebx,pole_b_sort
		jge @f ;????室?? ?? ??砫? ??????? 横??
		mov ecx,ebx
		shl ecx,2
		add ecx,pole_index
		get_cell_offset ecx,dword[ecx]
		mov edx,dword[ecx] ;+0 = .x
		add edx,dword[Cor_x]
		cmp edx,0
		jge @f ;????室?? ?? ??砫? ??????? 横??
			inc ebx ;i++; // ??? ?ய?᪠ ?祥? ?? ????? ᫥??
		jmp @b
	@@:

	;eax -> pole_index[firstC]
	;ebx -> pole_index[i]
	;edi -> coord_x
	;esi -> coord_y
	shl eax,2
	shl ebx,2
	add eax,pole_index
	add ebx,pole_index

	cmp byte[zoom],2
	jge .zoom2
	@@: ;for(;i<=fristC;i++){
		get_cell_offset ecx,dword[ebx]
;...
		mov edi,dword[Cor_x]
		add edi,dword[ecx] ;+0 = .x
		mov esi,dword[Cor_y]
		add esi,dword[ecx+4] ;+4 = .y
		movzx edx,byte[ecx+offs_cell_liv]
		and edx,3 ;??࠭?祭??
		shl edx,2
		add edx,shem_colors
		stdcall [buf2d_set_pixel], buf_0, edi, esi, [edx]
;...
		add ebx,4
		cmp ebx,eax
		jle @b

	jmp .no_draw
	.zoom2:

	@@: ;for(;i<=fristC;i++){
		get_cell_offset ecx,dword[ebx]

		movzx edx,byte[zoom] ;edx ?ᯮ???????? ??? ???ᥭ?? zoom ? 4 ???⭮? ??᫮
		mov edi,dword[ecx] ;+0 = .x
		add edi,dword[Cor_x]
		imul edi,edx
		mov esi,dword[ecx+4] ;+4 = .y
		add esi,dword[Cor_y]
		imul esi,edx

		movzx edx,byte[ecx+offs_cell_liv]
		and edx,3 ;??࠭?祭??
		shl edx,2
		add edx,shem_colors

		movzx ecx,byte[zoom]
		;;;dec ecx
		stdcall [buf2d_filled_rect_by_size], buf_0, edi, esi, ecx, ecx, [edx]
		add ebx,4
		cmp ebx,eax
		jle @b

	.no_draw:
	popad
	call p_paint_elems
	ret
endp

;?????஢?? ?祥? ????, ?㦭? ??? ????? ????ண? ???᪠
align 4
proc pole_sort uses eax edi, pole:dword
	mov edi,dword[pole]
	mov eax,pole_index
	mov eax,dword[eax] ;firstC -> eax
	stdcall pole_fl_sort, pole_index,eax ;??????㥬 ??? ?祩??
	mov pole_b_sort,eax ;?⠢?? ??᫮ ???????஢????? ?祥? ࠢ??? ????? ???? ???????????? ?祥?
	ret
endp

;?????஢?? ??????? a[1..n] ??⮤?? ??????
;??????? a[0] ? ?????஢?? ?? ?????????
align 4
proc pole_fl_sort uses eax ecx edx edi esi, a:dword, n:dword
	mov ecx,dword[a]
	;??ନ஢??? ??室??? ?????筮 㯮?冷祭??? ??ॢ?
	mov eax,dword[n]
	shr eax,1
	@@: ;for(i=n>>1; i>=2; i--)
		stdcall pole_fl_surface, ecx,eax,[n] ;(a,i,n)
		dec eax
		cmp eax,2
		jge @b
	;?믮????? ???楤??? ?ᯫ???? ?????? ??? ??????? ?????ॢ?
	mov eax,dword[n]
	@@: ;for(i=n; i>=2; i--){
		stdcall pole_fl_surface, ecx,1,eax ;(a,1,i)
		;????????? ????????? ???ᨬ?????? ??????? ? ????? ᯨ᪠
		mov edi,eax
		shl edi,2
		add edi,ecx ;edi -> &a[i]
		mov esi,dword[edi] ;w=a[i];
		mov edx,dword[ecx+4]
		mov dword[edi],edx ;a[i]=a[1];
		mov dword[ecx+4],esi ;a[1]=w;

		dec eax
		cmp eax,2
		jge @b
	ret
endp

;???楤??? ?ᯫ???? ?????? ?? ??ॢ? a[1..k]
align 4
proc pole_fl_surface, a:dword, i:dword, k:dword
locals
	copy dd ?
endl
	pushad
	;edx -> ...
	;edi -> m
	;esi -> j
	mov eax,dword[a]
	mov ebx,dword[i]
	mov ecx,dword[k]

	mov edx,ebx
	shl edx,2
	add edx,eax
	mov edx,dword[edx]
	mov dword[copy],edx ;copy=a[i];
	mov edi,ebx
	shl edi,1 ;m=i<<1;
	.cycle_b: ;while (m<=k) {
		cmp edi,ecx
		jg .cycle_e
		jne @f ;if (m==k) j=m;
			mov esi,edi
			jmp .else_e
		@@: ;else if (pole_compare_cells_bm(a[m],a[m+1])) j=m;
		mov edx,edi
		shl edx,2
		add edx,eax
		stdcall pole_compare_cells_bm, dword[edx],dword[edx+4]
		cmp dl,0
		je @f
			mov esi,edi
			jmp .else_e
		@@: ;else j=m+1;
			mov esi,edi
			inc esi
		.else_e:

		;if (pole_compare_cells_bm(a[j],copy)) {
		mov edx,esi
		shl edx,2
		add edx,eax
		stdcall pole_compare_cells_bm, dword[edx],dword[copy]
		cmp dl,0
		je .cycle_e ;} else break; //??室 ?? 横??

		mov edx,esi
		shl edx,2
		add edx,eax
		push dword[edx] ;push a[j];
		mov edx,ebx
		shl edx,2
		add edx,eax
		pop dword[edx] ;a[i]=a[j];
		mov ebx,esi ;i=j;
		mov edi,ebx
		shl edi,1 ;m=i<<1;

		jmp .cycle_b
	.cycle_e:

	;???祭?? ?????? ॣ???஢ 㦥 ?? ????? ?. ?. ????? ?㭪樨
	shl ebx,2
	add eax,ebx
	mov edx,dword[copy]
	mov dword[eax],edx ;a[i]=copy;

	popad
	ret
endp
;--------------------------------------

align 4
proc pole_draw_pok, pole:dword
pushad
	;mov edi,dword[pole]

	mov ecx,[sc.work_text]
	or  ecx,0x80000000 ;or (1 shl 30)
	;mov edi,[sc.work]
	mcall SF_DRAW_TEXT,(400 shl 16)+5,,txt_zoom

	add bx,9
	mov edx,txt_osob
	int 0x40

	add bx,9
	mov edx,txt_info
	int 0x40

	movzx ecx,byte[zoom]
	mov edx,(400+6*9)*65536+5
	mov esi,[sc.work_button_text]
	or  esi,(1 shl 30)
	mov edi,[sc.work_button]
	mcall SF_DRAW_NUMBER,(2 shl 16) ;????⠡

	mov edi,dword[pole]
	mov ecx,pole_index
	mov ecx,[ecx]
	mov edi,[sc.work_button]
	add edx,(6*0)*65536+9
	mcall ,(5 shl 16) ;??᫮ ??祪
popad
	ret
endp

align 4
but_zoom_p:
	cmp byte[zoom],16
	jge @f
		pushad
		;????᫥??? ᤢ???? ??? ????, ??????? ???ᯥ??? 業?஢?? ???? ??? 㢥??祭?? ????⠡?
		movzx ecx,byte[zoom]
		xor edx,edx
		mov eax,dword[buf_0.w]
		shr eax,1 ;? eax ???????? ??ਭ? ????
		mov ebx,eax ;?????? १?ࢭ?? ????? eax
		div ecx ;????? eax ?? ⥪?騩 ????⠡
		xchg eax,ebx
		xor edx,edx
		inc ecx
		div ecx ;????? eax ?? ????? ????⠡
		sub ebx,eax ;??????????? ᤢ?? ???? ??????? ???ᯥ??? 業?஢?? ????
		sub dword[Cor_x],ebx ;ᤢ????? ???? ?७?? ?? ??? x
		xor ecx,ecx
		mov cl,byte[zoom]
		xor edx,edx
		mov eax,dword[buf_0.h]
		shr eax,1
		mov ebx,eax
		div ecx
		xchg eax,ebx
		xor edx,edx
		inc ecx
		div ecx
		sub ebx,eax
		sub dword[Cor_y],ebx ;ᤢ????? ???? ?७?? ?? ??? y

		inc byte[zoom]
		stdcall pole_draw_pok, pole
		popad

		.buf_clear:
			call redraw_pole
	@@:
	ret

align 4
but_zoom_m:
	cmp byte[zoom],1
	jle @f
		pushad
		;????᫥??? ᤢ???? ??? ????, ??????? ???ᯥ??? 業?஢?? ???? ??? 㬥??襭?? ????⠡?
		movzx ecx,byte[zoom]
		xor edx,edx
		mov eax,dword[buf_0.w]
		shr eax,1 ;? eax ???????? ??ਭ? ????
		mov ebx,eax ;?????? १?ࢭ?? ????? eax
		div ecx ;????? eax ?? ⥪?騩 ????⠡
		xchg eax,ebx
		xor edx,edx
		dec ecx
		div ecx ;????? eax ?? ????? ????⠡
		sub ebx,eax ;??????????? ᤢ?? ???? ??????? ???ᯥ??? 業?஢?? ????
		sub dword[Cor_x],ebx ;ᤢ????? ???? ?७?? ?? ??? x
		xor ecx,ecx
		mov cl,byte[zoom]
		xor edx,edx
		mov eax,dword[buf_0.h]
		shr eax,1
		mov ebx,eax
		div ecx
		xchg eax,ebx
		xor edx,edx
		dec ecx
		div ecx
		sub ebx,eax
		sub dword[Cor_y],ebx ;ᤢ????? ???? ?७?? ?? ??? y

		dec byte[zoom]
		stdcall pole_draw_pok, pole
		popad

		.buf_clear:
			call redraw_pole
	@@:
	ret

;業?஢?? ?奬? ?? 業??? ??࠭?
align 4
proc but_center uses eax ebx ecx edx
	movzx ecx,byte[zoom]
	cmp ecx,1
	jle .end_m_1
		mov eax,[buf_0.w]
		mov ebx,[shem_w]
		imul ebx,ecx

		sub eax,ebx
		xor edx,edx
		shl ecx,1
		cmp eax,0
		jge @f
			neg eax
			inc eax
			div ecx
			neg eax
			inc eax
			jmp .set_x
		@@:
			div ecx
		.set_x:
		mov [Cor_x],eax
		mov eax,[buf_0.h]
		mov ebx,[shem_h]
		shr ecx,1
		imul ebx,ecx
		sub eax,ebx
		xor edx,edx
		shl ecx,1
		cmp eax,0
		jge @f
			neg eax
			inc eax
			div ecx
			neg eax
			inc eax
			jmp .set_y
		@@:
			div ecx
		.set_y:
		mov [Cor_y],eax
		jmp .end_m_n
	.end_m_1:
		mov eax,[buf_0.w]
		sub eax,[shem_w]
		shr eax,1
		bt eax,30
		jnc @f
			bts eax,31
		@@:
		mov [Cor_x],eax
		mov eax,[buf_0.h]
		sub eax,[shem_h]
		shr eax,1
		bt eax,30
		jnc @f
			bts eax,31
		@@:
		mov [Cor_y],eax
	.end_m_n:
	call redraw_pole
	ret
endp

align 4
but_pole_up:
	push eax ecx edx
	mov eax,dword[buf_0.h]
	shr eax,2
	movzx ecx,byte[zoom]
	cmp cx,2
	jl @f ;??????? ?? ????稭? zoom
		xor edx,edx
		div ecx
	@@:
	add dword[Cor_y],eax
	pop edx ecx eax
	call redraw_pole
	ret

align 4
but_pole_dn:
	push eax ecx edx
	mov eax,dword[buf_0.h]
	shr eax,2
	xor ecx,ecx
	mov cl,byte[zoom]
	cmp cx,2
	jl @f ;??????? ?? ????稭? zoom
		xor edx,edx
		div ecx
	@@:
	sub dword[Cor_y],eax
	pop edx ecx eax
	call redraw_pole
	ret

align 4
but_pole_left:
	push eax ecx edx
	mov eax,dword[buf_0.w]
	shr eax,2
	xor ecx,ecx
	mov cl,byte[zoom]
	cmp cx,2
	jl @f ;??????? ?? ????稭? zoom
		xor edx,edx
		div ecx
	@@:
	add dword[Cor_x],eax
	pop edx ecx eax
	call redraw_pole
	ret

align 4
but_pole_right:
	push eax ecx edx
	mov eax,dword[buf_0.w]
	shr eax,2
	xor ecx,ecx
	mov cl,byte[zoom]
	cmp cx,2
	jl @f ;??????? ?? ????稭? zoom
		xor edx,edx
		div ecx
	@@:
	sub dword[Cor_x],eax
	pop edx ecx eax
	call redraw_pole
	ret

;output:
; edx - count created points
align 4
proc shem_create_line uses eax ebx ecx edi, x:dword, y:dword, opt:dword
	mov edi,pole
	xor edx,edx

	mov ebx,[x]
	mov ecx,[y]
	bt dword[opt],0
	jnc @f
	.line_lr:
		inc ebx
		cmp ebx,[shem_w]
		jge @f
		stdcall pole_cell_find, pole,ebx,ecx
		cmp eax,0
		je .u0
			imul eax,sizeof.Cell
			add eax,pole_data
			cmp byte[eax+offs_cell_liv],1
			jne @f
		.u0:
		stdcall pole_cell_creat, pole,ebx,ecx,1
		inc edx
		jmp .line_lr
	@@:

	mov ebx,[x]
	;mov ecx,[y]
	bt dword[opt],2
	jnc @f
	.line_rl:
		dec ebx
		cmp ebx,0
		jl @f
		stdcall pole_cell_find, pole,ebx,ecx
		cmp eax,0
		je .u1
			imul eax,sizeof.Cell
			add eax,pole_data
			cmp byte[eax+offs_cell_liv],1
			jne @f
		.u1:
		stdcall pole_cell_creat, pole,ebx,ecx,1
		inc edx
		jmp .line_rl
	@@:

	mov ebx,[x]
	mov ecx,[y]
	bt dword[opt],3
	jnc @f
	.line_du:
		dec ecx
		cmp ecx,0
		jl @f
		stdcall pole_cell_find, pole,ebx,ecx
		cmp eax,0
		je .u2
			imul eax,sizeof.Cell
			add eax,pole_data
			cmp byte[eax+offs_cell_liv],1
			jne @f
		.u2:
		stdcall pole_cell_creat, pole,ebx,ecx,1
		inc edx
		jmp .line_du
	@@:

	;mov ebx,[x]
	mov ecx,[y]
	bt dword[opt],1
	jnc @f
	.line_ud:
		inc ecx
		cmp ecx,[shem_h]
		jge @f
		stdcall pole_cell_find, pole,ebx,ecx
		cmp eax,0
		je .u3
			imul eax,sizeof.Cell
			add eax,pole_data
			cmp byte[eax+offs_cell_liv],1
			jne @f
		.u3:
		stdcall pole_cell_creat, pole,ebx,ecx,1
		inc edx
		jmp .line_ud
	@@:

	ret
endp

align 4
redraw_pole:
	stdcall [buf2d_clear], buf_0, [buf_0.color]
	stdcall pole_paint, pole
	stdcall [buf2d_draw], buf_0
	ret
