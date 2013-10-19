;
; END
; KolibriOS Team 2005-2013
;
include "lang.inc"
include "..\..\..\macros.inc"
; <diamond> note that 'mov al,xx' is shorter than 'mov eax,xx'
;           and if we know that high 24 bits of eax are zero, we can use 1st form
;           the same about ebx,ecx,edx

meos_app_start
code
draw_window:
    mov   al,12
    mcall ,1

    mov  al,14
    mcall                                    ;eax=14 - get screen max x & max y
    movzx ecx,ax
    shr  eax,17
    shl  eax,16
    lea  ebx,[eax-110 shl 16+222]
    shr  ecx,1
    shl  ecx,16
    lea  ecx,[ecx-65 shl 16+137]

    xor eax,eax
    mcall  , , ,0x019098b0,0x01000000        ;define and draw window

    mov   al,13
    mcall   ,<0,223> ,<0,275>
    mcall   ,<1,221>,<1,136>,0xffffff
    mcall   ,<2,220>,<2,135>,0xe4dfe1
    mcall   ,<16,189>,<97,23>,0x9098b0

    mov    al,8
    mcall   ,<16,90> ,<20,27>,4,0x990022     ;eax=8 - draw buttons
    mcall   ,<113,90>,       ,2,0xaa7700
    mcall   ,        ,<54,27>,1,0x777777
    mcall   ,<16,90> ,       ,3,0x007700
    mcall   ,<17,186>,<98,20>,5,0xe4dfe1

    mov   al,4
    mcall  ,<28,105>,0x80000000,label4        ;eax=4 - write text
    mcall  ,<35,24> ,0x80ffffff,label2
    mcall  ,<34,58> ,          ,label3
    mcall  ,<47,37> ,          ,label5
    mcall  ,<43,71> ,          ,label6

    mov   al,12
    mcall   ,2

still:
    mov  al,10
    mcall                                    ;wait here for event
    dec  eax
    jz   draw_window
    dec  eax
    jnz  button

    mov  al,2
    mcall                                    ;eax=2 - get key code
    mov  al,ah
     cmp  al,13
     jz   restart
     cmp  al,19
     jz   run_rdsave
     cmp  al,27
     jz   close_1
     cmp  al,180
     jz   restart_kernel
     cmp  al,181
     jz   power_off
     jmp  still

button:
    mov  al,17
    mcall                                    ;eax=17 - get pressed button id
    xchg al,ah
    dec  eax
    jz   close_1
    dec  eax
    jz   restart_kernel
    dec  eax
    jz   restart
    dec  eax
    jnz   run_rdsave
;    dec  eax                                ; we have only one button left, this is close button
;    jnz  still

power_off:
    push 2
    jmp  mcall_and_close
restart:
    push 3
    jmp  mcall_and_close
restart_kernel:
    push 4
mcall_and_close:
    pop  ecx
    mov  al,18
    mcall   ,9

close_1:
    or  eax,-1
    mcall  

run_rdsave:
    mov  al,70
    mcall   ,rdsave
    jmp still

data
include 'data.inc'

udata

meos_app_end