; $$$$$$$$$$$$$$$$$$$ ABAKIS $$$$$$$$$$$$$$$$$$$$$
; *************** STAR^2 SOFTWARE ****************
; ?????????????????? FONT.INC ????????????????????

; 100% portable font

;;;;;;;;;;;;;;;;;;;; SYMBOLS ;;;;;;;;;;;;;;;;;;;;;

; my default character arrangement: 95 symbols.
; includes all standard visible characters

; 0 1 2 3 4 5 6 7 8 9 A B C D E F G H I
; J K L M N O P Q R S T U V W X Y Z a b
; c d e f g h i j k l m n o p q r s t u
; v w x y z _	. ? ; : ' " , ~ ! @ # $
; % ^ & * ( ) [ ] { } = + - < > / \ | `

; symbol lookup table to convert 7BIT ASCII
; character to index. 63=ignore (spaces, etc)

align 4

FONT.SYMBOLS: db \
63,63,63,63,63,63,63,63, 63,63,63,63,63,63,63,63,\
63,63,63,63,63,63,63,63, 63,63,63,63,63,63,63,63,\
63,72,69,74,75,76,78,68, 80,81,79,87,70,88,64,91,\
00,01,02,03,04,05,06,07, 08,09,67,66,89,86,90,65,\
73,10,11,12,13,14,15,16, 17,18,19,20,21,22,23,24,\
25,26,27,28,29,30,31,32, 33,34,35,82,92,83,77,62,\
94,36,37,38,39,40,41,42, 43,44,45,46,47,48,49,50,\
51,52,53,54,55,56,57,58, 59,60,61,84,93,85,71,63
NO.C=63

macro FONT [p] { common IMAGE p }

align

void font.image.p
integer font.image.w, font.w, font.h
integer font.color=WHITE

macro set.font i {
  . font.image.p=i#.p, r0=i#.w
  . font.image.w=r0, r1=95, r0/r1
  . font.w=r0, font.h=i#.h
}

;;;;;;;;;;;;;;;;; DRAW CHARACTER ;;;;;;;;;;;;;;;;;

function draw.c, c, x, y
  locals i, p, iw
  try visible x, y, font.w, font.h
  . r0=FONT.SYMBOLS, r0+c, r0=*r0
  if r0=NO.C, return, end
  . r0*font.w, r0*4, r0+font.image.p
  . p=r0, iw=font.image.w, iw*4, i=font.h
  loop i, r0=i
    if r0>=font.h, break, end
    draw.scanline p, x, y, font.w
    . r1=iw, p+r1, y++
  endl
endf

;;;;;;;;;;;;;;;;;;; DRAW TEXT ;;;;;;;;;;;;;;;;;;;;

align
integer clip.tx=-1, clip.ty=-1

function draw.text, t, x, y
  locals i, n, p
  . r0=t, p=r0, i=0
  while true		  ; draw all characters
    . r1=i, r1*font.w	  ; x=i*font.w+x
    . r1+x, r0=p, r0=*r0  ; get c
    if r0=0Dh		  ; return?
      . i=0, r1=font.h	  ; reset x
      . y+r1, p++	  ; y+font.h
      go .next
    end
    . r2=y, r2-font.h	  ; clipping
    if r2>screen.h
      return
    end
    . r2=clip.tx
    if r2<>-1, r2-font.w
      if r1>=r2, return, end
    end
    . r2=clip.ty
    if r2<>-1, r2-font.h
      if r1>=r2, return, end
    end
    draw.c r0, r1, y	  ; draw
    . i++
    .next:
    . p++, r0=p, r0=*r0
  endw
endf

; get width of text=a1 with insets

function get.text.w, t
  text.n t
  . r1=r0, r1+2, r0=font.w, r0*r1
endf