use32
org 0x0

db 'MENUET01'
dd 0x01, START, I_END, 0x4000, 0x4000, @PARAMS, 0x0

;-----------------------------------------------------------------------------

FALSE = 0
TRUE  = 1

include '../../../../../proc32.inc'
include '../../../../../macros.inc'
include 'dll.inc'

include '../../../develop/libraries/libs-dev/libio/libio.inc'
include '../../../develop/libraries/libs-dev/libimg/libimg.inc'

;-----------------------------------------------------------------------------

START:
	mcall	68, 11

	stdcall dll.Load, @IMPORT
	or	eax, eax
	jnz	exit

	mov	ecx, 1	; for 15.4: 1 = tile
	cmp	word [@PARAMS], '\T'
	jz	set_bgr
	inc	ecx	; for 15.4: 2 = stretch
	cmp	word [@PARAMS], '\S'
	jz	set_bgr

	cmp	byte [@PARAMS], 0
	jnz	params_given

	call	opendialog
	jc	exit
	mov	esi, path
	mov	edi, @PARAMS
	mov	ecx, 512/4
	rep	movsd
	mov	byte [edi-1], 0
	jmp	params_given

set_bgr:
	mcall	15, 4
	mov	eax, @PARAMS + 4
	call	load_image
	jc	exit_bgr

	mov	esi, [image]
	mov	ecx, [esi + Image.Width]
	mov	edx, [esi + Image.Height]
	mcall	15, 1

	mcall	15, 6
	test	eax, eax
	jz	exit_bgr

	push	eax
	invoke	img.to_rgb2, esi, eax
	pop	ecx
	mcall	15, 7

exit_bgr:
	mcall	15, 3
	jmp	exit

params_given:

	mov	eax, @PARAMS
	call	load_image
	jc	exit

;-----------------------------------------------------------------------------

red:
	call	draw_window

still:
	mcall	10
	dec	eax
	jz	red
	dec	eax
	jnz	button

  key:
	mcall	2
	jmp	still

  button:
	mcall	17
	shr	eax, 8

	; flip horizontally
	cmp	eax, 'flh'
	jne	@f

	invoke	img.flip, [image], FLIP_HORIZONTAL
	jmp	redraw_image

	; flip vertically
    @@: cmp	eax, 'flv'
	jne	@f

	invoke	img.flip, [image], FLIP_VERTICAL
	jmp	redraw_image

	; flip both horizontally and vertically
    @@: cmp	eax, 'flb'
	jne	@f

	invoke	img.flip, [image], FLIP_BOTH
	jmp	redraw_image

	; rotate left
    @@: cmp	eax, 'rtl'
	jne	@f

	push	ROTATE_90_CCW
.rotate_common:
	invoke	img.rotate, [image]
	mov	eax, [image]
	test	eax, eax	; clear ZF flag
	call	update_image_sizes
	jmp	redraw_image

	; rotate right
    @@: cmp	eax, 'rtr'
	jne	@f

	push	ROTATE_90_CW
	jmp	.rotate_common

	; open new file
    @@: cmp	eax, 'opn'
	jne	@f
	
	call	opendialog
	jc	still
	push	[image]
	mov	eax, path
	call	load_image
	jc	.restore_old
	mov	esi, path
	mov	edi, @PARAMS
	mov	ecx, 512/4
	rep	movsd
	invoke	img.destroy
	mov	byte [edi-1], 0
	jmp	red
    .restore_old:
	pop	[image]
	jmp	still

    @@: cmp	eax, 1
	jne	still

  exit:
	mcall	-1

  redraw_image = red

load_image:
	and	[img_data], 0
	push	eax
	invoke	file.open, eax, O_READ
	or	eax, eax
	jz	.error_pop
	mov	[fh], eax
	invoke	file.size
	mov	[img_data_len], ebx
	stdcall mem.Alloc, ebx
	test	eax, eax
	jz	.error_close
	mov	[img_data], eax
	invoke	file.read, [fh], eax, [img_data_len]
	cmp	eax, -1
	jz	.error_close
	cmp	eax, [img_data_len]
	jnz	.error_close
	invoke	file.close, [fh]
	inc	eax
	jz	.error

; img.decode checks for img.is_img
;	invoke	img.is_img, [img_data], [img_data_len]
;	or	eax, eax
;	jz	exit
	invoke	img.decode, [img_data], [img_data_len]
	or	eax, eax
	jz	.error
	cmp	[image], 0
	mov	[image], eax
	call	update_image_sizes
	call	free_img_data
	clc
	ret

.error_free:
	invoke	img.destroy, [image]
	jmp	.error

.error_pop:
	pop	eax
	jmp	.error
.error_close:
	invoke	file.close, [fh]
.error:
	call	free_img_data
	stc
	ret

free_img_data:
	mov	eax, [img_data]
	test	eax, eax
	jz	@f
	stdcall	mem.Free, eax
@@:
	ret

update_image_sizes:
	pushf
	mov	edx, [eax + Image.Width]
	add	edx, 19
	cmp	edx, 50 + 25*5
	jae	@f
	mov	edx, 50 + 25*5
@@:
	mov	[wnd_width], edx
	mov	esi, [eax + Image.Height]
	add	esi, 44
	mov	[wnd_height], esi
	popf
	jz	.no_resize
	mcall	48, 4
	add	esi, eax
	mcall	67,-1,-1
.no_resize:
	ret

draw_window:
	invoke	gfx.open, TRUE
	mov	[ctx], eax

	mcall	48, 4
	mov	ebp, eax	; save skin height
	add	eax, [wnd_height]
	__mov	ebx, 100, 0
	add	ebx, [wnd_width]
	lea	ecx, [100*65536 + eax]
	mcall	0, , , 0x73FFFFFF, , s_header

	mcall	9, procinfo, -1

	mov	ebx, [procinfo + 42]
	sub	ebx, 9
	mcall	13, , <0, 35>, 0xFFFFFF
	mov	ecx, [procinfo + 46]
	sub	ecx, ebp
	sub	ecx, 9
	shl	ecx, 16
	mov	cl, 5
	mcall
	mov	cl, 35
	ror	ecx, 16
	sub	cx, 35
	mov	ebx, 5
	mcall
; 5 pixels for indentation, [image.Width] pixels for image
; client_width - 5 - [image.Width] pixels must be white
	mov	ebx, [image]
	mov	esi, [procinfo + 62]
	inc	esi
	push	esi
	mov	ebx, [ebx + Image.Width]
	sub	esi, 5
	sub	esi, ebx
	pop	ebx
	sub	ebx, esi
	shl	ebx, 16
	add	ebx, esi
	mcall

	invoke	gfx.pen.color, [ctx], 0x007F7F7F
	mov	eax, [procinfo + 42]
	sub	eax, 10
	invoke	gfx.line, [ctx], 0, 30, eax, 30

	xor	ebp, ebp

	mcall	8, <5 + 25 * 0, 20>, <5, 20>, 'opn'+40000000h
	mcall	65, openbtn, <20, 20>, <5 + 25 * 0, 5>, 4, palette

	invoke	gfx.line, [ctx], 5 + 25 * 1, 0, 5 + 25 * 1, 30

	mcall	8, <10 + 25 * 1, 20>, <5, 20>, 'flh'+40000000h
	mcall	65, fliphorzbtn, <20, 20>, <10 + 25 * 1, 5>, 4, palette
	mcall	8, <10 + 25 * 2, 20>, <5, 20>, 'flv'+40000000h
	mcall	65, flipvertbtn, <20, 20>, <10 + 25 * 2, 5>, 4, palette

	invoke	gfx.line, [ctx], 10 + 25 * 3, 0, 10 + 25 * 3, 30

	mcall	8, <15 + 25 * 3, 20>, <5, 20>, 'rtr'+40000000h
	mcall	65, rotcwbtn, <20, 20>, <15 + 25 * 3, 5>, 4, palette
	mcall	8, <15 + 25 * 4, 20>, <5, 20>, 'rtl'+40000000h
	mcall	65, rotccwbtn, <20, 20>, <15 + 25 * 4, 5>, 4, palette
	mcall	8, <15 + 25 * 5, 20>, <5, 20>, 'flb'+40000000h
	mcall	65, rot180btn, <20, 20>, <15 + 25 * 5, 5>, 4, palette

	mov	ebx, [image]
	mov	ecx, [ebx + Image.Width]
	shl	ecx, 16
	add	ecx, [ebx + Image.Height]
	__mov	edx, 5, 35
	mov	esi, 8
	cmp	[ebx + Image.Type], Image.bpp8
	jz	@f
	mov	esi, 24
	cmp	[ebx + Image.Type], Image.bpp24
	jz	@f
	mov	esi, 32
@@:
	mov	edi, [ebx + Image.Palette]
	mov	ebx, [ebx + Image.Data]
	xor	ebp, ebp
	mcall	65

	invoke	gfx.close, [ctx]
	ret

; void* __stdcall mem.Alloc(unsigned size);
mem.Alloc:
	push	ebx ecx
	mov	ecx, [esp+12]
	mcall	68, 12
	pop	ecx ebx
	ret	4

; void* __stdcall mem.ReAlloc(void* mptr, unsigned size);
mem.ReAlloc:
	push	ebx ecx edx
	mov	edx, [esp+16]
	mov	ecx, [esp+20]
	mcall	68, 20
	pop	edx ecx ebx
	ret	8

; void __stdcall mem.Free(void* mptr);
mem.Free:
	push	ebx ecx
	mov	ecx, [esp+12]
	mcall	68, 13
	pop	ecx ebx
	ret	4

;-----------------------------------------------------------------------------

s_header db 'Kolibri Image Viewer', 0

;-----------------------------------------------------------------------------

opendialog:
;
; STEP 1 Run SYSXTREE with parametrs MYPID 4 bytes in dec,
; 1 byte space, 1 byte type of dialog (O - Open ,S - Save)
;

;;    mov esi,path
    mov edi,path
    xor eax,eax
    mov ecx,(1024+16)/4
    rep stosd

;mov [get_loops],0
mov [dlg_pid_get],0

; Get my PID in dec format 4 bytes
    mov eax,9
    mov ebx,procinfo
    or  ecx,-1
    mcall

; convert eax bin to param dec
    mov eax,dword [procinfo+30]  ;offset of myPID
    mov edi,param+4-1		 ;offset to 4 bytes
    mov ecx,4
    mov ebx,10
new_d:
    xor edx,edx
    div ebx
    add dl,'0'
    mov [edi],dl
    dec edi
    loop new_d

; wirite 1 byte space to param
    mov [param+4],byte 32    ;Space for next parametr
; and 1 byte type of dialog to param
    mov [param+5],byte 'O'   ;Get Open dialog (Use 'S' for Save dialog)

;
; STEP2 prepare IPC area for get messages
;

; prepare IPC area
    mov [path],dword 0
    mov [path+4],dword 8

; define IPC memory
    mov eax,60
    mov ebx,1	     ; define IPC
    mov ecx,path     ; offset of area
    mov edx,1024+16  ; size
    mcall

; change wanted events list 7-bit IPC event
    mov eax,40
    mov ebx,01000111b
	cmp	[image], 0
	jnz	@f
	mov	bl, 01000110b
@@:
    mcall

;
; STEP 3 run SYSTEM XTREE with parameters
;

    mov eax,70
    mov ebx,run_fileinfo
    mcall

    mov [get_loops],0
getmesloop:
    mov eax,23
    mov ebx,50	   ;0.5 sec
    mcall
        dec     eax
        jz      mred
        dec     eax
        jz      mkey
        dec     eax
        jz      mbutton
        cmp     al, 7-3
        jz      mgetmes

; Get number of procces
    mov ebx,procinfo
    mov ecx,-1
    mov eax,9
    mcall
    mov ebp,eax

loox:
    mov eax,9
    mov ebx,procinfo
    mov ecx,ebp
    mcall
    mov eax,[DLGPID]
    cmp [procinfo+30],eax    ;IF Dialog find
    je	dlg_is_work	     ;jmp to dlg_is_work
    dec ebp
    jnz loox

    jmp erroff

dlg_is_work:
    cmp [procinfo+50],word 9 ;If slot state 9 - dialog is terminated
    je	erroff		       ;TESTODP2 terminated too

    cmp [dlg_pid_get],dword 1
    je	getmesloop
    inc [get_loops]
    cmp [get_loops],4  ;2 sec if DLG_PID not get, TESTOP2  terminated
    jae erroff
    jmp getmesloop

mred:
	cmp	[image], 0
	jz	getmesloop
    call draw_window
    jmp  getmesloop
mkey:
    mov  eax,2
    mcall			; read (eax=2)
    jmp  getmesloop
mbutton:
    mov  eax,17 		; get id
    mcall
    cmp  ah,1			; button id=1 ?
    jne  getmesloop
    mov  eax,-1 		; close this program
    mcall
mgetmes:

; If dlg_pid_get then second message get jmp to still
    cmp  [dlg_pid_get],dword 1
    je	 ready

; First message is number of PID SYSXTREE dialog

; convert PID dec to PID bin
    movzx eax,byte [path+16]
    sub eax,48
    imul eax,10
    movzx ebx,byte [path+16+1]
    add eax,ebx
    sub eax,48
    imul eax,10
    movzx ebx,byte [path+16+2]
    add eax,ebx
    sub eax,48
    imul eax,10
    movzx ebx,byte [path+16+3]
    add eax,ebx
    sub eax,48
    mov [DLGPID],eax

; Claear and prepare IPC area for next message
    mov [path],dword 0
    mov [path+4],dword 8
    mov [path+8],dword 0
    mov [path+12],dword 0
    mov [path+16],dword 0

; Set dlg_pid_get for get next message
    mov [dlg_pid_get],dword 1
	cmp	[image], 0
	jz	getmesloop
    call draw_window
    jmp  getmesloop

ready:
;
; The second message get
; Second message is 100 bytes path to SAVE/OPEN file
; shl path string on 16 bytes
;
    mov esi,path+16
    mov edi,path
    mov ecx,1024/4
    rep movsd
    mov [edi],byte 0

openoff:
	mcall	40, 7
	clc
	ret

erroff:
	mcall	40, 7
	stc
	ret

;-----------------------------------------------------------------------------

align 4
@IMPORT:

library 			\
	libio  , 'libio.obj'  , \
	libgfx , 'libgfx.obj' , \
	libimg , 'libimg.obj'

import	libio			  , \
	libio.init , 'lib_init'   , \
	file.size  , 'file.size'  , \
	file.open  , 'file.open'  , \
	file.read  , 'file.read'  , \
	file.close , 'file.close'

import	libgfx				, \
	libgfx.init   , 'lib_init'	, \
	gfx.open      , 'gfx.open'	, \
	gfx.close     , 'gfx.close'	, \
	gfx.pen.color , 'gfx.pen.color' , \
	gfx.line      , 'gfx.line'

import	libimg			   , \
	libimg.init , 'lib_init'   , \
	img.is_img  , 'img.is_img' , \
	img.to_rgb2 , 'img.to_rgb2', \
	img.decode  , 'img.decode' , \
	img.flip    , 'img.flip'   , \
	img.rotate  , 'img.rotate' , \
	img.destroy , 'img.destroy'

;-----------------------------------------------------------------------------

palette:
	dd	0x000000, 0x800000, 0x008000, 0x808000
	dd	0x000080, 0x800080, 0x008080, 0x808080
	dd	0xC0C0C0, 0xFF0000, 0x00FF00, 0xFFFF00
	dd	0x0000FF, 0xFF00FF, 0x00FFFF, 0xFFFFFF

macro loadbtn filename
{
repeat 20
file filename:76h+(%-1)*12,10
end repeat
repeat 10
y = % - 1
z = 20 - %
repeat 10
load a byte from $ - 20*10 + y*10 + (%-1)
load b byte from $ - 20*10 + z*10 + (%-1)
store byte a at $ - 20*10 + z*10 + (%-1)
store byte b at $ - 20*10 + y*10 + (%-1)
end repeat
end repeat
}

openbtn:
	loadbtn 'open.bmp'
fliphorzbtn:
	loadbtn 'fliphorz.bmp'
flipvertbtn:
	loadbtn 'flipvert.bmp'
rotcwbtn:
	loadbtn 'rotcw.bmp'
rotccwbtn:
	loadbtn 'rotccw.bmp'
rot180btn:
	loadbtn 'rot180.bmp'

; DATA AREA
get_loops   dd 0
dlg_pid_get dd 0
DLGPID	    dd 0

param:
   dd 0    ; My dec PID
   dd 0,0  ; Type of dialog

run_fileinfo:
 dd 7
 dd 0
 dd param
 dd 0
 dd 0
;run_filepath
 db '/sys/SYSXTREE',0

;-----------------------------------------------------------------------------

I_END:

img_data     dd ?
img_data_len dd ?
fh	     dd ?
image	     dd ?
wnd_width	dd	?
wnd_height	dd	?

ctx dd ?

procinfo:	rb	1024
path:		rb	1024+16

@PARAMS rb 512
