/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <time.h>
#include <libc/farptrgs.h>
#include <libc/bss.h>

clock_t clock(void)
{
  unsigned result;
  __asm__ __volatile__("int $0x40" : "=a"(result) : "a"(26), "b"(9));
  return result;
}
