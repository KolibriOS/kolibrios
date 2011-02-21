/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <pc.h>
#include <libc/farptrgs.h>
#include <menuet/os.h>

int kbhit(void)
{
 int i;
 i=__menuet__check_for_event();
 if(i==2) return 1; else return 0;
}
