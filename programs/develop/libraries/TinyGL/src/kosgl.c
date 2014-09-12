/* simple gl like driver for TinyGL and KolibriOS - porting iadn */
#include <GL/gl.h>
#include "kosgl.h"
#include "zgl.h"



typedef struct {
    GLContext *gl_context;
    int xsize,ysize;
    int dx,dy;
    int x,y;
} TinyGLContext;


KOSGLContext kosglCreateContext(KOSGLContext shareList, int flags)
{
  TinyGLContext *ctx;

  if (shareList != NULL) {
    gl_fatal_error("No sharing available in TinyGL");    
  }

    ctx=gl_malloc(sizeof(TinyGLContext));
  if (!ctx)
      return NULL;
  ctx->gl_context=NULL;
  return (KOSGLContext) ctx;
}

void kosglDestroyContext( KOSGLContext ctx1 )
{
  TinyGLContext *ctx = (TinyGLContext *) ctx1;
  if (ctx->gl_context != NULL) {
    glClose();
  }
  gl_free(ctx);
}

/* resize the glx viewport : we try to use the xsize and ysize
   given. We return the effective size which is guaranted to be smaller */

static int gl_resize_viewport(GLContext *c,int *xsize_ptr,int *ysize_ptr)
{
  TinyGLContext *ctx;
  int xsize,ysize;
   
  ctx=(TinyGLContext *)c->opaque;

  xsize=*xsize_ptr;
  ysize=*ysize_ptr;

  /* we ensure that xsize and ysize are multiples of 2 for the zbuffer. 
     TODO: find a better solution */
  xsize&=~3;
  ysize&=~3;

  if (xsize == 0 || ysize == 0) return -1;

  *xsize_ptr=xsize-1;
  *ysize_ptr=ysize-1;
  ctx->dx = xsize;
  ctx->dy = ysize;
 
  ctx->xsize=xsize;
  ctx->ysize=ysize;  
    
  /* resize the Z buffer */
  ZB_resize(c->zb,NULL,xsize,ysize);
  return 0;
}

/* we assume here that drawable is a window */
int kosglMakeCurrent( int win_x0, int win_y0,int win_x, int win_y, KOSGLContext ctx1)
{ 	
  TinyGLContext *ctx = (TinyGLContext *) ctx1;
  int mode;
  ZBuffer *zb;
 
  if (ctx->gl_context == NULL) {
      /* create the TinyGL context */
      ctx->x = win_x0;
	  ctx->y = win_y0;
	  ctx->dx = win_x;
	  ctx->dy = win_y;

      /* currently, we only support 16 bit rendering */
      mode = ZB_MODE_RGB24;
      zb=ZB_open(win_x,win_y,mode,0,NULL,NULL,NULL);      
    
      if (zb == NULL) {
          fprintf(stderr, "Error while initializing Z buffer\n");
          exit(1);
      }
      
      /* initialisation of the TinyGL interpreter */
      glInit(zb); 

      ctx->gl_context=gl_get_context();
      
      ctx->gl_context->opaque=(void *) ctx;
      ctx->gl_context->gl_resize_viewport=gl_resize_viewport;

      /* set the viewport : we force a call to gl_resize_viewport */
      ctx->gl_context->viewport.xsize=-1;
      ctx->gl_context->viewport.ysize=-1;
      
      glViewport(0, 0, win_x, win_y);
    
  }  
  return 1;
}

void kosglSwapBuffers( )
{
    GLContext *gl_context;
    TinyGLContext *ctx;
     
    /* retrieve the current TinyGLContext */
    gl_context=gl_get_context();
    ctx=(TinyGLContext *)gl_context->opaque;

	__asm__ __volatile__("int $0x40"::"a"(7),
                                      "b"((char *)gl_context->zb->pbuf),
								      "c"((ctx->dx<<16)|ctx->dy),
									  "d"((ctx->x<<16)|ctx->y));
}
