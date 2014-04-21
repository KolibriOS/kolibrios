
\ ~mak/want.f WANT #define

0 VALUE  M#define-CODE

: M#define CREATE PARSE-WORD EVALUATE ,
 DOES> [ HERE 5 - TO M#define-CODE ] @ ;

: Archive_
  PARSE-WORD EVALUATE
  '  DUP 1+  REL@ CELL+ M#define-CODE =
  IF
 >BODY ! EXIT
  THEN  1 THROW ;


: Archive \ F7_ED
  BEGIN
    PARSE-WORD DUP 0=
    IF  NIP  REFILL   0= IF DROP TRUE THEN
    ELSE S" size" COMPARE 0=  THEN
  UNTIL
  REFILL DROP
  BEGIN REFILL 0= IF \EOF EXIT THEN
    SOURCE NIP
  WHILE M#define
  REPEAT

  BEGIN REFILL
  WHILE     SOURCE NIP 40 >
     IF
            [']  Archive_ CATCH DROP
     THEN
  REPEAT      POSTPONE \
 ;
