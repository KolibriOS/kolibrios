; Timer with three buttons ;
; %define lang "ru_RU"
; %define lang "it_IT"
use32
	org 0
	db 'MENUET01'
version dd 1
	dd program.start
	dd program.end
	dd program.memory
	dd program.stack
	dd 0,0

include '../../macros.inc'
include '../../proc32.inc'
include '../../KOSfuncs.inc'
include 'lang.inc'
; ---------------------------------------------------------------------------- ;
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

BUTTON_START_WIDTH = (BUTTON_PADDING * 2) + (sz_start.end - sz_start) * CHAR_WIDTH
BUTTON_PAUSE_WIDTH = (BUTTON_PADDING * 2) + (sz_pause.end - sz_pause) * CHAR_WIDTH
BUTTON_RESET_WIDTH = (BUTTON_PADDING * 2) + (sz_reset.end - sz_reset) * CHAR_WIDTH

BUTTON_HEIGHT  = (BUTTON_PADDING * 2) + CHAR_HEIGHT + 1

WINDOW_WIDTH   = (BORDER_SIZE * 2) + (MARGIN * 2) + (BUTTON_MARGIN * 2) + (BUTTON_START_WIDTH + BUTTON_PAUSE_WIDTH + BUTTON_RESET_WIDTH)

TIME_AREA_WIDTH = (CHAR2_WIDTH * 8) ; HH MM SS
TIME_AREA_LEFT  = (WINDOW_WIDTH - BORDER_SIZE * 2 - TIME_AREA_WIDTH) / 2
TIME_AREA_TOP   = MARGIN + 1

TEXT_ON_BUTTONS_TOP = (MARGIN + CHAR_HEIGHT + MARGIN + BUTTON_PADDING) + 1

BUTTON_START_BACK_COLOR equ 0x880000
BUTTON_PAUSE_BACK_COLOR equ 0x008800
BUTTON_RESET_BACK_COLOR equ 0x000088
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
sc system_colors
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
if lang eq ru_RU
               sz_timer       db "Таймер",0

               sz_start:      db "старт"
                 .end:        db 0
               sz_pause:      db "пауза"
                 .end:        db 0
               sz_reset:      db "сброс"
                 .end:        db 0
else if lang eq it_IT
               sz_timer       db "Timer",0

               sz_start:      db "lancio"
                 .end:        db 0
               sz_pause:      db "pausa"
                 .end:        db 0
               sz_reset:      db "reset"
                 .end:        db 0
else
               sz_timer       db "Timer",0

               sz_start:      db "start"
                 .end:        db 0
               sz_pause:      db "pause"
                 .end:        db 0
               sz_reset:      db "reset"
                 .end:        db 0
end if
; ---------------------------------------------------------------------------- ;
align 4
program.start:
; get.screen.size
        mcall SF_GET_GRAPHICAL_PARAMS,SSF_SCREEN_SIZE
        mov    [screen], eax

        movzx  eax, word[screen.width]
        sub    eax, [window.width]
        shr    eax, 1
        mov    [window.left], eax

; skin.height
        mcall SF_STYLE_SETTINGS,SSF_GET_SKIN_HEIGHT
        add    eax, (MARGIN * 3) + BORDER_SIZE + BUTTON_HEIGHT + CHAR_HEIGHT
        mov    [window.height], eax

        movzx  eax, word[screen.height]
        sub    eax, [window.height]
        shr    eax, 1
        mov    [window.top], eax

        call On_Redraw
; ---------------------------------------------------------------------------- ;
wait.event:
        mcall SF_WAIT_EVENT_TIMEOUT, 50
        call   dword[eax * 4 + Events]
        jmp    wait.event
; ---------------------------------------------------------------------------- ;
On_KeyPress:
        mcall SF_GET_KEY
        ret
; ---------------------------------------------------------------------------- ;
On_ButtonPress:
        mcall SF_GET_BUTTON
        movzx  eax, ah
        call   dword[eax * 4 + ButtonEvents]
        ret
; ---------------------------------------------------------------------------- ;
On_ButtonClose:
        mcall SF_TERMINATE_PROCESS
; ---------------------------------------------------------------------------- ;
On_ButtonStart:
; get system counter
        mcall SF_SYSTEM_GET,SSF_TIME_COUNT
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
        mcall SF_SYSTEM_GET,SSF_TIME_COUNT
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
        mcall SF_REDRAW,SSF_BEGIN_DRAW
; get.standart.colors
        mcall SF_STYLE_SETTINGS,SSF_GET_COLORS,sc,sizeof.system_colors
; skin.height
        mcall ,SSF_GET_SKIN_HEIGHT
        add    eax, (MARGIN * 3) + BORDER_SIZE + BUTTON_HEIGHT + CHAR_HEIGHT
        mov    [window.height], eax
; draw.window
        mov    ebx, [window.left]
        shl    ebx, 16
        add    ebx, [window.width]
        mov    ecx, [window.top]
        shl    ecx, 16
        add    ecx, [window.height]
        mov    edx, [sc.work]
        or     edx, 0x34000000
        mov    edi, sz_timer
        mcall SF_CREATE_WINDOW
; draw.buttons:
        mcall SF_DEFINE_BUTTON, (MARGIN shl 16) or BUTTON_START_WIDTH, ((MARGIN + CHAR_HEIGHT + MARGIN) shl 16) or BUTTON_HEIGHT, BUTTON_START, BUTTON_START_BACK_COLOR

        mcall , ((MARGIN + BUTTON_START_WIDTH + BUTTON_MARGIN) shl 16) or BUTTON_PAUSE_WIDTH,, BUTTON_PAUSE, BUTTON_PAUSE_BACK_COLOR

        mcall , ((MARGIN + BUTTON_START_WIDTH + BUTTON_MARGIN + BUTTON_PAUSE_WIDTH + BUTTON_MARGIN) shl 16) or BUTTON_RESET_WIDTH,, BUTTON_RESET, BUTTON_RESET_BACK_COLOR
;----------------------
; draw.texts:
        mcall SF_DRAW_TEXT, ((MARGIN + BUTTON_PADDING + 1) shl 16) or TEXT_ON_BUTTONS_TOP, 0x80FFFFFF, sz_start

        mcall , ((MARGIN + BUTTON_START_WIDTH + BUTTON_MARGIN + BUTTON_PADDING + 1) shl 16) or TEXT_ON_BUTTONS_TOP,, sz_pause

        mcall , ((MARGIN + BUTTON_START_WIDTH + BUTTON_MARGIN + BUTTON_PAUSE_WIDTH + BUTTON_MARGIN + BUTTON_PADDING + 1) shl 16) or TEXT_ON_BUTTONS_TOP,, sz_reset
;----------------------
        call   On_Idle
; redraw.finish
        mcall SF_REDRAW,SSF_END_DRAW
        ret
; ---------------------------------------------------------------------------- ;
DrawTime:
        mov    ebx, (1 shl 16)
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
        mcall SF_DRAW_NUMBER,,, (TIME_AREA_LEFT shl 16) or TIME_AREA_TOP
        and    esi, 0x30FFFFFF
        mcall ,,, ((TIME_AREA_LEFT + 1) shl 16) or TIME_AREA_TOP

        mov    ecx, ebp ; [hours] second digit

        or     esi, 0x70000000
        mcall ,,, ((TIME_AREA_LEFT + 1 + CHAR2_WIDTH) shl 16) or TIME_AREA_TOP
        and    esi, 0x30FFFFFF
        mcall ,,, ((TIME_AREA_LEFT + 1 + CHAR2_WIDTH + 1) shl 16) or TIME_AREA_TOP
; MM
        mov    eax, [minutes]
        xor    edx, edx
        mov    ebp, 10
        div    ebp
        mov    ebp, edx

        mov    ecx, eax ; [minutes] first digit

        or     esi, 0x70000000
        mcall SF_DRAW_NUMBER,,, ((TIME_AREA_LEFT + CHAR2_WIDTH * 3) shl 16) or TIME_AREA_TOP
        and    esi, 0x30FFFFFF
        mcall ,,, ((TIME_AREA_LEFT + CHAR2_WIDTH * 3 + 1) shl 16) or TIME_AREA_TOP

        mov    ecx, ebp ; [minutes] second digit

        or     esi, 0x70000000
        mcall ,,, ((TIME_AREA_LEFT + 1 + CHAR2_WIDTH * 3 + CHAR2_WIDTH) shl 16) or TIME_AREA_TOP
        and    esi, 0x30FFFFFF
        mcall ,,, ((TIME_AREA_LEFT + 1 + CHAR2_WIDTH * 3 + CHAR2_WIDTH + 1) shl 16) or TIME_AREA_TOP
; SS
        mov    eax, [seconds]
        xor    edx, edx
        mov    ebp, 10
        div    ebp
        mov    ebp, edx

        mov    ecx, eax ; [seconds] first digit

        or     esi, 0x70000000
        mcall SF_DRAW_NUMBER,,, ((TIME_AREA_LEFT + CHAR2_WIDTH * 6) shl 16) or TIME_AREA_TOP
        and    esi, 0x30FFFFFF
        mcall ,,, ((TIME_AREA_LEFT + CHAR2_WIDTH * 6 + 1) shl 16) or TIME_AREA_TOP

        mov    ecx, ebp ; [seconds] second digit

        or     esi, 0x70000000
        mcall ,,, ((TIME_AREA_LEFT + 1 + CHAR2_WIDTH * 6 + CHAR2_WIDTH) shl 16) or TIME_AREA_TOP
        and    esi, 0x30FFFFFF
        mcall ,,, ((TIME_AREA_LEFT + 1 + CHAR2_WIDTH * 6 + CHAR2_WIDTH + 1) shl 16) or TIME_AREA_TOP
        ret
; ---------------------------------------------------------------------------- ;
On_Idle:
        call   dword [timer_proc]
        call   DrawTime
        ret
; ---------------------------------------------------------------------------- ;
TimerStarted:
; get system counter
        mcall SF_SYSTEM_GET,SSF_TIME_COUNT
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
align 4
program.end:
	rb 512
align 16
program.stack:
program.memory:
