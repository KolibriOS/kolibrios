$Revision: 431 $
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                    ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved.       ;;
;; Distributed under terms of the GNU General Public License          ;;
;;                                                                    ;;
;;  SIS900.INC                                                        ;;
;;                                                                    ;;
;;  Ethernet driver for Menuet OS                                     ;;
;;                                                                    ;;
;;  Version 0.4  26 April 2004                                        ;;
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
;  How To Use:
;    Add the following lines to Ethernet.inc in their appropriate locations
;
;         include "Sis900.INC"
;         dd  0x09001039, SIS900_probe, SIS900_reset, SIS900_poll,
; SIS900_transmit
;         dd  0x70161039, SIS900_probe, SIS900_reset, SIS900_poll,
; SIS900_transmit   ;untested
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


;* buffers and descriptors
cur_rx  db  0
NUM_RX_DESC    equ    4               ;* Number of RX descriptors *
NUM_TX_DESC    equ    1               ;* Number of TX descriptors *
RX_BUFF_SZ          equ    1520            ;* Buffer size for each Rx buffer *
TX_BUFF_SZ          equ    1516            ;* Buffer size for each Tx buffer *

uglobal
align   4
txd: times (3 * NUM_TX_DESC) dd 0
rxd: times (3 * NUM_RX_DESC) dd 0
endg

txb equ eth_data_start
rxb equ txb + (NUM_TX_DESC * TX_BUFF_SZ)
SIS900_ETH_ALEN equ     6       ;* Size of Ethernet address *
SIS900_ETH_HLEN equ     14      ;* Size of ethernet header *
SIS900_ETH_ZLEN equ     60      ;* Minimum packet length *
SIS900_DSIZE equ 0x00000fff
SIS900_CRC_SIZE equ 4
SIS900_RFADDR_shift equ 16
;SIS900 Symbolic offsets to registers.
    SIS900_cr           equ     0x0               ; Command Register
    SIS900_cfg          equ     0x4       ; Configuration Register
    SIS900_mear     equ     0x8       ; EEPROM Access Register
    SIS900_ptscr    equ     0xc       ; PCI Test Control Register
    SIS900_isr          equ     0x10      ; Interrupt Status Register
    SIS900_imr          equ     0x14      ; Interrupt Mask Register
    SIS900_ier          equ     0x18      ; Interrupt Enable Register
    SIS900_epar         equ     0x18      ; Enhanced PHY Access Register
    SIS900_txdp     equ     0x20      ; Transmit Descriptor Pointer Register
    SIS900_txcfg    equ     0x24      ; Transmit Configuration Register
    SIS900_rxdp     equ     0x30      ; Receive Descriptor Pointer Register
    SIS900_rxcfg    equ     0x34      ; Receive Configuration Register
    SIS900_flctrl   equ     0x38      ; Flow Control Register
    SIS900_rxlen    equ     0x3c      ; Receive Packet Length Register
    SIS900_rfcr     equ     0x48      ; Receive Filter Control Register
    SIS900_rfdr     equ     0x4C      ; Receive Filter Data Register
    SIS900_pmctrl   equ     0xB0      ; Power Management Control Register
    SIS900_pmer         equ     0xB4      ; Power Management Wake-up Event Register
;SIS900 Command Register Bits
    SIS900_RELOAD       equ      0x00000400
    SIS900_ACCESSMODE   equ      0x00000200
    SIS900_RESET        equ      0x00000100
    SIS900_SWI          equ      0x00000080
    SIS900_RxRESET      equ      0x00000020
    SIS900_TxRESET      equ      0x00000010
    SIS900_RxDIS        equ      0x00000008
    SIS900_RxENA        equ      0x00000004
    SIS900_TxDIS        equ      0x00000002
    SIS900_TxENA        equ      0x00000001
;SIS900 Configuration Register Bits
    SIS900_DESCRFMT      equ    0x00000100 ; 7016 specific
    SIS900_REQALG        equ    0x00000080
    SIS900_SB            equ    0x00000040
    SIS900_POW           equ    0x00000020
    SIS900_EXD           equ    0x00000010
    SIS900_PESEL         equ    0x00000008
    SIS900_LPM           equ    0x00000004
    SIS900_BEM           equ    0x00000001
    SIS900_RND_CNT       equ    0x00000400
    SIS900_FAIR_BACKOFF  equ    0x00000200
    SIS900_EDB_MASTER_EN equ    0x00002000
;SIS900 Eeprom Access Reigster Bits
    SIS900_MDC        equ      0x00000040
    SIS900_MDDIR      equ      0x00000020
    SIS900_MDIO       equ      0x00000010  ; 7016 specific
    SIS900_EECS       equ      0x00000008
    SIS900_EECLK      equ      0x00000004
    SIS900_EEDO       equ      0x00000002
    SIS900_EEDI       equ      0x00000001
;SIS900 TX Configuration Register Bits
    SIS900_ATP        equ      0x10000000 ;Automatic Transmit Padding
    SIS900_MLB        equ      0x20000000 ;Mac Loopback Enable
    SIS900_HBI        equ      0x40000000 ;HeartBeat Ignore (Req for full-dup)
    SIS900_CSI        equ      0x80000000 ;CarrierSenseIgnore (Req for full-du
;SIS900 RX Configuration Register Bits
    SIS900_AJAB       equ      0x08000000 ;
    SIS900_ATX        equ      0x10000000 ;Accept Transmit Packets
    SIS900_ARP        equ      0x40000000 ;accept runt packets (<64bytes)
    SIS900_AEP        equ      0x80000000 ;accept error packets
;SIS900 Interrupt Reigster Bits
    SIS900_WKEVT           equ      0x10000000
    SIS900_TxPAUSEEND      equ      0x08000000
    SIS900_TxPAUSE         equ      0x04000000
    SIS900_TxRCMP          equ      0x02000000
    SIS900_RxRCMP          equ      0x01000000
    SIS900_DPERR           equ      0x00800000
    SIS900_SSERR           equ      0x00400000
    SIS900_RMABT           equ      0x00200000
    SIS900_RTABT           equ      0x00100000
    SIS900_RxSOVR          equ      0x00010000
    SIS900_HIBERR          equ      0x00008000
    SIS900_SWINT           equ      0x00001000
    SIS900_MIBINT          equ      0x00000800
    SIS900_TxURN           equ      0x00000400
    SIS900_TxIDLE          equ      0x00000200
    SIS900_TxERR           equ      0x00000100
    SIS900_TxDESC          equ      0x00000080
    SIS900_TxOK            equ      0x00000040
    SIS900_RxORN           equ      0x00000020
    SIS900_RxIDLE          equ      0x00000010
    SIS900_RxEARLY         equ      0x00000008
    SIS900_RxERR           equ      0x00000004
    SIS900_RxDESC          equ      0x00000002
    SIS900_RxOK            equ      0x00000001
;SIS900 Interrupt Enable Reigster Bits
    SIS900_IE      equ      0x00000001
;SIS900 Revision ID
        SIS900B_900_REV       equ      0x03
        SIS630A_900_REV       equ      0x80
        SIS630E_900_REV       equ      0x81
        SIS630S_900_REV       equ      0x82
        SIS630EA1_900_REV     equ      0x83
        SIS630ET_900_REV      equ      0x84
        SIS635A_900_REV       equ      0x90
        SIS900_960_REV        equ      0x91
;SIS900 Receive Filter Control Register Bits
    SIS900_RFEN          equ 0x80000000
    SIS900_RFAAB         equ 0x40000000
    SIS900_RFAAM         equ 0x20000000
    SIS900_RFAAP         equ 0x10000000
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
    SIS900_EEread          equ 0x0180
    SIS900_EEwrite         equ 0x0140
    SIS900_EEerase         equ 0x01C0
    SIS900_EEwriteEnable   equ 0x0130
    SIS900_EEwriteDisable  equ 0x0100
    SIS900_EEeraseAll      equ 0x0120
    SIS900_EEwriteAll      equ 0x0110
    SIS900_EEaddrMask      equ 0x013F
    SIS900_EEcmdShift      equ 16
;For SiS962 or SiS963, request the eeprom software access
        SIS900_EEREQ    equ 0x00000400
        SIS900_EEDONE   equ 0x00000200
        SIS900_EEGNT    equ 0x00000100
;General Varibles
        SIS900_pci_revision:     db       0
        SIS900_Status                dd   0x03000000
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
sis900_get_mac_func:    dd 0
sis900_special_func:    dd 0
sis900_table_entries:   db 8

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
   mov     al, 4
   mov     bh, [pci_dev]
   mov     ecx, 0
   mov     ah, [pci_bus]
   mov     bl, 0x40
   call    pci_write_reg
;*******Set some PCI Settings*********
   call    SIS900_adjust_pci_device
;*****Get Card Revision******
   mov     al, 1                                        ;one byte to read
   mov     bh, [pci_dev]
   mov     ah, [pci_bus]
   mov     bl, 0x08                                 ;Revision Register
   call    pci_read_reg
   mov [SIS900_pci_revision], al        ;save the revision for later use
;****** Look up through the sis900_specific_table
   mov     esi,sis900_specific_table
.probe_loop:
   cmp     dword [esi],0                ; Check if we reached end of the list
   je      .probe_loop_failed
   cmp     al,[esi]                     ; Check if revision is OK
   je      .probe_loop_ok
   add     esi,12                       ; Advance to next entry
   jmp     .probe_loop
.probe_loop_failed:
   jmp     SIS900_Probe_Unsupported
;*********Find Get Mac Function*********
.probe_loop_ok:
   mov      eax,[esi+4]         ; Get pointer to "get MAC" function
   mov      [sis900_get_mac_func],eax
   mov      eax,[esi+8]         ; Get pointer to special initialization fn
   mov      [sis900_special_func],eax
;******** Get MAC ********
   call     dword [sis900_get_mac_func]
;******** Call special initialization fn if requested ********
   cmp      dword [sis900_special_func],0
   je       .no_special_init
   call     dword [sis900_special_func]
.no_special_init:
;******** Set table entries ********
   mov      al,[SIS900_pci_revision]
   cmp      al,SIS635A_900_REV
   jae      .ent16
   cmp      al,SIS900B_900_REV
   je       .ent16
   jmp      .ent8
.ent16:
   mov      byte [sis900_table_entries],16
.ent8:
;*******Probe for mii transceiver*******
;TODO!!*********************
;*******Initialize Device*******
   call sis900_init
   ret

SIS900_Probe_Unsupported:
if defined SIS900_DEBUG
   mov     esi, SIS900_Debug_Str_Unsupported
   call    sys_msg_board_str
end if
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
   call SIS900_reset               ;Done
   call SIS900_init_rxfilter   ;Done
   call SIS900_init_txd        ;Done
   call SIS900_init_rxd            ;Done
   call SIS900_set_rx_mode     ;done
   call SIS900_set_tx_mode
   ;call SIS900_check_mode
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
   ;******Disable Interrupts and reset Receive Filter*******
   mov      ebp, [io_addr]      ; base address
   xor      eax, eax            ; 0 to initialize
   lea      edx,[ebp+SIS900_ier]
   out      dx, eax                     ; Write 0 to location
   lea      edx,[ebp+SIS900_imr]
   out      dx, eax                     ; Write 0 to location
   lea      edx,[ebp+SIS900_rfcr]
   out      dx, eax                     ; Write 0 to location
   ;*******Reset Card***********************************************
   lea      edx,[ebp+SIS900_cr]
   in       eax, dx                             ; Get current Command Register
   or       eax, SIS900_RESET           ; set flags
   or       eax, SIS900_RxRESET     ;
   or           eax, SIS900_TxRESET         ;
   out      dx, eax                             ; Write new Command Register
   ;*******Wait Loop************************************************
   lea      edx,[ebp+SIS900_isr]
   mov      ecx, [SIS900_Status]    ; Status we would like to see from card
   mov      ebx, 2001               ; only loop 1000 times
SIS900_Wait:
   dec      ebx                                     ; 1 less loop
   jz       SIS900_DoneWait_e           ; 1000 times yet?
   in       eax, dx                                 ; move interrup status to eax
   and      eax, ecx
   xor      ecx, eax
   jz       SIS900_DoneWait
   jmp      SIS900_Wait
SIS900_DoneWait_e:
if defined SIS900_DEBUG
   mov esi, SIS900_Debug_Reset_Failed
   call sys_msg_board_str
end if
SIS900_DoneWait:
   ;*******Set Configuration Register depending on Card Revision********
   lea      edx,[ebp+SIS900_cfg]
   mov      eax, SIS900_PESEL               ; Configuration Register Bit
   mov      bl, [SIS900_pci_revision]   ; card revision
   mov      cl, SIS635A_900_REV         ; Check card revision
   cmp      bl, cl
   je       SIS900_RevMatch
   mov      cl, SIS900B_900_REV         ; Check card revision
   cmp      bl, cl
   je       SIS900_RevMatch
   out      dx, eax                                 ; no revision match
   jmp      SIS900_Reset_Complete
SIS900_RevMatch:                                        ; Revision match
   or       eax, SIS900_RND_CNT         ; Configuration Register Bit
   out      dx, eax
SIS900_Reset_Complete:
   mov      eax, [pci_data]
   mov      [eth_status], eax
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
   ;****Get Receive Filter Control Register ********
   mov      ebp, [io_addr]          ; base address
   lea      edx,[ebp+SIS900_rfcr]
   in       eax, dx                         ; get register
   push     eax
   ;****disable packet filtering before setting filter*******
   mov      eax, SIS900_RFEN    ;move receive filter enable flag
   not      eax                         ;1s complement
   pop      ebx                         ;and with our saved register
   and      eax, ebx                    ;disable receiver
   push     ebx                 ;save filter for another use
   out      dx, eax                     ;set receive disabled
   ;********load MAC addr to filter data register*********
   xor      ecx, ecx
SIS900_RXINT_Mac_Write:
   ;high word of eax tells card which mac byte to write
   mov      eax, ecx
   lea      edx,[ebp+SIS900_rfcr]
   shl      eax, 16                                             ;
   out      dx, eax                                             ;
   lea      edx,[ebp+SIS900_rfdr]
   mov      ax,  word [node_addr+ecx*2] ; Get Mac ID word
   out      dx, ax                                              ; Send Mac ID
   inc      cl                                                  ; send next word
   cmp      cl, 3                                               ; more to send?
   jne      SIS900_RXINT_Mac_Write
   ;********enable packet filitering *****
   pop      eax                             ;old register value
   lea      edx,[ebp+SIS900_rfcr]
   or       eax, SIS900_RFEN    ;enable filtering
   out      dx, eax             ;set register
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
   mov     [txd], dword 0       ;put link to next descriptor in link field
   mov     [txd+4],dword 0      ;clear status field
   mov     [txd+8], dword txb   ;save address to buffer ptr field
   ;*************** load Transmit Descriptor Register ***************
   mov     dx, [io_addr]            ; base address
   add     dx, SIS900_txdp      ; TX Descriptor Pointer
   mov     eax, txd                         ; First Descriptor
   out     dx, eax                              ; move the pointer
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
   xor      ecx,ecx
   mov      [cur_rx], cl                                        ;Set cuurent rx discriptor to 0
   ;******** init RX descriptors ********
SIS900_init_rxd_Loop:
    mov     eax, ecx                                        ;current descriptor
    imul    eax, 12                         ;
    mov     ebx, ecx                                        ;determine next link descriptor
    inc     ebx                             ;
    cmp     ebx, NUM_RX_DESC                ;
    jne     SIS900_init_rxd_Loop_0          ;
    xor     ebx, ebx                        ;
SIS900_init_rxd_Loop_0:                    ;
    imul    ebx, 12                         ;
    add     ebx, rxd                        ;
    mov     [rxd+eax], ebx                                      ;save link to next descriptor
    mov     [rxd+eax+4],dword RX_BUFF_SZ        ;status bits init to buf size
    mov     ebx, ecx                                            ;find where the buf is located
    imul    ebx,RX_BUFF_SZ                  ;
    add     ebx, rxb                        ;
    mov     [rxd+eax+8], ebx                            ;save buffer pointer
    inc     ecx                                                     ;next descriptor
    cmp     ecx, NUM_RX_DESC                ;
    jne     SIS900_init_rxd_Loop            ;
    ;********* load Receive Descriptor Register with address of first
    ; descriptor*********
    mov     dx, [io_addr]
    add     dx, SIS900_rxdp
    mov     eax, rxd
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
   mov      ebp,[io_addr]
   lea      edx,[ebp+SIS900_cr]
   in       eax, dx                         ; Get current Command Register
   or       eax, SIS900_TxENA   ;Enable Receive
   out      dx, eax
   lea      edx,[ebp+SIS900_txcfg]; Transmit config Register offset
   mov      eax, SIS900_ATP             ;allow automatic padding
   or       eax, SIS900_HBI             ;allow heartbeat ignore
   or       eax, SIS900_CSI             ;allow carrier sense ignore
   or       eax, 0x00600000     ;Max DMA Burst
   or       eax, 0x00000100     ;TX Fill Threshold
   or       eax, 0x00000020     ;TX Drain Threshold
   out      dx, eax
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
SIS900_mc_filter: times 16 dw 0
SIS900_set_rx_mode:
   mov      ebp,[io_addr]
    ;**************update Multicast Hash Table in Receive Filter
   mov      ebx, 0xffff
   xor      cl, cl
SIS900_set_rx_mode_Loop:
   mov      eax, ecx
   shl      eax, 1
   mov      [SIS900_mc_filter+eax], ebx
   lea      edx,[ebp+SIS900_rfcr]           ; Receive Filter Control Reg offset
   mov      eax, 4                                          ;determine table entry
   add      al, cl
   shl      eax, 16
   out      dx, eax                                         ;tell card which entry to modify
   lea      edx,[ebp+SIS900_rfdr]           ; Receive Filter Control Reg offset
   mov      eax, ebx                                ;entry value
   out      dx, ax                                          ;write value to table in card
   inc      cl                                              ;next entry
   cmp      cl,[sis900_table_entries]   ;
   jl       SIS900_set_rx_mode_Loop
   ;*******Set Receive Filter Control Register*************
   lea      edx,[ebp+SIS900_rfcr]       ; Receive Filter Control Register offset
   mov      eax, SIS900_RFAAB           ;accecpt all broadcast packets
   or       eax, SIS900_RFAAM           ;accept all multicast packets
   or       eax, SIS900_RFAAP           ;Accept all packets
   or       eax, SIS900_RFEN            ;enable receiver filter
   out      dx, eax
   ;******Enable Receiver************
   lea      edx,[ebp+SIS900_cr] ; Command Register offset
   in       eax, dx                         ; Get current Command Register
   or       eax, SIS900_RxENA   ;Enable Receive
   out      dx, eax
   ;*********Set
   lea      edx,[ebp+SIS900_rxcfg]      ; Receive Config Register offset
   mov      eax, SIS900_ATX                     ;Accept Transmit Packets
                                    ; (Req for full-duplex and PMD Loopback)
   or       eax, 0x00600000                     ;Max DMA Burst
   or       eax, 0x00000002                     ;RX Drain Threshold, 8X8 bytes or 64bytes
   out      dx, eax                                     ;
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
   mov      ebp,[io_addr]
   ;**********Send Request for eeprom access*********************
   lea      edx,[ebp+SIS900_mear]               ; Eeprom access register
   mov      eax, SIS900_EEREQ                   ; Request access to eeprom
   out      dx, eax                                             ; Send request
   xor      ebx,ebx                                             ;
   ;******Loop 4000 times and if access not granted error out*****
SIS96X_Get_Mac_Wait:
   in       eax, dx                                     ;get eeprom status
   and      eax, SIS900_EEGNT       ;see if eeprom access granted flag is set
   jnz      SIS900_Got_EEP_Access       ;if it is, go access the eeprom
   inc      ebx                                         ;else keep waiting
   cmp      ebx, 4000                           ;have we tried 4000 times yet?
   jl       SIS96X_Get_Mac_Wait     ;if not ask again
   xor      eax, eax                ;return zero in eax indicating failure
   ;*******Debug **********************
if defined SIS900_DEBUG
   mov esi,SIS900_Debug_Str_GetMac_Failed
   call sys_msg_board_str
end if
   jmp SIS960_get_mac_addr_done
   ;**********EEprom access granted, read MAC from card*************
SIS900_Got_EEP_Access:
    ; zero based so 3-16 bit reads will take place
   mov      ecx, 2
SIS96x_mac_read_loop:
   mov      eax, SIS900_EEPROMMACAddr    ;Base Mac Address
   add      eax, ecx                                 ;Current Mac Byte Offset
   push     ecx
   call     sis900_read_eeprom           ;try to read 16 bits
   pop      ecx
   mov      [node_addr+ecx*2], ax        ;save 16 bits to the MAC ID varible
   dec      ecx                          ;one less word to read
   jns      SIS96x_mac_read_loop         ;if more read more
   mov      eax, 1                       ;return non-zero indicating success
   ;*******Debug Print MAC ID to debug window**********************
if defined SIS900_DEBUG
   mov esi,SIS900_Debug_Str_GetMac_Address2
   call sys_msg_board_str
   mov edx, node_addr
   call Create_Mac_String
end if
   ;**********Tell EEPROM We are Done Accessing It*********************
SIS960_get_mac_addr_done:
   lea      edx,[ebp+SIS900_mear]               ; Eeprom access register
   mov      eax, SIS900_EEDONE           ;tell eeprom we are done
   out      dx,eax
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
   mov      eax, SIS900_EEPROMSignature  ;Base Eeprom Signature
   call     sis900_read_eeprom           ;try to read 16 bits
   cmp ax, 0xffff
   je SIS900_Bad_Eeprom
   cmp ax, 0
   je SIS900_Bad_Eeprom
   ;**************Read MacID**************
   ; zero based so 3-16 bit reads will take place
   mov      ecx, 2
SIS900_mac_read_loop:
   mov      eax, SIS900_EEPROMMACAddr    ;Base Mac Address
   add      eax, ecx                                 ;Current Mac Byte Offset
   push     ecx
   call     sis900_read_eeprom           ;try to read 16 bits
   pop      ecx
   mov      [node_addr+ecx*2], ax        ;save 16 bits to the MAC ID storage
   dec      ecx                          ;one less word to read
   jns      SIS900_mac_read_loop         ;if more read more
   mov      eax, 1                       ;return non-zero indicating success
   ;*******Debug Print MAC ID to debug window**********************
if defined SIS900_DEBUG
   mov esi,SIS900_Debug_Str_GetMac_Address
   call sys_msg_board_str
   mov edx, node_addr
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
    mov     ebp,[io_addr]
    lea     edx,[ebp+SIS900_rfcr]
    in      eax,dx
    mov     edi,eax ; EDI=rfcrSave
    lea     edx,[ebp+SIS900_cr]
    or      eax,SIS900_RELOAD
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
    mov     esi,node_addr
.get_mac_loop:
    lea     edx,[ebp+SIS900_rfcr]
    mov     eax,ecx
    shl     eax,SIS900_RFADDR_shift
    out     dx,eax
    lea     edx,[ebp+SIS900_rfdr]
    in      eax,dx
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
    mov     edx, node_addr
    call    Create_Mac_String
end if
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
   mov       ebp,[io_addr]
   mov       ebx, eax              ;location of Mac byte to read
   or        ebx, SIS900_EEread    ;
   lea       edx,[ebp+SIS900_mear] ; Eeprom access register
   xor       eax, eax              ; start send
   out       dx,eax
   call      SIS900_Eeprom_Delay_1
   mov       eax, SIS900_EECLK
   out       dx, eax
   call      SIS900_Eeprom_Delay_1
    ;************ Shift the read command (9) bits out. *********
   mov       cl, 8                                      ;
sis900_read_eeprom_Send:
   mov       eax, 1
   shl       eax, cl
   and       eax, ebx
   jz SIS900_Read_Eeprom_8
   mov       eax, 9
   jmp       SIS900_Read_Eeprom_9
SIS900_Read_Eeprom_8:
   mov       eax, 8
SIS900_Read_Eeprom_9:
   out       dx, eax
   call      SIS900_Eeprom_Delay_1
   or        eax, SIS900_EECLK
   out       dx, eax
   call      SIS900_Eeprom_Delay_1
   cmp       cl, 0
   je        sis900_read_eeprom_Send_Done
   dec       cl
   jmp       sis900_read_eeprom_Send
   ;*********************
sis900_read_eeprom_Send_Done:
   mov       eax, SIS900_EECS           ;
   out       dx, eax
   call      SIS900_Eeprom_Delay_1
    ;********** Read 16-bits of data in ***************
    mov      cx, 16                             ;16 bits to read
sis900_read_eeprom_Send2:
    mov      eax, SIS900_EECS
    out      dx, eax
    call     SIS900_Eeprom_Delay_1
    or       eax, SIS900_EECLK
    out      dx, eax
    call     SIS900_Eeprom_Delay_1
    in       eax, dx
    shl      ebx, 1
    and      eax, SIS900_EEDO
    jz       SIS900_Read_Eeprom_0
    or       ebx, 1
SIS900_Read_Eeprom_0:
   dec       cx
   jnz       sis900_read_eeprom_Send2
   ;************** Terminate the EEPROM access. **************
   xor       eax, eax
   out       dx, eax
   call      SIS900_Eeprom_Delay_1
   mov       eax, SIS900_EECLK
   out       dx, eax
   mov       eax, ebx
   and       eax, 0x0000ffff                    ;return only 16 bits
   pop       ebx
   pop       ecx
   pop       edx
   pop       esi
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

;***************************************************************************
;   Function
;      SIS900_poll
;   Description
;      polls card to see if there is a packet waiting
;
;  Currently only supports one descriptor per packet, if packet is fragmented
;  between multiple descriptors you will lose part of the packet
;***************************************************************************
if defined SIS900_DEBUG
SIS900_Debug_Pull_Packet_good db 'Good Packet Waiting: ',13,10,0
SIS900_Debug_Pull_Bad_Packet_Status db 'Bad Packet Waiting: Status',13,10,0
SIS900_Debug_Pull_Bad_Packet_Size db 'Bad Packet Waiting: Size',13,10,0
end if
SIS900_poll:
    ;**************Get Status **************
    xor       eax, eax                      ;get RX_Status
    mov      [eth_rx_data_len], ax
    mov       al, [cur_rx]          ;find current discriptor
    imul      eax, 12               ;
    mov       ecx, [rxd+eax+4]          ; get receive status
    ;**************Check Status **************
    mov       ebx, ecx                          ;move status
    ;Check RX_Status to see if packet is waiting
    and       ebx, 0x80000000
    jnz       SIS900_poll_IS_packet
    ret
   ;**********There is a packet waiting check it for errors**************
SIS900_poll_IS_packet:
    mov       ebx, ecx                          ;move status
    and       ebx, 0x67C0000            ;see if there are any errors
    jnz       SIS900_Poll_Error_Status
   ;**************Check size of packet*************
   and       ecx, SIS900_DSIZE                                  ;get packet size minus CRC
   cmp       cx, SIS900_CRC_SIZE
   ;make sure packet contains data
   jle       SIS900_Poll_Error_Size
   ;*******Copy Good Packet to receive buffer******
   sub      cx, SIS900_CRC_SIZE                             ;dont want crc
   mov      word [eth_rx_data_len], cx          ;save size of packet
   ;**********Continue copying packet****************
   push     ecx
   ; first copy dword-wise, divide size by 4
   shr      ecx, 2
   mov      esi, [rxd+eax+8]                            ; set source
   mov      edi, Ether_buffer               ; set destination
   cld                                                                          ; clear direction
   rep      movsd                                                       ; copy the dwords
   pop      ecx
   and      ecx, 3                                                  ;
   rep      movsb
   ;********Debug, tell user we have a good packet*************
if defined SIS900_DEBUG
   mov      esi, SIS900_Debug_Pull_Packet_good
   call     sys_msg_board_str
end if
   jmp SIS900_Poll_Cnt                      ;
   ;*************Error occured let user know through debug window***********
SIS900_Poll_Error_Status:
if defined SIS900_DEBUG
                mov      esi, SIS900_Debug_Pull_Bad_Packet_Status
                call     sys_msg_board_str
end if
                jmp      SIS900_Poll_Cnt
SIS900_Poll_Error_Size:
if defined SIS900_DEBUG
                mov      esi, SIS900_Debug_Pull_Bad_Packet_Size
                call     sys_msg_board_str
end if
   ;*************Increment to next available descriptor**************
SIS900_Poll_Cnt:
    ;Reset status, allow ethernet card access to descriptor
   mov      ecx, RX_BUFF_SZ
   mov      [rxd+eax+4], ecx                ;
   inc      [cur_rx]                                            ;get next descriptor
   and      [cur_rx],3                      ;only 4 descriptors 0-3
   ;******Enable Receiver************
   mov          ebp, [io_addr]      ; Base Address
   lea      edx,[ebp+SIS900_cr] ; Command Register offset
   in       eax, dx                         ; Get current Command Register
   or       eax, SIS900_RxENA   ;Enable Receive
   out      dx, eax
   ret
;***************************************************************************
;   Function
;      SIS900_transmit
;   Description
;      Transmits a packet of data via the ethernet card
;         Pointer to 48 bit destination address in edi
;         Type of packet in bx
;         size of packet in ecx
;         pointer to packet data in esi
;
;      only one transmit descriptor is used
;
;***************************************************************************
if defined SIS900_DEBUG
SIS900_Debug_Transmit_Packet db 'Transmitting Packet: ',13,10,0
SIS900_Debug_Transmit_Packet_Err db 'Transmitting Packet Error: ',13,10,0
end if
SIS900_transmit:
   mov          ebp, [io_addr]      ; Base Address
   ;******** Stop the transmitter ********
   lea      edx,[ebp+SIS900_cr] ; Command Register offset
   in       eax, dx                         ; Get current Command Register
   or       eax, SIS900_TxDIS   ; Disable Transmitter
   out      dx, eax
   ;*******load Transmit Descriptor Register *******
   lea      edx,[ebp+SIS900_txdp]
   mov      eax, txd
   out      dx, eax
   ;******* copy packet to descriptor*******
   push    esi
   mov     esi, edi                ;copy destination addess
   mov     edi, txb
   cld
   movsd
   movsw
   mov     esi, node_addr  ;copy my mac address
   movsd
   movsw
   mov     [edi], bx       ;copy packet type
   add     edi, 2
   pop     esi             ;restore pointer to source of packet
   push    ecx             ;save packet size
   shr     ecx, 2          ;divide by 4, size in bytes send in dwords
   rep     movsd                   ;copy data to decriptor
   pop     ecx                     ;restore packet size
   push    ecx             ;save packet size
   and     ecx, 3          ;last three bytes if not a multiple of 4
   rep     movsb
   ;**************set length tag**************
   pop     ecx                           ;restore packet size
   add     ecx, SIS900_ETH_HLEN  ;add header to length
   and     ecx, SIS900_DSIZE     ;
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
   mov      [txd+4], dword 0x80000000                   ;card owns descriptor
   or       [txd+4], ecx                                                ;set size of packet
if defined SIS900_DEBUG
   mov      esi, SIS900_Debug_Transmit_Packet
   call     sys_msg_board_str
end if
   ;***************restart the transmitter ********
   lea      edx,[ebp+SIS900_cr]
   in       eax, dx                         ; Get current Command Register
   or       eax, SIS900_TxENA   ; Enable Transmitter
   out      dx, eax
   ;****make sure packet transmitted successfully****
;   mov      esi,10
;   call     delay_ms
   mov      eax, [txd+4]
   and      eax, 0x6200000
   jz       SIS900_transmit_OK
   ;**************Tell user there was an error through debug window
if defined SIS900_DEBUG
   mov      esi, SIS900_Debug_Transmit_Packet_Err
   call     sys_msg_board_str
end if
SIS900_transmit_OK:
   ;******** Disable interrupts by clearing the interrupt mask. ********
   lea      edx,[ebp+SIS900_imr]            ; Interupt Mask Register
   xor      eax, eax
   out      dx,eax
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
Create_Mac_String_done:                                 ;Insert CR and Zero Terminate
   mov [Mac_str_build+2+ecx*3],byte 13
   mov [Mac_str_build+3+ecx*3],byte 10
   mov [Mac_str_build+4+ecx*3],byte 0
   mov esi, Mac_str_build
   call sys_msg_board_str                               ;Print String to message board
   popa
   ret
end if
;***************************************************************************
;*      Set device to be a busmaster in case BIOS neglected to do so.
;*      Also adjust PCI latency timer to a reasonable value, 64.
;***************************************************************************
SIS900_adjust_pci_device:
   ;*******Get current setting************************
   mov     al, 2                                        ;read a word
   mov     bh, [pci_dev]
   mov     ah, [pci_bus]
   mov     bl, 0x04                                 ;from command Register
   call    pci_read_reg
   ;******see if its already set as bus master********
   mov      bx, ax
   and      bx,5
   cmp      bx,5
   je       SIS900_adjust_pci_device_Latency
   ;******Make card a bus master*******
   mov      cx, ax                              ;value to write
   mov     bh, [pci_dev]
   mov     al, 2                                ;write a word
   or       cx,5
   mov     ah, [pci_bus]
   mov     bl, 0x04                             ;to command register
   call    pci_write_reg
   ;******Check latency setting***********
SIS900_adjust_pci_device_Latency:
   ;*******Get current latency setting************************
   mov     al, 1                                        ;read a byte
   mov     bh, [pci_dev]
   mov     ah, [pci_bus]
   mov     bl, 0x0D                                 ;from Lantency Timer Register
   call    pci_read_reg
   ;******see if its aat least 64 clocks********
   cmp      ax,64
   jge      SIS900_adjust_pci_device_Done
   ;******Set latency to 32 clocks*******
   mov     cx, 64                               ;value to write
   mov     bh, [pci_dev]
   mov     al, 1                                ;write a byte
   mov     ah, [pci_bus]
   mov     bl, 0x0D                             ;to Lantency Timer Register
   call    pci_write_reg
   ;******Check latency setting***********
SIS900_adjust_pci_device_Done:
   ret
