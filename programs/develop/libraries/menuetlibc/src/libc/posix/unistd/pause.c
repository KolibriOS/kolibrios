/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>

int pause(void)
{
 __menuet__delay100(1);
 return 0;
}
