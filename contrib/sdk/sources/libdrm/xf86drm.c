/**
 * \file xf86drm.c
 * User-level interface to DRM device
 *
 * \author Rickard E. (Rik) Faith <faith@valinux.com>
 * \author Kevin E. Martin <martin@valinux.com>
 */

/*
 * Copyright 1999 Precision Insight, Inc., Cedar Park, Texas.
 * Copyright 2000 VA Linux Systems, Inc., Sunnyvale, California.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <dirent.h>
#include <stddef.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdarg.h>

/* Not all systems have MAP_FAILED defined */
#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif

#include "xf86drm.h"
#include "libdrm_macros.h"

#include <kos32sys.h>

#ifndef DRM_MAJOR
#define DRM_MAJOR 226		/* Linux */
#endif


int drmGetMagic(int fd, drm_magic_t * magic)
{
    drm_auth_t auth;

    *magic = 1;
//    if (drmIoctl(fd, DRM_IOCTL_GET_MAGIC, &auth))
//    return -errno;
//    *magic = auth.magic;
    return 0;
}

void drmFreeVersion(drmVersionPtr v)
{
    if (!v)
        return;
//    drmFree(v->name);
//    drmFree(v->date);
//    drmFree(v->desc);
    free(v);
}
drmVersionPtr drmGetVersion(int fd)
{
    drmVersionPtr v;

    v = malloc(sizeof(*v));

    v->version_major      = 1;
    v->version_minor      = 6;
    v->version_patchlevel = 0;
    v->name_len           = 4;
    v->name               = "i915";
    v->date_len           = 8;
    v->date               = "20151010";
    v->desc_len           = 14;
    v->desc               = "Intel Graphics";
    return v;
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
