/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <signal.h>
#include <unistd.h>

int kill(pid_t pid, int sig)
{
 if(pid==-1) __menuet__sys_exit();
}
