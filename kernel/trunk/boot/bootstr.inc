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
; draw seven digits of the git hash
__x = BUILD_CMTID
if __x > 0
  __x = __x SHR 4
  cur_line_pos = 75
  store byte ' ' at d80x25_top+cur_line_pos+1
  repeat 7
    digit = __x AND 0xf
    if digit > 9
      digit = digit - 10 + 'a'
    else
      digit = digit + '0'
    end if
    store byte digit at d80x25_top+cur_line_pos
    cur_line_pos = cur_line_pos - 1
    __x = __x SHR 4
  end repeat
  store byte ' ' at d80x25_top+cur_line_pos
  store dword ' GIT' at d80x25_top+cur_line_pos-4
end if
space_msg:
        line_space
verstr:
; version string
db 186,32
repeat 78
  load a byte from kernel_header.signature+%-1
  if a = 13
    break
  end if
  db a
end repeat
repeat 78 - ($-verstr)
  db ' '
end repeat
db 32,186
        line_half
d80x25_top_num = 4
