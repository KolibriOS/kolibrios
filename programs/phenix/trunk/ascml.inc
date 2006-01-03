lang equ ru

;
;   Assembler
;     SMALL
;       CODE
;         Massive operation
;           Libary
;
;   Ver 0.1 By Pavlushin Evgeni (RUSSIA)
;   www.waptap@mail.ru

;InfoList
;0.01 readmas,compmas,findmas

macro readmas masoff,obroff
{
local loo
    mov edi,masoff   ;tekoff
    add edi,8
    mov ebp,[masoff]   ;elements
    mov edx,[masoff+4] ;elemsize
    mov eax,0 ;count
loo:
    pushad
    call obroff
    popad
    add  edi,edx
    inc  eax
    cmp  eax,ebp
    jne  loo
}


macro compmas masoff1,masoff2,obroff
{
local loo,loo2
    mov esi,masoff2
    add esi,8
    mov ecx,[masoff2]
    mov ebx,[masoff2+4]
    mov eax,0

loo2:
    push eax

    mov edi,masoff1   ;tekoff
    add edi,8
    mov ebp,[masoff1]   ;elements1
    mov edx,[masoff1+4] ;elemsize1
    mov eax,0 ;count
loo:
    pushad
    call obroff
    popad
    add  edi,edx
    inc  eax
    cmp  eax,ebp
    jne  loo

    add  esi,ebx
    pop  eax
    inc  eax
    cmp  eax,ecx
    jne  loo2
}


macro findmas masoff,obroff
{
local loo,looend,lend
    mov edi,masoff   ;tekoff
    add edi,8
    mov ebp,[masoff]   ;elements
    mov edx,[masoff+4] ;elemsize
    mov eax,0 ;count
loo:
    pushad
    mov  eax,0
    call obroff
    cmp  eax,1
    je   looend
    popad
    add  edi,edx
    inc  eax
    cmp  eax,ebp
    jne  loo
    stc
    jmp  lend
looend:
    popad
    clc
lend:
}