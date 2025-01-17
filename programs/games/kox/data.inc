;#**************************
;#*     X.0.X - DATA
;#************************** 21/10/2007

macro DATA_SECTION
{
; ДАННЫЕ ПРОГРАММЫ
;-----------------------------------------------------------------------------
lootxt:   db 'oops! - Looser...'                     ; los
text:
texts:    db 'Congratulations! You are winner.'      ;  in window win
textend:  db 'New'     ;button id=200
px:       dw 50        ;x,y -
py:       dw 50        ;    - for x_put/o_put
seed:     dd  0        ;rnd
idx:      dd  0        ;N cell
;============ X = Y ============= coords of cells
 pole:    dw  50, 50   ;+0    or - dd 50*65536+50
          dw 100, 50   ;+4
          dw 150, 50   ;+8
          dw  50,100   ;+..
          dw 100,100   ;+
          dw 150,100   ;
          dw  50,150   ;
          dw 100,150   ;
          dw 150,150   ;
 pox:     dd 0,0       ;POLE  ( 3x3 )= 9 _bytes_:   'x' / 'o' / empty
 pox_:    db 0         ;9= 8+1
 fill:    db 0         ;turn counter  0..9
 xwin:    db 0         ;who is winner 0/1
 owin:    db 0         ;who is looser 0/1
 poxl:    dw 0         ;16 bits - x memory
 pool:    dw 0         ;16 bits - o memory
;----------------------------------------------------------------------------
 oxwall:  dw 100100100b,0010010010b,0001001001b    ;vert.
          dw 111000000b,0000111000b,0000000111b    ;horiz
          dw 100010001b,0001010100b                ;diag -winning combinations
 prav:   ;==== правила-комбинации=====
          dw 000000110b,0      ;ситуация - ход в ситуации
          dw 000000101b,1
          dw 000000011b,2
          dw 000110000b,3
          dw 000101000b,4
          dw 000011000b,5
          dw 110000000b,6
          dw 101000000b,7
          dw 011000000b,8      ;horiz

          dw 001001000b,0
          dw 001000001b,3
          dw 000001001b,6
          dw 010010000b,1
          dw 010000010b,4
          dw 000010010b,7
          dw 100100000b,2
          dw 100000100b,5
          dw 000100100b,8      ;vert

          dw 000010001b,8
          dw 100010000b,0
          dw 100000001b,4
          dw 000010100b,6
          dw 001010000b,2
          dw 001000100b,4      ;diag

caption	  db 'KOX v0.1', 0		

colors	  rd 10  
		  
}
