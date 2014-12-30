; $$$$$$$$$$$$$$$$$$$ ABAKIS $$$$$$$$$$$$$$$$$$$$$
; *************** STAR^2 SOFTWARE ****************
; ?????????????????? DRAW.INC ????????????????????

; fast portable graphics

include 'color.inc'
include 'box.inc'

;;;;;;;;;;;;;;;;;;;;; SCREEN ;;;;;;;;;;;;;;;;;;;;;

align

void vga.p

integer screen.x, screen.y,\
 screen.w, screen.h, screen.n,\
 screen.bpp, screen.size,\
 screen.pw, screen.pitch
void palette.p
BOX g.box

function set.screen, w, h, bpp
  alias n=r0, pw=r1
  . screen.w=w, screen.h=h  ; size
  . n=w, n*h, screen.n=n    ; # pixels
  . pw=bpp, pw/8	    ; pixel width
  . screen.pw=pw, n*pw	    ; screen size
  . screen.size=n	    ; in bytes
  . n=w, n*pw		    ; screen pitch
  . screen.pitch=n	    ; w in bytes
  . screen.bpp=bpp	    ; bits per pixel
endf

;;;;;;;;;;;;;;;;;;;; DRAWING ;;;;;;;;;;;;;;;;;;;;;

; erase screen with color

function clear.screen, c
  alias p=r0, n=r1, z=r2
  . p=vga.p, n=screen.n, z=c
  loop n, (u32) *p++=z, endl
endf

; calculate x/y offset: (y*screen.w+x)*4

macro get.xy x, y
  { . r0=y, r0*screen.w, r0+x, r0*4 }

; address &vga[(y*screen.w+x)*4]

function vga.xy, x, y
  get.xy x, y
  . r0+vga.p
endf

; draw pixel

function draw.pixel, x, y, c
  alias p=r0, z=r1
  try clip.pixel x, y
  vga.xy x, y
  . z=c, (u32) *p=z
endf 1

; draw horizontal line

function draw.line.h, x, y, n, c
  alias p=r0, z=r1, w=r2
  . p=&x, z=&y, w=&n
  try clip.line 0, p, z, w
  vga.xy x, y
  . z=c
  loop n, (u32) *p++=z, endl
endf 1

; draw vertical line

function draw.line.v, x, y, n, c
  locals swb
  alias p=r0, z=r1, h=r2
  . p=&x, z=&y, h=&n
  try clip.line 1, p, z, h
  vga.xy x, y
  . z=c, swb=screen.pitch
  loop n, (u32) *p=z, p+swb, endl
endf 1

; draw solid rectangle

function draw.box, x, y, w, h, c
  locals i
  try visible x, y, w, h
  . i=y, i--
  loop h, i++
    draw.line.h x, i, w, c
  endl
endf 1

; draw rectangle outline

function draw.outline, x, y, w, h, c
  try visible x, y, w, h
  draw.line.h x, y, w, c    ; top
  . r0=y, r0+h, r0--	    ; bottom
  draw.line.h x, r0, w, c
  . r0=y, r0++, r1=h, r1-2  ; left
  draw.line.v x, r0, r1, c
  . r0=x, r0+w, r0--
  . r2=y, r2++, r1=h, r1-2  ; right
  draw.line.v r0, r2, r1, c
endf 1

macro draw.box.s b, c
 { draw.box b#.x, b#.y, b#.w, b#.h, c }
macro draw.box.o b, c
 { draw.outline b#.x, b#.y, b#.w, b#.h, c }

macro draw.box a, b, c, d, e {
  IF ~e eq
    draw.box a, b, c, d, e
  ELSE IF ~d eq
    'Unsupported'
  ELSE IF ~c eq
    draw.box.s a, b
    draw.box.o a, c
  ELSE IF ~b eq
    draw.box.s a, b
  END IF
}

; draw scanline

function draw.scanline, pixels, x, y, w
  locals i
  alias p=r0, s=r1
  . r0=&i, r1=&x, r2=&y, r3=&w
  try clip.scanline r0, r1, r2, r3
  vga.xy x, y
  . s=pixels, s+i
  loop w, (u32) *p++=*s++, endl
endf 1

; draw scanline with transparent key

function draw.scanline.t, pixels, x, y, w, key
  locals i
  alias p=r0, s=r1, c=r2
  . r0=&i, r1=&x, r2=&y, r3=&w
  try clip.scanline r0, r1, r2, r3
  vga.xy x, y
  . s=pixels, s+i
  loop w, (u32) c=*s++
    if c<>key, (u32) *p=c, end, p+4
  endl
endf 1

; draw scanline with inverted x

function draw.scanline.ix, pixels, x, y, w
  locals i
  alias p=r0, s=r1
  . r0=x, r0+w
  vga.xy r0, y
  . p-4, s=pixels
  loop w, (u32) *p--=*s++, endl
endf 1

; draw variant scanline. pixels are
; grayscale, alpha intensity of co=color.
; for brushes and special effects

function draw.scanline.v, pixels, x, y, w, co
  locals a, i
  alias p=r0, s=r1, c=r2, c2=r3
  . r0=&i, r1=&x, r2=&y, r3=&w
  try clip.scanline r0, r1, r2, r3
  vga.xy x, y
  . s=pixels, s+i
  loop w, (u32) c=*s++
    . a=c, a&0FFh
    if a=0, go .next, end
    if a=0FFh, c=co, go .draw, end
    . (u32) c2=*p
    push p s
    get c=mix co, c2, a
    pop s p
    .draw: . (u32) *p=c
    .next: . p+4
  endl
endf 1

; draw bitmap

function draw.bitmap, pixels, x, y, w, h
  locals i, p
  try visible x, y, w, h
  . i=y, p=pixels
  loop h
    draw.scanline p, x, i, w
    . r0=w, r0*4, p+r0, i++
  endl
endf 1

; draw bitmap with transparency by
; upper left pixel color (X=0, Y=0)

function draw.bitmap.t, pixels, x, y, w, h
  locals i, p, key
  try visible x, y, w, h
  . i=y, r0=pixels, p=r0
  . (u32) r0=*r0, key=r0
  loop h
    draw.scanline.t p, x, i, w, key
    . r0=w, r0*4, p+r0, i++
  endl
endf 1

; draw bitmap with inverted x

function draw.bitmap.ix, pixels, x, y, w, h
  locals i, p
  try visible x, y, w, h
  . p=pixels
  loop h
    draw.scanline.ix p, x, y, w
    . r0=w, r0*4, p+r0, y++
  endl
endf 1

; draw bitmap with inverted y

function draw.bitmap.iy, pixels, x, y, w, h
  locals i, p
  try visible x, y, w, h
  . r0=h, r0--, y+r0, p=pixels
  loop h
    draw.scanline p, x, y, w
    . r0=w, r0*4, p+r0, y--
  endl
endf 1

; draw bitmap with both inverted

function draw.bitmap.ixy, pixels, x, y, w, h
  locals i, p, n
  try visible x, y, w, h
  . p=pixels
  loop h
    draw.scanline.ix p, x, y, w
    . r0=w, r0*4, p+r0, y--
  endl
endf 1

; draw variant bitmap

function draw.bitmap.v, pixels, x, y, w, h, c
  locals i, p
  try visible x, y, w, h
  . i=y, r0=pixels, p=r0
  loop h
    draw.scanline.v p, x, i, w, c
    . r0=w, r0*4, p+r0, i++
  endl
endf 1

; draw gradual vertical fade from
; color a to b. o/rient:
; 'h'=horizontal, 'v'=vertical

function draw.fade, bo, c1, c2
  locals x, y, w, h, i, n, c,\
   r, g, b, red, green, blue,\
   nr, ng, nb, first, last
  . r0=bo,\
   x=[?box.x+r0], y=[?box.y+r0],\
   w=[?box.w+r0], h=[?box.h+r0]

  . r0=y, first=r0, r1=h
  . n=r1, r0+r1, last=r0

  . r0=&r, r1=&g, r2=&b
  get.rgb c1, r0, r1, r2
  . r0=&red, r1=&green, r2=&blue
  get.rgb c2, r0, r1, r2

  . r<<8, g<<8, b<<8, r1=n
  if r1=0, r1++, end
  . r0=red, r0<<8, r0-r, r0/r1, nr=r0
  . r0=green, r0<<8, r0-g, r0/r1, ng=r0
  . r0=blue, r0<<8, r0-b, r0/r1, nb=r0

  . i=first
  forever
    . r0=r, r0>>8, r1=g, r1>>8, r2=b, r2>>8
    get c=rgb r0, r1, r2
    draw.line.h x, i, w, c
    . r0=nr, r+r0, r1=ng, g+r1, r2=nb, b+r2
    . i++, r0=last
    if i>r0, go .out, end
  endfv
  .out:
endf

; draw with vertical center fade:
; a to b then b to a

function draw.shade, bo, a, b
  memory.copy g.box, bo, 16
  . g.box.h>>>1
  draw.fade g.box, a, b
  . r0=g.box.h, g.box.y+r0
  draw.fade g.box, b, a
endf

;;;;;;;;;;;;;;;;; PALETTE PIXELS ;;;;;;;;;;;;;;;;;

; 8BPP versions with pa/lette. no clipping

function draw.scanline.8, pixels, x, y, w
  alias p=r0, s=r1, c=r2, q=r3
  vga.xy x, y
  . s=pixels
  loop w, q=*s++, q*4, q+palette.p
    . (u32) c=*q, (u32) *p++=c
  endl
endf 1

function draw.bitmap.8, pixels, x, y, w, h
  locals i, p
  try visible x, y, w, h
  . i=y, i--, p=pixels
  loop h, i++
    draw.scanline.8 p, x, i, w
    . r0=w, p+r0
  endl
endf 1

;;;;;;;;;;;;;;;;;;;;; SPECIAL ;;;;;;;;;;;;;;;;;;;;

; special variant 8BPP with alpha bias for
; fonts and sketching effects (example:
; chalkboard)

A.LIGHTEST=128
A.LIGHTER=96
A.LIGHT=64
A.DARK=-32
A.DARKER=-64
A.DARKEST=-96

align
integer alpha.bias=0 ; A.DARKEST

function draw.scanline.v.8, pixels, x, y, w, co
  locals a, i
  alias p=r0, s=r1, c=r2, c2=r3, q=r3
  vga.xy x, y
  . s=pixels
  loop w, q=*s++, q*4, q+palette.p
    . (u32) c=*q, a=c, a&0FFh
    if a=0, go .next, end
    . (u32) c2=*p
    push p s
    . r0=a
    if alpha.bias, r0+alpha.bias
      if r0<0, r0=0
      else.if r0>255, r0=255, end
    end
    get c=mix co, c2, r0
    pop s p
    .draw: . (u32) *p=c
    .next: . p+4
  endl
endf 1

function draw.bitmap.v.8, pixels, x, y, w, h, c
  locals i, p
  try visible x, y, w, h
  . i=y, i--, p=pixels
  loop h, i++
    draw.scanline.v.8 p, x, i, w, c
    . r0=w, p+r0
  endl
endf 1