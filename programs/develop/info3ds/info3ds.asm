use32
	org 0
	db 'MENUET01' ;������. �ᯮ��塞��� 䠩�� �ᥣ�� 8 ����
	dd 1, start, i_end, mem, stacktop, file_name, sys_path

version_edit equ 1

include '../../macros.inc'
include '../../proc32.inc'
include '../../KOSfuncs.inc'
include '../../develop/libraries/libs-dev/libimg/libimg.inc'
include '../../load_img.inc'
include '../../load_lib.mac'
include '../../develop/libraries/box_lib/trunk/box_lib.mac'
include '../../develop/libraries/TinyGL/asm_fork/kosgl.inc'
include '../../develop/libraries/TinyGL/asm_fork/opengl_const.inc'
include 'lang.inc' ; Language support for locales: ru_RU (CP866), en_US.
include 'info_fun_float.inc'
include 'info_menu.inc'
include 'data.inc'
include 'convert_stl_3ds.inc'

@use_library mem.Alloc,mem.Free,mem.ReAlloc,dll.Load

ID_ICON_CHUNK_MAIN equ 0 ;������ �������� �����
ID_ICON_CHUNK_NOT_FOUND equ 1 ;������ �� �����⭮�� �����
ID_ICON_DATA equ 2 ;������ ��� ������ �����, �� ��।������� ��������
ID_ICON_POINT equ 8
ID_ICON_POINT_SEL equ 9

FILE_ERROR_CHUNK_SIZE equ -3 ;�訡�� � ࠧ��� �����

include 'info_o3d.inc'

main_wnd_height equ 460 ;���� �������� ���� �ணࠬ��
IMAGE_TOOLBAR_ICON_SIZE equ 21*21*3

align 4
fl255 dd 255.0
open_file_data dd 0 ;㪠��⥫� �� ������ ��� ������ 䠩��� 3ds
open_file_size dd 0 ;ࠧ��� ����⮣� 䠩��
image_data_toolbar dd 0
icon_tl_sys dd 0 ;㪠��⥥�� �� ������ ��� �࠭���� ��⥬��� ������
icon_toolbar dd 0 ;㪠��⥥�� �� ������ ��� �࠭���� ������ ��ꥪ⮢
fn_toolbar db 'toolbar.png',0

align 4
level_stack dd 0
offs_last_timer dd 0 ;��᫥���� ᤢ�� �������� � �㭪樨 ⠩���

align 4
file_3ds: ;��६���� �ᯮ��㥬� �� ����⨨ 䠩��
.offs: dd 0 ;+0 㪠��⥫� �� ��砫� �����
.size: dd 0 ;+4 ࠧ��� ����� (��� 1-�� ��ࠬ��� = ࠧ��� 䠩�� 3ds)
rb 8*MAX_FILE_LEVEL

size_one_list equ 42
list_offs_chunk_del equ 8 ;����� �� ���� 㤠�����
list_offs_chunk_lev equ 9 ;�஢��� ����� (�ய�ᠭ � ����� 㧫�)
list_offs_p_data equ 10 ;㪠��⥫� �� ������� �����
list_offs_text equ 14 ;ᤢ�� ��砫� ⥪�� � ����
buffer rb size_one_list ;���� ��� ���������� ������� � ᯨ᮪ tree1

txt_3ds_symb db 0,0
;--------------------------------------

include 'info_wnd_coords.inc'

align 4
start:
	;--- copy cmd line ---
	mov esi,file_name
	mov edi,openfile_path
@@:
	lodsd
	or eax,eax
	jz @f ;��室, �᫨ 0
	stosd
	jmp @b
@@:
	stosd

	load_libraries l_libs_start,l_libs_end
	;�஢�ઠ �� ᪮�쪮 㤠筮 ���㧨���� ������⥪�
	mov	ebp,lib_0
	.test_lib_open:
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
		mcall SF_TERMINATE_PROCESS ;exit not correct
	@@:
	add ebp,ll_struc_size
	cmp ebp,l_libs_end
	jl .test_lib_open
	mcall SF_STYLE_SETTINGS,SSF_GET_COLORS,sc,sizeof.system_colors
	mcall SF_SET_EVENTS_MASK,0xC0000027

	stdcall [OpenDialog_Init],OpenDialog_data ;�����⮢�� �������

	;kmenu initialisation
	stdcall [kmenu_init],sc
	stdcall [ksubmenu_new]
	mov [main_menu], eax

	stdcall [ksubmenu_new]
	mov [main_menu_view], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Veiw_Vertexes, 5
	stdcall [ksubmenu_add], [main_menu_view], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Veiw_Faces, 6
	stdcall [ksubmenu_add], [main_menu_view], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Veiw_Faces_Fill, 7
	stdcall [ksubmenu_add], [main_menu_view], eax
	stdcall [kmenuitem_new], KMENUITEM_SEPARATOR, 0, 0
	stdcall [ksubmenu_add], [main_menu_view], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Veiw_Light, 8
	stdcall [ksubmenu_add], [main_menu_view], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Veiw_Smooth, 9
	stdcall [ksubmenu_add], [main_menu_view], eax
	stdcall [kmenuitem_new], KMENUITEM_SEPARATOR, 0, 0
	stdcall [ksubmenu_add], [main_menu_view], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Veiw_Reset, 10
	stdcall [ksubmenu_add], [main_menu_view], eax
	stdcall [kmenuitem_new], KMENUITEM_SUBMENU, sz_main_menu_View, [main_menu_view]
	stdcall [ksubmenu_add], [main_menu], eax

	stdcall [ksubmenu_new]
	mov [main_menu_vertexes], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Vertexes_Select, 11
	stdcall [ksubmenu_add], [main_menu_vertexes], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Vertexes_Deselect, 12
	stdcall [ksubmenu_add], [main_menu_vertexes], eax
	stdcall [kmenuitem_new], KMENUITEM_SEPARATOR, 0, 0
	stdcall [ksubmenu_add], [main_menu_vertexes], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Average_x, 13
	stdcall [ksubmenu_add], [main_menu_vertexes], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Average_y, 14
	stdcall [ksubmenu_add], [main_menu_vertexes], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Average_z, 15
	stdcall [ksubmenu_add], [main_menu_vertexes], eax
	stdcall [kmenuitem_new], KMENUITEM_SUBMENU, sz_main_menu_Vertexes, [main_menu_vertexes]
	stdcall [ksubmenu_add], [main_menu], eax

	mov dword[w_scr_t1.type],1
	stdcall [tl_data_init], tree1
	;��⥬�� ������ 16*16 ��� tree_list
	include_image_file 'tl_sys_16.png', icon_tl_sys
	;�᫨ ����ࠦ���� �� ���뫮��, � � icon_tl_sys ����
	;�� ���樠����஢���� �����, �� �訡�� �� �㤥�, �. �. ���� �㦭��� ࠧ���
	mov eax,dword[icon_tl_sys]
	mov dword[tree1.data_img_sys],eax

	load_image_file 'objects.png', icon_toolbar
	mov eax,dword[icon_toolbar]
	mov dword[tree1.data_img],eax

	stdcall [buf2d_create], buf_0 ;ᮧ����� ����

	load_image_file 'font8x9.bmp', image_data_toolbar
	stdcall [buf2d_create_f_img], buf_1,[image_data_toolbar] ;ᮧ���� ����
	stdcall mem.Free,[image_data_toolbar] ;�᢮������� ������
	stdcall [buf2d_conv_24_to_8], buf_1,1 ;������ ���� �஧�筮�� 8 ���
	stdcall [buf2d_convert_text_matrix], buf_1

	load_image_file fn_toolbar, image_data_toolbar

	;ࠡ�� � 䠩��� ����஥�
	copy_path ini_name,sys_path,file_name,0
	mov dword[def_dr_mode],0
	stdcall dword[ini_get_int],file_name,ini_sec_w3d,key_dv,1
	or eax,eax
	jz @f
		or dword[def_dr_mode], 1 shl bit_vertexes
	@@:
	stdcall dword[ini_get_int],file_name,ini_sec_w3d,key_df,1
	or eax,eax
	jz @f
		or dword[def_dr_mode], 1 shl bit_faces
	@@:
	stdcall dword[ini_get_int],file_name,ini_sec_w3d,key_dff,1
	or eax,eax
	jz @f
		or dword[def_dr_mode], 1 shl bit_faces_fill
	@@:
	stdcall dword[ini_get_int],file_name,ini_sec_w3d,key_dl,1
	or eax,eax
	jz @f
		or dword[def_dr_mode], 1 shl bit_light
	@@:
	stdcall dword[ini_get_int],file_name,ini_sec_w3d,key_ds,1
	or eax,eax
	jz @f
		or dword[def_dr_mode], 1 shl bit_smooth
	@@:
	stdcall dword[ini_get_color],file_name,ini_sec_w3d,key_ox,0x0000ff
	mov [color_ox],eax
	stdcall dword[ini_get_color],file_name,ini_sec_w3d,key_oy,0xff0000
	mov [color_oy],eax
	stdcall dword[ini_get_color],file_name,ini_sec_w3d,key_oz,0x00ff00
	mov [color_oz],eax
	stdcall dword[ini_get_color],file_name,ini_sec_w3d,key_bk,0x000000
	mov [color_bk],eax
	shr eax,8
	mov [color_bk+4],eax
	shr eax,8
	mov [color_bk+8],eax
	stdcall dword[ini_get_color],file_name,ini_sec_w3d,key_vert,0xffffff
	mov [color_vert],eax
	stdcall dword[ini_get_color],file_name,ini_sec_w3d,key_face,0x808080
	mov [color_face],eax
	stdcall dword[ini_get_color],file_name,ini_sec_w3d,key_select,0xffff00
	mov [color_select],eax
	finit
	fild dword[color_bk+8]
	fdiv dword[fl255]
	fstp dword[color_bk+8]
	mov eax,[color_bk+4]
	and eax,0xff
	mov [color_bk+4],eax
	fild dword[color_bk+4]
	fdiv dword[fl255]
	fstp dword[color_bk+4]
	mov eax,[color_bk]
	and eax,0xff
	mov [color_bk],eax
	fild dword[color_bk]
	fdiv dword[fl255]
	fstp dword[color_bk]

	mcall SF_SYSTEM_GET,SSF_TIME_COUNT
	mov [last_time],eax

	stdcall [kosglMakeCurrent], 3d_wnd_l,3d_wnd_t,3d_wnd_w,3d_wnd_h,ctx1
	stdcall [glEnable], GL_DEPTH_TEST
	stdcall [glEnable], GL_NORMALIZE ;����� ��ଠ�� ���������� ����稭� �� ��������� ���䠪⮢
	stdcall [glClearColor], [color_bk+8],[color_bk+4],[color_bk],0.0
	stdcall [glShadeModel], GL_SMOOTH
	call [gluNewQuadric]
	mov [qObj],eax

	mov eax,[ctx1.gl_context]
	mov eax,[eax] ;eax -> ZBuffer
	mov eax,[eax+ZBuffer.pbuf]
	mov dword[buf_ogl],eax

	;open file from cmd line
	cmp dword[openfile_path],0
	je @f
		call but_open_file.no_dlg
	@@:
	call draw_window

align 4
still:
	mcall SF_SYSTEM_GET,SSF_TIME_COUNT
	mov ebx,[last_time]
	add ebx,10 ;����প�
	cmp ebx,eax
	jge @f
		mov ebx,eax
	@@:
	sub ebx,eax
	mcall SF_WAIT_EVENT_TIMEOUT
	or eax,eax
	jz timer_funct

	cmp al,1
	jne @f
		call draw_window
		jmp still
	@@:
	cmp al,2
	jz key
	cmp al,3
	jz button
	cmp al,6
	jne @f
		mcall SF_THREAD_INFO,procinfo,-1
		cmp ax,word[procinfo+4]
		jne @f ;���� �� ��⨢��
		call mouse
	@@:
	jmp still

align 4
mouse:
	stdcall [tl_mouse], dword tree1
	ret

align 4
timer_funct:
	pushad
	mcall SF_SYSTEM_GET,SSF_TIME_COUNT
	mov [last_time],eax

	;��ᬠ�ਢ��� �뤥����� ���� ������
	stdcall [tl_node_get_data],tree1
	cmp eax,0
	je @f
		mov ebx,eax
		mov eax,dword[ebx]
		mov ecx,dword[ebx+4] ;ࠧ��� �����
		stdcall hex_in_str, txt_3ds_offs.dig, eax,8
		stdcall hex_in_str, txt_3ds_offs.siz, ecx,8

		add eax,dword[open_file_data] ;����砥� ���祭�� ᤢ��� � �����
		cmp dword[offs_last_timer],eax
		je @f
			;�᫨ �뤥����� ���� ������ �� ᮢ������ � ��᫥���� ����������
			mov dword[offs_last_timer],eax
			stdcall buf_draw_beg, buf_0
			stdcall [buf2d_draw_text], buf_0, buf_1,txt_3ds_offs,5,35,0xb000
			mov edx,dword[ebx+list_offs_p_data]
			cmp edx,0 ;ᬮ�ਬ ���� �� ���ᠭ�� �����
			je .no_info
				stdcall [buf2d_draw_text], buf_0, buf_1,edx,5,45,0xb000
			.no_info:
			add ecx,eax ;����砥� ࠧ��� �����
			stdcall buf_draw_hex_table,eax,ecx ;���������� 16-���� ������
			stdcall [buf2d_draw], buf_0 ;������塞 ���� �� ��࠭�
	@@:
	popad
	jmp still

align 4
proc buf_draw_hex_table, offs:dword, size_line:dword
	pushad
	locals
		coord_y dd 55 ;���न��� y ��� ��砫� �뢮�� ⠡����
	endl
		mov esi,dword[offs]
		mov edi,dword[open_file_data]
		add edi,dword[file_3ds.size] ;edi - 㪠��⥫� �� ����� 䠩�� � �����
		mov dword[txt_3ds_offs.dig],0
		cld
		.cycle_rows:
			mov ebx,5 ;����� ᫥�� ��� ���
			mov edx,5+10*24 ;����� ᫥�� ��� ⥪��
			mov ecx,10
			@@:
				stdcall hex_in_str, txt_3ds_offs.dig, dword[esi],2
				stdcall [buf2d_draw_text], buf_0, buf_1,txt_3ds_offs.dig,ebx,[coord_y],0

				mov al,byte[esi]
				mov byte[txt_3ds_symb],al
				stdcall [buf2d_draw_text], buf_0, buf_1,txt_3ds_symb,edx,[coord_y],0x808080
				inc esi
				cmp esi,dword[size_line]
				jne .end_block
					stdcall draw_block_end_line, dword[coord_y]
				.end_block:
				cmp esi,edi
				jge @f ;jg ???
				add ebx,24
				add edx,9 ;�ਭ� 1-�� ᨬ���� +1pix
				loop @b
			add dword[coord_y],10 ;���� 1-�� ᨬ���� (��� ���ࢠ� ����� ��ப���)
			mov ebx,dword[buf_0.h]
			cmp dword[coord_y],ebx
			jl .cycle_rows
		@@:
	popad
	ret
endp

align 4
proc draw_block_end_line uses eax ebx ecx, coord_y:dword
	add ebx,20 ;20 = width 2.5 symbols
	mov eax,[coord_y]
	sub eax,2
	mov ecx,eax
	add ecx,10
	stdcall [buf2d_line], buf_0, 0,ecx,ebx,ecx ,0xff
	stdcall [buf2d_line], buf_0, ebx,ecx,ebx,eax ,0xff
	stdcall [buf2d_line], buf_0, ebx,eax,5+10*24-4,eax ,0xff
	ret
endp

align 4
draw_window:
pushad
	mcall SF_REDRAW,SSF_BEGIN_DRAW
	xor eax,eax
	mov edx,[sc.work]
	or  edx,0x33000000
	mov edi,capt
	mcall , (20 shl 16)+560, (20 shl 16)+main_wnd_height

	mcall SF_THREAD_INFO,procinfo,-1
	mov eax,dword[procinfo.box.height]
	cmp eax,250
	jge @f
		mov eax,250
	@@:
	sub eax,65
	mov dword[tree1.box_height],eax
	mov word[w_scr_t1.y_size],ax ;���� ࠧ���� �஫�����
	cmp eax,dword[buf_0.h] ;㢥��稢��� ����� ����
	jle @f
		stdcall [buf2d_resize],buf_0,0,eax,1
		mov dword[offs_last_timer],0 ;��� ���������� ���� � ⠩���
	@@:

	mov eax,dword[procinfo.box.width]
	cmp eax,400
	jge @f
		mov eax,400
	@@:
	sub eax,[buf_0.w]
	sub eax,41
	mov dword[tree1.box_width],eax
	add ax,word[tree1.box_left]
	mov word[w_scr_t1.x_pos],ax
	add ax,16+5
	mov word[buf_0.l],ax

	mov esi,[sc.work_button]
	mcall SF_DEFINE_BUTTON,(5 shl 16)+20,(5 shl 16)+20,0x40000003
	mcall ,(30 shl 16)+20,,0x40000004

	cmp byte[can_save],0
	je @f
		mcall ,(55 shl 16)+20,,0x40000005
	@@:
	mcall ,(85 shl 16)+20,,0x40000006 ;���� � ���न��⠬�
	mcall ,(110 shl 16)+20,,0x40000007 ;㤠����� �����

	mcall SF_PUT_IMAGE,[image_data_toolbar],(21 shl 16)+21,(5 shl 16)+5 ;new

	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mov edx,(30 shl 16)+5 ;open
	int 0x40

	cmp byte[can_save],0
	je @f
		add ebx,IMAGE_TOOLBAR_ICON_SIZE
		mov edx,(55 shl 16)+5 ;save
		int 0x40
		sub ebx,IMAGE_TOOLBAR_ICON_SIZE
	@@:

	add ebx,4*IMAGE_TOOLBAR_ICON_SIZE
	mov edx,(85 shl 16)+5
	int 0x40

	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mov edx,(110 shl 16)+5
	int 0x40

	mov dword[w_scr_t1.all_redraw],1
	stdcall [tl_draw], tree1

	stdcall [buf2d_draw], buf_0

	mcall SF_REDRAW,SSF_END_DRAW
popad
	ret

align 4
key:
	mcall SF_GET_KEY
	stdcall [tl_key], dword tree1
	jmp still


align 4
button:
	mcall SF_GET_BUTTON
	cmp ah,3
	jne @f
		call but_new_file
		jmp still
	@@:
	cmp ah,4
	jne @f
		call but_open_file
		jmp still
	@@:
	cmp ah,5
	jne @f
		call but_save_file
		jmp still
	@@:
	cmp ah,6
	jne @f
		call but_wnd_coords
		jmp still
	@@:
	cmp ah,7
	jne @f
		call but_delete_chunk
		jmp still
	@@:

	cmp ah,1
	jne still
.exit:
	mov dword[tree1.data_img],0
	mov dword[tree1.data_img_sys],0
	stdcall [tl_data_clear], tree1
	stdcall [buf2d_delete],buf_0
	stdcall [buf2d_delete],buf_1 ;㤠�塞 ����
	stdcall mem.Free,[image_data_toolbar]
	stdcall mem.Free,[open_file_data]
	stdcall [gluDeleteQuadric], [qObj]
	mcall SF_TERMINATE_PROCESS


align 4
but_new_file:
	mov byte[can_save],0
	stdcall [tl_info_clear], tree1 ;���⪠ ᯨ᪠ ��ꥪ⮢
	stdcall [buf2d_clear], buf_0, [buf_0.color] ;��⨬ ����
	stdcall [tl_draw], tree1
	stdcall [buf2d_draw], buf_0 ;������塞 ���� �� ��࠭�
	ret

align 4
but_open_file:
	copy_path open_dialog_name,communication_area_default_path,file_name,0
	pushad
	mov [OpenDialog_data.type],0
	stdcall [OpenDialog_Start],OpenDialog_data
	cmp [OpenDialog_data.status],2
	je .end_open_file
	;��� �� 㤠筮� ����⨨ �������
	jmp .end0
.no_dlg: ;�᫨ ���㥬 ������ ������ 䠩��
		pushad
		mov esi,openfile_path
		stdcall str_len,esi
		add esi,eax
		@@: ;横� ��� ���᪠ ��砫� ����� 䠩��
			dec esi
			cmp byte[esi],'/'
			je @f
			cmp byte[esi],0x5c ;'\'
			je @f
			cmp esi,openfile_path
			jg @b
		@@:
		inc esi
		stdcall [OpenDialog_Set_file_name],OpenDialog_data,esi ;�����㥬 ��� 䠩�� � ������ ��࠭����
	.end0:
    mov [run_file_70.Function], SSF_GET_INFO
    mov [run_file_70.Position], 0
    mov [run_file_70.Flags], 0
    mov dword[run_file_70.Count], 0
    mov dword[run_file_70.Buffer], open_b
    mov byte[run_file_70+20], 0
    mov dword[run_file_70.FileName], openfile_path
    mcall SF_FILE,run_file_70

    mov ecx,dword[open_b+32] ;+32 qword: ࠧ��� 䠩�� � �����
    stdcall mem.ReAlloc,[open_file_data],ecx
    mov [open_file_data],eax

    mov [run_file_70.Function], SSF_READ_FILE
    mov [run_file_70.Position], 0
    mov [run_file_70.Flags], 0
    mov dword[run_file_70.Count], ecx
    m2m dword[run_file_70.Buffer], dword[open_file_data]
    mov byte[run_file_70+20], 0
    mov dword[run_file_70.FileName], openfile_path
    mcall SF_FILE,run_file_70 ;����㦠�� 䠩� 3ds
    cmp ebx,0xffffffff
    je .end_open_file

	mov [open_file_size],ebx
		;mcall SF_SET_CAPTION,1,openfile_path

	mov byte[can_save],0
	call init_tree
	stdcall [buf2d_draw], buf_0 ;������塞 ���� �� ��࠭�
	stdcall obj_clear_param, o3d ;��⨬ ��ࠬ���� ���� � ���न��⠬�
	cmp byte[prop_wnd_run],0
	je @f
		;��⨬ ���� � ���न��⠬�
		stdcall [tl_info_clear], tree3
	@@:

	.end_open_file:
	popad
	ret

align 4
init_tree:
	stdcall [tl_info_clear], tree1 ;���⪠ ᯨ᪠ ��ꥪ⮢

	mov esi,[open_file_data]
	stdcall convert_stl_3ds, esi,[open_file_size] ;�஢��塞 䠩� �ଠ� *.stl ?
	or eax,eax
	jz @f
		;�᫨ 䠩� � �ଠ� *.stl
		mov [open_file_size],ecx
		mov esi,eax
		stdcall mem.Free,[open_file_data]
		mov [open_file_data],esi
		mov byte[can_save],1
	@@:
	cmp word[esi],CHUNK_MAIN
	je @f
		stdcall buf_draw_beg, buf_0
		stdcall [buf2d_draw_text], buf_0, buf_1,txt_no_3ds,5,25,0xff0000 ;��㥬 ��ப� � ⥪�⮬
		jmp .end_open
	@@:
	;--- ��ࠡ�⪠ ����⮣� *.3ds 䠩��
	mov eax,file_3ds
	mov dword[level_stack],0 ;����塞 �஢��� �⥪�
	mov dword[offs_last_timer],0
	;--- ���������� �������� ����� � ᯨ᮪
	stdcall add_3ds_object, ID_ICON_CHUNK_MAIN,0,dword[esi+2],0
	call block_children ;�室 � ���୨� ����

	mov edi,[file_3ds.offs]
	add edi,[file_3ds.size]
	.cycle_main:
		cmp dword[level_stack],0
		jle .end_cycle

		cmp esi,edi ;�᫨ ����� 䠩��
		jge .end_cycle

		mov edx,[esi+2] ;ࠧ��� �����
		call block_analiz
		cmp dword[bl_found],0
		jne @f
			;��ꥪ� �� �����⭮�� ����
			stdcall add_3ds_object, ID_ICON_CHUNK_NOT_FOUND,dword[level_stack],edx,0
			call block_next
			jmp .cycle_main
		@@:
			;��ꥪ� �����⭮�� ����
			mov ecx,dword[bl_found]
			mov bx,word[ecx+2] ;����� ������ ��� ��ꥪ�
			stdcall add_3ds_object, ebx,dword[level_stack],edx,dword[ecx+5]
			cmp byte[ecx+4],1
			je .bl_data
				;���� ᮤ�ন� ���୨� �����
				call block_children ;�室 � ���୨� ����
				jmp .cycle_main
			.bl_data:
				;���� ᮤ�ন� �����
				call block_analiz_data
				jmp .cycle_main
	.end_cycle:
		stdcall [tl_cur_beg], tree1
		stdcall [tl_draw], tree1
	.end_open:
	ret

;������ ������ �����
;input:
; eax - stack pointer
; esi - memory pointer
;output:
; eax - new stack pointer
; esi - new memory pointer
align 4
block_analiz_data:
	push ebx ecx edx edi
		mov dx,word[esi]
		mov ecx,dword[esi+2]
		sub ecx,6 ;ࠧ��� ������ � �����
		add esi,6
		mov ebx,dword[level_stack]
		inc ebx
		; *** ������ ������ � ࠧ�묨 ����묨 � �뤥������ ���������
		cmp dx,CHUNK_OBJBLOCK ;��ꥪ�
		jne @f
			push ax
				cld
				xor al,al
				mov edi,esi
				repne scasb
			pop ax
			sub edi,esi ;edi - strlen
			stdcall add_3ds_object, ID_ICON_DATA,ebx,edi,0 ;�������� ��ꥪ�
			add esi,edi
			;sub ecx,edi ;㦥 ᤥ���� � repne
			jmp .next_bl
		@@:
		cmp dx,CHUNK_VERTLIST ;ᯨ᮪ ���設
		je .vertexes
		cmp dx,0x4111 ;䫠�� ���設
		je .vertexes
		cmp dx,CHUNK_MAPLIST ;⥪����� ���न����
		je .vertexes
		jmp @f
		.vertexes: ;��ࠡ�⪠ ������, ᮤ�ঠ�� ����� ���設
			stdcall add_3ds_object, ID_ICON_DATA,ebx,2,txt_count ;�᫮ ���設
			add esi,2
			sub ecx,2
			stdcall add_3ds_object, ID_ICON_DATA,ebx,ecx,0 ;����� ���設
			sub esi,8 ;����⠭������� esi
			call block_next
			jmp .end_f
		@@:
		cmp dx,CHUNK_FACELIST ;ᯨ᮪ �࠭��
		jne @f
			stdcall add_3ds_object, ID_ICON_DATA,ebx,2,txt_count ;�᫮ �࠭��
			push eax
			movzx eax,word[esi]
			shl eax,3
			add esi,2
			sub ecx,2
			stdcall add_3ds_object, ID_ICON_DATA,ebx,eax,0 ;����� �࠭��

			sub ecx,eax
			cmp ecx,1
			jl .data_3 ;�஢��塞 ���� �� ���� ����뢠�騩 ���ਠ�, �ਬ��塞� � ��ꥪ��
if 0
				add esi,eax
				mov ecx,dword[esi+2]
				stdcall add_3ds_object, 10,ebx,ecx,0 ;����� ���ਠ��
				sub esi,eax
else
				add esi,eax
				pop eax
				jmp .next_bl
end if
			.data_3:

			sub esi,8 ;����⠭������� esi
			pop eax
			call block_next
			jmp .end_f
		@@:
		cmp dx,CHUNK_FACEMAT ;���ਠ�� �࠭��
		jne @f
			push ax
				cld
				xor al,al
				mov edi,esi
				repne scasb
			pop ax
			sub edi,esi ;edi - strlen
			stdcall add_3ds_object, ID_ICON_DATA,ebx,edi,0 ;�������� ��ꥪ�
			add esi,edi
			;sub ecx,edi ;㦥 ᤥ���� � repne
			stdcall add_3ds_object, ID_ICON_DATA,ebx,2,txt_count ;�᫮ �࠭��
			add esi,2
			sub ecx,2
			stdcall add_3ds_object, ID_ICON_DATA,ebx,ecx,0 ;����� �࠭��, � ����� �ਬ���� ���ਠ�
			sub esi,edi ;����⠭������� esi (1)
			sub esi,8   ;����⠭������� esi (2)
			call block_next
			jmp .end_f
		@@:
		; *** ������ ����� � ����묨 �� 㬮�砭�� (��� �뤥����� ���������)
			stdcall add_3ds_object, ID_ICON_DATA,ebx,ecx,0
			sub esi,6 ;����⠭������� esi
			call block_next
			jmp .end_f
		.next_bl:
		; *** ����ன�� ��� ������� ��⠢���� ���������
			mov dword[eax],esi ;㪠��⥫� �� ��砫� �����
			mov ebx,dword[esi+2]
			mov dword[eax+4],ebx ;ࠧ��� �����
			inc dword[level_stack]
			add eax,8
		.end_f:
	pop edi edx ecx ebx
	ret

;�室 � 1-� ���୨� ����
;input:
; eax - 㪠��⥫� �� �६���� �⥪ 䠩�� file_3ds
; esi - ��砫� த�⥫�᪮�� �����
;output:
; ebx - destroy
; esi - ��砫� ������ த�⥫�᪮�� �����
align 4
block_children:
	push ecx
		;�஢�ઠ �ࠢ��쭮�� ࠧ��஢ ���୥�� �����
		mov ebx,esi
		add ebx,6 ;���室 �� ��砫� ���୥�� �����
		add ebx,dword[ebx+2] ;������塞 ࠧ��� ���୥�� �����
		mov ecx,esi
		add ecx,dword[esi+2] ;������塞 ࠧ��� த�⥫�᪮�� �����
		cmp ebx,ecx ;���뢠�� ��������� �� �㦭�, �. �. �ࠢ�������� ⮫쪮 ����� ������
		jle @f
			;��������஢��� �訡�� 䠩��, ���୨� ���� ��室�� �� �।��� த�⥫�᪮��
			mov dword[level_stack],FILE_ERROR_CHUNK_SIZE
			jmp .end_f
		@@:
		mov [eax],esi ;㪠��⥫� �� ��砫� �����
		mov ebx,[esi+2]
		mov [eax+4],ebx ;ࠧ��� �����
		add esi,6 ;���室�� � ����� �����
		inc dword[level_stack]
		add eax,8
	.end_f:
	pop ecx
	ret

;���室 � ᫥��饬� ����� ⥪�饣� �஢��
;input:
; eax - ���� �������� � ��६���묨
align 4
block_next:
push ebx
	add esi,dword[esi+2] ;�ய�᪠�� ����� �����

	;�஢�ઠ ࠧ��஢ த�⥫�᪮�� �����, ��� ���������� ��室� �� ���孨� �஢��� �᫨ ����� �����
	@@:
	mov ebx,dword[eax-8]
	add ebx,dword[eax-4]
	cmp esi,ebx
	jl @f
		dec dword[level_stack]
		sub eax,8
		cmp dword[level_stack],0
		jg @b
	@@:
pop ebx
	ret

;�㭪�� ���᪠ �������� ����뢠�饩 ����
;input:
;esi - memory pointer
;output:
;dword[bl_found] - pointer to chunk struct (= 0 if not found)
align 4
bl_found dd 0
block_analiz:
pushad
	mov dword[bl_found],0
	mov ecx,type_bloks
	@@:
		mov bx,word[ecx]
		cmp word[esi],bx
		je .found
		add ecx,sizeof.block_3ds
		cmp ecx,type_bloks.end
		jl @b
	jmp .no_found
	.found:
		mov dword[bl_found],ecx
	.no_found:
popad
	ret

;input:
; esi - 㪠��⥫� �� ��������㥬� �����
; icon - ����� ������
; level - �஢��� ���������� 㧫�
; size_bl - ࠧ��� �����
; info_bl - ��ப� � ���ᠭ��� �����
align 4
proc add_3ds_object, icon:dword, level:dword, size_bl:dword, info_bl:dword
	pushad
		mov bx,word[icon]
		shl ebx,16
		mov bx,word[level]

		mov eax,esi
		sub eax,dword[open_file_data]
		mov dword[buffer],eax ;ᬥ饭�� �����
		mov ecx,dword[size_bl]
		mov dword[buffer+4],ecx ;ࠧ��� ����� (�ᯮ������ � �㭪樨 buf_draw_hex_table ��� �ᮢ���� �����)
		mov ecx,dword[bl_found]
		or ecx,ecx
		jz @f
			;... ����� �㦥� ��㣮� ������ ����� �� 㤠�����
			mov cl,byte[ecx+4]
		@@:
		mov byte[buffer+list_offs_chunk_del],cl
		mov ecx,[level]
		mov byte[buffer+list_offs_chunk_lev],cl
		mov ecx,dword[info_bl]
		mov dword[buffer+list_offs_p_data],ecx
		stdcall hex_in_str, buffer+list_offs_text,dword[esi+1],2
		stdcall hex_in_str, buffer+list_offs_text+2,dword[esi],2 ;��� 3ds �����
		or ecx,ecx
		jnz @f
			mov byte[buffer+list_offs_text+4],0 ;0 - ᨬ��� ���� ��ப�
			jmp .no_capt
		@@:
			mov byte[buffer+list_offs_text+4],' '
			mov esi,ecx
			mov edi,buffer+list_offs_text+5
			mov ecx,size_one_list-(list_offs_text+5)
			cld
			rep movsb
			mov byte[buffer+size_one_list-1],0 ;0 - ᨬ��� ���� ��ப�
		.no_capt:
		stdcall [tl_node_add], tree1, ebx, buffer
		stdcall [tl_cur_next], tree1
	popad
	ret
endp

;input:
; eax - value
; buf - string buffer
; len - buffer len
;output:
align 4
proc convert_int_to_str, buf:dword, len:dword
pushad
	mov edi,[buf]
	mov esi,[len]
	add esi,edi
	dec esi
	call .str
popad
	ret
endp

align 4
.str:
	mov ecx,10
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

align 4
but_save_file:
	pushad
	copy_path open_dialog_name,communication_area_default_path,file_name,0
	mov [OpenDialog_data.type],1
	stdcall [OpenDialog_Set_file_ext],OpenDialog_data,Filter.1 ;.3ds
	stdcall [OpenDialog_Start],OpenDialog_data
	cmp [OpenDialog_data.status],2
	je .end_save_file
	;��� �� 㤠筮� ����⨨ �������

	mov [run_file_70.Function], SSF_CREATE_FILE
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov ebx, dword[open_file_data]
	mov [run_file_70.Buffer], ebx
	mov ebx,dword[ebx+2]
	mov dword[run_file_70.Count], ebx ;ࠧ��� 䠩��
	mov byte[run_file_70+20], 0
	mov dword[run_file_70.FileName], openfile_path
	mcall SF_FILE,run_file_70
	cmp ebx,0xffffffff
	je .end_save_file
		;...ᮮ�饭��...
	.end_save_file:
	popad
	ret

align 4
but_wnd_coords:
	cmp byte[prop_wnd_run],0
	jne @f
		pushad
		mcall SF_CREATE_THREAD,1,prop_start,thread_coords
		popad
	@@:
	ret

;description:
; 㤠����� ��࠭���� ����� �� ����⮣� 䠩��
align 4
but_delete_chunk:
	pushad
	stdcall [tl_node_get_data],tree1
	cmp eax,0
	je .end_f
	cmp byte[eax+list_offs_chunk_del],0 ;�᫨ ���� ���饭 �� 㤠�����
	jne .notify

	;(1) ����஢���� ������ ��� 䠩��
	mov edx,dword[eax+4] ;ࠧ��� �����
	sub [open_file_size],edx ;��������� ࠧ��஢ 䠩��
	mov ecx,[open_file_size]
	mov ebx,dword[eax]
	sub ecx,ebx ;ecx - ࠧ��� ������ ᤢ������� ��� 䠩��
	add ebx,dword[open_file_data] ;����砥� ���祭�� ᤢ��� � �����
	mov edi,ebx
	mov esi,ebx
	add esi,edx
	mov bl,byte[eax+list_offs_chunk_lev] ;��६ �஢��� ⥪�饣� 㧫�
	rep movsb
	mov byte[can_save],1

	;(2) ��������� ࠧ��஢ த�⥫�᪨� ������
	cmp bl,0
	je .end_2
	.cycle_2:
	stdcall [tl_cur_perv], tree1
	stdcall [tl_node_get_data],tree1
	cmp eax,0
	je .end_2
		cmp byte[eax+list_offs_chunk_lev],bl
		jge .cycle_2
		mov bl,byte[eax+list_offs_chunk_lev]
		mov ecx,[eax]
		add ecx,[open_file_data]
		sub dword[ecx+2],edx
		cmp bl,0 ;�᫨ ᠬ� ���孨� 㧥�, � bl=0
		jne .cycle_2
	.end_2:

	;(3) ���������� ᯨ᪠ tree1
	call init_tree
	call draw_window

	jmp .end_f
	.notify:
	notify_window_run txt_not_delete
	.end_f:
	popad
	ret

;����� ��� ������� ������ 䠩���
align 4
OpenDialog_data:
.type			dd 0 ;0 - ������, 1 - ��࠭���, 2 - ����� ��४���
.procinfo		dd procinfo ;+4
.com_area_name	dd communication_area_name ;+8
.com_area		dd 0 ;+12
.opendir_path	dd plugin_path ;+16
.dir_default_path	dd default_dir ;+20
.start_path		dd file_name ;+24 ���� � ������� ������ 䠩���
.draw_window	dd draw_window ;+28
.status 		dd 0 ;+32
.openfile_path	dd openfile_path ;+36 ���� � ���뢠����� 䠩��
.filename_area	dd filename_area ;+40
.filter_area	dd Filter
.x:
.x_size 		dw 420 ;+48 ; Window X size
.x_start		dw 10 ;+50 ; Window X position
.y:
.y_size 		dw 320 ;+52 ; Window y size
.y_start		dw 10 ;+54 ; Window Y position

default_dir db '/sys',0

communication_area_name:
	db 'FFFFFFFF_open_dialog',0
open_dialog_name:
	db 'opendial',0
communication_area_default_path:
	db '/sys/File managers/',0

Filter:
dd Filter.end - Filter.1
.1:
db '3DS',0
db 'STL',0
.3:
db 'PNG',0
.end:
db 0


align 4
system_dir_0 db '/sys/lib/'
lib_name_0 db 'proc_lib.obj',0
system_dir_1 db '/sys/lib/'
lib_name_1 db 'libimg.obj',0
system_dir_2 db '/sys/lib/'
lib_name_2 db 'box_lib.obj',0
system_dir_3 db '/sys/lib/'
lib_name_3 db 'buf2d.obj',0
system_dir_4 db '/sys/lib/'
lib_name_4 db 'kmenu.obj',0
system_dir_5 db '/sys/lib/'
lib_name_5 db 'tinygl.obj',0
system_dir_6 db '/sys/lib/'
lib_name_6 db 'libini.obj',0

align 4
l_libs_start:
	lib_0 l_libs lib_name_0, file_name, system_dir_0, import_proclib
	lib_1 l_libs lib_name_1, file_name, system_dir_1, import_libimg
	lib_2 l_libs lib_name_2, file_name, system_dir_2, import_box_lib
	lib_3 l_libs lib_name_3, file_name, system_dir_3, import_buf2d
	lib_4 l_libs lib_name_4, file_name, system_dir_4, import_libkmenu
	lib_5 l_libs lib_name_5, file_name, system_dir_5, import_tinygl
	lib_6 l_libs lib_name_6, file_name, system_dir_6, import_libini
l_libs_end:

align 4
import_libimg:
	dd alib_init1
	img_is_img  dd aimg_is_img
	img_info    dd aimg_info
	img_from_file dd aimg_from_file
	img_to_file dd aimg_to_file
	img_from_rgb dd aimg_from_rgb
	img_to_rgb  dd aimg_to_rgb
	img_to_rgb2 dd aimg_to_rgb2
	img_decode  dd aimg_decode
	img_encode  dd aimg_encode
	img_create  dd aimg_create
	img_destroy dd aimg_destroy
	img_destroy_layer dd aimg_destroy_layer
	img_count   dd aimg_count
	img_lock_bits dd aimg_lock_bits
	img_unlock_bits dd aimg_unlock_bits
	img_flip    dd aimg_flip
	img_flip_layer dd aimg_flip_layer
	img_rotate  dd aimg_rotate
	img_rotate_layer dd aimg_rotate_layer
	img_draw    dd aimg_draw

	dd 0,0
	alib_init1   db 'lib_init',0
	aimg_is_img  db 'img_is_img',0 ;��।���� �� �����, ����� �� ������⥪� ᤥ���� �� ��� ����ࠦ����
	aimg_info    db 'img_info',0
	aimg_from_file db 'img_from_file',0
	aimg_to_file db 'img_to_file',0
	aimg_from_rgb db 'img_from_rgb',0
	aimg_to_rgb  db 'img_to_rgb',0 ;�८�ࠧ������ ����ࠦ���� � ����� RGB
	aimg_to_rgb2 db 'img_to_rgb2',0
	aimg_decode  db 'img_decode',0 ;��⮬���᪨ ��।���� �ଠ� ����᪨� ������
	aimg_encode  db 'img_encode',0
	aimg_create  db 'img_create',0
	aimg_destroy db 'img_destroy',0
	aimg_destroy_layer db 'img_destroy_layer',0
	aimg_count   db 'img_count',0
	aimg_lock_bits db 'img_lock_bits',0
	aimg_unlock_bits db 'img_unlock_bits',0
	aimg_flip    db 'img_flip',0
	aimg_flip_layer db 'img_flip_layer',0
	aimg_rotate  db 'img_rotate',0
	aimg_rotate_layer db 'img_rotate_layer',0
	aimg_draw    db 'img_draw',0

align 4
import_proclib:
	OpenDialog_Init dd aOpenDialog_Init
	OpenDialog_Start dd aOpenDialog_Start
	OpenDialog_Set_file_name dd aOpenDialog_Set_file_name
	OpenDialog_Set_file_ext dd aOpenDialog_Set_file_ext
dd 0,0
	aOpenDialog_Init db 'OpenDialog_init',0
	aOpenDialog_Start db 'OpenDialog_start',0
	aOpenDialog_Set_file_name db 'OpenDialog_set_file_name',0
	aOpenDialog_Set_file_ext db 'OpenDialog_set_file_ext',0

align 4
import_buf2d:
	dd sz_init0
	buf2d_create dd sz_buf2d_create
	buf2d_create_f_img dd sz_buf2d_create_f_img
	buf2d_clear dd sz_buf2d_clear
	buf2d_draw dd sz_buf2d_draw
	buf2d_delete dd sz_buf2d_delete
	buf2d_resize dd sz_buf2d_resize
	buf2d_line dd sz_buf2d_line
	buf2d_rect_by_size dd sz_buf2d_rect_by_size
	buf2d_filled_rect_by_size dd sz_buf2d_filled_rect_by_size
	buf2d_circle dd sz_buf2d_circle
	buf2d_img_hdiv2 dd sz_buf2d_img_hdiv2
	buf2d_img_wdiv2 dd sz_buf2d_img_wdiv2
	buf2d_conv_24_to_8 dd sz_buf2d_conv_24_to_8
	buf2d_conv_24_to_32 dd sz_buf2d_conv_24_to_32
	buf2d_bit_blt dd sz_buf2d_bit_blt
	buf2d_bit_blt_transp dd sz_buf2d_bit_blt_transp
	buf2d_bit_blt_alpha dd sz_buf2d_bit_blt_alpha
	buf2d_convert_text_matrix dd sz_buf2d_convert_text_matrix
	buf2d_draw_text dd sz_buf2d_draw_text
	buf2d_crop_color dd sz_buf2d_crop_color
	buf2d_offset_h dd sz_buf2d_offset_h
	buf2d_set_pixel dd sz_buf2d_set_pixel
	dd 0,0
	sz_init0 db 'lib_init',0
	sz_buf2d_create db 'buf2d_create',0
	sz_buf2d_create_f_img db 'buf2d_create_f_img',0
	sz_buf2d_clear db 'buf2d_clear',0
	sz_buf2d_draw db 'buf2d_draw',0
	sz_buf2d_delete db 'buf2d_delete',0
	sz_buf2d_resize db 'buf2d_resize',0
	sz_buf2d_line db 'buf2d_line',0
	sz_buf2d_rect_by_size db 'buf2d_rect_by_size',0
	sz_buf2d_filled_rect_by_size db 'buf2d_filled_rect_by_size',0
	sz_buf2d_circle db 'buf2d_circle',0
	sz_buf2d_img_hdiv2 db 'buf2d_img_hdiv2',0
	sz_buf2d_img_wdiv2 db 'buf2d_img_wdiv2',0
	sz_buf2d_conv_24_to_8 db 'buf2d_conv_24_to_8',0
	sz_buf2d_conv_24_to_32 db 'buf2d_conv_24_to_32',0
	sz_buf2d_bit_blt db 'buf2d_bit_blt',0
	sz_buf2d_bit_blt_transp db 'buf2d_bit_blt_transp',0
	sz_buf2d_bit_blt_alpha db 'buf2d_bit_blt_alpha',0
	sz_buf2d_convert_text_matrix db 'buf2d_convert_text_matrix',0
	sz_buf2d_draw_text db 'buf2d_draw_text',0
	sz_buf2d_crop_color db 'buf2d_crop_color',0
	sz_buf2d_offset_h db 'buf2d_offset_h',0
	sz_buf2d_set_pixel db 'buf2d_set_pixel',0

align 4
import_box_lib:
	dd sz_init1
	edit_box_draw dd sz_edit_box_draw
	edit_box_key dd sz_edit_box_key
	edit_box_mouse dd sz_edit_box_mouse
	edit_box_set_text dd sz_edit_box_set_text
	scrollbar_ver_draw dd sz_scrollbar_ver_draw
	scrollbar_hor_draw dd sz_scrollbar_hor_draw

	tl_data_init dd sz_tl_data_init
	tl_data_clear dd sz_tl_data_clear
	tl_info_clear dd sz_tl_info_clear
	tl_key dd sz_tl_key
	tl_mouse dd sz_tl_mouse
	tl_draw dd sz_tl_draw
	tl_info_undo dd sz_tl_info_undo
	tl_info_redo dd sz_tl_info_redo
	tl_node_add dd sz_tl_node_add
	tl_node_set_data dd sz_tl_node_set_data
	tl_node_get_data dd sz_tl_node_get_data
	tl_node_delete dd sz_tl_node_delete
	tl_node_move_up dd sz_tl_node_move_up
	tl_node_move_down dd sz_tl_node_move_down
	tl_cur_beg dd sz_tl_cur_beg
	tl_cur_next dd sz_tl_cur_next
	tl_cur_perv dd sz_tl_cur_perv
	tl_node_close_open dd sz_tl_node_close_open
	tl_node_lev_inc dd sz_tl_node_lev_inc
	tl_node_lev_dec dd sz_tl_node_lev_dec
	tl_node_poi_get_info dd sz_tl_node_poi_get_info
	tl_node_poi_get_next_info dd sz_tl_node_poi_get_next_info
	tl_node_poi_get_data dd sz_tl_node_poi_get_data

	dd 0,0
	sz_init1 db 'lib_init',0
	sz_edit_box_draw db 'edit_box_draw',0
	sz_edit_box_key db 'edit_box_key',0
	sz_edit_box_mouse db 'edit_box_mouse',0
	sz_edit_box_set_text db 'edit_box_set_text',0
	sz_scrollbar_ver_draw db 'scrollbar_v_draw',0
	sz_scrollbar_hor_draw db 'scrollbar_h_draw',0

	sz_tl_data_init db 'tl_data_init',0
	sz_tl_data_clear db 'tl_data_clear',0
	sz_tl_info_clear db 'tl_info_clear',0
	sz_tl_key db 'tl_key',0
	sz_tl_mouse db 'tl_mouse',0
	sz_tl_draw db 'tl_draw',0
	sz_tl_info_undo db 'tl_info_undo',0
	sz_tl_info_redo db 'tl_info_redo',0
	sz_tl_node_add db 'tl_node_add',0
	sz_tl_node_set_data db 'tl_node_set_data',0
	sz_tl_node_get_data db 'tl_node_get_data',0
	sz_tl_node_delete db 'tl_node_delete',0
	sz_tl_node_move_up db 'tl_node_move_up',0
	sz_tl_node_move_down db 'tl_node_move_down',0
	sz_tl_cur_beg db 'tl_cur_beg',0
	sz_tl_cur_next db 'tl_cur_next',0
	sz_tl_cur_perv db 'tl_cur_perv',0
	sz_tl_node_close_open db 'tl_node_close_open',0
	sz_tl_node_lev_inc db 'tl_node_lev_inc',0
	sz_tl_node_lev_dec db 'tl_node_lev_dec',0
	sz_tl_node_poi_get_info db 'tl_node_poi_get_info',0
	sz_tl_node_poi_get_next_info db 'tl_node_poi_get_next_info',0
	sz_tl_node_poi_get_data db 'tl_node_poi_get_data',0

align 4
import_libkmenu:
	kmenu_init		       dd akmenu_init
	kmainmenu_draw		       dd akmainmenu_draw
	kmainmenu_dispatch_cursorevent dd akmainmenu_dispatch_cursorevent
	ksubmenu_new		       dd aksubmenu_new
	ksubmenu_delete 	       dd aksubmenu_delete
	ksubmenu_draw		       dd aksubmenu_draw
	ksubmenu_add		       dd aksubmenu_add
	kmenuitem_new		       dd akmenuitem_new
	kmenuitem_delete	       dd akmenuitem_delete
	kmenuitem_draw		       dd akmenuitem_draw
dd 0,0
	akmenu_init			db 'kmenu_init',0
	akmainmenu_draw 		db 'kmainmenu_draw',0
	akmainmenu_dispatch_cursorevent db 'kmainmenu_dispatch_cursorevent',0
	aksubmenu_new			db 'ksubmenu_new',0
	aksubmenu_delete		db 'ksubmenu_delete',0
	aksubmenu_draw			db 'ksubmenu_draw',0
	aksubmenu_add			db 'ksubmenu_add',0
	akmenuitem_new			db 'kmenuitem_new',0
	akmenuitem_delete		db 'kmenuitem_delete',0
	akmenuitem_draw 		db 'kmenuitem_draw',0

align 4
import_tinygl:
macro E_LIB n
{
if defined sz_#n
	n dd sz_#n
end if
}
include '../../develop/libraries/TinyGL/asm_fork/export.inc'
	dd 0,0
macro E_LIB n
{
if used n
	sz_#n db `n,0
end if
}
include '../../develop/libraries/TinyGL/asm_fork/export.inc'

align 4
import_libini:
	dd alib_init0
	ini_get_str   dd aini_get_str
	ini_get_int   dd aini_get_int
	ini_get_color dd aini_get_color
dd 0,0
	alib_init0     db 'lib_init',0
	aini_get_str   db 'ini_get_str',0
	aini_get_int   db 'ini_get_int',0
	aini_get_color db 'ini_get_color',0

align 4
mouse_dd dd 0
last_time dd 0

align 4
buf_0: dd 0 ;㪠��⥫� �� ���� ����ࠦ����
.l: dw 205 ;+4 left
.t: dw 35 ;+6 top
.w: dd 340 ;+8 w
.h: dd main_wnd_height-65 ;+12 h
.color: dd 0xffffd0 ;+16 color
	db 24 ;+20 bit in pixel

align 4
buf_ogl:
	dd 0 ;㪠��⥫� �� ���� ����ࠦ����
	dw 3d_wnd_l,3d_wnd_t ;+4 left,top
.w: dd 3d_wnd_w
.h: dd 3d_wnd_h
	dd 0,24 ;+16 color,bit in pixel

align 4
buf_1:
	dd 0 ;㪠��⥫� �� ���� ����ࠦ����
	dw 0,0 ;+4 left,top
	dd 128,144 ;+8 w,h
	dd 0,24 ;+16 color,bit in pixel

align 4
el_focus dd tree1
tree1 tree_list size_one_list,300+2, tl_key_no_edit+tl_draw_par_line,\
	16,16, 0xffffff,0xb0d0ff,0x10400040, 5,35,195-16,250, 16,list_offs_text,0,\
	el_focus,w_scr_t1,0

align 4
w_scr_t1 scrollbar 16,0, 3,0, 15, 100, 0,0, 0,0,0, 1

align 4
qObj dd 0

light_position dd 0.0, 0.0, -2.0, 1.0 ; ��ᯮ������� ���筨�� [0][1][2]
	;[3] = (0.0 - ��᪮��筮 㤠����� ���筨�, 1.0 - ���筨� ᢥ� �� ��।������� ����ﭨ�)
light_dir dd 0.0,0.0,0.0 ;���ࠢ����� �����

mat_specular dd 0.3, 0.3, 0.3, 1.0 ; ���� �����
mat_shininess dd 3.0 ; ������ ����� (���⭠� �ய����)
white_light dd 0.8, 0.8, 0.8, 1.0 ; ���� � ��⥭ᨢ����� �ᢥ饭��, ������㥬��� ���筨���
lmodel_ambient dd 0.3, 0.3, 0.3, 1.0 ; ��ࠬ���� 䮭����� �ᢥ饭��

if lang eq ru_RU
capt db 'info 3ds ����� 04.05.25',0 ;������� ����
else ; Default to en_US
capt db 'info 3ds version 04.05.25',0 ;window caption
end if

align 16
i_end:
	ctx1 TinyGLContext
	procinfo process_information
	run_file_70 FileInfoBlock
	sc system_colors
	angle_x rd 1 ;㣫� ������ �業�
	angle_y rd 1
	angle_z rd 1
	color_ox rd 1
	color_oy rd 1
	color_oz rd 1
	color_bk rd 3
	color_vert rd 1
	color_face rd 1
	color_select rd 1
	obj_poi_sel_c rd 1
	o3d obj_3d
	rb 2048
align 16
thread_coords:
	rb 2048
align 16
stacktop:
	sys_path rb 2048
	file_name rb 4096
	plugin_path rb 4096
	openfile_path rb 4096
	filename_area rb 256
mem:
