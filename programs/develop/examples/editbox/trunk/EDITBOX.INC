; SEE YOU File FAQ.txt and HISTORY. Good Like! 
;;;;;;;;;;;;;;;;;;
include 'editbox.mac'    ;макрос который должен облегчить жизнь :) специально для editbox
;;;;;;;;;;;;;;;;;;
macro use_edit_box structure_of_potock,scr_h,scr_w
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

;debug_func
;----------------------------------------------------------
;--- процедура прорисовки выделеной части -----------------
;----------------------------------------------------------
.draw_shift:
        test    word ed_flags,ed_shift_bac ;установка флага, выделенной области
        jz      @f
        mov     ebp,shift_color
        mov     ebx,dword       ed_shift_pos
        call    .sh_cl_
@@:     ret
;----------------------------------------------------------
;--- процедура прорисовки текста --------------------------
;----------------------------------------------------------
.draw_text:
;--- вычисляем, сколько помещается символов ---
;--- чтобы мусор не рисовать ---
        call    .get_n
        mov     esi,ed_size
        mov     ebx,ed_offset
        sub     esi,ebx
        cmp     eax,esi
        jae     @F
        mov     esi,eax         ;чтобы не выходить за пределы экрана
;--- рисуем текст ---                                            
@@:     mov     eax,4
        mov     ebx,ed_left
        mov     edx,ed_offset
        add     ebx,2
        shl     ebx,16
        mov     bx,ed_top
        add     ebx,4
        mov     ecx,ed_text_color
        add     edx,ed_text
        int     0x40
ret
;----------------------------------------------------------
;--- процедура прорисовки фона ----------------------------
;входные данные
;eax
;edx - color
;----------------------------------------------------------
;вход только цвет edx
.draw_bg:
        mov     ebx,ed_left
        add     ebx,1
        shl     ebx,16
        mov     bx,ed_width
        sub     ebx,1
        mov     edx,ed_color
.draw_bg_eax:
        mov     ecx,ed_top
        mov     eax,13
        add     ecx,1
        shl     ecx,16
        mov     cx,ed_height
        dec     ecx
        int     0x40
ret

;----------------------------------------------------------
;--- процедура получения количества символов в текущей щирине компонента --------------
;----------------------------------------------------------
.get_n:
        mov     eax,ed_width    ;получем ширину компонента
        xor     edx,edx   ;результат распологается в паре edx:eax в eax - остаток
        sub     eax,4           ;вычтим 4
        mov     ebx,6           ;загрузми делитель
        div     ebx             ;размделим на 6
ret
;----------------------------------------------------------
;--- процедура рисования курсора --------------------------
;----------------------------------------------------------
;входные ebp- цвет
.clear_cursor:
        mov     edx,ebp
        mov     ebx,cl_curs_x
        mov     ecx,cl_curs_y
        jmp     .draw_curs
.draw_cursor:
        mov     edx,ed_text_color
        mov     ebx,ed_pos
        mov     ecx,ed_offset
        sub     ebx,ecx

        lea     ebx,[ebx*2+ebx]
        shl     ebx,1
        ;imul   ebx,6
        add     bx,ed_left
        mov     ecx,ed_top
        inc     ebx
        add     ecx,2
        mov     ebp,ebx
        shl     ebx,16
        mov     bx,bp
        mov     ebp,ecx
        
        shl     ecx,16
        mov     cx,bp
        add     cx,ed_height-4
        
        mov     cl_curs_x,ebx
        mov     cl_curs_y,ecx
.draw_curs:              
        mov     eax,38
        int     0x40
ret
;----------------------------------------------------------
;--- процедура рисования рамки ----------------------------
;----------------------------------------------------------
.draw_border:
;--- цвет рамки ---
        test    word ed_flags,ed_focus
        mov     edx,ed_focus_border_color
        jne     @f
        mov     edx,ed_blur_border_color
@@:
;--- сверху ---
        mov     eax,38
        mov     ebx,ed_left
        mov     ecx,ebx
        shl     ebx,16
        mov     bx,cx
        add     bx,ed_width
        mov     ecx, ed_top
        mov     esi,ecx
        shl     ecx,16
        mov     cx,si
        int     0x40

;--- снизу ---
        mov     esi,ecx
        add     ecx,ed_height
        mov     ebp,ecx
        shl     ecx,16
        mov     cx,bp
        int     0x40
 
;--- слева ---
        mov     cx,si
        mov     ebp,ebx
        sub     bx,ed_width
        int     0x40

;--- справа ---
        mov     ebx,ebp
        shl     ebx,16
        mov     bx,bp
        int     0x40
ret
;----------------------------------------------------------
;--- проверка, зашел ли курсор за границы и, если надо, ---
;--- изменяем смещение ------------------------------------
;--- eax = -1, если не изменилось или eax = 0, если
; изменилось
;----------------------------------------------------------
.check_offset:
pusha
        mov     ecx,ed_pos
        mov     ebx,ed_offset
        cmp     ebx,ecx
        ja      .sub_8

        push    ebx
        call    .get_n                  ;получим кол-во символов в паре регистров edx:eax 
        pop     ebx
        mov     edx,ebx
        add     edx,eax         ;ed_offset+width editbox
        inc     edx             ;необходимо для номального положения курсора в крайней левой позиции
        cmp     edx,ecx
        ja      @f

        mov     edx,ed_size
        cmp     edx,ecx
        je      .add_end

        sub     edx,ecx
        cmp     edx,8
        jbe     .add_8
        add     ebx,8
        jmp     .chk_d

.sub_8: cmp     ecx,0
        je      .sub_min
        cmp     ebx,8
        jbe     .sub_min
        sub     ebx,8   ;ebx=ed_offset
        jmp     .chk_d
.sub_min:
        xor     ebx,ebx
        jmp     .chk_d

.add_end:sub    edx,eax
        mov     ebx,edx
        jmp     .chk_d
.add_8: add     ebx,edx
.chk_d: mov     ed_offset,ebx
        call    .draw_bg
        and     word ed_flags,ed_offset_cl
edit_ex
@@:
        or      word ed_flags,ed_offset_fl
edit_ex
;==========================================================
;=== обработка клавиатуры =================================
;==========================================================
.key:
pusha
        test    word ed_flags,ed_focus ; если не в фокусе, выходим
        je      @b
;Проверка на нажаты shift
        call    .check_shift
;----------------------------------------------------------
;--- проверяем, что нажато --------------------------------
;----------------------------------------------------------
        cmp     ah,8
        jz      .backspace
        cmp     ah,0xb6
        jz      .delete
        cmp     ah,176
        jz      .left
        cmp     ah,179
        jz      .right
        cmp     ah,180
        jz      .home
        cmp     ah,181
        jz      .end
        cmp     ah,185  ;insert
        jz      .insert
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Заглушка на обработку клавиш вверх и вниз
;;;;;;;;;;;;;;;;;;;;;;;;;;;;    
        cmp     ah,177
        jz      .no_figure
        cmp     ah,178
        jz      .no_figure
        cmp     ah,27   ;ESC - клавиша ))
        jz      .no_figure
;--- нажата другая клавиша ---
        test    word ed_flags,ed_figure_only  ; только цифры ?
        jz      @f
        cmp     ah,'0'
        jb      .no_figure
        cmp     ah,'9'
        ja      .no_figure
;проверка на shift
@@:     test    word ed_flags,ed_shift_on
        je      @f
        ;Входные данные edx=ed_size;ecx=ed_pos
        push    eax
        mov     edx,ed_size
        mov     ecx,ed_pos
        pusha
;clear
        mov     ebp,edx ;ed_size
        call    .clear_bg
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
;Обработка Shift для снятия выделения неизвестной области
.shift: ;;;;;;;SHIFT
        test    word ed_flags,ed_shift
        je      .f_exit

@@:     mov     ebp,shift_color
        or      word ed_flags,ed_shift_bac ;установка флага, выделенной области
        mov     ebx,dword       ed_shift_pos
        call    .sh_cl_
        jmp     .draw_cursor_text
;;;;;;;;;;;;;;;;;;;;;
.f_exit:call    .check_offset
        and     word ed_flags,ed_shift_cl
        call    .enable_null
        jmp     .draw_cursor_text
.sh_cl_:
;;;;;;SHIFT end
;обработка очистки, при левом - правом движении выделения
;для обработки снятия выделения
;входные параметры ebp=color ebx=ed_shift_pos
        mov     eax,dword       ed_pos
        cmp     eax,ebx

        jae     .sh_n
        push    eax  ;меньшее в eax
        push    ebx  ;большее
        jmp     .sh_n1
                     ;если иначе
.sh_n:  push    ebx  
        push    eax
.sh_n1: 
        call    .check_offset
        call    .get_n
        mov     edx,eax ;size of ed_box
        mov     ecx,ed_offset
        add     eax,ecx ;eax = w_off= ed_offset+width
        mov     edx,eax ;save
        pop     ebx     ;большее
        pop     eax     ;меньшее

        cmp     eax,ecx         ;сравнение с меньшего с offset.
        jae     .f_f            ;если больше
        xor     eax,eax
        cmp     edx,ebx         ;cравним размер w_off с большим
        jb      @f
        sub     ebx,ecx
        jmp     .nxt_f
@@:     mov     ebx,edx
        sub     ebx,ecx 
        jmp     .nxt_f
.f_f:   
        sub     eax,ecx
        cmp     edx,ebx         ;cравним размер w_off с большим
        jle     @f
        sub     ebx,ecx
        sub     ebx,eax
        jmp     .nxt_f
@@:
        mov     ebx,edx 
        sub     ebx,ecx
        sub     ebx,eax
.nxt_f:
        mov     edx,ebx
        lea     ebx,[eax*2+eax]
        shl     ebx,1
        add     ebx,ed_left
        inc     ebx
        shl     ebx,16
        lea     ecx,[edx*2+edx]
        shl     ecx,1   
        mov     bx,cx
        inc     ebx
        mov     edx,ebp;shift_color

        call    .draw_bg_eax
@@:     call    .enable_null
        ret
;;;;;;;;;;;;;;;;;;;;;
;Установка- снятие выделения в один символ
;;;;;;;;;;;;;;;;;;;;;
.drw_sim:
        mov     eax,dword       ed_pos
        call    .draw_rectangle   ;нарисовать прямоугольник с заданным цветом
        jmp     @b
;;;;;;;;;;;;;;;;;;;;;
;Фукция установки выделения при движения влево и вправо и нажатии shift
;Логика: 
;;;;;;;;;;
.draw_wigwag:
;функция установки переменных
        mov     ebp,shift_color
        call    .clear_cursor

        or      word ed_flags,ed_shift_bac ;установка флага, выделенной области
        mov     ebp,shift_color
        mov     eax,dword       ed_pos
        test    word ed_flags,ed_left_fl
        jz      .low
        jmp     @f
;;;;;;;;;;
;Фукция удаления выделения при движения влево и вправо и нажатии shift
;Логика: 
;;;;;;;;;;
.draw_wigwag_cl:
;функция установки переменных
        mov     ebp,ed_color
        call    .clear_cursor

        mov     ebp,ed_color
        mov     eax,dword       ed_pos
        test    word ed_flags,ed_left_fl
        jz      .low
@@:     call    .draw_rectangle  ;нарисовать прямоугольник закрашиваемой области
        ret
.low:   dec     eax
        jmp     @b
;входной параметр ebx - ed_pos
.sh_first_sh:
        test    word ed_flags,ed_shift
        je      @f
        mov     dword ed_shift_pos_old,ebx
        test    word ed_flags,ed_shift_on
        jne     @f
        mov     dword ed_shift_pos,ebx
        or      word ed_flags,ed_shift_on
@@:     ret
;Обработка крайних положений в editbox при нажатом shift
;производит снятие выделение, если нет shift
;иначе вообще выходит
.sh_st_of:
        test    word ed_flags,ed_shift
        jne     @f
        test    word ed_flags,ed_shift_bac
        je      @f
        mov     ebp,ed_color
        mov     ebx,dword       ed_shift_pos
        call    .sh_cl_  ;очистка выделеного фрагмента
        and     word ed_flags,ed_shift_cl ; очистка от того что убрали выделение 
        jmp     .draw_cursor_text
@@:
        and     word ed_flags,ed_shift_off
edit_ex
;проверка состояния shift был ли он нажат раньше?
.sh_enable:
        test    word ed_flags,ed_shift
        jne     .sh_ext_en ;нарисовать закрашеный прямоугольник

        test    word ed_flags,ed_shift_bac
        je      @f
        call    .check_offset

        mov     ebp,ed_color
        mov     ebx,dword       ed_shift_pos
        call    .sh_cl_  ;очистка выделеного фрагмента
        call    .draw_wigwag_cl 
        and     word ed_flags,ed_shift_cl ; 1вар не нужно 
        ret

@@:     mov     ebp,ed_color
        call    .clear_cursor
        call    .check_offset
        ret
.sh_ext_en:     
        call    .check_offset
        test    word ed_flags,ed_offset_fl
        je      @f
;Рисование закрашеных прямоугольников и очистка их
        mov     eax,dword       ed_shift_pos
        mov     ebx,dword       ed_pos
        mov     ecx,dword       ed_shift_pos_old
;проверка и рисование закрашеных областей
        cmp     eax,ecx
        je      .1_shem
        jb      .smaller
        cmp     ecx,ebx
        ja      .1_shem
        call    .draw_wigwag_cl ;clear
        jmp     .sh_e_end
.smaller:
        cmp     ecx,ebx
        jb      .1_shem
        call    .draw_wigwag_cl ;clear
        jmp     .sh_e_end
;alike  =
.1_shem: call   .draw_wigwag
.sh_e_end:      and     word ed_flags,ed_shift_off
        ret
@@:     mov     ebp,shift_color
        mov     ebx,dword       ed_shift_pos
        call    .sh_cl_
        jmp     .sh_e_end
;функция для обработки shift при нажатии home and end
.sh_home_end:
        mov     ebp,ed_color
        call    .clear_cursor
        test    word ed_flags,ed_shift_bac
        je      @f
        mov     ebp,ed_color
        mov     ebx,dword       ed_shift_pos_old
        call    .sh_cl_

@@:     test    word ed_flags,ed_shift
        je      .sh_exit_ ;выйти
        mov     ebp,shift_color
        mov     ebx,dword       ed_shift_pos
        call    .sh_cl_
        or      word ed_flags,ed_shift_bac ;установка флага, выделенной области
        jmp     .sh_e_end
.sh_exit_:      call    .check_offset
        ret
;функция внесения 0 по адресу ed_size+1
.enable_null:
        pusha
        mov     eax,ed_size
        mov     ebx,ed_text
        test    eax,eax
        add     eax,ebx
        jne     @f
        inc     eax
@@:     xor     ebx,ebx
        mov     [eax],bl
edit_ex
;- удаление символа
;Входные данные edx=ed_size;ecx=ed_pos
.del_char:
        mov     esi,ed_text
        test    word ed_flags,ed_shift_on
        je      @f
        mov     eax,dword ed_shift_pos
        mov     ebx,esi
        cmp     eax,ecx
        jae     .dh_n

        mov     ed_pos,eax      ;что бы не было убегания курсора
        mov     ebp,ecx
        sub     ebp,eax
        add     ebx,eax  ;eax меньше 
        sub     edx,ecx
        add     esi,ecx

        mov     dword ed_shift_pos,ebp
        jmp     .del_ch_sh
                     ;если иначе
.dh_n:  
        mov     ebp,eax
        sub     ebp,ecx
        add     ebx,ecx
        sub     edx,eax
        add     esi,eax
        mov     dword ed_shift_pos,ebp
        jmp     .del_ch_sh

@@:     add     esi,ecx ;указатель + смещение к реальному буфферу
        mov     ebx,esi
        inc     esi
        cld

        sub     edx,ecx
.del_ch_sh:

        push    edi
        mov     edi,ebx
@@:
        lodsb
        stosb
        dec edx
        jns @b
        
        pop edi
        ret
;вычислить закрашиваемую область
;соглашение в ebp - передается ed_size
.clear_bg:
        call    .get_n  ;получить размер в символах ширины компонента
        push    eax
        mov     ebx,ed_offset
        add     eax,ebx ;eax = w_off= ed_offset+width
        mov     ebx,ebp ;ed_size
        
        cmp     eax,ebx
        jb      @f
        mov     eax,ed_pos
        sub     ebx,eax
        mov     ecx,ed_offset
        sub     eax,ecx
        jmp     .nxt
@@:     mov     ebx,ed_pos
        push    ebx
        sub     eax,ebx
        mov     ebx,eax ;It is don't optimal
        
        pop     eax     ;ed_pos
        mov     ecx,ed_offset
        sub     eax,ecx
.nxt:
        mov     ebp,eax  ;проверка на выход закрашиваемой области за пределы длины
        add     ebp,ebx
        pop     edx
        cmp     ebp,edx
        je      @f
        inc     ebx

@@:     mov     edx,ebx
        lea     ebx,[eax*2+eax]
        shl     ebx,1
        add     ebx,ed_left
        inc     ebx
        shl     ebx,16
        lea     ecx,[edx*2+edx]
        shl     ecx,1   
        mov     bx,cx
        mov     edx,ed_color
        call    .draw_bg_eax
        ret
;;;;;;;;;;;;;;;;;;;
;;; Обработка примитивов
;;;;;;;;;;;;;;;;;;;;
;Нарисовать прямоугольник, цвет передается в ebp
;входные параметры:
;eax=dword ed_pos
;ebp=-цвет ed_color or shift_color 
.draw_rectangle:
        mov     ecx,dword ed_offset
        sub     eax,ecx
        lea     ebx,[eax*2+eax]
        shl     ebx,1
        inc     ebx
        add     ebx,ed_left
        shl     ebx,16
        mov     bx,6
        mov     edx,ebp
        call    .draw_bg_eax
        ret
;;;;;;;;;;;;;;;;;;
;;Проверка нажат ли shift
;;;;;;;;;;;;;;;;;;
.check_shift:
pusha   ;сохраним все регистры
        mcall   66,3,1
        test    al,0x03
        je      @f
        or      word ed_flags,ed_shift   ;установим флаг
@@:edit_ex
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
        mov     ecx,structure_of_potock
        test    word ed_flags,ed_mouse_on
        jne     .mouse_wigwag
;----------------------------------------------------------
;--- проверяем, попадает ли курсор в edit box -------------
;----------------------------------------------------------
        mov     ebx,dword [ecx+38]
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

        mov     ebx,dword [ecx+34]
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
        mov     ecx,structure_of_potock
        mov     ebx,dword [ecx+34]
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
        mov     ecx,structure_of_potock
        mov     ebx,dword [ecx+34]
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
macro draw_edit_boxes start,_end,use_f9,structure_of_potock
{
if use_f9 eq 
else
        mcall   9,structure_of_potock,-1
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

macro draw_edit_box ed_ptr,use_f9,structure_of_potock
{
if use_f9 eq 
else
        mcall   9,structure_of_potock,-1
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
;	xor	eax,eax
;	mov 	ed_shift_pos,eax
;	mov	ed_shift_pos_old,eax
	and	word ed_flags,ed_shift_cl
;	mov	ed_offset,eax
popa
}