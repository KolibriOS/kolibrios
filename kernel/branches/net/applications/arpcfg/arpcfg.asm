;
; ARPmanager for KolibriOS
;
; hidnplayr@gmail.com
;

use32

    org     0x0

    db	   'MENUET01'	     ; 8 byte id
    dd	   0x01 	     ; header version
    dd	   START	     ; start of code
    dd	   IM_END	     ; size of image
    dd	   (I_END+0x100)     ; memory for app
    dd	   (I_END+0x100)     ; esp
    dd	   I_PARAM , 0x0     ; I_Param , I_Icon

__DEBUG__ equ 1
__DEBUG_LEVEL__ equ 1

include '..\macros.inc'
include '..\debug-fdo.inc'

START:				       ; start of execution
    ; TODO: check Parameters

    DEBUGF 1, 'Hello!\n'

  redraw:

    mcall   12, 1

    mcall   0, 100 shl 16 + 520, 100 shl 16 + 240, 0x34bcbcbc, ,name

    mcall   4, 25 shl 16 + 31, 0x80000000, title

    mcall   12, 2

  draw_stats:

	mov	edx, 50 shl 16 + 50
	mov	[last],0

    .loop:
	mcall	76, 0x06080003, [last],,,ARP_ENTRY
	cmp	eax, -1
	je	mainloop


	mcall	4, edx, 0x80000000, str_entry
	mov	edx, ebx

	mov	eax, 47
	mov	ebx, 0x00030000
	mov	esi, 0x40000000
	mov	edi, 0x00bcbcbc
	xor	ecx, ecx

	mov	cl, byte[ARP_ENTRY.IP+0]
	mcall

	mov	cl, byte[ARP_ENTRY.IP+1]
	add	edx, 24 shl 16
	mcall

	mov	cl, byte[ARP_ENTRY.IP+2]
	add	edx, 24 shl 16
	mcall

	mov	cl, byte[ARP_ENTRY.IP+3]
	add	edx, 24 shl 16
	mcall


	mov	ebx, 0x00020100
	mov	cl, byte[ARP_ENTRY.MAC+0]
	add	edx, 36 shl 16
	mcall

	mov	cl, byte[ARP_ENTRY.MAC+1]
	add	edx, 18 shl 16
	mcall

	mov	cl, byte[ARP_ENTRY.MAC+2]
	add	edx, 18 shl 16
	mcall

	mov	cl, byte[ARP_ENTRY.MAC+3]
	add	edx, 18 shl 16
	mcall

	mov	cl, byte[ARP_ENTRY.MAC+4]
	add	edx, 18 shl 16
	mcall

	mov	cl, byte[ARP_ENTRY.MAC+5]
	add	edx, 18 shl 16
	mcall

	mov	ebx, 0x00040000
	mov	cx, [ARP_ENTRY.Status]
	add	edx, 30 shl 16
	mcall

	mov	cx, [ARP_ENTRY.TTL]
	add	edx, 60 shl 16
	mcall

	add	dx, 20
	rol	edx, 16
	mov	dx, 50
	rol	edx, 16
	inc	[last]

	jmp	.loop

  mainloop:

    mcall   23,50		   ; wait for event with timeout    (0,5 s)

    cmp     eax, 1
    je	    redraw
    cmp     eax, 2
    je	    key
    cmp     eax, 3
    je	    button

    jmp     draw_stats


  key:
    mcall   2
    jmp     mainloop


  button:			  ; button
    mcall   17			  ; get id
    cmp     ah, 1
    je	    close
    jmp     redraw

  close:
    mcall   -1



; DATA AREA

name	db 'ARP manager',0

title	db '#   IP-address        MAC-address         Status    TTL',0
str_entry   db '   .   .   .        -  -  -  -  -                     s',0

last	dd 0


ARP_ENTRY:
       .IP		dd  192 shl 0 + 168 shl 8 + 1 shl 16 + 1 shl 24
       .MAC		dp  0xdeadbeef1337
       .Status		dw  0x0300
       .TTL		dw  37
       .size:

include_debug_strings	 ; ALWAYS present in data section

IM_END:

I_PARAM rb 1024

I_END:


