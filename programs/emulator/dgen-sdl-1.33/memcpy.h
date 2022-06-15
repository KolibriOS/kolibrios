/* Header for optimized implementations of memcpy() */

#ifndef MEMCPY_H_
#define MEMCPY_H_

//#ifdef __cplusplus
//#include <cstring>
#define BEGIN_C_DECL extern "C" {
#define END_C_DECL }
//#else*/
#include <string.h>
//#define BEGIN_C_DECL
//#define END_C_DECL
//#endif

BEGIN_C_DECL

#if defined(WITH_X86_MMX)

#undef memcpy
#define memcpy(a, b, c) mmx_memcpy(a, b, c)
extern void mmx_memcpy(void *, void *, size_t);

#elif defined(WITH_X86_ASM)

#undef memcpy
#define memcpy(a, b, c) asm_memcpy(a, b, c)
extern void asm_memcpy(void *, void *, size_t);

#endif

END_C_DECL

#endif /* MEMCPY_H_ */
