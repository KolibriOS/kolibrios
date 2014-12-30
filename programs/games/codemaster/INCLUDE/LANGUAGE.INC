; $$$$$$$$$$$$$$ ABA-KIS EVOLUTION $$$$$$$$$$$$$$$
; *************** STAR^2 SOFTWARE ****************
; ???????????????? LANGUAGE.INC ??????????????????

;;;;;;;;;;;;;;;;;;; REGISTERS ;;;;;;;;;;;;;;;;;;;;

r0 fix eax
r1 fix ecx
r2 fix edx
r3 fix ebx
r4 fix esp
r5 fix ebp
r6 fix esi
r7 fix edi

is.i? fix eqtype 0

is.m? fix eqtype [0]

is.r? fix in \
 <eax,ecx,edx,ebx,esp,ebp,esi,edi,\
 ax,cx,dx,bx,al,ah,cl,ch,dl,dh,bl,bh>

is.r32? fix in \
 <eax,ecx,edx,ebx,esp,ebp,esi,edi>

is.r16? fix in <ax,cx,dx,bx>
is.r16x? fix in <ax,cx,dx,bx,sp,bp,si,di>

is.r8l? fix in <al,cl,dl,bl>
is.r8h? fix in <ah,ch,dh,bh>
is.r8? fix in <al,ah,cl,ch,dl,dh,bl,bh>

;;;;;;;;;;;;;;;;; SYNTAX, VERIFY ;;;;;;;;;;;;;;;;;

?not fix ~
?empty fix eq

macro syntax v {
 IF v eq
  define ?s 1
 ELSE
  define ?s v
 END IF
}

macro verify e {
 IF ?s eq 0
  display `e
  'Error: ' e
 END IF
}

if.syntax fix IF ?s eq
if.not.syntax fix IF ?s eq 0

macro verify.r [r] {
 IF ~r is.r?
  'Register expected:' r
 END IF
}

macro verify.r32 [r] {
 IF ~r is.r32?
  'Register expected:' r
 END IF
}

macro verify.i i {
 IF ~i is.i?
  'Number expected:' i
 END IF
}

macro verify.n n, min, max {
 IF n<min | n>max
  'Number exceeds range:' min-max
 END IF
} 

macro verify.u3 n  { verify.n n, 0, 7 }
macro verify.u4 n  { verify.n n, 0, 15 }
macro verify.u5 n  { verify.n n, 0, 31 }
macro verify.u6 n  { verify.n n, 0, 63 }
macro verify.u7 n  { verify.n n, 0, 127 }
macro verify.8 n   { verify.n n, -255, 255 } 
macro verify.u8 n  { verify.n n, 0, 255 } 
macro verify.i8 n  { verify.n n, -128, 127 } 
macro verify.12 n  { verify.n n, -4095, 4095 } 
macro verify.u12 n { verify.n n, 0, 4095 } 
macro verify.i12 n { verify.n n, -2048, 2047 } 
macro verify.16 n  { verify.n n, -65535, 65535 } 
macro verify.u16 n { verify.n n, 0, 65535 } 
macro verify.i16 n { verify.n n, -32768, 32767 }

; verify size of block/text/array

macro verify.size n {
 IF n eq
  'Size must be specified'
 ELSE IF ~ n eqtype 0
  'Size must be numeric'
 ELSE IF n eq 0
  'Size cannot be zero'
 END IF
}

;;;;;;;;;;;;;;;;;;; ALIGNMENT ;;;;;;;;;;;;;;;;;;;;

macro align n {
 IF n eq        ; by address size
  align 4
 ELSE
  IF n is.i?    ; align n (standard)
   align n
  ELSE          ; align integer i=123
   align 4
   n            ; line after
  END IF
 END IF
}

macro IF.align n, by { IF n mod by=0 }

macro IF.not.align n, by { IF n mod by<>0 }

macro verify.a n, by {
 display.h n
 IF.not.align n, by
  'Number' n 'must be aligned by:' by
 END IF
}

;;;;;;;;;;;;;;;;;;;; DISPLAY ;;;;;;;;;;;;;;;;;;;;;

; display numbers at assembly time

macro display.b n {
 REPEAT 32
  display '0'+((n shr (32-%)) and 1)
 END REPEAT
}     

macro display.h n {
 local c
 REPEAT 8
  c=(n and (0Fh shl ((8-%) shl 2))) \
   shr ((8-%) shl 2) + '0'
  IF c>'9'
   c=c+7
  END IF
  display c
  IF %=8
   display 'h'
  END IF
  IF (((%*4) and 31)=0)
   display ' '
  END IF
 END REPEAT
}

;;;;;;;;;;;;;;;;;; EQUATE LIST ;;;;;;;;;;;;;;;;;;;

macro ?list.attach list, item {
 match any, list \{ list equ list, item \}
 match , list \{ list equ item \}
}

macro ?display.list [p] {
 common
  match q, p \{
   ?p equ q
   irps s, q \\{
    display \\`s, ' '
    match a b, ?p \\\{
     restore ?p
     ?p equ b
    \\\}
   \\}
   restore ?p
  \}
}

;;;;;;;;;;;;;;;;;;;; SETTINGS ;;;;;;;;;;;;;;;;;;;;

; example: ?enable align

?setting.align equ
?setting.debug equ

IF.align fix match =1, ?setting.align
IF.debug fix match =1, ?setting.debug

macro ?enable s, v {
 restore ?setting.#s
 match , v
  \{ ?setting.#s equ 1 \}
 match any, v
  \{ ?setting.#s equ v \}
}

macro ?disable s {
 restore ?setting.#s
 ?setting.#s equ 0
}

;;;;;;;;;;;;;;;;;;;; VARIABLE ;;;;;;;;;;;;;;;;;;;;

_strict_ equ 0 ; strict type safety?
_scope_ equ 0  ; 0=global, 1=local

_object_ equ 0

; create variable: name (a), type, value,
; size (n), sign (s)

macro ?V a, t, v, n, s {

 IF.align \{ align \}

 IF n=8
  !#a dq v
 ELSE IF n=4
  !#a dd v
 ELSE IF n=2
  !#a dw v
 ELSE IF n=1
  !#a db v
 ELSE
  'Invalid size'
 END IF

 name$#a equ a
 type$#a=t
 size$#a=n
 sign$#a=s
 scope$#a=_scope_
 strict$#a=_strict_

 a equ [!#a]
}

; define X. syntax: type then any a/a=b
; sequence/s separated with ,s.
; variables=0 if there is no initial value

macro ?D type, [p] {
 forward
  define ?s 0
  match =0 \
   a==b, ?s p \{ ; a=b
   type a, b
   define ?s 1
  \}
  match =0 \
   a=,, ?s p \{  ; a, (next)
   type a, 0
   define ?s 1
  \}
  match =0 \
   a, ?s p \{    ; a (end)
   type a, 0
   define ?s 1
  \}
  verify variable
}

; real variable names...

macro i8 a, v  { ?V a, 'i', v, 1, 1 }
macro u8 a, v  { ?V a, 'i', v, 1, 0 }
macro i16 a, v { ?V a, 'i', v, 2, 1 }
macro u16 a, v { ?V a, 'i', v, 2, 0 }
macro i32 a, v { ?V a, 'i', v, 4, 1 }
macro u32 a, v { ?V a, 'i', v, 4, 0 }
macro i64 a, v { ?V a, 'i', v, 8, 1 }
macro u64 a, v { ?V a, 'i', v, 8, 0 }

; aliases...

macro void [a]   { ?D u32, a }

macro char [a]   { ?D i8, a }
macro uchar [a]  { ?D u8, a }
macro wchar [a]  { ?D u16, a }
macro short [a]  { ?D i16, a }
macro ushort [a] { ?D u16, a }
macro int [a]    { ?D i32, a }
macro uint [a]   { ?D u32, a }
macro long [a]   { ?D i64, a }
macro ulong [a]  { ?D u64, a }
macro float [a]  { ?D u32, a }
macro double [a] { ?D u64, a }

macro byte [a]     { ?D i8, a }
macro ubyte [a]    { ?D u8, a }
macro integer [a]  { ?D i32, a }
macro uinteger [a] { ?D u32, a }
macro boolean [a]  { ?D i32, a }

;;;;;;;;;;;;;;;;; LITERAL TABLE ;;;;;;;;;;;;;;;;

?literals.i=0 ; offset

?literals equ align 4,\
 ?LITERALS:, ?literals.i=0

; store literal 'text'. attach line

macro ?literal t {
 local n
 virtual at 0
   db t, 0
   n=$
 END virtual
 ?literals equ ?literals,\
  db t, db 0
 ?literals.i=?literals.i+n
}

; store table. expand line/s

macro @literals {
 IF ?literals.i
  match j, ?literals \{
   irp i, j \\{ i \\}
  \}
 END IF
}

;;;;;;;;;;;;;;;;; TEXT VARIABLES ;;;;;;;;;;;;;;;;;

; create HL 'text' variable/s. text t='X'.
; note: (n) is the size

macro text [p] {
 forward
  local l
  define ?s 0
  match =0 \           ; text t(n)='abc'
   name(n)==t, ?s p \{
   l=$
   verify.size n
   name: db t, 0
   times \
    (n-($-l)) db 0
   name\#.$=n
   define ?s 1
  \}
  match =0 name(n),\   ; text t(n)
   ?s p \{
   verify.size n
   name: db n dup(0)
   name\#.$=n
   define ?s 1
  \}
  match =0 name==t,\   ; text t='abc'
   ?s p \{
   l=$
   name: db t, 0
   name\#.$=($-l)
   define ?s 1
  \}
  match =0 name, ?s p \{
   'Size not specified:' name
   define ?s 1
  \}
  verify name
}

; helper macros: text/x/a/i/p

; text extended to 2+ lines:

; text name=+'Hi, how ',\
;  'are ', 'you?'

macro textx name, [p] {
 common
  name: db p, 0
}

; create array of pointers and literal
; 'text'. optional a/lign each to size
; or 0=none:

; text names[]='ann', 'kim', 'sue'

macro texta a, name, [t] {
 common
  local n
  n=0
  label name dword
 forward
  local l
  dd l
  n=n+1
 forward
  l db t, 0
  IF a
   times (a-($-l)) db 0
  END IF
 common
  name#.$=n
}

; like texta with ids (0-n):

; textai 0, names.ta, 'ann', 'kim', 'sue'

macro textai a, name, [p] {
 common
  local n
  n=0
  label name dword
 forward
  local l
  dd l
 forward
  define ?s 0
  match id==t, p \{
   l db t, 0
   id=n
   n=n+1
   define ?s 1
  \}
  verify textai
  IF a
   times (a-($-l)) db 0
  END IF
 common
  name#.$=n
}

; same with prefix

macro textaip a, name, x, [p] {
 common
  local n
  n=0
  label name dword
 forward
  local l
  dd l
 forward
  define ?s 0
  match id==t, p \{
   l db t, 0
   x\#id=n
   n=n+1
   define ?s 1
  \}
  verify textaip
  IF a
   times (a-($-l)) db 0
  END IF
 common
  name#.$=n
}

; upgrade text to support ()/textx/a/i/p

macro text [p] {
 common
  define ?s 0
  match =0 (n), ?s p \{
   db n dup(0)
   define ?s 1
  \}
  match =0 a==+b, ?s p \{
   textx a, b
   define ?s 1
  \}
  match =0 \
   a[]==name* t, ?s p \{
   match x==y, t \\{
    textaip 0, a, name, t
    define ?s 1
   \\}
   IF ?s eq 0
    texta 0, a, t
   END IF
   define ?s 1
  \}
  match =0 \
   a[]==t, ?s p \{
   match x==y, t \\{
    textai 0, a, t
    define ?s 1
   \\}
   IF ?s eq 0
    texta 0, a, t
   END IF
   define ?s 1
  \}
  match =0 \
   a[](n)==t, ?s p \{
   texta n, a, t
   define ?s 1
  \}
  IF ?s eq 0
   text p
  END IF
}

; create 'text' in code. r/egister='t/ext'
; WARNING: all commands that accept a literal
; 'text' parameter (.t suffix) will alter eax.
; only intended for testing and utilities

macro make.txt r, t {
 IF t eqtype ""
  local ..t, ..e
  jmp ..e
  ..t db t, 0
  ..e:
  mov r, ..t
 ELSE
  mov r, t
 END IF
}

;;;;;;;;;;;;;;; VARIABLE EXAMPLES ;;;;;;;;;;;;;;

; variables: void, int, float, x

; int i, n=123

; void p

; float f=1.0, g=0.5
; double d, e=7.25

; text t='ABC', name(64) ; byte[]

; extended 2+ lines:

; text name=+'Hi, how ',\
;  'are ', 'you?'

; void[]+'text'...

; text ta[]='A', '123'

; with alignment:

; text names[](8)='ann', 'kim', 'sue'

; with numeric ID #s (0-n):

; text xyz[]= \
;  ID.A='A', ID.B='B', ID.C='C'

; with prefix + ID #s (ERROR.NAME):

; text errors[]= ERROR.* \
;  NONE    = 'No error',\
;  SYNTAX  = 'Syntax',\
;  ADDRESS = 'Invalid address',\
;  MEMORY  = 'Memory allocation',\
;  SETUP   = 'Initiation'

; access text from pointer array:

; mov eax, [ta+3*4]
; say [errors+ERROR.MEMORY*4]

;;;;;;;;;;;;;; NUMERIC CONSTANTS ;;;;;;;;;;;;;;;;;

; create incremental (0-n) list of numeric
; constants with optional prefix. each token
; can be assigned or it equals previous+1.
; if first token ends with *, it becomes the
; prefix for subsequent tokens. example:

; numeric COLOR.*,\
; BLACK, WHITE, RED, GREEN, BLUE

; COLOR.BLACK=0 ; output...
; COLOR.WHITE=1
; COLOR.RED=2
; COLOR.GREEN=3
; COLOR.BLUE=4

macro numeric ?p, [p] {
 common
  local n
  n=0                ; n=0
  syntax 0
  match z$*, ?p \{   ; extract prefix
   syntax 1
  \}
  match a==b, ?p \{  ; first item
   a=b
   n=b+1
   syntax 1
  \}
  match =0, ?s \{    ; else, first=0
   ?p=0
   n=1
  \}
 forward
  syntax 0
  match a==b, p \{   ; assigment?
   n=b               ; set n
   match z$*, ?p \\{ ; extract prefix
    z$\\#a=b
    syntax 1
   \\}
   if.syntax 0       ; no prefix
    a=b
   END IF
   syntax 1          ; yes, specified
  \}
  match =0, ?s \{    ; else=n (previous+1)
   match z$*, ?p \\{ ; extract prefix
    z$\\#p=n
    syntax 1
   \\}
   if.syntax 0       ; no prefix
    p=n
   END IF
  \}
  n=n+1              ; n++
}

KB=1024             ; 1<<10
MB=1048576          ; 1<<20
GB=1073741824       ; 1<<30

numeric YES=1, NO=0, TRUE=1, FALSE=0,\
 NULL=0, DEFAULT=0, NONE=-1, INVALID=-1,\
 DETECT=-1, INFINITE=-1, NOTHING=-1

; note: -1 means "not zero and not maximum"
; (FFFF/FF/FFh) for indices, sizes, handles
; (index or pointer), etc, where >=0 is
; valid/finite/something...

; if n=INFINITE     ; if -1
; if handle=INVALID ; if -1
; if n not INVALID  ; if <>-1
; if win>NOTHING    ; if >=0

;;;;;;;;;;;;;;;;;;;;; POWERS ;;;;;;;;;;;;;;;;;;;;;

; create successive powers of 2 starting
; at BIT0, from right to left. example:

; powers IS.*, ACTIVE, VISIBLE,\
;  DRAWABLE, MOVEABLE, SIZABLE

macro _powers [id] {
 common local n
 n=0
 forward id=1 shl n
 n=n+1
}

macro powers ?p, [p] {
 common
  local n
  n=0                ; n=1
  syntax 0
  match z$*, ?p \{   ; extract prefix
   syntax 1
  \}
  match =0, ?s \{    ; else, first item
   ?p=0
   n=1
  \}
 forward
  syntax 0
  match a==b, p \{   ; assigment?
   n=b               ; set n
   match z$*, ?p \\{ ; extract prefix
    z$\\#a=1 shl n
    syntax 1
   \\}
   if.syntax 0       ; no prefix
    a=1 shl n
   END IF
   syntax 1          ; yes, specified
  \}
  match =0, ?s \{    ; else=n (previous+1)
   match z$*, ?p \\{ ; extract prefix
    z$\\#p=1 shl n
    syntax 1
   \\}
   if.syntax 0       ; no prefix
    p=1 shl n
   END IF
  \}
  n=n+1              ; next bit
}

; create readable bit structure from
; left to right. example: 0000ABCDb.
; powerz A, B, C, D ; A=8, B=4, C=2, D=1

macro powerz [id] {
 common local n
 n=1
 forward n=n+1
 common n=n-2
 forward id=1 shl n
 n=n-1
}

; get power of 2. n-1 then search from left
; to right for 1st 0 BIT. rv=return value

; n=0
; ?power.2 256, n
; say.n n

macro ?power.2 n, rv {
 local i, x
 i=0
 IF (n and (n-1))=0
  i=1
  x=n-1
  WHILE 1
   IF x and (1 shl i)=0
    ?break
   END IF
   i=i+1
  END WHILE
 END IF
 rv=i
}

;;;;;;;;;;;;;;;;;;;; IF BIT/S ;;;;;;;;;;;;;;;;;;;;

; compare BITs in r or eax. alters edx

; if.bit 20
; if.bits 4-7=1001b
; if.bit 20, eax
; if.bits 31-24=11110011b, eax

macro if.bit n, r {
 IF ~r eq
  mov edx, r
 ELSE
  mov edx, eax
 END IF
 local ..start, ..else, ..end
 ?IF equ
 ?START equ ..start
 ?ELSE equ ..else
 ?END equ ..end
 ?START:
 test edx, (1 shl n)
 jz ?ELSE
}

macro if.not.bit n, r {
 IF ~r eq
  mov edx, r
 ELSE
  mov edx, eax
 END IF
 local ..start, ..else, ..end
 ?IF equ
 ?START equ ..start
 ?ELSE equ ..else
 ?END equ ..end
 ?START:
 test edx, (1 shl n)
 jnz ?ELSE
}

; alters edx

macro if.bits n, r {
 local i, b,\
  lo, hi, mask
 syntax 0
 match l-h==v, n \{     ; BITs = value
  IF l<h                ; low to high
   lo=l
   hi=h
  ELSE                  ; high to low
   lo=h
   hi=l
  END IF
  i=0                   ; search value from
  REPEAT 32             ; left to right (31-0)
   IF v and \           ; for 1st BIT set...
    (1 shl (31-i))<>0
    ?break
   END IF
   i=i+1
  END REPEAT
  b=0                   ; # BITs required
  REPEAT 32-i           ; for v/alue
   b=b+1
  END REPEAT
  IF b>(hi-lo+1)        ; example: 4-5=111b
   'Value exceeds size' ; 4-5 is only 2BITs,
  END IF                ; not enough for 111b.
  i=0
  mask=0                ; create mask: 111xb
  REPEAT (hi-lo+1)      ; # 1 BITs
   mask=\
    (mask or (1 shl i))
   i=i+1
  END REPEAT
  local ..start, ..else, ..end
  ?IF equ
  ?START equ ..start
  ?ELSE equ ..else
  ?END equ ..end
  IF ~r eq
   mov edx, r
  ELSE
   mov edx, eax
  END IF
  shr edx, lo           ; extract BITs
  and edx, mask
  ?START:
  cmp edx, v
  jne ?ELSE
  syntax 1
 \}
 verify
}

;;;;;;;;;;;;;;;;;;;;; CLASS ;;;;;;;;;;;;;;;;;;;;;;

; unfinished

; macro class name {
;   name:
;   macro u8 a, v \{ u8 name\#.\#a, v \}
;   macro i8 a, v \{ i8 name\#.\#a, v \}
;   macro u16 a, v \{ u16 name\#.\#a, v \}
;   macro i16 a, v \{ i16 name\#.\#a, v \}
;   macro u32 a, v \{ u32 name\#.\#a, v \}
;   macro i32 a, v \{ i32 name\#.\#a, v \}
;   macro u64 a, v \{ u64 name\#.\#a, v \}
;   macro i64 a, v \{ i64 name\#.\#a, v \}
;   ...
; }
;
; macro endc [p] {
;  common
;   restore u8, i8, u16, i16,\
;    u32, i32, u64, i64
; }

; 2-DO: single-line classes:

; class RGB     = byte a, r, g, b
; class POINT   = integer x, y
; class LINE    = POINT a, b
; class BOX     = integer x, y, w, h
; class POINT3D = integer x, y, z
; class POLYGON = ARRAY POINT3D points[]

macro assume [p] {
 common
  match name==type, p \{
   virtual at 0
    type name
   END virtual
  \}
}

;;;;;;;;;;;;;;;;; LET ARITHMETIC ;;;;;;;;;;;;;;;;;

; perform assignment/s, operation/s and
; low-level "expressions".

; signed is the default for numbers that can
; be negative (ex, coordinates). some symbols
; are for unsigned operations (>>> is shr)

; = is a standard copy, move, assignment

; =& gets the address of integers/void/x

; WARNING: both ++/--/+-/-+ cannot used.
; replace let eax+[?image.x+edx] (has 2 ++)
; with: let ecx=[?image.x+edx], eax+ecx

macro let [p] {
forward
 define ?s 0

 ; a=&b, lea r, [b]

 match =0 a==&(b), ?s p \{
  lea a, [b]
  define ?s 1
 \}

 match =0 a==&b, ?s p \{
  lea a, b
  define ?s 1
 \}

 match =0 a><b, ?s p \{
  xor a, b
  define ?s 1
 \}

 ; a=b, mov or push/pop or xor

 match =0 a==b, ?s p \{
  IF a eqtype [] & b eqtype [] ; m=m
   push dword b
   pop dword a
  ELSE
   IF b eq 0 & \
    a in <eax,ecx,edx,ebx,esi,edi>
     xor a, a
   ELSE
    mov a, b
   END IF
  END IF
  define ?s 1
 \}

 ; binary and/or

 match =0 a&b, ?s p \{ and a, b
  define ?s 1 \}
 match =0 a|b, ?s p \{ or a, b
  define ?s 1 \}

 ; shifts. note: >>> must be matched
 ; before >> or there will be a partial match

 match =0 a>>>b, ?s p \{ shr a, b
  define ?s 1 \}
 match =0 a<<b, ?s p \{ sal a, b
  define ?s 1 \}
 match =0 a>>b, ?s p \{ sar a, b
  define ?s 1 \}
 match =0 a<>>b, ?s p \{ ror a, b
  define ?s 1 \}

 ; increment/decrement. must be matched
 ; before a-b, a+b

 match =0 a--, ?s p \{
  dec a
  define ?s 1
 \}

 match =0 a++, ?s p \{
  inc a
  define ?s 1
 \}

 ; add/subtract

 match =0 a-b, ?s p \{
  IF b eq 1
   dec a
  ELSE
   sub a, b
  END IF
  define ?s 1
 \}

 match =0 a+b, ?s p \{
  IF b eq 1
   inc a
  ELSE
   add a, b
  END IF
  define ?s 1
 \}

 ; multiply. 2/4/8/16 will be optimized
 ; and replaced with shift left 1/2/3/4

 match =0 a*b, ?s p \{
  IF b eq 2
   sal a, 1
  ELSE IF b eq 4
   sal a, 2
  ELSE IF b eq 8
   sal a, 3
  ELSE IF b eq 16
   sal a, 4
  ELSE
   imul a, b
  END IF
  define ?s 1
 \}

 ; divide. a: must be eax or m/2/4/8/16.
 ; b: must be m, ecx or 2/4/8/16.
 ; example: eax=n/123 may be written as:

 ; let eax=n, ecx=123, eax/ecx

 ; ... instead of 4 separate lines

 match =0 a/b, ?s p \{
  IF b eq 2
   sar a, 1
  ELSE IF b eq 4
   sar a, 2
  ELSE IF b eq 8
   sar a, 3
  ELSE IF b eq 16
   sar a, 4
  ELSE
   IF a eq eax
    cdq
    idiv b
   ELSE
    'Unsupported'
   END IF
  END IF
   define ?s 1
 \}

 ; jmp/jz/jnz. only support the most
 ; common ones for fast assembly speed...

 match =0 =jmp l, ?s p \{
  jmp l
  define ?s 1
 \}

 match =0 =jz l, ?s p \{
  jz l
  define ?s 1
 \}

 match =0 =jnz l, ?s p \{
  jnz l
  define ?s 1
 \}

 ; unary prefixes

 match =0 -a, ?s p \{
  neg a
  define ?s 1
 \}

 match =0 =neg a, ?s p \{
  neg a
  define ?s 1
 \}

 match =0 =not a, ?s p \{
  not a
  define ?s 1
 \}

 match =0 =bswap a, ?s p \{
  bswap a
  define ?s 1
 \}

 match =0 xchg(a=,b), ?s p \{
  xchg a, b
  define ?s 1
 \}

 match =0 =mul a, ?s p \{
  mul a
  define ?s 1
 \}

 match =0 =div a, ?s p \{
  div a
  define ?s 1
 \}

 verify let
}

;;;;;;;;;;;;;;;;; GET SIZE/SIGN ;;;;;;;;;;;;;;;;;;

; from t=type name, return s=size and
; i=sign in numeric constants. example:

; size=0
; sign=0
; @gets i32, size, sign ; s=32, i=1

macro @gets t, s, i {
 s=0
 i=0
 IF t eq u32 | t eq i32 | t eq dword
  s=32
 ELSE IF t eq u16 | t eq i16 | t eq word
  s=16
 ELSE IF t eq u8 | t eq i8 | t eq byte
  s=8
 ELSE
  'Unknown type'
 END IF
 IF t eq i32 | t eq i16 | t eq i8
  i=1
 END IF
}

;;;;;;;;;;;;;;; POINTER ARITHMETIC ;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;; GET/SET VALUE ;;;;;;;;;;;;;;;;;

; helper load/store...

; t=type, r=register, v=value,
; s=size, i=sign

macro @gv t, r, v, s, i {
 IF s=32
  mov r, v
 ELSE IF s=8
  IF i=1
   movsx r, byte v
  ELSE
   movzx r, byte v
  END IF
 ELSE IF s=16
  IF i=1
   movsx r, word v
  ELSE
   movzx r, word v
  END IF
 END IF
}

macro @get t, r, [v] {
 common
  local s, i              ; get size
  @gets t, s, i           ; and sign
  define ?s 0
  match *a, v \{          ; *p
   match x++, a \\{       ; *p++
    ?a equ x
    ?o equ add
    define ?s 1
   \\}
   match =0 \
    x--, ?s a \\{         ; *p--
    ?a equ x
    ?o equ sub
    define ?s 1
   \\}
   match =0 x, ?s a \\{   ; else, *p
    ?a equ x
    ?o equ                ; no advance
    define ?s 1
   \\}
   IF ~ ?a is.r?
    'Error'
   END IF
   @gv t, r, [?a], s, i   ; get value
   IF ~ ?o eq             ; advance?
     ?o ?a, s/8           ; add/sub n
   END IF
   verify
  \}
  match =0 a, ?s v \{     ; r
   @gv t, r, a, s, i
   define ?s 1
  \}
  verify
  restore ?a, ?o
}

; t=type, r=register, v=value

macro @sv t, r, v {
 IF t eq u32 | t eq i32 | t eq dword
  mov dword v, r
 ELSE IF t eq u16 \
  | t eq i16 | t eq word
  IF r eq eax
   mov v, ax
  ELSE IF r eq ecx
   mov v, cx
  ELSE IF r eq edx
   mov v, dx
  ELSE IF r eq ebx
   mov v, bx
  ELSE                  ; assume i
   mov dword v, r
  END IF
 ELSE IF t eq u8 \
  | t eq i8 | t eq byte
  IF r eq eax
   mov v, al
  ELSE IF r eq ecx
   mov v, cl
  ELSE IF r eq edx
   mov v, dl
  ELSE IF r eq ebx
   mov v, bl
  ELSE                  ; assume i
   mov dword v, r
  END IF
 ELSE
  'Unknown type/size'
 END IF
}

macro @set t, r, [v] {
 common
  local s, i              ; get size
  @gets t, s, i           ; and sign
  define ?s 0
  match *a, v \{          ; *p
   match x++, a \\{       ; *p++
    ?a equ x
    ?o equ add
    define ?s 1
   \\}
   match =0 \
    x--, ?s a \\{         ; *p--
    ?a equ x
    ?o equ sub
    define ?s 1
   \\}
   match =0 x, ?s a \\{   ; else, *p
    ?a equ x
    ?o equ                ; no advance
    define ?s 1
   \\}
   IF ~ ?a is.r?
    'Error'
   END IF
   @sv t, r, [?a]         ; set value
   IF ~ ?o eq             ; advance
    ?o ?a, s/8            ; add/sub n
   END IF
   verify
  \}
  match =0 a, ?s v \{     ; r
   @sv t, r, a            ; set value
   define ?s 1
  \}
  verify
  restore ?a, ?o
}

;;;;;;;;;;;;;;;; POINTER OPERATION ;;;;;;;;;;;;;;;

macro let [p] {
 forward
  define ?s 0
  match =0 (t) \     ; (t) a=*(b)
   a==*(b), ?s p \{
    @get t, a, [b]
    define ?s 1
  \}
  match =0 \         ; a=*(b)
   a==*(b), ?s p \{  ; (u8) default
    @get u8, a, [b]
    define ?s 1
  \}
  match =0 (t) \     ; (t) *(a)=b
   *(a)==b, ?s p \{
    @set t, b, [a]
    define ?s 1
  \}
  match =0 \         ; *(a)=b
   *(a)==b, ?s p \{  ; (u8) default
    @set u8, b, [a]
    define ?s 1
  \}
  match \
   (t) a==b, p \{    ; (t) a=b
   match *x, a \\{   ; m,m
    match *y, b \\\{
     @get t, ebx, b  ; use ebx
     @set t, ebx, a
     define ?s 1
    \\\}
   \\}
   IF ?s eq 0
    match *y, b \\{  ; source=*p
     @get t, a, b
     define ?s 1
    \\}
    IF ?s eq 0
     match *x, a \\{ ; destiny=*p
      @set t, b, a
      define ?s 1
     \\}
    END IF
   END IF
  \}
  match =0 \
   a==b, ?s p \{     ; a=b
   match *x, a \\{   ; m,m
    match *y, b \\\{
     @get u8, ebx, b ; use ebx
     @set u8, ebx, a
     define ?s 1
    \\\}
   \\}
   IF ?s eq 0
    match *y, b \\{  ; source=*
     @get u8, a, b
     define ?s 1
    \\}
    IF ?s eq 0
     match *x, a \\{ ; destiny=*p
      @set u8, b, a
      define ?s 1
     \\}
    END IF
   END IF
  \}

  ; 2-DO: insert improved let

  IF ?s eq 0         ; default
    let p
  END IF
}

;;;;;;;;;;;;;;;;;;; FUNCTION ;;;;;;;;;;;;;;;;;;;;;

macro pushr [p] { ; push parameters backwards
 common           ; to access forwards
  IF ~p eq
   reverse
   pushd p
  common
  END IF
}

; call a/ddress direct or p/ointer indirect

macro call a, [p] {
 common pushr p
 call a
}

macro callp f, [p] { common call [f], p }

; call "function" forward reference that
; has not been defined yet

macro callf f, [p] { common call !#f, p }

; call "variadic" procedure with "variable
; arguments" (...). push invisible # arguments
; last, call, then adjust esp after

macro callv f, [p] {
 common ?n=0
 reverse pushd p
 ?n=?n+1
 common push ?n
 call f
 add esp, (?n+1)*4
}

; call "interface"

macro callx c, x, [p] {
 common
 pushr p            ; push parameters
 mov eax, c
 push eax           ; push class address
 mov eax, [eax]
 call dword [eax+x] ; call method offset
}

; call function pointer IF non-zero

macro callfp f {
 cmp dword [f], 0
 jz @f
 call dword [f]
 @@:
}

; call function IF defined

macro ?call name, [p] {
 common
 IF defined name
  call name, p
 END IF
}

; call function then get return. example:

; get n=text.n t
; get c=rgb 128, 0, 64

macro get [p] {
 common
  define ?s 0
  match v==f ps, p \{
   f ps
   mov v, eax
   define ?s 1
  \}
  match =0 v==f, ?s p \{
   f
   mov v, eax
   define ?s 1
  \}
  verify get
}

; set label for try/fail

define ?ef .! ; end function

macro fault l { define ?ef l }
macro catch l { define ?ef l }

; call function then jmp to return
; if it returns 0. example:

; try open filename
; try p=allocate 4*KB

macro try [p] {
 common
  define ?s 0
  match a==b, p \{
   b
   mov a, eax
   define ?s 1
  \}
  IF ?s eq 0
   common p
  END IF
  fail ?ef ; endf
}

;;;;;;;;;;;;;; FUNCTION/PROCEDURE ;;;;;;;;;;;;;;;

; create "function/proc/edure"...

macro function name, [p] {
 common
  local i, n
  n=0
 forward
  n=n+1
 common
 !#name#$type='f'

 ; only insert this inside of the executable
 ; if it was accessed somewhere

 IF used !#name ; real function !name
  !#name:

 ; macro to call with no prefix.
 ; example: f a, b, c

 macro name p \{
  pushr p
  call !\#name
 \}

 ?begin equ !#name
 ?parameters equ p
 ?alias equ

 ..n.parameters=0
 ..n.locals=0
 ..locals.size=0

 ; create parameter names and offsets

 IF ~ p eq           ; if parameters
  virtual at ebp+8
  forward
  local ..p
  ..p dd ?           ; (ebp+8)+i*4
  p equ [..p]
  ..n.parameters=\   ; i++
  ..n.parameters+1
  common
  END virtual
  push ebp           ; create stack frame
  mov ebp, esp
 END IF
 ; ...
}

; HL return statement. use this instead of
; ret/n in functions. no ret/urn before endf.
; it inserts one automatically

macro return v {
 IF ~v eq               ; value?
  mov eax, v
 END IF
 IF ..n.parameters<>0   ; if parameters
  mov esp, ebp
  pop ebp
  ret ..n.parameters*4  ; ret n
 ELSE IF ..n.locals<>0  ; if locals
  mov esp, ebp
  pop ebp
  ret
 ELSE
  ret
 END IF
}

; end function

macro endf v {
 IF ~v eq
  mov eax, v
 END IF
 .!:
 return
 .$=$-?begin              ; total size
 IF ..n.parameters<>0     ; if parameters
  match p, ?parameters
  \{ restore p, ?begin \}
  END IF
 IF ..n.locals<>0         ; if locals
  match l, local.names
  \{ restore l \}
 match a, ?alias \
  \{ restore a \}
  fault .!                ; reset try label
 END IF

 ; end "if used name" at very beginning
 ; of function

 END IF
}

; aliases for register names inside functions.
; endf will restore names. example:

; alias x=r0, y=r1, w=r2, h=r3, c=v1

macro alias [x] {
 forward
  syntax 0
  match name==r, x \{
   ?list.attach ?alias, name
   name equ r
   syntax 1
  \}
  verify
}

; end alias outside function

macro endal
 { match a, ?alias \{ restore a \} }

;;;;;;;;;;;;;;;;;;;;; LOCALS ;;;;;;;;;;;;;;;;;;;;;

; locals ... - create local 32BIT variables.
; example: locals x, y, n, c

macro locals [p] {
 common local.names equ p
 forward ..n.locals=..n.locals+1
 common ..locals.size=..n.locals*4
 virtual at ebp-..locals.size
 forward
 local ..l
 ..l dd ?
 p equ [..l]
 common
 END virtual
 IF ..n.parameters=0   ; create stack frame?
  push ebp
  mov ebp, esp
 END IF
 sub esp, ..locals.size ; allocate locals
}

; create locals of specified sizes or 32BIT.
; example:

; locale x, y, username(32), filename(256),\
; image(IMAGE.$), my.font(FONT.$), etc

macro locale [p] {
 common
  ..locals.size=0
 forward                 ; get names and sizes
  define ?s 0
  match name(size), p \{ ; size specified
   ?list.attach \
    local.names, name
   verify.size size
   ..locals.size=..locals.size+size
   define ?s 1
  \}
  match =0 name, ?s p \{ ; default 32BIT
   ?list.attach \
    local.names, name
   ..locals.size=..locals.size+4
   define ?s 1
  \}
  ..n.locals=..n.locals+1
 common
  virtual at ebp-..locals.size ; get offsets
 forward
  local ..l
  define ?s 0
  match name(size), p \{
   ..l dd (size/4) dup(?)
   name equ [..l]
   define ?s 1
  \}
  match =0 name, ?s p \{   ; default 32BIT
   ..l dd ?
   name equ [..l]
   define ?s 1
  \}
 common
  END virtual
  IF ..n.parameters=0    ; create stack frame?
   push ebp
   mov ebp, esp
  END IF
  sub esp, ..locals.size  ; allocate locals
}

;;;;;;;;;;;;;;;;;; IF EXPRESSION ;;;;;;;;;;;;;;;;;

?NOT equ 0
?use.r equ 0

; jump if condition to l (or ?NOT IF 1)

macro jif l, [c] {
common
 local f
 define ?s 0

 match =0 =true, ?s c \{
  test eax, eax
  jz l
  define ?s 1
 \}
 match =0 =false, ?s c \{
  test eax, eax
  jnz l
  define ?s 1
 \}
 match =0 =valid, ?s c \{
  cmp eax, -1
  je l
  define ?s 1
 \}
 match =0 =invalid, ?s c \{
  cmp eax, -1
  jne l
  define ?s 1
 \}

 macro ?J O,A,C,B, [X] \{
 match =0 X, ?s c \\{
  IF ?use.r eq 0
   O A, B         ; opcode o1, o2
  ELSE
   mov ?use.r, A
   O ?use.r, B
  END IF
  IF ?NOT eq 0
   j\#C l
  ELSE
   jN\#C l
  END IF
  define ?s 1
 \\}
 \}

 ; standard expressions...

 ?J cmp,a,NE,b,  a<>b     ; a not b (a<>b)

 ?J cmp,a,BE,b,  a<<==b   ; a<<=b
 ?J cmp,a,AE,b,  a>>==b   ; a>>=b
 ?J cmp,a,B,b,   a<<b     ; a<<b
 ?J cmp,a,A,b,   a>>b     ; a>>b

 ?J cmp,a,LE,b,  a<==b    ; a<=b
 ?J cmp,a,GE,b,  a>==b    ; a>=b
 ?J cmp,a,L,b,   a<b      ; a<b
 ?J cmp,a,G,b,   a>b      ; a>b
 ?J cmp,a,E,b,   a==b     ; a=b
 ?J cmp,a,NE,b,  a =not b ; a not b (a<>b)
 ?J test,a,NE,b, a&b      ; a&b
 ?J or,a,NE,b,   a|b      ; a|b
 ?J cmp,a,E,0,  =not a    ; not a (=0)

 ; 2-DO: support pointer arithmetic
 ; and functions: if p=allocate 4*KB

 ; else, default: a (not 0, by itself)

 ?J cmp,a,NE,0,a

 verify 'Invalid expression' c
 ?use.r equ 0
}

; jump to l IF NOT condition

macro jifn l, [c] {
 common
 ?NOT equ 1
 jif l, c
 ?NOT equ 0 ; restore default
}

; HL IF/ELSE

macro if.begin {
 local ..start, ..else, ..end
 ?IF equ
 ?START equ ..start
 ?ELSE equ ..else
 ?END equ ..end
 ?START:
}

macro if.n [c] {
 common
  if.begin
  jif ?ELSE, c
}

macro if [p] {
 common
  if.begin
  define ?s 0
  match a=,b, p \{
   jifn ?ELSE, a
   let b
   define ?s 1
  \}
  match =0, ?s \{
   jifn ?ELSE, p
  \}
  verify if
}

macro else [p] {
 common
  jmp ?END
  ?ELSE:
  restore ?IF
  ?IF equ ,
  match =,x, p \{ let x \}
}

macro else.if [p] {
 common
  local ..else
  jmp ?END
  ?ELSE:
  restore ?ELSE
  ?ELSE equ ..else
  define ?s 0
  match a=,b, p \{
   jifn ?ELSE, a
   . b
   define ?s 1
  \}
  match =0 a, ?s p \{
   jifn ?ELSE, a
  \}
  verify else.if
}

macro end [p] {
 common
  IF ?IF eq
   ?ELSE:
  END IF
  ?END:
  restore ?IF, ?START, ?ELSE, ?END
  match =,x, p \{ let x \}
}

; call function with parameters then
; if non/zero

macro !if f, [p] {
 common f p
 if eax
}

macro !if.n f, [p] {
 common f p
 if eax=0
}

jNE equ jne
jNNE equ je
jNA equ jna
jNB equ jnb
jNG equ jng
jNL equ jnl

macro jifc r, c, l {
 test r, r
 j#c l
}

macro fail l {
 IF l eq
  jifc eax, z, ?ef
 ELSE
  jifc eax, z, l
 END IF
}

macro failn l {
 IF l eq
  cmp eax, -1
  je ?ef
 ELSE
  cmp eax, -1
  je l
 END IF
}

macro success l { jifc eax, nz, l }

failnz fix success

; fail? or escape? on condition
; example: escape? eax=-1

macro fail? [c] {
 common
 IF c eq
  test eax, eax
  jz ?ef
 ELSE
  if c
   jmp ?ef
  end
 END IF
}

;;;;;;;;;;;;;;;;;;;;;; LOOP ;;;;;;;;;;;;;;;;;;;;;;

; loop variations. unfinished...

macro forever [p] {
 common
  local ..start, ..next, ..end
  ?STARTFV equ ..start
  ?NEXTFV equ ..next
  ?ENDFV equ ..end
  ?STARTFV:
  ?HL.BLOCK='fv'
  match =,x, p \{ let x \}
}

macro endfv [p] {
 common
  ?NEXTFV:
   jmp ?STARTFV
  ?ENDFV:
  restore ?STARTFV, ?NEXTFV, ?ENDFV
  match a=,b, p \{ let b \}
}

macro repeat n {
 local ..start, ..end
 ?STARTR equ ..start
 ?ENDR equ ..end
 ?NR equ n
 ?HL.BLOCK='r'
 ?STARTR:
  cmp ?NR, 0
  jle ?ENDR
}

macro endr {
  dec ?NR
 jmp ?STARTR
 ?ENDR:
 restore ?STARTR, ?ENDR, ?NR
}

macro while [c] {
 common
  local ..a, ..b, ..c
  ?STARTW equ ..a
  ?NEXTW equ ..b
  ?ENDW equ ..c
  ?HL.BLOCK='w'
  ?STARTW:
  define ?s 0
  match a=,b, c \{
   jifn ?ENDW, a
   let b
   define ?s 1
  \}
  match =0 a, ?s c \{
   jifn ?ENDW, a
  \}
}

macro endw [p] {
 common
  ?NEXTW:
   jmp ?STARTW
  ?ENDW:
  restore ?STARTW, ?NEXTW, ?ENDW
  match =,x, p \{ let x \}
}

; unfinished, exhausted, can't keep
; eyes open... 2-DO: improve all
; blocks/if/loops

; loop          ; infinite
; loop n        ; # times
; loop i=0 to n ; iterate (2-DO)
; loop X, ...   ; optional arithmetic

?LOOP.N fix 1   ; repeat # times
?LOOP.I fix 2   ; index, i=0 to n
?LOOP.F fix 3   ; forever, infinite

macro loop [c] {
 common
  local ..a, ..b, ..c
  ?STARTL equ ..a
  ?NEXTL equ ..b
  ?ENDL equ ..c
  ?STARTL:
  define ?s 0
  match =0 n=,x, ?s c \{  ; loop n, ...
   ?HL.BLOCK=?LOOP.N
   ?N equ n
   cmp n, 0
   jle ?ENDL
   let x
   define ?s 1
  \}
  match =0 =,x, ?s c \{   ; loop, ...
   ?HL.BLOCK=?LOOP.F
   let x
   define ?s 1
  \}
  match =0 n, ?s c \{     ; loop n
   ?HL.BLOCK=?LOOP.N
   ?N equ n
   cmp n, 0
   jle ?ENDL
   define ?s 1
  \}
  verify loop
}

macro endl [p] {
 common
  IF ?HL.BLOCK=?LOOP.N
    dec ?N
    restore ?N
  END IF
  ?NEXTL:
  jmp ?STARTL
  ?ENDL:
  restore ?STARTL, ?NEXTL, ?ENDL
  match =,x, p \{ let x \}
}

macro break [p] {
 common
  IF ?HL.BLOCK='fv'
    jmp ?ENDFV
  ELSE IF ?HL.BLOCK='r'
    jmp ?ENDR
  ELSE IF ?HL.BLOCK='w'
    jmp ?ENDW
  ELSE IF ?HL.BLOCK='l'
    jmp ?ENDL
  END IF
 match =,x, p \{ let x \}
}

macro next [p] {
 common
  IF ?HL.BLOCK='fv'
    jmp ?STARTFV
  ELSE IF ?HL.BLOCK='r'
    jmp ?STARTR
  ELSE IF ?HL.BLOCK='w'
    jmp ?STARTW
  ELSE IF ?HL.BLOCK='l'
    jmp ?STARTL
  END IF
 match =,x, p \{ let x \}
}

macro continue [p] { common next p }

; 2-DO: create one "end" for all HL blocks

;;;;;;;;;;;;;;;;;; NEW LANGUAGE ;;;;;;;;;;;;;;;;;;

; upgrade let with multi-statements.
; unfinished. 2-DO: create one good "let"
; with helper macros

macro let [p] {
 forward
  define ?s 0
  match =0 \
   l:, ?s p \{
   l:
   define ?s 1
  \}
  match =0 \
   =go x, ?s p \{
   jmp x
   define ?s 1
  \}
  match =0 \
   =if x, ?s p \{
   if x
   define ?s 1
  \}
  match =0 \
   =return x, ?s p \{
   return x
   define ?s 1
  \}
  match =0 \
   =return, ?s p \{
   return
   define ?s 1
  \}
  match =0 \
   =while x, ?s p \{
   while x
   define ?s 1
  \}
  match =0 \
   =forever x, ?s p \{
   forever x
   define ?s 1
  \}
  match =0 \
   =loop x, ?s p \{
   loop x
   define ?s 1
  \}
  match =0 \
   =end, ?s p \{
   end
   define ?s 1
  \}
  match =0 \
   =endw, ?s p \{
   endw
   define ?s 1
  \}
  match =0 \
   =endl, ?s p \{
   endl
   define ?s 1
  \}
  match =0 \
   =endfv x, ?s p \{
   endfv
   define ?s 1
  \}
  match =0 \
   =break, ?s p \{
   break
   define ?s 1
  \}
  match =0 \
   =next, ?s p \{
   next
   define ?s 1
  \}
  IF ?s eq 0
   let p
  END IF
}

macro . [p] { common let p }

macro go [p] {
 common
  define ?s 0
  match a=,b, p \{
   jmp a
   let b
   define ?s 1
  \}
  match =0, ?s \{ jmp p \}
}