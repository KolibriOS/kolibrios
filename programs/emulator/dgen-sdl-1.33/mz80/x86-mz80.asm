; For assembly by NASM only
bits 32

; Theory of operation

; EDI=General purpose
; ESI=Program counter + base address
; EBP=z80Base
; AX=AF
; BX=HL
; CX=BC
; DX=General purpose

; Using stack calling conventions
; Extended input/output instructions treat (C) as I/O address

IFF1		equ	01h
IFF2		equ	02h
CPUREG_PC		equ	00h
CPUREG_SP		equ	01h
CPUREG_AF		equ	02h
CPUREG_BC		equ	03h
CPUREG_DE		equ	04h
CPUREG_HL		equ	05h
CPUREG_AFPRIME		equ	06h
CPUREG_BCPRIME		equ	07h
CPUREG_DEPRIME		equ	08h
CPUREG_HLPRIME		equ	09h
CPUREG_IX		equ	0ah
CPUREG_IY		equ	0bh
CPUREG_I		equ	0ch
CPUREG_A		equ	0dh
CPUREG_F		equ	0eh
CPUREG_B		equ	0fh
CPUREG_C		equ	10h
CPUREG_D		equ	11h
CPUREG_E		equ	12h
CPUREG_H		equ	13h
CPUREG_L		equ	14h
CPUREG_IFF1		equ	15h
CPUREG_IFF2		equ	16h
CPUREG_CARRY		equ	17h
CPUREG_NEGATIVE		equ	18h
CPUREG_PARITY		equ	19h
CPUREG_OVERFLOW		equ	1ah
CPUREG_HALFCARRY		equ	1bh
CPUREG_ZERO		equ	1ch
CPUREG_SIGN		equ	1dh
CPUREG_MAXINDEX		equ	1eh



		section	.data	use32 flat class=data

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

		global	_mz80contextBegin
_mz80contextBegin:
		global	_z80pc
		global	z80pc_
		global	z80pc
		global	_z80nmiAddr
		global	_z80intAddr
		global	z80intAddr

; DO NOT CHANGE THE ORDER OF AF, BC, DE, HL and THE PRIME REGISTERS!

_z80Base	dd	0	; Base address for Z80 stuff
_z80MemRead	dd	0	; Offset of memory read structure array
_z80MemWrite	dd	0	; Offset of memory write structure array
_z80IoRead	dd	0	; Base address for I/O reads list
_z80IoWrite	dd	0	; Base address for I/O write list
_z80clockticks	dd	0	; # Of clock tips that have elapsed
_z80iff	dd	0	; Non-zero if we're in an interrupt
_z80interruptMode dd	0	; Interrupt mode
_z80halted	dd	0	; 0=Not halted, 1=Halted
_z80af		dd	0	; A Flag & Flags
_z80bc		dd	0	; BC
_z80de		dd	0	; DE
_z80hl		dd	0	; HL
_z80afprime	dd	0	; A Flag & Flags prime
_z80bcprime	dd	0	; BC prime
_z80deprime	dd	0	; DE prime
_z80hlprime	dd	0	; HL prime

; The order of the following registers can be changed without adverse
; effect. Keep the WORD and DWORDs on boundaries of two for faster access

_z80ix		dd	0	; IX
_z80iy		dd	0	; IY
_z80sp		dd	0	; Stack pointer
z80pc:
z80pc_:
_z80pc		dd	0	; PC
_z80nmiAddr	dd	0	; Address to jump to for NMI
z80intAddr:
_z80intAddr	dd	0	; Address to jump to for INT
_z80rCounter	dd	0	; R Register counter
_z80i		db	0	; I register
_z80r		db	0	; R register
_z80intPending	db	0	; Non-zero if an interrupt is pending

_mz80contextEnd:

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

dwElapsedTicks	dd	0	; # Of ticks elapsed
cyclesRemaining	dd	0	; # Of cycles remaining
dwOriginalExec	dd	0	; # Of cycles originally executing
dwLastRSample	dd	0	; Last sample for R computation
dwEITiming	dd	0	; Used when we cause an interrupt
_orgval	dw	0	; Scratch area
_orgval2	dw	0	; Scratch area
_wordval	dw	0	; Scratch area
_intData	db	0	; Interrupt data when an interrupt is pending
bEIExit	db	0	; Are we exiting because of an EI instruction?

RegTextPC	db	'PC',0
RegTextAF	db	'AF',0
RegTextBC	db	'BC',0
RegTextDE	db	'DE',0
RegTextHL	db	'HL',0
RegTextAFP	db	'AF',27h,0
RegTextBCP	db	'BC',27h,0
RegTextDEP	db	'DE',27h,0
RegTextHLP	db	'HL',27h,0
RegTextIX	db	'IX',0
RegTextIY	db	'IY',0
RegTextSP	db	'SP',0
RegTextI	db	'I',0
RegTextR	db	'R',0
RegTextA	db	'A',0
RegTextB	db	'B',0
RegTextC	db	'C',0
RegTextD	db	'D',0
RegTextE	db	'E',0
RegTextH	db	'H',0
RegTextL	db	'L',0
RegTextF	db	'F',0
RegTextCarry	db	'Carry',0
RegTextNegative	db	'Negative',0
RegTextParity	db	'Parity',0
RegTextOverflow	db	'Overflow',0
RegTextHalfCarry	db	'HalfCarry',0
RegTextZero	db	'Zero',0
RegTextSign	db	'Sign',0
RegTextIFF1	db	'IFF1',0
RegTextIFF2	db	'IFF2',0

intModeTStates:
		db	13	; IM 0 - 13 T-States
		db	11	; IM 1 - 11 T-States
		db	11	; IM 2 - 11 T-States


;
; Info is in: pointer to text, address, shift value, mask value, size of data chunk
;

RegTable:
		dd	RegTextPC, _z80pc - _mz80contextBegin, 0, 0ffffh
		dd	RegTextSP, _z80sp - _mz80contextBegin, 0, 0ffffh
		dd	RegTextAF, _z80af - _mz80contextBegin, 0, 0ffffh
		dd	RegTextBC, _z80bc - _mz80contextBegin, 0, 0ffffh
		dd	RegTextDE, _z80de - _mz80contextBegin, 0, 0ffffh
		dd	RegTextHL, _z80hl - _mz80contextBegin, 0, 0ffffh
		dd	RegTextAFP, _z80af - _mz80contextBegin, 0, 0ffffh
		dd	RegTextBCP, _z80bc - _mz80contextBegin, 0, 0ffffh
		dd	RegTextDEP, _z80de - _mz80contextBegin, 0, 0ffffh
		dd	RegTextHLP, _z80hl - _mz80contextBegin, 0, 0ffffh
		dd	RegTextIX, _z80ix - _mz80contextBegin, 0, 0ffffh
		dd	RegTextIY, _z80iy - _mz80contextBegin, 0, 0ffffh
		dd	RegTextI, _z80i - _mz80contextBegin, 0, 0ffh
		dd	RegTextR, _z80r - _mz80contextBegin, 0, 0ffh
		dd	RegTextA, (_z80af + 1) - _mz80contextBegin, 0, 0ffh
		dd	RegTextF, _z80af - _mz80contextBegin, 0, 0ffh
		dd	RegTextB, (_z80bc + 1) - _mz80contextBegin, 0, 0ffh
		dd	RegTextC, _z80bc - _mz80contextBegin, 0, 0ffh
		dd	RegTextD, (_z80de + 1) - _mz80contextBegin, 0, 0ffh
		dd	RegTextE, _z80de - _mz80contextBegin, 0, 0ffh
		dd	RegTextH, (_z80hl + 1) - _mz80contextBegin, 0, 0ffh
		dd	RegTextL, _z80hl - _mz80contextBegin, 0, 0ffh
		dd	RegTextIFF1, _z80iff - _mz80contextBegin, 0, 01h
		dd	RegTextIFF2, _z80iff - _mz80contextBegin, 1, 01h
		dd	RegTextCarry, _z80af - _mz80contextBegin, 0, 01h
		dd	RegTextNegative, _z80af - _mz80contextBegin, 1, 01h
		dd	RegTextParity, _z80af - _mz80contextBegin, 2, 01h
		dd	RegTextOverflow, _z80af - _mz80contextBegin, 2, 01h
		dd	RegTextHalfCarry, _z80af - _mz80contextBegin, 4, 01h
		dd	RegTextZero, _z80af - _mz80contextBegin, 6, 01h
		dd	RegTextSign, _z80af - _mz80contextBegin, 7, 01h

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

z80regular:
		dd	RegInst00
		dd	RegInst01
		dd	RegInst02
		dd	RegInst03
		dd	RegInst04
		dd	RegInst05
		dd	RegInst06
		dd	RegInst07
		dd	RegInst08
		dd	RegInst09
		dd	RegInst0a
		dd	RegInst0b
		dd	RegInst0c
		dd	RegInst0d
		dd	RegInst0e
		dd	RegInst0f
		dd	RegInst10
		dd	RegInst11
		dd	RegInst12
		dd	RegInst13
		dd	RegInst14
		dd	RegInst15
		dd	RegInst16
		dd	RegInst17
		dd	RegInst18
		dd	RegInst19
		dd	RegInst1a
		dd	RegInst1b
		dd	RegInst1c
		dd	RegInst1d
		dd	RegInst1e
		dd	RegInst1f
		dd	RegInst20
		dd	RegInst21
		dd	RegInst22
		dd	RegInst23
		dd	RegInst24
		dd	RegInst25
		dd	RegInst26
		dd	RegInst27
		dd	RegInst28
		dd	RegInst29
		dd	RegInst2a
		dd	RegInst2b
		dd	RegInst2c
		dd	RegInst2d
		dd	RegInst2e
		dd	RegInst2f
		dd	RegInst30
		dd	RegInst31
		dd	RegInst32
		dd	RegInst33
		dd	RegInst34
		dd	RegInst35
		dd	RegInst36
		dd	RegInst37
		dd	RegInst38
		dd	RegInst39
		dd	RegInst3a
		dd	RegInst3b
		dd	RegInst3c
		dd	RegInst3d
		dd	RegInst3e
		dd	RegInst3f
		dd	RegInst40
		dd	RegInst41
		dd	RegInst42
		dd	RegInst43
		dd	RegInst44
		dd	RegInst45
		dd	RegInst46
		dd	RegInst47
		dd	RegInst48
		dd	RegInst49
		dd	RegInst4a
		dd	RegInst4b
		dd	RegInst4c
		dd	RegInst4d
		dd	RegInst4e
		dd	RegInst4f
		dd	RegInst50
		dd	RegInst51
		dd	RegInst52
		dd	RegInst53
		dd	RegInst54
		dd	RegInst55
		dd	RegInst56
		dd	RegInst57
		dd	RegInst58
		dd	RegInst59
		dd	RegInst5a
		dd	RegInst5b
		dd	RegInst5c
		dd	RegInst5d
		dd	RegInst5e
		dd	RegInst5f
		dd	RegInst60
		dd	RegInst61
		dd	RegInst62
		dd	RegInst63
		dd	RegInst64
		dd	RegInst65
		dd	RegInst66
		dd	RegInst67
		dd	RegInst68
		dd	RegInst69
		dd	RegInst6a
		dd	RegInst6b
		dd	RegInst6c
		dd	RegInst6d
		dd	RegInst6e
		dd	RegInst6f
		dd	RegInst70
		dd	RegInst71
		dd	RegInst72
		dd	RegInst73
		dd	RegInst74
		dd	RegInst75
		dd	RegInst76
		dd	RegInst77
		dd	RegInst78
		dd	RegInst79
		dd	RegInst7a
		dd	RegInst7b
		dd	RegInst7c
		dd	RegInst7d
		dd	RegInst7e
		dd	RegInst7f
		dd	RegInst80
		dd	RegInst81
		dd	RegInst82
		dd	RegInst83
		dd	RegInst84
		dd	RegInst85
		dd	RegInst86
		dd	RegInst87
		dd	RegInst88
		dd	RegInst89
		dd	RegInst8a
		dd	RegInst8b
		dd	RegInst8c
		dd	RegInst8d
		dd	RegInst8e
		dd	RegInst8f
		dd	RegInst90
		dd	RegInst91
		dd	RegInst92
		dd	RegInst93
		dd	RegInst94
		dd	RegInst95
		dd	RegInst96
		dd	RegInst97
		dd	RegInst98
		dd	RegInst99
		dd	RegInst9a
		dd	RegInst9b
		dd	RegInst9c
		dd	RegInst9d
		dd	RegInst9e
		dd	RegInst9f
		dd	RegInsta0
		dd	RegInsta1
		dd	RegInsta2
		dd	RegInsta3
		dd	RegInsta4
		dd	RegInsta5
		dd	RegInsta6
		dd	RegInsta7
		dd	RegInsta8
		dd	RegInsta9
		dd	RegInstaa
		dd	RegInstab
		dd	RegInstac
		dd	RegInstad
		dd	RegInstae
		dd	RegInstaf
		dd	RegInstb0
		dd	RegInstb1
		dd	RegInstb2
		dd	RegInstb3
		dd	RegInstb4
		dd	RegInstb5
		dd	RegInstb6
		dd	RegInstb7
		dd	RegInstb8
		dd	RegInstb9
		dd	RegInstba
		dd	RegInstbb
		dd	RegInstbc
		dd	RegInstbd
		dd	RegInstbe
		dd	RegInstbf
		dd	RegInstc0
		dd	RegInstc1
		dd	RegInstc2
		dd	RegInstc3
		dd	RegInstc4
		dd	RegInstc5
		dd	RegInstc6
		dd	RegInstc7
		dd	RegInstc8
		dd	RegInstc9
		dd	RegInstca
		dd	RegInstcb
		dd	RegInstcc
		dd	RegInstcd
		dd	RegInstce
		dd	RegInstcf
		dd	RegInstd0
		dd	RegInstd1
		dd	RegInstd2
		dd	RegInstd3
		dd	RegInstd4
		dd	RegInstd5
		dd	RegInstd6
		dd	RegInstd7
		dd	RegInstd8
		dd	RegInstd9
		dd	RegInstda
		dd	RegInstdb
		dd	RegInstdc
		dd	RegInstdd
		dd	RegInstde
		dd	RegInstdf
		dd	RegInste0
		dd	RegInste1
		dd	RegInste2
		dd	RegInste3
		dd	RegInste4
		dd	RegInste5
		dd	RegInste6
		dd	RegInste7
		dd	RegInste8
		dd	RegInste9
		dd	RegInstea
		dd	RegInsteb
		dd	RegInstec
		dd	RegInsted
		dd	RegInstee
		dd	RegInstef
		dd	RegInstf0
		dd	RegInstf1
		dd	RegInstf2
		dd	RegInstf3
		dd	RegInstf4
		dd	RegInstf5
		dd	RegInstf6
		dd	RegInstf7
		dd	RegInstf8
		dd	RegInstf9
		dd	RegInstfa
		dd	RegInstfb
		dd	RegInstfc
		dd	RegInstfd
		dd	RegInstfe
		dd	RegInstff

z80PrefixCB:
		dd	CBInst00
		dd	CBInst01
		dd	CBInst02
		dd	CBInst03
		dd	CBInst04
		dd	CBInst05
		dd	CBInst06
		dd	CBInst07
		dd	CBInst08
		dd	CBInst09
		dd	CBInst0a
		dd	CBInst0b
		dd	CBInst0c
		dd	CBInst0d
		dd	CBInst0e
		dd	CBInst0f
		dd	CBInst10
		dd	CBInst11
		dd	CBInst12
		dd	CBInst13
		dd	CBInst14
		dd	CBInst15
		dd	CBInst16
		dd	CBInst17
		dd	CBInst18
		dd	CBInst19
		dd	CBInst1a
		dd	CBInst1b
		dd	CBInst1c
		dd	CBInst1d
		dd	CBInst1e
		dd	CBInst1f
		dd	CBInst20
		dd	CBInst21
		dd	CBInst22
		dd	CBInst23
		dd	CBInst24
		dd	CBInst25
		dd	CBInst26
		dd	CBInst27
		dd	CBInst28
		dd	CBInst29
		dd	CBInst2a
		dd	CBInst2b
		dd	CBInst2c
		dd	CBInst2d
		dd	CBInst2e
		dd	CBInst2f
		dd	CBInst30
		dd	CBInst31
		dd	CBInst32
		dd	CBInst33
		dd	CBInst34
		dd	CBInst35
		dd	CBInst36
		dd	CBInst37
		dd	CBInst38
		dd	CBInst39
		dd	CBInst3a
		dd	CBInst3b
		dd	CBInst3c
		dd	CBInst3d
		dd	CBInst3e
		dd	CBInst3f
		dd	CBInst40
		dd	CBInst41
		dd	CBInst42
		dd	CBInst43
		dd	CBInst44
		dd	CBInst45
		dd	CBInst46
		dd	CBInst47
		dd	CBInst48
		dd	CBInst49
		dd	CBInst4a
		dd	CBInst4b
		dd	CBInst4c
		dd	CBInst4d
		dd	CBInst4e
		dd	CBInst4f
		dd	CBInst50
		dd	CBInst51
		dd	CBInst52
		dd	CBInst53
		dd	CBInst54
		dd	CBInst55
		dd	CBInst56
		dd	CBInst57
		dd	CBInst58
		dd	CBInst59
		dd	CBInst5a
		dd	CBInst5b
		dd	CBInst5c
		dd	CBInst5d
		dd	CBInst5e
		dd	CBInst5f
		dd	CBInst60
		dd	CBInst61
		dd	CBInst62
		dd	CBInst63
		dd	CBInst64
		dd	CBInst65
		dd	CBInst66
		dd	CBInst67
		dd	CBInst68
		dd	CBInst69
		dd	CBInst6a
		dd	CBInst6b
		dd	CBInst6c
		dd	CBInst6d
		dd	CBInst6e
		dd	CBInst6f
		dd	CBInst70
		dd	CBInst71
		dd	CBInst72
		dd	CBInst73
		dd	CBInst74
		dd	CBInst75
		dd	CBInst76
		dd	CBInst77
		dd	CBInst78
		dd	CBInst79
		dd	CBInst7a
		dd	CBInst7b
		dd	CBInst7c
		dd	CBInst7d
		dd	CBInst7e
		dd	CBInst7f
		dd	CBInst80
		dd	CBInst81
		dd	CBInst82
		dd	CBInst83
		dd	CBInst84
		dd	CBInst85
		dd	CBInst86
		dd	CBInst87
		dd	CBInst88
		dd	CBInst89
		dd	CBInst8a
		dd	CBInst8b
		dd	CBInst8c
		dd	CBInst8d
		dd	CBInst8e
		dd	CBInst8f
		dd	CBInst90
		dd	CBInst91
		dd	CBInst92
		dd	CBInst93
		dd	CBInst94
		dd	CBInst95
		dd	CBInst96
		dd	CBInst97
		dd	CBInst98
		dd	CBInst99
		dd	CBInst9a
		dd	CBInst9b
		dd	CBInst9c
		dd	CBInst9d
		dd	CBInst9e
		dd	CBInst9f
		dd	CBInsta0
		dd	CBInsta1
		dd	CBInsta2
		dd	CBInsta3
		dd	CBInsta4
		dd	CBInsta5
		dd	CBInsta6
		dd	CBInsta7
		dd	CBInsta8
		dd	CBInsta9
		dd	CBInstaa
		dd	CBInstab
		dd	CBInstac
		dd	CBInstad
		dd	CBInstae
		dd	CBInstaf
		dd	CBInstb0
		dd	CBInstb1
		dd	CBInstb2
		dd	CBInstb3
		dd	CBInstb4
		dd	CBInstb5
		dd	CBInstb6
		dd	CBInstb7
		dd	CBInstb8
		dd	CBInstb9
		dd	CBInstba
		dd	CBInstbb
		dd	CBInstbc
		dd	CBInstbd
		dd	CBInstbe
		dd	CBInstbf
		dd	CBInstc0
		dd	CBInstc1
		dd	CBInstc2
		dd	CBInstc3
		dd	CBInstc4
		dd	CBInstc5
		dd	CBInstc6
		dd	CBInstc7
		dd	CBInstc8
		dd	CBInstc9
		dd	CBInstca
		dd	CBInstcb
		dd	CBInstcc
		dd	CBInstcd
		dd	CBInstce
		dd	CBInstcf
		dd	CBInstd0
		dd	CBInstd1
		dd	CBInstd2
		dd	CBInstd3
		dd	CBInstd4
		dd	CBInstd5
		dd	CBInstd6
		dd	CBInstd7
		dd	CBInstd8
		dd	CBInstd9
		dd	CBInstda
		dd	CBInstdb
		dd	CBInstdc
		dd	CBInstdd
		dd	CBInstde
		dd	CBInstdf
		dd	CBInste0
		dd	CBInste1
		dd	CBInste2
		dd	CBInste3
		dd	CBInste4
		dd	CBInste5
		dd	CBInste6
		dd	CBInste7
		dd	CBInste8
		dd	CBInste9
		dd	CBInstea
		dd	CBInsteb
		dd	CBInstec
		dd	CBInsted
		dd	CBInstee
		dd	CBInstef
		dd	CBInstf0
		dd	CBInstf1
		dd	CBInstf2
		dd	CBInstf3
		dd	CBInstf4
		dd	CBInstf5
		dd	CBInstf6
		dd	CBInstf7
		dd	CBInstf8
		dd	CBInstf9
		dd	CBInstfa
		dd	CBInstfb
		dd	CBInstfc
		dd	CBInstfd
		dd	CBInstfe
		dd	CBInstff

z80PrefixED:
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	EDInst40
		dd	EDInst41
		dd	EDInst42
		dd	EDInst43
		dd	EDInst44
		dd	EDInst45
		dd	EDInst46
		dd	EDInst47
		dd	EDInst48
		dd	EDInst49
		dd	EDInst4a
		dd	EDInst4b
		dd	invalidInsWord
		dd	EDInst4d
		dd	invalidInsWord
		dd	EDInst4f
		dd	EDInst50
		dd	EDInst51
		dd	EDInst52
		dd	EDInst53
		dd	invalidInsWord
		dd	invalidInsWord
		dd	EDInst56
		dd	EDInst57
		dd	EDInst58
		dd	EDInst59
		dd	EDInst5a
		dd	EDInst5b
		dd	invalidInsWord
		dd	invalidInsWord
		dd	EDInst5e
		dd	EDInst5f
		dd	EDInst60
		dd	EDInst61
		dd	EDInst62
		dd	EDInst63
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	EDInst67
		dd	EDInst68
		dd	EDInst69
		dd	EDInst6a
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	EDInst6f
		dd	invalidInsWord
		dd	invalidInsWord
		dd	EDInst72
		dd	EDInst73
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	EDInst78
		dd	EDInst79
		dd	EDInst7a
		dd	EDInst7b
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	EDInsta0
		dd	EDInsta1
		dd	EDInsta2
		dd	EDInsta3
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	EDInsta8
		dd	EDInsta9
		dd	EDInstaa
		dd	EDInstab
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	EDInstb0
		dd	EDInstb1
		dd	EDInstb2
		dd	EDInstb3
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	EDInstb8
		dd	EDInstb9
		dd	EDInstba
		dd	EDInstbb
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord

z80PrefixDD:
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDInst09
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDInst19
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDInst21
		dd	DDInst22
		dd	DDInst23
		dd	DDInst24
		dd	DDInst25
		dd	DDInst26
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDInst29
		dd	DDInst2a
		dd	DDInst2b
		dd	DDInst2c
		dd	DDInst2d
		dd	DDInst2e
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDInst34
		dd	DDInst35
		dd	DDInst36
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDInst39
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDInst44
		dd	DDInst45
		dd	DDInst46
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDInst4c
		dd	DDInst4d
		dd	DDInst4e
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDInst54
		dd	DDInst55
		dd	DDInst56
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDInst5c
		dd	DDInst5d
		dd	DDInst5e
		dd	invalidInsWord
		dd	DDInst60
		dd	DDInst61
		dd	DDInst62
		dd	DDInst63
		dd	DDInst64
		dd	DDInst65
		dd	DDInst66
		dd	DDInst67
		dd	DDInst68
		dd	DDInst69
		dd	DDInst6a
		dd	DDInst6b
		dd	DDInst6c
		dd	DDInst6d
		dd	DDInst6e
		dd	DDInst6f
		dd	DDInst70
		dd	DDInst71
		dd	DDInst72
		dd	DDInst73
		dd	DDInst74
		dd	DDInst75
		dd	invalidInsWord
		dd	DDInst77
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDInst7c
		dd	DDInst7d
		dd	DDInst7e
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDInst84
		dd	DDInst85
		dd	DDInst86
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDInst8c
		dd	DDInst8d
		dd	DDInst8e
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDInst94
		dd	DDInst95
		dd	DDInst96
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDInst9c
		dd	DDInst9d
		dd	DDInst9e
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDInsta4
		dd	DDInsta5
		dd	DDInsta6
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDInstac
		dd	DDInstad
		dd	DDInstae
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDInstb4
		dd	DDInstb5
		dd	DDInstb6
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDInstbc
		dd	DDInstbd
		dd	DDInstbe
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDInstcb
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDInste1
		dd	invalidInsWord
		dd	DDInste3
		dd	invalidInsWord
		dd	DDInste5
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDInste9
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDInstf9
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord

z80PrefixFD:
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	FDInst09
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	FDInst19
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	FDInst21
		dd	FDInst22
		dd	FDInst23
		dd	FDInst24
		dd	FDInst25
		dd	FDInst26
		dd	invalidInsWord
		dd	invalidInsWord
		dd	FDInst29
		dd	FDInst2a
		dd	FDInst2b
		dd	FDInst2c
		dd	FDInst2d
		dd	FDInst2e
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	FDInst34
		dd	FDInst35
		dd	FDInst36
		dd	invalidInsWord
		dd	invalidInsWord
		dd	FDInst39
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	FDInst44
		dd	FDInst45
		dd	FDInst46
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	FDInst4c
		dd	FDInst4d
		dd	FDInst4e
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	FDInst54
		dd	FDInst55
		dd	FDInst56
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	FDInst5c
		dd	FDInst5d
		dd	FDInst5e
		dd	invalidInsWord
		dd	FDInst60
		dd	FDInst61
		dd	FDInst62
		dd	FDInst63
		dd	FDInst64
		dd	FDInst65
		dd	FDInst66
		dd	FDInst67
		dd	FDInst68
		dd	FDInst69
		dd	FDInst6a
		dd	FDInst6b
		dd	FDInst6c
		dd	FDInst6d
		dd	FDInst6e
		dd	FDInst6f
		dd	FDInst70
		dd	FDInst71
		dd	FDInst72
		dd	FDInst73
		dd	FDInst74
		dd	FDInst75
		dd	invalidInsWord
		dd	FDInst77
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	FDInst7c
		dd	FDInst7d
		dd	FDInst7e
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	FDInst84
		dd	FDInst85
		dd	FDInst86
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	FDInst8c
		dd	FDInst8d
		dd	FDInst8e
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	FDInst94
		dd	FDInst95
		dd	FDInst96
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	FDInst9c
		dd	FDInst9d
		dd	FDInst9e
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	FDInsta4
		dd	FDInsta5
		dd	FDInsta6
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	FDInstac
		dd	FDInstad
		dd	FDInstae
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	FDInstb4
		dd	FDInstb5
		dd	FDInstb6
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	FDInstbc
		dd	FDInstbd
		dd	FDInstbe
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	FDInstcb
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	FDInste1
		dd	invalidInsWord
		dd	FDInste3
		dd	invalidInsWord
		dd	FDInste5
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	FDInste9
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	FDInstf9
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
z80ddfdcbInstructions:
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInst06
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInst0e
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInst16
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInst1e
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInst26
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInst2e
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInst3e
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInst46
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInst4e
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInst56
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInst5e
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInst66
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInst6e
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInst76
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInst7e
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInst86
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInst8e
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInst96
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInst9e
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInsta6
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInstae
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInstb6
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInstbe
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInstc6
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInstce
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInstd6
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInstde
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInste6
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInstee
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInstf6
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	invalidInsWord
		dd	DDFDCBInstfe
		dd	invalidInsWord

		section	.text use32 flat class=code

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst00:
		sahf
		rol	ch, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	ch, ch
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst01:
		sahf
		rol	cl, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	cl, cl
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst02:
		mov	dx, [_z80de]	; Move DE into something half usable
		sahf
		rol	dh, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	dh, dh
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst03:
		mov	dx, [_z80de]	; Move DE into something half usable
		sahf
		rol	dl, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	dl, dl
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst04:
		sahf
		rol	bh, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	bh, bh
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst05:
		sahf
		rol	bl, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	bl, bl
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst06:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop0:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead0
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr0		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine0

nextAddr0:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop0

callRoutine0:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit0

memoryRead0:
		mov	dl, [ebp + ebx]	; Get our data

readExit0:
		mov	edi, [cyclesRemaining]
		sahf
		rol	dl, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	dl, dl
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop1:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite1	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr1	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine1	; If not, go call it!

nextAddr1:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop1

callRoutine1:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit1
memoryWrite1:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit1:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst07:
		sahf
		rol	al, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	al, al
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst08:
		sahf
		ror	ch, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	ch, ch
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst09:
		sahf
		ror	cl, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	cl, cl
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst0a:
		mov	dx, [_z80de]	; Move DE into something half usable
		sahf
		ror	dh, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	dh, dh
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst0b:
		mov	dx, [_z80de]	; Move DE into something half usable
		sahf
		ror	dl, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	dl, dl
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst0c:
		sahf
		ror	bh, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	bh, bh
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst0d:
		sahf
		ror	bl, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	bl, bl
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst0e:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop2:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead2
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr2		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine2

nextAddr2:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop2

callRoutine2:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit2

memoryRead2:
		mov	dl, [ebp + ebx]	; Get our data

readExit2:
		mov	edi, [cyclesRemaining]
		sahf
		ror	dl, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	dl, dl
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop3:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite3	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr3	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine3	; If not, go call it!

nextAddr3:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop3

callRoutine3:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit3
memoryWrite3:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit3:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst0f:
		sahf
		ror	al, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	al, al
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst10:
		sahf
		rcl	ch, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	ch, ch
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst11:
		sahf
		rcl	cl, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	cl, cl
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst12:
		mov	dx, [_z80de]	; Move DE into something half usable
		sahf
		rcl	dh, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	dh, dh
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst13:
		mov	dx, [_z80de]	; Move DE into something half usable
		sahf
		rcl	dl, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	dl, dl
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst14:
		sahf
		rcl	bh, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	bh, bh
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst15:
		sahf
		rcl	bl, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	bl, bl
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst16:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop4:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead4
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr4		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine4

nextAddr4:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop4

callRoutine4:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit4

memoryRead4:
		mov	dl, [ebp + ebx]	; Get our data

readExit4:
		mov	edi, [cyclesRemaining]
		sahf
		rcl	dl, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	dl, dl
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop5:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite5	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr5	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine5	; If not, go call it!

nextAddr5:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop5

callRoutine5:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit5
memoryWrite5:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit5:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst17:
		sahf
		rcl	al, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	al, al
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst18:
		sahf
		rcr	ch, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	ch, ch
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst19:
		sahf
		rcr	cl, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	cl, cl
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst1a:
		mov	dx, [_z80de]	; Move DE into something half usable
		sahf
		rcr	dh, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	dh, dh
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst1b:
		mov	dx, [_z80de]	; Move DE into something half usable
		sahf
		rcr	dl, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	dl, dl
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst1c:
		sahf
		rcr	bh, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	bh, bh
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst1d:
		sahf
		rcr	bl, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	bl, bl
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst1e:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop6:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead6
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr6		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine6

nextAddr6:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop6

callRoutine6:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit6

memoryRead6:
		mov	dl, [ebp + ebx]	; Get our data

readExit6:
		mov	edi, [cyclesRemaining]
		sahf
		rcr	dl, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	dl, dl
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop7:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite7	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr7	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine7	; If not, go call it!

nextAddr7:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop7

callRoutine7:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit7
memoryWrite7:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit7:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst1f:
		sahf
		rcr	al, 1
		lahf
		and	ah, 029h	; Clear H and N
		mov	byte [_z80af], ah
		or	al, al
		lahf
		and	ah, 0c4h	; Sign, zero, and parity
		or	ah, byte [_z80af]
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst20:
		sahf
		shl	ch, 1
		lahf
		and	ah, 0edh	; Clear H and N
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst21:
		sahf
		shl	cl, 1
		lahf
		and	ah, 0edh	; Clear H and N
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst22:
		mov	dx, [_z80de]	; Move DE into something half usable
		sahf
		shl	dh, 1
		lahf
		and	ah, 0edh	; Clear H and N
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst23:
		mov	dx, [_z80de]	; Move DE into something half usable
		sahf
		shl	dl, 1
		lahf
		and	ah, 0edh	; Clear H and N
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst24:
		sahf
		shl	bh, 1
		lahf
		and	ah, 0edh	; Clear H and N
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst25:
		sahf
		shl	bl, 1
		lahf
		and	ah, 0edh	; Clear H and N
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst26:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop8:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead8
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr8		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine8

nextAddr8:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop8

callRoutine8:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit8

memoryRead8:
		mov	dl, [ebp + ebx]	; Get our data

readExit8:
		mov	edi, [cyclesRemaining]
		sahf
		shl	dl, 1
		lahf
		and	ah, 0edh	; Clear H and N
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop9:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite9	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr9	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine9	; If not, go call it!

nextAddr9:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop9

callRoutine9:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit9
memoryWrite9:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit9:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst27:
		sahf
		shl	al, 1
		lahf
		and	ah, 0edh	; Clear H and N
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst28:
		sahf
		sar	ch, 1
		lahf
		and	ah, 0edh	; Clear H and N
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst29:
		sahf
		sar	cl, 1
		lahf
		and	ah, 0edh	; Clear H and N
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst2a:
		mov	dx, [_z80de]	; Move DE into something half usable
		sahf
		sar	dh, 1
		lahf
		and	ah, 0edh	; Clear H and N
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst2b:
		mov	dx, [_z80de]	; Move DE into something half usable
		sahf
		sar	dl, 1
		lahf
		and	ah, 0edh	; Clear H and N
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst2c:
		sahf
		sar	bh, 1
		lahf
		and	ah, 0edh	; Clear H and N
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst2d:
		sahf
		sar	bl, 1
		lahf
		and	ah, 0edh	; Clear H and N
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst2e:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop10:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead10
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr10		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine10

nextAddr10:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop10

callRoutine10:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit10

memoryRead10:
		mov	dl, [ebp + ebx]	; Get our data

readExit10:
		mov	edi, [cyclesRemaining]
		sahf
		sar	dl, 1
		lahf
		and	ah, 0edh	; Clear H and N
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop11:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite11	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr11	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine11	; If not, go call it!

nextAddr11:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop11

callRoutine11:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit11
memoryWrite11:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit11:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst2f:
		sahf
		sar	al, 1
		lahf
		and	ah, 0edh	; Clear H and N
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst30:
		sahf
		shl	ch, 1
		lahf
		or	ch, 1	; Slide in a 1 bit (SLIA)
		and	ah, 0edh	; Clear H and N
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst31:
		sahf
		shl	cl, 1
		lahf
		or	cl, 1	; Slide in a 1 bit (SLIA)
		and	ah, 0edh	; Clear H and N
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst32:
		mov	dx, [_z80de]	; Move DE into something half usable
		sahf
		shl	dh, 1
		lahf
		or	dh, 1	; Slide in a 1 bit (SLIA)
		and	ah, 0edh	; Clear H and N
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst33:
		mov	dx, [_z80de]	; Move DE into something half usable
		sahf
		shl	dl, 1
		lahf
		or	dl, 1	; Slide in a 1 bit (SLIA)
		and	ah, 0edh	; Clear H and N
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst34:
		sahf
		shl	bh, 1
		lahf
		or	bh, 1	; Slide in a 1 bit (SLIA)
		and	ah, 0edh	; Clear H and N
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst35:
		sahf
		shl	bl, 1
		lahf
		or	bl, 1	; Slide in a 1 bit (SLIA)
		and	ah, 0edh	; Clear H and N
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst36:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop12:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead12
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr12		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine12

nextAddr12:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop12

callRoutine12:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit12

memoryRead12:
		mov	dl, [ebp + ebx]	; Get our data

readExit12:
		mov	edi, [cyclesRemaining]
		sahf
		shl	dl, 1
		lahf
		or	dl, 1	; Slide in a 1 bit (SLIA)
		and	ah, 0edh	; Clear H and N
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop13:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite13	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr13	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine13	; If not, go call it!

nextAddr13:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop13

callRoutine13:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit13
memoryWrite13:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit13:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst37:
		sahf
		shl	al, 1
		lahf
		or	al, 1	; Slide in a 1 bit (SLIA)
		and	ah, 0edh	; Clear H and N
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst38:
		sahf
		shr	ch, 1
		lahf
		and	ah, 0edh	; Clear H and N
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst39:
		sahf
		shr	cl, 1
		lahf
		and	ah, 0edh	; Clear H and N
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst3a:
		mov	dx, [_z80de]	; Move DE into something half usable
		sahf
		shr	dh, 1
		lahf
		and	ah, 0edh	; Clear H and N
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst3b:
		mov	dx, [_z80de]	; Move DE into something half usable
		sahf
		shr	dl, 1
		lahf
		and	ah, 0edh	; Clear H and N
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst3c:
		sahf
		shr	bh, 1
		lahf
		and	ah, 0edh	; Clear H and N
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst3d:
		sahf
		shr	bl, 1
		lahf
		and	ah, 0edh	; Clear H and N
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst3e:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop14:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead14
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr14		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine14

nextAddr14:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop14

callRoutine14:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit14

memoryRead14:
		mov	dl, [ebp + ebx]	; Get our data

readExit14:
		mov	edi, [cyclesRemaining]
		sahf
		shr	dl, 1
		lahf
		and	ah, 0edh	; Clear H and N
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop15:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite15	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr15	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine15	; If not, go call it!

nextAddr15:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop15

callRoutine15:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit15
memoryWrite15:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit15:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst3f:
		sahf
		shr	al, 1
		lahf
		and	ah, 0edh	; Clear H and N
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst40:
		mov	byte [_z80af], ah ; Store F
		sahf
		test ch, 001h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst41:
		mov	byte [_z80af], ah ; Store F
		sahf
		test cl, 001h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst42:
		mov	byte [_z80af], ah ; Store F
		sahf
		test byte [_z80de + 1], 001h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst43:
		mov	byte [_z80af], ah ; Store F
		sahf
		test byte [_z80de], 001h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst44:
		mov	byte [_z80af], ah ; Store F
		sahf
		test bh, 001h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst45:
		mov	byte [_z80af], ah ; Store F
		sahf
		test bl, 001h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst46:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop16:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead16
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr16		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine16

nextAddr16:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop16

callRoutine16:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit16

memoryRead16:
		mov	dl, [ebp + ebx]	; Get our data

readExit16:
		mov	edi, [cyclesRemaining]
		mov	byte [_z80af], ah ; Store F
		sahf
		test	dl, 001h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		xor	edx, edx
		sub	edi, byte 12
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst47:
		mov	byte [_z80af], ah ; Store F
		sahf
		test al, 001h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst48:
		mov	byte [_z80af], ah ; Store F
		sahf
		test ch, 002h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst49:
		mov	byte [_z80af], ah ; Store F
		sahf
		test cl, 002h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst4a:
		mov	byte [_z80af], ah ; Store F
		sahf
		test byte [_z80de + 1], 002h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst4b:
		mov	byte [_z80af], ah ; Store F
		sahf
		test byte [_z80de], 002h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst4c:
		mov	byte [_z80af], ah ; Store F
		sahf
		test bh, 002h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst4d:
		mov	byte [_z80af], ah ; Store F
		sahf
		test bl, 002h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst4e:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop17:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead17
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr17		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine17

nextAddr17:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop17

callRoutine17:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit17

memoryRead17:
		mov	dl, [ebp + ebx]	; Get our data

readExit17:
		mov	edi, [cyclesRemaining]
		mov	byte [_z80af], ah ; Store F
		sahf
		test	dl, 002h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		xor	edx, edx
		sub	edi, byte 12
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst4f:
		mov	byte [_z80af], ah ; Store F
		sahf
		test al, 002h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst50:
		mov	byte [_z80af], ah ; Store F
		sahf
		test ch, 004h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst51:
		mov	byte [_z80af], ah ; Store F
		sahf
		test cl, 004h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst52:
		mov	byte [_z80af], ah ; Store F
		sahf
		test byte [_z80de + 1], 004h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst53:
		mov	byte [_z80af], ah ; Store F
		sahf
		test byte [_z80de], 004h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst54:
		mov	byte [_z80af], ah ; Store F
		sahf
		test bh, 004h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst55:
		mov	byte [_z80af], ah ; Store F
		sahf
		test bl, 004h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst56:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop18:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead18
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr18		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine18

nextAddr18:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop18

callRoutine18:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit18

memoryRead18:
		mov	dl, [ebp + ebx]	; Get our data

readExit18:
		mov	edi, [cyclesRemaining]
		mov	byte [_z80af], ah ; Store F
		sahf
		test	dl, 004h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		xor	edx, edx
		sub	edi, byte 12
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst57:
		mov	byte [_z80af], ah ; Store F
		sahf
		test al, 004h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst58:
		mov	byte [_z80af], ah ; Store F
		sahf
		test ch, 008h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst59:
		mov	byte [_z80af], ah ; Store F
		sahf
		test cl, 008h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst5a:
		mov	byte [_z80af], ah ; Store F
		sahf
		test byte [_z80de + 1], 008h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst5b:
		mov	byte [_z80af], ah ; Store F
		sahf
		test byte [_z80de], 008h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst5c:
		mov	byte [_z80af], ah ; Store F
		sahf
		test bh, 008h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst5d:
		mov	byte [_z80af], ah ; Store F
		sahf
		test bl, 008h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst5e:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop19:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead19
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr19		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine19

nextAddr19:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop19

callRoutine19:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit19

memoryRead19:
		mov	dl, [ebp + ebx]	; Get our data

readExit19:
		mov	edi, [cyclesRemaining]
		mov	byte [_z80af], ah ; Store F
		sahf
		test	dl, 008h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		xor	edx, edx
		sub	edi, byte 12
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst5f:
		mov	byte [_z80af], ah ; Store F
		sahf
		test al, 008h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst60:
		mov	byte [_z80af], ah ; Store F
		sahf
		test ch, 010h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst61:
		mov	byte [_z80af], ah ; Store F
		sahf
		test cl, 010h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst62:
		mov	byte [_z80af], ah ; Store F
		sahf
		test byte [_z80de + 1], 010h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst63:
		mov	byte [_z80af], ah ; Store F
		sahf
		test byte [_z80de], 010h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst64:
		mov	byte [_z80af], ah ; Store F
		sahf
		test bh, 010h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst65:
		mov	byte [_z80af], ah ; Store F
		sahf
		test bl, 010h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst66:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop20:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead20
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr20		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine20

nextAddr20:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop20

callRoutine20:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit20

memoryRead20:
		mov	dl, [ebp + ebx]	; Get our data

readExit20:
		mov	edi, [cyclesRemaining]
		mov	byte [_z80af], ah ; Store F
		sahf
		test	dl, 010h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		xor	edx, edx
		sub	edi, byte 12
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst67:
		mov	byte [_z80af], ah ; Store F
		sahf
		test al, 010h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst68:
		mov	byte [_z80af], ah ; Store F
		sahf
		test ch, 020h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst69:
		mov	byte [_z80af], ah ; Store F
		sahf
		test cl, 020h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst6a:
		mov	byte [_z80af], ah ; Store F
		sahf
		test byte [_z80de + 1], 020h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst6b:
		mov	byte [_z80af], ah ; Store F
		sahf
		test byte [_z80de], 020h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst6c:
		mov	byte [_z80af], ah ; Store F
		sahf
		test bh, 020h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst6d:
		mov	byte [_z80af], ah ; Store F
		sahf
		test bl, 020h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst6e:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop21:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead21
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr21		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine21

nextAddr21:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop21

callRoutine21:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit21

memoryRead21:
		mov	dl, [ebp + ebx]	; Get our data

readExit21:
		mov	edi, [cyclesRemaining]
		mov	byte [_z80af], ah ; Store F
		sahf
		test	dl, 020h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		xor	edx, edx
		sub	edi, byte 12
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst6f:
		mov	byte [_z80af], ah ; Store F
		sahf
		test al, 020h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst70:
		mov	byte [_z80af], ah ; Store F
		sahf
		test ch, 040h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst71:
		mov	byte [_z80af], ah ; Store F
		sahf
		test cl, 040h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst72:
		mov	byte [_z80af], ah ; Store F
		sahf
		test byte [_z80de + 1], 040h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst73:
		mov	byte [_z80af], ah ; Store F
		sahf
		test byte [_z80de], 040h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst74:
		mov	byte [_z80af], ah ; Store F
		sahf
		test bh, 040h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst75:
		mov	byte [_z80af], ah ; Store F
		sahf
		test bl, 040h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst76:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop22:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead22
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr22		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine22

nextAddr22:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop22

callRoutine22:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit22

memoryRead22:
		mov	dl, [ebp + ebx]	; Get our data

readExit22:
		mov	edi, [cyclesRemaining]
		mov	byte [_z80af], ah ; Store F
		sahf
		test	dl, 040h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		xor	edx, edx
		sub	edi, byte 12
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst77:
		mov	byte [_z80af], ah ; Store F
		sahf
		test al, 040h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst78:
		mov	byte [_z80af], ah ; Store F
		sahf
		test ch, 080h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst79:
		mov	byte [_z80af], ah ; Store F
		sahf
		test cl, 080h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst7a:
		mov	byte [_z80af], ah ; Store F
		sahf
		test byte [_z80de + 1], 080h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst7b:
		mov	byte [_z80af], ah ; Store F
		sahf
		test byte [_z80de], 080h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst7c:
		mov	byte [_z80af], ah ; Store F
		sahf
		test bh, 080h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst7d:
		mov	byte [_z80af], ah ; Store F
		sahf
		test bl, 080h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst7e:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop23:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead23
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr23		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine23

nextAddr23:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop23

callRoutine23:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit23

memoryRead23:
		mov	dl, [ebp + ebx]	; Get our data

readExit23:
		mov	edi, [cyclesRemaining]
		mov	byte [_z80af], ah ; Store F
		sahf
		test	dl, 080h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		xor	edx, edx
		sub	edi, byte 12
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst7f:
		mov	byte [_z80af], ah ; Store F
		sahf
		test al, 080h	; Do a bitwise check
		lahf
		and	ah, 0c0h	; Only care about Z and S
		or	ah, 10h	; Set half carry to 1
		and	byte [_z80af], 029h		; Only zero/non-zero!
		or	ah, byte [_z80af]	; Put it in with the real flags
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst80:
		and ch, 0feh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst81:
		and cl, 0feh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst82:
		mov	dx, [_z80de]	; Move DE into something half usable
		and dh, 0feh	; Reset a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst83:
		mov	dx, [_z80de]	; Move DE into something half usable
		and dl, 0feh	; Reset a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst84:
		and bh, 0feh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst85:
		and bl, 0feh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst86:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop24:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead24
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr24		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine24

nextAddr24:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop24

callRoutine24:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit24

memoryRead24:
		mov	dl, [ebp + ebx]	; Get our data

readExit24:
		mov	edi, [cyclesRemaining]
		and dl, 0feh	; Reset a bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop25:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite25	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr25	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine25	; If not, go call it!

nextAddr25:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop25

callRoutine25:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit25
memoryWrite25:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit25:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst87:
		and al, 0feh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst88:
		and ch, 0fdh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst89:
		and cl, 0fdh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst8a:
		mov	dx, [_z80de]	; Move DE into something half usable
		and dh, 0fdh	; Reset a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst8b:
		mov	dx, [_z80de]	; Move DE into something half usable
		and dl, 0fdh	; Reset a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst8c:
		and bh, 0fdh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst8d:
		and bl, 0fdh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst8e:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop26:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead26
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr26		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine26

nextAddr26:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop26

callRoutine26:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit26

memoryRead26:
		mov	dl, [ebp + ebx]	; Get our data

readExit26:
		mov	edi, [cyclesRemaining]
		and dl, 0fdh	; Reset a bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop27:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite27	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr27	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine27	; If not, go call it!

nextAddr27:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop27

callRoutine27:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit27
memoryWrite27:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit27:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst8f:
		and al, 0fdh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst90:
		and ch, 0fbh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst91:
		and cl, 0fbh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst92:
		mov	dx, [_z80de]	; Move DE into something half usable
		and dh, 0fbh	; Reset a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst93:
		mov	dx, [_z80de]	; Move DE into something half usable
		and dl, 0fbh	; Reset a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst94:
		and bh, 0fbh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst95:
		and bl, 0fbh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst96:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop28:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead28
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr28		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine28

nextAddr28:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop28

callRoutine28:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit28

memoryRead28:
		mov	dl, [ebp + ebx]	; Get our data

readExit28:
		mov	edi, [cyclesRemaining]
		and dl, 0fbh	; Reset a bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop29:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite29	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr29	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine29	; If not, go call it!

nextAddr29:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop29

callRoutine29:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit29
memoryWrite29:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit29:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst97:
		and al, 0fbh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst98:
		and ch, 0f7h	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst99:
		and cl, 0f7h	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst9a:
		mov	dx, [_z80de]	; Move DE into something half usable
		and dh, 0f7h	; Reset a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst9b:
		mov	dx, [_z80de]	; Move DE into something half usable
		and dl, 0f7h	; Reset a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst9c:
		and bh, 0f7h	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst9d:
		and bl, 0f7h	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst9e:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop30:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead30
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr30		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine30

nextAddr30:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop30

callRoutine30:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit30

memoryRead30:
		mov	dl, [ebp + ebx]	; Get our data

readExit30:
		mov	edi, [cyclesRemaining]
		and dl, 0f7h	; Reset a bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop31:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite31	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr31	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine31	; If not, go call it!

nextAddr31:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop31

callRoutine31:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit31
memoryWrite31:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit31:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInst9f:
		and al, 0f7h	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInsta0:
		and ch, 0efh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInsta1:
		and cl, 0efh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInsta2:
		mov	dx, [_z80de]	; Move DE into something half usable
		and dh, 0efh	; Reset a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInsta3:
		mov	dx, [_z80de]	; Move DE into something half usable
		and dl, 0efh	; Reset a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInsta4:
		and bh, 0efh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInsta5:
		and bl, 0efh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInsta6:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop32:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead32
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr32		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine32

nextAddr32:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop32

callRoutine32:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit32

memoryRead32:
		mov	dl, [ebp + ebx]	; Get our data

readExit32:
		mov	edi, [cyclesRemaining]
		and dl, 0efh	; Reset a bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop33:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite33	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr33	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine33	; If not, go call it!

nextAddr33:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop33

callRoutine33:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit33
memoryWrite33:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit33:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInsta7:
		and al, 0efh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInsta8:
		and ch, 0dfh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInsta9:
		and cl, 0dfh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstaa:
		mov	dx, [_z80de]	; Move DE into something half usable
		and dh, 0dfh	; Reset a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstab:
		mov	dx, [_z80de]	; Move DE into something half usable
		and dl, 0dfh	; Reset a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstac:
		and bh, 0dfh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstad:
		and bl, 0dfh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstae:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop34:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead34
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr34		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine34

nextAddr34:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop34

callRoutine34:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit34

memoryRead34:
		mov	dl, [ebp + ebx]	; Get our data

readExit34:
		mov	edi, [cyclesRemaining]
		and dl, 0dfh	; Reset a bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop35:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite35	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr35	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine35	; If not, go call it!

nextAddr35:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop35

callRoutine35:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit35
memoryWrite35:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit35:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstaf:
		and al, 0dfh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstb0:
		and ch, 0bfh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstb1:
		and cl, 0bfh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstb2:
		mov	dx, [_z80de]	; Move DE into something half usable
		and dh, 0bfh	; Reset a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstb3:
		mov	dx, [_z80de]	; Move DE into something half usable
		and dl, 0bfh	; Reset a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstb4:
		and bh, 0bfh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstb5:
		and bl, 0bfh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstb6:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop36:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead36
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr36		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine36

nextAddr36:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop36

callRoutine36:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit36

memoryRead36:
		mov	dl, [ebp + ebx]	; Get our data

readExit36:
		mov	edi, [cyclesRemaining]
		and dl, 0bfh	; Reset a bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop37:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite37	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr37	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine37	; If not, go call it!

nextAddr37:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop37

callRoutine37:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit37
memoryWrite37:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit37:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstb7:
		and al, 0bfh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstb8:
		and ch, 07fh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstb9:
		and cl, 07fh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstba:
		mov	dx, [_z80de]	; Move DE into something half usable
		and dh, 07fh	; Reset a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstbb:
		mov	dx, [_z80de]	; Move DE into something half usable
		and dl, 07fh	; Reset a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstbc:
		and bh, 07fh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstbd:
		and bl, 07fh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstbe:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop38:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead38
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr38		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine38

nextAddr38:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop38

callRoutine38:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit38

memoryRead38:
		mov	dl, [ebp + ebx]	; Get our data

readExit38:
		mov	edi, [cyclesRemaining]
		and dl, 07fh	; Reset a bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop39:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite39	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr39	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine39	; If not, go call it!

nextAddr39:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop39

callRoutine39:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit39
memoryWrite39:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit39:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstbf:
		and al, 07fh	; Reset a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstc0:
		or	ch, 001h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstc1:
		or	cl, 001h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstc2:
		mov	dx, [_z80de]	; Move DE into something half usable
		or	dh, 001h	; Set a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstc3:
		mov	dx, [_z80de]	; Move DE into something half usable
		or	dl, 001h	; Set a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstc4:
		or	bh, 001h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstc5:
		or	bl, 001h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstc6:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop40:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead40
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr40		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine40

nextAddr40:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop40

callRoutine40:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit40

memoryRead40:
		mov	dl, [ebp + ebx]	; Get our data

readExit40:
		mov	edi, [cyclesRemaining]
		or	dl, 001h	; Set a bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop41:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite41	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr41	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine41	; If not, go call it!

nextAddr41:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop41

callRoutine41:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit41
memoryWrite41:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit41:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstc7:
		or	al, 001h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstc8:
		or	ch, 002h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstc9:
		or	cl, 002h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstca:
		mov	dx, [_z80de]	; Move DE into something half usable
		or	dh, 002h	; Set a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstcb:
		mov	dx, [_z80de]	; Move DE into something half usable
		or	dl, 002h	; Set a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstcc:
		or	bh, 002h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstcd:
		or	bl, 002h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstce:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop42:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead42
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr42		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine42

nextAddr42:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop42

callRoutine42:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit42

memoryRead42:
		mov	dl, [ebp + ebx]	; Get our data

readExit42:
		mov	edi, [cyclesRemaining]
		or	dl, 002h	; Set a bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop43:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite43	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr43	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine43	; If not, go call it!

nextAddr43:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop43

callRoutine43:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit43
memoryWrite43:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit43:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstcf:
		or	al, 002h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstd0:
		or	ch, 004h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstd1:
		or	cl, 004h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstd2:
		mov	dx, [_z80de]	; Move DE into something half usable
		or	dh, 004h	; Set a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstd3:
		mov	dx, [_z80de]	; Move DE into something half usable
		or	dl, 004h	; Set a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstd4:
		or	bh, 004h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstd5:
		or	bl, 004h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstd6:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop44:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead44
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr44		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine44

nextAddr44:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop44

callRoutine44:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit44

memoryRead44:
		mov	dl, [ebp + ebx]	; Get our data

readExit44:
		mov	edi, [cyclesRemaining]
		or	dl, 004h	; Set a bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop45:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite45	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr45	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine45	; If not, go call it!

nextAddr45:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop45

callRoutine45:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit45
memoryWrite45:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit45:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstd7:
		or	al, 004h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstd8:
		or	ch, 008h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstd9:
		or	cl, 008h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstda:
		mov	dx, [_z80de]	; Move DE into something half usable
		or	dh, 008h	; Set a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstdb:
		mov	dx, [_z80de]	; Move DE into something half usable
		or	dl, 008h	; Set a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstdc:
		or	bh, 008h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstdd:
		or	bl, 008h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstde:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop46:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead46
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr46		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine46

nextAddr46:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop46

callRoutine46:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit46

memoryRead46:
		mov	dl, [ebp + ebx]	; Get our data

readExit46:
		mov	edi, [cyclesRemaining]
		or	dl, 008h	; Set a bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop47:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite47	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr47	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine47	; If not, go call it!

nextAddr47:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop47

callRoutine47:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit47
memoryWrite47:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit47:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstdf:
		or	al, 008h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInste0:
		or	ch, 010h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInste1:
		or	cl, 010h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInste2:
		mov	dx, [_z80de]	; Move DE into something half usable
		or	dh, 010h	; Set a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInste3:
		mov	dx, [_z80de]	; Move DE into something half usable
		or	dl, 010h	; Set a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInste4:
		or	bh, 010h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInste5:
		or	bl, 010h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInste6:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop48:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead48
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr48		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine48

nextAddr48:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop48

callRoutine48:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit48

memoryRead48:
		mov	dl, [ebp + ebx]	; Get our data

readExit48:
		mov	edi, [cyclesRemaining]
		or	dl, 010h	; Set a bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop49:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite49	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr49	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine49	; If not, go call it!

nextAddr49:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop49

callRoutine49:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit49
memoryWrite49:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit49:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInste7:
		or	al, 010h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInste8:
		or	ch, 020h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInste9:
		or	cl, 020h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstea:
		mov	dx, [_z80de]	; Move DE into something half usable
		or	dh, 020h	; Set a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInsteb:
		mov	dx, [_z80de]	; Move DE into something half usable
		or	dl, 020h	; Set a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstec:
		or	bh, 020h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInsted:
		or	bl, 020h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstee:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop50:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead50
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr50		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine50

nextAddr50:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop50

callRoutine50:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit50

memoryRead50:
		mov	dl, [ebp + ebx]	; Get our data

readExit50:
		mov	edi, [cyclesRemaining]
		or	dl, 020h	; Set a bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop51:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite51	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr51	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine51	; If not, go call it!

nextAddr51:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop51

callRoutine51:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit51
memoryWrite51:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit51:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstef:
		or	al, 020h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstf0:
		or	ch, 040h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstf1:
		or	cl, 040h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstf2:
		mov	dx, [_z80de]	; Move DE into something half usable
		or	dh, 040h	; Set a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstf3:
		mov	dx, [_z80de]	; Move DE into something half usable
		or	dl, 040h	; Set a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstf4:
		or	bh, 040h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstf5:
		or	bl, 040h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstf6:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop52:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead52
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr52		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine52

nextAddr52:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop52

callRoutine52:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit52

memoryRead52:
		mov	dl, [ebp + ebx]	; Get our data

readExit52:
		mov	edi, [cyclesRemaining]
		or	dl, 040h	; Set a bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop53:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite53	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr53	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine53	; If not, go call it!

nextAddr53:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop53

callRoutine53:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit53
memoryWrite53:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit53:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstf7:
		or	al, 040h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstf8:
		or	ch, 080h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstf9:
		or	cl, 080h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstfa:
		mov	dx, [_z80de]	; Move DE into something half usable
		or	dh, 080h	; Set a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstfb:
		mov	dx, [_z80de]	; Move DE into something half usable
		or	dl, 080h	; Set a bit
		mov	[_z80de], dx	; Once modified, put it back
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstfc:
		or	bh, 080h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstfd:
		or	bl, 080h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstfe:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop54:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead54
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr54		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine54

nextAddr54:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop54

callRoutine54:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit54

memoryRead54:
		mov	dl, [ebp + ebx]	; Get our data

readExit54:
		mov	edi, [cyclesRemaining]
		or	dl, 080h	; Set a bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop55:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite55	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr55	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine55	; If not, go call it!

nextAddr55:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop55

callRoutine55:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit55
memoryWrite55:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit55:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

CBInstff:
		or	al, 080h	; Set a bit
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst40:
		mov	dl, cl	; Address in DX... (C)
		mov	[cyclesRemaining], edi
		mov	edi, [_z80IoRead]	; Point to the read array

checkLoop56:
		cmp	[edi], word 0ffffh ; End of the list?
		je		ioRead56
		cmp	dx, [edi]	; Are we smaller?
		jb		nextAddr56		; Yes, go to the next address
		cmp	dx, [edi+2]	; Are we bigger?
		jbe	callRoutine56

nextAddr56:
		add	edi, 0ch		; Next structure!
		jmp	short checkLoop56

callRoutine56:
		call	ReadIOByte	; Standard read routine
		mov	ch, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit56

ioRead56:
		mov	ch, 0ffh	; An unreferenced read
readExit56:
		mov	edi, [cyclesRemaining]
;
; Remember, this variant of the IN instruction modifies the flags
;

		sahf	; Restore our flags
		mov	dh, ah	; Save flags for later
		or	ch, ch;
		lahf
		and	dh, 029h	; Only keep carry and two unused flags
		and	ah, 0d4h
		or	ah, dh
		xor	edx, edx
		sub	edi, byte 12
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst41:
		mov	dl, cl	; Address in DX... (C)
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, ch	; And our data to write
		mov	edi, [_z80IoWrite]	; Point to the I/O write array

checkLoop57:
		cmp	[edi], word 0ffffh ; End of our list?
		je	WriteMacroExit57	; Yes - ignore it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr57	; Yes... go to the next addr
		cmp	dx, [edi+2]	; Are we bigger?
		jbe	callRoutine57	; If not, go call it!

nextAddr57:
		add	edi, 0ch		; Next structure, please
		jmp	short checkLoop57

callRoutine57:
		call	WriteIOByte	; Go write the data!
WriteMacroExit57:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 12
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst42:
		mov	dx, cx	; Get our original register
		mov	[_orgval], dx	; Store this for later half carry computation
		mov	[_orgval2], bx	; Store this, too
		sahf		; Restore our flags
		sbb	bx, dx	; Do the operation!
		lahf		; Get our new flags
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0edh	; Knock out negative & half carry flags
		or	ah, 02h	; Negative!
		mov	[_z80hl], bx
		xor	bx, [_orgval]
		xor	bx, [_orgval2]
		and	bh, 10h	; Half carry?
		or	ah, bh	; OR It in if so
		mov	bx, [_z80hl]
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst43:
		mov	dx, [esi]	; Get our address to write to
		add	esi, 2		; Next address, please...
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, cl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop58:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite58	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr58	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine58	; If not, go call it!

nextAddr58:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop58

callRoutine58:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit58
memoryWrite58:
		mov	[ebp + edx], cl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit58:
		mov	edi, [cyclesRemaining]
		inc	dx
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, ch	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop59:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite59	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr59	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine59	; If not, go call it!

nextAddr59:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop59

callRoutine59:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit59
memoryWrite59:
		mov	[ebp + edx], ch
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit59:
		mov	edi, [cyclesRemaining]
		xor	edx, edx	; Zero our upper word
		sub	edi, byte 20
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst44:
		sahf
		sub	dh, al
		lahf
		mov	al, dh
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst45:
		mov	dx, word [_z80sp]	; Get our current stack pointer
		mov	si, [edx+ebp]	; Get our return address
		and	esi, 0ffffh		; Only within 64K!
		add	esi, ebp			; Add in our base address
		add	word [_z80sp], 02h	; Remove our two bytes from the stack
		xor	edx, edx
		mov	dl, [_z80iff]	; Get interrupt flags
		shr	dl, 1		; Move IFF2->IFF1
		and	[_z80iff], dword (~IFF1)	; Get rid of IFF 1
		and	dl, IFF1	; Just want the IFF 1 value now
		or	dword [_z80iff], edx
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 14
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst46:
		mov	dword [_z80interruptMode], 0 ; IM 0
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst47:
           mov     [_z80i], al
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst48:
		mov	dl, cl	; Address in DX... (C)
		mov	[cyclesRemaining], edi
		mov	edi, [_z80IoRead]	; Point to the read array

checkLoop60:
		cmp	[edi], word 0ffffh ; End of the list?
		je		ioRead60
		cmp	dx, [edi]	; Are we smaller?
		jb		nextAddr60		; Yes, go to the next address
		cmp	dx, [edi+2]	; Are we bigger?
		jbe	callRoutine60

nextAddr60:
		add	edi, 0ch		; Next structure!
		jmp	short checkLoop60

callRoutine60:
		call	ReadIOByte	; Standard read routine
		mov	cl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit60

ioRead60:
		mov	cl, 0ffh	; An unreferenced read
readExit60:
		mov	edi, [cyclesRemaining]
;
; Remember, this variant of the IN instruction modifies the flags
;

		sahf	; Restore our flags
		mov	dh, ah	; Save flags for later
		or	cl, cl;
		lahf
		and	dh, 029h	; Only keep carry and two unused flags
		and	ah, 0d4h
		or	ah, dh
		xor	edx, edx
		sub	edi, byte 12
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst49:
		mov	dl, cl	; Address in DX... (C)
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, cl	; And our data to write
		mov	edi, [_z80IoWrite]	; Point to the I/O write array

checkLoop61:
		cmp	[edi], word 0ffffh ; End of our list?
		je	WriteMacroExit61	; Yes - ignore it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr61	; Yes... go to the next addr
		cmp	dx, [edi+2]	; Are we bigger?
		jbe	callRoutine61	; If not, go call it!

nextAddr61:
		add	edi, 0ch		; Next structure, please
		jmp	short checkLoop61

callRoutine61:
		call	WriteIOByte	; Go write the data!
WriteMacroExit61:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 12
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst4a:
		mov	dx, cx	; Get our original register
		mov	[_orgval], dx	; Store this for later half carry computation
		mov	[_orgval2], bx	; Store this, too
		sahf		; Restore our flags
		adc	bx, dx	; Do the operation!
		lahf		; Get our new flags
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0edh	; Knock out negative & half carry flags
		mov	[_z80hl], bx
		xor	bx, [_orgval]
		xor	bx, [_orgval2]
		and	bh, 10h	; Half carry?
		or	ah, bh	; OR It in if so
		mov	bx, [_z80hl]
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst4b:
		mov	dx, [esi]	; Get address to load
		add	esi, 2	; Skip over it so we don't execute it
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop62:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead62
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr62		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine62

nextAddr62:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop62

callRoutine62:
		call	ReadMemoryByte	; Standard read routine
		mov	cl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit62

memoryRead62:
		mov	cl, [ebp + edx]	; Get our data

readExit62:
		mov	edi, [cyclesRemaining]
		inc	dx
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop63:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead63
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr63		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine63

nextAddr63:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop63

callRoutine63:
		call	ReadMemoryByte	; Standard read routine
		mov	ch, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit63

memoryRead63:
		mov	ch, [ebp + edx]	; Get our data

readExit63:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 20
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst4d:
		mov	dx, word [_z80sp]	; Get our current stack pointer
		mov	si, [edx+ebp]	; Get our return address
		and	esi, 0ffffh		; Only within 64K!
		add	esi, ebp			; Add in our base address
		add	word [_z80sp], 02h	; Remove our two bytes from the stack
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 14
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst4f:
           mov     [_z80r], al
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst50:
		mov	dl, cl	; Address in DX... (C)
		mov	[cyclesRemaining], edi
		mov	edi, [_z80IoRead]	; Point to the read array

checkLoop64:
		cmp	[edi], word 0ffffh ; End of the list?
		je		ioRead64
		cmp	dx, [edi]	; Are we smaller?
		jb		nextAddr64		; Yes, go to the next address
		cmp	dx, [edi+2]	; Are we bigger?
		jbe	callRoutine64

nextAddr64:
		add	edi, 0ch		; Next structure!
		jmp	short checkLoop64

callRoutine64:
		call	ReadIOByte	; Standard read routine
		mov	byte [_z80de + 1], al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit64

ioRead64:
		mov	byte [_z80de + 1], 0ffh	; An unreferenced read
readExit64:
		mov	edi, [cyclesRemaining]
;
; Remember, this variant of the IN instruction modifies the flags
;

		sahf	; Restore our flags
		mov	dh, ah	; Save flags for later
		mov	dl, byte [_z80de + 1]
		or	dl, dl
		lahf
		and	dh, 029h	; Only keep carry and two unused flags
		and	ah, 0d4h
		or	ah, dh
		xor	edx, edx
		sub	edi, byte 12
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst51:
		mov	dl, cl	; Address in DX... (C)
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, byte [_z80de + 1]	; And our data to write
		mov	edi, [_z80IoWrite]	; Point to the I/O write array

checkLoop65:
		cmp	[edi], word 0ffffh ; End of our list?
		je	WriteMacroExit65	; Yes - ignore it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr65	; Yes... go to the next addr
		cmp	dx, [edi+2]	; Are we bigger?
		jbe	callRoutine65	; If not, go call it!

nextAddr65:
		add	edi, 0ch		; Next structure, please
		jmp	short checkLoop65

callRoutine65:
		call	WriteIOByte	; Go write the data!
WriteMacroExit65:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 12
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst52:
		mov	dx, word [_z80de]	; Get our original register
		mov	[_orgval], dx	; Store this for later half carry computation
		mov	[_orgval2], bx	; Store this, too
		sahf		; Restore our flags
		sbb	bx, dx	; Do the operation!
		lahf		; Get our new flags
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0edh	; Knock out negative & half carry flags
		or	ah, 02h	; Negative!
		mov	[_z80hl], bx
		xor	bx, [_orgval]
		xor	bx, [_orgval2]
		and	bh, 10h	; Half carry?
		or	ah, bh	; OR It in if so
		mov	bx, [_z80hl]
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst53:
		mov	dx, [esi]	; Get our address to write to
		add	esi, 2		; Next address, please...
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, byte [_z80de]	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop66:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite66	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr66	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine66	; If not, go call it!

nextAddr66:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop66

callRoutine66:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit66
memoryWrite66:
		mov	edi, edx
		mov	dl, byte [_z80de]
		mov	[ebp + edi], dl
		mov	edx, edi
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit66:
		mov	edi, [cyclesRemaining]
		inc	dx
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, byte [_z80de + 1]	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop67:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite67	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr67	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine67	; If not, go call it!

nextAddr67:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop67

callRoutine67:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit67
memoryWrite67:
		mov	edi, edx
		mov	dl, byte [_z80de + 1]
		mov	[ebp + edi], dl
		mov	edx, edi
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit67:
		mov	edi, [cyclesRemaining]
		xor	edx, edx	; Zero our upper word
		sub	edi, byte 20
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst56:
		mov	dword [_z80interruptMode], 1 ; Interrupt mode 1
		mov	word [_z80intAddr], 038h	; Interrupt mode 1 cmd!
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst57:
           mov     al, [_z80i]
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst58:
		mov	dl, cl	; Address in DX... (C)
		mov	[cyclesRemaining], edi
		mov	edi, [_z80IoRead]	; Point to the read array

checkLoop68:
		cmp	[edi], word 0ffffh ; End of the list?
		je		ioRead68
		cmp	dx, [edi]	; Are we smaller?
		jb		nextAddr68		; Yes, go to the next address
		cmp	dx, [edi+2]	; Are we bigger?
		jbe	callRoutine68

nextAddr68:
		add	edi, 0ch		; Next structure!
		jmp	short checkLoop68

callRoutine68:
		call	ReadIOByte	; Standard read routine
		mov	byte [_z80de], al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit68

ioRead68:
		mov	byte [_z80de], 0ffh	; An unreferenced read
readExit68:
		mov	edi, [cyclesRemaining]
;
; Remember, this variant of the IN instruction modifies the flags
;

		sahf	; Restore our flags
		mov	dh, ah	; Save flags for later
		mov	dl, byte [_z80de]
		or	dl, dl
		lahf
		and	dh, 029h	; Only keep carry and two unused flags
		and	ah, 0d4h
		or	ah, dh
		xor	edx, edx
		sub	edi, byte 12
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst59:
		mov	dl, cl	; Address in DX... (C)
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, byte [_z80de]	; And our data to write
		mov	edi, [_z80IoWrite]	; Point to the I/O write array

checkLoop69:
		cmp	[edi], word 0ffffh ; End of our list?
		je	WriteMacroExit69	; Yes - ignore it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr69	; Yes... go to the next addr
		cmp	dx, [edi+2]	; Are we bigger?
		jbe	callRoutine69	; If not, go call it!

nextAddr69:
		add	edi, 0ch		; Next structure, please
		jmp	short checkLoop69

callRoutine69:
		call	WriteIOByte	; Go write the data!
WriteMacroExit69:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 12
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst5a:
		mov	dx, word [_z80de]	; Get our original register
		mov	[_orgval], dx	; Store this for later half carry computation
		mov	[_orgval2], bx	; Store this, too
		sahf		; Restore our flags
		adc	bx, dx	; Do the operation!
		lahf		; Get our new flags
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0edh	; Knock out negative & half carry flags
		mov	[_z80hl], bx
		xor	bx, [_orgval]
		xor	bx, [_orgval2]
		and	bh, 10h	; Half carry?
		or	ah, bh	; OR It in if so
		mov	bx, [_z80hl]
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst5b:
		mov	dx, [esi]	; Get address to load
		add	esi, 2	; Skip over it so we don't execute it
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop70:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead70
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr70		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine70

nextAddr70:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop70

callRoutine70:
		call	ReadMemoryByte	; Standard read routine
		mov	byte [_z80de], al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit70

memoryRead70:
		mov	di, dx
		mov	dl, [ebp + edx]
		mov	byte [_z80de], dl
		mov	dx, di
readExit70:
		mov	edi, [cyclesRemaining]
		inc	dx
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop71:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead71
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr71		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine71

nextAddr71:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop71

callRoutine71:
		call	ReadMemoryByte	; Standard read routine
		mov	byte [_z80de + 1], al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit71

memoryRead71:
		mov	di, dx
		mov	dl, [ebp + edx]
		mov	byte [_z80de + 1], dl
		mov	dx, di
readExit71:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 20
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst5e:
		mov	dword [_z80interruptMode], 2 ; IM 2
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst5f:
           mov     al, [_z80r]
		and	ah, 029h	; No N, H, Z, or S!
		or	al,al	; Get appropriate flags
		o16 pushf
		pop	dx
		and	dl, 0c0h
		or	ah, dl	; OR In our S & Z flags
		mov	dl, [_z80iff]
		and	dl, IFF2
		shl	dl, 1
		or	ah, dl
		mov	edx, [dwLastRSample]
		sub	edx, edi
		add	edx, [_z80rCounter]
		shr	edx, 2
		and	edx, 07fh
		and	byte [_z80r], 80h
		or		byte [_z80r], dl
		xor	edx, edx
		mov	[dwLastRSample], edi
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst60:
		mov	dl, cl	; Address in DX... (C)
		mov	[cyclesRemaining], edi
		mov	edi, [_z80IoRead]	; Point to the read array

checkLoop72:
		cmp	[edi], word 0ffffh ; End of the list?
		je		ioRead72
		cmp	dx, [edi]	; Are we smaller?
		jb		nextAddr72		; Yes, go to the next address
		cmp	dx, [edi+2]	; Are we bigger?
		jbe	callRoutine72

nextAddr72:
		add	edi, 0ch		; Next structure!
		jmp	short checkLoop72

callRoutine72:
		call	ReadIOByte	; Standard read routine
		mov	bh, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit72

ioRead72:
		mov	bh, 0ffh	; An unreferenced read
readExit72:
		mov	edi, [cyclesRemaining]
;
; Remember, this variant of the IN instruction modifies the flags
;

		sahf	; Restore our flags
		mov	dh, ah	; Save flags for later
		or	bh, bh;
		lahf
		and	dh, 029h	; Only keep carry and two unused flags
		and	ah, 0d4h
		or	ah, dh
		xor	edx, edx
		sub	edi, byte 12
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst61:
		mov	dl, cl	; Address in DX... (C)
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, bh	; And our data to write
		mov	edi, [_z80IoWrite]	; Point to the I/O write array

checkLoop73:
		cmp	[edi], word 0ffffh ; End of our list?
		je	WriteMacroExit73	; Yes - ignore it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr73	; Yes... go to the next addr
		cmp	dx, [edi+2]	; Are we bigger?
		jbe	callRoutine73	; If not, go call it!

nextAddr73:
		add	edi, 0ch		; Next structure, please
		jmp	short checkLoop73

callRoutine73:
		call	WriteIOByte	; Go write the data!
WriteMacroExit73:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 12
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst62:
		mov	dx, bx	; Get our original register
		mov	[_orgval], dx	; Store this for later half carry computation
		mov	[_orgval2], bx	; Store this, too
		sahf		; Restore our flags
		sbb	bx, dx	; Do the operation!
		lahf		; Get our new flags
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0edh	; Knock out negative & half carry flags
		or	ah, 02h	; Negative!
		mov	[_z80hl], bx
		xor	bx, [_orgval]
		xor	bx, [_orgval2]
		and	bh, 10h	; Half carry?
		or	ah, bh	; OR It in if so
		mov	bx, [_z80hl]
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst63:
		mov	dx, [esi]	; Get our address to write to
		add	esi, 2		; Next address, please...
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, bl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop74:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite74	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr74	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine74	; If not, go call it!

nextAddr74:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop74

callRoutine74:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit74
memoryWrite74:
		mov	[ebp + edx], bl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit74:
		mov	edi, [cyclesRemaining]
		inc	dx
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, bh	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop75:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite75	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr75	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine75	; If not, go call it!

nextAddr75:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop75

callRoutine75:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit75
memoryWrite75:
		mov	[ebp + edx], bh
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit75:
		mov	edi, [cyclesRemaining]
		xor	edx, edx	; Zero our upper word
		sub	edi, byte 20
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst67:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop76:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead76
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr76		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine76

nextAddr76:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop76

callRoutine76:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit76

memoryRead76:
		mov	dl, [ebp + ebx]	; Get our data

readExit76:
		mov	edi, [cyclesRemaining]
		mov	dh, dl	; Put a copy in DH
		shr	dl, 4	; Upper nibble to lower nibble
		shl	ecx, 16	; Save this
		mov	cl, al
		shl	cl, 4
		or	dl, cl	; OR In what was in A
		and	al, 0f0h	; Knock out lower part
		and	dh, 0fh	; Only the lower nibble
		or	al, dh	; OR In our nibble
		shr	ecx, 16	; Restore this
		and	ah, 29h	; Retain carry & two undefined bits
		mov	dh, ah	; Store our flags away for later
		or	al, al	; Get our flags
		lahf
		and	ah,0c4h	; Only partiy, zero, and sign
		or	ah, dh	; OR In our old flags
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop77:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite77	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr77	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine77	; If not, go call it!

nextAddr77:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop77

callRoutine77:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit77
memoryWrite77:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit77:
		mov	edi, [cyclesRemaining]
		xor	edx, edx	; Zero out this for later
		sub	edi, byte 18
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst68:
		mov	dl, cl	; Address in DX... (C)
		mov	[cyclesRemaining], edi
		mov	edi, [_z80IoRead]	; Point to the read array

checkLoop78:
		cmp	[edi], word 0ffffh ; End of the list?
		je		ioRead78
		cmp	dx, [edi]	; Are we smaller?
		jb		nextAddr78		; Yes, go to the next address
		cmp	dx, [edi+2]	; Are we bigger?
		jbe	callRoutine78

nextAddr78:
		add	edi, 0ch		; Next structure!
		jmp	short checkLoop78

callRoutine78:
		call	ReadIOByte	; Standard read routine
		mov	bl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit78

ioRead78:
		mov	bl, 0ffh	; An unreferenced read
readExit78:
		mov	edi, [cyclesRemaining]
;
; Remember, this variant of the IN instruction modifies the flags
;

		sahf	; Restore our flags
		mov	dh, ah	; Save flags for later
		or	bl, bl;
		lahf
		and	dh, 029h	; Only keep carry and two unused flags
		and	ah, 0d4h
		or	ah, dh
		xor	edx, edx
		sub	edi, byte 12
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst69:
		mov	dl, cl	; Address in DX... (C)
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, bl	; And our data to write
		mov	edi, [_z80IoWrite]	; Point to the I/O write array

checkLoop79:
		cmp	[edi], word 0ffffh ; End of our list?
		je	WriteMacroExit79	; Yes - ignore it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr79	; Yes... go to the next addr
		cmp	dx, [edi+2]	; Are we bigger?
		jbe	callRoutine79	; If not, go call it!

nextAddr79:
		add	edi, 0ch		; Next structure, please
		jmp	short checkLoop79

callRoutine79:
		call	WriteIOByte	; Go write the data!
WriteMacroExit79:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 12
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst6a:
		mov	dx, bx	; Get our original register
		mov	[_orgval], dx	; Store this for later half carry computation
		mov	[_orgval2], bx	; Store this, too
		sahf		; Restore our flags
		adc	bx, dx	; Do the operation!
		lahf		; Get our new flags
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0edh	; Knock out negative & half carry flags
		mov	[_z80hl], bx
		xor	bx, [_orgval]
		xor	bx, [_orgval2]
		and	bh, 10h	; Half carry?
		or	ah, bh	; OR It in if so
		mov	bx, [_z80hl]
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst6f:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop80:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead80
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr80		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine80

nextAddr80:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop80

callRoutine80:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit80

memoryRead80:
		mov	dl, [ebp + ebx]	; Get our data

readExit80:
		mov	edi, [cyclesRemaining]
		mov	dh, dl	; Put a copy in DH
		shr	dh, 4	; Get our upper nibble in position
		shl	dl, 4	; Get our lower nibble into the higher position
		shl	ecx, 16	; Save this for later
		mov	cl, al
		and	cl, 0fh
	; Only the lower nibble
		or	dl, cl	; OR In A->(HL) transfer
		and	al, 0f0h	; Only the upper 4 bits remain
		or	al, dh	; OR It in to our accumulator
		shr	ecx, 16	; Restore this
		and	ah, 29h	; Retain carry & two undefined bits
		mov	dh, ah	; Store our flags away for later
		or	al, al	; Get our flags
		lahf
		and	ah,0c4h	; Only partiy, zero, and sign
		or	ah, dh	; OR In our old flags
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop81:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite81	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr81	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine81	; If not, go call it!

nextAddr81:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop81

callRoutine81:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit81
memoryWrite81:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit81:
		mov	edi, [cyclesRemaining]
		xor	edx, edx	; Zero out this for later
		sub	edi, byte 18
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst72:
		mov	dx, word [_z80sp]	; Get our original register
		mov	[_orgval], dx	; Store this for later half carry computation
		mov	[_orgval2], bx	; Store this, too
		sahf		; Restore our flags
		sbb	bx, dx	; Do the operation!
		lahf		; Get our new flags
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0edh	; Knock out negative & half carry flags
		or	ah, 02h	; Negative!
		mov	[_z80hl], bx
		xor	bx, [_orgval]
		xor	bx, [_orgval2]
		and	bh, 10h	; Half carry?
		or	ah, bh	; OR It in if so
		mov	bx, [_z80hl]
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst73:
		mov	dx, [esi]	; Get our address to write to
		add	esi, 2		; Next address, please...
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, byte [_z80sp]	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop82:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite82	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr82	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine82	; If not, go call it!

nextAddr82:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop82

callRoutine82:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit82
memoryWrite82:
		mov	edi, edx
		mov	dl, byte [_z80sp]
		mov	[ebp + edi], dl
		mov	edx, edi
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit82:
		mov	edi, [cyclesRemaining]
		inc	dx
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, byte [_z80sp + 1]	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop83:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite83	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr83	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine83	; If not, go call it!

nextAddr83:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop83

callRoutine83:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit83
memoryWrite83:
		mov	edi, edx
		mov	dl, byte [_z80sp + 1]
		mov	[ebp + edi], dl
		mov	edx, edi
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit83:
		mov	edi, [cyclesRemaining]
		xor	edx, edx	; Zero our upper word
		sub	edi, byte 20
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst78:
		mov	dl, cl	; Address in DX... (C)
		mov	[cyclesRemaining], edi
		mov	edi, [_z80IoRead]	; Point to the read array

checkLoop84:
		cmp	[edi], word 0ffffh ; End of the list?
		je		ioRead84
		cmp	dx, [edi]	; Are we smaller?
		jb		nextAddr84		; Yes, go to the next address
		cmp	dx, [edi+2]	; Are we bigger?
		jbe	callRoutine84

nextAddr84:
		add	edi, 0ch		; Next structure!
		jmp	short checkLoop84

callRoutine84:
		call	ReadIOByte	; Standard read routine
		mov	[_z80af], al	; Save our new accumulator
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit84

ioRead84:
		mov	al, 0ffh	; An unreferenced read
readExit84:
		mov	edi, [cyclesRemaining]
;
; Remember, this variant of the IN instruction modifies the flags
;

		sahf	; Restore our flags
		mov	dh, ah	; Save flags for later
		or	al, al;
		lahf
		and	dh, 029h	; Only keep carry and two unused flags
		and	ah, 0d4h
		or	ah, dh
		xor	edx, edx
		sub	edi, byte 12
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst79:
		mov	dl, cl	; Address in DX... (C)
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	edi, [_z80IoWrite]	; Point to the I/O write array

checkLoop85:
		cmp	[edi], word 0ffffh ; End of our list?
		je	WriteMacroExit85	; Yes - ignore it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr85	; Yes... go to the next addr
		cmp	dx, [edi+2]	; Are we bigger?
		jbe	callRoutine85	; If not, go call it!

nextAddr85:
		add	edi, 0ch		; Next structure, please
		jmp	short checkLoop85

callRoutine85:
		call	WriteIOByte	; Go write the data!
WriteMacroExit85:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 12
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst7a:
		mov	dx, word [_z80sp]	; Get our original register
		mov	[_orgval], dx	; Store this for later half carry computation
		mov	[_orgval2], bx	; Store this, too
		sahf		; Restore our flags
		adc	bx, dx	; Do the operation!
		lahf		; Get our new flags
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0edh	; Knock out negative & half carry flags
		mov	[_z80hl], bx
		xor	bx, [_orgval]
		xor	bx, [_orgval2]
		and	bh, 10h	; Half carry?
		or	ah, bh	; OR It in if so
		mov	bx, [_z80hl]
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInst7b:
		mov	dx, [esi]	; Get address to load
		add	esi, 2	; Skip over it so we don't execute it
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop86:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead86
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr86		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine86

nextAddr86:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop86

callRoutine86:
		call	ReadMemoryByte	; Standard read routine
		mov	byte [_z80sp], al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit86

memoryRead86:
		mov	di, dx
		mov	dl, [ebp + edx]
		mov	byte [_z80sp], dl
		mov	dx, di
readExit86:
		mov	edi, [cyclesRemaining]
		inc	dx
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop87:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead87
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr87		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine87

nextAddr87:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop87

callRoutine87:
		call	ReadMemoryByte	; Standard read routine
		mov	byte [_z80sp + 1], al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit87

memoryRead87:
		mov	di, dx
		mov	dl, [ebp + edx]
		mov	byte [_z80sp + 1], dl
		mov	dx, di
readExit87:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 20
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInsta0:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop88:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead88
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr88		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine88

nextAddr88:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop88

callRoutine88:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit88

memoryRead88:
		mov	dl, [ebp + ebx]	; Get our data

readExit88:
		mov	edi, [cyclesRemaining]
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_z80de]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop89:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite89	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr89	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine89	; If not, go call it!

nextAddr89:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop89

callRoutine89:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit89
memoryWrite89:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit89:
		mov	edi, [cyclesRemaining]
		inc	bx	; Increment HL
		inc	word [_z80de]	; Increment DE
		dec	cx	; Decrement BC
		and	ah, 0e9h ; Knock out H & N and P/V
		or		cx, cx	; Flag BC
		jz	atZero90 ; We're done!
		or	ah, 04h	; Non-zero - we're still going!
atZero90:
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 16
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInsta1:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop91:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead91
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr91		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine91

nextAddr91:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop91

callRoutine91:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit91

memoryRead91:
		mov	dl, [ebp + ebx]	; Get our data

readExit91:
		mov	edi, [cyclesRemaining]
		mov	byte [_z80af], ah
		sahf
		cmp	al, dl	; Do our comparison
		lahf
		and	ah, 0fah	; No P/V or carry!
		dec	cx	; Dec BC
		jz	notBcZero92
		or	ah, 04h	; P/V set when BC not zero
notBcZero92:
		or	ah, 02h	; N Gets set when we do compares
		mov	dl, byte [_z80af]
		and	dl, 01h
		or	ah, dl	; Preserve carry!
		inc	bx	; Increment!
		xor	edx, edx
		sub	edi, byte 16
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInsta2:
		push	cx	; Save BC
		xor	ch, ch ; We want 8 bit ports
		mov	[cyclesRemaining], edi
		mov	edi, [_z80IoRead]	; Point to the read array

checkLoop93:
		cmp	[edi], word 0ffffh ; End of the list?
		je		ioRead93
		cmp	cx, [edi]	; Are we smaller?
		jb		nextAddr93		; Yes, go to the next address
		cmp	cx, [edi+2]	; Are we bigger?
		jbe	callRoutine93

nextAddr93:
		add	edi, 0ch		; Next structure!
		jmp	short checkLoop93

callRoutine93:
		mov	dx, cx	; Get our address
		call	ReadIOByte	; Standard read routine
		mov	dl, al	; Put it in DL for later consumption
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit93

ioRead93:
		mov	dl, 0ffh	; An unreferenced read
readExit93:
		mov	edi, [cyclesRemaining]
		pop	cx	; Restore BC
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop94:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite94	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr94	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine94	; If not, go call it!

nextAddr94:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop94

callRoutine94:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit94
memoryWrite94:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit94:
		mov	edi, [cyclesRemaining]
		inc	bx	; Increment HL
		dec	ch	; Decrement B (of C)
finalExit92:
		jnz	clearFlag92
		or	ah, 040h	; Set the Zero flag!
		jmp	short continue92
clearFlag92:
		and	ah, 0bfh	; Clear the zero flag
continue92:
		or	ah, 02h	; Set negative!
		xor	edx, edx
		sub	edi, byte 16
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInsta3:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop96:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead96
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr96		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine96

nextAddr96:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop96

callRoutine96:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit96

memoryRead96:
		mov	dl, [ebp + ebx]	; Get our data

readExit96:
		mov	edi, [cyclesRemaining]
		push	cx	; Save BC
		xor	ch, ch	; No 16 bit for this instruction!
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80IoWrite]	; Point to the I/O write array

checkLoop97:
		cmp	[edi], word 0ffffh ; End of our list?
		je	WriteMacroExit97	; Yes - ignore it!
		cmp	cx, [edi]	; Are we smaller?
		jb	nextAddr97	; Yes... go to the next addr
		cmp	cx, [edi+2]	; Are we bigger?
		jbe	callRoutine97	; If not, go call it!

nextAddr97:
		add	edi, 0ch		; Next structure, please
		jmp	short checkLoop97

callRoutine97:
		mov	dx, cx	; Get our address to target
		call	WriteIOByte	; Go write the data!
WriteMacroExit97:
		mov	edi, [cyclesRemaining]
		pop	cx	; Restore BC now that it has been "OUT"ed
		inc	bx	; Increment HL
		dec	ch	; Decrement B (of C)
finalExit95:
		jnz	clearFlag95
		or	ah, 040h	; Set the Zero flag!
		jmp	short continue95
clearFlag95:
		and	ah, 0bfh	; Clear the zero flag
continue95:
		or	ah, 02h	; Set negative!
		xor	edx, edx
		sub	edi, byte 16
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInsta8:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop98:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead98
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr98		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine98

nextAddr98:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop98

callRoutine98:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit98

memoryRead98:
		mov	dl, [ebp + ebx]	; Get our data

readExit98:
		mov	edi, [cyclesRemaining]
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_z80de]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop99:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite99	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr99	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine99	; If not, go call it!

nextAddr99:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop99

callRoutine99:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit99
memoryWrite99:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit99:
		mov	edi, [cyclesRemaining]
		dec	bx	; Decrement HL
		dec	word [_z80de]	; Decrement DE
		dec	cx	; Decrement BC
		and	ah, 0e9h ; Knock out H & N and P/V
		or		cx, cx	; Flag BC
		jz	atZero100 ; We're done!
		or	ah, 04h	; Non-zero - we're still going!
atZero100:
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 16
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInsta9:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop101:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead101
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr101		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine101

nextAddr101:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop101

callRoutine101:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit101

memoryRead101:
		mov	dl, [ebp + ebx]	; Get our data

readExit101:
		mov	edi, [cyclesRemaining]
		mov	byte [_z80af], ah
		sahf
		cmp	al, dl	; Do our comparison
		lahf
		and	ah, 0fah	; No P/V or carry!
		dec	cx	; Dec BC
		jz	notBcZero102
		or	ah, 04h	; P/V set when BC not zero
notBcZero102:
		or	ah, 02h	; N Gets set when we do compares
		mov	dl, byte [_z80af]
		and	dl, 01h
		or	ah, dl	; Preserve carry!
		dec	bx	; Decrement!
		xor	edx, edx
		sub	edi, byte 16
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInstaa:
		push	cx	; Save BC
		xor	ch, ch ; We want 8 bit ports
		mov	[cyclesRemaining], edi
		mov	edi, [_z80IoRead]	; Point to the read array

checkLoop103:
		cmp	[edi], word 0ffffh ; End of the list?
		je		ioRead103
		cmp	cx, [edi]	; Are we smaller?
		jb		nextAddr103		; Yes, go to the next address
		cmp	cx, [edi+2]	; Are we bigger?
		jbe	callRoutine103

nextAddr103:
		add	edi, 0ch		; Next structure!
		jmp	short checkLoop103

callRoutine103:
		mov	dx, cx	; Get our address
		call	ReadIOByte	; Standard read routine
		mov	dl, al	; Put it in DL for later consumption
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit103

ioRead103:
		mov	dl, 0ffh	; An unreferenced read
readExit103:
		mov	edi, [cyclesRemaining]
		pop	cx	; Restore BC
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop104:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite104	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr104	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine104	; If not, go call it!

nextAddr104:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop104

callRoutine104:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit104
memoryWrite104:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit104:
		mov	edi, [cyclesRemaining]
		dec	bx	; Decrement HL
		dec	ch	; Decrement B (of C)
finalExit102:
		jnz	clearFlag102
		or	ah, 040h	; Set the Zero flag!
		jmp	short continue102
clearFlag102:
		and	ah, 0bfh	; Clear the zero flag
continue102:
		or	ah, 02h	; Set negative!
		xor	edx, edx
		sub	edi, byte 16
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInstab:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop106:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead106
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr106		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine106

nextAddr106:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop106

callRoutine106:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit106

memoryRead106:
		mov	dl, [ebp + ebx]	; Get our data

readExit106:
		mov	edi, [cyclesRemaining]
		push	cx	; Save BC
		xor	ch, ch	; No 16 bit for this instruction!
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80IoWrite]	; Point to the I/O write array

checkLoop107:
		cmp	[edi], word 0ffffh ; End of our list?
		je	WriteMacroExit107	; Yes - ignore it!
		cmp	cx, [edi]	; Are we smaller?
		jb	nextAddr107	; Yes... go to the next addr
		cmp	cx, [edi+2]	; Are we bigger?
		jbe	callRoutine107	; If not, go call it!

nextAddr107:
		add	edi, 0ch		; Next structure, please
		jmp	short checkLoop107

callRoutine107:
		mov	dx, cx	; Get our address to target
		call	WriteIOByte	; Go write the data!
WriteMacroExit107:
		mov	edi, [cyclesRemaining]
		pop	cx	; Restore BC now that it has been "OUT"ed
		dec	bx	; Decrement HL
		dec	ch	; Decrement B (of C)
finalExit105:
		jnz	clearFlag105
		or	ah, 040h	; Set the Zero flag!
		jmp	short continue105
clearFlag105:
		and	ah, 0bfh	; Clear the zero flag
continue105:
		or	ah, 02h	; Set negative!
		xor	edx, edx
		sub	edi, byte 16
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInstb0:
ldRepeat108:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop108:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead108
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr108		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine108

nextAddr108:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop108

callRoutine108:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit108

memoryRead108:
		mov	dl, [ebp + ebx]	; Get our data

readExit108:
		mov	edi, [cyclesRemaining]
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_z80de]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop109:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite109	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr109	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine109	; If not, go call it!

nextAddr109:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop109

callRoutine109:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit109
memoryWrite109:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit109:
		mov	edi, [cyclesRemaining]
		inc	bx	; Increment HL
		inc	word [_z80de]	; Increment DE
		dec	cx	; Decrement BC
		jz	noMore110
		sub	edi, dword 16	; 16 T-States per iteration
		js	noMore110
		jmp	ldRepeat108 ; Loop until we're done!
noMore110:
		and	ah, 0e9h ; Knock out H & N and P/V
		or		cx, cx	; Flag BC
		jz	atZero110 ; We're done!
		or	ah, 04h	; Non-zero - we're still going!
		sub	esi, 2	; Adjust back to the beginning of the instruction
		jmp	noMoreExec

atZero110:
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 16
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInstb1:
cpRepeat111:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop112:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead112
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr112		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine112

nextAddr112:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop112

callRoutine112:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit112

memoryRead112:
		mov	dl, [ebp + ebx]	; Get our data

readExit112:
		mov	edi, [cyclesRemaining]
		mov	byte [_z80af], ah
		sahf
		cmp	al, dl	; Do our comparison
		lahf
		and	ah, 0fah	; No P/V or carry!
		dec	cx	; Dec BC
		jz	notBcZero113
		or	ah, 04h	; P/V set when BC not zero
notBcZero113:
		or	ah, 02h	; N Gets set when we do compares
		mov	dl, byte [_z80af]
		and	dl, 01h
		or	ah, dl	; Preserve carry!
		inc	bx	; Increment!
		sahf
		jz	BCDone111
		jnp	BCDone111
		sub	edi, dword 21
		js		BCDoneExit111
		jmp	cpRepeat111
BCDoneExit111:
		sub	esi, 2	;	Back up to the instruction again
		jmp	noMoreExec

BCDone111:
		xor	edx, edx
		sub	edi, byte 16
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInstb2:
loopIt113:
		push	cx	; Save BC
		xor	ch, ch ; We want 8 bit ports
		mov	[cyclesRemaining], edi
		mov	edi, [_z80IoRead]	; Point to the read array

checkLoop114:
		cmp	[edi], word 0ffffh ; End of the list?
		je		ioRead114
		cmp	cx, [edi]	; Are we smaller?
		jb		nextAddr114		; Yes, go to the next address
		cmp	cx, [edi+2]	; Are we bigger?
		jbe	callRoutine114

nextAddr114:
		add	edi, 0ch		; Next structure!
		jmp	short checkLoop114

callRoutine114:
		mov	dx, cx	; Get our address
		call	ReadIOByte	; Standard read routine
		mov	dl, al	; Put it in DL for later consumption
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit114

ioRead114:
		mov	dl, 0ffh	; An unreferenced read
readExit114:
		mov	edi, [cyclesRemaining]
		pop	cx	; Restore BC
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop115:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite115	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr115	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine115	; If not, go call it!

nextAddr115:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop115

callRoutine115:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit115
memoryWrite115:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit115:
		mov	edi, [cyclesRemaining]
		inc	bx	; Increment HL
		dec	ch	; Decrement B (of C)
		jz	near finalExit113
		sub	edi, dword 21
		js		loopExit113
		jmp	loopIt113

loopExit113:
		sub	esi, 2
		jmp	noMoreExec

finalExit113:
		jnz	clearFlag113
		or	ah, 040h	; Set the Zero flag!
		jmp	short continue113
clearFlag113:
		and	ah, 0bfh	; Clear the zero flag
continue113:
		or	ah, 02h	; Set negative!
		xor	edx, edx
		sub	edi, byte 16
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInstb3:
loopIt116:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop117:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead117
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr117		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine117

nextAddr117:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop117

callRoutine117:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit117

memoryRead117:
		mov	dl, [ebp + ebx]	; Get our data

readExit117:
		mov	edi, [cyclesRemaining]
		push	cx	; Save BC
		xor	ch, ch	; No 16 bit for this instruction!
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80IoWrite]	; Point to the I/O write array

checkLoop118:
		cmp	[edi], word 0ffffh ; End of our list?
		je	WriteMacroExit118	; Yes - ignore it!
		cmp	cx, [edi]	; Are we smaller?
		jb	nextAddr118	; Yes... go to the next addr
		cmp	cx, [edi+2]	; Are we bigger?
		jbe	callRoutine118	; If not, go call it!

nextAddr118:
		add	edi, 0ch		; Next structure, please
		jmp	short checkLoop118

callRoutine118:
		mov	dx, cx	; Get our address to target
		call	WriteIOByte	; Go write the data!
WriteMacroExit118:
		mov	edi, [cyclesRemaining]
		pop	cx	; Restore BC now that it has been "OUT"ed
		inc	bx	; Increment HL
		dec	ch	; Decrement B (of C)
		jz	near finalExit116
		sub	edi, dword 21
		js		loopExit116
		jmp	loopIt116

loopExit116:
		sub	esi, 2
		jmp	noMoreExec

finalExit116:
		jnz	clearFlag116
		or	ah, 040h	; Set the Zero flag!
		jmp	short continue116
clearFlag116:
		and	ah, 0bfh	; Clear the zero flag
continue116:
		or	ah, 02h	; Set negative!
		xor	edx, edx
		sub	edi, byte 16
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInstb8:
ldRepeat119:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop119:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead119
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr119		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine119

nextAddr119:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop119

callRoutine119:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit119

memoryRead119:
		mov	dl, [ebp + ebx]	; Get our data

readExit119:
		mov	edi, [cyclesRemaining]
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_z80de]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop120:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite120	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr120	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine120	; If not, go call it!

nextAddr120:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop120

callRoutine120:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit120
memoryWrite120:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit120:
		mov	edi, [cyclesRemaining]
		dec	bx	; Decrement HL
		dec	word [_z80de]	; Decrement DE
		dec	cx	; Decrement BC
		jz	noMore121
		sub	edi, dword 16	; 16 T-States per iteration
		js	noMore121
		jmp	ldRepeat119 ; Loop until we're done!
noMore121:
		and	ah, 0e9h ; Knock out H & N and P/V
		or		cx, cx	; Flag BC
		jz	atZero121 ; We're done!
		or	ah, 04h	; Non-zero - we're still going!
		sub	esi, 2	; Adjust back to the beginning of the instruction
		jmp	noMoreExec

atZero121:
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 16
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInstb9:
cpRepeat122:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop123:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead123
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr123		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine123

nextAddr123:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop123

callRoutine123:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit123

memoryRead123:
		mov	dl, [ebp + ebx]	; Get our data

readExit123:
		mov	edi, [cyclesRemaining]
		mov	byte [_z80af], ah
		sahf
		cmp	al, dl	; Do our comparison
		lahf
		and	ah, 0fah	; No P/V or carry!
		dec	cx	; Dec BC
		jz	notBcZero124
		or	ah, 04h	; P/V set when BC not zero
notBcZero124:
		or	ah, 02h	; N Gets set when we do compares
		mov	dl, byte [_z80af]
		and	dl, 01h
		or	ah, dl	; Preserve carry!
		dec	bx	; Decrement!
		sahf
		jz	BCDone122
		jnp	BCDone122
		sub	edi, dword 21
		js		BCDoneExit122
		jmp	cpRepeat122
BCDoneExit122:
		sub	esi, 2	;	Back up to the instruction again
		jmp	noMoreExec

BCDone122:
		xor	edx, edx
		sub	edi, byte 16
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInstba:
loopIt124:
		push	cx	; Save BC
		xor	ch, ch ; We want 8 bit ports
		mov	[cyclesRemaining], edi
		mov	edi, [_z80IoRead]	; Point to the read array

checkLoop125:
		cmp	[edi], word 0ffffh ; End of the list?
		je		ioRead125
		cmp	cx, [edi]	; Are we smaller?
		jb		nextAddr125		; Yes, go to the next address
		cmp	cx, [edi+2]	; Are we bigger?
		jbe	callRoutine125

nextAddr125:
		add	edi, 0ch		; Next structure!
		jmp	short checkLoop125

callRoutine125:
		mov	dx, cx	; Get our address
		call	ReadIOByte	; Standard read routine
		mov	dl, al	; Put it in DL for later consumption
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit125

ioRead125:
		mov	dl, 0ffh	; An unreferenced read
readExit125:
		mov	edi, [cyclesRemaining]
		pop	cx	; Restore BC
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop126:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite126	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr126	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine126	; If not, go call it!

nextAddr126:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop126

callRoutine126:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit126
memoryWrite126:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit126:
		mov	edi, [cyclesRemaining]
		dec	bx	; Decrement HL
		dec	ch	; Decrement B (of C)
		jz	near finalExit124
		sub	edi, dword 21
		js		loopExit124
		jmp	loopIt124

loopExit124:
		sub	esi, 2
		jmp	noMoreExec

finalExit124:
		jnz	clearFlag124
		or	ah, 040h	; Set the Zero flag!
		jmp	short continue124
clearFlag124:
		and	ah, 0bfh	; Clear the zero flag
continue124:
		or	ah, 02h	; Set negative!
		xor	edx, edx
		sub	edi, byte 16
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

EDInstbb:
loopIt127:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop128:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead128
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr128		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine128

nextAddr128:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop128

callRoutine128:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit128

memoryRead128:
		mov	dl, [ebp + ebx]	; Get our data

readExit128:
		mov	edi, [cyclesRemaining]
		push	cx	; Save BC
		xor	ch, ch	; No 16 bit for this instruction!
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80IoWrite]	; Point to the I/O write array

checkLoop129:
		cmp	[edi], word 0ffffh ; End of our list?
		je	WriteMacroExit129	; Yes - ignore it!
		cmp	cx, [edi]	; Are we smaller?
		jb	nextAddr129	; Yes... go to the next addr
		cmp	cx, [edi+2]	; Are we bigger?
		jbe	callRoutine129	; If not, go call it!

nextAddr129:
		add	edi, 0ch		; Next structure, please
		jmp	short checkLoop129

callRoutine129:
		mov	dx, cx	; Get our address to target
		call	WriteIOByte	; Go write the data!
WriteMacroExit129:
		mov	edi, [cyclesRemaining]
		pop	cx	; Restore BC now that it has been "OUT"ed
		dec	bx	; Decrement HL
		dec	ch	; Decrement B (of C)
		jz	near finalExit127
		sub	edi, dword 21
		js		loopExit127
		jmp	loopIt127

loopExit127:
		sub	esi, 2
		jmp	noMoreExec

finalExit127:
		jnz	clearFlag127
		or	ah, 040h	; Set the Zero flag!
		jmp	short continue127
clearFlag127:
		and	ah, 0bfh	; Clear the zero flag
continue127:
		or	ah, 02h	; Set negative!
		xor	edx, edx
		sub	edi, byte 16
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]



times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst09:
		mov	dh, ah	; Get our flags
		and	dh, 0ech	; Preserve the top three and bits 2 & 3
		mov	[cyclesRemaining], edi
		mov	di, [_z80ix]	; Get our value
		mov	[_orgval], di	; Store our original value
		add	di, cx
		lahf
		mov	[_z80ix], di	; Store our register back
		mov	di, [_orgval]	; Get original
		xor	di, word [_z80ix] ; XOR It with our computed value
		xor	di, cx
		and	di, 1000h	; Just our half carry
		or		dx, di	; Or in our flags
		and	ah, 01h	; Just carry
		or	ah, dh
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst19:
		mov	dh, ah	; Get our flags
		and	dh, 0ech	; Preserve the top three and bits 2 & 3
		mov	[cyclesRemaining], edi
		mov	di, [_z80ix]	; Get our value
		mov	[_orgval], di	; Store our original value
		add	di, word [_z80de]
		lahf
		mov	[_z80ix], di	; Store our register back
		mov	di, [_orgval]	; Get original
		xor	di, word [_z80ix] ; XOR It with our computed value
		xor	di, word [_z80de]
		and	di, 1000h	; Just our half carry
		or		dx, di	; Or in our flags
		and	ah, 01h	; Just carry
		or	ah, dh
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst21:
		mov	dx, [esi]	; Get our word to load
		add	esi, 2	; Advance past the word
		mov	[_z80ix], dx ; Store our new value
		xor	edx, edx
		sub	edi, byte 14
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst22:
		mov	dx, [esi]	 ; Get our address to store
		add	esi, 2
		mov	[_orgval], dx
		mov	dl, [_z80ix]
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop130:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite130	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr130	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine130	; If not, go call it!

nextAddr130:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop130

callRoutine130:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit130
memoryWrite130:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit130:
		mov	edi, [cyclesRemaining]
		inc	word [_orgval]
		mov	dl, [_z80ix + 1]
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop131:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite131	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr131	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine131	; If not, go call it!

nextAddr131:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop131

callRoutine131:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit131
memoryWrite131:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit131:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 20
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst23:
		inc	word [_z80ix]	; Increment our mz80Index register
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst24:
		sahf
		inc	byte [_z80ix + 1]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh	; Knock out N!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst25:
		sahf
		dec	byte [_z80ix + 1]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set negative!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst26:
		mov	dl, [esi]	; Get immediate byte to load
		inc	esi	; Next byte
		mov	byte [_z80ix + 1], dl
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst29:
		mov	dh, ah	; Get our flags
		and	dh, 0ech	; Preserve the top three and bits 2 & 3
		mov	[cyclesRemaining], edi
		mov	di, [_z80ix]	; Get our value
		mov	[_orgval], di	; Store our original value
		add	di, di
		lahf
		mov	[_z80ix], di	; Store our register back
		mov	di, [_orgval]	; Get original
		xor	di, word [_z80ix] ; XOR It with our computed value
		xor	di, di
		and	di, 1000h	; Just our half carry
		or		dx, di	; Or in our flags
		and	ah, 01h	; Just carry
		or	ah, dh
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst2a:
		mov	dx, [esi]	 ; Get our address to store
		add	esi, 2
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop132:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead132
		cmp	dx, [edi]	; Are we smaller?
		jb		nextAddr132		; Yes, go to the next address
		cmp	dx, [edi+4]	; Are we bigger?
		jbe	callRoutine132

nextAddr132:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop132

callRoutine132:
		push	ax		; Save this for later
		push	dx		; Save address
		call	ReadMemoryByte	; Standard read routine
		pop	dx		; Restore our address
		inc	dx		; Next byte, please
		push	ax		; Save returned byte
		call	ReadMemoryByte	; Standard read routine
		xchg	ah, al	; Swap for endian's sake
		pop	dx	; Restore LSB
		mov	dh, ah	; Our word is now in DX
		pop	ax		; Restore this
		mov	[_z80ix], dx	; Store our word
		jmp	readExit132

memoryRead132:
		mov	dx, [ebp + edx]
		mov	[_z80ix], dx
readExit132:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 20
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst2b:
		dec	word [_z80ix]	; Increment our mz80Index register
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst2c:
		sahf
		inc	byte [_z80ix]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh	; Knock out N!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst2d:
		sahf
		dec	byte [_z80ix]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set negative!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst2e:
		mov	dl, [esi]	; Get immediate byte to load
		inc	esi	; Next byte
		mov	byte [_z80ix], dl
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst34:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned133	; Nope!
		dec	dh			; Make it FFable
notSigned133:
		add	dx, [_z80ix]	; Our offset!
		mov	[_orgval], dx
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop134:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead134
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr134		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine134

nextAddr134:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop134

callRoutine134:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit134

memoryRead134:
		mov	dl, [ebp + edx]	; Get our data

readExit134:
		mov	edi, [cyclesRemaining]
		sahf
		inc	dl
		lahf
		o16	pushf
		shl	edx, 16
		and	ah, 0fbh	;	Knock out parity/overflow
		pop	dx
		and	dh, 08h ; Just the overflow
		shr	dh, 1	; Shift it into position
		or	ah, dh	; OR It in with the real flags
		shr	edx, 16
		and	ah, 0fdh	; Knock out N!
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop135:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite135	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr135	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine135	; If not, go call it!

nextAddr135:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop135

callRoutine135:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit135
memoryWrite135:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit135:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst35:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned136	; Nope!
		dec	dh			; Make it FFable
notSigned136:
		add	dx, [_z80ix]	; Our offset!
		mov	[_orgval], dx
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop137:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead137
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr137		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine137

nextAddr137:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop137

callRoutine137:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit137

memoryRead137:
		mov	dl, [ebp + edx]	; Get our data

readExit137:
		mov	edi, [cyclesRemaining]
		sahf
		dec	dl
		lahf
		o16	pushf
		shl	edx, 16
		and	ah, 0fbh	;	Knock out parity/overflow
		pop	dx
		and	dh, 08h ; Just the overflow
		shr	dh, 1	; Shift it into position
		or	ah, dh	; OR It in with the real flags
		shr	edx, 16
		or		ah, 02h	; Make it N!
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop138:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite138	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr138	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine138	; If not, go call it!

nextAddr138:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop138

callRoutine138:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit138
memoryWrite138:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit138:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst36:
		mov	dx, [esi]	; Get our address
		add	esi, 2	; Skip over our storage bytes
		mov	[cyclesRemaining], edi
		mov	di, dx	; Store it here for later
		xor	dh, dh
		or	dl, dl
		jns	noNegate139
		dec	dh
noNegate139:
		add	dx, [_z80ix]	; Add in our index
		mov	[_orgval], dx	; Store our address to write to
		mov	dx, di
		xchg	dh, dl
		mov	edi, [cyclesRemaining]
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop139:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite139	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr139	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine139	; If not, go call it!

nextAddr139:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop139

callRoutine139:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit139
memoryWrite139:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit139:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst39:
		mov	dh, ah	; Get our flags
		and	dh, 0ech	; Preserve the top three and bits 2 & 3
		mov	[cyclesRemaining], edi
		mov	di, [_z80ix]	; Get our value
		mov	[_orgval], di	; Store our original value
		add	di, word [_z80sp]
		lahf
		mov	[_z80ix], di	; Store our register back
		mov	di, [_orgval]	; Get original
		xor	di, word [_z80ix] ; XOR It with our computed value
		xor	di, word [_z80sp]
		and	di, 1000h	; Just our half carry
		or		dx, di	; Or in our flags
		and	ah, 01h	; Just carry
		or	ah, dh
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst44:
		mov	ch, byte [_z80ix + 1]
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst45:
		mov	ch, byte [_z80ix + 0]
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst46:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned141	; Nope!
		dec	dh			; Make it FFable
notSigned141:
		add	dx, [_z80ix]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop142:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead142
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr142		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine142

nextAddr142:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop142

callRoutine142:
		call	ReadMemoryByte	; Standard read routine
		mov	ch, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit142

memoryRead142:
		mov	ch, [ebp + edx]	; Get our data

readExit142:
		mov	edi, [cyclesRemaining]
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst4c:
		mov	cl, byte [_z80ix + 1]
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst4d:
		mov	cl, byte [_z80ix + 0]
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst4e:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned144	; Nope!
		dec	dh			; Make it FFable
notSigned144:
		add	dx, [_z80ix]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop145:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead145
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr145		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine145

nextAddr145:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop145

callRoutine145:
		call	ReadMemoryByte	; Standard read routine
		mov	cl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit145

memoryRead145:
		mov	cl, [ebp + edx]	; Get our data

readExit145:
		mov	edi, [cyclesRemaining]
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst54:
		mov	dx, [_z80de]	; Get a usable copy of DE here
		mov	dh, byte [_z80ix + 1]
		mov	[_z80de], dx	; Put it back!
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst55:
		mov	dx, [_z80de]	; Get a usable copy of DE here
		mov	dh, byte [_z80ix + 0]
		mov	[_z80de], dx	; Put it back!
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst56:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned147	; Nope!
		dec	dh			; Make it FFable
notSigned147:
		add	dx, [_z80ix]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop148:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead148
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr148		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine148

nextAddr148:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop148

callRoutine148:
		call	ReadMemoryByte	; Standard read routine
		mov	byte [_z80de + 1], al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit148

memoryRead148:
		mov	di, dx
		mov	dl, [ebp + edx]
		mov	byte [_z80de + 1], dl
		mov	dx, di
readExit148:
		mov	edi, [cyclesRemaining]
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst5c:
		mov	dx, [_z80de]	; Get a usable copy of DE here
		mov	dl, byte [_z80ix + 1]
		mov	[_z80de], dx	; Put it back!
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst5d:
		mov	dx, [_z80de]	; Get a usable copy of DE here
		mov	dl, byte [_z80ix + 0]
		mov	[_z80de], dx	; Put it back!
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst5e:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned150	; Nope!
		dec	dh			; Make it FFable
notSigned150:
		add	dx, [_z80ix]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop151:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead151
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr151		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine151

nextAddr151:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop151

callRoutine151:
		call	ReadMemoryByte	; Standard read routine
		mov	byte [_z80de], al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit151

memoryRead151:
		mov	di, dx
		mov	dl, [ebp + edx]
		mov	byte [_z80de], dl
		mov	dx, di
readExit151:
		mov	edi, [cyclesRemaining]
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst60:
		mov   byte [_z80ix + 1], ch
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst61:
		mov   byte [_z80ix + 1], cl
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst62:
	mov	dx, [_z80de]	; Get DE
		mov   byte [_z80ix + 1], dh
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst63:
	mov	dx, [_z80de]	; Get DE
		mov   byte [_z80ix + 1], dl
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst64:
	mov	dh, byte [_z80ix + 1]
		mov   byte [_z80ix + 1], bh
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst65:
	mov	dl, byte [_z80ix]
		mov   byte [_z80ix + 1], bl
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst66:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned153	; Nope!
		dec	dh			; Make it FFable
notSigned153:
		add	dx, [_z80ix]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop154:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead154
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr154		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine154

nextAddr154:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop154

callRoutine154:
		call	ReadMemoryByte	; Standard read routine
		mov	bh, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit154

memoryRead154:
		mov	bh, [ebp + edx]	; Get our data

readExit154:
		mov	edi, [cyclesRemaining]
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst67:
		mov   byte [_z80ix + 1], al
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst68:
		mov   byte [_z80ix + 0], ch
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst69:
		mov   byte [_z80ix + 0], cl
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst6a:
	mov	dx, [_z80de]	; Get DE
		mov   byte [_z80ix + 0], dh
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst6b:
	mov	dx, [_z80de]	; Get DE
		mov   byte [_z80ix + 0], dl
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst6c:
	mov	dh, byte [_z80ix + 1]
		mov   byte [_z80ix + 0], bh
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst6d:
	mov	dl, byte [_z80ix]
		mov   byte [_z80ix + 0], bl
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst6e:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned156	; Nope!
		dec	dh			; Make it FFable
notSigned156:
		add	dx, [_z80ix]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop157:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead157
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr157		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine157

nextAddr157:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop157

callRoutine157:
		call	ReadMemoryByte	; Standard read routine
		mov	bl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit157

memoryRead157:
		mov	bl, [ebp + edx]	; Get our data

readExit157:
		mov	edi, [cyclesRemaining]
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst6f:
		mov   byte [_z80ix + 0], al
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst70:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned159	; Nope!
		dec	dh			; Make it FFable
notSigned159:
		add	dx, [_z80ix]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, ch	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop160:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite160	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr160	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine160	; If not, go call it!

nextAddr160:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop160

callRoutine160:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit160
memoryWrite160:
		mov	[ebp + edx], ch
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit160:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst71:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned161	; Nope!
		dec	dh			; Make it FFable
notSigned161:
		add	dx, [_z80ix]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, cl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop162:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite162	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr162	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine162	; If not, go call it!

nextAddr162:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop162

callRoutine162:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit162
memoryWrite162:
		mov	[ebp + edx], cl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit162:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst72:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned163	; Nope!
		dec	dh			; Make it FFable
notSigned163:
		add	dx, [_z80ix]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, byte [_z80de + 1]	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop164:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite164	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr164	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine164	; If not, go call it!

nextAddr164:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop164

callRoutine164:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit164
memoryWrite164:
		mov	edi, edx
		mov	dl, byte [_z80de + 1]
		mov	[ebp + edi], dl
		mov	edx, edi
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit164:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst73:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned165	; Nope!
		dec	dh			; Make it FFable
notSigned165:
		add	dx, [_z80ix]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, byte [_z80de]	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop166:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite166	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr166	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine166	; If not, go call it!

nextAddr166:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop166

callRoutine166:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit166
memoryWrite166:
		mov	edi, edx
		mov	dl, byte [_z80de]
		mov	[ebp + edi], dl
		mov	edx, edi
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit166:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst74:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned167	; Nope!
		dec	dh			; Make it FFable
notSigned167:
		add	dx, [_z80ix]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, bh	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop168:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite168	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr168	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine168	; If not, go call it!

nextAddr168:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop168

callRoutine168:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit168
memoryWrite168:
		mov	[ebp + edx], bh
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit168:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst75:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned169	; Nope!
		dec	dh			; Make it FFable
notSigned169:
		add	dx, [_z80ix]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, bl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop170:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite170	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr170	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine170	; If not, go call it!

nextAddr170:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop170

callRoutine170:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit170
memoryWrite170:
		mov	[ebp + edx], bl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit170:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst77:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned171	; Nope!
		dec	dh			; Make it FFable
notSigned171:
		add	dx, [_z80ix]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop172:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite172	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr172	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine172	; If not, go call it!

nextAddr172:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop172

callRoutine172:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit172
memoryWrite172:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit172:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst7c:
		mov	al, byte [_z80ix + 1]
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst7d:
		mov	al, byte [_z80ix + 0]
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst7e:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned173	; Nope!
		dec	dh			; Make it FFable
notSigned173:
		add	dx, [_z80ix]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop174:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead174
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr174		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine174

nextAddr174:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop174

callRoutine174:
		call	ReadMemoryByte	; Standard read routine
		mov	[_z80af], al	; Save our new accumulator
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit174

memoryRead174:
		mov	al, [ebp + edx]	; Get our data

readExit174:
		mov	edi, [cyclesRemaining]
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst84:
		mov	dl, byte [_z80ix + 1]
		sahf		; Store our flags in x86 flag reg
		add	al, dl
		lahf		; Get flags back into AH
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh ; No N!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst85:
		mov	dl, byte [_z80ix]
		sahf		; Store our flags in x86 flag reg
		add	al, dl
		lahf		; Get flags back into AH
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh ; No N!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst86:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned176	; Nope!
		dec	dh			; Make it FFable
notSigned176:
		add	dx, [_z80ix]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop177:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead177
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr177		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine177

nextAddr177:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop177

callRoutine177:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit177

memoryRead177:
		mov	dl, [ebp + edx]	; Get our data

readExit177:
		mov	edi, [cyclesRemaining]
		sahf
		add	al, dl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh	; Knock out negative
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst8c:
		mov	dl, byte [_z80ix + 1]
		sahf		; Store our flags in x86 flag reg
		adc	al, dl
		lahf		; Get flags back into AH
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh ; No N!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst8d:
		mov	dl, byte [_z80ix]
		sahf		; Store our flags in x86 flag reg
		adc	al, dl
		lahf		; Get flags back into AH
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh ; No N!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst8e:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned178	; Nope!
		dec	dh			; Make it FFable
notSigned178:
		add	dx, [_z80ix]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop179:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead179
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr179		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine179

nextAddr179:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop179

callRoutine179:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit179

memoryRead179:
		mov	dl, [ebp + edx]	; Get our data

readExit179:
		mov	edi, [cyclesRemaining]
		sahf
		adc	al, dl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh	; Knock out negative
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst94:
		mov	dl, byte [_z80ix + 1]
		sahf		; Store our flags in x86 flag reg
		sub	al, dl
		lahf		; Get flags back into AH
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; N Gets set!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst95:
		mov	dl, byte [_z80ix]
		sahf		; Store our flags in x86 flag reg
		sub	al, dl
		lahf		; Get flags back into AH
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; N Gets set!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst96:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned180	; Nope!
		dec	dh			; Make it FFable
notSigned180:
		add	dx, [_z80ix]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop181:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead181
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr181		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine181

nextAddr181:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop181

callRoutine181:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit181

memoryRead181:
		mov	dl, [ebp + edx]	; Get our data

readExit181:
		mov	edi, [cyclesRemaining]
		sahf
		sub	al, dl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set negative
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst9c:
		mov	dl, byte [_z80ix + 1]
		sahf		; Store our flags in x86 flag reg
		sbb	al, dl
		lahf		; Get flags back into AH
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; N Gets set!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst9d:
		mov	dl, byte [_z80ix]
		sahf		; Store our flags in x86 flag reg
		sbb	al, dl
		lahf		; Get flags back into AH
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; N Gets set!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInst9e:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned182	; Nope!
		dec	dh			; Make it FFable
notSigned182:
		add	dx, [_z80ix]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop183:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead183
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr183		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine183

nextAddr183:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop183

callRoutine183:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit183

memoryRead183:
		mov	dl, [ebp + edx]	; Get our data

readExit183:
		mov	edi, [cyclesRemaining]
		sahf
		sbb	al, dl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set negative
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInsta4:
		mov	dl, byte [_z80ix + 1]
		sahf		; Store our flags in x86 flag reg
		and	al, dl
		lahf		; Get flags back into AH
		and	ah, 0ech	; Only these flags matter!
		or	ah, 010h	; Half carry gets set
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInsta5:
		mov	dl, byte [_z80ix]
		sahf		; Store our flags in x86 flag reg
		and	al, dl
		lahf		; Get flags back into AH
		and	ah, 0ech	; Only these flags matter!
		or	ah, 010h	; Half carry gets set
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInsta6:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned184	; Nope!
		dec	dh			; Make it FFable
notSigned184:
		add	dx, [_z80ix]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop185:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead185
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr185		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine185

nextAddr185:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop185

callRoutine185:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit185

memoryRead185:
		mov	dl, [ebp + edx]	; Get our data

readExit185:
		mov	edi, [cyclesRemaining]
		sahf
		and	al, dl
		lahf
		and	ah,0fch	; Knock out N & C
		or	ah, 10h	; Set half carry
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInstac:
		mov	dl, byte [_z80ix + 1]
		sahf		; Store our flags in x86 flag reg
		xor	al, dl
		lahf		; Get flags back into AH
		and	ah, 0ech	; Only these flags matter!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInstad:
		mov	dl, byte [_z80ix]
		sahf		; Store our flags in x86 flag reg
		xor	al, dl
		lahf		; Get flags back into AH
		and	ah, 0ech	; Only these flags matter!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInstae:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned186	; Nope!
		dec	dh			; Make it FFable
notSigned186:
		add	dx, [_z80ix]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop187:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead187
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr187		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine187

nextAddr187:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop187

callRoutine187:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit187

memoryRead187:
		mov	dl, [ebp + edx]	; Get our data

readExit187:
		mov	edi, [cyclesRemaining]
		sahf
		xor	al, dl
		lahf
		and	ah, 0ech	; Knock out H, N, and C
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInstb4:
		mov	dl, byte [_z80ix + 1]
		sahf		; Store our flags in x86 flag reg
		or	al, dl
		lahf		; Get flags back into AH
		and	ah, 0ech ; No H, N, or C
		and	ah, 0ech ; No H, N, or C
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInstb5:
		mov	dl, byte [_z80ix]
		sahf		; Store our flags in x86 flag reg
		or	al, dl
		lahf		; Get flags back into AH
		and	ah, 0ech ; No H, N, or C
		and	ah, 0ech ; No H, N, or C
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInstb6:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned188	; Nope!
		dec	dh			; Make it FFable
notSigned188:
		add	dx, [_z80ix]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop189:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead189
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr189		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine189

nextAddr189:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop189

callRoutine189:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit189

memoryRead189:
		mov	dl, [ebp + edx]	; Get our data

readExit189:
		mov	edi, [cyclesRemaining]
		sahf
		or	al, dl
		lahf
		and	ah, 0ech	; Knock out H, N, and C
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInstbc:
		mov	dl, byte [_z80ix + 1]
		sahf		; Store our flags in x86 flag reg
		cmp	al, dl
		lahf		; Get flags back into AH
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Negative gets set on a compare
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInstbd:
		mov	dl, byte [_z80ix]
		sahf		; Store our flags in x86 flag reg
		cmp	al, dl
		lahf		; Get flags back into AH
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Negative gets set on a compare
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInstbe:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned190	; Nope!
		dec	dh			; Make it FFable
notSigned190:
		add	dx, [_z80ix]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop191:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead191
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr191		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine191

nextAddr191:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop191

callRoutine191:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit191

memoryRead191:
		mov	dl, [ebp + edx]	; Get our data

readExit191:
		mov	edi, [cyclesRemaining]
		sahf
		cmp	al, dl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set negative
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]

DDInstcb:
		mov	dx, [esi]	; Get our instruction (and offset)
		add	esi, 2	; Increment our PC
		mov	byte [_orgval], dl ; Store our value
		or	dl, dl
		js	notNeg192
		mov	byte [_orgval + 1], 00h;
 		jmp	short jumpHandler192
notNeg192:
		mov	byte [_orgval + 1], 0ffh;	It's negative
jumpHandler192:
		shl	ebx, 16	; Save BX away
		mov	bx, [_z80ix]
		add	[_orgval], bx
		shr	ebx, 16	; Restore BX
		mov	dl, dh	; Get our instruction
		xor	dh, dh	; Zero this
		jmp	dword [z80ddfdcbInstructions+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInste1:
		mov	dx, [_z80sp]
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop193:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead193
		cmp	dx, [edi]	; Are we smaller?
		jb		nextAddr193		; Yes, go to the next address
		cmp	dx, [edi+4]	; Are we bigger?
		jbe	callRoutine193

nextAddr193:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop193

callRoutine193:
		push	ax		; Save this for later
		push	dx		; Save address
		call	ReadMemoryByte	; Standard read routine
		pop	dx		; Restore our address
		inc	dx		; Next byte, please
		push	ax		; Save returned byte
		call	ReadMemoryByte	; Standard read routine
		xchg	ah, al	; Swap for endian's sake
		pop	dx	; Restore LSB
		mov	dh, ah	; Our word is now in DX
		pop	ax		; Restore this
		mov	[_z80ix], dx	; Store our word
		jmp	readExit193

memoryRead193:
		mov	dx, [ebp + edx]
		mov	[_z80ix], dx
readExit193:
		mov	edi, [cyclesRemaining]
		add	word [_z80sp], 2
		xor	edx, edx
		sub	edi, byte 14
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInste3:
		mov	[cyclesRemaining], edi
		mov	dx, word [_z80sp]
		xor	edi, edi
		mov	di, [_z80ix]
		xchg	di, [ebp+edx]
		mov	[_z80ix], di
		xor	edx, edx
		mov	edi, [cyclesRemaining]
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInste5:
		sub	word [_z80sp], 2
		mov	dx, [_z80sp]
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop194:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryWrite194
		cmp	dx, [edi]	; Are we smaller?
		jb		nextAddr194		; Yes, go to the next address
		cmp	dx, [edi+4]	; Are we bigger?
		jbe	callRoutine194

nextAddr194:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop194

callRoutine194:
		push	ax		; Save this for later
		push	dx
		mov	ax, [_z80ix]
		call	WriteMemoryByte
		pop	dx
		pop	ax
		inc	dx

		push	ax
		push	dx
		mov	ax, [_z80ix]
		xchg	ah, al
		call	WriteMemoryByte
		pop	dx
		pop	ax	; Restore us!
		jmp	writeExit194

memoryWrite194:
		mov	di, [_z80ix]
		mov	[ebp + edx], di	; Store our word
writeExit194:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInste9:
		mov	dx, [_z80ix]	; Get our value
		mov	esi, edx		; New PC!
		add	esi, ebp		; Add in our base
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDInstf9:
		mov	dx, [_z80ix] ; Get our source register
		mov	word [_z80sp], dx	; Store our new SP
		xor	edx, edx
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInst06:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop195:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead195
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr195		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine195

nextAddr195:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop195

callRoutine195:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit195

memoryRead195:
		mov	dl, [ebp + edx]	; Get our data

readExit195:
		mov	edi, [cyclesRemaining]
		sahf		; Restore our flags
		rol	dl, 1
		lahf		; Get our flags back
		and	ah, 0edh	; Knock out H & N
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop196:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite196	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr196	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine196	; If not, go call it!

nextAddr196:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop196

callRoutine196:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit196
memoryWrite196:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit196:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInst0e:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop197:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead197
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr197		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine197

nextAddr197:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop197

callRoutine197:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit197

memoryRead197:
		mov	dl, [ebp + edx]	; Get our data

readExit197:
		mov	edi, [cyclesRemaining]
		sahf		; Restore our flags
		ror	dl, 1
		lahf		; Get our flags back
		and	ah, 0edh	; Knock out H & N
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop198:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite198	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr198	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine198	; If not, go call it!

nextAddr198:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop198

callRoutine198:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit198
memoryWrite198:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit198:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInst16:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop199:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead199
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr199		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine199

nextAddr199:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop199

callRoutine199:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit199

memoryRead199:
		mov	dl, [ebp + edx]	; Get our data

readExit199:
		mov	edi, [cyclesRemaining]
		sahf		; Restore our flags
		rcl	dl, 1
		lahf		; Get our flags back
		and	ah, 0edh	; Knock out H & N
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop200:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite200	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr200	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine200	; If not, go call it!

nextAddr200:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop200

callRoutine200:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit200
memoryWrite200:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit200:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInst1e:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop201:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead201
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr201		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine201

nextAddr201:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop201

callRoutine201:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit201

memoryRead201:
		mov	dl, [ebp + edx]	; Get our data

readExit201:
		mov	edi, [cyclesRemaining]
		sahf		; Restore our flags
		rcr	dl, 1
		lahf		; Get our flags back
		and	ah, 0edh	; Knock out H & N
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop202:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite202	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr202	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine202	; If not, go call it!

nextAddr202:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop202

callRoutine202:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit202
memoryWrite202:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit202:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInst26:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop203:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead203
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr203		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine203

nextAddr203:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop203

callRoutine203:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit203

memoryRead203:
		mov	dl, [ebp + edx]	; Get our data

readExit203:
		mov	edi, [cyclesRemaining]
		sahf		; Restore our flags
		shl	dl, 1
		lahf		; Get our flags back
		and	ah, 0edh	; No Half carry or negative!
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop204:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite204	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr204	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine204	; If not, go call it!

nextAddr204:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop204

callRoutine204:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit204
memoryWrite204:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit204:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInst2e:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop205:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead205
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr205		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine205

nextAddr205:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop205

callRoutine205:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit205

memoryRead205:
		mov	dl, [ebp + edx]	; Get our data

readExit205:
		mov	edi, [cyclesRemaining]
		sahf		; Restore our flags
		sar	dl, 1
		lahf		; Get our flags back
		and	ah, 0edh	; No Half carry or negative!
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop206:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite206	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr206	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine206	; If not, go call it!

nextAddr206:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop206

callRoutine206:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit206
memoryWrite206:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit206:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInst3e:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop207:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead207
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr207		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine207

nextAddr207:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop207

callRoutine207:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit207

memoryRead207:
		mov	dl, [ebp + edx]	; Get our data

readExit207:
		mov	edi, [cyclesRemaining]
		sahf		; Restore our flags
		shr	dl, 1
		lahf		; Get our flags back
		and	ah, 0edh	; Knock out H & N
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop208:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite208	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr208	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine208	; If not, go call it!

nextAddr208:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop208

callRoutine208:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit208
memoryWrite208:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit208:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInst46:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop209:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead209
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr209		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine209

nextAddr209:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop209

callRoutine209:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit209

memoryRead209:
		mov	dl, [ebp + edx]	; Get our data

readExit209:
		mov	edi, [cyclesRemaining]
		mov	dh, ah	; Store our original flags
		and	dh, 29h	; Keep our old flags
		sahf		; Restore our flags
		test	dl, 001h	; Is it set?
		lahf		; Get our flags back
		and	ah, 0edh	; No Half carry or negative!
		or	ah, 10h	; OR In our half carry
		and	ah, 0d0h ; New flags
		or	ah, dh	; OR In our old flags
		xor	edx, edx
		sub	edi, byte 20
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInst4e:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop210:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead210
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr210		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine210

nextAddr210:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop210

callRoutine210:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit210

memoryRead210:
		mov	dl, [ebp + edx]	; Get our data

readExit210:
		mov	edi, [cyclesRemaining]
		mov	dh, ah	; Store our original flags
		and	dh, 29h	; Keep our old flags
		sahf		; Restore our flags
		test	dl, 002h	; Is it set?
		lahf		; Get our flags back
		and	ah, 0edh	; No Half carry or negative!
		or	ah, 10h	; OR In our half carry
		and	ah, 0d0h ; New flags
		or	ah, dh	; OR In our old flags
		xor	edx, edx
		sub	edi, byte 20
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInst56:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop211:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead211
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr211		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine211

nextAddr211:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop211

callRoutine211:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit211

memoryRead211:
		mov	dl, [ebp + edx]	; Get our data

readExit211:
		mov	edi, [cyclesRemaining]
		mov	dh, ah	; Store our original flags
		and	dh, 29h	; Keep our old flags
		sahf		; Restore our flags
		test	dl, 004h	; Is it set?
		lahf		; Get our flags back
		and	ah, 0edh	; No Half carry or negative!
		or	ah, 10h	; OR In our half carry
		and	ah, 0d0h ; New flags
		or	ah, dh	; OR In our old flags
		xor	edx, edx
		sub	edi, byte 20
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInst5e:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop212:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead212
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr212		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine212

nextAddr212:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop212

callRoutine212:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit212

memoryRead212:
		mov	dl, [ebp + edx]	; Get our data

readExit212:
		mov	edi, [cyclesRemaining]
		mov	dh, ah	; Store our original flags
		and	dh, 29h	; Keep our old flags
		sahf		; Restore our flags
		test	dl, 008h	; Is it set?
		lahf		; Get our flags back
		and	ah, 0edh	; No Half carry or negative!
		or	ah, 10h	; OR In our half carry
		and	ah, 0d0h ; New flags
		or	ah, dh	; OR In our old flags
		xor	edx, edx
		sub	edi, byte 20
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInst66:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop213:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead213
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr213		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine213

nextAddr213:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop213

callRoutine213:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit213

memoryRead213:
		mov	dl, [ebp + edx]	; Get our data

readExit213:
		mov	edi, [cyclesRemaining]
		mov	dh, ah	; Store our original flags
		and	dh, 29h	; Keep our old flags
		sahf		; Restore our flags
		test	dl, 010h	; Is it set?
		lahf		; Get our flags back
		and	ah, 0edh	; No Half carry or negative!
		or	ah, 10h	; OR In our half carry
		and	ah, 0d0h ; New flags
		or	ah, dh	; OR In our old flags
		xor	edx, edx
		sub	edi, byte 20
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInst6e:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop214:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead214
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr214		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine214

nextAddr214:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop214

callRoutine214:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit214

memoryRead214:
		mov	dl, [ebp + edx]	; Get our data

readExit214:
		mov	edi, [cyclesRemaining]
		mov	dh, ah	; Store our original flags
		and	dh, 29h	; Keep our old flags
		sahf		; Restore our flags
		test	dl, 020h	; Is it set?
		lahf		; Get our flags back
		and	ah, 0edh	; No Half carry or negative!
		or	ah, 10h	; OR In our half carry
		and	ah, 0d0h ; New flags
		or	ah, dh	; OR In our old flags
		xor	edx, edx
		sub	edi, byte 20
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInst76:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop215:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead215
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr215		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine215

nextAddr215:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop215

callRoutine215:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit215

memoryRead215:
		mov	dl, [ebp + edx]	; Get our data

readExit215:
		mov	edi, [cyclesRemaining]
		mov	dh, ah	; Store our original flags
		and	dh, 29h	; Keep our old flags
		sahf		; Restore our flags
		test	dl, 040h	; Is it set?
		lahf		; Get our flags back
		and	ah, 0edh	; No Half carry or negative!
		or	ah, 10h	; OR In our half carry
		and	ah, 0d0h ; New flags
		or	ah, dh	; OR In our old flags
		xor	edx, edx
		sub	edi, byte 20
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInst7e:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop216:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead216
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr216		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine216

nextAddr216:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop216

callRoutine216:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit216

memoryRead216:
		mov	dl, [ebp + edx]	; Get our data

readExit216:
		mov	edi, [cyclesRemaining]
		mov	dh, ah	; Store our original flags
		and	dh, 29h	; Keep our old flags
		sahf		; Restore our flags
		test	dl, 080h	; Is it set?
		lahf		; Get our flags back
		and	ah, 0edh	; No Half carry or negative!
		or	ah, 10h	; OR In our half carry
		and	ah, 0d0h ; New flags
		or	ah, dh	; OR In our old flags
		xor	edx, edx
		sub	edi, byte 20
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInst86:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop217:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead217
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr217		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine217

nextAddr217:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop217

callRoutine217:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit217

memoryRead217:
		mov	dl, [ebp + edx]	; Get our data

readExit217:
		mov	edi, [cyclesRemaining]
		and	dl, 0feh	; Reset the bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop218:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite218	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr218	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine218	; If not, go call it!

nextAddr218:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop218

callRoutine218:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit218
memoryWrite218:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit218:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInst8e:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop219:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead219
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr219		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine219

nextAddr219:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop219

callRoutine219:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit219

memoryRead219:
		mov	dl, [ebp + edx]	; Get our data

readExit219:
		mov	edi, [cyclesRemaining]
		and	dl, 0fdh	; Reset the bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop220:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite220	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr220	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine220	; If not, go call it!

nextAddr220:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop220

callRoutine220:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit220
memoryWrite220:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit220:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInst96:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop221:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead221
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr221		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine221

nextAddr221:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop221

callRoutine221:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit221

memoryRead221:
		mov	dl, [ebp + edx]	; Get our data

readExit221:
		mov	edi, [cyclesRemaining]
		and	dl, 0fbh	; Reset the bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop222:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite222	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr222	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine222	; If not, go call it!

nextAddr222:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop222

callRoutine222:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit222
memoryWrite222:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit222:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInst9e:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop223:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead223
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr223		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine223

nextAddr223:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop223

callRoutine223:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit223

memoryRead223:
		mov	dl, [ebp + edx]	; Get our data

readExit223:
		mov	edi, [cyclesRemaining]
		and	dl, 0f7h	; Reset the bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop224:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite224	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr224	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine224	; If not, go call it!

nextAddr224:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop224

callRoutine224:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit224
memoryWrite224:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit224:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInsta6:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop225:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead225
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr225		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine225

nextAddr225:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop225

callRoutine225:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit225

memoryRead225:
		mov	dl, [ebp + edx]	; Get our data

readExit225:
		mov	edi, [cyclesRemaining]
		and	dl, 0efh	; Reset the bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop226:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite226	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr226	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine226	; If not, go call it!

nextAddr226:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop226

callRoutine226:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit226
memoryWrite226:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit226:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInstae:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop227:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead227
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr227		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine227

nextAddr227:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop227

callRoutine227:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit227

memoryRead227:
		mov	dl, [ebp + edx]	; Get our data

readExit227:
		mov	edi, [cyclesRemaining]
		and	dl, 0dfh	; Reset the bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop228:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite228	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr228	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine228	; If not, go call it!

nextAddr228:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop228

callRoutine228:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit228
memoryWrite228:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit228:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInstb6:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop229:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead229
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr229		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine229

nextAddr229:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop229

callRoutine229:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit229

memoryRead229:
		mov	dl, [ebp + edx]	; Get our data

readExit229:
		mov	edi, [cyclesRemaining]
		and	dl, 0bfh	; Reset the bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop230:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite230	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr230	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine230	; If not, go call it!

nextAddr230:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop230

callRoutine230:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit230
memoryWrite230:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit230:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInstbe:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop231:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead231
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr231		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine231

nextAddr231:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop231

callRoutine231:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit231

memoryRead231:
		mov	dl, [ebp + edx]	; Get our data

readExit231:
		mov	edi, [cyclesRemaining]
		and	dl, 07fh	; Reset the bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop232:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite232	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr232	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine232	; If not, go call it!

nextAddr232:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop232

callRoutine232:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit232
memoryWrite232:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit232:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInstc6:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop233:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead233
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr233		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine233

nextAddr233:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop233

callRoutine233:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit233

memoryRead233:
		mov	dl, [ebp + edx]	; Get our data

readExit233:
		mov	edi, [cyclesRemaining]
		or	dl, 001h	; Set the bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop234:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite234	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr234	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine234	; If not, go call it!

nextAddr234:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop234

callRoutine234:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit234
memoryWrite234:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit234:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInstce:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop235:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead235
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr235		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine235

nextAddr235:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop235

callRoutine235:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit235

memoryRead235:
		mov	dl, [ebp + edx]	; Get our data

readExit235:
		mov	edi, [cyclesRemaining]
		or	dl, 002h	; Set the bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop236:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite236	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr236	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine236	; If not, go call it!

nextAddr236:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop236

callRoutine236:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit236
memoryWrite236:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit236:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInstd6:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop237:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead237
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr237		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine237

nextAddr237:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop237

callRoutine237:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit237

memoryRead237:
		mov	dl, [ebp + edx]	; Get our data

readExit237:
		mov	edi, [cyclesRemaining]
		or	dl, 004h	; Set the bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop238:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite238	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr238	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine238	; If not, go call it!

nextAddr238:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop238

callRoutine238:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit238
memoryWrite238:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit238:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInstde:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop239:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead239
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr239		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine239

nextAddr239:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop239

callRoutine239:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit239

memoryRead239:
		mov	dl, [ebp + edx]	; Get our data

readExit239:
		mov	edi, [cyclesRemaining]
		or	dl, 008h	; Set the bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop240:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite240	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr240	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine240	; If not, go call it!

nextAddr240:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop240

callRoutine240:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit240
memoryWrite240:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit240:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInste6:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop241:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead241
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr241		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine241

nextAddr241:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop241

callRoutine241:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit241

memoryRead241:
		mov	dl, [ebp + edx]	; Get our data

readExit241:
		mov	edi, [cyclesRemaining]
		or	dl, 010h	; Set the bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop242:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite242	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr242	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine242	; If not, go call it!

nextAddr242:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop242

callRoutine242:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit242
memoryWrite242:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit242:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInstee:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop243:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead243
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr243		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine243

nextAddr243:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop243

callRoutine243:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit243

memoryRead243:
		mov	dl, [ebp + edx]	; Get our data

readExit243:
		mov	edi, [cyclesRemaining]
		or	dl, 020h	; Set the bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop244:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite244	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr244	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine244	; If not, go call it!

nextAddr244:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop244

callRoutine244:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit244
memoryWrite244:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit244:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInstf6:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop245:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead245
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr245		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine245

nextAddr245:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop245

callRoutine245:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit245

memoryRead245:
		mov	dl, [ebp + edx]	; Get our data

readExit245:
		mov	edi, [cyclesRemaining]
		or	dl, 040h	; Set the bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop246:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite246	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr246	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine246	; If not, go call it!

nextAddr246:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop246

callRoutine246:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit246
memoryWrite246:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit246:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

DDFDCBInstfe:
		mov	dx, [_orgval]	; Get our target address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop247:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead247
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr247		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine247

nextAddr247:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop247

callRoutine247:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit247

memoryRead247:
		mov	dl, [ebp + edx]	; Get our data

readExit247:
		mov	edi, [cyclesRemaining]
		or	dl, 080h	; Set the bit
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop248:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite248	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr248	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine248	; If not, go call it!

nextAddr248:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop248

callRoutine248:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit248
memoryWrite248:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit248:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst09:
		mov	dh, ah	; Get our flags
		and	dh, 0ech	; Preserve the top three and bits 2 & 3
		mov	[cyclesRemaining], edi
		mov	di, [_z80iy]	; Get our value
		mov	[_orgval], di	; Store our original value
		add	di, cx
		lahf
		mov	[_z80iy], di	; Store our register back
		mov	di, [_orgval]	; Get original
		xor	di, word [_z80iy] ; XOR It with our computed value
		xor	di, cx
		and	di, 1000h	; Just our half carry
		or		dx, di	; Or in our flags
		and	ah, 01h	; Just carry
		or	ah, dh
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst19:
		mov	dh, ah	; Get our flags
		and	dh, 0ech	; Preserve the top three and bits 2 & 3
		mov	[cyclesRemaining], edi
		mov	di, [_z80iy]	; Get our value
		mov	[_orgval], di	; Store our original value
		add	di, word [_z80de]
		lahf
		mov	[_z80iy], di	; Store our register back
		mov	di, [_orgval]	; Get original
		xor	di, word [_z80iy] ; XOR It with our computed value
		xor	di, word [_z80de]
		and	di, 1000h	; Just our half carry
		or		dx, di	; Or in our flags
		and	ah, 01h	; Just carry
		or	ah, dh
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst21:
		mov	dx, [esi]	; Get our word to load
		add	esi, 2	; Advance past the word
		mov	[_z80iy], dx ; Store our new value
		xor	edx, edx
		sub	edi, byte 14
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst22:
		mov	dx, [esi]	 ; Get our address to store
		add	esi, 2
		mov	[_orgval], dx
		mov	dl, [_z80iy]
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop249:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite249	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr249	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine249	; If not, go call it!

nextAddr249:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop249

callRoutine249:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit249
memoryWrite249:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit249:
		mov	edi, [cyclesRemaining]
		inc	word [_orgval]
		mov	dl, [_z80iy + 1]
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop250:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite250	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr250	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine250	; If not, go call it!

nextAddr250:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop250

callRoutine250:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit250
memoryWrite250:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit250:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 20
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst23:
		inc	word [_z80iy]	; Increment our mz80Index register
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst24:
		sahf
		inc	byte [_z80iy + 1]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh	; Knock out N!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst25:
		sahf
		dec	byte [_z80iy + 1]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set negative!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst26:
		mov	dl, [esi]	; Get immediate byte to load
		inc	esi	; Next byte
		mov	byte [_z80iy + 1], dl
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst29:
		mov	dh, ah	; Get our flags
		and	dh, 0ech	; Preserve the top three and bits 2 & 3
		mov	[cyclesRemaining], edi
		mov	di, [_z80iy]	; Get our value
		mov	[_orgval], di	; Store our original value
		add	di, di
		lahf
		mov	[_z80iy], di	; Store our register back
		mov	di, [_orgval]	; Get original
		xor	di, word [_z80iy] ; XOR It with our computed value
		xor	di, di
		and	di, 1000h	; Just our half carry
		or		dx, di	; Or in our flags
		and	ah, 01h	; Just carry
		or	ah, dh
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst2a:
		mov	dx, [esi]	 ; Get our address to store
		add	esi, 2
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop251:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead251
		cmp	dx, [edi]	; Are we smaller?
		jb		nextAddr251		; Yes, go to the next address
		cmp	dx, [edi+4]	; Are we bigger?
		jbe	callRoutine251

nextAddr251:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop251

callRoutine251:
		push	ax		; Save this for later
		push	dx		; Save address
		call	ReadMemoryByte	; Standard read routine
		pop	dx		; Restore our address
		inc	dx		; Next byte, please
		push	ax		; Save returned byte
		call	ReadMemoryByte	; Standard read routine
		xchg	ah, al	; Swap for endian's sake
		pop	dx	; Restore LSB
		mov	dh, ah	; Our word is now in DX
		pop	ax		; Restore this
		mov	[_z80iy], dx	; Store our word
		jmp	readExit251

memoryRead251:
		mov	dx, [ebp + edx]
		mov	[_z80iy], dx
readExit251:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 20
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst2b:
		dec	word [_z80iy]	; Increment our mz80Index register
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst2c:
		sahf
		inc	byte [_z80iy]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh	; Knock out N!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst2d:
		sahf
		dec	byte [_z80iy]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set negative!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst2e:
		mov	dl, [esi]	; Get immediate byte to load
		inc	esi	; Next byte
		mov	byte [_z80iy], dl
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst34:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned252	; Nope!
		dec	dh			; Make it FFable
notSigned252:
		add	dx, [_z80iy]	; Our offset!
		mov	[_orgval], dx
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop253:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead253
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr253		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine253

nextAddr253:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop253

callRoutine253:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit253

memoryRead253:
		mov	dl, [ebp + edx]	; Get our data

readExit253:
		mov	edi, [cyclesRemaining]
		sahf
		inc	dl
		lahf
		o16	pushf
		shl	edx, 16
		and	ah, 0fbh	;	Knock out parity/overflow
		pop	dx
		and	dh, 08h ; Just the overflow
		shr	dh, 1	; Shift it into position
		or	ah, dh	; OR It in with the real flags
		shr	edx, 16
		and	ah, 0fdh	; Knock out N!
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop254:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite254	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr254	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine254	; If not, go call it!

nextAddr254:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop254

callRoutine254:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit254
memoryWrite254:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit254:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst35:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned255	; Nope!
		dec	dh			; Make it FFable
notSigned255:
		add	dx, [_z80iy]	; Our offset!
		mov	[_orgval], dx
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop256:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead256
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr256		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine256

nextAddr256:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop256

callRoutine256:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit256

memoryRead256:
		mov	dl, [ebp + edx]	; Get our data

readExit256:
		mov	edi, [cyclesRemaining]
		sahf
		dec	dl
		lahf
		o16	pushf
		shl	edx, 16
		and	ah, 0fbh	;	Knock out parity/overflow
		pop	dx
		and	dh, 08h ; Just the overflow
		shr	dh, 1	; Shift it into position
		or	ah, dh	; OR It in with the real flags
		shr	edx, 16
		or		ah, 02h	; Make it N!
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop257:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite257	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr257	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine257	; If not, go call it!

nextAddr257:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop257

callRoutine257:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit257
memoryWrite257:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit257:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst36:
		mov	dx, [esi]	; Get our address
		add	esi, 2	; Skip over our storage bytes
		mov	[cyclesRemaining], edi
		mov	di, dx	; Store it here for later
		xor	dh, dh
		or	dl, dl
		jns	noNegate258
		dec	dh
noNegate258:
		add	dx, [_z80iy]	; Add in our index
		mov	[_orgval], dx	; Store our address to write to
		mov	dx, di
		xchg	dh, dl
		mov	edi, [cyclesRemaining]
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	dx, [_orgval]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop258:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite258	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr258	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine258	; If not, go call it!

nextAddr258:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop258

callRoutine258:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit258
memoryWrite258:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit258:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst39:
		mov	dh, ah	; Get our flags
		and	dh, 0ech	; Preserve the top three and bits 2 & 3
		mov	[cyclesRemaining], edi
		mov	di, [_z80iy]	; Get our value
		mov	[_orgval], di	; Store our original value
		add	di, word [_z80sp]
		lahf
		mov	[_z80iy], di	; Store our register back
		mov	di, [_orgval]	; Get original
		xor	di, word [_z80iy] ; XOR It with our computed value
		xor	di, word [_z80sp]
		and	di, 1000h	; Just our half carry
		or		dx, di	; Or in our flags
		and	ah, 01h	; Just carry
		or	ah, dh
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst44:
		mov	ch, byte [_z80iy + 1]
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst45:
		mov	ch, byte [_z80iy + 0]
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst46:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned260	; Nope!
		dec	dh			; Make it FFable
notSigned260:
		add	dx, [_z80iy]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop261:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead261
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr261		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine261

nextAddr261:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop261

callRoutine261:
		call	ReadMemoryByte	; Standard read routine
		mov	ch, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit261

memoryRead261:
		mov	ch, [ebp + edx]	; Get our data

readExit261:
		mov	edi, [cyclesRemaining]
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst4c:
		mov	cl, byte [_z80iy + 1]
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst4d:
		mov	cl, byte [_z80iy + 0]
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst4e:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned263	; Nope!
		dec	dh			; Make it FFable
notSigned263:
		add	dx, [_z80iy]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop264:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead264
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr264		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine264

nextAddr264:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop264

callRoutine264:
		call	ReadMemoryByte	; Standard read routine
		mov	cl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit264

memoryRead264:
		mov	cl, [ebp + edx]	; Get our data

readExit264:
		mov	edi, [cyclesRemaining]
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst54:
		mov	dx, [_z80de]	; Get a usable copy of DE here
		mov	dh, byte [_z80iy + 1]
		mov	[_z80de], dx	; Put it back!
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst55:
		mov	dx, [_z80de]	; Get a usable copy of DE here
		mov	dh, byte [_z80iy + 0]
		mov	[_z80de], dx	; Put it back!
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst56:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned266	; Nope!
		dec	dh			; Make it FFable
notSigned266:
		add	dx, [_z80iy]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop267:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead267
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr267		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine267

nextAddr267:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop267

callRoutine267:
		call	ReadMemoryByte	; Standard read routine
		mov	byte [_z80de + 1], al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit267

memoryRead267:
		mov	di, dx
		mov	dl, [ebp + edx]
		mov	byte [_z80de + 1], dl
		mov	dx, di
readExit267:
		mov	edi, [cyclesRemaining]
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst5c:
		mov	dx, [_z80de]	; Get a usable copy of DE here
		mov	dl, byte [_z80iy + 1]
		mov	[_z80de], dx	; Put it back!
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst5d:
		mov	dx, [_z80de]	; Get a usable copy of DE here
		mov	dl, byte [_z80iy + 0]
		mov	[_z80de], dx	; Put it back!
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst5e:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned269	; Nope!
		dec	dh			; Make it FFable
notSigned269:
		add	dx, [_z80iy]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop270:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead270
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr270		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine270

nextAddr270:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop270

callRoutine270:
		call	ReadMemoryByte	; Standard read routine
		mov	byte [_z80de], al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit270

memoryRead270:
		mov	di, dx
		mov	dl, [ebp + edx]
		mov	byte [_z80de], dl
		mov	dx, di
readExit270:
		mov	edi, [cyclesRemaining]
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst60:
		mov   byte [_z80iy + 1], ch
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst61:
		mov   byte [_z80iy + 1], cl
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst62:
	mov	dx, [_z80de]	; Get DE
		mov   byte [_z80iy + 1], dh
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst63:
	mov	dx, [_z80de]	; Get DE
		mov   byte [_z80iy + 1], dl
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst64:
	mov	dh, byte [_z80iy + 1]
		mov   byte [_z80iy + 1], bh
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst65:
	mov	dl, byte [_z80iy]
		mov   byte [_z80iy + 1], bl
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst66:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned272	; Nope!
		dec	dh			; Make it FFable
notSigned272:
		add	dx, [_z80iy]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop273:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead273
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr273		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine273

nextAddr273:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop273

callRoutine273:
		call	ReadMemoryByte	; Standard read routine
		mov	bh, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit273

memoryRead273:
		mov	bh, [ebp + edx]	; Get our data

readExit273:
		mov	edi, [cyclesRemaining]
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst67:
		mov   byte [_z80iy + 1], al
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst68:
		mov   byte [_z80iy + 0], ch
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst69:
		mov   byte [_z80iy + 0], cl
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst6a:
	mov	dx, [_z80de]	; Get DE
		mov   byte [_z80iy + 0], dh
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst6b:
	mov	dx, [_z80de]	; Get DE
		mov   byte [_z80iy + 0], dl
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst6c:
	mov	dh, byte [_z80iy + 1]
		mov   byte [_z80iy + 0], bh
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst6d:
	mov	dl, byte [_z80iy]
		mov   byte [_z80iy + 0], bl
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst6e:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned275	; Nope!
		dec	dh			; Make it FFable
notSigned275:
		add	dx, [_z80iy]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop276:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead276
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr276		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine276

nextAddr276:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop276

callRoutine276:
		call	ReadMemoryByte	; Standard read routine
		mov	bl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit276

memoryRead276:
		mov	bl, [ebp + edx]	; Get our data

readExit276:
		mov	edi, [cyclesRemaining]
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst6f:
		mov   byte [_z80iy + 0], al
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst70:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned278	; Nope!
		dec	dh			; Make it FFable
notSigned278:
		add	dx, [_z80iy]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, ch	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop279:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite279	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr279	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine279	; If not, go call it!

nextAddr279:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop279

callRoutine279:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit279
memoryWrite279:
		mov	[ebp + edx], ch
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit279:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst71:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned280	; Nope!
		dec	dh			; Make it FFable
notSigned280:
		add	dx, [_z80iy]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, cl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop281:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite281	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr281	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine281	; If not, go call it!

nextAddr281:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop281

callRoutine281:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit281
memoryWrite281:
		mov	[ebp + edx], cl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit281:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst72:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned282	; Nope!
		dec	dh			; Make it FFable
notSigned282:
		add	dx, [_z80iy]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, byte [_z80de + 1]	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop283:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite283	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr283	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine283	; If not, go call it!

nextAddr283:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop283

callRoutine283:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit283
memoryWrite283:
		mov	edi, edx
		mov	dl, byte [_z80de + 1]
		mov	[ebp + edi], dl
		mov	edx, edi
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit283:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst73:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned284	; Nope!
		dec	dh			; Make it FFable
notSigned284:
		add	dx, [_z80iy]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, byte [_z80de]	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop285:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite285	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr285	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine285	; If not, go call it!

nextAddr285:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop285

callRoutine285:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit285
memoryWrite285:
		mov	edi, edx
		mov	dl, byte [_z80de]
		mov	[ebp + edi], dl
		mov	edx, edi
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit285:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst74:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned286	; Nope!
		dec	dh			; Make it FFable
notSigned286:
		add	dx, [_z80iy]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, bh	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop287:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite287	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr287	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine287	; If not, go call it!

nextAddr287:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop287

callRoutine287:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit287
memoryWrite287:
		mov	[ebp + edx], bh
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit287:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst75:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned288	; Nope!
		dec	dh			; Make it FFable
notSigned288:
		add	dx, [_z80iy]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, bl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop289:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite289	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr289	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine289	; If not, go call it!

nextAddr289:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop289

callRoutine289:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit289
memoryWrite289:
		mov	[ebp + edx], bl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit289:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst77:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned290	; Nope!
		dec	dh			; Make it FFable
notSigned290:
		add	dx, [_z80iy]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop291:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite291	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr291	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine291	; If not, go call it!

nextAddr291:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop291

callRoutine291:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit291
memoryWrite291:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit291:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst7c:
		mov	al, byte [_z80iy + 1]
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst7d:
		mov	al, byte [_z80iy + 0]
		xor	edx, edx
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst7e:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned292	; Nope!
		dec	dh			; Make it FFable
notSigned292:
		add	dx, [_z80iy]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop293:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead293
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr293		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine293

nextAddr293:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop293

callRoutine293:
		call	ReadMemoryByte	; Standard read routine
		mov	[_z80af], al	; Save our new accumulator
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit293

memoryRead293:
		mov	al, [ebp + edx]	; Get our data

readExit293:
		mov	edi, [cyclesRemaining]
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst84:
		mov	dl, byte [_z80iy + 1]
		sahf		; Store our flags in x86 flag reg
		add	al, dl
		lahf		; Get flags back into AH
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh ; No N!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst85:
		mov	dl, byte [_z80iy]
		sahf		; Store our flags in x86 flag reg
		add	al, dl
		lahf		; Get flags back into AH
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh ; No N!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst86:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned295	; Nope!
		dec	dh			; Make it FFable
notSigned295:
		add	dx, [_z80iy]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop296:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead296
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr296		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine296

nextAddr296:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop296

callRoutine296:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit296

memoryRead296:
		mov	dl, [ebp + edx]	; Get our data

readExit296:
		mov	edi, [cyclesRemaining]
		sahf
		add	al, dl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh	; Knock out negative
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst8c:
		mov	dl, byte [_z80iy + 1]
		sahf		; Store our flags in x86 flag reg
		adc	al, dl
		lahf		; Get flags back into AH
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh ; No N!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst8d:
		mov	dl, byte [_z80iy]
		sahf		; Store our flags in x86 flag reg
		adc	al, dl
		lahf		; Get flags back into AH
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh ; No N!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst8e:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned297	; Nope!
		dec	dh			; Make it FFable
notSigned297:
		add	dx, [_z80iy]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop298:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead298
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr298		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine298

nextAddr298:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop298

callRoutine298:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit298

memoryRead298:
		mov	dl, [ebp + edx]	; Get our data

readExit298:
		mov	edi, [cyclesRemaining]
		sahf
		adc	al, dl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh	; Knock out negative
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst94:
		mov	dl, byte [_z80iy + 1]
		sahf		; Store our flags in x86 flag reg
		sub	al, dl
		lahf		; Get flags back into AH
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; N Gets set!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst95:
		mov	dl, byte [_z80iy]
		sahf		; Store our flags in x86 flag reg
		sub	al, dl
		lahf		; Get flags back into AH
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; N Gets set!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst96:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned299	; Nope!
		dec	dh			; Make it FFable
notSigned299:
		add	dx, [_z80iy]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop300:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead300
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr300		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine300

nextAddr300:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop300

callRoutine300:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit300

memoryRead300:
		mov	dl, [ebp + edx]	; Get our data

readExit300:
		mov	edi, [cyclesRemaining]
		sahf
		sub	al, dl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set negative
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst9c:
		mov	dl, byte [_z80iy + 1]
		sahf		; Store our flags in x86 flag reg
		sbb	al, dl
		lahf		; Get flags back into AH
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; N Gets set!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst9d:
		mov	dl, byte [_z80iy]
		sahf		; Store our flags in x86 flag reg
		sbb	al, dl
		lahf		; Get flags back into AH
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; N Gets set!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInst9e:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned301	; Nope!
		dec	dh			; Make it FFable
notSigned301:
		add	dx, [_z80iy]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop302:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead302
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr302		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine302

nextAddr302:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop302

callRoutine302:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit302

memoryRead302:
		mov	dl, [ebp + edx]	; Get our data

readExit302:
		mov	edi, [cyclesRemaining]
		sahf
		sbb	al, dl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set negative
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInsta4:
		mov	dl, byte [_z80iy + 1]
		sahf		; Store our flags in x86 flag reg
		and	al, dl
		lahf		; Get flags back into AH
		and	ah, 0ech	; Only these flags matter!
		or	ah, 010h	; Half carry gets set
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInsta5:
		mov	dl, byte [_z80iy]
		sahf		; Store our flags in x86 flag reg
		and	al, dl
		lahf		; Get flags back into AH
		and	ah, 0ech	; Only these flags matter!
		or	ah, 010h	; Half carry gets set
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInsta6:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned303	; Nope!
		dec	dh			; Make it FFable
notSigned303:
		add	dx, [_z80iy]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop304:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead304
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr304		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine304

nextAddr304:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop304

callRoutine304:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit304

memoryRead304:
		mov	dl, [ebp + edx]	; Get our data

readExit304:
		mov	edi, [cyclesRemaining]
		sahf
		and	al, dl
		lahf
		and	ah,0fch	; Knock out N & C
		or	ah, 10h	; Set half carry
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInstac:
		mov	dl, byte [_z80iy + 1]
		sahf		; Store our flags in x86 flag reg
		xor	al, dl
		lahf		; Get flags back into AH
		and	ah, 0ech	; Only these flags matter!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInstad:
		mov	dl, byte [_z80iy]
		sahf		; Store our flags in x86 flag reg
		xor	al, dl
		lahf		; Get flags back into AH
		and	ah, 0ech	; Only these flags matter!
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInstae:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned305	; Nope!
		dec	dh			; Make it FFable
notSigned305:
		add	dx, [_z80iy]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop306:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead306
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr306		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine306

nextAddr306:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop306

callRoutine306:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit306

memoryRead306:
		mov	dl, [ebp + edx]	; Get our data

readExit306:
		mov	edi, [cyclesRemaining]
		sahf
		xor	al, dl
		lahf
		and	ah, 0ech	; Knock out H, N, and C
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInstb4:
		mov	dl, byte [_z80iy + 1]
		sahf		; Store our flags in x86 flag reg
		or	al, dl
		lahf		; Get flags back into AH
		and	ah, 0ech ; No H, N, or C
		and	ah, 0ech ; No H, N, or C
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInstb5:
		mov	dl, byte [_z80iy]
		sahf		; Store our flags in x86 flag reg
		or	al, dl
		lahf		; Get flags back into AH
		and	ah, 0ech ; No H, N, or C
		and	ah, 0ech ; No H, N, or C
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInstb6:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned307	; Nope!
		dec	dh			; Make it FFable
notSigned307:
		add	dx, [_z80iy]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop308:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead308
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr308		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine308

nextAddr308:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop308

callRoutine308:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit308

memoryRead308:
		mov	dl, [ebp + edx]	; Get our data

readExit308:
		mov	edi, [cyclesRemaining]
		sahf
		or	al, dl
		lahf
		and	ah, 0ech	; Knock out H, N, and C
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInstbc:
		mov	dl, byte [_z80iy + 1]
		sahf		; Store our flags in x86 flag reg
		cmp	al, dl
		lahf		; Get flags back into AH
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Negative gets set on a compare
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInstbd:
		mov	dl, byte [_z80iy]
		sahf		; Store our flags in x86 flag reg
		cmp	al, dl
		lahf		; Get flags back into AH
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Negative gets set on a compare
		sub	edi, byte 9
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInstbe:
		mov	dl, [esi]	; Fetch our offset
		inc	esi		; Move past the offset
		or	dl, dl		; Is this bad boy signed?
		jns	notSigned309	; Nope!
		dec	dh			; Make it FFable
notSigned309:
		add	dx, [_z80iy]	; Our offset!
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop310:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead310
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr310		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine310

nextAddr310:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop310

callRoutine310:
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit310

memoryRead310:
		mov	dl, [ebp + edx]	; Get our data

readExit310:
		mov	edi, [cyclesRemaining]
		sahf
		cmp	al, dl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set negative
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]

FDInstcb:
		mov	dx, [esi]	; Get our instruction (and offset)
		add	esi, 2	; Increment our PC
		mov	byte [_orgval], dl ; Store our value
		or	dl, dl
		js	notNeg311
		mov	byte [_orgval + 1], 00h;
 		jmp	short jumpHandler311
notNeg311:
		mov	byte [_orgval + 1], 0ffh;	It's negative
jumpHandler311:
		shl	ebx, 16	; Save BX away
		mov	bx, [_z80iy]
		add	[_orgval], bx
		shr	ebx, 16	; Restore BX
		mov	dl, dh	; Get our instruction
		xor	dh, dh	; Zero this
		jmp	dword [z80ddfdcbInstructions+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInste1:
		mov	dx, [_z80sp]
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop312:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead312
		cmp	dx, [edi]	; Are we smaller?
		jb		nextAddr312		; Yes, go to the next address
		cmp	dx, [edi+4]	; Are we bigger?
		jbe	callRoutine312

nextAddr312:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop312

callRoutine312:
		push	ax		; Save this for later
		push	dx		; Save address
		call	ReadMemoryByte	; Standard read routine
		pop	dx		; Restore our address
		inc	dx		; Next byte, please
		push	ax		; Save returned byte
		call	ReadMemoryByte	; Standard read routine
		xchg	ah, al	; Swap for endian's sake
		pop	dx	; Restore LSB
		mov	dh, ah	; Our word is now in DX
		pop	ax		; Restore this
		mov	[_z80iy], dx	; Store our word
		jmp	readExit312

memoryRead312:
		mov	dx, [ebp + edx]
		mov	[_z80iy], dx
readExit312:
		mov	edi, [cyclesRemaining]
		add	word [_z80sp], 2
		xor	edx, edx
		sub	edi, byte 14
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInste3:
		mov	[cyclesRemaining], edi
		mov	dx, word [_z80sp]
		xor	edi, edi
		mov	di, [_z80iy]
		xchg	di, [ebp+edx]
		mov	[_z80iy], di
		xor	edx, edx
		mov	edi, [cyclesRemaining]
		sub	edi, byte 23
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInste5:
		sub	word [_z80sp], 2
		mov	dx, [_z80sp]
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop313:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryWrite313
		cmp	dx, [edi]	; Are we smaller?
		jb		nextAddr313		; Yes, go to the next address
		cmp	dx, [edi+4]	; Are we bigger?
		jbe	callRoutine313

nextAddr313:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop313

callRoutine313:
		push	ax		; Save this for later
		push	dx
		mov	ax, [_z80iy]
		call	WriteMemoryByte
		pop	dx
		pop	ax
		inc	dx

		push	ax
		push	dx
		mov	ax, [_z80iy]
		xchg	ah, al
		call	WriteMemoryByte
		pop	dx
		pop	ax	; Restore us!
		jmp	writeExit313

memoryWrite313:
		mov	di, [_z80iy]
		mov	[ebp + edx], di	; Store our word
writeExit313:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 15
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInste9:
		mov	dx, [_z80iy]	; Get our value
		mov	esi, edx		; New PC!
		add	esi, ebp		; Add in our base
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

FDInstf9:
		mov	dx, [_z80iy] ; Get our source register
		mov	word [_z80sp], dx	; Store our new SP
		xor	edx, edx
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst00:
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst01:
		mov	cx, [esi]	; Get our immediate value of BC
		add	esi, 2
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst02:
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop314:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite314	; Yes - go write it!
		cmp	cx, [edi]	; Are we smaller?
		jb	nextAddr314	; Yes... go to the next addr
		cmp	cx, [edi+4]	; Are we smaller?
		jbe	callRoutine314	; If not, go call it!

nextAddr314:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop314

callRoutine314:
		mov	dx, cx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit314
memoryWrite314:
		mov	[ebp + ecx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit314:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst03:
		inc	cx
		sub	edi, byte 6
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst04:
		sahf
		inc	ch
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh	; Knock out N!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst05:
		sahf
		dec	ch
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set negative!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst06:
		mov	ch, [esi]	; Get our immediate value
		inc	esi
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst07:
		sahf
		rol	al, 1
		lahf
		and	ah, 0edh
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst08:
		xchg	ah, al
		xchg	ax, [_z80afprime]
		xchg	ah, al
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst09:
		mov	dh, ah	; Get our flags
		and	dh, 0ech	; Preserve the top three and bits 2 & 3
		mov	[_orgval], bx	; Store our original value
		add	bx, cx
		lahf
		mov	[cyclesRemaining], edi
		mov	di, [_orgval]	; Get original
		xor	di, bx ; XOR It with our computed value
		xor	di, cx
		and	di, 1000h	; Just our half carry
		or		dx, di	; Or in our flags
		and	ah, 01h	; Just carry
		or	ah, dh
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 11
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst0a:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop315:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead315
		cmp	ecx, [edi]	; Are we smaller?
		jb		nextAddr315		; Yes, go to the next address
		cmp	ecx, [edi+4]	; Are we bigger?
		jbe	callRoutine315

nextAddr315:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop315

callRoutine315:
		mov	dx, cx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	[_z80af], al	; Save our new accumulator
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit315

memoryRead315:
		mov	al, [ebp + ecx]	; Get our data

readExit315:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst0b:
		dec	cx
		sub	edi, byte 6
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst0c:
		sahf
		inc	cl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh	; Knock out N!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst0d:
		sahf
		dec	cl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set negative!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst0e:
		mov	cl, [esi]	; Get our immediate value
		inc	esi
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst0f:
		sahf
		ror	al, 1
		lahf
		and	ah, 0edh
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst10:
		mov	dl, [esi] ; Get our relative offset
		inc	esi	; Next instruction, please!
		dec	ch	; Decrement B
		jz	noJump	; Don't take the jump if it's done!
; Otherwise, take the jump
		sub	edi, 5
		xchg	eax, edx
		cbw
		xchg 	eax, edx
		sub	esi, ebp
		add	si, dx
		add	esi, ebp
noJump:
		xor	edx, edx
		sub	edi, byte 8
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst11:
		mov	dx, [esi]	; Get our immediate value of DE
		mov	word [_z80de], dx ; Store DE
		xor	edx, edx
		add	esi, 2
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst12:
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	dx, [_z80de]
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop316:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite316	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr316	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine316	; If not, go call it!

nextAddr316:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop316

callRoutine316:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit316
memoryWrite316:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit316:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst13:
		inc	word [_z80de]
		sub	edi, byte 6
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst14:
		sahf
		inc	byte [_z80de + 1]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh	; Knock out N!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst15:
		sahf
		dec	byte [_z80de + 1]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set negative!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst16:
		mov	dl, [esi]	; Get our immediate value
		mov	byte [_z80de + 1], dl	; Store our new value
		inc	esi
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst17:
		sahf
		rcl	al, 1
		lahf
		and	ah, 0edh
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst18:
		sub	esi, ebp
		and	esi, 0ffffh
		add	esi, ebp
		mov	dl, [esi] ; Get our relative offset
		inc	esi	; Next instruction, please!
		cmp	dl, 0feh	; Jump to self?
		je		yesJrMan	; Yup! Bail out!
		xchg	eax, edx
		cbw
		xchg	eax, edx
		sub	esi, ebp
		add	si, dx
		and	esi, 0ffffh	; Only the lower 16 bits
		add	esi, ebp
		xor	dh, dh
noJumpMan317:
		sub	edi, byte 12
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]

yesJrMan:
		xor	edx, edx		; Zero me for later
		mov	edi, edx
		mov	[cyclesRemaining], edx
		sub	esi, 2	; Back to the instruction again
		jmp	noMoreExec


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst19:
		mov	dh, ah	; Get our flags
		and	dh, 0ech	; Preserve the top three and bits 2 & 3
		mov	[_orgval], bx	; Store our original value
		add	bx, word [_z80de]
		lahf
		mov	[cyclesRemaining], edi
		mov	di, [_orgval]	; Get original
		xor	di, bx ; XOR It with our computed value
		xor	di, word [_z80de]
		and	di, 1000h	; Just our half carry
		or		dx, di	; Or in our flags
		and	ah, 01h	; Just carry
		or	ah, dh
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 11
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst1a:
		mov	dx, [_z80de]
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop318:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead318
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr318		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine318

nextAddr318:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop318

callRoutine318:
		call	ReadMemoryByte	; Standard read routine
		mov	[_z80af], al	; Save our new accumulator
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit318

memoryRead318:
		mov	al, [ebp + edx]	; Get our data

readExit318:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst1b:
		dec	word [_z80de]
		sub	edi, byte 6
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst1c:
		sahf
		inc	byte [_z80de]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh	; Knock out N!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst1d:
		sahf
		dec	byte [_z80de]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set negative!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst1e:
		mov	dl, [esi]	; Get our immediate value
		mov	byte [_z80de], dl	; Store our new value
		inc	esi
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst1f:
		sahf
		rcr	al, 1
		lahf
		and	ah, 0edh
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst20:
		sub	esi, ebp
		and	esi, 0ffffh
		add	esi, ebp
		mov	dl, [esi] ; Get our relative offset
		inc	esi	; Next instruction, please!
		sahf
		jnz	takeJump319
		jmp	short noJumpMan319
takeJump319:
		sub	edi, 5
		xchg	eax, edx
		cbw
		xchg	eax, edx
		sub	esi, ebp
		add	si, dx
		and	esi, 0ffffh	; Only the lower 16 bits
		add	esi, ebp
		xor	dh, dh
noJumpMan319:
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst21:
		mov	bx, [esi]	; Get our immediate value of HL
		add	esi, 2
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst22:
		mov	dx, [esi]	; Get our address to write to
		add	esi, 2		; Next address, please...
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop320:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryWrite320
		cmp	dx, [edi]	; Are we smaller?
		jb		nextAddr320		; Yes, go to the next address
		cmp	dx, [edi+4]	; Are we bigger?
		jbe	callRoutine320

nextAddr320:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop320

callRoutine320:
		push	ax		; Save this for later
		push	dx
		mov	ax, bx
		call	WriteMemoryByte
		pop	dx
		pop	ax
		inc	dx

		push	ax
		push	dx
		mov	ax, bx
		xchg	ah, al
		call	WriteMemoryByte
		pop	dx
		pop	ax	; Restore us!
		jmp	writeExit320

memoryWrite320:
		mov	[ebp + edx], bx	; Store our word
writeExit320:
		mov	edi, [cyclesRemaining]
		xor	edx, edx	; Zero our upper byte
		sub	edi, byte 16
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst23:
		inc	bx
		sub	edi, byte 6
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst24:
		sahf
		inc	bh
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh	; Knock out N!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst25:
		sahf
		dec	bh
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set negative!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst26:
		mov	bh, [esi]	; Get our immediate value
		inc	esi
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst27:
		mov	dh, ah
		and	dh, 02ah
		test	ah, 02h	; Were we doing a subtraction?
		jnz	handleNeg ; Nope!
		sahf
		daa
		lahf
		jmp	short endDaa
handleNeg:
		sahf
		das
		lahf
endDaa:
		and	ah, 0d5h
		or	ah, dh
		xor	edx, edx
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst28:
		sub	esi, ebp
		and	esi, 0ffffh
		add	esi, ebp
		mov	dl, [esi] ; Get our relative offset
		inc	esi	; Next instruction, please!
		sahf
		jz	takeJump321
		jmp	short noJumpMan321
takeJump321:
		sub	edi, 5
		xchg	eax, edx
		cbw
		xchg	eax, edx
		sub	esi, ebp
		add	si, dx
		and	esi, 0ffffh	; Only the lower 16 bits
		add	esi, ebp
		xor	dh, dh
noJumpMan321:
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst29:
		mov	dh, ah	; Get our flags
		and	dh, 0ech	; Preserve the top three and bits 2 & 3
		mov	[_orgval], bx	; Store our original value
		add	bx, bx
		lahf
		mov	[cyclesRemaining], edi
		mov	di, [_orgval]	; Get original
		xor	di, bx ; XOR It with our computed value
		xor	di, bx
		and	di, 1000h	; Just our half carry
		or		dx, di	; Or in our flags
		and	ah, 01h	; Just carry
		or	ah, dh
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 11
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst2a:
		mov	dx, [esi]	; Get address to load
		add	esi, 2	; Skip over it so we don't execute it
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop322:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead322
		cmp	dx, [edi]	; Are we smaller?
		jb		nextAddr322		; Yes, go to the next address
		cmp	dx, [edi+4]	; Are we bigger?
		jbe	callRoutine322

nextAddr322:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop322

callRoutine322:
		push	ax		; Save this for later
		push	dx		; Save address
		call	ReadMemoryByte	; Standard read routine
		pop	dx		; Restore our address
		inc	dx		; Next byte, please
		push	ax		; Save returned byte
		call	ReadMemoryByte	; Standard read routine
		xchg	ah, al	; Swap for endian's sake
		pop	dx	; Restore LSB
		mov	dh, ah	; Our word is now in DX
		pop	ax		; Restore this
		mov	bx, dx	; Store our word
		jmp	readExit322

memoryRead322:
		mov	bx, [ebp + edx]
readExit322:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 16
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst2b:
		dec	bx
		sub	edi, byte 6
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst2c:
		sahf
		inc	bl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh	; Knock out N!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst2d:
		sahf
		dec	bl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set negative!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst2e:
		mov	bl, [esi]	; Get our immediate value
		inc	esi
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst2f:
		not	al
		or	ah, 012h	; N And H are now on!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst30:
		sub	esi, ebp
		and	esi, 0ffffh
		add	esi, ebp
		mov	dl, [esi] ; Get our relative offset
		inc	esi	; Next instruction, please!
		sahf
		jnc	takeJump323
		jmp	short noJumpMan323
takeJump323:
		sub	edi, 5
		xchg	eax, edx
		cbw
		xchg	eax, edx
		sub	esi, ebp
		add	si, dx
		and	esi, 0ffffh	; Only the lower 16 bits
		add	esi, ebp
		xor	dh, dh
noJumpMan323:
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst31:
		mov	dx, [esi]	; Get our immediate value of SP
		mov	word [_z80sp], dx	; Store it!
		xor	edx, edx
		add	esi, 2
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst32:
		mov	dx, [esi]	; Get our address to write to
		add	esi, 2		; Next address, please...
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop324:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite324	; Yes - go write it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr324	; Yes... go to the next addr
		cmp	dx, [edi+4]	; Are we smaller?
		jbe	callRoutine324	; If not, go call it!

nextAddr324:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop324

callRoutine324:
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit324
memoryWrite324:
		mov	[ebp + edx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit324:
		mov	edi, [cyclesRemaining]
		xor	edx, edx	; Zero our upper byte
		sub	edi, byte 13
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst33:
		inc	word [_z80sp]
		sub	edi, byte 6
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst34:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop325:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead325
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr325		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine325

nextAddr325:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop325

callRoutine325:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit325

memoryRead325:
		mov	dl, [ebp + ebx]	; Get our data

readExit325:
		mov	edi, [cyclesRemaining]
		sahf
		inc	dl
		lahf
		o16	pushf
		shl	edx, 16
		and	ah, 0fbh	;	Knock out parity/overflow
		pop	dx
		and	dh, 08h ; Just the overflow
		shr	dh, 1	; Shift it into position
		or	ah, dh	; OR It in with the real flags
		shr	edx, 16
		and	ah, 0fdh	; Knock out N!
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop326:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite326	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr326	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine326	; If not, go call it!

nextAddr326:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop326

callRoutine326:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit326
memoryWrite326:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit326:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 11
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst35:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop327:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead327
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr327		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine327

nextAddr327:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop327

callRoutine327:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit327

memoryRead327:
		mov	dl, [ebp + ebx]	; Get our data

readExit327:
		mov	edi, [cyclesRemaining]
		sahf
		dec	dl
		lahf
		o16	pushf
		shl	edx, 16
		and	ah, 0fbh	;	Knock out parity/overflow
		pop	dx
		and	dh, 08h ; Just the overflow
		shr	dh, 1	; Shift it into position
		or	ah, dh	; OR It in with the real flags
		shr	edx, 16
		or		ah, 02h	; Make it N!
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, dl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop328:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite328	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr328	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine328	; If not, go call it!

nextAddr328:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop328

callRoutine328:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit328
memoryWrite328:
		mov	[ebp + ebx], dl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit328:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 11
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst36:
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, [esi]	; And our data to write
		inc	esi	; Increment our program counter
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop329:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite329	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr329	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine329	; If not, go call it!

nextAddr329:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop329

callRoutine329:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit329
memoryWrite329:
		mov	[ebp + ebx], al	; Store our direct value
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit329:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst37:
		or	ah, 1
		and	ah,0edh
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst38:
		sub	esi, ebp
		and	esi, 0ffffh
		add	esi, ebp
		mov	dl, [esi] ; Get our relative offset
		inc	esi	; Next instruction, please!
		sahf
		jc	takeJump330
		jmp	short noJumpMan330
takeJump330:
		sub	edi, 5
		xchg	eax, edx
		cbw
		xchg	eax, edx
		sub	esi, ebp
		add	si, dx
		and	esi, 0ffffh	; Only the lower 16 bits
		add	esi, ebp
		xor	dh, dh
noJumpMan330:
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst39:
		mov	dh, ah	; Get our flags
		and	dh, 0ech	; Preserve the top three and bits 2 & 3
		mov	[_orgval], bx	; Store our original value
		add	bx, word [_z80sp]
		lahf
		mov	[cyclesRemaining], edi
		mov	di, [_orgval]	; Get original
		xor	di, bx ; XOR It with our computed value
		xor	di, word [_z80sp]
		and	di, 1000h	; Just our half carry
		or		dx, di	; Or in our flags
		and	ah, 01h	; Just carry
		or	ah, dh
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 11
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst3a:
		mov	dx, [esi]	; Get our address
		add	esi, 2		; Skip past the address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop331:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead331
		cmp	edx, [edi]	; Are we smaller?
		jb		nextAddr331		; Yes, go to the next address
		cmp	edx, [edi+4]	; Are we bigger?
		jbe	callRoutine331

nextAddr331:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop331

callRoutine331:
		call	ReadMemoryByte	; Standard read routine
		mov	[_z80af], al	; Save our new accumulator
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit331

memoryRead331:
		mov	al, [ebp + edx]	; Get our data

readExit331:
		mov	edi, [cyclesRemaining]
		xor	edx, edx	; Make sure we don't hose things
		sub	edi, byte 13
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst3b:
		dec	word [_z80sp]
		sub	edi, byte 6
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst3c:
		sahf
		inc	al
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh	; Knock out N!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst3d:
		sahf
		dec	al
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set negative!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst3e:
		mov	al, [esi]	; Get our immediate value
		inc	esi
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst3f:
		mov	dl, ah
		and	dl, 01h
		shl	dl, 4
		xor	ah, 01h
		and	ah, 0edh
		or	ah, dl
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst40:
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst41:
		mov	ch, cl
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst42:
		mov	ch, byte [_z80de + 1]
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst43:
		mov	ch, byte [_z80de]
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst44:
		mov	ch, bh
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst45:
		mov	ch, bl
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst46:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop332:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead332
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr332		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine332

nextAddr332:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop332

callRoutine332:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	ch, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit332

memoryRead332:
		mov	ch, [ebp + ebx]	; Get our data

readExit332:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst47:
		mov	ch, al
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst48:
		mov	cl, ch
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst49:
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst4a:
		mov	cl, byte [_z80de + 1]
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst4b:
		mov	cl, byte [_z80de]
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst4c:
		mov	cl, bh
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst4d:
		mov	cl, bl
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst4e:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop333:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead333
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr333		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine333

nextAddr333:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop333

callRoutine333:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	cl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit333

memoryRead333:
		mov	cl, [ebp + ebx]	; Get our data

readExit333:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst4f:
		mov	cl, al
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst50:
		mov	byte [_z80de + 1], ch
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst51:
		mov	byte [_z80de + 1], cl
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst52:
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst53:
		mov	dl, byte [_z80de]
		mov	[_z80de + 1], dl
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst54:
		mov	byte [_z80de + 1], bh
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst55:
		mov	byte [_z80de + 1], bl
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst56:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop334:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead334
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr334		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine334

nextAddr334:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop334

callRoutine334:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dh, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit334

memoryRead334:
		mov	dh, [ebp + ebx]	; Get our data

readExit334:
		mov	edi, [cyclesRemaining]
		mov	byte [_z80de + 1], dh
		xor	edx, edx
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst57:
		mov	byte [_z80de + 1], al
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst58:
		mov	byte [_z80de], ch
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst59:
		mov	byte [_z80de], cl
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst5a:
		mov	dl, byte [_z80de + 1]
		mov	[_z80de], dl
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst5b:
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst5c:
		mov	byte [_z80de], bh
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst5d:
		mov	byte [_z80de], bl
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst5e:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop335:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead335
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr335		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine335

nextAddr335:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop335

callRoutine335:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit335

memoryRead335:
		mov	dl, [ebp + ebx]	; Get our data

readExit335:
		mov	edi, [cyclesRemaining]
		mov	byte [_z80de], dl
		xor	edx, edx
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst5f:
		mov	byte [_z80de], al
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst60:
		mov	bh, ch
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst61:
		mov	bh, cl
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst62:
		mov	bh, byte [_z80de + 1]
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst63:
		mov	bh, byte [_z80de]
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst64:
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst65:
		mov	bh, bl
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst66:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop336:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead336
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr336		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine336

nextAddr336:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop336

callRoutine336:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	bh, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit336

memoryRead336:
		mov	bh, [ebp + ebx]	; Get our data

readExit336:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst67:
		mov	bh, al
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst68:
		mov	bl, ch
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst69:
		mov	bl, cl
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst6a:
		mov	bl, byte [_z80de + 1]
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst6b:
		mov	bl, byte [_z80de]
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst6c:
		mov	bl, bh
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst6d:
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst6e:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop337:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead337
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr337		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine337

nextAddr337:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop337

callRoutine337:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	bl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit337

memoryRead337:
		mov	bl, [ebp + ebx]	; Get our data

readExit337:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst6f:
		mov	bl, al
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst70:
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, ch	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop338:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite338	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr338	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine338	; If not, go call it!

nextAddr338:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop338

callRoutine338:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit338
memoryWrite338:
		mov	[ebp + ebx], ch
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit338:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst71:
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, cl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop339:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite339	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr339	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine339	; If not, go call it!

nextAddr339:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop339

callRoutine339:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit339
memoryWrite339:
		mov	[ebp + ebx], cl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit339:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst72:
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, byte [_z80de + 1]	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop340:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite340	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr340	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine340	; If not, go call it!

nextAddr340:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop340

callRoutine340:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit340
memoryWrite340:
		mov	edi, edx
		mov	dl, byte [_z80de + 1]
		mov	[ebp + ebx], dl
		mov	edx, edi
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit340:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst73:
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, byte [_z80de]	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop341:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite341	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr341	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine341	; If not, go call it!

nextAddr341:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop341

callRoutine341:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit341
memoryWrite341:
		mov	edi, edx
		mov	dl, byte [_z80de]
		mov	[ebp + ebx], dl
		mov	edx, edi
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit341:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst74:
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, bh	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop342:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite342	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr342	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine342	; If not, go call it!

nextAddr342:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop342

callRoutine342:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit342
memoryWrite342:
		mov	[ebp + ebx], bh
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit342:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst75:
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	al, bl	; And our data to write
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop343:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite343	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr343	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine343	; If not, go call it!

nextAddr343:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop343

callRoutine343:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit343
memoryWrite343:
		mov	[ebp + ebx], bl
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit343:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst76:
		mov	dword [_z80halted], 1	; We've halted the chip!
		xor	edi, edi
		mov	[cyclesRemaining], edi
		jmp	noMoreExec

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst77:
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop344:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite344	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr344	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine344	; If not, go call it!

nextAddr344:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop344

callRoutine344:
		mov	dx, bx	; Get our address to target
		call	WriteMemoryByte	; Go write the data!
		jmp	short WriteMacroExit344
memoryWrite344:
		mov	[ebp + ebx], al
		mov	ax, [_z80af] ; Get our accumulator and flags
WriteMacroExit344:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst78:
		mov	al, ch
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst79:
		mov	al, cl
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst7a:
		mov	al, byte [_z80de + 1]
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst7b:
		mov	al, byte [_z80de]
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst7c:
		mov	al, bh
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst7d:
		mov	al, bl
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst7e:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop345:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead345
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr345		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine345

nextAddr345:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop345

callRoutine345:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	[_z80af], al	; Save our new accumulator
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit345

memoryRead345:
		mov	al, [ebp + ebx]	; Get our data

readExit345:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst7f:
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst80:
		sahf
		add	al, ch
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh ; No N!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst81:
		sahf
		add	al, cl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh ; No N!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst82:
		sahf
		add	al, byte [_z80de + 1]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh ; No N!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst83:
		sahf
		add	al, byte [_z80de]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh ; No N!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst84:
		sahf
		add	al, bh
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh ; No N!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst85:
		sahf
		add	al, bl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh ; No N!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst86:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop346:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead346
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr346		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine346

nextAddr346:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop346

callRoutine346:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit346

memoryRead346:
		mov	dl, [ebp + ebx]	; Get our data

readExit346:
		mov	edi, [cyclesRemaining]
		sahf
		add	al, dl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh ; No N!
		xor	edx, edx	; Zero this...
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst87:
		sahf
		add	al, al
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh ; No N!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst88:
		sahf
		adc	al, ch
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh ; No N!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst89:
		sahf
		adc	al, cl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh ; No N!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst8a:
		sahf
		adc	al, byte [_z80de + 1]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh ; No N!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst8b:
		sahf
		adc	al, byte [_z80de]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh ; No N!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst8c:
		sahf
		adc	al, bh
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh ; No N!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst8d:
		sahf
		adc	al, bl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh ; No N!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst8e:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop347:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead347
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr347		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine347

nextAddr347:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop347

callRoutine347:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit347

memoryRead347:
		mov	dl, [ebp + ebx]	; Get our data

readExit347:
		mov	edi, [cyclesRemaining]
		sahf
		adc	al, dl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh ; No N!
		xor	edx, edx	; Zero this...
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst8f:
		sahf
		adc	al, al
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh ; No N!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst90:
		sahf
		sub	al, ch
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; N Gets set!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst91:
		sahf
		sub	al, cl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; N Gets set!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst92:
		sahf
		sub	al, byte [_z80de + 1]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; N Gets set!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst93:
		sahf
		sub	al, byte [_z80de]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; N Gets set!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst94:
		sahf
		sub	al, bh
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; N Gets set!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst95:
		sahf
		sub	al, bl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; N Gets set!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst96:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop348:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead348
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr348		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine348

nextAddr348:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop348

callRoutine348:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit348

memoryRead348:
		mov	dl, [ebp + ebx]	; Get our data

readExit348:
		mov	edi, [cyclesRemaining]
		sahf
		sub	al, dl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; N Gets set!
		xor	edx, edx	; Zero this...
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst97:
		sahf
		sub	al, al
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; N Gets set!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst98:
		sahf
		sbb	al, ch
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; N Gets set!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst99:
		sahf
		sbb	al, cl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; N Gets set!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst9a:
		sahf
		sbb	al, byte [_z80de + 1]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; N Gets set!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst9b:
		sahf
		sbb	al, byte [_z80de]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; N Gets set!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst9c:
		sahf
		sbb	al, bh
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; N Gets set!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst9d:
		sahf
		sbb	al, bl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; N Gets set!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst9e:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop349:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead349
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr349		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine349

nextAddr349:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop349

callRoutine349:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit349

memoryRead349:
		mov	dl, [ebp + ebx]	; Get our data

readExit349:
		mov	edi, [cyclesRemaining]
		sahf
		sbb	al, dl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; N Gets set!
		xor	edx, edx	; Zero this...
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInst9f:
		sahf
		sbb	al, al
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; N Gets set!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInsta0:
		sahf
		and	al, ch
		lahf
		and	ah, 0ech	; Only these flags matter!
		or	ah, 010h	; Half carry gets set
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInsta1:
		sahf
		and	al, cl
		lahf
		and	ah, 0ech	; Only these flags matter!
		or	ah, 010h	; Half carry gets set
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInsta2:
		sahf
		and	al, byte [_z80de + 1]
		lahf
		and	ah, 0ech	; Only these flags matter!
		or	ah, 010h	; Half carry gets set
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInsta3:
		sahf
		and	al, byte [_z80de]
		lahf
		and	ah, 0ech	; Only these flags matter!
		or	ah, 010h	; Half carry gets set
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInsta4:
		sahf
		and	al, bh
		lahf
		and	ah, 0ech	; Only these flags matter!
		or	ah, 010h	; Half carry gets set
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInsta5:
		sahf
		and	al, bl
		lahf
		and	ah, 0ech	; Only these flags matter!
		or	ah, 010h	; Half carry gets set
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInsta6:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop350:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead350
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr350		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine350

nextAddr350:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop350

callRoutine350:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit350

memoryRead350:
		mov	dl, [ebp + ebx]	; Get our data

readExit350:
		mov	edi, [cyclesRemaining]
		sahf
		and	al, dl
		lahf
		and	ah, 0ech	; Only these flags matter!
		or	ah, 010h	; Half carry gets set
		xor	edx, edx	; Zero this...
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInsta7:
		sahf
		and	al, al
		lahf
		and	ah, 0ech	; Only these flags matter!
		or	ah, 010h	; Half carry gets set
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInsta8:
		sahf
		xor	al, ch
		lahf
		and	ah, 0ech	; Only these flags matter!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInsta9:
		sahf
		xor	al, cl
		lahf
		and	ah, 0ech	; Only these flags matter!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstaa:
		sahf
		xor	al, byte [_z80de + 1]
		lahf
		and	ah, 0ech	; Only these flags matter!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstab:
		sahf
		xor	al, byte [_z80de]
		lahf
		and	ah, 0ech	; Only these flags matter!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstac:
		sahf
		xor	al, bh
		lahf
		and	ah, 0ech	; Only these flags matter!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstad:
		sahf
		xor	al, bl
		lahf
		and	ah, 0ech	; Only these flags matter!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstae:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop351:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead351
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr351		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine351

nextAddr351:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop351

callRoutine351:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit351

memoryRead351:
		mov	dl, [ebp + ebx]	; Get our data

readExit351:
		mov	edi, [cyclesRemaining]
		sahf
		xor	al, dl
		lahf
		and	ah, 0ech	; Only these flags matter!
		xor	edx, edx	; Zero this...
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstaf:
		sahf
		xor	al, al
		lahf
		and	ah, 0ech	; Only these flags matter!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstb0:
		sahf
		or	al, ch
		lahf
		and	ah, 0ech ; No H, N, or C
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstb1:
		sahf
		or	al, cl
		lahf
		and	ah, 0ech ; No H, N, or C
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstb2:
		sahf
		or	al, byte [_z80de + 1]
		lahf
		and	ah, 0ech ; No H, N, or C
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstb3:
		sahf
		or	al, byte [_z80de]
		lahf
		and	ah, 0ech ; No H, N, or C
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstb4:
		sahf
		or	al, bh
		lahf
		and	ah, 0ech ; No H, N, or C
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstb5:
		sahf
		or	al, bl
		lahf
		and	ah, 0ech ; No H, N, or C
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstb6:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop352:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead352
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr352		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine352

nextAddr352:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop352

callRoutine352:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit352

memoryRead352:
		mov	dl, [ebp + ebx]	; Get our data

readExit352:
		mov	edi, [cyclesRemaining]
		sahf
		or	al, dl
		lahf
		and	ah, 0ech ; No H, N, or C
		xor	edx, edx	; Zero this...
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstb7:
		sahf
		or	al, al
		lahf
		and	ah, 0ech ; No H, N, or C
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstb8:
		sahf
		cmp	al, ch
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set N for compare!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstb9:
		sahf
		cmp	al, cl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set N for compare!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstba:
		sahf
		cmp	al, byte [_z80de + 1]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set N for compare!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstbb:
		sahf
		cmp	al, byte [_z80de]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set N for compare!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstbc:
		sahf
		cmp	al, bh
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set N for compare!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstbd:
		sahf
		cmp	al, bl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set N for compare!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstbe:
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop353:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead353
		cmp	ebx, [edi]	; Are we smaller?
		jb		nextAddr353		; Yes, go to the next address
		cmp	ebx, [edi+4]	; Are we bigger?
		jbe	callRoutine353

nextAddr353:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop353

callRoutine353:
		mov	dx, bx	; Get our address
		call	ReadMemoryByte	; Standard read routine
		mov	dl, al	; Put our returned value here
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit353

memoryRead353:
		mov	dl, [ebp + ebx]	; Get our data

readExit353:
		mov	edi, [cyclesRemaining]
		sahf
		cmp	al, dl
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set N for compare!
		xor	edx, edx	; Zero this...
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstbf:
		sahf
		cmp	al, al
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set N for compare!
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstc0:
		sahf
		jnz	takeReturn354
		jmp	short retNotTaken354
takeReturn354:
		sub	edi, byte 6
		mov	dx, word [_z80sp]	; Get our current stack pointer
		mov	si, [edx+ebp]	; Get our return address
		and	esi, 0ffffh		; Only within 64K!
		add	esi, ebp			; Add in our base address
		add	word [_z80sp], 02h	; Remove our two bytes from the stack
		xor	edx, edx
retNotTaken354:
		sub	edi, byte 5
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstc1:
		mov	dx, [_z80sp]
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop355:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead355
		cmp	dx, [edi]	; Are we smaller?
		jb		nextAddr355		; Yes, go to the next address
		cmp	dx, [edi+4]	; Are we bigger?
		jbe	callRoutine355

nextAddr355:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop355

callRoutine355:
		push	ax		; Save this for later
		push	dx		; Save address
		call	ReadMemoryByte	; Standard read routine
		pop	dx		; Restore our address
		inc	dx		; Next byte, please
		push	ax		; Save returned byte
		call	ReadMemoryByte	; Standard read routine
		xchg	ah, al	; Swap for endian's sake
		pop	dx	; Restore LSB
		mov	dh, ah	; Our word is now in DX
		pop	ax		; Restore this
		mov	cx, dx	; Store our word
		jmp	readExit355

memoryRead355:
		mov	cx, [ebp + edx]
readExit355:
		mov	edi, [cyclesRemaining]
		add	word [_z80sp], 2
		xor	edx, edx
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstc2:
		sahf		; Restore our flags
		jnz	takeJump356	; We're going to take a jump
		add	esi, 2		; Skip past the address
		jmp	short nextInst356	 ; Go execute the next instruction
takeJump356:
		mov	si, [esi]	; Get our new offset
		and	esi, 0ffffh	; Only the lower WORD is valid
		add	esi, ebp		; Our new address!
nextInst356:
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstc3:
		mov	si, [esi]	; Get our new address
		and	esi, 0ffffh	; Only the lower 16 bits
		add	esi, ebp		; Our new address!
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstc4:
		sahf		; Restore our flags
		jnz	takeJump357	; We're going call in this case
		add	esi, 2		; Skip past the address
		jmp	short noCallTaken357	 ; Go execute the next instruction
takeJump357:
		sub	edi, 7
		mov	dx, [esi]	; Get our call to address
		mov	[_z80pc], dx ; Store our new program counter
		add	esi, 2		; Skip to our new address to be pushed
		sub	esi, ebp		; Value to push onto the "stack"
		mov	dx, word [_z80sp] ; Get the current stack pointer
		sub	dx, 2		; Back up two bytes
		mov	[ebp+edx], si ; PUSH It!
		mov	word [_z80sp], dx	; Store our new stack pointer
		mov	si, [_z80pc] ; Get our new program counter
		add	esi, ebp		; Naturalize it!
noCallTaken357:
		xor	edx, edx
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstc5:
		sub	word [_z80sp], 2
		mov	dx, [_z80sp]
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop358:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryWrite358
		cmp	dx, [edi]	; Are we smaller?
		jb		nextAddr358		; Yes, go to the next address
		cmp	dx, [edi+4]	; Are we bigger?
		jbe	callRoutine358

nextAddr358:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop358

callRoutine358:
		push	ax		; Save this for later
		push	dx
		mov	ax, cx
		call	WriteMemoryByte
		pop	dx
		pop	ax
		inc	dx

		push	ax
		push	dx
		mov	ax, cx
		xchg	ah, al
		call	WriteMemoryByte
		pop	dx
		pop	ax	; Restore us!
		jmp	writeExit358

memoryWrite358:
		mov	[ebp + edx], cx	; Store our word
writeExit358:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 11
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstc6:
		sahf
		add	al, [esi]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh ; Knock out N!
		inc	esi
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstc7:
		mov	dx, word [_z80sp]	; Get our stack pointer
		sub	dx, 2		; Make room for the new value!
		mov	word [_z80sp], dx	; Store our new stack pointer
		sub	esi, ebp		; Get our real PC
		mov	[ebp+edx], si	; Our return address
		mov	si, 000h	; Our new call address
		add	esi, ebp	; Back to the base!
		xor	edx, edx
		sub	edi, byte 11
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstc8:
		sahf
		jz	takeReturn359
		jmp	short retNotTaken359
takeReturn359:
		sub	edi, byte 6
		mov	dx, word [_z80sp]	; Get our current stack pointer
		mov	si, [edx+ebp]	; Get our return address
		and	esi, 0ffffh		; Only within 64K!
		add	esi, ebp			; Add in our base address
		add	word [_z80sp], 02h	; Remove our two bytes from the stack
		xor	edx, edx
retNotTaken359:
		sub	edi, byte 5
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstc9:
		mov	dx, word [_z80sp]	; Get our current stack pointer
		mov	si, [edx+ebp]	; Get our return address
		and	esi, 0ffffh		; Only within 64K!
		add	esi, ebp			; Add in our base address
		add	word [_z80sp], 02h	; Remove our two bytes from the stack
		xor	edx, edx
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstca:
		sahf		; Restore our flags
		jz	takeJump360	; We're going to take a jump
		add	esi, 2		; Skip past the address
		jmp	short nextInst360	 ; Go execute the next instruction
takeJump360:
		mov	si, [esi]	; Get our new offset
		and	esi, 0ffffh	; Only the lower WORD is valid
		add	esi, ebp		; Our new address!
nextInst360:
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]

;
; Handler for all CBxx instructions
;

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstcb:
		mov	dl, [esi]
		inc	esi
		jmp	dword [z80PrefixCB+edx*4]




times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstcc:
		sahf		; Restore our flags
		jz	takeJump361	; We're going call in this case
		add	esi, 2		; Skip past the address
		jmp	short noCallTaken361	 ; Go execute the next instruction
takeJump361:
		sub	edi, 7
		mov	dx, [esi]	; Get our call to address
		mov	[_z80pc], dx ; Store our new program counter
		add	esi, 2		; Skip to our new address to be pushed
		sub	esi, ebp		; Value to push onto the "stack"
		mov	dx, word [_z80sp] ; Get the current stack pointer
		sub	dx, 2		; Back up two bytes
		mov	[ebp+edx], si ; PUSH It!
		mov	word [_z80sp], dx	; Store our new stack pointer
		mov	si, [_z80pc] ; Get our new program counter
		add	esi, ebp		; Naturalize it!
noCallTaken361:
		xor	edx, edx
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstcd:
		mov	dx, [esi]	; Get our call to address
		mov	[_z80pc], dx ; Store our new program counter
		add	esi, 2		; Skip to our new address to be pushed
		sub	esi, ebp		; Value to push onto the "stack"
		mov	dx, word [_z80sp] ; Get the current stack pointer
		sub	dx, 2		; Back up two bytes
		mov	[ebp+edx], si ; PUSH It!
		mov	word [_z80sp], dx	; Store our new stack pointer
		mov	si, [_z80pc] ; Get our new program counter
		add	esi, ebp		; Naturalize it!
		xor	edx, edx
		sub	edi, byte 17
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstce:
		sahf
		adc	al, [esi]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		and	ah, 0fdh ; Knock out N!
		inc	esi
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstcf:
		mov	dx, word [_z80sp]	; Get our stack pointer
		sub	dx, 2		; Make room for the new value!
		mov	word [_z80sp], dx	; Store our new stack pointer
		sub	esi, ebp		; Get our real PC
		mov	[ebp+edx], si	; Our return address
		mov	si, 008h	; Our new call address
		add	esi, ebp	; Back to the base!
		xor	edx, edx
		sub	edi, byte 11
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstd0:
		sahf
		jnc	takeReturn362
		jmp	short retNotTaken362
takeReturn362:
		sub	edi, byte 6
		mov	dx, word [_z80sp]	; Get our current stack pointer
		mov	si, [edx+ebp]	; Get our return address
		and	esi, 0ffffh		; Only within 64K!
		add	esi, ebp			; Add in our base address
		add	word [_z80sp], 02h	; Remove our two bytes from the stack
		xor	edx, edx
retNotTaken362:
		sub	edi, byte 5
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstd1:
		mov	dx, [_z80sp]
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop363:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead363
		cmp	dx, [edi]	; Are we smaller?
		jb		nextAddr363		; Yes, go to the next address
		cmp	dx, [edi+4]	; Are we bigger?
		jbe	callRoutine363

nextAddr363:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop363

callRoutine363:
		push	ax		; Save this for later
		push	dx		; Save address
		call	ReadMemoryByte	; Standard read routine
		pop	dx		; Restore our address
		inc	dx		; Next byte, please
		push	ax		; Save returned byte
		call	ReadMemoryByte	; Standard read routine
		xchg	ah, al	; Swap for endian's sake
		pop	dx	; Restore LSB
		mov	dh, ah	; Our word is now in DX
		pop	ax		; Restore this
		mov	word [_z80de], dx	; Store our word
		jmp	readExit363

memoryRead363:
		mov	dx, [ebp + edx]
		mov	word [_z80de], dx
readExit363:
		mov	edi, [cyclesRemaining]
		add	word [_z80sp], 2
		xor	edx, edx
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstd2:
		sahf		; Restore our flags
		jnc	takeJump364	; We're going to take a jump
		add	esi, 2		; Skip past the address
		jmp	short nextInst364	 ; Go execute the next instruction
takeJump364:
		mov	si, [esi]	; Get our new offset
		and	esi, 0ffffh	; Only the lower WORD is valid
		add	esi, ebp		; Our new address!
nextInst364:
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstd3:
		mov	dl, [esi]	; Get our address to 'out' to
		inc	esi	; Next address
		mov	[cyclesRemaining], edi
		mov	[_z80af], ax	; Store AF
		mov	edi, [_z80IoWrite]	; Point to the I/O write array

checkLoop365:
		cmp	[edi], word 0ffffh ; End of our list?
		je	WriteMacroExit365	; Yes - ignore it!
		cmp	dx, [edi]	; Are we smaller?
		jb	nextAddr365	; Yes... go to the next addr
		cmp	dx, [edi+2]	; Are we bigger?
		jbe	callRoutine365	; If not, go call it!

nextAddr365:
		add	edi, 0ch		; Next structure, please
		jmp	short checkLoop365

callRoutine365:
		call	WriteIOByte	; Go write the data!
WriteMacroExit365:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 11
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstd4:
		sahf		; Restore our flags
		jnc	takeJump366	; We're going call in this case
		add	esi, 2		; Skip past the address
		jmp	short noCallTaken366	 ; Go execute the next instruction
takeJump366:
		sub	edi, 7
		mov	dx, [esi]	; Get our call to address
		mov	[_z80pc], dx ; Store our new program counter
		add	esi, 2		; Skip to our new address to be pushed
		sub	esi, ebp		; Value to push onto the "stack"
		mov	dx, word [_z80sp] ; Get the current stack pointer
		sub	dx, 2		; Back up two bytes
		mov	[ebp+edx], si ; PUSH It!
		mov	word [_z80sp], dx	; Store our new stack pointer
		mov	si, [_z80pc] ; Get our new program counter
		add	esi, ebp		; Naturalize it!
noCallTaken366:
		xor	edx, edx
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstd5:
		sub	word [_z80sp], 2
		mov	dx, [_z80sp]
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop367:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryWrite367
		cmp	dx, [edi]	; Are we smaller?
		jb		nextAddr367		; Yes, go to the next address
		cmp	dx, [edi+4]	; Are we bigger?
		jbe	callRoutine367

nextAddr367:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop367

callRoutine367:
		push	ax		; Save this for later
		push	dx
		mov	ax, word [_z80de]
		call	WriteMemoryByte
		pop	dx
		pop	ax
		inc	dx

		push	ax
		push	dx
		mov	ax, word [_z80de]
		xchg	ah, al
		call	WriteMemoryByte
		pop	dx
		pop	ax	; Restore us!
		jmp	writeExit367

memoryWrite367:
		mov	di, word [_z80de]
		mov	[ebp + edx], di	; Store our word
writeExit367:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 11
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstd6:
		sahf
		sub	al, [esi]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set negative!
		inc	esi
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstd7:
		mov	dx, word [_z80sp]	; Get our stack pointer
		sub	dx, 2		; Make room for the new value!
		mov	word [_z80sp], dx	; Store our new stack pointer
		sub	esi, ebp		; Get our real PC
		mov	[ebp+edx], si	; Our return address
		mov	si, 010h	; Our new call address
		add	esi, ebp	; Back to the base!
		xor	edx, edx
		sub	edi, byte 11
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstd8:
		sahf
		jc	takeReturn368
		jmp	short retNotTaken368
takeReturn368:
		sub	edi, byte 6
		mov	dx, word [_z80sp]	; Get our current stack pointer
		mov	si, [edx+ebp]	; Get our return address
		and	esi, 0ffffh		; Only within 64K!
		add	esi, ebp			; Add in our base address
		add	word [_z80sp], 02h	; Remove our two bytes from the stack
		xor	edx, edx
retNotTaken368:
		sub	edi, byte 5
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstd9:
		mov	[cyclesRemaining], edi
		mov	di, [_z80de]
		xchg	cx, [_z80bcprime]
		xchg	di, [_z80deprime]
		xchg	bx, [_z80hlprime]
		mov	[_z80de], di
		mov	edi, [cyclesRemaining]
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstda:
		sahf		; Restore our flags
		jc	takeJump369	; We're going to take a jump
		add	esi, 2		; Skip past the address
		jmp	short nextInst369	 ; Go execute the next instruction
takeJump369:
		mov	si, [esi]	; Get our new offset
		and	esi, 0ffffh	; Only the lower WORD is valid
		add	esi, ebp		; Our new address!
nextInst369:
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstdb:
		mov	dl, [esi]	; Get our address to 'out' to
		inc	esi	; Next address
		mov	[cyclesRemaining], edi
		mov	edi, [_z80IoRead]	; Point to the read array

checkLoop370:
		cmp	[edi], word 0ffffh ; End of the list?
		je		ioRead370
		cmp	dx, [edi]	; Are we smaller?
		jb		nextAddr370		; Yes, go to the next address
		cmp	dx, [edi+2]	; Are we bigger?
		jbe	callRoutine370

nextAddr370:
		add	edi, 0ch		; Next structure!
		jmp	short checkLoop370

callRoutine370:
		call	ReadIOByte	; Standard read routine
		mov	[_z80af], al	; Save our new accumulator
		mov	ax, [_z80af]	; Get our AF back
		jmp	short readExit370

ioRead370:
		mov	al, 0ffh	; An unreferenced read
readExit370:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 11
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstdc:
		sahf		; Restore our flags
		jc	takeJump371	; We're going call in this case
		add	esi, 2		; Skip past the address
		jmp	short noCallTaken371	 ; Go execute the next instruction
takeJump371:
		sub	edi, 7
		mov	dx, [esi]	; Get our call to address
		mov	[_z80pc], dx ; Store our new program counter
		add	esi, 2		; Skip to our new address to be pushed
		sub	esi, ebp		; Value to push onto the "stack"
		mov	dx, word [_z80sp] ; Get the current stack pointer
		sub	dx, 2		; Back up two bytes
		mov	[ebp+edx], si ; PUSH It!
		mov	word [_z80sp], dx	; Store our new stack pointer
		mov	si, [_z80pc] ; Get our new program counter
		add	esi, ebp		; Naturalize it!
noCallTaken371:
		xor	edx, edx
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]

;
; Handler for all DDxx instructions
;

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstdd:
		mov	dl, [esi]
		inc	esi
		jmp	dword [z80PrefixDD+edx*4]




times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstde:
		sahf
		sbb	al, [esi]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set negative!
		inc	esi
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstdf:
		mov	dx, word [_z80sp]	; Get our stack pointer
		sub	dx, 2		; Make room for the new value!
		mov	word [_z80sp], dx	; Store our new stack pointer
		sub	esi, ebp		; Get our real PC
		mov	[ebp+edx], si	; Our return address
		mov	si, 018h	; Our new call address
		add	esi, ebp	; Back to the base!
		xor	edx, edx
		sub	edi, byte 11
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInste0:
		sahf
		jpo	takeReturn372
		jmp	short retNotTaken372
takeReturn372:
		sub	edi, byte 6
		mov	dx, word [_z80sp]	; Get our current stack pointer
		mov	si, [edx+ebp]	; Get our return address
		and	esi, 0ffffh		; Only within 64K!
		add	esi, ebp			; Add in our base address
		add	word [_z80sp], 02h	; Remove our two bytes from the stack
		xor	edx, edx
retNotTaken372:
		sub	edi, byte 5
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInste1:
		mov	dx, [_z80sp]
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop373:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead373
		cmp	dx, [edi]	; Are we smaller?
		jb		nextAddr373		; Yes, go to the next address
		cmp	dx, [edi+4]	; Are we bigger?
		jbe	callRoutine373

nextAddr373:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop373

callRoutine373:
		push	ax		; Save this for later
		push	dx		; Save address
		call	ReadMemoryByte	; Standard read routine
		pop	dx		; Restore our address
		inc	dx		; Next byte, please
		push	ax		; Save returned byte
		call	ReadMemoryByte	; Standard read routine
		xchg	ah, al	; Swap for endian's sake
		pop	dx	; Restore LSB
		mov	dh, ah	; Our word is now in DX
		pop	ax		; Restore this
		mov	bx, dx	; Store our word
		jmp	readExit373

memoryRead373:
		mov	bx, [ebp + edx]
readExit373:
		mov	edi, [cyclesRemaining]
		add	word [_z80sp], 2
		xor	edx, edx
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInste2:
		sahf		; Restore our flags
		jpo	takeJump374	; We're going to take a jump
		add	esi, 2		; Skip past the address
		jmp	short nextInst374	 ; Go execute the next instruction
takeJump374:
		mov	si, [esi]	; Get our new offset
		and	esi, 0ffffh	; Only the lower WORD is valid
		add	esi, ebp		; Our new address!
nextInst374:
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInste3:
		mov	dx, word [_z80sp]
		xchg	bx, [ebp+edx]
		xor	edx, edx
		sub	edi, byte 19
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInste4:
		sahf		; Restore our flags
		jpo	takeJump375	; We're going call in this case
		add	esi, 2		; Skip past the address
		jmp	short noCallTaken375	 ; Go execute the next instruction
takeJump375:
		sub	edi, 7
		mov	dx, [esi]	; Get our call to address
		mov	[_z80pc], dx ; Store our new program counter
		add	esi, 2		; Skip to our new address to be pushed
		sub	esi, ebp		; Value to push onto the "stack"
		mov	dx, word [_z80sp] ; Get the current stack pointer
		sub	dx, 2		; Back up two bytes
		mov	[ebp+edx], si ; PUSH It!
		mov	word [_z80sp], dx	; Store our new stack pointer
		mov	si, [_z80pc] ; Get our new program counter
		add	esi, ebp		; Naturalize it!
noCallTaken375:
		xor	edx, edx
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInste5:
		sub	word [_z80sp], 2
		mov	dx, [_z80sp]
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop376:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryWrite376
		cmp	dx, [edi]	; Are we smaller?
		jb		nextAddr376		; Yes, go to the next address
		cmp	dx, [edi+4]	; Are we bigger?
		jbe	callRoutine376

nextAddr376:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop376

callRoutine376:
		push	ax		; Save this for later
		push	dx
		mov	ax, bx
		call	WriteMemoryByte
		pop	dx
		pop	ax
		inc	dx

		push	ax
		push	dx
		mov	ax, bx
		xchg	ah, al
		call	WriteMemoryByte
		pop	dx
		pop	ax	; Restore us!
		jmp	writeExit376

memoryWrite376:
		mov	[ebp + edx], bx	; Store our word
writeExit376:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 11
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInste6:
		sahf
		and	al, [esi]
		lahf
		and	ah, 0ech ; Only parity, half carry, sign, zero
		or	ah, 10h	; Half carry
		inc	esi
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInste7:
		mov	dx, word [_z80sp]	; Get our stack pointer
		sub	dx, 2		; Make room for the new value!
		mov	word [_z80sp], dx	; Store our new stack pointer
		sub	esi, ebp		; Get our real PC
		mov	[ebp+edx], si	; Our return address
		mov	si, 020h	; Our new call address
		add	esi, ebp	; Back to the base!
		xor	edx, edx
		sub	edi, byte 11
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInste8:
		sahf
		jpe	takeReturn377
		jmp	short retNotTaken377
takeReturn377:
		sub	edi, byte 6
		mov	dx, word [_z80sp]	; Get our current stack pointer
		mov	si, [edx+ebp]	; Get our return address
		and	esi, 0ffffh		; Only within 64K!
		add	esi, ebp			; Add in our base address
		add	word [_z80sp], 02h	; Remove our two bytes from the stack
		xor	edx, edx
retNotTaken377:
		sub	edi, byte 5
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInste9:
		mov	si, bx
		and	esi, 0ffffh
		add	esi, ebp
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstea:
		sahf		; Restore our flags
		jpe	takeJump378	; We're going to take a jump
		add	esi, 2		; Skip past the address
		jmp	short nextInst378	 ; Go execute the next instruction
takeJump378:
		mov	si, [esi]	; Get our new offset
		and	esi, 0ffffh	; Only the lower WORD is valid
		add	esi, ebp		; Our new address!
nextInst378:
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInsteb:
		xchg	[_z80de], bx	; Exchange DE & HL
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstec:
		sahf		; Restore our flags
		jpe	takeJump379	; We're going call in this case
		add	esi, 2		; Skip past the address
		jmp	short noCallTaken379	 ; Go execute the next instruction
takeJump379:
		sub	edi, 7
		mov	dx, [esi]	; Get our call to address
		mov	[_z80pc], dx ; Store our new program counter
		add	esi, 2		; Skip to our new address to be pushed
		sub	esi, ebp		; Value to push onto the "stack"
		mov	dx, word [_z80sp] ; Get the current stack pointer
		sub	dx, 2		; Back up two bytes
		mov	[ebp+edx], si ; PUSH It!
		mov	word [_z80sp], dx	; Store our new stack pointer
		mov	si, [_z80pc] ; Get our new program counter
		add	esi, ebp		; Naturalize it!
noCallTaken379:
		xor	edx, edx
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]

;
; Handler for all EDxx instructions
;

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInsted:
		mov	dl, [esi]
		inc	esi
		jmp	dword [z80PrefixED+edx*4]




times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstee:
		sahf
		xor	al, [esi]
		lahf
		and	ah, 0ech	; No H, N, or C
		inc	esi
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstef:
		mov	dx, word [_z80sp]	; Get our stack pointer
		sub	dx, 2		; Make room for the new value!
		mov	word [_z80sp], dx	; Store our new stack pointer
		sub	esi, ebp		; Get our real PC
		mov	[ebp+edx], si	; Our return address
		mov	si, 028h	; Our new call address
		add	esi, ebp	; Back to the base!
		xor	edx, edx
		sub	edi, byte 11
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstf0:
		sahf
		jns	takeReturn380
		jmp	short retNotTaken380
takeReturn380:
		sub	edi, byte 6
		mov	dx, word [_z80sp]	; Get our current stack pointer
		mov	si, [edx+ebp]	; Get our return address
		and	esi, 0ffffh		; Only within 64K!
		add	esi, ebp			; Add in our base address
		add	word [_z80sp], 02h	; Remove our two bytes from the stack
		xor	edx, edx
retNotTaken380:
		sub	edi, byte 5
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstf1:
		mov	dx, [_z80sp]
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemRead]	; Point to the read array

checkLoop381:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryRead381
		cmp	dx, [edi]	; Are we smaller?
		jb		nextAddr381		; Yes, go to the next address
		cmp	dx, [edi+4]	; Are we bigger?
		jbe	callRoutine381

nextAddr381:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop381

callRoutine381:
		push	dx		; Save address
		call	ReadMemoryByte	; Standard read routine
		pop	dx		; Restore our address
		inc	dx		; Next byte, please
		push	ax		; Save returned byte
		call	ReadMemoryByte	; Standard read routine
		xchg	ah, al	; Swap for endian's sake
		pop	dx	; Restore LSB
		mov	dh, ah	; Our word is now in DX
		mov	ax, dx
		xchg	ah, al
		jmp	readExit381

memoryRead381:
		mov	ax, [ebp + edx]
		xchg	ah, al
readExit381:
		mov	edi, [cyclesRemaining]
		add	word [_z80sp], 2
		xor	edx, edx
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstf2:
		sahf		; Restore our flags
		jns	takeJump382	; We're going to take a jump
		add	esi, 2		; Skip past the address
		jmp	short nextInst382	 ; Go execute the next instruction
takeJump382:
		mov	si, [esi]	; Get our new offset
		and	esi, 0ffffh	; Only the lower WORD is valid
		add	esi, ebp		; Our new address!
nextInst382:
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstf3:
		and	dword [_z80iff], (~IFF1)	; Not in an interrupt
		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstf4:
		sahf		; Restore our flags
		jns	takeJump383	; We're going call in this case
		add	esi, 2		; Skip past the address
		jmp	short noCallTaken383	 ; Go execute the next instruction
takeJump383:
		sub	edi, 7
		mov	dx, [esi]	; Get our call to address
		mov	[_z80pc], dx ; Store our new program counter
		add	esi, 2		; Skip to our new address to be pushed
		sub	esi, ebp		; Value to push onto the "stack"
		mov	dx, word [_z80sp] ; Get the current stack pointer
		sub	dx, 2		; Back up two bytes
		mov	[ebp+edx], si ; PUSH It!
		mov	word [_z80sp], dx	; Store our new stack pointer
		mov	si, [_z80pc] ; Get our new program counter
		add	esi, ebp		; Naturalize it!
noCallTaken383:
		xor	edx, edx
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstf5:
		sub	word [_z80sp], 2
		mov	dx, [_z80sp]
		mov	[cyclesRemaining], edi
		mov	edi, [_z80MemWrite]	; Point to the write array

checkLoop384:
		cmp	[edi], word 0ffffh ; End of the list?
		je		memoryWrite384
		cmp	dx, [edi]	; Are we smaller?
		jb		nextAddr384		; Yes, go to the next address
		cmp	dx, [edi+4]	; Are we bigger?
		jbe	callRoutine384

nextAddr384:
		add	edi, 10h		; Next structure!
		jmp	short checkLoop384

callRoutine384:
		push	ax		; Save this for later
		push	dx
		xchg	ah, al
		call	WriteMemoryByte
		pop	dx
		pop	ax
		inc	dx

		push	ax
		push	dx
		call	WriteMemoryByte
		pop	dx
		pop	ax	; Restore us!
		jmp	writeExit384

memoryWrite384:
		xchg	ah, al	; Swap for later
		mov	[ebp + edx], ax	; Store our word
		xchg	ah, al	; Restore
writeExit384:
		mov	edi, [cyclesRemaining]
		xor	edx, edx
		sub	edi, byte 11
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstf6:
		sahf
		or	al, [esi]
		lahf
		and	ah, 0ech	; No H, N, or C
		inc	esi
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstf7:
		mov	dx, word [_z80sp]	; Get our stack pointer
		sub	dx, 2		; Make room for the new value!
		mov	word [_z80sp], dx	; Store our new stack pointer
		sub	esi, ebp		; Get our real PC
		mov	[ebp+edx], si	; Our return address
		mov	si, 030h	; Our new call address
		add	esi, ebp	; Back to the base!
		xor	edx, edx
		sub	edi, byte 11
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstf8:
		sahf
		js	takeReturn385
		jmp	short retNotTaken385
takeReturn385:
		sub	edi, byte 6
		mov	dx, word [_z80sp]	; Get our current stack pointer
		mov	si, [edx+ebp]	; Get our return address
		and	esi, 0ffffh		; Only within 64K!
		add	esi, ebp			; Add in our base address
		add	word [_z80sp], 02h	; Remove our two bytes from the stack
		xor	edx, edx
retNotTaken385:
		sub	edi, byte 5
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstf9:
		mov	word [_z80sp], bx
		sub	edi, byte 6
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstfa:
		sahf		; Restore our flags
		js	takeJump386	; We're going to take a jump
		add	esi, 2		; Skip past the address
		jmp	short nextInst386	 ; Go execute the next instruction
takeJump386:
		mov	si, [esi]	; Get our new offset
		and	esi, 0ffffh	; Only the lower WORD is valid
		add	esi, ebp		; Our new address!
nextInst386:
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstfb:
		or		dword [_z80iff], IFF1	; Indicate interrupts are enabled now
		sub	edi, 4	; Takes 4 cycles!
		mov	[dwEITiming], edi	; Snapshot our current timing
		mov	[bEIExit], byte 1	; Indicate we're exiting because of an EI
		xor	edi, edi	; Force next instruction to exit
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi	; Next PC
		jmp	dword [z80regular+edx*4]

		sub	edi, byte 4
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstfc:
		sahf		; Restore our flags
		js	takeJump387	; We're going call in this case
		add	esi, 2		; Skip past the address
		jmp	short noCallTaken387	 ; Go execute the next instruction
takeJump387:
		sub	edi, 7
		mov	dx, [esi]	; Get our call to address
		mov	[_z80pc], dx ; Store our new program counter
		add	esi, 2		; Skip to our new address to be pushed
		sub	esi, ebp		; Value to push onto the "stack"
		mov	dx, word [_z80sp] ; Get the current stack pointer
		sub	dx, 2		; Back up two bytes
		mov	[ebp+edx], si ; PUSH It!
		mov	word [_z80sp], dx	; Store our new stack pointer
		mov	si, [_z80pc] ; Get our new program counter
		add	esi, ebp		; Naturalize it!
noCallTaken387:
		xor	edx, edx
		sub	edi, byte 10
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]

;
; Handler for all FDxx instructions
;

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstfd:
		mov	dl, [esi]
		inc	esi
		jmp	dword [z80PrefixFD+edx*4]




times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstfe:
		sahf
		cmp	al, [esi]
		lahf
		seto	dl
		and	ah, 0fbh	; Knock out parity/overflow
		shl	dl, 2
		or		ah, dl
		or	ah, 02h	; Set negative!
		inc	esi
		sub	edi, byte 7
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

RegInstff:
		mov	dx, word [_z80sp]	; Get our stack pointer
		sub	dx, 2		; Make room for the new value!
		mov	word [_z80sp], dx	; Store our new stack pointer
		sub	esi, ebp		; Get our real PC
		mov	[ebp+edx], si	; Our return address
		mov	si, 038h	; Our new call address
		add	esi, ebp	; Back to the base!
		xor	edx, edx
		sub	edi, byte 11
		js	near noMoreExec
		mov	dl, byte [esi]	; Get our next instruction
		inc	esi		; Increment PC
		jmp	dword [z80regular+edx*4]


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

; This is a generic read memory byte handler when a foreign
; handler is to be called

; EDI=Handler address, EDX=Address
; On return, EDX & EDI are undisturbed and AL=Byte read

ReadMemoryByte:
		mov	[_z80af], ax	; Save AF
		cmp	[edi+8], dword 0 ; Null handler?
		je	directReadHandler	; Yep! It's a direct read!

		mov	[_z80hl], bx	; Save HL
		mov	[_z80bc], cx	; Save BC
		sub	esi, ebp	; Our program counter
		mov	[_z80pc], si	; Save our program counter
		mov	esi, [dwOriginalExec]	
		sub	esi, [cyclesRemaining]
		add	[dwElapsedTicks], esi
		add	[_z80rCounter], esi
		sub	[dwOriginalExec], esi
		push	edi	; Save our structure address
		push	edx	; And our desired address
		call	dword [edi + 8]	; Go call our handler
		pop	edx	; Restore our address
		pop	edi	; Restore our handler's address
		xor	ebx, ebx	; Zero our future HL
		xor	esi, esi	; Zero it!
		mov	ebp, [_z80Base] ; Base pointer comes back
		mov	si, [_z80pc]	; Get our program counter back
		xor	ecx, ecx	; Zero our future BC
		add	esi, ebp	; Rebase it properly
		mov	bx, [_z80hl]	; Get HL back
		mov	cx, [_z80bc]	; Get BC back
		ret

directReadHandler:
		mov	eax, [edi+12]	; Get our base address
		sub	edx, [edi]	; Subtract our base (low) address
		mov	al, [edx+eax]	; Get our data byte
		and	eax, 0ffh	; Only the lower byte matters!
		add	edx, [edi]	; Add our base back
		ret		; Return to caller!


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

; This is a generic read memory byte handler when a foreign
; handler is to be called.
; EDI=Handler address, AL=Byte to write, EDX=Address
; EDI and EDX Are undisturbed on exit

WriteMemoryByte:
		cmp	[edi+8], dword 0	; Null handler?
		je	directWriteHandler

		mov	[_z80hl], bx	; Save HL
		mov	[_z80bc], cx	; Save BX
		sub	esi, ebp	; Our program counter
		mov	[_z80pc], si	; Save our program counter
		mov	esi, [dwOriginalExec]	
		sub	esi, [cyclesRemaining]
		add	[dwElapsedTicks], esi
		add	[_z80rCounter], esi
		sub	[dwOriginalExec], esi
		push	edi	; Save our structure address
		push	eax	; Data to write
		push	edx	; And our desired address
		call	dword [edi + 8]	; Go call our handler
		pop	edx	; Restore our address
		pop	eax	; Restore our data written
		pop	edi	; Save our structure address
		xor	ebx, ebx	; Zero our future HL
		xor	ecx, ecx	; Zero our future BC
		mov	bx, [_z80hl]	; Get HL back
		mov	cx, [_z80bc]	; Get BC back
		mov	ax, [_z80af]	; Get AF back
		xor	esi, esi	; Zero it!
		mov	si, [_z80pc]	; Get our program counter back
		mov	ebp, [_z80Base] ; Base pointer comes back
		add	esi, ebp	; Rebase it properly
		ret

directWriteHandler:
		sub	edx, [edi]	; Subtract our offset
		add	edx, [edi+12]	; Add in the base address
		mov	[edx], al	; Store our byte
		sub	edx, [edi+12]	; Restore our base address
		add	edx, [edi]	; And put our offset back
		ret


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

; This is a generic I/O read byte handler for when a foreign
; handler is to be called
; EDI=Handler address, EDX=I/O Address
; On return, EDX & EDI are undisturbed and AL=Byte read

ReadIOByte:
		mov	[_z80af], ax	; Save AF
		mov	[_z80hl], bx	; Save HL
		mov	[_z80bc], cx	; Save BC
		sub	esi, ebp	; Our program counter
		mov	[_z80pc], si	; Save our program counter
		mov	esi, [dwOriginalExec]	
		sub	esi, [cyclesRemaining]
		add	[dwElapsedTicks], esi
		add	[_z80rCounter], esi
		sub	[dwOriginalExec], esi
		push	edi	; Save our structure address
		push	edx	; And our desired I/O port
		call	dword [edi + 4]	; Go call our handler
		pop	edx	; Restore our address
		pop	edi	; Restore our handler's address
		xor	ebx, ebx	; Zero our future HL
		xor	ecx, ecx	; Zero our future BC
		xor	esi, esi	; Zero it!
		mov	si, [_z80pc]	; Get our program counter back
		mov	ebp, [_z80Base] ; Base pointer comes back
		add	esi, ebp	; Rebase it properly
		mov	bx, [_z80hl]	; Get HL back
		mov	cx, [_z80bc]	; Get BC back
		ret


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

; This is a generic write I/O byte handler when a foreign handler is to
; be called
; EDI=Handler address, AL=Byte to write, EDX=I/O Address
; EDI and EDX Are undisturbed on exit

WriteIOByte:
		mov	[_z80hl], bx	; Save HL
		mov	[_z80bc], cx	; Save BX
		sub	esi, ebp	; Our program counter
		mov	[_z80pc], si	; Save our program counter
		mov	esi, [dwOriginalExec]	
		sub	esi, [cyclesRemaining]
		add	[dwElapsedTicks], esi
		add	[_z80rCounter], esi
		sub	[dwOriginalExec], esi
		push	edi	; Save our structure address
		push	eax	; Data to write
		push	edx	; And our desired I/O address
		call	dword [edi + 4]	; Go call our handler
		pop	edx	; Restore our address
		pop	eax	; Restore our data written
		pop	edi	; Save our structure address
		xor	ebx, ebx	; Zero our future HL
		xor	ecx, ecx	; Zero our future BC
		mov	bx, [_z80hl]	; Get HL back
		mov	cx, [_z80bc]	; Get BC back
		mov	ax, [_z80af]	; Get AF back
		xor	esi, esi	; Zero it!
		mov	si, [_z80pc]	; Get our program counter back
		mov	ebp, [_z80Base] ; Base pointer comes back
		add	esi, ebp	; Rebase it properly
		ret

		global	_mz80GetContext
		global	mz80GetContext_
		global	mz80GetContext

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

mz80GetContext_:
_mz80GetContext:
mz80GetContext:
		mov	eax, [esp+4]	; Get our context address
		push	esi		; Save registers we use
		push	edi
		push	ecx
		push	es
		mov	di, ds
		mov	es, di
		mov	esi, _mz80contextBegin
		mov	edi, eax	; Source address in ESI
		mov     ecx, (_mz80contextEnd - _mz80contextBegin) >> 2
		rep	movsd
		mov     ecx, (_mz80contextEnd - _mz80contextBegin) & 0x03
		rep	movsb
		pop	es
		pop	ecx
		pop	edi
		pop	esi
		ret			; No return code
		global	_mz80SetContext
		global	mz80SetContext_
		global	mz80SetContext

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

mz80SetContext_:
_mz80SetContext:
mz80SetContext:
		mov	eax, [esp+4]	; Get our context address
		push	esi		; Save registers we use
		push	edi
		push	ecx
		push	es
		mov	di, ds
		mov	es, di
		mov	edi, _mz80contextBegin
		mov	esi, eax	; Source address in ESI
		mov     ecx, (_mz80contextEnd - _mz80contextBegin) >> 2
		rep	movsd
		mov     ecx, (_mz80contextEnd - _mz80contextBegin) & 0x03
		rep	movsb
		pop	es
		pop	ecx
		pop	edi
		pop	esi
		ret			; No return code
		global	_mz80GetContextSize
		global	mz80GetContextSize_
		global	mz80GetContextSize

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

mz80GetContextSize_:
_mz80GetContextSize:
mz80GetContextSize:
		mov     eax, _mz80contextEnd - _mz80contextBegin
		ret

		global	_mz80GetElapsedTicks
		global	mz80GetElapsedTicks_
		global	mz80GetElapsedTicks

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

mz80GetElapsedTicks_:
_mz80GetElapsedTicks:
mz80GetElapsedTicks:
		mov	eax, [esp+4]	; Get our context address
		or	eax, eax	; Should we clear it?
		jz	getTicks
		xor	eax, eax
		xchg	eax, [dwElapsedTicks]
		ret
getTicks:
		mov	eax, [dwElapsedTicks]
		ret
		global	_mz80ReleaseTimeslice
		global	mz80ReleaseTimeslice_
		global	mz80ReleaseTimeslice

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

mz80ReleaseTimeslice_:
_mz80ReleaseTimeslice:
mz80ReleaseTimeslice:
		mov	eax, [cyclesRemaining]
		sub	[dwOriginalExec], eax
		mov	[cyclesRemaining], dword 0
		ret

		global	_mz80reset
		global	mz80reset_
		global	mz80reset

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

mz80reset_:
_mz80reset:
mz80reset:
		xor	eax, eax 	; Zero AX

		mov	dword [_z80halted], eax	; We're not halted anymore!
		mov	word [_z80af], 0040h	; Zero A & flags - zero flag set
		mov	word [_z80bc], ax	; Zero BC
		mov	word [_z80de],	ax	; Zero DE
		mov	word [_z80hl], ax	; Zero HL
		mov	word [_z80afprime], ax	; Zero AF Prime
		mov	word [_z80bcprime], ax	; Zero BC prime
		mov	word [_z80deprime], ax ; Zero DE prime
		mov	word [_z80hlprime], ax ; Zero HL prime
		mov	byte [_z80i], al	; Zero Interrupt register
		mov	byte [_z80r], al	; Zero refresh register
		mov	word [_z80ix], 0ffffh	; Default mz80Index register
		mov	word [_z80iy], 0ffffh	; Default mz80Index register
		mov	word [_z80pc], ax	; Zero program counter
		mov	word [_z80sp], ax	; And the stack pointer
		mov	dword [_z80iff], eax ; IFF1/IFF2 disabled!
		mov	dword [_z80interruptMode], eax ; Clear our interrupt mode (0)
		mov	word [_z80intAddr], 38h ; Set default interrupt address
		mov	word [_z80nmiAddr], 66h ; Set default nmi addr

		ret

		global	_mz80int
		global	mz80int_
		global	mz80int

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

mz80int_:
_mz80int:
mz80int:
		mov	eax, [esp+4]	; Get our (potential) lower interrupt address
		mov	dword [_z80halted], 0	; We're not halted anymore!
		mov	ah, IFF1	; Is IFF1 enabled?
		and	ah, [_z80iff]	; Well, is it?
		jz		near interruptsDisabled

; Interrupts enabled. Clear IFF1 and IFF2

		and	dword [_z80iff], ~(IFF1 | IFF2);

		mov	[_z80intPending], byte 0

		push	ebp
		push	edi
		push	edx
		mov	ebp, [_z80Base]

		mov	dx, [_z80pc]
		xor	edi, edi
		mov	di, word [_z80sp]
		sub	di, 2
		mov	word [_z80sp], di
		mov	[ebp+edi], dx
		cmp	dword [_z80interruptMode], 2 ; Are we lower than mode 2?
		jb		justModeTwo
		mov	ah, [_z80i]	; Get our high address here
		and	eax, 0ffffh ; Only the lower part
		mov	ax, [eax+ebp] ; Get our vector
		jmp	short setNewVector ; Go set it!
justModeTwo:
		mov	ax, word [_z80intAddr]
setNewVector:
		mov	[_z80pc], ax

		pop	edx
		pop	edi
		pop	ebp

		xor	eax, eax	; Zero this so we can use it as an index
		mov	al, [_z80interruptMode]
		mov	al, [intModeTStates+eax]
		add	[dwElapsedTicks], eax
		add	[_z80rCounter], eax
		xor	eax, eax	; Indicate we took the interrupt
		jmp	short z80intExit

interruptsDisabled:
		mov	[_z80intPending], byte 1
		mov	[_intData], al	; Save this info for later
		mov	eax, 0ffffffffh		; Indicate we didn't take it

z80intExit:
		ret

		global	_mz80ClearPendingInterrupt
		global	mz80ClearPendingInterrupt_
		global	mz80ClearPendingInterrupt

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

mz80ClearPendingInterrupt_:
_mz80ClearPendingInterrupt:
mz80ClearPendingInterrupt:
		mov	[_z80intPending], byte 0
		ret

		global	_mz80nmi
		global	mz80nmi_
		global	mz80nmi

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

mz80nmi_:
_mz80nmi:
mz80nmi:
		mov	dword [_z80halted], 0	; We're not halted anymore!
		mov	al, [_z80iff]	; Get our IFF setting
		and	al, IFF1	; Just IFF 1
		shl	al, 1	; Makes IFF1->IFF2 and zeros IFF1
		mov	[_z80iff], al	; Store it back to the interrupt state!

		push	ebp
		push	edi
		mov	ebp, [_z80Base]

		xor	eax, eax
		mov	ax, [_z80pc]
		xor	edi, edi
		mov	di, word [_z80sp]
		sub	di, 2
		mov	word [_z80sp], di
		mov	[ebp+edi], ax
		mov	ax, [_z80nmiAddr]
		mov	[_z80pc], ax

		add	[dwElapsedTicks], dword 11	; 11 T-States for NMI
		add	[_z80rCounter], dword 11
		pop	edi
		pop	ebp

		xor	eax, eax	; Indicate we took the interrupt
		ret
		global	_mz80exec
		global	mz80exec_
		global	mz80exec

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

mz80exec_:
_mz80exec:
mz80exec:
		mov	eax, [esp+4]	; Get our execution cycle count
		push	ebx			; Save all registers we use
		push	ecx
		push	edx
		push	ebp
		push	esi
		push	edi

		mov	edi, eax
		mov	dword [cyclesRemaining], eax	; Store # of instructions to
		mov	[dwLastRSample], eax
		mov	[dwOriginalExec], eax	; Store this!
		cmp	dword [_z80halted], 0
		je	goCpu
		add	[_z80rCounter], eax
		add	dword [dwElapsedTicks], eax
		mov	dword [cyclesRemaining], 0	; Nothing left!
		mov	eax, 80000000h	; Successful exection
		jmp	popReg
goCpu:
		cld				; Go forward!

		xor	eax, eax		; Zero EAX 'cause we use it!
		xor	ebx, ebx		; Zero EBX, too
		xor	ecx, ecx		; Zero ECX
		xor	edx, edx		; And EDX
		xor	esi, esi		; Zero our source address

		mov	ax, [_z80af]		; Accumulator & flags
		xchg	ah, al		; Swap these for later
		mov	bx, [_z80hl]		; Get our HL value
		mov	cx, [_z80bc]		; And our BC value
		mov	ebp, [_z80Base]		; Get the base address
		mov	si, [_z80pc]		; Get our program counter
		add	esi, ebp		; Add in our base address
		cmp	[_z80intPending], byte 0	; Interrupt pending?
		jz		masterExecTarget

		call	causeInternalInterrupt

masterExecTarget:
		mov	dl, [esi]
		inc	esi
		jmp	dword [z80regular+edx*4]

; We get to invalidInsWord if it's a double byte invalid opcode

invalidInsWord:
		dec	esi

; We get to invalidInsByte if it's a single byte invalid opcode

invalidInsByte:
		xchg	ah, al		; Swap them back so they look good
		mov	[_z80af], ax		; Store A & flags
		dec	esi			; Back up one instruction...
		mov	edx, esi		; Get our address in EAX
		sub	edx, ebp		; And subtract our base for
						; an invalid instruction
		jmp	short emulateEnd

noMoreExec:
		cmp	[bEIExit], byte 0	; Are we exiting because of an EI?
		jne	checkEI
noMoreExecNoEI:
		xchg	ah, al		; Swap these for later
		mov	[_z80af], ax		; Store A & flags
		mov	edx, [dwOriginalExec]	; Original exec time
		sub	edx, edi		; Subtract # of cycles remaining
		add	[_z80rCounter], edx
		add	[dwElapsedTicks], edx	; Add our executed time
		mov	edx, 80000000h		; Indicate successful exec
		jmp	short emulateEnd	; All finished!

; Now let's tuck away the virtual registers for next time

storeFlags:
		xchg	ah, al		; Swap these for later
		mov	[_z80af], ax		; Store A & flags
emulateEnd:
		mov	[_z80hl], bx		; Store HL
		mov	[_z80bc], cx		; Store BC
		sub	esi, [_z80Base]		; Knock off physical address
		mov	[_z80pc], si		; And store virtual address
		mov	eax, edx		; Result code return

popReg:
		pop	edi			; Restore registers
		pop	esi
		pop	ebp
		pop	edx
		pop	ecx
		pop	ebx

		ret


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

checkEI:
		xor	edx, edx
		mov	[bEIExit], byte 0
		sub	edx, edi	; Find out how much time has passed
		mov	edi, [dwEITiming]
		sub	edi, edx
		js		noMoreExecNoEI
		xor	edx, edx
		cmp	[_z80intPending], byte 0
		je	near masterExecTarget
		call	causeInternalInterrupt
		jmp	masterExecTarget


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

causeInternalInterrupt:
		mov	dword [_z80halted], 0	; We're not halted anymore!
		test	[_z80iff], byte IFF1	; Interrupt enabled yet?
		jz		near internalInterruptsDisabled

; Interrupts enabled. Clear IFF1 and IFF2

		mov	[_z80intPending], byte 0

; Save off our active register sets

		xchg	ah, al		; Swap these for later
		mov	[_z80af], ax		; Store A & flags
		mov	[_z80hl], bx		; Store HL
		mov	[_z80bc], cx		; Store BC
		sub	esi, ebp			; Knock off physical address
		mov	[_z80pc], si		; And store virtual address
		xor	eax, eax
		mov	al, [_intData]


		push	edi

		mov	dx, [_z80pc]
		xor	edi, edi
		mov	di, word [_z80sp]
		sub	di, 2
		mov	word [_z80sp], di
		mov	[ebp+edi], dx
		cmp	dword [_z80interruptMode], 2 ; Are we lower than mode 2?
		jb		internalJustModeTwo
		mov	ah, [_z80i]	; Get our high address here
		and	eax, 0ffffh ; Only the lower part
		mov	ax, [eax+ebp] ; Get our vector
		jmp	short internalSetNewVector ; Go set it!
internalJustModeTwo:
		mov	ax, word [_z80intAddr]
internalSetNewVector:
		mov	[_z80pc], ax

		pop	edi

		xor	eax, eax	; Zero this so we can use it as an index
		mov	al, [_z80interruptMode]
		mov	al, [intModeTStates+eax]
		sub	edi, eax
		add	[_z80rCounter], eax

; Restore all the registers and whatnot

		mov	ax, [_z80af]		; Accumulator & flags
		xchg	ah, al		; Swap these for later
		mov	bx, [_z80hl]		; Get our HL value
		mov	cx, [_z80bc]		; And our BC value
		mov	ebp, [_z80Base]		; Get the base address
		mov	si, [_z80pc]		; Get our program counter
		add	esi, ebp		; Add in our base address
internalInterruptsDisabled:
		xor	edx, edx
		ret
		global	_mz80init
		global	mz80init_
		global	mz80init

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

mz80init_:
_mz80init:
mz80init:
		ret

		global	_mz80shutdown
		global	mz80shutdown_
		global	mz80shutdown

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

mz80shutdown_:
_mz80shutdown:
mz80shutdown:
		ret


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

;
; In : EAX=Reg #, ESI=Context address
; Out: EAX=Value of register
;
getRegValueInternal:
		push	ecx
		push	edx

		cmp	eax, CPUREG_MAXINDEX
		jae	badIndex2

		shl	eax, 4	; Times 16 for table entry size
		add	eax, RegTable	; Now it's the memory location
		mov	edx, [eax+4]	; Get the offset of the register
		mov	edx, [edx + esi]	; Get our value
		mov	ecx, [eax+8]	; Get our shift value
		shr	edx, cl			; Shift it right by a value
		and	edx, [eax+12]	; Mask off any unneeded bits
		mov	eax, edx			; Put our value in EAX
		jmp	short indexExit	; Index's exit!
badIndex2:
		mov	eax, 0ffffffffh

indexExit:
		pop	edx
		pop	ecx
		ret


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

;
; In : EAX=Value, EDX=Reg #, ESI=Context address
; Out: EAX=Value of register
;
convertValueToText:
		push	ecx
		push	edx

		cmp	edx, CPUREG_MAXINDEX
		jae	badIndex3

		shl	edx, 4	; Times 16 for table entry size
		add	edx, RegTable	; Now it's the memory location
		mov	edx, [edx + 12] ; Shift mask
		xor	ecx, ecx	; Zero our shift
shiftLoop:
		test	edx, 0f0000000h	; High nibble nonzero yet?
		jnz	convertLoop		; Yup!
		shl	edx, 4			; Move over, bacon
		shl	eax, 4		; Move the value over, too
		jmp	short shiftLoop	; Keep shiftin'

convertLoop:
		mov	ecx, eax			; Get our value
		shr	ecx, 28			; Only the top nibble
		add	cl, '0'			; Convert to ASCII
		cmp	cl, '9'			; Greater than 9?
		jbe	noAdd				; Nope! Don't add it
		add	cl, 32+7			; Convert from lowercase a-f
noAdd:
		mov	[edi], cl		; New value storage
		inc	edi			; Next byte, please
		shl	eax, 4			; Move the mask over
		shl	edx, 4			; Move the mask over
		jnz	convertLoop		; Keep convertin'

badIndex3:
		mov	[edi], byte 0	; Null terminate the sucker!
		pop	edx
		pop	ecx
		ret

		global	_mz80SetRegisterValue
		global	mz80SetRegisterValue_
		global	mz80SetRegisterValue

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

mz80SetRegisterValue_:
_mz80SetRegisterValue:
mz80SetRegisterValue:
		push	esi
		push	edi
		push	edx
		push	ecx
		mov	eax, [esp+20]	; Get our register #
		mov	esi, [esp+24]	; Get our context address
		mov	edi, [esp+28]	; Value to assign
		or	esi, esi	; Are we NULL?
		jnz	userDefined
		mov	esi, _mz80contextBegin
userDefined:

		shl	eax, 4	; Times 16 for reg entry size
		add	eax, RegTable
		mov	edx, [eax+12] ; Our mask
		not	edx	; Invert EDX!
		test	edi, edx	; Did we set any invalid bits?
		jnz	rangeViolation

		not	edx	; Toggle it back to normal
		mov	ecx, [eax+8]	; Get our shift value
		shl	edx, cl	; Shift our mask
		shl	eax, cl	; And our value to OR in
		not	edx	; Make it the inverse of what we want
		mov	eax, [eax+4]	; Get our offset into the context
		and	[esi+eax], edx	; Mask off the bits we're changin
		or	[esi+eax], edi	; Or in our new value

		xor	eax, eax
		jmp	short  setExit

rangeViolation:
		mov	eax, 0ffffffffh

setExit:
		pop	ecx
		pop	edx
		pop	edi
		pop	esi

		ret


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

		global	_mz80GetRegisterValue
		global	mz80GetRegisterValue_
		global	mz80GetRegisterValue

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

mz80GetRegisterValue_:
_mz80GetRegisterValue:
mz80GetRegisterValue:
		push	esi
		mov	eax, [esp+8]	; Get our register #
		mov	esi, [esp+12]	; Get our context address
		or	esi, esi	; Is context NULL?
		jnz	getVal	; Nope - use it!
		mov	esi, _mz80contextBegin

getVal:
		call	getRegValueInternal

		pop	esi
		ret


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

		global	_mz80GetRegisterName
		global	mz80GetRegisterName_
		global	mz80GetRegisterName

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

mz80GetRegisterName_:
_mz80GetRegisterName:
mz80GetRegisterName:
		mov	eax, [esp+4]	; Get our register #
		cmp	eax, CPUREG_MAXINDEX
		jae	badIndex
		shl	eax, 4	; Times 16 bytes for each entry
		mov	eax, [eax+RegTable]
		jmp	nameExit

badIndex:
		xor	eax, eax

nameExit:
		ret


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

		global	_mz80GetRegisterTextValue
		global	mz80GetRegisterTextValue_
		global	mz80GetRegisterTextValue

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

mz80GetRegisterTextValue_:
_mz80GetRegisterTextValue:
mz80GetRegisterTextValue:
		push	esi
		push	edi
		push	edx
		mov	eax, [esp+16]	; Get our register #
		mov	esi, [esp+20]	; Get our context address
		mov	edi, [esp+24]	; Address to place text
		or	esi, esi	; Is context NULL?
		jnz	getVal2	; Nope - use it!
		mov	esi, _mz80contextBegin

getVal2:
		mov	edx, eax	; Save off our index for later
		call	getRegValueInternal

; EAX Holds the value, EDX=Register #, and EDI=Destination!

		call	convertValueToText

		pop	edx
		pop	esi
		pop	edi
		ret


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

		global	_mz80WriteValue
		global	mz80WriteValue_
		global	mz80WriteValue

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

mz80WriteValue_:
_mz80WriteValue:
mz80WriteValue:
		push	esi
		push	edi
		push	edx
		push	ebx
		push	ecx
		push	ebp
		mov	eax, [esp+28]	; What kind of write is this?
		mov	ebx, [esp+32]	; Address
		mov	edx, [esp+36]	; Value
		cmp	eax, 1	; Is it a word write?
		je	near invalidWrite	; Yep - it's not valid
		cmp	eax, 2	; Is it a dword write?
		je	near invalidWrite	; Yep - it's not valid

		or	eax, eax	; Is it a byte write?
		jnz	itsIoDummy	; Nope... it's an I/O write

		mov	ebp, [_z80Base] ; Base pointer comes back
		mov	edi, [_z80MemWrite]	; Point to the write array
checkLoop:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryWrite	; Yes - go write it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddr	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutine	; If not, go call it!
nextAddr:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoop
callRoutine:

;
; EBX=Address to target, DL=Byte to write 
;

		cmp	[edi+8], dword 0	; Null handler?
		je	directWriteHandler2

		push	edi	; Handler
		push	edx	; Byte
		push	ebx	; Address
		call	dword [edi + 8]	; Go call our handler
		add	esp, 12
		jmp	short itsGood
directWriteHandler2:
		sub	ebx, [edi]	; Subtract our offset
		add	ebx, [edi+12]	; Add in the base address
		mov	[ebx], dl	; Store our byte
		jmp	short itsGood
memoryWrite:
		mov	[ebp + ebx], dl

		jmp	short itsGood
itsIoDummy:
		mov	edi, [_z80IoWrite]	; Point to the I/O write array
IOCheck:
		cmp	[edi], word 0ffffh ; End of our list?
		je	itsGood	; Yes - ignore it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextIOAddr	; Yes... go to the next addr
		cmp	bx, [edi+2]	; Are we bigger?
		jbe	callIOHandler	; If not, go call it!
nextIOAddr:
		add	edi, 0ch		; Next structure, please
		jmp	short IOCheck
callIOHandler:
		push	edi	; Handler
		push	edx	; Byte
		push	ebx	; Address
		call	dword [edi+4]	; Call the handler!
		add	esp, 12
		jmp	short itsGood

invalidWrite:
		mov	eax, 0ffffffffh
		jmp	short writeValueExit

itsGood:
		xor	eax, eax

writeValueExit:
		pop	ebp
		pop	ecx
		pop	ebx
		pop	edx
		pop	esi
		pop	edi
		ret


times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

		global	_mz80ReadValue
		global	mz80ReadValue_
		global	mz80ReadValue

times ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary

mz80ReadValue_:
_mz80ReadValue:
mz80ReadValue:
		push	esi
		push	edi
		push	edx
		push	ebx
		push	ecx
		push	ebp
		mov	eax, [esp+28]	; What kind of read is this?
		mov	ebx, [esp+32]	; Address
		cmp	eax, 1	; Is it a word read?
		je	near invalidRead	; Yep - it's not valid
		cmp	eax, 2	; Is it a dword read?
		je	near invalidRead	; Yep - it's not valid

		or	eax, eax	; Is it a byte read?
		jnz	itsIoDummyRead	; Nope... it's an I/O read

		mov	ebp, [_z80Base] ; Base pointer comes back
		mov	edi, [_z80MemRead]	; Point to the read array
checkLoopRead:
		cmp	[edi], word 0ffffh ; End of our list?
		je	memoryRead	; Yes - go read it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextAddrRead	; Yes... go to the next addr
		cmp	bx, [edi+4]	; Are we smaller?
		jbe	callRoutineRead	; If not, go call it!
nextAddrRead:
		add	edi, 10h		; Next structure, please
		jmp	short checkLoopRead
callRoutineRead:

;
; EBX=Address to target
;

		cmp	[edi+8], dword 0 ; NULL HAndler?
		je	handleSharedRead

		push	edi	; Handler
		push	ebx	; Address
		call	dword [edi + 8]	; Go call our handler
		mov	dl, al	; Get our byte read
		add	esp, 8
		jmp	short itsGoodRead

memoryRead:
		mov	dl, [ebp+ebx]

		jmp	short itsGoodRead

handleSharedRead:
		sub	ebx, [edi]
		add	ebx, [edi+12]
		mov	dl, [ebx]
		jmp	short itsGoodRead

itsIoDummyRead:
		mov	edi, [_z80IoRead]	; Point to the I/O read array
		mov	dl, 0ffh	; Assume no handler
IOCheckRead:
		cmp	[edi], word 0ffffh ; End of our list?
		je	itsGoodRead	; Yes - ignore it!
		cmp	bx, [edi]	; Are we smaller?
		jb	nextIOAddrRead	; Yes... go to the next addr
		cmp	bx, [edi+2]	; Are we bigger?
		jbe	callIOHandlerRead	; If not, go call it!
nextIOAddrRead:
		add	edi, 0ch		; Next structure, please
		jmp	short IOCheckRead
callIOHandlerRead:
		push	edi	; Handler
		push	ebx	; Address
		call	dword [edi+4]	; Call the handler!
		mov	dl, al	; Get our byte read
		add	esp, 8
		jmp	short itsGoodRead

invalidRead:
		mov	eax, 0ffffffffh
		jmp	short ReadValueExit

itsGoodRead:
		xor	eax, eax
		mov	al, dl

ReadValueExit:
		pop	ebp
		pop	ecx
		pop	ebx
		pop	edx
		pop	esi
		pop	edi
		ret


%ifdef NASM_STACK_NOEXEC
section .note.GNU-stack noalloc noexec nowrite progbits
%endif
