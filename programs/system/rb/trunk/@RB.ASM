;
;   DESKTOP CONTEXT MENU
;   written by Ivan Poddubny
;
;   Автор - Иван Поддубный
;   e-mail: ivan-yar@bk.ru
;
;   Compile with flat assembler
;
include 'lang.inc'
include 'macros.inc'

meos_app_start
code

  mov   eax,40       ; установим маску событий
  mov   ebx,100000b  ; нас интересует только мышь
  int   0x40

still:               ; главный цикл основного процесса

  mov   eax,10       ; ждём события
  int   0x40

  mov   eax,37       ; какие нажаты кпопки?
  mov   ebx,2
  int   0x40

  cmp   eax,2        ; если не правая, возврат
  jne   still

;---поехали!---

;  mov   eax,37       ; это для отладки - если мышь в точке (0;0), закроемся
;  xor   ebx,ebx
;  int   0x40
;  test  eax,eax      ; курсор в точке (0;0), т.е. eax = 0
;  je    exit


  mov   eax,9        ; получим число процессов в системе
  mov   ebx,procinfo
  xor   ecx,ecx
  int   0x40

  inc   eax          ; теперь в eax содержится число процессов + 1
  mov   [processes],eax
  mov   ecx,1

 new_process:
  pushad
  mov   eax,9        ; получим информацию о процессе; номер - в ecx
  mov   ebx,procinfo
  int   0x40
  mov   eax,37       ; координаты курсора
  xor   ebx,ebx
  int   0x40
  mov   ebx,eax                  ; eax = cursor_x
  shr   eax,16                   ; ebx = cursor_y
  and   ebx,0xffff
  mov   [curx1],eax              ; curx1 = cursor_x
  mov   [cury1],ebx              ; cury1 = cursor_y
; \begin{diamond}[18.09.2006]
; ignore minimized windows
        test    [procinfo.wnd_state], 2
        jnz     ne_goden
; \end{diamond}[18.09.2006]
  mov   eax,[procinfo.x_start]   ; eax = wnd_x_start
  mov   ebx,[procinfo.y_start]   ; ebx = wnd_y_start

  mov   ecx,[procinfo.x_size]
  add   ecx,eax                  ; ecx = wnd_x_end
  mov   edx,[procinfo.y_size]
  add   edx,ebx                  ; ecx = wnd_y_end

  cmp   eax,[curx1]  ; wnd_x_start > cursor_x => курсор левее окна
  jg    ne_goden
  cmp   ecx,[curx1]  ; wnd_x_end   < cursor_x => курсор правее окна
  jl    ne_goden
  cmp   ebx,[cury1]  ; wnd_y_start > cursor_y => курсор выше окна
  jg    ne_goden
  cmp   edx,[cury1]  ; wnd_y_end   < cursor_y => курсор ниже окна
  jl    ne_goden

goden:               ; клик был внутри какого-то окна, поэтому ничего не делаем
  popad
  jmp   still

ne_goden:            ; клик был снаружи рассматриваемого окна, поэтому
  popad
  inc   ecx
  cmp   ecx,[processes]
  jl    new_process  ; либо сморим следующее окно, либо запускаем меню


@@:             ; подождём, пока пользователь не отпустил правую кнопку мыши
  mov   eax,37
  mov   ebx,2   ; функция 37-2:
  int   0x40    ;   нажаты ли кнопки мыши?
  cmp   eax,ebx ; если отпустил, (eax != 2)
  jnz   @f      ;   идём в начало главного цикла

  mov   eax,5   ; иначе
  mov   ebx,2   ;   подождём 2 мс
  int   0x40

  jmp   @b      ;   и проверим мышь опять
@@:

; если уже было открыто меню, нужно подождать, пока оно закроется:
@@:
  cmp   [menu_opened],0
  je    @f
  mov   eax,5
  mov   ebx,3  ; ждём 3 мс
  int   0x40
  jmp   @b
@@:

  mov   eax,51           ; а теперь можно смело запускать процесс (поток) меню
  mov   ebx,1            ; создаём поток (thread)
  mov   ecx,start_wnd    ; точка входа потока
  mov   edx,stack_wnd    ; вершина стэка для потока
  int   0x40

  jmp   still



exit_menu:            ; если выходим из меню, надо записать в [menu_opened] 0
  mov   [menu_opened],0
exit:                 ; сюда мы идём, когда выходим из основного процесса
  or    eax,-1        ; eax = -1
  int   0x40




; здесь стартует процесс меню
start_wnd:
  mov   [menu_opened],1
  mov   eax,40      ; установим маску желаемых событий для этого процесса
  mov   ebx,100101b ; меню + кнопки + перерисовка
  int   0x40

red:
  call  draw_window

still2:             ; главный цикл процесса меню

  mov   eax,10      ; ждём события
  int   0x40

  cmp   eax,1       ; перерисовка?
  je    red
  cmp   eax,3       ; кнопка?
  je    button
  cmp   eax,6       ; мышь?
  je    mouse

  jmp   still2      ; вернёмся в начало главного цикла


; ОБРАБОТЧИК МЫШИ
mouse:            ; когда пользователь нажмёт кнопку мыши, закроемся
  mov   eax,37
  mov   ebx,2     ; какие кнопки нажаты?
  int   0x40
  test  eax,eax   ; никакие? - тогда прекрасно! вернёмся в гланый цикл
  jz    still2
  jmp   exit_menu ; а если всё-таки нажаты - закроем окно


; НАЖАТА КНОПКА
button:
  mov   eax,17        ; получить идентификатор нажатой кнопки
  int   0x40

  sub   ah,10         ; сравниваем с 10
  jl    nofuncbtns    ; если меньше - закрываем меню

  movzx ebx,ah        ; получили номер программы в списке в ebx
  mov   esi, [startapps + ebx*4]
  mov   edi, start_info.path
  cld
 @@:
  lodsb
  stosb
  test  al, al
  jnz   @b
  mcall 70, start_info

;  mov   eax,5         ; подождём, пока программа запуститься
;  mov   ebx,1         ; а то её окно не будет отрисовано (баг в ядре???)
;  int   0x40          ; раскомментируйте эти строки, если у вас проблемы
                       ; с отрисовкой

nofuncbtns:           ; закрываем меню
  jmp   exit_menu



_BTNS_            = 7     ; количество кнопок ("пунктов меню")

if lang eq ru
  font            = 0x00000000
  string_length   = 20          ; длина строки
  wnd_x_size      = 133         ; ширина окна
  header_pos      = 36 shl 16 + 7
else
  font            = 0x10000000
  string_length   = 12          ; длина строки
  wnd_x_size      = 105         ; ширина окна
  header_pos      = 23 shl 16 + 7
end if

;*******************************
;********  РИСУЕМ ОКНО  ********
;*******************************

draw_window:

  mov   eax,12           ; начинаем "рисовать"
  mov   ebx,1
  int   0x40

  mov   eax,[curx1]      ; текущие координаты курсора
  mov   [curx],eax       ; запишем в координаты окна
  mov   eax,[cury1]
  mov   [cury],eax

; теперь будем считать координаты окна, чтобы оно за край экрана не вылезло
  mov   eax,14                ; получим размер экрана
  int   0x40
  mov   ebx,eax
  shr   eax,16                ; в eax - x_screen
  and   ebx,0xffff            ; в ebx - y_screen
  add   eax,-wnd_x_size       ; eax = [x_screen - ширина окна]
  add   ebx,-_BTNS_*15-21     ; ebx = [y_screen - высота окна]

  cmp   eax,[curx]
  jg    .okx                  ; если окно слишком близко к правому краю,
  add   [curx],-wnd_x_size    ; сдвинем его влево на 100
 .okx:

  cmp   ebx, [cury]
  jg    .oky                  ; по вертикали точно также
  add   [cury], -_BTNS_*15-21
 .oky:

  mov   eax, 48                   ; получить системные цвета
  mov   ebx, 3
  mov   ecx, sc                   ;  адрес структуры
  mov   edx, sizeof.system_colors ;  и ее размер
  int   0x40

  xor   eax, eax           ; функция 0 - создать окно
  mov   ebx, [curx]        ;  ebx = [координата по x] shl 16 + [ширина]
  shl   ebx, 16
  add   ebx, wnd_x_size
  mov   ecx, [cury]        ;  ecx = [координата по y] shl 16 + [высота]
  shl   ecx, 16
  add   ecx, _BTNS_*15+21
  mov   edx, [sc.work]     ;  цвет рабочей области
  mov   esi, [sc.grab]     ;  цвет заголовка
  or    esi, 0x81000000
  mov   edi, [sc.frame]    ;  цвет рамки
  int   0x40

  mov   eax, 4             ; заголовок
  mov   ebx, header_pos    ;  [x] shl 16 + [y]
  mov   ecx, [sc.grab_text];  шрифт и цвет (серый)
  or    ecx, 0x10000000
;  add   ecx, -0x333333
  push  ecx
  push  ecx
  xor   edx,edx
.dec_color:
  sub   byte [esp+edx], 0x33
  jae   @f
  mov   byte [esp+edx], 0
@@:
  inc   edx
  jnp   .dec_color
  pop   ecx
  mov   edx, header        ;  адрес заголовка
  mov   esi, header.size   ;  длина заголовка ("M E N U")
  int   0x40
  pop   ecx
  add   ebx, 1 shl 16      ;  сдвинем вправо на 1
  int   0x40

  mov   ebx, 1*65536+wnd_x_size-2  ; начинаем делать кнопки
  mov   ecx, 20*65536+15
  mov   edx, 10 or 0x40000000 ; бит 30 установлен => кнопка не рисуется

  mov   edi,_BTNS_           ; количество кнопок (счётчик)

 newbtn:                     ; начало цикла
  mov   eax,8                ;  создаём кнопку
  int   0x40

                             ;  пишем текст на кнопке
  pushad                     ;   спасаем регистры
  shr   ecx, 16
  and   ebx, 0xffff0000
  add   ebx, ecx             ;   ebx = [x] shl 16 + [y];
  add   ebx, 10*65536+4      ;   ebx += смещение относительно края кнопки;
  mov   ecx, [sc.work_text]  ;   шрифт и цвет
  or    ecx, font
  add   edx, -10             ;   edx = номер кнопки;
  imul  edx, string_length   ;   edx *= длина строки;
  add   edx, text            ;   edx += text;  теперь в edx адрес строки
  mov   esi, string_length   ;   в esi - длина строки
  mov   eax, 4               ;   функция 4 - вывод текста
  int   0x40
  popad

  inc   edx                  ;  номер кнопки++;
  add   ecx,15*65536         ;  увеличим смещение по y
  dec   edi                  ;  уменьшим счётчик
  jnz   newbtn               ; если не ноль, повторим всё ещё раз

  mov   eax,12               ; закончили "рисовать"
  mov   ebx,2
  int   0x40

ret                          ; возврат



; ДАННЫЕ ПРОГРАММЫ
DATA

  macro strtbl name, [string]
  {
   common
     label name dword
   forward
     local str
     dd str
   forward
     str db string
  }

  strtbl startapps       ,\
    <"/RD/1/PIC4",0>     ,\
    <"/RD/1/SKINSEL",0>  ,\
    <"/RD/1/DESKTOP",0>  ,\
    <"/RD/1/ICON",0>     ,\
    <"/RD/1/SETUP",0>    ,\
    <"/RD/1/VRR",0> ,\
    <"/RD/1/CPU",0> 
    
  sz header, "KolibriOS"

  lsz text,\
    en, 'Background  ',\
    en, 'Change skin ',\
    en, 'Desktop     ',\
    en, 'Icon manager',\
    en, 'Device setup',\
    en, 'VRR         ',\
    en, 'Processes   ',\
    \
    ru, 'Генератор обоев     ',\
    ru, 'Смена скина         ',\
    ru, 'Настройка окон      ',\
    ru, 'Управление иконками ',\
    ru, 'Настройка устройств ',\
    ru, 'Настройка монитора  ',\
    ru, 'Процессы            '

start_info:
        .mode   dd      7
                dd      0
        .params dd      0
                dd      0
                dd      0
                db      0
                dd      start_info.path

; НЕИНИЦИАЛИЗИРОВАННЫЕ ДАННЫЕ
UDATA
  processes   dd ?              ; количество процессов в системе
  curx1       dd ?              ; координаты курсора
  cury1       dd ?
  curx        dd ?              ; координаты окна меню
  cury        dd ?

  menu_opened db ?              ; открыто меню или нет? (1-да, 0-нет)

align 4
start_info.path rb 256

  sc       system_colors        ; системные цвета
  procinfo process_information  ; информация о процессе

  rb 1024                       ; стэк для окна меню - хватит и 1 Кб
  align 32
  stack_wnd:


MEOS_APP_END
; КОНЕЦ ПРОГРАММЫ