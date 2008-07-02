
//ld -T ld.x -s --shared --image-base 0 --file-alignment 32 -o test.exe test.obj core.lib

#include "common.h"
#include "ati2d.h"
#include "accel_2d.h"

RHD_t rhd;

static clip_t  clip;

void STDCALL (*SelectHwCursor)(cursor_t*)__asm__("SelectHwCursor");
void STDCALL (*SetHwCursor)(cursor_t*,int x,int y)__asm__("SetHwCursor");
void STDCALL (*HwCursorRestore)(int x, int y)__asm("HwCursorRestore");
cursor_t* IMPORT (*HwCursorCreate)(void)__asm("HwCursorCreate"); //params eax, ebx, ecx

void (__stdcall *old_select)(cursor_t*);
void (__stdcall *old_set)(cursor_t*,int x,int y);
void (__stdcall *old_restore)(int x, int y);
cursor_t* (*old_create)(void);
cursor_t* __create_cursor(void);

static void Init3DEngine(RHDPtr rhdPtr);

u32 __stdcall drvEntry(int action)
{
  RHDPtr rhdPtr;
  u32 retval;

  int i;

  if(action != 1)
    return 0;

  if(!dbg_open("/hd0/2/ati2d.log"))
  {
     printf("Can't open /rd/1/drivers/ati2d.log\nExit\n");
     return 0;
  }

  if((rhdPtr=FindPciDevice())==NULL)
  {
    dbgprintf("Device not found\n");
    return 0;
  };

  for(i=0;i<6;i++)
  {
    if(rhd.memBase[i])
      dbgprintf("Memory base_%d 0x%x size 0x%x\n",
                i,rhd.memBase[i],(1<<rhd.memsize[i]));
  };
  for(i=0;i<6;i++)
  {
    if(rhd.ioBase[i])
      dbgprintf("Io base_%d 0x%x size 0x%x\n",
                i,rhd.ioBase[i],(1<<rhd.memsize[i]));
  };
  if(!RHDPreInit())
    return 0;

//  old_select = SelectHwCursor;
//  old_set = SetHwCursor;
//  old_restore = HwCursorRestore ;
//  old_create = HwCursorCreate;

  R5xx2DInit();
  rhd.has_tcl = 1;
  Init3DEngine(&rhd);

  //init_r500();

 // SelectHwCursor  = r500_SelectCursor;
 // SetHwCursor     = r500_SetCursor;
 // HwCursorCreate  = __create_cursor;
 // HwCursorRestore = r500_CursorRestore;

  retval = RegService("HDRAW", srv_2d);
  dbgprintf("reg service %s as: %x\n", "HDRAW", retval);

//  retval = RegService("HWCURSOR", srv_cursor);
  return retval;
};


#define ERR_PARAM  -1

#pragma pack (push,1)

#pragma pack (pop)

#define API_VERSION     0x01000100

#define SRV_GETVERSION  0

int _stdcall srv_cursor(ioctl_t *io)
{
  u32 *inp;
  u32 *outp;

  inp = io->input;
  outp = io->output;

  switch(io->io_code)
  {
    case SRV_GETVERSION:
      if(io->out_size==4)
      {
        *(u32*)io->output = API_VERSION;
        return 0;
      }
      break;

    default:
      return ERR_PARAM;
  };
  return ERR_PARAM;
}


int _stdcall srv_2d(ioctl_t *io)
{
  u32 *inp;
  u32 *outp;

  inp = io->input;
  outp = io->output;

  switch(io->io_code)
  {
    case SRV_GETVERSION:
      if(io->out_size==4)
      {
        *outp = API_VERSION;
        return 0;
      }
      break;

      case DRAW_RECT:
        if(io->inp_size==5)
          return DrawRect((draw_t*)inp);
        break;

      case FILL_RECT:
        if(io->inp_size==8)
          return FillRect((fill_t*)inp);
        break;

      case LINE_2P:
        if(io->inp_size==5)
          return Line2P((line2p_t*)inp);
        break;

      case BLIT:
        if(io->inp_size==6)
          return Blit((blit_t*)inp);
        break;

      case COMPIZ:
        if(io->inp_size==6)
          return RadeonComposite((blit_t*)inp);
        break;


    default:
      return ERR_PARAM;
  };
  return ERR_PARAM;
}


#include "init.c"
#include "pci.c"
#include "ati_mem.c"
#include "cursor.inc"

#include "r500.inc"
#include "accel_2d.inc"
#include "accel_3d.inc"

#define CLIP_TOP        1
#define CLIP_BOTTOM     2
#define CLIP_RIGHT      4
#define CLIP_LEFT       8


char _L1OutCode( int x, int y )
/*=================================

    Verify that a point is inside or outside the active viewport.   */
{
    char            flag;

    flag = 0;
    if( x < clip.xmin ) {
        flag |= CLIP_LEFT;
    } else if( x > clip.xmax ) {
        flag |= CLIP_RIGHT;
    }
    if( y < clip.ymin ) {
        flag |= CLIP_TOP;
    } else if( y > clip.ymax ) {
        flag |= CLIP_BOTTOM;
    }
    return( flag );
}


static void line_inter( int * x1, int* y1, int x2, int y2, int x )
/*===========================================================================

    Find the intersection of a line with a boundary of the viewport.
    (x1, y1) is outside and ( x2, y2 ) is inside the viewport.
    NOTE : the signs of denom and ( x - *x1 ) cancel out during division
           so make both of them positive before rounding.   */
{
    int            numer;
    int            denom;

    denom = abs( x2 - *x1 );
    numer = 2L * (long)( y2 - *y1 ) * abs( x - *x1 );
    if( numer > 0 ) {
        numer += denom;                     /* round to closest pixel   */
    } else {
        numer -= denom;
    }
    *y1 += numer / ( denom << 1 );
    *x1 = x;
}


int LineClip( int *x1, int *y1, int *x2, int *y2 )
/*=============================================================

    Clips the line with end points (x1,y1) and (x2,y2) to the active
    viewport using the Cohen-Sutherland clipping algorithm. Return the
    clipped coordinates and a decision drawing flag.    */
{
    char            flag1;
    char            flag2;

    flag1 = _L1OutCode( *x1, *y1 );
    flag2 = _L1OutCode( *x2, *y2 );
    for( ;; ) {
        if( flag1 & flag2 ) break;                  /* trivially outside    */
        if( flag1 == flag2 ) break;                 /* completely inside    */
        if( flag1 == 0 ) {                          /* first point inside   */
            if( flag2 & CLIP_TOP ) {
                line_inter( y2, x2, *y1, *x1, clip.ymin );
            } else if( flag2 & CLIP_BOTTOM ) {
                line_inter( y2, x2, *y1, *x1, clip.ymax );
            } else if( flag2 & CLIP_RIGHT ) {
                line_inter( x2, y2, *x1, *y1, clip.xmax );
            } else if( flag2 & CLIP_LEFT ) {
                line_inter( x2, y2, *x1, *y1, clip.xmin );
            }
            flag2 = _L1OutCode( *x2, *y2 );
        } else {                                    /* second point inside  */
            if( flag1 & CLIP_TOP ) {
                line_inter( y1, x1, *y2, *x2, clip.ymin );
            } else if( flag1 & CLIP_BOTTOM ) {
                line_inter( y1, x1, *y2, *x2, clip.ymax );
            } else if( flag1 & CLIP_RIGHT ) {
                line_inter( x1, y1, *x2, *y2, clip.xmax );
            } else if( flag1 & CLIP_LEFT ) {
                line_inter( x1, y1, *x2, *y2, clip.xmin );
            }
            flag1 = _L1OutCode( *x1, *y1 );
        }
    }
    return( flag1 & flag2 );
}


static void block_inter( int *x, int *y, int flag )
/*======================================================

    Find the intersection of a block with a boundary of the viewport.   */
{
    if( flag & CLIP_TOP ) {
        *y = clip.ymin;
    } else if( flag & CLIP_BOTTOM ) {
        *y = clip.ymax;
    } else if( flag & CLIP_RIGHT ) {
        *x = clip.xmax;
    } else if( flag & CLIP_LEFT ) {
        *x = clip.xmin;
    }
}


int BlockClip( int *x1, int *y1, int *x2, int* y2 )
/*==============================================================

    Clip a block with opposite corners (x1,y1) and (x2,y2) to the
    active viewport based on the Cohen-Sutherland algorithm for line
    clipping. Return the clipped coordinates and a decision drawing
    flag ( 0 draw : 1 don't draw ). */
{
    char            flag1;
    char            flag2;

    flag1 = _L1OutCode( *x1, *y1 );
    flag2 = _L1OutCode( *x2, *y2 );
    for( ;; ) {
        if( flag1 & flag2 ) break;                  /* trivially outside    */
        if( flag1 == flag2 ) break;                 /* completely inside    */
        if( flag1 == 0 ) {
            block_inter( x2, y2, flag2 );
            flag2 = _L1OutCode( *x2, *y2 );
        } else {
            block_inter( x1, y1, flag1 );
            flag1 = _L1OutCode( *x1, *y1 );
        }
    }
    return( flag1 & flag2 );
}

#if 0
typedef struct
{
   int left;
   int top;
   int right;
   int bottom;
}rect_t;

typedef struct _window
{
   struct _window *fd;
   struct _window *bk;

   rect_t pos;
   int width;
   int height;

   int level;
}win_t, *WINPTR;


WINPTR top    = NULL;
WINPTR bottom = NULL;

WINPTR alloc_window()
{
    WINPTR pwin = malloc(sizeof(win_t));

    return pwin;
};


WINPTR create_win(int l, int t, int w, int h, int level)
{
   WINPTR pwin = alloc_window();

   pwin->pos.left   = l;
   pwin->pos.top    = t;
   pwin->pos.right  = l+w-1;
   pwin->pos.bottom = t+h-1;
   pwin->width      = w;
   pwin->height     = h;

   pwin->level      = level;

   return pwin;
};

void insert_window(WINPTR pwin)
{
   WINPTR p = top;

   if(p)
   {
     if(pwin->level <= p->level)
     {
        pwin->fd = p;
        pwin->bk = p->bk;
        pwin->bk->fd = pwin;
        p->bk = pwin;
     }
     else
       p = p->fd;
   }
   else
     top = pwin;
}

#endif
