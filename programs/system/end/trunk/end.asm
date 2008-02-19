;
; END
;
; <diamond> note that 'mov al,xx' is shorter than 'mov eax,xx'
;           and if we know that high 24 bits of eax are zero, we can use 1st form
;           the same about ebx,ecx,edx
;
; 15.02.2007 merge old END with new one (fron Veliant & Leency) by Heavyiron

include "..\..\..\macros.inc"

meos_app_start
code

draw:

    mov  al,12 		   ; eax=12 - tell os about redraw start
    mov  ebx,1
    mcall

    mov  al,14 		   ; eax=14 - get screen max x & max y
    mcall

    movzx ecx, ax
    shr eax, 16
    shr ecx, 1
    shr eax, 1
    sub eax, 157
    sub ecx, 100
    mov ebx, eax
    shl ebx, 16
    add ebx, 253
    shl ecx, 16
    add ecx, 154

    xor  eax,eax			   ; define and draw window
    mov  edx,0x41ffffff
    mcall

    mov al, 7
    mov ebx, background
    mov ecx, 254 shl 16 + 155
    xor edx, edx
    mcall

   
    mov al,8
    mcall ,58 shl 16 + 32,59 shl 16 + 34, 1 shl 30 + 1
    inc edx
    mcall ,110 shl 16 + 32,54 shl 16 + 32
    inc edx
    mcall ,161 shl 16 + 32,59 shl 16 + 34
    inc edx
    mcall ,193 shl 16 + 43,125 shl 16 + 16,
    inc edx
    mcall ,144 shl 16 + 43

    mov  al,12 		   ;end of redraw 
    mov  ebx,2
    mcall

still:

    mov  eax,10 		; wait here for event
    mcall

    dec  eax
    jz   draw
    dec  eax
    jnz  button
  key:
    mov  al,2	; now eax=2 - get key code
    mcall
    mov  al,ah
     cmp  al,13
     jz   restart
     cmp  al,27
     jz   close_1
     cmp  al,180
     jz   restart_kernel
     cmp  al,181
     jz   power_off
     jmp  still

  button:
    mov  al,17	; now eax=17 - get pressed button id
    mcall
    xchg al,ah
    dec  eax
    jz   restart_kernel
    dec  eax
    jz   power_off
    dec  eax
    jz   restart
    dec  eax
    jnz   run_rdsave
; we have only one button left, this is close button
;    dec  eax
;    jnz  still
close_1:
    or   eax,-1
    mcall

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
    jmp  close_1

run_rdsave:
    mov eax,70
    mov ebx,rdsave
    mcall
    jmp still

data
background	file 'back.raw'
rdsave:
        dd      7
        dd      0
        dd      0
        dd      0
        dd      0
        db      '/sys/rdsave',0
udata

meos_app_end
