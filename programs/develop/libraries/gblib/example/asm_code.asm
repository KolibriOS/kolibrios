
format MS COFF

public Start
public _hStack
public _KOL_PATH
public _KOL_PARAM

extrn Memory
extrn hEnd

extrn _kol_main

section ".text" code
	db "MENUET01"
	dd 1, Start, hEnd, Memory, _hStack, _KOL_PARAM, _KOL_PATH

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

_KOL_PARAM rb 256
_KOL_PATH rb 256

rb 16*1024
_hStack:
