;
;  SNTP client for KolibriOS
;
;  (C) 2019 Coldy	
;  Thank's you for use this code and software based on it!
;  I will glad if it's will be helpful. 
;
;  Distributed under terms of GPL
;

format binary as ""

use32
	org	0x0

	db	'MENUET01'	 ; signature
	dd	1		         ; header version
	dd	START		     ; entry point
	dd	I_END		     ; initialized size
	dd	MEM	         ; required memory
	dd	STACKTOP	   ; stack pointer
	dd	params		   ; parameters
	dd	0		         ; path
  
__DEBUG__	= 1
__DEBUG_LEVEL__ = 2

include '../../proc32.inc'
include '../../macros.inc'
include '../../dll.inc'
include '../../struct.inc'
include '../../network.inc'
include '../../debug-fdo.inc'
include 'time.inc'


START:
  
; init heap
	mcall	68, 11
	test	eax, eax
  ; fatal error (not enough memory)
	jz	exit_now
  
; load libraries
	stdcall dll.Load, @IMPORT
	test	eax, eax
  ; fatal error(imports not loaded)
	jnz	exit_now
  
; initialize console
;	push	1
;	call	[con_start]
	push	str_title
	push	250
	push	80
	push	25
	push	80
	call	[con_init]
  ;test	eax, eax
  ; fatal error(console error)
	;jnz	exit

  ; setup params
  call parse_params
  ;clear eax
  call tz_validate
  ; is TZ correct?
  cmp ebx,0
  je @f
  mov   eax, ebx
  mov   ebx, str_err11
  jmp .tz_error
@@:
  ; is command line correct?
  cmp eax, 10
  jne @f
  mov   ebx, str_err10
  jmp .error
@@:
  ; empty command line (need help)?
  cmp eax, 0
  je @f
  cinvoke	con_printf, str_help
  jmp exit

@@:
  ; Prepare to do query time
  ; Convert host name to IP address
  invoke inet_addr, params
  ; Host name by IP address was provided?
  cmp	  eax, -1
  jne	   .resolved  
  
  push	  esp	  ; reserve stack place
    
  invoke getaddrinfo, params, 0, 0, esp
  
  ; Get ptr to result addrinfo struct
  pop	  esi
  
  ; Test for error  
  test	  eax, eax
  jz    @f
  mov   eax, 30
  mov   ebx, str_err3
  ;ret ; Error: Name not resolved!
  jmp .error
  
  @@: 
  mov	  eax, [esi+addrinfo.ai_addr]
  mov	  eax, [eax+sockaddr_in.sin_addr]
  
  push eax ; Store IP to stack
  
  invoke  inet_ntoa, eax
  ; Store string of IP
  mov edx, eax
  pop eax ; Load IP from stack
  jmp @f
  
.resolved:
  clear edx ; mark host is IP format
@@:
  mov	  [sockaddr1.ip], eax
  
  clear ebx
  mov bx, [port]
  
  ;cinvoke	con_printf, str_query, params, eax, ebx
  cinvoke	con_printf, str_query, params
  cmp edx,0
  je @f ; Skip IP display
  cinvoke	con_printf, str_ip, edx
@@:
  cinvoke	con_printf, str_port, ebx
  
  ; free allocated memory
  invoke  freeaddrinfo, esi
  
  ; Now we ready to query server   
  call sntp_query_time
  cmp eax,1
  jne @f
  mov eax, 59
  jmp .warning
@@:
  cmp eax,2
  jne @f
  mov eax, 61
.warning:
  cinvoke	con_printf, str_warn, eax
  jmp .display
@@:
  cmp eax, 41
  jge .error
 
.display: 
  ; Tell user results
  
  ;mov esi, datetime ; ?
  
  ; Display server date and time  
  xor eax, eax 
  mov al, [esi + DateTime.sec]
  push eax
  mov al, [esi + DateTime.min]
  push eax
  mov al, [esi + DateTime.hour] 
  push eax
  mov ax, [esi + DateTime.year]
  push eax
  
  xor eax, eax
  mov al, [esi + DateTime.month]
  push eax
  mov al, [esi + DateTime.day]
  push eax
  
  push str_dt
  
  call [con_printf]  
  add  esp, 7*4
  
  ; Display timezone
  
  cmp [tz_h],0
  jne @f
  cmp [tz_m],0
  jne @f
  mov eax, str_tz
  add eax, 9  
  cinvoke con_printf, eax ; \n\0 
  jmp .no_bias
  
@@:
  clear eax, ebx
  mov  ecx, str_tz
  mov al, [tz_h]
  test al, al
  jns @f
  mov byte[ecx+1],'-'    ; Change sign
  sub bl, al
  mov al, bl
  
@@:
  cmp [tz_m],0
  jne @f
  mov word[ecx+4],10 ; \n\0
    
@@:  
  mov bl, [tz_m]
   
  cinvoke con_printf, str_tz, eax, ebx
  
.no_bias:
  cmp [sync],0
  je  exit
  cmp [sync], SYNC_S
  jne @f
  mov eax, str_s
  jmp .sync_ok
@@:
  cmp [sync], SYNC_ST
  jne @f
  mov eax, str_st
  jmp .sync_ok
  @@:
  cmp [sync], SYNC_SS
  jne exit ; Fixed (24.04.2019): incorrect display with -ss
  mov eax, str_ss
  
.sync_ok:
  cinvoke	con_printf, str_sync, eax
  
  jmp exit
  
.error:
  mov esi, params
  
.tz_error:
  ; Note: esi assign by parse_params

;  cmp [notify],0
;  jne  @f
  cinvoke con_printf, str_err, eax, esi, ebx
  ;jmp exit
;@@:

  ; Do call @NOTIFY


  ; Finally... exit!
exit:
	push	0
	call	[con_exit]

exit_now:
	mcall	-1
  
; End of program

;notify:

; Time zone check helper
tz_validate:
;jmp .exit
  cmp [tz_h],-12
  jl .fail
  cmp [tz_h],14
  jg .fail
  
  cmp [tz_m],0
  je  .exit
  cmp [tz_m],30
  jne .tz_45m
  cmp [tz_h],-9
  je .exit
  cmp [tz_h],-3
  je .exit
  cmp [tz_h],3
  jl .fail   
  je .exit
  cmp [tz_h],6
  jg @f   
  jle .exit
@@:
  cmp [tz_h],9
  je .exit
  cmp [tz_h],10
  jne .fail
  je  .exit
     
.tz_45m:
  cmp [tz_m],45
  jne .fail
  cmp [tz_h],5
  je .exit
  cmp [tz_h],8
  je .exit
  cmp [tz_h],12
  jne .fail
.exit: 
  clear ebx
  ret
   
.fail:
  mov ebx,11  
  ret
  
 
; Sycronization constants  
SYNC_S	  = 1
SYNC_SS   = 2
SYNC_ST   = 3

parse_params:
  mov	  esi, params
  mov   ebx, esi
  clear ecx ; 26.04.2018 Fixed 
.f00:
	lodsb
  cmp	 al, 0
	jne	.f01
  
  dec esi
  cmp esi, ebx
  jne @f
  ;no params
  mov eax, -1
  ret
    
.exit:
  cmp ecx,0 ; 26.04.2018 Fixed
  je @f 
  ; mark end of TZ
  mov	  byte [ecx+1],  0
  ; now esi = start of TZ
  mov esi,ebp

@@:  
  mov eax, 0
  ret

.f01:
  cmp	  al, ' '
	jne	.f00
  
  ; Save end of host position
  mov edi, esi
  dec edi
  ;mov	  byte [esi-1],  0
	jmp	.param

 .param_loop:
	lodsb
	cmp		 al, 0
  je @f
  cmp	 al, ' '
	jne    .invalid
	jmp   .param
@@:
  mov	  byte [edi],  0
  jmp .exit ;ret
  
 .param:
  lodsb
	cmp	al, '-'
	jne	.invalid

	lodsb
  
;  cmp	  al, 'n'
;  jne   @f
;  mov   [notify],1
;  jmp    .param_loop
;@@:  
; cmp	  al, 'p'
;	je		.p
	cmp	al, 't'
	jne		@f
	lodsb
	cmp	al, 'z'
	jne	.invalid
	je	.tz
@@:
	cmp	al, 's'
  je	  .sync
	
;.p:
  ; port setup
;	lodsb
;	cmp	al, ' '
;	jne	.invalid
;	call	c2n
;	test	ebx, ebx
; jz	  .invalid
; mov	  [port], bx
;	jmp .param_loop
jmp .invalid
  
.tz:  
  ; tz setup
	lodsb
	cmp	al, ' '
	jne	.invalid
  
  ; save start of TZ
  ;push esi
  mov ebp, esi 
  call	  c2n
  ;cmp	  ebx, ebx  ; 0 is possible
  ;jz	  .invalid
  mov	  [tz_h], bl
	cmp	al, ':'
	je		  .tz_m
	;dec		  esi 
	jmp    @f;.param_loop

.tz_m:
	call	c2n
  ;test	  ebx, ebx  ; 0 is possible
  ;jz	  .invalid
  mov	  [tz_m], bl
@@:
  ; save end of TZ
  ;push esi
  mov ecx, esi
  jmp .param_loop

  .sync:
  ; sync setup
  lodsb
  cmp	  al, 's'
	jne		.st
  mov	  [sync], SYNC_SS
	jmp		.param_loop
  .st:
  cmp	  al, 't'
	jne		.s
  mov	  [sync], SYNC_ST
	jmp		.param_loop
.s:
  mov	  [sync], SYNC_S
  dec	esi
	jmp		.param_loop

.invalid:
	mov eax, 10
	ret
  

; Helper to convert char to number
; Input:
; esi  - ptr to char of digit
; Output:
; ebx - number
; Use registers (not restore):
; eax, edx
;  
c2n:
  xor	  eax, eax
  xor	  ebx, ebx
	xor	edx, edx
  .loop:
  lodsb
  test	  al, al
  jz	  .done
  cmp	  al, ' '
  je	  .done

	cmp	al, ':'
	;jne		.f0
  je .done1

	;.f0:
	cmp		al, '+'
	je		.f00
	cmp		al, '-'
	jne		.f01
	mov		dl,1
  .f00:
	lodsb
  .f01:
 
  sub	  al, '0'
  jb	  .fail
  cmp	  al, 9
  ja	  .fail
  lea	  ebx, [ebx*4+ebx]
  lea	  ebx, [ebx*2+eax]
  jmp	  .loop
  .fail:
  xor	  ebx, ebx
  .done:
  dec	  esi
 .done1:
	cmp		dl, 1
	jne		.ret
	neg		ebx
.ret:
  ret
 
 
; Sync worker
; Input:
; setuped sockaddr1
; Output:
; eax - error_code
; ebx - error_string
; esi  - ptr to DateTime
; Use registers (not restore):
; eax, edx, ecx, edx, esi,...
;  
;sntp_sync_time:
  ;mov	   edx, eax  
sntp_query_time:

  ; if -ss & 59:59 => waiting for new hour
  cmp [sync], SYNC_SS
  jne @f
.new_hour?:  
  ; Query system time
  mcall   3
  cmp ah, 59h ; 59 min. ?
  jne @f
  shr eax, 16
  cmp al, 59h ; 59 sec. ?
  jne @f
  ;DEBUGF  1, "SNTP: Waiting for new hour.\n"
  ; Wait 100 msec.
  mcall 5,10
  jmp .new_hour? 
@@:  
  ; Create socket
  mcall   socket, AF_INET4, SOCK_DGRAM, IPPROTO_IP
  cmp	  eax, -1
  jne   @f
  mov   eax, 41
  mov   ebx, str_err4
  ret ; Connection error (1)
     
@@:
  ;mov	  [socketnum], eax
  ;mcall   connect, [socketnum], sockaddr1, 18
  mov   ebp, eax  ; Store socket
  
  mcall   connect, ebp, sockaddr1, 18
  cmp	  eax, -1
  jne   @f
  mov   edx, 42
  mov   edi, str_err4
  jmp .error ; Connection error (2)
  ;DEBUGF  1, "Socket connected.\n"
  
@@:   
  mcall   send, ebp, sntp_packet, SIZEOF_SNTP_PACKET, 0
  cmp	  eax, -1
  jne   @f
  mov   edx, 43
  mov   edi, str_err4
  jmp .error ; Connection error (3)
  ;DEBUGF  1, "send done.\n"
  
@@:
  ; Wait 300 msec.
  mcall   5, 30
;do_recv:
  mcall   recv, ebp, sntp_packet, SIZEOF_SNTP_PACKET, MSG_DONTWAIT
  
  cmp	  eax, -1
  jne   @f
  mov   edx, 50
  mov   edi, str_err5  
  jmp .error ; no response
  @@:
  test    eax, eax
  jnz     @f
  mov   edx, 44
  mov   edi, str_err4
  jmp .error ; ; Connection error (4)

  ;DEBUGF  1, "recv done.\n"
  
@@:
  ; Kiss of death?
  cmp  [sntp_packet.Stratum], 0
  jne   @f
  mov   edx, 60
  mov   edi, str_err6
  jmp .error
  
@@:
  ;  cmp   eax, SIZEOF_SNTP_PACKET
;  jne   do_recv 
  
  ;push   sntp_packet.ReferenceID
  ;call   [con_write_asciiz]
  ;invoke	con_write_asciiz, str_refid
  
  ; TODO: calñ roudtrip
  mov	  eax, [sntp_packet.TransmitTime]
  bswap   eax
  
  ; Bias between epoch 
  sub	  eax, 0x83AA7E80  
  
  ;push eax
  
  ;cinvoke  con_printf, str_t, eax
  ;cinvoke  con_printf, str_tt, [sntp_packet.TransmitTime], [sntp_packet.TransmitTime + 32]
  
  ;pop eax
  
;.test:
  mov ebx, datetime
;  mov eax, 7fffffffh ; max timestamp
  call timestamp2DateTime
 
    
;  mov esi, datetime
  
  ; Calc time zone (hour only)
;  mov eax, [SystemTime]
;  clear ecx
;  mov	 cl, al
;  clear eax
;  mov al, [esi + DateTime.hour]
;  push ecx
;  b2bcd
;  pop ecx 
;  sub cl, al
  ;sub bh, [esi + DateTime.min]  
  
  ; correct minutes
  clear eax, ebx, ecx    
  mov al, [tz_m]
  test al, al      ; tz_m = 0 ?
  jz .tz_h
  mov bl, [tz_h]  ; tz_m < 0 ? This not work with tz_h = 0!
  test bl,bl 
  jns @f
  sub [esi + DateTime.min], al
  cmp [esi + DateTime.min], 0
  jge @f 
  mov al, 60
  mov cl, -1 
 
  add [esi + DateTime.min], al;bh
  ;mov [TimeZone], cx
  ;@@:
@@:
.tz_h:  
  ; correct hour    
;  cmp [sync], SYNC_SS
  ; if -ss ignore timezone for hour
;  je .tz_done
  clear eax
  mov al, [tz_h]
  add al, cl   
  add [esi + DateTime.hour], al ;3 ; MSK = GMT +3
  
  ; Correct day & hour if prev/new day
  ; hour < 0 ?
  mov   al,[esi + DateTime.hour]
  test  al, al
  jns @f
  add [esi + DateTime.hour],24
  dec [esi + DateTime.day]
  jmp .tz_done    
  
@@:         
  ; hour >= 24 ?
  cmp [esi + DateTime.hour], 24
  jl .tz_done
  inc [esi + DateTime.day]
  mov bl, [esi + DateTime.hour]
  sub ebx, 24 
  mov [esi + DateTime.hour],bl
   
.tz_done:

  ; {{
  ; Removed block 1.1 
  ;}}
  
  ;clear eax, ebx, ecx
  ;mov	 al, '+'
  ;mov ax, [TimeZone]
  ;mov bl, al
  ;mov cl, ah
  
  ; {{
  ; Removed block 1.2
  ;}}
  
  ; FIXED:  do sync before display!!!
  ;         It's need to do sync fast ASAP 
  ;         Take out any printf from sntp_query_time! 
  ; sync > 0 ?
  cmp [sync], 0
  je .nosync
  
  ;{{ 
  ; Removed block 2
  ; FIXME: Go it from sntp_query_time!
  ;}}
  
  
  
	; Convert time to BCD   
  clear eax, edx 
  mov al, [esi + DateTime.sec]
  b2bcd
  mov ecx, eax
  shl ecx, 16
  mov al, [esi + DateTime.min]
  b2bcd  
  mov ch, al
  
  cmp [sync], SYNC_SS
  jne @f
  ; if -ss ignore timezone for hour
  ; Query system time
  mcall   3  
  jmp .ss_done
@@:   
  mov al, [esi + DateTime.hour]
  b2bcd
.ss_done:
  mov cl, al  
  
  ; Display BCD time 
  ;cinvoke con_printf, str_t, ecx
  
;  mov ecx, [ebx + DateTime.date]
;  push ecx
;  push str_d
;  call [con_printf]
;  add	esp, 2*4 

 
  ; Set time 
  mov eax, 22
  mov ebx, 0
  int 0x40
  
  ; ? error
  
  cmp [sync], SYNC_S
  jne .nosync
  ; Convert date to BCD
  mov esi, datetime
  clear eax, edx 
  mov al, [esi + DateTime.day]
  b2bcd
  mov ecx, eax
  shl ecx, 16
  mov al, [esi + DateTime.month]
  b2bcd
  mov ch, al 
  mov ax, [esi + DateTime.year]
  sub ax, 2000
  b2bcd 
  mov cl, al
  
  ; Display BCD date 
  ;cinvoke con_printf, str_d, ecx
  
  ; Set date
  mov eax, 22
  mov ebx, 1
;  mov ecx, [edx + DateTime.date]
  int 0x40
  
  ; ? error
  
.nosync:
  
  ; Check LI field
;  mov al, byte [sntp_packet] 
;  bt al,0
;  jnc @f
;  mov edx,1
;@@:
;  bt ax, 1
;  jnc @f
;  mov edx,2
;  jmp .error 
  
@@:
  clear edx, edi ; no error
  
.error:
  
  ; Close socket
  mcall close, ebp     

  ; Ignore error from close,
  ; but it write result to eax & ebx
  ; so we need wtire result here 
  mov eax,edx
  mov ebx,edi 
  
   
  ret
  
;fail:
;  cinvoke con_printf, str_err, eax, ebx
;  ret

; data
str_title	db 'SNTP client',0
str_help  db 'sntp host [-tz [-[+]]hh[:ss]] [-s]|[-st]|[-ss]',10
;str_help  db 'sntp host [-c][[-p] [-tz [-[+]]hh[:ss]] [-s]|[-st]|[-ss]]',10
;'-ñ	Load config from Time.ini',10
;'-p	Set port, default is 123',10
          db 'host  Name or IP address of NTP/SNTP server',10
          db 'Options:',10 
          db '-tz 	Set time zone, default is GMT',10
             ;'-qm	Try query time from master SNTP server (if stratum > 1), defautl is disabled',10
          db 10
          db 'Syncronization, default is disabled',10  
          db '-s 	System date and time',10
          db '-st	System time (hours, mitutes and seconds) only',10
          db '-ss	Save current hour (syncronize minutes and seconds only)',10	
          db 10
;-dr	Display SNTP server reserence ID (if stratum = 0)
;-dt	Display accurate time from SNTP request, default is enabled
;-da	Display all SNTP server information
          db 'Examples:',10
          db 'sntp pool.ntp.org -tz 1 -s',10
          db 'sntp 88.147.254.227 -tz 1 -ss',10,0
;str_badoption   db 'Warning: unknown option %s, ignored',10, 0  
;str_query db 'Query - %s [%s] :%i',10,0
str_query db 'Query: %s',0
str_ip    db ' [%s]',0
str_port  db ' :%i',10,0
;str_tt   db 'Transmit time - 0x%X.%X',10,0
;str_t	  db 'Timestamp - %i',10,0
str_dt	  db 'Date & time: %i.%02i.%02i %i:%02i:%02i GMT',0 ; ' UTC %%i:%02i'
str_tz	  db ' +%i:%02i',10,0  ;Time zone: GMT +%i:%02i
;str_d	  db 'BCD date - 0x%08X',10,0
;str_t	  db 'BCD time - 0x%08X',10,0 
str_err  db 'Error: #%i, %s => %s',0
str_err10  db  'Bad command line, type ',39,'sntp',39,' for help.',10,0
str_err11 db  'Incorrect time zone! Visit https://www.timeanddate.com/time/map for details.',10,0
;str_err2  db 'Can',39,'t run @NOTIFY!',10,0
str_err3  db 'Host name not resolved!',0      ; @notify  => ntp.example.com \nError 30: Host name not resolved! 
str_err4  db 'Connection failed!',0           ; @notify  => ntp.example.com:100 \nError 4x: Connection failed!
str_err5  db 'Host not responce!',0           ; @notify  => ntp.example.com:100 \nError 50: Host not responce!
str_err6  db 'Received Kiss-o',39,'-Death (KoD) packet, repeat later or try another server!',0

;str_err7 db 'Server clock not syncronizing!',0 Not needed, see RFC 4330 about LI field 
str_err7 db 'Clock syncronizing failed!',10,0
;str_err62 db 'Date syncronizing failed',0
str_warn db 'Warning: Last minute will have %i seconds!',10,0              
str_s	  db 'Date & time',0
str_st	  db 'Time',0
str_ss	  db 'Mitutes & seconds',0
str_sync  db '%s syncronized',10,0
datetime  DateTime ?
;SystemTime	dd ?
;TimeZone	 dw 180 ; GMT + 3
;Flags		 db 0

port	  dw 123
tz_h	  db 0
tz_m	  db 0
sync	  db 0
;notify  db 0

sockaddr1:
	dw AF_INET4
.port	dw 0x7b00     ; 123 in network order (big endian)
.ip	dd 0
	rb 10

SIZEOF_SNTP_PACKET  = 48 	
sntp_packet	        db 0x23 ; Li = 0 Vn = 4 Mode = 3 (client)       
  .Stratum	        db 0
	.Pool		          db 0
	.Precision	      db 0 
	.RootDelay	      dd 0
	.RootDispersion   dd 0
  .ReferenceID	    dd 0
	.ReferenceTime	  dq 0
	.OriginateTime	  dq 0
	.ReceiveTime	    dq 0
  .TransmitTime     dd 0

; import
align 4
@IMPORT:

library network, 'network.obj', console, 'console.obj'
import network, \
  inet_addr, 'inet_addr',  \
  getaddrinfo,	'getaddrinfo',	\
	freeaddrinfo,	'freeaddrinfo', \
	inet_ntoa,	'inet_ntoa'

;	con_start,	'START',	\
import	console,	\
	con_init,	'con_init',	\
	con_write_asciiz,	'con_write_asciiz',	\
	con_printf,	  'con_printf',     \
	con_exit,	'con_exit',	\
	con_gets,	'con_gets',\
	con_cls,	'con_cls',\
	con_getch2,	'con_getch2',\
	con_set_cursor_pos, 'con_set_cursor_pos',\
	con_get_flags,	'con_get_flags'
  ;, \
  ;con_set_flags, 'con_set_flags'

;socketnum	dd ?

I_END:
		rb 4096
    align 16
;buffer_ptr:	 rb BUFFERSIZE	
STACKTOP:

MEM:
params	rb 1024
;buffer_ptr:	 rb BUFFERSIZE
     
	
IM_END: