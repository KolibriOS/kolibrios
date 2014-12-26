
format MS COFF

public Start
public ___path
public _PARAM
public ___main

extrn Memory
extrn hEnd

extrn _main

section ".text" code
	db "MENUET01"
	dd 1, Start, hEnd, Memory, hStack, _PARAM, ___path

Start:

; инициализация кучи
mov	eax, 68
mov	ebx, 11
int	0x40

; вызов главной процедуры
call _main

; завершение работы программы
mov	eax, -1
int	0x40

___main:
ret

section ".bss"

_PARAM:
rb	256

___path:
rb	256

rb	8*1024
hStack:
