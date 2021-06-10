/* memchr( const void *, int, size_t )

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

#include <string.h>

void * memchr( const void * s, int c, size_t n )
{
    const unsigned char * p = ( const unsigned char * ) s;

    while ( n-- )
    {
        if ( *p == ( unsigned char ) c )
        {
            return ( void * ) p;
        }

        ++p;
    }

    return NULL;
}