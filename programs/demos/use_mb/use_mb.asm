;Огромная благодарность Maxxxx32, Diamond, Heavyiron
;и другим программистам, а также
;Теплову Алексею (<Lrz> www.lrz.land.ru)
use32
  org 0
  db 'MENUET01' ;идентиф. исполняемого файла всегда 8 байт
  dd 0,start,i_end,mem,stacktop,0,cur_dir_path

include '../../KOSfuncs.inc'
include '../../macros.inc'
include '../../proc32.inc'
include '../../load_lib.mac'
include 'lang.inc'

@use_library

align 4
start:
  mcall SF_SET_EVENTS_MASK,0x25 ;маска ожидаемых событий

  sys_load_library  msgbox_name, library_path, system_path, msgbox_lib_import


red_win:
  mcall SF_REDRAW,SSF_BEGIN_DRAW

  xor eax,eax
  mov ebx,50*65536+200
  mov ecx,30*65536+170
  mov edx,0xffffd0
  or  edx,0x33000000
  mov edi,hed
  mcall

  mov eax,SF_DEFINE_BUTTON
  mov ebx,10*65536+30
  mov ecx,110*65536+20
  mov edx,7
  mov esi,0xffd040
  mcall

  mov ebx,10*65536+30
  mov ecx,85*65536+20
  mov edx,6
  mcall

  mov ebx,10*65536+30
  mov ecx,60*65536+20
  mov edx,5
  mcall

  mov ebx,10*65536+30
  mov ecx,35*65536+20
  mov edx,4
  mcall

  mov ebx,10*65536+30
  mov ecx,10*65536+20
  mov edx,3
  mcall

  mov eax,4 ;рисование текста
  mov ebx,50*65536+115
  mov ecx,0x4000d0
  or  ecx,0x80000000
  mov edx,txt5
  mcall

  mov ebx,50*65536+90
  mov edx,txt4
  mcall

  mov ebx,50*65536+65
  mov edx,txt3
  mcall

  mov ebx,50*65536+40
  mov edx,txt2
  mcall

  mov ebx,50*65536+15
  mov edx,txt1
  mcall

  push eax ebx ecx edx esi
  ;line numbers
  mov eax,SF_DRAW_NUMBER
  mov esi,0xd00000
  mov ebx,0x10000 ;format

  ; --- draw codes ---
  xor ecx,ecx
  mov cl,byte[msgbox_1]
  mov dx,25
  shl edx,16
  mov dx,15
  int 0x40

  mov cl,byte[msgbox_2]
  add dx,25
  int 0x40

  mov cl,byte[msgbox_3]
  add dx,25
  int 0x40

  mov cl,byte[msgbox_4]
  add dx,25
  int 0x40

  mov cl,byte[msgbox_5]
  add dx,25
  int 0x40

  pop esi edx ecx ebx eax
  call draw_square
  mcall SF_REDRAW,SSF_END_DRAW

align 4
still:
  mcall SF_WAIT_EVENT

  cmp al,1 ;изм. положение окна
  jz red_win
  cmp al,3
  jz button
  jmp still

button:
  mcall SF_GET_BUTTON

  cmp ah,3
  jz  but_1
  cmp ah,4
  jz  but_2
  cmp ah,5
  jz  but_3
  cmp ah,6
  jz  but_4
  cmp ah,7
  jz  but_5

  cmp ah,1
  jne still
.exit:
  mcall SF_TERMINATE_PROCESS

but_1:
  stdcall [mb_create],msgbox_1,thread
  jmp still

but_2:
  stdcall [mb_create],msgbox_2,thread
  stdcall [mb_setfunctions],msgbox_2_funct
  jmp still

but_3:
  stdcall [mb_create],msgbox_3,thread
  jmp still

but_4:
  stdcall [mb_create],msgbox_4,thread
  jmp still

but_5:
  stdcall [mb_create],msgbox_5,thread
  mcall SF_SLEEP,100 ;stop program
  stdcall [mb_reinit],msgbox_5_2
  jmp still

if lang eq ru
  txt1 db 'простое',0
  txt2 db '3 кнопки',0
  txt3 db '3 строки',0
  txt4 db 'большое',0
  txt5 db 'mb_reinit',0
  hed db 'Пример использования MsgBox',0
else
  txt1 db 'Simple',0
  txt2 db '3 buttons',0
  txt3 db '3 lines',0
  txt4 db 'Big',0
  txt5 db 'mb_reinit',0
  hed db 'MsgBox usage example',0
end if	
;sc system_colors


;---------------------------------------------------------------------
msgbox_1:
  dw 0
  db 'MBox',0 ;+2 = +MB_TEXT_OFFSET
if lang eq ru
  db 'Пример',0
  db 'Вижу',0 ;button1
else
  db 'Example',0
  db 'I see!',0 ;button1
end if
  db 0
msgbox_2:
  dw 0
  db 'MBox 3 buttons',0 ;+2 = +MB_TEXT_OFFSET
if lang eq ru
  db 'Пример с 3-мя кнопками',0
  db 'Да',0 ;button1
  db 'Отмена',0 ;button2
  db 'Помощь',0 ;button3
else
  db '3 buttons example',0
  db 'Yes',0 ;button1
  db 'Cancel',0 ;button2
  db 'Help',0 ;button3
end if
  db 0
msgbox_2_funct:
  dd 0,0,fun_show_help

msgbox_3:
  dw 0
  db 'MBox 3 lines',0 ;+2 = +MB_TEXT_OFFSET
if lang eq ru
  db 'Строка 1',13,'Строка 2',13,'Строка 3',0
  db '2020 г.',0 ;button1
else
  db 'Line 1',13,'Line 2',13,'Line 3',0
  db 'Year 2020',0 ;button1
end if	
  db 0
msgbox_4:
  dw 0
if lang eq ru
  db 'Введите день',0 ;+2 = +MB_TEXT_OFFSET
else
  db 'Select day',0 ;+2 = +MB_TEXT_OFFSET
end if
  db '  @@@@@@@@@@@@            @@@@@@@@',13
  db ' @............@          @........@',13
  db '@..............@        @..........@',13
  db '@.......@@@.....@      @......@.....@',13
  db '@.......@  @.....@@@@@@......@@......@',13
  db '@......@   @.................@@.......@',13
  db ' @....@   @.....@@......@@....@.......@',13
  db '  @..@    @.........@.........@......@',13
  db '   @@     @...................@@....@',13
  db '           @.......@@@.......@  @..@',13
  db '            @...............@    @@',13
  db '            @...............@',13
  db '           @.................@',13
  db '          @...................@',13
  db '         @.....................@',13
  db '        @.......@@......@@......@  @@',13
  db '       @......@....@..@....@.....@@..@',13
  db '       @......@....@..@....@.........@',13
  db '        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@',0
if lang eq ru
  db 'Пн',0,'Вт',0,'Ср',0,'Чт',0,'Пт',0,'Сб',0,'Воскресение',0
else
  db 'Sun',0,'Mon',0,'Tue',0,'Wed',0,'Thu',0,'Fri',0,'Sat',0
end if
  db 0
msgbox_5:
  dw 0
if lang eq ru
  db 'Сообщение',0 ;+2 = +MB_TEXT_OFFSET
  db 'Выполняю процесс ...',0
  db 'Остановить',0
  db 'Прервать',0
else
  db 'Message',0 ;+2 = +MB_TEXT_OFFSET
  db 'Running process ...',0
  db 'Stop',0
  db 'Terminate',0
end if
  db 0
msgbox_5_2:
  dw 0
if lang eq ru
  db 'Сообщение',0 ;+2 = +MB_TEXT_OFFSET
  db 'Все закончено',0
  db 'Закрыть окно',0
else
  db 'Message',0 ;+2 = +MB_TEXT_OFFSET
  db 'All finished',0
  db 'Close window',0
end if
  db 0

;--------------------------------------------------
align 4
msgbox_lib_import:
  mb_create dd amb_create
  mb_reinit dd amb_reinit
  mb_setfunctions dd amb_setfunctions
  dd 0,0
  amb_create db 'mb_create',0
  amb_reinit db 'mb_reinit',0
  amb_setfunctions db 'mb_setfunctions',0
;--------------------------------------------------
system_path db '/sys/lib/'
msgbox_name db 'msgbox.obj',0
;--------------------------------------------------

draw_square:
  cmp byte[sh_help],0
  je @f
    push eax ebx ecx edx
    mcall SF_DRAW_RECT, 105*65536+70, 15*65536+50, 0x8080ff
    mcall SF_DRAW_TEXT, 110*65536+25, 0xffffff, txt_help

    mov esi,0xffff
    mov ebx,0x10000 ;format
    movzx ecx,byte[sh_help]
    mov edx,(150 shl 16)+40
    mcall SF_DRAW_NUMBER

    pop edx ecx ebx eax
  @@:
  ret
fun_show_help:
  inc byte[sh_help]
  cmp byte[sh_help],10
  jne @f
    mov byte[sh_help],0
  @@:
  call draw_square
  ret
sh_help db 0
if lang eq ru
txt_help db 'Помощь...'
else
txt_help db 'Help...'
end if

align 16
i_end: ;конец кода
    rb 1024
  thread:
    rb 1024
stacktop:
  cur_dir_path:
    rb 4096
  library_path:
    rb 4096
  file_info:
    rb 40
mem:
