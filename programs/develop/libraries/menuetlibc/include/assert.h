/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#undef assert
#undef unimpl

#ifdef NDEBUG
#define assert(test) /*nothing*/
#else
#define assert(test) ((void)((test)||(__dj_assert(#test,__FILE__,__LINE__),0)))
#endif
#define unimpl()       __dj_unimp("Called unimplemented function in file \"" __FILE__ "\"\n")

#ifndef __dj_include_assert_h_
#define __dj_include_assert_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
void __declspec(noreturn) __dj_assert(const char *,const char *,int);
void __declspec(noreturn) __dj_unimp(const char *fn);
#else
void	__dj_assert(const char *,const char *,int) __attribute__((__noreturn__));
void 	__dj_unimp(const char *fn) __attribute__((__noreturn__));
#endif

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_assert_h_ */

#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */
