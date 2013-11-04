;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  HTTP library for KolibriOS                                     ;;
;;                                                                 ;;
;;   Written by hidnplayr@kolibrios.org                            ;;
;;                                                                 ;;
;;         GNU GENERAL PUBLIC LICENSE                              ;;
;;          Version 2, June 1991                                   ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; references:
; "HTTP made really easy", http://www.jmarshall.com/easy/http/
; "Hypertext Transfer Protocol -- HTTP/1.1", http://tools.ietf.org/html/rfc2616


        URLMAXLEN       = 65535
        BUFFERSIZE      = 4096

        __DEBUG__       = 1
        __DEBUG_LEVEL__ = 1


format MS COFF

public @EXPORT as 'EXPORTS'

include '../../../struct.inc'
include '../../../proc32.inc'
include '../../../macros.inc'
purge section,mov,add,sub
include '../../../debug-fdo.inc'

include '../../../network.inc'
include 'http.inc'

virtual at 0
        http_msg http_msg
end virtual

macro copy_till_zero {
  @@:
        lodsb
        test    al, al
        jz      @f
        stosb
        jmp     @r
  @@:
}

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
        mov     [mem.alloc], eax
        mov     [mem.free], ebx
        mov     [mem.realloc], ecx
        mov     [dll.load], edx

        invoke  dll.load, @IMPORT
        or      eax, eax
        jz      .ok

; load proxy settings
        invoke  ini.get_str, inifile, sec_proxy, key_proxy, proxyAddr, 256, proxyAddr
        invoke  ini.get_int, inifile, sec_proxy, key_proxyport, 80
        mov     [proxyPort], eax
        invoke  ini.get_str, inifile, sec_proxy, key_user, proxyUser, 256, proxyUser
        invoke  ini.get_str, inifile, sec_proxy, key_password, proxyPassword, 256, proxyPassword

        xor     eax, eax
        inc     eax
        ret

  .ok:
        xor     eax, eax
        ret





;;================================================================================================;;
proc HTTP_get URL ;///////////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;?                                                                                                ;;
;;------------------------------------------------------------------------------------------------;;
;> _                                                                                              ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) / buffer ptr                                                                   ;;
;;================================================================================================;;
locals
        hostname        dd ?
        pageaddr        dd ?
        sockaddr        dd ?
        socketnum       dd ?
        buffer          dd ?
endl

; split the URL into hostname and pageaddr
        stdcall parse_url, [URL]
        test    eax, eax
        jz      .error
        mov     [hostname], eax
        mov     [pageaddr], ebx

; Do we need to use a proxy?
        cmp     [proxyAddr], 0
        jne     .proxy_done

        ; TODO
  .proxy_done:

; Resolve the hostname
        DEBUGF  1, "Resolving hostname\n"
        push    esp     ; reserve stack place
        push    esp     ; fourth parameter
        push    0       ; third parameter
        push    0       ; second parameter
        push    [hostname]
        call    [getaddrinfo]
        pop     esi
        test    eax, eax
        jnz     .error

; getaddrinfo returns addrinfo struct, make the pointer to sockaddr struct
        mov     esi, [esi + addrinfo.ai_addr]
        mov     [sockaddr], esi
        mov     eax, [esi + sockaddr_in.sin_addr]
        test    eax, eax
        jz      .error

        DEBUGF  1, "Server ip=%u.%u.%u.%u\n", \
        [esi + sockaddr_in.sin_addr]:1, [esi + sockaddr_in.sin_addr + 1]:1, \
        [esi + sockaddr_in.sin_addr + 2]:1, [esi + sockaddr_in.sin_addr + 3]:1

        mov     [esi + sockaddr_in.sin_family], AF_INET4
        mov     [esi + sockaddr_in.sin_port], 80 shl 8  ;;; FIXME

; Connect to the server.
        mcall   socket, AF_INET4, SOCK_STREAM, 0
        test    eax, eax
        jz      .error
        mov     [socketnum], eax
        DEBUGF  1, "Socket: 0x%x\n", eax

        mcall   connect, [socketnum], [sockaddr], 18
        test    eax, eax
        jnz     .error
        DEBUGF  1, "Socket is now connected.\n"

        ; TODO: free address buffer(s)

; Create the HTTP request.
        invoke  mem.alloc, BUFFERSIZE
        test    eax, eax
        jz      .error
        mov     [buffer], eax
        DEBUGF  1, "Buffer has been allocated.\n"

        mov     dword[eax], 'GET '
        lea     edi, [eax + 4]
        mov     esi, [pageaddr] ; TODO: for proxy use http:// and then full URL
        copy_till_zero

        mov     esi, str_http11
        mov     ecx, str_http11.length
        rep     movsb

        mov     esi, [hostname]
        copy_till_zero

        mov     esi, str_close
        mov     ecx, str_close.length
        rep     movsb

        mov     byte[edi], 0
        DEBUGF  1, "Request:\n%s", [buffer]

; now send the request
        mov     esi, edi
        sub     esi, [buffer]   ; length
        xor     edi, edi        ; flags

        mcall   send, [socketnum], [buffer]
        test    eax, eax
        jz      .error
        DEBUGF  1, "Request has been sent to server.\n"

; Now that we have sent the request, re-purpose buffer as receive buffer
        mov     eax, [buffer]
        push    [socketnum]
        popd    [eax + http_msg.socket]
        lea     esi, [eax + http_msg.data]
        mov     [eax + http_msg.flags], 0
        mov     [eax + http_msg.write_ptr], esi
        mov     [eax + http_msg.buffer_length], BUFFERSIZE -  http_msg.data
        mov     [eax + http_msg.chunk_ptr], 0

        mov     [eax + http_msg.status], 0
        mov     [eax + http_msg.header_length], 0
        mov     [eax + http_msg.content_length], 0

        ret                     ; return buffer ptr

  .error:
        DEBUGF  1, "Error!\n"
        xor     eax, eax        ; return 0 = error
        ret

endp



;;================================================================================================;;
proc HTTP_process identifier ;////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;?                                                                                                ;;
;;------------------------------------------------------------------------------------------------;;
;> _                                                                                              ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (not finished) / 0 finished                                                           ;;
;;================================================================================================;;
        pusha
        mov     ebp, [identifier]

; Receive some data
        mcall   recv, [ebp + http_msg.socket], [ebp + http_msg.write_ptr], \
                      [ebp + http_msg.buffer_length], MSG_DONTWAIT
        cmp     eax, 0xffffffff
        je      .check_socket
        DEBUGF  1, "Received %u bytes\n", eax

; Update pointers
        mov     edi, [ebp + http_msg.write_ptr]
        add     [ebp + http_msg.write_ptr], eax
        sub     [ebp + http_msg.buffer_length], eax
        jz      .got_all_data

; If data is chunked, combine chunks into contiguous data.
        test    [ebp + http_msg.flags], FLAG_CHUNKED
        jnz     .chunk_loop

; Did we detect the header yet?
        test    [ebp + http_msg.flags], FLAG_GOT_HEADER
        jnz     .header_parsed

; We havent found the header yet, search for it..
        sub     eax, 4
        jl      .need_more_data
  .scan:
        ; scan for end of header (empty line)
        cmp     dword[edi], 0x0a0d0a0d                  ; end of header
        je      .end_of_header
        cmp     word[edi+2], 0x0a0a
        je      .end_of_header
        inc     edi
        dec     eax
        jnz     .scan

  .end_of_header:
        add     edi, 4 - http_msg.data
        sub     edi, ebp
        mov     [ebp + http_msg.header_length], edi
        or      [ebp + http_msg.flags], FLAG_GOT_HEADER
        DEBUGF  1, "Header length: %u\n", edi

; Ok, we have found header:
        cmp     dword[ebp + http_msg.data], 'HTTP'
        jne     .invalid_header
        cmp     dword[ebp + http_msg.data+4], '/1.0'
        je      .http_1.0
        cmp     dword[ebp + http_msg.data+4], '/1.1'
        jne     .invalid_header
        or      [ebp + http_msg.flags], FLAG_HTTP11
  .http_1.0:
        cmp     byte[ebp + http_msg.data+8], ' '
        jne     .invalid_header
        DEBUGF  1, "Header seems valid.\n"

        lea     esi, [ebp + http_msg.data+9]
        xor     eax, eax
        xor     ebx, ebx
        mov     ecx, 3
  .statusloop:
        lodsb
        sub     al, '0'
        jb      .invalid_header
        cmp     al, 9
        ja      .invalid_header
        lea     ebx, [ebx + 4*ebx]
        shl     ebx, 1
        add     ebx, eax
        dec     ecx
        jnz     .statusloop
        mov     [ebp + http_msg.status], ebx
        DEBUGF  1, "Status: %u\n", ebx

; Now, convert all header names to lowercase.
; This way, it will be much easier to find certain header fields, later on.

        lea     esi, [ebp + http_msg.data]
        mov     ecx, [ebp + http_msg.header_length]
  .need_newline:
        inc     esi
        dec     ecx
        jz      .convert_done
        cmp     byte[esi], 10
        jne     .need_newline
; Ok, we have a newline, a line beginning with space or tabs has no header fields.

        inc     esi
        dec     ecx
        jz      .convert_done
        cmp     byte[esi], ' '
        je      .need_newline
        cmp     byte[esi], 9    ; horizontal tab
        je      .need_newline
        jmp     .convert_loop
  .next_char:
        inc     esi
        dec     ecx
        jz      .convert_done
  .convert_loop:
        cmp     byte[esi], ':'
        je      .need_newline
        cmp     byte[esi], 'A'
        jb      .next_char
        cmp     byte[esi], 'Z'
        ja      .next_char
        or      byte[esi], 0x20 ; convert to lowercase
        jmp     .next_char
  .convert_done:
        mov     byte[esi-1], 0
        lea     esi, [ebp + http_msg.data]
        DEBUGF  1, "Header names converted to lowercase:\n%s\n", esi

; Check for content-length header field.
        stdcall find_header_field, ebp, str_cl
        test    eax, eax
        jz      .no_content
        or      [ebp + http_msg.flags], FLAG_CONTENT_LENGTH

        xor     edx, edx
  .cl_loop:
        movzx   ebx, byte[eax]
        inc     eax
        cmp     bl, 10
        je      .cl_ok
        cmp     bl, 13
        je      .cl_ok
        cmp     bl, ' '
        je      .cl_ok
        sub     bl, '0'
        jb      .invalid_header
        cmp     bl, 9
        ja      .invalid_header
        lea     edx, [edx + edx*4]      ; edx = edx*10
        shl     edx, 1                  ;
        add     edx, ebx
        jmp     .cl_loop

  .cl_ok:
        mov     [ebp + http_msg.content_length], edx
        DEBUGF  1, "Content-length: %u\n", edx

; Resize buffer according to content-length.
        mov     eax, [ebp + http_msg.header_length]
        add     eax, [ebp + http_msg.content_length]
        add     eax, http_msg.data

        mov     ecx, eax
        sub     ecx, [ebp + http_msg.write_ptr]
        mov     [ebp + http_msg.buffer_length], ecx

        invoke  mem.realloc, ebp, eax
        or      eax, eax
        jz      .no_ram
        jmp     .header_parsed  ; hooray!

  .no_content:
        DEBUGF  1, "Content-length not found.\n"

; We didnt find 'content-length', maybe server is using chunked transfer encoding?
; Try to find 'transfer-encoding' header.
        stdcall find_header_field, ebp, str_te
        test    eax, eax
        jz      .invalid_header

        mov     ebx, dword[eax]
        or      ebx, 0x20202020
        cmp     ebx, 'chun'
        jne     .invalid_header
        mov     ebx, dword[eax+4]
        or      ebx, 0x00202020
        and     ebx, 0x00ffffff
        cmp     ebx, 'ked'
        jne     .invalid_header

        or      [ebp + http_msg.flags], FLAG_CHUNKED
        DEBUGF  1, "Transfer type is: chunked\n"

; Set chunk pointer where first chunk should begin.
        lea     eax, [ebp + http_msg.data]
        add     eax, [ebp + http_msg.header_length]
        mov     [ebp + http_msg.chunk_ptr], eax

  .chunk_loop:
        mov     ecx, [ebp + http_msg.write_ptr]
        sub     ecx, [ebp + http_msg.chunk_ptr]
        jb      .need_more_data_chunked

; TODO: make sure we have the complete chunkline header
        mov     esi, [ebp + http_msg.chunk_ptr]
        xor     ebx, ebx
  .chunk_hexloop:
        lodsb
        sub     al, '0'
        jb      .chunk_
        cmp     al, 9
        jbe     .chunk_hex
        sub     al, 'A' - '0' - 10
        jb      .chunk_
        cmp     al, 15
        jbe     .chunk_hex
        sub     al, 'a' - 'A'
        cmp     al, 15
        ja      .chunk_
  .chunk_hex:
        shl     ebx, 4
        add     bl, al
        jmp     .chunk_hexloop
  .chunk_:
        DEBUGF  1, "got chunk of %u bytes\n", ebx
; If chunk size is 0, all chunks have been received.
        test    ebx, ebx
        jz      .got_all_data_chunked           ; last chunk, hooray! FIXME: what if it wasnt a valid hex number???
        mov     edi, [ebp + http_msg.chunk_ptr] ; we'll need this in about 25 lines...
        add     [ebp + http_msg.chunk_ptr], ebx

; Chunkline ends with a CR, LF or simply LF
  .end_of_chunkline?:           ; FIXME: buffer overflow possible!
        cmp     al, 10
        je      .end_of_chunkline
        lodsb
        jmp     .end_of_chunkline?

  .end_of_chunkline:
; Realloc buffer, make it 'chunksize' bigger.
        mov     eax, [ebp + http_msg.buffer_length]
        add     eax, ebx
        invoke  mem.realloc, ebp, eax
        or      eax, eax
        jz      .no_ram
        add     [ebp + http_msg.buffer_length], ebx

; Update write ptr
        mov     eax, esi
        sub     eax, edi
        sub     [ebp + http_msg.write_ptr], eax

; Now move all received data to the left (remove chunk header).
; Update content_length accordingly.
        mov     ecx, [ebp + http_msg.write_ptr]
        sub     ecx, esi
        add     [ebp + http_msg.content_length], ecx
        rep     movsb
        jmp     .chunk_loop

; Check if we got all the data.
  .header_parsed:
        mov     eax, [ebp + http_msg.header_length]
        add     eax, [ebp + http_msg.content_length]
        cmp     eax, [ebp + http_msg.buffer_length]
        je      .got_all_data
  .need_more_data:
        popa
        xor     eax, eax
        dec     eax
        ret

  .need_more_data_chunked:
        add     [ebp + http_msg.content_length], eax
        popa
        xor     eax, eax
        dec     eax
        ret

  .got_all_data_chunked:
        mov     eax, [ebp + http_msg.chunk_ptr]
        sub     eax, [ebp + http_msg.header_length]
        sub     eax, http_msg.data
        sub     eax, ebp
        mov     [ebp + http_msg.content_length], eax

  .got_all_data:
        DEBUGF  1, "We got all the data! (%u bytes)\n", [ebp + http_msg.content_length]
        or      [ebp + http_msg.flags], FLAG_GOT_DATA
        mcall   close, [ebp + http_msg.socket]
        popa
        xor     eax, eax
        ret

  .check_socket:
        cmp     ebx, EWOULDBLOCK
        je      .need_more_data
        DEBUGF  1, "ERROR: socket error %u\n", ebx

        or      [ebp + http_msg.flags], FLAG_SOCKET_ERROR
        popa
        xor     eax, eax
        ret

  .invalid_header:
        DEBUGF  1, "ERROR: invalid header\n"
        or      [ebp + http_msg.flags], FLAG_INVALID_HEADER
        popa
        xor     eax, eax
        ret

  .no_ram:
        DEBUGF  1, "ERROR: out of RAM\n"
        or      [ebp + http_msg.flags], FLAG_NO_RAM
        popa
        xor     eax, eax
        ret

endp



;;================================================================================================;;
proc find_header_field identifier, headername ;///////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;?                                                                                                ;;
;;------------------------------------------------------------------------------------------------;;
;> _                                                                                              ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (error) / 0                                                                           ;;
;;================================================================================================;;
        push    ebx ecx edx esi edi

        DEBUGF  1, "Find header field: %s\n", [headername]

        mov     ebx, [identifier]
        lea     edx, [ebx + http_msg.data]
        mov     ecx, edx
        add     ecx, [ebx + http_msg.header_length]

  .restart:
        mov     esi, [headername]
        mov     edi, edx
  .loop:
        cmp     edi, ecx
        jae     .fail
        lodsb
        scasb
        je      .loop
        test    al, al
        jz      .done?
  .next:
        inc     edx
        jmp     .restart

  .not_done:
        inc     edi
  .done?:
        cmp     byte[edi-1], ':'
        je      .almost_done
        cmp     byte[edi-1], ' '
        je      .not_done
        cmp     byte[edi-1], 9  ; tab
        je      .not_done

        jmp     .next

  .almost_done:                 ; FIXME: buffer overflow?
        dec     edi
        DEBUGF  1, "Found header field\n"
  .spaceloop:
        inc     edi
        cmp     byte[edi], ' '
        je      .spaceloop
        cmp     byte[edi], 9    ; tab
        je      .spaceloop

        mov     eax, edi
        pop     edi esi edx ecx ebx
        ret

  .fail:
        pop     edi esi edx ecx ebx
        xor     eax, eax
        ret

endp


; internal procedures start here:


;;================================================================================================;;
proc parse_url URL ;//////////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;?                                                                                                ;;
;;------------------------------------------------------------------------------------------------;;
;> _                                                                                              ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (error) / 0                                                                           ;;
;;================================================================================================;;

locals
        urlsize         dd ?
        hostname        dd ?
        pageaddr        dd ?
endl

        DEBUGF  1, "parsing URL: %s\n", [URL]

; remove any leading protocol text
        mov     esi, [URL]
        mov     ecx, URLMAXLEN
        mov     ax, '//'
  .loop1:
        cmp     byte[esi], 0            ; end of URL?
        je      .url_ok                 ; yep, so not found
        cmp     [esi], ax
        je      .skip_proto
        inc     esi
        dec     ecx
        jnz     .loop1

        DEBUGF  1, "Invalid URL\n"
        xor     eax, eax
        ret

  .skip_proto:
        inc     esi                     ; skip the two '/'
        inc     esi
        mov     [URL], esi              ; update pointer so it skips protocol
        jmp     .loop1                  ; we still need to find the length of the URL

  .url_ok:
        sub     esi, [URL]              ; calculate total length of URL
        mov     [urlsize], esi


; now look for page delimiter - it's a '/' character
        mov     ecx, esi                ; URL length
        mov     edi, [URL]
        mov     al, '/'
        repne   scasb
        jne     @f
        dec     edi                     ; return one char, '/' must be part of the pageaddr
        inc     ecx                     ;
  @@:
        push    ecx edi                 ; remember the pointer and length of pageaddr

        mov     ecx, edi
        sub     ecx, [URL]
        inc     ecx                     ; we will add a 0 byte at the end
        invoke  mem.alloc, ecx
        or      eax, eax
        jz      .no_mem

        mov     [hostname], eax         ; copy hostname to buffer
        mov     edi, eax
        mov     esi, [URL]
        dec     ecx
        rep     movsb
        xor     al, al
        stosb

        mov     [pageaddr], str_slash   ; assume there is no pageaddr
        pop     esi ecx
        test    ecx, ecx
        jz      .no_page
        inc     ecx                     ; we will add a 0 byte at the end
        invoke  mem.alloc, ecx
        or      eax, eax
        jz      .no_mem

        mov     [pageaddr], eax         ; copy pageaddr to buffer
        mov     edi, eax
        dec     ecx
        rep     movsb
        xor     al, al
        stosb
  .no_page:

        mov     eax, [hostname]
        mov     ebx, [pageaddr]

        DEBUGF  1, "hostname: %s\n", eax
        DEBUGF  1, "pageaddr: %s\n", ebx

        ret

  .no_mem:
        xor     eax, eax
        ret

endp





;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Imported functions section                                                                     ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;


align 16
@IMPORT:

library \
        libini, 'libini.obj', \
        network, 'network.obj'

import  libini, \
        ini.get_str, 'ini_get_str', \
        ini.get_int, 'ini_get_int'

import  network,\
        getaddrinfo, 'getaddrinfo',\
        freeaddrinfo,  'freeaddrinfo',\
        inet_ntoa, 'inet_ntoa'

;;===========================================================================;;
;;///////////////////////////////////////////////////////////////////////////;;
;;===========================================================================;;
;! Exported functions section                                                ;;
;;===========================================================================;;
;;///////////////////////////////////////////////////////////////////////////;;
;;===========================================================================;;


align 4
@EXPORT:
export  \
        lib_init                , 'lib_init'            , \
        0x00010001              , 'version'             , \
        HTTP_get                , 'get'                 , \
        find_header_field       , 'find_header_field'   , \
        HTTP_process            , 'process'

;        HTTP_head               , 'head'                , \
;        HTTP_post               , 'post'                , \
;        HTTP_put                , 'put'                 , \
;        HTTP_delete             , 'delete'              , \
;        HTTP_trace              , 'trace'               , \
;        HTTP_connect            , 'connect'             , \



section '.data' data readable writable align 16

inifile         db '/sys/settings/network.ini', 0

sec_proxy:
key_proxy       db 'proxy', 0
key_proxyport   db 'port', 0
key_user        db 'user', 0
key_password    db 'password', 0

str_http11      db ' HTTP/1.1', 13, 10, 'Host: '
  .length       = $ - str_http11
str_close       db 13, 10, 'User-Agent: KolibriOS libHTTP/1.0', 13, 10, 'Connection: Close', 13, 10, 13, 10
  .length       = $ - str_close
str_proxy_auth  db 13, 10, 'Proxy-Authorization: Basic '
  .length       = $ - str_proxy_auth

base64_table    db 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'
                db '0123456789+/'

str_cl          db 'content-length', 0
str_slash       db '/', 0
str_te          db 'transfer-encoding', 0

include_debug_strings

; uninitialized data
mem.alloc       dd ?
mem.free        dd ?
mem.realloc     dd ?
dll.load        dd ?

proxyAddr       rb 256
proxyUser       rb 256
proxyPassword   rb 256
proxyPort       dd ?