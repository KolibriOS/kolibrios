;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2010. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  loopback.inc                                                   ;;
;;                                                                 ;;
;;  LoopBack device for KolibriOS                                  ;;
;;                                                                 ;;
;;    Written by hidnplayr@kolibrios.org                           ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 2891 $

iglobal

LOOPBACK_DEVICE:
        .type            dd NET_TYPE_LOOPBACK
        .mtu             dd 4096
        .name            dd .namestr

        .unload          dd .dummy_fn
        .reset           dd .dummy_fn
        .transmit        dd LOOP_input

        .bytes_tx        dq 0
        .bytes_rx        dq 0
        .packets_tx      dd 0
        .packets_rx      dd 0

        .namestr         db 'loopback', 0

        .dummy_fn:
                ret

endg

;-----------------------------------------------------------------
;
; LOOP_input
;
;  IN:   [esp+4]  = Pointer to buffer
;       [esp+8] = size of buffer
;
;  OUT: /
;
;-----------------------------------------------------------------
align 4
LOOP_input:
        pop     ebx
        pop     eax
        pop     ecx

        push    ebx
        push    ecx
        push    eax

        DEBUGF  1,"LOOP_input: size=%u\n", ecx
        lea     edx, [eax + 2]
        mov     ax, word[eax]
        mov     ebx, LOOPBACK_DEVICE

        cmp     ax, ETHER_IPv4
        je      IPv4_input

        DEBUGF  2,"LOOP_input: Unknown packet type=%x\n", ax

  .dump:
        DEBUGF  2,"LOOP_input: dumping\n"
        call    kernel_free
        add     esp, 4
        ret

;-----------------------------------------------------------------
;
; LOOP_output
;
; IN:
;     ecx = packet size
;      di = protocol
;
; OUT: edi = 0 on error, pointer to buffer otherwise
;      eax = buffer start
;      ebx = to device structure
;      ecx = unchanged (packet size of embedded data)
;      edx = size of complete buffer
;
;-----------------------------------------------------------------
align 4
LOOP_output:

        DEBUGF  1,"LOOP_output\n"

        push    ecx
        push    di

        add     ecx, 2
        cmp     ecx, [LOOPBACK_DEVICE.mtu]
        ja      .out_of_ram
        stdcall kernel_alloc, ecx
        test    eax, eax
        jz      .out_of_ram
        mov     edi, eax
        pop     ax
        stosw

        lea     eax, [edi - 2]  ; Set eax to buffer start
        pop     ecx
        lea     edx, [ecx + 2]  ; Set edx to complete buffer size
        mov     ebx, LOOPBACK_DEVICE

  .done:
        DEBUGF  2,"LOOP_output: ptr=%x size=%u\n", eax, edx
        ret

  .out_of_ram:
        DEBUGF  2,"LOOP_output: failed\n"
        add     esp, 2+4
        sub     edi, edi
        ret


