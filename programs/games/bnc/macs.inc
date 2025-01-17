;************************************
;******** МАКРОСЫ *******************
;************************************
; Constants= colors
;-----------------------цвет:0xRRGGBB
cBLACK     = 0x000000 ; черный
cRED       = 0xFF0000 ; красный
cYELLOW    = 0xFFFF00 ; желтый
cGREEN     = 0x00FF00 ; зеленый
cNAVY      = 0x0000FF ; синий
cMAGENTA   = 0xFF00FF ; малиновый
cBLUE      = 0x00FFFF ; голубой
cWHITE     = 0xFFFFFF ; белый
cBROWN     = 0x554433 ; коричневый
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
