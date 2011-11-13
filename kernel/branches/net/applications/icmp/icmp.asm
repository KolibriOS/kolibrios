use32
	org	0x0
; standard header
	db	'MENUET01'	; signature
	dd	1		; header version
	dd	start		; entry point
	dd	I_END		; initialized size
	dd	mem		; required memory
	dd	mem		; stack pointer
	dd	0		; parameters
	dd	0		; path


BUFFERSIZE	equ 1500
; useful includes
include '../macros.inc'
purge mov,add,sub
include '../proc32.inc'
include '../dll.inc'

include '../network.inc'



; ICMP types & codes

ICMP_ECHOREPLY			equ 0		    ; echo reply message

ICMP_UNREACH			equ 3
ICMP_UNREACH_NET		equ  0		     ; bad net
ICMP_UNREACH_HOST		equ  1		     ; bad host
ICMP_UNREACH_PROTOCOL		equ  2		     ; bad protocol
ICMP_UNREACH_PORT		equ  3		     ; bad port
ICMP_UNREACH_NEEDFRAG		equ  4		     ; IP_DF caused drop
ICMP_UNREACH_SRCFAIL		equ  5		     ; src route failed
ICMP_UNREACH_NET_UNKNOWN	equ  6		     ; unknown net
ICMP_UNREACH_HOST_UNKNOWN	equ  7		     ; unknown host
ICMP_UNREACH_ISOLATED		equ  8		     ; src host isolated
ICMP_UNREACH_NET_PROHIB 	equ  9		     ; prohibited access
ICMP_UNREACH_HOST_PROHIB	equ 10		    ; ditto
ICMP_UNREACH_TOSNET		equ 11		    ; bad tos for net
ICMP_UNREACH_TOSHOST		equ 12		    ; bad tos for host
ICMP_UNREACH_FILTER_PROHIB	equ 13		    ; admin prohib
ICMP_UNREACH_HOST_PRECEDENCE	equ 14		   ; host prec vio.
ICMP_UNREACH_PRECEDENCE_CUTOFF	equ 15		 ; prec cutoff

ICMP_SOURCEQUENCH		equ 4		    ; Packet lost, slow down

ICMP_REDIRECT			equ 5		    ; shorter route, codes:
ICMP_REDIRECT_NET		equ  0		     ; for network
ICMP_REDIRECT_HOST		equ  1		     ; for host
ICMP_REDIRECT_TOSNET		equ  2		     ; for tos and net
ICMP_REDIRECT_TOSHOST		equ  3		     ; for tos and host

ICMP_ALTHOSTADDR		equ 6		    ; alternate host address
ICMP_ECHO			equ  8		     ; echo service
ICMP_ROUTERADVERT		equ  9		     ; router advertisement
ICMP_ROUTERADVERT_NORMAL	equ  0			; normal advertisement
ICMP_ROUTERADVERT_NOROUTE_COMMON equ 16 	; selective routing

ICMP_ROUTERSOLICIT		equ 10		    ; router solicitation
ICMP_TIMXCEED			equ 11		    ; time exceeded, code:
ICMP_TIMXCEED_INTRANS		equ 0		    ; ttl==0 in transit
ICMP_TIMXCEED_REASS		equ 1		    ; ttl==0 in reass

ICMP_PARAMPROB			equ 12		     ; ip header bad
ICMP_PARAMPROB_ERRATPTR 	equ 0		 ; error at param ptr
ICMP_PARAMPROB_OPTABSENT	equ 1		 ; req. opt. absent
ICMP_PARAMPROB_LENGTH		equ 2		 ; bad length

ICMP_TSTAMP			equ 13		    ; timestamp request
ICMP_TSTAMPREPLY		equ 14		    ; timestamp reply
ICMP_IREQ			equ 15		    ; information request
ICMP_IREQREPLY			equ 16		    ; information reply
ICMP_MASKREQ			equ 17		    ; address mask request
ICMP_MASKREPLY			equ 18		    ; address mask reply
ICMP_TRACEROUTE 		equ 30		    ; traceroute
ICMP_DATACONVERR		equ 31		    ; data conversion error
ICMP_MOBILE_REDIRECT		equ 32		    ; mobile host redirect
ICMP_IPV6_WHEREAREYOU		equ 33		    ; IPv6 where-are-you
ICMP_IPV6_IAMHERE		equ 34		    ; IPv6 i-am-here
ICMP_MOBILE_REGREQUEST		equ 35		    ; mobile registration req
ICMP_MOBILE_REGREPLY		equ 36		    ; mobile registreation reply
ICMP_SKIP			equ 39		    ; SKIP

ICMP_PHOTURIS			equ 40		    ; Photuris
ICMP_PHOTURIS_UNKNOWN_INDEX	equ 1		     ; unknown sec index
ICMP_PHOTURIS_AUTH_FAILED	equ 2		     ; auth failed
ICMP_PHOTURIS_DECRYPT_FAILED	equ 3		     ; decrypt failed



virtual at 0
	ICMP_Packet:
	.Type		db   ?
	.Code		db   ?
	.Checksum	dw   ?
	.Identifier	dw   ?
	.SequenceNumber dw   ?
	.Data:
end virtual


; entry point
start:
; load libraries
	stdcall dll.Load, @IMPORT
	test	eax, eax
	jnz	exit
; initialize console
	push	1
	call	[con_start]
	push	title
	push	25
	push	80
	push	25
	push	80
	call	[con_init]
; main loop
	push	str1
	call	[con_write_asciiz]
main:
; write prompt
	push	str2
	call	[con_write_asciiz]
; read string
	mov	esi, s
	push	256
	push	esi
	call	[con_gets]
; check for exit
	test	eax, eax
	jz	done
	cmp	byte [esi], 10
	jz	done
; delete terminating '\n'
	push	esi
@@:
	lodsb
	test	al, al
	jnz	@b
	mov	byte [esi-2], al
	pop	esi
; resolve name
	push	esp	; reserve stack place
	push	esp	; fourth parameter
	push	0	; third parameter
	push	0	; second parameter
	push	esi	; first parameter
	call	[getaddrinfo]
	pop	esi
; test for error
	test	eax, eax
	jnz	fail

; convert IP address to decimal notation
	mov	eax, [esi+addrinfo.ai_addr]
	mov	eax, [eax+sockaddr_in.sin_addr]
	mov	[sockaddr1.ip], eax
	push	eax
	call	[inet_ntoa]
; write result
	mov	[ip_ptr], eax

	push	eax

; free allocated memory
	push	esi
	call	[freeaddrinfo]

	push	str4
	call	[con_write_asciiz]

	mcall	socket, AF_INET4, SOCK_RAW, IPPROTO_ICMP
	cmp	eax, -1
	jz	fail2
	mov	[socketnum], eax

	mcall	connect, [socketnum], sockaddr1, 18

	mcall	40, 1 shl 7 ; + 7
;        call    [con_cls]

	mov	[count], 4

mainloop:
	push	str3
	call	[con_write_asciiz]
	push	[ip_ptr]
	call	[con_write_asciiz]

	mcall	26,9
	mov	[time_reference], eax
	mcall	send, [socketnum], icmp_packet, icmp_packet.length, 0

	mcall	23, 300 ; 3 seconds time-out
	mcall	26,9
	neg	[time_reference]
	add	[time_reference], eax

	mcall	recv, [socketnum], buffer_ptr, BUFFERSIZE, 0
	cmp	eax, -1
	je	.no_response

; validate the packet
	lea	esi, [buffer_ptr + ICMP_Packet.Data]
	mov	edi, icmp_packet.data
	mov	ecx, 32/4
	repe	cmpsd
	jne	.miscomp

	push	[time_reference]
	push	str7
	call	[con_printf]

	jmp	continue

  .miscomp:
	sub	edi, icmp_packet.data
	push	edi
	push	str9
	call	[con_printf]
	jmp	continue

  .no_response:
	push	str8
	call	[con_write_asciiz]

   continue:
	dec	[count]
	jz	done
	mcall	5, 100	; wait a second
	inc	[icmp_packet.id]
	jmp	mainloop



done:
	push	str10
	call	[con_write_asciiz]
	call	[con_getch2]
	push	1
	call	[con_exit]
exit:
	mcall	-1

fail:
	push	str5
	call	[con_write_asciiz]
	jmp	done
fail2:
	push	str6
	call	[con_write_asciiz]
	jmp	done


; data
title	db	'ICMP - test application',0
str1	db	'ICMP test application v0.1',10,' for KolibriOS # 1540 or later. ',10,10,0
str2	db	'> ',0
str3	db	'Ping to: ',0
str4	db	10,0
str5	db	'Name resolution failed.',10,10,0
str6	db	'Could not open socket',10,10,0
str7	db	' time= %u0ms',10,0
str8	db	' timeout!',10,0
str9	db	' miscompare at offset %u',10,0
str10	db	10,10,'Press any key to exit',0

sockaddr1:
	dw AF_INET4
.port	dw 0
.ip	dd 0
	rb 10

time_reference	dd ?
ip_ptr		dd ?
count		dd ?


; import
align 4
@IMPORT:

library network, 'network.obj', console, 'console.obj'
import	network,	\
	getaddrinfo,	'getaddrinfo',	\
	freeaddrinfo,	'freeaddrinfo', \
	inet_ntoa,	'inet_ntoa'

import	console,	\
	con_start,	'START',	\
	con_init,	'con_init',	\
	con_write_asciiz,	'con_write_asciiz',	\
	con_printf,	  'con_printf',     \
	con_exit,	'con_exit',	\
	con_gets,	'con_gets',\
	con_cls,	'con_cls',\
	con_getch2,	'con_getch2',\
	con_set_cursor_pos, 'con_set_cursor_pos'

socketnum	dd ?

icmp_packet:	db 8		; type
		db 0		; code
		dw 0		;
 .id		dw 0x0000	; identifier
 .seq		dw 0x0001	; sequence number
 .data		db 'abcdefghijklmnopqrstuvwxyz012345678'
 .length = $ - icmp_packet

I_END:

buffer_ptr	rb BUFFERSIZE

s	rb 256
align	4
rb	4096	; stack
mem:
