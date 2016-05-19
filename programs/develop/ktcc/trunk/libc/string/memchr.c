#include <string.h>

void* memchr(const void* buf,int c,size_t count)
{
  int i;
  for (i=0;i<count;i++)
    if (*(char*)buf==(char)c)
      return (void*)buf;
    else
      buf++;
  return (void*)0;
}
