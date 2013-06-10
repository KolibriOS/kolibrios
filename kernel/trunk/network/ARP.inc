;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved.    ;;
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

$Revision: 3386 $

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

struct  ARP_entry

        IP              dd ?
        MAC             dp ?
        Status          dw ?
        TTL             dw ?

ends

struct  ARP_header

        HardwareType    dw ?
        ProtocolType    dw ?
        HardwareSize    db ?
        ProtocolSize    db ?
        Opcode          dw ?
        SenderMAC       dp ?
        SenderIP        dd ?
        TargetMAC       dp ?
        TargetIP        dd ?

ends

align 4
uglobal


        ARP_table       rb NET_DEVICES_MAX*(ARP_TABLE_SIZE * sizeof.ARP_entry)

        ARP_entries_num rd NET_DEVICES_MAX
        ARP_PACKETS_TX  rd NET_DEVICES_MAX
        ARP_PACKETS_RX  rd NET_DEVICES_MAX
        ARP_CONFLICTS   rd NET_DEVICES_MAX


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
        mov     edi, ARP_entries_num
        mov     ecx, 4*NET_DEVICES_MAX
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

        xor     edi, edi
  .loop_outer:
        mov     ecx, [ARP_entries_num + 4*edi]
        test    ecx, ecx
        jz      .exit

        mov     esi, (ARP_TABLE_SIZE * sizeof.ARP_entry)
        imul    esi, edi
        add     esi, ARP_table
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

        push    esi edi ecx
        call    ARP_del_entry
        pop     ecx edi esi

        jmp     .next

  .response_timeout:
        mov     [esi + ARP_entry.Status], ARP_RESPONSE_TIMEOUT
        mov     [esi + ARP_entry.TTL], 10

        jmp     .next

  .exit:
        inc     edi
        cmp     edi, NET_DEVICES_MAX
        jb      .loop_outer

}


;-----------------------------------------------------------------
;
; ARP_input
;
;  IN:  Pointer to buffer in [esp]
;       size of buffer in [esp+4]
;       packet size (without ethernet header) in ecx
;       packet ptr in edx
;       device ptr in ebx
;  OUT: /
;
;-----------------------------------------------------------------
align 4
ARP_input:

;-----------------------------------------
; Check validity and print some debug info

        cmp     ecx, sizeof.ARP_header
        jb      .exit

        call    NET_ptr_to_num
        cmp     edi, -1
        jz      .exit

        inc     [ARP_PACKETS_RX + 4*edi]        ; update stats

        DEBUGF  DEBUG_NETWORK_VERBOSE, "ARP_input: got packet from %u.%u.%u.%u through device %u\n",\
        [edx + ARP_header.SenderIP]:1, [edx + ARP_header.SenderIP + 1]:1,\
        [edx + ARP_header.SenderIP + 2]:1, [edx + ARP_header.SenderIP + 3]:1, edi

;------------------------------
; First, check for IP collision

        mov     eax, [edx + ARP_header.SenderIP]
        cmp     eax, [IP_LIST + 4*edi]
        je      .collision

;---------------------
; Handle reply packets

        cmp     [edx + ARP_header.Opcode], ARP_REP_OPCODE
        jne     .maybe_request

        DEBUGF  DEBUG_NETWORK_VERBOSE, "ARP_input: It's a reply\n"

        mov     ecx, [ARP_entries_num + 4*edi]
        test    ecx, ecx
        jz      .exit

        mov     esi, (ARP_TABLE_SIZE * sizeof.ARP_entry)
        imul    esi, edi
        add     esi, ARP_table
  .loop:
        cmp     [esi + ARP_entry.IP], eax
        je      .gotit
        add     esi, sizeof.ARP_entry
        dec     ecx
        jnz     .loop

        DEBUGF  DEBUG_NETWORK_VERBOSE, "ARP_input: no matching entry found\n"
        jmp     .exit

  .gotit:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "ARP_input: found matching entry\n"

        cmp     [esi + ARP_entry.TTL], ARP_STATIC_ENTRY         ; if it is a static entry, dont touch it
        je      .exit

        DEBUGF  DEBUG_NETWORK_VERBOSE, "ARP_input: updating entry\n"

        mov     [esi + ARP_entry.Status], ARP_VALID_MAPPING
        mov     [esi + ARP_entry.TTL], ARP_ENTRY_TTL

        mov     eax, dword [edx + ARP_header.SenderMAC]
        mov     dword [esi + ARP_entry.MAC], eax
        mov     cx, word [edx + ARP_header.SenderMAC + 4]
        mov     word [esi + ARP_entry.MAC + 4], cx

        jmp     .exit

;-----------------------
; Handle request packets

  .maybe_request:
        cmp     [edx + ARP_header.Opcode], ARP_REQ_OPCODE
        jne     .exit

        DEBUGF  DEBUG_NETWORK_VERBOSE, "ARP_input: its a request\n"

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

        DEBUGF  DEBUG_NETWORK_VERBOSE, "ARP_input: Sending reply\n"

        call    [ebx + NET_DEVICE.transmit]
        ret

  .collision:
        inc     [ARP_CONFLICTS + 4*edi]
        DEBUGF  DEBUG_NETWORK_VERBOSE, "ARP_input: IP address conflict detected!\n"

  .exit:
        call    kernel_free
        add     esp, 4                                          ; pop (balance stack)

        DEBUGF  DEBUG_NETWORK_VERBOSE, "ARP_input: exiting\n"
        ret


;---------------------------------------------------------------------------
;
; ARP_output_request
;
; IN:   ebx = device ptr
;       eax = IP
; OUT: /
;       scratched: probably everything
;
;---------------------------------------------------------------------------
align 4
ARP_output_request:

        push    eax

        DEBUGF  DEBUG_NETWORK_VERBOSE, "ARP_output_request: ip=%u.%u.%u.%u device=0x%x\n",\
        [esp]:1, [esp + 1]:1, [esp + 2]:1, [esp + 3]:1, ebx

        lea     eax, [ebx + ETH_DEVICE.mac]     ; local device mac
        mov     edx, ETH_BROADCAST              ; broadcast mac
        mov     ecx, sizeof.ARP_header
        mov     di, ETHER_PROTO_ARP
        call    ETH_output
        jz      .exit

        mov     [edi + ARP_header.HardwareType], 0x0100         ; Ethernet
        mov     [edi + ARP_header.ProtocolType], 0x0008         ; IP
        mov     [edi + ARP_header.HardwareSize], 6              ; MAC-addr length
        mov     [edi + ARP_header.ProtocolSize], 4              ; IP-addr length
        mov     [edi + ARP_header.Opcode], ARP_REQ_OPCODE       ; Request

        add     edi, ARP_header.SenderMAC
        lea     esi, [ebx + ETH_DEVICE.mac]     ; SenderMac
        movsw                                   ;
        movsd                                   ;

;        mov     esi, [ebx + NET_DEVICE.number]
        xor     esi, esi        ;;;; FIXME
        inc     esi ;;;;;;;;;
        inc     [ARP_PACKETS_TX + 4*esi]        ; assume we will succeed
        lea     esi, [IP_LIST + 4*esi]          ; SenderIP
        movsd

        mov     esi, ETH_BROADCAST              ; DestMac
        movsw                                   ;
        movsd                                   ;
        popd    [edi]                           ; DestIP

        push    edx eax
        call    [ebx + NET_DEVICE.transmit]
        ret

  .exit:
        add     esp, 4
        DEBUGF  DEBUG_NETWORK_ERROR, "ARP_output_request: send failed\n"
        ret


;-----------------------------------------------------------------
;
; ARP_add_entry (or update)
;
; IN:  esi = ptr to entry (can easily be made on the stack)
;      edi = device num*4
; OUT: eax = entry #, -1 on error
;      esi = ptr to newly created entry
;
;-----------------------------------------------------------------      ; TODO: use a mutex
align 4
ARP_add_entry:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "ARP_add_entry: device=%u\n", edi

        mov     ecx, [ARP_entries_num + edi]
        cmp     ecx, ARP_TABLE_SIZE                                     ; list full ?
        jae     .full

; From this point on, we can only fail if IP has a static entry, or if table is corrupt.

        inc     [ARP_entries_num + edi]                                 ; assume we will succeed

        push    edi
        xor     ecx, ecx
        imul    edi, ARP_TABLE_SIZE*sizeof.ARP_entry/4
        add     edi, ARP_table
        mov     eax, [esi + ARP_entry.IP]
  .loop:
        cmp     [edi + ARP_entry.Status], ARP_NO_ENTRY                  ; is this slot empty?
        je      .add

        cmp     [edi + ARP_entry.IP], eax                               ; if not, check if it doesnt collide
        jne     .maybe_next

        cmp     [edi + ARP_entry.TTL], ARP_STATIC_ENTRY                 ; ok, its the same IP, update it if not static
        jne     .add

        DEBUGF  DEBUG_NETWORK_ERROR, "ARP_add_entry: failed, IP already has a static entry\n"
        jmp     .error

  .maybe_next:                                                          ; try the next slot
        add     edi, sizeof.ARP_entry
        inc     ecx
        cmp     ecx, ARP_TABLE_SIZE
        jb      .loop

  .add:
        push    ecx
        mov     ecx, sizeof.ARP_entry/2
        rep     movsw
        pop     ecx
        lea     esi, [edi - sizeof.ARP_entry]
        pop     edi
        DEBUGF  DEBUG_NETWORK_VERBOSE, "ARP_add_entry: entry=%u\n", ecx

        ret

  .error:
        pop     edi
        dec     [ARP_entries_num + edi]
        DEBUGF  DEBUG_NETWORK_ERROR, "ARP_add_entry_failed\n"
  .full:
        mov     eax, -1
        ret


;-----------------------------------------------------------------
;
; ARP_del_entry
;
; IN:   esi = ptr to arp entry
;       edi = device number
; OUT:  /
;
;-----------------------------------------------------------------
align 4
ARP_del_entry:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "ARP_del_entry: entry=%x entrys=%u\n", esi, [ARP_entries_num + 4*edi]
        DEBUGF  DEBUG_NETWORK_VERBOSE, "ARP_del_entry: IP=%u.%u.%u.%u\n", \
        [esi + ARP_entry.IP]:1, [esi + ARP_entry.IP + 1]:1, [esi + ARP_entry.IP + 2]:1, [esi + ARP_entry.IP + 3]:1

        push    edi
        imul    edi, (ARP_TABLE_SIZE) * sizeof.ARP_entry
        lea     ecx, [ARP_table + (ARP_TABLE_SIZE - 1) * sizeof.ARP_entry + edi]
        sub     ecx, esi
        shr     ecx, 1

; move all trailing entries, sizeof.ARP_entry bytes to left.
        mov     edi, esi
        add     esi, sizeof.ARP_entry
        rep     movsw

; now add an empty entry to the end (erasing previous one)
        xor     eax, eax
        mov     ecx, sizeof.ARP_entry/2
        rep     stosw

        pop     edi
        dec     [ARP_entries_num + 4*edi]
        DEBUGF  DEBUG_NETWORK_VERBOSE, "ARP_del_entry: success\n"

        ret





;-----------------------------------------------------------------
;
; ARP_IP_to_MAC
;
;  This function translates an IP address to a MAC address
;
;  IN:  eax = IPv4 address
;       edi = device number * 4
;  OUT: eax = -1 on error, -2 means request send
;      else, ax = first two bytes of mac (high 16 bits of eax will be 0)
;       ebx = last four bytes of mac
;       edi = unchanged
;
;-----------------------------------------------------------------
align 4
ARP_IP_to_MAC:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "ARP_IP_to_MAC: %u.%u", al, ah
        rol     eax, 16
        DEBUGF  DEBUG_NETWORK_VERBOSE, ".%u.%u device*4: %u\n", al, ah, edi
        rol     eax, 16

        cmp     eax, 0xffffffff
        je      .broadcast

;--------------------------------
; Try to find the IP in ARP_table

        mov     ecx, [ARP_entries_num + edi]
        test    ecx, ecx
        jz      .not_in_list
        mov     esi, edi
        imul    esi, (sizeof.ARP_entry * ARP_TABLE_SIZE)/4
        add     esi, ARP_table + ARP_entry.IP
  .scan_loop:
        cmp     [esi], eax
        je      .found_it
        add     esi, sizeof.ARP_entry
        dec     ecx
        jnz     .scan_loop

  .not_in_list:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "ARP_IP_to_MAC: preparing for ARP request\n"

        push    eax edi                 ; save IP for ARP_output_request
; Now craft the ARP entry on the stack
        pushw   ARP_REQUEST_TTL         ; TTL
        pushw   ARP_AWAITING_RESPONSE   ; status
        pushd   0                       ; mac
        pushw   0
        pushd   eax                     ; ip
        mov     esi, esp

; Add it to the list
        call    ARP_add_entry

; Delete the temporary entry
        add     esp, sizeof.ARP_entry   ; clear the entry from stack

; If we could not add it to the list, give up
        cmp     eax, -1                 ; did ARP_add_entry fail?
        je      .full

;-----------------------------------------------
; At this point, we got an ARP entry in the list

; Now send a request packet on the network
        pop     edi eax                 ; IP in eax, device number in ebx, for ARP_output_request

        push    esi edi
        mov     ebx, [NET_DRV_LIST + edi]
        call    ARP_output_request
        pop     edi esi
  .found_it:
        cmp     [esi + ARP_entry.Status], ARP_VALID_MAPPING             ; Does it have a MAC assigned?
        je      .valid

if ARP_BLOCK

        cmp     [esi + ARP_entry.Status], ARP_AWAITING_RESPONSE         ; Are we waiting for reply from remote end?
        jne     .give_up
        push    esi
        mov     esi, 10                 ; wait 10 ms
        call    delay_ms
        pop     esi
        jmp     .found_it               ; now check again

else

        jmp     .give_up

end if

  .valid:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "ARP_IP_to_MAC: found MAC\n"
        movzx   eax, word[esi + ARP_entry.MAC]
        mov     ebx, dword[esi + ARP_entry.MAC + 2]
        ret

  .full:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "ARP_IP_to_MAC: table is full!\n"
        add     esp, 8
  .give_up:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "ARP_IP_to_MAC: entry has no valid mapping!\n"
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
; This function is called by system function 76
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
        dd      .send_announce  ; 6
        dd      .conflicts      ; 7
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

  .conflicts:
        mov     eax, [ARP_CONFLICTS + eax]
        ret

  .entries:
        mov     eax, [ARP_entries_num + eax]
        ret

  .read:
        cmp     ecx, [ARP_entries_num + eax]
        jae     .error
        shr     eax, 2
        imul    eax, sizeof.ARP_entry*ARP_TABLE_SIZE
        add     eax, ARP_table
        ; edi = pointer to buffer
        ; ecx = # entry
        imul    ecx, sizeof.ARP_entry
        lea     esi, [eax + ecx]
        mov     ecx, sizeof.ARP_entry/2
        rep     movsw

        xor     eax, eax
        ret

  .write:
        ; esi = pointer to buffer
        mov     edi, eax
        call    ARP_add_entry           ; out: eax = entry number, -1 on error
        ret

  .remove:
        ; ecx = # entry
        cmp     ecx, [ARP_entries_num + eax]
        jae     .error
        imul    ecx, sizeof.ARP_entry
        lea     esi, [ARP_table + ecx]
        mov     edi, eax
        shr     edi, 2
        call    ARP_del_entry
        ret

  .send_announce:
        mov     ebx, [NET_DRV_LIST + eax]
        mov     eax, [IP_LIST + eax]
        call    ARP_output_request      ; now send a gratuitous ARP
        ret

