#ifndef KOLIBRI_TREELIST_H
#define KOLIBRI_TREELIST_H


/// константы стиля
enum tl_style {
    TL_KEY_NO_EDIT  = 1,    // элемент нельзя редактировать на клавиатуре (изменять уровни, удалять)
    TL_DRAW_PAR_LINE = 2,   // рисовать линии к родительскому узлу
    TL_LISTBOX_MODE  = 4    //стиль не отображает уровни (как в ListBox все одного уровня)
};

/// константы для функций
enum tl_err {
    TL_ERR_LOAD_CAPTION     = 1, //в памяти нет заголовка 'tree'
    TL_ERR_SAVE_MEMOTY_SIZE = 2, //не хватает памяти для сохранения элемента
    TL_ERR_LOAD_INFO_SIZE   = 4, //не совпадает размер информационной структуры при открытии
};

typedef struct __attribute__ ((__packed__)) {
    uint16_t    type;   //тип элемента, или индекс иконки для узла
    uint8_t     lev;    //уровень элемента
    uint8_t     clo;    //флаг закрытия, или открытия (имеет смысл для родительского узла)
    uint32_t    prev;   //индекс предыдущего элемента
    uint32_t    next;   //индекс последующего элемента
    uint32_t    tcreat; //врем. создания
    uint32_t    tdel;   //врем. удаления
} treelist_node;


typedef struct __attribute__ ((__packed__)) {
    uint32_t    left;
    uint32_t    top;
    uint32_t    width;
    uint32_t    height;
    void       *data_info;  // указатель на основные даные
    uint16_t    info_size;  // размер данных выделяемых для каждого узла (пользовательськие данные + текст для подписи)
    uint32_t    info_max_count; // максимальное количество узлов, которые можно добавить в элемент
    uint32_t    style;      // стили элемента
    treelist_node *data_nodes; // указатель на структуры узлов
    void       *data_img;   // указатель на изображения с иконками узлов
    uint16_t    img_cx;     // ширина иконок
    uint16_t    img_cy;     // высота иконок
    void       *data_img_sys;//указатель на системные изображения (стрелки, плюсики)
    uint32_t    ch_tim;     // количество изменений в файле
    uint32_t    tim_undo;   // количество отмененных действий
    uint32_t    cur_pos;    // позиция курсора
    color_t     col_bkg;    // цвет фона
    color_t     col_zag; // цвет заголовка
    color_t     col_txt; // цвет текста
    uint16_t    capt_cy;    // высота подписи
    uint16_t    info_capt_offs;//сдвиг для начала текста (подписи узла)
    uint16_t    info_capt_len;//длина текста подписи узла (если = 0 то до конца структуры)
    void       *el_focus;   // указатель на структуру элемента в фокусе
    scrollbar  *p_scroll;   // указатель на структуру скроллинга
    void       *on_press;   // +84 указатель на функцию, которая вызывается при нажатии Enter
} treelist;

static inline treelist* kolibri_new_treelist( uint32_t x_w, uint32_t y_h, uint16_t capt_cy, uint32_t icon_size_xy, uint16_t info_size, uint32_t info_max_count,
                                             uint16_t info_capt_len, uint16_t info_capt_offs, enum tl_style style, void *el_focus, color_t back, color_t title, color_t txt)
{
    treelist *tl = (treelist *)calloc(1, sizeof(treelist));
    tl->left= x_w >> 16;
    tl->width = x_w & 0xFFFF;
    tl->top = y_h >> 16;
    tl->height = y_h & 0xFFFF;
    tl->info_size = info_size;
    tl->info_max_count = info_max_count;
    tl->style = style;
    tl->img_cx = icon_size_xy >> 16;
    tl->img_cy = icon_size_xy & 0xFFFF;
    tl->col_bkg = back;
    tl->col_zag = title;
    tl->col_txt = txt;
    tl->info_capt_len = info_capt_len;
    tl->info_capt_offs = info_capt_offs;
    tl->el_focus = el_focus;
    tl->p_scroll = kolibri_new_scrollbar_def(X_Y(0, 16), X_Y(70, 30), 100, 30, 0);
    return tl;
}

static inline void gui_add_treelist(kolibri_window *wnd, treelist* tl)
{
    kolibri_window_add_element(wnd, KOLIBRI_TREELIST, tl);
}


///реакция на мышь
extern void (*tl_mouse)(treelist *) __attribute__((__stdcall__));

///вывод списка на экран
extern void (*tl_draw)(treelist *) __attribute__((__stdcall__));

///перемещаем узел вверх
extern void (*tl_node_move_up)(treelist *) __attribute__((__stdcall__));

///перемещаем узел вниз
extern void (*tl_node_move_down)(treelist *) __attribute__((__stdcall__));

extern void (*tl_data_init_asm)(treelist *) __attribute__((__stdcall__));
///выделение памяти для структур списка и основной информации (конструктор)
static inline void treelist_data_init(treelist *tl)
{
    __asm__ __volatile__ (
             "push %%edi \n\t":::);

    (*tl_data_init_asm)(tl);

    __asm__ __volatile__ (
             "pop %%edi \n\t":::);
}

extern void (*tl_data_clear_asm)(treelist *) __attribute__((__stdcall__));
///очистка памяти элемента (деструктор)
static inline void treelist_data_clear(treelist *tl)
{
    __asm__ __volatile__ (
             "push %%edi \n\t":::);

    (*tl_data_clear_asm)(tl);

    __asm__ __volatile__ (
             "pop %%edi \n\t":::);
}

extern void (*tl_info_clear_asm)(treelist *) __attribute__((__stdcall__));
///очистка списка (информации)
static inline void treelist_info_clear(treelist *tl)
{
    __asm__ __volatile__ (
             "push %%edi \n\t":::);

    (*tl_info_clear_asm)(tl);

    __asm__ __volatile__ (
             "pop %%edi \n\t":::);
}

extern void (*tl_key_asm)(treelist *) __attribute__((__stdcall__));
///реакция на клавиатуру
static inline void treelist_key(treelist *tl)
{
    __asm__ __volatile__ (
             "push %%ebx \n\t"
             "push %%edi \n\t":::);

    (*tl_key_asm)(tl);

    __asm__ __volatile__ (
             "pop %%edi \n\t"
             "pop %%ebx \n\t":::);
}

extern void (*tl_info_undo_asm)(treelist *) __attribute__((__stdcall__));
///отмена действия
static inline void treelist_undo(treelist *tl)
{
    __asm__ __volatile__ (
             "push %%edi \n\t":::);

    (*tl_info_undo_asm)(tl);

    __asm__ __volatile__ (
             "pop %%edi \n\t":::);
}

extern void (*tl_info_redo_asm)(treelist *) __attribute__((__stdcall__));
///повтор действия
static inline void treelist_redo(treelist *tl)
{
    __asm__ __volatile__ (
             "push %%edi \n\t":::);

    (*tl_info_redo_asm)(tl);

    __asm__ __volatile__ (
             "pop %%edi \n\t":::);
}

extern void (*tl_node_add_asm)(treelist *, uint32_t n_opt, void *n_info) __attribute__((__stdcall__));
///добавить узел
///input:
/// tlist - указатель на структуру листа
/// n_opt - опции добавления
/// n_info - указатель на добавляемые данные
static inline void treelist_node_add(treelist *tl, void *n_info, uint16_t type, uint8_t clos, uint8_t lev)
{
    __asm__ __volatile__ (
             "push %%ebx \n\t"
             "push %%edi \n\t":::);

    uint32_t    n_opt = (type << 16) | (clos << 8) | lev;
    (*tl_node_add_asm)(tl, n_opt, n_info);

    __asm__ __volatile__ (
             "pop %%edi \n\t"
             "pop %%ebx \n\t":::);
}

extern void (*tl_node_set_data_asm)(treelist *, void *n_info) __attribute__((__stdcall__));
///записать в текущий узел
///input:
/// tlist - указатель на структуру листа
/// n_info - указатель на данные
static inline void treelist_node_setdata(treelist *tl, void *n_info)
{
    __asm__ __volatile__ (
             "push %%esi \n\t"
             "push %%edi \n\t":::);

    (*tl_node_set_data_asm)(tl, n_info);

    __asm__ __volatile__ (
             "pop %%edi \n\t"
             "pop %%esi \n\t":::);
}

extern void* (*tl_node_get_data_asm)(treelist *) __attribute__((__stdcall__));
///взять указатель на данные узла под курсором
static inline void* treelist_getdata(treelist *tl)
{
    __asm__ __volatile__ (
             "push %%edi \n\t":::);

    void *res =
    (*tl_node_get_data_asm)(tl);

    __asm__ __volatile__ (
             "pop %%edi \n\t":::);
    return res;
}

extern void (*tl_node_delete_asm)(treelist *) __attribute__((__stdcall__));
///удалить узел под курсором
static inline void treelist_node_delete(treelist *tl)
{
    __asm__ __volatile__ (
             "push %%edi \n\t":::);

    (*tl_node_delete_asm)(tl);

    __asm__ __volatile__ (
             "pop %%edi \n\t":::);
}

extern void (*tl_cur_beg_asm)(treelist *) __attribute__((__stdcall__));
///поставить курсор на первый узел
static inline void treelist_cursor_begin(treelist *tl)
{
    __asm__ __volatile__ (
             "push %%edi \n\t":::);

    (*tl_cur_beg_asm)(tl);

    __asm__ __volatile__ (
             "pop %%edi \n\t":::);
}

extern void (*tl_cur_next_asm)(treelist *) __attribute__((__stdcall__));
///перенести курсор на 1 позицию ниже
static inline void treelist_cursor_next(treelist *tl)
{
    __asm__ __volatile__ (
             "push %%ebx \n\t"
             "push %%esi \n\t"
             "push %%edi \n\t":::);

    (*tl_cur_next_asm)(tl);

    __asm__ __volatile__ (
             "pop %%edi \n\t"
             "pop %%esi \n\t"
             "pop %%ebx \n\t":::);
}

extern void (*tl_cur_perv_asm)(treelist *) __attribute__((__stdcall__));
///перенести курсор на 1 позицию выше
static inline void treelist_cursor_prev(treelist *tl)
{
    __asm__ __volatile__ (
             "push %%ebx \n\t"
             "push %%esi \n\t"
             "push %%edi \n\t":::);

    (*tl_cur_perv_asm)(tl);

    __asm__ __volatile__ (
             "pop %%edi \n\t"
             "pop %%esi \n\t"
             "pop %%ebx \n\t":::);
}

extern void (*tl_node_close_open_asm)(treelist *) __attribute__((__stdcall__));
///открыть/закрыть узел (работает с узлами которые имеют дочерние узлы)
static inline void treelist_close_open(treelist *tl)
{
    __asm__ __volatile__ (
             "push %%edi \n\t":::);

    (*tl_node_close_open_asm)(tl);

    __asm__ __volatile__ (
             "pop %%edi \n\t":::);
}

extern void (*tl_node_lev_inc_asm)(treelist *) __attribute__((__stdcall__));
///увеличить уровень
static inline void treelist_level_inc(treelist *tl)
{
    __asm__ __volatile__ (
             "push %%edi \n\t":::);

    (*tl_node_lev_inc_asm)(tl);

    __asm__ __volatile__ (
             "pop %%edi \n\t":::);
}

extern void (*tl_node_lev_dec_asm)(treelist *) __attribute__((__stdcall__));
///уменьшить уровень
static inline void treelist_level_dec(treelist *tl)
{
    __asm__ __volatile__ (
             "push %%edi \n\t":::);

    (*tl_node_lev_dec_asm)(tl);

    __asm__ __volatile__ (
             "pop %%edi \n\t":::);
}

extern treelist_node* (*tl_node_poi_get_info_asm)(treelist *, int node_ind) __attribute__((__stdcall__));
///взять указатель на структуру узла в указанной позиции
///input:
/// tlist - pointer to 'TreeList' struct
/// node_ind - node index
///output - pointer to node info or NULL
static inline treelist_node* treelist_getnode(treelist *tl, int node_ind)
{
    __asm__ __volatile__ (
             "push %%ebx \n\t"
             "push %%edi \n\t":::);

    treelist_node *ret =
    (*tl_node_poi_get_info_asm)(tl, node_ind);

    __asm__ __volatile__ (
             "pop %%edi \n\t"
             "pop %%ebx \n\t":::);
    return ret;
}

extern treelist_node* (*tl_node_poi_get_next_info_asm)(treelist *, treelist_node*) __attribute__((__stdcall__));
///взять указатель на следущую структуру узла
///input:
/// tlist - pointer to 'TreeList' struct
/// node_p - node param struct
///output - pointer to next node struct or NULL
static inline treelist_node* treelist_getnode_next(treelist *tl, treelist_node* node)
{
    __asm__ __volatile__ (
             "push %%ebx \n\t"
             "push %%edi \n\t":::);

    treelist_node *ret =
    (*tl_node_poi_get_next_info_asm)(tl, node);

    __asm__ __volatile__ (
             "pop %%edi \n\t"
             "pop %%ebx \n\t":::);
    return ret;
}

extern void* (*_tl_node_poi_get_data_asm)(treelist *, treelist_node*) __attribute__((__stdcall__));
///;взять указатель на данные узла
///input:
/// tlist - pointer to 'TreeList' struct
/// node_p - node param struct
///output - pointer
static inline void* treelist_getnode_data(treelist *tl, treelist_node *node)
{
    __asm__ __volatile__ (
             "push %%edi \n\t":::);

    void *ret =
    (*_tl_node_poi_get_data_asm)(tl, node);

    __asm__ __volatile__ (
             "pop %%edi \n\t":::);
    return ret;
}

extern int (*tl_save_mem_asm)(treelist *, int opt, void *h_mem, int mem_size) __attribute__((__stdcall__));
/// tlist - pointer to 'TreeList' struct
/// opt - options: 0 - first element, 1 - add next element
/// h_mem - pointer to memory
/// mem_size - memory size
///output - error code
static inline int treelist_save2mem(treelist *tl, int opt, void *h_mem, int mem_size)
{
    __asm__ __volatile__ (
             "push %%ebx \n\t"
             "push %%esi \n\t"
             "push %%edi \n\t":::);

    int ret =
    (*tl_save_mem_asm)(tl, opt, h_mem, mem_size);

    __asm__ __volatile__ (
             "pop %%edi \n\t"
             "pop %%esi \n\t"
             "pop %%ebx \n\t":::);
    return ret;
}

extern int (*_tl_load_mem_asm)(treelist *, int opt, void *h_mem, int mem_size) __attribute__((__stdcall__));
/**input:
; tlist - pointer to 'TreeList' struct
; opt   - options: element index + (2*(add mode)+(init mode)) shl 16, tl_load_mode_add        equ 0x20000 ;опция считывания в режиме добавления информации
; h_mem - pointer to memory
; mem_size - memory size
;   размер памяти, пока не используется (назначался для контроля)
;   для его использования нужно доработать функцию
;output:
; eax - error code
;memory header format:
;  +0 - (4) 'tree'
;  +4 - (2) info size
;  +6 - (4) count nodes
; +10 - (4) tlist style
; +14 - (4) cursor pos
; +18 - (2) info capt offs
; +20 - (2) info capt len
; +22 - (4) scroll pos
;memory data format:
; +26 - (info size + 8) * count nodes */
static inline int treelist_load4mem(treelist *tl, int opt, void *h_mem, int mem_size)
{
    __asm__ __volatile__ (
             "push %%ebx \n\t"
             "push %%esi \n\t"
             "push %%edi \n\t":::);

    int ret =
    (*_tl_load_mem_asm)(tl, opt, h_mem, mem_size);

    __asm__ __volatile__ (
             "pop %%edi \n\t"
             "pop %%esi \n\t"
             "pop %%ebx \n\t":::);
    return ret;
}

extern int (*tl_get_mem_size_asm)(treelist *, void *h_mem) __attribute__((__stdcall__));
/// ;берет размер памяти занятой функцией tl_save_mem при сохранении элементов
/// tlist - pointer to 'TreeList' struct
/// h_mem - pointer to saved memory
static inline int treelist_get_memsize(treelist *tl, void *h_mem)
{
    __asm__ __volatile__ (
             "push %%ebx \n\t"
             "push %%edi \n\t":::);

    int ret =
    (*tl_get_mem_size_asm)(tl, h_mem);

    __asm__ __volatile__ (
             "pop %%edi \n\t"
             "pop %%ebx \n\t":::);
    return ret;
}

#endif //KOLIBRI_TREELIST_H
