(

)

Code	SYS1 ;( n -- n1 )
	INT 40H
	RET
EndCode

Code	SYS2 ;( n n1 -- n2 )
	MOV EBX, [EBP]
	INT 40H
	LEA EBP, [EBP+4]
	RET
EndCode

Code	SYS3 ;( n n1 n2 -- n3 )
	MOV ECX, [EBP+4]
	MOV EBX, [EBP]
	INT 40H
	LEA EBP, [EBP+8]
	RET
EndCode

Code	SYS4 ;( n n1 n2 n3 -- n4 )
	MOV EDX, [EBP+8]
	MOV ECX, [EBP+4]
	MOV EBX, [EBP]
	INT 40H
	LEA EBP, [EBP+0CH]
	RET
EndCode

Code	SYS5 ;( n n1 n2 n3 n4 -- n5 )
	MOV ESI, [EBP+0CH]
	MOV EDX, [EBP+8]
	MOV ECX, [EBP+4]
	MOV EBX, [EBP]
	INT 40H
	LEA EBP, [EBP+10H]
	RET
EndCode

Code	SYS6 ;( n n1 n2 n3 n4 n5 -- n6 )
	PUSH EDI
	MOV EDI, [EBP+10H]
	MOV ESI, [EBP+0CH]
	MOV EDX, [EBP+8]
	MOV ECX, [EBP+4]
	MOV EBX, [EBP]
	INT 40H
	LEA EBP, [EBP+14H]
	POP EDI
	RET
EndCode

VARIABLE reg_struc

Code	SYSV ;(  -- n )
	LEA EBP,  [EBP-4]
	MOV [EBP], EAX
	MOV EAX, { ' reg_struc }
	PUSH EDI
	MOV EDI, [EAX+14H]
	MOV ESI, [EAX+10H]
	MOV EDX, [EAX+0CH]
	MOV ECX, [EAX+8]
	MOV EBX, [EAX+4]
	MOV EAX, [EAX]
	INT 40H
	POP EDI
	RET
EndCode



80 VALUE SCR_WIDTH
60 VALUE SCR_WIDTH-S
25 VALUE SCR_HEIGHT


CREATE &AT-XY 0 , 0 ,
CREATE &KEY 0 ,
CREATE &ATRIB 0x0000FFFF ,

0x0011000D VALUE >PIC

CREATE SCR_BUF SCR_WIDTH SCR_HEIGHT 1+ * ALLOT

: AT-XY ( X Y -- )
 SWAP 16 LSHIFT + &AT-XY ! ;

: AT-XY? ( -- X Y )
 &AT-XY 2+ W@ &AT-XY W@ ;

: __PAGE-UP
  SCR_BUF SCR_WIDTH + SCR_BUF SCR_WIDTH SCR_HEIGHT * CMOVE
  SCR_BUF SCR_WIDTH SCR_HEIGHT 1- * + SCR_WIDTH  BLANK
  AT-XY? 1- 0 MAX AT-XY ;

VECT PAGE-UP

: ?PAGE-UP ( n -- )

 &AT-XY	2+	W@ + SCR_WIDTH	/MOD SWAP &AT-XY 2+	W!
 &AT-XY		W@ + SCR_HEIGHT	/MOD 
 IF PAGE-UP DROP SCR_HEIGHT 1- THEN  &AT-XY 	W! ;

: SCR_CR
 &AT-XY @ 0xFFFF AND 1+ &AT-XY ! 0 ?PAGE-UP ;

: SCR_TYPE ( addr len -- )
	TUCK SWAP
  &ATRIB @
  &AT-XY 2+	W@  0x60000 *
  &AT-XY	W@  0xD * +
 0x00060018 + 
 4 SYS5 DROP
\  &AT-XY 2+ +!
  ?PAGE-UP
;

: DRAW_LINS
	['] PAGE-UP >BODY @
	['] NOOP TO PAGE-UP
	AT-XY?
	0 0 AT-XY
	SCR_BUF SCR_WIDTH SCR_HEIGHT * BOUNDS
	DO I SCR_WIDTH SCR_TYPE \ SCR_CR
 SCR_WIDTH
	+LOOP
	AT-XY
	TO PAGE-UP ;

: _PAGE-UP
draw_window __PAGE-UP
DRAW_LINS
;

' _PAGE-UP TO PAGE-UP

CREATE LAST_CUR 0 , 0 ,

: CORSOR_DROW
  0  LAST_CUR 2@ 38 SYS4 DROP
  0x00FF00FF
  &AT-XY	W@ 0xD * 0x21	+ DUP	  16 LSHIFT + 
  &AT-XY 2+	W@ 0x6 * 6	+ DUP 6 + 16 LSHIFT +
 2DUP LAST_CUR 2!
  38 SYS4 DROP
;
: REDRAW
	draw_window
	DRAW_LINS CORSOR_DROW ;

: EVENT-CASE
     11 SYS1
	DUP	1 = IF DROP REDRAW	EXIT	THEN
	DUP	2 = IF DROP 2 SYS1 8 RSHIFT &KEY C! EXIT	THEN
		3 = IF -1 SYS1	EXIT	THEN ;


0 
  CELL	FIELD .CODE
2 CELLS	FIELD .FIRST \ позиция в файле (в байтах)
  CELL	FIELD .SIZE  \ сколько байт R/W
  CELL	FIELD .DATA  \ указатель на данные
  222	FIELD .NAME  \ ASCIIZ-имя файла
CONSTANT FILE_STR

: WINDOW
 draw_window CC_LINES ;

