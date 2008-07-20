;   Picture browser by lisovin@26.ru
;   Modified by Ivan Poddubny - v.0.3
;   Modified by Diamond - v.0.4
;   Modified by Mario79:
;      v.0.5  18.07.08 Dinamic Allocation Memory
;      v.0.6  20.07.08
;             1) Use Editbox (Author <Lrz>)
;             2) Draw window without fill working area (C = 1)
;             3) Open file with parameter in patch:
;                Size of parameter - 4 bytes. Parameter starts with the character "\",
;                the unused characters are filled by a blank (ASCII 20h).
;                '\T  /hd0/1/1.jpg' - set background, mode: tile
;                '\S  /hd0/1/1.jpg' - set background, mode: stretch
;   Compile with FASM for Menuet

;******************************************************************************
    use32
    org    0x0
    db	   'MENUET01'		   ; 8 byte id
    dd	   0x01 		   ; header version
    dd	   START		   ; start of code
    dd	   IM_END		   ; size of image
    dd	   I_END ;0x300000                 ; memory for app
    dd	   I_END ;0x300000                 ; esp
    dd	   temp_area , 0x0	   ; I_Param , I_Icon

include    'lang.inc'
include    '..\..\..\macros.inc'
;include    'macros.inc'
include    'EDITBOX.INC'
use_edit_box procinfo
;******************************************************************************

START:				; start of execution
	mcall	68, 11
; check for parameters
   cmp	 dword [temp_area],'BOOT'
   jne	 .no_boot
.background:
   call  load_image
   call  convert

   call  background
.exit:
   or	 eax,-1
   mcall
 .no_boot:

   cmp	 byte [temp_area],0
   jz	 .no_param


   mov	 edi,string	 ; clear string
   mov	 ecx,256/4	 ;   length of a string
   xor	 eax,eax	 ;   symbol <0>
   rep	 stosd


   mov	 edi,temp_area	 ; look for <0> in temp_area

   cmp	 [edi],byte "\"
   jne	 .continue
   cmp	 [edi+1],byte "T"
   jne	 @f
   mov	 [bgrmode],dword 1
   jmp	 .continue_1
@@:
   cmp	 [edi+1],byte "S"
   jne	 START.exit
   mov	 [bgrmode],dword 2
.continue_1:
   add	 edi,4
.continue:
   mov	 esi,edi
   mov	 ecx,257	 ;   strlen
   repne scasb
	lea	ecx, [edi-temp_area]

   mov	 edi,string
   rep	 movsb		 ; copy string from temp_area to "string" (filename)
   cmp	 [temp_area],byte "\"
   je	 START.background
   call  load_image
   call  convert

 .no_param:


   or  ecx,-1		; get information about me
   call getappinfo

   mov	edx,[process_info+30] ; теперь в edx наш идентификатор
   mov	ecx,eax

  @@:
   call getappinfo
   cmp	edx,[process_info+30]
   je	@f  ; если наш PID совпал с PID рассматриваемого процесса, мы нашли себя
   dec	ecx ; иначе смотрим следующий процесс
   jne	@b  ; возвращаемся, если не все процессы рассмотрены
  @@:

; теперь в ecx номер процесса
    mov  [process],ecx

draw_still:
    call draw_window

still:

    mov  eax,10 		; wait here for event
    mcall

	dec	eax
	jz	red
	dec	eax
	jnz	button

  key:				; key
    mov  al,2
    mcall
    mov  al,ah
    cmp  al,6
    je	 kfile
    cmp  al,15
    je	 kopen
    cmp  al,9
    je	 kinfo
    cmp  al,2
    je	 kbgrd
    jmp  still

  red:
	test	byte [status], 4
	jz	draw_still
	mov	al, 18
	mov	ebx, 3
	mov	ecx, [process]
	mcall
	and	byte [status], not 4
	jmp	still

  button:			; button
    mov  eax,17 		; get id
    mcall

    cmp  ah,1			; button id=1 ?
    jne  noclose

    mov  eax,-1 		; close this program
    mcall
  noclose:
    cmp  ah,2
    jne  nofile
  kfile:
	test	byte [status], 1
	jnz	still
	or	byte [status], 1
    mov  eax,51
    mov  ebx,1
    mov  ecx,thread1
    mov  edx,thread   ;0x29fff0
    mcall
    jmp  still
  nofile:
    cmp  ah,3
    jne  noopen

 kopen:
    mov ecx,-1
    call getappinfo
    mov eax,[image_file]
    mov ebx,dword [eax+42]
    mov ecx,dword [eax+46]
    add ebx,10*65536-15
    add ecx,50*65536-55
    mov edx,0xffffff
    mov eax,13
    mcall

    call load_image

  open1:
    mov eax,[image_file]
    cmp [eax],word 'BM'
    jne  still
    call convert
    call drawimage
    jmp  still
  noopen:

    cmp  ah,4
    jne  noinfo
  kinfo:
	test	byte [status], 2
	jnz	still
	or	byte [status], 2
    mov  eax,51
    mov  ebx,1
    mov  ecx,thread2
    mov  edx,thread-512  ;0x2afff0
    mcall
    jmp  still
  noinfo:

; УСТАНОВИТЬ ФОН
    cmp  ah,5
    jne  still
  kbgrd:
	test	byte [status], 8
	jnz	still
	or	byte [status], 8
    mov  eax,51
    mov  ebx,1
    mov  ecx,thread3
    mov  edx,thread-512*2  ;0x2bfff0
    mcall
    jmp  still
    ;call background

 getappinfo:
    mov  eax,9
    mov  ebx,process_info
    mcall
    ret


load_image:

	  mov	[fileinfo+0],dword 5
	  mov	[fileinfo+12],dword 0
	  mov	[fileinfo+16],dword process_info

    cmp  [image_file],0
    je	  @f
    mov   ecx,[image_file]
     mcall 68, 13,

@@:
	  mcall 70, fileinfo

	  mov	[fileinfo+0],dword 0
	  mov	[fileinfo+12],dword 512
	  mov	[fileinfo+16],dword process_info+40

	  mcall 70, fileinfo

	  mov	ecx,[process_info+32]
	  mov	[fileinfo+12],ecx

    mov  eax,[process_info+40+28]

	cmp	eax, 24
	jz	.convert24
	cmp	eax, 8
	jz	.convert8
	cmp	eax, 4
	jz	.convert4
     shl  ecx,2
.convert4:
     shl  ecx,1
.convert8:
     lea  ecx,[ecx*3]
.convert24:
     shl  ecx,1
@@:

	  mcall 68, 12


	  mov	[fileinfo+16],eax
	  mov	[image_file],eax

	  mcall 70, fileinfo

    mov  eax,[image_file]
    mov  ebx,[eax+22]
    mov  eax,[eax+18]
	test	ebx, ebx
	jns	@f
	neg	ebx
@@:
    add  eax,9	;20
    cmp  eax,210
    jae  @f
    mov  eax,210
@@:
    add  ebx,54     ;58
    cmp  ebx,56
    jae  @f
    mov  ebx,56
@@:
    mov  [wnd_width],eax
    mov  [wnd_height],ebx
    test [bWasDraw],1
    jz	 @f
    mov  esi,ebx
    mov  edx,eax
    mcall 67,-1,-1
@@:
    ret


  drawimage:
    mov  eax,[image_file]
    cmp  [eax],word 'BM'
    jne  nodrawimage
	mov	ecx, dword [eax+18-2]
	mov	cx, [eax+22]
	test	cx, cx
	jns	@f
	neg	cx
@@:
    mov  edx,5*65536+50
    mcall 7,[soi]
  nodrawimage:
    ret

; УСТАНОВИТЬ ФОН
  background:
    mov  eax,[image_file]
    cmp  [eax],word 'BM'
    jne  @f
    mov  ecx,[eax+18] ; ширина
    mov  edx,[eax+22] ; высота
    mcall 15,1

    mov  esi, ecx
    imul esi, edx
	lea	esi, [esi+esi*2]
    mov  ebx,5
    mov  ecx,[soi]
    xor  edx,edx
;;;    mov  esi, ;640*480*3
    mcall

    dec  ebx	;tile/stretch
    mov  ecx,dword [bgrmode]
    mcall

    dec  ebx
    mcall
   @@:
    ret

convert:
	mov	ecx, [image_file]  ;I_END
	add	ecx, [ecx+2]
	mov	[soi], ecx
	mov	eax,[image_file]
	mov	ebp, [eax+18]
	lea	ebp, [ebp*3]	; ebp = size of output scanline
	mov	eax, [eax+22]
	dec	eax
	mul	ebp
	add	eax, ecx
	mov	edi, eax	; edi points to last scanline
	mov	esi, [image_file]  ;I_END
	add	esi, [esi+10]
;        mov     ebx, I_END+54
	mov	ebx,[image_file]
	mov	edx, [ebx+22]
	add	ebx,54
	lea	eax, [ebp*2]
	mov	[delta], eax
	test	edx, edx
	jz	.ret
	jns	@f
	neg	edx
	and	[delta], 0
	mov	edi, ecx
@@:
;        movzx   eax, word [I_END+28]
	mov	eax,[image_file]
	movzx	eax,word [eax+28]
	cmp	eax, 24
	jz	convert24
	cmp	eax, 8
	jz	convert8
	cmp	eax, 4
	jz	convert4
	dec	eax
	jz	convert1
.ret:
	ret
convert24:
	mov	ecx, ebp
	rep	movsb
	sub	edi, [delta]
	mov	eax, ebp
	neg	eax
	and	eax, 3
	add	esi, eax
	dec	edx
	jnz	convert24
	ret
convert8:
	push	edi
	add	[esp], ebp
.loopi:
	xor	eax, eax
	lodsb
;        cmp     dword [I_END+30], 1
	push	eax
	mov	eax,[image_file]
	cmp	dword [eax+30],1
	pop	eax
	jnz	.nocompressed
.compressed:
	mov	ecx, eax
	jecxz	.special
	lodsb
	mov	eax, [ebx+eax*4]
@@:
	call	putpixel
	loop	@b
	jmp	.loopi
.nocompressed:
	mov	eax, [ebx+eax*4]
	call	putpixel
.loopicont:
	cmp	edi, [esp]
	jnz	.loopi
.next:
	pop	edi
	sub	edi, [delta]
	mov	eax, ebp
	and	eax, 3
	add	esi, eax
	dec	edx
	jnz	convert8
	ret
.special:
	lodsb
	test	al, al
	jz	.next
	cmp	al, 2
	jbe	.end
	mov	ecx, eax
	push	ecx
@@:
	xor	eax, eax
	lodsb
	mov	eax, [ebx+eax*4]
	call	putpixel
	loop	@b
	pop	ecx
	and	ecx, 1
	add	esi, ecx
	jmp	.loopi
.end:
	pop	edi
	ret
convert4:
	push	edi
	add	[esp], ebp
.loopi:
	xor	eax, eax
	lodsb
	shr	eax, 4
	mov	eax, [ebx+eax*4]
	call	putpixel
	cmp	edi, [esp]
	jz	.loopidone
	mov	al, [esi-1]
	and	eax, 0xF
	mov	eax, [ebx+eax*4]
	stosd
	dec	edi
	cmp	edi, [esp]
	jnz	.loopi
.loopidone:
	pop	edi
	sub	edi, [delta]
	call	align_input
	dec	edx
	jnz	convert4
	ret
convert1:
	push	edi
	add	[esp], ebp
.loopi:
	lodsb
	mov	ecx, 8
.loopii:
	add	al, al
	push	eax
	setc	al
	movzx	eax, al
	mov	eax, [ebx+eax*4]
	call	putpixel
	pop	eax
	cmp	edi, [esp]
	loopnz	.loopii
	jnz	.loopi
	pop	edi
	sub	edi, [delta]
	call	align_input
	dec	edx
	jnz	convert1
	ret

align_input:
	push	esi
	push	eax
	mov	eax,[image_file]
	sub	esi,eax    ;I_END
	sub	esi,[eax+10]
	pop	eax
	neg	esi
	and	esi, 3
	add	[esp], esi
	pop	esi
	ret

putpixel:
	push	eax
	stosw
	shr	eax, 16
	stosb
	pop	eax
	ret

;---------------------------------------------------------------------
get_window_param:
    mcall 9, procinfo, -1
    mov   eax,[ebx+46]
    mov   [window_high],eax
    mov   eax,[ebx+42]
    mov   [window_width],eax
;    mov   eax,[ebx+70]
;    mov   [window_status],eax
    mcall 48,4
    mov   [skin_high],eax
    ret
;---------------------------------------------------------------------

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:
    or	 [bWasDraw],1

    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,1			   ; 1, start of draw
    mcall

				   ; DRAW WINDOW
    xor  eax,eax		   ; function 0 : define and draw window
;    mov  ebx,350                   ; [x start] *65536 + [x size]
;    mov  ecx,400                   ; [y start] *65536 + [y size]
    mov  ebx,0*65536
    mov  ecx,0*65536
    add  ebx,[wnd_width]
    add  ecx,[wnd_height]
    mov  edx,0x43ffffff 	   ; color of work area RRGGBB,8->color gl
    mcall

    call get_window_param

    mov   ebx,5
    shl   ebx,16
    add   ebx,[window_width]
    sub   ebx,9
    push  ebx
    mov   ecx,[skin_high]
    shl   ecx,16
    add   ecx,50
    sub   ecx,[skin_high]
    mcall 13, , ,0xffffff
    mov  eax,[image_file]
    mov  ecx,[eax+22]
    mov  ebx,[eax+18]
    push ecx
    add  ebx,5
    mov  ax,bx
    shl  ebx,16
    add  ebx,[window_width]
    sub  ebx,4
    sub  bx,ax
    cmp  bx,0
    jb	 @f
    add  ecx,50 shl 16
    mcall 13, , ,0xffffff
@@:
    pop  ecx
    pop  ebx
    add  ecx,50
    mov  ax,cx
    shl  ecx,16
    add  ecx,[window_high]
    sub  cx,ax
    sub  ecx,4
    cmp  bx,0
    jb	 @f
    mcall 13, , ,0xffffff
@@:
    mov  eax,8
    mov  ebx,10*65536+46
    mov  ecx,25*65536+20
    mov  edx,2
    mov  esi,0x780078
  newbutton:
    mcall
    add  ebx,48*65536
    inc  edx
    cmp  edx,6
    jb	 newbutton

				   ; WINDOW LABEL
    mov  eax,4			   ; function 4 : write text to window
    mov  ebx,8*65536+8		   ; [x start] *65536 + [y start]
    mov  ecx,0x10ddeeff 	   ; font 1 & color ( 0xF0RRGGBB )
    mov  edx,labelt		   ; pointer to text beginning
    mov  esi,12 		   ; text length
    mcall

    mov  ebx,14*65536+32
    mov  edx,buttext
    mov  esi,26
    mcall

    call drawimage

    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,2			   ; 2, end of draw
    mcall

    ret


; DATA AREA

labelt:
	 db 'MeView v.0.4'

lsz buttext,\
    en,   ' FILE   OPEN   INFO   BGRD',\
    ru,   ' ФАЙЛ  ОТКР   ИНФО   ФОН  ',\
    de,   'DATEI OEFNEN  INFO   HGRD'

thread1:			; start of thread1

	  mcall 40, 0x27

    mov  esi,string
@@:
    cld
    lodsb
    test al,al
    jne  @r
    sub  esi,string
    mov  eax,esi
    dec  eax
    mov edi, edit1
    mov ed_size, eax
    mov ed_pos, eax
red1:
    call draw_window1

still1:

    mov  eax,10 		; wait here for event
    mcall

    cmp  eax,1			; redraw request ?
    je	 red1
    cmp  eax,2			; key in buffer ?
    je	 key1
    cmp  eax,3			; button in buffer ?
    je	 button1
	  mouse_edit_box name_editboxes
    jmp  still1

  key1: 			; key
    mcall 2
    cmp ah,13
    je	close1
    cmp ah,27
    je	close1
    key_edit_box name_editboxes
    jmp  still1

  button1:			; button
    mov  eax,17 		; get id
    mcall

    cmp  ah,1			; button id=1 ?
    jne  still1
  close1:
    bts  dword [status],2
    btr  dword [status],0
    mov  eax,-1 		; close this program
    mcall

    jmp  still1

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window1:


    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,1			   ; 1, start of draw
    mcall

				   ; DRAW WINDOW
    xor  eax,eax		   ; function 0 : define and draw window
    mov  ebx,100*65536+300	   ; [x start] *65536 + [x size]
    mov  ecx,100*65536+80	   ; [y start] *65536 + [y size]
    mov  edx,0x03780078 	   ; color of work area RRGGBB,8->color gl
    mcall

				   ; WINDOW LABEL
    mov  eax,4			   ; function 4 : write text to window
    mov  ebx,8*65536+8		   ; [x start] *65536 + [y start]
    mov  ecx,0x10ddeeff 	   ; font 1 & color ( 0xF0RRGGBB )
    mov  edx,labelt1		   ; pointer to text beginning
    mov  esi,labelt1.size	   ; text length
    mcall

;    call drawstring
	draw_edit_box name_editboxes

    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,2			   ; 2, end of draw
    mcall

    ret

 drawstring:
    pusha
    mov  eax,8		   ;invisible button
    mov  ebx,21*65536+258
    mov  ecx,40*65536+15
    mov  edx,0x60000002
    mcall

    mov  eax,13 	    ;bar
    mov  edx,0xe0e0e0
    mcall
    push eax		    ;cursor
    mov  eax,6*65536
    mul  dword [pos]
    add  eax,21*65536+6
    mov  ebx,eax
    pop  eax
    mov  edx,0x6a73d0
    mcall
    mov  eax,4		    ;path
    mov  ebx,21*65536+44
    xor  ecx,ecx
    mov  edx,string
    mov  esi,43
    mcall


    popa
    ret

; DATA AREA

lsz labelt1,\
   en,	'File',\
   ru,	'Файл',\
   de,	'Datei'

thread2:			  ; start of info thread

     call draw_window2

still2:

    mov  eax,10 		; wait here for event
    mcall

    cmp  eax,1			; redraw request ?
    je	 thread2
    cmp  eax,2			; key in buffer ?
    je	 close2
    cmp  eax,3			; button in buffer ?
    je	 button2

    jmp  still2

  button2:			 ; button
    mov  eax,17 		; get id
    mcall

    cmp  ah,1			; button id=1 ?
    jne  noclose2
  close2:
    btr dword [status],1
    bts dword [status],2
    mov  eax,-1 		; close this program
    mcall
  noclose2:

    jmp  still2




;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window2:


    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,1			   ; 1, start of draw
    mcall

				   ; DRAW WINDOW
    xor  eax,eax		   ; function 0 : define and draw window
    mov  ebx,100*65536+330	   ; [x start] *65536 + [x size]
    mov  ecx,100*65536+90	   ; [y start] *65536 + [y size]
    mov  edx,0x03780078 	   ; color of work area RRGGBB,8->color gl
    mcall

				   ; WINDOW LABEL
    mov  eax,4			   ; function 4 : write text to window
    mov  ebx,8*65536+8		   ; [x start] *65536 + [y start]
    mov  ecx,0x10ddeeff 	   ; font 1 & color ( 0xF0RRGGBB )
    mov  edx,labelt2		   ; pointer to text beginning
    mov  esi,labelt2.size	   ; text length
    mcall

    mov  ebx,10*65536+30
    mov  edx,string
    mov  esi,43
    mcall
    mov  edx,fitext
    mov  esi,14
    add  ebx,70*65536+10
 followstring:
    mcall
    add  ebx,10
    add  edx,esi
    cmp  ebx,80*65536+70
    jbe  followstring
    mov  eax,47
    mov  edx,200*65536+40
    mov  esi,ecx
    mov  ecx,[image_file]
    push ecx
    mov  ecx, [ecx+2]
    call digitcorrect
    mcall
    add  edx,10
    pop  ecx
    push ecx
    mov  ecx, [ecx+18]
    call digitcorrect
    mcall
    add  edx,10
    pop  ecx
    push ecx
    mov  ecx, [ecx+22]
    call digitcorrect
    mcall
    add  edx,10
    pop  ecx
    movzx ecx,word [ecx+28]
    call digitcorrect
    mcall

    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,2			   ; 2, end of draw
    mcall

    ret

 digitcorrect:
    xor  ebx,ebx
    mov  bh,6
    cmp  ecx,99999
    ja	 c_end
    dec  bh
    cmp  ecx,9999
    ja	 c_end
    dec  bh
    cmp  ecx,999
    ja	 c_end
    dec  bh
    cmp  ecx,99
    ja	 c_end
    dec  bh
    cmp  ecx,9
    ja	 c_end
    dec  bh
 c_end:
    bswap ebx
    ret


; DATA AREA

lsz labelt2,\
    en,   'File info',\
    ru,   'Информация о файле',\
    de,   'Dateiinfo'

lsz fitext,\
     en, 'FILE SIZE     ',\
     en, 'X SIZE        ',\
     en, 'Y SIZE        ',\
     en, 'BITS PER PIXEL',\
			  \
     ru, 'Размер файла  ',\
     ru, 'Ширина        ',\
     ru, 'Высота        ',\
     ru, 'Бит на пиксел ',\
			  \
     de, 'FATEIGROESSE  ',\
     de, 'X GROESSE     ',\
     de, 'Y GROESSE     ',\
     de, 'BITS PER PIXEL'

thread3:			  ; start of bgrd thread

     call draw_window3

still3:

    mov  eax,10 		; wait here for event
    mcall

    cmp  eax,1			; redraw request ?
    je	 thread3
    cmp  eax,2			; key in buffer ?
    je	 key3
    cmp  eax,3			; button in buffer ?
    je	 button3

    jmp  still3

  key3:
    mcall
    cmp  ah,27
    je	 close3
    cmp  ah,13
    je	 kok
    cmp  ah,178 ;up
    jne  nofup
    cmp  dword [bgrmode],1
    je	 fdn
  fup:
    dec dword [bgrmode]
    jmp  flagcont
  nofup:
    cmp  ah,177 ;down
    jne  still3
    cmp dword [bgrmode],2
    je	 fup
  fdn:
    inc dword [bgrmode]
    jmp  flagcont


  button3:			 ; button
    mov  eax,17 		; get id
    mcall

    cmp  ah,1			; button id=1 ?
    jne  noclose3
  close3:
    btr dword [status],3
    bts dword [status],2
    mov  eax,-1 		; close this program
    mcall
  noclose3:
    cmp  ah,4
    jne  nook
   kok:
    call background
    jmp  close3
  nook:
    cmp  ah,2
    jb	 still3
    cmp  ah,3
    ja	 still3
    dec  ah
    mov byte [bgrmode],ah
   flagcont:
    call drawflags
    jmp  still3




;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window3:


    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,1			   ; 1, start of draw
    mcall

				   ; DRAW WINDOW
    xor  eax,eax		   ; function 0 : define and draw window
    mov  ebx,100*65536+200	   ; [x start] *65536 + [x size]
    mov  ecx,100*65536+100	   ; [y start] *65536 + [y size]
    mov  edx,0x03780078 	   ; color of work area RRGGBB,8->color gl
    mcall

    mov  eax,8
    mov  ebx,70*65536+40
    mov  ecx,70*65536+20
    mov  edx,4
    mov  esi,0xac0000
    mcall

				   ; WINDOW LABEL
    mov  eax,4			   ; function 4 : write text to window
    mov  ebx,8*65536+8		   ; [x start] *65536 + [y start]
    mov  ecx,0x10ddeeff 	   ; font 1 & color ( 0xF0RRGGBB )
    mov  edx,labelt3		   ; pointer to text beginning
    mov  esi,labelt3.size	   ; text length
    mcall
    add  ebx,38*65536+20
    mov  ecx,0xddeeff
    mov  edx, bgrdtext
    mov  esi, bgrdtext.size
    mcall
    add  ebx,40*65536+15
    mov  edx, tiled
    mov  esi, tiled.size
    mcall
    add  ebx,15
    mov  edx, stretch
    mov  esi, stretch.size ;7
    mcall
    add  ebx,18
    mov  edx, ok_btn
    mov  esi, ok_btn.size ;2
    mcall

    call drawflags

    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,2			   ; 2, end of draw
    mcall

    ret

 drawflags:
    mov  eax,8
    mov  ebx,70*65536+10
    mov  ecx,40*65536+10
    mov  edx,2
    mov  esi,0xe0e0e0
    mcall
    add  ecx,15*65536
    inc  edx
    mcall
    mov  eax,4
    mov  ebx,73*65536+42
    xor  ecx,ecx
    mov  edx,vflag
    mov  esi,1
    cmp  dword [bgrmode],1
    je	 nodownflag
    add  ebx,15
 nodownflag:
    mcall
    ret


; DATA AREA
status	 dd 0  ;bit0=1 if file thread is created
soi	 dd 0
delta	dd	0
process  dd 0

bWasDraw db 0
vflag: db 'x'
bgrmode: dd 1

wnd_width dd 210
wnd_height dd 53

lsz labelt3,\
    en,   'Background set',\
    ru,   "Установка фона",\
    de,   'Hintergrund gesetzt'

lsz bgrdtext,\
    en, 'SET AS BACKGROUND:',\
    ru, 'Тип обоев:',\
    de, 'ALS HINTERGRUND'

lsz tiled,\
    en, 'TILED',\
    ru, 'замостить',\
    de, 'GEKACHELT'

lsz stretch,\
    en, 'STRETCH',\
    ru, 'растянуть',\
    de, 'GESTRECKT'

lsz ok_btn,\
    en, 'Ok',\
    ru, 'Ok',\
    de, 'Ok'

image_file dd 0
;image_file_size dd 0

pos: dd 6

window_high dd 0
window_width dd 0
skin_high dd 0
;---------------------------------------------------------------------
; for EDITBOX
name_editboxes:
edit1 edit_box 200,10,30,0xffffff,0xbbddff,0,0,0,255,string,ed_focus+ed_always_focus,0
name_editboxes_end:

mouse_flag: dd 0x0
;---------------------------------------------------------------------
fileinfo:
     dd 5
     dd 0
     dd 0
     dd 0		;x290000-I_END
     dd process_info	;I_END
string:
	db	'/sys/bgr.bmp',0

IM_END:
	rb	string+257-$

temp_area:
procinfo:
process_info:
rb 1024*4
rb 1024*2
;rb 0x10000
thread:
rb 512
I_END:
