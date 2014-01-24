#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "render.h"
#include <i915_drm.h>
#include <kos32sys.h>

static int drm_ioctl(int fd, unsigned long request, void *arg)
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

void render_swap_and_blit(struct render *render)
{
    char proc_info[1024];
    struct drm_i915_mask_update update;

    EGLContext context;
    EGLSurface draw, read;
    int winx, winy, winw, winh;

    float xscale, yscale;
    float *vertices  = render->vertices;
    float *texcoords = render->tc_src;
    int r, b;

    if(render == NULL)
        return;

    get_proc_info(proc_info);

    winx = *(uint32_t*)(proc_info+34);
    winy = *(uint32_t*)(proc_info+38);
    winw = *(uint32_t*)(proc_info+42)+1;
    winh = *(uint32_t*)(proc_info+46)+1;

    context = eglGetCurrentContext();
    draw = eglGetCurrentSurface(EGL_DRAW);
    read = eglGetCurrentSurface(EGL_READ);

    eglSwapBuffers(render->dpy,draw);

    if (!eglMakeCurrent(render->dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, render->context))
    {
        printf("failed to make window current");
        goto err1;
    };

#if 0
    glUseProgram(render->blit_prog);
    glUniform1i(render->sampler, 0);

    glVertexAttribPointer(0, 2, GL_FLOAT,GL_FALSE, 2 * sizeof(float),render->vertices);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),render->texcoords);
    glEnableVertexAttribArray(1);

#endif

    update.handle = render->mask_handle;
    update.bo_map = (int)render->mask_buffer;
    drm_ioctl(render->fd, SRV_MASK_UPDATE, &update);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, render->tx_buffers[render->back_buffer]);
    glTexParameteri(GL_TEXTURE_2D,
                  GL_TEXTURE_MIN_FILTER,
                  GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,
                  GL_TEXTURE_MAG_FILTER,
                  GL_NEAREST);


    xscale = 1.0/render->scr_width;
    yscale = 1.0/render->scr_height;

    r = winx + render->dx + render->width;
    b = winy + render->dy + render->height;

    float t0, t1, t2, t5;

    vertices[0]     = t0 = 2*(winx+render->dx)*xscale - 1.0;
    vertices[1 * 2] = t2 = 2*r*xscale - 1.0;

    vertices[2 * 2] = t2;
    vertices[3 * 2] = t0;

    vertices[1]     = t1 = 2*(winy+render->dy)*yscale - 1.0;
    vertices[2*2+1] = t5 = 2*b*yscale - 1.0;
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

    texcoords = render->tc_mask;

    xscale = 1.0/winw;
    yscale = 1.0/winh;

    texcoords[0]    = render->dx * xscale;
    texcoords[1]    = render->dy * yscale;
    texcoords[1*2]  = (render->dx+render->width)*xscale;
    texcoords[1*2+1]= render->dy * yscale;
    texcoords[2*2]  = (render->dx+render->width)*xscale;
    texcoords[2*2+1]= (render->dy+render->height)*yscale;
    texcoords[3*2]  = render->dx * xscale;
    texcoords[3*2+1]= (render->dy+render->height)*yscale;


    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glFlush();

//    glDisableVertexAttribArray(0);
//    glDisableVertexAttribArray(1);
//    glDisable(GL_TEXTURE_2D);
//    glUseProgram(0);

    render->back_buffer++;
    render->back_buffer&=1;

err1:
    eglMakeCurrent(render->dpy, draw, read, context);
}

