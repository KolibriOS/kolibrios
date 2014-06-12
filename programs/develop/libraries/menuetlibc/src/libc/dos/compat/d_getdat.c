#include <errno.h>
#include <dos.h>

#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)

void _dos_getdate(struct _dosdate_t *date)
{
 unsigned long tmp;
 __asm__ __volatile__("int $0x40":"=a"(tmp):"0"(29)); 
 date->year=2000+(tmp&0xff);
 date->month=(tmp>>8)&0xff;
 date->day= (tmp>>16)&0xff;
 date->dayofweek=0; /* xxx - how to do it correctly ? */
 BCD_TO_BIN(date->year);
 BCD_TO_BIN(date->month);
 BCD_TO_BIN(date->day);
}
