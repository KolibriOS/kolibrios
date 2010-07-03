;   RTF READER FOR MENUET v1.
;   Written in pure assembler by Ivushkin Andrey aka Willow
;
;---------------------------------------------------------------------
M64 equ 64*1024
N_A equ 0x412f4e
RTFSIZE equ M64
RTFSTACKSIZE equ M64
BGIFONTSIZE equ 120*1024
ESPSIZE equ M64
LMARGIN equ 20
CHARW equ 6
CHARH equ 11
WINW  equ 600
WINH  equ 450
WIN_COLOR equ 0x3f0f0f0
DEFCOLOR equ 0x303030
;RENDER equ PIX
;RENDER equ BGI
RENDER equ FREE

BGIFONT_PATH equ '/sys/fonts/'
FONT_NAME equ 'LITT'
TOP =55
MODE equ RTF
INVALHEX equ 0
RTF_COLORLESS equ 1
RTF_ALIGNLESS equ 2
RTF_NO1STLINE equ 4
RTF_OPENING   equ 8
RTF_HELP      equ 16
RTF_VALID     equ 32
RTF_BLIND     equ 64
RTF_TOEOF     equ 128
RTF_BOTTOM    equ 256
STEPBYSTEP equ 10
DEBUG_BLOCK equ 10
SHOWALIGN equ 10
GUTTER equ 10
BENCH  equ 0;1
syms equ 12

  use32              ; включить 32-битный режим ассемблера
  org    0x0         ; адресация с нуля

  db     'MENUET01'  ; 8-байтный идентификатор MenuetOS
  dd     0x01        ; версия заголовка (всегда 1)
  dd     START       ; адрес первой команды
  dd     I_END0      ; размер программы
  dd     esp_end     ; количество памяти
  dd     sys_mem     ; адрес вершины стэка
  dd     fname_buf   ; адрес буфера для параметров (не используется)
  dd     cur_dir_path         ; зарезервировано

include '..\..\..\macros.inc' ; макросы облегчают жизнь ассемблерщиков!
include '..\..\..\develop\libraries\box_lib\load_lib.mac'

;include 'MACROS.INC'
;include 'load_lib.mac'

@use_library

include 'debug.inc'

if ~ RENDER eq PIX
  TOP=TOP+4
  include 'bgifont.inc'
end if
include 'rtf_lite.inc'
include 'ascl.inc'
;---------------------------------------------------------------------
;---  НАЧАЛО ПРОГРАММЫ  ----------------------------------------------
;---------------------------------------------------------------------
help_file:
file  'reader.rtf'
help_end:

START:
        mcall 68, 11

load_libraries l_libs_start,end_l_libs

;OpenDialog	initialisation
	push    dword OpenDialog_data
	call    [OpenDialog_Init]

    mov  [pitch],2
  if ~ RENDER eq PIX
    mov  edx,FONT_NAME
    mov  edi,save_limit
    BGIfont_Prepare
  end if
 start2:
    cmp  byte[fname_buf],0
    je   load_file;top_red
    jmp  noactivate
 prep_load:
;    mcall 18,3,dword[prcinfo+30]
 noactivate:
;    and  ebp,not RTF_OPENING
;    and  ebp,not RTF_HELP

;    and  [mode],not RTF_OPENING
    and  [mode],not (RTF_HELP+RTF_OPENING)
    mov  ecx,16
    mov  edi,fileinfo.name
    mov  esi,fname_buf
    rep  movsd
 load_file:
        mov     eax, 70
        and     [fileattr+32], 0
        mov     ebx, attrinfo
        mcall
        mov     ebx, [fileattr+32]
        test    eax, eax
        jz      .sizok
        mov     dword [fileinfo.name], N_A
.sizok:
    and  [wSave],0
    mov  [HClick],-100
    mov  eax,ebx
    and  eax,RTFSIZE-1
    add  eax,I_END
    mov  [tail],eax
    shr  ebx,16
;    dpd ebx
    mov  [max_block],ebx
    xor  eax,eax
    mov  ecx,256
    mov  edi,fileinfo.name
    repne scasb
    sub  edi,fileinfo.name+1
    mov  [fname_size],edi
top_red:
    mov  [top],TOP
red:                    ; перерисовать окно

    call draw_window    ; вызываем процедуру отрисовки окна

;---------------------------------------------------------------------
;---  ЦИКЛ ОБРАБОТКИ СОБЫТИЙ  ----------------------------------------
;---------------------------------------------------------------------

still:
    mcall 10            ; функция 10 - ждать события

    cmp  eax,1          ; перерисовать окно ?
    je   red            ; если да - на метку red
    cmp  eax,3          ; нажата кнопка ?
    je   button         ; если да - на button

;---------------------------------------------------------------------


  key:                  ; нажата клавиша на клавиатуре
    mcall 2             ; функция 2 - считать код символа (в ah)
    cmp  ah,104         ; HELP
    jne  .nohelp
  .help:
    xor  [mode],RTF_HELP
    test [mode],RTF_HELP
    jz   load_file
    mov  ecx,help_end-help_file
    mov  [block_end],ecx
    add  [block_end],I_END
    mov  [tail],ecx
    add  [tail],I_END
    mov  esi,help_file
    mov  edi,I_END
    rep  movsb
;    or   [mode],RTF_HELP
    xor  eax,eax
    mov  [max_block],eax
    jmp  top_red
  .nohelp:
;    test [mode],RTF_HELP
;    jz   .nohelp2
;    and  [mode],not RTF_HELP
;    cmp  dword[fileinfo.name],N_A
;    je   still
;    jmp  prep_load
  .nohelp2:
    cmp  ah,114         ; R - redraw
    je   red
    cmp  ah,99          ; C - color
    jne  .nocolor
  .color:
    xor  [mode],RTF_COLORLESS
    jmp  red
  .nocolor:
    cmp  ah,97          ; A - alignment
    jne  .noalign
  .alignment:
    xor  [mode],RTF_ALIGNLESS
    jmp  red
  .noalign:
    cmp  ah,44          ; < - pitch dec
    jne  .nopd
  .decp:
    dec  [pitch]
    jmp  red
  .nopd:
    cmp  ah,46          ; < - pitch inc
    jne  .nopi
  .incp:
    inc  [pitch]
    jmp  red
  .nopi:
    cmp  ah,180         ; Home
    je   top_red
    mov  ebx,dword[prcinfo+46]
    sub  ebx,TOP+15
    cmp  ah,183 ;PgDn
    jne  .nopgdn
    sub  [top],bx
    jmp  red
  .nopgdn:
    cmp  ah,177 ;arrDn
    jne  .noardn
    sub  [top],CHARH
    jmp  red
  .noardn:
    mov  cx,[top]
    cmp  ah,184 ;PgUp
    jne  .nopgup
    add  [top],bx
    cmp  [top],TOP
    jl   red
    mov  [top],TOP
    cmp  cx,[top]
    je   still
    jmp  red
  .nopgup:
    cmp  ah,178 ;arrUp
    jne  .noarup
    add  [top],CHARH
    cmp  [top],TOP
    jl   red
    mov  [top],TOP
    cmp  cx,[top]
    je   still
    jmp  red
  .noarup:
  if  RENDER eq FREE
    cmp  ah,56 ;zoom+
    jne  .noplus
  .zplus:
    fld  [FreeFontscale]
    fmul [Zoomscale]
  .zoom:
    fstp [FreeFontscale]
    jmp  red
  .noplus:
    cmp  ah,54 ;zoom-
    jne  .nominus
  .zminus:
    fld  [FreeFontscale]
    fdiv [Zoomscale]
    jmp  .zoom
  .nominus:
  end if
    cmp  ah,108         ; L - load
    jne  still
  .file_open:
;---------------------------------------------------------------------
;OpenDialog_start:
;	copy_path	open_dialog_name,path,library_path,0
	
	push    dword OpenDialog_data
	call    [OpenDialog_Start]

;	cmp	[OpenDialog_data.status],2 ; OpenDialog does not start
;	je	.sysxtree  ; 	some kind of alternative, instead OpenDialog
	cmp	[OpenDialog_data.status],1
	je	prep_load
	jmp	still
;---------------------------------------------------------------------  
;.sysxtree:
;    or   [mode],RTF_OPENING
;    opendialog draw_window, prep_load, st_1, fname_buf
;  st_1:
;    and  [mode],not RTF_OPENING
;    jmp  still;red
;  stilld:
;    jmp  still
;---------------------------------------------------------------------

  button:
    mcall 17            ; 17 - получить идентификатор нажатой кнопки
    cmp   ah,2
    je    key.help
    cmp   ah,3
    je    key.color
    cmp   ah,4
    je    key.alignment
    cmp   ah,5
    je    key.file_open
    cmp   ah,6
    je    key.incp
    cmp   ah,7
    je    key.decp
  if RENDER eq FREE
    cmp   ah,8
    je    key.zminus
    cmp   ah,9
    je    key.zplus
  end if
    cmp   ah, 1         ; если НЕ нажата кнопка с номером 1,
    jne   .noexit       ;  вернуться

  .exit:
    mcall -1            ; иначе конец программы
  .noexit:
    cmp   ah,20
    jne   still
    mcall 37,1
    and   eax,0xffff
    cmp   eax,[HClick]
    je    still
    mov   [HClick],eax
    sub   eax,25
    mul   [HDoc]
    mov   ebx,dword[prcinfo+46]
    sub   ebx,25
    div   ebx
    dpd   eax
    mov   [top],TOP
    sub   [top],ax
    dps   'B'
    jmp   red;still
;---------------------------------------------------------------------
;---  ОПРЕДЕЛЕНИЕ И ОТРИСОВКА ОКНА  ----------------------------------
;---------------------------------------------------------------------

draw_window:

    mcall 12, 1                    ; функция 12: сообщить ОС об отрисовке окна
                                   ; 1 - начинаем рисовать

    mcall 0, <10,WINW>, <100,WINH>, WIN_COLOR,0x805080D0, 0x005080D0
    mcall 4, <8,8>, 0x10DDEEFF, title, titlesize-title
    mov  esi,ecx
    mcall 47,0x30000,isymImplemented,<114,8>
    add  edx,36 shl 16
    mcall ,,isymMax
    add  edx,40 shl 16
    mov  esi,0x104e00e7;0x10f27840
    cmp  dword[fileinfo.name],N_A
    jne  .noNA
    mov  esi,0x10ff0000
  .noNA:
    mcall 4,edx,esi,fileinfo.name,[fname_size]
    mov  edi,prcinfo
    mcall 9,edi,-1
    and  [mode],not RTF_TOEOF
    mov  ebx,[edi+42]
    cmp  ebx,[wSave]
    je   .nochg
  .chg:
    mov  [wSave],ebx
    or   [mode],RTF_TOEOF
    and  [HDoc],0
    and  [line_count],0
    mov  [HClick],-100
  .nochg:


mcall 13,<5,dword[prcinfo+42]>,<21,21>,0x8000459a

mcall 8,7*65536+39,23*65536+16,5,0x459a           ;load
mcall 8,46*65536+57,23*65536+16,4,0x459a ;align
mcall 8,103*65536+44,23*65536+16,3,0x459a   ;color
mcall 8,147*65536+37,23*65536+16,2,0x459a        ;help

mcall 8,184*65536+44,23*65536+16,7,0x459a ;prev
mcall 8,229*65536+46,23*65536+16,6,0x459a      ;next

if RENDER eq FREE
mcall 8,276*65536+44,23*65536+16,9,0x459a ;zoom+
mcall 8,320*65536+44,23*65536+16,8,0x459a ;zoom-
end if




;    sub  ebx,60
;    shl  ebx,16
;    mov  bx,12
;    mov  ecx,5 shl 16+12
;    mov  esi,0xb810e7
;    mov  edx,2
; BTN_SPACE equ 14 shl 16
;    mcall 8             ;2
;    sub  ebx,BTN_SPACE
;    inc  edx
;    mcall 8,,,,0x459a    ;3
;    sub  ebx,BTN_SPACE
;    inc  edx
;    mcall ,,,,0x107a30  ;4
;    sub  ebx,BTN_SPACE
;    inc  edx
;    mcall ,,,,0xcc0000  ;5
;    sub  ebx,BTN_SPACE
;    inc  edx
;    mcall ,,,,0x575f8c  ;6
;    sub  ebx,BTN_SPACE
;    inc  edx
;    mcall ,,,,0x575f8c  ;7
;  if RENDER eq FREE
;    sub  ebx,BTN_SPACE
;    inc  edx
;    mcall ,,,,0x6a73d0  ;8
;    sub  ebx,BTN_SPACE
;    inc  edx
;    mcall ,,,,0xd048c8  ;9
;  end if
;text
;    shr  ecx,16
;    mov  bx,cx
;    add  ebx,3 shl 16+3
    mcall 4,10*65536+27,0x10ddeeff,btn_text,btn_end-btn_text

    sub  dword[prcinfo+42],LMARGIN
    sub  dword[prcinfo+46],CHARH
 if GUTTER eq 1
    mov  ebx,LMARGIN shl 16+20
    mov  ecx,20
    mov  eax,4
    mov  edx,arrow
    mov  esi,1
  .loop1:
    push ecx
    mcall ,,0xff0000
    pop  ecx
    add  ebx,50 shl 16
    loop .loop1
 end if
    mov  ebx,dword[prcinfo+42]
    shl  ebx,16
    add  ebx,2 shl 16+13
    mov  ecx,dword[prcinfo+46]
    add  ecx,40 shl 16-35
    mov  edx,20+1 shl 29
    mcall 8
    mov  ecx,[HClick]
    shl  ecx,16
    add  ecx,6-3 shl 16 + 7
    mcall 13,,,0xe26830
 if MODE eq RTF
    test [mode],RTF_OPENING
    jne  .ex
    and  [mode],not (RTF_BOTTOM);+RTF_TOEOF)
    mov  [colorptr],colortbl
    mov  eax,DEFCOLOR
    mov  edi,colortbl
    mov  ecx,16
    rep  stosd
    xor  eax,eax
    mov  [cGroup],eax
    mov  edi,Chp
    mov  ecx,SIZE_save
    rep  stosb
    mov  ax,[top]
    mov  word[Free+6],10
    mov  word[Free+4],ax
    mov  esi,I_END
    call RtfParse
    dpd  eax
    dps  'Lines='
    mov  eax,[line_count]
    dpd  eax
    newline
;    movzx  eax,word[Free+4]
;    dpd  eax
    mov  eax,dword[prcinfo+42]
    mov  edx,WIN_COLOR
    call draw_progress
if BENCH eq 1
    mcall 26,9
    sub  eax,[bench]
    dps  <13,10,'Bench='>
    dpd  eax
end if
 else
    mov  [char],0
    mov  ebx,10 shl 16+TOP
    mov  ecx,16
  .l0:
    push ecx
    mov  ecx,16
  .l1:
    push ecx
  if RENDER eq BGI
    mov  edx,char
    mov  ecx,0x48000000
    mov  esi,1
    BGIfont_Outtext
  else
    mcall 4,,0x10000000,char,1
  end if
    pop  ecx
    inc  [char]
    add  ebx,(CHARW+3) shl 16
    loop .l1
    pop  ecx
    add  ebx,CHARH+2
    and  ebx,0x0000ffff
    add  ebx,10 shl 16
    loop .l0
 end if
 .ex:
    mcall 12, 2                    ; функция 12: сообщить ОС об отрисовке окна
                                   ; 2, закончили рисовать
    ret                            ; выходим из процедуры

if GUTTER eq 1
   arrow db 0x19
end if
;---------------------------------------------------------------------
;---  ДАННЫЕ ПРОГРАММЫ  ----------------------------------------------
;---------------------------------------------------------------------

; интерфейс программы многоязычный
;  Вы можете задать язык в MACROS.INC (lang fix язык)

title:
  db 'RTF Reader v1.    (     ):'
titlesize:
btn_text:
    db 'Load Aligment Color Help    <      >'
if RENDER eq FREE
    db '    Zoom+ Zoom- '
  end if 
btn_end:
;---------------------------------------------------------------------
l_libs_start:

library01  l_libs system_dir_ProcLib+9, cur_dir_path, library_path, system_dir_ProcLib, \
err_message_found_lib2, head_f_l, ProcLib_import, err_message_import2, head_f_i

end_l_libs:
;---------------------------------------------------------------------
system_dir_ProcLib			db '/sys/lib/proc_lib.obj',0

head_f_i:
head_f_l	db 'error',0

err_message_found_lib2		db 'proc_lib.obj - Not found!',0

err_message_import2			db 'proc_lib.obj - Wrong import!',0

;---------------------------------------------------------------------
align 4
ProcLib_import:
OpenDialog_Init		dd aOpenDialog_Init
OpenDialog_Start	dd aOpenDialog_Start
;OpenDialog__Version	dd aOpenDialog_Version
        dd      0
        dd      0
aOpenDialog_Init	db 'OpenDialog_init',0
aOpenDialog_Start	db 'OpenDialog_start',0
;aOpenDialog_Version	db 'Version_OpenDialog',0
;---------------------------------------------------------------------
OpenDialog_data:
.type			dd 0
.procinfo		dd procinfo ;+4
.com_area_name		dd communication_area_name ;+8
.com_area		dd 0 ;+12
.opendir_pach		dd temp_dir_pach ;+16
.dir_default_pach	dd communication_area_default_pach ;+20
.start_path		dd open_dialog_path ;+24
.draw_window		dd draw_window ;+28
.status			dd 0 ;+32
.openfile_pach		dd fname_buf ;+36
.filename_area		dd 0	;+40
.filter_area		dd Filter

communication_area_name:
	db 'FFFFFFFF_open_dialog',0
open_dialog_path:
	db '/sys/File Managers/opendial',0
communication_area_default_pach:
	db '/rd/1',0

Filter:
dd Filter.end - Filter
.1:
db 'RTF',0
.end:
db 0
;---------------------------------------------------------------------
attrinfo:
        dd      5
        dd      0
        dd      0
        dd      0
        dd      fileattr
        db      0
        dd      fileinfo.name

fileinfo:
  dd 0
.block:
  dd 0
  dd 0
.size  dd 1
  dd I_END
.name:
;   db '//'
;  db ' /HD/1/RTF/texts/FASM.TXT',0
;  db '/HD/1/RTF/TEST2.RTF',0
;  db '/HD/1/RTF/texts/index_ru.RTF',0
;  db '/HD/1/RTF/texts/stas.RTF',0
;  db '/HD/1/RTF/texts/zed.RTF',0
;  db '/HD/1/RTF/PRACT.RTF',0
;  db '/HD/1/RTF/SETUP2.RTF',0
;  db '/HD/1/RTF/texts/TC_RU.RTF',0
;  db '/HD/1/RTF/texts/WORD.RTF',0
;  db '/HD/1/RTF/texts/WORD97.RTF',0
;  db '/HD/1/RTF/texts/MASTAPP.RTF',0
;  db '/HD/1/RTF/texts/1c_tor77.RTF',0
;  db '/HD/1/RTF/texts/RELATION.RTF',0
;  db '/HD/1/RTF/texts/PLANETS.RTF',0
;  db '/HD/1/RTF/texts/LOTRRUS.RTF',0
;  db '/HD/1/RTF/texts/RULEBOOK.RTF',0
;  db '/HD/1/RTF/texts/RULEBK2.RTF',0
;  db '/HD/1/RTF/texts/GLEB.RTF',0
;  db '/HD/1/RTF/texts/DWG13_14.RTF',0
;  db '/HD/1/RTF/texts/LK.RTF',0

;  db '/HD/1/RTF/texts/JUSTIFY.RTF',0
   rb  256-($-.name)
;---------------------------------------------------------------------
;blind db ?
if RENDER eq PIX
;  rd 2
  Free rd 9
else
if RENDER eq BGI
  FreeFontscale dd 0.07
else
  Zoomscale dd 1.15
  FreeFontscale dd 0.04
end if
  Free BGIfree FONT_NAME,0,0,1.0,1.0,char,1,0x44000000,0
end if
I_END0:
fname_buf:
        rb      1024+16
fileattr rd 40/4
if BENCH eq 1
  bench dd ?
end if
tail dd ?
cGroup dd ?
Chp:
  CHP
Pap:
  PAP
Sep:
  SEP
Dop:
  DOP
rds db ?
ris db ?
cbBin dd ?
lParam dd ?
fSkipDestIfUnk db ?
mode dd ?
curheight dw ?
maxheight dw ?
RetroBlock dd ?
RetroSave:
  SAVE
prcinfo rb 1024
RetroPtr dd ?
colorptr dd ?
colortbl rd 16
ct_end:
fname_size dd ?
max_block dd ?
cur_block dd ?
HDoc dd ?
HClick dd ?
top dw ?
line_count dd ?
par_count  dd ?
char db ?
pitch db ?
wSave dd ?
RetroXY dd ?
RetroGroup dd ?

save_stack:
rb RTFSTACKSIZE
save_limit:
rb BGIFONTSIZE

listptr dd ?
szKeyword rb 31
szParameter rb 21
block_end dd ?
I_END:                             ; метка конца программы

procinfo process_information
rb RTFSIZE
esp1:
rb ESPSIZE
sys_mem:
rb ESPSIZE
;---------------------------------------------------------------------
temp_dir_pach:
        rb 4096
cur_dir_path:
	rb 4096
library_path:
	rb 4096
;---------------------------------------------------------------------
esp_end:
