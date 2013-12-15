/*
 * Demo of off-screen Mesa rendering
 *
 * See Mesa/include/GL/osmesa.h for documentation of the OSMesa functions.
 *
 * If you want to render BIG images you'll probably have to increase
 * MAX_WIDTH and MAX_Height in src/config.h.
 *
 * This program is in the public domain.
 *
 * Brian Paul
 *
 * PPM output provided by Joerg Schmalzl.
 * ASCII PPM output added by Brian Paul.
 *
 * Usage: osdemo [filename]
 */


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define GL_GLEXT_PROTOTYPES
#include "GL/osmesa.h"
#include <GL/glext.h>
#include "GL/glu.h"
#include "shaderutil.h"
#include <kos32sys.h>

int _CRT_MT=0;

static int Width = 500;
static int Height = 400;

int check_events();

GLfloat view_rotx = 20.0, view_roty = 30.0, view_rotz = 0.0;
GLfloat angle = 0.0;

GLboolean animate = GL_TRUE;	/* Animation */
GLfloat eyesep = 5.0;		/* Eye separation. */
GLfloat fix_point = 40.0;	/* Fixation point distance.  */
GLfloat left, right, asp;	/* Stereo frustum params.  */

void Init( void );
void Reshape( int width, int height );
void Draw( void );
void Idle();
void Key(unsigned char key, int x, int y);


inline void Blit(void *bitmap, int dst_x, int dst_y,
                        int src_x, int src_y, int w, int h,
                        int src_w, int src_h, int stride)
{
    volatile struct blit_call bc;

    bc.dstx = dst_x;
    bc.dsty = dst_y;
    bc.w    = w;
    bc.h    = h;
    bc.srcx = src_x;
    bc.srcy = src_y;
    bc.srcw = src_w;
    bc.srch = src_h;
    bc.stride = stride;
    bc.bitmap = bitmap;

    __asm__ __volatile__(
    "int $0x40"
    ::"a"(73),"b"(0),"c"(&bc.dstx));

};


static inline uint32_t wait_os_event(int time)
{
    uint32_t val;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(23),"b"(time));
    return val;
};


int main(int argc, char *argv[])
{
    OSMesaContext ctx;
    void *buffer;
    char *filename = NULL;
    int ev;
    int repeat=1;

   /* Create an RGBA-mode context */
   /* specify Z, stencil, accum sizes */

    ctx = OSMesaCreateContextExt( OSMESA_RGBA, 16, 0, 0, NULL );
    if (!ctx) {
        printf("OSMesaCreateContext failed!\n");
        return 0;
    }

   /* Allocate the image buffer */
    buffer = malloc( Width * Height * 4 * sizeof(GLubyte) );
    if (!buffer) {
        printf("Alloc image buffer failed!\n");
        return 0;
    }

 //  __asm__ __volatile__("int3");

   /* Bind the buffer to the context and make it current */
   if (!OSMesaMakeCurrent( ctx, buffer, GL_UNSIGNED_BYTE, Width, Height )) {
      printf("OSMesaMakeCurrent failed!\n");
      return 0;
   }

   {
      int z, s, a;
      glGetIntegerv(GL_DEPTH_BITS, &z);
      glGetIntegerv(GL_STENCIL_BITS, &s);
      glGetIntegerv(GL_ACCUM_RED_BITS, &a);
      printf("Depth=%d Stencil=%d Accum=%d\n", z, s, a);
   }

    Reshape(Width, Height);
    Init();
    Draw();

    printf("all done\n");

    DrawWindow(10, 10, Width+9, Height+26, "OpenGL Engine Demo", 0x000000, 0x74);
    Blit(buffer, 5, 22, 0, 0, Width, Height, Width,Height,Width*4);

    while(repeat)
    {
        oskey_t   key;

        ev = wait_os_event(1);
        switch(ev)
        {
            case 1:
                DrawWindow(10, 10, Width+9, Width+26, NULL, 0x000000,0x74);
                Blit(buffer, 5, 22, 0, 0, Width, Height, Width,Height,Width*4);
                continue;

            case 2:
                key = get_key();
                Key(key.code, 0, 0);
                Draw();
                Blit(buffer, 5, 22, 0, 0, Width, Height, Width,Height,Width*4);
                continue;

            case 3:
                if(get_os_button()==1)
                    repeat=0;
                    continue;
        };
        Idle();
        Draw();
        Blit(buffer, 5, 22, 0, 0, Width, Height, Width,Height,Width*4);
    };

   /* free the image buffer */
   free( buffer );

   /* destroy the context */
   OSMesaDestroyContext( ctx );

   return 0;
}

int atexit(void (*func)(void))
{
    return 0;
};

