format MS COFF

public @EXPORT as 'EXPORTS'

include '../struct.inc'
include '../proc32.inc'
include '../macros.inc'
purge section,mov,add,sub

include 'network.inc'

section '.flat' code readable align 16

;;===========================================================================;;
lib_init: ;//////////////////////////////////////////////////////////////////;;
;;---------------------------------------------------------------------------;;
;? Library entry point (called after library load)                           ;;
;;---------------------------------------------------------------------------;;
;> eax = pointer to memory allocation routine                                ;;
;> ebx = pointer to memory freeing routine                                   ;;
;> ecx = pointer to memory reallocation routine                              ;;
;> edx = pointer to library loading routine                                  ;;
;;---------------------------------------------------------------------------;;
;< eax = 1 (fail) / 0 (ok) (library initialization result)                   ;;
;;===========================================================================;;
	mov	[mem.alloc], eax
	mov	[mem.free], ebx
	mov	[mem.realloc], ecx
	mov	[dll.load], edx
	mov	[DNSrequestID], 1
	stdcall edx, @IMPORT
	ret	4

;;===========================================================================;;
;; in_addr_t __stdcall inet_addr(__in const char* hostname);                 ;;
inet_addr:								     ;;
;;---------------------------------------------------------------------------;;
;? Convert the string from standard IPv4 dotted notation to integer IP addr. ;;
;;---------------------------------------------------------------------------;;
;> first parameter = host name                                               ;;
;;---------------------------------------------------------------------------;;
;< eax = IP address on success / -1 on error                                 ;;
;;===========================================================================;;
; 0. Save used registers for __stdcall.
	push	ebx esi edi
	mov	esi, [esp+16]	; esi = hostname
; 1. Check that only allowed symbols are present.
; (hex digits, possibly letters 'x'/'X' and up to 3 dots)
	push	esi
	xor	ecx, ecx
.calcdots_loop:
; loop for all characters in string
	lodsb
; check for end of string
	cmp	al, 0
	jz	.calcdots_loop_done
; check for dot
	cmp	al, '.'
	jz	.dot
; check for digit
	sub	al, '0'
	cmp	al, 9
	jbe	.calcdots_loop
; check for hex letter
	sub	al, 'A' - '0'	; 'A'-'F' -> 0-5, 'a'-'f' -> 20h-25h
	and	al, not 20h
	cmp	al, 'F' - 'A'
	jbe	.calcdots_loop
; check for 'x'/'X'
	cmp	al, 'X' - 'A'
	jz	.calcdots_loop
	jmp	.fail.pop
.dot:
	inc	ecx
	jmp	.calcdots_loop
.calcdots_loop_done:
	cmp	ecx, 4
	jae	.fail.pop
; 2. The name can be valid dotted name; try to convert, checking limit
	pop	esi
	xor	edi, edi	; edi = address
	push	0xFFFFFFFF
	pop	edx		; edx = mask for rest of address
; 2a. Convert name except for last group.
	jecxz	.ip_convert_2b
.ip_convert_2a:
	push	ecx
	mov	ecx, 0xFF	; limit for all groups except for last
	call	.get_number
	pop	ecx
	jc	.fail
	cmp	byte [esi-1], '.'
	jnz	.fail
	shl	edi, 8
	shr	edx, 8
	add	edi, eax
	loop	.ip_convert_2a
; 2b. Convert last group.
.ip_convert_2b:
	mov	ecx, edx
	call	.get_number
	jc	.fail
	cmp	byte [esi-1], 0
	jnz	.fail
@@:
	shl	edi, 8
	shr	edx, 8
	jnz	@b
	add	edi, eax
; 2c. Convert to network byte order.
	bswap	edi
; 3. Set return value, restore used registers and return.
	xchg	eax, edi
.ret:
	pop	edi esi ebx
	ret	4
; 4. On error, return -1.
.fail.pop:
	pop	esi
.fail:
	push	-1
	pop	eax
	jmp	.ret

;;===========================================================================;;
;; Internal auxiliary function for IP parsing.                                        ;;
.get_number:								     ;;
;;---------------------------------------------------------------------------;;
;? Converts string to number.                                                ;;
;;---------------------------------------------------------------------------;;
;> esi -> string                                                             ;;
;> ecx = limit for number                                                    ;;
;;---------------------------------------------------------------------------;;
;< eax = number                                                              ;;
;< CF set on error (too big number) / cleared on success                     ;;
;< esi -> end of number representation                                       ;;
;;===========================================================================;;
; 0. Save edx, which is used in caller.
	push	edx
; 1. Initialize number, zero eax so that lodsb gets full dword.
	xor	eax, eax
	xor	edx, edx
; 2. Get used numeral system: 0x = hex, otherwise 0 = octal, otherwise decimal
	push	10
	pop	ebx
	lodsb
	cmp	al, '0'
	jnz	.convert
	push	8
	pop	ebx
	lodsb
	cmp	al, 'x'
	jnz	.convert
	add	ebx, ebx
; 3. Loop while digits are encountered.
.convert:
; 4. Convert digit from text representation to binary value.
	or	al, 20h ; '0'-'9' -> '0'-'9', 'A'-'F' -> 'a'-'f'
	sub	al, '0'
	cmp	al, 9
	jbe	.digit
	sub	al, 'a' - '0'
	cmp	al, 'f' - 'a'
	ja	.convert_done
	add	al, 10
.digit:
; 5. Digit must be less than base of numeral system.
	cmp	eax, ebx
	jae	.convert_done
; 6. Advance the number.
	imul	edx, ebx
	add	edx, eax
	cmp	edx, ecx
	ja	.gn_error
; 3b. Continue loop.
	lodsb
	jmp	.convert
.convert_done:
; 7. Invalid character, number converted, return success.
	xchg	eax, edx
	pop	edx
	clc
	ret
.gn_error:
; 8. Too big number, return error.
	pop	edx
	stc
	ret

;;===========================================================================;;
;; char* __stdcall inet_ntoa(struct in_addr in);                             ;;
inet_ntoa:								     ;;
;;---------------------------------------------------------------------------;;
;? Convert the Internet host address to standard IPv4 dotted notation.       ;;
;;---------------------------------------------------------------------------;;
;> first parameter = host address                                            ;;
;;---------------------------------------------------------------------------;;
;< eax = pointer to resulting string (in static buffer)                      ;;
;;===========================================================================;;
; 0. Save used registers for __stdcall.
	push	ebx esi edi
	mov	bl, 0xCD	; constant for div 10
; 1. Write octet 4 times.
	mov	edi, .buffer
	mov	edx, [esp+16]	; eax = in
	mov	al, dl
	call	.write
	mov	al, dh
	shr	edx, 16
	call	.write
	mov	al, dl
	call	.write
	mov	al, dh
	call	.write
; 2. Replace final dot with terminating zero.
	mov	byte [edi-1], 0
; 3. Restore used registers, set result value and return.
	pop	edi esi ebx
	mov	eax, .buffer
	ret	4

.write:
	movzx	esi, al
	mul	bl
	add	esi, ('.' shl 8) + '0'
	shr	ah, 3	; ah = al / 10
	movzx	ecx, ah
	add	ecx, ecx
	lea	ecx, [ecx*5]
	sub	esi, ecx	; lobyte(esi) = al % 10, hibyte(esi) = '.'
	test	ah, ah
	jz	.1digit
	cmp	ah, 10
	jb	.2digit
	cmp	ah, 20
	sbb	cl, cl
	add	cl, '2'
	mov	byte [edi], cl
	movzx	ecx, cl
	lea	ecx, [ecx*5]
	sub	ah, cl
	sub	ah, cl
	add	ah, ('0'*11) and 255
	mov	byte [edi+1], ah
	mov	word [edi+2], si
	add	edi, 4
	ret
.2digit:
	add	ah, '0'
	mov	byte [edi], ah
	mov	word [edi+1], si
	add	edi, 3
	ret
.1digit:
	mov	word [edi], si
	add	edi, 2
	ret

struct __gai_reqdata
	socket	dd	?
; external code should not look on rest of this structure,
; it is internal for getaddrinfo_start/process/abort
	reqid		dw	?	; DNS request ID
	socktype	db	?	; SOCK_* or 0 for any
			db	?
	service 	dd	?
	flags		dd	?
	reserved	rb	16
ends

;;===========================================================================;;
;; int __stdcall getaddrinfo(__in const char* hostname,                      ;;
;;                           __in const char* servname,                      ;;
;;                           __in const struct addrinfo* hints,              ;;
;;                           __out struct addrinfo **res);                   ;;
getaddrinfo:								     ;;
;;---------------------------------------------------------------------------;;
;? Get a list of IP addresses and port numbers for given host and service    ;;
;;---------------------------------------------------------------------------;;
;> first parameter (optional) = host name                                    ;;
;> second parameter (optional) = service name (decimal number for now)       ;;
;> third parameter (optional) = hints for socket type                        ;;
;> fourth parameter = pointer to result (head of L1-list)                    ;;
;;---------------------------------------------------------------------------;;
;< eax = 0 on success / one of EAI_ codes on error                           ;;
;;===========================================================================;;
; 0. Save used registers for __stdcall.
	push	ebx esi edi
	mov	edi, [esp+28]	; edi = res
; 1. Create and send DNS packet.
	sub	esp, sizeof.__gai_reqdata	; reserve stack place (1)
	push	esp		; fifth parameter = pointer to (1)
	push	edi		; fourth parameter = res
	push	dword [esp+32+sizeof.__gai_reqdata]	; third parameter = hints
	push	dword [esp+32+sizeof.__gai_reqdata]	; second parameter = servname
	push	dword [esp+32+sizeof.__gai_reqdata]	; first parameter = hostname
	call	getaddrinfo_start
	test	eax, eax
	jns	.ret	; if name resolved without network activity, return
; 2. Wait for DNS reply.
; 2a. Ignore all events except network stack.
	mcall	40, EVM_STACK
	push	eax	; save previous event mask (2)
; 2b. Get upper limit for wait time. Use timeout = 5 seconds.
	mcall	26, 9	; get time stamp
	xchg	esi, eax	; save time stamp to esi
	mov	ebx, 500	; start value for timeout
	add	esi, ebx
.wait:
; 2c. Wait for event with timeout.
	mcall	23	; wait for event - must be stack event
; 2d. Check for timeout.
	test	eax, eax
	lea	eax, [esp+4]	; pointer to (1)
	jz	.timeout
; 3. Got packet. Call processing function.
	push	edi	; second parameter: pointer to result
	push	eax	; first parameter: pointer to reqdata
	call	getaddrinfo_process
; 4. Test whether wait loop must be continued.
	test	eax, eax
	jns	.ret.restore
; 2e. Recalculate timeout value.
	mcall	26, 9
	mov	ebx, esi
	sub	ebx, eax
; 2f. Check that time is not over; if not, continue wait loop
	cmp	ebx, 500
	jbe	.wait
.timeout:
; 5. Timeout: abort and return error
	push	eax
	call	getaddrinfo_abort
	and	dword [edi], 0
	push	EAI_AGAIN
	pop	eax
.ret.restore:
; 6. Restore event mask.
	pop	ebx	; get event mask (2)
	push	eax	; save return code (3)
	mcall	40
	pop	eax	; restore return code (3)
.ret:
; 7. Restore stack pointer, used registers and return.
	add	esp, sizeof.__gai_reqdata	; undo (1)
	pop	edi esi ebx
	ret	16

;;===========================================================================;;
;; int __stdcall getaddrinfo_start(__in const char* hostname,                ;;
;;                                 __in const char* servname,                ;;
;;                                 __in const struct addrinfo* hints,        ;;
;;                                 __out struct addrinfo **res,              ;;
;;                                 __out struct __gai_reqdata* reqdata);     ;;
getaddrinfo_start:							     ;;
;;---------------------------------------------------------------------------;;
;? Initiator for getaddrinfo, sends DNS request                              ;;
;;---------------------------------------------------------------------------;;
;> first 4 parameters same as for getaddrinfo                                ;;
;> last parameter = pointer to buffer for __gai_reqdata, must be passed to   ;;
;>                  getaddrinfo_process as is                                ;;
;;---------------------------------------------------------------------------;;
;< eax = <0 if wait loop must be entered / 0 on success / EAI_* on error     ;;
;;===========================================================================;;
;; Known limitations:                                                        ;;
;; 1. No support for TCP connections =>                                      ;;
;; 1a. Long replies will be truncated, and not all IP addresses will be got. ;;
;; 2. No support for iterative resolving =>                                  ;;
;; 2a. In theory may fail with some servers.                                 ;;
;; 3. Assumes that domain for relative names is always root, ".".            ;;
;; 4. Does not support lookup of services by name,                           ;;
;;    only decimal representation is supported.                              ;;
;; 5. Assumes that IPv4 is always configured, so AI_ADDRCONFIG has no effect.;;
;;===========================================================================;;
; 0. Create stack frame and save used registers for __stdcall.
	push	ebx esi edi
	push	ebp
	mov	ebp, esp
virtual at ebp-8
.recent_restsize dd	?	; this is for memory alloc in ._.generate_data
.recent_page	dd	?	; this is for memory alloc in ._.generate_data
	rd	5	; saved regs and return address
.hostname	dd	?
.servname	dd	?
.hints		dd	?
.res		dd	?
.reqdata	dd	?
end virtual
	xor	edi, edi
	push	edi	; init .recent_page
	push	edi	; init .recent_restsize
; 1. Check that parameters are correct and can be handled by this implementation.
; 1a. If 'res' pointer is given, set result to zero.
	mov	eax, [.res]
	test	eax, eax
	jz	@f
	mov	[eax], edi
@@:
; 1b. Only AI_SUPPORTED flags are supported for hints->ai_flags.
	mov	ecx, [.hints]
	xor	edx, edx
	jecxz	.nohints
	mov	edx, [ecx+addrinfo.ai_flags]
.nohints:
	mov	ebx, [.reqdata]
	mov	[ebx+__gai_reqdata.flags], edx
	push	EAI_BADFLAGS
	pop	eax
	test	edx, not AI_SUPPORTED
	jnz	.ret
; 1c. Either hostname or servname must be given. If AI_CANONNAME is set,
; hostname must also be set.
	cmp	[.hostname], edi
	jnz	@f
	test	dl, AI_CANONNAME
	jnz	.ret
	push	EAI_NONAME
	pop	eax
	cmp	[.servname], edi
	jz	.ret
@@:
; 1d. Only IPv4 is supported, so hints->ai_family must be either PF_UNSPEC or PF_INET.
	push	EAI_FAMILY
	pop	eax
	jecxz	@f
	cmp	[ecx+addrinfo.ai_family], edi
	jz	@f
	cmp	[ecx+addrinfo.ai_family], PF_INET
	jnz	.ret
@@:
; 1e. Valid combinations for ai_socktype/ai_protocol: 0/0 for any or
;       SOCK_STREAM/IPPROTO_TCP, SOCK_DGRAM/IPPROTO_UDP
;       (raw sockets are not yet supported by the kernel)
	xor	edx, edx	; assume 0=any if no hints
	jecxz	.socket_type_ok
	mov	edx, [ecx+addrinfo.ai_socktype]
	mov	esi, [ecx+addrinfo.ai_protocol]
; 1f. Test for ai_socktype=0 and ai_protocol=0.
	test	edx, edx
	jnz	.check_socktype
	test	esi, esi
	jz	.socket_type_ok
; 1g. ai_socktype=0, ai_protocol is nonzero.
	push	EAI_SERVICE
	pop	eax
	inc	edx	; edx = SOCK_STREAM
	cmp	esi, IPPROTO_TCP
	jz	.socket_type_ok
	inc	edx	; edx = SOCK_DGRAM
	cmp	esi, IPPROTO_UDP
	jz	.socket_type_ok
.ret:
; Restore saved registers, destroy stack frame and return.
	mov	esp, ebp
	pop	ebp
	pop	edi esi ebx
	ret	20
; 1h. ai_socktype is nonzero.
.check_socktype:
	push	EAI_SOCKTYPE
	pop	eax
	cmp	edx, SOCK_STREAM
	jz	.check_tcp
	cmp	edx, SOCK_DGRAM
	jnz	.ret
	test	esi, esi
	jz	.socket_type_ok
	cmp	esi, IPPROTO_UDP
	jz	.socket_type_ok
	jmp	.ret
.check_tcp:
	test	esi, esi
	jz	.socket_type_ok
	cmp	esi, IPPROTO_TCP
	jnz	.ret
.socket_type_ok:
	mov	[ebx+__gai_reqdata.socktype], dl
; 2. Resolve service.
; 2a. If no name is given, remember value -1.
	push	-1
	pop	edx
	mov	esi, [.servname]
	test	esi, esi
	jz	.service_resolved
; 2b. Loop for characters of string while digits are encountered.
	xor	edx, edx
	xor	eax, eax
.serv_to_number:
	lodsb
	sub	al, '0'
	cmp	al, 9
	ja	.serv_to_number_done
; for each digit, set edx = edx*10 + <digit>
	lea	edx, [edx*5]
	lea	edx, [edx*2+eax]
; check for correctness: service port must fit in word
	cmp	edx, 0x10000
	jae	.service_not_number
	jmp	.serv_to_number
.serv_to_number_done:
	and	edx, 0xFFFF	; make sure that port fits
; 2c. If zero character reached, name is resolved;
; otherwise, return error (no support for symbolic names yet)
	cmp	al, -'0'
	jz	.service_resolved
.service_not_number:
	push	EAI_NONAME
	pop	eax
	jmp	.ret
.service_resolved:
; 2d. Save result to reqdata.
	mov	[ebx+__gai_reqdata.service], edx
; 3. Process host name.
	mov	esi, [.hostname]
; 3a. If hostname is not given,
;       use localhost for active sockets and INADDR_ANY for passive sockets.
	mov	eax, 0x0100007F ; 127.0.0.1 in network byte order
	test	byte [ebx+__gai_reqdata.flags], AI_PASSIVE
	jz	@f
	xor	eax, eax
@@:
	test	esi, esi
	jz	.hostname_is_ip
; 3b. Check for dotted IPv4 name.
	push	esi
	call	inet_addr
	cmp	eax, -1
	jz	.resolve_hostname
.hostname_is_ip:
; 3c. hostname is valid representation of IP address, and we have resolved it.
; Generate result, if .res pointer is not NULL.
	mov	ebx, [.reqdata]
	mov	esi, [.res]
	test	esi, esi
	jz	.no_result
	call	getaddrinfo._.generate_data
; 3d. Check for memory allocation error.
.3d:
	push	EAI_MEMORY
	pop	eax
	test	esi, esi
	jz	.ret
; 3e. If AI_CANONNAME is set, copy input name.
	test	byte [ebx+__gai_reqdata.flags], AI_CANONNAME
	jz	.no_result
; 3f. Calculate length of name.
	push	-1
	pop	ecx
	mov	edi, [.hostname]
	xor	eax, eax
	repnz	scasb
	not	ecx
; 3g. Check whether it fits on one page with main data.
	cmp	ecx, [.recent_restsize]
	jbe	.name_fits
; 3h. If not, allocate new page.
	push	ecx
	add	ecx, 4	; first dword contains number of objects on the page
	mcall	68, 12
	pop	ecx
; 3i. If allocation has failed, free addrinfo and return error.
	test	eax, eax
	jnz	.name_allocated
	push	[.res]
	call	freeaddrinfo
	push	EAI_MEMORY
	pop	eax
	jmp	.ret
.name_allocated:
; 3j. Otherwise, set edi to allocated memory and continue to 3l.
	xchg	edi, eax	; put result to edi
	push	1
	pop	eax
	stosd	; number of objects on the page = 1
	jmp	.copy_name
.name_fits:
; 3k. Get pointer to free memory in allocated page.
	mov	edi, [.recent_page]
	mov	eax, edi
	and	eax, not 0xFFF
	inc	dword [eax]	; increase number of objects
.copy_name:
; 3l. Put pointer to struct addrinfo.
	mov	eax, [.res]
	mov	eax, [eax]
	mov	[eax+addrinfo.ai_canonname], edi
; 3m. Copy name.
	rep	movsb
.no_result:
; 3n. Return success.
	xor	eax, eax
	jmp	.ret
; 4. Host address is not dotted IP. Test whether we are allowed to contact DNS.
; Return error if no.
.resolve_hostname:
	push	EAI_NONAME
	pop	eax
	mov	ebx, [.reqdata]
	test	byte [ebx+__gai_reqdata.flags], AI_NUMERICHOST
	jnz	.ret
; Host address is domain name. Contact DNS server.
	mov	esi, [.hostname]
; 5. Reserve stack place for UDP packet.
; According to RFC1035, maximum UDP packet size in DNS is 512 bytes.
	sub	esp, 512
; 6. Create DNS request packet.
; 6a. Set pointer to start of buffer.
	mov	edi, esp
; 6b. Get request ID, write it to buffer.
	push	1
	pop	eax
lock	xadd	[DNSrequestID], eax	; atomically increment ID, get old value
	stosw
	mov	[ebx+__gai_reqdata.reqid], ax
; 6c. Packed field: QR=0 (query), Opcode=0000 (standard query),
;       AA=0 (ignored in requests), TC=0 (no truncation),
;       RD=1 (recursion desired)
	mov	al, 00000001b
	stosb
; 6d. Packed field: ignored in requests
	mov	al, 0
	stosb
; 6e. Write questions count = 1 and answers count = 0
; Note that network byte order is big-endian.
	mov	eax, 0x00000100
	stosd
; 6f. Write nameservers count = 0 and additional records count = 0
	xor	eax, eax
	stosd
; 6g. Write request data: name
; According to RFC1035, maximum length of name is 255 bytes.
; For correct names, buffer cannot overflow.
	lea	ebx, [esi+256]	; ebx = limit for name (including terminating zero)
; translate string "www.yandex.ru" {00} to byte data {03} "www" {06} "yandex" {02} "ru" {00}
.nameloop:	; here we go in the start of each label: before "www", before "yandex", before "ru"
	xor	ecx, ecx	; ecx = length of current label
	inc	edi	; skip length, it will be filled later
.labelloop:	; here we go for each symbol of name
	lodsb	; get next character
	test	al, al	; terminating zero?
	jz	.endname
	cmp	esi, ebx	; limit exceeded?
	jae	.wrongname
	cmp	al, '.' ; end of label?
	jz	.labelend
	stosb	; put next character
	inc	ecx	; increment label length
	jmp	.labelloop
.wrongname:
	push	EAI_NONAME
	pop	eax
	jmp	.ret
.labelend:
	test	ecx, ecx	; null label can be only in the end of name
	jz	.wrongname
.endname:
	cmp	ecx, 63
	ja	.wrongname
; write length to byte [edi-ecx-1]
	mov	eax, ecx
	neg	eax
	mov	byte [edi+eax-1], cl
	cmp	byte [esi-1], 0 ; that was last label in the name?
	jnz	.nameloop
; write terminating zero if not yet
	mov	al, 0
	cmp	byte [edi-1], al
	jz	@f
	stosb
@@:
; 6h. Write request data:
;       query type = A (host address) = 1,
;       query class = IN (internet IPv4 address) = 1
; Note that network byte order is big-endian.
	mov	eax, 0x01000100
	stosd
; 7. Get DNS server address.
	mcall	75, 0x00000004 ; protocol IP=0, device number=0, function=get DNS address
	cmp	eax, -1
	je	.ret.dnserr
	mov	esi, eax	; put server address to esi
; 8. Open UDP socket to DNS server, port 53.
; 8a. Create new socket.
	mcall	74, 0, AF_INET, IPPROTO_UDP
	cmp	eax, -1 ; error?
	jz	.ret.dnserr
	mov	ecx, eax	; put socket handle to ecx
; 8b. Create sockaddr structure on the stack.
	push	0
	push	0	; sin_zero
	push	esi	; sin_addr
	push	AF_INET + (53 shl 16)
			; sin_family and sin_port in network byte order
; 8c. Connect.
	mcall	74, 4, , esp, sizeof.sockaddr_in
; 8d. Restore the stack, undo 8b.
	add	esp, esi
; 8e. Check result.
	cmp	eax, -1
	jz	.ret.close
; 9. Send DNS request packet.
	sub	edi, esp	; get packet length
	mov	esi, edi
	xor	edi, edi
	mcall	74, 6, , esp
	cmp	eax, -1
	jz	.ret.close
	mov	eax, [.reqdata]
	mov	[eax+__gai_reqdata.socket], ecx
	push	-1
	pop	eax	; return status: more processing required
	jmp	.ret.dns
.ret.close:
	mcall	74, 1
.ret.dnserr:
	push	EAI_AGAIN
	pop	eax
.ret.dns:
; 6. Restore stack pointer and return.
	jmp	.ret

;;===========================================================================;;
;; int __stdcall getaddrinfo_process(__in struct __gai_reqdata* reqdata,     ;;
;;                                   __out struct addrinfo** res);           ;;
getaddrinfo_process:							     ;;
;;---------------------------------------------------------------------------;;
;? Processes network events from DNS reply                                   ;;
;;---------------------------------------------------------------------------;;
;> first parameter = pointer to struct __gai_reqdata filled by ..._start     ;;
;> second parameter = same as for getaddrinfo                                ;;
;;---------------------------------------------------------------------------;;
;< eax = -1 if more processing required / 0 on success / >0 = error code     ;;
;;===========================================================================;;
; 0. Create stack frame.
	push	ebp
	mov	ebp, esp
virtual at ebp-.locals_size
.locals_start:
.datagram	rb	512
.addrname	dd	?
.name		dd	?
.res_list_tail	dd	?
.cname		dd	?
.recent_restsize dd	?	; this is for memory alloc in ._.generate_data
.recent_page	dd	?	; this is for memory alloc in ._.generate_data
.locals_size = $ - .locals_start
		rd	2
.reqdata	dd	?
.res		dd	?
end virtual
	xor	eax, eax
	push	eax	; initialize .recent_page
	push	eax	; initialize .recent_restsize
	push	eax	; initialize .cname
	push	[.res]	; initialize .res_list_tail
	sub	esp, .locals_size-16	; reserve place for other vars
	mov	edx, esp	; edx -> buffer for datagram
; 1. Save used registers for __stdcall.
	push	ebx esi edi
	mov	edi, [.reqdata]
; 2. Read UDP datagram.
	mov	ecx, [edi+__gai_reqdata.socket]
	push	edi
	mcall	74, 7, , , 512, 0
	pop	edi
; 3. Ignore events for other sockets (return if no data read)
	test	eax, eax
	jz	.ret.more_processing_required
; 4. Sanity check: discard too short packets.
	xchg	ecx, eax	; save packet length in ecx
	cmp	ecx, 12
	jb	.ret.more_processing_required
; 5. Discard packets with ID != request ID.
	mov	eax, dword [edi+__gai_reqdata.reqid]
	cmp	ax, [edx]
	jnz	.ret.more_processing_required
; 6. Sanity check: discard query packets.
	test	byte [edx+2], 80h
	jz	.ret.more_processing_required
; 7. Sanity check: must be exactly one query (our).
	cmp	word [edx+4], 0x0100	; note network byte order
	jnz	.ret.more_processing_required
; 8. Check for errors. Return EAI_NONAME for error code 3 and EAI_FAIL for other.
	mov	al, [edx+3]
	and	al, 0xF
	jz	@f
	cmp	al, 3
	jnz	.ret.no_recovery
	jmp	.ret.no_name
@@:
; 9. Locate answers section. Exactly 1 query is present in this packet.
	add	ecx, edx	; ecx = limit
	lea	esi, [edx+12]
	call	.skip_name
	lodsd		; skip QTYPE and QCLASS field
	cmp	esi, ecx
	ja	.ret.no_recovery
; 10. Loop through all answers.
	movzx	ebx, word [edx+6]	; get answers count
	xchg	bl, bh		; network -> Intel byte order
.answers_loop:
	dec	ebx
	js	.answers_done
; 10a. Process each record.
	mov	[.name], esi
; 10b. Skip name field.
	call	.skip_name
; 10c. Get record information, handle two types for class IN (internet).
	lodsd		; get type and class
	cmp	esi, ecx
	ja	.ret.no_recovery
	cmp	eax, 0x01000500 ; type=5, class=1?
	jz	.got_cname
	cmp	eax, 0x01000100 ; type=1, class=1?
	jnz	.answers_loop.next
.got_addr:
; 10d. Process record A, host address.
	add	esi, 10
	cmp	esi, ecx
	ja	.ret.no_recovery
	cmp	word [esi-6], 0x0400	; RDATA for A records must be 4 bytes long
	jnz	.ret.no_recovery
	mov	eax, [.name]
	mov	[.addrname], eax
; 10e. Create corresponding record in the answer.
	push	ebx ecx esi
	mov	eax, [esi-4]	; IP address
	mov	esi, [.res_list_tail]	; pointer to result
	test	esi, esi
	jz	.no_result	; do not save if .res is NULL
	mov	ebx, [.reqdata] ; request data
	call	getaddrinfo._.generate_data
	mov	[.res_list_tail], esi
	pop	esi ecx ebx
	cmp	[.res_list_tail], 0
	jnz	.answers_loop
; 10f. If generate_data failed (this means memory allocation failure), abort
	jmp	.ret.no_memory
.no_result:
	pop	esi ecx ebx
	jmp	.answers_loop
.got_cname:
; 10g. Process record CNAME, main host name.
	lea	eax, [esi+6]
	mov	[.cname], eax
.answers_loop.next:
; 10h. Skip other record fields, advance to next record.
	lodsd	; skip TTL
	xor	eax, eax
	lodsw	; get length of RDATA field
	xchg	al, ah	; network -> Intel byte order
	add	esi, eax
	cmp	esi, ecx
	ja	.ret.no_recovery
	jmp	.answers_loop
.answers_done:
; 11. Check that there is at least 1 answer.
	mov	eax, [.res_list_tail]
	cmp	[.res], eax
	jz	.ret.no_data
; 12. If canonical name was required, add it now.
	mov	eax, [.reqdata]
	test	byte [eax+__gai_reqdata.flags], AI_CANONNAME
	jz	.no_canon_name
; 12a. If at least one CNAME record is present, use name from last such record.
; Otherwise, use name from one of A records.
	mov	esi, [.cname]
	test	esi, esi
	jnz	.has_cname
	mov	esi, [.addrname]
.has_cname:
; 12b. Calculate name length.
	call	.get_name_length
	jc	.ret.no_recovery
; 12c. Check that the caller really want to get data.
	cmp	[.res], 0
	jz	.no_canon_name
; 12d. Allocate memory for name.
	call	getaddrinfo._.memalloc
	test	edi, edi
	jz	.ret.no_memory
; 12e. Make first entry in .res list point to canonical name.
	mov	eax, [.res]
	mov	eax, [eax]
	mov	[eax+addrinfo.ai_canonname], edi
; 12f. Decode name.
	call	.decode_name
.no_canon_name:
; 13. Set status to success.
	xor	eax, eax
	jmp	.ret.close
; Handle errors.
.ret.more_processing_required:
	push	-1
	pop	eax
	jmp	.ret
.ret.no_recovery:
	push	EAI_FAIL
	pop	eax
	jmp	.ret.destroy
.ret.no_memory:
	push	EAI_MEMORY
	pop	eax
	jmp	.ret.destroy
.ret.no_name:
.ret.no_data:
	push	EAI_NONAME
	pop	eax
.ret.destroy:
; 14. If an error occured, free memory acquired so far.
	push	eax
	mov	esi, [.res]
	test	esi, esi
	jz	@f
	pushd	[esi]
	call	freeaddrinfo
	and	dword [esi], 0
@@:
	pop	eax
.ret.close:
; 15. Close socket.
	push	eax
	mov	ecx, [.reqdata]
	mov	ecx, [ecx+__gai_reqdata.socket]
	mcall	74, 1
	pop	eax
; 16. Restore used registers, destroy stack frame and return.
.ret:
	pop	edi esi ebx
	mov	esp, ebp
	pop	ebp
	ret	8

;;===========================================================================;;
;; Internal auxiliary function for skipping names in DNS packet.             ;;
.skip_name:								     ;;
;;---------------------------------------------------------------------------;;
;? Skips name in DNS packet.                                                 ;;
;;---------------------------------------------------------------------------;;
;> esi -> name                                                               ;;
;> ecx = end of packet                                                       ;;
;;---------------------------------------------------------------------------;;
;< esi -> end of name                                                        ;;
;;===========================================================================;;
	xor	eax, eax
	cmp	esi, ecx
	jae	.skip_name.done
	lodsb
	test	al, al
	jz	.skip_name.done
	test	al, 0xC0
	jnz	.skip_name.pointer
	add	esi, eax
	jmp	.skip_name
.skip_name.pointer:
	inc	esi
.skip_name.done:
	ret

;;===========================================================================;;
;; Internal auxiliary function for calculating length of name in DNS packet. ;;
.get_name_length:							     ;;
;;---------------------------------------------------------------------------;;
;? Calculate length of name (including terminating zero) in DNS packet.      ;;
;;---------------------------------------------------------------------------;;
;> edx = start of packet                                                     ;;
;> esi -> name                                                               ;;
;> ecx = end of packet                                                       ;;
;;---------------------------------------------------------------------------;;
;< eax = length of name                                                      ;;
;< CF set on error / cleared on success                                      ;;
;;===========================================================================;;
	xor	ebx, ebx	; ebx will hold data length
.get_name_length.zero:
	xor	eax, eax
.get_name_length.loop:
	cmp	esi, ecx
	jae	.get_name_length.fail
	lodsb
	test	al, al
	jz	.get_name_length.done
	test	al, 0xC0
	jnz	.get_name_length.pointer
	add	esi, eax
	inc	ebx
	add	ebx, eax
	cmp	ebx, 256
	jbe	.get_name_length.loop
.get_name_length.fail:
	stc
	ret
.get_name_length.pointer:
	and	al, 0x3F
	mov	ah, al
	lodsb
	lea	esi, [edx+eax]
	jmp	.get_name_length.zero
.get_name_length.done:
	test	ebx, ebx
	jz	.get_name_length.fail
	xchg	eax, ebx
	clc
	ret

;;===========================================================================;;
;; Internal auxiliary function for decoding DNS name.                        ;;
.decode_name:								     ;;
;;---------------------------------------------------------------------------;;
;? Decode name in DNS packet.                                                ;;
;;---------------------------------------------------------------------------;;
;> edx = start of packet                                                     ;;
;> esi -> name in packet                                                     ;;
;> edi -> buffer for decoded name                                            ;;
;;===========================================================================;;
	xor	eax, eax
	lodsb
	test	al, al
	jz	.decode_name.done
	test	al, 0xC0
	jnz	.decode_name.pointer
	mov	ecx, eax
	rep	movsb
	mov	al, '.'
	stosb
	jmp	.decode_name
.decode_name.pointer:
	and	al, 0x3F
	mov	ah, al
	lodsb
	lea	esi, [edx+eax]
	jmp	.decode_name
.decode_name.done:
	mov	byte [edi-1], 0
	ret

;;===========================================================================;;
;; Internal auxiliary function for allocating memory for getaddrinfo.        ;;
getaddrinfo._.memalloc: 						     ;;
;;---------------------------------------------------------------------------;;
;? Memory allocation.                                                        ;;
;;---------------------------------------------------------------------------;;
;> eax = size in bytes, must be less than page size.                         ;;
;> [ebp-4] = .recent_page = last allocated page                              ;;
;> [ebp-8] = .recent_restsize = bytes rest in last allocated page            ;;
;;---------------------------------------------------------------------------;;
;< edi -> allocated memory / NULL on error                                   ;;
;;===========================================================================;;
; 1. Set edi to result of function.
	mov	edi, [ebp-4]
; 2. Check whether we need to allocate a new page.
	cmp	eax, [ebp-8]
	jbe	.no_new_page
; 2. Allocate new page if need. Reset edi to new result.
	push	eax ebx
	mcall	68, 12, 0x1000
	xchg	edi, eax	; put result to edi
	pop	ebx eax
; 3. Check returned value of allocator. Fail if it failed.
	test	edi, edi
	jz	.ret
; 4. Update .recent_page and .recent_restsize.
	add	edi, 4
	sub	ecx, 4
	mov	[ebp-4], edi
	mov	[ebp-8], ecx
.no_new_page:
; 5. Increase number of objects on this page.
	push	eax
	mov	eax, edi
	and	eax, not 0xFFF
	inc	dword [eax]
	pop	eax
; 6. Advance last allocated pointer, decrease memory size.
	add	[ebp-4], eax
	sub	[ebp-8], eax
; 7. Return.
.ret:
	ret

;;===========================================================================;;
;; Internal auxiliary function for freeing memory for freeaddrinfo.          ;;
getaddrinfo._.memfree:							     ;;
;;---------------------------------------------------------------------------;;
;? Free memory.                                                              ;;
;;---------------------------------------------------------------------------;;
;> eax = pointer                                                             ;;
;;===========================================================================;;
; 1. Get start of page.
	mov	ecx, eax
	and	ecx, not 0xFFF
; 2. Decrease number of objects.
	dec	dword [ecx]
; 3. If it goes to zero, free the page.
	jnz	@f
	push	ebx
	mcall	68, 13
	pop	ebx
@@:
; 4. Done.
	ret

;;===========================================================================;;
getaddrinfo._.generate_data:						     ;;
;;---------------------------------------------------------------------------;;
;? Generate item(s) of getaddrinfo result list by one IP address.            ;;
;;---------------------------------------------------------------------------;;
;> eax = IP address                                                          ;;
;> ebx = request data                                                        ;;
;> esi = pointer to result                                                   ;;
;> [ebp-4] = .recent_page = last allocated page                              ;;
;> [ebp-8] = .recent_restsize = bytes rest in last allocated page            ;;
;;---------------------------------------------------------------------------;;
;< esi = pointer to next list item for result / NULL on error                ;;
;;===========================================================================;;
; 1. If no service is given, append one item with zero port.
; append one item with zero socktype/protocol/port.
	cmp	[ebx+__gai_reqdata.service], -1
	jnz	.has_service
	call	.append_item
; 1a. If neither protocol nor socktype were specified,
;       leave zeroes in socktype and protocol.
	mov	cl, [ebx+__gai_reqdata.socktype]
	test	cl, cl
	jz	.no_socktype
; 1b. Otherwise, set socktype and protocol to desired.
	call	.set_socktype
.no_socktype:
	ret
.has_service:
; 2. If TCP is allowed, append item for TCP.
	cmp	[ebx+__gai_reqdata.socktype], 0
	jz	.tcp_ok
	cmp	[ebx+__gai_reqdata.socktype], SOCK_STREAM
	jnz	.tcp_disallowed
.tcp_ok:
	call	.append_item
	mov	cl, SOCK_STREAM
	call	.set_socktype
	call	.set_port
.tcp_disallowed:
; 3. If UDP is allowed, append item for UDP.
	cmp	[ebx+__gai_reqdata.socktype], 0
	jz	.udp_ok
	cmp	[ebx+__gai_reqdata.socktype], SOCK_DGRAM
	jnz	.udp_disallowed
.udp_ok:
	call	.append_item
	mov	cl, SOCK_DGRAM
	call	.set_socktype
	call	.set_port
.udp_disallowed:
	ret

.append_item:
; 1. Allocate memory for struct sockaddr_in and struct addrinfo.
	push	eax
	push	sizeof.addrinfo + sizeof.sockaddr_in
	pop	eax
	call	getaddrinfo._.memalloc
; 2. Check for memory allocation fail.
	test	edi, edi
	jz	.no_memory
; 3. Zero allocated memory.
	push	(sizeof.addrinfo + sizeof.sockaddr_in) / 4
	pop	ecx
	xor	eax, eax
	push	edi
	rep	stosd
	pop	edi
; 4. Fill struct addrinfo.
	mov	eax, [ebx+__gai_reqdata.flags]
	mov	[edi+addrinfo.ai_flags], eax
	mov	byte [edi+addrinfo.ai_family], PF_INET
	mov	byte [edi+addrinfo.ai_addrlen], sizeof.sockaddr_in
	lea	ecx, [edi+sizeof.addrinfo]
	mov	[edi+addrinfo.ai_addr], ecx
; 5. Fill struct sockaddr_in.
	mov	byte [ecx+sockaddr_in.sin_family], PF_INET
	pop	eax
	mov	[ecx+sockaddr_in.sin_addr], eax
; 6. Append new item to the list.
	mov	[esi], edi
	lea	esi, [edi+addrinfo.ai_next]
; 7. Return.
	ret
.no_memory:
	pop	eax
	xor	esi, esi
	ret

.set_socktype:
; Set ai_socktype and ai_protocol fields by given socket type.
	mov	byte [edi+addrinfo.ai_socktype], cl
	dec	cl
	jnz	.set_udp
.set_tcp:
	mov	byte [edi+addrinfo.ai_protocol], IPPROTO_TCP
	ret
.set_udp:
	mov	byte [edi+addrinfo.ai_protocol], IPPROTO_UDP
	ret

.set_port:
; Just copy port from input __gai_reqdata to output addrinfo.
	push	edx
	mov	edx, [ebx+__gai_reqdata.service]
	xchg	dl, dh	; convert to network byte order
	mov	[edi+sizeof.addrinfo+sockaddr_in.sin_port], dx
	pop	edx
	ret

;;===========================================================================;;
;; void __stdcall getaddrinfo_abort(__in struct __gai_reqdata* reqdata);      ;;
getaddrinfo_abort:							     ;;
;;---------------------------------------------------------------------------;;
;? Abort process started by getaddrinfo_start, free all resources.           ;;
;;---------------------------------------------------------------------------;;
;> first parameter = pointer to struct __gai_reqdata filled by ..._start     ;;
;;===========================================================================;;
; 0. Save used registers for __stdcall.
	push	ebx
; 1. Allocated resources: only socket, so close it and return.
	mov	eax, [esp+8]
	mov	ecx, [eax+__gai_reqdata.socket]
	mcall	74, 1
; 2. Restore used registers and return.
	pop	ebx
	ret	4

;;===========================================================================;;
;; void __stdcall freeaddrinfo(__in struct addrinfo* ai);                    ;;
freeaddrinfo:								     ;;
;;---------------------------------------------------------------------------;;
;? Free one or more addrinfo structures returned by getaddrinfo.             ;;
;;---------------------------------------------------------------------------;;
;> first parameter = head of list of structures                              ;;
;                    (may be arbitrary sublist of original)                  ;;
;;===========================================================================;;
; 1. Loop for all items in the list.
	mov	edx, [esp+4]	; eax = ai
.loop:
	test	edx, edx
	jz	.done
; 2. Free each item.
; 2a. Free ai_canonname, if allocated.
	mov	eax, [edx+addrinfo.ai_canonname]
	test	eax, eax
	jz	.no_canon_name
	call	getaddrinfo._.memfree
.no_canon_name:
; 2b. Remember next item
;       (after freeing the field ai_next can became unavailable).
	pushd	[edx+addrinfo.ai_next]
; 2c. Free item itself.
	xchg	eax, edx
	call	getaddrinfo._.memfree
; 2d. Restore pointer to next item and continue loop.
	pop	edx
	jmp	.loop
.done:
; 3. Done.
	ret	4

;;===========================================================================;;
;;///////////////////////////////////////////////////////////////////////////;;
;;===========================================================================;;
;! Exported functions section                                                ;;
;;===========================================================================;;
;;///////////////////////////////////////////////////////////////////////////;;
;;===========================================================================;;


align 4
@EXPORT:
export	\
	lib_init		, 'lib_init'		, \
	0x00010001		, 'version'		, \
	inet_addr		, 'inet_addr'		, \
	inet_ntoa		, 'inet_ntoa'		, \
	getaddrinfo		, 'getaddrinfo' 	, \
	getaddrinfo_start	, 'getaddrinfo_start'	, \
	getaddrinfo_process	, 'getaddrinfo_process' , \
	getaddrinfo_abort	, 'getaddrinfo_abort'	, \
	freeaddrinfo		, 'freeaddrinfo'

; import from libini
align 4
@IMPORT:

library libini, 'libini.obj'
import	libini, \
	ini.get_str, 'ini_get_str',	\
	ini.get_int, 'ini_get_int'


section '.data' data readable writable align 16
; uninitialized data
mem.alloc   dd ?
mem.free    dd ?
mem.realloc dd ?
dll.load    dd ?

DNSrequestID	dd	?

inet_ntoa.buffer	rb	16	; static buffer for inet_ntoa
