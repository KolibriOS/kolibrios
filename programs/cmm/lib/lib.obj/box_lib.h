//BOX_LIB - Asper
dword boxlib = #aEdit_box_lib;
char aEdit_box_lib[22]="/sys/lib/box_lib.obj\0";

dword box_lib_init   = #aboxlib_init;

dword edit_box_draw  = #aEdit_box_draw;
dword edit_box_key   = #aEdit_box_key;
dword edit_box_mouse = #aEdit_box_mouse;
dword version_ed     = #aVersion_ed;

dword scrollbar_v_draw  = #aScrollbar_v_draw;
dword scrollbar_v_mouse = #aScrollbar_v_mouse;
dword scrollbar_h_draw  = #aScrollbar_h_draw;
dword scrollbar_h_mouse = #aScrollbar_h_mouse;
dword version_scrollbar = #aVersion_scrollbar;

dword  am__ = 0x0;
dword  bm__ = 0x0;

char aEdit_box_draw[9]   = "edit_box\0";
char aEdit_box_key[13]   = "edit_box_key\0";
char aEdit_box_mouse[15] = "edit_box_mouse\0";
char aVersion_ed[11]     = "version_ed\0";

char aboxlib_init[9]        = "lib_init\0";
char aScrollbar_v_draw[17]  = "scrollbar_v_draw\0";
char aScrollbar_v_mouse[18] = "scrollbar_v_mouse\0";
char aScrollbar_h_draw[17]  = "scrollbar_h_draw\0";
char aScrollbar_h_mouse[18] = "scrollbar_h_mouse\0";
char aVersion_scrollbar[18] = "version_scrollbar\0";

char aCheck_box_draw  [15] = "check_box_draw\0";
char aCheck_box_mouse [16] = "check_box_mouse\0";
char aVersion_ch      [11] = "version_ch\0";

char aOption_box_draw [16] = "option_box_draw\0";
char aOption_box_mouse[17] = "option_box_mouse\0";
char aVersion_op      [11] = "version_op\0" ;


struct edit_box{
dword width, left, top, color, shift_color, focus_border_color, blur_border_color,
text_color, max, text, mouse_variable, flags, size, pos, offset, cl_curs_x, cl_curs_y, shift, shift_old;
};

struct scroll_bar
{
word size_x,//  equ [edi]     
start_x,//      equ [edi+2]
size_y,//		equ [edi+4]
start_y;//		equ [edi+6]
dword btn_height, //equ [edi+8]
type,//			equ [edi+12]
max_area,//		equ [edi+16]
cur_area,//		equ [edi+20]
position,//		equ [edi+24]
bckg_col,//		equ [edi+28]
frnt_col,//		equ [edi+32]
line_col,//		equ [edi+36]
redraw;//		equ [edi+40]
word delta,//		equ [edi+44]
delta2,//		equ [edi+46]
r_size_x,//		equ [edi+48]
r_start_x,//	equ [edi+50]
r_size_y,//		equ [edi+52]
r_start_y;//	equ [edi+54]
dword m_pos,//		equ [edi+56]
m_pos_2,//		equ [edi+60]
m_keys,//		equ [edi+64]
run_size,//		equ [edi+68]
position2,//	equ [edi+72]
work_size,//	equ [edi+76]
all_redraw,//	equ [edi+80]
ar_offset;//	equ [edi+84]
};

//ed_width        equ [edi]               ;ширина компонента
//ed_left         equ [edi+4]             ;положение по оси х
//ed_top          equ [edi+8]             ;положение по оси у
//ed_color        equ [edi+12]            ;цвет фона компонента
//shift_color     equ [edi+16]            ;=0x6a9480
//ed_focus_border_color   equ [edi+20]    ;цвет рамки компонента
//ed_blur_border_color    equ [edi+24]    ;цвет не активного компонента
//ed_text_color   equ [edi+28]            ;цвет текста
//ed_max          equ [edi+32]            ;кол-во символов которые можно максимально ввести
//ed_text         equ [edi+36]            ;указатель на буфер
//ed_flags        equ [edi+40]            ;флаги
//ed_size equ [edi+42]                    ;кол-во символов
//ed_pos  equ [edi+46]                    ;позиция курсора
//ed_offset       equ [edi+50]            ;смещение
//cl_curs_x       equ [edi+54]            ;предыдущая координата курсора по х
//cl_curs_y       equ [edi+58]            ;предыдущая координата курсора по у
//ed_shift_pos    equ [edi+62]            ;положение курсора
//ed_shift_pos_old equ [edi+66]           ;старое положение курсора
