;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2014. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  HTTP library for KolibriOS                                     ;;
;;                                                                 ;;
;;   Written by hidnplayr@kolibrios.org                            ;;
;;   Proxy code written by CleverMouse                             ;;
;;                                                                 ;;
;;         GNU GENERAL PUBLIC LICENSE                              ;;
;;          Version 2, June 1991                                   ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; references:
; "HTTP made really easy", http://www.jmarshall.com/easy/http/
; "Hypertext Transfer Protocol -- HTTP/1.1", http://tools.ietf.org/html/rfc2616


        URLMAXLEN       = 65535
        BUFFERSIZE      = 8192
        TIMEOUT         = 500  ; in 1/100 s

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
local   .copyloop, .copydone
  .copyloop:
        lodsb
        test    al, al
        jz      .copydone
        stosb
        jmp     .copyloop
  .copydone:
}

macro HTTP_init_buffer buffer, socketnum {

        mov     eax, buffer
        push    socketnum
        popd    [eax + http_msg.socket]
        lea     esi, [eax + http_msg.http_header]
        mov     [eax + http_msg.flags], FLAG_CONNECTED
        mov     [eax + http_msg.write_ptr], esi
        mov     [eax + http_msg.buffer_length], BUFFERSIZE -  http_msg.http_header
        mov     [eax + http_msg.chunk_ptr], 0

        mov     [eax + http_msg.status], 0
        mov     [eax + http_msg.header_length], 0
        mov     [eax + http_msg.content_ptr], 0
        mov     [eax + http_msg.content_length], 0
        mov     [eax + http_msg.content_received], 0

        push    eax ebp
        mov     ebp, eax
        mcall   26, 9
        mov     [ebp + http_msg.timestamp], eax
        pop     ebp eax
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
;< eax = 1 (fail) / 0 (ok)                                                   ;;
;;===========================================================================;;
        mov     [mem.alloc], eax
        mov     [mem.free], ebx
        mov     [mem.realloc], ecx
        mov     [dll.load], edx

        invoke  dll.load, @IMPORT
        test    eax, eax
        jnz     .error

; load proxy settings
        pusha
        invoke  ini.get_str, inifile, sec_proxy, key_proxy, proxyAddr, 256, proxyAddr
        invoke  ini.get_int, inifile, sec_proxy, key_proxyport, 80
        mov     [proxyPort], eax
        invoke  ini.get_str, inifile, sec_proxy, key_user, proxyUser, 256, proxyUser
        invoke  ini.get_str, inifile, sec_proxy, key_password, proxyPassword, 256, proxyPassword
        popa

        DEBUGF  1, "HTTP library: init OK\n"
        xor     eax, eax
        ret

  .error:
        DEBUGF  1, "ERROR loading libraries\n"
        xor     eax, eax
        inc     eax
        ret




;;================================================================================================;;
proc HTTP_get URL, add_header ;///////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Initiates a HTTP connection, using 'GET' method.                                               ;;
;;------------------------------------------------------------------------------------------------;;
;> URL          = pointer to ASCIIZ URL                                                           ;;
;> add_header   = pointer to additional header parameters (ASCIIZ), or null for none.             ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) / buffer ptr                                                                   ;;
;;================================================================================================;;
locals
        hostname        dd ?
        pageaddr        dd ?
        sockaddr        dd ?
        socketnum       dd ?
        buffer          dd ?
        port            dd ?
endl

        pusha

; split the URL into hostname and pageaddr
        stdcall parse_url, [URL]
        test    eax, eax
        jz      .error
        mov     [hostname], eax
        mov     [pageaddr], ebx
        mov     [port], ecx

; Connect to the other side.
        stdcall open_connection, [hostname], [port]
        test    eax, eax
        jz      .error
        mov     [socketnum], eax

; Create the HTTP request.
        invoke  mem.alloc, BUFFERSIZE
        test    eax, eax
        jz      .error
        mov     [buffer], eax
        mov     edi, eax
        DEBUGF  1, "Buffer allocated: 0x%x\n", eax

        mov     esi, str_get
        copy_till_zero

; If we are using a proxy, send complete URL, otherwise send only page address.
        cmp     [proxyAddr], 0
        je      .no_proxy
        mov     esi, str_http           ; prepend 'http://'
        copy_till_zero
        mov     esi, [hostname]
        copy_till_zero
  .no_proxy:
        mov     esi, [pageaddr]
        copy_till_zero

        mov     esi, str_http11
        mov     ecx, str_http11.length
        rep     movsb

        mov     esi, [hostname]
        copy_till_zero

        cmp     byte[proxyUser], 0
        je      @f
        call    append_proxy_auth_header
  @@:

        mov     ax, 0x0a0d
        stosw

        mov     esi, [add_header]
        test    esi, esi
        jz      @f
        copy_till_zero
  @@:

        mov     esi, str_close
        mov     ecx, str_close.length
        rep     movsb

        mov     byte[edi], 0
        DEBUGF  1, "Request:\n%s", [buffer]

; Free unused memory
        push    edi
        invoke  mem.free, [pageaddr]
        invoke  mem.free, [hostname]
        pop     esi

; Send the request
        sub     esi, [buffer]   ; length
        xor     edi, edi        ; flags
        mcall   send, [socketnum], [buffer]
        test    eax, eax
        jz      .error
        DEBUGF  1, "Request has been sent to server.\n"

        HTTP_init_buffer [buffer], [socketnum]

        popa
        mov     eax, [buffer]   ; return buffer ptr
        ret

  .error:
        DEBUGF  1, "Error!\n"
        popa
        xor     eax, eax        ; return 0 = error
        ret

endp



;;================================================================================================;;
proc HTTP_head URL, add_header ;//////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Initiates a HTTP connection, using 'HEAD' method.                                              ;;
;? This will only return HTTP header and status, no content                                       ;;
;;------------------------------------------------------------------------------------------------;;
;> URL          = pointer to ASCIIZ URL                                                           ;;
;> add_header   = pointer to additional header parameters (ASCIIZ), or null for none.             ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) / buffer ptr                                                                   ;;
;;================================================================================================;;
locals
        hostname        dd ?
        pageaddr        dd ?
        sockaddr        dd ?
        socketnum       dd ?
        buffer          dd ?
        port            dd ?
endl

        pusha
; split the URL into hostname and pageaddr
        stdcall parse_url, [URL]
        test    eax, eax
        jz      .error
        mov     [hostname], eax
        mov     [pageaddr], ebx
        mov     [port], ecx

; Connect to the other side.
        stdcall open_connection, [hostname], [port]
        test    eax, eax
        jz      .error
        mov     [socketnum], eax

; Create the HTTP request.
        invoke  mem.alloc, BUFFERSIZE
        test    eax, eax
        jz      .error
        mov     [buffer], eax
        mov     edi, eax
        DEBUGF  1, "Buffer has been allocated.\n"

        mov     esi, str_head
        copy_till_zero

; If we are using a proxy, send complete URL, otherwise send only page address.
        cmp     [proxyAddr], 0
        je      .no_proxy
        mov     esi, str_http           ; prepend 'http://'
        copy_till_zero
        mov     esi, [hostname]
        copy_till_zero
  .no_proxy:
        mov     esi, [pageaddr]
        copy_till_zero

        mov     esi, str_http11
        mov     ecx, str_http11.length
        rep     movsb

        mov     esi, [hostname]
        copy_till_zero

        cmp     byte[proxyUser], 0
        je      @f
        call    append_proxy_auth_header
  @@:

        mov     ax, 0x0a0d
        stosw

        mov     esi, [add_header]
        test    esi, esi
        jz      @f
        copy_till_zero
  @@:

        mov     esi, str_close
        mov     ecx, str_close.length
        rep     movsb

        mov     byte[edi], 0
        DEBUGF  1, "Request:\n%s", [buffer]


; Free unused memory
        push    edi
        invoke  mem.free, [pageaddr]
        invoke  mem.free, [hostname]
        pop     esi

; Send the request
        sub     esi, [buffer]   ; length
        xor     edi, edi        ; flags
        mcall   send, [socketnum], [buffer]
        test    eax, eax
        jz      .error
        DEBUGF  1, "Request has been sent to server.\n"

        HTTP_init_buffer [buffer], [socketnum]

        popa
        mov     eax, [buffer]
        ret                     ; return buffer ptr

  .error:
        DEBUGF  1, "Error!\n"
        popa
        xor     eax, eax        ; return 0 = error
        ret

endp


;;================================================================================================;;
proc HTTP_post URL, add_header, content_type, content_length ;////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Initiates a HTTP connection, using 'POST' method.                                              ;;
;? This method is used to send data to the HTTP server                                            ;;
;;------------------------------------------------------------------------------------------------;;
;> URL                  = pointer to ASCIIZ URL                                                   ;;
;> add_header           = pointer to additional header parameters (ASCIIZ), or null for none.     ;;
;> content_type         = pointer to ASCIIZ string containing content type                        ;;
;> content_length       = length of content (in bytes)                                            ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) / buffer ptr                                                                   ;;
;;================================================================================================;;
locals
        hostname        dd ?
        pageaddr        dd ?
        sockaddr        dd ?
        socketnum       dd ?
        buffer          dd ?
        port            dd ?
endl

        pusha
; split the URL into hostname and pageaddr
        stdcall parse_url, [URL]
        test    eax, eax
        jz      .error
        mov     [hostname], eax
        mov     [pageaddr], ebx
        mov     [port], ecx

; Connect to the other side.
        stdcall open_connection, [hostname], [port]
        test    eax, eax
        jz      .error
        mov     [socketnum], eax

; Create the HTTP request.
        invoke  mem.alloc, BUFFERSIZE
        test    eax, eax
        jz      .error
        mov     [buffer], eax
        mov     edi, eax
        DEBUGF  1, "Buffer has been allocated.\n"

        mov     esi, str_post
        copy_till_zero

; If we are using a proxy, send complete URL, otherwise send only page address.
        cmp     [proxyAddr], 0
        je      .no_proxy
        mov     esi, str_http           ; prepend 'http://'
        copy_till_zero
        mov     esi, [hostname]
        copy_till_zero
  .no_proxy:
        mov     esi, [pageaddr]
        copy_till_zero

        mov     esi, str_http11
        mov     ecx, str_http11.length
        rep     movsb

        mov     esi, [hostname]
        copy_till_zero

        mov     esi, str_post_cl
        mov     ecx, str_post_cl.length
        rep     movsb

        mov     eax, [content_length]
        call    eax_ascii_dec

        mov     esi, str_post_ct
        mov     ecx, str_post_ct.length
        rep     movsb

        mov     esi, [content_type]
        copy_till_zero

        cmp     byte[proxyUser], 0
        je      @f
        call    append_proxy_auth_header
  @@:

        mov     ax, 0x0a0d
        stosw

        mov     esi, [add_header]
        test    esi, esi
        jz      @f
        copy_till_zero
  @@:

        mov     esi, str_close
        mov     ecx, str_close.length
        rep     movsb

        mov     byte[edi], 0
        DEBUGF  1, "Request:\n%s", [buffer]

; Free unused memory
        push    edi
        invoke  mem.free, [pageaddr]
        invoke  mem.free, [hostname]
        pop     esi

; Send the request
        sub     esi, [buffer]   ; length
        xor     edi, edi        ; flags
        mcall   send, [socketnum], [buffer]
        test    eax, eax
        jz      .error
        DEBUGF  1, "Request has been sent to server.\n"

        HTTP_init_buffer [buffer], [socketnum]

        popa
        mov     eax, [buffer]
        ret                     ; return buffer ptr

  .error:
        DEBUGF  1, "Error!\n"
        popa
        xor     eax, eax        ; return 0 = error
        ret

endp



;;================================================================================================;;
proc HTTP_process identifier ;////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Receive data from the server, parse headers and put data in receive buffer.                    ;;
;? To complete a transfer, this procedure must be called over and over again untill it returns 0. ;;
;;------------------------------------------------------------------------------------------------;;
;> identifier   = pointer to buffer containing http_msg struct.                                   ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (not finished) / 0 finished                                                           ;;
;;================================================================================================;;

        pusha
        mov     ebp, [identifier]

; If the connection is closed, return immediately
        test    [ebp + http_msg.flags], FLAG_CONNECTED
        jz      .connection_closed

; Receive some data
        mcall   recv, [ebp + http_msg.socket], [ebp + http_msg.write_ptr], \
                      [ebp + http_msg.buffer_length], MSG_DONTWAIT
        cmp     eax, 0xffffffff
        je      .check_socket

        test    eax, eax
        jz      .server_closed
        DEBUGF  1, "Received %u bytes\n", eax

; Update timestamp
        push    eax
        mcall   26, 9
        mov     [ebp + http_msg.timestamp], eax
        pop     eax

; Update pointers
        mov     edi, [ebp + http_msg.write_ptr]
        add     [ebp + http_msg.write_ptr], eax
        sub     [ebp + http_msg.buffer_length], eax

; If data is chunked, combine chunks into contiguous data.
        test    [ebp + http_msg.flags], FLAG_CHUNKED
        jnz     .chunk_loop

; Did we detect the (final) header yet?
        test    [ebp + http_msg.flags], FLAG_GOT_HEADER
        jnz     .header_parsed

;--------------------------------------------------------------
;
; Header parsing code begins here
;

; We havent found the (final) header yet, search for it..
  .scan_again:
        ; eax = total number of bytes received so far
        mov     eax, [ebp + http_msg.write_ptr]
        sub     eax, http_msg.http_header
        sub     eax, ebp
        sub     eax, [ebp + http_msg.header_length]
        ; edi is ptr to begin of header
        lea     edi, [ebp + http_msg.http_header]
        add     edi, [ebp + http_msg.header_length]
        ; put it in esi for next proc too
        mov     esi, edi
        sub     eax, 3
        jle     .need_more_data_for_header
  .scan_loop:
        ; scan for end of header (empty line)
        cmp     dword[edi], 0x0a0d0a0d                  ; end of header
        je      .end_of_header
        cmp     word[edi+2], 0x0a0a                     ; notice the use of offset + 2, to calculate header length correctly :)
        je      .end_of_header
        inc     edi
        dec     eax
        jnz     .scan_loop
        jmp     .need_more_data_for_header

  .end_of_header:
        add     edi, 4 - http_msg.http_header
        sub     edi, ebp
        mov     [ebp + http_msg.header_length], edi     ; If this isnt the final header, we'll use this as an offset to find real header.
        DEBUGF  1, "Header length: %u\n", edi

; Ok, we have found the header
        cmp     dword[esi], 'HTTP'
        jne     .err_header
        cmp     dword[esi+4], '/1.0'
        je      .http_1.0
        cmp     dword[esi+4], '/1.1'
        jne     .err_header
        or      [ebp + http_msg.flags], FLAG_HTTP11
  .http_1.0:
        cmp     byte[esi+8], ' '
        jne     .err_header

        add     esi, 9
        xor     eax, eax
        xor     ebx, ebx
        mov     ecx, 3
  .statusloop:
        lodsb
        sub     al, '0'
        jb      .err_header
        cmp     al, 9
        ja      .err_header
        lea     ebx, [ebx + 4*ebx]
        shl     ebx, 1
        add     ebx, eax
        dec     ecx
        jnz     .statusloop

; Ignore "100 - Continue" lines
        cmp     ebx, 100
        je      .scan_again

        DEBUGF  1, "Status: %u\n", ebx
        mov     [ebp + http_msg.status], ebx
        or      [ebp + http_msg.flags], FLAG_GOT_HEADER

; Now, convert all header names to lowercase.
; This way, it will be much easier to find certain header fields, later on.
        lea     esi, [ebp + http_msg.http_header]
        mov     ecx, [ebp + http_msg.header_length]
  .need_newline:
        inc     esi
        dec     ecx
        jz      .convert_done
        cmp     byte[esi], 10
        jne     .need_newline
; We have found a newline
; A line beginning with space or tabs has no header fields.
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
        lea     esi, [ebp + http_msg.http_header]
        DEBUGF  1, "Header names converted to lowercase:\n%s\n", esi

; Check for content-length header field.
        stdcall HTTP_find_header_field, ebp, str_cl
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
        jb      .err_header
        cmp     bl, 9
        ja      .err_header
        lea     edx, [edx + edx*4]      ; edx = edx*10
        shl     edx, 1                  ;
        add     edx, ebx
        jmp     .cl_loop

  .cl_ok:
        mov     [ebp + http_msg.content_length], edx
        DEBUGF  1, "Content-length: %u\n", edx

        test    edx, edx
        jz      .got_all_data

        call    alloc_contentbuff
        test    eax, eax
        jz      .err_no_ram
        xor     eax, eax
        jmp     .header_parsed

  .no_content:
        DEBUGF  1, "Content-length not found.\n"

; We didnt find 'content-length', maybe server is using chunked transfer encoding?
; Try to find 'transfer-encoding' header.
        stdcall HTTP_find_header_field, ebp, str_te
        test    eax, eax
        jnz     .ct_hdr_found

  .not_chunked:
        mov     edx, BUFFERSIZE
        call    alloc_contentbuff
        test    eax, eax
        jz      .err_no_ram
        xor     eax, eax
        jmp     .header_parsed

  .ct_hdr_found:
        mov     ebx, dword[eax]
        or      ebx, 0x20202020
        cmp     ebx, 'chun'
        jne     .not_chunked
        mov     ebx, dword[eax+4]
        or      ebx, 0x00202020
        and     ebx, 0x00ffffff
        cmp     ebx, 'ked'
        jne     .not_chunked

        or      [ebp + http_msg.flags], FLAG_CHUNKED
        DEBUGF  1, "Transfer type is: chunked\n"

        mov     edx, BUFFERSIZE
        call    alloc_contentbuff
        test    eax, eax
        jz      .err_no_ram

; Set chunk pointer where first chunk should begin.
        mov     eax, [ebp + http_msg.content_ptr]
        mov     [ebp + http_msg.chunk_ptr], eax

;--------------------------------------------------------------
;
; Chunk parsing code begins here
;

  .chunk_loop:
        mov     ecx, [ebp + http_msg.write_ptr]
        sub     ecx, [ebp + http_msg.chunk_ptr]
        jbe     .need_more_data_chunked

; Chunkline starts here, convert the ASCII hex number into ebx
        mov     esi, [ebp + http_msg.chunk_ptr]
        DEBUGF  1, "Chunkline begins at 0x%x\n", esi

        xor     ebx, ebx
        cmp     byte[esi], 0x0d
        jne     .chunk_hex_loop
        dec     ecx
        jz      .need_more_data_chunked
        inc     esi
        cmp     byte[esi], 0x0a
        jne     .chunk_hex_loop
        dec     ecx
        jz      .need_more_data_chunked
        inc     esi
  .chunk_hex_loop:
        lodsb
        sub     al, '0'
        jb      .chunk_hex_end
        cmp     al, 9
        jbe     .chunk_hex
        sub     al, 'A' - '0' - 10
        jb      .chunk_hex_end
        cmp     al, 15
        jbe     .chunk_hex
        sub     al, 'a' - 'A'
        cmp     al, 15
        ja      .chunk_hex_end
  .chunk_hex:
        shl     ebx, 4
        add     bl, al
        dec     ecx
        jnz     .chunk_hex_loop
        jmp     .need_more_data_chunked
  .chunk_hex_end:
; Chunkline ends with a CR LF or simply LF
        dec     esi
  .end_of_chunkline?:
        lodsb
        cmp     al, 10                                  ; chunkline must always end with LF
        je      .end_of_chunkline
        dec     ecx
        jnz     .end_of_chunkline?
        xor     eax, eax
        jmp     .need_more_data_chunked                 ; chunkline is incomplete, request more data
  .end_of_chunkline:
        DEBUGF  1, "Chunk of 0x%x bytes\n", ebx
; If chunk size is 0, all chunks have been received.
        test    ebx, ebx
        jz      .got_all_data_chunked
; Calculate how many data bytes we have received already
        mov     ecx, [ebp + http_msg.write_ptr]
        sub     ecx, [ebp + http_msg.chunk_ptr]         ; ecx is now number of received data bytes
; Update content_received counter
        add     [ebp + http_msg.content_received], ecx
; Calculate new write ptr
        mov     edx, esi
        sub     edx, [ebp + http_msg.chunk_ptr]         ; edx is now length of chunkline
        sub     [ebp + http_msg.write_ptr], edx
; Realloc buffer, make it 'chunksize' bigger.
        lea     edx, [ebx + BUFFERSIZE]
        mov     [ebp + http_msg.buffer_length], edx     ; remaining space in new buffer
        add     edx, [ebp + http_msg.write_ptr]
        sub     edx, [ebp + http_msg.content_ptr]
        DEBUGF  1, "Resizing buffer 0x%x, it will now be %u bytes\n", [ebp + http_msg.content_ptr], edx
        invoke  mem.realloc, [ebp + http_msg.content_ptr], edx
        DEBUGF  1, "New buffer = 0x%x\n", eax
        or      eax, eax
        jz      .err_no_ram
        call    recalculate_pointers                    ; Because it's possible that buffer begins on another address now
        add     esi, eax                                ; recalculate esi too!
; Remove chunk header (aka chunkline) from the buffer by shifting all received data after chunkt_ptr to the left
        mov     edi, [ebp + http_msg.chunk_ptr]
        rep movsb
; Update chunk ptr to point to next chunk
        add     [ebp + http_msg.chunk_ptr], ebx
; Set number of received bytes to 0, we already updated content_received
        xor     eax, eax
        jmp     .chunk_loop

;--------------------------------------------------------------
;
; end of proc code begins here
;

  .header_parsed:
        ; Header was already parsed and connection isnt chunked.
        ; Update content_received
        add     [ebp + http_msg.content_received], eax
        ; If we received content-length parameter, check if we received all the data
        test    [ebp + http_msg.flags], FLAG_CONTENT_LENGTH
        jz      @f
        mov     eax, [ebp + http_msg.content_received]
        cmp     eax, [ebp + http_msg.content_length]
        jae     .got_all_data
  @@:
        cmp     [ebp + http_msg.buffer_length], 0
        je      .buffer_full
        ; Need more data
        popa
        xor     eax, eax
        dec     eax
        ret

  .buffer_full:
        ; Lets make it bigger..
        mov     eax, [ebp + http_msg.write_ptr]
        add     eax, BUFFERSIZE
        sub     eax, [ebp + http_msg.content_ptr]
        invoke  mem.realloc, [ebp + http_msg.content_ptr], eax
        or      eax, eax
        jz      .err_no_ram
        call    recalculate_pointers
        mov     [ebp + http_msg.buffer_length], BUFFERSIZE
        ; Need more data
        popa
        xor     eax, eax
        dec     eax
        ret

  .need_more_data_for_header:
        cmp     [ebp + http_msg.buffer_length], 0
        je      .err_header                     ; It's just too damn long!
        ; Need more data
        popa
        xor     eax, eax
        dec     eax
        ret

  .need_more_data_chunked:
        ; We only got a partial chunk, or need more chunks, update content_received and request more data
        add     [ebp + http_msg.content_received], eax
        popa
        xor     eax, eax
        dec     eax
        ret

  .got_all_data_chunked:
        ; Woohoo, we got all the chunked data, calculate total number of bytes received.
        mov     eax, [ebp + http_msg.chunk_ptr]
        sub     eax, [ebp + http_msg.content_ptr]
        mov     [ebp + http_msg.content_length], eax
        mov     [ebp + http_msg.content_received], eax
  .got_all_data:
        DEBUGF  1, "We got all the data! (%u bytes)\n", [ebp + http_msg.content_received]
        or      [ebp + http_msg.flags], FLAG_GOT_ALL_DATA
        and     [ebp + http_msg.flags], not FLAG_CONNECTED
        mcall   close, [ebp + http_msg.socket]
        popa
        xor     eax, eax
        ret

;--------------------------------------------------------------
;
; error handeling code begins here
;

  .check_socket:
        cmp     ebx, EWOULDBLOCK
        jne     .err_socket
        mcall   26, 9
        sub     eax, [ebp + http_msg.timestamp]
        cmp     eax, TIMEOUT
        ja      .err_timeout
        ; Need more data
        popa
        xor     eax, eax
        dec     eax
        ret

  .server_closed:
        DEBUGF  1, "server closed connection, transfer complete?\n"
        test    [ebp + http_msg.flags], FLAG_GOT_HEADER
        jz      .err_server_closed
        test    [ebp + http_msg.flags], FLAG_CONTENT_LENGTH
        jz      .got_all_data
  .err_server_closed:
        pop     eax
        DEBUGF  1, "ERROR: server closed connection unexpectedly\n"
        or      [ebp + http_msg.flags], FLAG_TRANSFER_FAILED
        jmp     .abort

  .err_header:
        pop     eax
        DEBUGF  1, "ERROR: invalid header\n"
        or      [ebp + http_msg.flags], FLAG_INVALID_HEADER
        jmp     .abort

  .err_no_ram:
        DEBUGF  1, "ERROR: out of RAM\n"
        or      [ebp + http_msg.flags], FLAG_NO_RAM
        jmp     .abort

  .err_timeout:
        DEBUGF  1, "ERROR: timeout\n"
        or      [ebp + http_msg.flags], FLAG_TIMEOUT_ERROR
        jmp     .abort

  .err_socket:
        DEBUGF  1, "ERROR: socket error %u\n", ebx
        or      [ebp + http_msg.flags], FLAG_SOCKET_ERROR
  .abort:
        and     [ebp + http_msg.flags], not FLAG_CONNECTED
        mcall   close, [ebp + http_msg.socket]
  .connection_closed:
        popa
        xor     eax, eax
        ret

endp


alloc_contentbuff:

; Allocate content buffer
        invoke  mem.alloc, edx
        or      eax, eax
        jz      .no_ram

        DEBUGF  1, "Content buffer allocated: 0x%x\n", eax

; Copy already received content into content buffer
        mov     edi, eax
        lea     esi, [ebp + http_msg.http_header]
        add     esi, [ebp + http_msg.header_length]
        mov     ecx, [ebp + http_msg.write_ptr]
        sub     ecx, esi
        mov     ebx, ecx
        rep movsb

; Update pointers to point to new buffer
        mov     [ebp + http_msg.content_ptr], eax
        mov     [ebp + http_msg.content_received], ebx
        sub     edx, ebx
        mov     [ebp + http_msg.buffer_length], edx
        add     eax, ebx
        mov     [ebp + http_msg.write_ptr], eax

; Shrink header buffer
        mov     eax, http_msg.http_header
        add     eax, [ebp + http_msg.header_length]
        invoke  mem.realloc, ebp, eax
        or      eax, eax
  .no_ram:

        ret



recalculate_pointers:

        sub     eax, [ebp + http_msg.content_ptr]
        jz      .done
        add     [ebp + http_msg.content_ptr], eax
        add     [ebp + http_msg.write_ptr], eax
        add     [ebp + http_msg.chunk_ptr], eax

  .done:
        ret




;;================================================================================================;;
proc HTTP_free identifier ;///////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Free the http_msg structure                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> identifier   = pointer to buffer containing http_msg struct.                                   ;;
;;------------------------------------------------------------------------------------------------;;
;< none                                                                                           ;;
;;================================================================================================;;
        DEBUGF  1, "HTTP_free: 0x%x\n", [identifier]
        pusha
        mov     ebp, [identifier]

        test    [ebp + http_msg.flags], FLAG_CONNECTED
        jz      .not_connected

        and     [ebp + http_msg.flags], not FLAG_CONNECTED
        mcall   close, [ebp + http_msg.socket]

  .not_connected:
        invoke  mem.free, ebp

        popa
        ret

endp



;;================================================================================================;;
proc HTTP_stop identifier ;///////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Stops the open connection                                                                      ;;
;;------------------------------------------------------------------------------------------------;;
;> identifier   = pointer to buffer containing http_msg struct.                                   ;;
;;------------------------------------------------------------------------------------------------;;
;< none                                                                                           ;;
;;================================================================================================;;

        pusha
        mov     ebp, [identifier]

        and     [ebp + http_msg.flags], not FLAG_CONNECTED
        mcall   close, [ebp + http_msg.socket]

        popa
        ret

endp



;;================================================================================================;;
proc HTTP_find_header_field identifier, headername ;//////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Find a header field in the received HTTP header                                                ;;
;;------------------------------------------------------------------------------------------------;;
;> identifier   = ptr to http_msg struct                                                          ;;
;> headername   = ptr to ASCIIZ string containg field you want to find (must be in lowercase)     ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) / ptr to content of the HTTP header field                                      ;;
;;================================================================================================;;
        push    ebx ecx edx esi edi

        DEBUGF  1, "Find header field: %s\n", [headername]

        mov     ebx, [identifier]
        test    [ebx + http_msg.flags], FLAG_GOT_HEADER
        jz      .fail

        lea     edx, [ebx + http_msg.http_header]
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
        DEBUGF  1, "Header field not found\n"
        pop     edi esi edx ecx ebx
        xor     eax, eax
        ret

endp



;;================================================================================================;;
proc URI_escape URI ;/////////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;?                                                                                                ;;
;;------------------------------------------------------------------------------------------------;;
;> URI = ptr to ASCIIZ URI                                                                        ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) / ptr to ASCIIZ URI                                                            ;;
;;================================================================================================;;

        pusha

        invoke  mem.alloc, URLMAXLEN
        test    eax, eax
        jz      .error
        mov     [esp + 7 * 4], eax              ; return ptr in eax
        mov     esi, [URI]
        mov     edi, eax
        xor     ebx, ebx
        xor     ecx, ecx
  .loop:
        lodsb
        test    al, al
        jz      .done

        mov     cl, al
        and     cl, 0x1f
        mov     bl, al
        shr     bl, 5
        bt      dword[bits_must_escape + ebx], ecx
        jc      .escape

        stosb
        jmp     .loop

  .escape:
        mov     al, '%'
        stosb
        mov     bl, byte[esi-1]
        shr     bl, 4
        mov     al, byte[str_hex + ebx]
        stosb
        mov     bl, byte[esi-1]
        and     bl, 0x0f
        mov     al, byte[str_hex + ebx]
        stosb
        jmp     .loop


  .done:
        stosb

        popa
        ret

  .error:
        popa
        xor     eax, eax
        ret

endp



;;================================================================================================;;
proc URI_unescape URI ;///////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;?                                                                                                ;;
;;------------------------------------------------------------------------------------------------;;
;> URI = ptr to ASCIIZ URI                                                                        ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) / ptr to ASCIIZ URI                                                            ;;
;;================================================================================================;;

        pusha

        invoke  mem.alloc, URLMAXLEN
        test    eax, eax
        jz      .error
        mov     [esp + 7 * 4], eax              ; return ptr in eax
        mov     esi, [URI]
        mov     edi, eax
  .loop:
        lodsb
        test    al, al
        jz      .done

        cmp     al, '%'
        je      .unescape

        stosb
        jmp     .loop

  .unescape:
        xor     ebx, ebx
        xor     ecx, ecx
  .unescape_nibble:
        lodsb
        sub     al, '0'
        jb      .fail
        cmp     al, 9
        jbe     .nibble_ok
        sub     al, 'A' - '0' - 10
        jb      .fail
        cmp     al, 15
        jbe     .nibble_ok
        sub     al, 'a' - 'A'
        cmp     al, 15
        ja      .fail
  .nibble_ok:
        shl     bl, 8
        or      bl, al
        dec     ecx
        jc      .unescape_nibble
        mov     al, bl
        stosb
        jmp     .loop

  .fail:
        DEBUGF  1, "ERROR: invalid URI!\n"
        jmp     .loop

  .done:
        stosb

        popa
        ret

  .error:
        popa
        xor     eax, eax
        ret

endp





;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Internal procedures section                                                                    ;;
;;                                                                                                ;;
;; NOTICE: These procedures do not follow stdcall conventions and thus may destroy any register.  ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;




;;================================================================================================;;
proc open_connection hostname, port ;/////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Connects to a HTTP server                                                                      ;;
;;------------------------------------------------------------------------------------------------;;
;> hostname     = ptr to ASCIIZ hostname                                                          ;;
;> port         = port (x86 byte order)                                                           ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) / socketnum                                                                    ;;
;;================================================================================================;;

locals
        sockaddr        dd ?
        socketnum       dd ?
endl

        cmp     [proxyAddr], 0
        je      .no_proxy

        mov     [hostname], proxyAddr

        push    [proxyPort]
        pop     [port]
  .no_proxy:

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
        jnz     .error1

; getaddrinfo returns addrinfo struct, make the pointer to sockaddr struct
        mov     esi, [esi + addrinfo.ai_addr]
        mov     [sockaddr], esi
        mov     eax, [esi + sockaddr_in.sin_addr]
        test    eax, eax
        jz      .error2

        DEBUGF  1, "Server ip=%u.%u.%u.%u\n", \
        [esi + sockaddr_in.sin_addr]:1, [esi + sockaddr_in.sin_addr + 1]:1, \
        [esi + sockaddr_in.sin_addr + 2]:1, [esi + sockaddr_in.sin_addr + 3]:1

        mov     [esi + sockaddr_in.sin_family], AF_INET4
        mov     eax, [port]
        xchg    al, ah
        mov     [esi + sockaddr_in.sin_port], ax

; Connect to the server.
        mcall   socket, AF_INET4, SOCK_STREAM, 0
        test    eax, eax
        jz      .error2
        mov     [socketnum], eax
        DEBUGF  1, "Socket: 0x%x\n", eax

        mcall   connect, [socketnum], [sockaddr], 18
        test    eax, eax
        jnz     .error2
        DEBUGF  1, "Socket is now connected.\n"

; free allocated memory
        push    [sockaddr]
        call    [freeaddrinfo]

        mov     eax, [socketnum]
        ret

  .error2:

; free allocated memory
        push    [sockaddr]
        call    [freeaddrinfo]

  .error1:
        xor     eax, eax
        ret

endp


;;================================================================================================;;
proc parse_url URL ;//////////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Split a given URL into hostname and pageaddr                                                   ;;
;;------------------------------------------------------------------------------------------------;;
;> URL = ptr to ASCIIZ URL                                                                        ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) / ptr to ASCIIZ hostname                                                       ;;
;< ebx = ptr to ASCIIZ pageaddr                                                                   ;;
;< ecx = port number                                                                              ;;
;;================================================================================================;;

locals
        urlsize         dd ?
        hostname        dd ?
        pageaddr        dd ?
        port            dd ?
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
        jmp     .invalid

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


; Create new buffer and put hostname in it.
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

; Check if user provided a port, and convert it if so.
        mov     esi, [hostname]
        mov     [port], 80              ; default port if user didnt provide one
  .portloop:
        lodsb
        test    al, al
        jz      .no_port
        cmp     al, ':'
        jne     .portloop

        push    esi
        call    ascii_dec_ebx
        pop     edi
        cmp     byte[esi-1], 0
        jne     .invalid
        cmp     [proxyAddr], 0          ; remove port number from hostname
        jne     @f                      ; unless when we are using proxy
        mov     byte[edi-1], 0
  @@:
        test    ebx, ebx
        je      .invalid
        cmp     ebx, 0xffff
        ja      .invalid
        mov     [port], ebx
  .no_port:


; Did user provide a pageaddr?
        mov     [pageaddr], str_slash   ; assume there is no pageaddr
        pop     esi ecx
        test    ecx, ecx
        jz      .no_page

; Create new buffer and put pageaddr into it.
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
        mov     ecx, [port]

        DEBUGF  1, "hostname: %s\n", eax
        DEBUGF  1, "pageaddr: %s\n", ebx
        DEBUGF  1, "port: %u\n", ecx

        ret

  .no_mem:
        DEBUGF  1, "Out of memory!\n"
        xor     eax, eax
        ret

  .invalid:
        DEBUGF  1, "Invalid URL!\n"
        xor     eax, eax
        ret

endp





;;================================================================================================;;
proc append_proxy_auth_header ;///////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Appends the proxy authentication header                                                        ;;
;;------------------------------------------------------------------------------------------------;;
;> /                                                                                              ;;
;;------------------------------------------------------------------------------------------------;;
;< /                                                                                              ;;
;;================================================================================================;;
        mov     esi, str_proxy_auth
        mov     ecx, str_proxy_auth.length
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

endp


;;================================================================================================;;
proc eax_ascii_dec ;//////////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Convert eax to ASCII decimal number                                                            ;;
;;------------------------------------------------------------------------------------------------;;
;> eax = number                                                                                   ;;
;> edi = ptr where to write ASCII decimal number                                                  ;;
;;------------------------------------------------------------------------------------------------;;
;< /                                                                                              ;;
;;================================================================================================;;

        push    -'0'
        mov     ecx, 10
  .loop:
        xor     edx, edx
        div     ecx
        add     dl, '0'
        push    edx
        test    eax, eax
        jnz     .loop

  .loop2:
        pop     eax
        add     al, '0'
        jz      .done
        stosb
        jmp     .loop2
  .done:

        ret

endp


;;================================================================================================;;
proc ascii_dec_ebx ;//////////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Convert ASCII decimal number to ebx                                                            ;;
;;------------------------------------------------------------------------------------------------;;
;> esi = ptr where to read ASCII decimal number                                                   ;;
;;------------------------------------------------------------------------------------------------;;
;> ebx = number                                                                                   ;;
;;================================================================================================;;

        xor     eax, eax
        xor     ebx, ebx
  .loop:
        lodsb
        sub     al, '0'
        jb      .done
        cmp     al, 9
        ja      .done
        lea     ebx, [ebx + 4*ebx]
        shl     ebx, 1
        add     ebx, eax
        jmp     .loop
  .done:

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
        HTTP_head               , 'head'                , \
        HTTP_post               , 'post'                , \
        HTTP_find_header_field  , 'find_header_field'   , \
        HTTP_process            , 'process'             , \
        HTTP_free               , 'free'                , \
        HTTP_stop               , 'stop'                , \
        URI_escape              , 'escape'              , \
        URI_unescape            , 'unescape'

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
str_post_cl     db 13, 10, 'Content-Length: '
  .length       = $ - str_post_cl
str_post_ct     db 13, 10, 'Content-Type: '
  .length       = $ - str_post_ct
str_proxy_auth  db 13, 10, 'Proxy-Authorization: Basic '
  .length       = $ - str_proxy_auth
str_close       db 'User-Agent: KolibriOS libHTTP/1.0', 13, 10, 'Connection: Close', 13, 10, 13, 10
  .length       = $ - str_close

str_http        db 'http://', 0

base64_table    db 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'
                db '0123456789+/'

str_cl          db 'content-length', 0
str_slash       db '/', 0
str_te          db 'transfer-encoding', 0
str_get         db 'GET ', 0
str_head        db 'HEAD ', 0
str_post        db 'POST ', 0

bits_must_escape:
dd      0xffffffff                                                      ; 00-1F
dd      1 shl 0 + 1 shl 2 + 1 shl 3 + 1 shl 5 + 1 shl 28 + 1 shl 30     ; "#%<>
dd      1 shl 27 + 1 shl 28 + 1 shl 29 + 1 shl 30                       ;[\]^
dd      1 shl 0 + 1 shl 27 + 1 shl 28 + 1 shl 29 + 1 shl 31             ;`{|} DEL

dd      0xffffffff
dd      0xffffffff
dd      0xffffffff
dd      0xffffffff

str_hex:
db '0123456789ABCDEF'

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