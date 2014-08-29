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
#include <sys/time.h>
#include <assert.h>
#define GL_GLEXT_PROTOTYPES
#include "GL/osmesa.h"
#include <GL/glext.h>
#include "eglut.h"
#include <kos32sys.h>

void init(void);
void reshape(int width, int height);
void draw(void);
void keyboard(unsigned char key);

extern GLfloat angle;

static void idle(void)
{
    static uint32_t t0;
    uint32_t t, dt;

    t = get_tick_count();

    dt = t - t0;
    t0 = t;

    angle += 70.0 * dt / 100;  /* 70 degrees per second */
    if (angle > 3600.0)
        angle -= 3600.0;
}

static int time_now(void)
{
   struct timeval tv;
   struct timezone tz;
   (void) gettimeofday(&tv, &tz);
   return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int main(int argc, char *argv[])
{
   int Width  = 384;
   int Height = 384;

    OSMesaContext ctx;
    void *buffer;
    char *filename = NULL;
    int ev;
    int repeat=1;

   /* Create an RGBA-mode context */
   /* specify Z, stencil, accum sizes */

    ctx = OSMesaCreateContextExt( OSMESA_BGRA, 16, 0, 0, NULL );
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

    reshape(Width, Height);

    glClearColor( 0, 0, 0, 1);

    init();
    draw();

    DrawWindow(10, 10, Width+TYPE_3_BORDER_WIDTH*2, Height+TYPE_3_BORDER_WIDTH+get_skin_height(),
               "OpenGL Engine Demo", 0x000000, 0x74);
    Blit(buffer, TYPE_3_BORDER_WIDTH, get_skin_height(), 0, 0, Width, Height, Width,Height,Width*4);

    int start = time_now();
    int frames = 0;

    while(repeat)
    {
        int now = time_now();
        oskey_t   key;

        ev = check_os_event();
        switch(ev)
        {
            case 1:
                DrawWindow(0,0,0,0, NULL, 0x000000,0x74);
                break;

            case 2:
                key = get_key();
                keyboard(key.code);
                break;

            case 3:
                if(get_os_button()==1)
                    repeat=0;
                    continue;
        };

        if (now - start >= 5000)
        {
            double elapsed = (double) (now - start) / 1000.0;

            printf("%d frames in %3.1f seconds = %6.3f FPS\n",
                    frames, elapsed, frames / elapsed);
            fflush(stdout);

            start = now;
            frames = 0;
        }

        idle();
        draw();
        glFlush();
        Blit(buffer, TYPE_3_BORDER_WIDTH, get_skin_height(), 0, 0, Width, Height, Width,Height,Width*4);
        frames++;
    };

   /* free the image buffer */
   free( buffer );

   /* destroy the context */
   OSMesaDestroyContext( ctx );

   return 0;
}

