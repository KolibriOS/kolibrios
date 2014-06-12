#include <dos.h>
#include <assert.h>

#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)

void gettime( struct time *tp)
{
 unsigned long tmp=__menuet__getsystemclock();
 tp->ti_hour=tmp&0xff;
 tp->ti_min=(tmp>>8)&0xff;
 tp->ti_sec=(tmp>>16)&0xff;
 tp->ti_hund=0;
 BCD_TO_BIN(tp->ti_hour);
 BCD_TO_BIN(tp->ti_min);
 BCD_TO_BIN(tp->ti_sec);
}
