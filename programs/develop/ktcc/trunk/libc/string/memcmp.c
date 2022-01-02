#include <string.h>

typedef unsigned char uc;
int memcmp(const void* buf1,const void* buf2,size_t count)
{
	int i;
	for (i=0;i<count;i++)
	{
		if (*(uc*)buf1<*(uc*)buf2)
			return -1;
		if (*(uc*)buf1>*(uc*)buf2)
			return 1;
		(uc*)buf1++;
		(uc*)buf2++;
	}
	return 0;
}
