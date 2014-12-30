; $$$$$$$$$$$$$$$$$$$ ABAKIS $$$$$$$$$$$$$$$$$$$$$  
; *************** STAR^2 SOFTWARE ****************
; ????????????????? MEMORY.INC ???????????????????

; allocate n          ; see SYSTEM.INC
; allocate.p p, n
; destroy p

; memory.set p, v, n  ; 32BIT set/copy/zero
; memory.copy a, b, n
; memory.zero p, n

; get.bit v, n   ; get/set/zero bit
; set.bit v, n
; zero.bit v, n
; enable.f v, n  ; enable/disable flag
; disable.f v, n

; power.2 n      ; is power of 2? which one?
; align.n n, p   ; versatile align n to p

;;;;;;;;;;;;; MEMORY COPY, SET, ZERO ;;;;;;;;;;;;;

; portable. 32BIT

function memory.copy, a, b, n
  alias p=r0, q=r1
  . p=a, q=b, n>>>2
  loop n, (u32) *p++=*q++, endl
endf

function memory.set, a, b, n
  alias p=r0, v=r1, x=r2
  . p=a, v=b, n>>>2
  loop n, (u32) *p++=v, endl
endf

macro memory.zero p, n { memory.set p, 0, n }

; x86 specific. aligned

function memory.copy.x, a, b, n
  push r6 r7
  . r7=a,\
   r6=b, r1=n
  test r7, r6    ; address=0?
  jz .e
  cmp r1, 4      ; if n<4
  jb @f
  push r1
  shr r1, 2      ; n/4
  rep movsd      ; copy dwords
  pop r1
  and r1, 3      ; modulo 4
  jz .e          ; remainder?
  @@:
  rep movsb      ; copy bytes
  .e:
  pop r7 r6
endf

function memory.set.x, p, v, n
  push r7
  . r7=p, r0=v,\
   r0*01010101h,\
   r1=n
  test r7, r7 ; address=0?
  jz .e
  cmp r1, 4   ; n<4?
  jb @f
  push r1
  shr r1, 2
  rep stosd   ; copy dwords
  pop r1
  and r1, 3   ; modulo 4
  jz .e       ; remainder?
  @@:
  rep stosb   ; copy bytes
  .e:
  pop r7
endf

;;;;;;;;;;;;;;;; GET/SET/ZERO BIT ;;;;;;;;;;;;;;;;

; 76543210. warning: r0/r1/r2 cannot be used
; as parameters. 'v' should be m, 'i' can be m/i

macro get.bit v, i {  ; (v>>i)&1
 . r0=v, r1=i, r0>>cl, r0&1
}

macro set.bit v, i {  ; v|=(1<<i)
 . r0=1, r1=i, r0<<cl, v|r0
}

macro zero.bit v, i { ; v&=~(1<<i)
 . r0=1, r1=i, r0<<cl, not r0, v&r0
}

; 1111.0000

macro get.nibble v, i { ; (v>>(i*4))&1111b
 . r0=v, r1=i, r1<<2, r0>>cl, r0&1111b
}

macro set.nibble v, i, n { ; v|=(n<<(i*4))
 . r0=v, r1=i, r2=n, r1<<2, r2<<cl,\
 r0|r2, v=r0
}

; 33.22.11.00

macro get.couple v, i { ; (v>>(i*2))&11b
 . r0=v, r1=i, r1<<1, r0>>cl, r0&11b
}

macro set.couple v, i, n { ; v|=(n<<(i*2))
 . r0=v, r1=i, r2=n, r1<<1, r2<<cl,\
 r0|r2, v=r0
}

; enable/disable flag

macro enable.f v, n { . r0=n, v|r0 }

macro disable.f v, n
 { . r0=n, not r0, v&r0 }

macro toggle n { xor n, 1 } ; invert 1/0

; create AA.BBB.CCCb/AA.BB.CC.DDb BIT structures

function triplet, a, b, c
  . r0=a, r0<<6, r1=b, r1<<3, r0|r1, r0|c
endf

function quadruplet, a, b, c, d
  . r0=a, r0<<6, r1=b, r1<<4
  . r2=c, r2<<2, r0|r1, r0|r2, r0|d
endf

; reverse byte order

macro reverse.32 n
 { . r0=n, bswap r0 }

macro reverse.24 n
 { . r0=n, bswap r0, r0>>>8 }

macro reverse.16 n
 { . r0=n, cl=al, al=ah, ah=cl }

;;;;;;;;;;;;;;;;;; POWERS OF 2 ;;;;;;;;;;;;;;;;;;;

; an unsigned number is a power of 2 if only
; 1 BIT is set: if !(n&n-1). subtracting 1
; inverts all BITs. if n=10000000b (80h/128),
; n&01111111b=0

; to find out which power of 2, search n
; for 1st 0 BIT from right to left

; is n power of 2? example: power.2 128
; returns 7

function power.2, n
  locals i
  . r0=n
  if r0<2, go .r0, end
  . r1=r0, r1-1, r0&r1
  test r0, r0
  jnz .r0
  . n--, i=1
  @@:
   . r0=1, r1=i, r0<<cl, i++
   test n, r0
  jnz @b
  . r0=i, r0--
  jmp @f
  .r0: . r0=0
  @@:
endf

;;;;;;;;;;;;;;;;;;;;; ALIGN ;;;;;;;;;;;;;;;;;;;;;;

; versatile align n/umber by power of 2

; return n aligned to p in r0. in r1,
; return the quantity to add to make n
; divisible by p. algorithm:

; n+(((p-1)-(n+p-1))&(p-1))

function align.n, n, p
  . r1=p, r1-1, r2=n, r2+r1, r0=r1
  . r0-r2, r0&r1, r1=r0, r2=n, r0+r2
endf

;;;;;;;;;;;;;;;; SOURCE, DESTINY ;;;;;;;;;;;;;;;;;

align

void source, destiny
integer origin, omega

function create.source, size
  destroy source
  try source=allocate size
  memory.zero source, size
  . origin=0, omega=0
endf 1

function create.destiny, size
  destroy destiny
  try destiny=allocate size
  memory.zero destiny, size
  . origin=0, omega=0
endf 1

;;;;;;;;;;;;;;;;;;; TESTING... ;;;;;;;;;;;;;;;;;;;

; optimized 128-BIT copy/set. ".fast"=CPU
; specific, but they do not replace the
; portable algorithms

; address p must be aligned by 16 (movaps) and
; size n must be divisible by 16. v/alue must
; be 32BIT or use 1/2 macros to expand

function memory.set.fast, p, v, n
  . r0=p, r2=v, r1=n,\  ; start at end
   r0+r1, neg r1        ; negate index
  movd xmm0, r2
  pshufd xmm0, xmm0, 0  ; duplicate dwords
  @@:
   movaps [r0+r1], xmm0
   add r1, 16
  jnz @b
endf

macro memory.set.2.fast p, v, n {
 . r0=v, r0*00010001h
 memory.set.fast p, r0, n
}

macro memory.set.1.fast p, v, n {
 . r0=v, r0*01010101h
 memory.set.fast p, r0, n
}

; destiny a/ddress must be aligned by 16 and
; size n must be divisible by 16

function memory.copy.fast.a, a, b, n
  . r0=a, r2=b, r1=n,\
   r0+r1, r2+r1, neg r1
  @@:
   movaps xmm0, [r2+r1]
   movaps [r0+r1], xmm0
   add r1, 16
  jnz @b
endf

; unaligned...

function memory.copy.fast, a, b, n
  . r0=a, r2=b, r1=n,\
   r0+r1, r2+r1, neg r1
  @@:
   movups xmm0, [r2+r1]
   movups [r0+r1], xmm0
   add r1, 16
  jnz @b
endf