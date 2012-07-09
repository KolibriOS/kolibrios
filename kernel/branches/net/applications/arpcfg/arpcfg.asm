;
; ARPmanager for KolibriOS
;
; hidnplayr@gmail.com
;

format binary as ""

use32

    org     0x0

    db     'MENUET01'           ; 8 byte id
    dd     0x01                 ; header version
    dd     START                ; start of code
    dd     IM_END               ; size of image
    dd     (I_END+0x100)        ; memory for app
    dd     (I_END+0x100)        ; esp
    dd     0x0 , 0x0            ; I_Param , I_Icon

include '../macros.inc'
purge   mov, add, sub
include '../struct.inc'
include '../network.inc'

START:

redraw:

        mcall   12, 1
        mcall   0, 100 shl 16 + 520, 100 shl 16 + 240, 0x34bcbcbc, , str_name
        mcall   4, 25 shl 16 + 31, 0x80000000, str_legend
        mcall   12, 2

draw_stats:

        mov     edx, 50 shl 16 + 50
        mov     [last], 0

  .loop:
        mcall   76, API_ARP + 3, [last],,, arp_buf
        cmp     eax, -1
        je      mainloop

        mcall   4, edx, 0x80000000, str_entry
        mov     edx, ebx

        mov     eax, 47
        mov     ebx, 0x00030000
        mov     esi, 0x40000000
        mov     edi, 0x00bcbcbc
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

        add     dx, 20
        rol     edx, 16
        mov     dx, 50
        rol     edx, 16
        inc     [last]

        jmp     .loop


mainloop:

        mcall   23,50                  ; wait for event with timeout    (0,5 s)

        cmp     eax, 1
        je      redraw
        cmp     eax, 2
        je      key
        cmp     eax, 3
        je      button

        jmp     draw_stats


key:
        mcall   2
        jmp     mainloop


button:                                 ; button
        mcall   17                      ; get id
        cmp     ah, 1
        je      exit
        jmp     redraw

exit:
        mcall   -1



; DATA AREA

str_name        db 'ARP manager', 0
str_legend      db '#   IP-address        MAC-address         Status    TTL', 0
str_entry       db '   .   .   .        -  -  -  -  -                     s', 0

IM_END:

last            dd ?
arp_buf         ARP_entry

I_END:


