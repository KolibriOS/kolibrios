IDA equ 1 ; We're running debugida.exe

STRIP  equ 40
LEVNUM equ 500
 XFOFS equ 10
 YFOFS equ 55
 MSGXO equ 32
GAMES_ALL equ 12
WNDCOLOR equ 0x13c0c0c0;0x13f6f6f6;
MINCS equ 40
MAXCS equ 80
macro icall lbl
{
	call [lbl+ebp*4]
}

  use32
  org	  0x0

  db	 'MENUET01'		 ; 8 byte id
  dd	 0x01			 ; header version
  dd	 run			 ; start of code
  dd	 I_END			 ; size of image
  dd	 end_mem		 ; memory for app
  dd	 stack_end		  ; esp
  dd	 0x0 , 0x0		 ; I_Param , I_Icon
lang fix ru
include "macros.inc"
purge mov
;include "../../../debug.inc"
COLOR_ORDER equ MENUETOS

include 'gif_lite.inc'

include 'tam.inc'
include 'smz.inc'
include 'tilt.inc'
include 'tilt2.inc'
include 'fhouse.inc'
include 'loops.inc'
include 'wriggle.inc'
include 'blackbox.inc'
include 'marble.inc'
include 'cstep.inc'
include 'orient.inc'
include 'nolt.inc'
include 'colzone.inc'

run:
		mov  [Ces],STRIP;20
    mcall 3
    mov  cl,16
    ror  eax,cl
    mov  [generator],eax    ; random generator from Tetris

		and [mpress_flag],0
		and [gif_unp],0
    xor  ebp,ebp ; game_t
;    mov  ebp,10
    mcall 64,1,end_mem
    mov  [pause_time],40
    call resize
  .game1:
    mov  [levnum],LEVNUM
  .game:
    and  [win_flag],0
    and  [drag_flag],0
    mov  ecx,[levnum]
    mov  esi,[levels+ebp*4]
    jecxz .skip
  .nxt:
    movzx eax,byte[esi]
    add  esi,eax
    xor  eax,eax
    cmp  al,byte[esi]
    jne  .noz
    mov  esi,LEVNUM;500
    sub  esi,ecx
    mov  [levcount],esi
    and  [levnum],0
    jmp  .game
  .noz:
    loop .nxt
  .skip:
		mcall 40,111b
	  mov  [mouse_processed],1
    xor  eax,eax
    icall prepare_level
wnd_size:
  	mov  eax,[Ces]
	  shr  eax,2
	  sub  eax,2
    mov  [sq_size],eax
  if ~ IDA eq 1
    mov   edx,[fx]
    imul  edx,[Ces]
    add   edx,XFOFS*2+20
    mov   esi,[fy]
    imul  esi,[Ces]
    add   esi,YFOFS+30
    mov   ecx,-1
    mcall 67,ecx
  end if
red:
    call  draw_window

drw:
		call  drwfld

still:
    mcall 2
    test al,al
    jz	 still
    mcall 10
    cmp  eax,2
    je	 key
    cmp  eax,1			; redraw request ?
    je	 red
    cmp  eax,3			; button in buffer ?
    je	 button
    cmp  eax,6			; mouse event ?
    je	 mouse
key:
    mcall 2
;    movzx ebx,ah
;    dpd  ebx
		cmp  ebp,10
		je   .noplus
    mov  ebx,5
    cmp  ah,184
    jne  .nomin
    cmp  [Ces],MINCS
    je	 still
  .ces:
    sub  [Ces],ebx
    call resize
    jmp  wnd_size
  .nomin:
    cmp  ah,183
    jne  .noplus
    cmp  [Ces],MAXCS
    je	 still
    neg  ebx
    jmp  .ces
	.noplus:
    cmp  ah,'h'
    jne  .nohlp
;  if IDA eq 1
;    cmp  [child],0
;    jne  still
;  else
    call kill_help
;  end if
    mov  [pack],ebp
    mcall 51,1,show_help,chs_end
;    dpd  eax
    mov  [child],eax
    jmp  still
  .nohlp:
    cmp  [win_flag],10
    je	 .levover
    cmp  [win_flag],2
    je	 .no_win
    cmp  [win_flag],1
    jne  .now
  .levover:
    call lev_next
  .no_win:
    and  [win_flag],al
    jmp  run.game
  .now:
    shr  eax,8
    mov  [jump],drw;red
    cmp  eax,13
    je	 run.game
    icall key_handler
    jmp  [jump]

button:
    mcall 17
    cmp  ah,1
    je	 close
    cmp  ah,2
    je	 .game_type
    cmp  ah,4
    je	 .inclev
    cmp  ah,3
    je	 .declev
    jmp  still
  .declev:
    dec  [levnum]
    cmp  [levnum],0
    jge  run.game
    push [levcount]
    pop  [levnum]
    jmp  run.game
  .inclev:
    call lev_next
    jmp  run.game
  .game_type:
    inc  ebp
    cmp  ebp,GAMES_ALL
    jbe  run.game1
    xor  ebp,ebp
    jmp  run.game1
  close:
  if ~ IDA eq 1
    call kill_help
  end if
    mcall -1

;if ~ IDA eq 1

kill_help:
		mcall 9,prc_info2,-1
		mov  edx,[child]
		mov  ecx,eax
	.lp:
		mcall 9
		cmp  edx,[ebx+30]
		jne   .nochild
		mcall 18,2
    and  [child],0
		ret
	.nochild:
		loop .lp
		ret

;end if

mouse:
		cmp  [mouse_processed],1
		jne  .ex
		mcall 37,1
		sub  eax,XFOFS shl 16+YFOFS
		xor  edx,edx
		mov  ebx,[Ces]
		mov  ecx,eax
		and  eax,0xffff
		div  ebx
		xor  edx,edx
		mov  [mouse_y],eax
		cmp  eax,[fy]
		jae  .ex
		mov  eax,ecx
		shr  eax,16
		div  ebx
		mov  [mouse_x],eax
		cmp  eax,[fx]
		jae  .ex
		mcall 37,2 ; get buttons
		cmp  [drag_flag],0
		je   .nodrag
		mov  ebx,eax
		cmp  eax,[mpress_flag]
		mov  eax,-1
		je   .nochg
		dec  eax
	.nochg:
		mov  [mpress_flag],ebx
		jmp  .call_hnd
	.nodrag:
		test eax,eax
		jz  .ex1
		cmp  eax,[mpress_flag]
		je   .ex
		mov  [mpress_flag],eax
		mov  eax,-1
	.call_hnd:
		mov  [mouse_processed],0
		icall key_handler
		jmp  [jump]
	.ex1:
		mov  [mpress_flag],0
	.ex:
		jmp  still

lev_next:
    push eax
    inc  [levnum]
    mov  eax,[levnum]
    cmp  eax,[levcount]
    jbe  .ex
    and  [levnum],0
  .ex:
    pop  eax
    ret
;---------------------------------------------------------------------
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
draw_window:
    pusha
    mcall 12,1
    mov   ebx,[fx]
    imul  ebx,[Ces]
    push  ebx
    add   ebx,50 shl 16+XFOFS*2+20
    mov   ecx,[fy]
    imul  ecx,[Ces]
    add   ecx,10 shl 16+YFOFS+30
	mov edx, WNDCOLOR 
	mov edi, header
    mcall 0
    
	mov esi, edx
	and	esi,0xffffff
	mcall 9,prc_info,-1
	pop	ebx
    
	mov	eax,[prc_info+70] ;status of window
	test	eax,100b
	jne	.end
    
    add   ebx,XFOFS shl 16

    mcall 8,,<25,12>,2

    mcall 8,<XFOFS,10>,<40,12>,3
    add   ebx,13 shl 16
    inc   edx
    mcall

    mov   ecx,ebp
    mov   edx,game_names+4
    call  get_mstr
    mcall 4,<XFOFS+2,28>,0x8000
    imul  esi,6
    add   esi,3
    shl   esi,16
    lea   edx,[ebx+esi]
    mcall 47,0x020001,levnum,,0x8000

    mcall 4,<XFOFS+3,43>,0x108000,next_msg,3
.end:
    mcall 12,2
    popa
    ret
;---------------------------------------------------------------------
unpack_level:
    mov   ecx,[cell_count]
    mov   edi,field
  .lp:
    movzx eax,byte[esi]
    inc   esi
    shl   ax,4
    shr   al,4
    stosw
    loop  .lp
    ret
;---------------------------------------------------------------------
get_xy:
; eax-coord
; out: [lx]-[x+2][CS-4],[ly]-[y+2][CS-4]
    pusha
    xor   edx,edx
    mov   esi,[sq_size]
    lea   edi,[esi*2]
    neg   edi
    add   edi,[Ces]
    mov   [lx],edi
    mov   [ly],edi
    mov   ebx,[fx]
    div   bx
    imul  eax,[Ces]
    imul  edx,[Ces]
    lea   edx,[edx+XFOFS+esi]
    lea   eax,[eax+YFOFS+esi]
    mov   word[ly+2],ax
    mov   word[lx+2],dx
    popa
    ret

check_move:
; eax-coord, ebx-direction
; CF=0 if move is valid
    bt	 dword[field+eax],ebx
    ret

delay:
    pusha
    mcall 5,[pause_time]
    popa
    ret

get_mstr:
; in:  edx,ecx
; out: edx,esi
    mov   esi,[edx-4]
    jecxz .ex
    lea   edx,[edx+esi+4]
    dec   ecx
    jmp   get_mstr
  .ex:
    ret

maze_field:
    mov   edi,field
    mov   [ly],YFOFS shl 16+YFOFS
    mov   eax,38
    mov   ecx,[fy]
  .lp1:
    push  ecx
    mov   [lx],XFOFS shl 16+XFOFS
    mov   ecx,[fx]
  .lp2:
    push  ecx
    xor   esi,esi
  .lp3:
    mov   edx,0xd0d0d0
    bt	  dword[edi],esi
    jnc   .gray
    mov   edx,0
  .gray:
    mov   ebx,[lx]
    add   ebx,[dir_ofs+esi*4]
    mov   ecx,[ly]
    add   ecx,[dir_ofs+esi*4+8]
    mcall
    inc   esi
    cmp   esi,4
    jb	  .lp3
    inc   edi
    mov   ecx,[Ces]
    add   word[lx],cx
    add   word[lx+2],cx
    pop   ecx
    loop  .lp2
    mov   ecx,[Ces]
    add   word[ly],cx
    add   word[ly+2],cx
    pop   ecx
    loop  .lp1
    ret

grid_field:
    mov   edx,0xa0a0a0
    mov   eax,38

    mov   ecx,[fy]
    mov   ebx,[fx]
    imul  ebx,[Ces]
    add   ebx,XFOFS shl 16+XFOFS
    mov   esi,YFOFS shl 16+YFOFS
    inc   ecx
  .lp1:
    push  ecx
    mov   ecx,esi
    mcall
    mov   ecx,[Ces]
    add   esi,ecx
    shl   ecx,16
    add   esi,ecx
    pop   ecx
    loop  .lp1

    mov   ecx,[fx]
    mov   esi,[fy]
    imul  esi,[Ces]
    add   esi,YFOFS shl 16+YFOFS
    mov   ebx,XFOFS shl 16+XFOFS
    inc   ecx
  .lp2:
    push  ecx
    mov   ecx,esi
    mcall
    mov   ecx,[Ces]
    add   ebx,ecx
    shl   ecx,16
    add   ebx,ecx
    pop   ecx
    loop  .lp2
    ret

get_xy_sf:
		xor  eax,eax
		and  [player],eax
    mov  [fx],eax
    mov  [fy],eax
    lodsb
    lodsb
    mov  ah,al
    shr  ah,4
    and  al,0xf
    mov  byte[fx],ah
    mov  byte[fy],al
    lodsb
    mov  byte[player],al
    lodsb
    mov  byte[finish],al
  .count:
    mov  eax,[fx]
    mov  [dirs+4],eax
    neg  eax
    mov  [dirs+8],eax
    mov  eax,[fx]
    imul eax,[fy]
    mov  [cell_count],eax

    ret

get_last_mclick:
; out - eax=0 no click or outside field
;                               eax>0 button pressed, ebx=[xy]
		mov  [mouse_processed],1
		mov  eax,[mpress_flag]
		mov  ebx,[mouse_y]
		imul ebx,[fx]
		add  ebx,[mouse_x]
		ret

erase_field:
		pusha
		mov  ecx,[cell_count]
		xor  eax,eax
		mov  edi,field
		rep  stosb
		popa
		ret

get_pxy:
; in: [pack] - cell#, out: word[pack] - packed  [xy]
		pusha
		mov  eax,[pack]
		cdq
		mov  ebx,[fx]
		div  ebx
		shl edx,8
		add  eax,edx
		mov  [pack],eax
		popa
		ret

check_bounds:
; in: eax - cell, ebx - dir
; out: ebx=-1 if bounds crossed
		push eax ecx
		mov  ecx,eax
		add  ecx,[dirs+ebx*4]
		mov  ebx,ecx
		call get_offset
		mov  ebx,eax
		pop  ecx eax
    cmp  ebx,-1
		ret

get_offset:
; in: eax - start, ebx-end
; out: eax - dir or -1 if not straight
;      ebx - distance
		push ecx edx
		mov  ecx,-1
		mov  edx,[cell_count]
		cmp  eax,edx
		jae  .ex
		cmp  ebx,edx
		jae  .ex
		mov  [pack],eax
		call get_pxy
		mov  eax,[pack]
		mov  [pack],ebx
		call get_pxy
		mov  ebx,[pack]
		cmp  ah,bh ; compare X
		jne  .noX
		mov  ecx,1
		sub  bl,al
		ja   .ok
		inc  ecx
	.ok2:
		neg  bl
		jmp  .ok
	.noX:
		cmp  al,bl ; compare Y
		jne  .ex
		inc  ecx
		sub  bh,ah
		mov  bl,bh
		jb   .ok2
		add  ecx,3
	.ok:
		movzx ebx,bl
	.ex:
		mov  eax,ecx
		pop edx ecx
		ret

show_help:
		mov   ebp,[pack]
	.red:
    mcall 12,1
    mov   ebx,[prc_info.box.left]
    add   ebx,[prc_info.box.width]
    shl   ebx,16
    add   ebx,310
    mov   ecx,[prc_info.box.top]
    shl   ecx,16
    add   ecx,220
    mcall 0,,,0x03c0c0c0
    mcall 4,<8,8>,0x00ffffff,help_hdr,help_hdr.size
    mov   ecx,ebp
    mov   edx,game_names+4
    call  get_mstr
    sub   esi,2
    mcall 4,<6*help_hdr.size+15,8>,0x00ffffff

    mov   edx,[help_msg+ebp*4]
    add   edx,4
    xor   ecx,ecx
    mov   ebx,10 shl 16+30
  .nxt:
    mov   esi,[edx-4]
    jecxz .drw
    dec   ecx
    lea   edx,[edx+esi+4]
    jmp   .nxt
  .drw:
    cmp   esi,-1
    je	  .edraw
    mcall ,,0x000000ff
    mov   ecx,1
    add   ebx,12
    jmp    .nxt
  .edraw:
    mcall 12,2
	.still:
    mcall 10
    cmp  eax,2
    jne  .nokey
    mcall
    jmp  .close
  .nokey:
    cmp  eax,1			; redraw request ?
    je	 .red
    cmp  eax,3			; button in buffer ?
    jne  .still
    mcall 17
    cmp  ah,1
    jne  .still
  .close:
    and  [child],0
    mcall -1

getline:
; in: esi,edi
		pusha
    mov  eax,esi
    call get_xy
    mov  ebx,[lx]
    mov  ecx,[ly]
    mov  eax,edi
    call get_xy
    mov  bx,word[lx+2]
    mov  cx,word[ly+2]
    mov  eax,[Ces]
    shr  eax,1
    sub  eax,[sq_size]
    add  ebx,eax
    add  ecx,eax
    shl  eax,16
    add  ebx,eax
    add  ecx,eax
    mov  [lx],ebx
    mov  [ly],ecx
		mcall 38
		popa
		ret

bold_line:
  WGSPC equ 1
    pusha
		mov  edi,WGSPC shl 16+WGSPC
    add  [lx],edi
    add  [ly],edi
		mcall 38,[lx],[ly]
		shl  edi,1
    sub  ebx,edi
    sub  ecx,edi
		mcall
		popa
		ret

get_rnd:
		mov  eax, [generator]
    sub  eax,0x43ab45b5    ; next random number
    ror  eax,1
    xor  eax,0x32c4324f
    ror  eax,1
    mov  [generator],eax
    ret

drwfld:
		pusha
    mov   ebx,[fx]
    imul  ebx,[Ces]
    add   ebx,XFOFS shl 16
    mov   ecx,[fy]
    imul  ecx,[Ces]
    add   ecx,YFOFS shl 16
    mcall 13,,,WNDCOLOR
    add   ebx,26 shl 16-26
    sub   ecx,16 shl 16
    mov   cx,16
    mcall
    icall draw_field
    icall draw_more
    cmp   [win_flag],0
    jz	  .edraw
    movzx ecx,[win_flag]
    mov   edx,msgs+4
    call  get_mstr
    mcall 4,<XFOFS+MSGXO,42>,0x100000ff
  .edraw:
    popa
    ret

resize:
    mov  eax,[Ces]
    mov  edi,dir_ofs+4
    stosd
    stosd
    stosw
    stosw
    mov  [edi+4],eax
		ret

need_image:
; in: eax - gif pointer
		cmp  [gif_unp],0
		jne  .skip
		push esi edi
		mov  esi,eax
		mov  eax,field
		mov  edi,raw_area
		call ReadGIF
		mov  [gif_unp],1
		pop  edi esi
	.skip:
		ret

out_image:
; in: eax - coord, ebx - image #

STRIP2 = STRIP-2
	pusha
  call get_xy
  mov  edx,[lx]
  mov  dx,word[ly+2]
  add  edx,1 shl 16+1
  imul ebx,STRIP2*STRIP2*3
	add  ebx,raw_area+12
  mcall 7,,STRIP2 shl 16+STRIP2
  popa
	ret

OR_strip:
file 'orientg2.gif'
	rd 2
	gif_unp dd 0

; DATA AREA
help_hdr db 'MegaMaze Help -'
	.size=$-help_hdr

dirs dd -1,0,0,1
dir_ofs dd 0, 1, 1, 1 shl 16+1, 0, 1

dir_rotate db 2,0,3,1,1,3,0,2

w_colors dd 0x0404CA,0xCC0404,0x00CC00
f_colors dd 0x9494FC,0xFC9494,0x99FF99,0xFFFF00

levnum	 dd LEVNUM

prepare_level dd TM_levelp,SMZ_levelp,TILT_levelp,TILT2_levelp,FH_levelp,\
	 LP_levelp,WG_levelp,BB_levelp,MAR_levelp,TM_levelp,OR_levelp,NLT_levelp,\
	 CZ_levelp
key_handler   dd TM_key,SMZ_key,TILT_key,TILT2_key,FH_key,LP_key,WG_key,BB_key,\
	 MAR_key,CSTEP_key,OR_key,NLT_key,CZ_key
draw_more     dd TM_drawm,SMZ_drawm,TILT_drawm,TILT2_drawm,FH_drawm,LP_drawm,\
	 WG_drawm,BB_drawm,MAR_drawm,CSTEP_drawm,OR_drawm,NLT_drawm,CZ_drawm
draw_field    dd maze_field,maze_field,maze_field,maze_field,grid_field,\
	 grid_field,grid_field,grid_field,maze_field,maze_field,OR_drawf,maze_field,\
	 grid_field
levels	      dd TM_level,SMZ_level,TILT_level,TILT2_level,FH_level,LP_level,\
	 WG_level,BB_level,MAR_level,CSTEP_level,OR_level,NLT_level,CZ_level
help_msg      dd TM_help,SMZ_help,TILT_help,TILT2_help,FH_help,LP_help,WG_help,\
	 BB_help,MAR_help,CSTEP_help,OR_help,NLT_help,CZ_help

header db 'Mega Maze', 0

next_msg db '< >'

game_names mstr \
	'Teseus & Minotaur #',\
	'1-2-3 Maze #',\
	'Tilt Maze #',\
	'Double Tilt #',\
	'Full-house #',\
	'Loops #',\
	'Wriggle #',\
	'BlackBox #',\
	'Marble #',\
	'Counter Step #',\
	'Orientation #',\
	'No left turn #',\
	'Colour-zone #'

msgs mstr ' ','You win!!!','Game over.','Start cell?','Dead end!',\
	"  atoms hidden.",'Ray emerged.','Ray absorbed.','Ray reflected.',\
	'Mark exactly    guesses','Your score is','Guess mark toggled'

I_END:
main_pid dd ?
child_stack rb 256
	chs_end:
drag_flag db ?
sq_size dd ?
mouse_processed dd ?
mpress_flag dd ?
mouse_x dd ?
mouse_y dd ?

Ces dd ?
pack dd ?
fy dd ?
fx dd ?
lx dd ?
ly dd ?
generator dd ?
cell_count dd ?
levptr	 dd ?
levcount dd ?

score	 dd ?
player:
teseus	 dd ?
finish:
minotaur dd ?

stepptr  dd ?
cur_step dd ?

win_flag db ?
jump	 dd ?
pause_time dd ?
child	 dd ?
area rb 1024
stak rb 1024
stack_end:
prc_info process_information
prc_info2 process_information
field:
    rb 128*128
raw_area:
		rb STRIP*STRIP*12*3+16
end_mem:
