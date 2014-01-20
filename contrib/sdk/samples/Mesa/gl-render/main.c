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
GLuint create_framebuffer(int width, int height, GLuint *tex);
GLint create_shader(GLenum type, const char *source);

int main()
{
    struct gbm_device *gbm;
    struct gbm_surface  *gs;

    EGLDisplay dpy;
    EGLint major, minor;

    EGLContext context;
    EGLSurface surface;
    EGLImageKHR front,fb_image;
    EGLConfig config;

    EGLint config_attribs[32];
    EGLint num_configs, i;
    GLuint texture, buffer;
    GLuint f_tex;

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

    sna_create_mask();

    surface = eglCreateWindowSurface(dpy,config, (EGLNativeWindowType)gs, NULL);
    if (surface == EGL_NO_SURFACE)
        printf("failed to create surface");

    if (!eglMakeCurrent(dpy, surface, surface, context))
        printf("failed to make window current");


    front = eglGetBufferImage(dpy, surface, EGL_DRM_BUFFER_BACK);
    glGenTextures(1, &f_tex);
    glBindTexture(GL_TEXTURE_2D, f_tex);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D,front);
    glBindTexture(GL_TEXTURE_2D, 0);

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

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
    }

	glViewport(0, 0, 1024, 768);
    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


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

	GLuint blit_prog;
	GLint  vs_shader, fs_shader;

  asm volatile ("int3");

	blit_prog = glCreateProgram();
    vs_shader = create_shader(GL_VERTEX_SHADER,vs_src);
    fs_shader = create_shader(GL_FRAGMENT_SHADER, fs_src);
 	glAttachShader(blit_prog, vs_shader);
 	glAttachShader(blit_prog, fs_shader);
 	glBindAttribLocation(blit_prog, 0, "v_position");
	glBindAttribLocation(blit_prog, 1, "v_texcoord0");

	GLint ok;

	glLinkProgram(blit_prog);
	glGetProgramiv(blit_prog, GL_LINK_STATUS, &ok);
	if (!ok) {
		GLchar *info;
		GLint size;

		glGetProgramiv(blit_prog, GL_INFO_LOG_LENGTH, &size);
		info = malloc(size);

		glGetProgramInfoLog(blit_prog, size, NULL, info);
		printf("Failed to link: %s\n", info);
		printf("GLSL link failure\n");
	}

    GLint sampler;
	float vertices[8], texcoords[8];
    GLfloat dst_xscale, dst_yscale; //, src_xscale, src_yscale;
    int l, t, r, b, stride;

	sampler = glGetUniformLocation(blit_prog,"sampler");
	glUseProgram(blit_prog);
	glUniform1i(sampler, 0);

	glVertexAttribPointer(0, 2, GL_FLOAT,GL_FALSE, 2 * sizeof(float),vertices);
	glEnableVertexAttribArray(0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, f_tex);
	glTexParameteri(GL_TEXTURE_2D,
				  GL_TEXTURE_MIN_FILTER,
				  GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,
				  GL_TEXTURE_MAG_FILTER,
				  GL_NEAREST);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),texcoords);
	glEnableVertexAttribArray(1);

	dst_xscale = 1.0/1024;
	dst_yscale = 1.0/768;
//	src_xscale = 1.0/400;
//	src_yscale = 1.0/300;

    stride = 2;

	l = 20;
	t = 20;
	r = l+400;
	b = t+300;

    float t0, t1, t2, t5;

    vertices[0]     = t0 = 2*l*dst_xscale - 1.0;
    vertices[1 * 2] = t2 = 2*r*dst_xscale - 1.0;

    vertices[2 * 2] = t2;
    vertices[3 * 2] = t0;

    vertices[1]     = t1 = 2*t*dst_yscale - 1.0;
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
	GLint prog;

	prog = glCreateShader(type);
	glShaderSource(prog, 1, (const GLchar **) &source, NULL);
	glCompileShader(prog);
	glGetShaderiv(prog, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		GLchar *info;
		GLint size;

		glGetShaderiv(prog, GL_INFO_LOG_LENGTH, &size);
		info = malloc(size);

		glGetShaderInfoLog(prog, size, NULL, info);
		printf("Failed to compile %s: %s\n",
                type == GL_FRAGMENT_SHADER ? "FS" : "VS",info);
		printf("Program source:\n%s", source);
		printf("GLSL compile failure\n");
	}

	return prog;
}

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
