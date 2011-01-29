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
        mov     eax,8               ; функция 8: определить и вывести кнопку
        mov     ebx,x*65536+dx      ; [x начальный] *65536 + [x размер]
        mov     ecx,y*65536+dy      ; [y начальный] *65536 + [y размер]
        mov     edx,nn              ; идентификатор кнопки  =1
        mov     esi,crgb            ;0x116611 ; цвет кнопки RRGGBB
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
;-----------------------цвет:0xRRGGBB
cBLACK     = 0x000000 ; черный
cRED       = 0xFF0000 ; красный
cYELLOW    = 0x3FFFF00 ; желтый
cGREEN     = 0x00FF00 ; зеленый
cNAVY      = 0x0000FF ; синий
cMAGENTA   = 0xFF00FF ; малиновый
cBLUE      = 0x00FFFF ; голубой
cWHITE     = 0xFFFFFF ; белый
cBROWN     = 0x554433 ;

;-------------------------------
cMAGENTAl   = 0xB800B8 ; magenta
cVIOLETl    = 0xF800F8 ; фиолетовый

