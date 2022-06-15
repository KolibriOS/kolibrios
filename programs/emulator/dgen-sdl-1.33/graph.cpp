// DGen v1.13+
#include <stdio.h>
#include "md.h"

// This allows you to get the palette, mess around with it
// (e.g. contrast/brightness)
// and then pass it back
int get_md_palette(unsigned char pal[256],unsigned char *cram)
{
  int c;
  if (pal==NULL) return 1;
  for (c=0;c<64;c++)
  {
    int r,g,b;
    b=(cram[c*2+0]&0x0e)<<4;
    g=(cram[c*2+1]&0xe0);
    r=(cram[c*2+1]&0x0e)<<4;

    pal[c*4+0]=r;
    pal[c*4+1]=g;
    pal[c*4+2]=b;
  }
  return 0;
}
