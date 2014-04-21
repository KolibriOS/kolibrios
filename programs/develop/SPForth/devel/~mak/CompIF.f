
: [ELSE]
    1
    BEGIN
      NextWord DUP
      IF  
         2DUP S" [IF]"   COMPARE 0= IF 2DROP 1+                 ELSE
         2DUP S" [ELSE]" COMPARE 0= IF 2DROP 1- DUP  IF 1+ THEN ELSE
              S" [THEN]" COMPARE 0= IF       1-                 THEN
                                    THEN  THEN   
      ELSE 2DROP REFILL  AND \   SOURCE TYPE
      THEN DUP 0=
    UNTIL  DROP ;  IMMEDIATE

: [IF] 0= IF [COMPILE] [ELSE] THEN ;  IMMEDIATE

: [THEN] ;  IMMEDIATE

C" \S" FIND NIP 0=
[IF]
: \S            \ comment to end of file
     SOURCE-ID FILE-SIZE DROP
     SOURCE-ID REPOSITION-FILE DROP
     [COMPILE] \ ; IMMEDIATE
[THEN]
