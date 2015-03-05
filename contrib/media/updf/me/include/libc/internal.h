/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_libc_internal_h__
#define __dj_include_libc_internal_h__

#ifdef __cplusplus
extern "C" {
#endif
void __crt1_startup(void);
void __main(void);
void _npxsetup(char *argv0);
void __emu387_exception_handler(void);
void __djgpp_exception_processor(void);
void __djgpp_exception_setup(void);

static inline int str_check_ptr(void * ptr)
{
 unsigned long p=(unsigned long)ptr;
 if(p<64 || p>(64*1024*1024)) return 0;
 return 1;
}

#ifdef __cplusplus
}
#endif

#endif /* __dj_include_libc_internal_h__ */
