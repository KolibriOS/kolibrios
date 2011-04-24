
; Keyboard indicators v0.1
; by Albom

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


_start:

call _key_set
call _wnd_draw

_event_wait:

mov	eax, 10
int	0x40

cmp	eax, 1
jne @f
call	_wnd_draw

@@:
cmp	eax, 2
jne @f
call   _key_check

@@:
cmp	eax, 3
jne @f
call	_btn_check

@@:


jmp  _event_wait

_key_set:
mov eax, 66
mov edx, 0
mov ebx, 4
mov cl, 69
int 0x40

mov eax, 66
mov edx, 0
mov ebx, 4
mov cl, 58
int 0x40

mov eax, 66
mov edx, 0
mov ebx, 4
mov cl, 70
int 0x40

ret


_wnd_draw:
pusha

mov	eax, 12
mov	ebx, 1
int	0x40

xor	eax, eax
mov	ebx, 10*65536+100
mov	ecx, 10*65536+30
mov	edx, 0x34ffffff
mov	edi, _ind
int	0x40

call	_indicators_check

mov	eax, 12
mov	ebx, 2
int	0x40

popa
ret

_key_check:
pusha
mov	eax, 2
int	0x40

call	_indicators_check

popa
ret

_btn_check:
pusha
mov	eax, 17
int	0x40
cmp	ah, 1
jne	@f
mov eax, -1
int 0x40
@@:
popa
ret


_indicators_check:
pusha
	mov eax, 66
	mov ebx, 3
	int 40h

test_ins:
	test eax, 0x80
	jz @f
	mov [_ind], '*'
	jmp test_caps
@@:
	mov [_ind], 'o'

test_caps:
	test eax, 0x40
	jz @f
	mov [_ind+1], '*'
	jmp test_scroll
@@:
	mov [_ind+1], 'o'

test_scroll:
	test eax, 0x100
	jz @f
	mov [_ind+2], '*'
	jmp test_ok
@@:
	mov [_ind+2], 'o'

test_ok:
	mov eax, 71
	mov ebx, 1
	mov ecx, _ind
	int 0x40
popa
ret

_ind db 'ooo', 0

_param:
rb 256

_end:

align 32
rb 2048
_stack:
_memory:
