#include "tp.h"
#include <menuet/thread.h>

typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;

extern void _tp_raise(TP,tp_obj);
extern tp_obj tp_dict(TP);
extern tp_obj tp_method(TP,tp_obj self,tp_obj v(TP));
extern tp_obj tp_number(tp_num v);
extern tp_obj tp_list(TP);
extern void _tp_list_append(TP,_tp_list *self, tp_obj v);
extern tp_obj tp_call(TP, const char *mod, const char *fnc, tp_obj params);
extern void _tp_call(TP,tp_obj *dest, tp_obj fnc, tp_obj params);
extern int tp_bool(TP,tp_obj v);
extern tp_obj tp_has(TP,tp_obj self, tp_obj k);
#define _cdecl __attribute__((cdecl))
extern int (* _cdecl con_printf)(const char* format,...);
static tp_obj kolibri_show(TP)
{
    tp_obj self = TP_TYPE(TP_DICT);
    uint16_t xpos = (uint16_t)(tp_get(tp, self, tp_string("x")).number.val);
    uint16_t ypos = (uint16_t)tp_get(tp, self, tp_string("y")).number.val;
    uint16_t height = (uint16_t)tp_get(tp, self, tp_string("height")).number.val;
    uint16_t width = (uint16_t)tp_get(tp, self, tp_string("width")).number.val;
    uint16_t fixedsize = (uint16_t)tp_get(tp, self, tp_string("fixedsize")).number.val;
    uint32_t bgcolor = (uint32_t)tp_get(tp, self, tp_string("bgcolor")).number.val;
    uint32_t status;
    uint32_t style;
    uint32_t x = xpos * 0x10000 + width;
    uint32_t y = ypos * 0x10000 + height;
    if (fixedsize)
        style = 0;
    else
        style = 0x33000000 + (bgcolor & 0xFFFFFF);
    asm volatile ("int $0x40"::"a"(12), "b"(1));
    asm volatile ("int $0x40"::
                  "a"(0), "b"(x), "c"(y), "d"(style),
                  "S"(0), "D"(0));
    asm volatile ("int $0x40"::"a"(12), "b"(2));
    /* If window has additional handler, run it. */
    if (tp_bool(tp, tp_has(tp, self, tp_string("on_show"))))
    {
        tp_obj result;
        tp_obj fnc = tp_get(tp, self, tp_string("on_show"));
        tp_obj param_list = tp_list(tp); /* Prepare parameters. */
        _tp_list_append(tp, param_list.list.val, self);
        _tp_call(tp, &result, fnc, param_list);
    }
    return tp_None;
}

static void window_function(void)
{
    uint32_t ev;
    /* Wait for event. */
    do {
        asm volatile("int $0x40":"=a"(ev):"a"(10));
    } while(ev != 3);
    asm volatile("int $040"::"a"(-1));
}

static tp_obj kolibri_default_handler(TP)
{
    return tp_None;
}

/* Run window_function() in separated thread. */
static tp_obj kolibri_run(TP)
{
    tp_obj self = TP_TYPE(TP_DICT);
    tp_obj redraw = tp_get(tp, self, tp_string("show"));
    tp_obj result;
    tp_obj key_handler = tp_None;
    tp_obj button_handler = tp_None;
    int    button_id;
    uint32_t ev;
    int    leave=0;
    tp_obj param_list;
    /* Obtain handlers. */
    if (tp_bool(tp, tp_has(tp, self, tp_string("on_key"))))
        key_handler = tp_get(tp, self, tp_string("on_key"));
    if (tp_bool(tp, tp_has(tp, self, tp_string("on_button"))))
        button_handler = tp_get(tp, self, tp_string("on_button"));

    while(!leave){
        asm volatile("int $0x40":"=a"(ev):"a"(10));
        switch (ev)
        {
            case 1:
                _tp_call(tp, &result, redraw, tp_None);
                break;
            case 2:
                if (key_handler.type == TP_FNC)
                {
                    param_list = tp_list(tp); /* Prepare parameters. */
                    _tp_list_append(tp, param_list.list.val, self);
                    _tp_list_append(tp, param_list.list.val, tp_number(__menuet__getkey()));
                    _tp_call(tp, &result, key_handler, param_list);
                }
                break;
            case 3:
                button_id = __menuet__get_button_id();
                if (button_id == 1)
                    leave = 1;
                else if (button_handler.type == TP_FNC)
                {
                    param_list = tp_list(tp); /* Prepare parameters. */
                    _tp_list_append(tp, param_list.list.val, self);
                    _tp_list_append(tp, param_list.list.val, tp_number(button_id));
                    _tp_call(tp, &result, button_handler, param_list);
                }
                break;
            default:
                con_printf("Got unknown event %d\n", ev);
                break;
        }
    };
    return tp_None;
}

static tp_obj kolibri_print_text(TP)
{
    tp_obj self = TP_TYPE(TP_DICT);
    uint32_t textcolor = (uint32_t)tp_get(tp, self, tp_string("textcolor")).number.val;
    uint16_t x = (uint16_t)tp_get(tp, self, tp_string("curx")).number.val;
    uint16_t y = (uint16_t)tp_get(tp, self, tp_string("cury")).number.val;
    uint32_t ofs;
    uint32_t width = (uint32_t)tp_get(tp, self, tp_string("width")).number.val;
    tp_obj text = TP_TYPE(TP_STRING);

    __menuet__write_text(x, y, textcolor, (char *)text.string.val, text.string.len);
    /* Update cursor position. */
    ofs = 6 * text.string.len;
    tp_set(tp, self, tp_string("cury"), tp_number(y + 9 * ((x + ofs) / width)));
    tp_set(tp, self, tp_string("curx"), tp_number((x + ofs)%width));

    return tp_None;
}

tp_obj kolibri_mainwindow(TP)
{
    tp_obj obj = tp_dict(tp);
    obj = tp_dict(tp);
    tp_set(tp, obj, tp_string("x"), TP_TYPE(TP_NUMBER));
    tp_set(tp, obj, tp_string("y"), TP_TYPE(TP_NUMBER));
    tp_set(tp, obj, tp_string("height"), TP_TYPE(TP_NUMBER));
    tp_set(tp, obj, tp_string("width"), TP_TYPE(TP_NUMBER));
    tp_set(tp, obj, tp_string("curx"), tp_number(0));
    tp_set(tp, obj, tp_string("cury"), tp_number(0));
    tp_set(tp, obj, tp_string("fixedsize"), TP_TYPE(TP_NUMBER));
    tp_set(tp, obj, tp_string("textcolor"), tp_number(0x202020));
    tp_set(tp, obj, tp_string("bgcolor"), tp_number(0xFFFFFF));
    tp_set(tp, obj, tp_string("show"), tp_method(tp, obj, kolibri_show));
    tp_set(tp, obj, tp_string("run"), tp_method(tp, obj, kolibri_run));
    /*tp_set(tp, obj, tp_string("keyhandler"), tp_method(tp, obj, kolibri_default_handler));
    tp_set(tp, obj, tp_string("buttonhandler"), tp_method(tp, obj, kolibri_default_handler));*/
    tp_set(tp, obj, tp_string("print_text"), tp_method(tp, obj, kolibri_print_text));
    return obj;
}
