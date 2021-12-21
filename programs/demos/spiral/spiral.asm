;
; Spiral demo using Turtle graphics
;
; Written in UASM by 0CorErr
; Translated to FASM by dunkaist
;

use32
    org 0
    db  'MENUET01'
    dd  0x01,start,i_end,e_end,e_end,params,0

include 'proc32.inc'
include 'macros.inc'

struct RGB
    Blue  db ?
    Green db ?
    Red   db ?
ends

struct HSV
    Hue dw ?
    Sat db ?
    Val db ?
ends

struct Turtle
    PosX        dd ?
    PosY        dd ?
    Orientation dd ?
    PenColor    HSV
ends


proc forward _t, _d
locals
    .x1 dd ?
    .x2 dd ?
    .y1 dd ?
    .y2 dd ?
endl
        mov     eax, [_t]

        fld     [eax+Turtle.PosX]
        fistp   [.x1]

        fld     [eax+Turtle.PosY]
        fistp   [.y1]

        fld     [eax+Turtle.Orientation]
        fcos
        fmul    [_d]
        fadd    [eax+Turtle.PosX]
        fstp    [eax+Turtle.PosX]

        fld     [eax+Turtle.Orientation]
        fsin
        fmul    [_d]
        fadd    [eax+Turtle.PosY]
        fstp    [eax+Turtle.PosY]

        fld     [eax+Turtle.PosX]
        fistp   [.x2]

        fld     [eax+Turtle.PosY]
        fistp   [.y2]

        mov     ebx, [.x1]
        shl     ebx, 16
        add     ebx, [.x2]
        mov     ecx, [.y1]
        shl     ecx, 16
        add     ecx, [.y2]
        mov     edx, [color]
        mcall   38
        ret
endp


proc turn _t, _degrees
        mov     eax, [_t]
        fldpi
        fmul    [_degrees]
        fdiv    [float_180]
        fadd    [eax+Turtle.Orientation]
        fstp    [eax+Turtle.Orientation]
        ret
endp


proc reposition _t, _x, _y
        mov     eax, [_t]
        mov     ecx, [_x]
        mov     [eax+Turtle.PosX], ecx
        mov     ecx, [_y]
        mov     [eax+Turtle.PosY], ecx
        ret
endp


; H = 0..360, S = 0..255, V = 0..255  -->  R = 0..255, G = 0..255, B = 0..255
proc hsv_to_rgb uses ebx, _h, _s, _v, _rgb
locals
        .f  dd ?
        .vs dd ?
endl
        mov     ebx, [_rgb]
        cmp     [_s], 0
        jnz     @f
        mov     eax, [_v]
        mov     [ebx+RGB.Red], al
        mov     [ebx+RGB.Green], al
        mov     [ebx+RGB.Blue], al
        jmp     .end
    @@:
        cmp     [_h], 360
        jnz     @f
        mov     [_h], 0
    @@:
        mov     eax, [_v]
        mul     [_s]
        mov     [.vs], eax
        mov     eax, [_h]
        mov     ecx, 60
        xor     edx, edx
        div     ecx
        mov     [.f], edx

        test    eax, eax
        jnz     @f
        mov     eax, [_v]
        mov     [ebx+RGB.Red], al
        mov     eax, 60
        sub     eax, [.f]
        mul     [.vs]
        xor     edx, edx
        div     [dword_255_mul_60]
        mov     edx, [_v]
        sub     dl, al
        mov     [ebx+RGB.Green], dl
        mov     eax, [.vs]
        xor     edx, edx
        div     [dword_255]
        mov     edx, [_v]
        sub     dl, al
        mov     [ebx+RGB.Blue], dl
        jmp     .end
    @@:
        cmp     eax, 1
        jnz     @f
        mov     eax, [.vs]
        mul     [.f]
        xor     edx, edx
        div     [dword_255_mul_60]
        mov     edx, [_v]
        sub     dl, al
        mov     [ebx+RGB.Red], dl
        mov     eax, [_v]
        mov     [ebx+RGB.Green], al
        mov     eax, [.vs]
        xor     edx, edx
        div     [dword_255]
        mov     edx, [_v]
        sub     dl, al
        mov     [ebx+RGB.Blue], dl
        jmp     .end
    @@:
        cmp     eax, 2
        jnz     @f
        mov     eax, [.vs]
        xor     edx, edx
        div     [dword_255]
        mov     edx, [_v]
        sub     dl, al
        mov     [ebx+RGB.Red], dl
        mov     eax, [_v]
        mov     [ebx+RGB.Green], al
        mov     eax, 60
        sub     eax, [.f]
        mul     [.vs]
        xor     edx, edx
        div     [dword_255_mul_60]
        mov     edx, [_v]
        sub     dl, al
        mov     [ebx+RGB.Blue], dl
        jmp     .end
    @@:
        cmp     eax, 3
        jnz     @f
        mov     eax, [.vs]
        xor     edx, edx
        div     [dword_255]
        mov     edx, [_v]
        sub     dl, al
        mov     [ebx+RGB.Red], dl
        mov     eax, [.vs]
        mul     [.f]
        xor     edx, edx
        div     [dword_255_mul_60]
        mov     edx, [_v]
        sub     dl, al
        mov     [ebx+RGB.Green], dl
        mov     eax, [_v]
        mov     [ebx+RGB.Blue], al
        jmp     .end
    @@:
        cmp     eax, 4
        jnz     @f
        mov     eax, 60
        sub     eax, [.f]
        mul     [.vs]
        xor     edx, edx
        div     [dword_255_mul_60]
        mov     edx, [_v]
        sub     dl, al
        mov     [ebx+RGB.Red], dl
        mov     eax, [.vs]
        xor     edx, edx
        div     [dword_255]
        mov     edx, [_v]
        sub     dl, al
        mov     [ebx+RGB.Green], dl
        mov     eax, [_v]
        mov     [ebx+RGB.Blue], al
        jmp     .end
    @@:
        cmp     eax, 5
        jnz     @f
        mov     eax, [_v]
        mov     [ebx+RGB.Red], al
        mov     eax, [.vs]
        xor     edx, edx
        div     [dword_255]
        mov     edx, [_v]
        sub     dl, al
        mov     [ebx+RGB.Green], dl
        mov     eax, [.vs]
        mul     [.f]
        xor     edx, edx
        div     [dword_255_mul_60]
        mov     edx, [_v]
        sub     dl, al
        mov     [ebx+RGB.Blue], dl
    @@:
  .end:
        ret
endp


proc hue_shift _t, _n
        mov     ecx, [_t]
        movzx   eax, [ecx+Turtle.PenColor.Hue]
        add     eax, [_n]
        xor     edx, edx
        div     [dword_360]
        mov     [ecx+Turtle.PenColor.Hue], dx

        movzx   eax, [ecx+Turtle.PenColor.Hue]
        movzx   edx, [ecx+Turtle.PenColor.Sat]
        movzx   ecx, [ecx+Turtle.PenColor.Val]

        stdcall hsv_to_rgb, eax, edx, ecx, color
        ret
endp


proc query_perf
locals
        .diff    dd ?
endl
        mcall   26, 9
        sub     eax, [frame_start]
        mov     [.diff], eax
        fild    [.diff]
        fild    [freq]
        fdivrp
        fstp    [instant]
        mcall   26, 9
        mov     [frame_start], eax
        ret
endp


proc waiting
        fld     [max_frame_rate]
        fld     [instant]
        fcompp
        fstsw   ax
        sahf
        jc      @f
        inc     [sleep_time]
        jmp     .end
    @@:
        fld     [min_frame_rate]
        fld     [instant]
        fcompp
        fstsw   ax
        sahf
        jnc     .end
        cmp     [sleep_time], 0
        jz      .end
        dec     [sleep_time]
  .end:
        mcall   5, [sleep_time]
        ret
endp


proc drawing
locals
        .i    dd ?
        .n    dd ?
        .posx dd ?
        .posy dd ?
endl
        mov     [turtle.Orientation], 0
        mov     eax, [window_width]
        mov     ecx, [window_height]
        shr     eax, 1
        shr     ecx, 1
        mov     [.posx], eax
        mov     [.posy], ecx
        fild    [.posx]
        fstp    [.posx]
        fild    [.posy]
        fstp    [.posy]
        stdcall reposition, turtle, [.posx], [.posy]
        mov     [.n], 5.0
        mov     eax, [window_height]
        imul    eax, 15
        shr     eax, 4
        mov     ecx, eax
        xor     edx, edx
        div     [dword_360]
        neg     edx
        add     edx, ecx
        sub     edx, 10
        mov     [.i], edx
    @@:
        stdcall hue_shift, turtle, 1
        stdcall forward, turtle, [.n]
        stdcall turn, turtle, 72.5
        fld     [.n]
        fadd    [ndelta]
        fstp    [.n]
        dec     [.i]
        jnz     @b
        ret
endp


start:
        cmp     dword[params], '@ss'
        setz    [screensaver]
        mov     ebx, EVM_REDRAW + EVM_KEY + EVM_BUTTON
        cmovz   ebx, EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE
        mcall   40

        mov     edi, transparent_cursor
        xor     eax, eax
        mov     ecx, 32*32
        rep stosd
        mcall   37, 4, transparent_cursor, 2
        mov     ecx, eax
        mcall   37, 5

        mcall   14
        add     eax, 0x00010001
        movzx   ecx, ax
        shr     eax, 16
        mov     [window_width], eax
        mov     [window_height], ecx
        mcall   26, 9
        mov     [frame_start], eax
  .still:
        mcall   11
        dec     eax
        js      .draw_spiral    ; no event
        jnz     .quit
        mcall   12, 1
        mcall   0, [window_width], [window_height], 0, 0x01000000
        mcall   12, 2
  .draw_spiral:
        cmp     [screensaver], 0
        jz      @f
        mcall   9, proc_info, -1
        cmp     [proc_info.window_stack_position], ax
        jnz     .quit
    @@:
        stdcall query_perf
        stdcall drawing
        stdcall waiting
        jmp     .still
  .quit:
        cmp     [screensaver], 0
        jz      @f
        mcall   70, f70
    @@:
        mcall   -1


align 4
dword_255_mul_60 dd 255 * 60
dword_255        dd 255
dword_360        dd 360
float_180        dd 180.0
ndelta           dd 0.6  ; used in drawing Proc: n = n + ndelta
max_frame_rate   dd 15.0 ; to keep the FrameRate
min_frame_rate   dd 8.0  ; around min_frame_rate..max_frame_rate FPS

freq       dd 100  ; GetTickCount return count of 1/100s of second
instant    dd 0.0
sleep_time dd 0

turtle Turtle 0.0, 0.0, 0.0, <180, 255, 255>

f70:    ; run
        dd 7, 0, 0, 0, 0
        db '/sys/@SS',0
i_end:

align 4
window_width  dd ?
window_height dd ?
frame_start   dd ?
color         dd ?

proc_info process_information
params rb 4
transparent_cursor rd 32*32
screensaver db ?
align 4
rb 0x200
e_end:
