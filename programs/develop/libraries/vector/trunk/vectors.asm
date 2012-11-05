format MS COFF
public EXPORTS
section '.flat' code readable align 16

include 'vectors.inc' ;vectors functions constant

macro swap v1, v2 {
  push v1
  push v2
  pop v1
  pop v2
}

BUF_STRUCT_SIZE equ 17 ;размер структуры, описывающей буфер
BUF_MAX_COUNT equ 8 ;максимальное число буферов

fun_draw_pixel dd drawpixel_scrn ;указатель на функцию рисования точки
active_buffer: ;начало структуры активного буфера (буфера в который делается рисование фигур или текста)
  dd 0 ;указатель на буфер изображения
  dw 0 ; +4 left
  dw 0 ; +6 top
  dw 0 ; +8 w ширина буфера
  dw 0 ;+10 h высота буфера
  dd 0 ;+12 color цвет фона
  db 24 ;+16 bit in pixel глубина цвета (в данной версии еще не используется)
rb BUF_STRUCT_SIZE * BUF_MAX_COUNT ;резервируем место для структур, описывающих буфера
;в активном буфере будет содержаться точная копия одной из этих структур


active_buffer_left  equ active_buffer+ 4
active_buffer_top   equ active_buffer+ 6
active_buffer_w     equ active_buffer+ 8
active_buffer_h     equ active_buffer+10
active_buffer_color equ active_buffer+12


;-----------------------------------------------------------------------------
;функция для выделения памяти
;input:
; ecx = size data
;otput:
; eax = pointer to memory
mem_Alloc:
  push ebx
  mov eax,68
  mov ebx,12
  int 0x40
  pop ebx
  ret
;-----------------------------------------------------------------------------
;функция для освобождения памяти
;input:
; ecx = pointer to memory
mem_Free:
  push eax ebx
  cmp ecx,0
  jz @f
    mov eax,68
    mov ebx,13
    int 0x40
  @@:
  pop ebx eax
  ret

;функция рисующая точку сразу на экран (без участия буфера)
align 4
drawpixel_scrn:
  bt bx,15
  jc @f
  bt cx,15
  jc @f
;  cmp bx,300
;  jge @f
;  cmp cx,300
;  jge @f
  int 0x40
  @@:
  ret

;функция рисующая точку в активном буфере
align 4
drawpixel_buf:
  bt bx,15 ;проверяем знак числа, если координата меньше 0
  jc @f ;тогда точка в экран не попала, конец функции
  bt cx,15
  jc @f
  cmp bx,word[active_buffer_w] ;проверяем координату точки, если больше ширины буфера
  jge @f ;тогда точка в экран не попала, конец функции
  cmp cx,word[active_buffer_h]
  jge @f

  push esi
  xor esi,esi ;тут будет указатель на пиксель из активного буфера
  mov si,word[active_buffer_w] ;ширина буфера по оси x
  imul esi,ecx ;size_x*y
  add esi,ebx  ;size_x*y+x
  lea esi,[esi+esi*2] ;(size_x*y+x)*3
  add esi,dword[active_buffer] ;ptr+(size_x*y+x)*3

  mov word[esi],dx ;копируем зеленый и синий спектр
  ror edx,16 ;крутим цвет на 2 байта
  mov byte[esi+2],dl ;копируем красный спектр
  ror edx,16 ;крутим цвет назад
  pop esi
  @@:
  ret

;функция создающая новый буфер, принимает параметры нужные для структуры
;input:
; [esp+8] = bit on pixel, index created buffer
; [esp+10] = color
; [esp+14] = size: w,h
; [esp+18] = size: l,t
align 4
buf_create:
  push ebp
  mov ebp,esp
  cmp byte[ebp+8],1 ;проверка правильности индекса создаваемого буфера
  jl .error_ind ;пользователь указал индекс меньше 1-цы, ... error :(
  cmp byte[ebp+8],BUF_MAX_COUNT ;проверка правильности индекса создаваемого буфера
  jg .error_ind ;пользователь указал индекс больше максимально возможного, ... error :(
  push eax ecx edi esi
    mov eax,dword[ebp+14] ;берем ширину и высоту буфера
    ror eax,16 ;меняем местами ширину и высоту, так будет удобнее при выводе для функции 7
    mov dword[active_buffer_w],eax ;помещаем значения в структуру активного буфера

    xor ecx,ecx ;тут вычисляем сколько памяти нужно для этого буфера
    mov cx,ax ;берем нижний размер буфера
    shr eax,16 ;затираем нижний размер, в eax теперь только верхний размер, на месте нижнего
    imul ecx,eax ;умножаем высоту на ширину (может и наоборот ширину на высоту)
    imul ecx,3 ; 24 bit = 3, 32 bit = 4 ... work if only 24
    call mem_Alloc ;просим память из системы
    mov dword[active_buffer],eax ;копируем указатель на полученую память в структуру активного буфера

    mov eax,dword[ebp+18] ;берем отступы слева и справа
    ror eax,16
    mov dword[active_buffer_left],eax

    mov eax,dword[ebp+10] ;get color - берем цвет для фона
    mov dword[active_buffer_color],eax

    ;копируем всю структуру активного буфера, в память отведенную для буферов
    ;иначе если появится новый буфер, то он затрет собой активную структуру
    ;потому нужно дублировать эту информацию
    mov di,word[ebp+8] ;copy buffer struct
    and edi,0xff ;index <= 255
    mov ecx,BUF_STRUCT_SIZE ;в ecx размер копируемых данных
    imul edi,ecx
    mov esi,active_buffer
    add edi,esi
    rep movsb ;повторяем копирование по байту, пока ecx станет не равно 0

    push word[ebp+8] ;при создании буфера, в нем может быть мусор,
    call buf_clear ;потому чистим его фоновым цветом
  pop esi edi ecx eax
  .error_ind:
  pop ebp
  ret 14

;функция установки активного буфера, если на входе 0 - то включается режим рисования на экран без буфера
;input:
; [esp+8] = index buffer (0-screen)
align 4
set_active_buf:
  push ebp
  mov ebp,esp

    cmp word[ebp+8],0
    jne @f
      .to_scrn:
      mov dword[fun_draw_pixel],drawpixel_scrn ;рисование в экран
      jmp .end_fun
    @@:
      cmp byte[ebp+8],BUF_MAX_COUNT ;if buffer index out of range
      jg .to_scrn
      mov dword[fun_draw_pixel],drawpixel_buf ;рисование в буфер
push ecx esi edi
	  mov si,word[ebp+8] ;copy buffer struct
	  and esi,0xff ;index <= 255
	  mov ecx,BUF_STRUCT_SIZE
	  imul esi,ecx
	  mov edi,active_buffer
	  add esi,edi
      rep movsb
pop edi esi ecx
      cmp dword[active_buffer],0 ;if buffer is empty
      je .to_scrn
  .end_fun:
  pop ebp
  ret 2

;функция очистки буфера фоновым цветом
;input:
; [esp+8] = index buffer (0-screen)
align 4
buf_clear:
  push ebp
  mov ebp,esp
  push eax ebx ecx edi

    mov di,word[ebp+8] ;get pointer to buffer struct
    and edi,0xff ;index <= 255
    imul edi,BUF_STRUCT_SIZE
    add edi,active_buffer ;edi = pointer to buffer struct

  cmp dword[edi],0 ;проверяем пустой указатель на буфер или нет
  je .no_draw ;если пустой, то выход
    xor ecx,ecx ;тут будет размер буфера в пикселях
    mov cx,word[edi+8] ;active_buffer_w]
    xor eax,eax
    mov ax,word[edi+10] ;active_buffer_h]
    imul ecx,eax ;ecx=x*y
    mov ebx,dword[edi+12] ;active_buffer_color]
    mov ax,bx
    shr ebx,16
    ;imul ecx,3
    ;rep stosb
    push dword[edi] ;save value in pointer
    pop edi ;get value in pointer
    @@:
      mov word[edi],ax
      add edi,2
      mov byte[edi],bl
      inc edi
      loop @b
  .no_draw:
  pop edi ecx ebx eax
  pop ebp
  ret 2

;функция рисующая содержимое буфера на экране, использует КОС функцию номер 7
;input:
; [esp+8] = index buffer (0-screen)
align 4
draw_buf:
  push ebp
  mov ebp,esp

    mov di,word[ebp+8] ;get pointer to buffer struct
    and edi,0xff ;index <= 255
    imul edi,BUF_STRUCT_SIZE
    add edi,active_buffer ;edi = pointer to buffer struct

      mov eax,7
      mov ebx,dword[edi] ;active_buffer]
      mov ecx,dword[edi+8] ;active_buffer_w] ;ecx = w*0xffff+h
      ror ecx,16

      ;push word[edi+4] ;active_buffer_left] ;загрузка точки левого верхнего угла
      ;pop dx
      mov dx,word[edi+4]
      shl edx,16
      ;push word[edi+6] ;active_buffer_top] ;загрузка точки левого верхнего угла
      ;pop dx
      mov dx,word[edi+6]
      int 0x40

  pop ebp
  ret 2

;функция очищающая память, занимаемую буфером
;input:
; [esp+8] = index buffer (0-screen)
align 4
buf_delete:
  push ebp
  mov ebp,esp

    mov cx,word[ebp+8] ;get pointer to buffer struct
    and ecx,0xff ;index <= 255
    imul ecx,BUF_STRUCT_SIZE
    add ecx,active_buffer ;edi = pointer to buffer struct

    push dword[ecx] ;save value in pointer
    pop ecx ;get value in pointer  
    call mem_Free

  pop ebp
  ret 2


;функция рисующая линию
;input:
; [esp+8] = p0
; [esp+12] = p1
; [esp+16] = color
loc_0 equ byte[ebp-4]
loc_1 equ word[ebp-6]
loc_2 equ word[ebp-8]
align 4
line_brs:
  push ebp
  mov ebp,esp
  sub esp,6 ;=1+2*2
    pushad ;eax ebx ecx edx si di
    mov edx,dword[ebp+16]
;---
    mov ax,word[ebp+14] ;y1
;    cmp ax,0 ;if y1<0 return
;    jl .coord_end
;    cmp word[ebp+10],0 ;if y0<0 return
;    jl .coord_end
    sub ax,word[ebp+10] ;y1-y0
    bt ax,15
    jae @f
      neg ax
      inc ax
    @@:
    mov bx,word[ebp+12] ;x1
;    cmp bx,0 ;if x1<0 return
;    jl .coord_end
;    cmp word[ebp+8],0 ;if x0<0 return
;    jl .coord_end
    sub bx,word[ebp+8] ;x1-x0
    bt bx,15
    jae @f
      neg bx
      inc bx
    @@:

    mov byte[ebp-4],byte 0 ;bool steep=false
    cmp ax,bx
    jle @f
      mov byte[ebp-4],byte 1 ;bool steep=true
      swap word[ebp+8],word[ebp+10] ;swap(x0, y0);
      swap word[ebp+12],word[ebp+14] ;swap(x1, y1);
    @@:
    mov ax,word[ebp+8] ;x0
    cmp ax,word[ebp+12] ;if(x0>x1)
    jle @f
      swap word[ebp+8],word[ebp+12] ;swap(x0, x1);
      swap word[ebp+10],word[ebp+14] ;swap(y0, y1);
    @@:

;  int deltax  si
;  int deltay  di
;  int error  ebp-6
;  int ystep  ebp-8

    mov ax,word[ebp+8] ;x=x0
    mov si,word[ebp+12] ;x1
    sub si,ax ;deltax = x1-x0
    mov bx,si
    shr bx,1
    mov loc_1,bx ;error = deltax/2

    mov ax,word[ebp+10] ;y=y0
    mov di,word[ebp+14] ;y1
      mov loc_2,word -1 ;ystep = -1
      cmp ax,di ;if (y0<y1) ystep = 1;
      jge @f
	mov loc_2,word 1 ;ystep = 1
      @@:
    sub di,ax ;y1-y0

    bts di,15
    jae @f
      neg di
      inc di
    @@:
    and di,0x7fff ;deltay = abs(y1-y0)

    mov eax,1 ;function, draw point
    xor ebx,ebx
    xor ecx,ecx

    cmp byte[ebp-4],0
    jne .coord_yx
      mov bx,word[ebp+10] ;y0
      mov cx,word[ebp+8]  ;x0

    @@: ;for (x=x0 ; x<x1; x++) ;------------------------------------
      cmp cx,word[ebp+12]
      jg @f ;jge ???
      call dword[fun_draw_pixel]

      sub loc_1,di ;error -= deltay
      cmp loc_1,0 ;if(error<0)
      jge .if0
	add bx,loc_2 ;y += ystep
	add loc_1,si ;error += deltax
      .if0:
      inc cx
      jmp @b
    @@:

      jmp .coord_end
    .coord_yx:
      mov bx,word[ebp+8]  ;x0
      mov cx,word[ebp+10] ;y0

    @@: ;for (x=x0 ; x<x1; x++) ;------------------------------------
      cmp bx,word[ebp+12]
      jg @f ;jge ???
      call dword[fun_draw_pixel]

      sub loc_1,di ;error -= deltay
      cmp loc_1,0 ;if(error<0)
      jge .if1
	add cx,loc_2 ;y += ystep
	add loc_1,si ;error += deltax
      .if1:
      inc bx
      jmp @b
    @@:

    .coord_end:
;---
    popad
  mov esp,ebp ; восстанавливаем стек
  pop ebp
  ret 12


;input:
; [esp+8] = p0
; [esp+12] = p1
; [esp+16] = p2
; [esp+20] = color
align 4
cruve_bezier:
  push ebp
  mov ebp,esp

  pushad

;float t, xt,yt;
;for(t=.0;t<1.;t+=.005){
;  xt=pow(1.-t,2)*x0+2*t*(1.-t)*x1+pow(t,2)*x2;
;  yt=pow(1.-t,2)*y0+2*t*(1.-t)*y1+pow(t,2)*y2;
;  dc.SetPixel(xt,yt,255L);
;}
  .beg_fun: ;для входа из другой функции


  mov edx,dword[ebp+20] ;set cruve color
  xor ebx,ebx
  xor ecx,ecx

  finit

  ; calculate delta t - вычисление шага изменения параметра t для рисования кривой Безье
  push dword[ebp+8]
  push dword[ebp+12]
  call line_len4i ;определяем длину отрезка p0p1
  fld dword[o_len]
  push dword[ebp+12]
  push dword[ebp+16]
  call line_len4i ;определяем длину отрезка p1p2
  fadd dword[o_len] ;находим сумарную длину (p0p1 + p1p2)
  fadd st0,st0 ; умножаем длинну (p0p1 + p1p2) на 2
  ftst
  fstsw ax

  fld1
  sahf
  jle @f ;избегаем деления на 0
    fdiv st0,st1 ;находим шаг для изменения параметра t по формуле 1 / (2 * (p0p1 + p1p2))
    ; т.к. прямая в некоторых случаях "рвется", то я думаю что данная формула не оптимальна,
    ; но ничего лучшего я пока не придумал, ... :(
  @@:
  fstp dword[delt_t]

  finit

  ;fild word[ebp+18] ;y2
  fild word[ebp+14] ;y1
  fild word[ebp+10] ;y0
  fild word[ebp+16] ;x2
  fild word[ebp+12] ;x1
  fild word[ebp+8] ;x0
  fld dword[delt_t]
  fldz ;t=.0

  @@:
  fld1
  fsub st0,st1 ;1.-t
  fmul st0,st0 ;pow(1.-t,2)
  fmul st0,st3 ;...*x0
  fstp dword[opr_param]

  fld1
  fsub st0,st1 ;1.-t
  fmul st0,st1 ;(1.-t)*t
  fadd st0,st0
  fmul st0,st4 ;...*x1
  mov edi,dword[opr_param]
  fstp dword[opr_param]

  fldz
  fadd st0,st1 ;0+t
  fmul st0,st0 ;t*t
  fmul st0,st5 ;...*x2

    fadd dword[opr_param]
    mov dword[opr_param],edi
    fadd dword[opr_param]
    fistp word[v_poi_0] ;x

  fld1
  fsub st0,st1 ;1.-t
  fmul st0,st0 ;pow(1.-t,2)
  fmul st0,st6 ;...*y0
  fstp dword[opr_param]

  fld1
  fsub st0,st1 ;1.-t
  fmul st0,st1 ;(1.-t)*t
  fadd st0,st0
  fmul st0,st7 ;...*y1
  mov edi,dword[opr_param]
  fstp dword[opr_param]

  fldz
  fadd st0,st1 ;0+t
  fmul st0,st0 ;t*t
  fimul word[ebp+18] ;...*y2

    fadd dword[opr_param]
    mov dword[opr_param],edi
    fadd dword[opr_param]
    fistp word[v_poi_0+2] ;y

  mov eax,1
  mov bx,word[v_poi_0+2]
  mov cx,word[v_poi_0]
  call dword[fun_draw_pixel]

  fadd st0,st1 ;t+dt

  fld1
  fcomp
  fstsw ax
  sahf
  jae @b

  .end_draw:
;  btr word[opt_bez],0 ;снимаем флаг рисования прямой линии с кривой Безье
;  btr word[opt_bez],1
  ;and word[opt_bez],0xfffc
  popad

  mov esp,ebp
  pop ebp
  ret 16

delt_t dd 0.05 ;шаг для параметра t из кривой Безье

;функция рисующая сегмент кривуй Безье по 3-м точкам, при этом координаты
; 1-й и 3-й точки смещаются ко 2-й точке, позволяя рисовать длинную кривую из нескольких кусков
;input:
; [esp+8] = p0
; [esp+12] = p1
; [esp+16] = p2
; [esp+20] = color
align 4
cruve_bezier_del2:
;  btr word[opt_bez],1 ;test
;  ret 16              ;test
  push ebp
  mov ebp,esp

  pushad
;jmp cruve_bezier.end_draw

    bt word[opt_bez],1 ;проверяем флаг рисования отрезка для 3-й точки (p2)
    jae @f
      push dword[ebp+20] ;line color
      push dword[ebp+8]
    @@:

  ;********* высчитывание усредненных координат *********
mov ax,word[ebp+8] ;x0
add ax,word[ebp+12]
shr ax,1
bt ax,14
jae @f
  or ax,0x8000
@@:
mov word[ebp+8],ax

mov ax,word[ebp+10] ;y0
add ax,word[ebp+14]
shr ax,1
bt ax,14
jae @f
  or ax,0x8000
@@:
mov word[ebp+10],ax

    btr word[opt_bez],1 ;проверяем флаг рисования отрезка для 3-й точки (p2)
    jae @f
      push dword[ebp+8]
      call line_brs ;рисуем прямой отрезок
    @@:
    bt word[opt_bez],0 ;проверяем флаг рисования отрезка для 1-й точки (p0)
    jae @f
      push dword[ebp+20] ;line color
      push dword[ebp+16]
    @@:

mov ax,word[ebp+16] ;x2
add ax,word[ebp+12]
shr ax,1
bt ax,14
jae @f
  or ax,0x8000
@@:
mov word[ebp+16],ax

mov ax,word[ebp+18] ;y2
add ax,word[ebp+14]
shr ax,1
bt ax,14
jae @f
  or ax,0x8000
@@:
mov word[ebp+18],ax

    btr word[opt_bez],0 ;проверяем флаг рисования отрезка для 1-й точки (p0)
    jae @f
      push dword[ebp+16]
      call line_brs ;рисуем прямой отрезок
    @@:

;jmp cruve_bezier.end_draw
  ;********* переход на основную функцию *********
  jmp cruve_bezier.beg_fun


;функция рисующая текст
;input:
; dword[ebp+8] = pointer to vector font data
; dword[ebp+12] = pointer to text param struct (color, x,y, angle, scale, ...)
; dword[ebp+16] = text string (0 - end string)
align 4
draw_text:
  push ebp
  mov ebp,esp
  pushad
    mov eax,dword[ebp+8]
    mov ebx,dword[ebp+12]
    mov edx,dword[ebp+16]

    mov esi,ebx
    add esi,4 ;skeep color
    mov edi,text_point
    mov ecx,12 ;(x+y+a)*3
    rep movsb ;copy base point

    finit
    fild word[ebx+12+4]
    fdiv dword[eax+4] ;sumbol height
    fstp dword[text_point.s]

    mov edi,dword[ebx];get color
    xor esi,esi ;line number
;------------------------
    @@:
      cmp byte[edx],0
      je @f
      xor ecx,ecx
      mov cl,byte[edx]
      shl cx,2 ;cx*=4
      add cx,32
      add ecx,eax

;mov ecx,eax
;add ecx,32+256*4+4

      push edi ;color
      push dword[ecx] ;copy sumbol pointer
      pop ecx
      add ecx,eax ;добавляем к ссылке на символ смещение начала самого шрифта
      push dword ecx
      push dword text_point ;output point
      call draw_poly_line

      cmp byte[edx],13
      je .new_line
	finit
	fld dword[eax] ;sumbol width
	fmul dword[text_point.s]

  fld dword[text_point.a]
  fcos
  fmul st0,st1
  fadd dword[text_point.x]
  fstp dword[text_point.x]

	fld dword[text_point.a]
	fsin
	fmul st0,st1
	fadd dword[text_point.y]
	fstp dword[text_point.y]
inc edx ;move next sumbol
	jmp @b
      .new_line:
	push edi esi
	  mov esi,dword[ebp+12]
	  add esi,4 ;skeep color
	  mov edi,text_point
	  mov ecx,8 ;(x+y)*4
	  rep movsb ;restore base point
	pop esi edi

	inc esi
	mov dword[opr_param],esi

	finit
	fld dword[eax+4] ;sumbol height
	fmul dword[text_point.s]
	fimul dword[opr_param]

	fld1
	fld1
	fadd st0,st1
	fldpi
	fdiv st0,st1
	fadd dword[text_point.a]
	fcos
;        fld dword[eax+4] ;sumbol height
;        fmul dword[text_point.s]
	fmul st0,st3
	fadd dword[text_point.x]
	fstp dword[text_point.x]

	fld1
	fld1
	fadd st0,st1
	fldpi
	fdiv st0,st1
	fadd dword[text_point.a]
	fsin
;        fld dword[eax+4] ;sumbol height
;        fmul dword[text_point.s]
	fmul st0,st5
	fadd dword[text_point.y]
	fstp dword[text_point.y]
inc edx ;move next sumbol
      jmp @b
    @@:
  popad
  pop ebp
  ret 12

text_point: ;точка для вывода текста
  .x dd 0.0
  .y dd 0.0
  .a dd 0.0 ;angle
  .s dd 1.0 ;scale


;функция для конвертирования координат из декартовой системы координат в полярную
;input:
; dword[ebp+8] = pointer to contur
align 4
convert_contur:
  push ebp
  mov ebp,esp

  push eax ebx ecx
    mov ebx,dword[ebp+8]
    finit
    .new_contur:
    mov cx,word[ebx]
    or word[ebx],VECT_POINTS_IS_POLAR

    add ebx,2
    btr cx,15 ;VECT_PARAM_COLOR
    jae @f
      add ebx,4
    @@:
    btr cx,14 ;VECT_PARAM_PROP_L
    jae @f
      add ebx,4
    @@:
    btr cx,12 ;проверка конвертированных
    jae @f
      and ecx,0xfff
    cmp cx,0
    je .end_contur
      shl ecx,3
      add ebx,ecx
      jmp .new_contur
    @@:
    and cx,0xfff
    cmp cx,0
    je .end_contur
    mov eax,opr_param

    @@:
      cmp cx,0
      je .new_contur
      dec cx
	fld dword[ebx]
	fistp word[eax]
	fld dword[ebx+4]
	fistp word[eax+2]

	push dword[eax]
	call opred2i
	push dword[o_ang]
	pop dword[ebx] ;x(n)
	push dword[o_len]
	pop dword[ebx+4] ;y(n)

	add ebx,8 ;move next coord
      jmp @b
    .end_contur:
  pop ecx ebx eax ebp
  ret 4

;функция для рисования контуров, состоящих из разных наборов точек (прямые, Безье)
;input:
; [esp+8] = x0,y0,a0,s0 - параметры центральной точки: координаты, угол поворота, масштаб
; [esp+12] = contur - контур, заданный координатами точек
; [esp+16] = color - первоначальный цвет
align 4
draw_poly_line:
  push ebp
  mov ebp,esp
  pushad
    mov eax,dword[ebp+8]
    mov ebx,dword[ebp+12]
    mov edx,dword[ebp+16]

    bt word[ebx],12 ;VECT_POINTS_IS_POLAR
    jc @f
      push dword ebx
      call convert_contur
    @@:

    mov word[opt_bez],0 ;clear draw options

    finit
    fld dword[eax]  ;x0 (st4)
    fld dword[eax+4];y0 (st3)
    fld dword[eax+8];a0 (st2)
    fld dword[eax+12];s0(st1)

    mov si,0 ;сплошная линия, без прерываний

    .new_contur:

    mov cx,word[ebx]
    mov edi,ebx ;резервное сохранение начала контура

    add ebx,2
    btr cx,15 ;VECT_PARAM_COLOR
    jae @f
      mov edx,dword[ebx]
      add ebx,4
    @@:
    btr cx,14 ;VECT_PARAM_PROP_L
    jae @f
      mov esi,dword[ebx] ;получаем новые параметры линии
      add ebx,4
      btr si,9 ;VECT_CONT_BEZIER = 0x200
      jc .BezB ;дальше контур Безье, не линейный
      and si,0xff
    @@:

    jmp .BezE
    .BezB: ;пропуск всех точек Безье
      bt word[opt_bez],2
      jc .after_bez_draw
      or word[opt_bez],4

      push edx
      push edi ;начало контура - ebx
      push dword[ebp+8] ;eax
      call draw_poly_bezier
      .after_bez_draw:
      and ecx,0xfff
      cmp ecx,0
      je .end_contur

      shl ecx,3
      add ebx,ecx
      jmp .new_contur
    .BezE:

    and cx,0xfff
    cmp cx,0
    je .end_contur

    mov di,cx
    sub di,si

      fld dword[ebx] ;st0=a(n)
      fadd st0,st2
      fcos
      fmul dword[ebx+4] ;l(n)
      fmul st0,st1 ;*=scale
      fadd st0,st4
      fistp word[v_poi_1+2] ;x(n)

      fld dword[ebx] ;st0=a(n)
      fadd st0,st2
      fsin
      fmul dword[ebx+4] ;l(n)
      fmul st0,st1 ;*=scale
      fadd st0,st3
      fistp word[v_poi_1] ;y(n)
    dec cx
    add ebx,8 ;move next coord

    @@: ;---------------------------------------------------------------
      push dword[v_poi_1]
      pop dword[v_poi_0]
;      call draw_vect_point

      cmp cx,0
      je .new_contur ;во избежание зацикливания

      fld dword[ebx] ;st0=a(n)
      fadd st0,st2
      fcos
      fmul dword[ebx+4] ;l(n)
      fmul st0,st1 ;*=scale
      fadd st0,st4
      fistp word[v_poi_1+2] ;x(n)

      fld dword[ebx] ;st0=a(n)
      fadd st0,st2
      fsin
      fmul dword[ebx+4] ;l(n)
      fmul st0,st1 ;*=scale
      fadd st0,st3
      fistp word[v_poi_1] ;y(n)
      add ebx,8 ;move next coord

      cmp cx,di
      je .end_draw_line

	push dword edx ;line color
	push dword[v_poi_0]
	push dword[v_poi_1]
	call line_brs

      loop @b
      jmp .new_contur

      .end_draw_line: ;-------------------------------------------------------------
      sub di,si
      loop @b
      jmp .new_contur

    .end_contur:
  popad
  pop ebp
  ret 12

;функция рисующая полигоны состоящие из кривых Безье
;input:
; [esp+8] = x0,y0,a0,...
; [esp+12] = contur
; [esp+16] = color
align 4
draw_poly_bezier:
  push ebp
  mov ebp,esp
  pushad
    mov eax,dword[ebp+8]
    mov ebx,dword[ebp+12]
    mov edx,dword[ebp+16]
    finit
    fld dword[eax]  ;x0 (st4)
    fld dword[eax+4];y0 (st3)
    fld dword[eax+8];a0 (st2)
    fld dword[eax+12];s0(st1)

    mov si,0 ;сплошная линия, без прерываний

    .new_contur:

    mov cx,word[ebx]
    add ebx,2
    btr cx,15
    jae @f
      mov edx,dword[ebx]
      add ebx,4
    @@:
    btr cx,14 ;VECT_PARAM_PROP_L
    jae @f
      mov esi,dword[ebx] ;получаем новые параметры линии
      btr si,8 ;VECT_CONT_LINE = 0x100
      ;jc .end_contur ;дальше контур линейный, не Безье
	  jae .skip
	and ecx,0xfff
	cmp ecx,0
	je .end_contur
	add ebx,4
	shl ecx,3
		add ebx,ecx
	    jmp .new_contur
	  .skip:
      and si,0xff
      add ebx,4
    @@:
    and ecx,0xfff
    cmp ecx,0
    je .end_contur

    cmp si,1 ;проверка контура на 3 точки
    je @f
    cmp si,2
    je @f
    jmp .3pt
      shl ecx,3
      add ebx,ecx

      jmp .new_contur
    .3pt: ;тут контуры минимум с 3-мя точками

    mov di,si

      fld dword[ebx] ;st0=a(n)
      fadd st0,st2
      fcos
      fmul dword[ebx+4] ;l(n)
      fmul st0,st1 ;*=scale
      fadd st0,st4
      fistp word[v_poi_1+2] ;x(n)

      fld dword[ebx] ;st0=a(n)
      fadd st0,st2
      fsin
      fmul dword[ebx+4] ;l(n)
      fmul st0,st1 ;*=scale
      fadd st0,st3
      fistp word[v_poi_1] ;y(n)
    dec cx
    add ebx,8 ;move next coord
    cmp cx,0
    je .new_contur ;во избежание зацикливания

      fld dword[ebx] ;st0=a(n)
      fadd st0,st2
      fcos
      fmul dword[ebx+4] ;l(n)
      fmul st0,st1 ;*=scale
      fadd st0,st4
      fistp word[v_poi_2+2] ;x(n)

      fld dword[ebx] ;st0=a(n)
      fadd st0,st2
      fsin
      fmul dword[ebx+4] ;l(n)
      fmul st0,st1 ;*=scale
      fadd st0,st3
      fistp word[v_poi_2] ;y(n)
    dec cx
    add ebx,8 ;move next coord

or word[opt_bez],1 ;begin line
    .bez_cycl: ;---------------------------------------------------------

      dec di
      push dword[v_poi_1]
      pop dword[v_poi_0]
      push dword[v_poi_2]
      pop dword[v_poi_1]

      cmp cx,0
      je .new_contur ;во избежание зацикливания

      fld dword[ebx] ;st0=a(n)
      fadd st0,st2
      fcos
      fmul dword[ebx+4] ;l(n)
      fmul st0,st1 ;*=scale
      fadd st0,st4
      fistp word[v_poi_2+2] ;x(n)

      fld dword[ebx] ;st0=a(n)
      fadd st0,st2
      fsin
      fmul dword[ebx+4] ;l(n)
      fmul st0,st1 ;*=scale
      fadd st0,st3
      fistp word[v_poi_2] ;y(n)
      add ebx,8 ;move next coord

      cmp di,2
      jne @f
	or word[opt_bez],2 ;end line
      @@:
      cmp cx,1
      jne @f
	or word[opt_bez],2 ;end line
      @@:
dec si
cmp di,si
      jne @f
or word[opt_bez],1 ;begin line
@@:
inc si
      cmp di,si
      je @f
      cmp di,1
      je .end_draw_line
	push dword edx ;line color
	push dword[v_poi_0]
	push dword[v_poi_1]
	push dword[v_poi_2]
	call cruve_bezier_del2
    finit
    fld dword[eax]  ;x0 (st4)
    fld dword[eax+4];y0 (st3)
    fld dword[eax+8];a0 (st2)
    fld dword[eax+12];s0(st1)

      jmp @f
	.end_draw_line: ;-------------------------------------------------------------
	mov di,si
	inc di
      @@:

      ;loop .bez_cycl ;@b
      dec cx
      cmp cx,0
      jg .bez_cycl

      jmp .new_contur
    .end_contur:
  popad
  pop ebp
  ret 12


;функция принимает координаты точки x,y и определяет угол и длину
;input:
; ebp+8  = adress int coord x
; ebp+10 = adress int coord y
align 4
opred2i:
  push ebp
  mov ebp,esp
  finit
  fild word [ebp+8]
  fmul st0,st0 ;st0=x^2
  fild word [ebp+10]
  fmul st0,st0 ;st0=y^2
  fadd st0,st1
  fsqrt
  fst dword [o_len]
  cmp dword [o_len],0
  jne @f
    mov dword [o_ang],0
    jmp .retf
  @@:
  fild word [ebp+8]
  fdiv dword [o_len]
  call acos

  cmp word [ebp+10],0
  jl @f
    fst [o_ang] ;a=acos(x/l);
    jmp .retf
  @@:

  fldpi
  fadd st0,st0 ;st0=2*pi
  fsub st0,st1 ;st0=2*pi-aac
  fst [o_ang] ;a=st0;
  .retf:
  pop ebp
  ret 4

;функция определяющая расстояние между точками, результат попадает в o_len
;input:
; ebp+8  = p0
; ebp+12 = p1
align 4
line_len4i:
  push ebp
  mov ebp,esp

  finit
  fild word [ebp+8]
  fisub word [ebp+12]
  fmul st0,st0 ;st0=x^2
  fild word [ebp+10]
  fisub word [ebp+14]
  fmul st0,st0 ;st0=y^2
  fadd st0,st1
  fsqrt
  fstp dword [o_len]

  pop ebp
  ret 8

;функция для нахождения арккосинуса
;input:
; st0 = float value
align 4
acos:
  fld1
  fadd st, st1
  fld1
  fsub st, st2
  fmulp st1, st
  fsqrt
  fxch st1
  fpatan
  ret

o_len dd ? ;длина
o_ang dd ? ;угол порота в радианах
opr_param dd ?
v_poi_0 dd ?
v_poi_1 dd ?
v_poi_2 dd ?
opt_bez dw ? ;опции рисования кусков для кривой Безье

align 16
EXPORTS:
  dd sz_buf_create, buf_create
  dd sz_set_active_buf, set_active_buf
  dd sz_buf_clear, buf_clear
  dd sz_draw_buf, draw_buf
  dd sz_buf_delete, buf_delete

  dd sz_line, line_brs
  dd sz_cruve_bezier, cruve_bezier
  dd sz_conv, convert_contur
  dd sz_draw, draw_poly_line
  dd sz_opred2i, opred2i
  dd sz_line_len4i, line_len4i
  dd sz_draw_text, draw_text
  dd sz_o_len, o_len
  dd sz_o_ang, o_ang
  dd 0,0
  sz_buf_create db 'vect_buf_create',0
  sz_set_active_buf db 'vect_buf_set_active',0
  sz_buf_clear db 'vect_buf_clear',0
  sz_draw_buf db 'vect_buf_draw',0
  sz_buf_delete db 'vect_buf_delete',0

  sz_line db 'vect_line',0
  sz_cruve_bezier db 'vect_c_bezier',0
  sz_conv db 'vect_conv_cont',0
  sz_draw db 'vect_draw_cont',0
  sz_opred2i db 'vect_opred2i',0
  sz_line_len4i db 'vect_line_len4i',0
  sz_draw_text db 'vect_draw_text',0
  sz_o_len db 'vect_o_len',0
  sz_o_ang db 'vect_o_ang',0

