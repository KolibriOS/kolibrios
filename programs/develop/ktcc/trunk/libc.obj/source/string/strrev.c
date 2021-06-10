#include <string.h>
 
char* strrev(char *p)
{
  char *q = p, *res = p, z;
  while(q && *q) ++q; /* find eos */
  for(--q; p < q; ++p, --q)
  {
        z = *p;
        *p = *q;
        *q = z;
  }
  return res;
}
 