; BGI Font Test
;
; Written in pure assembler by Ivushkin Andrey aka Willow
;
; Created: December 20, 2004
;
; Last changed: February 2, 2005
;

BGIFONT_PATH equ '/RD/1/FONTS/'
_X equ 340
_Y equ 240

BGI_WINDOW_CLIP equ 1
BGI_PRC_INFO equ pinfo

macro ListFonts
{
   mov  eax,lf_head
   call PixelPrint
   mov  ebx,20 shl 16+60
   mov  ecx,(BGIfont_names_end-BGIfont_names)/4
   mov  esi,BGIfont_names
 .l1:
   pusha
   pusha
   mcall 4,,0x10777fac,esi,4
   popa
   mov  edx,[esi]
   BGIfont_GetID
   mov  dword[_BGIfont_Prepare.font],edx
   mov  ecx,eax
   add  ebx,140 shl 16
   mov  edx,ebx
   mcall 47,0x80100,,,0x10ffffff
   jecxz .nofont
   lea  ebx,[edx+80 shl 16+12]
   mov  edx,_BGIfont_Prepare.fontfullname
   mov  esi,_BGIfont_Prepare.fontattr-1
   sub  esi,edx
   add  ecx,0x3b800b8
   BGIfont_Outtext
   sub  ebx,155 shl 16+12
   mcall 4,,0x1000f000,load_ok,load_fail-load_ok
   jmp  .pop
 .nofont:
   lea  ebx,[edx-80 shl 16]
   mcall 4,,0x10f00010,load_fail,font_msg-load_fail
 .pop:
   popa
   add  esi,4
   add  ebx,39
   loop .l2
   jmp  .ex
 .l2:
   jmp  .l1
.ex:
}
use32
  org	 0x0

  db	 'MENUET01'
  dd	 0x01
  dd	 START
  dd	 I_END
  dd	 0x100000
  dd	 0x100000
  dd	 0x0
not1strun  dd	 0x0

include 'lang.inc'
include  'macros.inc'
;include   'debug.inc'
include  'bgifont.inc'

START:
    mov  [help],0
    mov  ecx,(BGIfont_names_end-BGIfont_names)/4
    mov  esi,BGIfont_names
    mov  edi,I_END
    BGIfont_Init
reset:
    mov  esi,[BGIfont_Ptr]
    inc  esi
    mov  edi,freeconst
    movsd
    mov  esi,freeconst
    mov  edi,freetest
    mov  ecx,36
    rep  movsb
red:
    mov  eax,[freetest]
    mov  dword[title+19],eax
    call draw_window
    cmp  [not1strun],0
    jnz  still
    mcall 5,300
    not  [not1strun]
    inc  [help]
    jmp  red
still:

    mov  eax,10
    int  0x40

    cmp  eax,1
    je	 red
    cmp  eax,2
    je	 key
    cmp  eax,3
    je	 button

    jmp  still

button:
    mov  eax,17
    int  0x40
    cmp  ah,1
    jnz  noclose
close:
    xor  eax,eax
    dec  eax
    int  0x40
  noclose:
    jmp  still

key:
    mov  eax,2
    int  0x40
    shr  eax,8
    cmp  al,27 ; esc - close
    je   close
    cmp  al,32 ; Space - help
    jne  .noh
    inc  [help]
    cmp  [help],3
    jne  red
    and  [help],0
    jmp  red
  .noh:
    cmp  [help],2
    jne   still
    cmp  al,50
    jb   .noff
    cmp  al,57
    ja   .noff
    sub  al,46
  .gn:
    movzx ecx,al
    shl  ecx,28
    call BGIfont_GetName
    mov  [freetest],edx
    jmp  red
  .noff:
    mov  ecx,4
    mov  edi,Fkeys
  .fkey:
    cmp  al,[edi]
    je   .fndkey
    inc  edi
    loop .fkey
    jmp  .notfnd
  .fndkey:
    lea  eax,[edi-Fkeys+0xc]
    jmp  .gn
  .notfnd:
    cmp  al,114 ; r - reset
    je   reset
    cmp  al,176
    jne  .nol  ; left
    sub  dword[freetest+8],5
    jmp  red
  .nol:
    cmp  al,179
    jne  .nor  ; right
    add  dword[freetest+8],5
    jmp  red
  .nor:
    cmp  al,105 ; i -italic
    jne  .noi
    xor  dword[freetest+32],BGI_ITALIC
    jmp  red
  .noi:
    cmp  al,98 ; b -bold
    jne  .nob
    xor  dword[freetest+32],BGI_BOLD
    jmp  red
  .nob:
    mov  ecx,2
    mov  esi,ali
  .ol2:
    cmp  al,[esi]
    jne  .othl2
    mov  ax,[freetest+32]
    add  ax,[esi+3]
    mov  bx,ax
    mov  dx,[esi+1]
    and  bx,dx
    cmp  bx,dx
    jne  .ok
    not  dx
    and  ax,dx
  .ok:
    mov  [freetest+32],ax
    jmp  red
  .othl2:
    add  esi,5
    loop .ol2
  .other:
    mov  esi,delt
    mov  ecx,4
  .ol:
    cmp  al,[esi]
    jne  .othl
    fld  dword[esi+1]
    movzx eax,byte[esi+5]
    fadd dword[freetest+eax]
    fstp dword[freetest+eax]
    jmp  red
  .othl:
    add  esi,6
    loop .ol
    jmp  still

draw_window:

    mov  eax,48
    mov  ebx,3
    mov  ecx,sc
    mov  edx,sizeof.system_colors
    int  0x40

    mov  eax,12
    mov  ebx,1
    int  0x40

    xor  eax,eax
    mov  ebx,10*65536+_X*2+30
    mov  ecx,100*65536+_Y*2
    mov  edx,0x03261212
    mov  esi,0x805080d0
    mov  edi,0x005080d0
    int  0x40

    mov  eax,4
    mov  ebx,8*65536+8
    mov  ecx,[sc.grab_text]
    or   ecx,0x10000000
    mov  edx,title
    mov  esi,title_end-title
    cmp  [help],2
    je   .int
    sub  esi,12
  .int:
    int  0x40

    cmp  [help],0
    jnz  .help
    cmp  byte[I_END],0
    jnz  .fontsok
    mov  eax,font_msg
    call PixelPrint
  .fontsok:
    ListFonts
    jmp  .eod
  .help:
    cmp  [help],1
    jne  .nohelp
    mov  eax,helptxt
    cmp  byte[I_END],0
    jnz  .fontsok2
    mov  word[eax+2],_X-80
    call PixelPrint
    jmp  .eod
  .fontsok2:
    mov  word[eax+2],_X
    call Print
    jmp  .eod
  .nohelp:
    mov  edi,cross
    mov  eax,38
    mov  edx,0x4b331a
    mov  ebx,[edi]
    mov  ecx,[edi+4]
    int  0x40
    mov  ebx,[edi+8]
    mov  ecx,[edi+12]
    int  0x40
    mov  ebx,freetest
    BGIfont_Freetext
  .eod:
    mov  eax,12
    mov  ebx,2
    int  0x40
    ret

Print:	; eax-pointer to _txt struc
   pusha
   mov	ebx,[eax]
   movzx ecx,byte[eax+12]
   lea	edx,[eax+13]
   mov	edi,eax
 .nextstr:
   mov esi,[edx]
   add	edx,4
   push ecx
   mov	ecx,[edi+8]
   BGIfont_Outtext
   add	 ebx,[edi+4]
   pop	 ecx
   and  esi,0xfff
   add	 edx,esi
   loop .nextstr
   popa
   ret

PixelPrint: ; eax-pointer to _txt struc, but used differently
   pusha
   mov  ebp,eax
   mov  ebx,[ebp]
   movzx ecx,byte[ebp+12]
   mov  eax,4
   lea  edx,[ebp+17]
 .l:
   mov  esi,[edx-4]
   and  esi,0xfff
   push ecx
   mov  ecx,[ebp+8]
   int  0x40
   pop  ecx
   lea  edx,[esi+edx+4]
   add  ebx,[ebp+4]
   loop .l
   popa
   ret

macro _txt _xy,_vert,_color,_opt,[_str]
{
common
   _counter=0
forward
   _counter=_counter+1
common
   dd  _xy	      ; 0
   dd  _vert	    ; 4
   dd  _color	    ; 8
   db  _counter   ;_str_count   ; 12
forward
   local .str_beg,.str_end
   dd  (.str_end-.str_beg) or _opt  ; 13
 .str_beg:
   db  _str		    ;17
 .str_end:
}

title db 'BGIFONT.INC demo - FONT font'
title_end:
_M equ 30
cross dd _M shl 16+_X*2-_M,_Y shl 16+_Y,_X shl 16+_X,_M shl 16+_Y*2-_M
helptxt:
if  lang eq ru
_txt _X shl 16+60,40,0x434ba010,BGI_HACENTER,\
     "ГОРЯЧИЕ КЛАВИШИ:",\
     "Пробел - шрифты/этот текст/демка;",\
     "<-> стрелки - вращение строки;",\
     "V - выравнивание по вертикали;",\
     "H - выравнивание по горизонтали;",\
     "[,] - масштаб по оси X;",\
     "A,Z - масштаб по оси Y;",\
     "B,I - полужирный шрифт и курсив;",\
     "R - сброс параметров шрифта;",\
     "F1..F12 - выбор шрифта;",\
     "Esc - закрыть прогу :-("
alpha:
   db "Вот пример текста!"
lf_head:
_txt 10 shl 16+30,85 shl 16,0x6e00f7,0,"Имя шрифта","Статус","ID",\
     "Путь к файлу"
load_ok:
   db "загружен"
load_fail:
   db "не найден"
font_msg:
   _txt (_X+20) shl 16+180,25,0x10ff0000,0,\
   "К сожалению, не найдено ни одного",\
   "векторного шрифта *.CHR. Возможно,",\
   "вам следует исправить константу",\
   "BGIFONT_PATH в начале файла BGITEST.ASM",\
   "и перекомпилировать его :-("
else
_txt _X shl 16+60,40,0x434ba010, BGI_HACENTER,\
     "HOT KEYS:",\
     "Space - font list/this message/demo;",\
     "<-> arrows - rotate text string;",\
     "V - toggle vertical alignment;",\
     "H - toggle horizontal alignment;",\
     "[,] - scale on X axis;",\
     "A,Z - scale on Y axis;",\
     "B,I - toggle bold & italic mode;",\
     "R - reset font options;",\
     "F1..F12 - select font;",\
     "Esc - close demo :-("
alpha:
   db 'This is a Sample text!'
;    db 'ABCDEFGHIGKLMNOPQRSTUVWXWZ'
lf_head:
_txt 10 shl 16+30,85 shl 16,0x6e00f7,0,"Font name","Status","ID",\
     "Filename"
load_ok:
   db "loaded"
load_fail:
   db "not found"
font_msg:
   _txt (_X+20) shl 16+180,25,0x10ff0000,0,\
   "Sorry, no vector font *.CHR found.",\
   "Maybe you should correct BGIFONT_PATH",\
   "constant at the beginning of file",\
   "BGITEST.ASM and recompile it :-("
end if

macro Deltas _key,_scale,_ofs
{
   db _key
   dd _scale
   db _ofs
}

delt:
Deltas  91, -0.15,12
Deltas  93, 0.15,12
Deltas  97, 0.15,16
Deltas  122,-0.15,16
Fkeys db 48,49,68,255

macro Aligns _key,_mask,_add
{
   db _key
   dw _mask,_add
}
ali:
Aligns 118,BGI_VAMASK,BGI_VATOP
Aligns 104,BGI_HAMASK,BGI_HARIGHT
freeconst BGIfree 'FONT',_X shl 16+_Y,0, 1.0, 1.0, alpha,\
    lf_head-alpha,0xb800e7,\
    BGI_VACENTER or BGI_HACENTER
freetest BGIfree ?,?,?,?,?,?,?,?,?
help db ?
pinfo:
    times 1024 db ?
sc     system_colors
I_END:
