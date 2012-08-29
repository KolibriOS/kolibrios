
; Kolibri-A vectorized system fonts
; A.Jerdev <artem@jerdev.co.uk>
; Copyright (C) KolibriOS Team, 2011-12
;
; font data section


macro gptick	origin, r, tick
{    dw  (origin mod 32) shl 11 + (r mod 8) shl 8 + (tick mod 256) }

macro ritick	x, y, tick
{    dw  (x mod 16) shl 12 + (y mod 16) shl 8 + (tick mod 2) }

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

macro char_entry 	charpos, wdbits, numticks
{
	dw (charpos - .chars)*16 + (wdbits mod 4)*8 + (numticks mod 8)
}

;align 8
;nsvf_data:

.numfonts   db	2		; number of system fonts
.numsptks   db	32		; number of special ticks
.numticks   dw	?		; total number of ticks
.ticktble   dd	nsvf.tick_table	; general table

align 4
;   ---- special tickfields ----
.blank	    db	0, 0, 0, 0	   ; for straight lines
.cs2	    db	11001100b,  111100b
.cs3	    db	00010100b,  01000101b,	0001b
.cs0	    db	1111b		   ; 4-pix square
.ri1	    db	01010101b, 0101b   ; 8-pix ring (rot-invariant)

align 16
nsvf_info:

;    System font #0: 5x9
.fnt0.x     db	5	    ; + 0: X-width
.fnt0.y     db	9	    ; + 1: Y-heigth
.fnt0.rs    dw	0	    ; + 2: reserved
.fnt0.tab   dd	nsvf00.table      ; + 4
.fnt0.org   dd	nsvf00.origs      ; + 8
.fnt0.chr   dd    nsvf00.chars      ; +12


;align 16
;;    System font #1: 7x10
;.fnt1.x     db  7           ; X-width
;.fnt1.y     db  9           ; Y-heigth
;.fnt1.rs    dw  0           ; reserved
;.fnt1.tab   dd  .table1
;.fnt1.org   dd  .origs1


align 4

diff16 "sdsh_data.tick_table: ",0,$

nsvf:
.tick_table:
	    db	0, 0, 0, 0, 0, 0	;32..37 (reserved)
.v1:
			    ;  38   39
	    db	01b	    ; XX    XX
	    db	11b	    ;   X    X
.v2:
			    ;      40    41     42    43    44   45
	    db	0100b	    ;40    XXX    XXX   XX    XX    XX   XX
	    db	1100b	    ;41       X     X     X     XX    X    X
	    db	0001b	    ;42                    X          X   X
	    db	1001b	    ;43
	    db	0101b	    ;44
	    db	1101b	    ;45?
	    db	0	    ;46
	    db	0	    ;47
.v3:
			      ;             48   49   50    51    52    53   54   55    56
	    db	010100b     ;48    XXX   XX   XX    XX   XXXX   XX   XXX  XX    XX
	    db	000011b     ;49       X   X     X     X     X    X     X    X     X
	    db	010001b     ;50       X   X      X    X         X      X    X   XX
	    db	000101b     ;51           X      X    X        X           X
	    db	110000b     ;52
	    db	000111b     ;53
	    db	001100b     ;54
	    db	010101b     ;55
	    db	011101b     ;56
	    db	0, 0, 0		;57..59
	    db	0, 0, 0, 0		;60..63
.v4:
			      ;            64   65   66    67     68    69
	    db	01010001b   ;64:   XX    XX  XXXX   Y      XX    XX
	    db	01000101b   ;65:     X     X     X  X        X     X
	    db	01010000b   ;66:      X    X     X   X    XXX    XXX
	    db	01010010b   ;67:      X    X          X
	    db	00011101b   ;68:     X    X           X
	    db	00110101b   ;69:                     X
	    db	10100000b   ;70:
	    db	0	    ;71:
	    db	0, 0, 0, 0		;72..75
	    db	0, 0, 0, 0		;76..79


.v5:
	    db	00000001b, 01b		;80: )(
	    db	00000001b, 10b		;81: /7X
	    db	01000101b, 01b		;82: 8u
	    db	01010100b, 01b		;83:
	    db	00000010b, 01b		;84: \X&
	    db	00010100b, 00b		;85: hnu—
	    db	10111000b, 00b		;86: a
	    db	0, 0			;87:

.v6:
	    db	01000000b, 0101b	;88: Jfg
	    db	01010100b, 0001b	;89: BPR
	    db	11010010b, 0001b	;90: s$
	    db	0, 0			;91
	    db	0, 0, 0, 0		;92,93
	    db	0, 0, 0, 0		;94,95
.v7:
	    db	11011001b, 011001b	;96: ><vVY
	    db	00010001b, 010001b	;97: D
	    db	00010100b, 000101b	;98: bcdpqg
	    db	0, 0			      ;99:
	    db	0, 0, 0, 0		;100,101
	    db	0, 0, 0, 0		;102,103
.v8:
	    db	00110000b, 00001100b	;104: 5
	    db	01010100b, 01010001b	;105: 689
	    db	0, 0, 0, 0, 0, 0	;106..108
	    db	0, 0, 0, 0, 0, 0	;109..111

.v9:
	    db	0, 0, 0 		;112:
	    db	0, 0, 0 		;113:
	    db	0, 0, 0 		;114:
	    db	0, 0, 0 		;115:
.v10:
	    db	00010100b, 01010000b, 0100b	;116: @0CGOQ
	    db	01010100b, 01010001b, 0100b	;117: ---
	    db	0, 0, 0 			;118:
	    db	0, 0, 0 			;119:
.v11:
	    db	10100000b, 01010010b, 000001b	; 120: $s
	    db	0, 0, 0 			;121:
	    db	0, 0, 0 			;122:
	    db	0, 0, 0 			;123:
.v12:
	    db	0, 0, 0 		;124:
	    db	0, 0, 0 			;125:
	    db	0, 0, 0 			;126:
	    db	0, 0, 0 			;127:
.v13:
	; WARNING: 13-16 vertex fields not implemented yet!

include 'nsvf_00.asm'
include 'nsvf_01.asm'

