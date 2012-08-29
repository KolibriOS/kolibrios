
; Kolibri-A vectorized system fonts
; A.Jerdev <artem@jerdev.co.uk>
; Copyright (C) KolibriOS Team, 2011-12
;
; non-scalable vectorized font #01

nsvf01:

align 4
.origs:
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
	    db	0x37	; 31    J

align 4
.table:

diff16 "font01.table: ",0,$

   times 33 dw 0
	char_entry  .ch_33, 0, 2
;	dw (.ch_33 -.chars)*16 + 2	    ; #33 !
	dw (.ch_34 -.chars)*16 + 2	    ; #34 "
	dw (.ch_35 -.chars)*16 + 4	    ; #35 #
	dw (.ch_36 -.chars)*16 + 3	    ; #36 $
	dw (.ch_37 -.chars)*16 + 3	    ; #37 %
	dw (.ch_38 -.chars)*16 + 4	    ; #38 &
	dw (.ch_39 -.chars)*16 + 1	    ; #39 '
	dw (.ch_40 -.chars)*16 + 1	    ; #40 (
	dw (.ch_41 -.chars)*16 + 1	    ; #41 )
	dw (.ch_42 -.chars)*16 + 3	    ; #42 *
	dw (.ch_43 -.chars)*16 + 2	    ; #43 +
	dw (.ch_44 -.chars)*16 + 2	    ; #44 ,
	dw (.ch_45 -.chars)*16 + 1	    ; #45 -
	dw (.ch_46 -.chars)*16 + 1	    ; #46 .
	dw (.ch_47 -.chars)*16 + 1	    ; #47 /
	dw (.ch_48 -.chars)*16 + 2	    ; #48 0
	dw (.ch_49 -.chars)*16 + 2	    ; #49 1
	dw (.ch_50 -.chars)*16 + 3	    ; #50 2
	dw (.ch_51 -.chars)*16 + 3	    ; #51 3
	dw (.ch_52 -.chars)*16 + 3	    ; #52 4
	dw (.ch_53 -.chars)*16 + 2	    ; #53 5
	dw (.ch_54 -.chars)*16 + 2	    ; #54 6
	dw (.ch_55 -.chars)*16 + 2	    ; #55 7
	dw (.ch_56 -.chars)*16 + 2	    ; #56 8
	dw (.ch_57 -.chars)*16 + 2	    ; #57 9
	dw (.ch_58 -.chars)*16 + 2	    ; #58 :
	dw (.ch_59 -.chars)*16 + 3	    ; #59 ;
	dw (.ch_60 -.chars)*16 + 1	    ; #60 <
	dw (.ch_61 -.chars)*16 + 2	    ; #61 =
	dw (.ch_62 -.chars)*16 + 1	    ; #62 >
	dw (.ch_63 -.chars)*16 + 3	    ; #63 ?
	dw (.ch_64 -.chars)*16 + 2	    ; #64 @
	dw (.ch_65 -.chars)*16 + 4	    ; #65 A
	dw (.ch_66 -.chars)*16 + 3	    ; #66 B
	dw (.ch_67 -.chars)*16 + 2	    ; #67 C
	dw (.ch_68 -.chars)*16 + 2	    ; #68 D
	dw (.ch_69 -.chars)*16 + 4	    ; #69 E
	dw (.ch_70 -.chars)*16 + 3	    ; #70 F
	dw (.ch_71 -.chars)*16 + 2	    ; #71 G
	dw (.ch_72 -.chars)*16 + 3	    ; #72 H
	dw (.ch_73 -.chars)*16 + 3	    ; #73 I
	dw (.ch_74 -.chars)*16 + 2	    ; #74 J
	dw (.ch_75 -.chars)*16 + 3	    ; #75 K
	dw (.ch_76 -.chars)*16 + 2	    ; #76 L
	dw (.ch_77 -.chars)*16 + 4	    ; #77 M
	dw (.ch_78 -.chars)*16 + 3	    ; #78 N
	dw (.ch_79 -.chars)*16 + 2	    ; #79 O
	dw (.ch_80 -.chars)*16 + 2	    ; #80 P
	dw (.ch_81 -.chars)*16 + 3	    ; #81 Q
	dw (.ch_82 -.chars)*16 + 3	    ; #82 R
	dw (.ch_83 -.chars)*16 + 3	    ; #83 S
	dw (.ch_84 -.chars)*16 + 2	    ; #84 T
	dw (.ch_85 -.chars)*16 + 3	    ; #85 U
	dw (.ch_86 -.chars)*16 + 3	    ; #86 V
	dw (.ch_87 -.chars)*16 + 3	    ; #87 W
	dw (.ch_88 -.chars)*16 + 2	    ; #88 X
	dw (.ch_89 -.chars)*16 + 2	    ; #88 Y
	dw (.ch_90 -.chars)*16 + 3	    ; #90 Z
	dw (.ch_91 -.chars)*16 + 3	    ; #91 [
	dw (.ch_92 -.chars)*16 + 1	    ; #92 \
	dw (.ch_93 -.chars)*16 + 3	    ; #93 ]
	dw (.ch_94 -.chars)*16 + 1	    ; #94 ^
	dw (.ch_95 -.chars)*16 + 1	    ; #95 _
	dw (.ch_96 -.chars)*16 + 1	    ; #96 `
	dw (.ch_97 -.chars)*16 + 2	    ; #97  a
	dw (.ch_98 -.chars)*16 + 2	    ; #98  b
	dw (.ch_99 -.chars)*16 + 3	    ; #99  c
	dw (.ch_100-.chars)*16 + 2	    ; #100 d
	dw (.ch_101-.chars)*16 + 2	    ; #101 e
	dw (.ch_102-.chars)*16 + 3	    ; #102 f
	dw (.ch_103-.chars)*16 + 2	    ; #103 g
	dw (.ch_104-.chars)*16 + 2	    ; #104 h
	dw (.ch_105-.chars)*16 + 3	    ; #105 i
	dw (.ch_106-.chars)*16 + 3	    ; #106 j
	dw (.ch_107-.chars)*16 + 2	    ; #107 k
	dw (.ch_108-.chars)*16 + 3	    ; #108 l
	dw (.ch_109-.chars)*16 + 4	    ; #109 m
	dw (.ch_110-.chars)*16 + 2	    ; #110 m
	dw (.ch_111-.chars)*16 + 1	    ; #111 o
	dw (.ch_112-.chars)*16 + 2	    ; #112 p
	dw (.ch_113-.chars)*16 + 2	    ; #113 q
	dw (.ch_114-.chars)*16 + 2	    ; #114 r
	dw (.ch_115-.chars)*16 + 2	    ; #115 s
	dw (.ch_116-.chars)*16 + 3	    ; #116 t
	dw (.ch_117-.chars)*16 + 2	    ; #117 u
	dw (.ch_118-.chars)*16 + 1	    ; #118 v
	dw (.ch_119-.chars)*16 + 4	    ; #119 w
	dw (.ch_120-.chars)*16 + 2	    ; #120 x
	dw (.ch_121-.chars)*16 + 2	    ; #121 y
	dw (.ch_122-.chars)*16 + 3	    ; #122 z
	dw (.ch_123-.chars)*16 + 2	    ; #123 {
	dw (.ch_124-.chars)*16 + 1	    ; #124 |
	dw (.ch_125-.chars)*16 + 2	    ; #125 }
	dw (.ch_126-.chars)*16 + 2	    ; #126 ~
	dw (.ch_127-.chars)*16 + 2	    ; #127

	dw (.ch_128-.chars)*16 + 4	    ; #128 A
	dw (.ch_129-.chars)*16 + 3	    ; #129
	dw (.ch_130-.chars)*16 + 3	    ; #130
	dw (.ch_131-.chars)*16 + 2	    ; #131
	dw (.ch_132-.chars)*16 + 6	    ; #132
	dw (.ch_133-.chars)*16 + 4	    ; #133 E
	dw (.ch_134-.chars)*16 + 3	    ; #134
	dw (.ch_135-.chars)*16 + 3	    ; #135
	dw (.ch_136-.chars)*16 + 3	    ; #136
	dw (.ch_137-.chars)*16 + 4	    ; #137
	dw (.ch_138-.chars)*16 + 3	    ; #138 K
	dw (.ch_139-.chars)*16 + 3	    ; #139
	dw (.ch_140-.chars)*16 + 4	    ; #140 M
	dw (.ch_141-.chars)*16 + 3	    ; #141 H
	dw (.ch_142-.chars)*16 + 2	    ; #142 O
	dw (.ch_143-.chars)*16 + 3	    ; #143
	dw (.ch_144-.chars)*16 + 2	    ; #144 P
	dw (.ch_145-.chars)*16 + 2	    ; #145 C
	dw (.ch_146-.chars)*16 + 2	    ; #146 T
	dw (.ch_147-.chars)*16 + 3	    ; #147
	dw (.ch_148-.chars)*16 + 2	    ; #148
	dw (.ch_149-.chars)*16 + 2	    ; #149 X
	dw (.ch_150-.chars)*16 + 4	    ; #150
	dw (.ch_151-.chars)*16 + 2	    ; #151
	dw (.ch_152-.chars)*16 + 4	    ; #152
	dw (.ch_153-.chars)*16 + 5	    ; #153
	dw (.ch_154-.chars)*16 + 3	    ; #154 tvz
	dw (.ch_155-.chars)*16 + 3	    ; #155
	dw (.ch_156-.chars)*16 + 2	    ; #156 mz
	dw (.ch_157-.chars)*16 + 3	    ; #157
	dw (.ch_158-.chars)*16 + 4	    ; #158
	dw (.ch_159-.chars)*16 + 3	    ; #159
	dw (.ch_160-.chars)*16 + 2	    ; #160 a
	dw (.ch_161-.chars)*16 + 2	    ; #161
	dw (.ch_162-.chars)*16 + 3	    ; #162
	dw (.ch_163-.chars)*16 + 2	    ; #163
	dw (.ch_164-.chars)*16 + 4	    ; #164
	dw (.ch_165-.chars)*16 + 2	    ; #165
	dw (.ch_166-.chars)*16 + 3	    ; #166
	dw (.ch_167-.chars)*16 + 3	    ; #167
	dw (.ch_168-.chars)*16 + 3	    ; #168
	dw (.ch_169-.chars)*16 + 4	    ; #169
	dw (.ch_170-.chars)*16 + 2	    ; #170 ka
	dw (.ch_171-.chars)*16 + 2	    ; #171
	dw (.ch_172-.chars)*16 + 3	    ; #172
	dw (.ch_173-.chars)*16 + 3	    ; #173
	dw (.ch_174-.chars)*16 + 1	    ; #174
	dw (.ch_175-.chars)*16 + 3	    ; #175
	dw (.ch_176-.chars)*16 + 4	    ; #176
	dw (.ch_177-.chars)*16 + 6	    ; #177
	dw (.ch_178-.chars)*16 + 8	    ; #178
	dw (.ch_179-.chars)*16 + 1	    ; #179
	dw (.ch_180-.chars)*16 + 2	    ; #180
	dw (.ch_181-.chars)*16 + 3	    ; #181
	dw (.ch_182-.chars)*16 + 3	    ; #182
	dw (.ch_183-.chars)*16 + 3	    ; #183
	dw (.ch_184-.chars)*16 + 3	    ; #184
	dw (.ch_185-.chars)*16 + 3	    ; #185
	dw (.ch_186-.chars)*16 + 2	    ; #186
	dw (.ch_187-.chars)*16 + 3	    ; #187
	dw (.ch_188-.chars)*16 + 4	    ; #188
	dw (.ch_189-.chars)*16 + 3	    ; #189
	dw (.ch_190-.chars)*16 + 2	    ; #190
	dw (.ch_191-.chars)*16 + 2	    ; #191
	dw (.ch_192-.chars)*16 + 2	    ; #192
	dw (.ch_193-.chars)*16 + 2	    ; #193
	dw (.ch_194-.chars)*16 + 2	    ; #194
	dw (.ch_195-.chars)*16 + 2	    ; #195
	dw (.ch_196-.chars)*16 + 1	    ; #196
	dw (.ch_197-.chars)*16 + 2	    ; #197
	dw (.ch_198-.chars)*16 + 2	    ; #198
	dw (.ch_199-.chars)*16 + 3	    ; #199
	dw (.ch_200-.chars)*16 + 3	    ; #200
	dw (.ch_201-.chars)*16 + 3	    ; #201
	dw (.ch_202-.chars)*16 + 3	    ; #202
	dw (.ch_203-.chars)*16 + 3	    ; #203
	dw (.ch_204-.chars)*16 + 3	    ; #204
	dw (.ch_205-.chars)*16 + 2	    ; #205
	dw (.ch_206-.chars)*16 + 4	    ; #206
	dw (.ch_207-.chars)*16 + 3	    ; #207
	dw (.ch_208-.chars)*16 + 3	    ; #208
	dw (.ch_209-.chars)*16 + 3	    ; #209
	dw (.ch_210-.chars)*16 + 3	    ; #210
	dw (.ch_211-.chars)*16 + 3	    ; #211
	dw (.ch_212-.chars)*16 + 2	    ; #212
	dw (.ch_213-.chars)*16 + 2	    ; #213
	dw (.ch_214-.chars)*16 + 3	    ; #214
	dw (.ch_215-.chars)*16 + 4	    ; #215
	dw (.ch_216-.chars)*16 + 4	    ; #216
	dw (.ch_217-.chars)*16 + 2	    ; #217
	dw (.ch_218-.chars)*16 + 2	    ; #218 
   times 5 dw 0 			    ; #219-223
	dw (.ch_224-.chars)*16 + 2	    ; #224 p
	dw (.ch_225-.chars)*16 + 3	    ; #225 c
	dw (.ch_226-.chars)*16 + 2	    ; #226 â
	dw (.ch_227-.chars)*16 + 3	    ; #227 ã
	dw (.ch_228-.chars)*16 + 2	    ; #228 ä
	dw (.ch_229-.chars)*16 + 2	    ; #229 å
	dw (.ch_230-.chars)*16 + 3	    ; #230 æ
	dw (.ch_231-.chars)*16 + 2	    ; #231 ç
	dw (.ch_232-.chars)*16 + 4	    ; #232 è
	dw (.ch_233-.chars)*16 + 5	    ; #233 é
	dw (.ch_234-.chars)*16 + 2	    ; #234 ê
	dw (.ch_235-.chars)*16 + 3	    ; #235 ë
	dw (.ch_236-.chars)*16 + 2	    ; #236 ì
	dw (.ch_237-.chars)*16 + 3	    ; #237 í
	dw (.ch_238-.chars)*16 + 3	    ; #238 î
	dw (.ch_239-.chars)*16 + 3	    ; #239 ï
	dw (.ch_240-.chars)*16 + 5	    ; #240 ð
	dw (.ch_241-.chars)*16 + 4	    ; #241 ñ
   times 14 dw 0			    ; #242-255
diff10 "check font01 table size: ", .table, $

; ----------------------------------------------------
align 4

diff16 "font01.chars: ",0,$

.chars:
    dw	0
.ch_33:    ; !
    ritick	2, 2, 0
    lntick	2, 4, 2, 5
.ch_34:    ; "
.ch_39:    ; '
    lntick	3, 8, 2, 2
    lntick	1, 8, 2, 2
.ch_36:    ; $
    lntick	2, 1, 2, 7
.ch_115:   ; s
    gptick	7, 3, 90
    lntick	0, 2, 0, 4
.ch_37:    ; %
    cstick	0, 8, 0, 0
    lntick	0, 3, 1, 5
    cstick	3, 3, 0, 0
.ch_38:    ; &
    gptick	8, 6, 84
    gptick     19, 4, 44
    gptick	4, 0, 44
    ritick	3, 3, 0
.ch_40:    ; (
    gptick	5, 3, 80
.ch_41:    ; )
    gptick	9, 7, 80
.ch_42:    ; *
    gptick	3, 7, 39
    gptick     10, 3, 39
    lntick	2, 3, 2, 5
.ch_43:    ; +
    lntick	2, 2, 2, 5
.ch_45:    ; -
.ch_35:    ; #
.ch_61:    ; =
    lntick	0, 4, 0, 5
    lntick	0, 6, 0, 5
    lntick	1, 3, 2, 5
    lntick	3, 3, 2, 5
.ch_58:    ; :
.ch_59:    ; ;
    cstick	1, 7, 0, 0
.ch_46:    ; .
.ch_44:    ; ,
    cstick	1, 3, 0, 0
    ritick	1, 1, 0
.ch_47:    ; /
.ch_55:    ; 7
    gptick	1, 2, 81
    lntick	0, 8, 0, 4
.ch_64:    ; @
    cstick	2, 6, 0, 1
.ch_48:    ; 0
.ch_79:    ; O
.ch_142:   ;
.ch_81:    ; Q
    gptick	5, 4, 116
    lntick	4, 3, 2, 5
    lntick	3, 1, 0, 2
.ch_49:    ; 1
.ch_124:   ; |
    lntick	2, 2, 2, 7
    ritick	1, 6, 0
.ch_50:    ; 2
    lntick	0, 2, 0, 5
    lntick	1, 3, 1, 3
.ch_51:    ; 3
    gptick	8, 1, 64
    gptick     10, 6, 65
.ch_83:    ; S
    lntick	1, 5, 0, 3
    gptick	3, 2, 55
.ch_53:    ; 5
    gptick     10, 6, 65
    gptick     13, 4, 104
.ch_52:    ; 4
    lntick	0, 5, 1, 4
    lntick	0, 4, 0, 5
    lntick	3, 2, 2, 7
.ch_54:    ; 6
    gptick     14, 0, 105
    gptick	2, 2, 50
.ch_56:    ; 8
    gptick     10, 6, 82
.ch_57:    ; 9
    gptick	9, 0, 105
    gptick     15, 6, 50
.ch_60:    ; <
    gptick	7, 4, 96
.ch_62:    ; >
    gptick	8, 0, 96
.ch_63:    ; ?
    gptick	4, 1, 64
    gptick     24, 2, 38
    ritick	2, 2, 0
.ch_66:    ; B
.ch_130:   ; B
    gptick     10, 6, 51
    gptick	4, 0, 89
    lntick	0, 2, 2, 7
.ch_80:    ; P
.ch_144:   ;
.ch_82:    ; R
    gptick	9, 0, 98
.ch_75:    ; K
.ch_138:   ; K
    lntick	0, 2, 2, 7
    lntick	4, 2, 3, 3
    lntick	1, 5, 1, 4
.ch_67:    ; C
.ch_145:   ;
    ritick	4, 3, 0
.ch_71:    ; G
    gptick	5, 4, 116
    gptick     13, 0, 49
.ch_68:    ; D
    gptick	9, 0, 97
.ch_76:    ; L
    lntick	0, 2, 2, 7
    lntick	1, 2, 0, 4
.ch_240:   ; ð
    ritick	1, 9, 0
    ritick	3, 9, 0
.ch_133:   ; E
.ch_69:    ; E
    lntick	1, 2, 0, 4
.ch_70:    ; F
    lntick	1, 8, 0, 4
.ch_72:    ; H
.ch_141:   ; H
    lntick	1, 5, 0, 3
    lntick	0, 2, 2, 7
    lntick	4, 2, 2, 7
.ch_73:    ; I
    lntick	1, 8, 0, 3
    lntick	2, 3, 2, 5
    lntick	1, 2, 0, 3
.ch_74:    ; J
    gptick     31, 6, 88
    lntick	2, 8, 0, 3
.ch_84:    ; T
.ch_146:   ; ’
    lntick	2, 2, 2, 6
    lntick	0, 8, 0, 5
.ch_77:    ; M
.ch_140:   ; M
    gptick	21, 2, 38
    ritick	1, 7, 0
.ch_78:    ; N
    lntick	0, 2, 2, 7
    lntick	4, 2, 2, 7
    lntick	3, 4, 3, 3
.ch_85:    ; U
    lntick	1, 2, 0, 3
.ch_87:    ; W
    lntick	0, 3, 2, 6
    lntick	4, 3, 2, 6
    gptick	1, 1, 54
.ch_86:    ; V
    lntick	0, 7, 2, 2
    lntick	4, 7, 2, 2
.ch_118:   ; v
.ch_121:   ; y
    gptick     12, 6, 96
    lntick	1, 0, 1, 2
.ch_88:    ; X
.ch_149:   ;
    gptick	1, 2, 81
    gptick	4, 6, 84
.ch_89:    ; Y
    gptick     18, 6, 96
    lntick	2, 2, 2, 2
.ch_90:    ; Z
    lntick	0, 2, 0, 5
    lntick	0, 8, 0, 5
    lntick	0, 3, 1, 5
.ch_92:    ; \
    gptick	4, 6, 84
.ch_91:    ; [
    lntick	1, 3, 2, 5
.ch_93:    ; ]
    lntick	1, 2, 0, 3
    lntick	1, 8, 0, 3
    lntick	3, 3, 2, 5
.ch_65:    ; A
.ch_128:   ;
    lntick	0, 2, 2, 4
    lntick	4, 2, 2, 4
    lntick	1, 4, 0, 3
.ch_94:    ; ^
    gptick	3, 1, 54
.ch_95:    ; _
    lntick	0, 1, 0, 5
.ch_96:    ; `
    lntick	3, 8, 3, 2
.ch_97:    ; a
.ch_160:   ;
    gptick	5, 4, 89
    gptick	7, 2, 70
.ch_129:   ;
    lntick	1, 8, 0, 3
.ch_98:    ; b
    lntick	0, 2, 2, 7
.ch_112:   ; p
.ch_224:   ; p
    gptick     16, 0, 98
    lntick	0, 0, 2, 7
.ch_99:    ; c
.ch_225:   ; c
    ritick	4, 5, 0
    ritick	4, 2, 0
.ch_101:   ; e
.ch_165:   ;
.ch_241:   ; ñ
    gptick	5, 4, 98
    gptick     15, 6, 49
    ritick	1, 8, 0
    ritick	3, 8, 0
.ch_100:   ; d
    lntick	4, 2, 2, 7
.ch_113:   ; q
    gptick	5, 4, 98
    lntick	4, 0, 2, 7
.ch_102:   ; f
    gptick     17, 2, 88
    ritick	1, 2, 0
    lntick	0, 6, 0, 3
.ch_103:   ; g
    gptick     15, 6, 88
    gptick	5, 4, 98
.ch_104:   ; h
    gptick     16, 0, 85
.ch_107:   ; k
    lntick	0, 2, 2, 7
    gptick	5, 3, 54
.ch_105:   ; i
    lntick	1, 2, 0, 3
.ch_106:   ; j
    gptick     16, 0, 49
    ritick	2, 8, 0
    gptick     19, 5, 38
.ch_108:   ; l
    lntick	1, 2, 0, 3
    lntick	2, 3, 2, 6
    ritick	1, 8, 0
.ch_109:   ; m
    lntick	2, 3, 2, 2
    gptick     20, 5, 39
    lntick	4, 2, 2, 4
.ch_110:   ; n
    lntick	0, 2, 2, 5
    gptick     16, 0, 85
.ch_114:   ; r
    lntick	1, 2, 2, 5
    gptick     21, 1, 38
.ch_116:   ; t
    gptick     16, 0, 49
    gptick     20, 3, 38
    lntick	3, 2, 0, 2
.ch_117:   ; u
.ch_227:   ; ã
    lntick	4, 2, 2, 5
    gptick	5, 4, 85
    gptick     29, 5, 42
.ch_119:   ; w
    lntick	2, 4, 2, 2
    gptick     22, 1, 39
    lntick	0, 3, 2, 4
    lntick	4, 3, 2, 4
.ch_120:   ; x
    lntick	0, 2, 1, 5
    lntick	4, 2, 3, 5
.ch_122:   ; z
    lntick	0, 2, 0, 5
    lntick	0, 6, 0, 5
    lntick	1, 3, 1, 3
.ch_123:   ; {
    gptick	5, 3, 43
    gptick     23, 2, 38
.ch_125:   ; }
    gptick     13, 3, 43
    gptick     24, 6, 38
.ch_126:   ; ~
    gptick	2, 1, 39
    lntick	3, 4, 1, 2
.ch_127:   ;
    lntick	2, 2, 2, 3
    lntick	2, 6, 2, 3
.ch_131:   ;
.ch_132:   ;
    lntick	1, 8, 0, 4
    lntick	1, 2, 2, 6
    lntick	0, 1, 2, 2
.ch_150:   ;
    lntick	0, 2, 0, 5
    lntick	3, 3, 2, 6
    ritick	4, 1, 0
    lntick	0, 3, 2, 6
.ch_134:   ; ZH
    gptick	4, 6, 86
    gptick	6, 2, 86
.ch_148:   ; EF
    lntick	2, 2, 2, 7
    cstick	1, 8, 0, 3
.ch_137:   ; IJ
    gptick     25, 2, 39
.ch_136:   ; I
    lntick	0, 2, 2, 7
    lntick	1, 4, 1, 3
.ch_139:   ; K
    lntick	4, 2, 2, 7
    gptick     26, 2, 66
    lntick	0, 2, 1, 2
.ch_143:   ; PE
    lntick	0, 2, 2, 7
    lntick	1, 8, 0, 3
.ch_151:   ; CHA
    lntick	4, 2, 2, 7
.ch_147:   ; UU
    gptick     27, 4, 85
    lntick	4, 3, 2, 6
    lntick	1, 2, 0, 3
.ch_155:   ; YY
    gptick	3, 0, 82
.ch_152:   ; SHA
.ch_153:   ; SCHA
    lntick	0, 3, 2, 6
    lntick	4, 2, 2, 7
    lntick	2, 3, 2, 6
    lntick	0, 2, 0, 5
    lntick	4, 1, 2, 2
.ch_154:   ; TVZNAK
.ch_156:   ; MZNAK
    gptick     14, 0, 89
    lntick	1, 3, 2, 6
    ritick	0, 8, 0
.ch_135:   ; ZE
    ritick	3, 5, 0
.ch_157:   ; AE
    gptick	8, 1, 64
    gptick     10, 6, 65
    lntick	1, 5, 0, 4
.ch_158:   ; JU
    gptick     28, 7, 80
    lntick	2, 3, 2, 5
    lntick	0, 2, 2, 7
    ritick	1, 5, 0
.ch_159:   ; JA
    gptick     13, 4, 89
    lntick	4, 2, 2, 7
    lntick	0, 2, 1, 3
.ch_161:   ; be
    gptick	8, 1, 42
.ch_162:   ; ve
.ch_111:   ; o
.ch_174:   ; o
    cstick	1, 6, 0, 3
    gptick	3, 2, 44
    ritick	2, 7, 0
.ch_163:   ; ge
    lntick	1, 6, 0, 4
.ch_164:   ; de
    lntick	1, 2, 2, 4
    gptick     23, 0, 49
    gptick     19, 0, 41
    lntick	0, 1, 2, 2
.ch_166:   ; zhe
    gptick     12, 6, 82
    gptick	1, 2, 82
    lntick	2, 2, 2, 5
.ch_167:   ; ze
    lntick	2, 4, 0, 2
.ch_237:   ; ae
    gptick	2, 1, 50
    gptick	7, 5, 50
    lntick	2, 4, 0, 3
.ch_169:   ; ji
    gptick     28, 5, 39
.ch_168:   ; ii
    lntick	4, 2, 2, 5
    lntick	1, 3, 1, 3
.ch_170:   ; ka
    lntick	0, 2, 2, 5
    gptick	5, 3, 54
.ch_171:   ; el
    gptick	1, 1, 67
.ch_172:   ; em
    lntick	4, 2, 2, 5
    gptick     13, 5, 39
.ch_173:   ; en
    lntick	0, 2, 2, 5
    lntick	1, 4, 0, 3
.ch_175:   ; pe
    lntick	4, 2, 2, 5
    lntick	0, 2, 2, 4
.ch_226:   ; te
    lntick	0, 6, 0, 5
    lntick	2, 2, 2, 4
.ch_228:   ; ef
    cstick	1, 6, 0, 3
    lntick	2, 0, 2, 6
.ch_229:   ; ha
    lntick	0, 2, 1, 5
    lntick	4, 2, 3, 5
.ch_230:   ; tse
    lntick	0, 2, 2, 5
    lntick	3, 3, 2, 4
    gptick     22, 0, 52
.ch_231:   ; che
    gptick     27, 4, 48
.ch_232:   ; sha
.ch_233:   ; scha
    lntick	4, 2, 2, 5
    lntick	0, 3, 2, 4
    lntick	2, 3, 2, 4
    lntick	0, 2, 0, 4
    lntick	5, 1, 2, 2
.ch_234:   ; tvznak
    gptick     24, 0, 68
    gptick	3, 0, 49
.ch_235:   ; ë
.ch_236:   ; ì
    lntick	0, 3, 2, 4
    gptick     26, 0, 68
    lntick	4, 2, 2, 5
.ch_238:   ; î
    gptick     24, 2, 68
    lntick	3, 2, 3, 3
    lntick	0, 2, 2, 5
.ch_239:   ; ï
    gptick     26, 3, 53
    lntick	4, 2, 2, 5
    gptick	1, 1, 40

.ch_178:   ; pseudo-graphics
    ritick	0, 8, 0
    lntick	0, 5, 1, 4
    lntick	0, 2, 1, 5
    lntick	1, 0, 1, 4
.ch_176:   ;
    lntick	0, 4, 1, 5
    lntick	2, 0, 1, 3
.ch_177:   ;
    lntick	0, 7, 1, 2
    lntick	0, 1, 1, 5
    lntick	0, 3, 1, 4
    lntick	0, 5, 1, 4
    lntick	1, 0, 1, 4
    lntick	3, 0, 1, 2
.ch_184:   ;
    lntick	2, 0, 2, 6
.ch_181:   ;
    lntick	0, 3, 0, 2
    lntick	0, 5, 0, 2
.ch_179:   ;
.ch_180:   ;
    lntick	2, 0, 2, 9
    lntick	0, 4, 0, 2
.ch_183:   ;
    lntick	0, 4, 0, 4
    lntick	1, 0, 2, 4
    lntick	3, 0, 2, 4
.ch_185:   ;
    gptick     11, 0, 49
    gptick	9, 6, 52
    lntick	3, 0, 2, 9
.ch_182:   ;
.ch_215:   ;
    ritick	0, 4, 0
.ch_186:   ;
.ch_199:   ;
    lntick	1, 0, 2, 9
    lntick	3, 0, 2, 9
    ritick	4, 4, 0
.ch_204:   ;
    gptick     15, 4, 49
    gptick     30, 2, 52
    lntick	1, 0, 2, 9
.ch_187:   ;
    lntick	3, 0, 2, 5
    lntick	0, 5, 0, 4
    gptick     11, 0, 49
.ch_188:   ;
    ritick	0, 5, 0
    lntick	0, 3, 0, 4
.ch_189:   ;
    lntick	3, 4, 2, 5
    lntick	1, 5, 2, 4
    lntick	0, 4, 0, 3
.ch_190:   ;
    gptick	2, 0, 69
.ch_192:   ;
    lntick	2, 5, 2, 4
    lntick	2, 4, 0, 3
.ch_191:   ;
    lntick	0, 4, 0, 3
.ch_194:   ;
    lntick	2, 0, 2, 4
.ch_193:   ;
.ch_196:   ;
    lntick	0, 4, 0, 5
    lntick	2, 5, 2, 4
.ch_195:   ;
    lntick	0, 3, 0, 2
.ch_197:   ;
    lntick	2, 0, 2, 9
    lntick	0, 4, 0, 5
.ch_198:   ;
    lntick	2, 0, 2, 9
.ch_213:   ;
    gptick	7, 4, 69
    lntick	2, 0, 2, 4
.ch_200:   ;
    gptick     15, 4, 49
    lntick	1, 3, 0, 4
    lntick	1, 4, 2, 5
.ch_201:   ;
    gptick     30, 2, 52
    lntick	1, 5, 0, 4
    lntick	1, 0, 2, 5
.ch_207:   ;
.ch_216:   ;
    lntick	2, 6, 2, 3
.ch_205:   ;
.ch_209:   ;
    lntick	0, 3, 0, 5
    lntick	0, 5, 0, 5
    lntick	2, 0, 2, 3
.ch_202:   ;
    lntick	0, 3, 0, 5
.ch_206:   ;
    gptick	9, 6, 52
    gptick     15, 4, 49
.ch_203:   ;
    gptick     30, 2, 52
    gptick     11, 0, 49
    lntick	0, 5, 0, 5
.ch_211:   ;
    lntick	1, 4, 0, 1
.ch_208:   ;
    lntick	1, 5, 2, 4
    lntick	3, 5, 2, 4
.ch_210:   ;
    lntick	0, 4, 0, 5
.ch_214:   ;
    lntick	1, 0, 2, 4
    lntick	3, 0, 2, 4
    lntick	1, 4, 0, 1
.ch_212:   ;
    gptick	7, 4, 69
    lntick	2, 6, 2, 3
.ch_217:   ;
    lntick	0, 4, 0, 3
    lntick	2, 5, 2, 4
.ch_218:   ;
    lntick	2, 4, 0, 3
    lntick	2, 0, 2, 4

diff10 "font01 size ", .chars, $


