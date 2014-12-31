; $$$$$$$$$$$$$$$$$$$ ABAKIS $$$$$$$$$$$$$$$$$$$$$
; *************** STAR^2 SOFTWARE ****************
; ;;;;;;;;;;;;;;;;;;; IMAGE ;;;;;;;;;;;;;;;;;;;;;;

; image class/object/structure

macro IMAGE a {
  a:
  void a#.p
  integer a#.x, a#.y, a#.w, a#.h
  integer a#.bpp=32, a#.key, a#.alpha
}

virtual at 0
  ?image.p dd 0
  ?image.x dd 0
  ?image.y dd 0
  ?image.w dd 0
  ?image.h dd 0
  ?image.bpp dd 32
  ?image.key dd 0
  ?image.alpha dd 0
END virtual

?image.box fix ?image.x

; create image file/s with header:
; 8 bytes:

; byte s='I'  ; signature
; byte v=0    ; version: AABBCC.VV
; int16 w, h  ; size: w:h
; byte bpp    ; bpp: 32/24/16/15/8
; byte n      ; # colors or 0=256+

; byte pixels[w*h*(bpp/8)] ; or *2 if 15

macro IMAGE [p] {
 forward
  local w, h
  w=0
  h=0
  define ?s 0
  match a==b, p \{
   \local ..q
   ..q: inject.image b, 32
   load w word from ..q+2
   load h word from ..q+4
   a:
   void a\#.p=..q+8
   integer a\#.x, a\#.y, a\#.w=w, a\#.h=h
   integer a\#.bpp, a\#.key, a\#.alpha
   define ?s 1
  \}
  IF ?s eq 0
   IMAGE p
  END IF
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; load 24PP .BMP, store as .IMAGE 15/16/24/32.
; for OSs, ROMs. warning: causes slow compile
; with 1+ MB worth of images. to compile fast
; without images, comment "; IMAGE name='abc'"

macro inject.image name, bpp {
  local i, p, a, r, g, b,\
   x, y, w, h, wb
  virtual at 0
    p:: file CD#'media/'#name#'.bmp'
  END virtual
  IF ~bpp in <15,16,24,32>
    'Invalid BPP' name
  END IF
  load a word from p:0
  IF a<>'BM'
    'Invalid signature' name
  END IF
  load a byte from p:1Ch
  IF a<>24
    'Must be 24BPP' name
  END IF
  load w dword from p:12h
  load h dword from p:16h
  db 'I', 0
  dw w, h
  db bpp, 0
  a=((3-((w*3)+3)) and 3)
  wb=(w*3)+a
  y=h
  WHILE y>0
    o=36h+((y-1)*wb)
    x=0
    WHILE x<w
      i=o+(x*3)
      load b byte from p:i
      load g byte from p:i+1
      load r byte from p:i+2
      IF bpp=32
	dd (r shl 16) or (g shl 8) or b
      ELSE IF bpp=24
	db r, g, b   ; or b, g, r
      ELSE IF bpp=16
	r=((r and 11111b)/8) shl 11
	g=((g and 111111b)/4) shl 5
	b=((b and 11111b)/8)
	dw r or g or b
      ELSE IF bpp=15
	r=((r and 11111b)/8) shl 10
	g=((g and 11111b)/8) shl 5
	b=((b and 11111b)/8)
	dw r or g or b
      END IF
      x=x+1
    END WHILE
    y=y-1
  END WHILE
}

; insert 8BPP .BMP as .IMAGE with palette.
; note: must use special .8 drawing

macro inject.image.8 name {
  local i, p, a, c,\
   x, y, w, h, wb
  virtual at 0
    p:: file CD#'media/'#name#'.bmp'
  END virtual
  load a word from p:0
  IF a<>'BM'
    'Invalid signature' name
  END IF
  load a byte from p:1Ch
  IF a<>8
    'Must be 8BPP' name
  END IF
  load w dword from p:12h
  load h dword from p:16h
  db 'I', 0
  dw w, h
  db 8, 0
  i=0
  WHILE i<256
    o=36h+(i*4)
    load b byte from p:o
    load g byte from p:o+1
    load r byte from p:o+2
    db b, g, r, 0
    i=i+1
  END WHILE
  a=((3-(w+3)) and 3)
  wb=w+a
  y=h
  WHILE y>0
    o=436h+((y-1)*wb)
    x=0
    WHILE x<w
      load c byte from p:o+x
      db c
      x=x+1
    END WHILE
    y=y-1
  END WHILE
}

macro IMAGE8 [p] {
 forward
  local w, h
  w=0
  h=0
  define ?s 0
  match a==b, p \{
   \local ..q
   ..q: inject.image.8 b
   load w word from ..q+2
   load h word from ..q+4
   a:
   void a\#.p=..q+408h
   integer a\#.x, a\#.y, a\#.w=w, a\#.h=h
   integer a\#.bpp, a\#.key, a\#.alpha
   ; ...
   void a\#.palette=..q+8
   define ?s 1
  \}
  IF ?s eq 0
   'Error: 8BPP must specify file'
  END IF
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; unfinished, unorganized...

; 2-DO: convert to functions. create one
; good draw.scanline.x with bpp and type
; BIT flags:

; draw.scanline.x p, x, y, w, 32,\
;  G.KEY or G.ALPHA or G.GRAY or G.INVERT.X

macro move.image i, x, y {
 IF ~x eq
   . i#.x=x, i#.y=y
 END IF
}

macro draw.image i, x, y {
 move.image i, x, y
 draw.bitmap i#.p, i#.x, i#.y, i#.w, i#.h
}

macro draw.image.t i, x, y {
 IF ~x eq
   move.image i, x, y
 END IF
 draw.bitmap.t i#.p, i#.x, i#.y, i#.w, i#.h
}

; draw with inverted x/y

macro draw.image.ix i, x, y {
 IF ~x eq
   move.image i, x, y
 END IF
 draw.bitmap.ix i#.p,\
  i#.x, i#.y, i#.w, i#.h
}

macro draw.image.iy i, x, y {
 IF ~x eq
   move.image i, x, y
 END IF
 draw.bitmap.iy i#.p, i#.x, i#.y, i#.w, i#.h
}

macro draw.image.ixy i, x, y, ix, iy {
 IF ~x eq
   move.image i, x, y
 END IF
 draw.bitmap.ixy i#.p, i#.x, i#.y, i#.w, i#.h
}

macro draw.image.v i, x, y, c {
 IF ~x eq
   move.image i, x, y
 END IF
 draw.bitmap.v i#.p, i#.x, i#.y, i#.w, i#.h, c
}

; draw rotated. warning: no clipping

function draw.scanline.rl, pixels, x, y, w
  alias p=r0, s=r1, n=r2, c=r3
  xy x, y
  . s=w, s--, s*4, s+pixels, n=4096 ; screen.pitch
  loop w, (u32) c=*s--, (u32) *vga=c, p+n, endl
endf 1

function draw.scanline.rr, pixels, x, y, w
  alias p=r0, s=r1, n=r2, c=r3
  xy x, y
  . s=pixels, n=4096 ; screen.pitch
  loop w, (u32) c=*s++, (u32) *vga=c, p+n, endl
endf 1

function draw.bitmap.rl, pixels, x, y, w, h
  locals p
  try visible x, y, w, h
  . p=pixels
  loop h
    draw.scanline.rl p, x, y, w
    . r0=w, r0*4, p+r0, x++
  endl
endf 1

function draw.bitmap.rr, pixels, x, y, w, h
  locals p
  try visible x, y, w, h
  . r0=w, r0--, x+r0, p=pixels
  loop h
    draw.scanline.rr p, x, y, w
    . r0=w, r0*4, p+r0, x--
  endl
endf 1

macro draw.image.rl i, x, y {
 IF ~x eq
   move.image i, x, y
 END IF
 draw.bitmap.rl i#.p, i#.x, i#.y, i#.w, i#.h
}

macro draw.image.rr i, x, y {
 IF ~x eq
   move.image i, x, y
 END IF
 draw.bitmap.rr i#.p, i#.x, i#.y, i#.w, i#.h
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; 8BPP with palette...

macro draw.image.8 i, x, y {
 IF ~x eq
   move.image i, x, y
 END IF
 . palette.p=i#.palette
 draw.bitmap.8 i#.p, i#.x, i#.y, i#.w, i#.h
}

macro draw.image.v.8 i, x, y, c {
 IF ~x eq
   move.image i, x, y
 END IF
 . palette.p=i#.palette
 draw.bitmap.v.8 i#.p, i#.x, i#.y, i#.w, i#.h, c
}

function draw.image.v8, im, x, y, co
  locals p, w, h
  . r0=im
  . (u32) r1=*(r0+?image.p), p=r1
  . (u32) r1=*(r0+?image.w), w=r1
  . (u32) r1=*(r0+?image.h), h=r1
  draw.bitmap.v.8 p, x, y, w, h, co
endf

function draw.bitmap.viy.8, pixels,\
 x, y, w, h, c
  locals i, p
  try visible x, y, w, h
  . r0=h, y+r0, p=pixels
  loop h
    draw.scanline.v.8 p, x, y, w, c
    . r0=w, p+r0, y--
  endl
endf 1

function draw.image.viy.8, im, x, y, co
  locals p, w, h
  . r0=im
  . (u32) r1=*(r0+?image.p), p=r1
  . (u32) r1=*(r0+?image.w), w=r1
  . (u32) r1=*(r0+?image.h), h=r1
  draw.bitmap.viy.8 p, x, y, w, h, co
endf