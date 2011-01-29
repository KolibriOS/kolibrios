;************************************
;******** МАКРОСЫ *******************
;************************************
;------------------------------------
; constants
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

;-------------------------------
cMAGENTAl  = 0xB800B8 ; magenta light
cVIOLETl   = 0xF800F8 ; фиолетовый светл

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
  rb 256
  __end:
  ;org 0x10000
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

macro Subroutines
{
;------------------------------------------------------------------------------
 rndseed:
        mov     eax,26
        mov     ebx,9
        int     0x40
        mov dword[seed],eax
        ret
 rndnext:
        mov eax,3
        int 0x40
        shr eax,16
        add eax,dword[seed]
       ; not eax
        ror eax,3
        xor eax,0xdeadbeef
        mov dword[seed],eax
        ret
;============0========== word[px],word[py]
  o_put:
        mov     ax,word[px]
        shl     eax,16        ;*65536
        add     eax,35        ;+35
        mov     ebx,eax       ;posX*65536+dX       ; [x0] *65536 + [dx]
        mov     ax,word[py]   ;
        shl     eax,16        ;*65536
        add     eax,35        ;+35
        mov     ecx,eax       ;posY*65536+dY       ; [y0] *65536 + [dy]
        mov     edx,cB
        mov     eax,13         ; функция 13  -  box
        int     0x40           ;BLACK BOX---------------

        mov     ax,word[px]
        inc     ax             ;+1
        inc     ax             ;+1
        shl     eax,16         ;*65536
        add     eax,31         ;3         ;+35
        mov     ebx,eax        ;posX*65536+dX     ; [x0] *65536 + [dx]

        mov     ax,word[py]
        add    eax,2
        ;inc     eax
        ;inc     eax
        shl     eax,16
        add     eax,31         ;3       ;-2
        mov     ecx,eax        ;posY*65536+dY     ; [y0] *65536 + [dy]
        mov     edx,cW
        mov     eax,13                      ; функция 13  -  box
        int     0x40           ;WHITE BOX----------
        ret
;============X==========
  x_put:
        ;xor     eax,eax
        mov      ax,word[px]
        shl     eax,16        ;*65536
        add      ax,word[px]
        add     eax,35        ;+35
        mov     ebx,eax       ;posX*65536+dX

        xor     eax,eax
        mov      ax,word[py]
        shl     eax,16        ;*65536
        add      ax,word[py]
        add     eax,35        ;+35
        mov     ecx,eax       ;posX*65536+dX
        mov     edx,cB
        mov     eax,38        ; функция 38  -  line   \
        int     0x40
;------------------------------xx
        xor     eax,eax
        mov     ax,word[px]
        inc     eax
        shl     eax,16        ;*65536
        add      ax,word[px]
        add     eax,36        ;+35+1
        mov     ebx,eax       ;posX*65536+dX
        xor     eax,eax
        mov     ax,word[py]
        shl     eax,16        ;*65536
        add      ax,word[py]
        add     eax,35        ;+35
        mov     ecx,eax       ;posX*65536+dX       ; [x0] *65536 + [dx]
        mov     edx,cB
        mov     eax,38        ; функция 38  -  line    \\
        int     0x40
;---------------------------------
        xor     eax,eax
        mov     ax,word[px]
        shl     eax,16        ;*65536
        mov     ebx,eax       ;posX*65536 =    .BX

        xor     eax,eax
        mov     ax,word[px]
        add     eax,35
        add     ebx,eax       ;---------------------------BX

        xor     eax,eax
        mov     ax,word[py]
        add     eax,35
        shl     eax,16        ;*65536
        add     ax,word[py]   ;+y
        mov     ecx,eax       ;(posY+35)*65536+posY = .CX
        mov     edx,cB        ; цвет : черный
        mov     eax,38        ; функция 38  -  line     /
        int     0x40
;----------------------------------------
        xor     eax,eax
        mov      ax,word[px]
        inc      ax
        shl     eax,16        ;*65536
        mov     ebx,eax       ;posX*65536 =    .BX

        xor     eax,eax
        mov     ax,word[px]
        add     eax,36        ;+35+1
        add     ebx,eax       ;---------------------------BX

        xor     eax,eax
        mov     ax,word[py]
        add     eax,35
        shl     eax,16        ;*65536
        add     ax,word[py]   ;+y
        mov     ecx,eax       ;(posY+35)*65536+posY = .CX
        mov     edx,cB
        mov     eax,38        ; функция 38  -  line      //
        int     0x40
        ret
;------------------------------------ПРОВЕРКА Hа выигрышную ситуацию-----x
 x_win: xor ecx,ecx
        xor ebx,ebx
        xor eax,eax
  lx0:  mov  bx,word[poxl]        ;сравнить текущую ситуацию на поле X
        mov esi,ecx
        shl esi,1                 ;*2
        mov  ax,word[oxwall+esi]  ; next pattern
        and ebx,eax               ;с победными , если хоть одна совпадает -
        sub eax,ebx
         jz  wx                   ;победа  -> set byte xwin=1
        inc ecx
        cmp ecx,8                 ;цикл  ecx < 8
        jb lx0
  g00:                            ;нет совпадений
        ret
   wx:
        inc   byte[xwin]     ;mov   byte[xwin],1
        ret
;------------------------------------ПРОВЕРКА Hа выигрышную ситуацию-----0
 o_win: xor ecx,ecx
        xor ebx,ebx
        xor eax,eax
  lo0:  mov  bx,word[pool]        ;сравнить текущую ситуацию на поле O
        mov esi,ecx
        shl esi,1                 ;*2
        mov  ax,word[oxwall+esi]  ; next pattern
        and ebx,eax               ;с победными , если хоть одна совпадает -
        sub eax,ebx
        jz  wo                    ;победа  -> set byte owin=1
        inc ecx
        cmp ecx,8                 ;цикл
        jb lo0
        ret
   wo:  inc byte[owin]            ;mov   byte[owin],1
        ret
;==================================COmbinations -0- =============
 o_com: xor ecx,ecx
        xor ebx,ebx
        xor eax,eax
 l000:  mov  bx,word[poxl]        ;  текущую ситуацию на поле X
       ;  or  bx,word[pool]        ;+ текущую ситуацию на поле O
        mov esi,ecx
        shl esi,2                 ;*4 -сдвиг -слов
        mov  ax,word[prav+esi]    ; следующее правило
        and ebx,eax               ; если не совпадает -
        sub eax,ebx
        jnz @F                    ;next ecx
         xor eax,eax
         xor ebx,ebx
         mov ax,word[prav+esi+2]  ;esi<-из таблицы правил
         mov bl,byte[pox+eax]
         cmp ebx,0                ;empty?
         je  g00d
   @@:
        inc ecx
        cmp ecx,24                 ;цикл до 24
        jb l000
        ret                       ;нет совпадений  - do nothing
  g00d:
                                  ;применить его
        xor eax,eax
        mov ax,word[prav+esi+2]     ;esi<-из таблицы правил
        mov esi,eax
        mov dword[idx],esi        ;сохраним в idx
        ret

;==================================предвыигрышная комбинация...
 x_com: xor ecx,ecx
        xor ebx,ebx
        xor eax,eax
 l111:  mov  bx,word[pool]        ;  текущую ситуацию на поле O
        mov esi,ecx
        shl esi,2                 ;*4 -сдвиг -слов
        mov  ax,word[prav+esi]    ; следующее правило
        and ebx,eax               ; если не совпадает -
        sub eax,ebx
        jnz @F                    ;next ecx
         xor eax,eax
         xor ebx,ebx
         mov ax,word[prav+esi+2]  ;esi<-из таблицы правил
         mov bl,byte[pox+eax]
         cmp ebx,0                ;empty?
         je  g11d
   @@:
        inc ecx
        cmp ecx,24                ;цикл до 24
        jb l111
        ret                       ;нет совпадений  - do nothing

  g11d:
                                  ;применить его
        xor eax,eax
        mov ax,word[prav+esi+2]   ;esi<-из таблицы правил
        mov esi,eax
        mov dword[idx],esi        ;сохраним в idx
        ret
}

macro playing_field
{
  pole_x0:
        mov     eax,38                 ; func 38
        mov     edx,cB                 ; the color is black

        mov     ebx,92*65536+92        ;x
        mov     ecx,42*65536+192       ;y
        int     0x40                   ;-  line  vert 1
        mov     ebx,142*65536+142      ;x
        mov     ecx,42*65536+192       ;y
        int     0x40                   ;-  line  vert 2
        mov     ebx,42*65536+192       ;x
        mov     ecx,92*65536+92        ;y
        int     0x40                   ;-  line  horiz 1
        mov     ebx,42*65536+192       ;x
        mov     ecx,142*65536+142      ;y
        int     0x40                   ;-  line  horiz 2
}
