#ifndef __LIBMGFX_H
#define __LIBMGFX_H

#include<menuet/os.h>
#include<stdlib.h>
#include<stdio.h>
/* #include<string.h> */

typedef unsigned char byte;

typedef struct {
 int width,height;
 int bpp;
 byte * the_image;
 byte * pal;
 struct mgfx_image_format * fmt;
} mgfx_image_t;

struct mgfx_image_format {
 char * format_name;
 char * fmt_ext;
 int (* load_fn)(FILE * f,mgfx_image_t *);
 struct mgfx_image_format * next;
};

void register_image_format(struct mgfx_image_format * fmt);
struct mgfx_image_format * get_image_format(char * fname);

#define _PIC_OK			 0
#define _PICERR_NOFILE		-1
#define _PICERR_NOMEM		-2
#define _PICERR_BADMAGIC	-3
#define _PICERR_NOCOLOURMAP	-4
#define _PICERR_NOIMAGE		-5
#define _PICERR_UNSUPPORTED	-6
#define _PICERR_CORRUPT		-7
#define _PICERR_SHOWN_ALREADY	-8
#define _PICERR_ISRLE		-9

int load_image(char * fname,mgfx_image_t ** the_img);
void free_image(mgfx_image_t * img);
void paint_image(int x,int y,mgfx_image_t * img);

void mgfx_register_jpeg(void);

void init_mgfx_library(void);

#endif
