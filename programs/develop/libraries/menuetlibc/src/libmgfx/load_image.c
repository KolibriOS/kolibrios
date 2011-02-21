#include"libmgfx.h"

int load_image(char * fname,mgfx_image_t ** the_img)
{
 FILE * f;
 struct mgfx_image_format * fmt;
 mgfx_image_t * img;
 int __ret;
 if(!(img=(mgfx_image_t *)malloc(sizeof(mgfx_image_t))))
  return _PICERR_NOMEM;
 if(!(fmt=get_image_format(fname)))
  return _PICERR_UNSUPPORTED;
 f=fopen(fname,"rb");
 if(!f) return _PICERR_NOFILE;
 img->fmt=fmt;
 __ret=fmt->load_fn(f,img);
 if(__ret==_PIC_OK)
 {
  *the_img=img;
 } else {
  *the_img=NULL;
  free(img);
 }
 fclose(f);
 return __ret;
}

int load_image_f(char * fname,FILE * f,mgfx_image_t ** the_img)
{
 struct mgfx_image_format * fmt;
 mgfx_image_t * img;
 int __ret;
 if(!(img=(mgfx_image_t *)malloc(sizeof(mgfx_image_t))))
  return _PICERR_NOMEM;
 if(!(fmt=get_image_format(fname)))
  return _PICERR_UNSUPPORTED;
 img->fmt=fmt;
 __ret=fmt->load_fn(f,img);
 if(__ret==_PIC_OK)
 {
  *the_img=img;
 } else {
  *the_img=NULL;
  free(img);
 }
 fclose(f);
 return __ret;
}

void free_image(mgfx_image_t * img)
{
 if(img)
 {
  free(img->the_image);
 }
}

void paint_image(int x,int y,mgfx_image_t * img)
{
 if(!img || img->bpp!=24) return;
 __menuet__putimage(x,y,img->width,img->height,img->the_image);
}
