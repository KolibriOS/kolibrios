;---------------------------------------------------------------------
; Free3D version 0.6
;
; last update:  21/02/2011
; written by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      advanced control for mouse
;               advanced control for keyboard:
;               W,A,S,D adn Arrow UP,Down,Left,Right
;---------------------------------------------------------------------
; Free3D version 0.5
;
; last update:  20/02/2011
; written by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      PNG textures 128x128
;               using libraries cnv_png.obj and archiver.obj
;               using dinamically allocation of memory
;
;---------------------------------------------------------------------
;
;   Fisheye Raycasting Engine Etc. FREE3D for MENUETOS by Dieter Marfurt
;   Version 0.4 (requires some texture-files to compile (see Data Section))
;   dietermarfurt@angelfire.com - www.melog.ch/mos_pub/
;   Don't hit me - I'm an ASM-Newbie... since years :)
;
;   Compile with FASM for Menuet (requires .INC files - see DATA Section)
;
;   Willow - greatly srinked code size by using GIF texture and FPU to calculate sine table
;
;   !!!! Don't use GIF_LITE.INC in your apps - it's modified for FREE3D !!!!
;
;   Heavyiron - new 0-function of drawing window from kolibri (do not work correctly with menuet)

TEX_SIZE	equ	128*128*4	;64*64*4	;

ICON_SIZE_X	equ	128	;64
ICON_SIZE_Y	equ	128	;64

Floors_Height	equ	32000
;---------------------------------------------------------------------
use32
org	0x0
	db 'MENUET01'	; 8 byte id
	dd 0x01		; header version
	dd START	; start of code
	dd IM_END	; size of image
	dd I_END	; 0x100000 ; memory for app
	dd stacktop	; 0x100000 ; esp
	dd 0x0
	dd path
;---------------------------------------------------------------------
include '../../../macros.inc'
;include	'macros.inc'
include '../../../KOSfuncs.inc'
include '../../../load_lib.mac'
;include 'load_lib.mac'
@use_library
;---------------------------------------------------------------------
START:	; start of execution
	mcall	68,11
	mcall	66,1,1
	mcall	40,0x27

	mcall	9,procinfo,-1
	mov	ecx,[ebx+30]	; PID
	mcall	18,21
	mov	[active_process],eax	; WINDOW SLOT

load_libraries	l_libs_start,end_l_libs
	test	eax,eax
	jnz	finish

; unpack deflate
	mov	eax,[unpack_DeflateUnpack2]
	mov	[deflate_unpack],eax

	call	load_icons
	call	convert_icons

	mov	esi,sinus
	mov	ecx,360*10
	fninit
	fld	[sindegree]
;--------------------------------------	
.sinlp:
	fst	st1
	fsin
	fmul	[sindiv]
	fistp	dword[esi]
	add	esi,4
	fadd	[sininc]
	loop	.sinlp
;---------------------------------------------------------------------
	call	cursor_to_screen_center
	call	set_new_cursor_skin
;---------------------------------------------------------------------	
align	4
red:	; redraw
	call	draw_window
	call	draw_stuff
;---------------------------------------------------------------------
align	4
still:
	mcall	10	; ask no wait for full speed

	cmp	eax,1	; window redraw request ?
	je	red

	cmp	eax,2	; key in buffer ?
	je	key

	cmp	eax,3	; button in buffer ?
	je	button

	cmp	eax,6
	jne	still
;---------------------------------------------------------------------
mouse:
	mcall	18,7
	cmp	[active_process],eax
	jne	still

	mcall	37,1

	xor	ebx,ebx
	mov	bx,ax  ; EBX mouse y
	shr	eax,16 ; EAX mouse x

	mov	ecx,[mouse_position_old]
	xor	edx,edx
	mov	dx,cx  ; EDX mouse y old
	shr	ecx,16 ; ECX mouse x old
	
	cmp	eax,ecx
	je	.y	;still
	ja	.turn_left
;---------------------------------------------------------------------
.turn_right:
	xchg	eax,ecx
	sub	eax,ecx
	mov	edi,[vheading]
	add	edi,eax
	jmp	@f
;---------------------------------------------------------------------
.turn_left:
	sub	eax,ecx
	mov	edi,[vheading]
	sub	edi,eax
;--------------------------------------
@@:
	call	check_range
;---------------------------------------------------------------------
.y:
	cmp	ebx,edx
	je	.red
	ja	.walk_down
;--------------------------------------
.walk_up:	
	sub	edx,ebx
	mov	ecx,edx
	call	prepare_2
	jz	.1
;--------------------------------------	
	add	eax,edi	; newPx
	add	ebx,esi	; newPy
	jmp	.1
;---------------------------------------------------------------------
.walk_down:
	sub	ebx,edx
	mov	ecx,ebx
	call	prepare_2
	jz	.1
	
	sub	eax,edi	; newPx
	sub	ebx,esi	; newPy
;--------------------------------------
.1:
	mov	edi,eax	; newPx / ffff
	mov	esi,ebx	; newPy / ffff
	sar	edi,16
	sar	esi,16
	mov	ecx,esi
	sal	ecx,5
	lea	ecx,[grid+ecx+edi]
	cmp	[ecx],byte 0
	je	@f
	
	call	cursor_to_screen_center	
	jmp	still	;cannotwalk
;---------------------------------------------------------------------
@@:
	mov	[vpx],eax
	mov	[vpy],ebx
;--------------------------------------
.red:
	call	cursor_to_screen_center
	jmp	red
;---------------------------------------------------------------------
align	4
prepare_2:
	shr	ecx,4
	push	ecx
	call	prepare_1
	pop	ecx
	cmp	ecx,3
	jb	@f
	mov	ecx,3
@@:
	shl	edi,cl
	shl	esi,cl
	test	ecx,ecx
	ret
;---------------------------------------------------------------------
align	4
check_range:
	cmp	edi,0
	jge	@f

	mov	edi,3600
	jmp	.store
;--------------------------------------
@@:
	cmp	edi,3600
	jle	@f

	xor	edi,edi
;--------------------------------------	
@@:
.store:
	mov	[vheading],edi
	ret
;---------------------------------------------------------------------
align	4
cursor_to_screen_center:
	mcall	18,15
	mcall	37,1
	mov	[mouse_position_old],eax
	ret
;---------------------------------------------------------------------
set_new_cursor_skin:
	mcall	68,12,32*32*4
	mov	ecx,eax
	mcall	37,4,,2
	mov	ecx,eax
	mcall	37,5
	ret
;---------------------------------------------------------------------
align	4
key:	; key
	mcall	2
	cmp	[extended_key],1
	je	.extended_key
	test	al, al
	jnz	still

	cmp	ah, 0xE0
	jne	@f

	mov	[extended_key],1
	jmp	still
;---------------------------------------------------------------------
@@:
	cmp	ah,1	; Esc
	je	finish

	cmp	ah,17 ; W up
	je	s_up

	cmp	ah,31 ; S down
	je	s_down

	cmp	ah,30 ; A left
	je	w_left	;s_left

	cmp	ah,32 ; D right
	je	w_right	;s_right

	jmp	still
;---------------------------------------------------------------------
.extended_key:
	mov	[extended_key],0
	mov	[current_key_code],ah

	cmp	ah,27	; esc=End App
	je	finish

	cmp	ah,72 ; up arrow
	je	s_up

	cmp	ah,80 ; down arrow
	je	s_down

	cmp	ah,75 ; left arrow
	je	s_left

	cmp	ah,77 ; right arrow
	je	s_right

	jmp	still
;---------------------------------------------------------------------
align	4
smart_clr_key_buf:
	mov	al,[old_key_code]
	mov	ah,[current_key_code]
	mov	[old_key_code],ah
	cmp	al,ah
	jne	.end
;--------------------------------------
.still:
	mcall	2
	cmp	[extended_key],1
	je	.extended_key

	test	al, al
	jnz	.end

	cmp	ah, 0xE0
	jne	.end

	mov	[extended_key],1
	jmp	.still
.end:
	call	draw_stuff
	jmp	still
;---------------------------------------------------------------------
.extended_key:
	mov  [extended_key],0
	mov  [current_key_code],ah
	jmp  smart_clr_key_buf
;---------------------------------------------------------------------
align	4
w_left:		; walk left
	call	prepare_1
	add	eax,esi	; newPx
	sub	ebx,edi	; newPy
	jmp	s_down.1
;---------------------------------------------------------------------
align	4
w_right:	; walk right
	call	prepare_1
	sub	eax,esi	; newPx
	add	ebx,edi	; newPy
	jmp	s_down.1
;---------------------------------------------------------------------
align	4
s_up:	; walk forward (key or mouse)
	call	prepare_1
;	sal	esi,1	; edit walking speed here
;	sal	edi,1
	add	eax,edi	; newPx
	add	ebx,esi	; newPy
	jmp	s_down.1
;---------------------------------------------------------------------
align	4
s_down:	; walk	backward
	call	prepare_1
;	sal	esi,1	; edit walking speed here
;	sal	edi,1
	sub	eax,edi	; newPx
	sub	ebx,esi	; newPy
.1:
	mov	edi,eax	; newPx / ffff
	mov	esi,ebx	; newPy / ffff
	sar	edi,16
	sar	esi,16
	mov	ecx,esi
	sal	ecx,5
	lea	ecx,[grid+ecx+edi]
	cmp	[ecx],byte 0
	jne	smart_clr_key_buf	;cannotwalk

	mov	[vpx],eax
	mov	[vpy],ebx
	jmp	smart_clr_key_buf
;---------------------------------------------------------------------
align	4
prepare_1:
	mov	eax,[vpx]
	mov	ebx,[vpy]
	mov	ecx,[vheading]
	mov	edx,ecx
	mov	edi,[sinus+ecx*4]
	lea	edx,[sinus+3600+edx*4]
	cmp	edx,eosinus	; cosinus taken from (sinus plus 900) mod 3600
	jb	@f

	sub	edx,14400
;--------------------------------------
@@:
	mov	esi,[edx]
	ret
;---------------------------------------------------------------------
align	4
s_left:		; turn	left	(key)
	mov	edi,[vheading]
	add	edi,50
	jmp	s_right.1
;---------------------------------------------------------------------
align	4
s_right:	; turn	right
	mov	edi,[vheading]
	sub	edi,50
.1:
	call	check_range
	jmp	smart_clr_key_buf
;---------------------------------------------------------------------
align	4
button:	; button
	mcall	17
	cmp	ah,1	; button id=1 ?
	jne	still	;gamestart
;--------------------------------------
finish:
	mcall	-1	; close this program
;---------------------------------------------------------------------
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
align	4
draw_window:
	mcall	12,1
	
	mcall	0,<50,649>,<50,484>,0x01000000,0x01000000,0x01000000

	mcall	12,2
	ret
;---------------------------------------------------------------------
;   *********************************************
;   *******       COMPUTE 3D-VIEW        ********
;   *********************************************
align	4
draw_stuff:
	mov	[step1],dword 1
;	mov	[step64],dword 64
	mov	esi,[vheading]
	add	esi,320
	mov	[va],esi
	mov	eax,[vheading]
	sub	eax,320
	mov	[vacompare],eax
;------------------------------------ CAST 640 PIXEL COLUMNS ---------------
; FOR A=320+heading to -319+heading step -1 (a is stored in [va])
;---------------------------------------------------------------------------
;	mov	edx,5
	mov	[vx1],dword 0	;5 ;edx ; init x1 ... pixelcolumn
;--------------------------------------
align	4
for_a:
	mov	edx,[vx1]
	mov	[vx1b],edx
	sub	[vx1b],dword 320
	mov	edx,[va]	; a2
	cmp	edx,-1	; a2 is a mod 3600
	jg	ok1

	add	edx,3600
;--------------------------------------
ok1:
	cmp	edx,3600
	jb	ok2

	sub	edx,3600
;--------------------------------------
ok2:
; get stepx and stepy
	; pointer to stepx
	lea	ecx,[sinus+edx*4]
	mov	esi,[ecx]
	sar	esi,4	; accuracy
	mov	[vstepx],esi	; store stepx
	lea	esi,[sinus+3600+edx*4]
	cmp	esi,eosinus	;cosinus taken from ((sinus plus 900) mod 3600)
	jb	ok202

	sub	esi,14400
;--------------------------------------
ok202:
	mov	ecx,[esi]
	sar	ecx,4
	mov	[vstepy],ecx	; store	stepy

	mov	eax,[vpx]	; get Camera Position
	mov	ebx,[vpy]
	mov	[vxx],eax	; init caster position
	mov	[vyy],ebx

	mov	edi,0	; init L (number of raycsting-steps)
	mov	[step1],dword 1	; init Caster stepwidth for L.
;--------------------------------------
;  raycast a pixel column
align	4
raycast:
	add	edi,[step1]	; count caster steps
	jmp	nodouble	; use this to prevent blinking/wobbling textures: much slower!

;	cmp	edi,32
;	je	double
	
;	cmp	edi,512
;	je	double
	
;	cmp	edi,1024
;	je	double
	
;	jmp	nodouble
;---------------------------------------------------------------------
;double:
;	mov	edx,[step1]
;	sal	edx,1
;	mov	[step1],edx

;	mov	edx,[vstepx]
;	sal	edx,1
;	mov	[vstepx],edx

;	mov	edx,[vstepy]
;	sal	edx,1
;	mov	[vstepy],edx
;--------------------------------------
nodouble:
	mov	eax,Floors_Height	;32000	; 3600 ; determine Floors Height based on distance
	xor	edx,edx
	mov	ebx,edi

	div	ebx

	shl	edx,1
	cmp	ebx,edx
	jae	@f
	inc	eax
@@:

	mov	esi,eax
	mov	[vdd],esi
	mov	edx,260
	sub	edx,esi
	mov	[vh],edx

	cmp	edx,22
	jb	no_nu_pixel

	cmp	edx,259
	jg	no_nu_pixel	; draw only new pixels

	cmp	edx,[h_old]
	je	no_nu_pixel

	mov	eax,[vxx]	; calc floor pixel
	mov	ebx,[vyy]

	and	eax,0x0000FFFF
	and	ebx,0x0000FFFF

	shr	eax,10
	shr	ebx,10	; pixel coords inside Texture x,y 64*64
	mov	[xfrac],eax
	mov	[yfrac],ebx
; plot floor pixel !!!!
	mov	[vl],edi	; save L
	mov	[ytemp],esi	; remember L bzw. H

	mov	edi,[yfrac]	; get pixel color of this floor pixel
	sal	edi,9	;8 - for 64x64
	mov	esi,[xfrac]
	sal	esi,3	;2 - for 64x64
; in fact its floor, just using the wall texture :)
;	lea	edi,[wall+edi+esi]
	add	edi,[wall1]
	add	edi,esi
	mov	edx,[edi]
	mov	[remesi],esi
;**** calculate pixel adress:****
	mov	esi,[ytemp]
	add	esi,240
	imul	esi,1920
	mov	eax,[vx1]
	lea	eax,[eax+eax*2]
	lea	esi,[screen_buffer+eax+esi]

	cmp	esi,screen_buffer+1920*480
	jg	foff0

	cmp	esi,screen_buffer
	jb	foff0
; now we have the adress of the floor-pixel color in edi
; and the adress of the pixel in the image in esi
	mov	edx,[edi]
;******************** custom distance DARKEN Floor
	mov	eax,[vdd]
;	jmp	nodark0	; use this to deactivate darkening floor (a bit faster)

	cmp	eax,80
	jg	nodark0
; split	rgb
	mov	[blue_color],edx
	and	[blue_color],dword 255

	shr	edx,8
	mov	[green_color],edx
	and	[green_color],dword 255

	shr	edx,8
	mov	[red_color],edx
	and	[red_color],dword 255

	mov	eax,81	; darkness parameter
	sub	eax,[vdd]
	sal	eax,1
; reduce rgb
	sub	[red_color],eax
	cmp	[red_color],dword 0
	jg	notblack10

	mov	[red_color],dword 0
;--------------------------------------
notblack10:
	sub	[green_color],eax
	cmp	[green_color],dword 0
	jg	notblack20

	mov	[green_color],dword 0
;--------------------------------------
notblack20:
	mov	edx,[blue_color]
	sub	[blue_color],eax
	cmp	[blue_color],dword 0
	jg	notblack30

	mov	[blue_color],dword 0
;--------------------------------------
notblack30:
	shl	dword [red_color],16	; reassemble rgb
	shl	dword [green_color],8
	mov	edx,[red_color]
	or	edx,[green_color]
	or	edx,[blue_color]
;--------------------------------------
nodark0:
;   eo custom darken floor
	mov	eax,edx

;	cmp	esi,screen_buffer+1920*480
;	ja	foff0

;	cmp	esi,screen_buffer
;	jb	foff0

	mov	[esi],eax	; actually draw the floor pixel
; paint "forgotten" pixels
	mov	edx,[lasty]
	sub	edx,1920
	cmp	esi,edx
	je	foff0

	mov	[esi+1920],eax
	sub	edx,1920
	cmp	esi,edx
	je	foff0

	mov	[edx+1920],eax
	sub	edx,1920
	cmp	esi,edx
	je	foff0
	
	mov	[edx+1920],eax
;--------------------------------------
align	4
foff0:
	mov	[lasty],esi
;**** end of draw floor pixel ****
	mov	esi,[remesi]
	mov	edi,[vl]	; restore L
;--------------------------------------
no_nu_pixel:
	mov	esi,[vh]
	mov	[h_old],esi

	mov	eax,[vxx]
	mov	ebx,[vyy]

	add	eax,[vstepx]	; casting...
	add	ebx,[vstepy]

	mov	[vxx],eax
	mov	[vyy],ebx

	sar	eax,16
	sar	ebx,16

	mov	[vpxi],eax	; casters position in Map Grid
	mov	[vpyi],ebx

	mov	edx,ebx
	shl	edx,5
	lea	edx,[grid+edx+eax]

	cmp	[edx],byte 0	; raycaster reached a wall? (0=no)
	jne	getout

	cmp	edi,10000	; limit view range
	jb	raycast
;--------------------------------------
getout:
	mov	eax,[edx]	; store Grid Wall Value for Texture Selection
	mov	[vk],eax
	call	blur	; deactivate this (blurs the near floor) : a bit faster

; simply copy floor to ceil pixel column here
;	jmp	nocopy	; use this for test purposes
	pusha
	mov	eax,screen_buffer+1920*240
	mov	ebx,eax	;screen_buffer+1920*240
;--------------------------------------
align	4
copyfloor:
	sub	eax,1920
	add	ebx,1920
	mov	ecx,[vx1]
	lea	ecx,[ecx+ecx*2]
	lea	edx,[ecx+ebx]
	add	ecx,eax
	mov	esi,[edx]
	mov	[ecx],esi
	cmp	eax,screen_buffer
	jg	copyfloor
;@@:
	popa
; *** end of copy floor to ceil
;nocopy:
;--------------------------------------
; draw this pixelrows wall
	mov	[vl],edi
	mov	edi,260
	sub	edi,[vdd]
	cmp	edi,0
	jg	ok3
	
	xor	edi,edi
;--------------------------------------
ok3:
	mov	[vbottom],edi	; end wall ceil (or window top)
	mov	esi,262
	add	esi,[vdd]	; start wall floor
	xor	edi,edi
; somethin is wrong with xfrac,so recalc...
	mov	eax,[vxx]
	and	eax,0x0000FFFF
	shr	eax,10
	mov	[xfrac],eax

	mov	eax,[vyy]
	and	eax,0x0000FFFF
	shr	eax,10
	mov	[yfrac],eax
;--------------------------------------
pixelrow:
; find each pixels color:
	add	edi,ICON_SIZE_Y
	sub	esi,1
	cmp	esi,502		; dont calc offscreen-pixels
	jg	speedup

	xor	edx,edx
	mov	eax,edi
	mov	ebx,[vdd]
;	add	ebx,ebx
	shl	ebx,1
	div	ebx

	shl	edx,1
	cmp	ebx,edx
	jae	@f
	inc	eax
@@:
	and	eax,ICON_SIZE_Y-1
	mov	[ytemp],eax	; get y of texture for wall

	mov	eax,[xfrac]
	add	eax,[yfrac]
	and	eax,ICON_SIZE_X-1
	mov	[xtemp],eax	; get x of texture for wall
; now prepare to plot that wall-pixel...
	mov	[remedi],edi
	mov	edi,[ytemp]
	sal	edi,9	;8 - for 64x64
	mov	edx,[xtemp]
	sal	edx,3	;2 - for 64x64
	add	edi,edx
	mov	eax,[vk]	; determine which texture should be used
	and	eax,255

	cmp	eax,1
	jne	checkmore1

	add	edi,[wall0]	;ceil
	jmp	foundtex
;---------------------------------------------------------------------
align	4
checkmore1:
	cmp	eax,2
	jne	checkmore2

	add	edi,[wall1]
	jmp	foundtex
;---------------------------------------------------------------------
align	4
checkmore2:
	cmp	eax,3
	jne	checkmore3

	add	edi,[wall2]
	jmp	foundtex
;---------------------------------------------------------------------
align	4
checkmore3:
	cmp	eax,4
	jne	checkmore4

	add	edi,[wall3]
	jmp	foundtex
;---------------------------------------------------------------------
align	4
checkmore4:
	cmp	eax,5
	jne	checkmore5

	add	edi,[wall4]
	jmp	foundtex
;---------------------------------------------------------------------
align	4
checkmore5:
	cmp	eax,6
	jne	checkmore6

	add	edi,[wall5]
	jmp	foundtex
;---------------------------------------------------------------------
align	4
checkmore6:
	cmp	eax,7
	jne	checkmore7

	add	edi,[wall6]
	jmp	foundtex
;---------------------------------------------------------------------
align	4
checkmore7:
	cmp	eax,8
	jne	checkmore8

	add	edi,[wall7]
	jmp	foundtex
;---------------------------------------------------------------------
align	4
checkmore8:
foundtex:
	mov	edx,[edi]	; get pixel color inside texture
; ***pseudoshade south-west
;	jmp	east	; activate this for southwest pseudoshade : a bit slower + blink-bug
;---------------------------------------------------------------------
;	mov	edi,[yfrac]
;	mov	[pseudo],dword 0	; store flag for custom distance darkening
;	cmp	edi,[xfrac]
;	jge	east

;	and	edx,0x00FEFEFE
;	shr	edx,1
;	mov	[pseudo],dword 1
;--------------------------------------
east:
	call	dark_distance	; deactivate wall distance darkening: a bit faster
; ******* DRAW WALL PIXEL *******
	mov	eax,esi
	lea	eax,[esi-22]
	imul	eax,1920
	mov	ebx,[vx1]
	lea	ebx,[ebx+ebx*2]
	lea	eax,[eax+screen_buffer+ebx]

	cmp	eax,screen_buffer+1920*480
	ja	dont_draw

	cmp	eax,screen_buffer
	jb	dont_draw

	mov	[eax],edx	; actually set the pixel in the image
;--------------------------------------
; *** eo draw wall pixel
dont_draw:
	mov	edi,[remedi]
;--------------------------------------
speedup:
	cmp	esi,[vbottom]	; end of this column?
	jg	pixelrow

	mov	edi,[vl]	; restoring
	mov	eax,[vx1]	; inc X1
	add	eax,1
	mov	[vx1],eax
;*** NEXT A ***
	mov	esi,[va]
	sub	esi,1
	mov	[va],esi
	cmp	esi,[vacompare]
	jg	for_a
;*** EO NEXT A ***
;--------------------------------------
; **** put image !!!!!****
	xor	edx,edx
	mcall	7,screen_buffer,<640,480>
	ret
;---------------------------------------------------------------------
align	4
blur:
	pusha
	mov	eax,screen_buffer+360*1920
;--------------------------------------
align	4
copyfloor2:
	add	eax,1920
	mov	ebx,[vx1]
	lea	ebx,[ebx+ebx*2]
	add	ebx,eax
	
	mov	ecx,[ebx-15]
	and	ecx,0x00FEFEFE
	shr	ecx,1
	
	mov	edx,[ebx-12]
	and	edx,0x00FEFEFE
	shr	edx,1
	add	edx,ecx
	and	edx,0x00FEFEFE
	shr	edx,1

	mov	ecx,[ebx-9]
	and	ecx,0x00FEFEFE
	shr	ecx,1
	add	edx,ecx

	and	edx,0x00FEFEFE
	shr	edx,1

	mov	ecx,[ebx-6]
	and	ecx,0x00FEFEFE
	shr	ecx,1
	add	edx,ecx

	and	edx,0x00FEFEFE
	shr	edx,1

	mov	ecx,[ebx-3]
	and	ecx,0x00FEFEFE
	shr	ecx,1
	add	edx,ecx

	and	edx,0x00FEFEFE
	shr	edx,1

	mov	ecx,[ebx]
	and	ecx,0x00FEFEFE
	shr	ecx,1
	add	edx,ecx

	mov	[ebx],edx

	cmp	eax,screen_buffer+478*1920
	jb	copyfloor2

	popa
	ret
;---------------------------------------------------------------------
; ******* Darken by Distance *******
align	4
dark_distance:
; color must be in edx, wall height in [vdd]
	mov	eax,[vdd]
	cmp	eax,50
	jg	nodark
; split rgb
	mov	[blue_color],edx
	and	[blue_color],dword 255

	shr	edx,8
	mov	[green_color],edx
	and	[green_color],dword 255

	shr	edx,8
	mov	[red_color],edx
	and	[red_color],dword 255

	mov	eax,51	; darkness parameter
	sub	eax,[vdd]
	cmp	[pseudo],dword 1
	je	isdarkside

	sal	eax,2
;--------------------------------------
align	4
isdarkside:
; reduce rgb
	sub	[red_color],eax
	cmp	[red_color],dword 0
	jg	notblack10b

	mov	[red_color],dword 0
;--------------------------------------
align	4
notblack10b:
	sub	[green_color],eax
	cmp	[green_color],dword 0
	jg	notblack20b

	mov	[green_color],dword 0
;--------------------------------------
align	4
notblack20b:
	mov	edx,[blue_color]
	sub	[blue_color],eax
	cmp	[blue_color],dword 0
	jg	notblack30b

	mov	[blue_color],dword 0
;--------------------------------------	
align	4
notblack30b:
	shl	dword [red_color],16	; reassemble rgb
	shl	dword [green_color],8
	mov	edx,[red_color]
	or	edx,[green_color]
	or	edx,[blue_color]
	mov	eax,edx
;--------------------------------------
align	4
nodark:
	ret
;---------------------------------------------------------------------
load_icons:
	mov	ebx,icons_file_name
	mov	esi,path
	mov	edi,file_name
	call	copy_file_path

	mov	[fileinfo.subfunction],dword 5
	mov	[fileinfo.size],dword 0
	mov	[fileinfo.return],dword file_info
	mcall	70,fileinfo
	test	eax,eax
	jnz	.error

	mov	[fileinfo.subfunction],dword 0

	mov	ecx,[file_info+32]
	mov	[fileinfo.size],ecx
	mov	[img_size],ecx
	
	mcall	68,12
	test	eax,eax
	jz	finish	;memory_get_error

	mov	[fileinfo.return],eax
	mov	[image_file],eax

	mcall	70,fileinfo
	test	eax,eax
	jnz	.error
	ret
.error:
;	mov	[N_error],2
;	mov	[error_type],eax
	jmp	finish
;---------------------------------------------------------------------
copy_file_path:
	xor	eax,eax
	cld
@@:
	lodsb
	stosb
	test	eax,eax
	jnz	@b
	mov	esi,edi
	dec	esi
	std
@@:
	lodsb
	cmp	al,'/'
	jnz	@b
	mov	edi,esi
	add	edi,2
	mov	esi,ebx
	cld
@@:
	lodsb
	stosb
	test	eax,eax
	jnz	@b
	ret
;---------------------------------------------------------------------
convert_icons:
	xor	eax,eax
	mov	[return_code],eax
	push	image_file
	call	[cnv_png_import.Start]

	mov	ecx,[image_file]
	mcall	68,13,
	test	eax,eax
	jz	finish	;memory_free_error

	cmp	[return_code],dword 0
;	je	@f
;	mov	[N_error],6
;	jmp	button.exit
;@@:
	jne	finish

	mcall	68,20,ICON_SIZE_X*ICON_SIZE_Y*4*8+44,[raw_pointer]
	mov	[raw_pointer],eax

	mov	ebx,[raw_pointer]
; set RAW area for icon
	mov	eax,[ebx+28]
	add	eax,ebx
	mov	edi,eax
	mov	esi,eax
	add	esi,ICON_SIZE_X*ICON_SIZE_Y*8*3-1
	add	edi,ICON_SIZE_X*ICON_SIZE_Y*8*4-4

;	add	eax,TEX_SIZE
	mov	[wall0],eax
	add	eax,TEX_SIZE
	mov	[wall1],eax
	add	eax,TEX_SIZE
	mov	[wall2],eax
	add	eax,TEX_SIZE
	mov	[wall3],eax
	add	eax,TEX_SIZE
	mov	[wall4],eax
	add	eax,TEX_SIZE
	mov	[wall5],eax
	add	eax,TEX_SIZE
	mov	[wall6],eax
	add	eax,TEX_SIZE
	mov	[wall7],eax
	add	eax,TEX_SIZE
; conversion 24b to 32 b
	mov	ecx,ICON_SIZE_X*ICON_SIZE_Y*8
	std
@@:
	xor	eax,eax
	lodsb
	rol	eax,8
	lodsb
	rol	eax,8
	lodsb
;	ror	eax,16
	stosd
	dec	ecx
	jnz	@b
	cld
	ret
;---------------------------------------------------------------------
; DATA AREA
;ceil=ceil
;wall=wall floor
;2 corner stone
;3 leaf mosaic
;4 closed window
;5 greek mosaic
;6 old street stones
;7 maya wall
;---------------------------------------------------------------------
align	4
grid:	; 32*32 Blocks, Map: 0 = Air, 1 to 8 = Wall
db 2,1,2,1,2,1,2,1,2,1,2,1,1,1,1,1,1,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
db 1,0,0,0,1,0,0,0,0,0,0,3,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,8,8
db 5,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8
db 1,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,1,0,0,0,0,3,3,3,3,0,0,0,0,0,0,8
db 5,0,1,2,3,4,5,6,7,8,2,1,3,3,3,0,5,0,2,1,2,3,0,0,0,0,0,0,0,0,0,8
db 1,0,0,0,0,0,0,0,0,0,2,3,0,0,0,0,5,0,0,0,0,3,0,0,0,0,0,0,0,0,0,8
db 5,0,0,0,1,0,0,4,0,0,0,1,0,0,0,0,5,0,0,0,0,3,3,0,3,3,0,0,0,0,0,8
db 1,1,0,1,1,1,1,4,1,0,1,3,0,0,0,0,5,2,1,2,0,3,0,0,0,3,0,0,0,0,0,8
db 5,0,0,0,1,0,0,0,0,0,0,1,0,3,3,3,5,0,0,0,0,3,0,0,0,3,0,0,0,0,0,8
db 1,0,0,0,1,0,0,5,0,0,0,0,0,0,0,0,5,0,0,0,0,0,0,0,0,3,0,0,0,0,0,8
db 5,0,0,0,0,0,0,5,0,0,0,1,0,0,0,0,5,0,0,0,0,3,0,0,0,0,0,0,0,0,0,8
db 1,4,4,4,4,4,4,4,4,4,4,3,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,8,8
db 2,2,2,2,2,2,8,8,8,8,8,8,8,8,8,0,0,0,6,6,0,7,7,7,7,7,7,7,7,7,8,8
db 1,0,0,0,1,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,1
db 5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,0,2,2,2,2,0,0,0,0,3,3,3,3,3,1
db 1,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,6,0,0,0,0,2,0,0,0,0,3,0,0,0,0,1
db 5,0,2,3,2,3,2,3,2,3,2,1,0,0,0,0,6,0,2,2,0,2,0,0,0,0,3,0,5,5,0,1
db 1,0,0,0,0,0,0,4,0,0,0,3,0,0,0,0,6,0,0,2,0,2,0,2,0,0,3,0,0,0,0,1
db 5,0,0,0,1,0,0,4,0,0,0,1,0,0,0,0,6,0,0,2,2,2,0,2,0,0,3,3,3,3,0,1
db 1,1,0,1,1,1,1,4,1,0,1,3,7,7,7,0,6,0,0,0,0,0,0,2,0,0,0,0,0,3,0,1
db 5,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,6,0,0,0,0,2,2,2,0,0,0,0,0,3,0,1
db 1,0,0,0,1,0,0,5,0,0,0,3,0,0,0,0,6,0,0,0,0,2,0,0,0,0,0,0,0,0,0,1
db 5,0,0,0,0,0,0,5,0,0,0,1,0,0,0,0,6,0,5,1,0,2,0,0,4,4,0,4,4,0,0,1
db 1,4,1,4,1,4,1,4,1,4,1,3,0,0,0,0,6,0,0,5,0,2,0,0,0,4,0,4,0,0,0,1
db 1,0,0,0,0,0,0,4,0,0,0,3,0,3,3,3,6,0,0,1,0,1,0,0,4,4,0,4,4,0,0,1
db 5,0,0,0,1,0,0,4,0,0,0,1,0,0,0,0,6,0,0,5,0,1,0,4,4,0,0,0,4,4,0,1
db 1,1,0,1,1,1,1,4,1,0,1,3,0,0,0,0,6,0,0,1,0,1,0,4,0,0,0,0,0,4,0,1
db 5,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,6,0,0,5,0,1,0,4,0,0,0,0,0,4,0,1
db 1,0,0,0,1,0,0,5,0,0,0,3,0,0,0,0,6,1,5,1,0,1,0,4,4,0,0,0,4,4,0,1
db 5,0,0,0,0,0,0,5,0,0,0,0,0,0,1,1,0,0,0,0,0,1,0,0,4,4,4,4,4,0,0,1
db 1,4,1,4,1,4,1,4,1,4,1,3,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1
db 2,1,2,1,2,1,2,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
;---------------------------------------------------------------------
vpx:
	dd 0x0001FFFF	; initial player position * 0xFFFF
vpy:
	dd 0x0001FFFF

title	db 'Free3D v0.6 - fisheye raycasting engine etc.',0

sindegree	dd 0.0
sininc		dd 0.0017453292519943295769236907684886
sindiv		dd 6553.5
;textures:
;file 'texture.gif'
current_key_code	db 0
old_key_code		db 0
extended_key		db 0
;---------------------------------------------------------------------
align	4
fileinfo:
.subfunction	dd 5
.Offset		dd 0
.Offset_1	dd 0
.size		dd 0
.return		dd file_info
		db 0
.name:		dd file_name
;---------------------------------------------------------------------
icons_file_name		db 'texture_24b.png',0
;---------------------------------------------------------------------
plugins_directory	db 0

;system_dir_Boxlib	db '/sys/lib/box_lib.obj',0
system_dir_CnvPNG	db '/sys/lib/cnv_png.obj',0
;system_dir_Sort		db '/sys/lib/sort.obj',0
system_dir_UNPACK	db '/sys/lib/archiver.obj',0

align	4
l_libs_start:
library01	l_libs	system_dir_CnvPNG+9,file_name,system_dir_CnvPNG,\
cnv_png_import,plugins_directory

library02	l_libs	system_dir_UNPACK+9,file_name,system_dir_UNPACK,\
UNPACK_import,plugins_directory

end_l_libs:
;---------------------------------------------------------------------
align	4
cnv_png_import:
.Start		dd aCP_Start
.Version	dd aCP_Version
.Check		dd aCP_Check
.Assoc		dd aCP_Assoc
	dd 0
	dd 0
aCP_Start	db 'START',0
aCP_Version	db 'version',0
aCP_Check	db 'Check_Header',0
aCP_Assoc	db 'Associations',0
;---------------------------------------------------------------------
align	4
UNPACK_import:
;unpack_Version			dd aUnpack_Version
;unpack_PluginLoad		dd aUnpack_PluginLoad	
;unpack_OpenFilePlugin		dd aUnpack_OpenFilePlugin
;unpack_ClosePlugin		dd aUnpack_ClosePlugin
;unpack_ReadFolder		dd aUnpack_ReadFolder	
;unpack_SetFolder		dd aUnpack_SetFolder
;unpack_GetFiles		dd aUnpack_GetFiles
;unpack_GetOpenPluginInfo	dd aUnpack_GetOpenPluginInfo
;unpack_Getattr			dd aUnpack_Getattr
;unpack_Open			dd aUnpack_Open
;unpack_Read			dd aUnpack_Read
;unpack_Setpos			dd aUnpack_Setpos
;unpack_Close			dd aUnpack_Close
;unpack_DeflateUnpack		dd aUnpack_DeflateUnpack
unpack_DeflateUnpack2		dd aUnpack_DeflateUnpack2
	dd 0
	dd 0

;aUnpack_Version		db 'version',0
;aUnpack_PluginLoad		db 'plugin_load',0
;aUnpack_OpenFilePlugin		db 'OpenFilePlugin',0
;aUnpack_ClosePlugin		db 'ClosePlugin',0
;aUnpack_ReadFolder		db 'ReadFolder',0
;aUnpack_SetFolder		db 'SetFolder',0
;aUnpack_GetFiles		db 'GetFiles',0
;aUnpack_GetOpenPluginInfo	db 'GetOpenPluginInfo',0
;aUnpack_Getattr		db 'getattr',0
;aUnpack_Open			db 'open',0
;aUnpack_Read			db 'read',0
;aUnpack_Setpos			db 'setpos',0
;aUnpack_Close			db 'close',0
;aUnpack_DeflateUnpack		db 'deflate_unpack',0
aUnpack_DeflateUnpack2		db 'deflate_unpack2',0

;---------------------------------------------------------------------
IM_END:
;---------------------------------------------------------------------
; not	change	this	section!!!
; start	section
;---------------------------------------------------------------------
align	4
image_file	rd 1
raw_pointer	rd 1
return_code	rd 1
img_size	rd 1
deflate_unpack	rd 1
raw_pointer_2	rd 1	;+20
;---------------------------------------------------------------------
; end	section
;---------------------------------------------------------------------
align	4
;---------------------------------------------------------------------
wall0	rd 1
wall1	rd 1
wall2	rd 1
wall3	rd 1
wall4	rd 1
wall5	rd 1
wall6	rd 1
wall7	rd 1
;screen_buffer	rd 1
active_process	rd 1

;mouse_position		rd 1
mouse_position_old	rd 1
;---------------------------------------------------------------------
align	4
col1:
	dd ?	;-
; misc raycaster vars:
vxx:
	dd ?	;-
vyy:
	dd ?	;-
vl:
	dd ?	;-
vstepx:
	dd ?	;-
vstepy:
	dd ?	;-
vxxint:
	dd ?	;-
vyyint:
	dd ?	;-
vk:
	dd ?	;-
va:
	dd ?	;-
va2:
	dd ?	;-
vdd:
	dd ?	;-
vx1:
	dd ?	;-
vx1b:
	dd ?	;-
vh:
	dd ?	;-
vdt:
	dd ?	;-
vheading:	; initial heading: 0 to 3599
	dd ?	;-
vacompare:
	dd ?	;-
vpxi:
	dd ?	;-
vpyi:
	dd ?	;-
wtolong:
	dw ?,?	;-,?;-

xtemp:
	dd ?	;-
ytemp:
	dd ?	;-
xfrac:
	dd ?	;-
yfrac:
	dd ?	;-
h_old:
	dd ?	;-
vbottom:
	dd ?	;-
;mouseya:
;	dd ?	;-
remeax:
	dd ?	;-
remebx:
	dd ?	;-
remecx:
	dd ?	;-
remedx:
	dd ?	;-
remedi:
	dd ?	;-
remesi:
	dd ?	;-
red_color:
	dd ?	;-
green_color:
	dd ?	;-
blue_color:
	dd ?	;-
pseudo:
	dd ?	;-
step1:
	dd ?	;-
;step64:
;	dd ?	;-
lasty:
	dd ?	;-
;---------------------------------------------------------------------
;I_END:
IncludeUGlobals
align	4
sinus	rd 360*10
eosinus:
;	rd 16*1024*4
;---------------------------------------------------------------------
align	4
	rb 4096
stacktop:
;---------------------------------------------------------------------
procinfo:
	rb 1024
;---------------------------------------------------------------------
path:
	rb 4096
;---------------------------------------------------------------------
file_name:
	rb 4096
;---------------------------------------------------------------------
file_info:
	rb 40
;---------------------------------------------------------------------
screen_buffer:
	rb 640*480*3 *3/2
;---------------------------------------------------------------------
I_END:
;---------------------------------------------------------------------