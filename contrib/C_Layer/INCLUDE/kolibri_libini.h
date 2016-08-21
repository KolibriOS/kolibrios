#ifndef KOLIBRI_LIBINI_H
#define KOLIBRI_LIBINI_H

extern int kolibri_libini_init(void);

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
