/*
 * Copyright (C) 2010 LunarG Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Chia-I Wu <olv@lunarg.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define EGL_EGLEXT_PROTOTYPES
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "gbm.h"
#include <kos32sys.h>

#include "eglutint.h"

#define MAX_MODES 100

//static EGLScreenMESA kms_screen;
//static EGLModeMESA kms_mode;
//static EGLint kms_width, kms_height;

void
_eglutNativeInitDisplay(void)
{
   struct gbm_device *gbm;
   int fd;

   fd = get_service("DISPLAY");
   gbm = gbm_create_device(fd);
   if( gbm == NULL){
      _eglutFatal("failed to initialize GBM device");
      exit(1);
   };

//   init_pixlib(HW_BIT_BLIT);

   _eglut->native_dpy = gbm;
   _eglut->surface_type = EGL_WINDOW_BIT;
}

void
_eglutNativeFiniDisplay(void)
{
//    done_pixlib();
}

#if 0
static void
init_kms(void)
{
   EGLModeMESA modes[MAX_MODES];
   EGLint num_screens, num_modes;
   EGLint width, height, best_mode;
   EGLint i;

   if (!eglGetScreensMESA(_eglut->dpy, &kms_screen, 1, &num_screens) ||
         !num_screens)
      _eglutFatal("eglGetScreensMESA failed\n");

   if (!eglGetModesMESA(_eglut->dpy, kms_screen,
            modes, MAX_MODES, &num_modes) || !num_modes)
      _eglutFatal("eglGetModesMESA failed!\n");

   printf("Found %d modes:\n", num_modes);

   best_mode = 0;
   width = 0;
   height = 0;
   for (i = 0; i < num_modes; i++) {
      EGLint w, h;
      eglGetModeAttribMESA(_eglut->dpy, modes[i], EGL_WIDTH, &w);
      eglGetModeAttribMESA(_eglut->dpy, modes[i], EGL_HEIGHT, &h);
      printf("%3d: %d x %d\n", i, w, h);
      if (w > width && h > height) {
         width = w;
         height = h;
         best_mode = i;
      }
   }

   printf("Will use screen size: %d x %d\n", width, height);

   kms_mode = modes[best_mode];
   kms_width = width;
   kms_height = height;
}
#endif

void
_eglutNativeInitWindow(struct eglut_window *win, const char *title,
                       int x, int y, int w, int h)
{
    struct gbm_device   *gbm = _eglut->native_dpy;
    struct gbm_surface  *gs;

    gs = gbm_surface_create(gbm, w, h, GBM_BO_FORMAT_ARGB8888, GBM_BO_USE_RENDERING);

    win->native.u.surface = gs;

    win->native.width = w;
    win->native.height = h;

    BeginDraw();
    DrawWindow(x, y, w+TYPE_3_BORDER_WIDTH*2,
               h+TYPE_3_BORDER_WIDTH+get_skin_height(), title, 0x000000, 0x74);

    EndDraw();

//    sna_create_mask();
}

void
_eglutNativeFiniWindow(struct eglut_window *win)
{
//   eglShowScreenSurfaceMESA(_eglut->dpy,
//         kms_screen, EGL_NO_SURFACE, 0);
//   eglDestroySurface(_eglut->dpy, win->native.u.surface);
}

void
_eglutNativeEventLoop(void)
{
    struct eglut_window *win = _eglut->current;
    int start = _eglutNow();
    int frames = 0;
    int run = 1;
    _eglut->redisplay = 1;

    while (run)
    {
        int now = _eglutNow();
        oskey_t   key;
        int ev;

        ev = check_os_event();
//        ev = get_os_event();
        switch(ev)
        {
            case 1:
                BeginDraw();
                DrawWindow(10,10,10,10,NULL,0,0x74);
                EndDraw();
                break;

            case 2:
                key = get_key();
                if (key.code == 27)
                {
                    run = 0;
                    continue;
                }
                if(win->keyboard_cb)
                    win->keyboard_cb(key.code);
                break;

            case 3:
                if(get_os_button()==1)
                {
                    run = 0;
                    continue;
                }
                break;
        };

        if (now - start >= 5000)
        {
            double elapsed = (double) (now - start) / 1000.0;

            printf("%d frames in %3.1f seconds = %6.3f FPS\n",
                    frames, elapsed, frames / elapsed);
            fflush(stdout);

            start = now;
            frames = 0;
        }

        if (_eglut->idle_cb)
            _eglut->idle_cb();

        if (_eglut->redisplay)
        {
            _eglut->redisplay = 0;

            if (win->display_cb)
                win->display_cb();

            render_swap_and_blit(_eglut->render);
            frames++;
        }
   }
 //  glFinish();
   eglutDestroyWindow(_eglut->current->index);
   eglutFini();
}
