;Огромная благодарность Maxxxx32, Diamond, Heavyiron
;и другим программистам, а также
;Теплову Алексею (<Lrz> www.lrz.land.ru)


use32
  org 0x0
  db 'MENUET01' ;идентиф. исполняемого файла всегда 8 байт
  dd 0x01
  dd start
  dd i_end ; размер приложения
  dd mem
  dd stacktop
  dd file_name ; command line
  dd sys_path

MAX_COLOR_WORD_LEN equ 40
maxChars equ 100002 ;(...+2)
BUF_SIZE equ 1000 ;bufer for copy|paste
maxColWords equ 10000

include '../../proc32.inc'
include '../../macros.inc'
include '../../develop/libraries/box_lib/load_lib.mac'
include '../../develop/libraries/box_lib/trunk/box_lib.mac'

include 'lang.inc'
include 't_data.inc'
include 'strlen.inc'
include 't_work.inc' ;text work functions
include 't_draw.inc' ;draw main window functions
include 't_button.inc'

@use_library

start:
  mov eax,48
  mov ebx,3
  mov ecx,sc
  mov edx,sizeof.system_colors
  mcall

  m2m [wScr.bckg_col],[sc.work]
  m2m [wScr.frnt_col],[sc.work_button]
  m2m [wScr.line_col],[sc.work_button_text]

  m2m [hScr.bckg_col],[sc.work]
  m2m [hScr.frnt_col],[sc.work_button]
  m2m [hScr.line_col],[sc.work_button_text]


  mcall 68,11
  or eax,eax
  jz button.exit

  mcall 66,1,1 ;scan code
  ;mcall 26,2,1,conv_tabl

  mov ecx,sizeof.symbol*maxChars
  call mem_Alloc
  mov [tex],eax
  mov [tex_1],eax
  add [tex_1],sizeof.symbol
  mov [tex_end],eax
  add [tex_end],sizeof.symbol*maxChars

  mcall 40,0x27

  call Clear

;-------------------------------------------------
  mov ecx,maxColWords*sizeof.TexColViv+40
  ;add ecx,40
  call mem_Alloc
  mov [options_file],eax
  mov [options_file_end],eax
  add [options_file_end],maxColWords*sizeof.TexColViv
  add [options_file_end],40

;-------------------------------------------------
; init bmp file
  mov ecx,1200*18
  call mem_Alloc
  mov [bmp_icon],eax

  mov esi,file_name
  call strlen
  mov ecx,eax
  mov edi,buf_cmd_lin
  rep movsb

  copy_path fn_icon,sys_path,file_name,0x0

  mov eax,70 ;load icon file
  mov [run_file_70.func_n], 0
  mov [run_file_70.param1], 54
  mov [run_file_70.param2], 0
  mov [run_file_70.param3], 1200*18
  m2m [run_file_70.param4], [bmp_icon]
  mov [run_file_70.rezerv], 0
  mov [run_file_70.name], file_name
  mov ebx,run_file_70
  int 0x40

  cmp ebx,-1
  mov [err_ini0],1
  je @f ;if open file
    mov [err_ini0],0
  @@:

load_libraries l_libs_start,load_lib_end

;проверка на сколько удачно загузилась наша либа
	mov	ebp,lib0
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
	mcall	-1	;exit not correct
@@:
	mov	ebp,lib1 ;
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
	mcall	-1	;exit not correct
@@:


;---------------------------------------------------------------------
  push dword tree1
  call dword[tl_data_init]
;---------------------------------------------------------------------
; читаем bmp файл с курсорами и линиями
  copy_path fn_icon_tl_sys,sys_path,file_name,0x0

  mov ecx,3*256*13
  call mem_Alloc
  mov dword[tree1.data_img_sys],eax

  ;mov [run_file_70.func_n], 0
  ;mov [run_file_70.param1], 54
  ;mov [run_file_70.param2], 0
  mov [run_file_70.param3], 3*256*13
  mov [run_file_70.param4], eax
  ;mov [run_file_70.rezerv], 0
  ;mov [run_file_70.name], file_name

  mov eax,70 ;load icon file
  mov ebx,run_file_70
  int 0x40
  cmp ebx,0
  jg @f
    mov dword[tree1.data_img_sys],0
  @@:
;---------------------------------------------------------------------
; читаем bmp файл с иконками узлов
  copy_path fn_icon_tl_nod,sys_path,file_name,0x0

  mov ecx,3*256*2
  call mem_Alloc
  mov dword[tree1.data_img],eax

;  mov [run_file_70.func_n], 0
;  mov [run_file_70.param1], 54
;  mov [run_file_70.param2], 0
  mov [run_file_70.param3], 3*256*2
  mov [run_file_70.param4], eax
;  mov [run_file_70.rezerv], 0
;  mov [run_file_70.name], file_name

  mov eax,70 ;load icon file
  mov ebx,run_file_70
  int 0x40
  cmp ebx,0
  jg @f
    mov dword[tree1.data_img],0
  @@:
;------------------------------------------------------------------------------
  copy_path fn_syntax_dir,sys_path,file_name,0x0 ;берем путь к папке с файлами синтаксиса
  mov eax,70
  mov ebx,tree_file_struct
  int 0x40

cmp ebx,-1
je .end_dir_init

  mov eax,dir_mem
  add eax,32+4+1+3+4*6+8
mov ecx,ebx
@@:
  cmp byte[eax],'.' ;фильтруем файлы с именами '.' и '..'
  je .filter
    push dword tree1
    push dword 0x10000 ;1*2^16 - где 1 номер иконки с книгой
    push dword eax
    call dword[tl_node_add]

    push dword tree1
    call dword[tl_cur_next]
  .filter:
  add eax,304
  loop @b
.end_dir_init:

;  push dword tree1
;  push dword 0
;  push dword file_name
;  call dword[tl_node_add]


;--- load color option file ---
  call InitColText

;--- get cmd line ---
  cmp byte[buf_cmd_lin+3],0 ;buf_cmd_lin
  je @f ;if file names exist
    mov esi,buf_cmd_lin
    call strlen ;eax=strlen
    mov [edit1.size],eax
    call but_OpenFile
  @@:


red_win:
  mcall 12,1

  xor eax,eax
  mov ebx,10*65536+485
  mov ecx,10*65536+280
  mov edx,[sc.work]
  or  edx,0x73000000
  mov edi,hed
  int 0x40

  mov edi,tedit0

  mcall 9,procinfo,-1
  stdcall EvSize,edi

  mov eax,13 ;тхЁїэшщ яЁ ьюєуюы№эшъ, фы  юўшёЄъш тхЁїэхщ ярэхыш
  xor ebx,ebx
  mov ecx,ted_wnd_t
  mov bx,word [procinfo.client_box.width]
  inc bx
  int 0x40

  stdcall [edit_box_draw], dword edit1
  stdcall [menu_bar_draw], dword menu_data_1

  call draw_but_toolbar

  cmp [err_ini0],1
  jne @f
    mov eax,4
    mov ebx,ted_wnd_l
    add ebx,ted_rec_l
    shl ebx,16
    add ebx,ted_wnd_t
    add ebx,ted_rec_t
    mov ecx,0x80ff0000
    mov edx,txtErrIni0
    int 0x40

    add ebx,10 ;move <--y-->
    mov ecx,0x80ff0080
    mov edx,file_name
    int 0x40
  @@:

  cmp [err_ini0],1
  je @f
    stdcall draw_main_win, tedit0
  @@:

  mcall 12,2

still:
  mov eax,10
  mcall

  cmp al,1 ;изм. положение окна
  jz red_win
  cmp al,2
  jz key
  cmp al,3
  jz button
  cmp al,6
  jz mouse

  jmp still



mouse:
  stdcall [edit_box_mouse], dword edit1

  test word [edit1.flags],10b;ed_focus ; если не в фокусе, выходим
  jne still

;-----------------------------------------------
  cmp [hScr.delta2],0
  jne .horizontal
.vertical:
  mov eax,[wScr.max_area]
  cmp eax,[wScr.cur_area]
  jbe .horizontal
; mouse event for Vertical ScrollBar
  stdcall [scrollbar_ver_mouse], dword wScr
  mov eax,wScr.redraw
  xor ebx,ebx
  cmp [eax],ebx
  je @f
  mov [eax],ebx

  stdcall draw_main_win, tedit0
  jmp still
@@:
  cmp [wScr.delta2],0
  jne still
.horizontal:
    mov   eax,[hScr.max_area]
    cmp   eax,[hScr.cur_area]
    jbe   .other
; mouse event for Horizontal ScrollBar
	push dword hScr
	call [scrollbar_hor_mouse]
	mov eax,hScr.redraw
	xor ebx,ebx
	cmp [eax],ebx
	je .other
	mov [eax],ebx
  stdcall draw_main_win, tedit0
  jmp still
.other:
  cmp [wScr.delta2],0
  jne still
  cmp [hScr.delta2],0
  jne still

  stdcall mouse_wnd_main, tedit0

  cmp byte[panel_id],TE_PANEL_FIND ;if not panel
  jne @f
    push dword edit2
    call [edit_box_mouse]
  @@:
  cmp byte[panel_id],TE_PANEL_SYNTAX ;if not panel
  jne .menu_bar_1 ;@f
  push dword tree1
  call [tl_mouse]
;-----------------------------------------------
.menu_bar_1:
  mov [menu_data_1.get_mouse_flag],1
; mouse event for Menu 1
  push	dword menu_data_1
  call	[menu_bar_mouse]
  cmp	[menu_data_1.click],dword 1
  jne	.mnu_1
  cmp [menu_data_1.cursor_out],dword 4
  je	button.exit	
  cmp [menu_data_1.cursor_out],dword 3
  jne	@f
    call but_SaveFile
  @@:
  cmp [menu_data_1.cursor_out],dword 2
  jne	@f
    call but_OpenFile
  @@:
  cmp [menu_data_1.cursor_out],dword 1
  jne	@f
    call but_NewFile
  @@:
  ;cmp [menu_data_1.cursor_out],dword 0
  ;jne @f
.mnu_1:
  jmp still
;---------------------------------------------------------------------

KeyConvertToASCII:
  mov ebx,conv_tabl ;convert scan to ascii
  ror ax,8
  xor ah,ah
  add bx,ax
  mov ah,byte[ebx]
  ret

key:
  mov ecx,1
  mcall 66,3
  xor ebx,ebx
  test al,0x03 ;[Shift]
  jz @f
    inc cl
    or ebx,KM_SHIFT
  @@:
  test al,0x0c ;[Ctrl]
  jz @f
    or ebx,KM_CTRL
  @@:
  test al,0x30 ;[Alt]
  jz @f
    or ebx,KM_ALT
  @@:
  test al,0x80 ;[NumLock]
  jz @f
    or ebx,KM_NUMLOCK
  @@:

  mov [keyUpr],ebx
  mcall 26,2,,conv_tabl
  mcall 2

;  push dword tree1 ;???
;  call [tl_key]    ;???

  test word [edit1.flags],10b;ed_focus ; если не в фокусе, выходим
  je @f
    cmp ah,0x80 ;if key up
    ja still
    cmp ah,42 ;[Shift] (left)
    je still
    cmp ah,54 ;[Shift] (right)
    je still
    cmp ah,56 ;[Alt]
    je still
    cmp ah,29 ;[Ctrl]
    je still
    cmp ah,69 ;[Pause Break]
    je still
;    cmp [keyUpr],0
;    jne still

    call KeyConvertToASCII

    push dword edit1
    call [edit_box_key]
    jmp still
  @@:

  test word [edit2.flags],10b;ed_focus ; если не в фокусе, выходим
  je @f
    cmp ah,0x80 ;if key up
    ja still
    cmp ah,42 ;[Shift] (left)
    je still
    cmp ah,54 ;[Shift] (right)
    je still
    cmp ah,56 ;[Alt]
    je still
    cmp ah,29 ;[Ctrl]
    je still
    cmp ah,69 ;[Pause Break]
    je still
;    cmp [keyUpr],0
;    jne still

    call KeyConvertToASCII
    stdcall [edit_box_key], dword edit2
    jmp still
  @@:

  cmp ah,KEY_F1 ;[F1]
  jne @f
    call ShowHelpF1
    jmp still
  @@:
  cmp ah,KEY_F3 ;[F3]
  jne @f
    call but_FindText
    jmp still
  @@:

  test [keyUpr],KM_CTRL ;Ctrl+...
  jz .key_Ctrl

    cmp ah,24 ;Ctrl+O
    jne @f
      call but_OpenFile
    @@:
    cmp ah,33 ;Ctrl+F
    jne @f
    cmp byte[panel_id],TE_PANEL_FIND
    je @f
      stdcall but_find, tedit0
    @@:
    cmp ah,44 ;Ctrl+Z
    je but_undo
    cmp ah,46 ;Ctrl+C
    jne @f
      call but_Copy
    @@:
    cmp ah,47 ;Ctrl+V
    je but_paste
    cmp ah,49 ;Ctrl+N
    jne @f
      call but_NewFile
    @@:
    cmp ah,199 ;Ctrl+Home
    jne @f
      call but_CtrlHome
    @@:
    ;jmp still
  .key_Ctrl:

  test [keyUpr],KM_SHIFT ;Shift+...
  jz .key_Shift
    cmp ah,72 ;Shift+Up
    jne @f
      call sel_KeyUp
    @@:
    cmp ah,75 ;Shift+Left
    jne @f
      call sel_KeyLeft
    @@:
    cmp ah,77 ;Shift+Right
    jne @f
      call sel_KeyRight
    @@:
    cmp ah,80 ;Shift+Down
    jne @f
      call sel_KeyDown
    @@:
    ;mov [dragk],1 ;начинаем выделение от клавиатуры
    jmp .key_MoveCur
  .key_Shift:
;-------------------------------------------------
  cmp ah,72 ;178 ;Up
  jne @f
    push dx
    stdcall draw_cursor_sumb, tedit0
    call CurMoveUp
    cmp dl,8
    jne .no_red_0
      call OnInitialUpdate
      stdcall draw_main_win, tedit0
      pop dx
      jmp @f
    .no_red_0:
    stdcall draw_main_cursor, tedit0
    pop dx
    mov [dragk],0 ;заканчиваем выделение от клавиатуры
  @@:
  cmp ah,80 ;177 ;Down
  jne @f
    push dx
    stdcall draw_cursor_sumb, tedit0
    call CurMoveDown
    cmp dl,8
    jne .no_red_1
      call OnInitialUpdate
      stdcall draw_main_win, tedit0
      pop dx
      jmp @f
    .no_red_1:
    stdcall draw_main_cursor, tedit0
    pop dx
    mov [dragk],0 ;заканчиваем выделение от клавиатуры
  @@:
  cmp ah,75 ;176 ;Left
  jne @f
    push dx
    stdcall draw_cursor_sumb, tedit0
    call CurMoveLeft
    cmp dl,8
    jne .no_red_2
      call OnInitialUpdate
      stdcall draw_main_win, tedit0
      pop dx
      jmp @f
    .no_red_2:
    stdcall draw_main_cursor, tedit0
    pop dx
    mov [dragk],0 ;заканчиваем выделение от клавиатуры
  @@:
  cmp ah,77 ;179 ;Right
  jne @f
    push dx
    stdcall draw_cursor_sumb, tedit0
    call CurMoveRight
    cmp dl,8
    jne .no_red_3
      call OnInitialUpdate
      stdcall draw_main_win, tedit0
      pop dx
      jmp @f
    .no_red_3:
    stdcall draw_main_cursor, tedit0
    pop dx
    mov [dragk],0 ;заканчиваем выделение от клавиатуры
  @@:
  cmp ah,71 ;180 ;Home
  jne @f
    push dx
    stdcall draw_cursor_sumb, tedit0
    call CurMoveX_FirstChar
    cmp dl,8
    jne .no_red_4
      call OnInitialUpdate
      stdcall draw_main_win, tedit0
      pop dx
      jmp @f
    .no_red_4:
    stdcall draw_main_cursor, tedit0
    pop dx
    mov [dragk],0 ;заканчиваем выделение от клавиатуры
  @@:
  cmp ah,79 ;181 ;End
  jne @f
    push dx
    stdcall draw_cursor_sumb, tedit0
    call CurMoveX_LastChar
    cmp dl,8
    jne .no_red_5
      call OnInitialUpdate
      stdcall draw_main_win, tedit0
      pop dx
      jmp @f
    .no_red_5:
    stdcall draw_main_cursor, tedit0
    pop dx
    mov [dragk],0 ;заканчиваем выделение от клавиатуры
  @@:
  cmp ah,73 ;184 ;PageUp
  jne @f
    push dx
    call CurMovePageUp
    cmp dl,0
    pop dx
    je @f
    call OnInitialUpdate
    stdcall draw_main_win, tedit0
  @@:
  cmp ah,81 ;183 ;PageDown
  jne @f
    push dx
    call CurMovePageDown
    cmp dl,0
    pop dx
    je @f
    call OnInitialUpdate
    stdcall draw_main_win, tedit0
    mov [dragk],0 ;заканчиваем выделение от клавиатуры
  @@:
;-------------------------------------------------
    cmp [keyUpr],0
    jne still
  .key_MoveCur:

  cmp ah,69 ;[Pause Break]
  je still
  cmp ah,120 ;[Fn]
  je still
  cmp ah,0x80 ;if key up
  ja still
  call KeyConvertToASCII

  ;mov [dragk],0 ;заканчиваем выделение от клавиатуры

  push ebx
  xor ebx,ebx
  mov bl,ah
  add ebx,EvChar ;add char to text
  cmp byte [ebx],1
  jne @f
    push esi edi
    call SetUndo
    mov bx,0x0101
    call SelTextDel
    mov esi,1
    mov byte [key_new],ah
    mov edi,dword key_new
    cmp [curMod],1
    je .no_ins_mod
      call TextDel
      xor bl,1
    .no_ins_mod:
    call TextAdd
    call draw_but_toolbar
    cmp byte [key_new],13
    jne .dr_m_win
      stdcall draw_main_win, tedit0
      jmp .dr_cur_l
    .dr_m_win:
      stdcall draw_cur_line, tedit0
    .dr_cur_l:
    pop edi esi
  @@:
  pop ebx

  cmp ah,8 ;[<-]
  jne @f
    push ax bx
    call SetUndo

    mov bx,0x0001
    call SelTextDel
    cmp al,1
    je .del_one_b
      call TextDel
    .del_one_b:
    call draw_but_toolbar
    stdcall draw_main_win, tedit0
    pop bx ax
  @@:

  cmp ah,182 ;Delete
  jne @f
    push ax bx
    call SetUndo

    mov bx,0x0101
    call SelTextDel
    cmp al,1
    je .del_one_d
      call TextDel
    .del_one_d:
    call draw_but_toolbar
    stdcall draw_main_win, tedit0
    pop bx ax
  @@:

  cmp ah,185 ;Ins
  jne @f
    stdcall draw_cursor_sumb, tedit0
    xor [curMod],1
    stdcall draw_main_cursor, tedit0
  @@:

  jmp still

button:
;  cmp [menu_active],1 ;если нажали меню, то сначала реакция на меню
;  jne @f ;mouse.menu_bar_1
;    mov [menu_active],0
;    jmp still
;  @@:

  mcall 17 ;получить код нажатой кнопки
  cmp ah,3
  jne @f
    call but_NewFile
  @@:
  cmp ah,4
  jne @f
    call but_OpenFile
  @@:
  cmp ah,5
  jne @f
    call but_SaveFile
  @@:
  cmp ah,6
  jz  but_select_word
  cmp ah,7
  jz  but_cut
  cmp ah,8
  jne @f
    call but_Copy
  @@:
  cmp ah,9
  jz  but_paste
  cmp ah,10
  jne @f
    stdcall but_find, tedit0
  @@:
  cmp ah,11
  jz  but_replace
  cmp ah,12
  jz  but_find_key_w
  cmp ah,13
  jz  but_sumb_upper
  cmp ah,14
  jz  but_sumb_lover
  cmp ah,15
  jz  but_reverse
  cmp ah,16
  jz  but_undo
  cmp ah,17
  jz  but_redo
  cmp ah,18
  jz  but_sumb_invis
  cmp ah,19
  jz  but_k_words_show
  cmp ah,20
  jne @f
    stdcall but_synt_show, tedit0
  @@:

  cmp ah,200
  jne @f
    call but_OpenSyntax
  @@:
  cmp ah,201
  jne @f
    call but_FindText
  @@:

  cmp ah,1
  jne still
.exit:
  ;push eax
  call CanSave
  cmp al,1
  jne @f
    push thread
    push msgbox_8
    call [mb_create] ;message: save changes in file?
    jmp still
  @@:
  mov ecx,[tex]
  call mem_Free
  mov ecx,[bmp_icon]
  call mem_Free
  mov ecx,[options_file]
  call mem_Free
  push dword tree1
  call dword[tl_data_clear]
  mcall -1 ;выход из программы



txtErrIni0 db 'Не открылся файл с иконками',0
err_ini0 db 0

edit1 edit_box 250, 220, 5, 0xffffff, 0xff80, 0xff0000, 0xff, 0x4080, 300, buf_cmd_lin, mouse_dd, 0
edit2 edit_box TE_PANEL_WIDTH-1, 0, 20, 0xffffff, 0xff80, 0xff0000, 0xff, 0x4080, 300, buf_find, mouse_dd, 0

buf_cmd_lin db 302 dup(0)
buf_find db 302 dup(0)

if lang eq ru
  err_message_found_lib0 db 'Извините не удалось найти библиотеку box_lib.obj',0
  head_f_i0:
  head_f_l0  db 'Системная ошибка',0
  err_message_import0 db 'Ошибка при импорте библиотеки box_lib.obj',0
  err_message_found_lib1 db 'Извините не удалось найти библиотеку msgbox.obj',0
  err_message_import1 db 'Ошибка при импорте библиотеки msgbox.obj',0
else
  err_message_found_lib0 db 'Sorry I cannot found library box_lib.obj',0
  head_f_i0:
  head_f_l0  db 'System error',0
  err_message_import0 db 'Error on load import library box_lib.obj',0
  err_message_found_lib1 db 'Sorry I cannot found library msgbox.obj',0
  ;head_f_i1:
  ;head_f_l1 db 'System error',0
  err_message_import1 db 'Error on load import library msgbox.obj',0
end if

;library structures
l_libs_start:
  lib0 l_libs boxlib_name, sys_path, file_name, system_dir0, err_message_found_lib0, head_f_l0, myimport,err_message_import0, head_f_i0
  lib1 l_libs msgbox_name, sys_path, file_name, system_dir1, err_message_found_lib1, head_f_l0, msgbox_lib_import, err_message_import1, head_f_i0
load_lib_end:


i_end:
  align 16
  procinfo process_information
  thread:
    rb 1024
stacktop:
  sys_path:
    rb 4096
  file_name:
    rb 4096
  file_name_rez:
    rb 4096
  file_info:
    rb 40
mem:
