include "lang.inc"
include "../../../macros.inc"

WND_SIZE_X		= 320
WND_SIZE_Y		= 200

VC_DELTA = 1
HC_DELTA = 2

MEOS_APP_START
CODE
    fninit
    call init_sinus_table
    call init_background
    call init_palette
    mov eax,40
    mov ebx,101b
    mcall
    jmp .paint_window

.event_loop:
    mov eax,23
    mov ebx,1
    mcall

    test eax,eax
    je .draw_screen
    dec eax
    je .paint_window

    or  eax,-1
    mcall

.draw_screen:
    test [proc_info.wnd_state], 0x04
    jnz .event_loop
    add word [ver_counter],VC_DELTA
    add word [hor_counter],HC_DELTA
    call handle_animation
    xor ebp,ebp
    mcall 65,virtual_screen_8,<WND_SIZE_X,WND_SIZE_Y>,<0,0>,8,_palette
    jmp .event_loop

.paint_window:
	mcall	9,proc_info,-1
    mov eax,12
    mov ebx,1
    mcall

    mcall 48,4 ; get skin height
    lea ecx,[eax + (100 shl 16) + WND_SIZE_Y+4]
    mov edi,title
    mcall 0,<100,WND_SIZE_X+9>,,0x74000000

    test [proc_info.wnd_state], 0x04
    jnz @f

    xor ebp,ebp
    mcall 65,virtual_screen_8,<WND_SIZE_X,WND_SIZE_Y>,<0,0>,8,_palette
  @@:
    mov eax,12
    mov ebx,2
    mcall

    jmp .event_loop

init_palette:
    mov ecx,256
    mov edi,_palette
    xor eax,eax
.next_pal:
    mov al,ah
    shr al,2
    stosb
    stosb
    stosb
    stosb
    inc ah
    loop .next_pal
    ret

init_sinus_table:
    sub esp,4
    mov ecx,256
    mov edi,sinetable
.sin_loop:
    fld dword [esp]
    fld st0
    fsin
    fmul [scale_sin]
    fistp word [edi]
    fadd [delta_angle]
    fstp dword [esp]
    add edi,2
    loop .sin_loop
    add esp,4
    ret

init_background:
    mov edi,background
    xor edx,edx
.ib_vertical:
    xor ecx,ecx
.ib_horizontal:
    mov eax,ecx
    xor eax,edx
    stosb
    inc ecx
    cmp ecx,256
    jne .ib_horizontal
    inc edx
    cmp edx,256
    jne .ib_vertical
    ret

s_OFFX		= 0
s_OFFY		= 2

handle_animation:
    sub esp,4
    mov ebx,[ver_counter]
    and ebx,255
    add ebx,ebx
    mov ax,[sinetable+ebx]
    mov [esp+s_OFFY],ax
    mov ebx,[hor_counter]
    and ebx,255
    add ebx,ebx
    mov ax,[sinetable+ebx]
    mov [esp+s_OFFX],ax
    mov edi,virtual_screen_8
    mov edx,WND_SIZE_Y-1
.a_ver:
    mov ecx,WND_SIZE_X-1
    mov bx,[esp+s_OFFY]
    add bx,dx
    and ebx,255
    shl ebx,8
.a_hor:
    mov ax,[esp+s_OFFX]
    add ax,cx
    and eax,255
    mov al,[background+ebx+eax]
    stosb
    dec ecx
    jge .a_hor
    dec edx
    jge .a_ver
    add esp,4
    ret

DATA
  delta_angle dd 0.0245436926066		; pi/128
  scale_sin dd 128.0

  title      db 'MoveBack',0

UDATA
  ver_counter dd ?
  hor_counter dd ?

  _palette:	rd 256

  virtual_screen_8:
  	rb WND_SIZE_X*WND_SIZE_Y

  background:
  	rb 256*256

  sinetable:
  	rw 256

  proc_info	process_information

MEOS_APP_END
