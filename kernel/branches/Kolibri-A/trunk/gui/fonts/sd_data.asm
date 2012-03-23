
; Kolibri-A vectorized system fonts
; A.Jerdev <artem@jerdev.co.uk>
; Copyright (C) KolibriOS Team, 2011
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

;align 8
;sdsh_data:

.numfonts   db	2		; number of system fonts
.numsptks   db	32		; number of special ticks
.numticks   dw	?		; total number of ticks
;.sp_ticks   dd  .special_ticks  ; special table
.ticktble   dd	.tick_table	; general table
;.pix4       db  34
;.pix5       db  42
;.pix6       db  50
;.pix7       db  58
;.pix8       db  66

align 4
;   ---- special tickfields ----
.blank	    db	0, 0, 0, 0	   ; for straight lines
.cs2	    db	11001100b,  111100b
.cs3	    db	00010100b,  01000101b,	0001b
.cs0	    db	1111b		   ; 4-pix square
.ri1	    db	01010101b, 0101b   ; 8-pix ring (rot-invariant)

align 16
.info:

;    System font #0: 5x9
.fnt0.x     db	5	    ; + 0: X-width
.fnt0.y     db	9	    ; + 1: Y-heigth
.fnt0.rs    dw	0	    ; + 2: reserved
.fnt0.tab   dd	.table0     ; + 4
.fnt0.org   dd	.origs0     ; + 8

;align 16
;;    System font #1: 7x10
;.fnt1.x     db  7           ; X-width
;.fnt1.y     db  9           ; Y-heigth
;.fnt1.rs    dw  0           ; reserved
;.fnt1.tab   dd  .table1
;.fnt1.org   dd  .origs1

align 4
.origs0:
	    db	0x00	; zero
	    db	0x02	; 1     s/7X
	    db	0x05	; 2     6W~§
	    db	0x06	; 3     ^*S
	    db	0x08	; 4     \
	    db	0x32	; 5     0(adqceku{
	    db	0x42	; 6
	    db	0x43	; 7
	    db	0x07	; 8     &>?23
	    db	0x18	; 9     )9D
	    db	0x44	; 10    *8B
	    db	0x03	; 11    $
	    db	0x46	; 12    vJVg
	    db	0x35	; 13    5}
	    db	0x15	; 14    4
	    db	0x45	; 15    9e
	    db	0x16	; 16    abphin›
	    db	0x13	; 17    f
	    db	0x48	; 18    Y
	    db	0x22	; 19    j&
	    db	0x36	; 20    mt
	    db	0x25	; 21    r
	    db	0x12	; 22    wæ
	    db	0x26	; 23    {
	    db	0x24	; 24    }êî
	    db	0x28	; 25
	    db	0x14	; 26    ‹ëìï
	    db	0x34	; 27    ç
	    db	0x38	; 28
	    db	0x41	; 29
	    db	0x30	; 30
	    db	0x47	; 31    J

align 4

diff16 "sdsh_data.tick_table: ",0,$

.tick_table:
	    db	0, 0, 0, 0, 0, 0        ;32..37 (reserved)
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
                              ;       48   49   50    51    52    53   54   55
	    db	010100b     ;48    XXX   XX   XX    XX   XXXX   XX   XXX  XX
	    db	000011b     ;49       X   X     X     X     X    X     X    X
	    db	010001b     ;50       X   X      X    X         X      X    X
	    db	000101b     ;51           X      X    X        X           X
	    db	110000b     ;52
	    db	000111b     ;53
	    db	001100b     ;54
	    db	010101b     ;55
	    db	0, 0, 0, 0              ;56..59
	    db	0, 0, 0, 0              ;60..63
.v4:
                              ;       64   65   66    67     68    69
	    db	01010001b   ;64:   XX    XX  XXXX   Y      XX    XX
	    db	01000101b   ;65:     X     X     X  X        X     X
	    db	01010000b   ;66:      X    X     X   X    XXX    XXX
	    db	01010010b   ;67:      X    X          X
	    db	00011101b   ;68:     X    X           X
	    db	00110101b   ;69:                     X
	    db	0           ;70:
	    db	0           ;71:
	    db	0, 0, 0, 0              ;72..75
	    db	0, 0, 0, 0              ;76..79


.v5:
	    db	00000001b, 01b		;80: )(
	    db	00000001b, 10b		;81: /7X
	    db	01000101b, 01b		;82: 8u
	    db	01010100b, 01b		;83: BPRa
	    db	00000010b, 01b		;84: \X&
	    db	00010100b, 00b		;85: ahnu—
	    db	10111000b, 00b		;86:
	    db	0, 0                    ;87:

.v6:
	    db	01000000b, 0101b        ;88: Jfg
	    db	01010100b, 0001b        ;89:
	    db	0, 0, 0, 0              ;90,91
	    db	0, 0, 0, 0              ;92,93
	    db	0, 0, 0, 0              ;94,95
.v7:
	    db	11011001b, 011001b	;96: ><vVY
	    db	00010001b, 010001b	;97: ..none found
	    db	00010100b, 000101b	;98: bcdpqg
	    db	0, 0			      ;99:
	    db	0, 0, 0, 0              ;100,101
	    db	0, 0, 0, 0              ;102,103
.v8:
	    db	00110000b, 00001100b	;104: 5
	    db	0, 0                    ;105
	    db	0, 0, 0, 0, 0, 0        ;106..108
	    db	0, 0, 0, 0, 0, 0        ;109..111

.v9:
	    db	0, 0, 0 		;112:
	    db	0, 0, 0 		;113:
	    db	0, 0, 0 		;114:
	    db	0, 0, 0 		;115:
.v10:
	    db	00010100b, 01010000b, 0100b	;116: @0CGOQ
	    db	01010100b, 01010001b, 0100b	;117: 689
	    db	0, 0, 0 			;118:
	    db	0, 0, 0 			;119:
.v11:
	    db	10100000b, 01010010b, 000001b	; 120: $s
	    db	0, 0, 0 			;121:
	    db	0, 0, 0 			;122:
	    db	0, 0, 0 			;123:
.v12:
	    db	0, 0, 0                 ;124:
	    db	0, 0, 0 			;125:
	    db	0, 0, 0 			;126:
	    db	0, 0, 0 			;127:
.v13:
	; WARNING: 13-16 vertex fields not implemented yet!


align 4
.table0:

diff16 "sdsh_data.table0: ",0,$

   times 33 dw 0
	dw (.ch0_33 -.chars)*16 + 2	    ; #33 !
	dw (.ch0_34 -.chars)*16 + 2	    ; #34 "
	dw (.ch0_35 -.chars)*16 + 4	    ; #35 #
	dw (.ch0_36 -.chars)*16 + 2	    ; #36 $
	dw (.ch0_37 -.chars)*16 + 3	    ; #37 %
	dw (.ch0_38 -.chars)*16 + 4	    ; #38 &
	dw (.ch0_39 -.chars)*16 + 1	    ; #39 '
	dw (.ch0_40 -.chars)*16 + 1	    ; #40 (
	dw (.ch0_41 -.chars)*16 + 1	    ; #41 )
	dw (.ch0_42 -.chars)*16 + 3	    ; #42 *
	dw (.ch0_43 -.chars)*16 + 2	    ; #43 +
	dw (.ch0_44 -.chars)*16 + 2	    ; #44 ,
	dw (.ch0_45 -.chars)*16 + 1	    ; #45 -
	dw (.ch0_46 -.chars)*16 + 1	    ; #46 .
	dw (.ch0_47 -.chars)*16 + 1	    ; #47 /
	dw (.ch0_48 -.chars)*16 + 2	    ; #48 0
	dw (.ch0_49 -.chars)*16 + 2	    ; #49 1
	dw (.ch0_50 -.chars)*16 + 3	    ; #50 2
	dw (.ch0_51 -.chars)*16 + 3	    ; #51 3
	dw (.ch0_52 -.chars)*16 + 3	    ; #52 4
	dw (.ch0_53 -.chars)*16 + 2	    ; #53 5
	dw (.ch0_54 -.chars)*16 + 2	    ; #54 6
	dw (.ch0_55 -.chars)*16 + 2	    ; #55 7
	dw (.ch0_56 -.chars)*16 + 2	    ; #56 8
	dw (.ch0_57 -.chars)*16 + 2	    ; #57 9
	dw (.ch0_58 -.chars)*16 + 2	    ; #58 :
	dw (.ch0_59 -.chars)*16 + 3	    ; #59 ;
	dw (.ch0_60 -.chars)*16 + 1	    ; #60 <
	dw (.ch0_61 -.chars)*16 + 2	    ; #61 =
	dw (.ch0_62 -.chars)*16 + 1	    ; #62 >
	dw (.ch0_63 -.chars)*16 + 3	    ; #63 ?
	dw (.ch0_64 -.chars)*16 + 2	    ; #64 @
	dw (.ch0_65 -.chars)*16 + 4	    ; #65 A
	dw (.ch0_66 -.chars)*16 + 3	    ; #66 B
	dw (.ch0_67 -.chars)*16 + 2	    ; #67 C
	dw (.ch0_68 -.chars)*16 + 2	    ; #68 D
	dw (.ch0_69 -.chars)*16 + 4	    ; #69 E
	dw (.ch0_70 -.chars)*16 + 3	    ; #70 F
	dw (.ch0_71 -.chars)*16 + 2	    ; #71 G
	dw (.ch0_72 -.chars)*16 + 3	    ; #72 H
	dw (.ch0_73 -.chars)*16 + 3	    ; #73 I
	dw (.ch0_74 -.chars)*16 + 2	    ; #74 J
	dw (.ch0_75 -.chars)*16 + 3	    ; #75 K
	dw (.ch0_76 -.chars)*16 + 2	    ; #76 L
	dw (.ch0_77 -.chars)*16 + 4	    ; #77 M
	dw (.ch0_78 -.chars)*16 + 3	    ; #78 N
	dw (.ch0_79 -.chars)*16 + 2	    ; #79 O
	dw (.ch0_80 -.chars)*16 + 2	    ; #80 P
	dw (.ch0_81 -.chars)*16 + 3	    ; #81 Q
	dw (.ch0_82 -.chars)*16 + 3	    ; #82 R
	dw (.ch0_83 -.chars)*16 + 3	    ; #83 S
	dw (.ch0_84 -.chars)*16 + 3	    ; #84 T
	dw (.ch0_85 -.chars)*16 + 3	    ; #85 U
	dw (.ch0_86 -.chars)*16 + 3	    ; #86 V
	dw (.ch0_87 -.chars)*16 + 3	    ; #87 W
	dw (.ch0_88 -.chars)*16 + 2	    ; #88 X
	dw (.ch0_89 -.chars)*16 + 2	    ; #88 Y
	dw (.ch0_90 -.chars)*16 + 3	    ; #90 Z
	dw (.ch0_91 -.chars)*16 + 3	    ; #91 [
	dw (.ch0_92 -.chars)*16 + 1	    ; #92 \
	dw (.ch0_93 -.chars)*16 + 3	    ; #93 ]
	dw (.ch0_94 -.chars)*16 + 1	    ; #94 ^
	dw (.ch0_95 -.chars)*16 + 1	    ; #95 _
	dw (.ch0_96 -.chars)*16 + 1	    ; #96 `
	dw (.ch0_97 -.chars)*16 + 2	    ; #97  a
	dw (.ch0_98 -.chars)*16 + 2	    ; #98  b
	dw (.ch0_99 -.chars)*16 + 3	    ; #99  c
	dw (.ch0_100-.chars)*16 + 2	    ; #100 d
	dw (.ch0_101-.chars)*16 + 2	    ; #101 e
	dw (.ch0_102-.chars)*16 + 3	    ; #102 f
	dw (.ch0_103-.chars)*16 + 2	    ; #103 g
	dw (.ch0_104-.chars)*16 + 2	    ; #104 h
	dw (.ch0_105-.chars)*16 + 3	    ; #105 i
	dw (.ch0_106-.chars)*16 + 3	    ; #106 j
	dw (.ch0_107-.chars)*16 + 2	    ; #107 k
	dw (.ch0_108-.chars)*16 + 3	    ; #108 l
	dw (.ch0_109-.chars)*16 + 4	    ; #109 m
	dw (.ch0_110-.chars)*16 + 2	    ; #110 m
	dw (.ch0_111-.chars)*16 + 1	    ; #111 o
	dw (.ch0_112-.chars)*16 + 2	    ; #112 p
	dw (.ch0_113-.chars)*16 + 2	    ; #113 q
	dw (.ch0_114-.chars)*16 + 2	    ; #114 r
	dw (.ch0_115-.chars)*16 + 1	    ; #115 s
	dw (.ch0_116-.chars)*16 + 3	    ; #116 t
	dw (.ch0_117-.chars)*16 + 2	    ; #117 u
	dw (.ch0_118-.chars)*16 + 1	    ; #118 v
	dw (.ch0_119-.chars)*16 + 4	    ; #119 w
	dw (.ch0_120-.chars)*16 + 2	    ; #120 x
	dw (.ch0_121-.chars)*16 + 2	    ; #121 y
	dw (.ch0_122-.chars)*16 + 3	    ; #122 z
	dw (.ch0_123-.chars)*16 + 2	    ; #123 {
	dw (.ch0_124-.chars)*16 + 1	    ; #124 |
	dw (.ch0_125-.chars)*16 + 2	    ; #125 }
	dw (.ch0_126-.chars)*16 + 2	    ; #126 ~
	dw (.ch0_127-.chars)*16 + 2	    ; #127

	dw (.ch0_128-.chars)*16 + 4	    ; #128
	dw (.ch0_129-.chars)*16 + 3	    ; #129
	dw (.ch0_130-.chars)*16 + 3	    ; #130
	dw (.ch0_131-.chars)*16 + 2	    ; #131
	dw (.ch0_132-.chars)*16 + 6	    ; #132
	dw (.ch0_133-.chars)*16 + 4	    ; #133
	dw (.ch0_134-.chars)*16 + 3	    ; #134
	dw (.ch0_135-.chars)*16 + 3	    ; #135
	dw (.ch0_136-.chars)*16 + 3	    ; #136
	dw (.ch0_137-.chars)*16 + 4	    ; #137
	dw (.ch0_138-.chars)*16 + 3	    ; #138
	dw (.ch0_139-.chars)*16 + 3	    ; #139
	dw (.ch0_140-.chars)*16 + 4	    ; #140
	dw (.ch0_141-.chars)*16 + 4	    ; #141
	dw (.ch0_142-.chars)*16 + 2	    ; #142
	dw (.ch0_143-.chars)*16 + 3	    ; #143
	dw (.ch0_144-.chars)*16 + 2	    ; #144
	dw (.ch0_145-.chars)*16 + 2	    ; #145
	dw (.ch0_146-.chars)*16 + 3	    ; #146
	dw (.ch0_147-.chars)*16 + 3	    ; #147
	dw (.ch0_148-.chars)*16 + 3	    ; #148
	dw (.ch0_149-.chars)*16 + 2	    ; #149
	dw (.ch0_150-.chars)*16 + 4	    ; #150
	dw (.ch0_151-.chars)*16 + 2	    ; #151
	dw (.ch0_152-.chars)*16 + 4	    ; #152
	dw (.ch0_153-.chars)*16 + 5	    ; #153
	dw (.ch0_154-.chars)*16 + 3	    ; #154
	dw (.ch0_155-.chars)*16 + 3	    ; #155
	dw (.ch0_156-.chars)*16 + 2	    ; #156
	dw (.ch0_157-.chars)*16 + 3	    ; #157
	dw (.ch0_158-.chars)*16 + 4	    ; #158
	dw (.ch0_159-.chars)*16 + 3	    ; #159
	dw (.ch0_160-.chars)*16 + 2	    ; #160
	dw (.ch0_161-.chars)*16 + 2	    ; #161
	dw (.ch0_162-.chars)*16 + 3	    ; #162
	dw (.ch0_163-.chars)*16 + 2	    ; #163
	dw (.ch0_164-.chars)*16 + 4	    ; #164
	dw (.ch0_165-.chars)*16 + 2	    ; #165
	dw (.ch0_166-.chars)*16 + 3	    ; #166
	dw (.ch0_167-.chars)*16 + 3	    ; #167
	dw (.ch0_168-.chars)*16 + 3	    ; #168
	dw (.ch0_169-.chars)*16 + 4	    ; #169
	dw (.ch0_170-.chars)*16 + 3	    ; #170
	dw (.ch0_171-.chars)*16 + 2	    ; #171
	dw (.ch0_172-.chars)*16 + 3	    ; #172
	dw (.ch0_173-.chars)*16 + 3	    ; #173
	dw (.ch0_174-.chars)*16 + 1	    ; #174
	dw (.ch0_175-.chars)*16 + 3	    ; #175
	dw (.ch0_176-.chars)*16 + 4	    ; #176
	dw (.ch0_177-.chars)*16 + 6	    ; #177
	dw (.ch0_178-.chars)*16 + 8	    ; #178
	dw (.ch0_179-.chars)*16 + 1	    ; #179
	dw (.ch0_180-.chars)*16 + 2	    ; #180
	dw (.ch0_181-.chars)*16 + 3	    ; #181
	dw (.ch0_182-.chars)*16 + 3	    ; #182
	dw (.ch0_183-.chars)*16 + 3	    ; #183
	dw (.ch0_184-.chars)*16 + 3	    ; #184
	dw (.ch0_185-.chars)*16 + 3	    ; #185
	dw (.ch0_186-.chars)*16 + 2	    ; #186
	dw (.ch0_187-.chars)*16 + 3	    ; #187
	dw (.ch0_188-.chars)*16 + 4	    ; #188
	dw (.ch0_189-.chars)*16 + 3	    ; #189
	dw (.ch0_190-.chars)*16 + 2	    ; #190
	dw (.ch0_191-.chars)*16 + 2	    ; #191
	dw (.ch0_192-.chars)*16 + 2	    ; #192
	dw (.ch0_193-.chars)*16 + 2	    ; #193
	dw (.ch0_194-.chars)*16 + 2	    ; #194
	dw (.ch0_195-.chars)*16 + 2	    ; #195
	dw (.ch0_196-.chars)*16 + 1	    ; #196
	dw (.ch0_197-.chars)*16 + 2	    ; #197
	dw (.ch0_198-.chars)*16 + 2	    ; #198
	dw (.ch0_199-.chars)*16 + 3	    ; #199
	dw (.ch0_200-.chars)*16 + 3	    ; #200
	dw (.ch0_201-.chars)*16 + 3	    ; #201
	dw (.ch0_202-.chars)*16 + 3	    ; #202
	dw (.ch0_203-.chars)*16 + 3	    ; #203
	dw (.ch0_204-.chars)*16 + 3	    ; #204
	dw (.ch0_205-.chars)*16 + 2	    ; #205
	dw (.ch0_206-.chars)*16 + 4	    ; #206
	dw (.ch0_207-.chars)*16 + 3	    ; #207
	dw (.ch0_208-.chars)*16 + 3	    ; #208
	dw (.ch0_209-.chars)*16 + 3	    ; #209
	dw (.ch0_210-.chars)*16 + 3	    ; #210
	dw (.ch0_211-.chars)*16 + 3	    ; #211
	dw (.ch0_212-.chars)*16 + 2	    ; #212
	dw (.ch0_213-.chars)*16 + 2	    ; #213
	dw (.ch0_214-.chars)*16 + 3	    ; #214
	dw (.ch0_215-.chars)*16 + 4	    ; #215
	dw (.ch0_216-.chars)*16 + 4	    ; #216
	dw (.ch0_217-.chars)*16 + 2	    ; #217
	dw (.ch0_218-.chars)*16 + 2	    ; #218
   times 5 dw 0 			    ; #219-223
	dw (.ch0_224-.chars)*16 + 2	    ; #224 p
	dw (.ch0_225-.chars)*16 + 3	    ; #225 c
	dw (.ch0_226-.chars)*16 + 2	    ; #226 â
	dw (.ch0_227-.chars)*16 + 3	    ; #227 ã
	dw (.ch0_228-.chars)*16 + 2	    ; #228 ä
	dw (.ch0_229-.chars)*16 + 2	    ; #229 å
	dw (.ch0_230-.chars)*16 + 3	    ; #230 æ
	dw (.ch0_231-.chars)*16 + 2	    ; #231 ç
	dw (.ch0_232-.chars)*16 + 4	    ; #232 è
	dw (.ch0_233-.chars)*16 + 5	    ; #233 é
	dw (.ch0_234-.chars)*16 + 2	    ; #234 ê
	dw (.ch0_235-.chars)*16 + 3	    ; #235 ë
	dw (.ch0_236-.chars)*16 + 2	    ; #236 ì
	dw (.ch0_237-.chars)*16 + 3	    ; #237 í
	dw (.ch0_238-.chars)*16 + 3	    ; #238 î
	dw (.ch0_239-.chars)*16 + 3	    ; #239 ï
	dw (.ch0_240-.chars)*16 + 5	    ; #240 ð
	dw (.ch0_241-.chars)*16 + 4	    ; #241 ñ
   times 14 dw 0			    ; #242-255
diff10 "check font0 table size: ", .table0, $

; ----------------------------------------------------
align 4

diff16 "sdsh_data.chars: ",0,$

.chars:
    dw	0
.ch0_33:    ; !
    ritick	2, 2, 0
    lntick	2, 4, 2, 5
.ch0_34:    ; "
.ch0_39:    ; '
    lntick	3, 8, 2, 2
    lntick	1, 8, 2, 2
.ch0_36:    ; $
    lntick	2, 1, 2, 7
    gptick     11, 0, 120
.ch0_37:    ; %
    cstick	0, 8, 0, 0
    lntick	0, 3, 1, 5
    cstick	3, 3, 0, 0
.ch0_38:    ; &
    gptick	7, 6, 84
    gptick     19, 4, 44
.ch0_40:    ; (
    gptick	5, 3, 80
.ch0_41:    ; )
    gptick	9, 7, 80
.ch0_42:    ; *
    gptick	3, 7, 39
    gptick     10, 3, 39
    lntick	2, 3, 2, 5
.ch0_43:    ; +
    lntick	2, 2, 2, 5
.ch0_45:    ; -
.ch0_35:    ; #
.ch0_61:    ; =
    lntick	0, 4, 0, 5
    lntick	0, 6, 0, 5
    lntick	1, 3, 2, 5
    lntick	3, 3, 2, 5
.ch0_58:    ; :
.ch0_59:    ; ;
    cstick	1, 7, 0, 0
.ch0_46:    ; .
.ch0_44:    ; ,
    cstick	1, 3, 0, 0
    ritick	1, 1, 0
.ch0_47:    ; /
.ch0_55:    ; 7
    gptick	1, 2, 81
    lntick	0, 8, 0, 4
.ch0_64:    ; @
    cstick	3, 6, 0, 1
.ch0_48:    ; 0
.ch0_79:    ; O
.ch0_142:   ;
.ch0_81:    ; Q
    gptick	5, 4, 116
    lntick	4, 3, 2, 5
    lntick	3, 1, 0, 2
.ch0_49:    ; 1
.ch0_124:   ; |
    lntick	2, 2, 2, 7
    ritick	1, 6, 0
.ch0_50:    ; 2
    lntick	0, 2, 0, 5
    lntick	1, 3, 1, 3
.ch0_51:    ; 3
    gptick	8, 1, 64
.ch0_83:    ; S
    lntick	1, 5, 0, 3
    gptick	8, 1, 55
.ch0_53:    ; 5
    gptick     10, 6, 65
    gptick     13, 4, 104
.ch0_52:    ; 4
    gptick     14, 2, 38
    lntick	0, 4, 0, 5
    lntick	3, 2, 2, 7
.ch0_54:    ; 6
    gptick     14, 0, 117
    gptick	2, 2, 50
.ch0_56:    ; 8
    gptick     10, 6, 82
.ch0_57:    ; 9
    gptick	9, 0, 117
    gptick     15, 6, 50
.ch0_60:    ; <
    gptick	7, 4, 96
.ch0_62:    ; >
    gptick	8, 0, 96
.ch0_63:    ; ?
    gptick	7, 1, 64

.ch0_66:    ; B
.ch0_130:   ; B
    gptick     10, 6, 51
.ch0_80:    ; P
.ch0_144:   ;
.ch0_82:    ; R
    gptick	9, 0, 83
.ch0_75:    ; K
.ch0_138:   ; K
    lntick	0, 2, 2, 7
    lntick	4, 2, 3, 3
    lntick	1, 5, 1, 4
.ch0_67:    ; C
.ch0_145:   ;
    ritick	5, 3, 0
.ch0_71:    ; G
    gptick	5, 4, 116
    gptick	4, 5, 49
.ch0_68:    ; D
    gptick	9, 0, 96
.ch0_76:    ; L
    lntick	0, 2, 2, 7
    lntick	1, 2, 0, 4
.ch0_240:   ; ð
    ritick	1, 9, 0
    ritick	3, 9, 0
.ch0_133:   ; E
.ch0_69:    ; E
    lntick	1, 2, 0, 4
.ch0_70:    ; F
    lntick	1, 8, 0, 4
.ch0_72:    ; H
.ch0_141:   ; H
    lntick	1, 5, 0, 3
    lntick	0, 2, 2, 7
    lntick	5, 5, 2, 7
.ch0_73:    ; I
    lntick	1, 8, 0, 3
    lntick	2, 3, 2, 5
    lntick	1, 2, 0, 3
.ch0_74:    ; J
    gptick     31, 6, 88
    lntick	3, 8, 0, 3
.ch0_84:    ; T
.ch0_146:   ; ’
    lntick	3, 3, 2, 5
    lntick	2, 2, 0, 3
    lntick	1, 8, 0, 5
.ch0_77:    ; M
.ch0_140:   ; M
    gptick	21, 2, 38
    ritick	1, 7, 0
.ch0_78:    ; N
    lntick	0, 2, 2, 7
    lntick	4, 2, 2, 7
    lntick	3, 4, 3, 3
.ch0_85:    ; U
    lntick	1, 2, 0, 3
.ch0_87:    ; W
    lntick	0, 3, 2, 6
    lntick	4, 3, 2, 6
    gptick	1, 1, 54
.ch0_86:    ; V
    lntick	0, 7, 2, 2
    lntick	4, 7, 2, 2
.ch0_118:   ; v
.ch0_121:   ; y
    gptick     12, 6, 96
    lntick	2, 0, 2, 2
.ch0_88:    ; X
.ch0_149:   ;
    gptick	1, 2, 81
    gptick	4, 6, 84
.ch0_89:    ; Y
    gptick     18, 6, 96
    lntick	2, 2, 2, 2
.ch0_90:    ; Z
    lntick	0, 2, 0, 5
    lntick	0, 8, 0, 5
    lntick	0, 3, 1, 5
.ch0_92:    ; \
    gptick	4, 6, 84
.ch0_91:    ; [
    lntick	1, 3, 2, 5
.ch0_93:    ; ]
    lntick	1, 2, 0, 3
    lntick	1, 8, 0, 3
    lntick	3, 3, 2, 5
.ch0_65:    ; A
.ch0_128:   ;
    lntick	0, 2, 2, 4
    lntick	4, 2, 2, 4
    lntick	1, 4, 0, 3
.ch0_94:    ; ^
    gptick	3, 1, 54
.ch0_95:    ; _
    lntick	0, 1, 0, 5
.ch0_96:    ; `
    lntick	3, 8, 3, 2
.ch0_97:    ; a
.ch0_160:   ;
    gptick	5, 4, 83
    gptick     16, 0, 85
.ch0_129:   ;
    lntick	1, 8, 0, 3
.ch0_98:    ; b
    lntick	0, 2, 2, 7
.ch0_112:   ; p
.ch0_224:   ; p
    gptick     16, 0, 98
    lntick	0, 0, 2, 7
.ch0_99:    ; c
.ch0_225:   ; c
    ritick	4, 5, 0
    ritick	4, 2, 0
.ch0_101:   ; e
.ch0_165:   ;
.ch0_241:   ; ñ
    gptick	5, 4, 98
    gptick     15, 6, 49
    ritick	1, 8, 0
    ritick	3, 8, 0
.ch0_100:   ; d
    lntick	4, 2, 2, 7
.ch0_113:   ; q
    gptick	5, 4, 98
    lntick	4, 0, 2, 7
.ch0_102:   ; f
    gptick     17, 2, 88
    ritick	1, 2, 0
    lntick	0, 6, 0, 3
.ch0_103:   ; g
    gptick     15, 6, 88
    gptick	5, 4, 98
.ch0_104:   ; h
    gptick     16, 0, 85
.ch0_107:   ; k
    lntick	0, 2, 2, 7
    gptick	5, 3, 54
.ch0_105:   ; i
    lntick	1, 2, 0, 3
.ch0_106:   ; j
    gptick     16, 0, 49
    ritick	2, 8, 0
    gptick     19, 5, 38
.ch0_108:   ; l
    lntick	1, 2, 0, 3
    lntick	2, 3, 2, 6
    ritick	1, 8, 0
.ch0_109:   ; m
    lntick	2, 3, 2, 2
    gptick     20, 5, 39
    lntick	4, 2, 2, 4
.ch0_110:   ; n
    lntick	0, 2, 2, 5
    gptick     16, 0, 85
.ch0_114:   ; r
    lntick	1, 2, 2, 5
    gptick     21, 1, 38
.ch0_115:   ; s
    gptick	1, 0, 120
.ch0_116:   ; t
    gptick     16, 0, 49
    gptick     20, 3, 38
    lntick	3, 2, 0, 2
.ch0_117:   ; u
.ch0_227:   ; ã
    lntick	4, 2, 2, 5
    gptick	5, 4, 85
    gptick     29, 5, 42
.ch0_119:   ; w
    lntick	2, 4, 2, 2
    gptick     22, 1, 39
    lntick	0, 3, 2, 4
    lntick	4, 3, 2, 4
.ch0_120:   ; x
    lntick	0, 2, 1, 5
    lntick	4, 2, 3, 5
.ch0_122:   ; z
    lntick	0, 2, 0, 5
    lntick	0, 6, 0, 5
    lntick	1, 2, 3, 3
.ch0_123:   ; {
    gptick	5, 3, 43
    gptick     23, 2, 38
.ch0_125:   ; }
    gptick     13, 3, 43
    gptick     24, 6, 38
.ch0_126:   ; ~
    gptick	2, 1, 39
    lntick	3, 3, 1, 2
.ch0_127:   ;
    lntick	2, 2, 2, 3
    lntick	2, 6, 2, 3
.ch0_131:   ;
    lntick	1, 8, 0, 4
.ch0_132:   ;
    lntick	1, 3, 2, 5
    lntick	1, 8, 0, 4
    lntick	0, 1, 2, 2
.ch0_150:   ;
    lntick	0, 2, 0, 5
    lntick	3, 3, 2, 6
    ritick	4, 1, 0
    lntick	0, 3, 2, 6
.ch0_134:   ;
    gptick	4, 6, 86
    gptick	6, 2, 86
.ch0_148:   ;
    lntick	2, 2, 2, 7
    cstick	1, 8, 0, 3
.ch0_137:   ;
    gptick     25, 2, 39
.ch0_136:   ;
    lntick	0, 2, 2, 7
    lntick	1, 4, 1, 3
.ch0_139:   ;
    lntick	4, 2, 2, 7
    gptick     26, 2, 66
    lntick	0, 2, 1, 2
.ch0_143:   ;
    lntick	0, 2, 2, 7
    lntick	1, 8, 0, 3
.ch0_151:   ;
    lntick	4, 2, 2, 7
.ch0_147:   ;
    gptick     27, 4, 85
    lntick	4, 3, 2, 6
    lntick	1, 2, 0, 3
.ch0_155:   ;
    lntick     16, 0, 82
.ch0_152:   ;
.ch0_153:   ;
    lntick	0, 3, 2, 6
    lntick	4, 2, 2, 7
    lntick	2, 3, 2, 6
    lntick	0, 2, 0, 5
    lntick	4, 1, 2, 2
.ch0_154:   ;
.ch0_156:   ;
    lntick     26, 0, 82
    lntick	1, 3, 2, 6
    ritick	0, 8, 0
.ch0_135:   ;
    ritick	3, 5, 0
.ch0_157:   ;
    gptick	8, 1, 64
    gptick     10, 6, 65
    lntick	1, 5, 0, 4
.ch0_158:   ;
    gptick     28, 7, 80
    lntick	2, 3, 2, 5
    lntick	0, 2, 2, 7
    ritick	1, 5, 0
.ch0_159:   ;
    gptick     13, 4, 89
    lntick	4, 2, 2, 7
    lntick	0, 2, 1, 3
.ch0_161:   ;
    gptick	8, 1, 42
.ch0_162:   ;
.ch0_111:   ; o
.ch0_174:   ;
    cstick	1, 6, 0, 3
    gptick	3, 2, 44
    ritick	2, 7, 0
.ch0_163:   ;
    lntick	1, 6, 0, 4
.ch0_164:   ;
    lntick	1, 2, 2, 4
    gptick     23, 0, 49
    gptick     19, 0, 41
    lntick	0, 1, 2, 2
.ch0_166:   ;
    gptick     12, 6, 82
    gptick	1, 2, 82
    lntick	2, 2, 2, 5
.ch0_167:   ;
    lntick	2, 4, 0, 2
.ch0_237:   ;
    gptick	2, 1, 50
    gptick	7, 5, 50
    lntick	2, 4, 0, 3
.ch0_169:   ;
    gptick     28, 5, 39
.ch0_168:   ;
    lntick	4, 2, 2, 5
    lntick	1, 3, 1, 3
.ch0_170:   ;
    lntick	1, 2, 2, 5
    gptick	6, 3, 54
    ritick	1, 5, 0
.ch0_171:   ;
    gptick	1, 1, 67
.ch0_172:   ;
    lntick	4, 2, 2, 5
    gptick     13, 5, 39
.ch0_173:   ;
    lntick	0, 2, 2, 5
    lntick	1, 4, 0, 3
.ch0_175:   ;
    lntick	4, 2, 2, 5
    lntick	0, 2, 2, 4
.ch0_226:   ;
    lntick	0, 6, 0, 5
    lntick	2, 2, 2, 4
.ch0_228:   ; ä
    cstick	1, 6, 0, 3
    lntick	2, 0, 2, 6
.ch0_229:   ; å
    lntick	0, 2, 1, 5
    lntick	4, 2, 3, 5
.ch0_230:   ; æ
    lntick	0, 2, 2, 5
    lntick	3, 3, 2, 4
    gptick     22, 0, 52
.ch0_231:   ; ç
    gptick     27, 4, 48
.ch0_232:   ; è
.ch0_233:   ; é
    lntick	4, 2, 2, 5
    lntick	0, 3, 2, 4
    lntick	2, 3, 2, 4
    lntick	0, 2, 0, 4
    lntick	5, 1, 2, 2
.ch0_234:   ; ê
    gptick     24, 0, 68
    gptick	6, 0, 49
.ch0_235:   ; ë
.ch0_236:   ; ì
    lntick	0, 3, 2, 4
    gptick     26, 0, 68
    lntick	4, 2, 2, 5
.ch0_238:   ; î
    gptick     24, 2, 68
    lntick	3, 2, 3, 3
    lntick	0, 2, 2, 5
.ch0_239:   ; ï
    gptick     26, 3, 53
    lntick	4, 2, 2, 5
    gptick	1, 1, 40

.ch0_178:   ; pseudo-graphics
    ritick	0, 8, 0
    lntick	0, 5, 1, 4
    lntick	0, 2, 1, 5
    lntick	1, 0, 1, 4
.ch0_176:   ;
    lntick	0, 4, 1, 5
    lntick	2, 0, 1, 3
.ch0_177:   ;
    lntick	0, 7, 1, 2
    lntick	0, 1, 1, 5
    lntick	0, 3, 1, 4
    lntick	0, 5, 1, 4
    lntick	1, 0, 1, 4
    lntick	3, 0, 1, 2
.ch0_184:   ;
    lntick	3, 0, 2, 6
.ch0_181:   ;
    lntick	0, 3, 0, 2
    lntick	0, 5, 0, 2
.ch0_179:   ;
.ch0_180:   ;
    lntick	2, 0, 2, 9
    lntick	0, 4, 0, 2
.ch0_183:   ;
    lntick	0, 4, 0, 4
    lntick	1, 0, 2, 4
    lntick	3, 0, 2, 4
.ch0_185:   ;
    gptick     11, 0, 49
    gptick	9, 6, 52
.ch0_182:   ;
.ch0_215:   ;
    ritick	0, 4, 0
.ch0_186:   ;
.ch0_199:   ;
    lntick	1, 0, 2, 9
    lntick	3, 0, 2, 9
    ritick	4, 4, 0
.ch0_204:   ;
    gptick     15, 4, 49
    gptick     30, 2, 52
.ch0_187:   ;
    lntick	4, 0, 2, 5
    lntick	0, 5, 0, 4
    gptick     11, 0, 49
.ch0_188:   ;
    ritick	0, 5, 0
    lntick	0, 3, 0, 4
.ch0_189:   ;
    lntick	4, 4, 2, 5
    lntick	1, 5, 2, 4
    lntick	0, 4, 0, 3
.ch0_190:   ;
    gptick	2, 0, 69
.ch0_192:   ;
    lntick	2, 5, 2, 4
    lntick	2, 4, 0, 3
.ch0_191:   ;
    lntick	0, 4, 0, 3
.ch0_194:   ;
    lntick	2, 0, 2, 4
.ch0_193:   ;
.ch0_196:   ;
    lntick	0, 4, 0, 5
    lntick	2, 5, 2, 4
.ch0_195:   ;
    lntick	0, 3, 0, 2
.ch0_197:   ;
    lntick	2, 0, 2, 9
    lntick	0, 4, 0, 5
.ch0_198:   ;
    lntick	2, 0, 2, 9
.ch0_213:   ;
    gptick	7, 4, 69
    lntick	2, 0, 2, 4
.ch0_200:   ;
    gptick     15, 4, 49
    lntick	1, 3, 0, 4
    lntick	1, 4, 2, 5
.ch0_201:   ;
    gptick     30, 2, 52
    lntick	1, 5, 0, 4
    lntick	1, 0, 2, 5
.ch0_207:   ;
.ch0_216:   ;
    lntick	2, 6, 2, 3
.ch0_205:   ;
.ch0_209:   ;
    lntick	0, 3, 0, 5
    lntick	0, 5, 0, 5
    lntick	2, 0, 2, 3
.ch0_202:   ;
    lntick	0, 3, 0, 5
.ch0_206:   ;
    gptick	9, 6, 52
    gptick     15, 4, 49
.ch0_203:   ;
    gptick     30, 2, 52
    gptick     11, 0, 49
    lntick	0, 5, 0, 5
.ch0_211:   ;
    lntick	1, 4, 0, 1
.ch0_208:   ;
    lntick	1, 5, 2, 4
    lntick	3, 5, 2, 4
.ch0_210:   ;
    lntick	0, 4, 0, 5
.ch0_214:   ;
    lntick	1, 0, 2, 4
    lntick	3, 0, 2, 4
    lntick	1, 4, 0, 1
.ch0_212:   ;
    gptick	7, 4, 69
    lntick	2, 6, 2, 3
.ch0_217:   ;
    lntick	0, 4, 0, 3
    lntick	2, 5, 2, 4
.ch0_218:   ;
    lntick	2, 4, 0, 3
    lntick	2, 0, 2, 4

diff10 "font0 size ", .chars, $


