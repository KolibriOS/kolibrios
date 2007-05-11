;-----------------------------------------------------------------------------
; Copyright (c) 2007, SPraid
;-----------------------------------------------------------------------------
format MS COFF

public EXPORTS

include '../proc32.inc'
include 'proc.inc'

purge section
section '.flat' code readable align 16

mem.alloc   dd ?
mem.free    dd ?
mem.realloc dd ?
dll.load    dd ?

block_count: dd 0
sect_num: dd 0
data_adr: dd 0
data_adr_last dd 0

filei_len	EQU 0x4000
filei		   dd ?;0x4000   ; ссылки на данные - типа там храниться будит... вначале там - скока нада выделить
sec_i_len	EQU 0x4000
sec_i		   dd ?;0x4000
dat_i_len	EQU 0x4000
dat_i		   dd ?;0x4000


;-----------------------------------------------------------------------------
proc lib_init ;////////// Инцилизация библиотеки (автоматический вызов)///////
;-----------------------------------------------------------------------------
	mov	[mem.alloc],eax
	mov	[mem.free],ebx
	mov	[mem.realloc],ecx
	mov	[dll.load],edx
	xor eax,eax
	inc eax
	ret
endp


; структура блока
	block_adr EQU 0
	block_name EQU block_adr + 4
	block_i_count EQU block_name + 30
	block_len EQU block_i_count + 4
; структура данных
	data_name EQU 0
	data_prev EQU data_name+200
	data_next EQU data_prev+4
	data_len  EQU data_next+4




; взять следущий пару занчения ключа
proc ini.get_par stdcall, sect:dword, param:dword, val:dword, num:dword

  pushad
  mov edx,[block_count]
  mov edi,[sect]
  mov esi,[sec_i]
  add esi,block_name
@@: call strcmp
  cmp eax,-1
  jne .sec_found
  dec edx
  or edx,edx
  jz .sec_not_found
  add esi,block_len
  jmp @b
 .sec_found:

  mov eax,esi
  sub eax,block_name
  add eax, block_i_count
  mov eax,[eax]
  ; eax - count
  mov ebx,[num] 		 ; test max count
  ; ebx - num
  cmp eax,ebx
  jle .param_not_found

  sub esi,4
  mov esi,[esi]
  ; esi - first adr
  mov eax,ebx	 ; eax - num
@@:
  or eax,eax
  je .param_found
  dec eax
  add esi,data_next
  mov esi,[esi]
  jmp @b

 .param_found:
  mov ebx,esi
  mov eax,[param]
  mov cl,'='
  call copy_s
  add ebx,eax
  add ebx,1
  mov eax,[val]
  mov cl,0
  call copy_s
  jmp .ok

 .param_not_found:
 .sec_not_found:
; set_default_.... 0
 mov eax,[param]
 mov [eax],byte 0
 mov eax,[val]
 mov [eax],byte 0
 .ok:
  popad
  ret


endp
;------------------------------------------------------
proc ini.get_str stdcall, sect:dword, param:dword, buff:dword, default:dword
  ; sect - секция
  ; param - параметр
  ; buff - буфер
  ; default - если отсутствует
 pushad
  mov edx,[block_count]
  mov edi,[sect]
  mov esi,[sec_i]
  add esi,block_name
@@: call strcmp
  cmp eax,-1
  jne .sec_found
  dec edx
  or edx,edx
  jz .sec_not_found
  add esi,block_len
  jmp @b
 .sec_found:

  mov eax,esi
  sub eax,block_name
  add eax, block_i_count
  mov eax,[eax]
  ; eax - count
  sub esi,4
  mov esi,[esi]
  ; esi - first adr
  mov edi,[param]
  push eax

@@:
  mov cl,'='
  mov eax,text
  mov ebx,esi
  call copy_s
  mov edx,eax
  push esi
  mov esi,text
  call strcmp
  pop esi
  cmp eax,-1
  jne .param_found
  pop eax
  dec eax
  or eax,eax
  jz .sec_not_found
  push eax
  add esi,data_next
  mov esi,[esi]
  jmp @b
 .param_found:
  mov ebx,esi
  add ebx,edx
  add ebx,1
  pop eax
  mov eax,[buff]
  mov cl,0
  call copy_s
  jmp .ok
 .param_not_found:
 .sec_not_found:
; set_default_text
  mov eax,[buff]
  mov ebx,[default]
  mov cl,0
  call copy_s
 .ok:
  popad
  ret
endp
;--------------------------------------------------------------------------------














;-----------------------------------------------------------------------------
proc ini.load stdcall filename ;/(загрузка ини файла)/////////////////////////
locals
 reads: dd 0,0,0
 fsize	dd 0
 addr	dd filei
 nulls	db 0
 fname	dd 0
endl
	mov [reads],dword 0
	mov [reads+4],dword 0
	mov [reads+8],dword 0
	mov [nulls],byte 0

	stdcall get_filesize, [filename]
	mov [fsize],eax

	inc eax

	mov ebx,eax

	invoke	mem.alloc,eax
	mov [filei],eax

	mov [addr],eax

	add eax,ebx
	mov [eax], byte 0		;null string

	invoke	mem.alloc,sec_i_len
	mov [sec_i],eax

	invoke	mem.alloc,dat_i_len
	mov [dat_i],eax
	mov [data_adr],eax

	mov eax,[filename]
	mov [fname],eax

	mov eax,70
	mov ebx , reads-ebp
	add ebx,ebp
	int 0x40

	stdcall readlnf

    xor eax,eax
	ret


endp


proc readlnf
 pushad
 mov [.pos],dword 0
.char:
 mov eax,[.pos]
 inc eax
 mov [.pos],eax

 sub eax,1
 add eax,[filei]
 mov al,[eax]				; символ от позиции
							; тип по первому символу
 cmp al,' '
 je .char
 cmp al,9
 je .char
 cmp al,13
 je .char
 cmp al,10
 je .char
 cmp al,'['
 je .sect
 cmp al,0
 je .exit
;------------------------------------------------------------------------------------------
	; чтение параметров
 mov eax,[data_adr]
 mov ebx,[.pos]
 add ebx,[filei]
 dec ebx
 mov cl,0xD
 call copy_s
 xchg ebx,eax
 mov eax,[.pos]
 add eax,ebx
 mov [.pos],eax


 mov ebx,[data_adr]
 mov ecx,[data_adr_last]
 mov [ebx+data_prev],ecx
 ;add ebx,data_next
 mov [ecx],ebx
 add ebx,data_next
 mov [data_adr_last],ebx

 add ebx,data_len-data_next
 mov [data_adr],ebx
 mov eax,[sect_num]
 imul eax,eax,block_len
 add eax,[sec_i]
 add eax, block_i_count
 mov ebx,[eax]
 inc ebx
 mov [eax],ebx
 jmp .test_next
;-----------------------------------------------------------------------------------------------------------------
.sect:						; чтение секции
 mov eax,[block_count]
 imul eax,eax,block_len
							; копируем имя (до ])
 add eax,[sec_i]
 add eax,block_name			; кужа ложить имя
 mov ebx,[.pos]
 add ebx,[filei]				; откуда
 mov cl,']'
 call copy_s				; ложим


 mov ebx,[.pos]
 add ebx,eax
 ;sub ebx,filei
 add ebx,1
 mov [.pos],ebx

 mov eax,[block_count]		; прибавить количество блоков
 mov [sect_num],eax
 mov ebx,eax
 imul eax,eax,block_len 	; это будит адрес пердыдущего для первого элемента
 add eax,[sec_i]
 mov [data_adr_last],eax
 inc ebx
 mov [block_count],ebx


.test_next:
 cmp [.pos] ,dword 97
 jb .char

.exit:

 popad
 ret

 .pos dd 0

endp


   text db 255 dup(?)

align 16
EXPORTS:

export	\
	lib_init   ,'lib_init',\
	ini.load   ,'ini.load',\
	ini.get_str,'ini.get_str',\
	ini.get_par,'ini.get_par'