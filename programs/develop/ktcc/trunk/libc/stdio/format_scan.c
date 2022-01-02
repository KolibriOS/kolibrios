/*
        function for format read from any source

Siemargl formats as http://www.cplusplus.com/reference/cstdio/scanf/, no wchar though
http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap05.html  is used too

todo:
[characters], [^characters]
-%n nothing scanned, filled only if good result
-%d, i, u, o, x, p read similar - detect base by prefix 0 or 0x
-%a
-can overflow unsigned as signed
-radix point always '.', no LOCALEs
*/


#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
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

int     try_parse_real(long double *real, int ch, const void *src, void *save, virtual_getc vgetc, virtual_ungetc vungetc)
// returns 1 if OK, -1 == EOF, -2 parse broken
{
    int sign = 1, have_digits = 0;
    long long div;

    if (ch == '+')
    {
        ch = vgetc(save, src);
        if (ch == EOF) return EOF;
    } else
    if (ch == '-')
    {
        sign = -1;
        ch = vgetc(save, src);
        if (ch == EOF) return EOF;
    };
    *real = 0.0;
    for (;;) // mantissa before point
    {
        // test ch is valid
        if (isdigit(ch))
        {
            *real = *real * 10 + ch - '0';
            have_digits++;
            ch = vgetc(save, src);
            if (ch == EOF || isspace(ch)) break; // ok, just finish num
        } else
        if (ch == '.' || ch == 'E' || ch == 'e')
        {
            break; // ok
        }
        else
        {
            vungetc(save, ch, src);
            break;
        }
    }
    if (ch != '.' && ch != 'E' && ch != 'e') // ok, just integer part
    {
        *real *= sign;
        if (have_digits)
            return 1;
        else
            return -2;
    }

    if(ch == '.')
    {
        ch = vgetc(save, src);
        div = 10; // use as divisor
        for (;;) // mantissa after point
        {
            // test ch is valid
            if (isdigit(ch))
            {
                *real += (double)(ch - '0') / div;
                div *= 10;
                have_digits++;
                ch = vgetc(save, src);
                if (ch == EOF || isspace(ch)) break; // ok, just finish num
            } else
            if (ch == 'E' || ch == 'e')
            {
                break; // ok
            }
            else
            {
                vungetc(save, ch, src);
                break;
            }
        }
        if (ch != 'E' && ch != 'e')  // ok, real as XX.YY
        {
            *real *= sign;
            if (have_digits)
                return 1;
            else
                return -2;
        }
    }

    ch = vgetc(save, src);
    *real *= sign;
    // exponent
    sign = 1;
    if (ch == '+')
    {
        ch = vgetc(save, src);
        if (ch == EOF) return EOF;
    } else
    if (ch == '-')
    {
        sign = -1;
        ch = vgetc(save, src);
        if (ch == EOF) return EOF;
    };
    div = 0;
    for (;;)
    {
        // test ch is valid
        if (isdigit(ch))
        {
            div = div * 10 + ch - '0';
            ch = vgetc(save, src);
            if (ch == EOF || isspace(ch)) break; // ok, just finish num
        }
        else
        {
            vungetc(save, ch, src);
            break;
        }
    }
    div *= sign;
    *real *= pow(10, div);

    return 1;
}

int     try_parse_int(long long *digit, int ch, const void *src, void *save, virtual_getc vgetc, virtual_ungetc vungetc)
{
    int sign = 1, base = 10, have_digits = 0;

    if (ch == '+')
    {
        ch = vgetc(save, src);
        if (ch == EOF) return EOF;
    } else
    if (ch == '-')
    {
        sign = -1;
        ch = vgetc(save, src);
        if (ch == EOF) return EOF;
    };

    if (ch == '0')  // octal or hex, read next
    {
        base = 8;
        ch = vgetc(save, src);
        if (ch == EOF || isspace(ch))
            have_digits++;
        else
        if (ch == 'x' || ch == 'X')
        {
            base = 16;
            ch = vgetc(save, src);
            if (ch == EOF) return EOF;
        }
    }
    *digit = 0;
    for (;;)
    {
        // test ch is valid
        if ((isdigit(ch) && base == 10) ||
            (isdigit(ch) && base == 8 && ch < '8') ||
            (isxdigit(ch) && base == 16))
        {
            if (base == 16)
            {
                if (ch <= '9') ch-= '0';
                else
                if (ch <= 'F') ch = 10 + ch - 'A';
                else
                    ch = 10 + ch - 'a';
            }
            else
                ch -= '0';
            *digit = *digit * base + ch;
            have_digits++;
            ch = vgetc(save, src);
            if (ch == EOF || isspace(ch)) break; // ok, just finish num
        }
        else if (ch == EOF || isspace(ch))
            break;
        else
        {
            vungetc(save, ch, src);
            break;
        }
    }
    *digit *= sign;
    if (have_digits)
        return 1;
    else
        return -2;
}



int format_scan(const void *src, const char *fmt, va_list argp, virtual_getc vgetc, virtual_ungetc vungetc)
{
    int                     i;
    int                     length;
    int                     fmt1, fmt2;  // width, precision
    size_t                  pos, posc;
    const char            *fmtc;  // first point to %, fmtc points to specifier
    int                    ch;
    int                     format_flag;
    int                     flag_long;        // 2 = long double or long long int or wchar
    int                    *point_to_n = NULL, nread = 0;
    int                     flags;  // parsed flags
    int                     save = 0;
    char                *arg_str;
    int	        *arg_int;
    long        *arg_long;
    long long	*arg_longlong;
    float       *arg_float;
    double      *arg_double;
    long double *arg_longdouble;
    long long  digit;
    long double real;


    pos = 0;
    while(*fmt)
    {
        while (*fmt && isspace(*fmt)) fmt++; // skip paces in format str

        if (*fmt != '%')  // usual char
        {
            ch = vgetc(&save, src);
            if (ch != *fmt++) // char not match format
            {
                vungetc(&save, ch, src);
                break;
            }
            pos++;
            continue;
        }

        if (*(fmt + 1) == '%') // %%
        {
            ch = vgetc(&save, src);
            if (ch != '%') // char not match format
            {
                vungetc(&save, ch, src);
                break;
            }
            pos++;
            fmt += 2;
            continue;
        }
        //checking to containg format in the string
        fmtc = fmt;
        posc = pos;

        flags = 0;
        format_flag = 0;
        flag_long = 0;  // 2 = long double or long long int or wchar

        while(*fmtc != '\0' && !format_flag)    // searching end of format
        {
                fmtc++; posc++;
                switch( *fmtc )
                {
                case 'a':
                    format_flag = 1;
                    flags |= flag_unsigned;
                    break;
                case 'c':   case 'd':   case 'i':   case 'e':   case 'f':   case 'g':   case 's':   case 'n':
                    format_flag = 1;
                    break;
                case 'l':
                    flag_long  = flag_long ? 2 : 1;  // ll.eq.L
                    break;
                case 'L':
                    flag_long = 2;
                    break;
                case 'o':   case 'u':   case 'x':   case 'p':
                    format_flag = 1;
                    flags |= flag_unsigned;
                    break;
                case '*':   case '.':  // just skip
                    break;
                default:
                    if(isdigit(*fmtc))  break;
                    goto exit_me;  // non format char found - user error
                }
        }

        if (format_flag == 0)
        {
            goto exit_me;  // format char not found - user error
        }

        fmt1 = 0;
        fmt2 = 0;
        if (posc - pos > 1)  // try to read width, precision
        {
            fmt++;
            for(i = pos + 1; i < posc; i++)
            {
                switch(*fmt)
                {
                case '0':
                    if(fmt1 == 0 && (flags & flag_point) == 0)    flags |= flag_lead_zeros;
                case '1':   case '2':   case '3':   case '4':
                case '5':   case '6':   case '7':   case '8':   case '9':
                    if ((flags & flag_point) == 0)
                        fmt1 = fmt1 * 10 + (*fmt -'0');
                    else
                        fmt2 = fmt2 * 10 + (*fmt -'0');
                    break;
                case '*': // ignoring
                    break;
                case '.':
                    flags |= flag_point;
                    break;
                case 'l':   case 'L':      // valid chars - skip
                    break;
                default: // must be error
                    goto exit_me;  // format char not found - user error
                }
                fmt++;
            }
        }

        // do real work - format arguments values
        // skip input spaces
        do {
            ch = vgetc(&save, src);
            if (ch == EOF) goto exit_me;
        } while (isspace(ch));

        switch(*fmtc)
        {
        case 'n':
            point_to_n = va_arg(argp, int*);
            vungetc(&save, ch, src);
            break;
        case 'c':  // read width chars, ever spaces
            arg_str = va_arg(argp, char*);
            if (fmt1 == 0) length = 1;
            else length = fmt1;
            for (i = 0; i < length;)
            {
                *arg_str++ = ch; i++;
                ch = vgetc(&save, src);
                if (ch == EOF) break;
            }
            if (i < length) goto exit_me; // not enough chars
            break;
        case 's':
            arg_str = va_arg(argp, char*);
            if (fmt1 == 0) length = 4095;   // max string scan 4096
            else length = fmt1;
            for (i = 0; i < length; i++)
            {
                *arg_str++ = ch;
                ch = vgetc(&save, src);
                if (ch == EOF || isspace(ch)) break; // ok, just finish string
            }
            *arg_str++ = '\0';
            break;
        case 'd':   case 'i':   case 'u':
        case 'o':   case 'p':   case 'x':
            i = try_parse_int(&digit, ch, src, &save, vgetc, vungetc);
            if (i < 0) goto exit_me;

            if (flag_long == 0) { arg_int = va_arg(argp, int*); *arg_int = (int)digit; } else
            if (flag_long == 1) { arg_long = va_arg(argp, long*); *arg_long = (long)digit; } else
            if (flag_long == 2) { arg_longlong = va_arg(argp, long long*); *arg_longlong = digit; }
            break;
        case 'a':   case 'A':   case 'f':   case 'F':
        case 'e':   case 'E':
        case 'g':   case 'G':
            i = try_parse_real(&real, ch, src, &save, vgetc, vungetc);
            if (i < 0) goto exit_me;

            if (flag_long == 0) { arg_float = va_arg(argp, float*); *arg_float = (float)real; } else
            if (flag_long == 1) { arg_double = va_arg(argp, double*); *arg_double = (double)real; } else
            if (flag_long == 2) { arg_longdouble = va_arg(argp, long double*); *arg_longdouble = real; }
            break;
        }

        fmt = fmtc + 1;
        nread++;
    }
exit_me:
    if (point_to_n) *point_to_n = nread;
    return  nread;
}
