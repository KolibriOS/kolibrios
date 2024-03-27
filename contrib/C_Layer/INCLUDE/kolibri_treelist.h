#ifndef KOLIBRI_TREELIST_H
#define KOLIBRI_TREELIST_H


/// ��������� �����
enum tl_style {
    TL_KEY_NO_EDIT  = 1,    // ������� ������ ������������� �� ���������� (�������� ������ ���������, ������� DEL)
    TL_DRAW_PAR_LINE = 2,   // �������� ����� � ������������� ����
    TL_LISTBOX_MODE  = 4    //����� �� ���������� ������ (��� � ListBox ��� ������ ������)
};

/// ��������� ��� �������
enum tl_err {
    TL_ERR_LOAD_CAPTION     = 1, //� ������ ��� ��������� 'tree'
    TL_ERR_SAVE_MEMOTY_SIZE = 2, //�� ������� ������ ��� ���������� ��������
    TL_ERR_LOAD_INFO_SIZE   = 4, //�� ��������� ������ �������������� ��������� ��� ��������
};

typedef struct __attribute__ ((__packed__)) {
    uint16_t    type;   //��� ��������, ��� ������ ������ ��� ����
    uint8_t     lev;    //������� ��������
    uint8_t     clo;    //���� ��������, ��� �������� (����� ����� ��� ������������� ����)
    uint32_t    prev;   //������ ����������� ��������
    uint32_t    next;   //������ ������������ ��������
    uint32_t    tcreat; //����. ��������
    uint32_t    tdel;   //����. ��������
} treelist_node;


typedef struct __attribute__ ((__packed__)) {
    uint32_t    left;
    uint32_t    top;
    uint32_t    width;
    uint32_t    height;
    void       *data_info;  // ��������� �� �������� �����
    uint16_t    info_size;  // ������ ������ ���������� ��� ������� ���� (����������������� ������ + ����� ��� �������)
    uint32_t    info_max_count; // ������������ ���������� �����, ������� ����� �������� � �������
    uint32_t    style;      // ����� ��������
    treelist_node *data_nodes; // ��������� �� ��������� �����
    void       *data_img;   // ��������� �� ����������� � �������� �����
    uint16_t    img_cx;     // ������ ������
    uint16_t    img_cy;     // ������ ������
    void       *data_img_sys;//��������� �� ��������� ����������� (�������, �������)
    uint32_t    ch_tim;     // ���������� ��������� � �����
    uint32_t    tim_undo;   // ���������� ���������� ��������
    uint32_t    cur_pos;    // ������� �������
    color_t     col_bkg;    // ���� ����
    color_t     col_zag; // ���� ���������
    color_t     col_txt; // ���� ������
    uint16_t    capt_cy;    // ������ �������
    uint16_t    info_capt_offs;//����� ��� ������ ������ (������� ����)
    uint16_t    info_capt_len;//����� ������ ������� ���� (���� = 0 �� �� ����� ���������)
    void       *el_focus;   // ��������� �� ��������� �������� � ������
    scrollbar  *p_scroll;   // ��������� �� ��������� ����������
    void       *on_press;   // +84 ��������� �� �������, ������� ���������� ��� ������� Enter
} treelist;

/// @note capt_cy may be 0 = no caption
/// @note if icon_size is 16x16, and data_img id NULL, no icons - useful in list mode
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
    tl->capt_cy = capt_cy;
    tl->p_scroll = kolibri_new_scrollbar_def(X_Y(0, 16), X_Y(0, 0), 100, 30, 0);
    return tl;
}

static inline void gui_add_treelist(kolibri_window *wnd, treelist* tl)
{
    kolibri_window_add_element(wnd, KOLIBRI_TREELIST, tl);
}


///������� �� ����
extern void (*tl_mouse)(treelist *) __attribute__((__stdcall__));

///����� ������ �� �����
extern void (*tl_draw)(treelist *) __attribute__((__stdcall__));
__attribute__((__stdcall__)) static inline void treelist_draw(treelist *tl)
{
    tl->p_scroll->all_redraw = 1;
    (*tl_draw)(tl);
}


///���������� ���� �����
extern void (*tl_node_move_up)(treelist *) __attribute__((__stdcall__));

///���������� ���� ����
extern void (*tl_node_move_down)(treelist *) __attribute__((__stdcall__));

///��������� ������ ��� �������� ������ � �������� ���������� (�����������)
extern void (*tl_data_init)(treelist *) __attribute__((__stdcall__));


///������� ������ �������� (����������)
extern void (*tl_data_clear)(treelist *) __attribute__((__stdcall__));

static inline void treelist_data_clear(treelist *tl)
{
    (*tl_data_clear)(tl);
    free(tl->p_scroll);
}

///������� ������ (����������)
extern void (*tl_info_clear)(treelist *) __attribute__((__stdcall__));

extern void (*tl_key_asm)(treelist *) __attribute__((__stdcall__));

///������� �� ����������
__attribute__((__stdcall__)) static inline void treelist_key(treelist *tl, oskey_t code)
{
    __asm__ __volatile__ (
             "push %2\n\t"
             "call *%1 \n\t"::"a"(code.val), "m"(tl_key_asm), "m"(tl):);  // indirect call with asterisk *

//    (*tl_key_asm)(tl);
}

///������ ��������
extern void (*tl_info_undo)(treelist *) __attribute__((__stdcall__));

///������ ��������
extern void (*tl_info_redo)(treelist *) __attribute__((__stdcall__));

extern void (*tl_node_add)(treelist *, uint32_t n_opt, void *n_info) __attribute__((__stdcall__));

///�������� ����
/// @param tlist - ��������� �� ��������� �����
/// @param n_opt - ����� ����������
/// @param n_info - ��������� �� ����������� ������
static inline void treelist_node_add(treelist *tlist, void *n_info, uint16_t type, uint8_t clos, uint8_t lev)
{
    uint32_t    n_opt = (type << 16) | (clos << 8) | lev;
    (*tl_node_add)(tl, n_opt, n_info);
}

///�������� � ������� ����
/// @param tlist - ��������� �� ��������� �����
/// @param n_info - ��������� �� ������
extern void (*tl_node_set_data)(treelist * tlist, void *n_info) __attribute__((__stdcall__));

///����� ��������� �� ������ ���� ��� ��������
extern void* (*tl_node_get_data)(treelist *) __attribute__((__stdcall__));

///������� ���� ��� ��������
extern void (*tl_node_delete)(treelist *) __attribute__((__stdcall__));

///��������� ������ �� ������ ����
extern void (*tl_cur_beg)(treelist *) __attribute__((__stdcall__));

///��������� ������ �� 1 ������� ����
extern void (*tl_cur_next)(treelist *) __attribute__((__stdcall__));

///��������� ������ �� 1 ������� ����
extern void (*tl_cur_perv)(treelist *) __attribute__((__stdcall__));

///�������/������� ���� (�������� � ������ ������� ����� �������� ����)
extern void (*tl_node_close_open)(treelist *) __attribute__((__stdcall__));

///��������� �������
extern void (*tl_node_lev_inc)(treelist *) __attribute__((__stdcall__));

///��������� �������
extern void (*tl_node_lev_dec)(treelist *) __attribute__((__stdcall__));

/// @brief ����� ��������� �� ��������� ���� � ��������� �������
/// @param tlist pointer to 'TreeList' struct
/// @param node_ind node index
/// @return pointer to node info or NULL
extern treelist_node* (*tl_node_poi_get_info)(treelist * tlist, int node_ind) __attribute__((__stdcall__));

///����� ��������� �� �������� ��������� ����
/// @param tlist pointer to 'TreeList' struct
/// @param node_p node param struct
/// @return pointer to next node struct or NULL
extern treelist_node* (*tl_node_poi_get_next_info)(treelist *tlist, treelist_node *node_p) __attribute__((__stdcall__));

/// @brief ����� ��������� �� ������ ����
/// @param tlist pointer to 'TreeList' struct
/// @param node_p - node param struct
/// @return pointer
extern void* (*_tl_node_poi_get_data)(treelist *tlist, treelist_node *node_p) __attribute__((__stdcall__));

/// @param tlist pointer to 'TreeList' struct
/// @param opt options: 0 - first element, 1 - add next element
/// @param h_mem pointer to memory
/// @param mem_size memory size
/// @return error code
extern int (*tl_save_mem)(treelist *tlist, int opt, void *h_mem, int mem_size) __attribute__((__stdcall__));

/**input:
; tlist - pointer to 'TreeList' struct
; opt   - options: element index + (2*(add mode)+(init mode)) shl 16, tl_load_mode_add        equ 0x20000 ;����� ���������� � ������ ���������� ����������
; h_mem - pointer to memory
; mem_size - memory size
;   ������ ������, ���� �� ������������ (���������� ��� ��������)
;   ��� ��� ������������� ����� ���������� �������
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
extern int (*_tl_load_mem)(treelist *, int opt, void *h_mem, int mem_size) __attribute__((__stdcall__));

/// @brief ����� ������ ������ ������� �������� tl_save_mem ��� ���������� ���������
/// @param tlist pointer to 'TreeList' struct
/// @param h_mem pointer to saved memory
extern int (*tl_get_mem_size)(treelist *tlist, void *h_mem) __attribute__((__stdcall__));

#endif //KOLIBRI_TREELIST_H
