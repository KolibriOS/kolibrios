#include <stdlib.h>
#include <stdio.h>
#include <menuet/os.h>
#include "SDL.h"
#include "SDL_error.h"
#include "SDL_video.h"
#include "SDL_mouse.h"
#include "SDL_sysvideo.h"
#include "SDL_pixels_c.h"
#include "SDL_events_c.h"
#include "SDL_menuetvideo.h"
#include <string.h>

static SDL_VideoDevice * vm_suf=NULL;
static int was_initialized=0;

static int has_null_cursor=0;
static int null_cursor;

inline int get_skinh(void)
{
	int res;
	__asm__ ("int $0x40" : "=a"(res) : "a"(48),"b"(4));
	return res;
}

//#define KEEP_OBSOLETE_STYLE3

#ifdef KEEP_OBSOLETE_STYLE3
static int IsStyle4Available=0;
#endif

void MenuetOS_SDL_RepaintWnd(void)
{
 __menuet__window_redraw(1);
 __menuet__define_window(1,1,vm_suf->hidden->win_size_x+9,vm_suf->hidden->win_size_y+get_skinh()+4,
#ifdef KEEP_OBSOLETE_STYLE3
 	IsStyle4Available?0x34000000:0x33000000
#else
	0x34000000
#endif
 	,0,(int)vm_suf->hidden->__title);

 // __asm__ __volatile__("int3");

 if(vm_suf && vm_suf->hidden->__video_buffer)
  __menuet__putimage(0,0,
   vm_suf->hidden->win_size_x,vm_suf->hidden->win_size_y,
   vm_suf->hidden->__video_buffer);
 __menuet__window_redraw(2);
}

static int MenuetOS_AllocHWSurface(_THIS,SDL_Surface * surface)
{
 return -1;
}

static void MenuetOS_FreeHWSurface(_THIS,SDL_Surface * surface)
{
}

static int MenuetOS_LockHWSurface(_THIS,SDL_Surface * surface)
{
 return 0;
}

static void MenuetOS_UnlockHWSurface(_THIS,SDL_Surface * surface)
{
}

static void MenuetOS_DirectUpdate(_THIS,int numrects,SDL_Rect * rects)
{
 if(numrects)
 {
  __menuet__putimage(0,0,
   vm_suf->hidden->win_size_x,vm_suf->hidden->win_size_y,
   this->hidden->__video_buffer);
 }
}

int MenuetOS_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors)
{
 return 0;
}

void MenuetOS_VideoQuit(_THIS)
{
	if (has_null_cursor)
	{
		__asm__("int $0x40"::"a"(37),"b"(6),"c"(null_cursor));
		has_null_cursor = 0;
	}
}

void MenuetOS_FinalQuit(void)
{
}

void MenuetOS_SetCaption(_THIS,const char * title,const char * icon)
{
 this->hidden->__title=(char *)title;
 if(was_initialized) __asm__("int $0x40"::"a"(71),"b"(1),"c"(title));
}

SDL_Surface * MenuetOS_SetVideoMode(_THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags)
{
 int ly;
 char * lx;
 if(bpp!=24) return NULL;
 current->flags=flags;
 current->w=width;
 current->h=height;
 current->pitch=width*(bpp>>3);

 char info[100];
 sprintf(info, "width = %d, height = %d, pitch = %d, bpp = %d\n", current->w, current->h, current->pitch, bpp);

 void debug_board_write_byte(const char ch){
   __asm__ __volatile__(
			"int $0x40"
			:
			:"a"(63), "b"(1), "c"(ch));
 }

 void debug_board_write_str(const char* str){
   while(*str)
     debug_board_write_byte(*str++);
 }

 debug_board_write_str(info);
 // __asm__ __volatile__("int3");
 
 current->pixels=this->hidden->__video_buffer=realloc(this->hidden->__video_buffer,
	current->pitch*current->h);
 this->hidden->__lines=(unsigned char **)realloc(this->hidden->__lines,
		    sizeof(unsigned char *)*current->h);

 for(ly=0,lx=current->pixels;ly<current->h;ly++,lx+=current->pitch)
   this->hidden->__lines[ly]=lx;

 this->UpdateRects=MenuetOS_DirectUpdate;
 this->hidden->win_size_x=width;
 this->hidden->win_size_y=height;
 vm_suf=this;
 if (was_initialized)
 {
  unsigned newheight = height+get_skinh()+4;
  unsigned newwidth = width+9;
  __asm__("int $0x40"::"a"(67),"b"(-1),"c"(-1),"d"(newwidth),"S"(newheight));
 }
 else
 {
  __menuet__set_bitfield_for_wanted_events(0x27);
  was_initialized=1;
  MenuetOS_SDL_RepaintWnd();
 }
 return current;
}

/*static SDL_Rect video_mode[4];
static SDL_Rect * SDL_modelist[4]={NULL,NULL,NULL,NULL};*/

static SDL_Rect ** MenuetOS_ListModes(_THIS,SDL_PixelFormat * fmt,Uint32 flags)
{
// return (&SDL_modelist[((fmt->BitsPerPixel+7)/8)-1]);
	if (fmt->BitsPerPixel==24)
		return (SDL_Rect**)-1;
	else
		return NULL;
}

static int MenuetOS_Available(void)
{
 return 1;
}

static void MenuetOS_DeleteDevice(_THIS)
{
// free(this->hidden->__video_buffer);	// it will be freed as current->pixels
 free(this->hidden->__lines);
}

static int MenuetOS_VideoInit(_THIS,SDL_PixelFormat * vformat)
{
#ifdef KEEP_OBSOLETE_STYLE3
	char buf[16];
	__asm__("int $0x40"::"a"(18),"b"(13),"c"(buf));
	if (buf[5]=='K' && buf[6]=='o' && buf[7]=='l' && buf[8]=='i')
		/* kernels up to 0.7.0.0 do not support style 4 */;
	else if (*(unsigned*)(buf+5) >= 549)
		/* window style 4 was introduced in revision 549 */
		IsStyle4Available = 1;
#endif
 vformat->BitsPerPixel=24;
 vformat->BytesPerPixel=3;
 this->info.wm_available=1;
 this->info.hw_available=0;
 this->info.video_mem=0x200000;
/* video_mode[3].x=0;
 video_mode[3].y=0;
 video_mode[3].w=320;
 video_mode[3].h=200;
 video_mode[2].x=0;
 video_mode[2].y=0;
 video_mode[2].w=640;
 video_mode[2].h=400;
 video_mode[1].x=0;
 video_mode[1].y=0;
 video_mode[1].w=320;
 video_mode[1].h=240;
 video_mode[0].x=0;
 video_mode[0].y=0;
 video_mode[0].w=640;
 video_mode[0].h=480;
 SDL_modelist[2]=video_mode+0;*/
 return 0;
}

static int MenuetOS_FlipHWSurface(_THIS,SDL_Surface * surface)
{
 __menuet__putimage(0,0,surface->w,surface->h,
  surface->pixels);
 return 0;
}

WMcursor* KolibriOS_CreateWMCursor(_THIS,
	Uint8* data, Uint8* mask, int w, int h, int hot_x, int hot_y)
{
	int i,j;
	Uint32* cursor;
	WMcursor* res;

	if (w>32 || h>32) return NULL;
	if (w%8 || h%8) return NULL;
	cursor = (Uint32*)malloc(32*32*4);
	if (!cursor) return NULL;
	for (i=0;i<32;i++)
		for (j=0;j<32;j++)
		{
			if (i>=h || j>=w)
			{
				cursor[i*32+j] = 0x00000000;
				continue;
			}
			if (mask[i*w/8+j/8] & (0x80>>(j&7)))
				cursor[i*32+j] = (data[i*w/8+j/8] & (0x80>>(j&7)))?0xFF000000:0xFFFFFFFF;
			else
				cursor[i*32+j] = 0x00000000;
		}
	__asm__ ("int $0x40" : "=a"(res) : "a"(37),"b"(4),
		"c"(cursor),"d"((hot_x<<24)+(hot_y<<16)+2));
	free(cursor);
	return res;
}
int KolibriOS_ShowWMCursor(_THIS,WMcursor*cursor)
{
	if (!cursor)
	{
		if (!has_null_cursor)
		{
			unsigned* u = malloc(32*32*4);
			if (!u) return 1;
			memset(u,0,32*32*4);
			__asm__("int $0x40":"=a"(null_cursor):
				"a"(37),"b"(4),"c"(u),"d"(2));
			free(u);
			has_null_cursor = 1;
		}
		cursor = (WMcursor*)null_cursor;
	}
	__asm__("int $0x40" : : "a"(37),"b"(5),"c"(cursor));
	return 1;
}
void KolibriOS_FreeWMCursor(_THIS,WMcursor*cursor)
{
	__asm__("int $0x40" : : "a"(37),"b"(6),"c"(cursor));
}
void KolibriOS_CheckMouseMode(_THIS)
{
	if (this->input_grab == SDL_GRAB_OFF)
		return;
	struct process_table_entry buf;
	int res;
	__asm__ volatile("int $0x40" : "=a"(res): "a"(9), "b"(&buf), "c"(-1));
	if (res == buf.pos_in_windowing_stack)
	{
		int x = buf.winx_start + buf.client_left + this->hidden->win_size_x/2;
		int y = buf.winy_start + buf.client_top + this->hidden->win_size_y/2;
		__asm__("int $0x40" : : "a"(18),"b"(19),"c"(4),
			"d"(x*65536+y));
	}
}

char def_title[] = "KolibriOS SDL App";
static SDL_VideoDevice * MenuetOS_CreateDevice(int indx)
{
 SDL_VideoDevice * dev;
 dev=(SDL_VideoDevice *)malloc(sizeof(SDL_VideoDevice));
 if(dev) 
 {
  memset(dev,0,(sizeof *dev));
  dev->hidden = (struct SDL_PrivateVideoData *)malloc((sizeof *dev->hidden));
 }
 if((dev==NULL) || (dev->hidden==NULL)) 
 {
  SDL_OutOfMemory();
  if(dev) 
  {
   free(dev);
  }
  return(0);
 }
 memset(dev->hidden,0,(sizeof *dev->hidden));
 dev->hidden->__title = def_title;
 dev->VideoInit=MenuetOS_VideoInit;
 dev->ListModes=MenuetOS_ListModes;
 dev->SetVideoMode=MenuetOS_SetVideoMode;
 dev->SetColors=MenuetOS_SetColors;
 dev->UpdateRects=NULL;
 dev->VideoQuit=MenuetOS_VideoQuit;
 dev->AllocHWSurface=MenuetOS_AllocHWSurface;
 dev->CheckHWBlit=NULL;
 dev->FillHWRect=NULL;
 dev->SetHWColorKey=NULL;
 dev->SetHWAlpha=NULL;
 dev->LockHWSurface=MenuetOS_LockHWSurface;
 dev->UnlockHWSurface=MenuetOS_UnlockHWSurface;
 dev->FlipHWSurface=MenuetOS_FlipHWSurface;
 dev->FreeHWSurface=MenuetOS_FreeHWSurface;
 dev->SetCaption=MenuetOS_SetCaption;
 dev->SetIcon=NULL;
 dev->IconifyWindow=NULL;
 dev->GrabInput=NULL;
 dev->GetWMInfo=NULL;
 dev->InitOSKeymap=MenuetOS_InitOSKeymap;
 dev->PumpEvents=MenuetOS_PumpEvents;
 dev->free=MenuetOS_DeleteDevice;
	dev->CreateWMCursor = KolibriOS_CreateWMCursor;
	dev->FreeWMCursor = KolibriOS_FreeWMCursor;
	dev->ShowWMCursor = KolibriOS_ShowWMCursor;
	dev->CheckMouseMode = KolibriOS_CheckMouseMode;
 return dev;
}

VideoBootStrap mosvideo_bootstrab={
 "menuetos","MenuetOS Device Driver",
 MenuetOS_Available,MenuetOS_CreateDevice,
};
