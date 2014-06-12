/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dos.h>
#include <assert.h>

#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)

void getdate( struct date *dateblk)
{
 unsigned long tmp;
 __asm__ __volatile__("int $0x40":"=a"(tmp):"0"(29)); 
 dateblk->da_year=2000+(tmp&0xff);
 dateblk->da_mon=(tmp>>8)&0xff;
 dateblk->da_day=(tmp>>16)&0xff;
 BCD_TO_BIN(dateblk->da_year);
 BCD_TO_BIN(dateblk->da_mon);
 BCD_TO_BIN(dateblk->da_day);
}
