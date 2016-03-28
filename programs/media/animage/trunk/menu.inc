KMENUITEM_NORMAL equ 0
KMENUITEM_SUBMENU equ 1
KMENUITEM_SEPARATOR equ 2

macro menu_set code, m_group, m_number
{
local no_set

cmp eax, code
jne no_set
	mov [number_panel], m_group
	mov [number_menu], m_number
	jmp TakeInstruments
no_set:
}

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
	stdcall [ksubmenu_new]
	mov [main_menu_filters], eax
	
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
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Edit_SelectAll, 0x67
	stdcall [ksubmenu_add], [main_menu_edit], eax
	
	; add 'Edit' menu to main
	stdcall [kmenuitem_new], KMENUITEM_SUBMENU, sz_main_menu_Edit, [main_menu_edit]
	stdcall [ksubmenu_add], [main_menu], eax

	; init 'Instruments' menu
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Instr_Pencil, 0x51
	stdcall [ksubmenu_add], [main_menu_instr], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Instr_Brush, 0x52
	stdcall [ksubmenu_add], [main_menu_instr], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Instr_Spray, 0x53
	stdcall [ksubmenu_add], [main_menu_instr], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Instr_Lastik, 0x54
	stdcall [ksubmenu_add], [main_menu_instr], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Instr_Flood_fill, 0x55
	stdcall [ksubmenu_add], [main_menu_instr], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Instr_Pipetka, 0x56
	stdcall [ksubmenu_add], [main_menu_instr], eax
	stdcall [kmenuitem_new], KMENUITEM_SEPARATOR, 0, 0
	stdcall [ksubmenu_add], [main_menu_instr], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Instr_Line, 0x57
	stdcall [ksubmenu_add], [main_menu_instr], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Instr_Rectangle, 0x58
	stdcall [ksubmenu_add], [main_menu_instr], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Instr_Circle, 0x59
	stdcall [ksubmenu_add], [main_menu_instr], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Instr_Ellips, 0x5a
	stdcall [ksubmenu_add], [main_menu_instr], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Instr_Hard_contour, 0x5b
	stdcall [ksubmenu_add], [main_menu_instr], eax

	; add 'Instruments' menu to main
	stdcall [kmenuitem_new], KMENUITEM_SUBMENU, sz_main_menu_Instr, [main_menu_instr]
	stdcall [ksubmenu_add], [main_menu], eax

	; init 'Reflection' menu
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Filters_ReflectionLR, 0x41
	stdcall [ksubmenu_add], [main_menu_filters], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Filters_ReflectionTB, 0x42
	stdcall [ksubmenu_add], [main_menu_filters], eax

	; add 'Reflection' menu to main
	stdcall [kmenuitem_new], KMENUITEM_SUBMENU, sz_main_menu_Filters, [main_menu_filters]
	stdcall [ksubmenu_add], [main_menu], eax
	ret

put_main_menu:
	stdcall [kmainmenu_draw], [main_menu]
ret

button_handler_main_menu:
	menu_set 0x71, 7,1
	menu_set 0x72, 7,2
	menu_set 0x73, 7,3
	menu_set 0x74, 7,4
	menu_set 0x75, 7,5
	menu_set 0x61, 6,1
	menu_set 0x62, 6,2
	menu_set 0x63, 6,3
	menu_set 0x64, 6,4
	menu_set 0x65, 6,5
	menu_set 0x66, 6,6
	menu_set 0x67, 6,7
	menu_set 0x51, 5,1
	menu_set 0x52, 5,2
	menu_set 0x53, 5,3
	menu_set 0x54, 5,4
	menu_set 0x55, 5,5
	menu_set 0x56, 5,6
	menu_set 0x57, 5,7
	menu_set 0x58, 5,8
	menu_set 0x59, 5,9
	menu_set 0x5a, 5,10
	menu_set 0x5b, 5,11
	menu_set 0x41, 4,1
	menu_set 0x42, 4,2
.end_handler:
ret

main_menu dd 0
main_menu_file  dd 0
main_menu_edit dd 0
main_menu_instr dd 0
main_menu_filters dd 0

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
sz_main_menu_Edit_SelectAll db 'Select all', 0

sz_main_menu_Instr       db 'Instruments', 0
sz_main_menu_Instr_Pencil db 'Pencil', 0
sz_main_menu_Instr_Brush db 'Brush', 0
sz_main_menu_Instr_Spray db 'Spray', 0
sz_main_menu_Instr_Lastik db 'Lastik', 0
sz_main_menu_Instr_Flood_fill db 'Flood fill', 0
sz_main_menu_Instr_Pipetka db 'Pipetka', 0
sz_main_menu_Instr_Line db 'Line', 0
sz_main_menu_Instr_Rectangle db 'Rectangle', 0
sz_main_menu_Instr_Circle db 'Circle', 0
sz_main_menu_Instr_Ellips db 'Ellips', 0
sz_main_menu_Instr_Hard_contour db 'Hard contour', 0

sz_main_menu_Filters      db 'Filters', 0
sz_main_menu_Filters_ReflectionLR db 'Reflection |', 0
sz_main_menu_Filters_ReflectionTB db 'Reflection -', 0