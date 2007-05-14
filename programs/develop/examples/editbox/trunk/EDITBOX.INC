; SEE YOU File FAQ.txt and HISTORY. Good Like! 
;;;;;;;;;;;;;;;;;;
include 'editbox.mac'    ;макрос который должен облегчить жизнь :) специально для editbox
;;;;;;;;;;;;;;;;;;
macro use_edit_box procinfo,scr_h,scr_w
{
edit_box:
ed_width        equ [edi]               ;ширина компонента
ed_left         equ [edi+4]             ;положение по оси х
ed_top          equ [edi+8]             ;положение по оси у
ed_color        equ [edi+12]            ;цвет фона компонента
shift_color     equ [edi+16]            ;=0x6a9480
ed_focus_border_color   equ [edi+20]    ;цвет рамки компонента
ed_blur_border_color    equ [edi+24]    ;цвет не активного компонента
ed_text_color   equ [edi+28]            ;цвет текста
ed_max          equ [edi+32]                    ;кол-во символов которые можно максимально ввести
ed_text         equ [edi+36]                    ;указатель на буфер
ed_flags        equ [edi+40]            ;флаги
ed_size equ [edi+42]                    ;кол-во символов
ed_pos  equ [edi+46]                    ;позиция курсора
ed_offset       equ [edi+50]            ;смещение
cl_curs_x       equ [edi+54]            ;предыдущее координата курсора по х
cl_curs_y       equ [edi+58]            ;предыдущее координата курсора по у
ed_shift_pos    equ [edi+62]            ;положение курсора
ed_shift_pos_old equ [edi+66]           ;старое положение курсора
;==========================================================
;=== процедура прорисовки =================================
;==========================================================
.draw:
pusha
;--- рисуем рамку ---
        call    .draw_border            ; Функция стабильна
.draw_bg_cursor_text:
;--- изменяем смещение, если надо ---
        call    .check_offset           ;вычисление позиции курсора стабильна
;--- рисуем внутреннюю область ---
        call    .draw_bg                ;нарисовать прямоугольник рабочей области
;---- рисуем выделение, по shift если есть
        call    .draw_shift
.draw_cursor_text:
;--- рисуем курсор ---
        ;--- может его не надо рисовать ----
        test    word ed_flags,ed_focus
        je     @f
        call    .draw_cursor
@@:
        call    .draw_text
edit_ex
;==========================================================
;=== обработка клавиатуры =================================
;==========================================================
.key:
pusha
        test    word ed_flags,ed_focus ; если не в фокусе, выходим
        je      .no_figure
;Проверка нажат shift ?
        call    .check_shift
;----------------------------------------------------------
;--- проверяем, что нажато --------------------------------
;----------------------------------------------------------
use_key_process  backspase,delete,left,right,home,end,insert
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Заглушка на обработку клавиш вверх и вниз т.е. при обнаружении этих кодов происходит выход из обработчика
;;;;;;;;;;;;;;;;;;;;;;;;;;;;    
use_key_no_process   up,down,esc
;--- нажата другая клавиша ---
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Проверка установлен ли флаг при котором нужно выводить только цифры в нужном боксе если такойнеобходимости нет нужно закоментировать макрос
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
use_key_figures_only
;проверка на shift
@@:     test    word ed_flags,ed_shift_on
        je      @f
        ;Входные данные edx=ed_size;ecx=ed_pos
        push    eax
        mov     edx,ed_size
        mov     ecx,ed_pos
        pusha
;clear
;        mov     ebp,edx ;ed_size
;        call    .clear_bg
        mov     ebp,ed_color
        mov     ebx,dword       ed_shift_pos
        call    .sh_cl_

.sh_nxt:popa 
        call    .del_char
;;;;
        mov     eax,dword ed_shift_pos
        mov     ebx,ed_size
        sub     ebx,eax
        mov     ed_size,ebx
        pop     eax
        ; проверяем, находится ли курсор в конце
@@:     mov     ecx,ed_size
        mov     edx, ed_max
        test    word ed_flags,ed_insert
        jne     @f
        cmp     ecx,edx
        jae     .no_figure
@@:     mov     ebx, ed_pos
        cmp     ebx,edx
        jl      @f ; если меньше или равно
.no_figure:
edit_ex
.insert: test   word ed_flags,ed_insert ;not    word ed_insert
         je     .insert_1
         and    word ed_flags,ed_insert_cl
        jmp     .no_figure
.insert_1:
        or      word ed_flags,ed_insert
        jmp     .no_figure
.ins_v:
        dec     dword [ebp+42];ed_size    ;processing is insert
        sub     esi,ecx
        add     esi,ebx
        mov     edi,esi
;clear
pusha
        mov     edi,ebp
        mov     ebp,ed_pos
        call    .clear_bg
popa
        jmp     .In_k
@@:     ; сдвигаем символы после курсора вправо
        mov     ecx,ed_size
        push    edi eax
        mov     ebp,edi
        mov     esi,ed_text     ; Указатель на буфер
                                ;Будем работать со строкой
        add     esi,ecx         ;add ed_size добавим max size
        mov     edi,esi

        cmp     ecx,ebx         ;Если у нас позиция курсора = текущему размеру напечатанных символов т.е. курсор стоит в конце
        je      .In_k

        test    word [ebp+40],ed_insert ;IF insert is enable  т.к. edi изменен адресуем через ebp
        jne     .ins_v
;clear
pusha
        mov     edi,ebp
        mov     ebp,ed_size
        call    .clear_bg
popa
        sub     ecx,ebx         ;Найдем кол-во символов для передвижения.
        inc     edi             ;Сместим наши символы в право
        std
        inc     ecx
   @@:  
        ;--------
        lodsb
        stosb
        ;--------
        loop    @b
.In_k:  cld
        pop eax
        mov al,ah
        stosb
        pop edi
; вставляем код клавиши туда, где курсор
        ; увеличиваем значение размера и позиции
        inc      dword ed_size
        inc      dword ed_pos
        call    .draw_all2
        jmp     .shift;.draw_cursor_text
.delete:        
        mov     edx,ed_size
        mov     ecx,ed_pos
        cmp     edx,ecx
        jg      .bac_del
        test    word ed_flags,ed_shift_on
        jne     .del_bac
edit_ex
.bac_del:       
        call    .del_char
        jmp     .draw_all
        
;--- нажата клавиша backspace ---
.backspace:
        ; проверяем, курсор у левого края ?
        mov     ecx,ed_pos
        test    ecx,ecx
        jnz     .del_bac
        test    word ed_flags,ed_shift_on
        jne     .bac_del

edit_ex
.del_bac:
        mov     edx,ed_size
        cmp     edx,ecx ;if ed_pos=ed_size
        je      @f
        dec     ecx
        call    .del_char
@@:     test    word ed_flags,ed_shift_on
        jne     .bac_del
        dec      dword ed_pos
.draw_all:
        push    .shift;.draw_cursor_text;eax

        test    word ed_flags,ed_shift_on
        je      @f
        mov     eax,dword ed_shift_pos
        mov     ebx,ed_size
        sub     ebx,eax
        mov     ed_size,ebx

        mov     ebp,ed_color
        call    .clear_cursor
        call    .check_offset
        call    .draw_bg
        ret
@@:     dec      dword ed_size

.draw_all2:
        and    word ed_flags,ed_shift_cl
        mov     ebp,ed_color
        call    .clear_cursor
        call    .check_offset
        mov     ebp,ed_size
        call    .clear_bg
        ret
;--- нажата клавиша left ---
.left:  mov     ebx,ed_pos
        test    ebx,ebx
        jz      .sh_st_of
        or      word ed_flags,ed_left_fl
        call    .sh_first_sh
        dec      dword ed_pos
        call    .sh_enable
        jmp     .draw_cursor_text
;--- нажата клавиша right ---
.right: mov     ebx,ed_pos
        cmp     ebx,ed_size
        je      .sh_st_of
        and     word ed_flags,ed_right_fl
        call    .sh_first_sh
        inc     dword  ed_pos
        call    .sh_enable
        jmp     .draw_cursor_text
.home:  
        mov     ebx,ed_pos
        test    ebx,ebx
        jz      .sh_st_of
        call    .sh_first_sh
        xor     eax,eax
        mov     ed_pos,eax
        call    .sh_home_end
        jmp     .draw_cursor_text
.end:   
        mov     ebx,ed_pos
        cmp     ebx,dword ed_size
        je      .sh_st_of
        call    .sh_first_sh
        mov     eax,ed_size
        mov     ed_pos,eax
        call    .sh_home_end
        jmp     .draw_cursor_text
;==========================================================
;=== обработка мыши =======================================
;==========================================================
.mouse:
pusha
;debug
;----------------------------------------------------------
;--- получаем состояние кнопок мыши -----------------------
;----------------------------------------------------------     
        mcall   37,2
;----------------------------------------------------------
;--- проверяем состояние ----------------------------------
;----------------------------------------------------------
        test    eax,1              
        jnz     .mouse_left_button
        and     word ed_flags,ed_mouse_on_off
        xor     ebx,ebx
        mov     dword [mouse_flag],ebx
edit_ex
.mouse_left_button:
;----------------------------------------------------------
;--- блокировка от фокусировки в других боксах при попадании на них курсора
;----------------------------------------------------------
        mov     eax,dword [mouse_flag]
        test    eax,eax
        jz      @f
        cmp     eax,edi
        je      @f
        jmp     ._blur
;----------------------------------------------------------
;--- получаем координаты мыши относительно 0 т.е всей области экрана
;----------------------------------------------------------
@@:     mcall   37,0
;----------------------------------------------------------
;--- А не удерживаем ли мы клавишу мышки, перемещая курсор, во все разные стороны?
;----------------------------------------------------------
        test    word ed_flags,ed_mouse_on
        jne     .mouse_wigwag
;----------------------------------------------------------
;--- проверяем, попадает ли курсор в edit box -------------
;----------------------------------------------------------
        mov     ebx,[procinfo.box.top]
        add     ebx,ed_top
if scr_h eq 
else
        add     ebx,scr_h
end if
        cmp     ax,bx
        jl      ._blur;.mouse_end_no_focus

        add     ebx,ed_height
        cmp     ax,bx
        jg      ._blur;.mouse_end_no_focus

        shr     eax,16

        mov     ebx,[procinfo.box.left]
        add     ebx,ed_left
if scr_w eq 
else
        add     ebx,scr_w
end if
        cmp     ax,bx
        jl      ._blur;.mouse_end_no_focus

        add     ebx,ed_width
        cmp     ax,bx
        jg      ._blur;.mouse_end_no_focus
;--- изменяем позицию курсора ---
        push    eax
        mov     ebp,ed_color
        call    .clear_cursor
        pop     eax
._mvpos:
        mov     ebx,dword [procinfo.box.left]
        xor     edx,edx
        sub     eax,ed_left
        sub     eax,ebx
if scr_w eq 
else
        add     ebx,scr_w
        sub     eax,2
end if
        mov     ebx,6
        div     bx
        add     eax,ed_offset
        cmp     eax,ed_size
        jna     ._mshift
        mov     eax,ed_size
._mshift:
;;;;;;;
;;Секция обработки shift и выделения по shift
;;;;;;;
        test    word ed_flags,ed_shift_bac
        je      @f
        mov     ebp,dword       ed_color
        mov     ebx,dword ed_shift_pos
        push    eax
        call    .sh_cl_
        and     word ed_flags,ed_shift_bac_cl
        pop     eax
@@:     
        test    word ed_flags,ed_mouse_on
        jne     @f
        
        mov     dword ed_shift_pos,eax
        or      word  ed_flags,ed_mouse_on
        mov     dword ed_pos,eax
        mov     dword [mouse_flag],edi          ;установим идентификатор
        bts     word ed_flags,1                 ;установка фокуса
        jmp     .m_sh
@@:             
        cmp     eax,dword ed_shift_pos  ;если позиции не изменились
        je      .mouse_ex
        mov     ed_pos,eax
        mov     ebp,dword shift_color
        mov     ebx,dword ed_shift_pos
        call    .sh_cl_
        or      word ed_flags,ed_mous_adn_b     ;установим бит что мы выделили +shift_on +
.m_sh:  call    .draw_text
        call    .draw_cursor
;----------------------------------------------------------
;--- процедура установки фокуса ---------------------------
;----------------------------------------------------------
        jmp     .drc
._blur:
        test    word ed_flags,ed_always_focus
        jne     .mouse_ex
        btr     word ed_flags,1 ; если не в фокусе, выходим
        jnc     .mouse_ex

        mov     ebp,ed_color
        call    .clear_cursor
.drc:   call    .draw_border
.mouse_ex:

edit_ex
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Общие функции обработки 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
use_general_func
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Функции для работы с key
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
use_key_func
;----------------------------------------------------------
;--- процедура размывания фокуса --------------------------
;----------------------------------------------------------
;.blur:
;pusha
;._blur:
        ;btr     ed_flags,1
        ;jnc     @f
        ;call    .draw_border
        ;call    .clear_cursor
;@@:
;edit_ex

;----------------------------------------------------------
;--- Обработка .mouse_wigwag
;----------------------------------------------------------
.mouse_wigwag:
        shr     eax,16
        or      word ed_flags,ed_shift_bac+ed_shift_on+ed_shift
;;;;;;;;;;;;;;;;;;
;;процедура обработки положения выделенного текста, когда происходит выход за пределы editbox
;;;;;;;;;;;;;;;;;;
        mov     ebx,[procinfo.box.left]
        add     ebx,ed_left
if scr_w eq 
else
        add     ebx,scr_w
end if
        cmp     eax,ebx
        jb      .mleft

        add     ebx,ed_width
        cmp     eax,ebx
        ja      .mright 

        sub     ebx,ed_width

        xor     edx,edx
        sub     eax,ebx ; вычтим из координат мышки по оси х координаты до editbox по оси х
        mov     ebx,6
        div     ebx
;;;;;;;;;;;;;;;;;;
;;процедура обработки положения выделенного текста, в пределах области editbox
;;;;;;;;;;;;;;;;;;
;Получили координаты в eax мышки, т.е. куда она переместилась
;Рисование закрашеных прямоугольников и очистка их
        add     eax,ed_offset   ;добавим смещение
        cmp     eax,dword ed_size       ;если вышли за пределы, то ничего не делать
        ja      .mwigvag
.mdraw: 
        mov     dword   ed_pos,eax ;сохраним новое значение
;Рисование закрашеных прямоугольников и очистка их
        mov     ecx,dword       ed_shift_pos
        mov     ebx,dword       ed_shift_pos_old
        mov     dword   ed_shift_pos_old,eax    ;внесем новое значение старой позиции курсора
;проверка и рисование закрашеных областей
        cmp     ecx,ebx         ;выясняем куда было движение на один шаг назад
        je      .m1_shem        ;движения не было ранее 
        jb      .msmaller       ;движение было ->
        cmp     ebx,eax         ;движение было до этого <- и тут мы проверяем сейчас куда движение происходит
        ja      .m1_shem        ;если было движение <- то нужно закрасить область
        je      .mwigvag        ;если изменения не было, то ничего не делать
        mov     ebp,ed_color    ;тут нужно очистить область c ed_pos ed_shift_pos_old
;входные параметры ebp=color ebx=ed_shift_pos
        call    .sh_cl_
        jmp     .mwigvag
.msmaller:
        cmp     ebx,eax
        jb      .m1_shem
        mov     ebp,ed_color
;входные параметры ebp=color ebx=ed_shift_pos
        call    .sh_cl_
        jmp     .mwigvag
;alike  =
.m1_shem: 
        mov     ebp,shift_color
;входные параметры ebp=color ebx=ed_shift_pos
        mov     ebx,ecx
        call    .sh_cl_
        jmp     .mwigvag
.mwigvag:
        and     word ed_flags,ed_shift_mcl
        jmp     .draw_cursor_text
;       popa
;       ret
.mleft:
        mov     eax,ed_pos
        cmp     eax,0                                        
        jbe      .mwigvag
        dec     eax
        call    .check_offset
        push    eax
        mov     ebx,ed_shift_pos
        mov     ebp,shift_color
        call    .sh_cl_
        pop     eax
        jmp     .mdraw
.mright:
        mov     eax,ed_pos
        mov     ebx,ed_size
        cmp     eax,ebx
        jae     .mwigvag
        inc     eax
        call    .check_offset
        mov     ebx,ed_shift_pos
        mov     ebp,shift_color
        push    eax
        call    .sh_cl_
        pop     eax
        jmp     .mdraw  

}
ed_figure_only= 1000000000000000b   ;одни символы 
ed_always_focus= 100000000000000b
ed_focus=                     10b   ;фокус приложения
ed_shift_on=                1000b   ;если не установлен -значит впервые нажат shift,если был установлен, значит мы уже что - то делали удерживая shift
ed_shift_on_off=1111111111110111b
ed_shift=                    100b   ;включается при нажатии на shift т.е. если нажимаю
ed_shift_off=   1111111111111011b
ed_shift_bac=              10000b   ;бит для очистки выделеного shift т.е. при установке говорит что есть выделение
ed_shift_bac_cl=1111111111101111b   ;очистка при удалении выделения
ed_shift_cl=    1111111111100011b
ed_shift_mcl=   1111111111111011b
ed_left_fl=               100000b
ed_right_fl=    1111111111011111b
ed_offset_fl=            1000000b
ed_offset_cl=   1111111110111111b
ed_insert=              10000000b
ed_insert_cl=   1111111101111111b
ed_mouse_on =          100000000b
ed_mous_adn_b=         100011000b
ed_mouse_on_off=1111111011111111b
ed_height=14 ; высота
macro draw_edit_boxes start,_end,use_f9,procinfo
{
if use_f9 eq 
else
        mcall   9,procinfo,-1
end if
        mov     edi,start
        mov     ecx,((_end-start)/ed_struc_size)
@@:
        call    edit_box.draw
        add     edi,ed_struc_size
        loop    @b
}

macro mouse_edit_boxes start,_end
{
        mov     edi,start
        mov     ecx,((_end-start)/ed_struc_size)
@@:
        call    edit_box.mouse
        add     edi,ed_struc_size
        loop    @b
}

macro key_edit_boxes start,end
{
        mov     edi,start
        mov     ecx,((end-start)/ed_struc_size)
@@:
        call    edit_box.key
        add     edi,ed_struc_size
        loop    @b
}
ed_struc_size=70
struc  edit_box width,left,top,color,shift_color,focus_border_color,\
       blur_border_color,text_color,max,text,flags,size,pos
{
.width dd width
.left dd left
.top dd top
.color dd color
.shift_color dd shift_color
.focus_border_color dd focus_border_color
.blur_border_color dd blur_border_color
.text_color dd text_color
.max dd max
.text dd text
.flags dw flags+0
.size dd size+0
.pos dd pos+0
.offset dd 0
.cl_curs_x dd 0
.cl_curs_y dd 0
.shift   dd 0
.shift_old   dd 0
}


macro edit_boxes_set_sys_color start,end,color_table
{
        mov     edi,start
        mov     ecx,((end-start)/ed_struc_size)
        mov     esi,color_table
@@:
        mov     eax,[esi+36]
        mov     ebx,[esi+20]
        mov     ed_focus_border_color,eax
        shr     bh,1
        shr     bl,1
        shr     ah,1
        shr     al,1
        add     ah,bh
        add     al,bl
        ror     eax,16
        ror     ebx,16
        shr     bl,1
        shr     al,1
        add     al,bl
        ror     eax,16
        mov     ed_blur_border_color,eax
        add     edi,ed_struc_size
        loop    @b
}

macro draw_edit_box ed_ptr,use_f9,procinfo
{
if use_f9 eq 
else
        mcall   9,procinfo,-1
end if
        mov     edi,ed_ptr
        call    edit_box.draw
}

macro mouse_edit_box ed_ptr
{
        mov     edi,ed_ptr
        call    edit_box.mouse
}

macro key_edit_box ed_ptr
{
        mov     edi,ed_ptr
        call    edit_box.key
}
macro default_box ed_ptr
{
pusha
;       xor     eax,eax
;       mov     ed_shift_pos,eax
;       mov     ed_shift_pos_old,eax
        and     word ed_flags,ed_shift_cl
;       mov     ed_offset,eax
popa
}