; $$$$$$$$$$$$$$$$$$$ ABAKIS $$$$$$$$$$$$$$$$$$$$$
; *************** STAR^2 SOFTWARE ****************
; ???????????????????? TEXT ??????????????????????

; fast portable text operations

; text.zero t       ; initialize, *t=0
; text.end t        ; return end address (*t=0)
; text.n t          ; get # characters
; text.copy a, b    ; standard copy with 0
; text.copy.n...    ; copy with maximum size
; text.attach a, b  ; attach b to a
; text.attach.c...  ; attach c/haracter
; text.compare a, b ; compare lexical. <0>
; text.equal a, b   ; equal? return 1/0
; text.search a, b  ; search for text. &/0
; text.find a, c    ; search for c. &/0
; text.find.r a, c  ; search for c in reverse
; text.count.c t, c ; count # of 'c's
; text.count.w t    ; count # of words: 'a b c'
; text.count.n t    ; count # of lines
; text.go t, n      ; advance to line #
; text.upper t      ; uppercase
; text.lower t      ; lowercase
; text.reverse t    ; reverse
; text.begins a, b  ; begins with b?
; text.ends a, b    ; ends with b?
; text.skip.0 t     ; skip '0's. return &
; text.shift.l t, n ; shift characters left
; text.shift.la...  ; shift left and assign
; text.expand t, n  ; expand; shift right at 0
; text.align t, n   ; align; prefix with '0's

; x2t n, t ; convert number to text
; u2t n, t ; unsigned decimal
; h2t n, t ; hexadecimal
; b2t n, t ; binary

; t2x t    ; convert text to number
; t2u t    ; unsigned decimal
; t2h t    ; hexadecimal
; t2b t    ; binary

; print t, f, ... - a fast print formatted
; text to buffer

macro debug { callf _say, bug.t, bug.t }

RET equ ,0Dh,0Ah,

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; text.zero t - initialize

macro text.zero t { . r0=t, *r0=0 }

; text.end t - return end address: *p=0

function text.end, t
  alias p=r0, c=r1
  . p=t, c=1
  while c, c=*p++, endw, p--
endf

; text.n t - get length, # characters

function text.n, t
  alias p=r0, s=r1, c=r2
  . p=t, s=p, c=1
  while c, c=*p++, endw, p-s, p--
endf

; text.copy a, b - standard copy with
; 0 after. return advanced address

function text.copy, a, b
  alias p=r0, s=r1, c=r2
  . p=a, s=b, c=1
  while c, c=*s++, *p++=c, endw, p--
endf

; text.copy.n a, b, n - copy with maximum
; size specified. return &

function text.copy.n, a, b, n
  alias p=r0, s=r1, c=r2
  . p=a, s=b
  loop n, c=*s++, *p++=c
    if c=0, break, end
  endl, p--
endf

; text.attach a, b - attach b to a. return &

function text.attach, a, b
  text.end a
  text.copy r0, b
endf

; text.attach.c t, c - attach c/haracter.
; return &

function text.attach.c, t, c
  alias p=r0, x=r1
  text.end t
  . x=c, *p++=x, *p=0
endf

; text.compare a, b - lexical comparison.
; return <0>

function text.compare, a, b
  alias p=r0, s=r1, c=r2, d=r3
  . p=a, s=b, c=d
  while c=d, c=*p++, d=*s++
    if c=0, break, end
    if d=0, break, end
  endw, p=c, p-d
endf

; text.equal a, b - equal? return 1/0

function text.equal, a, b
  text.compare a, b
  if false, return 1, end
endf 0

; text.find t, c - search for character:
; if t contains c. return &/0

function text.find, t, x
  alias p=r0, c=r1, k=r2
  . p=t, k=x
  loop, c=*p
    if c=0, return 0, end
    if c=k, return p, end, p++
  endl
endf

; text.count.c t, c - count # of c/haracters

function text.count.c, t, x
  locals n
  alias p=r0, c=r1, k=r2
  . p=t, k=x, n=0, c=1
  while c, c=*p++
    if c=k, n++, end
  endw
endf n

; text.count.w t - count # words 'a b c'

function text.count.w, t
  try text.count.c t, ' '
  . r0++
endf

; text.count.n t - count # lines

function text.count.n, t
  try text.count.c t, 0Dh
  . r0++
endf

; text.go t, n - advance to line #

function text.go, t, n
  loop n
    try t=text.find t, 0Dh
    . t+2
  endl
endf t

; text.upper t - convert to uppercase

function text.upper, t
  alias p=r0, c=r1
  . p=t
  loop, c=*p
    if c=0, return, end
    if c>=97
      if c<=122, c-32, *p=c, end
    end, p++
  endl
endf

; text.lower t - convert to lowercase

function text.lower, t
  alias p=r0, c=r1
  . p=t
  loop, c=*p
    if c=0, return, end
    if c>=65
      if c<=90, c+32, *p=c, end
    end, p++
  endl
endf

; text.reverse t - reverse text

function text.reverse, t
  locals p
  alias s=r0, e=r1, c=r2
  get p=text.end t
  . e=&(s-1), s=t
  while s<e, c=*s, *s++=*e, *e--=c, endw
endf p

; text.begins a, b - a begins with b?

function text.begins, a, b
  alias p=r0, q=r1, c=r2, d=r3
  . p=a, q=b, d=c
  while c=d, c=*p++, d=*q++
    if c=0, break, end
    if d=0, break, end
  endw
  if d<>0, return 0, end
endf

; text.ends a, b - a ends with b?

function text.ends, a, b
  locals p
  get p=text.end a
  text.n b
  . p-r0
  text.equal p, b
endf

; text.search t, c - search for text:
; if a contains b. return &/0

function text.search, a, b
  locals n, p
  get n=text.n a
  . p=a
  loop n
    text.begins p, b
    if true, return p, end
    . p++
  endl
endf 0

; text.skip.0 a - skip 0s, return address or 0

function text.skip.0, a
  alias p=r0, c=r1
  . p=a, c='0'
  while c='0', c=*p++, endw, p--
  if c=0, return 0, end
endf

; shift all characters left

; before:    'abc123'
; after <<3: '123'

function text.shift.l, t, n
  alias p=r0, q=r1
  . p=t, q=p, q+n
  text.copy p, q
endf

; shift left and assign s to BITs
; copied out ('abc' in example above)

function text.shift.la, t, s, n
  alias p=r0, q=r1
  . p=t, q=p, q+n
  text.copy.n s, t, n
  text.shift.l t, n
endf

; expand; shift all characters right.
; example: 'abc123' becomes 'XXXabc123'
; after expand 3 where X is unknown

function text.expand, t, n
  locals x
  alias p=r0, q=r1, c=r2
  get x=text.n t
  . p+t, p--, q=p, p+n, *p=0
  loop x, *p--=*q--, endl
endf

; prefix text with '0's or ensure maximum n.
; example: before: t='7FAB'
; text.align t, 8 ; after: t='00007FAB'

function text.align, t, n
  locals tn
  alias n1=r0, n2=r1,\
   c=r1, p=r2
  get tn=text.n t
  . n2=n
  if n1=n2, return, end    ; same size
  if n1>n2, p=t, *(p+n1)=0 ; exceeds maximum
    return                 ; end at t+n
  end, n-n1                ; else, n2>n1
  text.expand t, n         ; expand t
  . p=t, c='0'             ; write '0's
  loop n, *p++=c, endl
endf

; search text array ta for t using text.equal.
; return index or -1 (<0) if not found. ta is
; an array of text addresses (text[])

function text.array.equal, ta, t, n
  locals i
  alias p=r0, q=r1, x=r2
  . i=0
  loop n, x=i, q=ta, (u32) p=*(q+x*4)
    text.equal p, t
    if true, return i, end
    . i++
  endl
endf -1

;;;;;;;;;;;;;;;;;; CONVERSIONS ;;;;;;;;;;;;;;;;;;;

; u2t n, t ; unsigned decimal
; h2t n, t ; hexadecimal
; b2t n, t ; binary

; convert unsigned 32BIT integer to text

function u2t, n, t
  alias i=r0, x=r1, y=r2, c=r3, p=r7
  push c p
  . i=n, p=t
  if i=0, *p++='0', *p=0
    . r0=p
    go .r
  end
  . x=1999999Ah
  while i, c=i, mul x, i=y, y=&(y+y*4)
    . y+y, c-y, c+'0', *p++=c
  endw, *p=0
  text.reverse t
  .r:
  pop p c
endf

; convert 32BIT hexadecimal number to text

function h2t, n, t
  alias p=r0, x=r1
  . p=t
  if n=0, *p++='0', *p=0
    return
  end
  while n, x=n, x&15
    . x=*(@HEX+x), *p++=x, n>>>4
  endw, *p=0
  text.reverse t
endf

align @HEX: db '0123456789ABCDEF'

; convert 32BIT binary number to text

function b2t, n, t
  alias p=r0, x=r1
  . p=t
  if n=0, *p++='0', *p=0
    return
  end
  while n, x=n, x&1, x+'0'
    . *p++=x, n>>>1
  endw, *p=0
  text.reverse t
endf

; t2u t ; unsigned decimal
; t2h t ; hexadecimal
; t2b t ; binary

; convert text to unsigned 32BIT integer

function t2u, t
  alias p=r0, c=r1, n=r2, x=r3
  try text.skip.0 t
  . n=0
  loop, c=*p++
    if c=0, return n, end
    . x=n, x<<2, n+x, n+n, n-'0', n+c
  endl
endf

; convert text to 32BIT hexadecimal

function t2h, t
  alias p=r0, c=r1, n=r2
  try text.skip.0 t
  . n=0
  loop, c=*p++
    if c=0, return n, end
    if c<=39h, c-30h
    else.if c>=61h, c-57h
    else, c-37h, end, n<<4, n+c
  endl
endf

; convert text to 32BIT binary

function t2b, t
  alias p=r0, c=r1, n=r2
  try text.skip.0 t
  . n=0
  loop, c=*p++
    if c=0, return n, end
    . n<<1, n-'0', n+c
  endl
endf

; convert number to text with alignment.
; example: n2t.a 1234h, t, 8, h
; after: t='00001234'

macro n2t.a n, t, a, b {
  b#2t n, t
  text.align t, a
}

macro u2t.a n, t, a { n2t.a n, t, a, u }
macro h2t.a n, t, a { n2t.a n, t, a, h }
macro b2t.a n, t, a { n2t.a n, t, a, b }

;;;;;;;;;;;;;;;;;;;;; PRINT ;;;;;;;;;;;;;;;;;;;;;;

; print t, f, ... - a fast print formatted
; text to buffer

; %t %s - 'text'; "string"
; %c    - character
; %n %u - 32BIT decimal unsigned
; %b    - 32BIT binary
; %h    - 32BIT hexadecimal
; %r    - return. insert 0Dh, 0Ah
; %0    - 0/NULL
; %%    - %

macro print t, f, [p] {
  common callv !print, t, f, p
}

; note: text.copy/reverse/etc and x2t
; must return end (*p=0) address for this

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align integer BASE='n'

; !print(n, $t, $f, ...)

!print:
 alias \               ; aliases...
  s=r6, p=r7,\         ; source, destiny
  c=r0, i=r1,\
  va.s=r2,\            ; &va[...] start
  va.n=[esp+4],\       ; # va
  va=[r2+r1*4],\       ; current va[i]
  n=[.a], z=[.z]       ; temporary

  . (u32) c=*(esp+8)   ; text buffer
  . (u32) i=*(esp+12)  ; format
  . va.s=&(esp+16)     ; &va[...] start

  push s p
  . p=c, s=i, i=0      ; index=0

  .get: . c=*s++       ; get c/haracter
  if c=0, go .e, end   ; end? *s=0
  if c<>'%'            ; not %
    .1: . *p++=c       ; copy c
    go .get            ; next
  end
  . c=*s++             ; %x
  if c='%', go .1      ; 2 %% = %
  end

  if c='0', *p++=0     ; '0' = 0/NULL
    go .get
  end

  if c='r'             ; 'r' = return
    .  *p++=\
     0A0Dh, go .get
  end

  if c='c'             ; %c
    . c=va, i++        ; get va[i++]
    . *p++=c
    go .get
  end

  if c='t'
    .t:                ; %t %s - text
    . z=va, i++        ; get va[i++]
    pusha
    get z=\
     text.copy p, z    ; copy, save &
    popa
    . p=z              ; advance text buffer
    go .get
  end
  if c='s', go .t
  end

  if c='n', go .n, end
  if c='u', go .n, end
  if c='h', go .n, end
  if c='b', go .n, end

  go .x                ; unrecognized

  .n:
  . n=va, i++          ; get va[i]
  pusha                ; copy/convert
  if c='n'             ; number to text
    u2t n, p           ; in selected base
  else.if c='u'
    u2t n, p
  else.if c='h'
    h2t n, p
  else.if c='b'
    b2t n, p
  end
  . z=r0               ; save end address
  popa
  . p=z                ; advance text buffer
  go .get

  .x: . *p++='?'       ; else, unrecognized,
  go .get              ; replace with '?'

  .0: . r0=0, go .q    ; error
  .e: . r0=p, *r0=0    ; success. terminate
  .q:                  ; return end
  pop p s
 ret                   ; callv adjusts esp after

endal                  ; end alias

.a dd 0
.z dd 0

;;;;;;;;;;;;;;; CHARACTER TABLES ;;;;;;;;;;;;;;;;;

; ILT - insensitive lookup table. A-Z/a-z are
; the same. this increases processing speed by
; many times. example: if (tt[a]=tt[b]) instead
; of: if ((a>='a'&a<='z')&(b>='a'&b<='z')) |
; ((a>='A'&a<='Z')&(b>='Z'&b<='Z'))

; TLT - type lookup table. each byte contains
; C.X BITs to determine its type fast in one
; comparison. example: if tt[c]&SYMBOL

align 8

; byte XLT[128]=

ILT db \
 00h,01h,02h,03h,04h,05h,06h,07h,08h,09h,0Ah,0Bh,0Ch,0Dh,0Eh,0Fh,\
 10h,11h,12h,13h,14h,15h,16h,17h,18h,19h,1Ah,1Bh,1Ch,1Dh,1Eh,1Fh,\
 20h,21h,22h,23h,24h,25h,26h,27h,28h,29h,2Ah,2Bh,2Ch,2Dh,2Eh,2Fh,\
 30h,31h,32h,33h,34h,35h,36h,37h,38h,39h,3Ah,3Bh,3Ch,3Dh,3Eh,3Fh,\
 40h,41h,42h,43h,44h,45h,46h,47h,48h,49h,4Ah,4Bh,4Ch,4Dh,4Eh,4Fh,\
 50h,51h,52h,53h,54h,55h,56h,57h,58h,59h,5Ah,5Bh,5Ch,5Dh,5Eh,5Fh,\
 60h,41h,42h,43h,44h,45h,46h,47h,48h,49h,4Ah,4Bh,4Ch,4Dh,4Eh,4Fh,\
 50h,51h,52h,53h,54h,55h,56h,57h,58h,59h,5Ah,7Bh,7Ch,7Dh,7Eh,7Fh

TLT db \
 00h,80h,80h,80h,80h,80h,80h,80h,80h,80h,40h,80h,80h,40h,80h,80h,\
 80h,80h,80h,80h,80h,80h,80h,80h,80h,80h,80h,80h,80h,80h,80h,80h,\
 20h,10h,04h,04h,10h,04h,04h,04h,04h,04h,04h,04h,04h,04h,10h,04h,\
 01h,01h,01h,01h,01h,01h,01h,01h,01h,01h,04h,04h,04h,04h,04h,10h,\
 10h,0Ah,0Ah,0Ah,0Ah,0Ah,0Ah,02h,0Ah,02h,02h,0Ah,02h,0Ah,02h,02h,\
 02h,02h,02h,02h,02h,02h,02h,02h,02h,02h,02h,04h,04h,04h,04h,10h,\
 04h,0Ah,0Ah,0Ah,0Ah,0Ah,0Ah,02h,0Ah,02h,02h,0Ah,02h,0Ah,02h,02h,\
 02h,02h,02h,02h,02h,02h,02h,02h,02h,02h,02h,04h,04h,04h,04h,80h

;            76543210b
C.NULL     = 00000000b ; 0
C.NUMBER   = 00000001b ; 0-9
C.ALPHA    = 00000010b ; A-Z, a-z
C.SYMBOL   = 00000100b ; all symbols except _.?!@$
C.NUMERIC  = 00001000b ; A-F/a-f, h,b,k,m/H,B,K,M
C.SYMBOLIC = 00010000b ; _.?!@$
C.SPACE    = 00100000b ; ' ', '/t'
C.RETURN   = 01000000b ; 0Dh, 0Ah
C.IGNORE   = 10000000b ; extended: 1.XXXXXXXb
C.KEYWORD  = 11111111b

C.DIGIT    = C.NUMBER or C.NUMERIC
C.NAME     = C.ALPHA or C.NUMBER or C.SYMBOLIC
C.SYMBOLS  = C.SYMBOL or C.SYMBOLIC
C.ALPHAN   = C.ALPHA or C.NUMBER
C.VISIBLE  = C.ALPHAN or C.SYMBOLS
C.WHITE    = C.SPACE or C.RETURN
C.BLANK    = C.WHITE or C.IGNORE
C.END      = C.SYMBOL or C.WHITE
C.0        = 0

function is.c, c, type
  . r0=c, r0=*(TLT+r0), r0&type
endf

macro if.is c, t {
  is.c c, t
  if true
}

;;;;;;;;;;;;;;;;;;; PARSE TEXT ;;;;;;;;;;;;;;;;;;;

; skip while type and not 0

; get p=text.skip.while p, C.WHITE

function text.skip.while, t, type
  alias p=r0, q=r1, c=r2
  . p=t, c=1
  while c, c=*p++
    . c=*(TLT+c), c&type
  endw, p--
endf

; skip until type and while not 0.
; kind of tricky

; get p=text.skip.until p, C.RETURN

function text.skip.until, t, type
  alias p=r0, q=r1, c=r2
  . p=t, c=0
  while c=0, c=*p++
    if c=0, return 0, end
    . c=*(TLT+c), c&type
  endw, p--
endf

; copy while type and not 0

; get s=text.copy.while t, s, C.NAME

function text.copy.while, t, s, type
  alias a=r7, b=r6, c=r1, x=r2
  . a=t, b=s, c=*b
  if c=0, return 0, end
  while c, c=*b++, *a++=c
    . x=*(TLT+c), x&type
    if x=0, break, end
  endw, a--, *a=0, b--
endf b

; copy until type and while not 0

; get s=text.copy.until t, s, C.END

function text.copy.until, t, s, type
  alias a=r7, b=r6, c=r1, x=r2
  . a=t, b=s, c=*b
  if c=0, return 0, end
  while c, c=*b++, *a++=c
    . x=*(TLT+c), x&type
    if x, break, end
  endw, a--, *a=0, b--
endf b

; copy until 'c'/x. if 0 or return is
; encountered before, return 0

function text.copy.until.c, t, s, x
  alias a=r7, b=r6, c=r1
  . a=t, b=s, c=*b
  if c=0, return 0, end
  while c, c=*b++, *a++=c
    if c=x, break, end
    if c=0, return 0, end
    if c=0Dh, return 0, end
  endw, a--, *a=0, b--
endf b

; skip whitespace and returns
; (if parse.skip.r?) or only spaces
; and tabs. return advanced &

align boolean parse.skip.r?=YES

function text.skip.ws, t
  if parse.skip.r?
    text.skip.while t, C.WHITE
  else
    text.skip.while t, C.SPACE
  end
endf

; skip all whitespace, returns
; (if parse.skip.r?) and comments.
; return advanced &

function text.skip.x, t
  locals q
  alias p=r0, c=r1
  . p=t, q=p
  .get:
  . p=q, c=*p
  if c=0
    return 0
  end
  if.is c, C.WHITE
    try q=text.skip.ws q
  end
  . p=q, c=*p
  if c=';'
    try q=text.skip.until \
     q, C.RETURN
    . p+2
    go .get
  end
endf q

function text.get, a, b
  locals q
  alias p=r0, c=r1
  . q=b
  try q=text.skip.x q
  . c=*p
  if c=0, return 0, end
  if.is c, C.SYMBOL
    . p=a, *p++=c, *p=0, q++
    return q
  end
  text.copy.until a, q, C.END
endf

;;;;;;;;;;;;;;;; SOURCE, DESTINY ;;;;;;;;;;;;;;;;;

align

void source.p, destiny.p, token.p
integer source.n, destiny.n, token.n

macro set.source p  { . source.p=p }
macro set.destiny p { . destiny.p=p }
macro set.token p   { . token.p=p }

function skip.while, type
  get source.p=text.skip.while \
   source.p, type
endf

function skip.until, type
  get source.p=text.skip.until \
   source.p, type
endf

function copy.while, type
  get source.p=text.copy.while \
   token.p, source.p, type
endf

function copy.until, type
  get source.p=text.copy.until \
   token.p, source.p, type
endf

function copy.until.c, c
  get source.p=text.copy.until.c \
   token.p, source.p, c
endf

macro skip.space { skip.while C.SPACE }
macro skip.white { skip.while C.WHITE }
macro skip.line  { skip.until C.RETURN }

; skip all whitespace, returns
; (if parse.skip.r?) and comments.
; return advanced &

function skip.x
  alias p=r0, c=r1
  .get:
  . p=source.p, c=*p
  if c=0, return 0, end
  if.is c, C.WHITE
    try skip.while C.WHITE
  end
  . p=source.p, c=*p
  if c=';'
    try skip.until C.RETURN
    . source.p+2
    go .get
  end
  . p=source.p
endf p

; get token

function get.token
  alias p=r0, c=r1
  try skip.x
  . c=*p
  if c=0, return 0, end
  if.is c, C.SYMBOL
    . p=token.p, *p++=c, *p++=0
    . source.p++, p=source.p
    if c='''', c=*p
      if c='''', source.p++
        return source.p
      end
      try copy.until.c ''''
      . source.p++
    end
    . p=source.p
    return
  end
  copy.until C.END
endf

; MOVE TO PARSE.INC...

;;;;;;;;;;;;;;;;;;; NAME TABLE ;;;;;;;;;;;;;;;;;;;

; name table:

; 'ann',0, 'kim',0, 'sue',0

align 4

void name.table, name.table.p,\
 name.table.n, name.table.size, name.table.end

; allocate name.table

function create.name.table, size
  try name.table=call !allocate, size
  . name.table.p=r0, r0+size
  . name.table.end=r0, name.table.n=0
  . name.table.size=0
endf 1

; get name address by index

function get.name, i
  locals p
  . p=name.table
  loop i
    get p=text.end p
    . p++
  endl
endf p

; search for name. return index or -1

function search.name, t
  locals i, p
  . p=name.table, i=name.table.n
  loop i
    get.name i
    text.equal p, r0
    if true, return i, end
  endl
endf -1

; attach 'text'. return address

function create.name, t
  locals p, n
  . p=name.table.p
  text.n t
  . r0++, n=r0, r0+p
  if r0>=name.table.end
    return 0
  end
  text.copy name.table.p, t
  . r0=p, r0+n, name.table.p=r0
  . name.table.n++, r0=n
  . name.table.size+r0
endf p

; remove name at index

function remove.name, i
  locals p, q, n, size
  get p=get.name i
  get n=text.n p
  . i++
  get q=get.name i
  . r0=name.table.p, r0+name.table.size
  . r0-p, size=r0, r0=n
  . name.table.size-r0, name.table.p-r0
  memory.copy p, q, size
  . name.table.n--
endf

; get name at index

function copy.name, i, t
  get.name i
  text.copy t, r0
endf

; name structure: 16 bytes

virtual at 0
  ?define:
  .type dd 0  ; type: TTTT...
  .value dd 0 ; name index
  .name dd 0  ; value or index
  .i dd 0     ; anything, align 16
  .$=$
END virtual

powers DEFINE.*, ALIVE, USED,\
 D, ABLE, NUMERIC

; .type =

; TTTT.SSSS XXXXXXXX XXXXXXXX XXX.NR.DUA
;           23-16    15-8     7-0

; T: type: number, define, constant, label,
; macro, register, instruction, variable
; S: data size
; P: parameter 1
; Q: parameter 2
; X: anything
; N: 1=numeric, 0=symbolic
; R: 1=redefinable, 0=not
; D: 1=defined, 0=not
; U: 1=used, 0=not
; A: 1=alive, 0=dead

; name structures. last structure is
; defines.p+((defines.n-1)*48)

align 4

void defines.p     ; name structures
void defines.end.p ; end allocation
integer defines.n

;;;;;;;;;;;;;;;;;;;;; ERRORS ;;;;;;;;;;;;;;;;;;;;;

; MOVE TO ASSEMBLER.INC

text errors.ta[]=\
 E.NONE     ='None',\
 E.SYNTAX   ='Syntax',\
 E.LOAD     ='Error loading',\
 E.FILE     ='File I/O error',\
 E.UNEXPECT ='Unexpected',\
 E.CPU      ='Unsupported by CPU',\
 E.ADDRESS  ='Address not aligned',\
 E.OPERANDS ='Invalid operand/s',\
 E.AMODE    ='Invalid addressing mode',\
 E.SHIFT    ='Invalid shift #',\
 E.NUMBER   ='Invalid number',\
 E.NUMBER2  ='Number can''t be encoded',\
 E.VALUE    ='Value exceeds size',\
 E.ALIGN    ='Number must be aligned',\
 E.POWER    ='# must be power of 2',\
 E.REGISTER ='Register expected',\
 E.EXPECTN  ='Number expected',\
 E.EXPECTA  ='Name expected',\
 E.INAME    ='Invalid name',\
 E.NAME     ='Name too long',\
 E.RESERVE  ='Reserved name',\
 E.UNDEFINE ='Undefined',\
 E.REDEFINE ='Redefinition',\
 E.EXPECTT  ='Text expected',\
 E.ENDT     ='Text has no end ''',\
 E.COMMA    =', expected',\
 E.END      ='end expected',\
 E.MISMATCH ='Mismatched parenthesis'

;;;;;;;;;;;;;;;;;;; KEYWORDS ;;;;;;;;;;;;;;;;;;;;;

; MOVE TO ASSEMBLER.INC

text keywords.ta[]=\
 K.ALIGN='align', K.BYTE='byte',\
 K.D1='d1', K.D2='d2', K.D4='d4', K.D8='d8',\
 K.DEFINE='define', K.DOUBLE='double',\
 K.EQUATE='equate', K.FLOAT='float',\
 K.FORMAT='format', K.INCLUDE='include',\
 K.INJECT='inject', K.INTEGER='integer',\
 K.MAIN='main', K.ORG='org',\
 K.RESTORE='restore', K.TEXT='text',\
 K.USE='use', K.VOID='void'

align 4

void keywords.ta.p=keywords.ta
integer keywords.n=keywords.ta.$

function is.keyword, t
  if keywords.n=0, return -1, end
  text.array.equal \
   keywords.ta.p, t, keywords.n
endf