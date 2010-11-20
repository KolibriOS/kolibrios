use32
	org 0x0
	db 'MENUET01'
	dd 0x1
	dd start
	dd i_end
	dd mem
	dd stacktop
	dd buf_cmd_lin
	dd sys_path

include '../../macros.inc'
include '../../proc32.inc'
include '../../develop/libraries/box_lib/load_lib.mac'
include '../../develop/libraries/box_lib/trunk/box_lib.mac'
include 'mem.inc'
include 'dll.inc'
include 'strlen.inc'

@use_library_mem mem.Alloc,mem.Free,mem.ReAlloc, dll.Load

hed db 'kol_f_edit 20.11.10',0

sizeof.TreeList equ 20 ;need for element 'tree_list'

BUF_STRUCT_SIZE equ 21
buf2d_data equ dword[edi] ;данные буфера изображения
buf2d_w equ dword[edi+8] ;ширина буфера
buf2d_h equ dword[edi+12] ;высота буфера
buf2d_l equ word[edi+4] ;отступ слева
buf2d_t equ word[edi+6] ;отступ сверху
buf2d_size_lt equ dword[edi+4] ;отступ слева и справа для буфера
buf2d_color equ dword[edi+16] ;цвет фона буфера
buf2d_bits equ byte[edi+20] ;количество бит в 1-й точке изображения

MAX_LEN_OBJ_TXT equ 200
MAX_CED_OBJECTS equ 200
MAX_OPT_FIELDS equ 11
MAX_OBJ_TYPES equ 18
WND_CAPT_COLOR equ 0xb0d0ff
BUF_SIZE equ 1000

C_TD equ 't' ;typedef
C_AC equ 'a' ;автоматические коды (auto code)
C_ST equ 's' ;struct
C_IN equ '#' ;include
C_TT equ 'd' ;блок кода (данных)
C_VR equ '-' ;переменная
C_CO equ ';' ;комментарий
C_WI equ 'w' ;окно
C_CB equ 'c' ;CheckBox
C_OP equ 'o' ;OptionBox
C_ED equ 'e' ;EditBox
C_BU equ 'b' ;Button
C_DT equ 'x' ;рисовать текст
C_RE equ 'r' ;рисовать прямоугольник
C_KE equ 'k' ;клавиатура
C_LIB equ 'l'
C_IFN equ 'f'

;modif
CPP_MOD_ACM equ 2 ;события мыши
BIT_MOD_ACM equ 1 ;события мыши
CPP_MOD_ACD equ 8 ;данные объектов
BIT_MOD_ACD equ 3 ;данные объектов
CPP_MOD_ABU equ 16 ;связывание кнопок
BIT_MOD_ABU equ 4 ;связывание кнопок

macro load_image_file path,buf,size ;макрос для загрузки изображений
{
	copy_path path,sys_path,fp_icon,0x0 ;формируем полный путь к файлу изображения, подразумеваем что он в одной папке с программой

	stdcall mem.Alloc, dword size ;выделяем память для изображения
	mov [buf],eax

	mov eax,70 ;70-я функция работа с файлами
	mov [run_file_70.Function], 0
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov [run_file_70.Count], dword size
	m2m [run_file_70.Buffer], [buf]
	mov byte[run_file_70+20], 0
	mov [run_file_70.FileName], fp_icon
	mov ebx,run_file_70
	int 0x40 ;загружаем файл изображения
	cmp ebx,0xffffffff
	je @f
		;определяем вид изображения и переводим его во временный буфер image_data
		stdcall dword[img_decode], dword[buf],ebx,0
		mov dword[image_data],eax
		;преобразуем изображение к формату rgb
		stdcall dword[img_to_rgb2], dword[image_data],dword[buf]
		;удаляем временный буфер image_data
		stdcall dword[img_destroy], dword[image_data]
	@@:
}

struct FileInfoBlock
	Function dd ?
	Position dd ?
	Flags	 dd ?
	Count	 dd ?
	Buffer	 dd ?
	rezerv	 db ?
	FileName dd ?
ends

struct object
	c db ? ;0
	txt rb MAX_LEN_OBJ_TXT ;1
	lvl db 0 ;1+mt
	clo db 0 ;2+mt
	rb 8 ;3+mt не используется, отсталось от Win версии
	typid dd ? ;3+8+mt
	modif dd ? ;7+8+mt
ends

struct ObjOpt
	c db ?
	bl_type db ?
	graph db ?
	info rb 30
	caption rb 200
	Col rw MAX_OPT_FIELDS
	img rw MAX_OPT_FIELDS ;индексы картинок (в файле 'icon.bmp')
ends

SKIN_H equ 22
SKIN_W1 equ 5
SKIN_W2 equ 7
SKIN_W3 equ 23
fn_skin_1 db 'left.bmp',0
fn_skin_2 db 'base.bmp',0
fn_skin_3 db 'oper.bmp',0
IMAGE_FILE_SKIN1_SIZE equ 3*(SKIN_W1+3)*SKIN_H+54
IMAGE_FILE_SKIN2_SIZE equ 3*(SKIN_W2+3)*SKIN_H+54
IMAGE_FILE_SKIN3_SIZE equ 3*(SKIN_W3+3)*SKIN_H+54

fn_font_s1 db 'font6x9.bmp',0
IMAGE_FILE_FONT1_SIZE equ 96*144*3 ;размер файла с 1-м системным шрифтом

fn_icon db 'icon.bmp',0
count_main_icons equ 33 ;число иконок в файле icon.bmp
bmp_icon rb 0x300*count_main_icons

fn_icon_tl_sys db 'tl_sys_16.png',0
TREE_ICON_SYS16_BMP_SIZE equ 256*3*11+54 ;размер bmp файла с системными иконками
icon_tl_sys dd 0 ;указатеель на память для хранения системных иконок
icon_font_s1 dd 0 ;указатель на временную память для загрузки шрифта

fn_syntax db 'asm.syn',0 ;имя загружаемого файла синтаксиса

include 'ced_wnd_m.inc'
include 'ced_constr.inc' ;файл с функциями окна конструктора
include 'ced_code_g.inc' ;файл с функциями генерирования кодов

align 4
start:
	load_libraries l_libs_start,load_lib_end

	;проверка на сколько удачно загузилась наша либа
	mov	ebp,lib0
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
		mcall -1 ;exit not correct
	@@:
	mov	ebp,lib1
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
		mcall -1 ;exit not correct
	@@:
	mov	ebp,lib2
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
		mcall -1 ;exit not correct
	@@:
	mov	ebp,lib3
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
		mcall -1 ;exit not correct
	@@:

	mov eax,[wndObjI.top]
	add eax,[recMain.top]
	inc eax
	mov edi,edit2
	@@:
		mov ed_top,eax
		add edi,ed_struc_size
		add eax,[recMain.height]
		cmp edi,prop_wnd_edits_end
		jl @b

	stdcall [buf2d_create], buf_fon

	mcall 48,3,sc,sizeof.system_colors
	mcall 40,0x27

	stdcall dword[tl_data_init], tree1
	stdcall dword[tl_data_init], tree2

	copy_path fn_icon,sys_path,fp_icon,0x0 ;формируем полный путь к файлу изображения, подразумеваем что он в одной папке с программой
	mov eax,70 ;load icon file
	mov [run_file_70.Function], 0
	mov [run_file_70.Position], 54
	mov [run_file_70.Flags], 0
	mov [run_file_70.Count], 0x300*count_main_icons
	mov [run_file_70.Buffer], bmp_icon
	mov [run_file_70.rezerv], 0
	mov [run_file_70.FileName], fp_icon
	mov ebx,run_file_70
	int 0x40

	cmp ebx,-1
	mov [err_ini0],1
	je @f ;if open file
		mov [err_ini0],0
		mov dword[tree1.data_img],bmp_icon
		mov dword[tree2.data_img],bmp_icon
	@@:

	;системные иконки 16*16 для tree_list
	load_image_file fn_icon_tl_sys, icon_tl_sys,TREE_ICON_SYS16_BMP_SIZE
	;если изображение не открылось, то в icon_tl_sys будут
	;не инициализированные данные, но ошибки не будет, т. к. буфер нужного размера
	m2m dword[tree1.data_img_sys],dword[icon_tl_sys]
	m2m dword[tree2.data_img_sys],dword[icon_tl_sys]

	;1-й файл скина
	load_image_file fn_skin_1, icon_font_s1,IMAGE_FILE_SKIN1_SIZE
	stdcall [buf2d_create_f_img], buf_skin1,[icon_font_s1] ;создаем буфер
	stdcall mem.Free,[icon_font_s1] ;освобождаем память
	;2-й файл скина
	load_image_file fn_skin_2, icon_font_s1,IMAGE_FILE_SKIN2_SIZE
	stdcall [buf2d_create_f_img], buf_skin2,[icon_font_s1] ;создаем буфер
	stdcall mem.Free,[icon_font_s1] ;освобождаем память
	;3-й файл скина
	load_image_file fn_skin_3, icon_font_s1,IMAGE_FILE_SKIN3_SIZE
	stdcall [buf2d_create_f_img], buf_skin3,[icon_font_s1] ;создаем буфер
	stdcall mem.Free,[icon_font_s1] ;освобождаем память

	;символы 1-го системного шрифта
	load_image_file fn_font_s1, icon_font_s1,IMAGE_FILE_FONT1_SIZE
	stdcall [buf2d_create_f_img], buf_font,[icon_font_s1] ;создаем буфер
	stdcall mem.Free,[icon_font_s1] ;освобождаем память
	stdcall [buf2d_conv_24_to_8], buf_font,1 ;делаем буфер прозрачности 8 бит
	stdcall [buf2d_convert_text_matrix], buf_font


	copy_path fn_obj_opt,sys_path,fp_obj_opt,0x0
	;load options file
	mov eax,70
	mov [run_file_70.Position], 0
	mov [run_file_70.Count], sizeof.ObjOpt*MAX_OBJ_TYPES
	mov [run_file_70.Buffer], obj_opt
	mov [run_file_70.FileName], fp_obj_opt
	mov ebx,run_file_70
	int 0x40

	cmp ebx,-1
	mov [err_ini1],1
	je .open_end ;jmp if not open file
		mov [err_ini1],0

		mov eax,obj_opt ;добавляем объекты
		@@:
			mov bl,byte[eax]
			cmp bl,0
			je @f
			;xor ecx,ecx ;в ecx будет индекс иконки
			mov cx,word[eax+sizeof.ObjOpt-2*MAX_OPT_FIELDS]
			cmp cx,0
			jge .zero
				xor cx,cx ;что-бы не глючило с отрицательным индексом
			.zero:
			shl ecx,16
			stdcall dword[tl_node_add], eax, ecx, tree1 ;добавляем название объекта
			stdcall dword[tl_cur_next], tree1 ;переносим курсор вниз, что-бы не поменялся порядок
			add eax,sizeof.ObjOpt ;переход на следующий объект
			jmp @b
		@@:
		stdcall dword[tl_cur_beg], tree1 ;переносим курсор вверх

	.open_end:

	stdcall [ted_init], tedit0
	copy_path fn_syntax,sys_path,fp_icon,0x0
	stdcall [ted_init_syntax_file], tedit0,run_file_70,fp_icon
	;mov edi,tedit0
    ;call [ted_text_colored]

	;get cmd line
	cmp [buf_cmd_lin],0
	je @f ;if file names exist
		mov esi,buf_cmd_lin
		call strlen ;eax=strlen
		mov edi,[edit1.text]
		mov [edit1.size],eax
		mov ecx,eax
		rep movsb
		call but_open_proj
	@@:



align 4
red_win:
	call draw_window

align 4
still:
	mcall 10

	cmp al,1
	jne @f
		call draw_window
	@@:
	cmp al,2
	jz key
	cmp al,3
	jz button
	cmp al,6
	jne @f 
		call mouse
	@@:

	jmp still

align 4
draw_window:
pushad
	mcall 12,1

	xor eax,eax
	mov ebx,20*65536+670
	mov ecx,20*65536+370
	mov edx,[sc.work]
	or  edx,0x33000000
	mov edi,hed
	int 0x40

	mov eax,8 ;button 'Open Project'
	mov esi,0x80ff
	mov ebx,230*65536+18
	mov ecx,5*65536+18
	mov edx,5
	int 0x40
	stdcall draw_icon, 22,231,6 ;22 - open

	;button 'Save Project'
	mov ebx,250*65536+18
	mov ecx,5*65536+18
	mov edx,6
	int 0x40
	stdcall draw_icon, 17,251,6 ;17 - save

	;button 'Show Constructor'
	mov ebx,310*65536+18
	mov ecx,5*65536+18
	mov edx,11
	int 0x40
	stdcall draw_icon, 12,311,6 ;12 - window

	;button 'Show Code'
	mov ebx,330*65536+18
	mov edx,12
	int 0x40
	stdcall draw_icon, 11,331,6 ;11 - text

	;button 'Update Code'
	mov ebx,350*65536+18
	mov edx,13
	int 0x40
	stdcall draw_icon, 32,351,6 ;32 - update

	;button 'Save Code'
	mov ebx,370*65536+18
	mov edx,14
	int 0x40
	stdcall draw_icon, 17,371,6 ;17 - save

	;button ']P'
	mov ebx,390*65536+18
	mov edx,15
	int 0x40
	stdcall draw_icon, 18,391,6 ;18 - знак конца абзаца

	;button 'Show color text'
	mov ebx,410*65536+18
	mov edx,16
	int 0x40
	stdcall draw_icon, 19,411,6

	;button 'Move Up'
	mov ebx,160*65536+18
	mov ecx,30*65536+18
	mov edx,21
	int 0x40
	stdcall draw_icon, 23,161,31 ;23 - move up

	;button 'Move Down'
	mov ebx,180*65536+18
	mov ecx,30*65536+18
	mov edx,22
	int 0x40
	stdcall draw_icon, 24,181,31 ;24 - move down

	;button 'Copy'
	mov ebx,200*65536+18
	mov ecx,30*65536+18
	mov edx,23
	int 0x40
	stdcall draw_icon, 30,201,31 ;30 - copy

	;button 'Paste'
	mov ebx,220*65536+18
	mov ecx,30*65536+18
	mov edx,24
	int 0x40
	stdcall draw_icon, 31,221,31 ;31 - paste

	;button 'Property'
	mov ebx,240*65536+18
	mov ecx,30*65536+18
	mov edx,25
	int 0x40
	stdcall draw_icon, 7,241,31 ;7 - property

; 10 30 50 70 90

	cmp [err_opn],1
	jne @f
		mov eax,4
		mov ebx,10*65536+35
		mov ecx,0x80ff0000
		mov edx,txtErrOpen
		int 0x40
	@@:

	cmp [err_ini0],1
	je err_init_icon
	cmp [err_ini1],1
	je err_init_icon
		call draw_obj_info ;окно редактирования выбранного объекта
	err_init_icon:

	stdcall [edit_box_draw], dword edit1
	stdcall [edit_box_draw], dword edit_sav
	stdcall [tl_draw],dword tree1
	mov dword[w_scr_t1.all_redraw],1
	stdcall [scrollbar_ver_draw],dword w_scr_t1
	stdcall [tl_draw],dword tree2
	mov dword[w_scr_t2.all_redraw],1
	stdcall [scrollbar_ver_draw],dword w_scr_t2

	cmp byte[show_mode],0 ;условие видимости окна конструктора
	jne @f
		stdcall [buf2d_draw], buf_fon
	@@:
	cmp byte[show_mode],1 ;условие видимости текстового окна
	jne @f
		stdcall [ted_draw], tedit0
	@@:
	mcall 12,2
popad
	ret

align 4
mouse:
	stdcall [edit_box_mouse], dword edit1
	stdcall [edit_box_mouse], dword edit2
	stdcall [edit_box_mouse], dword edit3
	stdcall [edit_box_mouse], dword edit4
	stdcall [edit_box_mouse], dword edit5
	stdcall [edit_box_mouse], dword edit6
	stdcall [edit_box_mouse], dword edit7
	stdcall [edit_box_mouse], dword edit8
	stdcall [edit_box_mouse], dword edit9
	stdcall [edit_box_mouse], dword edit_sav
	stdcall [tl_mouse], dword tree1
	stdcall [tl_mouse], dword tree2
	cmp byte[show_mode],1 ;условие видимости текстового окна
	jne @f
		stdcall [ted_mouse], tedit0
	@@:
	ret


align 4
key:
	mcall 2
	stdcall [edit_box_key], dword edit1
	stdcall [edit_box_key], dword edit2
	stdcall [edit_box_key], dword edit3
	stdcall [edit_box_key], dword edit4
	stdcall [edit_box_key], dword edit5
	stdcall [edit_box_key], dword edit6
	stdcall [edit_box_key], dword edit7
	stdcall [edit_box_key], dword edit8
	stdcall [edit_box_key], dword edit9
	stdcall [edit_box_key], dword edit_sav
	stdcall [tl_key], dword tree1
	stdcall [tl_key], dword tree2
 
;  cmp ah,178 ;Up
;  jne @f
;    cmp [cur_y],0
;    je @f
;    dec [cur_y]
;    call get_obj_pos
;    call draw_window
;  @@:

	jmp still

align 4
button:
	mcall 17
	cmp ah,5
	jne @f
		call but_open_proj
	@@:
	cmp ah,6
	jne @f
		call but_save_proj
	@@:
	cmp ah,10
	jne @f
		call but_element_change
	@@:
	cmp ah,11
	jne @f
		call but_show_constructor
	@@:
	cmp ah,12
	jne @f
		call but_show_code
	@@:
	cmp ah,13
	jne @f
		call but_code_gen
	@@:
	cmp ah,14
	jne @f
		call but_save_asm
	@@:
	cmp ah,15
	jne @f
		call but_show_invis
	@@:
	cmp ah,16
	jne @f
		call but_show_syntax
	@@:
	cmp ah,21
	jne @f
		call but_obj_move_up
	@@:
	cmp ah,22
	jne @f
		call but_obj_move_down
	@@:
	cmp ah,23
	jne @f
		call but_obj_copy
	@@:
	cmp ah,24
	jne @f
		call but_obj_paste
	@@:
	cmp ah,25
	jne @f
		call on_file_object_select
	@@:
	cmp ah,1
	jne still
.exit:
	stdcall mem.Free,[icon_tl_sys]
	mov dword[tree1.data_img],0
	mov dword[tree2.data_img],0
	mov dword[tree1.data_img_sys],0
	mov dword[tree2.data_img_sys],0
	stdcall dword[tl_data_clear], tree1
	stdcall dword[tl_data_clear], tree2
	stdcall [buf2d_delete],buf_fon ;удаляем буфер
	stdcall [buf2d_delete],buf_font ;удаляем буфер  
	stdcall [buf2d_delete],buf_skin1
	stdcall [buf2d_delete],buf_skin2
	stdcall [buf2d_delete],buf_skin3
	stdcall [ted_delete], tedit0
	mcall -1

align 4
but_open_proj:
	pushad
	mov eax,70
	mov [run_file_70.Function], 0
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov [run_file_70.Count], sizeof.object*MAX_CED_OBJECTS
	mov [run_file_70.Buffer], ced_info
	mov [run_file_70.rezerv], 0
	push [edit1.text]
	pop [run_file_70.FileName]
	mov ebx,run_file_70
	int 0x40

	cmp ebx,-1
	mov [err_opn],1
	je .open_end ;if open file
		mov ecx,ced_info
		add ecx,ebx
		mov byte [ecx],0
		mov [err_opn],0

		stdcall dword[tl_info_clear], tree2
		mov eax,ced_info ;добавляем объекты
		@@:
			mov bl,byte[eax]
			cmp bl,0
			je @f

			call find_obj_in_opt ;edi = pointer to ObjOpt struct

			mov cx,word[edi+sizeof.ObjOpt-2*MAX_OPT_FIELDS]
			cmp cx,0
			jge .zero
				xor cx,cx ;что-бы не глючило с отрицательным индексом
			.zero:
			shl ecx,16 ;в ecx индекс иконки
			mov cl,byte[eax+1+MAX_LEN_OBJ_TXT] ;уровень объекта
			stdcall dword[tl_node_add], eax, ecx, tree2 ;добавляем объект
			stdcall dword[tl_cur_next], tree2 ;переносим курсор вниз, что-бы не поменялся порядок
			add eax,sizeof.object ;переход на следующий объект
			jmp @b
		@@:
		stdcall dword[tl_cur_beg], tree2 ;переносим курсор вверх

		mov [foc_obj],0
		call draw_constructor
		call code_gen
	.open_end:
	call draw_window ;перерисовка окна идет в любом случае, даже если файл не открылся
	popad
	ret

;сохранение файла проэкта на диск
align 4
but_save_proj:
	pushad

	mov edi,ced_info

	stdcall [tl_node_poi_get_info], 0,tree2
	pop edx
	@@:
		cmp edx,0
		je @f
		stdcall [tl_node_poi_get_data], edx,tree2
		pop esi ;получаем данные узла

		mov bl,byte[edx+2] ;bl - уровень объекта
		mov byte[esi+1+MAX_LEN_OBJ_TXT],bl

		;вычисляем новый индекс для типа объекта
		mov ebx,[esi+3+8+MAX_LEN_OBJ_TXT] ;ebx - тип объекта
		;сохраняем тип объекта
		push ebx
			imul ebx,sizeof.TreeList
			add ebx,[tree2.data_nodes] ;ebx - указатель объект указывающий тип
			stdcall get_obj_npp,ebx
			mov [esi+3+8+MAX_LEN_OBJ_TXT],eax
			mov eax,esi

			;копируем объект в память для сохранения
			xor ecx,ecx
			mov cx,word[tree2.info_size]
			cld
			rep movsb
		;восстанавливаем тип объекта
		pop dword[eax+3+8+MAX_LEN_OBJ_TXT]

		stdcall [tl_node_poi_get_next_info], edx,tree2
		pop edx ;переходим к следущему узлу
		jmp @b
	@@:
	mov byte[edi],0
	inc edi
	mov ecx,edi
	sub ecx,ced_info ;ecx - размер сохраняемого файла       

	mov eax,70
	mov [run_file_70.Function], 2
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov [run_file_70.Count], ecx
	mov [run_file_70.Buffer], ced_info
	mov [run_file_70.rezerv], 0
	push [edit1.text]
	pop [run_file_70.FileName]
	mov ebx,run_file_70
	int 0x40

	popad
	ret

;берет номер по порядку по указателю на структуру объекта
align 4
proc get_obj_npp, p_obj_str:dword
	mov eax,2
	push ebx edx
	mov ebx,[p_obj_str]

	stdcall [tl_node_poi_get_info], 0,tree2
	pop edx
	@@:
		cmp edx,0
		je @f
		cmp edx,ebx
		je @f

		inc eax
		stdcall [tl_node_poi_get_next_info], edx,tree2
		pop edx ;переходим к следущему узлу
		jmp @b
	@@:
	pop edx ebx
	ret
endp

;функция для сохранения созданного asm файла
align 4
but_save_asm:
	;stdcall [ted_but_save_file], tedit0,run_file_70,[edit_sav.text]
	push edi
	mov edi, tedit0

	stdcall [ted_but_save_file],edi,run_file_70,[edit_sav.text]
	cmp ted_err_save,0
	jne @f
		stdcall [mb_create],msgbox_1,thread ;message: Файл был сохранен
	@@:
	pop edi
	ret

;функция для показа/скрытия невидимых символов
align 4
but_show_invis:
	push edi
	mov edi,tedit0

	xor ted_mode_invis,1
	cmp byte[show_mode],1 ;условие видимости текстового окна
	jne @f
		stdcall [ted_draw],edi
	@@:
	pop edi
	ret

;
align 4
but_show_syntax:
	push edi
	mov edi,tedit0

	xor ted_mode_color,1
	cmp byte[show_mode],1 ;условие видимости текстового окна
	jne @f
		stdcall [ted_draw],edi
	@@:
	pop edi
	ret

align 4
ted_save_err_msg:
	mov byte[msgbox_0.err],al
	stdcall [mb_create],msgbox_0,thread ;message: Can-t save text file!
	ret

;функция вызываемая при нажатии Enter в окне tree2
;обновляет текстовые поля значениями параметров взятых из объектов
;функция обратная к данной but_element_change
align 4
on_file_object_select:
	stdcall [tl_node_get_data], tree2
	pop dword[foc_obj]
	cmp dword[foc_obj],0
	je @f
		stdcall set_obj_win_param, 0,edit2
		stdcall set_obj_win_param, 1,edit3
		stdcall set_obj_win_param, 2,edit4
		stdcall set_obj_win_param, 3,edit5
		stdcall set_obj_win_param, 4,edit6
		stdcall set_obj_win_param, 5,edit7
		stdcall set_obj_win_param, 6,edit8
		stdcall set_obj_win_param, 7,edit9
	@@:
	call draw_window
	ret

;функция вызываемая при нажатии Enter в окне tree1
;добавляет новый объект в окно tree2
align 4
on_add_object:
push eax ebx ecx
	stdcall [tl_node_get_data], tree1
	pop eax
	cmp eax,0
	je @f
		xor ecx,ecx
		mov cx,word[eax+obj_opt.img-obj_opt] ;cx - индекс главной иконки добавляемого объекта
		shl ecx,16
		stdcall dword[tl_node_add], eax, ecx, tree2 ;добавляем объект
	@@:
pop ecx ebx eax
	call draw_window
	ret

align 4
proc set_obj_win_param, col:dword, edit:dword
	pushad
		stdcall get_obj_text_col, [foc_obj], [col]
		stdcall get_obj_text_len_col, [foc_obj], [col] ;eax - длинна поля
		mov edi,[edit]
		cmp eax,1
		jl @f
			dec eax
			mov ed_max,eax ;ed_max = edi+.max
			stdcall [edit_box_set_text], edi,edx ;обновляем editbox
			jmp .end_f
		@@:
			mov ed_max,dword MAX_LEN_OBJ_TXT
			stdcall [edit_box_set_text], edi,txt_null
		.end_f:
	popad
	ret
endp

align 4
but_ctrl_o:
	ret
align 4
but_ctrl_n:
	ret
align 4
but_ctrl_s:
	ret

align 4
buf_fon: ;фоновый буфер
	dd 0 ;указатель на буфер изображения
	dw 310 ;+4 left
	dw 50 ;+6 top
	dd 340 ;+8 w
	dd 280 ;+12 h
	dd 0xffffff ;+16 color
	db 24 ;+20 bit in pixel

align 4
buf_font: ;буфер матрицы со шрифтом
	dd 0 ;указатель на буфер изображения
	dw 25 ;+4 left
	dw 25 ;+6 top
	dd 96 ;+8 w
	dd 144 ;+12 h
	dd 0 ;+16 color
	db 24 ;+20 bit in pixel

align 4
buf_skin1:
	dd 0 ;указатель на буфер изображения
	dw 0 ;+4 left
	dw 0 ;+6 top
	dd SKIN_W1 ;+8 w
	dd SKIN_H ;+12 h
	dd 0 ;+16 color
	db 24 ;+20 bit in pixel
align 4
buf_skin2:
	dd 0 ;указатель на буфер изображения
	dw 0 ;+4 left
	dw 0 ;+6 top
	dd SKIN_W2 ;+8 w
	dd SKIN_H ;+12 h
	dd 0 ;+16 color
	db 24 ;+20 bit in pixel
align 4
buf_skin3:
	dd 0 ;указатель на буфер изображения
	dw 0 ;+4 left
	dw 0 ;+6 top
	dd SKIN_W3 ;+8 w
	dd SKIN_H ;+12 h
	dd 0 ;+16 color
	db 24 ;+20 bit in pixel

show_mode db 0 ;режим для показа определенного окна
txtErrOpen db 'Не найден файл, проверьте правильность имени',0
txtErrIni1 db 'Не открылся файл с опциями',0
err_opn db 0
err_ini0 db 0 ;???
err_ini1 db 0

edit1 edit_box 210, 10, 5, 0xffffff, 0xff80, 0xff, 0xff0000, 0x4080, 300, ed_text1, mouse_dd, 0, 7, 7

edit2 edit_box 115, 32, 20, 0xffffff, 0x80ff, 0xff, 0x808080, 0, MAX_LEN_OBJ_TXT, ed_text2, mouse_dd, 0
edit3 edit_box 115, 32, 20, 0xffffff, 0x80ff, 0xff, 0x808080, 0, MAX_LEN_OBJ_TXT, ed_text3, mouse_dd, 0
edit4 edit_box 115, 32, 20, 0xffffff, 0x80ff, 0xff, 0x808080, 0, MAX_LEN_OBJ_TXT, ed_text4, mouse_dd, 0
edit5 edit_box 115, 32, 20, 0xffffff, 0x80ff, 0xff, 0x808080, 0, MAX_LEN_OBJ_TXT, ed_text5, mouse_dd, 0
edit6 edit_box 115, 32, 20, 0xffffff, 0x80ff, 0xff, 0x808080, 0, MAX_LEN_OBJ_TXT, ed_text6, mouse_dd, 0
edit7 edit_box 115, 32, 20, 0xffffff, 0x80ff, 0xff, 0x808080, 0, MAX_LEN_OBJ_TXT, ed_text7, mouse_dd, 0
edit8 edit_box 115, 32, 20, 0xffffff, 0x80ff, 0xff, 0x808080, 0, MAX_LEN_OBJ_TXT, ed_text8, mouse_dd, 0
edit9 edit_box 115, 32, 20, 0xffffff, 0x80ff, 0xff, 0x808080, 0, MAX_LEN_OBJ_TXT, ed_text9, mouse_dd, 0
prop_wnd_edits_end: ;конец текстовых полей, отвечающих за свойства

edit_sav edit_box 210, 310, 30, 0xffffff, 0xff80, 0xff, 0xff0000, 0x4080, 300, ed_text_sav, mouse_dd, 0


ed_text1 db '/hd0/1/',0
	rb 295
ed_text2 rb MAX_LEN_OBJ_TXT+2
ed_text3 rb MAX_LEN_OBJ_TXT+2
ed_text4 rb MAX_LEN_OBJ_TXT+2
ed_text5 rb MAX_LEN_OBJ_TXT+2
ed_text6 rb MAX_LEN_OBJ_TXT+2
ed_text7 rb MAX_LEN_OBJ_TXT+2
ed_text8 rb MAX_LEN_OBJ_TXT+2
ed_text9 rb MAX_LEN_OBJ_TXT+2
ed_text_sav rb 302

txt_null db 'null',0
mouse_dd dd ?

el_focus dd tree1
;дерево со списком возможных типов объектов
tree1 tree_list sizeof.ObjOpt,20+2, tl_key_no_edit+tl_draw_par_line+tl_list_box_mode,\
	16,16, 0xffffff,0xb0d0ff,0xd000ff, 10,50,125,100, 0,3,0, el_focus,\
	w_scr_t1,on_add_object
;дерево с объектами в пользовательском файле
tree2 tree_list sizeof.object,MAX_CED_OBJECTS+2, tl_draw_par_line,\
	16,16, 0xffffff,0xb0d0ff,0xd000ff, 160,50,125,280, 13,1,MAX_LEN_OBJ_TXT, el_focus,\
	w_scr_t2,on_file_object_select

msgbox_0:
  db 1,0
  db 'Warning',0
  db 'Error saving file!',13,\
     'Maybe the file name is not entered correctly.',13,\
     '  (error code ='
  .err: db '?'
  db ')',0
  db 'Close',0
  db 0

msgbox_1:
	db 1,0
	db ':)',0
	db 'File was saved',0
	db 'Ok',0
	db 0

struct TexSelect
	x0 dd ?
	y0 dd ?
	x1 dd ?
	y1 dd ?
ends
;------------------------------------------------------------------------------
align 4
tedit0: ;структура текстового редактора
	.wnd BOX 310,50,325,260 ;+ 0
	.rec BOX 30,13,7,10   ;+16
	.drag_m db 0 ;+32 выделение от мыши
	.drag_k db 0 ;+33 выделение от клавиатуры
	.sel  TexSelect 0,0,0,0 ;+34 структура выделения
	.seln TexSelect ;+50 дополнительная структура выделения
	.tex	  dd 0 ;+66 text memory pointer
	.tex_1	  dd 0 ;+70 text first symbol pointer
	.tex_end  dd 0 ;+74 text end memory pointer
	.cur_x	  dd 0 ;+78 координата x курсора
	.cur_y	  dd 0 ;+82 координата y курсора
	.max_chars dd 25002 ;+86 максимальное число символов в одном документе
	.count_colors_text dd 1 ;+90 колличество цветов текста
	.count_key_words   dd 0 ;+94 колличество ключевых слов
	.color_cursor	   dd 0xff0000 ;+98 цвет курсора
	.color_wnd_capt    dd 0x0080c0 ;+102 цвет полей вокруг окна
	.color_wnd_work    dd	   0x0 ;+106 цвет фона окна
	.color_wnd_bord    dd 0xffffff ;+110 цвет текста на полях
	.color_select	   dd 0x0000ff ;+114 цвет выделения
	.color_cur_text    dd 0xffff00 ;+118 цвет символа под курсором
	.color_wnd_text    dd 0x80ffff ;+122 цвет текста в окне
	.syntax_file	   dd 0 ;+126 указатель на начало файла синтаксиса
	.syntax_file_size  dd 55*1024 ;+130 максимальный размер файла синтаксиса
	.text_colors	   dd 0 ;+134 указатель на массив цветов текста
	.help_text_f1	   dd 0 ;+138 указатель на текст справки (по нажатии F1)
	.help_id	   dd -1 ;+142 идентификатор для справки
	.key_words_data    dd 0 ;+146 указатель на структуры ключевых слов TexColViv
	.tim_ch      dd ? ;+150 количество изменений в файле
	.tim_undo    dd ? ;+154 количество отмененных действий
	.tim_ls      dd ? ;+158 время последнего сохранения
	.tim_co      dd ? ;+162 время последней цветовой разметки
	.el_focus    dd el_focus ;+166 указатель на переменную элемента в фокусе
	.err_save    db 0 ;+170 ошибка сохранения файла
	.panel_id    db 0 ;+171 номер открытой панели
	.key_new     db 0 ;+172 символ, который будет добавлятся с клавиатуры
	.symbol_new_line db 20 ;+173 символ завершения строки
	.scr_w	     dd scrol_w1 ;+174 вертикальный скроллинг
	.scr_h	     dd scrol_h1 ;+178 горизонтальный скроллинг
	.arr_key_pos dd 0 ;+182 указатель на массив позиций ключевых слов
	.buffer      dd text_buffer ;+186 указатель на буфер копирования/вставки
	.buffer_find dd 0 ;+190 указатель на буфер для поиска
	.cur_ins     db 1 ;+194 режим работы курсора (обычный или замена)
	.mode_color  db 1 ;+195 режим выделения слов цветом (0-выкл. 1-вкл.)
	.mode_invis  db 0 ;+196 режим показа непечатаемых символов
	.gp_opt      db 0 ;+197 опции возвращаемые функцией ted_get_pos_by_cursor
	.fun_on_key_ctrl_o dd but_ctrl_o ;+198 указатель на функцию вызываемую при нажатии Ctrl+O (открытие файла)
	.fun_on_key_ctrl_f dd 0 ;+202 ... Ctrl+F (вызова/скрытия панели поиска)
	.fun_on_key_ctrl_n dd but_ctrl_n ;+206 ... Ctrl+N (создание нового документа)
	.fun_on_key_ctrl_s dd 0 ;+210 ... Ctrl+S
	.buffer_size	   dd BUF_SIZE ;+214 размер буфера копирования/вставки
	.fun_find_err	   dd 0 ;+218 указатель на функцию вызываемую если поиск закончился неудачно
	.fun_init_synt_err dd 0 ;+222 указатель на функцию вызываемую при ошибочном открытии файла синтаксиса
	.fun_draw_panel_buttons dd 0 ;+226 указатель на функцию рисования панели с кнопками
	.fun_draw_panel_find	dd 0 ;+230 указатель на функцию рисования панели поиска
	.fun_draw_panel_syntax	dd 0 ;+234 указатель на функцию рисования панели синтаксиса
	.fun_save_err		dd ted_save_err_msg ;+238 указатель на функцию вызываемую если сохранение файла закончилось неудачно
	.increase_size dd 1000 ;+242 число символов на которые будет увечиваться память при нехватке
	.ptr_free_symb dd ? ;+246 указатель на свободную память, в которую можно добавлять символ (используется внутри элемента для ускорения вставки текста)
;------------------------------------------------------------------------------
align 4
scrol_w1:
.x:
.size_x   dw 16 ;+0
.start_x  dw 85 ;+2
.y:
.size_y   dw 100 ; +4
.start_y  dw  15 ; +6
.btn_high dd  15 ; +8
.type	  dd   1 ;+12
.max_area dd 100 ;+16
rb 4+4
.bckg_col dd 0xeeeeee ;+28
.frnt_col dd 0xbbddff ;+32
.line_col dd 0x808080 ;+36
.redraw   dd   0 ;+40
.delta	  dw   0 ;+44
.delta2   dw   0 ;+46
.run_x:
.r_size_x  dw 0 ;+48
.r_start_x dw 0 ;+50
.run_y:
.r_size_y   dw 0 ;+52
.r_start_y  dw 0 ;+54
.m_pos	    dd 0 ;+56
.m_pos_2    dd 0 ;+60
.m_keys     dd 0 ;+64
.run_size   dd 0 ;+68
.position2  dd 0 ;+72
.work_size  dd 0 ;+76
.all_redraw dd 0 ;+80
.ar_offset  dd 1 ;+84
;---------------------------------------------------------------------
align 4
scrol_h1:
.x:
.size_x     dw 85 ;+0
.start_x    dw 30 ;+2
.y:
.size_y     dw 16 ;+4
.start_y    dw 100 ;+6
.btn_high   dd 15 ;+8
.type	    dd 1  ;+12
.max_area   dd 100 ;+16
rb 4+4
.bckg_col   dd 0xeeeeee ;+28
.frnt_col   dd 0xbbddff ;+32
.line_col   dd 0x808080 ;+36
.redraw     dd 0  ;+40
.delta	    dw 0  ;+44
.delta2     dw 0  ;+46
.run_x:
.r_size_x   dw 0  ;+48
.r_start_x  dw 0  ;+50
.run_y:
.r_size_y   dw 0 ;+52
.r_start_y  dw 0 ;+54
.m_pos	    dd 0 ;+56
.m_pos_2    dd 0 ;+60
.m_keys     dd 0 ;+64
.run_size   dd 0 ;+68
.position2  dd 0 ;+72
.work_size  dd 0 ;+76
.all_redraw dd 0 ;+80
.ar_offset  dd 1 ;+84



align 4
w_scr_t1:
.size_x     dw 16 ;+0
rb 2+2+2
.btn_high   dd 15 ;+8
.type	    dd 1  ;+12
.max_area   dd 100  ;+16
rb 4+4
.bckg_col   dd 0xeeeeee ;+28
.frnt_col   dd 0xbbddff ;+32
.line_col   dd 0  ;+36
rb 4+2+2
.run_x:
rb 2+2+2+2+4+4+4+4+4+4
.all_redraw dd 0 ;+80
.ar_offset  dd 1 ;+84

align 4
w_scr_t2:
.size_x     dw 16 ;+0
rb 2+2+2
.btn_high   dd 15 ;+8
.type	    dd 1  ;+12
.max_area   dd 100  ;+16
rb 4+4
.bckg_col   dd 0xeeeeee ;+28
.frnt_col   dd 0xbbddff ;+32
.line_col   dd 0  ;+36
rb 4+2+2
.run_x:
rb 2+2+2+2+4+4+4+4+4+4
.all_redraw dd 0 ;+80
.ar_offset  dd 1 ;+84

data_of_code dd 0
sc system_colors

image_data dd 0 ;память для преобразования картинки функциями libimg

recMain BOX 3,13,16,18
ced_info object 0 ;on start == 0
	rb sizeof.object*(MAX_CED_OBJECTS-1)

wndObjI BOX 10,160,125+16,170
text_buffer db BUF_SIZE dup(0)
fn_obj_opt db 'ob_o.opt',0
obj_opt ObjOpt
	rb sizeof.ObjOpt*(MAX_OBJ_TYPES-1)
	db 0 ;eof options

cur_x dd 0
cur_y dd 0
foc_obj dd 0 ;объект в фокусе
obj_m_win dd 0 ;структура главного окна

;
if 1 ;lang eq ru

	err_message_found_lib0 db 'Не найдена библиотека box_lib.obj',0  ;строка, которая будет в сформированном окне, если библиотека не будет найдена
	err_message_import0 db 'Ошибка при импорте библиотеки box_lib.obj',0
	err_message_found_lib1 db 'Не найдена библиотека proc_lib.obj',0
	err_message_import1 db 'Ошибка при импорте библиотеки proc_lib.obj',0
	err_message_found_lib2 db 'Не удалось найти библиотеку buf2d.obj',0
	err_message_import2 db 'Ошибка при импорте библиотеки buf2d.obj',0
	err_message_found_lib3 db 'Не удалось найти библиотеку libimg.obj',0
	err_message_import3 db 'Ошибка при импорте библиотеки libimg.obj',0
	err_message_found_lib4 db 'Не удалось найти библиотеку msgbox.obj',0
	err_message_import4 db 'Ошибка при импорте библиотеки msgbox.obj',0

	head_f_i:
	head_f_l db 'Системная ошибка',0 ;заголовок окна, при возникновении ошибки
else

	err_message_found_lib0 db 'Sorry I cannot found library box_lib.obj',0
	err_message_import0 db 'Error on load import library box_lib.obj',0
	err_message_found_lib1 db 'Sorry I cannot found library proc_lib.obj',0
	err_message_import1 db 'Error on load import library proc_lib.obj',0
	err_message_found_lib2 db 'Sorry I cannot found library buf2d.obj',0
	err_message_import2 db 'Error on load import library buf2d.obj',0
	err_message_found_lib3 db 'Sorry I cannot found library libimg.obj',0
	err_message_import3 db 'Error on load import library libimg.obj',0
	err_message_found_lib4 db 'Sorry I cannot found library msgbox.obj',0
	err_message_import4 db 'Error on load import library msgbox.obj',0

	head_f_i:
	head_f_l db 'System error',0 ;заголовок окна, при возникновении ошибки
end if

	system_dir0 db '/sys/lib/'
	lib0_name db 'box_lib.obj',0

	system_dir1 db '/sys/lib/'
	lib1_name db 'proc_lib.obj',0

	system_dir2 db '/sys/lib/'
	lib2_name db 'buf2d.obj',0

	system_dir3 db '/sys/lib/'
	lib3_name db 'libimg.obj',0

	system_dir4 db '/sys/lib/'
	lib4_name db 'msgbox.obj',0

align 4
import_buf2d_lib:
	dd sz_lib_init
	buf2d_create dd sz_buf2d_create
	buf2d_create_f_img dd sz_buf2d_create_f_img
	buf2d_clear dd sz_buf2d_clear
	buf2d_draw dd sz_buf2d_draw
	buf2d_delete dd sz_buf2d_delete
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
	buf2d_cruve_bezier dd sz_buf2d_cruve_bezier
	buf2d_convert_text_matrix dd sz_buf2d_convert_text_matrix
	buf2d_draw_text dd sz_buf2d_draw_text
	buf2d_crop_color dd sz_buf2d_crop_color
	buf2d_offset_h dd sz_buf2d_offset_h	
dd 0,0
	sz_lib_init db 'lib_init',0
	sz_buf2d_create db 'buf2d_create',0
	sz_buf2d_create_f_img db 'buf2d_create_f_img',0
	sz_buf2d_clear db 'buf2d_clear',0
	sz_buf2d_draw db 'buf2d_draw',0
	sz_buf2d_delete db 'buf2d_delete',0
	sz_buf2d_line db 'buf2d_line',0
	sz_buf2d_rect_by_size db 'buf2d_rect_by_size',0 ;рисование прямоугольника, 2-я координата задана по размеру
	sz_buf2d_filled_rect_by_size db 'buf2d_filled_rect_by_size',0
	sz_buf2d_circle db 'buf2d_circle',0 ;рисование окружности
	sz_buf2d_img_hdiv2 db 'buf2d_img_hdiv2',0
	sz_buf2d_img_wdiv2 db 'buf2d_img_wdiv2',0
	sz_buf2d_conv_24_to_8 db 'buf2d_conv_24_to_8',0
	sz_buf2d_conv_24_to_32 db 'buf2d_conv_24_to_32',0
	sz_buf2d_bit_blt db 'buf2d_bit_blt',0
	sz_buf2d_bit_blt_transp db 'buf2d_bit_blt_transp',0
	sz_buf2d_bit_blt_alpha db 'buf2d_bit_blt_alpha',0
	sz_buf2d_cruve_bezier db 'buf2d_cruve_bezier',0
	sz_buf2d_convert_text_matrix db 'buf2d_convert_text_matrix',0
	sz_buf2d_draw_text db 'buf2d_draw_text',0
	sz_buf2d_crop_color db 'buf2d_crop_color',0
	sz_buf2d_offset_h db 'buf2d_offset_h',0

align 4
import_box_lib:
	dd alib_init2

	edit_box_draw dd aEdit_box_draw
	edit_box_key dd aEdit_box_key
	edit_box_mouse dd aEdit_box_mouse
	edit_box_set_text dd aEdit_box_set_text

	scrollbar_ver_draw dd aScrollbar_ver_draw
	scrollbar_hor_draw dd aScrollbar_hor_draw

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

	ted_but_save_file dd sz_ted_but_save_file
	ted_but_sumb_upper dd sz_ted_but_sumb_upper
	ted_but_sumb_lover dd sz_ted_but_sumb_lover
	ted_can_save dd sz_ted_can_save
	ted_clear dd sz_ted_clear
	ted_delete dd sz_ted_delete
	ted_draw dd sz_ted_draw
	ted_init dd sz_ted_init
	ted_init_scroll_bars dd sz_ted_init_scroll_bars
	ted_init_syntax_file dd sz_ted_init_syntax_file
	ted_is_select dd sz_ted_is_select
	ted_key dd sz_ted_key
	ted_mouse dd sz_ted_mouse
	ted_open_file dd sz_ted_open_file
	ted_text_add dd sz_ted_text_add
	ted_but_select_word dd sz_ted_but_select_word
	ted_but_cut dd sz_ted_but_cut
	ted_but_copy dd sz_ted_but_copy
	ted_but_paste dd sz_ted_but_paste
	ted_but_undo dd sz_ted_but_undo
	ted_but_redo dd sz_ted_but_redo
	ted_but_reverse dd sz_ted_but_reverse
	ted_but_find_next dd sz_ted_but_find_next
	ted_text_colored dd sz_ted_text_colored
	;version_text_edit dd sz_ted_version

dd 0,0
 
	alib_init2 db 'lib_init',0

	aEdit_box_draw	db 'edit_box',0
	aEdit_box_key	db 'edit_box_key',0
	aEdit_box_mouse db 'edit_box_mouse',0
	aEdit_box_set_text db 'edit_box_set_text',0

	aScrollbar_ver_draw  db 'scrollbar_v_draw',0
	aScrollbar_hor_draw  db 'scrollbar_h_draw',0
  
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

	sz_ted_but_save_file	db 'ted_but_save_file',0
	sz_ted_but_sumb_upper	db 'ted_but_sumb_upper',0
	sz_ted_but_sumb_lover	db 'ted_but_sumb_lover',0
	sz_ted_can_save 		db 'ted_can_save',0
	sz_ted_clear			db 'ted_clear',0
	sz_ted_delete			db 'ted_delete',0
	sz_ted_draw				db 'ted_draw',0
	sz_ted_init				db 'ted_init',0
	sz_ted_init_scroll_bars db 'ted_init_scroll_bars',0
	sz_ted_init_syntax_file db 'ted_init_syntax_file',0
	sz_ted_is_select		db 'ted_is_select',0
	sz_ted_key				db 'ted_key',0
	sz_ted_mouse			db 'ted_mouse',0
	sz_ted_open_file		db 'ted_open_file',0
	sz_ted_text_add 		db 'ted_text_add',0
	sz_ted_but_select_word	db 'ted_but_select_word',0
	sz_ted_but_cut			db 'ted_but_cut',0
	sz_ted_but_copy 		db 'ted_but_copy',0
	sz_ted_but_paste		db 'ted_but_paste',0
	sz_ted_but_undo 		db 'ted_but_undo',0
	sz_ted_but_redo 		db 'ted_but_redo',0
	sz_ted_but_reverse		db 'ted_but_reverse',0
	sz_ted_but_find_next	db 'ted_but_find_next',0
	sz_ted_text_colored		db 'ted_text_colored',0
	;sz_ted_version db 'version_text_edit',0

align 4
import_proc_lib:
	OpenDialog_Init dd aOpenDialog_Init
	OpenDialog_Start dd aOpenDialog_Start
dd 0,0
	aOpenDialog_Init db 'OpenDialog_init',0
	aOpenDialog_Start db 'OpenDialog_start',0

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
	aimg_is_img  db 'img_is_img',0 ;определяет по данным, может ли библиотека сделать из них изображение
	aimg_info    db 'img_info',0
	aimg_from_file db 'img_from_file',0
	aimg_to_file db 'img_to_file',0
	aimg_from_rgb db 'img_from_rgb',0
	aimg_to_rgb  db 'img_to_rgb',0 ;преобразование изображения в данные RGB
	aimg_to_rgb2 db 'img_to_rgb2',0
	aimg_decode  db 'img_decode',0 ;автоматически определяет формат графических данных
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
import_msgbox_lib:
	mb_create dd amb_create
	mb_reinit dd amb_reinit
	mb_setfunctions dd amb_setfunctions
dd 0,0
	amb_create db 'mb_create',0
	amb_reinit db 'mb_reinit',0
	amb_setfunctions db 'mb_setfunctions',0

;library structures
l_libs_start:
	lib0 l_libs lib0_name, sys_path, library_path, system_dir0, err_message_found_lib0, head_f_l, import_box_lib, err_message_import0, head_f_i
	lib1 l_libs lib1_name, sys_path, library_path, system_dir1, err_message_found_lib1, head_f_l, import_proc_lib,err_message_import1, head_f_i
	lib2 l_libs lib2_name, sys_path, library_path, system_dir2, err_message_found_lib2, head_f_l, import_buf2d_lib, err_message_import2, head_f_i
	lib3 l_libs lib3_name, sys_path, library_path, system_dir3, err_message_found_lib3, head_f_l, import_libimg, err_message_import3, head_f_i
	lib4 l_libs lib4_name, sys_path, library_path, system_dir4, err_message_found_lib4, head_f_l, import_msgbox_lib, err_message_import4, head_f_i
load_lib_end:


align 16
procinfo process_information
run_file_70 FileInfoBlock

i_end:
	buf_cmd_lin rb 1024
	fp_icon rb 1024 ;icon file path
	fp_obj_opt rb 1024 ;obj options file patch
	rb 1024
	thread: ;вверху дочерний стек для окна сообщения
	rb 1024
stacktop: ;вверху стек основной программы
	sys_path rb 1024
	library_path rb 1024
mem:
