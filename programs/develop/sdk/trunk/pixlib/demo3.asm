
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

macro _read_file path, offset, data, count
{
   mov eax, dword path
   mov ebx, dword data
   mov ecx, dword offset
   mov edx, dword count

   push 0
   push 0
   mov [esp+1], eax
   push ebx
   push edx
   push 0
   push ecx
   push 0
   mov ebx, esp
   mov eax, 70
   int 0x40
   add esp, 28
}

use32

db 'MENUET01'
dd 1
dd start
dd i_end
dd mem
dd mem
dd 0
dd app_path

include 'pixlib.inc'

align 4
start:
           fix_cwd app_path

           call load_pxlib
           test eax, eax
           jz .fail

           mov  eax,48                      ; get system colors
           mov  ebx,3
           mov  ecx,sc
           mov  edx,10*4
           int 0x40

           CreatePixmap 640, 384, ARGB32, PX_MEM_LOCAL   ; animation
           mov [pix_0], eax
           test eax, eax
           jz .fail

           CreatePixmap 64, 64, ARGB32, PX_MEM_LOCAL     ; saved screen
           mov [pix_1], eax
           test eax, eax
           jz .fail

           CreatePixmap 64, 64, ARGB32, PX_MEM_LOCAL     ; back buffer
           mov [pix_2], eax

           Blit [pix_1], 0,0, SCR_PIXMAP, 0,0, 64,64

           LockPixmap [pix_0]
              mov ebx, eax
              _read_file szfile, 128, ebx, 640*384*4
           UnlockPixmap [pix_0]


.redraw:
           call draw_window

.wait_event:

           mov ebx, 2
           mov eax, 23
           int 0x40


           dec eax                  ;   if event = 1
           jz  .redraw              ;   jump to redraw handler
           dec eax                  ;   else if event = 2
           jz  .key                 ;   jump to key handler
           dec eax
           jz  .button

           Blit   [pix_2], 0,0, [pix_1], 0,0, 64,64

           mov eax, [count]
           inc eax
           cmp eax, 60
           jb @F
           xor eax, eax
@@:
           mov [count], eax

           xor edx, edx
           mov ebx, 10
           div ebx

           shl eax, 6
           shl edx, 6

           TransparentBlit [pix_2], 0,0, [pix_0], edx,eax, 64,64 ,0xFF000000
           Blit SCR_PIXMAP, 0,0, [pix_2], 0, 0, 64,64

           jmp .wait_event

.button:                               ; button event handler
           mov al, 17                  ;   get button identifier
           int 0x40

           cmp ah, 1
           jne .wait_event             ;   return if button id != 1
.exit:
                                       ; restore old screen and cleanup

           Blit SCR_PIXMAP, 0,0, [pix_1], 0, 0, 64,64

           DestroyPixmap [pix_2]
           DestroyPixmap [pix_1]
           DestroyPixmap [pix_0]
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
           mov  ebx, 200*65536+200     ; (window_cx)*65536+(window_sx)
           mov  ecx, 200*65536+100     ; (window_cy)*65536+(window_sy)
           mov  edx, [sc.work]         ; work area color
           or   edx, 0x33000000        ; & window type 3
           mov  edi, title             ; window title
           int  0x40

           mov  eax, 12                ; finish drawing
           mov  ebx, 2
           int  0x40

           ret

align 4

count       dd  0

title       db  'Transparent blit',0

szfile      db  'donut.dds',0

i_end:

align 4

pix_0       rd 1
pix_1       rd 1
pix_2       rd 1

sc   system_colors


align 4

app_path:

rb 2048 ; stack
mem:

