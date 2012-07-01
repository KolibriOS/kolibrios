; LuhnA - Luhn algorithm
; by Albom

use32
 org 0
 db 'MENUET01'
 dd 1
 dd _start
 dd _end
 dd _memory
 dd _stack
 dd _param
 dd 0

include '../../macros.inc'


align 4
_start:

 mov al, [_param]
 test al, al
 jz exit

 call luhna
 xor edx, edx
 mov ecx, 10
 div ecx

 test edx, edx
 jz valid
 mov ebx, msg_not
 jmp print
valid:
 mov ebx, msg_valid

print:
 mov [notifyapp+2*4], ebx
 mcall 70, notifyapp


exit:
 mcall -1




align 4
luhna:
 xor edx, edx
 xor ecx, ecx
 xor eax, eax
 mov esi, _param
@@:
 lodsb
 test al, al
 jz .exit
 sub al, '0'
 mov dl, [odd+eax]
 add ecx, edx
 lodsb
 test al, al
 jz .exit
 sub al, '0'
 add ecx, eax
 jmp @b
.exit:
 mov eax, ecx
 ret

align 4
msg_not   db 'NOT '
msg_valid db 'VALID', 0

align 4
odd: db  0, 2, 4, 6, 8, 1, 3, 5, 7, 9

align 4
notifyapp:
	dd 7
	dd 0
	dd 0 ; адрес сообщения
	dd 0
	dd 0
	db '@notify', 0


_end:
align 4
 rb 256
_stack:
align 4
_param rb 256
_memory:
