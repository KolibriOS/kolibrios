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

include '../../../macros.inc'
SCREEN_WIDTH   = 320
SCREEN_HEIGHT  = 200
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
if SIMD eq SSE
        mov     ecx, SCREEN_WIDTH * SCREEN_HEIGHT / SIMD_BYTES
        mov     edi, buffer
        movq    mm1, qword [sub_mask]
  .lop:
        movq    mm0, [edi]
        psubusb mm0, mm1
        movq    [edi], mm0
        add     edi, SIMD_BYTES
        loop    .lop
else if SIMD eq AVX
        mov     ecx, SCREEN_WIDTH * SCREEN_HEIGHT / SIMD_BYTES
        mov     edi, buffer
        vmovdqa xmm1, xword [sub_mask]
  .lop:
        vmovdqa xmm0, [edi]
        vpsubusb xmm0, xmm0, xmm1
        vmovdqa [edi], xmm0
        add     edi, SIMD_BYTES
        loop    .lop
else if SIMD eq AVX2
        mov     ecx, SCREEN_WIDTH * SCREEN_HEIGHT / SIMD_BYTES
        mov     edi, buffer
        vmovdqa ymm1, yword [sub_mask]
  .lop:
        vmovdqa ymm0, [edi]
        vpsubusb ymm0, ymm0, ymm1
        vmovdqa [edi], ymm0
        add     edi, SIMD_BYTES
        loop    .lop
else if SIMD eq AVX512
        mov     ecx, SCREEN_WIDTH * SCREEN_HEIGHT / SIMD_BYTES
        mov     edi, buffer
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
        mov     ecx, (SCREEN_WIDTH * SCREEN_HEIGHT - SCREEN_WIDTH * 2 - SIMD_BYTES*2) / SIMD_BYTES
        mov     edi, buffer + SCREEN_WIDTH + SIMD_BYTES
}

macro blur
{
local .lop
if SIMD eq SSE
.lop:
        movq    mm0, [edi]
        movq    mm1, [edi + 1]
        movq    mm2, [edi - 1]
        movq    mm3, mm0
        movq    mm4, [edi - SCREEN_WIDTH]
        movq    mm5, [edi + SCREEN_WIDTH]

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
        vmovdqa xmm2, [edi - SCREEN_WIDTH]

        vpavgb  xmm0, xmm0, [edi + 1]
        vpavgb  xmm1, xmm1, [edi - 1]
        vpavgb  xmm2, xmm2, [edi + SCREEN_WIDTH]
        vpavgb  xmm1, xmm1, xmm2
        vpavgb  xmm0, xmm0, xmm1

        vmovdqa [edi], xmm0
        add     edi, SIMD_BYTES
        loop    .lop
else if SIMD eq AVX2
.lop:
        vmovdqa ymm0, [edi]
        vmovdqa ymm1, ymm0
        vmovdqa ymm2, [edi - SCREEN_WIDTH]

        vpavgb  ymm0, ymm0, [edi + 1]
        vpavgb  ymm1, ymm1, [edi - 1]
        vpavgb  ymm2, ymm2, [edi + SCREEN_WIDTH]
        vpavgb  ymm1, ymm1, ymm2
        vpavgb  ymm0, ymm0, ymm1

        vmovdqa [edi], ymm0
        add     edi, SIMD_BYTES
        loop    .lop
else if SIMD eq AVX512
.lop:
        vmovdqa64 zmm0, [edi]
        vmovdqa64 zmm1, zmm0
        vmovdqa64 zmm2, [edi - SCREEN_WIDTH]

        vpavgb  zmm0, zmm0, [edi + 1]
        vpavgb  zmm1, zmm1, [edi - 1]
        vpavgb  zmm2, zmm2, [edi + SCREEN_WIDTH]
        vpavgb  zmm1, zmm1, zmm2
        vpavgb  zmm0, zmm0, zmm1

        vmovdqa64 [edi], zmm0
        add     edi, SIMD_BYTES
        loop    .lop
end if
}

macro blur_right
{
local .lop
if SIMD eq SSE
  .lop:
        movq    mm0, [edi]
        movq    mm1, [edi + 1]
        movq    mm2, [edi + SCREEN_WIDTH]
        movq    mm3, [edi + SCREEN_WIDTH + 1]
        pavgb   mm0, mm1
        pavgb   mm3, mm2
        pavgb   mm0, mm3
        movq    [edi], mm0
        add     edi, SIMD_BYTES
        loop    .lop
else if SIMD eq AVX
  .lop:
        vmovdqa xmm0, [edi]
        vmovdqu xmm1, [edi + SCREEN_WIDTH + 1]
        vpavgb  xmm2, xmm0, [edi + 1]
        vpavgb  xmm3, xmm1, [edi + SCREEN_WIDTH]
        vpavgb  xmm4, xmm2, xmm3
        vmovdqa [edi], xmm4
        add     edi, SIMD_BYTES
        loop    .lop
else if SIMD eq AVX2
  .lop:
        vmovdqa ymm0, [edi]
        vmovdqu ymm1, [edi + SCREEN_WIDTH + 1]
        vpavgb  ymm2, ymm0, [edi + 1]
        vpavgb  ymm3, ymm1, [edi + SCREEN_WIDTH]
        vpavgb  ymm4, ymm2, ymm3
        vmovdqa [edi], ymm4
        add     edi, SIMD_BYTES
        loop    .lop
else if SIMD eq AVX512
  .lop:
        vmovdqa64 zmm0, [edi]
        vmovdqu64 zmm1, [edi + SCREEN_WIDTH + 1]
        vpavgb  zmm2, zmm0, [edi + 1]
        vpavgb  zmm3, zmm1, [edi + SCREEN_WIDTH]
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
red_loop:
        stosd
        stosd
        add     eax, 0x040000
        and     eax, 0xFFFFFF
        jnz     red_loop

        mov     eax, 63*4 SHL 16
@@:
        stosd
        stosd
        add     ax, 0x0404
        jnc     @b

;zero_buffer:
        mov     ecx, SCREEN_WIDTH * SCREEN_HEIGHT / 4
;       mov     edi, buffer
        xor     eax, eax
        rep     stosd

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


red:
        mcall   9, proc_info, -1
x = 100
y = 70
xsize = SCREEN_WIDTH+9
ysize = SCREEN_HEIGHT+4
areacolor = 0x54224466
        mov     eax, 12                 ; function 12:tell os about windowdraw
        mov     ebx, 1                  ; 1, start of draw
        int     0x40
        mov     eax, 48
        mov     ebx, 4
        int     0x40
        lea     ecx, [(y SHL 16) + ysize + eax]
        xor     eax, eax                ; function 0 : define and draw window
        mov     ebx, (x SHL 16) + xsize ; [x start] *65536 + [x size]
        mov     edx, areacolor          ; color of work area RRGGBB
        mov     edi, window_title
        int     0x40
        mov     eax, 12                 ; end of redraw
        mov     ebx, 2
        int     0x40

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
        cmp     eax, SCREEN_WIDTH - 5
        jge     .new_particle
        cmp     ebx, 5
        jb      .new_particle
        cmp     ebx, SCREEN_HEIGHT - 5
        jl      .part_ok

  .new_particle:
        call    init_particle
        jmp     .advance_particles

  .part_ok:
        imul    edi, ebx, SCREEN_WIDTH
        mov     dl, [ebp+COLOR_OFFSET]
        mov     [buffer+eax+edi], dl

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

        mcall   48, 4
        lea     edx, [(5 SHL 16) + eax]

        mov     eax, 65
        mov     ebx, buffer
        mov     ecx, (SCREEN_WIDTH SHL 16) + SCREEN_HEIGHT
        push    8
        pop     esi
        mov     edi, pal
        xor     ebp, ebp
        int     0x40


still:
        mov     eax, 11             ; Test if there is an event in the queue.
        int     0x40

        dec     eax                   ; redraw request ?
        jz      red
        dec     eax                   ; key in buffer ?
        jz      key
        dec     eax                   ; button in buffer ?
        jz      button

        jmp     MAIN


key:
        mov     eax, 2
        int     0x40
;       cmp     ah, 1               ; Test Esc in Scan
;       je      close_app
        cmp     ah, 27              ; Test Esc in ASCII
        je      close_app
        jmp     MAIN

button:
; we have only one button, close
close_app:
        mov     eax, -1         ; close this program
        int     0x40

init_particle:
        rdtsc
        and     al, 0x1F
        jnz     .dont_re_init_globals
        ; init x
        call    rand
        cdq
        ;xor dx, dx
        mov     ebx, SCREEN_WIDTH
        div     ebx
        shl     edx, 6
        mov     [4 + global_x], edx
        ; init y
        call    rand
        cdq
        ;xor    dx, dx
        mov     ebx, SCREEN_HEIGHT
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
window_title    db 'Firework demo',0
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
buffer          rb SCREEN_WIDTH * SCREEN_HEIGHT
E_END:
rd 0x200
stacktop:
