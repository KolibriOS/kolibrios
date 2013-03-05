;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2010-2013. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  netstat.asm - Network Status Tool for KolibriOS                ;;
;;                                                                 ;;
;;  Written by hidnplayr@kolibrios.org                             ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format binary as ""

use32

        org    0x0

        db     'MENUET01'        ; 8 byte id
        dd     0x01              ; header version
        dd     START             ; start of code
        dd     I_END             ; size of image
        dd     (I_END+0x100)     ; memory for app
        dd     (I_END+0x100)     ; esp
        dd     I_PARAM , 0x0     ; I_Param , I_Icon

include '..\macros.inc'
include '..\network.inc'

START:
        mcall   40, EVM_REDRAW + EVM_BUTTON

redraw:
        mcall   12, 1
        mcall   0, 100 shl 16 + 600, 100 shl 16 + 240, 0x34bcbcbc, , name       ; draw window

        call    draw_interfaces

        mov     edx, 101
        mov     esi, 0x00aaaaff
        mov     edi, 0x00aaffff

        cmp     dl, [mode]
        cmove   esi, edi
        mcall   8, 25 shl 16 + 65, 25 shl 16 + 20

  .morebuttons:
        inc     edx
        add     ebx, 75 shl 16
        mov     esi, 0x00aaaaff

        cmp     dl, [mode]
        cmove   esi, edi
        mcall

        cmp     edx, 105
        jle     .morebuttons

        mcall   4, 28 shl 16 + 31, 0x80000000, modes

        cmp     [mode], 101
        jne     .no_eth

        mcall   4, 20 shl 16 + 75, 0x80000000, str_packets_tx
        add     ebx, 18
        mov     edx, str_packets_rx
        mcall
        add     ebx, 18
        mov     edx, str_bytes_tx
        mcall
        add     ebx, 18
        mov     edx, str_bytes_rx
        mcall
        add     ebx, 18
        mov     edx, str_MAC
        mcall

        mov     ebx, API_ETH + 4
        mov     bh, [device]
        mcall   76
        push    eax
        push    bx

        mov     edx, 135 shl 16 + 75 + 4*18
        call    draw_mac
        jmp     end_of_draw

 .no_eth:

        cmp     [mode], 102
        jne     .no_ip

        mcall   4, 20 shl 16 + 75, 0x80000000, str_packets_tx
        add     ebx, 18
        mov     edx, str_packets_rx
        mcall
        add     ebx, 18
        mov     edx, str_ip
        mcall
        add     ebx, 18
        mov     edx, str_dns
        mcall
        add     ebx, 18
        mov     edx, str_subnet
        mcall
        add     ebx, 18
        mov     edx, str_gateway
        mcall


        mov     ebx, API_IPv4 + 8
        mov     bh, [device]
        mcall   76
        push    eax

        dec     bl
        dec     bl
        mcall   76
        push    eax

        dec     bl
        dec     bl
        mcall   76
        push    eax

        dec     bl
        dec     bl
        mcall   76
        push    eax

        mov     edx, 135 shl 16 + 75 + 2*18
        call    draw_ip

        add     edx, 18
        call    draw_ip

        add     edx, 18
        call    draw_ip

        add     edx, 18
        call    draw_ip

        jmp     end_of_draw

 .no_ip:

        cmp     [mode], 103
        jne     .no_arp

        mcall   4, 20 shl 16 + 75, 0x80000000, str_packets_tx
        add     ebx, 18
        mov     edx, str_packets_rx
        mcall
        add     ebx, 18
        mov     edx, str_arp
        mcall
        add     ebx, 18
        mov     edx, str_conflicts
        mcall

        jmp     end_of_draw

 .no_arp:

        mcall   4, 20 shl 16 + 75, 0x80000000, str_packets_tx

        add     ebx, 18
        mov     edx, str_packets_rx
        mcall

        cmp     [mode], 106
        jne     end_of_draw

        add     ebx, 18
        mov     edx, str_missed
        mcall

        add     ebx, 18
        mov     edx, str_dumped
        mcall



end_of_draw:
        mcall   12, 2

draw_stats:

        cmp     [mode], 101
        jne     not_101

        mov     ebx, API_ETH
        mov     bh, [device]
  @@:
        push    ebx
        mcall   76
        pop     ebx
        push    eax
        inc     bl
        cmp     bl, 3
        jbe     @r

        mov     ebx, 0x000a0000
        pop     ecx
        mov     edx, 135 shl 16 + 75 + 3*18
        mov     esi, 0x40000000
        mov     edi, 0x00bcbcbc
        mcall   47

        sub     edx, 18
        pop     ecx
        mcall

        sub     edx, 18
        pop     ecx
        mcall

        sub     edx, 18
        pop     ecx
        mcall

        jmp     mainloop


not_101:

        cmp     [mode], 102
        jne     not_102

        mov     ebx, API_IPv4
        mov     bh, [device]
        push    ebx
        mcall   76
        pop     ebx
        push    eax

        inc     bl
        push    ebx
        mcall   76
        pop     ebx
        push    eax


        mov     ebx, 0x000a0000
        pop     ecx
        mov     edx, 135 shl 16 + 75 + 18
        mov     esi, 0x40000000
        mov     edi, 0x00bcbcbc
        mcall   47

        sub     edx, 18
        pop     ecx
        mcall

        jmp     mainloop


not_102:

        cmp     [mode], 103
        jne     not_103

        mov     ebx, API_ARP
        mov     bh, [device]
        push    ebx
        mcall   76
        pop     ebx
        push    eax

        inc     bl
        push    ebx
        mcall   76
        pop     ebx
        push    eax

        inc     bl
        push    ebx
        mcall   76
        pop     ebx
        push    eax

        mov     bl, 7
        push    ebx
        mcall   76
        pop     ebx
        push    eax

        mov     ebx, 0x000a0000
        pop     ecx
        mov     edx, 135 shl 16 + 75 + 3*18
        mov     esi, 0x40000000
        mov     edi, 0x00bcbcbc
        mcall   47

        sub     edx, 18
        pop     ecx
        mcall

        sub     edx, 18
        pop     ecx
        mcall

        sub     edx, 18
        pop     ecx
        mcall

        jmp     mainloop

not_103:

        cmp     [mode], 104
        jne     not_104

        mov     ebx, API_ICMP
        mov     bh, [device]
        push    ebx
        mcall   76
        pop     ebx
        push    eax

        inc     bl
        push    ebx
        mcall   76
        pop     ebx
        push    eax

        mov     ebx, 0x000a0000
        pop     ecx
        mov     edx, 135 shl 16 + 75 + 18
        mov     esi, 0x40000000
        mov     edi, 0x00bcbcbc
        mcall   47

        sub     edx, 18
        pop     ecx
        mcall

        jmp     mainloop

not_104:

        cmp     [mode], 105
        jne     not_105

        mov     ebx, API_UDP
        mov     bh, [device]
        push    ebx
        mcall   76
        pop     ebx
        push    eax

        inc     bl
        push    ebx
        mcall   76
        pop     ebx
        push    eax

        mov     ebx, 0x000a0000
        pop     ecx
        mov     edx, 135 shl 16 + 75 + 18
        mov     esi, 0x40000000
        mov     edi, 0x00bcbcbc
        mcall   47

        sub     edx, 18
        pop     ecx
        mcall

        jmp     mainloop

not_105:

        cmp     [mode], 106
        jne     not_106

        mov     ebx, API_TCP
        mov     bh, [device]
        push    ebx
        mcall   76
        pop     ebx
        push    eax

        inc     bl
        push    ebx
        mcall   76
        pop     ebx
        push    eax

        inc     bl
        push    ebx
        mcall   76
        pop     ebx
        push    eax

        inc     bl
        push    ebx
        mcall   76
        pop     ebx
        push    eax

        mov     ebx, 0x000a0000
        pop     ecx
        mov     edx, 135 shl 16 + 75 + 18*3
        mov     esi, 0x40000000
        mov     edi, 0x00bcbcbc
        mcall   47

        sub     edx, 18
        pop     ecx
        mcall

        sub     edx, 18
        pop     ecx
        mcall

        sub     edx, 18
        pop     ecx
        mcall

        jmp     mainloop

not_106:

mainloop:

        mcall   23, 50          ; wait for event with timeout    (0,5 s)

        cmp     eax, 1
        je      redraw
        cmp     eax, 3
        je      button

        jmp     draw_stats

button:                         ; button
        mcall   17              ; get id
        cmp     ah, 1
        je      exit
        cmp     ah, 0
        je      .interface
        mov     [mode], ah
        jmp     redraw

  .interface:
        shr     eax, 16
        mov     [device], al
        jmp     redraw

exit:
        mcall   -1



draw_mac:

        mov     eax, 47
        mov     ebx, 0x00020100
        mov     esi, 0x40000000
        mov     edi, 0x00bcbcbc

        mov     cl, [esp+4]
        mcall

        mov     cl, [esp+4+1]
        add     edx, 15 shl 16
        mcall

        mov     cl, [esp+4+2]
        add     edx, 15 shl 16
        mcall

        mov     cl, [esp+4+3]
        add     edx, 15 shl 16
        mcall

        mov     cl, [esp+4+4]
        add     edx, 15 shl 16
        mcall

        mov     cl, [esp+4+5]
        add     edx, 15 shl 16
        mcall

        sub     edx, 5*15 shl 16

        ret     6


draw_ip:

        mov     eax, 47
        mov     ebx, 0x00030000
        mov     esi, 0x40000000
        mov     edi, 0x00bcbcbc

        xor     ecx, ecx

        mov     cl, [esp+4]
        mcall

        mov     cl, [esp+4+1]
        add     edx, 30 shl 16
        mcall

        mov     cl, [esp+4+2]
        add     edx, 30 shl 16
        mcall

        mov     cl, [esp+4+3]
        add     edx, 30 shl 16
        mcall

        sub     edx, 3*30 shl 16
        ret     4


draw_interfaces:

        mov     [.btnpos], 8 shl 16 + 20
        mov     [.txtpos], 490 shl 16 + 15

        mcall   74, -1          ; get number of active network devices
        mov     ecx, eax

        xor     ebx, ebx        ; get device type
  .loop:
        mcall   74
        cmp     eax, 1          ; ethernet?
        je      .hit
        inc     bh
        jb      .loop           ; tried all 256?
        ret


  .hit:
        push    ecx ebx
        movzx   edx, bh
        shl     edx, 8
        mov     esi, 0x00aaaaff
        cmp     bh, [device]
        cmove   esi, 0x00aaffff
        mcall   8, 485 shl 16 + 100, [.btnpos]
        mov     ebx, [esp]
        inc     bl
        mov     ecx, namebuf
        mov     edx, namebuf
        mcall   74              ; get device name
        cmp     eax, -1
        jne     @f
        mov     edx, str_unknown
       @@:
        mcall   4, [.txtpos], 0x80000000   ; print the name
        pop     ebx ecx

        inc     bh

        add     [.btnpos], 25 shl 16
        add     [.txtpos], 25

        dec     ecx
        jnz     .loop

        ret

  .btnpos       dd ?
  .txtpos       dd ?




; DATA AREA

name            db 'Netstat', 0
mode            db 101
device          db 0
modes           db 'Ethernet        IPv4        ARP         ICMP         UDP         TCP', 0

str_packets_tx  db 'Packets sent:', 0
str_packets_rx  db 'Packets received:', 0
str_bytes_tx    db 'Bytes sent:', 0
str_bytes_rx    db 'Bytes received:', 0
str_MAC         db 'MAC address:', 0
str_ip          db 'IP address:', 0
str_dns         db 'DNS address:', 0
str_subnet      db 'Subnet mask:', 0
str_gateway     db 'Standard gateway:', 0
str_arp         db 'ARP entrys:', 0
str_conflicts   db 'ARP conflicts:', 0
str_unknown     db 'unknown', 0
str_missed      db 'Packets missed:',0
str_dumped      db 'Packets dumped:',0

namebuf         rb 64

I_PARAM rb 1024

I_END:


