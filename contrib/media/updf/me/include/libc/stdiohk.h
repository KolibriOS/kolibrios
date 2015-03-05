/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_libc_stdiohk_h__
#define __dj_include_libc_stdiohk_h__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE

/* Force stdiohk.o to get linked in, and that module has the
   code for the stdio flush/fclose stuff. That .o causes the
   hook function to get initialized also. */

#include __DEV_CONFIG_H

#ifndef NEEDS_UNDERSCORES
__asm__(".long __stdio_cleanup_proc");
#else
__asm__(".long ___stdio_cleanup_proc");
#endif

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* __dj_include_libc_stdiohk_h__ */
