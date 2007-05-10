;
;   Modified from original icon editor
;
;   Compile with FASM for Menuet
;
   use32
   org	  0x0
   db	  'MENUET01'		  ; 8 byte id
   dd	  0x01			  ; header version
   dd	  START 		  ; start of code
   dd	  I_END 		  ; size of image
   dd	  0x100000		  ; memory for app
   dd	  0x7fff0		  ; esp
   dd	  0x0 , 0x0		  ; I_Param , I_Icon

include 'lang.inc'
include '..\..\..\macros.inc'

window_x_size  equ    346
window_y_size  equ    312

START:				; start of execution

    call draw_window		; at first, draw the window
    call get_screen_size	; get screen x and y size

check_mouse:

    call draw_mouse		; are we editing the icon
    call check_colour		; are we selecting a colour

still:

    mov  eax,23 		; wait here for event
    mov  ebx,1			; for 1ms
    mcall

    cmp  eax,1			; redraw request ?
    je	 red
    cmp  eax,2			; key in buffer ?
    je	 key
    cmp  eax,3			; button in buffer ?
    je	 button

    jmp  check_mouse		; start loop again

  red:				; redraw
    call draw_window		; draw our window
    jmp  check_mouse		; start the loop again
    key:

    mov  eax,2			; get a key
    mcall			; do it
    shr  eax,8			; move it to al
    cmp  byte [editstate],0	; are we in edit mode
    je	 check_mouse		; if not start loop again
    cmp  al,8			; is it a backspace
    jne  no_bksp		; if no jump over backspace code
    cmp  byte [editpos],0	; is it the first character
    je	 no_del_last		; if yes jump over backspace code
    dec  byte [editpos] 	; decrement our position
    xor  ebx,ebx		; clear pointer
    mov  bl,byte [editpos]	; get our position
    add  ebx,icon		; add our offset
    mov  byte [ebx],0		; delete character
no_del_last:
    call draw_filename		; update filename
    jmp  check_mouse		; start loop again
no_bksp:
    cmp  al,13			; is it the enter key
    jne  no_enter		; no then jump over enter code
    mov  byte [editstate],0	; get out of edit mode
    call draw_filename		; update filename
    jmp check_mouse		; start loop again
no_enter:
    cmp  al,31			; are we below character 31
    jbe  no_lit 		; then must be illegal
    cmp  al,97			; are we below character 97
    jb	 capital		; then must be ok
    sub  eax,32 		; else subtract 32 from it to make it legal
capital:
    xor  ebx,ebx		; clear our pointer
    mov  bl,byte [editpos]	; get our position
    add  ebx,icon		; add our offset
    mov  byte [ebx],al		; move our character into buffer
    inc  byte [editpos] 	; increment our edit position
    cmp  byte [editpos],12	; are we at our last position
    jne  no_null_state		; no then jump over last position code
    mov  byte [editstate],0	; get out of edit mode
no_null_state:
    call draw_filename		; update filename
no_lit:
    jmp  check_mouse		; start loop again

  button:			; button
    mov  eax,17 		; get id
    mcall

    cmp  ah,1			; button id=1 ?
    jne  button_3
    mov  eax,-1 		; close this program
    mcall
  button_3:
    cmp  ah,3			; was it button 3 - FILENAME
    jne  button_4		; no then try button 4
    cld 			;
    mov  byte [editstate],1	; enter edit mode
    mov  edi,icon		; point to our buffer
    mov  eax,0x20202020 	; file reg with 4 spaces
    mov  ecx,3			; going to write it 3 times
    rep  stosd			; repeat giving 12 spaces
    mov  byte [editpos],0	; zero our edit position
    call draw_filename		; update filename
    jmp  check_mouse		; start loop again
  button_4:
    cmp  ah,4			; was it button 4 - LOAD
    jne  button_5		; no then try button 5
    mov  byte [editstate],0	; dont want to be in edit mode
    call draw_filename		; update filename
    call load_file		; load the file
    call draw_icon		; update icon screen
    call draw_graph		; update edit screen
    jmp  check_mouse		; start loop again
  button_5:
    cmp  ah,5			; was it button 5 - SAVE
    jne  button_6		; no then try button 6
    mov  byte [editstate],0	; dont want to be in edit mode
    call draw_filename		; update filename
    call save_file		; save the file
    jmp  check_mouse		; start loop again
  button_6:
    cmp  ah,6			; was it button 6 - CLEAR ICON
    jne  button_7		; no then try button 7
    mov  byte [editstate],0	; dont want to be in edit mode
    call draw_filename		; update filename
    call clear_graph_icon      ; clear the icon and edit screens
    jmp  check_mouse

  button_7:
    cmp  ah,7			; was it button 7 - FLIP ICON
    jne  button_8		; no then try button 8
    call flip_icon		; flip
    jmp  check_mouse

  button_8:
    cmp  ah,8			; was it button 8 - MIRROR ICON
    jne  button_9		; no then try button 9
    call flip_diag		; flip L/R and U/D
    call flip_icon		; cheated now have to flip it
    jmp  check_mouse

  button_9:
    cmp  ah,9			; was it button 9 - FLIP L/R and U/D
    jne  button_10		; no then try button 10
    call flip_diag		; flip L/R and U/D
    call draw_icon		; update icon
    call draw_graph		; update graph
    jmp  check_mouse

  button_10:
    cmp  ah,10			; was it button 9 - SET AS BGR
    jne  check_mouse		; no then exit
    call set_background 	; set as background

    jmp  check_mouse		; start loop again

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
draw_window:

    mov  eax,12 		; function 12:tell os about windowdraw
    mov  ebx,1			; 1, start of draw
    mcall
				; DRAW WINDOW
    mov  eax,0			; function 0 : define and draw window
    mov  ebx,100*65536+window_x_size	    ; [x start] *65536 + [x size]
    mov  ecx,100*65536+window_y_size	    ; [y start] *65536 + [y size]
    mov  edx,0x13ffffff 	; color of work area 0x00RRGGBB
    mov  edi,title 		; WINDOW LABEL
    mcall
			
    mov  eax,13 		; function 13 : draw bar
    mov  ebx,5*65536+window_x_size-9	    ; [x start] *65536 + [x size]
    mov  ecx,(window_y_size-20)*65536+16    ; [y start] *65536 + [y size]
    mov  edx,0x7799bb		; colour 0x00RRGGBB
    mcall
				; DEFINE BUTTON 3 : FILENAME
    mov  eax,8			; function 8 : define button
    mov  ebx,5*65536+62 	; [x start] *65536 + [x size]
    mov  ecx,(window_y_size-19)*65536+14     ; [y start] *65536 + [y size]
    mov  edx,3			; button id number
    mov  esi,0x7799bb		; button color 0x00RRGGBB
    mcall
				; BUTTON 3 TEXT
    mov  eax,4			; function 4 : write text to window
    mov  ebx,13*65536+window_y_size-15	     ; [x start] *65536 + [y start]
    mov  ecx,0xddeeff		; text color 0x00RRGGBB
    mov  edx,button_text_3	; pointer to text beginning
    mov  esi,8			; text length
    mcall

    call draw_filename		; update filename

				; DEFINE BUTTON 4 : LOAD
    mov  eax,8			; function 8 : define button
    mov  ebx,(window_x_size-87)*65536+38     ; [x start] *65536 + [x size]
    mov  ecx,(window_y_size-19)*65536+14     ; [y start] *65536 + [y size]
    mov  edx,4			; button id number
    mov  esi,0x7799bb		; button color 0x00RRGGBB
    mcall
				; DEFINE BUTTON 5 : SAVE
    mov  ebx,(window_x_size-43)*65536+38     ; [x start] *65536 + [x size]
    mov  edx,5			; button id number
    mcall
				; DEFINE BUTTON 6 : CLEAR ICON
    mov  ebx,268*65536+72	; [x start] *65536 + [x size]
    mov  ecx,65*65536+14	; [y start] *65536 + [y size]
    mov  edx,6			; button id number
    mcall
				; DEFINE BUTTON 7 : FLIP VERTICAL
    mov  ecx,85*65536+14	; [y start] *65536 + [y size]
    mov  edx,7			; button id number
    mcall
				; DEFINE BUTTON 8 : FLIP HORIZONTAL
    mov  ecx,105*65536+14	; [y start] *65536 + [y size]
    mov  edx,8			; button id number
    mcall
				; DEFINE BUTTON 9 : SET AS BACKGROUND
    mov  ecx,125*65536+14	; [y start] *65536 + [y size]
    mov  edx,9			; button id number
    mcall
				; DEFINE BUTTON 10 : SET AS BACKGROUND
    mov  ecx,145*65536+14	; [y start] *65536 + [y size]
    mov  edx,10 		; button id number
    mcall
				; BUTTON 4 TEXT
    mov  eax,4			; function 4 : write text to window
    mov  ebx,267*65536+window_y_size-15      ; [x start] *65536 + [y start]
    mov  ecx,0xddeeff		; text color 0x00RRGGBB
    mov  edx,button_text_4	; pointer to text beginning
    mov  esi,4			; text length
    mcall
				; BUTTON 5 TEXT
    mov  ebx,311*65536+window_y_size-15      ; [x start] *65536 + [y start]
    mov  edx,button_text_5	; pointer to text beginning
    mcall
				; BUTTON 6 TEXT
    mov  ebx,275*65536+69	; [x start] *65536 + [y start]
    mov  edx,button_text_6	; pointer to text beginning
    mov  esi,10 		; text length
    mcall
				; BUTTON 7 TEXT
    mov  ebx,275*65536+89	; [x start] *65536 + [y start]
    mov  edx,button_text_7	; pointer to text beginning
    mcall
				; BUTTON 8 TEXT
    mov  ebx,275*65536+109	; [x start] *65536 + [y start]
    mov  edx,button_text_8	; pointer to text beginning
    mcall
				; BUTTON 9 TEXT
    mov  ebx,275*65536+129	; [x start] *65536 + [y start]
    mov  edx,button_text_9	; pointer to text beginning
    mcall
				; BUTTON 10 TEXT
    mov  ebx,275*65536+149	; [x start] *65536 + [y start]
    mov  edx,button_text_10	; pointer to text beginning
    mcall
				; DRAW GRAPH BACKGROUND
    mov  eax,13 		; function 13 : draw bar
    mov  ebx,6*65536+257	; [x start] *65536 + [x size]
    mov  ecx,26*65536+257	; [y start] *65536 + [y size]
    mov  edx,0x7799bb		; colour 0x00RRGGBB
    mcall
				; DRAW ICON BACKGROUND
    mov  ebx,268*65536+34	; [x start] *65536 + [x size]
    mov  ecx,26*65536+34	; [y start] *65536 + [y size]
    mcall
				; DRAW PALETTE BACKGROUND
    mov  ebx,268*65536+34	; [x start] *65536 + [x size]
    mov  ecx,217*65536+33	; [y start] *65536 + [y size]
    mcall
				; DRAW PALETTE BACKGROUND
    mov  ebx,268*65536+33	; [x start] *65536 + [x size]
    mov  ecx,250*65536+33	; [y start] *65536 + [y size]
    mcall
				; DRAW PALETTE BACKGROUND
    mov  ebx,301*65536+33	; [x start] *65536 + [x size]
    mov  ecx,249*65536+34	; [y start] *65536 + [y size]
    mcall
				; DRAW GREYSCALE BACKGROUND
    mov  ebx,307*65536+34	; [x start] *65536 + [x size]
    mov  ecx,210*65536+34	; [y start] *65536 + [y size]
    mcall
				; DRAW SELECTED COLOUR BACKGROUND
    mov  ebx,268*65536+73	; [x start] *65536 + [x size]
    mov  ecx,171*65536+34	; [y start] *65536 + [y size]
    mcall
				; DRAW WHITE TO START
    mov  ebx,269*65536+71	; [x start] *65536 + [x size]
    mov  ecx,172*65536+32	; [y start] *65536 + [y size]
    mov  edx,0xffffff		; colour 0x00RRGGBB
    mcall
				; DRAW PALETTE RED + GREEN SECTION
    xor  edi,edi		; clear column loop count
next_r_g_outer:
    xor  esi,esi		; clear line loop count
next_r_g_inner:
    mov  eax,1			; function 1 : putpixel
    mov  ebx,edi		; get column count
    shr  ebx,3			; divide by 8
    add  ebx,269		; add our x offset
    mov  ecx,esi		; get our line count
    shr  ecx,3			; divide by 8
    add  ecx,218		; add our y offset
    mov  edx,255		; maximum red
    sub  edx,edi		; minus column count
    shl  edx,8			; into position 0x0000RR00
    add  edx,255		; maximum green
    sub  edx,esi		; minus line count
    shl  edx,8			; into position 0x00RRGG00
    mcall
    add  esi,5			; colour in steps of 5 to keep small
    cmp  esi,255		; have we done a line
    jne  next_r_g_inner 	; nop then do next pixel
    add  edi,5			; colour in steps of 5 to keep small
    cmp  edi,255		; have we done all columns
    jne  next_r_g_outer 	; no then start the next one

				; DRAW PALETTE GREEN + BLUE SECTION
    mov  edi,0			; as above
next_g_b_outer:
    mov  esi,0
next_g_b_inner:
    mov  eax,1
    mov  ebx,edi
    shr  ebx,3
    add  ebx,269
    mov  ecx,esi
    shr  ecx,3
    add  ecx,250
    mov  edx,255
    sub  edx,edi
    shl  edx,16
    add  edx,esi
    mcall
    add  esi,5
    cmp  esi,255
    jne  next_g_b_inner
    add  edi,5
    cmp  edi,255
    jne  next_g_b_outer

				; DRAW PALETTE BLUE + RED SECTION
    mov  edi,0			; as above
next_b_r_outer:
    mov  esi,0
next_b_r_inner:
    mov  eax,1
    mov  ebx,edi
    shr  ebx,3
    add  ebx,301
    mov  ecx,esi
    shr  ecx,3
    add  ecx,250
    mov  edx,edi
    shl  edx,8
    add  edx,esi
    mcall
    add  esi,5
    cmp  esi,255
    jne  next_b_r_inner
    add  edi,5
    cmp  edi,255
    jne  next_b_r_outer

				; DRAW GREYSCALE
    mov  edi,0
next_b_w_outer:
    mov  esi,0
next_b_w_inner:
    mov  eax,1
    mov  ebx,edi
    shr  ebx,3
    add  ebx,308
    mov  ecx,esi
    shr  ecx,3
    add  ecx,211
    mov  edx,esi
    shl  edx,8
    add  edx,esi
    shl  edx,8
    add  edx,esi
    mcall
    add  esi,5
    cmp  esi,255
    jne  next_b_w_inner
    add  edi,5
    cmp  edi,255
    jne  next_b_w_outer

    cmp  [first_run],0		; is it the first window draw
    jne  dont_load		; no then dont reload the file
    call load_file		; load initial file
    mov  [first_run],1		; first window draw done
dont_load:
    call draw_icon		; draw icon area
    call draw_graph		; draw edit area

    mov  eax,12 		; function 12:tell os about windowdraw
    mov  ebx,2			; 2, end of draw
    mcall

    ret 			; return

;   *********************************************
;   *******  SET BACKGROUND              ********
;   *********************************************
set_background:

    mov  ecx,image+3600 	; point to our decode buffer
    mov  edx,image+54+32*32*3-96    ; point to last line of bmp

    mov  edi,0			; zero line count
decode_line:
    mov  esi,0			; zero column count
decode_column:
    mov  ebx,[edx]		; get our data
    mov  [ecx],ebx		; store our data
    add  ecx,4			; add 4 bytes to pointer as using double word
    add  edx,4			; add 4 bytes to pointer
    inc  esi			; increment column count
    cmp  esi,24 		; have we done all columns
    jne  decode_column		; no then do some more
    sub  edx,192		; move back 2 lines of bmp data
    inc  edi			; increment line count
    cmp  edi,33 		; have we done all lines
    jne  decode_line		; no then do another one

    mov  eax,15 		; background
    mov  ebx,1			; set background size
    mov  ecx,32 		; x size
    mov  edx,32 		; y size
    mcall			; do it
    mov  ebx,4			; type of background draw
    mov  ecx,1			; tile
    mcall			; do it
    mov  ebx,5			; blockmove image to os bgr memory
    mov  ecx,image+3600 	; point to image
    mov  edx,0			; start of bgr memory
    mov  esi,32*32*3		; byte count
    mcall			; do it
    mov  ebx,3			; draw background
    mcall			; do it

    ret 			; return

;   *********************************************
;   *******  GET SCREEN X and Y SIZE     ********
;   *********************************************
get_screen_size:

    mov  eax,14 		; function 14 : get screen max
    mcall			; returns eax : 0xXXXXYYYY
    push eax			; save reg
    and  eax,0x0000ffff 	; split out y size
    add  eax,1			; add 1 to remove zero base
    mov  [y_size],eax		; store for later
    pop  eax			; restore reg
    shr  eax,16 		; move x size down the reg
    add  eax,1			; add 1 to remove zero base
    mov  [x_size],eax		; store for later

    ret

;   *********************************************
;   *******  LOAD FILE                   ********
;   *********************************************
load_file:

    mov  eax,6			; function 6 : load file
    mov  ebx,icon		; point to the name
    mov  ecx,0			; reserved
    mov  edx,200000		; reserved
    mov  esi,image		; point to image buffer
    mcall			; do it

    ret 			; return

;   *********************************************
;   *******  SAVE FILE                   ********
;   *********************************************
save_file:

    mov  eax,33 		; function 33 : save file
    mov  ebx,icon		; point to name
    mov  ecx,image		; point to our data
    mov  edx,54+32*32*3 	; how much data to transfer
    xor  esi,esi		; 0 - create a new file
    mcall			; do it

    ret 			; return

;   *********************************************
;   *******  DRAW FILENAME TO SCREEN     ********
;   *********************************************
draw_filename:
				; DRAW COLOUR BACKGROUND
    pusha			; save all registers on stack
    mov  eax,13 		; function 13 : draw bar
    mov  ebx,73*65536+81	; [x start] *65536 + [x size]
    mov  ecx,(window_y_size-19)*65536+15     ; [y start] *65536 + [y size]
    mov  edx,0x557799		; colour 0x00RRGGBB
    mcall

    mov  eax,4			; function 4 : write text to window
    mov  ebx,78*65536+(window_y_size-15)     ; [y start] *65536 + [y size]
    xor  ecx,ecx		; colour 0x00RRGGBB = black
    mov  edx,icon		; point to text
    mov  esi,12 		; number of characters to write
    cmp  byte [editstate],1	; are we in edit mode
    jne  no_active_1		; no then jump over change colour
    mov  ecx,0x00112299 	; else change colour
no_active_1:
    mcall			; do it
    popa			; return all registers to original
    ret 			; return

;   *********************************************
;   *******  DRAW ICON FROM LOADED FILE  ********
;   *********************************************
draw_icon:

    mov  ecx,27 		; y start position
    mov  eax,image+51+32*32*3	; point to our bmp data
    mov  edi,0			; line count
line:
    mov  esi,0			; column count
    mov  ebx,269+31		; x start position
pixel:
    mov  edx,[eax]		; colour 0x00RRGGBB from image data
    push eax			; save our position
    mov  eax,1			; function 1 : put pixel
    mcall			; do it
    pop  eax			; restore our position
    sub  eax,3			; point to next pixel colour
    dec  ebx			; move our x position
    inc  esi			; increment our column count
    cmp  esi,32 		; have we done all columns
    jne  pixel			; no then do next pixel
    inc  ecx			; move our y position
    inc  edi			; increment line count
    cmp  edi,32 		; have we done all lines
    jne  line			; no then do next line

    ret 			; return

;   *********************************************
;   *******  DRAW GRAPH FROM LOADED FILE  *******
;   *********************************************
draw_graph:

    mov  edi,0			; line count
    mov  eax,image+51+32*32*3	; point to our bmp data
next_lin:
    mov  ecx,edi		; use our line count for position
    shl  ecx,3			; multiply by eight
    add  ecx,27 		; add y start position
    shl  ecx,16 		; move into top eight bytes [y start]
    add  ecx,7			; add box height [y size]
    mov  esi,32 		; column count
next_bo:
    mov  ebx,esi		; use our column count for position
    shl  ebx,3			; multiply by eight
    add  ebx,-1 		; add a correction
    shl  ebx,16 		; and move into top eight bytes [x start]
    add  ebx,7			; add box width [x size]
    mov  edx,[eax]		; point to bmp data
    push eax			; save our position
    mov  eax,13 		; function 13 : draw bar
    mcall			; do it
    pop  eax			; restore our position
    sub  eax,3			; point to next pixel colour
    dec  esi			; increment column count
    cmp  esi,0			; have we done all columns
    jne  next_bo		; no then do the next column
    inc  edi			; increment line count
    cmp  edi,32 		; have we done all lines
    jne  next_lin		; no then do the next line

    ret 			; return

;   *********************************************
;   *******  CLEAR GRAPH and ICON AREA   ********
;   *********************************************
clear_graph_icon:

				; CLEAR IMAGE DATA
    mov  edi,image+54		; point to our data
    mov  eax,0x00000000 	; data to write
    mov  ecx,(32*32*3)/4	; how much data
    rep  stosd			; repeat until all data transfered
    call draw_icon		; draw a blank icon
    call draw_graph		; draw a blank graph

    ret 			; return

;   *********************************************
;   *******  FLIP ICON TOP to BOTTOM     ********
;   *********************************************
flip_icon:

    mov  ecx,image+54		 ; point at first line
    mov  edx,image+54+32*32*3+96 ; point 1 line past end

    mov  edi,0			 ; zero line count
lines:
    mov  esi,0			 ; zero column count
    sub  edx,192		 ; move back 2 lines
columns:
    mov  eax,[ecx]		 ; get bytes
    mov  ebx,[edx]		 ; get bytes
    mov  [ecx],ebx		 ; swap bytes
    mov  [edx],eax		 ; swap bytes
    add  ecx,4			 ; move pointer
    add  edx,4			 ; move pointer
    inc  esi			 ; increment column count
    cmp  esi,24 		 ; have we done all columns
    jne  columns		 ; no then do next column
    inc  edi			 ; increment line count
    cmp  edi,16 		 ; have we done all lines
    jne  lines			 ; no then do next line
    call draw_icon		 ; update icon
    call draw_graph		 ; update graph

    ret 			 ; return

;   *********************************************
;   *******  FLIP ICON DIAGONAL          ********
;   *********************************************
flip_diag:

    mov  ecx,image+54		 ; point line 1 first bytes
    mov  edx,image+3600 	 ; point to  buffer
    xor  esi,esi		 ; zero byte count

    pusha			 ; save all registers
copy_out:
    mov  eax,[ecx]		 ; get bytes
    mov  [edx],eax		 ; copy bytes
    add  ecx,4			 ; move pointer
    add  edx,4			 ; move pointer
    inc  esi			 ; increment byte count
    cmp  esi,24*32		 ; have we done all bytes
    jne  copy_out		 ; no then do the next
    popa			 ; restore all registers

    mov  edx,image+3600+32*32*3-3	; point to last bytes
copy_in:
    mov  eax,[edx]		 ; get bytes
    mov  [ecx],eax		 ; copy to image first bytes
    add  ecx,3			 ; move pointer 3 bytes
    sub  edx,3			 ; move pointer 3 bytes
    inc  esi			 ; increment byte count
    cmp  esi,32*32		 ; have we done all bytes
    jne  copy_in		 ; no then do next

    ret 			 ; return

;   *********************************************
;   *******  DRAW MOUSE ON GRAPH / ICON  ********
;   *********************************************
draw_mouse:

    mov  eax,37 		; function 37
    mov  ebx,2			; sub function 2 : check for mouse button
    mcall			; do it
    cmp  eax,0			; was a button pressed
    je	 exit_mouse		; no then jump to exit
    cmp  eax,1			; left hand button
    je	 colour_mouse		; we are using selected colour
    mov  [draw_colour],0x000000 ; else draw with black
    jmp  blank			; jmp over colour select
colour_mouse:
    mov  eax,[sel_colour]	; get our selected colour
    mov  [draw_colour],eax	; save our selected colour
blank:
    mov  eax,37 		; function 37
    mov  ebx,1			; sub function 1 : get mouse position
    mcall			; do it
    push eax			; save our x/y position for later
    and  eax,0x0000ffff 	; y is in lower bytes
    add  eax,1			; add 1 because we are zero based
    mov  [my_pos],eax		; save y position
    pop  eax			; restore our x/y position
    shr  eax,16 		; shift x into lower bytes
    add  eax,1			; add 1 because we are zero based
    mov  [mx_pos],eax		; save x position
    cmp  [mx_pos],7		; check we are within x/y limits
    jle   exit_mouse
    cmp  [mx_pos],263
    jge   exit_mouse
    cmp  [my_pos],27
    jle   exit_mouse
    cmp  [my_pos],283
    jge   exit_mouse
    mov  eax,[mx_pos]		; calculate nearest square and save
    sub  eax,7			; subtract 7 graph zero offset
    shr  eax,3			; divide by 8 (box size) loose remainder
    mov  [icon_x],eax		; save for use later in icon
    shl  eax,3			; multiply by 8 to get box position
    add  eax,7			; add 7 graph zero offset
    mov  [gx_pos],eax		; save graph x position
    mov  eax,[my_pos]		; repeat for y
    sub  eax,27 		;
    shr  eax,3			;
    mov  [icon_y],eax		;
    shl  eax,3			;
    add  eax,27 		;
    mov  [gy_pos],eax		;
    mov  eax,13 		; function 13 : draw bar
    mov  ebx,[gx_pos]		; load graph x position
    shl  ebx,16 		; shift into high bytes
    add  ebx,7			; add box size
    mov  ecx,[gy_pos]		; repeat for y
    shl  ecx,16 		;
    add  ecx,7			;
    mov  edx,[draw_colour]	; give it a colour
    mcall			; do it
    mov  eax,1			; function 1 : put pixel
    mov  ebx,[icon_x]		; icon x from above
    add  ebx,269		; add icon x offset
    mov  ecx,[icon_y]		; icon y from above
    add  ecx,27 		; add icon y offset
    mov  edx,[draw_colour]	; give it a colour
    mcall			; do it
    mov  ecx,image+54		; point to our data
    mov  ebx,31 		; 32 lines in image zero based
    sub  ebx,[icon_y]		; get the correct line
    mov  eax,96 		; 96 or 3 bytes per colour * 32 columns
    mul  ebx			; multiply by 96 result in eax
    add  ecx,eax		; add to our position
    mov  ebx,[icon_x]		; get the correct column
    mov  eax,3			; 3 bytes per colour
    mul  ebx			; multiply by 3 result in eax
    add  ecx,eax		; add to our position
    mov  ebx,[draw_colour]	; get our colour
    mov  [ecx],bl	      ; move blue into image data
    mov  [ecx+1],bh		; move green into image data
    shr  ebx,16 		; shift red down
    mov  [ecx+2],bl		  ; move red into image data
exit_mouse:

    ret 			; return

;   *********************************************
;   *******  GET COLOUR TO DRAW WITH     ********
;   *********************************************
check_colour:

    mov  eax,37 		; function 37
    mov  ebx,2			; sub function 2 : check for mouse button
    mcall			; do it
    cmp  eax,0			; was a button pressed
    je	 exit_draw		; no then jump to exit
    mov  eax,37 		; function 37
    mov  ebx,1			; sub function 1 : get mouse position
    mcall			; do it
    push eax			; save our x/y position for later
    and  eax,0x0000ffff 	; y is in lower bytes
    add  eax,1			; add 1 because we are zero based
    mov  [my_pos],eax		; save y position
    pop  eax			; restore our x/y position
    shr  eax,16 		; shift x into lower bytes
    add  eax,1			; add 1 because we are zero based
    mov  [mx_pos],eax		; save x position
    cmp  [mx_pos],270		; check we are within x/y limits
    jl	 check_rb
    cmp  [mx_pos],301
    jg	 check_rb
    cmp  [my_pos],219
    jl	 check_rb
    cmp  [my_pos],250
    jg	 check_rb

    call decode_mouse

    mov  edi,0
next_sel_rg_outer:
    mov  esi,0
next_sel_rg_inner:
    mov  eax,1
    mov  ebx,edi
    shr  ebx,3
    add  ebx,308
    mov  ecx,esi
    shr  ecx,3
    add  ecx,211
    mov  edx,[sel_colour]
    add  edx,esi
    mcall
    add  esi,5
    cmp  esi,255
    jne  next_sel_rg_inner
    add  edi,5
    cmp  edi,255
    jne  next_sel_rg_outer


check_rb:

    cmp  [mx_pos],270		; check we are within x/y limits
    jl	 check_bg
    cmp  [mx_pos],301
    jg	 check_bg
    cmp  [my_pos],251
    jle  check_bg
    cmp  [my_pos],282
    jg	 check_bg

    call decode_mouse

    mov  edi,0
next_sel_rb_outer:
    mov  esi,0
next_sel_rb_inner:
    mov  ebx,edi
    shr  ebx,3
    add  ebx,308
    mov  ecx,esi
    shr  ecx,3
    add  ecx,211
    mov  edx,[sel_colour]
    mov  eax,esi
    shl  eax,8
    add  edx,eax
    mov  eax,1
    mcall
    add  esi,5
    cmp  esi,255
    jne  next_sel_rb_inner
    add  edi,5
    cmp  edi,255
    jne  next_sel_rb_outer


check_bg:

    cmp  [mx_pos],301		; check we are within x/y limits
    jl	 get_colour
    cmp  [mx_pos],333
    jg	 get_colour
    cmp  [my_pos],251
    jl	 get_colour
    cmp  [my_pos],282
    jg	 get_colour

    call decode_mouse

    mov  edi,0
next_sel_bg_outer:
    mov  esi,0
next_sel_bg_inner:
    mov  ebx,edi
    shr  ebx,3
    add  ebx,308
    mov  ecx,esi
    shr  ecx,3
    add  ecx,211
    mov  edx,[sel_colour]
    mov  eax,esi
    shl  eax,16
    add  edx,eax
    mov  eax,1
    mcall
    add  esi,5
    cmp  esi,255
    jne  next_sel_bg_inner
    add  edi,5
    cmp  edi,255
    jne  next_sel_bg_outer

get_colour:

    cmp  [mx_pos],309		; check we are within x/y limits
    jl	 exit_draw
    cmp  [mx_pos],340
    jg	 exit_draw
    cmp  [my_pos],212
    jl	 exit_draw
    cmp  [my_pos],243
    jg	 exit_draw

    call decode_mouse

    mov  eax,13
    mov  ebx,269*65536+71
    mov  ecx,172*65536+32
    mov  edx,[sel_colour]
    mcall

    mov  eax,[sel_colour]
    mov  [draw_colour],eax

    mov  eax,47
    xor  ebx,ebx
    mov  ebx,6
    shl  ebx,16
    mov  bh,1
    mov  ecx,[sel_colour]
    mov  edx,273*65536+176
    mov  esi,0x000000
    mcall

exit_draw:

    ret

;   *********************************************
;   *******  DECODE MOUSE POSITION GET PIX  *****
;   *********************************************

decode_mouse:

    mov  eax,37
    xor  ebx,ebx
    mcall
    mov  ebx,eax
    mov  ecx,eax
    and  ebx,0xffff0000
    shr  ebx,16
    and  ecx,0x0000ffff
    mov  eax,[x_size]
    imul ecx,eax
    add  ebx,ecx
    mov  eax,35
    dec  ebx
    mcall

    mov  [sel_colour],eax

    ret

;   *********************************************
;   *******  DATA AREA                      *****
;   *********************************************

mx_pos		  dd  0x0
my_pos		  dd  0x0

gx_pos		  dd  0x0
gy_pos		  dd  0x0

icon_x		  dd  0x0
icon_y		  dd  0x0

x_size		  dd  0x0
y_size		  dd  0x0

sel_colour	  dd  0x00ffffff
draw_colour	  dd  0x00ffffff

button_text_3	  db  'FILENAME'
button_text_4	  db  'LOAD'
button_text_5	  db  'SAVE'
button_text_6	  db  'CLEAR ICON'
button_text_7	  db  'FLIP VERT '
button_text_8	  db  'FLIP HORIZ'
button_text_9	  db  'FLIP DIAG '
button_text_10	  db  'SET AS BGR'

title   	  db  'ICON EDITOR',0

icon:		  db  'WRITE.BMP   '

editpos 	  db  0
editstate	  db  0

first_run	  db  0

image:

I_END:
