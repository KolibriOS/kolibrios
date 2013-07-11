/**************************************************************************
 *
 * Copyright 2010 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 *
 **************************************************************************/

#include "sw/kos_sw_winsys.h"
#include "pipe/p_screen.h"
#include "state_tracker/graw.h"
#include "target-helpers/inline_debug_helper.h"
#include "target-helpers/inline_sw_helper.h"
#include <kos32sys.h>
//#include <windows.h>

/*
static LRESULT CALLBACK
window_proc(HWND hWnd,
            UINT uMsg,
            WPARAM wParam,
            LPARAM lParam)
{
   switch (uMsg) {
   case WM_DESTROY:
      PostQuitMessage(0);
      break;

   default:
      return DefWindowProc(hWnd, uMsg, wParam, lParam);
   }

   return 0;
}
*/

static struct {
   void (* draw)(void);
} graw;

struct pipe_screen *
graw_create_window_and_screen(int x,
                              int y,
                              unsigned width,
                              unsigned height,
                              enum pipe_format format,
                              void **handle)
{
   struct sw_winsys *winsys = NULL;
   struct pipe_screen *screen = NULL;

   if (format != PIPE_FORMAT_B8G8R8X8_UNORM)
      goto fail;

   winsys = kos_create_sw_winsys();
   if (winsys == NULL)
      goto fail;

   screen = sw_screen_create(winsys);
   
   if (screen == NULL)
      goto fail;

    BeginDraw();
    DrawWindow(x, y,width-1,height-1,
               NULL,0,0x41);
    EndDraw();
    
    *handle = (void *)winsys; 
    return (screen);

fail:
  
   if (screen)
      screen->destroy(screen);

   return NULL;
}

void 
graw_set_display_func(void (* draw)(void))
{
   graw.draw = draw;
}

void
graw_main_loop(void)
{
    int ev;
    oskey_t   key;

    BeginDraw();
    DrawWindow(0,0,0,0, NULL, 0x000000,0x41);
    EndDraw();
            
    if (graw.draw)
    {
       graw.draw();
    }

    while(1)
    {
        ev = wait_for_event(2);

        switch(ev)
        {
            case 1:
                BeginDraw();
                DrawWindow(0,0,0,0, NULL, 0x000000,0x41);
                EndDraw();
            
                if (graw.draw)
                {
                    graw.draw();
                }
                continue;

            case 2:
                key = get_key();
//                printf("key %x\n", key.code);
                if( key.code == 0x1b)
                    return;
                continue;
        };
        if (graw.draw)
        {
            graw.draw();
        }
        
    };
}
