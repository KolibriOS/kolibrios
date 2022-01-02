/*
        function for format output to the string. much lighter than standard sprintf
        because of lesser formats supported
*/


#include <string.h>
//#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

char* __itoa(int n,char* s);
char* itoab(unsigned int n, char* s, int  b);

int tiny_vsnprintf (char * s, size_t n, const char * format, va_list args )
// support %c, %s, %d, %x, %u, %% for 32-bit values only. no width specs, left align
// always zero-ended
{
    char *fmt, *dest, buf[32];
    fmt = (char*)format;
    dest = s; dest[n - 1] = '\0';
    int     arg, len;
    while (*fmt && (dest - s < n - 1))
    {
        if (*fmt != '%')
        {
            *dest++ = *fmt++;
            continue;
        }
        if (fmt[1] == '%') // %%
        {
            *dest++ = '%';
            fmt += 2;
            continue;
        }
        arg = va_arg(args, int);
        len = n - 1 - (dest - s);
        switch (*++fmt)
        {
        case 'c':
            *dest++ = (char)arg;
            break;
        case 's':
            strncpy(dest, (char*)arg, len);
            dest = strchr(dest, 0);
            break;
        case 'd':
            __itoa(arg, buf);
            strncpy(dest, buf, len);
            dest = strchr(dest, 0);
            break;
        case 'x':
            itoab((unsigned)arg, buf, 16);
            strncpy(dest, buf, len);
            dest = strchr(dest, 0);
            break;
        case 'u':
            itoab((unsigned)arg, buf, 10);
            strncpy(dest, buf, len);
            dest = strchr(dest, 0);
            break;
        default:
            *dest++ = *fmt;
        }
        fmt++;
    }
    *dest = '\0';
    return dest - s;
}


int tiny_snprintf (char * s, size_t n, const char * format, ... )
{
   va_list      arg;
   int  rc;
   va_start(arg, format);

   rc = tiny_vsnprintf(s, n, format, arg);

   va_end(arg);
   return rc;
}

int tiny_sprintf (char * s, const char * format, ... )
{
   va_list      arg;
   int  rc;
   va_start(arg, format);

   rc = tiny_vsnprintf(s, 4096, format, arg);

   va_end(arg);
   return rc;
}



