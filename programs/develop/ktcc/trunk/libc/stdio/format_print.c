/*
        function for format output to the string

Siemargl update formats as http://www.cplusplus.com/reference/cstdio/printf/, no wchar though
http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap05.html  is used too
%g explain https://support.microsoft.com/en-us/kb/43392

todo:
-fix precision in %g
-%u printed as signed, %x, %o also is promoted to long long
// FAIL 0x0FFFF7A7E as %x - signed long promotes sign, need %llx or %Lx and type conversion
-%a
-%n nothing printed
-%17.18f digits maximum format
-use %C as w_char L'x' (non standard extension)
-radix point always '.', no LOCALEs
*/


#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>

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
int formatted_double_to_string(long double number, int format1, int format2, char *s, int flags);
int formatted_double_to_string_scientific(long double number, int format1, int format2, char *s, int flags);
int formatted_long_to_string(long long number, int format1, int prec, char *s, int flags);
int formatted_hex_to_string(unsigned long long number, int fmt1, int prec, char *s, int flags);
int formatted_octa_to_string(unsigned long long number, int fmt1, int prec, char *s, int flags);


int formatted_double_special(long double number, char *s)
// return 0 if no special values: NAN, INF. -0.0 ignored
// http://steve.hollasch.net/cgindex/coding/ieeefloat.html
{
    struct IEEExp {
        unsigned manl:32;
        unsigned manh:32;
        unsigned exp:15;
        unsigned sign:1;
    } *ip = (struct IEEExp *)&number;

    if (ip->exp != 0x7fff) return 0;

    if (ip->manh == 0x80000000 && ip->manl == 0) // Inf
    {
        if(ip->sign)
            strcpy(s, "-INF");
        else
            strcpy(s, "+INF");
    } else
    if (ip->manh & ~0x7fffffff)
        strcpy(s, "QNaN");
    else
        strcpy(s, "SNaN");

    return 4;
}

int copy_and_align(char *dest, int width, char *src, int src_len, char sign, int flags)
// alingn number in buffer, put sign and fills additional places
// flags used only flag_left_just and flag_lead_zeros
// sign can be zero, 0, x, X, space, +, -
{
    int     rc = 0, sign_len;
    char    fill;

    fill = (flags & flag_lead_zeros)&&((flags & flag_left_just)==0) ? '0' : ' ';
    if(sign == 'x' || sign == 'X')
    {
        sign_len = 2;
    } else
    if (sign)
        sign_len = 1;
    else
        sign_len = 0;

    if ((flags & flag_left_just) || (src_len + sign_len >= width))   // left justify or no room
    {
        if (sign)
        {
            if(sign == 'x' || sign == 'X')
            {
                dest[0] = '0';
                dest[1] = sign;
                memcpy(dest + 2, src, src_len);
                rc = src_len + 2;
            } else
            { // single sign
                dest[0] = sign;
                memcpy(dest + 1, src, src_len);
                rc = src_len + 1;
            }
        } else
        {
            memcpy(dest, src, src_len);
            rc = src_len;
        }
        if (rc < width)
        {
            memset(dest + rc, fill, width - rc);
            rc = width;
        }
    } else // right justify and fill
    {
        rc = width;
        memcpy(dest + width - src_len, src, src_len);
        memset(dest, fill, width - src_len);
        if (flags & flag_lead_zeros)
        {
            if(sign == 'x' || sign == 'X')
            {
                dest[0] = '0';
                dest[1] = sign;
            } else
            if (sign) dest[0] = sign;
        } else
        {
            if(sign == 'x' || sign == 'X')
            {
                dest[width - src_len - 2] = '0';
                dest[width - src_len - 1] = sign;
            } else
            if (sign) dest[width - src_len - 1] = sign;
        }
    }
    return rc;
}

int formatted_double_to_string_scientific(long double number, int format1, int format2, char *s, int flags)
{
    long double     norm_digit;
    long            mul = 0;
    char            sign = 0;
    char            buf[50];
    int     len;

    if((flags & flag_point) == 0) format2 = 6;  // default prec if no point spec

    len = formatted_double_special(number, buf);
    if (len == 0)
    {
        if (number < 0) { sign = '-'; norm_digit = -number; }
        else
        {
            norm_digit = number;
            if (flags & flag_plus) sign = '+';  else
            if (flags & flag_space_plus) sign = ' ';
        }
        // normalize
        while (norm_digit < 1.0 && norm_digit > 0) { norm_digit *= 10; mul--; }
        while (norm_digit >= 10.0) { norm_digit /= 10; mul++; }

        len = formatted_double_to_string(norm_digit, 0, format2, buf, flags & ~(flag_plus | flag_space_plus));

        if (flags & flag_register)
            buf[len++] = 'E';
        else
            buf[len++] = 'e';

        len += formatted_long_to_string(mul, 0, 3, buf + len, flag_plus | flag_lead_zeros);
    }
    else
        flags &= ~flag_lead_zeros; // no need for INF, NAN

    len = copy_and_align(s, format1, buf, len, sign, flags);

    return len;
}

int formatted_double_to_string(long double number, int format1, int format2, char *s, int flags)
{
    long double	nafter, beforpointdigit;
    long long	intdigit, mul;
    int	    div;
    int     i;
    char    sign = 0;
    int     fmt1;
    int     fmt2;
    char    buf[100], *pbuf = buf;
    char    buf_low[50], *pbuf_lo = buf_low;

    if((flags & flag_point) == 0) format2 = 6;  // default prec if no point spec

    i = formatted_double_special(number, buf);
    if (i == 0)
    {
        if (number < 0) {sign = '-'; number = -number; }
        else
        {
            if (flags & flag_plus) sign = '+';  else
            if (flags & flag_space_plus) sign = ' ';
        }

        fmt1 = 1;
        fmt2 = format2;
        if (fmt2 > 18) fmt2 = 18; //maximum size of long long type

        beforpointdigit = floor(number + 0.00000000000001);
        nafter = number - beforpointdigit;

        //print part of number before point
        mul = 1;
        for(i = 0; i < sizeof buf - 1; i++)
        {
            mul *= 10;
            if ((beforpointdigit/mul) < 1.0) { fmt1 = i + 1; break; }
        }
        if (i == sizeof buf - 1 || fmt1 > 17)
        {
            strcpy(s, "[too big number for %f, %a]");
            return strlen(s);
        }

        mul /= 10;
        while(mul > 1)
        {
            div = beforpointdigit / mul;
            *pbuf++ = (char)div + '0';
            beforpointdigit = beforpointdigit - div * mul;
            mul /= 10;
        }
        *pbuf++=(char)beforpointdigit + '0';

        //print part of number after point
        mul = 1;
        for(i = 0; i < fmt2; i++)
        {
            nafter = nafter*10;
            mul *= 10;
        }

        intdigit = roundl(nafter);

        mul /= 10;
        for(i = 0; i < fmt2 - 1; i++)
        {
                div = intdigit / mul;
                *pbuf_lo++=(char)div + '0';
                intdigit = intdigit - div * mul;
                mul /= 10;
                if (mul == 1) break;
        }
        *pbuf_lo++ = (char)intdigit + '0';

        // form full number
        if (roundl(nafter) != 0 || fmt2 != 0)
        {
            *pbuf++ = '.';
            memcpy(pbuf, buf_low, pbuf_lo - buf_low);  pbuf += pbuf_lo - buf_low;
        } else if (flags & flag_hash_sign)
            *pbuf++ = '.';
    }
    else
    {
        flags &= ~flag_lead_zeros; // no need for INF, NAN
        pbuf += i;
    }

    return copy_and_align(s, format1, buf, pbuf - buf, sign, flags);
}

int formatted_long_to_string(long long number, int format1, int prec, char *s, int flags)
{
        int         i;
        int         fmt;
        char        sign = 0;
        long long   digit;
        long long   mul;
        int         div;
        char        buf[100], *pbuf = buf;

        if (number == -9223372036854775807LL - 1)  // overflow all our math, cant minus this
        {
            strcpy(s, "-9223372036854775808");
            return strlen(s);
        }

        if (flags & flag_point) flags &= ~flag_lead_zeros;  // conflicting flags

        if (number < 0) {sign = '-'; number = -number; }
        else
        {
            if (flags & flag_plus) sign = '+';  else
            if (flags & flag_space_plus) sign = ' ';
        }

        digit = number;

        mul = (digit < 0) ? -1 : 1;

        for(i = 0; i < sizeof buf - 2; i++)
        {
            if (digit / mul < 10) { fmt = i + 1; break; }
            mul *= 10;
        }

        // add leading zeros by prec
        for(i = 0; i < prec - fmt; i++) *pbuf++ = '0';

        for(i = 0; i < fmt - 1; i++)
        {
            div = digit / mul;
            *pbuf++ = (char)div + '0';
            digit = digit - div * mul;
            mul /= 10;
            if (mul == 1 || mul == -1) break;
        }
        *pbuf++ = (char)digit + '0';

        return copy_and_align(s, format1, buf, pbuf - buf, sign, flags);
}

int formatted_hex_to_string(unsigned long long number, int fmt1, int prec, char *s, int flags)
{
    unsigned long long digit, mul;
    int             i, div, fmt;
    char            xdigs_lower[16]="0123456789abcdef";
    char            xdigs_upper[16]="0123456789ABCDEF";
    char            buf[50], *pbuf = buf, sign;

    if (number == -9223372036854775807LL - 1)  // overflow all our math, cant minus this
    {
        strcpy(buf, "FFFFFFFFFFFFFFFF");
        pbuf += strlen(buf);
    }
    else
    {
        if (flags & flag_point) flags &= ~flag_lead_zeros;  // conflicting flags

        digit = number;

        mul = (digit < 0) ? -1 : 1;

        for(i = 0; i < sizeof buf - 2; i++)
        {
            if (digit / mul < 16) { fmt = i + 1; break; }
            mul <<= 4;
        }

        // add leading zeros by prec
        for(i = 0; i < prec - fmt; i++) *pbuf++ = '0';

        for(i = 0; i < fmt - 1; i++)
        {
            div = digit / mul;
            *pbuf++ = (flags & flag_register) ? xdigs_upper[div] : xdigs_lower[div];
            digit = digit - div * mul;
            mul >>= 4;
            if (mul == 1 || mul == -1) break;
        }
        *pbuf++ = (flags & flag_register) ? xdigs_upper[digit] : xdigs_lower[digit];
    }

    sign = 0;
    if(flags & flag_hash_sign)
        sign = (flags & flag_register) ? 'X' : 'x';

    return copy_and_align(s, fmt1, buf, pbuf - buf, sign, flags);
}

int formatted_octa_to_string(unsigned long long number, int fmt1, int prec, char *s, int flags)
{
    unsigned long long digit, mul;
    int             i, div, fmt;
    char            xdigs_lower[16]="01234567";
    char            buf[50], *pbuf = buf;

    if (number == -9223372036854775807LL - 1)  // overflow all our math, cant minus this
    {
        strcpy(buf, "1777777777777777777777");
        pbuf += strlen(buf);
    }
    else
    {
        if (flags & flag_point) flags &= ~flag_lead_zeros;  // conflicting flags

        digit = number;

        mul = (digit < 0) ? -1 : 1;

        for(i = 0; i < sizeof buf - 2; i++)
        {
            if (digit / mul < 8) { fmt = i + 1; break; }
            mul <<= 3;
        }

        // add leading zeros by prec
        for(i = 0; i < prec - fmt; i++) *pbuf++ = '0';

        for(i = 0; i < fmt - 1; i++)
        {
            div = digit / mul;
            *pbuf++ = xdigs_lower[div & 0x7];
            digit = digit - div * mul;
            mul >>= 3;
            if (mul == 1 || mul == -1) break;
        }
        *pbuf++ = xdigs_lower[digit];
    }

    return copy_and_align(s, fmt1, buf, pbuf - buf, (flags & flag_hash_sign) ? '0' : 0, flags);
}

//int vsnprintf (char * s, size_t n, const char * format, va_list arg );
int format_print(char *dest, size_t maxlen, const char *fmt0, va_list argp)
{
    int                     i;
    int                     length;
    int                     fmt1, fmt2;  // width, precision
    size_t                  pos, posc;
    long	long	        intdigit;
    long	double          doubledigit;
    const   char            *fmt, *fmtc;  // first point to %, fmtc points to specifier
    char                    *s; // pointer to current dest char
    char                    *str;
    char                    buf[200];  // buffer for current argument value print representation
    int                     format_flag;
    int                     flag_long;        // 2 = long double or long long int or wchar
    int                    *point_to_n = NULL;
    int                     flags;  // parsed flags

    fmt = fmt0;
    s = dest;
    pos = 0;
    while(pos < maxlen)
    {
        if (*fmt != '%')  // usual char
        {
            if ('\0' == (*s++ = *fmt++)) break;
            pos++;
            continue;
        }

        if (*(fmt + 1) == '%') // %%
        {
                *s++ = '%'; pos++;
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
                case 'A':
                    format_flag = 1;
                    flags |= flag_unsigned | flag_register;
                    break;
                case 'c':   case 'd':   case 'i':   case 'e':   case 'f':   case 'g':   case 's':   case 'n':
                    format_flag = 1;
                    break;
                case 'E':   case 'F':   case 'G':
                    format_flag = 1;
                    flags |= flag_register;
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
                case 'X':   case 'P':
                    format_flag = 1;
                    flags |= flag_unsigned | flag_register;
                    break;
                case '+':
                    flags |= flag_plus;
                    break;
                case '-':
                    flags |= flag_left_just;
                    break;
                case ' ': // space
                    flags |= flag_space_plus;
                    break;
                case '#':
                    flags |= flag_hash_sign;
                    break;
                case '*':   case '.':  // just skip
                    break;
                default:
                    if(isdigit(*fmtc))  break;
                    strncpy(dest, "print format error - in % invalid char found", maxlen);
                    return -1;  // non format char found - user error
                }
        }

        if (format_flag == 0)
        {
            strncpy(dest, "print format error - % without format specifier", maxlen);
            return -1;  // format char not found - user error
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
                case '*':
                    if (flag_point == 0)
                        fmt1 = va_arg(argp, int);
                    else
                        fmt2 = va_arg(argp, int);
                    break;
                case '.':
                    flags |= flag_point;
                    break;
                case 'l':   case 'L':   case '+':   // valid chars - skip
                case '-':   case ' ':   case '#':
                    break;
                default: // must be error
                    strncpy(dest, "print format error - %width.precision", maxlen);
                    return -1;  // format char not found - user error
                }
                fmt++;
            }
        }

        // do real work - format arguments values
        length = 0;
        switch(*fmtc)
        {
        case 'n':
            point_to_n = va_arg(argp, int*);
            break;
        case 'c':
            if (pos + 1 <= maxlen)
            {
                buf[0] = (char)va_arg(argp, int);
                length = 1;
            }
            break;
        case 's':   // special case - without buf
            str = va_arg(argp, char*);
            length = strlen(str);
            if ((flags & flag_point) && (length > fmt2)) length = fmt2;  // cut by precision
            if (pos + length > maxlen) length = maxlen - pos;
            memcpy(s, str ,length);
            s += length;
            pos += length;
            break;
        case 'd':   case 'i':   case 'u':   case 'U':
            if (flag_long == 0) intdigit = va_arg(argp, int); else
            if (flag_long == 1) intdigit = va_arg(argp, long); else
            if (flag_long == 2) intdigit = va_arg(argp, long long);
            length = formatted_long_to_string(intdigit, fmt1, fmt2, buf, flags);
            break;
        case 'o':
            if (flag_long == 0) intdigit = va_arg(argp, int); else
            if (flag_long == 1) intdigit = va_arg(argp, long); else
            if (flag_long == 2) intdigit = va_arg(argp, long long);
            length = formatted_octa_to_string(intdigit, fmt1, fmt2, buf, flags);
            break;
        case 'p':   case 'P':   case 'x':   case 'X':
            if (flag_long == 0) intdigit = va_arg(argp, int); else
            if (flag_long == 1) intdigit = va_arg(argp, long); else
            if (flag_long == 2) intdigit = va_arg(argp, long long);
            length=formatted_hex_to_string(intdigit, fmt1, fmt2, buf, flags);
            break;
        case 'a':   case 'A':   case 'f':   case 'F':
            if (flag_long <= 1) doubledigit = va_arg(argp, double); else
            if (flag_long == 2) doubledigit = va_arg(argp, long double);
            length = formatted_double_to_string(doubledigit, fmt1, fmt2, buf, flags);
            break;
        case 'e':   case 'E':
            if (flag_long <= 1) doubledigit = va_arg(argp, double); else
            if (flag_long == 2) doubledigit = va_arg(argp, long double);
            length = formatted_double_to_string_scientific(doubledigit, fmt1, fmt2, buf, flags);
            break;
        case 'g':   case 'G':
 //prec special case, this is just workaround
            if (flag_long <= 1) doubledigit = va_arg(argp, double); else
            if (flag_long == 2) doubledigit = va_arg(argp, long double);
            if (flags & flag_point)
                length = formatted_double_to_string(doubledigit, fmt1, fmt2, buf, flags);
            else
                length = formatted_double_to_string(doubledigit, fmt1, 1, buf, flags | flag_point);
            i = formatted_double_to_string_scientific(doubledigit, fmt1, fmt2, buf + sizeof buf / 2, flags);
            if(length > i)
            {
                memcpy(buf, buf + sizeof buf / 2, i);
                length = i;
            }
            break;
        }
        if (*fmtc != 's' && length > 0) // skip multiple string copying
        {
            if (pos + length > maxlen)  length = maxlen - pos;
            memcpy(s, buf, length);
            s += length;
            pos += length;
        }
        fmt = fmtc + 1;
    }

    if (point_to_n) *point_to_n = pos;
    return(pos);
}
