/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <pc.h>
#include <assert.h>

void sound(int freq)
{
  int scale;
  if (freq == 0)
  {
    outportb(0x61, inportb(0x61) & ~3);
    return;
  }
  scale = 1193046 / freq;
  outportb(0x43, 0xb6);
  outportb(0x42, scale & 0xff);
  outportb(0x42, scale >> 8);
  outportb(0x61, inportb(0x61) | 3);
}
