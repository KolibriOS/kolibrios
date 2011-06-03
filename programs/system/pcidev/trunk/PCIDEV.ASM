;***************************************************************
; project name:    PCI Device Enumeration
; target platform: KolibriOS
; compiler:        flat assembler 1.68
; version:         2.2
; last update:     December 2007
; maintained by:   Jason Delozier (cordata51@hotmail.com)
;                  Sergey Kuzmin (kuzmin_serg@list.ru)
;                  Mihailov Ilia (ghost.nsk@gmail.com)
;                  Artem Jerdev  (art_zh@yahoo.com)
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
;2.2: PCIDEV   03/01/2010
;Author:    Artem Jerdev <art_zh@yahoo.com>
;Features:
;           fixed
;                 * English in some comments ;)
;           added
;                 * ! user-accessible MMIO channels detection
;                 * ! ASCII-dump as PCI memory access demonstration
;           optimization
;                 * (yet needed)
;----------------------------------------------------------------
;2.1: PCIDEV   ??/04/2007
;Author:    Mihailov Ilia aka Ghost <ghost.nsk@gmail.com>
;Features:
;           fixed
;                 * Some little bugs
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
include '../../../macros.inc'
;include 'macros.inc'

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
	jne	still
	mcall	-1			; close this program

draw_window:
	mov	byte [total], 0
	mcall	12, 1			; start of draw
	; DRAW WINDOW
	mcall	0, dword [Form], dword [Form + 4], 0x13ffffff, 0x805080d0, title
	; Insert horizontal bars  in list area
	mov	eax, 13 		; draw bar system function
	mov	ebx, 18 		; set Xstart position of bar
	shl	ebx, 16 		;
	mov	bx, word [Form] ; get width of window
	sub	bx, 32			; bar is 32 pixels shorter then window width
	mov	ecx, 119 * 65536 + 10	; set Ystart(109) and Height(10) of bar   109
	mov	edx, 0xC0C0C0		; set color of bar
again:	;begin draw bar loop
	mcall				; draw bar to window area
	shr	ecx, 16 		; move the Ystart position to working area
	add	ecx, 34 		; add 34 pixels to Y Start (moves bar down)
	cmp	cx, word [Form + 4]	; is the Ystart position outside of window area
	jae	nomo			; if so stop drawing bars
	sub	ecx, 14 		; if not, we only need 20 pixels between bar tops
	shl	ecx, 16 		; set that values as Ystart
	add	ecx, 10 		; Bar Height is always 10 pixels
	jmp	again			; draw another bar
nomo:					;done drawing bars here
	; start PCI stuff
	call	Get_PCI_Info		; get pci version and last bus, scan for and draw each pci device

	; Window inteface
	mov	cx, [PCI_Version]
	add	ch, '0'
	mov	[PCIWin + 85], ch	; 0xBADCODE but it works !
	mov	ch, cl
	shr	cl, 4
	and	ch, 0x0f
	add	cx, '00'
	mov	[PCIWin + 87], cx
	mov	cl, [PCI_LastBus]	; will only work if [PCI_LastBus] < 10
	add	cl, '0'
	mov	[PCIWin + 106], cl

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
	
	mov	ah, [MMIO_allowed]
	or 	ah, ah
	jz 	@f
	mov	ah, [MMIO_Bus]	; =255 if MMIO disabled / not found
	and	ah, 0x7f
	inc	ah
	jo	@f	
	call	Try_MMIO
@@:
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
	cmp	al, 0xff		; 0xFF means no pci bus found
	jne	Pci_Exists		;
	ret				; if no bus then leave
Pci_Exists:
	mov	byte [V_Bus], 0 	; reset varibles
	mov	byte [V_Dev], 0 	;
	mov	edx,  20 * 65536 + 110	; set start write position
Start_Enum:
	mov	bl, 6			; get a dword
	mov	bh, byte [V_Bus]	; bus of pci device
	mov	ch, byte [V_Dev]	; device number/function
	mov	cl, 0			; offset to device/vendor id
	mcall	62			; get ID's

	cmp	ax, 0			; Vendor ID should not be 0 or 0xFFFF
	je	nextDev 		; check next device if nothing exists here
	cmp	ax, 0xffff		;
	je	nextDev 		;

	mov	word [PCI_Vendor], ax	; There is a device here, save the ID's
	shr	eax, 16 		;
	mov	word [PCI_Device], ax	;
	mov	bl, 4			; Read config byte
	mov	bh, byte [V_Bus]	; Bus #
	mov	ch, byte [V_Dev]	; Device # on bus
	mov	cl, 0x08		; Register to read (Get Revision)
	mcall	62			; Read it
	mov	byte [PCI_Rev], al	; Save it
	mov	cl, 0x0b		; Register to read (Get class)
	mcall	62			; Read it

	mov	byte [PCI_Class], al	; Save it
	mov	cl, 0x0a		; Register to read (Get Subclass)
	mcall	62			; Read it
	mov	byte [PCI_SubClass], al; Save it
; by Mario79 august 2006
	mov	cl, 0x09		; Register to read (Get Interface)
	mcall	62			; Read it
	mov  [PCI_Interface], al	; Save it
;
; by Ghost april 2007
	mov	cl, 0x3c		; Register to read (Get IRQ)
@@:	mcall	62			; Read it
	mov	[PCI_IRQ], al		; Save it
; by CleverMouse juny 2011
	mov	cl, 0x0e
	mcall	62
	push	eax
	inc	byte [total]		; one more device found
	call	Print_New_Device	; print device info to screen
; don't scan for nonzero functions if zero function says "not multifunction device"
	pop	eax
	test	al, al
	js	nextDev
	test	byte [V_Dev], 7
	jnz	nextDev
	or	byte [V_Dev], 7
nextDev:
	inc	byte [V_Dev]		; next device on this bus
	jnz	Start_Enum		; jump until we reach zero
	;(used to be JNO which caused bug!!! 30-4-2006, JMD)
	mov	byte [V_Dev], 0 	; reset device number
	inc	byte [V_Bus]		; next bus
	mov	al, byte [PCI_LastBus]	; get last bus
	cmp	byte [V_Bus], al	; was it last bus
	jbe	Start_Enum		; if not jump to keep searching
	ret

no_ummio_allowed:
	xor 	al,al
	mov 	[MMIO_allowed],al		; re-enter the subroutine
;------------------------------------------------------------------
;* Print device info to screen

Print_New_Device:
	xor 	esi, esi	    	; default text color
	mov 	cl, [MMIO_allowed]
	or	cl,cl
	jz	no_ummio_here
	mov 	ch, byte [V_Bus]
	mov 	cl, byte [V_Dev]
	mcall	62, 11		; detect uMMIO
	and	ax,0x7fff
	inc 	ax			; -1 returned?
	jo 	no_ummio_allowed
	inc 	ax			; -2 returned?
	jo 	no_ummio_here
	inc 	ax			; -3 returned?
	jo 	no_ummio_here
	mov 	esi, 0x990033   ; highlighted text color
	mov 	bh, byte [V_Bus]
	mov 	bl, byte [V_Dev]
	mov 	byte [MMIO_Bus], bh
	mov 	byte [MMIO_Dev], bl
	add 	bh,'0'
	mov 	[PCIWin + 129], bh	; uMMIO bus
	mov 	al, bl
	shr 	al, 1
	shr 	al, 1
	shr 	al, 1
	add 	al,'0'
	mov 	[PCIWin + 131], al	; uMMIO device
	and 	bl, 7
	add 	bl, '0'
	mov 	[PCIWin + 133], bl	; uMMIO function

no_ummio_here:
	movzx	ecx,word [PCI_Vendor]	; Pointer to number to be written
	mcall	47, 0x00040100		; Write Vendor ID
	and	edx, 0xFFFF		;*****************************************
	or	edx, 54 * 65536 ; X start becomes 54
	movzx	ecx, word [PCI_Device]	; get Vendor ID
	mcall				; Draw Vendor ID to Window
	and	edx, 0xFFFF		;*****************************************
	or	edx, 98 * 65536 ; X start becomes 98
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
; from Ghost april 2007                 ;*****************************************
	movzx	ecx, [PCI_IRQ]		; get Interface
	cmp	cl, 0x0f		; IRQ between 0..15
	ja	@f
	and	edx, 0xFFFF
	or	edx, 310 * 65536	; X start becomes 310
	mcall
@@:
;
	;Write Names
	movzx	ebx, dx 	; Set y position
	or	ebx, 340 * 65536	; set Xposition to 340

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
.endfc: test	edx, edx
	jnz	@f
	mov	edx, [Classes + esi * 8 - 4]
@@:
	and	ebx, 0x0000FFFF 	; clear X position
	or	ebx, 0x24E0000		; set X position to 590 pixels
	mcall	4,, 0x80000000,, 32	; draw the text
	movzx	edx, bx 	; get y coordinate
	add	edx, 0x0014000A 	; add 10 to y coordinate and set x coordinate to 20
	mov	[gr_pos], edx
	ret
;------------------------------------------------------------------
; Get the user-MMIO related info
;
; Added on ??-12-2009 by art_zh
;------------------------------------------------------------------
Try_MMIO:
	xor	ebx, ebx
	mov	edx, ebx
	mov	bh, [MMIO_BAR]
	or	bx, 12			; function 12
	mov	ecx, 4096		; =1 page to map
	mcall	62
	mov	[MMIO_Map], eax 	; store MMIO lin.addr.
	mov	ecx, 0x80990022 	; print color : red
	add	bh, '0'
	cmp	eax, -3
	jne	@f
	mov	[bar_um+3], bh
	mov	ebx, [gr_pos]
	mov	edx, bar_um
	mcall	4
	jmp	mmio_next_bar
@@:
	cmp	eax, -4
	jne	@f
	mov	[bar_io+3], bh
	mov	ebx, [gr_pos]
	mov	edx, bar_io
	mcall	4
	jmp	mmio_next_bar
@@:
	cmp	bh, '6' 	; expansion ROM ?
	je	@f
	mov	[bar_ram+3], bh
	mov	ebx, [gr_pos]
	mov	edx, bar_ram
	mcall	4
	jmp	mmio_dump
@@:
	mov	ebx, [gr_pos]
	mov	edx, bar_rom
	mcall	4

mmio_dump:
	mov	edx, [MMIO_Map]
	mov	esi, 64
	mov	ecx, 0x099		; dump color : blue
	add	ebx, 10
	mov	[gr_pos], ebx
	mcall	4
	mov	ecx, [MMIO_Map] 	; release the tried page
	mcall	62,13

mmio_next_bar:
	mov	bh, [MMIO_BAR]
	inc	bh
	cmp	bh,7
	je	@f
	mov	[MMIO_BAR], bh
	add	[gr_pos], 10
	jmp	Try_MMIO

@@:
	xor	bh,bh
	mov	[MMIO_BAR], bh
	ret


include 'vendors.inc'
;------------------------------------------------------------------
; DATA AREA
DATA


Form:	dw 800 ; window width (no more, special for 800x600)
	dw 100 ; window x start
	dw 620 ; window height
	dw 20 ; window y start

title	db 'PCI Device Enumerator v 2.2 by J.Delozier, S.Kuzmin, V.Hanla, M.Zakiyanov, A.Jerdev', 0

PCIWin mls \
	'   Don`t forget to enable PCI Access to Applications in Setup Menu.',\
	'',\
	'PCI Version  = x.xx; Last PCI Bus = x',\
	'User MMIO channel = 0F.F:F ',\
	'Number of PCI units =',\
	'',\
	'VenID DevID Bus# Dev# Fnc Rev  Class  Subclass/ IRQ                 Company                      Description',\
	'                                      Interface',\
	'----- ----- ---- ---- --- ---  -----  --------- --- ------------------------------------------ ----------------'

bar_ram db 'BARx: MMIO block', 0
bar_io	db 'BARx: IO ports',0
bar_um	db 'BARx: unmapped',0
bar_rom db 'BAR6: Expansion ROM', 0

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
PCI_Bus 	db ?
PCI_Dev 	db ?
PCI_Rev 	db ?
; don`t change order!!!
PCI_Class	db ?
PCI_SubClass	db ?
PCI_Interface	db ?
PCI_IRQ 	db ?

align 4
MMIO_Bus	db 255
MMIO_Dev	db 255
MMIO_BAR	db 0
MMIO_allowed	db 1
MMIO_Map	rd 8

gr_pos		dd ?

Proc_Info	process_information
MEOS_APP_END

