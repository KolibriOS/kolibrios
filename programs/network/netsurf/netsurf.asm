;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2026. All rights reserved.         ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  netsurf - launcher/installer stub for the NetSurf browser      ;;
;;                                                                 ;;
;;  1. run /tmp0/1/netsurf/netsurf if it is already there          ;;
;;  2. otherwise fetch netsurf.7z with /sys/network/dl, which      ;;
;;     unpacks it into /tmp0/1/netsurf/                            ;;
;;  3. wait for the unpacked binary and run it                     ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format binary as ""
use32
        org     0x0
        db      'MENUET01'      ; header
        dd      0x01            ; header version
        dd      START           ; entry point
        dd      I_END           ; image size
        dd      MEM             ; required memory
        dd      MEM             ; esp
        dd      0x0             ; I_Param
        dd      0x0             ; I_Path

include '../../macros.inc'

; how long we are willing to wait for the download and unpacking to finish
DELAY_CS   = 50                 ; 0.5 s per poll
MAX_POLLS  = 600                ; 600 * 0.5 s = 5 minutes

START:
        call    netsurf_exists
        je      run_netsurf

; not installed yet - let the downloader fetch and unpack the archive
        mcall   70, fi_dl
        test    eax, eax
        js      download_failed

; poll until the downloader (and unz behind it) produces the binary
        mov     ebx, MAX_POLLS
.wait:
        mcall   5, DELAY_CS
        call    netsurf_exists
        je      run_netsurf
        dec     ebx
        jnz     .wait

download_failed:
        mcall   70, fi_notify
        mcall   -1

run_netsurf:
        mcall   70, fi_run
        mcall   -1

;-----------------------------------------------------------------------------
; Returns ZF=1 when /tmp0/1/netsurf/netsurf is present
;-----------------------------------------------------------------------------
netsurf_exists:
        mcall   70, fi_stat
        test    eax, eax
        ret

;-----------------------------------------------------------------------------
; Data area
;-----------------------------------------------------------------------------
netsurf_path    db '/tmp0/1/downloads/netsurf/netsurf', 0

; "URL|save path" tells the downloader where to put the archive,
; "-e " makes it close itself once the file is saved and unpacked
dl_param        db '-e http://ftp.kolibrios.org/users/Leency/netsurf/netsurf.zip',0
dl_path         db '/sys/network/dl', 0

notify_param    db "'NetSurf\nDownload failed' -E", 0

align 4
fi_stat         dd 5
                dd 0, 0, 0
                dd bdvk_buf
                db 0
                dd netsurf_path

fi_run          dd 7
                dd 0, 0, 0, 0
                db 0
                dd netsurf_path

fi_dl           dd 7
                dd 0
                dd dl_param
                dd 0, 0
                db 0
                dd dl_path

fi_notify       dd 7
                dd 0
                dd notify_param
                dd 0, 0
                db 0
                db '/sys/@notify', 0

I_END:
align 4
bdvk_buf        rb 560
                rb 1024         ; stack
MEM:
