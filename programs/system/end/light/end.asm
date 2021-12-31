; END
; KolibriOS Team 2005-2021

use32         ; включить 32-битный режим ассемблера
org 0x0       ; адресация с нуля

db 'MENUET01' ; 8-байтный идентификатор MenuetOS
dd 0x01       ; версия заголовка (всегда 1)
dd START      ; адрес первой команды
dd IM_END     ; размер программы
dd I_END      ; количество памяти
dd stacktop   ; адрес вершины стека
dd 0x0        ; адрес буфера для параметров
dd 0x0

include 'lang.inc'
include '../../../macros.inc'
include '../../../proc32.inc'
include '../../../dll.inc'
include '../../../KOSfuncs.inc'
include '../../../gui_patterns.inc'
include '../../../string.inc'

START:
    mcall SF_SYS_MISC, SSF_HEAP_INIT 
    mcall SF_SYS_MISC, SSF_MEM_OPEN, checkbox_sharedname
    mov [checkbox_img], eax

    stdcall dll.Load,importLib
    or      eax, eax
    jnz     redraw

    invoke ini_get_int,ini_file,asettings,aautosave,0
    mov   [autosave],eax
redraw:
    call  draw_window
still:
    mcall SF_WAIT_EVENT        ;wait here for event
    dec   eax
    jz    redraw
    dec   eax
    jz    key
    dec   eax
    jz    button
    jmp   still

key:
    mcall SF_GET_KEY        ;get key code
    mov   al,ah
     cmp  al,13
     je   restart
     cmp  al,19
     je   checkbox
     cmp  al,180
     je   restart_kernel
     cmp  al,181
     je   power_off
     cmp  al,27
     jne  still

close:
    mcall SF_TERMINATE_PROCESS

button:
    mcall SF_GET_BUTTON                     ;get pressed button id
    xchg  al,ah
    dec   eax
    jz    close
    dec   eax
    jz    restart_kernel
    dec   eax
    jz    restart
    dec   eax
    jnz   checkbox

power_off:
    push  2
    jmp   mcall_and_close

restart:
    push  3
    jmp   mcall_and_close

restart_kernel:
    push  4

mcall_and_close:
    invoke ini_set_int,ini_file,asettings,aautosave,[autosave]
    cmp   [autosave],1
    jne   no_save

    mcall SF_DRAW_TEXT,<55,108>,0x90FF990A,TEXT_SAVING

    mcall SF_FILE,rdsave
    test  eax,eax
    js    no_save
    mov   ecx,eax
    mcall SF_SYSTEM,SSF_GET_THREAD_SLOT
    mov   ecx,eax
@@:
    push ecx
    mcall SF_WAIT_EVENT_TIMEOUT,100
    dec   eax
    jnz   no_red
    call  draw_window
no_red:
    pop   ecx
    mcall SF_THREAD_INFO,proc_info
    cmp   [proc_info+50],9
    je    no_save
    jmp   @b
no_save:
    pop   ecx
    mcall SF_SYSTEM,SSF_SHUTDOWN
    mcall SF_TERMINATE_PROCESS
ret

checkbox:
    cmp   [autosave],1
    je    .1
    mov   [autosave],1
    jmp   .draw
.1:
    mov   [autosave],0
.draw:
    call  draw_checkbox_flag
    jmp   still

draw_window:
    mcall SF_REDRAW,SSF_BEGIN_DRAW
    
    mcall SF_GET_SCREEN_SIZE
    movzx ecx,ax
    shr   eax,17
    shl   eax,16
    lea   ebx,[eax-(WIN_W/2) shl 16+WIN_W-1]
    shr   ecx,1
    shl   ecx,16
    lea   ecx,[ecx-(WIN_H/2) shl 16+WIN_H-1]

    xor   eax,eax
    mov   edx, 0x41000000
    mcall ;define and draw window
    
    DrawWideRectangle 0, 0, WIN_W, WIN_H, 2, 0xA3A7AA
    mcall SF_DRAW_RECT, <2,WIN_W-4>, <2,WIN_H-BOT_PANEL_H-2>, 0x202020
    mcall SF_DRAW_RECT, <2,WIN_W-4>, <WIN_H-BOT_PANEL_H-2,BOT_PANEL_H>, 0x4B4B4B
    
    mcall SF_DRAW_TEXT, <30,21>, 0x91FFFfff, TEXT_WTITLE
    mcall SF_DRAW_TEXT, <55,70>, 0x90FFFfff, TEXT_RDSAVE1
    mcall SF_DRAW_TEXT, <55,86>, 0x90FFFfff, TEXT_RDSAVE2
    mcall SF_DRAW_TEXT, <WIN_W-23,5>, 0x81FFFfff, TEXT_CANCEL
    
    mcall SF_DEFINE_BUTTON, <WIN_W-35,32>, <2,22>, CANCEL_BUTTON_ID
    mcall SF_DEFINE_BUTTON, <32,14>, <70,14>, CHECKBOX_BUTTON_ID
    mcall SF_DEFINE_BUTTON, <47,WIN_W-47>, <68,34>, CHECKBOX_BUTTON_ID+BT_NOFRAME
    DrawRectangle3D 32, 70, 14, 14, 0x606060, 0xAFAFAF
    call draw_checkbox_flag

    stdcall EndButton,  20, 0x4E91C5, HOME_BUTTON_ID,     TEXT_KERNEL, TEXT_HOME
    stdcall EndButton, 160, 0x41C166, REBOOT_BUTTON_ID,   TEXT_REBOOT, TEXT_ENTER
    stdcall EndButton, 300, 0xC75C54, POWEROFF_BUTTON_ID, TEXT_OFF,    TEXT_END

    mcall SF_REDRAW,SSF_END_DRAW
    ret

proc EndButton x, bgcol, id, but_text, hotkey_text
    BUTY = WIN_H-60
    BUTW = 116
    mov ebx,[x]
    sub ebx,3
    mcall SF_DRAW_RECT, <ebx,BUTW+7>, <BUTY-3,43+6>, 0x202020
    mcall SF_DEFINE_BUTTON, <[x],BUTW>, <BUTY,43-1>, [id], [bgcol]
    ; -strlen(but_text)*8 + BUTW / 2 + x, BUTY+8
    stdcall string.length, [but_text]
    neg  eax
    lea  ebx,[eax*4+BUTW/2]
    add  ebx,[x]
    mcall SF_DRAW_TEXT, <ebx,BUTY+8>, 0x90FFFfff, [but_text]
    add  ebx,1 shl 16
    mcall
    stdcall string.length, [hotkey_text]
    neg  eax
    lea  ebx,[eax*3+BUTW/2]
    add  ebx,[x]
    shl  ebx,16
    add  ebx,BUTY+26
    mcall SF_DRAW_TEXT, , 0x80FFFfff, [hotkey_text]
    ret
endp

draw_checkbox_flag:
    cmp [autosave],0
    je  .flag_unset
    cmp [checkbox_img],0
    je  .flag_set_but_no_checkbox_img
.flag_set:
    mcall SF_PUT_IMAGE, [checkbox_img], <13,13>, <33,71>
    ret
.flag_set_but_no_checkbox_img:
    mcall SF_DRAW_RECT, <33,13>, <71,13>, 0xffffff
    mcall SF_DRAW_RECT, <34,11>, <72,11>, 0x58C33C
    ret
.flag_unset:
    mcall SF_DRAW_RECT, <33,13>, <71,13>, 0xFFFfff
    ret
;---------------------------------------------------------------------
include 'data.inc'
;---------------------------------------------------------------------
IM_END:
;---------------------------------------------------------------------
align 4
proc_info  rb 1024
autosave rd 1
;---------------------------------------------------------------------
library_path rb 4096
;---------------------------------------------------------------------
align 32
    rb 4096
stacktop:
I_END:    ; метка конца программы