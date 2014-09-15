/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <crt0.h>
#include <string.h>
#include <libc/internal.h>
#include <stdlib.h>
#include <libc/farptrgs.h>
#include <pc.h>
#include <libc/bss.h>
#include <menuet/os.h>

/* Global variables */

#define ds _my_ds()

int __bss_count = 1;

char **environ;
int _crt0_startup_flags;	/* default to zero unless app overrides them */
int __crt0_argc=0;
char ** __crt0_argv=NULL;

char * __dos_argv0;

extern __u32 __menuet__getmemsize(void);

extern void __main(void);
extern int  main(int, char **);
extern void _crt0_init_mcount(void);	/* For profiling */
void __crt0_setup_arguments(void);
extern char __menuet__app_param_area[];

//void dosemu_atexit(void);

void __crt1_startup(void)
{
 init_brk();
 __crt0_setup_arguments();
 dosemu_inithandles();
 init_dir_stack();
// atexit(dosemu_atexit);
 __main();
 {
  int stat=main(__crt0_argc,__crt0_argv);
  exit(stat);
 }
}
