
format MS COFF

public Start

extrn Memory
extrn hEnd

extrn _kol_main

section ".text" code
	db "MENUET01"
	dd 1, Start, hEnd, Memory, hStack, 0, 0

Start:

; инициализация кучи
mov	eax, 68
mov	ebx, 11
int	0x40

; вызов главной процедуры
mov	eax, _kol_main
call	eax

; завершение работы программы
mov	eax, -1
int	0x40

section ".bss"

rb 4096
hStack:
