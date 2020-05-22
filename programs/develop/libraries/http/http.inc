;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2020. All rights reserved.    ;;
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


; Bitflags for http_msg.flags

; status
FLAG_HTTP11             = 1 shl 0
FLAG_GOT_HEADER         = 1 shl 1
FLAG_GOT_ALL_DATA       = 1 shl 2
FLAG_CONTENT_LENGTH     = 1 shl 3
FLAG_CHUNKED            = 1 shl 4
FLAG_CONNECTED          = 1 shl 5

; user options
FLAG_KEEPALIVE          = 1 shl 8
FLAG_STREAM             = 1 shl 9
FLAG_REUSE_BUFFER       = 1 shl 10
FLAG_BLOCK              = 1 shl 11
FLAG_RING               = 1 shl 12

; error
FLAG_INVALID_HEADER     = 1 shl 16
FLAG_NO_RAM             = 1 shl 17      ; alloc failed
FLAG_SOCKET_ERROR       = 1 shl 18
FLAG_TIMEOUT_ERROR      = 1 shl 19
FLAG_TRANSFER_FAILED    = 1 shl 20
FLAG_NEED_MORE_SPACE    = 1 shl 21      ; need more space in existing buffer

struc http_msg {

        .socket                 dd ?    ; socket on which the actual transfer happens
        .flags                  dd ?    ; flags, reflects status of the transfer using bitflags
        .write_ptr              dd ?    ; internal use only (where to write new data in buffer)
        .buffer_length          dd ?    ; internal use only (number of available bytes in buffer)
        .chunk_ptr              dd ?    ; internal use only (where the next chunk begins)
        .timestamp              dd ?    ; internal use only (when last data was received)

        .status                 dd ?    ; HTTP status
        .header_length          dd ?    ; length of HTTP header
        .content_ptr            dd ?    ; ptr to content
        .content_length         dd ?    ; total length of HTTP content
        .content_received       dd ?    ; number of currently received content bytes

        .http_header:

}