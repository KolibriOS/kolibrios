lang equ ru

;
;   Assembler
;     SMALL
;       CODE
;         GaMe
;           Libary
;
;   Ver 0.05
;

; game_collision_2d - get collision of two 2d rectangles
; result of collision test placed in CF.
; if CF = 1, objects is collided
_1dbounce_count=0;
macro game_collision_2d xy1,wh1,xy2,wh2  ;img1_off,x1,y1,img2_off,x2,y2 ;,otv
{
    movt eax,xy1
    movt ebx,wh1
    movt ecx,xy2
    movt edx,wh2
    call game_collision_proc

if ~ defined game_collision_used
game_collision_used equ 1
   jmp exit
   ; eax = x1*65536+y1
   ; ebx = w1*65536+h1
   ; ecx = x2*65536+y2
   ; edx = w2*65536+h2
game_collision_proc:
   ; y h test
   call _1dbounce
   jnc @f
   ; x w test
   shr eax,16 ;eax,y1 ;
   shr ebx,16 ;mov ebx,[img1_off+4] ;h1
   shr ecx,16 ;mov ecx,y2 ;
   shr edx,16 ;mov edx,[img2_off+4] ;h2
   call _1dbounce
@@:
   ret
; ax - x1, bx - w1, cx - x2, dx - w2
; or
; ax - y1, bx - h1, cx - y2, dx - h2
; if collision ecx is incremented
_1dbounce:
   cmp cx,ax   ; if x2 < x1 jmp anot
   jb  anot
   add ax,bx
   cmp ax,cx   ; if x1+xs <= x2 not coll
   jbe not_coll
coll:
   stc           ; CF = 1
   ret
anot:
   add cx,dx
   cmp cx,ax   ; x2 + xs2 > x1
   ja  coll
not_coll:
   clc           ; CF = 0
   ret
exit:

end if
}

;  approxto
macro approxto value,target_value,step
{
local plus,minus,equal
    mov eax,target_value
    cmp value,eax
    je  equal
    mov eax,step
    ja  minus
plus:
    add value,eax
    jmp equal
minus:
    sub value,eax
equal:
}

macro clamp min,max,arg
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
