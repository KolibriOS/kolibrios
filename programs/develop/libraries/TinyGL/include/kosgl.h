#ifndef KOSGL_H
#define KOSGL_H

#include <GL/gl.h>
#include <GL/glu.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef void *KOSGLContext;

extern KOSGLContext kosglCreateContext( KOSGLContext shareList, int flags );

extern void kosglDestroyContext( KOSGLContext ctx1 );

extern int kosglMakeCurrent( int win_x0, int win_y0,
                             int win_x, int win_y,  
                             KOSGLContext ctx);

extern void kosglSwapBuffers(); 

#ifdef __cplusplus
}
#endif

#endif
