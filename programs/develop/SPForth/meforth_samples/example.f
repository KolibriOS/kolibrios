DECIMAL 
 S" /sys/menuet.f" INCLUDED

: not_emit
	emit_proc DUP @ NOT SWAP ! ;

new_reg wnd_size

CREATE Music $3090 , \ $90 C, $30 C, 0 C,

: my_wnd
  12 ax 1 bx sysv DROP $805080D0 DUP $02AABBCC 200 50 << 200 DUP << 0
 	sys6 2 bx sysv 2DROP
  $10DDEEFF $" ˆŒ… Žƒ€ŒŒ›" COUNT 8 DUP sys_print
  0 $"  ¦¬¨â¥ «î¡ãî ª« ¢¨èã" COUNT 8 30 sys_print
  $6688DD 1 5 12 << 200 19 - 12 << 8 sys5 DROP ;

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

: new_me ( new main loop)
  handlers msg_loop_console ;
