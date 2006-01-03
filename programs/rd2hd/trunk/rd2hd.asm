;
;   Save Ramdisk to HD
;
;   Compile with FASM for Menuet
;

include 'lang.inc'
include 'macros.inc'

  use32
  org	 0x0

  db	 'MENUET01'		; 8 byte id
  dd	 0x01			; header version
  dd	 START			; start of code
  dd	 I_END			; size of image
  dd	 0x1000 		; memory for app
  dd	 0x1000 		; esp
  dd	 0x0 ; , 0x0            ; I_Param , I_Icon


;******************************************************************************


START:				; start of execution
    xor  eax,eax
    mov  edi,bootpath
    mov  ecx,128
    rep  stosd

    mcall 6,filename,0,-1,bootpath

    mov esi,bootpath+1
    mov cx,512
  start_search:
    lodsb
    cmp al,"'"
    jz	set_end_path
    dec cx
    cmp cx,0
    ja	start_search
  set_end_path:
    mov [esi-1],byte 0

    mov  eax,40
    mov  ebx,101b
    int  0x40

red:
    call draw_window

still:
    mov  eax, 10		 ; wait here for event
    int  0x40

    dec  eax		      ; redraw request ?
    je	 red
;    dec  eax
;    dec  eax                  ; button in buffer ?
;    je   button


  button:		       ; button
    mov  eax,17 	       ; get id
    int  0x40
    cmp  ah,2
    jne  ah_3
    mcall 18,6,1
    jmp  red
  ah_3:
    cmp  ah,3
    jne  ah_4
    mcall 18,6,2
    jmp  red
  ah_4:
    cmp  ah,4
    jne  ah_1
    mcall 18,6,3,bootpath+1
    jmp  red
  ah_1:
    cmp  ah,1
    je	 exit
    jmp  still

  exit:
    or	 eax,-1 		   ; close this program
    int  0x40


;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************

draw_window:

    mov  eax, 12		; function 12:tell os about windowdraw
    mov  ebx, 1 		; 1, start of draw
    int  0x40
				; DRAW WINDOW
    mov  eax, 0 		; function 0 : define and draw window
    mov  ebx, 200*65536+220	; [x start] *65536 + [x size]
    mov  ecx, 200*65536+150	 ; [y start] *65536 + [y size]
    mov  edx, 0x03ccddee       ; color of work area RRGGBB,8->color gl
    int  0x40
				; WINDOW LABEL
    mov  eax, 4 		; function 4 : write text to window
    mov  ebx, 8*65536+8 	; [x start] *65536 + [y start]
    mov  ecx, 0x10ffffff	; font 1 & color ( 0xF0RRGGBB )
    mov  edx, header		; pointer to text beginning
    mov  esi, header.size	; text length
    int  0x40

    xor  edx,edx
    mcall 13,<15,20>,<30,20>
    mcall 13, ,<60,20>
    mcall 13, ,<90,20>

    mcall 8,<16,17>,<31,17>,2,0xdd7700
    inc  edx
    mcall 8, ,<61,17>, ,0xbb00
    inc  edx
    mcall 8, ,<91,17>, ,0xcc5555

    mcall 4,<22,36>,0x10ffffff,text_123,1
    add bx,30
    add edx,1
    mcall 4
    add bx,30
    add edx,1
    mcall 4

    mcall 4,<45,36>,0,text_1,text_1.size
    add  bx,30
    mcall 4, ,0,text_2,text_2.size
    add  bx,30
    mcall 4, ,0,text_3,text_3.size
    mcall 4,<20,120>,0,text_4,text_4.size
    mcall 4,<40,130>,0,text_5,text_5.size

    mcall 4,<40,46>,0,text_6,text_6.size
    mcall 4,<40,106>,0,text_6,text_6.size

    mov  eax,12 		; function 12:tell os about windowdraw
    mov  ebx,2			; 2, end of draw
    int  0x40
ret




sz  header, "RD2HD сохранить IMG>HD"
text_123 db '123'
sz text_1, 'В папку /KOLIBRI'
sz text_2, 'В корень диска'
sz text_3, 'Путь в файле RD2HD.TXT'
sz text_4, 'п.3 для резервного сохранения,'
sz text_5, 'т.к. в ядре его нет.'
sz text_6, '(папка должна присутствовать)'
filename db 'RD2HD   TXT'
I_END:
bootpath: