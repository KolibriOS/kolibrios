align 4
main_menu dd 0
main_menu_file dd 0
main_menu_select dd 0
main_menu_sort dd 0
main_menu_help dd 0

KMENUITEM_NORMAL equ 0
KMENUITEM_SUBMENU equ 1
KMENUITEM_SEPARATOR equ 2

align 4
draw_menu_bar:
    pushad
    mov   ecx,19
    mov   ebx,[window_width]
    sub   ebx,9
    mcall SF_DRAW_RECT, ,,[sc.work]
    pushad
    cld
    stdcall [kmainmenu_draw], [main_menu]
    popad
    add   ecx,37 shl 16
    mov   cx,FILE_BR_TOP_LINE-67
	add   cx,word[skin_high]
    mcall ; horizontal line before disk buttons
    push  ecx
	ror   ecx,16
    add   cx,FILE_BR_TOP_LINE-54
	add   cx,word[skin_high]
	ror   ecx,16
    mov   cx,3
    mcall ; horizontal line after disk buttons
    add   ecx,16 shl 16
    mov   cx,1
    mcall ; horizontal line before sort buttons
    pop   ecx
    shr   ecx,16
    add   ecx,3
    mov   eax,[window_high]
    sub   eax,ecx
    sub   eax,4
    shl   ecx,16
    add   ecx,eax
    mov   bx,2
    sub   ecx,33
    sub   ecx,[skin_high]
    mcall SF_DRAW_RECT ; left vertical line
    push  ebx
    movzx ebx,[left_panel_x.start_x]
    add   bx,[left_panel_x.size_x]
    inc   bx
    mov   si,bx
    shl   ebx,16
    add   bx,[right_panel_x.start_x]
    sub   bx,si
    push  ecx
    sub   ecx,13
    add   ecx,13 shl 16
    mcall ; middle vertical line
    pop   ecx
    pop   ebx
    mov   ax,cx
    dec   ax
    shl   eax,16
    add   ecx,eax
    mov   cx,3
    mov   eax,[window_width] 
    mov   bx,ax
    sub   ebx,9
    mcall SF_DRAW_RECT ; horizontal line before path to the current folder
    add   ecx,15 shl 16
    mcall ; horizontal line after path to the current folder
    popad
    ret

align 4
init_menu:
    cmp     [library03.status_lib],0 ; test load library
    jnz     .end
    stdcall [kmenu_init], sc
    stdcall [ksubmenu_new]
    mov     [main_menu], eax

    stdcall [ksubmenu_new]
    mov     [main_menu_file], eax
    stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_menu_Copy_file, 153
    stdcall [ksubmenu_add], [main_menu_file], eax
    stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_menu_Move_file, 154
    stdcall [ksubmenu_add], [main_menu_file], eax
    stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_menu_Create_dir, 152
    stdcall [ksubmenu_add], [main_menu_file], eax
    stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_menu_Delete_files, 157
    stdcall [ksubmenu_add], [main_menu_file], eax
    stdcall [kmenuitem_new], KMENUITEM_SEPARATOR, 0, 0
    stdcall [ksubmenu_add], [main_menu_file], eax
    stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_menu_Exit, 112
    stdcall [ksubmenu_add], [main_menu_file], eax
    stdcall [kmenuitem_new], KMENUITEM_SUBMENU, sz_main_menu_File, [main_menu_file]
    stdcall [ksubmenu_add], [main_menu], eax

    ;stdcall [ksubmenu_new]
    ;mov     [main_menu_select], eax
    ;stdcall [kmenuitem_new], KMENUITEM_SUBMENU, sz_main_menu_Select, [main_menu_select]
    ;stdcall [ksubmenu_add], [main_menu], eax

    cmp     [sort_init], f_null_1
    je      @f
    stdcall [ksubmenu_new]
    mov     [main_menu_sort], eax
    stdcall [kmenuitem_new], KMENUITEM_NORMAL, sort_button_name.name, 142
    stdcall [ksubmenu_add], [main_menu_sort], eax
    stdcall [kmenuitem_new], KMENUITEM_NORMAL, sort_button_name.type, 143
    stdcall [ksubmenu_add], [main_menu_sort], eax
    stdcall [kmenuitem_new], KMENUITEM_NORMAL, sort_button_name.size, 144
    stdcall [ksubmenu_add], [main_menu_sort], eax
    stdcall [kmenuitem_new], KMENUITEM_NORMAL, sort_button_name.date, 145
    stdcall [ksubmenu_add], [main_menu_sort], eax
    stdcall [kmenuitem_new], KMENUITEM_SEPARATOR, 0, 0
    stdcall [ksubmenu_add], [main_menu_sort], eax
    stdcall [kmenuitem_new], KMENUITEM_NORMAL, sort_button_name.name, 146
    stdcall [ksubmenu_add], [main_menu_sort], eax
    stdcall [kmenuitem_new], KMENUITEM_NORMAL, sort_button_name.type, 147
    stdcall [ksubmenu_add], [main_menu_sort], eax
    stdcall [kmenuitem_new], KMENUITEM_NORMAL, sort_button_name.size, 148
    stdcall [ksubmenu_add], [main_menu_sort], eax
    stdcall [kmenuitem_new], KMENUITEM_NORMAL, sort_button_name.date, 149
    stdcall [ksubmenu_add], [main_menu_sort], eax
    stdcall [kmenuitem_new], KMENUITEM_SUBMENU, sz_main_menu_Sort, [main_menu_sort]
    stdcall [ksubmenu_add], [main_menu], eax
@@:
    stdcall [ksubmenu_new]
    mov     [main_menu_help], eax
    stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_menu_About, 101
    stdcall [ksubmenu_add], [main_menu_help], eax
    stdcall [kmenuitem_new], KMENUITEM_SUBMENU, sz_main_menu_Help, [main_menu_help]
    stdcall [ksubmenu_add], [main_menu], eax
.end:
    ret