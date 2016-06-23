#ifndef KOLIBRI_LIBINI_H
#define KOLIBRI_LIBINI_H

extern int init_libini_asm(void); 

int kolibri_libini_init(void)
{
  int asm_init_status = init_libini_asm();
  
  /* just return asm_init_status? or return init_boxlib_asm() ?*/

  if(asm_init_status == 0)
    return 0;
  else
    return 1;
}

extern uint32_t (*LIBINI_enum_sections)(const char*, void*) __attribute__((__stdcall__));
extern uint32_t (*LIBINI_enum_keys)(const char*, const char*, void*) __attribute__((__stdcall__));

extern uint32_t (*LIBINI_get_str)(const char*, const char*, const char*, char*, uint32_t) __attribute__((__stdcall__));
extern uint32_t (*LIBINI_set_str)(const char*, const char*, const char*, const char*, uint32_t) __attribute__((__stdcall__));
extern uint32_t (*LIBINI_get_int)(const char*, const char*, const char*, uint32_t) __attribute__((__stdcall__));
extern uint32_t (*LIBINI_set_int)(const char*, const char*, const char*, uint32_t) __attribute__((__stdcall__));
extern uint32_t (*LIBINI_get_color)(const char*, const char*, const char*, uint32_t) __attribute__((__stdcall__));
extern uint32_t (*LIBINI_set_color)(const char*, const char*, const char*, uint32_t) __attribute__((__stdcall__));
extern uint32_t (*LIBINI_get_shortcut)(const char*, const char*, const char*, uint32_t, const char*, uint32_t) __attribute__((__stdcall__));

#endif /* KOLIBRI_LIBINI_H */
