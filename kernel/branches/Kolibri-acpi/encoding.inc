;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2013-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 5082 $

; fetch the UTF-8 character in addrspace:offs to char
macro fetch_utf8_char addrspace, offs, char
{ local first_byte, b
  ; fetch first byte
  load first_byte byte from addrspace:offs
  if first_byte < 0x80
    char = first_byte
    offs = offs + 1
  else if first_byte < 0xC0
    err Invalid UTF-8 string
  else if first_byte < 0xE0
    char = first_byte and 0x1F
    load b byte from addrspace:offs + 1
    char = (char shl 6) + (b and 0x3F)
    offs = offs + 2
  else if first_byte < 0xF0
    char = first_byte and 0xF
    load b byte from addrspace:offs + 1
    char = (char shl 6) + (b and 0x3F)
    load b byte from addrspace:offs + 2
    char = (char shl 6) + (b and 0x3F)
    offs = offs + 3
  else if first_byte < 0xF8
    char = first_byte and 0x7
    load b byte from addrspace:offs + 1
    char = (char shl 6) + (b and 0x3F)
    load b byte from addrspace:offs + 2
    char = (char shl 6) + (b and 0x3F)
    load b byte from addrspace:offs + 3
    char = (char shl 6) + (b and 0x3F)
    offs = offs + 4
  else
    err Invalid UTF-8 string
  end if
}

; Worker macro for all encodings.
; Common part for all encodings: map characters 0-0x7F trivially,
; translate pseudographics.
; Pseudographics for the boot screen:
; 0x2500 -> 0xC4, 0x2502 -> 0xB3, 0x250C -> 0xDA, 0x2510 -> 0xBF,
; 0x2514 -> 0xC0, 0x2518 -> 0xD9, 0x252C -> 0xC2, 0x2534 -> 0xC1, 0x2551 -> 0xBA
macro convert_utf8 encoding, [arg]
{ common
  local ..addrspace, offs, char
  offs = 0
  virtual at 0
  ..addrspace:: db arg
  ..addrspace#.size = $
  end virtual
  while offs < ..addrspace#.size
    fetch_utf8_char ..addrspace, offs, char
    if char = 0x2500
      db 0xC4
    else if char = 0x2502
      db 0xB3
    else if char = 0x250C
      db 0xDA
    else if char = 0x2510
      db 0xBF
    else if char = 0x2514
      db 0xC0
    else if char = 0x2518
      db 0xD9
    else if char = 0x252C
      db 0xC2
    else if char = 0x2534
      db 0xC1
    else if char = 0x2551
      db 0xBA
    else if char < 0x80
      db char
    else
      encoding char
    end if
  end while
}

macro declare_encoding encoding
{
  macro encoding [arg]
  \{ common convert_utf8 encoding#char, arg \}
  struc encoding [arg]
  \{ common convert_utf8 encoding#char, arg \}
  macro encoding#char char
}

; Russian: use CP866.
; 0x410-0x43F -> 0x80-0xAF
; 0x440-0x44F -> 0xE0-0xEF
; 0x401 -> 0xF0, 0x451 -> 0xF1
declare_encoding cp866
{
  if char = 0x401
    db 0xF0
  else if char = 0x451
    db 0xF1
  else if (char < 0x410) | (char > 0x44F)
    err Failed to convert to CP866
  else if char < 0x440
    db char - 0x410 + 0x80
  else
    db char - 0x440 + 0xE0
  end if
}

; Latin-1 encoding
; 0x00-0xFF - trivial map
declare_encoding latin1
{
  if char < 0x100
    db char
  else
    err Failed to convert to Latin-1
  end if
}

; CP850 encoding
declare_encoding cp850
{
  if char = 0xBF
    db 0xA8
  else if char = 0xE1
    db 0xA0
  else if char = 0xE9
    db 0x82
  else if char = 0xED
    db 0xA1
  else if char = 0xF3
    db 0xA2
  else if char = 0xFA
    db 0xA3
  else
    err Failed to convert to CP850
  end if
}
