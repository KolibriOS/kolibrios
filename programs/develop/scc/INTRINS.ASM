;
; Small-C Run Time Library for Win NT
;
; Nasm version  17/Nov/98  H T Walheim
; Revised:	20/Nov/98  HTW [Bugs in switch]
;

_CCARGC:
;B+ Ellipses arguments ( ,...)
  ;cl - argument count

  xor  eax,eax
  movzx eax,cl  ; No sign-extension
  ret
;E:.

;B+ Compare

__ult:
;B+ ???
  cmp  eax,ebx
  ja   true
  xor  eax,eax
  ret
;E:.

__ugt:
;B+ ???
  cmp  eax,ebx
  jb   true
  xor  eax,eax
  ret
;E:.

__ule:
;B+ ???
  cmp  eax,ebx
  jae  true
  xor  eax,eax
  ret
;E:.

__uge:
;B+ ???
  cmp  eax,ebx
  jbe  true
  xor  eax,eax
  ret
;E:.

__eq:
;B+ ???
  cmp  eax,ebx
  je   true
  xor  eax,eax
  ret
;E:.

__ne:
;B+ ???
  cmp  eax,ebx
  jne  true
  xor  eax,eax
  ret
;E:.

__lt:
;B+ ???
  cmp  eax,ebx
  jg   true
  xor  eax,eax
  ret
;E:.

__gt:
;B+ ???
  cmp  eax,ebx
  jl   true
  xor  eax,eax
  ret
;E:.

__le:
;B+ ???
  cmp  eax,ebx
  jge  true
  xor  eax,eax
  ret
;E:.

__ge:
;B+ ???
  cmp  eax,ebx
  jle  true
  xor  eax,eax
  ret
;E:.

;E:.

__lneg:
;B+ Logical Negate of Primary
  or   eax,eax
  jnz  false
true:
  mov  eax,1
  ret

false:
  xor  eax,eax
  ret
;E:.

__switch:
;B+ Execute "switch" statement

 ;eax   - switch value
 ;[esp] - pointer to switch table
 ;   dd addr1,value1
 ;         ...
 ;   dd 0
 ;   [jmp default]
 ;   continuation
 ;
 ; Revised: 20/Nov/98 [JECXZ needed]

 pop  ebx
  jmp  skip
back:
  add  ebx,8        ;next case-pair
skip:
  mov  ecx,[ebx]    ;case-label location (adress)
  jecxz default
  cmp  eax,[ebx+4]  ;test case-value
  jnz  back
  jmp  ecx          ;match -- jump to case
default:
  add  ebx,4
  jmp  ebx          ;jump to default/continuation
;E:.
