\ 80386 DISASSEMBLER
\ ANDREW MCKEWAN, APRIL 1994
\ TOM ZIMMER,  05/18/94 PORT TO WIN32F
\ MODIFIED TO WORD IN DECIMAL 08/03/94 10:04 TJZ
\ 06-??-95 SMUB NEXT SEQUENCE DEFINED IN FKERNEL
\ 06-21-95 SMUB REMOVED REDUNDANT COUNT CALLS FROM TXB, LXS.
\ 04-??-97 EXTENDED BY C.L. TO INCLUDE P6 AND MMX INSTRUCTIONS
\ 14-11-2000 Adapted from SPFOPT (Michael Maximov) by Dmitry Yakimov

\ ??-11-2000 Fixed FE. FF. (Bandaletov) and H.R (Yakimov)
\ 15-11-2000 Fixed MV2 (Yakimov)
\ 25-12-2000 Added float literals recognition (Yakimov)
\ 26-07-2001 Fixed MVX (Maksimov)
\ 11-05-2004 Fixed FDA and CMV (Serguei Jidkov)

CR .( Loading Intel Pentium MMX disassembler...)

WARNING @
BASE @
GET-CURRENT
( warn base wid )

WARNING 0!
DECIMAL

REQUIRE [DEFINED] lib/include/tools.f
\ REQUIRE [IF] ~mak\CompIF.f
REQUIRE CASE lib/ext/case.f
\ REQUIRE WITHIN lib\include\core-ext.f
REQUIRE NextNFA lib/ext/vocs.f

: DEFER VECT ;

: DUP>R R> OVER >R >R ;

: UMAX ( D1 D2  -- FLAG )
   2DUP U< IF NIP ELSE DROP THEN ;


80 CONSTANT MAXSTRING

255 CONSTANT MAXCOUNTED   \ maximum length of contents of a counted string

: 0X  BASE @ HEX >R BL WORD ?LITERAL
      R> BASE ! ; IMMEDIATE

: "CLIP"        ( a1 n1 -- a1 n1' )   \ clip a string to between 0 and MAXCOUNTED
                MAXCOUNTED MIN 0 MAX ;

: PLACE         ( addr len dest -- )
                SWAP "CLIP" SWAP
                2DUP 2>R
                CHAR+ SWAP MOVE
                2R> C! ;

: +PLACE        ( addr len dest -- ) \ append string addr,len to counted
                                     \ string dest
                >R "CLIP" MAXCOUNTED  R@ C@ -  MIN R>
                                        \ clip total to MAXCOUNTED string
                2DUP 2>R

                COUNT CHARS + SWAP MOVE
                2R> +! ;

: C+PLACE       ( c1 a1 -- )    \ append char c1 to the counted string at a1
                DUP 1+! COUNT + 1- C! ;


: OFF     0! ;

: BLANK         ( addr len -- )     \ fill addr for len with spaces (blanks)
                BL FILL ;

128 CONSTANT SPCS-MAX  ( optimization for SPACES )

CREATE SPCS  SPCS-MAX ALLOT
       SPCS  SPCS-MAX BLANK

C" UPC" FIND NIP 0=
[IF]
: UPC  ( c -- c' )
   DUP [CHAR] Z U>
   IF  0xDF AND
   THEN   ;
[THEN]

: (D.)          ( d -- addr len )       TUCK DABS  <# #S ROT SIGN #> ;


80 VALUE COLS

: H.R           ( n1 n2 -- )    \ display n1 as a hex number right
                                \ justified in a field of n2 characters
                BASE @ >R HEX >R
                0 <# #S #> R> OVER - 0 MAX SPACES TYPE
                R> BASE ! ;

: H.N           ( n1 n2 -- )    \ display n1 as a HEX number of n2 digits
                BASE @ >R HEX >R
                0 <# R> 0 ?DO # LOOP #> TYPE
                R> BASE ! ;

0 VALUE DEFAULT-16BIT?

: DEFAULT-16BIT ( -- )
                TRUE TO DEFAULT-16BIT? ;

: DEFAULT-32BIT ( -- )
                FALSE TO DEFAULT-16BIT? ;

      DEFER SHOW-NAME   ( CFA -- )      \ DISPLAY NEAREST SYMBOL

0 VALUE BASE-ADDR

VOCABULARY DISASSEMBLER
ALSO DISASSEMBLER DEFINITIONS

CREATE S-BUF MAXSTRING ALLOT

: >S            ( A1 N1 -- )
                S-BUF +PLACE ;

: 0>S           ( -- )  \ RESET S-BUF
                S-BUF OFF ;

: SSPACES       ( N1 -- )
                SPCS SWAP S-BUF  +PLACE ;

: SSPACE        ( -- )
                1 SSPACES ;

: EMIT>S        ( C1 -- )
                S-BUF C+PLACE ;

: S>            ( -- A1 N1 )
                S-BUF COUNT ;

: (.S")         ( addr len -- )
                S-BUF +PLACE ;

: .S"           ( 'TEXT' -- )
                [CHAR] " PARSE
                POSTPONE SLITERAL
                POSTPONE (.S")  ; IMMEDIATE


: D.R>S         ( D W -- )
                >R (D.) R> OVER - SSPACES >S ;

: .R>S          ( N W -- )
                >R  S>D  R>  D.R>S ;

: U.R>S         ( U W -- )
                0 SWAP D.R>S ;

: H.>S          ( U -- )
                BASE @ SWAP  HEX 0 (D.) >S SSPACE   BASE ! ;

: H.R>S           ( N1 N2 -- )
                BASE @ >R HEX >R
                0 <# #S #> R> OVER - SSPACES >S
                R> BASE ! ;

: ?.NAME>S      ( CFA -- )
\ ELIMINATE " 0X"
                DUP   1 H.R>S SSPACE
                NEAR_NFA
                >R DUP
                IF .S"  ( " DUP COUNT >S
                     NAME> R> - DUP
                     IF   DUP .S" +" NEGATE H.>S
                     THEN DROP        .S"  ) "
                ELSE RDROP DROP
                THEN
                ;

' ?.NAME>S TO SHOW-NAME

\ 32 CONSTANT COMMENT-COL

0 VALUE SIZE
0 VALUE 16-BIT-DATA
0 VALUE 16-BIT-ADDR
0 VALUE PREFIX-OP
0 VALUE MMX-REG

: @+  ( ADDR -- ADDR N )  DUP CELL+ SWAP @ ;
: W@+ ( ADDR -- ADDR N )  DUP 2 + SWAP W@ ;

: SEXT  ( BYTE -- N )  DUP 128 AND IF 0xFFFFFF00 OR THEN ;
: MOD/SIB ( MOD-R-R/M -- R/M R MOD ) \ R INCLUDING GENERAL, SPECIAL, SEGMENT, MMX
          ( MOD-OP-R/M -- R/M OP MOD )
          ( S-I-B -- B I S )
          255 AND 8 /MOD 8 /MOD ;

: ???   ( N1 -- )
        .S" ??? " DROP ;

: SS. ( N ADR LEN W )  >R DROP  SWAP R@ * +  R> >S SSPACE ;

: TTTN ( CODE -- ) 15 AND S" O NOB AEE NEBEA S NSP NPL GELEG " 2 SS. ;

: SREG  ( SREG -- )  3 RSHIFT 7 AND S" ESCSSSDSFSGSXXXX" 2 SS. ;
: CREG  ( EEE --  )  3 RSHIFT 7 AND S" CR0???CR2CR3CR4?????????" 3 SS. ;
: DREG  ( EEE --  )  3 RSHIFT 7 AND S" DR0DR1DR2DR3??????DR6DR7" 3 SS. ;
: TREG  ( EEE --  )  3 RSHIFT 7 AND S" ?????????TR3TR4TR5TR6TR7" 3 SS. ; \ OBSOLETE
: MREG  ( N -- )  7 AND S" MM0MM1MM2MM3MM4MM5MM6MM7" 3 SS. ;

: REG8  ( N -- )  7 AND S" ALCLDLBLAHCHDHBH" 2 SS. ;
: REG16 ( N -- )  7 AND S" AXCXDXBXSPBPSIDI" 2 SS. ;
: REG32 ( N -- )  7 AND S" EAXECXEDXEBXESPEBPESIEDI" 3 SS. ;
: REG16/32      ( N -- )
                16-BIT-DATA
                IF   REG16
                ELSE REG32
                THEN  ;
: REG   ( A N -- A )
        MMX-REG
        IF   MREG
        ELSE SIZE
             IF   REG16/32
             ELSE REG8
             THEN
        THEN
 ;

: [BASE16] ( R/M -- )   4 - S" [SI][DI][BP][BX]" 4 SS. ;
                        \ R/M = 4 , 5 , 6 , 7
: [IND16]  ( R/M -- )   S" [BX+SI][BX+DI][BP+SI][BP+DI]" 7 SS. ;
                        \ R/M = 0  ,   1  ,   2  ,   3
: [REG16]  ( R/M -- )   DUP 4 <
                        IF    [IND16]
                        ELSE  [BASE16]
                        THEN ;
: [REG32]  ( N -- )     7 AND S" [EAX][ECX][EDX][EBX][ESP][EBP][ESI][EDI]" 5 SS. ;


: [REG*2]  ( I -- )     S" [EAX*2][ECX*2][EDX*2][EBX*2][XXX*2][EBP*2][ESI*2][EDI*2]" 7 SS. ;
: [REG*4]  ( I -- )     S" [EAX*4][ECX*4][EDX*4][EBX*4][XXX*4][EBP*4][ESI*4][EDI*4]" 7 SS. ;
: [REG*8]  ( I -- )     S" [EAX*8][ECX*8][EDX*8][EBX*8][XXX*8][EBP*8][ESI*8][EDI*8]" 7 SS. ;
: [INDEX]  ( SIB -- )   MOD/SIB OVER 4 =
                        IF    2DROP                     \ NO ESP SCALED INDEX
                        ELSE  CASE ( S )
                                0 OF [REG32] ENDOF
                                1 OF [REG*2] ENDOF
                                2 OF [REG*4] ENDOF
                                3 OF [REG*8] ENDOF
                              ENDCASE
                        THEN DROP ;

: DISP8  ( ADR -- ADR' )  COUNT H.>S ;
: DISP16 ( ADR -- ADR' )  W@+ SHOW-NAME ;
: DISP32 ( ADR -- ADR' ) @+ ( BODY> ) SHOW-NAME ;
: DISP16/32 ( ADR -- ADR' )
            16-BIT-ADDR
            IF   DISP16
            ELSE DISP32
            THEN ;

: .,     ( -- )           .S" , " ;

: .#  ., .S" # " ;

: IMM8   ( ADR -- ADR' )  .# COUNT H.>S ;

\ : IMM16  ( ADR -- ADR' )  .# W@+ H.>S ;

: IMM16/32  ( ADR -- ADR' )
        .# 16-BIT-DATA
        IF   W@+
        ELSE @+
        THEN H.>S ;

: SIB   ( ADR MOD -- ADR )
        >R COUNT TUCK 7 AND 5 = R@ 0= AND
        IF    DISP32 SWAP [INDEX] R> DROP       \ EBP BASE AND MOD = 00
        ELSE  R> CASE ( MOD )
                   1 OF DISP8  ENDOF
                   2 OF DISP32 ENDOF
                 ENDCASE
              SWAP DUP [REG32] [INDEX]
        THEN ;


: MOD-R/M32     ( ADR R/M MOD -- ADR' )
                DUP 3 =
                IF    DROP  REG                         \ MOD = 3, REGISTER CASE
                ELSE  OVER 4 =
                      IF NIP SIB                        \ R/M = 4, SIB CASE
                      ELSE  2DUP 0= SWAP 5 = AND        \ MOD = 0, R/M = 5,
                            IF 2DROP DISP32             \ DISP32 CASE
                            ELSE ROT SWAP
                                 CASE ( MOD )
                                   1 OF DISP8  ENDOF
                                   2 OF DISP32 ENDOF
                                 ENDCASE
                                 SWAP [REG32]
                            THEN
                      THEN
                THEN ;

: MOD-R/M16     ( ADR R/M MOD -- ADR' )
                2DUP 0= SWAP 6 = AND
                IF   2DROP DISP16                       \ DISP16 CASE
                ELSE CASE ( MOD )
                       0 OF [REG16]                     ENDOF
                       1 OF SWAP DISP8  SWAP [REG16]    ENDOF
                       2 OF SWAP DISP16 SWAP [REG16]    ENDOF
                       3 OF REG                         ENDOF
                     ENDCASE
                THEN ;

: MOD-R/M ( ADR MODR/M -- ADR' )
          MOD/SIB NIP 16-BIT-ADDR
          IF    MOD-R/M16
          ELSE  MOD-R/M32
          THEN ;


: R/M8      0 TO SIZE MOD-R/M ;
: R/M16/32  1 TO SIZE MOD-R/M ;
: R/M16     TRUE TO 16-BIT-DATA R/M16/32 ;

: R,R/M  ( ADR -- ADR' )
        COUNT DUP 3 RSHIFT REG .,  MOD-R/M ;

: R/M,R  ( ADR -- ADR' )
        COUNT DUP>R MOD-R/M ., R> 3 RSHIFT REG ;

: R/M  ( ADR OP -- ADR' )
        2 AND
        IF     R,R/M
        ELSE   R/M,R
        THEN  ;

\ -------------------- SIMPLE OPCODES --------------------

: INH   ( -<NAME>- )
        CREATE
        BL WORD COUNT HERE PLACE
        HERE C@ 1+ ALLOT
        DOES> COUNT >S SSPACE DROP ;

INH CLC  CLC
INH STC  STC
INH CLD  CLD
INH STD  STD
\ INH RPNZ REPNZ
\ INH REPZ REPZ
INH CBW  CBW
INH CDQ  CDQ
INH DAA  DAA
INH DAS  DAS
INH AAA  AAA
INH AAS  AAS
\ INH LOCK LOCK
INH INB  INSB
INH OSB  OUTSB
INH SAH  SAHF
INH LAH  LAHF
\ INH AAM  AAM
\ INH AAD  AAD
INH HLT  HLT
INH CMC  CMC
INH XLT  XLAT
INH CLI  CLI
INH STI  STI

INH CLT CLTS
INH INV INVD
INH WIV WBINVD
INH UD2 UD2
INH WMR WRMSR
INH RTC RDTSC
INH RMR RDMSR
INH RPC RDPMC
INH EMS EMMS
INH RSM RSM
INH CPU CPUID
INH UD1 UD1
\ INH LSS LSS
\ INH LFS LFS
\ INH LGS LGS

\ INH D16: D16:
\ INH A16: A16:
\ INH ES:  ES:
\ INH CS:  CS:
\ INH DS:  DS:
\ INH FS:  FS:
\ INH GS:  GS:

: AAM   ( ADR CODE -- ADR' )
        .S" AAM" DROP COUNT DROP ;

: AAD   ( ADR CODE -- ADR' )
        .S" AAD" DROP COUNT DROP ;

: D16   ( ADR CODE -- ADR' )
        DROP .S" D16:"
        TRUE TO 16-BIT-DATA
        TRUE TO PREFIX-OP
        ;

: A16   ( ADR CODE -- ADR' )
        DROP .S" A16:"
        TRUE TO 16-BIT-ADDR
        TRUE TO PREFIX-OP
        ;

: RPZ   ( ADR CODE -- ADR' )
        DROP .S" REPNZ"
        TRUE TO PREFIX-OP
        ;

: REP   ( ADR CODE -- ADR' )
        DROP .S" REPZ"
        TRUE TO PREFIX-OP
        ;

: LOK   ( ADR CODE -- ADR' )  \ THIS SHOULD HAVE ERROR CHECKING ADDED
        DROP .S" LOCK"
        TRUE TO PREFIX-OP
        ;

: CS:   ( ADR CODE -- ADR' )
        DROP .S" CS:"
        TRUE TO PREFIX-OP
        ;

: DS:   ( ADR CODE -- ADR' )
        DROP .S" DS:"
        TRUE TO PREFIX-OP
        ;

: SS:   ( ADR CODE -- ADR' )
        DROP .S" SS:"
        TRUE TO PREFIX-OP
        ;

: ES:   ( ADR CODE -- ADR' )
        DROP .S" ES:"
        TRUE TO PREFIX-OP
        ;

: GS:   ( ADR CODE -- ADR' )
        DROP .S" GS:"
        TRUE TO PREFIX-OP
        ;

: FS:   ( ADR CODE -- ADR' )
        DROP .S" FS:"
        TRUE TO PREFIX-OP
        ;

: ISD   ( ADR CODE -- ADR' )
        DROP 16-BIT-DATA
        IF      .S" INSW    "
        ELSE    .S" INSD    "
        THEN ;

: OSD   ( ADR CODE -- ADR' )
        DROP 16-BIT-DATA
        IF      .S" OUTSW    "
        ELSE    .S" OUTSD    "
        THEN ;

: INP   ( ADDR CODE -- ADDR' )
        .S" IN      " 1 AND
        IF      16-BIT-DATA
                IF      .S" AX , "
                ELSE    .S" EAX , "
                THEN
        ELSE    .S" AL , "
        THEN
        COUNT H.>S ;

: OTP   ( ADDR CODE -- ADDR' )
        .S" OUT     " 1 AND
        IF      COUNT H.>S 16-BIT-DATA
                IF      .S" , AX"
                ELSE    .S" , EAX"
                THEN
        ELSE    COUNT H.>S .S" , AL"
        THEN
        ;

: IND   ( ADDR CODE -- ADDR' )
        .S" IN      " 1 AND
        IF      16-BIT-DATA
                IF      .S" AX , DX"
                ELSE    .S" EAX , DX"
                THEN
        ELSE    .S" AL , DX"
        THEN
        ;

: OTD   ( ADDR CODE -- ADDR' )
        .S" OUT     " 1 AND
        IF      16-BIT-DATA
                IF      .S" DX , AX"
                ELSE    .S" DX , EAX"
                THEN
        ELSE    .S" DX , AL"
        THEN
        ;

\ -------------------- ALU OPCODES --------------------

: .ALU  ( N -- )
        7 AND S" ADDOR ADCSBBANDSUBXORCMP"  3 SS. 4 SSPACES
    ;

: ALU  ( ADR OP -- ADR' )
        DUP 3 RSHIFT .ALU R/M ;

: ALI ( ADR OP -- ADR' )
        >R COUNT
        DUP 3 RSHIFT .ALU
        MOD-R/M
        R> 3 AND ?DUP
        IF      1 =
                IF      IMM16/32
                ELSE    .# COUNT SEXT 0 .R>S SSPACE
                THEN
        ELSE    IMM8
        THEN ;

: ALA  ( ADR OP -- ADR' )
        DUP 3 RSHIFT .ALU
        1 AND IF 0 REG IMM16/32 ELSE 0 REG8 IMM8 THEN ;


\ -------------------- TEST/XCHG --------------------

: TXB   ( ADDR OP -- ADDR' )
        DUP 3 AND S" TESTTESTXCHGXCHG" 4 SS. 3 SSPACES
        1 AND
        IF      1 TO SIZE R,R/M     \ SMUB REMOVED COUNT
        ELSE    0 TO SIZE R,R/M     \ SMUB REMOVED COUNT
        THEN
        ;

: TST   ( ADDR OP -- ADDR' )
        .S" TEST    " 1 AND
        IF      16-BIT-DATA
                IF   .S" AX , "
                ELSE .S" EAX , "
                THEN
                IMM16/32
        ELSE    .S" AL , " IMM8
        THEN
        ;

\ -------------------- INC/DEC ----------------------

: INC  ( ADDR OP -- ADDR' )
        .S" INC     " REG16/32 ;

: DEC  ( ADDR OP -- ADDR' )
        .S" DEC     " REG16/32 ;


\ -------------------- PUSH/POP --------------------

: PSH   ( ADDR OP -- ADDR' )
        .S" PUSH    " REG16/32 ;

: POP   ( ADDR OP -- ADDR' )
        .S" POP     " REG16/32 ;

: PSS   ( ADDR OP -- ADDR' )
        .S" PUSH    " SREG ;

: PPS   ( ADDR OP -- ADDR' )
        .S" POP     " SREG ;

: PSA   ( ADDR OP -- ADDR' )
        DROP 16-BIT-DATA
        IF      .S" PUSHA   "
        ELSE    .S" PUSHAD  "
        THEN ;

: PPA   ( ADDR OP -- ADDR' )
        DROP 16-BIT-DATA
        IF      .S" POPA    "
        ELSE    .S" POPAD   "
        THEN ;

: PSI   ( ADDR OP -- ADDR' )
        .S" PUSH    " 2 AND
        IF      IMM8
        ELSE    IMM16/32
        THEN ;

: PSF   ( ADDR OP -- ADDR' )
        DROP 16-BIT-DATA
        IF      .S" PUSHF   "
        ELSE    .S" PUSHFD  "
        THEN ;

: PPF   ( ADDR OP -- ADDR' )
        DROP 16-BIT-DATA
        IF      .S" POPF    "
        ELSE    .S" POPFD   "
        THEN ;

: 8F.   ( ADDR OP -- ADDR' )
        DROP COUNT .S" POP     " R/M16/32 ;

\ -------------------- MOVE --------------------

: MOV  ( ADDR OP -- ADDR' )
        .S" MOV     " R/M ;

: MRI  ( ADDR OP -- ADDR' ) ( MOV REGISTER, IMM )
        .S" MOV     " DUP 8 AND
        IF      REG16/32 IMM16/32
        ELSE    REG8 IMM8
        THEN ;

: MVI  ( ADR OP -- ADR' )   ( MOV MEM, IMM )
        .S" MOV     " DROP COUNT MOD-R/M
        SIZE
        IF      IMM16/32
        ELSE    IMM8
        THEN
        ;

: MRS   ( ADDR OP -- ADDR' )
\ ? REMOVE REDUNDANT >R , R>
        16-BIT-DATA
        IF      .S" MOV     " DROP
                1 TO SIZE
                COUNT DUP MOD-R/M .,
                SREG
        ELSE    ???
        THEN ;

: MSR   ( ADDR OP -- ADDR' )
        16-BIT-DATA
        IF      .S" MOV     " DROP
                1 TO SIZE
                COUNT DUP SREG .,
                MOD-R/M
        ELSE    ???
        THEN ;

: MRC   ( ADDR OP -- ADDR' )
        .S" MOV     "
        DROP COUNT DUP REG32 .S" , "
        CREG ;

: MCR   ( ADDR OP -- ADDR' )
        .S" MOV     "
        DROP COUNT DUP CREG .S" , "
        REG32 ;

: MRD   ( ADDR OP -- ADDR' )
        .S" MOV     "
        DROP COUNT DUP REG32 .S" , "
        DREG ;

: MDR   ( ADDR OP -- ADDR' )
        .S" MOV     "
        DROP COUNT DUP DREG .S" , "
        REG32 ;

: MRT   ( ADDR OP -- ADDR' )
\ OBSOLETE
        .S" MOV     "
        DROP COUNT DUP REG32 .S" , "
        TREG ;

: MTR   ( ADDR OP -- ADDR' )
\ OBSOLETE
        .S" MOV     "
        DROP COUNT DUP TREG .S" , "
        REG32 ;

: MV1   ( ADDR OP -- ADDR' )
        .S" MOV     " 1 AND
        IF      16-BIT-DATA
                IF      .S" AX , "
                ELSE    .S" EAX , "
                THEN
        ELSE    .S" AL , "
        THEN
        DISP16/32 ;

: MV2   ( ADDR OP -- ADDR' )
        .S" MOV     " SWAP DISP16/32 .,
        SWAP 1 AND
        IF      16-BIT-DATA
                IF      .S"  AX"
                ELSE    .S"  EAX"
                THEN
        ELSE    .S"  AL"
        THEN
        ;

: LEA  ( ADDR OP -- ADDR' )
        .S" LEA     " DROP  1 TO SIZE R,R/M ;

: LXS   ( ADDR OP -- ADDR' )
        1 AND
        IF      .S" LDS     "
        ELSE    .S" LES     "
        THEN
        R,R/M   \ SMUB REMOVED COUNT
        ;

: BND  ( ADDR OP -- ADDR' )
        .S" BOUND   " DROP  1 TO SIZE R,R/M ;

: ARP   ( ADDR OP -- ADDR' )
        .S" ARPL    " DROP
        1 TO SIZE
        TRUE TO 16-BIT-DATA
        R,R/M
        ;

: MLI   ( ADDR OP -- ADDR' )
        1 TO SIZE
        .S" IMUL    " 0x69 =
        IF      R,R/M IMM16/32
        ELSE    R,R/M IMM8
        THEN ;

\ -------------------- JUMPS AND CALLS --------------------

0 VALUE MAX_REFERENCE

: >MAX_R  DUP MAX_REFERENCE UMAX TO MAX_REFERENCE ;

: REL8  ( ADDR OP -- ADDR' )
        COUNT SEXT OVER + BASE-ADDR - >MAX_R H.>S ;



: REL16/32 ( ADDR OP -- ADDR' )
        16-BIT-ADDR
        IF      W@+
        ELSE    @+
        THEN    OVER + BASE-ADDR - >MAX_R SHOW-NAME ;

: JSR  ( ADDR OP -- ADDR' )
        .S" CALL    " DROP REL16/32 ;

: JMP  ( ADDR OP -- ADDR' )
        .S" JMP     " 2 AND IF REL8 ELSE REL16/32 THEN ;

: .JXX  ( ADDR OP -- ADDR' )
        .S" J" TTTN 4 SSPACES ;

: BRA  ( ADDR OP -- ADDR' )
        .JXX REL8 ;

: LUP  ( ADDR OP -- ADDR' )
        3 AND S" LOOPNZLOOPZ LOOP  JECXZ " 6 SS. 1 SSPACES REL8 ;

: LBR  ( ADDR OP -- ADDR' )
        .JXX REL16/32 ;

: RTN  ( ADDR OP -- ADDR' )
        .S" RET     NEAR " 1 AND 0=
        IF      W@+ H.>S
        THEN ;

: RTF  ( ADDR OP -- ADDR' )
        .S" RET     FAR " 1 AND 0=
        IF      W@+ H.>S
        THEN ;

: ENT  ( ADDR OP -- ADDR' )
       DROP
        .S" ENTER   " W@+ H.>S ., COUNT H.>S ;

: CIS   ( ADDR OP -- ADDR' )
        0x9A =
        IF      .S" CALL    "
        ELSE    .S" JMP     "
        THEN
        16-BIT-DATA
        IF      .S" PTR16:16 "
        ELSE    .S" PTR16:32 "
        THEN
        COUNT MOD-R/M ;

: NT3   ( ADDR OP -- ADDR' )
        DROP .S" INT     3 "
        ;

: INT   ( ADDR OP -- ADDR' )
        DROP .S" INT     "
        COUNT H.>S ;

INH LEV LEAVE
INH IRT  IRET
INH NTO  INTO

\ -------------------- STRING OPS --------------------

: STR   INH DOES> COUNT >S  1 AND IF .S" D" ELSE .S" B" THEN ;

STR MVS MOVS
STR CPS CMPS
STR STS STOS
STR LDS LODS
STR SCS SCAS

\ -------------------- EXCHANGE --------------------

: XGA  ( ADDR OP -- ADDR' )
        .S" XCHG     EAX, " REG16/32 ;

\ : XCH  ( ADDR OP -- ADDR' )
\       .S" XCHG    " DROP R,R/M ;


\ -------------------- SHIFTS & ROTATES --------------------

: .SHIFT ( N -- )
        7 AND S" ROLRORRCLRCRSHLSHRXXXSAR" 3 SS.  4 SSPACES ;

: SHF  ( ADDR OP -- ADDR' )
        >R COUNT
        DUP 3 RSHIFT .SHIFT
        MOD-R/M .,
        R> 0xD2 AND
        CASE
           0xC0 OF COUNT H.>S      ENDOF
           0xD0 OF 1 H.>S          ENDOF
           0xD2 OF 1 REG8          ENDOF
        ENDCASE ;

\ -------------------- EXTENDED OPCODES --------------------

: WF1  ( ADDR -- ADDR' )
        1+ COUNT DUP
        0x0C0 <
        IF      DUP
                3 RSHIFT 7 AND
                CASE 6 OF     .S" FSTENV  "      MOD-R/M   ENDOF
                     7 OF     .S" FSTCW   WORD " MOD-R/M   ENDOF
                     2DROP 2 - DUP .S" FWAIT   "
                ENDCASE
        ELSE    DROP 2 - .S" FWAIT   "
        THEN ;

: WF2  ( ADDR -- ADDR' )
        1+ COUNT
        CASE 0xE2 OF   .S" FCLEX   "  ENDOF
             0xE3 OF   .S" FINIT   "  ENDOF
             SWAP 2 - SWAP .S" FWAIT   "
        ENDCASE ;

: WF3  ( ADDR -- ADDR' )
        1+ COUNT DUP 3 RSHIFT 7 AND
        CASE 6 OF     .S" FSAVE   "      MOD-R/M   ENDOF
             7 OF     .S" FSTSW   WORD " MOD-R/M   ENDOF
             2DROP 2 - DUP .S" FWAIT   "
        ENDCASE ;

: WF4  ( ADDR -- ADDR' )
        1+ COUNT 0xE0 =
        IF      .S" FSTSW   AX "
        ELSE    2 - .S" FWAIT   "
        THEN ;

: FWAITOPS   ( ADDR OP -- ADDR' )
        CASE 0xD9 OF    WF1     ENDOF
             0xDB OF    WF2     ENDOF
             0xDD OF    WF3     ENDOF
             0xDF OF    WF4     ENDOF
             .S" FWAIT   "
        ENDCASE ;

: W8F   ( ADDR OP -- ADDR' )
        DROP DUP C@ DUP 0xF8 AND 0xD8 =
        IF      FWAITOPS
        ELSE    DROP .S" WAIT    "
        THEN ;

: FALU1   ( XOPCODE -- )
        3 RSHIFT 7 AND
        S" FADD FMUL FCOM FCOMPFSUB FSUBRFDIV FDIVR"
        5 SS. 2 SSPACES ;

: FALU5   ( XOPCODE -- )
        3 RSHIFT 7 AND
        S" FADD FMUL ???? ???? FSUBRFSUB FDIVRFDIV "
        5 SS. 2 SSPACES ;

: STI.   ( OP -- )
        7 AND .S" ST(" 1 .R>S .S" )";

\ : STI.ST   ( OP -- )
\        7 AND
\        .S" ST(" 1 .R>S .S" )" .S"  ST " ;

: FD8   ( ADDR OPCODE -- ADDR' )
        DROP COUNT DUP FALU1
        DUP 0xC0 <
        IF      .S" FLOAT " MOD-R/M
        ELSE    DUP 0xF0 AND 0xD0 =
                IF      STI.
                ELSE    .S" ST , " STI.
                THEN
        THEN ;

: FDC   ( ADDR OPCODE -- ADDR' )
        DROP COUNT
        DUP DUP 0xC0 <
        IF      FALU1 .S" DOUBLE " MOD-R/M
        ELSE    FALU5 STI. .S"  , ST"
        THEN ;

: FNULLARY-F   ( OP -- )
        0x0F AND DUP 8 <
        IF
           S" F2XM1  FYL2X  FPTAN  FPATAN FXTRACTFPREM1 FDECSTPFINCSTP"
        ELSE  8 -
           S" FPREM  FYL2XP1FSQRT  FSINCOSFRNDINTFSCALE FSIN   FCOS   "
        THEN
        7 SS. ;

: FNULLARY-E   ( OP -- )
        0x0F AND DUP 8 <
        IF
           S" FCHS   FABS   ???    ???    FTST   FXAM   ???    ???    "
        ELSE  8 -
           S" FLD1   FLDL2T FLDL2E FLDPI  FLDLG2 FLDLN2 FLDZ   ???    "
        THEN
        7 SS. ;

: FNULLARY   ( OP -- )
        DUP 0xEF >
        IF      FNULLARY-F EXIT
        THEN
        DUP 0xE0 <
        IF      0xD0 =
                IF      .S" FNOP"
                ELSE    DUP ???
                THEN
                EXIT
        THEN
        FNULLARY-E ;


\ : FALU2   ( OP -- )
\        3 RSHIFT 7 AND
\        S" FLD    ???    FST    FSTP   FLDENV FLDCW  FNSTENVFNSTCW "
\        7 SS. ;

: FD9   ( ADDR OP -- ADDR' )
        DROP COUNT DUP 0xC0 <
        IF      DUP 0x38 AND
                CASE
                        0x00 OF .S" FLD     FLOAT "  ENDOF
                        0x10 OF .S" FST     FLOAT "  ENDOF
                        0x18 OF .S" FSTP    FLOAT "  ENDOF
                        0x20 OF .S" FLDENV  "        ENDOF
                        0x28 OF .S" FLDCW   WORD "   ENDOF
                        0x30 OF .S" FNSTENV "        ENDOF
                        0x38 OF .S" FNSTCW  WORD "   ENDOF
                            DUP ???
                ENDCASE
                MOD-R/M
        ELSE
                DUP 0xD0 <
                IF      DUP 0xC8 <
                        IF      .S" FLD     "
                        ELSE    .S" FXCH    "
                        THEN
                        STI.
                ELSE    FNULLARY
                THEN
        THEN ;

: FALU3   ( OP -- )
        3 RSHIFT 7 AND
        S" FIADD FIMUL FICOM FICOMPFISUB FISUBRFIDIV FIDIVR"
        6 SS. 1 SSPACES ;

: FCMOVA  ( OP -- )
        3 RSHIFT 7 AND
        S" FCMOVB FCMOVE FCMOVBEFCMOVU ???    ???    ???    ???    "
        7 SS. ;

: FDA   ( ADDR OP -- )
        DROP COUNT DUP 0xC0 <
        IF      DUP FALU3 .S" DWORD " MOD-R/M
        ELSE    DUP 0xE9 =
                IF      .S" FUCOMPP" DROP
                ELSE    DUP FCMOVA STI.
                THEN
        THEN ;

: FALU7  ( OP -- )
        3 RSHIFT 7 AND
        S" FADDP FMULP ???   ???   FSUBRPFSUBP FDIVRPFDIVP "
        6 SS. SSPACE ;

: FDE   ( ADDR OP -- ADDR' )
        DROP COUNT DUP 0xC0 <
        IF      DUP FALU3 .S" WORD " MOD-R/M
        ELSE    DUP 0xD9 =
                IF    .S" FCOMPP" DROP
                ELSE  DUP FALU7 STI.
                THEN
        THEN ;


: FCMOVB  ( OP -- )
        3 RSHIFT 7 AND
        S" FCMOVNB FCMOVNE FCMOVNBEFCMOVNU ???     FUCOMI  FCOMI   ???     "
        8 SS. ;

: FDB   ( ADDR OP -- ADDR' )
        DROP COUNT DUP 0xC0 <
        IF      DUP 0x38 AND
                CASE    0x00 OF .S" FILD    DWORD "    ENDOF
                        0x10 OF .S" FIST    DWORD "    ENDOF
                        0x18 OF .S" FISTP   DWORD "    ENDOF
                        0x28 OF .S" FLD     EXTENDED " ENDOF
                        0x38 OF .S" FSTP    EXTENDED " ENDOF
                            DUP ???
                ENDCASE
                MOD-R/M
        ELSE
                CASE    0xE2 OF .S" FNCLEX" ENDOF
                        0xE3 OF .S" FNINIT" ENDOF
                            DUP DUP FCMOVB STI.
                ENDCASE
        THEN ;

: FALU6  ( OP -- )
        3 RSHIFT 7 AND
        S" FFREE ???   FST   FSTP  FUCOM FUCOMP???   ???   "
        6 SS. SSPACE ;

: FDD   ( ADDR OP -- ADDR' )
        DROP COUNT DUP 0xC0 <
        IF      DUP 0x38 AND
                CASE    0x00 OF .S" FLD     DOUBLE "  ENDOF
                        0x10 OF .S" FST     DOUBLE "  ENDOF
                        0x18 OF .S" FSTP    DOUBLE "  ENDOF
                        0x20 OF .S" FRSTOR  "         ENDOF
                        0x30 OF .S" FNSAVE  "         ENDOF
                        0x38 OF .S" FNSTSW  WORD   "  ENDOF
                            DUP ???
                ENDCASE
                MOD-R/M
        ELSE    DUP FALU6 STI.
        THEN ;

: FDF   ( ADDR OP -- ADDR' )
        DROP COUNT DUP 0xC0 <
        IF      DUP 0x38 AND
                CASE    0x00 OF .S" FILD    WORD "   ENDOF
                        0x10 OF .S" FIST    WORD "   ENDOF
                        0x18 OF .S" FISTP   WORD "   ENDOF
                        0x20 OF .S" FBLD    TBYTE "  ENDOF
                        0x28 OF .S" FILD    QWORD "  ENDOF
                        0x30 OF .S" FBSTP   TBYTE "  ENDOF
                        0x38 OF .S" FISTP   QWORD "  ENDOF
                            DUP ???
                ENDCASE
                MOD-R/M
        ELSE    DUP 0xE0 =
                IF      .S" FNSTSW  AX " DROP
                ELSE    DUP 0x38 AND
                        CASE    0x28 OF .S" FUCOMIP " STI. ENDOF
                                0x30 OF .S" FCOMIP  " STI. ENDOF
                                        ???
                        ENDCASE
                THEN
        THEN ;

: GP6 ( ADDR OP -- ADDR' )
        DROP COUNT DUP 3 RSHIFT
        7 AND S" SLDTSTR LLDTLTR VERRVERW??? ???" 4 SS. 3 SSPACES
        R/M16 ;

: GP7 ( ADDR OP -- ADDR' )
        DROP COUNT DUP 3 RSHIFT
        7 AND DUP S" SGDT  SIDT  LGDT  LIDT  SMSW  ???   LMSW  INVLPG" 6 SS. 1 SSPACES
        4 AND 4 =
        IF   R/M16
        ELSE R/M16/32
        THEN ;

: BTX.  ( N -- )
        3 RSHIFT
        3 AND S" BT BTSBTRBTC" 3 SS. 4 SSPACES ;

: GP8 ( ADDR OP -- ADDR' )
        DROP COUNT DUP BTX.
        R/M16/32 IMM8 ;

: LAR ( ADDR OP -- ADDR' )
        .S" LAR     " DROP R,R/M ;

: LSL ( ADDR OP -- ADDR' )
        .S" LSL     " DROP R,R/M ;

: LSS ( ADDR OP -- ADDR' )
        .S" LSS     " DROP R,R/M ;

: LFS ( ADDR OP -- ADDR' )
        .S" LFS     " DROP R,R/M ;

: LGS ( ADDR OP -- ADDR' )
        .S" LGS     " DROP R,R/M ;

: BTX ( ADDR OP -- ADDR' )
        BTX. R/M,R ;

: SLI ( ADDR OP -- ADDR' )
        .S" SHLD    " DROP R/M,R IMM8 ;

: SRI ( ADDR OP -- ADDR' )
        .S" SHRD    " DROP R/M,R IMM8 ;

: SLC ( ADDR OP -- ADDR' )
        .S" SHLD    " DROP R/M,R .S" , CL" ;

: SRC ( ADDR OP -- ADDR' )
        .S" SHRD    " DROP R/M,R .S" , CL" ;

: IML ( ADDR OP -- ADDR' )
        .S" IMUL    " DROP R,R/M ;

: CXC ( ADDR OP -- ADDR' )
        .S" CMPXCHG " 1 AND TO SIZE R/M,R ;

: MVX ( ADDR OP -- ADDR' )
        DUP 8 AND
        IF      .S" MOVSX   "
        ELSE    .S" MOVZX   "
        THEN
        1 AND >R
        COUNT MOD/SIB R>                        \ SIZE BIT
        IF    SWAP REG32 .,                     \ WORD TO DWORD CASE
              3 =
              IF   REG16
              ELSE .S" WORD PTR "  DROP DUP 1- C@ MOD-R/M
              THEN
        ELSE  SWAP REG16/32 .,                  \ BYTE CASE
              3 =
              IF   REG8
              ELSE .S" BYTE PTR "  DROP DUP 1- C@ MOD-R/M
              THEN
        THEN ;

: XAD ( ADDR OP -- ADDR' )
        .S" XADD    " 1 AND TO SIZE R/M,R ;

: BSF ( ADDR OP -- ADDR' )
        .S" BSF     " DROP R,R/M ;

: BSR ( ADDR OP -- ADDR' )
        .S" BSR     " DROP R,R/M ;

: CX8 ( ADDR OP -- ADDR' )
        .S" CMPXCHG8B " DROP COUNT R/M16/32 ;

: BSP ( ADDR OP -- ADDR' )
        .S" BSWAP   " REG32 ;


: F6.  ( ADDR OP -- ADDR' )
\ ??
        >R COUNT
        DUP 3 RSHIFT 7 AND DUP>R S" TESTXXXXNOT NEG MUL IMULDIV IDIV" 4 SS. 3 SSPACES
        MOD-R/M
        R> 0= IF
                R@ 1 AND IF IMM16/32
                         ELSE IMM8
                         THEN
              THEN
        R> DROP ;

: FE.  ( ADDR OP -- ADDR' )
        DROP COUNT
        DUP 3 RSHIFT 7 AND
        CASE
                0 OF .S" INC     "  ENDOF
                1 OF .S" DEC     "  ENDOF
                     .S" ???     "
        ENDCASE R/M8 ;

: FF.  ( ADDR OP -- ADDR' )
        DROP COUNT
        DUP 3 RSHIFT 7 AND
        CASE
                0 OF .S" INC     "      ENDOF
                1 OF .S" DEC     "      ENDOF
                2 OF .S" CALL    "      ENDOF
                3 OF .S" CALL    FAR "  ENDOF
                4 OF .S" JMP     "      ENDOF
                5 OF .S" JMP     FAR "  ENDOF
                6 OF .S" PUSH    "      ENDOF
                     .S" ???     "
        ENDCASE R/M16/32 ;


\ --------------------- CONDITIONAL MOVE ---------------

: SET   ( ADR OP -- )
        .S" SET"
        TTTN 2 SSPACES
        COUNT R/M8 ;

: CMV   ( ADR OP -- )
        .S" CMOV"
        TTTN 1 SSPACES
        R,R/M ;

\ --------------------- MMX OPERATIONS -----------------

: MMX-SIZE ( OP -- )
        3 AND S" BWDQ" 1 SS. ;

: UPL   ( ADR OP -- ADR' )
        3 AND S" PUNPCKLBWPUNPCKLWDPUNPCKLDQ" 9 SS. R,R/M ;

: UPH   ( ADR OP -- ADR' )
        3 AND S" PUNPCKHBWPUNPCKHWDPUNPCKHDQ" 9 SS. R,R/M ;

: CGT   ( ADR OP -- ADR' )
        .S" PCMPGT" MMX-SIZE R,R/M ;

: CEQ   ( ADR OP -- ADR' )
        .S" PCMPEQ" MMX-SIZE R,R/M ;

: PSH.  ( OP -- )
        0x30 AND
        CASE
             0x10 OF .S" PSRL" ENDOF
             0x20 OF .S" PSRA" ENDOF
             0x30 OF .S" PSLL" ENDOF
        ENDCASE ;

: GPA   ( ADR OP -- ADR' )
        >R COUNT DUP PSH. R> MMX-SIZE 2 SSPACES MREG IMM8 ;

: PUW   ( ADR OP -- ADR' )
        .S" PACKUSDW " DROP R,R/M ;

: PSB   ( ADR OP -- ADR' )
        .S" PACKSSWB " DROP R,R/M ;

: PSW   ( ADR OP -- ADR' )
        .S" PACKSSDW " DROP R,R/M ;

: MPD   ( ADR OP -- ADR' )
        .S" MOVD    " DROP COUNT MOD/SIB
        SWAP MREG ., 3 =
        IF   REG32
        ELSE MOD-R/M
        THEN ;

: MDP   ( ADR OP -- ADR' )
        .S" MOVD    " DROP COUNT MOD/SIB
        3 =
        IF   SWAP REG32
        ELSE SWAP MOD-R/M
        THEN ., MREG ;

: MPQ   ( ADR OP -- ADR' )
        .S" MOVQ    " DROP R,R/M ;

: MQP   ( ADR OP -- ADR' )
        .S" MOVQ    " DROP R/M,R ;

: SHX   ( ADR OP -- ADR' )
        DUP PSH. MMX-SIZE 2 SSPACES R,R/M ;

: MLL   ( ADR OP -- ADR' )
        .S" PMULLW  " DROP R,R/M ;

: MLH   ( ADR OP -- ADR' )
        .S" PMULHW  " DROP R,R/M ;

: MAD   ( ADR OP -- ADR' )
        .S" PMADDWD " DROP R,R/M ;

: SUS   ( ADR OP -- ADR' )
        .S" PSUBUS" MMX-SIZE R,R/M ;

: SBS   ( ADR OP -- ADR' )
        .S" PSUBS" MMX-SIZE SSPACE R,R/M ;

: SUB   ( ADR OP -- ADR' )
        .S" PSUB" MMX-SIZE 2 SSPACES R,R/M ;

: AUS   ( ADR OP -- ADR' )
        .S" PADDUS" MMX-SIZE R,R/M ;

: ADS   ( ADR OP -- ADR' )
        .S" PADDS" MMX-SIZE SSPACE R,R/M ;

: ADD   ( ADR OP -- ADR' )
        .S" PADD" MMX-SIZE 2 SSPACES R,R/M ;

: PAD   ( ADR OP -- ADR' )
        .S" PAND    " DROP R,R/M ;

: POR   ( ADR OP -- ADR' )
        .S" POR     " DROP R,R/M ;

: PAN   ( ADR OP -- ADR' )
        .S" PANDN   " DROP R,R/M ;

: PXR   ( ADR OP -- ADR' )
        .S" PXOR    " DROP R,R/M ;


\ -------------------- OPCODE TABLE --------------------

: OPS 0x10 0 DO ' , LOOP ;


CREATE OP-TABLE2

\     0   1   2   3    4   5   6   7    8   9   A   B    C   D   E   F

OPS  GP6 GP7 LAR LSL  ??? ??? CLT ???  INV WIV ??? UD2  ??? ??? ??? ???  \ 0
OPS  ??? ??? ??? ???  ??? ??? ??? ???  ??? ??? ??? ???  ??? ??? ??? ???  \ 1
OPS  MRC MRD MCR MDR  MRT ??? MTR ???  ??? ??? ??? ???  ??? ??? ??? ???  \ 2
OPS  WMR RTC RMR RPC  ??? ??? ??? ???  ??? ??? ??? ???  ??? ??? ??? ???  \ 3

OPS  CMV CMV CMV CMV  CMV CMV CMV CMV  CMV CMV CMV CMV  CMV CMV CMV CMV  \ 4
OPS  ??? ??? ??? ???  ??? ??? ??? ???  ??? ??? ??? ???  ??? ??? ??? ???  \ 5
OPS  UPL UPL UPL PUW  CGT CGT CGT PSB  UPH UPH UPH PSW  ??? ??? MPD MPQ  \ 6
OPS  ??? GPA GPA GPA  CEQ CEQ CEQ EMS  ??? ??? ??? ???  ??? ??? MDP MQP  \ 7

OPS  LBR LBR LBR LBR  LBR LBR LBR LBR  LBR LBR LBR LBR  LBR LBR LBR LBR  \ 8
OPS  SET SET SET SET  SET SET SET SET  SET SET SET SET  SET SET SET SET  \ 9
OPS  PSS PPS CPU BTX  SLI SLC ??? ???  PSS PPS RSM BTX  SRI SRC ??? IML  \ A
OPS  CXC CXC LSS BTX  LFS LGS MVX MVX  ??? UD1 GP8 BTX  BSF BSR MVX MVX  \ B

OPS  XAD XAD ??? ???  ??? ??? ??? CX8  BSP BSP BSP BSP  BSP BSP BSP BSP  \ C
OPS  ??? SHX SHX SHX  ??? MLL ??? ???  SUS SUS ??? PAD  AUS AUS ??? PAN  \ D
OPS  ??? SHX SHX ???  ??? MLH ??? ???  SBS SBS ??? POR  ADS ADS ??? PXR  \ E
OPS  ??? ??? SHX SHX  ??? MAD ??? ???  SUB SUB SUB ???  ADD ADD ADD ???  \ F

\     0   1   2   3    4   5   6   7    8   9   A   B    C   D   E   F

: 0F.  ( ADR CODE -- )
        DROP COUNT DUP
        DUP 0x70 AND 0x50 0x80 WITHIN TO MMX-REG
        CELLS OP-TABLE2 + @ EXECUTE
        0 TO MMX-REG ;


CREATE OP-TABLE

\     0   1   2   3    4   5   6   7    8   9   A   B    C   D   E   F

OPS  ALU ALU ALU ALU  ALA ALA PSS PPS  ALU ALU ALU ALU  ALA ALA PSS 0F.  \ 0
OPS  ALU ALU ALU ALU  ALA ALA PSS PPS  ALU ALU ALU ALU  ALA ALA PSS PPS  \ 1
OPS  ALU ALU ALU ALU  ALA ALA ES: DAA  ALU ALU ALU ALU  ALA ALA CS: DAS  \ 2
OPS  ALU ALU ALU ALU  ALA ALA SS: AAA  ALU ALU ALU ALU  ALA ALA DS: AAS  \ 3

OPS  INC INC INC INC  INC INC INC INC  DEC DEC DEC DEC  DEC DEC DEC DEC  \ 4
OPS  PSH PSH PSH PSH  PSH PSH PSH PSH  POP POP POP POP  POP POP POP POP  \ 5
OPS  PSA PPA BND ARP  FS: GS: D16 A16  PSI MLI PSI MLI  INB ISD OSB OSD  \ 6
OPS  BRA BRA BRA BRA  BRA BRA BRA BRA  BRA BRA BRA BRA  BRA BRA BRA BRA  \ 7

OPS  ALI ALI ??? ALI  TXB TXB TXB TXB  MOV MOV MOV MOV  MRS LEA MSR 8F.  \ 8
OPS  XGA XGA XGA XGA  XGA XGA XGA XGA  CBW CDQ CIS W8F  PSF PPF SAH LAH  \ 9
OPS  MV1 MV1 MV2 MV2  MVS MVS CPS CPS  TST TST STS STS  LDS LDS SCS SCS  \ A
OPS  MRI MRI MRI MRI  MRI MRI MRI MRI  MRI MRI MRI MRI  MRI MRI MRI MRI  \ B

OPS  SHF SHF RTN RTN  LXS LXS MVI MVI  ENT LEV RTF RTF  NT3 INT NTO IRT  \ C
OPS  SHF SHF SHF SHF  AAM AAD ??? XLT  FD8 FD9 FDA FDB  FDC FDD FDE FDF  \ D
OPS  LUP LUP LUP LUP  INP INP OTP OTP  JSR JMP CIS JMP  IND IND OTD OTD  \ E
OPS  LOK ??? RPZ REP  HLT CMC F6. F6.  CLC STC CLI STI  CLD STD FE. FF.  \ F

\     0   1   2   3    4   5   6   7    8   9   A   B    C   D   E   F

: DIS-OP  ( ADR -- ADR' )
        0>S
        FALSE TO PREFIX-OP           \ SMUB
        COUNT
        DUP 1 AND TO SIZE
        DUP CELLS OP-TABLE + @ EXECUTE
        PREFIX-OP 0=
        IF DEFAULT-16BIT? 0=
           IF   FALSE TO 16-BIT-DATA
                FALSE TO 16-BIT-ADDR
           ELSE TRUE  TO 16-BIT-DATA
                TRUE  TO 16-BIT-ADDR
           THEN
        THEN ;


0 VALUE NEXT-INST

: X".  ( ADDR -- ADDR' )
\       CR DUP  BASE-ADDR - 6 H.R SPACE
        DUP C@ 2DUP DUMP
        + 2+
\       ."  C, " 1+ OVER + SWAP
\       DO I C@ 2 H.R  ."  C, " LOOP
\       COUNT  + 1+
;

[DEFINED] G. [IF]

: FLIT8.  ( ADDR -- ADDR' )
       ." FLITERAL: "
       DUP DF@ G.  8 +
;

: FLIT10.  ( ADDR -- ADDR' )
       ." FLITERAL: "
       DUP F@ G.  10 +
;

[ELSE]

: FLIT8.
       CR DUP  BASE-ADDR - 6 H.R SPACE
       ."  A; " DUP 8 OVER + SWAP
       DO I C@ 3 H.R ."  C," LOOP
       8 +
;

: FLIT10. ( ADDR -- ADDR' )
       CR DUP  BASE-ADDR - 6 H.R SPACE
       ."  A; "  DUP 10 OVER + SWAP
       DO I C@ 3 H.R ."  C," LOOP
       10 +
;

[THEN]

: VECT. ( ADDR -- ADDR' )
       CR DUP  BASE-ADDR - 6 H.R SPACE
       ."  A; " DUP @ 8 H.R DUP CELL+ SWAP @ ."  ,  \ " WordByAddr TYPE
;

: CONS. ( ADDR -- )
       CR DUP BASE-ADDR - 6 H.R SPACE
       ."  A; " @ 8 H.R ."  ,"
;

: USER. ( ADDR -- )
       CR DUP  BASE-ADDR - 6 H.R SPACE
       ."  A; " @ 8 H.R ."  , \ Relative in heap [hex]" \ CELL+
;

: UVAL. ( ADDR -- ADDR' )
       CR DUP  BASE-ADDR - 6 H.R SPACE
       ."  A; " DUP @ 8 H.R ."  , \ Relative in heap [hex]" CELL+
;

: CODE. ( ADDR -- )
        DUP NextNFA
        ?DUP
        IF OVER - 5 -
        ELSE
           DUP DP @ SWAP - ABS DUP 512 > IF DROP 40 THEN \ no applicable end found
        THEN
        ." Size of data: ~" DUP .
        DUMP
;


: DIS-DB   CR .S" DB " COUNT H.>S ;
: DIS-DW   CR .S" DW " W@+ H.>S ;
: DIS-DD   CR .S" DD " @+ H.>S ;
: DIS-DS   CR .S" STRING " 0x22 EMIT>S COUNT 2DUP >S + 0x22 EMIT>S ;

: FIND-REST-END ( xt -- addr | 0)
    DUP NextNFA DUP
    IF
      NIP
      NAME>C 1- \ Skip CFA field
    ELSE
      DROP
      DP @ - ABS 100 > IF 0 EXIT THEN \ no applicable end found
      DP @ 1-
    THEN

    BEGIN \ Skip alignment
      DUP C@ 0= WHILE 1-
    REPEAT ;

( wid ) SET-CURRENT

: INST  ( ADR -- ADR' )
        DUP TO NEXT-INST
        COLS 0x29 <
        IF      DIS-OP
                S-BUF COUNT TYPE
        ELSE    DUP DIS-OP
                OVER BASE-ADDR - 6  H.R SPACE
                DUP ROT
                2DUP - DUP>R 0x10 U> ABORT" DECOMPILER ERROR"
                DO I C@ 2 H.N LOOP
                R> 5 < IF 9 EMIT THEN
                9 EMIT S-BUF COUNT TYPE
        THEN    NEXT-INST C@ 0xE8 =
                IF  NEXT-INST 1+ @+ SWAP +
                    CASE
                   ['] _CLITERAL-CODE OF  X".   ENDOF
                   ['] _SLITERAL-CODE OF  X".   ENDOF
                   ['] _VECT-CODE     OF  VECT. 2DROP RDROP ENDOF
                   ['] _CONSTANT-CODE OF  CONS. DROP RDROP ENDOF
                   ['] _USER-CODE     OF  USER. DROP RDROP ENDOF
                   ['] _CREATE-CODE   OF  CODE. DROP RDROP ENDOF
                   ['] _USER-VALUE-CODE OF UVAL. ENDOF
                   ['] _FLIT-CODE10   OF  FLIT10. ENDOF
                   ['] _FLIT-CODE8    OF  FLIT8. ENDOF
                    ENDCASE
                THEN  ;


: (REST-AREA) ( addr1 addr2 -- )
\ if addr2 = 0 continue till RET instruction
                SWAP DUP TO NEXT-INST
                BEGIN
                        \ We do not look for JMP's because there may be
                         \ a jump in a forth word
                        CR
                        OVER 0= IF  NEXT-INST C@ 0xC3 <> 
                                ELSE 2DUP < INVERT
                                THEN
                WHILE   INST
                REPEAT  2DROP ." END-CODE  "
                ;

VECT REST-AREA                
' (REST-AREA) TO REST-AREA 

: REST ( addr -- )
    DUP HERE U> 0=  HERE 1- AND REST-AREA
;

: SEE       ( "name" -- )
    ' DUP FIND-REST-END ['] REST-AREA CATCH DROP
;

PREVIOUS

( warn base )
BASE !
WARNING !

.(  Ok) CR
