/* Copyright (C) 2019-2021 Logaev Maxim (turbocat2001), GPLv3 */

#include "tinypy.h"
#include <kos32sys.h>

#define GET_NUM_ARG() TP_TYPE(TP_NUMBER).number.val
#define GET_STR_ARG() TP_TYPE(TP_STRING).string.val

void debug_write_byte(const char ch){
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(63), "b"(1), "c"(ch)
    );
}

static tp_obj _debug_print(TP){
    tp_obj str = TP_TYPE(TP_STRING);
    for(int i=0; i < str.string.len; i++)
    {
       debug_write_byte(str.string.val[i]); 
    }
    return tp_None;
} 

static tp_obj _start_draw(TP){
    begin_draw();
    return tp_None;
}


static tp_obj _end_draw(TP){
    end_draw();
    return tp_None;
}

static tp_obj _create_window(TP){
    int x = GET_NUM_ARG();
    int y = GET_NUM_ARG();
    int w = GET_NUM_ARG(); 
    int h = GET_NUM_ARG();
    const char *title= GET_STR_ARG();
    unsigned int color = GET_NUM_ARG();
    unsigned int style = GET_NUM_ARG();
    sys_create_window(x,y,w,h, title, color,style);
    return tp_None;
} 

static tp_obj _create_button(TP){
    unsigned int x = GET_NUM_ARG();
    unsigned int y = GET_NUM_ARG();
    unsigned int h = GET_NUM_ARG();
    unsigned int w = GET_NUM_ARG();
    unsigned int id = GET_NUM_ARG();
    unsigned int color = GET_NUM_ARG();
    define_button((x << 16) + w, (y << 16) + h, id, color);
    return tp_None;
}

static tp_obj _draw_text(TP){
    const char *str= GET_STR_ARG();
    int x = GET_NUM_ARG();
    int y = GET_NUM_ARG();
    int len = GET_NUM_ARG(); 
    unsigned color = (unsigned)GET_NUM_ARG();
    draw_text_sys(str, x, y, len, color);
    return tp_None;
}

static tp_obj _get_event(TP){
    return tp_number(get_os_event());
}

static tp_obj _get_button(TP){
    return tp_number(get_os_button());
}

static tp_obj _get_key(TP){
    tp_obj key_obj = tp_dict(tp); 
    oskey_t key_info = get_key();
    tp_set(tp, key_obj, tp_string("code"), tp_number(key_info.code));
    tp_set(tp, key_obj, tp_string("ctrl_key"), tp_number(key_info.ctrl_key)); 
    tp_set(tp, key_obj, tp_string("state"), tp_number(key_info.state));
    tp_set(tp, key_obj, tp_string("val"), tp_number(key_info.val));
    return key_obj;
}

static tp_obj _start_app(TP){
     const char *prog_name = GET_STR_ARG();
     const char *args = GET_STR_ARG();
}

static tp_obj _get_sys_colors(TP){
    tp_obj color_obj = tp_dict(tp);
    struct kolibri_system_colors colors;
    get_system_colors(&colors); 
    tp_set(tp, color_obj, tp_string("frame_area"), tp_number(colors.frame_area));
    tp_set(tp, color_obj, tp_string("grab_bar"), tp_number(colors.grab_bar));
    tp_set(tp, color_obj, tp_string("grab_bar_button)"), tp_number(colors.grab_bar_button));
    tp_set(tp, color_obj, tp_string( "grab_button_text)"), tp_number(colors.grab_button_text));
    tp_set(tp, color_obj, tp_string("grab_text"), tp_number(colors.grab_text));
    tp_set(tp, color_obj, tp_string("work_area"), tp_number(colors.work_area));
    tp_set(tp, color_obj, tp_string("work_button"), tp_number(colors.work_button));
    tp_set(tp, color_obj, tp_string("work_button_text"), tp_number(colors.work_button_text));
    tp_set(tp, color_obj, tp_string("work_graph"), tp_number(colors.work_graph));
    tp_set(tp, color_obj, tp_string("work_text"), tp_number(colors.work_text));
    return color_obj;
}
