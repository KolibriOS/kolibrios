;
; Newton fractal demo
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

WS_DRAW_NOTHING        = 0x0100_0000
WS_SKINNED_FIXED       = 0x0400_0000
WS_BACKGROUND_NOREDRAW = 0x4000_0000
WS_COORD_CLIENT        = 0x2000_0000
WS_CAPTION             = 0x1000_0000

CS_MOVABLE        = 0

WINDOW_STATE_ROLLED_UP = 4

WINDOW_BORDER_SIZE = 5

;COUNT = 16
COUNT = 100

struct TSize
  SizeY dw ?
  SizeX dw ?
ends

struct TBox
  X     dd ?
  Y     dd ?
  SizeX dd ?
  SizeY dd ?
ends

struct Complex
  X dd ?
  Y dd ?
ends


proc get_newton_fractal uses ebx edi, _width, _height
locals
        Pow2_Z_X dd ?
        Pow2_D_X dd ?
        Pow2_T_Y dd ?
        Pow2_T_X dd ?
        P        dd ?
        N        dd ?
        YMax     dd ?
        XMax     dd ?
        Y        dd ?
        X        dd ?
        D        Complex
        T        Complex
        Z        Complex
        Buf      dd ?
        ImgH     dd ?
        ImgW     dd ?
endl
        mov     eax, [_width]
        mov     [ImgW], eax
        shr     eax, 1
        mov     [XMax], eax

        mov     eax, [_height]
        mov     [ImgH], eax
        shr     eax, 1
        mov     [YMax], eax

        mov     eax, [ImgH]
        dec     eax
        imul    eax, [ImgW]
        mov     [line_shift], eax

        ; allocate buffer for image
        mov     ecx, [ImgW]
        imul    ecx, [ImgH]
        mcall   68, 12, 
        mov     [Buf], eax

        mov     edi, [Buf]
        mov     eax, [YMax]
        neg     eax
        mov     [Y], eax
.next_line:
        mov     eax, [XMax]
        neg     eax
        mov     [X], eax
.next_pixel:
        mov     [N], 0
        fild    [X]
        fdiv    [Float_250]
        fstp    [Z.X]
        fild    [Y]
        fdiv    [Float_250]
        fstp    [Z.Y]
        mov     eax, [Z.X]
        mov     ecx, [Z.Y]
        mov     [D.X], eax
        mov     [D.Y], ecx
@@:
        fld     [Z.X]
        fmul    st0, st0
        fstp    [Pow2_Z_X]
        fld     [Z.Y]
        fmul    st0, st0
        fadd    [Pow2_Z_X]
        fcomp   [Max]
        fnstsw  ax
;.Break .If !(ah & 1)   ; >= Max
        test    ah, 1
        jz      @f
        fld     [D.X]
        fmul    st0, st0
        fstp    [Pow2_D_X]
        fld     [D.Y]
        fmul    st0, st0
        fadd    [Pow2_D_X]
        fcomp   [Min]
        fnstsw  ax
;.Break .If ah & 1      ; <= Min
        test    ah, 1
        jnz     @f
        mov     eax, [Z.X]
        mov     ecx, [Z.Y]
        mov     [T.X], eax
        mov     [T.Y], ecx

        fld     [T.X]
        fmul    st0, st0        ; Pow2_T_X = T.X * T.X
        fstp    [Pow2_T_X]

        fld     [T.Y]
        fmul    st0, st0
        fst     [Pow2_T_Y]      ; Pow2_T_Y = T.Y * T.Y
        fadd    [Pow2_T_X]
        fmul    st0, st0
        fstp    [P]             ; P = (Pow2_T_X + Pow2_T_Y) * (Pow2_T_X + Pow2_T_Y)

        fld     [Pow2_T_X]
        fsub    [Pow2_T_Y]
        fld     [Float_3]
        fmul    [P]
        fxch    st1
        fdivrp                  ; Z.X = (Pow2_T_X - Pow2_T_Y) / (3 * P) + 2 * T.X / 3
        fld     [Float_2]
        fmul    [T.X]
        fdiv    [Float_3]
        fxch    st1
        faddp
        fstp    [Z.X]

        fld     [P]
        fsub    [T.X]
        fdiv    [P]
        fmul    [T.Y]           ; Z.Y = (P - T.X) / P * 2 * T.Y / 3
        fmul    [Float_2]
        fdiv    [Float_3]
        fstp    [Z.Y]

        fld     [T.X]
        fsub    [Z.X]
        fabs
        fstp    [D.X]

        fld     [T.Y]
        fsub    [Z.Y]
        fabs
        fstp    [D.Y]

        inc     [N]
        cmp     [N], COUNT
        jbe     @b
@@:
        mov     eax, [N]
        mov     ecx, palette_colors
        xor     edx, edx
        div     ecx

        ; Set next pixel
        mov     [edi], dl
        mov     eax, [line_shift]
        mov     [edi+eax], dl
        inc     edi
        mov     eax, [X]
        inc     eax
        mov     [X], eax
        cmp     eax, [XMax]
        jnz     .next_pixel

        mov     eax, [ImgW]
        add     eax, eax
        sub     [line_shift], eax
        mov     eax, [Y]
        inc     eax
        mov     [Y], eax
        cmp     eax, 0
        jle     .next_line
.done:
        mov     eax, [Buf]
        ret
endp


start:
        mcall   68, 11
        cmp     dword[params], '@ss'
        setz    [screensaver]
        jnz     .windowed
        ; fullscreen
        mcall   14
        movzx   ecx, ax
        shr     eax, 16
        mov     [Window.SizeX], eax
        mov     [Window.SizeY], ecx
        xor     [window_style], WS_SKINNED_FIXED + WS_DRAW_NOTHING
        mov     [window_border_size], 0
        mov     [skin_height], 0

        mov     edi, transparent_cursor
        xor     eax, eax
        mov     ecx, 32*32
        rep     stosd
        mcall   37, 4, transparent_cursor, 2
        mov     ecx, eax
        mcall   37, 5

        mov     ebx, EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE
        jmp     .common
.windowed:
        mcall   48, 4
        mov     [skin_height], eax
        bt      eax, 0
        sbb     [Window.SizeY], 0
        mcall   14
        movzx   ecx, ax
        shr     eax, 16
        sub     eax, [Window.SizeX]
        sub     ecx, [Window.SizeY]
        shr     eax, 1
        shr     ecx, 1
        mov     [Window.X], eax
        mov     [Window.Y], ecx

        mov     ebx, EVM_REDRAW + EVM_KEY + EVM_BUTTON
.common:
        mcall   40

        mov     ebx, [Window.SizeX]
        inc     ebx
        sub     ebx, [window_border_size]
        sub     ebx, [window_border_size]
        mov     ecx, [Window.SizeY]
        inc     ecx
        sub     ecx, [skin_height]
        sub     ecx, [window_border_size]
        stdcall get_newton_fractal, ebx, ecx
        mov     [image], eax

.still:
;        mcall   11
;        dec     eax
;        jz      .redraw
;        jns     .exit
;        mcall   5, 10
        mcall   10
        dec     eax
        jnz     .exit
.redraw:
        mcall   12, 1
        mcall   0, <[Window.X],[Window.SizeX]>, <[Window.Y],[Window.SizeY]>, [window_style], 0, window_title
        mcall   9, proc_info, -1
        test    [proc_info.wnd_state], WINDOW_STATE_ROLLED_UP
        jnz     .no_draw
        inc     [proc_info.client_box.width]
        inc     [proc_info.client_box.height]
        mcall   65, [image], <[proc_info.client_box.width],[proc_info.client_box.height]>, 0, 8, palette, 0
;        call    rotate_palette
.no_draw:
        mcall   12, 2
        jmp     .still
.exit:
        mcall   -1


proc rotate_palette
        mov     esi, palette
        mov     edi, esi
        mov     ecx, palette_colors
.next_color:
        mov     ebx, 2
        lodsd
.next_channel:
        cmp     [palette_rotate_direction+ebx], 1
        jnz     .down
.up:
        add     al, 1
        jnc     .ok
        btc     word[palette_rotate_direction+ebx], 0
        sub     al, 1
.down:
        sub     al, 1
        jnc     .ok
        btc     word[palette_rotate_direction+ebx], 0
        add     al, 2
.ok:
        ror     eax, 8
        dec     ebx
        jns     .next_channel
        ror     eax, 8
        stosd
        dec     ecx
        jnz     .next_color
        ret
endp

palette:
        dd 0x00C5037D, 0x006D398B, 0x00454E99, 0x002A71AF, \
           0x000696BB, 0x00008F5A, 0x008DBB25, 0x00F3E500, \
           0x00FCC609, 0x00F28E1C, 0x00E96220, 0x00E32322
palette_colors = ($-palette)/4

;        dd 0x00C21460, 0x008601AF, 0x004424D6, 0x000247FE, \
;           0x00347C98, 0x0066B032, 0x00B2D732, 0x00FEFE33, \
;           0x00FCCC1A, 0x00FB9902, 0x00FC600A, 0x00FE2712

;        dd 0x00C21460, 0x008601AF, 0x004424D6, 0x00347C98, \
;           0x0066B032, 0x00B2D732, 0x00FCCC1A, 0x00FB9902, \
;           0x00FC600A

;        dd 0x00C21460, 0x004424D6, 0x00347C98, 0x0066B032, \
;           0x00B2D732, 0x00FEFE33, 0x00FCCC1A, 0x00FC600A, \
;           0x00FE2712

;        dd 0x00FE2712, 0x00FC600A, 0x00FCCC1A, 0x00FEFE33, \
;           0x00B2D732, 0x0066B032, 0x00347C98, 0x004424D6, \
;           0x00C21460

;        dd 0x00347C98, 0x004424D6, 0x00C21460, 0x00FE2712, \
;           0x00FC600A, 0x00FCCC1A, 0x00FEFE33, 0x00B2D732, \
;           0x0066B032

Float_250 dd 250.0
Float_3   dd 3.0
Float_2   dd 2.0

;Max dd 10000.0
;Min dd 0.0001
Max dd 10000000.0
Min dd 0.0000001

Window TBox 0, 0, 320*3+1, 240*3+1
window_style dd WS_SKINNED_FIXED + WS_COORD_CLIENT + WS_CAPTION + \
                WS_BACKGROUND_NOREDRAW
window_border_size dd WINDOW_BORDER_SIZE
image dd 0
line_shift dd 0
;palette_rotate_direction db 1, 0, 1, 0
window_title db "Newton fractal",0

i_end:
align 4
proc_info process_information

transparent_cursor rd 32*32
skin_height dd ?
screensaver db ?
params rb 0x400
align 4
rb 0x200
e_end:
