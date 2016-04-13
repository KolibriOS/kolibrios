; Test new device
.device ATmega328P

; Test number sign labels 
#define TEST
.define DOTTEST


; Test whitespace between function name and value
ldi r16, high(0)
ldi r17, high (0)

;---
; Test data segment start with a number sign instead of a dot
;---
#DSEG
Buffer: .BYTE 8  ; Reserve 64 bits

;---
; EEPROM segment
;---
.ESEG

; Test line continuation
AVERAGE: .db 0xF0, 0xFF, \
             0x55, 0xFF, \
             0x55, 0x0F
