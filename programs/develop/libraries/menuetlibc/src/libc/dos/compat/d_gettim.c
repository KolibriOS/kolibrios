#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)

#include <dos.h>

void _dos_gettime(struct _dostime_t *time)
{
 unsigned long tmp=__menuet__getsystemclock();
 time->hour=tmp&0xff;
 time->minute=(tmp>>8)&0xff;
 time->second=(tmp>>16)&0xff;
 time->hsecond=0;
 BCD_TO_BIN(time->hour); 
 BCD_TO_BIN(time->minute); 
 BCD_TO_BIN(time->second); 
}
