KMENUITEM_NORMAL equ 0
KMENUITEM_SUBMENU equ 1
KMENUITEM_SEPARATOR equ 2

init_main_menu:
	stdcall [kmenu_init], syscolors
	
	stdcall [ksubmenu_new]
	mov [main_menu], eax
	
	stdcall [ksubmenu_new]
	mov [main_menu_file], eax
	stdcall [ksubmenu_new]
	mov [main_menu_edit], eax
	stdcall [ksubmenu_new]
	mov [main_menu_instr], eax
	
	; init 'File' menu
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_File_New, 0x71
	stdcall [ksubmenu_add], [main_menu_file], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_File_Open, 0x72
	stdcall [ksubmenu_add], [main_menu_file], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_File_Save, 0x73
	stdcall [ksubmenu_add], [main_menu_file], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_File_SaveAs, 0x74
	stdcall [ksubmenu_add], [main_menu_file], eax
	stdcall [kmenuitem_new], KMENUITEM_SEPARATOR, 0, 0
	stdcall [ksubmenu_add], [main_menu_file], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_File_Exit, 0x75
	stdcall [ksubmenu_add], [main_menu_file], eax
	
	; add 'File' menu to main
	stdcall [kmenuitem_new], KMENUITEM_SUBMENU, sz_main_menu_File, [main_menu_file]
	stdcall [ksubmenu_add], [main_menu], eax
	
	; init 'Edit' menu
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Edit_Undo, 0x61
	stdcall [ksubmenu_add], [main_menu_edit], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Edit_Redo, 0x62
	stdcall [ksubmenu_add], [main_menu_edit], eax
	stdcall [kmenuitem_new], KMENUITEM_SEPARATOR, 0, 0
	stdcall [ksubmenu_add], [main_menu_edit], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Edit_Copy, 0x63
	stdcall [ksubmenu_add], [main_menu_edit], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Edit_Paste, 0x64
	stdcall [ksubmenu_add], [main_menu_edit], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Edit_Cut, 0x65
	stdcall [ksubmenu_add], [main_menu_edit], eax
	stdcall [kmenuitem_new], KMENUITEM_SEPARATOR, 0, 0
	stdcall [ksubmenu_add], [main_menu_edit], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Edit_ClS, 0x66
	stdcall [ksubmenu_add], [main_menu_edit], eax
	
	; add 'Edit' menu to main
	stdcall [kmenuitem_new], KMENUITEM_SUBMENU, sz_main_menu_Edit, [main_menu_edit]
	stdcall [ksubmenu_add], [main_menu], eax

	; init 'Edit' menu
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Instr_Pencil, 0x51
	stdcall [ksubmenu_add], [main_menu_instr], eax

	; add 'Instruments' menu to main
	stdcall [kmenuitem_new], KMENUITEM_SUBMENU, sz_main_menu_Instr, [main_menu_instr]
	stdcall [ksubmenu_add], [main_menu], eax

	ret

put_main_menu:
	stdcall [kmainmenu_draw], [main_menu]
ret

button_handler_main_menu:
	cmp eax, 0x71
	jne @f
		mov [number_panel], 7
		mov [number_menu], 1
		jmp TakeInstruments
@@:
	cmp eax, 0x72
	jne @f
		mov [number_panel], 7
		mov [number_menu], 2
		jmp TakeInstruments
@@:
	cmp eax, 0x73
	jne @f
		mov [number_panel], 7
		mov [number_menu], 3
		jmp TakeInstruments
@@:
	cmp eax, 0x74
	jne @f
		mov [number_panel], 7
		mov [number_menu], 4
		jmp TakeInstruments
@@:
	cmp eax, 0x75
	jne @f
		mov [number_panel], 7
		mov [number_menu], 5
		jmp TakeInstruments
@@:
	cmp eax, 0x61
	jne @f
		mov [number_panel], 6
		mov [number_menu], 1
		jmp TakeInstruments
@@:
	cmp eax, 0x62
	jne @f
		mov [number_panel], 6
		mov [number_menu], 2
		jmp TakeInstruments
@@:
	cmp eax, 0x63
	jne @f
		mov [number_panel], 6
		mov [number_menu], 3
		jmp TakeInstruments
@@:
	cmp eax, 0x64
	jne @f
		mov [number_panel], 6
		mov [number_menu], 4
		jmp TakeInstruments
@@:
	cmp eax, 0x65
	jne @f
		mov [number_panel], 6
		mov [number_menu], 5
		jmp TakeInstruments
@@:
	cmp eax, 0x66
	jne @f
		mov [number_panel], 6
		mov [number_menu], 6
		jmp TakeInstruments
@@:
	cmp eax, 0x51
	jne @f
		mov [number_panel], 5
		mov [number_menu], 1
		jmp TakeInstruments
@@:
.end_handler:
ret

main_menu dd 0
main_menu_file  dd 0
main_menu_edit dd 0
main_menu_instr dd 0

sz_main_menu_File        db 'File', 0
sz_main_menu_File_New    db 'New', 0
sz_main_menu_File_Open   db 'Open', 0
sz_main_menu_File_Save   db 'Save', 0
sz_main_menu_File_SaveAs db 'Save as', 0
sz_main_menu_File_Exit   db 'Exit', 0

sz_main_menu_Edit       db 'Edit', 0
sz_main_menu_Edit_Undo  db 'Undo', 0
sz_main_menu_Edit_Redo  db 'Redo', 0
sz_main_menu_Edit_Copy  db 'Copy', 0
sz_main_menu_Edit_Paste db 'Paste', 0
sz_main_menu_Edit_Cut   db 'Cut', 0
sz_main_menu_Edit_ClS   db 'Clear screen', 0

sz_main_menu_Instr       db 'Instruments', 0
sz_main_menu_Instr_Pencil db 'Pencil', 0