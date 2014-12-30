; $$$$$$$$$$$$$$$$$$$ ABAKIS $$$$$$$$$$$$$$$$$$$$$
; *************** STAR^2 SOFTWARE ****************
; ?????????????????? DRAW.INC ????????????????????

; fast portable graphics

include 'color.inc'
include 'box.inc'

;;;;;;;;;;;;;;;;;;;;; SCREEN ;;;;;;;;;;;;;;;;;;;;;

align

vga equ (gs:r0)

; void vga.p

integer \
 screen.w, screen.h, screen.n,\
 screen.size, screen.pitch,\
 screen.bpp, screen.pw

void palette.p

function set.screen, w, h, bpp
  alias n=r0, pw=r1
  . screen.w=w, screen.h=h ; size
  . n=w, n*h, screen.n=n   ; # pixels
  . n=bpp, n>>>3, pw=n     ; pixel width
  . screen.pw=pw, n*pw     ; size
  . screen.size=n          ; in bytes
  . n=w, n*4               ; width
  . screen.pitch=n         ; in bytes
  . screen.bpp=bpp         ; bits per pixel
endf

; calculate x/y offset: (y*screen.w+x)*4

macro xy x, y {
  . r0=y, r0*1024, r0+x, r0*4
  . r1=WINDOW.Y, r1*4096, r0+r1
  . r1=WINDOW.X, r1*4, r0+r1
}

; erase screen with color

function clear.screen, c
  callf draw.box, 0, 0, WINDOW.W-1, WINDOW.H-1, c
endf

; draw straight line: loop n, *p++=c

function draw.line.h, x, y, w, c
  xy x, y
  . r1=c
  loop w, (u32) *vga=r1, r0+4, endl
endf

; draw vertical line

function draw.line.v, x, y, n, c
  locals swb
  alias p=r0, z=r1, h=r2
  xy x, y
  . z=c, swb=4096
  loop n, (u32) *vga=z, p+swb, endl
endf 1

; draw solid rectangle

function draw.box, x, y, w, h, c
  try visible x, y, w, h
  loop h
    draw.line.h x, y, w, c
    . y++
  endl
endf

; draw rectangle outline

function draw.outline, x, y, w, h, c
  try visible x, y, w, h
  draw.line.h x, y, w, c
  . r0=y, r0+h, r0--
  draw.line.h x, r0, w, c
  . r0=y, r0++, r1=h, r1-2
  draw.line.v x, r0, r1, c
  . r0=x, r0+w, r0--
  . r1=y, r1++, r2=h, r2-2
  draw.line.v r0, r1, r2, c
endf

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

; draw scanline: multi-color line,
; array of pixels:

; loop w, *p++=*s++, endl

function draw.scanline, s, x, y, w
  alias p=r0, q=r1, c=r2
  xy x, y
  . q=s
  loop w, (u32) c=*q, (u32) *vga=c
    . p+4, q+4
  endl
endf

; draw transparent scanline with color
; "key" to exclude

function draw.scanline.t, s, x, y, w, k
  alias p=r0, q=r1, c=r2
  xy x, y
  . q=s
  loop w, (u32) c=*q
    if c<>k, (u32) *vga=c, end
    . p+4, q+4
  endl
endf

; draw scanline with inverted x

function draw.scanline.ix, pixels, x, y, w
  alias p=r0, s=r1, c=r2
  . r0=x, r0+w
  xy r0, y
  . p-4, s=pixels
  loop w, (u32) c=*s++, (u32) *vga=c, p-4, endl
endf 1

; draw variant scanline. pixels are
; grayscale, alpha intensity of co=color.
; for brushes and special effects

function draw.scanline.v, pixels, x, y, w, co
  locals a
  alias p=r0, s=r1, c=r2, c2=r3
  xy x, y
  . s=pixels
  loop w, (u32) c=*s++
    . a=c, a&0FFh
    if a=0, go .next, end
    if a=0FFh, c=co, go .draw, end
    . (u32) c2=*p
    push p s
    get c=mix co, c2, a
    pop s p
    .draw: . (u32) *vga=c
    .next: . p+4
  endl
endf 1

; draw transparent scanline with key and
; alpha (0-255) applied to entire line

function draw.scanline.a, s, x, y, w, k, a
  alias p=r0, q=r1, c=r2, c2=r3
  xy x, y
  . q=s
  loop w, (u32) c=*q
    if c<>k, (u32) c2=*vga
      push p q
      get c=mix c, c2, a
      pop q p
    end, (u32) *vga=c
    .next:
    . p+4, q+4
  endl
endf

; draw bitmap; 2D array of pixels

function draw.bitmap, p, x, y, w, h
  locals i
  try visible x, y, w, h
  . i=y
  loop h, i++
    draw.scanline p, x, i, w
    . r0=w, r0*4, p+r0
  endl
endf

; draw transparent bitmap with color
; "key" by upper left pixel (X0,Y0)

function draw.bitmap.t, p, x, y, w, h
  locals i, k
  try visible x, y, w, h
  . r0=p, (u32) r0=*r0, k=r0
  . i=y
  loop h, i++
    draw.scanline.t p, x, i, w, k
    . r0=w, r0*4, p+r0
  endl
endf

; draw transparent bitmap with key and
; alpha (0-255) applied to entire image

function draw.bitmap.a, p, x, y, w, h, a
  locals i, k
  try visible x, y, w, h
  . i=y, r0=p, (u32) r0=*r0, k=r0
  loop h, i++
    draw.scanline.a p, x, i, w, k, a
    . r0=w, r0*4, p+r0
  endl
endf

; draw bitmap with inverted x

function draw.bitmap.ix, pixels, x, y, w, h
  locals p
  try visible x, y, w, h
  . p=pixels
  loop h
    draw.scanline.ix p, x, y, w
    . r0=w, r0*4, p+r0, y++
  endl
endf 1

; draw bitmap with inverted y

function draw.bitmap.iy, pixels, x, y, w, h
  locals p
  try visible x, y, w, h
  . r0=h, r0--, y+r0, p=pixels
  loop h
    draw.scanline p, x, y, w
    . r0=w, r0*4, p+r0, y--
  endl
endf 1

; draw bitmap with both inverted

function draw.bitmap.ixy, pixels, x, y, w, h
  locals p, n
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

;;;;;;;;;;;;;;;;; PALETTE PIXELS ;;;;;;;;;;;;;;;;;

; 8BPP versions with pa/lette. no clipping

function draw.scanline.8, pixels, x, y, w
  alias p=r0, s=r1, c=r2, q=r3
  xy x, y
  . s=pixels
  loop w, q=*s++, q*4, q+palette.p
    . (u32) c=*q, (u32) *vga=c, r0+4
  endl
endf 1

function draw.bitmap.8, pixels, x, y, w, h
  locals i, p
  try visible x, y, w, h
  . i=y, p=pixels
  loop h
    draw.scanline.8 p, x, i, w
    . i++, r0=w, p+r0
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
  locals a
  alias p=r0, s=r1, c=r2, c2=r3, q=r3
  xy x, y
  . s=pixels
  loop w, q=*s++, q*4, q+palette.p
    . (u32) c=*q, a=c, a&0FFh
    if a=0, go .next, end
    . (u32) c2=*vga
    push p s
    . r0=a
    if alpha.bias, r0+alpha.bias
      if r0<0, r0=0
      else.if r0>255, r0=255, end
    end
    get c=mix co, c2, r0
    pop s p
    .draw: . (u32) *vga=c
    .next: . p+4
  endl
endf 1

function draw.bitmap.v.8, pixels, x, y, w, h, c
  locals i, p
  try visible x, y, w, h
  . i=y, p=pixels
  loop h
    draw.scanline.v.8 p, x, i, w, c
    . i++, r0=w, p+r0
  endl
endf 1