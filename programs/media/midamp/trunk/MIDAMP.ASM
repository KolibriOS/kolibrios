; MIDI PLAYER FOR MENUET v1.0
; Written in pure assembler by Ivushkin Andrey aka Willow
;
;
; Created:      December 7, 2004
; Last changed: July 29, 2005
;
; COMPILE WITH FASM

PLAYLIST_PATH equ '/HD/1/MIDI/PLAYLIST.TXT'
APP_MEM   equ 150*1024
DIR_SIZE  equ 1024

IPC_PLAY  equ 0xa1
IPC_PAUS  equ 0xa2
IPC_TRIG  equ 0xa3
IPC_UPDT  equ 0xb1
IPC_NEXT  equ 0xb2

LISTITEMS equ 40
WND_BACK  equ 0x24263c
PLY	  equ 63
WND_HEIGHT equ (PLY+9*LISTITEMS+10)+25

BTNS_XY   equ 14 shl 16+42
BTNS_SIZE equ 222 shl 16+17

BROWSE_X  equ 10 shl 16+8
BROWSE_Y  equ 26 shl 16+8
FN_XY	  equ 12 shl 16+15
BAR_WIDTH equ 251
BAR_X	  equ 10 shl 16
BAR_Y	  equ 29 shl 16+5
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
FL_ADD	   equ 0x20
FL_PLAY    equ 0x40
FL_LOCK    equ 0x80
FL_BOTTRED equ 0x100
FL_MULSEL  equ 0x8000

use32
  org	 0x0

  db	 'MENUET01'
  dd	 0x01
  dd	 START
  dd	 I_END
  dd	 APP_MEM
  dd	 APP_MEM-1024
  dd	 I_PARAM
listsel    dd 0
channel dd 0
COLOR_ORDER equ MENUETOS
include 'macros.inc' ; decrease code size (optional)
lang fix en
;purge mov
include 'debug.inc'
include 'dlg.inc'
include 'playlist.inc'
include 'gif_lite.inc'
bottom:
    file 'bottom.gif'
hdrimg:
    file 'hdr.gif'
btns:
    file 'buttons.gif'
START:
    or	 [flag],FL_BOTTRED;+FL_MUTE
    mov  ecx,ipcarea
    call init_ipc
    mcall 40,1000111b
    mov  esi,btns
    mov  edi,btn_raw
    mov  eax,hash_table
    call ReadGIF
    mov  esi,hdrimg
    mov  edi,hdr_raw
    mov  eax,hash_table
    call ReadGIF
    mov  esi,bottom
    mov  edi,bottom_raw
    mov  eax,hash_table
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
    je	 noparam
    mov  eax,[pl_ptr]
    or	 word[eax],FL_MULSEL
    jmp  auto_load
  .yesparam:
    mov  al,byte[esi]
    cmp  al,'/'
    je	 .defact
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
    or	 [flag],FL_ADD
    call fopen
  get_path:
    cmp  byte[filename],0
    jz	 still
  open_file:
    cmp  [param],'W'
    je	 .noplay
    cmp  [param],'H'
    jne  .nohidd
;    or   [flag],FL_PLAY
    or	 [flag],FL_HIDDEN
    call draw_window
    and  [flag],not FL_HIDDEN
    call Shade
;    jmp  .noplay

  .nohidd:
    or	 [flag],FL_PLAY
  .noplay:
    xor  eax,eax
    mov  [play_area],ax
    mov  [tick_count],eax
    mov  [delta],eax
    inc  eax
    mov  [fsize],eax
    mov  [curnote],0x80
    mov  ecx,64
    mov  esi,filename
    mov  edi,I_PARAM
    rep  movsd
    mov  eax,58
    mov  ebx,file_info
    int  0x40
    mov  eax,ebx
    shr  eax,9
    inc  eax
    mov  [fsize],eax
    add  ebx,workarea
    mov  [midi_limit],ebx
    mov  edi,I_PARAM
    call find_slash
    mov  [fn_ptr],edi
    mov  edi,filename
    call str_len
    mov  [fname_len],eax
    mov  eax,58
    mov  ebx,file_info
    int  0x40
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
    jz	 .eof
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
    jz	 .noplay
    call draw_window
    mcall 5,100
    mov  eax,IPC_PLAY
    call ipc_send
  .noplay:
    test [flag],FL_ADD
    jz	 red
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
    int  0x40
prc_event:
    test eax,eax
    jz	 still
  .evt:
    cmp  eax,1
    je	 red
    cmp  eax,2
    je	 key
    cmp  eax,3
    je	 button
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
    je	 _close
    xor  edx,edx
    test [flag],FL_SHUFFLE
    jz	 .noshuf
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
    jb	 bList.next
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

fopen:
    or	 [flag],FL_LOCK
    opendialog draw_window, ret_path, ret_path, filename
ret_path:
    and  [flag],not FL_LOCK
    ret

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

dots	   db ':-'
text	   db 'tone>     chnl>  <trk'
text_end:
coo	   dd main_coo
play_limit dd playlist
pl_ptr	   dd playlist
param	   db 'W'
curnote    db 0x80
tick_count dd 0
play_area  dw ?
file_info:
	   dd 0
	   dd 0
fsize	   dd 1
	   dd workarea
	   dd hash_table
I_END:	; конец программы
filename:
	   rb 1024+16
prcinfo    process_information
I_PARAM    rb 256
childPID   dd ?
parentPID  dd ?
play_num   dd ?
counter    dd ?
flag	   dd ?
fname_len  dd ?
fn_ptr	   dd ?
delta	   dd ?
cur_ptr    dd ?
cur_tick   dd ?
quarter    dd ?
octave	   db ?
tempo	   dd ?
midi_limit dd ?
track_len  dd ?
list_count db ?
cur_track  db ?
sel_track  db ?
ipcmsg	   db ?
fnbuf:
	   rb 256
btn_raw    rb 222*17*3+12
hdr_raw    rb 275*29*3+12
bottom_raw rb 25*378*3+12
	   rb 4
playlist   rb 256*LISTITEMS
hash_table:
	   rd 4096
dir_table  rb DIR_SIZE
workarea:
