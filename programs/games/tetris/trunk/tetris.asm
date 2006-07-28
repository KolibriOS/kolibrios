; Date : 1st April 2001
; TETRIS for MENUET
; Author : Paolo Minazzi (email paolo.minazzi@inwind.it)
;
; -Note-
; 1. This program requires a PENTIUM or higher because uses the RDTSC
;    instrucion for get a random number.
; 2. You must use NASM to compile. Compiling is OK with NASM 0.98, I
;    don't know what happen with other versions.
; 3. You must use the arrow key to move and rotate a block.
; 4. In the near future there will be a new version of TETRIS. This is
;    only the beginning.
;
; Thanks to Ville, the author of this wonderful OS !
; Join with us to code !
;
;
; Changelog:
;
; 28.06.2001 - fasm port & framed blocks - Ville Turjanmaa
; 31.10.2001 - rdtsc replaced            - quickcode <quickcode@mail.ru>
; 03.11.2003 - added new blocks & random - Ivan Poddubny
;

LEN_X equ 14
LEN_Y equ 24
BORDER_LEFT equ 2
BORDER_RIGHT equ 2
BORDER_TOP equ 1
BORDER_BOTTOM equ 1
ADOBE_SIZE equ 12
X_LOCATION equ 6
Y_LOCATION equ 21
_MAXBLOCKS_ = 7*4
SCORE_TO_NEW_LEVEL equ 100000


use32

		org	0x0

		db	'MENUET01'    ; 8 byte id
		dd	0x01	      ; header version
		dd	START	      ; program start
		dd	I_END	      ; program image size
		dd	IM_END	      ; reguired amount of memory
		dd	IM_END	      ; esp
		dd	0x0,0x0       ; I_PARAM, I_ICON
include 'lang.inc'
START:				      ; start of execution

    mov   eax,3 		      ;
    int   0x40			      ;
    mov   cl,16 		      ;
    ror   eax,cl		      ; to make seconds more significant
    mov   [generator],eax	      ;
    call  random		      ;

    mov  byte[status],'0'
    mov  byte[menu],'0'
    call draw_window		      ; at first, draw the window

still:

    cmp  byte[status],'2'
    je	 attesa

    cmp  byte[status],'1'

    jne  attesa
    xor  edx,edx

    call draw_block


attesa:

    call mouse
    ;disabled because of bug
    ;EDIT: the bug is somewhere else..
    ;NOTE: dont release this without fixing the bug you lazy bastard!

    mov  eax,11 		      ; get event
    int  0x40

    cmp  eax,1			      ; redraw request ?
    jz	 red
    cmp  eax,2			      ; key in buffer ?
    jnz  check_button
    jmp  key
check_button:
    cmp  eax,3			      ; button in buffer ?
    jnz  scendi
    mov  eax,-1 		      ; close this program
    int  0x40

  red:				      ; redraw
    call draw_window
    jmp  still

parallel dd 1

scendi: 	cmp  byte[status],'1'
		jne  still

		cmp  byte[blabla],10
		je   blabla_0
		inc  byte[blabla]
		jmp  blabla_1
	      blabla_0:
		mov  byte[blabla],0
		inc  dword [current_block_y]
		call  check_crash
		jne   block_crash
	      blabla_1:


draw:		movzx edx,byte [current_block_color]
		call  draw_block
		mov   eax,5
		mov   ebx,5
		sub   ebx,[speed]
		int   0x40
		jmp   still

block_crash:	dec dword [current_block_y]
		movzx edx,byte [current_block_color]
		call draw_block
		call fix_block
		call check_full_line
		call draw_table
		call new_block
		call write_score
		call check_crash
		jz adr400
aspetta:	mov eax,10
		int 0x40
		cmp eax,1
		jne adr10000
		call draw_window
adr10000:	cmp eax,3
		jne aspetta

new_game:	mov dword [score],0
		mov dword [lines],0
		mov dword [level],0
		mov dword [speed],0
		mov byte [status],'1'
		call clear_table_tetris
		call first_block
		call new_block
		call draw_window

adr400: 	movzx edx,byte [current_block_color]
		call draw_block
		jmp still

include 'key.inc'
include 'mouse.inc'
include 'menu.inc'
include 'window.inc'
include 'block.inc'
include 'table.inc'
include 'random.inc'
include 'score.inc'

; DATA AREA

include 'blocks.inc'

  labelt:		db 'TETRIS II'
  scoretext:		db 'Score:'
  linestext:		db 'Lines:'
  speedtext:		db 'Speed:'
  leveltext:		db 'Level:'
  startgame:		db 'START'
  instr:		db 'INSTRUCTIONS'
  hist: 		db 'HISTORY'
  quit: 		db 'EXIT'
  paused:		db 'PAUSED'
  txt_end:

history:
    db 'TETRIS for MENUET v2.0            '
    db '                                  '
    db '                                  '
    db 'Originally made                   '
    db '               by Paolo Minazzi   '
    db '                                  '
    db 'Port & framed blocks              '
    db '               by Ville Turjanmaa '
    db '                                  '
    db 'RDTSC replaced                    '
    db '               by quickcode       '
    db '                                  '
    db 'New blocks & better random        '
    db '               by Ivan Poddubny   '
    db '                                  '
    db 'Better control, logo, menu, pause '
    db '               by Jeffrey Amelynck'

    db 'x <- END MARKER, DONT DELETE      '

instructions:
    db 'TETRIS for MENUET v2.0            '
    db '                                  '
    db 'Controls:                         '
    db '                                  '
    db 'Use left & right key to navigate  '
    db 'Use up key to turn the block      '
    db 'Use down key to make block fall   '
    db 'Use P to pause game               '
    db 'Use N to start a new game         '
    db 'Use ESC to go back to menu or exit'
    db 'You can also use the mouse to move'
    db 'the blocks, left button to let the'
    db 'blocks fall and right button to   '
    db 'rotate them                       '
    db 'you can pause the game by pressing'
    db 'both mouse buttons                '
    db '                                  '
    db 'DONT FORGET: move mouse out of the'
    db 'window if you want to use keyboard'

    db 'x <- END MARKER, DONT DELETE      '

I_END:

score:			dd 0
level:			dd 0
speed:			dd 0
lines:			dd 0
TMP_0:			dd 0
TMP_1:			dd 0
generator:		dd 0
current_block_x:	dd 0
current_block_y:	dd 0
current_block_pointer:	dd 0
current_block_color:	db 0
next_block_pointer:	dd 0
next_block_color:	db 0
blabla			dd 0
lastmousebutton 	dd 0
number_str:		db 0,0,0,0,0,0,0,0,0
end_number_str:
size_of_number_str	dd 7
delay:			dd 5
status: 		dd 0 ; 0=menu, 1=playing, 2=paused, 3=history , 4=instructions
menu:			dd 0 ; 0=start, 1=instructions, 2=history, 3=exit

table_tetris:		rb 2048+55

IM_END:
