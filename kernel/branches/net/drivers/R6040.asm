;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  R6040 driver for KolibriOS                                     ;;
;;                                                                 ;;
;;  based on R6040.c from linux                                    ;;
;;                                                                 ;;
;;    Written by Asper (asper.85@mail.ru)                          ;;
;;            and hidnplayr (hidnplayr@gmail.com)                  ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format MS COFF

	API_VERSION		equ 0x01000100
	DRIVER_VERSION		equ 5

	MAX_DEVICES		equ 16

	DEBUG			equ 1
	__DEBUG__		equ 1
	__DEBUG_LEVEL__ 	equ 1

	TX_RING_SIZE		equ 4
	RX_RING_SIZE		equ 4


include 'proc32.inc'
include 'imports.inc'
include 'fdo.inc'
include 'netdrv.inc'

public START
public service_proc
public version


; ethernet address length
ETH_ALEN	equ 6
ETH_HLEN	equ (2 * ETH_ALEN + 2)
ETH_ZLEN	equ 60	    ; 60 + 4bytes auto payload for mininmum 64bytes frame length

; system timer frequency
HZ		equ 1000

; max time out delay time
W_MAX_TIMEOUT	equ 0x0FFF

;; Size of the in-memory receive ring.
RX_BUF_LEN_IDX	equ 3	    ;; 0==8K, 1==16K, 2==32K, 3==64K
RX_BUF_LEN	equ (8192 << RX_BUF_LEN_IDX)

;; PCI Tuning Parameters
;   Threshold is bytes transferred to chip before transmission starts.
TX_FIFO_THRESH	equ 256     ;; In bytes, rounded down to 32 byte units.

;; The following settings are log_2(bytes)-4:  0 == 16 bytes .. 6==1024.
RX_FIFO_THRESH	equ 4	    ;; Rx buffer level before first PCI xfer.
RX_DMA_BURST	equ 4	    ;; Maximum PCI burst, '4' is 256 bytes
TX_DMA_BURST	equ 4

;; Operational parameters that usually are not changed.
PHY1_ADDR	equ 1	    ;For MAC1
PHY2_ADDR	equ 3	    ;For MAC2
PHY_MODE	equ 0x3100  ;PHY CHIP Register 0
PHY_CAP 	equ 0x01E1  ;PHY CHIP Register 4

;; Time in jiffies before concluding the transmitter is hung.
TX_TIMEOUT	equ ((6000*HZ)/1000)

IO_SIZE   equ 256     ; RDC MAC I/O Size
MAX_MAC 	equ 2	    ; MAX RDC MAC

;**************************************************************************
; RDC R6040 Register Definitions
;**************************************************************************
MCR0		equ 0x00    ;Control register 0
MCR1		equ 0x01    ;Control register 1
MAC_RST 	equ 0x0001  ;Reset the MAC
MBCR		equ 0x08    ;Bus control
MT_ICR		equ 0x0C    ;TX interrupt control
MR_ICR		equ 0x10    ;RX interrupt control
MTPR		equ 0x14    ;TX poll command register
MR_BSR		equ 0x18    ;RX buffer size
MR_DCR		equ 0x1A    ;RX descriptor control
MLSR		equ 0x1C    ;Last status
MMDIO		equ 0x20    ;MDIO control register
MDIO_WRITE	equ 0x4000  ;MDIO write
MDIO_READ	equ 0x2000  ;MDIO read
MMRD		equ 0x24    ;MDIO read data register
MMWD		equ 0x28    ;MDIO write data register
MTD_SA0 	equ 0x2C    ;TX descriptor start address 0
MTD_SA1 	equ 0x30    ;TX descriptor start address 1
MRD_SA0 	equ 0x34    ;RX descriptor start address 0
MRD_SA1 	equ 0x38    ;RX descriptor start address 1
MISR		equ 0x3C    ;Status register
MIER		equ 0x40    ;INT enable register
MSK_INT 	equ 0x0000  ;Mask off interrupts
RX_FINISH	equ 0x0001  ;RX finished
RX_NO_DESC	equ 0x0002  ;No RX descriptor available
RX_FIFO_FULL	equ 0x0004  ;RX FIFO full
RX_EARLY	equ 0x0008  ;RX early
TX_FINISH	equ 0x0010  ;TX finished
TX_EARLY	equ 0x0080  ;TX early
EVENT_OVRFL	equ 0x0100  ;Event counter overflow
LINK_CHANGED	equ 0x0200  ;PHY link changed
ME_CISR 	equ 0x44    ;Event counter INT status
ME_CIER 	equ 0x48    ;Event counter INT enable
MR_CNT		equ 0x50    ;Successfully received packet counter
ME_CNT0 	equ 0x52    ;Event counter 0
ME_CNT1 	equ 0x54    ;Event counter 1
ME_CNT2 	equ 0x56    ;Event counter 2
ME_CNT3 	equ 0x58    ;Event counter 3
MT_CNT		equ 0x5A    ;Successfully transmit packet counter
ME_CNT4 	equ 0x5C    ;Event counter 4
MP_CNT		equ 0x5E    ;Pause frame counter register
MAR0		equ 0x60    ;Hash table 0
MAR1		equ 0x62    ;Hash table 1
MAR2		equ 0x64    ;Hash table 2
MAR3		equ 0x66    ;Hash table 3
MID_0L		equ 0x68    ;Multicast address MID0 Low
MID_0M		equ 0x6A    ;Multicast address MID0 Medium
MID_0H		equ 0x6C    ;Multicast address MID0 High
MID_1L		equ 0x70    ;MID1 Low
MID_1M		equ 0x72    ;MID1 Medium
MID_1H		equ 0x74    ;MID1 High
MID_2L		equ 0x78    ;MID2 Low
MID_2M		equ 0x7A    ;MID2 Medium
MID_2H		equ 0x7C    ;MID2 High
MID_3L		equ 0x80    ;MID3 Low
MID_3M		equ 0x82    ;MID3 Medium
MID_3H		equ 0x84    ;MID3 High
PHY_CC		equ 0x88    ;PHY status change configuration register
PHY_ST		equ 0x8A    ;PHY status register
MAC_SM		equ 0xAC    ;MAC status machine
MAC_ID		equ 0xBE    ;Identifier register
;?[
;TX_DCNT         equ 0x80    ;TX descriptor count
;RX_DCNT         equ 0x80    ;RX descriptor count
MAX_BUF_SIZE	equ 0x600   ;1536

MBCR_DEFAULT	equ 0x012A  ;MAC Bus Control Register
MCAST_MAX	equ 3	    ;Max number multicast addresses to filter

;Descriptor status
DSC_OWNER_MAC	equ 0x8000  ;MAC is the owner of this descriptor
DSC_RX_OK	equ 0x4000  ;RX was successfull
DSC_RX_ERR	equ 0x0800  ;RX PHY error
DSC_RX_ERR_DRI	equ 0x0400  ;RX dribble packet
DSC_RX_ERR_BUF	equ 0x0200  ;RX length exceeds buffer size
DSC_RX_ERR_LONG equ 0x0100  ;RX length > maximum packet length
DSC_RX_ERR_RUNT equ 0x0080  ;RX packet length < 64 byte
DSC_RX_ERR_CRC	equ 0x0040  ;RX CRC error
DSC_RX_BCAST	equ 0x0020  ;RX broadcast (no error)
DSC_RX_MCAST	equ 0x0010  ;RX multicast (no error)
DSC_RX_MCH_HIT	equ 0x0008  ;RX multicast hit in hash table (no error)
DSC_RX_MIDH_HIT equ 0x0004  ;RX MID table hit (no error)
DSC_RX_IDX_MID_MASK  equ 3  ;RX mask for the index of matched MIDx

;PHY settings
ICPLUS_PHY_ID	equ 0x0243

RX_INTS 	equ RX_FIFO_FULL or RX_NO_DESC or RX_FINISH
TX_INTS 	equ TX_FINISH
INT_MASK	equ RX_INTS or TX_INTS


virtual at 0
x_head:
  .status	  dw ?	 ;0-1
  .len		  dw ?	 ;2-3
  .buf		  dd ?	 ;4-7
  .ndesc	  dd ?	 ;8-B
  .rev1 	  dd ?	 ;C-F
  .vbufp	  dd ?	 ;10-13
  .vndescp	  dd ?	 ;14-17
  .skb_ptr	  dd ?	 ;18-1B
  .rev2 	  dd ?	 ;1C-1F
  .sizeof:
end virtual


virtual at ebx

	device:

	ETH_DEVICE

	.io_addr	dd ?

	.cur_rx 	dw ?
	.cur_tx 	dw ?
	.last_tx	dw ?
	.phy_addr	dw ?
	.phy_mode	dw ?
	.mcr0		dw ?
	.mcr1		dw ?
	.switch_sig	dw ?

	.pci_bus	db ?
	.pci_dev	db ?
	.irq_line	db ?

	rb 1		; dword alignment

	.tx_ring:	rb (((x_head.sizeof*TX_RING_SIZE)+32) and 0xfffffff0)
	.rx_ring:	rb (((x_head.sizeof*RX_RING_SIZE)+32) and 0xfffffff0)

	.size = $ - device

end virtual



section '.flat' code readable align 16

;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                        ;;
;; proc START             ;;
;;                        ;;
;; (standard driver proc) ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
proc START stdcall, state:dword

	cmp [state], 1
	jne .exit

  .entry:

	DEBUGF	2,"Loading R6040 driver\n"
	stdcall RegService, my_service, service_proc
	ret

  .fail:
  .exit:
	xor eax, eax
	ret

endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                        ;;
;; proc SERVICE_PROC      ;;
;;                        ;;
;; (standard driver proc) ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
proc service_proc stdcall, ioctl:dword

	mov	edx, [ioctl]
	mov	eax, [IOCTL.io_code]

;------------------------------------------------------

	cmp	eax, 0 ;SRV_GETVERSION
	jne	@F

	cmp	[IOCTL.out_size], 4
	jl	.fail
	mov	eax, [IOCTL.output]
	mov	[eax], dword API_VERSION

	xor	eax, eax
	ret

;------------------------------------------------------
  @@:
	cmp	eax, 1 ;SRV_HOOK
	jne	.fail

	cmp	[IOCTL.inp_size], 3			; Data input must be at least 3 bytes
	jl	.fail

	mov	eax, [IOCTL.input]
	cmp	byte [eax], 1				; 1 means device number and bus number (pci) are given
	jne	.fail					; other types arent supported for this card yet

; check if the device is already listed

	mov	esi, device_list
	mov	ecx, [devices]
	test	ecx, ecx
	jz	.firstdevice

;        mov     eax, [IOCTL.input]                      ; get the pci bus and device numbers
	mov	ax , [eax+1]				;
  .nextdevice:
	mov	ebx, [esi]
	cmp	ax , word [device.pci_bus]		; compare with pci and device num in device list (notice the usage of word instead of byte)
	je	.find_devicenum 			; Device is already loaded, let's find it's device number
	add	esi, 4
	loop	.nextdevice


; This device doesnt have its own eth_device structure yet, lets create one
  .firstdevice:
	cmp	[devices], MAX_DEVICES			; First check if the driver can handle one more card
	jge	.fail

	allocate_and_clear ebx, device.size, .fail	; Allocate the buffer for device structure

; Fill in the direct call addresses into the struct

	mov	[device.reset], reset
	mov	[device.transmit], transmit
	mov	[device.get_MAC], read_mac
	mov	[device.set_MAC], .fail
	mov	[device.unload], unload
	mov	[device.name], my_service

; save the pci bus and device numbers

	mov	eax, [IOCTL.input]
	mov	cl , [eax+1]
	mov	[device.pci_bus], cl
	mov	cl , [eax+2]
	mov	[device.pci_dev], cl

; Now, it's time to find the base io addres of the PCI device

	find_io [device.pci_bus], [device.pci_dev], [device.io_addr]

; We've found the io address, find IRQ now

	find_irq [device.pci_bus], [device.pci_dev], [device.irq_line]

	DEBUGF	2,"Hooking into device, dev:%x, bus:%x, irq:%x, addr:%x\n",\
	[device.pci_dev]:1,[device.pci_bus]:1,[device.irq_line]:1,[device.io_addr]:4

; Ok, the eth_device structure is ready, let's probe the device
	cli

	call	probe							; this function will output in eax
	test	eax, eax
	jnz	.err_sti						; If an error occured, exit

	mov	eax, [devices]						; Add the device structure to our device list
	mov	[device_list+4*eax], ebx				; (IRQ handler uses this list to find device)
	inc	[devices]						;

	mov	[device.type], NET_TYPE_ETH
	call	NetRegDev
	sti

	cmp	eax, -1
	je	.destroy

	ret

; If the device was already loaded, find the device number and return it in eax

  .find_devicenum:
	DEBUGF	2,"Trying to find device number of already registered device\n"
	call	NetPtrToNum						; This kernel procedure converts a pointer to device struct in ebx
									; into a device number in edi
	mov	eax, edi						; Application wants it in eax instead
	DEBUGF	2,"Kernel says: %u\n", eax
	ret

; If an error occured, remove all allocated data and exit (returning -1 in eax)

  .destroy:
	; todo: reset device into virgin state

  .err_sti:
	sti

  .err:
	stdcall KernelFree, ebx

  .fail:
	or	eax, -1
	ret

;------------------------------------------------------
endp


;;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\;;
;;                                                                        ;;
;;        Actual Hardware dependent code starts here                      ;;
;;                                                                        ;;
;;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\;;


macro r6040_mdio_write reg, val {
	stdcall r6040_phy_read, [io_addr], [r6040_private.phy_addr], reg
}

macro r6040_mdio_write reg, val {
	stdcall r6040_phy_write, [io_addr], [r6040_private.phy_addr], reg, val
}


align 4
unload:
	; TODO: (in this particular order)
	;
	; - Stop the device
	; - Detach int handler
	; - Remove device from local list (RTL8139_LIST)
	; - call unregister function in kernel
	; - Remove all allocated structures and buffers the card used

	or	eax,-1

ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  probe: enables the device (if it really is RTL8139)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
probe:
	DEBUGF	2,"Probing R6040 device\n"

	make_bus_master [device.pci_bus], [device.pci_dev]

	; If PHY status change register is still set to zero
	; it means the bootloader didn't initialize it

	set_io	0
	set_io	PHY_CC
	in	ax, dx
	test	ax, ax
	jnz	@f
	mov	ax, 0x9F07
	out	dx, ax
     @@:

	call	read_mac

	; Some bootloaders/BIOSes do not initialize MAC address, warn about that
	and	eax, 0xFF
	or	eax, dword [device.mac]
	test	eax, eax
	jnz	@f
	DEBUGF	1, "MAC address not initialized\n" ;, generating random"

	;Asper: Add here generate function call!
	;       Temporary workaround: init by constant adress

	mov	dword [device.mac], 0x00006000
	mov	word [device.mac+4], 0x0001
     @@:

	; Init RDC private data
	mov	[device.mcr0], 0x1002
	;mov     [private.phy_addr], 1 ; Asper: Only one network card is supported now.
	mov	[device.switch_sig], 0

	; Check the vendor ID on the PHY, if 0xFFFF assume none attached
	stdcall phy_read, 1, 2
	cmp	ax, 0xFFFF
	jne	@f
	DEBUGF	1, "Failed to detect an attached PHY\n" ;, generating random"
	mov	eax, -1
	ret
     @@:

	; Set MAC address
	call	init_mac_regs

	; Initialize and alloc RX/TX buffers
	call	init_txbufs
	call	init_rxbufs

	; Read the PHY ID
	mov	[device.phy_mode], 0x8000
	stdcall phy_read, 0, 2
	mov	[device.switch_sig], ax
	cmp	ax, ICPLUS_PHY_ID
	jne	@f
	stdcall phy_write, 29, 31, 0x175C ; Enable registers
	jmp	.phy_readen
      @@:

	; PHY Mode Check
	movzx	eax, [device.phy_addr]
	stdcall phy_write, eax, 4, PHY_CAP
	stdcall phy_write, eax, 0, PHY_MODE

;      if PHY_MODE = 0x3100
	call	phy_mode_chk
	mov	[device.phy_mode], ax
	jmp	.phy_readen
;      end if

;      if not (PHY_MODE and 0x0100)
	mov	[device.phy_mode], 0
;      end if
      .phy_readen:

	; Set duplex mode
	mov	ax, [device.phy_mode]
	or	[device.mcr0], ax

	; improve performance (by RDC guys)
	stdcall phy_read, 30, 17
	or	ax, 0x4000
	stdcall phy_write, 30, 17, eax

	stdcall phy_read, 30, 17
	and	ax, not 0x2000
	stdcall phy_write, 30, 17, eax

	stdcall phy_write, 0, 19, 0x0000
	stdcall phy_write, 0, 30, 0x01F0

	; Initialize all Mac registers
	call	init_mac_regs



align 4
reset:

	DEBUGF	1,"Resetting R6040\n"

	; Mask off Interrupt
	xor	ax, ax
	set_io	0
	set_io	MIER
	out	dx, ax


; attach int handler

	movzx	eax, [device.irq_line]
	DEBUGF	1,"Attaching int handler to irq %x\n", eax:1
	stdcall AttachIntHandler, eax, int_handler, dword 0
	test	eax, eax
	jnz	@f
	DEBUGF	1,"\nCould not attach int handler!\n"
;        or      eax, -1
;        ret
       @@:


	;Reset RDC MAC
	mov	eax, MAC_RST
	set_io	0
	set_io	MCR1
	out	dx, ax

	mov	ecx, 2048 ;limit
  .read:
	in	ax, dx
	test	ax, 0x1
	jnz	 @f
	dec	ecx
	test	ecx, ecx
	jnz	.read
  @@:
	;Reset internal state machine
	mov	ax,  2
	set_io	MAC_SM
	out	dx, ax

	xor	ax, ax
	out	dx, ax

	mov	esi, 5
	stdcall Sleep

	;MAC Bus Control Register
	mov	ax, MBCR_DEFAULT
	set_io	0
	set_io	MBCR
	out	dx, ax

	;Buffer Size Register
	mov	ax, MAX_BUF_SIZE
	set_io	MR_BSR
	out	dx, ax

	;Write TX ring start address
	lea	eax, [device.tx_ring]
	GetRealAddr
	set_io	MTD_SA0
	out	dx, ax
	shr	eax, 16
	set_io	MTD_SA1
	out	dx, ax

	;Write RX ring start address
	lea	eax, [device.rx_ring]
	GetRealAddr
	set_io	MRD_SA0
	out	dx, ax
	shr	eax, 16
	set_io	MRD_SA1
	out	dx, ax

	;Set interrupt waiting time and packet numbers
	xor	ax, ax
	set_io	MT_ICR
	out	dx, ax

	;Enable interrupts
	mov	ax, INT_MASK
	set_io	MIER
	out	dx, ax

	;Enable TX and RX
	mov	ax, [device.mcr0]
	or	ax, 0x0002
	set_io	0
	out	dx, ax

	;Let TX poll the descriptors
	;we may got called by tx_timeout which has left
	;some unset tx buffers
	xor	ax, ax
	inc	ax
	set_io	0
	set_io	MTPR
	out	dx, ax

; Set the mtu, kernel will be able to send now
	mov	[device.mtu], 1514

	DEBUGF	1,"Reset ok\n"
	xor	eax, eax

	ret



align 4
init_txbufs:

	DEBUGF	1,"Init TxBufs\n"

	lea	esi, [device.tx_ring]
	lea	eax, [device.tx_ring + x_head.sizeof]
	GetRealAddr
	mov	ecx, TX_RING_SIZE

    .next_desc:
	mov	[esi + x_head.ndesc], eax
	mov	[edi + x_head.skb_ptr], 0
	mov	[esi + x_head.status], DSC_OWNER_MAC

	add	eax, x_head.sizeof
	add	esi, x_head.sizeof

	dec	ecx
	jnz	.next_desc

	lea	eax, [device.tx_ring]
	GetRealAddr
	mov	[device.tx_ring + x_head.sizeof*(TX_RING_SIZE - 1) + x_head.ndesc], eax

	DEBUGF	1,"ok\n"

	ret



align 4
init_rxbufs:

	DEBUGF	1,"Init RxBufs\n"

	lea	esi, [device.rx_ring]
	lea	eax, [device.rx_ring + x_head.sizeof]
	GetRealAddr
	mov	edx, eax
	mov	ecx, RX_RING_SIZE

    .next_desc:
	DEBUGF	1,"esi=0x%x, edx=0x%x, ", esi, edx
	mov	[esi + x_head.ndesc], edx

	push	esi ecx
	stdcall KernelAlloc, dword 2000
	pop	ecx esi

	DEBUGF	1,"eax=0x%x, ", eax
	mov	[esi + x_head.skb_ptr], eax
	GetRealAddr
	DEBUGF	1,"eax=0x%x\n", eax
	mov	[esi + x_head.buf], eax
	mov	[esi + x_head.status], DSC_OWNER_MAC

	add	edx, x_head.sizeof
	add	esi, x_head.sizeof

	dec	ecx
	jnz	.next_desc

	; complete the ring by linking the last to the first

	lea	eax, [device.rx_ring]
	GetRealAddr
	mov	[device.rx_ring + x_head.sizeof*(RX_RING_SIZE - 1) + x_head.ndesc], eax

	DEBUGF	1,"ok\n"

	ret



align 4
phy_mode_chk:

	DEBUGF	1,"Checking PHY mode\n"

	; PHY Link Status Check
	movzx	eax, [device.phy_addr]
	stdcall phy_read, eax, 1
	test	eax, 0x4
	jz	.ret_0x8000

;        jnz     @f
;        mov     eax, 0x8000             ; Link Failed, full duplex
;  @@:

	; PHY Chip Auto-Negotiation Status
	movzx	eax, [device.phy_addr]
	stdcall phy_read, eax, 1
	test	eax, 0x0020
	jnz	.auto_nego

	; Force Mode
	movzx	eax, [device.phy_addr]
	stdcall phy_read, eax, 0
	test	eax, 0x100
	jnz	.ret_0x8000

  .auto_nego:
	; Auto Negotiation Mode
	movzx	eax, [device.phy_addr]
	stdcall phy_read, eax, 5
	mov	ecx, eax
	movzx	eax, [device.phy_addr]
	stdcall phy_read, eax, 4
	and	eax, ecx
	test	eax, 0x140
	jnz	.ret_0x8000

	xor	eax, eax
	ret

  .ret_0x8000:
	mov	eax, 0x8000
	ret





;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                         ;;
;; Transmit                                ;;
;;                                         ;;
;; In: buffer pointer in [esp+4]           ;;
;;     size of buffer in [esp+8]           ;;
;;     pointer to device structure in ebx  ;;
;;                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
align 4
transmit:
	DEBUGF	1,"\nTransmitting packet, buffer:%x, size:%u\n", [esp+4], [esp+8]
	mov	eax, [esp+4]
	DEBUGF	1,"To: %x-%x-%x-%x-%x-%x From: %x-%x-%x-%x-%x-%x Type:%x%x\n",\
	[eax+00]:2,[eax+01]:2,[eax+02]:2,[eax+03]:2,[eax+04]:2,[eax+05]:2,\
	[eax+06]:2,[eax+07]:2,[eax+08]:2,[eax+09]:2,[eax+10]:2,[eax+11]:2,\
	[eax+13]:2,[eax+12]:2

	cmp	dword [esp+8], 1514
	jg	.fail
	cmp	dword [esp+8], 60
	jl	.fail

	movzx	edi, [device.cur_tx]
	shl	edi, 5
	add	edi, ebx
	add	edi, device.tx_ring - ebx

	DEBUGF	1,"R6040: TX buffer status: 0x%x\n", [edi + x_head.status]:4

	test	[edi + x_head.status], 0x8000  ; check if buffer is available
	jnz	.wait_to_send

  .do_send:

	mov	eax, [esp+4]
	mov	[edi + x_head.skb_ptr], eax
	GetRealAddr
	mov	[edi + x_head.buf], eax
	mov	ecx, [esp+8]
	mov	[edi + x_head.len], cx
	mov	[edi + x_head.status], 0x8000

	; Trigger the MAC to check the TX descriptor
	mov	ax, 0x01
	set_io	0
	set_io	MTPR
	out	dx, ax

	inc	[device.cur_tx]
	and	[device.cur_tx], TX_RING_SIZE-1
	xor	eax, eax

	ret	8

  .wait_to_send:

;        mov     ecx, [timer_ticks]
;        add     ecx, 100
;     .l2:
;        test    [eax + x_head.status], 0x8000
;        jz      .do_send
;        mov     esi, 10
;        call    Sleep
;        cmp     ecx, [timer_ticks]
;        jl      .l2

	xor	eax, eax
	dec	eax
  .fail:
	DEBUGF	1,"Send timeout\n"
	ret	8





;;;;;;;;;;;;;;;;;;;;;;;
;;                   ;;
;; Interrupt handler ;;
;;                   ;;
;;;;;;;;;;;;;;;;;;;;;;;
align 4
int_handler:

	DEBUGF	1,"\nIRQ %x ", eax:2   ; no, you cant replace 'eax:2' with 'al', this must be a bug in FDO

; Find pointer of device wich made IRQ occur

	mov	esi, device_list
	mov	ecx, [devices]
	test	ecx, ecx
	jz	.fail
  .nextdevice:
	mov	ebx, dword [esi]

	; Find reason for IRQ

	set_io	0
	set_io	MISR
	in	ax, dx
	out	dx, ax		; send it back to ACK

	DEBUGF	1,"MISR=%x\n", eax:4

	; Check if we are interessed in some of the reasons

	test	ax, INT_MASK
	jnz	.got_it

	; If not, try next device

	add	esi, 4
	dec	ecx
	jnz	.nextdevice

  .fail:				; If no device was found, abort (The irq was probably for a device, not registered to this driver)
	ret

; At this point, test for all possible reasons, and handle accordingly

  .got_it:
	push ax

	test	word [esp], RX_FINISH
	jz	.no_RX

	push	ebx
  .more_RX:
	pop	ebx

	; Find the current RX descriptor

	movzx	edx, [device.cur_rx]
	shl	edx, 5
	lea	edx, [device.rx_ring + edx]

	; Check the descriptor status

	mov	cx, [edx + x_head.status]
	test	cx, DSC_OWNER_MAC
	jnz	.no_RX

	DEBUGF	1,"packet status=0x%x\n", cx

	test	cx, DSC_RX_ERR		; Global error status set
	jnz	.no_RX

	; Packet successfully received

	movzx	ecx, [edx + x_head.len]
	and	ecx, 0xFFF
	sub	ecx, 4			; Do not count the CRC

	; Push packet size and pointer, kernel will need it..

	push	ebx
	push	.more_RX

	push	ecx
	push	[edx + x_head.skb_ptr]

	DEBUGF	1,"packet ptr=0x%x\n", [edx + x_head.skb_ptr]

	; reset the RX descriptor

	push	edx
	stdcall KernelAlloc, 2000
	pop	edx
	mov	[edx + x_head.skb_ptr], eax
	GetRealAddr
	mov	[edx + x_head.buf], eax
	mov	[edx + x_head.status], DSC_OWNER_MAC

	; Use next descriptor next time

	inc	[device.cur_rx]
	and	[device.cur_rx], RX_RING_SIZE - 1

	; At last, send packet to kernel

	jmp	EthReceiver


  .no_RX:

	test	word [esp], TX_FINISH
	jz	.no_TX

      .loop_tx:
	movzx	edi, [device.last_tx]
	shl	edi, 5
	lea	edi, [device.tx_ring + edi]

	test	[edi + x_head.status], DSC_OWNER_MAC
	jnz	.tx_loop_end

	cmp	[edi + x_head.skb_ptr], 0
	je	 .tx_loop_end

	DEBUGF	1,"Freeing buffer 0x%x\n", [edi + x_head.skb_ptr]

	push	[edi + x_head.skb_ptr]
	call	KernelFree
	mov	[edi + x_head.skb_ptr], 0

	inc	[device.last_tx]
	and	[device.last_tx], TX_RING_SIZE - 1
	jmp	.loop_tx

      .tx_loop_end:

  .no_TX:
	pop	ax
	ret




align 4
init_mac_regs:

	DEBUGF	1,"initializing MAC regs\n"

	; MAC operation register
	mov	ax, 1
	set_io	0
	set_io	MCR1
	out	dx, ax
	; Reset MAC
	mov	ax, 2
	set_io	MAC_SM
	out	dx, ax
	; Reset internal state machine
	xor	ax, ax
	out	dx, ax
	mov	esi, 5
	stdcall Sleep

	call	read_mac

	ret




; Read a word data from PHY Chip

align 4
proc  phy_read stdcall, phy_addr:dword, reg:dword

	DEBUGF	1,"PHY read, addr=0x%x reg=0x%x\n", [phy_addr]:8, [reg]:8

	mov	eax, [phy_addr]
	shl	eax, 8
	add	eax, [reg]
	add	eax, MDIO_READ
	set_io	0
	set_io	MMDIO
	out	dx, ax

	;Wait for the read bit to be cleared.
	mov	ecx, 2048 ;limit
  .read:
	in	ax, dx
	test	ax, MDIO_READ
	jz	@f
	dec	ecx
	jnz	.read
  @@:

	set_io	MMRD
	in	ax, dx
	and	eax, 0xFFFF

	DEBUGF	1,"PHY read, val=0x%x\n", eax:4

	ret

endp




; Write a word data to PHY Chip

align 4
proc  phy_write stdcall, phy_addr:dword, reg:dword, val:dword

	DEBUGF	1,"PHY write, addr=0x%x reg=0x%x val=0x%x\n", [phy_addr]:8, [reg]:8, [val]:8

	mov	eax, [val]
	set_io	0
	set_io	MMWD
	out	dx, ax

	;Write the command to the MDIO bus

	mov	eax, [phy_addr]
	shl	eax, 8
	add	eax, [reg]
	add	eax, MDIO_WRITE
	set_io	MMDIO
	out	dx, ax

	;Wait for the write bit to be cleared.
	mov	ecx, 2048 ;limit
  .write:
	in	ax, dx
	test	ax, MDIO_WRITE
	jz	@f
	dec	ecx
	jnz	.write
  @@:

	DEBUGF	1,"PHY write ok\n"

	ret
endp



align 4
read_mac:

	DEBUGF	2,"Reading MAC: "

	mov	cx, 3
	lea	edi, [device.mac]
	set_io	0
	set_io	MID_0L
     .mac:
	in	ax, dx
	stosw
	inc	dx
	inc	dx
	dec	cx
	jnz	.mac

	DEBUGF	2,"%x-%x-%x-%x-%x-%x\n",[edi-6]:2, [edi-5]:2, [edi-4]:2, [edi-3]:2, [edi-2]:2, [edi-1]:2

	ret




; End of code

section '.data' data readable writable align 16 ; place all uninitialized data place here
align 4 					; Place all initialised data here

devices 	dd 0
version 	dd (DRIVER_VERSION shl 16) or (API_VERSION and 0xFFFF)
my_service	db 'R6040',0			; max 16 chars include zero

include_debug_strings				; All data wich FDO uses will be included here

device_list	rd MAX_DEVICES			; This list contains all pointers to device structures the driver is handling

