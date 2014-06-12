#include <libc/stubs.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <process.h>
#include <libc/dosexec.h>
#include <libc/unconst.h>
#include <assert.h>

int system (const char *cmdline)
{
 unimpl();
}
