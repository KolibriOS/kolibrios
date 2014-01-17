#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define EGL_EGLEXT_PROTOTYPES
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "GL/gl.h"
#include "gbm.h"
#include <kos32sys.h>
#include <pixlib2.h>

int main()
{
    struct gbm_device *gbm;
    struct gbm_surface  *gs;

    EGLDisplay dpy;
    EGLint major, minor;

    EGLContext context;
    EGLSurface surface;
    EGLConfig config;

    EGLint config_attribs[32];
    EGLint num_configs, i;
    GLint list;

    int fd;

    fd = get_service("DISPLAY");
    gbm = gbm_create_device(fd);
    if( gbm == NULL){
        printf("failed to initialize GBM device");
        return 1;
    };

    dpy = eglGetDisplay((EGLNativeDisplayType)gbm);

    if (!eglInitialize(dpy, &major, &minor))
        printf("failed to initialize EGL display");

    printf("EGL_VERSION = %s\n", eglQueryString(dpy, EGL_VERSION));
    printf("EGL_VENDOR = %s\n", eglQueryString(dpy, EGL_VENDOR));
    printf("EGL_EXTENSIONS = %s\n", eglQueryString(dpy, EGL_EXTENSIONS));
    printf("EGL_CLIENT_APIS = %s\n",eglQueryString(dpy, EGL_CLIENT_APIS));

    i = 0;
    config_attribs[i++] = EGL_RED_SIZE;
    config_attribs[i++] = 1;
    config_attribs[i++] = EGL_GREEN_SIZE;
    config_attribs[i++] = 1;
    config_attribs[i++] = EGL_BLUE_SIZE;
    config_attribs[i++] = 1;
    config_attribs[i++] = EGL_DEPTH_SIZE;
    config_attribs[i++] = 1;

    config_attribs[i++] = EGL_SURFACE_TYPE;
    config_attribs[i++] = EGL_WINDOW_BIT;

    config_attribs[i++] = EGL_RENDERABLE_TYPE;
    config_attribs[i++] = EGL_OPENGL_BIT;
    config_attribs[i] = EGL_NONE;

    if (!eglChooseConfig(dpy,config_attribs, &config, 1, &num_configs) || !num_configs)
        printf("failed to choose a config");

    eglBindAPI(EGL_OPENGL_API);
    context = eglCreateContext(dpy, config, EGL_NO_CONTEXT, NULL);
    if (!context)
        printf("failed to create context");

    gs = gbm_surface_create(gbm, 400, 300, GBM_BO_FORMAT_ARGB8888, GBM_BO_USE_RENDERING);


    BeginDraw();
    DrawWindow(20, 20, 400+9, 300+24, "gl-render", 0x000000, 0x74);
    EndDraw();

    surface = eglCreateWindowSurface(dpy,config, (EGLNativeWindowType)gs, NULL);
    if (surface == EGL_NO_SURFACE)
        printf("failed to create surface");

    if (!eglMakeCurrent(dpy, surface, surface, context))
        printf("failed to make window current");

    glClearColor( 0, 0, 0, 1);

    list = glGenLists(1);
    glNewList(list, GL_COMPILE);

   /* XXX: this state-change will only be executed if list is called
    * from outside a begin/end pair:
    */
    glShadeModel( GL_FLAT );
    glBegin(GL_TRIANGLES);
    glColor3f(0,0,.7);
    glVertex3f( -0.9,  0.9, -30.0);
    glColor3f(0,.9,0);
    glVertex3f( -0.9, -0.9, -30.0);
    glColor3f(.8,0,0);
    glVertex3f(  0.9,  0.0, -30.0);
    glEnd();

   /* This statechange is potentially NOT redundant:
    */
    glShadeModel( GL_FLAT );
    glBegin(GL_TRIANGLES);
    glColor3f(0,1,0);
    glVertex3f( -0.5,  0.5, -30.0);
    glColor3f(0,0,1);
    glVertex3f( -0.5, -0.5, -30.0);
    glColor3f(1,0,0);
    glVertex3f(  0.5,  0.0, -30.0);
    glEnd();

    glEndList();

    glDrawBuffer(GL_BACK);

    glViewport(0, 0, (GLint)400, (GLint)300);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -0.5, 1000.0);
    glMatrixMode(GL_MODELVIEW);

   asm volatile ("int3");


//    glClear(GL_COLOR_BUFFER_BIT);

    glShadeModel( GL_SMOOTH );

    glBegin(GL_TRIANGLES);

   /* Note: call the list from inside a begin/end pair.  The end is
    * provided by the display list...
    */
    glCallList(list);
    glFlush();

  asm volatile ("int3");

    eglSwapBuffers(dpy, surface);

    glFinish();
    eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(dpy, surface);
    gbm_surface_destroy(gs);
    eglDestroyContext(dpy, context);
    eglTerminate(dpy);

    while(1)
    {
        delay(1);
    }
    return 0;
}
