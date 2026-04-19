#ifndef KOLIBRI_MSGBOX_H
#define KOLIBRI_MSGBOX_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint8_t     retval;  // 0 - win closed, 1 to n - button num, also default button on start
    uint8_t     reserv;
    char        texts[2048];      // must be enough ;-)
    char        msgbox_stack[1024];
    uint32_t    top_stack;
}__attribute__((packed)) msgbox;

typedef void (*msgbox_callback)(void);

DLLAPI void __stdcall mb_create(msgbox *, void *thread); // clears callbacks, ! if fix lib, we can return eax as of Fn51
DLLAPI void __stdcall mb_setfunctions(msgbox_callback*); // must be called immediately after create, zero-ended array
DLLAPI void __stdcall mb_reinit(msgbox *);  // recalc sizes when structure changes, called auto when MsgBoxCreate

static inline msgbox* kolibri_new_msgbox(char* title, char* text, int def_but, ...)
/// text can be multilined by code 13 = "\r"
/// def_but - highlighted and used on Enter (if zero - default is [X]), user may use Tabs or Arrows
/// last params are buttons text, max 8. last must set as NULL
{
    va_list vl = 0;
    va_start(vl, def_but);
    msgbox* box = calloc(sizeof(msgbox), 1);
    box->retval = (uint8_t)def_but;
    char    *pc = box->texts;
    strcpy(pc, title);
    pc += strlen(title) + 1;
    strcpy(pc, text);
    pc += strlen(text) + 1;
    char  *but_text = va_arg(vl, char*);
    while (but_text)
    {
        strcpy(pc, but_text);
        pc += strlen(but_text) + 1;
        but_text = va_arg(vl, char*);
    }

    va_end(vl);
    return box;
}

static inline void kolibri_start_msgbox(msgbox* box, msgbox_callback cb[])
{
    mb_create(box, &box->top_stack);
    if (cb) mb_setfunctions(cb);
}

#endif
