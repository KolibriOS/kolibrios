/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>

static char slash[] = "/";
static char shell[] = "sh";

struct passwd *
getpwuid(uid_t uid)
{
  static struct passwd rv;
  rv.pw_uid = getuid();
  if (uid != rv.pw_uid)
    return 0;
  rv.pw_name = getlogin();
  rv.pw_gid = getgid();
  rv.pw_dir = getenv("HOME");
  if (rv.pw_dir == 0)
    rv.pw_dir = slash;
  rv.pw_shell = getenv("SHELL");
  if (rv.pw_shell == 0)
    rv.pw_shell = getenv("COMSPEC");
  if (rv.pw_shell == 0)
    rv.pw_shell = shell;
  return &rv;
}
