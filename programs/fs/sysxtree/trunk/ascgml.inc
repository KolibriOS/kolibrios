lang equ ru

;
;   Assembler
;     SMALL
;       CODE
;         GaMe
;           Libary
;
;   Ver 0.03 By Pavlushin Evgeni (RUSSIA)
;   www.waptap@mail.ru

;InfoList
;0.01 correct
;0.02 control ~14.05.2004
;0.03 all macros optimized by halyavin, add at ~07.06.2004


;         corectiryemoe,corectnoe,step
macro correct arg1,arg2,arg3
{
local plus,minus,equal
    mov eax,arg2
    cmp arg1,eax
    je  equal
    mov eax,arg3   
    ja  minus
plus:
    add arg1,eax   
    jmp equal
minus:
    sub arg1,eax  
equal:
}

macro control min,max,arg
{
local gr,low,norm
    mov eax,max
    cmp arg,eax
    jg  gr
    mov eax,min
    cmp arg,eax
    jnl norm
gr:
low:
    mov arg,eax
norm:
}
