;constants
;for keys
KEY_ESC      equ 27
KEY_PGDN     equ 183
KEY_PGUP     equ 184
KEY_LNDN     equ 177
KEY_LNUP     equ 178
KEY_RIGHT    equ 179
KEY_LEFT     equ 176
KEY_HOME     equ 180
KEY_END      equ 181
KEY_HOMETOP  equ 251  ;Ctrl + '['
KEY_ENDBOTTOM equ 253 ;Ctrl + ']'

 use32
 org 0x0
 db 'MENUET01'  ; 8 byte id
 dd 0x01        ; header version
 dd START       ; start of code
 dd I_END       ; size of image
 dd 0x80000     ; memory for app
 dd 0x80000     ; esp
 dd 0x0 , 0x0   ; I_Param , I_Icon

 include 'lang.inc'
 include 'macros.inc'


START: ; start of execution

  mov  eax,40
  mov  ebx,100111b ;event mouse
  int  0x40
  mov  dword [process_info+42],540
  mov  dword [process_info+46],414
  call draw_window

still:
  mov  eax,10  ; wait here for event
  int  0x40
  dec  al      ; redraw request ?
  je   red
  dec  al      ; key in buffer ?
  je   key
  dec  al      ; button in buffer ?
  je   button
               ; mouse event received
 mouse:
  mov  eax,37
  mov  ebx,2
  int  0x40
  or   eax,eax
  jz   still
  cmp  [menu_opened],1
  jne  still
  mov  [menu_opened],0

 red:          ; redraw
  call redraw_window
  jmp  still

 key:          ; key
  mov  eax,2
  int  0x40
; test al,al
; jnz  still
  cmp  ah,KEY_ESC
  jz   close
  mov  al,[o_s_flag]
  and  al,8  ;set bit 3?
  jz   edit_keys ;not - no output to filename area
  cmp  ah,8  ;BACKSPACE
  jnz  no_backspace
  dec  [name_l]
  cmp  [name_l],0
  mov  edx,filename
  je   A1
  jg   @f
  mov  [name_l],0
  jmp  A1
 @@:
  add  dl,[name_l]
 A1:
  sub  [x_cursor],0x60000
  mov  [edx],byte 0
  cmp  [x_cursor],0xb0005
  jge  @f
  mov  [x_cursor],0xb0005
@@:
  call redraw_window
  jmp  still  ;END BACKSPACE

 no_backspace:
  cmp  ah,0x30 ;'0'
  jl   A2
  cmp  ah,0x39 ;'9'
  jle  bigsym
  cmp  ah,65 ;'A'
  jbe  bigsym
  cmp  ah,90 ;'Z'
  jle  bigsym
  cmp  ah,97 ;'a'
  jbe  smsym
  cmp  ah,122;'z'
  jle  smsym
  jmp  still
 A2:
  cmp  ah,46 ;'.'
  jz   bigsym
  cmp  ah,0x20
  jz   bigsym
  jmp  still
 smsym:
  sub  ah,32
 bigsym:
  cmp  [name_l],13 ;yes. filename length <13
  jl   @f
  sub  [o_s_flag],8 ;not - disable output to filename area
  mov  [x_cursor],0x680005; - set x-base & x-size
  jmp  _end_keys
 @@:
  mov  edx,filename  ;
  add  dl,[name_l]
  mov  [edx],ah
  inc  [name_l]
  add  [x_cursor],0x60000
 _end_keys:
  call redraw_window
  jmp  still
 edit_keys:
  cmp  ah,KEY_PGDN
  jnz  @f
  call PgDn
  jmp  still
 @@:
  cmp  ah,KEY_PGUP
  jnz  @f
  call PgUp
  jmp  still
 @@:
  cmp  ah,KEY_HOME ;Home
  jnz  @f
  call Home
  jmp  still
 @@:
  cmp  ah,KEY_END ;Home
  jnz  @f
  call _End
  jmp  still
 @@:
  cmp  ah,KEY_HOMETOP
  jnz  @f
  call CtrlHome
  jmp  still
 @@:
  cmp  ah,KEY_ENDBOTTOM
  jnz  @f
  call CtrlEnd
  jmp  still
 @@:
  cmp  ah,KEY_LNDN
  jnz  @f
  call LnDn
  jmp  still
 @@:
  cmp  ah,KEY_LNUP
  jnz  @f
  call LnUp
  jmp  still
 @@:
  cmp  ah,KEY_RIGHT ;Right
  jnz  @f
  call Right
  jmp  still
 @@:
  cmp  ah,KEY_LEFT ;Left
  jnz  @f
  call Left
 @@:
  ;редактирование строки в hex-представлении
  mov  esi,[current]
  mov  bl,[posx]
  add  esi,0x10000 ;0x80000
  cmp  ah,0x30
  jl   still   ;ah<'0'
  cmp  ah,0x39
  jle  A23   ;ah='0'...'9' - перевод из символов в hex
;проверка на коды старших hex-цифр
  cmp  ah,0x61 ;ah<'a'
  jl   A27   ;может быть вводятся большие буквы?
  cmp  ah,0x66 ;ah>'f'
  jg   still
  sub  ah,0x20 ;конвертируем в большие буквы
  jmp  A24
 A27:
  cmp  ah,0x41
  jl   still   ;ah<'A'
  cmp  ah,0x46
  jg   still   ;ah>'F'
 A24:
  add  ah,9
 A23:
  mov  dl,[esi];оригинальный байт
  and  bl,1    ;если нечет - редактируется младший полубайт, чет - старший
  jz   hi_half_byte
  ;младший полубайт
  and  ah,0x0f ;обнуляем старший полубайт введенной цифры
 ;если ah = 0x30...0x39, то все нормально
 ;если ah = 0x41...0x46, то на метке А24 получим
 ;ah = 0x4A...0x4F и тоже все нормально
  and  dl,0xf0 ;обнуляем младший полубайт у оригинального байта
  jmp  patch_byte
 hi_half_byte:
  ;старший полубайт
  shl  ah,4    ;одновременно сдвигаем нужное значение в старший полубайт
 ;и обнуляем младший
  and  dl,0x0f ;обнуляем старший полубайт у оригинального байта
 patch_byte:
  or   ah,dl   ;объединяем полубайты
  mov  [esi],ah;патчим в памяти
  mov  ebx,0x20100
  movzx ecx,ah ;для функции вывода числа
  ;теперь надо расчитать координаты вывода для числа
  ;edx = x shl 16 + y
  mov  edx,[x_cursor]
  mov  edi,[y_cursor]
  and  edx,0xffff0000
  shr  edi,0x10
  xor  esi,esi
  or   edx,edi
  mov  eax,47
  add  edx,8
  int  0x40
  call redraw_window
  jmp  still

 button:   ; button
  mov  eax,17 ; get id
  int  0x40
  dec  ah  ;close programm button
  jne  @f
 close:
  mov  eax,-1 ; close this program
  int  0x40
 @@:
  dec  ah
  jne  @f
  call redraw_window
  call menufile
  jmp  still
 @@:
  dec  ah  ;menu 'coding' button
  jne  @f
  call redraw_window
  call menucoding
  jmp  still
 @@:
  dec  ah  ;menu 'Help' button
  jne  @f
  call redraw_window
  call menuhelp
  jmp  still
 @@:
  ;now processed low_level menu buttons
  ;id m_open = 5
  ;id m_save = 6
  ;id m_exit = 7
  ;id m_win2dos 8
  ;id m_win2koi 9
  ;id m_win2iso 10
  ;id m_dos2win 11
  ;id m_dos2koi 12
  ;id m_dos2iso 13
  ;id m_help 14
  ;id m_about 15
  dec  ah ;open?
  jne  @f
  cmp  [o_s_flag],0  ;disable 'Open' if flag !=0
  jnz  no_open
;  call redraw_window
  mov  [color],0x0072b9fc
  call f2 ;set x_cursor & y_cursor for input filename
  mov  [o_s_flag],8   ;for output to filename area
 no_open:
  call redraw_window
  jmp  still
 @@:
  dec  ah ;save?
  jne  @f
  cmp  [o_s_flag],1  ;enable save if flag = 1
  jnz  no_save
  movzx ecx,[name_l] ;begin clear filename string
  mov  edi,filename
  xor  al,al
  jecxz no_clear
 clear:
  mov  [edi+ecx],al
  loop clear
  mov  [name_l],al ;set filename length = 0
 no_clear:  ;ebd clear
  call f2 ;set x_cursor & y_cursor for input filename
  mov  [o_s_flag],9 ;for output to filename area
 no_save:
  call redraw_window
  jmp  still
 @@:
  dec  ah ;exit?
  jne  @f
  jmp  close
 @@:
  dec  ah ;m_win2dos?
  jne  @f
  push dword WIN_TABLE
  push dword DOS_TABLE
  call coding
  call redraw_window
  jmp  still
 @@:
  dec  ah ;m_win2koi?
  jne  @f
  push WIN_TABLE
  push KOI_TABLE
  call coding
  call redraw_window
  jmp  still
 @@:
  dec  ah ;m_win2iso?
  jne  @f
  push WIN_TABLE
  push ISO_TABLE
  call coding
  call redraw_window
  jmp  still
 @@:
  dec  ah ;m_dos2win?
  jne  @f
  push DOS_TABLE
  push WIN_TABLE
  call coding
  call redraw_window
  jmp  still
 @@:
  dec  ah ;m_dos2koi?
  jne  @f
  push DOS_TABLE
  push KOI_TABLE
  call coding
  call redraw_window
  jmp  still
 @@:
  dec  ah ;dos2iso?
  jne  @f
  push DOS_TABLE
  push ISO_TABLE
  call coding
  call redraw_window
  jmp  still
 @@:
  dec  ah ;m_help?
  jne  @f
  ;create new thread for output help info
  ;parameter: address for entry_point thread
  push help_thread
  call create_process
  call redraw_window
  jmp  still
 @@:
  dec  ah ;m_about?
  jne  @f
  ;create new thread for output about info
  ;parameter: address for entry_point thread
  push about_thread
  call create_process
  call redraw_window
  jmp  still
 @@:
  ;button 'Go'
  and  [o_s_flag],1
  jnz  _savefile
  ;open file
  mov  eax,6
  mov  ebx,filename
  xor  ecx,ecx
  or   edx,-1
  mov  esi,0x10000
  int  0x40
  inc  [o_s_flag]
  mov  [sizefile],eax
  jmp  end_Go
 _savefile:
  ;save file
  mov  ebx,filename
  mov  ecx,0x10000
  mov  edx,[sizefile]
  xor  esi,esi
  dec  edx
  mov  eax,33
  int  0x40
 end_Go:
  call CtrlHome
  jmp  still

Right:
  pushad
  mov  al,[posx]
  inc  al
  cmp  al,0x20
  jl   @f
  mov  [posx],0
  mov  [x_cursor],0x680005
  mov  [text_cursor],0x01200000
  sub  [current],0xf ;because [current] add 0x10 in LnDn
  call LnDn
  popad
  ret
 @@:
  mov  [posx],al
  and  al,1
  jnz  @f ;not increment [current]
  ;increment
  add  [x_cursor],0xa0000
  sub  [text_cursor],0x40000
  inc  [current]
  jmp  end_r
 @@:
  add  [x_cursor],0x60000
  sub  [text_cursor],0x60000
 end_r:
  call redraw_window
  popad
  ret

Left:
  pushad
  mov  al,[posx]
  dec  al
  jge  @f
  mov  [posx],0x1f
  mov  [x_cursor],0x015e0005
  mov  [text_cursor],0x00840000
  add  [current],0x0f
  call LnUp
  popad
  ret
 @@:
  mov  [posx],al
  and  al,1
  jnz @f ;decrement [current]
  ;not decrement
  sub  [x_cursor],0x60000
  add  [text_cursor],0x60000
  jmp  end_l
 @@:
  cmp  [current],0
  jle  end_l
  sub  [x_cursor],0xa0000
  add  [text_cursor],0x40000
  dec  [current]
 end_l:
  call redraw_window
  popad
  ret

LnDn:
  pushad
  add  [current],0x10
  movzx ecx,[lines]
  cmp  cl,[posy]
  jl   @f ;when counter strings >= number strings in window
  add  [y_cursor],0xa0000
  inc  [posy]
  call redraw_window
  popad
  ret
 @@:
  mov  eax,0x10
  xor  edx,edx
  imul ecx
  sub  eax,0x10
  sub  [end_str],eax
;  mov  eax,[sizefile]
;  add  eax,0x80000
;  cmp  eax,[end_str]
;  jge  @f
;  mov  [end_str],eax
; @@:
  call draw_window
  popad
  ret

LnUp:
  pushad
  sub  [current],0x10
  cmp  [current],0
  jge  @f
  mov  [current],0
 @@:
  cmp  [posy],3
  jle @f ;when counter strings < number top string
  sub  [y_cursor],0xa0000
  dec  [posy]
  call redraw_window
  popad
  ret
 @@:
;  movzx ecx,[lines]
;  mov  eax,0x10
;  xor  edx,edx
;  imul ecx
;  add  eax,0x10
  sub  [end_str],0x10
  cmp  [end_str],0x10000
  jge  @f
  mov  [end_str],0x10000
 @@:
  call redraw_window
  popad
  ret

CtrlEnd:
  pushad
  popad
  ret

CtrlHome:
  pushad
  mov  [x_cursor],0x00680005   ;устанавливаются значения, как при открытии
  mov  [y_cursor],0x00280008
  mov  [text_cursor],0x01200000
  mov  [posx],0
  mov  [posy],3
  call b_in_screen
  mov  [end_str],0x10000
  mov  [current],0
  call redraw_window
  popad
  ret

_End:
  pushad
  mov  [x_cursor],0x015e0005
  mov  [posx],0x1f
  mov  [text_cursor],0x00840000
  or   [current],0xf
  call b_in_screen
  call redraw_window
  popad
  ret

Home:
  pushad
  mov  [x_cursor],0x00680005 ;устанавливаются значения для начала строки
  mov  [posx],0
  mov  [text_cursor],0x01200000
  and  [current],0xfffffff0
  call b_in_screen
  call redraw_window
  popad
  ret

PgDn:
  pushad
  xor  edx,edx
  movzx ecx,[lines]
  mov  eax,0x10
  imul ecx
  add  [current],eax
  add  [end_str],eax
  call redraw_window
  popad
  ret

PgUp:
  pushad
  xor  edx,edx
  movzx ecx,[lines]
  mov  eax,0x10
  imul ecx
  sub  [current],eax
  cmp  [current],0
  jge  @f
  mov  [current],0
 @@:
  sub  [end_str],eax
  cmp  [end_str],0x10000
  jge  @f
;  call CtrlHome
  mov  [end_str],0x10000
 @@:
  call redraw_window
  popad
  ret

b_in_screen:
  pushad
  call get_process_info
  mov  eax,[process_info+0x2e]
  mov  ebx,0x0a
  sub  eax,0x3c
  cmp  eax,0x10   ;now
  jg   @f   ;now
  mov  [lines],0  ;now
  jmp  C1   ;now
 @@:
  xor  edx,edx
  div  ebx
  mov  [lines],al
 C1:
  popad
  ret



output_screen:
  pushad
  movzx ecx,[lines]
  jecxz no_out ;now
  cmp  [rflag],1
  jz   _redraw
  mov  eax,[end_str]
  sub  eax,0x80001
  cmp  eax,[sizefile]
  jl   @f
 _redraw:
  xor  edx,edx
  mov  eax,0x10
  imul ecx
  sub  [end_str],eax
  cmp  [end_str],0x10000
  jge  A3
  mov  [end_str],0x10000
 A3:
  mov  [rflag],0
 @@:
  mov  eax,0x28
 @@:
  push ecx
  push eax
  call form_str
  mov  ebx,0x01880000
  add  ebx,eax
  mov  ecx,0x00ffffff
  add  eax,10
  mov  edx,[end_str]
  push eax
  sub  edx,0x10
  mov  esi,0x10
  mov  eax,4
  int  0x40
  pop  eax
  pop  ecx
  loop @b
 no_out:
  popad
  ret



form_str:
  pushad
  mov  ebp,[end_str]  ;последний выведенный байт
  xor  edi,edi ;счетчик байт <= 16
  ;вывести адрес
  mov  ecx,ebp
  mov  ebx,0x80100  ;8 цифр, 16-ричные, число в ecx
  sub  ecx,0x10000  ;нормализация адреса
  mov  edx,0x80000  ;начало по Х
  add  edx,[esp+0x24]  ;начало по У
  mov  esi,0x00ffffff ;цвет
  mov  eax,47 ;вывести число
  int  0x40
  add  edx,0x600000   ;0x40 - 8chars + 0x20 - space
  mov  ebx,0x20100    ;2 цифры, 16-ричные, число в ecx
 @@:
  mov  ecx,[ebp+edi]   ;число в ecx
  inc  edi
  and  ecx,0xff
  cmp  edi,0x11
  jz   endstr
  int  0x40
  add  edx,0x100000
  jmp  @b
 endstr:
  dec  edi
  add  ebp,edi
  mov  [end_str],ebp
  popad
  ret  4



draw_cursor:
  pushad
  mov  ebx,[x_cursor]
  mov  ecx,[esp+0x24]
  mov  edx,[color]
  mov  eax,13
  int  0x40
  movzx edi,[o_s_flag]
  and  edi,8
  jnz  @f
  add  ebx,[text_cursor]
  int  0x40
 @@:
  popad
  ret  4

f2:
  mov  eax,[process_info+46]
  mov  [x_cursor],0xb0005
  sub  eax,0x11
;  mov  [text_cursor],-1
  shl  eax,0x10

  mov  [y_filename_area],eax
  ret

menufile:
  mov  [menu_opened],1
  mov  ebx,[xf_menu];x-base+x_size for hi-level menu button
  mov  edx,5;first id button for this group
  mov  edi,3;counter buttons
  call f1
  ;output text for menu
  shr  ecx,0x10  ;y-base button
  and  ebx,0xffff0000
  add  ecx,0x6000f ;for y-base text
  mov  esi,4 ;length text
  add  ebx,ecx ;full base text
  mov  edx,m_open
  mov  ecx,[sc.work_button_text]
  or   ecx,0x10000000
  sub  eax,4
  int  0x40
  add  ebx,0x0c ;next full base text
  add  edx,4 ;next string
  int  0x40
  add  ebx,0x0c
  add  edx,4
  int  0x40
  ret

menucoding:
  mov  [menu_opened],1
  mov  ebx,[xe_menu]
  mov  edx,8 ;first id
  mov  edi,6 ;counter
  add  ebx,0x10 ;add width buttons
  push edi
  call f1
  pop  edi
  shr  ecx,0x10
  and  ebx,0xffff0000
  add  ecx,0x8000f
  mov  esi,8 ;length text
  add  ebx,ecx
  mov  edx,m_win2dos
  mov  ecx,[sc.work_button_text]
  or   ecx,0x10000000
  sub  eax,4
 @@:
  int  0x40
  add  ebx,0x0c
  add  edx,8 ;next string
  dec  edi
  jnz  @b
  ret

menuhelp:
  mov  [menu_opened],1
  mov  ebx,[xh_menu]
  mov  edx,14 ;first id
  add  ebx,6 ;add width buttons
  mov  edi,2 ;counter
  call f1
  shr  ecx,0x10
  and  ebx,0xffff0000
  add  ecx,0x8000f
  mov  esi,4 ;length text
  add  ebx,ecx
  mov  edx,m_help
  mov  ecx,[sc.work_button_text]
  or   ecx,0x10000000
  sub  eax,4
  int  0x40
  add  ebx,0x0c
  inc  esi ;add lebgth output text
  add  edx,4
  int  0x40
  ret

f1:;uses for drawing low-level menu buttons
  ;counter buttons get into
  ;menufile,menucoding,menuhelp funcs.
  mov  ecx,[y_menu]         ;y-base+y_size for hi-level menu button
  mov  esi,[sc.work_button] ;color buttons
  mov  eax,8
  push ecx                  ;for output text
 @@:
  add  ecx,0xc0000
  int  0x40
  inc  edx ;id
  dec  edi ;counter
  jnz  @b
  pop  ecx ;for output text
  ret

redraw_window:
  call get_process_info
  mov  [rflag],1
  call draw_window
  ret

;this is debug func
debug:
  pushad
  mov  ecx,[esp+0x24]
  mov  ebx,0x80100
  mov  edx,0x10000a0
  mov  eax,47
  mov  esi,0x00ffffff
  int  0x40
  popad
  ret  4

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************

draw_window:
  pushad
  mov  eax,48
  mov  ebx,3
  mov  ecx,sc
  mov  edx,sizeof.system_colors
  int  0x40

  mov  eax,12                 ; function 12:tell os about windowdraw
  mov  ebx,1                  ; 1, start of draw
  int  0x40
     ; DRAW WINDOW
  mov  eax,0                  ; function 0 : define and draw window
                              ; [x start] *65536 + [x size]
  mov  ebx,[process_info+42]
                              ; [y start] *65536 + [y size]
  mov  ecx,[process_info+46]
  mov  edx,0x03000000         ; color of work area RRGGBB,8->color gl
  int  0x40
     ; WINDOW LABEL
  mov  eax,4                  ; function 4 : write text to window
  mov  ebx,8*65536+8          ; [x start] *65536 + [y start]
  mov  ecx,[sc.grab_text]
  or   ecx,0x10000000         ; font 1 & color ( 0xF0RRGGBB )
  mov  edx,labelt             ; pointer to text beginning
  mov  esi,labellen-labelt    ; text length
  int  0x40
    ;check for only header window output
  cmp  dword [process_info+46],25
  jle  minimaze_view

     ;MENU AREA
  mov  eax,[process_info+42] ;x-size window
  mov  ecx,[process_info+46] ;y-size window
  push ecx ;for using done
  mov  ebx,0x40000
  sub  eax,8
  mov  edi,ecx
  add  ebx,eax ;x-base + x-size
  sub  edi,22 ;temporary value for menu area
  push ebx ;for drawing buttons area
  sub  ecx,edi ;y-base menu area
  mov  edx,[sc.work_graph]
  shl  ecx,0x10
  mov  eax,13
  add  ecx,0x10
  int  0x40
     ;MENU BUTTONS
  ;now in hi-half ecx register begin Y-coord. menu area
  ;in hi-half ebx begin X-coord.
  ;menu 'File'
  mov  esi,edx ;color
  and  ecx,0xffff0000
  and  ebx,0xffff0000
  add  ecx,0x1000c
  add  ebx,0x20028   ;40x12
  mov  edx,2 ;menu 'File' id = 2
  mov  [y_menu],ecx ;for low-level menus func.
  mov  [xf_menu],ebx;for low-level menus func.
  mov  eax,8
  push ebx ;for output buttons texts
  int  0x40
  ;registers is't change
  ;menu 'Coding'
  add  ebx,0x290018 ;80x12
  inc  edx ;menu 'coding' id = 3
  mov  [xe_menu],ebx;for low-level menus func.
  int  0x40
  ;menu 'Help'
  add  ebx,0x40ffe8 ;+0x280000 - 0x28, 40x12
  inc  edx ;menu 'Help' id = 4
  mov  [xh_menu],ebx;for low-level menus func.
  int  0x40
     ;MENU BUTTONS TEXTS
  ;'File'
  pop  ebx
  shr  ecx,0x10
  and  ebx,0xffff0000
  add  ecx,3
  mov  eax,4                     ; function 4 : write text to window
  add  ebx,0x80000
  mov  edx,f_menu
  add  ebx,ecx                   ; [x start] *65536 + [y start]
  mov  esi,4
  mov  ecx,[sc.work_button_text]
  or   ecx,0x10000000            ; font 1 & color ( 0xF0RRGGBB )
  push esi                       ;for 'Help' menu text
  int  0x40
  ;'coding'
  ;registers is't change
  add  ebx,0x2d0000
  ;mov  esi,6
  add  esi,2
;  mov  edx,e_menu
  add  edx,4
  int  0x40
  ;'Help'
  add  ebx,0x3b0000
;   mov  esi,4
  pop  esi
;  mov  edx,h_menu
  add  edx,6
  int  0x40
 ;LOW_LEVEL MENU
  ;for every hi-level menu exists one procedure
  ;in begin programm they are not calls,
  ;but when user click on one item hi-level menu
  ;or press hot keys, call one func. and after
  ;end work this func. she is redraw window -
  ;low-level menu is hide. Functions:
  ;menufile,menucoding,menuhelp.
  ;Thay uses global virables, top-left corner every
  ;hi-level menu buttons: [xf_menu],[xe_menu],[xh_menu],[y_menu]

     ;DRAW BUTTONS AREA
  pop  ebx ;for push ebx into processed menu area: x-bzse + x-size
;  mov  ecx,[process_info+46]
  pop  ecx
  push ecx
  sub  ecx,24
  mov  edx,[sc.work_graph]
  shl  ecx,16              ;y start
  mov  eax,13
  add  ecx,20
  int  0x40

;filename input area
;  mov  ecx,[process_info+46]
  pop  ecx
  push ecx ;for info strings
  mov  ebx,0x0008005a
  sub  ecx,21
  xor  edx,edx
  shl  ecx,16
  mov  [y_filename_area],ecx
  dec  edx
  add  ecx,16
  mov  eax,13
  push ecx ;for button 'Go'
  int  0x40

;button 'Go', press in case open/save if filename input complete
  ;button size = 24x16
  mov  eax,8
  pop  ecx ;y-base+y-size
  mov  ebx,0x00680018;x-base+x-size
  dec  ecx
  mov  edx,0xff ;id
  mov  esi,[sc.work_button]
  int  0x40
  shr  ecx,0x10
  and  ebx,0xffff0000
  add  ecx,0x50004
  mov  edx,b_go
  add  ebx,ecx
  mov  esi,2
  mov  ecx,[sc.work_button_text]
  or   ecx,0x10000000
  sub  eax,4
  int  0x40

;where output cursor?
  mov  al,[o_s_flag]
  and  al,8
  je   @f
  mov  ecx,[y_filename_area]
  add  ecx,0x40008
  jmp  cursor
 @@:  ;o_s_flag<0 - not output cursor into filename area
  mov  ecx,[y_cursor]
 cursor:
  push ecx
  call draw_cursor

  mov  eax,[y_filename_area]
  mov  ebx,0xa0000
  mov  edx,filename
  shr  eax,0x10
  and  ebx,0xffff0000
  add  eax,4
  xor  ecx,ecx
  add  ebx,eax
  movzx esi,[name_l]
  mov  eax,4
  int  0x40

;info strings
     ; sizefile text
;    mov  eax,[process_info+46]
  pop  eax
  mov  ebx,0x00840000
  sub  eax,18
  xor  ecx,ecx
  add  ebx,eax
  mov  edx,sizestr   ; pointer to text beginning
  mov  eax,4
  mov  esi,5
  int  0x40
  add  ebx,0x00530000
  inc  esi
;    mov  edx,offst
  add  edx,5
  inc  esi
  int  0x40
    ;sizefile
  mov  ecx,[sizefile]
  mov  edx,ebx
  xor  esi,esi
  sub  edx,0x00350000
  mov  eax,47
  mov  ebx,0x80100
  int  0x40
  mov  ecx,[current]
  add  edx,0x005f0000
  int  0x40

  push [text_cursor] ;это позиция курсора в текстовой строке
  call draw_cursor
  mov  ecx,[sizefile]
  jecxz minimaze_view
  call output_screen

 minimaze_view:
  mov  eax,12  ; function 12:tell os about windowdraw
  mov  ebx,2 ; 2, end of draw
  int  0x40
  popad
  ret





get_process_info:
  pushad
  mov  eax,9
  mov  ebx,process_info
  xor  ecx,ecx
  dec  ecx
  int  0x40
  popad
  ret

coding:
  pushad
  mov  ebp,0x10000 ;0x80000
  mov  edi,[esp+0x28] ;source table
  mov  esi,[esp+0x24] ;destination table
  xor  ecx,ecx ;index in file
 new_char:
  xor  ebx,ebx ;index in tables
 not_c:
  mov  ah,[ebp+ecx] ;load char
  cmp  ah,[edi+ebx] ;
  jz   @f
  inc  ebx
  cmp  ebx,0x40
  jge  end_table
  jmp  not_c
 @@:
  mov  al,[esi+ebx]
  inc  ebx
  mov  [ebp+ecx],al
 end_table:
  inc  ecx
  cmp  ecx,[sizefile]
  jle  new_char
  popad
  ret  8

create_process:
  pushad
  mov  eax,51
  xor  ebx,ebx
  mov  ecx,[esp+0x24]
  inc  ebx
  mov  edx,0x7E000 ;0x1000
  int  0x40
  popad
  ret  4

help_thread:
  call help_window
 help_still:
  mov  eax,10
  int  0x40
  dec  eax
  jz   help_red
  dec  eax
  jz   help_key
  dec  eax
  jz   help_button
  jmp  help_still
 help_red:
  call help_window
  jmp  help_still
 help_key:
  inc  eax
  inc  eax
  int  0x40
  jmp  help_still
 help_button:
  mov  eax,17
  int  0x40
  dec  ah
  jne  help_still
  shr  eax,8
  dec  eax
  int  0x40

help_window:
  pushad
  mov  eax,12  ; function 12:tell os about windowdraw
  mov  ebx,1 ; 1, start of draw
  int  0x40
     ; DRAW WINDOW
  mov  eax,0 ; function 0 : define and draw window
  mov  ebx,0x500140 ; [x start] *65536 + [x size]
  mov  ecx,0x700110 ; [y start] *65536 + [y size]
  mov  edx,0x03000000 ; color of work area RRGGBB,8->color gl
  int  0x40
     ; WINDOW LABEL
  mov  eax,4 ; function 4 : write text to window
  mov  ebx,8*65536+8 ; [x start] *65536 + [y start]
  mov  ecx,0x10ffffff ; font 1 & color ( 0xF0RRGGBB )
  mov  edx,help_label ; pointer to text beginning
  mov  esi,14 ; text length
  int  0x40
     ; HELP TEXT
  add  edx,14 ;help_text addr.
  add  esi,37 ; = 51 - length 1 line
  mov  ecx,0x00ffffff
  mov  edi,15
 @@:
  add  ebx,0x10
  int  0x40
  add  edx,51
  dec  edi
  jnz  @b


  mov  eax,12  ; function 12:tell os about windowdraw
  mov  ebx,2 ; 2, end of draw
  int  0x40
  popad
  ret

about_thread:
  call about_window
 about_still:
  mov  eax,10
  int  0x40
  dec  eax
  jz   about_red
  dec  eax
  jz   about_key
  dec  eax
  jz   about_button
  jmp  about_still
 about_red:
  call about_window
  jmp  about_still
 about_key:
  inc  eax
  inc  eax
  int  0x40
  jmp  about_still
 about_button:
  mov  eax,17
  int  0x40
  dec  ah
  jne  about_still
  shr  eax,8
  dec  eax
  int  0x40

about_window:
  pushad
  mov  eax,12  ; function 12:tell os about windowdraw
  mov  ebx,1 ; 1, start of draw
  int  0x40
     ; DRAW WINDOW
  mov  eax,0           ; function 0 : define and draw window
  mov  ebx,0x500140    ; [x start] *65536 + [x size]
  mov  ecx,0x700110    ; [y start] *65536 + [y size]
  mov  edx,0x03000000  ; color of work area RRGGBB,8->color gl
  int  0x40
     ; WINDOW LABEL
  mov  eax,4           ; function 4 : write text to window
  mov  ebx,8*65536+8   ; [x start] *65536 + [y start]
  mov  ecx,[sc.work_button_text]
  or   ecx,0x10000000  ; font 1 & color ( 0xF0RRGGBB )
  mov  edx,about_label ; pointer to text beginning
  mov  esi,17          ; text length
  int  0x40
     ; ABOUT TEXT
  add  edx,17 ;about_text addr.
  add  esi,34 ; = 51 - length 1 line
  mov  ecx,0x00ddeeff
  mov  edi,15
 @@:
  add  ebx,0x10
  int  0x40
  add  edx,51
  dec  edi
  jnz  @b

  mov  eax,12  ; function 12:tell os about windowdraw
  mov  ebx,2 ; 2, end of draw
  int  0x40
  popad
  ret

; DATA AREA
sizefile dd 0
current  dd 0 ;current offset relative begin file. Uses as offset for patch.
;Coordinates left hi-level menu buttons
;Uses into low-level menu output.
xf_menu  dd 0
xe_menu  dd 0
xh_menu  dd 0
y_menu dd 0 ;top coord. menu
y_filename_area dd 0 ;top coord. filename input area
color dd 0
y_cursor dd 0x280008 ;y coord. shl 16 + y size for cursor
x_cursor dd 0x680005 ;x coord. shl 16 + x size for cursor
name_l db 0 ;counter chars into filename
o_s_flag db 0 ;
rflag dd 0;
posx db 0
posy db 3
lines db 0
end_str dd 0x10000 ;addr. first byte for output
text_cursor dd 0x01200000

filename: rb 13

b_go: db 'Go'

sizestr: db 'SIZE:'

offst: db 'OFFSET:'

labelt: db   'HeEd'
labellen:

;text for hi-level menu buttons
f_menu: db 'File'
e_menu: db 'Coding'
h_menu: db 'Help'
;text for low-level menu buttons
;menu File
m_open: db 'Open'
m_save: db 'Save'
m_exit: db 'Exit'
;menu coding
m_win2dos: db 'Win->Dos'
m_win2koi: db 'Win->Koi'
m_win2iso: db 'Win->Iso'
m_dos2win: db 'Dos->Win'
m_dos2koi: db 'Dos->Koi'
m_dos2iso: db 'Dos->Iso'
;menu Help
m_help: db 'Help'
m_about: db 'About'
;tables for coding
WIN_TABLE:
db 0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9
db 0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3
db 0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD
db 0xDE,0xDF,0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7
db 0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0,0xF1
db 0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB
db 0xFC,0xFD,0xFE,0xFF
DOS_TABLE:
db 0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89
db 0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93
db 0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D
db 0x9E,0x9F,0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7
db 0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xE0,0xE1
db 0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB
db 0xEC,0xED,0xEE,0xEF
KOI_TABLE:
db 0xE1,0xE2,0xF7,0xE7,0xE4,0xE5,0xF6,0xFA,0xE9,0xEA
db 0xEB,0xEC,0xED,0xEE,0xEF,0xF0,0xF2,0xF3,0xF4,0xF5
db 0xE6,0xE8,0xE3,0xFE,0xFB,0xFD,0xFF,0xF9,0xF8,0xFC
db 0xE0,0xF1,0xC1,0xC2,0xD7,0xC7,0xC4,0xC5,0xD6,0xDA
db 0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD2,0xD3
db 0xD4,0xD5,0xC6,0xC8,0xC3,0xDE,0xDB,0xDD,0xDF,0xD9
db 0xD8,0xDC,0xC0,0xD1
ISO_TABLE:
db 0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9
db 0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,0xC0,0xC1,0xC2,0xC3
db 0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD
db 0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7
db 0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,0xE0,0xE1
db 0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB
db 0xEC,0xED,0xEE,0xEF

;text for help_window
help_label: db 'Help for HeEd.'
help_text:
 db '1.HeEd в состоянии открыть файл только один раз и  '
 db '  нужное число раз сохранить его.                  '
 db '2.При открытии файла без расширения надо это расши-'
 db '  ширение все равно указывать как три пробела после'
 db '  точки.                                           '
 db '3.Само открытие файла происходит при нажатии кнопки'
 db '  "Go".                                            '
 db '4.Создание новых файлов в меню не предусмотрено, но'
 db '  можно же редактировать...                        '
 db '5.В файл записывается только количество байт, кото-'
 db '  рое было размером файла до открытия.             '
 db '6.При нажатии "Go" с пустым полем имени файла выво-'
 db '  дится память с адреса 0х80000, но размер файла   '
 db '  равен 0xFFFFFFFF.                                '
 db '               (см. инфо "About")                  '
help_end:
;text for about_window
about_label: db 'About this funny.'
about_text:
 db 'Некоторая информация для тех, кто захочет дописать '
 db 'сюда что-то свое: код практичкски не оптимизирован,'
 db 'так что разобраться будет не так уж сложно. Строки '
 db 'для кнопок меню должны идти прямо друг за другом,  '
 db 'т. к. я при выводе использую не mov esi,размер и   '
 db 'mov  edx,адрес а просто прибавляю смещения. Что ка-'
 db 'сается кодировок и размеров файла для сохранения,  '
 db 'то остается только добавить кнопки меню с текстом  '
 db '(при добавлении надо учитывать, что ID кнопки опоз-'
 db 'наются dec ah, а не как cmp ah,ID). Если все же бу-'
 db 'дет неприятно разбираться, то можете написать и    '
 db 'спросить. Эта программа была написана в ходе разбо-'
 db 'рок с GUI MeOS и поэтому не претендует на что-то   '
 db 'большее, чем пример. Просто надоела эта тема, а вы-'
 db 'кинуть жалко.            mailto:babalbes@yandex.ru '
about_end:

I_END:

sc system_colors

process_info:
  rb 1024
menu_opened db ?
m_text:
