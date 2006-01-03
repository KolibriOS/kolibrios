;
; END
;

include "lang.inc"
include "macros.inc"

meos_app_start
code

    mov   eax,40
    mov   ebx,0111b
    int   0x40

    call  draw_window

still:

    mov  eax,10 		; wait here for event
;    mov  ebx,100
    int  0x40

    cmp  eax,1
    jz	 red
    cmp  eax,2
    jz	 key
    cmp  eax,3
    jz	 button

    jmp  still

  red:
    call draw_window
    jmp  still

  key:
     mcall 2
     cmp  ah,13
     jz   restart
     cmp  ah,27
     jz   close_1
     cmp  ah,180
     jz   restart_kernel
     cmp  ah,181
     jz   power_off
     jmp  red

  button:
    mov  eax,17
    int  0x40

    cmp  ah,1
    jne  no_1
 power_off:
    mcall 18,9,2
    jmp  close_1

no_1:
    cmp  ah,3
    jne  no_2
 restart:
    mcall 18,9,3
    jmp  close_1

no_2:
    cmp  ah,2
    jne  no_4

  restart_kernel:
    mcall 18,9,4

no_4:
    cmp  ah,4
    jne  still

  close_1:
    or	 eax,-1    ; close this program
    int  0x40


draw_window:

    pusha

    mov  eax,48
    mov  ebx,3
    mov  ecx,sc
    mov  edx,sizeof.system_colors
    int  0x40

    mov  eax,12 		   ; tell os about redraw start
    mov  ebx,1
    int  0x40

    mov  eax,14 		   ; get screen max x & max y
    int  0x40

    xor  ecx,ecx
    mov  cx,ax

    shr  eax,17
    sub  eax,110
    shl  eax,16
    mov  ebx,eax
    add  ebx,220

    shr  ecx,1
    sub  ecx,50
    shl  ecx,16
    add  ecx,100

    mov  eax,0			   ; define and draw window
    mov  edx,[sc.work_button]
    mov  esi,edx ;[sc.work_button]
    xor edi,edi
    int  0x40

   xor edx,edx
   mcall 13,14 shl 16+90,25 shl 16+27
   mcall 13,117 shl 16+90,
   mcall 13,14 shl 16+90,59 shl 16+27
   mcall 13,117 shl 16+90,

   mcall 8,15 shl 16+87,26 shl 16+24,1,0xdd7700
   inc dl
   mcall 8,118 shl 16+87,,,0xbbbb   ;cccc
   inc dl
   mcall 8,15 shl 16+87,60 shl 16+24,,0xbb00
   inc dl
   mcall 8,118 shl 16+87,,,0xbbbbbb ;cccccc

    mov  eax,4			   ; 0x00000004 = write text
    mov  ebx,75*65536+10
    mov  ecx,[sc.work_button_text] ; 8b window nro - RR GG BB color
    or	 ecx,0x10000000
    mov  edx,label1		   ; pointer to text beginning
    mov  esi,label1_len 	   ; text length
    int  0x40

    mov  ecx,0x10ffffff
    mov  ebx,25*65536+30
    mov  edx,label2		   ; pointer to text beginning
    mov  esi,label2_len 	   ; text length
    int  0x40

    mov  ebx,20*65536+64
    mov  edx,label3		   ; pointer to text beginning
    mov  esi,label3_len 	   ; text length
    int  0x40

    mov  ecx,0xffffff
    mov  ebx,45*65536+41
    mov  edx,label4		   ; pointer to text beginning
    mov  esi,label4_len 	   ; text length
    int  0x40

    mov  ebx,40*65536+75
    mov  edx,label5		   ; pointer to text beginning
    mov  esi,label5_len 	   ; text length
    int  0x40

    mov  eax,12 		   ; tell os about redraw end
    mov  ebx,2
    int  0x40

    popa

    ret



data

if lang eq ru

  label1:
      db   'ÇÄò ÇõÅéê:'
  label1_len = $ - label1

  label2:
      db   'Çõäãûóàíú         üÑêé'
  label2_len = $ - label2

  label3:
      db   'èÖêÖáÄèìëä       éíåÖçÄ'
  label3_len = $ - label3

  label4:
      db   '(End)            (Home)'
  label4_len = $ - label4

  label5:
      db   '(Enter)           (Esc)'
  label5_len = $ - label5

else

  label1:
      db   ' SELECT:'
  label1_len = $ - label1

  label2:
      db   'POWER OFF        KERNEL'
  label2_len = $ - label2

  label3:
      db   '  RESTART         CANCEL'
  label3_len = $ - label3

  label4:
      db   '(End)            (Home)'
  label4_len = $ - label4

  label5:
      db   '(Enter)           (Esc)'
  label5_len = $ - label5  

end if


udata
  sc  system_colors

meos_app_end
