/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <pc.h>
#include <menuet/os.h>

int getkey(void)
{
 if(!kbhit()) return -1;
 return __menuet__getkey();
}
