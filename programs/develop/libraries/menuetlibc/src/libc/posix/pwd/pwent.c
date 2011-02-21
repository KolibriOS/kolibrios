/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <pwd.h>
#include <unistd.h>

static int count=0;

void
setpwent(void)
{
  count=0;
}

struct passwd *
getpwent(void)
{
  if (count == 0)
  {
    count++;
    return getpwuid(getuid());
  }
  return 0;
}

void
endpwent(void)
{
  count=0;
}
