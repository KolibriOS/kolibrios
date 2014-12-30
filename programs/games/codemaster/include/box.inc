; $$$$$$$$$$$$$$$$$$$ ABAKIS $$$$$$$$$$$$$$$$$$$$$
; *************** STAR^2 SOFTWARE ****************
; ??????????????????? BOX.INC ????????????????????

screen.w equ [!screen.w] ; forward reference
screen.h equ [!screen.h] ; restore at end

macro BOX [p] {
 forward p: integer p#.x, p#.y, p#.w, p#.h
}

;;;;;;;;;;;;;;;;;; VISIBILITY ;;;;;;;;;;;;;;;;;;;;

; is visible or partially? return 0 if
; completely invisible

function visible, x, y, w, h
  if w<=0, go .0, end
  if h<=0, go .0, end
  . r0=x, r1=y
  if r0>=screen.w, go .0, end
  if r1>=screen.h, go .0, end
  . r0+w, r1+h
  if r0<0, go .0, end
  if r1<0, go .0, end
  return 1
  .0:
endf 0

;;;;;;;;;;;;;;; POINT INSIDE BOX? ;;;;;;;;;;;;;;;;

; x>=bx and x<bx+bw and y>=by and y<by+bh

function point.inside, b, x, y
  locals b.x, b.y, b.w, b.h
  . r0=b,\
   b.x=[?box.x+r0], b.y=[?box.y+r0],\
   b.w=[?box.w+r0], b.h=[?box.h+r0],\
   r1=x, r2=b.x
  cmp r1, r2
  jl .r0
  add r2, b.w
  cmp r1, r2
  jge .r0
  . r1=y, r2=b.y
  cmp r1, r2
  jl .r0
  add r2, b.h
  cmp r1, r2
  jge .r0
  return 1
  .r0: . r0=0
endf

;;;;;;;;;;;;;;;;;;;;;; CLIP ;;;;;;;;;;;;;;;;;;;;;;

; "clipping"; to exclude invisible sections of
; imagery before drawing

; clip.pixel x, y. if invisible, return 0

function clip.pixel, x, y
  . r0=x, r1=y
  if r0<0, go .0, end
  if r0>=screen.w, go .0, end
  if r1<0, go .0, end
  if r1>=screen.h, go .0, end
  return 1
  .0:
endf 0

; 2-DO: convert to ABAKIS...

; clip.line o, &x, &y, &n. return 0 if
; completely invisible or adjust x/y/n
; if partially in/visible. o/rientation=
; 'h'/'v'. n=w/h. parameters sent by
; reference

function clip.line, o, x, y, n
  push r6 r7 r3
  . r6=x, r7=y, r3=n,\
   r0=[r6], r2=[r7], r1=[r3]

  ; if invisible, return 0

  cmp r0, screen.w	; x>=screen.w?
  jge .0
  cmp r2, screen.h	; y>=screen.h?
  jge .0
  cmp r1, 0		; w/h<=0?
  jle .0
  cmp o, 0		; orientation?
  jne .vertical
  .horizontal:
  cmp r2, 0		; y<0?
  jl .0
  cmp r2, screen.h	; y>=screen.h?
  jge .0
  . r2=r0, r2+r1	; x+w<0?
  cmp r2, 0
  jl .0
  if r0<0		; if x<0
   . r0+r1,\		; { w=x+w, x=0 }
    [r3]=r0,\
    dword [r6]=0
  end
  cmp r2, screen.w	; if x+w>=screen.w
  jl @f 		; { w=screen.w-x }
  . r0=screen.w,\
   r0-[r6], [r3]=r0
  @@:
  jmp .yes
  .vertical:
  cmp r0, 0		; x<0?
  jl .0
  cmp r0, screen.w	; x>=screen.w?
  jge .0
  . r1=r2,\
  r1+[r3]		; y+h<0?
  cmp r1, 0
  jl .0
  if r2<0		; if y<0
    . [r3]=r1,\ 	; { h=y+h, y=0 }
    dword [r7]=0
  end
  cmp r1, screen.h	; if y+h>=screen.h
  jl .yes		; { h=screen.h-y }
  . r0=screen.h,\
   r0-[r7], [r3]=r0
  .yes: . r0=YES
  jmp .e
  .0: . r0=NO
  .e:
  pop r3 r7 r6
endf

; clip.scanline &s, &x, &y, &w does the same
; as clip.line but sets the s/tart offset of
; pixels and adjusts w correctly. this only
; applies to scanlines, not one-color lines

function clip.scanline, s, x, y, w
  push r3
  . r3=s,\
   dword [r3]=0,\	  ; offset=0 initially
   r0=x, r0=[r0]
  cmp r0, screen.w	  ; x>=screen.w?
  jge .0
  . r2=y, r2=[r2]
  cmp r2, screen.h	  ; y>=screen.h?
  jge .0
  cmp r2, 0		  ; y<0?
  jl .0
  cmp r2, screen.h	  ; y>=screen.h?
  jge .0
  . r1=w, r1=[r1],\
   r3=r0, r3+r1 	  ; x+w<0?
  cmp r3, 0
  jl .0
  cmp r3, screen.w	  ; if x+w>=screen.w
  jl @f 		  ; w=screen.w-x
  . r1=screen.w,\
   r1-r0, r3=w,\
   [r3]=r1
  @@:
  cmp r0, 0		  ; if x<0, clip
  jg .e
  . r2=r0, -r2, r2*4	  ; index=-x * scale
  . r3=s, [r3]=r2,\
   r1=w, [r1]+r0,\	  ; w+=x
   r3=x, dword [r3]=0	  ; x=0
  . r0=YES
  jmp .e
  .0: . r0=NO
  .e:
  pop r3
endf

;;;;;;;;;;;;;;;;;;;;;;; BOX ;;;;;;;;;;;;;;;;;;;;;;

; BOX structure...

virtual at 0
  ?box:
  .x dd 0
  .y dd 0
  .w dd 0
  .h dd 0
END virtual

align

box:
 integer .x, .y, .w, .h

macro set.box box, x, y, w, h {
 . box#.x=x, box#.y=y,\
   box#.w=w, box#.h=h
}

function move.box.right, b, n
  . r0=b, r1=n, r0+?box.x, [r0]+r1
endf

function move.box.down, b, n
  . r0=b, r1=n, r0+?box.y, [r0]+r1
endf

function move.box.r, b
  . r0=b, r1=[?box.w+r0], r0+?box.x, [r0]+r1
endf

function move.box.d, b
  . r0=b, r1=[?box.h+r0], r0+?box.y, [r0]+r1
endf

restore screen.w, screen.h