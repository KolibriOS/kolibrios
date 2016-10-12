#ifndef KOLIBRI_TREELIST_H
#define KOLIBRI_TREELIST_H

/*
el_focus dd tedit0
mouse_dd dd 0
tree1 tree_list 264,count_of_dir_list_files+2, tl_key_no_edit+tl_draw_par_line+tl_list_box_mode,\
    16,16, 0x8080ff,0x0000ff,0xffffff, 0,70,TED_PANEL_WIDTH-17,120, 0,0,0, el_focus,\
    ws_dir_lbox,0

tree3 tree_list MAX_COLOR_WORD_LEN,3,tl_key_no_edit,\
	16,16, 0x8080ff,0x0000ff,0xffffff, 5,30,300,160, 16, 0,0, el_focus, w_scr_t3,0

tree_file_struct:
  dd 1
  dd 0,0,count_of_dir_list_files
  dd dir_mem
  db 0
  dd file_name ;sys_path
*/
/*
;struct TreeList
;  type dw ? ;+ 0 тип элемента, или индекс иконки для узла
;  lev db ?  ;+ 2 уровень элемента
;  clo db ?  ;+ 3 флаг закрытия, или открытия (имеет смысл для родительского узла)
;  perv dd ? ;+ 4 индекс предыдущего элемента
;  next dd ? ;+ 8 индекс последующего элемента
;  tc dd ?   ;+12 врем. создания
;  td dd ?   ;+16 врем. удаления
;ends

struc tree_list info_size,info_max_count,style, img_cx,img_cy,\
    col_bkg,col_zag,col_txt, box_l,box_t,box_w,box_h, capt_cy,info_capt_offs,\
    info_capt_len,el_focus, p_scroll,on_press {
.box_left    dd box_l
.box_top     dd box_t
.box_width   dd box_w
.box_height  dd box_h
.data_info   dd 0
.info_size   dw info_size
.info_max_count dd info_max_count
.style       dd style
.data_nodes  dd 0
.data_img    dd 0
.img_cx      dw img_cx
.img_cy      dw img_cy
.data_img_sys dd 0
.ch_tim      dd 0
.tim_undo    dd 0
.cur_pos     dd 0
.col_bkg     dd col_bkg
.col_zag     dd col_zag
.col_txt     dd col_txt
.capt_cy     dw capt_cy
.info_capt_offs dw info_capt_offs
.info_capt_len dw info_capt_len
.el_focus    dd el_focus
.p_scroll    dd p_scroll
.on_press    dd on_press
}
*/
// константы стиля
enum tl_style {
    TL_KEY_NO_EDIT  = 1,    // элемент нельзя редактировать на клавиатуре (изменять уровни, удалять)
    TL_DRAW_PAR_LINE = 2,   // рисовать линии к родительскому узлу
    TL_LISTBOX_MODE  = 4    //стиль не отображает уровни (как в ListBox все одного уровня)
};

typedef struct __attribute__ ((__packed__)) {
    uint32_t    left;
    uint32_t    top;
    uint32_t    width;
    uint32_t    height;
    void       *data_info;  // указатель на основные даные
    uint16_t    info_size;  // размер данных выделяемых для каждого узла (пользовательськие данные + текст для подписи)
    uint32_t    info_max_count; // максимальное количество узлов, которые можно добавить в элемент
    uint32_t    style;      // стили элемента
    void       *data_nodes; // указатель на структуры узлов
    void       *data_img;   // указатель на изображения с иконками узлов
    uint16_t    img_cx;     // ширина иконок
    uint16_t    img_cy;     // высота иконок
    void       *data_img_sys;//указатель на системные изображения (стрелки, плюсики)
    uint32_t    ch_tim;     // количество изменений в файле
    uint32_t    tim_undo;   // количество отмененных действий
    uint32_t    cur_pos;    // позиция курсора
    color_t     col_bkg;    // цвет фона
    color_t     tl_col_zag; // цвет заголовка
    color_t     tl_col_txt; // цвет текста
    uint16_t    capt_cy;    // высота подписи
    uint16_t    info_capt_offs;//сдвиг для начала текста (подписи узла)
    uint16_t    info_capt_len;//длина текста подписи узла (если = 0 то до конца структуры)
    void       *el_focus;   // указатель на структуру элемента в фокусе
    void       *p_scroll;   // указатель на структуру скроллинга
    void       *on_press;   // +84 указатель на функцию, которая вызывается при нажатии Enter
} treelist;

/*
;константы для функций
tl_err_save_memory_size equ  10b ;не хватает памяти для сохранения элемента
tl_err_load_caption     equ   1b ;в памяти нет заголовка 'tree'
tl_err_load_info_size   equ 100b ;не совпадает размер информационной структуры при открытии
tl_load_mode_add        equ 0x20000 ;опция считывания в режиме добавления информации
tl_save_load_heder_size equ 26 ;размер заголовка для записи/чтения элементов
*/

#endif //KOLIBRI_TREELIST_H
