
REQUIRE [DEFINED] lib/include/tools.f

\ –аспечатать список словарей.
: VOCS
        VOC-LIST
        BEGIN @ DUP WHILE
                DUP CELL+ VOC-NAME.
                DUP 3 CELLS + @ \ wid предка
                ?DUP IF ."  defined in "  VOC-NAME.
                     ELSE ."  is the main vocabulary"
                     THEN CR
        REPEAT
        DROP
;

0x200 VALUE MAX-WORD-SIZE

C" NEAR_NFA" FIND NIP 0=
[IF] : NEAR_NFA ( addr -- NFA addr | 0 addr ) DUP  WordByAddr DROP 1- SWAP
        2DUP 1000 - U< IF NIP 0 SWAP THEN ;
[THEN]

\ Opposite to CDR, might be slow!
 \ It does not take wordlists into account.
: NextNFA ( nfa1 -- nfa2 | 0 )
    NEAR_NFA SWAP >R
    BEGIN
      1+ NEAR_NFA ( nfa addr )
      OVER 0 >
      ROT R@ <> AND
      OVER R@ - MAX-WORD-SIZE > OR
    UNTIL

    DUP R> - MAX-WORD-SIZE >
    IF DROP 0
    ELSE  NEAR_NFA DROP
    THEN
;

: NFAInVoc? ( nfa voc -- f )
    @ \ last nfa
    BEGIN  ( nfa 'nfa )
      DUP
    WHILE
      2DUP = IF 2DROP TRUE EXIT THEN
      CDR
    REPEAT 2DROP 0
;

: VocByNFA ( nfa -- wid | 0 )
    VOC-LIST
    BEGIN @ DUP WHILE ( nfa voc )
           2DUP CELL+ NFAInVoc?
           IF
              NIP CELL+ EXIT
           THEN
    REPEAT
    2DROP 0
;
