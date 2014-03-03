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
; 30.12.2007 - cleaning keybuffer before reading an key - Gluk
; 09.11.2005 - fix for work under MeosDebug & better random - Andrey Halyavin
; 23.09.2005 - fixed bug, when 011111111111111-like sequence interpreted
;              as _11111111111111 in check_full_line - Sergey Kuzmin aka Wildwest
; 18.09.2005 - added 'Pause' button and made appropriate code and interface redesign
;              BTW, you can MOVE and ROTATE block during Pause ;)  - Sergey Kuzmin aka Wildwest
; 03.11.2003 - added new blocks & random - Ivan Poddubny
; 31.10.2001 - rdtsc replaced            - quickcode <quickcode@mail.ru>
; 28.06.2001 - fasm port & framed blocks - Ville Turjanmaa
; 
;

LEN_X equ 19  ;width of table
LEN_Y equ 29  ; height of table
BORDER_LEFT equ 1
BORDER_RIGHT equ 1
BORDER_TOP equ 1
BORDER_BOTTOM equ 1
ADOBE_SIZE equ 12

X_LOCATION equ 30 ; distance to table from left side
Y_LOCATION equ 50 ; distance to table from header 

UP_KEY equ 130+48
DOWN_KEY equ 129+48
LEFT_KEY equ 128+48
RIGHT_KEY equ 131+48
;BACKGROUND equ 03000080h

_MAXBLOCKS_ = 7*4

use32

                org     0x0

                db      'MENUET01'    ; 8 byte id
                dd      0x01          ; header version
                dd      START         ; program start
                dd      I_END         ; program image size
                dd      I_END+1024    ; reguired amount of memory
                dd      I_END+1024    ; esp
                dd      0x0,0x0       ; I_PARAM, I_ICON
include 'lang.inc'
include '../../../macros.inc'

START:                          ; start of execution


    ; --  quickcode - start

    mov   eax,3
    int   0x40
    mov   cl,16
    ror   eax,cl ; to make seconds more significant
    mov   [generator],eax
    call  random

    ; --  quickcode - end


    call clear_table_tetris
    call new_block
    call draw_window            ; at first, draw the window

still:
    xor  edx,edx
    call draw_block

attesa:

    mov  eax,11                 ; get event
    int  0x40

    cmp  eax,1                  ; redraw request ?
    jz   red
    cmp  eax,2                  ; key in buffer ?
    jnz  check_button
    jmp  key
check_button:
    cmp  eax,3                  ; button in buffer ?
    jnz  scendi
    jmp  button

  red:                          ; redraw
    call draw_window
    jmp  still



scendi:
;///////////////////////////////////////////////// Wildwest
                 cmp dword [pauses], 0
                 je ok
                 jne not_ok
not_ok:  
                inc   dword [current_block_y]
                dec  dword [current_block_y]
                call  check_crash
                jne   block_crash
                jmp draw            
ok:  
 ;/////////////////////////////////////////////////  
                inc   dword [current_block_y]
                call  check_crash
                jne   block_crash
draw:           movzx edx,byte [current_block_color]
                call  draw_block
                mov   eax,5
                movzx ebx,byte [delay]
                int   0x40
                mov   [force_down],	1
                jmp   still

block_crash:    dec dword [current_block_y]
                movzx edx,byte [current_block_color]
                call draw_block
                call fix_block
                call check_full_line
                call draw_table
                call new_block
                inc dword [score]
                call write_score
                call check_crash
                jz adr400
aspetta:        mov eax,10
                int 0x40
                cmp eax,1
                jne adr10000
                call draw_window
adr10000:       cmp eax,3
                jne aspetta
new_game:       mov eax,17
                int 0x40
                cmp ah,1
                jnz adr401
                jmp end_program
adr401:      
;///////////////////////////////////////////////////// Wildwest
                mov dword [pauses], 0   
;////////////////////////////////////////////////////
                mov dword [score],0
                call clear_table_tetris
                call new_block
                call draw_window

adr400:         movzx edx,byte [current_block_color]
                call draw_block
                mov eax,5
                movzx ebx,byte [delay]
                int 0x40
                jmp still

key:            mov  eax,2 ; Gluk
                int  0x40
				cmp eax,1
				jne getkeyi
				mov ah,dh
				jmp adr32

getkeyi:		mov dh,ah ; Gluk
				jmp key

adr32:          cmp ah,LEFT_KEY
                jne adr_30
                cmp dword[pauses], 1 ; If game is paused, dont allow movement
                je adr4000
                dec dword [current_block_x]
                call check_crash
                jz adr4000
                inc dword [current_block_x]
adr4000:        jmp scendi

adr_30:         cmp ah,RIGHT_KEY
                jne adr_31
                cmp dword[pauses], 1 ; If game is paused, dont allow movement
                je adr3000
                inc dword [current_block_x]
                call check_crash
                jz adr3000
                dec dword [current_block_x]
adr3000:        jmp scendi

adr_31:         cmp ah,UP_KEY
                jne adr51
                cmp dword[pauses], 1 ; If game is paused, dont allow movement
                je adr50
                mov edx,[current_block_pointer]
                mov edx,[edx+16]
                mov esi,[current_block_pointer]
                mov [current_block_pointer],edx
                call check_crash
                jz adr50
                mov [current_block_pointer],esi
adr50:          jmp scendi

adr51:          cmp ah,DOWN_KEY
                jne adr61
                cmp dword[pauses], 1 ; If game is paused, disable force_down
                je adr52
                cmp [force_down], 1
                 jne scendi
                mov byte [delay],5  ;!!! 2
adr52:          jmp scendi

adr61:          cmp ah,' '
                jne adr62
                cmp dword[pauses], 1 ; If game is paused, disable force_down
                je adr62
                cmp [force_down], 1
                 jne scendi
                mov byte [delay],5  ;!!! 2
adr62:          jmp scendi


button:                       ; button
    mov  eax,17
    int  0x40
    cmp  ah,1                   ; button id=1 ?
    jz  end_program
    cmp ah,2
    jz go_new_game
    cmp  ah,3                   ; button id=3 ? ///Wildwest's  'Pause' button
    jz  pause_program
    jmp still

end_program:
    or   eax,-1                ; close this program
    int  0x40

go_new_game:
    jmp new_game
;//////////////////////////////////////////////////// Wildwest
pause_program:
cmp dword [pauses], 0
je incr
mov dword [pauses], 0
 jmp still
incr:
mov dword [pauses], 1
jmp still
;////////////////////////////////////////////////////
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
draw_window:


    mov  eax,48
    mov  ebx,3
    mov  ecx,sc
    mov  edx,sizeof.system_colors
    int  0x40

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    int  0x40

                                 ; DRAW WINDOW
  xor  eax,eax                   ; function 0 : define and draw window
  mov  ebx,320*65536+(LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2
  mov  ecx,25*65536+ (LEN_Y-BORDER_TOP-BORDER_BOTTOM)*ADOBE_SIZE+Y_LOCATION+30
  mov  edx,[sc.work]             ; color of work area RRGGBB
  or   edx,0x13000000
  mov  edi,title                 ; WINDOW LABEL
  int  0x40

                                    
    mov eax,8
    mov ebx,30*65536+100
    mov ecx,378*65536+18
    mov edx,2
    mov esi,[sc.work_button]
    int 0x40
;/////////////////////////////////////////////// Wildwest's  'Pause' button
    ;mov eax,8
    mov ebx,132*65536+102
    mov ecx,378*65536+18
    mov edx,3
    mov esi,[sc.work_button];
    int 0x40

    mov  eax,4                      ; function 4 : write text to window
    mov  ebx,164*65536+384          ; [x start] *65536 + [y start]
    mov  ecx,[sc.work_button_text]  ; color of text RRGGBB
    or   ecx,0x90000000
    mov  edx,labe                   ; pointer to text
    int  0x40
;///////////////////////////////////////////////
    ;mov eax,4
    mov ebx,49*65536+384
    mov edx,game_finished
    int 0x40
    call draw_table

    movzx edx,byte [current_block_color]
    call draw_block

    cld
    mov  ebx,38*65536+35           ; draw info text with function 4
    mov  ecx,[sc.work_text]              ; color
    or   ecx,0x90000000    
    mov  edx,text
    mov  eax,4
    int  0x40

    call write_score

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40

    ret

;-------------------------------------------------------------
; CHECK CRASH
; output        Z  flag => OK
;               NZ flag => NO
;-------------------------------------------------------------

check_crash:    mov ebx,[current_block_pointer]

                mov edx,[current_block_y]
                imul edx,LEN_X
                add edx,[current_block_x]          ;find the offset in tetris_t

                add edx,table_tetris

                mov ecx,4
                xor ax,ax

adr_1:          cmp byte [ebx],1
                jne adr_2
                add al,[edx]
                adc ah,0
adr_2:          inc ebx
                inc edx

                cmp byte [ebx],1
                jne adr_3
                add al,[edx]
                adc ah,0
adr_3:          inc ebx
                inc edx

                cmp byte [ebx],1
                jne adr_4
                add al,[edx]
                adc ah,0
adr_4:          inc ebx
                inc edx

                cmp byte [ebx],1
                jne adr_5
                add al,[edx]
                adc ah,0
adr_5:          inc ebx
                add edx,LEN_X-3

                loop adr_1
                or ax,ax
                ret
;-------------------------------------------------------------
;NEW BLOCK
;-------------------------------------------------------------
new_block:      mov dword [current_block_y],1
                mov dword [current_block_x],7

				mov   [force_down],	0				; allow fast falling _after_ first game step

                call random
                and al,7
                setz ah
                add al,ah
                mov [current_block_color],al

                call random
                ;and eax,15
            ; ---- Ivan ----
                and eax,0xff
              @@:
                cmp eax,_MAXBLOCKS_
                jl  @f
                add eax,-(_MAXBLOCKS_)
                jmp @b
              @@:
            ; ---- Ivan ----
                mov edx,[block_table+eax*4]
                mov [current_block_pointer],edx

                mov byte [delay],22 ;19 ;!!! 15
                ret
;-------------------------------------------------------------
; FIX BLOCK
;-------------------------------------------------------------
fix_block:      mov ebx,[current_block_pointer]

                mov edx,[current_block_y]
                imul edx,LEN_X
                add edx,[current_block_x]       ;find the offset in tetris_t

                add edx,table_tetris

                mov ecx,4
                mov al,[current_block_color]

adr_21:         cmp byte [ebx],1
                jne adr_22
                mov [edx],al
adr_22:         inc ebx
                inc edx

                cmp byte [ebx],1
                jne adr_23
                mov [edx],al
adr_23:         inc ebx
                inc edx

                cmp byte [ebx],1
                jne adr_24
                mov [edx],al
adr_24:         inc ebx
                inc edx

                cmp byte [ebx],1
                jne adr_25
                mov [edx],al
adr_25:         inc ebx
                add edx,LEN_X-3

                loop adr_21
                ret

;--------------------------------------------------------------
; DRAW_TABLE
;--------------------------------------------------------------
draw_table:     mov esi,table_tetris+LEN_X*BORDER_TOP+BORDER_LEFT

                mov ebx,X_LOCATION*65536+ADOBE_SIZE
                mov ecx,Y_LOCATION*65536+ADOBE_SIZE
                mov edi,LEN_Y-BORDER_TOP-BORDER_BOTTOM
y_draw:         push edi

                mov edi,LEN_X-BORDER_LEFT-BORDER_RIGHT
x_draw:         push edi
              ;  pusha
              ;  mov eax,5
              ;  mov ebx,10
              ;  int 0x40
              ;  popa
                mov eax,13
                movzx edx,byte [esi]
                mov edx,[color_table+edx*4]
                int 0x40
                call draw_frames
                inc esi
                add ebx,65536*ADOBE_SIZE
                pop edi
                dec edi
                jnz x_draw

                add esi,BORDER_LEFT+BORDER_RIGHT
                mov ebx,X_LOCATION*65536+ADOBE_SIZE
                add ecx,65536*ADOBE_SIZE
                pop edi
                dec edi
                jnz y_draw

                ret
;--------------------------------------------------------------
;DRAW BLOCK
;
; ebx=x [0..LEN_X-1]
; ecx=y [0..LEN_Y-1]
; edi=pointer block
;--------------------------------------------------------------
draw_block: 
              
                mov eax,13
                mov edx,[color_table+edx*4]

                mov ebx,[current_block_x]
                mov ecx,[current_block_y]
                mov edi,[current_block_pointer]

                sub ebx,BORDER_LEFT
                imul ebx,ADOBE_SIZE
                add ebx,X_LOCATION
                shl ebx,16
                mov bx,ADOBE_SIZE

                sub ecx,BORDER_TOP
                imul ecx,ADOBE_SIZE
                add ecx,Y_LOCATION
                shl ecx,16
                mov cx,ADOBE_SIZE

                mov dword [TMP_1],4
adr_122:        mov dword [TMP_0],4
adr_121:        cmp byte [edi],0
                je adr_120

                int 040h

                call draw_frames

adr_120:        inc edi
                add ebx,ADOBE_SIZE*65536
                dec dword [TMP_0]
                jnz adr_121
                sub ebx,4*ADOBE_SIZE*65536
                add ecx,ADOBE_SIZE*65536
                dec dword [TMP_1]
                jnz adr_122

                ret

draw_frames:
                 cmp edx,0
                 jne df1
                 ret
             df1:
                 pusha
                 mov bx,1
                 add edx,0x282828
                 mov eax,13
                 int 0x40
                 popa

                 pusha
                 mov cx,1
                 add edx,0x282828
                 mov eax,13
                 int 0x40
                 popa

                 pusha
                 push ebx
                 sub  bx,1
                 add  [esp+2],bx
                 pop  ebx
                 mov  bx,1
                 shr  edx,1
                 and  edx,0x7f7f7f
                 mov  eax,13
                 int  0x40
                 popa

                 pusha
                 push ecx
                 sub  cx,1
                 add  [esp+2],cx
                 pop  ecx
                 mov  cx,1
                 shr  edx,1
                 and  edx,0x7f7f7f
                 mov  eax,13
                 int  0x40
                 popa

                 ret


;--------------------------------------------------------------
clear_table_tetris:
                cld
                mov al,1
                mov edi,table_tetris
                mov ecx,LEN_X*BORDER_TOP
                rep stosb

                mov edx,LEN_Y-BORDER_TOP-BORDER_BOTTOM
adr300:         mov cl,BORDER_LEFT
                rep stosb
                dec ax  ;AL=0
                mov cl,LEN_X-BORDER_LEFT-BORDER_RIGHT
                rep stosb
                inc ax  ;AL=1
                mov cl,BORDER_RIGHT
                rep stosb
                dec dx
                jne adr300

                mov ecx,LEN_X*BORDER_BOTTOM
                rep stosb
                ret
;--------------------------------------------------------------
;edx = pointer
;ebx = contatore
check_full_line:
                std
                mov al,0
                mov edx,table_tetris+LEN_X*(LEN_Y-BORDER_BOTTOM)-1
                mov ebx,(LEN_Y-BORDER_TOP-BORDER_BOTTOM-1)*LEN_X

adr_5000:       mov edi,edx
                mov ecx,LEN_X-BORDER_LEFT-BORDER_RIGHT+1;Wildwest's fix for bug 
                                 ;(was LEN_X-BORDER_LEFT-BORDER_RIGHT)
                repne scasb
                jz no_full_line

                lea esi,[edx-LEN_X]
                mov edi,edx
                mov ecx,ebx
                rep movsb
                sub edi,BORDER_RIGHT
                mov ecx,LEN_X-BORDER_LEFT-BORDER_RIGHT
                rep stosb
                add dword [score],50
                jmp adr_5000

no_full_line:   sub edx,LEN_X
                sub ebx,LEN_X
                jnz adr_5000

                ret
;--------------------------------------------------------------
random:         mov eax,[generator]
                add eax,-43ab45b5h
                ror eax,1
                xor eax,32c4324fh
                ror eax,1
                mov [generator],eax
                ; --- IVAN ---
                push ebx
                mov eax,26
                mov ebx,9
                int 0x40
                pop ebx
                xor eax,0xdeadbeef
                add eax,[generator]
                ; --- IVAN ---
                mov [generator],eax ; Halyavin
                ret
;--------------------------------------------------------------
number_to_str:  mov edi,end_number_str-1
                mov ecx,9;size_of_number_str
                mov ebx,10
                cld
new_digit:      xor edx,edx
                div ebx
                add dl,'0'
                mov [edi],dl
                dec edi
                loop new_digit
                ret
;--------------------------------------------------------------
write_score:
    mov  eax,[score]
    call number_to_str

    mov  ebx,90*65536+35          ; draw info text with function 4
    mov  ecx,[sc.work_text]        ; color
    or   ecx,0x50000000    
    mov  edx,number_str
    mov  esi,[size_of_number_str]
    mov  edi,[sc.work]
    mov  eax,4
    int  0x40
    ret

; DATA AREA

;--------------------------------------------------------------
;DEFINITION BLOCKS
;--------------------------------------------------------------
t_block_0:  db 0,0,0,0
                db 1,1,1,0
                db 0,1,0,0
                db 0,0,0,0
                dd t_block_3

t_block_1:  db 0,1,0,0
                db 1,1,0,0
                db 0,1,0,0
                db 0,0,0,0
                dd t_block_0

t_block_2:  db 0,1,0,0
                db 1,1,1,0
                db 0,0,0,0
                db 0,0,0,0
                dd t_block_1

t_block_3   db 0,1,0,0
                db 0,1,1,0
                db 0,1,0,0
                db 0,0,0,0
                dd t_block_2
;--------------------------------------------------------------
i_block_0:  db 0,1,0,0
                db 0,1,0,0
                db 0,1,0,0
                db 0,1,0,0
                dd i_block_1

i_block_1:  db 0,0,0,0
                db 1,1,1,1
                db 0,0,0,0
                db 0,0,0,0
                dd i_block_0
;--------------------------------------------------------------
q_block_0: db 0,1,1,0
                db 0,1,1,0
                db 0,0,0,0
                db 0,0,0,0
                dd q_block_0
;--------------------------------------------------------------
s_block_0:
      db 0,0,0,0
      db 0,1,1,0
      db 1,1,0,0
      db 0,0,0,0
      dd s_block_1

s_block_1: db 1,0,0,0
                db 1,1,0,0
                db 0,1,0,0
                db 0,0,0,0
                dd s_block_0
;--------------------------------------------------------------
l_block_0:  db 0,0,0,0
                db 1,1,1,0
                db 1,0,0,0
                db 0,0,0,0
                dd l_block_3

l_block_1:  db 1,1,0,0
                db 0,1,0,0
                db 0,1,0,0
                db 0,0,0,0
                dd l_block_0

l_block_2:  db 0,0,1,0
                db 1,1,1,0
                db 0,0,0,0
                db 0,0,0,0
                dd l_block_1

l_block_3:  db 0,1,0,0
                db 0,1,0,0
                db 0,1,1,0
                db 0,0,0,0
                dd l_block_2
;-----------------------------------------------

g_block_0:

db 0,1,0,0
db 0,1,0,0
db 1,1,0,0
db 0,0,0,0
dd g_block_1

g_block_1:
db 0,0,0,0
db 1,1,1,0
db 0,0,1,0
db 0,0,0,0
dd g_block_2

g_block_2:
db 0,1,1,0
db 0,1,0,0
db 0,1,0,0
db 0,0,0,0
dd g_block_3

g_block_3:
db 1,0,0,0
db 1,1,1,0
db 0,0,0,0
db 0,0,0,0
dd g_block_0

;-----------------------------------

k_block_0:
      db 0,0,0,0
      db 1,1,0,0
      db 0,1,1,0
      db 0,0,0,0
      dd k_block_1

k_block_1:
      db 0,1,0,0
      db 1,1,0,0
      db 1,0,0,0
      db 0,0,0,0
      dd k_block_0

;-----------------------------------

color_table: dd 00000000h    ;black      0
                  dd 00cccccch    ;white      1
                  dd 00cc0000h    ;red         2
                  dd 0000cc00h    ;green     3
                  dd 000000cch    ;blue        4
                  dd 00cccc00h    ;yellow     5
                  dd 0000cccch    ;cyan      6
                  dd 00cc00cch    ;pink        7

block_table:
    dd t_block_0 ; t
    dd t_block_1
    dd t_block_2
    dd t_block_3
    dd i_block_0 ; i
    dd i_block_1
    dd i_block_0
    dd i_block_1
    dd q_block_0 ; q
    dd q_block_0
    dd q_block_0
    dd q_block_0
    dd s_block_0 ; s
    dd s_block_1
    dd s_block_0
    dd s_block_1
    dd l_block_0 ; l
    dd l_block_1
    dd l_block_2
    dd l_block_3
    dd g_block_0 ; g
    dd g_block_1
    dd g_block_2
    dd g_block_3
    dd k_block_0 ; k
    dd k_block_1
    dd k_block_0
    dd k_block_1

if lang eq ru

  title          db 'íÖíêàë 1.61 - ëíêÖãäà à èêéÅÖã',0
  labe           db 'èÄìáÄ',0
  text           db 'éÁ™®:',0
  game_finished: db '  çéÇÄü',0

else if lang eq et

  title          db 'TETRIS 1.61 - NOOLED & T‹HIK',0
  labe           db 'PAUS',0
  text           db 'Tulemus:',0
  game_finished: db 'UUS MƒNG',0

else

  title          db 'TETRIS 1.61 - ARROWS & SPACE',0
  labe           db 'PAUSE',0
  text           db 'Score:',0
  game_finished: db 'NEW GAME',0

end if

pauses dd 0  ;/// Wildwest
parallel dd 1
score:                  dd 0
TMP_0:                  dd 0
TMP_1:                  dd 0
generator:              dd 0
current_block_x:        dd 0
current_block_y:        dd 0
current_block_pointer:  dd 0
current_block_color:    db 0
number_str:             db 0,0,0,0,0,0,0,0,0
end_number_str:
size_of_number_str      dd 9
delay:                  db 40
force_down				dd 1
sc     system_colors
table_tetris:

I_END:

