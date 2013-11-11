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


FLAG_HTTP11             = 1 shl 0
FLAG_GOT_HEADER         = 1 shl 1
FLAG_GOT_ALL_DATA       = 1 shl 2
FLAG_CONTENT_LENGTH     = 1 shl 3
FLAG_CHUNKED            = 1 shl 4
FLAG_CONNECTED          = 1 shl 5

; error flags go into the upper word
FLAG_INVALID_HEADER     = 1 shl 16
FLAG_NO_RAM             = 1 shl 17
FLAG_SOCKET_ERROR       = 1 shl 18

struc http_msg {
        .socket         dd ?
        .flags          dd ?
        .write_ptr      dd ?
        .buffer_length  dd ?
        .chunk_ptr      dd ?

        .status         dd ?
        .header_length  dd ?
        .content_length dd ?
        .content_received   dd ?
        .data:
}