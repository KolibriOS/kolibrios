;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2014. All rights reserved.    ;;
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

; network driver types

        NET_TYPE_ETH    = 1
        NET_TYPE_SLIP   = 2

; link state

        ETH_LINK_DOWN   = 0             ; Link is down
        ETH_LINK_UNKOWN = 1b            ; There could be an active link
        ETH_LINK_FD     = 10b           ; full duplex flag
        ETH_LINK_10M    = 100b          ; 10 mbit
        ETH_LINK_100M   = 1000b         ; 100 mbit
        ETH_LINK_1G     = 10000b        ; gigabit


        LAST_IO = 0
macro   set_io addr {

        if      addr = 0
        mov     edx, [device.io_addr]
        else if addr = LAST_IO
        else
        add     edx, addr - LAST_IO
        end if

        LAST_IO = addr
}

macro   allocate_and_clear dest, size, err {

; We need to allocate at least 8 pages, if we want a continuous memory in ram
        push    edx
    if (size < 8*4096) & (size > 4096)
        stdcall KernelAlloc, 8*4096
    else
        stdcall KernelAlloc, size
    end if
        pop     edx

        test    eax, eax
        jz      err
        mov     dest, eax               ; Save the address to it into the device struct
        mov     edi, eax                ; look at last part of code!

; Release the unused pages (if any)
    if (size < 8*4096) & (size > 4096)
        add     eax, (size/4096+1)*4096
        mov     ecx, 8-(size/4096+1)
        push    edx
        call    ReleasePages
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


if used null_op
align 4
null_op:
        or      eax, -1
        ret

end if


macro   GetRealAddr {             ; input and output is eax

        push    ax
        call    GetPgAddr
        and     word[esp], PAGESIZE - 1
        or      ax, word[esp]
        inc     esp
        inc     esp

}

macro   NET_DEVICE {

        .type           dd ?    ; Type field
        .mtu            dd ?    ; Maximal Transmission Unit
        .name           dd ?    ; Ptr to 0 terminated string

        .unload         dd ?    ; Ptrs to driver functions
        .reset          dd ?    ;
        .transmit       dd ?    ;

        .bytes_tx       dq ?    ; Statistics, updated by the driver
        .bytes_rx       dq ?    ;
        .packets_tx     dd ?    ;
        .packets_rx     dd ?    ;

        .state          dd ?    ; link state (0 = no link)
        .hwacc          dd ?    ; bitmask stating enabled HW accelerations

        .end:
}


macro   ETH_DEVICE {
        NET_DEVICE

        .mac            dp ?
                        dw ?    ; qword alignment

}



macro   SLIP_DEVICE {
        NET_DEVICE

}