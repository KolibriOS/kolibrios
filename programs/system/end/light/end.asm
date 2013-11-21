;
; END
; KolibriOS Team 2005-2013
;
; <diamond> note that 'mov al,xx' is shorter than 'mov eax,xx'
;           and if we know that high 24 bits of eax are zero, we can use 1st form
;           the same about ebx,ecx,edx
fade equ 0

use32        ; включить 32-битный режим ассемблера
org 0x0      ; адресация с нуля

db 'MENUET01'    ; 8-байтный идентификатор MenuetOS
dd 0x01          ; версия заголовка (всегда 1)
dd START         ; адрес первой команды
dd IM_END        ; размер программы
dd I_END         ; количество памяти
dd stacktop      ; адрес вершины стека
dd 0x0           ; адрес буфера для параметров
dd cur_dir_path

include 'lang.inc'
include '../../../macros.inc'
include '../../../proc32.inc'
include '../../../dll.inc'
include '../../../develop/libraries/box_lib/load_lib.mac'
include '../../../develop/libraries/box_lib/trunk/box_lib.mac'

        @use_library

align 4
START:

load_libraries l_libs_start,end_l_libs
        inc     eax
        test    eax,eax
        jz      close

push    dword check1
call    [init_checkbox2]

stdcall dll.Init,[init_lib]

invoke  ini_get_int,ini_file,asettings,aautosave,0
        mov   [autosave],eax
        dec   eax
        jnz   @f
        bts   dword [check1.flags],1
@@:
        mcall   40,0x80000027
redraw:
    call draw_window
still:
    mov  al,10
    mcall                                    ;wait here for event
    dec  eax
    jz   redraw
    dec  eax
    jz   key
    dec  eax
    jz   button

    push dword check1
    call [check_box_mouse2]
    bt   dword [check1.flags],1
    jnc  @f
    mov  [autosave],1
    jmp  still
@@:
    mov  [autosave],0
    jmp  still

key:
    mov  al,2
    mcall                                    ;eax=2 - get key code
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
    mcall 17                                 ;eax=17 - get pressed button id
    xchg al,ah
    dec  eax
    jz   close
    dec  eax
    jz   restart_kernel
    dec  eax
    jz   restart
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
else
    mov   al,4
    mcall   ,<50,110> ,0x800000cc,label7        ;eax=4 - write text
end if

    invoke  ini_set_int,ini_file,asettings,aautosave,[autosave]
    cmp  [autosave],1
    jne   no_save
    mcall 70,rdsave
    test  eax,eax
    js    no_save
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
    je    no_save
    jmp   @b
no_save:
    pop  ecx
  mcall 18,9

checkbox:
    btc   dword [check1.flags],1
    jc    .1
    mov   [autosave],1
    jmp   .draw
.1:
    mov   [autosave],0
.draw:
    push  dword check1
    call  [check_box_draw2]
    jmp    still

draw_window:
    mov   al,12
    mcall   ,1

    mov   al,14
    mcall                                    ;eax=14 - get screen max x & max y
    movzx ecx,ax
    shr   eax,17
    shl   eax,16
    lea   ebx,[eax-110 shl 16+222]
    shr   ecx,1
    shl   ecx,16
    lea   ecx,[ecx-70 shl 16+122]

    xor   eax,eax
    mcall  , , ,[color1],0x01000000        ;define and draw window

    mov   al,13
    mcall   ,<0,223> ,<0,123>
    mcall   ,<1,221>,<1,121>,[color2]
    mcall   ,<2,220>,<2,120>, [color3]

    mov   al,8
    mcall   ,<16,90> ,<20,27>,4,[color4]     ;eax=8 - draw buttons
    mcall   ,<113,90>,       ,2,[color5]
    mcall   ,        ,<54,27>,1,[color6]
    mcall   ,<16,90> ,       ,3,[color7]

    mov   al,4
    mcall   ,<27,24> ,[color8],label2        ;eax=4 - write text
    mcall   ,<23,58> ,        ,label3
    mcall   ,<47,37> ,        ,label5
    mcall   ,<41,71> ,        ,label6

    push  dword check1
    call  [check_box_draw2]

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
I_END:  ; метка конца программы