/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>
#include <libc/farptrgs.h>
#include <libc/bss.h>
#include <menuet/os.h>

static int pid_count = -1;
static pid_t my_pid;

static struct process_table_entry __tmp_proctab;

pid_t getpid(void)
{
 __menuet__get_process_table(&__tmp_proctab,PID_WHOAMI);
 return __tmp_proctab.pid;
}
