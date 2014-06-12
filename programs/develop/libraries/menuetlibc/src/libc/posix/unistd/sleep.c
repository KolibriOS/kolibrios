/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>
#include <time.h>
#include <menuet/os.h>

unsigned int sleep(unsigned int _seconds)
{
 for(;_seconds;_seconds--) __menuet__delay100(100);
 return _seconds;
}
