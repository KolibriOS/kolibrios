
: TYPE TYPE ;
: (ABORT") (ABORT") ;
\ : COMPILE, COMPILE, ;

: CUR_POS ABORT ;
: B_CR ( -- ) \ 94
\ Перевод строки.
 10 EMIT
 13 EMIT
;

: GETPR      ABORT ;

: SPP_M		ABORT ;
: TIBB_M	ABORT ;
: NTIB_M	ABORT ;
: NTIB		ABORT ;
: draw_window	ABORT ;
: CC_LINES	ABORT ;
: ?KEY		ABORT ;
: EMIT_N	ABORT ;

 : DR_CUR	ABORT ;
 : CL_CUR	ABORT ;

: KEY_M		ABORT ;
: _BYE		ABORT ;
 
: ROWH		ABORT ;
: ROWW		ABORT ;
: draw_window	ABORT ;
: MEMS		ABORT ;
\ FORWORD

: READ		ABORT ;
: CLIT, ABORT ;
: _PREVIOUS PREVIOUS ;
: _SHEADER  SHEADER ;
: _:  : ;
: [;] POSTPONE  ; ;

: S,	ABORT ;
: SWORD, ABORT ;
: PARSE-NAME PARSE-WORD ;
: PSLITERAL POSTPONE SLITERAL ;