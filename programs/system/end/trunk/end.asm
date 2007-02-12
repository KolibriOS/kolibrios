;
; END
;
; <diamond> note that 'mov al,xx' is shorter than 'mov eax,xx'
;           and if we know that high 24 bits of eax are zero, we can use 1st form
;           the same about ebx,ecx,edx

include "lang.inc"
include "macros.inc"

meos_app_start
code

do_draw:

    mov  eax,48
    mov  ebx,3
    mov  ecx,sc
    mov  edx,sizeof.system_colors
    int  0x40

    mov  al,12 		   ; eax=12 - tell os about redraw start
    mov  bl,1
    int  0x40

    mov  al,14 		   ; eax=14 - get screen max x & max y
    int  0x40

    movzx ecx,ax

    shr  eax,17
    shl  eax,16
    lea  ebx,[eax-110*10000h+220]

    shr  ecx,1
    shl  ecx,16
    sub  ecx,50*10000h - 120

    xor  eax,eax			   ; define and draw window
    mov  edx,[sc.work]
    mov  esi,edx
    mov  edi,edx
    int  0x40

   mov edx,0x444444
   mov al,13
   mcall ,18 shl 16+90,29 shl 16+27
   push ebx
   mcall ,121 shl 16+90
   xchg ebx,[esp]
   mcall ,,63 shl 16+27
   pop  ebx
   mcall
   mcall ,179 shl 16+30,99 shl 16+15

   xor edx,edx
   mov al,8
   inc edx
   mcall ,15 shl 16+87,26 shl 16+24,,0xbb0055
   inc edx
   mcall ,118 shl 16+87,,,0xaaaa   ;cccc
   inc edx
   mcall ,15 shl 16+87,60 shl 16+24,,0x9900
   inc edx
   mcall ,118 shl 16+87,,,0xaaaaaa ;cccccc
   inc edx
   mcall ,175 shl 16+30,95 shl 16+15,,[sc.work_button]

    mov  al,4			   ; 0x00000004 = write text
    mov  ebx,75*65536+10
    mov  ecx,[sc.work_text] ; 8b window nro - RR GG BB color
    or   ecx,0x90000000
    mov  edx,label1		   ; pointer to text beginning
    int  0x40

    mov  ebx,15*65536+101
    mov  edx,label4
    int  0x40

    mov  ecx,0x90eeeeee            ; 8b window nro - RR GG BB color
    mov  ebx,25*65536+30
    mov  edx,label2		     ; pointer to text beginning
    int  0x40

    mov  ebx,20*65536+64
    mov  edx,label3
    int  0x40

    mov  ebx,45*65536+41
    mov  edx,label5
    int  0x40

    mov  ebx,40*65536+75
    mov  edx,label6
    int  0x40

    mov  al,12 		   ;end of redraw 
    mov  ebx,2
    int  0x40

still:

    mov  eax,10 		; wait here for event
    int  0x40

    dec  eax
    jz   do_draw
    dec  eax
    jnz  button
  key:
    mov  al,2	; now eax=2 - get key code
    int  40h
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
    int  0x40
    xchg al,ah
    dec  eax
    jz   power_off
    dec  eax
    jz   restart_kernel
    dec  eax
    jz   restart
    dec  eax
    jnz   run_rdsave
; we have only one button left, this is close button
;    dec  eax
;    jnz  still
close_1:
    or   eax,-1
    int  40h

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
    int 0x40
    jmp still

data

if lang eq ru
  label1:
      db   'Ç†Ë ¢Î°Æ‡:',0
  label2:
      db   'ÇÎ™´ÓÁ®‚Ï         ü§‡Æ',0
  label3:
      db   'è•‡•ß†Ø„·™       é‚¨•≠†',0
  label4:
      db   'ëÆÂ‡†≠®‚Ï ≠†·‚‡Æ©™®',0

else if lang eq en
  label1:
      db   ' SELECT:',0
  label2:
      db   'POWER OFF        KERNEL',0
  label3:
      db   '  RESTART         CANCEL',0
  label4:
      db   'Save settings',0

else if lang eq et
  label1:
      db   '  VALI:',0
  label2:
      db   'L‹LITA VƒLJA     KERNEL',0
  label3:
      db   '  RESTART         T‹HISTA',0
  label4:
      db   'Save settings',0

else 
  label1:
      db   'WAEHLEN:',0
  label2:
      db   ' BEENDEN         KERNEL',0
  label3:
      db   '  NEUSTART     ABBRECHEN',0
  label4:
      db   'Save settings',0

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
        db      '/rd/1/rdsave',0
udata
  sc  system_colors

meos_app_end
