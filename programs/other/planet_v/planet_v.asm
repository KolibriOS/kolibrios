;Огромная благодарность Maxxxx32, Diamond, Heavyiron
;и другим программистам, а также
;Теплову Алексею (<Lrz> www.lrz.land.ru)
use32
  org 0x0
  db 'MENUET01' ;идентиф. исполняемого файла всегда 8 байт
  dd 0x1
  dd start
  dd i_end ; размер приложения
  dd mem
  dd stacktop
  dd 0x0
  dd sys_path

include '../../proc32.inc'
include '../../macros.inc'
include 'mem.inc'
include 'dll.inc'

include '../../develop/libraries/box_lib/load_lib.mac'
include '../../develop/libraries/box_lib/trunk/box_lib.mac'

min_window_w equ 485 ;минимальная ширина окна
min_window_h equ 325 ;минимальная высота окна
otst_panel_left equ 265

include 'tile_fun.inc'
include 'pl_import.inc'

  @use_library_mem mem.Alloc,mem.Free,mem.ReAlloc, dll.Load

struct FileInfoBlock
	Function dd ?
	Position dd ?
	Flags	 dd ?
	Count	 dd ?
	Buffer	 dd ?
		db ?
	FileName dd ?
ends

fn_metki db 'pl_metki.lst',0
fn_icon1 db 'tl_sys_16.png',0
fn_icon2 db 'tl_nod_16.bmp',0

ini_name db 'planet_v.ini',0
ini_sec  db 'Map',0
ini_k_cache db 'Cache',0
ini_def_cache db '/cache/sat',0
ini_ext  db 'ext'
.number  db '?'
.def db 0 ;расширение по умолчанию


align 4
start:
  load_libraries l_libs_start,load_lib_end

;проверка на сколько удачно загузилась наша либа
	mov	ebp,lib0
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
	mcall	-1	;exit not correct
@@:
	mov	ebp,lib1
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
	mcall	-1	;exit not correct
@@:
	mov	ebp,lib2
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
	mcall	-1	;exit not correct
@@:
	mov	ebp,lib3
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
	mcall	-1	;exit not correct
@@:

  copy_path ini_name,sys_path,file_name,0x0
  stdcall dword[ini_get_str],file_name,ini_sec,ini_k_cache,dword[edit1.text],dword[edit1.max],ini_def_cache
  stdcall [str_len],dword[edit1.text],dword[edit1.max]
  mov dword[edit1.size],eax
  mov dword[edit1.pos],eax

  stdcall dword[tl_data_init], tree1
  stdcall dword[tl_data_init], tree2

;считываем расширения карт из *.ini файла
  mov byte[ini_ext.number],'0'
@@: ;считываем параметры от ext1 до ext9
  inc byte[ini_ext.number]
  stdcall dword[ini_get_str],file_name,ini_sec,ini_ext,txt_tile_type_0,dword[tree1.info_capt_len],ini_ext.def
  cmp byte[txt_tile_type_0],0
  je @f
    stdcall dword[tl_node_add], txt_tile_type_0, 0, tree1
  jmp @b
@@:
  mov byte[ini_ext.number],'0' ;считываем параметр от ext0 который будет выбран в списке
  stdcall dword[ini_get_str],file_name,ini_sec,ini_ext,txt_tile_type_0,dword[tree1.info_capt_len],ini_ext.def
  cmp byte[txt_tile_type_0],0
  jne @f
    mov dword[txt_tile_type_0],'.bmp' ;если в *.ini файле ничего нет добавляем расширение .bmp
  @@:
  stdcall dword[tl_node_add], txt_tile_type_0, 0, tree1

; init bmp file
  stdcall mem.Alloc, dword RGB_TILE_SIZE+300 ;300 - запасные байты с учетом заголовка bmp файла
  mov [bmp_icon],eax

  stdcall array_tile_function, tile_00,max_tiles_count,tile_init
  stdcall tiles_init_grid, tile_00,max_tiles_count,max_tiles_cols

  stdcall mem.Alloc, dword TREE_ICON_SYS16_BMP_SIZE
  mov [tree_sys_icon],eax

  stdcall mem.Alloc, dword TREE_ICON_NOD16_BMP_SIZE
  mov [tree_nod_icon],eax

  copy_path fn_icon1,sys_path,file_name,0x0
  mov eax,70 ;load icon file
  mov [run_file_70.Function], 0
  mov [run_file_70.Position], 0
  mov [run_file_70.Flags], 0
  mov [run_file_70.Count], TREE_ICON_SYS16_BMP_SIZE
  m2m [run_file_70.Buffer], [tree_sys_icon]
  mov byte[run_file_70+20], 0
  mov [run_file_70.FileName], file_name
  mov ebx,run_file_70
  int 0x40
  cmp ebx,0xffffffff
  je @f
    stdcall dword[img_decode], dword[tree_sys_icon],ebx,0
    mov dword[data_icon],eax
    stdcall dword[img_to_rgb2], dword[data_icon],dword[tree_sys_icon]
    stdcall dword[img_destroy], dword[data_icon]

    m2m dword[tree1.data_img_sys],dword[tree_sys_icon]
    m2m dword[tree2.data_img_sys],dword[tree_sys_icon]
  @@:


  copy_path fn_icon2,sys_path,file_name,0x0
  mov eax,70 ;load icon file
  mov [run_file_70.Count], TREE_ICON_NOD16_BMP_SIZE
  m2m [run_file_70.Buffer], [tree_nod_icon]
  mov ebx,run_file_70
  int 0x40
  cmp ebx,0xffffffff
  je @f
    stdcall dword[img_decode], dword[tree_nod_icon],ebx,0
    mov dword[data_icon],eax
    stdcall dword[img_to_rgb2], dword[data_icon],dword[tree_nod_icon]
    stdcall dword[img_destroy], dword[data_icon]

    m2m dword[tree1.data_img],dword[tree_nod_icon]
    m2m dword[tree2.data_img],dword[tree_nod_icon]
  @@:

  mcall 40,0x27

  mcall 48,3,sc,sizeof.system_colors
  ;установка системных цветов
  edit_boxes_set_sys_color edit1,editboxes_end,sc
  check_boxes_set_sys_color ch1,checkboxes_end,sc

  mov byte[file_name],0

  ; OpenDialog initialisation
  stdcall [OpenDialog_Init],OpenDialog_data

align 4
red_win:
  call draw_window
  call but_MetLoad

align 4
still:
  mov eax,10
  mcall

  cmp al,0x1 ;изм. положение окна
  jz red_win
  cmp al,0x2
  jz key
  cmp al,0x3
  jz button

  push dword ch2
  call [check_box_mouse]
  push dword ch1
  call [check_box_mouse]

  stdcall [edit_box_mouse], edit1
  stdcall [edit_box_mouse], edit2

  stdcall [tl_mouse], tree1
  stdcall [tl_mouse], tree2

  jmp still

align 4
key:
  push eax ebx
  mcall 2
  stdcall [edit_box_key], edit1
  stdcall [edit_box_key], edit2

  push dword tree1
  call [tl_key]
  push dword tree2
  call [tl_key]

  mov ebx,dword[el_focus] ;что-бы карта не двигалась если окна treelist в фокусе
  cmp ebx, dword tree1
  je .end_f
  cmp ebx, dword tree2
  je .end_f

  ;что-бы карта не двигалась если текстовые поля в фокусе
  test word[edit1.flags],10b ;ed_focus
  jne .end_f
  test word[edit2.flags],10b ;ed_focus
  jne .end_f

    cmp ah,179 ;Right
    jne @f
      call CursorMoveRight
    @@:
    cmp ah,176 ;Left
    jne @f
    cmp dword[map.coord_x],0
    je @f
      dec dword[map.coord_x]
      ;сдвигаем сетку тайлов вправо, что-бы часть тайлов совпали и пришлось меньше загружать новых
      stdcall tiles_grid_move_right, tile_00,max_tiles_count,max_tiles_cols
      call but_Refresh
    @@:
    cmp ah,177 ;Down
    jne @f
      call CursorMoveDown
    @@:
    cmp ah,178 ;Up
    jne @f
    cmp dword[map.coord_y],0
    je @f
      dec dword[map.coord_y]
      ;сдвигаем сетку тайлов вниз
      stdcall tiles_grid_move_down, tile_00,max_tiles_count,max_tiles_rows
      call but_Refresh
    @@:

    cmp ah,45 ;-
    jne @f
      call but_ZoomM
    @@:
    cmp ah,61 ;+
    jne @f
      call but_ZoomP
    @@:

  .end_f:
  pop ebx eax
  jmp still


align 4
draw_window:
pushad
  mcall 12,1

  xor eax,eax
  mov ebx,20*65536+min_window_w
  mov ecx,20*65536+min_window_h
  mov edx,[sc.work]
  or  edx,0x33000000
  mov edi,hed
  mcall

  mcall 9,procinfo,-1

  cmp dword[procinfo.box.width],min_window_w ; проверяем ширину окна
  jge @f
    mov dword[procinfo.box.width],min_window_w ; если окно очень узкое, увеличиваем ширину для избежания глюков
  @@:

  mov edi,dword[procinfo.box.width]
  sub edi,min_window_w-otst_panel_left
  mov dword[tree1.box_left],edi
  mov dword[tree2.box_left],edi

  mov eax,dword[tree2.box_left] ;двигаем скроллинг
  add eax,dword[tree2.box_width]
  mov ebx,dword[tree2.p_scrol]
  mov word[ebx+2],ax

  mov dword[edit2.left],edi
  add dword[edit2.left],370-otst_panel_left

  stdcall dword[tl_draw],dword tree1
  stdcall dword[tl_draw],dword tree2
  mov dword[wScrMetki.all_redraw],1
  stdcall [scrollbar_ver_draw], dword wScrMetki

  mov eax,8 ;кнопка
  mov ebx,145*65536+20
  mov ecx,5*65536+25
  mov edx,6
  mov esi,[sc.work_button]
  int 0x40

  mov ebx,100*65536+20
  mov ecx,5*65536+25
  mov edx,5
  int 0x40

  mov ebx,170*65536+40 ;кнопка вызова диалога OpenDial
  ;mov ecx,5*65536+25
  mov edx,13
  int 0x40

  mov bx,di
  shl ebx,16
  mov bx,100
  mov ecx,265*65536+25
  mov edx,9
  int 0x40

  ;ebx ...
  mov ecx,235*65536+25
  mov edx,8
  int 0x40

  mov bx,di
  add bx,410-otst_panel_left
  shl ebx,16
  mov bx,55
  ;mov ebx,410*65536+55
  mov ecx,5*65536+25
  mov edx,7
  int 0x40

  mov bx,di
  add bx,440-otst_panel_left
  shl ebx,16
  mov bx,30
  ;mov ebx,440*65536+30
  mov ecx,265*65536+25
  mov edx,12
  int 0x40

  mov bx,di
  add bx,405-otst_panel_left
  shl ebx,16
  mov bx,30
  ;mov ebx,405*65536+30
  ;mov ecx,265*65536+25
  mov edx,11
  int 0x40

  mov bx,di
  add bx,370-otst_panel_left
  shl ebx,16
  mov bx,30
  ;mov ebx,370*65536+30
  ;mov ecx,265*65536+25
  mov edx,10
  int 0x40

  mov eax,4 ;рисование текста
  mov ebx,152*65536+13
  mov ecx,[sc.work_button_text]
  or  ecx,0x80000000
  mov edx,txt_zoom_p
  mcall

  mov ebx,107*65536+13
  mov edx,txt_zoom_m
  int 0x40

  mov bx,di
  add bx,270-otst_panel_left
  shl ebx,16
  mov bx,243
  ;mov ebx,270*65536+243
  mov edx,txt151
  int 0x40

  mov bx,di
  add bx,270-otst_panel_left
  shl ebx,16
  mov bx,273
  ;mov ebx,270*65536+273
  mov edx,txt152
  int 0x40

  mov bx,di
  add bx,415-otst_panel_left
  shl ebx,16
  mov bx,13
  ;mov ebx,415*65536+13
  mov edx,txt_but_refresh
  int 0x40

  mov bx,di
  add bx,380-otst_panel_left
  shl ebx,16
  mov bx,275
  ;mov ebx,380*65536+275
  mov edx,txt_met_up
  int 0x40

  mov bx,di
  add bx,415-otst_panel_left
  shl ebx,16
  mov bx,275
  ;mov ebx,415*65536+275
  mov edx,txt_met_dn
  int 0x40


  mov bx,di
  add bx,450-otst_panel_left
  shl ebx,16
  mov bx,275
  ;mov ebx,450*65536+275
  mov edx,txt_met_sh
  int 0x40

  mov ecx,[sc.work_text]
  or  ecx,0x80000000

  mov ebx,175*65536+13
  mov edx,txt_cache
  int 0x40

  mov bx,di
  ;add bx,450-otst_panel_left
  shl ebx,16
  mov bx,35
  ;mov ebx,265*65536+35
  mov edx,txt141
  int 0x40

  mov bx,135
  ;mov ebx,265*65536+135
  mov edx,txt142
  int 0x40

  call draw_tiles

  stdcall [check_box_draw], dword ch1
  stdcall [check_box_draw], dword ch2
  stdcall [edit_box_draw], edit1
  stdcall [edit_box_draw], edit2

  mcall 12,2
popad
  ret

  head_f_i:
  head_f_l  db 'Системная ошибка',0
  err_message_found_lib0 db 'Не найдена библиотека box_lib.obj',0
  err_message_import0 db 'Ошибка при импорте библиотеки box_lib.obj',0
  err_message_found_lib1 db 'Не найдена библиотека libimg.obj',0
  err_message_import1 db 'Ошибка при импорте библиотеки libimg.obj',0
  err_message_found_lib2 db 'Не найдена библиотека str.obj',0
  err_message_import2 db 'Ошибка при импорте библиотеки str.obj',0
  err_message_found_lib3 db 'Не найдена библиотека libini.obj',0
  err_message_import3 db 'Ошибка при импорте библиотеки libini.obj',0
  err_message_found_lib4 db 'Не найдена библиотека proc_lib.obj',0
  err_message_import4 db 'Ошибка при импорте библиотеки proc_lib.obj',0

system_dir0 db '/sys/lib/'
lib0_name db 'box_lib.obj',0

system_dir1 db '/sys/lib/'
lib1_name db 'libimg.obj',0

system_dir2 db '/sys/lib/'
lib2_name db 'str.obj',0

system_dir3 db '/sys/lib/'
lib3_name db 'libini.obj',0

system_dir4 db '/sys/lib/'
lib4_name db 'proc_lib.obj',0

;library structures
l_libs_start:
  lib0 l_libs lib0_name, sys_path, file_name, system_dir0, err_message_found_lib0, head_f_l, boxlib_import,err_message_import0, head_f_i
  lib1 l_libs lib1_name, sys_path, file_name, system_dir1, err_message_found_lib1, head_f_l, libimg_import, err_message_import1, head_f_i
  lib2 l_libs lib2_name, sys_path, file_name, system_dir2, err_message_found_lib2, head_f_l, strlib_import, err_message_import2, head_f_i
  lib3 l_libs lib3_name, sys_path, file_name, system_dir3, err_message_found_lib3, head_f_l, libini_import, err_message_import3, head_f_i
  lib4 l_libs lib4_name, sys_path, file_name, system_dir4, err_message_found_lib4, head_f_l, proclib_import, err_message_import4, head_f_i
load_lib_end:

align 4
button:
  mcall 17 ;получить код нажатой кнопки
  cmp ah,5
  jne @f
    call but_ZoomM
  @@:
  cmp ah,6
  jne @f
    call but_ZoomP
  @@:
  cmp ah,7
  jne @f
    call but_Refresh
  @@:

  cmp ah,9
  jz  but_MetSave
  cmp ah,8
  jz  but_MetAdd

  cmp ah,10
  jne @f
    call but_met_up
  @@:
  cmp ah,11
  jne @f
    call but_met_dn
  @@:
  cmp ah,12
  jne @f
    call fun_goto_met
  @@:
  cmp ah,13 ;диалог OpenDialog для поиска папки
  jne @f
    call fun_opn_dlg
  @@:
  cmp ah,1
  jne still

.exit:
  push dword[bmp_icon]
  call mem.Free
  stdcall array_tile_function, tile_00,max_tiles_count,tile_destroy

  stdcall dword[tl_data_clear], tree1
  mov dword[tree2.data_img_sys],0 ;чистим указатель на системные иконки,
    ;т. к. они были удалены верхней функцией tl_data_clear
    ;повторный вызов tl_data_clear без чистки указателя вызвет ошибку
  mov dword[tree2.data_img],0 ;чистим указатель на иконки узлов
  stdcall dword[tl_data_clear], tree2

;  stdcall dword[img_destroy], dword[data_icon]
  mcall -1 ;выход из программы


;input:
;data_rgb - pointer to rgb data
;size - count img pixels (size img data / 3(rgb) )
align 4
proc img_rgb_wdiv2 data_rgb:dword, size:dword
  push eax ebx ecx edx
  mov eax,dword[data_rgb]
  mov ecx,dword[size] ;ecx = size
  imul ecx,3
  @@: ;затемнение цвета пикселей
    shr byte[eax],1
    and byte[eax],0x7f
    inc eax
    loop @b

  mov eax,dword[data_rgb]
  mov ecx,dword[size] ;ecx = size
  shr ecx,1
  @@: ;сложение цветов пикселей
    mov ebx,dword[eax+3] ;копируем цвет соседнего пикселя
    add word[eax],bx
    shr ebx,16
    add byte[eax+2],bl

    add eax,6 ;=2*3
    loop @b

  mov eax,dword[data_rgb]
  add eax,3
  mov ebx,eax
  add ebx,3
  mov ecx,dword[size] ;ecx = size
  shr ecx,1
  dec ecx ;лишний пиксель
  @@: ;поджатие пикселей
    mov edx,dword[ebx]
    mov word[eax],dx
    shr edx,16
    mov byte[eax+2],dl

    add eax,3
    add ebx,6
    loop @b

  pop edx ecx ebx eax
  ret
endp

;input:
;data_rgb - pointer to rgb data
;size - count img pixels (size img data / 3(rgb) )
;size_w - width img in pixels
align 4
proc img_rgb_hdiv2, data_rgb:dword, size:dword, size_w:dword
  pushad

  mov eax,dword[data_rgb] ;eax =
  mov ecx,dword[size]	  ;ecx = size
  imul ecx,3
  @@: ;затемнение цвета пикселей
    shr byte[eax],1
    and byte[eax],0x7f
    inc eax
    loop @b

  mov eax,dword[data_rgb] ;eax =
  mov edi,dword[size_w]
  lea esi,[edi+edi*2] ;esi = width*3(rgb)
  mov ebx,esi
  add ebx,eax
  mov ecx,dword[size]  ;ecx = size
  shr ecx,1
  xor edi,edi
  @@: ;сложение цветов пикселей
    mov edx,dword[ebx] ;копируем цвет нижнего пикселя
    add word[eax],dx
    shr edx,16
    add byte[eax+2],dl

    add eax,3
    add ebx,3
    inc edi
    cmp edi,dword[size_w]
    jl .old_line
      add eax,esi
      add ebx,esi
      xor edi,edi
    .old_line:
    loop @b


  mov eax,dword[data_rgb] ;eax =
  add eax,esi ;esi = width*3(rgb)
  mov ebx,esi
  add ebx,eax
  mov ecx,dword[size] ;ecx = size
  shr ecx,1
  sub ecx,dword[size_w] ;лишняя строка пикселей
  xor edi,edi
  @@: ;поджатие пикселей
    mov edx,dword[ebx] ;копируем цвет нижнего пикселя
    mov word[eax],dx
    shr edx,16
    mov byte[eax+2],dl

    add eax,3
    add ebx,3
    inc edi
    cmp edi,dword[size_w]
    jl .old_line_2
      add ebx,esi
      xor edi,edi
    .old_line_2:
    loop @b

  popad
  ret
endp

;input:
;data_rgb - pointer to rgb data
;size - count img pixels (size img data / 3(rgb) )
align 4
proc img_rgb_wmul2, data_rgb:dword, size:dword
  push eax ebx ecx edx
  ;eax - source
  ;ebx - destination
  mov ecx,dword[size] ;ecx = size
  mov eax,ecx
  dec eax
  lea eax,[eax+eax*2] ;eax = (size-1)*3
  mov ebx,eax ;ebx = size*3
  add eax,dword[data_rgb] ;eax = pointer + size*3
  add ebx,eax ;ebx = pointer + 2*size*3
  @@:
    mov edx,dword[eax] ;edx = pixel color
    mov word[ebx],dx
    mov word[ebx+3],dx
    shr edx,16
    mov byte[ebx+2],dl
    mov byte[ebx+3+2],dl
    sub eax,3
    sub ebx,6
    loop @b
  pop edx ecx ebx eax
  ret
endp

;функция для растягивания изображения по высоте в 2 раза
;в указателе data_rgb памяти должно быть в 2 раза больше чем size*3
;иначе при растягивании будет ошибка, выхода на чужую память
;input:
;data_rgb - pointer to rgb data
;size - count img pixels (size img data / 3(rgb) )
;size_w - width img in pixels
align 4
proc img_rgb_hmul2, data_rgb:dword, size:dword, size_w:dword
  pushad

  mov esi,dword[size_w]
  lea esi,[esi+esi*2] ;esi = width * 3(rgb)
  mov eax,dword[size]
  lea eax,[eax+eax*2]
  mov edi,eax
  shl edi,1
  add eax,dword[data_rgb] ;eax = pointer to end pixel (old image) + 1
  add edi,dword[data_rgb] ;edi = pointer to end pixel (new image) + 1
  mov ebx,edi
  sub ebx,esi

  .beg_line:
  mov ecx,dword[size_w]
  @@:
    sub eax,3
    sub ebx,3
    sub edi,3

    mov edx,dword[eax] ;edx = pixel color
    mov word[ebx],dx
    mov word[edi],dx
    shr edx,16
    mov byte[ebx+2],dl
    mov byte[edi+2],dl

    loop @b

  sub ebx,esi
  sub edi,esi

  cmp eax,dword[data_rgb]
  jg .beg_line

  popad
  ret
endp

;input:
;data_rgb - pointer to rgb data
;size - count img pixels (size img data / 3(rgb) )
;size_w - width img in pixels
align 4
proc img_rgb_hoffs, data_rgb:dword, size:dword, size_w:dword, hoffs:dword
  push eax ebx ecx edx esi

  mov esi,dword[size_w]
  lea esi,[esi+esi*2] ;esi = width * 3(rgb)
  imul esi,dword[hoffs]

  mov eax,dword[size]
  lea eax,[eax+eax*2]
  add eax,dword[data_rgb] ;eax = pointer to end pixel + 1
  sub eax,3
  mov ebx,eax
  add ebx,esi

  mov ecx,dword[size]
  dec ecx
  @@:
    mov edx,dword[eax] ;edx = pixel color
    mov word[ebx],dx
    shr edx,16
    mov byte[ebx+2],dl

    sub eax,3
    sub ebx,3
    loop @b
  pop esi edx ecx ebx eax
  ret
endp


;input:
;data_rgb - pointer to rgb data
;size_w_old - width img in pixels
;size_w_new - new width img in pixels
;size_h - height img in pixels
align 4
proc img_rgb_wcrop, data_rgb:dword, size_w_old:dword, size_w_new:dword, size_h:dword
  pushad
    mov eax, dword[size_w_old]
    lea eax, dword[eax+eax*2] ;eax = width(old) * 3(rgb)
    mov ebx, dword[size_w_new]
    lea ebx, dword[ebx+ebx*2] ;ebx = width(new) * 3(rgb)
    mov edx, dword[size_h]
    ;dec edx
    mov edi, dword[data_rgb] ;edi - получает данные
    mov esi, edi
    add edi, ebx
    add esi, eax
    cld
  @@:
    dec edx ;уменьшаем счетчик оставшихся строк на 1
    cmp edx,0
    jle @f

    mov ecx, ebx
    rep movsb ;перенос (копирование) строки пикселей
;stdcall mem_copy,esi,edi,ebx

    add esi,eax ;переход на новую строчку изображения
    sub esi,ebx
;add esi,eax
;add edi,ebx
    jmp @b
  @@:

  popad
  ret
endp

align 4
proc mem_copy, source:dword, destination:dword, len:dword
  push ecx esi edi
    cld
    mov esi, dword[source]
    mov edi, dword[destination]
    mov ecx, dword[len]
    rep movsb
  pop edi esi ecx
  ret
endp

align 4
proc mem_clear, mem:dword, len:dword
  push eax ecx edi
    cld
    xor al,al
    mov edi, dword[mem]
    mov ecx, dword[len]
    repne stosb
  pop edi ecx eax
  ret
endp

align 4
fun_opn_dlg: ;функция для вызова OpenFile диалога
	pushad
	copy_path open_dialog_name,communication_area_default_path,file_name,0
	mov [OpenDialog_data.type],2
	mov dword[plugin_path],0 ;что-бы при открытии диалогового окна путь всегда брался из OpenDialog_data.dir_default_path

	stdcall [OpenDialog_Start],OpenDialog_data
	cmp [OpenDialog_data.status],2
	je @f
		mov esi,[OpenDialog_data.openfile_path]
		stdcall [str_len],dword[edit1.text],dword[edit1.max]
		mov [edit1.size],eax
		mov [edit1.pos],eax
		stdcall [edit_box_draw], edit1
	@@:
	popad
	ret

  txt_met_up db 24,0
  txt_met_dn db 25,0
  txt_met_sh db '*',0
  txt_zoom_m db '-',0
  txt_zoom_p db '+',0
  txt151 db 'Добавить метку',0
  txt152 db 'Сохранить метки',0
  txt_but_refresh db 'Обновить',0
  txt_cache db 'Cache:',0
  txt141 db 'Вид карты',0
  txt142 db 'Выбор метки',0

; check_boxes
ch1 check_box 5,  5, 6, 12, 0xffffd0, 0x800000, 0, ch_text1, 12,ch_flag_en
ch2 check_box 5, 20, 6, 12, 0xffffd0, 0x800000, 0, ch_text2, 11,ch_flag_en
checkboxes_end:

ch_text1 db 'брать сверху'
ch_text2 db 'брать снизу'

edit1 edit_box 190, 215,  10, 0xd0ffff, 0xff, 0x80ff, 0, 0xa000, 4090, openfile_path, mouse_dd, 0
edit2 edit_box 100, 370, 240, 0xd0ffff, 0xff, 0x80ff, 0, 0xa000,  30, ed_buffer.2, mouse_dd, 0
editboxes_end:

tree1 tree_list 10,10, tl_list_box_mode+tl_key_no_edit, 16,16,\
    0x8080ff,0x0000ff,0xffffff, 265,45,90,85, 0,0,0,\
    el_focus, 0,fun_new_map_type
tree2 tree_list 32,300, tl_draw_par_line, 16,16,\
    0x8080ff,0x0000ff,0xffffff, 265,145,190,85, 0,12,0,\
    el_focus, wScrMetki,fun_goto_met

align 4
wScrMetki:
.x:
.size_x     dw 16 ;+0
.start_x    dw 0 ;+2
.y:
.size_y     dw 100 ;+4
.start_y    dw 0 ;+6
.btn_high   dd 15 ;+8
.type	    dd 1  ;+12
.max_area   dd 100  ;+16
.cur_area   dd 30  ;+20
.position   dd 0  ;+24
.bckg_col   dd 0xeeeeee ;+28
.frnt_col   dd 0xbbddff ;+32
.line_col   dd 0  ;+36
.redraw     dd 0  ;+40
.delta	    dw 0  ;+44
.delta2     dw 0  ;+46
.run_x:
.r_size_x   dw 0  ;+48
.r_start_x  dw 0  ;+50
.run_y:
.r_size_y   dw 0 ;+52
.r_start_y  dw 0 ;+54
.m_pos	    dd 0 ;+56
.m_pos_2    dd 0 ;+60
.m_keys     dd 0 ;+64
.run_size   dd 0 ;+68
.position2  dd 0 ;+72
.work_size  dd 0 ;+76
.all_redraw dd 0 ;+80
.ar_offset  dd 1 ;+84

ed_buffer: ;ЄхъёЄ фы  edit
  .2: rb 32

el_focus dd tree1

tree_sys_icon dd 0
tree_nod_icon dd 0

bmp_icon   dd 0 ;память для загрузки изображения
data_icon  dd 0 ;память для преобразования картинки функциями libimg

run_file_70 FileInfoBlock


txt_tile_path db 'tile path',0
  rb 300
txt_tile_type dd txt_tile_type_0 ;указатель на выбранный тип файлов
txt_tile_type_0 db 0
  rb 10

;---------------------------------------------------------------------
align 4
OpenDialog_data:
.type			dd 2
.procinfo		dd procinfo	;+4
.com_area_name		dd communication_area_name	;+8
.com_area		dd 0	;+12
.opendir_path		dd plugin_path	;+16
.dir_default_path	dd default_dir ;+20
.start_path		dd file_name ;+24 путь к диалогу открытия файлов
.draw_window		dd draw_window	;+28
.status 		dd 0	;+32
.openfile_path		dd openfile_path	;+36 путь к открываемому файлу
.filename_area		dd filename_area	;+40
.filter_area		dd Filter
.x:
.x_size 		dw 420 ;+48 ; Window X size
.x_start		dw 10 ;+50 ; Window X position
.y:
.y_size 		dw 320 ;+52 ; Window y size
.y_start		dw 10 ;+54 ; Window Y position

default_dir db '/rd/1',0 ;директория по умолчанию

communication_area_name:
	db 'FFFFFFFF_open_dialog',0
open_dialog_name:
	db 'opendial',0
communication_area_default_path:
	db '/rd/1/File managers/',0

Filter:
dd Filter.end - Filter.1
.1:
db 'TXT',0
.end:
db 0

align 4
map: ;координаты карты
  .coord_x dd 0 ;координата x
  .coord_y dd 0 ;координата y
  .zoom    db 1 ;масштаб

align 4
tile_00 rb size_tile_struc * max_tiles_count

;этот код не мой, он преобразует число в строку
;input:
; eax = value
; edi = string buffer
;output:
; edi =
align 4
tl_convert_to_str:
  pushad
    mov dword[edi+1],0;0x20202020
    call .str
  popad
  ret

align 4
.str:
  mov ecx,0x0a ;задается система счисления изменяются регистры ebx,eax,ecx,edx входные параметры eax - число
    ;преревод числа в ASCII строку взодные данные ecx=система счисленя edi адрес куда записывать, будем строку, причем конец переменной 
  cmp eax,ecx  ;сравнить если в eax меньше чем в ecx то перейти на @@-1 т.е. на pop eax
  jb @f
  xor edx,edx  ;очистить edx
  div ecx      ;разделить - остаток в edx
  push edx     ;положить в стек
  ;dec edi             ;смещение необходимое для записи с конца строки
  call .str;перейти на саму себя т.е. вызвать саму себя и так до того момента пока в eax не станет меньше чем в ecx
  pop eax
  @@: ;cmp al,10 ;проверить не меньше ли значение в al чем 10 (для системы счисленя 10 данная команда - лишная))
  or al,0x30  ;данная команда короче  чем две выше
  stosb       ;записать элемент из регистра al в ячеку памяти es:edi
  ret	      ;вернуться чень интересный ход т.к. пока в стеке храниться кол-во вызовов то столько раз мы и будем вызываться


hed db 'Planet viewer 24.08.10',0 ;подпись окна

sc system_colors  ;системные цвета
mouse_dd dd 0 ;нужно для Shift-а в editbox
i_end:
	rb 1024
	align 16
	procinfo process_information
stacktop:
	sys_path rb 4096
	file_name:
		rb 4096
	plugin_path:
		rb 4096
	openfile_path:
		rb 4096
	filename_area:
		rb 256
mem:

