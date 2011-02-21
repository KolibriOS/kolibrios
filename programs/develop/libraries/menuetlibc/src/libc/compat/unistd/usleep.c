/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>
#include <time.h>

unsigned int usleep(unsigned int _useconds)
{
 clock_t cl_time;
 clock_t start_time = clock();
 _useconds >>= 10;
 cl_time = _useconds * CLOCKS_PER_SEC / 977;
 while (1)
 {
  clock_t elapsed = clock() - start_time;
  if (elapsed >= cl_time) break;
  __menuet__delay100(1);
 }
 return 0;
}
