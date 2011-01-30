;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   PSX-Pad for KolibriOS
;   Copyright (C) Jeffrey Amelynck 2008. All rights reserved.
;
;   hidnplayr@kolibrios.org
;
;   v0.1
;   date: 4/09/2008
;   type: private beta
;   functions implemented: Read raw data from Digital controller and Analog controller with red led.
;
;   v0.2:
;   date: 5/09/2008
;   type: public beta
;   functions implemented: Same as above plus converting keycodes from keypad do keyboard scancodes.
;                        : To use this function you need a kernel wich can input scancodes using function 18,23.
;                        ; I also did some cleanup and speedup
;
;
;   v0.2.1
;   by O. Bogomaz aka Albom, albom85@yandex.ru
;   using of standart kernel function 72.1
;
;
;   TODO: - Multiple controllers
;         - Analog controller(s)
;
;
;   More info about PSX/PS2 gamepad protocol:
;   http://curiousinventor.com/guides/ps2
;   http://www.geocities.com/digitan000/Hardware/22/e22_page.html
;
;   How to connect your PSX pad to the PC:
;   http://www.emulatronia.com/reportajes/directpad/psxeng/print.htm
;
;
;   PSX-Pad for KolibriOS is distributed in the hope that it will be useful,
;   but WITHOUT ANY WARRANTY.
;   No author or distributor accepts responsibility to anyone for the
;   consequences of using it or for whether it serves any particular purpose or
;   works at all, unless he says so in writing. Refer to the GNU General Public
;   License (the "GPL") for full details.
;
;   Everyone is granted permission to copy, modify and redistribute KolibriOS,
;   but only under the conditions described in the GPL. A copy of this license
;   is supposed to have been given to you along with KolibriOS so you can know
;   your rights and responsibilities. It should be in a file named COPYING.
;   Among other things, the copyright notice and this notice must be preserved
;   on all copies.
;

use32
   
		org    0x0
   
		db     'MENUET01'	       ; 8 byte id
		dd     0x01		           ; header version
		dd     START		       ; start of code
		dd     I_END		       ; size of image
		dd     0x100000 	       ; memory for app
		dd     0x100000 	       ; esp
		dd     0x0 , 0x0	       ; I_Param , I_Icon

; Bits on Data Port (outputs for pc)
command 	equ 0
attention	equ 1
clock		equ 2
vcc		equ (1 shl 3 + 1 shl 4 + 1 shl 5 + 1 shl 6 + 1 shl 7)

; Bits on Status Port (inputs for PC)
data		equ 6
ack		equ 5

__DEBUG__ equ 1
__DEBUG_LEVEL__ equ 2

include '../../macros.inc'
;include 'fdo.inc'

START:
  mov	eax, 40       ; Disable notification of all events
  xor	ebx, ebx
  int	0x40

;  DEBUGF 2,"\nPSX-Pad for KolibriOS v0.2\n\n"

  mov	eax, 46       ; Ask the kernel if wse may use the LPT port
  mov	ebx, 0
  movzx ecx, [BASE]
  movzx edx, [CONTROL]
  int	0x40
  test	eax, eax
  jz	@f

;  DEBUGF 2,"Could not reserve port!\n"
  jmp	exit
@@:

  mov	dx, [CONTROL] ; disable bi-directional data port
  in	al, dx
  and	al, 0xdf
  out	dx, al

  mov	eax, 18       ; read CPU-speed, we'll need it for 100us delay
  mov	ebx, 5
  int	0x40	      ; now we've got the cpuspeed in hz, we need it in Mhz
  xor	edx, edx
  mov	ecx, 1000000
  div	ecx
  mov	[CPUSPEED], eax
;  DEBUGF 2,"CPUspeed: %u\n",eax

;  DEBUGF 1,"Raising attention line\n"
  call	raise_att

;  DEBUGF 1,"Raising Clock\n"
  call	raise_clk

;  DEBUGF 1,"Powering Up controller\n"
  call	raise_vcc

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; All things are ready to go, enter mainloop!
;
; This loop constantly poll's the PSX-pad for data
;

mainloop:

  mov	eax, 5	    ; Lets start by giving the other applications some cpu time, we'll take ours later.
  mov	ebx, 5
  int	0x40

;  DEBUGF 1,"Lowering attention line\n"
  call	lower_att   ; We've got the attention from the PSX-Pad now :) (yes, it's active low..)

;  DEBUGF 1,"Sending Startup byte.. "
  mov	ah, 0x01    ; Startup code
  call	tx_rx
  call	wait_for_ack
;  DEBUGF 1,"Rx: %x\n",bl

;  DEBUGF 1,"Request for data.. "
  mov	ah, 0x42    ; Request for data
  call	tx_rx
  call	wait_for_ack
;  DEBUGF 1,"Rx: %x\n",bl

  cmp	bl, 0x41
  je	digital_controller

  cmp	bl, 0x73
  je	analog_red_controller

;  cmp   ah, 0x23
;  je    negcon_controller

;  cmp   ah, 0x53
;  je    analog_green_controller

;  cmp   ah, 0x12
;  je    psx_mouse


;  DEBUGF 2,"Unsupported controller/mode:%x !\n",bl
  jmp	exit




digital_controller:
  call	command_idle
  call	wait_for_ack
  ; Right now, we receive 0x5a from the controller, wich means: sending data!
;  DEBUGF 1,"Receiving data.. "

  call	command_idle
  call	wait_for_ack
  mov	byte [digital+1], bl

  call	command_idle
  mov	byte [digital+0], bl

;  DEBUGF 1,"Digital data: %x\n",[digital]:4

  mov	ax, word [digital_]
  xor	ax, word [digital]
  mov	cx, word [digital]

  bt	ax, 6	  ; X
  jnc	@f
  pusha
  and	cx, 1 shl 6
  shl	cx, 1
  add	cl, 29
  call	sendkey
  popa
@@:

  bt	ax, 5	  ; O
  jnc	@f
  pusha
  mov	cx, word [digital]
  and	cx, 1 shl 5
  shl	cx, 2
  add	cl, 56
  call	sendkey
  popa
@@:

  bt	ax, 11	  ; Start
  jnc	@f
  pusha
  mov	cx, word [digital]
  and	cx, 1 shl 11
  shr	cx, 4
  add	cl, 28
  call	sendkey
  popa
@@:

  bt	ax, 8	 ; Select
  jnc	@f
  pusha
  mov	cx, word [digital]
  and	cx, 1 shl 8
  shr	cx, 1
  add	cl, 14
  call	sendkey
  popa
@@:

  bt	ax, 12	  ; up
  jnc	@f
  pusha
  mov	cl, 224
  call	sendkey
  mov	cx, word [digital]
  and	cx, 1 shl 12
  shr	cx, 5
  add	cl, 72
  call	sendkey
  popa
@@:

  bt	ax, 13	  ; right
  jnc	@f
  pusha
  mov	cl, 224
  call	sendkey
  mov	cx, word [digital]
  and	cx, 1 shl 13
  shr	cx, 6
  add	cl, 77
  call	sendkey
  popa
@@:

  bt	ax, 14	  ; down
  jnc	@f
  pusha
  mov	cl, 224
  call	sendkey
  mov	cx, word [digital]
  and	cx, 1 shl 14
  shr	cx, 7
  add	cl, 80
  call	sendkey
  popa
@@:

  bt	ax, 15	  ; left
  jnc	@f
  pusha
  mov	cl, 224   ; extended key
  call	sendkey
  mov	cx, word [digital]
  and	cx, 1 shl 15
  shr	cx, 8
  add	cl, 75	  ; left
  call	sendkey
  popa
@@:

  mov	ax, word [digital]
  mov	word [digital_],ax

  call	raise_att
  jmp	mainloop


analog_red_controller:
  call	command_idle
  call	wait_for_ack
  ; Right now, we receive 0x5a from the controller, wich means: sending data!
;  DEBUGF 1,"Receiving data.. "

  call	command_idle
  call	wait_for_ack
  mov	byte [analog_red+5], bl

  call	command_idle
  call	wait_for_ack
  mov	byte [analog_red+4], bl

  call	command_idle
  call	wait_for_ack
  mov	byte [analog_red+3], bl

  call	command_idle
  call	wait_for_ack
  mov	byte [analog_red+2], bl

  call	command_idle
  call	wait_for_ack
  mov	byte [analog_red+1], bl

  call	command_idle
  mov	byte [analog_red+0], bl


;  DEBUGF 2,"Analog data: %x%x\n",[analog_red]:8,[analog_red+4]:4
  call	raise_att
  jmp	mainloop




exit:
  mov	eax, -1
  int	0x40



sendkey:	 ; This function inserts Keyboard Scan-codes into the kernel's queue
		 ; Scancode is in cl
;  mov	eax, 18
;  mov	ebx, 23
;  int	0x40

  pushad

  mov   eax, 72  ; <-- standart function (by Albom)
  mov   ebx, 1
  mov   edx, ecx
  and   edx, 0xff
  mov   ecx, 2
  int   0x40
 
  popad
  ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  Low-level code starts here
;


raise_att:
    mov  al, [PORT_DATA]
    bts  ax, attention
    mov  [PORT_DATA], al

    mov  dx, [BASE]
    out  dx, al

    ret

lower_att:
    mov  al, [PORT_DATA]
    btr  ax, attention
    mov  [PORT_DATA], al

    mov  dx, [BASE]
    out  dx, al

    ret




raise_clk:
    mov  al, [PORT_DATA]
    bts  ax, clock
    mov  [PORT_DATA], al

    mov  dx, [BASE]
    out  dx, al

    ret

lower_clk:
    mov  al, [PORT_DATA]
    btr  ax, clock
    mov  [PORT_DATA], al

    mov  dx, [BASE]
    out  dx, al

    ret




raise_vcc:
    mov  al, [PORT_DATA]
    or	 al, vcc
    mov  [PORT_DATA], al

    mov  dx, [BASE]
    out  dx, al

    ret

lower_vcc:
    mov  al, [PORT_DATA]
    and  al, 0xff - vcc
    mov  [PORT_DATA], al

    mov  dx, [BASE]
    out  dx, al

    ret




wait_for_ack:
    mov  dx, [STATUS]
    mov  ecx, 10000
.loop:
    in	 al, dx
    bt	 ax, ack
    jnc  .ack
    loop .loop

;    DEBUGF 2,"ACK timeout!\n"

;    pop  eax      ; balance the stack, we're not doing a ret like we should..
;    jmp  mainloop

.ack:
;    DEBUGF 1,"ACK !\n"

    ret



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; This code comes from Serge's audio driver.
; If you know a better way to do 100 us wait, please tell me.
; This RDTSC stuff is know to have a bug in the newer AMD processors.
delay:

    push ecx
    push edx
    push ebx
    push eax

    mov  eax, 100
    mov  ecx, [CPUSPEED]
    mul  ecx
    mov  ebx, eax	;low
    mov  ecx, edx	;high
    rdtsc
    add  ebx, eax
    adc  ecx,edx
  @@:
    rdtsc
    sub  eax, ebx
    sbb  edx, ecx
    js	 @B

    pop  eax
    pop  ebx
    pop  edx
    pop  ecx

    ret







tx_rx:
    ; ah = byte to send
    ; bl = received byte
    mov  ecx, 8
    mov  bl, 0

tx_rx_loop:
    call delay
    call lower_clk

    mov  dl, ah
    and  dl, 1
;    DEBUGF 1,"OUTb:%u ", dl

    mov  al, [PORT_DATA]
    and  al, 0xfe
    or	 al, dl
    mov  [PORT_DATA], al

    mov  dx, [BASE]
    out  dx, al

    shr  ah, 1

    call delay
    call raise_clk

    mov  dx, [STATUS]
    in	 al, dx

    shl  al, 1
    and  al, 1 shl 7
;    DEBUGF 1,"INb:%x\n", al
    shr  bl, 1
    or	 bl, al

    loop tx_rx_loop

ret



command_idle:
    ; bl = received byte
    mov  bl, 0
    mov  ecx, 8

command_idle_loop:
    call delay
    call lower_clk

    call delay
    call raise_clk

    mov  dx, [STATUS]
    in	 al, dx

    shl  al, 1
    and  al, 1 shl 7

    shr  bl, 1
    or	 bl, al

    loop  command_idle_loop

ret
   
   
; DATA AREA

;include_debug_strings ; ALWAYS present in data section

; Addresses to PORT
BASE	    dw 0x378
STATUS	    dw 0x379
CONTROL     dw 0x37a
; Buffer for data port
PORT_DATA   db 0

; hmm, what would this be...
CPUSPEED    dd ?

; buffers for data from controller
digital     rb 2
digital_    rb 2   ; this buffer is used to find keychanges (if somebody just pressed/released a key)
analog_red  rb 6
analog_red_ rb 6

I_END:
