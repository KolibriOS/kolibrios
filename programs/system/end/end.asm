; SPDX-License-Identifier: NOASSERTION
;
; END - the shutdown dialog of KolibriOS
;
; Offers kernel restart / reboot / power off and optionally saves the
; RAM-disk before leaving: when the "save changes" checkbox is on, the
; image is written back to the boot medium (SF 18.6 with ecx = 0).
; A failed save cancels the shutdown so the user never loses changes
; silently.
;
; Authors: Veliant, Leency, Heavyiron, Burer

use32
org 0

db 'MENUET01'
dd 1
dd START
dd IM_END
dd I_END
dd stacktop
dd 0
dd 0

include '../../macros.inc'
include '../../proc32.inc'
include '../../dll.inc'
include '../../KOSfuncs.inc'
include '../../encoding.inc'
include '../../gui_patterns.inc'
include '../../string.inc'

WIN_W       = 440
WIN_H       = 200
BOT_PANEL_H = 70
BUT_W       = 116
BUT_Y       = WIN_H-60

BTN_CANCEL   = 1
BTN_KERNEL   = 2
BTN_REBOOT   = 3
BTN_POWEROFF = 4
BTN_CHECKBOX = 5

; mcall 18,9 shutdown codes
SHUTDOWN_POWEROFF = 2
SHUTDOWN_REBOOT   = 3
SHUTDOWN_KERNEL   = 4

;---------------------------------------------------------------------

START:
        mcall   SF_SYS_MISC, SSF_HEAP_INIT
        mcall   SF_SYS_MISC, SSF_MEM_OPEN, checkbox_sharedname
        mov     [checkbox_img], eax

        stdcall dll.Load, importLib
        test    eax, eax
        jnz     .no_ini                 ; no libini - live without autosave
        inc     [ini_ok]
        invoke  ini_get_int, ini_file, ini_sec, key_autosave, 0
        mov     [autosave], eax
        .no_ini:

redraw:
        call    draw_window

still:
        mcall   SF_WAIT_EVENT
        dec     eax
        jz      redraw
        dec     eax
        jz      on_key
        
        ; button event
        mcall   SF_GET_BUTTON
        shr     eax, 8
        cmp     eax, BTN_CANCEL
        je      exit
        cmp     eax, BTN_KERNEL
        je      restart_kernel
        cmp     eax, BTN_REBOOT
        je      reboot
        cmp     eax, BTN_POWEROFF
        je      power_off
        cmp     eax, BTN_CHECKBOX
        je      toggle_checkbox
        jmp     still

on_key:
        mcall   SF_GET_KEY
        cmp     ah, 13                  ; Enter
        je      reboot
        cmp     ah, 19                  ; Ctrl+S
        je      toggle_checkbox
        cmp     ah, 180                 ; Home
        je      restart_kernel
        cmp     ah, 181                 ; End
        je      power_off
        cmp     ah, 27                  ; Esc
        jne     still

exit:
        mcall   SF_TERMINATE_PROCESS

;---------------------------------------------------------------------

power_off:
        mov     [shutdown_code], SHUTDOWN_POWEROFF
        jmp     save_and_shutdown

reboot:
        mov     [shutdown_code], SHUTDOWN_REBOOT
        jmp     save_and_shutdown

restart_kernel:
        mov     [shutdown_code], SHUTDOWN_KERNEL

save_and_shutdown:
        cmp     [ini_ok], 0
        je      @f
        invoke  ini_set_int, ini_file, ini_sec, key_autosave, [autosave]
        
        @@:
        cmp     [autosave], 1
        jne     .shutdown

        mcall   SF_DRAW_TEXT, <55,108>, 0x90FF990A, TEXT_SAVING

        mcall   SF_SYSTEM, SSF_RD_TO_HDD, 0     ; save back to the boot medium
        test    eax, eax
        jz      .shutdown

        .save_failed:   ; the image is not saved - cancel the shutdown
        call    draw_window
        mcall   SF_DRAW_TEXT, <55,108>, 0x90FF5050, TEXT_SAVE_FAILED
        jmp     still

        .shutdown:
        mcall   SF_SYSTEM, SSF_SHUTDOWN, [shutdown_code]
        mcall   SF_TERMINATE_PROCESS

;---------------------------------------------------------------------

toggle_checkbox:
        xor     [autosave], 1
        call    draw_checkbox
        jmp     still

;---------------------------------------------------------------------

draw_window:
        mcall   SF_REDRAW, SSF_BEGIN_DRAW

        mcall   SF_GET_SCREEN_SIZE
        movzx   ecx, ax
        shr     eax, 17
        shl     eax, 16
        lea     ebx, [eax-(WIN_W/2) shl 16+WIN_W-1]
        shr     ecx, 1
        shl     ecx, 16
        lea     ecx, [ecx-(WIN_H/2) shl 16+WIN_H-1]
        xor     eax, eax
        mov     edx, 0x41000000
        mcall                           ; borderless window centered on the screen

        DrawWideRectangle 0, 0, WIN_W, WIN_H, 2, 0xA3A7AA
        mcall   SF_DRAW_RECT, <2,WIN_W-4>, <2,WIN_H-BOT_PANEL_H-2>, 0x202020
        mcall   SF_DRAW_RECT, <2,WIN_W-4>, <WIN_H-BOT_PANEL_H-2,BOT_PANEL_H>, 0x4B4B4B

        mcall   SF_DRAW_TEXT, <30,21>, 0x91FFFFFF, TEXT_WTITLE
        mcall   SF_DRAW_TEXT, <55,70>, 0x90FFFFFF, TEXT_RDSAVE1
        mcall   SF_DRAW_TEXT, <55,86>, 0x90FFFFFF, TEXT_RDSAVE2
        mcall   SF_DRAW_TEXT, <WIN_W-23,5>, 0x81FFFFFF, TEXT_CANCEL

        mcall   SF_DEFINE_BUTTON, <WIN_W-35,32>, <2,22>, BTN_CANCEL+BT_HIDE
        mcall   SF_DEFINE_BUTTON, <32,14>, <70,14>, BTN_CHECKBOX+BT_HIDE
        mcall   SF_DEFINE_BUTTON, <47,WIN_W-47>, <68,34>, BTN_CHECKBOX+BT_HIDE+BT_NOFRAME
        DrawRectangle3D 32, 70, 14, 14, 0x606060, 0xAFAFAF
        call    draw_checkbox

        stdcall shutdown_button,  20, 0x4E91C5, BTN_KERNEL,   TEXT_KERNEL, TEXT_HOME
        stdcall shutdown_button, 160, 0x41C166, BTN_REBOOT,   TEXT_REBOOT, TEXT_ENTER
        stdcall shutdown_button, 300, 0xC75C54, BTN_POWEROFF, TEXT_OFF,    TEXT_END

        mcall   SF_REDRAW, SSF_END_DRAW
        ret

; one big colored button with a centered caption and a hotkey hint
proc shutdown_button x:dword, bgcol:dword, id:dword, caption:dword, hotkey:dword
        mov     ebx, [x]
        sub     ebx, 3
        mcall   SF_DRAW_RECT, <ebx,BUT_W+7>, <BUT_Y-3,43+6>, 0x202020
        mcall   SF_DEFINE_BUTTON, <[x],BUT_W>, <BUT_Y,43-1>, [id], [bgcol]
        stdcall string.length, [caption]
        neg     eax
        lea     ebx, [eax*4+BUT_W/2]    ; center the 8 px wide characters
        add     ebx, [x]
        mcall   SF_DRAW_TEXT, <ebx,BUT_Y+8>, 0x90FFFFFF, [caption]
        add     ebx, 1 shl 16           ; draw twice for a bold effect
        mcall
        stdcall string.length, [hotkey]
        neg     eax
        lea     ebx, [eax*3+BUT_W/2]    ; center the 6 px wide characters
        add     ebx, [x]
        shl     ebx, 16
        add     ebx, BUT_Y+26
        mcall   SF_DRAW_TEXT, , 0x80FFFFFF, [hotkey]
        ret
endp

draw_checkbox:
        cmp     [autosave], 0
        je      .unset

        cmp     [checkbox_img], 0
        je      .no_image

        mcall   SF_PUT_IMAGE, [checkbox_img], <13,13>, <33,71>
        ret

        .no_image:
        mcall   SF_DRAW_RECT, <33,13>, <71,13>, 0xFFFFFF
        mcall   SF_DRAW_RECT, <34,11>, <72,11>, 0x58C33C
        ret

        .unset:
        mcall   SF_DRAW_RECT, <33,13>, <71,13>, 0xFFFFFF
        ret

;---------------------------------------------------------------------
;---  Data  ----------------------------------------------------------
;---------------------------------------------------------------------

if lang eq ru_RU
        TEXT_WTITLE      cp866 'Завершение работы',0
        TEXT_RDSAVE1     cp866 'Сохранить изменения, сделанные в',0
        TEXT_RDSAVE2     cp866 'процессе работы в системе (Ctrl+S)',0
        TEXT_KERNEL      cp866 'Ядро',0
        TEXT_REBOOT      cp866 'Перезагрузка',0
        TEXT_OFF         cp866 'Выключение',0
        TEXT_SAVING      cp866 'Сохранение образа...',0
        TEXT_SAVE_FAILED cp866 'Не удалось сохранить образ!',0
else if lang eq es_ES
        TEXT_WTITLE      cp850 'Apagar el equipo',0
        TEXT_RDSAVE1     cp850 'Guardar los cambios hechos durante',0
        TEXT_RDSAVE2     cp850 'la sesión de trabajo (Ctrl+S)',0
        TEXT_KERNEL      cp850 'Núcleo',0
        TEXT_REBOOT      cp850 'Reiniciar',0
        TEXT_OFF         cp850 'Apagar',0
        TEXT_SAVING      cp850 'Guardando la imagen...',0
        TEXT_SAVE_FAILED cp850 'No se pudo guardar la imagen!',0
else ; Default to en_US
        TEXT_WTITLE      db 'Shutdown computer',0
        TEXT_RDSAVE1     db 'Save all changes that were done',0
        TEXT_RDSAVE2     db 'during system work (Ctrl+S)',0
        TEXT_KERNEL      db 'Kernel',0
        TEXT_REBOOT      db 'Reboot',0
        TEXT_OFF         db 'Power off',0
        TEXT_SAVING      db 'Saving RAM-drive...',0
        TEXT_SAVE_FAILED db 'Failed to save the RAM-drive!',0
end if

TEXT_HOME   db 'Home',0
TEXT_ENTER  db 'Enter',0
TEXT_END    db 'End',0
TEXT_CANCEL db 'x',0

;---------------------------------------------------------------------

ini_file     db '/sys/settings/app.ini',0
ini_sec      db 'RDSave',0
key_autosave db 'autosave',0

checkbox_sharedname db 'CHECKBOX',0

;---------------------------------------------------------------------

align 4
importLib:
library libini, 'libini.obj'

import  libini, \
        ini_get_int, 'ini_get_int', \
        ini_set_int, 'ini_set_int'

;---------------------------------------------------------------------

IM_END:

align 4
checkbox_img  rd 1
autosave      rd 1
ini_ok        rd 1
shutdown_code rd 1

;---------------------------------------------------------------------

align 32
        rb 4096
stacktop:
I_END:
