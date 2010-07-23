;   RTF READER FOR KOLIBRI >= 0.7.7.0
;   Written in pure assembler by Ivushkin Andrey aka Willow
;   Menu_bar and scroll_bar from box_lib provided by dunkaist
;---------------------------------------------------------------------
M64 equ 64*1024
N_A equ 0x412f4e
RTFSIZE equ M64
RTFSTACKSIZE equ M64
BGIFONTSIZE equ 120*1024
ESPSIZE equ M64
LMARGIN equ 15
CHARW equ 6
CHARH equ 11
WINW  equ 600
WINH  equ 450
WIN_COLOR equ 0x33f0f0f0;0x3f0f0f0
DEFCOLOR equ 0x303030
;RENDER equ PIX
;RENDER equ BGI
RENDER equ FREE

BGIFONT_PATH equ '/sys/fonts/'
FONT_NAME equ 'LITT'
TOP = 45
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

;-------------------------------

scroll_width_size       equ     15
AR_OFFSET               equ     10

;-------------------------------

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

include '../../../macros.inc' ; макросы облегчают жизнь ассемблерщиков!
include '../../../develop/libraries/box_lib/trunk/box_lib.mac'
include '../../../develop/libraries/box_lib/load_lib.mac'

;include 'MACROS.INC'
;include 'load_lib.mac'

@use_library

include 'debug.inc'

if ~ RENDER eq PIX
  TOP=TOP+4
  include 'bgifont.inc'
end if
include 'rtf_lite.inc'
;include 'ascl.inc'
;---------------------------------------------------------------------
;---  НАЧАЛО ПРОГРАММЫ  ----------------------------------------------
;---------------------------------------------------------------------
help_file:
    file  'reader.rtf'
help_end:

START:
        mcall 68, 11
        mcall 40, 0x27

load_libraries l_libs_start,end_l_libs

;OpenDialog     initialisation
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
    mov  [is_scroll_bar_needed],    0
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
;    mov  [HClick],-100
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
    mov  [scroll_bar_data_vertical.position],0
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
    cmp  eax,6
    je   mouse

;---------------------------------------------------------------------


  key:                  ; нажата клавиша на клавиатуре
    mcall 2             ; функция 2 - считать код символа (в ah)
    cmp  ah,104         ; HELP
    jne  .nohelp
  .help:
    mov  [is_scroll_bar_needed],    0
    xor  [mode],RTF_HELP
    test [mode],RTF_HELP
    jz   load_file
    mov  dword[HDoc],200                        ; it makes the help page not scroll
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
    
    cmp  dword[is_scroll_bar_needed], 0
     je  still
    
    mov  ebx,dword[prcinfo+46]
    sub  ebx,TOP+15
    cmp  ah,183 ;PgDn
    jne  .nopgdn
;    sub  [top],bx

    cmp  dword[is_scroll_bar_needed], 0
     je  still
    
    mov  eax, [scroll_bar_data_vertical.position]
    add  eax, AR_OFFSET*7
    mov  ebx, [scroll_bar_data_vertical.max_area]
    sub  ebx, [scroll_bar_data_vertical.cur_area]
    cmp  eax, ebx
    mov  dword[scroll_bar_data_vertical.position], eax
    jl  @f
    mov  dword[scroll_bar_data_vertical.position], ebx
  @@:
    call Set_position
    jmp  red
  .nopgdn:
    cmp  ah,177 ;arrDn
    jne  .noardn
;    sub  [top],CHARH

    cmp  dword[is_scroll_bar_needed], 0
     je  still
    
    mov  eax, [scroll_bar_data_vertical.position]
    add  eax, AR_OFFSET
    mov  ebx, [scroll_bar_data_vertical.max_area]
    sub  ebx, [scroll_bar_data_vertical.cur_area]
    cmp  eax, ebx
    mov  dword[scroll_bar_data_vertical.position], eax
    jl  @f
    mov  dword[scroll_bar_data_vertical.position], ebx
  @@:
    call Set_position

    jmp  red
  .noardn:
    mov  cx,[top]
    cmp  ah,184 ;PgUp
    jne  .nopgup
;    add  [top],bx
;    cmp  [top],TOP
;    jl   red
;    mov  [top],TOP
;    cmp  cx,[top]
;    je   still

    cmp  dword[is_scroll_bar_needed], 0
     je  still
    
    cmp  dword[scroll_bar_data_vertical.position], AR_OFFSET*7
    sub  dword[scroll_bar_data_vertical.position], AR_OFFSET*7
    jg  @f
    mov  dword[scroll_bar_data_vertical.position], 0
  @@:
    call Set_position
    jmp  red
  .nopgup:
    cmp  ah,178 ;arrUp
    jne  .noarup
;    add  [top],CHARH

    cmp  dword[is_scroll_bar_needed], 0
     je  still
    
    cmp  dword[scroll_bar_data_vertical.position], AR_OFFSET
    sub  dword[scroll_bar_data_vertical.position], AR_OFFSET
    jg  @f
    mov  dword[scroll_bar_data_vertical.position], 0
  @@:
    call Set_position

;    cmp  [top],TOP
;    jl   red
;    mov  [top],TOP
;    cmp  cx,[top]
;    je   still
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
    cmp  ah,0xB5        ; end
    jne  .pre_file_open
  .end:

    cmp  dword[is_scroll_bar_needed], 0
     je  still
    
    mov  eax, [scroll_bar_data_vertical.max_area]
    sub  eax, [scroll_bar_data_vertical.cur_area]
    mov  dword[scroll_bar_data_vertical.position], eax
    call Set_position
    jmp  red
  .pre_file_open:
    cmp  ah,108         ; L - load
    jne  still
  .file_open:
;---------------------------------------------------------------------
;OpenDialog_start:
;       copy_path       open_dialog_name,path,library_path,0
        
        push    dword OpenDialog_data
        call    [OpenDialog_Start]

;       cmp     [OpenDialog_data.status],2 ; OpenDialog does not start
;       je      .sysxtree  ;    some kind of alternative, instead OpenDialog
        cmp     [OpenDialog_data.status],1
        je      prep_load
        jmp     still
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

    cmp   ah, 1         ; если нажата кнопка с номером 1,
    je    .exit
    jmp still
    
  .exit:
    mcall -1            ; иначе конец программы
    

;---------------------------------------------------------------------
;---  MOUSE EVENT PROCESSING  ----------------------------------------
;---------------------------------------------------------------------    
mouse:
        mcall   37,7
        test    eax,    eax
        je      .menu_bar_1;.mouse
        jmp     still


.menu_bar_1:
        call    .set_mouse_flag
@@:
        push    dword menu_data_1       ;mouse event for Menu 1
        call    [menu_bar_mouse]
        cmp     [menu_data_1.click],dword 1
        jne     .menu_bar_2
        cmp     [menu_data_1.cursor_out],dword 0
        jne     .analyse_out_menu_1
        jmp     .menu_bar_1
.menu_bar_2:
        push    dword menu_data_2
        call    [menu_bar_mouse]
        cmp     [menu_data_2.click],dword 1
        jne     .menu_bar_3
        cmp     [menu_data_2.cursor_out],dword 0
        jne     .analyse_out_menu_2
        jmp     .menu_bar_1
.menu_bar_3:
        push    dword menu_data_3
        call    [menu_bar_mouse]
        cmp     [menu_data_3.click],dword 1
        jne     .scroll_bar
        cmp     [menu_data_3.cursor_out],dword 0
        jne     .analyse_out_menu_3
        jmp     .menu_bar_1

.set_mouse_flag:
        xor     eax,eax
        inc     eax
        mov     [menu_data_1.get_mouse_flag],eax
        mov     [menu_data_2.get_mouse_flag],eax
        mov     [menu_data_3.get_mouse_flag],eax
        ret

.analyse_out_menu_1:
        cmp     [menu_data_1.cursor_out],dword 1
        je      key.file_open
        cmp     [menu_data_1.cursor_out],dword 2
        je      button.exit
        jmp     red

.analyse_out_menu_2:
        cmp     [menu_data_2.cursor_out],dword 1
        je      key.zplus
        cmp     [menu_data_2.cursor_out],dword 2
        je      key.zminus
        cmp     [menu_data_2.cursor_out],dword 3
        je      key.incp
        cmp     [menu_data_2.cursor_out],dword 4
        je      key.decp
        cmp     [menu_data_2.cursor_out],dword 5
        je      key.alignment
        cmp     [menu_data_2.cursor_out],dword 6
        je      key.color
        jmp     red

.analyse_out_menu_3:
        cmp     [menu_data_3.cursor_out],dword 1
        je      key.help
        jmp     red

.scroll_bar:
        cmp     dword[is_scroll_bar_needed], 0
        je      still
.vertical:
        mov     eax,[scroll_bar_data_vertical.max_area]
        cmp     eax,[scroll_bar_data_vertical.cur_area]
        jbe     still
; mouse event for Vertical ScrollBar

        push    dword scroll_bar_data_vertical
        call    [scrollbar_ver_mouse]
  
        call    Set_position
        
        mov     eax,scroll_bar_data_vertical.redraw
        xor     ebx,ebx
        cmp     [eax],ebx
        je      @f
        mov     [eax],ebx
        jmp     red
@@:
        cmp     [scroll_bar_data_vertical.delta2],0
        jne     still
.other:
        jmp     still
;---------------------------------------------------------------------
;---  ОПРЕДЕЛЕНИЕ И ОТРИСОВКА ОКНА  ----------------------------------
;---------------------------------------------------------------------

draw_window:

    mcall 9, procinfo2, -1
    mov  edx, -1
    mov  esi, -1
    
    mov  eax, [procinfo2.box.width]
    cmp  eax, 140
     je  @f
    mov  [is_scroll_bar_needed],    0
     jg  @f
    mov  edx, 140
  @@:

    mov  eax, [procinfo2.box.height]
    cmp  eax, 80
     je  @f
    mov  [is_scroll_bar_needed],    0
     jg  @f
    mov  esi, 80
  @@:

    mcall 67, -1, -1

    mcall 12, 1
;    mcall 0, <10,WINW>, <100,WINH>, WIN_COLOR,0x805080D0, 0x005080D0
;    mcall 4, <8,8>, 0x10DDEEFF, title, titlesize-title
    mcall 0, <10,WINW>, <100,WINH>, WIN_COLOR,0x80000000, window_title
;---------------------------------------------
    cmp  [is_scroll_bar_needed],    0
     je  @f
    call Set_scroll_position
        xor     eax,eax
        inc     eax
        mov     [scroll_bar_data_vertical.all_redraw],eax
; draw for Vertical ScrollBar
        push    dword scroll_bar_data_vertical
        call    [scrollbar_ver_draw]
; reset all_redraw flag 
        xor     eax,eax
        mov     [scroll_bar_data_vertical.all_redraw],eax
  @@:
;---------------------------------------------
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
;    mcall 4,edx,esi,fileinfo.name,[fname_size]
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
;    mov  [HClick],-100
  .nochg:

;---------------------------------------------
    call  Set_scroll_position

    mov ebx, dword[prcinfo+0x3E]
    mcall     38, , 65536*18+18, 0x8b8b89
    inc ebx
    mcall     13, , 65536*0+18, 0xe9e9e2
;---------------------------------------------
; draw for Menu 1
        push    dword menu_data_1
        call    [menu_bar_draw] 
; draw for Menu 2
        push    dword menu_data_2
        call    [menu_bar_draw] 
; draw for Menu 3
        push    dword menu_data_3
        call    [menu_bar_draw]         
;---------------------------------------------

    sub  dword[prcinfo+42],2*LMARGIN+scroll_width_size
    sub  dword[prcinfo+46],CHARH+25
    
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
;    dpd  eax
;    dps  'Lines='
    mov  eax,[line_count]
;    dpd  eax
;    newline
;    movzx  eax,word[Free+4]
;    dpd  eax
    mov  eax,dword[prcinfo+42]
    mov  edx,WIN_COLOR
    call draw_progress
if BENCH eq 1
    mcall 26,9
    sub  eax,[bench]
;    dps  <13,10,'Bench='>
;    dpd  eax
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
call Set_position
;---------------------------------------------
    cmp  dword[is_scroll_bar_needed], 0
     je  @f
        xor     eax,eax
        inc     eax
        mov     [scroll_bar_data_vertical.all_redraw],eax
; draw for Vertical ScrollBar
        push    dword scroll_bar_data_vertical
        call    [scrollbar_ver_draw]
; reset all_redraw flag 
        xor     eax,eax
        mov     [scroll_bar_data_vertical.all_redraw],eax
  @@:
;---------------------------------------------
    mcall 12, 2
    ret

;---------------------------------------------------------------------
Set_position:
    mov  eax, dword[prcinfo+46]
    cmp  eax, [HDoc]
    mov  dword[is_scroll_bar_needed], 0
     jnl .quit
    mov  dword[is_scroll_bar_needed], 1

    mov  eax, [scroll_bar_data_vertical.max_area]
    mul  dword[prcinfo+46]
    div  dword[HDoc]
    cmp  eax, [scroll_bar_data_vertical.max_area]
    mov  dword[scroll_bar_data_vertical.cur_area],eax
     jng @f
    mov  eax, [scroll_bar_data_vertical.max_area]
    mov  dword[scroll_bar_data_vertical.cur_area], eax
  @@:
    mov eax, [HDoc]
    cmp eax, dword[prcinfo+46]
    sub eax, dword[prcinfo+46]
    add eax, 20                    ; height of clear area under text when you are at the end of document
     jg @f
    mov eax, 0
  @@:
    mul [scroll_bar_data_vertical.position]
    mov ebx, [scroll_bar_data_vertical.max_area]
    sub ebx, [scroll_bar_data_vertical.cur_area]
    div ebx
    
    mov dword[top], TOP
    sub dword[top], eax
    
  .quit:
    ret
;---------------------------------------------------------------------
Set_scroll_position:
    mcall 9, procinfo2, -1
    mov eax, dword[procinfo2+0x3E]
    sub eax, scroll_width_size
    mov word[scroll_bar_data_vertical.start_x], ax

    mov eax, dword[procinfo2+0x42]
    sub eax, 17
    mov word[scroll_bar_data_vertical.size_y], ax
    
    ret
;---------------------------------------------------------------------

if GUTTER eq 1
   arrow db 0x19
end if
;---------------------------------------------------------------------
;---  ДАННЫЕ ПРОГРАММЫ  ----------------------------------------------
;---------------------------------------------------------------------

; интерфейс программы многоязычный
;  Вы можете задать язык в MACROS.INC (lang fix язык)

window_title:           db      'RtfRead v1.034',0
buf_cmd_lin             dd      0x0
is_scroll_bar_needed    dd      0x0
;---------------------------------------------------------------------
l_libs_start:

library01  l_libs system_dir_ProcLib+9, cur_dir_path, library_path, system_dir_ProcLib, \
err_message_found_lib2, head_f_l, ProcLib_import, err_message_import2, head_f_i

library02  l_libs system_dir_Boxlib+9, cur_dir_path, library_path, system_dir_Boxlib, \
err_message_found_lib1, head_f_l, Box_lib_import, err_message_import1, head_f_i

end_l_libs:
;---------------------------------------------------------------------
system_dir_ProcLib      db '/sys/lib/proc_lib.obj',0
system_dir_Boxlib       db '/sys/lib/box_lib.obj',0

head_f_i:
head_f_l                db 'error',0

err_message_found_lib1  db 'box_lib.obj - Not found!',0
err_message_found_lib2  db 'proc_lib.obj - Not found!',0

err_message_import1     db 'box_lib.obj - Wrong import!',0
err_message_import2     db 'proc_lib.obj - Wrong import!',0

;---------------------------------------------------------------------
align 4
ProcLib_import:
OpenDialog_Init         dd aOpenDialog_Init
OpenDialog_Start        dd aOpenDialog_Start
;OpenDialog__Version    dd aOpenDialog_Version
        dd      0
        dd      0
aOpenDialog_Init        db 'OpenDialog_init',0
aOpenDialog_Start       db 'OpenDialog_start',0
;aOpenDialog_Version    db 'Version_OpenDialog',0
;---------------------------------------------------------------------
OpenDialog_data:
.type                   dd 0
.procinfo               dd procinfo ;+4
.com_area_name          dd communication_area_name ;+8
.com_area               dd 0 ;+12
.opendir_pach           dd temp_dir_pach ;+16
.dir_default_pach       dd communication_area_default_pach ;+20
.start_path             dd open_dialog_path ;+24
.draw_window            dd draw_window ;+28
.status                 dd 0 ;+32
.openfile_pach          dd fname_buf ;+36
.filename_area          dd 0    ;+40
.filter_area            dd Filter

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

;  db '/HD/1/RTF/texts/index_ru.RTF',0

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
;I_END0:
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
;HClick dd ?
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

;---------------------------------------------------------------------
align   4
Box_lib_import:

menu_bar_draw           dd aMenu_bar_draw
menu_bar_mouse          dd aMenu_bar_mouse

scrollbar_ver_draw      dd aScrollbar_ver_draw
scrollbar_ver_mouse     dd aScrollbar_ver_mouse

        dd 0
        dd 0

aMenu_bar_draw          db 'menu_bar_draw',0
aMenu_bar_mouse         db 'menu_bar_mouse',0
;aVersion_menu_bar       db 'version_menu_bar',0

aScrollbar_ver_draw     db 'scrollbar_v_draw',0
aScrollbar_ver_mouse    db 'scrollbar_v_mouse',0
;---------------------------------------------------------------------
align   4
menu_data_1:
.type:          dd 0    ;+0
.x:
.size_x         dw 40   ;+4
.start_x        dw 2    ;+6
.y:
.size_y         dw 15   ;+8
.start_y        dw 2    ;+10
.text_pointer:  dd menu_text_area       ;0      ;+12
.pos_pointer:   dd menu_text_area.1     ;0      ;+16
.text_end       dd menu_text_area.end   ;0      ;+20
.mouse_pos      dd 0    ;+24
.mouse_keys     dd 0    ;+28
.x1:
.size_x1        dw 40   ;+32
.start_x1       dw 2    ;+34
.y1:
.size_y1        dw 100  ;+36
.start_y1       dw 18   ;+38
.bckg_col       dd 0xeeeeee     ;+40
.frnt_col       dd 0xff ;+44
.menu_col       dd 0xffffff     ;+48
.select         dd 0    ;+52
.out_select     dd 0    ;+56
.buf_adress     dd 0    ;+60
.procinfo       dd 0    ;+64
.click          dd 0    ;+68
.cursor         dd 0    ;+72
.cursor_old     dd 0    ;+76
.interval       dd 16   ;+80
.cursor_max     dd 0    ;+84
.extended_key   dd 0    ;+88
.menu_sel_col   dd 0x00cc00     ;+92
.bckg_text_col  dd 0    ;+96
.frnt_text_col  dd 0xffffff     ;+100
.mouse_keys_old dd 0    ;+104
.font_height    dd 8    ;+108
.cursor_out     dd 0    ;+112
.get_mouse_flag dd 0    ;+116

menu_text_area:
        db 'File',0
.1:
        db 'Open',0
        db 'Exit',0
.end:
        db 0
;---------------------------------------------------------------------
align   4
menu_data_2:
.type:          dd 0    ;+0
.x:
.size_x         dw 40   ;+4
.start_x        dw 43   ;+6
.y:
.size_y         dw 15   ;+8
.start_y        dw 2    ;+10
.text_pointer:  dd menu_text_area_2     ;0      ;+12
.pos_pointer:   dd menu_text_area_2.1   ;0      ;+16
.text_end       dd menu_text_area_2.end ;0      ;+20
.mouse_pos      dd 0    ;+24
.mouse_keys     dd 0    ;+28
.x1:
.size_x1        dw 50   ;+32
.start_x1       dw 43   ;+34
.y1:
.size_y1        dw 100  ;+36
.start_y1       dw 18   ;+38
.bckg_col       dd 0xeeeeee     ;+40
.frnt_col       dd 0xff ;+44
.menu_col       dd 0xffffff     ;+48
.select         dd 0    ;+52
.out_select     dd 0    ;+56
.buf_adress     dd 0    ;+60
.procinfo       dd 0    ;+64
.click          dd 0    ;+68
.cursor         dd 0    ;+72
.cursor_old     dd 0    ;+76
.interval       dd 16   ;+80
.cursor_max     dd 0    ;+84
.extended_key   dd 0    ;+88
.menu_sel_col   dd 0x00cc00     ;+92
.bckg_text_col  dd 0    ;       +96
.frnt_text_col  dd 0xffffff     ;+100
.mouse_keys_old dd 0    ;+104
.font_height    dd 8    ;+108
.cursor_out     dd 0    ;+112
.get_mouse_flag dd 0    ;+116

menu_text_area_2:
        db 'View',0
.1:
        db 'Zoom +',0
        db 'Zoom -',0
        db ' > >',0
        db ' << ',0
        db 'Align',0
        db 'Color',0
.end:
        db 0
;---------------------------------------------------------------------
align   4
menu_data_3:
.type:          dd 0    ;+0
.x:
.size_x         dw 40   ;+4
.start_x        dw 84   ;+6
.y:
.size_y         dw 15   ;+8
.start_y        dw 2    ;+10
.text_pointer:  dd menu_text_area_3     ;0      ;+12
.pos_pointer:   dd menu_text_area_3.1   ;0      ;+16
.text_end       dd menu_text_area_3.end ;0      ;+20
.mouse_pos      dd 0    ;+24
.mouse_keys     dd 0    ;+28
.x1:
.size_x1        dw 40   ;+32
.start_x1       dw 84   ;+34
.y1:
.size_y1        dw 100  ;+36
.start_y1       dw 18   ;+38
.bckg_col       dd 0xeeeeee     ;+40
.frnt_col       dd 0xff ;+44
.menu_col       dd 0xffffff     ;+48
.select         dd 0    ;+52
.out_select     dd 0    ;+56
.buf_adress     dd 0    ;+60
.procinfo       dd 0    ;+64
.click          dd 0    ;+68
.cursor         dd 0    ;+72
.cursor_old     dd 0    ;+76
.interval       dd 16   ;+80
.cursor_max     dd 0    ;+84
.extended_key   dd 0    ;+88
.menu_sel_col   dd 0x00cc00     ;+92
.bckg_text_col  dd 0    ;       +96
.frnt_text_col  dd 0xffffff     ;+100
.mouse_keys_old dd 0    ;+104
.font_height    dd 8    ;+108
.cursor_out     dd 0    ;+112
.get_mouse_flag dd 0    ;+116

menu_text_area_3:
        db 'Help',0
.1:
        db 'Home',0
.end:
        db 0
;---------------------------------------------------------------------
align   4
scroll_bar_data_vertical:
.x:
.size_x         dw scroll_width_size;+0
.start_x        dw WINW-25  ;+2
.y:
.size_y         dw WINH-45  ;+4
.start_y        dw 19   ;+6
.btn_high       dd scroll_width_size    ;+8
.type           dd 1    ;+12
.max_area       dd 300       ;+16
.cur_area       dd 50   ;+20
.position       dd 0    ;+24
.bckg_col       dd 0xAAAAAA     ;+28
.frnt_col       dd 0xCCCCCC     ;+32
.line_col       dd 0    ;+36
.redraw         dd 0    ;+40
.delta          dw 0    ;+44
.delta2         dw 0    ;+46
.run_x:
.r_size_x       dw 0    ;+48
.r_start_x      dw 0    ;+50
.run_y:
.r_size_y       dw 0    ;+52
.r_start_y      dw 0    ;+54
.m_pos          dd 0    ;+56
.m_pos_2        dd 0    ;+60
.m_keys         dd 0    ;+64
.run_size       dd 0    ;+68
.position2      dd 0    ;+72
.work_size      dd 0    ;+76
.all_redraw     dd 0    ;+80
.ar_offset      dd AR_OFFSET   ;+84
;---------------------------------------------------------------------
I_END0:
I_END:                             ; метка конца программы

procinfo process_information
rb RTFSIZE
esp1:
rb ESPSIZE
procinfo2 process_information
;---------------------------------------------------------------------
temp_dir_pach:
        rb 4096
cur_dir_path:
        rb 4096
library_path:
        rb 4096
;---------------------------------------------------------------------
    rb ESPSIZE                      ;stack
esp_end:
sys_mem:
