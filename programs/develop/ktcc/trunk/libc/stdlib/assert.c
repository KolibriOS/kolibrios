#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <kolibrisys.h>


void __assert_func (char *file, int line, char *ass)
{
	char buf[100];

	snprintf(buf,100,"Assertion failed: %s, file %s, line %d\n", ass, file, line);
	debug_out_str(buf);
	exit(-1);
}

void __trace_func (char *file, int line, char *msg)
{
	char buf[100];
	snprintf(buf,100,"Trace: %s, file %s, line %d\n", msg, file, line);
	debug_out_str(buf);
}
