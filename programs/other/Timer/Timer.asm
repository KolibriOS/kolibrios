; Timer with three buttons ;
; %define lang "ru"
; %define lang "it"
ORG 0
BITS 32
; ---------------------------------------------------------------------------- ;
STACK_SIZE     equ 256

BUTTON_START   equ 2
BUTTON_PAUSE   equ 3
BUTTON_RESET   equ 4

BORDER_SIZE    equ 5
MARGIN         equ 8
BUTTON_MARGIN  equ 3
BUTTON_PADDING equ 5

CHAR_WIDTH     equ 6
CHAR2_WIDTH    equ 8
CHAR_HEIGHT    equ 9

BUTTON_START_WIDTH equ (BUTTON_PADDING * 2) + (sz_start.end - sz_start) * CHAR_WIDTH
BUTTON_PAUSE_WIDTH equ (BUTTON_PADDING * 2) + (sz_pause.end - sz_pause) * CHAR_WIDTH
BUTTON_RESET_WIDTH equ (BUTTON_PADDING * 2) + (sz_reset.end - sz_reset) * CHAR_WIDTH

BUTTON_HEIGHT  equ (BUTTON_PADDING * 2) + CHAR_HEIGHT + 1

WINDOW_WIDTH   equ (BORDER_SIZE * 2) + (MARGIN * 2) + (BUTTON_MARGIN * 2) + (BUTTON_START_WIDTH + BUTTON_PAUSE_WIDTH + BUTTON_RESET_WIDTH)

TIME_AREA_WIDTH equ (CHAR2_WIDTH * 8) ; HH MM SS
TIME_AREA_LEFT  equ (WINDOW_WIDTH - BORDER_SIZE * 2 - TIME_AREA_WIDTH) / 2
TIME_AREA_TOP   equ MARGIN + 1

TEXT_ON_BUTTONS_TOP equ (MARGIN + CHAR_HEIGHT + MARGIN + BUTTON_PADDING) + 1

BUTTON_START_BACK_COLOR equ 0x880000
BUTTON_PAUSE_BACK_COLOR equ 0x008800
BUTTON_RESET_BACK_COLOR equ 0x000088
; ---------------------------------------------------------------------------- ;
MENUET01       db 'MENUET01'
version        dd 1
program.start  dd START
program.end    dd END
program.memory dd END + STACK_SIZE
program.stack  dd END + STACK_SIZE
program.params dd 0
program.path   dd 0
; ---------------------------------------------------------------------------- ;
screen:
.height        dw 0
.width         dw 0
; ---------------------------------------------------------------------------- ;
window:
.left          dd 0
.top           dd 0
.width         dd WINDOW_WIDTH
.height        dd 0
; ---------------------------------------------------------------------------- ;
sc:
.frames           dd 0
.grab             dd 0
.work_dark        dd 0
.work_light       dd 0
.grab_text        dd 0
.work             dd 0
.work_button      dd 0
.work_button_text dd 0
.work_text        dd 0
.work_graph       dd 0
; ---------------------------------------------------------------------------- ;
timer_ticks       dd 0
last_timer_ticks  dd 0
hours             dd 0
minutes           dd 0
seconds           dd 0
timer_proc        dd EmptyProc ; at start Timer yet disabled
; ---------------------------------------------------------------------------- ;
Events:
        dd     On_Idle
        dd     On_Redraw
        dd     On_KeyPress
        dd     On_ButtonPress
; ---------------------------------------------------------------------------- ;
ButtonEvents:
        dd     0
.close  dd     On_ButtonClose
.start  dd     On_ButtonStart
.pause  dd     EmptyProc       ; at start Pause must not work
.reset  dd     On_ButtonReset
; ---------------------------------------------------------------------------- ;
%ifidn     lang, "ru"
               sz_timer       db "Таймер",0

               sz_start:      db "старт"
                 .end:        db 0
               sz_pause:      db "пауза"
                 .end:        db 0
               sz_reset:      db "сброс"
                 .end:        db 0
%elifidn   lang, "it"
               sz_timer       db "Timer",0

               sz_start:      db "lancio"
                 .end:        db 0
               sz_pause:      db "pausa"
                 .end:        db 0
               sz_reset:      db "reset"
                 .end:        db 0
%else
               sz_timer       db "Timer",0

               sz_start:      db "start"
                 .end:        db 0
               sz_pause:      db "pause"
                 .end:        db 0
               sz_reset:      db "reset"
                 .end:        db 0
%endif
; ---------------------------------------------------------------------------- ;
START:
; get.screen.size
        mov    eax, 61
        mov    ebx, 1
        int    64
        mov    [screen], eax

        movzx  eax, word[screen.width]
        sub    eax, [window.width]
        shr    eax, 1
        mov    [window.left], eax

; skin.height
        mov    eax, 48
        mov    ebx, 4
        int    64
        add    eax, (MARGIN * 3) + BORDER_SIZE + BUTTON_HEIGHT + CHAR_HEIGHT
        mov    [window.height], eax

        movzx  eax, word[screen.height]
        sub    eax, [window.height]
        shr    eax, 1
        mov    [window.top], eax

        call On_Redraw
; ---------------------------------------------------------------------------- ;
wait.event:
        mov    eax, 23
        mov    ebx, 50
        int    64
        call   [eax * 4 + Events]
        jmp    wait.event
; ---------------------------------------------------------------------------- ;
On_KeyPress:
        mov    eax, 2
        int    64
        ret
; ---------------------------------------------------------------------------- ;
On_ButtonPress:
        mov    eax, 17
        int    64
        movzx  eax, ah
        call   [eax * 4 + ButtonEvents]
        ret
; ---------------------------------------------------------------------------- ;
On_ButtonClose:
        or     eax, -1
        int    64
; ---------------------------------------------------------------------------- ;
On_ButtonStart:
; get system counter
        mov    eax, 26
        mov    ebx, 9
        int    64
        sub    eax, [last_timer_ticks]
        and    [last_timer_ticks], dword 0
        mov    [timer_ticks], eax
        mov    [timer_proc], dword TimerStarted
        mov    [ButtonEvents.start], dword EmptyProc      ; disable ButtonStart
        mov    [ButtonEvents.pause], dword On_ButtonPause ; enable  ButtonPause
        ret
; ---------------------------------------------------------------------------- ;
On_ButtonPause:
; get system counter
        mov    eax, 26
        mov    ebx, 9
        int    64
        sub    eax, [timer_ticks]
        mov    [last_timer_ticks], eax
        mov    [timer_proc], dword EmptyProc
        mov    [ButtonEvents.start], dword On_ButtonStart ; enable  ButtonStart
        mov    [ButtonEvents.pause], dword EmptyProc      ; disable ButtonPause
        ret
; ---------------------------------------------------------------------------- ;
On_ButtonReset:
        xor    eax, eax
        mov    [seconds], eax
        mov    [hours], eax
        mov    [minutes], eax
        mov    [last_timer_ticks], eax
        mov    [timer_proc], dword EmptyProc
        mov    [ButtonEvents.start], dword On_ButtonStart ; enable  ButtonStart
        mov    [ButtonEvents.pause], dword EmptyProc      ; disable ButtonPause
        ret
; ---------------------------------------------------------------------------- ;
On_Redraw:
; redraw.start
        mov    eax, 12
        mov    ebx, 1
        int    64
; get.standart.colors
        mov    eax, 48
        mov    ebx, 3
        mov    ecx, sc
        mov    edx, 40
        int    64
; skin.height
        mov    eax, 48
        mov    ebx, 4
        int    64
        add    eax, (MARGIN * 3) + BORDER_SIZE + BUTTON_HEIGHT + CHAR_HEIGHT
        mov    [window.height], eax
; draw.window
        xor    eax, eax
        mov    ebx, [window.left]
        shl    ebx, 16
        add    ebx, [window.width]
        mov    ecx, [window.top]
        shl    ecx, 16
        add    ecx, [window.height]
        mov    edx, [sc.work]
        or     edx, 0x34000000
        mov    edi, sz_timer
        int    64
; draw.buttons:
        mov    eax, 8
        mov    ecx, ((MARGIN + CHAR_HEIGHT + MARGIN) << 16) | BUTTON_HEIGHT

        mov    ebx, (MARGIN << 16) | BUTTON_START_WIDTH
        mov    edx, BUTTON_START
        mov    esi, BUTTON_START_BACK_COLOR
        int    64

        mov    ebx, ((MARGIN + BUTTON_START_WIDTH + BUTTON_MARGIN) << 16) | BUTTON_PAUSE_WIDTH
        mov    edx, BUTTON_PAUSE
        mov    esi, BUTTON_PAUSE_BACK_COLOR
        int    64

        mov    ebx, ((MARGIN + BUTTON_START_WIDTH + BUTTON_MARGIN + BUTTON_PAUSE_WIDTH + BUTTON_MARGIN) << 16) | BUTTON_RESET_WIDTH
        mov    edx, BUTTON_RESET
        mov    esi, BUTTON_RESET_BACK_COLOR
        int    64
;----------------------
; draw.texts:
        mov    eax, 4
        mov    ecx, 0x80FFFFFF

        mov    ebx, ((MARGIN + BUTTON_PADDING + 1) << 16) | TEXT_ON_BUTTONS_TOP
        mov    edx, sz_start
        int    64

        mov    ebx, ((MARGIN + BUTTON_START_WIDTH + BUTTON_MARGIN + BUTTON_PADDING + 1) << 16) | TEXT_ON_BUTTONS_TOP
        mov    edx, sz_pause
        int    64

        mov    ebx, ((MARGIN + BUTTON_START_WIDTH + BUTTON_MARGIN + BUTTON_PAUSE_WIDTH + BUTTON_MARGIN + BUTTON_PADDING + 1) << 16) | TEXT_ON_BUTTONS_TOP
        mov    edx, sz_reset
        int    64
;----------------------
        call   On_Idle
; redraw.finish
        mov    eax, 12
        mov    ebx, 2
        int    64
        ret
; ---------------------------------------------------------------------------- ;
DrawTime:
        mov    ebx, (1 << 16)
        mov    esi, [sc.work_text]
        mov    edi, [sc.work]
; HH
        mov    eax, [hours]
        xor    edx, edx
        mov    ebp, 10
        div    ebp
        mov    ebp, edx

        mov    ecx, eax ; [hours] first digit

        or     esi, 0x70000000
        mov    eax, 47
        mov    edx, (TIME_AREA_LEFT << 16) | TIME_AREA_TOP
        int    64
        and    esi, 0x30FFFFFF
        mov    eax, 47
        mov    edx, ((TIME_AREA_LEFT + 1) << 16) | TIME_AREA_TOP
        int    64

        mov    ecx, ebp ; [hours] second digit

        or     esi, 0x70000000
        mov    eax, 47
        mov    edx, ((TIME_AREA_LEFT + 1 + CHAR2_WIDTH) << 16) | TIME_AREA_TOP
        int    64
        and    esi, 0x30FFFFFF
        mov    eax, 47
        mov    edx, ((TIME_AREA_LEFT + 1 + CHAR2_WIDTH + 1) << 16) | TIME_AREA_TOP
        int    64
; MM
        mov    eax, [minutes]
        xor    edx, edx
        mov    ebp, 10
        div    ebp
        mov    ebp, edx

        mov    ecx, eax ; [minutes] first digit

        or     esi, 0x70000000
        mov    eax, 47
        mov    edx, ((TIME_AREA_LEFT + CHAR2_WIDTH * 3) << 16) | TIME_AREA_TOP
        int    64
        and    esi, 0x30FFFFFF
        mov    eax, 47
        mov    edx, ((TIME_AREA_LEFT + CHAR2_WIDTH * 3 + 1) << 16) | TIME_AREA_TOP
        int    64

        mov    ecx, ebp ; [minutes] second digit

        or     esi, 0x70000000
        mov    eax, 47
        mov    edx, ((TIME_AREA_LEFT + 1 + CHAR2_WIDTH * 3 + CHAR2_WIDTH) << 16) | TIME_AREA_TOP
        int    64
        and    esi, 0x30FFFFFF
        mov    eax, 47
        mov    edx, ((TIME_AREA_LEFT + 1 + CHAR2_WIDTH * 3 + CHAR2_WIDTH + 1) << 16) | TIME_AREA_TOP
        int    64
; SS
        mov    eax, [seconds]
        xor    edx, edx
        mov    ebp, 10
        div    ebp
        mov    ebp, edx

        mov    ecx, eax ; [seconds] first digit

        or     esi, 0x70000000
        mov    eax, 47
        mov    edx, ((TIME_AREA_LEFT + CHAR2_WIDTH * 6) << 16) | TIME_AREA_TOP
        int    64
        and    esi, 0x30FFFFFF
        mov    eax, 47
        mov    edx, ((TIME_AREA_LEFT + CHAR2_WIDTH * 6 + 1) << 16) | TIME_AREA_TOP
        int    64

        mov    ecx, ebp ; [seconds] second digit

        or     esi, 0x70000000
        mov    eax, 47
        mov    edx, ((TIME_AREA_LEFT + 1 + CHAR2_WIDTH * 6 + CHAR2_WIDTH) << 16) | TIME_AREA_TOP
        int    64
        and    esi, 0x30FFFFFF
        mov    eax, 47
        mov    edx, ((TIME_AREA_LEFT + 1 + CHAR2_WIDTH * 6 + CHAR2_WIDTH + 1) << 16) | TIME_AREA_TOP
        int    64
        ret
; ---------------------------------------------------------------------------- ;
On_Idle:
        call   dword [timer_proc]
        call   DrawTime
        ret
; ---------------------------------------------------------------------------- ;
TimerStarted:
; get system counter
        mov    eax, 26
        mov    ebx, 9
        int    64
        sub    eax, [timer_ticks]
        xor    edx, edx
        mov    ecx, 100
        div    ecx
        mov    ecx, 60
        xor    edx, edx
        div    ecx
        mov    [seconds], edx
        xor    edx, edx
        div    ecx
        mov    [hours], eax
        mov    [minutes], edx
        ret
; ---------------------------------------------------------------------------- ;
EmptyProc:
        ret
; ---------------------------------------------------------------------------- ;
END: