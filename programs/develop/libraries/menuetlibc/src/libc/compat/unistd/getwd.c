/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>
#include <limits.h>

extern char* __get_curdir(void);
char * getwd(char *buffer)
{
  if (buffer == 0)
    return 0;
 char * p=__get_curdir();
 sprintf(buffer,"%s",p);
 return buffer;
}
