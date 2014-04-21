
\ from gforth
: REPLACE-WORD ( by-xt what-xt )
    [ HEX ] E9 [ DECIMAL ] OVER C!  \ JMP ...
    1+ DUP >R
    CELL+ -
    R> !
;
