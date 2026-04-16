
use32
org	0
db	'MENUET01'
dd	1
dd	_start
dd	_end
dd	_memory
dd	_stack
dd	_param
dd	0

;=========================================

_start:

mov	ah, byte [_param]
test	ah, ah
jz      _exit

call	console_lib_init

push    cobj_caption
push    -1
push    -1
push    -1
push    -1
call    [con_init]

mov     eax, 68
mov     ebx, 19
mov     ecx, _param
int     0x40
test    eax, eax
jz      _exit_and_close_console

mov	edx, eax


_main_loop:

cmp	dword [edx], 0
je	_exit_and_close_console

push	dword [edx]
push	specification
call	[con_printf]
add     esp, 8

add	edx, 8

jmp	_main_loop

_exit_and_close_console:

push	0
call	[con_exit]

_exit:
mov	eax, -1
int	0x40


;=========================================

console_lib_init:

; load DLL
	mov     eax, 68
	mov     ebx, 19
	mov     ecx, console_lib_name
	int     0x40
	test    eax, eax
	jz      _exit

; initialize import
	mov	edx, eax
	mov     esi, console_lib_import
import_loop:
	lodsd
	test    eax, eax
	jz      import_done
	push    edx
import_find:
	mov     ebx, [edx]
	test    ebx, ebx
	jz      _exit;import_not_found
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

;=========================================

console_lib_name	db	'/sys/lib/console.obj',0

align 4
console_lib_import:
dll_start		dd      aStart
con_init		dd      aConInit
con_exit		dd      aConExit
con_printf		dd	aCon_printf
			dd      0

aStart			db	'START',0
aConInit		db	'con_init',0
aConExit		db	'con_exit',0
aCon_printf		db	'con_printf',0


;=========================================

cobj_caption		db	'cObj v0.2 by Albom',0
specification		db	'%s',13,10,0

;=========================================

_param:
db	0
rb 256

_end:

align 32
rb 2048
_stack:
_memory:

;=========================================