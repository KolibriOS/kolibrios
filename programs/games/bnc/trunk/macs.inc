;************************************
;******** ??????? *******************
;************************************
; Constants= colors
;-----------------------梥?:0xRRGGBB
cBLACK     = 0x000000 ; ??????
cRED       = 0xFF0000 ; ???????
cYELLOW    = 0xFFFF00 ; ??????
cGREEN     = 0x00FF00 ; ???????
cNAVY      = 0x0000FF ; ᨭ??
cMAGENTA   = 0xFF00FF ; ?????????
cBLUE      = 0x00FFFF ; ???㡮?
cWHITE     = 0xFFFFFF ; ?????
cBROWN     = 0x554433 ; ????筥???
cB     equ cBLACK
cW     equ cWHITE

macro BeginProgram
 {
  use32
  org 0x0

  db 'MENUET01'
  dd 0x01
  dd __start
  dd __end
  dd __memory  ;   0x10000
  dd __stack   ;   0x10000
  dd 0x0       ; dd __params
  dd 0x0
  __start:
 }

macro EndProgram
 {
  align 32
  rb 256   ;512
  __end:
  __stack:
  __memory:
 }


macro LoopBegin  lab,begi
{
        push  ecx                        ; ECX->STACK store  old
        mov   ecx,begi
  lab:
        push  ecx                        ; ECX->STACK counter store
}

macro LoopEnd lab,endi
{
        pop   ecx                        ;ECX<-STACK counter restore
        inc   ecx
        cmp   ecx,endi
        jne   lab
        pop   ecx                        ;ECX<-STACK restore  old
}

macro SetEventMask mmm
{
        mov eax,40
        mov ebx,mmm
        int 0x40
}

macro  redraw_begin
{
        mov     eax,12
        mov     ebx,1
        int     0x40
}

macro  redraw_end
{
        mov     eax,12
        mov     ebx,2
        int     0x40
}

macro  EventIs  eee,lll
{
        cmp     eax,eee
        je      lll
}
