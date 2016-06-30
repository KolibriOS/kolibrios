#ifndef KOLIBRI_KMENU_H
#define KOLIBRI_KMENU_H

extern int init_kmenu_asm(void); 

int kolibri_kmenu_init(void)
{
  int asm_init_status = init_kmenu_asm();
  
  /* just return asm_init_status? or return init_boxlib_asm() ?*/

  if(asm_init_status == 0)
    return 0;
  else
    return 1;
}



extern void (*kmainmenu_draw)(void *) __attribute__((__stdcall__));
extern void (*kmainmenu_dispatch_cursorevent)(void *) __attribute__((__stdcall__));
extern void (*kmenu_init)(void *) __attribute__((__stdcall__));
extern void* (*ksubmenu_new)() __attribute__((__stdcall__));
extern void (*ksubmenu_add)(void *, void *) __attribute__((__stdcall__));
extern void* (*kmenuitem_new)(uint32_t, const char *, uint32_t) __attribute__((__stdcall__));
extern void* (*kmenuitem__submenu_new)(uint32_t, const char *, void *) __attribute__((__stdcall__));

#endif /* KOLIBRI_KMENU_H */
