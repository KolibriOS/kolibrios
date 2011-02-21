#include<menuet/os.h>

int close(int handle)
{
 return dosemu_close(handle);
}
