;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2010. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  DEC 21x4x driver for KolibriOS                                 ;;
;;                                                                 ;;
;;  Based on dec21140.Asm from Solar OS by                         ;;
;;     Eugen Brasoveanu,                                           ;;
;;       Ontanu Bogdan Valentin                                    ;;
;;                                                                 ;;
;;  Written by hidnplayr@kolibrios.org                             ;;
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

include 'proc32.inc'
include 'imports.inc'
include 'fdo.inc'
include 'netdrv.inc'

public START
public service_proc
public version

virtual at ebx

	device:

	ETH_DEVICE

	.rx_p_des	  dd ?	; descriptors ring with received packets
	.tx_p_des	  dd ?	; descriptors ring with 'to transmit' packets
	.tx_free_des	  dd ?	; Tx descriptors available
	.tx_wr_des	  dd ?	; Tx current descriptor to write data to
	.tx_rd_des	  dd ?	; Tx current descriptor to read TX completion
	.rx_crt_des	  dd ?	; Rx current descriptor

	.io_addr	  dd ?
	.pci_bus	  db ?
	.pci_dev	  db ?
	.irq_line	  db ?

	.size = $ - device

end virtual

;-------------------------------------------
; configuration registers
;-------------------------------------------
CFCS	equ	4	; configuration and status register

CSR0	equ	0x00	 ; Bus mode
CSR1	equ	0x08	 ; Transmit Poll Command
CSR2	equ	0x10	 ; Receive Poll Command
CSR3	equ	0x18	 ; Receive list base address
CSR4	equ	0x20	 ; Transmit list base address
CSR5	equ	0x28	 ; Status
CSR6	equ	0x30	 ; Operation mode
CSR7	equ	0x38	 ; Interrupt enable
CSR8	equ	0x40	 ; Missed frames and overflow counter
CSR9	equ	0x48	 ; Boot ROM, serial ROM, and MII management
CSR10	equ	0x50	 ; Boot ROM programming address
CSR11	equ	0x58	 ; General-purpose timer
CSR12	equ	0x60	 ; General-purpose port
CSR13	equ	0x68
CSR14	equ	0x70
CSR15	equ	0x78	 ; Watchdog timer

;--------bits/commands of CSR0-------------------
CSR0_RESET		equ	1b

CSR0_WIE		equ	1 SHL 24	; Write and Invalidate Enable
CSR0_RLE		equ	1 SHL 23	; PCI Read Line Enable
CSR0_RML		equ	1 SHL 21	; PCI Read Multiple

CSR0_CACHEALIGN_NONE	equ	00b SHL 14
CSR0_CACHEALIGN_32	equ	01b SHL 14
CSR0_CACHEALIGN_64	equ	10b SHL 14
CSR0_CACHEALIGN_128	equ	11b SHL 14

; using values from linux driver.. :P
CSR0_DEFAULT		equ	CSR0_WIE+CSR0_RLE+CSR0_RML+CSR0_CACHEALIGN_NONE ;32

;------- CSR5 -STATUS- bits --------------------------------
CSR5_TI 		equ	1 SHL 0 	; Transmit interupt - frame transmition completed
CSR5_TPS		equ	1 SHL 1 	; Transmit process stopped
CSR5_TU 		equ	1 SHL 2 	; Transmit Buffer unavailable
CSR5_TJT		equ	1 SHL 3 	; Transmit Jabber Timeout (transmitter had been excessively active)
CSR5_UNF		equ	1 SHL 5 	; Transmit underflow - FIFO underflow
CSR5_RI 		equ	1 SHL 6 	; Receive Interrupt
CSR5_RU 		equ	1 SHL 7 	; Receive Buffer unavailable
CSR5_RPS		equ	1 SHL 8 	; Receive Process stopped
CSR5_RWT		equ	1 SHL 9 	; Receive Watchdow Timeout
CSR5_ETI		equ	1 SHL 10	; Early transmit Interrupt
CSR5_GTE		equ	1 SHL 11	; General Purpose Timer Expired
CSR5_FBE		equ	1 SHL 13	; Fatal bus error
CSR5_ERI		equ	1 SHL 14	; Early receive Interrupt
CSR5_AIS		equ	1 SHL 15	; Abnormal interrupt summary
CSR5_NIS		equ	1 SHL 16	; normal interrupt summary
CSR5_RS_SH		equ	1 SHL 17	; Receive process state  -shift
CSR5_RS_MASK		equ	111b		;                        -mask
CSR5_TS_SH		equ	1 SHL 20	; Transmit process state -shift
CSR5_TS_MASK		equ	111b		;                        -mask
CSR5_EB_SH		equ	1 SHL 23	; Error bits             -shift
CSR5_EB_MASK		equ	111b		; Error bits             -mask

;CSR5 TS values
CSR5_TS_STOPPED 	       equ     000b
CSR5_TS_RUNNING_FETCHING_DESC  equ     001b
CSR5_TS_RUNNING_WAITING_TX     equ     010b
CSR5_TS_RUNNING_READING_BUFF   equ     011b
CSR5_TS_RUNNING_SETUP_PCKT     equ     101b
CSR5_TS_SUSPENDED	       equ     110b
CSR5_TS_RUNNING_CLOSING_DESC   equ     111b

;------- CSR6 -OPERATION MODE- bits --------------------------------
CSR6_HP 		equ	1 SHL 0 	; Hash/Perfect Receive Filtering mode
CSR6_SR 		equ	1 SHL 1 	; Start/Stop receive
CSR6_HO 		equ	1 SHL 2 	; Hash only Filtering mode
CSR6_PB 		equ	1 SHL 3 	; Pass bad frames
CSR6_IF 		equ	1 SHL 4 	; Inverse filtering
CSR6_SB 		equ	1 SHL 5 	; Start/Stop backoff counter
CSR6_PR 		equ	1 SHL 6 	; Promiscuos mode -default after reset
CSR6_PM 		equ	1 SHL 7 	; Pass all multicast
CSR6_F			equ	1 SHL 9 	; Full Duplex mode
CSR6_OM_SH		equ	1 SHL 10	; Operating Mode -shift
CSR6_OM_MASK		equ	11b		;                -mask
CSR6_FC 		equ	1 SHL 12	; Force Collision Mode
CSR6_ST 		equ	1 SHL 13	; Start/Stop Transmission Command
CSR6_TR_SH		equ	1 SHL 14	; Threshold Control      -shift
CSR6_TR_MASK		equ	11b		;                        -mask
CSR6_CA 		equ	1 SHL 17	; Capture Effect Enable
CSR6_PS 		equ	1 SHL 18	; Port select SRL / MII/SYM
CSR6_HBD		equ	1 SHL 19	; Heartbeat Disable
CSR6_SF 		equ	1 SHL 21	; Store and Forward -transmit full packet only
CSR6_TTM		equ	1 SHL 22	; Transmit Threshold Mode -
CSR6_PCS		equ	1 SHL 23	; PCS active and MII/SYM port operates in symbol mode
CSR6_SCR		equ	1 SHL 24	; Scrambler Mode
CSR6_MBO		equ	1 SHL 25	; Must Be One
CSR6_RA 		equ	1 SHL 30	; Receive All
CSR6_SC 		equ	1 SHL 31	; Special Capture Effect Enable


;------- CSR7 -INTERRUPT ENABLE- bits --------------------------------
CSR7_TI 		equ	1 SHL 0 	; transmit Interrupt Enable (set with CSR7<16> & CSR5<0> )
CSR7_TS 		equ	1 SHL 1 	; transmit Stopped Enable (set with CSR7<15> & CSR5<1> )
CSR7_TU 		equ	1 SHL 2 	; transmit buffer underrun Enable (set with CSR7<16> & CSR5<2> )
CSR7_TJ 		equ	1 SHL 3 	; transmit jabber timeout enable (set with CSR7<15> & CSR5<3> )
CSR7_UN 		equ	1 SHL 5 	; underflow Interrupt enable (set with CSR7<15> & CSR5<5> )
CSR7_RI 		equ	1 SHL 6 	; receive Interrupt enable (set with CSR7<16> & CSR5<5> )
CSR7_RU 		equ	1 SHL 7 	; receive buffer unavailable enable (set with CSR7<15> & CSR5<7> )
CSR7_RS 		equ	1 SHL 8 	; Receive stopped enable (set with CSR7<15> & CSR5<8> )
CSR7_RW 		equ	1 SHL 9 	; receive watchdog timeout enable (set with CSR7<15> & CSR5<9> )
CSR7_ETE		equ	1 SHL 10	; Early transmit Interrupt enable (set with CSR7<15> & CSR5<10> )
CSR7_GPT		equ	1 SHL 11	; general purpose timer enable (set with CSR7<15> & CSR5<11> )
CSR7_FBE		equ	1 SHL 13	; Fatal bus error enable (set with CSR7<15> & CSR5<13> )
CSR7_ERE		equ	1 SHL 14	; Early receive enable (set with CSR7<16> & CSR5<14> )
CSR7_AI 		equ	1 SHL 15	; Abnormal Interrupt Summary Enable (enables CSR5<0,3,7,8,9,10,13>)
CSR7_NI 		equ	1 SHL 16	; Normal Interrup Enable (enables CSR5<0,2,6,11,14>)
CSR7_DEFAULT		equ	CSR7_TI+CSR7_TS+CSR7_RI+CSR7_RS+CSR7_TU+CSR7_TJ+CSR7_UN+\
					CSR7_RU+CSR7_RW+CSR7_FBE+CSR7_AI+CSR7_NI

;----------- descriptor structure ---------------------
struc DES {
	.DES0	 DD	 ?	 ; bit 31 is 'own' and rest is 'status'
	.DES1	 DD	 ?	 ; control bits + bytes-count buffer 1 + bytes-count buffer 2
	.DES2	 DD	 ?	 ; pointer to buffer1
	.DES3	 DD	 ?	 ; pointer to buffer2 or in this case to next descriptor, as we use a chained structure
	.realaddr dd ?
	.size = 64
}

virtual at 0
	DES DES
end virtual

;common to Rx and Tx
DES0_OWN	equ	1 SHL 31	; if set, the NIC controls the descriptor, otherwise driver 'owns' the descriptors

;receive
RDES0_ZER	equ	1 SHL 0 	; must be 0 if legal length :D
RDES0_CE	equ	1 SHL 1 	; CRC error, valid only on last desc (RDES0<8>=1)
RDES0_DB	equ	1 SHL 2 	; dribbling bit - not multiple of 8 bits, valid only on last desc (RDES0<8>=1)
RDES0_RE	equ	1 SHL 3 	; Report on MII error.. i dont realy know what this means :P
RDES0_RW	equ	1 SHL 4 	; received watchdog timer expiration - must set CSR5<9>, valid only on last desc (RDES0<8>=1)
RDES0_FT	equ	1 SHL 5 	; frame type: 0->IEEE802.0 (len<1500) 1-> ETHERNET frame (len>1500), valid only on last desc (RDES0<8>=1)
RDES0_CS	equ	1 SHL 6 	; Collision seen, valid only on last desc (RDES0<8>=1)
RDES0_TL	equ	1 SHL 7 	; Too long(>1518)-NOT AN ERROR, valid only on last desc (RDES0<8>=1)
RDES0_LS	equ	1 SHL 8 	; Last descriptor of current frame
RDES0_FS	equ	1 SHL 9 	; First descriptor of current frame
RDES0_MF	equ	1 SHL 10	; Multicast frame, valid only on last desc (RDES0<8>=1)
RDES0_RF	equ	1 SHL 11	; Runt frame, valid only on last desc (RDES0<8>=1) and id overflow
RDES0_DT_SERIAL equ	00b SHL 12	; Data type-Serial recv frame, valid only on last desc (RDES0<8>=1)
RDES0_DT_INTERNAL equ	01b SHL 12	; Data type-Internal loopback recv frame, valid only on last desc (RDES0<8>=1)
RDES0_DT_EXTERNAL equ	11b SHL 12	; Data type-External loopback recv frame, valid only on last desc (RDES0<8>=1)
RDES0_DE	equ	1 SHL 14	; Descriptor error - cant own a new desc and frame doesnt fit, valid only on last desc (RDES0<8>=1)
RDES0_ES	equ	1 SHL 15	; Error Summmary - bits 1+6+11+14, valid only on last desc (RDES0<8>=1)
RDES0_FL_SH	equ	16		; Field length shift, valid only on last desc (RDES0<8>=1)
RDES0_FL_MASK	equ	11111111111111b ; Field length mask (+CRC), valid only on last desc (RDES0<8>=1)
RDES0_FF	equ	1 SHL 30	; Filtering fail-frame failed address recognition test(must CSR6<30>=1), valid only on last desc (RDES0<8>=1)

RDES1_RBS1_MASK equ	11111111111b	; firsd buffer size MASK
RDES1_RBS2_SH	equ	1 SHL 11	; second buffer size SHIFT
RDES1_RBS2_MASK equ	11111111111b	; second buffer size MASK
RDES1_RCH	equ	1 SHL 24	; Second address chained - second address (buffer) is next desc address
RDES1_RER	equ	1 SHL 25	; Receive End of Ring - final descriptor, NIC must return to first desc

;transmition
TDES0_DE	equ	1 SHL 0 	; Deffered
TDES0_UF	equ	1 SHL 1 	; Underflow error
TDES0_LF	equ	1 SHL 2 	; Link fail report (only if CSR6<23>=1)
TDES0_CC_SH	equ	3		; Collision Count shift - no of collision before transmition
TDES0_CC_MASK	equ	1111b		; Collision Count mask
TDES0_HF	equ	1 SHL 7 	; Heartbeat fail
TDES0_EC	equ	1 SHL 8 	; Excessive Collisions - >16 collisions
TDES0_LC	equ	1 SHL 9 	; Late collision
TDES0_NC	equ	1 SHL 10	; No carrier
TDES0_LO	equ	1 SHL 11	; Loss of carrier
TDES0_TO	equ	1 SHL 14	; Transmit Jabber Timeout
TDES0_ES	equ	1 SHL 15	; Error summary TDES0<1+8+9+10+11+14>=1

TDES1_TBS1_MASK equ	11111111111b	; Buffer 1 size mask
TDES1_TBS2_SH	equ	11		; Buffer 2 size shift
TDES1_TBS2_MASK equ	11111111111b	; Buffer 2 size mask
TDES1_FT0	equ	1 SHL 22	; Filtering type 0
TDES1_DPD	equ	1 SHL 23	; Disabled padding for packets <64bytes, no padding
TDES1_TCH	equ	1 SHL 24	; Second address chained - second buffer pointer is to next desc
TDES1_TER	equ	1 SHL 25	; Transmit end of ring - final descriptor
TDES1_AC	equ	1 SHL 26	; Add CRC disable -pretty obvious
TDES1_SET	equ	1 SHL 27	; Setup packet
TDES1_FT1	equ	1 SHL 28	; Filtering type 1
TDES1_FS	equ	1 SHL 29	; First segment - buffer is first segment of frame
TDES1_LS	equ	1 SHL 30	; Last segment
TDES1_IC	equ	1 SHL 31	; Interupt on completion (CSR5<0>=1) valid when TDES1<30>=1

MAX_ETH_FRAME_SIZE	equ	1514

RX_DES_COUNT		equ	4	       ; no of RX descriptors, must be power of 2
RX_BUFF_SIZE		equ	2048		; size of buffer for each descriptor, must be multiple of 4 and <= 2048 TDES1_TBS1_MASK
TX_DES_COUNT		equ	4	       ; no of TX descriptors, must be power of 2
TX_BUFF_SIZE		equ	2048		; size of buffer for each descriptor, used for memory allocation only

RX_MEM_TOTAL_SIZE	equ	RX_DES_COUNT*(DES.size+RX_BUFF_SIZE)
TX_MEM_TOTAL_SIZE	equ	TX_DES_COUNT*(DES.size+TX_BUFF_SIZE)

;=============================================================================
; serial ROM operations
;=============================================================================
CSR9_SR 		equ	1 SHL 11	; SROM Select
CSR9_RD 		equ	1 SHL 14	; ROM Read Operation
CSR9_SROM_DO		equ	1 SHL 3 	; Data Out for SROM
CSR9_SROM_DI		equ	1 SHL 2 	; Data In to SROM
CSR9_SROM_CK		equ	1 SHL 1 	; clock for SROM
CSR9_SROM_CS		equ	1 SHL 0 	; chip select.. always needed

; assume dx is CSR9
macro SROM_Delay {
	push	eax
	in	eax, dx
	in	eax, dx
	in	eax, dx
	in	eax, dx
	in	eax, dx
	in	eax, dx
	in	eax, dx
	in	eax, dx
	in	eax, dx
	in	eax, dx
	pop	eax
}

; assume dx is CSR9
macro MDIO_Delay {
	push	eax
	in	eax, dx
	pop	eax
}

macro Bit_Set a_bit {
	in	eax, dx
	or	eax, a_bit
	out	dx , eax
}

macro Bit_Clear a_bit {
	in	eax, dx
	and	eax, NOT (a_bit)
	out	dx , eax
}


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

	DEBUGF	2,"Loading dec21x4x driver\n"
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

;        mov     eax, [IOCTL.input]                     ; get the pci bus and device numbers
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

	push	edx
	stdcall KernelAlloc, dword device.size		; Allocate the buffer for eth_device structure
	pop	edx
	test	eax, eax
	jz	.fail
	mov	ebx, eax				; ebx is always used as a pointer to the structure (in driver, but also in kernel code)

; Fill in the direct call addresses into the struct

	mov	[device.reset], reset
	mov	[device.transmit], transmit
	mov	[device.get_MAC], read_mac
	mov	[device.set_MAC], write_mac
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
	[device.pci_dev]:1,[device.pci_bus]:1,[device.irq_line]:1,[device.io_addr]:8

	allocate_and_clear [device.rx_p_des], RX_DES_COUNT*(DES.size+RX_BUFF_SIZE), .err
	allocate_and_clear [device.tx_p_des], TX_DES_COUNT*(DES.size+TX_BUFF_SIZE), .err

; Ok, the eth_device structure is ready, let's probe the device
; Because initialization fires IRQ, IRQ handler must be aware of this device
	mov	eax, [devices]						; Add the device structure to our device list
	mov	[device_list+4*eax], ebx				; (IRQ handler uses this list to find device)
	inc	[devices]						;

	call	probe							; this function will output in eax
	test	eax, eax
	jnz	.err2							; If an error occured, exit


	mov	[device.type], NET_TYPE_ETH
	call	NetRegDev

	cmp	eax, -1
	je	.destroy

	ret

; If the device was already loaded, find the device number and return it in eax

  .find_devicenum:
	DEBUGF	2,"Trying to find device number of already registered device\n"
	mov	ebx, eax
	call	NetPtrToNum						; This kernel procedure converts a pointer to device struct in ebx
									; into a device number in edi
	mov	eax, edi						; Application wants it in eax instead
	DEBUGF	2,"Kernel says: %u\n", eax
	ret

; If an error occured, remove all allocated data and exit (returning -1 in eax)

  .destroy:
	; todo: reset device into virgin state

  .err2:
	dec	[devices]
  .err:
	DEBUGF	2,"removing device structure\n"
	stdcall KernelFree, [device.rx_p_des]
	stdcall KernelFree, [device.tx_p_des]
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


macro status {
	set_io	CSR5
	in	eax, dx
	DEBUGF	1,"CSR5: %x\n", eax
}


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                         ;;
;; Probe                                   ;;
;;                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
probe:

	DEBUGF	2,"Probing dec21x4x device: "

	make_bus_master [device.pci_bus], [device.pci_dev]

	movzx	eax, [device.pci_bus]
	movzx	ecx, [device.pci_dev]
	stdcall PciRead32, eax ,ecx ,0				      ; get device/vendor id

	DEBUGF	1,"Vendor id: 0x%x\n", ax

	cmp	ax , 0x1011
	jne	.notfound
	shr	eax, 16

	DEBUGF	1,"Vendor ok!, device id: 0x%x\n", ax		      ; TODO: use another method to detect chip!

	cmp	ax , 0x0009
	je	.supported_device

	cmp	ax , 0x0019
	je	.supported_device2

  .notfound:
	DEBUGF	1,"Device not supported!\n"
	or	eax, -1
	ret

  .supported_device2:

	; wake up the 21143

	movzx	ecx, [device.pci_bus]
	movzx	edx, [device.pci_dev]
	xor	eax, eax
	stdcall PciWrite32, ecx, edx, 0x40, eax


  .supported_device:
	call	SROM_GetWidth		; TODO: use this value returned in ecx
					; in the read_word routine!

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                         ;;
;; Reset                                   ;;
;;                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
reset:

	DEBUGF	2,"Resetting dec21x4x\n"

;-----------------------------------------------------------
; board software reset - if fails, dont do nothing else

	set_io	0
	status
	set_io	CSR0
	mov	eax, CSR0_RESET
	out	dx , eax

; wait at least 50 PCI cycles
	mov	esi, 1000
	call	Sleep

;-----------
; setup CSR0

	set_io	0
	status
	set_io	CSR0
	mov	eax, CSR0_DEFAULT
	out	dx , eax


; wait at least 50 PCI cycles
	mov	esi, 1000
	call	Sleep

;-----------------------------------
; Read mac from eeprom to driver ram

	call	read_mac_eeprom

;--------------------------------
; insert irq handler on given irq

	movzx	eax, [device.irq_line]
	DEBUGF	1,"Attaching int handler to irq %x\n", eax:1
	stdcall AttachIntHandler, eax, int_handler, dword 0
	test	eax, eax
	jnz	@f
	DEBUGF	1,"\nCould not attach int handler!\n"
;        or      eax, -1
;        ret
  @@:

	set_io	0
	status

;------------------------------------------
; Setup RX descriptors (use chained method)

	mov	eax, [device.rx_p_des]
	call	GetPgAddr
	mov	edx, eax
	lea	esi, [eax + RX_DES_COUNT*(DES.size)]	; jump over RX descriptors

	mov	eax, [device.rx_p_des]
	add	eax, RX_DES_COUNT*(DES.size)		; jump over RX descriptors

	mov	edi, [device.rx_p_des]
	mov	ecx, RX_DES_COUNT

	push	edx		       ;;
  .loop_rx_des:
	add	edx, DES.size
	mov	[edi+DES.DES0], DES0_OWN			; hardware owns buffer
	mov	[edi+DES.DES1], 1984+RDES1_RCH			; only size of first buffer, chained buffers
	mov	[edi+DES.DES2], esi				; hw buffer address
	mov	[edi+DES.DES3], edx				; pointer to next descriptor
	mov	[edi+DES.realaddr], eax 			; virtual buffer address

	DEBUGF	1,"RX desc %u, buff addr: %x, next desc: %x, real buff addr: %x, real descr addr: %x \n", ecx, esi, edx, eax, edi

	add	esi, RX_BUFF_SIZE
	add	eax, RX_BUFF_SIZE
	add	edi, DES.size
	dec	ecx
	jnz	.loop_rx_des

; set last descriptor as LAST
	sub	edi, DES.size
	or	[edi+DES.DES1], RDES1_RER	; EndOfRing
	pop	edx			;;
	mov	[edi+DES.DES3], edx	;;

;---------------------
; Setup TX descriptors

	mov	eax, [device.tx_p_des]
	call	GetPgAddr
	mov	edx, eax
	lea	esi, [eax + TX_DES_COUNT*(DES.size)]	; jump over TX descriptors

	mov	eax, [device.tx_p_des]
	add	eax, TX_DES_COUNT*(DES.size)	; jump over TX descriptors

	mov	edi, [device.tx_p_des]
	mov	ecx, TX_DES_COUNT

	push	edx		       ;;
  .loop_tx_des:
	add	edx, DES.size
	mov	[edi+DES.DES0], 0		; owned by driver
	mov	[edi+DES.DES1], TDES1_TCH	; chained method
	mov	[edi+DES.DES2], esi		; pointer to buffer
	mov	[edi+DES.DES3], edx		; pointer to next descr
	mov	[edi+DES.realaddr], eax

	DEBUGF	1,"TX desc %u, buff addr: %x, next desc: %x, real buff addr: %x, real descr addr: %x \n", ecx, esi, edx, eax, edi

	add	esi, TX_BUFF_SIZE
	add	eax, TX_BUFF_SIZE
	add	edi, DES.size
	dec	ecx
	jnz	.loop_tx_des
	
; set last descriptor as LAST
	sub	edi, DES.size
	or	[edi+DES.DES1], TDES1_TER	; EndOfRing
	pop	edx			       ;;;
	mov	[edi+DES.DES3], edx	       ;;;

;------------------
; Reset descriptors

	mov	[device.tx_wr_des], 0
	mov	[device.tx_rd_des], 0
	mov	[device.rx_crt_des], 0
	mov	[device.tx_free_des], TX_DES_COUNT

;--------------------------------------------
; setup CSR3 & CSR4 (pointers to descriptors)

	set_io	0
	status
	set_io	CSR3
	mov	eax, [device.rx_p_des]
	call	GetPgAddr
	DEBUGF	1,"RX descriptor base address: %x\n", eax
	out	dx , eax

	set_io	CSR4
	mov	eax, [device.tx_p_des]
	call	GetPgAddr
	DEBUGF	1,"TX descriptor base address: %x\n", eax
	out	dx , eax

;-------------------------------------------------------
; setup interrupt mask register -expect IRQs from now on

	status
	DEBUGF	1,"Enabling interrupts\n"
	set_io	CSR7
	mov	eax, CSR7_DEFAULT
	out	dx , eax
	status

;----------
; enable RX

	set_io	0
	status
	DEBUGF	1,"Enable RX\n"

	set_io	CSR6
	Bit_Set CSR6_SR; or CSR6_PR or CSR6_ST
	DEBUGF	1,"CSR6: %x\n", eax

	status

	call	start_link


; wait a bit
	mov	esi, 3000
	call	Sleep

;----------------------------------------------------
; send setup packet to notify the board about the MAC

	call	Send_Setup_Packet

	xor	eax, eax
; clear packet/byte counters

	lea	edi, [device.bytes_tx]
	mov	ecx, 6
	rep	stosd

; Set the mtu, kernel will be able to send now
	mov	[device.mtu], 1514

	DEBUGF	1,"Reset done\n"

	ret


align 4
start_link:

	; TODO: write working code here

	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                         ;;
;; Send setup packet                       ;;
;;                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
Send_Setup_Packet:

	DEBUGF	1,"Sending setup packet\n"

; if no descriptors available, out
	mov	ecx, 1000
@@loop_wait_desc:
	cmp	[device.tx_free_des], 0
	jne	@f

	dec	ecx
	jnz	@@loop_wait_desc

	mov	eax, -1
	ret
      @@:

; go to current send descriptor
	mov	edi, [device.tx_p_des]
	mov	eax, [device.tx_wr_des]
	DEBUGF	1,"Got free descriptor: %u (%x)", eax, edi
	mov	edx, DES.size
	mul	edx
	add	edi, eax
	DEBUGF	1,"=>%x\n",  edi

; if NOT sending FIRST setup packet, must set current descriptor to 0 size for both buffers,
;  and go to next descriptor for real setup packet...            ;; TODO: check if 2 descriptors are available

;       cmp     [device.tx_packets], 0
;       je      .not_first
;               
;       and     [edi+DES.des1], 0
;       mov     [edi+DES.des0], DES0_OWN
;               
; go to next descriptor
;        inc     [device.tx_wr_des]
;        and     [device.tx_wr_des], TX_DES_COUNT-1
;
; dec free descriptors count
;        cmp     [device.tx_free_des], 0
;        jz      @f
;        dec     [device.tx_free_des]
;       @@:
;
;       ; recompute pointer to current descriptor
;       mov     edi, [device.tx_p_des]
;       mov     eax, [device.tx_wr_des]
;       mov     edx, DES.size
;       mul     edx
;       add     edi, eax

  .not_first:

	push	edi
; copy setup packet to current descriptor
	mov	edi, [edi+DES.realaddr]
; copy once the address
	lea	esi, [device.mac]
	DEBUGF	1,"copying packet to %x from %x\n", edi, esi
	mov	ecx, 3	; mac is 6 bytes thus 3 words
     .loop:
	DEBUGF	1,"%x ", [esi]:4
	movsw
	dec	esi
	dec	esi
	movsw
	dec	ecx
	jnz	.loop

	DEBUGF	1,"\n"

; copy 15 times the broadcast address
	mov	ecx, 3*15
	mov	eax, 0xffffffff
	rep	stosd

	pop	edi

; setup descriptor
	DEBUGF	1,"setting up descriptor\n"
	mov	[edi+DES.DES1], TDES1_IC+TDES1_SET+TDES1_TCH+192	; size must be EXACTLY 192 bytes
	mov	[edi+DES.DES0], DES0_OWN

	DEBUGF	1,"TDES0: %x\n", [edi+DES.DES0]:8
	DEBUGF	1,"TDES1: %x\n", [edi+DES.DES1]:8
	DEBUGF	1,"TDES2: %x\n", [edi+DES.DES2]:8
	DEBUGF	1,"TDES3: %x\n", [edi+DES.DES3]:8

; go to next descriptor
	inc	[device.tx_wr_des]
	and	[device.tx_wr_des], TX_DES_COUNT-1

; dec free descriptors count
	cmp	[device.tx_free_des], 0
	jz	@f
	dec	[device.tx_free_des]
       @@:

; start tx
	set_io	0
	status
	set_io	CSR6
	in	eax, dx
	test	eax, CSR6_ST		; if NOT started, start now
	jnz	.already_started
	or	eax, CSR6_ST
	DEBUGF	1,"Starting TX\n"
	jmp	.do_it
  .already_started:
					; if already started, issue a Transmit Poll command
	set_io	CSR1
	mov	eax, 0
	DEBUGF	1,"Issuing transmit poll command\n"
  .do_it:
	out	dx , eax
	status

	DEBUGF	1,"Sending setup packet, completed!\n"

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

	DEBUGF	1,"Transmitting packet, buffer:%x, size:%u\n",[esp+4],[esp+8]
	mov	eax, [esp+4]
	DEBUGF	1,"To: %x-%x-%x-%x-%x-%x From: %x-%x-%x-%x-%x-%x Type:%x%x\n",\
	[eax+00]:2,[eax+01]:2,[eax+02]:2,[eax+03]:2,[eax+04]:2,[eax+05]:2,\
	[eax+06]:2,[eax+07]:2,[eax+08]:2,[eax+09]:2,[eax+10]:2,[eax+11]:2,\
	[eax+13]:2,[eax+12]:2

	cmp	dword [esp+8], MAX_ETH_FRAME_SIZE
	ja	.fail

	cmp	[device.tx_free_des], 0
	je	.fail

;--------------------------
; copy packet to crt buffer
	
	mov	eax, [device.tx_wr_des]
	mov	edx, DES.size
	mul	edx
	add	eax, [device.tx_p_des]
	mov	edi, [eax+DES.realaddr] 		; pointer to buffer
	mov	esi, [esp+4]
	mov	ecx, [esp+8]
	DEBUGF	1,"copying %u bytes from %x to %x\n", ecx, esi, edi
	rep	movsb

; set packet size
	mov	ecx, [eax+DES.DES1]
	and	ecx, TDES1_TER				; preserve 'End of Ring' bit
	or	ecx, [esp+8]				; set size
	or	ecx, TDES1_FS or TDES1_LS or TDES1_IC or TDES1_TCH    ; first descr, last descr, interrupt on complete, chained modus
	mov	[eax+DES.DES1], ecx

; set descriptor info
	mov	[eax+DES.DES0], DES0_OWN		; say it is now owned by the 21x4x

; start tx
	set_io	0
	status
	set_io	CSR6
	in	eax, dx
	test	eax, CSR6_ST		; if NOT started, start now
	jnz	.already_started
	or	eax, CSR6_ST
	DEBUGF	1,"Starting TX\n"
	jmp	.do_it
  .already_started:
					; if already started, issues a Transmit Poll command
	set_io	CSR1
	mov	eax, -1
  .do_it:
	out	dx , eax

; Update stats

	inc	[device.packets_tx]
	mov	eax, [esp+8]
	add	dword [device.bytes_tx], eax
	adc	dword [device.bytes_tx + 4], 0

; go to next descriptor
	inc	[device.tx_wr_des]
	and	[device.tx_wr_des], TX_DES_COUNT-1

; dec free descriptors count
	test	[device.tx_free_des], -1
	jz	.end
	dec	[device.tx_free_des]
  .end:
	status

	DEBUGF	1,"transmit ok\n"
	xor	eax, eax
	stdcall KernelFree, [esp+4]
	ret	8

  .fail:
	DEBUGF	1,"transmit failed\n"
	or	eax, -1
	stdcall KernelFree, [esp+4]
	ret	8


;;;;;;;;;;;;;;;;;;;;;;;
;;                   ;;
;; Interrupt handler ;;
;;                   ;;
;;;;;;;;;;;;;;;;;;;;;;;

align 4
int_handler:

	DEBUGF	1,"IRQ %x ",eax:2			; no, you cant replace 'eax:2' with 'al', this must be a bug in FDO

; find pointer of device wich made IRQ occur

	mov	ecx, [devices]
	test	ecx, ecx
	jz	.fail
	mov	esi, device_list
  .nextdevice:
	mov	ebx, dword [esi]

	set_io	0
	set_io	CSR5
	in	eax, dx
	out	dx , eax				; send it back to ACK

	and	eax, CSR7_DEFAULT			; int mask
	test	eax, eax
	jnz	.got_it

  .continue:
	add	esi, 4
	dec	ecx
	jnz	.nextdevice

	ret						; If no device was found, abort (The irq was probably for a device, not registered to this driver)

  .got_it:

	DEBUGF	1,"CSR7: %x ", eax

; looks like we've found it!

; Lets found out why the irq occured then..

;----------------------------------
; TX ok?

	test	eax, CSR5_TI
	jz	.not_tx
	push	ax esi ecx

	DEBUGF 1,"TX ok!\n"
		
	; go to current descriptor
	mov	edi, [device.tx_p_des]

	mov	eax, [device.tx_rd_des]
	mov	edx, DES.size
	mul	edx
	add	edi, eax
		
      .loop_tx:
		
	; done if all desc are free
	cmp	[device.tx_free_des], TX_DES_COUNT
	jz	.end_tx

	mov	eax, [edi+DES.DES0]

	; we stop at first desc that is owned be NIC
	test	eax, DES0_OWN
	jnz	.end_tx

	; detect is setup packet
	cmp	eax, (0ffffffffh - DES0_OWN)		; all other bits are 1
	jne	.not_setup_packet
	DEBUGF	1,"Setup Packet detected\n"
      .not_setup_packet:

	DEBUGF	1,"packet status: %x\n", eax

	; next descriptor
	add	edi, DES.size
	inc	[device.tx_rd_des]
	and	[device.tx_rd_des], TX_DES_COUNT-1

	; inc free desc
	inc	[device.tx_free_des]
	cmp	[device.tx_free_des], TX_DES_COUNT
	jle	@f
	mov	[device.tx_free_des], TX_DES_COUNT
       @@:

	jmp	.loop_tx
      .end_tx:
		
	;------------------------------------------------------
	; here must be called standard Ethernet Tx Irq Handler
	;------------------------------------------------------

	pop	ecx esi ax

;----------------------------------
; RX irq
  .not_tx:
	test	eax, CSR5_RI
	jz	.not_rx
	push	ax esi ecx

	DEBUGF 1,"RX ok!\n"
		
	;go to current descriptor
	mov	edi, [device.rx_p_des]

	mov	eax, [device.rx_crt_des]
	mov	edx, DES.size
	mul	edx
	add	edi, eax
		
      .loop_rx_start_of_packet:
	mov	eax, [edi+DES.DES0]

	test	eax, DES0_OWN
	jnz	.end_rx 				; current desc is busy, nothing to do

	test	eax, RDES0_FS
	jz	.end_rx 				; current desc is NOT first packet, ERROR!

	test	eax, RDES0_LS				; if not last desc of packet, error for now
	jz	.end_rx

;        .IF     ZERO?
;                ODS2 <"Net_Interrupt: packet > 1 descriptor, not supported yet :P">
;                jmp     @@end_rx
;        .ENDIF

	test	eax, RDES0_ES
	jnz	.end_rx

;        .IF     !ZERO?
;                ODS2 <"Net_Interrupt: RX error">
;                jmp     @@end_rx
;        .ENDIF

	mov	esi, [edi+DES.realaddr]
	mov	ecx, [edi+DES.DES0]
	shr	ecx, RDES0_FL_SH
	and	ecx, RDES0_FL_MASK
	sub	ecx, 4					 ; crc

	DEBUGF	1,"Received packet!, size=%u, addr:%x\n", ecx, esi

	push	esi edi ecx
	stdcall KernelAlloc, ecx ; Allocate a buffer to put packet into
	pop	ecx edi esi
	test	eax, eax
	jz	.fail

	push	edi
	push	dword .continue_rx
	push	ecx eax
	mov	edi, eax

; update statistics
	inc	[device.packets_rx]

	add	dword [device.bytes_rx], ecx
	adc	dword [device.bytes_rx + 4], 0

; copy packet data
	shr	cx , 1
	jnc	.nb
	movsb
  .nb:
	shr	cx , 1
	jnc	.nw
	movsw
  .nw:
	rep	movsd

	jmp	EthReceiver

    .continue_rx:
	pop	edi

	; free descriptor
	mov	[edi+DES.DES0], DES0_OWN
		
	; next descriptor
	add	edi, DES.size

	inc	[device.rx_crt_des]
	and	[device.rx_crt_des], RX_DES_COUNT-1

	jmp	.loop_rx_start_of_packet

      .end_rx:
      .fail:
	pop	ecx esi ax
      .not_rx:

	jmp	.continue



align 4
write_mac:	; in: mac pushed onto stack (as 3 words)

	DEBUGF	2,"Writing MAC: "

; write data into driver cache
	mov	esi, esp
	lea	edi, [device.mac]
	movsd
	movsw
	add	esp, 6
	
; send setup packet (only if driver is started)
	call	Send_Setup_Packet

align 4
read_mac:

	DEBUGF 1,"Read_mac\n"

	ret



align 4
read_mac_eeprom:

	DEBUGF 1,"Read_mac_eeprom\n"

	lea	edi, [device.mac]
	mov	esi, 20/2		; read words, start address is 20
     .loop:
	push	esi edi
	call	SROM_Read_Word
	pop	edi esi
	stosw
	inc	esi
	cmp	esi, 26/2
	jl	.loop

	DEBUGF	2,"%x-%x-%x-%x-%x-%x\n",[edi-6]:2,[edi-5]:2,[edi-4]:2,[edi-3]:2,[edi-2]:2,[edi-1]:2

	ret

align 4
write_mac_eeprom:

	DEBUGF 1,"Write_mac_eeprom\n"

	ret


align 4
SROM_GetWidth:	; should be 6 or 8 according to some manuals (returns in ecx)

	DEBUGF 1,"SROM_GetWidth\n"

	call	SROM_Idle
	call	SROM_EnterAccessMode

;        set_io  0
;        set_io  CSR9

	; send 110b

	in	eax, dx
	or	eax, CSR9_SROM_DI
	call	SROM_out

	in	eax, dx
	or	eax, CSR9_SROM_DI
	call	SROM_out

	in	eax, dx
	and	eax, not (CSR9_SROM_DI)
	call	SROM_out
	
	mov	ecx,1
  .loop2:
	Bit_Set CSR9_SROM_CK
	SROM_Delay
	
	in	eax, dx
	and	eax, CSR9_SROM_DO
	jnz	.not_zero

	Bit_Clear CSR9_SROM_CK
	SROM_Delay
	jmp	.end_loop2
  .not_zero:
	
	Bit_Clear CSR9_SROM_CK
	SROM_Delay
	
	inc	ecx
	cmp	ecx, 12
	jle	.loop2
  .end_loop2:
	
	DEBUGF 1,"Srom width=%u\n", ecx
	
	call	SROM_Idle
	call	SROM_EnterAccessMode
	call	SROM_Idle
	
	ret


align 4
SROM_out:

	out	dx, eax
	SROM_Delay
	Bit_Set CSR9_SROM_CK
	SROM_Delay
	Bit_Clear CSR9_SROM_CK
	SROM_Delay

	ret



align 4
SROM_EnterAccessMode:

	DEBUGF 1,"SROM_EnterAccessMode\n"

	set_io	0
	set_io	CSR9
	mov	eax, CSR9_SR
	out	dx, eax
	SROM_Delay

	Bit_Set CSR9_RD
	SROM_Delay

	Bit_Clear CSR9_SROM_CK
	SROM_Delay

	Bit_Set CSR9_SROM_CS
	SROM_Delay
	
	ret



align 4
SROM_Idle:

	DEBUGF 1,"SROM_Idle\n"

	call	SROM_EnterAccessMode
	
;        set_io  0
;        set_io  CSR9
	
	mov	ecx, 25
     .loop_clk:

	Bit_Clear CSR9_SROM_CK
	SROM_Delay
	Bit_Set CSR9_SROM_CK
	SROM_Delay
	
	dec	ecx
	jnz	.loop_clk

	
	Bit_Clear CSR9_SROM_CK
	SROM_Delay
	Bit_Clear CSR9_SROM_CS
	SROM_Delay
	
	xor	eax, eax
	out	dx, eax
	
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                      ;;
;; Read serial EEprom word                                              ;;
;;                                                                      ;;
;; In: esi = read address                                               ;;
;; OUT: ax = data word                                                  ;;
;;                                                                      ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
align 4
SROM_Read_Word:

	DEBUGF 1,"SROM_Read_word at: %x result: ", esi

	set_io	0
	set_io	CSR9

; enter access mode
	mov	eax, CSR9_SR + CSR9_RD
	out	dx , eax
	or	eax, CSR9_SROM_CS
	out	dx , eax

	; TODO: change this hard-coded 6-bit stuff to use value from srom_getwidth
	
; send read command "110b" + address to read from
	and	esi, 111111b
	or	esi, 110b shl 6
	
	mov	ecx, 1 shl 9
  .loop_cmd:
	mov	eax, CSR9_SR + CSR9_RD + CSR9_SROM_CS
	test	esi, ecx
	jz	@f
	or	eax, CSR9_SROM_DI
       @@:
	out	dx , eax
	SROM_Delay
	or	eax, CSR9_SROM_CK
	out	dx , eax
	SROM_Delay
	
	shr	ecx, 1
	jnz	.loop_cmd

; read data from SROM

	xor	esi, esi
	mov	ecx, 17 ;;; TODO: figure out why 17, not 16
  .loop_read:
	
	mov	eax, CSR9_SR + CSR9_RD + CSR9_SROM_CS + CSR9_SROM_CK
	out	dx , eax
	SROM_Delay
	
	in	eax, dx
	and	eax, CSR9_SROM_DO
	shr	eax, 3
	shl	esi, 1
	or	esi, eax
	
	mov	eax, CSR9_SR + CSR9_RD + CSR9_SROM_CS
	out	dx , eax
	SROM_Delay
	
	dec	ecx
	jnz	.loop_read
	
	mov	eax, esi

	DEBUGF 1,"%x\n", ax

	ret







;<<<<<<<<<<<<<<<<<<<<<<<<<<<<



;*********************************************************************
;* Media Descriptor Code                                             *
;*********************************************************************

; MII transceiver control section.
; Read and write the MII registers using software-generated serial
; MDIO protocol.  See the MII specifications or DP83840A data sheet
; for details.

; The maximum data clock rate is 2.5 Mhz.  The minimum timing is usually
; met by back-to-back PCI I/O cycles, but we insert a delay to avoid
; "overclocking" issues or future 66Mhz PCI.

; Read and write the MII registers using software-generated serial
; MDIO protocol.  It is just different enough from the EEPROM protocol
; to not share code.  The maxium data clock rate is 2.5 Mhz.

MDIO_SHIFT_CLK		equ	0x10000
MDIO_DATA_WRITE0	equ	0x00000
MDIO_DATA_WRITE1	equ	0x20000
MDIO_ENB		equ	0x00000 	; Ignore the 0x02000 databook setting.
MDIO_ENB_IN		equ	0x40000
MDIO_DATA_READ		equ	0x80000

; MII transceiver control section.
; Read and write the MII registers using software-generated serial
; MDIO protocol.  See the MII specifications or DP83840A data sheet
; for details.

align 4
mdio_read:	; phy_id:edx, location:esi

	DEBUGF	1,"mdio read, phy=%x, location=%x", edx, esi

	shl	edx, 5
	or	esi, edx
	or	esi, 0xf6 shl 10

	set_io	0
	set_io	CSR9

;    if (tp->chip_id == LC82C168) {
;        int i = 1000;
;        outl(0x60020000 + (phy_id<<23) + (location<<18), ioaddr + 0xA0);
;        inl(ioaddr + 0xA0);
;        inl(ioaddr + 0xA0);
;        while (--i > 0)
;            if ( ! ((retval = inl(ioaddr + 0xA0)) & 0x80000000))
;                return retval & 0xffff;
;        return 0xffff;
;    }
;
;    if (tp->chip_id == COMET) {
;        if (phy_id == 1) {
;            if (location < 7)
;                return inl(ioaddr + 0xB4 + (location<<2));
;            else if (location == 17)
;                return inl(ioaddr + 0xD0);
;            else if (location >= 29 && location <= 31)
;                return inl(ioaddr + 0xD4 + ((location-29)<<2));
;        }
;        return 0xffff;
;    }

; Establish sync by sending at least 32 logic ones.

	mov	ecx, 32
  .loop:
	mov	eax, MDIO_ENB or MDIO_DATA_WRITE1
	out	dx, eax
	MDIO_Delay

	or	eax, MDIO_SHIFT_CLK
	out	dx, eax
	MDIO_Delay

	dec	ecx
	jnz	.loop


; Shift the read command bits out.

	mov	ecx, 1 shl 15
  .loop2:
	mov	eax, MDIO_ENB
	test	esi, ecx
	jz	@f
	or	eax, MDIO_DATA_WRITE1
       @@:
	out	dx, eax
	MDIO_Delay

	or	eax, MDIO_SHIFT_CLK
	out	dx, eax
	MDIO_Delay

	shr	ecx, 1
	jnz	.loop2


; Read the two transition, 16 data, and wire-idle bits.

	xor	esi, esi
	mov	ecx, 19
  .loop3:
	mov	eax, MDIO_ENB_IN
	out	dx, eax
	MDIO_Delay

	shl	esi, 1
	in	eax, dx
	test	eax, MDIO_DATA_READ
	jz	@f
	inc	esi
       @@:

	mov	eax, MDIO_ENB_IN or MDIO_SHIFT_CLK
	out	dx, eax
	MDIO_Delay

	dec	ecx
	jnz	.loop3

	shr	esi, 1
	movzx	eax, si

	DEBUGF	1,", data=%x\n", ax

	ret




align 4
mdio_write:	;int phy_id: edx, int location: edi, int value: ax)

	DEBUGF	1,"mdio write, phy=%x, location=%x, data=%x\n", edx, edi, ax

	shl	edi, 18
	or	edi, 0x5002 shl 16
	shl	edx, 23
	or	edi, edx
	mov	di, ax

	set_io	0
	set_io	CSR9

;    if (tp->chip_id == LC82C168) {
;        int i = 1000;
;        outl(cmd, ioaddr + 0xA0);
;        do
;            if ( ! (inl(ioaddr + 0xA0) & 0x80000000))
;                break;
;        while (--i > 0);
;        return;
;    }

;    if (tp->chip_id == COMET) {
;        if (phy_id != 1)
;            return;
;        if (location < 7)
;            outl(value, ioaddr + 0xB4 + (location<<2));
;        else if (location == 17)
;            outl(value, ioaddr + 0xD0);
;        else if (location >= 29 && location <= 31)
;            outl(value, ioaddr + 0xD4 + ((location-29)<<2));
;        return;
;    }


; Establish sync by sending at least 32 logic ones.

	mov	ecx, 32
  .loop:
	mov	eax, MDIO_ENB or MDIO_DATA_WRITE1
	out	dx, eax
	MDIO_Delay

	or	eax, MDIO_SHIFT_CLK
	out	dx, eax
	MDIO_Delay

	dec	ecx
	jnz	.loop


; Shift the command bits out.

	mov	ecx, 1 shl 31
  .loop2:
	mov	eax, MDIO_ENB
	test	edi, ecx
	jz	@f
	or	eax, MDIO_DATA_WRITE1
       @@:
	out	dx, eax
	MDIO_Delay

	or	eax, MDIO_SHIFT_CLK
	out	dx, eax
	MDIO_Delay

	shr	ecx, 1
	jnz	.loop2


; Clear out extra bits.

	mov	ecx, 2
  .loop3:
	mov	eax, MDIO_ENB
	out	dx, eax
	MDIO_Delay

	or	eax, MDIO_SHIFT_CLK
	out	dx, eax
	MDIO_Delay

	dec	ecx
	jnz	.loop3

	ret


; End of code
align 4 					; Place all initialised data here

devices       dd 0
version       dd (DRIVER_VERSION shl 16) or (API_VERSION and 0xFFFF)
my_service    db 'DEC21X4X',0			 ; max 16 chars include zero

include_debug_strings				; All data wich FDO uses will be included here

section '.data' data readable writable align 16 ; place all uninitialized data place here

device_list rd MAX_DEVICES		       ; This list contains all pointers to device structures the driver is handling


