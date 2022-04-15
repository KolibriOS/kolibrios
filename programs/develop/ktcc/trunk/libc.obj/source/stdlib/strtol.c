#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

int getdigit(char ch, int base)
{
    if (isdigit(ch))
        ch -= '0';
    else if (isalpha(ch) && ch <= 'Z')
        ch = 10 + ch - 'A';
    else if (isalpha(ch))
        ch = 10 + ch - 'a';
    else
        return -1;

    if (ch / base != 0)
        return -1;

    return ch;
}

long int strtol(const char* str, char** endptr, int base)
{
    long int res = 0;
    int sign = 1;

    if (base > 36) {
        errno = EINVAL;
        goto bye;
    }

    while (isspace(*str))
        str++;

    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+')
        str++;

    if (base == 0 || base == 16) {
        if (*str == '0' && (str[1] == 'x' || str[1] == 'X')) {
            base = 16;
            str += 2;
        }
    }

    if (base == 0 && *str == '0')
        base = 8;

    if (base == 0)
        base = 10;

    int digit;
    while ((digit = getdigit(*str, base)) >= 0) {
        res = base * res + digit;
        str++;
        if (res < 0) {
            errno = ERANGE;
            if (sign > 0)
                res = LONG_MAX;
            else
                res = LONG_MIN;
        }
    }

bye:
    if (endptr)
        *endptr = (char*)str;

    return res * sign;
}
