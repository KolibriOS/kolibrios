;
;    ICON APPLICATION
;
;    Compile with FASM for Menuet
;
;    22.02.05 Mario79
;      1) multi-threading
;      2)dinamic load BMP files
;      3)type on desktop with function 7
;      4)Image in memory only 8000h (32Kb) for all 48 icons.
;        Old version ICON 5000h*48(decimal)=960Kb.
;

use32

  org	 0x0

  db	 'MENUET01'
  dd	 0x01
  dd	 START
  dd	 I_END
  dd	 0x8000
  dd	 0x8000
  dd	 I_Param , 0x0

include 'lang.inc'
include 'macros.inc'

get_bg_info:
    mov  eax,39
    mov  ebx,4
    int  0x40
    mov  [bgrdrawtype],eax

    mov  eax,39     ; get background size
    mov  ebx,1
    int  0x40
    mov  [bgrxy],eax

    mov  ebx,eax
    shr  eax,16
    and  ebx,0xffff
    mov  [bgrx],eax
    mov  [bgry],ebx
    ret

START:
load_icon_list:
    mov   eax,6
    mov   ebx,icon_lst
    xor   ecx,ecx
    mov   edx,-1
    mov   esi,icon_data
    int   0x40

    add   eax,10
    xor   edx,edx
    mov   ebx,52
    div   ebx
    mov   [icons],eax

    call  get_bg_info

    mov  eax,14
    int  0x40
    add  eax,0x00010001
    mov  [scrxy],eax

apply_changes:

    mov  edi,[icons]
    mov  esi,icon_data
    mov  ebp,0x5000 ; threads stack starting point

  start_new:
    movzx eax,byte [esi]    ; x position
    sub  eax,'A'	    ;eax - number of letter
    cmp  eax,4
    jg	 no_left
    shl  eax,6 ;imul eax,64
    add  eax,24
    jmp  x_done
  no_left:
    sub  eax,9
    shl  eax,6 ;imul eax,64
    sub  eax,24+32-1
    movzx ebx,word [scrxy+2]
    add  eax,ebx
  x_done:
;    mov  [xpos],eax
    mov  [ebp-12],eax

    movzx eax,byte [esi+1]  ; y position
    sub  eax,'A'	    ; eax - number of letter
    cmp  eax,4
    jg	 no_up
    lea  eax,[eax+4*eax]
    shl  eax,4		    ;imul eax,80
    add  eax,34
    jmp  y_done
  no_up:
    sub  eax,9
    lea  eax,[eax+4*eax]
    shl  eax,4		    ;imul eax,80
    movzx ebx,word [scrxy]
    sub  eax,32-1
    add  eax,ebx
  y_done:
;    mov  [ypos],eax
    mov  [ebp-8],eax

    mov  eax,51
    mov  ebx,1
    mov  ecx,thread
;    mov  edx,[thread_stack]
    mov  edx,ebp
;    sub  edx,4
;    mov  [edx],esi
    mov  [ebp-4],esi
    int  0x40
;    add  [thread_stack],0x100
    add  ebp,0x100

    mov  eax,5
    mov  ebx,1
wait_thread_start:		 ;wait until thread draw itself first time
    cmp  [create_thread_event],bl
    jz	 wait_thread_end
    int  0x40
    jmp  wait_thread_start
wait_thread_end:
    dec  [create_thread_event]	 ;reset event


    add  esi,50+2
    dec  edi
    jnz  start_new
    or	 eax,-1
    int  0x40

thread:
;   pop  ebp ;ebp - address of our icon
    sub  esp,12
    mov  ebp,esp
    call draw_window
    mov  [create_thread_event],1
    mov  eax,40
    mov  ebx,010101b
    int  0x40
    jmp  still
red:
    call draw_window

still:

    mov  eax,10
    int  0x40

    cmp  eax,1
    je	 red
    cmp  eax,3
    je	 button
    cmp  eax,5
    jne  still

    call  get_bg_info
    mov   eax,5
    mov   ebx,1
    call  draw_icon

    jmp  still

  key:
    mov  eax,2
    int  0x40

    jmp  still

  button:
    mov  eax,17
    int  0x40

;    mcall 55,eax, , ,klick_music

    mov  ebx,[ebp+8]
    add  ebx,19
;    lea  ebx,[ebp+19]
    mov  eax,19
    xor  ecx,ecx
    int  0x40
    mcall 55,eax, , ,klick_music
    jmp  still

klick_music db 0x85,0x60,0x85,0x70,0x85,0x65,0

draw_picture:
    mov  [image],0x3000
    xor  ebx,ebx
    xor  ecx,ecx
    mov  esi,data_from_file+54+32*3*33-96
    mov  [pixpos],0
    mov  [pixpos2],32
  newb:
    push ebx
    push ecx

    cmp  ebx,10
    jb	 yesbpix
    cmp  ebx,42
    jge  yesbpix
    cmp  ecx,32
    jg	 yesbpix

    push esi
    mov  esi,data_from_file+54+32*3*33-96
    sub  esi,[pixpos]

    dec  [pixpos2]
    cmp  [pixpos2],0
    jne  no_correction_pixpos
    add  [pixpos],192
    mov  [pixpos2],32
no_correction_pixpos:
    sub  [pixpos],3
    mov  eax,[esi]
    and  eax,0xffffff

    pop  esi

    cmp eax,0
    je	yesbpix
    cmp eax,0xf5f5f5
    je	yesbpix
    jmp nobpix

  yesbpix:

  stretch:
    cmp   [bgrdrawtype],dword 2
    jne   nostretch
;    mov   eax,[ypos]
    mov   eax,[ebp+4]
    add   eax,ecx
    imul  eax,[bgry]
    cdq
    movzx ebx,word [scrxy]
    div   ebx
    imul  eax,[bgrx]
    push  eax
;    mov   eax,[xpos]
    mov   eax,[ebp+0]
    add   eax,[esp+8]
    imul  eax,[bgrx]
    cdq
    movzx ebx,word [scrxy+2]
    div   ebx
    add   eax,[esp]
    add   esp,4

    jmp   notiled

  nostretch:

    cmp   [bgrdrawtype],dword 1
    jne   notiled
;    mov   eax,[ypos]
    mov   eax,[ebp+4]
    add   eax,ecx
    cdq
    movzx ebx,word [bgrxy]
    div   ebx
    mov   eax,edx
    imul  eax,[bgrx]
    push  eax
;    mov   eax,[xpos]
    mov   eax,[ebp+0]
    add   eax,[esp+8]
    movzx ebx,word [bgrxy+2]
    cdq
    div   ebx
    mov   eax,edx
    add   eax,[esp]
    add   esp,4

  notiled:

    lea  ecx,[eax+eax*2]
    mov  eax,39
    mov  ebx,2
    int  0x40

  nobpix:

    pop  ecx
    pop  ebx

    mov  edx,eax
    mov  eax,[image]
    mov  [eax],dl
    inc  eax
    ror  edx,8
    mov  [eax],dl
    inc  eax
    ror  edx,8
    mov  [eax],dl
    inc  eax
    mov  [image],eax

    inc  ebx
    mov  eax,[yw]
    inc  eax
    cmp  ebx,eax
    jnz  newb
    xor  ebx,ebx

    inc  ecx

    mov  eax,[ya]
    add  [pixpos],eax

    cmp  [top],1
    jne  notop
    cmp  ecx,38
    je	 toponly

  notop:

    cmp  ecx,52
    jnz  newb

  toponly:

    mov  eax,7
    mov  ebx,0x3000
    mov  ecx,52 shl 16 + 52
    xor  edx,edx
    int  0x40
    mov  [load_pic],0
    ret

draw_text:

    mov  esi,[ebp+8]
    add  esi,33
;    lea  esi,[ebp+33]
    push edi
    mov  edi,labelt
    mov  ecx,8
    cld
    rep  movsb
    pop  edi
    mov   eax,labelt
  news:
    cmp   [eax],byte 33
    jb	  founde
    inc   eax
    cmp   eax,labelt+11
    jb	  news
   founde:
    sub   eax,labelt
    mov   [tl],eax

    mov   eax,[tl]
    lea   eax,[eax+eax*2]  ; eax *= char_width/2
    shl   eax,16

    mov   ebx,27*65536+42
    sub   ebx,eax

    mov   eax,4
    xor   ecx,ecx		 ; black shade of text
    mov   edx,labelt
    mov   esi,[tl]
    add   ebx,1 shl 16	  ;*65536+1
    int   0x40
    inc   ebx
    int   0x40
    add   ebx,1 shl 16
    int   0x40
    inc   ebx
    int   0x40
    sub   ebx,1 shl 16
    int   0x40
    dec   ebx
    sub   ebx,1 shl 16
    int   0x40
    sub   ebx,1 shl 16
    dec   ebx
    int   0x40
    dec   ebx
    add   ebx,1 shl 16
    int   0x40
    inc   ebx
    mov   ecx,0xffffff

    int   0x40
    mov   [draw_pic],0
    ret

load_icon_file:
    mov  ebx,[ebp+8]
    add  ebx,5
;    lea  ebx,[ebp+5]

    mov  eax,6
    xor  ecx,ecx
    mov  edx,-1
    mov  esi,data_from_file
    int  0x40

    ret

    ; for y = 32 to 0
    ;   for x = 0 to 32
    ;     if (pix[y][x]==0) then
    ;        pix[y][x]=background(x,y);

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,1			   ; 1, start of draw
    int  0x40

				   ; DRAW WINDOW
    xor  eax,eax		     ; function 0 : define and draw window
;    mov  ebx,[xpos-2]
    mov  ebx,[ebp+0-2]
;    mov  ecx,[ypos-2]
    mov  ecx,[ebp+4-2]
    add  ebx,[yw]		   ; [x start] *65536 + [x size]
    add  ecx,51 		   ; [y start] *65536 + [y size]
    mov  edx,0x01000000 	   ; color of work area RRGGBB,8->color gl
    int  0x40

    mov  eax,8	  ; button
    mov  ebx,51
    mov  ecx,50
    mov  edx,1+20000000 ; or 0x40000000
    int  0x40

    mov  eax,5
    mov  ebx,1
draw_icon:
    xchg [load_pic],bl
    test bl,bl
    je	 draw_icon_end
    int  0x40
    jmp  draw_icon
draw_icon_end:

    call load_icon_file

    mov  eax,5
    mov  ebx,1
draw_icon_2:
    xchg [draw_pic],bl
    test bl,bl
    je	 draw_icon_end_2
    int  0x40
    jmp  draw_icon_2
draw_icon_end_2:

    mov  eax,9
    mov  ebx,process_table
    mov  ecx,-1
    int  0x40
;    mov  eax,process_table
;    add  eax,34
;    mov  ebx,[eax]
;    mov  [xpos],ebx
;    add  eax,4
;    mov  ebx,[eax]
;    mov  [ypos],ebx

    call draw_picture
    call draw_text

    mov  eax,12
    mov  ebx,2
    int  0x40

    ret



icon_lst db 'ICON    LST'

tl	    dd	  8
yw	    dd	 51
ya	    dd	  0

;xpos       dd   15
;ypos       dd  185
draw_pic    db	  0
load_pic    db	  0
create_thread_event db 0

labelt:
	    db	'SETUP      '
labellen:

bgrxy	    dd	0x0
scrxy	    dd	0x0
bgrdrawtype dd	0x0

pixpos	    dd	0
pixpos2     db	0

top	  dd 0

image	      dd  0x3000
;thread_stack  dd  0x5000

icons dd 0


I_Param:

 icon_data = I_END+0x1400
 process_table = I_END+0x2400

I_END:

bgrx dd ?
bgry dd ?
data_from_file:
  rb 54 ;header
 raw_data:
  rb 32*32
