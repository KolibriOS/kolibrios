/* Zgv v2.7 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1995 Russell Marks. See README for license details.
 *
 * readjpeg.c - interface to the IJG's JPEG software, derived from
 *               their example.c.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <setjmp.h>
#include <sys/file.h>  /* for open et al */
#include <jpeglib.h>
#include "libmgfx.h"

static mgfx_image_t * the_image;
static FILE * global_jpeg_infile;
static struct jpeg_decompress_struct cinfo;

struct my_error_mgr 
{
 struct jpeg_error_mgr pub;
 jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr * my_error_ptr;

static void jpegerr(char *msgtext)
{
}

static void my_error_exit(j_common_ptr cinfo)
{
 my_error_ptr myerr=(my_error_ptr) cinfo->err;
 char buf[JMSG_LENGTH_MAX];
 (*cinfo->err->format_message)(cinfo,buf);
 jpegerr(buf);
 longjmp(myerr->setjmp_buffer, 1);
}

static void my_output_message(j_common_ptr cinfo)
{
}

static void aborted_file_jpeg_cleanup()
{
 jpeg_destroy_decompress(&cinfo);
 fclose(global_jpeg_infile);
}

int read_JPEG_file(FILE * in,mgfx_image_t * img)
{
 struct my_error_mgr jerr;
 int row_stride;
 int tmp,f;
 unsigned char *ptr;
 the_image=img;
 if((img->pal=(byte *)malloc(768))==NULL) return(_PICERR_NOMEM);
 cinfo.err=jpeg_std_error(&jerr.pub);
 jerr.pub.error_exit=my_error_exit;
 jerr.pub.output_message=my_output_message;
 if(setjmp(jerr.setjmp_buffer))
 {
  jpeg_destroy_decompress(&cinfo);
  free(img->pal);
  return(_PICERR_CORRUPT);
 }
 jpeg_create_decompress(&cinfo);
 jpeg_stdio_src(&cinfo,in);
 jpeg_read_header(&cinfo,TRUE);
 cinfo.dct_method=JDCT_FLOAT;
 img->bpp=24;
 if(cinfo.jpeg_color_space==JCS_GRAYSCALE)
 {
  cinfo.out_color_space=JCS_GRAYSCALE;
  cinfo.desired_number_of_colors=256;
  cinfo.quantize_colors=FALSE;
  cinfo.two_pass_quantize=FALSE;
  img->bpp=8;
  for(f=0;f<256;f++)
   img->pal[f]=img->pal[256+f]=img->pal[512+f]=f;
 }
 img->width=cinfo.image_width;
 img->height=cinfo.image_height;
 img->the_image=(byte *)malloc(((img->bpp+7)/8)*img->width*img->height);
 if(img->the_image==NULL)
 {
  jpegerr("Out of memory");
  longjmp(jerr.setjmp_buffer,1);
 }
 jpeg_start_decompress(&cinfo);
 if(img->bpp==8 && cinfo.jpeg_color_space!=JCS_GRAYSCALE)
   for(f=0;f<cinfo.actual_number_of_colors;f++)
   {
    img->pal[    f]=cinfo.colormap[0][f];
    img->pal[256+f]=cinfo.colormap[1][f];
    img->pal[512+f]=cinfo.colormap[2][f];
   }
 ptr=img->the_image;
 row_stride=((img->bpp+7)/8)*img->width;
 if(img->bpp==8)
   while(cinfo.output_scanline<img->height)
    {
    jpeg_read_scanlines(&cinfo,&ptr,1);
    ptr+=row_stride;
    }
 else
  while(cinfo.output_scanline<img->height)
    {
    jpeg_read_scanlines(&cinfo,&ptr,1);
    for(f=0;f<img->width;f++) { tmp=*ptr; *ptr=ptr[2]; ptr[2]=tmp; ptr+=3; }
    }
 jpeg_finish_decompress(&cinfo);
 jpeg_destroy_decompress(&cinfo);
 return(_PIC_OK);
}

static struct mgfx_image_format this_fmt_1={
 "JPEG",
 "jpg",
 read_JPEG_file,
 NULL,
};

static struct mgfx_image_format this_fmt_2={
 "JPEG",
 "jpeg",
 read_JPEG_file,
 NULL,
};

void mgfx_register_jpeg(void)
{
 register_image_format(&this_fmt_1);
 register_image_format(&this_fmt_2);
}
