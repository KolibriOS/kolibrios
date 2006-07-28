; <--- description --->
; compiler:     FASM 1.54
; name:         MenuetOS RUN
; version:      0.02
; last update:  28/09/2004
; written by:   Ivan Poddubny
; e-mail:       ivan-yar@bk.ru


; <--- include all MeOS stuff --->
include "lang.inc"
include "macros.inc"
;;;lang fix en


; <--- start of MenuetOS application --->
MEOS_APP_START

;include "DEBUG.INC"

; <--- start of code --->
CODE
    call    draw_window            ; at first create and draw the window

  wait_event:                      ; main cycle
    mcall   10

    cmp     eax, 1                 ;   if event == 1
    je      redraw                 ;     jump to redraw handler
    cmp     eax, 2                 ;   else if event == 2
    je      key                    ;     jump to key handler
    cmp     eax, 3                 ;   else if event == 3
    je      button                 ;     jump to button handler

    jmp     wait_event             ;   else return to the start of main cycle


  redraw:                          ; redraw event handler
    call    draw_window
    jmp     wait_event


  key:                             ; key event handler
    mcall   2

    cmp     ah, 13
    je      _run
    cmp     ah, 8
    je      .backspace

    mov     bl, ah
    mov     eax, [position]
    mov     [filename + eax], bl
    inc     [position]
    call    draw_string

    jmp     wait_event

   .backspace:
    xor     eax, eax
    cmp     [position], eax
    je      wait_event
    dec     [position]
    call    draw_string
    jmp     wait_event


  button:                          ; button event handler
    mcall   17

    cmp     ah, 10
    je      _run

    dec     ah
    jne     wait_event             ;   return if button id != 1

  _exit:
    or      eax, -1                ;   exit application
    int     0x40


_run:
    mcall   58, fileinfo
;   dps     "58th function returned "
;   dpd     eax
;   newline
    jmp     _exit


draw_window:
    mcall   12, 1

    mcall   14
    and     eax, 0xFFFF
    sub     eax, 100
    shl     eax, 16
    add     eax, 80
    mov     ecx, eax

    mov     ebx, 148*65536+400     ;   (window_cx)*65536+(window_sx)
    mov     edx, 0x03DDDDDD        ;   work area color & window type 3
    mcall   0

    mov     ebx, 8*65536+8         ;   coordinates
    mov     ecx, 0x10ffffff        ;   color & font N1
    mov     edx, header            ;   address of text
    mov     esi, header.size       ;   length of text
    mcall   4

    mpack   ebx, 10, 26
    mov     ecx, 0
    mov     edx, message
    mov     esi, message.size
    mcall

    mpack   ebx, 385-(runbtn.size*6), runbtn.size*6+4
    mpack   ecx, 56, 14
    mov     edx, 10
    mov     esi, 0xa0a0a0
    mcall   8

;   mpack   ebx, 385-runbtn.size*6-findbtn.size*6-8, findbtn.size*6+4
;   inc     edx
;   mcall   8

;   mpack   ebx, 388-runbtn.size*6-findbtn.size*6-7, 59
;   mov     ecx, 0
;   mov     edx, findbtn
;   mov     esi, findbtn.size
;   mcall   4

    mpack   ebx, 388-runbtn.size*6, 59
    mov     ecx, 0
    mov     edx, runbtn
    mov     esi, runbtn.size
    mcall   4

    call    draw_string

    mcall   12, 2
ret


draw_string:
    mpack   ebx, 10, 380
    mpack   ecx, 38, 14
    mov     edx, 0xA0A0A0
    mcall   13

    mpack   ebx, 14, 41
    mov     ecx, 0
    mov     edx, filename
    mov     esi, [position]
    mcall   4
ret



; <--- initialised data --->
DATA

  position  dd filename.size

  lsz header,\
    ru, "Запуск программы",\
    en, "Start program"

  lsz message,\
    ru, "Введите путь к файлу:",\
    en, "Enter path to file:"

; lsz findbtn,\
;   ru, "Найти...",\
;   en, "Find..."

  lsz runbtn,\
    ru, "Запустить",\
    en, "Run"


  fileinfo:
    .mode      dd  16
               dd  0
    .param     dd  0
               dd  0
    .workarea  dd  workarea

  sz filename, "/rd/1/"
    rb 122


; <--- uninitialised data --->
UDATA
  workarea rb 4096


MEOS_APP_END
; <--- end of MenuetOS application --->