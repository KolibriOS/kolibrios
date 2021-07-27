#include <stdio.h>
#include <sys/ksys.h>
#include <string.h>

static char buf[FILENAME_MAX + 1];
static int static_index = 0;

char *tmpnam(char *name) {
	ksys_thread_t table;
	_ksys_thread_info(&table, -1);

	char *out = name ? name : buf;
	// PID is also unique for each thread
	sprintf(out, "/tmp0/1/%x_%x.tmp", table.pid, static_index++);
	return out;
}
