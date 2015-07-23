#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "render.h"
#include <drm/i915_drm.h>
#include <kos32sys.h>

#define WIN_STATE_MINIMIZED  0x02
#define WIN_STATE_ROLLED     0x04

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
    uint32_t winx, winy, winw, winh;
    uint8_t  state;
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
    state  = *(uint8_t*)(proc_info+70);

    if(state & (WIN_STATE_MINIMIZED|WIN_STATE_ROLLED))
        return;

    context = eglGetCurrentContext();
    draw = eglGetCurrentSurface(EGL_DRAW);
    read = eglGetCurrentSurface(EGL_READ);

    eglSwapBuffers(render->dpy,draw);

    render->back_buffer++;
    render->back_buffer&=1;

    update.handle = render->mask_handle;
    update.dx     = render->dx;
    update.dy     = render->dy;
    update.width  = render->width;
    update.height = render->height;
    update.bo_pitch = (render->width+15) & ~15;
    update.bo_map = (int)render->mask_buffer;

    if(drm_ioctl(render->fd, SRV_MASK_UPDATE_EX, &update))
        return;

    if (!eglMakeCurrent(render->dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, render->context))
    {
        printf("failed to make window current");
        goto err1;
    };

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, render->tx_buffers[render->back_buffer]);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, render->tx_mask);

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

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glFinish();

err1:
    eglMakeCurrent(render->dpy, draw, read, context);
}

