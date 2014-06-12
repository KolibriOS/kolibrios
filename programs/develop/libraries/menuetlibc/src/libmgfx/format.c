#include <string.h>
#include"libmgfx.h"

static struct mgfx_image_format * the_fmts=NULL;

void register_image_format(struct mgfx_image_format * fmt)
{
 fmt->next=the_fmts;
 the_fmts=fmt;
}

struct mgfx_image_format * get_image_format(char * fname)
{
 char * p;
 int j;
 struct mgfx_image_format * fmt;
 if(!the_fmts) return NULL;
 p=strstr(fname,".");
 if(!p) return NULL;
 p++;
 j=strlen(p);
 if(!j) return NULL;
 strlwr(p);
 for(fmt=the_fmts;fmt;fmt=fmt->next)
 {
  if(strlen(fmt->fmt_ext)==j)
   if(!strncmp(p,fmt->fmt_ext,j)) return fmt;
 }
 return NULL;
}
