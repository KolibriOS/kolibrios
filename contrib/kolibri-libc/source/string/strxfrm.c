/* strxfrm( char *, const char *, size_t )

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

#include <string.h>

size_t strxfrm( char * s1, const char * s2, size_t n )
{
    size_t len = strlen( s2 );

    if ( len < n )
    {
        /* Cannot use strncpy() here as the filling of s1 with '\0' is not part
           of the spec.
        */
        /* FIXME: This should access _PDCLIB_lc_collate. */
        while ( n-- && ( *s1++ = ( unsigned char )*s2++ ) )
        {
            /* EMPTY */
        }
    }

    return len;
}