/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include <libc/dosio.h>
#include <libc/bss.h>

static char init_file_handle_modes[20] = {
  O_TEXT,
  O_TEXT,
  O_TEXT,
  O_BINARY,
  O_BINARY
};

static int dosio_bss_count = -1;
static size_t count=20;	/* DOS default limit */

char *__file_handle_modes = init_file_handle_modes;

void
__file_handle_set(int fd, int mode)
{
  /* Fill in the value */
  __file_handle_modes[fd] = mode;
}
