#ifndef KOLIBRI_KMENU_H
#define KOLIBRI_KMENU_H

#define KMENUITEM_NORMAL    0
#define KMENUITEM_SUBMENU   1
#define KMENUITEM_SEPARATOR 2

#define KMENUITEM_MAINMENU   0x80000000

#define KMENUITEM_SEPARATOR_WIDTH 10//170
#define KMENUITEM_SEPARATOR_HEIGHT 2

#define KMENU_LBORDER_SIZE 2
#define KMENU_DBORDER_SIZE 1

extern int kolibri_kmenu_init(void); 

extern void (*kmainmenu_draw)(void *) __attribute__((__stdcall__));
extern void (*kmainmenu_dispatch_cursorevent)(void *) __attribute__((__stdcall__));
extern void (*kmenu_init)(void *) __attribute__((__stdcall__));
extern void* (*ksubmenu_new)() __attribute__((__stdcall__));
extern void (*ksubmenu_add)(void *, void *) __attribute__((__stdcall__));
extern void* (*kmenuitem_new)(uint32_t, const char *, uint32_t) __attribute__((__stdcall__));
extern void* (*kmenuitem__submenu_new)(uint32_t, const char *, void *) __attribute__((__stdcall__));

#endif /* KOLIBRI_KMENU_H */
