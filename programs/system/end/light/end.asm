
; END
; KolibriOS Team 2005-2016

fade equ 0

use32	     ; включить 32-битный режим ассемблера
org 0x0      ; адресация с нуля

db 'MENUET01'	 ; 8-байтный идентификатор MenuetOS
dd 0x01 	 ; версия заголовка (всегда 1)
dd START	 ; адрес первой команды
dd IM_END	 ; размер программы
dd I_END	 ; количество памяти
dd stacktop	 ; адрес вершины стека
dd 0x0		 ; адрес буфера для параметров
dd cur_dir_path

include 'lang.inc'
include '../../../macros.inc'
include '../../../proc32.inc'
include '../../../dll.inc'
include '../../../develop/libraries/box_lib/load_lib.mac'
include '../../../develop/libraries/box_lib/trunk/box_lib.mac'
include '../../../gui_patterns.inc'
include "../../../string.inc"

WIN_W equ 440
WIN_H equ 200
BOT_PANEL_H equ 70

CANCEL_BUTTON_ID equ 1+BT_HIDE
HOME_BUTTON_ID equ 2
REBOOT_BUTTON_ID equ 3
POWEROFF_BUTTON_ID equ 4

@use_library

	
align 4
START:


load_libraries l_libs_start,end_l_libs
	inc	eax
	test	eax,eax
	jz	close

push	dword check1
call	[init_checkbox2]

stdcall dll.Init,[init_lib]

invoke	ini_get_int,ini_file,asettings,aautosave,0
	mov   [autosave],eax
	dec   eax
	jnz   @f
	bts   dword [check1.flags],1
@@:
	mcall	40,0x80000027
redraw:
    call draw_window
still:
    mov  al,10
    mcall				     ;wait here for event
    dec  eax
    jz	 redraw
    dec  eax
    jz	 key
    dec  eax
    jz	 button

    push dword check1
    call [check_box_mouse2]
    bt	 dword [check1.flags],1
    jnc  @f
    mov  [autosave],1
    jmp  still
@@:
    mov  [autosave],0
    jmp  still

key:
    mov  al,2
    mcall				     ;eax=2 - get key code
    mov  al,ah
     cmp  al,13
     je   restart
     cmp  al,19
     je   checkbox
     cmp  al,180
     je   restart_kernel
     cmp  al,181
     je   power_off
     cmp  al,27
     jne   still

close:
    mcall -1

button:
    mcall 17				     ;eax=17 - get pressed button id
    xchg al,ah
    dec  eax
    jz	 close
    dec  eax
    jz	 restart_kernel
    dec  eax
    jz	 restart
    dec  eax
    jnz  checkbox

power_off:
    push 2
    jmp  mcall_and_close

restart:
    push 3
    jmp  mcall_and_close

restart_kernel:
    push 4

mcall_and_close:
if fade=1
 ; === FADE IN ===
    mov eax, color1
  @@:
    mov ebx, [eax + 32]
    mov [eax], ebx
    add eax, 4
    cmp eax, color21
    jne @b

    call    draw_window
end if

    invoke  ini_set_int,ini_file,asettings,aautosave,[autosave]
    cmp  [autosave],1
    jne   no_save

if fade=0
    mov   al,4
    mcall   ,<50,120> ,0x800000cc,label7	;eax=4 - write text
end if

    mcall 70,rdsave
    test  eax,eax
    js	  no_save
    mov   ecx,eax
    mcall 18,21
    mov   ecx,eax
@@:
    push ecx
    mcall 23,100
    dec   eax
    jnz   no_red
    call draw_window
no_red:
    pop   ecx
    mcall 9,proc_info
    cmp   [proc_info+50],9
    je	  no_save
    jmp   @b
no_save:
    pop  ecx
    mcall 18,9
    mcall -1
ret

checkbox:
    btc   dword [check1.flags],1
    jc	  .1
    mov   [autosave],1
    jmp   .draw
.1:
    mov   [autosave],0
.draw:
    push  dword check1
    call  [check_box_draw2]
    jmp    still

draw_window:
    mcall 12,1
	
    mov   al,14
    mcall				     ;eax=14 - get screen max x & max y
    movzx ecx,ax
    shr   eax,17
    shl   eax,16
    lea   ebx,[eax-(WIN_W/2) shl 16+WIN_W-1]
    shr   ecx,1
    shl   ecx,16
    lea   ecx,[ecx-(WIN_H/2) shl 16+WIN_H-1]

    xor   eax,eax
	mov edx, 0x41000000
	mcall ;define and draw window
	
	DrawWideRectangle 0, 0, WIN_W, WIN_H, 2, 0xA3A7AA
	DrawBar 2, 2, WIN_W-4, WIN_H-BOT_PANEL_H-2, 0x202020
	DrawBar 2, WIN_H-BOT_PANEL_H-2, WIN_W-4, BOT_PANEL_H, 0x4B4B4B
	WriteText 30, 27, 10010001b, 0xFFFfff, TEXT_TITLE
	WriteText 55, 70, 10010000b, 0xFFFfff, TEXT_RDSAVE1
	WriteText 55, 86, 10010000b, 0xFFFfff, TEXT_RDSAVE2
	
	DefineButton  WIN_W-33, 2, 32, 20, CANCEL_BUTTON_ID, 0
	WriteText  WIN_W-23, 5, 10000001b, 0xFFFfff, TEXT_CANCEL

    push  dword check1
    call  [check_box_draw2]

macro EndButton  x, bgcol, id, but_text, hotkey_text
{
	buty equ WIN_H-60
	butw equ 116
	buth equ 43
	DrawWideRectangle x-3, buty-3, butw+6, buth+6, 3, 0x202020
	DefineButton x, buty, butw-1, buth-1, id, bgcol
	; WriteTextBold -strlen(but_text)*8 + butw / 2 + x, buty+8,  10010000b, 0xFFFfff, but_text
	; WriteText     -strlen(but_text)*6 + butw / 2 + x, buty+26, 10000000b, 0xFFFfff, hotkey_text
	stdcall string.length, but_text
	mov  ebx,eax
	imul ebx,4
	neg  ebx
	add  ebx,butw / 2 + x
	shl  ebx,16
	add  ebx,buty+8
	mcall 4, , 10010000b shl 24 + 0xFFFfff, but_text
	add ebx,1 shl 16
	mcall
	stdcall string.length, hotkey_text
	mov  ebx,eax
	imul ebx,3
	neg  ebx
	add  ebx,butw / 2 + x
	shl  ebx,16
	add  ebx,buty+26
	mcall 4, , 10000000b shl 24 + 0xFFFfff, hotkey_text
}

	EndButton  20, 0x4E91C5, HOME_BUTTON_ID,     TEXT_KERNEL, TEXT_HOME
	EndButton 160, 0x55C891, REBOOT_BUTTON_ID,   TEXT_REBOOT, TEXT_ENTER
	EndButton 300, 0xC75C54, POWEROFF_BUTTON_ID, TEXT_OFF,    TEXT_END

    mov   al,12
    mcall   ,2
    ret
;---------------------------------------------------------------------
;data
include 'data.inc'
;---------------------------------------------------------------------
IM_END:
;---------------------------------------------------------------------
align 4

proc_info  rb 1024

autosave rd 1
;---------------------------------------------------------------------
cur_dir_path:
	rb 4096
;---------------------------------------------------------------------
library_path:
	rb 4096
;---------------------------------------------------------------------
align 32
	rb 4096
stacktop:
I_END:	; метка конца программы