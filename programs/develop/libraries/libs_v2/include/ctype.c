
#include "ctype.h"

static inline int toupper(int c)
{

if ( (c >= 97) && (c <= 122) )
	return c-32 ;

if ( (c >= 160) && (c <= 175) )
	return c-32 ;

if ( (c >= 224) && (c <= 239) )
	return c-80 ;

if ( (c == 241) || (c == 243) || (c == 245) || (c == 247) )
	return c-1;

return c;
}

static inline int tolower(int c)
{

if ( (c >= 65) && (c <= 90) )
	return c+32 ;

if ( (c >= 128) && (c <= 143) )
	return c+32 ;

if ( (c >= 144) && (c <= 159) )
	return c+80 ;

if ( (c == 240) || (c == 242) || (c == 244) || (c == 246) )
	return c+1;

return c;
}

