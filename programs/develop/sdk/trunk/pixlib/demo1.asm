
include '../../../../proc32.inc'

struc system_colors
{
  .frame            dd ?
  .grab             dd ?
  .work_dark        dd ?
  .work_light       dd ?
  .grab_text        dd ?
  .work             dd ?
  .work_button      dd ?
  .work_button_text dd ?
  .work_text        dd ?
  .work_graph       dd ?
}

use32

db 'MENUET01'
dd 1
dd start
dd i_end
dd mem
dd mem
dd 0
dd 0

include 'pixlib.inc'

align 4
start:
           call load_pxlib
           test eax, eax
           jz .fail

           sub esp, 1024

           mov eax, 9
           mov ebx, esp
           mov ecx, -1
           int 0x40

           movzx ecx, word [esp+0x1E]
           mov eax, 18
           mov ebx, 21
           int 0x40

           mov [slot], eax

           add esp, 1024

           mov  eax,48                      ; get system colors
           mov  ebx,3
           mov  ecx,sc
           mov  edx,10*4
           int 0x40
.redraw:
           call draw_window

.wait_event:

           mov eax, 18
           mov ebx, 7
           int 0x40
           cmp eax, [slot]
           jne .skip_draw

           sub esp, 1024

           mov eax, 9
           mov ebx, esp
           mov ecx, -1
           int 0x40

           mov edx, [esp+0x22]; xwin
           mov ecx, [esp+0x26]; ywin
           add edx, [esp+0x36]
           add ecx, [esp+0x3A]

           mov eax, [esp+0x3E]
           mov ebx, [esp+0x42]

           add esp, 1024

           test eax, eax
           jle .skip_draw
           test ebx, ebx
           jle .skip_draw

           push ebx
           push eax
           push ecx
           push edx
           call _Draw
           add esp, 16

.skip_draw:

        ;   mov ebx, 1
        ;   mov eax, 23
        ;   int 0x40

           mov eax, 11
           int 0x40

           dec eax                     ;   if event = 1
          js .wait_event

           jz  .redraw                 ;   jump to redraw handler
           dec eax                     ;   else if event = 2
           jz  .key                    ;   jump to key handler
           dec eax
           jz  .button

           jmp .wait_event

.button:                               ; button event handler
           mov al, 17                  ;   get button identifier
           int 0x40

           cmp ah, 1
           jne .wait_event             ;   return if button id != 1
.exit:
                                       ; restore old screen and cleanup
.fail:
           or eax, -1                  ;   exit application
           int 0x40
.key:                                  ; key event handler
           mov al, 2                   ;   get key code
           int 0x40

           jmp .wait_event

draw_window:
           mov eax, 12                 ; start drawing
           mov ebx, 1
           int 0x40

           xor  eax, eax               ; create and draw the window
           mov  ebx, 100*65536+320     ; (window_cx)*65536+(window_sx)
           mov  ecx, 100*65536+240     ; (window_cy)*65536+(window_sy)
           mov  edx, [sc.work]         ; work area color
           or   edx, 0x33000000        ; & window type 3
           mov  edi, title             ; window title
           int  0x40

           mov  eax, 12                ; finish drawing
           mov  ebx, 2
           int  0x40

           ret

DWORD equ dword
PTR   equ

_Draw:
        push    ebp
        xor     edx, edx
        push    edi
        push    esi
        push    ebx
        sub     esp, 44
        imul    eax, DWORD PTR [_glSeed], 69069
        mov     ebp, DWORD PTR [esp+72]
        mov     ecx, DWORD PTR [esp+76]
        inc     eax
        mov     DWORD PTR [esp+32], eax
        imul    eax, eax, 69069
        inc     eax
        mov     DWORD PTR [esp+36], eax
        imul    eax, eax, 69069
        inc     eax
        mov     DWORD PTR [esp+40], eax
        imul    eax, eax, 69069
        lea     ebx, [eax+1]
        mov     eax, ebx
        div     ebp
        imul    eax, ebx, 69069
        lea     ebx, [eax+1]
        mov     eax, ebx
        mov     edi, edx
        xor     edx, edx
        div     ecx
        imul    eax, ebx, 69069
        lea     ebx, [eax+1]
        mov     eax, ebx
        mov     esi, edx
        xor     edx, edx
        div     ebp
        imul    eax, ebx, 69069
        inc     eax
        mov     DWORD PTR [_glSeed], eax
        mov     ebp, edx
        mov     DWORD PTR [esp+28], edx
        xor     edx, edx
        div     ecx
        cmp     ebp, edi
        mov     ebx, edx
        jge     L2
        mov     DWORD PTR [esp+28], edi
        mov     edi, ebp
L2:
        cmp     ebx, esi
        jge     L4
        mov     ebx, esi
        mov     esi, edx
L4:
        push    eax
        mov     ecx, 255
        push    -16777216
        mov     eax, DWORD PTR [esp+40]
        xor     edx, edx
        div     ecx
        mov     eax, DWORD PTR [esp+48]
        mov     ebp, edx
        xor     edx, edx
        div     ecx
        mov     eax, DWORD PTR [esp+44]
        sal     ebp, 16
        or      ebp, -16777216
        mov     DWORD PTR [esp+8], edx
        or      ebp, edx
        xor     edx, edx
        div     ecx
        lea     eax, [ebx+1]
        sub     eax, esi
        sal     edx, 8
        or      ebp, edx
        push    ebp
        push    eax
        mov     eax, DWORD PTR [esp+44]
        inc     eax
        sub     eax, edi
        push    eax
        mov     eax, esi
        add     eax, DWORD PTR [esp+88]
        push    eax
        mov     eax, edi
        add     eax, DWORD PTR [esp+88]
        push    eax
        push    -1
        call    [imp_DrawRect]
        add     esp, 76
        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret
align 4

count       dd  0
_glSeed     dd  -365387184

title       db  'Draw rect demo',0

i_end:

align 4

slot         rd 1

sc   system_colors

align 16


rb 2048  ;stack
mem:

