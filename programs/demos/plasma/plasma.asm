; Originally written by Jarek Pelczar
include "lang.inc"
include "..\..\macros.inc"
include "..\..\KOSfuncs.inc"

KOS_APP_START

WND_SIZE_X dd 640
WND_SIZE_Y dd 400

if lang eq ru_RU
	title db 'Плазма',0
else
	title db 'Plasma',0
end if

CODE
	mcall SF_SYS_MISC,SSF_HEAP_INIT
	mov ecx,[WND_SIZE_X]
	imul ecx,[WND_SIZE_Y]
	mcall SF_SYS_MISC,SSF_MEM_ALLOC
	mov [virtual_screen_8],eax

    fninit
    mcall SF_SET_EVENTS_MASK, 101b
    call init_palette
    call init_texture
    jmp .paint_window
.event_loop:
    mcall SF_WAIT_EVENT_TIMEOUT, 1
    test eax,eax
    je .draw_screen
    dec eax
    je .paint_window

    mcall SF_TERMINATE_PROCESS

.draw_screen:
    xor ebp,ebp
	mov ecx,[WND_SIZE_X]
	shl ecx,16
	add ecx,[WND_SIZE_Y]
    mcall SF_PUT_IMAGE_EXT, [virtual_screen_8],,0,8,_palette
    call rotate_pal
    jmp .event_loop

.paint_window:
    mcall SF_REDRAW, SSF_BEGIN_DRAW

	;if window resize
	mcall SF_THREAD_INFO,procinfo,-1
	cmp dword[procinfo.box.height],0
	je .resize_end
	mcall SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
	add eax,4
	sub eax,[procinfo.box.height]
	neg eax
	cmp eax,[WND_SIZE_Y]
	je .end_h
	cmp eax,32 ;min height
	jge @f
		mov eax,32
	@@:
	mov [WND_SIZE_Y],eax
	xor eax,eax
	mov [WND_SIZE_X],eax
	.end_h:
	
	mov eax,[procinfo.box.width]
	sub eax,9
	cmp eax,[WND_SIZE_X]
	je .resize_end
	cmp eax,64 ;min width
	jge @f
		mov eax,64
	@@:
	mov [WND_SIZE_X],eax

	call OnResize
	call init_texture
	.resize_end:

    mcall SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
    lea ecx,[eax + (110 shl 16) + 4]
	add ecx,[WND_SIZE_Y]
    mov edi,title
	mov ebx,[WND_SIZE_X]
	add ebx,(110 shl 16)+9
    mcall SF_CREATE_WINDOW,,,0x73000000

	xor ebp,ebp
	mov ecx,[WND_SIZE_X]
	shl ecx,16
	add ecx,[WND_SIZE_Y]
    mcall SF_PUT_IMAGE_EXT, [virtual_screen_8],,0,8,_palette

    mcall SF_REDRAW, SSF_END_DRAW

    jmp .event_loop

align 4
OnResize:
	mov ecx,[WND_SIZE_X]
	imul ecx,[WND_SIZE_Y]
	mcall SF_SYS_MISC,SSF_MEM_ALLOC
	mov [virtual_screen_8],eax
	ret

align 4
init_palette:
    mov edi,_palette
    mov ecx,64
    xor eax,eax
.color1:
    inc ah
    mov al,ah
    stosb
    xor al,al
    stosb
    stosb
    stosb
    loop .color1
    mov ecx,64
    push ecx
    xor eax,eax
.color2:
    mov al,63
    stosb
    mov al,ah
    stosb
    xor al,al
    stosb
    stosb
    inc ah
    loop .color2
    pop ecx
    push ecx
    xor eax,eax
.color3:
    mov al,63
    stosb
    stosb
    mov al,ah
    stosb
    mov al,0
    stosb
    inc ah
    loop .color3
    pop ecx
    mov eax,0x003f3f3f
    rep stosd
    ret

init_texture:
    fldpi
    mov [_fpom16],180
    fidiv [_fpom16]
    fstp [_st_rad]
    mov edi,[virtual_screen_8]
    cdq
.itex_vertical:
    xor ecx,ecx
    fld [_st_rad]
    mov [_fpom16],5
    fimul [_fpom16]
    mov [_fpom16],dx
    fimul [_fpom16]
    fsin
    fmul [_multiplier]
    fstp [_fpom32]
.itex_horizontal:
    fld [_st_rad]
    mov [_fpom16],3
    fimul [_fpom16]
    mov [_fpom16],cx
    fimul [_fpom16]
    fsin
    fmul [_multiplier]
    fadd [_fpom32]
    mov [_fpom16],127
    fiadd [_fpom16]
    fistp [_fpom16]
    mov ax,[_fpom16]
    inc eax
    stosb
    inc ecx
    cmp ecx,[WND_SIZE_X]
    jne .itex_horizontal
    inc edx
    cmp edx,[WND_SIZE_Y]
    jne .itex_vertical
    ret

rotate_pal:
    mov ebx,[_palette+4]
    mov edi,_palette+4
    mov esi,_palette+8
    xor ecx,ecx
    mov cl,255
;    cld
    rep movsd
    mov [_palette+1020],ebx
    ret

DATA
  _multiplier	dd 63.5


UDATA
  _fpom32		rd 1
  _fpom16		rw 1
  _st_rad		rd 1
  _palette:	rd 256

  virtual_screen_8 rd 1
  procinfo process_information

KOS_APP_END
