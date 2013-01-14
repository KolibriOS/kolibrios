
; Kolibri-A vectorized system fonts
; A.Jerdev <artem@jerdev.co.uk>
; Copyright (C) KolibriOS Team, 2011-12
;
; font data section


macro gptick	origin, r, tick
{    dw  (origin mod 32) shl 11 + (r mod 8) shl 8 + (tick mod 256) }

macro ritick	x, y, tick
{    dw  (x mod 16) shl 12 + (y mod 16) shl 8 + (tick and 2) shl 3 + (tick mod 2) }

macro cstick	x, y, r, tick
{    dw  (x mod 16) shl 12 + (y mod 16) shl 8 + 0xD8 + (r mod 2) shl 2 + (tick mod 4) }

macro lntick	x, y, r, len
{
if  len in <2, 3, 4, 5, 6, 7>
dw  ((x mod 16) shl 12 + (y mod 16) shl 8 + (r mod 4) shl 3 + len)
else
dw  ((x mod 16) shl 12 + (y mod 16) shl 8 + (r mod 4) shl 3 + ((len-8) mod 8) + 0xE0)
end if
}

macro char_entry	charpos, wdbits, numticks
{
	dw (charpos - .chars)*16 + (wdbits mod 4)*8 + (numticks mod 8)
}

;align 8
;nsvf_data:

.numfonts   db	2		; number of system fonts
.numsptks   db	32		; number of special ticks
.numticks   dw	?		; total number of ticks
.ticktble   dd	nsvf.tick_table ; general table

align 4
;   ---- special tickfields ----
.blank	    db	0, 0, 0, 0	   ; for straight lines
.cs2	    db	11001100b,  111100b
.cs3	    db	00010100b,  01000101b,	0001b
.cs0	    db	1111b		   ; 4-pix square
.ri1	    db	01010101b, 0101b       ; rot-invariants: 8-pix ring
.ri2	    db	01000100b, 01000100b, 01000100b, 000100b   ; 16-pix ring

align 16
nsvf_info:

;    System font #0: 5x9
.fnt0.x     db	5	    ; + 0: X-width
.fnt0.y     db	9	    ; + 1: Y-heigth
.fnt0.rs    dw	0	    ; + 2: reserved
.fnt0.tab   dd	nsvf00.table	  ; + 4
.fnt0.org   dd	nsvf00.origs	  ; + 8
.fnt0.chr   dd	  nsvf00.chars	    ; +12


;align 16
;    System font #1: 7x10
.fnt1.x     db	7	    ; X-width
.fnt1.y     db	10	    ; Y-heigth
.fnt1.rs    dw	0	    ; reserved
.fnt1.tab   dd	nsvf01.table
.fnt1.org   dd	nsvf01.origs
.fnt1.chr   dd	nsvf01.chars


align 4

diff16 "sdsh_data.tick_table: ",0,$

nsvf:
.tick_table:
	    db	0, 0, 0, 0, 0, 0	;32..37 (reserved)

.v1:                ; 38    39
        db  01b     ; XX    XX
        db  11b     ;   X    X

.v2:                    ;      40    41     42    43    44   45
        db  0100b       ;40    XXX    XXX   XX    XX    XX   XX
        db  1100b       ;41       X     X     X     XX    X    X
        db  0001b       ;42                    X          X   X
        db  1001b       ;43    
        db  0101b       ;44    
        db  1101b       ;45    
        db  0000b       ;46    
        db  0000b       ;47    
          
.v3:               ;            48   49   50    51    52    53   54   
	    db	010100b     ;48    XXX   XX   XX    XX   XXXX   XX   XXX  
	    db	000011b     ;49       X   X     X     X     X    X     X    
	    db	010001b     ;50       X   X      X    X         X      X    
	    db	000101b     ;51           X      X    X        X           
	    db	110000b     ;52
	    db	000111b     ;53     55    56    57
	    db	001100b     ;54     XX    XX    XXX
	    db	010101b     ;55       X     X    XX
	    db	011101b     ;56       X   XX
	    db	111100b     ;57      X
        db  000000b     ;58
        db  000000b     ;59
        db  000000b     ;60
        db  000000b     ;61
        db  000000b     ;62
        db  000000b     ;63
.v4:
			      ;            64   65   66    67     68    69     70
	    db	01010001b   ;64:   XX    XX  XXXX   Y      XX    XX    Y
	    db	01000101b   ;65:     X     X     X  X        X     X   X
	    db	01010000b   ;66:      X    X     X   X    XXX    XXX   X
	    db	01010010b   ;67:      X    X          X                X
	    db	00011101b   ;68:     X    X           X                 XX
	    db	00110101b   ;69:                     X
	    db	10100000b   ;70:
	    db	01010101b   ;71:   71
	    db	00000000b   ;72:   XX
	db  00000000b	;73:     X
	db  00000000b	;74:     X
	db  00000000b	;75:   XX
	    db	00000000b   ;76:
	db  00000000b	;77:
	db  00000000b	;78:
	db  00000000b	;79:


.v5:                                ;   80  81  82   83    84  85    86
	    db	00000001b, 01b      ;80:  X  X   XX   XXX    X  XXX    X
	    db	00000001b, 10b      ;81:   X  X    X     X  X      X   X
	    db	01000101b, 01b      ;82:   X  X    X     X  X      X   X
	    db	01010100b, 01b      ;83:   X  X    X   XX   X      X    X 
        db	00000010b, 01b      ;84:   X  X   X         X      X   X
	    db	00010100b, 00b      ;85:  X    X           X           X
	    db	10111000b, 00b      ;86:                               X
	    db	0, 0			;87:

.v6:                                ;     88   89    90     91
	    db	01000000b, 0101b    ;88:    X  XXX    X       X
	    db	01010100b, 0001b    ;89:    X     X   X       X
	    db	11010010b, 0001b    ;90:    X     X    X    X X
	    db	11010000b, 0001b    ;91:    X    X    X X   X X
	    db	00000000b, 0000b    ;92: X  X   X      XX    X
	    db	00000000b, 0000b    ;93:  XX
	    db	00000000b, 0000b    ;94:   
	    db	00000000b, 0000b    ;95:   

.v7:                                    ;        96  97    98    99
	    db	11011001b, 011001b	; 96: X   X  XX    XXX   XX
	    db	00010001b, 010001b	; 97: X   X    X      X  X
	    db	00010100b, 000101b	; 98:  X X      X     X  X
	    db	00000011b, 110000b	; 99:  X X      X     X  X
	    db	00000000b, 000000b	;100:   X       X  XXX   X
	    db	00000000b, 000000b	;101:          X         X
	    db	00000000b, 000000b	;102:        XX          XX
	    db	00000000b, 000000b	;103:        
.v8:                                  ;     104    105    106
	    db	00110000b, 00001100b  ;104: XXXX   XXX   XXXXX
	    db	01010100b, 01010001b  ;105:    X  X   X X     X
	    db	00000001b, 01000101b  ;106:    X  X   X       X
	    db	00000000b, 00000000b  ;107: XXXX   XXX        X
	    db	00000000b, 00000000b  ;108:                  X
	    db	00000000b, 00000000b  ;109: 
	    db	00000000b, 00000000b  ;110: 
	    db	00000000b, 00000000b  ;111: 

.v9:                                          
	    db	00100100b, 00100100b, 00b   ;112:  XXX
	    db	0, 0, 0 		    ;113:     XXXX
	    db	0, 0, 0 		    ;114:         XXX
	    db	0, 0, 0 		;115:
.v10:
	    db	00010100b, 01010000b, 0100b     ;116:
	    db	01010100b, 01010001b, 0100b     ;117:
	    db	01010001b, 00000100b, 0100b     ;118:
	    db	0, 0, 0 			;119:
.v11:
	    db	10100000b, 01010010b, 000001b	; 120: $s
	    db	01100100b, 01000000b, 000110b	; 121: )(
	    db	01000100b, 01000100b, 000000b	; 122: 0
	    db	0, 0, 0 			;123:
.v12:
	    db	10011001b, 10011001b, 10011001b ;124:
	    db	01000100b, 01000100b, 01000000b ;125:
	    db	00000000b, 00000000b, 00000000b ;126:
	    db	00000000b, 00000000b, 00000000b ;127:
.v13:
	; WARNING: 13-16 vertex fields not implemented yet!

include 'nsvf_00.asm'
include 'nsvf_01.asm'

