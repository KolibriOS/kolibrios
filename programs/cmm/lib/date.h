//IO library
#ifndef INCLUDE_DATE_H
#define INCLUDE_DATE_H

#ifndef INCLUDE_STRING_H
#include "../lib/strings.h"
#endif

:struct date
{
	byte day;
	byte month;
	word year;
};

:struct time
{
	byte seconds;
	byte minutes;
	byte hours;
	byte rez;
};

:void DrawDateTime(dword x, y, color, _date, _time)
{
	EDI = _date;
	EAX = 47;
	EBX = 2<<16;
	EDX = x<<16+y;
	ESI = 0x90<<24+color;
	ECX = EDI.date.day;
	$int 64
	EDX += 24<<16;
	ECX = EDI.date.month;
	$int 64
	EDX += 24<<16;
	EBX = 4<<16;
	ECX = EDI.date.year;
	$int 64

	EDI = _time;
	EDX += 40<<16;
	EBX = 2<<16;
	ECX = EDI.time.hours;
	$int 64
	EDX += 24<<16;
	ECX = EDI.time.minutes;
	$int 64
	EDX += 24<<16;
	ECX = EDI.time.seconds;
	$int 64
	
	WriteText(x,y,0x90,color, "  .  .       :  :");
}

#endif