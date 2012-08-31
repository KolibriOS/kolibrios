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

; Ethernet protocol numbers
ETHER_PPP_DISCOVERY     = 0x6388
ETHER_PPP_SESSION       = 0x6488

; PPP protocol numbers
PPP_IPv4                = 0x2100
PPP_LCP                 = 0x21c0

; PPP Active Discovery...
PPPoE_PADI      = 0x09  ; .. Initiation
PPPoE_PADO      = 0x07  ; .. Offer
PPPoE_PADR      = 0x19  ; .. Request
PPPoE_PADS      = 0x65  ; .. Session-confirmation
PPPoE_PADT      = 0xa7  ; .. Terminate

TAG_EOL         = 0x0000
TAG_SERVICE_NAME= 0x0101
TAG_AC_NAME     = 0x0201
TAG_HOST_UNIQ   = 0x0301
TAG_AC_COOKIE   = 0x0401

LCP_config_request      = 1
LCP_config_ack          = 2
LCP_config_nak          = 3
LCP_config_reject       = 4
LCP_terminate_request   = 5
LCP_terminate_ack       = 6
LCP_code_reject         = 7
LCP_protocol_reject     = 8
LCP_echo_request        = 9
LCP_echo_reply          = 10
LCP_discard_request     = 11

struct  ETH_frame
        DestMac         dp ?
        SrcMac          dp ?
        Type            dw ?
ends

struct  PPPoE_frame     ETH_frame
        VersionAndType  db ?
        Code            db ?
        SessionID       dw ?
        Length          dw ?            ; Length of payload, does NOT include the length PPPoE header.
        Payload         rb 0
ends

struct  PPP_frame       PPPoE_frame
        Protocol        dw ?
ends

struct  LCP_frame       PPP_frame
        LCP_Code        db ?
        LCP_Identifier  db ?
        LCP_Length      dw ?
        LCP_Data        rb 0
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
        mcall   send, [socketnum], PADI, PADI.length, 0

mainloop:
        mcall   10

        call    [con_get_flags]
        test    eax, 0x200                      ; con window closed?
        jnz     close_conn

        mcall   recv, [socketnum], buffer, 4096
        cmp     eax, sizeof.PPPoE_frame
        jb      mainloop

        cmp     word [buffer + ETH_frame.Type], ETHER_PPP_SESSION
        je      LCP_input

        cmp     word [buffer + ETH_frame.Type], ETHER_PPP_DISCOVERY
        jne     mainloop

        cmp     [buffer + PPPoE_frame.Code], PPPoE_PADO
        je      pado

        cmp     [buffer + PPPoE_frame.Code], PPPoE_PADS
        je      pads

        cmp     [buffer + PPPoE_frame.Code], PPPoE_PADT
        je      padt

        jmp     mainloop

pado:

        push    str2
        call    [con_write_asciiz]

        lea     esi, [buffer + ETH_frame.SrcMac]                ; source mac -> dest mac
        lea     edi, [buffer + ETH_frame.DestMac]
        movsw
        movsd

        mov     byte [buffer + PPPoE_frame.Code], PPPoE_PADR    ; change packet type to PADR

        mov     al, byte [buffer + PPPoE_frame.Length + 1]      ; get packet size
        mov     ah, byte [buffer + PPPoE_frame.Length + 0]
        movzx   esi, ax
        add     esi, sizeof.PPPoE_frame

        mcall   send, [socketnum], buffer, , 0  ; now send it!

        jmp     mainloop


pads:

        push    str3
        call    [con_write_asciiz]

        mov     edx, dword [buffer + ETH_frame.SrcMac]                ; source mac -> dest mac
        mov      si, word [buffer + ETH_frame.SrcMac + 4]
        mov     dword [PADT.mac], edx
        mov     word [PADT.mac + 4], si

        mov     cx, word [buffer + PPPoE_frame.SessionID]       ; and Session ID
        mov     [PADT.sid], cx

        mcall   76, API_PPPOE + 0               ; Start PPPoE session

        jmp     mainloop

padt:

        push    str4
        call    [con_write_asciiz]

        mcall   76, API_PPPOE + 1

exit:
        mcall   close, [socketnum]
        mcall   -1


close_conn:

        mcall   send, [socketnum], PADT, 14 + 6, 0
        jmp     exit


LCP_input:

        cmp     word [buffer + PPP_frame.Protocol], PPP_LCP
        jne     mainloop

        cmp     [buffer + LCP_frame.LCP_Code], LCP_echo_request
        je      .echo

  .dump:
        jmp     mainloop

  .echo:
        mov     [buffer + LCP_frame.LCP_Code], LCP_echo_reply

        push    dword [buffer + ETH_frame.DestMac]
        push    dword [buffer + ETH_frame.SrcMac]
        pop     dword [buffer + ETH_frame.DestMac]
        pop     dword [buffer + ETH_frame.SrcMac]
        push    word [buffer + ETH_frame.DestMac + 4]
        push    word [buffer + ETH_frame.SrcMac + 4]
        pop     word [buffer + ETH_frame.DestMac + 4]
        pop     word [buffer + ETH_frame.SrcMac + 4]

        mov     esi, eax
        mcall   send, [socketnum], buffer, , 0  ; now send it!

        jmp     mainloop

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
        dw      20 shl 8

        dw      TAG_SERVICE_NAME
        dw      0x0000

        dw      TAG_HOST_UNIQ
        dw      0x0c00          ; 12 bytes long
        dd      0xdead          ; some random id
        dd      0xbeef
        dd      0x1337

        .length = $ - PADI

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
sid             dw ?
buffer          rb 4096
                rb 4096    ; stack
mem:
