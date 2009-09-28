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

$Revision: 983 $


ARP_NO_ENTRY		equ  0
ARP_VALID_MAPPING	equ  1
ARP_AWAITING_RESPONSE	equ  2
ARP_RESPONSE_TIMEOUT	equ  3

ETHER_ARP		equ  0x0608

ARP_REQ_OPCODE		equ  0x0100  ; request
ARP_REP_OPCODE		equ  0x0200  ; reply

ARP_TABLE_SIZE		equ  20      ; Size of table

struct ARP_ENTRY
       .IP		dd  ?
       .MAC		dp  ?
       .Status		dw  ?
       .TTL		dw  ?  ; in seconds
       .size:
ends

struct ARP_Packet
       .HardwareType	dw  ?
       .ProtocolType	dw  ?
       .HardwareSize	db  ?
       .ProtocolSize	db  ?
       .Opcode		dw  ?
       .SenderMAC	dp  ?
       .SenderIP	dd  ?
       .TargetMAC	dp  ?
       .TargetIP	dd  ?
ends


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

align 4
uglobal

	NumARP		dd ?
	ARPTable	rb ARP_ENTRY.size * ARP_TABLE_SIZE

	ARP_PACKETS_TX	rd  MAX_NET_DEVICES
	ARP_PACKETS_RX	rd  MAX_NET_DEVICES


endg




ARP_init:

	xor	eax, eax

	mov	[NumARP], eax

	mov	edi, ARP_PACKETS_TX
	mov	ecx, 2*MAX_NET_DEVICES
	rep	stosd

	ret




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
ARP_TABLE_ADD		      equ  1
ARP_TABLE_DEL		      equ  2
ARP_TABLE_GET		      equ  3
ARP_TABLE_GET_ENTRIES_NUMBER  equ  4
ARP_TABLE_IP_TO_MAC	      equ  5
ARP_TABLE_TIMER 	      equ  6

;Index's constants
EXTRA_IS_ARP_PACKET_PTR  equ  0   ;if Extra contain pointer to ARP_Packet
EXTRA_IS_ARP_ENTRY_PTR	 equ  -1  ;if Extra contain pointer to ARP_ENTRY

align 4
proc arp_table_manager stdcall uses ebx esi edi ecx edx, Opcode:DWORD,Index:DWORD,Extra:DWORD

    mov     ebx, ARPTable  ;ARPTable base
    mov     ecx, dword[NumARP]	       ;ARP-entries counter

    mov     eax, dword[Opcode]

    cmp     eax, ARP_TABLE_TIMER
    je	    .timer

    DEBUGF 1,"ARP table manager opcode:%u numARP:%u\n",eax,ecx

    cmp     eax, ARP_TABLE_ADD
    je	    .add
    cmp     eax, ARP_TABLE_DEL
    je	    .del
    cmp     eax, ARP_TABLE_GET
    je	    .get
    cmp     eax, ARP_TABLE_IP_TO_MAC
    je	    .ip_to_mac
    cmp     eax, ARP_TABLE_GET_ENTRIES_NUMBER
    je	    .get_entries_number
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
    jz	    .exit    ;if NumARP=0 nothing to do
;    sub     ecx, ARP_TABLE_ENTRIES  ;ecx=dynamic entries number
;    jz      .exit    ;if NumARP=number of static entries then exit

;    add     ebx, ARP_TABLE_ENTRIES*ARP_ENTRY_SIZE  ;ebx=dynamic entries base

  .timer_loop:
    movsx   esi, word [ebx + ARP_ENTRY.TTL]
    cmp     esi, 0xFFFFFFFF
    je	    .timer_loop_end  ;if TTL==0xFFFF then it's static entry

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
    sub     esi, ecx	      ;esi=index of entry, will be deleted
    stdcall arp_table_manager,ARP_TABLE_DEL,esi,0 ;opcode,index,extra
    jmp     .timer_loop_end


  .timer_loop_end_with_dec:
    dec     word [ebx + ARP_ENTRY.TTL]	;decrease TTL
  .timer_loop_end:
    add     ebx, ARP_ENTRY.size
    loop    .timer_loop

    jmp     .exit
;;END TIMER

;;BEGIN ADD
;;Description: it adds an entry in the table. If ARP-table already
;;             contains same IP, it will be updated.
;;IN:   Operation: ARP_TABLE_ADD
;;      Index: specifies what contains Extra-parameter
;;      Extra: if Index==EXTRA_IS_ARP_Packet_PTR,
;;             then Extra contains pointer to ARP_Packet,
;;             otherwise Extra contains pointer to ARP_ENTRY
;;OUT:
;;  EAX=index of entry, that has been added
;;
.add:

    DEBUGF 1,"1"

    sub     esp, ARP_ENTRY.size   ;Allocate ARP_ENTRY_SIZE byte in stack

    mov     esi, [Extra]   ;pointer
    mov     edi, [Index]   ;opcode

    cmp     edi, EXTRA_IS_ARP_PACKET_PTR
    je	    .ARP_Packet_to_entry ;if Extra contain ptr to ARP_Packet and we have to form arp-entry
				 ;else it contain ptr to arp-entry

    DEBUGF 1,"2"

    cld
	  ; esi already has been loaded
    mov     edi, esp	  ;ebx + eax=ARPTable_base + ARP-entry_base(where we will add)
    mov     ecx,ARP_ENTRY.size/2  ;ARP_ENTRY_SIZE must be even number!!!
    rep     movsw    ;copy
    jmp     .search

  .ARP_Packet_to_entry:

    DEBUGF 1,"3"
    mov     edx, dword[esi + ARP_Packet.SenderIP] ;esi=base of ARP_Packet
    mov     [esp + ARP_ENTRY.IP], edx

    cld
    lea     esi, [esi + ARP_Packet.SenderMAC]
    lea     edi, [esp + ARP_ENTRY.MAC]
    movsd
    movsw
    mov     word[esp + ARP_ENTRY.Status], ARP_VALID_MAPPING  ; specify the type - a valid entry
    mov     word[esp + ARP_ENTRY.TTL], 0x0E10	 ; = 1 hour

  .search:

    DEBUGF 1,"4"
    mov     edx, dword[esp + ARP_ENTRY.IP]  ;edx=IP-address, which we'll search
    mov     ecx, dword[NumARP]		    ;ecx=ARP-entries counter
    jecxz   .add_to_end 		    ;if ARP-entries number == 0
    imul    eax, ecx, ARP_ENTRY.size	    ;eax=current table size(in bytes)
  @@:
    sub     eax, ARP_ENTRY.size
    cmp     dword[ebx + eax + ARP_ENTRY.IP], edx
    loopnz  @b
;    jz      .replace       ; found, replace existing entry, ptr to it is in eax

  .add_to_end:
;
;    DEBUGF 1,"5\n"
;    ;else add to end
;    or      eax,-1    ;set eax=0xFFFFFFFF if adding is impossible
;    mov     ecx, dword[NumARP]
;    cmp     ecx, ARP_TABLE_SIZE
;    je      .add_exit   ;if arp-entries number is equal to arp-table maxsize

;    imul    eax, dword[NumARP], ARP_ENTRY.size ;eax=ptr to end of ARPTable
;    inc     dword [NumARP]    ;increase ARP-entries counter

;  .replace:
    DEBUGF 1,"Updating ARP entry: %x-%x-%x-%x-%x-%x = %u.%u.%u.%u to slot:%u\n",\
    [esp + ARP_ENTRY.MAC]:2,[esp + ARP_ENTRY.MAC+1]:2,[esp + ARP_ENTRY.MAC+2]:2,[esp + ARP_ENTRY.MAC+3]:2,[esp + ARP_ENTRY.MAC+4]:2,[esp + ARP_ENTRY.MAC+5]:2,\
    [esp + ARP_ENTRY.IP]:1,[esp + ARP_ENTRY.IP+1]:1,[esp + ARP_ENTRY.IP+2]:1,[esp + ARP_ENTRY.IP+3]:1,eax

    cld
    mov     esi, esp		  ;esp=base of ARP-entry, that will be added
    lea     edi, [ebx + eax]	  ;ebx + eax=ARPTable_base + ARP-entry_base(where we will add)
    mov     ecx,ARP_ENTRY.size/2  ;ARP_ENTRY_SIZE must be even number!!!
    rep     movsw

    mov     ecx, ARP_ENTRY.size
    xor     edx, edx  ;"div" takes operand from EDX:EAX
    div     ecx       ;eax=index of entry, which has been added



.add_exit:

    add     esp, ARP_ENTRY.size   ;free stack
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
    imul    esi, ARP_ENTRY.size

    mov     ecx, (ARP_TABLE_SIZE - 1) * ARP_ENTRY.size
    sub     ecx, esi

    lea     edi, [ebx + esi]		;edi=ptr to entry that should be deleted
    lea     esi, [edi + ARP_ENTRY.size] ;esi=ptr to next entry

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
    imul    esi, ARP_ENTRY.size  ;esi=ptr to required ARP_ENTRY
    mov     edi, [Extra]	  ;edi=buffer for reading
    mov     ecx, ARP_ENTRY.size/2 ; must be even number!!!
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

    DEBUGF 1,"Trying to find MAC for %u.%u.%u.%u\n",[Index]:1,[Index+1]:1,[Index+2]:1,[Index+3]:1

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


;;; TODO: get device number ! (in edx)
    xor     edx, edx

    mov     eax, [Index]       ;eax=required IP
    mov     esi, eax
    and     esi, [SUBNET_LIST+edx]
    mov     ecx, [IP_LIST+edx]
    and     ecx, [SUBNET_LIST+edx]
    cmp     esi, ecx
    je	    @f	      ;if we and target IP are located in the same network
    mov     eax, [GATEWAY_LIST+edx]
    mov     [Index], eax
    DEBUGF 1,"IP is not on subnet, using %u.%u.%u.%u instead\n",[Index]:1,[Index+1]:1,[Index+2]:1,[Index+3]:1
  @@:

    cmp     dword[NumARP], 0
    je	    .ip_to_mac_send_request ;if ARP-table not contain an entries, we have to request IP.
				    ;EAX will be containing a zero, it's equal to ARP_NO_ENTRY

    mov     ecx, dword[NumARP]
    imul    esi, ecx, ARP_ENTRY.size  ;esi=current ARP-table size

  @@:
    sub     esi, ARP_ENTRY.size
    cmp     [ebx + esi + ARP_ENTRY.IP], eax	    ; ebx=ARPTable base
    loopnz  @b			     ; Return back if non match
    jnz     .ip_to_mac_send_request  ; and request IP->MAC if none found in the table

    ; Return the entry status in eax
    movzx   eax, word[ebx + esi + ARP_ENTRY.Status]

    DEBUGF 1,"MAC found:  %x-%x-%x-%x-%x-%x status:%x in slot:%u\n",\
    [ebx + esi + ARP_ENTRY.MAC]:2,[ebx + esi + ARP_ENTRY.MAC+1]:2,[ebx + esi + ARP_ENTRY.MAC+2]:2,[ebx + esi + ARP_ENTRY.MAC+3]:2,[ebx + esi + ARP_ENTRY.MAC+4]:2,[ebx + esi + ARP_ENTRY.MAC+5]:2, ax, esi

    ; esi holds index
    cld
    lea     esi, [ebx + esi + ARP_ENTRY.MAC]
    mov     edi, [Extra]   ;edi=ptr to buffer for write MAC
    movsd
    movsw
    jmp     .exit

  .ip_to_mac_send_request:
;;; TODO: get device number ! (in edx)
    xor     edx, edx
    mov     edx, [ETH_DRV_LIST + 4*edx]
    lea     ecx, [edx + ETH_DEVICE.mac]

    stdcall arp_request,[Index],[IP_LIST+edx],ecx  ;TargetIP,SenderIP_ptr,SenderMAC_ptr
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

    DEBUGF 1,"Create ARP request\n"


    stdcall kernel_alloc, 60  ; minimum eth packet size
    test    eax, eax
    jz	    .exit

    mov     ebx, eax

    mov     word [ebx + ETH_FRAME.Data + ARP_Packet.HardwareType], 0x0100 ;Ethernet
    mov     word [ebx + ETH_FRAME.Data + ARP_Packet.ProtocolType], 0x0008 ;IP
    mov     byte [ebx + ETH_FRAME.Data + ARP_Packet.HardwareSize], 0x06   ;MAC-addr length
    mov     byte [ebx + ETH_FRAME.Data + ARP_Packet.ProtocolSize], 0x04   ;IP-addr length
    mov     word [ebx + ETH_FRAME.Data + ARP_Packet.Opcode], 0x0100	  ;Request

    DEBUGF 1,"1"

    cld
    mov     esi, [SenderMAC_ptr]
    lea     edi, [ebx + ETH_FRAME.Data + ARP_Packet.SenderMAC]	     ;Our MAC-addr
    movsd
    movsw

    DEBUGF 1,"2"

    mov     esi, [SenderIP_ptr]
    mov     [ebx + ETH_FRAME.Data + ARP_Packet.SenderIP], esi	     ;Our IP-addr
;    movsd

    DEBUGF 1,"3"

    lea     edi, [ebx + ETH_FRAME.Data + ARP_Packet.TargetMAC]	     ; Required MAC-addr
    xor     eax, eax
    stosd
    stosw

    DEBUGF 1,"4"

    lea     edi, [ebx + ETH_FRAME.DstMAC]
    stosd
    stosw

    DEBUGF 1,"5"

    mov     esi, [TargetIP]
    mov     dword [ebx + ETH_FRAME.Data + ARP_Packet.TargetIP], esi   ;Required IP-addr(we get it as function parameter)


    DEBUGF 1,"6"

    mov     esi, [SenderMAC_ptr]
    lea     edi, [ebx + ETH_FRAME.SrcMAC]
    movsd
    movsw

    DEBUGF 1,"7"

    mov     ax , ETHER_ARP
    stosw

    DEBUGF 1,"8"

;;; TODO: get device number in edx !!
    xor     edx, edx
    shl     edx, 2

    inc     [ARP_PACKETS_TX+edx]

    push    dword .returnaddr
    push    dword 60
    push    ebx
    mov     ebx, [ETH_DRV_LIST + edx]
    jmp     [ebx + ETH_DEVICE.transmit]
.returnaddr:

    ; Add an entry in the ARP table, awaiting response
    sub     esp, ARP_ENTRY.size    ;allocate memory for ARP-entry

    mov     esi, dword[TargetIP]
    mov     dword[esp + ARP_ENTRY.IP],esi

    lea     edi, [esp + ARP_ENTRY.MAC]
    xor     eax, eax
    stosd
    stosw

    mov     word[esp + ARP_ENTRY.Status], ARP_AWAITING_RESPONSE
    mov     word[esp + ARP_ENTRY.TTL], 10  ; 10 seconds

    stdcall arp_table_manager,ARP_TABLE_ADD,EXTRA_IS_ARP_ENTRY_PTR,esp
    add     esp, ARP_ENTRY.size ; free memory

.exit:

    DEBUGF 1,"ARP request - end\n"
    ret
endp





;---------------------------------------------------------------------------
;
; ARP_decrease_entry_ttls
;
; IN: /
; OUT: /
;
;---------------------------------------------------------------------------

align 4
ARP_decrease_entry_ttls:

	mov	ecx, [NumARP]
	test	ecx, ecx
	jz	.exit

	mov	ebx, ARPTable

.timer_loop:

	movsx	esi, word [ebx + ARP_ENTRY.TTL]
	cmp	esi, 0xFFFFFFFF
	je	.timer_loop_end  ;if TTL==0xFFFF then it's static entry

	test	esi, esi
	jnz	.timer_loop_end_with_dec  ;if TTL!=0

	; Ok, TTL is 0
	;if Status==AWAITING_RESPONSE and TTL==0
	;then we have to change it to ARP_RESPONSE_TIMEOUT
	cmp	word [ebx + ARP_ENTRY.Status], ARP_AWAITING_RESPONSE
	jne	@f

	mov	word [ebx + ARP_ENTRY.Status], ARP_RESPONSE_TIMEOUT
	mov	word [ebx + ARP_ENTRY.TTL], word 0x000A   ;10 sec
	jmp	.timer_loop_end

  @@:
	;if TTL==0 and Status==VALID_MAPPING, we have to delete it
	;if TTL==0 and Status==RESPONSE_TIMEOUT, delete too
	mov	esi, dword[NumARP]
	sub	esi, ecx	  ;esi=index of entry, will be deleted

	call	ARP_del_entry

	jmp	.timer_loop_end


.timer_loop_end_with_dec:

	dec	word [ebx + ARP_ENTRY.TTL]  ;decrease TTL

.timer_loop_end:

	add	ebx, ARP_ENTRY.size
	loop	.timer_loop

.exit:

	ret


;---------------------------------------------------------------------------
;
; ARP_del_entry
;
; IN: entry # in esi
; OUT: /
;
;---------------------------------------------------------------------------

align 4
ARP_del_entry:

	imul	esi, ARP_ENTRY.size

	mov	ecx, (ARP_TABLE_SIZE - 1) * ARP_ENTRY.size
	sub	ecx, esi

	lea	edi, [ebx + esi]	    ;edi=ptr to entry that should be deleted
	lea	esi, [edi + ARP_ENTRY.size] ;esi=ptr to next entry

	shr	ecx,1	   ;ecx/2 => ARP_ENTRY_SIZE MUST BE EVEN NUMBER!
	cld
	rep	movsw

	dec	dword[NumARP] ;decrease arp-entries counter
	ret





;-----------------------------------------------------
;
; ARP_Handler:
;
;  This function handles ARP protocol over ethernet
;  (other protocols may follow in the future)
;
;  IN:  Pointer to buffer in [esp]
;       size of buffer in [esp+4]
;       packet size (without ethernet header) in ecx
;  OUT: /
;
;-----------------------------------------------------
align 4
ARP_Handler:

	DEBUGF	1,"ARP_Handler - start\n"
	cmp	ecx, 28
	jl	.exit

; Is this a REQUEST?
; Is this a request for My Host IP
; Yes - So construct a response message.
; Send this message to the ethernet card for transmission

;        push    ebx edx
	stdcall arp_table_manager, ARP_TABLE_ADD, EXTRA_IS_ARP_PACKET_PTR, edx
;        pop     edx ebx

	cmp	word [edx + ARP_Packet.Opcode], ARP_REQ_OPCODE	; Is this a request packet?
	jne	.exit

	call	ETH_struc2dev
	DEBUGF	1,"ARP Packet came from device: %u\n", edi
	inc	[ARP_PACKETS_RX+4*edi]
	cmp	edi, -1
	jz	.exit

	mov	eax, edi
	shl	eax, 2
	add	eax, IP_LIST
	mov	eax, [eax]
	cmp	eax, [edx + ARP_Packet.TargetIP]		; Is it looking for my IP address?
	jnz	.exit
	push	eax
	push	edi

;        DEBUGF  1,"ETH_ARP_Handler - request for %u.%u.%u.%u\n",[edi+0]:1,[edi+1]:1,[edi+2]:1,[edi+3]:1

; OK, it is a request for one of our MAC addresses. Build the frame and send it
; We can reuse the buffer.

	cld
	lea	esi, [edx + ARP_Packet.SenderMAC]
	lea	edi, [edx + ARP_Packet.TargetMAC]
	movsd							; Move Sender Mac to Dest MAC
	movsw							;
	movsd							; Move sender IP to Dest IP

	pop	esi
	mov	esi, [ETH_DRV_LIST + 4*esi]
	lea	esi, [esi + ETH_DEVICE.mac]
	lea	edi, [edx + ARP_Packet.SenderMAC]
	movsd							; Copy MAC address from in MAC_LIST
	movsw							;
	pop	eax
	stosd							; Write our IP

	mov	word [edx + ARP_Packet.Opcode], ARP_REP_OPCODE

; Now, Fill in ETHERNET header

	mov	edi, [esp]
	lea	esi, [edx + ARP_Packet.TargetMAC]
	movsd
	movsw
	lea	esi, [edx + ARP_Packet.SenderMAC]
	movsd
	movsw
	mov	ax , ETHER_ARP
	stosw

	jmp	ETH_Sender					; And send it!

       .exit:
	call	kernel_free
	add	esp, 4						; pop (balance stack)

	DEBUGF 1,"ARP_Handler - fail\n"
	ret




;---------------------------------------------------------------------------
;
; ARP_API
;
; This function is called by system function 75
;
; IN:  subfunction number in bl
;      device number in bh
;      ecx, edx, .. depends on subfunction
;
; OUT:
;
;---------------------------------------------------------------------------

align 4
ARP_API:

	movzx	eax, bh
	shl	eax, 2

	test	bl, bl
	jz	.packets_tx	; 0
	dec	bl
	jz	.packets_rx	; 1
	dec	bl
	jz	.entries	; 2
	dec	bl
	jz	.read		; 3
	dec	bl
	jz	.write		; 4
	dec	bl
	jz	.remove 	; 5
	dec	bl


.error:
	mov	eax, -1
	ret

.packets_tx:
	add	eax, ARP_PACKETS_TX
	mov	eax, [eax]
	ret

.packets_rx:
	add	eax, ARP_PACKETS_RX
	mov	eax, [eax]
	ret

.entries:
	mov	eax, [NumARP]
	ret

.read:
	; TODO: write code
	ret

.write:
	; TODO: write code
	ret

.remove:
	; TODO: write code
	ret

