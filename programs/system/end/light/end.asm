;
; END
; KolibriOS Team 2005-2013
;
; <diamond> note that 'mov al,xx' is shorter than 'mov eax,xx'
;           and if we know that high 24 bits of eax are zero, we can use 1st form
;           the same about ebx,ecx,edx

include "lang.inc"
include "..\..\..\macros.inc"

meos_app_start
code

do_draw:

    mcall 12,1

    mov  al,14 		   ; eax=14 - get screen max x & max y
    mcall

    movzx ecx,ax

    shr  eax,17
    shl  eax,16
    lea  ebx,[eax-110*10000h+222]

    shr  ecx,1
    shl  ecx,16
    sub  ecx,50*10000h - 115

    xor  eax,eax			   ; define and draw window
    mov  edx,0x01cccccc
    mov  esi,edx
    mov  edi,edx
    mcall

   mcall 13,0 shl 16+222,0 shl 16+230, 0xcccccc
   mcall ,19 shl 16+87,21 shl 16+24, 0x555555
   push ebx
   mcall ,122 shl 16+87
   xchg ebx,[esp]
   mcall ,,55 shl 16+24
   pop  ebx
   mcall

   mov al,8
   mcall ,15 shl 16+87,17 shl 16+24,4,0xaa0044
   mcall ,118 shl 16+87,,2,0xbb7700
   mcall ,15 shl 16+87,51 shl 16+24,3,0x8800
   mcall ,118 shl 16+87,,1,0x999999
   mcall ,10 shl 16+200,88 shl 16+15,0x40000005
   mcall 38,27 shl 16 +193,102 shl 16 +102,0x000000dd

    mov  al,4			   ; 0x00000004 = write text
    mov  ebx,28*65536+93
    mov  ecx,0x800000dd
    mov  edx,label4
    mcall

    mov  ecx,0x90eeeeee            ; 8b window nro - RR GG BB color
    mov  ebx,24*65536+20
    mov  edx,label2		     ; pointer to text beginning
    mcall

    mov  ebx,19*65536+54
    mov  edx,label3
    mcall

    mov  ebx,44*65536+31
    mov  edx,label5
    mcall

    mov  ebx,39*65536+65
    mov  edx,label6
    mcall

    mcall 12,2

still:

    mov  eax,10 		; wait here for event
    mcall

    dec  eax
    jz   do_draw
    dec  eax
    jnz  button
  key:
    mov  al,2	; now eax=2 - get key code
    mcall
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
    mov  al,17	; now eax=17 - get pressed button id
    mcall
    xchg al,ah
    dec  eax
    jz   close_1
    dec  eax
    jz   restart_kernel
    dec  eax
    jz   restart
    dec  eax
    jnz   run_rdsave
; we have only one button left, this is close button
;    dec  eax
;    jnz  still
power_off:
    push 2
    jmp  mcall_and_close

close_1:
    or   eax,-1
    mcall

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

if lang eq ru
  label2:
      db   'Выключить         Ядро',0
  label3:
      db   'Перезапуск       Отмена',0
  label4:
      db   'Сохранить настройки (Ctrl-S)',0

else if lang eq ge
  label2:
      db   '  Beenden         Kernel',0
  label3:
      db   '   Neustart      Abbrechen',0
  label4:
      db   'Save your settings (Ctrl-S)',0

else if lang eq it
  label2:
      db   '  Spegni         Kernel',0
  label3:
      db   '   Riavvio        Annulla',0
  label4:
      db   'Salva impostazioni (Ctrl-S)',0

else
  label2:
      db   'Power off          Kernel',0
  label3:
      db   '   Restart          Cancel',0
  label4:
      db   'Save your settings (Ctrl-S)',0

end if

  label5:
      db   '(End)           (Home)',0
  label6:
      db   '(Enter)          (Esc)',0

rdsave:
        dd      7
        dd      0
        dd      0
        dd      0
        dd      0
        db      '/sys/rdsave',0
udata
meos_app_end
