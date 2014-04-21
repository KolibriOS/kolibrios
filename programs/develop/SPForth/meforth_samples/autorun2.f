: >asciiz + 0 SWAP C! ;
: >path
	finfo @ 20 + 0 finfo @ 8 + ! DUP >R SWAP DUP >R CMOVE R> R> SWAP >asciiz ;
: >param
	OVER finfo @ 8 + ! >asciiz ;
: exec
	16 finfo @ ! finfo @ 58 sys2 ." started, code=" . CR 5 sys2 DROP ;
S" /RD/1/GRSCREEN" >path 99 exec
S" /RD/1/@RB" >path 30 exec
S" /RD/1/@SS" >path 30 exec
S" /RD/1/@PANEL" >path 30 exec
S" /RD/1/SETUP" >path S" BOOT" >param 30 exec
S" /RD/1/ICON2" >path S" BOOT" >param 10 exec
S" /RD/1/board" >path 25 exec
BYE
