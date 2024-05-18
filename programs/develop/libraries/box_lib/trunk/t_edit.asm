; ����� ��� ��⥬��� ������⥪� box_lib.obj
; ����� TextEditor ��� KolibriOS
; 䠩� ��᫥���� ࠧ �������� 12.01.2021 IgorA
; �� ��� �ਬ����� GPL2 ��業���

;input:
; edi = pointer to tedit struct
; reg = index
;output:
; reg = pointer to 'tex' struct
macro ConvertIndexToPointer reg {
	imul reg,sizeof.symbol
	add reg,ted_tex
}

;--- out_reg = ted_key_words_data[ind_reg].Text[0] ---
macro ColToIndexOffset ind_reg,out_reg {
	mov out_reg,ind_reg
	imul out_reg,sizeof.TexColViv
	add out_reg,ted_key_words_data
}

TED_LINES_IN_NEW_FILE equ 30 ;�᫮ ��ப � ����� 䠩��
MAX_COLOR_WORD_LEN equ 40
;------------------------------------------------------------------------------
struct TexSelect
	x0 dd ?
	y0 dd ?
	x1 dd ?
	y1 dd ?
ends

struct TexColViv
	Text  rb MAX_COLOR_WORD_LEN ; ᫮�� ��� ���ᢥ⪨
	f1    dd 0 ; �ࠢ�� �� ᫮��
	flags db ? ; f1+4 䫠�� �ᯮ��㥬� �� �뤥�����
	endc  db ? ; f1+5 ᨬ��� ���� �뤥����� (�ᯮ������ �� flags&4)
	escc  db ? ; f1+6 �࠭����騩 ᨬ��� (�ᯮ������ �� flags&4)
	color db ? ; f1+7 ����� 梥�
ends

struct symbol
	c db ?    ;  +0 ᨬ���
	col db ?  ;  +1 梥�
	perv dd ? ;  +2
	next dd ? ;  +6 㪠��⥫�
	tc dd ?   ; +10 �६. ᮧ�����
	td dd ?   ; +14 �६. 㤠�����
ends
;------------------------------------------------------------------------------

ted_symbol_space db 32 ;ascii ��� �஡���, ������ �뢠�� �㦥� � ����
ted_symbol_tab db 26 ;ascii ��� ��५�� ��ࠢ�, �ᯮ������ ��� �ᮢ���� ⠡��樨 � ०��� ������ ��������� ᨬ�����

if lang eq ru

txtRow db '��ப�',0
txtCol db '����',0
txtOtm db '�⬥��',0
txtBuf db '����:',0

else

txtRow db 'Rows',0
txtCol db 'Cols',0
txtOtm db 'Undo',0
txtBuf db 'Buffer:',0

end if

;EvChar - ⠡��� ��� 䨫��஢���� ������塞�� ᨬ�����, ��-�� �� ������ ��譨� �����
align 16
EvChar db 0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0
    db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

KM_SHIFT equ 0x00010000
KM_CTRL equ 0x00020000
KM_ALT equ 0x00040000
KM_NUMLOCK equ 0x00080000

; KEY CODES
KEY_F1 equ 0x0000003B
KEY_F2 equ 0x0000003C
KEY_F3 equ 0x0000003D



align 16
proc ted_init uses eax ecx edi, edit:dword
	mov edi,[edit]

	mov ecx,sizeof.symbol
	imul ecx,ted_max_chars
	invoke mem.alloc,ecx ;�뤥�塞 ������
	mov ted_tex,eax
	mov ted_tex_1,eax
	add ted_tex_1,sizeof.symbol
	add eax,ecx
	mov ted_tex_end,eax

	stdcall ted_clear, edi,1

;-------------------------------------------------
	mov ecx,1024 ;1024 - ��� ���ᨢ� ted_arr_key_pos
	add ecx,ted_syntax_file_size
	invoke mem.alloc,ecx
	mov ted_arr_key_pos,eax
	add eax,1024
	mov ted_syntax_file,eax

	stdcall ted_init_scroll_bars,edi,3
	ret
endp

MIN_W_SCRL_ARE equ 3 ;�������쭠� �⮡ࠦ����� ������� ��� ����. �஫�����
MIN_H_SCRL_ARE equ 3 ;�������쭠� �⮡ࠦ����� ������� ��� ��ਧ. �஫�����
;input:
; opt = 1 - ������ 梥� �஫������, 2 - ���������� ࠧ���� ����,
;  4 - ���������� ࠧ���� ���㬥��
align 16
proc ted_init_scroll_bars, edit:dword, opt:dword
	pushad
	mov edi,[edit]
	mov esi,ted_scr_w
	mov ebx,ted_scr_h
	bt dword[opt],0
	jae @f
		mov ecx,ted_color_wnd_work
		mov dword[esi+sb_offs_bckg_col],ecx
		mov dword[ebx+sb_offs_bckg_col],ecx
		mov ecx,ted_color_wnd_capt
		mov dword[esi+sb_offs_frnt_col],ecx
		mov dword[ebx+sb_offs_frnt_col],ecx
		mov ecx,ted_color_wnd_bord
		mov dword[esi+sb_offs_line_col],ecx
		mov dword[ebx+sb_offs_line_col],ecx
	@@:
	bt dword[opt],2 ; ���������� ࠧ���� ���㬥�� ?
	jae .doc_resize
		call ted_get_num_lines
		cmp eax,TED_LINES_IN_NEW_FILE
		jge @f
			mov eax,TED_LINES_IN_NEW_FILE
		@@:
		mov dword[esi+sb_offs_max_area],eax
	.doc_resize:
	bt dword[opt],1 ; ���������� ࠧ���� ���� ?
	jae .no_size
			mov edx,ted_wnd_l
			add edx,ted_rec_l
			mov word[ebx+sb_offs_start_x],dx ;���⠢�塞 ���� ����� ��ਧ. �஫�����
			mov eax,ted_wnd_h ;calculate lines in page
			mov edx,ted_wnd_t
			add edx,eax
			mov word[ebx+sb_offs_start_y],dx ;���⠢�塞 ���孨� ����� ��ਧ. �஫�����
		sub eax,ted_rec_t
		xor edx,edx
		mov ecx,ted_rec_h
		div ecx
		cmp eax,MIN_W_SCRL_ARE
		jg @f
			mov eax,MIN_W_SCRL_ARE
		@@:
		mov dword[esi+sb_offs_cur_area],eax

		mov eax,ted_wnd_w ;calculate cols in page
			mov edx,ted_wnd_l ;���� ����� ����
			add edx,eax ;������塞 �ਭ� ����
			mov word[esi+sb_offs_start_x],dx ;���⠢�塞 ���� ����� ����. �஫�����
			mov edx,ted_wnd_t
			mov word[esi+sb_offs_start_y],dx ;���⠢�塞 ���孨� ����� ����. �஫�����
			mov edx,ted_wnd_h
			mov word[esi+sb_offs_size_y],dx ;���⠢�塞 ����� ����. �஫�����
		sub eax,ted_rec_l
			mov word[ebx+sb_offs_size_x],ax ;���⠢�塞 �ਭ� ��ਧ. �஫�����
		xor edx,edx
		mov ecx,ted_rec_w
		div ecx
		cmp eax,MIN_H_SCRL_ARE
		jg @f
			mov eax,MIN_H_SCRL_ARE
		@@:
		dec eax
		mov dword[ebx+sb_offs_cur_area],eax ;��⠭�������� �᫮ ᨬ�����, ����� ������ � �࠭ ��� ��ਧ. �஫�����
	.no_size:
	popad
	ret
endp

align 16
proc ted_delete uses edi, edit:dword
	mov edi,[edit]
	invoke mem.free,ted_tex
	invoke mem.free,ted_arr_key_pos ;ted_syntax_file
	ret
endp


;input:
; eax = key kodes
align 16
proc ted_key, edit:dword, table:dword, control:dword
	pushad
	mov edi,[edit]
	mov esi,ted_el_focus
	cmp dword[esi],edi
	jne .end_key_fun ;����� �� � 䮪�� ��室�� �� �㭪樨
	mov esi,dword[control]

	cmp ah,KEY_F1 ;[F1]
	jne @f
		stdcall ted_show_help_f1,edi
		jmp .end_key_fun
	@@:
	cmp ah,KEY_F3 ;[F3]
	jne @f
		stdcall ted_but_find,edi,0
		jmp .end_key_fun
	@@:

	test esi,KM_CTRL ;Ctrl+...
	jz .key_Ctrl
		; *** �맮� ���譨� �㭪権 ����� �ॡ��� ���� ������/��࠭����/���᪠/...
		cmp ted_fun_on_key_ctrl_all,0
		je .end0
		xor al,al
		cmp ah,24 ;Ctrl+O
		jne @f
			mov al,'O'
		@@:
		cmp ah,31 ;Ctrl+S
		jne @f
			mov al,'S'
		@@:
		cmp ah,33 ;Ctrl+F
		jne @f
			mov al,'F'
		@@:
		cmp ah,34 ;Ctrl+G
		jne @f
			mov al,'G'
		@@:
		cmp ah,35 ;Ctrl+H
		jne @f
			mov al,'H'
		@@:
		cmp ah,49 ;Ctrl+N
		jne @f
			mov al,'N'
		@@:
		or al,al
		jz .end0
			and eax,0xff
			test esi,KM_SHIFT
			jz @f
				or eax,0x100
			@@:
			stdcall ted_fun_on_key_ctrl_all, eax
			jmp .end_key_fun
		.end0:

		; *** �맮� ����७��� �㭪権
		cmp ah,30 ;Ctrl+A
		jne @f
			call ted_sel_all
		@@:
		cmp ah,44 ;Ctrl+Z
		jne @f
			stdcall ted_but_undo,edi
		@@:
		cmp ah,45 ;Ctrl+X
		jne @f
			stdcall ted_but_cut,edi
		@@:
		cmp ah,46 ;Ctrl+C
		jne @f
			stdcall ted_but_copy,edi
		@@:
		cmp ah,47 ;Ctrl+V
		jne @f
			stdcall ted_but_paste,edi
		@@:
		cmp ah,199 ;Ctrl+Home
		jne @f
			call ted_key_ctrl_home
		@@:
		cmp ah,207 ;Ctrl+End
		jne @f
			call ted_key_ctrl_end
		@@:
		jmp .end_key_fun
	.key_Ctrl:

	test esi,KM_SHIFT ;Shift+...
	jz .key_Shift
		cmp ah,72 ;Shift+Up
		jne @f
			call ted_sel_key_up
		@@:
		cmp ah,75 ;Shift+Left
		jne @f
			call ted_sel_key_left
		@@:
		cmp ah,77 ;Shift+Right
		jne @f
			call ted_sel_key_right
		@@:
		cmp ah,80 ;Shift+Down
		jne @f
			call ted_sel_key_down
		@@:
		;mov ted_drag_k,1 ;��稭��� �뤥����� �� ����������
		jmp .key_MoveCur
	.key_Shift:
;-------------------------------------------------
	cmp ah,72 ;178 ;Up
	jne @f
		call ted_draw_cursor_sumb
		call ted_cur_move_up
		cmp dl,8
		jne .no_red_0
			call ted_scroll_set_redraw
			stdcall ted_draw,edi
			jmp @f
		.no_red_0:
		call ted_sel_end
	@@:
	cmp ah,80 ;177 ;Down
	jne @f
		call ted_draw_cursor_sumb
		call ted_cur_move_down
		cmp dl,8
		jne .no_red_1
			call ted_scroll_set_redraw
			stdcall ted_draw,edi
			jmp @f
		.no_red_1:
		call ted_sel_end
	@@:
	cmp ah,75 ;176 ;Left
	jne @f
		call ted_draw_cursor_sumb
		call ted_cur_move_left
		cmp dl,8
		jne .no_red_2
			call ted_scroll_set_redraw
			stdcall ted_draw,edi
			jmp @f
		.no_red_2:
		call ted_sel_end
	@@:
	cmp ah,77 ;179 ;Right
	jne @f
		call ted_draw_cursor_sumb
		call ted_cur_move_right
		cmp dl,8
		jne .no_red_3
			call ted_scroll_set_redraw
			stdcall ted_draw,edi
			jmp @f
		.no_red_3:
		call ted_sel_end
	@@:
	cmp ah,71 ;180 ;Home
	jne @f
		call ted_draw_cursor_sumb
		call ted_cur_move_x_first_char
		cmp dl,8
		jne .no_red_4
			call ted_scroll_set_redraw
			stdcall ted_draw,edi
			jmp @f
		.no_red_4:
		call ted_sel_end
	@@:
	cmp ah,79 ;181 ;End
	jne @f
		call ted_draw_cursor_sumb
		call ted_cur_move_x_last_char
		cmp dl,8
		jne .no_red_5
			call ted_scroll_set_redraw
			stdcall ted_draw,edi
			jmp @f
		.no_red_5:
		call ted_sel_end
	@@:
	cmp ah,73 ;184 ;PageUp
	jne @f
		call ted_cur_move_page_up
		cmp dl,0
		je @f
		call ted_scroll_set_redraw
		stdcall ted_draw,edi
		mov ted_drag_k,0 ;�����稢��� �뤥����� �� ����������
	@@:
	cmp ah,81 ;183 ;PageDown
	jne @f
		call ted_cur_move_page_down
		cmp dl,0
		je @f
		call ted_scroll_set_redraw
		stdcall ted_draw,edi
		mov ted_drag_k,0 ;�����稢��� �뤥����� �� ����������
	@@:
;-------------------------------------------------
	.key_MoveCur:

	;���� �ய�᪠���� �㦥��� ������, ����� ����� ������ ����� ᨬ���� � ����
	cmp ah,42 ;[L Shift] ����� ����� ��� ��㣨� ������
	je .end_key_fun
	cmp ah,54 ;[R Shift] ����� ����� ��� ��㣨� ������
	je .end_key_fun
	cmp ah,58 ;[Caps Lock]
	je .end_key_fun
	cmp ah,69 ;[Pause Break]
	je .end_key_fun
	cmp ah,120 ;[Fn]
	je .end_key_fun
	cmp ah,0x80 ;if key up
	ja .end_key_fun

	cmp dword[table],0
	je @f
		stdcall KeyConvertToASCII, [table]
	@@:

	;mov ted_drag_k,0 ;�����稢��� �뤥����� �� ����������

	lea edx,[EvChar] ;��६ ���� ⠡���� � �����⨬묨 ᨬ������
	add dl,ah
	jae @f
		add edx,0x100 ;�᫨ �뫮 ��९������� �� ���������� ���� ᨬ����
	@@:
	cmp byte[edx],1
	jne @f
		mov ted_key_new,ah
		call ted_set_undo
		mov edx,ted_opt_ed_change_time+ted_opt_ed_move_cursor
		stdcall ted_sel_text_del,edx
		cmp al,1
		jne .del
			mov edx,ted_opt_ed_move_cursor
		.del:
		cmp ted_cur_ins,1
		je .no_ins_mod
			stdcall ted_text_del,edi,ted_opt_ed_change_time
			mov edx,ted_opt_ed_move_cursor
		.no_ins_mod:
		mov ecx,edi
		add ecx,ted_offs_key_new
		stdcall ted_text_add,edi,ecx,1,edx ;������塞 ᨬ��� �������� � ����������
		cmp ted_key_new,13
		jne .dr_m_win
			;�᫨ ��⠢��� ᨬ��� ����� ��ப�
			mov ecx,ted_scr_w
			inc dword[ecx+sb_offs_max_area] ;㢥��稢��� ࠧ��� ���⨪��쭮�� �஫�����
			mov edx,ted_cur_y
			cmp edx,[ecx+sb_offs_cur_area]
			jl .no_change
				dec ted_cur_y ;����� ��⠢�塞 �� ����
				inc dword[ecx+sb_offs_position] ;ᤢ����� ����㭮�
			.no_change:
			stdcall ted_draw,edi
			jmp .dr_cur_l
		.dr_m_win:
			stdcall ted_draw_cur_line,edi
		.dr_cur_l:
		cmp ted_fun_draw_panel_buttons,0
		je @f
			call ted_fun_draw_panel_buttons
	@@:

	cmp ah,8 ;[<-]
	jne @f
		call ted_set_undo
		stdcall ted_sel_text_del,ted_opt_ed_change_time
		cmp al,1
		je .del_one_b
			stdcall ted_text_del,edi,ted_opt_ed_change_time+ted_opt_ed_move_cursor
		.del_one_b:
		stdcall ted_draw,edi
		cmp ted_fun_draw_panel_buttons,0
		je .end_key_fun
			call ted_fun_draw_panel_buttons
		jmp .end_key_fun
align 4
	@@:

	cmp ah,182 ;Delete
	jne @f
		call ted_set_undo
		stdcall ted_sel_text_del,ted_opt_ed_change_time
		cmp al,1
		je .del_one_d
			stdcall ted_text_del,edi,ted_opt_ed_change_time
		.del_one_d:
		stdcall ted_draw,edi
		cmp ted_fun_draw_panel_buttons,0
		je .end_key_fun
			call ted_fun_draw_panel_buttons
		jmp .end_key_fun
	@@:

	cmp ah,185 ;Ins
	jne @f
		call ted_draw_cursor_sumb
		xor ted_cur_ins,1
		call ted_draw_main_cursor
	@@:

	.end_key_fun:
	popad
	ret
endp

;output:
; al = 1 - can save
align 16
proc ted_can_save uses ecx edi, edit:dword
	mov edi,[edit]

	mov ecx,ted_tim_ch
	sub ecx,ted_tim_undo
	mov al,1
	cmp ted_tim_ls,ecx
	jne @f
		dec al
	@@:
	ret
endp

;input:
; edi = pointer to tedit struct
;output:
; al = 1 - selected
align 16
proc ted_is_select uses ebx
	xor al,al
	cmp ted_drag_m,1
	je @f
		inc al
		mov ebx,ted_sel_x0
		cmp ebx,ted_sel_x1
		jne @f
		mov ebx,ted_sel_y0
		cmp ebx,ted_sel_y1
		jne @f
		xor al,al
	@@:
	ret
endp

;input:
; edi = pointer to tedit struct
align 16
proc ted_sel_normalize uses ecx esi
	push edi
		mov esi,edi
		add esi,ted_offs_sel
		add edi,ted_offs_seln
		mov ecx,sizeof.TexSelect/4
		rep movsd
	pop edi

	jmp @f
		.swp_f:
		mov ecx,ted_seln_x0
		m2m ted_seln_x0,ted_seln_x1
		mov ted_seln_x1,ecx

		mov ecx,ted_seln_y0
		cmp ecx,ted_seln_y1 ;(sel_y0>sel_y1)
		jle .end_f
		m2m ted_seln_y0,ted_seln_y1
		mov ted_seln_y1,ecx

		jmp .end_f
align 4
	@@:

	mov ecx,ted_seln_y0
	cmp ecx,ted_seln_y1 ;(sel_y0>sel_y1)
	jg .swp_f

	cmp ecx,ted_seln_y1 ;(sel_y0==sel_y1)
	jne .end_f
		mov ecx,ted_seln_x0
		cmp ecx,ted_seln_x1 ;(sel_x0>sel_x1)
		jg .swp_f

	.end_f:
	ret
endp

;input:
; edi = pointer to tedit struct
;description:
; �㭪�� ��뢠���� �� ��砫� �뤥�����
align 16
proc ted_sel_start uses eax ecx
	mov eax,ted_scr_h
	mov ecx,ted_cur_x
	add ecx,[eax+sb_offs_position]
	mov ted_sel_x0,ecx
	mov ted_sel_x1,ecx

	mov eax,ted_scr_w
	mov ecx,ted_cur_y
	add ecx,[eax+sb_offs_position]
	mov ted_sel_y0,ecx
	mov ted_sel_y1,ecx
	ret
endp

;input:
; edi = pointer to tedit struct
;description:
; �㭪�� ��뢠���� �� ��⨨ �뤥�����
align 16
proc ted_sel_end uses eax
	mov ted_drag_k,0 ;�����稢��� �뤥����� �� ����������
	call ted_is_select
	or al,al
	jz @f
		xor eax,eax
		mov ted_sel_x0,eax
		mov ted_sel_x1,eax
		mov ted_sel_y0,eax
		mov ted_sel_y1,eax
		stdcall ted_draw,edi
		jmp .end_f
	@@:
	call ted_draw_main_cursor
	.end_f:
	ret
endp

;input:
; edi = pointer to tedit struct
;description:
; �㭪�� ��뢠���� �� ��६�饭�� �뤥�����
align 16
proc ted_sel_move
	push eax ecx
		mov ecx,ted_cur_x
		mov eax,ted_scr_h
		add ecx,[eax+sb_offs_position]
		mov ted_sel_x1,ecx

		mov eax,ted_scr_w
		mov ecx,ted_cur_y
		add ecx,[eax+sb_offs_position]
		mov ted_sel_y1,ecx
	pop ecx eax
	cmp ted_fun_draw_panel_buttons,0 ;redraw toolbar (need to button Copy)
	je @f
		call ted_fun_draw_panel_buttons
	@@:
	ret
endp

;input:
; edi = pointer to tedit struct
;description:
; �㭪�� ��뢠���� �� �뤥����� �ᥣ� ���㬥��
align 16
proc ted_sel_all
	push eax
		xor eax,eax
		mov ted_sel_x0,eax
		mov ted_sel_y0,eax

		mov ted_sel_x1,eax ;???
		call ted_get_num_lines
		mov ted_sel_y1,eax
	pop eax
	stdcall ted_draw,edi
	cmp ted_fun_draw_panel_buttons,0 ;redraw toolbar (need to button Copy)
	je @f
		call ted_fun_draw_panel_buttons
	@@:
	ret
endp

;input:
; cl_al_mem = 1 - clear all memory
align 16
proc ted_clear uses ecx edi, edit:dword, cl_al_mem:dword
	mov edi,[edit]

	mov ted_cur_x,0
	mov ted_cur_y,0
	mov ted_tim_ch,0
	mov ted_tim_ls,0
	mov ted_tim_co,0
	mov ted_tim_undo,0
	mov ted_help_id,-1
	mov ecx,sizeof.symbol
	shl ecx,1
	add ecx,ted_tex
	mov ted_ptr_free_symb,ecx

	mov ecx,ted_scr_w
	mov dword[ecx+sb_offs_position],0
	mov dword[ecx+sb_offs_max_area],100 ;�᫮ ��ப ������� � ����� ���㬥��
	mov dword[ecx+sb_offs_redraw],1
	mov ecx,ted_scr_h
	mov dword[ecx+sb_offs_position],0
	mov dword[ecx+sb_offs_max_area],100 ;�᫮ ᨬ����� ������� � ����� ���㬥��

	mov ted_sel_x0,0
	mov ted_sel_y0,0
	mov ted_sel_x1,0
	mov ted_sel_y1,0

	cmp dword[cl_al_mem],0
	je .exit

	push edx
	mov ecx,sizeof.symbol
	imul ecx,ted_max_chars
	mov edx,ted_tex
	@@:
		mov byte [edx],0
		inc edx
	loop @b
	mov edx,ted_tex
	mov dword [edx+symbol.next],1
	pop edx

	.exit:
	ret
endp


align 16
proc ted_init_syntax_file, edit:dword
	pushad
	mov edi,[edit]

	mov ecx,0x100
	mov edx,ted_arr_key_pos
	@@:
		mov dword[edx],-1
		add edx,4
	loop @b

	;init: ted_colors_text_count, ted_key_words_count, ...
	mov ted_colors_text_count,1
	mov ted_help_text_f1,0
	mov ted_help_id,-1 ;�����䨪��� ᫮�� ��� �ࠢ��

	mov eax,edi ;��࠭塞 ���祭�� edi
	mov esi,ted_syntax_file
	add edi,ted_offs_count_colors ;edi = &ted_key_words_count
	mov ecx,9
	rep movsd
	mov edi,eax ;���⠭�������� ���祭�� edi

	mov eax,ted_syntax_file
	add eax,32
	mov ted_text_colors,eax

	mov eax,ted_colors_text_count ;init: count_colors_text (offset to key words)
	lea eax,[4*eax+32]
	add eax,ted_syntax_file
	mov ted_key_words_data,eax

	mov ecx,ted_key_words_count ;init: ted_arr_key_pos (first key positions)
	or ecx,ecx
	jz .no_words
	xor eax,eax
	@@:
		ColToIndexOffset eax,edx
		movzx ebx,byte[edx]
		mov esi,ted_arr_key_pos
		lea esi,[esi+4*ebx]
		cmp dword[esi],-1
		jne .no_ch_key
			mov [esi],eax
		.no_ch_key:
		inc eax
	loop @b
	.no_words:

	;init: ted_help_text_f1
	mov ecx,ted_key_words_count ;������⢮ ���祢�� ᫮�
	imul ecx,sizeof.TexColViv   ;ࠧ��� �������� � 1-� ��. �.
	add ecx,ted_key_words_data  ;��砫� 䠩�� � ��. �.
	mov ted_help_text_f1,ecx    ;��⪠ � �����, ��� ��稭���� ⥪�� � �ࠢ���

	stdcall ted_init_scroll_bars,edi,1 ;���塞 梥� �஫������
	.no_colors:
	popad
	ret
endp

;input:
; ebx = file size
; edi = pointer to tedit struct
;description:
; �㭪�� ��뢠���� �� ����⨨ 䠩��
align 16
proc ted_on_open_file
	push eax ;destination
	push ecx ;for cycle
	push edx ;source
	push esi

	or ebx,ebx
	jnz @f
		;�᫨ 䠩� ���⮩
		stdcall ted_clear,edi,1 ;��⨬ ��� ������
		jmp .end_opn
align 4
	@@:
	stdcall ted_clear,edi,0 ;��⨬ �� ��� ������, ��⮬� �� ���� �㤥� �� ��������� ���묨 ���묨

	mov edx,ted_tex
	mov ecx,ebx
	.s_10:
		cmp word[edx],0xa0d ;�ய�᪠�� 10-� ᨬ��� �᫨ ��। ��� �⮨� 13-�
		jne @f
			dec ecx
			jz .no_10
			dec ecx
			jz .no_10
			add edx,2
		@@:
		cmp byte[edx],10
		jne @f
			mov byte[edx],13 ;���塞 10-� ᨬ��� ���� ��ப�
		@@:
		inc edx
		loop .s_10
	.no_10:

	;��ॢ���� ������ 䠩� ������ ����� t_edit
	mov ecx,ebx
	lea eax,[ebx+2]
	ConvertIndexToPointer eax
	mov edx,ted_tex
	add edx,ebx
	push ebx
	@@:
		mov ebx,[edx]
		mov byte[eax],bl
		mov dword[eax+symbol.perv],ecx
		inc dword[eax+symbol.perv]
		mov dword[eax+symbol.next],ecx
		add dword[eax+symbol.next],3
		;mov byte[eax+1],0 ;col=0
		mov dword[eax+symbol.tc],-1
		mov dword[eax+symbol.td],0

		or ecx,ecx
		jz @f
		dec ecx
		dec edx
		sub eax,sizeof.symbol
		jmp @b
align 4
	@@:
	pop ebx
	mov dword[eax+symbol.perv],0 ; first sumbol 'perv=0'

	mov edx,ted_tex ; ����ன�� ��砫쭮�� �㦥����� ᨬ����
	; begining sumbol 'perv=0' 'next=2'
	mov dword[edx+symbol.perv],0
	mov dword[edx+symbol.next],2

	add edx,sizeof.symbol ; ����ன�� ����筮�� �㦥����� ᨬ����
	mov dword[edx+symbol.next],0 ; last sumbol 'next=0'
	mov dword[edx+symbol.perv],ebx ; last sumbol 'perv=last'
	inc dword[edx+symbol.perv]
	mov dword[edx+symbol.tc],0 ; �⠢�� �६� ᮧ����� ࠢ��� 0, �� �� ᨬ��� �ࠢ��쭮 ��ࠡ��뢠��� �� ����⨨ 䠩��� ������ 28 ����

	mov edx,ebx
	inc edx ;2 = rezerv sumbols
	imul edx,sizeof.symbol
	add edx,ted_tex
	mov dword[edx+symbol.next],1 ; last sumbol 'next=1'

	@@: ;clear memory, need if before was open big file
		add edx,sizeof.symbol
		cmp edx,ted_tex_end
		jge .end_opn
			mov dword[edx+symbol.tc],0
			mov dword[edx+symbol.td],0
		jmp @b
align 4
	.end_opn:

	call ted_get_num_lines
	cmp eax,TED_LINES_IN_NEW_FILE
	jge @f
		mov eax,TED_LINES_IN_NEW_FILE
	@@:
	mov esi,ted_scr_w
	mov dword[esi+sb_offs_max_area],eax
	pop esi edx ecx eax

	call ted_text_colored
	stdcall ted_draw,edi
	cmp ted_fun_draw_panel_buttons,0
	je @f
		call ted_fun_draw_panel_buttons
	@@:
	ret
endp

;input:
; edx = pointer to symbol struct
; edi = pointer to tedit struct
;output:
; edx = pointer to 'perv' visible symbol struct
align 16
ted_iterat_perv:
	cmp ted_tim_undo,0
	je .else
	push ebx
	@@:
		mov edx,[edx+symbol.perv]
		or edx,edx
		jz @f
		imul edx,sizeof.symbol
		add edx,ted_tex
		call ted_symbol_not_vis
		cmp bl,1
		je @b
		cmp byte[edx],10 ;�ய�� ᨬ���� � ����� 10
		je @b
	pop ebx
	ret
	@@:
	mov edx,ted_tex ;��砫� 䠩��
	pop ebx
	ret
	.else:
		mov edx,[edx+symbol.perv]
		or edx,edx
		jz @f
		imul edx,sizeof.symbol
		add edx,ted_tex
		cmp dword[edx+symbol.td],0
		jne .else
		cmp byte[edx],10 ;�ய�� ᨬ���� � ����� 10
		je .else
	ret
	@@:
	mov edx,ted_tex ;��砫� 䠩��
	ret


;input:
; edx = pointer to symbol struct
; edi = pointer to tedit struct
;output:
; edx = pointer to 'next' visible symbol struct
align 16
ted_iterat_next:
	cmp ted_tim_undo,0
	je .else
	push ebx
	@@:
		mov edx,[edx+symbol.next]
		cmp edx,1
		jle @f
		imul edx,sizeof.symbol
		add edx,ted_tex

		call ted_symbol_not_vis
		cmp bl,1
		je @b
		cmp byte[edx],10 ;�ய�� ᨬ���� � ����� 10
		je @b
	pop ebx
	ret
	@@:
	mov edx,ted_tex_1 ;����� 䠩��
	pop ebx
	ret
	.else:
		mov edx,[edx+symbol.next]
		cmp edx,1
		jle @f
		imul edx,sizeof.symbol
		add edx,ted_tex

		cmp dword[edx+symbol.td],0
		jne .else
		cmp byte[edx],10 ;�ய�� ᨬ���� � ����� 10
		je .else
	ret
	@@:
	mov edx,ted_tex_1 ;����� 䠩��
	ret

;input:
; bl = symbol end of select
; bh = �࠭����騩 ᨬ��� (= 0 �᫨ ��� �஢�ન �� ���)
; edx = pointer to symbol struct
; edi = pointer to tedit struct
;description:
; ���� ᫥������ ������ 㪠������� ᨬ����
align 16
ted_iterat_next_pos_char:
	push ax
	mov al,1 ;�।��騩 ᨬ���, �㦨� ��� �ࠢ����� � ᨬ����� bh
	@@:
		cmp bl,byte[edx]
		je .found
		.no_found:
		cmp edx,ted_tex_1
		jle @f
			mov al,byte[edx]
			call ted_iterat_next
			jmp @b
	.found:
		cmp bh,al
		je .no_found
	@@:
	call ted_iterat_next
	pop ax
	ret

;input:
; edx = pointer to symbol struct
; edi = pointer to tedit struct
align 16
ted_iterat_perv_color_tag:
	@@:
		cmp byte[edx+1],0
		jne @f
		call ted_iterat_perv
		cmp edx,ted_tex_1
		jle @f
		jmp @b
align 4
	@@:
	ret

;input:
; edx = pointer to symbol struct
; edi = pointer to tedit struct
align 16
ted_iterat_next_color_tag:
	@@:
		call ted_iterat_next
		cmp byte[edx+1],0
		jne @f
		cmp edx,ted_tex_1
		jle @f
		jmp @b
align 4
	@@:
	ret

;input:
; edx = pointer to symbol struct
; edi = pointer to tedit struct
;output:
; bl = 1 if sumbol not visible
; (tex[i].td+ted_tim_undo<=ted_tim_ch && tex[i].td) || (tex[i].tc>ted_tim_ch-ted_tim_undo)
align 16
ted_symbol_not_vis:
	push eax

	xor bl,bl
	cmp dword[edx+symbol.td],0
	je @f
	mov eax,[edx+symbol.td] ;eax=tex[i].td
	add eax,ted_tim_undo
	cmp eax,ted_tim_ch
	jg @f
		mov bl,1
		pop eax
		ret
	@@:

	mov eax,ted_tim_ch
	sub eax,ted_tim_undo
	cmp [edx+symbol.tc],eax
	jle @f
		or bl,1
	@@:

	pop eax
	ret

;input:
; text - pointer to text string
; add_opt - options
align 16
proc ted_text_add, edit:dword, text:dword, t_len:dword, add_opt:dword
	locals
		new_spc dd ? ;count new spaces
		new_lin dd ? ;count new lines
	endl
;�ᯮ�짮����� ॣ���஢ ����� �㭪樨:
;eax - ������ ��� ��⠢�� ⥪��
;ebx - ��� �६����� �㦤, ������ ��⠢�塞��� ⥪��
;ecx - ��� �६����� �㦤
;edx - 㪠��⥫� �� �������� ᨬ����
	pushad
	cmp dword[t_len],1 ;�஢��塞 ������ �����塞��� ⥪��
	jl .no_add ;����� ������ <1 ��룠�� �� ����� �㭪樨, �� ��������� ���

	mov edi,[edit]
	mov esi,[text]

	call ted_get_pos_by_cursor
	call ted_get_text_perv_pos
	call ted_get_text_arr_index ;eax=po_t

	mov dword[new_spc],0
	cmp ted_gp_opt,2
	je @f
		push eax ;c_sp=cur[cn].x+Scroller->XPos-StrLen(cur[cn].y+Scroller->YPos);
			mov eax,ted_scr_h
			mov eax,[eax+sb_offs_position]
			add eax,ted_cur_x ;eax - ����� ᨬ����
			mov [new_spc],eax

			mov eax,ted_scr_w
			mov eax,[eax+sb_offs_position]
			add eax,ted_cur_y ;eax - ����� ��ப�
			call ted_strlen ;ebx = line len
			sub [new_spc],ebx ;�� ����樨 ����� �⭨���� ������ ��ப�, 㧭��� �������⢮ ������塞�� �஡����
		pop eax
	@@:

	mov ebx,[t_len]

	mov dword[new_lin],0
	cmp ted_gp_opt,0
	jne @f
		push eax
			mov eax,ted_scr_w
			mov eax,[eax+sb_offs_position]
			add eax,ted_cur_y
			inc eax
			mov [new_lin],eax

			call ted_get_num_lines
			sub [new_lin],eax
			;㢥��稢��� ����� � �஫����� �� �᫮ ����������� �������⥫��� ��ப
			mov ecx,ted_scr_w
			add [ecx+sb_offs_max_area],eax ;㢥��稢��� ࠧ��� ���⨪��쭮�� �஫�����
		pop eax
	@@:

	mov edx,ted_ptr_free_symb
	.beg_cycle: ;for(i=...;i<ted_max_chars;i++)
		cmp dword[edx+symbol.tc],0 ;if(!tex[i].tc && !tex[i].td)
		jne .u1f
		cmp dword[edx+symbol.td],0
		jne .u1f
			test dword[add_opt],ted_opt_ed_change_time ;if(n_tim) ted_tim_ch++;
			jz .no_tim
				inc ted_tim_ch
			.no_tim:
			test dword[add_opt],ted_opt_ed_move_cursor
			jz .no_cur_mov
			cmp dword[new_lin],0 ;�᫨ ���� �������� ��ப�, � ����� �� �� �������
			jg .no_cur_mov
			cmp dword[new_spc],0 ;�᫨ ��� ��������� �஡����, � ����� ⮦� �� �������
			jg .no_cur_mov
				inc ted_cur_x ;move cursor
				;call ted_go_to_pos
				cmp byte[esi],13
				jne .no_cur_mov
					mov ted_cur_x,0
					inc ted_cur_y
					;㢥��稢��� ����� � �஫����� �� �᫮ ����������� � ⥪�� ��ப
					mov ecx,ted_scr_w
					inc dword[ecx+sb_offs_max_area] ;㢥��稢��� ࠧ��� ���⨪��쭮�� �஫�����
			.no_cur_mov:

			; *** ��⠢�� ⥪�饣� ᨬ���� �� ��ப� ***
			mov ecx,ted_opt_ed_change_time
			not ecx
			and [add_opt],ecx ;n_tim=false;

			mov cl,byte[esi] ;tex[i].c=ta[ns];
			mov byte[edx],cl
			m2m dword[edx+symbol.tc],ted_tim_ch ;tex[i].tc=ted_tim_ch;
			mov [edx+symbol.perv],eax ;tex[i].perv=po_t;

			mov ecx,eax
			imul ecx,sizeof.symbol
			add ecx,ted_tex ; *** ecx = tex[po_t] ***
			add ecx,symbol.next ; *** ecx = tex[po_t].next ***
			m2m dword[edx+symbol.next],dword[ecx] ;tex[i].next=tex[po_t].next;

			call ted_get_text_arr_index ;*** eax = i ***
			mov [ecx],eax ;tex[po_t].next=i; // ��뫪� ��७��ࠢ�塞
			mov ecx,[edx+symbol.next] ; *** ecx = tex[i].next ***
			imul ecx,sizeof.symbol
			add ecx,ted_tex ; *** ecx = tex[tex[i].next] ***
			mov [ecx+symbol.perv],eax ;tex[tex[i].next].perv=i;

			; *** ��⠢�� �������⥫��� ��ப � �஡����
			; �᫨ ����� �� �६� ��⠢�� ��室���� �� ⥪�⮬ ***
			cmp dword[new_lin],0 ;add lines or text
			jle .spc_add
				dec dword[new_lin]
				mov byte [edx],13
				jmp .u1f
			.spc_add:
			cmp dword[new_spc],0 ;add spaces or text
			jle .tex_add
				dec dword[new_spc]
				mov byte [edx],' '
				jmp .u1f
			.tex_add:
			inc esi ; ���室 � ᫥���饬� ��⠢�塞��� ᨬ����
			dec ebx
		.u1f:
		add edx,sizeof.symbol
		cmp edx,ted_tex_end
		jge @f ;out of memory
		or ebx,ebx
		jnz .beg_cycle
		mov ted_ptr_free_symb,edx ;���塞 㪠��⥫� �� ᢮����� ᨬ���, ��� ����� ����ண� ���᪠ �����
		jmp .add_all
	@@:
	cmp ted_increase_size,0
	je .add_all
		call ted_memory_increase
		or ebx,ebx
		jnz .beg_cycle
	.add_all: ;�� ᨬ���� ���������

	call ted_text_colored
	.no_add:
	popad
	ret
endp

;input:
;  edx = pointer to sumbol, when insert
;  edi = pointer to tedit struct
;output:
;  edx = new pointer to sumbol, when insert
align 16
proc ted_memory_increase
	cmp ted_increase_size,0
	je @f
		push eax ebx ecx
		mov ebx,ted_tex
		mov ecx,ted_max_chars
		call ted_mem_resize.no_2
		sub edx,ebx
		add edx,ted_tex
		mov ted_ptr_free_symb,edx
		pop ecx ebx eax
	@@:
	ret
endp

;input:
;  ecx = position to free insert cell
;  edx = pointer to sumbol, when insert
;  esi = added symbol
;  edi = pointer to tedit struct
;output:
;  ecx = position to inserted cell
align 16
ted_char_add:
	.loop_b:
		cmp ecx,ted_tex_end
		jge .end_f
		cmp dword[ecx+symbol.tc],0
		jne @f
			cmp dword[ecx+symbol.td],0
			je .loop_e
		@@:
		add ecx,sizeof.symbol
		jmp .loop_b
align 4
	.loop_e:

	push eax ebx
	mov eax,ted_tim_ch
	mov [ecx+symbol.tc],eax
	mov ax,si
	mov byte[ecx],al

	call ted_get_text_arr_index ; *** eax=pos ***
	mov [ecx+symbol.perv],eax ;tex[i].perv=pos;
	m2m dword[ecx+symbol.next],dword[edx+symbol.next] ;tex[i].next=tex[pos].next;

	push edx
		mov edx,ecx
		call ted_get_text_arr_index ; *** eax=i ***
	pop edx

	mov [edx+symbol.next],eax ;tex[pos].next=i; // ��뫪� ��७��ࠢ�塞
	mov ebx,[ecx+symbol.next]
	ConvertIndexToPointer ebx
	mov [ebx+symbol.perv],eax ;tex[tex[i].next].perv=i; // ...
	pop ebx eax

	.end_f:
	call ted_text_colored
	ret

;description:
; �㭪�� ��� ᬥ�� ����஢��
;input:
; table - ⠡��� ��� ��४���஢��
align 16
proc ted_but_convert_by_table uses eax edx edi esi, edit:dword, table:dword
	mov edi,[edit]
	mov esi,[table]
	mov edx,ted_tex
	.cycle:
		;���室�� �� ᫥���騩 ᨬ���
		mov edx,[edx+symbol.next]
		cmp edx,1
		jle .end_text
		imul edx,sizeof.symbol
		add edx,ted_tex

		movzx eax,byte[edx]
		add eax,esi
		mov al,byte[eax]
		cmp al,0
		je @f
			mov byte[edx],al ;���塞 ����஢�� ᨬ����
		@@:
		jmp .cycle
	.end_text:
	;cmp esi,0
	;je @f
		stdcall ted_draw,edi ;������塞 ����
	;@@:
	ret
endp

;input:
; edi = pointer to tedit struct
;output:
; esi = count converted symbols
;description:
; �㭪�� �ᯮ������ ��� ᬥ�� ॣ���� ��࠭��� ᨬ�����
align 16
proc ted_convert_sel_text, conv_fun:dword
	locals
		conv_cou dd ?
	endl
	mov dword[conv_cou],0
	pushad

	call ted_is_select
	or al,al
	jz .end_f
		call ted_set_undo
		call ted_sel_normalize

		mov esi,ted_seln_x0
		mov ecx,ted_seln_y0
		call ted_get_pos_by_coords
		mov eax,edx
		mov esi,ted_seln_x1
		mov ecx,ted_seln_y1
		call ted_get_pos_by_coords
		;call ted_get_text_perv_pos
		mov ebx,edx

		cmp eax,ebx
		je .end_f

		inc ted_tim_ch
		mov edx,eax ;i=p0;
		mov ecx,ted_ptr_free_symb
		@@:
		push eax
		mov al,byte[edx]
		call dword[conv_fun] ;�८�ࠧ������ ᨬ����
		mov esi,eax
		cmp byte[edx],al
		pop eax
		je .no_change
			m2m dword[edx+symbol.td],ted_tim_ch
			call ted_char_add ;b_pos=ted_char_add(tex[i].c^32,i,false,b_pos);
			call ted_get_text_next_pos ;go to added symbol
			inc dword[conv_cou]
		.no_change:

		call ted_iterat_next
		cmp edx,ted_tex
		je @f 
		cmp edx,ebx
		jne @b
		@@:
		cmp dword[conv_cou],0
		jne @f
			dec ted_tim_ch
		@@:
	.end_f:
	popad
	mov esi,[conv_cou]
	ret
endp

;output:
; bl = 0 - no delete
; bl = 1 - delete
align 16
proc ted_text_del uses ecx edx edi, edit:dword, del_opt:dword
	mov edi,[edit]
	mov ebx,[del_opt]

	xor cl,cl
	test ebx,ted_opt_ed_move_cursor
	jz @f
		call ted_cur_move_left
		cmp dl,0
		je .no_del
	@@:
	call ted_get_pos_by_cursor
	cmp ted_gp_opt,1
	je .no_del
		test ebx,ted_opt_ed_change_time
		jz @f
			inc ted_tim_ch
		@@:
		m2m dword[edx+symbol.td], ted_tim_ch
		mov cl,1
	.no_del:
	mov bl,cl
	ret
endp

;input:
; edi = pointer to tedit struct
;output:
; al = 1 if delete
;description:
; �㭪�� 㤠��� �뤥����� ⥪��
align 16
proc ted_sel_text_del uses ebx ecx edx esi, del_opt:dword
	call ted_is_select
	or al,al
	jz .end_f
		call ted_sel_normalize

		mov esi,ted_seln_x1
		mov ecx,ted_seln_y1
		call ted_get_pos_by_coords
		mov ebx,edx

		mov esi,ted_seln_x0
		mov ecx,ted_seln_y0
		call ted_get_pos_by_coords

		test dword[del_opt],ted_opt_ed_change_time
		jz @f
			inc ted_tim_ch
		@@:
		cmp edx,ted_tex
		je @f
		cmp edx,ebx ;if(i==te)break;
		je @f
			m2m dword[edx+symbol.td],ted_tim_ch
			mov esi,ted_opt_ed_change_time
			not esi
			and dword[del_opt],esi ;n_tim=false;
			call ted_iterat_next
			jmp @b
align 4
		@@:
		test dword[del_opt],ted_opt_ed_change_time
		jz @f
			dec ted_tim_ch
			xor al,al
		@@:
		test dword[del_opt],ted_opt_ed_change_time
		jnz @f
			mov ecx,ted_seln_x0
			mov edx,ted_seln_y0
			call ted_go_to_pos
			mov ted_sel_x0,0
			mov ted_sel_y0,0
			mov ted_sel_x1,0
			mov ted_sel_y1,0
		@@:
	.end_f:
	ret
endp


;input:
; eax = pointer to begin select
; ebx = pointer to end select
; edi = pointer to tedit struct
align 16
ted_revers:
	cmp eax,ebx
	jne @f
		ret
	@@:

	push ecx edx

	mov edx,ted_tex_1
	cmp edx,ebx ;if(p1==1)p1=tex[1].perv;
	jne @f
		call ted_get_text_perv_pos
		mov ebx,edx
	@@:

	push esi
		mov edx,[eax+symbol.perv] ; *** edx = tex[p0].perv ***
		ConvertIndexToPointer edx
		add edx,symbol.next
		mov ecx,[edx] ;ecx = tex[tex[p0].perv].next

		mov esi,[ebx+symbol.next] ; *** esi = tex[p1].next ***
		ConvertIndexToPointer esi
		add esi,symbol.perv
		m2m dword[edx],dword[esi] ;tex[tex[p0].perv].next = tex[tex[p1].next].perv

		mov [esi],ecx ;tex[tex[p1].next].perv = ecx
	pop esi

	mov ecx,[eax+symbol.perv] ;ecx = tex[p0].perv
	m2m dword[eax+symbol.perv],dword[ebx+symbol.next] ;tex[p0].perv = tex[p1].next
	mov [ebx+symbol.next],ecx ;tex[p1].next = ecx

	mov edx,eax ;i=p0;
	@@:
		mov ecx,[edx+symbol.next] ;ecx = tex[i].next
		m2m dword[edx+symbol.next],dword[edx+symbol.perv] ;tex[i].next = tex[i].perv
		mov [edx+symbol.perv],ecx ;tex[i].perv = ecx
		cmp edx,ebx ;if(i==p1)break;
		je @f
; ---
;cmp edx,ted_tex
;je @f
; ---
		mov edx,ecx ;i = ecx
		ConvertIndexToPointer edx
		jmp @b
	@@:
	pop edx ecx
	call ted_text_colored
	ret


;input:
; edi = pointer to tedit struct
;output:
; dl = 0 not move
; dl = 2 if move up
; dl = 8 if scroll move up
align 16
ted_cur_move_up:
  cmp ted_cur_y,0
  je @f
    dec ted_cur_y
    mov dl,2
    ret
  @@:
  push eax
  mov eax,ted_scr_w
  cmp dword[eax+sb_offs_position],0
  je @f
    dec dword[eax+sb_offs_position]
    mov dl,8
    jmp .ret_f
  @@:
  mov dl,0
  .ret_f:
  pop eax
  ret

;input:
; edi = pointer to tedit struct
;output:
; dl = 0 not move
; dl = 2 if move down
; dl = 8 if scroll move down
align 16
ted_cur_move_down:
  push eax ebx
  mov ebx,ted_scr_w
  xor dl,dl
  mov eax,[ebx+sb_offs_cur_area]
  dec eax
  cmp ted_cur_y,eax
  jge @f
    inc ted_cur_y
    mov dl,2
    jmp .ret_f
  @@:
  mov eax,ted_cur_y
  add eax,[ebx+sb_offs_position]
  inc eax
  cmp [ebx+sb_offs_max_area],eax
  jle @f
    inc dword[ebx+sb_offs_position]
    mov dl,8
  @@:
  .ret_f:
  pop ebx eax
  ret


;input:
; edi = pointer to tedit struct
;output:
; dl = 0 not move
; dl = 1 if move up
align 16
ted_cur_move_page_up:
	push eax ebx
	mov ebx,ted_scr_w
	mov eax,[ebx+sb_offs_cur_area]
	xor dl,dl
	cmp eax,[ebx+sb_offs_position]
	jg @f
		sub [ebx+sb_offs_position],eax
		mov dl,1
	@@:
	cmp dword[ebx+sb_offs_position],0
	je @f
	cmp dl,1
	je @f
		mov dword[ebx+sb_offs_position],0
		mov dl,1
	@@:
	pop ebx eax
	ret

;input:
; edi = pointer to tedit struct
align 16
ted_cur_move_page_down:
	push eax ebx ecx
	mov ecx,ted_scr_w

	xor dl,dl
	mov eax,[ecx+sb_offs_max_area]
	sub eax,[ecx+sb_offs_cur_area]
	cmp [ecx+sb_offs_position],eax
	jge @f
		mov ebx,[ecx+sb_offs_cur_area]
		add [ecx+sb_offs_position],ebx
		mov dl,1
		mov dword[ecx+sb_offs_redraw],1
		cmp [ecx+sb_offs_position],eax
		jle @f
			mov [ecx+sb_offs_position],eax
	@@:
	pop ecx ebx eax
	ret

;input:
; edi = pointer to tedit struct
;output:
; dl = 0 not move
; dl = 1 if move left
; dl = 3 if move left and up
; dl = 8 if scroll move up
align 16
ted_cur_move_left:
	cmp ted_cur_x,0
	je @f
		dec ted_cur_x
		mov dl,1
		ret
	@@:
	push eax
	mov eax,ted_scr_h
	cmp dword[eax+sb_offs_position],0
	je @f
		dec dword[eax+sb_offs_position]
		mov dl,8
		jmp .ret_f
	@@:
	cmp ted_cur_y,0
	jne @f
		mov eax,ted_scr_w
		mov dl,0
		cmp dword[eax+sb_offs_position],0
		je .ret_f
			dec dword[eax+sb_offs_position]
			call ted_scroll_set_redraw
			call ted_cur_move_x_last_char
			mov dl,8
			jmp .ret_f
	@@:
	cmp ted_cur_y,0
	je @f
		dec ted_cur_y
		call ted_cur_move_x_last_char
		cmp dl,8
		je .ret_f
		mov dl,3
		jmp .ret_f
	@@:
	mov dl,0
	.ret_f:
	pop eax
	ret

;input:
; edi = pointer to tedit struct
align 16
ted_cur_move_right:
	push eax ebx
	mov eax,ted_scr_h
	xor dl,dl
	mov ebx,[eax+sb_offs_cur_area]
	cmp ted_cur_x,ebx
	jge @f
		inc ted_cur_x
		mov dl,1
		jmp .ret_f
	@@:
		inc dword[eax+sb_offs_position]
		mov dl,8
	.ret_f:
	pop ebx eax
	ret

;input:
; edi = pointer to tedit struct
align 16
ted_cur_move_x_last_char:
;[hScr.position]
;[hScr.cur_area]
;dl-???
  push eax ebx ecx
  mov eax,ted_cur_y
  mov ecx,ted_scr_w
  add eax,[ecx+sb_offs_position]
  call ted_strlen
  xor dl,dl

  mov ecx,ted_scr_h
  cmp ebx,[ecx+sb_offs_position]
  jge @f
    mov dl,8
    mov [ecx+sb_offs_position],ebx
  @@:
  sub ebx,[ecx+sb_offs_position]

  cmp ebx,[ecx+sb_offs_cur_area]
  jle @f ; b---[---]---e
    add [ecx+sb_offs_position],ebx
    mov ebx,[ecx+sb_offs_cur_area]
    sub [ecx+sb_offs_position],ebx
    mov dl,8
  @@:
  mov ted_cur_x,ebx
  pop ecx ebx eax
  ret

;input:
; edi = pointer to tedit struct
;output:
; dl = 0 not move
; dl = 1 move cursor
; dl = 8 move cursor and scroll
align 16
ted_cur_move_x_first_char:
	xor dl,dl
	cmp ted_cur_x,0
	je @f
		mov ted_cur_x,0
		mov dl,1
	@@:
	push eax
	mov eax,ted_scr_h
	cmp dword[eax+sb_offs_position],0
	je @f
		mov dword[eax+sb_offs_position],0
		mov dl,8
	@@:
	pop eax
	ret

;input:
; edx = pointer to symbol struct
; edi = pointer to tedit struct
;output:
; eax = array index
align 16
ted_get_text_arr_index:
	push ecx edx
		mov eax,edx
		sub eax,ted_tex
		xor edx,edx
		mov ecx,sizeof.symbol
		div ecx
	pop edx ecx
	ret

;input:
; edx = pointer to symbol struct
; edi = pointer to tedit struct
;output:
; edx = pointer to 'perv' struct
align 16
ted_get_text_perv_pos:
	mov edx,[edx+symbol.perv]
	imul edx,sizeof.symbol
	add edx,ted_tex
	ret

;input:
; edx = pointer to symbol struct
;output:
; edx = pointer to 'next' symbol struct
align 16
ted_get_text_next_pos:
	mov edx,[edx+symbol.next]
	imul edx,sizeof.symbol
	add edx,ted_tex
	ret

;input:
; edi = pointer to tedit struct
;output:
; edx = symbol under cursor
; ted_gp_opt = 1,2
; edx = tex[1].perv if error
; ted_gp_opt = 0
align 16
ted_get_pos_by_cursor:
	push eax ecx esi
		mov esi,ted_cur_x
		mov eax,ted_scr_h
		add esi,[eax+sb_offs_position]
		mov ecx,ted_cur_y
		mov eax,ted_scr_w
		add ecx,[eax+sb_offs_position]
		call ted_get_pos_by_coords
	pop esi ecx eax
	ret

;input:
; esi = XPos
; ecx = YPos
; edi = pointer to tedit struct
;output:
; edx = symbol under cursor
; ted_gp_opt = 1 if found text line
; ted_gp_opt = 2 if found text line and column
; edx = tex[1] if error
; ted_gp_opt = 0 if text no found
align 16
proc ted_get_pos_by_coords uses eax ebx 
	xor eax,eax ;Row
	xor ebx,ebx ;Col
  mov ted_gp_opt,0
  mov edx,ted_tex
  @@:
    call ted_iterat_next
    cmp edx,ted_tex_1
    jle @f 
    cmp ebx,esi
    jne .u1_0 ;Col <> ted_cur_x
      mov ted_gp_opt,1
      cmp eax,ecx
      jge @f ; Row >= ted_cur_y
    .u1_0:
    mov ted_gp_opt,0
    inc ebx
    cmp byte [edx],13
    jne @b
    cmp eax,ecx
    jge @f ; Row >= ted_cur_y
    inc eax
    xor ebx,ebx
    jmp @b
  @@:
  cmp eax,ecx
  jne @f ; Row = ted_cur_y
    inc ted_gp_opt
  @@:
  cmp ted_gp_opt,0
  jne @f
    mov edx,ted_tex_1
    ;call ted_get_text_perv_pos
  @@:
  ret
endp

;input:
; eax = Row
; edi = pointer to tedit struct
;output:
; ebx = str len
align 16
ted_strlen:
  push edx ecx
  ;ecx = Row, from cycle

  xor ebx,ebx
  xor ecx,ecx
  mov edx,ted_tex
  @@:
    call ted_iterat_next
    cmp edx,ted_tex_1
    jle @f 
    inc ebx
    cmp byte [edx],13
    jne @b
    dec ebx ;lenght minus 1 sumbol to paragraph
    cmp eax,ecx
    je @f
    xor ebx,ebx
    inc ecx
    jmp @b
  @@:

  cmp eax,ecx
  je @f
    xor ebx,ebx
  @@:

  pop ecx edx
  ret


;input:
; edx = symbol position
; edi = pointer to tedit struct
;output:
; eax = number of line
; ebx = symbol position in line
align 16
ted_get_text_coords:
	push edx
	xor eax,eax
	xor ebx,ebx
	@@:
		call ted_iterat_perv

		or eax,eax
		jnz .no_col_mov
			inc ebx
		.no_col_mov:

		cmp edx,ted_tex_1
		jle @f
		cmp byte [edx],13
		jne @b
		inc eax
		jmp @b
	@@:
	dec ebx
	pop edx
	ret

;input:
; edi = pointer to tedit struct
;output:
; eax = num lines
align 16
ted_get_num_lines:
	push edx
	mov eax,1
	mov edx,ted_tex
	@@:
		call ted_iterat_next
		cmp edx,ted_tex_1
		jle @f
		cmp byte[edx],13
		jne @b
		inc eax
		jmp @b
	@@:
;...
;dec eax
	pop edx
	ret


;input:
; edi = pointer to tedit struct
;description:
; �⬥��� �⬥����� ����⢨�, ��। ���������� ���㬥��
align 16
proc ted_set_undo
	mov ted_drag_k,0 ;�����稢��� �뤥����� �� ����������
	cmp ted_tim_undo,1
	jl .no_work

	push eax ebx edx
	mov edx,ted_tex
	call ted_get_text_next_pos ;long i=tex[0].next;
	mov eax,ted_tim_undo
	sub ted_tim_ch,eax ;ted_tim_ch-=ted_tim_undo;
	mov eax,ted_tim_ch
	cmp ted_tim_ls,eax ;if(ted_tim_ls>ted_tim_ch)
	jle @f
		mov ted_tim_ls,0
	@@:
		cmp edx,ted_tex_1
		jle @f

		;if(tex[i].tc>ted_tim_ch){ // �᫨ ᮧ����� ᨬ���� �뫮 �⬥����
		cmp [edx+symbol.tc],eax
		jle .no_u1
			mov dword[edx+symbol.tc],0
			mov dword[edx+symbol.td],0

			mov ebx,[edx+symbol.perv]
			imul ebx,sizeof.symbol
			add ebx,ted_tex ;ebx=tex[i].perv
			m2m dword [ebx+symbol.next],dword [edx+symbol.next] ;tex[tex[i].perv].next=tex[i].next;

			mov ebx,[edx+symbol.next]
			imul ebx,sizeof.symbol
			add ebx,ted_tex ;ebx=tex[i].next
			m2m dword [ebx+symbol.perv],dword [edx+symbol.perv] ;tex[tex[i].next].perv=tex[i].perv;

			cmp ted_ptr_free_symb,edx
			jle .no_cor_free
				mov ted_ptr_free_symb,edx ;���塞 㪠��⥫� �� ᢮����� ᨬ���, ��� ����� ����ண� ���᪠ �����
			.no_cor_free:
			mov edx,ebx ;��⨬����㥬 �� ᪮��� (edx ��᫥ �맮�� ted_get_text_next_pos �㤥� ࠢ�� ebx)
			jmp @b
		.no_u1:

		;else if(tex[i].td>ted_tim_ch) tex[i].td=0; // �᫨ 㤠����� ᨬ���� �뫮 �⬥����
		cmp [edx+symbol.td],eax
		jle .no_u2
			mov dword[edx+symbol.td],0
		.no_u2:

		call ted_get_text_next_pos
		jmp @b
	@@:
	mov ted_tim_undo,0
	mov eax,ted_tim_co
	cmp ted_tim_ch,eax
	jge @f
		mov ted_tim_co,0
	@@:
	pop edx ebx eax
	.no_work:
	ret
endp

;description:
; ���室 �� 㪠������ ������
;input:
; row = ����� ��ப�
; col = ᨬ���
align 16
proc ted_go_to_position uses ecx edx edi, edit:dword, row:dword, col:dword
	mov edi,[edit]
	; �����⮢�� ��ப�
	mov edx,[row]
	call ted_get_num_lines
	cmp edx,eax
	jle @f
		mov edx,eax ;��࠭�祭�� �� ��ப� max
	@@:
	dec edx
	cmp edx,0
	jge @f
		xor edx,edx ;��࠭�祭�� �� ��ப� min
	@@:
	; �����⮢�� ᨬ����
	mov ecx,[col]
	dec ecx
	cmp ecx,0
	jge @f
		xor ecx,ecx
	@@:
	call ted_go_to_pos
	stdcall ted_draw,edi
	ret
endp

;input:
; ecx = Col
; edx = Row
; edi = pointer to tedit struct
;output:
; ecx = cursor x
; edx = cursor y
align 16
ted_go_to_pos:
	push eax ebx
	mov eax,ted_scr_h
	sub ecx,[eax+sb_offs_position]
	cmp ecx,0 ;ted_cur_x < 0
	jge @f
		add [eax+sb_offs_position],ecx ;�ப��⪠ �஫����� �����
		xor ecx,ecx
	@@:
	mov ebx,5 ;5 - ������� ����� ᫥��
	cmp ecx,ebx
	jge .end0
		sub ebx,ecx ;ebx - �� ᪮�쪮 ᨬ����� �㦭� ᤢ����� �����
		cmp [eax+sb_offs_position],ebx
		jge @f 
			add ecx,[eax+sb_offs_position]
			mov dword[eax+sb_offs_position],0
			jmp .end0
		@@:
			sub [eax+sb_offs_position],ebx
			add ecx,ebx
	.end0:
	cmp ecx,[eax+sb_offs_cur_area] ;ted_cur_x > [.cur_area]
	jl .end1
		mov ebx,ecx
		sub ebx,[eax+sb_offs_cur_area]
		inc ebx
		add [eax+sb_offs_position],ebx ;�ப��⪠ �஫����� ��ࠢ�
		sub ecx,ebx
	.end1:
	mov ted_cur_x,ecx

	mov eax,ted_scr_w
	sub edx,[eax+sb_offs_position]
	cmp edx,0 ;ted_cur_y < 0
	jge @f
		add [eax+sb_offs_position],edx ;�ப��⪠ �஫����� �����
		xor edx,edx
		jmp .end2
	@@:
	cmp edx,[eax+sb_offs_cur_area] ;ted_cur_y > [.cur_area]
	jl .end2
		mov ebx,edx
		sub ebx,[eax+sb_offs_cur_area]
		inc ebx
		add [eax+sb_offs_position],ebx ;�ப��⪠ �஫����� ����
		sub edx,ebx
	.end2:
	mov ted_cur_y,edx
	pop ebx eax
	ret

;input:
; edi = pointer to tedit struct
align 16
ted_text_colored:
	push eax edx
	mov eax,ted_tim_ch
	sub eax,ted_tim_undo
	mov ted_tim_co,eax
	mov edx,ted_tex
	@@:
		call ted_iterat_next
		cmp edx,ted_tex_1
		jle @f
		mov byte[edx+1],0
		jmp @b
	@@:

	cmp ted_key_words_count,1
	jl .no_colors
	mov edx,ted_tex
	@@:
		call ted_text_find_sel_color
		cmp edx,ted_tex_1
		jg @b

	xor ax,ax
	mov edx,ted_tex
	.cycle0:
		call ted_iterat_next
		cmp edx,ted_tex_1
		jle .no_colors
		mov al,byte[edx+1]
		or al,al
		jz .cycle0
		cmp ah,al
		jne @f
			mov byte[edx+1],0 ;᫨ﭨ� �冷� ����� ᫮� ������ 梥�
		@@:
		shl ax,8
		jmp .cycle0
	.no_colors:
	pop edx eax
	ret


;input:
; edx = pointer to start symbol
; edi = pointer to tedit struct
;output:
; edx = pointer to next symbol
;description:
; �㭪�� ��� ���᪠ � �뤥����� ���ᢥ祭�� ᫮�
align 16
proc ted_text_find_sel_color uses eax ebx ecx esi
locals
	begPos dd ? ;��砫쭠� ������
	endPos dd ? ;����筠� ������
	find db ? ;0 - �� �������, 1 - �������, 2 - ������� � ���� 䠩��
	f_color db ? ;������ 梥� ��������� ᫮��
endl
;eax = word_n ⥪�騩 ����� (������) �஢��塞��� ᫮�� � ᯨ᪥
;ebx = ��� ࠧ��� 楫��
;ecx = l_pos ��᫥���� ����� (������) ���室�饣� ᫮�� � ᯨ᪥
;esi = ��� ࠧ��� 楫��, ����� �஢��塞��� ᨬ���� � ᫮��
	mov dword[begPos],1
	mov dword[endPos],1
	mov byte[find],0
	mov byte[f_color],1
	.cycle0:
		call ted_iterat_next
		cmp edx,ted_tex_1
		jle .cycle0end

		movzx eax,byte[edx]
		shl eax,2 ;eax*=4
		add eax,ted_arr_key_pos
		mov eax,[eax]
		cmp eax,0
		jl .cycle0 ;if( (word_n=ted_arr_key_pos[(unsigned char)tex[i].c])>-1 ){

		mov ecx,eax
		;while(l_pos<ted_key_words_count && Col[l_pos].Text[0]==Col[word_n].Text[0])
		.wh_1b:
			cmp ecx,ted_key_words_count
			jge .wh_1e
			ColToIndexOffset ecx,esi
			mov bl,byte[esi]
			ColToIndexOffset eax,esi
			cmp bl,byte[esi]
			jne .wh_1e
				inc ecx
			jmp .wh_1b
		.wh_1e:

		mov [begPos],edx ;bP=i;
		mov esi,1
align 4
		.wh_2b: ;while(1){
		call ted_iterat_next

		;while(l_pos>word_n && Col[l_pos-1].Text[pos]!=tex[i].c)
		@@:
			cmp ecx,eax
			jle @f
			dec ecx
			ColToIndexOffset ecx,ebx
			inc ecx
			;cmp byte[ebx+esi],byte[edx]
			mov bl,byte[ebx+esi]
			cmp bl,byte[edx]
			je @f
				dec ecx
			jmp @b
		@@:

		ColToIndexOffset eax,ebx
		cmp byte[ebx+esi],0
		jne .if_0 ;if(Col[word_n].Text[pos]==0){
		mov [endPos],edx ;eP=i;
		ColToIndexOffset eax,ebx
		mov bl,[ebx+MAX_COLOR_WORD_LEN+7]
		mov [f_color],bl ;f_color=Col[word_n].color;

		mov byte[find],1
		ColToIndexOffset eax,ebx ;... ebx = Col[word_n]
		mov bl,[ebx+MAX_COLOR_WORD_LEN+4]
		cmp bl,0 ;if(Col[word_n].wwo)
		je .if_2n
			push edx
			mov edx,[begPos]
			call ted_iterat_perv

			btr bx,0 ;1-1
			jae @f ;if(Col[word_n].wwo&1)
				;u1= !(isalnum(cont_s)||cont_s=='_')
				call isalnum
				jae @f
					mov byte[find],0
					jmp .if_4e
			@@:

			btr bx,3 ;4-1
			jae .if_4e ;if(Col[word_n].wwo&8)
				;u1= !isalpha(cont_s);
				call isalpha
				jae .if_4e
					mov byte[find],0
			.if_4e:

			mov edx,[endPos]
			;call ted_iterat_next

			btr bx,1 ;2-1
			jae @f ;if(Col[word_n].wwo&2)
				;u1= !(isalnum(cont_s)||cont_s=='_')
				call isalnum
				jae @f
					mov byte[find],0
					jmp .if_6e
			@@:

			btr bx,4 ;5-1
			jae .if_6e ;if(Col[word_n].wwo&16)
				;u1= !isalpha(cont_s);
				call isalpha
				jae .if_6e
					mov byte[find],0
			.if_6e:

			btr bx,2 ;3-1
			jae .if_7e ;if(Col[word_n].wwo&4)
				ColToIndexOffset eax,ebx
				mov bx,word[ebx+MAX_COLOR_WORD_LEN+5]
				call ted_iterat_next_pos_char
				cmp edx,ted_tex_1
				jg @f
					;�᫨ ��諨 �� ���� 䠩�� � �� ��諨 ᨬ��� ���� ࠧ��⪨
					call ted_iterat_perv
					mov byte[find],2
				@@:
					mov dword[endPos],edx
			.if_7e:

			pop edx
		.if_2n:
;if(i!=1){ // �� ����� ���㬥��
;  cont_s=tex[eP].c;
;  if(Col[word_n].wwo&2) u2= !(isalnum(cont_s)||cont_s=='_');  // �� �㪢.-��. ᨬ���
;  if(u2 && Col[word_n].wwo&16) u2= !isalpha(cont_s); // �� ��. ᨬ���
;  if(Col[word_n].wwo&4) eP=ted_iterat_next_pos_char(eP,Col[word_n].endc);

			cmp eax,ecx
			je .wh_2e ;if(word_n==l_pos) break; // do double - �᫨ ᫮�� �筮 ��᫥����
		.if_0:

		cmp edx,ted_tex_1
		jle .wh_2e ;if(i==1) break;

		;while(l_pos>word_n && Col[word_n].Text[pos]!=tex[i].c)
		.wh_4b:
		cmp ecx,eax
		jle .wh_4e
			ColToIndexOffset eax,ebx
			;cmp byte[ebx+esi],byte[edx]
			mov bl,byte[ebx+esi]
			cmp bl,byte[edx]
			je .wh_4e
				inc eax
			jmp .wh_4b
		.wh_4e:

		cmp eax,ecx
		je .wh_2e;if(word_n==l_pos) break;
			inc esi ;pos++;
			jmp .wh_2b
		.wh_2e:

		cmp byte[find],0 ;if(fnd)break;
		jne .cycle0end
			mov edx,[begPos];i=bP;
		jmp .cycle0
	.cycle0end:

	cmp byte[find],0
	je .if_1e ;if(fnd){ // �뤥����� ��������� ⥪��
		;if(!mode_sf1 || (mode_sf1 && strlen(Col[word_n].f1->c_str())>0)){
		mov eax,[begPos]
		mov bl,[f_color]
		mov [eax+1],bl ;tex[bP].col=f_color;
		mov eax,[endPos]
		mov byte[eax+1],0xff ;tex[eP].col=255;
		cmp byte[find],2
		je .if_1e
		;return ItPoPerv(eP); // �����頥� ������ ���� �宦�����		
		mov edx,[endPos]
		call ted_get_text_perv_pos
		jmp @f
	.if_1e:
		mov edx,ted_tex
	@@:

	ret
endp

;input:
; edx = pointer to char (byte)
;output:
; cf=1 if symbol is...
align 16
tab_all_num db 0,0,0,0,0,0,0xff,11b,11111110b,0xff,0xff,10000111b,11111110b,0xff,0xff,111b,0,0,0,0,0,0,0,0;,0,0,0,0,0,0,0,0 - tab_alpha_0,0,0,0,0,0,0,0
tab_alpha db 0,0,0,0,0,0,0,0,11111110b,0xff,0xff,10000111b,11111110b,0xff,0xff,111b,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

;output:
; cf=1 �᫨ � [edx] �㪢�, ��� ��� '_'
align 16
isalnum:
	push eax ebx
	movzx eax,byte[edx] ;al=offset
	shr eax,3
	lea ebx,[tab_all_num]
	add ebx,eax
	movzx ax,byte[edx] ;al=bit
	and ax,111b
	bt word[ebx],ax
	pop ebx eax
	ret

;output:
; cf=1 �᫨ � [edx] �㪢� ��� '_'
align 16
isalpha:
	push eax ebx
	movzx eax,byte[edx] ;al=offset
	shr eax,3
	lea ebx,[tab_alpha]
	add ebx,eax
	movzx ax,byte[edx] ;al=bit
	and ax,111b
	bt word[ebx],ax
	pop ebx eax
	ret

align 16
proc ted_show_help_f1 uses eax edx edi, edit:dword
	mov edi,[edit]

	call ted_get_pos_by_cursor
	push edx
		call ted_iterat_next_color_tag
		mov eax,edx
	pop edx
	call ted_iterat_perv_color_tag

	cmp eax,ted_tex
	jle @f
	cmp edx,ted_tex_1
	jle @f
		stdcall ted_find_help_id,eax
	@@:
	;call ted_draw_main_cursor
	call ted_draw_help_f1
	ret 
endp

;input:
; edx = position begin 'symbol' struct
; edi = pointer to tedit struct
; end_pos = position end 'symbol' struct
align 16
proc ted_find_help_id uses ebx ecx, end_pos:dword
; ecx = word_n
; ebx = l_pos
  mov ted_help_id,-1

    movzx ebx,byte[edx]
    shl ebx,2 ;ebx*=4
    add ebx,ted_arr_key_pos
    mov ecx,[ebx]
    cmp ecx,0
    jl .if_0e ;if( (word_n=ted_arr_key_pos[(unsigned char)tf[0]])>-1 ){
      push esi eax
      mov ebx,ecx ;l_pos=word_n;
      ColToIndexOffset ecx,esi
      push cx
      mov cl,[esi]
      @@:
	cmp ebx,ted_key_words_count ;while(l_pos<ted_key_words_count)
	jge @f
	;ColToIndexOffset ecx,esi
	ColToIndexOffset ebx,eax
	cmp cl,[eax] ;&& Col[l_pos].Text[0]==Col[word_n].Text[0])
	jne @f
	  inc ebx ;l_pos++;
	  jmp @b
      @@:
      pop cx
      call ted_iterat_next ;pos=1;
      mov esi,1
      @@:
	push dx
	push word[edx]
	pop dx 
	  .wh_0b:
	    cmp ebx,ecx ;while(l_pos>word_n
	    jle .wh_0e
	    dec ebx
	    ColToIndexOffset ebx,eax
	    inc ebx
	    cmp byte[eax+esi],dl ;&& Col[l_pos-1].Text[pos]!=tf[i])
	    je .wh_0e
	      dec ebx ;l_pos--;
	    jmp .wh_0b
	  .wh_0e:

	  .wh_1b:
	    cmp ebx,ecx ;while(l_pos>word_n
	    jle .wh_1e
	    ColToIndexOffset ecx,eax
	    cmp byte[eax+esi],dl
	    je .wh_1e
	      inc ecx ;word_n++;
	    jmp .wh_1b
	  .wh_1e:
	pop dx

	cmp ecx,ebx ;if(word_n==l_pos) break;
	je @f
	call ted_iterat_next ;pos++;
	cmp edx,[end_pos] ;for(...;i<strlen;...)
	je @f ;jge
	inc esi
	jmp @b
      @@:
      pop eax esi

      mov ted_help_id,ecx
      ;return word_n;

    .if_0e:
  ret
endp

;description:
; �����塞 ࠧ��� ����� ��� ⥪�� (��⠭���� ted_ptr_free_symb �� 1 ᨬ���)
;input:
; ecx - �᫮ ᨬ����� � 䠩��
; edi - pointer to tedit struct
;output:
; eax, ecx - ࠧ�������
align 16
ted_mem_resize:
	add ecx,2 ;������ ��� ⥪�� + �㦥��� ��砫�� � ������ ᨬ����
.no_2:
	add ecx,ted_increase_size ;������ ��� ।���஢���� 䠩��
	mov ted_max_chars,ecx
	imul ecx,sizeof.symbol
	invoke mem.realloc, ted_tex,ecx
	mov ted_tex,eax
	mov ted_tex_1,eax
	add ted_tex_1,sizeof.symbol
	add eax,ecx
	mov ted_tex_end,eax
	mov ecx,ted_tex_1
	add ecx,sizeof.symbol
	mov ted_ptr_free_symb,ecx
	ret

;output:
; eax = ��� �訡��
; ebx = �������⢮ ���⠭��� ����
align 16
proc ted_open_file uses ecx edx edi esi, edit:dword, file:dword, f_name:dword ;�㭪�� ������ 䠩��
	locals
		unpac_mem dd ?
	endl
	mov edi,[edit]

	; *** �஢��塞 ࠧ��� ����� � �᫨ �� 墠⠥� � 㢥��稢��� ***
	;�஡㥬 ������� ���ଠ�� � 䠩��
	mov ebx,[file]
	mov dword[ebx], SSF_GET_INFO
	mov dword[ebx+4], 0
	mov dword[ebx+8], 0
	mov dword[ebx+12], 0
	m2m dword[ebx+16], ted_tex
	mov  byte[ebx+20], 0
	push dword[f_name]
	pop dword[ebx+21]
	mcall SF_FILE
	or eax,eax
	jz .end_0
		mov edx,ted_max_chars
		cmp eax,2 ;�㭪�� �� �����ন������ ��� ������ 䠩����� ��⥬�
		je @f
		jmp .ret_f
align 4
	.end_0:
	;�஢��塞 墠�� �� ����� ��� ����㧪� 䠩��
	mov ecx,ted_max_chars
	sub ecx,2 ;ecx = ���ᨬ��쭮� �᫮ ����, ��� ������ �뫠 �뤥���� ������
	mov edx,ted_tex
	mov edx,[edx+32] ;+32 = +0x20: qword: ࠧ��� 䠩�� � �����
	cmp edx,ecx
	jl @f
		mov ecx,edx
		call ted_mem_resize
	@@:

	; *** �஡㥬 ������ 䠩� ***
	mov ebx,[file]
	mov dword[ebx], SSF_READ_FILE
	mov dword[ebx+4], 0
	mov dword[ebx+8], 0
	m2m dword[ebx+12], edx ;�᫮ ����, ����� ����� ���� ��⠭� � 䠩�� (�� ����� 祬 ted_max_chars)
	m2m dword[ebx+16], ted_tex
	mov  byte[ebx+20], 0
	push dword[f_name]
	pop dword[ebx+21]
	mcall SF_FILE

	or eax,eax
	jz @f
	cmp eax,6
	jne .ret_f
	@@:
	cmp ebx,-1
	je .ret_f
		;if open file
		push eax
		mov eax,ted_tex
		cmp dword[eax],'KPCK'
		jne .end_unpack
			;�뤥����� ����� ��� �ᯠ����� 䠩��
			invoke mem.alloc,[eax+4]
			mov [unpac_mem],eax
			stdcall unpack,ted_tex,[unpac_mem]
			mov ecx,ted_max_chars
			sub ecx,2 ;ecx = ���ᨬ��쭮� �᫮ ����, ��� ������ �뫠 �뤥���� ������
			mov eax,ted_tex
			mov ebx,[eax+4]
			cmp ebx,ecx
			jl @f ;�᫨ ��� �ᯠ�������� 䠩�� �� 墠⠥� �뤥������ �����
				mov ecx,ebx
				call ted_mem_resize
			@@:
			mov edi,ted_tex
			mov esi,[unpac_mem]
			mov ecx,ebx
			cld
			rep movsb
			mov edi,[edit]
			invoke mem.free,[unpac_mem]
		.end_unpack:
		pop eax
		call ted_on_open_file
	.ret_f:
	ret
endp

align 16
proc ted_but_select_word, edit:dword
	pushad
	mov edi,[edit]

	call ted_get_pos_by_cursor
	push edx
		call ted_iterat_perv_color_tag
		cmp edx,ted_tex_1
		jle @f
			call ted_get_text_coords
			mov ted_sel_x0,ebx
			mov ted_sel_y0,eax
		@@:
	pop edx
	call ted_iterat_next_color_tag
	cmp edx,ted_tex_1
	jle @f
		call ted_get_text_coords
		mov ted_sel_x1,ebx
		mov ted_sel_y1,eax
	@@:

	cmp ted_fun_draw_panel_buttons,0
	je @f
		call ted_fun_draw_panel_buttons
	@@:
	stdcall ted_draw,edi
	popad
	ret
endp

;output:
; al = 1 if delete
align 16
proc ted_but_cut uses edi, edit:dword
	mov edi,[edit]

	stdcall ted_but_copy,edi
	call ted_set_undo
	stdcall ted_sel_text_del,ted_opt_ed_change_time

	cmp al,1
	jne @f
		stdcall ted_draw,edi
		cmp ted_fun_draw_panel_buttons,0
		je @f
			call ted_fun_draw_panel_buttons
	@@:
	ret
endp

align 16
proc ted_but_copy, edit:dword
	pushad
	mov edi,[edit]

	call ted_is_select
	or al,al
	jz .end_f ;if not selected text
	call ted_sel_normalize

	mov esi,ted_seln_x1
	mov ecx,ted_seln_y1
	call ted_get_pos_by_coords
	mov ebx,edx
	mov esi,ted_seln_x0
	mov ecx,ted_seln_y0
	call ted_get_pos_by_coords
	mov esi,ebx

	mov ecx,12 ;system buffer header size
	mov ebx,ted_buffer
	mov dword[ebx+4],0 ;text data
	mov dword[ebx+8],1 ;code 866
	add ebx,ecx
	@@:
		cmp edx,ted_tex_1 ;end of file
		jle @f
		cmp edx,esi ;end of select
		je @f
		inc ecx
		cmp ecx,ted_buffer_size ;owerflow bufer
		je @f

		mov al,byte[edx]
		mov byte[ebx],al
		inc ebx
		cmp al,13
		jne .no_13
			mov byte[ebx],10 ;������ ����� ��ப� � ���� 13,10 ��� ᮢ���⨬��� � ��㣨�� �ணࠬ����
			inc ebx
			inc ecx
		.no_13:
		
		call ted_iterat_next
		jmp @b
	@@:
	mov byte[ebx],0

	cmp ecx,12
	je .end_f
		mov ebx,ted_buffer
		mov [ebx],ecx
		mcall SF_CLIPBOARD,SSF_WRITE_CB,ecx,ted_buffer
		call ted_draw_buffer
		cmp ted_fun_draw_panel_buttons,0
		je .end_f
			call ted_fun_draw_panel_buttons
	.end_f:
	popad
	ret
endp


align 16
proc ted_but_paste, edit:dword
	pushad
	mov edi,[edit]

	mcall SF_CLIPBOARD,SSF_GET_SLOT_COUNT
	cmp eax,1
	jl .no_buf_r

	mov esi,eax
	.cycle: ;����� 横� �� ᫮⠬
	dec esi ;����� ⥪�饣�, �஢��塞��� ᫮�
	mcall SF_CLIPBOARD,SSF_READ_CB,esi
	cmp eax,1
	je .no_buf_r
	cmp eax,-1
	je .no_buf_r
		mov ecx,[eax]
		cmp ecx,1 ;size
		jl .no_buf_r
		cmp dword[eax+4],0 ;text
		je @f
			cmp esi,1
			jge .cycle ;�᫨ � ���� �� ⥪��, � ᫮⮢ � ���� ��᪮�쪮, �஡㥬 ��३� � ���孥�� ᫮��
			jmp .no_buf_r
		@@:
		cmp dword[eax+8],1 ;866
		je @f
			cmp esi,1
			jge .cycle ;�᫨ � ���� ⥪�� �� � ����஢�� 866 ... �஡㥬 ��३� � ���孥�� ᫮��
			jmp .no_buf_r
		@@:
		;����஢���� ⥪�� �� ��⥬���� ���� �� ����७���
		cmp ecx,ted_buffer_size
		jle @f
			mov ecx,ted_buffer_size
		@@:
		mov edi,ted_buffer
		mov esi,eax
		add	esi,4 ;12
		mov dword[edi],ecx
		add edi,4 ;12
		sub ecx,4 ;12
		rep movsb
		mov edi,[edit]

		mov esi,eax
		add	esi,12
		jmp .buf_r
	.no_buf_r:
		
	;�᫨ �� 㤠���� ������ ����� �� ��⥬���� ����, �������� �
	mov esi,ted_buffer
	cmp dword[esi],1 ;�஢��塞 ���� �� ����� �� ����७��� ����
	jl .no_paste ;�᫨ ����� ��祣� �� 㤠���� ������ ���� �� ��室
	add esi,12 ;system buffer header size
	.buf_r:
	
	mov edx,esi
	call tl_strlen
	cmp eax,1
	jl .no_paste
		mov esi,eax
		call ted_set_undo
		mov ebx,ted_opt_ed_change_time+ted_opt_ed_move_cursor
		stdcall ted_sel_text_del,ebx
		cmp al,1
		jne .del
			mov ebx,ted_opt_ed_move_cursor
		.del:
		stdcall ted_text_add,edi,edx,esi,ebx
		stdcall ted_draw,edi
		cmp ted_fun_draw_panel_buttons,0
		je .no_paste
			call ted_fun_draw_panel_buttons
	.no_paste:
	popad
	ret
endp

align 16
proc ted_but_sumb_upper uses edi esi, edit:dword
	mov edi,[edit]

	stdcall ted_convert_sel_text,fb_char_toupper
	or esi,esi
	jz @f
		stdcall ted_draw,edi
	@@:
	ret
endp

align 16
proc ted_but_sumb_lover uses edi esi, edit:dword
	mov edi,[edit]

	stdcall ted_convert_sel_text,fb_char_todown
	or esi,esi
	jz @f
		stdcall ted_draw,edi
	@@:
	ret
endp

align 16
proc ted_but_reverse uses eax ebx edi, edit:dword
	mov edi,[edit]

	call ted_is_select
	or al,al
	jz @f
		call ted_sel_normalize
		push esi ecx edx
			mov esi,ted_seln_x0
			mov ecx,ted_seln_y0
			call ted_get_pos_by_coords
			mov eax,edx
			mov esi,ted_seln_x1
			cmp esi,0
			je .beg_str
				dec esi
			.beg_str:
			mov ecx,ted_seln_y1
			call ted_get_pos_by_coords
			;call ted_get_text_perv_pos
			mov ebx,edx
		pop edx ecx esi
		;cmp eax,...
		;je @f
		call ted_revers
	@@:
	stdcall ted_draw,edi
	ret
endp

align 16
proc ted_but_undo uses eax edi, edit:dword
	mov edi,[edit]

	mov eax,ted_tim_undo
	cmp ted_tim_ch,eax
	jbe @f
		inc ted_tim_undo
		;call ted_text_colored
		stdcall ted_draw,edi
		cmp ted_fun_draw_panel_buttons,0
		je @f
			call ted_fun_draw_panel_buttons
	@@:
	ret
endp

align 16
proc ted_but_redo uses edi, edit:dword
	mov edi,[edit]

	cmp ted_tim_undo,1
	jb @f
		dec ted_tim_undo
		;call ted_text_colored
		stdcall ted_draw,edi
		cmp ted_fun_draw_panel_buttons,0
		je @f
			call ted_fun_draw_panel_buttons
	@@:
	ret
endp

;description:
; �㭪�� ��室�� ⥪�� �� ����� 㪠�뢠�� ted_buffer_find
;input:
; f_opt = ��ࠬ���� ���᪠:
;   (0 - �᪠�� ���� �����, 1 - �᪠�� ��� �����, 2 - �᪠�� �� ��砫� 䠩��)
;   �᫨ ��⠭����� 31-� ���, � �� ���������� ����
;   �᫨ ��⠭����� 30-� ���, � ���� ���� ��� ��� ॣ���� ᨬ�����
;output:
; eax = �� �� ������ �᪮�� ⥪�� (0 - ���, 1 - ��)
align 16
proc ted_but_find uses ebx ecx edx edi esi, edit:dword, f_opt:dword
	mov eax,[f_opt]
	push eax
	push [edit]
	cmp al,2
	jne @f
		call _but_find_first
		jmp .end0
	@@:
	cmp al,0
	jne @f
		call _but_find_next
		jmp .end0
	@@:
	cmp al,1
	jne @f
		call _but_find_perv
		jmp .end0
	@@:
	add esp,8
	.end0:

	bt dword[f_opt],31
	jc .end1
	or eax,eax
	jz @f
		;⥪�� ������, ������塞 ����
		stdcall ted_draw,edi
		jmp .end1
	@@:
		;⥪�� �� ������, �஡㥬 �맢��� ᮮ�饭��
		cmp ted_fun_find_err,0
		je .end1
			call ted_fun_find_err ;���짮��⥫�᪠� �㭪��
	.end1:
	ret
endp

;description:
; �㭪�� ��室�� ⥪�� �� ��砫� 䠩��, ��� �� ���� ⥪�饣� �뤥�����
;input:
; f_opt = ��樨 ��� ���᪠
;output:
; eax = �� �� ������ �᪮�� ⥪�� (0 - ���, 1 - ��)
; ebx, ecx, edx, edi, edi - ��������
align 16
proc _but_find_first, edit:dword, f_opt:dword
	mov edi,[edit]

	call ted_is_select
	or al,al
	jz @f
		call ted_sel_normalize
		mov edx,ted_sel_y1
		mov ecx,ted_sel_x1
		call ted_go_to_pos ;���室 �� ����� �뤥�����
		call ted_get_pos_by_cursor
		jmp .end0
	@@:
		mov edx,ted_tex
		call ted_iterat_next
	.end0:
	mov ebx,ted_buffer_find
	@@:
		mov eax,[f_opt]
		call ted_get_find_rezult
		cmp ah,1
		je @f ; find
			call ted_iterat_next
			cmp edx,ted_tex_1
			jle @f
			jmp @b
	@@:
	call _but_find_select
	ret
endp

;description:
; �㭪�� ��室�� ⥪�� ��� �����
;input:
; f_opt = ��樨 ��� ���᪠
;output:
; eax = �� �� ������ �᪮�� ⥪�� (0 - ���, 1 - ��)
; ebx, ecx, edx, edi, edi - ��������
align 16
proc _but_find_perv, edit:dword, f_opt:dword
	mov edi,[edit]

	call ted_is_select
	or al,al
	jz @f
		call ted_sel_normalize
		mov edx,ted_sel_y0
		mov ecx,ted_sel_x0
		call ted_go_to_pos ;���室 �� ��砫� �뤥�����
		call ted_get_pos_by_cursor
		call ted_iterat_perv ;���室 �� 1-� ᨬ��� ��। �뤥������
		jmp .end0
	@@:
	call ted_get_pos_by_cursor
	.end0:
	mov ebx,ted_buffer_find
	@@:
		mov eax,[f_opt]
		call ted_get_find_rezult
		cmp ah,1
		je @f ; find
			call ted_iterat_perv
			cmp edx,ted_tex_1
			jle @f
			jmp @b
	@@:
	call _but_find_select
	ret
endp

;description:
; �㭪�� ��室�� ⥪�� ���� �����
;input:
; f_opt = ��樨 ��� ���᪠
;output:
; eax = �� �� ������ �᪮�� ⥪�� (0 - ���, 1 - ��)
; ebx, ecx, edx, edi, esi - ��������
align 16
proc _but_find_next, edit:dword, f_opt:dword
	mov edi,[edit]

	call ted_get_pos_by_cursor
	mov ebx,ted_buffer_find
	@@:
		mov eax,[f_opt]
		call ted_get_find_rezult
		cmp ah,1
		je @f ; find
			call ted_iterat_next
			cmp edx,ted_tex_1
			jle @f
			jmp @b
	@@:
	call _but_find_select
	ret
endp

;description:
; �ᯮ����⥫쭠� �㭪��, �뤥��� �������� ⥪��
;input:
; ah = �� �� ������ �᪮�� ⥪�� (0 - ���, 1 - ��)
; esi = first symbol pointer
align 16
_but_find_select:
	or ah,ah
	jz @f
		call ted_get_text_coords
		inc ebx ;move cursor right
		mov ted_sel_x1,ebx
		mov ted_sel_y1,eax
		mov edx,eax
		mov ecx,ebx
		call ted_go_to_pos
		mov edx,esi
		call ted_get_text_coords
		mov ted_sel_x0,ebx
		mov ted_sel_y0,eax
		xor eax,eax
		inc eax
		jmp .end0
	@@:
		xor eax,eax ;⥪�� �� ������
	.end0:
	ret

;input:
; rpl_text = ⥪�� ��� ������
; r_opt = ��ࠬ���� ���᪠:
;   (0 - �᪠�� ���� �����, 1 - �᪠�� ��� �����, 2 - �᪠�� �� ��砫� 䠩��)
; n_tim = 䨪�஢��� ������ � ���������� (0 - ���, 1 - ��)
;output:
; eax = 0 - �� 㤠筮, 1 - 㤠筮
align 16
proc ted_but_replace uses edx edi esi, edit:dword, rpl_text:dword, r_opt:dword, n_tim:dword
	mov edi,[edit]
	mov eax,[r_opt]
	bts eax,31
	stdcall ted_but_find, edi,eax
	or eax,eax
	jz .end0

	xor edx,edx
	cmp dword[n_tim],0
	je @f
		call ted_set_undo
		mov edx,ted_opt_ed_change_time
	@@:
	stdcall ted_sel_text_del, edx
	or eax,0xff
	jz .end0
		mov esi,[rpl_text]
		stdcall tl_strlen
		or eax,eax
		jz .end0
		stdcall ted_text_add, edi,esi,eax,ted_opt_ed_move_cursor
		xor eax,eax
		inc eax
	.end0:
	ret
endp

;description:
; �㭪�� �஢���� ᮢ������ �� ⥪�� � ���� ebx
; � ⥪�⮬ ।���� �� 㪠��⥫� edx.
; �⠭����� �㭪樨 (����. strcmp) ��� �� ��������, ��⮬� ��
; � ����� ।���� ⥪�� ᮤ�ন��� �� � ���� ascii ��ப.
;input:
; eax - options to find
; ebx - text need find
; edx - first symbol pointer
; edi - pointer to tedit struct
;output:
; ah - rezult
; edx - last text position (if find sucess)
; esi - first symbol pointer
align 16
proc ted_get_find_rezult uses ebx
	mov al,byte[ebx]
	bt eax,30
	jnc @f
		call fb_char_toupper
	@@:
	mov ah,1
	mov esi,edx ;copy edx
	.cycle0:
		mov cl,al
		mov al,byte[edx]
		bt eax,30
		jnc @f
			call fb_char_toupper
		@@:
		cmp al,cl
		jne .no_text

		inc ebx ;*** get next symbol (in find text) ***
		mov al,byte[ebx]
		or al,al
		jz .end_f ;end of find text
		bt eax,30
		jnc @f
			call fb_char_toupper
		@@:

		call ted_iterat_next ;*** get next symbol (in editor text) ***
		cmp edx,ted_tex_1
		jg .cycle0
align 4
		.no_text:
	xor ah,ah
	mov edx,esi ;restore edx
	.end_f:
	ret
endp

;input:
; edi = pointer to tedit struct
align 16
ted_key_ctrl_home:
	mov ted_cur_x,0
	mov ted_cur_y,0
	push eax
		mov eax,ted_scr_w
		mov dword[eax+sb_offs_position],0
		mov eax,ted_scr_h
		mov dword[eax+sb_offs_position],0
	pop eax
	stdcall ted_draw,edi
	cmp ted_fun_draw_panel_buttons,0
	je @f
		call ted_fun_draw_panel_buttons
	@@:
	ret

;input:
; edi = pointer to tedit struct
align 16
ted_key_ctrl_end:
	push eax ebx
		call ted_get_num_lines
		mov ebx,ted_scr_w
		mov [ebx+sb_offs_position],eax ;�⠢�� ����㭮� �� ��᫥���� ��ப� ���㬥��
		cmp eax,[ebx+sb_offs_cur_area]
		jle @f
			mov eax,[ebx+sb_offs_cur_area] ;����砥� �᫮ ��ப ������� � ����
		@@:
		sub [ebx+sb_offs_position],eax ;�⭨���� �� ����㭪� �᫮ ��ப ������� � ���� (�� �� ����� ��, �� ���� � ���㬥��)
		dec eax
		mov ted_cur_y,eax ;�⠢�� ����� �� ��᫥���� ��ப� ���㬥��
	pop ebx eax
	call ted_cur_move_x_last_char
	stdcall ted_draw,edi
	cmp ted_fun_draw_panel_buttons,0
	je @f
		call ted_fun_draw_panel_buttons
	@@:
	ret

;input:
; edi = pointer to tedit struct
align 16
proc ted_sel_key_up
  cmp ted_drag_k,1
  je @f
    call ted_sel_start
    mov ted_drag_k,1
  @@:
  push dx
    call ted_cur_move_up
    cmp dl,8
    jne @f
      call ted_scroll_set_redraw
    @@:
  pop dx
  call ted_sel_move
  stdcall ted_draw,edi
  ret
endp

;input:
; edi = pointer to tedit struct
align 16
proc ted_sel_key_down
  cmp ted_drag_k,1
  je @f
    call ted_sel_start
    mov ted_drag_k,1
  @@:
  push dx
    call ted_cur_move_down
    cmp dl,8
    jne @f
      call ted_scroll_set_redraw
    @@:
  pop dx
  call ted_sel_move
  stdcall ted_draw,edi
  ret
endp

;input:
; edi = pointer to tedit struct
align 16
proc ted_sel_key_left
	cmp ted_drag_k,1
	je @f
		call ted_sel_start
	@@:
	push dx
		call ted_cur_move_left
		call ted_sel_move
		cmp ted_drag_k,1
		je @f
			mov ted_drag_k,1
			mov dl,8
		@@:
		cmp dl,8
		jne @f
			call ted_scroll_set_redraw
			stdcall ted_draw,edi
			jmp .end_f
		@@:
		stdcall ted_draw_cur_line,edi
		.end_f:
	pop dx
	ret
endp

;input:
; edi = pointer to tedit struct
align 16
proc ted_sel_key_right
	cmp ted_drag_k,1
	je @f
		call ted_sel_start
	@@:
	push dx
		call ted_cur_move_right
		call ted_sel_move
		cmp ted_drag_k,1
		je @f
			mov ted_drag_k,1
			mov dl,8
		@@:
		cmp dl,8
		jne @f
			call ted_scroll_set_redraw
			stdcall ted_draw,edi
			jmp .end_f
		@@:
		stdcall ted_draw_cur_line,edi
		.end_f:
	pop dx
	ret
endp

;input:
; edi = pointer to tedit struct
;description:
; this function need to optimize output
align 16
proc ted_draw_cursor_sumb
pushad
	mov ebx,ted_wnd_l
	add ebx,ted_rec_l
	mov edx,ted_cur_x
	imul edx,ted_rec_w
	add ebx,edx
	shl ebx,16
	add ebx,ted_rec_w

	mov ecx,ted_wnd_t ;calc rect -> y0,y1
	add ecx,ted_rec_t
	mov edx,ted_cur_y
	imul edx,ted_rec_h
	add ecx,edx
	shl ecx,16
	add ecx,ted_rec_h

	mov edx,ted_color_wnd_work
	call ted_sel_normalize

	mov esi,ted_scr_w
	mov eax,[esi+sb_offs_position]
	sub ted_seln_y0,eax
	sub ted_seln_y1,eax

	mov eax,ted_cur_y
	cmp eax,ted_seln_y0
	jl .no_cur_sel
	cmp eax,ted_seln_y1
	jg .no_cur_sel
		mov edx,ted_color_select ;���塞 梥� 䮭� �� 梥� �뤥�����
		mov esi,ted_scr_h
		cmp eax,ted_seln_y0
		jne @f
			mov eax,ted_cur_x
			add eax,[esi+sb_offs_position]
			cmp eax,ted_seln_x0
			jge @f
			mov edx,ted_color_wnd_work
		@@:
		mov eax,ted_cur_y
		cmp eax,ted_seln_y1
		jne .no_cur_sel
			mov eax,ted_cur_x
			add eax,[esi+sb_offs_position]
			cmp eax,ted_seln_x1
			jl .no_cur_sel
			mov edx,ted_color_wnd_work
	.no_cur_sel:
	mcall SF_DRAW_RECT

	call ted_get_pos_by_cursor ;��६ ������ ᨬ����
	cmp ted_gp_opt,2
	jne @f
		mov esi,1
		ror ecx,16
		mov bx,cx
		add ebx,0x10001
		call ted_get_symb_color
		call ted_convert_invis_symb
		mcall SF_DRAW_TEXT ;�ᮢ���� ᨬ����
	@@:
popad
	ret
endp

;input:
; edx -> pointer to text
; edi -> 㪠��⥫� �� �������� tedit
;output:
; ecx = color
; if ted_mode_color=0 then ecx=ted_color_wnd_text
align 16
ted_get_symb_color:
	mov ecx,ted_color_wnd_text ;������ 梥� ⥪�� �� 㬮�砭��

	push eax edx
	cmp ted_mode_color,0
	je .exit
		jmp .on_first
		@@:
			call ted_iterat_perv
			cmp edx,ted_tex_1
			jle .exit
		.on_first:
			movzx eax,byte[edx+1]
			or eax,eax ;�᫨ al=0 � 梥� �� �������
			jz @b

		cmp eax,ted_colors_text_count
		jge .exit

		mov ecx,ted_text_colors ;�ਡ���塞 ᬥ饭�� 1-�� 梥�
		lea ecx,[ecx+4*eax]
		mov ecx,[ecx] ;��⠭�������� ⥪�騩 梥� ⥪�� �� ᬥ饭��
	.exit:
	or ecx,ted_font_size
	pop edx eax
	ret

;input:
; edx = pointer to text
; edi = pointer to tedit struct
;description:
; �㭪�� �८�ࠧ�� �������� ᨬ���� � ���⠥�� �� �࠭�
align 16
ted_convert_invis_symb:
	cmp ted_mode_invis,1
	jne .else
		cmp byte[edx],9
		jne @f
			lea edx,[ted_symbol_tab]
			jmp .end_f
align 4
		@@:
		cmp byte[edx],13
		jne @f
			mov edx,edi
			add edx,ted_offs_symbol_new_line
		@@:
		jmp .end_f
align 4
	.else:
		cmp byte[edx],9
		je @f
		cmp byte[edx],13
		jne .end_f
		@@:
			lea edx,[ted_symbol_space]
	.end_f:
	ret

;input:
; edi = pointer to tedit struct
align 16
ted_scroll_set_redraw:
	push eax
	mov eax,ted_scr_w
	mov dword[eax+sb_offs_redraw],1
	mov eax,ted_scr_h
	mov dword[eax+sb_offs_redraw],1
	pop eax
	ret

align 16
proc ted_draw, edit:dword
	locals
		line_num dd ?
	endl
	pushad
	mov edi,[edit]

	mov eax,SF_DRAW_TEXT
	mov ecx,ted_text_colors
	mov ecx,[ecx]

	mov ebx,ted_wnd_l
	add ebx,ted_rec_l
	shl ebx,16
	add ebx,ted_wnd_t
	add ebx,ted_rec_t
	add ebx,0x10001 ;������塞 ������ ��� ��ࠢ������� �㪢� �� 業���

	call ted_sel_normalize ;need before draw select
	mov esi,ted_scr_w
	mov esi,[esi+sb_offs_position]
	mov [line_num],esi

	stdcall ted_clear_line_before_draw, edi,ebx,1,esi
	call ted_get_first_visible_pos
	or edx,edx
	jz .no_draw_text
	mov esi,1 ;������ �뢮������ ⥪�� �� 1-�� ᨬ����
	@@:
		call ted_iterat_next
		cmp edx,ted_tex_1
		jle .no_draw_text

		; *** 梥⮢�� ࠧ��⪠
		cmp ted_mode_color,0
		je .no_col_change
		cmp byte[edx+1],0
		je .no_col_change
			call ted_get_symb_color
		.no_col_change:

		cmp byte[edx],13
		jne .no_13
			cmp ted_mode_invis,1
			jne .no_invis
				push edx
				mov edx,edi
				add edx,ted_offs_symbol_new_line
				int 0x40
				pop edx
			.no_invis:
			add ebx,ted_rec_h
			;optimized output \/
			mov eax,ted_wnd_h
			add eax,ted_wnd_t
			cmp bx,ax
			jg .no_draw_text
			mov eax,SF_DRAW_TEXT
			;optimized output /\        
			and ebx,0xffff
			ror ebx,16
			add ebx,ted_wnd_l
			add ebx,ted_rec_l
			inc ebx
			ror ebx,16
			inc dword[line_num] ;increment line number
			stdcall ted_clear_line_before_draw,edi,ebx,1,dword[line_num]
			call ted_opt_draw_line_left
			jmp @b
align 4
		.no_13:

		int 0x40
		ror ebx,16
		add ebx,ted_rec_w
		mov esi,ted_wnd_l
		add esi,ted_wnd_w
		cmp bx,si
		jl .no_opt
			call ted_opt_draw_line_right
		.no_opt:
		mov si,1
		ror ebx,16
		jmp @b
	.no_draw_text:

	inc dword[line_num]
	stdcall ted_clear_line_before_draw,edi,ebx,0,dword[line_num]
	call ted_draw_line_numbers
	call ted_draw_main_cursor

;---------------------------------------------
; set all_redraw flag for draw all ScrollBar
; In some cases it is necessity to draw only the area
; of moving of a "runner", for acceleration of output - 
; in this case the flag needs to be reset to 0 (zero).
	mov eax,ted_scr_h
	mov esi,ted_scr_w
	mov dword[eax+sb_offs_all_redraw],1
	mov dword[esi+sb_offs_all_redraw],1

; �ᮢ���� ����� �ப��⪨
	stdcall scroll_bar_horizontal.draw,eax ;[scrollbar_hor_draw]
	stdcall scroll_bar_vertical.draw,esi ;[scrollbar_ver_draw]
; reset all_redraw flag 
	mov dword[eax+sb_offs_all_redraw],0
	mov dword[esi+sb_offs_all_redraw],0
;---------------------------------------------

	;left-bottom square
	mov ebx,ted_wnd_l
	shl ebx,16
	add ebx,ted_rec_l
	mov ecx,ted_wnd_t
	add ecx,ted_wnd_h
	shl ecx,16
	mov cx,word[eax+sb_offs_size_y]
	inc cx
	mcall SF_DRAW_RECT,,,ted_color_wnd_capt ;[sc.work]

	;right-bottom square
	mov ebx,ted_wnd_l
	add ebx,ted_wnd_w
	shl ebx,16
	mov bx,word[esi+sb_offs_size_x]
	inc bx
	int 0x40

	cmp ted_fun_draw_panels,0
	je @f
		stdcall ted_fun_draw_panels, edi
	@@:
	popad
	ret
endp

;input:
; edi = pointer to tedit struct
align 16
proc ted_draw_main_cursor
pushad
	mov eax,SF_DRAW_RECT ;draw cursor
	mov ecx,ted_wnd_t ;calc rect -> y0,y1
	add ecx,ted_rec_t
	mov edx,ted_cur_y
	imul edx,ted_rec_h
	add ecx,edx

	shl ecx,16
	add ecx,ted_rec_h

	mov ebx,ted_wnd_l ;calc rect -> x0,x1
	add ebx,ted_rec_l
	mov edx,ted_cur_x
	imul edx,ted_rec_w
	add ebx,edx
	shl ebx,16
	add ebx,ted_rec_w
	cmp ted_cur_ins,1 ;�஢�ઠ ०��� ࠡ��� ����� (����� ��� ��⠢��)
	jne @f
		shr bx,2 ;㬥��蠥� �ਭ� �����
	@@:

	mov edx,ted_color_cursor
	int 0x40 ;�뢮� �����

	call ted_get_pos_by_cursor
	cmp ted_gp_opt,2
	jne @f
		mov esi,1
		ror ecx,16
		mov bx,cx
		add ebx,0x10001
		mov ecx,ted_color_cur_text
		or ecx,ted_font_size
		call ted_convert_invis_symb
		mcall SF_DRAW_TEXT
	@@:

	mov ebx,ted_wnd_l
	add ebx,ted_rec_l
	shl ebx,16
	add ebx,ted_wnd_t
	add ebx,3
	mov ecx,ted_color_wnd_bord
	or  ecx,0x80000000
	lea edx,[txtRow]
	mcall SF_DRAW_TEXT ;�뢮� ������ '��ப�'

	add ebx,0x500000
	lea edx,[txtCol]
	int 0x40 ;�뢮� ������ '����'

	cmp ted_tim_undo,0
	je @f
		add ebx,0x500000
		lea edx,[txtOtm]
		int 0x40
		sub ebx,0x500000
	@@:

	call ted_draw_buffer
	call ted_draw_help_f1

	mov eax,47 ;draw cursor coords
	mov esi,ted_color_wnd_bord
	or  esi,0x40000000

	mov edx,ebx
	ror edx,16
	sub edx,35
	ror edx,16
	;add edx,3
	mov ebx,0x40000 ;Row=...
	mov ecx,ted_scr_w
	mov ecx,[ecx+sb_offs_position]
	add ecx,ted_cur_y
	inc ecx

push edi
	mov edi,ted_color_wnd_work
	int 0x40 ;�뢮� �᫠ ⥪�饩 ��ப�
pop edi

	;mov ebx,0x40000 ;Col=...
	mov ecx,ted_scr_h
	mov ecx,[ecx+sb_offs_position]
	add ecx,ted_cur_x
	inc ecx
	add edx,0x500000
push edi
	mov edi,ted_color_wnd_work
	int 0x40 ;�뢮� �᫠ ������
pop edi

	cmp ted_tim_undo,0
	je @f
		mov ecx,ted_tim_undo
		add edx,0x500000
		mov edi,ted_color_wnd_work ;���⨬ ॣ���� edi, �� � ���� �㭪樨 �� 㦥 �� �����
		int 0x40 ;�뢮� �᫠ �⬥������ ����⢨�
	@@:

popad
	ret
endp

;input:
; edi = pointer to tedit struct
align 16
proc ted_draw_buffer
	pushad

	mov eax,ted_buffer
	cmp dword[eax],1 ;ᬮ�ਬ ࠧ��� ����
	jl @f
		mov ebx,ted_rec_l
		add bx,300
		cmp ebx,ted_wnd_w ;�ࠢ������ ���न���� ��� �뢮� ⥪��
		jge @f ;������� �� ������ � ����

		add ebx,ted_wnd_l
		shl ebx,16
		add ebx,ted_wnd_t
		add ebx,3
		mov ecx,ted_color_wnd_bord
		or ecx,0x40000000

		mov edx,ted_buffer
		add edx,12
		mov esi,edx
		mov edi,ted_color_wnd_work ;edi - destroy not pointer to tedit
		call tl_strlen
		;cmp eax,0 ;���� ����
		;je @f
		cmp eax,20
		jle .crop_buf
			mov eax,20 ;��१�� ������ �� 20 ᨬ�����
		.crop_buf:
		mov esi,eax
		mcall SF_DRAW_TEXT ;�뢮� ᮤ�ন���� ����

		sub ebx,50 shl 16
		lea edx,[txtBuf]
		mov esi,edx
		call tl_strlen
		mov esi,eax
		xor ecx,0x40000000 ;㡨ࠥ� 梥� 䮭�
		mcall SF_DRAW_TEXT ;�뢮� ������ ��� ����
	@@:
	popad
	ret
endp

;input:
; edi = pointer to tedit struct
align 16
proc ted_draw_help_f1
	pushad
	cmp ted_rec_t,13 ;�������쭠� ���� ��� �ᮢ���� �ࠢ��
	jle @f
		;clear place before draw help
		mov ebx,ted_wnd_l
		add ebx,ted_rec_l
		shl ebx,16
		add ebx,ted_wnd_w
		sub ebx,ted_rec_l
		mov ecx,ted_wnd_t
		add ecx,13
		shl ecx,16
		add ecx,9 ;9 - ���� 0-�� ����, �⠢��� ted_rec_h ���� �� ࠭�
		mcall SF_DRAW_RECT,,,ted_color_wnd_capt

	cmp ted_help_id,-1
	je @f
		mov eax,ted_help_id
		ColToIndexOffset eax,edx

		;SetCoordinates
		mov ebx,ted_wnd_l
		add ebx,ted_rec_l
		shl ebx,16
		add ebx,ted_wnd_t
		add ebx,13 ;=3+10

		;SetTextColor
		movzx eax,byte[edx+MAX_COLOR_WORD_LEN+7]
		shl eax,2
		mov ecx,ted_text_colors
		add ecx,eax
		mov ecx,[ecx]
		or	ecx,0xc0000000 ;SetTextStyles
		mov esi,edi
		mcall SF_DRAW_TEXT,,,,,ted_color_wnd_work
		mov edi,esi

		mov esi,edx
		call tl_strlen

		;*** draw help string ***
		mov ecx,ted_color_wnd_bord
		or ecx,0x80000000
		mov edx,[edx+MAX_COLOR_WORD_LEN]
		or edx,edx
		jz @f
			add edx,ted_help_text_f1
			inc eax
			imul eax,6 ;�ਭ� ᨬ���� � ���. ����
			shl eax,16			
			add ebx,eax
			mcall SF_DRAW_TEXT
	@@:
	popad
	ret
endp

;input:
; edi = pointer to tedit struct
align 16
proc ted_draw_line_numbers
pushad
	;top panel with caption
	mov ebx,ted_wnd_l
	;add ebx,ted_rec_l
	shl ebx,16
	add ebx,ted_wnd_w
	;sub ebx,ted_rec_l
	mov edx,ted_color_wnd_work
	mov ecx,ted_wnd_t
	shl ecx,16
	add ecx,ted_rec_t
	mov edx,ted_color_wnd_capt
	mcall SF_DRAW_RECT

	;line numbers
	mov ebx,0x40000 ;format
	mov ecx,ted_scr_w
	mov ecx,[ecx+sb_offs_position]
	inc ecx
	mov edx,3
	add edx,ted_wnd_l
	rol edx,16
	add edx,ted_wnd_t
	add edx,ted_rec_t
	@@:

push ebx ecx edx
	;left panel with numbers
	mov ebx,ted_wnd_l
	shl ebx,16
	add ebx,ted_rec_l
	mov ecx,ted_rec_h
	rol ecx,16
	mov cx,dx
	rol ecx,16
	mov edx,ted_color_wnd_capt
	mcall SF_DRAW_RECT ;��㥬 ��אַ㣮�쭨� ��� ����஬ ��ப�
pop edx ecx ebx

		mov esi,ted_color_wnd_bord
		mcall SF_DRAW_NUMBER ;��㥬 ����� ��ப�
		inc ecx
		add edx,ted_rec_h
		sub edx,ted_wnd_t
		mov esi,edx
		and esi,0xffff
		cmp esi,ted_wnd_h
		jge @f
		add edx,ted_wnd_t
		jmp @b
align 4
	@@:
popad
	ret
endp

;output:
; ah = symbol
align 16
proc KeyConvertToASCII uses ebx, table:dword
	mov ebx,[table] ;convert scan to ascii
	shr ax,8
	add bx,ax ;? ebx,eax
	mov ah,byte[ebx]
	ret
endp

align 16
proc ted_draw_cur_line, edit:dword
pushad
	mov edi,[edit]

	mov ebx,ted_wnd_l
	add ebx,ted_rec_l
	shl ebx,16
	mov eax,ted_cur_y
	imul eax,ted_rec_h
	mov bx,ax
	add ebx,ted_wnd_t
	add ebx,ted_rec_t ;ebx - ���न���� ��� ��אַ㣮�쭨�� ���⪨ �����
	add ebx,0x10001   ;������塞 ������ ��� ��ࠢ������� �㪢� �� 業���

	call ted_sel_normalize ;need before draw select
	mov ecx,ted_cur_y
	mov eax,ted_scr_w
	add ecx,[eax+sb_offs_position]
	stdcall ted_clear_line_before_draw,edi,ebx,1,ecx

	mov eax,ted_scr_h
	mov esi,[eax+sb_offs_position]
	call ted_get_pos_by_coords

	cmp ted_gp_opt,2
	jne .no_draw_text
	call ted_get_symb_color
	mov esi,1 ;draw 1 symbol
	@@:
		;call ted_iterat_next
		cmp edx,ted_tex_1
		jle .no_draw_text

		; *** 梥⮢�� ࠧ��⪠
		cmp ted_mode_color,0
		je .no_col_change
		cmp byte[edx+1],0
		je .no_col_change
			call ted_get_symb_color
		.no_col_change:

		mov eax,SF_DRAW_TEXT
		cmp byte [edx],13
		jne .no_13
			cmp ted_mode_invis,1
			jne .no_draw_text
			push edx
			mov edx,edi
			add edx,ted_offs_symbol_new_line
			int 0x40
			pop edx
			jmp .no_draw_text
align 4
		.no_13:

		int 0x40
		ror ebx,16
		add ebx,ted_rec_w
		mov eax,ted_wnd_w
		add eax,ted_wnd_l ;ax = ����� �� �� x
		cmp bx,ax
		jge .no_draw_text ;Opt
		ror ebx,16
		call ted_iterat_next
		jmp @b
align 4
	.no_draw_text:

	call ted_draw_main_cursor
popad
	ret
endp

;input:
;  clear_o - �᫨ =1 ������ ���� ��ப�, =0 ������ �� ��ப� ���� �� ����
align 16
proc ted_clear_line_before_draw, edit:dword, coords:dword, clear_o:dword, numb_lin:dword
	pushad
	mov edi,[edit]
	mov ebx,[coords] ;ebx = x*2^16+y coords to left up point clear line
	mov esi,[numb_lin] ;esi - number text line

	sub ebx,0x10001 ;�⭨���� ������ ��� ��ࠢ������� �㪢� �� 業���
	cmp dword[clear_o],0
	jne @f
		add ebx,ted_rec_h
		ror ebx,16
		xor bx,bx
		add ebx,ted_wnd_l
		add ebx,ted_rec_l ;bx = ted_wnd_l+ted_rec_l
		ror ebx,16
	@@:

	mov eax,ted_wnd_h
	add eax,ted_wnd_t
	cmp ax,bx
	jl .no_clear
	sub ax,bx

	mov cx,bx
	shl ecx,16

	xor bx,bx
	add ebx,ted_wnd_w
	sub ebx,ted_rec_l
	xor cx,cx
	add ecx,ted_rec_h
	mov edx,ted_color_wnd_work

	cmp dword[clear_o],0
	je .pusto
	cmp ax,cx
	jge @f
	.pusto:
		mov cx,ax
		jmp .no_select ;�᫨ ���⪠ ���� �� ����, � �ᥣ�� 䮭��� 梥⮬
	@@:

	call ted_is_select
	or al,al
	jz .no_select
	cmp ted_seln_y0,esi
	jg .no_select
	cmp ted_seln_y1,esi
	jl .no_select
		mov edx,ted_color_select ;draw selected line
	.no_select:

	mcall SF_DRAW_RECT ;����᪠ ������ ��ப� 梥⮬ 䮭� ��� 梥⮬ �뤥�����

	call ted_is_select
	or al,al
	jz .no_clear

	mov al,SF_DRAW_RECT
	xor cx,cx
	add ecx,ted_rec_h
	cmp ted_seln_y0,esi
	jne @f
		push bx esi
		mov edx,ted_seln_x0 ; ������ ����� (���ࠥ� ᫥��)
		mov esi,ted_scr_h
		cmp edx,[esi+sb_offs_position]
		jle .in_wnd
			sub edx,[esi+sb_offs_position]
			imul edx,ted_rec_w
			mov bx,dx
			jmp .no_wnd
		.in_wnd:
		xor bx,bx
		.no_wnd:
		mov edx,ted_color_wnd_work
		int 0x40
		pop esi bx
	@@:
	cmp ted_seln_y1,esi
	jne @f
		;push esi
		;�᫨ �뤥��� ���� 䠩� ��� ����� ᤥ���� ��室, �� ⮣�� �㦭� ��� ���� jmp .no_select
		mov edx,ted_seln_x1 ; ������ ����� (���ࠥ� �ࠢ�)
		mov esi,ted_scr_h
		cmp edx,[esi+sb_offs_position]
		jle .in_wnd2
			sub edx,[esi+sb_offs_position]
			imul edx,ted_rec_w
			sub bx,dx
			shl edx,16
			add ebx,edx
		.in_wnd2:
		mov edx,ted_color_wnd_work
		int 0x40
		;pop esi
	@@:

	.no_clear:
	popad
	ret
endp

;input:
; edi = pointer to tedit struct
;output:
; ecx = ���� 梥� ᨬ����
; edx = pointer to symbol
; edx = 0 if text not in screen
align 16
ted_get_first_visible_pos:
	push eax ecx
	mov eax,ted_scr_w
	mov edx,ted_tex
	xor ecx,ecx
	@@:
		cmp ecx,[eax+sb_offs_position]
		je @f
		call ted_iterat_next
		cmp edx,ted_tex_1
		jle @f
		cmp byte[edx],13
		jne @b
		inc ecx
		jmp @b
align 4
	@@:

	cmp ecx,[eax+sb_offs_position]
	je @f
		xor edx,edx
	@@:
	cmp ecx,[eax+sb_offs_max_area]
	jle @f
		mov [eax+sb_offs_max_area],ecx
	@@:
	pop ecx eax
	call ted_opt_draw_line_left
	ret

;input:
; edx = pointer to symbol
; edi = pointer to tedit struct
;output:
; ecx = 梥� ᨬ����
; edx = 㪠��⥫� �� ���� ���� ᨬ���
;description:
; �㭪�� �㦭� ��� ��⨬���樨 �뢮�� ⥪��
align 16
proc ted_opt_draw_line_left uses ebx
	mov ebx,ted_scr_h
	mov ebx,[ebx+sb_offs_position]
	or ebx,ebx
	jz .ret_f
	push eax
	mov eax,edx

	cmp edx,ted_tex
	jne @f
		call ted_iterat_next
		jmp .beg_cycle
	@@:

	or ebx,ebx
	jz @f

	cmp byte[edx],13
	jne @f
		call ted_iterat_next
		.beg_cycle:
	@@:
		cmp edx,ted_tex_1
		jle @f
		cmp byte[edx],13
		je @f
		or ebx,ebx
		jz @f
;--------------------------------------
;eax �㤥� ��������
movzx eax,byte[edx+1]
or eax,eax
jz .no_color
cmp eax,ted_colors_text_count
jge .no_color
	movzx ecx,byte[edx+1]
	shl ecx,2
	add ecx,ted_text_colors
	mov ecx,[ecx]
.no_color:
;--------------------------------------
		mov eax,edx
		call ted_iterat_next
		dec ebx
		jmp @b
align 4
	@@:
		mov edx,eax
	pop eax
	.ret_f:
	call ted_get_symb_color
	ret
endp

;input:
; edx = pointer to symbol
; edi = pointer to tedit struct
;output:
; ecx = symbol color
; edx = pointer to 13 symbol
;description:
; �㭪�� �㦭� ��� ��⨬���樨 �뢮�� ⥪��
align 16
proc ted_opt_draw_line_right uses eax
	mov eax,edx
	@@:
		cmp edx,ted_tex_1
		jle @f
		cmp byte[edx],13
		je @f
		mov eax,edx
		call ted_iterat_next
		jmp @b
	@@:
	mov edx,eax ;perv sumbol
	call ted_get_symb_color
	ret
endp

align 16
proc ted_mouse, edit:dword
	pushad
	mov edi,[edit]

	;��ࠡ��뢠�� �஫�����
	mov edx,ted_scr_h
	mov ecx,ted_scr_w

	cmp word[edx+sb_offs_delta2],0
	jne .horizontal

	mov eax,[ecx+sb_offs_max_area]
	cmp eax,[ecx+sb_offs_cur_area]
	jbe .horizontal
	; mouse event for Vertical ScrollBar
	stdcall scroll_bar_vertical.mouse,ecx ;[scrollbar_ver_mouse]
	cmp dword[ecx+sb_offs_redraw],0
	je @f
		mov dword[ecx+sb_offs_redraw],0
		stdcall ted_draw,edi
		jmp .no_in_wnd
	@@:
	cmp word[ecx+sb_offs_delta2],0
	jne .no_in_wnd
	.horizontal:
	mov eax,[edx+sb_offs_max_area]
	cmp eax,[edx+sb_offs_cur_area]
	jbe .other
	; mouse event for Horizontal ScrollBar
	stdcall scroll_bar_horizontal.mouse,edx ;[scrollbar_hor_mouse]
	cmp dword[edx+sb_offs_redraw],0
	je .other
		mov dword[edx+sb_offs_redraw],0
		stdcall ted_draw,edi
		jmp .no_in_wnd
	.other:
	cmp word[ecx+sb_offs_delta2],0
	jne .no_in_wnd
	cmp word[edx+sb_offs_delta2],0
	jne .no_in_wnd

	;��ࠡ��뢠�� ���� ।����
	mcall SF_MOUSE_GET,SSF_BUTTON
	cmp al,1
	jne @f
		mcall SF_MOUSE_GET,SSF_WINDOW_POSITION
		mov ebx,ted_wnd_t
		add ebx,ted_rec_t
		cmp ax,bx
		jl @f ;y_mouse<y_wnd

		sub ebx,ted_rec_t
		add ebx,ted_wnd_h
		cmp bx,ax
		jl @f ;y_mouse>y_wnd

		mov ebx,ted_wnd_l
		add ebx,ted_rec_l
		mov ecx,eax
		shr ecx,16
		cmp cx,bx
		jl @f ;x_mouse<x_wnd

		sub ebx,ted_rec_l
		add ebx,ted_wnd_w
		cmp bx,cx
		jl @f ;x_mouse>x_wnd

		call ted_draw_cursor_sumb
		call ted_wnd_main_click
		jmp .no_in_wnd
	@@:
	mov edx,ted_el_focus
	cmp [edx],edi
	jne @f
		call ted_wnd_main_mouse_scroll ;ᬮ�ਬ �� �ப���� ����� ���
	@@:
	cmp ted_drag_m,0
	je .no_in_wnd
		mov ted_drag_m,0
		stdcall ted_draw,edi
		cmp ted_fun_draw_panel_buttons,0
		je .no_in_wnd
			call ted_fun_draw_panel_buttons
	.no_in_wnd:
	popad
	ret
endp

;input:
; eax -> (x,y)
; edi -> 㪠��⥫� �� �������� tedit
;description:
; �㭪�� ��뢥��� �� ����⨨ ������� ��� � ��������� ����஬ � ���� ।����
align 16
ted_wnd_main_click:
	push ebx ecx edx
	mov ebx,ted_el_focus
	mov [ebx],edi ;�⠢�� 䮪��

	push eax
		shr eax,16
		sub eax,ted_wnd_l
		sub eax,ted_rec_l

		xor edx,edx
		mov ecx,ted_rec_w
		div cx
		;inc eax
		mov ebx,ted_scr_h
		cmp eax,[ebx+sb_offs_cur_area]
		jle @f
			mov eax,[ebx+sb_offs_cur_area]
		@@:
		;dec eax
		mov ted_cur_x,eax
	pop eax

	push eax
		and eax,0xffff
		sub eax,ted_wnd_t
		sub eax,ted_rec_t

		xor edx,edx
		mov ecx,ted_rec_h
		div cx
		inc eax
		mov ebx,ted_scr_w
		cmp eax,[ebx+sb_offs_cur_area]
		jle @f
			mov eax,[ebx+sb_offs_cur_area]
		@@:
		dec eax
		mov ted_cur_y,eax
	pop eax

	cmp ted_drag_m,0
	je @f
		call ted_sel_move
		jmp .sel_move
	@@:
		mov ted_drag_m,1
		call ted_sel_start
	.sel_move:
	pop edx ecx ebx
	ret

;input:
; edi = pointer to tedit struct
align 16
ted_wnd_main_mouse_scroll:
	push eax ebx ecx
	mcall SF_MOUSE_GET,SSF_SCROLL_DATA
	or ax,ax
	jz .no_scroll
		mov ecx,ted_scr_w
		movsx eax,ax
		lea eax,[eax+eax*2] ;㬭����� �� 3 (�᫮ ��ப �ப��⪨ �� ���� �������� ����� ���)
		add eax,[ecx+sb_offs_position]
		mov ebx,[ecx+sb_offs_max_area]
		shl ebx,1
		sub ebx,[ecx+sb_offs_cur_area] ;�⭨���� �������� ����� ����
		shr ebx,1
		cmp eax,ebx
		jae .no_scroll
		mov ebx,ted_cur_y ;������ �����
		sub ebx,eax       ;- ����� ������ �஫�����
		add ebx,[ecx+sb_offs_position] ;+ ���� ������ �஫�����
		bt ebx,31
		jnc @f
			xor ebx,ebx ;�᫨ ����� �⠫ ��� ����, � �⠢�� �� ������ ��ப�
		@@:
		inc ebx
		cmp ebx,[ecx+sb_offs_cur_area]
		jle @f
			mov ebx,[ecx+sb_offs_cur_area] ;�᫨ ����� �⠫ ���� ����, � �⠢�� �� ������ ��ப�
		@@:
		dec ebx
		mov ted_cur_y,ebx
		mov [ecx+sb_offs_position],eax
		stdcall ted_draw,edi
	.no_scroll:
	pop ecx ebx eax
	ret

align 16
proc ted_save_file, edit:dword, file:dword, f_name:dword
pushad
	mov edi,[edit]

	stdcall ted_can_save,edi
	;or al,al
	;jz .no_save

	mov ecx,ted_max_chars
	invoke mem.alloc,ecx
	push eax ;���������� 㪠��⥫� �� �뤥������ ������

	mov edx,ted_tex
	xor ecx,ecx
	@@:
		call ted_iterat_next
		cmp edx,ted_tex_1
		jle @f ;edx = ted_tex or edx = ted_tex+sizeof.symbol
		mov bl,[edx]
		mov byte[eax],bl
		inc eax
		inc ecx
		jmp @b
align 4
	@@:

	or ecx,ecx
	jz @f
		mov ebx,[file]
		pop eax ;�����뢠�� 㪠��⥫� �� �뤥������ ������
		mov [ebx+16],eax
		push eax ;���⭮ ���������� 㪠��⥫� �� �뤥������ ������
		mov dword[ebx], SSF_CREATE_FILE
		mov dword[ebx+4], 0
		mov dword[ebx+8], 0
		mov [ebx+12], ecx
		mov  byte[ebx+20], 0
		push dword[f_name]
		pop dword[ebx+21]
		mcall SF_FILE

		mov ted_err_save,al

		or eax,eax
		jz .no_msg
		cmp ax,10
		jl .zifra_0_9
			mov al,'?'
			sub ax,48
		.zifra_0_9:
		add ax,48
		cmp ted_fun_save_err,0
		je @f
		call ted_fun_save_err
		jmp @f
		.no_msg:
		m2m ted_tim_ls,ted_tim_ch
	@@:

	pop ecx ;�����뢠�� 㪠��⥫� �� �뤥������ ������
	invoke mem.free,ecx
	.no_save:
popad
	ret
endp
