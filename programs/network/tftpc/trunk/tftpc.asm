;
;    TFTP Client
   
use32
 org	0x0
 db	'MENUET01'    ; header
 dd	0x01	      ; header version
 dd	START	      ; entry point
 dd	I_END	      ; image size
 dd	I_END+0x10000 ; required memory
 dd	I_END+0x10000 ; esp
 dd	0x0 , 0x0     ; I_Param , I_Path

include 'lang.inc'
include '..\..\..\macros.inc'
   
START:				; start of execution
     mov  eax,40		 ; Report events
     mov  ebx,10000111b 	 ; Stack 8 + defaults
     mcall
   
    mov   dword [prompt], p1
    mov  dword [promptlen], p1len - p1

red:   
    call draw_window		; at first, draw the window
   
still:
    mov  eax,10 		; wait here for event
    mcall
   
    cmp  eax,1			; redraw request ?
    jz	 red
    cmp  eax,2			; key in buffer ?
    jz	 key
    cmp  eax,3			; button in buffer ?
    jz	 button
   
    jmp  still

key:			       ; Keys are not valid at this part of the
    mov  eax,2			; loop. Just read it and ignore
    mcall
    jmp  still
   
button: 		       ; button
    mov  eax,17 		; get id
    mcall
   
    cmp  ah,1			; button id=1 ?
    jnz  noclose
   
   
    ; close socket before exiting
 mov  eax, 53
 mov  ebx, 1
 mov  ecx, [socketNum]
    int   0x40
   
 mov  [socketNum], dword 0
   
   
    or  eax,-1 	; close this program
    mcall
   
noclose:
    cmp  ah,2			; copy file to local machine?
    jnz  nocopyl
   
    mov   dword [prompt], p5
    mov  dword [promptlen], p5len - p5
    call  draw_window		 ;
   
    ; Copy File from Remote Host to this machine
    call translateData	; Convert Filename & IP address
    mov  edi, tftp_filename + 1
    mov  [edi], byte 0x01 ; setup tftp msg
    call copyFromRemote
   
    jmp  still
   
nocopyl:
   
    cmp  ah,3	     ; Copy file to host?
    jnz  nocopyh
   
    mov   dword [prompt], p5
    mov  dword [promptlen], p5len - p5
    call  draw_window		 ;
   
    ; Copy File from this machine to Remote Host
    call translateData	; Convert Filename & IP address
    mov  edi, tftp_filename + 1
    mov  [edi], byte 0x02 ; setup tftp msg
    call copyToRemote
   
    jmp  still
   
nocopyh:
    cmp  ah,4
    jz	 f1
    cmp  ah,5
    jz	 f2
    jmp  nof12
   
  f1:
    mov  [addr],dword source
    mov  [ya],dword 35
    jmp  rk
   
  f2:
    mov  [addr],dword destination
    mov  [ya],dword 35+16
   
  rk:
    mov  ecx,15
    mov  edi,[addr]
    mov  al,' '
    rep  stosb
   
    call print_text
   
    mov  edi,[addr]
   
  f11:
    mov  eax,10
    mcall
    cmp  eax,2
    jz	 fbu
    jmp  still
  fbu:
    mov  eax,2
    mcall  ; get key
    shr  eax,8
    cmp  eax,8
    jnz  nobs
    cmp  edi,[addr]
    jz	 f11
    sub  edi,1
    mov  [edi],byte ' '
    call print_text
    jmp  f11
  nobs:
    cmp  eax,dword 31
    jbe  f11
    cmp  eax,dword 95
    jb	 keyok
    sub  eax,32
  keyok:
    mov  [edi],al
   
    call print_text
   
    add  edi,1
    mov  esi,[addr]
    add  esi,15
    cmp  esi,edi
    jnz  f11
   
    jmp  still
   
print_text:
   
    mov  eax,13
    mov  ebx,103*65536+15*6
    mov  ecx,[ya]
    shl  ecx,16
    mov  cx,8
    mov  edx,0x224466
    mcall
   
    mov  eax,4
    mov  ebx,103*65536
    add  ebx,[ya]
    mov  ecx,0xffffff
    mov  edx,[addr]
    mov  esi,15
    mcall
   
    ret
   
   
  nof12:
    jmp  still
   
   
;***************************************************************************
;   Function
;      translateData
;
;   Description
;      Coverts the filename and IP address typed in by the user into
;      a format suitable for the IP layer.
;
;    The filename, in source, is converted and stored in tftp_filename
;      The host ip, in destination, is converted and stored in tftp_IP
;
;***************************************************************************
translateData:
   
 ; first, build up the tftp command string. This includes the filename
 ; and the transfer protocol
   
   
 ; First, write 0,0
 mov  al, 0
 mov  edi, tftp_filename
 mov  [edi], al
 inc  edi
 mov  [edi], al
 inc  edi
   
 ; Now, write the file name itself, and null terminate it
 mov  ecx, 15
 mov  ah, ' '
 mov  esi, source
   
td001:
 lodsb
 stosb
 cmp  al, ah
 loopnz td001
   
 cmp  al,ah  ; Was the entire buffer full of characters?
 jne  td002
 dec  edi   ; No - so remove ' ' character
   
td002:
 mov  [edi], byte 0
 inc  edi
 mov  [edi], byte 'O'
 inc  edi
 mov  [edi], byte 'C'
 inc  edi
 mov  [edi], byte 'T'
 inc  edi
 mov  [edi], byte 'E'
 inc  edi
 mov  [edi], byte 'T'
 inc  edi
 mov  [edi], byte 0
   
 mov  esi, tftp_filename
 sub  edi, esi
 mov  [tftp_len], edi
   
   
 ; Now, convert the typed IP address into a real address
 ; No validation is done on the number entered
 ; ip addresses must be typed in normally, eg
 ; 192.1.45.24
   
 xor  eax, eax
 mov  dh, 10
 mov  dl, al
 mov  [tftp_IP], eax
   
 ; 192.168.24.1   1.1.1.1       1. 9.2.3.
   
 mov  esi, destination
 mov  edi, tftp_IP
   
 mov  ecx, 4
   
td003:
 lodsb
 sub  al, '0'
 add  dl, al
 lodsb
 cmp  al, '.'
 je  ipNext
 cmp  al, ' '
 je  ipNext
 mov  dh, al
 sub  dh, '0'
 mov  al, 10
 mul  dl
 add  al, dh
 mov  dl, al
 lodsb
 cmp  al, '.'
 je  ipNext
 cmp  al, ' '
 je  ipNext
 mov  dh, al
 sub  dh, '0'
 mov  al, 10
 mul  dl
 add  al, dh
 mov  dl, al
 lodsb
   
ipNext:
 mov  [edi], dl
 inc  edi
 mov  dl, 0
 loop td003
   
 ret
   
   
   
;***************************************************************************
;   Function
;      copyFromRemote
;
;   Description
;
;***************************************************************************
copyFromRemote:
 xor  eax, eax
 mov  [filesize], eax
 mov  eax, I_END + 512 ; This is the point where the file buffer is
 mov  [fileposition], eax
   
 ; Get a random # for the local socket port #
 mov  eax, 3
 int	 0x40
 mov  ecx, eax
 shr  ecx, 8	; Set up the local port # with a random #
   
   ; open socket
 mov  eax, 53
 mov  ebx, 0
 mov  edx, 69	 ; remote port
 mov  esi, [tftp_IP]  ; remote IP ( in intenet format )
    int   0x40
   
   mov	[socketNum], eax
   
 ; make sure there is no data in the socket - there shouldn't be..
   
cfr001:
 mov  eax, 53
 mov  ebx, 3
 mov  ecx, [socketNum]
    int   0x40	  ; read byte
   
 mov  eax, 53
 mov  ebx, 2
 mov  ecx, [socketNum]
    int   0x40	  ; any more data?
   
 cmp  eax, 0
 jne  cfr001	; yes, so get it
   
 ; Now, request the file
 mov  eax, 53
 mov  ebx, 4
 mov  ecx, [socketNum]
 mov  edx, [tftp_len]
 mov  esi, tftp_filename
    int   0x40
   
cfr002:
    mov  eax,10 		; wait here for event
    mcall
   
    cmp  eax,1			; redraw request ?
    je	 cfr003
    cmp  eax,2			; key in buffer ?
    je	 cfr004
    cmp  eax,3			; button in buffer ?
    je	 cfr005
   
    ; Any data to fetch?
 mov  eax, 53
 mov  ebx, 2
 mov  ecx, [socketNum]
    int   0x40
   
 cmp  eax, 0
 je  cfr002
   
 push eax     ; eax holds # chars
   
 ; Update the text on the display - once
 mov  eax, [prompt]
 cmp  eax, p3
 je  cfr008
    mov   dword [prompt], p3
    mov  dword [promptlen], p3len - p3
    call  draw_window		 ;
   
cfr008:
 ; we have data - this will be a tftp frame
   
 ; read first two bytes - opcode
 mov  eax, 53
 mov  ebx, 3
 mov  ecx, [socketNum]
    int   0x40	 ; read byte
   
 mov  eax, 53
 mov  ebx, 3
 mov  ecx, [socketNum]
    int   0x40	 ; read byte
   
 pop  eax
 ; bl holds tftp opcode. Can only be 3 (data) or 5 ( error )
   
 cmp  bl, 3
 jne  cfrerr
   
 push eax
   
 ; do data stuff. Read block #. Read data. Send Ack.
 mov  eax, 53
 mov  ebx, 3
 mov  ecx, [socketNum]
    int   0x40	 ; read byte
   
 mov  [blockNumber], bl
   
 mov  eax, 53
 mov  ebx, 3
 mov  ecx, [socketNum]
 int   0x40   ; read byte
   
 mov  [blockNumber+1], bl
   
cfr007:
   
 mov  eax, 53
 mov  ebx, 2
 mov  ecx, [socketNum]
 mcall   ; any more data?
   
 cmp  eax, 0
 je   no_more_data ; no
   
 mov  eax, 53
 mov  ebx, 3
 mov  ecx, [socketNum]
 mcall   ; read byte
   
 mov  esi, [fileposition]
 mov  [esi], bl
 inc  dword [fileposition]
 inc  dword [filesize]
   
 jmp  cfr007					       
   
no_more_data:
   
 ; write the block number into the ack
 mov  al, [blockNumber]
 mov  [ack + 2], al
   
 mov  al, [blockNumber+1]
 mov  [ack + 3], al
   
 ; send an 'ack'
 mov  eax, 53
 mov  ebx, 4
 mov  ecx, [socketNum]
 mov  edx, ackLen - ack
 mov  esi, ack
    int   0x40
   
 ; If # of chars in the frame is less that 516,
 ; this frame is the last
 pop  eax
 cmp  eax, 516
 je  cfr002
   
 ; Write the file
 mov  eax, 33
 mov  ebx, source
 mov  edx, [filesize]
 mov  ecx, I_END + 512
 mov  esi, 0
 mcall
   
 jmp  cfrexit
   
cfrerr:
 ; simple implementation on error - just read all data, and return
 mov  eax, 53
 mov  ebx, 3
 mov  ecx, [socketNum]
    int   0x40	  ; read byte
   
 mov  eax, 53
 mov  ebx, 2
 mov  ecx, [socketNum]
    int   0x40	  ; any more data?
   
 cmp  eax, 0
 jne  cfrerr	; yes, so get it
   
 jmp  cfr006	; close socket and close app
   
cfr003: 			; redraw request
    call draw_window
    jmp  cfr002
   
cfr004: 			; key pressed
    mov  eax,2			; just read it and ignore
    mcall
    jmp  cfr002
   
cfr005: 		       ; button
    mov  eax,17 		; get id
    mcall
   
    cmp  ah,1			; button id=1 ?
    jne  cfr002     ; If not, ignore.
   
cfr006:
    ; close socket
 mov  eax, 53
 mov  ebx, 1
 mov  ecx, [socketNum]
    int   0x40
   
 mov  [socketNum], dword 0
   
    mov  eax,-1 		; close this program
    mcall
   
    jmp $
   
cfrexit:
    ; close socket
 mov  eax, 53
 mov  ebx, 1
 mov  ecx, [socketNum]
    int   0x40
   
 mov  [socketNum], dword 0
   
    mov   dword [prompt], p4
    mov  dword [promptlen], p4len - p4
    call  draw_window		 ;
   
 ret
   
   
   
;***************************************************************************
;   Function
;      copyToRemote
;
;   Description
;
;***************************************************************************
copyToRemote:
    mov   eax,6    ; Read file from floppy (image)
    mov   ebx,source
    mov   ecx,0
    mov   edx,0xffffffff
    mov   esi,I_END + 512
    int   0x40
   
    cmp   eax,0xffffffff
    jnz   filefound
   
    mov   dword [prompt], p6
    mov  dword [promptlen], p6len - p6
    call  draw_window		 ;
 jmp  ctr_exit
   
filefound:
 mov  [filesize], eax
   
 ; First, set up the file pointers
 mov  eax, 0x01000300
 mov  [blockBuffer], eax ; This makes sure our TFTP header is valid
   
 mov  eax, I_END + 512 ; This is the point where the file buffer is
 mov  [fileposition], eax
   
 mov  eax, [filesize]
 cmp  eax, 512
 jb  ctr000
 mov  eax, 512
ctr000:
 mov  [fileblocksize], ax
   
 ; Get a random # for the local socket port #
 mov  eax, 3
 int	 0x40
 mov  ecx, eax
 shr  ecx, 8	; Set up the local port # with a random #
   
   ; First, open socket
 mov  eax, 53
 mov  ebx, 0
 mov  edx, 69	 ; remote port
 mov  esi, [tftp_IP]
    int   0x40
   
   mov	[socketNum], eax
   
   ; write to socket ( request write file )
 mov  eax, 53
 mov  ebx, 4
 mov  ecx, [socketNum]
 mov  edx, [tftp_len]
 mov  esi, tftp_filename
    int   0x40
   
 ; now, we wait for
 ; UI redraw
 ; UI close
 ; or data from remote
   
ctr001:
    mov   eax,10		 ; wait here for event
    int   0x40
   
    cmp   eax,1 		 ; redraw request ?
    je	  ctr003
    cmp   eax,2 		 ; key in buffer ?
    je	  ctr004
    cmp   eax,3 		 ; button in buffer ?
    je	  ctr005
   
   
    ; Any data in the UDP receive buffer?
 mov  eax, 53
 mov  ebx, 2
 mov  ecx, [socketNum]
    int   0x40
   
 cmp  eax, 0
 je  ctr001
   
 ; Update the text on the display - once
 mov  eax, [prompt]
 cmp  eax, p2
 je  ctr002
   
    mov   dword [prompt], p2
    mov  dword [promptlen], p2len - p2
    call  draw_window		 ;
   
 ; we have data - this will be the ack
ctr002:
 mov  eax, 53
 mov  ebx, 3
 mov  ecx, [socketNum]
    int   0x40	 ; read byte - opcode
   
 mov  eax, 53
 mov  ebx, 3
 mov  ecx, [socketNum]
    int   0x40	 ; read byte - opcode
   
 mov  eax, 53
 mov  ebx, 3
 mov  ecx, [socketNum]
    int   0x40	 ; read byte - block (high byte)
   
 mov  [blockNumber], bl
   
 mov  eax, 53
 mov  ebx, 3
 mov  ecx, [socketNum]
    int   0x40	 ; read byte - block (low byte )
   
 mov  [blockNumber+1], bl
   
ctr0022:
 mov  eax, 53
 mov  ebx, 3
 mov  ecx, [socketNum]
    int   0x40	 ; read byte (shouldn't have worked)
   
   
 mov  eax, 53
 mov  ebx, 2
 mov  ecx, [socketNum]
    int   0x40	 ; any more data?
   
 cmp  eax, 0
 jne  ctr0022  ; yes, so get it, and dump it
   
 ; If the ack is 0, it is to the request
   
 mov  bx, [blockNumber]
 cmp  bx, 0
 je   txd
   
 ; now, the ack should be one more than the current field - otherwise, resend
   
 cmp  bx, [blockBuffer+2]
 jne  txre     ; not the same, so send again
   
 ; update the block number
 mov  esi, blockBuffer + 3
 mov  al, [esi]
 inc  al
 mov  [esi], al
 cmp  al, 0
 jne  ctr008
 dec  esi
 inc	 byte [esi]
   
ctr008:
 ; Move forward through the file
 mov  eax, [fileposition]
 movzx ebx, word [fileblocksize]
 add  eax, ebx
 mov  [fileposition], eax
   
 ; new ..
 ; fs = 0 , fbs = 512 -> send with fbs = 0
   
 cmp  [filesize],0
 jne  no_special_end
 cmp  [fileblocksize],512
 jne  no_special_end
 mov  ax,0
 jmp  ctr006
no_special_end:
   
 mov  eax, [filesize]
 cmp  eax, 0
 je  ctr009
 cmp  eax, 512
 jb  ctr006
 mov  eax, 512
ctr006:
 mov  [fileblocksize], ax
   
   
txd:
 ; Readjust the file size variable ( before sending )
 mov  eax, [filesize]
 movzx ebx, word [fileblocksize]
 sub  eax, ebx
 mov  [filesize], eax
   
txre:
 ; Copy the fragment of the file to the block buffer
 movzx ecx, word [fileblocksize]
 mov  esi, [fileposition]
 mov  edi, I_END
 cld
 rep  movsb
   
 ; send the file data
 mov  eax, 53
 mov  ebx, 4
 mov  ecx, [socketNum]
 movzx edx, word [fileblocksize]
 add  edx, 4
 mov  esi, blockBuffer
    int   0x40
   
 jmp  ctr001
   
ctr003: 		; redraw
    call  draw_window
    jmp   ctr001
   
ctr004: 		; key
    mov   eax,2 	 ; just read it and ignore
    int   0x40
    jmp   ctr001
   
ctr005: 		; button
    mov   eax,17	 ; get id
    int   0x40
   
    cmp   ah,1		 ; button id=1 ?
    jne   ctr001
   
    ; close socket
 mov  eax, 53
 mov  ebx, 1
 mov  ecx, [socketNum]
    int   0x40
   
 mov  [socketNum], dword 0
   
    mov   eax,-1	 ; close this program
    int   0x40
 jmp  $
   
ctr009:
    ; close socket
 mov  eax, 53
 mov  ebx, 1
 mov  ecx, [socketNum]
    int   0x40
   
    mov   dword [prompt], p4
    mov  dword [promptlen], p4len - p4
    call  draw_window		 ;
   
ctr_exit:
 ret
   
   
   
   
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
   
   
draw_window:
   
    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,1			   ; 1, start of draw
    mcall
   
				   ; DRAW WINDOW
    mov  eax,0			   ; function 0 : define and draw window
    mov  ebx,100*65536+230	   ; [x start] *65536 + [x size]
    mov  ecx,100*65536+170	   ; [y start] *65536 + [y size]
    mov  edx,0x13224466 	   ; color of work area RRGGBB
    mov  edi,title
    mcall
   
    mov  eax,8		    ; COPY BUTTON
    mov  ebx,20*65536+190
    mov  ecx,79*65536+15
    mov  edx,2
    mov  esi,0x557799
    mcall
   
 ;   mov  eax,8		    ; DELETE BUTTON
    mov  ebx,20*65536+190
    mov  ecx,111*65536+15
    mov  edx,3
    mcall
   
 ;   mov  eax,8
 ;   mov  ebx,200*65536+10
    mov  ecx,34*65536+10
    mov  edx,4
    mcall
   
 ;   mov  eax,8
 ;   mov  ebx,200*65536+10
    mov  ecx,50*65536+10
    mov  edx,5
    mcall
   
   
 ; Copy the file name to the screen buffer
 ; file name is same length as IP address, to
 ; make the math easier later.
    cld
    mov  esi,source
    mov  edi,text+13
    mov  ecx,15
    rep  movsb
   
   
 ; copy the IP address to the screen buffer
    mov  esi,destination
    mov  edi,text+40+13
    mov  ecx,15
    rep  movsb
   
  ; copy the prompt to the screen buffer
    mov  esi,[prompt]
    mov  edi,text+280
    mov  ecx,[promptlen]
    rep  movsb
   
    ; Re-draw the screen text
    cld
    mov  eax,4
    mov  ebx,25*65536+35	   ; draw info text with function 4
    mov  ecx,0xffffff
    mov  edx,text
    mov  esi,40
  newline:
    mcall
    add  ebx,16
    add  edx,40
    cmp  [edx],byte 'x'
    jnz  newline
   
   
    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,2			   ; 2, end of draw
    mcall
   
    ret
   
   
; DATA AREA
   
source	     db  'KERNEL.ASM     '
destination  db  '192.168.1.23   '
   
   
tftp_filename:	times 15 + 9 db 0
tftp_IP:   dd 0
tftp_len:   dd 0
   
addr  dd  0x0
ya    dd  0x0
   
fileposition dd 0 ; Points to the current point in the file
filesize  dd 0 ; The number of bytes written / left to write
fileblocksize dw 0 ; The number of bytes to send in this frame
   
text:
    db 'SOURCE FILE: xxxxxxxxxxxxxxx            '
    db 'HOST IP ADD: xxx.xxx.xxx.xxx            '
    db '                                        '
    db '  COPY HOST   ->   LOCAL                '
    db '                                        '
    db '  COPY LOCAL  ->   HOST                 '
    db '                                        '
    db '                                        '
    db 'x' ; <- END MARKER, DONT DELETE
   
   
title	db   'TFTP Client',0   
   
prompt: dd 0
promptlen: dd 0
   
   
p1:  db 'Waiting for Command'
p1len:
   
p2:  db 'Sending File       '
p2len:
   
p3:  db 'Receiving File     '
p3len:
   
p4:  db 'Tranfer Complete   '
p4len:
   
p5:  db 'Contacting Host... '
p5len:
   
p6:  db 'File not found.    '
p6len:
   
ack:
 db 00,04,0,1
ackLen:
   
socketNum:
 dd 0
   
blockNumber:
 dw 0
   
; This must be the last part of the file, because the blockBuffer
; continues at I_END.
blockBuffer:
 db 00, 03, 00, 01
I_END:
   
   
   
   
   
   
   