; macros to write stuff to menuet's debug message board.
; the macros don't change any registers, not even flags.
; they take only effect if DEBUG is defined.
;
; Copyright (c) 2003 Thomas Mathys
; killer@vantage.ch
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
;


if DEBUG eq

macro DBG_BOARD_PRINTNEWLINE {
}

macro DBG_BOARD_PRINTCHAR c1 {
}

macro DBG_BOARD_PRINTDWORD w1 {
}

macro DBG_BOARD_PRINTSTRINGLITERAL p1 {
}

macro DBG_BOARD_PRINTSTRING s1 {
}

else

;********************************************************************
;	print newline
;	no input
;********************************************************************
macro DBG_BOARD_PRINTNEWLINE {
	call	dbg_board_printnewline
}


;********************************************************************
;	print a character
;
;	examples	:	DBG_BOARD_PRINTCHAR '?'
;				DBG_BOARD_PRINTCHAR 65
;				DBG_BOARD_PRINTCHAR cl
;				DBG_BOARD_PRINTCHAR [esi]
;				DBG_BOARD_PRINTCHAR [somevariable]
;********************************************************************
macro DBG_BOARD_PRINTCHAR c1 {
	push	ecx
	mov	cl,byte c1
	call	dbg_board_printchar
	pop	ecx
}



;********************************************************************
;	print a dword (in hex)
;
;	examples:	DBG_BOARD_PRINTDWORD esp
;			DBG_BOARD_PRINTDWORD 0xdeadbeef
;			DBG_BOARD_PRINTDWORD [somevariable]
;********************************************************************
macro DBG_BOARD_PRINTDWORD w1 {
	push	dword w1
	call	dbg_board_printdword
}


;********************************************************************
;	print a string literal
;	a terminating zero is automagically appended to the string.
;
;	examples	DBG_BOARD_PRINTSTRINGLITERAL "foo",0
;			DBG_BOARD_PRINTSTRINGLITERAL "bar",10,13,0
;********************************************************************
macro DBG_BOARD_PRINTSTRINGLITERAL p1 {
	local .foo
	jmp	@f
.foo	db	p1, 0		; terminate string, just to be sure
@@:
	push	dword .foo
	call	dbg_board_printstring
}


;********************************************************************
;	print a string (asciiz)
;
;	examples	DBG_BOARD_PRINTSTRING addressofstring
;			DBG_BOARD_PRINTSTRING esi
;			DBG_BOARD_PRINTSTRING [ebx]
;********************************************************************
macro DBG_BOARD_PRINTSTRING s1 {
	push	dword s1
	call	dbg_board_printstring
}


; no input
dbg_board_printnewline:
	pushad
	pushfd
	mcall SF_BOARD,SSF_DEBUG_WRITE,10
	mcall ,,13
	popfd
	popad
	ret
	
	
; input : cl = character to print
dbg_board_printchar:
	pushad
	pushfd
	and	ecx,0xff
	mcall SF_BOARD,SSF_DEBUG_WRITE
	popfd
	popad
	ret

	
; input : dword to print on stack 
dbg_board_printdword:
	enter	0,0
	pushad
	pushfd
	; print 0x prefix
	mcall SF_BOARD,SSF_DEBUG_WRITE,'0'
	mcall ,,'x'
	mov	edx,[ebp + 8]		; get dword to print
	mov	esi,8			; iterate through all nibbles
.loop:
	mov	ecx,edx			; display hex digit
	shr	ecx,28
	movzx	ecx,byte [dbg_board_printdword_digits + ecx]
	mcall
	shl	edx,4			; next nibble
	dec	esi
	jnz	.loop
	popfd
	popad
	leave
	ret	4
dbg_board_printdword_digits:
	db	'0','1','2','3','4','5','6','7'
	db	'8','9','a','b','c','d','e','f'

	
; input : address of string (asciiz) to print on stack
dbg_board_printstring:
	enter	0,0
	pushad
	pushfd
	cld
	mov	esi,[ebp + 8]		; esi -> string
	mov	ebx,1
.loop:
	lodsb				; get character
	or	al,al			; zero ?
	je	.done			; yeah -> get outta here
	movzx	ecx,al			; nope -> display character
	mcall SF_BOARD
	jmp	.loop
.done:
	popfd
	popad
	leave
	ret	4

end if

