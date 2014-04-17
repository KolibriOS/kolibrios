;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2013-2014. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


; fetch the UTF-8 character in string+offs to char
; common part for all encodings: translate pseudographics
; Pseudographics for the boot screen:
; 0x2500 -> 0xC4, 0x2502 -> 0xB3, 0x250C -> 0xDA, 0x2510 -> 0xBF,
; 0x2514 -> 0xC0, 0x2518 -> 0xD9, 0x252C -> 0xC2, 0x2534 -> 0xC1, 0x2551 -> 0xBA
macro fetch_utf8_char string, offs, char, graph
{ local first_byte, b
  virtual at 0
    db string
    if offs >= $
      char = -1
    else
      ; fetch first byte
      load first_byte byte from offs
      if first_byte < 0x80
        char = first_byte
        offs = offs + 1
      else if first_byte < 0xC0
        .err Invalid UTF-8 string
      else if first_byte < 0xE0
        char = first_byte and 0x1F
        load b byte from offs + 1
        char = (char shl 6) + (b and 0x3F)
        offs = offs + 2
      else if first_byte < 0xF0
        char = first_byte and 0xF
        load b byte from offs + 1
        char = (char shl 6) + (b and 0x3F)
        load b byte from offs + 2
        char = (char shl 6) + (b and 0x3F)
        offs = offs + 3
      else if first_byte < 0xF8
        char = first_byte and 0x7
        load b byte from offs + 1
        char = (char shl 6) + (b and 0x3F)
        load b byte from offs + 2
        char = (char shl 6) + (b and 0x3F)
        load b byte from offs + 3
        char = (char shl 6) + (b and 0x3F)
        offs = offs + 4
      else
        .err Invalid UTF-8 string
      end if
    end if
  end virtual
  if char = 0x2500
    graph = 0xC4
  else if char = 0x2502
    graph = 0xB3
  else if char = 0x250C
    graph = 0xDA
  else if char = 0x2510
    graph = 0xBF
  else if char = 0x2514
    graph = 0xC0
  else if char = 0x2518
    graph = 0xD9
  else if char = 0x252C
    graph = 0xC2
  else if char = 0x2534
    graph = 0xC1
  else if char = 0x2551
    graph = 0xBA
  else
    graph = 0
  end if
}

; Russian: use CP866.
; 0x00-0x7F - trivial map
; 0x410-0x43F -> 0x80-0xAF
; 0x440-0x44F -> 0xE0-0xEF
; 0x401 -> 0xF0, 0x451 -> 0xF1
macro cp866 [arg]
{ local offs, char, graph
  offs = 0
  while 1
    fetch_utf8_char arg, offs, char, graph
    if char = -1
      break
    end if
    if graph
      db graph
    else if char < 0x80
      db char
    else if char = 0x401
      db 0xF0
    else if char = 0x451
      db 0xF1
    else if (char < 0x410) | (char > 0x44F)
      .err Failed to convert to CP866
    else if char < 0x440
      db char - 0x410 + 0x80
    else
      db char - 0x440 + 0xE0
    end if
  end while
}

struc cp866 [arg]
{
common
  cp866 arg
}

; Latin-1 encoding
; 0x00-0xFF - trivial map
macro latin1 [arg]
{ local offs, char, graph
  offs = 0
  while 1
    fetch_utf8_char arg, offs, char, graph
    if char = -1
      break
    end if
    if graph
      db graph
    else if char < 0x100
      db char
    else
      .err Failed to convert to Latin-1
    end if
  end while
}

struc latin1 [arg]
{
common
  latin1 arg
}

; CP850 encoding
macro cp850 [arg]
{ local offs, char, graph
  offs = 0
  while 1
    fetch_utf8_char arg, offs, char, graph
    if char = -1
      break
    end if
    if graph
      db graph
    else if char < 0x80
      db char
    else if char = 0xBF
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
  end while
}

struc cp850 [arg]
{
common
  cp850 arg
}
