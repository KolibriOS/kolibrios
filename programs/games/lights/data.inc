;#**************************
;#*        LIGHTS       (c)* data.inc
;#************************** 15/10/2007
L0 equ 0x0000F8    ; light off
L1 equ 0xFFFF00    ; light on
deltaX equ 35
deltaY equ 35
posX   equ 50
posY   equ 50

macro DATA_SECTION
{
; ÑÄççõÖ èêéÉêÄååõ
;-----------------------------------------------------------------------------
text:     db 'LIGHTS v0.002          [ESC]     - quit' ; winheader string
texts:    db 'Congratulations!!!'              ; in window
textend:

;=================Table===================== 18x18 :: N,Color,X,Y , up,lf,rt,dn
tButIdx: dd 0
tBut:
  db   5,0,  posX+deltaX*0, posY+deltaY*0 , 0 , 0 , 1 , 1
  db   6,0,  posX+deltaX*1, posY+deltaY*0 , 0 , 1 , 1 , 1
  db   7,0,  posX+deltaX*2, posY+deltaY*0 , 0 , 1 , 1 , 1
  db   8,0,  posX+deltaX*3, posY+deltaY*0 , 0 , 1 , 1 , 1
  db   9,0,  posX+deltaX*4, posY+deltaY*0 , 0 , 1 , 0 , 1

  db  10,0,  posX+deltaX*0, posY+deltaY*1 , 1 , 0 , 1 , 1
  db  11,0,  posX+deltaX*1, posY+deltaY*1 , 1 , 1 , 1 , 1
  db  12,0,  posX+deltaX*2, posY+deltaY*1 , 1 , 1 , 1 , 1
  db  13,0,  posX+deltaX*3, posY+deltaY*1 , 1 , 1 , 1 , 1
  db  14,0,  posX+deltaX*4, posY+deltaY*1 , 1 , 1 , 0 , 1

  db  15,0, posX+deltaX*0, posY+deltaY*2  , 1 , 0 , 1 , 1
  db  16,0, posX+deltaX*1, posY+deltaY*2  , 1 , 1 , 1 , 1
  db  17,0, posX+deltaX*2, posY+deltaY*2  , 1 , 1 , 1 , 1
  db  18,0, posX+deltaX*3, posY+deltaY*2  , 1 , 1 , 1 , 1
  db  19,0, posX+deltaX*4, posY+deltaY*2  , 1 , 1 , 0 , 1

  db  20,0, posX+deltaX*0, posY+deltaY*3  , 1 , 0 , 1 , 1
  db  21,0, posX+deltaX*1, posY+deltaY*3  , 1 , 1 , 1 , 1
  db  22,0, posX+deltaX*2, posY+deltaY*3  , 1 , 1 , 1 , 1
  db  23,0, posX+deltaX*3, posY+deltaY*3  , 1 , 1 , 1 , 1
  db  24,0, posX+deltaX*4, posY+deltaY*3  , 1 , 1 , 0 , 1

  db  25,0, posX+deltaX*0, posY+deltaY*4  , 1 , 0 , 1 , 0
  db  26,0, posX+deltaX*1, posY+deltaY*4  , 1 , 1 , 1 , 0
  db  27,0, posX+deltaX*2, posY+deltaY*4  , 1 , 1 , 1 , 0
  db  28,0, posX+deltaX*3, posY+deltaY*4  , 1 , 1 , 1 , 0
  db  29,0, posX+deltaX*4, posY+deltaY*4  , 1 , 1 , 0 , 0
;=======================================:: N,Color,X,Y , up,lf,rt,dn
;----------------------------------------------------------------------------
}
