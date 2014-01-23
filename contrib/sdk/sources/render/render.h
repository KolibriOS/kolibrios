#ifndef __GL_RENDER_H__
#define __GL_RENDER_H__

#define EGL_EGLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES

#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "GL/gl.h"

enum px_buffer
{
    PX_FRONT = 0,
    PX_BACK  = 1
};

struct render
{
    int fd;
    EGLDisplay dpy;
    EGLContext context;
    EGLint dx, dy;
    EGLint width, height;
    EGLint scr_width, scr_height;
    GLuint framebuffer;
    EGLImageKHR screen, front, back, mask;
    GLuint tx_buffers[2];
    GLuint tx_screen;
    GLuint tx_mask;
    void *mask_buffer;
    int back_buffer;
    GLuint blit_prog;
    GLint sampler;
    float vertices[8], texcoords[8];
};

struct render* create_render(EGLDisplay dpy, EGLSurface surface, int dx, int dy);
void render_blit(struct render *render, enum px_buffer buffer);
void render_swap_and_blit(struct render *render);

#endif  /* __GL_RENDER_H__ */
