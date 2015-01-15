
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <drm/i915_drm.h>
#include <kos32sys.h>

#define EGL_EGLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES

#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "GL/gl.h"
#include "gbm.h"
#include "pxgl.h"

static struct gbm_device *gbm;

static EGLConfig choose_config(EGLDisplay *dpy)
{
   EGLConfig config = NULL;
   EGLint config_attribs[32];
   EGLint num_configs, i;

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

   eglChooseConfig(dpy, config_attribs, &config, 1, &num_configs);

   return config;
};

int egl_initialize(EGLDisplay *dpy, EGLConfig *config, EGLContext *context)
{
    EGLint major, minor;
    int fd;

    fd = get_service("DISPLAY");
    if(fd == 0)
        return -1;

    gbm = gbm_create_device(fd);
    if( gbm == NULL)
    {
        DBG("failed to initialize GBM device\n");
        goto err_0;
    };

    *dpy = eglGetDisplay(gbm);

    if (!eglInitialize(*dpy, &major, &minor))
    {
        DBG("failed to initialize EGL display\n");
        goto err_1;
    };

    DBG("EGL_VERSION = %s\n", eglQueryString(*dpy, EGL_VERSION));
    DBG("EGL_VENDOR = %s\n", eglQueryString(*dpy, EGL_VENDOR));
    DBG("EGL_EXTENSIONS = %s\n", eglQueryString(*dpy, EGL_EXTENSIONS));
    DBG("EGL_CLIENT_APIS = %s\n", eglQueryString(*dpy, EGL_CLIENT_APIS));

    *config = choose_config(*dpy);
    if( *config == NULL)
    {
        DBG("failed to choose a config\n");
        goto err_2;
    };

    eglBindAPI(EGL_OPENGL_API);
    *context = eglCreateContext(*dpy, *config, EGL_NO_CONTEXT, NULL);
    if (context == NULL)
    {
        DBG("failed to create context\n");
        goto err_2;
    };

    if (!eglMakeCurrent(*dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, *context))
    {
        DBG("failed to make context current");
        goto err_3;
    };

    return 0;

err_3:
    eglDestroyContext(*dpy, *context);
err_2:
    eglTerminate(*dpy);
err_1:
    gbm_device_destroy(gbm);
err_0:
    return -1;
};

void egl_destroy(EGLDisplay dpy, EGLContext context)
{
    eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(dpy, context);
    eglTerminate(dpy);
    gbm_device_destroy(gbm);
};

