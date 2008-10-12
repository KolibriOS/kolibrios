
//ld -T ld.x -s --shared --image-base 0 --file-alignment 32 -o test.exe test.obj core.lib

#include "common.h"

#include "ati2d.h"
#include "accel_2d.h"

RHD_t rhd;

static clip_t  clip;

static local_pixmap_t scr_pixmap;


/*
void STDCALL (*SelectHwCursor)(cursor_t*)__asm__("SelectHwCursor");
void STDCALL (*SetHwCursor)(cursor_t*,int x,int y)__asm__("SetHwCursor");
void STDCALL (*HwCursorRestore)(int x, int y)__asm("HwCursorRestore");
cursor_t* IMPORT (*HwCursorCreate)(void)__asm("HwCursorCreate"); //params eax, ebx, ecx

void (__stdcall *old_select)(cursor_t*);
void (__stdcall *old_set)(cursor_t*,int x,int y);
void (__stdcall *old_restore)(int x, int y);
cursor_t* (*old_create)(void);
cursor_t* __create_cursor(void);
*/

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
//  Init3DEngine(&rhd);

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


#define API_VERSION     0x01000100

#define SRV_GETVERSION  0

/*
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
*/

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
        if(io->inp_size==6)
          return DrawRect((draw_t*)inp);
        break;

      case FILL_RECT:
        if(io->inp_size==9)
          return FillRect((fill_t*)inp);
        break;
/*
      case LINE_2P:
        if(io->inp_size==6)
          return Line2P((draw_t*)inp);
        break;

      case BLIT:
        if(io->inp_size==6)
          return Blit((blit_t*)inp);
        break;

      case COMPIZ:
        if(io->inp_size==6)
          return RadeonComposite((blit_t*)inp);
        break;
*/
      case PX_CREATE:
        if(io->inp_size==7)
          return CreatePixmap((pixmap_t*)inp);
        break;

      case PIXBLIT:
        if(io->inp_size==8)
          return PixBlit((pixblit_t*)inp);
        break;

//      case PIXLOCK:
//        if(io->inp_size==6)
//          return LockPixmap((userpixmap_t*)inp);
//        break;

//      case PIXUNLOCK:
//        if(io->inp_size==6)
//          return UnlockPixmap((userpixmap_t*)inp);
//        break;

      case PIXDESTROY:
        if(io->inp_size==6)
          return DestroyPixmap((pixmap_t*)inp);
        break;

     case  TRANSBLIT:
        if(io->inp_size==8)
          return TransBlit((pixblit_t*)inp);
        break;


    default:
      return ERR_PARAM;
  };
  return ERR_PARAM;
}


#include "init.c"
#include "pci.c"
#include "ati_mem.c"
//#include "cursor.inc"

#include "r500.inc"

#include "clip.inc"
#include "accel_2d.inc"
#include "accel_3d.inc"


