;
; ? ?⮬ 䠩?? ᮡ࠭? ?㭪樨 ?????騥 ???? ????????????
;

;??।?????? ???न???? ?⭮??⥫쭮 ᪨??
macro move_to_skin coord_x,coord_y
{
	push eax
	mov eax,[obj_m_win] ;㪠??⥫? ?? ????
	cmp eax,0
	je @f
	test_bool_prop eax,BIT_MOD_WI_CORD_OTN_CL_OBL
	;bt dword[[obj_m_win]+u_object.modif-u_object],n_prop
	jnc @f
		add coord_x,SKIN_W1
		add coord_y,SKIN_H
	@@:
	pop eax
}

;??????ᮢ?? ???? ????????????
align 4
draw_constructor:
	stdcall [buf2d_clear], buf_fon,0xffffff

pushad
	cmp [err_ini1],1
	jne @f
		stdcall [buf2d_draw_text], buf_fon, buf_font,txtErrIni1,10,10,\
			0xff0000 ;???㥬 ??ப? ? ⥪?⮬
		stdcall [buf2d_draw_text], buf_fon, buf_font,fp_obj_opt,10,20,\
			0xff0000 ;???㥬 ??ப? ? ⥪?⮬
		jmp .end_draw
	@@:

	mov edx,C_WND
	stdcall find_obj_in_file,edx ;eax - 㪠??⥫? ?? ????????? ????
	cmp eax,0
	je .end_draw
	mov dword[obj_m_win],eax

	;4-width
	stdcall get_obj_text_col,[obj_m_win],4 ;edx = offset
	stdcall conv_str_to_int,edx
	mov ecx,eax ;ecx=width

	;5-height
	stdcall get_obj_text_col,[obj_m_win],5 ;edx = offset
	stdcall conv_str_to_int,edx
	;eax=height

	stdcall [buf2d_rect_by_size], buf_fon, 0,0,ecx,eax, dword[sc.frame] ;???㥬 ??????? ࠬ?? ????
	sub eax,2
	sub ecx,2
	stdcall [buf2d_filled_rect_by_size], buf_fon, 1,1,ecx,eax, dword[sc.grab] ;???㥬 䮭 ࠬ?? ????

	sub eax,6
	sub ecx,6
	stdcall [buf2d_rect_by_size], buf_fon, 4,4,ecx,eax, dword[sc.frame] ;???㥬 ????७??? ࠬ?? ????
	sub eax,2
	sub ecx,2
	stdcall [buf2d_filled_rect_by_size], buf_fon, 5,5,ecx,eax, dword[sc.work] ;???㥬 䮭 ????

;---;---;---
;jmp .end_skin
;---;---;---

	stdcall get_obj_text_col,[obj_m_win],1
	stdcall conv_str_to_int,edx ;eax = window style
	cmp eax,3
	jl .end_skin
		stdcall [buf2d_bit_blt], buf_fon, 0,0, buf_skin1 ;????? ᪨?
		mov edi,buf_skin1
		mov eax,buf2d_w ;eax - ??????? ??? ??ᮢ???? ?।???? ᪨??
		mov edi,buf_skin2
		mov ebx,buf2d_w ;ebx - ??ਭ? ?????? ᪨??
		sub ecx,ebx ;?⭨???? ?? ??ਭ? ???? ??ਭ? ?।???? ᪨??
		@@:
			stdcall [buf2d_bit_blt], buf_fon, eax,0, buf_skin2 ;?????ᮢ?? ?।???? ᪨??
			add eax,ebx
			cmp eax,ecx ;?ࠢ?????? ??????? ?।???? ᪨?? ? ??ਭ?? ????
			jl @b

		;4-width
		stdcall get_obj_text_col,[obj_m_win],4 ;edx = offset
		stdcall conv_str_to_int,edx
		mov edi,buf_skin3
		sub eax,buf2d_w ;?⭨???? ??ਭ? ?ࠢ??? ᪨??
		stdcall [buf2d_bit_blt], buf_fon, eax,0, buf_skin3 ;?????ᮢ?? ?ࠢ??? ᪨??
	.end_skin:

	;???㥬 ???????᪨? ????????
	mov edx,C_BUT
	stdcall draw_arr_objects, edx,draw_obj_but
	mov edx,C_EDB
	stdcall draw_arr_objects, edx,draw_obj_edit
	mov edx,C_CHB
	stdcall draw_arr_objects, edx,draw_obj_check
	mov edx,C_DTT
	stdcall draw_arr_objects, edx,draw_obj_text
	mov edx,C_REC
	stdcall draw_arr_objects, edx,draw_obj_rect_f13

	.end_draw:
popad
	ret

;?㭪??? ?????? ???ᨢ? ??।??????? ??????⮢
align 4
proc draw_arr_objects, obj_t:dword,p_fun_draw:dword
push eax ebx
	stdcall find_obj_in_file,[obj_t] ;eax - 㪠??⥫? ?? ????????? ??ꥪ??
	cmp eax,0
	je .end_draw_o
		mov ebx,[p_fun_draw]
		stdcall ebx,eax
		@@:
			stdcall find_obj_in_file_next,[obj_t] ;eax - 㪠??⥫? ?? ????????? ??ꥪ??
			cmp eax,0
			je .end_draw_o
				stdcall ebx,eax
		jmp @b
	.end_draw_o:
pop ebx eax
	ret
endp

;?㭪??? ?????? ??ꥪ? ??????, ? ???? ????????????
align 4
proc draw_obj_but, p_obj:dword
locals
	coor_x dd ?
	coor_y dd ?
endl
	pushad
		mov ebx,dword[p_obj]
		;1,2,3,4,5 - x,y,w,h,color

		stdcall get_obj_text_col,ebx,1
		stdcall conv_str_to_int,edx
		mov [coor_x],eax
		stdcall get_obj_text_col,ebx,2
		stdcall conv_str_to_int,edx
		mov [coor_y],eax
		move_to_skin [coor_x],[coor_y]
		stdcall get_obj_text_col,ebx,3
		stdcall conv_str_to_int,edx
		mov edi,eax
		stdcall get_obj_text_col,ebx,4
		stdcall conv_str_to_int,edx
		mov ecx,eax
		stdcall get_obj_text_col,ebx,5
		stdcall conv_str_to_int,edx
		test_bool_prop ebx,BIT_MOD_BUT_NFON
		jc .no_draw_fon
			stdcall [buf2d_filled_rect_by_size], buf_fon, dword[coor_x],dword[coor_y],edi,ecx,eax
			jmp @f
		.no_draw_fon: ;?? ?????ᮢ뢠?? ??????
			stdcall [buf2d_rect_by_size], buf_fon, dword[coor_x],dword[coor_y],edi,ecx,eax
		@@:
	popad
	ret
endp

;?㭪??? ?????? ??ꥪ? ⥪?⮢??? ????, ? ???? ????????????
align 4
proc draw_obj_edit, p_obj:dword
locals
	coor_x dd ?
	coor_y dd ?
	col_txt dd ? ;梥? ⥪???
endl
	pushad
		mov ebx,dword[p_obj]
		;1,2,3,4,...,6,...8,...,10 - w,x,y,col_fon,...,col_frame,...,col_txt,...,txt

		stdcall get_obj_text_col,ebx,2
		stdcall conv_str_to_int,edx
		mov [coor_x],eax
		stdcall get_obj_text_col,ebx,3
		stdcall conv_str_to_int,edx
		mov [coor_y],eax
		move_to_skin [coor_x],[coor_y]
		stdcall get_obj_text_col,ebx,1
		stdcall conv_str_to_int,edx
		mov ecx,eax
		stdcall get_obj_text_col,ebx,6
		stdcall conv_str_to_int,edx
		stdcall [buf2d_rect_by_size], buf_fon, dword[coor_x],dword[coor_y],\
			ecx,15, eax

		inc dword[coor_x]
		inc dword[coor_y]
		sub ecx,2
		stdcall get_obj_text_col,ebx,4
		stdcall conv_str_to_int,edx
		stdcall [buf2d_filled_rect_by_size], buf_fon, dword[coor_x],dword[coor_y],\
			ecx,13, eax

		stdcall get_obj_text_col,ebx,8
		stdcall conv_str_to_int,edx
		mov [col_txt],eax
		add [coor_x],2-1 ;ᬥ頥? ???न????, ???-??
		add [coor_y],4-1 ;⥪?? ??? ? ???? ??஢???
		stdcall get_obj_text_col,ebx,10
		stdcall [buf2d_draw_text], buf_fon, buf_font,edx,dword[coor_x],dword[coor_y],\
			dword[col_txt] ;???㥬 ??ப? ? ⥪?⮬

	popad
	ret
endp

;?㭪??? ?????? ??ꥪ? 䫠???, ? ???? ????????????
align 4
proc draw_obj_check, p_obj:dword
locals
	coor_x dd ?
	coor_y dd ?
	ot_txt dd ? ;?????? ??? ⥪???
	size_w dd ? ;ࠧ??? ??????⨪? w
	size_h dd ? ;ࠧ??? ??????⨪? h
	col_txt dd ? ;梥? ⥪???
endl
	pushad
		mov ebx,dword[p_obj]
		;1,2,3,4,5, 6,7,8, 9 - ot_x,ot_y,w,h,ot_w_txt, col_fon,col_frame,col_txt, txt

		stdcall get_obj_text_col,ebx,1 ;ot_x
		stdcall conv_str_to_int,edx
		mov [coor_x],eax
		stdcall get_obj_text_col,ebx,2 ;ot_y
		stdcall conv_str_to_int,edx
		mov [coor_y],eax
		move_to_skin [coor_x],[coor_y]
		stdcall get_obj_text_col,ebx,5 ;ot_w_txt
		stdcall conv_str_to_int,edx
		mov [ot_txt],eax
		stdcall get_obj_text_col,ebx,3 ;w
		stdcall conv_str_to_int,edx
		dec eax ;ࠧ??? ?????? ?? 1 ???ᥫ?
		mov [size_w],eax
		stdcall get_obj_text_col,ebx,4 ;h
		stdcall conv_str_to_int,edx
		dec eax ;ࠧ??? ?????? ?? 1 ???ᥫ?
		mov [size_h],eax
		stdcall get_obj_text_col,ebx,7 ;col_frame
		stdcall conv_str_to_int,edx
		stdcall [buf2d_rect_by_size], buf_fon, dword[coor_x],dword[coor_y],\
			dword[size_w],dword[size_h], eax

		;??????? 䮭? checkbox-?
		inc dword[coor_x]
		inc dword[coor_y]
		sub dword[size_w],2
		sub dword[size_h],2
		stdcall get_obj_text_col,ebx,6 ;col_fon
		stdcall conv_str_to_int,edx
		stdcall [buf2d_filled_rect_by_size], buf_fon, dword[coor_x],dword[coor_y],\
			dword[size_w],dword[size_h], eax

		test_bool_prop ebx,BIT_MOD_CHE
		jnc @f
			;??ᮢ???? ?⬥⪨ ? checkbox-?
			inc dword[coor_x]
			inc dword[coor_y]
			sub dword[size_w],2
			sub dword[size_h],2
			stdcall get_obj_text_col,ebx,7 ;col_frame
			stdcall conv_str_to_int,edx
			stdcall [buf2d_filled_rect_by_size], buf_fon, dword[coor_x],dword[coor_y],\
				dword[size_w],dword[size_h], eax
		@@:

		;???⠭???????? ????????? ? ࠧ????
		stdcall get_obj_text_col,ebx,1 ;ot_x
		stdcall conv_str_to_int,edx
		mov [coor_x],eax
		stdcall get_obj_text_col,ebx,2 ;ot_y
		stdcall conv_str_to_int,edx
		mov [coor_y],eax
		move_to_skin [coor_x],[coor_y]
		stdcall get_obj_text_col,ebx,3 ;w
		stdcall conv_str_to_int,edx
		dec eax ;ࠧ??? ?????? ?? 1 ???ᥫ?
		mov [size_w],eax
		stdcall get_obj_text_col,ebx,4 ;h
		stdcall conv_str_to_int,edx
		dec eax ;ࠧ??? ?????? ?? 1 ???ᥫ?
		mov [size_h],eax

		stdcall get_obj_text_col,ebx,8 ;col_txt
		stdcall conv_str_to_int,edx
		mov [col_txt],eax
		mov eax,[coor_x]
		add eax,[size_w]
		add eax,[ot_txt] ;??????塞 ???न???? ??? ⥪?⮢?? ???????

		test_bool_prop ebx,BIT_MOD_CHE_T
		jc .end_txt_align ;if text aligment = top

		test_bool_prop ebx,BIT_MOD_CHE_M
		jnc @f
			shl [coor_y],1 ;㬭????? ?? 2
			mov ecx,[size_h]
			add [coor_y],ecx ;??????塞 ?????? checkbox
			sub [coor_y],9 ;?⭨???? ?????? ᨬ????
			shr [coor_y],1 ;????? ?? ????稢襥?? 2, ????砥? ?????? ??? ????饣? ?? 業??? ⥪???
			jmp .end_txt_align
		@@:

		test_bool_prop ebx,BIT_MOD_CHE_B
		jnc @f
			mov ecx,[size_h]
			add [coor_y],ecx ;??????塞 ?????? checkbox
			sub [coor_y],9 ;?⭨???? ?????? ᨬ????
				;????砥? ?????? ??? ????饣? ᭨?? ⥪???
		@@:
		.end_txt_align:

		stdcall get_obj_text_col,ebx,9 ;txt
		stdcall [buf2d_draw_text], buf_fon, buf_font,edx,eax,dword[coor_y],\
			dword[col_txt] ;???㥬 ??ப? ? ⥪?⮬
	popad
	ret
endp

;?㭪??? ?????? ??ꥪ? ⥪?⮢?? ???????, ? ???? ????????????
align 4
proc draw_obj_text, p_obj:dword
locals
	coor_x dd ?
	coor_y dd ?
	col_txt dd ? ;梥? ⥪???
endl
	pushad
		mov ebx,dword[p_obj]
		;0,1,2,3,... - txt,x,y,col_txt,...

		stdcall get_obj_text_col,ebx,1
		stdcall conv_str_to_int,edx
		mov [coor_x],eax
		stdcall get_obj_text_col,ebx,2
		stdcall conv_str_to_int,edx
		mov [coor_y],eax
		move_to_skin [coor_x],[coor_y]

		stdcall get_obj_text_col,ebx,3
		stdcall conv_str_to_int,edx
		mov [col_txt],eax

		stdcall get_obj_text_col,ebx,0
		stdcall [buf2d_draw_text], buf_fon, buf_font,edx,dword[coor_x],dword[coor_y],\
			dword[col_txt] ;???㥬 ??ப? ? ⥪?⮬
	popad
	ret
endp

;?㭪??? ?????? ??ꥪ? ??אַ㣮?쭨?? (?????. ?. 13)
align 4
proc draw_obj_rect_f13, p_obj:dword
locals
	coor_x dd ?
	coor_y dd ?
	color dd ? ;梥? ⥪???
endl
	pushad
		mov ebx,dword[p_obj]
		;0,1,2,3,4 - color,x,y,w,h

		stdcall get_obj_text_col,ebx,1
		stdcall conv_str_to_int,edx
		mov [coor_x],eax
		stdcall get_obj_text_col,ebx,2
		stdcall conv_str_to_int,edx
		mov [coor_y],eax
		move_to_skin [coor_x],[coor_y]

		stdcall get_obj_text_col,ebx,0
		stdcall conv_str_to_int,edx
		mov [color],eax

		stdcall get_obj_text_col,ebx,3
		stdcall conv_str_to_int,edx
		mov ecx,eax

		stdcall get_obj_text_col,ebx,4
		stdcall conv_str_to_int,edx
		mov esi,eax

		stdcall [buf2d_filled_rect_by_size], buf_fon,\
			dword[coor_x],dword[coor_y],ecx,esi,dword[color]
	popad
	ret
endp
