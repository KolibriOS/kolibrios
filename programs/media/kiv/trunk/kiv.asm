use32
org 0x0

db 'MENUET01'
dd 0x01, START, I_END, 0x4000, 0x4000, @PARAMS, 0x0

;-----------------------------------------------------------------------------

FALSE = 0
TRUE  = 1

include '../../../proc32.inc'
include '../../../macros.inc'
include 'dll.inc'

include '../../../develop/libraries/libs-dev/libio/libio.inc'
include '../../../develop/libraries/libs-dev/libimg/libimg.inc'

;-----------------------------------------------------------------------------

START:
	mcall	68, 11

	stdcall dll.Load, @IMPORT
	or	eax, eax
	jnz	exit

	invoke	sort.START, 1

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
	jc	exit

	call	set_as_bgr
	jmp	exit

params_given:

	mov	eax, @PARAMS
	call	load_image
	jc	exit

;-----------------------------------------------------------------------------

red:
	call	draw_window

still:
	mov	eax, [image]
	test	byte [eax + Image.Flags], Image.IsAnimated
	push	10
	pop	eax
	jz	@f
	mcall	26, 9
	mov	edx, [cur_frame]
	mov	ebx, [cur_frame_time]
	add	ebx, [edx + Image.Delay]
	sub	ebx, eax
	cmp	ebx, [edx + Image.Delay]
	ja	red_update_frame
	test	ebx, ebx
	jz	red_update_frame
	push	23
	pop	eax
  @@:
	mcall
	dec	eax
	js	red_update_frame
	jz	red
	dec	eax
	jnz	button

key:
	mcall	2
	jmp	still

red_update_frame:
	mov	eax, [cur_frame]
	mov	eax, [eax + Image.Next]
	test	eax, eax
	jnz	@f
	mov	eax, [image]
  @@:
	mov	[cur_frame], eax
	mcall	26, 9
	mov	[cur_frame_time], eax
	mcall	9, procinfo, -1
	call	draw_cur_frame
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
	mov	byte [edi-1], 0
	invoke	img.destroy
	call	free_directory
	jmp	red
    .restore_old:
	pop	[image]
	call	init_frame
	jmp	still

	; set background
    @@:
	cmp	eax, 'bgr'
	jne	@f

	call	set_as_bgr
	jmp	still

    @@:

	cmp	eax, 'bck'
	jnz	@f
	call	prev_image
	jmp	red
    @@:
	cmp	eax, 'fwd'
	jnz	@f
	call	next_image
	jmp	red
    @@:

	cmp	eax, 1
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
	pushf
	mov	[image], eax
	call	init_frame
	popf
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
	mov	[draw_width], edx
	add	edx, 19
	cmp	edx, 40 + 25*9
	jae	@f
	mov	edx, 40 + 25*9
@@:
	mov	[wnd_width], edx
	mov	esi, [eax + Image.Height]
	mov	[draw_height], esi
	add	esi, 44
	mov	[wnd_height], esi
	popf
	jz	.no_resize
	mcall	48, 4
	add	esi, eax
	mcall	67,-1,-1
.no_resize:
	ret

set_as_bgr:
	mov	esi, [image]
	mov	ecx, [esi + Image.Width]
	mov	edx, [esi + Image.Height]
	mcall	15, 1

	mcall	15, 6
	test	eax, eax
	jz	@f

	push	eax
	invoke	img.to_rgb2, esi, eax
	pop	ecx
	mcall	15, 7

@@:
	mcall	15, 3
	ret

prev_image:
	call	load_directory
	cmp	[directory_ptr], 0
	jz	.ret
	mov	ebx, [directory_ptr]
	mov	eax, [cur_file_idx]
	cmp	eax, -1
	jnz	@f
	mov	eax, [ebx+4]
@@:
	push	[image]
.scanloop:
	dec	eax
	jns	@f
	mov	eax, [ebx+4]
	dec	eax
	cmp	[cur_file_idx], -1
	jz	.notfound
@@:
	cmp	eax, [cur_file_idx]
	jz	.notfound
	push	eax ebx
	imul	esi, eax, 304
	add	esi, [directory_ptr]
	add	esi, 32 + 40
	mov	edi, curdir
@@:
	inc	edi
	cmp	byte [edi-1], 0
	jnz	@b
	mov	byte [edi-1], '/'
@@:
	lodsb
	stosb
	test	al, al
	jnz	@b
	mov	eax, curdir
	call	load_image
	pushf
	mov	esi, curdir
	push	esi
	mov	edi, @PARAMS
	mov	ecx, 512/4
	rep	movsd
	mov	byte [edi-1], 0
	pop	esi
@@:
	lodsb
	test	al, al
	jnz	@b
@@:
	dec	esi
	cmp	byte [esi], '/'
	jnz	@b
	mov	byte [esi], 0
	popf
	pop	ebx eax
	jc	.scanloop
	mov	[cur_file_idx], eax
	invoke	img.destroy
.ret:
	ret
.notfound:
	pop	[image]
	call	init_frame
	ret

next_image:
	call	load_directory
	cmp	[directory_ptr], 0
	jz	.ret
	mov	ebx, [directory_ptr]
	mov	eax, [cur_file_idx]
	push	[image]
.scanloop:
	inc	eax
	cmp	eax, [ebx+4]
	jb	@f
	xor	eax, eax
	cmp	[cur_file_idx], -1
	jz	.notfound
@@:
	cmp	eax, [cur_file_idx]
	jz	.notfound
	push	eax ebx
	imul	esi, eax, 304
	add	esi, [directory_ptr]
	add	esi, 32 + 40
	mov	edi, curdir
@@:
	inc	edi
	cmp	byte [edi-1], 0
	jnz	@b
	mov	byte [edi-1], '/'
@@:
	lodsb
	stosb
	test	al, al
	jnz	@b
	mov	eax, curdir
	call	load_image
	pushf
	mov	esi, curdir
	push	esi
	mov	edi, @PARAMS
	mov	ecx, 512/4
	rep	movsd
	mov	byte [edi-1], 0
	pop	esi
@@:
	lodsb
	test	al, al
	jnz	@b
@@:
	dec	esi
	cmp	byte [esi], '/'
	jnz	@b
	mov	byte [esi], 0
	popf
	pop	ebx eax
	jc	.scanloop
	mov	[cur_file_idx], eax
	invoke	img.destroy
.ret:
	ret
.notfound:
	pop	[image]
	call	init_frame
	ret

load_directory:
	cmp	[directory_ptr], 0
	jnz	.ret
	mov	esi, @PARAMS
	mov	ecx, esi
@@:
	lodsb
	test	al, al
	jnz	@b
@@:
	dec	esi
	cmp	byte [esi], '/'
	jnz	@b
	mov	[last_name_component], esi
	sub	esi, ecx
	xchg	ecx, esi
	mov	edi, curdir
	rep	movsb
	mov	byte [edi], 0
	mcall	68, 12, 0x1000
	test	eax, eax
	jz	.ret
	mov	ebx, readdir_fileinfo
	mov	dword [ebx+12], (0x1000 - 32) / 304
	mov	dword [ebx+16], eax
	mcall	70
	cmp	eax, 6
	jz	.dirok
	test	eax, eax
	jnz	free_directory
	mov	edx, [directory_ptr]
	mov	ecx, [edx+8]
	mov	[readblocks], ecx
	imul	ecx, 304
	add	ecx, 32
	mcall	68, 20
	test	eax, eax
	jz	free_directory
	mov	[directory_ptr], eax
	mcall	70, readdir_fileinfo
.dirok:
	cmp	ebx, 0
	jle	free_directory
	mov	eax, [directory_ptr]
	add	eax, 32
	mov	edi, eax
	push	0
.dirskip:
	push	eax
	test	byte [eax], 18h
	jnz	.nocopy
	lea	esi, [eax+40]
	mov	ecx, esi
@@:
	lodsb
	test	al, al
	jnz	@b
@@:
	dec	esi
	cmp	esi, ecx
	jb	.noext
	cmp	byte [esi], '.'
	jnz	@b
	inc	esi
	mov	ecx, [esi]
	or	ecx, 0x202020
	cmp	ecx, 'jpg'
	jz	.copy
	cmp	ecx, 'bmp'
	jz	.copy
	cmp	ecx, 'gif'
	jz	.copy
	cmp	ecx, 'png'
	jz	.copy
	cmp	ecx, 'jpe'
	jz	.copy
	cmp	ecx, 'jpeg'
	jz	@f
	cmp	ecx, 'jpeG'
	jnz	.nocopy
@@:
	cmp	byte [esi+4], 0
	jnz	.nocopy
.copy:
	mov	esi, [esp]
	mov	ecx, 304 / 4
	rep	movsd
	inc	dword [esp+4]
.nocopy:
.noext:
	pop	eax
	add	eax, 304
	dec	ebx
	jnz	.dirskip
	mov	eax, [directory_ptr]
	pop	ebx
	mov	[eax+4], ebx
	test	ebx, ebx
	jz	free_directory
	push	0	; sort mode
	push	ebx
	add	eax, 32
	push	eax
	call	[SortDir]
	xor	eax, eax
	mov	edi, [directory_ptr]
	add	edi, 32 + 40
.scan:
	mov	esi, [last_name_component]
	inc	esi
	push	edi
	invoke	strcmpi
	pop	edi
	jz	.found
	inc	eax
	add	edi, 304
	dec	ebx
	jnz	.scan
	or	eax, -1
.found:
	mov	[cur_file_idx], eax
.ret:
	ret

free_directory:
	mcall	68, 13, [directory_ptr]
	and	[directory_ptr], 0
	ret

init_frame:
	push	eax
	mov	eax, [image]
	mov	[cur_frame], eax
	test	byte [eax + Image.Flags], Image.IsAnimated
	jz	@f
	push	ebx
	mcall	26, 9
	pop	ebx
	mov	[cur_frame_time], eax
@@:
	pop	eax
	ret

draw_window:
	cmp	[bFirstDraw], 0
	jz	.posok
	or	ecx, -1
	mcall	9, procinfo

	cmp	dword [ebx + 66], 0
	jle	.posok

	mov	edx, ecx
	mov	esi, ecx
	cmp	dword [ebx + 42], 40 + 25 * 9
	jae	@f
	mov	edx, 40 + 25 * 9
@@:
	cmp	dword [ebx + 46], 70
	jae	@f
	mov	esi, 70
@@:
	mov	eax, edx
	and	eax, esi
	cmp	eax, -1
	jz	@f
	mov	ebx, ecx
	mcall	67
@@:

.posok:
	mcall	12, 1
	mcall	48, 4
	mov	ebp, eax	; save skin height
	add	eax, [wnd_height]
	__mov	ebx, 100, 0
	add	ebx, [wnd_width]
	lea	ecx, [100*65536 + eax]
	mcall	0, , , 0x73FFFFFF, , s_header

	mcall	9, procinfo, -1
	mov	[bFirstDraw], 1
	cmp	dword [ebx + 66], 0
	jle	.nodraw
	mov	ebx, [ebx + 62]
	inc	ebx
	mcall	13, , <0, 35>, 0xFFFFFF
	mov	ecx, [procinfo + 66]
	inc	ecx
	mov	esi, [draw_height]
	add	esi, 35
	sub	ecx, esi
	jbe	@f
	push	esi
	shl	esi, 16
	add	ecx, esi
	pop	esi
	mcall
	xor	ecx, ecx
@@:
	add	ecx, esi
	add	ecx, 35*10000h - 35
	__mov	ebx, 0, 5
	mcall
	mov	esi, [draw_width]
	add	esi, ebx
	mov	ebx, [procinfo+62]
	inc	ebx
	sub	ebx, esi
	jbe	@f
	shl	esi, 16
	add	ebx, esi
	mcall
@@:

	mov	ebx, [procinfo + 62]
	push	ebx
	mcall	38, , <30, 30>, 0x007F7F7F
	mcall	, <5 + 25 * 1, 5 + 25 * 1>, <0, 30>
	mcall	, <10 + 25 * 3, 10 + 25 * 3>
	mcall	, <15 + 25 * 4, 15 + 25 * 4>
	pop	ebx
	sub	ebx, 25 * 5 + 10
	push	ebx
	imul	ebx, 10001h
	mcall

	mcall	8, <5 + 25 * 0, 20>, <5, 20>, 'opn'+40000000h
	mcall	, <10 + 25 * 1, 20>, , 'bck'+40000000h
	mcall	, <10 + 25 * 2, 20>, , 'fwd'+40000000h
	mcall	, <15 + 25 * 3, 20>, , 'bgr'+40000000h
	pop	ebx
	add	ebx, 5
	shl	ebx, 16
	mov	bl, 20
	mcall	, , , 'flh'+40000000h
	add	ebx, 25 * 65536
	mcall	, , , 'flv'+40000000h
	add	ebx, 30 * 65536
	mcall	, , , 'rtr'+40000000h
	add	ebx, 25 * 65536
	mcall	, , , 'rtl'+40000000h
	add	ebx, 25 * 65536
	mcall	, , , 'flb'+40000000h

	mov	ebp, (numimages-1)*20

	mcall	65, buttons+openbtn*20, <20, 20>, <5 + 25 * 0, 5>, 8, palette
	mcall	, buttons+backbtn*20, , <10 + 25 * 1, 5>
	mcall	, buttons+forwardbtn*20, , <10 + 25 * 2, 5>
	mcall	, buttons+bgrbtn*20, , <15 + 25 * 3, 5>
	mov	edx, [procinfo + 62]
	sub	edx, 25 * 5 + 4
	shl	edx, 16
	mov	dl, 5
	mcall	, buttons+fliphorzbtn*20
	add	edx, 25 * 65536
	mcall	, buttons+flipvertbtn*20
	add	edx, 30 * 65536
	mcall	, buttons+rotcwbtn*20
	add	edx, 25 * 65536
	mcall	, buttons+rotccwbtn*20
	add	edx, 25 * 65536
	mcall	, buttons+rot180btn*20

	call	draw_cur_frame

.nodraw:
	mcall	12, 2

	ret

draw_cur_frame:
	push	0	; ypos
	push	0	; xpos
	mov	eax, [procinfo+66]
	sub	eax, 34
	push	eax	; max height
	mov	eax, [procinfo+62]
	sub	eax, 4
	push	eax	; max width
	push	35	; y
	push	5	; x
	push	[cur_frame]
	call	[img.draw]
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
	libimg , 'libimg.obj' , \
	sort   , 'sort.obj'

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
	img.destroy , 'img.destroy', \
	img.draw    , 'img.draw'

import  sort, sort.START, 'START', SortDir, 'SortDir', strcmpi, 'strcmpi'

bFirstDraw	db	0
;-----------------------------------------------------------------------------

virtual at 0
file 'kivicons.bmp':0xA,4
load offbits dword from 0
end virtual
numimages = 9
openbtn = 0
backbtn = 1
forwardbtn = 2
bgrbtn = 3
fliphorzbtn = 4
flipvertbtn = 5
rotcwbtn = 6
rotccwbtn = 7
rot180btn = 8

palette:
	file 'kivicons.bmp':0x36,offbits-0x36
buttons:
	file 'kivicons.bmp':offbits
repeat 10
y = % - 1
z = 20 - %
repeat numimages*5
load a dword from $ - numimages*20*20 + numimages*20*y + (%-1)*4
load b dword from $ - numimages*20*20 + numimages*20*z + (%-1)*4
store dword a at $ - numimages*20*20 + numimages*20*z + (%-1)*4
store dword b at $ - numimages*20*20 + numimages*20*y + (%-1)*4
end repeat
end repeat

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

readdir_fileinfo:
	dd	1
	dd	0
	dd	0
readblocks dd	0
directory_ptr	dd	0

;-----------------------------------------------------------------------------

I_END:

curdir		rb	1024

align 4
img_data     dd ?
img_data_len dd ?
fh	     dd ?
image	     dd ?
wnd_width	dd	?
wnd_height	dd	?
draw_width	dd	?
draw_height	dd	?
last_name_component	dd	?
cur_file_idx	dd	?
cur_frame_time	dd	?
cur_frame	dd	?

ctx dd ?

procinfo:	rb	1024
path:		rb	1024+16

@PARAMS rb 512
