;
;    ARP Status Monitor
;
;    Compile with FASM for Menuet
;
;  This program displays the ARP table, and it's settings
   
use32
   
                org     0x0
   
                db      'MENUET00'              ; 8 byte id
                dd      38                      ; required os
                dd      START                   ; program start
                dd      I_END                   ; program image size
                dd      0x100000                ; required amount of memory
                dd      0x00000000              ; reserved=no extended header
   include 'lang.inc'
   include '..\..\..\macros.inc'
   purge mov	; decrease kpack'ed size
   
START:                          ; start of execution
    call draw_window            ; at first, draw the window
   
still:
    mov  eax,23                 ; wait here for event
    mov  ebx,200				; Time out after 2s
    mcall
   
    cmp  eax,1                  ; redraw request ?
    jz   red
    cmp  eax,2                  ; key in buffer ?
    jz   key
    cmp  eax,3                  ; button in buffer ?
    jz   button
   
	; read the stack status data, and write it to the screen buffer

	mov		eax, 53
	mov		ebx, 255
	mov		ecx, 200
	mcall

	push    eax		
	mov		ebx, text + 24
	call	printhex

	mov		eax, 53
	mov		ebx, 255
	mov		ecx, 201
	mcall
		
	mov		ebx, text + 64
	call	printhex
	
	
	; Fill the table with blanks
	mov		edi, text + 160
doBlank:
	mov		esi, blank
	mov		ecx, 40
	rep		movsb

	cmp		edi, text + 560
	jne		doBlank
	
	pop		ecx					; The number of entries
	
	mov		ebx, text+ 160	+1	; the position for the first IP address line
	
	xor		edx, edx			; edx is index into the ARP table

	cmp		ecx, 10
	jle		show_entries
	mov		ecx, 10
	
	
; The following code is not very efficient; Sorry about that.
; ARPSTAT is a debugging tool, so I didn't want to put much effort in
show_entries:
	; Ecx now holds the number of entries to populate.
	; Ebx holds the place to put the data
	; edx is a counter
	cmp		ecx, 0
	je		red
	
	push	ecx
	push	edx
	push	ebx
	

	; select the arp table entry (in edx)
	mov		eax, 53
	mov		ebx, 255
	mov		ecx, 202
	mcall
	
	; Read the IP address
	mov		eax, 53
	mov		ebx, 255
	mov		ecx, 203
	mcall
	
	; IP in eax. Get the address to put it back 
	pop		ebx
	push		ebx
	
	call		writeDecimal			; Extract 1 byte from eax, store it in string	
	add		ebx, 4
	shr		eax, 8
	call		writeDecimal			; Extract 1 byte from eax, store it in string	
	add		ebx, 4
	shr		eax, 8
	call		writeDecimal			; Extract 1 byte from eax, store it in string	
	add		ebx, 4
	shr		eax, 8
	call		writeDecimal			; Extract 1 byte from eax, store it in string	

	add		ebx, 4
	
	; Now display the 6 byte MAC
	push		ebx
	mov		eax, 53
	mov		ebx, 255
	mov		ecx, 204
	mcall
	pop		ebx
		
	mov		ecx, eax

	shr		eax, 4
	and		eax, 0x0f
	mov		al, [eax + hextable]
	mov		[ebx], al
	inc		ebx
	mov		eax, ecx
	and		eax, 0x0f
	mov		al, [eax + hextable]
	mov		[ebx], al
	inc		ebx
	mov		eax, ecx
	shr		eax, 12
	and		eax, 0x0f
	mov		al, [eax + hextable]
	mov		[ebx], al
	inc		ebx
	mov		eax, ecx
	shr		eax, 8
	and		eax, 0x0f
	mov		al, [eax + hextable]
	mov		[ebx], al
	inc		ebx
	mov		eax, ecx
	shr		eax, 20
	and		eax, 0x0f
	mov		al, [eax + hextable]
	mov		[ebx], al
	inc		ebx
	mov		eax, ecx
	shr		eax, 16
	and		eax, 0x0f
	mov		al, [eax + hextable]
	mov		[ebx], al
	inc		ebx
	mov		eax, ecx
	shr		eax, 28
	mov		al, [eax + hextable]
	mov		[ebx], al
	inc		ebx
	mov		eax, ecx
	shr		eax, 24
	and		eax, 0x0f
	mov		al, [eax + hextable]
	mov		[ebx], al
	inc		ebx

	push		ebx
	mov		eax, 53
	mov		ebx, 255
	mov		ecx, 205
	mcall
	pop		ebx
	
	mov		ecx, eax

	shr		eax, 4
	and		eax, 0x0f
	mov		al, [eax + hextable]
	mov		[ebx], al
	inc		ebx
	mov		eax, ecx
	and		eax, 0x0f
	mov		al, [eax + hextable]
	mov		[ebx], al
	inc		ebx
	mov		eax, ecx
	shr		eax, 12
	and		eax, 0x0f
	mov		al, [eax + hextable]
	mov		[ebx], al
	inc		ebx
	mov		eax, ecx
	shr		eax, 8
	and		eax, 0x0f
	mov		al, [eax + hextable]
	mov		[ebx], al

	; Now display the stat field
	inc		ebx
	inc		ebx
	push		ebx
	mov		eax, 53
	mov		ebx, 255
	mov		ecx, 206
	mcall
	pop		ebx
	
	mov		ecx, eax

	shr		eax, 4
	and		eax, 0x0f
	mov		al, [eax + hextable]
	mov		[ebx], al
	inc		ebx
	mov		eax, ecx
	and		eax, 0x0f
	mov		al, [eax + hextable]
	mov		[ebx], al
	inc		ebx
	mov		eax, ecx
	shr		eax, 12
	and		eax, 0x0f
	mov		al, [eax + hextable]
	mov		[ebx], al
	inc		ebx
	mov		eax, ecx
	shr		eax, 8
	and		eax, 0x0f
	mov		al, [eax + hextable]
	mov		[ebx], al
	
	; Now display the TTL field (this is intel word format)
	inc		ebx
	inc		ebx
	push		ebx
	mov		eax, 53
	mov		ebx, 255
	mov		ecx, 207
	mcall
	pop		ebx
	
	mov		ecx, eax

	shr		eax, 12
	and		eax, 0x0f
	mov		al, [eax + hextable]
	mov		[ebx], al
	inc		ebx
	mov		eax, ecx
	shr		eax, 8
	and		eax, 0x0f
	mov		al, [eax + hextable]
	mov		[ebx], al
	inc		ebx
	mov		eax, ecx
	shr		eax, 4
	and		eax, 0x0f
	mov		al, [eax + hextable]
	mov		[ebx], al
	inc		ebx
	mov		eax, ecx
	and		eax, 0x0f
	mov		al, [eax + hextable]
	mov		[ebx], al

	
	pop		ebx
	add		ebx, 40
	pop		edx
	inc		edx
	pop		ecx
	dec		ecx
	jmp		show_entries

red:                          	; redraw
    call draw_window
    jmp  still
   
key:                          	; Keys are not valid at this part of the
    mov  eax,2                  ; loop. Just read it and ignore
    mcall
    jmp  still
   
button:                       	; button
    mov  eax,17                 ; get id
    mcall
   
    cmp  ah,1                   ; button id=1 ?
    jnz  still

    mov  eax,0xffffffff         ; close this program
    mcall


writeDecimal:
	pusha
	and	eax, 0xff
	mov	dl, 100
	div	dl
	movzx	ecx, ah
	add	al, '0'
	mov	[ebx], al
	inc	ebx
	mov	eax, ecx
	mov	dl, 10
	div	dl
	add	ax, '00'
	mov	[ebx], ax
	popa
	ret
   
   
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
   
   
draw_window:
   
    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    mcall
   
                                   ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,100*65536+280         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+270         ; [y start] *65536 + [y size]
    mov  edx,0x14224466            ; color of work area RRGGBB
    mov  edi,title                 ; WINDOW LABEL
    mcall
   
                                   
   	; Re-draw the screen text
    cld
    mov  eax,4
    mov  ebx,25*65536+35           ; draw info text with function 4
    mov  ecx,0xffffff
    mov  edx,text
    mov  esi,40
  newline:
    mcall
    add  ebx,16
    add  edx,esi
    cmp  [edx],byte 'x'
    jnz  newline
   
   
    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    mcall
   
    ret
 
; Taken from PS.ASM  
printhex:
; number in eax
; print to ebx
; xlat from hextable
 pusha
 mov esi, ebx
 add esi, 8
 mov ebx, hextable
 mov ecx, 8
phex_loop:
 mov edx, eax
 and eax, 15
 xlatb
 mov [esi], al
 mov eax, edx
 shr eax, 4
 dec esi
 loop phex_loop
 popa
 ret
 
   
; DATA AREA
   
text:
    db ' Number of ARP entries: xxxxxxxx        '
    db ' Maximum # of entries : xxxxxxxx        '
    db '                                        '
    db ' IP Address      MAC          Stat TTL  '
    db ' xxx.xxx.xxx.xxx xxxxxxxxxxxx xxxx xxxx '
    db ' xxx.xxx.xxx.xxx xxxxxxxxxxxx xxxx xxxx '
    db ' xxx.xxx.xxx.xxx xxxxxxxxxxxx xxxx xxxx '
    db ' xxx.xxx.xxx.xxx xxxxxxxxxxxx xxxx xxxx '
    db ' xxx.xxx.xxx.xxx xxxxxxxxxxxx xxxx xxxx '
    db ' xxx.xxx.xxx.xxx xxxxxxxxxxxx xxxx xxxx '
    db ' xxx.xxx.xxx.xxx xxxxxxxxxxxx xxxx xxxx '
    db ' xxx.xxx.xxx.xxx xxxxxxxxxxxx xxxx xxxx '
    db ' xxx.xxx.xxx.xxx xxxxxxxxxxxx xxxx xxxx '
    db ' xxx.xxx.xxx.xxx xxxxxxxxxxxx xxxx xxxx '
    db 'x' ; <- END MARKER, DONT DELETE

 
blank:
 	  db ' xxx.xxx.xxx.xxx xxxxxxxxxxxx xxxx xxxx '
 

title    db   'ARP Table ( First 10 Entries )',0
   
hextable db '0123456789ABCDEF'


I_END:
