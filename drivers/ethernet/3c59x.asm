;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  3Com network driver for KolibriOS                           ;;
;;                                                              ;;
;;  Ported to KolibriOS net-branch by hidnplayr                 ;;
;;                                                              ;;
;;  Thanks to: scrap metal recyclers, whom provide me with      ;;
;;                         loads of hardware                    ;;
;;             diamond: who makes me understand KolibriOS       ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Original copyright from menuetos driver:
;;
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


format PE DLL native
entry START

        CURRENT_API             = 0x0200
        COMPATIBLE_API          = 0x0100
        API_VERSION             = (COMPATIBLE_API shl 16) + CURRENT_API

        MAX_DEVICES             = 16
        FORCE_FD                = 0     ; forcing full duplex mode makes sense at some cards and link types

        NUM_RX_DESC             = 4     ; a power of 2 number
        NUM_TX_DESC             = 4     ; a power of 2 number

        __DEBUG__               = 1
        __DEBUG_LEVEL__         = 2     ; 1 = verbose, 2 = errors only

section '.flat' readable writable executable

include '../proc32.inc'
include '../struct.inc'
include '../macros.inc'
include '../fdo.inc'
include '../netdrv.inc'

; Registers
        REG_POWER_MGMT_CTRL     = 0x7c
        REG_UP_LIST_PTR         = 0x38
        REG_UP_PKT_STATUS       = 0x30
        REG_TX_FREE_THRESH      = 0x2f
        REG_DN_LIST_PTR         = 0x24
        REG_DMA_CTRL            = 0x20
        REG_TX_STATUS           = 0x1b
        REG_RX_STATUS           = 0x18
        REG_TX_DATA             = 0x10

; Common window registers
        REG_INT_STATUS          = 0xe
        REG_COMMAND             = 0xe

; Register window 7
        REG_MASTER_STATUS       = 0xc
        REG_POWER_MGMT_EVENT    = 0xc
        REG_MASTER_LEN          = 0x6
        REG_VLAN_ETHER_TYPE     = 0x4
        REG_VLAN_MASK           = 0x0
        REG_MASTER_ADDRESS      = 0x0

; Register window 6
        REG_BYTES_XMITTED_OK    = 0xc
        REG_BYTES_RCVD_OK       = 0xa
        REG_UPPER_FRAMES_OK     = 0x9
        REG_FRAMES_DEFERRED     = 0x8
        REG_FRAMES_RCVD_OK      = 0x7
        REG_FRAMES_XMITTED_OK   = 0x6
        REG_RX_OVERRUNS         = 0x5
        REG_LATE_COLLISIONS     = 0x4
        REG_SINGLE_COLLISIONS   = 0x3
        REG_MULTIPLE_COLLISIONS = 0x2
        REG_SQE_ERRORS          = 0x1
        REG_CARRIER_LOST        = 0x0

; Register window 5
        REG_INDICATION_ENABLE   = 0xc
        REG_INTERRUPT_ENABLE    = 0xa
        REG_TX_RECLAIM_THRESH   = 0x9
        REG_RX_FILTER           = 0x8
        REG_RX_EARLY_THRESH     = 0x6
        REG_TX_START_THRESH     = 0x0

; Register window 4
        REG_UPPER_BYTES_OK      = 0xe
        REG_BAD_SSD             = 0xc
        REG_MEDIA_STATUS        = 0xa
        REG_PHYSICAL_MGMT       = 0x8
        REG_NETWORK_DIAGNOSTIC  = 0x6
        REG_FIFO_DIAGNOSTIC     = 0x4
        REG_VCO_DIAGNOSTIC      = 0x2   ; may not supported

; Bits in register window 4
        BIT_AUTOSELECT          = 24

; Register window 3
        REG_TX_FREE             = 0xc
        REG_RX_FREE             = 0xa
        REG_MEDIA_OPTIONS       = 0x8
        REG_MAC_CONTROL         = 0x6
        REG_MAX_PKT_SIZE        = 0x4
        REG_INTERNAL_CONFIG     = 0x0

; Register window 2
        REG_RESET_OPTIONS       = 0xc
        REG_STATION_MASK_HI     = 0xa
        REG_STATION_MASK_MID    = 0x8
        REG_STATION_MASK_LO     = 0x6
        REG_STATION_ADDRESS_HI  = 0x4
        REG_STATION_ADDRESS_MID = 0x2
        REG_STATION_ADDRESS_LO  = 0x0

; Register window 1
        REG_TRIGGER_BITS        = 0xc
        REG_SOS_BITS            = 0xa
        REG_WAKE_ON_TIMER       = 0x8
        REG_SMB_RXBYTES         = 0x7
        REG_SMB_DIAG            = 0x5
        REG_SMB_ARB             = 0x4
        REG_SMB_STATUS          = 0x2
        REG_SMB_ADDRESS         = 0x1
        REG_SMB_FIFO_DATA       = 0x0

; Register window 0
        REG_EEPROM_DATA         = 0xc
        REG_EEPROM_COMMAND      = 0xa
        REG_BIOS_ROM_DATA       = 0x8
        REG_BIOS_ROM_ADDR       = 0x4

; Physical management bits
        BIT_MGMT_DIR            = 2     ; drive with the data written in mgmtData
        BIT_MGMT_DATA           = 1     ; MII management data bit
        BIT_MGMT_CLK            = 0     ; MII management clock

; MII commands
        MII_CMD_MASK            = (1111b shl 10)
        MII_CMD_READ            = (0110b shl 10)
        MII_CMD_WRITE           = (0101b shl 10)

; eeprom bits and commands
        EEPROM_CMD_READ         = 0x80
        EEPROM_BIT_BUSY         = 15

; eeprom registers
        EEPROM_REG_OEM_NODE_ADDR= 0xa
        EEPROM_REG_CAPABILITIES = 0x10

; Commands for command register
        SELECT_REGISTER_WINDOW  = (1 shl 11)

; Hw capabilities bitflags
        IS_VORTEX               = 0x0001
        IS_BOOMERANG            = 0x0002
        IS_CYCLONE              = 0x0004
        IS_TORNADO              = 0x0008
        EEPROM_8BIT             = 0x0010
        HAS_PWR_CTRL            = 0x0020
        HAS_MII                 = 0x0040
        HAS_NWAY                = 0x0080
        HAS_CB_FNS              = 0x0100
        INVERT_MII_PWR          = 0x0200
        INVERT_LED_PWR          = 0x0400
        MAX_COLLISION_RESET     = 0x0800
        EEPROM_OFFSET           = 0x1000
        HAS_HWCKSM              = 0x2000
        EXTRA_PREAMBLE          = 0x4000

; Status
        IntLatch                = 0x0001
        HostError               = 0x0002
        TxComplete              = 0x0004
        TxAvailable             = 0x0008
        RxComplete              = 0x0010
        RxEarly                 = 0x0020
        IntReq                  = 0x0040
        StatsFull               = 0x0080
        DMADone                 = 0x0100
        DownComplete            = 0x0200
        UpComplete              = 0x0400
        DMAInProgress           = 0x0800        ; 1 shl 11  (DMA controller is still busy)
        CmdInProgress           = 0x1000        ; 1 shl 12  (EL3_CMD is still busy)

        S_5_INTS                = HostError + RxEarly + UpComplete + DownComplete + StatsFull ;+ TxComplete + RxComplete  + TxAvailable

; Commands
        TotalReset              = 0 shl 11
        SelectWindow            = 1 shl 11
        StartCoax               = 2 shl 11
        RxDisable               = 3 shl 11
        RxEnable                = 4 shl 11
        RxReset                 = 5 shl 11
        UpStall                 = 6 shl 11
        UpUnstall               = (6 shl 11)+1
        DownStall               = (6 shl 11)+2
        DownUnstall             = (6 shl 11)+3
        RxDiscard               = 8 shl 11
        TxEnable                = 9 shl 11
        TxDisable               = 10 shl 11
        TxReset                 = 11 shl 11
        FakeIntr                = 12 shl 11
        AckIntr                 = 13 shl 11
        SetIntrEnb              = 14 shl 11
        SetStatusEnb            = 15 shl 11
        SetRxFilter             = 16 shl 11
        SetRxThreshold          = 17 shl 11
        SetTxThreshold          = 18 shl 11
        SetTxStart              = 19 shl 11
        StartDMAUp              = 20 shl 11
        StartDMADown            = (20 shl 11)+1
        StatsEnable             = 21 shl 11
        StatsDisable            = 22 shl 11
        StopCoax                = 23 shl 11
        SetFilterBit            = 25 shl 11

; Rx mode bits
        RxStation               = 1
        RxMulticast             = 2
        RxBroadcast             = 4
        RxProm                  = 8

        MAX_ETH_FRAME_SIZE      = 1514


struct  tx_desc

        next_ptr                dd ?
        frame_start_hdr         dd ?
        frag_addr               dd ?    ; for packet data
        frag_len                dd ?    ; for packet data
        realaddr                dd ?
                                rd 3    ; align 32
ends


struct  rx_desc

        next_ptr                dd ?
        pkt_status              dd ?
        frag_addr               dd ?
        frag_len                dd ?    ; for packet data
        realaddr                dd ?
                                rd 3    ; align 32
ends


struct  device                  ETH_DEVICE

        io_addr                 dd ?
        pci_bus                 dd ?
        pci_dev                 dd ?
        irq_line                db ?
                                rb 3    ; alignment

        curr_tx                 dd ?
        curr_rx                 dd ?
        prev_tx_frame           dd ?
        ver_id                  db ?
        full_bus_master         db ?
        has_hwcksm              db ?
        preamble                db ?
        dn_list_ptr_cleared     db ?
        internal_link           dd ?    ; link state (to be used only internally by driver)

        rb 0x100 - ($ and 0xff) ; align 256
        tx_desc_buffer          rd (sizeof.tx_desc*NUM_TX_DESC)/4
        rx_desc_buffer          rd (sizeof.rx_desc*NUM_RX_DESC)/4

ends


;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                        ;;
;; proc START             ;;
;;                        ;;
;; (standard driver proc) ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc START c, reason:dword, cmdline:dword

        cmp     [reason], DRV_ENTRY
        jne     .fail

        DEBUGF  2,"Loading driver\n"
        invoke  RegService, my_service, service_proc
        ret

  .fail:
        xor     eax, eax
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
        mov     eax, [edx + IOCTL.io_code]

;------------------------------------------------------

        cmp     eax, 0 ;SRV_GETVERSION
        jne     @F

        cmp     [edx + IOCTL.out_size], 4
        jb      .fail
        mov     eax, [edx + IOCTL.output]
        mov     [eax], dword API_VERSION

        xor     eax, eax
        ret

;------------------------------------------------------
  @@:
        cmp     eax, 1 ;SRV_HOOK
        jne     .fail

        cmp     [edx + IOCTL.inp_size], 3               ; Data input must be at least 3 bytes
        jb      .fail

        mov     eax, [edx + IOCTL.input]
        cmp     byte [eax], 1                           ; 1 means device number and bus number (pci) are given
        jne     .fail                                   ; other types of this hardware dont exist

; check if the device is already listed

        mov     ecx, [vortex_devices]
        test    ecx, ecx
        jz      .maybeboomerang

        mov     esi, vortex_list
        mov     eax, [edx + IOCTL.input]                ; get the pci bus and device numbers
        mov     ax , [eax+1]                            ;
  .nextdevice:
        mov     ebx, [esi]
        cmp     al, byte[ebx + device.pci_bus]
        jne     @f
        cmp     ah, byte[ebx + device.pci_dev]
        je      .find_devicenum                         ; Device is already loaded, let's find it's device number
       @@:
        add     esi, 4
        loop    .nextdevice


  .maybeboomerang:
        mov     ecx, [boomerang_devices]
        test    ecx, ecx
        jz      .firstdevice

        mov     esi, boomerang_list
        mov     eax, [edx + IOCTL.input]                ; get the pci bus and device numbers
        mov     ax, [eax+1]                             ;
  .nextdevice2:
        mov     ebx, [esi]
        cmp     al, byte[ebx + device.pci_bus]
        jne     @f
        cmp     ah, byte[ebx + device.pci_dev]
        je      .find_devicenum                         ; Device is already loaded, let's find it's device number
       @@:
        add     esi, 4
        loop    .nextdevice2


; This device doesnt have its own eth_device structure yet, lets create one
  .firstdevice:
        mov     ecx, [boomerang_devices]
        add     ecx, [vortex_devices]
        cmp     ecx, MAX_DEVICES                        ; First check if the driver can handle one more card
        jae     .fail

        allocate_and_clear ebx, sizeof.device, .fail    ; Allocate the buffer for device structure

; Fill in the direct call addresses into the struct

        mov     [ebx + device.reset], reset
        mov     [ebx + device.transmit], null_op
        mov     [ebx + device.unload], null_op
        mov     [ebx + device.name], my_service

; save the pci bus and device numbers

        mov     eax, [edx + IOCTL.input]
        movzx   ecx, byte[eax+1]
        mov     [ebx + device.pci_bus], ecx
        movzx   ecx, byte[eax+2]
        mov     [ebx + device.pci_dev], ecx

; Now, it's time to find the base io addres of the PCI device

        stdcall PCI_find_io, [ebx + device.pci_bus], [ebx + device.pci_dev]
        mov     [ebx + device.io_addr], eax

; We've found the io address, find IRQ now

        invoke  PciRead8, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.interrupt_line
        mov     [ebx + device.irq_line], al

        DEBUGF  1,"Hooking into device, dev:%x, bus:%x, irq:%x, addr:%x\n",\
        [ebx + device.pci_dev]:1,[ebx + device.pci_bus]:1,[ebx + device.irq_line]:1,[ebx + device.io_addr]:4

; Ok, the eth_device structure is ready, let's probe the device
        call    probe                                                   ; this function will output in eax
        test    eax, eax
        jnz     .err                                                    ; If an error occured, exit


        movzx   ecx, [ebx + device.ver_id]
        test    word [hw_versions+2+ecx*4], IS_VORTEX
        jz      .not_vortex

        mov     eax, [vortex_devices]                                   ; Add the device structure to our device list
        mov     [vortex_list+4*eax], ebx                                ; (IRQ handler uses this list to find device)
        inc     [vortex_devices]                                        ;

  .register:
        mov     [ebx + device.type], NET_TYPE_ETH
        invoke  NetRegDev

        cmp     eax, -1
        je      .destroy

        call    start_device
        ret

  .not_vortex:
        mov     eax, [boomerang_devices]                                ; Add the device structure to our device list
        mov     [boomerang_list+4*eax], ebx                             ; (IRQ handler uses this list to find device)
        inc     [boomerang_devices]

        jmp     .register

; If the device was already loaded, find the device number and return it in eax

  .find_devicenum:
        DEBUGF  2,"Trying to find device number of already registered device\n"
        invoke  NetPtrToNum                                             ; This kernel procedure converts a pointer to device struct in ebx
                                                                        ; into a device number in edi
        mov     eax, edi                                                ; Application wants it in eax instead
        DEBUGF  2,"Kernel says: %u\n", eax
        ret

; If an error occured, remove all allocated data and exit (returning -1 in eax)

  .destroy:
        ; todo: reset device into virgin state

  .err:
        invoke  KernelFree, ebx
  .fail:
        DEBUGF  2, "Failed to load\n"
        or      eax, -1
        ret

;------------------------------------------------------
endp


;;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\;;
;;                                                                        ;;
;;        Actual Hardware dependent code starts here                      ;;
;;                                                                        ;;
;;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\;;


align 4
null_op:

        ret



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

; Make the device a bus master
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command
        or      al, PCI_CMD_MASTER
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command, eax

; wake up the card
        DEBUGF 1,"Checking if the device is awake\n"

; wake up - we directly do it by programming PCI
; check if the device is power management capable
        invoke  PciRead16, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.status
        test    al, PCI_STATUS_CAPA     ; is there "new capabilities" linked list?
        jz      .device_awake

; search for power management register
        invoke  PciRead8, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.cap_ptr
        and     al, not 3
        cmp     al, 0x3f
        jbe     .device_awake           ; invalid pointer if less then PCI header size

; traverse the list
        movzx   esi, al
  .pm_loop:
        invoke  PciRead16, [ebx + device.pci_bus], [ebx + device.pci_dev], esi
        cmp     al, 1
        je      .set_pm_state
        movzx   esi, ah                 ; load address of next capability
        test    ah, ah                  ; 0 if none left
        jnz     .pm_loop
        jmp     .device_awake

; wake up the device if necessary
  .set_pm_state:
        DEBUGF 1,"Waking up the device\n"
        add     esi, 4                  ; offset for power management
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], esi
        test    al, 11b
        jz      .device_awake
        and     al, not 11b             ; set state to D0
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], esi, eax

  .device_awake:
        DEBUGF 1,"Device is awake\n"

; Check device/vendor ID
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], 0

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
        DEBUGF  2,"Device id not found in list!\n"
        or      eax, -1
        ret
  .found:
        mov     esi, [hw_str+ecx*4]
        DEBUGF  1,"Hardware type: %s\n", esi
        mov     [ebx + device.name], esi

        mov     [ebx + device.ver_id], cl
        test    word [hw_versions+2+ecx*4], HAS_HWCKSM
        setnz   [ebx + device.has_hwcksm]
; set pci latency for vortex cards
        test    word [hw_versions+2+ecx*4], IS_VORTEX
        jz      .not_vortex

        mov     al, 11111000b ; 248 = max latency
        invoke  PciWrite8, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.max_latency, eax

  .not_vortex:
; set RX/TX functions
        mov     ax, EEPROM_REG_CAPABILITIES
        call    read_eeprom
        test    al, 100000b ; full bus master?
        setnz   [ebx + device.full_bus_master]
        jnz     .boomerang_func
        mov     [ebx + device.transmit], vortex_transmit
        DEBUGF  2,"Device is a vortex type\n"
        DEBUGF  2,"I'm sorry but vortex code hasnt been tested yet\n"
        DEBUGF  2,"Please contact me on hidnplayr@kolibrios.org\n"
        DEBUGF  2,"If you can help me finish it!\n"
        or      eax, -1
        ret
        jmp     @f
  .boomerang_func: ; full bus master, so use boomerang functions
        mov     [ebx + device.transmit], boomerang_transmit
        DEBUGF  1,"Device is a boomerang type\n"
       @@:
        call    read_mac_eeprom

        test    byte [ebx + device.full_bus_master], 0xff
        jz      .set_preamble
; switch to register window 2
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+2
        out     dx, ax
; activate xcvr by setting some magic bits
        set_io  [ebx + device.io_addr], REG_RESET_OPTIONS
        in      ax, dx
        and     ax, not 0x4010
        movzx   ecx, [ebx + device.ver_id]
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
        mov     byte [ebx + device.preamble], 1 ; enable preamble

        DEBUGF 1,"Global reset..\n"

; GlobalReset
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
        xor     eax, eax
;       or      al, 0x14
        out     dx, ax
; wait for GlobalReset to complete
        mov     ecx, 64000
  .rsloop:
        in      ax , dx
        test    ah , 10000b             ; CmdInProgress
        loopz   .rsloop

        DEBUGF 1,"Waiting for nic to boot..\n"
; wait for 2 seconds for NIC to boot
        mov     esi, 2000               ; WTF? FIXME
        invoke  Sleep ; 2 seconds

        DEBUGF 1,"Ok!\n"


;--------------------------
;
; RESET
;
;--------------------------

reset:

        movzx   eax, [ebx + device.irq_line]
        DEBUGF  1,"Attaching int handler to irq %x\n", eax:1

        movzx   ecx, [ebx + device.ver_id]
        test    word [hw_versions+2+ecx*4], IS_VORTEX
        jz      .not_vortex
        mov     esi, int_vortex
        jmp     .reg_int
  .not_vortex:
        mov     esi, int_boomerang
  .reg_int:
        invoke  AttachIntHandler, eax, esi, ebx
        test    eax, eax
        jnz     @f
        DEBUGF  2,"Could not attach int handler!\n"
        or      eax, -1
        ret
  @@:

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW + 0
        out     dx, ax

        mov     ax, StopCoax
        out     dx, ax                        ; stop transceiver

        mov     ax, SELECT_REGISTER_WINDOW + 4
        out     dx, ax                        ; disable UTP

        set_io  [ebx + device.io_addr], REG_MEDIA_STATUS
        mov     ax, 0x0

        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW + 0
        out     dx, ax

        set_io  [ebx + device.io_addr], REG_FIFO_DIAGNOSTIC
        mov     ax, 0
        out     dx, ax                        ; disable card

        mov     ax, 1
        out     dx, ax                        ; enable card

        call    write_mac

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW + 1
        out     dx, ax

        mov     ecx, 32
        set_io  [ebx + device.io_addr], 0x0b
  .loop:
        in      al, dx
        loop    .loop

; Get rid of stray ints
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, AckIntr + 0xff
        out     dx, ax

        mov     ax, SetStatusEnb + S_5_INTS
        out     dx, ax

        mov     ax, SetIntrEnb + S_5_INTS
        out     dx, ax

        DEBUGF  1,"Setting RX mode\n"

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND

if      defined PROMISCIOUS
        mov     ax, SetRxFilter + RxStation + RxMulticast + RxBroadcast + RxProm
else if  defined ALLMULTI
        mov     ax, SetRxFilter + RxStation + RxMulticast + RxBroadcast
else
        mov     ax, SetRxFilter + RxStation + RxBroadcast
end if
        out     dx, ax

        call    set_active_port

        call    create_rx_ring
        test    eax, eax
        jnz     .err

        call    rx_reset
        call    tx_reset

        xor     eax, eax
; clear packet/byte counters

        lea     edi, [ebx + device.bytes_tx]
        mov     ecx, 6
        rep     stosd

        xor     eax, eax
        ret

  .err:
        DEBUGF  2,"reset failed\n"
        or      eax, -1
        ret


align 4
start_device:
        DEBUGF  1,"Starting the device\n"

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, SetTxThreshold + 60 ;2047 ; recommended by the manual :)
        out     dx, ax

        call    check_tx_status

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
; switch to register window 4
        mov     ax, SELECT_REGISTER_WINDOW+4
        out     dx, ax

; wait for linkDetect
        set_io  [ebx + device.io_addr], REG_MEDIA_STATUS
        mov     ecx, 20         ; wait for max 2s
  .link_detect_loop:
        mov     esi, 100
        invoke  Sleep           ; 100 ms
        in      ax, dx
        test    ah, 1000b       ; linkDetect
        jnz     @f
        loop    .link_detect_loop
        DEBUGF  2,"Link detect timed-out!\n"
       @@:

; print link type
        xor     eax, eax
        bsr     ax, word[ebx + device.internal_link]
        jz      @f
        sub     ax, 5
       @@:

        mov     esi, [link_str+eax*4]
        DEBUGF  1,"Established Link type: %s\n", esi

; enable interrupts
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW + 1
        out     dx, ax

        mov     ax, AckIntr + 0xff
        out     dx, ax

        mov     ax, SetStatusEnb + S_5_INTS
        out     dx, ax

        mov     ax, SetIntrEnb + S_5_INTS
        out     dx, ax

; Start RX/TX

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, RxEnable
        out     dx, ax

        mov     ax, TxEnable
        out     dx, ax

        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, SetRxThreshold + 208
        out     dx, ax

        mov     ax, SetTxThreshold + 60 ;16 ; recommended by the manual :)
        out     dx, ax

        mov     ax, SELECT_REGISTER_WINDOW + 1
        out     dx, ax

; Set the mtu, kernel will be able to send now
        mov     [ebx + device.mtu], 1514

; Set link state to unknown
        mov     [ebx + device.state], ETH_LINK_UNKNOWN
        xor     eax, eax

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
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
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
; init last TX descriptor
        lea     eax, [ebx + device.tx_desc_buffer + (NUM_TX_DESC-1)*sizeof.tx_desc]
        mov     [ebx + device.curr_tx], eax

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

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
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

        lea     eax, [ebx + device.rx_desc_buffer]
        mov     [ebx + device.curr_rx], eax
        invoke  GetPhysAddr
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_UP_LIST_PTR
        out     dx, eax

  .rx_enable:
        ret



align 4
create_rx_ring:
; create rx descriptor ring
        lea     eax, [ebx + device.rx_desc_buffer]
        invoke  GetPhysAddr
        mov     edi, eax                                                ; real addr of first descr

        lea     esi, [ebx + device.rx_desc_buffer]                          ; ptr to first descr
        lea     edx, [ebx + device.rx_desc_buffer + (NUM_RX_DESC-1)*sizeof.rx_desc]     ; ptr to last descr

        mov     ecx, NUM_RX_DESC
  .loop:
        mov     [edx + rx_desc.next_ptr], edi

        push    ecx edx
        invoke  NetAlloc, MAX_ETH_FRAME_SIZE+NET_BUFF.data
        pop     edx ecx
        test    eax, eax
        jz      .out_of_mem
        mov     [esi + rx_desc.realaddr], eax
        invoke  GetPhysAddr
        add     eax, NET_BUFF.data
        mov     [esi + rx_desc.frag_addr], eax
        and     [esi + rx_desc.pkt_status], 0
        mov     [esi + rx_desc.frag_len], MAX_ETH_FRAME_SIZE or (1 shl 31)

        DEBUGF  1,"rx_desc: lin=%x phys=%x len=%x next ptr=%x\n", [esi+rx_desc.realaddr]:8, [esi+rx_desc.frag_addr]:8, [esi+rx_desc.frag_len]:8, edi
        DEBUGF  1,"rx_desc: cur=%x prev=%x\n", esi, edx

        mov     edx, esi
        add     esi, sizeof.rx_desc
        add     edi, sizeof.rx_desc
        dec     ecx
        jnz     .loop

        xor     eax, eax
        ret

  .out_of_mem:
        or      eax, -1
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

        DEBUGF  1,"Trying to detect link\n"

; create self-directed packet
        invoke  NetAlloc, 20+NET_BUFF.data     ; create a buffer for the self-directed packet
        test    eax, eax
        jz      .fail

        push    eax
        mov     [eax + NET_BUFF.length], 20

        lea     edi, [eax + NET_BUFF.data]
        lea     esi, [ebx + device.mac]
        movsw
        movsd
        sub     esi, 6
        movsw
        movsd
        mov     ax , 0x0608
        stosw

; download self-directed packet
        call    [ebx + device.transmit]

; switch to register window 4
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+4
        out     dx, ax

; See if we have received the packet by now..
        cmp     [ebx + device.packets_rx], 0
        jnz     .link_detected

; switch to register window 4
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+4
        out     dx, ax

; read linkbeatdetect
        set_io  [ebx + device.io_addr], REG_MEDIA_STATUS
        in      ax, dx
        test    ah, 1000b ; test linkBeatDetect
        jnz     .link_detected
        xor     al, al
        jmp     .finish

  .link_detected:
        DEBUGF  1,"Link detected!\n"
        setb    al

  .finish:
        test    al, al
        jz      @f
        or      byte [ebx + device.internal_link+1], 100b
       @@:
        xor     eax, eax
        ret

  .fail:
        DEBUGF  1,"No link detected!\n"
        or      eax, -1
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

        mov     al, MII_BMCR
        push    eax
        call    mdio_read       ; returns with window #4
        or      ah, 0x80        ; software reset
        mov     esi, eax
        mov     eax, [esp]
        call    mdio_write      ; returns with window #4

; wait for reset to complete
        mov     esi, 2000
        invoke  Sleep           ; 2s FIXME
        mov     eax, [esp]
        call    mdio_read       ; returns with window #4
        test    ah, 0x80
        jnz     .fail1
        mov     eax, [esp]

; wait for a while after reset
        mov     esi, 20
        invoke  Sleep           ; 20ms
        mov     eax, [esp]
        mov     al , MII_BMSR
        call    mdio_read        ; returns with window #4
        test    al, 1            ; extended capability supported?
        jz      .fail2
        DEBUGF  1,"Extended capability supported\n"

; auto-neg capable?
        test    al , 1000b
        jz      .fail2           ; not auto-negotiation capable
        DEBUGF  1,"Auto-negotiation capable\n"

; auto-neg complete?
        test    al , 100000b
        jnz     .auto_neg_ok
        DEBUGF  1,"Restarting auto-negotiation\n"

; restart auto-negotiation
        mov     eax, [esp]
        mov     al, MII_ADVERTISE
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
        invoke  Sleep  ; 4 seconds
        mov     eax, [esp]
        mov     al , MII_BMSR
        call    mdio_read ; returns with window #4
        test    al , 100000b ; auto-neg complete?
        jnz     .auto_neg_ok
        jmp     .fail3
  .auto_neg_ok:
        DEBUGF  1,"Auto-negotiation complete\n"

; compare advertisement and link partner ability registers
        mov     eax, [esp]
        mov     al, MII_ADVERTISE
        call    mdio_read               ; returns with window #4
        xchg    eax, [esp]
        mov     al, MII_LPA
        call    mdio_read               ; returns with window #4
        pop     esi
        and     eax, esi
        and     eax, 11111100000b       ; Mask off
        push    eax
        mov     word[ebx + device.internal_link+2], ax

; switch to register window 3
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+3
        out     dx, ax

; set full-duplex mode
        set_io  [ebx + device.io_addr], REG_MAC_CONTROL
        in      ax, dx
        and     ax, not 0x120           ; clear full duplex and flow control
        pop     esi
        test    esi, 1010b shl 5        ; check for full-duplex
        jz      .half_duplex
        or      ax, 0x120               ; set full duplex and flow control
  .half_duplex:
        DEBUGF 1,"Using half-duplex\n"
        out     dx, ax
        mov     al, 1
        ret

  .fail1:
        DEBUGF  2,"reset failed!\n"
        pop     eax
        xor     al, al
        ret

  .fail2:
        DEBUGF  2,"This device is not auto-negotiation capable!\n"
        pop     eax
        xor     al, al
        ret

  .fail3:
        DEBUGF  2,"Auto-negotiation reset failed!\n"
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
;
;   Return value
;      al - 0
;      al - 1
;   Destroyed registers
;      eax, ebx, ecx, edx, esi
;
;***************************************************************************

align 4
try_mii:

        DEBUGF  1,"Trying to find MII PHY\n"

; switch to register window 3
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+3
        out     dx, ax
        set_io  [ebx + device.io_addr], REG_INTERNAL_CONFIG
        in      eax, dx
        and     eax, (1111b shl 20)
        cmp     eax, (1000b shl 20) ; is auto-negotiation set?
        jne     .mii_device

        DEBUGF  1,"Auto-negotiation is set\n"
; switch to register window 4
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+4
        out     dx, ax

; PHY==24 is the on-chip auto-negotiation logic
; it supports only 10base-T and 100base-TX
        mov     ah, 24
        call    try_phy
        test    al, al
        jz      .fail

        mov     cl, 24
        jmp     .check_preamble

  .mii_device:
        cmp     eax, (0110b shl 20)
        jne     .fail

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax , SELECT_REGISTER_WINDOW+4
        out     dx , ax

        set_io  [ebx + device.io_addr], REG_PHYSICAL_MGMT
        in      ax , dx
        and     al , (1 shl BIT_MGMT_DIR) or (1 shl BIT_MGMT_DATA)
        cmp     al , (1 shl BIT_MGMT_DATA)
        je      .search_for_phy

        xor     al, al
        ret

  .search_for_phy:
; search for PHY
        mov     cx, 31
  .search_phy_loop:
        DEBUGF  1,"Searching for the PHY\n"
        cmp     cx, 24
        je      .next_phy
        mov     ah, cl                  ; ah = phy
        mov     al, MII_BMSR            ; al = Basic Mode Status Register       ; BUGFIX HERE! :):)
        push    cx
        call    mdio_read
        pop     cx
        test    ax, ax
        jz      .next_phy
        cmp     ax, 0xffff
        je      .next_phy
        mov     ah, cl                  ; ah = phy
        push    cx
        call    try_phy
        pop     cx
        test    al, al
        jnz     .check_preamble
  .next_phy:
        loopw   .search_phy_loop

  .fail:
        DEBUGF  1,"PHY not found\n"
        xor     al, al
        ret

; epilog
  .check_preamble:
        DEBUGF  1,"Using PHY: %u\nChecking PreAmble\n", cl
        push    eax ; eax contains the return value of try_phy
; check hard coded preamble forcing
        movzx   eax, [ebx + device.ver_id]
        test    word [eax*4+hw_versions+2], EXTRA_PREAMBLE
        setnz   [ebx + device.preamble] ; force preamble
        jnz     .finish

; check mii for preamble suppression
        mov     ah, cl
        mov     al, MII_BMSR
        call    mdio_read
        test    al, 1000000b ; preamble suppression?
        setz    [ebx + device.preamble] ; no

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

        DEBUGF 1,"Sending test packet\n"

; switch to register window 3
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+3
        out     dx, ax

; set fullDuplexEnable in MacControl register
        set_io  [ebx + device.io_addr], REG_MAC_CONTROL
        in      ax, dx
        or      ax, 0x120
        out     dx, ax

; switch to register window 5
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+5
        out     dx, ax

; set RxFilter to enable individual address matches
        mov     ax, (10000b shl 11)
        set_io  [ebx + device.io_addr], REG_RX_FILTER
        in      al, dx
        or      al, 1
        set_io  [ebx + device.io_addr], REG_COMMAND
        out     dx, ax

; issue RxEnable and TxEnable
        call    rx_reset
        call    tx_reset

; create self-directed packet
        invoke  NetAlloc, 20 + NET_BUFF.data   ; create a buffer for the self-directed packet
        test    eax, eax
        jz      .fail

        push    eax
        mov     [eax + NET_BUFF.length], 20
;        mov     [eax + NET_BUFF.device], ebx

        lea     edi, [eax + NET_BUFF.data]
        lea     esi, [ebx + device.mac]
        movsw
        movsd
        sub     esi, 6
        movsw
        movsd
        mov     ax, 0x0608
        stosw

; download self-directed packet
        call    [ebx + device.transmit]

; wait for 2s
        mov     esi, 2000       ; FIXME
        invoke  Sleep

; check if self-directed packet is received
        mov     eax, [ebx + device.packets_rx]
        test    eax, eax
        jnz     .finish

; switch to register window 3
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+3
        out     dx, ax

; clear fullDuplexEnable in MacControl register
        set_io  [ebx + device.io_addr], REG_MAC_CONTROL
        in      ax, dx
        and     ax, not 0x120
        out     dx, ax
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
;
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
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+3
        out     dx, ax
        mov     eax, [esp]

        mov     cl, al
        inc     cl
        shl     cl, 3
        or      byte [ebx + device.internal_link+1], cl

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
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, (10111b shl 11)
        out     dx, ax
  .aui_finish:
        pop     eax ; al contains the result of operation

        test    al, al
        jnz     @f
        and     byte [ebx + device.internal_link+1], not 11000b
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
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW + 3
        out     dx, ax

        set_io  [ebx + device.io_addr], REG_INTERNAL_CONFIG
        in      eax, dx
        test    eax, (1 shl 24) ; check if autoselect enable
        jz      .set_first_available_media

; check 100BASE-TX and 10BASE-T
        set_io  [ebx + device.io_addr], REG_MEDIA_OPTIONS
        in      ax, dx
        test    al, 1010b       ; check whether 100BASE-TX or 10BASE-T available
        jz      .mii_device     ; they are not available

; set auto-negotiation
        set_io  [ebx + device.io_addr], REG_INTERNAL_CONFIG
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
        set_io  [ebx + device.io_addr], 0
; check for off-chip mii device
        set_io  [ebx + device.io_addr], REG_MEDIA_OPTIONS
        in      ax, dx
        test    al, 1000000b ; check miiDevice
        jz      .base_fx
        set_io  [ebx + device.io_addr], REG_INTERNAL_CONFIG
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
        set_io  [ebx + device.io_addr], 0
; check for 100BASE-FX
        set_io  [ebx + device.io_addr], REG_MEDIA_OPTIONS
        in      ax, dx ; read media option register
        test    al, 100b ; check 100BASE-FX
        jz      .aui_enable
        set_io  [ebx + device.io_addr], REG_INTERNAL_CONFIG
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
        set_io  [ebx + device.io_addr], 0
; check for 10Mbps AUI connector
        set_io  [ebx + device.io_addr], REG_MEDIA_OPTIONS
        in      ax, dx ; read media option register
        test    al, 100000b ; check 10Mbps AUI connector
        jz      .coax_available
        set_io  [ebx + device.io_addr], REG_INTERNAL_CONFIG
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
        set_io  [ebx + device.io_addr], 0
; check for coaxial 10BASE-2 port
        set_io  [ebx + device.io_addr], REG_MEDIA_OPTIONS
        in      ax, dx ; read media option register
        test    al, 10000b ; check 10BASE-2
        jz      .set_first_available_media

        set_io  [ebx + device.io_addr], REG_INTERNAL_CONFIG
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
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+3
        out     dx, ax

        set_io  [ebx + device.io_addr], REG_MEDIA_OPTIONS
        in      ax, dx
        DEBUGF  1,"available media:%x\n", al
        mov     cl, al

        set_io  [ebx + device.io_addr], REG_INTERNAL_CONFIG
        in      eax, dx
        and     eax, not (1111b shl 20) ; these bits hold the 'transceiver select' value

        test    cl, 10b         ; baseTXAvailable
        jz      @f

        DEBUGF  1,"base TX is available\n"
        or      eax, (100b shl 20)
if FORCE_FD
        mov     word [ebx + device.internal_link], (1 shl 8)
else
        mov     word [ebx + device.internal_link], (1 shl 7)
end if
        jmp     .set_media
       @@:

        test    cl, 100b        ; baseFXAvailable
        jz      @f

        DEBUGF  1,"base FX is available\n"
        or      eax, (101b shl 20)
        mov     word [ebx + device.internal_link], (1 shl 10)
        jmp     .set_media
       @@:

        test    cl, 1000000b    ; miiDevice
        jz      @f

        DEBUGF  1,"mii-device is available\n"
        or      eax, (0110b shl 20)
        mov     word [ebx + device.internal_link], (1 shl 13)
        jmp     .set_media
       @@:

        test    cl, 1000b       ; 10bTAvailable
        jz      @f

        DEBUGF  1,"10base-T is available\n"
  .set_default:
if FORCE_FD
        mov     word [ebx + device.internal_link], (1 shl 6)
else
        mov     word [ebx + device.internal_link], (1 shl 5)
end if
        jmp     .set_media
       @@:

        test    cl, 10000b      ; coaxAvailable
        jz      @f

        DEBUGF  1,"coax is available\n"
        push    eax
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, (10b shl 11) ; EnableDcConverter
        out     dx, ax
        pop     eax

        or      eax, (11b shl 20)
        mov     word [ebx + device.internal_link], (1 shl 12)
        jmp     .set_media
       @@:

        test    cl, 10000b      ; auiAvailable
        jz      .set_default

        DEBUGF  1,"AUI is available\n"
        or      eax, (1 shl 20)
        mov     word [ebx + device.internal_link], (1 shl 11)

  .set_media:
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_INTERNAL_CONFIG
        out     dx, eax

if FORCE_FD
        DEBUGF  1,"Forcing full duplex\n"
        set_io  [ebx + device.io_addr], REG_MAC_CONTROL
        in      ax, dx
        or      ax, 0x120
        out     dx, ax
end if

        mov     al, 1
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
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+0
        out     dx, ax
        pop     eax
        and     ax, 111111b ; take only the first 6 bits into account
        movzx   esi, [ebx + device.ver_id]

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

        set_io  [ebx + device.io_addr], REG_EEPROM_COMMAND
        out     dx, ax
        mov     ecx, 0xffff ; duration of about 162 us ;-)
.wait_for_reading:
        in      ax, dx
        test    ah, 0x80 ; check bit eepromBusy
        jz      .read_data
        loop    .wait_for_reading
.read_data:
        set_io  [ebx + device.io_addr], REG_EEPROM_DATA
        in      ax, dx

        DEBUGF 1,"ok!\n"

        ret

;***************************************************************************
;   Function
;      mdio_sync
;   Description
;      initial synchronization
;   Parameters
;
;   Return value
;   Destroyed registers
;      ax, edx, cl
;
;***************************************************************************

align 4
mdio_sync:

        DEBUGF 1,"syncing mdio\n"

; switch to register window 4
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+4
        out     dx, ax
        cmp     [ebx + device.preamble], 0
        je      .no_preamble
; send 32 logic ones
        set_io  [ebx + device.io_addr], REG_PHYSICAL_MGMT
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
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_PHYSICAL_MGMT
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
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_PHYSICAL_MGMT
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
;      eax, ecx, edx
;
;***************************************************************************

align 4
check_tx_status:

        DEBUGF 1,"Checking TX status\n"

; clear TxStatus queue
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_TX_STATUS
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
;;     pointer to device structure in ebx  ;;
;;                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc vortex_transmit stdcall bufferptr

        pushf
        cli

        mov     esi, [bufferptr]
        DEBUGF  1,"Transmitting packet, buffer:%x, size:%u\n", [bufferptr], [esi + NET_BUFF.length]
        lea     eax, [esi + NET_BUFF.data]
        DEBUGF  1,"To: %x-%x-%x-%x-%x-%x From: %x-%x-%x-%x-%x-%x Type:%x%x\n",\
        [eax+00]:2,[eax+01]:2,[eax+02]:2,[eax+03]:2,[eax+04]:2,[eax+05]:2,\
        [eax+06]:2,[eax+07]:2,[eax+08]:2,[eax+09]:2,[eax+10]:2,[eax+11]:2,\
        [eax+13]:2,[eax+12]:2

        cmp     [esi + NET_BUFF.length], 1514
        ja      .fail
        cmp     [esi + NET_BUFF.length], 60
        jb      .fail

        call    check_tx_status

; switch to register window 7
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+7
        out     dx, ax

; check for master operation in progress
        set_io  [ebx + device.io_addr], REG_MASTER_STATUS
        in      ax, dx
        test    ah, 0x80
        jnz     .fail ; no DMA for sending

; program frame address to be sent
        set_io  [ebx + device.io_addr], REG_MASTER_ADDRESS
        mov     eax, esi
        add     eax, [eax + NET_BUFF.offset]
        invoke  GetPhysAddr
        out     dx, eax

; program frame length
        set_io  [ebx + device.io_addr], REG_MASTER_LEN
        mov     eax, [esi + NET_BUFF.length]
        out     dx, ax

; start DMA Down
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, (10100b shl 11) + 1 ; StartDMADown
        out     dx, ax
  .finish:
        popf
        xor     eax, eax
        ret

  .fail:
        DEBUGF  2,"Send failed\n"
        invoke  NetFree, [bufferptr]
        popf
        or      eax, -1
        ret

endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                         ;;
;; Transmit (boomerang)                    ;;
;;                                         ;;
;; In: buffer pointer in [esp+4]           ;;
;;     pointer to device structure in ebx  ;;
;;                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc boomerang_transmit stdcall bufferptr

        pushf
        cli

        mov     esi, [bufferptr]
        DEBUGF  1,"Transmitting packet, buffer:%x, size:%u\n", [bufferptr], [esi + NET_BUFF.length]
        lea     eax, [esi + NET_BUFF.data]
        DEBUGF  1,"To: %x-%x-%x-%x-%x-%x From: %x-%x-%x-%x-%x-%x Type:%x%x\n",\
        [eax+00]:2,[eax+01]:2,[eax+02]:2,[eax+03]:2,[eax+04]:2,[eax+05]:2,\
        [eax+06]:2,[eax+07]:2,[eax+08]:2,[eax+09]:2,[eax+10]:2,[eax+11]:2,\
        [eax+13]:2,[eax+12]:2

        cmp     [esi + NET_BUFF.length], 1514
        ja      .fail
        cmp     [esi + NET_BUFF.length], 60
        jb      .fail

        call    check_tx_status                         ; Reset TX engine if needed

; calculate descriptor address
        mov     edi, [ebx + device.curr_tx]
        DEBUGF  1,"Previous TX desc: %x\n", edi
        add     edi, sizeof.tx_desc
        lea     ecx, [ebx + device.tx_desc_buffer + (NUM_TX_DESC)*sizeof.tx_desc]
        cmp     edi, ecx
        jb      @f
        lea     edi, [ebx + device.tx_desc_buffer]      ; Wrap if needed
       @@:
        DEBUGF  1,"Using TX desc: %x\n", esi

; check DnListPtr
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_DN_LIST_PTR
        in      eax, dx
; mark if Dn_List_Ptr is cleared
        test    eax, eax
        setz    [ebx + device.dn_list_ptr_cleared]

; finish if no more free descriptor is available - FIXME!
;        cmp     eax, esi
;        jz      .finish

; update statistics
        inc     [ebx + device.packets_tx]
        mov     ecx, [esi + NET_BUFF.length]
        add     dword [ebx + device.bytes_tx], ecx
        adc     dword [ebx + device.bytes_tx + 4], 0

; program DPD
        and     [edi + tx_desc.next_ptr], 0
        mov     eax, [bufferptr]
        mov     [edi + tx_desc.realaddr], eax
        add     eax, [eax + NET_BUFF.offset]
        invoke  GetPhysAddr
        mov     [edi + tx_desc.frag_addr], eax
;;;        mov     ecx, [buffersize]
        or      ecx, 0x80000000         ; last fragment flag
        mov     [edi + tx_desc.frag_len], ecx

;;;        mov     ecx, [buffersize]       ; packet size
        or      ecx, 0x80008000         ; set OWN bit + transmission complete notification flag
;        test    byte [ebx + device.has_hwcksm], 0xff
;        jz      @f
;        or      ecx, (1 shl 26)         ; set AddTcpChecksum
;@@:
        mov     [edi + tx_desc.frame_start_hdr], ecx
        DEBUGF  1,"TX desc: lin=%x phys=%x len=%x start hdr=%x\n", [edi+tx_desc.realaddr]:8, [edi+tx_desc.frag_addr]:8, [edi+tx_desc.frag_len]:8, [edi+tx_desc.frame_start_hdr]:8

; calculate physical address of tx descriptor
        mov     eax, edi
        invoke  GetPhysAddr
        cmp     [ebx + device.dn_list_ptr_cleared], 0
        je      .add_to_list

; write Dn_List_Ptr
        DEBUGF  1,"TX desc phys addr=%x\n", eax
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_DN_LIST_PTR
        out     dx, eax
        jmp     .finish

  .add_to_list:
        DEBUGF  1,"Adding To list\n"
        push    eax
; DnStall
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
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
        mov     ecx, [ebx + device.curr_tx]
        mov     [ecx + tx_desc.next_ptr], eax

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_DN_LIST_PTR
        in      eax, dx
        test    eax, eax
        pop     eax
        jnz     .dnunstall

; if Dn_List_Ptr has been cleared fill it up
        DEBUGF  1,"DnList Ptr has been cleared\n"
        out     dx, eax

  .dnunstall:
; DnUnStall
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, ((110b shl 11)+3)
        out     dx, ax

  .finish:
        mov     [ebx + device.curr_tx], edi
        popf
        xor     eax, eax
        ret

  .fail:
        DEBUGF  2,"Send failed\n"
        invoke  NetFree, [bufferptr]
        popf
        or      eax, -1
        ret

endp



;---------------------------------
; Write MAC

align 4
write_mac:

        DEBUGF 1,"Writing mac\n"

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND

; switch to register window 2
        mov     ax, SELECT_REGISTER_WINDOW+2
        out     dx, ax

; write MAC addres back into the station address registers
        set_io  [ebx + device.io_addr], REG_STATION_ADDRESS_LO
        lea     esi, [ebx + device.mac]
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

        DEBUGF 1,"Reading MAC\n"

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND

; switch to register window 2
        mov     ax, SELECT_REGISTER_WINDOW+2
        out     dx, ax

; Read the MAC and write it to device.mac
        set_io  [ebx + device.io_addr], REG_STATION_ADDRESS_LO
        lea     edi, [ebx + device.mac]
        insw
        inc     dx
        inc     dx
        insw
        inc     dx
        inc     dx
        insw

        DEBUGF 1,"%x-%x-%x-%x-%x-%x\n", \
        [ebx + device.mac+0]:2,[ebx + device.mac+1]:2,[ebx + device.mac+2]:2,\
        [ebx + device.mac+3]:2,[ebx + device.mac+4]:2,[ebx + device.mac+5]:2

        ret


;------------------------------------
; Read MAC from eeprom

align 4
read_mac_eeprom:

        DEBUGF 1,"Reading MAC from eeprom\n"

; read MAC from eeprom and write it to device.mac
        mov     ecx, 3
  .mac_loop:
        lea     ax, [EEPROM_REG_OEM_NODE_ADDR+ecx-1]
        push    ecx
        call    read_eeprom
        pop     ecx
        xchg    ah, al
        mov     word [ebx + device.mac+ecx*2-2], ax
        loop    .mac_loop

        DEBUGF 1,"%x-%x-%x-%x-%x-%x\n",\
        [ebx + device.mac+0]:2,[ebx + device.mac+1]:2,[ebx + device.mac+2]:2,\
        [ebx + device.mac+3]:2,[ebx + device.mac+4]:2,[ebx + device.mac+5]:2

        ret





;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                          ;;
;; Vortex Interrupt handler ;;
;;                          ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
int_vortex:

        push    ebx esi edi

        DEBUGF  1,"INT\n"

; find pointer of device wich made IRQ occur

        mov     ecx, [vortex_devices]
        test    ecx, ecx
        jz      .nothing
        mov     esi, vortex_list
  .nextdevice:
        mov     ebx, [esi]


        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_INT_STATUS
        in      ax, dx
        and     ax, S_5_INTS
        jnz     .nothing

        add     esi, 4

        test    ax , ax
        jnz     .got_it
        loop    .nextdevice

  .nothing:
        pop     edi esi ebx
        xor     eax, eax

        ret

.got_it:

        DEBUGF  1,"Device: %x Status: %x\n", ebx, eax:4

        test    ax, RxComplete
        jz      .noRX

        set_io  [ebx + device.io_addr], 0
  .rx_status_loop:
; examine RxStatus
        set_io  [ebx + device.io_addr], REG_RX_STATUS
        in      ax, dx
        test    ax, ax
        jz      .finish

        test    ah, 0x80 ; rxIncomplete
        jnz     .finish

        test    ah, 0x40
        jz      .check_length

; discard the top frame received advancing the next one
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, (01000b shl 11)
        out     dx, ax
        jmp     .rx_status_loop

  .check_length:
        and     eax, 0x1fff
        cmp     eax, MAX_ETH_FRAME_SIZE
        ja      .discard_frame ; frame is too long discard it

  .check_dma:
        mov     ecx, eax
; switch to register window 7
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, SELECT_REGISTER_WINDOW+7
        out     dx, ax
; check for master operation in progress
        set_io  [ebx + device.io_addr], REG_MASTER_STATUS
        in      ax, dx

        test    ah, 0x80
        jnz     .finish

  .read_frame:
; program buffer address to read in
        push    ecx
        invoke  NetAlloc, MAX_ETH_FRAME_SIZE + NET_BUFF.data
        pop     ecx
        test    eax, eax
        jz      .finish

        push    .discard_frame
        push    eax
        mov     [eax + NET_BUFF.length], ecx
        mov     [eax + NET_BUFF.device], ebx
        mov     [eax + NET_BUFF.offset], NET_BUFF.data
        invoke  GetPhysAddr
        add     eax, NET_BUFF.data
        set_io  [ebx + device.io_addr], REG_MASTER_ADDRESS
        out     dx, eax

; program frame length
        set_io  [ebx + device.io_addr], REG_MASTER_LEN
        mov     ax, 1560
        out     dx, ax

; start DMA Up
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, (10100b shl 11) ; StartDMAUp
        out     dx, ax

; check for master operation in progress
        set_io  [ebx + device.io_addr], REG_MASTER_STATUS   ; TODO: use timeout and reset after timeout expired
  .dma_loop:
        in      ax, dx
        test    ah, 0x80
        jnz     .dma_loop

; registrate the received packet to kernel
        jmp     [EthInput]

; discard the top frame received
  .discard_frame:
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, (01000b shl 11)
        out     dx, ax

  .finish:


  .noRX:

        test    ax, DMADone
        jz      .noDMA

        push    ax

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], 12
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
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, AckIntr + IntReq + IntLatch
        out     dx, ax

        pop     edi esi ebx

        ret




;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                             ;;
;; Boomerang Interrupt handler ;;
;;                             ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
int_boomerang:

        push    ebx esi edi

        DEBUGF  1,"INT\n"

; find pointer of device wich made IRQ occur

        mov     ecx, [boomerang_devices]
        test    ecx, ecx
        jz      .nothing
        mov     esi, boomerang_list
  .nextdevice:
        mov     ebx, [esi]

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_INT_STATUS
        in      ax, dx
        test    ax, S_5_INTS
        jnz     .got_it
  .continue:
        add     esi, 4
        dec     ecx
        jnz     .nextdevice
  .nothing:
        pop     edi esi ebx
        xor     eax, eax

        ret

  .got_it:

        DEBUGF  1,"Device: %x Status: %x\n", ebx, ax
        push    ax

; disable all INTS

        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, SetIntrEnb
        out     dx, ax

;--------------------------------------------------------------------------
        test    word[esp], UpComplete
        jz      .noRX

        push    ebx

  .receive:
        DEBUGF  1,"UpComplete\n"

; check if packet is uploaded
        mov     esi, [ebx + device.curr_rx]
        test    byte [esi+rx_desc.pkt_status+1], 0x80 ; upPktComplete
        jz      .finish
        DEBUGF  1, "Current RX desc: %x\n", esi
; packet is uploaded check for any error
  .check_error:
        test    byte [esi + rx_desc.pkt_status+1], 0x40 ; upError
        jz      .copy_packet_length
        DEBUGF  1,"Error in packet\n"
        and     [esi + rx_desc.pkt_status], 0           ; mark packet as read
        jmp     .finish
  .copy_packet_length:
        mov     ecx, [esi + rx_desc.pkt_status]
        and     ecx, 0x1fff

;        cmp     ecx, MAX_ETH_PKT_SIZE
;        jbe     .copy_packet
;        and     [esi+rx_desc.pkt_status], 0
;        jmp     .finish
;  .copy_packet:

        DEBUGF  1, "Received %u bytes in buffer %x\n", ecx, [esi + rx_desc.realaddr]:8

        push    dword .loop ;.finish
        mov     eax, [esi + rx_desc.realaddr]
        push    eax
        mov     [eax + NET_BUFF.length], ecx
        mov     [eax + NET_BUFF.device], ebx
        mov     [eax + NET_BUFF.offset], NET_BUFF.data

; update statistics
        inc     [ebx + device.packets_rx]
        add     dword [ebx + device.bytes_rx], ecx
        adc     dword [ebx + device.bytes_rx + 4], 0

; update rx descriptor (Alloc new buffer for next packet)
        invoke  NetAlloc, MAX_ETH_FRAME_SIZE + NET_BUFF.data
        mov     [esi + rx_desc.realaddr], eax
        invoke  GetPhysAddr
        add     eax, NET_BUFF.data
        mov     [esi + rx_desc.frag_addr], eax
        and     [esi + rx_desc.pkt_status], 0
        mov     [esi + rx_desc.frag_len], MAX_ETH_FRAME_SIZE or (1 shl 31)

; Update rx descriptor pointer
        add     esi, sizeof.rx_desc
        lea     ecx, [ebx + device.rx_desc_buffer+(NUM_RX_DESC)*sizeof.rx_desc]
        cmp     esi, ecx
        jb      @f
        lea     esi, [ebx + device.rx_desc_buffer]
       @@:
        mov     [ebx + device.curr_rx], esi
        DEBUGF  1, "Next RX desc: %x\n", esi

        jmp     [EthInput]
  .loop:

        mov     ebx, [esp]
        jmp     .receive

  .finish:
        pop     ebx

; check if the NIC is in the upStall state
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_UP_PKT_STATUS
        in      eax, dx
        test    ah, 0x20             ; UpStalled
        jz      .noUpUnStall

        DEBUGF  1, "upUnStalling\n"
; issue upUnStall command
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, ((11b shl 12)+1) ; upUnStall
        out     dx, ax

        ;;;; FIXME: make upunstall work

  .noUpUnStall:
.noRX:
        test    word[esp], DownComplete
        jz      .noTX
        DEBUGF  1, "Downcomplete!\n"

        mov     ecx, NUM_TX_DESC
        lea     esi, [ebx + device.tx_desc_buffer]
  .txloop:
        test    [esi+tx_desc.frame_start_hdr], 1 shl 31
        jz      .maybenext

        and     [esi+tx_desc.frame_start_hdr], 0
        push    ecx
        invoke  NetFree, [esi+tx_desc.realaddr]
        pop     ecx

  .maybenext:
        add     esi, sizeof.tx_desc
        dec     ecx
        jnz     .txloop

.noTX:
        pop     ax

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
        or      ax, AckIntr
        out     dx, ax

        set_io  [ebx + device.io_addr], REG_INT_STATUS
        in      ax, dx
        test    ax, S_5_INTS
        jnz     .got_it

;re-enable ints
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     ax, SetIntrEnb + S_5_INTS
        out     dx, ax

        pop     edi esi ebx

        ret




; End of code

data fixups
end data

include '../peimport.inc'

my_service           db '3C59X',0                    ; max 16 chars include zero

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

align 4
vortex_devices          dd 0
boomerang_devices       dd 0
vortex_list             rd MAX_DEVICES
boomerang_list          rd MAX_DEVICES




