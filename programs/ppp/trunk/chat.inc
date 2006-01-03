;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;;  CHAT.INC                                                       ;;
;;                                                                 ;;
;;  Modem Chat Initiator for PPP Dialer                            ;;
;;                                                                 ;;
;;  Version 3    2nd May 2003                                      ;;
;;                                                                 ;;
;;  Copyright 2002 Shrirang Bhagwat, b_shrirang@hotmail.com        ;;
;;                                                                 ;;
;;  See file COPYING for details                                   ;;
;;                                                                 ;;
;;  2/5/03 - Shrirang - Added Abort Strings For sendwait        ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Request And Response Chat Strings
; Following Data Structure is used for Request Response Chatting
; with the modem, for each request there is an expected response
; chatreq <-> chatres, 0 (NULL) is delimeter for 1 string
; '`' is delimiter for the section, modify according to your
; modem dialing scheme; in future MenuetOS might provide a graphical
; client to generate this kind of data structure and the PAP packet
; for username and password!
; Aborts strings are used to get outta sendwait early...
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; REQUESTS
chatreq db 'ATH',13,0  ; 0 (NULL) is required by sendwait
        db 'ATZ',13,0
        db 'ATM1',13,0
        db 'ATX1',13,0  ; My Modem doesn't connect without this
 ;       db 'ATDT;',13,0
        db 'ATDT phonenumber',13,0
        db '`'   ; <- End Marker

; RESPONSES
chatres db 'OK',0
        db 'OK',0
        db 'OK',0
        db 'OK',0
 ;       db 'OK',0
        db 'CONNECT',0
        db '`'   ; <- End Marker

; ABORTS
aborts  db 'ERROR',0
        db 'NO CARRIER',0
        db 'NO DIALTONE',0
        db 'BUSY',0
        db 'LINE IN USE',0
        db 'NO ANSWER',0
        db '`'   ; <- End Marker
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  modem_chat   -  chats with the modem to initiate a connection
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
modem_chat:

 push edi
 push esi
 push edx

 mov edi, chatreq ; init everytime, safe, stateless
 mov esi, chatres

chat_now:

 cmp byte [esi], '`' ; are we done?
 je chatover

 mov edx, 6000 ; strings like "atdt;" take long on my modem
 call sendwait

 and eax, eax
 jz timeoutoccured

updatereq:

 inc edi
 cmp byte [edi], '`'
 je updateres

 cmp byte [edi], 0
 jne updatereq

 inc edi

updateres:

 inc esi
 cmp byte [esi], '`'
 je chatover

 cmp byte [esi], 0
 jne updateres

 inc esi

 jmp chat_now


chatover:

 xor eax, eax ; SUCCESS!
 inc eax

 pop edx
 pop esi
 pop edi

 ret

timeoutoccured:

 xor eax, eax ; FAIL!

 pop edx
 pop esi
 pop edi

 ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  scanaborts   -  scans the response from modem for abort srings
;     ESI - Response from modem
;     EDI - Pointer to Abort Table
;     ECX - Response Length
;     Returns 1 if abort detected, 0 otherwise
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
scanaborts:

 push esi
 push edi
 push ecx

checkit:

 push esi
 repe cmpsb   ; any abort matches?
 je abortdetected
    pop  esi


updatetbl:

 inc edi
 cmp byte [edi], '`'
 je noabortfound

 cmp byte [edi], 0
 jne updatetbl

 inc edi

 jmp checkit

abortdetected:

 pop  esi
 pop  ecx
 pop  edi
 pop  esi

 xor eax, eax  ; SUCCESS!
    inc eax
 ret

noabortfound :

 pop  ecx
 pop  edi
 pop  esi


 xor eax, eax  ; FAILED!
 ret






