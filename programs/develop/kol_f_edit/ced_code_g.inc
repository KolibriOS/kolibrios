;
; ? ?⮬ 䠩?? ᮡ࠭? ?㭪樨 ᮧ???騥 ?ணࠬ??? ???
;
u_object object ;?६????? ??ꥪ? ??? ??????? ᬥ饭??

macro insert_coma
{
	stdcall [ted_text_add], tedit0,txt_coma,1,ted_opt_ed_move_cursor
}
macro insert_nl
{
	stdcall [ted_text_add], tedit0,txt_nl,1,ted_opt_ed_move_cursor
}
macro insert_com
{
	stdcall [ted_text_add], tedit0,txt_com,1,ted_opt_ed_move_cursor
}
macro insert_sp
{
	stdcall [ted_text_add], tedit0,txt_sp,1,ted_opt_ed_move_cursor
}

;?㭪??? ??⠢???? ⥪?⮢?? ??ப? ? ⥪?⮢?? ।?????
align 4
proc insert_text, txt:dword
	push eax
	stdcall str_len,dword[txt]
	cmp eax,1
	jl @f
		stdcall [ted_text_add], tedit0,dword[txt],eax,ted_opt_ed_move_cursor
	@@:
	pop eax
	ret
endp

;??⠢???? 㪠?????? ⥪?⮢?? ᢮??⢮ ?? ??ꥪ?? ? ⥪?⮢?? ।?????
align 4
proc insert_text_prop, p_obj:dword,index:dword
	push eax edx
		mov eax,[p_obj]
		mov edx,[index]
		stdcall get_obj_text_col,eax,edx
		stdcall str_len,edx
		stdcall [ted_text_add], tedit0,edx,eax,ted_opt_ed_move_cursor
	pop edx eax
	ret
endp

;??⠢???? 㪠?????? ??⮢?? ᢮??⢮ ?? ??ꥪ?? ? ⥪?⮢?? ।?????
align 4
proc insert_bit_prop, p_obj:dword,index:dword
	push ebx ecx edi
		mov ebx,[p_obj]
		mov ecx,[index]
		test_bool_prop ebx,ecx
		jnc @f ;?᫨ ???짮??⥫? ?? ???ࠫ ? ??ꥪ?? ?????? ᢮??⢮
			mov ebx,dword[ebx]
			call find_obj_in_opt
			mov ecx,dword[edi+obj_opt.bit_val-obj_opt] ;ecx - ᬥ饭?? ???祭?? ᢮???? (????⠭?)
			cmp byte[ecx],0
			je @f ;?᫨ ? ??ꥪ?? ??? ???????? ??⮢?? ᢮????
				add ecx,obj_opt ;??????塞 ??砫? 䠩?? ? ????ﬨ
				stdcall get_string_by_ind, ecx,[index]
				cmp byte[edi],0
				je @f ;?᫨ ? ??ꥪ?? ??⮢?? ᢮??⢮ ???⮥
				stdcall insert_text, txt_plus
				stdcall insert_text, edi
		@@:
	pop edi ecx ebx
	ret
endp

;?㭪??? ??⠢???? 2 ??ࠬ???? ?? 16 ??? ? ????:
; (_p1_ shl 16 + _p2_)
align 4
proc insert_2_prop_16_bit, obj:dword, p_hi:dword, p_lo:dword
	stdcall insert_text, txt_skob_o
	stdcall insert_text_prop, [obj],[p_hi]
	stdcall insert_text, txt_shl_16
	stdcall insert_text, txt_skob_c
	stdcall insert_text, txt_plus
	stdcall insert_text_prop, [obj],[p_lo]
	ret
endp

;??⠢?塞 ??ப? ????:
; txt_(n) db (s)
align 4
proc insert_text_label_data, obj:dword, col:dword
	push eax edx
	mov eax,[obj]
		stdcall insert_text, txt_txt
		mov edx,file_name
		stdcall get_obj_id_str, eax,edx ;edx = 'n'
		stdcall str_cat, edx,txt_db ;edx = 'n db '
		stdcall insert_text, edx
		stdcall insert_text_prop, eax,[col]
		insert_nl
	pop edx eax
	ret
endp

macro insert_tab
{
	stdcall [ted_text_add], tedit0,txt_tab,1,ted_opt_ed_move_cursor
}
macro insert_br
{
	stdcall [ted_text_add], tedit0,txt_br,3,ted_opt_ed_move_cursor
}
macro insert_a_met
{
	stdcall [ted_text_add], tedit0,txt_a_met,3,ted_opt_ed_move_cursor
}
macro insert_0
{
	stdcall [ted_text_add], tedit0,txt_0,1,ted_opt_ed_move_cursor
}
macro insert_plus
{
	stdcall [ted_text_add], tedit0,txt_plus,1,ted_opt_ed_move_cursor
}
macro insert_xor
{
	stdcall [ted_text_add], tedit0,txt_xor,4,ted_opt_ed_move_cursor
}
macro insert_cmp
{
	stdcall [ted_text_add], tedit0,txt_cmp,4,ted_opt_ed_move_cursor
}
macro insert_reg reg_name
{
	stdcall [ted_text_add], tedit0,reg_name,3,ted_opt_ed_move_cursor
}
macro insert_reg8 reg_name
{
	stdcall [ted_text_add], tedit0,reg_name,2,ted_opt_ed_move_cursor
}

;
macro insert_key_beg
{
	stdcall [ted_text_add], tedit0,txt_key_1,txt_key_2-txt_key_1,ted_opt_ed_move_cursor
}
macro insert_key_end
{
	stdcall [ted_text_add], tedit0,txt_key_2,txt_key_3-txt_key_2,ted_opt_ed_move_cursor
}

macro insert_d_poi
{
	stdcall [ted_text_add], tedit0,txt_d_poi,1,ted_opt_ed_move_cursor
}
macro insert_ret
{
	stdcall [ted_text_add], tedit0,txt_ret,4,ted_opt_ed_move_cursor
}


align 4
but_update:
	call draw_constructor
	cmp byte[show_mode],0 ;?᫮??? ????????? ???? ????????????
	jne @f
		stdcall [buf2d_draw], buf_fon
	@@:
	call code_gen
	cmp byte[show_mode],1 ;?᫮??? ????????? ⥪?⮢??? ????
	jne @f
		stdcall [ted_draw], tedit0
	@@:
	ret

;?㭪??? ᮧ?????? ??? ?ᥣ? 䠩??
align 4
code_gen:
	stdcall [ted_clear], tedit0,1
pushad
	stdcall dword[tl_node_poi_get_info], tree2,0
	mov esi,eax
	xor bl,bl
	call code_gen_obj
	stdcall [ted_text_add], tedit0,\
		txt_nl,1,ted_opt_ed_move_cursor+ted_opt_ed_change_time ;??????塞 ᨬ??? ????? ??ப? ? ??樥? ted_opt_ed_change_time ??? ??????????? ???࠭???? 䠩??
	stdcall [ted_init_scroll_bars], tedit0, 4 ;???塞 ࠧ???? ??஫??????
popad
	ret

;input:
; bl - ?஢??? ??ꥪ??
; esi - 㪠??⥫? ?? 㧥?
align 4
code_gen_obj:
push eax ebx edx
	.beg_iterat:
		cmp esi,0
		je .find_no

		cmp byte[esi+2],bl ;lev
		jl .find_no

		stdcall dword[tl_node_poi_get_data], tree2,esi
		cmp eax,0
		je .find_no

		cmp dword[eax],C_STR
		jne .no_struct
			stdcall get_obj_text_col, eax,1 ;edx - 㪠??⥫? ?? 1-? ᢮??⢮ ?????????
			stdcall insert_text,txt_struc
			cmp byte[edx],0
			jne @f
				stdcall insert_text,txt_t
			@@:
			insert_sp
			stdcall insert_text_prop, eax,0

			cmp byte[edx],0
			je @f
				;?᫨ ????????? ?????? ?????ᮬ
				insert_sp
				stdcall insert_text_prop, eax,1
				insert_nl
				stdcall insert_text, txt_skob_fo
			@@:
			insert_nl
			stdcall get_code_children_objects, esi
			call move_next_no_child_objects ;?ய??? ????୨? ????⮢
			cmp byte[edx],0
			je @f
				;?᫨ ????????? ?????? ?????ᮬ
				stdcall insert_text, txt_skob_fc
				insert_nl
			@@:
			stdcall insert_text,txt_ends
		.no_struct:
		cmp dword[eax],C_TXT
		jne @f
			stdcall get_code_text,eax ;??ࠡ?⪠ ??ꥪ??
		@@:
		cmp dword[eax],C_INC
		jne @f
			stdcall get_code_incl,eax ;??ࠡ?⪠ ??ꥪ??
		@@:
		cmp dword[eax],C_VAR
		jne @f
			stdcall get_code_variable,eax,ebx ;??ࠡ?⪠ ??ꥪ??
		@@:
		cmp dword[eax],C_WND
		jne @f
			stdcall get_code_window,eax ;??ࠡ?⪠ ??ꥪ??
			stdcall get_code_children_objects, esi ;??ࠡ?⪠ ????୨? ????⮢
			call move_next_no_child_objects ;?ய??? ????୨? ????⮢
			stdcall insert_text, txt_draw_we ;??? ????????騩 ?㭪??? ??ᮢ????
		@@:
		cmp dword[eax],C_ACO
		jne @f
			stdcall get_code_auto,esi ;??ࠡ?⪠ ??ꥪ??
			call move_next_no_child_objects ;?ய??? ????୨? ????⮢
		@@:
		cmp dword[eax],C_KEY ;??????????
		jne @f
			stdcall get_code_key,eax ;??ࠡ?⪠ ??ꥪ??
		@@:

		stdcall dword[tl_node_poi_get_next_info], tree2,esi
		mov esi,eax ;????室?? ? ᫥??饬? 㧫?

		jmp .beg_iterat
	.find_no:
pop edx ebx eax
	ret

;????? ???? ????୨? ??ꥪ⮢
align 4
proc get_code_children_objects uses eax ebx esi, p_node:dword
	mov esi,[p_node]
	mov bl,byte[esi+2] ;???࠭塞 ?஢??? ⥪?饣? ??ꥪ??

	stdcall dword[tl_node_poi_get_next_info], tree2,esi
	cmp eax,0 ;?᫨ ???⨣?? ????? ᯨ᪠ ??ꥪ⮢
	je @f
	cmp byte[eax+2],bl ;?᫨ ?஢??? ᫥???饣? ??ꥪ?? ?????? ??? ࠢ?? ⥪?饬?
	jle @f ;?? ????୨? 㧫?? ???
		mov esi,eax
		mov bl,byte[esi+2]
		call code_gen_obj ;४??ᨢ??? ?맮? ????୨? ??ꥪ⮢
	@@:
	ret
endp

;?ய??? ???? ????୨? ??ꥪ⮢
;input:
; esi - 㪠??⥫? ?? 㧥?
;outpet:
; esi - 㪠??⥫? ᫥??騩 ?? ????୨? (?⭮??⥫쭮 ?室?饣?) ?? 㧥?
align 4
move_next_no_child_objects:
	push eax ebx
		mov bl,byte[esi+2] ;???࠭塞 ?஢??? ⥪?饣? ??ꥪ??
		@@:
			stdcall dword[tl_node_poi_get_next_info], tree2,esi
			cmp eax,0 ;?᫨ ???⨣?? ????? ᯨ᪠ ??ꥪ⮢
			je @f
			cmp byte[eax+2],bl ;?᫨ ?஢??? ᫥???饣? ??ꥪ?? ?????? ??? ࠢ?? ⥪?饬?
			jle @f ;?? ????୨? 㧫?? ???
			mov esi,eax ;?᫨ ᫥??騩 㧥? ???????? ????୨?, ?????⠢?塞 㪠??⥫?
			jmp @b
		@@:
	pop ebx eax
	ret

;?????????? ??ꥪ?? ? ?ணࠬ??? ????? (⥪?⮬)
align 4
proc get_code_text, p_obj:dword
	;pushad
		insert_nl
		stdcall insert_text_prop, dword[p_obj],0
		insert_nl
	;popad
	ret
endp

;??ࠡ?⪠ ??ꥪ?? ᮧ???饣? ??⮬???祪ᨥ ????
align 4
proc get_code_auto, p_node:dword
	pushad
		insert_nl
		mov esi,dword[p_node]
		stdcall dword[tl_node_poi_get_data], tree2,esi
		mov ebx,eax ;????砥? ?????? 㧫?

		test_bool_prop ebx,BIT_MOD_ACI
		jae @f
			mov edx,C_CHB
			stdcall find_obj_in_file,edx ;eax - 㪠??⥫? ?? ????????? check_box
			cmp eax,0
			je @f
				stdcall insert_text, txt_std_init_cb
		@@:
		test_bool_prop ebx,BIT_MOD_ACM
		jae @f
			;??⮬?????᪨? ᮡ???? ?? ????
			call get_code_auto_m
		@@:
		test_bool_prop ebx,BIT_MOD_ABU
		jae @f
			;ᮧ????? ?㭪樨 button
			stdcall insert_text, txt_but_beg
			call get_code_auto_but ;???? ?맮?? ??????
			stdcall insert_text, txt_but_mid
			stdcall get_code_children_objects, esi
			;????? ?㭪樨 button (?????? id=-1, ?????襭?? ?ணࠬ??)
			stdcall insert_text, txt_but_end

			call get_code_a_but_fun ;???? ?㭪権, ???뢠???? ??? ????⨨ ??????
		@@:
		test_bool_prop ebx,BIT_MOD_ACD
		jae @f
			call get_code_auto_data_el

			mov edx,C_CHB
			stdcall find_obj_in_file,edx ;eax - 㪠??⥫? ?? ????????? check_box
			cmp eax,0
			je .no_check
				stdcall insert_text, txt_cb_b
				call get_code_auto_data_el_cbox
				stdcall insert_text, txt_cb_e
			.no_check:

			call get_code_auto_data_dop

			mov edx,C_LIB
			stdcall find_obj_in_file,edx ;eax - 㪠??⥫? ?? ????????? ??????⥪?
			cmp eax,0
			je @f
				insert_nl
				stdcall insert_text, txt_head_i
				stdcall insert_text, txt_d_poi
				insert_nl
				stdcall insert_text, txt_head_l
				stdcall insert_text, txt_db
				stdcall insert_text, txt_str_beg
				stdcall insert_text, txt_sys_err
				stdcall insert_text, txt_str_end
				insert_nl

				call get_code_auto_data_libs_param
				stdcall insert_text, txt_ll_start
				call get_code_auto_data_libs_struc
				stdcall insert_text, txt_ll_end
				call get_code_auto_data_libs ;⠡???? ??????? ??????⥪
		@@:
		insert_nl
	popad
	ret
endp

align 4
proc get_code_auto_m
	pushad
	stdcall dword[tl_node_poi_get_info], tree2,0
	mov esi,eax
	.beg_iterat:
		cmp esi,0
		je .find_no

		stdcall dword[tl_node_poi_get_data], tree2,esi
		cmp eax,0
		je .find_no
		mov ebx,eax ;????砥? ?????? 㧫?

		cmp dword[ebx],C_EDB
		jne @f
			stdcall insert_text, txt_std_mouse_ed
			stdcall insert_text_prop, ebx,0
			insert_nl
		@@:
		cmp dword[ebx],C_CHB
		jne @f
			stdcall insert_text, txt_std_mouse_cb
			stdcall insert_text_prop, ebx,0
			insert_nl
		@@:

		stdcall dword[tl_node_poi_get_next_info], tree2,esi
		mov esi,eax ;????室?? ? ᫥??饬? 㧫?

		jmp .beg_iterat
	.find_no:
	popad
	ret
endp

;ᮧ????? ?????? (???????? ??ꥪ⮢)
align 4
proc get_code_auto_data_el
	pushad
	insert_nl ;?????? ??? ??ଠ??஢????

	stdcall dword[tl_node_poi_get_info], tree2,0
	mov esi,eax
	.beg_iterat:
		cmp esi,0
		je .find_no

		stdcall dword[tl_node_poi_get_data], tree2,esi
		cmp eax,0
		je .find_no
		mov ebx,eax ;????砥? ?????? 㧫?

		cmp dword[ebx],C_EDB
		jne @f
			;1,2,3,4,...,6,...8,...,10 - w,x,y,col_fon,...,col_frame,...,col_txt,...,txt
			stdcall insert_text_prop, ebx,0
			stdcall insert_text, txt_ebox

			mov ecx,1
			.prop:
				insert_sp
				stdcall insert_text_prop, ebx,ecx
				insert_coma
				inc ecx
				cmp ecx,10
			jl .prop

			insert_br
			stdcall insert_text, txt_txt

			mov edx,file_name
			stdcall get_obj_id_str, ebx,edx
			stdcall insert_text, edx

			stdcall insert_text, txt_mouse_dd

			stdcall insert_text, txt_0 ;᢮??⢠ editbox
			stdcall insert_bit_prop, ebx,BIT_MOD_EDIT_FOC
			stdcall insert_bit_prop, ebx,BIT_MOD_EDIT_FIO

			xchg eax,edx ;???࠭塞 ???祭?? edx
			stdcall get_obj_text_col,ebx,10
			cmp byte[edx],0 ;?஢??塞 ?????? ?? ??ப? ??? ????᪥ editbox
			je .null_string
				xchg eax,edx ;???⠭???????? ???祭?? edx
				stdcall insert_text, txt_coma

				stdcall insert_text, txt_txt
				stdcall insert_text, edx
				stdcall insert_text, txt_met_end
				stdcall insert_text, txt_minus
				stdcall insert_text, txt_txt
				stdcall insert_text, edx
				stdcall insert_text, txt_minus
				stdcall insert_text, txt_1
				stdcall insert_text, txt_coma

				stdcall insert_text, txt_txt
				stdcall insert_text, edx
				stdcall insert_text, txt_met_end
				stdcall insert_text, txt_minus
				stdcall insert_text, txt_txt
				stdcall insert_text, edx
				stdcall insert_text, txt_minus
				stdcall insert_text, txt_1
			.null_string:
			insert_nl
		@@:
		cmp dword[ebx],C_DTT
		jne @f
			stdcall insert_text_label_data, ebx,0
		@@:

		stdcall dword[tl_node_poi_get_next_info], tree2,esi
		mov esi,eax ;????室?? ? ᫥??饬? 㧫?

		jmp .beg_iterat
	.find_no:
	popad
	ret
endp

align 4
proc get_code_auto_data_el_cbox
	pushad
	insert_nl ;?????? ??? ??ଠ??஢????

	stdcall dword[tl_node_poi_get_info], tree2,0
	mov esi,eax
	.beg_iterat:
		cmp esi,0
		je .find_no

		stdcall dword[tl_node_poi_get_data], tree2,esi
		cmp eax,0
		je .find_no
		mov ebx,eax ;????砥? ?????? 㧫?

		cmp dword[ebx],C_CHB
		jne .no_check
			;1,2,3,4,5, 6,7,8, 9 - ot_x,ot_y,w,h,ot_w_txt, col_fon,col_frame,col_txt, txt

			stdcall insert_text_prop, ebx,0
			stdcall insert_text, txt_cbox
			insert_sp

			stdcall insert_2_prop_16_bit, ebx,1,3
			stdcall insert_text, txt_coma
			stdcall insert_2_prop_16_bit, ebx,2,4
			stdcall insert_text, txt_coma

			stdcall insert_text_prop, ebx,5
			stdcall insert_text, txt_coma
			insert_sp

			stdcall insert_text_prop, ebx,6
			stdcall insert_text, txt_coma
			stdcall insert_text_prop, ebx,7
			stdcall insert_text, txt_coma
			stdcall insert_text_prop, ebx,8
			stdcall insert_text, txt_coma
			stdcall insert_text, txt_br

			stdcall insert_text, txt_txt
			mov edx,file_name
			stdcall get_obj_id_str, ebx,edx
			stdcall insert_text, edx
			stdcall insert_text, txt_coma

			;䫠??
			stdcall insert_text,txt_0
			stdcall insert_bit_prop, ebx,BIT_MOD_CHE
			stdcall insert_bit_prop, ebx,BIT_MOD_CHE_T
			stdcall insert_bit_prop, ebx,BIT_MOD_CHE_M
			stdcall insert_bit_prop, ebx,BIT_MOD_CHE_B

			insert_nl
		.no_check:
		cmp dword[ebx],C_DTT
		jne @f
			stdcall insert_text_label_data, ebx,0
		@@:

		stdcall dword[tl_node_poi_get_next_info], tree2,esi
		mov esi,eax ;????室?? ? ᫥??饬? 㧫?

		jmp .beg_iterat
	.find_no:
	popad
	ret
endp

;?????? ??? ??????⥪
align 4
proc get_code_auto_data_libs_param
	pushad
	insert_nl ;?????? ??? ??ଠ??஢????

	stdcall dword[tl_node_poi_get_info], tree2,0
	mov esi,eax
	.beg_iterat:
		cmp esi,0
		je .find_no

		stdcall dword[tl_node_poi_get_data], tree2,esi
		cmp eax,0
		je .find_no
		mov ebx,eax ;????砥? ?????? 㧫?

		cmp dword[ebx],C_LIB
		jne @f
			;0,1,2,3,4,5 - l_nam,s_dir,i_met,...

			stdcall insert_text, txt_sys_dir
			;object code
			mov edx,file_name
			stdcall get_obj_id_str, ebx,edx
			stdcall insert_text,edx
			stdcall insert_text,txt_db
			;stdcall insert_text, txt_str_beg
			stdcall insert_text_prop, ebx,1
			;stdcall insert_text, txt_str_end
			insert_nl

			stdcall insert_text, txt_lib_name
			stdcall insert_text,edx
			stdcall insert_text,txt_db
			stdcall insert_text, txt_str_beg
			stdcall insert_text_prop, ebx,0
			stdcall insert_text, txt_str_end
			insert_nl

			stdcall insert_text,txt_err_m_f_lib
			stdcall insert_text,edx
			stdcall insert_text,txt_db
			stdcall insert_text, txt_str_beg
			stdcall insert_text_prop, ebx,4
			stdcall insert_text, txt_str_end
			insert_nl

			stdcall insert_text,txt_err_m_i_lib
			stdcall insert_text,edx
			stdcall insert_text,txt_db
			stdcall insert_text, txt_str_beg
			stdcall insert_text_prop, ebx,5
			stdcall insert_text, txt_str_end
			insert_nl

			insert_nl
		@@:

		stdcall dword[tl_node_poi_get_next_info], tree2,esi
		mov esi,eax ;????室?? ? ᫥??饬? 㧫?
		jmp .beg_iterat
	.find_no:
	popad
	ret
endp

;?????? ??? ??????⥪
align 4
proc get_code_auto_data_libs_struc
	pushad
	;insert_nl ;?????? ??? ??ଠ??஢????

	stdcall dword[tl_node_poi_get_info], tree2,0
	mov esi,eax
	.beg_iterat:
		cmp esi,0
		je .find_no

		stdcall dword[tl_node_poi_get_data], tree2,esi
		cmp eax,0
		je .find_no
		mov ebx,eax ;????砥? ?????? 㧫?
		cmp dword[ebx],C_LIB
		jne @f
			;0,1,2,3,4,5 - l_nam,s_dir,i_met,...

			stdcall insert_text, txt_lib
			;object code
			mov edx,file_name
			stdcall get_obj_id_str, ebx,edx
			stdcall insert_text,edx
			insert_sp
			stdcall insert_text, txt_ll
			stdcall insert_text, txt_lib_name
			stdcall insert_text,edx
			stdcall insert_text,txt_coma

			stdcall insert_text, txt_lib_path

			stdcall insert_text, txt_sys_dir
			stdcall insert_text,edx
			stdcall insert_text,txt_coma
			stdcall insert_text,txt_br
			stdcall insert_text,txt_tab

			stdcall insert_text,txt_err_m_f_lib
			stdcall insert_text,edx
			stdcall insert_text,txt_coma
			stdcall insert_text,txt_head_l
			stdcall insert_text,txt_coma

			stdcall insert_text_prop, ebx,2
			stdcall insert_text,txt_coma

			stdcall insert_text,txt_err_m_i_lib
			stdcall insert_text,edx
			stdcall insert_text,txt_coma
			stdcall insert_text,txt_head_i

			insert_nl
		@@:

		stdcall dword[tl_node_poi_get_next_info], tree2,esi
		mov esi,eax ;????室?? ? ᫥??饬? 㧫?

		jmp .beg_iterat
	.find_no:
	popad
	ret
endp

;?????? ??? ??????⥪
align 4
proc get_code_auto_data_libs
	pushad
	insert_nl ;?????? ??? ??ଠ??஢????

	stdcall dword[tl_node_poi_get_info], tree2,0
	mov esi,eax
	.beg_iterat:
		cmp esi,0
		je .find_no

		stdcall dword[tl_node_poi_get_data], tree2,esi
		cmp eax,0
		je .find_no
		mov ebx,eax ;????砥? ?????? 㧫?
		cmp dword[ebx],C_LIB
		jne @f
			;0,1,2,3,4,5 - l_nam,s_dir,i_met,...
			stdcall insert_text, txt_align_4
			stdcall insert_text_prop, ebx,2
			stdcall insert_text, txt_d_poi
			insert_nl

			call get_code_lib_table
		@@:

		stdcall dword[tl_node_poi_get_next_info], tree2,esi
		mov esi,eax ;????室?? ? ᫥??饬? 㧫?

		jmp .beg_iterat
	.find_no:
	popad
	ret
endp

;input:
; esi
align 4
get_code_lib_table:
pushad
	mov bl,byte[esi+2]
	mov edi,esi
	stdcall dword[tl_node_poi_get_data], tree2,esi
	mov edx,eax ;????祭?? ?????? த?⥫?᪮?? ??ꥪ??
	cmp eax,0
	je @f
	@@:
		stdcall dword[tl_node_poi_get_next_info], tree2,esi
		mov esi,eax ;????室?? ? ᫥??饬? 㧫?
		cmp esi,0
		je @f
		cmp byte[esi+2],bl ;lev
		jle @f
			stdcall dword[tl_node_poi_get_data], tree2,esi
			cmp eax,0
			je @f
			cmp dword[eax],C_IFN
			jne @b
				mov ecx,eax ;????砥? ?????? 㧫?
				;??⠢?? 㪠??⥫? ?? ?㭪???
				test_bool_prop ecx,BIT_MOD_IMPORT_FUNCT_COMMENT
				jae .no_com_0
					stdcall insert_text, txt_com
				.no_com_0:
				insert_tab
				stdcall insert_text_prop, ecx,0
				stdcall insert_text, txt_dd
				stdcall insert_text_prop, edx,3 ;????⠢?? ? ?㭪樨
				stdcall insert_text_prop, ecx,0
				insert_nl
			jmp @b
	@@:
	stdcall insert_text, txt_dd_0_0
	mov esi,edi
	@@:
		stdcall dword[tl_node_poi_get_next_info], tree2,esi
		mov esi,eax ;????室?? ? ᫥??饬? 㧫?
		cmp esi,0
		je @f
		cmp byte[esi+2],bl ;lev
		jle @f
			stdcall dword[tl_node_poi_get_data], tree2,esi
			cmp eax,0
			je @f
			cmp dword[eax],C_IFN
			jne @b
				mov ecx,eax ;????砥? ?????? 㧫?
				;??⠢?? ????? ???????஢????? ?㭪樨
				test_bool_prop ecx,BIT_MOD_IMPORT_FUNCT_COMMENT
				jae .no_com_1
					stdcall insert_text, txt_com
				.no_com_1:
				insert_tab
				stdcall insert_text_prop, edx,3 ;????⠢?? ? ?㭪樨
				stdcall insert_text_prop, ecx,0
				stdcall insert_text, txt_db
				stdcall insert_text, txt_str_beg
				stdcall insert_text_prop, ecx,1
				stdcall insert_text, txt_str_end
				insert_nl
			jmp @b
	@@:
popad
	ret

;
align 4
proc get_code_auto_data_dop
	pushad
	insert_nl ;?????? ??? ??ଠ??஢????

	stdcall dword[tl_node_poi_get_info], tree2,0
	mov esi,eax
	.beg_iterat:
		cmp esi,0
		je .find_no

		stdcall dword[tl_node_poi_get_data], tree2,esi
		cmp eax,0
		je .find_no
		mov ebx,eax ;????砥? ?????? 㧫?

		cmp dword[ebx],C_EDB
		jne @f
			;1,2,3,4,...,6,...8,...,10 - w,x,y,col_fon,...,col_frame,...,col_txt,...,txt

			;??ନ?㥬 ⥪?⮢?? ?????? ??? editbox-??
			stdcall insert_text, txt_txt
			mov edx,file_name
			stdcall get_obj_id_str, ebx,edx
			stdcall insert_text,edx

			stdcall get_obj_text_col,ebx,10
			cmp byte[edx],0
			je .no_text
				stdcall insert_text, txt_d_poi
				stdcall insert_text, txt_db
				stdcall insert_text, txt_str_beg
				stdcall str_len,edx
				stdcall [ted_text_add], tedit0,edx,eax,ted_opt_ed_move_cursor
				stdcall insert_text, txt_str_end
				insert_nl

				insert_tab
				stdcall insert_text, txt_met_end
				stdcall insert_text, txt_rb
				stdcall insert_text, txt_txt
				mov edx,file_name
				stdcall get_obj_id_str, ebx,edx
				stdcall insert_text,edx

				stdcall insert_text, txt_minus
				stdcall insert_text, txt_txt
				stdcall insert_text,edx
				stdcall insert_text, txt_met_end

				insert_plus
				stdcall insert_text_prop,ebx,9
				insert_plus
				stdcall insert_text, txt_3
				
				insert_nl

				jmp @f

			.no_text: ;??ࢮ??砫쭮 ⥪?? ?? ?????
				stdcall insert_text, txt_times

				stdcall insert_text_prop,ebx,9
				insert_plus
				stdcall insert_text, txt_2
				stdcall insert_text, txt_db
				insert_0
				insert_nl
		@@:
		cmp dword[ebx],C_CHB
		jne @f
			stdcall insert_text_label_data, ebx,9
		@@:
		stdcall dword[tl_node_poi_get_next_info], tree2,esi
		mov esi,eax ;????室?? ? ᫥??饬? 㧫?

		jmp .beg_iterat
	.find_no:
	popad
	ret
endp

;????뢠?? ?????? ? ?㭪??ﬨ
align 4
proc get_code_auto_but
	pushad
	stdcall dword[tl_node_poi_get_info], tree2,0
	mov esi,eax
	.beg_iterat:
		cmp esi,0
		je .find_no

		stdcall dword[tl_node_poi_get_data], tree2,esi
		cmp eax,0
		je .find_no
		mov ebx,eax ;????砥? ?????? 㧫?

		cmp dword[ebx],C_BUT
		jne @f
			insert_tab
			insert_cmp
			insert_reg8 txt_ah
			insert_coma
			stdcall get_obj_text_col,ebx,0
			stdcall str_len,edx
			stdcall [ted_text_add], tedit0,edx,eax,ted_opt_ed_move_cursor
			insert_nl
			insert_tab
			stdcall insert_text, txt_jne_f
			insert_nl
			insert_tab
			insert_tab
			stdcall insert_text, txt_call
			stdcall get_obj_text_col,ebx,6
			stdcall str_len,edx
			cmp eax,1
			jl .c_fun
				stdcall [ted_text_add], tedit0,edx,eax,ted_opt_ed_move_cursor
			.c_fun:
			insert_nl
			insert_tab
			insert_a_met ;@@:
			insert_nl
		@@:

		stdcall dword[tl_node_poi_get_next_info], tree2,esi
		mov esi,eax ;????室?? ? ᫥??饬? 㧫?

		jmp .beg_iterat
	.find_no:
	popad
	ret
endp

;??६ ???? ??? ?㭪権 ????⨩ ?? ??????
align 4
proc get_code_a_but_fun
	pushad
	stdcall dword[tl_node_poi_get_info], tree2,0
	mov esi,eax
	.beg_iterat:
		cmp esi,0
		je .find_no

		stdcall dword[tl_node_poi_get_data], tree2,esi
		cmp eax,0
		je .find_no
		mov ebx,eax ;????砥? ?????? 㧫?

		cmp dword[ebx],C_BUT
		jne @f
			stdcall get_obj_text_col,ebx,6
			stdcall str_len,edx
			cmp eax,1
			jl .c_fun
				insert_nl
				stdcall insert_text, txt_align_4
				stdcall [ted_text_add], tedit0,edx,eax,ted_opt_ed_move_cursor
				insert_d_poi
				insert_nl
				insert_tab
				insert_ret
			.c_fun:
		@@:

		stdcall dword[tl_node_poi_get_next_info], tree2,esi
		mov esi,eax ;????室?? ? ᫥??饬? 㧫?

		jmp .beg_iterat
	.find_no:
	popad
	ret
endp

align 4
proc get_code_key, p_obj:dword
	pushad
		insert_nl
		mov ebx,dword[p_obj]

		insert_key_beg
		call get_code_funct_key ;???? ?맮?? ??????
		insert_key_end

		insert_nl
	popad
	ret
endp

align 4
proc get_code_funct_key
	pushad
	stdcall dword[tl_node_poi_get_info], tree2,0
	mov esi,eax
	.beg_iterat:
		cmp esi,0
		je .find_no

		stdcall dword[tl_node_poi_get_data], tree2,esi
		cmp eax,0
		je .find_no

		mov ebx,eax ;????砥? ?????? 㧫?
		cmp dword[ebx],C_EDB
		jne @f
			stdcall insert_text, txt_std_key_ed
			stdcall insert_text_prop,ebx,0
			insert_nl
		@@:

		stdcall dword[tl_node_poi_get_next_info], tree2,esi
		mov esi,eax ;????室?? ? ᫥??饬? 㧫?

		jmp .beg_iterat
	.find_no:
	popad
	ret
endp

;??ꥪ?? ??????祭?? 䠩??? 'include ...'
align 4
proc get_code_incl, p_obj:dword
	pushad
		stdcall insert_text, txt_incl
		mov ebx,dword[p_obj]
		stdcall insert_text_prop,ebx,0

		stdcall get_obj_text_col,ebx,1
		stdcall str_len,edx
		cmp eax,1
		jl @f
			insert_sp
			insert_com
			stdcall [ted_text_add], tedit0,edx,eax,ted_opt_ed_move_cursor
		@@:

		insert_nl
	popad
	ret
endp

;ᮧ???? ??? ??????? ????
align 4
proc get_code_window, p_obj:dword
	pushad
		mov ebx,dword[p_obj]
		;2,3,4,5,6 - x,y,w,h,col_bkg

		stdcall insert_text, txt_draw_wb

		insert_tab
		insert_xor
		insert_reg txt_eax
		insert_coma
		insert_reg txt_eax
		insert_nl

		;mov ebx,... ???????? ??ਭ? ???? ? ?????? ?? ?????? ???? ??࠭?
		insert_tab
		stdcall insert_text, txt_mov
		insert_reg txt_ebx
		insert_coma
		stdcall insert_2_prop_16_bit, ebx,2,4
		insert_nl

		;mov ecx,... ???????? ?????? ???? ? ?????? ?? ???孥?? ???? ??࠭?
		insert_tab
		stdcall insert_text, txt_mov
		insert_reg txt_ecx
		insert_coma
		stdcall insert_2_prop_16_bit, ebx,3,5
		insert_nl

		;mov edx,... ???????? 梥? ࠡ?祩 ??????? ????
		insert_tab
		stdcall insert_text, txt_mov
		insert_reg txt_edx
		insert_coma
		stdcall insert_text_prop, ebx,6
		insert_nl

		;or edx,0x33000000
		insert_tab
		stdcall insert_text, txt_or
		insert_reg txt_edx
		insert_coma
		stdcall insert_text,txt_skob_o
		stdcall insert_text_prop, ebx,1 ;?⨫? ????
		stdcall insert_text,txt_shl_24
		stdcall insert_text,txt_skob_c
		stdcall insert_bit_prop, ebx,BIT_MOD_WI_CAPT
		stdcall insert_bit_prop, ebx,BIT_MOD_WI_CORD_OTN_CL_OBL
		stdcall insert_bit_prop, ebx,BIT_MOD_WI_REDR
		stdcall insert_bit_prop, ebx,BIT_MOD_WI_GRAD
		insert_nl

		;mov edi,...
		insert_tab
		stdcall insert_text, txt_mov
		insert_reg txt_edi
		insert_coma
		stdcall insert_text_prop, ebx,0
		insert_nl

		insert_tab
		stdcall insert_text, txt_i_40
		insert_nl

		call get_code_draw_obj
	popad
	ret
endp


;??ନ?㥬 ?맮? ?㭪権 ??? ??ᮢ???? ??ꥪ⮢
align 4
proc get_code_draw_obj
	pushad
	stdcall dword[tl_node_poi_get_info], tree2,0
	mov esi,eax
	.beg_iterat:
		cmp esi,0
		je .find_no

		stdcall dword[tl_node_poi_get_data], tree2,esi
		cmp eax,0
		je .find_no
		mov ebx,eax ;????砥? ?????? 㧫?

		cmp dword[ebx],C_EDB
		jne @f
			stdcall insert_text, txt_editb_draw
			stdcall insert_text_prop, ebx,0
			insert_nl
		@@:
		cmp dword[ebx],C_CHB
		jne @f
			stdcall insert_text, txt_cbox_draw
			stdcall insert_text_prop, ebx,0
			insert_nl
		@@:
		cmp dword[ebx],C_BUT
		jne .ob_draw_but
			insert_tab
			stdcall insert_text, txt_mov
			insert_reg txt_eax
			insert_coma
			stdcall insert_text, txt_8
			insert_nl

			insert_tab
			stdcall insert_text, txt_mov
			insert_reg txt_ebx
			insert_coma
			stdcall insert_2_prop_16_bit, ebx,1,3
			insert_nl

			insert_tab
			stdcall insert_text, txt_mov
			insert_reg txt_ecx
			insert_coma
			stdcall insert_2_prop_16_bit, ebx,2,4
			insert_nl

			insert_tab
			stdcall insert_text, txt_mov
			insert_reg txt_edx
			insert_coma
			stdcall insert_text_prop, ebx,0
			test_bool_prop ebx,BIT_MOD_BUT_NFON
			jnc @f
				stdcall insert_text, txt_plus
				stdcall insert_text, txt_bit_30
			@@:
			test_bool_prop ebx,BIT_MOD_BUT_NBORD
			jnc @f
				stdcall insert_text, txt_plus
				stdcall insert_text, txt_bit_29
			@@:
			insert_nl

			insert_tab
			stdcall insert_text, txt_mov
			insert_reg txt_esi
			insert_coma
			stdcall insert_text_prop, ebx,5 ;button color
			insert_nl

			insert_tab
			stdcall insert_text, txt_i_40
			insert_nl
		.ob_draw_but:
		cmp dword[ebx],C_REC
		jne @f
			insert_tab
			stdcall insert_text, txt_mov
			insert_reg txt_eax
			insert_coma
			stdcall insert_text, txt_1
			stdcall insert_text, txt_3
			insert_nl

			insert_tab
			stdcall insert_text, txt_mov
			insert_reg txt_ebx
			insert_coma
			stdcall insert_2_prop_16_bit, ebx,1,3
			insert_nl

			insert_tab
			stdcall insert_text, txt_mov
			insert_reg txt_ecx
			insert_coma
			stdcall insert_2_prop_16_bit, ebx,2,4
			insert_nl

			insert_tab
			stdcall insert_text, txt_mov
			insert_reg txt_edx
			insert_coma
			stdcall insert_text_prop, ebx,0 ;rect color
			insert_nl

			insert_tab
			stdcall insert_text, txt_i_40
			insert_nl
		@@:
		cmp dword[ebx],C_DTT ;??ᮢ???? ⥪??? ??१ ????. ?. 4
		jne .ob_draw_text
			insert_tab
			stdcall insert_text, txt_mov
			insert_reg txt_eax
			insert_coma
			stdcall insert_text, txt_4
			insert_nl

			insert_tab
			stdcall insert_text, txt_mov
			insert_reg txt_ebx
			insert_coma
			stdcall insert_2_prop_16_bit, ebx,1,2
			insert_nl

			insert_tab
			stdcall insert_text, txt_mov
			insert_reg txt_ecx
			insert_coma
			stdcall insert_text_prop, ebx,3
			stdcall insert_bit_prop, ebx,BIT_MOD_TXT_ASCII_0 ;31 - ??? ??? ??ப? ASCIIZ
			stdcall insert_bit_prop, ebx,BIT_MOD_TXT_NO_TRAN ;30 - ??? ??? ?? ?஧??筮?? ⥪???
			stdcall insert_bit_prop, ebx,BIT_MOD_TXT_CHAR2 ;28 - ??? ??? 2-?? ????. ??????
			insert_nl

			insert_tab
			stdcall insert_text, txt_mov
			insert_reg txt_edx
			insert_coma
			stdcall insert_text, txt_txt
			mov edx,file_name
			stdcall get_obj_id_str, ebx,edx
			stdcall insert_text,edx
			insert_nl

