;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2024. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; boot data: common strings (for all languages)
macro line_full_top {
        db      201
        times   78 db 205
        db      187
}
macro line_full_bottom {
        db      200
        times   78 db 205
        db      188
}
macro line_half {
        db      186,' '
        times   76 db 0xc4
        db      ' ', 186
}
macro line_space {
        db      186
        times   78 db 32
        db      186
}
d80x25_top:
        line_full_top
space_msg:
        line_space
; Version line in git describe format with the build time at the right end:
; KolibriOS 0.7.7.0-9230-g723a1b44           auto-build 11:08 09.10.2026
; BUILD_TIME is YYYYMMDDhhmm in UTC.
verstr:
        db      186, ' '
repeat 78
  load a byte from kernel_header.signature+%-1
  if a = 13
    break
  end if
  db a
end repeat
if BUILD_CMTID > 0
        db      '-'
        db_decimal BUILD_OFFSET
        db      '-g'
        db_digits BUILD_CMTID, 8, 16
end if
if BUILD_DBGTAG > 0
        db      '-', BUILD_DBGTAG
end if
if BUILD_TIME > 0
        times   verstr + 78 - (.time_end - .time) - $ db ' '
.time:
  if lang eq ru_RU
        cp866   "автосборка"
  else if lang eq es_ES
        cp850   "autocompilación"
  else
        db      "auto-build"
  end if
        db      ' '
        db_digits (BUILD_TIME / 100) mod 100, 2, 10     ; hh
        db      ':'
        db_digits BUILD_TIME mod 100, 2, 10             ; mm
        db      ' '
        db_digits (BUILD_TIME / 10000) mod 100, 2, 10   ; DD
        db      '.'
        db_digits (BUILD_TIME / 1000000) mod 100, 2, 10 ; MM
        db      '.'
        db_digits BUILD_TIME / 100000000, 4, 10         ; YYYY
.time_end:
end if
        times   verstr + 78 - $ db ' '
        db      ' ', 186
        line_half
d80x25_top_num = 4
