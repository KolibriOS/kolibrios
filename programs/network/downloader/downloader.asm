;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2009-2013. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  downloader.asm - HTTP client for KolibriOS                     ;;
;;                                                                 ;;
;;  Based on HTTPC.asm for menuetos by ville turjanmaa             ;;
;;                                                                 ;;
;;  Programmers: Barsuk, Clevermouse, Marat Zakiyanov,             ;;
;;      Kirill Lipatov, dunkaist, HidnPlayr                        ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

URLMAXLEN               = 1024
primary_buffer_size     = 4096

__DEBUG__       = 1
__DEBUG_LEVEL__ = 1

format binary as ""

use32
        org     0x0

        db      'MENUET01'      ; header
        dd      0x01            ; header version
        dd      START           ; entry point
        dd      IM_END          ; image size
        dd      I_END           ; required memory
        dd      I_END           ; esp
        dd      params          ; I_PARAM
        dd      0x0             ; I_Path

include '../macros.inc'
include '../proc32.inc'
include '../network.inc'
include '../../develop/libraries/box_lib/trunk/box_lib.mac'
include '../dll.inc'
include '../debug-fdo.inc'

START:

        mcall   68, 11                  ; init heap so we can allocate memory dynamically

        mcall   40, EV_STACK

; load libraries
        stdcall dll.Load, @IMPORT
        test    eax, eax
        jnz     exit

; prepare webAddr area  

        mov     al, ' '
        mov     edi, webAddr
        mov     ecx, URLMAXLEN
        rep     stosb
        xor     eax, eax
        stosb

; prepare document area 
        mov     al, '/'
        mov     edi, document
        stosb
        mov     al, ' '
        mov     ecx, URLMAXLEN-1
        rep     stosb

        call    load_settings
        cmp     byte[params], 0
        je      prepare_event   ;red

; we have an url
        mov     edi, document_user
        mov     al, ' '
        mov     ecx, URLMAXLEN
        rep     stosb
        
        mov     esi, params
        mov     edi, document_user

; copy untill space or 0
  .copy_param:
        mov     al, [esi]
        test    al, al
        jz      .done

        cmp     al, ' '
        jz      .done_inc

        mov     [edi], al
        inc     esi
        inc     edi
        jmp     .copy_param

  .done_inc:

; url is followed by shared memory name.
        inc     esi
  .done:
        mov     [shared_name], esi

        mov     ah, 22   ; strange way to tell that socket should be opened...
        call    socket_commands

        jmp     still

prepare_event:
; Report events
; Stack 8 + defaults
        mcall   40, 10100111b

red:    ; redraw
        call    draw_window

still:
        mcall   23, 1   ; wait here for event
        cmp     eax, 1  ; redraw request ?
        je      red

        cmp     eax, 2  ; key in buffer ?
        je      key

        cmp     eax, 3  ; button in buffer ?
        je      button
        
        cmp     eax, 6  ; mouse in buffer ?
        je      mouse

; Get the web page data from the remote server
        cmp     eax, 8
        jne     still

        call    read_incoming_data

        cmp     [status], 4
        je      .no_send

        mov     [onoff], 1
        call    send_request

  .no_send:
        call    print_status

        cmp     [prev_status], 4
        jne     no_close
        cmp     [status], 4             ; connection closed by server
        jbe     no_close                ; respond to connection close command
; draw page
        call    read_incoming_data
        mcall   close, [socketnum]
        mov     [onoff], 0

no_close:
        jmp     still

key:
        mcall   2       ; read key

        stdcall [edit_box_key], dword edit1

        shr     eax, 8
        cmp     eax, 13
        je      retkey
        
        jmp     still

        
button:

        mcall   17      ; get id
        cmp     ah, 26
        je      save
        cmp     ah, 1   ; button id=1 ?
        jne     noclose
;        DEBUGF  1, "Closing socket before exit...\n"

close_end_exit:

exit:
        or      eax, -1  ; close this program
        mcall
        
mouse:
        stdcall [edit_box_mouse], edit1
        jmp     still

save:
        DEBUGF  1, "file saved\n"
        mcall   70, fileinfo

        mov     ecx, [sc.work_text]
        or      ecx, 0x80000000
        mcall   4, <10, 93>, , download_complete

        jmp     still

noclose:
        cmp     ah, 31
        jne     noup
        sub     [display_from], 20
        jmp     still

noup:
        cmp     ah, 32
        jne     nourl
        add     [display_from], 20
        jmp     still


retkey:
        mov     ah, 22  ; start load

nourl:
        call    socket_commands ; opens or closes the connection
        jmp     still


;****************************************************************************
;    Function
;       send_request
;
;   Description
;       Transmits the GET request to the server.
;       This is done as GET then URL then HTTP/1.1', 13, 10, 13, 10 in 3 packets
;
;****************************************************************************
send_request:

        DEBUGF  1, "Sending request\n"

        pusha
        mov     esi, string0
        mov     edi, request
        movsd
; If proxy is used, make absolute URI - prepend http://<host>
        cmp     byte[proxyAddr], 0
        jz      .noproxy
        mov     dword[edi], 'http'
        mov     byte[edi+4], ':'
        mov     word[edi+5], '//'
        add     edi, 7
        mov     esi, webAddr

  .copy_host_loop:
        lodsb
        cmp     al, ' '
        jz      .noproxy
        stosb
        jmp     .copy_host_loop

  .noproxy:
        xor     edx, edx ; 0

  .next_edx:
; Determine the length of the url to send in the GET request
        mov     al, [edx+document]
        cmp     al, ' '
        jbe     .document_done
        mov     [edi], al
        inc     edi
        inc     edx
        jmp     .next_edx

  .document_done:
        mov     esi, stringh
        mov     ecx, stringh_end-stringh
        rep     movsb
        xor     edx, edx ; 0

  .webaddr_next:
        mov     al, [webAddr + edx]
        cmp     al, ' '
        jbe     .webaddr_done
        mov     [edi], al
        inc     edi
        inc     edx
        jmp     .webaddr_next

  .webaddr_done:
        cmp     byte[proxyUser], 0
        jz      @f
        call    append_proxy_auth_header
    @@:
        mov     esi, connclose
        mov     ecx, connclose_end-connclose
        rep     movsb

        pusha   
        mov     eax, 63
        mov     ebx, 1
        mov     edx, request
    @@:
        mov     cl, [edx]
        cmp     edx, edi
        jz      @f
        mcall
        inc     edx
        jmp     @b
    @@:
        popa

        mov     esi, edi
        sub     esi, request    ; length
        xor     edi, edi        ; flags
;;;;now write \r\nConnection: Close \r\n\r\n
        mcall   send, [socketnum], request  ;' HTTP/1.1 .. '
        popa
        ret

;****************************************************************************
;    Function
;       print_status
;
;   Description
;       displays the socket/data received status information
;
;****************************************************************************
print_status:
        pusha
        mcall   26, 9
        cmp     eax, [nextupdate]
        jb      status_return

        add     eax, 25
        mov     [nextupdate], eax

        mov     ecx, [winys]
        shl     ecx, 16
        add     ecx, -18 shl 16 + 10
        mcall   13, <5, 100>, , 0xffffff

        mov     edx, 12 shl 16 - 18
        add     edx, [winys]
        xor     esi, esi
        mcall   47, <3, 0>, [status], ,

        mov     edx, 40 shl 16 - 18
        add     edx, [winys]
        mcall   , <6, 0>, [pos]

status_return:
        popa
        ret

;****************************************************************************
;    Function
;       read_incoming_data
;
;   Description
;       receive the web page from the server, storing it without processing
;
;****************************************************************************
read_incoming_data:
        cmp     [onoff], 1
        je      .rid
        ret

  .rid:
        push    esi
        push    edi
        DEBUGF  1, "rid\n"

  .read:
        mcall   recv, [socketnum], primary_buf, primary_buffer_size, 0
        inc     eax             ; -1 = error (socket closed?)
        jz      .no_more_data
        dec     eax             ; 0 bytes...
        jz      .read
        
        mov     edi, [pos]
        add     [pos], eax
        push    eax
        mcall   68, 20, [pos], [buf_ptr]
        mov     [buf_ptr], eax
        add     edi, eax
        mov     esi, primary_buf
        pop     ecx             ; number of recently read bytes
        lea     edx, [ecx - 3]
        rep     movsb
        
  .no_more_data:

;        mov     [status], 4     ; connection closed by server

        call    parse_result
        mov     ecx, [shared_name]
        test    ecx, ecx
        jz      @f
        cmp     [ecx], byte 0
        jnz     save_in_shared
    @@:

        mcall   70, fileinfo
        
        mov     ecx, [sc.work_text]
        or      ecx, 0x80000000
        mcall   4, <10, 93>, , download_complete
        DEBUGF  1, "file saved\n"

        pop     edi
        pop     esi

; if called from command line, then exit
        cmp     byte[params], 0
        jne     exit

        ret
        
save_in_shared:

        mov     esi, 1   ; SHM_OPEN+SHM_WRITE
        mcall   68, 22
        test    eax, eax
        jz      save_in_shared_done

        sub     edx, 4
        jbe     save_in_shared_done

        mov     ecx, [final_size]
        cmp     ecx, edx
        jb      @f

        mov     ecx, edx
    @@:
        mov     [eax], ecx
        lea     edi, [eax+4]
        mov     esi, [final_buffer]
        mov     edx, ecx
        shr     ecx, 2
        rep     movsd
        mov     ecx, edx
        and     ecx, 3
        rep     movsb

save_in_shared_done:
        pop     edi
        pop     esi
        jmp     exit
        
; this function cuts header, and removes chunk sizes if doc is chunked
; in: buf_ptr, pos; out: buf_ptr, pos.
        
parse_result:
; close socket
        mcall   close, [socketnum]
        
        DEBUGF  1, "close socketnum: 0x%x\n", eax
        mov     edi, [buf_ptr]
        mov     edx, [pos]
        mov     [buf_size], edx
;       mcall   70, fileinfo_tmp
        DEBUGF  1, "pos = 0x%x\n", edx

; first, find end of headers
  .next_byte:
        cmp     dword[edi], 0x0d0a0d0a  ; мне лень читать стандарт, пусть будут оба варианта
        je      .end_of_headers
        cmp     dword[edi], 0x0a0d0a0d
        je      .end_of_headers
        inc     edi
        dec     edx
        jne     .next_byte
; no end of headers. it's an error. let client see all those headers.
        ret

  .end_of_headers:
; here we look at headers and search content-length or transfer-encoding headers
;       DEBUGF  1, "eoh\n"

        sub     edi, [buf_ptr]
        add     edi, 4
        mov     [body_pos], edi  ; store position where document body starts
        mov     [is_chunked], 0
; find content-length in headers
; not good method, but should work for 'Content-Length:'
        mov     esi, [buf_ptr]
        mov     edi, s_contentlength
        mov     ebx, [body_pos]
        xor     edx, edx ; 0
  .cl_next:
        mov     al, [esi]
        cmp     al, [edi + edx]
        jne     .cl_fail
        inc     edx
        cmp     edx, len_contentlength
        je      .cl_found
        jmp     .cl_incr
  .cl_fail:
        xor     edx, edx ; 0
  .cl_incr:
        inc     esi
        dec     ebx
        je      .cl_error
        jmp     .cl_next
  .cl_error:
;       DEBUGF  1, "content-length not found\n"

; find 'chunked'
; да, я копирую код, это ужасно, но мне хочется, чтобы поскорее заработало
; а там уж отрефакторю
        mov     esi, [buf_ptr]
        mov     edi, s_chunked
        mov     ebx, [body_pos]
        xor     edx, edx ; 0

  .ch_next:
        mov     al, [esi]
        cmp     al, [edi + edx]
        jne     .ch_fail
        inc     edx
        cmp     edx, len_chunked
        je      .ch_found
        jmp     .ch_incr

  .ch_fail:
        xor     edx, edx ; 0

  .ch_incr:
        inc     esi
        dec     ebx
        je      .ch_error
        jmp     .ch_next

  .ch_error:
; if neither of the 2 headers is found, it's an error
;       DEBUGF  1, "transfer-encoding: chunked not found\n"
        mov     eax, [pos]
        sub     eax, [body_pos]
        jmp     .write_final_size

  .ch_found:
        mov     [is_chunked], 1
        mov     eax, [body_pos]
        add     eax, [buf_ptr]
        sub     eax, 2
        mov     [prev_chunk_end], eax
        jmp     parse_chunks
        
  .cl_found:
        call    read_number     ; eax = number from *esi

  .write_final_size:
        mov     [final_size], eax        ; if this works, i will b very happy...
        
        mov     ebx, [pos]       ; we well check if it is right
        sub     ebx, [body_pos]

; everything is ok, so we return
        mov     eax, [body_pos]
        mov     ebx, [buf_ptr]
        add     ebx, eax
        mov     [final_buffer], ebx
;       mov     ebx, [pos]
;       sub     ebx, eax
;       mov     [final_size], ebx
        ret
        
parse_chunks:
;       DEBUGF  1, "parse chunks\n"
        ; we have to look through the data and remove sizes of chunks we see
        ; 1. read size of next chunk
        ; 2. if 0, it's end. if not, continue.
        ; 3. make a good buffer and copy a chunk there
        xor     eax, eax
        mov     [final_buffer], eax      ; 0
        mov     [final_size], eax        ; 0
        
.read_size:
        mov     eax, [prev_chunk_end]
        mov     ebx, eax
        sub     ebx, [buf_ptr]
        mov     edx, eax
;       DEBUGF  1, "rs "
        cmp     ebx, [pos]
        jae     chunks_end      ; not good
        
        call    read_hex        ; in: eax=pointer to text. out:eax=hex number, ebx=end of text.
        cmp     eax, 0
        jz      chunks_end

        add     ebx, 1
        mov     edx, ebx ; edx = size of size of chunk
        
        add     ebx, eax
        mov     [prev_chunk_end], ebx
        
;       DEBUGF  1, "sz "

; do copying: from buf_ptr+edx to final_buffer+prev_final_size count eax
; realloc final buffer
        push    eax
        push    edx
        push    dword [final_size]
        add     [final_size], eax
        mcall   68, 20, [final_size], [final_buffer]
        mov     [final_buffer], eax
;       DEBUGF  1, "re "
        pop     edi
        pop     esi
        pop     ecx
;       add     [pos], ecx
        add     edi, [final_buffer]
;       DEBUGF  1, "cp "

        rep     movsb
        jmp     .read_size
        
chunks_end:
; free old buffer
        DEBUGF  1, "chunks end\n"

        mcall   68, 13, [buf_ptr]
; done!
        ret

; reads content-length from [edi+ecx], result in eax
read_number:
        push    ebx
        xor     eax, eax
        xor     ebx, ebx

  .next:
        mov     bl, [esi]

        cmp     bl, '0'
        jb      .not_number
        cmp     bl, '9'
        ja      .not_number
        sub     bl, '0'
        shl     eax, 1
        lea     eax, [eax + eax * 4]     ; eax *= 10
        add     eax, ebx

  .not_number:
        cmp     bl, 13
        je      .done
        inc     esi
        jmp     .next

  .done:
        pop     ebx
        ret
        
; reads hex from eax, result in eax, end of text in ebx
read_hex:
        add     eax, 2
        mov     ebx, eax
        mov     eax, [ebx]
        mov     [deba], eax

        xor     eax, eax
        xor     ecx, ecx
.next:
        mov     cl, [ebx]
        inc     ebx
        
        cmp     cl, 0x0d
        jz      .done

        or      cl, 0x20
        sub     cl, '0'
        jb      .bad

        cmp     cl, 0x9
        jbe     .adding

        sub     cl, 'a'-'0'-10
        cmp     cl, 0x0a
        jb      .bad

        cmp     cl, 0x0f
        ja      .bad

.adding:
        shl     eax, 4
        or      eax, ecx
;       jmp     .not_number
;.bad:
.bad:
        jmp     .next
.done:

        ret

;****************************************************************************
;    Function
;       socket_commands
;
;   Description
;       opens or closes the socket
;
;****************************************************************************
socket_commands:
        cmp     ah, 22   ; open socket
        jne     tst3

        DEBUGF  1, "opening socket\n"

; Clear all page memory
        xor     eax, eax
        mov     [prev_chunk_end], eax    ; 0
        cmp     [buf_ptr], eax   ; 0
        jz      no_free

        mcall   68, 13, [buf_ptr] ; free  buffer

no_free:
        xor     eax, eax
        mov     [buf_size], eax  ; 0
; Parse the entered url
        call    parse_url

        mov     edx, 80
        cmp     byte [proxyAddr], 0
        jz      @f
        mov     eax, [proxyPort]
        xchg    al, ah
        mov     [server_port], ax
    @@:

        mcall   socket, AF_INET4, SOCK_STREAM, 0
        mov     [socketnum], eax
        mcall   connect, [socketnum], sockaddr1, 18
        mov     [pagexs], 80
        
        push    eax
        xor     eax, eax ; 0
        mov     [pos], eax
        mov     [pagex], eax
        mov     [pagey], eax
        mov     [command_on_off], eax
        mov     [is_body], eax
        pop     eax
        ret

tst3:
        cmp     ah, 24   ; close socket
        jne     no_24

        mcall   close, [socketnum]
no_24:
        ret

;****************************************************************************
;    Function
;       parse_url
;
;   Description
;       parses the full url typed in by the user into a web address ( that
;       can be turned into an IP address by DNS ) and the page to display
;       DNS will be used to translate the web address into an IP address, if
;       needed.
;       url is at document_user and will be space terminated.
;       web address goes to webAddr and is space terminated.
;       ip address goes to server_ip
;       page goes to document and is space terminated.
;
;       Supported formats:
;       <protocol://>address<page>
;       <protocol> is optional, removed and ignored - only http supported
;       <address> is required. It can be an ip address or web address
;       <page> is optional and must start with a leading / character
;
;****************************************************************************
parse_url:
; First, reset destination variables
        mov     al, ' '
        mov     edi, document
        mov     ecx, URLMAXLEN
        rep     stosb
        mov     edi, webAddr
        mov     ecx, URLMAXLEN
        rep     stosb

        mov     al, '/'
        mov     [document], al

        mov     esi, document_user
; remove any leading protocol text
        mov     ecx, URLMAXLEN
        mov     ax, '//'

pu_000:
        cmp     [esi], byte ' '         ; end of text?
        je      pu_002                  ; yep, so not found
        cmp     [esi], ax
        je      pu_001                  ; Found it, so esi+2 is start
        inc     esi
        loop    pu_000

pu_002:
; not found, so reset esi to start
        mov     esi, document_user-2

pu_001:
        add     esi, 2
        mov     ebx, esi ; save address of start of web address
        mov     edi, document_user + URLMAXLEN   ; end of string
; look for page delimiter - it's a '/' character
pu_003:
        cmp     [esi], byte ' '  ; end of text?
        je      pu_004          ; yep, so none found
        cmp     esi, edi         ; end of string?
        je      pu_004          ; yep, so none found
        cmp     [esi], byte '/'  ; delimiter?
        je      pu_005          ; yep - process it
        inc     esi
        jmp     pu_003

pu_005:
; copy page to document address
; esi = delimiter
        push    esi
        mov     ecx, edi         ; end of document_user
        mov     edi, document

pu_006:
        movsb
        cmp     esi, ecx
        je      pu_007          ; end of string?
        cmp     [esi], byte ' '  ; end of text
;       je      pu_007          ; дзен-ассемблер
;       jmp     pu_006          ; не надо плодить сущности по напрасну
        jne     pu_006

pu_007:
        pop     esi     ; point esi to '/' delimiter

pu_004:
; copy web address to webAddr
; start in ebx, end in esi-1
        mov     ecx, esi
        mov     esi, ebx
        mov     edi, webAddr
  @@:
        movsb
        cmp     esi, ecx
        jne     @r
        mov     byte [edi], 0

pu_009:
; For debugging, display resulting strings
        DEBUGF  1, "document_user: %s\n", document_user
        DEBUGF  1, "webAddr: %s\n", webAddr
        DEBUGF  1, "document: %s\n", document

; Look up the ip address, or was it specified?
        mov     al, [proxyAddr]
        cmp     al, 0
        jnz     pu_015
        mov     al, [webAddr]
pu_015:
        cmp     al, '0'
        jb      pu_010  ; Resolve address
        cmp     al, '9'
        ja      pu_010  ; Resolve address

        DEBUGF  1, "GotIP\n"

; Convert address
; If proxy is given, get proxy address instead of server
        mov     esi, proxyAddr-1
        cmp     byte[esi+1], 0
        jne     pu_020
        mov     esi, webAddr-1

pu_020:
        mov     edi, server_ip
        xor     eax, eax

ip1:
        inc     esi
        cmp     [esi], byte '0'
        jb      ip2
        cmp     [esi], byte '9'
        ja      ip2
        imul    eax, 10
        movzx   ebx, byte [esi]
        sub     ebx, 48
        add     eax, ebx
        jmp     ip1

ip2:
        mov     [edi], al
        xor     eax, eax
        inc     edi
        cmp     edi, server_ip+3
        jbe     ip1
        jmp     pu_011

pu_010:
        DEBUGF  1, "Resolving %s\n", webAddr

; resolve name
        push    esp     ; reserve stack place
        push    esp     ; fourth parameter
        push    0       ; third parameter
        push    0       ; second parameter
        push    webAddr
        call    [getaddrinfo]
        pop     esi
; test for error
        DEBUGF  1, "eax=0x%x\n", eax
;        test    eax, eax
;        jnz     .fail_dns

; fill in ip in sockstruct
        mov     eax, [esi + addrinfo.ai_addr]
        mov     eax, [eax + sockaddr_in.sin_addr]
        mov     [server_ip], eax

        DEBUGF  1, "Resolved to %u.%u.%u.%u\n", [server_ip]:1, [server_ip + 1]:1, [server_ip + 2]:1, [server_ip + 3]:1

pu_011:

        ret

;***************************************************************************
;   Function
;       load_settings
;
;   Description
;       Load settings from configuration file network.ini
;
;***************************************************************************
load_settings:

        invoke  ini.get_str, inifile, sec_proxy, key_proxy, proxyAddr, 256, proxyAddr
        invoke  ini.get_int, inifile, sec_proxy, key_proxyport, 80
        mov     [proxyPort], eax
        invoke  ini.get_str, inifile, sec_proxy, key_user, proxyUser, 256, proxyUser
        invoke  ini.get_str, inifile, sec_proxy, key_password, proxyPassword, 256, proxyPassword

        ret

;***************************************************************************
;   Function
;       append_proxy_auth_header
;
;   Description
;       Append header to HTTP request for proxy authentification
;
;***************************************************************************
append_proxy_auth_header:
        mov     esi, proxy_auth_basic
        mov     ecx, proxy_auth_basic_end - proxy_auth_basic
        rep     movsb
; base64-encode string <user>:<password>
        mov     esi, proxyUser

apah000:
        lodsb
        test    al, al
        jz      apah001
        call    encode_base64_byte
        jmp     apah000

apah001:
        mov     al, ':'
        call    encode_base64_byte
        mov     esi, proxyPassword

apah002:
        lodsb
        test    al, al
        jz      apah003
        call    encode_base64_byte
        jmp     apah002

apah003:
        call    encode_base64_final
        ret

encode_base64_byte:
        inc     ecx
        shl     edx, 8
        mov     dl, al
        cmp     ecx, 3
        je      ebb001
        ret

ebb001:
        shl     edx, 8
        inc     ecx

ebb002:
        rol     edx, 6
        xor     eax, eax
        xchg    al, dl
        mov     al, [base64_table+eax]
        stosb
        loop    ebb002
        ret

encode_base64_final:
        mov     al, 0
        test    ecx, ecx
        jz      ebf000
        call    encode_base64_byte
        test    ecx, ecx
        jz      ebf001
        call    encode_base64_byte
        mov     byte [edi-2], '='

ebf001:
        mov     byte [edi-1], '='

ebf000:
        ret

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************

draw_window:

        mcall   12, 1

        mcall   48, 3, sc, 40 ;get system colors

        mov     edx, [sc.work]
        or      edx, 0x34000000
        mcall   0, <50, 370>, <350, 140>, , 0, title   ;draw window
        
        mov     ecx, [sc.work_text]
        or      ecx, 80000000h
        mcall   4, <14, 14>, , type_pls ;"URL:"

        edit_boxes_set_sys_color edit1, editboxes_end, sc
        stdcall [edit_box_draw], edit1

; RELOAD
        mcall   8, <90, 68>, <54, 16>, 22, [sc.work_button]
; STOP
        mcall   , <166, 50>, <54, 16>, 24
; SAVE
        mcall   , <224, 54>, , 26
; BUTTON TEXT
        mov     ecx, [sc.work_button_text]
        or      ecx, 80000000h
        mcall   4, <102, 59>, , button_text

        mcall   12, 2 ; end window redraw
        ret
;-----------------------------------------------------------------------------
; Data area
;-----------------------------------------------------------------------------
align   4
@IMPORT:

library libini, 'libini.obj', \
        box_lib, 'box_lib.obj', \
        network, 'network.obj'

import  libini, \
        ini.get_str, 'ini_get_str', \
        ini.get_int, 'ini_get_int'

import  box_lib, \
        edit_box_draw, 'edit_box', \
        edit_box_key, 'edit_box_key', \
        edit_box_mouse, 'edit_box_mouse'

import  network,\
        getaddrinfo,    'getaddrinfo',\
        freeaddrinfo,   'freeaddrinfo',\
        inet_ntoa,      'inet_ntoa'

;---------------------------------------------------------------------
fileinfo        dd 2, 0, 0
final_size      dd 0
final_buffer    dd 0
                db '/rd/1/.download', 0
        
body_pos        dd 0

buf_size        dd 0
buf_ptr         dd 0

deba            dd 0
                db 0

;---------------------------------------------------------------------

mouse_dd        dd 0
edit1           edit_box 295, 48, 10, 0xffffff, 0xff, 0x80ff, 0, 0x8000, URLMAXLEN, document_user, mouse_dd, ed_focus+ed_always_focus, 7, 7
editboxes_end:

;---------------------------------------------------------------------

include_debug_strings

;---------------------------------------------------------------------

type_pls        db 'URL:', 0
button_text     db 'DOWNLOAD     STOP     RESAVE', 0
download_complete db 'File saved as /rd/1/.download', 0
display_from    dd 20
pos             dd 0
pagex           dd 0
pagey           dd 0
pagexs          dd 80
command_on_off  dd 0
text_type       db 1
com2            dd 0
script          dd 0
socketnum       dd 0

addr            dd 0
ya              dd 0
len             dd 0

title           db 'HTTP Downloader', 0

;---------------------------------------------------------------------
s_contentlength db 'Content-Length:'
len_contentlength = 15

s_chunked       db 'Transfer-Encoding: chunked'
len_chunked     = $ - s_chunked

is_body         dd 0    ; 0 if headers, 1 if content
is_chunked      dd 0
prev_chunk_end  dd 0
cur_chunk_size  dd 0

string0:        db 'GET '

stringh                 db ' HTTP/1.1', 13, 10, 'Host: '
stringh_end:
proxy_auth_basic        db 13, 10, 'Proxy-Authorization: Basic '
proxy_auth_basic_end:
connclose               db 13, 10, 'User-Agent: Kolibrios Downloader', 13, 10, 'Connection: Close', 13, 10, 13, 10
connclose_end:

base64_table    db 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'
                db '0123456789+/'

inifile         db '/sys/network.ini', 0

sec_proxy:
key_proxy       db 'proxy', 0
key_proxyport   db 'port', 0
key_user        db 'user', 0
key_password    db 'password', 0

sockaddr1:
                dw AF_INET4
server_port     dw 0x5000       ; 80
server_ip       dd 0
                rb 10

proxyPort       dd 80

shared_name     dd 0

status          dd 0x0
prev_status     dd 0x0

onoff           dd 0x0

nextupdate      dd 0
winys           dd 400

;---------------------------------------------------------------------
document_user   db 'http://', 0
;---------------------------------------------------------------------
IM_END:
;---------------------------------------------------------------------
                rb URLMAXLEN-(IM_END - document_user)
;---------------------------------------------------------------------
                sc system_colors
;---------------------------------------------------------------------
align 4
document        rb URLMAXLEN
;---------------------------------------------------------------------
align 4
webAddr         rb URLMAXLEN+1
;---------------------------------------------------------------------

align 4
primary_buf     rb primary_buffer_size

params          rb 1024

request         rb 256

proxyAddr       rb 256
proxyUser       rb 256
proxyPassword   rb 256

                rb 4096         ; stack

I_END:



