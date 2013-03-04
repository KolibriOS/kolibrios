;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  3Com network driver for KolibriOS                           ;;
;;                                                              ;;
;;  Ported to KolibriOS net-branch by hidnplayr (28/05/10)      ;;
;;                                                              ;;
;;  Thanks to: scrap metal recyclers, whom provide me with      ;;
;;                         loads of hardware                    ;;
;;             diamond: who makes me understand KolibriOS       ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                         ;;
;;  3C59X.INC                                                              ;;
;;                                                                         ;;
;;  Ethernet driver for Menuet OS                                          ;;
;;                                                                         ;;
;;  Driver for 3Com fast etherlink 3c59x and                               ;;
;;         etherlink XL 3c900 and 3c905 cards                              ;;
;;  References:                                                            ;;
;;    www.3Com.com - data sheets                                           ;;
;;    DP83840A.pdf - ethernet physical layer                               ;;
;;    3c59x.c - linux driver                                               ;;
;;    ethernet driver template by Mike Hibbett                             ;;
;;                                                                         ;;
;;  Credits                                                                ;;
;;   Mike Hibbett,                                                         ;;
;;         who kindly supplied me with a 3Com905C-TX-M card                ;;
;;                                                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Copyright (c) 2004, Endre Kozma <endre.kozma@axelero.hu>
;; All rights reserved.
;;
;; Redistribution  and  use  in  source  and  binary  forms, with or without
;; modification, are permitted provided  that  the following  conditions are
;; met:
;;
;; 1. Redistributions of source code must retain the above  copyright notice,
;;    this list of conditions and the following disclaimer.
;;
;; 2. Redistributions  in  binary form  must  reproduce  the above copyright
;;    notice, this  list of conditions  and the  following disclaimer in the
;;    documentation and/or other  materials  provided with  the distribution.
;;
;; 3. The name of the author may not be used to  endorse or promote products
;;    derived from this software without  specific prior  written permission.
;;
;; THIS SOFTWARE IS  PROVIDED  BY  THE  AUTHOR  ``AS IS'' AND ANY EXPRESS OR
;; IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
;; OF  MERCHANTABILITY AND FITNESS  FOR A PARTICULAR  PURPOSE ARE DISCLAIMED.
;; IN  NO  EVENT  SHALL  THE  AUTHOR  BE  LIABLE  FOR  ANY  DIRECT, INDIRECT,
;; INCIDENTAL, SPECIAL, EXEMPLARY, OR  CONSEQUENTIAL DAMAGES (INCLUDING, BUT
;; NOT LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
;; DATA, OR  PROFITS; OR  BUSINESS  INTERRUPTION)  HOWEVER CAUSED AND ON ANY
;; THEORY OF  LIABILITY, WHETHER IN  CONTRACT,  STRICT  LIABILITY,  OR  TORT
;; (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY WAY OUT OF THE USE OF
;; THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;;
;;  History
;;  =======
;;  $Log: 3C59X.INC,v $
;;  Revision 1.3  2004/07/11 12:21:12  kozma
;;  Support of vortex chips (3c59x) added.
;;  Support of 3c920 and 3c982 added.
;;  Corrections.
;;
;;  Revision 1.2  2004/06/12 19:40:20  kozma
;;  Function e3c59x_set_available_media added in order to set
;;  the default media in case auto detection finds no valid link.
;;  Incorrect mii check removed (3c900 Cyclone works now).
;;  Cleanups.
;;
;;  Revision 1.1  2004/06/12 18:27:15  kozma
;;  Initial revision
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format MS COFF

        API_VERSION             =   0x01000100
        DRIVER_VERSION          =   5

        MAX_DEVICES             =   16
        FORCE_FD                =   0  ; forcing full duplex mode makes sense at some cards and link types
        PROMISCIOUS             =   0  ; enables promiscous mode

        DEBUG                   =   1
        __DEBUG__               =   1
        __DEBUG_LEVEL__         =   1

include 'proc32.inc'
include 'imports.inc'
include 'fdo.inc'
include 'netdrv.inc'

public START
public service_proc
public version

struc DPD {     ; Download Packet Descriptor

        .next_ptr         dd ?
        .frame_start_hdr  dd ?
        .frag_addr        dd ?    ; for packet data
        .frag_len         dd ?    ; for packet data
        .realaddr         dd ?
        .size             = 32
}

virtual at 0
  dpd DPD
end virtual


struc UPD {     ; Upload Packet Descriptor

        .next_ptr       dd ?
        .pkt_status     dd ?
        .frag_addr      dd ?
        .frag_len       dd ?    ; for packet data
        .realaddr       dd ?
        .size           = 32

}

virtual at 0
  upd UPD
end virtual

; Registers
        REG_POWER_MGMT_CTRL     =   0x7c
        REG_UP_LIST_PTR         =   0x38
        REG_UP_PKT_STATUS       =   0x30
        REG_TX_FREE_THRESH      =   0x2f
        REG_DN_LIST_PTR         =   0x24
        REG_DMA_CTRL            =   0x20
        REG_TX_STATUS           =   0x1b
        REG_RX_STATUS           =   0x18
        REG_TX_DATA             =   0x10

; Common window registers
        REG_INT_STATUS          =   0xe
        REG_COMMAND             =   0xe

; Register window 7
        REG_MASTER_STATUS       =   0xc
        REG_POWER_MGMT_EVENT    =   0xc
        REG_MASTER_LEN          =   0x6
        REG_VLAN_ETHER_TYPE     =   0x4
        REG_VLAN_MASK           =   0x0
        REG_MASTER_ADDRESS      =   0x0

; Register window 6
        REG_BYTES_XMITTED_OK    =   0xc
        REG_BYTES_RCVD_OK       =   0xa
        REG_UPPER_FRAMES_OK     =   0x9
        REG_FRAMES_DEFERRED     =   0x8
        REG_FRAMES_RCVD_OK      =   0x7
        REG_FRAMES_XMITTED_OK   =   0x6
        REG_RX_OVERRUNS         =   0x5
        REG_LATE_COLLISIONS     =   0x4
        REG_SINGLE_COLLISIONS   =   0x3
        REG_MULTIPLE_COLLISIONS =   0x2
        REG_SQE_ERRORS          =   0x1
        REG_CARRIER_LOST        =   0x0

; Register window 5
        REG_INDICATION_ENABLE   =   0xc
        REG_INTERRUPT_ENABLE    =   0xa
        REG_TX_RECLAIM_THRESH   =   0x9
        REG_RX_FILTER           =   0x8
        REG_RX_EARLY_THRESH     =   0x6
        REG_TX_START_THRESH     =   0x0

; Register window 4
        REG_UPPER_BYTES_OK      =   0xe
        REG_BAD_SSD             =   0xc
        REG_MEDIA_STATUS        =   0xa
        REG_PHYSICAL_MGMT       =   0x8
        REG_NETWORK_DIAGNOSTIC  =   0x6
        REG_FIFO_DIAGNOSTIC     =   0x4
        REG_VCO_DIAGNOSTIC      =   0x2 ; may not supported

; Bits in register window 4
        BIT_AUTOSELECT          =   24

; Register window 3
        REG_TX_FREE             =   0xc
        REG_RX_FREE             =   0xa
        REG_MEDIA_OPTIONS       =   0x8
        REG_MAC_CONTROL         =   0x6
        REG_MAX_PKT_SIZE        =   0x4
        REG_INTERNAL_CONFIG     =   0x0

; Register window 2
        REG_RESET_OPTIONS       =   0xc
        REG_STATION_MASK_HI     =   0xa
        REG_STATION_MASK_MID    =   0x8
        REG_STATION_MASK_LO     =   0x6
        REG_STATION_ADDRESS_HI  =   0x4
        REG_STATION_ADDRESS_MID =   0x2
        REG_STATION_ADDRESS_LO  =   0x0

; Register window 1
        REG_TRIGGER_BITS        =   0xc
        REG_SOS_BITS            =   0xa
        REG_WAKE_ON_TIMER       =   0x8
        REG_SMB_RXBYTES         =   0x7
        REG_SMB_DIAG            =   0x5
        REG_SMB_ARB             =   0x4
        REG_SMB_STATUS          =   0x2
        REG_SMB_ADDRESS         =   0x1
        REG_SMB_FIFO_DATA       =   0x0

; Register window 0
        REG_EEPROM_DATA         =   0xc
        REG_EEPROM_COMMAND      =   0xa
        REG_BIOS_ROM_DATA       =   0x8
        REG_BIOS_ROM_ADDR       =   0x4

; Physical management bits
        BIT_MGMT_DIR            =   2 ; drive with the data written in mgmtData
        BIT_MGMT_DATA           =   1 ; MII management data bit
        BIT_MGMT_CLK            =   0 ; MII management clock

; MII commands
        MII_CMD_MASK            =   (1111b shl 10)
        MII_CMD_READ            =   (0110b shl 10)
        MII_CMD_WRITE           =   (0101b shl 10)

; MII registers
        REG_MII_BMCR            =   0 ; basic mode control register
        REG_MII_BMSR            =   1 ; basic mode status register
        REG_MII_ANAR            =   4 ; auto negotiation advertisement register
        REG_MII_ANLPAR          =   5 ; auto negotiation link partner ability register
        REG_MII_ANER            =   6 ; auto negotiation expansion register

; MII bits
        BIT_MII_AUTONEG_COMPLETE     =   5 ; auto-negotiation complete
        BIT_MII_PREAMBLE_SUPPRESSION =   6

; eeprom bits and commands
        EEPROM_CMD_READ         =   0x80
        EEPROM_BIT_BUSY         =   15

; eeprom registers
        EEPROM_REG_OEM_NODE_ADDR =   0xa
        EEPROM_REG_CAPABILITIES  =   0x10

; Commands for command register
        SELECT_REGISTER_WINDOW  =   (1 shl 11)

        IS_VORTEX               =   0x1
        IS_BOOMERANG            =   0x2
        IS_CYCLONE              =   0x4
        IS_TORNADO              =   0x8
        EEPROM_8BIT             =   0x10
        HAS_PWR_CTRL            =   0x20
        HAS_MII                 =   0x40
        HAS_NWAY                =   0x80
        HAS_CB_FNS              =   0x100
        INVERT_MII_PWR          =   0x200
        INVERT_LED_PWR          =   0x400
        MAX_COLLISION_RESET     =   0x800
        EEPROM_OFFSET           =   0x1000
        HAS_HWCKSM              =   0x2000
        EXTRA_PREAMBLE          =   0x4000

; Status
        IntLatch                =   0x0001
        HostError               =   0x0002
        TxComplete              =   0x0004
        TxAvailable             =   0x0008
        RxComplete              =   0x0010
        RxEarly                 =   0x0020
        IntReq                  =   0x0040
        StatsFull               =   0x0080
        DMADone                 =   0x0100
        DownComplete            =   0x0200
        UpComplete              =   0x0400
        DMAInProgress           =   0x0800 ; 1 shl 11  (DMA controller is still busy)
        CmdInProgress           =   0x1000 ; 1 shl 12  (EL3_CMD is still busy)

        S_5_INTS                =   HostError + RxEarly + UpComplete + DownComplete ;+ TxComplete + RxComplete  + TxAvailable

; Commands
        TotalReset              =   0 shl 11
        SelectWindow            =   1 shl 11
        StartCoax               =   2 shl 11
        RxDisable               =   3 shl 11
        RxEnable                =   4 shl 11
        RxReset                 =   5 shl 11
        UpStall                 =   6 shl 11
        UpUnstall               =   (6 shl 11)+1
        DownStall               =   (6 shl 11)+2
        DownUnstall             =   (6 shl 11)+3
        RxDiscard               =   8 shl 11
        TxEnable                =   9 shl 11
        TxDisable               =   10 shl 11
        TxReset                 =   11 shl 11
        FakeIntr                =   12 shl 11
        AckIntr                 =   13 shl 11
        SetIntrEnb              =   14 shl 11
        SetStatusEnb            =   15 shl 11
        SetRxFilter             =   16 shl 11
        SetRxThreshold          =   17 shl 11
        SetTxThreshold          =   18 shl 11
        SetTxStart              =   19 shl 11
        StartDMAUp              =   20 shl 11
        StartDMADown            =   (20 shl 11)+1
        StatsEnable             =   21 shl 11
        StatsDisable            =   22 shl 11
        StopCoax                =   23 shl 11
        SetFilterBit            =   25 shl 11

; Rx mode bits
        RxStation               =   1
        RxMulticast             =   2
        RxBroadcast             =   4
        RxProm                  =   8

; RX/TX buffers sizes
        MAX_ETH_PKT_SIZE        =   1536   ; max packet size
        NUM_RX_DESC             =   4      ; a power of 2 number
        NUM_TX_DESC             =   4      ; a power of 2 number
        MAX_ETH_FRAME_SIZE      =   1520        ; size of ethernet frame + bytes alignment

virtual at ebx

        device:

        ETH_DEVICE

        .dpd_buffer       rd (dpd.size*NUM_TX_DESC)/4
        .upd_buffer       rd (upd.size*NUM_RX_DESC)/4
        .curr_upd         dd ?
        .prev_dpd         dd ?

        .io_addr          dd ?
        .pci_bus          dd ?
        .pci_dev          dd ?
        .irq_line         db ?
                rb 3    ; alignment

        .prev_tx_frame            dd ?
        .ver_id                   db ?
        .full_bus_master          db ?
        .has_hwcksm               db ?
        .preamble                 db ?
        .dn_list_ptr_cleared      db ?

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

        DEBUGF 1,"Loading %s driver\n", my_service
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

        mov     edx, [ioctl]
        mov     eax, [IOCTL.io_code]

;------------------------------------------------------

        cmp     eax, 0 ;SRV_GETVERSION
        jne     @F

        cmp     [IOCTL.out_size], 4
        jb      .fail
        mov     eax, [IOCTL.output]
        mov     [eax], dword API_VERSION

        xor     eax, eax
        ret

;------------------------------------------------------
  @@:
        cmp     eax, 1 ;SRV_HOOK
        jne     .fail

        cmp     [IOCTL.inp_size], 3               ; Data input must be at least 3 bytes
        jb      .fail

        mov     eax, [IOCTL.input]
        cmp     byte [eax], 1                           ; 1 means device number and bus number (pci) are given
        jne     .fail                                   ; other types of this hardware dont exist

; check if the device is already listed

        mov     ecx, [VORTEX_DEVICES]
        test    ecx, ecx
        jz      .maybeboomerang

        mov     esi, VORTEX_LIST
        mov     eax, [IOCTL.input]                      ; get the pci bus and device numbers
        mov     ax , [eax+1]                            ;
  .nextdevice:
        mov     ebx, [esi]
        cmp     al, byte[device.pci_bus]
        jne     @f
        cmp     ah, byte[device.pci_dev]
        je      .find_devicenum                         ; Device is already loaded, let's find it's device number
       @@:
        add     esi, 4
        loop    .nextdevice


  .maybeboomerang:
        mov     ecx, [BOOMERANG_DEVICES]
        test    ecx, ecx
        jz      .firstdevice

        mov     esi, BOOMERANG_LIST
        mov     eax, [IOCTL.input]                      ; get the pci bus and device numbers
        mov     ax , [eax+1]                            ;
  .nextdevice2:
        mov     ebx, [esi]
        cmp     al, byte[device.pci_bus]
        jne     @f
        cmp     ah, byte[device.pci_dev]
        je      .find_devicenum                         ; Device is already loaded, let's find it's device number
       @@:
        add     esi, 4
        loop    .nextdevice2


; This device doesnt have its own eth_device structure yet, lets create one
  .firstdevice:
        mov     ecx, [BOOMERANG_DEVICES]
        add     ecx, [VORTEX_DEVICES]
        cmp     ecx, MAX_DEVICES                        ; First check if the driver can handle one more card
        jae     .fail

        allocate_and_clear ebx, device.size, .fail      ; Allocate the buffer for device structure

; Fill in the direct call addresses into the struct

        mov     [device.reset], reset
        mov     [device.transmit], null_op
        mov     [device.get_MAC], read_mac
        mov     [device.set_MAC], write_mac
        mov     [device.unload], null_op
        mov     [device.name], my_service

; save the pci bus and device numbers

        mov     eax, [IOCTL.input]
        movzx   ecx, byte[eax+1]
        mov     [device.pci_bus], ecx
        movzx   ecx, byte[eax+2]
        mov     [device.pci_dev], ecx

; Now, it's time to find the base io addres of the PCI device
        PCI_find_io

; We've found the io address, find IRQ now
        PCI_find_irq

        DEBUGF  1,"Hooking into device, dev:%x, bus:%x, irq:%x, addr:%x\n",\
        [device.pci_dev]:1,[device.pci_bus]:1,[device.irq_line]:1,[device.io_addr]:4

; Ok, the eth_device structure is ready, let's probe the device
        call    probe                                                   ; this function will output in eax
        test    eax, eax
        jnz     .err                                                    ; If an error occured, exit


        movzx   ecx, [device.ver_id]
        test    word [hw_versions+2+ecx*4], IS_VORTEX
        jz      .not_vortex

        mov     eax, [VORTEX_DEVICES]                                   ; Add the device structure to our device list
        mov     [VORTEX_LIST+4*eax], ebx                                ; (IRQ handler uses this list to find device)
        inc     [VORTEX_DEVICES]                                        ;

  .register:
        mov     [device.type], NET_TYPE_ETH
        call    NetRegDev

        cmp     eax, -1
        je      .destroy

        call    start_device
        ret

  .not_vortex:
        mov     eax, [BOOMERANG_DEVICES]                                          ; Add the device structure to our device list
        mov     [BOOMERANG_LIST+4*eax], ebx                                ; (IRQ handler uses this list to find device)
        inc     [BOOMERANG_DEVICES]

        jmp     .register

; If the device was already loaded, find the device number and return it in eax

  .find_devicenum:
        DEBUGF  1,"Trying to find device number of already registered device\n"
        call    NetPtrToNum                                             ; This kernel procedure converts a pointer to device struct in ebx
                                                                        ; into a device number in edi
        mov     eax, edi                                                ; Application wants it in eax instead
        DEBUGF  1,"Kernel says: %u\n", eax
        ret

; If an error occured, remove all allocated data and exit (returning -1 in eax)

  .destroy:
        ; todo: reset device into virgin state

  .err:
        stdcall KernelFree, ebx


  .fail:
        or      eax, -1
        ret

;------------------------------------------------------
endp


;;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\;;
;;                                                                        ;;
;;        Actual Hardware dependent code starts here                      ;;
;;                                                                        ;;
;;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\;;





;***************************************************************************
;   Function
;      probe
;   Description
;      Searches for an ethernet card, enables it and clears the rx buffer
;   Destroyed registers
;      eax, ebx, ecx, edx, edi, esi
;
;***************************************************************************

align 4
probe:

        DEBUGF  1,"Probing 3com card\n"

        PCI_make_bus_master

; wake up the card
        call    wake_up

        stdcall PciRead32, [device.pci_bus], [device.pci_dev], 0                                ; get device/vendor id

        DEBUGF  1,"Vendor id: 0x%x\n", ax

        cmp     ax, 0x10B7
        jne     .notfound
        shr     eax, 16

        DEBUGF  1,"Vendor ok!, device id: 0x%x\n", ax

; get chip version
        mov     ecx, HW_VERSIONS_SIZE/4-1
  .loop:
        cmp     ax, [hw_versions+ecx*4]
        jz      .found
        loop    .loop
  .notfound:
        DEBUGF  1,"Device id not found in list!\n"
        or      eax, -1
        ret
  .found:
        mov     esi, [hw_str+ecx*4]
        DEBUGF  1,"Hardware type: %s\n", esi
        mov     [device.name], esi

        mov     [device.ver_id], cl
        test    word [hw_versions+2+ecx*4], HAS_HWCKSM
        setnz   [device.has_hwcksm]
; set pci latency for vortex cards
        test    word [hw_versions+2+ecx*4], IS_VORTEX
        jz      .not_vortex

        mov     eax, 11111000b ; 248 = max latency
        stdcall PciWrite32, [device.pci_bus], [device.pci_dev], PCI_REG_LATENCY, eax

  .not_vortex:
; set RX/TX functions
        mov     ax, EEPROM_REG_CAPABILITIES
        call    read_eeprom
        test    al, 100000b ; full bus master?
        setnz   [device.full_bus_master]
        jnz     .boomerang_func
        mov     [device.transmit], vortex_transmit
        DEBUGF  1,"Device is a vortex type\n"
        DEBUGF  1,"I'm sorry but vortex code hasnt been tested yet\n"
        DEBUGF  1,"Please contact me on hidnplayr@kolibrios.org\n"
        DEBUGF  1,"If you can help me finish it!\n"
        or      eax, -1
        ret
        jmp     @f
  .boomerang_func: ; full bus master, so use boomerang functions
        mov     [device.transmit], boomerang_transmit
        DEBUGF  1,"Device is a boomerang type\n"
       @@:
        call    read_mac_eeprom

        test    byte [device.full_bus_master], 0xff
        jz      .set_preamble
; switch to register window 2
        set_io  0
        set_io  REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+2
        out     dx, ax
; activate xcvr by setting some magic bits
        set_io  REG_RESET_OPTIONS
        in      ax, dx
        and     ax, not 0x4010
        movzx   ecx, [device.ver_id]
        test    word [ecx*4+hw_versions+2], INVERT_LED_PWR
        jz      @f
        or      al, 0x10
       @@:
        test    word [ecx*4+hw_versions+2], INVERT_MII_PWR
        jz      @f
        or      ah, 0x40
       @@:
        out     dx, ax
  .set_preamble:
; use preamble as default
        mov     byte [device.preamble], 1 ; enable preamble

        call    global_reset

;--------------------------
; RESET

align 4
reset:

        movzx   eax, [device.irq_line]
        DEBUGF  1,"Attaching int handler to irq %x\n",eax:1

        movzx   ecx, [device.ver_id]
        test    word [hw_versions+2+ecx*4], IS_VORTEX
        jz      .not_vortex

        mov     esi, int_vortex
        jmp     .reg_int

.not_vortex:
        mov     esi, int_boomerang

.reg_int:
        stdcall AttachIntHandler, eax, esi, dword 0
        test    eax, eax
        jnz     @f
        DEBUGF  1,"\nCould not attach int handler!\n"
;        or      eax, -1
;        ret
  @@:

        set_io  0
        set_io  REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW + 0
        out     dx, ax

        mov     ax, StopCoax
        out     dx, ax                        ; stop transceiver

        mov     ax, SELECT_REGISTER_WINDOW + 4
        out     dx, ax                        ; disable UTP

        set_io  REG_MEDIA_STATUS
        mov     ax, 0x0

        set_io  REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW + 0
        out     dx, ax

        set_io  REG_FIFO_DIAGNOSTIC
        mov     ax, 0
        out     dx, ax                        ; disable card

        mov     ax, 1
        out     dx, ax                        ; enable card

        call    write_mac


;<<<<<<<<<<<<<<

        set_io  REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW + 1
        out     dx, ax

        mov     ecx, 32
        set_io  0x0b
  .loop:
        in      al, dx
        loop    .loop

; Get rid of stary ints
        set_io  REG_COMMAND
        mov     ax, AckIntr + 0xff
        out     dx, ax

        mov     ax, SetStatusEnb + S_5_INTS
        out     dx, ax

        mov     ax, SetIntrEnb + S_5_INTS
        out     dx, ax

        call    set_rx_mode
        call    set_active_port

;>>>>>>>>>>

        call    create_rx_ring
        call    rx_reset
        call    tx_reset

;>>>>>>>>>>>>>>>>>>

        set_io  0
        set_io  REG_COMMAND
        mov     ax, RxEnable
        out     dx, ax

        mov     ax, TxEnable
        out     dx, ax

        set_io  REG_COMMAND
        mov     ax, SetRxThreshold + 208
        out     dx, ax

        mov     ax, SetTxThreshold + 60 ;16 ; recommended by the manual :)
        out     dx, ax

        mov     ax, SELECT_REGISTER_WINDOW + 1
        out     dx, ax

        xor     eax, eax
; clear packet/byte counters

        lea     edi, [device.bytes_tx]
        mov     ecx, 6
        rep     stosd

; Set the mtu, kernel will be able to send now
        mov     [device.mtu], 1514

        ret





align 4
start_device:
        DEBUGF  1,"Starting the device\n"

        set_io  0
        set_io  REG_COMMAND
        mov     ax, SetTxThreshold + 60 ;2047 ; recommended by the manual :)
        out     dx, ax

        call    check_tx_status

        set_io  0
        set_io  REG_COMMAND
; switch to register window 4
        mov     ax, SELECT_REGISTER_WINDOW+4
        out     dx, ax

; wait for linkDetect
        set_io  REG_MEDIA_STATUS
        mov     ecx, 20 ; wait for max 2s
  .link_detect_loop:
        mov     esi, 100
        call    Sleep ; 100 ms
        in      ax, dx
        test    ah, 1000b ; linkDetect
        jnz     @f
        loop    .link_detect_loop
        DEBUGF  1,"Link detect timed-out!\n"
       @@:

; print link type
        xor     eax, eax
        bsr     ax, word [device.mode]
        jz      @f
        sub     ax, 4
       @@:

        mov     esi, [link_str+eax*4]
        DEBUGF  1,"Established Link type: %s\n", esi

; enable interrupts

        set_io  REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW + 1
        out     dx, ax

        mov     ax, AckIntr + 0xff
        out     dx, ax

        mov     ax, SetStatusEnb + S_5_INTS
        out     dx, ax

        mov     ax, SetIntrEnb + S_5_INTS
        out     dx, ax

        ret







align 4
set_rx_mode:

        DEBUGF  1,"Setting RX mode\n"

        set_io  0
        set_io  REG_COMMAND

if      defined PROMISCIOUS
        mov     ax, SetRxFilter + RxStation + RxMulticast + RxBroadcast + RxProm
else if  defined ALLMULTI
        mov     ax, SetRxFilter + RxStation + RxMulticast + RxBroadcast
else
        mov     ax, SetRxFilter + RxStation + RxBroadcast
end if
        out     dx, ax

        ret





;***************************************************************************
;   Function
;      global_reset
;   Description
;      resets the device
;   Parameters:
;      ebp - io_addr
;   Return value:
;   Destroyed registers
;      ax, ecx, edx, esi
;
;***************************************************************************1

align 4
global_reset:

        DEBUGF 1,"Global reset..\n"

; GlobalReset
        set_io  0
        set_io  REG_COMMAND
        xor     eax, eax
;       or      al, 0x14
        out     dx, ax
; wait for GlobalReset to complete
        mov     ecx, 64000
  .loop:
        in      ax , dx
        test    ah , 10000b ; check CmdInProgress
        loopz   .loop

        DEBUGF 1,"Waiting for nic to boot..\n"
; wait for 2 seconds for NIC to boot
        mov     esi, 2000
        call    Sleep ; 2 seconds

        DEBUGF 1,"Ok!\n"

        ret



;***************************************************************************
;   Function
;      tx_reset
;   Description
;      resets and enables transmitter engine
;
;***************************************************************************

align 4
tx_reset:
        DEBUGF 1,"tx reset\n"

; TxReset
        set_io  0
        set_io  REG_COMMAND
        mov     ax, TxReset
        out     dx, ax
; Wait for TxReset to complete
        mov     ecx, 200000
.tx_reset_loop:
        in      ax, dx
        test    ah, 10000b ; check CmdInProgress
        jz      .tx_set_prev
        dec     ecx
        jnz     .tx_reset_loop
.tx_set_prev:
; init last_dpd
        lea     eax, [device.dpd_buffer + (NUM_TX_DESC-1)*dpd.size]
        mov     [device.prev_dpd], eax

.tx_enable:
        ret



;***************************************************************************
;   Function
;      rx_reset
;   Description
;      resets and enables receiver engine
;
;***************************************************************************

align 4
rx_reset:

        DEBUGF 1,"rx reset\n"

        set_io  0
        set_io  REG_COMMAND
        mov     ax, RxReset or 0x4
        out     dx, ax

; wait for RxReset to complete
        mov     ecx, 200000
  .loop:
        in      ax, dx
        test    ah, 10000b ; check CmdInProgress
        jz      .done
        dec     ecx
        jnz     .loop
  .done:

        lea     eax, [device.upd_buffer]
        mov     [device.curr_upd], eax
        GetRealAddr
        set_io  0
        set_io  REG_UP_LIST_PTR
        out     dx, eax

  .rx_enable:
        ret


align 4
create_rx_ring:
; create upd ring
        lea     eax, [device.upd_buffer]
        GetRealAddr
        mov     edi, eax                                                ; real addr of first descr

        lea     esi, [device.upd_buffer]                                ; ptr to first descr
        lea     edx, [device.upd_buffer + (NUM_RX_DESC-1)*upd.size]     ; ptr to last descr

        mov     ecx, NUM_RX_DESC

  .upd_loop:
        mov     [edx + upd.next_ptr], edi

        push    ecx edx
        stdcall KernelAlloc, MAX_ETH_FRAME_SIZE
        pop     edx ecx
        mov     [esi + upd.realaddr], eax
        call    GetPgAddr
        mov     [esi + upd.frag_addr], eax
        and     [esi + upd.pkt_status], 0
        mov     [esi + upd.frag_len], MAX_ETH_FRAME_SIZE or (1 shl 31)

        DEBUGF  1,"UPD: lin=%x phys=%x len=%x next ptr=%x\n", [esi+upd.realaddr]:8, [esi+upd.frag_addr]:8, [esi+upd.frag_len]:8, edi
        DEBUGF  1,"UPD: cur=%x prev=%x\n", esi, edx

        mov     edx, esi
        add     esi, upd.size
        add     edi, upd.size
        dec     ecx
        jnz     .upd_loop

        ret



;---------------------------------------------------------------------------
;   Function
;      try_link_detect
;   Description
;      try_link_detect checks if link exists
;   Parameters
;      ebx = device structure
;   Return value
;      al - 0 ; no link detected
;      al - 1 ; link detected
;   Destroyed registers
;      eax, ebx, ecx, edx, edi, esi
;
;---------------------------------------------------------------------------

align 4
try_link_detect:

        DEBUGF  1,"trying to detect link\n"

; create self-directed packet
        stdcall KernelAlloc, 20 ; create a buffer for the self-directed packet
        test    eax, eax
        jz      .fail

        pushd   20              ; Packet parameters for device.transmit
        push    eax             ;

        mov     edi, eax

        lea     esi, [device.mac]
        movsw
        movsd
        sub     esi, 6
        movsw
        movsd
        mov     ax , 0x0608
        stosw

; download self-directed packet
        call    [device.transmit]

; switch to register window 4
        set_io  0
        set_io  REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+4
        out     dx, ax

; See if we have received the packet by now..
        cmp     [device.packets_rx], 0
        jnz     .link_detected

; switch to register window 4
        set_io  REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+4
        out     dx, ax

; read linkbeatdetect
        set_io  REG_MEDIA_STATUS
        in      ax, dx
        test    ah, 1000b ; test linkBeatDetect
        jnz     .link_detected
        xor     al, al
        jmp     .finish

  .link_detected:
        DEBUGF  1,"link detected!\n"
        setb    al

  .finish:
        test    al, al
        jz      @f
        or      byte [device.mode+1], 100b
       @@:
        ret

  .fail:
        ret



;***************************************************************************
;   Function
;      try_phy
;   Description
;      try_phy checks the auto-negotiation function
;      in the PHY at PHY index. It can also be extended to
;      include link detection for non-IEEE 802.3u
;      auto-negotiation devices, for instance the BCM5000.              ; TODO: BCM5000
;   Parameters
;       ah - PHY index
;       ebx - device stucture
;   Return value
;      al - 0 link is auto-negotiated
;      al - 1 no link is auto-negotiated
;   Destroyed registers
;       eax, ebx, ecx, edx, esi
;
;***************************************************************************

align 4
try_phy:

        DEBUGF 1,"PHY=%u\n", ah
        DEBUGF 1,"Detecting if device is auto-negotiation capable\n"

        mov     al, REG_MII_BMCR
        push    eax
        call    mdio_read       ; returns with window #4
        or      ah , 0x80       ; software reset
        mov     esi, eax
        mov     eax, dword [esp]
        call    mdio_write      ; returns with window #4

; wait for reset to complete
        mov     esi, 2000
        stdcall Sleep      ; 2s
        mov     eax, [esp]
        call    mdio_read       ; returns with window #4
        test    ah , 0x80
        jnz     .fail1
        mov     eax, [esp]

; wait for a while after reset
        mov     esi, 20
        stdcall Sleep      ; 20ms
        mov     eax, [esp]
        mov     al , REG_MII_BMSR
        call    mdio_read        ; returns with window #4
        test    al , 1           ; extended capability supported?
        jz      .fail2

; auto-neg capable?
        test    al , 1000b
        jz      .fail2           ; not auto-negotiation capable

        DEBUGF  1,"Device is auto-negotiation capable\n"

; auto-neg complete?
        test    al , 100000b
        jnz     .auto_neg_ok

        DEBUGF  1,"Restarting auto-negotiation\n"

; restart auto-negotiation
        mov     eax, [esp]
        mov     al , REG_MII_ANAR
        push    eax
        call    mdio_read       ; returns with window #4
        or      ax , 1111b shl 5; advertise only 10base-T and 100base-TX
        mov     esi, eax
        pop     eax
        call    mdio_write      ; returns with window #4
        mov     eax, [esp]
        call    mdio_read       ; returns with window #4
        mov     esi, eax
        or      bh , 10010b     ; restart auto-negotiation
        mov     eax, [esp]
        call    mdio_write      ; returns with window #4
        mov     esi, 4000
        stdcall Sleep  ; 4 seconds
        mov     eax, [esp]
        mov     al , REG_MII_BMSR
        call    mdio_read ; returns with window #4
        test    al , 100000b ; auto-neg complete?
        jnz     .auto_neg_ok
        jmp     .fail3
  .auto_neg_ok:

        DEBUGF  1,"Auto-negotiation complete\n"

; compare advertisement and link partner ability registers
        mov     eax, [esp]
        mov     al , REG_MII_ANAR
        call    mdio_read       ; returns with window #4
        xchg    eax, [esp]
        mov     al , REG_MII_ANLPAR
        call    mdio_read       ; returns with window #4
        pop     esi
        and     eax, esi
        and     eax, 1111100000b
        push    eax

        mov     word[device.mode+2], ax

; switch to register window 3
        set_io  0
        set_io  REG_COMMAND
        mov     ax , SELECT_REGISTER_WINDOW+3
        out     dx , ax

; set full-duplex mode
        set_io  REG_MAC_CONTROL
        in      ax , dx
        and     ax , not 0x120  ; clear full duplex and flow control
        pop     esi
        test    esi, 1010b shl 5; check for full-duplex
        jz      .half_duplex
        or      ax , 0x120      ; set full duplex and flow control
  .half_duplex:
        DEBUGF 1,"Using half-duplex\n"
        out     dx , ax
        mov     al , 1
        ret


  .fail1:
        DEBUGF  1,"reset failed!\n"
        pop     eax
        xor     al, al
        ret

  .fail2:
        DEBUGF  1,"This device is not auto-negotiation capable!\n"
        pop     eax
        xor     al, al
        ret

  .fail3:
        DEBUGF  1,"auto-negotiation reset failed!\n"
        pop     eax
        xor     al, al
        ret



;***************************************************************************
;   Function
;      try_mii
;   Description
;      try_MII checks the on-chip auto-negotiation logic
;      or an off-chip MII PHY, depending upon what is set in
;      xcvrSelect by the caller.
;      It exits when it finds the first device with a good link.
;   Parameters
;      ebp - io_addr
;   Return value
;      al - 0
;      al - 1
;   Destroyed registers
;      eax, ebx, ecx, edx, esi
;
;***************************************************************************

align 4
try_mii:

        DEBUGF  1,"trying to find MII PHY\n"

; switch to register window 3
        set_io  0
        set_io  REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+3
        out     dx, ax
        set_io  REG_INTERNAL_CONFIG
        in      eax, dx
        and     eax, (1111b shl 20)
        cmp     eax, (1000b shl 20) ; is auto-negotiation set?
        jne     .mii_device

        DEBUGF  1,"auto-negotiation is set\n"
; switch to register window 4
        set_io  REG_COMMAND
        mov     ax , SELECT_REGISTER_WINDOW+4
        out     dx , ax

; PHY==24 is the on-chip auto-negotiation logic
; it supports only 10base-T and 100base-TX
        mov     ah , 24
        call    try_phy
        test    al , al
        jz      .fail_finish

        mov     cl , 24
        jmp     .check_preamble

  .mii_device:
        cmp     eax, (0110b shl 20)
        jne     .fail_finish

        set_io  0
        set_io  REG_COMMAND
        mov     ax , SELECT_REGISTER_WINDOW+4
        out     dx , ax

        set_io  REG_PHYSICAL_MGMT
        in      ax , dx
        and     al , (1 shl BIT_MGMT_DIR) or (1 shl BIT_MGMT_DATA)
        cmp     al , (1 shl BIT_MGMT_DATA)
        je      .search_for_phy

        xor     al , al
        ret

  .search_for_phy:
; search for PHY
        mov     cx , 31
  .search_phy_loop:
        DEBUGF  1,"Searching the PHY\n"
        cmp     cx , 24
        je      .next_phy
        mov     ah , cl ; ah = phy
        mov     al , REG_MII_BMCR ; al = Basic Mode Status Register
        push    cx
        call    mdio_read
        pop     cx
        test    ax , ax
        jz      .next_phy
        cmp     ax , 0xffff
        je      .next_phy
        mov     ah , cl ; ah = phy
        push    cx
        call    try_phy
        pop     cx
        test    al , al
        jnz     .check_preamble
  .next_phy:
        loopw   .search_phy_loop

  .fail_finish:
        xor     al, al
        ret

; epilog
  .check_preamble:
        DEBUGF  1,"Using PHY: %u\nChecking PreAmble\n", cl
        push    eax ; eax contains the return value of try_phy
; check hard coded preamble forcing
        movzx   eax, [device.ver_id]
        test    word [eax*4+hw_versions+2], EXTRA_PREAMBLE
        setnz   [device.preamble] ; force preamble
        jnz     .finish

; check mii for preamble suppression
        mov     ah, cl
        mov     al, REG_MII_BMSR
        call    mdio_read
        test    al, 1000000b ; preamble suppression?
        setz    [device.preamble] ; no

  .finish:
        pop     eax
        ret



;***************************************************************************
;   Function
;      test_packet
;   Description
;      try_loopback try a loopback packet for 10BASE2 or AUI port
;   Parameters
;      ebx = device structure
;
;***************************************************************************

align 4
test_packet:

        DEBUGF 1,"sending test packet\n"

; switch to register window 3
        set_io  0
        set_io  REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+3
        out     dx, ax

; set fullDuplexEnable in MacControl register
        set_io  REG_MAC_CONTROL
        in      ax, dx
        or      ax, 0x120
        out     dx, ax

; switch to register window 5
        set_io  REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+5
        out     dx, ax

; set RxFilter to enable individual address matches
        mov     ax, (10000b shl 11)
        set_io  REG_RX_FILTER
        in      al, dx
        or      al, 1
        set_io  REG_COMMAND
        out     dx, ax

; issue RxEnable and TxEnable
        call    rx_reset
        call    tx_reset

; create self-directed packet
        stdcall KernelAlloc, 20 ; create a buffer for the self-directed packet
        test    eax, eax
        jz      .fail

        pushd   20              ; Packet parameters for device.transmit
        push    eax             ;

        mov     edi, eax
        lea     esi, [device.mac]
        movsw
        movsd
        sub     esi, 6
        movsw
        movsd
        mov     ax , 0x0608
        stosw

; download self-directed packet
        call    [device.transmit]

; wait for 2s
        mov     esi, 2000
        call    Sleep

; check if self-directed packet is received
        mov     eax, [device.packets_rx]
        test    eax, eax
        jnz     .finish

; switch to register window 3
        set_io  0
        set_io  REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+3
        out     dx, ax

; clear fullDuplexEnable in MacControl register
        set_io  REG_MAC_CONTROL
        in      ax , dx
        and     ax , not 0x120
        out     dx , ax
  .fail:
        xor     eax, eax

  .finish:
        ret



;***************************************************************************
;   Function
;      try_loopback
;   Description
;      tries a loopback packet for 10BASE2 or AUI port
;   Parameters
;      al -  0: 10Mbps AUI connector
;            1: 10BASE-2
;      ebp - io_addr
;   Return value
;      al - 0
;      al - 1
;   Destroyed registers
;      eax, ebx, ecx, edx, edi, esi
;
;***************************************************************************

align 4
try_loopback:

        DEBUGF 1,"trying loopback\n"

        push    eax
; switch to register window 3
        set_io  0
        set_io  REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+3
        out     dx, ax
        mov     eax, [esp]

        mov     cl, al
        inc     cl
        shl     cl, 3
        or      byte [device.mode+1], cl

        test    al, al ; aui or coax?
        jz      .complete_loopback
; enable 100BASE-2 DC-DC converter
        mov     ax, (10b shl 11) ; EnableDcConverter
        out     dx, ax
  .complete_loopback:

        mov     cx, 2 ; give a port 3 chances to complete a loopback
  .next_try:
        push    ecx
        call    test_packet
        pop     ecx
        test    eax, eax
        loopzw  .next_try

  .finish:
        xchg    eax, [esp]
        test    al, al
        jz      .aui_finish

; issue DisableDcConverter command
        set_io  0
        set_io  REG_COMMAND
        mov     ax, (10111b shl 11)
        out     dx, ax
  .aui_finish:
        pop     eax ; al contains the result of operation

        test    al, al
        jnz     @f
        and     byte [device.mode+1], not 11000b
       @@:

        ret


;***************************************************************************
;   Function
;      set_active_port
;   Description
;      It selects the media port (transceiver) to be used
;   Return value:
;   Destroyed registers
;      eax, ebx, ecx, edx, edi, esi
;
;***************************************************************************

align 4
set_active_port:

        DEBUGF 1,"Trying to find the active port\n"

; switch to register window 3
        set_io  0
        set_io  REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW + 3
        out     dx, ax

        set_io  REG_INTERNAL_CONFIG
        in      eax, dx
        test    eax, (1 shl 24) ; check if autoselect enable
        jz      .set_first_available_media

; check 100BASE-TX and 10BASE-T
        set_io  REG_MEDIA_OPTIONS
        in      ax, dx
        test    al, 1010b       ; check whether 100BASE-TX or 10BASE-T available
        jz      .mii_device     ; they are not available

; set auto-negotiation
        set_io  REG_INTERNAL_CONFIG
        in      eax, dx
        and     eax, not (1111b shl 20)
        or      eax, (1000b shl 20)
        out     dx, eax
        call    try_mii
        test    al, al
        jz      .mii_device
        DEBUGF 1,"Using auto negotiation\n"
        ret

  .mii_device:
; switch to register window 3
        set_io  0
; check for off-chip mii device
        set_io  REG_MEDIA_OPTIONS
        in      ax, dx
        test    al, 1000000b ; check miiDevice
        jz      .base_fx
        set_io  REG_INTERNAL_CONFIG
        in      eax, dx
        and     eax, not (1111b shl 20)
        or      eax, (0110b shl 20) ; set MIIDevice
        out     dx, eax
        call    try_mii
        test    al, al
        jz      .base_fx
        DEBUGF 1,"Using off-chip mii device\n"
        ret

  .base_fx:
; switch to register window 3
        set_io  0
; check for 100BASE-FX
        set_io  REG_MEDIA_OPTIONS
        in      ax, dx ; read media option register
        test    al, 100b ; check 100BASE-FX
        jz      .aui_enable
        set_io  REG_INTERNAL_CONFIG
        in      eax, dx
        and     eax, not (1111b shl 20)
        or      eax, (0101b shl 20) ; set 100base-FX
        out     dx, eax
        call    try_link_detect
        test    al, al
        jz      .aui_enable
        DEBUGF 1,"Using 100Base-FX\n"
        ret

  .aui_enable:
; switch to register window 3
        set_io  0
; check for 10Mbps AUI connector
        set_io  REG_MEDIA_OPTIONS
        in      ax, dx ; read media option register
        test    al, 100000b ; check 10Mbps AUI connector
        jz      .coax_available
        set_io  REG_INTERNAL_CONFIG
        in      eax, dx
        and     eax, not (1111b shl 20)
        or      eax, (0001b shl 20) ; set 10Mbps AUI connector
        out     dx, eax
        xor     al, al ; try 10Mbps AUI connector
        call    try_loopback
        test    al, al
        jz      .coax_available
        DEBUGF 1,"Using 10Mbps aui\n"
        ret

  .coax_available:
; switch to register window 3
        set_io  0
; check for coaxial 10BASE-2 port
        set_io  REG_MEDIA_OPTIONS
        in      ax, dx ; read media option register
        test    al, 10000b ; check 10BASE-2
        jz      .set_first_available_media

        set_io  REG_INTERNAL_CONFIG
        in      eax, dx
        and     eax, not (1111b shl 20)
        or      eax, (0011b shl 20) ; set 10BASE-2
        out     dx, eax
        mov     al, 1
        call    try_loopback
        test    al, al
        jz      .set_first_available_media
        DEBUGF 1,"Using 10BASE-2 port\n"
        ret

  .set_first_available_media:
        DEBUGF  1,"Using the first available media\n"

;***************************************************************************
;   Function
;      set_available_media
;   Description
;      sets the first available media
;   Parameters
;      ebx - ptr to device struct
;   Return value
;      al - 0
;      al - 1
;   Destroyed registers
;      eax, edx
;
;***************************************************************************

align 4
set_available_media:

        DEBUGF  1,"Setting the available media\n"
; switch to register window 3
        set_io  0
        set_io  REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+3
        out     dx, ax

        set_io  REG_MEDIA_OPTIONS
        in      ax, dx
        DEBUGF  1,"available media:%x\n", al
        mov     cl, al

        set_io  REG_INTERNAL_CONFIG
        in      eax, dx
        and     eax, not (1111b shl 20) ; these bits hold the 'transceiver select' value

        test    cl, 10b         ; baseTXAvailable
        jz      @f

        DEBUGF  1,"base TX is available\n"
        or      eax, (100b shl 20)
if defined FORCE_FD
        mov     word [device.mode], (1 shl 8)
else
        mov     word [device.mode], (1 shl 7)
end if
        jmp     .set_media
       @@:

        test    cl, 100b        ; baseFXAvailable
        jz      @f

        DEBUGF  1,"base FX is available\n"
        or      eax, (101b shl 20)
        mov     word [device.mode], (1 shl 10)
        jmp     .set_media
       @@:

        test    cl, 1000000b    ; miiDevice
        jz      @f

        DEBUGF  1,"mii-device is available\n"
        or      eax, (0110b shl 20)
        mov     word [device.mode], (1 shl 13)
        jmp     .set_media
       @@:

        test    cl, 1000b       ; 10bTAvailable
        jz      @f

        DEBUGF  1,"10base-T is available\n"
  .set_default:
if FORCE_FD
        mov     word [device.mode], (1 shl 6)
else
        mov     word [device.mode], (1 shl 5)
end if
        jmp     .set_media
       @@:

        test    cl, 10000b      ; coaxAvailable
        jz      @f

        DEBUGF  1,"coax is available\n"
        push    eax
        set_io  REG_COMMAND
        mov     ax, (10b shl 11) ; EnableDcConverter
        out     dx, ax
        pop     eax

        or      eax, (11b shl 20)
        mov     word [device.mode], (1 shl 12)
        jmp     .set_media
       @@:

        test    cl, 10000b      ; auiAvailable
        jz      .set_default

        DEBUGF  1,"AUI is available\n"
        or      eax, (1 shl 20)
        mov     word [device.mode], (1 shl 11)

  .set_media:
        set_io  0
        set_io  REG_INTERNAL_CONFIG
        out     dx, eax

if FORCE_FD
        DEBUGF  1,"Forcing full duplex\n"
        set_io  REG_MAC_CONTROL
        in      ax, dx
        or      ax, 0x120
        out     dx, ax
end if

        mov     al, 1
        ret



;***************************************************************************
;   Function
;      wake_up
;   Description
;      set the power state to D0
;
;***************************************************************************

align 4
wake_up:

        DEBUGF 1,"Waking up NIC: "

; wake up - we directly do it by programming PCI
; check if the device is power management capable
        stdcall PciRead32, [device.pci_bus], [device.pci_dev], PCI_REG_STATUS

        test    al, 10000b      ; is there "new capabilities" linked list?
        jz      .device_awake

; search for power management register
        stdcall PciRead16, [device.pci_bus], [device.pci_dev], PCI_REG_CAP_PTR
        cmp     al, 0x3f
        jbe     .device_awake

; traverse the list
        movzx   esi, al
  .pm_loop:
        stdcall PciRead32, [device.pci_bus], [device.pci_dev], esi

        cmp     al , 1
        je      .set_pm_state

        movzx   esi, ah

        test    ah , ah
        jnz     .pm_loop
        jmp     .device_awake

; waku up the device if necessary
  .set_pm_state:

        add     esi, PCI_REG_PM_CTRL
        stdcall PciRead32, [device.pci_bus], [device.pci_dev], esi
        test    al, 3
        jz      .device_awake
        and     al, not 11b ; set state to D0
        stdcall PciWrite32, [device.pci_bus], [device.pci_dev], esi, eax

  .device_awake:
        DEBUGF 1,"Device is awake\n"

        ret




;***************************************************************************
;   Function
;      write_eeprom
;   Description
;      reads eeprom
;      Note : the caller must switch to the register window 0
;             before calling this function
;   Parameters:
;      ax - register to be read (only the first 63 words can be read)
;      cx - value to be read into the register
;   Return value:
;      ax - word read
;   Destroyed registers
;      ax, ebx, edx
;
;***************************************************************************
;       align 4
;write_eeprom:
;       mov     edx, [io_addr]
;       add     edx, REG_EEPROM_COMMAND
;       cmp     ah, 11b
;       ja      .finish ; address may have a value of maximal 1023
;       shl     ax, 2
;       shr     al, 2
;       push    eax
;; wait for busy
;       mov     ebx, 0xffff
;@@:
;       in      ax, dx
;       test    ah, 0x80
;       jz      .write_enable
;       dec     ebx
;       jns     @r
;; write enable
;.write_enable:
;       xor     eax, eax
;       mov     eax, (11b shl 4)
;       out     dx, ax
;; wait for busy
;       mov     ebx, 0xffff
;@@:
;       in      ax, dx
;       test    ah, 0x80
;       jz      .erase_loop
;       dec     ebx
;       jns     @r
;.erase_loop:
;       pop     eax
;       push    eax
;       or      ax, (11b shl 6) ; erase register
;       out     dx, ax
;       mov     ebx, 0xffff
;@@:
;       in      ax, dx
;       test    ah, 0x80
;       jz      .write_reg
;       dec     ebx
;       jns     @r
;.write_reg:
;       add     edx, REG_EEPROM_DATA-REG_EEPROM_COMMAND
;       mov     eax, ecx
;       out     dx, ax
;; write enable
;       add     edx, REG_EEPROM_COMMAND-REG_EEPROM_DATA
;       xor     eax, eax
;       mov     eax, (11b shl 4)
;       out     dx, ax
; wait for busy
;       mov     ebx, 0xffff
;@@:
;       in      ax, dx
;       test    ah, 0x80
;       jz      .issue_write_reg
;       dec     ebx
;       jns     @r
;.issue_write_reg:
;       pop     eax
;       or      ax, 01b shl 6
;       out     dx, ax
;.finish:
;       ret


;***************************************************************************
;   Function
;      read_eeprom
;   Description
;      reads eeprom
;   Parameters:
;       ax - register to be read (only the first 63 words can be read)
;      ebx = driver structure
;   Return value:
;      ax - word read
;   Destroyed registers
;      ax, ebx, edx
;
;***************************************************************************

align 4
read_eeprom:

        DEBUGF 1,"Reading from eeprom.. "

        push    eax
; switch to register window 0
        set_io  0
        set_io  REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+0
        out     dx, ax
        pop     eax
        and     ax, 111111b ; take only the first 6 bits into account
        movzx   esi, [device.ver_id]

        test    word [esi*4+hw_versions+2], EEPROM_8BIT
        jz      @f
        add     ax, 0x230 ; hardware constant
        jmp     .read
@@:

        add     ax, EEPROM_CMD_READ
        test    word [esi*4+hw_versions+2], EEPROM_OFFSET
        jz      .read
        add     ax, 0x30
.read:

        set_io  REG_EEPROM_COMMAND
        out     dx, ax
        mov     ecx, 0xffff ; duration of about 162 us ;-)
.wait_for_reading:
        in      ax, dx
        test    ah, 0x80 ; check bit eepromBusy
        jz      .read_data
        loop    .wait_for_reading
.read_data:
        set_io  REG_EEPROM_DATA
        in      ax, dx

        DEBUGF 1,"ok!\n"

        ret

;***************************************************************************
;   Function
;      mdio_sync
;   Description
;      initial synchronization
;   Parameters
;      ebp - io_addr
;   Return value
;   Destroyed registers
;      ax, edx, cl
;
;***************************************************************************

align 4
mdio_sync:

        DEBUGF 1,"syncing mdio\n"

; switch to register window 4
        set_io  0
        set_io  REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+4
        out     dx, ax
        cmp     [device.preamble], 0
        je      .no_preamble
; send 32 logic ones
        set_io  REG_PHYSICAL_MGMT
        mov     ecx, 31
  .loop:
        mov     ax, (1 shl BIT_MGMT_DATA) or (1 shl BIT_MGMT_DIR)
        out     dx, ax
        in      ax, dx ; delay
        mov     ax, (1 shl BIT_MGMT_DATA) or (1 shl BIT_MGMT_DIR) or (1 shl BIT_MGMT_CLK)
        out     dx, ax
        in      ax, dx ; delay
        loop    .loop
  .no_preamble:

        ret

;***************************************************************************
;   Function
;      mdio_read
;   Description
;      read MII register
;      see page 16 in D83840A.pdf
;   Parameters
;       ah - PHY addr
;       al - register addr
;      ebx = device structure
;   Return value
;      ax - register read
;
;***************************************************************************

align 4
mdio_read:

        DEBUGF 1,"Reading MII registers\n"

        push    eax
        call    mdio_sync ; returns with window #4
        pop     eax
        set_io  0
        set_io  REG_PHYSICAL_MGMT
        shl     al, 3
        shr     ax, 3
        and     ax, not MII_CMD_MASK
        or      ax, MII_CMD_READ

        mov     esi, eax
        mov     ecx, 13
  .cmd_loop:
        mov     ax, (1 shl BIT_MGMT_DIR) ; write mii
        bt      esi, ecx
        jnc     .zero_bit
        or      al, (1 shl BIT_MGMT_DATA)

  .zero_bit:
        out     dx, ax
        push    ax
        in      ax, dx ; delay
        pop     ax
        or      al, (1 shl BIT_MGMT_CLK) ; write
        out     dx, ax
        in      ax, dx ; delay
        loop    .cmd_loop

; read data (18 bits with the two transition bits)
        mov     ecx, 17
        xor     esi, esi
  .read_loop:
        shl     esi, 1
        xor     eax, eax ; read comand
        out     dx, ax
        in      ax, dx ; delay
        in      ax, dx
        test    al, (1 shl BIT_MGMT_DATA)
        jz      .dont_set
        inc     esi
  .dont_set:
        mov     ax, (1 shl BIT_MGMT_CLK)
        out     dx, ax
        in      ax, dx ; delay
        loop    .read_loop
        mov     eax, esi

        ret



;***************************************************************************
;   Function
;      mdio_write
;   Description
;      write MII register
;      see page 16 in D83840A.pdf
;   Parameters
;       ah - PHY addr
;       al - register addr
;       si - word to be written
;   Return value
;      ax - register read
;
;***************************************************************************

align 4
mdio_write:

        DEBUGF 1,"Writing MII registers\n"

        push    eax
        call    mdio_sync
        pop     eax
        set_io  0
        set_io  REG_PHYSICAL_MGMT
        shl     al, 3
        shr     ax, 3
        and     ax, not MII_CMD_MASK
        or      ax, MII_CMD_WRITE
        shl     eax, 2
        or      eax, 10b ; transition bits
        shl     eax, 16
        mov     ax, si
        mov     esi, eax
        mov     ecx, 31

  .cmd_loop:
        mov     ax, (1 shl BIT_MGMT_DIR) ; write mii
        bt      esi, ecx
        jnc     @f
        or      al, (1 shl BIT_MGMT_DATA)
       @@:
        out     dx, ax
        push    eax
        in      ax, dx ; delay
        pop     eax
        or      al, (1 shl BIT_MGMT_CLK) ; write
        out     dx, ax
        in      ax, dx ; delay
        loop    .cmd_loop

        ret


;***************************************************************************
;   Function
;      check_tx_status
;   Description
;      Checks TxStatus queue.
;   Return value
;      al - 0 no error was found
;      al - 1 error was found TxReset was needed
;   Destroyed registers
;      eax, ecx, edx, ebp
;
;***************************************************************************

align 4
check_tx_status:

        DEBUGF 1,"Checking TX status\n"

; clear TxStatus queue
        set_io  0
        set_io  REG_TX_STATUS
        mov     ecx, 31 ; max number of queue entries

  .tx_status_loop:
        in      al, dx
        test    al, al
        jz      .finish ; no error
        test    al, 0x3f
        jnz     .error
  .no_error_found:
; clear current TxStatus entry which advances the next one
        xor     al, al
        out     dx, al
        loop    .tx_status_loop

  .finish:

        ret

  .error:
        call    tx_reset
        ret



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                         ;;
;; Transmit (vortex)                       ;;
;;                                         ;;
;; In: buffer pointer in [esp+4]           ;;
;;     size of buffer in [esp+8]           ;;
;;     pointer to device structure in ebx  ;;
;;                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
vortex_transmit:

        DEBUGF 1,"Sending packet (vortex)\n"

        cmp     dword [esp+8], MAX_ETH_FRAME_SIZE
        ja      .finish ; packet is too long

        call    check_tx_status

; switch to register window 7
        set_io  0
        set_io  REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+7
        out     dx, ax
; check for master operation in progress
        set_io  REG_MASTER_STATUS
        in      ax, dx
        test    ah, 0x80
        jnz     .finish ; no DMA for sending
; program frame address to be sent
        set_io  REG_MASTER_ADDRESS
        mov     eax, [esp+4]
        call    GetPgAddr
        out     dx, eax
; program frame length
        set_io  REG_MASTER_LEN
        mov     eax, [esp+8]
;;;        and     eax, not 3
        out     dx, ax
; start DMA Down
        set_io  REG_COMMAND
        mov     ax, (10100b shl 11) + 1 ; StartDMADown
        out     dx, ax
.finish:
        call    KernelFree
        add     esp, 4
        ret



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                         ;;
;; Transmit (boomerang)                    ;;
;;                                         ;;
;; In: buffer pointer in [esp+4]           ;;
;;     size of buffer in [esp+8]           ;;
;;     pointer to device structure in ebx  ;;
;;                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
boomerang_transmit:

        DEBUGF  1,"Transmitting packet, buffer:%x, size:%u\n",[esp+4],[esp+8]
        mov     eax, [esp+4]
        DEBUGF  1,"To: %x-%x-%x-%x-%x-%x From: %x-%x-%x-%x-%x-%x Type:%x%x\n",\
        [eax+00]:2,[eax+01]:2,[eax+02]:2,[eax+03]:2,[eax+04]:2,[eax+05]:2,\
        [eax+06]:2,[eax+07]:2,[eax+08]:2,[eax+09]:2,[eax+10]:2,[eax+11]:2,\
        [eax+13]:2,[eax+12]:2

        cmp     dword [esp+8], MAX_ETH_FRAME_SIZE
        ja      .fail

        call    check_tx_status

; calculate descriptor address
        mov     esi, [device.prev_dpd]
        DEBUGF  1,"Previous DPD: %x\n", esi
        add     esi, dpd.size
        lea     ecx, [device.dpd_buffer + (NUM_TX_DESC)*dpd.size]
        cmp     esi, ecx
        jb      @f
        lea     esi, [device.dpd_buffer]        ; Wrap if needed
       @@:
        DEBUGF  1,"Found a free DPD: %x\n", esi

; check DnListPtr
        set_io  0
        set_io  REG_DN_LIST_PTR
        in      eax, dx
; mark if Dn_List_Ptr is cleared
        test    eax, eax
        setz    [device.dn_list_ptr_cleared]

; finish if no more free descriptor is available - FIXME!
;        cmp     eax, esi
;        jz      .finish

; update statistics
        inc     [device.packets_tx]
        mov     ecx, [esp+8]            ; buffer size
        add     dword [device.bytes_tx], ecx
        adc     dword [device.bytes_tx + 4], 0

; program DPD
        and     [esi+dpd.next_ptr], 0
        mov     eax, [esp+4]            ; Tx buffer address
        mov     [esi+dpd.realaddr], eax
        call    GetPgAddr
        mov     [esi+dpd.frag_addr], eax
        mov     ecx, [esp+8]            ; packet size
        or      ecx, 0x80000000         ; last fragment
        mov     [esi+dpd.frag_len], ecx

        mov     ecx, [esp+8]            ; packet size
;        or      ecx, 0x8000             ; transmission complete notification

        or      ecx, 1 shl 31

;        test    byte [device.has_hwcksm], 0xff
;        jz      @f
;        or      ecx, (1 shl 26)         ; set AddTcpChecksum
;@@:
        mov     [esi+dpd.frame_start_hdr], ecx

        DEBUGF  1,"DPD: lin=%x phys=%x len=%x start hdr=%x\n", [esi+dpd.realaddr]:8, [esi+dpd.frag_addr]:8, [esi+dpd.frag_len]:8, [esi+dpd.frame_start_hdr]:8

; calculate physical address of dpd
        mov     eax, esi
        GetRealAddr
        cmp     [device.dn_list_ptr_cleared], 0
        jz      .add_to_list

; write Dn_List_Ptr
        DEBUGF  1,"DPD phys addr=%x\n", eax
        set_io  0
        set_io  REG_DN_LIST_PTR
        out     dx, eax
        jmp     .finish

  .add_to_list:
        DEBUGF  1,"Adding To list\n"
        push    eax
; DnStall
        set_io  0
        set_io  REG_COMMAND
        mov     ax, ((110b shl 11)+2)
        out     dx, ax

; wait for DnStall to complete
        DEBUGF  1,"Waiting for DnStall\n"
        mov     ecx, 6000
  .wait_for_stall:
        in      ax, dx                  ; read REG_INT_STATUS
        test    ah, 10000b
        jz      .dnstall_ok
        dec     ecx
        jnz     .wait_for_stall

  .dnstall_ok:
        DEBUGF  1,"DnStall ok!\n"
        mov     ecx, [device.prev_dpd]
        mov     [ecx+dpd.next_ptr], eax

        set_io  0
        set_io  REG_DN_LIST_PTR
        in      eax, dx
        test    eax, eax
        pop     eax
        jnz     .dnunstall

; if Dn_List_Ptr has been cleared fill it up
        DEBUGF  1,"DnList Ptr has been cleared\n"
        out     dx, eax

  .dnunstall:
; DnUnStall
        set_io  0
        set_io  REG_COMMAND
        mov     ax, ((110b shl 11)+3)
        out     dx, ax

  .finish:
        mov     [device.prev_dpd], esi
        xor     eax, eax
        ret     8

  .fail:
        stdcall KernelFree, [esp+4]
        or      eax, -1
        ret     8


;---------------------------------
; Write MAC

align 4
write_mac:

        DEBUGF 1,"Writing mac\n"

        set_io  0
        set_io  REG_COMMAND

; switch to register window 2
        mov     ax, SELECT_REGISTER_WINDOW+2
        out     dx, ax

; write MAC addres back into the station address registers
        set_io  REG_STATION_ADDRESS_LO
        lea     esi, [device.mac]
        outsw
        inc     dx
        inc     dx
        outsw
        inc     dx
        inc     dx
        outsw


;----------------------------
; Read MAC

align 4
read_mac:

        set_io  0
        set_io  REG_COMMAND

; switch to register window 2
        mov     ax, SELECT_REGISTER_WINDOW+2
        out     dx, ax

; write MAC addres back into the station address registers
        set_io  REG_STATION_ADDRESS_LO
        lea     edi, [device.mac]
        insw
        inc     dx
        inc     dx
        insw
        inc     dx
        inc     dx
        insw

        DEBUGF 1,"%x-%x-%x-%x-%x-%x\n",[device.mac]:2,[device.mac+1]:2,[device.mac+2]:2,[device.mac+3]:2,[device.mac+4]:2,[device.mac+5]:2

        ret


;------------------------------------
; Read MAC from eeprom

align 4
read_mac_eeprom:        ; Tested - ok

        DEBUGF 1,"Reading mac from eeprom\n"

; read MAC from eeprom
        mov     ecx, 3
  .mac_loop:
        lea     ax, [EEPROM_REG_OEM_NODE_ADDR+ecx-1]
        push    ecx
        call    read_eeprom
        pop     ecx
        xchg    ah, al ; htons
        mov     word [device.mac+ecx*2-2], ax
        loop    .mac_loop

        DEBUGF 1,"%x-%x-%x-%x-%x-%x\n",[device.mac]:2,[device.mac+1]:2,[device.mac+2]:2,[device.mac+3]:2,[device.mac+4]:2,[device.mac+5]:2

        ret





;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                          ;;
;; Vortex Interrupt handler ;;
;;                          ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
int_vortex:

        DEBUGF  1,"vortex IRQ %x ",eax:2

; find pointer of device wich made IRQ occur

        mov     esi, VORTEX_LIST
        mov     ecx, [VORTEX_DEVICES]
        test    ecx, ecx
        jz      .fail
  .nextdevice:
        mov     ebx, dword [esi]


        set_io  0
        set_io  REG_INT_STATUS
        in      ax, dx
;;        and     ax, INT_MASK
        jnz     .got_it


        add     esi, 4

        test    ax , ax
        jnz     .got_it
        loop    .nextdevice

  .fail:

        ret

.got_it:

        DEBUGF  1,"Device: %x Status: %x ",ebx,eax:4

        test    ax, RxComplete
        jz      .noRX

        set_io  0
  .rx_status_loop:
; examine RxStatus
        set_io  REG_RX_STATUS
        in      ax, dx
        test    ax, ax
        jz      .finish

        test    ah, 0x80 ; rxIncomplete
        jnz     .finish

        test    ah, 0x40
        jz      .check_length

; discard the top frame received advancing the next one
        set_io  REG_COMMAND
        mov     ax, (01000b shl 11)
        out     dx, ax
        jmp     .rx_status_loop

  .check_length:
        and     eax, 0x1fff
        cmp     eax, MAX_ETH_PKT_SIZE
        ja      .discard_frame ; frame is too long discard it

  .check_dma:
        mov     ecx, eax
; switch to register window 7
        set_io  0
        set_io  REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+7
        out     dx, ax
; check for master operation in progress
        set_io  REG_MASTER_STATUS
        in      ax, dx

        test    ah, 0x80
        jnz     .finish

  .read_frame:
; program buffer address to read in
        push    ecx
        stdcall KernelAlloc, MAX_ETH_FRAME_SIZE
        pop     ecx
        test    eax, eax
        jz      .finish

        push    .discard_frame
        push    ecx
        push    eax
;        zero_to_dma eax
        set_io  REG_MASTER_ADDRESS
        out     dx, eax

; program frame length
        set_io  REG_MASTER_LEN
        mov     ax, 1560
        out     dx, ax

; start DMA Up
        set_io  REG_COMMAND
        mov     ax, (10100b shl 11) ; StartDMAUp
        out     dx, ax

; check for master operation in progress
        set_io  REG_MASTER_STATUS   ; TODO: use timeout and reset after timeout expired
  .dma_loop:
        in      ax, dx
        test    ah, 0x80
        jnz     .dma_loop

; registrate the received packet to kernel
        jmp     Eth_input

; discard the top frame received
  .discard_frame:
        set_io  0
        set_io  REG_COMMAND
        mov     ax, (01000b shl 11)
        out     dx, ax

  .finish:


.noRX:

        test    ax, DMADone
        jz      .noDMA

        push    ax

        set_io  0
        set_io  12
        in      ax, dx
        test    ax, 0x1000
        jz      .nodmaclear

        mov     ax, 0x1000
        out     dx, ax

  .nodmaclear:

        pop     ax

        DEBUGF  1, "DMA Done!\n", cx



.noDMA:



.ACK:
        set_io  0
        set_io  REG_COMMAND
        mov     ax, AckIntr + IntReq + IntLatch
        out     dx, ax

        ret




;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                             ;;
;; Boomerang Interrupt handler ;;
;;                             ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
int_boomerang:

        DEBUGF  1,"\nBoomerang int\n"

; find pointer of device wich made IRQ occur

        mov     ecx, [BOOMERANG_DEVICES]
        test    ecx, ecx
        jz      .nothing
        mov     esi, BOOMERANG_LIST
  .nextdevice:
        mov     ebx, [esi]

        set_io  0
        set_io  REG_INT_STATUS
        in      ax, dx
        test    ax, ax
        jnz     .got_it
  .continue:
        add     esi, 4
        dec     ecx
        jnz     .nextdevice
  .nothing:
        ret

  .got_it:

        DEBUGF  1,"Device: %x Status: %x ", ebx, ax
        push    ax

; disable all INTS

        set_io  REG_COMMAND
        mov     ax, SetIntrEnb
        out     dx, ax

;--------------------------------------------------------------------------
        test    word[esp], UpComplete
        jz      .noRX

        push    ebx

  .receive:
        DEBUGF  1,"UpComplete\n"

; check if packet is uploaded
        mov     esi, [device.curr_upd]
        test    byte [esi+upd.pkt_status+1], 0x80 ; upPktComplete
        jz      .finish
        DEBUGF  1, "Current upd: %x\n", esi
; packet is uploaded check for any error
  .check_error:
        test    byte [esi+upd.pkt_status+1], 0x40 ; upError
        jz      .copy_packet_length
        DEBUGF  1,"Error in packet\n"
        and     [esi+upd.pkt_status], 0           ; mark packet as read
        jmp     .finish
  .copy_packet_length:
        mov     ecx, [esi+upd.pkt_status]
        and     ecx, 0x1fff

;        cmp     ecx, MAX_ETH_PKT_SIZE
;        jbe     .copy_packet
;        and     [esi+upd.pkt_status], 0
;        jmp     .finish
;  .copy_packet:

        DEBUGF  1, "Received %u bytes in buffer %x\n", ecx, [esi+upd.realaddr]:8

        push    dword .loop ;.finish
        push    ecx
        push    [esi+upd.realaddr]

; update statistics
        inc     [device.packets_rx]
        add     dword [device.bytes_rx], ecx
        adc     dword [device.bytes_rx + 4], 0

; update UPD (Alloc new buffer for next packet)
        stdcall KernelAlloc, MAX_ETH_FRAME_SIZE
        mov     [esi + upd.realaddr], eax
        GetRealAddr
        mov     [esi + upd.frag_addr], eax
        and     [esi + upd.pkt_status], 0
        mov     [esi + upd.frag_len], MAX_ETH_FRAME_SIZE or (1 shl 31)

; Update UPD pointer
        add     esi, upd.size
        lea     ecx, [device.upd_buffer+(NUM_RX_DESC)*upd.size]
        cmp     esi, ecx
        jb      @f
        lea     esi, [device.upd_buffer]
       @@:
        mov     [device.curr_upd], esi
        DEBUGF  1, "Next upd: %x\n", esi

        jmp     Eth_input
  .loop:

        mov     ebx, [esp]
        jmp     .receive

  .finish:
        pop     ebx

; check if the NIC is in the upStall state
        set_io  0
        set_io  REG_UP_PKT_STATUS
        in      eax, dx
        test    ah, 0x20             ; UpStalled
        jz      .noUpUnStall

        DEBUGF  1, "upUnStalling\n"
; issue upUnStall command
        set_io  REG_COMMAND
        mov     ax, ((11b shl 12)+1) ; upUnStall
        out     dx, ax

        ;;;; FIXME: make upunstall work

  .noUpUnStall:
.noRX:
        test    word[esp], DownComplete
        jz      .noTX
        DEBUGF  1, "Downcomplete!\n"

        mov     ecx, NUM_TX_DESC
        lea     esi, [device.dpd_buffer]
  .txloop:
        test    [esi+dpd.frame_start_hdr], 1 shl 31
        jz      .maybenext

        and     [esi+dpd.frame_start_hdr], 0
        push    ecx
        stdcall KernelFree, [esi+dpd.realaddr]
        pop     ecx

  .maybenext:
        add     esi, dpd.size
        dec     ecx
        jnz     .txloop

.noTX:
        pop     ax

        set_io  0
        set_io  REG_COMMAND
        or      ax, AckIntr
        out     dx, ax

        set_io  REG_INT_STATUS
        in      ax, dx
        test    ax, S_5_INTS
        jnz     .got_it

;re-enable ints
        set_io  REG_COMMAND
        mov     ax, SetIntrEnb + S_5_INTS
        out     dx, ax

        ret




; End of code
align 4                                         ; Place all initialised data here

macro strtbl name, [string]
{
common
        label name dword
forward
        local label
        dd label
forward
        label db string, 0
}

VORTEX_DEVICES       dd 0
BOOMERANG_DEVICES    dd 0
version              dd (DRIVER_VERSION shl 16) or (API_VERSION and 0xFFFF)
my_service           db '3C59X',0                    ; max 16 chars include zero


strtbl link_str, \
        "No valid link type detected", \
        "10BASE-T half duplex", \
        "10BASE-T full-duplex", \
        "100BASE-TX half duplex", \
        "100BASE-TX full duplex", \
        "100BASE-T4", \
        "100BASE-FX", \
        "10Mbps AUI", \
        "10Mbps COAX (BNC)", \
        "miiDevice - not supported"

strtbl hw_str, \
        "3c590 Vortex 10Mbps", \
        "3c592 EISA 10Mbps Demon/Vortex", \
        "3c597 EISA Fast Demon/Vortex", \
        "3c595 Vortex 100baseTx", \
        "3c595 Vortex 100baseT4", \
        "3c595 Vortex 100base-MII", \
        "3c900 Boomerang 10baseT", \
        "3c900 Boomerang 10Mbps Combo", \
        "3c900 Cyclone 10Mbps TPO", \
        "3c900 Cyclone 10Mbps Combo", \
        "3c900 Cyclone 10Mbps TPC", \
        "3c900B-FL Cyclone 10base-FL", \
        "3c905 Boomerang 100baseTx", \
        "3c905 Boomerang 100baseT4", \
        "3c905B Cyclone 100baseTx", \
        "3c905B Cyclone 10/100/BNC", \
        "3c905B-FX Cyclone 100baseFx", \
        "3c905C Tornado", \
        "3c980 Cyclone", \
        "3c982 Dual Port Server Cyclone", \
        "3cSOHO100-TX Hurricane", \
        "3c555 Laptop Hurricane", \
        "3c556 Laptop Tornado", \
        "3c556B Laptop Hurricane", \
        "3c575 [Megahertz] 10/100 LAN CardBus", \
        "3c575 Boomerang CardBus", \
        "3CCFE575BT Cyclone CardBus", \
        "3CCFE575CT Tornado CardBus", \
        "3CCFE656 Cyclone CardBus", \
        "3CCFEM656B Cyclone+Winmodem CardBus", \
        "3CXFEM656C Tornado+Winmodem CardBus", \
        "3c450 HomePNA Tornado", \
        "3c920 Tornado", \
        "3c982 Hydra Dual Port A", \
        "3c982 Hydra Dual Port B", \
        "3c905B-T4", \
        "3c920B-EMB-WNM Tornado"



align 4
hw_versions:
dw 0x5900, IS_VORTEX                                                                                                            ; 3c590 Vortex 10Mbps
dw 0x5920, IS_VORTEX                                                                                                            ; 3c592 EISA 10Mbps Demon/Vortex
dw 0x5970, IS_VORTEX                                                                                                            ; 3c597 EISA Fast Demon/Vortex
dw 0x5950, IS_VORTEX                                                                                                            ; 3c595 Vortex 100baseTx
dw 0x5951, IS_VORTEX                                                                                                            ; 3c595 Vortex 100baseT4
dw 0x5952, IS_VORTEX                                                                                                            ; 3c595 Vortex 100base-MII
dw 0x9000, IS_BOOMERANG                                                                                                         ; 3c900 Boomerang 10baseT
dw 0x9001, IS_BOOMERANG                                                                                                         ; 3c900 Boomerang 10Mbps Combo
dw 0x9004, IS_CYCLONE or HAS_NWAY or HAS_HWCKSM                                                                                 ; 3c900 Cyclone 10Mbps TPO
dw 0x9005, IS_CYCLONE or HAS_HWCKSM                                                                                             ; 3c900 Cyclone 10Mbps Combo
dw 0x9006, IS_CYCLONE or HAS_HWCKSM                                                                                             ; 3c900 Cyclone 10Mbps TPC
dw 0x900A, IS_CYCLONE or HAS_HWCKSM                                                                                             ; 3c900B-FL Cyclone 10base-FL
dw 0x9050, IS_BOOMERANG or HAS_MII                                                                                              ; 3c905 Boomerang 100baseTx
dw 0x9051, IS_BOOMERANG or HAS_MII                                                                                              ; 3c905 Boomerang 100baseT4
dw 0x9055, IS_CYCLONE or HAS_NWAY or HAS_HWCKSM or EXTRA_PREAMBLE                                                               ; 3c905B Cyclone 100baseTx
dw 0x9058, IS_CYCLONE or HAS_NWAY or HAS_HWCKSM                                                                                 ; 3c905B Cyclone 10/100/BNC
dw 0x905A, IS_CYCLONE or HAS_HWCKSM                                                                                             ; 3c905B-FX Cyclone 100baseFx
dw 0x9200, IS_TORNADO or HAS_NWAY or HAS_HWCKSM                                                                                 ; 3c905C Tornado
dw 0x9800, IS_CYCLONE or HAS_NWAY or HAS_HWCKSM                                                                                 ; 3c980 Cyclone
dw 0x9805, IS_TORNADO or HAS_NWAY or HAS_HWCKSM                                                                                 ; 3c982 Dual Port Server Cyclone
dw 0x7646, IS_CYCLONE or HAS_NWAY or HAS_HWCKSM                                                                                 ; 3cSOHO100-TX Hurricane
dw 0x5055, IS_CYCLONE or EEPROM_8BIT or HAS_HWCKSM                                                                              ; 3c555 Laptop Hurricane
dw 0x6055, IS_TORNADO or HAS_NWAY or EEPROM_8BIT or HAS_CB_FNS or INVERT_MII_PWR or HAS_HWCKSM                                  ; 3c556 Laptop Tornado
dw 0x6056, IS_TORNADO or HAS_NWAY or EEPROM_OFFSET or HAS_CB_FNS or INVERT_MII_PWR or HAS_HWCKSM                                ; 3c556B Laptop Hurricane
dw 0x5b57, IS_BOOMERANG or HAS_MII or EEPROM_8BIT                                                                               ; 3c575 [Megahertz] 10/100 LAN CardBus
dw 0x5057, IS_BOOMERANG or HAS_MII or EEPROM_8BIT                                                                               ; 3c575 Boomerang CardBus
dw 0x5157, IS_CYCLONE or HAS_NWAY or HAS_CB_FNS or EEPROM_8BIT or INVERT_LED_PWR or HAS_HWCKSM                                  ; 3CCFE575BT Cyclone CardBus
dw 0x5257, IS_TORNADO or HAS_NWAY or HAS_CB_FNS or EEPROM_8BIT or INVERT_MII_PWR or MAX_COLLISION_RESET or HAS_HWCKSM           ; 3CCFE575CT Tornado CardBus
dw 0x6560, IS_CYCLONE or HAS_NWAY or HAS_CB_FNS or EEPROM_8BIT or INVERT_MII_PWR or INVERT_LED_PWR or HAS_HWCKSM                ; 3CCFE656 Cyclone CardBus
dw 0x6562, IS_CYCLONE or HAS_NWAY or HAS_CB_FNS or EEPROM_8BIT or INVERT_MII_PWR or INVERT_LED_PWR or HAS_HWCKSM                ; 3CCFEM656B Cyclone+Winmodem CardBus
dw 0x6564, IS_TORNADO or HAS_NWAY or HAS_CB_FNS or EEPROM_8BIT or INVERT_MII_PWR or MAX_COLLISION_RESET or HAS_HWCKSM           ; 3CXFEM656C Tornado+Winmodem CardBus
dw 0x4500, IS_TORNADO or HAS_NWAY or HAS_HWCKSM                                                                                 ; 3c450 HomePNA Tornado
dw 0x9201, IS_TORNADO or HAS_NWAY or HAS_HWCKSM                                                                                 ; 3c920 Tornado
dw 0x1201, IS_TORNADO or HAS_HWCKSM or HAS_NWAY                                                                                 ; 3c982 Hydra Dual Port A
dw 0x1202, IS_TORNADO or HAS_HWCKSM or HAS_NWAY                                                                                 ; 3c982 Hydra Dual Port B
dw 0x9056, IS_CYCLONE or HAS_NWAY or HAS_HWCKSM or EXTRA_PREAMBLE                                                               ; 3c905B-T4
dw 0x9210, IS_TORNADO or HAS_NWAY or HAS_HWCKSM                                                                                 ; 3c920B-EMB-WNM Tornado
HW_VERSIONS_SIZE = $ - hw_versions

include_debug_strings                           ; All data wich FDO uses will be included here

section '.data' data readable writable align 16 ; place all uninitialized data place here

VORTEX_LIST    rd MAX_DEVICES                   ; This list contains all pointers to device structures the driver is handling
BOOMERANG_LIST rd MAX_DEVICES




