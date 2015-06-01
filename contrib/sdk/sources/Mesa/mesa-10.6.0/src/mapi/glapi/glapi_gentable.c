/* DO NOT EDIT - This file generated automatically by gl_gentable.py (from Mesa) script */

/*
 * Copyright (C) 1999-2001  Brian Paul   All Rights Reserved.
 * (C) Copyright IBM Corporation 2004, 2005
 * (C) Copyright Apple Inc 2011
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL, IBM,
 * AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* GLXEXT is the define used in the xserver when the GLX extension is being
 * built.  Hijack this to determine whether this file is being built for the
 * server or the client.
 */
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#if (defined(GLXEXT) && defined(HAVE_BACKTRACE)) \
	|| (!defined(GLXEXT) && defined(DEBUG) && !defined(__CYGWIN__) && !defined(__MINGW32__) && !defined(__OpenBSD__) && !defined(__NetBSD__) && !defined(__DragonFly__))
#define USE_BACKTRACE
#endif

#ifdef USE_BACKTRACE
#include <execinfo.h>
#endif

#ifndef _WIN32
#include <dlfcn.h>
#endif
#include <stdlib.h>
#include <stdio.h>

#include "main/glheader.h"

#include "glapi.h"
#include "glapitable.h"

#ifdef GLXEXT
#include "os.h"
#endif

static void
__glapi_gentable_NoOp(void) {
    const char *fstr = "Unknown";

    /* Silence potential GCC warning for some #ifdef paths.
     */
    (void) fstr;
#if defined(USE_BACKTRACE)
#if !defined(GLXEXT)
    if (getenv("MESA_DEBUG") || getenv("LIBGL_DEBUG"))
#endif
    {
        void *frames[2];

        if(backtrace(frames, 2) == 2) {
            Dl_info info;
            dladdr(frames[1], &info);
            if(info.dli_sname)
                fstr = info.dli_sname;
        }

#if !defined(GLXEXT)
        fprintf(stderr, "Call to unimplemented API: %s\n", fstr);
#endif
    }
#endif
#if defined(GLXEXT)
    LogMessage(X_ERROR, "GLX: Call to unimplemented API: %s\n", fstr);
#endif
}

static void
__glapi_gentable_set_remaining_noop(struct _glapi_table *disp) {
    GLuint entries = _glapi_get_dispatch_table_size();
    void **dispatch = (void **) disp;
    unsigned i;

    /* ISO C is annoying sometimes */
    union {_glapi_proc p; void *v;} p;
    p.p = __glapi_gentable_NoOp;

    for(i=0; i < entries; i++)
        if(dispatch[i] == NULL)
            dispatch[i] = p.v;
}

struct _glapi_table *
_glapi_create_table_from_handle(void *handle, const char *symbol_prefix) {
    struct _glapi_table *disp = calloc(_glapi_get_dispatch_table_size(), sizeof(_glapi_proc));
    char symboln[512];

    if(!disp)
        return NULL;

    if(symbol_prefix == NULL)
        symbol_prefix = "";


    if(!disp->NewList) {
        void ** procp = (void **) &disp->NewList;
        snprintf(symboln, sizeof(symboln), "%sNewList", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EndList) {
        void ** procp = (void **) &disp->EndList;
        snprintf(symboln, sizeof(symboln), "%sEndList", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CallList) {
        void ** procp = (void **) &disp->CallList;
        snprintf(symboln, sizeof(symboln), "%sCallList", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CallLists) {
        void ** procp = (void **) &disp->CallLists;
        snprintf(symboln, sizeof(symboln), "%sCallLists", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteLists) {
        void ** procp = (void **) &disp->DeleteLists;
        snprintf(symboln, sizeof(symboln), "%sDeleteLists", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenLists) {
        void ** procp = (void **) &disp->GenLists;
        snprintf(symboln, sizeof(symboln), "%sGenLists", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ListBase) {
        void ** procp = (void **) &disp->ListBase;
        snprintf(symboln, sizeof(symboln), "%sListBase", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Begin) {
        void ** procp = (void **) &disp->Begin;
        snprintf(symboln, sizeof(symboln), "%sBegin", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Bitmap) {
        void ** procp = (void **) &disp->Bitmap;
        snprintf(symboln, sizeof(symboln), "%sBitmap", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color3b) {
        void ** procp = (void **) &disp->Color3b;
        snprintf(symboln, sizeof(symboln), "%sColor3b", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color3bv) {
        void ** procp = (void **) &disp->Color3bv;
        snprintf(symboln, sizeof(symboln), "%sColor3bv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color3d) {
        void ** procp = (void **) &disp->Color3d;
        snprintf(symboln, sizeof(symboln), "%sColor3d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color3dv) {
        void ** procp = (void **) &disp->Color3dv;
        snprintf(symboln, sizeof(symboln), "%sColor3dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color3f) {
        void ** procp = (void **) &disp->Color3f;
        snprintf(symboln, sizeof(symboln), "%sColor3f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color3fv) {
        void ** procp = (void **) &disp->Color3fv;
        snprintf(symboln, sizeof(symboln), "%sColor3fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color3i) {
        void ** procp = (void **) &disp->Color3i;
        snprintf(symboln, sizeof(symboln), "%sColor3i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color3iv) {
        void ** procp = (void **) &disp->Color3iv;
        snprintf(symboln, sizeof(symboln), "%sColor3iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color3s) {
        void ** procp = (void **) &disp->Color3s;
        snprintf(symboln, sizeof(symboln), "%sColor3s", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color3sv) {
        void ** procp = (void **) &disp->Color3sv;
        snprintf(symboln, sizeof(symboln), "%sColor3sv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color3ub) {
        void ** procp = (void **) &disp->Color3ub;
        snprintf(symboln, sizeof(symboln), "%sColor3ub", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color3ubv) {
        void ** procp = (void **) &disp->Color3ubv;
        snprintf(symboln, sizeof(symboln), "%sColor3ubv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color3ui) {
        void ** procp = (void **) &disp->Color3ui;
        snprintf(symboln, sizeof(symboln), "%sColor3ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color3uiv) {
        void ** procp = (void **) &disp->Color3uiv;
        snprintf(symboln, sizeof(symboln), "%sColor3uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color3us) {
        void ** procp = (void **) &disp->Color3us;
        snprintf(symboln, sizeof(symboln), "%sColor3us", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color3usv) {
        void ** procp = (void **) &disp->Color3usv;
        snprintf(symboln, sizeof(symboln), "%sColor3usv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color4b) {
        void ** procp = (void **) &disp->Color4b;
        snprintf(symboln, sizeof(symboln), "%sColor4b", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color4bv) {
        void ** procp = (void **) &disp->Color4bv;
        snprintf(symboln, sizeof(symboln), "%sColor4bv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color4d) {
        void ** procp = (void **) &disp->Color4d;
        snprintf(symboln, sizeof(symboln), "%sColor4d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color4dv) {
        void ** procp = (void **) &disp->Color4dv;
        snprintf(symboln, sizeof(symboln), "%sColor4dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color4f) {
        void ** procp = (void **) &disp->Color4f;
        snprintf(symboln, sizeof(symboln), "%sColor4f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color4fv) {
        void ** procp = (void **) &disp->Color4fv;
        snprintf(symboln, sizeof(symboln), "%sColor4fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color4i) {
        void ** procp = (void **) &disp->Color4i;
        snprintf(symboln, sizeof(symboln), "%sColor4i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color4iv) {
        void ** procp = (void **) &disp->Color4iv;
        snprintf(symboln, sizeof(symboln), "%sColor4iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color4s) {
        void ** procp = (void **) &disp->Color4s;
        snprintf(symboln, sizeof(symboln), "%sColor4s", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color4sv) {
        void ** procp = (void **) &disp->Color4sv;
        snprintf(symboln, sizeof(symboln), "%sColor4sv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color4ub) {
        void ** procp = (void **) &disp->Color4ub;
        snprintf(symboln, sizeof(symboln), "%sColor4ub", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color4ubv) {
        void ** procp = (void **) &disp->Color4ubv;
        snprintf(symboln, sizeof(symboln), "%sColor4ubv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color4ui) {
        void ** procp = (void **) &disp->Color4ui;
        snprintf(symboln, sizeof(symboln), "%sColor4ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color4uiv) {
        void ** procp = (void **) &disp->Color4uiv;
        snprintf(symboln, sizeof(symboln), "%sColor4uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color4us) {
        void ** procp = (void **) &disp->Color4us;
        snprintf(symboln, sizeof(symboln), "%sColor4us", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color4usv) {
        void ** procp = (void **) &disp->Color4usv;
        snprintf(symboln, sizeof(symboln), "%sColor4usv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EdgeFlag) {
        void ** procp = (void **) &disp->EdgeFlag;
        snprintf(symboln, sizeof(symboln), "%sEdgeFlag", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EdgeFlagv) {
        void ** procp = (void **) &disp->EdgeFlagv;
        snprintf(symboln, sizeof(symboln), "%sEdgeFlagv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->End) {
        void ** procp = (void **) &disp->End;
        snprintf(symboln, sizeof(symboln), "%sEnd", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Indexd) {
        void ** procp = (void **) &disp->Indexd;
        snprintf(symboln, sizeof(symboln), "%sIndexd", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Indexdv) {
        void ** procp = (void **) &disp->Indexdv;
        snprintf(symboln, sizeof(symboln), "%sIndexdv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Indexf) {
        void ** procp = (void **) &disp->Indexf;
        snprintf(symboln, sizeof(symboln), "%sIndexf", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Indexfv) {
        void ** procp = (void **) &disp->Indexfv;
        snprintf(symboln, sizeof(symboln), "%sIndexfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Indexi) {
        void ** procp = (void **) &disp->Indexi;
        snprintf(symboln, sizeof(symboln), "%sIndexi", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Indexiv) {
        void ** procp = (void **) &disp->Indexiv;
        snprintf(symboln, sizeof(symboln), "%sIndexiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Indexs) {
        void ** procp = (void **) &disp->Indexs;
        snprintf(symboln, sizeof(symboln), "%sIndexs", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Indexsv) {
        void ** procp = (void **) &disp->Indexsv;
        snprintf(symboln, sizeof(symboln), "%sIndexsv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Normal3b) {
        void ** procp = (void **) &disp->Normal3b;
        snprintf(symboln, sizeof(symboln), "%sNormal3b", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Normal3bv) {
        void ** procp = (void **) &disp->Normal3bv;
        snprintf(symboln, sizeof(symboln), "%sNormal3bv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Normal3d) {
        void ** procp = (void **) &disp->Normal3d;
        snprintf(symboln, sizeof(symboln), "%sNormal3d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Normal3dv) {
        void ** procp = (void **) &disp->Normal3dv;
        snprintf(symboln, sizeof(symboln), "%sNormal3dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Normal3f) {
        void ** procp = (void **) &disp->Normal3f;
        snprintf(symboln, sizeof(symboln), "%sNormal3f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Normal3fv) {
        void ** procp = (void **) &disp->Normal3fv;
        snprintf(symboln, sizeof(symboln), "%sNormal3fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Normal3i) {
        void ** procp = (void **) &disp->Normal3i;
        snprintf(symboln, sizeof(symboln), "%sNormal3i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Normal3iv) {
        void ** procp = (void **) &disp->Normal3iv;
        snprintf(symboln, sizeof(symboln), "%sNormal3iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Normal3s) {
        void ** procp = (void **) &disp->Normal3s;
        snprintf(symboln, sizeof(symboln), "%sNormal3s", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Normal3sv) {
        void ** procp = (void **) &disp->Normal3sv;
        snprintf(symboln, sizeof(symboln), "%sNormal3sv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RasterPos2d) {
        void ** procp = (void **) &disp->RasterPos2d;
        snprintf(symboln, sizeof(symboln), "%sRasterPos2d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RasterPos2dv) {
        void ** procp = (void **) &disp->RasterPos2dv;
        snprintf(symboln, sizeof(symboln), "%sRasterPos2dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RasterPos2f) {
        void ** procp = (void **) &disp->RasterPos2f;
        snprintf(symboln, sizeof(symboln), "%sRasterPos2f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RasterPos2fv) {
        void ** procp = (void **) &disp->RasterPos2fv;
        snprintf(symboln, sizeof(symboln), "%sRasterPos2fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RasterPos2i) {
        void ** procp = (void **) &disp->RasterPos2i;
        snprintf(symboln, sizeof(symboln), "%sRasterPos2i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RasterPos2iv) {
        void ** procp = (void **) &disp->RasterPos2iv;
        snprintf(symboln, sizeof(symboln), "%sRasterPos2iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RasterPos2s) {
        void ** procp = (void **) &disp->RasterPos2s;
        snprintf(symboln, sizeof(symboln), "%sRasterPos2s", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RasterPos2sv) {
        void ** procp = (void **) &disp->RasterPos2sv;
        snprintf(symboln, sizeof(symboln), "%sRasterPos2sv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RasterPos3d) {
        void ** procp = (void **) &disp->RasterPos3d;
        snprintf(symboln, sizeof(symboln), "%sRasterPos3d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RasterPos3dv) {
        void ** procp = (void **) &disp->RasterPos3dv;
        snprintf(symboln, sizeof(symboln), "%sRasterPos3dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RasterPos3f) {
        void ** procp = (void **) &disp->RasterPos3f;
        snprintf(symboln, sizeof(symboln), "%sRasterPos3f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RasterPos3fv) {
        void ** procp = (void **) &disp->RasterPos3fv;
        snprintf(symboln, sizeof(symboln), "%sRasterPos3fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RasterPos3i) {
        void ** procp = (void **) &disp->RasterPos3i;
        snprintf(symboln, sizeof(symboln), "%sRasterPos3i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RasterPos3iv) {
        void ** procp = (void **) &disp->RasterPos3iv;
        snprintf(symboln, sizeof(symboln), "%sRasterPos3iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RasterPos3s) {
        void ** procp = (void **) &disp->RasterPos3s;
        snprintf(symboln, sizeof(symboln), "%sRasterPos3s", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RasterPos3sv) {
        void ** procp = (void **) &disp->RasterPos3sv;
        snprintf(symboln, sizeof(symboln), "%sRasterPos3sv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RasterPos4d) {
        void ** procp = (void **) &disp->RasterPos4d;
        snprintf(symboln, sizeof(symboln), "%sRasterPos4d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RasterPos4dv) {
        void ** procp = (void **) &disp->RasterPos4dv;
        snprintf(symboln, sizeof(symboln), "%sRasterPos4dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RasterPos4f) {
        void ** procp = (void **) &disp->RasterPos4f;
        snprintf(symboln, sizeof(symboln), "%sRasterPos4f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RasterPos4fv) {
        void ** procp = (void **) &disp->RasterPos4fv;
        snprintf(symboln, sizeof(symboln), "%sRasterPos4fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RasterPos4i) {
        void ** procp = (void **) &disp->RasterPos4i;
        snprintf(symboln, sizeof(symboln), "%sRasterPos4i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RasterPos4iv) {
        void ** procp = (void **) &disp->RasterPos4iv;
        snprintf(symboln, sizeof(symboln), "%sRasterPos4iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RasterPos4s) {
        void ** procp = (void **) &disp->RasterPos4s;
        snprintf(symboln, sizeof(symboln), "%sRasterPos4s", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RasterPos4sv) {
        void ** procp = (void **) &disp->RasterPos4sv;
        snprintf(symboln, sizeof(symboln), "%sRasterPos4sv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Rectd) {
        void ** procp = (void **) &disp->Rectd;
        snprintf(symboln, sizeof(symboln), "%sRectd", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Rectdv) {
        void ** procp = (void **) &disp->Rectdv;
        snprintf(symboln, sizeof(symboln), "%sRectdv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Rectf) {
        void ** procp = (void **) &disp->Rectf;
        snprintf(symboln, sizeof(symboln), "%sRectf", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Rectfv) {
        void ** procp = (void **) &disp->Rectfv;
        snprintf(symboln, sizeof(symboln), "%sRectfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Recti) {
        void ** procp = (void **) &disp->Recti;
        snprintf(symboln, sizeof(symboln), "%sRecti", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Rectiv) {
        void ** procp = (void **) &disp->Rectiv;
        snprintf(symboln, sizeof(symboln), "%sRectiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Rects) {
        void ** procp = (void **) &disp->Rects;
        snprintf(symboln, sizeof(symboln), "%sRects", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Rectsv) {
        void ** procp = (void **) &disp->Rectsv;
        snprintf(symboln, sizeof(symboln), "%sRectsv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord1d) {
        void ** procp = (void **) &disp->TexCoord1d;
        snprintf(symboln, sizeof(symboln), "%sTexCoord1d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord1dv) {
        void ** procp = (void **) &disp->TexCoord1dv;
        snprintf(symboln, sizeof(symboln), "%sTexCoord1dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord1f) {
        void ** procp = (void **) &disp->TexCoord1f;
        snprintf(symboln, sizeof(symboln), "%sTexCoord1f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord1fv) {
        void ** procp = (void **) &disp->TexCoord1fv;
        snprintf(symboln, sizeof(symboln), "%sTexCoord1fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord1i) {
        void ** procp = (void **) &disp->TexCoord1i;
        snprintf(symboln, sizeof(symboln), "%sTexCoord1i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord1iv) {
        void ** procp = (void **) &disp->TexCoord1iv;
        snprintf(symboln, sizeof(symboln), "%sTexCoord1iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord1s) {
        void ** procp = (void **) &disp->TexCoord1s;
        snprintf(symboln, sizeof(symboln), "%sTexCoord1s", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord1sv) {
        void ** procp = (void **) &disp->TexCoord1sv;
        snprintf(symboln, sizeof(symboln), "%sTexCoord1sv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord2d) {
        void ** procp = (void **) &disp->TexCoord2d;
        snprintf(symboln, sizeof(symboln), "%sTexCoord2d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord2dv) {
        void ** procp = (void **) &disp->TexCoord2dv;
        snprintf(symboln, sizeof(symboln), "%sTexCoord2dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord2f) {
        void ** procp = (void **) &disp->TexCoord2f;
        snprintf(symboln, sizeof(symboln), "%sTexCoord2f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord2fv) {
        void ** procp = (void **) &disp->TexCoord2fv;
        snprintf(symboln, sizeof(symboln), "%sTexCoord2fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord2i) {
        void ** procp = (void **) &disp->TexCoord2i;
        snprintf(symboln, sizeof(symboln), "%sTexCoord2i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord2iv) {
        void ** procp = (void **) &disp->TexCoord2iv;
        snprintf(symboln, sizeof(symboln), "%sTexCoord2iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord2s) {
        void ** procp = (void **) &disp->TexCoord2s;
        snprintf(symboln, sizeof(symboln), "%sTexCoord2s", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord2sv) {
        void ** procp = (void **) &disp->TexCoord2sv;
        snprintf(symboln, sizeof(symboln), "%sTexCoord2sv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord3d) {
        void ** procp = (void **) &disp->TexCoord3d;
        snprintf(symboln, sizeof(symboln), "%sTexCoord3d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord3dv) {
        void ** procp = (void **) &disp->TexCoord3dv;
        snprintf(symboln, sizeof(symboln), "%sTexCoord3dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord3f) {
        void ** procp = (void **) &disp->TexCoord3f;
        snprintf(symboln, sizeof(symboln), "%sTexCoord3f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord3fv) {
        void ** procp = (void **) &disp->TexCoord3fv;
        snprintf(symboln, sizeof(symboln), "%sTexCoord3fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord3i) {
        void ** procp = (void **) &disp->TexCoord3i;
        snprintf(symboln, sizeof(symboln), "%sTexCoord3i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord3iv) {
        void ** procp = (void **) &disp->TexCoord3iv;
        snprintf(symboln, sizeof(symboln), "%sTexCoord3iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord3s) {
        void ** procp = (void **) &disp->TexCoord3s;
        snprintf(symboln, sizeof(symboln), "%sTexCoord3s", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord3sv) {
        void ** procp = (void **) &disp->TexCoord3sv;
        snprintf(symboln, sizeof(symboln), "%sTexCoord3sv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord4d) {
        void ** procp = (void **) &disp->TexCoord4d;
        snprintf(symboln, sizeof(symboln), "%sTexCoord4d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord4dv) {
        void ** procp = (void **) &disp->TexCoord4dv;
        snprintf(symboln, sizeof(symboln), "%sTexCoord4dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord4f) {
        void ** procp = (void **) &disp->TexCoord4f;
        snprintf(symboln, sizeof(symboln), "%sTexCoord4f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord4fv) {
        void ** procp = (void **) &disp->TexCoord4fv;
        snprintf(symboln, sizeof(symboln), "%sTexCoord4fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord4i) {
        void ** procp = (void **) &disp->TexCoord4i;
        snprintf(symboln, sizeof(symboln), "%sTexCoord4i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord4iv) {
        void ** procp = (void **) &disp->TexCoord4iv;
        snprintf(symboln, sizeof(symboln), "%sTexCoord4iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord4s) {
        void ** procp = (void **) &disp->TexCoord4s;
        snprintf(symboln, sizeof(symboln), "%sTexCoord4s", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoord4sv) {
        void ** procp = (void **) &disp->TexCoord4sv;
        snprintf(symboln, sizeof(symboln), "%sTexCoord4sv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Vertex2d) {
        void ** procp = (void **) &disp->Vertex2d;
        snprintf(symboln, sizeof(symboln), "%sVertex2d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Vertex2dv) {
        void ** procp = (void **) &disp->Vertex2dv;
        snprintf(symboln, sizeof(symboln), "%sVertex2dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Vertex2f) {
        void ** procp = (void **) &disp->Vertex2f;
        snprintf(symboln, sizeof(symboln), "%sVertex2f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Vertex2fv) {
        void ** procp = (void **) &disp->Vertex2fv;
        snprintf(symboln, sizeof(symboln), "%sVertex2fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Vertex2i) {
        void ** procp = (void **) &disp->Vertex2i;
        snprintf(symboln, sizeof(symboln), "%sVertex2i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Vertex2iv) {
        void ** procp = (void **) &disp->Vertex2iv;
        snprintf(symboln, sizeof(symboln), "%sVertex2iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Vertex2s) {
        void ** procp = (void **) &disp->Vertex2s;
        snprintf(symboln, sizeof(symboln), "%sVertex2s", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Vertex2sv) {
        void ** procp = (void **) &disp->Vertex2sv;
        snprintf(symboln, sizeof(symboln), "%sVertex2sv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Vertex3d) {
        void ** procp = (void **) &disp->Vertex3d;
        snprintf(symboln, sizeof(symboln), "%sVertex3d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Vertex3dv) {
        void ** procp = (void **) &disp->Vertex3dv;
        snprintf(symboln, sizeof(symboln), "%sVertex3dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Vertex3f) {
        void ** procp = (void **) &disp->Vertex3f;
        snprintf(symboln, sizeof(symboln), "%sVertex3f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Vertex3fv) {
        void ** procp = (void **) &disp->Vertex3fv;
        snprintf(symboln, sizeof(symboln), "%sVertex3fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Vertex3i) {
        void ** procp = (void **) &disp->Vertex3i;
        snprintf(symboln, sizeof(symboln), "%sVertex3i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Vertex3iv) {
        void ** procp = (void **) &disp->Vertex3iv;
        snprintf(symboln, sizeof(symboln), "%sVertex3iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Vertex3s) {
        void ** procp = (void **) &disp->Vertex3s;
        snprintf(symboln, sizeof(symboln), "%sVertex3s", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Vertex3sv) {
        void ** procp = (void **) &disp->Vertex3sv;
        snprintf(symboln, sizeof(symboln), "%sVertex3sv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Vertex4d) {
        void ** procp = (void **) &disp->Vertex4d;
        snprintf(symboln, sizeof(symboln), "%sVertex4d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Vertex4dv) {
        void ** procp = (void **) &disp->Vertex4dv;
        snprintf(symboln, sizeof(symboln), "%sVertex4dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Vertex4f) {
        void ** procp = (void **) &disp->Vertex4f;
        snprintf(symboln, sizeof(symboln), "%sVertex4f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Vertex4fv) {
        void ** procp = (void **) &disp->Vertex4fv;
        snprintf(symboln, sizeof(symboln), "%sVertex4fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Vertex4i) {
        void ** procp = (void **) &disp->Vertex4i;
        snprintf(symboln, sizeof(symboln), "%sVertex4i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Vertex4iv) {
        void ** procp = (void **) &disp->Vertex4iv;
        snprintf(symboln, sizeof(symboln), "%sVertex4iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Vertex4s) {
        void ** procp = (void **) &disp->Vertex4s;
        snprintf(symboln, sizeof(symboln), "%sVertex4s", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Vertex4sv) {
        void ** procp = (void **) &disp->Vertex4sv;
        snprintf(symboln, sizeof(symboln), "%sVertex4sv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClipPlane) {
        void ** procp = (void **) &disp->ClipPlane;
        snprintf(symboln, sizeof(symboln), "%sClipPlane", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ColorMaterial) {
        void ** procp = (void **) &disp->ColorMaterial;
        snprintf(symboln, sizeof(symboln), "%sColorMaterial", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CullFace) {
        void ** procp = (void **) &disp->CullFace;
        snprintf(symboln, sizeof(symboln), "%sCullFace", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Fogf) {
        void ** procp = (void **) &disp->Fogf;
        snprintf(symboln, sizeof(symboln), "%sFogf", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Fogfv) {
        void ** procp = (void **) &disp->Fogfv;
        snprintf(symboln, sizeof(symboln), "%sFogfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Fogi) {
        void ** procp = (void **) &disp->Fogi;
        snprintf(symboln, sizeof(symboln), "%sFogi", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Fogiv) {
        void ** procp = (void **) &disp->Fogiv;
        snprintf(symboln, sizeof(symboln), "%sFogiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FrontFace) {
        void ** procp = (void **) &disp->FrontFace;
        snprintf(symboln, sizeof(symboln), "%sFrontFace", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Hint) {
        void ** procp = (void **) &disp->Hint;
        snprintf(symboln, sizeof(symboln), "%sHint", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Lightf) {
        void ** procp = (void **) &disp->Lightf;
        snprintf(symboln, sizeof(symboln), "%sLightf", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Lightfv) {
        void ** procp = (void **) &disp->Lightfv;
        snprintf(symboln, sizeof(symboln), "%sLightfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Lighti) {
        void ** procp = (void **) &disp->Lighti;
        snprintf(symboln, sizeof(symboln), "%sLighti", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Lightiv) {
        void ** procp = (void **) &disp->Lightiv;
        snprintf(symboln, sizeof(symboln), "%sLightiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LightModelf) {
        void ** procp = (void **) &disp->LightModelf;
        snprintf(symboln, sizeof(symboln), "%sLightModelf", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LightModelfv) {
        void ** procp = (void **) &disp->LightModelfv;
        snprintf(symboln, sizeof(symboln), "%sLightModelfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LightModeli) {
        void ** procp = (void **) &disp->LightModeli;
        snprintf(symboln, sizeof(symboln), "%sLightModeli", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LightModeliv) {
        void ** procp = (void **) &disp->LightModeliv;
        snprintf(symboln, sizeof(symboln), "%sLightModeliv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LineStipple) {
        void ** procp = (void **) &disp->LineStipple;
        snprintf(symboln, sizeof(symboln), "%sLineStipple", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LineWidth) {
        void ** procp = (void **) &disp->LineWidth;
        snprintf(symboln, sizeof(symboln), "%sLineWidth", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Materialf) {
        void ** procp = (void **) &disp->Materialf;
        snprintf(symboln, sizeof(symboln), "%sMaterialf", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Materialfv) {
        void ** procp = (void **) &disp->Materialfv;
        snprintf(symboln, sizeof(symboln), "%sMaterialfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Materiali) {
        void ** procp = (void **) &disp->Materiali;
        snprintf(symboln, sizeof(symboln), "%sMateriali", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Materialiv) {
        void ** procp = (void **) &disp->Materialiv;
        snprintf(symboln, sizeof(symboln), "%sMaterialiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PointSize) {
        void ** procp = (void **) &disp->PointSize;
        snprintf(symboln, sizeof(symboln), "%sPointSize", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PolygonMode) {
        void ** procp = (void **) &disp->PolygonMode;
        snprintf(symboln, sizeof(symboln), "%sPolygonMode", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PolygonStipple) {
        void ** procp = (void **) &disp->PolygonStipple;
        snprintf(symboln, sizeof(symboln), "%sPolygonStipple", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Scissor) {
        void ** procp = (void **) &disp->Scissor;
        snprintf(symboln, sizeof(symboln), "%sScissor", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ShadeModel) {
        void ** procp = (void **) &disp->ShadeModel;
        snprintf(symboln, sizeof(symboln), "%sShadeModel", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexParameterf) {
        void ** procp = (void **) &disp->TexParameterf;
        snprintf(symboln, sizeof(symboln), "%sTexParameterf", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexParameterfv) {
        void ** procp = (void **) &disp->TexParameterfv;
        snprintf(symboln, sizeof(symboln), "%sTexParameterfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexParameteri) {
        void ** procp = (void **) &disp->TexParameteri;
        snprintf(symboln, sizeof(symboln), "%sTexParameteri", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexParameteriv) {
        void ** procp = (void **) &disp->TexParameteriv;
        snprintf(symboln, sizeof(symboln), "%sTexParameteriv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexImage1D) {
        void ** procp = (void **) &disp->TexImage1D;
        snprintf(symboln, sizeof(symboln), "%sTexImage1D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexImage2D) {
        void ** procp = (void **) &disp->TexImage2D;
        snprintf(symboln, sizeof(symboln), "%sTexImage2D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexEnvf) {
        void ** procp = (void **) &disp->TexEnvf;
        snprintf(symboln, sizeof(symboln), "%sTexEnvf", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexEnvfv) {
        void ** procp = (void **) &disp->TexEnvfv;
        snprintf(symboln, sizeof(symboln), "%sTexEnvfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexEnvi) {
        void ** procp = (void **) &disp->TexEnvi;
        snprintf(symboln, sizeof(symboln), "%sTexEnvi", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexEnviv) {
        void ** procp = (void **) &disp->TexEnviv;
        snprintf(symboln, sizeof(symboln), "%sTexEnviv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexGend) {
        void ** procp = (void **) &disp->TexGend;
        snprintf(symboln, sizeof(symboln), "%sTexGend", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexGendv) {
        void ** procp = (void **) &disp->TexGendv;
        snprintf(symboln, sizeof(symboln), "%sTexGendv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexGenf) {
        void ** procp = (void **) &disp->TexGenf;
        snprintf(symboln, sizeof(symboln), "%sTexGenf", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexGenf) {
        void ** procp = (void **) &disp->TexGenf;
        snprintf(symboln, sizeof(symboln), "%sTexGenfOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexGenfv) {
        void ** procp = (void **) &disp->TexGenfv;
        snprintf(symboln, sizeof(symboln), "%sTexGenfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexGenfv) {
        void ** procp = (void **) &disp->TexGenfv;
        snprintf(symboln, sizeof(symboln), "%sTexGenfvOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexGeni) {
        void ** procp = (void **) &disp->TexGeni;
        snprintf(symboln, sizeof(symboln), "%sTexGeni", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexGeni) {
        void ** procp = (void **) &disp->TexGeni;
        snprintf(symboln, sizeof(symboln), "%sTexGeniOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexGeniv) {
        void ** procp = (void **) &disp->TexGeniv;
        snprintf(symboln, sizeof(symboln), "%sTexGeniv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexGeniv) {
        void ** procp = (void **) &disp->TexGeniv;
        snprintf(symboln, sizeof(symboln), "%sTexGenivOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FeedbackBuffer) {
        void ** procp = (void **) &disp->FeedbackBuffer;
        snprintf(symboln, sizeof(symboln), "%sFeedbackBuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SelectBuffer) {
        void ** procp = (void **) &disp->SelectBuffer;
        snprintf(symboln, sizeof(symboln), "%sSelectBuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RenderMode) {
        void ** procp = (void **) &disp->RenderMode;
        snprintf(symboln, sizeof(symboln), "%sRenderMode", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->InitNames) {
        void ** procp = (void **) &disp->InitNames;
        snprintf(symboln, sizeof(symboln), "%sInitNames", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LoadName) {
        void ** procp = (void **) &disp->LoadName;
        snprintf(symboln, sizeof(symboln), "%sLoadName", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PassThrough) {
        void ** procp = (void **) &disp->PassThrough;
        snprintf(symboln, sizeof(symboln), "%sPassThrough", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PopName) {
        void ** procp = (void **) &disp->PopName;
        snprintf(symboln, sizeof(symboln), "%sPopName", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PushName) {
        void ** procp = (void **) &disp->PushName;
        snprintf(symboln, sizeof(symboln), "%sPushName", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawBuffer) {
        void ** procp = (void **) &disp->DrawBuffer;
        snprintf(symboln, sizeof(symboln), "%sDrawBuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Clear) {
        void ** procp = (void **) &disp->Clear;
        snprintf(symboln, sizeof(symboln), "%sClear", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearAccum) {
        void ** procp = (void **) &disp->ClearAccum;
        snprintf(symboln, sizeof(symboln), "%sClearAccum", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearIndex) {
        void ** procp = (void **) &disp->ClearIndex;
        snprintf(symboln, sizeof(symboln), "%sClearIndex", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearColor) {
        void ** procp = (void **) &disp->ClearColor;
        snprintf(symboln, sizeof(symboln), "%sClearColor", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearStencil) {
        void ** procp = (void **) &disp->ClearStencil;
        snprintf(symboln, sizeof(symboln), "%sClearStencil", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearDepth) {
        void ** procp = (void **) &disp->ClearDepth;
        snprintf(symboln, sizeof(symboln), "%sClearDepth", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->StencilMask) {
        void ** procp = (void **) &disp->StencilMask;
        snprintf(symboln, sizeof(symboln), "%sStencilMask", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ColorMask) {
        void ** procp = (void **) &disp->ColorMask;
        snprintf(symboln, sizeof(symboln), "%sColorMask", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DepthMask) {
        void ** procp = (void **) &disp->DepthMask;
        snprintf(symboln, sizeof(symboln), "%sDepthMask", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IndexMask) {
        void ** procp = (void **) &disp->IndexMask;
        snprintf(symboln, sizeof(symboln), "%sIndexMask", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Accum) {
        void ** procp = (void **) &disp->Accum;
        snprintf(symboln, sizeof(symboln), "%sAccum", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Disable) {
        void ** procp = (void **) &disp->Disable;
        snprintf(symboln, sizeof(symboln), "%sDisable", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Enable) {
        void ** procp = (void **) &disp->Enable;
        snprintf(symboln, sizeof(symboln), "%sEnable", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Finish) {
        void ** procp = (void **) &disp->Finish;
        snprintf(symboln, sizeof(symboln), "%sFinish", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Flush) {
        void ** procp = (void **) &disp->Flush;
        snprintf(symboln, sizeof(symboln), "%sFlush", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PopAttrib) {
        void ** procp = (void **) &disp->PopAttrib;
        snprintf(symboln, sizeof(symboln), "%sPopAttrib", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PushAttrib) {
        void ** procp = (void **) &disp->PushAttrib;
        snprintf(symboln, sizeof(symboln), "%sPushAttrib", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Map1d) {
        void ** procp = (void **) &disp->Map1d;
        snprintf(symboln, sizeof(symboln), "%sMap1d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Map1f) {
        void ** procp = (void **) &disp->Map1f;
        snprintf(symboln, sizeof(symboln), "%sMap1f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Map2d) {
        void ** procp = (void **) &disp->Map2d;
        snprintf(symboln, sizeof(symboln), "%sMap2d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Map2f) {
        void ** procp = (void **) &disp->Map2f;
        snprintf(symboln, sizeof(symboln), "%sMap2f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MapGrid1d) {
        void ** procp = (void **) &disp->MapGrid1d;
        snprintf(symboln, sizeof(symboln), "%sMapGrid1d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MapGrid1f) {
        void ** procp = (void **) &disp->MapGrid1f;
        snprintf(symboln, sizeof(symboln), "%sMapGrid1f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MapGrid2d) {
        void ** procp = (void **) &disp->MapGrid2d;
        snprintf(symboln, sizeof(symboln), "%sMapGrid2d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MapGrid2f) {
        void ** procp = (void **) &disp->MapGrid2f;
        snprintf(symboln, sizeof(symboln), "%sMapGrid2f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EvalCoord1d) {
        void ** procp = (void **) &disp->EvalCoord1d;
        snprintf(symboln, sizeof(symboln), "%sEvalCoord1d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EvalCoord1dv) {
        void ** procp = (void **) &disp->EvalCoord1dv;
        snprintf(symboln, sizeof(symboln), "%sEvalCoord1dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EvalCoord1f) {
        void ** procp = (void **) &disp->EvalCoord1f;
        snprintf(symboln, sizeof(symboln), "%sEvalCoord1f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EvalCoord1fv) {
        void ** procp = (void **) &disp->EvalCoord1fv;
        snprintf(symboln, sizeof(symboln), "%sEvalCoord1fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EvalCoord2d) {
        void ** procp = (void **) &disp->EvalCoord2d;
        snprintf(symboln, sizeof(symboln), "%sEvalCoord2d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EvalCoord2dv) {
        void ** procp = (void **) &disp->EvalCoord2dv;
        snprintf(symboln, sizeof(symboln), "%sEvalCoord2dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EvalCoord2f) {
        void ** procp = (void **) &disp->EvalCoord2f;
        snprintf(symboln, sizeof(symboln), "%sEvalCoord2f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EvalCoord2fv) {
        void ** procp = (void **) &disp->EvalCoord2fv;
        snprintf(symboln, sizeof(symboln), "%sEvalCoord2fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EvalMesh1) {
        void ** procp = (void **) &disp->EvalMesh1;
        snprintf(symboln, sizeof(symboln), "%sEvalMesh1", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EvalPoint1) {
        void ** procp = (void **) &disp->EvalPoint1;
        snprintf(symboln, sizeof(symboln), "%sEvalPoint1", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EvalMesh2) {
        void ** procp = (void **) &disp->EvalMesh2;
        snprintf(symboln, sizeof(symboln), "%sEvalMesh2", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EvalPoint2) {
        void ** procp = (void **) &disp->EvalPoint2;
        snprintf(symboln, sizeof(symboln), "%sEvalPoint2", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->AlphaFunc) {
        void ** procp = (void **) &disp->AlphaFunc;
        snprintf(symboln, sizeof(symboln), "%sAlphaFunc", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlendFunc) {
        void ** procp = (void **) &disp->BlendFunc;
        snprintf(symboln, sizeof(symboln), "%sBlendFunc", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LogicOp) {
        void ** procp = (void **) &disp->LogicOp;
        snprintf(symboln, sizeof(symboln), "%sLogicOp", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->StencilFunc) {
        void ** procp = (void **) &disp->StencilFunc;
        snprintf(symboln, sizeof(symboln), "%sStencilFunc", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->StencilOp) {
        void ** procp = (void **) &disp->StencilOp;
        snprintf(symboln, sizeof(symboln), "%sStencilOp", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DepthFunc) {
        void ** procp = (void **) &disp->DepthFunc;
        snprintf(symboln, sizeof(symboln), "%sDepthFunc", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PixelZoom) {
        void ** procp = (void **) &disp->PixelZoom;
        snprintf(symboln, sizeof(symboln), "%sPixelZoom", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PixelTransferf) {
        void ** procp = (void **) &disp->PixelTransferf;
        snprintf(symboln, sizeof(symboln), "%sPixelTransferf", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PixelTransferi) {
        void ** procp = (void **) &disp->PixelTransferi;
        snprintf(symboln, sizeof(symboln), "%sPixelTransferi", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PixelStoref) {
        void ** procp = (void **) &disp->PixelStoref;
        snprintf(symboln, sizeof(symboln), "%sPixelStoref", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PixelStorei) {
        void ** procp = (void **) &disp->PixelStorei;
        snprintf(symboln, sizeof(symboln), "%sPixelStorei", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PixelMapfv) {
        void ** procp = (void **) &disp->PixelMapfv;
        snprintf(symboln, sizeof(symboln), "%sPixelMapfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PixelMapuiv) {
        void ** procp = (void **) &disp->PixelMapuiv;
        snprintf(symboln, sizeof(symboln), "%sPixelMapuiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PixelMapusv) {
        void ** procp = (void **) &disp->PixelMapusv;
        snprintf(symboln, sizeof(symboln), "%sPixelMapusv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ReadBuffer) {
        void ** procp = (void **) &disp->ReadBuffer;
        snprintf(symboln, sizeof(symboln), "%sReadBuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ReadBuffer) {
        void ** procp = (void **) &disp->ReadBuffer;
        snprintf(symboln, sizeof(symboln), "%sReadBufferNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CopyPixels) {
        void ** procp = (void **) &disp->CopyPixels;
        snprintf(symboln, sizeof(symboln), "%sCopyPixels", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ReadPixels) {
        void ** procp = (void **) &disp->ReadPixels;
        snprintf(symboln, sizeof(symboln), "%sReadPixels", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawPixels) {
        void ** procp = (void **) &disp->DrawPixels;
        snprintf(symboln, sizeof(symboln), "%sDrawPixels", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetBooleanv) {
        void ** procp = (void **) &disp->GetBooleanv;
        snprintf(symboln, sizeof(symboln), "%sGetBooleanv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetClipPlane) {
        void ** procp = (void **) &disp->GetClipPlane;
        snprintf(symboln, sizeof(symboln), "%sGetClipPlane", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetDoublev) {
        void ** procp = (void **) &disp->GetDoublev;
        snprintf(symboln, sizeof(symboln), "%sGetDoublev", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetError) {
        void ** procp = (void **) &disp->GetError;
        snprintf(symboln, sizeof(symboln), "%sGetError", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetFloatv) {
        void ** procp = (void **) &disp->GetFloatv;
        snprintf(symboln, sizeof(symboln), "%sGetFloatv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetIntegerv) {
        void ** procp = (void **) &disp->GetIntegerv;
        snprintf(symboln, sizeof(symboln), "%sGetIntegerv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetLightfv) {
        void ** procp = (void **) &disp->GetLightfv;
        snprintf(symboln, sizeof(symboln), "%sGetLightfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetLightiv) {
        void ** procp = (void **) &disp->GetLightiv;
        snprintf(symboln, sizeof(symboln), "%sGetLightiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetMapdv) {
        void ** procp = (void **) &disp->GetMapdv;
        snprintf(symboln, sizeof(symboln), "%sGetMapdv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetMapfv) {
        void ** procp = (void **) &disp->GetMapfv;
        snprintf(symboln, sizeof(symboln), "%sGetMapfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetMapiv) {
        void ** procp = (void **) &disp->GetMapiv;
        snprintf(symboln, sizeof(symboln), "%sGetMapiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetMaterialfv) {
        void ** procp = (void **) &disp->GetMaterialfv;
        snprintf(symboln, sizeof(symboln), "%sGetMaterialfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetMaterialiv) {
        void ** procp = (void **) &disp->GetMaterialiv;
        snprintf(symboln, sizeof(symboln), "%sGetMaterialiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetPixelMapfv) {
        void ** procp = (void **) &disp->GetPixelMapfv;
        snprintf(symboln, sizeof(symboln), "%sGetPixelMapfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetPixelMapuiv) {
        void ** procp = (void **) &disp->GetPixelMapuiv;
        snprintf(symboln, sizeof(symboln), "%sGetPixelMapuiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetPixelMapusv) {
        void ** procp = (void **) &disp->GetPixelMapusv;
        snprintf(symboln, sizeof(symboln), "%sGetPixelMapusv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetPolygonStipple) {
        void ** procp = (void **) &disp->GetPolygonStipple;
        snprintf(symboln, sizeof(symboln), "%sGetPolygonStipple", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetString) {
        void ** procp = (void **) &disp->GetString;
        snprintf(symboln, sizeof(symboln), "%sGetString", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTexEnvfv) {
        void ** procp = (void **) &disp->GetTexEnvfv;
        snprintf(symboln, sizeof(symboln), "%sGetTexEnvfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTexEnviv) {
        void ** procp = (void **) &disp->GetTexEnviv;
        snprintf(symboln, sizeof(symboln), "%sGetTexEnviv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTexGendv) {
        void ** procp = (void **) &disp->GetTexGendv;
        snprintf(symboln, sizeof(symboln), "%sGetTexGendv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTexGenfv) {
        void ** procp = (void **) &disp->GetTexGenfv;
        snprintf(symboln, sizeof(symboln), "%sGetTexGenfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTexGenfv) {
        void ** procp = (void **) &disp->GetTexGenfv;
        snprintf(symboln, sizeof(symboln), "%sGetTexGenfvOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTexGeniv) {
        void ** procp = (void **) &disp->GetTexGeniv;
        snprintf(symboln, sizeof(symboln), "%sGetTexGeniv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTexGeniv) {
        void ** procp = (void **) &disp->GetTexGeniv;
        snprintf(symboln, sizeof(symboln), "%sGetTexGenivOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTexImage) {
        void ** procp = (void **) &disp->GetTexImage;
        snprintf(symboln, sizeof(symboln), "%sGetTexImage", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTexParameterfv) {
        void ** procp = (void **) &disp->GetTexParameterfv;
        snprintf(symboln, sizeof(symboln), "%sGetTexParameterfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTexParameteriv) {
        void ** procp = (void **) &disp->GetTexParameteriv;
        snprintf(symboln, sizeof(symboln), "%sGetTexParameteriv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTexLevelParameterfv) {
        void ** procp = (void **) &disp->GetTexLevelParameterfv;
        snprintf(symboln, sizeof(symboln), "%sGetTexLevelParameterfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTexLevelParameteriv) {
        void ** procp = (void **) &disp->GetTexLevelParameteriv;
        snprintf(symboln, sizeof(symboln), "%sGetTexLevelParameteriv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsEnabled) {
        void ** procp = (void **) &disp->IsEnabled;
        snprintf(symboln, sizeof(symboln), "%sIsEnabled", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsList) {
        void ** procp = (void **) &disp->IsList;
        snprintf(symboln, sizeof(symboln), "%sIsList", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DepthRange) {
        void ** procp = (void **) &disp->DepthRange;
        snprintf(symboln, sizeof(symboln), "%sDepthRange", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Frustum) {
        void ** procp = (void **) &disp->Frustum;
        snprintf(symboln, sizeof(symboln), "%sFrustum", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LoadIdentity) {
        void ** procp = (void **) &disp->LoadIdentity;
        snprintf(symboln, sizeof(symboln), "%sLoadIdentity", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LoadMatrixf) {
        void ** procp = (void **) &disp->LoadMatrixf;
        snprintf(symboln, sizeof(symboln), "%sLoadMatrixf", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LoadMatrixd) {
        void ** procp = (void **) &disp->LoadMatrixd;
        snprintf(symboln, sizeof(symboln), "%sLoadMatrixd", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MatrixMode) {
        void ** procp = (void **) &disp->MatrixMode;
        snprintf(symboln, sizeof(symboln), "%sMatrixMode", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultMatrixf) {
        void ** procp = (void **) &disp->MultMatrixf;
        snprintf(symboln, sizeof(symboln), "%sMultMatrixf", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultMatrixd) {
        void ** procp = (void **) &disp->MultMatrixd;
        snprintf(symboln, sizeof(symboln), "%sMultMatrixd", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Ortho) {
        void ** procp = (void **) &disp->Ortho;
        snprintf(symboln, sizeof(symboln), "%sOrtho", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PopMatrix) {
        void ** procp = (void **) &disp->PopMatrix;
        snprintf(symboln, sizeof(symboln), "%sPopMatrix", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PushMatrix) {
        void ** procp = (void **) &disp->PushMatrix;
        snprintf(symboln, sizeof(symboln), "%sPushMatrix", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Rotated) {
        void ** procp = (void **) &disp->Rotated;
        snprintf(symboln, sizeof(symboln), "%sRotated", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Rotatef) {
        void ** procp = (void **) &disp->Rotatef;
        snprintf(symboln, sizeof(symboln), "%sRotatef", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Scaled) {
        void ** procp = (void **) &disp->Scaled;
        snprintf(symboln, sizeof(symboln), "%sScaled", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Scalef) {
        void ** procp = (void **) &disp->Scalef;
        snprintf(symboln, sizeof(symboln), "%sScalef", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Translated) {
        void ** procp = (void **) &disp->Translated;
        snprintf(symboln, sizeof(symboln), "%sTranslated", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Translatef) {
        void ** procp = (void **) &disp->Translatef;
        snprintf(symboln, sizeof(symboln), "%sTranslatef", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Viewport) {
        void ** procp = (void **) &disp->Viewport;
        snprintf(symboln, sizeof(symboln), "%sViewport", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ArrayElement) {
        void ** procp = (void **) &disp->ArrayElement;
        snprintf(symboln, sizeof(symboln), "%sArrayElement", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ArrayElement) {
        void ** procp = (void **) &disp->ArrayElement;
        snprintf(symboln, sizeof(symboln), "%sArrayElementEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindTexture) {
        void ** procp = (void **) &disp->BindTexture;
        snprintf(symboln, sizeof(symboln), "%sBindTexture", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindTexture) {
        void ** procp = (void **) &disp->BindTexture;
        snprintf(symboln, sizeof(symboln), "%sBindTextureEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ColorPointer) {
        void ** procp = (void **) &disp->ColorPointer;
        snprintf(symboln, sizeof(symboln), "%sColorPointer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DisableClientState) {
        void ** procp = (void **) &disp->DisableClientState;
        snprintf(symboln, sizeof(symboln), "%sDisableClientState", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawArrays) {
        void ** procp = (void **) &disp->DrawArrays;
        snprintf(symboln, sizeof(symboln), "%sDrawArrays", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawArrays) {
        void ** procp = (void **) &disp->DrawArrays;
        snprintf(symboln, sizeof(symboln), "%sDrawArraysEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawElements) {
        void ** procp = (void **) &disp->DrawElements;
        snprintf(symboln, sizeof(symboln), "%sDrawElements", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EdgeFlagPointer) {
        void ** procp = (void **) &disp->EdgeFlagPointer;
        snprintf(symboln, sizeof(symboln), "%sEdgeFlagPointer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EnableClientState) {
        void ** procp = (void **) &disp->EnableClientState;
        snprintf(symboln, sizeof(symboln), "%sEnableClientState", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IndexPointer) {
        void ** procp = (void **) &disp->IndexPointer;
        snprintf(symboln, sizeof(symboln), "%sIndexPointer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Indexub) {
        void ** procp = (void **) &disp->Indexub;
        snprintf(symboln, sizeof(symboln), "%sIndexub", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Indexubv) {
        void ** procp = (void **) &disp->Indexubv;
        snprintf(symboln, sizeof(symboln), "%sIndexubv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->InterleavedArrays) {
        void ** procp = (void **) &disp->InterleavedArrays;
        snprintf(symboln, sizeof(symboln), "%sInterleavedArrays", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->NormalPointer) {
        void ** procp = (void **) &disp->NormalPointer;
        snprintf(symboln, sizeof(symboln), "%sNormalPointer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PolygonOffset) {
        void ** procp = (void **) &disp->PolygonOffset;
        snprintf(symboln, sizeof(symboln), "%sPolygonOffset", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoordPointer) {
        void ** procp = (void **) &disp->TexCoordPointer;
        snprintf(symboln, sizeof(symboln), "%sTexCoordPointer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexPointer) {
        void ** procp = (void **) &disp->VertexPointer;
        snprintf(symboln, sizeof(symboln), "%sVertexPointer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->AreTexturesResident) {
        void ** procp = (void **) &disp->AreTexturesResident;
        snprintf(symboln, sizeof(symboln), "%sAreTexturesResident", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->AreTexturesResident) {
        void ** procp = (void **) &disp->AreTexturesResident;
        snprintf(symboln, sizeof(symboln), "%sAreTexturesResidentEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CopyTexImage1D) {
        void ** procp = (void **) &disp->CopyTexImage1D;
        snprintf(symboln, sizeof(symboln), "%sCopyTexImage1D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CopyTexImage1D) {
        void ** procp = (void **) &disp->CopyTexImage1D;
        snprintf(symboln, sizeof(symboln), "%sCopyTexImage1DEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CopyTexImage2D) {
        void ** procp = (void **) &disp->CopyTexImage2D;
        snprintf(symboln, sizeof(symboln), "%sCopyTexImage2D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CopyTexImage2D) {
        void ** procp = (void **) &disp->CopyTexImage2D;
        snprintf(symboln, sizeof(symboln), "%sCopyTexImage2DEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CopyTexSubImage1D) {
        void ** procp = (void **) &disp->CopyTexSubImage1D;
        snprintf(symboln, sizeof(symboln), "%sCopyTexSubImage1D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CopyTexSubImage1D) {
        void ** procp = (void **) &disp->CopyTexSubImage1D;
        snprintf(symboln, sizeof(symboln), "%sCopyTexSubImage1DEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CopyTexSubImage2D) {
        void ** procp = (void **) &disp->CopyTexSubImage2D;
        snprintf(symboln, sizeof(symboln), "%sCopyTexSubImage2D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CopyTexSubImage2D) {
        void ** procp = (void **) &disp->CopyTexSubImage2D;
        snprintf(symboln, sizeof(symboln), "%sCopyTexSubImage2DEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteTextures) {
        void ** procp = (void **) &disp->DeleteTextures;
        snprintf(symboln, sizeof(symboln), "%sDeleteTextures", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteTextures) {
        void ** procp = (void **) &disp->DeleteTextures;
        snprintf(symboln, sizeof(symboln), "%sDeleteTexturesEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenTextures) {
        void ** procp = (void **) &disp->GenTextures;
        snprintf(symboln, sizeof(symboln), "%sGenTextures", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenTextures) {
        void ** procp = (void **) &disp->GenTextures;
        snprintf(symboln, sizeof(symboln), "%sGenTexturesEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetPointerv) {
        void ** procp = (void **) &disp->GetPointerv;
        snprintf(symboln, sizeof(symboln), "%sGetPointerv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetPointerv) {
        void ** procp = (void **) &disp->GetPointerv;
        snprintf(symboln, sizeof(symboln), "%sGetPointervEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsTexture) {
        void ** procp = (void **) &disp->IsTexture;
        snprintf(symboln, sizeof(symboln), "%sIsTexture", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsTexture) {
        void ** procp = (void **) &disp->IsTexture;
        snprintf(symboln, sizeof(symboln), "%sIsTextureEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PrioritizeTextures) {
        void ** procp = (void **) &disp->PrioritizeTextures;
        snprintf(symboln, sizeof(symboln), "%sPrioritizeTextures", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PrioritizeTextures) {
        void ** procp = (void **) &disp->PrioritizeTextures;
        snprintf(symboln, sizeof(symboln), "%sPrioritizeTexturesEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexSubImage1D) {
        void ** procp = (void **) &disp->TexSubImage1D;
        snprintf(symboln, sizeof(symboln), "%sTexSubImage1D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexSubImage1D) {
        void ** procp = (void **) &disp->TexSubImage1D;
        snprintf(symboln, sizeof(symboln), "%sTexSubImage1DEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexSubImage2D) {
        void ** procp = (void **) &disp->TexSubImage2D;
        snprintf(symboln, sizeof(symboln), "%sTexSubImage2D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexSubImage2D) {
        void ** procp = (void **) &disp->TexSubImage2D;
        snprintf(symboln, sizeof(symboln), "%sTexSubImage2DEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PopClientAttrib) {
        void ** procp = (void **) &disp->PopClientAttrib;
        snprintf(symboln, sizeof(symboln), "%sPopClientAttrib", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PushClientAttrib) {
        void ** procp = (void **) &disp->PushClientAttrib;
        snprintf(symboln, sizeof(symboln), "%sPushClientAttrib", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlendColor) {
        void ** procp = (void **) &disp->BlendColor;
        snprintf(symboln, sizeof(symboln), "%sBlendColor", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlendColor) {
        void ** procp = (void **) &disp->BlendColor;
        snprintf(symboln, sizeof(symboln), "%sBlendColorEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlendEquation) {
        void ** procp = (void **) &disp->BlendEquation;
        snprintf(symboln, sizeof(symboln), "%sBlendEquation", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlendEquation) {
        void ** procp = (void **) &disp->BlendEquation;
        snprintf(symboln, sizeof(symboln), "%sBlendEquationEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlendEquation) {
        void ** procp = (void **) &disp->BlendEquation;
        snprintf(symboln, sizeof(symboln), "%sBlendEquationOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawRangeElements) {
        void ** procp = (void **) &disp->DrawRangeElements;
        snprintf(symboln, sizeof(symboln), "%sDrawRangeElements", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawRangeElements) {
        void ** procp = (void **) &disp->DrawRangeElements;
        snprintf(symboln, sizeof(symboln), "%sDrawRangeElementsEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ColorTable) {
        void ** procp = (void **) &disp->ColorTable;
        snprintf(symboln, sizeof(symboln), "%sColorTable", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ColorTable) {
        void ** procp = (void **) &disp->ColorTable;
        snprintf(symboln, sizeof(symboln), "%sColorTableSGI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ColorTable) {
        void ** procp = (void **) &disp->ColorTable;
        snprintf(symboln, sizeof(symboln), "%sColorTableEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ColorTableParameterfv) {
        void ** procp = (void **) &disp->ColorTableParameterfv;
        snprintf(symboln, sizeof(symboln), "%sColorTableParameterfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ColorTableParameterfv) {
        void ** procp = (void **) &disp->ColorTableParameterfv;
        snprintf(symboln, sizeof(symboln), "%sColorTableParameterfvSGI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ColorTableParameteriv) {
        void ** procp = (void **) &disp->ColorTableParameteriv;
        snprintf(symboln, sizeof(symboln), "%sColorTableParameteriv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ColorTableParameteriv) {
        void ** procp = (void **) &disp->ColorTableParameteriv;
        snprintf(symboln, sizeof(symboln), "%sColorTableParameterivSGI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CopyColorTable) {
        void ** procp = (void **) &disp->CopyColorTable;
        snprintf(symboln, sizeof(symboln), "%sCopyColorTable", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CopyColorTable) {
        void ** procp = (void **) &disp->CopyColorTable;
        snprintf(symboln, sizeof(symboln), "%sCopyColorTableSGI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetColorTable) {
        void ** procp = (void **) &disp->GetColorTable;
        snprintf(symboln, sizeof(symboln), "%sGetColorTable", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetColorTable) {
        void ** procp = (void **) &disp->GetColorTable;
        snprintf(symboln, sizeof(symboln), "%sGetColorTableSGI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetColorTable) {
        void ** procp = (void **) &disp->GetColorTable;
        snprintf(symboln, sizeof(symboln), "%sGetColorTableEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetColorTableParameterfv) {
        void ** procp = (void **) &disp->GetColorTableParameterfv;
        snprintf(symboln, sizeof(symboln), "%sGetColorTableParameterfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetColorTableParameterfv) {
        void ** procp = (void **) &disp->GetColorTableParameterfv;
        snprintf(symboln, sizeof(symboln), "%sGetColorTableParameterfvSGI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetColorTableParameterfv) {
        void ** procp = (void **) &disp->GetColorTableParameterfv;
        snprintf(symboln, sizeof(symboln), "%sGetColorTableParameterfvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetColorTableParameteriv) {
        void ** procp = (void **) &disp->GetColorTableParameteriv;
        snprintf(symboln, sizeof(symboln), "%sGetColorTableParameteriv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetColorTableParameteriv) {
        void ** procp = (void **) &disp->GetColorTableParameteriv;
        snprintf(symboln, sizeof(symboln), "%sGetColorTableParameterivSGI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetColorTableParameteriv) {
        void ** procp = (void **) &disp->GetColorTableParameteriv;
        snprintf(symboln, sizeof(symboln), "%sGetColorTableParameterivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ColorSubTable) {
        void ** procp = (void **) &disp->ColorSubTable;
        snprintf(symboln, sizeof(symboln), "%sColorSubTable", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ColorSubTable) {
        void ** procp = (void **) &disp->ColorSubTable;
        snprintf(symboln, sizeof(symboln), "%sColorSubTableEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CopyColorSubTable) {
        void ** procp = (void **) &disp->CopyColorSubTable;
        snprintf(symboln, sizeof(symboln), "%sCopyColorSubTable", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CopyColorSubTable) {
        void ** procp = (void **) &disp->CopyColorSubTable;
        snprintf(symboln, sizeof(symboln), "%sCopyColorSubTableEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ConvolutionFilter1D) {
        void ** procp = (void **) &disp->ConvolutionFilter1D;
        snprintf(symboln, sizeof(symboln), "%sConvolutionFilter1D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ConvolutionFilter1D) {
        void ** procp = (void **) &disp->ConvolutionFilter1D;
        snprintf(symboln, sizeof(symboln), "%sConvolutionFilter1DEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ConvolutionFilter2D) {
        void ** procp = (void **) &disp->ConvolutionFilter2D;
        snprintf(symboln, sizeof(symboln), "%sConvolutionFilter2D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ConvolutionFilter2D) {
        void ** procp = (void **) &disp->ConvolutionFilter2D;
        snprintf(symboln, sizeof(symboln), "%sConvolutionFilter2DEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ConvolutionParameterf) {
        void ** procp = (void **) &disp->ConvolutionParameterf;
        snprintf(symboln, sizeof(symboln), "%sConvolutionParameterf", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ConvolutionParameterf) {
        void ** procp = (void **) &disp->ConvolutionParameterf;
        snprintf(symboln, sizeof(symboln), "%sConvolutionParameterfEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ConvolutionParameterfv) {
        void ** procp = (void **) &disp->ConvolutionParameterfv;
        snprintf(symboln, sizeof(symboln), "%sConvolutionParameterfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ConvolutionParameterfv) {
        void ** procp = (void **) &disp->ConvolutionParameterfv;
        snprintf(symboln, sizeof(symboln), "%sConvolutionParameterfvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ConvolutionParameteri) {
        void ** procp = (void **) &disp->ConvolutionParameteri;
        snprintf(symboln, sizeof(symboln), "%sConvolutionParameteri", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ConvolutionParameteri) {
        void ** procp = (void **) &disp->ConvolutionParameteri;
        snprintf(symboln, sizeof(symboln), "%sConvolutionParameteriEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ConvolutionParameteriv) {
        void ** procp = (void **) &disp->ConvolutionParameteriv;
        snprintf(symboln, sizeof(symboln), "%sConvolutionParameteriv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ConvolutionParameteriv) {
        void ** procp = (void **) &disp->ConvolutionParameteriv;
        snprintf(symboln, sizeof(symboln), "%sConvolutionParameterivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CopyConvolutionFilter1D) {
        void ** procp = (void **) &disp->CopyConvolutionFilter1D;
        snprintf(symboln, sizeof(symboln), "%sCopyConvolutionFilter1D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CopyConvolutionFilter1D) {
        void ** procp = (void **) &disp->CopyConvolutionFilter1D;
        snprintf(symboln, sizeof(symboln), "%sCopyConvolutionFilter1DEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CopyConvolutionFilter2D) {
        void ** procp = (void **) &disp->CopyConvolutionFilter2D;
        snprintf(symboln, sizeof(symboln), "%sCopyConvolutionFilter2D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CopyConvolutionFilter2D) {
        void ** procp = (void **) &disp->CopyConvolutionFilter2D;
        snprintf(symboln, sizeof(symboln), "%sCopyConvolutionFilter2DEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetConvolutionFilter) {
        void ** procp = (void **) &disp->GetConvolutionFilter;
        snprintf(symboln, sizeof(symboln), "%sGetConvolutionFilter", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetConvolutionFilter) {
        void ** procp = (void **) &disp->GetConvolutionFilter;
        snprintf(symboln, sizeof(symboln), "%sGetConvolutionFilterEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetConvolutionParameterfv) {
        void ** procp = (void **) &disp->GetConvolutionParameterfv;
        snprintf(symboln, sizeof(symboln), "%sGetConvolutionParameterfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetConvolutionParameterfv) {
        void ** procp = (void **) &disp->GetConvolutionParameterfv;
        snprintf(symboln, sizeof(symboln), "%sGetConvolutionParameterfvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetConvolutionParameteriv) {
        void ** procp = (void **) &disp->GetConvolutionParameteriv;
        snprintf(symboln, sizeof(symboln), "%sGetConvolutionParameteriv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetConvolutionParameteriv) {
        void ** procp = (void **) &disp->GetConvolutionParameteriv;
        snprintf(symboln, sizeof(symboln), "%sGetConvolutionParameterivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetSeparableFilter) {
        void ** procp = (void **) &disp->GetSeparableFilter;
        snprintf(symboln, sizeof(symboln), "%sGetSeparableFilter", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetSeparableFilter) {
        void ** procp = (void **) &disp->GetSeparableFilter;
        snprintf(symboln, sizeof(symboln), "%sGetSeparableFilterEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SeparableFilter2D) {
        void ** procp = (void **) &disp->SeparableFilter2D;
        snprintf(symboln, sizeof(symboln), "%sSeparableFilter2D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SeparableFilter2D) {
        void ** procp = (void **) &disp->SeparableFilter2D;
        snprintf(symboln, sizeof(symboln), "%sSeparableFilter2DEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetHistogram) {
        void ** procp = (void **) &disp->GetHistogram;
        snprintf(symboln, sizeof(symboln), "%sGetHistogram", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetHistogram) {
        void ** procp = (void **) &disp->GetHistogram;
        snprintf(symboln, sizeof(symboln), "%sGetHistogramEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetHistogramParameterfv) {
        void ** procp = (void **) &disp->GetHistogramParameterfv;
        snprintf(symboln, sizeof(symboln), "%sGetHistogramParameterfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetHistogramParameterfv) {
        void ** procp = (void **) &disp->GetHistogramParameterfv;
        snprintf(symboln, sizeof(symboln), "%sGetHistogramParameterfvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetHistogramParameteriv) {
        void ** procp = (void **) &disp->GetHistogramParameteriv;
        snprintf(symboln, sizeof(symboln), "%sGetHistogramParameteriv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetHistogramParameteriv) {
        void ** procp = (void **) &disp->GetHistogramParameteriv;
        snprintf(symboln, sizeof(symboln), "%sGetHistogramParameterivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetMinmax) {
        void ** procp = (void **) &disp->GetMinmax;
        snprintf(symboln, sizeof(symboln), "%sGetMinmax", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetMinmax) {
        void ** procp = (void **) &disp->GetMinmax;
        snprintf(symboln, sizeof(symboln), "%sGetMinmaxEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetMinmaxParameterfv) {
        void ** procp = (void **) &disp->GetMinmaxParameterfv;
        snprintf(symboln, sizeof(symboln), "%sGetMinmaxParameterfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetMinmaxParameterfv) {
        void ** procp = (void **) &disp->GetMinmaxParameterfv;
        snprintf(symboln, sizeof(symboln), "%sGetMinmaxParameterfvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetMinmaxParameteriv) {
        void ** procp = (void **) &disp->GetMinmaxParameteriv;
        snprintf(symboln, sizeof(symboln), "%sGetMinmaxParameteriv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetMinmaxParameteriv) {
        void ** procp = (void **) &disp->GetMinmaxParameteriv;
        snprintf(symboln, sizeof(symboln), "%sGetMinmaxParameterivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Histogram) {
        void ** procp = (void **) &disp->Histogram;
        snprintf(symboln, sizeof(symboln), "%sHistogram", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Histogram) {
        void ** procp = (void **) &disp->Histogram;
        snprintf(symboln, sizeof(symboln), "%sHistogramEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Minmax) {
        void ** procp = (void **) &disp->Minmax;
        snprintf(symboln, sizeof(symboln), "%sMinmax", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Minmax) {
        void ** procp = (void **) &disp->Minmax;
        snprintf(symboln, sizeof(symboln), "%sMinmaxEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ResetHistogram) {
        void ** procp = (void **) &disp->ResetHistogram;
        snprintf(symboln, sizeof(symboln), "%sResetHistogram", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ResetHistogram) {
        void ** procp = (void **) &disp->ResetHistogram;
        snprintf(symboln, sizeof(symboln), "%sResetHistogramEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ResetMinmax) {
        void ** procp = (void **) &disp->ResetMinmax;
        snprintf(symboln, sizeof(symboln), "%sResetMinmax", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ResetMinmax) {
        void ** procp = (void **) &disp->ResetMinmax;
        snprintf(symboln, sizeof(symboln), "%sResetMinmaxEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexImage3D) {
        void ** procp = (void **) &disp->TexImage3D;
        snprintf(symboln, sizeof(symboln), "%sTexImage3D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexImage3D) {
        void ** procp = (void **) &disp->TexImage3D;
        snprintf(symboln, sizeof(symboln), "%sTexImage3DEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexImage3D) {
        void ** procp = (void **) &disp->TexImage3D;
        snprintf(symboln, sizeof(symboln), "%sTexImage3DOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexSubImage3D) {
        void ** procp = (void **) &disp->TexSubImage3D;
        snprintf(symboln, sizeof(symboln), "%sTexSubImage3D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexSubImage3D) {
        void ** procp = (void **) &disp->TexSubImage3D;
        snprintf(symboln, sizeof(symboln), "%sTexSubImage3DEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexSubImage3D) {
        void ** procp = (void **) &disp->TexSubImage3D;
        snprintf(symboln, sizeof(symboln), "%sTexSubImage3DOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CopyTexSubImage3D) {
        void ** procp = (void **) &disp->CopyTexSubImage3D;
        snprintf(symboln, sizeof(symboln), "%sCopyTexSubImage3D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CopyTexSubImage3D) {
        void ** procp = (void **) &disp->CopyTexSubImage3D;
        snprintf(symboln, sizeof(symboln), "%sCopyTexSubImage3DEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CopyTexSubImage3D) {
        void ** procp = (void **) &disp->CopyTexSubImage3D;
        snprintf(symboln, sizeof(symboln), "%sCopyTexSubImage3DOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ActiveTexture) {
        void ** procp = (void **) &disp->ActiveTexture;
        snprintf(symboln, sizeof(symboln), "%sActiveTexture", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ActiveTexture) {
        void ** procp = (void **) &disp->ActiveTexture;
        snprintf(symboln, sizeof(symboln), "%sActiveTextureARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClientActiveTexture) {
        void ** procp = (void **) &disp->ClientActiveTexture;
        snprintf(symboln, sizeof(symboln), "%sClientActiveTexture", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClientActiveTexture) {
        void ** procp = (void **) &disp->ClientActiveTexture;
        snprintf(symboln, sizeof(symboln), "%sClientActiveTextureARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord1d) {
        void ** procp = (void **) &disp->MultiTexCoord1d;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord1d) {
        void ** procp = (void **) &disp->MultiTexCoord1d;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1dARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord1dv) {
        void ** procp = (void **) &disp->MultiTexCoord1dv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord1dv) {
        void ** procp = (void **) &disp->MultiTexCoord1dv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1dvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord1fARB) {
        void ** procp = (void **) &disp->MultiTexCoord1fARB;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord1fARB) {
        void ** procp = (void **) &disp->MultiTexCoord1fARB;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1fARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord1fvARB) {
        void ** procp = (void **) &disp->MultiTexCoord1fvARB;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord1fvARB) {
        void ** procp = (void **) &disp->MultiTexCoord1fvARB;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1fvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord1i) {
        void ** procp = (void **) &disp->MultiTexCoord1i;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord1i) {
        void ** procp = (void **) &disp->MultiTexCoord1i;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1iARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord1iv) {
        void ** procp = (void **) &disp->MultiTexCoord1iv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord1iv) {
        void ** procp = (void **) &disp->MultiTexCoord1iv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1ivARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord1s) {
        void ** procp = (void **) &disp->MultiTexCoord1s;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1s", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord1s) {
        void ** procp = (void **) &disp->MultiTexCoord1s;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1sARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord1sv) {
        void ** procp = (void **) &disp->MultiTexCoord1sv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1sv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord1sv) {
        void ** procp = (void **) &disp->MultiTexCoord1sv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord1svARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord2d) {
        void ** procp = (void **) &disp->MultiTexCoord2d;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord2d) {
        void ** procp = (void **) &disp->MultiTexCoord2d;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2dARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord2dv) {
        void ** procp = (void **) &disp->MultiTexCoord2dv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord2dv) {
        void ** procp = (void **) &disp->MultiTexCoord2dv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2dvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord2fARB) {
        void ** procp = (void **) &disp->MultiTexCoord2fARB;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord2fARB) {
        void ** procp = (void **) &disp->MultiTexCoord2fARB;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2fARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord2fvARB) {
        void ** procp = (void **) &disp->MultiTexCoord2fvARB;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord2fvARB) {
        void ** procp = (void **) &disp->MultiTexCoord2fvARB;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2fvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord2i) {
        void ** procp = (void **) &disp->MultiTexCoord2i;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord2i) {
        void ** procp = (void **) &disp->MultiTexCoord2i;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2iARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord2iv) {
        void ** procp = (void **) &disp->MultiTexCoord2iv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord2iv) {
        void ** procp = (void **) &disp->MultiTexCoord2iv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2ivARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord2s) {
        void ** procp = (void **) &disp->MultiTexCoord2s;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2s", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord2s) {
        void ** procp = (void **) &disp->MultiTexCoord2s;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2sARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord2sv) {
        void ** procp = (void **) &disp->MultiTexCoord2sv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2sv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord2sv) {
        void ** procp = (void **) &disp->MultiTexCoord2sv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord2svARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord3d) {
        void ** procp = (void **) &disp->MultiTexCoord3d;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord3d) {
        void ** procp = (void **) &disp->MultiTexCoord3d;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3dARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord3dv) {
        void ** procp = (void **) &disp->MultiTexCoord3dv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord3dv) {
        void ** procp = (void **) &disp->MultiTexCoord3dv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3dvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord3fARB) {
        void ** procp = (void **) &disp->MultiTexCoord3fARB;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord3fARB) {
        void ** procp = (void **) &disp->MultiTexCoord3fARB;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3fARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord3fvARB) {
        void ** procp = (void **) &disp->MultiTexCoord3fvARB;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord3fvARB) {
        void ** procp = (void **) &disp->MultiTexCoord3fvARB;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3fvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord3i) {
        void ** procp = (void **) &disp->MultiTexCoord3i;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord3i) {
        void ** procp = (void **) &disp->MultiTexCoord3i;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3iARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord3iv) {
        void ** procp = (void **) &disp->MultiTexCoord3iv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord3iv) {
        void ** procp = (void **) &disp->MultiTexCoord3iv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3ivARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord3s) {
        void ** procp = (void **) &disp->MultiTexCoord3s;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3s", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord3s) {
        void ** procp = (void **) &disp->MultiTexCoord3s;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3sARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord3sv) {
        void ** procp = (void **) &disp->MultiTexCoord3sv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3sv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord3sv) {
        void ** procp = (void **) &disp->MultiTexCoord3sv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord3svARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord4d) {
        void ** procp = (void **) &disp->MultiTexCoord4d;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord4d) {
        void ** procp = (void **) &disp->MultiTexCoord4d;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4dARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord4dv) {
        void ** procp = (void **) &disp->MultiTexCoord4dv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord4dv) {
        void ** procp = (void **) &disp->MultiTexCoord4dv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4dvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord4fARB) {
        void ** procp = (void **) &disp->MultiTexCoord4fARB;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord4fARB) {
        void ** procp = (void **) &disp->MultiTexCoord4fARB;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4fARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord4fvARB) {
        void ** procp = (void **) &disp->MultiTexCoord4fvARB;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord4fvARB) {
        void ** procp = (void **) &disp->MultiTexCoord4fvARB;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4fvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord4i) {
        void ** procp = (void **) &disp->MultiTexCoord4i;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord4i) {
        void ** procp = (void **) &disp->MultiTexCoord4i;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4iARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord4iv) {
        void ** procp = (void **) &disp->MultiTexCoord4iv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord4iv) {
        void ** procp = (void **) &disp->MultiTexCoord4iv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4ivARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord4s) {
        void ** procp = (void **) &disp->MultiTexCoord4s;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4s", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord4s) {
        void ** procp = (void **) &disp->MultiTexCoord4s;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4sARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord4sv) {
        void ** procp = (void **) &disp->MultiTexCoord4sv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4sv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord4sv) {
        void ** procp = (void **) &disp->MultiTexCoord4sv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4svARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CompressedTexImage1D) {
        void ** procp = (void **) &disp->CompressedTexImage1D;
        snprintf(symboln, sizeof(symboln), "%sCompressedTexImage1D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CompressedTexImage1D) {
        void ** procp = (void **) &disp->CompressedTexImage1D;
        snprintf(symboln, sizeof(symboln), "%sCompressedTexImage1DARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CompressedTexImage2D) {
        void ** procp = (void **) &disp->CompressedTexImage2D;
        snprintf(symboln, sizeof(symboln), "%sCompressedTexImage2D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CompressedTexImage2D) {
        void ** procp = (void **) &disp->CompressedTexImage2D;
        snprintf(symboln, sizeof(symboln), "%sCompressedTexImage2DARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CompressedTexImage3D) {
        void ** procp = (void **) &disp->CompressedTexImage3D;
        snprintf(symboln, sizeof(symboln), "%sCompressedTexImage3D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CompressedTexImage3D) {
        void ** procp = (void **) &disp->CompressedTexImage3D;
        snprintf(symboln, sizeof(symboln), "%sCompressedTexImage3DARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CompressedTexImage3D) {
        void ** procp = (void **) &disp->CompressedTexImage3D;
        snprintf(symboln, sizeof(symboln), "%sCompressedTexImage3DOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CompressedTexSubImage1D) {
        void ** procp = (void **) &disp->CompressedTexSubImage1D;
        snprintf(symboln, sizeof(symboln), "%sCompressedTexSubImage1D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CompressedTexSubImage1D) {
        void ** procp = (void **) &disp->CompressedTexSubImage1D;
        snprintf(symboln, sizeof(symboln), "%sCompressedTexSubImage1DARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CompressedTexSubImage2D) {
        void ** procp = (void **) &disp->CompressedTexSubImage2D;
        snprintf(symboln, sizeof(symboln), "%sCompressedTexSubImage2D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CompressedTexSubImage2D) {
        void ** procp = (void **) &disp->CompressedTexSubImage2D;
        snprintf(symboln, sizeof(symboln), "%sCompressedTexSubImage2DARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CompressedTexSubImage3D) {
        void ** procp = (void **) &disp->CompressedTexSubImage3D;
        snprintf(symboln, sizeof(symboln), "%sCompressedTexSubImage3D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CompressedTexSubImage3D) {
        void ** procp = (void **) &disp->CompressedTexSubImage3D;
        snprintf(symboln, sizeof(symboln), "%sCompressedTexSubImage3DARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CompressedTexSubImage3D) {
        void ** procp = (void **) &disp->CompressedTexSubImage3D;
        snprintf(symboln, sizeof(symboln), "%sCompressedTexSubImage3DOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetCompressedTexImage) {
        void ** procp = (void **) &disp->GetCompressedTexImage;
        snprintf(symboln, sizeof(symboln), "%sGetCompressedTexImage", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetCompressedTexImage) {
        void ** procp = (void **) &disp->GetCompressedTexImage;
        snprintf(symboln, sizeof(symboln), "%sGetCompressedTexImageARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LoadTransposeMatrixd) {
        void ** procp = (void **) &disp->LoadTransposeMatrixd;
        snprintf(symboln, sizeof(symboln), "%sLoadTransposeMatrixd", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LoadTransposeMatrixd) {
        void ** procp = (void **) &disp->LoadTransposeMatrixd;
        snprintf(symboln, sizeof(symboln), "%sLoadTransposeMatrixdARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LoadTransposeMatrixf) {
        void ** procp = (void **) &disp->LoadTransposeMatrixf;
        snprintf(symboln, sizeof(symboln), "%sLoadTransposeMatrixf", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LoadTransposeMatrixf) {
        void ** procp = (void **) &disp->LoadTransposeMatrixf;
        snprintf(symboln, sizeof(symboln), "%sLoadTransposeMatrixfARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultTransposeMatrixd) {
        void ** procp = (void **) &disp->MultTransposeMatrixd;
        snprintf(symboln, sizeof(symboln), "%sMultTransposeMatrixd", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultTransposeMatrixd) {
        void ** procp = (void **) &disp->MultTransposeMatrixd;
        snprintf(symboln, sizeof(symboln), "%sMultTransposeMatrixdARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultTransposeMatrixf) {
        void ** procp = (void **) &disp->MultTransposeMatrixf;
        snprintf(symboln, sizeof(symboln), "%sMultTransposeMatrixf", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultTransposeMatrixf) {
        void ** procp = (void **) &disp->MultTransposeMatrixf;
        snprintf(symboln, sizeof(symboln), "%sMultTransposeMatrixfARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SampleCoverage) {
        void ** procp = (void **) &disp->SampleCoverage;
        snprintf(symboln, sizeof(symboln), "%sSampleCoverage", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SampleCoverage) {
        void ** procp = (void **) &disp->SampleCoverage;
        snprintf(symboln, sizeof(symboln), "%sSampleCoverageARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlendFuncSeparate) {
        void ** procp = (void **) &disp->BlendFuncSeparate;
        snprintf(symboln, sizeof(symboln), "%sBlendFuncSeparate", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlendFuncSeparate) {
        void ** procp = (void **) &disp->BlendFuncSeparate;
        snprintf(symboln, sizeof(symboln), "%sBlendFuncSeparateEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlendFuncSeparate) {
        void ** procp = (void **) &disp->BlendFuncSeparate;
        snprintf(symboln, sizeof(symboln), "%sBlendFuncSeparateINGR", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlendFuncSeparate) {
        void ** procp = (void **) &disp->BlendFuncSeparate;
        snprintf(symboln, sizeof(symboln), "%sBlendFuncSeparateOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FogCoordPointer) {
        void ** procp = (void **) &disp->FogCoordPointer;
        snprintf(symboln, sizeof(symboln), "%sFogCoordPointer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FogCoordPointer) {
        void ** procp = (void **) &disp->FogCoordPointer;
        snprintf(symboln, sizeof(symboln), "%sFogCoordPointerEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FogCoordd) {
        void ** procp = (void **) &disp->FogCoordd;
        snprintf(symboln, sizeof(symboln), "%sFogCoordd", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FogCoordd) {
        void ** procp = (void **) &disp->FogCoordd;
        snprintf(symboln, sizeof(symboln), "%sFogCoorddEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FogCoorddv) {
        void ** procp = (void **) &disp->FogCoorddv;
        snprintf(symboln, sizeof(symboln), "%sFogCoorddv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FogCoorddv) {
        void ** procp = (void **) &disp->FogCoorddv;
        snprintf(symboln, sizeof(symboln), "%sFogCoorddvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiDrawArrays) {
        void ** procp = (void **) &disp->MultiDrawArrays;
        snprintf(symboln, sizeof(symboln), "%sMultiDrawArrays", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiDrawArrays) {
        void ** procp = (void **) &disp->MultiDrawArrays;
        snprintf(symboln, sizeof(symboln), "%sMultiDrawArraysEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PointParameterf) {
        void ** procp = (void **) &disp->PointParameterf;
        snprintf(symboln, sizeof(symboln), "%sPointParameterf", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PointParameterf) {
        void ** procp = (void **) &disp->PointParameterf;
        snprintf(symboln, sizeof(symboln), "%sPointParameterfARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PointParameterf) {
        void ** procp = (void **) &disp->PointParameterf;
        snprintf(symboln, sizeof(symboln), "%sPointParameterfEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PointParameterf) {
        void ** procp = (void **) &disp->PointParameterf;
        snprintf(symboln, sizeof(symboln), "%sPointParameterfSGIS", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PointParameterfv) {
        void ** procp = (void **) &disp->PointParameterfv;
        snprintf(symboln, sizeof(symboln), "%sPointParameterfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PointParameterfv) {
        void ** procp = (void **) &disp->PointParameterfv;
        snprintf(symboln, sizeof(symboln), "%sPointParameterfvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PointParameterfv) {
        void ** procp = (void **) &disp->PointParameterfv;
        snprintf(symboln, sizeof(symboln), "%sPointParameterfvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PointParameterfv) {
        void ** procp = (void **) &disp->PointParameterfv;
        snprintf(symboln, sizeof(symboln), "%sPointParameterfvSGIS", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PointParameteri) {
        void ** procp = (void **) &disp->PointParameteri;
        snprintf(symboln, sizeof(symboln), "%sPointParameteri", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PointParameteri) {
        void ** procp = (void **) &disp->PointParameteri;
        snprintf(symboln, sizeof(symboln), "%sPointParameteriNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PointParameteriv) {
        void ** procp = (void **) &disp->PointParameteriv;
        snprintf(symboln, sizeof(symboln), "%sPointParameteriv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PointParameteriv) {
        void ** procp = (void **) &disp->PointParameteriv;
        snprintf(symboln, sizeof(symboln), "%sPointParameterivNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3b) {
        void ** procp = (void **) &disp->SecondaryColor3b;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3b", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3b) {
        void ** procp = (void **) &disp->SecondaryColor3b;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3bEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3bv) {
        void ** procp = (void **) &disp->SecondaryColor3bv;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3bv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3bv) {
        void ** procp = (void **) &disp->SecondaryColor3bv;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3bvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3d) {
        void ** procp = (void **) &disp->SecondaryColor3d;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3d) {
        void ** procp = (void **) &disp->SecondaryColor3d;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3dEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3dv) {
        void ** procp = (void **) &disp->SecondaryColor3dv;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3dv) {
        void ** procp = (void **) &disp->SecondaryColor3dv;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3dvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3i) {
        void ** procp = (void **) &disp->SecondaryColor3i;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3i) {
        void ** procp = (void **) &disp->SecondaryColor3i;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3iEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3iv) {
        void ** procp = (void **) &disp->SecondaryColor3iv;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3iv) {
        void ** procp = (void **) &disp->SecondaryColor3iv;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3ivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3s) {
        void ** procp = (void **) &disp->SecondaryColor3s;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3s", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3s) {
        void ** procp = (void **) &disp->SecondaryColor3s;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3sEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3sv) {
        void ** procp = (void **) &disp->SecondaryColor3sv;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3sv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3sv) {
        void ** procp = (void **) &disp->SecondaryColor3sv;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3svEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3ub) {
        void ** procp = (void **) &disp->SecondaryColor3ub;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3ub", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3ub) {
        void ** procp = (void **) &disp->SecondaryColor3ub;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3ubEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3ubv) {
        void ** procp = (void **) &disp->SecondaryColor3ubv;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3ubv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3ubv) {
        void ** procp = (void **) &disp->SecondaryColor3ubv;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3ubvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3ui) {
        void ** procp = (void **) &disp->SecondaryColor3ui;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3ui) {
        void ** procp = (void **) &disp->SecondaryColor3ui;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3uiEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3uiv) {
        void ** procp = (void **) &disp->SecondaryColor3uiv;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3uiv) {
        void ** procp = (void **) &disp->SecondaryColor3uiv;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3uivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3us) {
        void ** procp = (void **) &disp->SecondaryColor3us;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3us", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3us) {
        void ** procp = (void **) &disp->SecondaryColor3us;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3usEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3usv) {
        void ** procp = (void **) &disp->SecondaryColor3usv;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3usv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3usv) {
        void ** procp = (void **) &disp->SecondaryColor3usv;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3usvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColorPointer) {
        void ** procp = (void **) &disp->SecondaryColorPointer;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColorPointer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColorPointer) {
        void ** procp = (void **) &disp->SecondaryColorPointer;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColorPointerEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos2d) {
        void ** procp = (void **) &disp->WindowPos2d;
        snprintf(symboln, sizeof(symboln), "%sWindowPos2d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos2d) {
        void ** procp = (void **) &disp->WindowPos2d;
        snprintf(symboln, sizeof(symboln), "%sWindowPos2dARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos2d) {
        void ** procp = (void **) &disp->WindowPos2d;
        snprintf(symboln, sizeof(symboln), "%sWindowPos2dMESA", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos2dv) {
        void ** procp = (void **) &disp->WindowPos2dv;
        snprintf(symboln, sizeof(symboln), "%sWindowPos2dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos2dv) {
        void ** procp = (void **) &disp->WindowPos2dv;
        snprintf(symboln, sizeof(symboln), "%sWindowPos2dvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos2dv) {
        void ** procp = (void **) &disp->WindowPos2dv;
        snprintf(symboln, sizeof(symboln), "%sWindowPos2dvMESA", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos2f) {
        void ** procp = (void **) &disp->WindowPos2f;
        snprintf(symboln, sizeof(symboln), "%sWindowPos2f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos2f) {
        void ** procp = (void **) &disp->WindowPos2f;
        snprintf(symboln, sizeof(symboln), "%sWindowPos2fARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos2f) {
        void ** procp = (void **) &disp->WindowPos2f;
        snprintf(symboln, sizeof(symboln), "%sWindowPos2fMESA", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos2fv) {
        void ** procp = (void **) &disp->WindowPos2fv;
        snprintf(symboln, sizeof(symboln), "%sWindowPos2fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos2fv) {
        void ** procp = (void **) &disp->WindowPos2fv;
        snprintf(symboln, sizeof(symboln), "%sWindowPos2fvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos2fv) {
        void ** procp = (void **) &disp->WindowPos2fv;
        snprintf(symboln, sizeof(symboln), "%sWindowPos2fvMESA", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos2i) {
        void ** procp = (void **) &disp->WindowPos2i;
        snprintf(symboln, sizeof(symboln), "%sWindowPos2i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos2i) {
        void ** procp = (void **) &disp->WindowPos2i;
        snprintf(symboln, sizeof(symboln), "%sWindowPos2iARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos2i) {
        void ** procp = (void **) &disp->WindowPos2i;
        snprintf(symboln, sizeof(symboln), "%sWindowPos2iMESA", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos2iv) {
        void ** procp = (void **) &disp->WindowPos2iv;
        snprintf(symboln, sizeof(symboln), "%sWindowPos2iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos2iv) {
        void ** procp = (void **) &disp->WindowPos2iv;
        snprintf(symboln, sizeof(symboln), "%sWindowPos2ivARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos2iv) {
        void ** procp = (void **) &disp->WindowPos2iv;
        snprintf(symboln, sizeof(symboln), "%sWindowPos2ivMESA", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos2s) {
        void ** procp = (void **) &disp->WindowPos2s;
        snprintf(symboln, sizeof(symboln), "%sWindowPos2s", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos2s) {
        void ** procp = (void **) &disp->WindowPos2s;
        snprintf(symboln, sizeof(symboln), "%sWindowPos2sARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos2s) {
        void ** procp = (void **) &disp->WindowPos2s;
        snprintf(symboln, sizeof(symboln), "%sWindowPos2sMESA", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos2sv) {
        void ** procp = (void **) &disp->WindowPos2sv;
        snprintf(symboln, sizeof(symboln), "%sWindowPos2sv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos2sv) {
        void ** procp = (void **) &disp->WindowPos2sv;
        snprintf(symboln, sizeof(symboln), "%sWindowPos2svARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos2sv) {
        void ** procp = (void **) &disp->WindowPos2sv;
        snprintf(symboln, sizeof(symboln), "%sWindowPos2svMESA", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos3d) {
        void ** procp = (void **) &disp->WindowPos3d;
        snprintf(symboln, sizeof(symboln), "%sWindowPos3d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos3d) {
        void ** procp = (void **) &disp->WindowPos3d;
        snprintf(symboln, sizeof(symboln), "%sWindowPos3dARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos3d) {
        void ** procp = (void **) &disp->WindowPos3d;
        snprintf(symboln, sizeof(symboln), "%sWindowPos3dMESA", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos3dv) {
        void ** procp = (void **) &disp->WindowPos3dv;
        snprintf(symboln, sizeof(symboln), "%sWindowPos3dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos3dv) {
        void ** procp = (void **) &disp->WindowPos3dv;
        snprintf(symboln, sizeof(symboln), "%sWindowPos3dvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos3dv) {
        void ** procp = (void **) &disp->WindowPos3dv;
        snprintf(symboln, sizeof(symboln), "%sWindowPos3dvMESA", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos3f) {
        void ** procp = (void **) &disp->WindowPos3f;
        snprintf(symboln, sizeof(symboln), "%sWindowPos3f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos3f) {
        void ** procp = (void **) &disp->WindowPos3f;
        snprintf(symboln, sizeof(symboln), "%sWindowPos3fARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos3f) {
        void ** procp = (void **) &disp->WindowPos3f;
        snprintf(symboln, sizeof(symboln), "%sWindowPos3fMESA", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos3fv) {
        void ** procp = (void **) &disp->WindowPos3fv;
        snprintf(symboln, sizeof(symboln), "%sWindowPos3fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos3fv) {
        void ** procp = (void **) &disp->WindowPos3fv;
        snprintf(symboln, sizeof(symboln), "%sWindowPos3fvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos3fv) {
        void ** procp = (void **) &disp->WindowPos3fv;
        snprintf(symboln, sizeof(symboln), "%sWindowPos3fvMESA", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos3i) {
        void ** procp = (void **) &disp->WindowPos3i;
        snprintf(symboln, sizeof(symboln), "%sWindowPos3i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos3i) {
        void ** procp = (void **) &disp->WindowPos3i;
        snprintf(symboln, sizeof(symboln), "%sWindowPos3iARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos3i) {
        void ** procp = (void **) &disp->WindowPos3i;
        snprintf(symboln, sizeof(symboln), "%sWindowPos3iMESA", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos3iv) {
        void ** procp = (void **) &disp->WindowPos3iv;
        snprintf(symboln, sizeof(symboln), "%sWindowPos3iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos3iv) {
        void ** procp = (void **) &disp->WindowPos3iv;
        snprintf(symboln, sizeof(symboln), "%sWindowPos3ivARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos3iv) {
        void ** procp = (void **) &disp->WindowPos3iv;
        snprintf(symboln, sizeof(symboln), "%sWindowPos3ivMESA", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos3s) {
        void ** procp = (void **) &disp->WindowPos3s;
        snprintf(symboln, sizeof(symboln), "%sWindowPos3s", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos3s) {
        void ** procp = (void **) &disp->WindowPos3s;
        snprintf(symboln, sizeof(symboln), "%sWindowPos3sARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos3s) {
        void ** procp = (void **) &disp->WindowPos3s;
        snprintf(symboln, sizeof(symboln), "%sWindowPos3sMESA", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos3sv) {
        void ** procp = (void **) &disp->WindowPos3sv;
        snprintf(symboln, sizeof(symboln), "%sWindowPos3sv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos3sv) {
        void ** procp = (void **) &disp->WindowPos3sv;
        snprintf(symboln, sizeof(symboln), "%sWindowPos3svARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos3sv) {
        void ** procp = (void **) &disp->WindowPos3sv;
        snprintf(symboln, sizeof(symboln), "%sWindowPos3svMESA", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BeginQuery) {
        void ** procp = (void **) &disp->BeginQuery;
        snprintf(symboln, sizeof(symboln), "%sBeginQuery", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BeginQuery) {
        void ** procp = (void **) &disp->BeginQuery;
        snprintf(symboln, sizeof(symboln), "%sBeginQueryARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindBuffer) {
        void ** procp = (void **) &disp->BindBuffer;
        snprintf(symboln, sizeof(symboln), "%sBindBuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindBuffer) {
        void ** procp = (void **) &disp->BindBuffer;
        snprintf(symboln, sizeof(symboln), "%sBindBufferARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BufferData) {
        void ** procp = (void **) &disp->BufferData;
        snprintf(symboln, sizeof(symboln), "%sBufferData", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BufferData) {
        void ** procp = (void **) &disp->BufferData;
        snprintf(symboln, sizeof(symboln), "%sBufferDataARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BufferSubData) {
        void ** procp = (void **) &disp->BufferSubData;
        snprintf(symboln, sizeof(symboln), "%sBufferSubData", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BufferSubData) {
        void ** procp = (void **) &disp->BufferSubData;
        snprintf(symboln, sizeof(symboln), "%sBufferSubDataARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteBuffers) {
        void ** procp = (void **) &disp->DeleteBuffers;
        snprintf(symboln, sizeof(symboln), "%sDeleteBuffers", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteBuffers) {
        void ** procp = (void **) &disp->DeleteBuffers;
        snprintf(symboln, sizeof(symboln), "%sDeleteBuffersARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteQueries) {
        void ** procp = (void **) &disp->DeleteQueries;
        snprintf(symboln, sizeof(symboln), "%sDeleteQueries", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteQueries) {
        void ** procp = (void **) &disp->DeleteQueries;
        snprintf(symboln, sizeof(symboln), "%sDeleteQueriesARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EndQuery) {
        void ** procp = (void **) &disp->EndQuery;
        snprintf(symboln, sizeof(symboln), "%sEndQuery", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EndQuery) {
        void ** procp = (void **) &disp->EndQuery;
        snprintf(symboln, sizeof(symboln), "%sEndQueryARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenBuffers) {
        void ** procp = (void **) &disp->GenBuffers;
        snprintf(symboln, sizeof(symboln), "%sGenBuffers", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenBuffers) {
        void ** procp = (void **) &disp->GenBuffers;
        snprintf(symboln, sizeof(symboln), "%sGenBuffersARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenQueries) {
        void ** procp = (void **) &disp->GenQueries;
        snprintf(symboln, sizeof(symboln), "%sGenQueries", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenQueries) {
        void ** procp = (void **) &disp->GenQueries;
        snprintf(symboln, sizeof(symboln), "%sGenQueriesARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetBufferParameteriv) {
        void ** procp = (void **) &disp->GetBufferParameteriv;
        snprintf(symboln, sizeof(symboln), "%sGetBufferParameteriv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetBufferParameteriv) {
        void ** procp = (void **) &disp->GetBufferParameteriv;
        snprintf(symboln, sizeof(symboln), "%sGetBufferParameterivARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetBufferPointerv) {
        void ** procp = (void **) &disp->GetBufferPointerv;
        snprintf(symboln, sizeof(symboln), "%sGetBufferPointerv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetBufferPointerv) {
        void ** procp = (void **) &disp->GetBufferPointerv;
        snprintf(symboln, sizeof(symboln), "%sGetBufferPointervARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetBufferPointerv) {
        void ** procp = (void **) &disp->GetBufferPointerv;
        snprintf(symboln, sizeof(symboln), "%sGetBufferPointervOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetBufferSubData) {
        void ** procp = (void **) &disp->GetBufferSubData;
        snprintf(symboln, sizeof(symboln), "%sGetBufferSubData", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetBufferSubData) {
        void ** procp = (void **) &disp->GetBufferSubData;
        snprintf(symboln, sizeof(symboln), "%sGetBufferSubDataARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetQueryObjectiv) {
        void ** procp = (void **) &disp->GetQueryObjectiv;
        snprintf(symboln, sizeof(symboln), "%sGetQueryObjectiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetQueryObjectiv) {
        void ** procp = (void **) &disp->GetQueryObjectiv;
        snprintf(symboln, sizeof(symboln), "%sGetQueryObjectivARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetQueryObjectuiv) {
        void ** procp = (void **) &disp->GetQueryObjectuiv;
        snprintf(symboln, sizeof(symboln), "%sGetQueryObjectuiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetQueryObjectuiv) {
        void ** procp = (void **) &disp->GetQueryObjectuiv;
        snprintf(symboln, sizeof(symboln), "%sGetQueryObjectuivARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetQueryiv) {
        void ** procp = (void **) &disp->GetQueryiv;
        snprintf(symboln, sizeof(symboln), "%sGetQueryiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetQueryiv) {
        void ** procp = (void **) &disp->GetQueryiv;
        snprintf(symboln, sizeof(symboln), "%sGetQueryivARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsBuffer) {
        void ** procp = (void **) &disp->IsBuffer;
        snprintf(symboln, sizeof(symboln), "%sIsBuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsBuffer) {
        void ** procp = (void **) &disp->IsBuffer;
        snprintf(symboln, sizeof(symboln), "%sIsBufferARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsQuery) {
        void ** procp = (void **) &disp->IsQuery;
        snprintf(symboln, sizeof(symboln), "%sIsQuery", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsQuery) {
        void ** procp = (void **) &disp->IsQuery;
        snprintf(symboln, sizeof(symboln), "%sIsQueryARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MapBuffer) {
        void ** procp = (void **) &disp->MapBuffer;
        snprintf(symboln, sizeof(symboln), "%sMapBuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MapBuffer) {
        void ** procp = (void **) &disp->MapBuffer;
        snprintf(symboln, sizeof(symboln), "%sMapBufferARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MapBuffer) {
        void ** procp = (void **) &disp->MapBuffer;
        snprintf(symboln, sizeof(symboln), "%sMapBufferOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UnmapBuffer) {
        void ** procp = (void **) &disp->UnmapBuffer;
        snprintf(symboln, sizeof(symboln), "%sUnmapBuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UnmapBuffer) {
        void ** procp = (void **) &disp->UnmapBuffer;
        snprintf(symboln, sizeof(symboln), "%sUnmapBufferARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UnmapBuffer) {
        void ** procp = (void **) &disp->UnmapBuffer;
        snprintf(symboln, sizeof(symboln), "%sUnmapBufferOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->AttachShader) {
        void ** procp = (void **) &disp->AttachShader;
        snprintf(symboln, sizeof(symboln), "%sAttachShader", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindAttribLocation) {
        void ** procp = (void **) &disp->BindAttribLocation;
        snprintf(symboln, sizeof(symboln), "%sBindAttribLocation", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindAttribLocation) {
        void ** procp = (void **) &disp->BindAttribLocation;
        snprintf(symboln, sizeof(symboln), "%sBindAttribLocationARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlendEquationSeparate) {
        void ** procp = (void **) &disp->BlendEquationSeparate;
        snprintf(symboln, sizeof(symboln), "%sBlendEquationSeparate", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlendEquationSeparate) {
        void ** procp = (void **) &disp->BlendEquationSeparate;
        snprintf(symboln, sizeof(symboln), "%sBlendEquationSeparateEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlendEquationSeparate) {
        void ** procp = (void **) &disp->BlendEquationSeparate;
        snprintf(symboln, sizeof(symboln), "%sBlendEquationSeparateATI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlendEquationSeparate) {
        void ** procp = (void **) &disp->BlendEquationSeparate;
        snprintf(symboln, sizeof(symboln), "%sBlendEquationSeparateOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CompileShader) {
        void ** procp = (void **) &disp->CompileShader;
        snprintf(symboln, sizeof(symboln), "%sCompileShader", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CompileShader) {
        void ** procp = (void **) &disp->CompileShader;
        snprintf(symboln, sizeof(symboln), "%sCompileShaderARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CreateProgram) {
        void ** procp = (void **) &disp->CreateProgram;
        snprintf(symboln, sizeof(symboln), "%sCreateProgram", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CreateShader) {
        void ** procp = (void **) &disp->CreateShader;
        snprintf(symboln, sizeof(symboln), "%sCreateShader", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteProgram) {
        void ** procp = (void **) &disp->DeleteProgram;
        snprintf(symboln, sizeof(symboln), "%sDeleteProgram", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteShader) {
        void ** procp = (void **) &disp->DeleteShader;
        snprintf(symboln, sizeof(symboln), "%sDeleteShader", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DetachShader) {
        void ** procp = (void **) &disp->DetachShader;
        snprintf(symboln, sizeof(symboln), "%sDetachShader", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DisableVertexAttribArray) {
        void ** procp = (void **) &disp->DisableVertexAttribArray;
        snprintf(symboln, sizeof(symboln), "%sDisableVertexAttribArray", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DisableVertexAttribArray) {
        void ** procp = (void **) &disp->DisableVertexAttribArray;
        snprintf(symboln, sizeof(symboln), "%sDisableVertexAttribArrayARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawBuffers) {
        void ** procp = (void **) &disp->DrawBuffers;
        snprintf(symboln, sizeof(symboln), "%sDrawBuffers", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawBuffers) {
        void ** procp = (void **) &disp->DrawBuffers;
        snprintf(symboln, sizeof(symboln), "%sDrawBuffersARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawBuffers) {
        void ** procp = (void **) &disp->DrawBuffers;
        snprintf(symboln, sizeof(symboln), "%sDrawBuffersATI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawBuffers) {
        void ** procp = (void **) &disp->DrawBuffers;
        snprintf(symboln, sizeof(symboln), "%sDrawBuffersNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawBuffers) {
        void ** procp = (void **) &disp->DrawBuffers;
        snprintf(symboln, sizeof(symboln), "%sDrawBuffersEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EnableVertexAttribArray) {
        void ** procp = (void **) &disp->EnableVertexAttribArray;
        snprintf(symboln, sizeof(symboln), "%sEnableVertexAttribArray", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EnableVertexAttribArray) {
        void ** procp = (void **) &disp->EnableVertexAttribArray;
        snprintf(symboln, sizeof(symboln), "%sEnableVertexAttribArrayARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetActiveAttrib) {
        void ** procp = (void **) &disp->GetActiveAttrib;
        snprintf(symboln, sizeof(symboln), "%sGetActiveAttrib", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetActiveAttrib) {
        void ** procp = (void **) &disp->GetActiveAttrib;
        snprintf(symboln, sizeof(symboln), "%sGetActiveAttribARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetActiveUniform) {
        void ** procp = (void **) &disp->GetActiveUniform;
        snprintf(symboln, sizeof(symboln), "%sGetActiveUniform", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetActiveUniform) {
        void ** procp = (void **) &disp->GetActiveUniform;
        snprintf(symboln, sizeof(symboln), "%sGetActiveUniformARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetAttachedShaders) {
        void ** procp = (void **) &disp->GetAttachedShaders;
        snprintf(symboln, sizeof(symboln), "%sGetAttachedShaders", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetAttribLocation) {
        void ** procp = (void **) &disp->GetAttribLocation;
        snprintf(symboln, sizeof(symboln), "%sGetAttribLocation", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetAttribLocation) {
        void ** procp = (void **) &disp->GetAttribLocation;
        snprintf(symboln, sizeof(symboln), "%sGetAttribLocationARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetProgramInfoLog) {
        void ** procp = (void **) &disp->GetProgramInfoLog;
        snprintf(symboln, sizeof(symboln), "%sGetProgramInfoLog", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetProgramiv) {
        void ** procp = (void **) &disp->GetProgramiv;
        snprintf(symboln, sizeof(symboln), "%sGetProgramiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetShaderInfoLog) {
        void ** procp = (void **) &disp->GetShaderInfoLog;
        snprintf(symboln, sizeof(symboln), "%sGetShaderInfoLog", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetShaderSource) {
        void ** procp = (void **) &disp->GetShaderSource;
        snprintf(symboln, sizeof(symboln), "%sGetShaderSource", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetShaderSource) {
        void ** procp = (void **) &disp->GetShaderSource;
        snprintf(symboln, sizeof(symboln), "%sGetShaderSourceARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetShaderiv) {
        void ** procp = (void **) &disp->GetShaderiv;
        snprintf(symboln, sizeof(symboln), "%sGetShaderiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetUniformLocation) {
        void ** procp = (void **) &disp->GetUniformLocation;
        snprintf(symboln, sizeof(symboln), "%sGetUniformLocation", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetUniformLocation) {
        void ** procp = (void **) &disp->GetUniformLocation;
        snprintf(symboln, sizeof(symboln), "%sGetUniformLocationARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetUniformfv) {
        void ** procp = (void **) &disp->GetUniformfv;
        snprintf(symboln, sizeof(symboln), "%sGetUniformfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetUniformfv) {
        void ** procp = (void **) &disp->GetUniformfv;
        snprintf(symboln, sizeof(symboln), "%sGetUniformfvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetUniformiv) {
        void ** procp = (void **) &disp->GetUniformiv;
        snprintf(symboln, sizeof(symboln), "%sGetUniformiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetUniformiv) {
        void ** procp = (void **) &disp->GetUniformiv;
        snprintf(symboln, sizeof(symboln), "%sGetUniformivARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetVertexAttribPointerv) {
        void ** procp = (void **) &disp->GetVertexAttribPointerv;
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribPointerv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetVertexAttribPointerv) {
        void ** procp = (void **) &disp->GetVertexAttribPointerv;
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribPointervARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetVertexAttribPointerv) {
        void ** procp = (void **) &disp->GetVertexAttribPointerv;
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribPointervNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetVertexAttribdv) {
        void ** procp = (void **) &disp->GetVertexAttribdv;
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribdv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetVertexAttribdv) {
        void ** procp = (void **) &disp->GetVertexAttribdv;
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribdvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetVertexAttribfv) {
        void ** procp = (void **) &disp->GetVertexAttribfv;
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetVertexAttribfv) {
        void ** procp = (void **) &disp->GetVertexAttribfv;
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribfvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetVertexAttribiv) {
        void ** procp = (void **) &disp->GetVertexAttribiv;
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetVertexAttribiv) {
        void ** procp = (void **) &disp->GetVertexAttribiv;
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribivARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsProgram) {
        void ** procp = (void **) &disp->IsProgram;
        snprintf(symboln, sizeof(symboln), "%sIsProgram", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsShader) {
        void ** procp = (void **) &disp->IsShader;
        snprintf(symboln, sizeof(symboln), "%sIsShader", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LinkProgram) {
        void ** procp = (void **) &disp->LinkProgram;
        snprintf(symboln, sizeof(symboln), "%sLinkProgram", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LinkProgram) {
        void ** procp = (void **) &disp->LinkProgram;
        snprintf(symboln, sizeof(symboln), "%sLinkProgramARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ShaderSource) {
        void ** procp = (void **) &disp->ShaderSource;
        snprintf(symboln, sizeof(symboln), "%sShaderSource", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ShaderSource) {
        void ** procp = (void **) &disp->ShaderSource;
        snprintf(symboln, sizeof(symboln), "%sShaderSourceARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->StencilFuncSeparate) {
        void ** procp = (void **) &disp->StencilFuncSeparate;
        snprintf(symboln, sizeof(symboln), "%sStencilFuncSeparate", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->StencilMaskSeparate) {
        void ** procp = (void **) &disp->StencilMaskSeparate;
        snprintf(symboln, sizeof(symboln), "%sStencilMaskSeparate", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->StencilOpSeparate) {
        void ** procp = (void **) &disp->StencilOpSeparate;
        snprintf(symboln, sizeof(symboln), "%sStencilOpSeparate", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->StencilOpSeparate) {
        void ** procp = (void **) &disp->StencilOpSeparate;
        snprintf(symboln, sizeof(symboln), "%sStencilOpSeparateATI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform1f) {
        void ** procp = (void **) &disp->Uniform1f;
        snprintf(symboln, sizeof(symboln), "%sUniform1f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform1f) {
        void ** procp = (void **) &disp->Uniform1f;
        snprintf(symboln, sizeof(symboln), "%sUniform1fARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform1fv) {
        void ** procp = (void **) &disp->Uniform1fv;
        snprintf(symboln, sizeof(symboln), "%sUniform1fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform1fv) {
        void ** procp = (void **) &disp->Uniform1fv;
        snprintf(symboln, sizeof(symboln), "%sUniform1fvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform1i) {
        void ** procp = (void **) &disp->Uniform1i;
        snprintf(symboln, sizeof(symboln), "%sUniform1i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform1i) {
        void ** procp = (void **) &disp->Uniform1i;
        snprintf(symboln, sizeof(symboln), "%sUniform1iARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform1iv) {
        void ** procp = (void **) &disp->Uniform1iv;
        snprintf(symboln, sizeof(symboln), "%sUniform1iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform1iv) {
        void ** procp = (void **) &disp->Uniform1iv;
        snprintf(symboln, sizeof(symboln), "%sUniform1ivARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform2f) {
        void ** procp = (void **) &disp->Uniform2f;
        snprintf(symboln, sizeof(symboln), "%sUniform2f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform2f) {
        void ** procp = (void **) &disp->Uniform2f;
        snprintf(symboln, sizeof(symboln), "%sUniform2fARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform2fv) {
        void ** procp = (void **) &disp->Uniform2fv;
        snprintf(symboln, sizeof(symboln), "%sUniform2fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform2fv) {
        void ** procp = (void **) &disp->Uniform2fv;
        snprintf(symboln, sizeof(symboln), "%sUniform2fvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform2i) {
        void ** procp = (void **) &disp->Uniform2i;
        snprintf(symboln, sizeof(symboln), "%sUniform2i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform2i) {
        void ** procp = (void **) &disp->Uniform2i;
        snprintf(symboln, sizeof(symboln), "%sUniform2iARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform2iv) {
        void ** procp = (void **) &disp->Uniform2iv;
        snprintf(symboln, sizeof(symboln), "%sUniform2iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform2iv) {
        void ** procp = (void **) &disp->Uniform2iv;
        snprintf(symboln, sizeof(symboln), "%sUniform2ivARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform3f) {
        void ** procp = (void **) &disp->Uniform3f;
        snprintf(symboln, sizeof(symboln), "%sUniform3f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform3f) {
        void ** procp = (void **) &disp->Uniform3f;
        snprintf(symboln, sizeof(symboln), "%sUniform3fARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform3fv) {
        void ** procp = (void **) &disp->Uniform3fv;
        snprintf(symboln, sizeof(symboln), "%sUniform3fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform3fv) {
        void ** procp = (void **) &disp->Uniform3fv;
        snprintf(symboln, sizeof(symboln), "%sUniform3fvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform3i) {
        void ** procp = (void **) &disp->Uniform3i;
        snprintf(symboln, sizeof(symboln), "%sUniform3i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform3i) {
        void ** procp = (void **) &disp->Uniform3i;
        snprintf(symboln, sizeof(symboln), "%sUniform3iARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform3iv) {
        void ** procp = (void **) &disp->Uniform3iv;
        snprintf(symboln, sizeof(symboln), "%sUniform3iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform3iv) {
        void ** procp = (void **) &disp->Uniform3iv;
        snprintf(symboln, sizeof(symboln), "%sUniform3ivARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform4f) {
        void ** procp = (void **) &disp->Uniform4f;
        snprintf(symboln, sizeof(symboln), "%sUniform4f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform4f) {
        void ** procp = (void **) &disp->Uniform4f;
        snprintf(symboln, sizeof(symboln), "%sUniform4fARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform4fv) {
        void ** procp = (void **) &disp->Uniform4fv;
        snprintf(symboln, sizeof(symboln), "%sUniform4fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform4fv) {
        void ** procp = (void **) &disp->Uniform4fv;
        snprintf(symboln, sizeof(symboln), "%sUniform4fvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform4i) {
        void ** procp = (void **) &disp->Uniform4i;
        snprintf(symboln, sizeof(symboln), "%sUniform4i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform4i) {
        void ** procp = (void **) &disp->Uniform4i;
        snprintf(symboln, sizeof(symboln), "%sUniform4iARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform4iv) {
        void ** procp = (void **) &disp->Uniform4iv;
        snprintf(symboln, sizeof(symboln), "%sUniform4iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform4iv) {
        void ** procp = (void **) &disp->Uniform4iv;
        snprintf(symboln, sizeof(symboln), "%sUniform4ivARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UniformMatrix2fv) {
        void ** procp = (void **) &disp->UniformMatrix2fv;
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix2fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UniformMatrix2fv) {
        void ** procp = (void **) &disp->UniformMatrix2fv;
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix2fvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UniformMatrix3fv) {
        void ** procp = (void **) &disp->UniformMatrix3fv;
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix3fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UniformMatrix3fv) {
        void ** procp = (void **) &disp->UniformMatrix3fv;
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix3fvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UniformMatrix4fv) {
        void ** procp = (void **) &disp->UniformMatrix4fv;
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix4fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UniformMatrix4fv) {
        void ** procp = (void **) &disp->UniformMatrix4fv;
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix4fvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UseProgram) {
        void ** procp = (void **) &disp->UseProgram;
        snprintf(symboln, sizeof(symboln), "%sUseProgram", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UseProgram) {
        void ** procp = (void **) &disp->UseProgram;
        snprintf(symboln, sizeof(symboln), "%sUseProgramObjectARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ValidateProgram) {
        void ** procp = (void **) &disp->ValidateProgram;
        snprintf(symboln, sizeof(symboln), "%sValidateProgram", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ValidateProgram) {
        void ** procp = (void **) &disp->ValidateProgram;
        snprintf(symboln, sizeof(symboln), "%sValidateProgramARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib1d) {
        void ** procp = (void **) &disp->VertexAttrib1d;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib1d) {
        void ** procp = (void **) &disp->VertexAttrib1d;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1dARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib1dv) {
        void ** procp = (void **) &disp->VertexAttrib1dv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib1dv) {
        void ** procp = (void **) &disp->VertexAttrib1dv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1dvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib1s) {
        void ** procp = (void **) &disp->VertexAttrib1s;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1s", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib1s) {
        void ** procp = (void **) &disp->VertexAttrib1s;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1sARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib1sv) {
        void ** procp = (void **) &disp->VertexAttrib1sv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1sv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib1sv) {
        void ** procp = (void **) &disp->VertexAttrib1sv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1svARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib2d) {
        void ** procp = (void **) &disp->VertexAttrib2d;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib2d) {
        void ** procp = (void **) &disp->VertexAttrib2d;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2dARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib2dv) {
        void ** procp = (void **) &disp->VertexAttrib2dv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib2dv) {
        void ** procp = (void **) &disp->VertexAttrib2dv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2dvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib2s) {
        void ** procp = (void **) &disp->VertexAttrib2s;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2s", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib2s) {
        void ** procp = (void **) &disp->VertexAttrib2s;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2sARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib2sv) {
        void ** procp = (void **) &disp->VertexAttrib2sv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2sv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib2sv) {
        void ** procp = (void **) &disp->VertexAttrib2sv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2svARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib3d) {
        void ** procp = (void **) &disp->VertexAttrib3d;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib3d) {
        void ** procp = (void **) &disp->VertexAttrib3d;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3dARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib3dv) {
        void ** procp = (void **) &disp->VertexAttrib3dv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib3dv) {
        void ** procp = (void **) &disp->VertexAttrib3dv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3dvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib3s) {
        void ** procp = (void **) &disp->VertexAttrib3s;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3s", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib3s) {
        void ** procp = (void **) &disp->VertexAttrib3s;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3sARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib3sv) {
        void ** procp = (void **) &disp->VertexAttrib3sv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3sv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib3sv) {
        void ** procp = (void **) &disp->VertexAttrib3sv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3svARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4Nbv) {
        void ** procp = (void **) &disp->VertexAttrib4Nbv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4Nbv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4Nbv) {
        void ** procp = (void **) &disp->VertexAttrib4Nbv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4NbvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4Niv) {
        void ** procp = (void **) &disp->VertexAttrib4Niv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4Niv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4Niv) {
        void ** procp = (void **) &disp->VertexAttrib4Niv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4NivARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4Nsv) {
        void ** procp = (void **) &disp->VertexAttrib4Nsv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4Nsv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4Nsv) {
        void ** procp = (void **) &disp->VertexAttrib4Nsv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4NsvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4Nub) {
        void ** procp = (void **) &disp->VertexAttrib4Nub;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4Nub", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4Nub) {
        void ** procp = (void **) &disp->VertexAttrib4Nub;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4NubARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4Nubv) {
        void ** procp = (void **) &disp->VertexAttrib4Nubv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4Nubv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4Nubv) {
        void ** procp = (void **) &disp->VertexAttrib4Nubv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4NubvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4Nuiv) {
        void ** procp = (void **) &disp->VertexAttrib4Nuiv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4Nuiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4Nuiv) {
        void ** procp = (void **) &disp->VertexAttrib4Nuiv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4NuivARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4Nusv) {
        void ** procp = (void **) &disp->VertexAttrib4Nusv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4Nusv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4Nusv) {
        void ** procp = (void **) &disp->VertexAttrib4Nusv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4NusvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4bv) {
        void ** procp = (void **) &disp->VertexAttrib4bv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4bv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4bv) {
        void ** procp = (void **) &disp->VertexAttrib4bv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4bvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4d) {
        void ** procp = (void **) &disp->VertexAttrib4d;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4d) {
        void ** procp = (void **) &disp->VertexAttrib4d;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4dARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4dv) {
        void ** procp = (void **) &disp->VertexAttrib4dv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4dv) {
        void ** procp = (void **) &disp->VertexAttrib4dv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4dvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4iv) {
        void ** procp = (void **) &disp->VertexAttrib4iv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4iv) {
        void ** procp = (void **) &disp->VertexAttrib4iv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4ivARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4s) {
        void ** procp = (void **) &disp->VertexAttrib4s;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4s", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4s) {
        void ** procp = (void **) &disp->VertexAttrib4s;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4sARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4sv) {
        void ** procp = (void **) &disp->VertexAttrib4sv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4sv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4sv) {
        void ** procp = (void **) &disp->VertexAttrib4sv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4svARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4ubv) {
        void ** procp = (void **) &disp->VertexAttrib4ubv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4ubv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4ubv) {
        void ** procp = (void **) &disp->VertexAttrib4ubv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4ubvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4uiv) {
        void ** procp = (void **) &disp->VertexAttrib4uiv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4uiv) {
        void ** procp = (void **) &disp->VertexAttrib4uiv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4uivARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4usv) {
        void ** procp = (void **) &disp->VertexAttrib4usv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4usv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4usv) {
        void ** procp = (void **) &disp->VertexAttrib4usv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4usvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribPointer) {
        void ** procp = (void **) &disp->VertexAttribPointer;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribPointer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribPointer) {
        void ** procp = (void **) &disp->VertexAttribPointer;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribPointerARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UniformMatrix2x3fv) {
        void ** procp = (void **) &disp->UniformMatrix2x3fv;
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix2x3fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UniformMatrix2x4fv) {
        void ** procp = (void **) &disp->UniformMatrix2x4fv;
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix2x4fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UniformMatrix3x2fv) {
        void ** procp = (void **) &disp->UniformMatrix3x2fv;
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix3x2fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UniformMatrix3x4fv) {
        void ** procp = (void **) &disp->UniformMatrix3x4fv;
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix3x4fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UniformMatrix4x2fv) {
        void ** procp = (void **) &disp->UniformMatrix4x2fv;
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix4x2fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UniformMatrix4x3fv) {
        void ** procp = (void **) &disp->UniformMatrix4x3fv;
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix4x3fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BeginConditionalRender) {
        void ** procp = (void **) &disp->BeginConditionalRender;
        snprintf(symboln, sizeof(symboln), "%sBeginConditionalRender", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BeginConditionalRender) {
        void ** procp = (void **) &disp->BeginConditionalRender;
        snprintf(symboln, sizeof(symboln), "%sBeginConditionalRenderNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BeginTransformFeedback) {
        void ** procp = (void **) &disp->BeginTransformFeedback;
        snprintf(symboln, sizeof(symboln), "%sBeginTransformFeedback", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BeginTransformFeedback) {
        void ** procp = (void **) &disp->BeginTransformFeedback;
        snprintf(symboln, sizeof(symboln), "%sBeginTransformFeedbackEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindBufferBase) {
        void ** procp = (void **) &disp->BindBufferBase;
        snprintf(symboln, sizeof(symboln), "%sBindBufferBase", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindBufferBase) {
        void ** procp = (void **) &disp->BindBufferBase;
        snprintf(symboln, sizeof(symboln), "%sBindBufferBaseEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindBufferRange) {
        void ** procp = (void **) &disp->BindBufferRange;
        snprintf(symboln, sizeof(symboln), "%sBindBufferRange", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindBufferRange) {
        void ** procp = (void **) &disp->BindBufferRange;
        snprintf(symboln, sizeof(symboln), "%sBindBufferRangeEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindFragDataLocation) {
        void ** procp = (void **) &disp->BindFragDataLocation;
        snprintf(symboln, sizeof(symboln), "%sBindFragDataLocationEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindFragDataLocation) {
        void ** procp = (void **) &disp->BindFragDataLocation;
        snprintf(symboln, sizeof(symboln), "%sBindFragDataLocation", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClampColor) {
        void ** procp = (void **) &disp->ClampColor;
        snprintf(symboln, sizeof(symboln), "%sClampColorARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClampColor) {
        void ** procp = (void **) &disp->ClampColor;
        snprintf(symboln, sizeof(symboln), "%sClampColor", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearBufferfi) {
        void ** procp = (void **) &disp->ClearBufferfi;
        snprintf(symboln, sizeof(symboln), "%sClearBufferfi", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearBufferfv) {
        void ** procp = (void **) &disp->ClearBufferfv;
        snprintf(symboln, sizeof(symboln), "%sClearBufferfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearBufferiv) {
        void ** procp = (void **) &disp->ClearBufferiv;
        snprintf(symboln, sizeof(symboln), "%sClearBufferiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearBufferuiv) {
        void ** procp = (void **) &disp->ClearBufferuiv;
        snprintf(symboln, sizeof(symboln), "%sClearBufferuiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ColorMaski) {
        void ** procp = (void **) &disp->ColorMaski;
        snprintf(symboln, sizeof(symboln), "%sColorMaskIndexedEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ColorMaski) {
        void ** procp = (void **) &disp->ColorMaski;
        snprintf(symboln, sizeof(symboln), "%sColorMaski", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Disablei) {
        void ** procp = (void **) &disp->Disablei;
        snprintf(symboln, sizeof(symboln), "%sDisableIndexedEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Disablei) {
        void ** procp = (void **) &disp->Disablei;
        snprintf(symboln, sizeof(symboln), "%sDisablei", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Enablei) {
        void ** procp = (void **) &disp->Enablei;
        snprintf(symboln, sizeof(symboln), "%sEnableIndexedEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Enablei) {
        void ** procp = (void **) &disp->Enablei;
        snprintf(symboln, sizeof(symboln), "%sEnablei", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EndConditionalRender) {
        void ** procp = (void **) &disp->EndConditionalRender;
        snprintf(symboln, sizeof(symboln), "%sEndConditionalRender", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EndConditionalRender) {
        void ** procp = (void **) &disp->EndConditionalRender;
        snprintf(symboln, sizeof(symboln), "%sEndConditionalRenderNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EndTransformFeedback) {
        void ** procp = (void **) &disp->EndTransformFeedback;
        snprintf(symboln, sizeof(symboln), "%sEndTransformFeedback", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EndTransformFeedback) {
        void ** procp = (void **) &disp->EndTransformFeedback;
        snprintf(symboln, sizeof(symboln), "%sEndTransformFeedbackEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetBooleani_v) {
        void ** procp = (void **) &disp->GetBooleani_v;
        snprintf(symboln, sizeof(symboln), "%sGetBooleanIndexedvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetBooleani_v) {
        void ** procp = (void **) &disp->GetBooleani_v;
        snprintf(symboln, sizeof(symboln), "%sGetBooleani_v", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetFragDataLocation) {
        void ** procp = (void **) &disp->GetFragDataLocation;
        snprintf(symboln, sizeof(symboln), "%sGetFragDataLocationEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetFragDataLocation) {
        void ** procp = (void **) &disp->GetFragDataLocation;
        snprintf(symboln, sizeof(symboln), "%sGetFragDataLocation", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetIntegeri_v) {
        void ** procp = (void **) &disp->GetIntegeri_v;
        snprintf(symboln, sizeof(symboln), "%sGetIntegerIndexedvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetIntegeri_v) {
        void ** procp = (void **) &disp->GetIntegeri_v;
        snprintf(symboln, sizeof(symboln), "%sGetIntegeri_v", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetStringi) {
        void ** procp = (void **) &disp->GetStringi;
        snprintf(symboln, sizeof(symboln), "%sGetStringi", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTexParameterIiv) {
        void ** procp = (void **) &disp->GetTexParameterIiv;
        snprintf(symboln, sizeof(symboln), "%sGetTexParameterIivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTexParameterIiv) {
        void ** procp = (void **) &disp->GetTexParameterIiv;
        snprintf(symboln, sizeof(symboln), "%sGetTexParameterIiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTexParameterIuiv) {
        void ** procp = (void **) &disp->GetTexParameterIuiv;
        snprintf(symboln, sizeof(symboln), "%sGetTexParameterIuivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTexParameterIuiv) {
        void ** procp = (void **) &disp->GetTexParameterIuiv;
        snprintf(symboln, sizeof(symboln), "%sGetTexParameterIuiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTransformFeedbackVarying) {
        void ** procp = (void **) &disp->GetTransformFeedbackVarying;
        snprintf(symboln, sizeof(symboln), "%sGetTransformFeedbackVarying", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTransformFeedbackVarying) {
        void ** procp = (void **) &disp->GetTransformFeedbackVarying;
        snprintf(symboln, sizeof(symboln), "%sGetTransformFeedbackVaryingEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetUniformuiv) {
        void ** procp = (void **) &disp->GetUniformuiv;
        snprintf(symboln, sizeof(symboln), "%sGetUniformuivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetUniformuiv) {
        void ** procp = (void **) &disp->GetUniformuiv;
        snprintf(symboln, sizeof(symboln), "%sGetUniformuiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetVertexAttribIiv) {
        void ** procp = (void **) &disp->GetVertexAttribIiv;
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribIivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetVertexAttribIiv) {
        void ** procp = (void **) &disp->GetVertexAttribIiv;
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribIiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetVertexAttribIuiv) {
        void ** procp = (void **) &disp->GetVertexAttribIuiv;
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribIuivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetVertexAttribIuiv) {
        void ** procp = (void **) &disp->GetVertexAttribIuiv;
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribIuiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsEnabledi) {
        void ** procp = (void **) &disp->IsEnabledi;
        snprintf(symboln, sizeof(symboln), "%sIsEnabledIndexedEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsEnabledi) {
        void ** procp = (void **) &disp->IsEnabledi;
        snprintf(symboln, sizeof(symboln), "%sIsEnabledi", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexParameterIiv) {
        void ** procp = (void **) &disp->TexParameterIiv;
        snprintf(symboln, sizeof(symboln), "%sTexParameterIivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexParameterIiv) {
        void ** procp = (void **) &disp->TexParameterIiv;
        snprintf(symboln, sizeof(symboln), "%sTexParameterIiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexParameterIuiv) {
        void ** procp = (void **) &disp->TexParameterIuiv;
        snprintf(symboln, sizeof(symboln), "%sTexParameterIuivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexParameterIuiv) {
        void ** procp = (void **) &disp->TexParameterIuiv;
        snprintf(symboln, sizeof(symboln), "%sTexParameterIuiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TransformFeedbackVaryings) {
        void ** procp = (void **) &disp->TransformFeedbackVaryings;
        snprintf(symboln, sizeof(symboln), "%sTransformFeedbackVaryings", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TransformFeedbackVaryings) {
        void ** procp = (void **) &disp->TransformFeedbackVaryings;
        snprintf(symboln, sizeof(symboln), "%sTransformFeedbackVaryingsEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform1ui) {
        void ** procp = (void **) &disp->Uniform1ui;
        snprintf(symboln, sizeof(symboln), "%sUniform1uiEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform1ui) {
        void ** procp = (void **) &disp->Uniform1ui;
        snprintf(symboln, sizeof(symboln), "%sUniform1ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform1uiv) {
        void ** procp = (void **) &disp->Uniform1uiv;
        snprintf(symboln, sizeof(symboln), "%sUniform1uivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform1uiv) {
        void ** procp = (void **) &disp->Uniform1uiv;
        snprintf(symboln, sizeof(symboln), "%sUniform1uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform2ui) {
        void ** procp = (void **) &disp->Uniform2ui;
        snprintf(symboln, sizeof(symboln), "%sUniform2uiEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform2ui) {
        void ** procp = (void **) &disp->Uniform2ui;
        snprintf(symboln, sizeof(symboln), "%sUniform2ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform2uiv) {
        void ** procp = (void **) &disp->Uniform2uiv;
        snprintf(symboln, sizeof(symboln), "%sUniform2uivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform2uiv) {
        void ** procp = (void **) &disp->Uniform2uiv;
        snprintf(symboln, sizeof(symboln), "%sUniform2uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform3ui) {
        void ** procp = (void **) &disp->Uniform3ui;
        snprintf(symboln, sizeof(symboln), "%sUniform3uiEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform3ui) {
        void ** procp = (void **) &disp->Uniform3ui;
        snprintf(symboln, sizeof(symboln), "%sUniform3ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform3uiv) {
        void ** procp = (void **) &disp->Uniform3uiv;
        snprintf(symboln, sizeof(symboln), "%sUniform3uivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform3uiv) {
        void ** procp = (void **) &disp->Uniform3uiv;
        snprintf(symboln, sizeof(symboln), "%sUniform3uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform4ui) {
        void ** procp = (void **) &disp->Uniform4ui;
        snprintf(symboln, sizeof(symboln), "%sUniform4uiEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform4ui) {
        void ** procp = (void **) &disp->Uniform4ui;
        snprintf(symboln, sizeof(symboln), "%sUniform4ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform4uiv) {
        void ** procp = (void **) &disp->Uniform4uiv;
        snprintf(symboln, sizeof(symboln), "%sUniform4uivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform4uiv) {
        void ** procp = (void **) &disp->Uniform4uiv;
        snprintf(symboln, sizeof(symboln), "%sUniform4uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI1iv) {
        void ** procp = (void **) &disp->VertexAttribI1iv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI1ivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI1iv) {
        void ** procp = (void **) &disp->VertexAttribI1iv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI1iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI1uiv) {
        void ** procp = (void **) &disp->VertexAttribI1uiv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI1uivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI1uiv) {
        void ** procp = (void **) &disp->VertexAttribI1uiv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI1uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI4bv) {
        void ** procp = (void **) &disp->VertexAttribI4bv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI4bvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI4bv) {
        void ** procp = (void **) &disp->VertexAttribI4bv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI4bv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI4sv) {
        void ** procp = (void **) &disp->VertexAttribI4sv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI4svEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI4sv) {
        void ** procp = (void **) &disp->VertexAttribI4sv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI4sv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI4ubv) {
        void ** procp = (void **) &disp->VertexAttribI4ubv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI4ubvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI4ubv) {
        void ** procp = (void **) &disp->VertexAttribI4ubv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI4ubv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI4usv) {
        void ** procp = (void **) &disp->VertexAttribI4usv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI4usvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI4usv) {
        void ** procp = (void **) &disp->VertexAttribI4usv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI4usv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribIPointer) {
        void ** procp = (void **) &disp->VertexAttribIPointer;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribIPointerEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribIPointer) {
        void ** procp = (void **) &disp->VertexAttribIPointer;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribIPointer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PrimitiveRestartIndex) {
        void ** procp = (void **) &disp->PrimitiveRestartIndex;
        snprintf(symboln, sizeof(symboln), "%sPrimitiveRestartIndex", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PrimitiveRestartIndex) {
        void ** procp = (void **) &disp->PrimitiveRestartIndex;
        snprintf(symboln, sizeof(symboln), "%sPrimitiveRestartIndexNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexBuffer) {
        void ** procp = (void **) &disp->TexBuffer;
        snprintf(symboln, sizeof(symboln), "%sTexBufferARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexBuffer) {
        void ** procp = (void **) &disp->TexBuffer;
        snprintf(symboln, sizeof(symboln), "%sTexBuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FramebufferTexture) {
        void ** procp = (void **) &disp->FramebufferTexture;
        snprintf(symboln, sizeof(symboln), "%sFramebufferTextureARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FramebufferTexture) {
        void ** procp = (void **) &disp->FramebufferTexture;
        snprintf(symboln, sizeof(symboln), "%sFramebufferTexture", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetBufferParameteri64v) {
        void ** procp = (void **) &disp->GetBufferParameteri64v;
        snprintf(symboln, sizeof(symboln), "%sGetBufferParameteri64v", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetInteger64i_v) {
        void ** procp = (void **) &disp->GetInteger64i_v;
        snprintf(symboln, sizeof(symboln), "%sGetInteger64i_v", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribDivisor) {
        void ** procp = (void **) &disp->VertexAttribDivisor;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribDivisorARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribDivisor) {
        void ** procp = (void **) &disp->VertexAttribDivisor;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribDivisor", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MinSampleShading) {
        void ** procp = (void **) &disp->MinSampleShading;
        snprintf(symboln, sizeof(symboln), "%sMinSampleShadingARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MinSampleShading) {
        void ** procp = (void **) &disp->MinSampleShading;
        snprintf(symboln, sizeof(symboln), "%sMinSampleShading", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindProgramARB) {
        void ** procp = (void **) &disp->BindProgramARB;
        snprintf(symboln, sizeof(symboln), "%sBindProgramARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindProgramARB) {
        void ** procp = (void **) &disp->BindProgramARB;
        snprintf(symboln, sizeof(symboln), "%sBindProgramNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteProgramsARB) {
        void ** procp = (void **) &disp->DeleteProgramsARB;
        snprintf(symboln, sizeof(symboln), "%sDeleteProgramsARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteProgramsARB) {
        void ** procp = (void **) &disp->DeleteProgramsARB;
        snprintf(symboln, sizeof(symboln), "%sDeleteProgramsNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenProgramsARB) {
        void ** procp = (void **) &disp->GenProgramsARB;
        snprintf(symboln, sizeof(symboln), "%sGenProgramsARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenProgramsARB) {
        void ** procp = (void **) &disp->GenProgramsARB;
        snprintf(symboln, sizeof(symboln), "%sGenProgramsNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetProgramEnvParameterdvARB) {
        void ** procp = (void **) &disp->GetProgramEnvParameterdvARB;
        snprintf(symboln, sizeof(symboln), "%sGetProgramEnvParameterdvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetProgramEnvParameterfvARB) {
        void ** procp = (void **) &disp->GetProgramEnvParameterfvARB;
        snprintf(symboln, sizeof(symboln), "%sGetProgramEnvParameterfvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetProgramLocalParameterdvARB) {
        void ** procp = (void **) &disp->GetProgramLocalParameterdvARB;
        snprintf(symboln, sizeof(symboln), "%sGetProgramLocalParameterdvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetProgramLocalParameterfvARB) {
        void ** procp = (void **) &disp->GetProgramLocalParameterfvARB;
        snprintf(symboln, sizeof(symboln), "%sGetProgramLocalParameterfvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetProgramStringARB) {
        void ** procp = (void **) &disp->GetProgramStringARB;
        snprintf(symboln, sizeof(symboln), "%sGetProgramStringARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetProgramivARB) {
        void ** procp = (void **) &disp->GetProgramivARB;
        snprintf(symboln, sizeof(symboln), "%sGetProgramivARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsProgramARB) {
        void ** procp = (void **) &disp->IsProgramARB;
        snprintf(symboln, sizeof(symboln), "%sIsProgramARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsProgramARB) {
        void ** procp = (void **) &disp->IsProgramARB;
        snprintf(symboln, sizeof(symboln), "%sIsProgramNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramEnvParameter4dARB) {
        void ** procp = (void **) &disp->ProgramEnvParameter4dARB;
        snprintf(symboln, sizeof(symboln), "%sProgramEnvParameter4dARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramEnvParameter4dARB) {
        void ** procp = (void **) &disp->ProgramEnvParameter4dARB;
        snprintf(symboln, sizeof(symboln), "%sProgramParameter4dNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramEnvParameter4dvARB) {
        void ** procp = (void **) &disp->ProgramEnvParameter4dvARB;
        snprintf(symboln, sizeof(symboln), "%sProgramEnvParameter4dvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramEnvParameter4dvARB) {
        void ** procp = (void **) &disp->ProgramEnvParameter4dvARB;
        snprintf(symboln, sizeof(symboln), "%sProgramParameter4dvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramEnvParameter4fARB) {
        void ** procp = (void **) &disp->ProgramEnvParameter4fARB;
        snprintf(symboln, sizeof(symboln), "%sProgramEnvParameter4fARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramEnvParameter4fARB) {
        void ** procp = (void **) &disp->ProgramEnvParameter4fARB;
        snprintf(symboln, sizeof(symboln), "%sProgramParameter4fNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramEnvParameter4fvARB) {
        void ** procp = (void **) &disp->ProgramEnvParameter4fvARB;
        snprintf(symboln, sizeof(symboln), "%sProgramEnvParameter4fvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramEnvParameter4fvARB) {
        void ** procp = (void **) &disp->ProgramEnvParameter4fvARB;
        snprintf(symboln, sizeof(symboln), "%sProgramParameter4fvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramLocalParameter4dARB) {
        void ** procp = (void **) &disp->ProgramLocalParameter4dARB;
        snprintf(symboln, sizeof(symboln), "%sProgramLocalParameter4dARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramLocalParameter4dvARB) {
        void ** procp = (void **) &disp->ProgramLocalParameter4dvARB;
        snprintf(symboln, sizeof(symboln), "%sProgramLocalParameter4dvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramLocalParameter4fARB) {
        void ** procp = (void **) &disp->ProgramLocalParameter4fARB;
        snprintf(symboln, sizeof(symboln), "%sProgramLocalParameter4fARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramLocalParameter4fvARB) {
        void ** procp = (void **) &disp->ProgramLocalParameter4fvARB;
        snprintf(symboln, sizeof(symboln), "%sProgramLocalParameter4fvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramStringARB) {
        void ** procp = (void **) &disp->ProgramStringARB;
        snprintf(symboln, sizeof(symboln), "%sProgramStringARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib1fARB) {
        void ** procp = (void **) &disp->VertexAttrib1fARB;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib1fARB) {
        void ** procp = (void **) &disp->VertexAttrib1fARB;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1fARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib1fvARB) {
        void ** procp = (void **) &disp->VertexAttrib1fvARB;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib1fvARB) {
        void ** procp = (void **) &disp->VertexAttrib1fvARB;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1fvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib2fARB) {
        void ** procp = (void **) &disp->VertexAttrib2fARB;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib2fARB) {
        void ** procp = (void **) &disp->VertexAttrib2fARB;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2fARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib2fvARB) {
        void ** procp = (void **) &disp->VertexAttrib2fvARB;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib2fvARB) {
        void ** procp = (void **) &disp->VertexAttrib2fvARB;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2fvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib3fARB) {
        void ** procp = (void **) &disp->VertexAttrib3fARB;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib3fARB) {
        void ** procp = (void **) &disp->VertexAttrib3fARB;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3fARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib3fvARB) {
        void ** procp = (void **) &disp->VertexAttrib3fvARB;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib3fvARB) {
        void ** procp = (void **) &disp->VertexAttrib3fvARB;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3fvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4fARB) {
        void ** procp = (void **) &disp->VertexAttrib4fARB;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4fARB) {
        void ** procp = (void **) &disp->VertexAttrib4fARB;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4fARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4fvARB) {
        void ** procp = (void **) &disp->VertexAttrib4fvARB;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4fvARB) {
        void ** procp = (void **) &disp->VertexAttrib4fvARB;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4fvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->AttachObjectARB) {
        void ** procp = (void **) &disp->AttachObjectARB;
        snprintf(symboln, sizeof(symboln), "%sAttachObjectARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CreateProgramObjectARB) {
        void ** procp = (void **) &disp->CreateProgramObjectARB;
        snprintf(symboln, sizeof(symboln), "%sCreateProgramObjectARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CreateShaderObjectARB) {
        void ** procp = (void **) &disp->CreateShaderObjectARB;
        snprintf(symboln, sizeof(symboln), "%sCreateShaderObjectARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteObjectARB) {
        void ** procp = (void **) &disp->DeleteObjectARB;
        snprintf(symboln, sizeof(symboln), "%sDeleteObjectARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DetachObjectARB) {
        void ** procp = (void **) &disp->DetachObjectARB;
        snprintf(symboln, sizeof(symboln), "%sDetachObjectARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetAttachedObjectsARB) {
        void ** procp = (void **) &disp->GetAttachedObjectsARB;
        snprintf(symboln, sizeof(symboln), "%sGetAttachedObjectsARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetHandleARB) {
        void ** procp = (void **) &disp->GetHandleARB;
        snprintf(symboln, sizeof(symboln), "%sGetHandleARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetInfoLogARB) {
        void ** procp = (void **) &disp->GetInfoLogARB;
        snprintf(symboln, sizeof(symboln), "%sGetInfoLogARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetObjectParameterfvARB) {
        void ** procp = (void **) &disp->GetObjectParameterfvARB;
        snprintf(symboln, sizeof(symboln), "%sGetObjectParameterfvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetObjectParameterivARB) {
        void ** procp = (void **) &disp->GetObjectParameterivARB;
        snprintf(symboln, sizeof(symboln), "%sGetObjectParameterivARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawArraysInstancedARB) {
        void ** procp = (void **) &disp->DrawArraysInstancedARB;
        snprintf(symboln, sizeof(symboln), "%sDrawArraysInstancedARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawArraysInstancedARB) {
        void ** procp = (void **) &disp->DrawArraysInstancedARB;
        snprintf(symboln, sizeof(symboln), "%sDrawArraysInstancedEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawArraysInstancedARB) {
        void ** procp = (void **) &disp->DrawArraysInstancedARB;
        snprintf(symboln, sizeof(symboln), "%sDrawArraysInstanced", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawElementsInstancedARB) {
        void ** procp = (void **) &disp->DrawElementsInstancedARB;
        snprintf(symboln, sizeof(symboln), "%sDrawElementsInstancedARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawElementsInstancedARB) {
        void ** procp = (void **) &disp->DrawElementsInstancedARB;
        snprintf(symboln, sizeof(symboln), "%sDrawElementsInstancedEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawElementsInstancedARB) {
        void ** procp = (void **) &disp->DrawElementsInstancedARB;
        snprintf(symboln, sizeof(symboln), "%sDrawElementsInstanced", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindFramebuffer) {
        void ** procp = (void **) &disp->BindFramebuffer;
        snprintf(symboln, sizeof(symboln), "%sBindFramebuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindFramebuffer) {
        void ** procp = (void **) &disp->BindFramebuffer;
        snprintf(symboln, sizeof(symboln), "%sBindFramebufferOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindRenderbuffer) {
        void ** procp = (void **) &disp->BindRenderbuffer;
        snprintf(symboln, sizeof(symboln), "%sBindRenderbuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindRenderbuffer) {
        void ** procp = (void **) &disp->BindRenderbuffer;
        snprintf(symboln, sizeof(symboln), "%sBindRenderbufferOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlitFramebuffer) {
        void ** procp = (void **) &disp->BlitFramebuffer;
        snprintf(symboln, sizeof(symboln), "%sBlitFramebuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlitFramebuffer) {
        void ** procp = (void **) &disp->BlitFramebuffer;
        snprintf(symboln, sizeof(symboln), "%sBlitFramebufferEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CheckFramebufferStatus) {
        void ** procp = (void **) &disp->CheckFramebufferStatus;
        snprintf(symboln, sizeof(symboln), "%sCheckFramebufferStatus", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CheckFramebufferStatus) {
        void ** procp = (void **) &disp->CheckFramebufferStatus;
        snprintf(symboln, sizeof(symboln), "%sCheckFramebufferStatusEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CheckFramebufferStatus) {
        void ** procp = (void **) &disp->CheckFramebufferStatus;
        snprintf(symboln, sizeof(symboln), "%sCheckFramebufferStatusOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteFramebuffers) {
        void ** procp = (void **) &disp->DeleteFramebuffers;
        snprintf(symboln, sizeof(symboln), "%sDeleteFramebuffers", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteFramebuffers) {
        void ** procp = (void **) &disp->DeleteFramebuffers;
        snprintf(symboln, sizeof(symboln), "%sDeleteFramebuffersEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteFramebuffers) {
        void ** procp = (void **) &disp->DeleteFramebuffers;
        snprintf(symboln, sizeof(symboln), "%sDeleteFramebuffersOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteRenderbuffers) {
        void ** procp = (void **) &disp->DeleteRenderbuffers;
        snprintf(symboln, sizeof(symboln), "%sDeleteRenderbuffers", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteRenderbuffers) {
        void ** procp = (void **) &disp->DeleteRenderbuffers;
        snprintf(symboln, sizeof(symboln), "%sDeleteRenderbuffersEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteRenderbuffers) {
        void ** procp = (void **) &disp->DeleteRenderbuffers;
        snprintf(symboln, sizeof(symboln), "%sDeleteRenderbuffersOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FramebufferRenderbuffer) {
        void ** procp = (void **) &disp->FramebufferRenderbuffer;
        snprintf(symboln, sizeof(symboln), "%sFramebufferRenderbuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FramebufferRenderbuffer) {
        void ** procp = (void **) &disp->FramebufferRenderbuffer;
        snprintf(symboln, sizeof(symboln), "%sFramebufferRenderbufferEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FramebufferRenderbuffer) {
        void ** procp = (void **) &disp->FramebufferRenderbuffer;
        snprintf(symboln, sizeof(symboln), "%sFramebufferRenderbufferOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FramebufferTexture1D) {
        void ** procp = (void **) &disp->FramebufferTexture1D;
        snprintf(symboln, sizeof(symboln), "%sFramebufferTexture1D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FramebufferTexture1D) {
        void ** procp = (void **) &disp->FramebufferTexture1D;
        snprintf(symboln, sizeof(symboln), "%sFramebufferTexture1DEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FramebufferTexture2D) {
        void ** procp = (void **) &disp->FramebufferTexture2D;
        snprintf(symboln, sizeof(symboln), "%sFramebufferTexture2D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FramebufferTexture2D) {
        void ** procp = (void **) &disp->FramebufferTexture2D;
        snprintf(symboln, sizeof(symboln), "%sFramebufferTexture2DEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FramebufferTexture2D) {
        void ** procp = (void **) &disp->FramebufferTexture2D;
        snprintf(symboln, sizeof(symboln), "%sFramebufferTexture2DOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FramebufferTexture3D) {
        void ** procp = (void **) &disp->FramebufferTexture3D;
        snprintf(symboln, sizeof(symboln), "%sFramebufferTexture3D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FramebufferTexture3D) {
        void ** procp = (void **) &disp->FramebufferTexture3D;
        snprintf(symboln, sizeof(symboln), "%sFramebufferTexture3DEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FramebufferTexture3D) {
        void ** procp = (void **) &disp->FramebufferTexture3D;
        snprintf(symboln, sizeof(symboln), "%sFramebufferTexture3DOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FramebufferTextureLayer) {
        void ** procp = (void **) &disp->FramebufferTextureLayer;
        snprintf(symboln, sizeof(symboln), "%sFramebufferTextureLayer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FramebufferTextureLayer) {
        void ** procp = (void **) &disp->FramebufferTextureLayer;
        snprintf(symboln, sizeof(symboln), "%sFramebufferTextureLayerARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FramebufferTextureLayer) {
        void ** procp = (void **) &disp->FramebufferTextureLayer;
        snprintf(symboln, sizeof(symboln), "%sFramebufferTextureLayerEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenFramebuffers) {
        void ** procp = (void **) &disp->GenFramebuffers;
        snprintf(symboln, sizeof(symboln), "%sGenFramebuffers", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenFramebuffers) {
        void ** procp = (void **) &disp->GenFramebuffers;
        snprintf(symboln, sizeof(symboln), "%sGenFramebuffersEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenFramebuffers) {
        void ** procp = (void **) &disp->GenFramebuffers;
        snprintf(symboln, sizeof(symboln), "%sGenFramebuffersOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenRenderbuffers) {
        void ** procp = (void **) &disp->GenRenderbuffers;
        snprintf(symboln, sizeof(symboln), "%sGenRenderbuffers", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenRenderbuffers) {
        void ** procp = (void **) &disp->GenRenderbuffers;
        snprintf(symboln, sizeof(symboln), "%sGenRenderbuffersEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenRenderbuffers) {
        void ** procp = (void **) &disp->GenRenderbuffers;
        snprintf(symboln, sizeof(symboln), "%sGenRenderbuffersOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenerateMipmap) {
        void ** procp = (void **) &disp->GenerateMipmap;
        snprintf(symboln, sizeof(symboln), "%sGenerateMipmap", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenerateMipmap) {
        void ** procp = (void **) &disp->GenerateMipmap;
        snprintf(symboln, sizeof(symboln), "%sGenerateMipmapEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenerateMipmap) {
        void ** procp = (void **) &disp->GenerateMipmap;
        snprintf(symboln, sizeof(symboln), "%sGenerateMipmapOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetFramebufferAttachmentParameteriv) {
        void ** procp = (void **) &disp->GetFramebufferAttachmentParameteriv;
        snprintf(symboln, sizeof(symboln), "%sGetFramebufferAttachmentParameteriv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetFramebufferAttachmentParameteriv) {
        void ** procp = (void **) &disp->GetFramebufferAttachmentParameteriv;
        snprintf(symboln, sizeof(symboln), "%sGetFramebufferAttachmentParameterivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetFramebufferAttachmentParameteriv) {
        void ** procp = (void **) &disp->GetFramebufferAttachmentParameteriv;
        snprintf(symboln, sizeof(symboln), "%sGetFramebufferAttachmentParameterivOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetRenderbufferParameteriv) {
        void ** procp = (void **) &disp->GetRenderbufferParameteriv;
        snprintf(symboln, sizeof(symboln), "%sGetRenderbufferParameteriv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetRenderbufferParameteriv) {
        void ** procp = (void **) &disp->GetRenderbufferParameteriv;
        snprintf(symboln, sizeof(symboln), "%sGetRenderbufferParameterivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetRenderbufferParameteriv) {
        void ** procp = (void **) &disp->GetRenderbufferParameteriv;
        snprintf(symboln, sizeof(symboln), "%sGetRenderbufferParameterivOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsFramebuffer) {
        void ** procp = (void **) &disp->IsFramebuffer;
        snprintf(symboln, sizeof(symboln), "%sIsFramebuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsFramebuffer) {
        void ** procp = (void **) &disp->IsFramebuffer;
        snprintf(symboln, sizeof(symboln), "%sIsFramebufferEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsFramebuffer) {
        void ** procp = (void **) &disp->IsFramebuffer;
        snprintf(symboln, sizeof(symboln), "%sIsFramebufferOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsRenderbuffer) {
        void ** procp = (void **) &disp->IsRenderbuffer;
        snprintf(symboln, sizeof(symboln), "%sIsRenderbuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsRenderbuffer) {
        void ** procp = (void **) &disp->IsRenderbuffer;
        snprintf(symboln, sizeof(symboln), "%sIsRenderbufferEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsRenderbuffer) {
        void ** procp = (void **) &disp->IsRenderbuffer;
        snprintf(symboln, sizeof(symboln), "%sIsRenderbufferOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RenderbufferStorage) {
        void ** procp = (void **) &disp->RenderbufferStorage;
        snprintf(symboln, sizeof(symboln), "%sRenderbufferStorage", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RenderbufferStorage) {
        void ** procp = (void **) &disp->RenderbufferStorage;
        snprintf(symboln, sizeof(symboln), "%sRenderbufferStorageEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RenderbufferStorage) {
        void ** procp = (void **) &disp->RenderbufferStorage;
        snprintf(symboln, sizeof(symboln), "%sRenderbufferStorageOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RenderbufferStorageMultisample) {
        void ** procp = (void **) &disp->RenderbufferStorageMultisample;
        snprintf(symboln, sizeof(symboln), "%sRenderbufferStorageMultisample", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RenderbufferStorageMultisample) {
        void ** procp = (void **) &disp->RenderbufferStorageMultisample;
        snprintf(symboln, sizeof(symboln), "%sRenderbufferStorageMultisampleEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FlushMappedBufferRange) {
        void ** procp = (void **) &disp->FlushMappedBufferRange;
        snprintf(symboln, sizeof(symboln), "%sFlushMappedBufferRange", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FlushMappedBufferRange) {
        void ** procp = (void **) &disp->FlushMappedBufferRange;
        snprintf(symboln, sizeof(symboln), "%sFlushMappedBufferRangeEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MapBufferRange) {
        void ** procp = (void **) &disp->MapBufferRange;
        snprintf(symboln, sizeof(symboln), "%sMapBufferRange", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MapBufferRange) {
        void ** procp = (void **) &disp->MapBufferRange;
        snprintf(symboln, sizeof(symboln), "%sMapBufferRangeEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindVertexArray) {
        void ** procp = (void **) &disp->BindVertexArray;
        snprintf(symboln, sizeof(symboln), "%sBindVertexArray", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindVertexArray) {
        void ** procp = (void **) &disp->BindVertexArray;
        snprintf(symboln, sizeof(symboln), "%sBindVertexArrayOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteVertexArrays) {
        void ** procp = (void **) &disp->DeleteVertexArrays;
        snprintf(symboln, sizeof(symboln), "%sDeleteVertexArrays", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteVertexArrays) {
        void ** procp = (void **) &disp->DeleteVertexArrays;
        snprintf(symboln, sizeof(symboln), "%sDeleteVertexArraysAPPLE", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteVertexArrays) {
        void ** procp = (void **) &disp->DeleteVertexArrays;
        snprintf(symboln, sizeof(symboln), "%sDeleteVertexArraysOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenVertexArrays) {
        void ** procp = (void **) &disp->GenVertexArrays;
        snprintf(symboln, sizeof(symboln), "%sGenVertexArrays", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenVertexArrays) {
        void ** procp = (void **) &disp->GenVertexArrays;
        snprintf(symboln, sizeof(symboln), "%sGenVertexArraysOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsVertexArray) {
        void ** procp = (void **) &disp->IsVertexArray;
        snprintf(symboln, sizeof(symboln), "%sIsVertexArray", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsVertexArray) {
        void ** procp = (void **) &disp->IsVertexArray;
        snprintf(symboln, sizeof(symboln), "%sIsVertexArrayAPPLE", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsVertexArray) {
        void ** procp = (void **) &disp->IsVertexArray;
        snprintf(symboln, sizeof(symboln), "%sIsVertexArrayOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetActiveUniformBlockName) {
        void ** procp = (void **) &disp->GetActiveUniformBlockName;
        snprintf(symboln, sizeof(symboln), "%sGetActiveUniformBlockName", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetActiveUniformBlockiv) {
        void ** procp = (void **) &disp->GetActiveUniformBlockiv;
        snprintf(symboln, sizeof(symboln), "%sGetActiveUniformBlockiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetActiveUniformName) {
        void ** procp = (void **) &disp->GetActiveUniformName;
        snprintf(symboln, sizeof(symboln), "%sGetActiveUniformName", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetActiveUniformsiv) {
        void ** procp = (void **) &disp->GetActiveUniformsiv;
        snprintf(symboln, sizeof(symboln), "%sGetActiveUniformsiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetUniformBlockIndex) {
        void ** procp = (void **) &disp->GetUniformBlockIndex;
        snprintf(symboln, sizeof(symboln), "%sGetUniformBlockIndex", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetUniformIndices) {
        void ** procp = (void **) &disp->GetUniformIndices;
        snprintf(symboln, sizeof(symboln), "%sGetUniformIndices", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UniformBlockBinding) {
        void ** procp = (void **) &disp->UniformBlockBinding;
        snprintf(symboln, sizeof(symboln), "%sUniformBlockBinding", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CopyBufferSubData) {
        void ** procp = (void **) &disp->CopyBufferSubData;
        snprintf(symboln, sizeof(symboln), "%sCopyBufferSubData", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClientWaitSync) {
        void ** procp = (void **) &disp->ClientWaitSync;
        snprintf(symboln, sizeof(symboln), "%sClientWaitSync", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteSync) {
        void ** procp = (void **) &disp->DeleteSync;
        snprintf(symboln, sizeof(symboln), "%sDeleteSync", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FenceSync) {
        void ** procp = (void **) &disp->FenceSync;
        snprintf(symboln, sizeof(symboln), "%sFenceSync", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetInteger64v) {
        void ** procp = (void **) &disp->GetInteger64v;
        snprintf(symboln, sizeof(symboln), "%sGetInteger64v", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetSynciv) {
        void ** procp = (void **) &disp->GetSynciv;
        snprintf(symboln, sizeof(symboln), "%sGetSynciv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsSync) {
        void ** procp = (void **) &disp->IsSync;
        snprintf(symboln, sizeof(symboln), "%sIsSync", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WaitSync) {
        void ** procp = (void **) &disp->WaitSync;
        snprintf(symboln, sizeof(symboln), "%sWaitSync", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawElementsBaseVertex) {
        void ** procp = (void **) &disp->DrawElementsBaseVertex;
        snprintf(symboln, sizeof(symboln), "%sDrawElementsBaseVertex", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawElementsInstancedBaseVertex) {
        void ** procp = (void **) &disp->DrawElementsInstancedBaseVertex;
        snprintf(symboln, sizeof(symboln), "%sDrawElementsInstancedBaseVertex", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawRangeElementsBaseVertex) {
        void ** procp = (void **) &disp->DrawRangeElementsBaseVertex;
        snprintf(symboln, sizeof(symboln), "%sDrawRangeElementsBaseVertex", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiDrawElementsBaseVertex) {
        void ** procp = (void **) &disp->MultiDrawElementsBaseVertex;
        snprintf(symboln, sizeof(symboln), "%sMultiDrawElementsBaseVertex", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProvokingVertex) {
        void ** procp = (void **) &disp->ProvokingVertex;
        snprintf(symboln, sizeof(symboln), "%sProvokingVertexEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProvokingVertex) {
        void ** procp = (void **) &disp->ProvokingVertex;
        snprintf(symboln, sizeof(symboln), "%sProvokingVertex", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetMultisamplefv) {
        void ** procp = (void **) &disp->GetMultisamplefv;
        snprintf(symboln, sizeof(symboln), "%sGetMultisamplefv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SampleMaski) {
        void ** procp = (void **) &disp->SampleMaski;
        snprintf(symboln, sizeof(symboln), "%sSampleMaski", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexImage2DMultisample) {
        void ** procp = (void **) &disp->TexImage2DMultisample;
        snprintf(symboln, sizeof(symboln), "%sTexImage2DMultisample", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexImage3DMultisample) {
        void ** procp = (void **) &disp->TexImage3DMultisample;
        snprintf(symboln, sizeof(symboln), "%sTexImage3DMultisample", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlendEquationSeparateiARB) {
        void ** procp = (void **) &disp->BlendEquationSeparateiARB;
        snprintf(symboln, sizeof(symboln), "%sBlendEquationSeparateiARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlendEquationSeparateiARB) {
        void ** procp = (void **) &disp->BlendEquationSeparateiARB;
        snprintf(symboln, sizeof(symboln), "%sBlendEquationSeparateIndexedAMD", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlendEquationSeparateiARB) {
        void ** procp = (void **) &disp->BlendEquationSeparateiARB;
        snprintf(symboln, sizeof(symboln), "%sBlendEquationSeparatei", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlendEquationiARB) {
        void ** procp = (void **) &disp->BlendEquationiARB;
        snprintf(symboln, sizeof(symboln), "%sBlendEquationiARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlendEquationiARB) {
        void ** procp = (void **) &disp->BlendEquationiARB;
        snprintf(symboln, sizeof(symboln), "%sBlendEquationIndexedAMD", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlendEquationiARB) {
        void ** procp = (void **) &disp->BlendEquationiARB;
        snprintf(symboln, sizeof(symboln), "%sBlendEquationi", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlendFuncSeparateiARB) {
        void ** procp = (void **) &disp->BlendFuncSeparateiARB;
        snprintf(symboln, sizeof(symboln), "%sBlendFuncSeparateiARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlendFuncSeparateiARB) {
        void ** procp = (void **) &disp->BlendFuncSeparateiARB;
        snprintf(symboln, sizeof(symboln), "%sBlendFuncSeparateIndexedAMD", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlendFuncSeparateiARB) {
        void ** procp = (void **) &disp->BlendFuncSeparateiARB;
        snprintf(symboln, sizeof(symboln), "%sBlendFuncSeparatei", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlendFunciARB) {
        void ** procp = (void **) &disp->BlendFunciARB;
        snprintf(symboln, sizeof(symboln), "%sBlendFunciARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlendFunciARB) {
        void ** procp = (void **) &disp->BlendFunciARB;
        snprintf(symboln, sizeof(symboln), "%sBlendFuncIndexedAMD", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlendFunciARB) {
        void ** procp = (void **) &disp->BlendFunciARB;
        snprintf(symboln, sizeof(symboln), "%sBlendFunci", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindFragDataLocationIndexed) {
        void ** procp = (void **) &disp->BindFragDataLocationIndexed;
        snprintf(symboln, sizeof(symboln), "%sBindFragDataLocationIndexed", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetFragDataIndex) {
        void ** procp = (void **) &disp->GetFragDataIndex;
        snprintf(symboln, sizeof(symboln), "%sGetFragDataIndex", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindSampler) {
        void ** procp = (void **) &disp->BindSampler;
        snprintf(symboln, sizeof(symboln), "%sBindSampler", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteSamplers) {
        void ** procp = (void **) &disp->DeleteSamplers;
        snprintf(symboln, sizeof(symboln), "%sDeleteSamplers", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenSamplers) {
        void ** procp = (void **) &disp->GenSamplers;
        snprintf(symboln, sizeof(symboln), "%sGenSamplers", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetSamplerParameterIiv) {
        void ** procp = (void **) &disp->GetSamplerParameterIiv;
        snprintf(symboln, sizeof(symboln), "%sGetSamplerParameterIiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetSamplerParameterIuiv) {
        void ** procp = (void **) &disp->GetSamplerParameterIuiv;
        snprintf(symboln, sizeof(symboln), "%sGetSamplerParameterIuiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetSamplerParameterfv) {
        void ** procp = (void **) &disp->GetSamplerParameterfv;
        snprintf(symboln, sizeof(symboln), "%sGetSamplerParameterfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetSamplerParameteriv) {
        void ** procp = (void **) &disp->GetSamplerParameteriv;
        snprintf(symboln, sizeof(symboln), "%sGetSamplerParameteriv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsSampler) {
        void ** procp = (void **) &disp->IsSampler;
        snprintf(symboln, sizeof(symboln), "%sIsSampler", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SamplerParameterIiv) {
        void ** procp = (void **) &disp->SamplerParameterIiv;
        snprintf(symboln, sizeof(symboln), "%sSamplerParameterIiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SamplerParameterIuiv) {
        void ** procp = (void **) &disp->SamplerParameterIuiv;
        snprintf(symboln, sizeof(symboln), "%sSamplerParameterIuiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SamplerParameterf) {
        void ** procp = (void **) &disp->SamplerParameterf;
        snprintf(symboln, sizeof(symboln), "%sSamplerParameterf", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SamplerParameterfv) {
        void ** procp = (void **) &disp->SamplerParameterfv;
        snprintf(symboln, sizeof(symboln), "%sSamplerParameterfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SamplerParameteri) {
        void ** procp = (void **) &disp->SamplerParameteri;
        snprintf(symboln, sizeof(symboln), "%sSamplerParameteri", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SamplerParameteriv) {
        void ** procp = (void **) &disp->SamplerParameteriv;
        snprintf(symboln, sizeof(symboln), "%sSamplerParameteriv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetQueryObjecti64v) {
        void ** procp = (void **) &disp->GetQueryObjecti64v;
        snprintf(symboln, sizeof(symboln), "%sGetQueryObjecti64v", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetQueryObjecti64v) {
        void ** procp = (void **) &disp->GetQueryObjecti64v;
        snprintf(symboln, sizeof(symboln), "%sGetQueryObjecti64vEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetQueryObjectui64v) {
        void ** procp = (void **) &disp->GetQueryObjectui64v;
        snprintf(symboln, sizeof(symboln), "%sGetQueryObjectui64v", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetQueryObjectui64v) {
        void ** procp = (void **) &disp->GetQueryObjectui64v;
        snprintf(symboln, sizeof(symboln), "%sGetQueryObjectui64vEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->QueryCounter) {
        void ** procp = (void **) &disp->QueryCounter;
        snprintf(symboln, sizeof(symboln), "%sQueryCounter", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ColorP3ui) {
        void ** procp = (void **) &disp->ColorP3ui;
        snprintf(symboln, sizeof(symboln), "%sColorP3ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ColorP3uiv) {
        void ** procp = (void **) &disp->ColorP3uiv;
        snprintf(symboln, sizeof(symboln), "%sColorP3uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ColorP4ui) {
        void ** procp = (void **) &disp->ColorP4ui;
        snprintf(symboln, sizeof(symboln), "%sColorP4ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ColorP4uiv) {
        void ** procp = (void **) &disp->ColorP4uiv;
        snprintf(symboln, sizeof(symboln), "%sColorP4uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoordP1ui) {
        void ** procp = (void **) &disp->MultiTexCoordP1ui;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoordP1ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoordP1uiv) {
        void ** procp = (void **) &disp->MultiTexCoordP1uiv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoordP1uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoordP2ui) {
        void ** procp = (void **) &disp->MultiTexCoordP2ui;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoordP2ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoordP2uiv) {
        void ** procp = (void **) &disp->MultiTexCoordP2uiv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoordP2uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoordP3ui) {
        void ** procp = (void **) &disp->MultiTexCoordP3ui;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoordP3ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoordP3uiv) {
        void ** procp = (void **) &disp->MultiTexCoordP3uiv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoordP3uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoordP4ui) {
        void ** procp = (void **) &disp->MultiTexCoordP4ui;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoordP4ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoordP4uiv) {
        void ** procp = (void **) &disp->MultiTexCoordP4uiv;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoordP4uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->NormalP3ui) {
        void ** procp = (void **) &disp->NormalP3ui;
        snprintf(symboln, sizeof(symboln), "%sNormalP3ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->NormalP3uiv) {
        void ** procp = (void **) &disp->NormalP3uiv;
        snprintf(symboln, sizeof(symboln), "%sNormalP3uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColorP3ui) {
        void ** procp = (void **) &disp->SecondaryColorP3ui;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColorP3ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColorP3uiv) {
        void ** procp = (void **) &disp->SecondaryColorP3uiv;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColorP3uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoordP1ui) {
        void ** procp = (void **) &disp->TexCoordP1ui;
        snprintf(symboln, sizeof(symboln), "%sTexCoordP1ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoordP1uiv) {
        void ** procp = (void **) &disp->TexCoordP1uiv;
        snprintf(symboln, sizeof(symboln), "%sTexCoordP1uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoordP2ui) {
        void ** procp = (void **) &disp->TexCoordP2ui;
        snprintf(symboln, sizeof(symboln), "%sTexCoordP2ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoordP2uiv) {
        void ** procp = (void **) &disp->TexCoordP2uiv;
        snprintf(symboln, sizeof(symboln), "%sTexCoordP2uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoordP3ui) {
        void ** procp = (void **) &disp->TexCoordP3ui;
        snprintf(symboln, sizeof(symboln), "%sTexCoordP3ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoordP3uiv) {
        void ** procp = (void **) &disp->TexCoordP3uiv;
        snprintf(symboln, sizeof(symboln), "%sTexCoordP3uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoordP4ui) {
        void ** procp = (void **) &disp->TexCoordP4ui;
        snprintf(symboln, sizeof(symboln), "%sTexCoordP4ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoordP4uiv) {
        void ** procp = (void **) &disp->TexCoordP4uiv;
        snprintf(symboln, sizeof(symboln), "%sTexCoordP4uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribP1ui) {
        void ** procp = (void **) &disp->VertexAttribP1ui;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribP1ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribP1uiv) {
        void ** procp = (void **) &disp->VertexAttribP1uiv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribP1uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribP2ui) {
        void ** procp = (void **) &disp->VertexAttribP2ui;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribP2ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribP2uiv) {
        void ** procp = (void **) &disp->VertexAttribP2uiv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribP2uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribP3ui) {
        void ** procp = (void **) &disp->VertexAttribP3ui;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribP3ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribP3uiv) {
        void ** procp = (void **) &disp->VertexAttribP3uiv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribP3uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribP4ui) {
        void ** procp = (void **) &disp->VertexAttribP4ui;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribP4ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribP4uiv) {
        void ** procp = (void **) &disp->VertexAttribP4uiv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribP4uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexP2ui) {
        void ** procp = (void **) &disp->VertexP2ui;
        snprintf(symboln, sizeof(symboln), "%sVertexP2ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexP2uiv) {
        void ** procp = (void **) &disp->VertexP2uiv;
        snprintf(symboln, sizeof(symboln), "%sVertexP2uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexP3ui) {
        void ** procp = (void **) &disp->VertexP3ui;
        snprintf(symboln, sizeof(symboln), "%sVertexP3ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexP3uiv) {
        void ** procp = (void **) &disp->VertexP3uiv;
        snprintf(symboln, sizeof(symboln), "%sVertexP3uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexP4ui) {
        void ** procp = (void **) &disp->VertexP4ui;
        snprintf(symboln, sizeof(symboln), "%sVertexP4ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexP4uiv) {
        void ** procp = (void **) &disp->VertexP4uiv;
        snprintf(symboln, sizeof(symboln), "%sVertexP4uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawArraysIndirect) {
        void ** procp = (void **) &disp->DrawArraysIndirect;
        snprintf(symboln, sizeof(symboln), "%sDrawArraysIndirect", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawElementsIndirect) {
        void ** procp = (void **) &disp->DrawElementsIndirect;
        snprintf(symboln, sizeof(symboln), "%sDrawElementsIndirect", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetUniformdv) {
        void ** procp = (void **) &disp->GetUniformdv;
        snprintf(symboln, sizeof(symboln), "%sGetUniformdv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform1d) {
        void ** procp = (void **) &disp->Uniform1d;
        snprintf(symboln, sizeof(symboln), "%sUniform1d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform1dv) {
        void ** procp = (void **) &disp->Uniform1dv;
        snprintf(symboln, sizeof(symboln), "%sUniform1dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform2d) {
        void ** procp = (void **) &disp->Uniform2d;
        snprintf(symboln, sizeof(symboln), "%sUniform2d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform2dv) {
        void ** procp = (void **) &disp->Uniform2dv;
        snprintf(symboln, sizeof(symboln), "%sUniform2dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform3d) {
        void ** procp = (void **) &disp->Uniform3d;
        snprintf(symboln, sizeof(symboln), "%sUniform3d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform3dv) {
        void ** procp = (void **) &disp->Uniform3dv;
        snprintf(symboln, sizeof(symboln), "%sUniform3dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform4d) {
        void ** procp = (void **) &disp->Uniform4d;
        snprintf(symboln, sizeof(symboln), "%sUniform4d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Uniform4dv) {
        void ** procp = (void **) &disp->Uniform4dv;
        snprintf(symboln, sizeof(symboln), "%sUniform4dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UniformMatrix2dv) {
        void ** procp = (void **) &disp->UniformMatrix2dv;
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix2dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UniformMatrix2x3dv) {
        void ** procp = (void **) &disp->UniformMatrix2x3dv;
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix2x3dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UniformMatrix2x4dv) {
        void ** procp = (void **) &disp->UniformMatrix2x4dv;
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix2x4dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UniformMatrix3dv) {
        void ** procp = (void **) &disp->UniformMatrix3dv;
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix3dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UniformMatrix3x2dv) {
        void ** procp = (void **) &disp->UniformMatrix3x2dv;
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix3x2dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UniformMatrix3x4dv) {
        void ** procp = (void **) &disp->UniformMatrix3x4dv;
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix3x4dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UniformMatrix4dv) {
        void ** procp = (void **) &disp->UniformMatrix4dv;
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix4dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UniformMatrix4x2dv) {
        void ** procp = (void **) &disp->UniformMatrix4x2dv;
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix4x2dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UniformMatrix4x3dv) {
        void ** procp = (void **) &disp->UniformMatrix4x3dv;
        snprintf(symboln, sizeof(symboln), "%sUniformMatrix4x3dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindTransformFeedback) {
        void ** procp = (void **) &disp->BindTransformFeedback;
        snprintf(symboln, sizeof(symboln), "%sBindTransformFeedback", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteTransformFeedbacks) {
        void ** procp = (void **) &disp->DeleteTransformFeedbacks;
        snprintf(symboln, sizeof(symboln), "%sDeleteTransformFeedbacks", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawTransformFeedback) {
        void ** procp = (void **) &disp->DrawTransformFeedback;
        snprintf(symboln, sizeof(symboln), "%sDrawTransformFeedback", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenTransformFeedbacks) {
        void ** procp = (void **) &disp->GenTransformFeedbacks;
        snprintf(symboln, sizeof(symboln), "%sGenTransformFeedbacks", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsTransformFeedback) {
        void ** procp = (void **) &disp->IsTransformFeedback;
        snprintf(symboln, sizeof(symboln), "%sIsTransformFeedback", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PauseTransformFeedback) {
        void ** procp = (void **) &disp->PauseTransformFeedback;
        snprintf(symboln, sizeof(symboln), "%sPauseTransformFeedback", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ResumeTransformFeedback) {
        void ** procp = (void **) &disp->ResumeTransformFeedback;
        snprintf(symboln, sizeof(symboln), "%sResumeTransformFeedback", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BeginQueryIndexed) {
        void ** procp = (void **) &disp->BeginQueryIndexed;
        snprintf(symboln, sizeof(symboln), "%sBeginQueryIndexed", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawTransformFeedbackStream) {
        void ** procp = (void **) &disp->DrawTransformFeedbackStream;
        snprintf(symboln, sizeof(symboln), "%sDrawTransformFeedbackStream", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EndQueryIndexed) {
        void ** procp = (void **) &disp->EndQueryIndexed;
        snprintf(symboln, sizeof(symboln), "%sEndQueryIndexed", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetQueryIndexediv) {
        void ** procp = (void **) &disp->GetQueryIndexediv;
        snprintf(symboln, sizeof(symboln), "%sGetQueryIndexediv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearDepthf) {
        void ** procp = (void **) &disp->ClearDepthf;
        snprintf(symboln, sizeof(symboln), "%sClearDepthf", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearDepthf) {
        void ** procp = (void **) &disp->ClearDepthf;
        snprintf(symboln, sizeof(symboln), "%sClearDepthfOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DepthRangef) {
        void ** procp = (void **) &disp->DepthRangef;
        snprintf(symboln, sizeof(symboln), "%sDepthRangef", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DepthRangef) {
        void ** procp = (void **) &disp->DepthRangef;
        snprintf(symboln, sizeof(symboln), "%sDepthRangefOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetShaderPrecisionFormat) {
        void ** procp = (void **) &disp->GetShaderPrecisionFormat;
        snprintf(symboln, sizeof(symboln), "%sGetShaderPrecisionFormat", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ReleaseShaderCompiler) {
        void ** procp = (void **) &disp->ReleaseShaderCompiler;
        snprintf(symboln, sizeof(symboln), "%sReleaseShaderCompiler", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ShaderBinary) {
        void ** procp = (void **) &disp->ShaderBinary;
        snprintf(symboln, sizeof(symboln), "%sShaderBinary", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetProgramBinary) {
        void ** procp = (void **) &disp->GetProgramBinary;
        snprintf(symboln, sizeof(symboln), "%sGetProgramBinary", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetProgramBinary) {
        void ** procp = (void **) &disp->GetProgramBinary;
        snprintf(symboln, sizeof(symboln), "%sGetProgramBinaryOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramBinary) {
        void ** procp = (void **) &disp->ProgramBinary;
        snprintf(symboln, sizeof(symboln), "%sProgramBinary", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramBinary) {
        void ** procp = (void **) &disp->ProgramBinary;
        snprintf(symboln, sizeof(symboln), "%sProgramBinaryOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramParameteri) {
        void ** procp = (void **) &disp->ProgramParameteri;
        snprintf(symboln, sizeof(symboln), "%sProgramParameteriARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramParameteri) {
        void ** procp = (void **) &disp->ProgramParameteri;
        snprintf(symboln, sizeof(symboln), "%sProgramParameteri", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramParameteri) {
        void ** procp = (void **) &disp->ProgramParameteri;
        snprintf(symboln, sizeof(symboln), "%sProgramParameteriEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetVertexAttribLdv) {
        void ** procp = (void **) &disp->GetVertexAttribLdv;
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribLdv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribL1d) {
        void ** procp = (void **) &disp->VertexAttribL1d;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribL1d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribL1dv) {
        void ** procp = (void **) &disp->VertexAttribL1dv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribL1dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribL2d) {
        void ** procp = (void **) &disp->VertexAttribL2d;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribL2d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribL2dv) {
        void ** procp = (void **) &disp->VertexAttribL2dv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribL2dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribL3d) {
        void ** procp = (void **) &disp->VertexAttribL3d;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribL3d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribL3dv) {
        void ** procp = (void **) &disp->VertexAttribL3dv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribL3dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribL4d) {
        void ** procp = (void **) &disp->VertexAttribL4d;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribL4d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribL4dv) {
        void ** procp = (void **) &disp->VertexAttribL4dv;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribL4dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribLPointer) {
        void ** procp = (void **) &disp->VertexAttribLPointer;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribLPointer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DepthRangeArrayv) {
        void ** procp = (void **) &disp->DepthRangeArrayv;
        snprintf(symboln, sizeof(symboln), "%sDepthRangeArrayv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DepthRangeIndexed) {
        void ** procp = (void **) &disp->DepthRangeIndexed;
        snprintf(symboln, sizeof(symboln), "%sDepthRangeIndexed", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetDoublei_v) {
        void ** procp = (void **) &disp->GetDoublei_v;
        snprintf(symboln, sizeof(symboln), "%sGetDoublei_v", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetFloati_v) {
        void ** procp = (void **) &disp->GetFloati_v;
        snprintf(symboln, sizeof(symboln), "%sGetFloati_v", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ScissorArrayv) {
        void ** procp = (void **) &disp->ScissorArrayv;
        snprintf(symboln, sizeof(symboln), "%sScissorArrayv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ScissorIndexed) {
        void ** procp = (void **) &disp->ScissorIndexed;
        snprintf(symboln, sizeof(symboln), "%sScissorIndexed", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ScissorIndexedv) {
        void ** procp = (void **) &disp->ScissorIndexedv;
        snprintf(symboln, sizeof(symboln), "%sScissorIndexedv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ViewportArrayv) {
        void ** procp = (void **) &disp->ViewportArrayv;
        snprintf(symboln, sizeof(symboln), "%sViewportArrayv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ViewportIndexedf) {
        void ** procp = (void **) &disp->ViewportIndexedf;
        snprintf(symboln, sizeof(symboln), "%sViewportIndexedf", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ViewportIndexedfv) {
        void ** procp = (void **) &disp->ViewportIndexedfv;
        snprintf(symboln, sizeof(symboln), "%sViewportIndexedfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetGraphicsResetStatusARB) {
        void ** procp = (void **) &disp->GetGraphicsResetStatusARB;
        snprintf(symboln, sizeof(symboln), "%sGetGraphicsResetStatusARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetnColorTableARB) {
        void ** procp = (void **) &disp->GetnColorTableARB;
        snprintf(symboln, sizeof(symboln), "%sGetnColorTableARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetnCompressedTexImageARB) {
        void ** procp = (void **) &disp->GetnCompressedTexImageARB;
        snprintf(symboln, sizeof(symboln), "%sGetnCompressedTexImageARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetnConvolutionFilterARB) {
        void ** procp = (void **) &disp->GetnConvolutionFilterARB;
        snprintf(symboln, sizeof(symboln), "%sGetnConvolutionFilterARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetnHistogramARB) {
        void ** procp = (void **) &disp->GetnHistogramARB;
        snprintf(symboln, sizeof(symboln), "%sGetnHistogramARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetnMapdvARB) {
        void ** procp = (void **) &disp->GetnMapdvARB;
        snprintf(symboln, sizeof(symboln), "%sGetnMapdvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetnMapfvARB) {
        void ** procp = (void **) &disp->GetnMapfvARB;
        snprintf(symboln, sizeof(symboln), "%sGetnMapfvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetnMapivARB) {
        void ** procp = (void **) &disp->GetnMapivARB;
        snprintf(symboln, sizeof(symboln), "%sGetnMapivARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetnMinmaxARB) {
        void ** procp = (void **) &disp->GetnMinmaxARB;
        snprintf(symboln, sizeof(symboln), "%sGetnMinmaxARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetnPixelMapfvARB) {
        void ** procp = (void **) &disp->GetnPixelMapfvARB;
        snprintf(symboln, sizeof(symboln), "%sGetnPixelMapfvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetnPixelMapuivARB) {
        void ** procp = (void **) &disp->GetnPixelMapuivARB;
        snprintf(symboln, sizeof(symboln), "%sGetnPixelMapuivARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetnPixelMapusvARB) {
        void ** procp = (void **) &disp->GetnPixelMapusvARB;
        snprintf(symboln, sizeof(symboln), "%sGetnPixelMapusvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetnPolygonStippleARB) {
        void ** procp = (void **) &disp->GetnPolygonStippleARB;
        snprintf(symboln, sizeof(symboln), "%sGetnPolygonStippleARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetnSeparableFilterARB) {
        void ** procp = (void **) &disp->GetnSeparableFilterARB;
        snprintf(symboln, sizeof(symboln), "%sGetnSeparableFilterARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetnTexImageARB) {
        void ** procp = (void **) &disp->GetnTexImageARB;
        snprintf(symboln, sizeof(symboln), "%sGetnTexImageARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetnUniformdvARB) {
        void ** procp = (void **) &disp->GetnUniformdvARB;
        snprintf(symboln, sizeof(symboln), "%sGetnUniformdvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetnUniformfvARB) {
        void ** procp = (void **) &disp->GetnUniformfvARB;
        snprintf(symboln, sizeof(symboln), "%sGetnUniformfvARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetnUniformivARB) {
        void ** procp = (void **) &disp->GetnUniformivARB;
        snprintf(symboln, sizeof(symboln), "%sGetnUniformivARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetnUniformuivARB) {
        void ** procp = (void **) &disp->GetnUniformuivARB;
        snprintf(symboln, sizeof(symboln), "%sGetnUniformuivARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ReadnPixelsARB) {
        void ** procp = (void **) &disp->ReadnPixelsARB;
        snprintf(symboln, sizeof(symboln), "%sReadnPixelsARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawArraysInstancedBaseInstance) {
        void ** procp = (void **) &disp->DrawArraysInstancedBaseInstance;
        snprintf(symboln, sizeof(symboln), "%sDrawArraysInstancedBaseInstance", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawElementsInstancedBaseInstance) {
        void ** procp = (void **) &disp->DrawElementsInstancedBaseInstance;
        snprintf(symboln, sizeof(symboln), "%sDrawElementsInstancedBaseInstance", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawElementsInstancedBaseVertexBaseInstance) {
        void ** procp = (void **) &disp->DrawElementsInstancedBaseVertexBaseInstance;
        snprintf(symboln, sizeof(symboln), "%sDrawElementsInstancedBaseVertexBaseInstance", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawTransformFeedbackInstanced) {
        void ** procp = (void **) &disp->DrawTransformFeedbackInstanced;
        snprintf(symboln, sizeof(symboln), "%sDrawTransformFeedbackInstanced", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawTransformFeedbackStreamInstanced) {
        void ** procp = (void **) &disp->DrawTransformFeedbackStreamInstanced;
        snprintf(symboln, sizeof(symboln), "%sDrawTransformFeedbackStreamInstanced", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetInternalformativ) {
        void ** procp = (void **) &disp->GetInternalformativ;
        snprintf(symboln, sizeof(symboln), "%sGetInternalformativ", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetActiveAtomicCounterBufferiv) {
        void ** procp = (void **) &disp->GetActiveAtomicCounterBufferiv;
        snprintf(symboln, sizeof(symboln), "%sGetActiveAtomicCounterBufferiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindImageTexture) {
        void ** procp = (void **) &disp->BindImageTexture;
        snprintf(symboln, sizeof(symboln), "%sBindImageTexture", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MemoryBarrier) {
        void ** procp = (void **) &disp->MemoryBarrier;
        snprintf(symboln, sizeof(symboln), "%sMemoryBarrier", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexStorage1D) {
        void ** procp = (void **) &disp->TexStorage1D;
        snprintf(symboln, sizeof(symboln), "%sTexStorage1D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexStorage2D) {
        void ** procp = (void **) &disp->TexStorage2D;
        snprintf(symboln, sizeof(symboln), "%sTexStorage2D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexStorage3D) {
        void ** procp = (void **) &disp->TexStorage3D;
        snprintf(symboln, sizeof(symboln), "%sTexStorage3D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TextureStorage1DEXT) {
        void ** procp = (void **) &disp->TextureStorage1DEXT;
        snprintf(symboln, sizeof(symboln), "%sTextureStorage1DEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TextureStorage2DEXT) {
        void ** procp = (void **) &disp->TextureStorage2DEXT;
        snprintf(symboln, sizeof(symboln), "%sTextureStorage2DEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TextureStorage3DEXT) {
        void ** procp = (void **) &disp->TextureStorage3DEXT;
        snprintf(symboln, sizeof(symboln), "%sTextureStorage3DEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearBufferData) {
        void ** procp = (void **) &disp->ClearBufferData;
        snprintf(symboln, sizeof(symboln), "%sClearBufferData", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearBufferSubData) {
        void ** procp = (void **) &disp->ClearBufferSubData;
        snprintf(symboln, sizeof(symboln), "%sClearBufferSubData", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DispatchCompute) {
        void ** procp = (void **) &disp->DispatchCompute;
        snprintf(symboln, sizeof(symboln), "%sDispatchCompute", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DispatchComputeIndirect) {
        void ** procp = (void **) &disp->DispatchComputeIndirect;
        snprintf(symboln, sizeof(symboln), "%sDispatchComputeIndirect", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CopyImageSubData) {
        void ** procp = (void **) &disp->CopyImageSubData;
        snprintf(symboln, sizeof(symboln), "%sCopyImageSubData", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TextureView) {
        void ** procp = (void **) &disp->TextureView;
        snprintf(symboln, sizeof(symboln), "%sTextureView", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindVertexBuffer) {
        void ** procp = (void **) &disp->BindVertexBuffer;
        snprintf(symboln, sizeof(symboln), "%sBindVertexBuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribBinding) {
        void ** procp = (void **) &disp->VertexAttribBinding;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribBinding", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribFormat) {
        void ** procp = (void **) &disp->VertexAttribFormat;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribFormat", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribIFormat) {
        void ** procp = (void **) &disp->VertexAttribIFormat;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribIFormat", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribLFormat) {
        void ** procp = (void **) &disp->VertexAttribLFormat;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribLFormat", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexBindingDivisor) {
        void ** procp = (void **) &disp->VertexBindingDivisor;
        snprintf(symboln, sizeof(symboln), "%sVertexBindingDivisor", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiDrawArraysIndirect) {
        void ** procp = (void **) &disp->MultiDrawArraysIndirect;
        snprintf(symboln, sizeof(symboln), "%sMultiDrawArraysIndirect", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiDrawElementsIndirect) {
        void ** procp = (void **) &disp->MultiDrawElementsIndirect;
        snprintf(symboln, sizeof(symboln), "%sMultiDrawElementsIndirect", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetProgramInterfaceiv) {
        void ** procp = (void **) &disp->GetProgramInterfaceiv;
        snprintf(symboln, sizeof(symboln), "%sGetProgramInterfaceiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetProgramResourceIndex) {
        void ** procp = (void **) &disp->GetProgramResourceIndex;
        snprintf(symboln, sizeof(symboln), "%sGetProgramResourceIndex", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetProgramResourceLocation) {
        void ** procp = (void **) &disp->GetProgramResourceLocation;
        snprintf(symboln, sizeof(symboln), "%sGetProgramResourceLocation", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetProgramResourceLocationIndex) {
        void ** procp = (void **) &disp->GetProgramResourceLocationIndex;
        snprintf(symboln, sizeof(symboln), "%sGetProgramResourceLocationIndex", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetProgramResourceName) {
        void ** procp = (void **) &disp->GetProgramResourceName;
        snprintf(symboln, sizeof(symboln), "%sGetProgramResourceName", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetProgramResourceiv) {
        void ** procp = (void **) &disp->GetProgramResourceiv;
        snprintf(symboln, sizeof(symboln), "%sGetProgramResourceiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexBufferRange) {
        void ** procp = (void **) &disp->TexBufferRange;
        snprintf(symboln, sizeof(symboln), "%sTexBufferRange", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexStorage2DMultisample) {
        void ** procp = (void **) &disp->TexStorage2DMultisample;
        snprintf(symboln, sizeof(symboln), "%sTexStorage2DMultisample", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexStorage3DMultisample) {
        void ** procp = (void **) &disp->TexStorage3DMultisample;
        snprintf(symboln, sizeof(symboln), "%sTexStorage3DMultisample", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BufferStorage) {
        void ** procp = (void **) &disp->BufferStorage;
        snprintf(symboln, sizeof(symboln), "%sBufferStorage", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearTexImage) {
        void ** procp = (void **) &disp->ClearTexImage;
        snprintf(symboln, sizeof(symboln), "%sClearTexImage", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearTexSubImage) {
        void ** procp = (void **) &disp->ClearTexSubImage;
        snprintf(symboln, sizeof(symboln), "%sClearTexSubImage", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindBuffersBase) {
        void ** procp = (void **) &disp->BindBuffersBase;
        snprintf(symboln, sizeof(symboln), "%sBindBuffersBase", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindBuffersRange) {
        void ** procp = (void **) &disp->BindBuffersRange;
        snprintf(symboln, sizeof(symboln), "%sBindBuffersRange", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindImageTextures) {
        void ** procp = (void **) &disp->BindImageTextures;
        snprintf(symboln, sizeof(symboln), "%sBindImageTextures", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindSamplers) {
        void ** procp = (void **) &disp->BindSamplers;
        snprintf(symboln, sizeof(symboln), "%sBindSamplers", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindTextures) {
        void ** procp = (void **) &disp->BindTextures;
        snprintf(symboln, sizeof(symboln), "%sBindTextures", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindVertexBuffers) {
        void ** procp = (void **) &disp->BindVertexBuffers;
        snprintf(symboln, sizeof(symboln), "%sBindVertexBuffers", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClipControl) {
        void ** procp = (void **) &disp->ClipControl;
        snprintf(symboln, sizeof(symboln), "%sClipControl", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindTextureUnit) {
        void ** procp = (void **) &disp->BindTextureUnit;
        snprintf(symboln, sizeof(symboln), "%sBindTextureUnit", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BlitNamedFramebuffer) {
        void ** procp = (void **) &disp->BlitNamedFramebuffer;
        snprintf(symboln, sizeof(symboln), "%sBlitNamedFramebuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CheckNamedFramebufferStatus) {
        void ** procp = (void **) &disp->CheckNamedFramebufferStatus;
        snprintf(symboln, sizeof(symboln), "%sCheckNamedFramebufferStatus", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearNamedBufferData) {
        void ** procp = (void **) &disp->ClearNamedBufferData;
        snprintf(symboln, sizeof(symboln), "%sClearNamedBufferData", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearNamedBufferSubData) {
        void ** procp = (void **) &disp->ClearNamedBufferSubData;
        snprintf(symboln, sizeof(symboln), "%sClearNamedBufferSubData", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearNamedFramebufferfi) {
        void ** procp = (void **) &disp->ClearNamedFramebufferfi;
        snprintf(symboln, sizeof(symboln), "%sClearNamedFramebufferfi", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearNamedFramebufferfv) {
        void ** procp = (void **) &disp->ClearNamedFramebufferfv;
        snprintf(symboln, sizeof(symboln), "%sClearNamedFramebufferfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearNamedFramebufferiv) {
        void ** procp = (void **) &disp->ClearNamedFramebufferiv;
        snprintf(symboln, sizeof(symboln), "%sClearNamedFramebufferiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearNamedFramebufferuiv) {
        void ** procp = (void **) &disp->ClearNamedFramebufferuiv;
        snprintf(symboln, sizeof(symboln), "%sClearNamedFramebufferuiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CompressedTextureSubImage1D) {
        void ** procp = (void **) &disp->CompressedTextureSubImage1D;
        snprintf(symboln, sizeof(symboln), "%sCompressedTextureSubImage1D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CompressedTextureSubImage2D) {
        void ** procp = (void **) &disp->CompressedTextureSubImage2D;
        snprintf(symboln, sizeof(symboln), "%sCompressedTextureSubImage2D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CompressedTextureSubImage3D) {
        void ** procp = (void **) &disp->CompressedTextureSubImage3D;
        snprintf(symboln, sizeof(symboln), "%sCompressedTextureSubImage3D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CopyNamedBufferSubData) {
        void ** procp = (void **) &disp->CopyNamedBufferSubData;
        snprintf(symboln, sizeof(symboln), "%sCopyNamedBufferSubData", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CopyTextureSubImage1D) {
        void ** procp = (void **) &disp->CopyTextureSubImage1D;
        snprintf(symboln, sizeof(symboln), "%sCopyTextureSubImage1D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CopyTextureSubImage2D) {
        void ** procp = (void **) &disp->CopyTextureSubImage2D;
        snprintf(symboln, sizeof(symboln), "%sCopyTextureSubImage2D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CopyTextureSubImage3D) {
        void ** procp = (void **) &disp->CopyTextureSubImage3D;
        snprintf(symboln, sizeof(symboln), "%sCopyTextureSubImage3D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CreateBuffers) {
        void ** procp = (void **) &disp->CreateBuffers;
        snprintf(symboln, sizeof(symboln), "%sCreateBuffers", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CreateFramebuffers) {
        void ** procp = (void **) &disp->CreateFramebuffers;
        snprintf(symboln, sizeof(symboln), "%sCreateFramebuffers", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CreateProgramPipelines) {
        void ** procp = (void **) &disp->CreateProgramPipelines;
        snprintf(symboln, sizeof(symboln), "%sCreateProgramPipelines", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CreateQueries) {
        void ** procp = (void **) &disp->CreateQueries;
        snprintf(symboln, sizeof(symboln), "%sCreateQueries", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CreateRenderbuffers) {
        void ** procp = (void **) &disp->CreateRenderbuffers;
        snprintf(symboln, sizeof(symboln), "%sCreateRenderbuffers", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CreateSamplers) {
        void ** procp = (void **) &disp->CreateSamplers;
        snprintf(symboln, sizeof(symboln), "%sCreateSamplers", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CreateTextures) {
        void ** procp = (void **) &disp->CreateTextures;
        snprintf(symboln, sizeof(symboln), "%sCreateTextures", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CreateTransformFeedbacks) {
        void ** procp = (void **) &disp->CreateTransformFeedbacks;
        snprintf(symboln, sizeof(symboln), "%sCreateTransformFeedbacks", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CreateVertexArrays) {
        void ** procp = (void **) &disp->CreateVertexArrays;
        snprintf(symboln, sizeof(symboln), "%sCreateVertexArrays", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DisableVertexArrayAttrib) {
        void ** procp = (void **) &disp->DisableVertexArrayAttrib;
        snprintf(symboln, sizeof(symboln), "%sDisableVertexArrayAttrib", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EnableVertexArrayAttrib) {
        void ** procp = (void **) &disp->EnableVertexArrayAttrib;
        snprintf(symboln, sizeof(symboln), "%sEnableVertexArrayAttrib", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FlushMappedNamedBufferRange) {
        void ** procp = (void **) &disp->FlushMappedNamedBufferRange;
        snprintf(symboln, sizeof(symboln), "%sFlushMappedNamedBufferRange", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenerateTextureMipmap) {
        void ** procp = (void **) &disp->GenerateTextureMipmap;
        snprintf(symboln, sizeof(symboln), "%sGenerateTextureMipmap", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetCompressedTextureImage) {
        void ** procp = (void **) &disp->GetCompressedTextureImage;
        snprintf(symboln, sizeof(symboln), "%sGetCompressedTextureImage", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetNamedBufferParameteri64v) {
        void ** procp = (void **) &disp->GetNamedBufferParameteri64v;
        snprintf(symboln, sizeof(symboln), "%sGetNamedBufferParameteri64v", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetNamedBufferParameteriv) {
        void ** procp = (void **) &disp->GetNamedBufferParameteriv;
        snprintf(symboln, sizeof(symboln), "%sGetNamedBufferParameteriv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetNamedBufferPointerv) {
        void ** procp = (void **) &disp->GetNamedBufferPointerv;
        snprintf(symboln, sizeof(symboln), "%sGetNamedBufferPointerv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetNamedBufferSubData) {
        void ** procp = (void **) &disp->GetNamedBufferSubData;
        snprintf(symboln, sizeof(symboln), "%sGetNamedBufferSubData", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetNamedFramebufferAttachmentParameteriv) {
        void ** procp = (void **) &disp->GetNamedFramebufferAttachmentParameteriv;
        snprintf(symboln, sizeof(symboln), "%sGetNamedFramebufferAttachmentParameteriv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetNamedFramebufferParameteriv) {
        void ** procp = (void **) &disp->GetNamedFramebufferParameteriv;
        snprintf(symboln, sizeof(symboln), "%sGetNamedFramebufferParameteriv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetNamedRenderbufferParameteriv) {
        void ** procp = (void **) &disp->GetNamedRenderbufferParameteriv;
        snprintf(symboln, sizeof(symboln), "%sGetNamedRenderbufferParameteriv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetQueryBufferObjecti64v) {
        void ** procp = (void **) &disp->GetQueryBufferObjecti64v;
        snprintf(symboln, sizeof(symboln), "%sGetQueryBufferObjecti64v", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetQueryBufferObjectiv) {
        void ** procp = (void **) &disp->GetQueryBufferObjectiv;
        snprintf(symboln, sizeof(symboln), "%sGetQueryBufferObjectiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetQueryBufferObjectui64v) {
        void ** procp = (void **) &disp->GetQueryBufferObjectui64v;
        snprintf(symboln, sizeof(symboln), "%sGetQueryBufferObjectui64v", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetQueryBufferObjectuiv) {
        void ** procp = (void **) &disp->GetQueryBufferObjectuiv;
        snprintf(symboln, sizeof(symboln), "%sGetQueryBufferObjectuiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTextureImage) {
        void ** procp = (void **) &disp->GetTextureImage;
        snprintf(symboln, sizeof(symboln), "%sGetTextureImage", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTextureLevelParameterfv) {
        void ** procp = (void **) &disp->GetTextureLevelParameterfv;
        snprintf(symboln, sizeof(symboln), "%sGetTextureLevelParameterfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTextureLevelParameteriv) {
        void ** procp = (void **) &disp->GetTextureLevelParameteriv;
        snprintf(symboln, sizeof(symboln), "%sGetTextureLevelParameteriv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTextureParameterIiv) {
        void ** procp = (void **) &disp->GetTextureParameterIiv;
        snprintf(symboln, sizeof(symboln), "%sGetTextureParameterIiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTextureParameterIuiv) {
        void ** procp = (void **) &disp->GetTextureParameterIuiv;
        snprintf(symboln, sizeof(symboln), "%sGetTextureParameterIuiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTextureParameterfv) {
        void ** procp = (void **) &disp->GetTextureParameterfv;
        snprintf(symboln, sizeof(symboln), "%sGetTextureParameterfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTextureParameteriv) {
        void ** procp = (void **) &disp->GetTextureParameteriv;
        snprintf(symboln, sizeof(symboln), "%sGetTextureParameteriv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTransformFeedbacki64_v) {
        void ** procp = (void **) &disp->GetTransformFeedbacki64_v;
        snprintf(symboln, sizeof(symboln), "%sGetTransformFeedbacki64_v", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTransformFeedbacki_v) {
        void ** procp = (void **) &disp->GetTransformFeedbacki_v;
        snprintf(symboln, sizeof(symboln), "%sGetTransformFeedbacki_v", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTransformFeedbackiv) {
        void ** procp = (void **) &disp->GetTransformFeedbackiv;
        snprintf(symboln, sizeof(symboln), "%sGetTransformFeedbackiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetVertexArrayIndexed64iv) {
        void ** procp = (void **) &disp->GetVertexArrayIndexed64iv;
        snprintf(symboln, sizeof(symboln), "%sGetVertexArrayIndexed64iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetVertexArrayIndexediv) {
        void ** procp = (void **) &disp->GetVertexArrayIndexediv;
        snprintf(symboln, sizeof(symboln), "%sGetVertexArrayIndexediv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetVertexArrayiv) {
        void ** procp = (void **) &disp->GetVertexArrayiv;
        snprintf(symboln, sizeof(symboln), "%sGetVertexArrayiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->InvalidateNamedFramebufferData) {
        void ** procp = (void **) &disp->InvalidateNamedFramebufferData;
        snprintf(symboln, sizeof(symboln), "%sInvalidateNamedFramebufferData", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->InvalidateNamedFramebufferSubData) {
        void ** procp = (void **) &disp->InvalidateNamedFramebufferSubData;
        snprintf(symboln, sizeof(symboln), "%sInvalidateNamedFramebufferSubData", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MapNamedBuffer) {
        void ** procp = (void **) &disp->MapNamedBuffer;
        snprintf(symboln, sizeof(symboln), "%sMapNamedBuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MapNamedBufferRange) {
        void ** procp = (void **) &disp->MapNamedBufferRange;
        snprintf(symboln, sizeof(symboln), "%sMapNamedBufferRange", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->NamedBufferData) {
        void ** procp = (void **) &disp->NamedBufferData;
        snprintf(symboln, sizeof(symboln), "%sNamedBufferData", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->NamedBufferStorage) {
        void ** procp = (void **) &disp->NamedBufferStorage;
        snprintf(symboln, sizeof(symboln), "%sNamedBufferStorage", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->NamedBufferSubData) {
        void ** procp = (void **) &disp->NamedBufferSubData;
        snprintf(symboln, sizeof(symboln), "%sNamedBufferSubData", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->NamedFramebufferDrawBuffer) {
        void ** procp = (void **) &disp->NamedFramebufferDrawBuffer;
        snprintf(symboln, sizeof(symboln), "%sNamedFramebufferDrawBuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->NamedFramebufferDrawBuffers) {
        void ** procp = (void **) &disp->NamedFramebufferDrawBuffers;
        snprintf(symboln, sizeof(symboln), "%sNamedFramebufferDrawBuffers", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->NamedFramebufferParameteri) {
        void ** procp = (void **) &disp->NamedFramebufferParameteri;
        snprintf(symboln, sizeof(symboln), "%sNamedFramebufferParameteri", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->NamedFramebufferReadBuffer) {
        void ** procp = (void **) &disp->NamedFramebufferReadBuffer;
        snprintf(symboln, sizeof(symboln), "%sNamedFramebufferReadBuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->NamedFramebufferRenderbuffer) {
        void ** procp = (void **) &disp->NamedFramebufferRenderbuffer;
        snprintf(symboln, sizeof(symboln), "%sNamedFramebufferRenderbuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->NamedFramebufferTexture) {
        void ** procp = (void **) &disp->NamedFramebufferTexture;
        snprintf(symboln, sizeof(symboln), "%sNamedFramebufferTexture", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->NamedFramebufferTextureLayer) {
        void ** procp = (void **) &disp->NamedFramebufferTextureLayer;
        snprintf(symboln, sizeof(symboln), "%sNamedFramebufferTextureLayer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->NamedRenderbufferStorage) {
        void ** procp = (void **) &disp->NamedRenderbufferStorage;
        snprintf(symboln, sizeof(symboln), "%sNamedRenderbufferStorage", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->NamedRenderbufferStorageMultisample) {
        void ** procp = (void **) &disp->NamedRenderbufferStorageMultisample;
        snprintf(symboln, sizeof(symboln), "%sNamedRenderbufferStorageMultisample", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TextureBuffer) {
        void ** procp = (void **) &disp->TextureBuffer;
        snprintf(symboln, sizeof(symboln), "%sTextureBuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TextureBufferRange) {
        void ** procp = (void **) &disp->TextureBufferRange;
        snprintf(symboln, sizeof(symboln), "%sTextureBufferRange", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TextureParameterIiv) {
        void ** procp = (void **) &disp->TextureParameterIiv;
        snprintf(symboln, sizeof(symboln), "%sTextureParameterIiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TextureParameterIuiv) {
        void ** procp = (void **) &disp->TextureParameterIuiv;
        snprintf(symboln, sizeof(symboln), "%sTextureParameterIuiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TextureParameterf) {
        void ** procp = (void **) &disp->TextureParameterf;
        snprintf(symboln, sizeof(symboln), "%sTextureParameterf", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TextureParameterfv) {
        void ** procp = (void **) &disp->TextureParameterfv;
        snprintf(symboln, sizeof(symboln), "%sTextureParameterfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TextureParameteri) {
        void ** procp = (void **) &disp->TextureParameteri;
        snprintf(symboln, sizeof(symboln), "%sTextureParameteri", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TextureParameteriv) {
        void ** procp = (void **) &disp->TextureParameteriv;
        snprintf(symboln, sizeof(symboln), "%sTextureParameteriv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TextureStorage1D) {
        void ** procp = (void **) &disp->TextureStorage1D;
        snprintf(symboln, sizeof(symboln), "%sTextureStorage1D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TextureStorage2D) {
        void ** procp = (void **) &disp->TextureStorage2D;
        snprintf(symboln, sizeof(symboln), "%sTextureStorage2D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TextureStorage2DMultisample) {
        void ** procp = (void **) &disp->TextureStorage2DMultisample;
        snprintf(symboln, sizeof(symboln), "%sTextureStorage2DMultisample", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TextureStorage3D) {
        void ** procp = (void **) &disp->TextureStorage3D;
        snprintf(symboln, sizeof(symboln), "%sTextureStorage3D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TextureStorage3DMultisample) {
        void ** procp = (void **) &disp->TextureStorage3DMultisample;
        snprintf(symboln, sizeof(symboln), "%sTextureStorage3DMultisample", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TextureSubImage1D) {
        void ** procp = (void **) &disp->TextureSubImage1D;
        snprintf(symboln, sizeof(symboln), "%sTextureSubImage1D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TextureSubImage2D) {
        void ** procp = (void **) &disp->TextureSubImage2D;
        snprintf(symboln, sizeof(symboln), "%sTextureSubImage2D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TextureSubImage3D) {
        void ** procp = (void **) &disp->TextureSubImage3D;
        snprintf(symboln, sizeof(symboln), "%sTextureSubImage3D", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TransformFeedbackBufferBase) {
        void ** procp = (void **) &disp->TransformFeedbackBufferBase;
        snprintf(symboln, sizeof(symboln), "%sTransformFeedbackBufferBase", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TransformFeedbackBufferRange) {
        void ** procp = (void **) &disp->TransformFeedbackBufferRange;
        snprintf(symboln, sizeof(symboln), "%sTransformFeedbackBufferRange", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UnmapNamedBuffer) {
        void ** procp = (void **) &disp->UnmapNamedBuffer;
        snprintf(symboln, sizeof(symboln), "%sUnmapNamedBuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexArrayAttribBinding) {
        void ** procp = (void **) &disp->VertexArrayAttribBinding;
        snprintf(symboln, sizeof(symboln), "%sVertexArrayAttribBinding", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexArrayAttribFormat) {
        void ** procp = (void **) &disp->VertexArrayAttribFormat;
        snprintf(symboln, sizeof(symboln), "%sVertexArrayAttribFormat", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexArrayAttribIFormat) {
        void ** procp = (void **) &disp->VertexArrayAttribIFormat;
        snprintf(symboln, sizeof(symboln), "%sVertexArrayAttribIFormat", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexArrayAttribLFormat) {
        void ** procp = (void **) &disp->VertexArrayAttribLFormat;
        snprintf(symboln, sizeof(symboln), "%sVertexArrayAttribLFormat", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexArrayBindingDivisor) {
        void ** procp = (void **) &disp->VertexArrayBindingDivisor;
        snprintf(symboln, sizeof(symboln), "%sVertexArrayBindingDivisor", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexArrayElementBuffer) {
        void ** procp = (void **) &disp->VertexArrayElementBuffer;
        snprintf(symboln, sizeof(symboln), "%sVertexArrayElementBuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexArrayVertexBuffer) {
        void ** procp = (void **) &disp->VertexArrayVertexBuffer;
        snprintf(symboln, sizeof(symboln), "%sVertexArrayVertexBuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexArrayVertexBuffers) {
        void ** procp = (void **) &disp->VertexArrayVertexBuffers;
        snprintf(symboln, sizeof(symboln), "%sVertexArrayVertexBuffers", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->InvalidateBufferData) {
        void ** procp = (void **) &disp->InvalidateBufferData;
        snprintf(symboln, sizeof(symboln), "%sInvalidateBufferData", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->InvalidateBufferSubData) {
        void ** procp = (void **) &disp->InvalidateBufferSubData;
        snprintf(symboln, sizeof(symboln), "%sInvalidateBufferSubData", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->InvalidateFramebuffer) {
        void ** procp = (void **) &disp->InvalidateFramebuffer;
        snprintf(symboln, sizeof(symboln), "%sInvalidateFramebuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->InvalidateSubFramebuffer) {
        void ** procp = (void **) &disp->InvalidateSubFramebuffer;
        snprintf(symboln, sizeof(symboln), "%sInvalidateSubFramebuffer", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->InvalidateTexImage) {
        void ** procp = (void **) &disp->InvalidateTexImage;
        snprintf(symboln, sizeof(symboln), "%sInvalidateTexImage", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->InvalidateTexSubImage) {
        void ** procp = (void **) &disp->InvalidateTexSubImage;
        snprintf(symboln, sizeof(symboln), "%sInvalidateTexSubImage", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PolygonOffsetEXT) {
        void ** procp = (void **) &disp->PolygonOffsetEXT;
        snprintf(symboln, sizeof(symboln), "%sPolygonOffsetEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawTexfOES) {
        void ** procp = (void **) &disp->DrawTexfOES;
        snprintf(symboln, sizeof(symboln), "%sDrawTexfOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawTexfvOES) {
        void ** procp = (void **) &disp->DrawTexfvOES;
        snprintf(symboln, sizeof(symboln), "%sDrawTexfvOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawTexiOES) {
        void ** procp = (void **) &disp->DrawTexiOES;
        snprintf(symboln, sizeof(symboln), "%sDrawTexiOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawTexivOES) {
        void ** procp = (void **) &disp->DrawTexivOES;
        snprintf(symboln, sizeof(symboln), "%sDrawTexivOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawTexsOES) {
        void ** procp = (void **) &disp->DrawTexsOES;
        snprintf(symboln, sizeof(symboln), "%sDrawTexsOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawTexsvOES) {
        void ** procp = (void **) &disp->DrawTexsvOES;
        snprintf(symboln, sizeof(symboln), "%sDrawTexsvOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawTexxOES) {
        void ** procp = (void **) &disp->DrawTexxOES;
        snprintf(symboln, sizeof(symboln), "%sDrawTexxOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DrawTexxvOES) {
        void ** procp = (void **) &disp->DrawTexxvOES;
        snprintf(symboln, sizeof(symboln), "%sDrawTexxvOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PointSizePointerOES) {
        void ** procp = (void **) &disp->PointSizePointerOES;
        snprintf(symboln, sizeof(symboln), "%sPointSizePointerOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->QueryMatrixxOES) {
        void ** procp = (void **) &disp->QueryMatrixxOES;
        snprintf(symboln, sizeof(symboln), "%sQueryMatrixxOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SampleMaskSGIS) {
        void ** procp = (void **) &disp->SampleMaskSGIS;
        snprintf(symboln, sizeof(symboln), "%sSampleMaskSGIS", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SampleMaskSGIS) {
        void ** procp = (void **) &disp->SampleMaskSGIS;
        snprintf(symboln, sizeof(symboln), "%sSampleMaskEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SamplePatternSGIS) {
        void ** procp = (void **) &disp->SamplePatternSGIS;
        snprintf(symboln, sizeof(symboln), "%sSamplePatternSGIS", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SamplePatternSGIS) {
        void ** procp = (void **) &disp->SamplePatternSGIS;
        snprintf(symboln, sizeof(symboln), "%sSamplePatternEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ColorPointerEXT) {
        void ** procp = (void **) &disp->ColorPointerEXT;
        snprintf(symboln, sizeof(symboln), "%sColorPointerEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EdgeFlagPointerEXT) {
        void ** procp = (void **) &disp->EdgeFlagPointerEXT;
        snprintf(symboln, sizeof(symboln), "%sEdgeFlagPointerEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IndexPointerEXT) {
        void ** procp = (void **) &disp->IndexPointerEXT;
        snprintf(symboln, sizeof(symboln), "%sIndexPointerEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->NormalPointerEXT) {
        void ** procp = (void **) &disp->NormalPointerEXT;
        snprintf(symboln, sizeof(symboln), "%sNormalPointerEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexCoordPointerEXT) {
        void ** procp = (void **) &disp->TexCoordPointerEXT;
        snprintf(symboln, sizeof(symboln), "%sTexCoordPointerEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexPointerEXT) {
        void ** procp = (void **) &disp->VertexPointerEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexPointerEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DiscardFramebufferEXT) {
        void ** procp = (void **) &disp->DiscardFramebufferEXT;
        snprintf(symboln, sizeof(symboln), "%sDiscardFramebufferEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ActiveShaderProgram) {
        void ** procp = (void **) &disp->ActiveShaderProgram;
        snprintf(symboln, sizeof(symboln), "%sActiveShaderProgram", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ActiveShaderProgram) {
        void ** procp = (void **) &disp->ActiveShaderProgram;
        snprintf(symboln, sizeof(symboln), "%sActiveShaderProgramEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindProgramPipeline) {
        void ** procp = (void **) &disp->BindProgramPipeline;
        snprintf(symboln, sizeof(symboln), "%sBindProgramPipeline", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindProgramPipeline) {
        void ** procp = (void **) &disp->BindProgramPipeline;
        snprintf(symboln, sizeof(symboln), "%sBindProgramPipelineEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CreateShaderProgramv) {
        void ** procp = (void **) &disp->CreateShaderProgramv;
        snprintf(symboln, sizeof(symboln), "%sCreateShaderProgramv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CreateShaderProgramv) {
        void ** procp = (void **) &disp->CreateShaderProgramv;
        snprintf(symboln, sizeof(symboln), "%sCreateShaderProgramvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteProgramPipelines) {
        void ** procp = (void **) &disp->DeleteProgramPipelines;
        snprintf(symboln, sizeof(symboln), "%sDeleteProgramPipelines", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteProgramPipelines) {
        void ** procp = (void **) &disp->DeleteProgramPipelines;
        snprintf(symboln, sizeof(symboln), "%sDeleteProgramPipelinesEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenProgramPipelines) {
        void ** procp = (void **) &disp->GenProgramPipelines;
        snprintf(symboln, sizeof(symboln), "%sGenProgramPipelines", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenProgramPipelines) {
        void ** procp = (void **) &disp->GenProgramPipelines;
        snprintf(symboln, sizeof(symboln), "%sGenProgramPipelinesEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetProgramPipelineInfoLog) {
        void ** procp = (void **) &disp->GetProgramPipelineInfoLog;
        snprintf(symboln, sizeof(symboln), "%sGetProgramPipelineInfoLog", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetProgramPipelineInfoLog) {
        void ** procp = (void **) &disp->GetProgramPipelineInfoLog;
        snprintf(symboln, sizeof(symboln), "%sGetProgramPipelineInfoLogEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetProgramPipelineiv) {
        void ** procp = (void **) &disp->GetProgramPipelineiv;
        snprintf(symboln, sizeof(symboln), "%sGetProgramPipelineiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetProgramPipelineiv) {
        void ** procp = (void **) &disp->GetProgramPipelineiv;
        snprintf(symboln, sizeof(symboln), "%sGetProgramPipelineivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsProgramPipeline) {
        void ** procp = (void **) &disp->IsProgramPipeline;
        snprintf(symboln, sizeof(symboln), "%sIsProgramPipeline", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->IsProgramPipeline) {
        void ** procp = (void **) &disp->IsProgramPipeline;
        snprintf(symboln, sizeof(symboln), "%sIsProgramPipelineEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LockArraysEXT) {
        void ** procp = (void **) &disp->LockArraysEXT;
        snprintf(symboln, sizeof(symboln), "%sLockArraysEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform1d) {
        void ** procp = (void **) &disp->ProgramUniform1d;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform1d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform1dv) {
        void ** procp = (void **) &disp->ProgramUniform1dv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform1dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform1f) {
        void ** procp = (void **) &disp->ProgramUniform1f;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform1f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform1f) {
        void ** procp = (void **) &disp->ProgramUniform1f;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform1fEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform1fv) {
        void ** procp = (void **) &disp->ProgramUniform1fv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform1fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform1fv) {
        void ** procp = (void **) &disp->ProgramUniform1fv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform1fvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform1i) {
        void ** procp = (void **) &disp->ProgramUniform1i;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform1i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform1i) {
        void ** procp = (void **) &disp->ProgramUniform1i;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform1iEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform1iv) {
        void ** procp = (void **) &disp->ProgramUniform1iv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform1iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform1iv) {
        void ** procp = (void **) &disp->ProgramUniform1iv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform1ivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform1ui) {
        void ** procp = (void **) &disp->ProgramUniform1ui;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform1ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform1ui) {
        void ** procp = (void **) &disp->ProgramUniform1ui;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform1uiEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform1uiv) {
        void ** procp = (void **) &disp->ProgramUniform1uiv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform1uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform1uiv) {
        void ** procp = (void **) &disp->ProgramUniform1uiv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform1uivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform2d) {
        void ** procp = (void **) &disp->ProgramUniform2d;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform2d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform2dv) {
        void ** procp = (void **) &disp->ProgramUniform2dv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform2dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform2f) {
        void ** procp = (void **) &disp->ProgramUniform2f;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform2f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform2f) {
        void ** procp = (void **) &disp->ProgramUniform2f;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform2fEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform2fv) {
        void ** procp = (void **) &disp->ProgramUniform2fv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform2fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform2fv) {
        void ** procp = (void **) &disp->ProgramUniform2fv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform2fvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform2i) {
        void ** procp = (void **) &disp->ProgramUniform2i;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform2i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform2i) {
        void ** procp = (void **) &disp->ProgramUniform2i;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform2iEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform2iv) {
        void ** procp = (void **) &disp->ProgramUniform2iv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform2iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform2iv) {
        void ** procp = (void **) &disp->ProgramUniform2iv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform2ivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform2ui) {
        void ** procp = (void **) &disp->ProgramUniform2ui;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform2ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform2ui) {
        void ** procp = (void **) &disp->ProgramUniform2ui;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform2uiEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform2uiv) {
        void ** procp = (void **) &disp->ProgramUniform2uiv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform2uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform2uiv) {
        void ** procp = (void **) &disp->ProgramUniform2uiv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform2uivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform3d) {
        void ** procp = (void **) &disp->ProgramUniform3d;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform3d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform3dv) {
        void ** procp = (void **) &disp->ProgramUniform3dv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform3dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform3f) {
        void ** procp = (void **) &disp->ProgramUniform3f;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform3f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform3f) {
        void ** procp = (void **) &disp->ProgramUniform3f;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform3fEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform3fv) {
        void ** procp = (void **) &disp->ProgramUniform3fv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform3fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform3fv) {
        void ** procp = (void **) &disp->ProgramUniform3fv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform3fvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform3i) {
        void ** procp = (void **) &disp->ProgramUniform3i;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform3i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform3i) {
        void ** procp = (void **) &disp->ProgramUniform3i;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform3iEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform3iv) {
        void ** procp = (void **) &disp->ProgramUniform3iv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform3iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform3iv) {
        void ** procp = (void **) &disp->ProgramUniform3iv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform3ivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform3ui) {
        void ** procp = (void **) &disp->ProgramUniform3ui;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform3ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform3ui) {
        void ** procp = (void **) &disp->ProgramUniform3ui;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform3uiEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform3uiv) {
        void ** procp = (void **) &disp->ProgramUniform3uiv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform3uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform3uiv) {
        void ** procp = (void **) &disp->ProgramUniform3uiv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform3uivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform4d) {
        void ** procp = (void **) &disp->ProgramUniform4d;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform4d", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform4dv) {
        void ** procp = (void **) &disp->ProgramUniform4dv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform4dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform4f) {
        void ** procp = (void **) &disp->ProgramUniform4f;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform4f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform4f) {
        void ** procp = (void **) &disp->ProgramUniform4f;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform4fEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform4fv) {
        void ** procp = (void **) &disp->ProgramUniform4fv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform4fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform4fv) {
        void ** procp = (void **) &disp->ProgramUniform4fv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform4fvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform4i) {
        void ** procp = (void **) &disp->ProgramUniform4i;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform4i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform4i) {
        void ** procp = (void **) &disp->ProgramUniform4i;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform4iEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform4iv) {
        void ** procp = (void **) &disp->ProgramUniform4iv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform4iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform4iv) {
        void ** procp = (void **) &disp->ProgramUniform4iv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform4ivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform4ui) {
        void ** procp = (void **) &disp->ProgramUniform4ui;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform4ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform4ui) {
        void ** procp = (void **) &disp->ProgramUniform4ui;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform4uiEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform4uiv) {
        void ** procp = (void **) &disp->ProgramUniform4uiv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform4uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniform4uiv) {
        void ** procp = (void **) &disp->ProgramUniform4uiv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniform4uivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix2dv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix2dv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix2dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix2fv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix2fv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix2fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix2fv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix2fv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix2fvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix2x3dv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix2x3dv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix2x3dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix2x3fv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix2x3fv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix2x3fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix2x3fv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix2x3fv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix2x3fvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix2x4dv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix2x4dv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix2x4dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix2x4fv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix2x4fv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix2x4fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix2x4fv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix2x4fv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix2x4fvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix3dv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix3dv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix3dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix3fv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix3fv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix3fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix3fv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix3fv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix3fvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix3x2dv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix3x2dv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix3x2dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix3x2fv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix3x2fv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix3x2fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix3x2fv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix3x2fv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix3x2fvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix3x4dv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix3x4dv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix3x4dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix3x4fv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix3x4fv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix3x4fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix3x4fv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix3x4fv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix3x4fvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix4dv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix4dv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix4dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix4fv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix4fv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix4fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix4fv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix4fv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix4fvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix4x2dv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix4x2dv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix4x2dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix4x2fv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix4x2fv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix4x2fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix4x2fv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix4x2fv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix4x2fvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix4x3dv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix4x3dv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix4x3dv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix4x3fv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix4x3fv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix4x3fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramUniformMatrix4x3fv) {
        void ** procp = (void **) &disp->ProgramUniformMatrix4x3fv;
        snprintf(symboln, sizeof(symboln), "%sProgramUniformMatrix4x3fvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UnlockArraysEXT) {
        void ** procp = (void **) &disp->UnlockArraysEXT;
        snprintf(symboln, sizeof(symboln), "%sUnlockArraysEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UseProgramStages) {
        void ** procp = (void **) &disp->UseProgramStages;
        snprintf(symboln, sizeof(symboln), "%sUseProgramStages", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UseProgramStages) {
        void ** procp = (void **) &disp->UseProgramStages;
        snprintf(symboln, sizeof(symboln), "%sUseProgramStagesEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ValidateProgramPipeline) {
        void ** procp = (void **) &disp->ValidateProgramPipeline;
        snprintf(symboln, sizeof(symboln), "%sValidateProgramPipeline", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ValidateProgramPipeline) {
        void ** procp = (void **) &disp->ValidateProgramPipeline;
        snprintf(symboln, sizeof(symboln), "%sValidateProgramPipelineEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DebugMessageCallback) {
        void ** procp = (void **) &disp->DebugMessageCallback;
        snprintf(symboln, sizeof(symboln), "%sDebugMessageCallbackARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DebugMessageCallback) {
        void ** procp = (void **) &disp->DebugMessageCallback;
        snprintf(symboln, sizeof(symboln), "%sDebugMessageCallback", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DebugMessageControl) {
        void ** procp = (void **) &disp->DebugMessageControl;
        snprintf(symboln, sizeof(symboln), "%sDebugMessageControlARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DebugMessageControl) {
        void ** procp = (void **) &disp->DebugMessageControl;
        snprintf(symboln, sizeof(symboln), "%sDebugMessageControl", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DebugMessageInsert) {
        void ** procp = (void **) &disp->DebugMessageInsert;
        snprintf(symboln, sizeof(symboln), "%sDebugMessageInsertARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DebugMessageInsert) {
        void ** procp = (void **) &disp->DebugMessageInsert;
        snprintf(symboln, sizeof(symboln), "%sDebugMessageInsert", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetDebugMessageLog) {
        void ** procp = (void **) &disp->GetDebugMessageLog;
        snprintf(symboln, sizeof(symboln), "%sGetDebugMessageLogARB", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetDebugMessageLog) {
        void ** procp = (void **) &disp->GetDebugMessageLog;
        snprintf(symboln, sizeof(symboln), "%sGetDebugMessageLog", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetObjectLabel) {
        void ** procp = (void **) &disp->GetObjectLabel;
        snprintf(symboln, sizeof(symboln), "%sGetObjectLabel", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetObjectPtrLabel) {
        void ** procp = (void **) &disp->GetObjectPtrLabel;
        snprintf(symboln, sizeof(symboln), "%sGetObjectPtrLabel", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ObjectLabel) {
        void ** procp = (void **) &disp->ObjectLabel;
        snprintf(symboln, sizeof(symboln), "%sObjectLabel", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ObjectPtrLabel) {
        void ** procp = (void **) &disp->ObjectPtrLabel;
        snprintf(symboln, sizeof(symboln), "%sObjectPtrLabel", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PopDebugGroup) {
        void ** procp = (void **) &disp->PopDebugGroup;
        snprintf(symboln, sizeof(symboln), "%sPopDebugGroup", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PushDebugGroup) {
        void ** procp = (void **) &disp->PushDebugGroup;
        snprintf(symboln, sizeof(symboln), "%sPushDebugGroup", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3fEXT) {
        void ** procp = (void **) &disp->SecondaryColor3fEXT;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3f", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3fEXT) {
        void ** procp = (void **) &disp->SecondaryColor3fEXT;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3fEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3fvEXT) {
        void ** procp = (void **) &disp->SecondaryColor3fvEXT;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3fv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SecondaryColor3fvEXT) {
        void ** procp = (void **) &disp->SecondaryColor3fvEXT;
        snprintf(symboln, sizeof(symboln), "%sSecondaryColor3fvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiDrawElementsEXT) {
        void ** procp = (void **) &disp->MultiDrawElementsEXT;
        snprintf(symboln, sizeof(symboln), "%sMultiDrawElements", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiDrawElementsEXT) {
        void ** procp = (void **) &disp->MultiDrawElementsEXT;
        snprintf(symboln, sizeof(symboln), "%sMultiDrawElementsEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FogCoordfEXT) {
        void ** procp = (void **) &disp->FogCoordfEXT;
        snprintf(symboln, sizeof(symboln), "%sFogCoordf", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FogCoordfEXT) {
        void ** procp = (void **) &disp->FogCoordfEXT;
        snprintf(symboln, sizeof(symboln), "%sFogCoordfEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FogCoordfvEXT) {
        void ** procp = (void **) &disp->FogCoordfvEXT;
        snprintf(symboln, sizeof(symboln), "%sFogCoordfv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FogCoordfvEXT) {
        void ** procp = (void **) &disp->FogCoordfvEXT;
        snprintf(symboln, sizeof(symboln), "%sFogCoordfvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ResizeBuffersMESA) {
        void ** procp = (void **) &disp->ResizeBuffersMESA;
        snprintf(symboln, sizeof(symboln), "%sResizeBuffersMESA", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos4dMESA) {
        void ** procp = (void **) &disp->WindowPos4dMESA;
        snprintf(symboln, sizeof(symboln), "%sWindowPos4dMESA", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos4dvMESA) {
        void ** procp = (void **) &disp->WindowPos4dvMESA;
        snprintf(symboln, sizeof(symboln), "%sWindowPos4dvMESA", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos4fMESA) {
        void ** procp = (void **) &disp->WindowPos4fMESA;
        snprintf(symboln, sizeof(symboln), "%sWindowPos4fMESA", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos4fvMESA) {
        void ** procp = (void **) &disp->WindowPos4fvMESA;
        snprintf(symboln, sizeof(symboln), "%sWindowPos4fvMESA", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos4iMESA) {
        void ** procp = (void **) &disp->WindowPos4iMESA;
        snprintf(symboln, sizeof(symboln), "%sWindowPos4iMESA", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos4ivMESA) {
        void ** procp = (void **) &disp->WindowPos4ivMESA;
        snprintf(symboln, sizeof(symboln), "%sWindowPos4ivMESA", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos4sMESA) {
        void ** procp = (void **) &disp->WindowPos4sMESA;
        snprintf(symboln, sizeof(symboln), "%sWindowPos4sMESA", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->WindowPos4svMESA) {
        void ** procp = (void **) &disp->WindowPos4svMESA;
        snprintf(symboln, sizeof(symboln), "%sWindowPos4svMESA", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiModeDrawArraysIBM) {
        void ** procp = (void **) &disp->MultiModeDrawArraysIBM;
        snprintf(symboln, sizeof(symboln), "%sMultiModeDrawArraysIBM", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiModeDrawElementsIBM) {
        void ** procp = (void **) &disp->MultiModeDrawElementsIBM;
        snprintf(symboln, sizeof(symboln), "%sMultiModeDrawElementsIBM", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->AreProgramsResidentNV) {
        void ** procp = (void **) &disp->AreProgramsResidentNV;
        snprintf(symboln, sizeof(symboln), "%sAreProgramsResidentNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ExecuteProgramNV) {
        void ** procp = (void **) &disp->ExecuteProgramNV;
        snprintf(symboln, sizeof(symboln), "%sExecuteProgramNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetProgramParameterdvNV) {
        void ** procp = (void **) &disp->GetProgramParameterdvNV;
        snprintf(symboln, sizeof(symboln), "%sGetProgramParameterdvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetProgramParameterfvNV) {
        void ** procp = (void **) &disp->GetProgramParameterfvNV;
        snprintf(symboln, sizeof(symboln), "%sGetProgramParameterfvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetProgramStringNV) {
        void ** procp = (void **) &disp->GetProgramStringNV;
        snprintf(symboln, sizeof(symboln), "%sGetProgramStringNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetProgramivNV) {
        void ** procp = (void **) &disp->GetProgramivNV;
        snprintf(symboln, sizeof(symboln), "%sGetProgramivNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTrackMatrixivNV) {
        void ** procp = (void **) &disp->GetTrackMatrixivNV;
        snprintf(symboln, sizeof(symboln), "%sGetTrackMatrixivNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetVertexAttribdvNV) {
        void ** procp = (void **) &disp->GetVertexAttribdvNV;
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribdvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetVertexAttribfvNV) {
        void ** procp = (void **) &disp->GetVertexAttribfvNV;
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribfvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetVertexAttribivNV) {
        void ** procp = (void **) &disp->GetVertexAttribivNV;
        snprintf(symboln, sizeof(symboln), "%sGetVertexAttribivNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LoadProgramNV) {
        void ** procp = (void **) &disp->LoadProgramNV;
        snprintf(symboln, sizeof(symboln), "%sLoadProgramNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramParameters4dvNV) {
        void ** procp = (void **) &disp->ProgramParameters4dvNV;
        snprintf(symboln, sizeof(symboln), "%sProgramParameters4dvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramParameters4fvNV) {
        void ** procp = (void **) &disp->ProgramParameters4fvNV;
        snprintf(symboln, sizeof(symboln), "%sProgramParameters4fvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->RequestResidentProgramsNV) {
        void ** procp = (void **) &disp->RequestResidentProgramsNV;
        snprintf(symboln, sizeof(symboln), "%sRequestResidentProgramsNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TrackMatrixNV) {
        void ** procp = (void **) &disp->TrackMatrixNV;
        snprintf(symboln, sizeof(symboln), "%sTrackMatrixNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib1dNV) {
        void ** procp = (void **) &disp->VertexAttrib1dNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1dNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib1dvNV) {
        void ** procp = (void **) &disp->VertexAttrib1dvNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1dvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib1fNV) {
        void ** procp = (void **) &disp->VertexAttrib1fNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1fNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib1fvNV) {
        void ** procp = (void **) &disp->VertexAttrib1fvNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1fvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib1sNV) {
        void ** procp = (void **) &disp->VertexAttrib1sNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1sNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib1svNV) {
        void ** procp = (void **) &disp->VertexAttrib1svNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib1svNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib2dNV) {
        void ** procp = (void **) &disp->VertexAttrib2dNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2dNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib2dvNV) {
        void ** procp = (void **) &disp->VertexAttrib2dvNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2dvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib2fNV) {
        void ** procp = (void **) &disp->VertexAttrib2fNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2fNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib2fvNV) {
        void ** procp = (void **) &disp->VertexAttrib2fvNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2fvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib2sNV) {
        void ** procp = (void **) &disp->VertexAttrib2sNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2sNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib2svNV) {
        void ** procp = (void **) &disp->VertexAttrib2svNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib2svNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib3dNV) {
        void ** procp = (void **) &disp->VertexAttrib3dNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3dNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib3dvNV) {
        void ** procp = (void **) &disp->VertexAttrib3dvNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3dvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib3fNV) {
        void ** procp = (void **) &disp->VertexAttrib3fNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3fNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib3fvNV) {
        void ** procp = (void **) &disp->VertexAttrib3fvNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3fvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib3sNV) {
        void ** procp = (void **) &disp->VertexAttrib3sNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3sNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib3svNV) {
        void ** procp = (void **) &disp->VertexAttrib3svNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib3svNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4dNV) {
        void ** procp = (void **) &disp->VertexAttrib4dNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4dNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4dvNV) {
        void ** procp = (void **) &disp->VertexAttrib4dvNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4dvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4fNV) {
        void ** procp = (void **) &disp->VertexAttrib4fNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4fNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4fvNV) {
        void ** procp = (void **) &disp->VertexAttrib4fvNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4fvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4sNV) {
        void ** procp = (void **) &disp->VertexAttrib4sNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4sNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4svNV) {
        void ** procp = (void **) &disp->VertexAttrib4svNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4svNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4ubNV) {
        void ** procp = (void **) &disp->VertexAttrib4ubNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4ubNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttrib4ubvNV) {
        void ** procp = (void **) &disp->VertexAttrib4ubvNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttrib4ubvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribPointerNV) {
        void ** procp = (void **) &disp->VertexAttribPointerNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribPointerNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribs1dvNV) {
        void ** procp = (void **) &disp->VertexAttribs1dvNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribs1dvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribs1fvNV) {
        void ** procp = (void **) &disp->VertexAttribs1fvNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribs1fvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribs1svNV) {
        void ** procp = (void **) &disp->VertexAttribs1svNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribs1svNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribs2dvNV) {
        void ** procp = (void **) &disp->VertexAttribs2dvNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribs2dvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribs2fvNV) {
        void ** procp = (void **) &disp->VertexAttribs2fvNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribs2fvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribs2svNV) {
        void ** procp = (void **) &disp->VertexAttribs2svNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribs2svNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribs3dvNV) {
        void ** procp = (void **) &disp->VertexAttribs3dvNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribs3dvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribs3fvNV) {
        void ** procp = (void **) &disp->VertexAttribs3fvNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribs3fvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribs3svNV) {
        void ** procp = (void **) &disp->VertexAttribs3svNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribs3svNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribs4dvNV) {
        void ** procp = (void **) &disp->VertexAttribs4dvNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribs4dvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribs4fvNV) {
        void ** procp = (void **) &disp->VertexAttribs4fvNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribs4fvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribs4svNV) {
        void ** procp = (void **) &disp->VertexAttribs4svNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribs4svNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribs4ubvNV) {
        void ** procp = (void **) &disp->VertexAttribs4ubvNV;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribs4ubvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTexBumpParameterfvATI) {
        void ** procp = (void **) &disp->GetTexBumpParameterfvATI;
        snprintf(symboln, sizeof(symboln), "%sGetTexBumpParameterfvATI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTexBumpParameterivATI) {
        void ** procp = (void **) &disp->GetTexBumpParameterivATI;
        snprintf(symboln, sizeof(symboln), "%sGetTexBumpParameterivATI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexBumpParameterfvATI) {
        void ** procp = (void **) &disp->TexBumpParameterfvATI;
        snprintf(symboln, sizeof(symboln), "%sTexBumpParameterfvATI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexBumpParameterivATI) {
        void ** procp = (void **) &disp->TexBumpParameterivATI;
        snprintf(symboln, sizeof(symboln), "%sTexBumpParameterivATI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->AlphaFragmentOp1ATI) {
        void ** procp = (void **) &disp->AlphaFragmentOp1ATI;
        snprintf(symboln, sizeof(symboln), "%sAlphaFragmentOp1ATI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->AlphaFragmentOp2ATI) {
        void ** procp = (void **) &disp->AlphaFragmentOp2ATI;
        snprintf(symboln, sizeof(symboln), "%sAlphaFragmentOp2ATI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->AlphaFragmentOp3ATI) {
        void ** procp = (void **) &disp->AlphaFragmentOp3ATI;
        snprintf(symboln, sizeof(symboln), "%sAlphaFragmentOp3ATI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BeginFragmentShaderATI) {
        void ** procp = (void **) &disp->BeginFragmentShaderATI;
        snprintf(symboln, sizeof(symboln), "%sBeginFragmentShaderATI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindFragmentShaderATI) {
        void ** procp = (void **) &disp->BindFragmentShaderATI;
        snprintf(symboln, sizeof(symboln), "%sBindFragmentShaderATI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ColorFragmentOp1ATI) {
        void ** procp = (void **) &disp->ColorFragmentOp1ATI;
        snprintf(symboln, sizeof(symboln), "%sColorFragmentOp1ATI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ColorFragmentOp2ATI) {
        void ** procp = (void **) &disp->ColorFragmentOp2ATI;
        snprintf(symboln, sizeof(symboln), "%sColorFragmentOp2ATI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ColorFragmentOp3ATI) {
        void ** procp = (void **) &disp->ColorFragmentOp3ATI;
        snprintf(symboln, sizeof(symboln), "%sColorFragmentOp3ATI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeleteFragmentShaderATI) {
        void ** procp = (void **) &disp->DeleteFragmentShaderATI;
        snprintf(symboln, sizeof(symboln), "%sDeleteFragmentShaderATI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EndFragmentShaderATI) {
        void ** procp = (void **) &disp->EndFragmentShaderATI;
        snprintf(symboln, sizeof(symboln), "%sEndFragmentShaderATI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenFragmentShadersATI) {
        void ** procp = (void **) &disp->GenFragmentShadersATI;
        snprintf(symboln, sizeof(symboln), "%sGenFragmentShadersATI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PassTexCoordATI) {
        void ** procp = (void **) &disp->PassTexCoordATI;
        snprintf(symboln, sizeof(symboln), "%sPassTexCoordATI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SampleMapATI) {
        void ** procp = (void **) &disp->SampleMapATI;
        snprintf(symboln, sizeof(symboln), "%sSampleMapATI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SetFragmentShaderConstantATI) {
        void ** procp = (void **) &disp->SetFragmentShaderConstantATI;
        snprintf(symboln, sizeof(symboln), "%sSetFragmentShaderConstantATI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ActiveStencilFaceEXT) {
        void ** procp = (void **) &disp->ActiveStencilFaceEXT;
        snprintf(symboln, sizeof(symboln), "%sActiveStencilFaceEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindVertexArrayAPPLE) {
        void ** procp = (void **) &disp->BindVertexArrayAPPLE;
        snprintf(symboln, sizeof(symboln), "%sBindVertexArrayAPPLE", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenVertexArraysAPPLE) {
        void ** procp = (void **) &disp->GenVertexArraysAPPLE;
        snprintf(symboln, sizeof(symboln), "%sGenVertexArraysAPPLE", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetProgramNamedParameterdvNV) {
        void ** procp = (void **) &disp->GetProgramNamedParameterdvNV;
        snprintf(symboln, sizeof(symboln), "%sGetProgramNamedParameterdvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetProgramNamedParameterfvNV) {
        void ** procp = (void **) &disp->GetProgramNamedParameterfvNV;
        snprintf(symboln, sizeof(symboln), "%sGetProgramNamedParameterfvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramNamedParameter4dNV) {
        void ** procp = (void **) &disp->ProgramNamedParameter4dNV;
        snprintf(symboln, sizeof(symboln), "%sProgramNamedParameter4dNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramNamedParameter4dvNV) {
        void ** procp = (void **) &disp->ProgramNamedParameter4dvNV;
        snprintf(symboln, sizeof(symboln), "%sProgramNamedParameter4dvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramNamedParameter4fNV) {
        void ** procp = (void **) &disp->ProgramNamedParameter4fNV;
        snprintf(symboln, sizeof(symboln), "%sProgramNamedParameter4fNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramNamedParameter4fvNV) {
        void ** procp = (void **) &disp->ProgramNamedParameter4fvNV;
        snprintf(symboln, sizeof(symboln), "%sProgramNamedParameter4fvNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PrimitiveRestartNV) {
        void ** procp = (void **) &disp->PrimitiveRestartNV;
        snprintf(symboln, sizeof(symboln), "%sPrimitiveRestartNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTexGenxvOES) {
        void ** procp = (void **) &disp->GetTexGenxvOES;
        snprintf(symboln, sizeof(symboln), "%sGetTexGenxvOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexGenxOES) {
        void ** procp = (void **) &disp->TexGenxOES;
        snprintf(symboln, sizeof(symboln), "%sTexGenxOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexGenxvOES) {
        void ** procp = (void **) &disp->TexGenxvOES;
        snprintf(symboln, sizeof(symboln), "%sTexGenxvOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DepthBoundsEXT) {
        void ** procp = (void **) &disp->DepthBoundsEXT;
        snprintf(symboln, sizeof(symboln), "%sDepthBoundsEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindFramebufferEXT) {
        void ** procp = (void **) &disp->BindFramebufferEXT;
        snprintf(symboln, sizeof(symboln), "%sBindFramebufferEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindRenderbufferEXT) {
        void ** procp = (void **) &disp->BindRenderbufferEXT;
        snprintf(symboln, sizeof(symboln), "%sBindRenderbufferEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BufferParameteriAPPLE) {
        void ** procp = (void **) &disp->BufferParameteriAPPLE;
        snprintf(symboln, sizeof(symboln), "%sBufferParameteriAPPLE", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->FlushMappedBufferRangeAPPLE) {
        void ** procp = (void **) &disp->FlushMappedBufferRangeAPPLE;
        snprintf(symboln, sizeof(symboln), "%sFlushMappedBufferRangeAPPLE", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI1iEXT) {
        void ** procp = (void **) &disp->VertexAttribI1iEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI1iEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI1iEXT) {
        void ** procp = (void **) &disp->VertexAttribI1iEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI1i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI1uiEXT) {
        void ** procp = (void **) &disp->VertexAttribI1uiEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI1uiEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI1uiEXT) {
        void ** procp = (void **) &disp->VertexAttribI1uiEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI1ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI2iEXT) {
        void ** procp = (void **) &disp->VertexAttribI2iEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI2iEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI2iEXT) {
        void ** procp = (void **) &disp->VertexAttribI2iEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI2i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI2ivEXT) {
        void ** procp = (void **) &disp->VertexAttribI2ivEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI2ivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI2ivEXT) {
        void ** procp = (void **) &disp->VertexAttribI2ivEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI2iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI2uiEXT) {
        void ** procp = (void **) &disp->VertexAttribI2uiEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI2uiEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI2uiEXT) {
        void ** procp = (void **) &disp->VertexAttribI2uiEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI2ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI2uivEXT) {
        void ** procp = (void **) &disp->VertexAttribI2uivEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI2uivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI2uivEXT) {
        void ** procp = (void **) &disp->VertexAttribI2uivEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI2uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI3iEXT) {
        void ** procp = (void **) &disp->VertexAttribI3iEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI3iEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI3iEXT) {
        void ** procp = (void **) &disp->VertexAttribI3iEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI3i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI3ivEXT) {
        void ** procp = (void **) &disp->VertexAttribI3ivEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI3ivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI3ivEXT) {
        void ** procp = (void **) &disp->VertexAttribI3ivEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI3iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI3uiEXT) {
        void ** procp = (void **) &disp->VertexAttribI3uiEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI3uiEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI3uiEXT) {
        void ** procp = (void **) &disp->VertexAttribI3uiEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI3ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI3uivEXT) {
        void ** procp = (void **) &disp->VertexAttribI3uivEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI3uivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI3uivEXT) {
        void ** procp = (void **) &disp->VertexAttribI3uivEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI3uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI4iEXT) {
        void ** procp = (void **) &disp->VertexAttribI4iEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI4iEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI4iEXT) {
        void ** procp = (void **) &disp->VertexAttribI4iEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI4i", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI4ivEXT) {
        void ** procp = (void **) &disp->VertexAttribI4ivEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI4ivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI4ivEXT) {
        void ** procp = (void **) &disp->VertexAttribI4ivEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI4iv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI4uiEXT) {
        void ** procp = (void **) &disp->VertexAttribI4uiEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI4uiEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI4uiEXT) {
        void ** procp = (void **) &disp->VertexAttribI4uiEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI4ui", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI4uivEXT) {
        void ** procp = (void **) &disp->VertexAttribI4uivEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI4uivEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VertexAttribI4uivEXT) {
        void ** procp = (void **) &disp->VertexAttribI4uivEXT;
        snprintf(symboln, sizeof(symboln), "%sVertexAttribI4uiv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearColorIiEXT) {
        void ** procp = (void **) &disp->ClearColorIiEXT;
        snprintf(symboln, sizeof(symboln), "%sClearColorIiEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearColorIuiEXT) {
        void ** procp = (void **) &disp->ClearColorIuiEXT;
        snprintf(symboln, sizeof(symboln), "%sClearColorIuiEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BindBufferOffsetEXT) {
        void ** procp = (void **) &disp->BindBufferOffsetEXT;
        snprintf(symboln, sizeof(symboln), "%sBindBufferOffsetEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BeginPerfMonitorAMD) {
        void ** procp = (void **) &disp->BeginPerfMonitorAMD;
        snprintf(symboln, sizeof(symboln), "%sBeginPerfMonitorAMD", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeletePerfMonitorsAMD) {
        void ** procp = (void **) &disp->DeletePerfMonitorsAMD;
        snprintf(symboln, sizeof(symboln), "%sDeletePerfMonitorsAMD", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EndPerfMonitorAMD) {
        void ** procp = (void **) &disp->EndPerfMonitorAMD;
        snprintf(symboln, sizeof(symboln), "%sEndPerfMonitorAMD", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GenPerfMonitorsAMD) {
        void ** procp = (void **) &disp->GenPerfMonitorsAMD;
        snprintf(symboln, sizeof(symboln), "%sGenPerfMonitorsAMD", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetPerfMonitorCounterDataAMD) {
        void ** procp = (void **) &disp->GetPerfMonitorCounterDataAMD;
        snprintf(symboln, sizeof(symboln), "%sGetPerfMonitorCounterDataAMD", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetPerfMonitorCounterInfoAMD) {
        void ** procp = (void **) &disp->GetPerfMonitorCounterInfoAMD;
        snprintf(symboln, sizeof(symboln), "%sGetPerfMonitorCounterInfoAMD", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetPerfMonitorCounterStringAMD) {
        void ** procp = (void **) &disp->GetPerfMonitorCounterStringAMD;
        snprintf(symboln, sizeof(symboln), "%sGetPerfMonitorCounterStringAMD", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetPerfMonitorCountersAMD) {
        void ** procp = (void **) &disp->GetPerfMonitorCountersAMD;
        snprintf(symboln, sizeof(symboln), "%sGetPerfMonitorCountersAMD", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetPerfMonitorGroupStringAMD) {
        void ** procp = (void **) &disp->GetPerfMonitorGroupStringAMD;
        snprintf(symboln, sizeof(symboln), "%sGetPerfMonitorGroupStringAMD", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetPerfMonitorGroupsAMD) {
        void ** procp = (void **) &disp->GetPerfMonitorGroupsAMD;
        snprintf(symboln, sizeof(symboln), "%sGetPerfMonitorGroupsAMD", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SelectPerfMonitorCountersAMD) {
        void ** procp = (void **) &disp->SelectPerfMonitorCountersAMD;
        snprintf(symboln, sizeof(symboln), "%sSelectPerfMonitorCountersAMD", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetObjectParameterivAPPLE) {
        void ** procp = (void **) &disp->GetObjectParameterivAPPLE;
        snprintf(symboln, sizeof(symboln), "%sGetObjectParameterivAPPLE", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ObjectPurgeableAPPLE) {
        void ** procp = (void **) &disp->ObjectPurgeableAPPLE;
        snprintf(symboln, sizeof(symboln), "%sObjectPurgeableAPPLE", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ObjectUnpurgeableAPPLE) {
        void ** procp = (void **) &disp->ObjectUnpurgeableAPPLE;
        snprintf(symboln, sizeof(symboln), "%sObjectUnpurgeableAPPLE", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ActiveProgramEXT) {
        void ** procp = (void **) &disp->ActiveProgramEXT;
        snprintf(symboln, sizeof(symboln), "%sActiveProgramEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CreateShaderProgramEXT) {
        void ** procp = (void **) &disp->CreateShaderProgramEXT;
        snprintf(symboln, sizeof(symboln), "%sCreateShaderProgramEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->UseShaderProgramEXT) {
        void ** procp = (void **) &disp->UseShaderProgramEXT;
        snprintf(symboln, sizeof(symboln), "%sUseShaderProgramEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TextureBarrierNV) {
        void ** procp = (void **) &disp->TextureBarrierNV;
        snprintf(symboln, sizeof(symboln), "%sTextureBarrier", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TextureBarrierNV) {
        void ** procp = (void **) &disp->TextureBarrierNV;
        snprintf(symboln, sizeof(symboln), "%sTextureBarrierNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VDPAUFiniNV) {
        void ** procp = (void **) &disp->VDPAUFiniNV;
        snprintf(symboln, sizeof(symboln), "%sVDPAUFiniNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VDPAUGetSurfaceivNV) {
        void ** procp = (void **) &disp->VDPAUGetSurfaceivNV;
        snprintf(symboln, sizeof(symboln), "%sVDPAUGetSurfaceivNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VDPAUInitNV) {
        void ** procp = (void **) &disp->VDPAUInitNV;
        snprintf(symboln, sizeof(symboln), "%sVDPAUInitNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VDPAUIsSurfaceNV) {
        void ** procp = (void **) &disp->VDPAUIsSurfaceNV;
        snprintf(symboln, sizeof(symboln), "%sVDPAUIsSurfaceNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VDPAUMapSurfacesNV) {
        void ** procp = (void **) &disp->VDPAUMapSurfacesNV;
        snprintf(symboln, sizeof(symboln), "%sVDPAUMapSurfacesNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VDPAURegisterOutputSurfaceNV) {
        void ** procp = (void **) &disp->VDPAURegisterOutputSurfaceNV;
        snprintf(symboln, sizeof(symboln), "%sVDPAURegisterOutputSurfaceNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VDPAURegisterVideoSurfaceNV) {
        void ** procp = (void **) &disp->VDPAURegisterVideoSurfaceNV;
        snprintf(symboln, sizeof(symboln), "%sVDPAURegisterVideoSurfaceNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VDPAUSurfaceAccessNV) {
        void ** procp = (void **) &disp->VDPAUSurfaceAccessNV;
        snprintf(symboln, sizeof(symboln), "%sVDPAUSurfaceAccessNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VDPAUUnmapSurfacesNV) {
        void ** procp = (void **) &disp->VDPAUUnmapSurfacesNV;
        snprintf(symboln, sizeof(symboln), "%sVDPAUUnmapSurfacesNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->VDPAUUnregisterSurfaceNV) {
        void ** procp = (void **) &disp->VDPAUUnregisterSurfaceNV;
        snprintf(symboln, sizeof(symboln), "%sVDPAUUnregisterSurfaceNV", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->BeginPerfQueryINTEL) {
        void ** procp = (void **) &disp->BeginPerfQueryINTEL;
        snprintf(symboln, sizeof(symboln), "%sBeginPerfQueryINTEL", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->CreatePerfQueryINTEL) {
        void ** procp = (void **) &disp->CreatePerfQueryINTEL;
        snprintf(symboln, sizeof(symboln), "%sCreatePerfQueryINTEL", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DeletePerfQueryINTEL) {
        void ** procp = (void **) &disp->DeletePerfQueryINTEL;
        snprintf(symboln, sizeof(symboln), "%sDeletePerfQueryINTEL", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EndPerfQueryINTEL) {
        void ** procp = (void **) &disp->EndPerfQueryINTEL;
        snprintf(symboln, sizeof(symboln), "%sEndPerfQueryINTEL", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetFirstPerfQueryIdINTEL) {
        void ** procp = (void **) &disp->GetFirstPerfQueryIdINTEL;
        snprintf(symboln, sizeof(symboln), "%sGetFirstPerfQueryIdINTEL", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetNextPerfQueryIdINTEL) {
        void ** procp = (void **) &disp->GetNextPerfQueryIdINTEL;
        snprintf(symboln, sizeof(symboln), "%sGetNextPerfQueryIdINTEL", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetPerfCounterInfoINTEL) {
        void ** procp = (void **) &disp->GetPerfCounterInfoINTEL;
        snprintf(symboln, sizeof(symboln), "%sGetPerfCounterInfoINTEL", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetPerfQueryDataINTEL) {
        void ** procp = (void **) &disp->GetPerfQueryDataINTEL;
        snprintf(symboln, sizeof(symboln), "%sGetPerfQueryDataINTEL", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetPerfQueryIdByNameINTEL) {
        void ** procp = (void **) &disp->GetPerfQueryIdByNameINTEL;
        snprintf(symboln, sizeof(symboln), "%sGetPerfQueryIdByNameINTEL", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetPerfQueryInfoINTEL) {
        void ** procp = (void **) &disp->GetPerfQueryInfoINTEL;
        snprintf(symboln, sizeof(symboln), "%sGetPerfQueryInfoINTEL", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PolygonOffsetClampEXT) {
        void ** procp = (void **) &disp->PolygonOffsetClampEXT;
        snprintf(symboln, sizeof(symboln), "%sPolygonOffsetClampEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->StencilFuncSeparateATI) {
        void ** procp = (void **) &disp->StencilFuncSeparateATI;
        snprintf(symboln, sizeof(symboln), "%sStencilFuncSeparateATI", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramEnvParameters4fvEXT) {
        void ** procp = (void **) &disp->ProgramEnvParameters4fvEXT;
        snprintf(symboln, sizeof(symboln), "%sProgramEnvParameters4fvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ProgramLocalParameters4fvEXT) {
        void ** procp = (void **) &disp->ProgramLocalParameters4fvEXT;
        snprintf(symboln, sizeof(symboln), "%sProgramLocalParameters4fvEXT", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EGLImageTargetRenderbufferStorageOES) {
        void ** procp = (void **) &disp->EGLImageTargetRenderbufferStorageOES;
        snprintf(symboln, sizeof(symboln), "%sEGLImageTargetRenderbufferStorageOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->EGLImageTargetTexture2DOES) {
        void ** procp = (void **) &disp->EGLImageTargetTexture2DOES;
        snprintf(symboln, sizeof(symboln), "%sEGLImageTargetTexture2DOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->AlphaFuncx) {
        void ** procp = (void **) &disp->AlphaFuncx;
        snprintf(symboln, sizeof(symboln), "%sAlphaFuncxOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->AlphaFuncx) {
        void ** procp = (void **) &disp->AlphaFuncx;
        snprintf(symboln, sizeof(symboln), "%sAlphaFuncx", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearColorx) {
        void ** procp = (void **) &disp->ClearColorx;
        snprintf(symboln, sizeof(symboln), "%sClearColorxOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearColorx) {
        void ** procp = (void **) &disp->ClearColorx;
        snprintf(symboln, sizeof(symboln), "%sClearColorx", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearDepthx) {
        void ** procp = (void **) &disp->ClearDepthx;
        snprintf(symboln, sizeof(symboln), "%sClearDepthxOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClearDepthx) {
        void ** procp = (void **) &disp->ClearDepthx;
        snprintf(symboln, sizeof(symboln), "%sClearDepthx", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color4x) {
        void ** procp = (void **) &disp->Color4x;
        snprintf(symboln, sizeof(symboln), "%sColor4xOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Color4x) {
        void ** procp = (void **) &disp->Color4x;
        snprintf(symboln, sizeof(symboln), "%sColor4x", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DepthRangex) {
        void ** procp = (void **) &disp->DepthRangex;
        snprintf(symboln, sizeof(symboln), "%sDepthRangexOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->DepthRangex) {
        void ** procp = (void **) &disp->DepthRangex;
        snprintf(symboln, sizeof(symboln), "%sDepthRangex", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Fogx) {
        void ** procp = (void **) &disp->Fogx;
        snprintf(symboln, sizeof(symboln), "%sFogxOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Fogx) {
        void ** procp = (void **) &disp->Fogx;
        snprintf(symboln, sizeof(symboln), "%sFogx", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Fogxv) {
        void ** procp = (void **) &disp->Fogxv;
        snprintf(symboln, sizeof(symboln), "%sFogxvOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Fogxv) {
        void ** procp = (void **) &disp->Fogxv;
        snprintf(symboln, sizeof(symboln), "%sFogxv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Frustumf) {
        void ** procp = (void **) &disp->Frustumf;
        snprintf(symboln, sizeof(symboln), "%sFrustumfOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Frustumf) {
        void ** procp = (void **) &disp->Frustumf;
        snprintf(symboln, sizeof(symboln), "%sFrustumf", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Frustumx) {
        void ** procp = (void **) &disp->Frustumx;
        snprintf(symboln, sizeof(symboln), "%sFrustumxOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Frustumx) {
        void ** procp = (void **) &disp->Frustumx;
        snprintf(symboln, sizeof(symboln), "%sFrustumx", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LightModelx) {
        void ** procp = (void **) &disp->LightModelx;
        snprintf(symboln, sizeof(symboln), "%sLightModelxOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LightModelx) {
        void ** procp = (void **) &disp->LightModelx;
        snprintf(symboln, sizeof(symboln), "%sLightModelx", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LightModelxv) {
        void ** procp = (void **) &disp->LightModelxv;
        snprintf(symboln, sizeof(symboln), "%sLightModelxvOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LightModelxv) {
        void ** procp = (void **) &disp->LightModelxv;
        snprintf(symboln, sizeof(symboln), "%sLightModelxv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Lightx) {
        void ** procp = (void **) &disp->Lightx;
        snprintf(symboln, sizeof(symboln), "%sLightxOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Lightx) {
        void ** procp = (void **) &disp->Lightx;
        snprintf(symboln, sizeof(symboln), "%sLightx", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Lightxv) {
        void ** procp = (void **) &disp->Lightxv;
        snprintf(symboln, sizeof(symboln), "%sLightxvOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Lightxv) {
        void ** procp = (void **) &disp->Lightxv;
        snprintf(symboln, sizeof(symboln), "%sLightxv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LineWidthx) {
        void ** procp = (void **) &disp->LineWidthx;
        snprintf(symboln, sizeof(symboln), "%sLineWidthxOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LineWidthx) {
        void ** procp = (void **) &disp->LineWidthx;
        snprintf(symboln, sizeof(symboln), "%sLineWidthx", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LoadMatrixx) {
        void ** procp = (void **) &disp->LoadMatrixx;
        snprintf(symboln, sizeof(symboln), "%sLoadMatrixxOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->LoadMatrixx) {
        void ** procp = (void **) &disp->LoadMatrixx;
        snprintf(symboln, sizeof(symboln), "%sLoadMatrixx", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Materialx) {
        void ** procp = (void **) &disp->Materialx;
        snprintf(symboln, sizeof(symboln), "%sMaterialxOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Materialx) {
        void ** procp = (void **) &disp->Materialx;
        snprintf(symboln, sizeof(symboln), "%sMaterialx", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Materialxv) {
        void ** procp = (void **) &disp->Materialxv;
        snprintf(symboln, sizeof(symboln), "%sMaterialxvOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Materialxv) {
        void ** procp = (void **) &disp->Materialxv;
        snprintf(symboln, sizeof(symboln), "%sMaterialxv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultMatrixx) {
        void ** procp = (void **) &disp->MultMatrixx;
        snprintf(symboln, sizeof(symboln), "%sMultMatrixxOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultMatrixx) {
        void ** procp = (void **) &disp->MultMatrixx;
        snprintf(symboln, sizeof(symboln), "%sMultMatrixx", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord4x) {
        void ** procp = (void **) &disp->MultiTexCoord4x;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4xOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->MultiTexCoord4x) {
        void ** procp = (void **) &disp->MultiTexCoord4x;
        snprintf(symboln, sizeof(symboln), "%sMultiTexCoord4x", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Normal3x) {
        void ** procp = (void **) &disp->Normal3x;
        snprintf(symboln, sizeof(symboln), "%sNormal3xOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Normal3x) {
        void ** procp = (void **) &disp->Normal3x;
        snprintf(symboln, sizeof(symboln), "%sNormal3x", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Orthof) {
        void ** procp = (void **) &disp->Orthof;
        snprintf(symboln, sizeof(symboln), "%sOrthofOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Orthof) {
        void ** procp = (void **) &disp->Orthof;
        snprintf(symboln, sizeof(symboln), "%sOrthof", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Orthox) {
        void ** procp = (void **) &disp->Orthox;
        snprintf(symboln, sizeof(symboln), "%sOrthoxOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Orthox) {
        void ** procp = (void **) &disp->Orthox;
        snprintf(symboln, sizeof(symboln), "%sOrthox", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PointSizex) {
        void ** procp = (void **) &disp->PointSizex;
        snprintf(symboln, sizeof(symboln), "%sPointSizexOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PointSizex) {
        void ** procp = (void **) &disp->PointSizex;
        snprintf(symboln, sizeof(symboln), "%sPointSizex", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PolygonOffsetx) {
        void ** procp = (void **) &disp->PolygonOffsetx;
        snprintf(symboln, sizeof(symboln), "%sPolygonOffsetxOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PolygonOffsetx) {
        void ** procp = (void **) &disp->PolygonOffsetx;
        snprintf(symboln, sizeof(symboln), "%sPolygonOffsetx", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Rotatex) {
        void ** procp = (void **) &disp->Rotatex;
        snprintf(symboln, sizeof(symboln), "%sRotatexOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Rotatex) {
        void ** procp = (void **) &disp->Rotatex;
        snprintf(symboln, sizeof(symboln), "%sRotatex", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SampleCoveragex) {
        void ** procp = (void **) &disp->SampleCoveragex;
        snprintf(symboln, sizeof(symboln), "%sSampleCoveragexOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->SampleCoveragex) {
        void ** procp = (void **) &disp->SampleCoveragex;
        snprintf(symboln, sizeof(symboln), "%sSampleCoveragex", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Scalex) {
        void ** procp = (void **) &disp->Scalex;
        snprintf(symboln, sizeof(symboln), "%sScalexOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Scalex) {
        void ** procp = (void **) &disp->Scalex;
        snprintf(symboln, sizeof(symboln), "%sScalex", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexEnvx) {
        void ** procp = (void **) &disp->TexEnvx;
        snprintf(symboln, sizeof(symboln), "%sTexEnvxOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexEnvx) {
        void ** procp = (void **) &disp->TexEnvx;
        snprintf(symboln, sizeof(symboln), "%sTexEnvx", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexEnvxv) {
        void ** procp = (void **) &disp->TexEnvxv;
        snprintf(symboln, sizeof(symboln), "%sTexEnvxvOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexEnvxv) {
        void ** procp = (void **) &disp->TexEnvxv;
        snprintf(symboln, sizeof(symboln), "%sTexEnvxv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexParameterx) {
        void ** procp = (void **) &disp->TexParameterx;
        snprintf(symboln, sizeof(symboln), "%sTexParameterxOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexParameterx) {
        void ** procp = (void **) &disp->TexParameterx;
        snprintf(symboln, sizeof(symboln), "%sTexParameterx", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Translatex) {
        void ** procp = (void **) &disp->Translatex;
        snprintf(symboln, sizeof(symboln), "%sTranslatexOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->Translatex) {
        void ** procp = (void **) &disp->Translatex;
        snprintf(symboln, sizeof(symboln), "%sTranslatex", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClipPlanef) {
        void ** procp = (void **) &disp->ClipPlanef;
        snprintf(symboln, sizeof(symboln), "%sClipPlanefOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClipPlanef) {
        void ** procp = (void **) &disp->ClipPlanef;
        snprintf(symboln, sizeof(symboln), "%sClipPlanef", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClipPlanex) {
        void ** procp = (void **) &disp->ClipPlanex;
        snprintf(symboln, sizeof(symboln), "%sClipPlanexOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->ClipPlanex) {
        void ** procp = (void **) &disp->ClipPlanex;
        snprintf(symboln, sizeof(symboln), "%sClipPlanex", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetClipPlanef) {
        void ** procp = (void **) &disp->GetClipPlanef;
        snprintf(symboln, sizeof(symboln), "%sGetClipPlanefOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetClipPlanef) {
        void ** procp = (void **) &disp->GetClipPlanef;
        snprintf(symboln, sizeof(symboln), "%sGetClipPlanef", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetClipPlanex) {
        void ** procp = (void **) &disp->GetClipPlanex;
        snprintf(symboln, sizeof(symboln), "%sGetClipPlanexOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetClipPlanex) {
        void ** procp = (void **) &disp->GetClipPlanex;
        snprintf(symboln, sizeof(symboln), "%sGetClipPlanex", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetFixedv) {
        void ** procp = (void **) &disp->GetFixedv;
        snprintf(symboln, sizeof(symboln), "%sGetFixedvOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetFixedv) {
        void ** procp = (void **) &disp->GetFixedv;
        snprintf(symboln, sizeof(symboln), "%sGetFixedv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetLightxv) {
        void ** procp = (void **) &disp->GetLightxv;
        snprintf(symboln, sizeof(symboln), "%sGetLightxvOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetLightxv) {
        void ** procp = (void **) &disp->GetLightxv;
        snprintf(symboln, sizeof(symboln), "%sGetLightxv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetMaterialxv) {
        void ** procp = (void **) &disp->GetMaterialxv;
        snprintf(symboln, sizeof(symboln), "%sGetMaterialxvOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetMaterialxv) {
        void ** procp = (void **) &disp->GetMaterialxv;
        snprintf(symboln, sizeof(symboln), "%sGetMaterialxv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTexEnvxv) {
        void ** procp = (void **) &disp->GetTexEnvxv;
        snprintf(symboln, sizeof(symboln), "%sGetTexEnvxvOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTexEnvxv) {
        void ** procp = (void **) &disp->GetTexEnvxv;
        snprintf(symboln, sizeof(symboln), "%sGetTexEnvxv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTexParameterxv) {
        void ** procp = (void **) &disp->GetTexParameterxv;
        snprintf(symboln, sizeof(symboln), "%sGetTexParameterxvOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->GetTexParameterxv) {
        void ** procp = (void **) &disp->GetTexParameterxv;
        snprintf(symboln, sizeof(symboln), "%sGetTexParameterxv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PointParameterx) {
        void ** procp = (void **) &disp->PointParameterx;
        snprintf(symboln, sizeof(symboln), "%sPointParameterxOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PointParameterx) {
        void ** procp = (void **) &disp->PointParameterx;
        snprintf(symboln, sizeof(symboln), "%sPointParameterx", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PointParameterxv) {
        void ** procp = (void **) &disp->PointParameterxv;
        snprintf(symboln, sizeof(symboln), "%sPointParameterxvOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->PointParameterxv) {
        void ** procp = (void **) &disp->PointParameterxv;
        snprintf(symboln, sizeof(symboln), "%sPointParameterxv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexParameterxv) {
        void ** procp = (void **) &disp->TexParameterxv;
        snprintf(symboln, sizeof(symboln), "%sTexParameterxvOES", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    if(!disp->TexParameterxv) {
        void ** procp = (void **) &disp->TexParameterxv;
        snprintf(symboln, sizeof(symboln), "%sTexParameterxv", symbol_prefix);
#ifdef _WIN32
        *procp = GetProcAddress(handle, symboln);
#else
        *procp = dlsym(handle, symboln);
#endif
    }


    __glapi_gentable_set_remaining_noop(disp);

    return disp;
}

