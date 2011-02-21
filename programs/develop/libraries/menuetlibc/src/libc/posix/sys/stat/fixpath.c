#include <libc/stubs.h>
#include <stdio.h>		/* For FILENAME_MAX */
#include <errno.h>		/* For errno */
#include <string.h>		/* For strlen() */
#include <sys/stat.h>
#include <libc/dosio.h>

static inline int is_slash(char c)
{
 return c=='/' || c=='\\';
}

void fix_slashes(char * in,char * out)
{ 
 int slash_count;
 for(slash_count=1;in && out && *in;in++)
 {
  if(is_slash(*in))
  {
   slash_count++;
   continue;
  } else {
   if(slash_count)
   {
    slash_count=0;
    *out++='/';
   }
   *out++=*in;
  }
 }
 *out='\0';
}

char * __libc_combine_path(char * c);

void _fixpath(const char *in, char *out)
{
 char * combined;
 combined=__libc_combine_path((char *)in);
 fix_slashes(combined,out);
}
