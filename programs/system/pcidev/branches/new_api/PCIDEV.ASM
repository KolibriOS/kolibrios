;***************************************************************
; project name:    PCI Device Enumeration
; target platform: KolibriOS
; compiler:        flat assmebler 1.66
; version:         2.1
; last update:     April 2007
; maintained by:   Jason Delozier (cordata51@hotmail.com)
;                  Sergey Kuzmin (kuzmin_serg@list.ru)
;                  Mihailov Ilia (ghost.nsk@gmail.com)
; project site:    http://www.coolthemes.narod.ru/pcidev.html
;***************************************************************
;Summary: This program will attempt to scan the PCI Bus
;        and display basic information about each device
;        connected to the PCI Bus.
;***************************************************************
;HISTORY:
;keep dates in european format (dd/mm/yyyy), please
; '!' means big changes
;
;to-do:
; more vendors
; vendor's website
; Subsystem id and Subsystem vendor id detection

; Full device detection (like "ATI Radeon 9200") will increase app
; size a lot and probably it is function of particular drivers
;----------------------------------------------------------------
;2.1n: PCIDEV   ??/04/2007
;Author:    Mihailov Ilia aka Ghost <ghost.nsk@gmail.com>
;Features:
;           added
;                 * new PCI API
;----------------------------------------------------------------
;2.1: PCIDEV   ??/04/2007
;Author:    Mihailov Ilia aka Ghost <ghost.nsk@gmail.com>
;Features:
;           fixed
;                 * Same little bugs
;                 * Source code formating
;                 * PCI version normalized 0210 -> 2.10
;           added
;                 * + 13 vendor (total now 1000!)
;                 * + 3 class type
;                 * Fast call source ready
;                 * Device IRQ detection
;           optimized
;                 * ! size optimized (61025 -> 32501 see added)
;                 * easy to add new vendor & class names
;----------------------------------------------------------------
;2.0: PCIDEV   30/08/2006
;(it differs a lot from the version 1.0, which was introduced 19 months ago)
;Author:    Marat Zakiyanov aka Mario79 <mario79@bk.ru>
;           Sergey Kuzmin aka Wildwest <kuzmin_serg@list.ru>
;Features:
;           added
;                 * Detection of Interface by Mario79
;                 * 122 vendor id's by Wildwest
;                 * Description is based on Class, SubClass and Interface now (PCI 3.0) by Wildwest
;----------------------------------------------------------------
;1.31: PCIDEV   13/05/2006
;Author:    Jason Delozier <cordata51@hotmail.com>
;Features:
;           fixed
;                 * ! bug in Company Name look up code that would cause Unknown Name errors.
;                 * ! possible bugs, many instructions missing byte, word, dword prefixes
;                 * ! possible bug which could have occured after removing  "PREVIOUSVERSIONLIST"
;                     entry in loop up code and not fixing jump parameters.
;           added
;                 *  comments to various parts of the code
;           optimized
;                 * various parts of the source, too many to remember and mention.
;                 * changed entries for Subclasses in vendors.inc to Byte format, saves a little space.
;----------------------------------------------------------------
;1.30: PCIDEV   11/05/2006
;Author:    Sergey Kuzmin aka Wildwest <kuzmin_serg@list.ru>
;Features:
;           added
;                * 3 new vendor id's (ESS from Madis Kalme and 2 id's
;                  forgotten from 1.15 release: Broadcom -SiByte and Chaintech Comp.)
;           changed
;                * I don't know why other devs (Jason or Victor) changed window style
;                  to old ugly one, so I changed it back to skinned type 3.
;                * the same goes to the use of macroc.inc - it is enabled again.
;           deleted
;                * there is no more label "PREVIOUSVERSIONLIST" - id's moved to the
;                  appropriate parts of global list.
;----------------------------------------------------------------
;1.29: PCIDEV   30/04/2006
;Author:    Jason Delozier <cordata51@hotmail.com>
;Features:
;           fixed
;                 * ! bug that would not allow devices with device
;                   numbers > 16 to be displayed.
;           added
;                 * ! another heading called "FNC" (function) which allows
;                   the multipurpose Device/Function varible to be split and
;                   displayed to the user properly.
;                 * horizontal bars to display for easier reading.
;           optimized
;                 * vendor/description search routines for speed and space.
;----------------------------------------------------------------
;1.25: PCIDEV   02/10/2005
;Author:    Sergey Kuzmin aka Wildwest <kuzmin_serg@list.ru>
;Features:
;            changed
;                  * ! Description is based on Class and SubClass
;                     now (PCI 3.0). The Names of Classes and SubClasses
;                     are in the end of Vendors.inc
;            deleted
;                  * label "Descriptions" (names of Classes)
;
;----------------------------------------------------------------
;1.20: PCIDEV   16/08/2005
;Author:    Victor Alberto Gil Hanla a.k.a. vhanla <vhanla@gmail.com>
;Features:
;           added
;                  * ! many vendor lists (865)
;           deleted
;                  * previous version's list
;           changed
;                  * previous Company Name searching and printing
;----------------------------------------------------------------
;1.15: PCIDEV   03/06/2005
;Author:    Sergey Kuzmin aka Wildwest <kuzmin_serg@list.ru>
;Features:
;           added
;                 * quantity of devices,
;                 * ! detection of Company Name based on Vendor ID,
;                 * database of VenID (35 ID's),
;                 * macros.inc for smaller size,
;           changed
;                 * interface+(skinned window),
;                 * VenID before DevID in 'table'(was DevID before VenID)
;----------------------------------------------------------------
;1.0: PCIDEV    30/01/2005
;Author:    Jason Delozier
;Features:
;          able to
;                 * detect PCI version,
;                 * quantity of PCI buses,
;                 * Vendor&Device ID for appropriate Device on Bus;
;                 * detect Revision, Class and Subclass of Device,
;                 * and make Description based on Class
;-------------------------------------------------------------
include 'lang.inc'
include	'macros.inc'
MEOS_APP_START
CODE
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
	mov	eax, [Proc_Info.x_start]; store the window coordinates into the Form Structure
	mov	[Form + 2], ax		; x start position
	mov	eax, [Proc_Info.y_start];
	mov	[Form + 6], ax		; ystart position
	mov	eax, [Proc_Info.x_size]	;
	mov	[Form], ax		; window width
	mov	eax, [Proc_Info.y_size]	;
	mov	[Form + 4] ,ax		; window height
	call	draw_window		; go redraw window now
	jmp	still

key:					; key
	mcall	2			; just read it and ignore
	jmp	still
button:					; button
	mcall	17			; get id
	cmp	ah, 1			; button id = 1 ?
	jne	still
	mcall	-1			; close this program

draw_window:
	mov	byte [total], 0
	mcall	12, 1			; start of draw
	; DRAW WINDOW
	mcall	0, dword [Form], dword [Form + 4], 0x13ffffff, 0x805080d0, title
	; Insert horizontal bars  in list area
	mov	eax, 13			; draw bar system function
	mov	ebx, 18			; set Xstart position of bar
	shl	ebx, 16			;
	mov	bx, word [Form]	; get width of window
	sub	bx, 32			; bar is 32 pixels shorter then window width
	mov	ecx, 119 * 65536 + 10	; set Ystart(109) and Height(10) of bar   109
	mov	edx, 0xC0C0C0		; set color of bar
again:	;begin draw bar loop
	mcall				; draw bar to window area
	shr	ecx, 16			; move the Ystart position to working area
	add	ecx, 34			; add 34 pixels to Y Start (moves bar down)
	cmp	cx, word [Form + 4]	; is the Ystart position outside of window area
	jae	nomo			; if so stop drawing bars
	sub	ecx, 14			; if not, we only need 20 pixels between bar tops
	shl	ecx, 16			; set that values as Ystart
	add	ecx, 10			; Bar Height is always 10 pixels
	jmp	again			; draw another bar
nomo:					;done drawing bars here
	; start PCI stuff
	call	Get_PCI_Info		; get pci version and last bus, scan for and draw each pci device

	; Window inteface
	mov	cx, [PCI_Version]
	add	ch, '0'
	mov	[PCIWin + 85], ch	; 0xBADCODE but it work !
	mov	ch, cl
	shr	cl, 4
	and	ch, 0x0f
	add	cx, '00'
	mov	[PCIWin + 87], cx
	mov	cl, [PCI_LastBus]
	add	cl, '0'
	mov	[PCIWin + 105], cl

	mov	edx, PCIWin
	mov	ebx, 20 * 65536 + 25	; x start, ystart of text
	mov	ecx, 0x224466		; color of text
	mov	eax, 4
@@:	movzx	esi, byte[edx]
	inc	edx
	mcall
	add	ebx, 10
	add	edx, esi
	cmp	byte[edx], -1
	jne	@b
	; Quantity of devices...
	movzx	ecx, byte [total]	; number to draw
	mcall	47, 0x00020000,,150 * 65536 + 65, 0x224466

	mcall	12, 2			; end of draw
	ret

;------------------------------------------------------------------
;* Gets the PCI Version and Last Bus
Get_PCI_Info:
	mov	ebp, 1
	mcall	57
	mov	word [PCI_Version], bx
	mov	byte [PCI_LastBus], cl
	;----------------------------------------------------------
	;* Get all devices on PCI Bus
	cmp	al, 0xff		; 0xFF means no pci bus found
	jne	Pci_Exists		;
	ret				; if no bus then leave
Pci_Exists:
	mov	byte [V_Bus], 0		; reset varibles
	mov	byte [V_Dev], 0		;
	mov	edx,  20 * 65536 + 110	; set start write position
Start_Enum:
	mov	ebp, 0x0a		; get a dword
	xor	edi, edi		; offset to device/vendor id
	mov	bh, byte [V_Bus]	; bus of pci device
	mov	bl, byte [V_Dev]	; device number/function
	mcall	57			; get ID's
	cmp	cx, 0			; Vendor ID should not be 0 or 0xFFFF
	je	nextDev			; check next device if nothing exists here
	cmp	cx, 0xffff		;
	je	nextDev			;
	mov	word [PCI_Vendor], cx	; There is a device here, save the ID's
	shr	ecx, 16			;
	mov	word [PCI_Device], cx	;



	mov	ebp, 8			; Read config byte
	mov	bh, byte [V_Bus]	; Bus #
	mov	bl, byte [V_Dev]	; Device # on bus
	mov	edi, 0x08		; Register to read (Get Revision)
	mcall	57			; Read it
	mov	byte [PCI_Rev], cl	; Save it

	mov	edi, 0x0b		; Register to read (Get class)
	mcall	57			; Read it
	mov	byte [PCI_Class], cl	; Save it

	mov	edi, 0x0a		; Register to read (Get Subclass)
	mcall	57			; Read it
	mov	byte [PCI_SubClass], cl; Save it
; from Mario79 august 2006
	mov	edi, 0x09		; Register to read (Get Interface)
	mcall	57			; Read it
	mov  [PCI_Interface], cl	; Save it
;
; from Ghost april 2007
	mov	edi, 0x3c		; Register to read (Get IRQ)
	mcall	57			; Read it
	mov	[PCI_IRQ], cl		; Save it
;
	inc	byte [total]		; one more device found
	call	Print_New_Device	; print device info to screen
nextDev:
	inc	byte [V_Dev]		; next device on this bus
	jnz	Start_Enum		; jump until we reach zero
	;(used to be JNO which caused bug!!! 30-4-2006, JMD)
	mov	byte [V_Dev], 0		; reset device number
	inc	byte [V_Bus]		; next bus
	mov	al, byte [PCI_LastBus]	; get last bus
	cmp	byte [V_Bus], al	; was it last bus
	jbe	Start_Enum		; if not jump to keep searching
	ret

;------------------------------------------------------------------
;* Print device info to screen
Print_New_Device:
	xor	esi, esi		; Color of text
	movzx	ecx,word [PCI_Vendor]	; Pointer to number to be written
	mcall	47, 0x00040100		; Write Vendor ID
	and	edx, 0xFFFF		;*****************************************
	or	edx, 54 * 65536	; X start becomes 54
	movzx	ecx, word [PCI_Device]	; get Vendor ID
	mcall				; Draw Vendor ID to Window
	and	edx, 0xFFFF		;*****************************************
	or	edx, 98 * 65536	; X start becomes 98
	movzx	ecx, byte [V_Bus]	; get bus number
	mcall	,0x00020100		; draw bus number to screen
	and	edx, 0xFFFF		;*****************************************
	or	edx, 128 * 65536	; X start becomes 128
	movzx	ecx, byte [V_Dev]	; get device number
	shr	ecx, 3			; device number is bits 3-7
	mcall				; Draw device Number To Window

	and	edx, 0xFFFF		;*****************************************
	or	edx, 155 * 65536	; X start becomes 155
	movzx	ecx, byte [V_Dev]	; get Function number
	and	ecx, 7			; function is first 3 bits
	mcall				; Draw Function Number To Window
	and	edx, 0xFFFF		;*****************************************
	or	edx, 179 * 65536	; X start becomes 179
	movzx	ecx, byte [PCI_Rev]	; get revision number
	mcall				; Draw Revision to screen
	and	edx, 0xFFFF		;*****************************************
	or	edx, 215*65536		; X start becomes 215
	movzx	ecx, byte [PCI_Class]	; get PCI_Class
	mcall				; Draw Class to screen
	and	edx, 0xFFFF		;*****************************************
	or	edx, 250*65536		; X start becomes 250
	movzx	ecx, byte [PCI_SubClass]; get sub class
	mcall				; Draw Sub Class to screen
; from Mario79 august 2006
	and	edx, 0xFFFF		;*****************************************
	or	edx, 280 * 65536	; X start becomes 280
	movzx	ecx, [PCI_Interface]	; get Interface
	mcall
;
; from Ghost april 2007			;*****************************************
	movzx	ecx, [PCI_IRQ]		; get Interface
	cmp	cl, 0x0f		; IRQ between 0..15
	ja	@f
	and	edx, 0xFFFF
	or	edx, 310 * 65536	; X start becomes 310
	mcall
@@:
;
	;Write Names
	movzx	ebx, dx		; Set y position
	or	ebx, 340*65536		; set Xposition to 340

;------------------------------------------------------------------
; Prints the Vendor's Name based on Vendor ID
;
; Modified on ??-04-2007 by Ghost for size
;------------------------------------------------------------------
	mov	edx, VendorsTab
	mov	cx, word[PCI_Vendor]

.fn:	mov	ax, [edx]
	add	edx, 6
	test	ax, ax
	jz	.find
	cmp	ax, cx
	jne	.fn
.find:	mov	edx, [edx - 4]
	mcall	4,, 0x80000000		; lets print the vendor Name

;------------------------------------------------------------------
; Get description based on Class/Subclass
;
; Modified on ??-04-2007 by Ghost for size
;------------------------------------------------------------------
	mov	eax, dword [PCI_Class]
	and	eax, 0xffffff
	xor	edx, edx
	xor	esi, esi
.fnc:	inc	esi
	mov	ecx, [Classes + esi * 8 - 8]
	cmp	cx, 0xffff
	je	.endfc
	cmp	cx, ax
	jne	.fnc
	test	ecx, 0xff000000
	jz	@f
	mov	edx, [Classes + esi * 8 - 4]
	jmp	.fnc
@@:	cmp	eax, ecx
	jne	.fnc
	xor	edx, edx
.endfc:	test	edx, edx
	jnz	@f
	mov	edx, [Classes + esi * 8 - 4]
@@:
	and	ebx, 0x0000FFFF		; clear X position
	or	ebx, 0x24E0000		; set X position to 590 pixels
	mcall	4,, 0x80000000,, 32	; draw the text
	movzx	edx, bx		; get y coordinate
	add	edx, 0x0014000A		; add 10 to y coordinate and set x coordinate to 20
	ret

include	'vendors.inc'
;------------------------------------------------------------------
; DATA AREA
DATA


Form:	dw 800 ; window width (no more, special for 800x600)
	dw 100 ; window x start
	dw 420 ; window height
	dw 100 ; window y start

if lang eq it
title	db 'PCI Device Enumeration v 2.1n new PCI API test', 0
PCIWin mls \
	'Ricordarsi di abilitare l accesso alla PCI per le applicazioni dal menu.',\
	'',\
	'Versione PCI = x.xx',\
	'Ultimo Bus PCI = x',\
	'Numerod di devices =',\
	'',\
	'VenID DevID Bus# Dev# Fnc Rev  Class  Subclass/ IRQ                 Compania                     Descrizione',\
	'                                      Interface',\
	'----- ----- ---- ---- --- ---  -----  --------- --- ------------------------------------------ ----------------'
else
title	db 'PCI Device Enumeration v 2.1n new PCI API test', 0
PCIWin mls \
	'Please remember to enable PCI Access to Applications in Setup Menu.',\
	'',\
	'PCI Version  = x.xx',\
	'Last PCI Bus = x',\
	'Quantity of devices =',\
	'',\
	'VenID DevID Bus# Dev# Fnc Rev  Class  Subclass/ IRQ                 Company                      Description',\
	'                                      Interface',\
	'----- ----- ---- ---- --- ---  -----  --------- --- ------------------------------------------ ----------------'
end if
;------------------------------------------------------------------
; UNINITIALIZED DATA AREA
UDATA

total		db ?
V_Bus		db ?
V_Dev		db ?
PCI_Version	dw ?
PCI_LastBus	db ?
PCI_Device	dw ?
PCI_Vendor	dw ?
PCI_Bus		db ?
PCI_Dev		db ?
PCI_Rev		db ?
; don`t change order!!!
PCI_Class	db ?
PCI_SubClass	db ?
PCI_Interface	db ?
PCI_IRQ		db ?

Proc_Info	process_information
MEOS_APP_END
