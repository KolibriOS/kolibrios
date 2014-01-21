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

enum px_buffer
{
    PX_FRONT = 0,
    PX_BACK  = 1
};

struct render
{
    EGLDisplay dpy;
    EGLContext context;
    GLuint framebuffer;
    EGLImageKHR front, back, screen;
    GLuint tx_buffers[2];
    GLuint tx_screen;
    int back_buffer;
    GLuint blit_prog;
    GLint sampler;
    float vertices[8], texcoords[8];
};


EGLImageKHR px_create_image(EGLDisplay display, EGLContext context,
			 int width, int height, int stride, int name);
GLuint create_framebuffer(int width, int height, GLuint *tex);
GLint create_shader(GLenum type, const char *source);
struct render* create_render(EGLDisplay dpy, EGLSurface surface);
void blit_texture(struct render *render, GLuint tex, int x, int y, int w, int h);
void render_swap_buffers(struct render *render, int x, int y, int w, int h);

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

    gs = gbm_surface_create(gbm, 400, 300, GBM_BO_FORMAT_ARGB8888, GBM_BO_USE_RENDERING);

    BeginDraw();
    DrawWindow(20, 20, 400+9, 300+24, "gl-render", 0x000000, 0x74);
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

	glViewport(0, 0, 400, 300);

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

  asm volatile ("int3");

    render = create_render(dpy, surface);
  	glViewport(0, 0, 1024, 768);
    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

    render_swap_buffers(render, 20, 20, 400, 300);

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
	attribs[5] = stride/4;

    printf("%s w:%d :%d pitch:%d handle %d\n", __FUNCTION__,
           width, height, stride, name);

	image = eglCreateImageKHR(display, context, EGL_DRM_BUFFER_MESA,
						 (void *) (uintptr_t)name, attribs);

	return image;
}

GLint create_shader(GLenum type, const char *source)
{
	GLint ok;
    GLint shader;

    shader = glCreateShader(type);
    if(shader == 0)
        goto err;

    glShaderSource(shader, 1, (const GLchar **) &source, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		GLchar *info;
		GLint size;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &size);
		info = malloc(size);

        glGetShaderInfoLog(shader, size, NULL, info);
		printf("Failed to compile %s: %s\n",
                type == GL_FRAGMENT_SHADER ? "FS" : "VS",info);
		printf("Program source:\n%s", source);
		printf("GLSL compile failure\n");
        free(info);
        glDeleteProgram(shader);
        shader = 0;
	}
err:
    return shader;
}


struct render* create_render(EGLDisplay dpy, EGLSurface surface)
{
    const char *vs_src =
	    "attribute vec4 v_position;\n"
	    "attribute vec4 v_texcoord0;\n"
	    "varying vec2 source_texture;\n"
	    "void main()\n"
	    "{\n"
	    "	gl_Position = v_position;\n"
	    "	source_texture = v_texcoord0.xy;\n"
	    "}\n";

	const char *fs_src =
//	    "precision mediump float;\n"
	    "varying vec2 source_texture;\n"
	    "uniform sampler2D sampler;\n"
	    "void main()\n"
	    "{\n"
	    "   vec3 cg = texture2D(sampler, source_texture).rgb;\n"
	    "   gl_FragColor = vec4(cg.r,cg.g,cg.b,1.0);\n"
	    "}\n";
    EGLint config_attribs[14];
    EGLConfig config;
    EGLint num_configs;

    EGLContext context;

    struct drm_i915_fb_info fb;
    GLint  vs_shader, fs_shader;
    GLenum status;
    GLint ret;
    int fd;
    struct render *render;



    fd = get_service("DISPLAY");

    memset(&fb, 0, sizeof(fb));
    ret = drmIoctl(fd, SRV_FBINFO, &fb);
    if( ret != 0 )
    {   printf("failed to get framebuffer info\n");
        goto err;
    };

    render = (struct render*)malloc(sizeof(struct render));
    if(render == NULL)
        goto err;

    render->dpy = dpy;

    render->front = eglGetBufferImage(dpy, surface, EGL_DRM_BUFFER_FRONT);
    if(render->front == EGL_NO_IMAGE_KHR)
        goto err1;

    render->back  = eglGetBufferImage(dpy, surface, EGL_DRM_BUFFER_BACK);
    if( render->back == EGL_NO_IMAGE_KHR)
        goto err2;

    glGenTextures(2, render->tx_buffers);
    if(glGetError() != GL_NO_ERROR)
       goto err3;

    glBindTexture(GL_TEXTURE_2D, render->tx_buffers[EGL_DRM_BUFFER_FRONT]);
    if(glGetError() != GL_NO_ERROR)
       goto err4;

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D,render->front);
    if(glGetError() != GL_NO_ERROR)
       goto err4;

    glBindTexture(GL_TEXTURE_2D, render->tx_buffers[EGL_DRM_BUFFER_BACK]);
    if(glGetError() != GL_NO_ERROR)
       goto err4;

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D,render->back);
    if(glGetError() != GL_NO_ERROR)
       goto err4;

    glBindTexture(GL_TEXTURE_2D, 0);

    render->back_buffer = EGL_DRM_BUFFER_BACK;

    context = eglGetCurrentContext();

    config_attribs[0] = EGL_RED_SIZE;
    config_attribs[1] = 1;
    config_attribs[2] = EGL_GREEN_SIZE;
    config_attribs[3] = 1;
    config_attribs[4] = EGL_BLUE_SIZE;
    config_attribs[5] = 1;
    config_attribs[6] = EGL_DEPTH_SIZE;
    config_attribs[7] = 1;

    config_attribs[8] = EGL_SURFACE_TYPE;
    config_attribs[9] = EGL_WINDOW_BIT;

    config_attribs[10] = EGL_RENDERABLE_TYPE;
    config_attribs[11] = EGL_OPENGL_BIT;
    config_attribs[12] = EGL_NONE;

    if (!eglChooseConfig(dpy,config_attribs, &config, 1, &num_configs) || !num_configs)
    {
        printf("failed to choose a config");
        goto err4;
    }

    render->context = eglCreateContext(dpy, config, EGL_NO_CONTEXT, NULL);
    if (!context)
    {
        printf("failed to create context");
        goto err4;
    };

    if (!eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, render->context))
    {
        printf("failed to make window current");
        goto err5;
    };

    render->screen = px_create_image(dpy,context,fb.width,fb.height,
                                     fb.pitch,fb.name);
    if(render->screen == EGL_NO_IMAGE_KHR)
        goto err6;

    glGenTextures(1, &render->tx_screen);
    if(glGetError() != GL_NO_ERROR)
       goto err6;

    glBindTexture(GL_TEXTURE_2D, render->tx_screen);
    if(glGetError() != GL_NO_ERROR)
       goto err7;

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D,render->screen);
    if(glGetError() != GL_NO_ERROR)
       goto err7;

    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &render->framebuffer);
    if(glGetError() != GL_NO_ERROR)
       goto err8;

    glBindFramebuffer(GL_FRAMEBUFFER, render->framebuffer);
    if(glGetError() != GL_NO_ERROR)
       goto err9;

    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, render->tx_screen,0);

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
        printf("destination is framebuffer incomplete: %s [%#x]\n", str, status);
        goto err9;
    }

    render->blit_prog = glCreateProgram();
    if(render->blit_prog == 0)
        goto err9;

    vs_shader = create_shader(GL_VERTEX_SHADER,vs_src);
    if(vs_shader == 0)
        goto err10;

    fs_shader = create_shader(GL_FRAGMENT_SHADER, fs_src);
    if(fs_shader == 0)
        goto err11;

    glAttachShader(render->blit_prog, vs_shader);
    glAttachShader(render->blit_prog, fs_shader);
    glBindAttribLocation(render->blit_prog, 0, "v_position");
    glBindAttribLocation(render->blit_prog, 1, "v_texcoord0");
    glLinkProgram(render->blit_prog);

    glGetProgramiv(render->blit_prog, GL_LINK_STATUS, &ret);
    if (!ret)
    {
        GLchar *info;
        GLint size;

        glGetProgramiv(render->blit_prog, GL_INFO_LOG_LENGTH, &size);
        info = malloc(size);

        glGetProgramInfoLog(render->blit_prog, size, NULL, info);
        printf("Failed to link: %s\n", info);
        printf("GLSL link failure\n");
        free(info);
    }

    render->sampler = glGetUniformLocation(render->blit_prog,"sampler");

    eglMakeCurrent(dpy, surface, surface, context);

    return render;

err11:
    glDeleteShader(vs_shader);
err10:
    glDeleteProgram(render->blit_prog);
err9:
    glDeleteFramebuffers(1, &render->framebuffer);
err8:
    eglDestroyImageKHR(dpy, render->screen);
err7:
    glDeleteTextures(1, &render->tx_screen);
err6:
    eglMakeCurrent(dpy, surface, surface, context);
err5:
    eglDestroyContext(dpy, render->context);
err4:
    glDeleteTextures(2, render->tx_buffers);
err3:
    eglDestroyImageKHR(dpy, render->back);
err2:
    eglDestroyImageKHR(dpy, render->front);
err1:
    free(render);
err:
    return NULL;
};

void render_swap_buffers(struct render *render, int x, int y, int w, int h)
{
    EGLContext context;
    EGLSurface draw, read;

    float dst_xscale, dst_yscale;
    float *vertices  = render->vertices;
    float *texcoords = render->texcoords;
    int r, b;

    if(render == NULL)
        return;

    context = eglGetCurrentContext();
    draw = eglGetCurrentSurface(EGL_DRAW);
    read = eglGetCurrentSurface(EGL_READ);

    eglSwapBuffers(render->dpy,draw);

    if (!eglMakeCurrent(render->dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, render->context))
    {
        printf("failed to make window current");
        goto err1;
    };

    glUseProgram(render->blit_prog);
    glUniform1i(render->sampler, 0);

    glVertexAttribPointer(0, 2, GL_FLOAT,GL_FALSE, 2 * sizeof(float),render->vertices);
    glEnableVertexAttribArray(0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, render->tx_buffers[render->back_buffer]);
    glTexParameteri(GL_TEXTURE_2D,
                  GL_TEXTURE_MIN_FILTER,
                  GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,
                  GL_TEXTURE_MAG_FILTER,
                  GL_NEAREST);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),render->texcoords);
    glEnableVertexAttribArray(1);

    dst_xscale = 1.0/1024;
    dst_yscale = 1.0/768;

    r = x+w-1;
    b = y+h-1;

    float t0, t1, t2, t5;

    vertices[0]     = t0 = 2*x*dst_xscale - 1.0;
    vertices[1 * 2] = t2 = 2*r*dst_xscale - 1.0;

    vertices[2 * 2] = t2;
    vertices[3 * 2] = t0;

    vertices[1]     = t1 = 2*y*dst_yscale - 1.0;
    vertices[2*2+1] = t5 = 2*b*dst_yscale - 1.0;
    vertices[1*2+1] = t1;
    vertices[3*2+1] = t5;

    texcoords[0]    = 0.0;
    texcoords[1]    = 0.0;
    texcoords[1*2]  = 1.0;
    texcoords[1*2+1]= 0.0;
    texcoords[2*2]  = 1.0;
    texcoords[2*2+1]= 1.0;
    texcoords[3*2]  = 0.0;
    texcoords[3*2+1]= 1.0;

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisable(GL_TEXTURE_2D);
    glUseProgram(0);

    render->back_buffer++;
    render->back_buffer&=1;

err1:
    eglMakeCurrent(render->dpy, draw, read, context);
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
