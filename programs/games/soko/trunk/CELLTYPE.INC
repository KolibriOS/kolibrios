
EOF equ 0xf0            ; image index:
macro EndConf {db 0xff}
tEmpty      equ 0000b   ;   (0xf) - missing
tPlace      equ 0001b   ;   9
tPlayer     equ 0010b   ;   0
tWall       equ 0100b   ;   1
tBlock      equ 0110b   ;   5

tHole       equ 0111b   ;   2
tConnect    equ 1000b   ;   7
tStConnect  equ 1001b   ;   8
tPlate      equ 1010b   ;   3
tBroken     equ 1011b   ;   4

tLaserW     equ 1100b   ;   0xa
tLaserS     equ 1101b   ;   0xa
tLaserN     equ 1110b   ;   0xa
tLaserE     equ 1111b   ;   0xa

tRedB       equ 1000b   ;   0xc
tBluB       equ 1001b   ;   0xb
tGrnB       equ 1010b   ;   0xd
tYelB       equ 1011b   ;   0xe

sSokoban    equ 0xf1
sSokolor    equ 0xf2
sSokonex    equ 0xf3