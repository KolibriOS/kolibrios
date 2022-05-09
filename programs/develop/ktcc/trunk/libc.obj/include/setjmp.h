#ifndef _SETJMP_H_
#define _SETJMP_H_

#include <stddef.h>

typedef unsigned long __jmp_buf[6];

typedef struct __jmp_buf_tag {
    __jmp_buf __jb;
    unsigned long __fl;
    unsigned long __ss[128 / sizeof(long)];
} jmp_buf[1];

DLLAPI int setjmp(jmp_buf env);
DLLAPI void longjmp(jmp_buf env, int val);

#endif // _SETJMP_H_
