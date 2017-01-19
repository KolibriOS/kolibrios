    use32
    org     0
    db	    'MENUET01'
    dd	    1, @entry, @end, @memory, @stack, 0, 0

    include "../../macros.inc"
    include "../../proc32.inc"
;    include "../../debug.inc"
    include "../../notify.inc"
    include "../../string.inc"

 @entry:
    mcall   68, 11

 ;; RUN NOTIFY
    call    make_param
    mcall   70, fi_launch

 ;; CONVERT PID TO STR
    mov     ebx, 10
    mov     ecx, 0
  @@:
    mov     edx, 0
    div     ebx
    push    edx
    inc     ecx
    cmpne   eax, 0, @b

    mov     ebx, ctrl.name
  @@:
    pop     eax
    add     al, "0"
    mov     [ebx], al
    inc     ebx
    loop    @b

 ;; ADD POSTFIX TO STR
    mov     dword [ebx + 0], "-NOT"
    mov     dword [ebx + 4], "IFY"

 ;; OPEN CONTROLLER (0x08 + 0x01 -- CREATE AND READ/WRITE)
    mcall   68, 22, ctrl.name, 2048, 0x09
    mov     [ctrl.addr], eax

 ;; WAIT UNTIL CONTROLLER BECOMES READY TO USE
    add     eax, NTCTRL_READY
  @@:
    mcall   5, 1
    cmpe    byte [eax], 0, @b

 ;; CONFIG PBAR
    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_PBAR_MAX
    mov     dword [eax], 9

    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_PBAR_CUR
    mov     dword [eax], 9

    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_APPLY_PBAR
    mov     byte [eax], 1

 ;; LOOP OF NOTIFIES CHANGES
  @@:
  ;; SHIFT TEXT
    call    shift
  ;; ADD UNSHIFTABLE TEXT
    mov     byte [params], 0
    call    make_text

  ;; SEND TEXT TO NOTIFY
    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_TEXT
    stdcall string.copy, params, eax

  ;; APPLY NEW TEXT
    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_APPLY_TEXT
    mov     byte [eax], 1

  ;; CLOSE NOTIFY IF TIME IS END
    cmpe    byte [sz_time], "0", .exit

  ;; WAIT AND DO NEXT ITERATION
    mcall   5, 20
    jmp     @b

 .exit:
  ;; CLOSE NOTIFY
    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_CLOSE
    mov     byte [eax], 1

    mcall   -1

;-------------------------------------------------------------------------------
 shift:
 ;; SHIFT TEXT
    mov     eax, sz_text
    mov     dh, [eax]
  @@:
    mov     dl, [eax + 1]
    mov     [eax], dl
    inc     eax
    cmpne   byte [eax + 1], 0, @b
    mov     [eax], dh

    inc     byte [timer]
    cmpne   byte [timer], 5, .skip_changes
    sub     byte [timer], 5

 ;; CHANGE TIMER TEXT
    dec     byte [sz_time]

 ;; CHANGE ICON
    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_ICON
    inc     byte [eax]
    cmpne   byte [eax], 12, @f
    sub     byte [eax], 11
  @@:

 ;; APPLY NEW ICON
    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_APPLY_ICON
    mov     byte [eax], 1

 ;; CHANGE TITLE
    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_TITLE
    mov     dl, 1
    sub     dl, [eax]
    mov     [eax], dl

 ;; APPLY NEW TITLE
    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_APPLY_TITLE
    mov     byte [eax], 1

 ;; CHANGE PBAR
    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_PBAR_CUR
    dec     dword [eax]

 ;; APPLY PBAR
    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_APPLY_PBAR
    mov     byte [eax], 1

 .skip_changes:

    ret
;-------------------------------------------------------------------------------
 make_text:
    stdcall string.concatenate, sz_text, params
    stdcall string.concatenate, sz_sec_line_start, params
    stdcall string.concatenate, sz_time, params
    stdcall string.concatenate, sz_sec_line_end, params

    ret
;-------------------------------------------------------------------------------
 make_param:
    stdcall string.copy, sz_quote, params
    call    make_text
    stdcall string.concatenate, sz_quote, params
    stdcall string.concatenate, sz_flags, params

    ret
;-------------------------------------------------------------------------------

 sz_time:
    db "9", 0
 sz_text:
    db "Hello, World!!! It is a marquee!    ", 0
 sz_quote:
    db "'", 0
 sz_sec_line_start:
    db 10, "Will close after ", 0
 sz_sec_line_end:
    db " seconds", 0
 sz_flags:
    db "Idcp", 0

 fi_launch:
    dd	    7, 0, params, 0, 0
    ;db      "/usbhd0/2/svn/programs/system/notify3/notify", 0
    db	    "@notify", 0

 @end:
;=====================================================================
 timer	rb 1
 params rb 256
 ctrl:
  .name rb 32
  .addr rd 1
	rb 2048
 @stack:

 @memory:
