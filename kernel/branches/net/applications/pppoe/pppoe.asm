;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2012. All rights reserved.         ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  pppoe.asm - PPPoE dialer for KolibriOS                         ;;
;;                                                                 ;;
;;  Written by hidnplayr@kolibrios.org                             ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format binary as ""

use32

        db      'MENUET01'      ; signature
        dd      1               ; header version
        dd      start           ; entry point
        dd      i_end           ; initialized size
        dd      mem             ; required memory
        dd      mem             ; stack pointer
        dd      0               ; parameters
        dd      0               ; path

include '../macros.inc'
purge mov,add,sub
include '../proc32.inc'
include '../dll.inc'
include '../network.inc'
include '../struct.inc'

; PPP Active Discovery...
PPPoE_PADI      = 0x09  ; .. Initiation
PPPoE_PADO      = 0x07  ; .. Offer
PPPoE_PADR      = 0x19  ; .. Request
PPPoE_PADS      = 0x65  ; .. Session-confirmation
PPPoE_PADT      = 0xa7  ; .. Terminate

struct  PPPoE_frame
        VersionAndType  db ?
        Code            db ?
        SessionID       dw ?
        Length          dw ?            ; Length of payload, does NOT include the length PPPoE header.
        Payload         rb 0
ends

; entry point
start:
; load libraries
        stdcall dll.Load, @IMPORT
        test    eax, eax
        jnz     exit
; initialize console
        push    1
        call    [con_start]
        push    title
        push    25
        push    80
        push    25
        push    80
        call    [con_init]

main:
        mcall   40,  1 shl 7

        call    [con_cls]
; Welcome user
        push    str1
        call    [con_write_asciiz]

        mcall   socket, 777, 3, 666
        mov     [socketnum], eax
        mcall   send, [socketnum], PADI, 14 + 6 + 4, 0

  .recv:
        mcall   10

        call    [con_get_flags]
        test    eax, 0x200                      ; con window closed?
        jnz     close_conn

        mcall   recv, [socketnum], buffer, 4096
        cmp     eax, 20
        jb      .recv

        cmp     [buffer + 14 + PPPoE_frame.Code], PPPoE_PADO
        je      .pado

        cmp     [buffer + 14 + PPPoE_frame.Code], PPPoE_PADS
        je      .pads

        cmp     [buffer + 14 + PPPoE_frame.Code], PPPoE_PADT
        je      .padt

        jmp     .recv

  .pado:

        push    str2
        call    [con_write_asciiz]

        lea     esi, [buffer + 6]               ; source mac -> dest mac
        lea     edi, [buffer]
        movsb
        movsd

        mov     byte [buffer + 15], PPPoE_PADR  ; change packet type to PADR

        mov     al, byte [buffer + 19]          ; get packet size
        mov     ah, byte [buffer + 18]
        movzx   esi, ax
        add     esi, 20

        mcall   send, [socketnum], buffer, , 0  ; now send it!

        jmp     .recv


  .pads:

        push    str3
        call    [con_write_asciiz]

        mov     edx, dword [buffer + 6]         ; copy the MAC address
        mov     si, word [buffer + 6 +4]
        mov     dword [PADT.mac], edx
        mov     word [PADT.mac + 4], si

        mov     cx, word [buffer + 6 + 2]       ; and Session ID
        mov     [PADT.sid], cx

        mcall   75, API_PPPOE + 0               ; Start PPPoE session

        jmp     .recv

  .padt:

        push    str4
        call    [con_write_asciiz]

        mcall   75, API_PPPOE + 1

exit:
        mcall   close, [socketnum]
        mcall   -1


close_conn:

        mcall   send, [socketnum], PADT, 14 + 6, 0
        jmp     exit

; data
title   db      'PPPoE',0
str1    db      'Sending PADI',13,10,0
str2    db      'Got PADO',13,10,'Sending PADR',13,10,0
str3    db      'Got PADS',13,10,'starting PPPoE session',13,10,0
str4    db      'Got PADT - connection terminated by Access Concentrator',13,10,0


PADI:
        dp      -1              ; dest mac
        dp      0               ; source mac (overwritten by kernel)
        dw      0               ; type       (overwritten by kernel)

        db      0x11
        db      PPPoE_PADI
        dw      0               ; session ID
        dw      4 shl 8

        dw      0x0101          ; service name tag with zero length
        dw      0x0000

PADT:

  .mac  dp      0
        dp      0
        dw      0

        db      0x11
        db      PPPoE_PADT
  .sid  dw      0
        dw      0


; import
align 4
@IMPORT:

library console, 'console.obj'
import  console,        \
        con_start,      'START',        \
        con_init,       'con_init',     \
        con_write_asciiz,       'con_write_asciiz',     \
        con_exit,       'con_exit',     \
        con_gets,       'con_gets',\
        con_cls,        'con_cls',\
        con_getch2,     'con_getch2',\
        con_set_cursor_pos, 'con_set_cursor_pos',\
        con_write_string, 'con_write_string',\
        con_get_flags,  'con_get_flags'


i_end:

socketnum       dd ?
buffer          rb 4096
                rb 4096    ; stack
mem:
