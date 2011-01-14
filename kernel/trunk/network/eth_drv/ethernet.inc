;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2008. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  ETHERNET.INC                                                   ;;
;;                                                                 ;;
;;  Ethernet network layer for Menuet OS                           ;;
;;                                                                 ;;
;;  This file contains the following:                              ;;
;;      PCI bus scanning for valid devices                         ;;
;;      Table of supported ethernet drivers                        ;;
;;      Code to identify and activate a supported driver           ;;
;;      ARP handler                                                ;;
;;      Driver interface to the IP layer                           ;;
;;      Gateway support                                            ;;
;;                                                                 ;;
;;  Individual driver files are included here                      ;;
;;                                                                 ;;
;;  The PCI bus scanning code was ported from the etherboot        ;;
;;  5.0.6 project. The copyright statement for that code is        ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;  remaining parts Copyright 2002 Mike Hibbett                    ;;
;;   mikeh@oceanfree.net                                           ;;
;;                                                                 ;;
;;  See file COPYING for details                                   ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


;********************************************************************
;   Interface
;      ethernet_driver   called by stack_handler in stack.inc
;      eth_probe         called by app_stack_handler in stack.inc
;
;********************************************************************

ETHER_IP		    equ     0x0008	; Reversed from 0800 for intel
ETHER_ARP		    equ     0x0608	; Reversed from 0806 for intel
ETHER_RARP		    equ     0x3580

struc ETH_FRAME
{  .DstMAC	 dp   ?  ;destination MAC-address [6 bytes]
   .SrcMAC	 dp   ?  ;source MAC-address [6 bytes]
   .Type	 dw   ?  ;type of the upper-layer protocol [2 bytes]
   .Data	 db   ?  ;data [46-1500 bytes]
}

virtual at Ether_buffer
  ETH_FRAME ETH_FRAME
end virtual


; Some useful information on data structures

;     Ethernet Packet - ARP Request example
;
;   0                   1                   2                   3
;   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
;
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;   |       Dest   H/W Address                                      |
;   |                    ( 14 byte header )                         |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;   |                               |     Source     H/W Address    |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;   |                                                               |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;   |    Protocol - ARP 08  06      |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;   |  H/W Type  00           01    |  Protocol Type   08 00        |
;   |                   ( ARP Request packet )                      |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;   | HLen    0x06  | PLen    0x04  |    OpCode        00   01      |
;   |               ( 0001 for request, 0002 for reply )            |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;   | Source Hardware Address ( MAC Address )                       |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;   |                               |  Source IP Address            |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;   |                               | Destination Hardware Address  |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;   |                                                               |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;   | Destination IP Address                                        |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

; Include individual drivers source files at this point.
; If you create a new driver, include it below.

include "drivers/rtl8029.inc"
include "drivers/i8255x.inc"
include "drivers/rtl8139.inc"
include "drivers/3c59x.inc"
include "drivers/sis900.inc"
include "drivers/pcnet32.inc"
include "drivers/rtl8169.inc"
include "drivers/forcedeth.inc"

; PCICards
; ========
; PCI vendor and hardware types for hardware supported by the above drivers
; If you add a driver, ensure you update this datastructure, otherwise the
; card will not be probed.
; Each driver is defined by 4 double words. These are
;   PCIVendorDevice  probeFunction ResetFunction PollFunction transmitFunction
; The last entry must be kept at all zeros, to indicate the end of the list
; As a PCI driver may support more than one hardware implementation, there may
; be several lines which refer to the same functions.
; The first driver found on the PCI bus will be the one used.

PCICARDS_ENTRY_SIZE	    equ     24	  ; Size of each PCICARDS entry

iglobal
PCICards:
dd  0x12098086, I8255x_probe, I8255x_reset, I8255x_poll, I8255x_transmit, 0
dd  0x10298086, I8255x_probe, I8255x_reset, I8255x_poll, I8255x_transmit, 0
dd  0x12298086, I8255x_probe, I8255x_reset, I8255x_poll, I8255x_transmit, 0
dd  0x10308086, I8255x_probe, I8255x_reset, I8255x_poll, I8255x_transmit, 0
dd  0x24498086, I8255x_probe, I8255x_reset, I8255x_poll, I8255x_transmit, 0
dd  0x10688086, I8255x_probe, I8255x_reset, I8255x_poll, I8255x_transmit, 0

dd  0x802910ec, rtl8029_probe, rtl8029_reset, rtl8029_poll, rtl8029_transmit, 0

dd  0x813910ec, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit, rtl8139_cable  ; tested by hidnplayr: works ok
dd  0x813810ec, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit, rtl8139_cable
dd  0x12111113, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit, rtl8139_cable  ; tested by hidnplayr: works ok
dd  0x13601500, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit, rtl8139_cable
dd  0x13604033, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit, rtl8139_cable
dd  0x13001186, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit, rtl8139_cable
dd  0x13401186, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit, rtl8139_cable
dd  0xab0613d1, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit, rtl8139_cable
dd  0xa1171259, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit, rtl8139_cable
dd  0xa11e1259, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit, rtl8139_cable
dd  0xab0614ea, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit, rtl8139_cable
dd  0xab0714ea, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit, rtl8139_cable
dd  0x123411db, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit, rtl8139_cable
dd  0x91301432, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit, rtl8139_cable
dd  0x101202ac, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit, rtl8139_cable
dd  0x0106018a, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit, rtl8139_cable
dd  0x1211126c, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit, rtl8139_cable
dd  0x81391743, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit, rtl8139_cable
dd  0x8139021b, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit, rtl8139_cable

dd  0x816810ec, rtl8169_probe, rtl8169_reset, rtl8169_poll, rtl8169_transmit, 0
dd  0x816910ec, rtl8169_probe, rtl8169_reset, rtl8169_poll, rtl8169_transmit, 0
dd  0x011616ec, rtl8169_probe, rtl8169_reset, rtl8169_poll, rtl8169_transmit, 0
dd  0x43001186, rtl8169_probe, rtl8169_reset, rtl8169_poll, rtl8169_transmit, 0
dd  0x816710ec, rtl8169_probe, rtl8169_reset, rtl8169_poll, rtl8169_transmit, 0

dd  0x590010b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x592010b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x597010b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x595010b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x595110b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x595210b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x900010b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x900110b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x900410b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x900510b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x900610b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x900A10b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x905010b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x905110b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x905510b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x905810b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x905A10b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x920010b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x980010b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x980510b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x764610b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x505510b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x605510b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x605610b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x5b5710b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x505710b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x515710b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x525710b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x656010b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x656210b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x656410b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0
dd  0x450010b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit, 0

dd  0x09001039, SIS900_probe, SIS900_reset, SIS900_poll, SIS900_transmit, 0

dd  0x20001022, pcnet32_probe, pcnet32_reset, pcnet32_poll, pcnet32_xmit, 0
dd  0x26251022, pcnet32_probe, pcnet32_reset, pcnet32_poll, pcnet32_xmit, 0
dd  0x20011022, pcnet32_probe, pcnet32_reset, pcnet32_poll, pcnet32_xmit, 0

;dd  0x08031516, mtd80x_probe, mtd80x_reset, mtd80x_poll, mtd80x_transmit, mtd80x_cable

; following cards are untested
dd  0x70161039, SIS900_probe, SIS900_reset, SIS900_poll, SIS900_transmit, 0
;dd  0x08001516, mtd80x_probe, mtd80x_reset, mtd80x_poll, mtd80x_transmit, mtd80x_cable
;dd  0x08911516, mtd80x_probe, mtd80x_reset, mtd80x_poll, mtd80x_transmit, mtd80x_cable

dd 0x006610de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; nVidia Corporation nForce2 Ethernet Controller
dd 0x01c310de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x00D610de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x008610de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x008c10de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x00e610de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x00df10de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x005610de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x005710de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x003710de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x003810de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x026810de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x026910de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x037210de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x037310de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x03e510de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x03e610de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x03ee10de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x03ef10de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x045010de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x045110de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x045210de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x045310de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x054c10de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x054d10de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x054e10de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x054f10de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x07dc10de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x07dd10de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x07de10de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x07df10de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x076010de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; MCP77 Ethernet Controller
dd 0x076110de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x076210de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x076310de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x0ab010de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x0ab110de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x0ab210de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x0ab310de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested
dd 0x0d7d10de, forcedeth_probe, forcedeth_reset, forcedeth_poll, forcedeth_transmit, forcedeth_cable ; not tested

rb PCICARDS_ENTRY_SIZE	; end of list marker, do not remove
endg

uglobal
;Net-stack's interface's settings
  node_addr:	      db  0,0,0,0,0,0
  gateway_ip:	      dd  0
  dns_ip:	      dd  0

  eth_rx_data_len:    dw  0
  eth_status:	      dd  0
  io_addr:	      dd  0
  hdrtype:	      db  0
  vendor_device:      dd  0
  pci_data:	      dd  0
  pci_dev:	      dd  0
  pci_bus:	      dd  0

  ; These will hold pointers to the selected driver functions
  drvr_probe:	      dd  0
  drvr_reset:	      dd  0
  drvr_poll:	      dd  0
  drvr_transmit:      dd  0
  drvr_cable:	      dd  0

endg

iglobal
  broadcast_add:      db  0xff,0xff,0xff,0xff,0xff,0xff
  subnet_mask:	      dd  0x00ffffff   ; 255.255.255.0
endg

include "arp.inc"    ;arp-protocol functions
include "pci.inc"    ;PCI bus access functions


;***************************************************************************
;   Function
;      eth_tx
;
;   Description
;      Looks at the NET1OUT_QUEUE for data to send.
;      Stores that destination IP in a location used by the tx routine
;      Looks up the MAC address in the ARP table; stores that where
;      the tx routine can get it
;      Get the length of the data. Store that where the tx routine wants it
;      Call tx
;      Places buffer on empty queue when the tx routine finished
;
;***************************************************************************
proc eth_tx stdcall uses ebx esi edi
local MACAddress  dp  ?  ;allocate 6 bytes in the stack

    ; Look for a buffer to tx
    mov     eax, NET1OUT_QUEUE
    call    dequeue
    cmp     ax, NO_BUFFER
    je	    .exit	     ; Exit if no buffer available

    push    eax  ;save buffer number

    ; convert buffer pointer eax to the absolute address
    imul    eax, IPBUFFSIZE
    add     eax, IPbuffs

    ; Extract the destination IP
    ; find the destination IP in the ARP table, get MAC
    ; store this MAC in 'MACAddress'
    mov     ebx, eax		   ; Save buffer address
    mov     edx, [ebx + 16]	   ; get destination address

    ; If the destination address is 255.255.255.255,
    ; set the MACAddress to all ones ( broadcast )
    cld
    mov     esi, broadcast_add
    lea     edi, [MACAddress]
    movsd
    movsw
    cmp     edx, 0xffffffff
    je	    .send		 ; If it is broadcast, just send

    lea     eax, [MACAddress]	 ;cause this is local variable
    stdcall arp_table_manager, ARP_TABLE_IP_TO_MAC, edx, eax ;opcode,IP,MAC_ptr - Get the MAC address.

    cmp     eax, ARP_VALID_MAPPING
    je	    .send

    ; No valid entry. Has the request been sent, but timed out?
    cmp     eax, ARP_RESPONSE_TIMEOUT
    je	    .freebuf

  .wait_response:		    ;we wait arp-response
				    ; Re-queue the packet, and exit
    pop     ebx
    mov     eax, NET1OUT_QUEUE
    call    queue			 ; Get the buffer back
    jmp     .exit

  .send:    ;if ARP_VALID_MAPPING then send the packet
    lea     edi, [MACAddress]	  ; Pointer to 48 bit destination address
    movzx   ecx, word[ebx+2]	  ; Size of IP packet to send
    xchg    ch, cl		  ; because mirror byte-order
    mov     esi, ebx		  ; Pointer to packet data
    mov     bx, ETHER_IP	  ; Type of packet
    push    ebp
    call    dword [drvr_transmit] ; Call the drivers transmit function
    pop     ebp

    ; OK, we have sent a packet, so increment the count
    inc     dword [ip_tx_count]

    ; And finally, return the buffer to the free queue
  .freebuf:
    pop     eax
    call    freeBuff

  .exit:
    ret
endp

;***************************************************************************
;   Function
;      ether_IP_handler
;
;   Description
;      Called when an IP ethernet packet is received on the ethernet
;      Header + Data is in Ether_buffer[]
;      We just need to get a buffer from the 'free' queue, and
;      store the packet in it, then insert the packet number into the
;      IPRX queue.
;      If no queue entry is available, the packet is silently discarded
;      All registers may be destroyed
;
;***************************************************************************
;uglobal
;  ether_IP_handler_cnt dd ?
;endg
ether_IP_handler:
    mov     eax, EMPTY_QUEUE
    call    dequeue
    cmp     ax, NO_BUFFER
    je	    eiph00x

    ; convert buffer pointer eax to the absolute address
    push    eax
    mov     ecx, IPBUFFSIZE
    mul     ecx
    add     eax, IPbuffs

    mov     edi, eax

    ; get a pointer to the start of the DATA
    mov     esi, ETH_FRAME.Data

    ; Now store it all away
    mov     ecx, IPBUFFSIZE / 4     ; Copy all of the available
			      ; data across - worse case
    cld
    rep     movsd

;        inc     [ether_IP_handler_cnt]
;        DEBUGF  1, "K : ether_IP_handler (%u)\n", [ether_IP_handler_cnt]

    ; And finally, place the buffer in the IPRX queue
    pop     ebx
    mov     eax, IPIN_QUEUE
    call    queue

eiph00x:
    ret

;***************************************************************************
;   Function
;      eth_probe
;   Description
;      Searches for an ethernet card. If found, the card is enabled and
;      the ethernet -> IP link established
;
;      This function scans the PCI bus looking for a supported device.
;      ISA bus is currently not supported.
;
;        eax is 0 if no hardware found
;***************************************************************************
eth_probe:
    ; Find a card on the PCI bus, and get it's address
    call    scan_bus			; Find the ethernet cards PIC address
    xor     eax, eax
    cmp     [io_addr], eax
    je	    ep_00x			; Return 0 in eax if no cards found

    call    dword [drvr_probe]		; Call the drivers probe function

    mov     eax, [io_addr]		; return a non zero value

ep_00x:
    ret

;***************************************************************************
;   Function
;      ethernet_driver
;
;   Description
;       The ethernet RX and TX handler
;       This is a kernel function, called by stack_handler
;
;***************************************************************************
ethernet_driver:
    ; Do nothing if the driver is inactive
    cmp     [ethernet_active], byte 0
    je	    eth_exit

    call    eth_rx
    call    eth_tx

eth_exit:
    ret

;***************************************************************************
;   Function
;      eth_rx
;
;   Description
;      Polls the ethernet card for received data. Extracts if present
;       Depending on the Protocol within the packet:
;         ARP : Pass to ARP_handler. This may result in an ARP reply
;               being tx'ed
;         IP  : Store in an IP buffer
;
;***************************************************************************
eth_rx:
    xor     ax, ax
    mov     [eth_rx_data_len], ax
    call    dword [drvr_poll]	    ; Call the drivers poll function

    mov     ax, [eth_rx_data_len]
    cmp     ax, 0
    je	    .exit


    ; Check the protocol. Call appropriate handler

    mov     ax, [ETH_FRAME.Type]    ; The address of the protocol word

    cmp     ax, ETHER_IP
    je	    .is_ip		     ; It's IP

    cmp     ax, ETHER_ARP
    je	    .is_arp		     ; It is ARP

    DEBUGF  1,"K : eth_rx - dumped (%u)\n", ax
    inc     [dumped_rx_count]
    jmp     .exit		; If not IP or ARP, ignore

  .is_ip:
;    DEBUGF  1,"K : eth_rx - IP packet\n"
    inc     dword [ip_rx_count]
    call    ether_IP_handler
    jmp     .exit

  .is_arp:
;    DEBUGF  1,"K : eth_rx - ARP packet\n"
    ; At this point, the packet is still in the Ether_buffer
    call    arp_handler

  .exit:
    ret
