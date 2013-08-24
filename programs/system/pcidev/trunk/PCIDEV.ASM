;***************************************************************
; project name:    PCI Device Enumeration
; target platform: KolibriOS
; compiler:        flat assembler 1.68
; version:         2.3
; last update:     Jule 2011
; maintained by:   Jason Delozier <cordata51@hotmail.com>
;                  Sergey Kuzmin <kuzmin_serg@list.ru>
;                  Mihailov Ilia <ghost.nsk@gmail.com>
;                  Marat Zakiyanov <mario79@bk.ru>
;                  Artem Jerdev  <art_zh@yahoo.com>
;                  Evgeny Grechnikov (diamond)
;                  Veronica (CleverMouse)
; old project site:  http://www.coolthemes.narod.ru/pcidev.html
; new project site:  http://board.kolibrios.org/viewtopic.php?f=42&t=73
;***************************************************************
;Summary: This program will attempt to scan the PCI Bus
;        and display basic information about each device
;        connected to the PCI Bus.
;***************************************************************
	use32
	org 0x0
	db 'MENUET01'	; 8 byte id
	dd 0x01		; header version
	dd START	; start of code
	dd IM_END	; size of image
	dd I_END	; memory for app
	dd stacktop	; esp
	dd 0	; I_Param
	dd path		; APPLICATION PATH
;-----------------------------------------------------------------------------
include 'lang.inc'	;language support
include '../../../macros.inc'
include	'../../../develop/libraries/box_lib/load_lib.mac'
;-----------------------------------------------------------------------------
@use_library	; load_lib macro
;-----------------------------------------------------------------------------
START:
	mcall	68,11
	mcall	66,1,1
;-----------------------------------------------------------------------------
load_libraries l_libs_start,end_l_libs
;-----------------------------------------------------------------------------
;OpenDialog	initialisation
	push    dword OpenDialog_data
	call    [OpenDialog_Init]

	mov	edi,filename_area
	mov	esi,start_temp_file_name
	call	copy_file_name_path
;-----------------------------------------------------------------------------
	mcall	68,12,4096*4 ; 16 Kb - I hope this will be enough for store of data
	mov	[store_text_area_start],eax
;-----------------------------------------------------------------------------
	call draw_window
still:
	mcall	10			; wait here for event
	dec	eax			; redraw request ?
	jz	red
	dec	eax			; key in buffer ?
	jz	key
	dec	eax			; button in buffer ?
	jz	button
	jmp	still
;-----------------------------------------------------------------------------
red:					; redraw
	call	get_window_param
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
;-----------------------------------------------------------------------------
key:					; key
	mcall	2			; just read it and ignore
	cmp	[extended_key],1
	je	.extended_key
	test	al, al
	jnz	still
	cmp	ah, 0xE0
	jne	@f
	mov	[extended_key],1
	jmp	still
@@:
	cmp	ah,129	; Esc
	je	button.exit
	cmp	ah,159
	je	call_OpenDialog
	jmp	still
.extended_key:
	mov	[extended_key],0
	cmp	ah,129	; Esc
	je	button.exit
	cmp	ah,159
	je	call_OpenDialog
	jmp	still
;-----------------------------------------------------------------------------
button: 				; button
	mcall	17			; get id
	cmp	ah,2
	je	call_OpenDialog
	cmp	ah, 1			; button id = 1 ?
	jne	still
.exit:
	mcall	-1			; close this program
;-----------------------------------------------------------------------------
call_OpenDialog:
	mov	[OpenDialog_data.type],1	; Save

	push    dword OpenDialog_data
	call    [OpenDialog_Start]

	cmp	[OpenDialog_data.status],2	; OpenDialog does not start
	je	.save_file_default_path

	cmp	[OpenDialog_data.status],1
	jne	still

	call	store_data
	jmp	still
;----------------------------------------
.save_file_default_path:
	mov	edi,file_name
	mov	esi,file_default_path
	call	copy_file_name_path
	call	store_data
	jmp	still
;----------------------------------------
copy_file_name_path:
	xor	eax,eax
	cld
@@:
	lodsb
	stosb
	test	eax,eax
	jnz	@r
	ret
;-----------------------------------------------------------------------------
prepare_text_area:
	mov	edi,[store_text_area_start]

	push	edi
	mov	ecx,4096 ; 16 Kb - I hope this will be enough for store of data
	mov	eax,dword '    '
	cld
	rep	stosd
	pop	edi

	mov	esi,PCIWin
	xor	ecx,ecx
@@:
	mov	cl,[esi]
	inc	esi
	rep	movsb
	mov	al,0Ah ; CR - carriage return
	stosb
	cmp	[esi],byte 0xFF
	jne	@r

	mov	[store_text_area_end],edi

	xor	edi,edi
	ret
;-----------------------------------------------------------------------------
get_window_param:
	mcall	9, Proc_Info, -1	; window redraw requested so get
					; new window coordinates and size
	ret
;-----------------------------------------------------------------------------
draw_window:
	call	prepare_text_area

	mov	byte [total], 0
	mcall	12, 1			; start of draw
	; DRAW WINDOW
	mcall	0,dword [Form],dword [Form + 4],0x13ffffff,0x805080d0,title

	call	get_window_param

	mov	eax,[Proc_Info+70] ;status of window
	test	eax,100b
	jne	.end

	mcall	8,<450,100>,<25,25>,2,0xC0C0C0
	shr	ecx,16
	mov	bx,cx
	add	ebx,13 shl 16+4
	mcall	4,,0x80000000,text_save_button
	add	bx,11
	mcall	,,,text_save_button.1
	; Insert horizontal bars  in list area
	mov	eax, 13 		; draw bar system function
	mov	ebx, 18 		; set Xstart position of bar
	shl	ebx, 16 		;
	mov	bx, word [Form] ; get width of window
	sub	bx, 32			; bar is 32 pixels shorter then window width
	mov	ecx, 119 * 65536 + 10	; set Ystart(109) and Height(10) of bar   109
	mov	edx, 0xC0C0C0		; set color of bar
.again:	;begin draw bar loop
	mcall				; draw bar to window area
	shr	ecx, 16 		; move the Ystart position to working area
	add	ecx, 34 		; add 34 pixels to Y Start (moves bar down)
	cmp	cx, word [Form + 4]	; is the Ystart position outside of window area
	jae	.nomo			; if so stop drawing bars
	sub	ecx, 14 		; if not, we only need 20 pixels between bar tops
	shl	ecx, 16 		; set that values as Ystart
	add	ecx, 10 		; Bar Height is always 10 pixels
	jmp	.again			; draw another bar
;-----------------------------------------------------------------------------
.nomo:					;done drawing bars here
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
@@:
	movzx	esi, byte[edx]
	inc	edx
	mcall
	add	ebx, 10
	add	edx, esi
	cmp	byte[edx], -1
	jne	@b
	; Quantity of devices...
	movzx	ecx, byte [total]	; number to draw
	mcall	47, 0x00020000,,150 * 65536 + 65, 0x224466

	mov	ebx,ecx
	mov	ecx,2
	mov	edi,[store_text_area_start]
	add	edi,157
	push	edi
	call	binary_to_hex_string
	pop	edi
	mov	[edi+2],byte 'h'

	mov	ah, [MMIO_allowed]
	or 	ah, ah
	jz 	@f
	mov	ah, [MMIO_Bus]	; =255 if MMIO disabled / not found
	and	ah, 0x7f
	inc	ah
	jo	@f
	call	Try_MMIO
@@:
.end:
	mcall	12, 2			; end of draw
	ret
;-----------------------------------------------------------------------------
store_data:
	mov	eax,[store_text_area_start]
	mov	[fileinfo.return],eax
	mov	ebx,[store_text_area_end]
	sub	ebx,eax
	inc	ebx
	mov	[fileinfo.size],ebx
	mcall	70,fileinfo
	ret
;-----------------------------------------------------------------------------
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
;-----------------------------------------------------------------------------
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
@@:
	mcall	62			; Read it

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
;-----------------------------------------------------------------------------
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

	call	store_4_digits

	and	edx, 0xFFFF		;*****************************************
	or	edx, 54 * 65536 ; X start becomes 54
	movzx	ecx, word [PCI_Device]	; get Vendor ID
	mcall				; Draw Vendor ID to Window

	call	store_4_digits

	and	edx, 0xFFFF		;*****************************************
	or	edx, 98 * 65536 ; X start becomes 98
	movzx	ecx, byte [V_Bus]	; get bus number
	mcall	,0x00020100		; draw bus number to screen

	call	store_2_digits

	and	edx, 0xFFFF		;*****************************************
	or	edx, 128 * 65536	; X start becomes 128
	movzx	ecx, byte [V_Dev]	; get device number
	shr	ecx, 3			; device number is bits 3-7
	mcall				; Draw device Number To Window

	call	store_2_digits

	and	edx, 0xFFFF		;*****************************************
	or	edx, 155 * 65536	; X start becomes 155
	movzx	ecx, byte [V_Dev]	; get Function number
	and	ecx, 7			; function is first 3 bits
	mcall				; Draw Function Number To Window

	call	store_2_digits

	and	edx, 0xFFFF		;*****************************************
	or	edx, 179 * 65536	; X start becomes 179
	movzx	ecx, byte [PCI_Rev]	; get revision number
	mcall				; Draw Revision to screen

	call	store_2_digits

	and	edx, 0xFFFF		;*****************************************
	or	edx, 215*65536		; X start becomes 215
	movzx	ecx, byte [PCI_Class]	; get PCI_Class
	mcall				; Draw Class to screen

	call	store_2_digits

	and	edx, 0xFFFF		;*****************************************
	or	edx, 250*65536		; X start becomes 250
	movzx	ecx, byte [PCI_SubClass]; get sub class
	mcall				; Draw Sub Class to screen

	call	store_2_digits

; from Mario79 august 2006
	and	edx, 0xFFFF		;*****************************************
	or	edx, 280 * 65536	; X start becomes 280
	movzx	ecx, [PCI_Interface]	; get Interface
	mcall

	call	store_2_digits

;
; from Ghost april 2007                 ;*****************************************
	and	edx, 0xFFFF
	or	edx, 310 * 65536	; X start becomes 310
	movzx	ecx, [PCI_IRQ]		; get Interface
	cmp	cl, 23   		; IRQ between 0..23
	ja	@f

	mcall

	call	store_2_digits
	jmp	.PCI_Vendor
@@:
	call	store_NA
.PCI_Vendor:
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

.fn:
	mov	ax, [edx]
	add	edx, 6
	test	ax, ax
	jz	.find

	cmp	ax, cx
	jne	.fn

.find:
	mov	edx, [edx - 4]
	mcall	4,, 0x80000000		; lets print the vendor Name

	mov	[store_text_size],42
	call	store_text
;------------------------------------------------------------------
; Get description based on Class/Subclass
;
; Modified on ??-04-2007 by Ghost for size
;------------------------------------------------------------------
	mov	eax, dword [PCI_Class]
	and	eax, 0xffffff
	xor	edx, edx
	xor	esi, esi
.fnc:
	inc	esi
	mov	ecx, [Classes + esi * 8 - 8]
	cmp	cx, 0xffff
	je	.endfc

	cmp	cx, ax
	jne	.fnc

	test	ecx, 0xff000000
	jz	@f

	mov	edx, [Classes + esi * 8 - 4]
	jmp	.fnc
@@:
	cmp	eax, ecx
	jne	.fnc

	xor	edx, edx
.endfc:
	test	edx, edx
	jnz	@f

	mov	edx, [Classes + esi * 8 - 4]
@@:
	and	ebx, 0x0000FFFF 	; clear X position
	or	ebx, 0x24E0000		; set X position to 590 pixels
	mcall	4,, 0x80000000,, 32	; draw the text

	mov	[store_text_size],0
	call	store_text
	call	store_CR

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
;-----------------------------------------------------------------------------
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
;-----------------------------------------------------------------------------
@@:
	xor	bh,bh
	mov	[MMIO_BAR], bh
	ret
;-----------------------------------------------------------------------------
store_CR:
	pusha
	mov	edi,[store_text_area_end]
	mov	[edi],word 0A20h ; CR (carriage return) + SPACE
	add	dword [store_text_area_end],2
	popa
	ret
;-----------------------------------------------------------------------------
store_text:
	pusha
	inc	dword [store_text_area_end]
	mov	esi,edx
	mov	edi,[store_text_area_end]
	push	edi
	xor	eax,eax
	cld
@@:
	lodsb
	test	eax,eax
	jz	@f
	stosb
	inc	dword [store_text_area_end]
	jmp	@r
@@:
	pop	esi
	mov	eax,[store_text_size]
	test	eax,eax
	jz	@f
	sub	edi,esi
	sub	eax,edi
	add	[store_text_area_end],eax
@@:
	popa
	ret
;-----------------------------------------------------------------------------
store_NA:
	pusha
	mov	ebx,edx
	mcall	4,,0x80000000,text_NA
	mov	edi,[store_text_area_end]
	mov	ax,[edx]
	mov	[edi+1],ax
	add	[store_text_area_end],dword 5
	popa
	ret
;-----------------------------------------------------------------------------
store_2_digits:
	pusha
	inc	[store_text_area_end]
	mov	ebx,ecx
	mov	ecx,2
	mov	edi,[store_text_area_end]
	call	binary_to_hex_string
	add	[store_text_area_end],dword 4
	popa
	ret
;-----------------------------------------------------------------------------
store_4_digits:
	pusha
	mov	ebx,ecx
	mov	ecx,4
	mov	edi,[store_text_area_end]
	call	binary_to_hex_string
	add	[store_text_area_end],dword 6
	popa
	ret
;-----------------------------------------------------------------------------
; ebx - value
; ecx - digits
; edi - output string
binary_to_hex_string:
	add	edi,ecx
	dec	edi
	std
.1:
	mov	al,bl
	and	al,0xf
	shr	ebx,4
	cmp	al,9
	jbe	@f

	add	al,0x27
@@:
	add	al,0x30
	stosb
	dec	ecx
	jnz	.1
	cld
	ret
;-----------------------------------------------------------------------------
include 'vendors.inc'
;-----------------------------------------------------------------------------
; DATA AREA
DATA

Form:	dw 800 ; window width (no more, special for 800x600)
	dw 100 ; window x start
	dw 620 ; window height
	dw 20 ; window y start

title	db 'PCI Device Enumerator v 2.3', 0

if lang eq it
PCIWin mls \
	'   Don`t forget to enable PCI Access to Applications in Setup Menu.',\
	'',\
	'Versione PCI = x.xx; Ultimo Bus PCI = x',\
	'User MMIO channel = 0F.F:F ',\
	'Numbero di unità PCI =    ',\
	'',\
	'VenID DevID Bus# Dev# Fnc Rev  Class  Subclass/ IRQ                 Compania                    Descrizzione',\
	'                                      Interfaccia',\
	'----- ----- ---- ---- --- ---  -----  --------- --- ------------------------------------------ --------------------------------'

bar_ram db 'BARx: MMIO block', 0
bar_io	db 'BARx: porte IO',0
bar_um	db 'BARx: unmapped',0
bar_rom db 'BAR6: Expansion ROM', 0

text_save_button:
	db 'Salva lista PCI',0
.1:	db '(Premere S)',0
else
PCIWin mls \
	'   Don`t forget to enable PCI Access to Applications in Setup Menu.',\
	'',\
	'PCI Version  = x.xx; Last PCI Bus = x',\
	'User MMIO channel = 0F.F:F ',\
	'Number of PCI units =    ',\
	'',\
	'VenID DevID Bus# Dev# Fnc Rev  Class  Subclass/ IRQ                 Company                      Description',\
	'                                      Interface',\
	'----- ----- ---- ---- --- ---  -----  --------- --- ------------------------------------------ --------------------------------'

bar_ram db 'BARx: MMIO block', 0
bar_io	db 'BARx: IO ports',0
bar_um	db 'BARx: unmapped',0
bar_rom db 'BAR6: Expansion ROM', 0

text_save_button:
	db 'Save PCI list',0
.1:	db '(Press S key)',0
end if

text_NA:
	db '--',0
;---------------------------------------------------------------------
system_dir_ProcLib	db '/sys/lib/proc_lib.obj',0

err_message_found_lib2	db 'proc_lib.obj - Not found!',0

err_message_import2	db 'proc_lib.obj - Wrong import!',0

head_f_i:
head_f_l	db 'error',0
;---------------------------------------------------------------------
l_libs_start:

library02  l_libs system_dir_ProcLib+9, path, library_path, system_dir_ProcLib, \
err_message_found_lib2, head_f_l, ProcLib_import, err_message_import2, head_f_i

end_l_libs:
;---------------------------------------------------------------------
align 4
ProcLib_import:
OpenDialog_Init		dd aOpenDialog_Init
OpenDialog_Start	dd aOpenDialog_Start
;OpenDialog__Version	dd aOpenDialog_Version
        dd      0
        dd      0
aOpenDialog_Init	db 'OpenDialog_init',0
aOpenDialog_Start	db 'OpenDialog_start',0
;aOpenDialog_Version	db 'Version_OpenDialog',0
;---------------------------------------------------------------------
align 4
OpenDialog_data:
.type			dd 0
.procinfo		dd Proc_Info	;+4
.com_area_name		dd communication_area_name	;+8
.com_area		dd 0	;+12
.opendir_pach		dd temp_dir_pach	;+16
.dir_default_pach	dd communication_area_default_pach	;+20
.start_path		dd open_dialog_path	;+24
.draw_window		dd draw_window	;+28
.status			dd 0	;+32
.openfile_pach 		dd file_name	;+36
.filename_area		dd filename_area	;+40
.filter_area		dd Filter
.x:
.x_size			dw 420 ;+48 ; Window X size
.x_start		dw 10 ;+50 ; Window X position
.y:
.y_size			dw 320 ;+52 ; Window y size
.y_start		dw 10 ;+54 ; Window Y position

communication_area_name:
	db 'FFFFFFFF_open_dialog',0
open_dialog_path:
	db '/sys/File Managers/opendial',0
communication_area_default_pach:
	db '/sys',0
Filter:
dd	Filter.end - Filter.1
.1:
db	'TXT',0
db	'LOG',0
.end:
dd	0

file_default_path:
	db '/sys/'
start_temp_file_name:
	db 'pcidev.txt',0
;---------------------------------------------------------------------
align	4
fileinfo:
.subfunction	dd 2
.Offset		dd 0
.Offset_1	dd 0
.size		dd 4096
.return		dd 0
		db 0
.name:		dd file_name
;-----------------------------------------------------------------------------

; UNINITIALIZED DATA AREA
IM_END:
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

store_text_area_start	dd ?
store_text_area_end	dd ?
store_text_size		dd ?

extended_key 	rb 1
;---------------------------------------------------------------------
library_path:
	rb 4096
;---------------------------------------------------------------------
path:
	rb 4096
;---------------------------------------------------------------------
temp_dir_pach:
	rb 4096
;---------------------------------------------------------------------
file_name:
	rb 4096
;---------------------------------------------------------------------
file_name_1:
	rb 4096
;---------------------------------------------------------------------
filename_area:
	rb 256
;---------------------------------------------------------------------
	rb 4096
stacktop:
;---------------------------------------------------------------------
Proc_Info	process_information
;---------------------------------------------------------------------
I_END:
;-----------------------------------------------------------------------------
