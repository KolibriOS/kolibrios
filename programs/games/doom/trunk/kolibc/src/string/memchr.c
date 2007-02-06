#include "string.h"

#if 0
void* memchr(const void* buf,int c,int count)
{
  int i;
  for (i=0;i<count;i++)
    if (*(char*)buf==c)
      return (void*)buf;
    else
      ((char*)buf)++;
  return (void*)0;
}


void *memset (void *dst, int val, size_t count)
{ void *start = dst;

  while (count--)
  {
    *(char *)dst = (char)val;
     dst = (char *)dst + 1;
  }
  return start;
}


int strcmp (const char * src, const char * dst)
{
  int ret = 0 ;

  while( ! (ret = *(unsigned char *)src - *(unsigned char *)dst) && *dst)
             ++src, ++dst;

  if ( ret < 0 )
     ret = -1 ;
  else if ( ret > 0 )
     ret = 1 ;
  return( ret );
}

char *strcat (char * dst, const char * src)
{
  char * cp = dst;

  while( *cp )
    cp++;                 

  while( *cp++ = *src++ ) ;   

  return dst ;                
}

int abs (int number )
{
  return( number>=0 ? number : -number );
}

#endif