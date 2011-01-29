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
;;;;;;;;;;;;;;;;;;;;;;;;;;
;Общий выход из editbox для всех функций и пост обработчиков
;;;;;;;;;;;;;;;;;;;;;;;;;;
.editbox_exit:
edit_ex
;==========================================================
;=== обработка клавиатуры =================================
;==========================================================
.key:
pusha
        test    word ed_flags,ed_focus ; если не в фокусе, выходим
        je      .editbox_exit
        test    word ed_flags,ed_mouse_on
        jne     .editbox_exit

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
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;проверка на shift был ли нажат 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
are_key_shift_press
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; проверяем, находится ли курсор в конце + дальнейшая обработка
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
are_key_cur_end
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Обработка клавиш insert,delete.backspase,home,end,left,right
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
use_work_key
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
        jmp     .editbox_exit
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
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Функция обработки  мышки получение координат и проверка их + выделения
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
use_work_mause scr_h,scr_w
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Общие функции обработки 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
use_general_func
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Функции для работы с key
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
use_key_func
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Функции для работы с mouse
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
use_mouse_func scr_w
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Bit mask from editbox 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
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