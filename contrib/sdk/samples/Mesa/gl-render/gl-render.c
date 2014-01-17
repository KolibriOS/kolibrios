#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define EGL_EGLEXT_PROTOTYPES
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "GL/gl.h"
#include "gbm.h"
#include <kos32sys.h>

#include "eglut.h"

GLint list;

static void init(void)
{
//   fprintf(stderr, "GL_RENDERER   = %s\n", (char *) glGetString(GL_RENDERER));
//   fprintf(stderr, "GL_VERSION    = %s\n", (char *) glGetString(GL_VERSION));
//   fprintf(stderr, "GL_VENDOR     = %s\n", (char *) glGetString(GL_VENDOR));
//   fflush(stderr);

   list = glGenLists(1);
   glNewList(list, GL_COMPILE);

   /* XXX: this state-change will only be executed if list is called
    * from outside a begin/end pair:
    */
   glShadeModel( GL_FLAT );
   glBegin(GL_TRIANGLES);
   glColor3f(0,0,0.7);
   glVertex3f( -0.9,  0.9, -30.0);
   glColor3f(0,0.9,0);
   glVertex3f( -0.9, -0.9, -30.0);
   glColor3f(0.8,0,0);
   glVertex3f(  0.9,  0.0, -30.0);
   glEnd();

   glEndList();
}

static void
idle(void)
{
    eglutPostRedisplay();
}

static void reshape(int width, int height)
{
  asm volatile ("int3");
/*
    glViewport(0, 0, (GLint)width, (GLint)height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -0.5, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
*/
    glViewport(0, 0, (GLint) width, (GLint) height);

    GLfloat h = (GLfloat) height / (GLfloat) width;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -h, h, 5.0, 60.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

static void draw(void)
{
   asm volatile ("int3");

   glClear(GL_COLOR_BUFFER_BIT);

   glShadeModel( GL_SMOOTH );

   glBegin(GL_TRIANGLES);

   /* Note: call the list from inside a begin/end pair.  The end is
    * provided by the display list...
    */
   glCallList(list);

 //  glFlush();

}

int
main(int argc, char *argv[])
{
   eglutInitWindowSize(384, 384);
   eglutInitAPIMask(EGLUT_OPENGL_BIT);
   eglutInit(argc, argv);

   eglutCreateWindow("gl-render");

   eglutIdleFunc(idle);
   eglutReshapeFunc(reshape);
   eglutDisplayFunc(draw);

   glClearColor( 0, 0, 0, 1.0);

   init();
   glDrawBuffer(GL_BACK);

   eglutMainLoop();

   return 0;
}
