$Revision: 1598 $

SMBUS_PCIE_ADDR		equ	0xF00A0000    	; bdf0:20.0  = SB7xx SMBus PCI Config Registers
LPC_PCIE_ADDR		equ	0xF00A3000    	; bdf0:20.3  = SB7xx LPC ISA bridge Config Registers

SB_SIO_INDEX	equ	0x2e

;---------------------------------------------------------------------
align 4
smbus_read_pciconfig:
; in:  dl = reg#  |  out: eax = data
	mov	ebx, SMBUS_PCIE_ADDR
	and	edx, 0x0FC
	mov	eax, dword [ebx+edx]
	ret
;------------------------------------------------
align 4
smbus_write_pciconfig:
; in: dl = reg#; eax = data
	mov	ebx, SMBUS_PCIE_ADDR
	and	edx, 0x0FC
	mov	dword [ebx+edx], eax
	ret

;--------------------------------------------------------------------
align 4
lpc_read_pciconfig:
; in:  dl = reg#  |  out: eax = data
	mov	ebx, LPC_PCIE_ADDR
	and	edx, 0x0FC
	mov	eax, dword [ebx+edx]
	ret
;------------------------------------------------
align 4
lpc_write_pciconfig:
; in: dl = reg#; eax = data
	mov	ebx, LPC_PCIE_ADDR
	and	edx, 0x0FC
	mov	dword [ebx+edx], eax
	ret

;--------------------------------------------------------------------
align 4
read_sio_cfg:
; in:  al = reg#  |  out: al =  data
	mov	dx,  SB_SIO_INDEX
	out	dx, al
	inc	dl
	in	al, dx
	ret

;------------------------------------------------
align 4
write_sio_cfg:
; in:  al = reg#; ah = data
;------------------------------------------------
	mov	dx,  SB_SIO_INDEX
	out	dx, al
	inc	dl
	xchg	al, ah
	out	dx, al
	xchg	al, ah
	ret
;------------------------------------------------
align 4
enter_sio_cfg_mode:
; the magic sequence to unlock the port
;------------------------------------------------
	mov	dx, SB_SIO_INDEX
	mov	eax, 0x55550187 ; low byte first
	out	dx, al
	shr	eax, 8
	out	dx, al
	shr	eax, 8
	out	dx, al
	shr	eax, 8
	out	dx, al
	ret

;-----------------------------------------------------------------------
; ATTENTION: the functions assume that RESET# signals use pins 84 and 34 
; of IT8712F SuperIO chip. These signals may be (and will be!) different 
; for every particular motherboard and SIO. Please refer to your m/board
; documentation to define the correct pins and GPIO lines! 
;
;   Note this example DOES NOT PRETEND to be 100% correct implementation 
;   of PCIe hotplug techniques !!
;-----------------------------------------------------------------------
align 4
init_pcie_slot_control:
;------------------------------------------------
	call	enter_sio_cfg_mode
	mov	ax, 0x0707	; LDN = 07
	call	write_sio_cfg
	mov	al, 0x25
	call	read_sio_cfg	; ah = reg25h (Multy-function pin selector)
	or	ah, 3		; set bits 0, 1 (GPIO)
	call	write_sio_cfg
	mov	al, 0x2A
	call	read_sio_cfg	; ah = reg2Ah (Extended fn pin selector)
	or	ah, 3		; set bits 0, 1 (GPIO)
	call	write_sio_cfg
	mov	al, 0xB8
	call	read_sio_cfg	; ah = regB8h (internal pull-up enable)
	or	ah, 3		; set bits 0, 1
	call	write_sio_cfg
	mov	al, 0xC0
	call	read_sio_cfg	; ah = regC0h (simple IO enable)
	or	ah, 3		; set bits 0, 1
	call	write_sio_cfg
	mov	ax, 0x0202	; Lock SIO config ports
	call	write_sio_cfg
	ret
 
align 4
reset_pcie_slot:
;------------------------------------------------
	call	enter_sio_cfg_mode
	mov	ax, 0x0707	; LDN = 07
	call	write_sio_cfg
	mov	al, 0xB0
	call	read_sio_cfg	; ah = regB0h (Pin polarity)
	and	ah, 0xFC	; invert bits 0, 1
	call	write_sio_cfg
	or	ah, 3		; restore bits 0, 1 
	call	write_sio_cfg
	mov	ax, 0x0202	; Lock SIO config ports
	call	write_sio_cfg
	ret


