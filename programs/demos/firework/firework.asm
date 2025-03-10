;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; A firework demo
; Programmed by Yaniv LEVIATHAN
; http://yaniv.leviathanonline.com
; Converted to DexOS, By Dex
; Converted to KolibriOS, By Asper
; Optimized for KolibriOS, By Diamond
; Assemble with
; c:fasm firework.asm firework.kex
; NOTE: Needs MMX & SSE,
; optionally AVX, AVX2, AVX512
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
use32
        org     0x0

        db      'MENUET01'      ; 8 byte id
        dd      0x01            ; version
        dd      STARTAPP        ; program start
        dd      I_END           ; program image size
        dd      E_END           ; required amount of memory
        dd      stacktop        ; reserved=no extended header
        dd      0, 0

include '../../macros.inc'
include '../../KOSfuncs.inc'

SCREEN_WIDTH   = 600 ;.. mod 8 == 0
Screen_W      dd SCREEN_WIDTH
Screen_H      dd 400
lost_bytes    dd 0

SIMD equ SSE
SIMD_BYTES = 8
; SSE    8
; AVX    16
; AVX2   32
; AVX512 64
assert SCREEN_WIDTH mod SIMD_BYTES = 0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Global defines
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

NUM_PARTS      = 150
X_OFFSET       = 0
Y_OFFSET       = 4
X_SPEED_OFFSET = 8
Y_SPEED_OFFSET = 12
COLOR_OFFSET   = 16
PART_SIZE      = 20

macro shade
{
local .lop
	mov     ecx, [Screen_H]
	imul    ecx, [Screen_W]
	shr     ecx, 3 ;ecx = Screen_W * Screen_H / SIMD_BYTES
	mov     edi, [buffer]
if SIMD eq SSE
        movq    mm1, qword [sub_mask]
.lop:
        movq    mm0, [edi]
        psubusb mm0, mm1
        movq    [edi], mm0
        add     edi, SIMD_BYTES
        loop    .lop
else if SIMD eq AVX
        vmovdqa xmm1, xword [sub_mask]
.lop:
        vmovdqa xmm0, [edi]
        vpsubusb xmm0, xmm0, xmm1
        vmovdqa [edi], xmm0
        add     edi, SIMD_BYTES
        loop    .lop
else if SIMD eq AVX2
        vmovdqa ymm1, yword [sub_mask]
.lop:
        vmovdqa ymm0, [edi]
        vpsubusb ymm0, ymm0, ymm1
        vmovdqa [edi], ymm0
        add     edi, SIMD_BYTES
        loop    .lop
else if SIMD eq AVX512
        vmovdqa64 zmm1, zword [sub_mask]
.lop:
        vmovdqa64 zmm0, [edi]
        vpsubusb zmm0, zmm0, zmm1
        vmovdqa64 [edi], zmm0
        add     edi, SIMD_BYTES
        loop    .lop
end if
}

macro blur_prepare
{
	mov     ecx, [Screen_H]
	dec     ecx
	dec     ecx
	imul    ecx, [Screen_W]
	sub     ecx, SIMD_BYTES*2
	shr     ecx, 3 ;ecx = (Screen_W * (Screen_H - 2) - SIMD_BYTES*2) / SIMD_BYTES
	mov     edi, SIMD_BYTES
	add     edi, [buffer]
	add     edi, [Screen_W]
}

; eax = [Screen_W]
macro blur
{
local .lop
if SIMD eq SSE
.lop:
        movq    mm0, [edi]
        movq    mm1, [edi + 1]
        movq    mm2, [edi - 1]
        movq    mm3, mm0
		neg     eax
        movq    mm4, [edi + eax]
		neg     eax
        movq    mm5, [edi + eax]

        pavgb   mm0, mm1 ; mm0 = avg(cur,cur+1)
        pavgb   mm3, mm2 ; mm3 = avg(cur,cur-1)
        pavgb   mm4, mm5 ; mm4 = avg(cur+width,cur-width)
        pavgb   mm3, mm4 ; mm3 = avg(avg(cur,cur-1),avg(cur+width,cur-width))
        pavgb   mm0, mm3 ; mm0 = avg(avg(cur,cur+1),

        movq    [edi], mm0
        add     edi, SIMD_BYTES
        loop    .lop
else if SIMD eq AVX
.lop:
        vmovdqa xmm0, [edi]
        vmovdqa xmm1, xmm0
		neg     eax
        vmovdqa xmm2, [edi + eax]
		neg     eax

        vpavgb  xmm0, xmm0, [edi + 1]
        vpavgb  xmm1, xmm1, [edi - 1]
		vpavgb  xmm2, xmm2, [edi + eax]
        vpavgb  xmm1, xmm1, xmm2
        vpavgb  xmm0, xmm0, xmm1

        vmovdqa [edi], xmm0
        add     edi, SIMD_BYTES
        loop    .lop
else if SIMD eq AVX2
.lop:
        vmovdqa ymm0, [edi]
        vmovdqa ymm1, ymm0
		neg     eax
        vmovdqa ymm2, [edi + eax]
		neg     eax

        vpavgb  ymm0, ymm0, [edi + 1]
        vpavgb  ymm1, ymm1, [edi - 1]
        vpavgb  ymm2, ymm2, [edi + eax]
        vpavgb  ymm1, ymm1, ymm2
        vpavgb  ymm0, ymm0, ymm1

        vmovdqa [edi], ymm0
        add     edi, SIMD_BYTES
        loop    .lop
else if SIMD eq AVX512
.lop:
        vmovdqa64 zmm0, [edi]
        vmovdqa64 zmm1, zmm0
		neg     eax
        vmovdqa64 zmm2, [edi + eax]
		neg     eax

        vpavgb  zmm0, zmm0, [edi + 1]
        vpavgb  zmm1, zmm1, [edi - 1]
        vpavgb  zmm2, zmm2, [edi + eax]
        vpavgb  zmm1, zmm1, zmm2
        vpavgb  zmm0, zmm0, zmm1

        vmovdqa64 [edi], zmm0
        add     edi, SIMD_BYTES
        loop    .lop
end if
}

; eax = [Screen_W]
macro blur_right
{
local .lop
if SIMD eq SSE
  .lop:
        movq    mm0, [edi]
        movq    mm1, [edi + 1]
        movq    mm2, [edi + eax]
        movq    mm3, [edi + eax + 1]
        pavgb   mm0, mm1
        pavgb   mm3, mm2
        pavgb   mm0, mm3
        movq    [edi], mm0
        add     edi, SIMD_BYTES
        loop    .lop
else if SIMD eq AVX
  .lop:
        vmovdqa xmm0, [edi]
        vmovdqu xmm1, [edi + eax + 1]
		vpavgb  xmm2, xmm0, [edi + 1]
        vpavgb  xmm3, xmm1, [edi + eax]
		vpavgb  xmm4, xmm2, xmm3
        vmovdqa [edi], xmm4
        add     edi, SIMD_BYTES
        loop    .lop
else if SIMD eq AVX2
  .lop:
        vmovdqa ymm0, [edi]
        vmovdqu ymm1, [edi + eax + 1]
		vpavgb  ymm2, ymm0, [edi + 1]
        vpavgb  ymm3, ymm1, [edi + eax]
		vpavgb  ymm4, ymm2, ymm3
        vmovdqa [edi], ymm4
        add     edi, SIMD_BYTES
        loop    .lop
else if SIMD eq AVX512
  .lop:
        vmovdqa64 zmm0, [edi]
        vmovdqu64 zmm1, [edi + eax + 1]
        vpavgb  zmm2, zmm0, [edi + 1]
        vpavgb  zmm3, zmm1, [edi + eax]
		vpavgb  zmm4, zmm2, zmm3
        vmovdqa64 [edi], zmm4
        add     edi, SIMD_BYTES
        loop    .lop
end if 
}

STARTAPP:

init_palette:
        mov     edi, pal
        xor     eax, eax
@@:
        stosd
        stosd
        add     eax, 0x040000
        and     eax, 0xFFFFFF
        jnz     @b

        mov     eax, 63*4 SHL 16
@@:
        stosd
        stosd
        add     ax, 0x0404
        jnc     @b

	;init buffer
	mcall SF_SYS_MISC,SSF_HEAP_INIT
	mov ecx,[Screen_W]
	imul ecx,[Screen_H]
	mcall SF_SYS_MISC,SSF_MEM_ALLOC
	mov [buffer],eax

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Main Functions
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
virtual at esp
        global_x        dd ?
        global_y        dd ?
        seed            dd ?
end virtual

        rdtsc
        push    eax      ; seed
        push    100 * 64 ; global_y
        push    160 * 64 ; global_x

   jmp MAIN

align 4
OnResize:
	mov ecx,[Screen_W]
	imul ecx,[Screen_H]
	;ecx = SCREEN_W*SCREEN_H
	mcall SF_SYS_MISC,SSF_MEM_REALLOC,,[buffer]
	ret

align 4
red:
        mcall   SF_THREAD_INFO, proc_info, -1
areacolor = 0x53224466
        mcall   SF_REDRAW, SSF_BEGIN_DRAW
        mcall   SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
		add     eax, 4
		push    eax ;for test resize
		add     eax, [Screen_H]
        lea     ecx, [(70 shl 16) + eax]
		mov     ebx, [Screen_W]
		lea     ebx, [(100 shl 16) + 9 + ebx]
        mcall   SF_CREATE_WINDOW,,, areacolor,, window_title
		
	;test resize
	pop eax
	cmp dword[proc_info.box.height],0
	je .resize_end
	sub eax,[proc_info.box.height]
	neg eax
	cmp eax,[Screen_H]
	je .end_h
	cmp eax,32 ;min height
	jge @f
		mov eax,32
	@@:
		mov [Screen_H],eax
		xor eax,eax
		mov [Screen_W],eax
	.end_h:
	
	mov eax,[proc_info.box.width]
	sub eax,9
	mov [lost_bytes],eax
	and eax,0xffff-(SIMD_BYTES-1)
	cmp eax,[Screen_W]
	je .resize_end
	cmp eax,64 ;min width
	jge @f
		mov eax,64
	@@:
	mov [Screen_W],eax

	call OnResize
	.resize_end:
		
	mcall   SF_REDRAW, SSF_END_DRAW ; end of redraw

align 4
MAIN:
        test    [proc_info.wnd_state], 0x04
        jnz     still
        mov     ecx, NUM_PARTS
        mov     ebp, particles
  .advance_particles:
        mov     eax, [ebp + X_OFFSET]
        mov     ebx, [ebp + Y_OFFSET]

        sar     eax, 6
        sar     ebx, 6

        cmp     eax, 5
        jb      .new_particle
        mov     edx, [Screen_W]
		sub     edx, 5
		cmp     eax, edx
        jge     .new_particle
        cmp     ebx, 5
        jb      .new_particle
		mov     edx, [Screen_H]
		sub     edx, 5
        cmp     ebx, edx
        jl      .part_ok

  .new_particle:
        call    init_particle
        jmp     .advance_particles

  .part_ok:
		mov     edi, ebx
        imul    edi, [Screen_W]
        mov     dl, [ebp+COLOR_OFFSET]
		add     eax,[buffer]
        mov     [eax+edi], dl

        mov     eax, [ebp+X_SPEED_OFFSET]
        add     [ebp+X_OFFSET], eax
        mov     eax, [ebp+Y_SPEED_OFFSET]
        add     [ebp+Y_OFFSET], eax

        rdtsc
        and     al, 0x7F
        jnz     .dont_inc_y_speed
        inc     dword [ebp+Y_SPEED_OFFSET]
    .dont_inc_y_speed:

        add     ebp, PART_SIZE
        loop    .advance_particles

        shade
;       jmp     .copy_buffer_to_video
        blur_prepare
		mov     eax, [Screen_W]
        test    dword [blur_right_flag] , 0x800000
        jnz     .do_blur_right
        blur
        rdtsc
        and     al, 1
        jz      .blur_ok
        jmp     .dont_blur
    .do_blur_right:
        blur_right
    .blur_ok:
        add     dword [blur_right_flag], 0x1000
    .dont_blur:

    .copy_buffer_to_video:

	mcall   SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
	lea     edx, [(5 shl 16) + eax]
	push    8
	pop     esi
	xor     ebp, ebp
	mov     ecx, [Screen_W]
	shl     ecx, 16
	add     ecx, [Screen_H]
	mcall   SF_PUT_IMAGE_EXT, [buffer],,,, pal

	and     [lost_bytes], SIMD_BYTES-1
	jz      still
	mcall   SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
	mov     ecx, eax
	shl     ecx, 16
	add     ecx, [Screen_H]
	mov     ebx, [Screen_W]
	add     ebx, 5 ;left border
	shl     ebx, 16
	add     ebx, [lost_bytes]
	xor     edx, edx
	mcall   SF_DRAW_RECT

align 4
still:      
	mcall   SF_WAIT_EVENT_TIMEOUT, 1

	dec     eax                 ; redraw request ?
	jz      red
	dec     eax                 ; key in buffer ?
	jz      key
	dec     eax                 ; button in buffer ?
	jz      button

	jmp     MAIN

align 4
key:
        mcall   SF_GET_KEY
;       cmp     ah, 1               ; Test Esc in Scan
;       je      close_app
        cmp     ah, 27              ; Test Esc in ASCII
        je      close_app
        jmp     MAIN

button:
; we have only one button, close
close_app:
        mcall   SF_TERMINATE_PROCESS

init_particle:
        rdtsc
        and     al, 0x1F
        jnz     .dont_re_init_globals
        ; init x
        call    rand
        cdq
        ;xor dx, dx
        mov     ebx, [Screen_W]
        div     ebx
        shl     edx, 6
        mov     [4 + global_x], edx
        ; init y
        call    rand
        cdq
        ;xor    dx, dx
        mov     ebx, [Screen_H]
        div     ebx
        shl     edx, 6
        mov     [4 + global_y], edx
  .dont_re_init_globals:
        ; init x
        mov     eax, [4 + global_x]
        mov     [ebp + X_OFFSET], eax
        ; init y
        mov     eax, [4 + global_y]
        mov     [ebp + Y_OFFSET], eax
        ; init x speed
        call    rand
        and     eax, 31
        sub     eax, 15
        ;shl    ax, 6
        mov     [ebp + X_SPEED_OFFSET], eax
        ; init y speed
        call    rand
        and     eax, 31
        sub     eax, 15
        ;shl    ax, 6
        mov     [ebp + Y_SPEED_OFFSET], eax
        ; init color
        mov     [ebp + COLOR_OFFSET], dword 255
        ret

rand:
        mov     eax, [8 + seed]
        imul    eax, 214013
        add     eax, 2531011
        mov     [8 + seed], eax
        shr     eax, 16
        ret

; DATA AREA
window_title    db 'Firework',0
align SIMD_BYTES
sub_mask        db SIMD_BYTES dup 0x01
;                             x, y, x_speed, y_speed, color
particles: times NUM_PARTS dd 0, 0, 0,       0,       0
blur_right_flag dd 0
I_END:
proc_info       process_information
align 16
pal             rb 256 * 4
align SIMD_BYTES
buffer          dd 0
align 4
	rd 1024
stacktop:
E_END: