REQUIRE [IF] ~MAK\CompIF.f

C" I'" FIND NIP
[IF]
: I'  BL WORD  FIND 0= IF -321 THROW THEN (  -? ) ;
[THEN]


 0x75 CONSTANT 0=   0x74 CONSTANT 0<>   0x79 CONSTANT 0<
 0x78 CONSTANT 0>=  0x7D CONSTANT <     0x7C CONSTANT >=
 0x7F CONSTANT <=   0x7E CONSTANT >     0x73 CONSTANT U<
 0x72 CONSTANT U>=  0x77 CONSTANT U<=   0x76 CONSTANT U>
 0x71 CONSTANT OV   0x70 CONSTANT NOV   0xE1 CONSTANT <>C0=
 0xE2 CONSTANT C0=  0xE0 CONSTANT ?C0=  0xE3 CONSTANT C0<>

: SIF     (  - a) I'  EXECUTE  C,  HERE ( origin)  0 C, ( blank) ;
: SWHILE  ( a1 "opcode" - a2 a1) SIF SWAP ;
: STHEN   ( a -) HERE OVER 1+ - SWAP C! ;
: SELSE   ( a - a') 0xEB ( short jmp) C,
  HERE OVER - SWAP C!  HERE  0 C, ;
: SBEGIN  ( - a) HERE ;
: SUNTIL_ ( a opc -) C,  HERE  1+ - C, ;
: SUNTIL  ( a -) I'  EXECUTE  SUNTIL_ ;
: SAGAIN  ( a -) 0xEB SUNTIL_ ;
: SREPEAT ( a a1 -) SAGAIN STHEN  ;

: SLOOP   ( a -) 0xE2 SUNTIL_ ;
: SLOOPZ  ( a -) 0xE1 SUNTIL_ ;
: SLOOPNZ ( a -) 0xE0 SUNTIL_ ;

: LIF    ( opcode - a) I'  EXECUTE  0xF10 + W,  HERE ( origin)  0 , ( blank) ;
: LWHILE ( a1 opcode - a2 a1) LIF SWAP ;
: LTHEN  ( a -) HERE OVER CELL+ - SWAP ! ;
: LELSE  ( a - a') 0xE9 ( short jmp) C,
  HERE OVER - SWAP !  HERE  0 , ;
: LUNTIL ( a opc -)  I'  EXECUTE 0xF10 +  W,  HERE  CELL+ - , ;
: LAGAIN  ( a -) 0xE9 C, HERE  CELL+ - , ;
: LREPEAT ( a -) LAGAIN LTHEN ;

: II  BL WORD  FIND1 0=  ;