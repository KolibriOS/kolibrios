;
;   DEBUG BOARD for APPLICATIONS and KERNEL DEVELOPMENT
;
;   See f63
;
;   Compile with FASM for Menuet
;
WRITE_LOG  equ 1

include 'lang.inc'

   use32
   org	  0x0
   db	  'MENUET01'		  ; 8 byte id
   dd	  0x01			  ; header version
   dd	  START 		  ; start of code
   dd	  I_END 		  ; size of image
   dd	  mem			; memory for app
   dd	  mem			; esp
   dd	  filename , 0x0	  ; I_Param , I_Icon
include '../../../macros.inc'
include 'debug.inc'
purge newline
MAXSTRINGS = 16
TMP = 80*(MAXSTRINGS+1)

START:				; start of execution
	mov	edi, filename
	cmp	byte [edi], 0
	jnz	param
	mov	esi, default_filename
@@:
	lodsb
	stosb
	test	al, al
	jnz	@b
param:

; allow user to see messages written before start
;     mov  ecx,4096
;    flush:
;     mov  eax,63
;     mov  ebx,2
;     mcall
;     loop flush

     mov  ecx, TMP
     xor  eax, eax
     mov  edi, [targ]
     rep  stosb

     mov  [tmp1],'x'
     mov  [tmp2],'x'

     mov  eax,14
     mcall
     and  eax,0xffff0000
     sub  eax,399 shl 16
     add  eax,399
     mov  [xstart],eax

     mov  eax,48
     mov  ebx,3
     mov  ecx,sc
     mov  edx,sizeof.system_colors
     mcall
  if WRITE_LOG
     mov  esi, filename
     call CreateFile
  end if

  red:
     call draw_window

still:

    mov  eax,23 		; wait here for event
    mov  ebx,1
    mcall

    cmp  eax,1			; redraw request ?
    je	 red
    cmp  eax,2			; key in buffer ?
    je	 key
    cmp  eax,3			; button in buffer ?
    je	 button

    mov  eax,63
    mov  ebx,2
    mcall

    cmp  ebx,1
    jne  still

  new_data:
  if WRITE_LOG
     pusha
     mov  [tmp], al
     mov  edx, tmp
     mov  ecx, 1
     mov  esi, filename
  .write_to_logfile:
     call WriteToFile
     cmp  eax, 5
     jne  @f
     mov  esi, filename
     mov  [filepos], 0
     call CreateFile
     jnc  .write_to_logfile
  @@:
     inc  [filepos]
     popa
  end if

    mov  ebp,[targ]
  .no4:
    cmp  al,13
    jne  no13
    and  dword[ebp-8],0
    jmp  new_check
   no13:
    cmp  al,10
    jne  no10
    and  dword[ebp-8],0
    inc  dword[ebp-4]
    cmp  dword[ebp-4],MAXSTRINGS
    jbe  .noypos
    mov  dword[ebp-4],MAXSTRINGS
    lea  esi,[ebp+80]
    mov  edi,ebp
    mov  ecx,80*(MAXSTRINGS)
    cld
    rep  movsb

    mov  esi,[ebp-4]
    imul esi,80
    add  esi,[ebp-8]
    add  esi,ebp
    mov  ecx,80
    xor  al,al
    rep  stosb
  .noypos:
    mov  [targ],text2
    and  [krnl_cnt],0
    jmp  new_check
  no10:
    cmp  ebp,text1
    je	 add2
		mov  ecx,[krnl_cnt]
		  cmp  al,[krnl_msg+ecx]
		jne  .noknl
		  inc  [krnl_cnt]
		cmp  [krnl_cnt],4
		jne  new_check
    mov  [targ],text1
	.noknl:
    mov  ebp,[targ]
	   jecxz .add
    push eax
    mov  esi,krnl_msg
   .l1:
    lodsb
    call add_char
    loop .l1
    pop  eax
   .add:
    and  [krnl_cnt],0
  add2:
    call add_char

  new_check:

    mov  eax,63
    mov  ebx,2
    mcall

    cmp  ebx,1
    je	 new_data

    call draw_window

    jmp  still

  key:				; key
    mov  al,2		       ; just read it and ignore
    mcall
    cmp  ah,' '
    je	 button.noclose
    jmp  still

  button:			; button
    mov  al,17		       ; get id
    mcall

    cmp  ah,1			; button id=1 ?
    jne  .noclose

    or	 eax,-1 		; close this program
    mcall
  .noclose:
    xor  [vmode],1
    jmp  red

add_char:
    push esi
    mov  esi,[ebp-4]
    imul esi,80
    add  esi,[ebp-8]
    mov  [ebp+esi],al
    inc  dword[ebp-8]
    cmp  dword[ebp-8],80
    jb	 .ok
    mov  dword[ebp-8],79
  .ok:
    pop  esi
    ret

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,1			   ; 1, start of draw
    mcall

				   ; DRAW WINDOW
    xor  eax,eax		     ; function 0 : define and draw window
;   mov  ebx,50*65536+400          ; [x start] *65536 + [x size]
    mov  ebx,[xstart]
    mov  ecx,MAXSTRINGS*10+45	   ; [y start] *65536 + [y size]
    mov  edx,[sc.work]		   ; color of work area RRGGBB,8->color gl
    or	 edx,0x13000000
    mov  edi,title		  ; WINDOW LABEL
    mcall

    mov  ecx,4
    mov  esi,[sc.work]
    mov  ebx,296 shl 16+5*6
    mov  edx,3;+1 shl 30
    mcall 8,,<5,12>
    mov  edx,[vmode]
    lea  edx,[edx*4+duk]
    mcall 4,<300,8>,,,4

    mov  ebx,15*65536+33	   ; draw info text with function 4
    mov  ecx,[sc.work_text]
    mov  edx,text1
    cmp  [vmode],0
    je	 .kern
    mov  edx,text2
  .kern:
    mov  esi,80
    mov  eax,4
  newline:
    mcall
    add  ebx,10
    add  edx,80
    cmp  [edx],byte 'x'
    jne  newline
    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,2			   ; 2, end of draw
    mcall

    ret


if WRITE_LOG
;********************************************
;*  input:  esi = pointer to the file name  *
;********************************************

CreateFile:
    pusha
    mov      dword [InfoStructure],    2   ; create file
    mov      dword [InfoStructure+4],  0   ; reserved
    mov      dword [InfoStructure+8],  0   ; reserved
    mov      dword [InfoStructure+12], 0   ; 0 bytes to write (just create)
    mov      dword [InfoStructure+16], 0   ; NULL data pointer (no data)
    mov      dword [InfoStructure+20], 0   ; reserved
    mov      dword [InfoStructure+21], esi ; pointer to the file name
    mcall    70, InfoStructure
    test     eax, eax
    jz	     .out
    stc
  .out:
    popa
    ret

;********************************************
;*  input:  esi = pointer to the file name  *
;*          edx = pointer to data buffer    *
;*          ecx = data length               *
;********************************************

WriteToFile:
    push     ebx
    mov      dword [InfoStructure],    3   ; write to file
    mov      eax,  [filepos]
    mov      dword [InfoStructure+4],  eax ; lower position addr
    mov      dword [InfoStructure+8],  0   ; upper position addr (0 for FAT)
    mov      dword [InfoStructure+12], ecx ; number of bytes to write
    mov      dword [InfoStructure+16], edx ; pointer to data buffer
    mov      dword [InfoStructure+20], 0   ; reserved
    mov      dword [InfoStructure+21], esi ; pointer to the file name
    mcall    70, InfoStructure
    clc
    test     eax, eax
    jz	     .out
    stc
  .out:
    pop      ebx
    ret


InfoStructure:
		     dd      0x0     ; subfunction number
		     dd      0x0     ; position in the file in bytes
		     dd      0x0     ; upper part of the position address
		     dd      0x0     ; number of     bytes to read
		     dd      0x0     ; pointer to the buffer to write data
		     db      0
		     dd      0	     ; pointer to the filename

filepos  dd 0
default_filename db '/sys/boardlog.txt',0
tmp	 db 0
end if




krnl_msg db 'K : '
duk db 'KernUser'

; DATA AREA

; 11,11 > 0,-1
; 5,11  > 0,-1
if lang eq ru
   title    db	 'Доска отладки и сообщений',0
else if lang eq en
   title    db	 'General debug & message board',0
else
   title    db	 'Allgemeines debug- & nachrichtenboard',0
end if
   krnl_cnt dd 0
   vmode dd 1
   targ  dd text2
I_END:
     offs dd ?
     flag rb 1
     rd 2
;     x1pos  dd ?
;     y1pos  dd ?
     text1 rb 80*(MAXSTRINGS+1)
     tmp1  db ?
     rd 2
;     x2pos  dd ?
;     y2pos  dd ?
     text2 rb 80*(MAXSTRINGS+1)
     tmp2  db ?
     xstart dd ?
     sc system_colors
i_end:

filename	rb	256
align 4
stackbuf	rb	2000h
mem:
