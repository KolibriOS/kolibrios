
#include <ksys.h>

#define SHELL_OK		0
#define SHELL_EXIT		1
#define SHELL_PUTC		2
#define SHELL_PUTS		3
#define SHELL_GETC		4
#define SHELL_GETS		5
#define SHELL_CLS		6

#define SHELL_SHM_MAX  1024*16

extern char __shell_shm_name[32]; 
extern char *__shell_shm;
extern int __shell_is_init;
extern int __shell_init();

#define SHELL_WAIT() while (*__shell_shm) _ksys_delay(5)



