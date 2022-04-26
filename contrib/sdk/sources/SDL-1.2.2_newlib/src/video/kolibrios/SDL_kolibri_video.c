#include <stdlib.h>
#include <stdio.h>
#include <sys/ksys.h>
#include "SDL.h"
#include "SDL_error.h"
#include "SDL_video.h"
#include "SDL_mouse.h"
#include "SDL_sysvideo.h"
#include "SDL_pixels_c.h"
#include "SDL_kolibri_video.h"
#include <string.h>

static SDL_VideoDevice * vm_suf=NULL;
static int was_initialized = 0;
static int scrn_size_defined = 0;

static int has_null_cursor=0;
static void* null_cursor;

#define WINDOW_BORDER_H 4
#define WINDOW_BORDER_W 9

ksys_pos_t screen_size = {0};

void kos_SDL_RepaintWnd(void)
{
    int win_pos_x, win_pos_y;
    int win_size_w = vm_suf->hidden->win_size_x+WINDOW_BORDER_W;
    int win_size_h = vm_suf->hidden->win_size_y+_ksys_get_skin_height()+WINDOW_BORDER_H;

    if (!screen_size.val) {
        screen_size = _ksys_screen_size();
        win_pos_x = screen_size.x/2-win_size_w/2;
        win_pos_y = screen_size.y/2-win_size_h/2;
    }

    _ksys_start_draw();
    _ksys_create_window(win_pos_x, win_pos_y, win_size_w, win_size_h, vm_suf->hidden->__title, 0, 0x34);

    if (vm_suf && vm_suf->hidden->__video_buffer) {
        _ksys_draw_bitmap(vm_suf->hidden->__video_buffer, 0, 0,
                          vm_suf->hidden->win_size_x, vm_suf->hidden->win_size_y);
    }
    _ksys_end_draw();
}

static int kos_AllocHWSurface(_THIS,SDL_Surface * surface)
{
    return -1;
}

static void kos_FreeHWSurface(_THIS,SDL_Surface * surface) {/*STUB*/}

static int kos_LockHWSurface(_THIS,SDL_Surface * surface)
{
    return 0;
}

static void kos_UnlockHWSurface(_THIS,SDL_Surface * surface) {/*STUB*/}

static void kos_DirectUpdate(_THIS,int numrects,SDL_Rect * rects)
{
    if (numrects) {
        _ksys_draw_bitmap(this->hidden->__video_buffer, 0,0,
                          vm_suf->hidden->win_size_x,vm_suf->hidden->win_size_y);
    }
}

int kos_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors)
{
    return 0;
}

void kos_VideoQuit(_THIS)
{
    if (has_null_cursor) {
        _ksys_delete_cursor(null_cursor);
        has_null_cursor = 0;
    }
}

void kos_FinalQuit(void) {/*STUB*/}

void kos_SetCaption(_THIS,const char * title, const char * icon)
{
    this->hidden->__title=(char *)title;
    if (was_initialized) _ksys_set_window_title(title);
}

SDL_Surface *kos_SetVideoMode(_THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags)
{
    int ly;
    unsigned char* lx;
    if (bpp!=24) return NULL;
 
    current->flags=flags;
    current->w=width;
    current->h=height;
    current->pitch=width*(bpp>>3);
 
    char info[100];
    sprintf(info, "width = %d, height = %d, pitch = %d, bpp = %d\n", current->w, current->h, current->pitch, bpp);
    _ksys_debug_puts(info);

    current->pixels=this->hidden->__video_buffer=realloc(this->hidden->__video_buffer, current->pitch*current->h);
    this->hidden->__lines=(unsigned char **)realloc(this->hidden->__lines, sizeof(unsigned char *)*current->h);

    for (ly=0, lx=current->pixels; ly<current->h; ly++, lx+=current->pitch)
            this->hidden->__lines[ly]=lx;

    this->UpdateRects=kos_DirectUpdate;
    this->hidden->win_size_x=width;
    this->hidden->win_size_y=height;
    vm_suf=this;

    if (was_initialized) {
        unsigned newheight = height+_ksys_get_skin_height()+WINDOW_BORDER_H;
        unsigned newwidth  = width+WINDOW_BORDER_W;
        int win_pos_x = screen_size.x/2-newwidth/2;
        int win_pos_y = screen_size.y/2-newheight/2;
        _ksys_change_window(win_pos_x, win_pos_y, newwidth, newheight);
    } else {
        _ksys_set_event_mask(0x27);
        was_initialized=1;
        kos_SDL_RepaintWnd();
    }
    return current;
}

/*static SDL_Rect video_mode[4];
static SDL_Rect * SDL_modelist[4]={NULL,NULL,NULL,NULL};*/

static SDL_Rect** kos_ListModes(_THIS,SDL_PixelFormat * fmt,Uint32 flags)
{
// return (&SDL_modelist[((fmt->BitsPerPixel+7)/8)-1]);
    if (fmt->BitsPerPixel==24)
        return (SDL_Rect**)-1;
    else
        return NULL;
}

static int kos_Available(void)
{
    return 1;
}

static void kos_DeleteDevice(_THIS)
{
//  free(this->hidden->__video_buffer);	// it will be freed as current->pixels
    free(this->hidden->__lines);
}

static int kos_VideoInit(_THIS,SDL_PixelFormat * vformat)
{
    vformat->BitsPerPixel = 24;
    vformat->BytesPerPixel = 3;
    this->info.wm_available = 1;
    this->info.hw_available = 0;
    this->info.video_mem = 0x200000;
    return 0;
}

static int kos_FlipHWSurface(_THIS,SDL_Surface * surface)
{
    _ksys_draw_bitmap(surface->pixels, 0, 0, surface->w,surface->h);
    return 0;
}

WMcursor* kos_CreateWMCursor(_THIS, Uint8* data, Uint8* mask, int w, int h, int hot_x, int hot_y)
{
    int i,j;
    Uint32* cursor;
    WMcursor* res;

    if (w>32 || h>32) return NULL;
    if (w%8 || h%8) return NULL;
    cursor = (Uint32*)malloc(32*32*4);
    if (!cursor) return NULL;
    for (i=0;i<32;i++) {
        for (j=0;j<32;j++) {
            if (i>=h || j>=w) {
                cursor[i*32+j] = 0x00000000;
                continue;
            }
            if (mask[i*w/8+j/8] & (0x80>>(j&7)))
                cursor[i*32+j] = (data[i*w/8+j/8] & (0x80>>(j&7)))?0xFF000000:0xFFFFFFFF;
            else
                cursor[i*32+j] = 0x00000000;
        }
    }
    res = _ksys_load_cursor(cursor, (hot_x<<24)+(hot_y<<16)+KSYS_CURSOR_INDIRECT);
    free(cursor);
    return res;
}

int kos_ShowWMCursor(_THIS, WMcursor* cursor)
{
    if (!cursor) {
        if (!has_null_cursor) {
            unsigned* u = malloc(32*32*4);
            if (!u) return 1;
            memset(u, 0, 32*32*4);
            null_cursor = _ksys_load_cursor(u, KSYS_CURSOR_INDIRECT);
            free(u);
            has_null_cursor = 1;
        }
        cursor = (WMcursor*)null_cursor;
    }
    _ksys_set_cursor(cursor);
    return 1;
}
void kos_FreeWMCursor(_THIS, WMcursor* cursor)
{
    _ksys_delete_cursor(cursor);
}

void kos_CheckMouseMode(_THIS)
{
    if (this->input_grab == SDL_GRAB_OFF)
        return;
    ksys_thread_t thread_info;
    int top = _ksys_thread_info(&thread_info, KSYS_THIS_SLOT);
    
    if (top == thread_info.pos_in_window_stack) {
        int x = thread_info.winx_start + thread_info.clientx + this->hidden->win_size_x/2;
        int y = thread_info.winy_start + thread_info.clienty + this->hidden->win_size_y/2;
        _ksys_set_mouse_pos(x, y);
    }
}

char def_title[] = "KolibriOS SDL App";

static SDL_VideoDevice *kos_CreateDevice(int indx)
{
    SDL_VideoDevice * dev;
    dev = (SDL_VideoDevice *)malloc(sizeof(SDL_VideoDevice));
    if (dev) {
        memset(dev,0,(sizeof *dev));
        dev->hidden = (struct SDL_PrivateVideoData*)malloc((sizeof *dev->hidden));
    }

    if ((dev==NULL) || (dev->hidden==NULL)) {
        SDL_OutOfMemory();
        if(dev) {
           free(dev);
        }
        return(0);
    }

    memset(dev->hidden, 0, (sizeof *dev->hidden));
    dev->hidden->__title = def_title;
    dev->VideoInit = kos_VideoInit;
    dev->ListModes = kos_ListModes;
    dev->SetVideoMode = kos_SetVideoMode;
    dev->SetColors = kos_SetColors;
    dev->UpdateRects = NULL;
    dev->VideoQuit = kos_VideoQuit;
    dev->AllocHWSurface=kos_AllocHWSurface;
    dev->CheckHWBlit = NULL;
    dev->FillHWRect = NULL;
    dev->SetHWColorKey = NULL;
    dev->SetHWAlpha = NULL;
    dev->LockHWSurface = kos_LockHWSurface;
    dev->UnlockHWSurface = kos_UnlockHWSurface;
    dev->FlipHWSurface = kos_FlipHWSurface;
    dev->FreeHWSurface = kos_FreeHWSurface;
    dev->SetCaption = kos_SetCaption;
    dev->SetIcon = NULL;
    dev->IconifyWindow = NULL;
    dev->GrabInput = NULL;
    dev->GetWMInfo = NULL;
    dev->InitOSKeymap = kos_InitOSKeymap;
    dev->PumpEvents	= kos_PumpEvents;
    dev->free = kos_DeleteDevice;
    dev->CreateWMCursor = kos_CreateWMCursor;
    dev->FreeWMCursor = kos_FreeWMCursor;
    dev->ShowWMCursor = kos_ShowWMCursor;
    dev->CheckMouseMode = kos_CheckMouseMode;
    return dev;
}

VideoBootStrap kos_video_bootstrab = {
    "kolibrios", "KolibriOS Device Driver",
    kos_Available, kos_CreateDevice,
};
