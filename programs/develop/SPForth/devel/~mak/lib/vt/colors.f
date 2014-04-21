
: (ESC) 27 EMIT TYPE ;
: CLEAR S" [2J" (ESC)  ;    : HOME CLEAR S" [1;1H" (ESC) ;
: NORMAL S" [0m" (ESC) ;    : BOLD S" [1m" (ESC) ;

: BLACK S" [30m" (ESC) ;    : RED S" [31m" (ESC) ;
: GREEN S" [32m" (ESC) ;    : YELLOW S" [33m" (ESC) ;
: BLUE S" [34m" (ESC) ;     : MAGENTA S" [35m" (ESC) ;
: CYAN S" [36m" (ESC) ;     : WHITE S" [37m" (ESC) ;

: ONBLACK S" [40m" (ESC) ;  : ONRED     S" [41m" (ESC) ;
: ONGREEN S" [42m" (ESC) ;  : ONYELLOW  S" [43m" (ESC) ;
: ONBLUE  S" [44m" (ESC) ;  : ONMAGENTA S" [45m" (ESC) ;
: ONCYAN  S" [46m" (ESC) ;  : ONWHITE   S" [47m" (ESC) ;

