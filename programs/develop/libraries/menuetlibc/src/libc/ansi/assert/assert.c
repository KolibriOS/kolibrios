#include<assert.h>
#include<menuet/os.h>
#include <stdlib.h>

void __dj_assert(const char *msg, const char *file, int line)
{
 __libclog_printf("Assertion failed at line %u in file %s\n",
   line,file);
 __libclog_printf("Assertion: '%s'\n",msg);
 exit(-1);
}
