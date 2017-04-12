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

#endif