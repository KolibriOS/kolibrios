;
;   MENU EXAMPLE
;
;   Compile with FASM for Menuet
;

  use32
  org    0x0

  db     'MENUET01'    ; 8 byte id
  dd     0x01          ; header version
  dd     START         ; start of code
  dd     I_END         ; size of image
  dd     0x1000        ; memory for app
  dd     0x1000        ; esp
  dd     0x0 , 0x0     ; I_Param , I_Icon

include  'lang.inc'
include  '..\..\..\..\macros.inc'

START:                             ; start of execution

  red: 
    call draw_window               ; draw window
    call clear_data                ; clear status bar

still:

    mov  eax,10                    ; wait here for event
    mcall                      ; do it

    cmp  eax,1                     ; redraw request ?
    jz   red                       ; yes jump to it
    cmp  eax,2                     ; key in buffer ?
    jnz   button

  key:                             ; key
    mov  eax,2                     ; just read it and ignore
    mcall                      ; do it
    jmp  still                     ; start again

  button:                          ; button
    mov  eax,17                    ; get id
    mcall                      ; do it

    cmp  ah,1                      ; is it the close button
    jne  noclose                   ; no then jump code
    or   eax,-1                    ; close this program
    mcall                      ; do it
noclose:

    cmp  ah,100                    ; is it main menu
    jb   not_menu                  ; no then jump code
    cmp  ah,104                    ; is it main menu
    ja   not_menu                  ; no then jump code
    call draw_window               ; redraw window
    call clear_data                ; clear status info
    call draw_data                 ; update status info
    call write_sub                 ; draw a sub menu
    jmp  still                     ; start again
not_menu:

    cmp  ah,110                    ; is it a sub menu
    jb   not_sub                   ; no then jump code
    cmp  ah,145                    ; is it a sub menu
    ja   not_sub                   ; no then jump code
    call draw_window               ; redraw window
    call clear_data                ; clear status info
    mov  [button_press],1          ; sub button pressed
    call draw_data                 ; update status info
    mov  [button_press],0          ; clear pressed
    jmp  still                     ; start again
not_sub:

    jmp  still                     ; start again


;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************

draw_window:

    push eax                       ; save register

    mov  eax,12                    ; function 12: tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    mcall                      ; do it

    mov  eax,0                     ; function 0: define and draw window
    mov  ebx,50*65536              ; [x start] *65536
    add  ebx,[x_size]              ; add [x size]
    mov  ecx,50*65536              ; [y start] *65536
    add  ecx,[y_size]              ; add [y size]
    mov  edx,0x80ffffff            ; colour of work area RRGGBB
    mov  esi,0x806688dd            ; grab bar colour. negative glide
    mcall                      ; do it

    mov  eax,4                     ; function 4: write text to window
    mov  ebx,6*65536+7             ; [x start] *65536 + [y start]
    mov  ecx,0x00ffffff            ; text colour
    mov  edx,window_text           ; pointer to text beginning
    mov  esi,12                    ; text length
    mcall                      ; do it

    mov  eax,8                     ; function 8: define and draw button
    mov  ebx,(381-18)*65536+13     ; [x start] *65536 + [x size]
    mov  ecx,4*65536+13            ; [y start] *65536 + [y size]
    mov  edx,1                     ; button id
    mov  esi,0x6688dd              ; button color RRGGBB
    mcall                      ; do it

    mov  eax,13                    ; function 13: draw bar
    mov  ebx,1*65536               ; [x start] *65536
    add  ebx,[x_size]              ; add [x size]
    dec  ebx                       ; x size - 1
    mov  ecx,[y_size]              ; [y start] *65536
    sub  ecx,17                    ; minus height
    shl  ecx,16                    ; *65536
    add  ecx,17                    ; add height
    mov  edx,0x006688dd            ; bar colour
    mcall                      ; do it

    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,5*65536               ; [x start] *65536
    add  ebx,[y_size]              ; add [y start]
    sub  ebx,12                    ; move up
    xor  ecx,ecx                   ; text colour
    mov  edx,button_no             ; pointer to text beginning
    mov  esi,14                    ; text length
    mcall                      ; do it

    add  ebx,95*65536              ; move xy position
    mov  edx,menu_text             ; pointer to text beginning
    mcall                      ; do it

    call write_main                ; draw menu

    mov  eax,12                    ; function 12: tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    mcall                      ; do it

    pop  eax                       ; restore register
    ret                            ; return

 ; ************* WRITE MAIN *************

write_main:

    mov  eax,13                    ; function 13: draw bar
    mov  ebx,1*65536               ; [x start] *65536
    add  ebx,[x_size]              ; +[x_size]
    dec  ebx                       ; x size - 1
    mov  ecx,21*65536+17           ; [y start] *65536 +[y size]
    mov  edx,[menu_colour]         ; menu colour
    mcall                      ; do it

    mov  [main_pos],1              ; start position first button
    xor  edi,edi                   ; data offset = 0

next_main_item:
    mov  al,[MENU_DATA+edi]        ; get byte at menu_data + offset
    cmp  al,'E'                    ; is it the END
    je   main_get_out              ; yes then exit
    cmp  al,'0'                    ; is it a main menu item
    jne  not_main_menu             ; no then jump code

main_menu:
    mov  al,[MENU_DATA+edi+1]      ; get byte at menu_data + offset + 1
    cmp  al,'0'                    ; is it a divider
    je   is_main_bar               ; yes then jump code
    mov  eax,8                     ; function 8: define button
    mov  ebx,[main_pos]            ; [x start]
    shl  ebx,16                    ; *65536
    add  bl,75                     ; +[x size]
    mov  ecx,21*65536+16           ; [y start] *65536 +[y size]
    xor  edx,edx                   ; clear register
    mov  dl,[MENU_DATA+edi+2]      ; get byte button id number
    mov  esi,[menu_colour]         ; button colour
    mcall                      ; do it
    mov  eax,4                     ; function 4: write text to window
    add  ebx,6*65536-49            ; move xy position
    xor  ecx,ecx                   ; text colour
    mov  edx,MENU_DATA+3           ; point at menu text
    add  edx,edi                   ; add our offset
    mov  esi,11                    ; number of characters
    mcall                      ; do it

is_main_bar:
    add  [main_pos],76             ; update button position

not_main_menu:
    add  edi,14                    ; update offset
    jmp  next_main_item            ; do next menu item

main_get_out:

    ret                            ; return

; *********** DRAW DATA ***********

draw_data:

    push eax                       ; save register
    mov  ebx,0x00030000            ; display 3 decimal characters
    xor  ecx,ecx                   ; clear register
    mov  cl,ah                     ; swap data
    mov  eax,47                    ; function 47: display number to window
    mov  edx,70*65536              ; [x start] *65536
    add  edx,[y_size]              ; +[y start]
    sub  edx,12                    ; move position
    xor  esi,esi                   ; text colour
    mcall                      ; do it
    pop  eax                       ; restore register

    cmp  [button_press],1          ; has a sub button been pressed
    je   draw_get_out              ; then jump code

    push eax                       ; save register
    xor  edx,edx                   ; clear register
    shr  ax,8                      ; move button id into al
    sub  eax,100                   ; subtract 100
    mov  dx,14                     ; use record length as multiplier
    mul  dx                        ; multiply
    mov  edx,eax                   ; swap registers
    add  edx,MENU_DATA             ; add offset
    inc  edx                       ; add 1
    mov  ebx,188*65536             ; [x start] *65536
    add  ebx,[y_size]              ; +[y start]
    sub  ebx,12                    ; move position
    mov  esi,1                     ; 1 character
    mov  eax,4                     ; function 4: write text to window
    xor  ecx,ecx                   ; text colour
    mcall                      ; do it
    pop  eax                       ; restore register

draw_get_out:
    ret                            ; return

; **************** CLEAR DATA ******************

clear_data:

    push eax                       ; save register
    mov  eax,13                    ; function 13: draw bar
    mov  ebx,67*65536+23           ; [x start] *65536 +[x size]
    mov  ecx,[y_size]              ; [y start]
    sub  ecx,15                    ; move position
    shl  ecx,16                    ; *65536
    add  ecx,13                    ; [y size]
    mov  edx,0x00aaaaaa            ; bar colour
    mcall                      ; do it
    mov  ebx,185*65536+11          ; move position
    mcall                      ; do it again

    pop  eax                       ; restore register
    ret                            ; return

 ; ************* WRITE SUB *************

write_sub:

    push eax                       ; save register
    mov  [but_pos],38              ; y start position offset
    mov  [sub_pos],1               ; x start position offset
    xor  edx,edx                   ; clear register
    shr  ax,8                      ; move button id into al
    sub  eax,100                   ; subtract 100
    mov  dx,76                     ; menu width + 1
    mul  dx                        ; multiply
    add  [sub_pos],eax             ; add menu position to offset
    pop  eax                       ; restore register

    xor  edx,edx                   ; clear register
    shr  ax,8                      ; move button id into al
    sub  eax,100                   ; subtract 100
    mov  dx,14                     ; use record length as multiplier
    mul  dx                        ; multiply
    add  eax,MENU_DATA             ; add offset
    inc  eax                       ; plus 1
    mov  al,[eax]                  ; get menu number byte
    mov  [menu_number],al          ; save it

    xor  edi,edi                   ; clear offset

next_sub_item:
    mov  al,[MENU_DATA+edi]        ; get byte at menu_data + offset
    cmp  al,'E'                    ; is it the END
    je   sub_get_out               ; yes then exit
    cmp  al,[menu_number]          ; is it sub menu item
    jne  not_sub_menu              ; no then jump code

sub_menu:
    mov  al,[MENU_DATA+edi+1]      ; get byte at menu_data + offset + 1
    cmp  al,'0'                    ; is it a divider
    jne  is_sub_button             ; no then jump code
    mov  eax,13                    ; function 13: draw bar
    mov  edx,[menu_colour]         ; bar colour
    mov  ebx,[sub_pos]             ; [x start]
    shl  ebx,16                    ; *65536
    add  ebx,76                    ; [x size]
    mov  ecx,[but_pos]             ; [y start]
    shl  ecx,16                    ; *65536
    add  ecx,17                    ; [y size]
    mcall                      ; do it
    jmp  is_sub_bar                ; jump button code

is_sub_button:
    mov  eax,8                     ; function 8: define and draw button
    xor  edx,edx                   ; clear register
    mov  dl,[MENU_DATA+edi+2]      ; get byte button id number
    mov  ebx,[sub_pos]             ; [x start]
    shl  ebx,16                    ; *65536
    add  ebx,75                    ; [x size]
    mov  ecx,[but_pos]             ; [y start]
    shl  ecx,16                    ; *65536
    add  ecx,16                    ; [y size]
    mov  esi,[menu_colour]         ; button colour
    mcall                      ; do it

    mov  ebx,[sub_pos]             ; [x start]
    shl  ebx,16                    ; *65536
    add  ebx,6*65536               ; move position
    add  ebx,[but_pos]             ; [y start]
    add  bl,5                      ; move position
    xor  ecx,ecx                   ; clear register
    mov  edx,MENU_DATA+3           ; point to button text
    add  edx,edi                   ; add offset
    mov  esi,11                    ; number of characters
    mov  eax,4                     ; function 4: write text to window
    mcall                      ; do it
is_sub_bar:
    add  [but_pos],17              ; move y position

not_sub_menu:
    add  edi,14                    ; move offset
    jmp  next_sub_item             ; do next button

sub_get_out:

    ret                            ; return

; ***************** DATA AREA ******************

x_size:       dd 381               ; window x size
y_size:       dd 200               ; window y size

window_text   db 'MENU EXAMPLE'    ; grab bar text
button_no     db 'BUTTON No:    '  ; status bar text
menu_text     db 'MENU SELECTED:'  ; status bar text

button_press  dd 0

menu_colour   dd 0x00aaaaaa        ; menu & button colour

menu_number   db '0'               ; menu selection

sub_pos       dd 1                 ; sub menu x position
but_pos       dd 38                ; sub menu y position

main_pos      dd 1                 ; main menu x position

MENU_DATA:    db '01'              ; MAIN MENU = 0 - 1 = menu
              db 100               ; button id
              db 'FILE       '     ; button text
              db '02'              ; MAIN MENU = 0 - 2 = menu
              db 101               ; button id
              db 'EDIT       '     ; button text
              db '04'              ; MAIN MENU = 0 - 3 = menu
              db 102               ; button id
              db 'TEST       '     ; button text
              db '00'              ; MAIN MENU = 0 - 0 = divider
              db 103               ; SPACER ID
              db '           '     ; SPACER TEXT padding
              db '03'              ; MAIN MENU = 0 - 4 = menu
              db 104               ; button id
              db 'HELP       '     ; button text

              db '11'              ; menu level = 1 - 1 = button
              db 110               ; button id
              db 'LOAD       '     ; button text
              db '11'              ; menu level = 1 - 1 = button
              db 111               ; button id
              db 'SAVE       '     ; button text
              db '10'              ; menu level = 1 - 0 = divider
              db 112               ; SPACER ID
              db '           '     ; SPACER TEXT padding
              db '11'              ; menu level = 1 - 1 = button
              db 113               ; button id
              db 'QUIT       '     ; button text

              db '21'              ; menu level = 2 - 1 = button
              db 120               ; button id
              db 'COPY       '     ; button text
              db '21'              ; menu level = 2 - 1 = button
              db 121               ; button id
              db 'PASTE      '     ; button text

              db '31'              ; menu level = 3 - 1 = button
              db 130               ; button id
              db 'SETUP      '     ; button text
              db '31'              ; menu level = 3 - 1 = button
              db 131               ; button id
              db 'ABOUT..    '     ; button text

              db '41'              ; menu level = 3 - 1 = button
              db 140               ; button id
              db 'TEST 1     '     ; button text
              db '41'              ; menu level = 3 - 1 = button
              db 141               ; button id
              db 'TEST 2     '     ; button text
              db '41'              ; menu level = 3 - 1 = button
              db 142               ; button id
              db 'TEST 3     '     ; button text
              db '41'              ; menu level = 3 - 1 = button
              db 143               ; button id
              db 'TEST 4     '     ; button text
              db '41'              ; menu level = 3 - 1 = button
              db 144               ; button id
              db 'TEST 5     '     ; button text
              db '41'              ; menu level = 3 - 1 = button
              db 145               ; button id
              db 'TEST 6     '     ; button text

              db 'END'             ; IMPORTANT need an END
I_END:
