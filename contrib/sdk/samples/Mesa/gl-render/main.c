#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define EGL_EGLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES

#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "GL/gl.h"
#include "gbm.h"
#include <i915_drm.h>
#include <kos32sys.h>
#include <pixlib2.h>

EGLImageKHR px_create_image(EGLDisplay display, EGLContext context,
			 int width, int height, int stride, int name);

int main()
{
    struct gbm_device *gbm;
    struct gbm_surface  *gs;

    EGLDisplay dpy;
    EGLint major, minor;

    EGLContext context;
    EGLSurface surface;
    EGLImageKHR fb_image;
    EGLConfig config;

    EGLint config_attribs[32];
    EGLint num_configs, i;
    GLint list;
    GLuint texture, buffer;


    int fd;

    fd = get_service("DISPLAY");
    gbm = gbm_create_device(fd);
    if( gbm == NULL){
        printf("failed to initialize GBM device");
        return 1;
    };

    init_pixlib(HW_BIT_BLIT);

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

    gs = gbm_surface_create(gbm, 1024, 768, GBM_BO_FORMAT_ARGB8888, GBM_BO_USE_RENDERING);


    BeginDraw();
    DrawWindow(20, 20, 400+9, 300+24, "gl-render", 0x000000, 0x74);
    EndDraw();

    sna_create_mask();

    surface = eglCreateWindowSurface(dpy,config, (EGLNativeWindowType)gs, NULL);
    if (surface == EGL_NO_SURFACE)
        printf("failed to create surface");

    if (!eglMakeCurrent(dpy, surface, surface, context))
        printf("failed to make window current");


    if(fd)
    {
        int ret;
        GLenum status;
        struct drm_i915_fb_info fb;

        memset(&fb, 0, sizeof(fb));
        ret = drmIoctl(fd, SRV_FBINFO, &fb);
        if( ret != 0 )
            printf("failed to get framebuffer info\n");

		fb_image = px_create_image(dpy,context,fb.width,fb.height,
                                   fb.pitch,fb.name);

        printf("fb_image %p\n", fb_image);

  asm volatile ("int3");

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D,fb_image);
        glBindTexture(GL_TEXTURE_2D, 0);

  	    glGenFramebuffers(1, &buffer);
        glBindFramebuffer(GL_FRAMEBUFFER, buffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER,
					 GL_COLOR_ATTACHMENT0,
					 GL_TEXTURE_2D, texture,0);
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            const char *str;
            switch (status)
            {
                case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                    str = "incomplete attachment";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                    str = "incomplete/missing attachment";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                    str = "incomplete draw buffer";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                    str = "incomplete read buffer";
                    break;
                case GL_FRAMEBUFFER_UNSUPPORTED:
                    str = "unsupported";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                    str = "incomplete multiple";
                    break;
                default:
                    str = "unknown error";
                    break;
            }

            printf("destination is framebuffer incomplete: %s [%#x]\n",
			   str, status);
        }

        glViewport(0, 0,fb.width, fb.height);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-1.0, 1.0, -1.0, 1.0, -0.5, 100.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

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

  asm volatile ("int3");

    glDrawBuffer(GL_BACK);

    glClear(GL_COLOR_BUFFER_BIT);

    glShadeModel( GL_SMOOTH );

    glBegin(GL_TRIANGLES);

   /* Note: call the list from inside a begin/end pair.  The end is
    * provided by the display list...
    */
    glCallList(list);
    glFlush();


//    eglSwapBuffers(dpy, surface);

    glFinish();
    eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
//    eglDestroySurface(dpy, surface);
  //  gbm_surface_destroy(gs);
    eglDestroyContext(dpy, context);
    eglTerminate(dpy);

    while(1)
    {
        delay(1);
    }

    return 0;
}

int drmIoctl(int fd, unsigned long request, void *arg)
{
    ioctl_t  io;

    io.handle   = fd;
    io.io_code  = request;
    io.input    = arg;
    io.inp_size = 64;
    io.output   = NULL;
    io.out_size = 0;

    return call_service(&io);
}

EGLImageKHR px_create_image(EGLDisplay display, EGLContext context,
			 int width, int height, int stride, int name)
{
	EGLImageKHR image;
	EGLint attribs[] = {
		EGL_WIDTH, 0,
		EGL_HEIGHT, 0,
		EGL_DRM_BUFFER_STRIDE_MESA, 0,
		EGL_DRM_BUFFER_FORMAT_MESA,
		EGL_DRM_BUFFER_FORMAT_ARGB32_MESA,
		EGL_DRM_BUFFER_USE_MESA,
		EGL_DRM_BUFFER_USE_SHARE_MESA |
		    EGL_DRM_BUFFER_USE_SCANOUT_MESA,
		EGL_NONE
	};
	attribs[1] = width;
	attribs[3] = height;
	attribs[5] = stride;

    printf("%s w:%d :%d pitch:%d handle %d\n", __FUNCTION__,
           width, height, stride, name);

	image = eglCreateImageKHR(display, context, EGL_DRM_BUFFER_MESA,
						 (void *) (uintptr_t)name, attribs);

	return image;
}
