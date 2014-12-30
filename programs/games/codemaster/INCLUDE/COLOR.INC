; $$$$$$$$$$$$$$$$$$$ ABAKIS $$$$$$$$$$$$$$$$$$$$$
; *************** STAR^2 SOFTWARE ****************
; ?????????????????? COLOR.INC ???????????????????

COLOR fix integer
PIXEL fix integer

; rgb r, g, b
; get.rgb c, &r, &g, &b
; set.rgb &c, r, g, b

; mix a, b, n	   ; alpha combination
; lightness c, n   ; adjust light/darkness. n=+/-
; colorize a, b, n ; in/decrease r/g/b
; grayscale c	   ; convert to grayscale
; inversion c	   ; invert
; channelize a, b  ; split r/g/b channel

; PAL1	=		    I (2)
; PAL2	=		   II (4)
; PAL4	=		 IIII (16)
; PAL8	=	     IIIIIIII (256)
; PAL10 =	   IIIIIIIIII (1K, 1024)
; PAL12 =	 IIIIIIIIIIII (4K, 4096)
; RGB15 = X.RRRRR.GGGGG.BBBBB (32768, 32K)
; RGB16 =  RRRRR.GGGGGG.BBBBB (65535, 64K)
; RGB24 = R8.G8.B8 (16777216, 16M)
; RGB32 = A8.R8.G8.B8 (same with alpha)

; "standard" colors

numeric BLACK=0, WHITE=0FFFFFFh,\
 RED=0FF0000h, GREEN=0FF00h, BLUE=0FFh,\
 CYAN=BLUE+GREEN, MAGENTA=RED+BLUE,\
 YELLOW=RED+GREEN, GRAY32=202020h,\
 GRAY64=404040h, GRAY128=808080h,\
 GRAY192=0C0C0C0h, GRAY224=0E0E0E0h,\
 GRAY=808080h, GRAY25=404040h,\
 GRAY50=GRAY, GRAY75=0C0C0C0h

; personally selected "cool colors". includes
; nature colors for custom brushes/textures -
; grass, dirt, etc

numeric \
 RED.CHERRY=720E21h, FIRE.RED=821100h,\
 RED.ROSE=7B0024h, DARK.RED=470707h,\
 POWER.BLUE=0B008Ah, ROYAL.BLUE=140357h,\
 BEACH.BLUE=0362C1h, DARK.BLUE=070720h,\
 ICE.BLUE=8ACAE2h, SKY.BLUE=0A3A3F9h,\
 CLOUD.BLUE=11A3F5h, BABY.BLUE=4F77FFh,\
 LILAC=0DA4FE7h, PURPLE=700E7Fh,\
 VIOLET=08C047Ah, DARK.VIOLET=2D0327h,\
 LIME.GREEN=8CE007h, SEA.GREEN=72CC90h,\
 LEAF.GREEN=7EBB1Dh, GRASS.GREEN=507B11h,\
 DARK.GREEN=0B1F0Fh, EMERALD.GREEN=1C3E14h,\
 PINK=0FF67A0h, PASTEL.PINK=0F7A7CFh,\
 ROSE.PINK=0FB57A0h, PINK.FLESH=0FBD7BDh,\
 FLESH=0FCEEDEh, MEDIUM.FLESH=0EF9F5Dh,\
 DARK.FLESH=0E87D2Eh, LIGHT.BROWN=0C77237h,\
 DARK.BROWN=4D2207h, ORANGE.BROWN=0B04700h,\
 RED.BROWN=782712h, SAND=0D1A877h,\
 COOL.GRAY=837B9Bh, LILAC.GRAY=0E1ABFFh,\
 METAL.GRAY=0AEBEBEh, LIGHT.GRAY=0E7E7E7h

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; construct RGB 24/32BPP. assumes 0-255

function rgb, r, g, b
  . r0=r, r0<<16, r1=g, r1<<8,\
  r0|r1, r2=b, r0|r2
endf

macro rgbm r, g, b {
  . r0=r, r0<<16, r1=g, r1<<8,\
  r0|r1, r0|b
}

; with truncation. ensure 0-255

function rgbc, r, g, b
  . r0=r, r0&0FFh, r0<<16,\
  r1=g, r1&0FFh, r1<<8, r0|r1,\
  r2=b, r2&0FFh, r0|r2
endf

; extract rgb components from color
; parameters: c, &r, &g, &b. addresses

function get.rgb, c, r, g, b
  . r0=c,\
  r1=r0, r1>>16, r1&0FFh, r2=r, [r2]=r1,\
  r1=r0, r1>>8, r1&0FFh, r2=g, [r2]=r1,\
  r1=r0, r1&0FFh, r2=b, [r2]=r1
endf

; set rgb of color. parameters: &c, r, g, b.
; c is address

function set.rgb, c, r, g, b
  rgbm r, g, b
  . r1=c, [r1]=r0
endf

;;;;;;;;;;;;;;;;;;; CLIP RGB ;;;;;;;;;;;;;;;;;;;;;

; clip RGB components, 0-255. if c<min, c=min.
; if c>max, c=max. all parameters are address

function clip.c, c
  . r0=c, r0=[r0]
  if r0<0, r0=0
  else.if r0>255
    r0=255
  end
  . r2=c, [r2]=r0
endf

function clip.rgb, r, g, b
  clip.c r
  clip.c g
  clip.c b
endf

;;;;;;;;;;;;;;;;;;;;; ALPHA ;;;;;;;;;;;;;;;;;;;;;;

; mix a, b, n - alpha blend/combination.
; n=0-255. (((s-d)*n)/256)+d

function mix, a, b, n
  locals sr, sg, sb, dr, dg, db
  . r0=a, r2=b,\
   r1=r0, r1>>16, r1&0FFh, sr=r1,\
   r1=r0, r1>>8, r1&0FFh, sg=r1,\
   r1=r0, r1&0FFh, sb=r1,\
   r1=r2, r1>>16, r1&0FFh, dr=r1,\
   r1=r2, r1>>8, r1&0FFh, dg=r1,\
   r1=r2, r1&0FFh, db=r1,\
   r0=sr, r0-dr, r0*n, r0>>8, r0+dr,\
   r1=sg, r1-dg, r1*n, r1>>8, r1+dg,\
   r2=sb, r2-db, r2*n, r2>>8, r2+db,\
   r0<<16, r1<<8, r0|r1, r0|r2
endf

; mix25/50/75 a, b, n - much faster
; if alpha is constant 25%/50%/75%
; ((a&0FEFEFEh)>>1)|((b&0FEFEFEh)>>1)

macro mix50 a, b {
  . r0=a, r2=b, r1=0FEFEFEh,\
  r0&r1, r0>>1, r2&r1, r2>>1, r0|r2
}

function mix25, a, b
  mix50 a, b
  mix50 r0, b
endf

function mix75, a, b
  mix50 a, b
  mix50 a, r0
endf

;;;;;;;;;;;;;;;; COLOR OPERATIONS ;;;;;;;;;;;;;;;;

macro operation name, [ps] {
  common
  function name, ps
    locals r, g, b
    . r0=&r, r1=&g, r2=&b
    get.rgb c, r0, r1, r2
    ; ...
}

macro endo {
    ; ...
    . r0=&r, r1=&g, r2=&b
    clip.rgb r0, r1, r2
    rgb r, g, b
  endf
}

; adjust light/darkness. n=+/-

operation lightness, c, n
  . r1=n, r+r1, g+r1, b+r1
endo

; in/decrease r/g/b

operation colorize, c, x, n
  . r1=n
  if x=RED, r+r1, g-r1, b-r1
  else.if x=GREEN, r-r1, g+r1, b-r1
  else.if x=BLUE, r-r1, g-r1, b+r1
  end
endo

; convert to grayscale. algorithm:
; g=(r+g+b)/3, rgb(g,g,g). no "endo"
; after. r0=return. note: n/3 =
; (n*155h)>>10

operation grayscale, c
  . r0=r, r0+g, r0+b, r0*155h,\
  r0>>>10, r1=r0, r1<<8, r0|r1,\
  r1<<8, r0|r1
endf

; invert color: c=255-c

operation inversion, c
  . \
  r1=255, r1-r, r0=&r, [r0]=r1,\
  r1=255, r1-g, r0=&g, [r0]=r1,\
  r1=255, r1-b, r0=&b, [r0]=r1
endo

; split rgb channel: a=&~b. for 3D
; anaglyph images with R/B glasses

function channelize, a, b
  . r0=a, r1=b, not r1, r0&r1
endf

purge operation, endo