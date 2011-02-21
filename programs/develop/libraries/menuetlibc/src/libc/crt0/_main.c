/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/internal.h>
#include <libc/bss.h>

typedef void (*FUNC)(void);
extern FUNC djgpp_first_ctor[] __asm__("djgpp_first_ctor");
extern FUNC djgpp_last_ctor[] __asm__("djgpp_last_ctor");

void
__main(void)
{
  static int been_there_done_that = -1;
  int i;
  if (been_there_done_that == __bss_count)
    return;
  been_there_done_that = __bss_count;
  for (i=0; i<djgpp_last_ctor-djgpp_first_ctor; i++)
    djgpp_first_ctor[i]();
}
