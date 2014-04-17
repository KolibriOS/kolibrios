;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2010-2014. All rights reserved.    ;;
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

BOX_WIDTH       = 390
BOX_HEIGHT      = 185

use32

        org    0x0

        db     'MENUET01'       ; 8 byte id
        dd     0x01             ; header version
        dd     START            ; start of code
        dd     I_END            ; size of image
        dd     (I_END+0x1000)   ; memory for app
        dd     (I_END+0x1000)   ; esp
        dd     0, 0             ; I_Param, I_Path

include '../../macros.inc'
include '../../network.inc'

START:
        mcall   40, EVM_REDRAW + EVM_BUTTON + EVM_STACK2 + EVM_KEY

window_redraw:
; Notify kernel of start of window draw
        mcall   12, 1

; Draw the window
        mcall   0, 100 shl 16 + 600, 100 shl 16 + 240, 0x34E1E1E1, , name

; Define the buttons (for tabs)
        mov     ebx, 5 shl 16 + 54
        mov     ecx, 4 shl 16 + 21
        mov     edx, 0x60000000 + 101
  .buttonloop:
        mcall   8
        add     ebx, 60 shl 16
        inc     edx
        cmp     dl, 106
        jle     .buttonloop

; draw sides and upper lines of the tab buttons
        mov     eax, 13
        mov     ebx, 5 shl 16 + 1
        mov     ecx, 4 shl 16 + 21
        mov     edx, 0x00777777
  .loop:
        mcall
        mov     bx, 54
        mov     cx, 1
        mcall
        mov     bx, 1
        mov     cx, 21
        add     ebx, 54 shl 16
        mcall
        add     ebx, 6 shl 16
        cmp     ebx, 360 shl 16
        jb      .loop

; Draw sides and bottom lines of the rectangle
        mcall  , 0 shl 16 + 1, 25 shl 16 + BOX_HEIGHT;, 0x00777777
        mcall  , (0+BOX_WIDTH) shl 16 +1, 25 shl 16 + (BOX_HEIGHT+1)
        mcall  , 0 shl 16 + BOX_WIDTH, (25+BOX_HEIGHT) shl 16 + 1

redraw:

; Draw interface buttons (on the right hand side)
        call    draw_interfaces

; Draw upper line of rectangle
        mcall   13, 0 shl 16 + BOX_WIDTH, 25 shl 16 + 1, 0x00777777

; Fill rectangle
        mcall   13, 1 shl 16 + BOX_WIDTH-1, 26 shl 16 + BOX_HEIGHT-1, 0x00F3F3F3

; Fill tab buttons
        mov     eax, 13
        mov     ebx, 6 shl 16 + 53
        mov     si, 101
  .buttonloop:
        mov     ecx, 6 shl 16 + 19
        mov     edx, 0x00BBBBBB
        cmp     si, [mode]
        jne     @f
        mov     edx, 0x00F3F3F3         ; Activated button has other colors
        inc     ecx
  @@:
        mcall
        mov     edx, 0x00E1E1E1
        cmp     si, [mode]
        jne     @f
        mov     edx, 0x00FFFFFF         ; Activated button has other colors
  @@:
        mov     ecx, 5 shl 16 + 1
        mcall
        add     ebx, 60 shl 16
        inc     si
        cmp     si, 106
        jle     .buttonloop
; Print button names on top of the buttons
        mcall   4, 9 shl 16 + 12, 0x80000000, modes

; Get information about the selected device
        xor     ebx, ebx
        mov     bh, [device]
        mcall   74
        mov     [device_type], eax

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

        mov     edx, 134 shl 16 + 35 + 5*18
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

        mov     edx, 134 shl 16 + 35 + 2*18
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

        mcall   12, 2

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

        pop     eax
        test    al, al
        jnz     @f
        mov     edx, str_down
        jmp     .print_link
  @@:
        cmp     al, 100b
        jnz     @f
        mov     edx, str_10m
        jmp     .print_link
  @@:
        cmp     al, 110b
        jnz     @f
        mov     edx, str_10mfd
        jmp     .print_link
  @@:
        cmp     al, 1000b
        jnz     @f
        mov     edx, str_100m
        jmp     .print_link
  @@:
        cmp     al, 1010b
        jnz     @f
        mov     edx, str_100mfd
        jmp     .print_link
  @@:
        cmp     al, 10000b
        jnz     @f
        mov     edx, str_1g
        jmp     .print_link
  @@:
        cmp     al, 10010b
        jnz     @f
        mov     edx, str_1gfd
        jmp     .print_link
  @@:
        mov     edx, str_unknown

  .print_link:
        mov     ebx, 134 shl 16 + 35 + 4*18
        mov     ecx, 0xc0000000
        mov     edi, 0x00f3f3f3
        mcall   4

        mov     ebx, 0x000a0000
        pop     ecx
        mov     edx, 134 shl 16 + 35 + 3*18
        mov     esi, 0x40000000
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
        mov     edx, 134 shl 16 + 35 + 18
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
        mov     edx, 134 shl 16 + 35 + 3*18
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
        mov     edx, 134 shl 16 + 35 + 18
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
        mov     edx, 134 shl 16 + 35 + 18
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
        mov     edx, 134 shl 16 + 35 + 18*3
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

        cmp     eax, EV_REDRAW
        je      window_redraw
        cmp     eax, EV_BUTTON
        je      button
        cmp     eax, EV_KEY
        je      key
        cmp     eax, 11
        je      redraw

        jmp     draw_stats

button:                         ; button
        mcall   17              ; get id
        cmp     ah, 1
        je      exit
        cmp     ah, 0
        je      .interface
        shr     ax, 8
        mov     [mode], ax
        jmp     redraw

  .interface:
        shr     eax, 16
        mov     [device], al
        jmp     redraw

key:
        mcall   2
        cmp     ah, 9
        je      .tab
        cmp     ah, 183
        je      .pgdown
        cmp     ah, 184
        je      .pgup
        jmp     mainloop

  .tab:
        inc     [mode]
        cmp     [mode], 106
        jbe     redraw
        mov     [mode], 101
        jmp     redraw

  .pgdown:
        inc     [device]
        mov     al, [device]
        cmp     al, [last_device]
        jbe     redraw
        mov     [device], 0
        jmp     redraw

  .pgup:
        dec     [device]
        cmp     [device], 0
        jge     redraw
        mov     al, [last_device]
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
        add     edx, 18 shl 16
        mcall

        mov     cl, [esp+4+2]
        add     edx, 18 shl 16
        mcall

        mov     cl, [esp+4+3]
        add     edx, 18 shl 16
        mcall

        mov     cl, [esp+4+4]
        add     edx, 18 shl 16
        mcall

        mov     cl, [esp+4+5]
        add     edx, 18 shl 16
        mcall

        sub     edx, 5*18 shl 16

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
        add     edx, 24 shl 16
        mcall

        mov     cl, [esp+4+2]
        add     edx, 24 shl 16
        mcall

        mov     cl, [esp+4+3]
        add     edx, 24 shl 16
        mcall

        sub     edx, 3*24 shl 16
        ret     4


draw_interfaces:

        mov     [.btnpos], 5 shl 16 + 20
        mov     [.txtpos], 405 shl 16 + 12

        mcall   74, -1          ; get number of active network devices
        mov     ecx, eax
        dec     al
        mov     [last_device], al

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
        mcall   8, 400 shl 16 + 185, [.btnpos]
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

name            db 'Network status', 0
mode            dw 101
device          db 0
last_device     db 0
device_type     dd 0
last            dd 0
modes           db 'Physical    IPv4      ARP       ICMP      UDP       TCP', 0

str_packets_tx  db 'Packets sent:', 0
str_packets_rx  db 'Packets received:', 0
str_bytes_tx    db 'Bytes sent:', 0
str_bytes_rx    db 'Bytes received:', 0
str_MAC         db 'MAC address:           -  -  -  -  -', 0
str_ip          db 'IP address:             .   .   .', 0
str_dns         db 'DNS address:            .   .   .', 0
str_subnet      db 'Subnet mask:            .   .   .', 0
str_gateway     db 'Standard gateway:       .   .   .', 0
str_arp         db 'ARP entrys:', 0
str_conflicts   db 'ARP conflicts:', 0
str_missed      db 'Packets missed:',0
str_dumped      db 'Packets dumped:',0
str_link        db 'Link state:',0

str_down        db 'down', 0
str_unknown     db 'unknown', 0
str_10m         db '10 Mbit Half duplex', 0
str_10mfd       db '10 Mbit Full duplex', 0
str_100m        db '100 Mbit Half duplex', 0
str_100mfd      db '100 Mbit Full duplex', 0
str_1g          db '1 Gbit Half duplex', 0
str_1gfd        db '1 Gbit Full duplex', 0

str_ARP_legend  db 'IP-address        MAC-address         Status    TTL', 0
str_ARP_entry   db '   .   .   .        -  -  -  -  -', 0

namebuf         rb 64
arp_buf         ARP_entry

I_END: