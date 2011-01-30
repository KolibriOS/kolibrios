
lib_console_init:

; load DLL
	mov     eax, 68
	mov     ebx, 19
	mov     ecx, dll_name
	int     0x40
	test    eax, eax
	jz      exit

; initialize import
	mov	edx, eax
	mov     esi, myimport
import_loop:
	lodsd
	test    eax, eax
	jz      import_done
	push    edx
import_find:
	mov     ebx, [edx]
	test    ebx, ebx
	jz      exit;import_not_found
	push    eax
@@:
	mov     cl, [eax]
	cmp     cl, [ebx]
	jnz     import_find_next
	test    cl, cl
	jz      import_found
	inc     eax
	inc     ebx
	jmp     @b
import_find_next:
	pop     eax
	add     edx, 8
	jmp     import_find
import_found:
	pop     eax
	mov     eax, [edx+4]
	mov     [esi-4], eax
	pop     edx
	jmp     import_loop
import_done:

ret


exit:
	or      eax, -1
	int     0x40


new_line	db	13, 10, 0
dll_name	db	'/sys/lib/console.obj',0
caption		db	'Eliza for KolibriOS', 0


align 4
myimport:
dll_start		dd      aStart
dll_ver			dd      aVersion
con_init		dd      aConInit
con_write_asciiz	dd	aConWriteAsciiz
con_exit		dd      aConExit
con_getch		dd	aCon_getch
con_cls 		dd	aCon_cls
con_set_cursor_pos	dd	aCon_set_cursor_pos
con_gets		dd	aCon_gets
con_printf		dd	aCon_printf
			dd      0

aStart			db	'START',0
aVersion		db	'version',0
aConInit		db	'con_init',0
aConWriteAsciiz		db	'con_write_asciiz',0
aConExit		db	'con_exit',0
aCon_getch		db	'con_getch',0
aCon_cls		db	'con_cls',0
aCon_set_cursor_pos	db	'con_set_cursor_pos',0
aCon_gets		db	'con_gets',0
aCon_printf		db	'con_printf',0


buffer			rb	1024