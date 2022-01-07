: >asciiz + 0 SWAP C! ;
: >path
	finfo @ 20 + 0 finfo @ 8 + ! DUP >R SWAP DUP >R CMOVE R> R> SWAP >asciiz ;
: >param
	OVER finfo @ 8 + ! >asciiz ;
: exec
	16 finfo @ ! finfo @ 58 sys2 ." started, code=" . CR 5 sys2 DROP ;
S" /SYS/GRSCREEN" >path 99 exec
S" /SYS/@RB" >path 30 exec
S" /SYS/@SS" >path 30 exec
S" /SYS/@TASKBAR" >path 30 exec
S" /SYS/SETUP" >path S" BOOT" >param 30 exec
S" /SYS/ICON2" >path S" BOOT" >param 10 exec
S" /SYS/board" >path 25 exec
BYE
