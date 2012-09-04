;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2008. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  ARP.INC                                                     ;;
;;                                                              ;;
;;  Address Resolution Protocol                                 ;;
;;                                                              ;;
;;  This file contains the following:                           ;;
;;   arp_table_manager - Manages an ARPTable                    ;;
;;   arp_request - Sends an ARP request on the ethernet         ;;
;;   arp_handler - Called when an ARP packet is received        ;;
;;                                                              ;;
;;  Changes history:                                            ;;
;;   22.09.2003 - [Mike Hibbett] : mikeh@oceanfree.net          ;;
;;   11.11.2006 - [Johnny_B] and [smb]                          ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


ARP_NO_ENTRY                equ  0
ARP_VALID_MAPPING           equ  1
ARP_AWAITING_RESPONSE       equ  2
ARP_RESPONSE_TIMEOUT        equ  3

struc ARP_ENTRY     ;=14 bytes
{  .IP       dd  ?  ;+00
   .MAC      dp  ?  ;+04
   .Status   dw  ?  ;+10
   .TTL      dw  ?  ;+12 : ( in seconds )
}

virtual at 0
  ARP_ENTRY ARP_ENTRY
end virtual

; The TTL field is decremented every second, and is deleted when it
; reaches 0. It is refreshed every time a packet is received
; If the TTL field is 0xFFFF it is a static entry and is never deleted
; The status field can be the following values:
; 0x0000  entry not used
; 0x0001  entry holds a valid mapping
; 0x0002  entry contains an IP address, awaiting ARP response
; 0x0003  No response received to ARP request.
; The last status value is provided to allow the network layer to delete
; a packet that is queued awaiting an ARP response


; The follow is the ARP Table.
; This table must be manually updated and the kernel recompilied if
; changes are made to it.
; Empty entries are filled with zeros

ARP_ENTRY_SIZE              equ     14          ; Number of bytes per entry
ARP_TABLE_SIZE              equ     20          ; Size of table
ARP_TABLE_ENTRIES           equ     0           ; Number of static entries in the table

;TO ADD A STATIC ENTRY, DONT FORGET, PUT "ARPTable" from "uglobal" to "iglobal"!!!
;AND ALSO - IP and MAC have net byte-order, BUT STATUS AND TTL HAVE A MIRROR BYTE-ORDER!!!
uglobal
  ARPTable:
;example, static entry ->  db  11,22,33,44, 0x11,0x22,0x33,0x44,0x55,0x66, 0x01,0x00, 0xFF,0xFF
  times ( ARP_TABLE_SIZE - ARP_TABLE_ENTRIES ) * ARP_ENTRY_SIZE  db 0
endg

iglobal
  NumARP:        dd    ARP_TABLE_ENTRIES
  ARPTable_ptr   dd    ARPTable   ;pointer to ARPTable
endg

ARP_REQ_OPCODE              equ     0x0100  ;request
ARP_REP_OPCODE              equ     0x0200  ;reply

struc ARP_PACKET
{  .HardwareType dw   ?  ;+00
   .ProtocolType dw   ?  ;+02
   .HardwareSize db   ?  ;+04
   .ProtocolSize db   ?  ;+05
   .Opcode       dw   ?  ;+06
   .SenderMAC    dp   ?  ;+08
   .SenderIP     dd   ?  ;+14
   .TargetMAC    dp   ?  ;+18
   .TargetIP     dd   ?  ;+24
}

virtual at 0
  ARP_PACKET ARP_PACKET
end virtual



;***************************************************************************
;   Function
;      arp_table_manager  [by Johnny_B]
;
;   Description
;     Does a most required operations with ARP-table
;  IN:
;   Operation: see Opcode's constants below
;       Index: Index of entry in the ARP-table
;       Extra: Extra parameter for some Opcodes
;  OUT:
;   EAX = Returned value depends on opcodes, more detailed see below
;
;***************************************************************************
;Opcode's constants
ARP_TABLE_ADD                 equ  1
ARP_TABLE_DEL                 equ  2
ARP_TABLE_GET                 equ  3
ARP_TABLE_GET_ENTRIES_NUMBER  equ  4
ARP_TABLE_IP_TO_MAC           equ  5
ARP_TABLE_TIMER               equ  6

;Index's constants
EXTRA_IS_ARP_PACKET_PTR  equ  0   ;if Extra contain pointer to ARP_PACKET
EXTRA_IS_ARP_ENTRY_PTR   equ  -1  ;if Extra contain pointer to ARP_ENTRY

align 4
proc arp_table_manager stdcall uses ebx esi edi ecx edx,\
    Opcode:DWORD,Index:DWORD,Extra:DWORD

    mov     ebx, dword[ARPTable_ptr]   ;ARPTable base
    mov     ecx, dword[NumARP]         ;ARP-entries counter

    mov     eax, dword[Opcode]
    cmp     eax, ARP_TABLE_TIMER
    je      .timer
    cmp     eax, ARP_TABLE_ADD
    je      .add
    cmp     eax, ARP_TABLE_DEL
    je      .del
    cmp     eax, ARP_TABLE_GET
    je      .get
    cmp     eax, ARP_TABLE_IP_TO_MAC
    je      .ip_to_mac
    cmp     eax, ARP_TABLE_GET_ENTRIES_NUMBER
    je      .get_entries_number
    jmp     .exit     ;if unknown opcode


;;BEGIN TIMER
;;Description: it must be callback every second. It is responsible for removing expired routes.
;;IN:   Operation: ARP_TABLE_TIMER
;;      Index: must be zero
;;      Extra: must be zero
;;OUT:
;;  EAX=not defined
;;
.timer:
    test    ecx, ecx
    jz      .exit    ;if NumARP=0 nothing to do
    sub     ecx, ARP_TABLE_ENTRIES  ;ecx=dynamic entries number
    jz      .exit    ;if NumARP=number of static entries then exit

    add     ebx, ARP_TABLE_ENTRIES*ARP_ENTRY_SIZE  ;ebx=dynamic entries base

  .timer_loop:
    movsx   esi, word [ebx + ARP_ENTRY.TTL]
    cmp     esi, 0xFFFFFFFF
    je      .timer_loop_end  ;if TTL==0xFFFF then it's static entry

    test    esi, esi
    jnz     .timer_loop_end_with_dec  ;if TTL!=0

    ; Ok, TTL is 0
    ;if Status==AWAITING_RESPONSE and TTL==0
    ;then we have to change it to ARP_RESPONSE_TIMEOUT
    cmp     word [ebx + ARP_ENTRY.Status], ARP_AWAITING_RESPONSE
    jne     @f

    mov     word [ebx + ARP_ENTRY.Status], ARP_RESPONSE_TIMEOUT
    mov     word [ebx + ARP_ENTRY.TTL], word 0x000A   ;10 sec
    jmp     .timer_loop_end

  @@:
    ;if TTL==0 and Status==VALID_MAPPING, we have to delete it
    ;if TTL==0 and Status==RESPONSE_TIMEOUT, delete too
    mov     esi, dword[NumARP]
    sub     esi, ecx          ;esi=index of entry, will be deleted
    stdcall arp_table_manager,ARP_TABLE_DEL,esi,0 ;opcode,index,extra
    jmp     .timer_loop_end


  .timer_loop_end_with_dec:
    dec     word [ebx + ARP_ENTRY.TTL]  ;decrease TTL
  .timer_loop_end:
    add     ebx, ARP_ENTRY_SIZE
    loop    .timer_loop

    jmp     .exit
;;END TIMER

;;BEGIN ADD
;;Description: it adds an entry in the table. If ARP-table already
;;             contains same IP, it will be updated.
;;IN:   Operation: ARP_TABLE_ADD
;;      Index: specifies what contains Extra-parameter
;;      Extra: if Index==EXTRA_IS_ARP_PACKET_PTR,
;;             then Extra contains pointer to ARP_PACKET,
;;             otherwise Extra contains pointer to ARP_ENTRY
;;OUT:
;;  EAX=index of entry, that has been added
;;
.add:

    sub     esp, ARP_ENTRY_SIZE   ;Allocate ARP_ENTRY_SIZE byte in stack

    mov     esi, [Extra]   ;pointer
    mov     edi, [Index]   ;opcode

    cmp     edi, EXTRA_IS_ARP_PACKET_PTR
    je      .arp_packet_to_entry ;if Extra contain ptr to ARP_PACKET and we have to form arp-entry
                                 ;else it contain ptr to arp-entry

    cld
          ; esi already has been loaded
    mov     edi, esp      ;ebx + eax=ARPTable_base + ARP-entry_base(where we will add)
    mov     ecx,ARP_ENTRY_SIZE/2  ;ARP_ENTRY_SIZE must be even number!!!
    rep     movsw    ;copy
    jmp     .search

  .arp_packet_to_entry:
    mov     edx, dword[esi + ARP_PACKET.SenderIP] ;esi=base of ARP_PACKET
    mov     [esp + ARP_ENTRY.IP], edx

    cld
    lea     esi, [esi + ARP_PACKET.SenderMAC]
    lea     edi, [esp + ARP_ENTRY.MAC]
    movsd
    movsw
    mov     word[esp + ARP_ENTRY.Status], ARP_VALID_MAPPING  ; specify the type - a valid entry
    mov     word[esp + ARP_ENTRY.TTL], 0x0E10    ; = 1 hour

  .search:
    mov     edx, dword[esp + ARP_ENTRY.IP]  ;edx=IP-address, which we'll search
    mov     ecx, dword[NumARP]              ;ecx=ARP-entries counter
    jecxz   .add_to_end                     ;if ARP-entries number == 0
    imul    eax, ecx, ARP_ENTRY_SIZE        ;eax=current table size(in bytes)
  @@:
    sub     eax, ARP_ENTRY_SIZE
    cmp     dword[ebx + eax + ARP_ENTRY.IP], edx
    loopnz  @b
    jz      .replace       ; found, replace existing entry, ptr to it is in eax

  .add_to_end:
    ;else add to end
    or      eax,-1    ;set eax=0xFFFFFFFF if adding is impossible
    mov     ecx, dword[NumARP]
    cmp     ecx, ARP_TABLE_SIZE
    je      .add_exit   ;if arp-entries number is equal to arp-table maxsize

    imul    eax, dword[NumARP], ARP_ENTRY_SIZE ;eax=ptr to end of ARPTable
    inc     dword [NumARP]    ;increase ARP-entries counter

  .replace:
    cld
    mov     esi, esp              ;esp=base of ARP-entry, that will be added
    lea     edi, [ebx + eax]      ;ebx + eax=ARPTable_base + ARP-entry_base(where we will add)
    mov     ecx,ARP_ENTRY_SIZE/2  ;ARP_ENTRY_SIZE must be even number!!!
    rep     movsw

    mov     ecx, ARP_ENTRY_SIZE
    xor     edx, edx  ;"div" takes operand from EDX:EAX
    div     ecx       ;eax=index of entry, which has been added

.add_exit:
    add     esp, ARP_ENTRY_SIZE   ;free stack
    jmp     .exit
;;END ADD

;;BEGIN DEL
;;Description: it deletes an entry in the table.
;;IN:   Operation: ARP_TABLE_DEL
;;      Index: index of entry, that should be deleted
;;      Extra: must be zero
;;OUT:
;;  EAX=not defined
;;
.del:
    mov     esi, [Index]
    imul    esi, ARP_ENTRY_SIZE

    mov     ecx, (ARP_TABLE_SIZE - 1) * ARP_ENTRY_SIZE
    sub     ecx, esi

    lea     edi, [ebx + esi]            ;edi=ptr to entry that should be deleted
    lea     esi, [edi + ARP_ENTRY_SIZE] ;esi=ptr to next entry

    shr     ecx,1      ;ecx/2 => ARP_ENTRY_SIZE MUST BE EVEN NUMBER!
    cld
    rep     movsw

    dec     dword[NumARP] ;decrease arp-entries counter
    jmp     .exit
;;END DEL

;;BEGIN GET
;;Description: it reads an entry of table into buffer.
;;IN:   Operation: ARP_TABLE_GET
;;      Index: index of entry, that should be read
;;      Extra: pointer to buffer for reading(size must be equal to ARP_ENTRY_SIZE)
;;OUT:
;;  EAX=not defined
;;
.get:
    mov     esi, [Index]
    imul    esi, ARP_ENTRY_SIZE   ;esi=ptr to required ARP_ENTRY
    mov     edi, [Extra]          ;edi=buffer for reading
    mov     ecx, ARP_ENTRY_SIZE/2 ; must be even number!!!
    cld
    rep     movsw
    jmp     .exit
;;END GET

;;BEGIN IP_TO_MAC
;;Description: it gets an IP from Index, scans each entry in the table and writes
;;             MAC, that relates to specified IP, into buffer specified in Extra.
;;             And if it cannot find an IP-address in the table, it does an ARP-request of that.
;;IN:   Operation: ARP_TABLE_IP_TO_MAC
;;      Index: IP that should be transformed into MAC
;;      Extra: pointer to buffer where will be written the MAC-address.
;;OUT:
;;  EAX=ARP table entry status code.
;;      If EAX==ARP_NO_ENTRY, IP isn't found in the table and we have sent the request.
;;      If EAX==ARP_AWAITING_RESPONSE, we wait the response from remote system.
;;      If EAX==ARP_RESPONSE_TIMEOUT, remote system not responds too long.
;;      If EAX==ARP_VALID_MAPPING, all is ok, we've got a true MAC.
;;
;;  If MAC will equal to a zero, in the buffer. It means, that IP-address was not yet
;;  resolved, or that doesn't exist. I recommend you, to do at most 3-5 calls of this
;;  function with 1sec delay. sure, only if it not return a valid MAC after a first call.
;;  
.ip_to_mac:

    xor     eax, eax
    mov     edi, dword[Extra]
    cld
    stosd
    stosw


    ; first, check destination IP to see if it is on 'this' network.
    ; The test is:
    ; if ( destIP & subnet_mask == stack_ip & subnet_mask )
    ;   destination is local
    ; else
    ;  destination is remote, so pass to gateway

    mov     eax, [Index]       ;eax=required IP
    mov     esi, eax
    and     esi, [subnet_mask]
    mov     ecx, [stack_ip]
    and     ecx, [subnet_mask]
    cmp     esi, ecx
    je      @f        ;if we and target IP are located in the same network
    mov     eax, [gateway_ip]
    mov     [Index], eax
  @@:

    cmp     dword[NumARP], 0
    je      .ip_to_mac_send_request ;if ARP-table not contain an entries, we have to request IP.
                                    ;EAX will be containing a zero, it's equal to ARP_NO_ENTRY

    mov     ecx, dword[NumARP]
    imul    esi, ecx, ARP_ENTRY_SIZE  ;esi=current ARP-table size

  @@:
    sub     esi, ARP_ENTRY_SIZE
    cmp     [ebx + esi], eax         ; ebx=ARPTable base
    loopnz  @b                       ; Return back if non match
    jnz     .ip_to_mac_send_request  ; and request IP->MAC if none found in the table

    ; Return the entry status in eax
    movzx   eax, word[ebx + esi + ARP_ENTRY.Status]

    ; esi holds index
    cld
    lea     esi, [ebx + esi + ARP_ENTRY.MAC]
    mov     edi, [Extra]   ;edi=ptr to buffer for write MAC
    movsd
    movsw
    jmp     .exit

  .ip_to_mac_send_request:
    stdcall arp_request,[Index],stack_ip,node_addr  ;TargetIP,SenderIP_ptr,SenderMAC_ptr
    mov     eax, ARP_NO_ENTRY
    jmp     .exit

;;END IP_TO_MAC

;;BEGIN GET_ENTRIES_NUMBER
;;Description: returns an ARP-entries number in the ARPTable
;;IN:   Operation: ARP_TABLE_GET_ENTRIES_NUMBER
;;      Index: must be zero
;;      Extra: must be zero
;;OUT:
;;  EAX=ARP-entries number in the ARPTable
  .get_entries_number:
    mov     eax, dword[NumARP]
    jmp     .exit
;;END GET_ENTRIES_NUMBER

.exit:
    ret
endp


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

    stdcall arp_table_manager,ARP_TABLE_ADD,EXTRA_IS_ARP_PACKET_PTR,ETH_FRAME.Data + ARP_PACKET

    inc     dword[arp_rx_count] ;increase ARP-packets counter

    cmp     word[ETH_FRAME.Data + ARP_PACKET.Opcode], ARP_REQ_OPCODE  ; Is this a request packet?
    jne     .exit            ; No - so exit

    mov     eax, [stack_ip] 
    cmp     eax, dword[ETH_FRAME.Data + ARP_PACKET.TargetIP]         ; Is it looking for my IP address?
    jne     .exit            ; No - so quit now

    ; OK, it is a request for my MAC address. Build the frame and send it
    ; We can reuse the packet.

    mov     word[ETH_FRAME.Data + ARP_PACKET.Opcode], ARP_REP_OPCODE

    cld
    mov     esi, ETH_FRAME.Data + ARP_PACKET.SenderMAC
    mov     edi, ETH_FRAME.Data + ARP_PACKET.TargetMAC
    movsd
    movsw

    mov     esi, ETH_FRAME.Data + ARP_PACKET.SenderIP
    mov     edi, ETH_FRAME.Data + ARP_PACKET.TargetIP
    movsd

    mov     esi, node_addr
    mov     edi, ETH_FRAME.Data + ARP_PACKET.SenderMAC
    movsd
    movsw

    mov     esi, stack_ip
    mov     edi, ETH_FRAME.Data + ARP_PACKET.SenderIP
    movsd

    ; Now, send it!
    mov     edi, ETH_FRAME.Data + ARP_PACKET.TargetMAC   ;ptr to destination MAC address
    mov     bx, ETHER_ARP               ;type of protocol
    mov     ecx, 28                     ;data size
    mov     esi, ETH_FRAME.Data + ARP_PACKET             ;ptr to data
    push    ebp
    call    dword [drvr_transmit]       ;transmit packet
    pop     ebp

  .exit:
    ret


;***************************************************************************
;   Function
;      arp_request  [by Johnny_B]
;
;   Description
;      Sends an ARP request on the ethernet
;   IN:
;     TargetIP      : requested IP address
;     SenderIP_ptr  : POINTER to sender's IP address(our system's address)
;     SenderMAC_ptr : POINTER to sender's MAC address(our system's address)
;   OUT:
;     EAX=0 (if all is ok), otherwise EAX is not defined
;
;      EBX,ESI,EDI will be saved
;
;***************************************************************************
proc arp_request stdcall uses ebx esi edi,\
    TargetIP:DWORD, SenderIP_ptr:DWORD, SenderMAC_ptr:DWORD

    inc     dword[arp_tx_count]  ; increase counter

    sub     esp, 28  ; allocate memory for ARP_PACKET

    mov     word[esp + ARP_PACKET.HardwareType],0x0100 ;Ethernet
    mov     word[esp + ARP_PACKET.ProtocolType],0x0008 ;IP
    mov     byte[esp + ARP_PACKET.HardwareSize],0x06   ;MAC-addr length
    mov     byte[esp + ARP_PACKET.ProtocolSize],0x04   ;IP-addr length
    mov     word[esp + ARP_PACKET.Opcode],0x0100       ;Request

    cld
    mov     esi,[SenderMAC_ptr]
    lea     edi,[esp + ARP_PACKET.SenderMAC]       ;Our MAC-addr
    movsd
    movsw

    mov     esi,[SenderIP_ptr]
    lea     edi,[esp + ARP_PACKET.SenderIP]        ;Our IP-addr
    movsd

    xor     eax, eax
    lea     edi, [esp + ARP_PACKET.TargetMAC]      ;Required MAC-addr(zeroed)
    stosd
    stosw

    mov     esi, dword[TargetIP]
    mov     dword[esp + ARP_PACKET.TargetIP],esi   ;Required IP-addr(we get it as function parameter)

    ; Now, send it!
    mov     edi, broadcast_add     ; Pointer to 48 bit destination address
    mov     bx, ETHER_ARP          ; Type of packet
    mov     ecx, 28                ; size of packet
    lea     esi, [esp + ARP_PACKET]; pointer to packet data
    push    ebp
    call    dword [drvr_transmit]  ; Call the drivers transmit function
    pop     ebp

    add     esp, 28  ; free memory, allocated before for ARP_PACKET

    ; Add an entry in the ARP table, awaiting response
    sub     esp, ARP_ENTRY_SIZE    ;allocate memory for ARP-entry

    mov     esi, dword[TargetIP]
    mov     dword[esp + ARP_ENTRY.IP],esi

    lea     edi, [esp + ARP_ENTRY.MAC]
    xor     eax, eax
    stosd
    stosw

    mov     word[esp + ARP_ENTRY.Status], ARP_AWAITING_RESPONSE
    mov     word[esp + ARP_ENTRY.TTL], 0x000A  ; 10 seconds

    stdcall arp_table_manager,ARP_TABLE_ADD,EXTRA_IS_ARP_ENTRY_PTR,esp
    add     esp, ARP_ENTRY_SIZE  ; free memory

.exit:
    ret
endp
