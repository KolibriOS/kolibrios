;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                    ;;
;; Copyright (C) KolibriOS team 2004-2009. All rights reserved.       ;;
;; Distributed under terms of the GNU General Public License          ;;
;;                                                                    ;;
;;  Ethernet driver for KolibriOS                                     ;;
;;  This is an adaptation of MenuetOS driver with minimal changes.    ;;
;;  Changes were made by CleverMouse. Original copyright follows.     ;;
;;                                                                    ;;
;;  This driver is based on the SIS900 driver from                    ;;
;;  the etherboot 5.0.6 project. The copyright statement is           ;;
;;                                                                    ;;
;;          GNU GENERAL PUBLIC LICENSE                                ;;
;;             Version 2, June 1991                                   ;;
;;                                                                    ;;
;;  remaining parts Copyright 2004 Jason Delozier,                    ;;
;;   cordata51@hotmail.com                                            ;;
;;                                                                    ;;
;;  See file COPYING for details                                      ;;
;;                                                                    ;;
;;  Updates:                                                          ;;
;;    Revision Look up table and SIS635 Mac Address by Jarek Pelczar  ;;
;;                                                                    ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; $Revision$

format MS COFF

	API_VERSION		equ 0x01000100

	DEBUG			equ 1
	__DEBUG__		equ 1
	__DEBUG_LEVEL__ 	equ 1

include 'proc32.inc'
include 'imports.inc'
include 'fdo.inc'
include 'netdrv.inc'

public START
public version

NUM_RX_DESC    equ    4 	      ;* Number of RX descriptors *
NUM_TX_DESC    equ    1 	      ;* Number of TX descriptors *
RX_BUFF_SZ	    equ    1520 	   ;* Buffer size for each Rx buffer *
TX_BUFF_SZ	    equ    1516 	   ;* Buffer size for each Tx buffer *
MAX_ETH_FRAME_SIZE  equ    1516

TOTAL_BUFFERS_SIZE equ NUM_RX_DESC*RX_BUFF_SZ + NUM_TX_DESC*TX_BUFF_SZ

virtual at 0
	device:

	ETH_DEVICE

; device specific
      .io_addr		dd ?
      .pci_bus		db ?
      .pci_dev		db ?
      .irq_line 	db ?
      .cur_rx		db ?
      .pci_revision	db ?
      .table_entries:	db ?
align 4
      .special_func:	dd 0
      .txd: times (3 * NUM_TX_DESC) dd 0
      .rxd: times (3 * NUM_RX_DESC) dd 0
      .size:
end virtual

; First page is designated to ETH_DEVICE, buffers start from second
ALLOCATION_SIZE = ((device.size+0FFFh) and not 0FFFh) + TOTAL_BUFFERS_SIZE
; Note that buffers must be contiguous in the physical memory;
; because KernelAlloc allocates contiguous physical pages only in 8-pages blocks,
; align ALLOCATION_SIZE up to 8*(page size) = 8000h
ALLOCATION_SIZE = (ALLOCATION_SIZE + 7FFFh) and not 7FFFh

MAX_DEVICES = 16	; maximum number of devices which this driver can handle


	PCI_HEADER_TYPE 	      equ 0x0e	;8 bit
	PCI_BASE_ADDRESS_0	      equ 0x10	;32 bit
	PCI_BASE_ADDRESS_5	      equ 0x24	;32 bits
	PCI_BASE_ADDRESS_SPACE_IO     equ 0x01
	PCI_VENDOR_ID		      equ 0x00	;16 bit
	PCI_BASE_ADDRESS_IO_MASK      equ 0xFFFFFFFC

section '.flat' code readable align 16

; Driver entry point - register our service when the driver is loading.
; TODO: add needed operations when unloading
START:
	cmp	dword [esp+4], 1
	jne	.exit
	stdcall RegService, my_service, service_proc
	ret	4
.exit:
	xor	eax, eax
	ret	4

; Service procedure for the driver - handle all I/O requests for the driver.
; Currently handled requests are: SRV_GETVERSION = 0 and SRV_HOOK = 1.
service_proc:
; 1. Get parameter from the stack: [esp+4] is the first parameter,
;       pointer to IOCTL structure.
	mov	edx, [esp+4]	; edx -> IOCTL
; 2. Get request code and select a handler for the code.
	mov	eax, [edx+IOCTL.io_code]
	test	eax, eax	; check for SRV_GETVERSION
	jnz	@f
; 3. This is SRV_GETVERSION request, no input, 4 bytes output, API_VERSION.
; 3a. Output size must be at least 4 bytes.
	cmp	[edx+IOCTL.out_size], 4
	jl	.fail
; 3b. Write result to the output buffer.
	mov	eax, [edx+IOCTL.output]
	mov	[eax], dword API_VERSION
; 3c. Return success.
	xor	eax, eax
	ret	4
@@:
	dec	eax	; check for SRV_HOOK
	jnz	.fail
; 4. This is SRV_HOOK request, input defines the device to hook, no output.
; 4a. The driver works only with PCI devices,
;       so input must be at least 3 bytes long.
	cmp	[edx + IOCTL.inp_size], 3
	jl	.fail
; 4b. First byte of input is bus type, 1 stands for PCI.
	mov	eax, [edx + IOCTL.input]
	cmp	byte [eax], 1
	jne	.fail
; 4c. Second and third bytes of the input define the device: bus and dev.
;       Word in bx holds both bytes.
	mov	bx, [eax+1]
; 4d. Check if the device was already hooked,
;       scan through the list of known devices.
	mov	esi, DEV_LIST
	mov	ecx, [NUM_DEV]
	test	ecx, ecx
	jz	.firstdevice
  .nextdevice:
	lodsd
	cmp	bx, word [eax + device.pci_bus]
	je	.find_devicenum
	loop	.nextdevice
; 4e. This device doesn't have its own eth_device structure yet, let's create one
  .firstdevice:
; 4f. Check that we have place for new device.
	cmp	[NUM_DEV], MAX_DEVICES
	jge	.fail
; 4g. Allocate memory for device descriptor and receive+transmit buffers.
	stdcall KernelAlloc, ALLOCATION_SIZE
	test	eax, eax
	jz	.fail
; 4h. Zero the structure.
	push	eax
	mov	edi, eax
	mov	ecx, (device.size + 3) shr 2
	xor	eax, eax
	rep	stosd
	pop	eax
; 4i. Save PCI coordinates, loaded to bx at 4c.
	mov	word [eax+device.pci_bus], bx
	mov	ebx, eax				; ebx is always used as a pointer to the structure (in driver, but also in kernel code)
; 4j. Fill in the direct call addresses into the struct.
; Note that get_MAC pointer is filled in initialization by SIS900_probe.
	mov	dword [ebx+device.reset], sis900_init
	mov	dword [ebx+device.transmit], transmit
;       mov     dword [ebx+device.get_MAC], read_mac
	mov	dword [ebx+device.set_MAC], write_mac
	mov	dword [ebx+device.unload], unload
	mov	dword [ebx+device.name], my_service

; 4k. Now, it's time to find the base io addres of the PCI device
; TODO: implement check if bus and dev exist on this machine

	mov	edx, PCI_BASE_ADDRESS_0
.reg_check:
	push	edx
	stdcall PciRead16, dword [ebx+device.pci_bus], dword [ebx+device.pci_dev], edx
	pop	edx
	test	al, PCI_BASE_ADDRESS_SPACE_IO
	jz	.inc_reg
	and	eax, PCI_BASE_ADDRESS_IO_MASK
	jnz	.got_io

  .inc_reg:
	add	edx, 4
	cmp	edx, PCI_BASE_ADDRESS_5
	jbe	.reg_check
	jmp	.fail

  .got_io:
	mov	[ebx+device.io_addr], eax

; 4l. We've found the io address, find IRQ now
	stdcall PciRead8, dword [ebx+device.pci_bus], dword [ebx+device.pci_dev], 0x3c
	mov	byte [ebx+device.irq_line], al

; 4m. Add new device to the list (required for int_handler).
	mov	eax, [NUM_DEV]
	mov	[DEV_LIST+4*eax], ebx
	inc	[NUM_DEV]

; 4m. Ok, the eth_device structure is ready, let's probe the device

	call	SIS900_probe
	test	eax, eax
	jnz	.destroy
; 4n. If device was successfully initialized, register it for the kernel.

	mov	[device.type], NET_TYPE_ETH
	call	NetRegDev

	cmp	eax, -1
	je	.destroy

	ret	4

; 5. If the device was already loaded, find the device number and return it in eax

  .find_devicenum:
	mov	ebx, eax
	call	NetPtrToNum						; This kernel procedure converts a pointer to device struct in ebx
									; into a device number in edi
	mov	eax, edi						; Application wants it in eax instead
	ret	4

; If an error occured, remove all allocated data and exit (returning -1 in eax)

  .destroy:
	dec	[NUM_DEV]
	; todo: reset device into virgin state

  .err:
	stdcall KernelFree, ebx


  .fail:
	xor	eax, eax
	ret	4


;;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\;;
;;                                                                        ;;
;;        Actual Hardware dependent code starts here                      ;;
;;                                                                        ;;
;;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\;;

unload:
	; TODO: (in this particular order)
	;
	; - Stop the device
	; - Detach int handler
	; - Remove device from local list
	; - call unregister function in kernel
	; - Remove all allocated structures and buffers the card used

	or	eax,-1

ret

;********************************************************************
;   Interface
;      SIS900_reset
;      SIS900_probe
;      SIS900_poll
;      SIS900_transmit
;
;********************************************************************
;********************************************************************
;  Comments:
;    Known to work with the following SIS900 ethernet cards:
;      -  Device ID: 0x0900   Vendor ID: 0x1039   Revision: 0x91
;      -  Device ID: 0x0900   Vendor ID: 0x1039   Revision: 0x90
;
;    If your card is not listed, try it and let me know if it
;    functions properly and it will be aded to the list.  If not
;    we may be able to add support for it.
;
;  ToDo:
;     -  Enable MII interface for reading speed
;        and duplex settings.
;
;     -  Update Poll routine to support packet fragmentation.
;
;     -  Add additional support for other sis900 based cards
;
;********************************************************************

; comment the next line out if you don't want debug info printed
; on the debug board. This option adds a lot of bytes to the driver
; so it's worth to comment it out.
;        SIS900_DEBUG equ 1

SIS900_ETH_ALEN equ	6	;* Size of Ethernet address *
SIS900_ETH_HLEN equ	14	;* Size of ethernet header *
SIS900_ETH_ZLEN equ	60	;* Minimum packet length *
SIS900_DSIZE equ 0x00000fff
SIS900_CRC_SIZE equ 4
SIS900_RFADDR_shift equ 16
;SIS900 Symbolic offsets to registers.
    SIS900_cr		equ	0x0		  ; Command Register
    SIS900_cfg		equ	0x4	  ; Configuration Register
    SIS900_mear     equ     0x8       ; EEPROM Access Register
    SIS900_ptscr    equ     0xc       ; PCI Test Control Register
    SIS900_isr		equ	0x10	  ; Interrupt Status Register
    SIS900_imr		equ	0x14	  ; Interrupt Mask Register
    SIS900_ier		equ	0x18	  ; Interrupt Enable Register
    SIS900_epar 	equ	0x18	  ; Enhanced PHY Access Register
    SIS900_txdp     equ     0x20      ; Transmit Descriptor Pointer Register
    SIS900_txcfg    equ     0x24      ; Transmit Configuration Register
    SIS900_rxdp     equ     0x30      ; Receive Descriptor Pointer Register
    SIS900_rxcfg    equ     0x34      ; Receive Configuration Register
    SIS900_flctrl   equ     0x38      ; Flow Control Register
    SIS900_rxlen    equ     0x3c      ; Receive Packet Length Register
    SIS900_rfcr     equ     0x48      ; Receive Filter Control Register
    SIS900_rfdr     equ     0x4C      ; Receive Filter Data Register
    SIS900_pmctrl   equ     0xB0      ; Power Management Control Register
    SIS900_pmer 	equ	0xB4	  ; Power Management Wake-up Event Register
;SIS900 Command Register Bits
    SIS900_RELOAD	equ	 0x00000400
    SIS900_ACCESSMODE	equ	 0x00000200
    SIS900_RESET	equ	 0x00000100
    SIS900_SWI		equ	 0x00000080
    SIS900_RxRESET	equ	 0x00000020
    SIS900_TxRESET	equ	 0x00000010
    SIS900_RxDIS	equ	 0x00000008
    SIS900_RxENA	equ	 0x00000004
    SIS900_TxDIS	equ	 0x00000002
    SIS900_TxENA	equ	 0x00000001
;SIS900 Configuration Register Bits
    SIS900_DESCRFMT	 equ	0x00000100 ; 7016 specific
    SIS900_REQALG	 equ	0x00000080
    SIS900_SB		 equ	0x00000040
    SIS900_POW		 equ	0x00000020
    SIS900_EXD		 equ	0x00000010
    SIS900_PESEL	 equ	0x00000008
    SIS900_LPM		 equ	0x00000004
    SIS900_BEM		 equ	0x00000001
    SIS900_RND_CNT	 equ	0x00000400
    SIS900_FAIR_BACKOFF  equ	0x00000200
    SIS900_EDB_MASTER_EN equ	0x00002000
;SIS900 Eeprom Access Reigster Bits
    SIS900_MDC	      equ      0x00000040
    SIS900_MDDIR      equ      0x00000020
    SIS900_MDIO       equ      0x00000010  ; 7016 specific
    SIS900_EECS       equ      0x00000008
    SIS900_EECLK      equ      0x00000004
    SIS900_EEDO       equ      0x00000002
    SIS900_EEDI       equ      0x00000001
;SIS900 TX Configuration Register Bits
    SIS900_ATP	      equ      0x10000000 ;Automatic Transmit Padding
    SIS900_MLB	      equ      0x20000000 ;Mac Loopback Enable
    SIS900_HBI	      equ      0x40000000 ;HeartBeat Ignore (Req for full-dup)
    SIS900_CSI	      equ      0x80000000 ;CarrierSenseIgnore (Req for full-du
;SIS900 RX Configuration Register Bits
    SIS900_AJAB       equ      0x08000000 ;
    SIS900_ATX	      equ      0x10000000 ;Accept Transmit Packets
    SIS900_ARP	      equ      0x40000000 ;accept runt packets (<64bytes)
    SIS900_AEP	      equ      0x80000000 ;accept error packets
;SIS900 Interrupt Reigster Bits
    SIS900_WKEVT	   equ	    0x10000000
    SIS900_TxPAUSEEND	   equ	    0x08000000
    SIS900_TxPAUSE	   equ	    0x04000000
    SIS900_TxRCMP	   equ	    0x02000000
    SIS900_RxRCMP	   equ	    0x01000000
    SIS900_DPERR	   equ	    0x00800000
    SIS900_SSERR	   equ	    0x00400000
    SIS900_RMABT	   equ	    0x00200000
    SIS900_RTABT	   equ	    0x00100000
    SIS900_RxSOVR	   equ	    0x00010000
    SIS900_HIBERR	   equ	    0x00008000
    SIS900_SWINT	   equ	    0x00001000
    SIS900_MIBINT	   equ	    0x00000800
    SIS900_TxURN	   equ	    0x00000400
    SIS900_TxIDLE	   equ	    0x00000200
    SIS900_TxERR	   equ	    0x00000100
    SIS900_TxDESC	   equ	    0x00000080
    SIS900_TxOK 	   equ	    0x00000040
    SIS900_RxORN	   equ	    0x00000020
    SIS900_RxIDLE	   equ	    0x00000010
    SIS900_RxEARLY	   equ	    0x00000008
    SIS900_RxERR	   equ	    0x00000004
    SIS900_RxDESC	   equ	    0x00000002
    SIS900_RxOK 	   equ	    0x00000001
;SIS900 Interrupt Enable Reigster Bits
    SIS900_IE	   equ	    0x00000001
;SIS900 Revision ID
	SIS900B_900_REV       equ      0x03
	SIS630A_900_REV       equ      0x80
	SIS630E_900_REV       equ      0x81
	SIS630S_900_REV       equ      0x82
	SIS630EA1_900_REV     equ      0x83
	SIS630ET_900_REV      equ      0x84
	SIS635A_900_REV       equ      0x90
	SIS900_960_REV	      equ      0x91
;SIS900 Receive Filter Control Register Bits
    SIS900_RFEN 	 equ 0x80000000
    SIS900_RFAAB	 equ 0x40000000
    SIS900_RFAAM	 equ 0x20000000
    SIS900_RFAAP	 equ 0x10000000
    SIS900_RFPromiscuous equ 0x70000000
;SIS900 Reveive Filter Data Mask
    SIS900_RFDAT equ  0x0000FFFF
;SIS900 Eeprom Address
    SIS900_EEPROMSignature equ 0x00
    SIS900_EEPROMVendorID  equ 0x02
    SIS900_EEPROMDeviceID  equ 0x03
    SIS900_EEPROMMACAddr   equ 0x08
    SIS900_EEPROMChecksum  equ 0x0b
;The EEPROM commands include the alway-set leading bit.
;SIS900 Eeprom Command
    SIS900_EEread	   equ 0x0180
    SIS900_EEwrite	   equ 0x0140
    SIS900_EEerase	   equ 0x01C0
    SIS900_EEwriteEnable   equ 0x0130
    SIS900_EEwriteDisable  equ 0x0100
    SIS900_EEeraseAll	   equ 0x0120
    SIS900_EEwriteAll	   equ 0x0110
    SIS900_EEaddrMask	   equ 0x013F
    SIS900_EEcmdShift	   equ 16
;For SiS962 or SiS963, request the eeprom software access
	SIS900_EEREQ	equ 0x00000400
	SIS900_EEDONE	equ 0x00000200
	SIS900_EEGNT	equ 0x00000100

SIS900_pci_revision equ ebx+device.pci_revision
sis900_get_mac_func equ ebx+device.get_MAC
sis900_special_func equ ebx+device.special_func
sis900_table_entries equ ebx+device.table_entries
cur_rx equ ebx+device.cur_rx
sys_msg_board_str equ SysMsgBoardStr
;***************************************************************************
;   Function
;      SIS900_probe
;   Description
;      Searches for an ethernet card, enables it and clears the rx buffer
;      If a card was found, it enables the ethernet -> TCPIP link
;not done  - still need to probe mii transcievers
;***************************************************************************
if defined SIS900_DEBUG
SIS900_Debug_Str_Unsupported db 'Sorry your card is unsupported ',13,10,0
end if
SIS900_probe:
;******Wake Up Chip*******
   stdcall PciWrite8, dword [ebx+device.pci_bus], dword [ebx+device.pci_dev], 0x40, 0
;*******Set some PCI Settings*********
   call    SIS900_adjust_pci_device
;*****Get Card Revision******
   stdcall PciRead8, dword [ebx+device.pci_bus], dword [ebx+device.pci_dev], 0x08
   mov [SIS900_pci_revision], al	;save the revision for later use
;****** Look up through the sis900_specific_table
   mov	   esi,sis900_specific_table
.probe_loop:
   cmp	   dword [esi],0		; Check if we reached end of the list
   je	   .probe_loop_failed
   cmp	   al,[esi]			; Check if revision is OK
   je	   .probe_loop_ok
   add	   esi,12			; Advance to next entry
   jmp	   .probe_loop
.probe_loop_failed:
   jmp	   SIS900_Probe_Unsupported
;*********Find Get Mac Function*********
.probe_loop_ok:
   mov	    eax,[esi+4] 	; Get pointer to "get MAC" function
   mov	    [sis900_get_mac_func],eax
   mov	    eax,[esi+8] 	; Get pointer to special initialization fn
   mov	    [sis900_special_func],eax
;******** Get MAC ********
   call     dword [sis900_get_mac_func]
;******** Call special initialization fn if requested ********
   cmp	    dword [sis900_special_func],0
   je	    .no_special_init
   call     dword [sis900_special_func]
.no_special_init:
;******** Set table entries ********
   mov	    al,[SIS900_pci_revision]
   cmp	    al,SIS635A_900_REV
   jae	    .ent16
   cmp	    al,SIS900B_900_REV
   je	    .ent16
   mov	    byte [sis900_table_entries],8
   jmp	    .ent8
.ent16:
   mov	    byte [sis900_table_entries],16
.ent8:
;*******Probe for mii transceiver*******
;TODO!!*********************
;*******Initialize Device*******
   call sis900_init
   ret

SIS900_Probe_Unsupported:
if defined SIS900_DEBUG
   mov	   esi, SIS900_Debug_Str_Unsupported
   call    sys_msg_board_str
end if
   or	   eax, -1
   ret
;***************************************************************************
; Function: sis900_init
;
; Description: resets the ethernet controller chip and various
;    data structures required for sending and receiving packets.
;
; Arguments:
;
; returns:   none
;not done
;***************************************************************************
sis900_init:
   call SIS900_reset		   ;Done
   call SIS900_init_rxfilter   ;Done
   call SIS900_init_txd        ;Done
   call SIS900_init_rxd 	   ;Done
   call SIS900_set_rx_mode     ;done
   call SIS900_set_tx_mode
   ;call SIS900_check_mode
; enable interrupts on packet receive
	xor	eax, eax
	inc	eax	; eax = 1 = SIS900_RxOK
	mov	edx, [ebx+device.io_addr]
	add	edx, SIS900_imr
	out	dx, eax
; globally enable interrupts
	add	edx, SIS900_ier-SIS900_imr
	out	dx, eax ; eax is still 1
	xor	eax, eax
   ret

;***************************************************************************
;   Function
;      SIS900_reset
;   Description
;      disables interrupts and soft resets the controller chip
;
;done+
;***************************************************************************
if defined SIS900_DEBUG
   SIS900_Debug_Reset_Failed db 'Reset Failed ',0
end if
SIS900_reset:
	movzx	eax, [ebx+device.irq_line]
	stdcall AttachIntHandler, eax, int_handler, 0
   push     ebp
   mov	    ebp, [ebx+device.io_addr] ; base address
   ;******Disable Interrupts and reset Receive Filter*******
   xor	    eax, eax		; 0 to initialize
   lea	    edx,[ebp+SIS900_ier]
   out	    dx, eax			; Write 0 to location
   lea	    edx,[ebp+SIS900_imr]
   out	    dx, eax			; Write 0 to location
   lea	    edx,[ebp+SIS900_rfcr]
   out	    dx, eax			; Write 0 to location
   ;*******Reset Card***********************************************
   lea	    edx,[ebp+SIS900_cr]
   in	    eax, dx				; Get current Command Register
   or	    eax, SIS900_RESET		; set flags
   or	    eax, SIS900_RxRESET     ;
   or		eax, SIS900_TxRESET	    ;
   out	    dx, eax				; Write new Command Register
   ;*******Wait Loop************************************************
   push     ebx
   lea	    edx,[ebp+SIS900_isr]
   mov	    ecx, 0x03000000	    ; Status we would like to see from card
   mov	    ebx, 2001		    ; only loop 1000 times
SIS900_Wait:
   dec	    ebx 				    ; 1 less loop
   jz	    SIS900_DoneWait_e		; 1000 times yet?
   in	    eax, dx				    ; move interrup status to eax
   and	    eax, ecx
   xor	    ecx, eax
   jz	    SIS900_DoneWait
   jmp	    SIS900_Wait
SIS900_DoneWait_e:
if defined SIS900_DEBUG
   mov esi, SIS900_Debug_Reset_Failed
   call sys_msg_board_str
end if
SIS900_DoneWait:
   pop	    ebx
   ;*******Set Configuration Register depending on Card Revision********
   lea	    edx,[ebp+SIS900_cfg]
   mov	    eax, SIS900_PESEL		    ; Configuration Register Bit
   mov	    cl, [SIS900_pci_revision]	; card revision
   cmp	    cl, SIS635A_900_REV
   je	    SIS900_RevMatch
   cmp	    cl, SIS900B_900_REV 	; Check card revision
   je	    SIS900_RevMatch
   out	    dx, eax				    ; no revision match
   jmp	    SIS900_Reset_Complete
SIS900_RevMatch:					; Revision match
   or	    eax, SIS900_RND_CNT 	; Configuration Register Bit
   out	    dx, eax
SIS900_Reset_Complete:
   xor	    eax, eax
   pop	    ebp
   ret

;***************************************************************************
; Function: sis_init_rxfilter
;
; Description: sets receive filter address to our MAC address
;
; Arguments:
;
; returns:
;done+
;***************************************************************************
SIS900_init_rxfilter:
   push     ebp
   mov	    ebp, [ebx+device.io_addr]	; base address
   ;****Get Receive Filter Control Register ********
   lea	    edx,[ebp+SIS900_rfcr]
   in	    eax, dx			    ; get register
   push     eax
   ;****disable packet filtering before setting filter*******
   mov	    eax, SIS900_RFEN	;move receive filter enable flag
   not	    eax 			;1s complement
   and	    eax, [esp]			;disable receiver
   out	    dx, eax			;set receive disabled
   ;********load MAC addr to filter data register*********
   xor	    ecx, ecx
SIS900_RXINT_Mac_Write:
   ;high word of eax tells card which mac byte to write
   mov	    eax, ecx
   lea	    edx,[ebp+SIS900_rfcr]
   shl	    eax, 16						;
   out	    dx, eax						;
   lea	    edx,[ebp+SIS900_rfdr]
   mov	    ax,  word [ebx+device.mac+ecx*2] ; Get Mac ID word
   out	    dx, ax						; Send Mac ID
   inc	    cl							; send next word
   cmp	    cl, 3						; more to send?
   jne	    SIS900_RXINT_Mac_Write
   ;********enable packet filitering *****
   pop	    eax 			    ;old register value
   lea	    edx,[ebp+SIS900_rfcr]
   or	    eax, SIS900_RFEN	;enable filtering
   out	    dx, eax		;set register
   pop	    ebp
   ret

;***************************************************************************
;*
;* Function: sis_init_txd
;*
;* Description: initializes the Tx descriptor
;*
;* Arguments:
;*
;* returns:
;*done
;***************************************************************************
SIS900_init_txd:
   ;********** initialize TX descriptor **************
   mov	   [ebx+device.txd], dword 0	   ;put link to next descriptor in link field
   mov	   [ebx+device.txd+4],dword 0	   ;clear status field
   lea	   eax, [ebx+0x1000]
   call    GetPgAddr
   mov	   [ebx+device.txd+8], eax   ;save address to buffer ptr field
   ;*************** load Transmit Descriptor Register ***************
   mov	   edx, [ebx+device.io_addr]	    ; base address
   add	   edx, SIS900_txdp	 ; TX Descriptor Pointer
   add	   eax, device.txd - 0x1000	   ; First Descriptor
   out	   dx, eax			       ; move the pointer
   ret

;***************************************************************************
;* Function: sis_init_rxd
;*
;* Description: initializes the Rx descriptor ring
;*
;* Arguments:
;*
;* Returns:
;*done
;***************************************************************************
SIS900_init_rxd:
   xor	    ecx,ecx
   mov	    [cur_rx], cl					;Set cuurent rx discriptor to 0
   mov	    eax, ebx
   call     GetPgAddr
   mov	    esi, eax
   ;******** init RX descriptors ********
SIS900_init_rxd_Loop:
    mov     edx, ecx					    ;current descriptor
    imul    edx, 12			    ;
    mov     eax, ecx					    ;determine next link descriptor
    inc     eax 			    ;
    cmp     eax, NUM_RX_DESC		    ;
    jne     SIS900_init_rxd_Loop_0	    ;
    xor     eax, eax			    ;
SIS900_init_rxd_Loop_0: 		   ;
    imul    eax, 12			    ;
    lea     eax, [eax+esi+device.rxd]
    mov     [ebx+device.rxd+edx], eax					   ;save link to next descriptor
    mov     [ebx+device.rxd+edx+4],dword RX_BUFF_SZ	   ;status bits init to buf size
    mov     eax, ecx						;find where the buf is located
    imul    eax,RX_BUFF_SZ		    ;
    lea     eax, [eax+esi+0x1000+NUM_TX_DESC*TX_BUFF_SZ]
    mov     [ebx+device.rxd+edx+8], eax 			   ;save buffer pointer
    inc     ecx 						    ;next descriptor
    cmp     ecx, NUM_RX_DESC		    ;
    jne     SIS900_init_rxd_Loop	    ;
    ;********* load Receive Descriptor Register with address of first
    ; descriptor*********
    mov     edx, [ebx+device.io_addr]
    add     edx, SIS900_rxdp
    lea     eax, [esi+device.rxd]
    out     dx, eax
    ret

;***************************************************************************
;* Function: sis900_set_tx_mode
;*
;* Description:
;*    sets the transmit mode to allow for full duplex
;*
;*
;* Arguments:
;*
;* Returns:
;*
;* Comments:
;*     If you are having problems transmitting packet try changing the
;*     Max DMA Burst, Possible settings are as follows:
;*         0x00000000 = 512 bytes
;*         0x00100000 = 4 bytes
;*         0x00200000 = 8 bytes
;*         0x00300000 = 16 bytes
;*         0x00400000 = 32 bytes
;*         0x00500000 = 64 bytes
;*         0x00600000 = 128 bytes
;*         0x00700000 = 256 bytes
;***************************************************************************
SIS900_set_tx_mode:
   push     ebp
   mov	    ebp,[ebx+device.io_addr]
   lea	    edx,[ebp+SIS900_cr]
   in	    eax, dx			    ; Get current Command Register
   or	    eax, SIS900_TxENA	;Enable Receive
   out	    dx, eax
   lea	    edx,[ebp+SIS900_txcfg]; Transmit config Register offset
   mov	    eax, SIS900_ATP		;allow automatic padding
   or	    eax, SIS900_HBI		;allow heartbeat ignore
   or	    eax, SIS900_CSI		;allow carrier sense ignore
   or	    eax, 0x00600000	;Max DMA Burst
   or	    eax, 0x00000100	;TX Fill Threshold
   or	    eax, 0x00000020	;TX Drain Threshold
   out	    dx, eax
   pop	    ebp
   ret

;***************************************************************************
;* Function: sis900_set_rx_mode
;*
;* Description:
;*    sets the receive mode to accept all broadcast packets and packets
;*    with our MAC address, and reject all multicast packets.  Also allows
;*    full-duplex
;*
;* Arguments:
;*
;* Returns:
;*
;* Comments:
;*     If you are having problems receiving packet try changing the
;*     Max DMA Burst, Possible settings are as follows:
;*         0x00000000 = 512 bytes
;*         0x00100000 = 4 bytes
;*         0x00200000 = 8 bytes
;*         0x00300000 = 16 bytes
;*         0x00400000 = 32 bytes
;*         0x00500000 = 64 bytes
;*         0x00600000 = 128 bytes
;*         0x00700000 = 256 bytes
;***************************************************************************
SIS900_set_rx_mode:
   push     ebp
   mov	    ebp,[ebx+device.io_addr]
    ;**************update Multicast Hash Table in Receive Filter
   xor	    cl, cl
SIS900_set_rx_mode_Loop:
   mov	    eax, ecx
   shl	    eax, 1
   lea	    edx,[ebp+SIS900_rfcr]	    ; Receive Filter Control Reg offset
   mov	    eax, 4					    ;determine table entry
   add	    al, cl
   shl	    eax, 16
   out	    dx, eax					    ;tell card which entry to modify
   lea	    edx,[ebp+SIS900_rfdr]	    ; Receive Filter Control Reg offset
   mov	    eax, 0xffff 			    ;entry value
   out	    dx, ax					    ;write value to table in card
   inc	    cl						    ;next entry
   cmp	    cl,[sis900_table_entries]	;
   jl	    SIS900_set_rx_mode_Loop
   ;*******Set Receive Filter Control Register*************
   lea	    edx,[ebp+SIS900_rfcr]	; Receive Filter Control Register offset
   mov	    eax, SIS900_RFAAB		;accecpt all broadcast packets
   or	    eax, SIS900_RFAAM		;accept all multicast packets
   or	    eax, SIS900_RFAAP		;Accept all packets
   or	    eax, SIS900_RFEN		;enable receiver filter
   out	    dx, eax
   ;******Enable Receiver************
   lea	    edx,[ebp+SIS900_cr] ; Command Register offset
   in	    eax, dx			    ; Get current Command Register
   or	    eax, SIS900_RxENA	;Enable Receive
   out	    dx, eax
   ;*********Set
   lea	    edx,[ebp+SIS900_rxcfg]	; Receive Config Register offset
   mov	    eax, SIS900_ATX			;Accept Transmit Packets
				    ; (Req for full-duplex and PMD Loopback)
   or	    eax, 0x00600000			;Max DMA Burst
   or	    eax, 0x00000002			;RX Drain Threshold, 8X8 bytes or 64bytes
   out	    dx, eax					;
   pop	    ebp
   ret

;***************************************************************************
; *     SIS960_get_mac_addr: - Get MAC address for SiS962 or SiS963 model
; *     @pci_dev: the sis900 pci device
; *     @net_dev: the net device to get address for
; *
; *     SiS962 or SiS963 model, use EEPROM to store MAC address. And EEPROM
; *     is shared by
; *     LAN and 1394. When access EEPROM, send EEREQ signal to hardware first
; *     and wait for EEGNT. If EEGNT is ON, EEPROM is permitted to be access
; *     by LAN, otherwise is not. After MAC address is read from EEPROM, send
; *     EEDONE signal to refuse EEPROM access by LAN.
; *     The EEPROM map of SiS962 or SiS963 is different to SiS900.
; *     The signature field in SiS962 or SiS963 spec is meaningless.
; *     MAC address is read into @net_dev->dev_addr.
; *done
;*
;* Return 0 is EAX = failure
;*Done+
;***************************************************************************
if defined SIS900_DEBUG
SIS900_Debug_Str_GetMac_Start db 'Attempting to get SIS900 Mac ID: ',13,10,0
SIS900_Debug_Str_GetMac_Failed db 'Access to EEprom Failed',13,10,0
SIS900_Debug_Str_GetMac_Address db 'Your Mac ID is: ',0
SIS900_Debug_Str_GetMac_Address2 db 'Your SIS96x Mac ID is: ',0
end if
SIS960_get_mac_addr:
   push     ebp
   mov	    ebp,[ebx+device.io_addr]
   ;**********Send Request for eeprom access*********************
   lea	    edx,[ebp+SIS900_mear]		; Eeprom access register
   mov	    eax, SIS900_EEREQ			; Request access to eeprom
   out	    dx, eax						; Send request
   xor	    ecx,ecx						;
   ;******Loop 4000 times and if access not granted error out*****
SIS96X_Get_Mac_Wait:
   in	    eax, dx					;get eeprom status
   and	    eax, SIS900_EEGNT	    ;see if eeprom access granted flag is set
   jnz	    SIS900_Got_EEP_Access	;if it is, go access the eeprom
   inc	    ecx 					;else keep waiting
   cmp	    ecx, 4000				;have we tried 4000 times yet?
   jl	    SIS96X_Get_Mac_Wait     ;if not ask again
   xor	    eax, eax		    ;return zero in eax indicating failure
   ;*******Debug **********************
if defined SIS900_DEBUG
   mov esi,SIS900_Debug_Str_GetMac_Failed
   call sys_msg_board_str
end if
   jmp SIS960_get_mac_addr_done
   ;**********EEprom access granted, read MAC from card*************
SIS900_Got_EEP_Access:
    ; zero based so 3-16 bit reads will take place
   mov	    ecx, 2
SIS96x_mac_read_loop:
   mov	    eax, SIS900_EEPROMMACAddr	 ;Base Mac Address
   add	    eax, ecx				     ;Current Mac Byte Offset
   push     ecx
   call     sis900_read_eeprom		 ;try to read 16 bits
   pop	    ecx
   mov	    word [ebx+device.mac+ecx*2], ax	   ;save 16 bits to the MAC ID varible
   dec	    ecx 			 ;one less word to read
   jns	    SIS96x_mac_read_loop	 ;if more read more
   mov	    eax, 1			 ;return non-zero indicating success
   ;*******Debug Print MAC ID to debug window**********************
if defined SIS900_DEBUG
   mov esi,SIS900_Debug_Str_GetMac_Address2
   call sys_msg_board_str
   lea edx, [ebx+device.mac]
   call Create_Mac_String
end if
   ;**********Tell EEPROM We are Done Accessing It*********************
SIS960_get_mac_addr_done:
   lea	    edx,[ebp+SIS900_mear]		; Eeprom access register
   mov	    eax, SIS900_EEDONE		 ;tell eeprom we are done
   out	    dx,eax
   pop	    ebp
   ret
;***************************************************************************
;*      sis900_get_mac_addr: - Get MAC address for stand alone SiS900 model
;*      @pci_dev: the sis900 pci device
;*      @net_dev: the net device to get address for
;*
;*      Older SiS900 and friends, use EEPROM to store MAC address.
;*      MAC address is read from read_eeprom() into @net_dev->dev_addr.
;* done/untested
;***************************************************************************
SIS900_get_mac_addr:
   ;*******Debug **********************
if defined SIS900_DEBUG
   mov esi,SIS900_Debug_Str_GetMac_Start
   call sys_msg_board_str
end if
   ;******** check to see if we have sane EEPROM *******
   mov	    eax, SIS900_EEPROMSignature  ;Base Eeprom Signature
   call     sis900_read_eeprom		 ;try to read 16 bits
   cmp ax, 0xffff
   je SIS900_Bad_Eeprom
   cmp ax, 0
   je SIS900_Bad_Eeprom
   ;**************Read MacID**************
   ; zero based so 3-16 bit reads will take place
   mov	    ecx, 2
SIS900_mac_read_loop:
   mov	    eax, SIS900_EEPROMMACAddr	 ;Base Mac Address
   add	    eax, ecx				     ;Current Mac Byte Offset
   push     ecx
   call     sis900_read_eeprom		 ;try to read 16 bits
   pop	    ecx
   mov	    word [ebx+device.mac+ecx*2], ax	   ;save 16 bits to the MAC ID storage
   dec	    ecx 			 ;one less word to read
   jns	    SIS900_mac_read_loop	 ;if more read more
   mov	    eax, 1			 ;return non-zero indicating success
   ;*******Debug Print MAC ID to debug window**********************
if defined SIS900_DEBUG
   mov esi,SIS900_Debug_Str_GetMac_Address
   call sys_msg_board_str
   lea edx, [ebx+device.mac]
   call Create_Mac_String
end if
   ret

SIS900_Bad_Eeprom:
   xor eax, eax
   ;*******Debug **********************
if defined SIS900_DEBUG
   mov esi,SIS900_Debug_Str_GetMac_Failed
   call sys_msg_board_str
end if
   ret
;***************************************************************************
;*      Get_Mac_SIS635_900_REV: - Get MAC address for model 635
;*
;*
;***************************************************************************
Get_Mac_SIS635_900_REV:
if defined SIS900_DEBUG
    mov     esi,SIS900_Debug_Str_GetMac_Start
    call    sys_msg_board_str
end if
    push    ebp
    mov     ebp,[ebx+device.io_addr]
    lea     edx,[ebp+SIS900_rfcr]
    in	    eax,dx
    mov     edi,eax ; EDI=rfcrSave
    lea     edx,[ebp+SIS900_cr]
    or	    eax,SIS900_RELOAD
    out     dx,eax
    xor     eax,eax
    out     dx,eax
    ; Disable packet filtering before setting filter
    lea     edx,[ebp+SIS900_rfcr]
    mov     eax,edi
    and     edi,not SIS900_RFEN
    out     dx,eax
    ; Load MAC to filter data register
    xor     ecx,ecx
    lea     esi,[ebx+device.mac]
.get_mac_loop:
    lea     edx,[ebp+SIS900_rfcr]
    mov     eax,ecx
    shl     eax,SIS900_RFADDR_shift
    out     dx,eax
    lea     edx,[ebp+SIS900_rfdr]
    in	    eax,dx
    mov     [esi],ax
    add     esi,2
    inc     ecx
    cmp     ecx,3
    jne .get_mac_loop
    ; Enable packet filtering
    ;lea     edx,[ebp+SIS900_rfcr]
    ;mov     eax,edi
    ;or      eax,SIS900_RFEN
    ;out     dx, eax
   ;*******Debug Print MAC ID to debug window**********************
if defined SIS900_DEBUG
    mov     esi,SIS900_Debug_Str_GetMac_Address
    call    sys_msg_board_str
    lea     edx, [ebx+device.mac]
    call    Create_Mac_String
end if
    pop     ebp
    ret
;***************************************************************************
;* Function: sis900_read_eeprom
;*
;* Description: reads and returns a given location from EEPROM
;*
;* Arguments: eax - location:       requested EEPROM location
;*
;* Returns:   eax :                contents of requested EEPROM location
;*
; Read Serial EEPROM through EEPROM Access Register, Note that location is
;   in word (16 bits) unit */
;done+
;***************************************************************************
sis900_read_eeprom:
   push      esi
   push      edx
   push      ecx
   push      ebx
   push      ebp
   mov	     ebp,[ebx+device.io_addr]
   mov	     ebx, eax		   ;location of Mac byte to read
   or	     ebx, SIS900_EEread    ;
   lea	     edx,[ebp+SIS900_mear] ; Eeprom access register
   xor	     eax, eax		   ; start send
   out	     dx,eax
   call      SIS900_Eeprom_Delay_1
   mov	     eax, SIS900_EECLK
   out	     dx, eax
   call      SIS900_Eeprom_Delay_1
    ;************ Shift the read command (9) bits out. *********
   mov	     cl, 8					;
sis900_read_eeprom_Send:
   mov	     eax, 1
   shl	     eax, cl
   and	     eax, ebx
   jz SIS900_Read_Eeprom_8
   mov	     eax, 9
   jmp	     SIS900_Read_Eeprom_9
SIS900_Read_Eeprom_8:
   mov	     eax, 8
SIS900_Read_Eeprom_9:
   out	     dx, eax
   call      SIS900_Eeprom_Delay_1
   or	     eax, SIS900_EECLK
   out	     dx, eax
   call      SIS900_Eeprom_Delay_1
   cmp	     cl, 0
   je	     sis900_read_eeprom_Send_Done
   dec	     cl
   jmp	     sis900_read_eeprom_Send
   ;*********************
sis900_read_eeprom_Send_Done:
   mov	     eax, SIS900_EECS		;
   out	     dx, eax
   call      SIS900_Eeprom_Delay_1
    ;********** Read 16-bits of data in ***************
    mov      cx, 16				;16 bits to read
sis900_read_eeprom_Send2:
    mov      eax, SIS900_EECS
    out      dx, eax
    call     SIS900_Eeprom_Delay_1
    or	     eax, SIS900_EECLK
    out      dx, eax
    call     SIS900_Eeprom_Delay_1
    in	     eax, dx
    shl      ebx, 1
    and      eax, SIS900_EEDO
    jz	     SIS900_Read_Eeprom_0
    or	     ebx, 1
SIS900_Read_Eeprom_0:
   dec	     cx
   jnz	     sis900_read_eeprom_Send2
   ;************** Terminate the EEPROM access. **************
   xor	     eax, eax
   out	     dx, eax
   call      SIS900_Eeprom_Delay_1
   mov	     eax, SIS900_EECLK
   out	     dx, eax
   mov	     eax, ebx
   and	     eax, 0x0000ffff			;return only 16 bits
   pop	     ebp
   pop	     ebx
   pop	     ecx
   pop	     edx
   pop	     esi
   ret
;***************************************************************************
;   Function
;      SIS900_Eeprom_Delay_1
;   Description
;
;
;
;
;***************************************************************************
SIS900_Eeprom_Delay_1:
   push eax
   in eax, dx
   pop eax
   ret

write_mac:
	DEBUGF 1,'Setting MAC is not supported for SIS900 card.\n'
	add	esp, 6
	ret

;***************************************************************************
;   Function
;      int_handler
;   Description
;      handles received IRQs, which signal received packets
;
;  Currently only supports one descriptor per packet, if packet is fragmented
;  between multiple descriptors you will lose part of the packet
;***************************************************************************
if defined SIS900_DEBUG
SIS900_Debug_Pull_Packet_good db 'Good Packet Waiting: ',13,10,0
SIS900_Debug_Pull_Bad_Packet_Status db 'Bad Packet Waiting: Status',13,10,0
SIS900_Debug_Pull_Bad_Packet_Size db 'Bad Packet Waiting: Size',13,10,0
end if
int_handler:
; find pointer of device which made IRQ occur
	mov	esi, DEV_LIST
	mov	ecx, [NUM_DEV]
	test	ecx, ecx
	jz	.nothing
.nextdevice:
	mov	ebx, [esi]
	mov	edx, [ebx+device.io_addr]
	add	edx, SIS900_isr
	in	eax, dx ; note that this clears all interrupts
	test	al, SIS900_RxOK
	jnz	.got_it
	loop	.nextdevice
.nothing:
	ret
.got_it:
    ;**************Get Status **************
    movzx     eax, [ebx+device.cur_rx]		;find current discriptor
    imul      eax, 12		    ;
    mov       ecx, [ebx+device.rxd+eax+4]	   ; get receive status
    ;**************Check Status **************
    ;Check RX_Status to see if packet is waiting
    test      ecx, 0x80000000
    jnz       SIS900_poll_IS_packet
    ret
   ;**********There is a packet waiting check it for errors**************
SIS900_poll_IS_packet:
    test      ecx, 0x67C0000		;see if there are any errors
    jnz       SIS900_Poll_Error_Status
   ;**************Check size of packet*************
   and	     ecx, SIS900_DSIZE					;get packet size minus CRC
   cmp	     ecx, SIS900_CRC_SIZE
   ;make sure packet contains data
   jle	     SIS900_Poll_Error_Size
   ;*******Copy Good Packet to receive buffer******
   sub	    ecx, SIS900_CRC_SIZE			     ;dont want crc
   ; update statistics
   inc	    dword [ebx+device.packets_rx]
   add	    dword [ebx+device.bytes_rx], ecx
   adc	    dword [ebx+device.bytes_rx+4], 0
   push     ecx
   stdcall  KernelAlloc, ecx
   pop	    ecx
   test     eax, eax
   jz	    int_handler.nothing
   push     ebx
   push     .return ; return address for EthReceiver
   ;**********Continue copying packet****************
   push     ecx eax ; save buffer pointer and size for EthReceiver
   mov	    edi, eax
   movzx    esi, byte [ebx+device.cur_rx]
   imul     esi, RX_BUFF_SZ
   lea	    esi, [esi+ebx+0x1000+NUM_TX_DESC*TX_BUFF_SZ]
   ; first copy dword-wise, divide size by 4
   shr	    ecx, 2
   rep	    movsd							; copy the dwords
   mov	    ecx, [esp+4]
   and	    ecx, 3						    ;
   rep	    movsb
   ;********Debug, tell user we have a good packet*************
if defined SIS900_DEBUG
   mov	    esi, SIS900_Debug_Pull_Packet_good
   call     sys_msg_board_str
end if
   jmp	    EthReceiver
.return:
   pop	    ebx
   jmp	    SIS900_Poll_Cnt
   ;*************Error occured let user know through debug window***********
SIS900_Poll_Error_Status:
if defined SIS900_DEBUG
		mov	 esi, SIS900_Debug_Pull_Bad_Packet_Status
		call	 sys_msg_board_str
end if
		jmp	 SIS900_Poll_Cnt
SIS900_Poll_Error_Size:
if defined SIS900_DEBUG
		mov	 esi, SIS900_Debug_Pull_Bad_Packet_Size
		call	 sys_msg_board_str
end if
   ;*************Increment to next available descriptor**************
SIS900_Poll_Cnt:
    ;Reset status, allow ethernet card access to descriptor
   movzx    eax, [ebx+device.cur_rx]
   lea	    eax, [eax*3]
   mov	    ecx, RX_BUFF_SZ
   mov	    [ebx+device.rxd+eax*4+4], ecx		 ;
   inc	    [ebx+device.cur_rx] 			 ;get next descriptor
   and	    [ebx+device.cur_rx],NUM_RX_DESC-1		 ;only 4 descriptors 0-3
   ;******Enable Receiver************
   mov	    edx, [ebx+device.io_addr]
   add	    edx, SIS900_cr ; Command Register offset
   in	    eax, dx			    ; Get current Command Register
   or	    eax, SIS900_RxENA	;Enable Receive
   out	    dx, eax
   ret
;***************************************************************************
;   Function
;      transmit
;   Description
;      Transmits a packet of data via the ethernet card
;         buffer pointer in [esp+4]
;         size of buffer in [esp+8]
;         pointer to device structure in ebx
;
;      only one transmit descriptor is used
;
;***************************************************************************
if defined SIS900_DEBUG
SIS900_Debug_Transmit_Packet db 'Transmitting Packet: ',13,10,0
SIS900_Debug_Transmit_Packet_Err db 'Transmitting Packet Error: ',13,10,0
end if
str1 db 'Transmitting packet:',13,10,0
str2 db ' ',0
transmit:
   cmp	    dword [esp+8], MAX_ETH_FRAME_SIZE
   jg	    transmit_finish
   cmp	    dword [esp+8], 60
   jl	    transmit_finish
   push     ebp
   mov	    ebp, [ebx+device.io_addr] ; Base Address
   ;******** Stop the transmitter ********
   lea	    edx,[ebp+SIS900_cr] ; Command Register offset
   in	    eax, dx			    ; Get current Command Register
   or	    eax, SIS900_TxDIS	; Disable Transmitter
   out	    dx, eax
   ;*******load Transmit Descriptor Register *******
   lea	    edx,[ebp+SIS900_txdp]
   mov	    eax, ebx
   call     GetPgAddr
   add	    eax, device.txd
   out	    dx, eax
   ;******* copy packet to descriptor*******
   mov	   esi, [esp+8]
   lea	   edi, [ebx+0x1000]
   mov	   ecx, [esp+12]
   mov	   edx, ecx
   shr	   ecx, 2
   and	   edx, 3
   rep	   movsd
   mov	   ecx, edx
   rep	   movsb
   ;**************set length tag**************
   mov	   ecx, [esp+12]		  ;restore packet size
   and	   ecx, SIS900_DSIZE	 ;
   inc	    [ebx+device.packets_tx]
   add	    dword [ebx+device.bytes_tx], ecx
   adc	    dword [ebx+device.bytes_tx+4], 0
   ;**************pad to minimum packet size **************not needed
   ;cmp       ecx, SIS900_ETH_ZLEN
   ;jge       SIS900_transmit_Size_Ok
   ;push      ecx
   ;mov       ebx, SIS900_ETH_ZLEN
   ;sub       ebx, ecx
   ;mov       ecx, ebx
   ;rep       movsb
   ;pop       ecx
SIS900_transmit_Size_Ok:
   or	    ecx, 0x80000000				;card owns descriptor
   mov	    [ebx+device.txd+4], ecx
if defined SIS900_DEBUG
   mov	    esi, SIS900_Debug_Transmit_Packet
   call     sys_msg_board_str
end if
   ;***************restart the transmitter ********
   lea	    edx,[ebp+SIS900_cr]
   in	    eax, dx			    ; Get current Command Register
   or	    eax, SIS900_TxENA	; Enable Transmitter
   out	    dx, eax
   ;****make sure packet transmitted successfully****
;   mov      esi,10
;   call     delay_ms
   mov	    eax, [ebx+device.txd+4]
   and	    eax, 0x6200000
   jz	    SIS900_transmit_OK
   ;**************Tell user there was an error through debug window
if defined SIS900_DEBUG
   mov	    esi, SIS900_Debug_Transmit_Packet_Err
   call     sys_msg_board_str
end if
SIS900_transmit_OK:
   pop	    ebp
transmit_finish:
   ret

;***************************************************************************
;* Function: Create_Mac_String
;*
;* Description: Converts the 48 bit value to a string for display
;*
;* String Format: XX:XX:XX:XX:XX:XX
;*
;* Arguments: node_addr is location of 48 bit MAC ID
;*
;* Returns:   Prints string to general debug window
;*
;*
;done
;***************************************************************************
if defined SIS900_DEBUG

SIS900_Char_String    db '0','1','2','3','4','5','6','7','8','9'
		      db 'A','B','C','D','E','F'
Mac_str_build: times 20 db 0
Create_Mac_String:
   pusha
   xor ecx, ecx
Create_Mac_String_loop:
   mov al,byte [edx+ecx];[node_addr+ecx]
   push eax
   shr eax, 4
   and eax, 0x0f
   mov bl, byte [SIS900_Char_String+eax]
   mov [Mac_str_build+ecx*3], bl
   pop eax
   and eax, 0x0f
   mov bl, byte [SIS900_Char_String+eax]
   mov [Mac_str_build+1+ecx*3], bl
   cmp ecx, 5
   je Create_Mac_String_done
   mov bl, ':'
   mov [Mac_str_build+2+ecx*3], bl
   inc ecx
   jmp Create_Mac_String_loop
Create_Mac_String_done: 				;Insert CR and Zero Terminate
   mov [Mac_str_build+2+ecx*3],byte 13
   mov [Mac_str_build+3+ecx*3],byte 10
   mov [Mac_str_build+4+ecx*3],byte 0
   mov esi, Mac_str_build
   call sys_msg_board_str				;Print String to message board
   popa
   ret
end if
;***************************************************************************
;*      Set device to be a busmaster in case BIOS neglected to do so.
;*      Also adjust PCI latency timer to a reasonable value, 64.
;***************************************************************************
SIS900_adjust_pci_device:
   ;*******Get current setting************************
   stdcall PciRead16, dword [ebx+device.pci_bus], dword [ebx+device.pci_dev], 0x04
   ;******see if its already set as bus master********
   mov	    cx, ax
   and	    cx,5
   cmp	    cx,5
   je	    SIS900_adjust_pci_device_Latency
   ;******Make card a bus master*******
   mov	    cx, ax				;value to write
   or	    cx,5
   stdcall PciWrite16, dword [ebx+device.pci_bus], dword [ebx+device.pci_dev], 0x04, ecx
   ;******Check latency setting***********
SIS900_adjust_pci_device_Latency:
   ;*******Get current latency setting************************
   stdcall PciRead8, dword [ebx+device.pci_bus], dword [ebx+device.pci_dev], 0x0D
   ;******see if its aat least 64 clocks********
   cmp	    al,64
   jge	    SIS900_adjust_pci_device_Done
   ;******Set latency to 32 clocks*******
   stdcall PciWrite8, dword [ebx+device.pci_bus], dword [ebx+device.pci_dev], 0x0D, 64
   ;******Check latency setting***********
SIS900_adjust_pci_device_Done:
   ret

; End of code

align 4 					; Place all initialised data here

NUM_DEV   dd 0

sis900_specific_table:
;    dd SIS630A_900_REV,Get_Mac_SIS630A_900_REV,0
;    dd SIS630E_900_REV,Get_Mac_SIS630E_900_REV,0
    dd SIS630S_900_REV,Get_Mac_SIS635_900_REV,0
    dd SIS630EA1_900_REV,Get_Mac_SIS635_900_REV,0
    dd SIS630ET_900_REV,Get_Mac_SIS635_900_REV,0;SIS630ET_900_REV_SpecialFN
    dd SIS635A_900_REV,Get_Mac_SIS635_900_REV,0
    dd SIS900_960_REV,SIS960_get_mac_addr,0
    dd SIS900B_900_REV,SIS900_get_mac_addr,0
    dd 0,0,0,0 ; end of list

version       dd (5 shl 16) or (API_VERSION and 0xFFFF)
my_service    db 'SIS900',0			; max 16 chars include zero

include_debug_strings				; All data wich FDO uses will be included here

section '.data' data readable writable align 16 ; place all uninitialized data place here

DEV_LIST rd MAX_DEVICES 		; This list contains all pointers to device structures the driver is handling

