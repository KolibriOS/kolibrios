\ S" /sys/1st.4th" INCLUDED
 S" /sys/locals.f" INCLUDED

\ : sys_wnd ( border, header, workarea, y, x -- )
\   1 12 sys2 DROP 0 sys6 2 12 sys2 2DROP ;

\ : thread ( stack, entry -- )
\  1 51 sys4 DROP ;

: >regs
	reg_struc ! ;

: new_reg
	CREATE 6 CELLS ALLOT LAST @ NAME> 9 + >regs ;

: ax reg_struc @ ! ;
: bx reg_struc @ 1 CELLS + ! ;
: cx reg_struc @ 2 CELLS + ! ;
: dx reg_struc @ 3 CELLS + ! ;
: si reg_struc @ 4 CELLS + ! ;
: di reg_struc @ 5 CELLS + ! ;

: << ( x,y -- x<<16+y )
	SWAP 16 LSHIFT + ;

: sys_print ( color, stra, u, x, y -- )
  << >R SWAP ROT R> 4 sys5 DROP ;