;mov esi,...
			test_bool_prop ebx,BIT_MOD_TXT_NO_TRAN
			jnc @f
				insert_tab
				stdcall insert_text, txt_mov
				insert_reg txt_edi
				insert_coma
				stdcall insert_text_prop, ebx,4 ;梥? 䮭?
				insert_nl
			@@:

			insert_tab
			stdcall insert_text, txt_i_40
			insert_nl
		.ob_draw_text:

		stdcall dword[tl_node_poi_get_next_info], tree2,esi
		mov esi,eax ;????室?? ? ᫥??饬? 㧫?

		jmp .beg_iterat
	.find_no:
	popad
	ret
endp

;ᮧ???? ??? ??????? ??६?????
align 4
proc get_code_variable, p_obj:dword,o_lev:dword
	pushad
		mov ebx,dword[p_obj]
		;0,1,2 - name,value,comment

		mov ecx,dword[o_lev]
		cmp cl,0
		je .no_tab ;???????? ??横??????? (0-1 = 2^32-1)
		and ecx,0xff
		cld
		@@:
			insert_tab ;??ନ஢???? ????㯠 ??। ??६?????
		loop @b
		.no_tab:

		stdcall insert_text_prop, ebx,0 ;??? ??६?????
		insert_sp

		mov ecx,dword[ebx+u_object.typid-u_object] ;ecx - ?⭮??⥫???? ?????? த?⥫?᪮?? ??ꥪ?? (? ??????)
		xor eax,eax
		mov ax,[tree2.info_size]
		imul ecx,eax ;㬭????? ?????? ?? ࠧ??? ?????????
		add ecx,[tree2.data_info] ;ecx - 㪠??⥫? ?????? த?⥫?᪮?? 㧫?

		;?஢???? ?? ?ࠢ??쭮??? ecx ???

		stdcall insert_text_prop, ecx,0 ;⨯ ??६?????
		insert_sp
		stdcall insert_text_prop, ebx,1 ;???祭??

		stdcall get_obj_text_col,ebx,2 ;????????਩
		stdcall str_len,edx
		cmp eax,1
		jl @f
			insert_sp
			insert_com
			stdcall [ted_text_add], tedit0,edx,eax,ted_opt_ed_move_cursor
		@@:
		insert_nl

	popad
	ret
endp



;--- ⥪?⮢?? ????⠭?? ---
txt_str_beg db 39,0
txt_str_end db 39,',0',0
txt_coma db ',',0
txt_skob_o db '(',0
txt_skob_c db ')',0
txt_skob_fo db '{',0 ;᪮??? 䨣?ୠ? ????뢠????
txt_skob_fc db '}',0 ;᪮??? 䨣?ୠ? ????뢠????
txt_i_40 db 'int 0x40',13,0
txt_0 db '0',0
txt_1 db '1',0
txt_2 db '2',0
txt_3 db '3',0
txt_4 db '4',0
txt_8 db '8',0
txt_plus db '+',0
txt_minus db '-',0
;txt_dol db '$',0
txt_nl db 13,0
txt_com db ';',0
txt_sp db ' ',0
txt_tab db 9,0
txt_br db '\',13,9,0
txt_txt db 'txt_',0
txt_db db ' db ',0
txt_dd db ' dd ',0
txt_rb db ' rb ',0
txt_dd_0_0 db 9,'dd 0,0',13,0
txt_mouse_dd db ', mouse_dd, ',0
txt_times db ':',13,9,'times ',0
txt_draw_wb:
	db 13,'align 4',13,'draw_window:',13,'pushad',13,9,'mcall 12,1',13,0
txt_draw_we:
	db 9,'mcall 12,2',13,'popad',13,9,'ret',13,0
txt_sys_dir db 'system_dir_',0
txt_lib_name db 'lib_name_',0
txt_lib_path db ' sys_path, library_path, ',0
txt_sys_err db '???⥬??? ?訡??',0
txt_ll_start db 'l_libs_start:',13,0
txt_lib db 9,'lib_',0
txt_ll db 'l_libs ',0
txt_err_m_f_lib db 'err_msg_found_lib_',0
txt_head_l db 'head_f_l',0
txt_err_m_i_lib db 'err_msg_import_',0
txt_head_i db 'head_f_i',0
txt_ll_end db 'l_libs_end:',13,0
;txt_bit_28 db '0x10000000',0
txt_bit_29 db '0x20000000',0
txt_bit_30 db '0x40000000',0
;txt_bit_31 db '0x80000000',0
txt_struc db 'struc',0
txt_t db 't',0
txt_met_end db '.end',0
txt_ends db 'ends',13,13,0
txt_mov db 'mov ',0
txt_xor db 'xor ',0
txt_or db 'or  ',0
txt_cmp db 'cmp ',0
txt_call db 'call ',0
txt_jne_f db 'jne @f',0
txt_a_met db '@@:',0
txt_ah db 'ah',0
txt_eax db 'eax',0
txt_ebx db 'ebx',0
txt_ecx db 'ecx',0
txt_edx db 'edx',0
txt_edi db 'edi',0
txt_esi db 'esi',0
txt_align_4 db 'align 4',13,0
txt_d_poi db ':',0
txt_ret db 'ret',13,0
txt_shl_16 db ' shl 16',0
txt_shl_24 db ' shl 24',0
txt_std_mouse_ed:
	db 9,'stdcall [edit_box_mouse], ',0
txt_std_init_cb:
	db 9,'init_checkboxes2 check_boxes,check_boxes_end',13,0
txt_std_mouse_cb:
	db 9,'stdcall [check_box_mouse], ',0
txt_std_key_ed:
	db 9,'stdcall [edit_box_key], ',0
txt_incl:
	db 'include ',0

txt_cbox db ' check_box2',0
txt_cb_b db 13,'check_boxes:',0
txt_cb_e db 'check_boxes_end:',13,0

txt_ebox db ' edit_box',0
txt_editb_draw db 9,'stdcall [edit_box_draw], ',0
txt_cbox_draw db 9,'stdcall [check_box_draw], ',0

;??砫? ?㭪樨 ??????
txt_but_beg db 'align 4',13,'button:',13,9,'mcall 17',13,0
txt_but_mid db 9,'cmp ah,1',13,9,'jne still',13,'.exit:',13,0
;????? ?㭪樨 ?????? (?????襭?? ?ணࠬ?)
txt_but_end db 9,'mcall -1',13,0

txt_key_1:
db 'align 4',13,'key:',13,9,'mcall 2',13
txt_key_2:
db 9,'jmp still',13
txt_key_3:

;????砥? ???浪???? ????? ??ꥪ?? ?⭮??⥫쭮 ?????? ???????? tree2
;output:
; txt_buf - ??ப? ? ????஬ ??ꥪ??
align 4
proc get_obj_id_str, p_obj:dword, txt_buf:dword
	push eax ecx edx
	mov eax,dword[p_obj]
	sub eax,[tree2.data_info]
	xor edx,edx
	xor ecx,ecx
	mov cx,word[tree2.info_size]
	div ecx

	mov edx,dword[txt_buf]
	mov byte[edx],0
	stdcall str_cat_value,edx,eax
	pop edx ecx eax
	ret
endp

;????砥? ???浪???? ????? ??ꥪ?? ?⭮??⥫쭮 ?????? ???????? tree2
;output:
; eax - ????? ??ꥪ??
align 4
proc get_obj_id, p_obj:dword
	push ecx edx
	mov eax,dword[p_obj]
	sub eax,[tree2.data_info]
	xor edx,edx
	xor ecx,ecx
	mov cx,word[tree2.info_size]
	div ecx
	pop edx ecx
	ret
endp

;--- ⥪?⮢?? ?㭪樨 ---

;?롨ࠥ? ??ப? ?? ??ᨢ?, ?? 㪠??????? ???????
;input:
; str - 㪠??⥫? ?? ??ᨢ ??ப
; ind - ?????? ?᪮??? ??ப?
;output:
; edi - 㪠??⥫? ?? ???????? ??ப?
align 4
proc get_string_by_ind, str:dword, ind:dword
push eax ebx ecx
	mov edi,[str]
	mov ebx,[ind]
	xor al,al ;0 - ᨬ??? ????? ??ப?
	@@:
		cmp ebx,0
		jle @f ;?᫨ ??????? ??ப? ? 㪠?????? ?????ᮬ
		xor ecx,ecx
		dec ecx ;ecx - ??᫮ ???ᬠ?ਢ????? ????
		cld
		repne scasb ;?饬 ????? ??ப? (al=0)
		cmp byte[edi],0
		je @f ;?᫨ ᫥???饩 ??ப? ??? ??室 ?? 横??
		dec ebx
		jmp @b
	@@:
pop ecx ebx eax
	ret
endp

align 4
proc mem_clear, mem:dword, len:dword
	push eax ecx edi
	cld
	xor al,al
	mov edi, dword[mem]
	mov ecx, dword[len]
	repne stosb
	pop edi ecx eax
	ret
endp

align 4
proc str_cat, str1:dword, str2:dword
	push eax ecx edi esi
	mov esi,dword[str2]
	stdcall str_len,esi
	mov ecx,eax
	inc ecx
	mov edi,dword[str1]
	stdcall str_len,edi
	add edi,eax
	cld
	repne movsb
	pop esi edi ecx eax
	ret
endp

align 4
proc str_cat_value, str1:dword, value:dword
	pushad
	mov edi,[str1]
	stdcall str_len,edi
	add edi,eax
	mov eax,[value]
	call tl_convert_to_str
	popad
	ret
endp

;output:
; eax = strlen
align 4
proc str_len, str1:dword
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

;input:
; eax = value
; edi = string buffer
align 4
tl_convert_to_str:
	pushad
	stdcall mem_clear,edi,12
	call .str
	popad
	ret

align 4
.str:
	mov ecx,0x0a ;???????? ???⥬? ???᫥??? ?????????? ॣ????? ebx,eax,ecx,edx ?室??? ??ࠬ???? eax - ??᫮
		;???ॢ?? ??᫠ ? ASCII ??ப? ?室??? ?????? ecx=???⥬? ???᫥?? edi ????? ?㤠 ?????뢠??, ?㤥? ??ப?, ???祬 ????? ??६????? 
	cmp eax,ecx  ;?ࠢ???? ?᫨ ? eax ?????? 祬 ? ecx ?? ??३?? ?? @@-1 ?.?. ?? pop eax
	jb @f
		xor edx,edx ;???????? edx
		div ecx     ;ࠧ?????? - ????⮪ ? edx
		push edx    ;???????? ? ?⥪
		;dec edi    ;ᬥ饭?? ????室???? ??? ?????? ? ????? ??ப?
		call .str   ;??३?? ?? ᠬ? ᥡ? ?.?. ?맢??? ᠬ? ᥡ? ? ⠪ ?? ⮣? ??????? ???? ? eax ?? ?⠭?? ?????? 祬 ? ecx
		pop eax
	@@: ;cmp al,10 ;?஢????? ?? ?????? ?? ???祭?? ? al 祬 10 (??? ???⥬? ???᫥?? 10 ?????? ??????? - ??譠?))
	or al,0x30  ;?????? ??????? ??????  祬 ??? ???? 
	stosb	    ;???????? ??????? ?? ॣ????? al ? ?祪? ?????? es:edi
	ret