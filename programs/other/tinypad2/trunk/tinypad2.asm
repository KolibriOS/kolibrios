; SYSTEM HEADER:
   use32
   org 0x0
   db  'MENUET01'  ; identifier
   dd  0x01        ; version
   dd  START       ; pointer to start
   dd  TINYPAD_END ; size of file
   dd  0x300f00 ;0x500000; 0x3fff00;0x300000    ; size of memory
   dd  0xeff0 ;0x4fffff ;0x3ff000;0xeff0      ; esp
   dd  I_PARAM     ; parameters
   dd  0           ; reserved
include 'ascl.inc'
include 'lang.inc'
include 'macros.inc' ; useful stuff
include 'dialogs1.inc'
include 'scroll.inc'
include 'debug.inc'
purge mov            ;  SPEED
;******************************************************************************
; INITIALIZING
START:
;debug_print_hex TINYPAD_END
call mask_events

    cmp    [I_PARAM],byte 0
    jz     noparams

;//Willow
    cmp    byte[I_PARAM],'*'
    jne    .noipc
; convert size from decimal representation to dword
        mov     esi, I_PARAM+1
        xor     edx, edx
        xor     eax, eax
@@:
        lodsb
        test    al, al
        jz      @f
        lea     edx, [edx*4+edx]
        lea     edx, [edx*2+eax]
        jmp     @b
@@:
    add    edx,20
    mcall  60,1,0x10000-16
    mov    dword[0x10000+-16+4],8
    mcall  40,1000000b
    mcall  23,200
;    dpd    eax
    cmp    eax,7
    jne    exit_now
    mov    esi,0x10000-16
    mov    byte[esi],1
    call   mask_events
    mov    eax,[esi+12]
    inc    eax
    call   file_found
    jmp    do_load_file.restorecursor
  .noipc:
;//Willow
    ; parameters are at I_PARAM
    mov    esi,I_PARAM
    mov    edi,filename
    mov    ecx,50
    cld
    rep    movsb

    mov    edi,filename
    mov    ecx,50
    xor    eax,eax
    repne  scasb
    sub    edi,filename
    dec    edi
    mov    [filename_len],edi
    jmp    do_load_file
  noparams:
    jmp newfile
;******************************************************************************
; MAIN LOOP
still:
    call writepos ; write current position & number of strings
    call mouse_info

    mov  eax,10   ; wait here until event
    int  0x40

    cmp eax,1
    je red
    cmp eax,2
    je key
    cmp eax,3
    je button
    cmp eax,6
    je mouse
    jmp  still
;******************************************************************************
; *********************************
; *          MOUSE                *
; *********************************
mouse:
mov eax,37    ;mouse click
mov ebx,2
int 0x40
cmp eax,0
je .leave_now
;if exit window is on
cmp [exit_wnd_on],1
jne @f
mov [exit_wnd_on],0
jmp red
;else
@@:
cmp eax,1     ;mouse 1
jne .leave_now

mov eax,37    ;mouse position
mov ebx,1
int 0x40

mov word[mouse_y],ax
shr eax,16
mov word[mouse_x],ax


cmp [mouse_x],7
jb .leave_now ;.leave_now       ;if <
cmp [mouse_x],485;487
ja .leave_now ;.leave_now       ;if >

cmp [mouse_y],45
jb .leave_now ;.leave_now ;if <
cmp [mouse_y],342 ;345
ja .leave_now ;.leave_now       ;if >

call main_cursor_move

.leave_now:
jmp still

; *********************************
; *         BUTTON HANDLER        *
; *********************************

  button:

    mov  eax,17
    int  0x40
;;;;;;;;;;;;;;;exit dialog box check;;;;;;;;;;;;;
cmp ah,49
je save_and_exit
cmp ah,48
je exit_now
cmp ah,47
mov [exit_wnd_on],0
je red
cmp ah,46
jne @f

  call save_file
  jmp newfile
@@:
cmp ah,45
je newfile
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;MENU CHECK;;;;;;;;;;;;;;;;;;;;;;;
cmp ah,15
jne no_d_w
call draw_window_for_dialogs
jmp still
no_d_w:
cmp ah,97
je draw_win_menu_file
cmp ah,98
je draw_win_menu_code
cmp ah,96
je draw_win_menu_text
cmp ah,95
je goto_string
cmp ah,92
je search_window
cmp ah,94
je red
cmp ah,99
  je help_wnd
cmp ah,100
  je new_pad_wnd
cmp ah,101
  je doyouwannasave
cmp ah,102
  jne nosavenow
   for_key_save:
   savedialog draw_window_for_dialogs,copy_fpath_s,saveerror,mypath
nosavenow:
cmp ah,103
  jne noopennow
   for_key_open:
   opendialog draw_window_for_dialogs,copy_fpath,openerror,mypath
noopennow:
cmp ah,104
je exit
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    shr  eax,8

    cmp  al,50
    jne  no_search

; SEARCH {
  search:

    cmp [lines],1  ;something like bug fix
    je red ;still

    xor  esi,esi
    mov  edi,[post]
    add  edi,80
    imul ecx,[lines],80
    sub  ecx,edi         ; ecx (loop counter) = lines*80-(post+80)
  news:
    push edi
  news2:

     ; 1) LOAD BYTES
     mov  al,[esi+search_string]
     mov  bl,[edi+0x80000]

     ; 2) COMPARE BYTES
     cmp  al,bl
     je   yes_char

     ; 3) LETTER?
     cmp  al,'A'
     jb   notequal
     cmp  al,'z'
     ja   notequal

     ; 4) A->a OR a->A
     cmp  al,'a'
     jb   @f
     add  al,-32
     jmp  compare_bytes
    @@:
     cmp  al,'Z'
     ja   notequal
     add  al,32

    compare_bytes:
     cmp  al,bl
     jne  notequal

  yes_char:
    inc  edi
    inc  esi
    cmp  esi,[search_len]
    jge  sfound
    jmp  news2

  notequal:
    pop  edi
    xor  esi,esi
    inc  edi
    loop news
    call string_not_found
    jmp  still

  sfound:
    mov  eax,edi
    cdq                   ; edx = 0;
    mov  ebx,80
    div  ebx
;;;;;   imul eax,80
;    lea  eax,[eax+eax*4]  ; eax *= 5;
;    shl  eax,4           ; eax *= 16;

;    mov  [post],eax
    mov  [posy],0
;---------------
;    mov ebx,80
;    div ebx
    call goto_pos
    call draw_vertical_scroll
    jmp  still
; SEARCH }

  no_search:

; TOOLBAR {
   cmp     eax,10000
   jb      no_toolbar

   add     eax,-10000
   jne     @f
lbl_compile_file:
   mov     [run_outfile],0
   call    start_fasm
   jmp     still
 @@:
   dec     eax
   jnz     @f
lbl_run_outfile:
   mov     [run_outfile],1
   call    start_fasm
   jmp     still
 @@:
   dec     eax
   jnz     @f
   call    open_debug_board
   jmp     still
 @@:
   dec     eax
   jnz     still
   call    open_sysfuncs_txt
   jmp     still
; TOOLBAR }

  no_toolbar:

    cmp  al,4
    jne  noid4

; LOAD_FILE {
  do_load_file:
    mov [modified],0

  call empty_work_space


  cmp  [filename],'/'
    jne  @f

    call loadhdfile
    jmp  .restorecursor
  @@:
   call loadfile1
  .restorecursor:
    mov  edi,0x78000
    mov  ecx,80*80/4
    mov  eax,0x01010101
    cld
    rep  stosd
    xor  eax,eax
    mov  [post],eax
    mov  [posx],eax
    mov  [posy],eax

; enable color syntax for ASM and INC files:
    mov  [asm_mode], 0

    mov  eax, [filename_len]
    add  eax, filename
    cmp  word [eax-3],'AS'
    jne  @f
    cmp  byte [eax-1],'M'
    jne  @f
    mov  [asm_mode], 1
    jmp  .nocol
  @@:
    cmp  word [eax-3],'IN'
    jne  @f
    cmp  byte [eax-1],'C'
    jne  @f
    mov  [asm_mode], 1
  @@:
  .nocol:

; if the header is the same as the previous,
; just redraw the text area
; else redraw the window

;------pos00=filename_len
    mov eax,[filename_len]
    mov [pos00],eax
;-------------------------
    mov  ecx, [filename_len]
    add  ecx, 10             ; strlen(" - TINYPAD");
    cmp  ecx, [headlen]
    jne  @f
    add  ecx, -10
    mov  esi, filename       ; strcmp(filename,header);
    mov  edi, header
    rep  cmpsb
    jne  @f
    call drawfile
    call draw_window_for_dialogs ;redraw, because it's needed
    cmp [to_return],0
    je still
    ret
    ;jmp  still
  @@:
call set_title
cmp [to_return],0
je still
ret
;    jmp  still
; LOAD_FILE }

  noid4:

    cmp  al, 2
    jz   yessave

    dec  al       ; close if butid == 0
    jnz  nosave
; EXIT:
exit:
cmp [modified],0
je exit_now

      mov  eax,55           ; beep
      mov  ebx,eax
      mov  esi,save_beep1
      int  0x40
      delay 4
      mov  eax,55           ; beep
      mov  ebx,eax
      mov  esi,save_beep2
      int  0x40

 ;---------"EXIT" dialog box

 mov eax,13
 mov ebx,150*65536+200
 mov ecx,100*65536+70
 mov edx,[sc.work_graph] ;0x00dd9438 ;0x00ff7512
 int 0x40


mov eax,38
mov ebx,150*65536+350
mov ecx,100*65536+100
mov edx,cl_White
int 0x40
mov eax,38
mov ebx,150*65536+350
mov ecx,170*65536+170
mov edx,cl_White
int 0x40
mov eax,38
mov ebx,150*65536+150
mov ecx,100*65536+170
mov edx,cl_White
int 0x40
mov eax,38
mov ebx,350*65536+350
mov ecx,100*65536+170
mov edx,cl_White
int 0x40


if lang eq ru
 putlabel 190,120,'Сохранить документ?',cl_White

 drawlbut 170,140,30,15,'Да',49,[sc.work_button],cl_White
 drawlbut 230,140,30,15,'Нет',48,[sc.work_button],cl_White
 drawlbut 290,140,45,15,'Отмена',47,[sc.work_button],cl_White
else
 putlabel 190,120,'Save the document?',cl_White

 drawlbut 170,140,30,15,'Yes',49,[sc.work_button],cl_White
 drawlbut 230,140,30,15,'No',48,[sc.work_button],cl_White
 drawlbut 290,140,45,15,'Cancel',47,[sc.work_button],cl_White
end if

 mov [exit_wnd_on],1
;----------------

jmp still

    exit_now:
    mov  [MainWndClosed], 1
    or   eax, -1
    int  0x40

    save_and_exit:
    call save_file
    jmp exit_now
;    mov  [MainWndClosed], 1
;    or   eax, -1
;    int  0x40

; SAVE_FILE {
  yessave:
  call clear_screen
  call drawwindow
    call save_file
;  call clear_screen
;  call drawwindow
    mov [modified],0

    jmp  still
; SAVE_FILE }

  nosave:

    inc  al
    call read_string

    jmp  still

;**********************************
;*         REDRAW HANDLER         *
;**********************************

red:
; перерисовка окна
   call   clear_screen
   call   drawwindow
   jmp    still



;**********************************
;*          KEY HANDLER           *
;**********************************

  key:
    mov  eax, 2   ; GET KEY
    int  0x40

  cmp [exit_wnd_on],1 ;exit window is ON?
  jne no_exit_wnd     ; no - goto other checks

;  cmp eax,13          ;enter - save n exit
;  je save_and_exit

;  cmp eax,173         ;ctrl+enter - exit
;  je exit_now
  mov [exit_wnd_on],0 ;other keys - cancel
;  call draw_window_for_dialogs

  jmp red ;redraw and still


  no_exit_wnd:
    shr  eax, 8

; HELP_TEXT {
    cmp  al, 210  ; Ctrl + F1
    jne  no_help_text
help_wnd:
call clear_screen
call drawwindow
    mov  eax, 51
    mov  ebx, 1
    mov  ecx, help_thread_start
    mov  edx, 0xfff0
    int  0x40
    jmp  still

help_thread_start:
    call draw_help_wnd

  still_hw:
    cmp  [MainWndClosed], 1
    je   exit_hw
    mov  eax, 10
    int  0x40
    cmp  eax, 1
    je   red_hw
    cmp  eax, 2
    je   key_hw
    cmp  eax, 3
    je   button_hw
    jmp  still_hw

  red_hw:
    call draw_help_wnd
    jmp  still_hw

  key_hw:
;   mov  eax, 2
    int  0x40
    cmp  ah, 27
    jne  still_hw

  button_hw:
    mov  eax, 17
    int  0x40
  exit_hw:
    or   eax, -1
    int  0x40

; draw help window
 draw_help_wnd:
    mov  eax, 12
    mov  ebx, 1
    int  0x40

    xor  eax, eax
    mov  ebx, 200 shl 16 + 320
    mov  ecx, 200 shl 16 + 260
    mov  edx, 0x03e0e0e0   ; work
    mov  esi, [sc.grab]
    or   esi, 0x80000000   ; grab
    mov  edi, [sc.frame]   ; frame
    int  0x40

    mov  eax, 4
    mov  ebx, 8 shl 16 + 8
    mov  ecx, 0xffffff
    mov  edx, help_wnd_header
    mov  esi, hlphead_len
    int  0x40

    mov  eax, 4
    mov  ebx, 8 shl 16 + 34
    xor  ecx, ecx
    mov  esi, 51
    mov  edx, help_text
  .new_help_line:
    int  0x40
    add  ebx, 10
    add  edx, esi
    cmp  [edx], byte 'x'
    jne  .new_help_line

    mov  eax, 12
    mov  ebx, 2
    int  0x40
    ret
; HELP_TEXT }

  no_help_text:
; LOAD_FILE {
    cmp  al, 211       ; Ctrl + F2
    je   for_key_open ;do_load_file
; LOAD_FILE }

; SEARCH {
    cmp  al, 212       ; Ctrl + F3
    je   search
; SEARCH }

; SAVE_FILE {
    cmp  al, 213       ; Ctrl + F4
    je   yessave
; SAVE_FILE }

; ENTER_FILENAME {
    cmp  al, 214       ; Ctrl + F5
    jne  @f
jmp for_key_save
  @@:
; ENTER_FILENAME }

; ENTER_SEARCH {
    cmp  al, 215       ; Ctrl + F6
    jne  @f
;    mov  al, 51
;    call read_string
;    jmp  still
    jmp search_window
  @@:
; ENTER_SEARCH }

; CHANGE_LAYOUT {
    cmp  al, 217       ; Ctrl + F8
    jne  @f
    call layout
    jmp  still
  @@:
; CHANGE_LAYOUT }

; COMPILE_FILE {
    cmp al, 208
    je lbl_compile_file
; COMPILE_FILE }

; RUN_OUTFILE {
   cmp al, 209
   je lbl_run_outfile

; RUN_OUTFILE }

;run debug board {      -----
   cmp al,255
   jne @f
   call open_debug_board
   call activate_me
   jmp still
;}
   @@:
;open sysfuncR {        -----
;   cmp al,228
;   jne @f
;   call open_sysfuncs_txt
;   jmp still
;   @@:

;fast_save_and_open {
   cmp al,216
   jne @f
   call make_fast_so
   jmp still
;}
   @@:
; 3 times english -> русский
; 2 times русский -> english

; COPY START {
    cmp  al, 19
    jne  no_copy_start
    mov  eax, [post]
    imul ebx, [posy], 80
    add  eax, ebx
    mov  [copy_start], eax
    jmp  still
; COPY START }

  no_copy_start:
; COPY END {
    cmp  al, 5
    jne  no_copy_end
    cmp  [copy_start], 0
    je   still
    mov  ecx, [post]
    imul ebx, [posy], 80
    add  ecx, ebx
    add  ecx, 80
    cmp  ecx, [copy_count]
    jb   still
    sub  ecx, [copy_start]
    mov  [copy_count], ecx
    mov  esi, [copy_start]
    add  esi, 0x80000
    mov  edi, 0x2f0000
    cld
    rep  movsb
    jmp  still
; COPY END }

  no_copy_end:

; PASTE {
    cmp  al, 16
    jne  no_copy_paste
    cmp  [copy_count], 0
    je   still
    mov  eax,[copy_count]
    cdq
    mov  ebx, 80
    div  ebx
    add  [lines], eax
    mov  ecx, 0x2e0000
    mov  eax, [post]
    imul ebx, [posy], 80
    add  eax, ebx
    add  eax, 0x80000
    sub  ecx, eax
    mov  esi, 0x2e0000
    sub  esi, [copy_count]
    mov  edi, 0x2e0000
    std
    rep  movsb
    mov  esi, 0x2f0000
    mov  edi, [post]
    imul eax, [posy], 80
    add  edi, eax
    add  edi, 0x80000
    mov  ecx, [copy_count]
    cld
    rep  movsb

    call clear_screen
    call drawfile
call draw_vertical_scroll
    mov [modified],1

    jmp  still
; PASTE }


  no_copy_paste:


; INSERT_SEPARATOR {
    cmp  al,0xc       ; Ctrl+L
    jne  no_insert_separator

    imul eax,[posy],80
    add  eax,[post]
    add  eax,0x80000
    mov  ebx,eax

    imul eax,[lines],80
    add  eax,0x80000     ; теперь указывает на конец файла

    mov  ecx,eax ; size
    sub  ecx,ebx
    inc  ecx

    mov  esi,eax ; from
    mov  edi,eax
    add  edi,80  ; to

    std
    rep  movsb

    mov  ecx,80/4
    mov  esi,comment_string
    mov  edi,ebx
    cld
    rep  movsd

    inc  [lines]

    call clear_screen
    call drawfile
;call calc_scroll_size_and_pos
call draw_vertical_scroll
    mov [modified],1

    jmp  still
; INSERT_SEPARATOR }


  no_insert_separator:


; DEL_LINE {
    cmp  al,4
    jne  no_delete_line
    mov  eax,[post]
    cdq
    mov  ebx,80
    div  ebx
    add  eax,[posy]
    inc  eax
    cmp  eax,[lines]
    jge  still
    dec  dword [lines]
    imul edi,[posy],80
    add  edi,[post]
    add  edi,0x80000
    mov  esi,edi
    add  esi,80
    mov  ecx,0x2e0000
    sub  ecx,esi
    shr  ecx,4
    cld
    rep  movsd
    call clear_screen
    call drawfile
;call calc_scroll_size_and_pos
call draw_vertical_scroll
    mov [modified],1

    jmp  still
; DEL_LINE }

  no_delete_line:

; ENTER {
    cmp  al,13
    jnz  noenter

    ; lines down
    mov  eax,[posy]
    inc  eax
;   imul eax,80
    lea  eax,[eax+eax*4]  ; eax *= 5
    shl  eax,4            ; eax *= 16
    add  eax,0x80000
    add  eax,[post]
    mov  ebx,eax

    ; ebx = ([posy]+1)*80 + 0x80000 + [post]
    ; ebx -> first byte of next string

    imul eax,[lines],80
    add  eax,0x80000
    mov  ecx,eax

    ; ecx = [lines]*80 + 0x80000
    ; ecx -> end of the document

    cmp  ebx,ecx
    jz   .bug_fixed

   @@:
    dec  ecx
    mov  dl,[ecx]
    mov  [ecx+80],dl

    cmp  ecx,ebx
    jnz  @b

   .bug_fixed:

    ; save for later
    imul eax,[posy],80
    add  eax,0x80000
    add  eax,[post]
    mov  ebx,eax
    add  eax,[posx]
    ; eax = 0x80000 + [post] + [posy]*80 + [posx]

    push eax

    dec  ebx
    xor  ecx,ecx
  @@:
    cmp  ecx,80
    je   @f
    inc  ecx
    inc  ebx
    cmp  byte [ebx],' '
    je   @b
  @@:
    dec  ecx

    cmp  ecx,80-1
    jne  @f
;   mov  [posx],0
    jmp  .lbl
  @@:

    cmp  [posx],ecx
    jbe  @f
    mov  [posx],ecx
    jmp  .lbl
  @@:
    mov  [posx],0

  .lbl:
    inc  [posy]

    ;clear line
    imul eax,[posy],80
    add  eax,0x80000
    add  eax,[post]

    mov  edi,eax
    mov  eax,'    '
    mov  ecx,80/4
    cld
    rep  stosd


    ; end of line to next line beginning
    imul eax,[posy],80
    add  eax,0x80000
    add  eax,[post]
;   add  eax,[posx]
    mov  ebx,eax
    ; ebx -> beginning of this line

    pop  esi
    mov  edi,eax

   @@:
    mov  al,[esi]
    mov  [ebx],al
    mov  [esi],byte ' '

    inc  esi
    inc  ebx

    cmp  esi,edi
    jb   @b

    inc  [lines]

    mov  ecx,[posy]
    cmp  ecx,[slines]
    jne  @f

    dec  [posy]
    add  [post],80

   @@:
    call clear_screen
    call drawfile
;call calc_scroll_size_and_pos

    call draw_vertical_scroll
    mov [modified],1

    jmp  still
; ENTER }


  noenter:


; UP {
    cmp  al,130+48
    jnz  noup
    mov  ecx,[posy]
    test ecx,ecx
    jnz  .up1
    mov  ecx,[post]
    test ecx,ecx
    jz   still
    add  ecx,-80
    mov  [post],ecx
    call clear_screen
    jmp  .finish
  .up1:
    dec  ecx
    mov  [posy],ecx
  .finish:
    call drawfile
    call draw_vertical_scroll
;call calc_scroll_size_and_pos

    jmp  still
; UP }

  noup:

; DOWN {
    cmp  al,129+48
    jnz  nodown

    mov  ecx,[posy]
    mov  eax,[slines]
    dec  eax
    cmp  ecx,eax
    jb   .do1         ; goto do1 if [posy] < [slines]-1

    mov  eax,[lines]
    sub  eax,[slines]
    dec  eax
    jb   still        ; goto still if [lines] < [slines]-1
;   imul eax,80
    lea  eax,[eax+eax*4]
    shl  eax,4
    cmp  [post],eax
    jg   still        ; goto still if [post] > ([lines]-[slines]-1)*80

    add  [post],80
    call clear_screen
    call drawfile
    call draw_vertical_scroll
;call calc_scroll_size_and_pos

    jmp  still

  .do1:
    pusha
    mov  eax,[post]
    cdq
    mov  ebx,80
    div  ebx
    add  eax,[posy]
    inc  eax
    cmp  eax,[lines]
    jb   .do10
    popa
    jmp  still

  .do10:
    popa
    inc  ecx
    mov  [posy],ecx
    call drawfile
    call draw_vertical_scroll
;call calc_scroll_size_and_pos

    jmp  still
; DOWN }


  nodown:


; LEFT {
    cmp  al,128+48
    jnz  noleft
    cmp  [posx],0
    je   still
    dec  [posx]
    call drawfile
    jmp  still
; LEFT }


  noleft:


; RIGHT {
    cmp  al,131+48
    jnz  noright
    cmp  [posx],79
    je   still
    inc  [posx]
    call drawfile
    jmp  still
; RIGHT }


  noright:


; PAGE_UP {
  page_up:
    cmp  al,136+48
    jnz  nopu
scrl_up:
    mov  eax,[slines]
    dec  eax
;   imul eax,80
    lea  eax,[eax+eax*4]
    shl  eax,4
    mov  ecx,[post]
    cmp  eax,ecx
    jbe  pu1
    mov  ecx,eax
   pu1:
    sub  ecx,eax
    mov  [post],ecx

    call clear_screen
    call drawfile
    call draw_vertical_scroll
;call calc_scroll_size_and_pos
    jmp  still
; PAGE_UP }


  nopu:


; PAGE_DOWN {
  page_down:
    cmp  al,135+48
    jnz  nopd
scrl_down:
    mov  eax,[lines]
    cmp  eax,[slines]
    jb   still

    mov  eax,[post]   ; eax = offset
    cdq
    mov  ebx,80
    div  ebx          ; eax /= 80
    mov  ecx,[lines]  ; ecx = lines in the file
    cmp  eax,ecx      ; if eax < ecx goto pfok
    jnb  still
    mov  eax,[slines] ; eax = lines on the screen
    dec  eax          ; eax--
;   imul eax,80       ; eax *= 80
    lea  eax,[eax+eax*4]
    shl  eax,4
    add  [post],eax   ; offset += eax

    mov  eax,[lines]  ; eax =  lines in the file
    sub  eax,[slines] ; eax -= lines on the screen
;   imul eax,80       ; eax *= 80
    lea  eax,[eax+eax*4]
    shl  eax,4
    cmp  [post],eax
    jb   @f
    mov  [post],eax
  @@:

    call clear_screen
    call drawfile
    call draw_vertical_scroll
;call calc_scroll_size_and_pos

    jmp  still
; PAGE_DOWN }

  nopd:

; HOME {
    cmp  al,132+48
    jnz  nohome

    push 0
    pop  [posx]

    call drawfile
    jmp  still
; HOME }


  nohome:


; END {
  end_key:
    cmp    al,133+48
    jnz    noend

    imul   eax,[posy],80
    add    eax,0x80000
    add    eax,[post]

    mov    esi,eax
    add    eax,80+1

@@: dec    eax
    cmp    eax,esi
    je     @f
    cmp    byte [eax-1],' '
    jbe    @b
@@:
    sub    eax,esi
    cmp    eax,80-1
    jbe    @f
    dec    eax
@@:
    mov    [posx],eax

    call drawfile
    jmp  still
; END }


  noend:


; GO_START {
    cmp  al,251         ; Ctrl + [
    jnz  no_go_to_start

    push 0
    pop  [post]         ; offset = 0

    call clear_screen
    call drawfile       ; draw file
    call draw_vertical_scroll
;call calc_scroll_size_and_pos
    jmp  still          ; go to still
; GO_START }


  no_go_to_start:


; GO_END {
    cmp  al,253         ; Ctrl + ]
    jnz  no_go_to_end
    cmp [lines],30   ;to fix ctrl+] bug
    jb @f
    mov  eax,[lines]    ; eax = lines in the file
    sub  eax,[slines]   ; eax -= lines on the screen
;   imul eax,80         ; eax *= 80 (length of line)
    lea  eax,[eax+eax*4]
    shl  eax,4
    mov  [post],eax     ; offset in the file

    call clear_screen
    call drawfile       ; draw file
    call draw_vertical_scroll
    @@:
    jmp  still          ; go to still
; GO_END }


  no_go_to_end:


; DELETE {
    cmp  al,134+48
    jne  nodel

    imul eax,[posy],80
    add  eax,0x80000
    add  eax,[post]
    add  eax,[posx]
    mov  ecx,eax

    imul eax,[posy],80
    add  eax,0x80000+79
    add  eax,[post]
    mov  ebx,eax

    push ebx

    dec  ecx
    dec  ebx


    push ecx ebx

    push ebx

    imul eax,[posy],80
    add  eax,0x80000
    add  eax,[post]
    mov  ecx,eax

    xor  eax,eax
    cdq

    pop  ebx

    dec  ecx
   @@:
    inc  ecx
    mov  dh,[ecx]
    cmp  dh,33
    jb   .nok
    xor  eax,eax
    inc  eax
   .nok:
    cmp  ecx,ebx
    jb   @b

    pop  ebx ecx

   @@:
    inc  ecx
    mov  dl,[ecx+1]
    mov  [ecx],dl
    cmp  ecx,ebx
    jb   @b


    pop  ebx
    mov  [ebx],byte 32

    test eax,eax
    jz   dellinesup

    call clear_screen
    call drawfile
    mov [modified],1

    jmp  still

  dellinesup:

    ; lines -1

    pusha

    mov  eax,[post]
    cdq
    mov  ebx,80
    div  ebx
    add  eax,[posy]
    inc  eax

    cmp  eax,[lines]
    jb   @f

    popa
    mov [modified],1

    jmp  still

  @@:

    popa

    dec  [lines]

    ; lines up

    mov  [posx],dword 0

    imul eax,[posy],80
    add  eax,0x80000-1
    add  eax,[post]
    mov  ebx,eax

    push ebx

    imul eax,[lines],80
    add  eax,0x80000-1
    add  eax,[post]
    mov  ecx,eax

    pop  ebx

   @@:
    mov  dl,[ebx+80]
    mov  [ebx],dl
    inc  ebx

    cmp  ecx,ebx
    jnz  @b

    call clear_screen
    call drawfile
    mov [modified],1

    jmp  still
; DELETE }


  nodel:


; INSERT {
    cmp  al,137+48
    jnz  noins

    imul eax,[posy],80
    add  eax,0x80000
    add  eax,[post]
    add  eax,[posx]
    mov  ecx,eax
    ; ecx = [posy]*80+0x80000+[post]+[posx]

    imul eax,[posy],80
    add  eax,0x80000+79
    add  eax,[post]
    mov  ebx,eax
    ; ebx = [posy]*80+0x80000+79+[post]

   .movstr:
    dec  ebx
    mov  dl,[ebx]
    mov  [ebx+1],dl
    cmp  ecx,ebx
    jb   .movstr

    mov  [ecx],byte ' '

    call invalidate_string
    call drawfile
    mov [modified],1

    jmp  still
; INSERT }


  noins:


; BACKSPACE {
    cmp  al,8
    jnz  nobs
    mov  ecx,[posx]
    test ecx,ecx
    jz   still
    dec  ecx
    mov  [posx],ecx

    imul eax,[posy],80
    add  eax,0x80000
    add  eax,[post]
    add  eax,[posx]
    mov  ebx,eax

    push ebx

    imul eax,[posy],80
    add  eax,0x80000+79
    add  eax,[post]
    mov  ebx,eax

    pop  ecx

    push ebx

    dec  ecx
   .movleft:
    inc  ecx
    mov  dl,[ecx+1]
    mov  [ecx],dl
    cmp  ecx,ebx
    jb   .movleft

    pop  ebx
    mov  [ebx],byte ' '

    call invalidate_string
    call drawfile
    mov [modified],1

    jmp  still
; BACKSPACE }


  nobs:


; TAB {
    cmp  eax,9  ; Tab
    jne  notab

    mov  eax,[posx]
    cmp  eax,80-1
    jae  still
    add  eax,5          ; 9         5        3
    and  eax,11111100b  ; ...1000b, ...100b, ...10b
    dec  eax
    mov  [posx], eax

    call drawfile
    mov [modified],1

    jmp  still
; TAB }


  notab:


; ADD_KEY {
    push eax  ; add key

    imul eax,[posy],80
    add  eax,0x80000
    add  eax,[post]
    add  eax,[posx]
    mov  ecx,eax

    push ecx

    imul eax,[posy],80
    add  eax,0x80000+79
    add  eax,[post]
    mov  ebx,eax

   .movright:
    dec  ebx
    mov  al,[ebx]
    mov  [ebx+1],al
    cmp  ecx,ebx
    jbe  .movright

    pop  ebx

    pop  eax

    mov  [ebx],al
    mov  edx,78
    mov  ecx,[posx]
    cmp  edx,ecx
    jb   noxp
    inc  ecx
    mov  [posx],ecx
  noxp:

    call invalidate_string
    call drawfile
    mov [modified],1
;    call draw_vertical_scroll
    jmp  still
; ADD_KEY }


;******************************************************************************

start_fasm:
   cmp     [asm_mode],1
   je      @f
   ret
  @@:
   mov     esi,filename
   mov     edi,fasm_parameters

   cmp     byte [esi],'/'
   je      .yes_systree

   mov     ecx,[filename_len]
   rep     movsb

   mov     al,','
   stosb

   mov     ecx,[filename_len]
   add     ecx,-4
   mov     esi,filename
   rep     movsb

   mov     al,','
   stosb

   mov     [edi],dword '/RD/'
   add     edi,4
   mov     [edi],word '1/'
   inc     edi
   inc     edi

   mov     al,0
   stosb

   jmp     .run

 .yes_systree:
   add     esi,[filename_len]
   dec     esi

   xor     ecx,ecx
   mov     al,'/'
 @@:
   cmp     [esi],al
   je      @f
   dec     esi
   inc     ecx
   jmp     @b
 @@:
   inc     esi

   push    esi
   push    esi
   push    ecx

   rep     movsb

   mov     al,','
   stosb

   pop     ecx
   pop     esi

   add     ecx,-4
   rep     movsb

   mov     al,','
   stosb

   pop     ecx
   sub     ecx,filename
   mov     esi,filename

   rep     movsb

   mov     al,0
   stosb

 .run:
   cmp     [run_outfile],1
   jne     @f
   dec     edi
   mov     eax,',run'
   stosd
   mov     al,0
   stosb
  @@:

   mov     eax,19
   mov     ebx,fasm_filename
   mov     ecx,fasm_parameters
   int     0x40
ret

open_debug_board:
   mov     eax,19
   mov     ebx,debug_filename
   xor     ecx,ecx
   int     0x40
ret

open_sysfuncs_txt:
   mov     eax,19
   mov     ebx,tinypad_filename
   mov     ecx,sysfuncs_filename
   int     0x40
ret


empty_work_space:
; очистить все
   mov     eax,'    '
   mov     edi,0x80000
   mov     ecx,(0x300000-0x90000)/4
   cld
   rep     stosd
   mov     edi,0x10000
   mov     ecx,0x60000/4
   rep     stosd
ret


clear_screen:
; очистить экран
   mov     ecx,80*40
   mov     edi,0x78000
   xor     eax,eax
 @@:
   mov     [edi],eax
   add     edi,4
   dec     ecx
   jnz     @b
ret

invalidate_string:
   imul    eax,[posy],80
   add     eax,0x78000
   mov     edi,eax
   mov     al,1
   mov     ecx,80/4
   rep     stosd
ret

layout:
; сменить раскладку клавиатуры
   mov     eax,19
   mov     ebx,setup
   mov     ecx,param_setup
   int     0x40
   mov     eax,5
   mov     ebx,eax
   int     0x40
   call    activate_me
ret


activate_me:
; 1) get info about me
   mov     eax,9
   mov     ebx,procinfo
   mov     ecx,-1
   int     0x40
   ; eax = number of processes

; save process counter
   inc     eax
   inc     eax
   mov     [proccount],eax

   mov     eax,[procinfo.PID]
   mov     [PID],eax

; 2) get my process number
   mov     eax,9
   mov     ebx,procinfo
   mov     ecx,[proccount]
 @@:
   dec     ecx
   jz      @f    ; counter=0 => not found? => return
   mov     eax,9
   int     0x40
   mov     edx,[procinfo.PID]
   cmp     edx,[PID]
   jne     @b

   ;found: ecx = process_number
   mov     eax,18
   mov     ebx,3
   int     0x40

   mov     eax,5
   mov     ebx,eax
   int     0x40

 @@:
ret




; *******************************************************************
; **************************  DRAW WINDOW  **************************
; *******************************************************************

align 4
drawwindow:

    mov  eax,12                   ; WINDOW DRAW START
    mov  ebx,1
    int  0x40
  mov [menu_is_on],0
    mov  eax,48  ; get system colors
    mov  ebx,3
    mov  ecx,sc
    mov  edx,sizeof.system_colors
    int  0x40

    mov  [sc.work],0xe0e0e0

    xor  eax,eax                  ; DEFINE WINDOW
    mov  ebx,100*65536+506 ; 496
    mov  ecx,75*65536+400 ;385;400  ; sum < 480 for 640x480
    mov  edx,[sc.work]
    add  edx,0x03000000
    mov  esi,[sc.grab]
    or   esi,0x80000000
    mov  edi,[sc.frame]
    int  0x40

; header string
    mov  eax,4
    mov  ebx,10*65536+8
    mov  ecx,[sc.grab_text]
    mov  edx,header
    mov  esi,[headlen]
    int  0x40

    mov  eax,9    ; get info about me
    mov  ebx,procinfo
    or   ecx,-1
    int  0x40

    mov  eax,[procinfo.y_size]

    mov  [do_not_draw],1 ; do_not_draw = true
    cmp  eax,100
    jb   .no_draw        ; do not draw text & buttons if height < 100
    mov  [do_not_draw],0 ; do_not_draw = false
    add  eax,-(46+47) ;  46 = y offs
    cdq
    mov  ebx,10
    div  ebx
    mov  [slines],eax

    cmp  eax,[posy]
    jnb  @f
    dec  eax
    mov  [posy],eax
  @@:

    mov  eax,[procinfo.y_size] ; calculate buttons position
    add  eax,-47
    mov  [dstart],eax

;    mov  eax,8                   ; STRING BUTTON
;    mov  ebx,5*65536+57
;    mov  ecx,[dstart]
;    add  ecx,29
;    shl  ecx,16
;    add  ecx,13
;    mov  edx,51              ;;;;;-----string button ID=51
;    mov  esi,[sc.work_button]
;    int  0x40
                                  ; SEARCH BUTTON
;    mov  ebx,(505-129)*65536+125
;    mov  edx,50
;    mov  esi,[sc.work_button]
;    int  0x40

;    mov  eax,4                   ; SEARCH TEXT
;    mov  ebx,[dstart]
;    add  ebx,7*65536+32
;    mov  ecx,[sc.work_button_text]
;    mov  edx,searcht
;    mov  esi,searchtl-searcht
;    int  0x40


    mov  eax,13                   ; BAR STRIPE
    mov  ebx,5*65536+497
    mov  ecx,[dstart]
    shl  ecx,16
    add  ecx,30 ;15

    mov  edx,0x00aaaaaa
    int  0x40

;    mov  eax,4                   ; FIRST TEXT LINE (POSITION...)
;    mov  ebx,12*65536
;    add  ebx,[dstart]
;    add  ebx,38 ;18
;    mov  ecx,[sc.work_button_text]
;    mov  edx,htext2
;    mov  esi,htextlen2-htext2
;    int  0x40


    call drawfile

;    mov  eax,[dstart]

;    add  eax,31
;    mov  [ya],eax
;    mov  [addr],search_string
;    call print_text

  .no_draw:
  call draw_win_menu

  call draw_vertical_scroll

    mov  eax,12                   ; WINDOW DRAW END
    mov  ebx,2
    int  0x40

    ret




; **********************************
; ***********  DRAWFILE  ***********
; **********************************

drawfile:
;---------------
cmp [menu_is_on],1
jne .ff
call drawwindow
.ff:
;---------------
    mov  [next_not_quote],1
    mov  [next_not_quote2],1

    mov  eax,[post]        ; print from position

    pusha

    mov  edi,[post]
    mov  [posl],edi

    mov  ebx,8*65536+46    ; letters (46 = y offs)
    xor  ecx,ecx

    mov  edx,0x80000
    add  edx,eax
    mov  edi,edx

    imul esi,[slines],80
    add  edi,esi


  nd:

    pusha

    mov       edx,ebx
    mov       edi,ebx
    add       edi,(6*65536)*80

  wi1:


    ; draw ?


    pusha

    push      ecx

    imul      eax,[posx],6
    add       eax,8
    shl       eax,16
    mov       ecx,eax

;    ecx = ([posx]*6+8)<<16

    imul      eax,[posy],10
    add       eax,46  ; y offs
    add       eax,ecx

;    eax = [posy]*10+46+ecx

    pop       ecx

    cmp       edx,eax
    jnz       drwa

    mov       eax,0x7ffff
    call      check_pos
    jmp       drlet

  drwa:

    popa


    pusha

    imul      eax,[posxm],6
    add       eax,8
    shl       eax,16
    mov       ecx,eax

    imul      eax,[posym],10
    add       eax,46     ; y offs
    add       eax,ecx

    cmp       edx,eax
    jnz       drwa2

    mov       eax,0x7ffff
    call      check_pos
    jmp       drlet

  drwa2:

    popa

    pusha

    mov       eax,0x78000  ; screen
    add       eax,[posl]   ; screen+abs
    sub       eax,[post]   ; eax = screen+abs-base = y*80+x + screen

    mov       edx,0x80000 ; file
    add       edx,[posl]  ; edx = absolute
    mov       bl,[edx]    ; in the file

    call      check_pos

    mov       cl,[eax]   ; on the screen
    cmp       bl,cl
    jnz       drlet

    popa

    jmp       nodraw


    ; draw letter


  drlet:

    mov       [eax],bl ; mov byte to the screen
    mov       [tmpabc],bl
    popa      ; restore regs

;!!!!!!!!!!!!

    cmp       [tmpabc],' '
    je        @f
    call      draw_letter
    jmp       nodraw
   @@:
    call      clear_char

    nodraw:

    inc       [posl]

    add       edx,6*65536
    cmp       edx,edi
    jz        wi3
    jmp       wi1

  wi3:

    popa

    add       ebx,10
    add       edx,80
    cmp       edi,edx
    jbe       nde
    jmp       nd

  nde:

    mov       eax,[posx]
    mov       ebx,[posy]

    mov       [posxm],eax
    mov       [posym],ebx

    popa

    ret

 stText    = 0
 stInstr   = 1
 stReg     = 2
 stNum     = 3
 stQuote   = 4
 stComment = 5
 stSymbol  = 6

align 4

clear_char:

    pusha
    mov       ebx,[sc.work]

    push      ecx

    imul      eax,[posx],6
    add       eax,8
    shl       eax,16
    mov       ecx,eax

    imul      eax,[posy],10
    add       eax,46 ; 26
    add       eax,ecx

    pop       ecx
    cmp       edx,eax
    jnz       @f
    mov       ebx,0xffffff   ; light blue 0x00ffff
  @@:

                     ; draw bar
    push      ebx
    mov       eax,13
    mov       ebx,edx
    mov       bx,6
    mov       ecx,edx
    shl       ecx,16
    add       ecx,10
    pop       edx
    int       0x40
    popa
    ret

align 4

; CHECK_POSITION {
check_pos:
  cmp  [asm_mode],1
  je   @f

  mov  [d_status],stText
  ret

 @@:
  pushad

; COMMENT TERMINATOR
  cmp  [d_status],stComment
  jnz  @f
  mov  eax,[posl]
  sub  eax,[post]
  cdq
  mov  ebx,80
  div  ebx
  test edx,edx
  jnz  end_check_pos
  mov  [d_status],stText
 @@:

; QUOTE TERMINATOR B
  cmp  [next_not_quote],1
  jne  @f
  mov  [d_status],stText
 @@:

  mov  eax,[posl]
  add  eax,0x80000
  mov  edx,eax
  mov  al,[eax]

; QUOTE TERMINATOR A
  cmp  [d_status],stQuote
  jnz  @f
  cmp  al,[quote]
  jne  end_check_pos
  mov  [next_not_quote],1
  jmp  end_check_pos
 @@:
  mov  [next_not_quote],0

; START QUOTE 1
  cmp  al,"'"
  jne  @f
  mov  [d_status],stQuote
  mov  [quote],al
  jmp  end_check_pos
 @@:

; START QUOTE 2
  cmp  al,'"'
  jne  @f
  mov  [d_status],stQuote
  mov  [quote],al
  jmp  end_check_pos
 @@:

; START COMMENT
  cmp  al,';'
  jne  @f
  mov  [d_status],stComment
  jmp  end_check_pos
 @@:

; NUMBER TERMINATOR
  cmp  [d_status],stNum
  jne  nonumt
  mov  ecx,23
 @@:
  dec  ecx
  jz   nonumt
  cmp  al,[symbols+ecx]
  jne  @b

 nonumt1:
  mov  [d_status],stText
 nonumt:

; START NUMBER
  cmp  [d_status],stNum
  je   end_check_pos
  cmp  al,'0'
  jb   nonum
  cmp  al,'9'
  ja   nonum
  mov  bl,[edx-1]
  mov  ecx,23
 @@:
  dec  ecx
  jz   nonum
  cmp  bl,[symbols+ecx]
  jne  @b
 @@:
  mov  [d_status],stNum
  jmp  end_check_pos
 nonum:

; SYMBOL
  mov   esi,symbols
  mov   ecx,21
 @@:
  cmp   byte [esi],al
  je    @f
  dec   ecx
  jz    nosymbol
  inc   esi
  jmp   @b
 @@:
  mov   [d_status],stSymbol
  jmp   end_check_pos

 nosymbol:
  mov   [d_status],stText

 end_check_pos:
  popad
  ret
; CHECK_POSITION }


;;;;;;;;;;;;;;;;;
;; DRAW LETTER ;;
;;;;;;;;;;;;;;;;;
draw_letter:

    call      clear_char

    pusha

    mov       ebx,edx  ; x & y

    mov       eax,[d_status]
    mov       ecx,[eax*4+color_tbl]
    mov       eax,4

    xor       esi,esi
    inc       esi
    mov       edx,0x80000
    mov       edi,[posl]
    add       edx,edi
    int       0x40

    popa

    ret


; ********************************************
; ****************  SAVEFILE  ****************
; ********************************************
save_file:
   mov     esi,0x80000
   mov     edi,0x10000
   or      ecx,-1
 .new_string:
   inc     ecx
   call    save_string
   cmp     ecx,[lines]
   jb      .new_string

   sub     edi,0x10004  ; why???
   mov     [filelen],edi

   cmp     byte [filename],'/'
   je      .systree_save

   mov     eax,33
   mov     ebx,filename
   mov     ecx,0x10000
   mov     edx,[filelen]
   xor     esi,esi
   int     0x40

   test    eax,eax
   je      .finish
;   call    file_not_found
   call disk_is_full
;==============================
   jmp     .finish

 .systree_save:
   mov     eax,[filelen]
   mov     [fileinfo_write+8],eax

   mov     esi,filename
   mov     edi,pathfile_write
   mov     ecx,50
   cld
   rep     movsb

   mov     eax,58
   mov     ebx,fileinfo_write
   int     0x40
   cmp eax,0
   je .finish
   call disk_is_full
 .finish:
call draw_window_for_dialogs
    mov [modified],0
ret

save_string:
   push    ecx
   push    esi
   mov     eax,esi
   mov     ebx,eax
   add     ebx,79
 .countlen:
   cmp     ebx,eax
   jb      .endcount
   cmp     byte [ebx],' '
   jne     .endcount
   dec     ebx
   jmp     .countlen
 .endcount:
   inc     ebx
   sub     ebx,eax

   mov     ecx,ebx
   jecxz   .endcopy
 .copystr:
   mov     al,[esi]
   mov     [edi],al
   inc     esi
   inc     edi
   dec     ecx
   jnz     .copystr
 .endcopy:

   mov     eax,0x0a0d
   stosw

   pop     esi
   add     esi,80
   pop     ecx
ret



; ********************************************
; ****************  LOADFILE  ****************
; ********************************************

loadhdfile:

     mov  esi,filename
     mov  edi,pathfile_read
     mov  ecx,250 ;50
     cld
     rep  movsb

     mov  eax,58
     mov  ebx,fileinfo_read
     int  0x40

     xchg eax,ebx
     inc  eax
     test ebx,ebx   ;errorcode=0 - ok
     je   file_found
     cmp  ebx,6     ;errorcode=5 - ok
     je   file_found
     call file_not_found
     ret


loadfile1:

    mov  eax,6        ; 6 = open file
    mov  ebx,filename
    xor  ecx,ecx
    mov  edx,16800
    mov  esi,0x10000
    int  0x40

    inc  eax          ; eax = -1 -> file not found
    jnz  file_found   ;strannaya proverka (Ed)

    call file_not_found
    ret


  file_found:
    dec  eax
;   eax = file size
    jz   .finish
    mov  [filesize],eax

    mov  edi,0x80000     ; clear all
  @@:
    mov  [edi],byte ' '
    inc  edi
    cmp  edi,0x2effff
    jnz  @b

    mov  [lines],0
    mov  edi,0x10000
    mov  ebx,0x80000

; edi = from
; ebx = to
; eax = filesize

  .new_char:
    mov  cl,[edi]     ; get_char();
    cmp  cl,13        ; if (char==13)
    je   .new_str1    ;   goto .new_str1;
    cmp  cl,10        ; if (char==10)
    je   .new_str2    ;   goto .new_str2;
    mov  [ebx],cl     ; store_char();
    inc  ebx          ; dest++;
  .back:
    inc  edi          ; src++;
    dec  eax          ; counter--;
    jnz  .new_char    ; if (counter!=0) goto .new_char;

  .finish:
    inc  [lines]      ;   [lines]++;
    ret

  .new_str1:
    pusha
    mov  eax,ebx        ; eax = destination
    add  eax,-0x80000   ; eax = offset
    cdq
    mov  ecx,80
    div  ecx            ; offset /= 80;
    test edx,edx        ; if not the first char in the string
    jne  @f             ;   go forward
    test eax,eax        ; if first line
    je   @f             ;   go forward
    cmp  [edi-1],byte 10; if previous char != 10 continue without line feed
    jne  .contin
   @@:
    inc  eax            ; offset++;
    imul eax,80         ; offset *= 80;
    add  eax,0x80000
    mov  [esp+4*4],eax  ; to ebx
   .contin:
    popa
    inc  edi     ; do not look on the next char (10)
    dec  eax     ; counter--;
    inc  [lines] ; [lines]++;
    jmp  .back


  .new_str2:
    pusha
    mov  eax,ebx
    add  eax,-0x80000
    cdq
    mov  ecx,80
    div  ecx
    inc  eax
    imul eax,80
    add  eax,0x80000
    mov  [esp+4*4],eax ; to ebx
    popa
    inc  [lines]
    jmp  .back


file_not_found:
   mov  eax,55           ; beep
   mov  ebx,eax
   mov  esi,error_beep
   int  0x40
   mov  [lines],1        ; open empty document

   mov [to_return2],1
   call openerror

   ret

disk_is_full:
   mov  eax,55           ; beep
   mov  ebx,eax
   mov  esi,error_beep
   int  0x40
   mov [to_return2],1
   call saveerror
   mov [error2_found],1
ret


; *****************************
; ******  WRITE POSITION ******
; *****************************

writepos:

    cmp [do_not_draw],1  ; return if drawing is not permitted
    jne @f
    ret
   @@:

    pusha

    mov  eax,[posx]
    inc  eax
    cdq
    mov  ebx,10
    div  ebx
    add  al,'0'
    add  dl,'0'
    mov  [htext2+ 9],al
    mov  [htext2+10],dl

    mov  eax,[post]
    cdq
    mov  ebx,80
    div  ebx
    mov [real_posy],eax ;=====!!!!!!!!!

    add  eax,[posy]
    inc  eax
    mov  ebx,10
    cdq
    div  ebx
    add  dl,'0'
    mov  [htext2+16],dl  ; 00001
    cdq
    div  ebx
    add  dl,'0'
    mov  [htext2+15],dl  ; 00010
    cdq
    div  ebx
    add  dl,'0'
    mov  [htext2+14],dl  ; 00100
    cdq
    div  ebx
    add  dl,'0'
    add  al,'0'
    mov  [htext2+13],dl  ; 01000
    mov  [htext2+12],al  ; 10000


    mov  eax,[lines]     ; number of lines
    cdq
    mov  ebx,10
    div  ebx
    add  dl,'0'
    mov  [htext2+31],dl  ; 0001
    cdq
    div  ebx
    add  dl,'0'
    mov  [htext2+30],dl  ; 0010
    cdq
    div  ebx
    add  dl,'0'
    mov  [htext2+29],dl  ; 0100
    cdq
    div  ebx
    add  dl,'0'
    add  al,'0'
    mov  [htext2+28],dl
    mov  [htext2+27],al  ; 10000

;   НАДО БЫ ОТОБРАЖАТЬ РАСКЛАДКУ КЛАВИАТУРЫ!
;    mov  [htext2+42], word 'RU'

;    mov  eax,13      ; draw bar
;    mov  ebx,5*65536+38*6
;    mov  ecx,[dstart]
;    shl  ecx,16
;    add  ecx,15
;    mov  edx,[sc.work_graph]
;    int  0x40

    mov  eax,13                   ; BAR STRIPE
    mov  ebx,5*65536+497
    mov  ecx,[dstart]
    add  ecx,29;30 ;15
    shl  ecx,16
    add  ecx,14
    mov  edx,[sc.work_graph]
    int  0x40

    mov  eax,4       ; write position
    mov  ebx,12*65536
    mov  bx,word [dstart]
    add  ebx,33 ;18
    mov  ecx,[sc.work_button_text]
    mov  edx,htext2
    mov  esi,38
    int  0x40

    cmp [modified],1
    jne no_mod
if lang eq ru
     putlabel 270,386,'ИЗМЕНЕН',[sc.work_button_text]
else
     putlabel 270,386,'MODIFIED',[sc.work_button_text]
end if
    no_mod:
    popa

    ret

;-----------------------------
;   search window
;-----------------------------
search_window:

mov eax,13
mov ebx,55*65536+380
mov ecx,100*65536+60
mov edx,[sc.work_graph]
int 0x40

mov eax,38
mov ebx,55*65536+435
mov ecx,100*65536+100
mov edx,cl_White
int 0x40
mov eax,38
mov ebx,55*65536+55
mov ecx,100*65536+160
mov edx,cl_White
int 0x40
mov eax,38
mov ebx,435*65536+435
mov ecx,100*65536+160
mov edx,cl_White
int 0x40
mov eax,38
mov ebx,55*65536+435
mov ecx,160*65536+160
mov edx,cl_White
int 0x40

if lang eq ru
drawlbut 375,110,50,15,'Поиск',50,[sc.work_button],[sc.work_button_text]
drawlbut 375,130,50,15,'Отмена',94,[sc.work_button],[sc.work_button_text]
else
drawlbut 375,110,50,15,'Search',50,[sc.work_button],[sc.work_button_text]
drawlbut 375,130,50,15,'Cancel',94,[sc.work_button],[sc.work_button_text]
end if

call read_string

;    add  eax,31
;    mov  [ya],eax
;    mov  [addr],search_string
;    call print_text



jmp search

string_not_found:
 mov eax,13
 mov ebx,150*65536+200
 mov ecx,100*65536+70
 mov edx,[sc.work_graph] ;0x00dd9438 ;0x00ff7512
 int 0x40

mov eax,38
mov ebx,150*65536+350
mov ecx,100*65536+100
mov edx,cl_White
int 0x40
mov eax,38
mov ebx,150*65536+350
mov ecx,170*65536+170
mov edx,cl_White
int 0x40
mov eax,38
mov ebx,150*65536+150
mov ecx,100*65536+170
mov edx,cl_White
int 0x40
mov eax,38
mov ebx,350*65536+350
mov ecx,100*65536+170
mov edx,cl_White
int 0x40

if lang eq ru
 putlabel 195,120,'Строка не найдена!',cl_White
else
 putlabel 195,120,'String not found!',cl_White
end if

 drawlbut 235,140,30,15,'Ок',94,[sc.work_button],cl_White

ret

read_string:

push eax
;----------------
mov eax,40
mov ebx,00000000000000000000000000000111b
int 0x40
;----------------
pop eax

;    cmp  al,51
;    jz  .f2
;    ret

  .f2:
    mov  [addr],dword search_string
    mov  eax,[dstart]
    add  eax,17+14
    mov  [ya],eax
    mov  [case_sens],1

  .rk:

    mov  edi,[addr]

    mov  eax,[addr]
    mov  eax,[eax-4]
    mov  [temp],eax

    add  edi,eax

    call print_text

  .waitev:
    mov  eax, 10
    int  0x40
    cmp  eax, 2
    jne  .read_done
    int  0x40
    shr  eax, 8

    cmp  al, 13     ; enter
    je   .read_done

    cmp al,27
    jne ._f
    jmp red
    ._f:
    cmp  al, 192    ; Ctrl + space
    jne  .noclear

    xor  eax, eax
    mov  [temp], eax
    mov  edi, [addr]
    mov  [edi-4], eax
    mov  ecx, 49
    cld
    rep  stosb
    mov  edi, [addr]
    call print_text
    jmp  .waitev

  .noclear:

    cmp  al, 8      ; backspace
    jnz  .nobsl
    cmp  [temp], 0
    jz   .waitev
    dec  [temp]
    mov  edi, [addr]
    add  edi, [temp]
    mov  [edi], byte 0

    mov  eax,[addr]
    dec  dword [eax-4]

    call print_text
    jmp  .waitev

  .nobsl:
    cmp  [temp],50
    jae  .read_done

; CONVERT CHAR TO UPPER CASE:
    cmp  al, ' '        ; below "space" - ignore
    jb   .waitev
    cmp  [case_sens], 1 ; case sensitive?
    je   .keyok
    cmp  al, 'a'
    jb   .keyok
    cmp  al, 'z'
    ja   .keyok
    sub  al, 32
   .keyok:

    mov  edi,[addr]
    add  edi,[temp]
    mov  [edi],al

    inc  [temp]

    mov  eax,[addr]
    inc  dword [eax-4]
    call print_text

    cmp  [temp],50
    jbe  .waitev

  .read_done:
    mov ecx,50
    sub ecx,[temp]
    mov edi,[addr]
    add edi,[temp]
    xor eax,eax
    cld
    rep stosb

    mov [temp],987

    call print_text
    call mask_events
    ret


print_text:

    pusha

    mov  eax,13
    mov  ebx,64*65536+50*6+2
;    mov  ecx,[ya]
;    shl  ecx,16
;    add  ecx,12
    mov ecx,110*65536+12
    mov  edx,[sc.work]
    int  0x40

    mov  edx,[addr]
    mov  esi,[edx-4]
    mov  eax,4
    mov  ebx,65*65536+112 ;2
;    add  ebx,[ya]
    mov  ecx,[color_tbl+0]
    int  0x40

    cmp  [temp],50
    ja   @f

; draw cursor
; {
;    mov  eax,[ya]
    mov eax,18*65536+102 ;65
    mov  ebx,eax
    shl  eax,16
    add  eax,ebx
    add  eax,10
    mov  ecx,eax

    mov  eax,[temp]
;   imul eax,6
    lea  eax,[eax+eax*2]
    shl  eax,1
    add  eax,65
    mov  ebx,eax
    shl  eax,16
    add  ebx,eax

    mov  eax,38
    mov  edx,[color_tbl+0]
    int  0x40
; }

@@:
    popa

    ret



;    mov  eax,8                   ; STRING BUTTON
;    mov  ebx,5*65536+57
;    mov  ecx,[dstart]
;    add  ecx,29
;    shl  ecx,16
;    add  ecx,13
;    mov  edx,51              ;;;;;-----string button ID=51
;    mov  esi,[sc.work_button]
;    int  0x40
                                  ; SEARCH BUTTON
;    mov  ebx,(505-129)*65536+125
;    mov  edx,50
;    mov  esi,[sc.work_button]
;    int  0x40

;    mov  eax,4                   ; SEARCH TEXT
;    mov  ebx,[dstart]
;    add  ebx,7*65536+32
;    mov  ecx,[sc.work_button_text]
;    mov  edx,searcht
;    mov  esi,searchtl-searcht
;    int  0x40



; ****************************
; ******* READ STRING ********
; ****************************
goto_string:

mov [num_goto_string],0
call read_str_num
mov eax,[num_goto_string]
cmp eax,[lines]
ja .leave

;---------------
mov [posy],0
call goto_pos

.leave:
call draw_window_for_dialogs
call mask_events

jmp still


read_str_num:
push eax
;----------------
mov eax,40
mov ebx,00000000000000000000000000000111b
int 0x40
;----------------
pop eax

mov eax,13
mov ebx,100*65536+100
mov ecx,70*65536+60
mov edx,[sc.work_button]
int 0x40


mov eax,38
mov ebx,100*65536+200
mov ecx,70*65536+70
mov edx,cl_White
int 0x40
mov eax,38
mov ebx,100*65536+200
mov ecx,130*65536+130
mov edx,cl_White
int 0x40
mov eax,38
mov ebx,100*65536+100
mov ecx,70*65536+130
mov edx,cl_White
int 0x40
mov eax,38
mov ebx,200*65536+200
mov ecx,70*65536+130
mov edx,cl_White
int 0x40



putlabel 105,75,'GoTo Line #',cl_White
    mov  eax,13
    mov  ebx,110*65536+40
    mov  ecx,90*65536+12;[ya]
    mov  edx,[sc.work]
    int  0x40

outcount [num_goto_string],112,92,cl_Black,6*65536
drawlbut 110,105,40,15,'GoTo',93,cl_Grey,cl_Black
drawlbut 153,105,40,15,'Cancel',94,cl_Grey,cl_Black
  .waitev:
    mov  eax, 10
    int  0x40
;    cmp eax,6
;    je .mouse
    cmp eax,3
    je .but
    cmp  eax, 2
;    jne  .read_done
    jne .waitev
    int  0x40
    shr  eax, 8

    cmp  al, 13     ; enter
    je   .read_done
    cmp al,27
    je goto_string.leave
    cmp  al, 8      ; backspace
    jnz  .nobsl

xor edx,edx

mov eax,[num_goto_string]
mov ebx,dword 10
div ebx
mov [num_goto_string],eax
call print_text2
jmp .waitev

;.mouse:
;mov eax,37
;mov ebx,2
;int 0x40
;cmp eax,2
;je goto_string.leave
;jmp .waitev

.but:
mov eax,17
int 0x40
cmp ah,94
je goto_string.leave
cmp ah,93
je .read_done
jmp .waitev


  .nobsl:
xor ecx,ecx
xor edx,edx

sub al,48
mov cl,al

mov eax,[num_goto_string]
cmp eax,99999
ja .read_done
mov ebx,10
mul ebx
add eax,ecx
mov [num_goto_string],eax

call print_text2
jmp .waitev

  .read_done:
mov eax,[num_goto_string]
dec eax
mov [num_goto_string],eax
    call print_text2
    ret


print_text2:

    pusha

    mov  eax,13
    mov  ebx,110*65536+40
    mov  ecx,90*65536+12;[ya]
    mov  edx,[sc.work]
    int  0x40

outcount [num_goto_string],112,92,cl_Black,6*65536
    popa

    ret

;******************************************************************************
calc_scroll_size_and_pos:

;cmp [menu_is_on],0
;je ._ff
;call drawwindow
;mov [menu_is_on],0
;._ff:

cmp [lines],30
jbe .lines_less_30

xor edx,edx
mov eax,[post]
mov ebx,80
div ebx
add eax,[posy]
;add eax,[slines]

;checking for bug
mov ebx,[lines]
sub ebx,30

cmp eax,ebx
ja .f
mov [VScroll_1+16],eax
jmp .ff
.f:
mov [VScroll_1+16],ebx

.ff:
;---------------------
mov eax,[lines]
sub eax,30       ;---max=lines-30
mov [VScroll_1+12],eax
jmp .leave

.lines_less_30:

mov [VScroll_1+16],dword 0
mov [VScroll_1+12],dword 1

.leave:

ret
;============Draw vertical scroll bar=========
draw_vertical_scroll:
call calc_scroll_size_and_pos
;========================
    xor  ecx,ecx                    ;start at top of controls list
Draw_Controls_Loop:                 ;Redraw Controls Loop
    mov  ebp, [App_Controls+ecx]    ;get controls data location
    or   ebp,ebp                    ;is this the last control?
    jz   Draw_Controls_Done         ;
    call dword [App_Controls+ecx+4] ;call controls draw function
    add  ecx, 12                    ;get next control
    jmp  Draw_Controls_Loop         ;loop till done
Draw_Controls_Done:                 ;all done
;========================

mov eax,38
mov ebx,488*65536+488
mov ecx,43*65536+388
mov edx,0x00000000
int 0x40
ret

mouse_info:
;call
   mov eax, 37            ;get mouse cordinates
   mov ebx, 1             ;
   int 0x40               ;
   mov ecx, eax           ;
   push ecx               ;
   mov eax, 37            ;get mouse buttons
   mov ebx, 2             ;
   int 0x40               ;
;------------------
; if menu is on - then we need to redraw window before continue
cmp eax,1
jne ._f1

pusha
cmp [menu_is_on],0
je ._ff
call drawwindow
mov [menu_is_on],0
._ff:
popa

._f1:
;------------------
   cmp [mouseb], eax      ;compare old mouse states to new states
   jne redraw_mouse_info  ;
   cmp [mousey], cx       ;
   jne redraw_mouse_info  ;
   shr ecx, 16            ;
   cmp [mousex], cx       ;
   jne redraw_mouse_info  ;
   pop ecx                ;
ret                       ;return if no change in states
redraw_mouse_info:
   pop   ecx
   mov   [mouseb], eax         ;save new mouse states
   mov   dword [mousey], ecx
   xor   ecx, ecx
Check_Mouse_Over_Controls_Loop:
   mov   ebp, [App_Controls+ecx]
   or    ebp, ebp
   jz    Check_Mouse_Over_Controls_Loop_done

   movzx eax,word [ebp+2]
   cmp    ax, [mousex]
   ja    mouse_not_on_control
   movzx eax,word [ebp+6]
   cmp    ax, [mousey]
   ja    mouse_not_on_control
   movzx eax,word [ebp]
   add    ax, [ebp+2]
   cmp    ax, [mousex]
   jb    mouse_not_on_control
   movzx eax,word [ebp+4]
   add    ax, [ebp+6]
   cmp    ax, [mousey]
   jb    mouse_not_on_control
   call  dword [App_Controls+ecx+8]
   ;------------------------------
   cmp [mouseb],1
   jne mouse_not_on_control
   mov eax,[VScroll_1+16]
   call goto_pos
   ;------------------------------

mouse_not_on_control:
   add ecx, 12
   jmp Check_Mouse_Over_Controls_Loop
Check_Mouse_Over_Controls_Loop_done:

ret
;******************************************************************************
goto_pos:
;pusha

mov ecx,eax   ;save new position number in ecx for future

cmp [lines],30    ;check for 'cursor' bug
jbe .lines_less_30
;---------------

mov edx,[lines]   ;if new pos is > than (lines-30)
sub edx,30

cmp eax,edx
ja .f1
jmp .ff

.f1:
mov eax,edx       ;than newpos is = (lines-30)

sub ecx,edx       ;and posY=newpos-(lines-30)
mov [posy],ecx

.ff:

;-----------------------
   ;in eax must be string number
   mov ecx,80
   mul ecx

;   add eax,[slines]
;   sub
;------------------------
   mov [post],eax

.lines_less_30:
    call clear_screen
    call drawfile
;popa
ret
;******************************************************************************
mask_events:
mov eax,40
mov ebx,00100111b
int 0x40
ret
;******************************************************************************
main_cursor_move:
;call drawwindow
sub [mouse_x],7
sub [mouse_y],45

xor edx,edx
mov eax,[mouse_x]
mov ebx,6
div ebx ;eax=result
mov [posx],eax

xor edx,edx
mov eax,dword [mouse_y]
mov ebx,dword 10
div ebx ;eax=result=new posY

;error checking ------
cmp [lines],dword 1 ;for "1st line" bug
je ._do_nothing

mov ebx,[lines]
sub ebx,dword 1

cmp eax,ebx ;[lines]
ja ._do_nothing

;----------------------
mov [posy],eax

._do_nothing:
call clear_screen
call drawfile
call draw_vertical_scroll
ret

;******************************************************************************
make_fast_so:
;===========================
; 1) get info about me
   mov     eax,9
   mov     ebx,procinfo
   mov     ecx,-1
   int     0x40
   ; eax = number of processes

; save process counter
   inc     eax
   inc     eax
   mov     [proccount],eax

   mov     eax,[procinfo.PID]
   mov     [PID],eax
;==========================


mov eax,51
mov ebx,1
mov ecx,fast_so_thread_start
mov edx,so_stack
int 0x40
ret
;******************************************************************************

;fast save & fast open
draw_fastso_window:
startwd
colorwindow 120,100,454,70,window_Type1+0x00cccccc,0x00cccccc,cl_Black

call draw_string00
drawlbut 10,40,30,20,'Save',17,cl_Grey,cl_Black

drawlbut 50,40,30,20,'Open',18,cl_Grey,cl_Black

drawlbut 90,40,37,20,'Close',19,cl_Grey,cl_Black
endwd
ret

draw_string00:
mov ebx,10*65536+433
mov ecx,10*65536+20
mov edx,0x00ffffff
mov eax,13
int 0x40

push eax
mov eax,6*65536
mul dword [pos00]
add eax,10*65536+6
mov ebx,eax
pop eax
mov edx,0x6a73d0
int 0x40

mov eax,4
mov ebx,12*65536+17
mov ecx,cl_Black ;0x00000000
mov edx,mypath ;filename    ;path
mov esi,71 ;200
int 0x40
ret

fast_so_thread_start:
;copy filename to mypath
    cld
    mov esi,filename
    mov edi,mypath
    mov ecx,71 ;200
    rep movsb
    mov    edi,mypath
    mov    ecx,71 ;200
    xor    eax,eax
    repne  scasb
;end copy
call draw_fastso_window

fastso_still:
  wtevent fred,fkey,fbut
jmp fastso_still

fred:
call draw_fastso_window
jmp fastso_still

;====KEY
fkey:

mov eax,2
int 0x40

cmp ah,179
jne noright00
mov eax,[pos00]
cmp eax,70 ;41
ja fastso_still
inc eax
mov [pos00],eax
call draw_string00
jmp fastso_still
noright00:
cmp ah,176
jne noleft00
mov eax,[pos00]
test eax,eax
je fastso_still
dec eax
mov [pos00],eax
call draw_string00
jmp fastso_still
noleft00:
cmp ah,182
jne nodelete00
call shiftback00
call draw_string00
jmp fastso_still
nodelete00:
cmp ah,8
jne noback00
mov eax,[pos00]
test eax,eax
je fastso_still
dec eax
mov [pos00],eax
call shiftback00
call draw_string00
jmp fastso_still
noback00:
enter00:

cmp ah,19 ;ctrl+s
je fast_save
cmp ah,15 ;ctrl+o
je fast_open

cmp ah,27  ;esli escape
jne noesc00
jmp closethis      ;to zakrivaem okno i nifiga ne delayem

noesc00:
cmp dword [pos00],71 ;200 ;42
jae fastso_still ;if pos>71 then jump to still

;============letters==================
;~~~~~~~TEST CODE~~~~~~~~~
; upper case
shr eax,8
 cmp eax,dword 31
 jbe no_lit
 cmp eax,dword 95
 jb  capital
 sub eax,32
 capital:
;~~~~~~~~~~~~~~~~~~~~~~~~~
mov edi,mypath ;filename ;mypath   ;**************PATHNAME
add edi,71 ;200    ;count of letters
mov esi,edi
dec esi
mov ecx,71 ;200    ;again???
sub ecx,[pos00]
std
rep movsb

;shr eax,8
mov esi,mypath ;filename  ;*************PATH AGAIN
add esi,[pos00]
mov byte [esi],al
inc dword [pos00]
call draw_string00

no_lit:
jmp fastso_still
;===============================
shiftback00:
mov edi,mypath ;filename ;******PATH
add edi,[pos00]
mov esi,edi
inc esi
mov ecx,71 ;200  ; count???
sub ecx,[pos00]
cld
rep movsb
ret

;******************************************************************************

;====button
fbut:
mov eax,17
int 0x40
cmp ah,17
je fast_save
cmp ah,18
je fast_open
cmp ah,19
je closethis
jmp fastso_still
;******************************************************************************
;******************************************************************************

fast_open:
call path_copy
call set_title
mov [to_return],1
call do_load_file
jmp closethis
fast_save:
call path_copy
call save_file
call set_title
;call copy_fpath_s
closethis:
mov [to_return],0
cmp [error_found],1
je @f
call activate_main
@@:
mov [error_found],0
close


activate_main:
   mov     eax,9
   mov     ebx,procinfo
   mov     ecx,[proccount]
 @@:
   dec     ecx
   jz      @f    ; counter=0 => not found? => return
   mov     eax,9
   int     0x40
   mov     edx,[procinfo.PID]
   cmp     edx,[PID]
   jne     @b

   ;found: ecx = process_number
   mov     eax,18
   mov     ebx,3
   int     0x40

   mov     eax,5
   mov     ebx,eax
   int     0x40

 @@:
ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
doyouwannasave:
;******************************************************************************
cmp [modified],0
je newfile
      mov  eax,55           ; beep
      mov  ebx,eax
      mov  esi,save_beep1
      int  0x40
      delay 4
      mov  eax,55           ; beep
      mov  ebx,eax
      mov  esi,save_beep2
      int  0x40

 mov eax,13
 mov ebx,150*65536+200
 mov ecx,100*65536+70
 mov edx,[sc.work_graph] ;0x00dd9438 ;0x00ff7512
 int 0x40

mov eax,38
mov ebx,150*65536+350
mov ecx,100*65536+100
mov edx,cl_White
int 0x40
mov eax,38
mov ebx,150*65536+350
mov ecx,170*65536+170
mov edx,cl_White
int 0x40
mov eax,38
mov ebx,150*65536+150
mov ecx,100*65536+170
mov edx,cl_White
int 0x40
mov eax,38
mov ebx,350*65536+350
mov ecx,100*65536+170
mov edx,cl_White
int 0x40

if lang eq ru
 putlabel 190,120,'Сохранить документ?',cl_White

 drawlbut 170,140,30,15,'Да',46,[sc.work_button],cl_White
 drawlbut 230,140,30,15,'Нет',45,[sc.work_button],cl_White
 drawlbut 290,140,45,15,'Отмена',47,[sc.work_button],cl_White
else
 putlabel 190,120,'Save document?',cl_White

 drawlbut 170,140,30,15,'Yes',46,[sc.work_button],cl_White
 drawlbut 230,140,30,15,'No',45,[sc.work_button],cl_White
 drawlbut 290,140,45,15,'Cancel',47,[sc.work_button],cl_White
end if

 mov [exit_wnd_on],1
 jmp still
;-------------
newfile:
;if filename is not NEWDOC## than change it to it!
mov eax,dword [orig_filename]
cmp dword [filename],eax ;[orig_filename]
je @f
    cld
    mov esi,orig_filename
    mov edi,filename
    mov ecx,71 ;50
    rep movsb
    mov    edi,filename
    mov    ecx,71 ;50
    xor    eax,eax
    repne  scasb
    sub    edi,filename
    dec    edi
    mov    [filename_len],edi

@@:

call change_fname
call set_title
call draw_window_for_dialogs
call empty_work_space
mov [lines],1
jmp do_load_file.restorecursor

change_fname:
cmp [filename+7],'9'
jne addfname
cmp [filename+6],'9'
je error_creating_new_file
mov [filename+7],'0'
add [filename+6],0x1
jmp leavenow
addfname:
add [filename+7],0x1
leavenow:
ret

set_title:
    cmp [error2_found],1
    je  no_set_title
    mov  esi, filename
    mov  edi, header
    mov  ecx, [filename_len]
    mov  eax, ecx
    add  eax, 10
    mov  [headlen], eax
    cld
    rep  movsb

    mov  [edi], dword ' -  '
    add  edi, 3
    mov  esi, htext
    mov  ecx, htext.size
    rep  movsb

    call drawwindow
no_set_title:
mov [error2_found],0
ret

draw_window_for_dialogs:
call clear_screen
call drawwindow
ret

copy_fpath:
call mask_events
call path_copy
call set_title
call draw_window_for_dialogs
jmp do_load_file

copy_fpath_s:
call mask_events
call path_copy
call save_file
call set_title
call draw_window_for_dialogs
jmp still

path_copy:
    cld
    mov esi,mypath
    mov edi,filename
    mov ecx,250 ;71 ;50
    rep movsb
    mov    edi,filename
    mov    ecx,250 ;71 ;50
    xor    eax,eax
    repne  scasb
    sub    edi,filename
    dec    edi
    mov    [filename_len],edi
ret

openerror:
mov eax,360
mov ebx,openerrtext
mov ecx,1
call alert_box

cmp [to_return2],1
jne jmp_to_still
mov [to_return2],0
mov [error_found],1
call mask_events

ret

saveerror:
mov eax,390
mov ebx,saveerrtext
mov ecx,1
call alert_box
cmp [to_return2],0
je jmp_to_still
mov [to_return2],0
mov [error_found],1
call mask_events
ret
;jmp still
jmp_to_still:
call mask_events
jmp still

error_creating_new_file:
mov eax,200
mov ebx,newfileerror
mov ecx,1
call alert_box
jmp still

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;MENU;;;;;;;;;;;;;;;;;;;;;;
draw_win_menu:

;mov eax,8
;mov ebx,145*65536+276
;mov ecx,22*65536+15
;mov edx,00100000000000000000000000001111b
;mov esi,0x00aaaaaa
;int 0x40

mov eax,13
mov ebx,5*65536+497
mov ecx,22*65536+17
mov edx,[sc.work_graph] ;0x00999999
int 0x40
mov ecx,39*65536+5
mov edx,0x00aaaaaa
int 0x40

if lang eq ru
drawlbut 5,22,70,15,'Файл',97,[sc.work_button],[sc.work_button_text]
drawlbut 75,22,70,15,'Код',98,[sc.work_button],[sc.work_button_text]
drawlbut 145,22,70,15,'Текст',96,[sc.work_button],[sc.work_button_text]
drawlbut 431,22,70,15,'Справка',99,[sc.work_button],[sc.work_button_text]
else
drawlbut 5,22,70,15,'File',97,[sc.work_button],[sc.work_button_text]
drawlbut 75,22,70,15,'Code',98,[sc.work_button],[sc.work_button_text]
drawlbut 145,22,70,15,'Text',96,[sc.work_button],[sc.work_button_text]
drawlbut 431,22,70,15,'Help',99,[sc.work_button],[sc.work_button_text]
end if

ret

draw_win_menu_file:
call clear_screen
call drawwindow
;mov eax,13
;mov ebx,5*65536+71
;mov ecx,35*65536+90
;mov edx,[sc.grab_button];0x00999999
;int 0x40
;mov eax,38
;mov ebx,5*65536+35
;mov ecx,40*65536+47
;mov edx,0x00ff0000
;int 0x40
mov [menu_is_on],1

if lang eq ru
drawlbut 5,38,70,15,'Новое окно',100,[sc.grab_button],[sc.grab_button_text]
drawlbut 5,53,70,15,'Новый',101,[sc.grab_button],[sc.grab_button_text]
drawlbut 5,68,70,15,'Сохранить',2,[sc.grab_button],[sc.grab_button_text]
drawlbut 5,83,70,15,'Сохр. как',102,[sc.grab_button],[sc.grab_button_text]
drawlbut 5,98,70,15,'Открыть',103,[sc.grab_button],[sc.grab_button_text]
drawlbut 5,113,70,15,'Выход',104,[sc.grab_button],[sc.grab_button_text]
else
drawlbut 5,38,70,15,'New window',100,[sc.grab_button],[sc.grab_button_text]
drawlbut 5,53,70,15,'New',101,[sc.grab_button],[sc.grab_button_text]
drawlbut 5,68,70,15,'Save',2,[sc.grab_button],[sc.grab_button_text]
drawlbut 5,83,70,15,'Save as',102,[sc.grab_button],[sc.grab_button_text]
drawlbut 5,98,70,15,'Open',103,[sc.grab_button],[sc.grab_button_text]
drawlbut 5,113,70,15,'Exit',104,[sc.grab_button],[sc.grab_button_text]
end if
jmp still

draw_win_menu_code:
call clear_screen
call drawwindow
if lang eq ru
drawlbut 75,38,70,15,'Компил.',10000,[sc.grab_button],[sc.grab_button_text]
drawlbut 75,53,70,15,'Запустить',10001,[sc.grab_button],[sc.grab_button_text]
drawlbut 75,68,70,15,'Доска отл.',10002,[sc.grab_button],[sc.grab_button_text]
drawlbut 75,83,70,15,'SysFunc',10003,[sc.grab_button],[sc.grab_button_text]
else
drawlbut 75,38,70,15,'Compile',10000,[sc.grab_button],[sc.grab_button_text]
drawlbut 75,53,70,15,'Run',10001,[sc.grab_button],[sc.grab_button_text]
drawlbut 75,68,70,15,'Debug board',10002,[sc.grab_button],[sc.grab_button_text]
drawlbut 75,83,70,15,'SysFunc',10003,[sc.grab_button],[sc.grab_button_text]
end if
mov [menu_is_on],1
jmp still

draw_win_menu_text:
call clear_screen
call drawwindow
if lang eq ru
drawlbut 145,38,70,15,'GoTo Line#',95,[sc.grab_button],[sc.grab_button_text]
drawlbut 145,53,70,15,'Найти',92,[sc.grab_button],[sc.grab_button_text]
drawlbut 145,68,70,15,'Найти далее',50,[sc.grab_button],[sc.grab_button_text]
else
drawlbut 145,38,70,15,'GoTo Line#',95,[sc.grab_button],[sc.grab_button_text]
drawlbut 145,53,70,15,'Find',92,[sc.grab_button],[sc.grab_button_text]
drawlbut 145,68,70,15,'Find next',50,[sc.grab_button],[sc.grab_button_text]
end if
mov [menu_is_on],1
jmp still

new_pad_wnd:
mov eax,19
mov ebx,tinypad_filename
mov ecx,0
int 0x40
jmp still

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
if lang eq ru
openerrtext db 'Ошибка открытия файла или открытие отменено!',0
saveerrtext db 'Ошибка сохранения файла или сохранение отменено!',0
newfileerror db 'Невозможно создать новый файл',0
else
openerrtext db 'Error while opening file or opening canceled!',0
saveerrtext db 'Error while saving file or saving canceled!',0
newfileerror db 'Cannot create new file',0
end if
; ********************
; ******  DATA  ******
; ********************
sz fasm_filename,    'FASM        '
sz debug_filename,   'BOARD       '
sz tinypad_filename, 'TINYPAD     '

lsz sysfuncs_filename,\
  ru, <'SYSFUNCR.TXT',0>,\
  en, <'SYSFUNCS.TXT',0>

sz setup, 'SETUP      '   ; we need to run SETUP to change keyboard layout
param_setup db 'LANG',0   ; parameter for SETUP


addr            dd   filename  ; address of input string

filename_len    dd   12
filename        db   'NEWDOC00.TXT'
times 256 db 0;51-12    db   0
;times  db 0   ; я не знаю почему Вилл не поставил эту строчку
orig_filename   db   'NEWDOC00.TXT'
rb 256
search_len      dd   5
search_string   db   'still'
times 51-5      db   0

case_sens       db   0    ; for search function

align 4
comment_string:
db ';***************************************'
db '*************************************** '



; INTERFACE DATA:


sz htext, 'TINYPAD'
;sz toolbar_btn_text,  'COMPILE    RUN     BOARD   SYSFUNC'


searcht:
if lang eq ru
    db  ' СТРОКА >                              '
    db  '                               ПОИСК   '
else
    db  ' STRING >                              '
    db  '                             SEARCH    '
end if
searchtl:


htext2:
if lang eq ru
    db  ' ПОЗИЦИЯ 00:00000    ДЛИНА 00000 СТРОК '
else
    db  'POSITION 00:00000   LENGTH 00000 LINES '
end if
htextlen2:

help_text:
if lang eq ru
    db  '                 КОМАНДЫ:                          '
    db  '                                                   '
    db  '  CTRL+F1  : Это окно                              '
    db  '  CTRL+S   : Первая строка для копирования         '
    db  '  CTRL+E   : Последняя строка для копирования      '
    db  '  CTRL+P   : Вставить выбранное на текущую позицию '
    db  '  CTRL+D   : Удалить строку                        '
    db  '  CTRL+L   : Вставить строку-разделитель           '
    db  '  CTRL+[   : Перейти в начало файла                '
    db  '  CTRL+]   : Перейти в конец файла                 '
    db  '  CTRL+F2  : Загрузить файл                        '
    db  '  CTRL+F3  : Поиск                                 '
    db  '  CTRL+F4  : Сохранить файл                        '
    db  '  CTRL+F5  : Сохранить файл как...                 '
    db  '  CTRL+F6  : Ввести строку для поиска              '
    db  '  CTRL+F7  : "Окно быстрого сохранения и загрузки" '
    db  '  CTRL+F8  : Сменить раскладку клавиатуры          '
    db  '  CTRL+F9  : Компилировать файл                    '
    db  '  CTRL+F10 : Компилировать и запустить             '
    db  '  F12      : Запустить доску отладки               '
    db  'x'
else
    db  '                COMMANDS:                          '
    db  '                                                   '
    db  '  CTRL+F1  : SHOW THIS WINDOW                      '
    db  '  CTRL+S   : SELECT FIRST STRING TO COPY           '
    db  '  CTRL+E   : SELECT LAST STRING TO COPY            '
    db  '  CTRL+P   : PASTE SELECTED TO CURRENT POSITION    '
    db  '  CTRL+D   : DELETE CURRENT LINE                   '
    db  '  CTRL+L   : INSERT SEPARATOR LINE                 '
    db  '  CTRL+[   : GO TO THE BEGINNING OF FILE           '
    db  '  CTRL+]   : GO TO THE END OF FILE                 '
    db  '  CTRL+F2  : LOAD FILE                             '
    db  '  CTRL+F3  : SEARCH                                '
    db  '  CTRL+F4  : SAVE FILE                             '
    db  '  CTRL+F5  : SAVE FILE AS...                       '
    db  '  CTRL+F6  : ENTER SEARCH STRING                   '
    db  '  CTRL+F7  : DRAW "FAST SAVE AND OPEN" WINDOW      '
    db  '  CTRL+F8  : CHANGE KEYBOARD LAYOUT                '
    db  '  CTRL+F9  : COMPILE FILE                          '
    db  '  CTRL+F10 : RUN OUTFILE                           '
    db  '  F12      : OPEN DEBUG BOARD                      '
    db  'x'
end if

help_wnd_header:
if lang eq ru
    db  'ПОМОЩЬ',0
else
    db  'TINYPAD HELP',0
end if

hlphead_len = $ - help_wnd_header

save_yes_no:
if lang eq ru
    db 'Сохранить файл перед выходом?',0
else
    db 'Save file before exit?',0
end if

VScroll_1:
   dw  13    ;width         +0
   dw  489   ;x             +2
   dw  341 ;326   ;height        +4
   dw  43    ;y             +6
   dd  0     ;min           +8
   dd  100 ;scrl_max ;lines   ;max           +12
   dd  0 ;cur_pos          ;current       +16
   dd  1     ;small change  +20
   dd  10    ;big change    +24

App_Controls:
     dd VScroll_1 , drawvscroll, VScroll_mouse_over   ;
     dd 0,0,0                                       ;last control do not delete

; END OF INTERFACE DATA

symbols db '%#&*\:/<>|{}()[]=+-,. '

error_beep:
    db  0xA0,0x30,0
save_beep1:
    db  0x84,0x43,0
save_beep2:
    db  0x84,0x48,0


align 4
fileinfo_read:
    dd  0
    dd  0
    dd  300000/512
    dd  0x10000
    dd  0x70000
pathfile_read:
    times 250 db 0 ;51 db 0

align 4
fileinfo_write:
    dd  1
    dd  0
    dd  0
    dd  0x10000
    dd  0x70000
pathfile_write:
    times 250 db 0 ; 51 db 0

align 4

temp     dd 0xabc ; used in read_string
d_status dd 0

color_tbl:
   dd 0x00000000 ; text
   dd 0x00000000 ; instruction
   dd 0x00000000 ; register
   dd 0x00009000 ; number
   dd 0x00a00000 ; quote
   dd 0x00909090 ; comment
   dd 0x003030f0 ; symbol


next_not_quote2 db 0  ; "
next_not_quote  db 0  ; '
quote           db 0
asm_mode        db 0  ; ASM highlight?
tmpabc          db 0


I_PARAM  db 0    ; parameters are HERE - параметры будут начинаться ЗДЕСЬ!
TINYPAD_END:     ; end of file


; Uninitialised data
; Неинициализированные данные
align 4

posx   dd ?      ; x на экране (on the screen)
posy   dd ?      ; y на экране
post   dd ?      ; смещение от начала - offset
posl   dd ?
lines  dd ?      ; количество строк в документе
posxm  dd ?
posym  dd ?

dstart dd ?      ; смещение по оси y для отрисовки кнопок и др.

filelen     dd ? ; длина файла

PID         dd ? ; идентификатор процесса
proccount   dd ? ; количество процессов

filesize dd ?    ; размер файла
ya       dd ?    ; для read_string
slines   dd ?    ; number of strings on the screen - количество строк на экране

run_outfile dd ? ; for FASM
copy_start  dd ? ; Ctrl+S
copy_count  dd ? ; Ctrl+E
headlen     dd ? ; header length
do_not_draw dd ? ; to draw or not to draw - this is a serious question ;)

MainWndClosed dd ?
sc  system_colors

to_return db 0
to_return2 db 0
error_found db 0
error2_found db 0

header:          ; window header - заголовок окна
rb 256

; информация о процессе записывается в эту структуру
procinfo process_information

virtual at procinfo
fasm_parameters rb 256
end virtual
pos00 dd 0
newdoc db ?
mypath:
times 258 db 0
real_posy dd 0
vscroll_size dd 0
vscroll_pos dd 0
cur_pos dd 0
scrl_max dd 100

mouse_x dd 0
mouse_y dd 0
mousey dw 0
mousex dw 0
mouseb dd 0

num_goto_string dd 0

menu_is_on db 0
exit_wnd_on db 0
modified db 0
;fast save n open stack
rb 1024
so_stack:
;growing down
; the end!


