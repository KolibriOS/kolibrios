DECIMAL
 S" /rd/1/menuet.f" INCLUDED

: not_emit
	emit_proc DUP @ NOT SWAP ! ;

new_reg wnd_size

CREATE Music $3090 , \ $90 C, $30 C, 0 C,

73 CONSTANT img.width
22 CONSTANT img.height
CREATE raw  img.width img.height * 3 * 16 + ALLOT

: my_wnd
  12 ax 1 bx sysv DROP $805080D0 DUP $02AABBCC 200 150 << 200 DUP << 0
 	sys6 2 bx sysv 2DROP
  $10DDEEFF $" ˆŒ… Žƒ€ŒŒ›" COUNT 8 DUP sys_print
  0 $"  ¦¬¨â¥ «î¡ãî ª« ¢¨èã" COUNT 8 30 sys_print
  $6688DD 1 5 12 << 200 19 - 12 << 8 sys5 DROP 
  30 DUP << img.width img.height << raw 12 + 7 sys4 ;

: my_wnd_resize
	200 dx 50 si 67 ax -1 DUP bx cx sysv DROP ;

: my_key
	2 sys1 8 RSHIFT DUP 96 = IF not_emit DROP ELSE Music DUP si 1+ C!
 55 DUP ax bx sysv DROP THEN ;

: my_btn
	17 sys1 8 RSHIFT ." Pressed button #" DUP . CR 1 = IF BYE THEN ;

CREATE handlers ' my_wnd , ' my_key , ' my_btn ,

VARIABLE hnd

: msg_loop_console ( subs -- )
	CR DUP hnd ! @EXECUTE  my_wnd_resize
	0 emit_proc !
	BEGIN
	10 sys1 ?DUP
	IF
	  1-
    CELLS hnd @ + @EXECUTE
    emit_proc @
    IF
    	WINDOW KEY 96 =
    	IF
    		not_emit hnd @ @EXECUTE  my_wnd_resize
    	ELSE
    	  EXIT
    	THEN
    THEN
  THEN
  AGAIN ;

CREATE gif 600 ALLOT


: new_me ( new main loop)
  gif DUP $" /rd/1/Menu.gif" COUNT READ 2DROP
  gif raw READ_GIF .
  ;

new_me handlers msg_loop_console WORDS

ABORT


        : ENDOF ( orig1 #of -- orig2 #of )
          >R    ( ïåðåìåñòèòü ñî ñòåêà â ñëó÷àå, åñëè )
                ( ñòåê ïîòîêà óïðàâëåíèÿ ýòî ñòåê äàííûõ. )
          POSTPONE ELSE
          R>    ( ìû äîëæíû òåïåðü âåðíóòü ñ÷åò÷èê íàçàä )
        ; IMMEDIATE

        : ENDCASE  ( orig1..orign #of -- )
        	
          POSTPONE DROP  ( óäàëèòü case ïàðàìåòð )
          0 ?DO
            POSTPONE THEN
          LOOP
        ; IMMEDIATE

[THEN]

          : SS2   ( N ---> S:ÑÓÌÌÀ ÊÂÀÄÐÀÒÎÂ ÎÒ 1 ÄÎ N)
              0  SWAP           ( 0,N         S[0]=0  )
              1+  1             ( S[0],N+1,1          )
                    DO   I      ( S[I-1],I            )
                      DUP * +       ( S[I] S[I]=S[I-1]+I*I)
                    LOOP  ;     ( S[N]   )


 5 SS2 . CR
 
: test 10 0 ?DO I . LEAVE LOOP 4 ;
test 

    : priem ( N:ÍÎÌÅÐ ÄÍß->)   CASE
        3 OF ." nepriemn"  ENDOF

        1 OF ." priemn" ENDOF

        6 OF ." holiday" ENDOF

        CR . ." - day #?" ABORT 
        ( ENDCASE ) ." day" ;

