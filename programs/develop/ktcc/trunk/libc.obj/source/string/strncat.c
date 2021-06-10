/* strncat( char *, const char *, size_t )

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

#include <string.h>

char * strncat( char * s1, const char * s2, size_t n )
{
    char * rc = s1;

    while ( *s1 )
    {
        ++s1;
    }

    while ( n && ( *s1++ = *s2++ ) )
    {
        --n;
    }

    if ( n == 0 )
    {
        *s1 = '\0';
    }

    return rc;
}