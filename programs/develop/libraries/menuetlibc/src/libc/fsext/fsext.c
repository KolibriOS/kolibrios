/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/fsext.h>
#include <libc/bss.h>
#include <libc/dosio.h>

static int num_fds;
static __FSEXT_Function **func_list;
static int tmp_ext=0xF000;

static void
init(void)
{
  static int init_count = -1;
  if (init_count == __bss_count)
    return;
  init_count = __bss_count;
  num_fds = 0;
  func_list = 0;
}

int __FSEXT_alloc_fd(__FSEXT_Function *_function)
{
 int fd;
 init();
 fd=tmp_ext++; 
 __FSEXT_set_function(fd, _function);
 return fd;
}

int
__FSEXT_set_function(int _fd, __FSEXT_Function *_function)
{
  init();

  if (_fd < 0)
    return 1;

  if (num_fds <= _fd)
  {
    int old_fds = num_fds, i;
    num_fds = (_fd+256) & ~255;
    func_list = (__FSEXT_Function **)realloc(func_list, num_fds * sizeof(__FSEXT_Function *));
    if (func_list == 0)
      return 1;
    for (i=old_fds; i<num_fds; i++)
      func_list[i] = 0;
  }
  func_list[_fd] = _function;
  return 0;
}

__FSEXT_Function *
__FSEXT_get_function(int _fd)
{
  init();
  if (_fd < 0 || _fd >= num_fds)
    return 0;
  return func_list[_fd];
}
