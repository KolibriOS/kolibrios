#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <render.h>
#include "gbm.h"
#include <kos32sys.h>


int main()
{
    struct gbm_device *gbm;
    struct gbm_surface  *gs;
    struct render *render;

    EGLDisplay dpy;
    EGLint major, minor;

    EGLContext context;
    EGLSurface surface;

    EGLConfig config;

    EGLint config_attribs[32];
    EGLint num_configs, i;
    int width  = 400;
    int height = 300;
    int skinh;
    int run =1;

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

    gs = gbm_surface_create(gbm, width, height, GBM_BO_FORMAT_ARGB8888, GBM_BO_USE_RENDERING);

    skinh = get_skin_height();

    BeginDraw();
    DrawWindow(20, 20, width+TYPE_3_BORDER_WIDTH*2,
               height+TYPE_3_BORDER_WIDTH+skinh, "gl-render", 0x000000, 0x74);
    EndDraw();

    surface = eglCreateWindowSurface(dpy,config, (EGLNativeWindowType)gs, NULL);
    if (surface == EGL_NO_SURFACE)
        printf("failed to create surface");

    if (!eglMakeCurrent(dpy, surface, surface, context))
        printf("failed to make window current");


    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -0.5, 1000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glViewport(0, 0, width, height);

    glClearColor( 0, 0, 0, 1);

    glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_QUADS);
    glColor3f(1,0,0);
    glVertex3f( 0.9, -0.9, -30.0);
    glColor3f(1,1,0);
    glVertex3f( 0.9,  0.9, -30.0);

    glColor3f(1,1,1);
    glVertex3f( 0.1,  0.9, -30.0);
    glColor3f(1,0,1);
    glVertex3f( 0.1, -0.9, -30.0);
    glEnd();

    glFlush();

 // asm volatile ("int3");

    render = create_render(dpy, surface, TYPE_3_BORDER_WIDTH, skinh);
    render_blit(render, PX_BACK);

    while (run)
    {
        oskey_t   key;
        int ev;

        ev = get_os_event();
        switch(ev)
        {
            case 1:
                BeginDraw();
                DrawWindow(0,0,0,0,NULL,0,0x74);
                EndDraw();
                render_blit(render, PX_BACK);
                break;

            case 2:
                key = get_key();
                if (key.code == 27)
                    run = 0;
                break;

            case 3:
                if(get_os_button()==1)
                    run = 0;
                break;
        };
    }


    glFinish();
    eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
//    eglDestroySurface(dpy, surface);
  //  gbm_surface_destroy(gs);
    eglDestroyContext(dpy, context);
    eglTerminate(dpy);

    return 0;
}


#if 0
GLuint create_framebuffer(int width, int height, GLuint *tex)
{
    GLuint buffer;

    glGenTextures(1, tex);
    glBindTexture(GL_TEXTURE_2D, *tex);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
			       GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, buffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
					 GL_COLOR_ATTACHMENT0,
					 GL_TEXTURE_2D, *tex,0);
   return buffer;
}
#endif
