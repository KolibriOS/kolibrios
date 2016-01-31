#ifndef __GL_RENDER_H__
#define __GL_RENDER_H__

#define EGL_EGLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES

#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "EGL/eglmesaext.h"
#include "GL/gl.h"

struct bitmap
{
    uint32_t    width;
    uint32_t    height;
    uint32_t    pitch;
    void       *buffer;
    GLuint      tex;
    GLuint      handle;
    GLuint      name;
    EGLImageKHR image;
};

struct planar
{
    uint32_t    width;
    uint32_t    height;
    GLuint      name;
    EGLImageKHR planar_image;
    EGLImageKHR image[3];
    GLuint      offset[3];
    GLuint      pitch[3];
    GLuint      tex[3];
};

enum
{
    TEX_SCREEN = 0,
    TEX_MASK   = 1
};

struct render
{
    int fd;
    EGLDisplay dpy;
    EGLContext context;
    EGLint dx;
    EGLint dy;
    EGLint width;
    EGLint height;
    EGLint scr_width;
    EGLint scr_height;
    GLuint texture[2];
    GLuint framebuffer;
    EGLImageKHR screen;
    struct bitmap mask;

    int    mask_size;

    GLuint blit_prog;
    GLint sampler, sm_mask;
    float vertices[8],tc_src[8];
};

int egl_initialize(EGLDisplay *dpy, EGLConfig *config, EGLContext *context);
void egl_destroy(EGLDisplay dpy, EGLContext context);

#define ENTER()   printf("enter %s\n",__FUNCTION__)
#define LEAVE()   printf("leave %s\n",__FUNCTION__)
#define FAIL()    printf("fail %s\n",__FUNCTION__)

#ifdef DEBUG
#define DBG(...) do {                   \
    fprintf(stderr, __VA_ARGS__);       \
} while (0)
#else
#define DBG(...)
#endif

#endif  /* __GL_RENDER_H__ */
