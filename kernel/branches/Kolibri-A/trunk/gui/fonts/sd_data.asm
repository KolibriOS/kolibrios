
; Kolibri-A vectorized system fonts
; A.Jerdev <artem@jerdev.co.uk>
; Copyright (C) KolibriOS Team, 2011
; 
; font data section


macro gptick	origin, r, tick
{    dw  (origin mod 64) shl 10 + (r mod 8) shl 7 + (tick mod 128) }

macro ritick	x, y, tick
{    dw  (x mod 16) shl 12 + (y mod 32) shl 7 + (tick mod 4) shl 4 }

macro cstick	x, y, r, tick
{    dw  (x mod 16) shl 12 + (y mod 32) shl 7 + (tick mod 4) shl 4 + 2 + (r mod 2)}

macro mstick	x, y, r, tick
{    dw  (x mod 16) shl 12 + (y mod 32) shl 7 + (rmod 4) shl 5 + (tick mod 32) }

macro lntick	x, y, r, len
{    dw  (x mod 16) shl 12 + (y mod 32) shl 7 + (rmod 4) shl 5 + (len mod 16) }

;align 8
sdsh_data:

.numfonts   db  2               ; number of system fonts
.numsptks   db  32              ; number of special ticks
.numticks   dw  ?               ; total number of ticks
.sp_ticks   dd  .special_ticks  ; special table
.ticktble   dd  .tick_table     ; general table
.origtble   dd  .origs          ; origins table
.chartble   dd  .chars          ; characters table
.pix4       db  34
.pix5       db  40
.pix6       db  50
.pix7       db  60
.pix8       db  70


align 16
.info:

;    System font #0: 5x9  
.fnt0.x     db  5       ; X-width
.fnt0.y     db  9       ; Y-heigth
.fnt0.rs    dw  0       ; reserved
.fnt0.tab   dd  .table0
.fnt0.org   dd  .origs0

align 16
;    System font #1: 7x10  
.fnt1.x     db  7       ; X-width
.fnt1.y     db  9       ; Y-heigth
.fnt1.rs    dw  0       ; reserved
.fnt1.tab   dd  .table1
.fnt1.org   dd  .origs1

align 4
.origs0:
            db  0x00    ; zero
            db  0x02    ; 1     s/7Xæëÿ
            db  0x05    ; 2     6W~ç
            db  0x06    ; 3     ^*Sâú
            db  0x08    ; 4     \ÆÚ
            db  0x32    ; 5     0(adqceku{
            db  0x42    ; 6     Æê
            db  0x43    ; 7     <ç
            db  0x07    ; 8     &>?23á
            db  0x18    ; 9     )9D
            db  0x44    ; 10    *8B
            db  0x03    ; 11    $
            db  0x46    ; 12    vJVgæ
            db  0x35    ; 13    5}ßì
            db  0x15    ; 14    4
            db  0x45    ; 15    9e
            db  0x16    ; 16    abphinÛ
            db  0x13    ; 17    f
            db  0x48    ; 18    Y
            db  0x22    ; 19    j&ä
            db  0x36    ; 20    mt
            db  0x25    ; 21    r
            db  0x12    ; 22    wö
            db  0x26    ; 23    {Ú
            db  0x24    ; 24    }úþ
            db  0x28    ; 25    É
            db  0x14    ; 26    Ëûüÿ
            db  0x34    ; 27    ×Ó÷
            db  0x38    ; 28    Þé
            db  0x41    ; 29    ó

align 4
.tick_table:
.v1:
                            ;  32   33
            db  01b         ; XX    XX   
            db  11b         ;   X    X  
.v2:
                            ;  34    35     36    37    38
            db  0100b       ; XXX    XXX   XX    XX    XX
            db  1100b       ;    X     X     X     XX    X
            db  0001b       ;                 X          X
            db  1001b       ;
            db  0101b       ;
            db  ?
.v3:
                            ;  40    41     42     43    44   45    46     47
            db  011001b     ; XX     XXX   XXX     XX   XX    XX   XXXX    XX
            db  001100b     ;   XX     X      X     X     X     X     X     X
            db  000100b     ;     X    X      X     X      X    X          X
            db  000011b     ;                       X      X    X         X
            db  010001b     ;
            db  000101b     ;
            db  110000b     ;
            db  000111b     ;

.v4:
            db  01010001b                       ; f0: ?23S
            db  01000101b                       ; f0: 35
            db  01010000b                       ; f0: Ë
            db  01010010b                       ; f0: ë
            db  00011101b                       ; f0: úûü
.v5:
            db  00000001b, 01b                  ;1f0: )(Þ
            db  00000001b, 10b                  ; f0: /7X
            db  01000101b, 01b                  ;3f0: 8uÚÛæ
            db  01010100b, 00b                  ; f0: BPRa
            db  00000010b, 01b                  ;5f0: \X&
            db  00010100b, 00b                  ; f0: ahnu×
            db  10111000b, 00b                  ;7f0: Æ

.v6:
            db  01000000b, 0101b                ; f0: Jfg
            db  01010100b, 0001b                ; f0: ß
.v7:
            db  11011001b, 011001b              ; f0: ><vVY
            db  00010001b, 010001b              ; f0: D
            db  00010100b, 000101b              ; f0: bcdpqg
.v8:
            db  00110000b, 00001100b            ; f0: 5
.v10:
            db  00010100b, 01010000b, 0100b     ; f0: @0CGOQÑÎÝ
            db  01010100b, 01010001b, 0100b     ; f0: 689
.v11:
            db  10100000b, 01010010b, 000001b   ; f0: $s


align 4
.table0:
   times 33 dw 0
        dw (.ch0_33 -.chars)*16 + 2         ; #33 !
        dw (.ch0_34 -.chars)*16 + 2         ; #34 "		
        dw (.ch0_35 -.chars)*16 + 4         ; #35 # 
        dw (.ch0_36 -.chars)*16 + 2         ; #36 $ 
        dw (.ch0_37 -.chars)*16 + 3         ; #37 % 
        dw (.ch0_38 -.chars)*16 + 4         ; #38 &  
        dw (.ch0_39 -.chars)*16 + 1         ; #39 '
        dw (.ch0_40 -.chars)*16 + 1         ; #40 ( 	     
        dw (.ch0_41 -.chars)*16 + 1         ; #41 ) 
        dw (.ch0_42 -.chars)*16 + 3         ; #42 * 
        dw (.ch0_43 -.chars)*16 + 2         ; #43 + 
        dw (.ch0_44 -.chars)*16 + 2         ; #44 ,	
        dw (.ch0_45 -.chars)*16 + 1         ; #45 -	
        dw (.ch0_46 -.chars)*16 + 1         ; #46 .			
        dw (.ch0_47 -.chars)*16 + 1         ; #47 /		
        dw (.ch0_48 -.chars)*16 + 2         ; #48 0
        dw (.ch0_49 -.chars)*16 + 2         ; #49 1
        dw (.ch0_50 -.chars)*16 + 3         ; #50 2
        dw (.ch0_51 -.chars)*16 + 3         ; #51 3
        dw (.ch0_52 -.chars)*16 + 3         ; #52 4
        dw (.ch0_53 -.chars)*16 + 2         ; #53 5
        dw (.ch0_54 -.chars)*16 + 2         ; #54 6
        dw (.ch0_55 -.chars)*16 + 2         ; #55 7
        dw (.ch0_56 -.chars)*16 + 2         ; #56 8
        dw (.ch0_57 -.chars)*16 + 2         ; #57 9
        dw (.ch0_58 -.chars)*16 + 2         ; #58 :
        dw (.ch0_59 -.chars)*16 + 3         ; #59 ;
        dw (.ch0_60 -.chars)*16 + 1         ; #60 <
        dw (.ch0_61 -.chars)*16 + 2         ; #61 =
        dw (.ch0_62 -.chars)*16 + 1         ; #62 >
        dw (.ch0_63 -.chars)*16 + 3         ; #63 ?
        dw (.ch0_64 -.chars)*16 + 2         ; #64 @
        dw (.ch0_65 -.chars)*16 + 4         ; #65 A
        dw (.ch0_66 -.chars)*16 + 3         ; #66 B
        dw (.ch0_67 -.chars)*16 + 2         ; #67 C
        dw (.ch0_68 -.chars)*16 + 2         ; #68 D
        dw (.ch0_69 -.chars)*16 + 4         ; #69 E
        dw (.ch0_70 -.chars)*16 + 3         ; #70 F
        dw (.ch0_71 -.chars)*16 + 2         ; #71 G
        dw (.ch0_72 -.chars)*16 + 3         ; #72 H
        dw (.ch0_73 -.chars)*16 + 3         ; #73 I
        dw (.ch0_74 -.chars)*16 + 2         ; #74 J
        dw (.ch0_75 -.chars)*16 + 3         ; #75 K
        dw (.ch0_76 -.chars)*16 + 2         ; #76 L
        dw (.ch0_78 -.chars)*16 + 3         ; #78 N
        dw (.ch0_79 -.chars)*16 + 2         ; #79 O
        dw (.ch0_80 -.chars)*16 + 2         ; #80 P
        dw (.ch0_81 -.chars)*16 + 3         ; #81 Q
        dw (.ch0_82 -.chars)*16 + 3         ; #82 R
        dw (.ch0_83 -.chars)*16 + 3         ; #83 S
        dw (.ch0_84 -.chars)*16 + 3         ; #84 T
        dw (.ch0_85 -.chars)*16 + 3         ; #85 U
        dw (.ch0_86 -.chars)*16 + 3         ; #86 V
        dw (.ch0_87 -.chars)*16 + 3         ; #87 W
        dw (.ch0_88 -.chars)*16 + 2         ; #88 X
        dw (.ch0_89 -.chars)*16 + 2         ; #88 Y
        dw (.ch0_90 -.chars)*16 + 3         ; #90 Z
        dw (.ch0_91 -.chars)*16 + 3         ; #91 [
        dw (.ch0_92 -.chars)*16 + 1         ; #92 \
        dw (.ch0_93 -.chars)*16 + 3         ; #93 ]
        dw (.ch0_94 -.chars)*16 + 1         ; #94 ^
        dw (.ch0_95 -.chars)*16 + 1         ; #95 _
        dw (.ch0_96 -.chars)*16 + 1         ; #96 `
        dw (.ch0_97 -.chars)*16 + 2         ; #97  a
        dw (.ch0_98 -.chars)*16 + 2         ; #98  b
        dw (.ch0_99 -.chars)*16 + 3         ; #99  c
        dw (.ch0_100-.chars)*16 + 2         ; #100 d
        dw (.ch0_101-.chars)*16 + 2         ; #101 e
        dw (.ch0_102-.chars)*16 + 3         ; #102 f
        dw (.ch0_103-.chars)*16 + 2         ; #103 g
        dw (.ch0_104-.chars)*16 + 2         ; #104 h
        dw (.ch0_105-.chars)*16 + 3         ; #105 i
        dw (.ch0_106-.chars)*16 + 3         ; #106 j
        dw (.ch0_107-.chars)*16 + 2         ; #107 k
        dw (.ch0_108-.chars)*16 + 3         ; #108 l
        dw (.ch0_109-.chars)*16 + 4         ; #109 m
        dw (.ch0_110-.chars)*16 + 2         ; #110 m
        dw (.ch0_111-.chars)*16 + 1         ; #111 o
        dw (.ch0_112-.chars)*16 + 2         ; #112 p
        dw (.ch0_113-.chars)*16 + 2         ; #113 q
        dw (.ch0_113-.chars)*16 + 2         ; #113 r
        dw (.ch0_115-.chars)*16 + 1         ; #115 s
        dw (.ch0_116-.chars)*16 + 3         ; #116 t
        dw (.ch0_117-.chars)*16 + 2         ; #117 u
        dw (.ch0_118-.chars)*16 + 1         ; #118 v
        dw (.ch0_119-.chars)*16 + 4         ; #119 w
        dw (.ch0_120-.chars)*16 + 2         ; #120 x
        dw (.ch0_121-.chars)*16 + 2         ; #121 y
        dw (.ch0_122-.chars)*16 + 3         ; #122 z
        dw (.ch0_123-.chars)*16 + 2         ; #123 {
        dw (.ch0_124-.chars)*16 + 1         ; #124 |
        dw (.ch0_125-.chars)*16 + 2         ; #125 }
        dw (.ch0_126-.chars)*16 + 2         ; #126 ~
        dw (.ch0_127-.chars)*16 + 2         ; #127 ¦

        dw (.ch0_128-.chars)*16 + 4         ; #128 A
        dw (.ch0_129-.chars)*16 + 3         ; #129 Á
        dw (.ch0_130-.chars)*16 + 3         ; #130 Â
        dw (.ch0_131-.chars)*16 + 2         ; #131 Ã
        dw (.ch0_132-.chars)*16 + 6         ; #132 Ä
        dw (.ch0_133-.chars)*16 + 4         ; #133 Å
        dw (.ch0_134-.chars)*16 + 3         ; #134 Æ
        dw (.ch0_135-.chars)*16 + 3         ; #135 Ç
        dw (.ch0_136-.chars)*16 + 3         ; #136 È
        dw (.ch0_137-.chars)*16 + 4         ; #137 É
        dw (.ch0_138-.chars)*16 + 3         ; #138 K
        dw (.ch0_139-.chars)*16 + 3         ; #139 Ë
        dw (.ch0_140-.chars)*16 + 4         ; #140 Ì
        dw (.ch0_141-.chars)*16 + 4         ; #141 Í
        dw (.ch0_142-.chars)*16 + 2         ; #142 Î
        dw (.ch0_143-.chars)*16 + 3         ; #143 Ï
        dw (.ch0_144-.chars)*16 + 2         ; #144 Ð
        dw (.ch0_145-.chars)*16 + 2         ; #145 Ñ
        dw (.ch0_146-.chars)*16 + 3         ; #146 T
        dw (.ch0_147-.chars)*16 + 3         ; #147 Ó
        dw (.ch0_148-.chars)*16 + 3         ; #148 Ô
        dw (.ch0_149-.chars)*16 + 2         ; #149 Õ
        dw (.ch0_150-.chars)*16 + 4         ; #150 Ö
        dw (.ch0_151-.chars)*16 + 2         ; #151 ×
        dw (.ch0_152-.chars)*16 + 4         ; #152 Ø
        dw (.ch0_153-.chars)*16 + 5         ; #153 Ù
        dw (.ch0_154-.chars)*16 + 3         ; #154 Ú
        dw (.ch0_155-.chars)*16 + 3         ; #155 Û
        dw (.ch0_156-.chars)*16 + 2         ; #156 Ü
        dw (.ch0_157-.chars)*16 + 3         ; #157 Ý
        dw (.ch0_158-.chars)*16 + 4         ; #158 Þ
        dw (.ch0_159-.chars)*16 + 3         ; #159 ß
        dw (.ch0_160-.chars)*16 + 2         ; #160 a
        dw (.ch0_161-.chars)*16 + 2         ; #161 á
        dw (.ch0_162-.chars)*16 + 3         ; #162 â
        dw (.ch0_163-.chars)*16 + 2         ; #163 ã
        dw (.ch0_164-.chars)*16 + 4         ; #164 ä
        dw (.ch0_165-.chars)*16 + 2         ; #165 e
        dw (.ch0_166-.chars)*16 + 3         ; #166 æ
        dw (.ch0_167-.chars)*16 + 3         ; #167 ç
        dw (.ch0_168-.chars)*16 + 3         ; #168 è
        dw (.ch0_169-.chars)*16 + 4         ; #169 é
        dw (.ch0_170-.chars)*16 + 3         ; #170 ê
        dw (.ch0_171-.chars)*16 + 2         ; #171 ë
        dw (.ch0_172-.chars)*16 + 3         ; #172 ì
        dw (.ch0_173-.chars)*16 + 3         ; #173 í
        dw (.ch0_174-.chars)*16 + 1         ; #174 î
        dw (.ch0_175-.chars)*16 + 3         ; #175 ï
   times 48 dw 0                            ; #176-223
        dw (.ch0_224-.chars)*16 + 2         ; #224 p
        dw (.ch0_225-.chars)*16 + 2         ; #225 c
        dw (.ch0_226-.chars)*16 + 2         ; #226 ò
        dw (.ch0_227-.chars)*16 + 3         ; #227 ó
        dw (.ch0_228-.chars)*16 + 2         ; #228 ô
        dw (.ch0_229-.chars)*16 + 2         ; #229 õ
        dw (.ch0_230-.chars)*16 + 3         ; #230 ö
        dw (.ch0_231-.chars)*16 + 2         ; #231 ÷
        dw (.ch0_232-.chars)*16 + 4         ; #232 ø
        dw (.ch0_233-.chars)*16 + 5         ; #233 ù
        dw (.ch0_234-.chars)*16 + 2         ; #234 ú
        dw (.ch0_235-.chars)*16 + 3         ; #235 û
        dw (.ch0_236-.chars)*16 + 2         ; #236 ü
        dw (.ch0_237-.chars)*16 + 3         ; #237 ý
        dw (.ch0_238-.chars)*16 + 3         ; #238 þ
        dw (.ch0_239-.chars)*16 + 3         ; #239 ÿ
        dw (.ch0_240-.chars)*16 + 5         ; #240 ¨
        dw (.ch0_241-.chars)*16 + 4         ; #241 ¸
   times 14 dw 0                            ; #142-255

align 4
.chars:
.ch0_33:    !
    ritick      2, 2, 0      
    lntick      2, 4, 2, 5      
.ch0_34:    ; "
.ch0_39:    ; '
    lntick      1, 8, 2, 2      
    lntick      3, 8, 2, 2      
.ch0_36:    ; $
    lntick      2, 2, 2, 5
    gptick     11, 0, v11_1       
.ch0_37:    ; %
    cstick      0, 9, 0, 0      
    lntick      0, 3, 1, 5      
    cstick      3, 3, 0, 0      
.ch0_38:    ; &
    gptick      7, 6, v5_5
    gptick     19, 4, 38 
.ch0_40:    ; (
    gptick      5, 3, v5_1
.ch0_41:    ; )
    gptick      9, 7, v5_1
.ch0_42:    ; *
    gptick      3, 7, 33
    gptick     10, 3, 33
    lntick      3, 3, 2, 5      
.ch0_43:    ; +
    lntick      3, 3, 2, 5      
.ch0_45:    ; -
.ch0_35:    ; #
.ch0_61:    ; =
    lntick      0, 4, 0, 5      
    lntick      0, 6, 0, 5      
    lntick      1, 2, 2, 5      
    lntick      3, 2, 2, 5      
.ch0_58:    ; :
.ch0_59:    ; ;
    cstick      1, 7, 0, 0      
.ch0_46:    ; .
.ch0_44:    ; ,
    cstick      1, 3, 0, 0      
    ritick      1, 1, 0      
.ch0_47:    ; /
.ch0_55:    ; 7
    gptick      1, 2, v5_2
    lntick      0, 8, 0, 4
.ch0_64:    ; @
    cstick      3, 6, 0, 1
.ch0_48:    ; 0
.ch0_79:    ; O
.ch0_142:   ; Î
.ch0_81:    ; Q
    gptick      5, 4, v10_1
    lntick      4, 4, 2, 4 
    lntick      3, 1, 0, 2 
.ch0_49:    ; 1
.ch0_124:   ; |
    lntick      2, 2, 2, 7      
    ritick      1, 6, 0      
.ch0_50:    ; 2
    lntick      0, 2, 0, 5
    lntick      1, 3, 1, 3
.ch0_83:    ; S
    gptick      3, 2, v4_1
.ch0_51:    ; 3
    lntick      1, 5, 0, 3
    gptick      8, 1, v4_1
.ch0_53:    ; 5
    gptick     10, 6, v4_2
    gptick     13, 4, v8_1
.ch0_52:    ; 4
    gptick     14, 2, 32
    lntick      0, 4, 0, 5
    lntick      3, 2, 2, 7
.ch0_54:    ; 6
    gptick     14, 0, v10_2 
    gptick      2, 0, 44 
.ch0_56:    ; 8
    gptick     10, 6, v5_3
.ch0_57:    ; 9
    gptick      9, 0, v10_2 
    gptick     15, 0, 44 
.ch0_60:    ; <
    gptick      7, 4, v7_1
.ch0_62:    ; >
    gptick      8, 4, v7_1
.ch0_63:    ; ?
    gptick      7, 1, v4_1

.ch0_66:    ; B
.ch0_130:   ; B
    gptick     10, 6, 45 
.ch0_80:    ; P
.ch0_144:   ; Ð
.ch0_82:    ; R
    gptick      9, 0, v5_4
.ch0_75:    ; K
.ch0_138:   ; K
    lntick      0, 2, 2, 7
    lntick      4, 2, 3, 3
    lntick      1, 5, 1, 4
.ch0_67:    ; C
.ch0_145:   ; Ñ
    ritick      5, 3, 0
.ch0_71:    ; G
    gptick      5, 4, v10_1
    gptick      4, 5, 43
.ch0_68:    ; D
    gptick      9, 0, v7_1
.ch0_76:    ; L
    lntick      0, 2, 2, 7      
    lntick      1, 2, 0, 4      
.ch0_240:   ; ¨
    ritick      1, 9, 0
    ritick      3, 9, 0
.ch0_133:   ; E
.ch0_69:    ; E
    lntick      1, 2, 0, 4      
.ch0_70:    ; F
    lntick      1, 8, 0, 4      
.ch0_72:    ; H
.ch0_141:   ; H
    lntick      1, 5, 0, 3      
    lntick      0, 2, 2, 7      
    lntick      0, 4, 2, 7      
.ch0_73:    ; I
    lntick      1, 8, 0, 3
.ch0_74:    ; J
    gptick     12, 6, v6_1
    lntick      3, 8, 0, 3 
.ch0_84:    ; T
.ch0_146:   ; Ò
    lntick      3, 3, 2, 5
    lntick      2, 2, 0, 3
    lntick      0, 8, 0, 5
.ch0_77:    ; M
.ch0_140:   ; M
    gptick      3, 5, 2, 32
    ritick      1, 7, 0
.ch0_78:    ; N
    lntick      0, 2, 2, 7
    lntick      4, 2, 2, 7
    lntick      3, 4, 3, 3
.ch0_85:    ; U
    lntick      1, 2, 0, 3
.ch0_87:    ; W
    lntick      0, 3, 2, 6
    lntick      4, 3, 2, 6
    gptick      0, 2, 1, 41
.ch0_86:    ; V
    lntick      0, 7, 0, 2
    lntick      4, 7, 0, 2
.ch0_118:   ; v
.ch0_121:   ; y
    gptick     12, 6, v7_1       
    lntick      0, 1, 2, 2       
.ch0_88:    ; X
.ch0_149:   ; Õ
    gptick      1, 2, v5_2
    gptick      4, 6, v5_5
.ch0_89:    ; Y
    gptick     18, 6, v7_1
    lntick      3, 2, 2, 2 
.ch0_90:    ; Z
    lntick      0, 2, 0, 5      
    lntick      0, 8, 0, 5      
    lntick      0, 3, 1, 5      
.ch0_92:    ; \
    gptick      4, 6, v5_5
.ch0_91:    ; [
    lntick      1, 3, 2, 5
.ch0_93:    ; ]
    lntick      1, 2, 0, 3
    lntick      1, 8, 0, 3
    lntick      3, 3, 2, 5
.ch0_65:    ; A
.ch0_128:   ; À
    lntick      0, 2, 2, 4
    lntick      4, 2, 2, 4
    lntick      1, 4, 0, 3
.ch0_94:    ; ^
    gptick      3, 1, 41      
.ch0_95:    ; _
    lntick      0, 1, 0, 5      
.ch0_96:    ; `
    lntick      3, 8, 3, 2      
.ch0_97:    ; a
.ch0_160:   ; a
    gptick      5, 4, v5_4
    gptick     16, 0, v5_6
.ch0_129:   ; Á
    lntick      1, 8, 0, 3
.ch0_98:    ; b
    lntick      0, 2, 2, 7
.ch0_112:   ; p
    gptick     16, 0, v7_3
    lntick      0, 0, 2, 7
.ch0_99:    ; c
.ch0_225:   ; c
    ritick      4, 5, 0
    ritick      4, 2, 0
.ch0_101:   ; e
.ch0_165:   ; å
.ch0_241:   ; ¸
    gptick      5, 4, v7_3
    gptick     15, 6, 43
    ritick      1, 8, 0
    ritick      3, 8, 0
.ch0_100:   ; d
    lntick      4, 2, 2, 7
.ch0_113:   ; p
.ch0_224:   ; p
    gptick      5, 4, v7_3
    lntick      4, 0, 2, 7
.ch0_102:   ; f
    gptick     17, 2, v6_1
    ritick      1, 2, 0
    lntick      0, 6, 0, 3
.ch0_103:   ; g
    gptick     12, 6, v6_1
    gptick      5, 4, v7_3 
.ch0_104:   ; h
    gptick     16, 0, v5_6
.ch0_107:   ; k
    lntick      0, 2, 2, 7
    gptick      5, 3, 41
.ch0_105:   ; i
    lntick      1, 2, 0, 3
.ch0_106:   ; j
    gptick     16, 0, 43
    ritick      2, 8, 0
    gptick     19, 5, 32
.ch0_108:   ; l
    lntick      1, 2, 0, 3
    lntick      2, 3, 2, 6
    ritick      1, 8, 0
.ch0_109:   ; m
    lntick      2, 3, 2, 2
    gptick     20, 5, 33
    lntick      4, 2, 2, 4
.ch0_110:   ; n
    lntick      0, 2, 2, 5
    gptick     16, 0, v5_6
.ch0_114:   ; r
    lntick      1, 2, 2, 5
    gptick     21, 1, 32
.ch0_115:   ; s
    gptick      1, 0, v11_1       
.ch0_116:   ; t
    gptick     16, 0, 43
    gptick     20, 3, 32       
    lntick      3, 2, 0, 2
.ch0_117:   ; u
.ch0_227:   ; ó
    lntick      4, 2, 2, 5
    gptick      5, 4, v5_6
    gptick     29, 5, 36
.ch0_119:   ; w
    lntick      2, 4, 2, 2
    gptick     22, 1, 33
    lntick      0, 3, 2, 4
    lntick      4, 3, 2, 4
.ch0_120:   ; x
    lntick      0, 2, 1, 5
    lntick      4, 2, 3, 5
.ch0_122:   ; z
    lntick      0, 2, 0, 5
    lntick      0, 6, 0, 5
    lntick      1, 2, 3, 3
.ch0_123:   ; {
    gptick      5, 3, 37
    gptick     23, 2, 32
.ch0_125:   ; }
    gptick     13, 3, 37
    gptick     24, 6, 32
.ch0_126:   ; ~
    gptick      2, 1, 33
    lntick      3, 3, 1, 2
.ch0_127:   ; ¦
    lntick      2, 2, 2, 3
    lntick      2, 6, 2, 3
.ch0_131:   ; Ã
    lntick      1, 8, 0, 4
.ch0_132:   ; Ä
    lntick      1, 2, 2, 6
    lntick      2, 8, 0, 3
    lntick      0, 1, 2, 2
.ch0_150:   ; Ö
    lntick      1, 2, 0, 4
    lntick      3, 3, 2, 6
    ritick      4, 1, 0
    lntick      0, 3, 2, 6
.ch0_134:   ; Æ
    gptick      4, 6, v5_7
    gptick      6, 2, v5_7
.ch0_148:   ; Ô
    lntick      2, 2, 2, 7
    cstick      1, 8, 0, 3
.ch0_137:   ; É
    gptick     25, 2, 33 
.ch0_136:   ; È
    lntick      0, 2, 2, 7
    lntick      1, 4, 1, 3
.ch0_139:   ; Ë
    lntick      4, 2, 2, 7
    gptick     26, 2, v4_3 
    lntick      0, 2, 1, 2 
.ch0_143:   ; Ï
    lntick      0, 2, 2, 7
    lntick      1, 8, 0, 3
.ch0_151:   ; ×
    lntick      4, 2, 2, 7
.ch0_147:   ; Ó
    gptick     27, 4, v5_6
    lntick      4, 3, 2, 6
    lntick      1, 2, 0, 3
.ch0_155:   ; Û
    lntick     16, 0, v5_3 
.ch0_152:   ; Ø
.ch0_153:   ; Ù
    lntick      0, 3, 2, 6     
    lntick      4, 2, 2, 7     
    lntick      2, 3, 2, 6     
    lntick      0, 2, 0, 5     
    lntick      4, 1, 2, 2
.ch0_154:   ; Ú
.ch0_156:   ; Ü
    lntick     26, 0, v5_3 
    lntick      1, 3, 2, 6
    ritick      0, 8, 0
.ch0_135:   ; Ç
    ritick      3, 5, 0
.ch0_157:   ; Ý
    gptick      8, 1, v4_1
    gptick     10, 6, v4_2
    lntick      1, 5, 0, 4
.ch0_158:   ; Þ
    gptick     28, 7, v5_1
    lntick      2, 3, 2, 5
    lntick      0, 2, 2, 7
    ritick      1, 5, 0 
.ch0_159:   ; ß
    gptick     13, 4, v6_2 
    lntick      4, 2, 2, 7
    lntick      0, 2, 1, 3
.ch0_161:   ; á
    gptick      8, 1, 36
.ch0_162:   ; â
.ch0_111:   ; o
.ch0_174:   ; o
    cstick      1, 6, 0, 3
    gptick      3, 2, 38
    ritick      2, 7, 0
.ch0_163:   ; ã
    lntick      1, 7, 0, 4
.ch0_164:   ; ä
    lntick      1, 2, 2, 4
    gptick     23, 0, 43
    gptick     19, 0, 35
    lntick      0, 1, 2, 2
.ch0_166:   ; æ
    gptick     12, 6, v5_3
    gptick      1, 2, v5_3
    lntick      2, 2, 2, 5
.ch0_167:   ; ç
    lntick      2, 4, 0, 2
.ch0_237:   ; ý
    gptick      2, 1, 44
    gptick      7, 5, 44
    lntick      2, 4, 0, 3
.ch0_169:   ; é
    gptick     28, 5, 33 
.ch0_168:   ; è
    lntick      4, 2, 2, 5
    lntick      1, 3, 1, 3
.ch0_170:   ; ê
    lntick      2, 2, 2, 5
    gptick      6, 3, 41 
    ritick      1, 5, 0
.ch0_171:   ; ë
    gptick      1, 1, v4_4 
.ch0_172:   ; ì
    lntick      4, 2, 2, 5
    gptick     13, 5, 33 
.ch0_173:   ; í
    lntick      0, 2, 2, 5
    lntick      1, 4, 0, 3
.ch0_175:   ; ï
    lntick      4, 2, 2, 4
    lntick      0, 2, 2, 4
.ch0_226:   ; ò
    lntick      0, 6, 0, 5
    lntick      2, 2, 2, 4
.ch0_228:   ; ô
    cstick      1, 6, 0, 3
    lntick      2, 0, 2, 6
.ch0_229:   ; õ
    lntick      0, 2, 1, 5
    lntick      4, 2, 3, 5
.ch0_230:   ; ö
    lntick      0, 2, 2, 5
    lntick      3, 3, 2, 4
    gptick     22, 0, 46 
.ch0_231:   ; ÷
    gptick     27, 4, 42 
.ch0_232:   ; ø
.ch0_233:   ; ù
    lntick      4, 2, 2, 5
    lntick      0, 3, 2, 4
    lntick      2, 3, 2, 4
    lntick      0, 2, 0, 4
    lntick      5, 1, 2, 2
.ch0_234:   ; ú
    gptick     24, 0, v4_5 
    gptick      6, 0, 43 
.ch0_235:   ; û
.ch0_236:   ; ü
    lntick      0, 3, 2, 4
    gptick     26, 0, v4_5 
    lntick      4, 2, 2, 5
.ch0_238:   ; þ
    gptick     24, 2, v4_5 
    lntick      3, 2, 3, 3
    lntick      0, 2, 2, 5
.ch0_239:   ; ÿ
    gptick     26, 3, 47 
    lntick      4, 2, 2, 5
    gptick      1, 1, 34
    