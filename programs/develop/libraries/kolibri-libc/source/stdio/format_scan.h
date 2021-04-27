#include <stdio.h>

typedef int (*virtual_getc)(void *sp, const void *obj);
typedef void (*virtual_ungetc)(void *sp, int c, const void *obj);

enum flags_t
{
        flag_unsigned   = 0x02,
        flag_register   = 0x04,
        flag_plus       = 0x08,
        flag_left_just  = 0x10,
        flag_lead_zeros = 0x20,
        flag_space_plus = 0x40,
        flag_hash_sign  = 0x80,
        flag_point      = 0x100
};

char *__scanf_buffer;

extern int _format_scan(const void *src, const char *fmt, va_list argp, virtual_getc vgetc, virtual_ungetc vungetc);