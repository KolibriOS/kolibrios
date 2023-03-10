macro BeginProgram
 {
  use32
  org 0x0

  db 'MENUET01'
  dd 0x01
  dd __start
  dd __end
  dd 0x10000   ;__memory
  dd 0x10000   ;__stack
  dd 0x0       ; dd __params
  dd 0x0
  __start:
 }

macro EndProgram
 {
  align 32
  rb 2048
  __end:
  ;org 0x10000
  __stack:
  __memory:
 }

macro  mButton nn,crgb,x,y,dx,dy
{
        mov     eax,8               ; ?㭪??? 8: ??।????? ? ?뢥??? ??????
        mov     ebx,x*65536+dx      ; [x ??砫????] *65536 + [x ࠧ???]
        mov     ecx,y*65536+dy      ; [y ??砫????] *65536 + [y ࠧ???]
        mov     edx,nn              ; ??????䨪???? ??????  =1
        mov     esi,crgb            ;0x116611 ; 梥? ?????? RRGGBB
        int     0x40
}

macro LoopBegin  lab,begi
{
        push  ecx                        ; ECX->STACK store
        mov   ecx,begi
  lab:
        push  ecx                        ; ECX->STACK counter
}

macro LoopEnd lab,endi
{
        pop   ecx                        ;ECX<-STACK counter
        inc   ecx
        cmp   ecx,endi
        jne   lab
        pop   ecx                        ;ECX<-STACK restore
}



;------------------------------------
; constants
;-----------------------梥?:0xRRGGBB
cBLACK     = 0x000000 ; ??????
cRED       = 0xFF0000 ; ???????
cYELLOW    = 0x3FFFF00 ; ??????
cGREEN     = 0x00FF00 ; ???????
cNAVY      = 0x0000FF ; ᨭ??
cMAGENTA   = 0xFF00FF ; ?????????
cBLUE      = 0x00FFFF ; ???㡮?
cWHITE     = 0xFFFFFF ; ?????
cBROWN     = 0x554433 ;

;-------------------------------
cMAGENTAl   = 0xB800B8 ; magenta
cVIOLETl    = 0xF800F8 ; 䨮??⮢??

