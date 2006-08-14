;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;;  ETHERNET.INC                                                   ;;
;;                                                                 ;;
;;  Ethernet network layer for Menuet OS                           ;;
;;                                                                 ;;
;;  Version 0.4  22 September 2003                                 ;;
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

;********************************************************************
;   Interface
;      ethernet_driver   called by stack_handler in stack.inc
;      eth_probe         called by app_stack_handler in stack.inc
;
;********************************************************************

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

include "rtl8029.inc"
include "i8255x.inc"
include "rtl8139.inc"
include "3c59x.inc"
include "sis900.inc"
include "pcnet32.inc"

; DEBUGGING_STATE enables or disables output of received and transmitted
; data over the serial port
DEBUGGING_ENABLED           equ     1
DEBUGGING_DISABLED          equ     0
DEBUGGING_STATE             equ     DEBUGGING_ENABLED

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

PCICARDS_ENTRY_SIZE         equ     20    ; Size of each PCICARDS entry

iglobal
PCICards:
dd  0x12098086, I8255x_probe, I8255x_reset, I8255x_poll, I8255x_transmit
dd  0x10298086, I8255x_probe, I8255x_reset, I8255x_poll, I8255x_transmit
dd  0x12298086, I8255x_probe, I8255x_reset, I8255x_poll, I8255x_transmit
dd  0x10308086, I8255x_probe, I8255x_reset, I8255x_poll, I8255x_transmit
dd  0x24498086, I8255x_probe, I8255x_reset, I8255x_poll, I8255x_transmit
dd  0x802910ec, rtl8029_probe, rtl8029_reset, rtl8029_poll, rtl8029_transmit
dd  0x12111113, rtl8029_probe, rtl8029_reset, rtl8029_poll, rtl8029_transmit
dd  0x813910ec, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit
; /+/ Новые вендоры сетевых карт на базе rtl8139
dd  0x813810ec, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit
dd  0x12111113, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit
dd  0x13601500, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit
dd  0x13604033, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit
dd  0x13001186, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit
dd  0x13401186, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit
dd  0xab0613d1, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit
dd  0xa1171259, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit
dd  0xa11e1259, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit
dd  0xab0614ea, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit
dd  0xab0714ea, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit
dd  0x123411db, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit
dd  0x91301432, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit
dd  0x101202ac, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit
dd  0x0106018a, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit
dd  0x1211126c, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit
dd  0x81391743, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit
dd  0x8139021b, rtl8139_probe, rtl8139_reset, rtl8139_poll, rtl8139_transmit
; /-/
dd  0x590010b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x592010b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x597010b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x595010b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x595110b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x595210b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x900010b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x900110b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x900410b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x900510b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x900610b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x900A10b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x905010b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x905110b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x905510b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x905810b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x905A10b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x920010b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x980010b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x980510b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x764610b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x505510b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x605510b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x605610b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x5b5710b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x505710b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x515710b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x525710b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x656010b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x656210b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x656410b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x450010b7, e3c59x_probe, e3c59x_reset, e3c59x_poll, e3c59x_transmit
dd  0x09001039, SIS900_probe, SIS900_reset, SIS900_poll, SIS900_transmit
dd  0x20001022, pcnet32_probe, pcnet32_reset, pcnet32_poll, pcnet32_xmit
dd  0x26251022, pcnet32_probe, pcnet32_reset, pcnet32_poll, pcnet32_xmit
dd  0x20011022, pcnet32_probe, pcnet32_reset, pcnet32_poll, pcnet32_xmit
; following card is untested
dd  0x70161039, SIS900_probe, SIS900_reset, SIS900_poll, SIS900_transmit
dd  0,0,0,0,0  ; end of list marker, do not remove
endg

; PCI Bus defines
PCI_HEADER_TYPE             equ     0x0e  ;8 bit
PCI_BASE_ADDRESS_0          equ     0x10  ;32 bit
PCI_BASE_ADDRESS_5          equ     0x24  ;32 bits
PCI_BASE_ADDRESS_SPACE_IO   equ     0x01
PCI_VENDOR_ID               equ     0x00  ;16 bit
PCI_BASE_ADDRESS_IO_MASK    equ     0xFFFFFFFC

ETHER_IP                    equ     0x0008      ; Reversed from 0800 for intel
ETHER_ARP                   equ     0x0608      ; Reversed from 0806 for intel
ETHER_RARP                  equ     0x3580
ARP_REQ_OPCODE              equ     0x0100
ARP_REP_OPCODE              equ     0x0200

uglobal
  arp_rx_count:       dd  0
  ip_rx_count:        dd  0
  dumped_rx_count:    dd  0
  ip_tx_count:        dd  0
  node_addr:          db  0,0,0,0,0,0
  eth_rx_data_len:    dw  0
  eth_status:         dd  0
  io_addr:            dd  0
  hdrtype:            db  0
  vendor_device:      dd  0
  pci_data:           dd  0
  pci_dev:            dd  0
  pci_bus:            dd  0

  ; These will hold pointers to the selected driver functions
  drvr_probe:         dd  0
  drvr_reset:         dd  0
  drvr_poll:          dd  0
  drvr_transmit:      dd  0

  ; These hold the destination Host identity for ARP responses
  remote_ip_add:      dd  0
  remote_hw_add:      db  0, 0, 0, 0, 0, 0
endg

iglobal
  broadcast_add:      db  0xff,0xff,0xff,0xff,0xff,0xff
  subnet_mask:        dd  0x00ffffff
endg

uglobal
  ; This is used by getMACfromIP
  MACAddress:         db  0,0,0,0,0,0
  gateway_ip:         db  0, 0, 0, 0
  dns_ip:             dd  0
endg

; The follow is the ARP Table.
; This table must be manually updated and the kernel recompilied if
; changes are made to it.
; ARP_TABLE_SIZE defines the size of the table
; ARP_TABLE_ENTRIES defines the number of entries in the table
; Each entry is 10 bytes: 4 Byte IP address, 6 byte MAC Address,
;                         2 bytes status, 2 bytes TTL ( in seconds )
; Empty entries are filled with zeros
; The TTL field is decremented every second, and is deleted when it
; reaches 0. It is refreshed every time a packet is received
; If the TTL field is 0xFFFF it is a permanent entry and is never deleted
; The status field can be the following values
; 0x0000  entry not used
; 0x0001  entry holds a valid mapping
; 0x0002  entry contains an IP address, awaiting ARP response
; 0x0003  No response received to ARP request.
; The last status value is provided to allow the network layer to delete
; a packet that is queued awaiting an ARP response

ARP_NO_ENTRY                equ 0
ARP_VALID_MAPPING           equ 1
ARP_AWAITING_RESPONSE       equ 2
ARP_RESPONSE_TIMEOUT        equ 3

ARP_ENTRY_SIZE              equ     14          ; Number of bytes per entry
ARP_TABLE_SIZE              equ     20          ; Size of table
ARP_TABLE_ENTRIES           equ     0           ; Inital, hardcoded entries

uglobal
  ARPTable:
  times ( ARP_TABLE_SIZE - ARP_TABLE_ENTRIES ) * ARP_ENTRY_SIZE  db 0
endg

iglobal
  NumARP:        db    ARP_TABLE_ENTRIES
endg

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
    call    scan_bus                    ; Find the ethernet cards PIC address
    xor     eax, eax
    cmp     [io_addr], eax
    je      ep_00x                      ; Return 0 in eax if no cards found

    call    dword [drvr_probe]          ; Call the drivers probe function

    mov     eax, [io_addr]              ; return a non zero value

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
    je      eth_exit

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
    call    dword [drvr_poll]       ; Call the drivers poll function

    mov     ax, [eth_rx_data_len]
    cmp     ax, 0
    je      erx_exit

if DEBUGGING_STATE = DEBUGGING_ENABLED
    pusha
    mov     eax, 0                    ;Indicate that this is a received packet
    mov     cx, [eth_rx_data_len]
    mov     esi, Ether_buffer
    cmp     word [esi + 12], ETHER_IP
    jnz     erxd_done
;    cmp     byte [esi + 14 + 9], 0x06  ; TCP
;    jnz     erxd_done
    call    eth_dump
erxd_done:
    popa
end if

    ; Check the protocol. Call appropriate handler
    mov     eax, Ether_buffer
    add     eax, 12                  ; The address of the protocol word

    mov     ax, [eax]

    cmp     ax, ETHER_ARP
    je      erx_001                  ; It is ARP

    cmp     ax, ETHER_IP
    je      erx_002                  ; It's IP

;    inc     dword [dumped_rx_count]

    jmp     erx_exit               ; If not IP or ARP, ignore

erx_001:
    mov     eax, [arp_rx_count]
    inc     eax
    mov     [arp_rx_count], eax

    ; At this point, the packet is still in the Ether_buffer
    call    arp_handler

    jmp     erx_exit

erx_002:
    mov     eax, [ip_rx_count]
    inc     eax
    mov     [ip_rx_count], eax

    ; Check to see if the MAC address is in our arp table
    ; refresh the arp ttl if so

    mov     esi, Ether_buffer
    add     esi, 6

    call    refreshARP

    call    ether_IP_handler

    jmp     erx_exit

erx_exit:
    ret

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
eth_tx:
    ; Look for a buffer to tx
    mov     eax, NET1OUT_QUEUE
    call    dequeue
    cmp     ax, NO_BUFFER
    je      eth_exit            ; Exit if no buffer available

    push    eax

    ; convert buffer pointer eax to the absolute address
    mov     ecx, IPBUFFSIZE
    mul     ecx
    add     eax, IPbuffs

    ; Extract the destination IP
    ; find the destination IP in the ARP table, get MAC
    ; store this MAC in 'MACAddress'
    mov     ebx, eax               ; Save buffer address
    mov     edx, [ebx + 16]        ; get destination address

    ; If the destination address is 255.255.255.255,
    ; set the MACAddress to all ones ( broadcast )
    mov     [MACAddress], dword 0xffffffff
    mov     [MACAddress + 4], word 0xffff
    cmp     edx, 0xffffffff
    je      etx_send                ; If it is broadcast, just send

    call    getMACfromIP           ; Get the MAC address.

    cmp     eax, ARP_VALID_MAPPING
    jz      etx_send

    ; No valid entry. Are we waiting for a response?
    cmp     eax, ARP_AWAITING_RESPONSE
    jne     etx_001

    ; Re-queue the packet, and exit
    pop     ebx
    mov     eax, NET1OUT_QUEUE
    call    queue
    jmp     etx_exit

etx_001:
    ; HAs the request been sent, but timed out?
    cmp     eax, ARP_RESPONSE_TIMEOUT
    jne     etx_002

    pop     eax
    call    freeBuff
    jmp     etx_exit

etx_002:
    ; There is no entry. Re queue the request, and ask ARP to send a request

    ; IP address is in edx
    push    edx
    call    arp_request
    pop     ebx

    ; Add an entry in the ARP table, awaiting response

    cmp     byte [NumARP], ARP_TABLE_SIZE
    je      etx_003            ; We cannot add a new entry in the table

    inc     byte [NumARP]

    movzx   eax, byte [NumARP]
    mov     ecx, ARP_ENTRY_SIZE
    mul     ecx
    sub     eax, ARP_ENTRY_SIZE

    mov     [eax + ARPTable], ebx
    xor     ebx, ebx
    mov     [eax + ARPTable + 4], ebx
    mov     [eax + ARPTable + 8], bx

    ; set the status field up - awaiting response
    mov     cl, 0x00
    mov     [eax + ARPTable + 10], cl
    mov     cl, 0x02
    mov     [eax + ARPTable + 11], cl

    ; Initialise the time to live field - 10s
    mov     cx, 0x000A
    mov     [eax + ARPTable + 12], cx

etx_003:
    pop     ebx                        ; Get the buffer back
    mov     eax, NET1OUT_QUEUE
    call    queue
    jmp     etx_exit

etx_send:
    xor     ecx, ecx
    mov     ch, [ebx+2]
    mov     cl, [ebx+3]          ; ; Size of IP packet to send

    mov     esi, ebx

    mov     edi, MACAddress

if DEBUGGING_STATE = DEBUGGING_ENABLED
    pusha
    mov     cx, 42
    mov     eax, 1                    ; Indicate that this is a tx packet
    call    eth_dump
    popa
end if

    mov     bx, ETHER_IP
    call    dword [drvr_transmit]       ; Call the drivers transmit function

    ; OK, we have sent a packet, so increment the count
    inc     dword [ip_tx_count]

    ; And finally, return the buffer to the free queue
    pop     eax
    call    freeBuff

etx_exit:
    ret

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
ether_IP_handler:
    mov     eax, EMPTY_QUEUE
    call    dequeue
    cmp     ax, NO_BUFFER
    je      eiph00x

    ; convert buffer pointer eax to the absolute address
    push    eax
    mov     ecx, IPBUFFSIZE
    mul     ecx
    add     eax, IPbuffs

    mov     edi, eax

    ; get a pointer to the start of the DATA
    mov     esi, Ether_buffer + 14

    ; Now store it all away
    mov     ecx, IPBUFFSIZE / 4     ; Copy all of the available
                              ; data across - worse case
    cld
    rep     movsd

    ; And finally, place the buffer in the IPRX queue
    pop     ebx
    mov     eax, IPIN_QUEUE
    call    queue

eiph00x:
    ret

;***************************************************************************
;
;  ARP CODE FOLLOWS
;
;  The ARP code is used by ethernet drivers to translate an destination
;  IP address into an ethernet hardware address. Functions to broadcast
;  requests and handle response are (or will be) here.
;  The IP layer has no knowledge of ARP, as this is a network interface
;  issue
;
;***************************************************************************

;***************************************************************************
;   Function
;      arp_timer
;
;   Description
;      Called every 1s
;      It is responsible for removing expired routes
;      All registers may be destroyed
;
;***************************************************************************
arp_timer:
    ; loop through all the ARP entries, decrementing each one
    ; that doesn't have a TTL of 0xFFFF
    movzx   eax, byte [NumARP]

arp_001:
    cmp     eax, 0
    je      arp_003

    push    eax
    dec     eax
    mov     ecx, ARP_ENTRY_SIZE
    mul     ecx
    cmp     word [ eax + ARPTable + 12], 0xFFFF
    je      arp_002

    cmp     word [ eax + ARPTable + 12], 0
    je      arp_002

    dec     word [eax + ARPTable + 12]

arp_002:
    pop     eax
    dec     eax
    jmp     arp_001

    ; Now, look for entries with a TTL of 0
    ; Valid entries and response timeout entries get removed
    ; awaiting response gets converted into a response timeout, with a
    ; short life time - this allows queued packets to be flushed
arp_003:
    movzx   edx, byte [NumARP]
    cmp     edx, 0
    je      arp_exit

    ; EDX holds the # of entries to search through
    mov     eax, 0

arp_005:
    cmp     word [ eax + ARPTable + 12], 0
    jne     arp_004

    ; If it's status code is 0001 or 0003, delete the entry
    cmp     word [eax + ARPTable + 10], 0x0100
    je      arp_007
    cmp     word [eax + ARPTable + 10], 0x0300
    je      arp_007

    ; The only other valid code is 0002 - indicating a
    ; timeout while waiting for a response. Change the
    ; entry to response timed out

    mov     [eax + ARPTable + 10], word 0x0300
    mov     [eax + ARPTable + 12], word 0x000A
    jmp     arp_004

arp_007:
    ; Delete this entry
    mov     edi, ARPTable
    add     edi, eax
    mov     esi, edi
    add     esi, ARP_ENTRY_SIZE

    mov     ecx, (ARP_TABLE_SIZE - 1) * ARP_ENTRY_SIZE
    sub     ecx, eax

    rep     movsb

    dec     byte [NumARP]
    jmp     arp_006

arp_004:
    add     eax, ARP_ENTRY_SIZE
arp_006:
    dec     edx
    cmp     edx, 0
    jne     arp_005

arp_exit:
    ret

;***************************************************************************
;   Function
;      arp_request
;
;   Description
;      Sends an ARP request on the ethernet
;        The requested IP address is in edx
;      All registers may be destroyed
;
;***************************************************************************
arp_request:
    mov     ebx, Ether_buffer
    mov     ax, 0x0100
    mov     [ebx], ax
    add     ebx, 2

    mov     ax, 0x0008
    mov     [ebx], ax
    add     ebx, 2

    mov     ax, 0x0406
    mov     [ebx], ax
    add     ebx, 2

    mov     ax, 0x0100
    mov     [ebx], ax
    add     ebx, 2

    mov     ecx, node_addr
    mov     eax, [ecx]
    mov     [ebx], eax
    add     ecx, 4
    add     ebx, 4
    mov     ax, [ecx]
    mov     [ebx], ax
    add     ebx, 2
    mov     eax, [stack_ip]
    mov     [ebx], eax
    add     ebx, 4

    xor     eax, eax
    mov     [ebx], eax
    add     ebx, 4
    mov     [ebx], ax

    add     ebx, 2
    mov     [ebx], edx

    ; Now, send it!

    ; Pointer to 48 bit destination address in edi
    ; Type of packet in bx
    ; size of packet in ecx
    ; pointer to packet data in esi
    mov      edi, broadcast_add

;if DEBUGGING_STATE = DEBUGGING_ENABLED
;    pusha
;    mov     eax, 1                    ; Indicate that this is a tx packet
;    mov     ecx, 28
;   mov      esi, Ether_buffer
;    call    eth_dump
;    popa
;end if

    mov     bx, ETHER_ARP
    mov     ecx, 28
    mov     esi, Ether_buffer
    call    dword [drvr_transmit]       ; Call the drivers transmit function
    ret

;***************************************************************************
;   Function
;      arp_handler
;
;   Description
;      Called when an ARP packet is received on the ethernet
;      Header + Data is in Ether_buffer[]
;       It looks to see if the packet is a request to resolve this Hosts
;       IP address. If it is, send the ARP reply packet.
;      This Hosts IP address is in dword [stack_ip]  ( in network format )
;       This Hosts MAC address is in node_addr[6]
;      All registers may be destroyed
;
;***************************************************************************
arp_handler:
    ; Is this a REQUEST?
    ; Is this a request for My Host IP
    ; Yes - So construct a response message.
    ; Send this message to the ethernet card for transmission

    mov     ebx, Ether_buffer

    mov     edx, ebx
    add     edx, 20
    mov     ax, [edx]
    cmp     ax, ARP_REQ_OPCODE      ; Is this a request packet?
    jne     arph_resp            ; No - so test for response

    mov     edx, ebx
    add     edx, 38
    mov     eax, [edx]

    cmp     eax, [stack_ip]         ; Is it looking for my IP address?
    jne     arph_exit            ; No - so quit now

    ; OK, it is a request for my MAC address. Build the frame and send it

    ; Save the important data from the original packet
    ; remote MAC address first
    mov     ecx, remote_hw_add
    mov     edx, ebx
    add     edx, 22               ; edx points to Source h/w address
    mov     eax, [edx]
    mov     [ecx], eax
    add     edx, 4
    add     ecx, 4
    mov     ax, [edx]
    mov     [ecx],ax

    ; and also the remote IP address
    add     edx, 2
    mov     eax,[edx]
    mov     [remote_ip_add], eax

    ; So now we can reuse the packet. ebx still holds the address of
    ; the header + packet
    ; We dont need the header ( first 14 bytes )

    mov     edx, ebx
    add     edx, 20
    mov     ax, ARP_REP_OPCODE
    mov     [edx], ax
    add     edx, 2

    mov     ecx, node_addr
    mov     eax, [ecx]
    mov     [edx], eax
    add     ecx, 4
    add     edx, 4
    mov     ax, [ecx]
    mov     [edx], ax
    add     edx, 2
    mov     eax, [stack_ip]
    mov     [edx], eax
    add     edx, 4
    mov     ecx, remote_hw_add
    mov     eax, [ecx]
    mov     [edx], eax
    add     ecx, 4
    add     edx, 4
    mov     ax, [ecx]
    mov     [edx], ax

    add     edx, 2
    mov     eax, [remote_ip_add]
    mov     [edx], eax

    ; Now, send it!

    ; Pointer to 48 bit destination address in edi
    ; Type of packet in bx
    ; size of packet in ecx
    ; pointer to packet data in esi
    mov     edi, remote_hw_add

;if DEBUGGING_STATE = DEBUGGING_ENABLED
;    pusha
;    mov     eax, 1                    ; Indicate that this is a tx packet
;    mov     ecx, 28
;   mov      esi, Ether_buffer + 14
 ;   call    eth_dump
;    popa
;end if

    mov     bx, ETHER_ARP
    mov     ecx, 28
    mov     esi, Ether_buffer + 14
    call    dword [drvr_transmit]       ; Call the drivers transmit function
    jmp     arph_exit

arph_resp:
    cmp     ax, ARP_REP_OPCODE      ; Is this a replypacket?
    jne     arph_resp            ; No - so quit

    ; This was a reply, probably directed at me.
    ; save the remotes MAC & IP
    mov     ecx, remote_hw_add
    mov     edx, ebx
    add     edx, 22               ; edx points to Source h/w address
    mov     eax, [edx]
    mov     [ecx], eax
    add     edx, 4
    add     ecx, 4
    mov     ax, [edx]
    mov     [ecx],ax

    ; and also the remote IP address
    add     edx, 2
    mov     eax,[edx]
    mov     [remote_ip_add], eax

    ; Now, add an entry in the table for this IP address if it doesn't exist

    push    eax
    movzx   eax, byte [NumARP]
    mov     ecx, ARP_ENTRY_SIZE
    mul     ecx
    pop     edx
    movzx   ecx, byte [NumARP]
    cmp     ecx, 0
    je      arph_002

arph_001:
    sub     eax, ARP_ENTRY_SIZE
    cmp     [eax + ARPTable], edx
    loopnz  arph_001                      ; Return back if non match

    jnz     arph_002                   ; None found, add to end

    mov     ecx, [remote_hw_add]
    mov     [eax + ARPTable + 4], ecx
    mov     cx, [remote_hw_add+4]
    mov     [eax + ARPTable + 8], cx

    ; specify the type - a valid entry
    mov     cl, 0x00
    mov     [eax + ARPTable + 10], cl
    mov     cl, 0x01
    mov     [eax + ARPTable + 11], cl

    ; Initialise the time to live field - 1 hour
    mov     cx, 0x0E10
    mov     [eax + ARPTable + 12], cx
    jmp     arph_exit

arph_002:

    cmp     byte [NumARP], ARP_TABLE_SIZE
    je      arph_exit

    inc     byte [NumARP]

    movzx   eax, byte [NumARP]
    mov     ecx, ARP_ENTRY_SIZE
    mul     ecx
    sub     eax, ARP_ENTRY_SIZE

    mov     ecx, [remote_ip_add]
    mov     [eax + ARPTable], ecx
    mov     ecx, [remote_hw_add]
    mov     [eax + ARPTable + 4], ecx
    mov     cx, [remote_hw_add+4]
    mov     [eax + ARPTable + 8], cx

    mov     cl, 0x00
    mov     [eax + ARPTable + 10], cl
    mov     cl, 0x01
    mov     [eax + ARPTable + 11], cl

    ; Initialise the time to live field - 1 hour
    mov     cx, 0x0E10
    mov     [eax + ARPTable + 12], cx

arph_exit:
    ret

; pointer to MAC in esi
refreshARP:
    mov     ebx, [esi]
    mov     dx, [esi+4]
    push    edx
    movzx   eax, byte [NumARP]
    mov     ecx, ARP_ENTRY_SIZE
    mul     ecx
    pop     edx
    movzx   ecx, byte [NumARP]
    cmp     ecx, 0
    je      rf_exit

rf_001:
    sub     eax, ARP_ENTRY_SIZE
    cmp     [eax + ARPTable+4], ebx

    je      rf_002
    loop    rf_001
    jmp     rf_exit

rf_002:
    cmp     [eax + ARPTable+8], dx
    je      rf_gotone
    loop    rf_001
    jmp     rf_exit

rf_gotone:
    ; Initialise the time to live field - 1 hour
    mov     cx, 0x0E10
    mov     [eax + ARPTable + 12], cx

rf_exit:
    ret

;***************************************************************************
;   Function
;      getMACfromIP
;
;   Description
;       Takes an IP address in edx and scans the ARP table for
;        a matching entry
;       If a match is found, it's MAC address is stored in MACAddress.
;      Otherwise the value 0 is writen to MACAddress
;      eax holds ARP table entry status code ( ARP_ )
;      ebx unchanged
;
;***************************************************************************
getMACfromIP:
    ; first, check destination IP to see if it is on 'this' network.
    ; The test is:
    ; if ( destIP & subnet_mask == stack_ip & subnet_mask )
    ;   desitnation is local
    ; else
    ;  destination is remote, so pass to gateway

    mov     eax, edx
    and     eax, [subnet_mask]
    mov     ecx, [stack_ip]
    and     ecx, [subnet_mask]
    cmp     eax, ecx
    je      gm0

    mov     edx, [gateway_ip]
gm0:
    push    edx
    xor     eax, eax
    mov     [MACAddress], eax
    mov     [MACAddress + 4], ax

    movzx   eax, byte [NumARP]
    mov     ecx, ARP_ENTRY_SIZE
    mul     ecx

    pop     edx

    movzx   ecx, byte [NumARP]
    cmp     ecx, 0
    je      gm_none
gm1:
    sub     eax, ARP_ENTRY_SIZE
    cmp     [eax + ARPTable], edx
    loopnz  gm1                      ; Return back if non match
    jnz     gm_none                   ; Quit if none found

    ; eax holds index
    mov     ecx, [eax + ARPTable + 4]
    mov     [MACAddress], ecx
    mov     cx, [eax + ARPTable + 8]
    mov     [MACAddress+4], cx

    ; Return the entry status in eax
    mov     ch, [eax + ARPTable + 10]
    mov     cl, [eax + ARPTable + 11]
    movzx   eax, cx
    jmp     gm_exit

gm_none:
    mov     eax, ARP_NO_ENTRY

gm_exit:
    ret

;***************************************************************************
;
;  PCI CODE FOLLOWS
;
;  the following functions provide access to the PCI interface.
;  These functions are used by scan_bus, and also some ethernet drivers
;
;***************************************************************************

;***************************************************************************
;   Function
;      config_cmd
;
;   Description
;       creates a command dword  for use with the PCI bus
;       bus # in ebx
;      devfn in ecx
;       where in edx
;
;      command dword returned in eax
;       Only eax destroyed
;***************************************************************************
config_cmd:
    push    ecx
    mov     eax, ebx
    shl     eax, 16
    or      eax, 0x80000000
    shl     ecx, 8
    or      eax, ecx
    pop     ecx
    or      eax, edx
    and     eax, 0xFFFFFFFC
    ret

;***************************************************************************
;   Function
;      pcibios_read_config_byte
;
;   Description
;       reads a byte from the PCI config space
;       bus # in ebx
;      devfn in ecx
;       where in edx ( ls 16 bits significant )
;
;      byte returned in al ( rest of eax zero )
;       Only eax/edx destroyed
;***************************************************************************
pcibios_read_config_byte:
    call    config_cmd
    push    dx
    mov     dx, 0xCF8
    out     dx, eax
    pop     dx

    xor     eax, eax
    and     dx, 0x03
    add     dx, 0xCFC
;   and     dx, 0xFFC
    in      al, dx
    ret

;***************************************************************************
;   Function
;      pcibios_read_config_word
;
;   Description
;       reads a word from the PCI config space
;       bus # in ebx
;      devfn in ecx
;       where in edx ( ls 16 bits significant )
;
;      word returned in ax ( rest of eax zero )
;       Only eax/edx destroyed
;***************************************************************************
pcibios_read_config_word:
    call    config_cmd
    push    dx
    mov     dx, 0xCF8
    out     dx, eax
    pop     dx

    xor     eax, eax
    and     dx, 0x02
    add     dx, 0xCFC
;   and     dx, 0xFFC
    in      ax, dx
    ret

;***************************************************************************
;   Function
;      pcibios_read_config_dword
;
;   Description
;       reads a dword from the PCI config space
;       bus # in ebx
;      devfn in ecx
;       where in edx ( ls 16 bits significant )
;
;      dword returned in eax
;       Only eax/edx destroyed
;***************************************************************************
pcibios_read_config_dword:
    push    edx
    call    config_cmd
    push    dx
    mov     dx, 0xCF8
    out     dx, eax
    pop     dx
    xor     eax, eax
    mov     dx, 0xCFC
    in      eax, dx
    pop     edx
    ret

;***************************************************************************
;   Function
;      pcibios_write_config_byte
;
;   Description
;       write a byte in al to the PCI config space
;       bus # in ebx
;      devfn in ecx
;       where in edx ( ls 16 bits significant )
;
;       Only eax/edx destroyed
;***************************************************************************
pcibios_write_config_byte:
    push    ax
    call    config_cmd
    push    dx
    mov     dx, 0xCF8
    out     dx, eax
    pop     dx
    pop     ax

    and     dx, 0x03
    add     dx, 0xCFC
    out     dx, al
    ret

;***************************************************************************
;   Function
;      pcibios_write_config_word
;
;   Description
;       write a word in ax to the PCI config space
;       bus # in ebx
;      devfn in ecx
;       where in edx ( ls 16 bits significant )
;
;       Only eax/edx destroyed
;***************************************************************************
pcibios_write_config_word:
    push    ax
    call    config_cmd
    push    dx
    mov     dx, 0xCF8
    out     dx, eax
    pop     dx
    pop     ax

    and     dx, 0x02
    add     dx, 0xCFC
    out     dx, ax
    ret

;***************************************************************************
;   Function
;      delay_us
;
;   Description
;       delays for 30 to 60 us
;
;        I would prefer this routine to be able to delay for
;       a selectable number of microseconds, but this works for now.
;
;       If you know a better way to do 2us delay, pleae tell me!
;***************************************************************************
delay_us:
    push    eax
    push    ecx

    mov     ecx,2

    in      al,0x61
    and     al,0x10
    mov     ah,al
    cld

dcnt1:
    in      al,0x61
    and     al,0x10
    cmp     al,ah
    jz      dcnt1

    mov     ah,al
    loop    dcnt1

    pop     ecx
    pop     eax

    ret

;***************************************************************************
;   Function
;      scan_bus
;
;   Description
;       Scans the PCI bus for a supported device
;        If a supported device is found, the drvr_ variables are initialised
;       to that drivers functions ( as defined in the PCICards table)
;
;        io_addr   holds card I/O space. 32 bit, but only LS 16 bits valid
;        pci_data  holds the PCI vendor + device code
;       pci_dev   holds PCI bus dev #
;       pci_bus   holds PCI bus #
;
;        io_addr will be zero if no card found
;
;***************************************************************************
scan_bus:
    xor     eax, eax
    mov     [hdrtype], al
    mov     [pci_data], eax

    xor     ebx, ebx         ; ebx = bus# 0 .. 255

sb_bus_loop:
    xor     ecx, ecx         ; ecx = devfn# 0 .. 254  ( not 255? )

sb_devf_loop:
    mov     eax, ecx
    and     eax, 0x07

    cmp     eax, 0
    jne     sb_001

    mov     edx, PCI_HEADER_TYPE
    call    pcibios_read_config_byte
    mov     [hdrtype], al
    jmp     sb_002

sb_001:
    mov     al, [hdrtype]
    and     al, 0x80
    cmp     al, 0x80
    jne     sb_inc_devf

sb_002:
    mov     edx, PCI_VENDOR_ID
    call    pcibios_read_config_dword
    mov     [vendor_device], eax
    cmp     eax, 0xffffffff
    je      sb_empty
    cmp     eax, 0
    jne     sb_check_vendor

sb_empty:
    mov     [hdrtype], byte 0
    jmp     sb_inc_devf

sb_check_vendor:
    ; iterate though PCICards until end or match found
    mov     esi, PCICards

sb_check:
    cmp     [esi], dword 0
    je      sb_inc_devf                ; Quit if at last entry
    cmp     eax, [esi]
    je      sb_got_card
    add     esi, PCICARDS_ENTRY_SIZE
    jmp     sb_check

sb_got_card:
    ; indicate that we have found the card
    mov     [pci_data], eax
    mov     [pci_dev], ecx
    mov     [pci_bus], ebx

    ; Define the driver functions
    push    eax
    mov     eax, [esi+4]
    mov     [drvr_probe], eax
    mov     eax, [esi+8]
    mov     [drvr_reset], eax
    mov     eax, [esi+12]
    mov     [drvr_poll], eax
    mov     eax, [esi+16]
    mov     [drvr_transmit], eax
    pop     eax

    mov     edx, PCI_BASE_ADDRESS_0

sb_reg_check:
    call    pcibios_read_config_dword
    mov     [io_addr], eax
    and     eax, PCI_BASE_ADDRESS_IO_MASK
    cmp     eax, 0
    je      sb_inc_reg
    mov     eax, [io_addr]
    and     eax, PCI_BASE_ADDRESS_SPACE_IO
    cmp     eax, 0
    je      sb_inc_reg

    mov     eax, [io_addr]
    and     eax, PCI_BASE_ADDRESS_IO_MASK
    mov     [io_addr], eax

sb_exit1:
    ret

sb_inc_reg:
    add     edx, 4
    cmp     edx, PCI_BASE_ADDRESS_5
    jbe     sb_reg_check

sb_inc_devf:
    inc     ecx
    cmp     ecx, 255
    jb      sb_devf_loop
    inc     ebx
    cmp     ebx, 256
    jb      sb_bus_loop

    ; We get here if we didn't find our card
    ; set io_addr to 0 as an indication
    xor     eax, eax
    mov     [io_addr], eax

sb_exit2:
    ret

;***************************************************************************
;
;  DEBUGGING CODE FOLLOWS
;
;  If debugging data output is not required, ALL code & data below may
;  be removed.
;
;***************************************************************************

if DEBUGGING_STATE = DEBUGGING_ENABLED

;***************************************************************************
;   Function
;      eth_dump
;
;   Description
;       Dumps a tx or rx ethernet packet over the rs232 link
;       This is a debugging routine that seriously slows down the stack.
;       Use with caution.
;
;       Baud rate is 57600, 8n1  com1
;         eax : type (0 == rx, 1 == tx )
;          cx : # of bytes in buffer
;         esi : address of buffer start
;         edi : pointer to MACAddress ( tx only )
;
;***************************************************************************
eth_dump:
    pusha

    ; Set the port to the desired speed
    mov     ebx, 0x3f8                     ; combase

    mov     edx, ebx
    add     edx, 3                        ; data format register
    mov     al, 0x80                    ; enable access to divisor latch
    out     dx, al

    mov     edx, ebx
    add     edx, 1                        ; interrupt enable register
    mov     al, 0x00                    ; No interruts enabled
    out     dx, al

    mov     edx, ebx
    mov     al, 0x20 / 16                ; set baud rate to 57600 0x10 =115200
    out     dx, al

    mov     edx, ebx
    add     edx, 3                        ; data format register
    mov     al, 0x03                    ; 8 data bits
    out     dx, al

    mov     edx, ebx
    add     edx, 4                        ; Modem control register
    mov     al, 0x08                    ; out2 enabled. No handshaking.
    out     dx, al

    mov     edx, ebx
    add     edx, 1                        ; interrupt enable register
    mov     al, 0x01                    ; Receive data interrupt enabled,
    out     dx, al

    popa

    ; First, display the type of the buffer.
    ; If it is a tx buffer, display the macaddress

    pusha

    cmp     eax, 0
    jne     dd001

    mov     bl, 0x0a
    call    tx_byted
    mov     bl, 0x0d
    call    tx_byted

    ; Output "RX:"
    mov     bl, 'R'
    call    tx_byted
    mov     bl, 'X'
    call    tx_byted
    mov     bl, ':'
    call    tx_byted
    jmp     dump_data

dd001:
    mov     bl, 0x0a
    call    tx_byted
    mov     bl, 0x0d
    call    tx_byted

    ; Output TX: xxxxxxxxxxxx
    mov     bl, 'T'
    call    tx_byted
    mov     bl, 'X'
    call    tx_byted
    mov     bl, ':'
    call    tx_byted
    mov     bl, ' '
    call    tx_byted

    ; Display MAC address
    xor     eax, eax
    mov     al, [edi]
    shr     al, 4
    mov     bl, [eax + hexchars]
    call    tx_byted ; byte in bl eax ebx edx destroyed

    xor     eax, eax
    mov     al, [edi]
    and     al, 0x0f
    mov     bl, [eax + hexchars]
    call    tx_byted ; byte in bl eax ebx edx destroyed

    inc     edi
    xor     eax, eax
    mov     al, [edi]
    shr     al, 4
    mov     bl, [eax + hexchars]
    call    tx_byted ; byte in bl eax ebx edx destroyed

    xor     eax, eax
    mov     al, [edi]
    and     al, 0x0f
    mov     bl, [eax + hexchars]
    call    tx_byted ; byte in bl eax ebx edx destroyed

    inc     edi
    xor     eax, eax
    mov     al, [edi]
    shr     al, 4
    mov     bl, [eax + hexchars]
    call    tx_byted ; byte in bl eax ebx edx destroyed

    xor     eax, eax
    mov     al, [edi]
    and     al, 0x0f
    mov     bl, [eax + hexchars]
    call    tx_byted ; byte in bl eax ebx edx destroyed

    inc     edi
    xor     eax, eax
    mov     al, [edi]
    shr     al, 4
    mov     bl, [eax + hexchars]
    call    tx_byted ; byte in bl eax ebx edx destroyed

    xor     eax, eax
    mov     al, [edi]
    and     al, 0x0f
    mov     bl, [eax + hexchars]
    call    tx_byted ; byte in bl eax ebx edx destroyed

    inc     edi
    xor     eax, eax
    mov     al, [edi]
    shr     al, 4
    mov     bl, [eax + hexchars]
    call    tx_byted ; byte in bl eax ebx edx destroyed

    xor     eax, eax
    mov     al, [edi]
    and     al, 0x0f
    mov     bl, [eax + hexchars]
    call    tx_byted ; byte in bl eax ebx edx destroyed

    inc     edi
    xor     eax, eax
    mov     al, [edi]
    shr     al, 4
    mov     bl, [eax + hexchars]
    call    tx_byted ; byte in bl eax ebx edx destroyed

    xor     eax, eax
    mov     al, [edi]
    and     al, 0x0f
    mov     bl, [eax + hexchars]
    call    tx_byted ; byte in bl eax ebx edx destroyed

dump_data:
    popa

    ; OK, we come in here with
    ; cx == number of byte to send
    ; esi == buffer start
    ;
dd_000:
    mov     bl, 0x0a
    call    tx_byted
    mov     bl, 0x0d
    call    tx_byted

    mov     eax, 16        ; Number of characters on the line
    mov     edi, esi    ; Save first byte position for later

    push    ecx

dd_001:
    push    eax

    ; Print a byte, and a space
    xor     eax, eax
    mov     al, [esi]
    shr     al, 4
    mov     bl, [eax + hexchars]
    call    tx_byted ; byte in bl eax ebx edx destroyed

    xor     eax, eax
    mov     al, [esi]
    and     al, 0x0f
    mov     bl, [eax + hexchars]
    call    tx_byted ; byte in bl eax ebx edx destroyed

    mov     bl, ' '
    call    tx_byted

    pop     eax

    inc     esi
    dec     ecx
    cmp     ecx, 0
    je      dd_0011            ; Print the ASCII format

    dec     eax

    cmp     eax, 0
    je      dd_002            ; Print the ASCII format
    jmp     dd_001            ; Print rest of line

dd_0011:
    ; First, complete the 16 bytes of data, by printing spaces
    dec     eax
    cmp     eax, 0
    je      dd_002

    push    eax
    mov     bl, ' '
    call    tx_byted
    mov     bl, ' '
    call    tx_byted
    mov     bl, ' '
    call    tx_byted
    pop     eax
    jmp     dd_0011

dd_002:
    pop     ecx
    mov     esi, edi        ; Go back to the start of the line data

    mov     eax, 16

outLineAscii:
    push    eax

    xor     eax, eax
    mov     al, [esi]
    mov     bl, '.'

    cmp     al, 0x1F
    jle     outAscii
    cmp     al, 0x7e
    jge     outAscii

    mov     bl, al

outAscii:
    call    tx_byted ; byte in bl eax ebx edx destroyed

    pop     eax
    dec     ecx
    inc     esi
    cmp     ecx, 0
    je      dd_003

    dec     eax
    cmp     eax, 0
    je      dd_003
    jmp     outLineAscii

dd_003:
    cmp     ecx, 0
    je      dd_004
    jmp     dd_000

dd_004:
    ret

;***************************************************************************
;   Function
;      tx_byte
;
;   Description
;       Send a byte in bl out of the com port 1
;       destroys eax, edx
;
;***************************************************************************
tx_byted:
    push    ebx                     ; Save the byte

    mov     ebx, 0x3f8            ; get the com port address

    ; Wait for transmit buffer to empty. This could take 1ms @ 9600baud

    mov     edx, ebx
    add     edx, 5

wait_txd:
    in      al, dx                   ; read uart serialisation status
    and     al, 0x40
    cmp     al, 0
    jz      wait_txd                  ; loop until free

    mov     edx, ebx
    pop     eax                     ; restore the byte to send
    out     dx, al
    ret

iglobal
  ; This is used for translating hex to ASCII for display or output
  hexchars    db '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
endg
end if

