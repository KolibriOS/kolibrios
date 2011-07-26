; Originally written by Jarek Pelczar
include "lang.inc"
include "..\..\..\macros.inc"

WND_SIZE_X		= 320
WND_SIZE_Y		= 200

MEOS_APP_START
CODE
    fninit
    mov al,40
    mov bl,101b
    mcall
    call init_palette
    call init_texture
    jmp .paint_window
.event_loop:
    mov al,23
    mov ebx,1
    mcall
    test eax,eax
    je .draw_screen
    dec eax
    je .paint_window

    or  eax,-1
    mcall

.draw_screen:
    call blit_8_to_32
    mov al,7
    mov ebx,virtual_screen_32
    mov ecx,(WND_SIZE_X shl 16)+WND_SIZE_Y
    xor edx,edx
    mcall
    call rotate_pal
    jmp .event_loop

.paint_window:
    mov al,12
    push eax
    xor ebx,ebx
    inc ebx
    mcall

    xor eax,eax
    mov ebx,(110 shl 16)+(WND_SIZE_X+9)
    mov ecx,(110 shl 16)+(WND_SIZE_Y+26)
    mov edx,0x74000000
    mov edi,title
    mcall

    mov al,7
    mov ebx,virtual_screen_32
    mov ecx,(WND_SIZE_X shl 16)+WND_SIZE_Y
    xor edx,edx
    mcall

    pop eax
    and ebx,2 ; bit 1 is set
    mcall

    jmp .event_loop

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
    inc edi
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
    inc ah
    inc edi
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
    inc ah
    inc edi
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
    mov edi,virtual_screen_8
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
    cmp ecx,WND_SIZE_X
    jne .itex_horizontal
    inc edx
    cmp edx,WND_SIZE_Y
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

include "graph8.inc"

DATA
  _multiplier	dd 63.5

  title          db 'Plasma',0

UDATA
  _fpom32		rd 1
  _fpom16		rw 1
  _st_rad		rd 1
  _palette:	rd 256

  virtual_screen_8:
   	rb WND_SIZE_X*WND_SIZE_Y

  virtual_screen_32:
   	rb WND_SIZE_X*WND_SIZE_Y*3

MEOS_APP_END
