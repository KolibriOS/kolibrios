;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved.    ;;
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

format PE DLL native
entry START

        CURRENT_API             = 0x0200
        COMPATIBLE_API          = 0x0100
        API_VERSION             = (COMPATIBLE_API shl 16) + CURRENT_API

        MAX_DEVICES             = 16

        TX_RING_SIZE            = 4
        RX_RING_SIZE            = 4

        __DEBUG__               = 1
        __DEBUG_LEVEL__         = 2     ; 1 = verbose, 2 = errors only

section '.flat' readable writable executable

include '../proc32.inc'
include '../struct.inc'
include '../macros.inc'
include '../fdo.inc'
include '../netdrv.inc'

; Capability flags used in chiplist
        FLAG_HAS_MII            = 1 shl 0
        FLAG_HAS_MEDIA_TABLE    = 1 shl 1
        FLAG_CSR12_IN_SROM      = 1 shl 2
        FLAG_ALWAYS_CHECK_MII   = 1 shl 3
        FLAG_HAS_ACPI           = 1 shl 4

; Chip id's
        DC21040                 = 0
        DC21041                 = 1
        DC21140                 = 2
        DC21142                 = 3
        DC21143                 = 3
        LC82C168                = 4
        MX98713                 = 5
        MX98715                 = 6
        MX98725                 = 7

;-------------------------------------------
; configuration registers
;-------------------------------------------
CFCS                    = 4             ; configuration and status register

CSR0                    = 0x00          ; Bus mode
CSR1                    = 0x08          ; Transmit Poll Command
CSR2                    = 0x10          ; Receive Poll Command
CSR3                    = 0x18          ; Receive list base address
CSR4                    = 0x20          ; Transmit list base address
CSR5                    = 0x28          ; Status
CSR6                    = 0x30          ; Operation mode
CSR7                    = 0x38          ; Interrupt enable
CSR8                    = 0x40          ; Missed frames and overflow counter
CSR9                    = 0x48          ; Boot ROM, serial ROM, and MII management
CSR10                   = 0x50          ; Boot ROM programming address
CSR11                   = 0x58          ; General-purpose timer
CSR12                   = 0x60          ; General-purpose port
CSR13                   = 0x68
CSR14                   = 0x70
CSR15                   = 0x78          ; Watchdog timer

;--------bits/commands of CSR0-------------------
CSR0_RESET              = 1b

CSR0_WIE                = 1 shl 24      ; Write and Invalidate Enable
CSR0_RLE                = 1 shl 23      ; PCI Read Line Enable
CSR0_RML                = 1 shl 21      ; PCI Read Multiple

CSR0_CACHEALIGN_NONE    = 00b shl 14
CSR0_CACHEALIGN_32      = 01b shl 14
CSR0_CACHEALIGN_64      = 10b shl 14
CSR0_CACHEALIGN_128     = 11b shl 14

; using values from linux driver..
CSR0_DEFAULT            = CSR0_WIE + CSR0_RLE + CSR0_RML + CSR0_CACHEALIGN_NONE

;------- CSR5 -STATUS- bits --------------------------------
CSR5_TI                 = 0x00000001    ;1 shl 0        ; Transmit interupt - frame transmition completed
CSR5_TPS                = 0x00000002    ;1 shl 1        ; Transmit process stopped
CSR5_TU                 = 0x00000004    ;1 shl 2        ; Transmit Buffer unavailable
CSR5_TJT                = 0x00000008    ;1 shl 3        ; Transmit Jabber Timeout (transmitter had been excessively active)
CSR5_LP                 = 0x00000010    ;1 shl 4        ; Link pass
CSR5_UNF                = 0x00000020    ;1 shl 5        ; Transmit underflow - FIFO underflow
CSR5_RI                 = 0x00000040    ;1 shl 6        ; Receive Interrupt
CSR5_RU                 = 0x00000080    ;1 shl 7        ; Receive Buffer unavailable
CSR5_RPS                = 0x00000100    ;1 shl 8        ; Receive Process stopped
CSR5_RWT                = 0x00000200    ;1 shl 9        ; Receive Watchdow Timeout
CSR5_ETI                = 0x00000400    ;1 shl 10       ; Early transmit Interrupt
CSR5_GTE                = 0x00000800    ;1 shl 11       ; General Purpose Timer Expired
CSR5_LF                 = 0x00001000    ;1 shl 12       ; Link Fail
CSR5_FBE                = 0x00002000    ;1 shl 13       ; Fatal bus error
CSR5_ERI                = 0x00004000    ;1 shl 14       ; Early receive Interrupt
CSR5_AIS                = 0x00008000    ;1 shl 15       ; Abnormal interrupt summary
CSR5_NIS                = 0x00010000    ;1 shl 16       ; normal interrupt summary
CSR5_RS_SH              = 17            ; Receive process state  -shift
CSR5_RS_MASK            = 111b          ;                        -mask
CSR5_TS_SH              = 20            ; Transmit process state -shift
CSR5_TS_MASK            = 111b          ;                        -mask
CSR5_EB_SH              = 23            ; Error bits             -shift
CSR5_EB_MASK            = 111b          ; Error bits             -mask

;CSR5 TS values
CSR5_TS_STOPPED                 = 000b
CSR5_TS_RUNNING_FETCHING_DESC   = 001b
CSR5_TS_RUNNING_WAITING_TX      = 010b
CSR5_TS_RUNNING_READING_BUFF    = 011b
CSR5_TS_RUNNING_SETUP_PCKT      = 101b
CSR5_TS_SUSPENDED               = 110b
CSR5_TS_RUNNING_CLOSING_DESC    = 111b

;------- CSR6 -OPERATION MODE- bits --------------------------------
CSR6_HP                 = 1 shl 0       ; Hash/Perfect Receive Filtering mode
CSR6_SR                 = 1 shl 1       ; Start/Stop receive
CSR6_HO                 = 1 shl 2       ; Hash only Filtering mode
CSR6_PB                 = 1 shl 3       ; Pass bad frames
CSR6_IF                 = 1 shl 4       ; Inverse filtering
CSR6_SB                 = 1 shl 5       ; Start/Stop backoff counter
CSR6_PR                 = 1 shl 6       ; Promiscuous mode -default after reset
CSR6_PM                 = 1 shl 7       ; Pass all multicast
CSR6_F                  = 1 shl 9       ; Full Duplex mode
CSR6_OM_SH              = 10            ; Operating Mode -shift
CSR6_OM_MASK            = 11b           ;                -mask
CSR6_FC                 = 1 shl 12      ; Force Collision Mode
CSR6_ST                 = 1 shl 13      ; Start/Stop Transmission Command
CSR6_TR_SH              = 14            ; Threshold Control      -shift
CSR6_TR_MASK            = 11b           ;                        -mask
CSR6_CA                 = 1 shl 17      ; Capture Effect Enable
CSR6_PS                 = 1 shl 18      ; Port select SRL / MII/SYM
CSR6_HBD                = 1 shl 19      ; Heartbeat Disable
CSR6_SF                 = 1 shl 21      ; Store and Forward -transmit full packet only
CSR6_TTM                = 1 shl 22      ; Transmit Threshold Mode -
CSR6_PCS                = 1 shl 23      ; PCS active and MII/SYM port operates in symbol mode
CSR6_SCR                = 1 shl 24      ; Scrambler Mode
CSR6_MBO                = 1 shl 25      ; Must Be One
CSR6_RA                 = 1 shl 30      ; Receive All
CSR6_SC                 = 1 shl 31      ; Special Capture Effect Enable


;------- CSR7 -INTERRUPT ENABLE- bits --------------------------------
CSR7_TI                 = 1 shl 0       ; transmit Interrupt Enable (set with CSR7<16> & CSR5<0> )
CSR7_TS                 = 1 shl 1       ; transmit Stopped Enable (set with CSR7<15> & CSR5<1> )
CSR7_TU                 = 1 shl 2       ; transmit buffer underrun Enable (set with CSR7<16> & CSR5<2> )
CSR7_TJ                 = 1 shl 3       ; transmit jabber timeout enable (set with CSR7<15> & CSR5<3> )
CSR7_UN                 = 1 shl 5       ; underflow Interrupt enable (set with CSR7<15> & CSR5<5> )
CSR7_RI                 = 1 shl 6       ; receive Interrupt enable (set with CSR7<16> & CSR5<5> )
CSR7_RU                 = 1 shl 7       ; receive buffer unavailable enable (set with CSR7<15> & CSR5<7> )
CSR7_RS                 = 1 shl 8       ; Receive stopped enable (set with CSR7<15> & CSR5<8> )
CSR7_RW                 = 1 shl 9       ; receive watchdog timeout enable (set with CSR7<15> & CSR5<9> )
CSR7_ETE                = 1 shl 10      ; Early transmit Interrupt enable (set with CSR7<15> & CSR5<10> )
CSR7_GPT                = 1 shl 11      ; general purpose timer enable (set with CSR7<15> & CSR5<11> )
CSR7_FBE                = 1 shl 13      ; Fatal bus error enable (set with CSR7<15> & CSR5<13> )
CSR7_ERE                = 1 shl 14      ; Early receive enable (set with CSR7<16> & CSR5<14> )
CSR7_AI                 = 1 shl 15      ; Abnormal Interrupt Summary Enable (enables CSR5<0,3,7,8,9,10,13>)
CSR7_NI                 = 1 shl 16      ; Normal Interrup Enable (enables CSR5<0,2,6,11,14>)

CSR7_DEFAULT            = CSR7_TI + CSR7_TS + CSR7_RI + CSR7_RS + CSR7_TU + CSR7_TJ + CSR7_UN + \
                                        CSR7_RU + CSR7_RW + CSR7_FBE + CSR7_AI + CSR7_NI
;common to Rx and Tx
DES0_OWN                = 1 shl 31              ; if set, the NIC controls the descriptor, otherwise driver 'owns' the descriptors

;receive
RDES0_ZER               = 1 shl 0               ; must be 0 if legal length :D
RDES0_CE                = 1 shl 1               ; CRC error, valid only on last desc (RDES0<8>=1)
RDES0_DB                = 1 shl 2               ; dribbling bit - not multiple of 8 bits, valid only on last desc (RDES0<8>=1)
RDES0_RE                = 1 shl 3               ; Report on MII error.. i dont realy know what this means :P
RDES0_RW                = 1 shl 4               ; received watchdog timer expiration - must set CSR5<9>, valid only on last desc (RDES0<8>=1)
RDES0_FT                = 1 shl 5               ; frame type: 0->IEEE802.0 (len<1500) 1-> ETHERNET frame (len>1500), valid only on last desc (RDES0<8>=1)
RDES0_CS                = 1 shl 6               ; Collision seen, valid only on last desc (RDES0<8>=1)
RDES0_TL                = 1 shl 7               ; Too long(>1518)-NOT AN ERROR, valid only on last desc (RDES0<8>=1)
RDES0_LS                = 1 shl 8               ; Last descriptor of current frame
RDES0_FS                = 1 shl 9               ; First descriptor of current frame
RDES0_MF                = 1 shl 10              ; Multicast frame, valid only on last desc (RDES0<8>=1)
RDES0_RF                = 1 shl 11              ; Runt frame, valid only on last desc (RDES0<8>=1) and id overflow
RDES0_DT_SERIAL         = 00b shl 12            ; Data type-Serial recv frame, valid only on last desc (RDES0<8>=1)
RDES0_DT_INTERNAL       = 01b shl 12            ; Data type-Internal loopback recv frame, valid only on last desc (RDES0<8>=1)
RDES0_DT_EXTERNAL       = 11b shl 12            ; Data type-External loopback recv frame, valid only on last desc (RDES0<8>=1)
RDES0_DE                = 1 shl 14              ; Descriptor error - cant own a new desc and frame doesnt fit, valid only on last desc (RDES0<8>=1)
RDES0_ES                = 1 shl 15              ; Error Summmary - bits 1+6+11+14, valid only on last desc (RDES0<8>=1)
RDES0_FL_SH             = 16                    ; Field length shift, valid only on last desc (RDES0<8>=1)
RDES0_FL_MASK           = 11111111111111b       ; Field length mask (+CRC), valid only on last desc (RDES0<8>=1)
RDES0_FF                = 1 shl 30              ; Filtering fail-frame failed address recognition test(must CSR6<30>=1), valid only on last desc (RDES0<8>=1)

RDES1_RBS1_MASK         = 11111111111b          ; first buffer size MASK
RDES1_RBS2_SH           = 11                    ; second buffer size SHIFT
RDES1_RBS2_MASK         = 11111111111b          ; second buffer size MASK
RDES1_RCH               = 1 shl 24              ; Second address chained - second address (buffer) is next desc address
RDES1_RER               = 1 shl 25              ; Receive End of Ring - final descriptor, NIC must return to first desc

;transmition
TDES0_DE                = 1 shl 0               ; Deffered
TDES0_UF                = 1 shl 1               ; Underflow error
TDES0_LF                = 1 shl 2               ; Link fail report (only if CSR6<23>=1)
TDES0_CC_SH             = 3                     ; Collision Count shift - no of collision before transmition
TDES0_CC_MASK           = 1111b                 ; Collision Count mask
TDES0_HF                = 1 shl 7               ; Heartbeat fail
TDES0_EC                = 1 shl 8               ; Excessive Collisions - >16 collisions
TDES0_LC                = 1 shl 9               ; Late collision
TDES0_NC                = 1 shl 10              ; No carrier
TDES0_LO                = 1 shl 11              ; Loss of carrier
TDES0_TO                = 1 shl 14              ; Transmit Jabber Timeout
TDES0_ES                = 1 shl 15              ; Error summary TDES0<1+8+9+10+11+14>=1

TDES1_TBS1_MASK         = 11111111111b          ; Buffer 1 size mask
TDES1_TBS2_SH           = 11                    ; Buffer 2 size shift
TDES1_TBS2_MASK         = 11111111111b          ; Buffer 2 size mask
TDES1_FT0               = 1 shl 22              ; Filtering type 0
TDES1_DPD               = 1 shl 23              ; Disabled padding for packets <64bytes, no padding
TDES1_TCH               = 1 shl 24              ; Second address chained - second buffer pointer is to next desc
TDES1_TER               = 1 shl 25              ; Transmit end of ring - final descriptor
TDES1_AC                = 1 shl 26              ; Add CRC disable -pretty obvious
TDES1_SET               = 1 shl 27              ; Setup packet
TDES1_FT1               = 1 shl 28              ; Filtering type 1
TDES1_FS                = 1 shl 29              ; First segment - buffer is first segment of frame
TDES1_LS                = 1 shl 30              ; Last segment
TDES1_IC                = 1 shl 31              ; Interupt on completion (CSR5<0>=1) valid when TDES1<30>=1

FULL_DUPLEX_MAGIC       = 0x6969
;MAX_ETH_FRAME_SIZE      = 1514

struct  device          ETH_DEVICE

        io_addr         dd ?
        pci_bus         dd ?
        pci_dev         dd ?
        irq_line        db ?
                        rb 3    ; alignment

        id              dd ?    ; identification number
        io_size         dd ?
        flags           dd ?
        csr6            dd ?
        csr7            dd ?
        if_port         dd ?
        saved_if_port   dd ?
        default_port    dd ?
        mtable          dd ?
        mii_cnt         dd ?

        cur_rx          dd ?
        cur_tx          dd ?    ; Tx current descriptor to write data to
        last_tx         dd ?    ; Tx current descriptor to read TX completion

        rb 0x100-($ and 0xff)   ; align 256
        rx_ring         rb RX_RING_SIZE*2*sizeof.desc

        rb 0x100-($ and 0xff)   ; align 256
        tx_ring         rb TX_RING_SIZE*2*sizeof.desc

ends

;----------- descriptor structure ---------------------
struct  desc
        status          dd ?    ; bit 31 is 'own' and rest is 'status'
        length          dd ?    ; control bits + bytes-count buffer 1 + bytes-count buffer 2
        buffer1         dd ?    ; pointer to buffer1
        buffer2         dd ?    ; pointer to buffer2
ends

;=============================================================================
; serial ROM operations
;=============================================================================
CSR9_SR                 = 1 shl 11        ; SROM Select
CSR9_RD                 = 1 shl 14        ; ROM Read Operation
CSR9_SROM_DO            = 1 shl 3         ; Data Out for SROM
CSR9_SROM_DI            = 1 shl 2         ; Data In to SROM
CSR9_SROM_CK            = 1 shl 1         ; clock for SROM
CSR9_SROM_CS            = 1 shl 0         ; chip select.. always needed

; assume dx is CSR9
macro SROM_Delay {
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

; assume dx is CSR9
macro MDIO_Delay {
        push    eax
        in      eax, dx
        pop     eax
}

macro Bit_Set a_bit {
        in      eax, dx
        or      eax, a_bit
        out     dx , eax
}

macro Bit_Clear a_bit {
        in      eax, dx
        and     eax, not (a_bit)
        out     dx, eax
}


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

        cmp     [edx + IOCTL.inp_size], 3                               ; Data input must be at least 3 bytes
        jb      .fail

        mov     eax, [edx + IOCTL.input]
        cmp     byte [eax], 1                                           ; 1 means device number and bus number (pci) are given
        jne     .fail                                                   ; other types arent supported for this card yet

; check if the device is already listed

        mov     esi, device_list
        mov     ecx, [devices]
        test    ecx, ecx
        jz      .firstdevice

;        mov     eax, [edx + IOCTL.input]                                ; get the pci bus and device numbers
        mov     ax, [eax+1]                                             ;
  .nextdevice:
        mov     ebx, [esi]
        cmp     al, byte[ebx + device.pci_bus]
        jne     @f
        cmp     ah, byte[ebx + device.pci_dev]
        je      .find_devicenum                                         ; Device is already loaded, let's find it's device number
       @@:
        add     esi, 4
        loop    .nextdevice


; This device doesnt have its own eth_device structure yet, lets create one
  .firstdevice:
        cmp     [devices], MAX_DEVICES                                  ; First check if the driver can handle one more card
        jae     .fail

        allocate_and_clear ebx, sizeof.device, .fail

; Fill in the direct call addresses into the struct
        mov     [ebx + device.reset], reset
        mov     [ebx + device.transmit], transmit
        mov     [ebx + device.unload], unload
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

        DEBUGF  2,"Hooking into device, dev:%x, bus:%x, irq:%x, addr:%x\n",\
        [ebx + device.pci_dev]:1,[ebx + device.pci_bus]:1,[ebx + device.irq_line]:1,[ebx + device.io_addr]:8

; Ok, the eth_device structure is ready, let's probe the device
; Because initialization fires IRQ, IRQ handler must be aware of this device
        mov     eax, [devices]                                          ; Add the device structure to our device list
        mov     [device_list+4*eax], ebx                                ; (IRQ handler uses this list to find device)
        inc     [devices]                                               ;

        call    probe                                                   ; this function will output in eax
        test    eax, eax
        jnz     .err2                                                   ; If an error occured, exit

        mov     [ebx + device.type], NET_TYPE_ETH
        invoke  NetRegDev

        cmp     eax, -1
        je      .destroy

        ret

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

  .err2:
        dec     [devices]
  .err:
        DEBUGF  2,"removing device structure\n"
        invoke  KernelFree, ebx

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



align 4
unload:
        ; TODO: (in this particular order)
        ;
        ; - Stop the device
        ; - Detach int handler
        ; - Remove device from local list
        ; - call unregister function in kernel
        ; - Remove all allocated structures and buffers the card used

        or      eax, -1
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                         ;;
;; Probe                                   ;;
;;                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
probe:

        DEBUGF  2,"Probing\n"

        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], 0        ; get device/vendor id
        mov     esi, chiplist
  .loop:
        cmp     dword[esi], eax
        je      .got_it
        add     esi, 6*4
        cmp     dword[esi], 0
        jne     .loop
        DEBUGF  2, "Unknown chip: 0x%x aborting\n", eax

        or      eax, -1
        ret

  .got_it:
        lodsd
        lodsd
        mov     [ebx + device.id], eax
        lodsd
        mov     [ebx + device.io_size], eax
        lodsd
        mov     [ebx + device.csr7], eax
        lodsd
        mov     [ebx + device.name], eax
        DEBUGF  1, "Detected chip = %s\n", eax
        lodsd
        mov     [ebx + device.flags], eax

; PROBE1

        test    [ebx + device.flags], FLAG_HAS_ACPI
        jz      .no_acpi
        DEBUGF  1, "Device has ACPI capabilities, time to wake it up\n"
        xor     eax, eax
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], 0x40, eax       ; wake up the 21143
  .no_acpi:

        call    SROM_GetWidth           ; TODO: use this value returned in ecx in the read_word routine!

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                         ;;
;; Reset                                   ;;
;;                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
reset:

        DEBUGF  2,"Reset\n"

; Make the device a bus master
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command
        or      al, PCI_CMD_MASTER
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command, eax

; Stop TX and RX
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], CSR6
        in      eax, dx
        and     eax, not (CSR6_ST or CSR6_SR)
        out     dx, eax

; Clear missed packet counter
        set_io  [ebx + device.io_addr], CSR8
        in      eax, dx

;; wait at least 50 PCI cycles
;        mov     esi, 1000
;        invoke  Sleep

        cmp     [ebx + device.id], DC21041
        jne     @f
;        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], CSR9
        in      eax, dx
        test    eax, 0x8000
        jz      @f
        DEBUGF  1, "21040 compatibility mode\n"
        mov     [ebx + device.id], DC21040
  @@:


; Reset the xcvr interface and turn on heartbeat.
        cmp     [ebx + device.id], DC21041
        jne     @f
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], CSR13
        xor     eax, eax
        out     dx, eax
        set_io  [ebx + device.io_addr], CSR14
        dec     eax
        out     dx, eax
        set_io  [ebx + device.io_addr], CSR15
        inc     eax
        mov     al, 8
        out     dx, eax
        set_io  [ebx + device.io_addr], CSR6
        in      eax, dx
        or      ax, CSR6_ST
        out     dx, eax
        set_io  [ebx + device.io_addr], CSR13
        xor     eax, eax
        mov     ax, 0xEF05
        out     dx, eax
        jmp     .reset_done
  @@:
        cmp     [ebx + device.id], DC21040
        jne     @f
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], CSR13
        xor     eax, eax
        out     dx, eax
        mov     al, 4
        out     dx, eax
        jmp     .reset_done
  @@:
        cmp     [ebx + device.id], DC21140
        jne     @f
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], CSR12
        mov     eax, 0x100
        out     dx, eax
        jmp     .reset_done
  @@:
        cmp     [ebx + device.id], DC21142
        jne     @f
        ; if tp->mii_cnt
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], CSR6
        mov     eax, 0x82020000
        out     dx, eax
        set_io  [ebx + device.io_addr], CSR13
        xor     eax, eax
        out     dx, eax
        set_io  [ebx + device.io_addr], CSR14
        out     dx, eax
        set_io  [ebx + device.io_addr], CSR6
        mov     eax, 0x820E0000
        out     dx, eax
        jmp     .reset_done
        ;;;; TODO
  @@:
        cmp     [ebx + device.id], LC82C168
        jne     @f
        ; TODO
  @@:
        cmp     [ebx + device.id], MX98713
        jne     @f
        ; TODO
  @@:

  .reset_done:


; OPEN

; Reset chip
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], CSR0
        mov     eax, CSR0_RESET
        out     dx, eax

; wait at least 50 PCI cycles
        mov     esi, 100
        invoke  Sleep

;-----------------------------------
; Read mac from eeprom to driver ram

        call    read_mac_eeprom

;--------------------------------
; insert irq handler on given irq

        movzx   eax, [ebx + device.irq_line]
        DEBUGF  1,"Attaching int handler to irq %x\n", eax:1
        invoke  AttachIntHandler, eax, int_handler, ebx
        test    eax, eax
        jnz     @f
        DEBUGF  2,"Could not attach int handler!\n"
        or      eax, -1
        ret
  @@:

;----------------
; Set cache modes

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], CSR0
        mov     eax, 0x01A00000 or 0x4800 ; CSR0_DEFAULT
        out     dx, eax

        ; wait at least 50 PCI cycles
        mov     esi, 100
        invoke  Sleep

;---------------------------
; Initialize RX and TX rings

        call    init_ring
        test    eax, eax
        jnz     .err

;-------------------
; Set receive filter

        call    create_setup_frame

;--------------------------------------------
; setup CSR3 & CSR4 (pointers to descriptors)

        lea     eax, [ebx + device.rx_ring]
        invoke  GetPhysAddr
        DEBUGF  1,"RX descriptor base address: %x\n", eax
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], CSR3
        out     dx, eax

        lea     eax, [ebx + device.tx_ring]
        invoke  GetPhysAddr
        DEBUGF  1,"TX descriptor base address: %x\n", eax
        set_io  [ebx + device.io_addr], CSR4
        out     dx, eax

; Select media
        push    [ebx + device.if_port]
        pop     [ebx + device.saved_if_port]
        cmp     [ebx + device.if_port], 0
        jne     @f
        push    [ebx + device.default_port]
        pop     [ebx + device.if_port]
  @@:
        cmp     [ebx + device.id], DC21041
        jne     @f
        cmp     [ebx + device.if_port], 4
        jbe     @f
        ; invalid port, select inital TP, autosense, autonegotiate
        mov     [ebx + device.if_port], 4                               ; CHECKME
  @@:

; Allow selecting a default media
        cmp     [ebx + device.mtable], 0
        je      .media_picked

        cmp     [ebx + device.if_port], 0
        je      @f
        ;; TODO
        jmp     .media_picked
  @@:

  .media_picked:
        mov     [ebx + device.csr6], 0

        cmp     [ebx + device.id], DC21142
        jne     @f
        cmp     [ebx + device.if_port], 0
        jne     @f
        ;; TODO
        mov     [ebx + device.csr6], 0x82420200
        mov     [ebx + device.if_port], 11
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], CSR14
        mov     eax, 0x0003FFF
        out     dx, eax
        set_io  [ebx + device.io_addr], CSR15
        xor     eax, eax
        mov     al, 8
        out     dx, eax
        set_io  [ebx + device.io_addr], CSR13
        mov     al, 1
        out     dx, eax
        set_io  [ebx + device.io_addr], CSR12
        mov     ax, 0x1301
        out     dx, eax

  @@:
        cmp     [ebx + device.id], LC82C168
        jne     @f
        ;; TODO
  @@:
        cmp     [ebx + device.id], MX98713
        jne     @f

  @@:
;; wait a bit
;        mov     esi, 500
;        invoke  Sleep

; else:
        xor     eax, eax
        inc     eax
        call    select_media

; Start the chip's tx to process setup frame
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], CSR6
        mov     eax, [ebx + device.csr6]
        out     dx, eax
        or      ax, CSR6_ST
        out     dx, eax

; Enable interrupts by setting the interrupt mask.
        set_io  [ebx + device.io_addr], CSR5
        mov     eax, [ebx + device.csr7]
        DEBUGF  1, "Setting CSR7 to 0x%x\n", eax
        out     dx, eax
        set_io  [ebx + device.io_addr], CSR7
        out     dx, eax

; Enable receiver
        set_io  [ebx + device.io_addr], CSR6
        mov     eax, [ebx + device.csr6]
        or      eax, 0x2002 + CSR6_RA
        out     dx, eax

; RX poll demand
        set_io  [ebx + device.io_addr], CSR2
        xor     eax, eax
        out     dx, eax

; Set the mtu, kernel will be able to send now
        mov     [ebx + device.mtu], 1514

; Set link state to unknown
        mov     [ebx + device.state], ETH_LINK_UNKNOWN

        DEBUGF  1,"Reset completed\n"
;        xor     eax, eax
        ret

  .err:
        DEBUGF  2,"Reset failed\n"
        or      eax, -1
        ret



align 4
init_ring:

        DEBUGF  1,"Init ring\n"

;---------------------
; Setup RX descriptors

        lea     eax, [ebx + device.rx_ring]
        invoke  GetPhysAddr
        mov     edx, eax
        push    eax
        lea     edi, [ebx + device.rx_ring]
        mov     ecx, RX_RING_SIZE
  .loop_rx_des:
        DEBUGF  1,"RX descriptor 0x%x\n", edi
        add     edx, sizeof.desc
        mov     [edi + desc.status], DES0_OWN
        mov     [edi + desc.length], 1514
        push    edx edi ecx
        invoke  NetAlloc, 1514+NET_BUFF.data
        pop     ecx edi edx
        test    eax, eax
        jz      .out_of_mem
        mov     [edi + RX_RING_SIZE*sizeof.desc], eax
        push    edx
        invoke  GetPhysAddr
        add     eax, NET_BUFF.data
        pop     edx
        mov     [edi + desc.buffer1], eax
        mov     [edi + desc.buffer2], edx
        add     edi, sizeof.desc
        dec     ecx
        jnz     .loop_rx_des

; set last descriptor as LAST
        or      [edi - sizeof.desc + desc.length], RDES1_RER           ; EndOfRing
        pop     [edi - sizeof.desc + desc.buffer2]                     ; point it to the first descriptor

;---------------------
; Setup TX descriptors

        lea     eax, [ebx + device.tx_ring]
        invoke  GetPhysAddr
        mov     edx, eax
        push    eax
        lea     edi, [ebx + device.tx_ring]
        mov     ecx, TX_RING_SIZE
  .loop_tx_des:
        DEBUGF  1,"TX descriptor 0x%x\n", edi
        add     edx, sizeof.desc
        mov     [edi + desc.status], 0                                  ; owned by driver
        mov     [edi + desc.length], 0
        mov     [edi + desc.buffer1], 0
        mov     [edi + desc.buffer2], edx                               ; pointer to next descr
        add     edi, sizeof.desc
        dec     ecx
        jnz     .loop_tx_des
; set last descriptor as LAST
        or      [edi - sizeof.desc + desc.length], TDES1_TER            ; EndOfRing
        pop     [edi - sizeof.desc + desc.buffer2]                      ; point it to the first descriptor

;------------------
; Reset descriptors

        xor     eax, eax
        mov     [ebx + device.cur_tx], eax
        mov     [ebx + device.last_tx], eax
        mov     [ebx + device.cur_rx], eax

;        xor     eax, eax
        ret

  .out_of_mem:
        DEBUGF  2, "Out of memory!\n"
        pop     eax
        or      eax, -1
        ret


; IN: eax = startup
align 4
select_media:

        DEBUGF  1, "Selecting media\n"

        cmp     [ebx + device.mtable], 0
        je      .no_mtable
        DEBUGF  1, "Device has a media table\n"


; default:
        mov     eax, 0x020E0000
        jmp     .update_csr6

  .no_mtable:
        DEBUGF  1, "Device has no media table\n"

        cmp     [ebx + device.id], DC21041
        jne     .not_41
        DEBUGF  1, "DC21041\n"

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], CSR13
        xor     eax, eax
        out     dx, eax         ; reset serial interface
        set_io  [ebx + device.io_addr], CSR14
        mov     eax, 0x7F3F     ;0x7F3F     ;0x7F3D     ; 10T-FD
        out     dx, eax
        set_io  [ebx + device.io_addr], CSR15
        mov     eax, 0x0008     ;0x0008     ;0x0008     ; 10T-FD
        out     dx, eax
        set_io  [ebx + device.io_addr], CSR13
        mov     eax, 0xEF05     ;0xEF01     ;0xEF09     ; 10T-FD
        out     dx, eax
        mov     eax, 0x80020000
        jmp     .update_csr6
  .not_41:
        cmp     [ebx + device.id], LC82C168
        jne     .not_LC
        DEBUGF  1, "LC82C168\n"

        ;; TODO

        mov     eax, 0x812C0000
        jmp     .update_csr6
  .not_LC:
        cmp     [ebx + device.id], DC21040
        jne     .not_40
        DEBUGF  1, "DC21040\n"

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], CSR11
        mov     eax, FULL_DUPLEX_MAGIC
        out     dx, eax
        ; reset serial interface
        set_io  [ebx + device.io_addr], CSR13
        xor     eax, eax
        out     dx, eax

        set_io  [ebx + device.io_addr], CSR13
        xor     eax, eax
        cmp     [ebx + device.if_port], 0
        je      @f
        mov     al, 0xc
        out     dx, eax
        mov     eax, 0x01860000
        jmp     .update_csr6
  @@:
        mov     al, 4
        out     dx, eax
        mov     eax, 0x00420000
        jmp     .update_csr6

  .not_40:
        DEBUGF  1, "Unkown chip with no media table\n"

        cmp     [ebx + device.default_port], 0
        jne     .not_0
        cmp     [ebx + device.mii_cnt], 0
        je      @f
        mov     [ebx + device.if_port], 11
        jmp     .not_0
  @@:
        mov     [ebx + device.if_port], 3
  .not_0:
        mov     eax, 0x020E0000 ;;;;;

  .update_csr6:
        and     [ebx + device.csr6], 0xfdff
        or      ax, 0x0200                     ;; FULL DUPLEX
        or      [ebx + device.csr6], eax
        DEBUGF  1, "new CSR6: 0x%x\n", [ebx + device.csr6]

        ret


align 4
start_link:

        DEBUGF  1,"Starting link\n"

        ; TODO: write working code here

        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                         ;;
;; Send setup packet                       ;;
;;                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
create_setup_frame:

        DEBUGF  1,"Creating setup packet\n"

        invoke  NetAlloc, 192 + NET_BUFF.data
        test    eax, eax
        jz      .err
        mov     [eax + NET_BUFF.device], ebx
        mov     [eax + NET_BUFF.length], 192

        push    eax

        lea     edi, [eax + NET_BUFF.data]
        xor     eax, eax
        dec     ax
        stosd
        stosd
        stosd

        mov     ecx, 15
  .loop:
        lea     esi, [ebx + device.mac]
        lodsw
        stosd
        dec     ecx
        jnz     .loop

        pop     eax

; setup descriptor
        lea     edi, [ebx + device.tx_ring]
        DEBUGF  1, "attaching setup packet 0x%x to descriptor 0x%x\n", eax, edi
        mov     [edi + TX_RING_SIZE*sizeof.desc], eax
        invoke  GetPhysAddr
        add     eax, NET_BUFF.data
        mov     [edi + desc.buffer1], eax
        mov     [edi + desc.length], TDES1_SET or 192       ; size must be EXACTLY 192 bytes + TDES1_IC
        mov     [edi + desc.status], DES0_OWN
        DEBUGF  1, "descriptor 0x%x\n", edi

; go to next descriptor
        inc     [ebx + device.cur_tx]
        and     [ebx + device.cur_tx], TX_RING_SIZE-1

        xor     eax, eax
        ret

  .err:
        DEBUGF  2, "Out of memory!\n"
        or      eax, -1
        ret



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                         ;;
;; Transmit                                ;;
;;                                         ;;
;; In:  ebx = pointer to device structure  ;;
;; Out: eax = 0 on success                 ;;
;;                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc transmit stdcall bufferptr

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

        mov     eax, [ebx + device.cur_tx]
        mov     edx, sizeof.desc
        mul     edx
        lea     edi, [ebx + device.tx_ring + eax]
        test    [edi + desc.status], DES0_OWN
        jnz     .fail

        mov     eax, [bufferptr]
        mov     [edi + TX_RING_SIZE*sizeof.desc], eax
        add     eax, [eax + NET_BUFF.offset]
        invoke  GetPhysAddr
        mov     [edi + desc.buffer1], eax

; set packet size
        mov     eax, [edi + desc.length]
        and     eax, TDES1_TER                          ; preserve 'End of Ring' bit
        or      eax, [esi + NET_BUFF.length]            ; set size
        or      eax, TDES1_FS or TDES1_LS or TDES1_IC   ; first descr, last descr, interrupt on complete
        mov     [edi + desc.length], eax

; set descriptor status
        mov     [edi + desc.status], DES0_OWN            ; say it is now owned by the 21x4x

; Check if transmitter is running
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], CSR6
        in      eax, dx
        test    eax, CSR6_ST                            ; if NOT started, start now
        jnz     .already_started
        or      eax, CSR6_ST
        DEBUGF  1,"(Re) starting TX\n"
        jmp     .do_it
  .already_started:

; Trigger immediate transmit demand
        set_io  [ebx + device.io_addr], CSR1
        xor     eax, eax
  .do_it:
        out     dx, eax

; Update stats
        inc     [ebx + device.packets_tx]
        mov     ecx, [esi + NET_BUFF.length]
        add     dword [ebx + device.bytes_tx], ecx
        adc     dword [ebx + device.bytes_tx + 4], 0

; go to next descriptor
        inc     [ebx + device.cur_tx]
        and     [ebx + device.cur_tx], TX_RING_SIZE-1

        DEBUGF  1,"Transmit ok\n"
        popf
        xor     eax, eax
        ret

  .fail:
        DEBUGF  1,"Transmit failed\n"
        invoke  NetFree, [bufferptr]
        popf
        or      eax, -1
        ret

endp

;;;;;;;;;;;;;;;;;;;;;;;
;;                   ;;
;; Interrupt handler ;;
;;                   ;;
;;;;;;;;;;;;;;;;;;;;;;;

align 4
int_handler:

        push    ebx esi edi

        DEBUGF  1,"INT\n"

; find pointer of device wich made IRQ occur

        mov     ecx, [devices]
        test    ecx, ecx
        jz      .nothing
        mov     esi, device_list
  .nextdevice:
        mov     ebx, [esi]

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], CSR5
        in      eax, dx
        and     eax, 0x0001ffff
        out     dx, eax                                 ; send it back to ACK
        jnz     .got_it
  .continue:
        add     esi, 4
        dec     ecx
        jnz     .nextdevice
  .nothing:
        pop     edi esi ebx
        xor     eax, eax

        ret                                             ; If no device was found, abort (The irq was probably for a device, not registered to this driver)

  .got_it:
        DEBUGF  1,"Device: %x CSR5: %x\n", ebx, eax

;----------------------------------
; TX ok?

        test    eax, CSR5_TI
        jz      .not_tx

        push    eax esi ecx
        DEBUGF  1,"TX ok!\n"
      .loop_tx:
        ; get last descriptor
        mov     eax, [ebx + device.last_tx]
        mov     edx, sizeof.desc
        mul     edx
        lea     eax, [ebx + device.tx_ring + eax]

        DEBUGF  1,"descriptor 0x%x\n", eax
        test    [eax + desc.status], DES0_OWN           ; owned by the card?
        jnz     .end_tx
        cmp     [eax + desc.buffer1], 0                 ; empty descriptor?
        je      .end_tx

        mov     [eax + desc.buffer1], 0
        DEBUGF  1, "Free buffer 0x%x\n", [eax + TX_RING_SIZE*sizeof.desc]
        invoke  NetFree, [eax + TX_RING_SIZE*sizeof.desc]

        ; advance to next descriptor
        inc     [ebx + device.last_tx]
        and     [ebx + device.last_tx], TX_RING_SIZE-1

        jmp     .loop_tx
  .end_tx:
        pop     ecx esi eax
  .not_tx:

;----------------------------------
; RX irq
        test    eax, CSR5_RI
        jz      .not_rx
        push    eax esi ecx

        DEBUGF 1,"RX ok!\n"

        push    ebx
  .rx_loop:
        pop     ebx

        ; get current descriptor
        mov     eax, [ebx + device.cur_rx]
        mov     edx, sizeof.desc
        mul     edx
        lea     edi, [ebx + device.rx_ring + eax]

        ; now check status
        mov     eax, [edi + desc.status]

        test    eax, DES0_OWN
        jnz     .end_rx                                 ; current desc is busy, nothing to do
        test    eax, RDES0_FS
        jz      .end_rx                                 ; current desc is NOT first packet, ERROR!
        test    eax, RDES0_LS                           ; if not last desc of packet, error for now
        jz      .end_rx
        test    eax, RDES0_ES
        jnz     .end_rx

; Calculate length
        mov     ecx, [edi + desc.status]
        shr     ecx, RDES0_FL_SH
        and     ecx, RDES0_FL_MASK
        sub     ecx, 4                                  ; throw away the CRC
        DEBUGF  1,"got %u bytes\n", ecx

; Push arguments for EthInput (and some more...)
        push    ebx
        push    .rx_loop                                ; return addr
        mov     eax, dword[edi + RX_RING_SIZE*sizeof.desc]
        push    eax
        mov     [eax + NET_BUFF.length], ecx
        mov     [eax + NET_BUFF.device], ebx
        mov     [eax + NET_BUFF.offset], NET_BUFF.data

; update statistics
        inc     [ebx + device.packets_rx]
        add     dword[ebx + device.bytes_rx], ecx
        adc     dword[ebx + device.bytes_rx + 4], 0

; Allocate new descriptor
        push    edi ebx
        invoke  NetAlloc, 1514 + NET_BUFF.data          ; Allocate a buffer to put packet into
        pop     ebx edi
        jz      .fail
        mov     [edi + RX_RING_SIZE*sizeof.desc], eax
        invoke  GetPhysAddr
        add     eax, NET_BUFF.data
        mov     [edi + desc.buffer1], eax
        mov     [edi + desc.status], DES0_OWN           ; mark descriptor as being free

; Move to next rx desc
        inc     [ebx + device.cur_rx]                   ; next descriptor
        and     [ebx + device.cur_rx], RX_RING_SIZE-1

        jmp     [EthInput]
  .end_rx:
  .fail:
        pop     ecx esi eax
  .not_rx:

        pop     edi esi ebx
        ret



align 4
write_mac:      ; in: mac pushed onto stack (as 3 words)

        DEBUGF  1,"Writing MAC\n"

; write data into driver cache
        mov     esi, esp
        lea     edi, [ebx + device.mac]
        movsd
        movsw
        add     esp, 6
        
;; send setup packet (only if driver is started)
;;        call    Create_Setup_Packet

align 4
read_mac_eeprom:

        DEBUGF  1,"Reading MAC from eeprom\n"

        lea     edi, [ebx + device.mac]
        mov     esi, 20/2               ; read words, start address is 20
     .loop:
        push    esi edi
        call    SROM_Read_Word
        pop     edi esi
        stosw
        inc     esi
        cmp     esi, 26/2
        jb      .loop

        DEBUGF  1,"%x-%x-%x-%x-%x-%x\n",[edi-6]:2,[edi-5]:2,[edi-4]:2,[edi-3]:2,[edi-2]:2,[edi-1]:2

        ret


align 4
SROM_GetWidth:  ; should be 6 or 8 according to some manuals (returns in ecx)

;        DEBUGF 1,"SROM_GetWidth\n"

        call    SROM_Idle
        call    SROM_EnterAccessMode

;        set_io  [ebx + device.io_addr], 0
;        set_io  [ebx + device.io_addr], CSR9

        ; send 110b

        in      eax, dx
        or      eax, CSR9_SROM_DI
        call    SROM_out

        in      eax, dx
        or      eax, CSR9_SROM_DI
        call    SROM_out

        in      eax, dx
        and     eax, not (CSR9_SROM_DI)
        call    SROM_out
        
        mov     ecx,1
  .loop2:
        Bit_Set CSR9_SROM_CK
        SROM_Delay
        
        in      eax, dx
        and     eax, CSR9_SROM_DO
        jnz     .not_zero

        Bit_Clear CSR9_SROM_CK
        SROM_Delay
        jmp     .end_loop2
  .not_zero:
        
        Bit_Clear CSR9_SROM_CK
        SROM_Delay
        
        inc     ecx
        cmp     ecx, 12
        jbe     .loop2
  .end_loop2:
        
        DEBUGF  1,"SROM width=%u\n", ecx
        
        call    SROM_Idle
        call    SROM_EnterAccessMode
        call    SROM_Idle
        
        ret


align 4
SROM_out:

        out     dx, eax
        SROM_Delay
        Bit_Set CSR9_SROM_CK
        SROM_Delay
        Bit_Clear CSR9_SROM_CK
        SROM_Delay

        ret



align 4
SROM_EnterAccessMode:

;        DEBUGF 1,"SROM_EnterAccessMode\n"

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], CSR9
        mov     eax, CSR9_SR
        out     dx, eax
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

;        DEBUGF 1,"SROM_Idle\n"

        call    SROM_EnterAccessMode
        
;        set_io  [ebx + device.io_addr], 0
;        set_io  [ebx + device.io_addr], CSR9
        
        mov     ecx, 25
     .loop_clk:

        Bit_Clear CSR9_SROM_CK
        SROM_Delay
        Bit_Set CSR9_SROM_CK
        SROM_Delay
        
        dec     ecx
        jnz     .loop_clk

        
        Bit_Clear CSR9_SROM_CK
        SROM_Delay
        Bit_Clear CSR9_SROM_CS
        SROM_Delay
        
        xor     eax, eax
        out     dx, eax
        
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

;        DEBUGF 1,"SROM_Read_word at: %x\n", esi

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], CSR9

; enter access mode
        mov     eax, CSR9_SR + CSR9_RD
        out     dx , eax
        or      eax, CSR9_SROM_CS
        out     dx , eax

        ; TODO: change this hard-coded 6-bit stuff to use value from srom_getwidth
        
; send read command "110b" + address to read from
        and     esi, 111111b
        or      esi, 110b shl 6
        
        mov     ecx, 1 shl 9
  .loop_cmd:
        mov     eax, CSR9_SR + CSR9_RD + CSR9_SROM_CS
        test    esi, ecx
        jz      @f
        or      eax, CSR9_SROM_DI
       @@:
        out     dx , eax
        SROM_Delay
        or      eax, CSR9_SROM_CK
        out     dx , eax
        SROM_Delay
        
        shr     ecx, 1
        jnz     .loop_cmd

; read data from SROM

        xor     esi, esi
        mov     ecx, 17 ;;; TODO: figure out why 17, not 16
  .loop_read:
        
        mov     eax, CSR9_SR + CSR9_RD + CSR9_SROM_CS + CSR9_SROM_CK
        out     dx , eax
        SROM_Delay
        
        in      eax, dx
        and     eax, CSR9_SROM_DO
        shr     eax, 3
        shl     esi, 1
        or      esi, eax
        
        mov     eax, CSR9_SR + CSR9_RD + CSR9_SROM_CS
        out     dx , eax
        SROM_Delay
        
        dec     ecx
        jnz     .loop_read
        
        mov     eax, esi

;        DEBUGF 1,"%x\n", ax

        ret



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

MDIO_SHIFT_CLK          = 0x10000
MDIO_DATA_WRITE0        = 0x00000
MDIO_DATA_WRITE1        = 0x20000
MDIO_ENB                = 0x00000       ; Ignore the 0x02000 databook setting.
MDIO_ENB_IN             = 0x40000
MDIO_DATA_READ          = 0x80000

; MII transceiver control section.
; Read and write the MII registers using software-generated serial
; MDIO protocol.  See the MII specifications or DP83840A data sheet
; for details.

align 4
mdio_read:      ; phy_id:edx, location:esi

        DEBUGF  1,"mdio read, phy=%x, location=%x\n", edx, esi

        shl     edx, 5
        or      esi, edx
        or      esi, 0xf6 shl 10

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], CSR9

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

        mov     ecx, 32
  .loop:
        mov     eax, MDIO_ENB or MDIO_DATA_WRITE1
        out     dx, eax
        MDIO_Delay

        or      eax, MDIO_SHIFT_CLK
        out     dx, eax
        MDIO_Delay

        dec     ecx
        jnz     .loop


; Shift the read command bits out.

        mov     ecx, 1 shl 15
  .loop2:
        mov     eax, MDIO_ENB
        test    esi, ecx
        jz      @f
        or      eax, MDIO_DATA_WRITE1
       @@:
        out     dx, eax
        MDIO_Delay

        or      eax, MDIO_SHIFT_CLK
        out     dx, eax
        MDIO_Delay

        shr     ecx, 1
        jnz     .loop2


; Read the two transition, 16 data, and wire-idle bits.

        xor     esi, esi
        mov     ecx, 19
  .loop3:
        mov     eax, MDIO_ENB_IN
        out     dx, eax
        MDIO_Delay

        shl     esi, 1
        in      eax, dx
        test    eax, MDIO_DATA_READ
        jz      @f
        inc     esi
       @@:

        mov     eax, MDIO_ENB_IN or MDIO_SHIFT_CLK
        out     dx, eax
        MDIO_Delay

        dec     ecx
        jnz     .loop3

        shr     esi, 1
        movzx   eax, si

        DEBUGF  1,"data=%x\n", ax

        ret




align 4
mdio_write:     ;int phy_id: edx, int location: edi, int value: ax)

        DEBUGF  1,"mdio write, phy=%x, location=%x, data=%x\n", edx, edi, ax

        shl     edi, 18
        or      edi, 0x5002 shl 16
        shl     edx, 23
        or      edi, edx
        mov     di, ax

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], CSR9

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

        mov     ecx, 32
  .loop:
        mov     eax, MDIO_ENB or MDIO_DATA_WRITE1
        out     dx, eax
        MDIO_Delay

        or      eax, MDIO_SHIFT_CLK
        out     dx, eax
        MDIO_Delay

        dec     ecx
        jnz     .loop


; Shift the command bits out.

        mov     ecx, 1 shl 31
  .loop2:
        mov     eax, MDIO_ENB
        test    edi, ecx
        jz      @f
        or      eax, MDIO_DATA_WRITE1
       @@:
        out     dx, eax
        MDIO_Delay

        or      eax, MDIO_SHIFT_CLK
        out     dx, eax
        MDIO_Delay

        shr     ecx, 1
        jnz     .loop2


; Clear out extra bits.

        mov     ecx, 2
  .loop3:
        mov     eax, MDIO_ENB
        out     dx, eax
        MDIO_Delay

        or      eax, MDIO_SHIFT_CLK
        out     dx, eax
        MDIO_Delay

        dec     ecx
        jnz     .loop3

        ret






; End of code

data fixups
end data

include '../peimport.inc'

my_service    db 'DEC21X4X',0                    ; max 16 chars include zero

chiplist:
;   PCI id's , chip ,IO size, CSR7      , name  ,  flags
dd 0x00021011, DC21040,  128, 0x0001ebef, sz_040,  0
dd 0x00141011, DC21041,  128, 0x0001ebef, sz_041,  FLAG_HAS_MEDIA_TABLE
dd 0x00091011, DC21140,  128, 0x0001ebef, sz_140,  FLAG_HAS_MII or FLAG_HAS_MEDIA_TABLE or FLAG_CSR12_IN_SROM
dd 0x00191011, DC21143,  128, 0x0001ebef, sz_143,  FLAG_HAS_MII or FLAG_HAS_MEDIA_TABLE or FLAG_ALWAYS_CHECK_MII or FLAG_HAS_ACPI
dd 0x000211AD, LC82C168, 256, 0x0801fbff, sz_lite, FLAG_HAS_MII
dd 0x051210D9, MX98713,  128, 0x0001ebef, sz_m512, FLAG_HAS_MII or FLAG_HAS_MEDIA_TABLE
dd 0x053110D9, MX98715,  256, 0x0001ebef, sz_m513, FLAG_HAS_MEDIA_TABLE
dd 0x1400125B, MX98725,  128, 0x0001fbff, sz_asix, FLAG_HAS_MII or FLAG_HAS_MEDIA_TABLE or FLAG_CSR12_IN_SROM
dd 0

sz_040  db "Digital DC21040 Tulip", 0
sz_041  db "Digital DC21041 Tulip", 0
sz_140  db "Digital DS21140 Tulip", 0
sz_143  db "Digital DS21143 Tulip", 0
sz_lite db "Lite-On 82c168 PNIC", 0
sz_m512 db "Macronix 98713 PMAC", 0
sz_m513 db "Macronix 987x5 PMAC", 0
sz_asix db "ASIX AX88140", 0

include_debug_strings                           ; All data wich FDO uses will be included here

align 4
devices         dd 0
device_list     rd MAX_DEVICES                  ; This list contains all pointers to device structures the driver is handling