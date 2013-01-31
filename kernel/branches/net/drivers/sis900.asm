;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                    ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved.       ;;
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

format MS COFF

        NUM_RX_DESC             =   4           ; Number of RX descriptors
        NUM_TX_DESC             =   4           ; Number of TX descriptors
        RX_BUFF_SZ              =   1520        ; Buffer size for each Rx buffer
        TX_BUFF_SZ              =   1516        ; Buffer size for each Tx buffer
        MAX_ETH_FRAME_SIZE      =   1516

        API_VERSION             =   0x01000100
        DRIVER_VERSION          =   5

        MAX_DEVICES             =   16

        DEBUG                   =   1
        __DEBUG__               =   1
        __DEBUG_LEVEL__         =   1

        DSIZE                   =   0x00000fff
        CRC_SIZE                =   4
        RFADDR_shift            =   16

; If you are having problems sending/receiving packet try changing the
; Max DMA Burst, Possible settings are as follows:
;
; 0x00000000 = 512 bytes
; 0x00100000 = 4 bytes
; 0x00200000 = 8 bytes
; 0x00300000 = 16 bytes
; 0x00400000 = 32 bytes
; 0x00500000 = 64 bytes
; 0x00600000 = 128 bytes
; 0x00700000 = 256 bytes

        RX_DMA                  = 0x00600000
        TX_DMA                  = 0x00600000

;-------------------------------------------------------------------------------------------------
; Symbolic offsets to registers.
        cr              =   0x0               ; Command Register
        cfg             =   0x4       ; Configuration Register
        mear            =   0x8       ; EEPROM Access Register
        ptscr           =   0xc       ; PCI Test Control Register
        isr             =   0x10      ; Interrupt Status Register
        imr             =   0x14      ; Interrupt Mask Register
        ier             =   0x18      ; Interrupt Enable Register
        epar            =   0x18      ; Enhanced PHY Access Register
        txdp            =   0x20      ; Transmit Descriptor Pointer Register
        txcfg           =   0x24      ; Transmit Configuration Register
        rxdp            =   0x30      ; Receive Descriptor Pointer Register
        rxcfg           =   0x34      ; Receive Configuration Register
        flctrl          =   0x38      ; Flow Control Register
        rxlen           =   0x3c      ; Receive Packet Length Register
        rfcr            =   0x48      ; Receive Filter Control Register
        rfdr            =   0x4C      ; Receive Filter Data Register
        pmctrl          =   0xB0      ; Power Management Control Register
        pmer            =   0xB4      ; Power Management Wake-up Event Register

; Command Register Bits
        RELOAD          =   0x00000400
        ACCESSMODE      =   0x00000200
        RESET           =   0x00000100
        SWI             =   0x00000080
        RxRESET         =   0x00000020
        TxRESET         =   0x00000010
        RxDIS           =   0x00000008
        RxENA           =   0x00000004
        TxDIS           =   0x00000002
        TxENA           =   0x00000001

; Configuration Register Bits
        DESCRFMT        =   0x00000100 ; 7016 specific
        REQALG          =   0x00000080
        SB              =   0x00000040
        POW             =   0x00000020
        EXD             =   0x00000010
        PESEL           =   0x00000008
        LPM             =   0x00000004
        BEM             =   0x00000001
        RND_CNT         =   0x00000400
        FAIR_BACKOFF    =   0x00000200
        EDB_MASTER_EN   =   0x00002000

; Eeprom Access Reigster Bits
        MDC             =   0x00000040
        MDDIR           =   0x00000020
        MDIO            =   0x00000010   ; 7016 specific
        EECS            =   0x00000008
        EECLK           =   0x00000004
        EEDO            =   0x00000002
        EEDI            =   0x00000001

; TX Configuration Register Bits
        ATP             =   0x10000000  ; Automatic Transmit Padding
        MLB             =   0x20000000  ; Mac Loopback Enable
        HBI             =   0x40000000  ; HeartBeat Ignore (Req for full-dup)
        CSI             =   0x80000000  ; CarrierSenseIgnore (Req for full-du

; RX Configuration Register Bits
        AJAB            =   0x08000000  ;
        ATX             =   0x10000000  ; Accept Transmit Packets
        ARP             =   0x40000000  ; accept runt packets (<64bytes)
        AEP             =   0x80000000  ; accept error packets

; Interrupt Register Bits
        WKEVT           =   0x10000000
        TxPAUSEEND      =   0x08000000
        TxPAUSE         =   0x04000000
        TxRCMP          =   0x02000000  ; Transmit Reset Complete
        RxRCMP          =   0x01000000  ; Receive Reset Complete
        DPERR           =   0x00800000
        SSERR           =   0x00400000
        RMABT           =   0x00200000
        RTABT           =   0x00100000
        RxSOVR          =   0x00010000
        HIBERR          =   0x00008000
        SWINT           =   0x00001000
        MIBINT          =   0x00000800
        TxURN           =   0x00000400
        TxIDLE          =   0x00000200
        TxERR           =   0x00000100
        TxDESC          =   0x00000080
        TxOK            =   0x00000040
        RxORN           =   0x00000020
        RxIDLE          =   0x00000010
        RxEARLY         =   0x00000008
        RxERR           =   0x00000004
        RxDESC          =   0x00000002
        RxOK            =   0x00000001

; Interrupt Enable Register Bits
        IE              =   RxOK + TxOK

; Revision ID
        SIS900B_900_REV         =   0x03
        SIS630A_900_REV         =   0x80
        SIS630E_900_REV         =   0x81
        SIS630S_900_REV         =   0x82
        SIS630EA1_900_REV       =   0x83
        SIS630ET_900_REV        =   0x84
        SIS635A_900_REV         =   0x90
        SIS900_960_REV          =   0x91

; Receive Filter Control Register Bits
        RFEN            =   0x80000000          ; enable
        RFAAB           =   0x40000000          ; accept all broadcasts
        RFAAM           =   0x20000000          ; accept all multicasts
        RFAAP           =   0x10000000          ; accept all packets

; Reveive Filter Data Mask
        RFDAT           =   0x0000FFFF

; Eeprom Address
        EEPROMSignature =   0x00
        EEPROMVendorID  =   0x02
        EEPROMDeviceID  =   0x03
        EEPROMMACAddr   =   0x08
        EEPROMChecksum  =   0x0b

; The EEPROM commands include the alway-set leading bit.
        EEread          =   0x0180
        EEwrite         =   0x0140
        EEerase         =   0x01C0
        EEwriteEnable   =   0x0130
        EEwriteDisable  =   0x0100
        EEeraseAll      =   0x0120
        EEwriteAll      =   0x0110
        EEaddrMask      =   0x013F
        EEcmdShift      =   16

; For SiS962 or SiS963, request the eeprom software access
        EEREQ           =   0x00000400
        EEDONE          =   0x00000200
        EEGNT           =   0x00000100


include 'proc32.inc'
include 'imports.inc'
include 'fdo.inc'
include 'netdrv.inc'

public START
public version


virtual at ebx
        device:

        ETH_DEVICE

        .io_addr        dd ?
        .pci_bus        dd ?
        .pci_dev        dd ?
        .irq_line       db ?
        .cur_rx         db ?
        .cur_tx         db ?
        .last_tx        db ?
        .pci_revision   db ?
        .table_entries  db ?
        rb 2    ; alignment

        .txd            rd (4 * NUM_TX_DESC)
        .rxd            rd (4 * NUM_RX_DESC)

        .size = $ - device

end virtual

macro   ee_delay {
        push    eax
        in      eax, dx
        in      eax, dx
        in      eax, dx
        in      eax, dx
        in      eax, dx
        in      eax, dx
        in      eax, dx
        in      eax, dx
        in      eax, dx
        in      eax, dx
        pop     eax
}


section '.flat' code readable align 16

; Driver entry point - register our service when the driver is loading.
; TODO: add needed operations when unloading
START:
        cmp     dword [esp+4], 1
        jne     .exit
        stdcall RegService, my_service, service_proc
        ret     4
  .exit:
        xor     eax, eax
        ret     4

; Service procedure for the driver - handle all I/O requests for the driver.
; Currently handled requests are: SRV_GETVERSION = 0 and SRV_HOOK = 1.
service_proc:
; 1. Get parameter from the stack: [esp+4] is the first parameter,
;       pointer to IOCTL structure.
        mov     edx, [esp+4]    ; edx -> IOCTL
; 2. Get request code and select a handler for the code.
        mov     eax, [IOCTL.io_code]
        test    eax, eax        ; check for SRV_GETVERSION
        jnz     @f
; 3. This is SRV_GETVERSION request, no input, 4 bytes output, API_VERSION.
; 3a. Output size must be at least 4 bytes.
        cmp     [IOCTL.out_size], 4
        jb      .fail
; 3b. Write result to the output buffer.
        mov     eax, [IOCTL.output]
        mov     [eax], dword API_VERSION
; 3c. Return success.
        xor     eax, eax
        ret     4
  @@:
        dec     eax     ; check for SRV_HOOK
        jnz     .fail
; 4. This is SRV_HOOK request, input defines the device to hook, no output.
; 4a. The driver works only with PCI devices,
;       so input must be at least 3 bytes long.
        cmp     [IOCTL.inp_size], 3
        jb      .fail
; 4b. First byte of input is bus type, 1 stands for PCI.
        mov     eax, [IOCTL.input]
        cmp     byte [eax], 1
        jne     .fail
; 4c. Second and third bytes of the input define the device: bus and dev.
;       Word in bx holds both bytes.
        mov     bx, [eax+1]
; 4d. Check if the device was already hooked,
;       scan through the list of known devices.
; check if the device is already listed
        mov     esi, device_list
        mov     ecx, [devices]
        test    ecx, ecx
        jz      .firstdevice

;        mov     eax, [IOCTL.input]                      ; get the pci bus and device numbers
        mov     ax, [eax+1]                            ;
  .nextdevice:
        mov     ebx, [esi]
        cmp     al, byte[device.pci_bus]
        jne     @f
        cmp     ah, byte[device.pci_dev]
        je      .find_devicenum                         ; Device is already loaded, let's find it's device number
       @@:
        add     esi, 4
        loop    .nextdevice
; 4e. This device doesn't have its own eth_device structure yet, let's create one
  .firstdevice:
; 4f. Check that we have place for new device.
        cmp     [devices], MAX_DEVICES
        jae     .fail
; 4g. Allocate memory for device descriptor and receive+transmit buffers.
; 4h. Zero the structure.
        allocate_and_clear ebx, device.size, .fail
; 4i. Save PCI coordinates
        mov     eax, [IOCTL.input]
        movzx   ecx, byte[eax+1]
        mov     [device.pci_bus], ecx
        movzx   ecx, byte[eax+2]
        mov     [device.pci_dev], ecx
; 4j. Fill in the direct call addresses into the struct.
; Note that get_MAC pointer is filled in initialization by probe.
        mov     [device.reset], reset
        mov     [device.transmit], transmit
        mov     [device.set_MAC], write_mac
        mov     [device.unload], unload
        mov     [device.name], my_service

; 4k. Now, it's time to find the base io addres of the PCI device
; TODO: implement check if bus and dev exist on this machine

; Now, it's time to find the base io addres of the PCI device
        PCI_find_io

; We've found the io address, find IRQ now
        PCI_find_irq

; 4m. Add new device to the list (required for int_handler).
        mov     eax, [devices]
        mov     [device_list+4*eax], ebx
        inc     [devices]

; 4m. Ok, the eth_device structure is ready, let's probe the device
        call    probe
        test    eax, eax
        jnz     .destroy
; 4n. If device was successfully initialized, register it for the kernel.

        mov     [device.type], NET_TYPE_ETH
        call    NetRegDev

        cmp     eax, -1
        je      .destroy

        ret     4

; 5. If the device was already loaded, find the device number and return it in eax

  .find_devicenum:
        call    NetPtrToNum                                             ; This kernel procedure converts a pointer to device struct in ebx
                                                                        ; into a device number in edi
        mov     eax, edi                                                ; Application wants it in eax instead
        ret     4

; If an error occured, remove all allocated data and exit (returning -1 in eax)

  .destroy:
        dec     [devices]
        ; todo: reset device into virgin state

  .err:
        stdcall KernelFree, ebx

  .fail:
        xor     eax, eax
        ret     4


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

        or      eax,-1

ret

;***************************************************************************
;
; probe
;
; checks the card and enables it
;
; TODO: probe mii transceivers
;
;***************************************************************************
align 4
probe:
        DEBUGF  1, "Probe\n"

; wake up device   CHECKME
        stdcall PciWrite8, [device.pci_bus], [device.pci_dev], 0x40, 0

        PCI_make_bus_master

        PCI_adjust_latency 64

; Get Card Revision
        stdcall PciRead8, [device.pci_bus], [device.pci_dev], 0x08
        mov     [device.pci_revision], al       ; save the revision for later use

; Look up through the specific_table
        mov     esi, specific_table
  .tableloop:
        cmp     dword [esi], 0                  ; Check if we reached end of the list
        je      .notsupported
        cmp     al, [esi]                       ; Check if revision is OK
        je      .ok
        add     esi, 12                         ; Advance to next entry
        jmp     .tableloop

  .ok:
        mov     eax, [esi + 4]                  ; Get pointer to "get MAC" function
        mov     [device.get_MAC], eax

        call    [device.get_MAC]

; Set table entries
        mov      [device.table_entries], 16
        cmp      [device.pci_revision], SIS635A_900_REV
        jae      @f
        cmp      [device.pci_revision], SIS900B_900_REV
        je       @f
        mov      [device.table_entries], 8
       @@:

; TODO: Probe for mii transceiver

        jmp     reset

  .notsupported:
        DEBUGF  1, "Device not supported\n"
        or      eax, -1

        ret

reset:

        DEBUGF  1, "reset\n"

        movzx   eax, [device.irq_line]
        stdcall AttachIntHandler, eax, int_handler, 0

;--------------------------------------------
; Disable Interrupts and reset Receive Filter

        set_io  0
        set_io  ier
        xor     eax, eax
        out     dx, eax

        set_io  imr
        out     dx, eax

        set_io  rfcr
        out     dx, eax

;-----------
; Reset Card

        set_io  cr
        in      eax, dx                         ; Get current Command Register
        or      eax, RESET + RxRESET + TxRESET  ; set flags
        out     dx, eax                         ; Write new Command Register

;----------
; Wait loop

        set_io  isr
        mov     ecx, 1000
  .loop:
        dec     ecx
        jz      .fail
        in      eax, dx                         ; read interrupt status
        test    eax, TxRCMP + RxRCMP
        jz      .loop
        DEBUGF  1, "status=%x\n", eax

;------------------------------------------------------
; Set Configuration Register depending on Card Revision

        set_io  cfg
        mov     eax, PESEL                      ; Configuration Register Bit
        cmp     [device.pci_revision], SIS635A_900_REV
        je      .match
        cmp     [device.pci_revision], SIS900B_900_REV ; Check card revision
        jne     .done
  .match:                                       ; Revision match
        or      eax, RND_CNT                    ; Configuration Register Bit
  .done:
        out     dx, eax

        DEBUGF  1, "Initialising RX Filter\n"

; Get Receive Filter Control Register
        set_io  rfcr
        in      eax, dx
        push    eax

; disable packet filtering before setting filter
        and     eax, not RFEN
        out     dx, eax

; load MAC addr to filter data register
        xor     ecx, ecx
  .macloop:
        mov     eax, ecx
        set_io  0
        set_io  rfcr
        shl     eax, 16                                             ; high word of eax tells card which mac byte to write
        out     dx, eax                                             ;
        set_io  rfdr
        mov     ax, word [device.mac + ecx*2]                       ; Get Mac ID word
        out     dx, ax                                              ; Send Mac ID
        inc     cl                                                  ; send next word
        cmp     cl, 3                                               ; more to send?
        jne     .macloop

; enable packet filtering
        pop     eax                     ; old register value
        set_io  rfcr
        or      eax, RFEN               ; enable filtering
        out     dx, eax                 ; set register

        DEBUGF  1, "Initialising TX Descriptors\n"

        mov     ecx, NUM_TX_DESC
        lea     esi, [device.txd]
  .txdescloop:
        lea     eax, [esi + 16]                 ; next ptr
        GetRealAddr
        mov     dword [esi], eax                ; link to next desc
        mov     dword [esi + 4], 0              ; status field
        mov     dword [esi + 8], 0              ; ptr to buffer
        add     esi, 16
        dec     ecx
        jnz     .txdescloop

        lea     eax, [device.txd]
        GetRealAddr
        mov     dword [esi - 16], eax           ; correct last descriptor link ptr

        set_io  txdp                            ; TX Descriptor Pointer
;        lea     eax, [device.txd]
;        GetRealAddr
        out     dx, eax

        mov     [device.cur_tx], 0              ; Set current tx descriptor to 0
        mov     [device.last_tx], 0

        DEBUGF  1, "Initialising RX Descriptors\n"

        mov     ecx, NUM_RX_DESC
        lea     esi, [device.rxd]
  .rxdescloop:
        lea     eax, [esi + 16]                 ; next ptr
        GetRealAddr
        mov     dword [esi], eax
        mov     dword [esi + 4], RX_BUFF_SZ     ; size

        push    ecx esi
        stdcall KernelAlloc, RX_BUFF_SZ
        pop     esi ecx
        test    eax, eax
        jz      .fail
        mov     dword [esi + 12], eax           ; address
        GetRealAddr
        mov     dword [esi + 8], eax            ; real address
        add     esi, 16
        dec     ecx
        jnz     .rxdescloop

        lea     eax, [device.rxd]
        GetRealAddr
        mov     dword [esi - 16], eax           ; correct last descriptor link ptr

        set_io  0
        set_io  rxdp
;        lea     eax, [device.rxd]
;        GetRealAddr
        out     dx, eax

        mov     [device.cur_rx], 0              ; Set current rx descriptor to 0

        DEBUGF  1, "setting RX mode\n"

        xor     cl, cl
  .rxfilterloop:
        set_io  0
        set_io  rfcr                    ; Receive Filter Control Reg offset
        mov     eax, 4                  ; determine table entry
        add     al, cl
        shl     eax, 16
        out     dx, eax                 ; tell card which entry to modify

        set_io  rfdr                    ; Receive Filter Control Reg offset
        mov     eax, 0xffff             ; entry value
        out     dx, ax                  ; write value to table in card

        inc     cl                      ; next entry
        cmp     cl, [device.table_entries]
        jb      .rxfilterloop

        set_io  rfcr                    ; Receive Filter Control Register offset
        mov     eax, RFAAB + RFAAM + RFAAP + RFEN
        out     dx, eax

        set_io  rxcfg                   ; Receive Config Register offset
        mov     eax, ATX + RX_DMA + 2   ; 0x2 : RX Drain Threshold = 8*8=64 bytes
        out     dx, eax

        DEBUGF  1, "setting TX mode\n"

        set_io  txcfg                   ; Transmit config Register offset
        mov     eax, ATP + HBI + CSI + TX_DMA + 0x120
                                        ; TX Fill threshold = 0x100
                                        ; TX Drain Threshold = 0x20
        out     dx, eax

        DEBUGF  1, "Enabling interrupts\n"

        set_io  imr
        mov     eax, IE                 ; Interrupt enable mask
        out     dx, eax

        set_io  cr
        in      eax, dx
        or      eax, RxENA              ; Enable Receive
        out     dx, eax

        set_io  ier                     ; Interrupt enable
        mov     eax, 1
        out     dx, eax

        mov     [device.mtu], 1514
        xor     eax, eax

        ret

  .fail:
        DEBUGF  1, "Resetting device failed\n"
        or      eax, -1

        ret


;***************************************************************************
;
; SIS960_get_mac_addr: - Get MAC address for SiS962 or SiS963 model
;
; SiS962 or SiS963 model, use EEPROM to store MAC address.
; EEPROM is shared by LAN and 1394.
; When access EEPROM, send EEREQ signal to hardware first, and wait for EEGNT.
; If EEGNT is ON, EEPROM is permitted to be accessed by LAN, otherwise is not.
; After MAC address is read from EEPROM, send
; EEDONE signal to refuse EEPROM access by LAN.
; The EEPROM map of SiS962 or SiS963 is different to SiS900.
; The signature field in SiS962 or SiS963 spec is meaningless.
;
; Return 0 is EAX = failure
;
;***************************************************************************
align 4
SIS960_get_mac_addr:
        DEBUGF  1, "SIS960 - get mac: "

;-------------------------------
; Send Request for eeprom access

        set_io  0
        set_io  mear            ; Eeprom access register
        mov     eax, EEREQ      ; Request access to eeprom
        out     dx, eax         ; Send request

;-----------------------------------------------------
; Loop 4000 times and if access not granted, error out

        mov     ecx, 4000
  .loop:
        in      eax, dx         ; get eeprom status
        test    eax, EEGNT      ; see if eeprom access granted flag is set
        jnz     .got_access     ; if it is, go access the eeprom
        loop    .loop           ; else keep waiting

        DEBUGF  1, "Access to EEprom failed!\n", 0

        set_io  mear            ; Eeprom access register
        mov     eax, EEDONE     ; tell eeprom we are done
        out     dx, eax

        or      eax, -1         ; error
        ret

  .got_access:

;------------------------------------------
; EEprom access granted, read MAC from card

    ; zero based so 3-16 bit reads will take place

        mov     ecx, 2
  .read_loop:
        mov     eax, EEPROMMACAddr      ; Base Mac Address
        add     eax, ecx                ; Current Mac Byte Offset
        push    ecx
        call    read_eeprom             ; try to read 16 bits
        pop     ecx
        mov     word [device.mac+ecx*2], ax     ; save 16 bits to the MAC ID varible
        dec     ecx                     ; one less word to read
        jns     .read_loop              ; if more read more
        mov     eax, 1                  ; return non-zero indicating success

        DEBUGF  2,"%x-%x-%x-%x-%x-%x\n",[device.mac]:2,[device.mac+1]:2,[device.mac+2]:2,[device.mac+3]:2,[device.mac+4]:2,[device.mac+5]:2

;-------------------------------------
; Tell EEPROM We are Done Accessing It

  .done:
        set_io  0
        set_io  mear            ; Eeprom access register
        mov     eax, EEDONE     ; tell eeprom we are done
        out     dx, eax

        xor     eax, eax        ; ok
        ret




;***************************************************************************
;
; get_mac_addr: - Get MAC address for stand alone SiS900 model
;
; Older SiS900 and friends, use EEPROM to store MAC address.
;
;***************************************************************************
align 4
SIS900_get_mac_addr:
        DEBUGF  1, "SIS900 - get mac: "

;------------------------------------
; check to see if we have sane EEPROM

        mov     eax, EEPROMSignature  ; Base Eeprom Signature
        call    read_eeprom           ; try to read 16 bits
        cmp     ax, 0xffff
        je      .err
        test    ax, ax
        je      .err

;-----------
; Read MacID

; zero based so 3-16 bit reads will take place

        mov     ecx, 2
  .loop:
        mov     eax, EEPROMMACAddr      ; Base Mac Address
        add     eax, ecx                ; Current Mac Byte Offset
        push    ecx
        call    read_eeprom             ; try to read 16 bits
        pop     ecx
        mov     word [device.mac+ecx*2], ax     ; save 16 bits to the MAC ID storage
        dec     ecx                             ; one less word to read
        jns     .loop                           ; if more read more

        DEBUGF  2,"%x-%x-%x-%x-%x-%x\n",[device.mac]:2,[device.mac+1]:2,[device.mac+2]:2,[device.mac+3]:2,[device.mac+4]:2,[device.mac+5]:2

        xor     eax, eax
        ret


  .err:
        DEBUGF  1, "Access to EEprom failed!\n", 0

        or      eax, -1
        ret


;***************************************************************************
;
; Get_Mac_SIS635_900_REV: - Get MAC address for model 635
;
;***************************************************************************
align 4
Get_Mac_SIS635_900_REV:
        DEBUGF  1, "SIS635 - get mac: "

        set_io  0
        set_io  rfcr
        in      eax, dx
        mov     esi, eax

        set_io  cr
        or      eax, RELOAD
        out     dx, eax

        xor     eax, eax
        out     dx, eax

;-----------------------------------------------
; Disable packet filtering before setting filter

        set_io  rfcr
        mov     eax, esi
        and     eax, not RFEN
        out     dx, eax

;---------------------------------
; Load MAC to filter data register

        xor     ecx, ecx
        lea     edi, [device.mac]
  .loop:
        set_io  0
        set_io  rfcr
        mov     eax, ecx
        shl     eax, RFADDR_shift
        out     dx, eax

        set_io  rfdr
        in      ax, dx
        stosw
        inc     ecx
        cmp     ecx, 3
        jb      .loop

;------------------------
; Enable packet filtering

        set_io  rfcr
        mov     eax, esi
        or      eax, RFEN
        out     dx, eax

        DEBUGF  2,"%x-%x-%x-%x-%x-%x\n",[device.mac]:2,[device.mac+1]:2,[device.mac+2]:2,[device.mac+3]:2,[device.mac+4]:2,[device.mac+5]:2

        xor     eax, eax
        ret

;***************************************************************************
;
; read_eeprom
;
; reads and returns a given location from EEPROM
;
; IN:  si = addr
; OUT: ax = data
;
;***************************************************************************
align 4
read_eeprom:

        set_io  0
        set_io  mear

        xor     eax, eax              ; start send
        out     dx, eax
        ee_delay

        or      eax, EECLK
        out     dx, eax
        ee_delay

;------------------------------------
; Send the read command

        or      esi, EEread
        mov     ecx, 1 shl 9

  .loop:
        mov     eax, EECS
        test    esi, ecx
        jz      @f
        or      eax, EEDI
       @@:
        out     dx, eax
        ee_delay

        or      eax, EECLK
        out     dx, eax
        ee_delay

        shr     esi, 1
        jnc     .loop

        mov     eax, EECS
        out     dx, eax
        ee_delay

;------------------------
; Read 16-bits of data in

        xor     esi, esi
        mov     cx, 16
  .loop2:
        mov     eax, EECS
        out     dx, eax
        ee_delay

        or      eax, EECLK
        out     dx, eax
        ee_delay

        in      eax, dx
        shl     esi, 1
        test    eax, EEDO
        jz      @f
        inc     esi
       @@:
        loop    .loop2

;----------------------------
; Terminate the EEPROM access

        xor     eax, eax
        out     dx, eax
        ee_delay

        mov     eax, EECLK
        out     dx, eax
        ee_delay

        movzx   eax, si

        ret



align 4
write_mac:
        DEBUGF  1,'Setting MAC is not supported for SIS900 card.\n'
        add     esp, 6
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
;***************************************************************************
align 4
transmit:
        DEBUGF  1,"Transmitting packet, buffer:%x, size:%u\n",[esp+4],[esp+8]
        mov     eax, [esp+4]
        DEBUGF  1,"To: %x-%x-%x-%x-%x-%x From: %x-%x-%x-%x-%x-%x Type:%x%x\n",\
        [eax+00]:2,[eax+01]:2,[eax+02]:2,[eax+03]:2,[eax+04]:2,[eax+05]:2,\
        [eax+06]:2,[eax+07]:2,[eax+08]:2,[eax+09]:2,[eax+10]:2,[eax+11]:2,\
        [eax+13]:2,[eax+12]:2

        cmp     dword [esp + 8], MAX_ETH_FRAME_SIZE
        ja      .error
        cmp     dword [esp + 8], 60
        jb      .error

        movzx   ecx, [device.cur_tx]
        shl     ecx, 4                  ; *16
        lea     ecx, [device.txd + ecx]

        test    dword [ecx + 4], 0x80000000     ; card owns descriptor ?
        jnz     .error

        mov     eax, [esp + 4]
        mov     dword [ecx + 12], eax
        GetRealAddr
        mov     dword [ecx + 8], eax    ; buffer address

        mov     eax, [esp + 8]
        and     eax, DSIZE
        or      eax, 0x80000000         ; card owns descriptor
        mov     dword [ecx + 4], eax    ; status field

        set_io  0
        set_io  cr
        in      eax, dx
        or      eax, TxENA              ; Enable the transmit state machine
        out     dx, eax

        inc     [device.cur_tx]
        and     [device.cur_tx], NUM_TX_DESC-1

; update stats
        mov     ecx, [esp + 8]
        inc     [device.packets_tx]
        add     dword [device.bytes_tx], ecx
        adc     dword [device.bytes_tx + 4], 0

  .finish:
        DEBUGF  1,"Packet sent!\n"
        xor     eax, eax
        ret     8

  .error:
        DEBUGF  1,"ERROR!\n"
        stdcall KernelFree, [esp+4]
        or      eax, -1
        ret     8


;***************************************************************************
;
; int_handler
;
; handles received IRQs, which signal received packets
;
; Currently only supports one descriptor per packet, if packet is fragmented
; between multiple descriptors you will lose part of the packet
;
;***************************************************************************

align 4
int_handler:

        DEBUGF  1,"\n%s int\n", my_service

; find pointer of device which made IRQ occur

        mov     ecx, [devices]
        test    ecx, ecx
        jz      .nothing
        mov     esi, device_list
  .nextdevice:
        mov     ebx, [esi]

        set_io  0
        set_io  isr
        in      eax, dx                 ; note that this clears all interrupts
        test    ax, IE
        jnz     .got_it
  .continue:
        add     esi, 4
        dec     ecx
        jnz     .nextdevice
  .nothing:
        ret

  .got_it:

        DEBUGF  1,"Device: %x Status: %x ", ebx, ax

        test    ax, RxOK
        jz      .no_rx_

        push    ax

  .rx_loop:

;-----------
; Get Status
        movzx   eax, [device.cur_rx]                    ; find current descriptor
        shl     eax, 4                                  ; * 16
        mov     ecx, dword[device.rxd + eax + 4]        ; get receive status

;-------------------------------------------
; Check RX_Status to see if packet is waiting
        test    ecx, 0x80000000
        jz      .no_rx

;----------------------------------------------
; There is a packet waiting check it for errors
        test    ecx, 0x67C0000                  ; see if there are any errors
        jnz     .error_status

;---------------------
; Check size of packet
        and     ecx, DSIZE                      ; get packet size minus CRC
        sub     ecx, CRC_SIZE                   ; make sure packet contains data
        jbe     .error_size

; update statistics
        inc     dword [device.packets_rx]
        add     dword [device.bytes_rx], ecx
        adc     dword [device.bytes_rx + 4], 0

        push    ebx
        push    .return
        push    ecx                             ; packet size
        pushd   [device.rxd + eax + 12]         ; packet ptr
        DEBUGF  1, "Packet received OK\n"
        jmp     Eth_input
  .return:
        pop     ebx

; Reset status, allow ethernet card access to descriptor
        stdcall KernelAlloc, RX_BUFF_SZ
        test    eax, eax
        jz      .fail
        movzx   ecx, [device.cur_rx]
        shl     ecx, 4                          ; *16
        lea     ecx, [device.rxd + ecx]
        mov     dword [ecx + 12], eax
        GetRealAddr
        mov     dword [ecx + 8], eax
        mov     dword [ecx + 4], RX_BUFF_SZ

        inc     [device.cur_rx]                         ; get next descriptor
        and     [device.cur_rx], NUM_RX_DESC-1          ; only 4 descriptors 0-3

        jmp     .rx_loop

  .no_rx:
        set_io  0
        set_io  cr
        in      eax, dx
        or      eax, RxENA                              ; Re-Enable the Receive state machine
        out     dx, eax

        pop     ax

  .no_rx_:
        test    ax, TxOK
        jz      .no_tx

        DEBUGF  1, "TX ok!\n"

  .tx_loop:
        movzx   ecx, [device.last_tx]
        shl     ecx, 4                  ; *16
        lea     ecx, [device.txd + ecx]

        test    dword [ecx + 4], 0x80000000   ; card owns descr
        jnz     .no_tx
        cmp     dword [ecx + 12], 0
        je      .no_tx

        DEBUGF  1, "Freeing packet = %x\n", [ecx + 12]:8
        push    dword [ecx + 12]
        mov     dword [ecx + 12], 0
        call    KernelFree

        inc     [device.last_tx]
        and     [device.last_tx], NUM_TX_DESC-1
        jmp     .tx_loop

  .no_tx:

        ret

  .error_status:
        DEBUGF  1, "Packet error: %x\n", ecx
        jmp     .fail

  .error_size:
        DEBUGF  1, "Packet too large/small\n"
        jmp     .fail

  .fail:
        DEBUGF  1, "FAILED\n"
        jmp     $
        ret



; End of code

align 4                                         ; Place all initialised data here

devices         dd 0

specific_table:
;    dd SIS630A_900_REV, Get_Mac_SIS630A_900_REV, 0
;    dd SIS630E_900_REV, Get_Mac_SIS630E_900_REV, 0
    dd SIS630S_900_REV, Get_Mac_SIS635_900_REV, 0
    dd SIS630EA1_900_REV, Get_Mac_SIS635_900_REV, 0
    dd SIS630ET_900_REV, Get_Mac_SIS635_900_REV, 0 ;SIS630ET_900_REV_SpecialFN
    dd SIS635A_900_REV, Get_Mac_SIS635_900_REV, 0
    dd SIS900_960_REV, SIS960_get_mac_addr, 0
    dd SIS900B_900_REV, SIS900_get_mac_addr, 0
    dd 0                                        ; end of list

version         dd (DRIVER_VERSION shl 16) or (API_VERSION and 0xFFFF)
my_service      db 'SIS900',0                  ; max 16 chars include zero

include_debug_strings                          ; All data wich FDO uses will be included here

section '.data' data readable writable align 16; place all uninitialized data place here

device_list     rd MAX_DEVICES                 ; This list contains all pointers to device structures the driver is handling

