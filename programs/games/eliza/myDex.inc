
; длина строки
macro strlen string
{
  local .bcl,.ebcl
	mov esi,string
	mov ecx,0
.bcl:
	cmp byte [esi+ecx],0
	je .ebcl
	inc ecx
	jmp .bcl
.ebcl:

}


Clstext			dd	_Clstext
PrintString		dd	_PrintString
WaitForKeyPress		dd	_WaitForKeyPress
SetCursorPos		dd	_SetCursorPos
TextColor		dd	_TextColor
GetUserInput		dd	_GetUserInput
UpperCase		dd	_UpperCase
PrintChar		dd	_PrintChar
PrintCharCursor		dd	_PrintCharCursor


_Clstext:
	call	[con_cls]
	ret

_PrintString:
	pusha
	push	esi
	call	[con_write_asciiz]
	popa
	ret

_WaitForKeyPress:
	pusha
	call	[con_getch]
	popa
	ret

_SetCursorPos:
	pusha
	mov	ebx, eax
	and	ebx, 0xff
	mov	ecx, eax
	and	ecx, 0xff00
	shr	ecx, 8
	push	ecx
	push	ebx
	call	[con_set_cursor_pos]
	popa
	ret

_TextColor:
	ret

_GetUserInput:
	pusha
	push	new_line
	call	[con_write_asciiz]
	push	256
	push	buffer
	call	[con_gets]
	popa
	mov	edi, buffer
	strlen	edi
	ret

_UpperCase: ; скопировано из исходника DexOS
	pushad
	push  es
;	mov   ax,sys_data ; <- лишняя деталь :)
;	mov   es,ax    
UcaseNextChar:
	mov   al,byte[es:edi]
	cmp   al,0
	je    UcaseDone
	cmp   al,0x61
	jb    DontUcaseChar
	cmp   al,0x7a
	ja    DontUcaseChar
	sub   al,0x20
	mov   byte[es:edi],al  
DontUcaseChar:
	inc   edi
	jmp   UcaseNextChar
UcaseDone:
	pop   es
	popad
	ret

_PrintChar:
	pusha
	and	eax, 0xff
	push	eax
	push	char_spec
	call	[con_printf]
	add     esp, 8
	popa
	ret

_PrintCharCursor:
	pusha
	and	eax, 0xff
	push	eax
	push	char_spec
	call	[con_printf]
	add     esp, 8
	popa
	ret



char_spec	db '%c',0