;
;   MENU EXAMPLE
;
;   Compile with FASM for Kolibri
;

  use32
  org    0x0
  db     'MENUET01'    ; 8 byte id
  dd     0x01          ; header version
  dd     start         ; start of code
  dd     i_end         ; size of image
  dd     mem, stacktop, file_name, sys_path

include 'lang.inc'
include '../../../../macros.inc'
include '../../../../proc32.inc'
include '../../../../KOSfuncs.inc'
include '../../../../develop/libraries/box_lib/load_lib.mac'
include '../../../../dll.inc'

KMENUITEM_NORMAL equ 0
KMENUITEM_SUBMENU equ 1
KMENUITEM_SEPARATOR equ 2

@use_library_mem mem.Alloc,mem.Free,mem.ReAlloc,dll.Load

start:                             ; start of execution

	load_libraries l_libs_start,l_libs_end
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
	mcall SF_SET_EVENTS_MASK,0x27

	;kmenu initialisation
	stdcall [kmenu_init],sc
	stdcall [ksubmenu_new]
	mov [main_menu], eax

	stdcall [ksubmenu_new]
	mov [main_menu_File], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_Load, 110
	stdcall [ksubmenu_add], [main_menu_File], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_Save, 111
	stdcall [ksubmenu_add], [main_menu_File], eax
	stdcall [kmenuitem_new], KMENUITEM_SEPARATOR, 0, 0
	stdcall [ksubmenu_add], [main_menu_File], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_Quit, 112
	stdcall [ksubmenu_add], [main_menu_File], eax
	stdcall [kmenuitem_new], KMENUITEM_SUBMENU, sz_File, [main_menu_File]
	stdcall [ksubmenu_add], [main_menu], eax

	stdcall [ksubmenu_new]
	mov [main_menu_Edit], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_Copy, 120
	stdcall [ksubmenu_add], [main_menu_Edit], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_Paste, 121
	stdcall [ksubmenu_add], [main_menu_Edit], eax
	stdcall [kmenuitem_new], KMENUITEM_SUBMENU, sz_Edit, [main_menu_Edit]
	stdcall [ksubmenu_add], [main_menu], eax

	stdcall [ksubmenu_new]
	mov [main_menu_Test], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_Test1, 130
	stdcall [ksubmenu_add], [main_menu_Test], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_Test2, 131
	stdcall [ksubmenu_add], [main_menu_Test], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_Test3, 132
	stdcall [ksubmenu_add], [main_menu_Test], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_Test4, 133
	stdcall [ksubmenu_add], [main_menu_Test], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_Test5, 134
	stdcall [ksubmenu_add], [main_menu_Test], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_Test6, 134
	stdcall [ksubmenu_add], [main_menu_Test], eax
	stdcall [kmenuitem_new], KMENUITEM_SUBMENU, sz_Test, [main_menu_Test]
	stdcall [ksubmenu_add], [main_menu], eax

	stdcall [ksubmenu_new]
	mov [main_menu_Help], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_Setup, 140
	stdcall [ksubmenu_add], [main_menu_Help], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_About, 141
	stdcall [ksubmenu_add], [main_menu_Help], eax
	stdcall [kmenuitem_new], KMENUITEM_SUBMENU, sz_Help, [main_menu_Help]
	stdcall [ksubmenu_add], [main_menu], eax

  red: 
    call draw_window               ; draw window
	stdcall [kmainmenu_draw], [main_menu]

still:
    mcall SF_WAIT_EVENT

    cmp  eax,1                     ; redraw request ?
    jz   red                       ; yes jump to it
    cmp  eax,2
    jz   key
    cmp  eax,3
    jz   button
	cmp  eax,6
	jne  @f 
		call mouse
		jmp still
	@@:

key:
    mcall SF_GET_KEY
    jmp  still                     ; start again

button:
	mcall SF_GET_BUTTON

	mov  [button_press],eax
	shr  dword[button_press],8
	call draw_data

	cmp  ah,1                      ; is it the close button
	jne  still                     ; no then jump code
	mcall SF_TERMINATE_PROCESS     ; close this program

mouse:
	stdcall [kmainmenu_dispatch_cursorevent], [main_menu]
	ret

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************

draw_window:

    push eax                       ; save register
    mcall SF_REDRAW,SSF_BEGIN_DRAW

    mov  ebx,50*65536              ; [x start] *65536
    add  ebx,[x_size]              ; add [x size]
    mov  ecx,50*65536              ; [y start] *65536
    add  ecx,[y_size]              ; add [y size]
    mcall SF_CREATE_WINDOW,,,0xb3ffffff,,window_text

    mov  ebx,1*65536               ; [x start] *65536
    add  ebx,[x_size]              ; add [x size]
    sub  ebx,11                    ; x size - 11
    mov  ecx,[y_size]              ; [y start] *65536
    sub  ecx,47                    ; minus height
    shl  ecx,16                    ; *65536
    add  ecx,17                    ; add height
    mcall SF_DRAW_RECT,,,0x006688dd

    mov  ebx,5*65536               ; [x start] *65536
    add  ebx,[y_size]              ; add [y start]
    sub  ebx,42                    ; move up
    xor  ecx,ecx                   ; text colour
    mcall SF_DRAW_TEXT,,,button_no,9

	call draw_data

    mcall SF_REDRAW,SSF_END_DRAW
    pop  eax                       ; restore register
    ret                            ; return

; *********** DRAW DATA ***********

draw_data:

    push eax                       ; save register
    mov  ecx,[y_size]              ; [y start]
    sub  ecx,45                    ; move position
    shl  ecx,16                    ; *65536
    add  ecx,13                    ; [y size]
    mov  edx,0x00aaaaaa            ; bar colour
    mcall SF_DRAW_RECT, (67 shl 16)+23

    mov  ebx,0x00030000            ; display 3 decimal characters
    mov  ecx,[button_press]        ; menu id
    mov  edx,70*65536              ; [x start] *65536
    add  edx,[y_size]              ; +[y start]
    sub  edx,42                    ; move position
    xor  esi,esi                   ; text colour
    mcall SF_DRAW_NUMBER
    pop  eax                       ; restore register

    ret                            ; return

; ***************** DATA AREA ******************

x_size:       dd 381               ; window x size
y_size:       dd 200               ; window y size

if lang eq ru
window_text   db 'Пример меню',0
button_no     db 'Кнопка N:'
else
window_text   db 'Menu example',0  ; grab bar text
button_no     db 'Button N:'  ; status bar text
end if

main_menu dd 0
main_menu_File dd 0
main_menu_Edit dd 0
main_menu_Test dd 0
main_menu_Help dd 0

if lang eq ru

sz_File db 'Файл',0
sz_Load db 'Открыть',0
sz_Save db 'Сохранить',0
sz_Quit db 'Выход',0

sz_Edit db 'Правка',0
sz_Copy db 'Копировать',0
sz_Paste db 'Вставить',0

sz_Test db 'Тест',0
sz_Test1 db 'Тест 1',0
sz_Test2 db 'Тест 2',0
sz_Test3 db 'Тест 3',0
sz_Test4 db 'Тест 4',0
sz_Test5 db 'Тест 5',0
sz_Test6 db 'Тест 6',0

sz_Help db 'Помощь',0
sz_Setup db 'Настройки',0
sz_About db 'О программе..',0

else

sz_File db 'File',0
sz_Load db 'Load',0
sz_Save db 'Save',0
sz_Quit db 'Quit',0

sz_Edit db 'Edit',0
sz_Copy db 'Copy',0
sz_Paste db 'Paste',0

sz_Test db 'Test',0
sz_Test1 db 'Test 1',0
sz_Test2 db 'Test 2',0
sz_Test3 db 'Test 3',0
sz_Test4 db 'Test 4',0
sz_Test5 db 'Test 5',0
sz_Test6 db 'Test 6',0

sz_Help db 'Help',0
sz_Setup db 'Setup',0
sz_About db 'About..',0

end if

button_press  dd 0

align 4
system_dir_0 db '/sys/lib/'
lib_name_0 db 'kmenu.obj',0

if lang eq ru
	head_f_i:
	head_f_l db 'Системная ошибка',0
	err_msg_found_lib_0 db 'Не найдена библиотека ',39,'kmenu.obj',39,0
	err_msg_import_0 db 'Ошибка при импорте библиотеки ',39,'kmenu',39,0
else
	head_f_i:
	head_f_l db 'System error',0
	err_msg_found_lib_0 db 'Sorry I cannot found library ',39,'kmenu.obj',39,0
	err_msg_import_0 db 'Error on load import library ',39,'kmenu.obj',39,0
end if

align 4
l_libs_start:
	lib_0 l_libs lib_name_0, sys_path, file_name, system_dir_0,\
		err_msg_found_lib_0, head_f_l, import_libkmenu,err_msg_import_0,head_f_i
l_libs_end:

align 4
import_libkmenu:
	kmenu_init      dd akmenu_init
	kmainmenu_draw  dd akmainmenu_draw
	kmainmenu_dispatch_cursorevent dd akmainmenu_dispatch_cursorevent
	ksubmenu_new    dd aksubmenu_new
	ksubmenu_delete dd aksubmenu_delete
	ksubmenu_draw   dd aksubmenu_draw
	ksubmenu_add    dd aksubmenu_add
	kmenuitem_new   dd akmenuitem_new
	kmenuitem_delete dd akmenuitem_delete
	kmenuitem_draw  dd akmenuitem_draw
dd 0,0
	akmenu_init     db 'kmenu_init',0
	akmainmenu_draw db 'kmainmenu_draw',0
	akmainmenu_dispatch_cursorevent db 'kmainmenu_dispatch_cursorevent',0
	aksubmenu_new   db 'ksubmenu_new',0
	aksubmenu_delete db 'ksubmenu_delete',0
	aksubmenu_draw  db 'ksubmenu_draw',0
	aksubmenu_add   db 'ksubmenu_add',0
	akmenuitem_new  db 'kmenuitem_new',0
	akmenuitem_delete db 'kmenuitem_delete',0
	akmenuitem_draw  db 'kmenuitem_draw',0

i_end:
	sc system_colors
	rb 2048
stacktop:
	sys_path rb 2048
	file_name rb 4096
mem:
