;
; Netcfg v1.02
;
; Application to load network drivers in KolibriOS
;
; By hidnplayr
;

use32
	       org    0x0

	       db     'MENUET01'	    ; 8 byte id
	       dd     0x01		    ; header version
	       dd     START		    ; start of code
	       dd     IM_END		    ; size of image
	       dd     (I_END+0x100)	    ; memory for app
	       dd     (I_END+0x100)	    ; esp
	       dd     param, 0x0	   ; I_Param , I_Icon

type_ethernet equ 1

include '../macros.inc'
include 'proc32.inc'
include 'struct.inc'

START:
	; first, check boot parameters

	cmp	byte[param], 0
	je	.noparams

	mcall 40, 0


	push	exit
	cmp	byte[param], 'A'	; A for All
	je	Get_PCI_Info

	cmp	byte[param], 'F'	; F for First
	je	Get_PCI_Info

	ret

.noparams:
	call draw_window

still:	mcall	10			; wait here for event
	dec	eax			; redraw request ?
	jz	red
	dec	eax			; key in buffer ?
	jz	key
	dec	eax			; button in buffer ?
	jz	button
	jmp	still

red:					; redraw
	mcall	9, Proc_Info, -1	; window redraw requested so get new window coordinates and size
	mov	eax, [Proc_Info.box.left]; store the window coordinates into the Form Structure
	mov	[Form + 2], ax		; x start position
	mov	eax, [Proc_Info.box.top];
	mov	[Form + 6], ax		; ystart position
	mov	eax, [Proc_Info.box.width]	;
	mov	[Form], ax		; window width
	mov	eax, [Proc_Info.box.height]	;
	mov	[Form + 4] ,ax		; window height
	call	draw_window		; go redraw window now
	jmp	still

key:					; key
	mcall	2			; just read it and ignore
	jmp	still
button: 				; button
	mcall	17			; get id

	cmp	ah, 1			; button id = 1 ?
	jne	@f
exit:	mcall	-1			; close this program
       @@:
	cmp	eax,0x0000ff00
	jg	load_drv

	cmp	ah, 4
	je	hook

	cmp	ah, 5
	je	reset

	cmp	ah, 6
	je	unload

	jmp	still


load_drv:
	shr	eax, 16
	mov	word [selected], ax

	mov	bl , 6			; get a dword
	mov	bh , ah 		; bus
	mov	ch , al 		; dev
	mov	cl , 0			; offset to device/vendor id
	mcall	62			; get ID's

	mov	word [PCI_Vendor], ax
	shr	eax, 16
	mov	word [PCI_Device], ax
	call	get_drv_ptr

	mov	ecx, eax
	mcall	68, 16

	mov	[IOCTL.handle], eax

	call	draw_window

	cmp	[IOCTL.handle], 0
	jne	still

	mcall	4, 20 shl 16 + 30, 1 shl 31 + 0x00ff0000 , load_error

	jmp	still


hook:
	mov	ax , [selected]
	test	ax , ax
	jz	still

	mov	[hardwareinfo.pci_dev], al
	mov	[hardwareinfo.pci_bus], ah

	mov	[IOCTL.io_code], 1 ; SRV_HOOK
	mov	[IOCTL.inp_size], 3
	mov	[IOCTL.input], hardwareinfo
	mov	[IOCTL.out_size], 0
	mov	[IOCTL.output], 0

	mcall	68, 17, IOCTL

	mov	byte[drivernumber], al

	jmp	still

reset:
	movzx	ebx, byte[drivernumber]
	mcall	74,,2

	jmp	still

unload:
	movzx	ebx, byte[drivernumber]
	mcall	74,,3

	jmp	still

draw_window:
	mcall	12, 1			; start of draw
	mcall	0, dword [Form], dword [Form + 4], 0x13ffffff, 0x805080d0, title

	mcall	8, 136 shl 16 + 100, 35 shl 16 + 18, 4, 0x00007f00	 ; SLIP

	call	Get_PCI_Info		; get pci version and last bus, scan for and draw each pci device

	cmp	edx, 20 shl 16 + 110
	je	.nonefound

	mcall	4, 20 shl 16 + 100, 1 shl 31 + 0x00000000 , caption

	cmp	[selected], 0
	jz	.done
	cmp	[IOCTL.handle] ,0
	jz	.done

	mcall	8, 18 shl 16 + 100, 35 shl 16 + 18, 4, 0x00007f00
	mcall	,, 55 shl 16 + 18, 5, 0x0000007f
	mcall	,, 75 shl 16 + 18, 6, 0x007f0000

	mcall	4, 33 shl 16 + 42, 1 shl 31 + 0x00ffffff , btn_start
	mcall	, 33 shl 16 + 62, , btn_reset
	mcall	, 36 shl 16 + 82, , btn_stop

;        mcall   , 140 shl 16 + 62, 1 shl 31 + 0x00000000 , devicename

	jmp	.done

.nonefound:
	mcall	4, 20 shl 16 + 30, 1 shl 31 + 0x00ff0000 , nonefound
.done:
	mcall	12, 2			; end of draw
	ret





;------------------------------------------------------------------
;* Gets the PCI Version and Last Bus
Get_PCI_Info:
	mcall	62, 0
	mov	word [PCI_Version], ax
	mcall	62, 1
	mov	byte [PCI_LastBus], al
	;----------------------------------------------------------
	;* Get all devices on PCI Bus
	mov	edx, 20 shl 16 + 110  ; set start write position
	cmp	al , 0xff		 ; 0xFF means no pci bus found
	jne	Pci_Exists		;
	ret				; if no bus then leave
Pci_Exists:
	mov	byte [V_Bus], 0 	; reset varibles
	mov	byte [V_Dev], 0 	;
Start_Enum:
	mov	bl , 6			 ; get a dword
	mov	bh , byte [V_Bus]	 ; bus of pci device
	mov	ch , byte [V_Dev]	 ; device number/function
	mov	cl , 0			 ; offset to device/vendor id
	mcall	62			; get ID's

	cmp	ax, 0			; Vendor ID should not be 0 or 0xFFFF
	je	nextDev 		; check next device if nothing exists here
	cmp	ax, 0xffff		;
	je	nextDev 		;

	mov	word [PCI_Vendor], ax	; There is a device here, save the ID's
	shr	eax, 16 		;
	mov	word [PCI_Device], ax	;
	mov	bl , 4			 ; Read config byte
	mov	bh , byte [V_Bus]	 ; Bus #
	mov	ch , byte [V_Dev]	 ; Device # on bus
	mov	cl , 0x08		 ; Register to read (Get Revision)
	mcall	62			; Read it
	mov	byte [PCI_Rev], al	; Save it
	mov	cl , 0x0b		 ; Register to read (Get class)
	mcall	62			; Read it
	
	mov	byte [PCI_Class], al	; Save it
	mov	cl , 0x0a		 ; Register to read (Get Subclass)
	mcall	62			; Read it
	mov	byte [PCI_SubClass], al ; Save it
	mov	cl , 0x09		 ; Register to read (Get Interface)
	mcall	62			; Read it
	mov	[PCI_Interface], al	; Save it
	mov	cl , 0x3c		 ; Register to read (Get IRQ)
@@:	mcall	62			; Read it
	mov	[PCI_IRQ], al		; Save it
;
;        inc     byte [total]            ; one more device found

	cmp	byte [PCI_Class],2
	jne	nextDev

	cmp	byte[param], 0
	jne	load_and_start

	call	Print_New_Device	; print device info to screen

nextDev:
	add	byte [V_Dev], 8 	; lower 3 bits are the function number

	jnz	Start_Enum		; jump until we reach zero
	mov	byte [V_Dev], 0 	; reset device number
	inc	byte [V_Bus]		; next bus
	mov	al , byte [PCI_LastBus]  ; get last bus
	cmp	byte [V_Bus], al	; was it last bus
	jbe	Start_Enum		; if not jump to keep searching
	ret



load_and_start:

	call	get_drv_ptr
	cmp	eax, lbl_none
	je	.next

	mov	ecx, eax
	mcall	68, 16
	test	eax, eax
	jz	.next
	mov	[IOCTL.handle], eax

	mov	al, [V_Dev]
	mov	[hardwareinfo.pci_dev], al
	mov	al, [V_Bus]
	mov	[hardwareinfo.pci_bus], al

	mov	[IOCTL.io_code], 1 ; SRV_HOOK
	mov	[IOCTL.inp_size], 3
	mov	[IOCTL.input], hardwareinfo
	mov	[IOCTL.out_size], 0
	mov	[IOCTL.output], 0

	mcall	68, 17, IOCTL

       .next:
	cmp	byte[param], 'A'
	je	nextDev
	jmp	exit



;------------------------------------------------------------------
;* Print device info to screen
Print_New_Device:

	push	edx			; Magic ! (to print a button...)

	mov	ebx, 18 shl 16
	mov	bx , [Form]
	sub	bx , 36

	mov	cx , dx
	dec	cx
	shl	ecx, 16
	add	ecx, 9

	movzx	edx, byte [V_Bus]
	shl	dx , 8
	mov	dl , byte [V_Dev]

	mov	esi, 0x0000c0ff        ; color: yellow if selected, blue otherwise
	cmp	word [selected], dx
	jne	@f
	mov	esi, 0x00c0c000
       @@:

	shl	edx, 8
	or	dl , 0xff

	mcall	8
	pop	edx

	xor	esi, esi		; Color of text
	movzx	ecx, word [PCI_Vendor]	; number to be written
	mcall	47, 0x00040100		; Write Vendor ID

	add	edx, (4*6+18) shl 16
	movzx	ecx, word [PCI_Device]	; get Vendor ID
	mcall				; Draw Vendor ID to Window

	add	edx, (4*6+18) shl 16
	movzx	ecx, byte [V_Bus]	; get bus number
	mcall	,0x00020100		; draw bus number to screen

	add	edx, (2*6+18) shl 16
	movzx	ecx, byte [V_Dev]	; get device number
	shr	ecx, 3			; device number is bits 3-7
	mcall				; Draw device Number To Window

	add	edx, (2*6+18) shl 16
	movzx	ecx, byte [PCI_Rev]	; get revision number
	mcall				; Draw Revision to screen

	add	edx, (2*6+18) shl 16
	movzx	ecx, [PCI_IRQ]
	cmp	cl , 0x0f		; IRQ must be between 0 and 15
	ja	@f
	mcall
@@:
;
	;Write Names
	movzx	ebx, dx 		; Set y position
	or	ebx, 230 shl 16 	; set Xposition

;------------------------------------------------------------------
; Prints the Vendor's Name based on Vendor ID
;------------------------------------------------------------------
	mov	edx, VendorsTab
	mov	cx , word[PCI_Vendor]
	
.fn:	mov	ax , [edx]
	add	edx, 6
	test	ax , ax
	jz	.find
	cmp	ax , cx
	jne	.fn
.find:	mov	edx, [edx - 4]
	mcall	4,, 0x80000000		; lets print the vendor Name

;------------------------------------------------------------------
; Get description based on Class/Subclass
;------------------------------------------------------------------
	mov	eax, dword [PCI_Class]
	and	eax, 0xffffff
	xor	edx, edx
	xor	esi, esi
.fnc:	inc	esi
	mov	ecx, [Classes + esi * 8 - 8]
	cmp	cx , 0xffff
	je	.endfc
	cmp	cx , ax
	jne	.fnc
	test	ecx, 0xff000000
	jz	@f
	mov	edx, [Classes + esi * 8 - 4]
	jmp	.fnc
@@:	cmp	eax, ecx
	jne	.fnc
	xor	edx, edx
.endfc: test	edx, edx
	jnz	@f
	mov	edx, [Classes + esi * 8 - 4]
@@:	
	add	ebx, 288 shl 16
	mcall	4,, 0x80000000,, 32	; draw the text
	movzx	edx, bx 		; get y coordinate
	add	edx, 0x0014000A 	; add 10 to y coordinate and set x coordinate to 20

;------------------------------------------------------------------
; Print Driver Name
;------------------------------------------------------------------
	push	edx
	add	ebx, 120 shl 16
	push	ebx

	call	get_drv_ptr
	mov	edx, eax
	pop	ebx
	mcall	4,,0x80000000	       ; lets print the vendor Name
	pop	edx
	ret

get_drv_ptr:
	mov	eax, driverlist        ; eax will be the pointer to latest driver title
	mov	ebx, driverlist        ; ebx is the current pointer
	mov	ecx, dword[PCI_Vendor] ; the device/vendor id of we want to find

       driverloop:
	inc	ebx

	cmp	byte[ebx],0
	jne	driverloop

	inc	ebx		       ; the device/vendor id list for the driver eax is pointing to starts here.

       deviceloop:
	cmp	dword[ebx],0
	je	nextdriver

	cmp	dword[ebx],ecx
	je	driverfound

	add	ebx,4
	jmp	deviceloop

       nextdriver:
	add	ebx,4

	cmp	dword[ebx],0
	je	nodriver

	mov	eax,ebx
	jmp	driverloop

       nodriver:
	mov	eax, lbl_none	       ; lets print the vendor Name
	ret

       driverfound:
	ret

include 'vendors.inc'
include 'drivers.inc'


;------------------------------------------------------------------
; DATA AREA


DATA


Form:	dw 800 ; window width (no more, special for 800x600)
	dw 100 ; window x start
	dw 220 ; window height
	dw 100 ; window y start

title	db 'Network Driver Control Center', 0

caption db 'Vendor Device Bus  Dev  Rev  IRQ   Company                                         Description         DRIVER',0
nonefound db 'No compatible devices were found!',0
btn_start db 'Start device',0
btn_reset db 'Reset device',0
btn_stop db 'Stop device',0
lbl_none db 'none',0
load_error db 'Could not load driver!',0

devicename     db 'test'
rb 64
		db 0

hardwareinfo:
   .type	db 1 ; pci
   .pci_bus	db ?
   .pci_dev	db ?


IM_END:

;------------------------------------------------------------------
; UNINITIALIZED DATA AREA


IOCTL:
   .handle	dd ?
   .io_code	dd ?
   .input	dd ?
   .inp_size	dd ?
   .output	dd ?
   .out_size	dd ?

drivernumber	db ?
MAC		dp ?


type		db ?
selected	dw ?
V_Bus		db ?
V_Dev		db ?
PCI_Version	dw ?
PCI_LastBus	db ?
PCI_Vendor	dw ?
PCI_Device	dw ?
PCI_Bus 	db ?
PCI_Dev 	db ?
PCI_Rev 	db ?
; don`t change order!!!
PCI_Class	db ?
PCI_SubClass	db ?
PCI_Interface	db ?
PCI_IRQ 	db ?

Proc_Info	process_information

param		rb 1024


I_END: