use32
    org 0x0
    db  'MENUET01'
    dd  0x01,start,i_end,e_end,e_end,0,0

include '../../proc32.inc'
include '../../macros.inc'


NUMCOLORS = 148         ; CSS4 named colors (http://dev.w3.org/csswg/css-color/)


start:
        mcall   66, 1, 1        ; set kbd mode to scancodes
still:
        mcall   10
        dec     eax
        jz      redraw
        dec     eax
        jz      key

button:
        mcall   17
        shr     eax, 8

        cmp     eax, 1
        jz      quit
        jmp     still

redraw:
        mcall   12, 1

        mov     ebx, [window.left]
        shl     ebx, 16
        add     ebx, [window.width]
        mov     ecx, [window.top]
        shl     ecx, 16
        add     ecx, [window.height]
        mcall   0, , , [window_style], , window_title
        mcall   9, proc_info, -1


        mov     eax, [current_color]
        mov     ebx, [proc_info.client_box.width]
        inc     ebx
        mov     ecx, [proc_info.client_box.height]
        inc     ecx
        or      edx, [colors + eax*4]
        mcall   13, , , 

        mov     eax, [current_color]
        movzx   edx, byte[colors + eax*4 + 0]
        add     dl, byte[colors + eax*4 + 1]
        adc     dh, 0
        add     dl, byte[colors + eax*4 + 2]
        adc     dh, 0
        xor     ecx, ecx
        cmp     edx, 0x80*3
        jae     @f
        add     ecx, 0xffffff
    @@:
        movzx   edx, [names + eax*2]
        movzx   esi, byte[edx]
        inc     edx
        lea     eax, [esi*3]
        add     eax, eax

        mov     ebx, [proc_info.client_box.width]
        sub     ebx, eax
        inc     ebx
        shr     ebx, 1
        shl     ebx, 16
        add     ebx, [proc_info.client_box.height]
        sub     ebx, 9
        shr     bx, 1

        mcall   4, , , 

        mcall   12, 2
        jmp     still

key:
        mcall   2
        cmp     ah, 63  ; f5
        jnz     @f
        call    toggle_fullscreen
        jmp     redraw
    @@:
        cmp     ah, 1   ; esc
        jnz     @f
        bt      [window_style], 25
        jc      quit
        call    toggle_fullscreen
        jmp     redraw
    @@:
        cmp     ah, 72  ; up
        jz      .prev
        cmp     ah, 75  ; left
        jz      .prev
        cmp     ah, 73  ; page up
        jz      .prev
        cmp     ah, 77  ; right
        jz      .next
        cmp     ah, 80  ; down
        jz      .next
        cmp     ah, 81  ; page down
        jz      .next
        jmp     still

.prev:
        dec     [current_color]
        jns     @f
        mov     [current_color], NUMCOLORS - 1
    @@:
        jmp     redraw

.next:
        inc     [current_color]
        cmp     [current_color], NUMCOLORS
        jnz     @f
        mov     [current_color], 0
    @@:
        jmp     redraw


toggle_fullscreen:
        btc     [window_style], 25
        jc      .fullscreen
        ; back from fullscreen
        mcall   67, [window.left], [window.top], [window.width], [window.height]
        jmp     .done
  .fullscreen:
        mov     eax, [proc_info.box.width]
        mov     [window.width], eax
        mov     eax, [proc_info.box.height]
        mov     [window.height], eax
        mov     eax, [proc_info.box.left]
        mov     [window.left], eax
        mov     eax, [proc_info.box.top]
        mov     [window.top], eax
        mcall   14
        mov     edx, eax
        shr     edx, 16
        movzx   eax, ax
        mov     esi, eax
        mcall   67, 0, 0, ,
  .done:
        ret

quit:
        mcall   -1


window_title    db 'Rainbow',0
window_style    dd 0x73000000
window          BOX 300, 300, 315, 184  ; left top width height
current_color   dd 0

align 4
include 'colors.asm'    ; generated data

i_end:
proc_info               process_information
rb 0x100                                        ;stack
e_end:
