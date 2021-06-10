/* strcoll( const char *, const char * )

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

#include <string.h>

int strcoll( const char * s1, const char * s2 )
{
    /* FIXME: This should access _PDCLIB_lc_collate. */
    return strcmp( s1, s2 );
}
