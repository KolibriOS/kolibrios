VARIABLE wd 256 ALLOT

: setwd ( uaddr)
	COUNT 1+ SWAP 1- SWAP wd SWAP CMOVE ;

: _wd
	$" /sys/" setwd ; _wd

: "/ [CHAR] / ;

: strcat ( uaddr1 uaddr2 -- uaddr1+uaddr2)
	>R DUP COUNT ( ua1 a1 c1)
	>R R@ ( ua1 a1 c1)
	+ OVER R> R@ SWAP >R ( ua1 ea1 ua1 ua1)
	C@ R> + ( ua1 ea1 ua1 u+u2 )
	SWAP C! ( ua1 ea1 )
	R> COUNT ( ua1 ea1 a2 c2)
	>R
	SWAP R> CMOVE ;

: add/ ( uaddr -- uaddr/)
	DUP DUP COUNT SWAP DROP + C@ "/ = IF ELSE $" /" strcat THEN	;

: cut/ ( uaddr -- uaddr w/o slash)
	COUNT OVER SWAP + 1- ( ua1 lasta1)
	DUP C@ "/ = IF 1- THEN .S
	BEGIN 2DUP < WHILE 4 . DUP C@ "/ = IF OVER - OVER 1- C! 1- LEAVE ELSE 1- THEN
	REPEAT ;

: t $" cat" $" dog" strcat COUNT TYPE ;

: makepath ( path normalizer: uaddr1 uaddr2 -- uaddr )
	DUP 1+ C@ [CHAR] / = IF SWAP DROP ELSE DUP strcat THEN
	add/ ;


: pwd ( print working directory:  -- )
	CR wd COUNT TYPE ;

: cd ( change directory)
	wd BL WORD makepath setwd ;
