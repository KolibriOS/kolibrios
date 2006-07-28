;-------------------------
;
; ASM JJ's 2 C macros ;-)
;
; version history:
;
; *0.01(02/08/05): if_e, if_ne, if_a, if_b
; *0.02(02/09/05): push_abc, pop_abc
;                  ch_sign, abs
;
; E.X.: if_e dword [x],100,.end_if
;       if x = 100 do the instrucsions to .end_if:
; ==
macro if_e arg1,arg2,label
{
if arg1 eq 0 | arg2 eq 0 | arg1 eqtype 12 | arg2 eqtype 12
    cmp  arg1,arg2
    jne label
else
    push edx
    mov  edx,arg1
    cmp  edx,arg2
    pop  edx
    jne label
end if
}

; !=
macro if_ne arg1,arg2,label
{
if arg1 eq 0 | arg2 eq 0 | arg1 eqtype 12 | arg2 eqtype 12
    cmp  arg1,arg2
    je label
else
    push edx
    mov  edx,arg1
    cmp  edx,arg2
    pop  edx
    je label
end if
}

; >
macro if_a arg1,arg2,label
{
if arg1 eq 0 | arg2 eq 0 | arg1 eqtype 12 | arg2 eqtype 12
    cmp  arg1,arg2
    jbe label
else
    push edx
    mov  edx,arg1
    cmp  edx,arg2
    pop  edx
    jbe label
end if
}

; <
macro if_b arg1,arg2,label
{
if arg1 eq 0 | arg2 eq 0 | arg1 eqtype 12 | arg2 eqtype 12
    cmp  arg1,arg2
    jae label
else
    push edx
    mov  edx,arg1
    cmp  edx,arg2
    pop  edx
    jae label
end if
}

macro push_abc
{
    push eax
    push ebx
    push ecx
    push edx
    push edi
    push esi
}

macro pop_abc
{
    pop esi
    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
}

UNDER_0 equ 0x7FFFFFFF
; changes sign e.x. ch_sign -1 = 1
macro ch_sign arg
{
    push edx
    mov  edx,0
    sub  edx,arg
    pop  edx
}

; absolut  e.x. abs -24 = 24
macro abs arg
{
local .end_if
    if_a arg,UNDER_0,.end_if
       ch_sign arg
    .end_if:
}
  ;---------------------------------------




