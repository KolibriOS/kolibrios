;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2013-2026. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


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
; Common for all encodings: map characters 0-0x7F trivially,
;   translate 'box drawing' pseudographics (used on boot screen).
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
    ; Encode 'Box Drawing' glyphs from Unicode into CP437/850/866
    if char = 0x2500            ; '─' U+2500 - Box Drawings Light Horizontal
      db 0xC4
    else if char = 0x2502       ; '│' U+2502 - Box Drawings Light Vertical
      db 0xB3
    else if char = 0x250C       ; '┌' U+250C - Box Drawings Light Down and Right
      db 0xDA
    else if char = 0x2510       ; '┐' U+2510 - Box Drawings Light Down and Left
      db 0xBF
    else if char = 0x2514       ; '└' U+2514 - Box Drawings Light Up and Right
      db 0xC0
    else if char = 0x2518       ; '┘' U+2518 - Box Drawings Light Up and Left
      db 0xD9
    else if char = 0x252C       ; '┬' U+252C - Box Drawings Light Down and Horizontal
      db 0xC2
    else if char = 0x2534       ; '┴' U+2534 - Box Drawings Light Up and Horizontal
      db 0xC1
    else if char = 0x2551       ; '║' U+2551 - Box Drawings Double Vertical
      db 0xBA
    ; Code points 0x20 to 0x7F are common for CP437/850/866, Latin-1 and UTF-8
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
;   Mapping is trivial, as Unicode defined the first 256 code points (0x00-0xFF)
;   to correspond to the characters and numerical values of Latin-1.
declare_encoding latin1
{
  if char < 0x100
    db char
  else
    err Failed to convert to Latin-1
  end if
}

; CP850 encoding
;   Code points 0x00 to 0x7F are common for CP437/850/866
;   Code points mapped below are common for CP437/850
declare_encoding cp850
{
  if char = 0xBF        ; '¿' U+00BF - Inverted Question Mark
    db 0xA8
  else if char = 0xE1   ; 'á' U+00E1 - Latin Small Letter A with Acute
    db 0xA0
  else if char = 0xE9   ; 'é' U+00E9 - Latin Small Letter E with Acute
    db 0x82
  else if char = 0xED   ; 'í' U+00ED - Latin Small Letter I with Acute
    db 0xA1
  else if char = 0xF3   ; 'ó' U+00F3 - Latin Small Letter O with Acute
    db 0xA2
  else if char = 0xFA   ; 'ú' U+00FA - Latin Small Letter U with Acute
    db 0xA3
  else if char = 0xF1   ; 'ñ' U+00F1 - Latin Small Letter N with Tilde
    db 0xA4
  else if char = 0xFC   ; 'ü' U+00FC - Latin Small Letter U with Diaeresis
    db 0x81
  else if char = 0xE4   ; 'ä' U+00E4 - Latin Small Letter A with Diaeresis
    db 0x84
  else
    err Failed to convert to CP850
  end if
}
