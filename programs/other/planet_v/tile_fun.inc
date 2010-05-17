TILE_LEN	  equ 256
TILE_LEN_W	  equ TILE_LEN
TILE_LEN_H	  equ TILE_LEN
RGB_TILE_SIZE_1N4 equ (TILE_LEN*TILE_LEN*3/4) ;1/4 часть памяти картинки
RGB_TILE_SIZE_1N2 equ (TILE_LEN*TILE_LEN*3/2) ;1/2 часть памяти картинки
RGB_TILE_SIZE	  equ TILE_LEN*TILE_LEN*3     ;размер 1-й картинки
TREE_ICON_SYS16_BMP_SIZE equ 256*3*11+54 ;размер bmp файла с системными иконками
TREE_ICON_NOD16_BMP_SIZE equ 256*3* 1+54
TILE_TEXT_MAX_LEN equ 40 ;максимальная длинна текста для вывода имени файла

FRACT_TILE_00 equ 1 ;дробный файл, 1/4 - верхний левый угол
FRACT_TILE_01 equ 2 ;дробный файл, 1/4 - нижний левый угол
FRACT_TILE_10 equ 4 ;дробный файл, 1/4 - верхний правый угол
FRACT_TILE_11 equ 8 ;дробный файл, 1/4 - нижний правый угол
FRACT_TILE_ALL equ 15 ;15=1+2+4+8
FRACT_TILE_BIT00 equ 0
FRACT_TILE_BIT01 equ 1
FRACT_TILE_BIT10 equ 2
FRACT_TILE_BIT11 equ 3

;флаги, нужные при растягивании:
SCALE_0BIT_USE equ 8
SCALE_1BIT_USE equ 9
SCALE_0_USE equ 0x100 ;флаг открытия картинки текущего уровня
SCALE_1_USE equ 0x200 ;флаг открытия картинки высшего уровня (будет растягиватся в проприсовке)

met_fr_opn db 0 ;флаг открытия файла с метками

size_tile_struc equ 32 ;размер структуры тайла
max_tiles_cols	equ 4 ;максимальное число тайлов по ширине окна
max_tiles_rows	equ 3 ;максимальное число тайлов по высоте окна
max_tiles_count equ (max_tiles_cols*max_tiles_rows) ;максимальное число тайлов в программе

tile_coord_x	equ dword[edi] ;координата x (как параметр для загрузки)
tile_coord_y	equ dword[edi+ 4] ;координата y (как параметр для загрузки)
tile_zoom	equ  byte[edi+ 8] ;масштаб загружаемого тайла
tile_err_f	equ  byte[edi+ 9] ;код ошибки при открытии изображения
tile_drobn	equ  word[edi+10] ;поле для сохранения результатов открытия дробных картинок
tile_offs_l_load  equ 12 ;offset params last load tile
tile_key_coord_x  equ dword[edi+12] ;координата x (как ключ последней загрузки)
tile_key_coord_y  equ dword[edi+16] ;координата y (как ключ последней загрузки)
tile_key_path_len equ  byte[edi+21] ;длинна текста в поле Cache
tile_rgb_icon	equ dword[edi+22] ;указатель на буфер для хранения основной или растянутой картинки
tile_drobn_icon equ dword[edi+26] ;указатель на буфер для хранения 4-х дробных картинок
tile_delta_x	equ  byte[edi+30] ;сдвиг тайла относительно курсора по оси x
tile_delta_y	equ  byte[edi+31] ;сдвиг тайла относительно курсора по оси y


; *** порядок вызова функций при загрузке тайлов ***
;load_tile
;       get_tile_path
;       load_new_tile (текущая картинка)
;       get_tile_path
;       load_new_tile (нижняя картинка)
;       get_tile_path
;       load_new_tile (нижняя картинка)
;       get_tile_path
;       load_new_tile (нижняя картинка)
;       get_tile_path
;       load_new_tile (нижняя картинка)
;       get_parent_tiles (циклический просмотр родительских тайлов)
;               get_tile_path
;               load_new_tile (верхняя картинка)


;???
align 4
CursorMoveRight: ;передвигаем координату курсора вправо
  push ebx ecx
  xor ebx,ebx
  xor ecx,ecx
  mov cl,byte[map.zoom]
  dec ecx
  cmp cx,0
  jle .no_move ;при 0-м масштабе не двигаемся
  @@:
    rol ebx,1
    or ebx,1 ;накапливаем максимальную координату, при даном масштабе
    loop @b
  cmp dword[map.coord_x],ebx
  jge .no_move ;если координата крайняя, не двигаемся
    inc dword[map.coord_x]
    ;сдвигаем сетку тайлов влево, что-бы часть тайлов совпали и пришлось меньше загружать новых
    stdcall tiles_grid_move_left, tile_00,max_tiles_count,max_tiles_cols
    call but_Refresh
  .no_move:
  pop ecx ebx
  ret

;???
align 4
CursorMoveDown: ;передвигаем координату курсора вниз
  push ebx ecx
  xor ebx,ebx
  xor ecx,ecx
  mov cl,byte[map.zoom]
  dec ecx
  cmp cx,0
  jle .no_move ;при 0-м масштабе не двигаемся
  @@:
    rol ebx,1
    or ebx,1 ;накапливаем максимальную координату, при даном масштабе
    loop @b
  cmp dword[map.coord_y],ebx
  jge .no_move ;если координата крайняя, не двигаемся
    inc dword[map.coord_y]
    ;сдвигаем сетку тайлов вверх
    stdcall tiles_grid_move_up, tile_00,max_tiles_count,max_tiles_rows
    call but_Refresh
  .no_move:
  pop ecx ebx
  ret

;???
align 4
but_met_up: ;перемещение метки вверх, по списку
  stdcall dword[tl_node_move_up], tree2
  ret

;???
align 4
but_met_dn: ;перемещение метки вниз, по списку
  stdcall dword[tl_node_move_down], tree2
  ret

;???
align 4
fun_new_map_type: ;меняем расширение для карты
  push eax
    stdcall dword[tl_node_get_data], tree1
    pop eax
    cmp eax,0
    je @f
      mov dword[txt_tile_type],eax
    @@:
  pop eax
  ret

;задаем расположение тайлов по сетке
align 4
proc tiles_init_grid, beg_tiles:dword, count:dword, cols:dword
  push eax ebx ecx edx edi
  xor eax,eax
  xor ebx,ebx
  mov edi,[beg_tiles]
  mov ecx,[count]
  mov edx,[cols]
  @@:
    mov tile_delta_x,al
    mov tile_delta_y,bl
    add edi,size_tile_struc
    inc ax
    cmp eax,edx
    jl .old_lin
      xor eax,eax
      inc bx
    .old_lin:
    loop @b
  pop edi edx ecx ebx eax
  ret
endp

;сдвигаем сетку влево, вылезшие столбцы будут перекинуты на правую сторону
;функция нужна для оптимизации обращения к диску, при движении курсора мы двигаем
;тайлы в противоположную сторону, в результате координаты большинства тайлов не меняются
;а значит программа не будет перечитывать их заново, чем и достигается оптимизация
align 4
proc tiles_grid_move_left, beg_tiles:dword, count:dword, cols:dword
  push eax ebx ecx edx edi
  xor eax,eax
  xor ebx,ebx
  mov edi,[beg_tiles]
  mov ecx,[count]
  mov edx,[cols]
  @@:
    cmp tile_delta_x,0
    jne .no_jump
      add tile_delta_x,dl ;если столбец = 0, то добавляем 'cols' что-бы не вылазил влево
    .no_jump:
    dec tile_delta_x
    add edi,size_tile_struc
    loop @b
  pop edi edx ecx ebx eax
  ret
endp

;сдвигаем сетку вправо, вылезшие столбцы будут перекинуты на левую сторону
;делает тоже что и tiles_grid_move_left только двигает координаты в другую сторону
align 4
proc tiles_grid_move_right, beg_tiles:dword, count:dword, cols:dword
  push eax ebx ecx edx edi
  xor eax,eax
  xor ebx,ebx
  mov edi,[beg_tiles]
  mov ecx,[count]
  mov edx,[cols]
  @@:
    inc tile_delta_x
    cmp tile_delta_x,dl
    jl .no_jump
      mov tile_delta_x,0 ;если столбец = 'cols', то ставим ему 0 что-бы не вылазил вправо
    .no_jump:
    add edi,size_tile_struc
    loop @b
  pop edi edx ecx ebx eax
  ret
endp

align 4
proc tiles_grid_move_up, beg_tiles:dword, count:dword, rows:dword
  push eax ebx ecx edx edi
  xor eax,eax
  xor ebx,ebx
  mov edi,[beg_tiles]
  mov ecx,[count]
  mov edx,[rows]
  @@:
    cmp tile_delta_y,0
    jne .no_jump
      add tile_delta_y,dl ;если строка = 0, то добавляем 'rows' что-бы не вылазила вниз
    .no_jump:
    dec tile_delta_y
    add edi,size_tile_struc
    loop @b
  pop edi edx ecx ebx eax
  ret
endp

align 4
proc tiles_grid_move_down, beg_tiles:dword, count:dword, rows:dword
  push eax ebx ecx edx edi
  xor eax,eax
  xor ebx,ebx
  mov edi,[beg_tiles]
  mov ecx,[count]
  mov edx,[rows]
  @@:
    inc tile_delta_y
    cmp tile_delta_y,dl
    jl .no_jump
      mov tile_delta_y,0 ;если строка = 'rows', то ставим ей 0 что-бы не вылазила вверх
    .no_jump:
    add edi,size_tile_struc
    loop @b
  pop edi edx ecx ebx eax
  ret
endp

align 4
proc array_tile_function, beg_tiles:dword, count:dword, funct:dword
  push ecx edi
  mov edi,[beg_tiles]
  mov ecx,[count]
  @@:
    stdcall [funct],edi
    add edi,size_tile_struc
    loop @b
  pop edi ecx
  ret
endp

align 4
draw_tiles:
pushad
  mov eax,47
  mov ebx,0x20000
  xor ecx,ecx
  mov cl,byte[map.zoom]
  mov edx,128*65536+13
  mov esi,[sc.work_text]
  or  esi,0x40000000
  mov edi,[sc.work]
  int 0x40

  stdcall array_tile_function, tile_00,max_tiles_count,draw_tile
popad
  ret

; edi - pointer tile struct
;output:
; eax - 7
; edx - coordinates to tile draw

align 4
proc tile_clear_area, height:dword
  pushad
    mov eax,13 ;draw rect

    mov cx,dx
    shl ecx,16
    mov cx,word[height]

    shr edx,16
    mov bx,dx
    shl ebx,16
    mov bx,TILE_LEN_W

    mov edx,0xffffd0
    int 0x40
  popad
  ret
endp

;???
align 4
proc draw_tile, tile:dword
  pushad
  mov edi,[tile]

;*** set base point ***
  mov edx,5*65536+35
  xor eax,eax
  mov al,tile_delta_y
  shl ax,8 ;*256
  add edx,eax
  xor eax,eax
  mov al,tile_delta_x
  shl eax,8+16
  add edx,eax
;*** set base point ***

  mov ebx,dword[procinfo.box.width] ;проверяем влазит ли картинка по оси x
  sub ebx,min_window_w-5 ;5 - отступ слева для 1-й картинки
  shl ebx,16
  mov bx,dx ;нижняя координата пока не учитывается, потому делаем их равными
  cmp edx,ebx
  jg .end_f

  ;проверяем влазит ли картинка по оси y
  mcall 48,4 ;eax = высота скина
  mov ecx,dword[procinfo.box.height]
  mov ebx,edx
  and ebx,0xffff ;ebx = координата 'y' для вывода тайла
  add eax,ebx
  add eax,5+5 ;5-рамка

  cmp ecx,eax
  jle .end_f

  sub ecx,eax
  cmp ecx,TILE_LEN_H
  jle @f
    mov ecx,TILE_LEN_H
  @@:

  mov eax,7
  bt tile_drobn,SCALE_0BIT_USE
  jc .draw
  bt tile_drobn,SCALE_1BIT_USE
  jc .draw
  jmp @f
    .draw:
    mov ebx,tile_rgb_icon
    or ecx,TILE_LEN_W*65536 ;задание ширины для целой картинки
    int 0x40 ;вывод на экран целой или увеличиной картинки
    jmp .end_clear_tile
  @@:
    cmp tile_drobn,FRACT_TILE_ALL
    je .end_clear_tile
      stdcall tile_clear_area, ecx ;чистка области тайла
  .end_clear_tile:

    cmp tile_drobn,0
    je .info ;если нет ни одной дробной картинки

mov si,cx ;si - высота нижней дробной картинки
sub si,TILE_LEN_H/2 ;si - высота нижней дробной картинки
cmp cx,TILE_LEN_H/2
jl @f
  mov cx,TILE_LEN_H/2 ;задание высоты для верхней дробной картинки
@@:

    and ecx,0xffff ;стирание старой ширины
    or ecx,(TILE_LEN_W/2)*65536 ;задание ширины для дробной картинки
    mov ebx,tile_drobn_icon
    bt tile_drobn,FRACT_TILE_BIT00
    jae @f
      int 0x40
    @@:
    add dx,TILE_LEN_H/2
    add ebx,RGB_TILE_SIZE_1N4
    bt tile_drobn,FRACT_TILE_BIT01
    jae @f
      xchg cx,si
      int 0x40
      xchg cx,si
    @@:

    sub dx,TILE_LEN_H/2
    ror edx,16
    add dx,TILE_LEN_W/2
    ror edx,16
    add ebx,RGB_TILE_SIZE_1N4
    bt tile_drobn,FRACT_TILE_BIT10
    jae @f
      int 0x40
    @@:

    add dx,TILE_LEN_H/2
    add ebx,RGB_TILE_SIZE_1N4
    bt tile_drobn,FRACT_TILE_BIT11
    jae @f
      xchg cx,si
      int 0x40
    @@:

    jmp .end_f
  .info: ;вывод информации, при не удачном открытии картинок

;;;jmp .end_f ;test
  bt tile_drobn,SCALE_0BIT_USE
  jc .end_f
  bt tile_drobn,SCALE_1BIT_USE
  jc .end_f

    mov ebx,edx
    add ebx,0x30003
    mov edx,tile_rgb_icon ;при неудачном открытии в tile_rgb_icon путь к файлу
;;;    mov dword[edx+RGB_TILE_SIZE_1N4],0 ;ставим нули, на случай если в строке с именем файла мусор
    stdcall [str_len],tile_rgb_icon,RGB_TILE_SIZE_1N4
    mov edi,eax

    mov eax,4 ;draw text
    mov ecx,0xff
;or ecx,0x80000000
    mov esi,TILE_TEXT_MAX_LEN

    @@:
      cmp edi,TILE_TEXT_MAX_LEN
      jg .no_line_last
	mov esi,edi
	int 0x40
	jmp .end_f
      .no_line_last:
      int 0x40
      add edx,esi
      sub edi,esi
      add ebx,10 ;опускаемся на нижнюю строку
      jmp @b
  .end_f:
  popad
  ret
endp

;увеличение масштаба
align 4
but_ZoomP:
  cmp byte[map.zoom],24
  jge @f
    inc byte[map.zoom]
    shl dword[map.coord_x],1
    shl dword[map.coord_y],1
    call but_Refresh
  @@:
  ret

;уменьшение масштаба
align 4
but_ZoomM:
  cmp byte[map.zoom],1
  jle @f
    dec byte[map.zoom]
    shr dword[map.coord_x],1
    shr dword[map.coord_y],1
    call but_Refresh
  @@:
  ret

;???
align 4
but_MetSave: ;сохранение файла с метками
  stdcall dword[tl_save_mem], TREE_ICON_SYS16_BMP_SIZE,[bmp_icon],0,tree2

  push eax ebx ecx edi
    copy_path fn_metki,sys_path,file_name,0x0

    stdcall dword[tl_get_mem_size], [bmp_icon],tree2
    pop ecx
    inc ecx ;для записи в файл завершающего нуля

    mov eax,70
    m2m [run_file_70.Buffer], [bmp_icon]
    mov [run_file_70.Function], 2
    mov [run_file_70.Position], 0
    mov [run_file_70.Flags], 0
    mov [run_file_70.Count], ecx
    mov byte[run_file_70+20], 0
    mov [run_file_70.FileName], file_name
    mov ebx,run_file_70
    int 0x40
  pop edi ecx ebx eax
  jmp still

;???
align 4
but_MetLoad: ;считываем файл с метками
  cmp byte[met_fr_opn],0
  jne .end_f
  pushad
    mov byte[met_fr_opn],1
    copy_path fn_metki,sys_path,file_name,0x0
    mov eax,70 ;load icon file
    mov [run_file_70.Function], 0
    mov [run_file_70.Position], 0
    mov [run_file_70.Flags], 0
    mov [run_file_70.Count], TREE_ICON_SYS16_BMP_SIZE
    mov byte[run_file_70+20], 0
    mov [run_file_70.FileName], file_name
    m2m [run_file_70.Buffer], [bmp_icon]
    mov eax,70
    mov ebx,run_file_70
    int 0x40
    cmp ebx,0xffffffff
    je @f
      stdcall [tl_load_mem], TREE_ICON_SYS16_BMP_SIZE,dword[bmp_icon],0,tree2
      stdcall [tl_cur_beg], tree2 ;?
      stdcall [tl_draw], tree2 ;?
      pop eax
    @@:
    mov byte[file_name],0 ;clear file name
  popad
  .end_f:
  ret

;???
align 4
but_MetAdd: ;добавление метки
  push eax
    mov eax,dword[bmp_icon]
    m2m dword[eax],  dword[map.coord_x]
    m2m dword[eax+4],dword[map.coord_y]
    m2m  word[eax+8], word[map.zoom]
    add eax,12
    stdcall dword[str_copy], eax,30,dword[edit2.text],30
    stdcall dword[tl_node_add], dword[bmp_icon], 0, tree2
    stdcall dword[tl_draw], tree2
  pop eax
  jmp still

;???
align 4
fun_goto_met: ;переход к выбранной метке
  push eax
  stdcall dword[tl_node_get_data], tree2
  pop eax
  cmp eax,0
  je @f ;курсор не стоит на метке
    m2m dword[map.coord_x],dword[eax]
    m2m dword[map.coord_y],dword[eax+4]
    m2m word[map.zoom],word[eax+8]
    call but_Refresh
  @@:
  pop eax
  ret

;???
align 4
but_Refresh:
  call fun_new_map_type ;проверяем не поменялся ли тип просматриваемых файлов

  stdcall array_tile_function, tile_00,max_tiles_count,load_tile
  stdcall draw_tiles
  ret

align 4
proc tile_init, tile:dword
  push eax edi
    mov edi,[tile]

    push dword RGB_TILE_SIZE+300
    call mem.Alloc
    mov tile_rgb_icon,eax

    push dword RGB_TILE_SIZE
    call mem.Alloc
    mov tile_drobn_icon,eax
  pop edi eax
  ret
endp

align 4
proc tile_destroy, tile:dword
  push edi
    mov edi,[tile]
    push tile_rgb_icon
    call mem.Free
    push tile_drobn_icon
    call mem.Free
  pop edi
  ret
endp


;???
align 4 ;загружает тайлы, в определенной области
proc load_tile, tile:dword
  push eax ebx ecx edi
  mov edi,dword[tile] ;копируем указатель на структуру тайла, что-бы работали константы

  stdcall mem_copy, map,edi, 9 ;копируем координаты курсора в структуру тайла
  xor eax,eax
  mov al,tile_delta_x
  add tile_coord_x,eax ;сдвигаем координаты тайла, относительно курсора
  mov al,tile_delta_y
  add tile_coord_y,eax
  stdcall get_tile_path,edi    ;берем путь к тайлу

;***
  ; проверка координат последней загруженной картинки
  mov eax,edi
  mov ecx,9
  @@:
    mov bl,byte[eax+tile_offs_l_load]
    cmp byte[eax],bl
    jne .st_load
    inc eax
    loop @b
  mov ebx,dword[edit1.size];проверяем также тип карт, который был ранее загружен
  cmp tile_key_path_len,bl ;при изменении типа карты нужно также перечитывать файлы
  jne .st_load

  jmp @f    ;если координаты совпали то уже не загружаем
  .st_load: ;start load tile

  ; установка новых координат для последней загруженной картинки
  mov eax,edi
  add eax,tile_offs_l_load
  stdcall mem_copy, edi,eax,9
  mov ebx,dword[edit1.size]
  mov tile_key_path_len,bl
;***

  ;mov tile_drobn,0
  mov tile_drobn,SCALE_0_USE ;ставим флаг открытия картинки текущего уровня
  stdcall load_new_tile,edi
  cmp tile_err_f,0
  je @f ;если удачно открыли текущий файл то остальные действия пропускаем
;  cmp tile_err_f,6
;  je @f
    mov tile_drobn,0
    mov eax,ch2 ;проверяем опцию 'брать снизу'
    bt dword[eax+32],1 ;ch_flags equ [edi+32] ;флаги
    jae .opt_drobn_false

    ;если файл не найден, попадаем сюда
    inc tile_zoom
    shl tile_coord_y,1
    shl tile_coord_x,1
    stdcall get_tile_path,edi
    stdcall load_new_tile,edi

    cmp tile_err_f,0
    jne .no_tile_00 ;если неудачно открыли то не уменьшаем изображение
      or tile_drobn,FRACT_TILE_00
; 1 сжатие картинки по высоте
      stdcall img_rgb_hdiv2, tile_rgb_icon, TILE_LEN*TILE_LEN, TILE_LEN
; 2 сжатие картинки по ширине
      stdcall img_rgb_wdiv2, tile_rgb_icon, TILE_LEN*TILE_LEN/2
; 3 копирование 1-й картинки в резервный буфер
      stdcall mem_copy, tile_rgb_icon, tile_drobn_icon, RGB_TILE_SIZE_1N4
    .no_tile_00:

    ;если файл не найден, попадаем сюда
    inc tile_coord_y
    stdcall get_tile_path,edi
    stdcall load_new_tile,edi

    cmp tile_err_f,0
    jne .no_tile_01 ;если неудачно открыли то не уменьшаем изображение
;      mov byte[map.drobn],0
      or tile_drobn,FRACT_TILE_01
; 1 сжатие картинки по высоте
      stdcall img_rgb_hdiv2, tile_rgb_icon, TILE_LEN*TILE_LEN, TILE_LEN
; 2 сжатие картинки по ширине
      stdcall img_rgb_wdiv2, tile_rgb_icon, TILE_LEN*TILE_LEN/2
; 3 копирование 2-й картинки в резервный буфер
      mov eax,tile_drobn_icon
      add eax,RGB_TILE_SIZE_1N4 ; сдвиг картинки вниз (т. к. она нижняя)
      stdcall mem_copy, tile_rgb_icon, eax, RGB_TILE_SIZE_1N4
    .no_tile_01:

    ;если файл не найден, попадаем сюда
    dec tile_coord_y
    inc tile_coord_x
    stdcall get_tile_path,edi
    stdcall load_new_tile,edi

    cmp tile_err_f,0
    jne .no_tile_10 ;если неудачно открыли то не уменьшаем изображение
;      mov byte[map.drobn],0
      or tile_drobn,FRACT_TILE_10
; 1 сжатие картинки по высоте
      stdcall img_rgb_hdiv2, tile_rgb_icon, TILE_LEN*TILE_LEN, TILE_LEN
; 2 сжатие картинки по ширине
      stdcall img_rgb_wdiv2, tile_rgb_icon, TILE_LEN*TILE_LEN/2
; 3 копирование 3-й картинки в резервный буфер
      mov eax,tile_drobn_icon
      add eax,RGB_TILE_SIZE_1N2 ; сдвиг картинки вниз (т. к. она нижняя)
      stdcall mem_copy, tile_rgb_icon, eax, RGB_TILE_SIZE_1N4
    .no_tile_10:

    ;если файл не найден, попадаем сюда
    inc tile_coord_y
    stdcall get_tile_path,edi
    stdcall load_new_tile,edi

    cmp tile_err_f,0
    jne .no_tile_11 ;если неудачно открыли то не уменьшаем изображение
;      mov byte[map.drobn],0
      or tile_drobn,FRACT_TILE_11
; 1 сжатие картинки по высоте
      stdcall img_rgb_hdiv2, tile_rgb_icon, TILE_LEN*TILE_LEN, TILE_LEN
; 2 сжатие картинки по ширине
      stdcall img_rgb_wdiv2, tile_rgb_icon, TILE_LEN*TILE_LEN/2
; 3 сдвиг картинки вниз без копирования (т. к. она нижняя)
      stdcall img_rgb_hoffs, tile_rgb_icon, TILE_LEN*TILE_LEN/4, TILE_LEN/2, TILE_LEN*3/2

      ;копируем последнюю дробную картинку в буфер drobn_icon
      ;в котором уже были сформированы 3 другие
      mov eax,tile_drobn_icon
      add eax,RGB_TILE_SIZE_1N4*3
      stdcall mem_copy, tile_rgb_icon, eax, RGB_TILE_SIZE_1N4
    .no_tile_11:
  .opt_drobn_false:


  mov eax,ch1 ;проверяем опцию 'брать сверху'
  bt dword[eax+32],1 ;ch_flags equ [edi+32] ;флаги
  jae @f
  cmp tile_drobn,FRACT_TILE_ALL ;если собрана вся картинка, то сверху уже не берем
  je @f
    ;не знаем была-ли включена опция 'брать снизу', для которой изменялись координаты
    ;потому опять копируем координаты курсора в структуру тайла
    stdcall mem_copy, map,edi, 9
    xor eax,eax
    mov al,tile_delta_x
    add tile_coord_x,eax ;сдвигаем координаты тайла, относительно курсора
    mov al,tile_delta_y
    add tile_coord_y,eax

    xor ecx,ecx
    .zoom:
      cmp tile_zoom,0
      je @f ;просматривается самая верхняя картинка, выходим
      call get_parent_tiles ;циклический просмотр родительских тайлов
      bt tile_drobn,SCALE_1BIT_USE
      jc @f
      cmp ecx,5 ;5 - ограничение на степень растягивания картинки
      je @f
      jmp .zoom
    @@:

  pop edi ecx ebx eax
  ret
endp

;output:
;ecx - if not file = count parent levels else = 0
align 4
proc get_parent_tiles
  dec tile_zoom
  shr tile_coord_y,1
  shr tile_coord_x,1
  inc ecx

  stdcall get_tile_path,edi ;берем путь к тайлу
  stdcall load_new_tile,edi
  cmp tile_err_f,0
  jne @f ;если неудачно открыли то не растягиваем изображение
    or tile_drobn,SCALE_1_USE
    mov ebx,ecx
  .zoom_cycle:
    xor eax,eax
    dec ebx
    bt tile_key_coord_y,ebx
    jae .scale_bot_l
      add eax,RGB_TILE_SIZE_1N2
    .scale_bot_l:
    bt tile_key_coord_x,ebx
    jae .scale_rig_l
      add eax,TILE_LEN_W*3/2
    .scale_rig_l:
    cmp eax,0
    je .scale_top_left
      add eax,tile_rgb_icon
      stdcall mem_copy, eax, tile_rgb_icon, RGB_TILE_SIZE_1N2
    .scale_top_left:

    ;растягивание верхней картинки
    stdcall img_rgb_wcrop, tile_rgb_icon,TILE_LEN_W,TILE_LEN_W/2,TILE_LEN_H ;отрезаем лишнее
    stdcall img_rgb_wmul2, tile_rgb_icon,RGB_TILE_SIZE_1N4/3 ;растигиваем по ширине
    stdcall img_rgb_hmul2, tile_rgb_icon,RGB_TILE_SIZE_1N2/3,TILE_LEN_W ;растягиваем по высоте
  loop .zoom_cycle
  @@:
  ret
endp

;tile - указатель на структуру тайла
align 4 ;загружает определенный тайл
proc load_new_tile, tile:dword
  pushad

  mov eax,70 ;load icon file
  mov [run_file_70.Function], 0
  mov [run_file_70.Position], 0
  mov [run_file_70.Flags], 0
  mov [run_file_70.Count], RGB_TILE_SIZE+300 ;+54
  m2m [run_file_70.Buffer], [bmp_icon]
  mov byte[run_file_70+20], 0
  mov [run_file_70.FileName], file_name
  mov ebx,run_file_70
  int 0x40
  mov edi,[tile]
;  cmp ebx,0xffffffff
;  je .no_load
  cmp al,6 ;ошибка 6 - когда файл закончился раньше чем буфер
  jne @f
    xor al,al ;нам эта ошибка не страшна, потому ставим код равным 0, как при обычном открытии
  @@:
  mov tile_err_f,al
  cmp al,0
  jne .no_load

  stdcall dword[img_decode], dword[bmp_icon],ebx,0
  mov dword[data_icon],eax
  stdcall dword[img_to_rgb2], dword[data_icon],tile_rgb_icon
  stdcall dword[img_destroy], dword[data_icon]
    jmp @f
  .no_load:

  mov al,byte[map.zoom]
  cmp tile_zoom,al
  jne @f
    mov eax,tile_rgb_icon
    mov bl,tile_err_f
    add bl,'0' ;bl - код ошибки открытия
    mov byte[eax],bl
    inc eax
    mov byte[eax],' '
    inc eax
    stdcall mem_copy, file_name,eax,250 ;копируем путь к файлу при ошибочном открытии
  @@:
  popad
  ret
endp

;функция генерирует часть пути к файлу в виде: \z?\?\x?\?\y?
align 4
proc get_tile_path, tile:dword
  push eax ecx edi esi
    mov ecx,300
    mov edi,txt_tile_path
    @@:
      mov byte[edi],0 ;чистим строку
      inc edi
      loop @b

    mov esi,dword[tile] ;берем указатель

    mov ecx,0x80000000 ;rol 0x80000000,1 = 1
    xor eax,eax
    mov al,byte[esi+8] ;+8 = zoom
    .calc_coord:
      rol ecx,1 ;ecx = maximum coord
      dec ax
      cmp ax,0
      jg .calc_coord

    mov byte[txt_tile_path],'z'
    mov edi,txt_tile_path
    inc edi
    xor eax,eax
    mov al,byte[esi+8] ;+8 = zoom
    call tl_convert_to_str

    inc edi
    cmp byte[esi+8],9 ;+8 = zoom
    jle @f
      inc edi
    @@:
    mov byte[edi],'/'
    inc edi

    mov eax,[esi] ;coord_x
cmp eax,ecx
jge .out_of_rang ;coord_x больше допустимого значения при заданном масштабе
    shr eax,10
    call tl_convert_to_str

    @@:
      inc edi
      cmp byte[edi],'0'
      jge @b
    mov byte[edi],'/'
    inc edi
    mov byte[edi],'x'
    inc edi

    mov eax,[esi] ;coord_x
    call tl_convert_to_str

    @@:
      inc edi
      cmp byte[edi],'0'
      jl @f
      cmp byte[edi],'9'
      jg @f
      jmp @b
    @@:

    ;coord y
    mov byte[edi],'/'
    inc edi

    mov eax,[esi+4] ;coord_y
cmp eax,ecx
jge .out_of_rang ;coord_y больше допустимого значения при заданном масштабе
    shr eax,10
    call tl_convert_to_str

    @@:
      inc edi
      cmp byte[edi],'0'
      jge @b
    mov byte[edi],'/'
    inc edi
    mov byte[edi],'y'
    inc edi

    mov eax,[esi+4] ;coord_y]
    call tl_convert_to_str

.out_of_rang:
    call get_all_path
  pop esi edi ecx eax
  ret
endp

align 4
get_all_path: ;функция для собирания полного пути из всех кусков
  mov ecx,dword[edit1.text]
  add ecx,dword[edit1.size]
  dec ecx

  stdcall [str_copy], file_name,4096-1, dword[edit1.text],dword[edit1.max]

  cmp byte[ecx],'/'
  je @f
    stdcall [str_catchar],file_name,4096-1, dword '/'
  @@:

  stdcall [str_cat], file_name,4096-1, txt_tile_path,300
  stdcall [str_cat], file_name,4096-1, dword [txt_tile_type],10
ret
