;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2012. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  ARP.INC                                                        ;;
;;                                                                 ;;
;;  Part of the tcp/ip network stack for KolibriOS                 ;;
;;                                                                 ;;
;;  Based on the work of [Johnny_B] and [smb]                      ;;
;;                                                                 ;;
;;    Written by hidnplayr@kolibrios.org                           ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June- 1991                               ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

ARP_NO_ENTRY            = 0
ARP_VALID_MAPPING       = 1
ARP_AWAITING_RESPONSE   = 2
ARP_RESPONSE_TIMEOUT    = 3

ARP_REQUEST_TTL         = 31          ; 20 s
ARP_ENTRY_TTL           = 937         ; 600 s
ARP_STATIC_ENTRY        = -1

ARP_REQ_OPCODE          = 0x0100      ; request
ARP_REP_OPCODE          = 0x0200      ; reply

ARP_TABLE_SIZE          = 20          ; Size of table

struct ARP_entry

       IP               dd ?
       MAC              dp ?
       Status           dw ?
       TTL              dw ?

ends

struct ARP_header

       HardwareType     dw ?
       ProtocolType     dw ?
       HardwareSize     db ?
       ProtocolSize     db ?
       Opcode           dw ?
       SenderMAC        dp ?
       SenderIP         dd ?
       TargetMAC        dp ?
       TargetIP         dd ?

ends

align 4
uglobal

        NumARP          dd ?

        ARP_table       rb ARP_TABLE_SIZE * sizeof.ARP_entry    ; TODO: separate ARP table and stats per interface

        ARP_PACKETS_TX  rd MAX_NET_DEVICES
        ARP_PACKETS_RX  rd MAX_NET_DEVICES


endg



;-----------------------------------------------------------------
;
; ARP_init
;
;  This function resets all ARP variables
;
;-----------------------------------------------------------------
macro ARP_init {

        xor     eax, eax
        mov     [NumARP], eax

        mov     edi, ARP_PACKETS_TX
        mov     ecx, 2*MAX_NET_DEVICES
        rep     stosd

}

;---------------------------------------------------------------------------
;
; ARP_decrease_entry_ttls
;
;---------------------------------------------------------------------------

macro ARP_decrease_entry_ttls {

local   .loop
local   .exit

; The TTL field is decremented every second, and is deleted when it reaches 0.
; It is refreshed every time a packet is received.
; If the TTL field is 0xFFFF it is a static entry and is never deleted.
; The status field can be the following values:
; 0x0000  entry not used
; 0x0001  entry holds a valid mapping
; 0x0002  entry contains an IP address, awaiting ARP response
; 0x0003  No response received to ARP request.
; The last status value is provided to allow the network layer to delete
; a packet that is queued awaiting an ARP response

        mov     ecx, [NumARP]
        test    ecx, ecx
        jz      .exit

        mov     esi, ARP_table
  .loop:
        cmp     [esi + ARP_entry.TTL], ARP_STATIC_ENTRY
        je      .next

        dec     [esi + ARP_entry.TTL]
        jz      .time_out

  .next:
        add     esi, sizeof.ARP_entry
        dec     ecx
        jnz     .loop
        jmp     .exit

  .time_out:
        cmp     [esi + ARP_entry.Status], ARP_AWAITING_RESPONSE
        je      .response_timeout

        push    esi ecx
        call    ARP_del_entry
        pop     ecx esi

        jmp     .next

  .response_timeout:
        mov     [esi + ARP_entry.Status], ARP_RESPONSE_TIMEOUT
        mov     [esi + ARP_entry.TTL], 10

        jmp     .next

  .exit:

}


;-----------------------------------------------------------------
;
; ARP_input
;
;  IN:  Pointer to buffer in [esp]
;       size of buffer in [esp+4]
;       packet size (without ethernet header) in ecx
;       packet ptr in edx
;  OUT: /
;
;-----------------------------------------------------------------
align 4
ARP_input:

        cmp     ecx, sizeof.ARP_header
        jb      .exit

;---------------------
; Handle Reply packets

        cmp     [edx + ARP_header.Opcode], ARP_REP_OPCODE
        jne     .maybe_request

        DEBUGF  1,"ARP_input: got reply packet from %u.%u.%u.%u\n",\
        [edx + ARP_header.SenderIP]:1, [edx + ARP_header.SenderIP + 1]:1,\
        [edx + ARP_header.SenderIP + 2]:1, [edx + ARP_header.SenderIP + 3]:1

        mov     ecx, [NumARP]
        test    ecx, ecx
        jz      .exit

        mov     eax, [edx + ARP_header.SenderIP]
        mov     esi, ARP_table
  .loop:
        cmp     [esi + ARP_entry.IP], eax
        je      .gotit
        add     esi, sizeof.ARP_entry
        dec     ecx
        jnz     .loop

        DEBUGF  1,"ARP_input: no matching entry found\n"
        jmp     .exit

  .gotit:
        DEBUGF  1,"ARP_input: found matching entry\n"

        cmp     [esi + ARP_entry.TTL], ARP_STATIC_ENTRY         ; if it is a static entry, dont touch it
        je      .exit

        DEBUGF  1,"ARP_input: updating entry\n"

        mov     [esi + ARP_entry.Status], ARP_VALID_MAPPING
        mov     [esi + ARP_entry.TTL], ARP_ENTRY_TTL

        mov     eax, dword [edx + ARP_header.SenderMAC]
        mov     dword [esi + ARP_entry.MAC], eax
        mov     cx, word [edx + ARP_header.SenderMAC + 4]
        mov     word [esi + ARP_entry.MAC + 4], cx

        jmp     .exit


;-----------------------
; Handle Request packets

  .maybe_request:
        cmp     [edx + ARP_header.Opcode], ARP_REQ_OPCODE
        jne     .exit

        call    NET_ptr_to_num
        cmp     edi, -1
        jz      .exit
        DEBUGF  1,"ARP_input: got request packet through device: %u\n", edi
        inc     [ARP_PACKETS_RX + 4*edi]

        mov     eax, [IP_LIST + 4*edi]
        cmp     eax, [edx + ARP_header.TargetIP]                ; Is it looking for my IP address?
        jne     .exit

        push    eax
        push    edi

; OK, it is a request for one of our MAC addresses.
; Build the frame and send it. We can reuse the buffer.  (faster then using ARP_create_packet)

        lea     esi, [edx + ARP_header.SenderMAC]
        lea     edi, [edx + ARP_header.TargetMAC]
        movsd                                                   ; Move Sender Mac to Dest MAC
        movsw                                                   ;
        movsd                                                   ; Move sender IP to Dest IP

        pop     esi
        mov     esi, [NET_DRV_LIST + 4*esi]
        lea     esi, [esi + ETH_DEVICE.mac]
        lea     edi, [edx + ARP_header.SenderMAC]
        movsd                                                   ; Copy MAC address from in MAC_LIST
        movsw                                                   ;
        pop     eax
        stosd                                                   ; Write our IP

        mov     [edx + ARP_header.Opcode], ARP_REP_OPCODE

; Now, Fill in ETHERNET header

        mov     edi, [esp]
        lea     esi, [edx + ARP_header.TargetMAC]
        movsd
        movsw
        lea     esi, [edx + ARP_header.SenderMAC]
        movsd
        movsw
;        mov     ax , ETHER_ARP                                 ; It's already there, I'm sure of it!
;        stosw

        DEBUGF  1,"ARP_input: Sending reply\n"

        call    [ebx + NET_DEVICE.transmit]
        ret

  .exit:
        call    kernel_free
        add     esp, 4                                          ; pop (balance stack)

        DEBUGF  1,"ARP_input: exiting\n"
        ret


;---------------------------------------------------------------------------
;
; ARP_output_request
;
; IN:  ip in eax
;      device in edi
; OUT: /
;
;---------------------------------------------------------------------------
align 4
ARP_output_request:

        push    eax                             ; DestIP
        pushd   [IP_LIST + edi]                 ; SenderIP

        DEBUGF  1,"ARP_output_request: ip=%u.%u.%u.%u\n",\
        [esp]:1, [esp + 1]:1, [esp + 2]:1, [esp + 3]:1

        mov     ebx, [NET_DRV_LIST + edi]       ; device ptr

        lea     eax, [ebx + ETH_DEVICE.mac]     ; local device mac
        mov     edx, ETH_BROADCAST              ; broadcast mac
        mov     ecx, sizeof.ARP_header
        mov     di, ETHER_ARP
        call    ETH_output
        jz      .exit

        mov     ecx, eax

        mov     [edi + ARP_header.HardwareType], 0x0100         ; Ethernet
        mov     [edi + ARP_header.ProtocolType], 0x0008         ; IP
        mov     [edi + ARP_header.HardwareSize], 6              ; MAC-addr length
        mov     [edi + ARP_header.ProtocolSize], 4              ; IP-addr length
        mov     [edi + ARP_header.Opcode], ARP_REQ_OPCODE       ; Request

        add     edi, ARP_header.SenderMAC

        lea     esi, [ebx + ETH_DEVICE.mac]     ; SenderMac
        movsw                                   ;
        movsd                                   ;
        pop     eax                             ; SenderIP
        stosd                                   ;

        mov     eax, -1                         ; DestMac
        stosd                                   ;
        stosw                                   ;
        pop     eax                             ; DestIP
        stosd                                   ;

        DEBUGF  1,"ARP_output_request: device=%x\n", ebx

        push    edx ecx
        call    [ebx + NET_DEVICE.transmit]
        ret

  .exit:
        add     esp, 4 + 4
        DEBUGF  1,"ARP_output_request: failed\n"
        sub     eax, eax
        ret


;-----------------------------------------------------------------
;
; ARP_add_entry (or update)
;
; IN:  esi = ptr to entry (can easily be made on the stack)
; OUT: eax = entry #, -1 on error
;
;-----------------------------------------------------------------      ; TODO: use a mutex
align 4
ARP_add_entry:

        DEBUGF  1,"ARP_add_entry: "

        mov     ecx, [NumARP]
        cmp     ecx, ARP_TABLE_SIZE                                     ; list full ?
        jae     .error

        xor     eax, eax
        mov     edi, ARP_table
        mov     ecx, [esi + ARP_entry.IP]
  .loop:
        cmp     [edi + ARP_entry.Status], ARP_NO_ENTRY                  ; is this slot empty?
        je      .add

        cmp     [edi + ARP_entry.IP], ecx                               ; if not, check if it doesnt collide
        jne     .maybe_next

        cmp     [edi + ARP_entry.TTL], ARP_STATIC_ENTRY                 ; ok, its the same IP, update it if not static
        jne     .add

  .maybe_next:                                                          ; try the next slot
        add     edi, sizeof.ARP_entry
        inc     eax
        cmp     eax, ARP_TABLE_SIZE
        jae     .error
        jmp     .loop

  .add:
        mov     ecx, sizeof.ARP_entry/2
        rep     movsw
        inc     [NumARP]
        DEBUGF  1,"entry=%u\n", eax

        ret

  .error:
        DEBUGF  1,"failed\n"
        mov     eax, -1
        ret


;-----------------------------------------------------------------
;
; ARP_del_entry
;
; IN:  esi = ptr to arp entry
; OUT: /
;
;-----------------------------------------------------------------
align 4
ARP_del_entry:

        DEBUGF 1,"ARP_del_entry: entry=%x entrys=%u\n", esi, [NumARP]
        DEBUGF 1,"ARP_del_entry: IP=%u.%u.%u.%u\n", \
        [esi + ARP_entry.IP]:1, [esi + ARP_entry.IP + 1]:1, [esi + ARP_entry.IP + 2]:1, [esi + ARP_entry.IP + 3]:1

        mov     ecx, ARP_table + (ARP_TABLE_SIZE - 1) * sizeof.ARP_entry
        sub     ecx, esi
        shr     ecx, 1

        mov     edi, esi
        add     esi, sizeof.ARP_entry
        rep     movsw

        xor     eax, eax
        mov     ecx, sizeof.ARP_entry/2
        rep     stosw

        dec     [NumARP]
        DEBUGF 1,"ARP_del_entry: success\n"

        ret





;-----------------------------------------------------------------
;
; ARP_IP_to_MAC
;
;  This function translates an IP address to a MAC address
;
;  IN:  eax = IPv4 address
;       edi = device number
;  OUT: eax = -1 on error, -2 means request send
;      else, ax = first two bytes of mac (high 16 bits of eax will be 0)
;       ebx = last four bytes of mac
;       edi = unchanged
;
;-----------------------------------------------------------------
align 4
ARP_IP_to_MAC:

        DEBUGF 1,"ARP_IP_to_MAC: %u.%u", al, ah
        rol     eax, 16
        DEBUGF 1,".%u.%u\n", al, ah
        rol     eax, 16

        cmp     eax, 0xffffffff
        je      .broadcast

;--------------------------------
; Try to find the IP in ARP_table

        mov     ecx, [NumARP]
        test    ecx, ecx
        jz      .not_in_list
        mov     esi, ARP_table + ARP_entry.IP
  .scan_loop:
        cmp     [esi], eax
        je      .found_it
        add     esi, sizeof.ARP_entry
        loop    .scan_loop

  .not_in_list:
        DEBUGF 1,"ARP_IP_to_MAC: preparing for ARP request\n"

;--------------------
; Send an ARP request

        push    eax edi                    ; save IP for ARP_output_request

; Now create the ARP entry
        pushw   ARP_REQUEST_TTL         ; TTL
        pushw   ARP_AWAITING_RESPONSE   ; status
        pushd   0                       ; mac
        pushw   0
        pushd   eax                     ; ip
        mov     esi, esp
        call    ARP_add_entry
        add     esp, sizeof.ARP_entry

        cmp     eax, -1
        je      .full

; And send a request
        pop     edi eax
        call    ARP_output_request      ; IP in eax
;; TODO: check if driver could transmit packet

        mov     eax, -2                 ; request send
        ret

  .found_it:
        DEBUGF  1,"ARP_IP_to_MAC: found IP\n"
        cmp     [esi + ARP_entry.Status], ARP_VALID_MAPPING
        jne     .invalid

        movzx   eax, word [esi + ARP_entry.MAC]
        mov     ebx, dword[esi + ARP_entry.MAC + 2]
        ret

  .invalid:
        DEBUGF  1,"ARP_IP_to_MAC: entry has no valid mapping!\n"
        mov     eax, -1
        ret

  .full:
        DEBUGF  1,"ARP_IP_to_MAC: table is full!\n"
        add     esp, 8
        mov     eax, -1
        ret

  .broadcast:
        mov     eax, 0x0000ffff
        mov     ebx, 0xffffffff
        ret


;-----------------------------------------------------------------
;
; ARP_API
;
; This function is called by system function 75
;
; IN:  subfunction number in bl
;      device number in bh
;      ecx, edx, .. depends on subfunction
;
; OUT:  ?
;
;-----------------------------------------------------------------
align 4
ARP_api:

        movzx   eax, bh
        shl     eax, 2

        and     ebx, 0xff
        cmp     ebx, .number
        ja      .error
        jmp     dword [.table + 4*ebx]

  .table:
        dd      .packets_tx     ; 0
        dd      .packets_rx     ; 1
        dd      .entries        ; 2
        dd      .read           ; 3
        dd      .write          ; 4
        dd      .remove         ; 5
  .number = ($ - .table) / 4 - 1

  .error:
        mov     eax, -1
        ret

  .packets_tx:
        mov     eax, [ARP_PACKETS_TX + eax]
        ret

  .packets_rx:
        mov     eax, [ARP_PACKETS_RX + eax]
        ret

  .entries:
        mov     eax, [NumARP]
        ret

  .read:
        cmp     ecx, [NumARP]
        jae     .error
        ; edi = pointer to buffer
        ; ecx = # entry
        imul    ecx, sizeof.ARP_entry
        add     ecx, ARP_table
        mov     esi, ecx
        mov     ecx, sizeof.ARP_entry/2
        rep     movsw

        xor     eax, eax
        ret

  .write:
        ; esi = pointer to buffer
        call    ARP_add_entry        ;out: eax = entry number, -1 on error
        ret

  .remove:
        ; ecx = # entry
        cmp     ecx, [NumARP]
        jae     .error
        imul    ecx, sizeof.ARP_entry
        lea     esi, [ARP_table + ecx]
        call    ARP_del_entry
        ret

