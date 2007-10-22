;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                          ;
;   Stack Configuration Tool               ;
;                                          ;
;   Compile with FASM for Menuet           ;
;                                          ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


memsize = 100000h
	       org 0
PARAMS	= memsize - 1024

use32

	       db     'MENUET01'	      ; 8 byte id
	       dd     0x01		      ; header version
	       dd     START		      ; start of code
	       dd     I_END		      ; size of image
	       dd     memsize		      ; memory for app
	       dd     memsize - 1024	      ; esp
	       dd     PARAMS , 0x0	      ; I_Param , I_Icon

include 'lang.inc'
include 'macros.inc'

START:				; start of execution

    cmp     [PARAMS], dword 'BOOT'
    jz	    boot_set_settings

no_params:

  red:
    call draw_window		; at first, draw the window

still:

    mov  eax,10 		; wait here for event
    mcall

    cmp  eax,1			; redraw request ?
    jz	 red
    cmp  eax,2			; key in buffer ?
    jnz  button

  key:				; key
;    mov  al,2                  ; just read it and ignore
    mcall
    jmp  still

  button:			; button
    mov  al,17		       ; get id
    mcall

    shr  eax,8

    dec  eax			 ; button id=1 ?
    jne  noclose
    or	 eax,-1 		; close this program
    mcall
  noclose:

    dec  eax
    je	 read_stack_setup

    dec  eax
    jne  no_apply_stack_setup
    call apply_stack_setup
    jmp  still

no_apply_stack_setup:
    dec  eax		      ; GET COM PORT
    dec  eax
    jne  no_read_comport
    mov  [string_x],272
    mov  [string_y],40
    mov  [string_length],3
    call read_string
    movzx eax,byte [string]
    cmp  eax,'A'
    jb	 gcp1
    sub  eax,'A'-'9'-1
   gcp1:
    sub  eax,48
    shl  eax,8
    mov  ebx,eax
    movzx eax,byte [string+1]
    cmp  eax,'A'
    jb	 gcp2
    sub  eax,'A'-'9'-1
   gcp2:
    sub  eax,48
    shl  eax,4
    add  ebx,eax
    movzx eax,byte [string+2]
    cmp  eax,'A'
    jb	 gcp3
    sub  eax,'A'-'9'-1
   gcp3:
    sub  eax,48
    add  ebx,eax
    mov  [com_add],ebx
    jmp  red
   no_read_comport:

    dec  eax		      ; GET COM IRQ
    jne  no_read_comirq
    mov  [string_x],284
    mov  [string_y],50
    mov  [string_length],1
    call read_string
    movzx eax,byte [string]
    cmp  eax,'A'
    jb	 gci1
    sub  eax,'A'-'9'-1
   gci1:
    sub  eax,48
    mov  [com_irq],eax
    jmp  red
    no_read_comirq:

    dec  eax		      ; GET IP
    jne  no_read_ip
    mov  [string_x],205
    mov  [string_y],80
    mov  [string_length],15
    call read_string
    mov   esi,string-1
    mov   edi,ip_address
   ip0:
    xor   eax,eax
   ip1:
    inc   esi
    cmp   [esi],byte '0'
    jb	  ip2
    cmp   [esi],byte '9'
    jg	  ip2
    imul  eax,10
    movzx ebx,byte [esi]
    sub   ebx,48
    add   eax,ebx
    jmp   ip1
   ip2:
    stosb
    cmp   edi,ip_address+3
    jbe   ip0
    jmp   red
   no_read_ip:

    dec     eax 		; set gateway ip
    jne     no_set_gateway

    mov  [string_x],205
    mov  [string_y],90
    mov  [string_length],15
    call read_string
    mov   esi,string-1
    mov   edi,gateway_ip
   gip0:
    xor   eax,eax
   gip1:
    inc   esi
    cmp   [esi],byte '0'
    jb	  gip2
    cmp   [esi],byte '9'
    jg	  gip2
    imul  eax,10
    movzx ebx,byte [esi]
    sub   ebx,48
    add   eax,ebx
    jmp   gip1
   gip2:
    stosb
    cmp   edi,gateway_ip+3
    jbe   gip0
    jmp   red

  no_set_gateway:

    dec     eax
    jne     no_set_subnet

    mov  [string_x],205
    mov  [string_y],100
    mov  [string_length],15
    call read_string
    mov   esi,string-1
    mov   edi,subnet_mask
   sip0:
    xor   eax,eax
   sip1:
    inc   esi
    cmp   [esi],byte '0'
    jb	  sip2
    cmp   [esi],byte '9'
    jg	  sip2
    imul  eax,10
    movzx ebx,byte [esi]
    sub   ebx,48
    add   eax,ebx
    jmp   sip1
   sip2:
    stosb
    cmp   edi,subnet_mask+3
    jbe   sip0
    jmp   red

  no_set_subnet:
    dec     eax
    jne     no_set_dns

    mov  [string_x],205
    mov  [string_y],110
    mov  [string_length],15
    call read_string
    mov   esi,string-1
    mov   edi,dns_ip
   dip0:
    xor   eax,eax
   dip1:
    inc   esi
    cmp   [esi],byte '0'
    jb	  dip2
    cmp   [esi],byte '9'
    jg	  dip2
    imul  eax,10
    movzx ebx,byte [esi]
    sub   ebx,48
    add   eax,ebx
    jmp   dip1
   dip2:
    stosb
    cmp   edi,dns_ip+3
    jbe   dip0
    jmp   red

  no_set_dns:

    dec  eax
    jb	 no_set_interface
    cmp  eax,14-11
    ja	 no_set_interface
    mov  [interface],eax
    jmp  red
   no_set_interface:

    sub  eax,21-11
    jb	 no_ip_sf
    cmp  eax,22-21
    ja	 no_ip_sf
    xor  eax,1
    mov  [assigned],eax
    jmp  red
    no_ip_sf:
    jmp  still

read_stack_setup:

    mov  eax,52
    mov  ebx,0
    mcall
    mov  [config],eax

    mov  eax,52
    mov  ebx,1
    mcall
    mov  dword [ip_address],eax

    mov  eax,52
    mov  ebx,9
    mcall
    mov  dword [gateway_ip],eax

    mov  eax,52
    mov  ebx,10
    mcall
    mov  dword [subnet_mask],eax

    mov  eax,52
    mov  ebx,13
    mcall
    mov  dword [dns_ip],eax

    mov  eax,[config]	; unwrap com IRQ
    shr  eax,8
    and  eax,0xf
    mov  [com_irq],eax

    mov  eax,[config]	; unwrap com PORT
    shr  eax,16
    and  eax,0xfff
    mov  [com_add],eax

    mov  eax,[config]	; unwrap IRQ
    and  eax,0xf
    mov  [interface],eax

    mov  eax,[config]	; unwrap com PORT
    shr  eax,7
    and  eax,1
    mov  [assigned],eax
    jmp  red

apply_stack_setup:

    mov  eax,[com_irq]
    shl  eax,8
    mov  ebx,[com_add]
    shl  ebx,16
    add  eax,ebx
    add  eax,[interface]
    mov  ebx,[assigned]
    shl  ebx,7
    add  eax,ebx
    mov  [config],eax

    mov  eax,52
    mov  ebx,3
    mov  ecx,dword [ip_address]
    mcall

    mov  eax,52
    mov  ebx,11
    mov  ecx,dword [gateway_ip]
    mcall

    mov  eax,52
    mov  ebx,12
    mov  ecx,dword [subnet_mask]
    mcall

    mov  eax,52
    mov  ebx,14
    mov  ecx,dword [dns_ip]
    mcall

    mov  eax,52
    mov  ebx,2
    mov  ecx,[config]
    mcall

    ret


string_length  dd    16
string_x       dd    200
string_y       dd    60

string	       db    '________________'


read_string:

    mov  edi,string
    mov  eax,'_'
    mov  ecx,[string_length]
    cld
    rep  stosb
    call print_text

    mov  edi,string
  f11:
    mov  eax,10
    mcall
    cmp  eax,2
    jne  read_done
;    mov  eax,2
    mcall
    shr  eax,8
    cmp  eax,13
    je	 read_done
    cmp  eax,8
    jnz  nobsl
    cmp  edi,string
    jz	 f11
    sub  edi,1
    mov  [edi],byte '_'
    call print_text
    jmp  f11
  nobsl:
    cmp  eax,dword 31
    jbe  f11
    cmp  eax,dword 95
    jb	 keyok
    sub  eax,32
  keyok:
    mov  [edi],al
    call print_text

    inc  edi
    mov  esi,string
    add  esi,[string_length]
    cmp  esi,edi
    jnz  f11

  read_done:

print_text:

    pusha

    mov  eax,13
    mov  ebx,[string_x]
    shl  ebx,16
    add  ebx,[string_length]
    imul bx,6
    mov  ecx,[string_y]
    shl  ecx,16
    mov  cx,8
    mov  edx,0xffffff
    mcall

    mov  eax,4
    mov  ebx,[string_x]
    shl  ebx,16
    add  ebx,[string_y]
    mov  ecx,0x000000
    mov  edx,string
    mov  esi,[string_length]
    mcall

    popa
    ret







;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,1			   ; 1, start of draw
    mcall

				   ; DRAW WINDOW
    mov  eax,0			   ; function 0 : define and draw window
    mov  ebx,100*65536+330	   ; [x start] *65536 + [x size]
    mov  ecx,100*65536+157	   ; [y start] *65536 + [y size]
    mov  edx,0x14ffffff 	   ; color of work area RRGGBB,8->color gl
    mov  edi,title		  ; WINDOW LABEL
    mcall


    mov  eax,8			   ; BUTTON : READ SETUP
    mov  ebx,90*65536+65
    mov  ecx,127*65536+12
    mov  edx,2
    mov  esi,[button_color]
    mcall

    ;mov  eax,8                     ; BUTTON : APPLY SETUP
    mov  ebx,163*65536+65
    mov  ecx,127*65536+12
    mov  edx,3
    mcall

    ;mov  eax,8                     ; BUTTONS 11-14 : SELECT INTERFACE
    mov  ebx,29*65536+8
    mov  ecx,39*65536+8
    mov  edx,11
  interface_select:
    mcall
    add  ecx,10*65536
    inc  edx
    cmp  edx,11+4
    jb	 interface_select

    mov  ebx,[interface]	   ; PRINT SELECTED INTERFACE 'X'
    imul ebx,10
    add  ebx,31*65536+39
    mov  eax,4
    mov  ecx,0xffffff
    mov  edx,xx
    mov  esi,1
    mcall

    mov  eax,8			  ; BUTTONS 21-22 : SERVER / MANUAL IP
    mov  ebx,143*65536+8
    mov  ecx,69*65536+8
    mov  edx,21
    mov  esi,[button_color]
    mcall
    ;mov  eax,8
    mov  ebx,143*65536+8
    mov  ecx,79*65536+8
    mov  edx,22
    mcall
    mov  ebx,[assigned] 	  ; PRINT SELECTED SERVER/MANUAL 'X'
    not  ebx
    and  ebx,1
    imul ebx,10
    add  ebx,145*65536+69
    mov  eax,4
    mov  ecx,0xffffff
    mov  edx,xx
    mov  esi,1
    mcall

    mov  eax,47 		  ; COM ADDRESS
    mov  ebx,3*65536+1*256
    mov  ecx,[com_add]
    mov  edx,272*65536+40
    mov  esi,0x000000
    mcall

    ;mov  eax,47                   ; COM IRQ
    mov  ebx,1*65536+1*256
    mov  ecx,[com_irq]
    mov  edx,(266+3*6)*65536+50
    mov  esi,0x000000
    mcall

    mov  edi,ip_address
    mov  edx,205*65536+80
    mov  esi,0x000000
    mov  ebx,3*65536
  ipdisplay:
    ;mov  eax,47
    movzx ecx,byte [edi]
    mcall
    add  edx,6*4*65536
    inc  edi
    cmp  edi,ip_address+4
    jb	 ipdisplay

    mov  edi,gateway_ip
    mov  edx,205*65536+90
    mov  esi,0x000000
    mov  ebx,3*65536
  gipdisplay:
    ;mov  eax,47
    movzx ecx,byte [edi]
    mcall
    add  edx,6*4*65536
    inc  edi
    cmp  edi,gateway_ip+4
    jb	 gipdisplay

    mov  edi,subnet_mask
    mov  edx,205*65536+100
    mov  esi,0x000000
    mov  ebx,3*65536
  sipdisplay:
    ;mov  eax,47
    movzx ecx,byte [edi]
    mcall
    add  edx,6*4*65536
    inc  edi
    cmp  edi,subnet_mask+4
    jb	 sipdisplay

    mov  edi,dns_ip
    mov  edx,205*65536+110
    mov  esi,0x000000
    mov  ebx,3*65536
  dipdisplay:
    ;mov  eax,47
    movzx ecx,byte [edi]
    mcall
    add  edx,6*4*65536
    inc  edi
    cmp  edi,dns_ip+4
    jb	 dipdisplay


    mov  eax,8			   ; BUTTON 5 : SET PORT
    mov  ebx,299*65536+8
    mov  ecx,39*65536+8
    mov  edx,5
    mov  esi,[button_color]
    mcall
    ;mov  eax,8                     ; BUTTON 6 : SET IRQ
    mov  ecx,49*65536+8
    inc  edx
    mcall
    ;mov  eax,8                     ; BUTTON 7 : SET IP
    mov  ecx,79*65536+8
    inc  edx
    mcall

    ;mov  eax,8                     ; BUTTON 8 : SET gateway IP
    mov  ebx,299*65536+8
    mov  ecx,89*65536+8
    inc  edx
    mcall

    ;mov  eax,8                     ; BUTTON 9 : SET subnet
    mov  ecx,99*65536+8
    inc  edx
    mcall

    ;mov  eax,8                     ; BUTTON 10 : SET dns ip
    mov  ecx,109*65536+8
    inc  edx
    mcall

    mov  ebx,31*65536+40	   ; draw info text with function 4
    mov  edx,text
    mov  esi,49
    mov  eax,4
  newline:
    mov  ecx,0x224466
    cmp  [edx],byte 'w'
    jne  nowhite
    mov  ecx,0xeeeeee
   nowhite:
    inc  edx
    mcall
    add  ebx,10
    add  edx,49
    cmp  [edx],byte 'x'
    jne  newline

    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,2			   ; 2, end of draw
    mcall

    ret

;******************************************************************************

boot_set_settings:

    mov  eax,52
    mov  ebx,0
    mcall
    mov  [config],eax

    shr  eax,8		; unwrap com IRQ
    and  eax,0xf
    mov  [com_irq],eax

    mov  eax,[config]	; unwrap com PORT
    shr  eax,16
    and  eax,0xfff
    mov  [com_add],eax

    call apply_stack_setup

    mov  eax,-1 		; close this program
    mcall

;******************************************************************************


; DATA AREA

if lang eq ru
title	   db  'Настройка сетевого стека',0
text:
    db '   Неактивный      Модем на Com-порту: 0x     <   '
    db '   Slip            Прерывание модема:    0x   <   '
    db '   PPP                                            '
    db '   Драйвер пакетов    IP server назначенный       '
    db '                      Фикс.      .   .   .    <   '
    db '                      Шлюз:      .   .   .    <   '
    db '                      Подсеть:   .   .   .    <   '
    db '                      DNS IP:    .   .   .    <   '
    db '                                                  '
    db 'w            Читать     Применить                 '

else
title	   db  'Stack configuration',0
text:
    db '   Not active       Modem Com Port:    0x     <   '
    db '   Slip             Modem Com Irq:       0x   <   '
    db '   PPP                                            '
    db '   Packet Driver      IP server assigned          '
    db '                      Fixed:     .   .   .    <   '
    db '                      Gateway:   .   .   .    <   '
    db '                      Subnet:    .   .   .    <   '
    db '                      DNS IP:    .   .   .    <   '
    db '                                                  '
    db 'w             READ        APPLY                   '
    end if

xx: db 'x' ;<- END MARKER, DONT DELETE

button_color dd  0x2254b9


;ENTER YOUR SETTINGS HERE:

ip_address  db	010,005,004,175
gateway_ip  db	010,005,000,001
subnet_mask db	255,255,000,000
dns_ip	    db	213,184,238,006


com_irq     dd	    0	; irq for slip/ppp
com_add     dd	    0	; com port address for slip/ppp
interface   dd	    3	; not active,slip,ppp,packet driver
assigned    dd	    0	; get ip from server

config	    dd	    0

I_END:
