		;;      rs780 internal GFX config space test
		;;	(C) art_zh 2010
		;;	<kolibri@jerdev.co.uk>


  use32 	     ;
  org	 0x0	     ;

  db	 'MENUET01'  ;
  dd	 0x01	     ;
  dd	 START	     ;
  dd	 I_END	     ;
  dd	 0x1000      ;
  dd	 0x1000      ;
  dd	 0x0	     ;
  dd	 0x0	     ;

include 'MACROS.INC' ;

PCIE_SPACE	equ	0xF0000000
PCIE_ADDR       equ     0xF0008000   ; bdf0:1.0  = internal GFX P2P bridge
BOX_COLOR	equ	0xE0D8D0

START:





red:

    call draw_window

still:
    mcall 10		; event waiting

    cmp  eax,1		; redraw window
    je	 red		;
    cmp  eax,2		; key pressed?
    je	 key		;
    cmp  eax,3		; button hit?
    je	 button 	;

    jmp  still		; none of that

;---------------------------------------------------------------------

 key:		       ; key pressed
   jmp	  red

;---------------------------------------------------------------------

  button:
    mcall 17		; get the button ID
    cmp   ah, 1
    jne   .bt2
    mcall -1
.bt2:
	cmp	ah, 2
	jne	.bt3
	sub	[Reg],4   ; Rg# decrement
	jmp	red
.bt3:
	cmp	ah, 3
	jne	.bt4
	add	[Reg],4   ; Rg# increment
	jmp	red
.bt4:
	cmp	ah, 4
	jne	.bt5
	add	[Reg],4*16	; PgDn
	jmp	red
.bt5:
	cmp	ah, 5
	jne	.bt6
	mov	edx, [Reg]
	cmp	edx, 4*16
	jb	@f
	sub	edx, 4*16
	mov	[Reg],edx      ; PgUp
	jmp	red
@@:
	xor	edx, edx
	mov	[Reg], edx
	jmp	red

.bt6:
	cmp	ah, 6
	jne	still
	mcall	37, 1		; get the mouse pointer
	shr	eax, 16 	; only X needed
	sub	eax, 124	; check the left border
	jb	red
	xor	edx, edx
	mov	ebx, 12
	div	ebx
	cmp	eax, 32 	; check the right border
	jnb	red
	mov	ecx, 31
	sub	ecx, eax	 ; reverse the bit order
	mov	ebx, [Rct]
	btc	ebx, ecx	; invert the bit
	mov	eax, [Reg]
	add	eax, PCIE_ADDR
	mov	[Rct], ebx
	mov	[eax], ebx

    jmp   red


;------------------------------------------------
print_config_reg:
;------------------------------------------------
	mov	eax, [reg]
	and	eax, 0x0FFC
	mov	ebx, 4*65536+256   ; 4 hex digits
	mov	ecx, eax
	mov	dx,[stX]
	shl	edx,16		   ; = X*65536
	mov	dx,[stY]	   ; = edx + Y
	mov	esi,0
	mcall 47		   ; print reg#
	mov	eax, [reg]
	add	eax, PCIE_ADDR
	mov	ecx, [eax]
	add	edx, 36*65536	   ; right column
	mov	ebx, 8*65536+256   ; 8 hex digits
	mcall 47		   ; print config data
	ret
;------------------------------------------------
read_nbconfig:
; in:  dl = reg#  |  out: eax = data
;------------------------------------------------
	mov	ebx, 0xF0000000
	and	edx, 0x0FC
	mov	eax, dword [ebx+edx]
	ret
;------------------------------------------------
write_nbconfig:
; in: dl = reg#; eax = data
;------------------------------------------------
	mov	ebx, 0xF0000000
	and	edx, 0x0FC
	mov	dword [ebx+edx], eax
	ret

;------------------------------------------------
;read_htiunbind:
; in:  [HTr] = reg#  |  out: eax = data
;------------------------------------------------
;        mov     dl,  0x94
;        mov     al,  byte[HTr]
;        and     eax, 0x07C
;        call    write_nbconfig
;        add     dl,  4
;        call    read_nbconfig
;        ret
;------------------------------------------------
;write_htiunbind:
; in:  [HTr] = reg#; ecx = data
;------------------------------------------------
;        mov     dl,  0x94
;        mov     al,  byte[Reg]
;        and     eax, 0x017C
;        call    write_nbconfig
;        add     dl,  4
;        mov     ecx, eax
;        call    write_nbconfig
;        sub     dl,  4
;        mov     eax, 0x0
;        call    write_nbconfig
;        ret







;------------------------------------------------
    draw_window:
;------------------------------------------------


    mcall 12, 1
    mcall 0, 600*65536+530, 120*65536+290, 0x1499AABB,,title
; -----------------------------------------------------------------
; BUTTONS:   Xleft    Xwid,  Ytop    Yheig
    mcall 8, 370*65536+ 40,  26*65536+ 18, 2, 0x99AABB ;  <<
    mcall  ,		  ,  51*65536+ 18, 3,	       ;  >>
    mcall  , 425*65536+ 90,  26*65536+ 18, 4,	       ; PCIe Cfg
    mcall  ,		  ,  51*65536+ 18, 5,	       ; NB config
    mcall  , 117*65536+400,  97*65536+ 40, 6,	       ; Bits

	mov	edx, [Reg]
	add	edx, PCIE_ADDR		; reading the current reg content
	mov	ecx, [edx]
	mov	[Rct], ecx

	mov	ebx, bitstr2
	inc	ebx
	mov	edx, [Rct]
	mov	ecx, 0x80000000
	xor	eax, eax
.stringtest:
	test	edx, ecx
	jz	@f
	mov	byte [ebx+eax*2],'I'	; bit dump
	jmp	.nextbit
@@:
	mov	byte [ebx+eax*2],'0'
.nextbit:
	inc	eax
	shr	ecx, 1
	jnz	.stringtest

; button txt:  X *65536+ Y
    mcall  4, 378*65536+32 ,0x10000000, butstr2,3
    mcall  ,  378*65536+57 ,	      , butstr3,
    mcall  ,  436*65536+32 ,	      , butstr4,9
    mcall  ,  436*65536+57 ,	      , butstr5,

    mcall  4, 122*65536+101,0	       , bitstr0,65
    mcall  ,  122*65536+110,0	       , bitstr1,65
    mcall  ,  122*65536+117,0	       , bitstr2,65
    mcall  ,  122*65536+126,0	       , bitstr3,65
; -----------------------------------------------------------------
;    draw the reg-value box
	mov	ebx, 10*65536+100	; Xleft | Xwidth
	mov	ecx, 26*65536+250	; Ytop  | Yheight
	mov	edx, BOX_COLOR
	mcall	13
;    draw the reg-address box
	mov	ebx, 206*65536+146	; Xleft | Xwidth
	mov	cx, 44			; Yheight only
	mcall	13
;    draw ZiS status box
;        mov     ebx, 206*65536+274      ; Xleft | Xwidth
;        mov     ecx,  84*65536+ 64      ; Ytop  | Yheight
;        mcall   13
;    draw the dump box
;        mov     ebx, 206*65536+274      ; Xleft | Xwidth
;        mov     ecx, 190*65536+232      ; Ytop  | Yheight
;        mcall   13

;    fill the data box
	mov	ebx, [Reg]
	mov	[reg],ebx
	mov	bx, 40	       ; upper position
	mov	[stY],bx
.print_reg_names:
	call	print_config_reg
	add	[stY],14
	add	[reg], 4
	mov	edx,[Reg]
	add	edx,16*4
	cmp	edx,[reg]
	ja	.print_reg_names

;   fill the status box
	mcall	4, 210*65536+30,0,str1,12
	mcall	,  210*65536+44, ,str2,
	mcall	,  210*65536+56, ,str3,
	mov	ecx, PCIE_ADDR
	mov	edx, 300*65536+30
	mov	ebx, 8*65536+256
	mcall	47
	add	dx, 14
	mov	ecx,[Reg]
	mov	esi, 0
	mcall	47
	add	dx,14
	mov	ecx, [Rct]
	mcall	47

;    print extra info
	mov	ebx, 120*65536+180
	xor	ecx, ecx
	mov	edx, info1
@@:
	mcall	4,,,,66
	add	edx, 66
	add	ebx, 14
	cmp	edx, info_end
	jb	@b


    mcall 12, 2 		   

ret


align 4
;-------------------------------------------------

  pix	dd  0x55AACC33
  pxX	dd  200
  pxY	dd  160
  stX	dw  18
  stY	dw  0
  reg	dd  0

  Rct	dd  0	     ; reg content
  Reg	dd  0x00     ; reg number


 title db '          RS780 GFX registers ',0
;------------------------------------------------------------------------------------
 reg_str   db	'Reg#| hex.Value  '
;------------------------------------------------------------------------------------
str1	db	'bdf address:'
str2	db	'Reg. number:'
str3	db	'Reg.content:'

 butstr2 db ' << '
 butstr3 db ' >> '
 butstr4 db 'Next Page'
 butstr5 db 'Prev Page'

bitstr0  db	'31',209,205,209,205,209,205,209,205,209,205,209,205,'24',\
		209,205,209,205,209,205,209,205,209,205,209,205,209,205,'16',\
		209,'15',205,209,205,209,205,209,205,209,205,209,205,209,'8',\
		205,'7',209,205,209,205,209,205,209,205,209,205,209,205,209,'0',184
bitstr1  db	179,' | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | ',179
bitstr2  db	179,'1 0 9 8',179,'7 6 5 4',179,'3 2 1 0',179,'9 8 7 6',\
		179,'5 4 3 2',179,'1 0 9 8',179,'7 6 5 4',179,'3 2 1 0',179
bitstr3  db	212,205,207,205,207,205,207,205,207,205,207,205,207,205,207,205,207,\
		205,207,205,207,205,207,205,207,205,207,205,207,205,207,205,207,205,207,\
		205,207,205,207,205,207,205,207,205,207,205,207,205,207,205,207,205,207,\
		205,207,205,207,205,207,205,207,205,207,205,190

info1	db	'--------------------------- extra info ---------------------------'
info2	db	'| reg 00[31:16] (DeviceID): 9602=int. GFX;  9603=PCIE GFX bridge |'
	db	'| reg                                                            |'
	db	'| reg                                                            |'
	db	'| reg                                                            |'
	db	'------------------------------------------------------------------'
info_end:

I_END:		; end of program

	rd 256

align 256
st_0:
