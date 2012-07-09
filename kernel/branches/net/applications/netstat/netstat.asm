;
; Netstat for KolibriOS v0.2
;
; 0.1 - 22 sept 2009 - initial release
; 0.2 - 9 july 2012 - converted to new sysfunc numbers
;
; hidnplayr@gmail.com
;

format binary as ""

use32

    org     0x0

    db     'MENUET01'        ; 8 byte id
    dd     0x01              ; header version
    dd     START             ; start of code
    dd     I_END             ; size of image
    dd     (I_END+0x100)     ; memory for app
    dd     (I_END+0x100)     ; esp
    dd     I_PARAM , 0x0     ; I_Param , I_Icon

__DEBUG__ equ 1
__DEBUG_LEVEL__ equ 1

include '..\macros.inc'
include '..\debug-fdo.inc'
include '..\network.inc'

START:                                 ; start of execution
    ; TODO: check Parameters

    DEBUGF  1, 'Netstat application loaded!\n'

  redraw:
    mcall   12, 1

    mcall   0, 100 shl 16 + 520, 100 shl 16 + 240, 0x34bcbcbc , , name

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
    add     ebx, 18
    mov     edx, str_queue_in
    mcall
    add     ebx, 18
    mov     edx, str_queue_out
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

    jmp     end_of_draw

 .no_arp:

    mcall   4, 20 shl 16 + 75, 0x80000000, str_packets_tx
    add     ebx, 18
    mov     edx, str_packets_rx
    mcall

 end_of_draw:

    mcall   12, 2

    jmp     draw_stats

  mainloop:

    mcall   23,500                  ; wait for event with timeout    (0,5 s)

    cmp     eax, 1
    je      redraw
    cmp     eax, 2
    je      key
    cmp     eax, 3
    je      button




;-------------------------------
;
;------------------------------

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
    jle     @r

    inc     bl   ;5
    inc     bl   ;6

   @@:
    push    ebx
    mcall   76
    pop     ebx
    push    eax
    inc     bl
    cmp     bl, 7
    jle     @r

    mov     eax, 47
    mov     ebx, 0x000a0000
    mov     esi, 0x40000000
    mov     edi, 0x00bcbcbc
    mov     edx, 135 shl 16 + 75 + 6*18
    pop     ecx
    mcall
    sub     edx, 18
    pop     ecx
    mcall
    sub     edx, 2*18
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
    inc     bl

    mov     eax, 47
    mov     ebx, 0x000a0000
    mov     esi, 0x40000000
    mov     edi, 0x00bcbcbc
    mov     edx, 135 shl 16 + 75 + 18
    pop     ecx
    mcall
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
    inc     bl

    mov     eax, 47
    mov     ebx, 0x000a0000
    mov     esi, 0x40000000
    mov     edi, 0x00bcbcbc
    mov     edx, 135 shl 16 + 75 + 2*18
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
    inc     bl

    mov     eax, 47
    mov     ebx, 0x000a0000
    mov     esi, 0x40000000
    mov     edi, 0x00bcbcbc
    mov     edx, 135 shl 16 + 75 + 18
    pop     ecx
    mcall
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
    inc     bl

    mov     eax, 47
    mov     ebx, 0x000a0000
    mov     esi, 0x40000000
    mov     edi, 0x00bcbcbc
    mov     edx, 135 shl 16 + 75 + 18
    pop     ecx
    mcall
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

    mov     eax, 47
    mov     ebx, 0x000a0000
    mov     esi, 0x40000000
    mov     edi, 0x00bcbcbc
    mov     edx, 135 shl 16 + 75 + 18
    pop     ecx
    mcall
    sub     edx, 18
    pop     ecx
    mcall

    jmp     mainloop

not_106:

    jmp     mainloop

  key:
    mcall   2
    jmp     mainloop


  button:                         ; button
    mcall   17                    ; get id
    cmp     ah, 1
    je      exit
    mov     [mode], ah
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


; DATA AREA

name    db 'Netstat', 0
mode    db 101
device  db 0
modes   db 'Ethernet        IPv4        ARP         ICMP         UDP         TCP', 0

str_packets_tx db 'Packets sent:', 0
str_packets_rx db 'Packets received:', 0
str_bytes_tx   db 'Bytes sent:', 0
str_bytes_rx   db 'Bytes received:', 0
str_MAC        db 'MAC address:', 0
str_queue_in   db 'IN-queue size:', 0
str_queue_out  db 'OUT-queue size:', 0
str_ip         db 'IP address:', 0
str_dns        db 'DNS address:', 0
str_subnet     db 'Subnet mask:', 0
str_gateway    db 'Standard gateway:', 0
str_arp        db 'ARP entrys:', 0

include_debug_strings    ; ALWAYS present in data section

I_PARAM rb 1024

I_END:


