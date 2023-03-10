;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2021. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; This macro will prepend driver name to all debug output through DEBUGF macro
; The driver name is taken from my_service label

if defined my_service

        macro DEBUGF _level,_format, [args] {
        common DEBUGF _level, "%s: " # _format, my_service, args
        }

end if

include 'pci.inc'
include 'mii.inc'

; Kernel variables

        PAGESIZE        = 4096

; Network driver types

        NET_TYPE_ETH    = 1
        NET_TYPE_SLIP   = 2

; Link state

        ETH_LINK_DOWN           = 0             ; Link is down
        ETH_LINK_UNKNOWN        = 1b            ; There could be an active link
        ETH_LINK_FULL_DUPLEX    = 10b           ; full duplex flag

        ETH_LINK_SPEED_10M      = 100b          ; 10 Mbit
        ETH_LINK_SPEED_100M     = 1000b         ; 100 Mbit
        ETH_LINK_SPEED_1G       = 1100b         ; Gigabit

        ETH_LINK_SPEED_MASK     = 1100b

; Macro to easily set i/o addresses to access device.
; In the beginning of a procedure (or ofter edx may have been destroyed),
; always use set_io with offset 0 to reset the variables.

        LAST_IO = 0

macro   set_io  baseaddr, offset {

        if      offset = 0
        mov     edx, baseaddr
        else if offset = LAST_IO
        else
        add     edx, offset - LAST_IO
        end if

        LAST_IO = offset
}

; Macro to allocate a contiguous buffer in memory
; And initialise it to all zeros

; This macro will destroy eax, ecx and edi !

macro   allocate_and_clear dest, size, err {

; We need to allocate at least 8 pages, if we want a contiguous area in ram
        push    edx
    if (size < 8*4096) & (size > 4096)
        invoke  KernelAlloc, 8*4096
    else
        invoke  KernelAlloc, size
    end if
        pop     edx

        test    eax, eax
        jz      err
        mov     dest, eax
        mov     edi, eax                ; look at last part of code!

; Release the unused pages (if any)
    if (size < 8*4096) & (size > 4096)
        add     eax, (size/4096+1)*4096
        mov     ecx, 8-(size/4096+1)
        push    edx
        invoke  ReleasePages
        pop     edx
    end if

; Clear the allocated buffer
        mov     ecx, size/4             ; divide by 4 because of DWORD
        xor     eax, eax
        rep     stosd

     if (size - size/4*4)
        mov     ecx, size - size/4*4
        rep     stosb
     end if

}


struct  NET_DEVICE

        type            dd ?    ; Type field
        mtu             dd ?    ; Maximal Transmission Unit
        name            dd ?    ; Ptr to 0 terminated string

        unload          dd ?    ; Ptrs to driver functions
        reset           dd ?    ;
        transmit        dd ?    ;

        state           dd ?    ; link state (0 = no link)
        hwacc           dd ?    ; bitmask stating enabled HW accelerations (offload engines)

        bytes_tx        dq ?    ; Statistics, updated by the driver
        bytes_rx        dq ?    ;

        packets_tx      dd ?    ;
        packets_tx_err  dd ?    ; CRC errors, too long or too short frames
        packets_tx_drop dd ?    ;
        packets_tx_ovr  dd ?    ; FIFO overrun

        packets_rx      dd ?    ;
        packets_rx_err  dd ?    ; CRC errors, too long or too short frames
        packets_rx_drop dd ?    ;
        packets_rx_ovr  dd ?    ; FIFO overrun

ends


struct  NET_BUFF

        NextPtr         dd ?    ; pointer to next frame in list
        PrevPtr         dd ?    ; pointer to previous frame in list
        device          dd ?    ; ptr to NET_DEVICE structure
        type            dd ?    ; data type (e.g. Ethernet)
        length          dd ?    ; data length
        offset          dd ?    ; offset to actual data (24 bytes for default frame)
        data            rb 0

ends


struct  ETH_DEVICE      NET_DEVICE

        mac             dp ?
                        dw ?    ; qword alignment

ends