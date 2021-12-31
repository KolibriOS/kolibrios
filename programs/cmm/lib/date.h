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

:void DrawDate(dword x, y, color, in_date)
{
	EDI = in_date;
	EAX = 47;
	EBX = 2<<16;
	EDX = x<<16+y;
	ESI = 0x90<<24+color;
	ECX = EDI.date.day;
	$int 64
	EDX += 20<<16;
	ECX = EDI.date.month;
	$int 64
	EDX += 20<<16;
	EBX = 4<<16;
	ECX = EDI.date.year;
	$int 64
	DrawBar(x+17,y+10,2,2,color);
	$add ebx, 20 << 16
	$int 64
}

#endif