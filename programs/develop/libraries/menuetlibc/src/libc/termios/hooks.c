#include<libc/ttyprvt.h>

ssize_t (*__libc_read_termios_hook)(int handle, void *buffer, size_t count,
	   			           ssize_t *rv)=NULL;
ssize_t (*__libc_write_termios_hook)(int handle, const void *buffer, size_t count,
					    ssize_t *rv)=NULL;
// int __libc_termios_hook_common_count;
