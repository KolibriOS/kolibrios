;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;;  kperf.asm - simple iperf-style TCP throughput tester           ;;
;;                for KolibriOS                                    ;;
;;                                                                 ;;
;;  Usage:                                                         ;;
;;    kperf -c <ip>     connect to a server and blast data         ;;
;;    kperf -s          listen, accept, and receive data           ;;
;;                                                                 ;;
;;  Compatible with real iperf2 in "-C" (compatibility) mode:      ;;
;;      Linux side, receive test:  iperf -s -p 5001 -i 1           ;;
;;      Linux side, send test:     iperf -c <kolibri-ip> -p 5001 \ ;;
;;                                        -C -i 1                  ;;
;;  No iperf header is sent or expected in either direction - it's ;;
;;  a raw byte-count throughput test, matching -C behaviour.       ;;
;;                                                                 ;;
;;  Socket API and command-line parsing patterns verified against  ;;
;;  tcpserv.asm and telnet.asm (KolibriOS team, hidnplayr).        ;;
;;  The interval/summary reporting math below is new code and has ;;
;;  not been run - sanity check the numbers on first use.          ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format binary as ""

PORT            = 5001         ; classic iperf2 default port
SENDBUFSIZE     = 32768
RECVBUFSIZE     = 32768
DURATION_CS     = 1000         ; client test duration: 10.00 sec (centiseconds)
REPORT_CS       = 100          ; report interval: 1.00 sec (centiseconds)

use32
; standard header
        db      'MENUET01'      ; signature
        dd      1               ; header version
        dd      start           ; entry point
        dd      i_end           ; initialized size
        dd      mem+4096        ; required memory
        dd      mem+4096        ; stack pointer
        dd      cmdline         ; parameters - kernel writes ASCIIZ args here
        dd      0               ; path

include '../../macros.inc'
purge mov,add,sub
include '../../proc32.inc'
include '../../dll.inc'
include '../../network.inc'

; ======================================================================
; entry point
; ======================================================================
start:
        stdcall dll.Load, @IMPORT
        test    eax, eax
        jnz     exit

        invoke  con_start, 1
        invoke  con_init, 80, 25, 80, 25, title

        mcall   40, EVM_STACK

        cmp     byte[cmdline], 0
        jne     .have_args
        invoke  con_write_asciiz, str_usage
        jmp     done
  .have_args:

        mov     esi, cmdline
        cmp     byte[esi], '-'
        jne     .modechar
        inc     esi
  .modechar:
        lodsb
        cmp     al, 's'
        je      server_mode
        cmp     al, 'c'
        je      client_prep
        invoke  con_write_asciiz, str_usage
        jmp     done

; ======================================================================
; client mode - parse IP, connect, blast data
; ======================================================================
client_prep:
  .skipsp:
        cmp     byte[esi], ' '
        jne     .gotip
        inc     esi
        jmp     .skipsp
  .gotip:
        push    esp
        invoke  getaddrinfo, esi, 0, 0, esp
        pop     edi
        test    eax, eax
        jnz     dns_err

        mov     eax, [edi+addrinfo.ai_addr]
        mov     eax, [eax+sockaddr_in.sin_addr]
        mov     [sockaddr1.ip], eax
        invoke  freeaddrinfo, edi

        invoke  con_write_asciiz, str_connecting
        invoke  inet_ntoa, [sockaddr1.ip]
        invoke  con_write_asciiz, eax
        invoke  con_write_asciiz, str_nl

        mov     ax, PORT
        xchg    al, ah                  ; host->network byte order
        mov     [sockaddr1.port], ax

        mcall   socket, AF_INET4, SOCK_STREAM, 0
        cmp     eax, -1
        je      socket_err
        mov     [socketnum], eax

        mcall   connect, [socketnum], sockaddr1, 18
        test    eax, eax
        jnz     socket_err

        invoke  con_write_asciiz, str_connected

; --- timing setup ---
        mcall   26, 9
        mov     [start_time], eax
        mov     [last_report], eax
        mov     dword[interval_bytes], 0
        mov     dword[total_bytes], 0

  .sendloop:
        invoke  con_get_flags
        test    eax, 0x200
        jnz     .client_closed

        mcall   send, [socketnum], sendbuf, SENDBUFSIZE, 0
        cmp     eax, -1
        je      .client_closed
        ;cmp     eax, 0                 ; 0 means buffer is full
        ;je      .client_closed

        add     [interval_bytes], eax
        add     [total_bytes], eax

        mcall   26, 9
        mov     ecx, eax                ; ecx = now
        mov     ebx, ecx
        sub     ebx, [last_report]      ; ebx = elapsed since last report
        cmp     ebx, REPORT_CS
        jl      .check_dur

        mov     edx, [last_report]
        sub     edx, [start_time]       ; edx = interval start (rel. to test start)
        stdcall print_interval, [interval_bytes], ebx, edx, ecx
        mov     [last_report], ecx
        mov     dword[interval_bytes], 0

  .check_dur:
        mcall   26, 9
        mov     ebx, eax
        sub     ebx, [start_time]
        cmp     ebx, DURATION_CS
        jl      .sendloop

        mcall   26, 9
        mov     ebx, eax
        sub     ebx, [start_time]
        stdcall print_summary, [total_bytes], ebx
        jmp     .client_done

  .client_closed:
        invoke  con_write_asciiz, str_conn_closed
        mov     ebx, [last_report]
        sub     ebx, [start_time]
        stdcall print_summary, [total_bytes], ebx

  .client_done:
        mcall   close, [socketnum]
        jmp     done

; ======================================================================
; server mode - listen, accept, receive data
; ======================================================================
server_mode:
        mcall   socket, AF_INET4, SOCK_STREAM, 0
        cmp     eax, -1
        je      socket_err
        mov     [socketnum], eax

        mov     ax, PORT
        xchg    al, ah
        mov     [sockaddr1.port], ax
        mov     dword[sockaddr1.ip], 0

        mcall   bind, [socketnum], sockaddr1, 18
        cmp     eax, -1
        je      socket_err

        mcall   listen, [socketnum], 1
        cmp     eax, -1
        je      socket_err

        invoke  con_write_asciiz, str_listening

        mcall   accept, [socketnum], sockaddr1, 18
        cmp     eax, -1
        je      socket_err
        mov     [socketnum2], eax

        invoke  con_write_asciiz, str_accepted
        invoke  inet_ntoa, [sockaddr1.ip]
        invoke  con_write_asciiz, eax
        invoke  con_write_asciiz, str_nl

        mcall   26, 9
        mov     [start_time], eax
        mov     [last_report], eax
        mov     dword[interval_bytes], 0
        mov     dword[total_bytes], 0

  .recvloop:
        invoke  con_get_flags
        test    eax, 0x200
        jnz     .server_closed

        mcall   recv, [socketnum2], recvbuf, RECVBUFSIZE, 0
        cmp     eax, -1
        je      .server_closed
        cmp     eax, 0
        je      .server_closed

        add     [interval_bytes], eax
        add     [total_bytes], eax

        mcall   26, 9
        mov     ecx, eax
        mov     ebx, ecx
        sub     ebx, [last_report]
        cmp     ebx, REPORT_CS
        jl      .recvloop

        mov     edx, [last_report]
        sub     edx, [start_time]
        stdcall print_interval, [interval_bytes], ebx, edx, ecx
        mov     [last_report], ecx
        mov     dword[interval_bytes], 0
        jmp     .recvloop

  .server_closed:
        invoke  con_write_asciiz, str_conn_closed
        mcall   26, 9
        mov     ebx, eax
        sub     ebx, [start_time]
        stdcall print_summary, [total_bytes], ebx

        mcall   close, [socketnum2]
        jmp     done

; ======================================================================
; error handlers
; ======================================================================
socket_err:
        invoke  con_write_asciiz, str_sockerr
        jmp     done

dns_err:
        invoke  con_write_asciiz, str_dnserr
        jmp     done

done:
        invoke  con_getch2
        invoke  con_exit, 1
exit:
        mcall   -1


; ======================================================================
; print_interval: bytes, interval_cs, range_start_cs, range_end_cs
;   Prints "S.CC-S.CC sec   X.XX Mbits/sec"
; ======================================================================
proc print_interval uses eax ebx ecx edx, pbytes, pinterval, prange_start, prange_end
        stdcall fmt_cs, [prange_start]
        invoke  con_write_asciiz, linebuf
        invoke  con_write_asciiz, str_dash
        stdcall fmt_cs, [prange_end]
        invoke  con_write_asciiz, linebuf
        invoke  con_write_asciiz, str_sec_tab

        stdcall fmt_mbps, [pbytes], [pinterval]
        invoke  con_write_asciiz, linebuf
        invoke  con_write_asciiz, str_mbits_nl
        ret
endp

; ======================================================================
; print_summary: bytes, total_cs
;   Prints "0.00-S.CC sec (total)  X.XX Mbits/sec"
; ======================================================================
proc print_summary uses eax ebx ecx edx, pbytes, pinterval
        invoke  con_write_asciiz, str_zero
        invoke  con_write_asciiz, str_dash
        stdcall fmt_cs, [pinterval]
        invoke  con_write_asciiz, linebuf
        invoke  con_write_asciiz, str_sec_total

        stdcall fmt_mbps, [pbytes], [pinterval]
        invoke  con_write_asciiz, linebuf
        invoke  con_write_asciiz, str_mbits_nl
        ret
endp

; ======================================================================
; fmt_cs: format a centisecond dword value as "S.CC" into linebuf
; ======================================================================
proc fmt_cs uses eax ebx edx edi, pcs
        mov     eax, [pcs]
        mov     ebx, 100
        xor     edx, edx
        div     ebx                     ; eax = whole seconds, edx = centiseconds
        push    edx                     ; save centiseconds
        mov     edi, linebuf
        stdcall itoa, eax, edi
        mov     edi, eax                ; itoa returns pointer to end of written digits
        mov     byte[edi], '.'
        inc     edi
        pop     eax                     ; eax = centiseconds (0-99)
        mov     ebx, 10
        xor     edx, edx
        div     ebx                     ; eax = tens digit, edx = ones digit
        add     al, '0'
        mov     [edi], al
        inc     edi
        add     dl, '0'
        mov     [edi], dl
        inc     edi
        mov     byte[edi], 0
        ret
endp

; ======================================================================
; fmt_mbps: format (bytes, interval_cs) as "X.XX" Mbits/sec into linebuf
;   Mbps*100 = (bytes * 8) / (interval_cs * 100)
; ======================================================================
proc fmt_mbps uses ebx ecx edx edi, pbytes, pinterval
        mov     eax, [pbytes]
        cmp     eax, 0x1FFFFFFF         ; guard against *8 overflow at extreme rates
        jb      .noshift
        shr     eax, 4                  ; scale down bytes and interval together
        mov     ecx, [pinterval]
        shr     ecx, 0                  ; (kept for clarity; no shift on interval)
        jmp     .have_scaled
  .noshift:
        mov     ecx, [pinterval]
  .have_scaled:
        mov     ebx, 8
        mul     ebx                     ; edx:eax = bytes*8
        mov     ebx, ecx
        imul    ebx, 100                ; ebx = interval_cs * 100
        cmp     ebx, 0
        jnz     .divok
        mov     ebx, 1                  ; avoid div-by-zero on a zero-length interval
  .divok:
        div     ebx                     ; eax = Mbps*100

        mov     edi, linebuf
        mov     ebx, 100
        xor     edx, edx
        div     ebx                     ; eax = whole Mbps, edx = fractional (0-99)
        push    edx
        stdcall itoa, eax, edi
        mov     edi, eax
        mov     byte[edi], '.'
        inc     edi
        pop     eax
        mov     ebx, 10
        xor     edx, edx
        div     ebx
        add     al, '0'
        mov     [edi], al
        inc     edi
        add     dl, '0'
        mov     [edi], dl
        inc     edi
        mov     byte[edi], 0
        ret
endp

; ======================================================================
; itoa: unsigned dword -> decimal ASCII, no leading zeros ("0" for zero)
;   in: value, dest buffer pointer
;   out: eax = pointer to the byte AFTER the last digit written
;        (buffer is NOT null-terminated by this routine)
; ======================================================================
proc itoa uses ebx ecx edx esi, value, dest
        mov     eax, [value]
        mov     edi, [dest]
        mov     ecx, 0                  ; digit count
        mov     ebx, 10
        test    eax, eax
        jnz     .divloop
        mov     byte[edi], '0'
        inc     edi
        mov     eax, edi
        ret
  .divloop:
        xor     edx, edx
        div     ebx
        push    edx
        inc     ecx
        test    eax, eax
        jnz     .divloop
  .popl:
        pop     edx
        add     dl, '0'
        mov     [edi], dl
        inc     edi
        loop    .popl
        mov     eax, edi
        ret
endp


; data
title           db      'kperf',0

str_usage       db      'Usage: kperf -c <ip>   (client)',10,\
                        '       kperf -s        (server)',10,10,0

str_connecting  db      'Connecting to ',0
str_connected   db      'Connected. Starting test...',10,10,0
str_listening   db      'Listening on port 5001...',10,0
str_accepted    db      'Accepted connection from ',0
str_conn_closed db      10,'Connection closed.',10,0
str_sockerr     db      'Socket error.',10,10,0
str_dnserr      db      'Could not parse address.',10,10,0
str_nl          db      10,0
str_dash        db      '-',0
str_zero        db      '0.00',0
str_sec_tab     db      ' sec   ',0
str_sec_total   db      ' sec (total)   ',0
str_mbits_nl    db      ' Mbits/sec',10,0

sockaddr1:
        dw      AF_INET4
.port   dw      0
.ip     dd      0
        rb      10

align 4
@IMPORT:

library network, 'network.obj', console, 'console.obj'
import  network,        \
        getaddrinfo,    'getaddrinfo',  \
        freeaddrinfo,   'freeaddrinfo', \
        inet_ntoa,      'inet_ntoa'
import  console,        \
        con_start,      'START',        \
        con_init,       'con_init',     \
        con_write_asciiz,       'con_write_asciiz',     \
        con_exit,       'con_exit',     \
        con_getch2,     'con_getch2',   \
        con_get_flags,  'con_get_flags'

i_end:

socketnum       dd ?
socketnum2      dd ?
start_time      dd ?
last_report     dd ?
interval_bytes  dd ?
total_bytes     dd ?

linebuf         rb 32
cmdline         rb 256

sendbuf         rb SENDBUFSIZE
recvbuf         rb RECVBUFSIZE

align 4
rb 4096         ; stack
mem:
