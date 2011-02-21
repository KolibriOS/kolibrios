#include<assert.h>
#include<menuet/os.h>

void __dj_unimp(const char *fn)
{
 __libclog_printf(fn);
 exit(-1); 
 for(;;);
}
