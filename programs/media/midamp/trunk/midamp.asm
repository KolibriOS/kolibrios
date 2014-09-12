; MIDI PLAYER FOR MENUET v1.0
; Written in pure assembler by Ivushkin Andrey aka Willow
;
;
; Created:      December 7, 2004
; Last changed: July 29, 2005
;
; COMPILE WITH FASM

format binary as ""

PLAYLIST_PATH equ '/HD0/1/PLAYLIST.TXT'
;APP_MEM   equ 150*1024

IPC_PLAY  equ 0xa1
IPC_PAUS  equ 0xa2
IPC_TRIG  equ 0xa3
IPC_UPDT  equ 0xb1
IPC_NEXT  equ 0xb2

LISTITEMS equ 40
WND_BACK  equ 0x24263c
PLY       equ 63
WND_HEIGHT equ (PLY+9*LISTITEMS+10)+25

BTNS_XY   equ 14 shl 16+42
BTNS_SIZE equ 222 shl 16+17

BROWSE_X  equ 10 shl 16+8
BROWSE_Y  equ 26 shl 16+8
FN_XY     equ 12 shl 16+15
BAR_WIDTH equ 251
BAR_X     equ 10 shl 16
BAR_Y     equ 29 shl 16+5
TOTALTIME_XY equ 124 shl 16+28
CURTIME_X equ 225 shl 16+40
CURTIME_Y equ 15 shl 16+11
CURTIME_XY equ 236 shl 16+15

NONCRITICAL_MSG equ 0
SOUND equ ON;OFF
OUTDUMP equ 0
OUTLINE equ 8
FL_SHUFFLE equ 0x01
FL_REPEAT  equ 0x02
FL_HIDDEN  equ 0x04
FL_MUTE    equ 0x08
FL_REVERSE equ 0x10
FL_ADD     equ 0x20
FL_PLAY    equ 0x40
FL_LOCK    equ 0x80
FL_BOTTRED equ 0x100
FL_MULSEL  equ 0x8000

use32
  org    0x0

  db     'MENUET01'
  dd     0x01
  dd     START
  dd     IM_END
  dd     I_END ;APP_MEM
  dd     stacktop ;APP_MEM - 1024
  dd     I_PARAM
  dd     cur_dir_path
  
listsel    dd 0
channel dd 0
COLOR_ORDER equ MENUETOS
include '../../../config.inc'           ;for nightbuild
include '../../../macros.inc' ; decrease code size (optional)
include '../../../develop/libraries/box_lib/load_lib.mac'

@use_library

include 'lang.inc'
;purge mov
include '../../../debug.inc'
;include 'dlg.inc'
include 'playlist.inc'
include 'gif_lite.inc'
bottom:
    file 'bottom.gif'
hdrimg:
    file 'hdr.gif'
btns:
    file 'buttons.gif'
START:
        mcall 68, 11
                
load_libraries l_libs_start,end_l_libs

    mov  esi,I_PARAM
    cmp  dword[esi],0
    jz @f
    mov edi,filename
    mov ecx,256/4
    rep movsd
    mov byte [edi-1], 0
@@:
;OpenDialog initialisation
        push    dword OpenDialog_data
        call    [OpenDialog_Init]

    or   [flag],FL_BOTTRED;+FL_MUTE
    mov  ecx,ipcarea
    call init_ipc
    mcall 40,1000111b
    mov  esi,btns
    mov  edi,btn_raw
    call ReadGIF
    mov  esi,hdrimg
    mov  edi,hdr_raw
    call ReadGIF
    mov  esi,bottom
    mov  edi,bottom_raw
    call ReadGIF
    call respawn
    mcall 9,prcinfo,-1
    mov  edx,[ebx+30]
    mov  [parentPID],edx
    mov  esi,I_PARAM
    cmp  dword[esi],0
    jnz  .yesparam
    call PL_load
    cmp  [list_count],0
    je   noparam
    mov  eax,[pl_ptr]
    or   word[eax],FL_MULSEL
    jmp  auto_load
  .yesparam:
    mov  al,byte[esi]
    cmp  al,'/'
    je   .defact
    mov  [param],al
    inc  esi
  .defact:
        mov  edi,filename;fnbuf
    mov  ecx,64
    rep  movsd
    jmp  open_file
clearpath:
    newline
    mov  [fname_len],0
  noparam:
    mov  [param],'W'
    or   [flag],FL_ADD
;---------------------------------------------------------------------
;OpenDialog_start:
;       copy_path       open_dialog_name,path,library_path,0
        mov     [OpenDialog_data.type],0        ; Open
        or      [flag],FL_LOCK  
        push    dword OpenDialog_data
        call    [OpenDialog_Start]
        and     [flag],not FL_LOCK
;       cmp     [OpenDialog_data.status],2 ; OpenDialog does not start
;       je      .fopen  ;       some kind of alternative, instead OpenDialog
        cmp     [OpenDialog_data.status],1
;       je      open_file
;       jmp     still
        jne     still
;---------------------------------------------------------------------

;.fopen:
;    call fopen
;  get_path:
;    cmp  byte[filename],0
;    jz  still
  open_file:
    cmp  [param],'W'
    je   .noplay
    cmp  [param],'H'
    jne  .nohidd
;    or   [flag],FL_PLAY
    or   [flag],FL_HIDDEN
    call draw_window
    and  [flag],not FL_HIDDEN
    call Shade
;    jmp  .noplay

  .nohidd:
    or   [flag],FL_PLAY
  .noplay:
    xor  eax,eax
    mov  [play_area],ax
    mov  [tick_count],eax
    mov  [delta],eax
    mov  [curnote],0x80
    mov  ecx,64
    mov  esi,filename
    mov  edi,I_PARAM
    rep  movsd
    mov  eax,70
    mov  ebx,file_info
    mcall
    add  ebx,workarea
    mov  [midi_limit],ebx
    mov  edi,I_PARAM
    call find_slash
    mov  [fn_ptr],edi
    mov  edi,filename
    call str_len
    mov  [fname_len],eax
midi_kill:
    call kill
include 'midilite.inc'

decode_end:
;    dpd  edi
;    dps  <13,10,'Notes='>
;    sub  edi,[midi_limit]
;    shr  edi,1
;    dpd  edi
    dps ' Notes: max='
    movzx eax,[max_note]
    dpd  eax
    dps 'min='
    movzx eax,[min_note]
    dpd  eax
    newline
;    sub  esi,workarea
;    jmp  _close
  .play:
    call kill
    call respawn
    xor  edx,edx
    mov  esi,[midi_limit]
    mov  [cur_ptr],esi
    mov  [cur_tick],edx
    mov  [delta],edx
  .count_ticks:
    lodsw
    test eax,eax
    jz   .eof
    and  eax,0x7f
    add  edx,eax
    jmp  .count_ticks
  .eof:
    mov  [tick_count],edx
  if OUTDUMP eq 1
    mov  esi,[midi_limit]
    call out_dump
  end if
    and  [flag],not FL_LOCK
    test [flag],FL_PLAY
    jz   .noplay
    call draw_window
    mcall 5,100
    mov  eax,IPC_PLAY
    call ipc_send
  .noplay:
    test [flag],FL_ADD
    jz   red
    mov  esi,filename
    mov  ecx,[fname_len]
    movzx eax,[list_count]
    mov  [play_num],eax
 add_song:
    call PL_add
    and  [flag],not FL_ADD
red:
    call draw_window
still:
    mov  ecx,ipcarea
    call init_ipc
    mov  eax,10
    mcall
prc_event:
    test eax,eax
    jz   still
  .evt:
    cmp  eax,1
    je   red
    cmp  eax,2
    je   key
    cmp  eax,3
    je   button
    cmp  eax,7
    jne  still
    movzx eax,byte[ipcarea+16]
    cmp  eax,IPC_UPDT
    jne  .noupdt
    call draw_bar
    jmp  still
  .noupdt:
    cmp  eax,IPC_NEXT
    jne  still
    cmp  [param],'H'
    je   _close
    xor  edx,edx
    test [flag],FL_SHUFFLE
    jz   .noshuf
    mcall 26,9
    movzx ebx,byte[list_count]
    div  ebx
    mov  eax,edx
    jmp  play_
  .noshuf:
    test [flag],FL_REPEAT
    jnz  decode_end.play
    mov  eax,[play_num]
    inc  eax
    cmp  al,[list_count]
    jb   bList.next
    mov  eax,IPC_PAUS
    call ipc_send
    jmp  red

if OUTDUMP eq 1
out_dump:
    mov  ecx,OUTLINE
  .next_byte:
    lodsd
    bswap eax
    dph  eax
    dps  ' '
    lodsd
    bswap eax
    dph  eax
    dps  <13,10>
    loop .next_byte
    ret
end if

str_len:
; in: edi-str ptr
; out: eax-str length
    push ecx edi
    xor  eax,eax
    mov  ecx,256
    repne scasb
    jecxz .nofn
    sub  edi,[esp]
    mov  eax,edi
  .nofn:
    pop  edi ecx
    ret

;fopen:
;    or  [flag],FL_LOCK
;;    opendialog draw_window, ret_path, ret_path, filename
;ret_path:
;    and  [flag],not FL_LOCK
;    ret

include 'event.inc'
include "thread.inc"
include "draw.inc"
; Здесь находятся данные программы:

     dd -2 shl 16+4,251,12 shl 16,29 shl 16+5
     dd 21,16
main_coo:
     dd 14 shl 16, 42 shl 16,23 shl 16
     dd 228 shl 16+38
     dd 14 shl 16+10
     dd 236 shl 16+15
btncoords:
     dd 120 shl 16+20, 1 shl 16+15
     dd 149 shl 16+44, 2 shl 16+12
     dd 195 shl 16+26, 2 shl 16+12

     dd -2 shl 16+4,54,63 shl 16,6 shl 16+4
     dd 6,6
main_coo2:
     dd 169 shl 16, 4 shl 16,9 shl 16
     dd 121 shl 16+40
     dd 3 shl 16+9
     dd 130 shl 16+4
btncoords2:
     dd 48 shl 16+6, 6
     dd 2000 shl 16+44, 2 shl 16+12
     dd 2000 shl 16+26, 2 shl 16+12
ipcarea    rb 20
ipcarea2   rb 20

dots       db ':-'
text       db 'tone>     chnl>  <trk'
text_end:
coo        dd main_coo
play_limit dd playlist
pl_ptr     dd playlist
param      db 'W'
curnote    db 0x80
tick_count dd 0
;---------------------------------------------------------------------
OpenDialog_data:
.type                   dd 0
.procinfo               dd prcinfo ;+4
.com_area_name          dd communication_area_name ;+8
.com_area               dd 0 ;+12
.opendir_pach           dd temp_dir_pach ;+16
.dir_default_pach       dd communication_area_default_pach ;+20
.start_path             dd open_dialog_path ;+24
.draw_window            dd draw_window ;+28
.status                 dd 0 ;+32
.openfile_pach          dd filename ;+36
.filename_area          dd 0    ;+40
.filter_area            dd Filter
.x:
.x_size                 dw 420 ;+48 ; Window X size
.x_start                dw 10 ;+50 ; Window X position
.y:
.y_size                 dw 320 ;+52 ; Window y size
.y_start                dw 10 ;+54 ; Window Y position

communication_area_name:
        db 'FFFFFFFF_open_dialog',0
open_dialog_path:
if __nightbuild eq yes
    db '/sys/MANAGERS/opendial',0
else
    db '/sys/File Managers/opendial',0
end if
communication_area_default_pach:
        db '/rd/1',0

Filter:
dd Filter.end - Filter
.1:
db 'MID',0
.end:
db 0
;---------------------------------------------------------------------
system_dir_ProcLib                      db '/sys/lib/proc_lib.obj',0

head_f_i:
head_f_l        db 'error',0
err_message_found_lib2          db 'proc_lib.obj - Not found!',0

err_message_import2                     db 'proc_lib.obj - Wrong import!',0

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
l_libs_start:

library01  l_libs system_dir_ProcLib+9, cur_dir_path, temp_dir_pach, system_dir_ProcLib, \
err_message_found_lib2, head_f_l, ProcLib_import, err_message_import2, head_f_i

end_l_libs:
;---------------------------------------------------------------------
dir_info:
        dd      1
        dd      0
        dd      0
        dd      1
        dd      dir_table
        db      0
        dd      filename
play_area  dw ?
file_info:
           dd 0
           dd 0
           dd 0
fsize      dd 120*1024  ;APP_MEM-2048-workarea     ; max size
           dd workarea
IM_END: ; конец программы
filename:
           rb 4096 ;1024+16
prcinfo    process_information
I_PARAM    rb 256
childPID   dd ?
parentPID  dd ?
play_num   dd ?
counter    dd ?
flag       dd ?
fname_len  dd ?
fn_ptr     dd ?
delta      dd ?
cur_ptr    dd ?
cur_tick   dd ?
quarter    dd ?
octave     db ?
tempo      dd ?
midi_limit dd ?
track_len  dd ?
list_count db ?
cur_track  db ?
sel_track  db ?
ipcmsg     db ?
fnbuf:
           rb 1024
btn_raw    rb 222*17*3+8
hdr_raw    rb 275*29*3+8
bottom_raw rb 25*378*3+8
           rb 4
playlist   rb 256*LISTITEMS
IncludeUGlobals
;----------------------------------------------------------------
temp_dir_pach:
        rb 4096
;----------------------------------------------------------------
cur_dir_path:
        rb 4096
;----------------------------------------------------------------
        rb 4096
thread_stack:
        rb 4096
stacktop:
;----------------------------------------------------------------
dir_table:
        rb 32+304
workarea:
        rb 120*1024
I_END:
