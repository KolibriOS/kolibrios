;
; END
; KolibriOS Team 2005-2013
;
include "lang.inc"
include "..\..\..\macros.inc"

meos_app_start
code
draw_window:
    mcall 12,1

    mcall 14                                 ;eax=14 - get screen max x & max y
    movzx ecx,ax
    shr  eax,17
    shl  eax,16
    lea  ebx,[eax-110 shl 16+222]
    shr  ecx,1
    shl  ecx,16
    lea  ecx,[ecx-70 shl 16+137]

    mcall 0, , ,0x01ffffff                   ;define and draw window

    mcall 13,   223 ,   138 ,0x9098b0
    mcall   ,<1,221>,<1,136>,0xffffff
    mcall   ,<2,220>,<2,135>,0xe4dfe1
    mcall   ,<16,189>,<97,23>,0x9098b0

    mcall  8,<16,90> ,<20,27>,4,0x990022     ;eax=8 - draw buttons
    mcall   ,<113,90>,       ,2,0xaa7700
    mcall   ,        ,<54,27>,1,0x777777
    mcall   ,<16,90> ,       ,3,0x007700
    mcall   ,<17,186>,<98,20>,5,0xe4dfe1

    mcall 4,<28,105>,0x80000000,label4        ;eax=4 - write text
    mcall  ,<35,24> ,0x80ffffff,label2
    mcall  ,<34,58> ,          ,label3
    mcall  ,<47,37> ,          ,label5
    mcall  ,<43,71> ,          ,label6

    mcall 12,2

still:
    mcall 10                                 ;wait here for event
    dec  eax
    jz   draw_window
    dec  eax
    jnz  button

    mcall 2                                  ;eax=2 - get key code
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
    mcall 17                                 ;eax=17 - get pressed button id
    xchg al,ah
    dec  eax
    jz   close_1
    dec  eax
    jz   restart_kernel
    dec  eax
    jz   restart
    dec  eax
    jnz   run_rdsave

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
    mcall 18,9

close_1:
    mcall -1

run_rdsave:
    mcall 70,rdsave
    jmp still

data
include 'data.inc'

udata

meos_app_end