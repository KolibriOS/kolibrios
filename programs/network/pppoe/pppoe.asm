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
PPP_LCP                 = 0x21c0        ; Link Configure Protocol
PPP_CBCP                = 0x29c0        ; CallBack Control Protocol
PPP_PAP                 = 0x23c0        ; Password Authenication Protocol packet
PPP_CHAP                = 0x23c2        ; Challenge Handshake Authentication Protocol
PPP_IPCP                = 0x2180        ; Internet Protocol Configure Protocol (maybe this should be in kernel?)
PPP_CCP                 = 0xfd80        ; Compression Configure Protocol

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

        mcall   recv, [socketnum], buffer, 4096, 0
        cmp     eax, sizeof.PPPoE_frame
        jb      mainloop

        cmp     word [buffer + ETH_frame.Type], ETHER_PPP_SESSION
        je      SESSION_input

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
        mcall   send, [socketnum], buffer, , 0                  ; now send it!

        jmp     mainloop


pads:

        push    str3
        call    [con_write_asciiz]

        mov     edx, dword [buffer + ETH_frame.SrcMac]          ; source mac -> dest mac
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

        mcall   76, API_PPPOE + 1               ; Stop PPPoE session

exit:
        mcall   close, [socketnum]
        mcall   -1


close_conn:

        mcall   send, [socketnum], PADT, PADT.length, 0
        jmp     exit


SESSION_input:

        mov     ax, word[buffer + PPP_frame.Protocol]

        cmp     ax, PPP_LCP
        je      LCP_input

        cmp     ax, PPP_CBCP
        je      CBCP_input

        cmp     ax, PPP_PAP
        je      PAP_input

        cmp     ax, PPP_CHAP
        je      CHAP_input

        cmp     ax, PPP_IPCP
        je      IPCP_input

        cmp     ax, PPP_CCP
        je      CCP_input

        jmp     mainloop



LCP_input:

        stdcall con_write_asciiz, str_lcp

        cmp     [buffer + LCP_frame.LCP_Code], LCP_echo_request
        je      .echo

  .dump:
        jmp     mainloop

  .echo:
        mov     [buffer + LCP_frame.LCP_Code], LCP_echo_reply

        lea     esi, [buffer + ETH_frame.SrcMac]        ; source mac -> dest mac
        lea     edi, [buffer + ETH_frame.DestMac]
        movsw
        movsd

        mov     esi, eax
        mcall   send, [socketnum], buffer, , 0          ; now send it back!

        jmp     mainloop

CBCP_input:

        stdcall con_write_asciiz, str_cbcp

        jmp     mainloop

PAP_input:

        stdcall con_write_asciiz, str_pap

        jmp     mainloop

CHAP_input:

        stdcall con_write_asciiz, str_chap

        jmp     mainloop

IPCP_input:

        stdcall con_write_asciiz, str_ipcp

        jmp     mainloop

CCP_input:

        stdcall con_write_asciiz, str_ccp

        jmp     mainloop

; data
title   db      'PPPoE',0
str1    db      'Sending PADI',13,10,0
str2    db      'Got PADO',13,10,'Sending PADR',13,10,0
str3    db      'Got PADS',13,10,'starting PPPoE session',13,10,0
str4    db      'Got PADT - connection terminated by Access Concentrator',13,10,0
str_lcp db      'Got LCP packet',13,10,0
str_cbcp db     'got CBCP packet',13,10,0
str_pap db      'got PAP packet',13,10,0
str_chap db     'got CHAP packet',13,10,0
str_ipcp db     'got IPCP packet',13,10,0
str_ccp db      'got CCP packet',13,10,0


PADI:
        dp      0xffffffffffff          ; dest mac: broadcast
        dp      0                       ; source mac (overwritten by kernel)
        dw      ETHER_PPP_DISCOVERY     ; type

        db      0x11                    ; Version and Type
        db      PPPoE_PADI              ; Code
        dw      0                       ; session ID
        dw      20 shl 8                ; Payload Length

        dw      TAG_SERVICE_NAME        ; tag
        dw      0x0000                  ; length

        dw      TAG_HOST_UNIQ           ; tag
        dw      0x0c00                  ; length = 12 bytes

        dd      0xdead                  ; some random id
        dd      0xbeef
        dd      0x1337

        .length = $ - PADI

PADT:

  .mac  dp      0                       ; Dest mac, to be filled in
        dp      0                       ; source mac (overwritten by kernel)
        dw      ETHER_PPP_DISCOVERY     ; Type

        db      0x11                    ; Version and Type
        db      PPPoE_PADT              ; Code: terminate connection
  .sid  dw      0                       ; session id, to be filled in
        dw      0                       ; PAyload length = 0

        .length = $ - PADT


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
