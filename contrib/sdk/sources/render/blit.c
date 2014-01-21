#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "render.h"
#include <kos32sys.h>

void render_blit(struct render *render, enum px_buffer buffer)
{
    char proc_info[1024];

    EGLContext context;
    EGLSurface draw, read;
    int winx, winy;

    float dst_xscale, dst_yscale;
    float *vertices  = render->vertices;
    float *texcoords = render->texcoords;
    int r, b;

    if(render == NULL)
        return;

    get_proc_info(proc_info);

    winx = *(uint32_t*)(proc_info+34);
    winy = *(uint32_t*)(proc_info+38);

    context = eglGetCurrentContext();
    draw = eglGetCurrentSurface(EGL_DRAW);
    read = eglGetCurrentSurface(EGL_READ);

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
    glBindTexture(GL_TEXTURE_2D, render->tx_buffers[buffer]);
    glTexParameteri(GL_TEXTURE_2D,
                  GL_TEXTURE_MIN_FILTER,
                  GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,
                  GL_TEXTURE_MAG_FILTER,
                  GL_NEAREST);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),render->texcoords);
    glEnableVertexAttribArray(1);

    dst_xscale = 1.0/render->scr_width;
    dst_yscale = 1.0/render->scr_height;

    r = winx + render->dx + render->width ;
    b = winy + render->dy + render->height;

    float t0, t1, t2, t5;

    vertices[0]     = t0 = 2*(winx+render->dx)*dst_xscale - 1.0;
    vertices[1 * 2] = t2 = 2*r*dst_xscale - 1.0;

    vertices[2 * 2] = t2;
    vertices[3 * 2] = t0;

    vertices[1]     = t1 = 2*(winy+render->dy)*dst_yscale - 1.0;
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

err1:
    eglMakeCurrent(render->dpy, draw, read, context);
}

