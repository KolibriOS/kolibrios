/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stddef.h>

#ifndef unconst
#define unconst(__v, __t) __extension__({union { const __t __cp; __t __p; } __q; __q.__cp = __v; __q.__p; })
#endif

long double strntold(const char* s, char** sret, size_t n)
{
    long double r; /* result */
    int e; /* exponent */
    long double d; /* scale */
    int sign; /* +- 1.0 */
    int esign;
    int i;
    int flags = 0;
    const char* start = s; /* original ptr; endptr on no conversion (per strtod) */

    r = 0.0;
    sign = 1;
    e = 0;
    esign = 1;

    while (((*s == ' ') || (*s == '\t')) && n)
    {
        s++;
        n--;
    }    

    if (n) 
    {
        if (*s == '+')
        {
            s++;
            n--;
        }
        else if (*s == '-') {
            sign = -1;
            s++;
            n--;
        }
    }

    while ((*s >= '0') && (*s <= '9') && n) {
        flags |= 1;
        r *= 10.0;
        r += *s - '0';
        s++;
        n--;
    }

    if (*s == '.' && n) {
        d = 0.1L;
        s++;
        n--;

        while ((*s >= '0') && (*s <= '9') && n) {
            flags |= 2;
            r += d * (*s - '0');
            s++;
            d *= 0.1L;
            n--;
        }
    }

    if (flags == 0) {
        if (sret)
            *sret = unconst(start, char*);
        return 0;
    }

    if (((*s == 'e') || (*s == 'E')) && n) {
        const char* exp_pos = s; /* rewind here if the exponent is malformed */
        s++;
        n--;
        if ((*s == '+') && n)
        {
            s++;
            n--;
        }
        else if ((*s == '-') && n) {
            s++;
            esign = -1;
            n--;
        }

        if ((*s < '0') || (*s > '9') || !n) {
            if (sret)
                *sret = unconst(exp_pos, char*);
            return r * sign;
        }

        while ((*s >= '0') && (*s <= '9') && n) {
            e *= 10;
            e += *s - '0';
            s++;
            n--;
        }
    }
    if (esign < 0)
        for (i = 1; i <= e; i++)
            r *= 0.1L;
    else
        for (i = 1; i <= e; i++)
            r *= 10.0;

    if (sret)
        *sret = unconst(s, char*);
    return r * sign;
}
