#ifndef _STDDEF_H_
#define _STDDEF_H_

typedef __SIZE_TYPE__ size_t;
typedef __PTRDIFF_TYPE__ ssize_t;
typedef __WCHAR_TYPE__ wchar_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;
typedef __PTRDIFF_TYPE__ intptr_t;
typedef __SIZE_TYPE__ uintptr_t;

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifdef _BUILD_LIBC

/* FIXME: Now _FUNC is used in conio only.
          This should be removed after revrite somme apps who use conio
          (because these app use pointer dereferencing for conio)
*/
#define _FUNC(func) (func)
 
#define DLLAPI //__attribute__((dllexport)) // Comming soon(tcc not support yet)
#else
#define _FUNC(func) (*func) // FIXME: this needed for legacy reason (see above)
#define DLLAPI __attribute__((dllimport))
#endif

#define offsetof(type, field) ((size_t) & ((type*)0)->field)

#ifndef __stdcall
#define __stdcall __attribute__((stdcall))
#endif

#ifndef __cdecl
#define __cdecl __attribute__((cdecl))
#endif

#endif /* _STDDEF_H_ */