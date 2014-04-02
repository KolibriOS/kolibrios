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

        db     'MENUET01'       ; 8 byte id
        dd     0x01             ; header version
        dd     START            ; start of code
        dd     I_END            ; size of image
        dd     (I_END+0x1000)   ; memory for app
        dd     (I_END+0x1000)   ; esp
        dd     0, 0             ; I_Param , I_Path

include '../../macros.inc'
include '../../network.inc'

macro DrawRect x, y, w, h, color
{
	; DrawBar(x,y,w,1,color1);
	; DrawBar(x,y+h,w,1,color1);
	; DrawBar(x,y,1,h,color1);
	; DrawBar(x+w,y,1,h+1,color1);

	mcall 13, x shl 16 + w,     y shl 16 + 1,   color  ; top
	mcall   , x shl 16 + 1,     y shl 16 + h,   color  ; left
	mcall   , (x+w) shl 16 +1,  y shl 16 + (h+1), color  ; right
	mcall   , x shl 16 + w,   (y+h) shl 16 + 1, color  ; bottom
}


START:
        mcall   40, EVM_REDRAW + EVM_BUTTON + EVM_STACK2

window_redraw:
        mcall   12, 1
        mcall   0, 100 shl 16 + 600, 100 shl 16 + 240, 0x34E1E1E1, , name       ; draw window
		mcall   12, 2
		DrawRect 0, 25, 400, 180, 0x777777

redraw:
		mcall   13, 1 shl 16 + 399, 26 shl 16 + 179, 0x00F3F3F3
        call    draw_interfaces

        xor     ebx, ebx
        mov     bh, [device]
        mcall   74
        mov     [device_type], eax

        mov     edx, 101
        mov     esi, 0x00BBBbbb
        mov     edi, 0x0081BBFF

        cmp     dl, [mode]
        cmove   esi, edi
        mcall   8, 5 shl 16 + 55, 5 shl 16 + 20
  .morebuttons:
        inc     edx
        add     ebx, 60 shl 16
        mov     esi, 0x00BBBbbb

        cmp     dl, [mode]
        cmove   esi, edi
        mcall

        cmp     edx, 105
        jle     .morebuttons

        mcall   4, 9 shl 16 + 12, 0x80000000, modes

        cmp     [mode], 101
        jne     .no_eth

        mcall   4, 8 shl 16 + 35, 0x80000000, str_packets_tx
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
        mov     edx, str_link
        mcall

        cmp     [device_type], 1
        jne     end_of_draw

        add     ebx, 18
        mov     edx, str_MAC
        mcall

        mov     ebx, API_ETH
        mov     bh, [device]
        mcall   76
        push    eax
        push    bx

        mov     edx, 135 shl 16 + 35 + 5*18
        call    draw_mac
        jmp     end_of_draw

 .no_eth:

        cmp     [mode], 102
        jne     .no_ip

        mcall   4, 8 shl 16 + 35, 0x80000000, str_packets_tx
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

        mov     edx, 135 shl 16 + 35 + 2*18
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

        mcall   4, 8 shl 16 + 35, 0x80000000, str_packets_tx
        add     ebx, 18
        mov     edx, str_packets_rx
        mcall
        add     ebx, 18
        mov     edx, str_arp
        mcall
        add     ebx, 18
        mov     edx, str_conflicts
        mcall

        mcall   4, 8 shl 16 + 130, 0x80000000, str_ARP_legend

        jmp     end_of_draw

 .no_arp:

        mcall   4, 8 shl 16 + 35, 0x80000000, str_packets_tx

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

draw_stats:

        cmp     [mode], 101
        jne     not_101

        mov     ebx, API_ETH
        mov     bh, [device]
        mov     bl, 6
  @@:
        push    ebx
        mcall   74
        pop     ebx
        push    eax
        inc     bl
        cmp     bl, 10
        jbe     @r

        mov     ebx, 0x000a0000
        pop     ecx
        mov     edx, 135 shl 16 + 35 + 4*18
        mov     esi, 0x40000000
        mov     edi, 0x00F3F3F3
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
        mov     edx, 135 shl 16 + 35 + 18
        mov     esi, 0x40000000
        mov     edi, 0x00F3F3F3
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
        mcall   76      ; tx
        pop     ebx
        push    eax

        inc     bl
        push    ebx
        mcall   76      ; rx
        pop     ebx
        push    eax

        inc     bl
        push    ebx
        mcall   76      ; entries
        pop     ebx
        push    eax

        mov     bl, 7
        push    ebx
        mcall   76
        pop     ebx
        push    eax

        mov     ebx, 0x000a0000
        pop     ecx
        mov     edx, 135 shl 16 + 35 + 3*18
        mov     esi, 0x40000000
        mov     edi, 0x00F3F3F3
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

;        mov     edx, 50 shl 16 + 150
        mov     [last], 0

  .arp_loop:
        mov     ebx, API_ARP + 3                ; read ARP entry
        mov     bh, [device]
        mcall   76, ,[last], , , arp_buf
        cmp     eax, -1
        je      mainloop

        mov     ebx, [last]
        imul    ebx, 16
        add     ebx, 8 shl 16 + 140
        mcall   4, , 0x80000000, str_ARP_entry
        mov     edx, ebx

        mov     eax, 47
        mov     ebx, 0x00030000
        mov     esi, 0x40000000
        mov     edi, 0x00F3F3F3
        xor     ecx, ecx

        mov     cl, byte[arp_buf.IP+0]
        mcall

        mov     cl, byte[arp_buf.IP+1]
        add     edx, 24 shl 16
        mcall

        mov     cl, byte[arp_buf.IP+2]
        add     edx, 24 shl 16
        mcall

        mov     cl, byte[arp_buf.IP+3]
        add     edx, 24 shl 16
        mcall


        mov     ebx, 0x00020100
        mov     cl, byte[arp_buf.MAC+0]
        add     edx, 36 shl 16
        mcall

        mov     cl, byte[arp_buf.MAC+1]
        add     edx, 18 shl 16
        mcall

        mov     cl, byte[arp_buf.MAC+2]
        add     edx, 18 shl 16
        mcall

        mov     cl, byte[arp_buf.MAC+3]
        add     edx, 18 shl 16
        mcall

        mov     cl, byte[arp_buf.MAC+4]
        add     edx, 18 shl 16
        mcall

        mov     cl, byte[arp_buf.MAC+5]
        add     edx, 18 shl 16
        mcall

        mov     ebx, 0x00040000
        mov     cx, [arp_buf.status]
        add     edx, 30 shl 16
        mcall

        mov     cx, [arp_buf.TTL]
        add     edx, 60 shl 16
        mcall

        add     dx, 18
        rol     edx, 16
        mov     dx, 8
        rol     edx, 16
        inc     [last]

        jmp     .arp_loop

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
        mov     edx, 135 shl 16 + 35 + 18
        mov     esi, 0x40000000
        mov     edi, 0x00F3F3F3
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
        mov     edx, 135 shl 16 + 35 + 18
        mov     esi, 0x40000000
        mov     edi, 0x00F3F3F3
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
        mov     edx, 135 shl 16 + 35 + 18*3
        mov     esi, 0x40000000
        mov     edi, 0x00F3F3F3
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
        je      window_redraw
        cmp     eax, 3
        je      button
        cmp     eax, 11
        je      redraw

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
        mov     edi, 0x00F3F3F3

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
        mov     edi, 0x00F3F3F3

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

        mov     [.btnpos], 5 shl 16 + 20
        mov     [.txtpos], 455 shl 16 + 12

        mcall   74, -1          ; get number of active network devices
        mov     ecx, eax

        xor     ebx, ebx        ; get device type
  .loop:
        mcall   74
        cmp     eax, 1          ; loopback or ethernet?
        jbe     .hit
        inc     bh
        jb      .loop           ; tried all 256?
        ret


  .hit:
        push    ecx ebx
        movzx   edx, bh
        shl     edx, 8
        mov     esi, 0x00BBBbbb
        cmp     bh, [device]
        cmove   esi, 0x0081BBFF
        mcall   8, 450 shl 16 + 135, [.btnpos]
        mov     ebx, [esp]
        inc     bl
        mov     ecx, namebuf
        mov     edx, namebuf
        mcall   74                              ; get device name
        cmp     eax, -1
        jne     @f
        mov     edx, str_unknown
       @@:
        mcall   4, [.txtpos], 0x80000000        ; print the name
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
device_type     dd 0
last            dd 0
modes           db 'Physical    IPv4       ARP      ICMP      UDP       TCP', 0

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
str_link        db 'Link state:',0

str_ARP_legend  db 'IP-address        MAC-address         Status    TTL', 0
str_ARP_entry   db '   .   .   .        -  -  -  -  -                    s', 0

namebuf         rb 64
arp_buf         ARP_entry

I_END:


