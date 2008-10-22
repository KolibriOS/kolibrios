

#define R300_PIO     0       /* now we have cp */


#include "types.h"

#include <stdio.h>
#include <malloc.h>
#include <memory.h>

#include "pci.h"

#include "syscall.h"

#include "radeon_reg.h"

#include "ati2d.h"
#include "accel_2d.h"

RHD_t rhd __attribute__ ((aligned (128)));    /* reduce cache lock */

static clip_t  clip;

static local_pixmap_t scr_pixmap;

static void Init3DEngine(RHDPtr rhdPtr);

int __stdcall srv_2d(ioctl_t *io);

u32_t __stdcall drvEntry(int action)
{
  RHDPtr rhdPtr;
  u32_t retval;

  int i;

  if(action != 1)
    return 0;

  if(!dbg_open("/rd/1/drivers/ati2d.log"))
  {
     printf("Can't open /rd/1/drivers/ati2d.log\nExit\n");
     return 0;
  }
  if( GetScreenBpp() != 32)
  {
     dbgprintf("32 bpp dispaly mode required !\nExit\t");
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

  R5xx2DInit();
  rhd.has_tcl = 1;

//  Init3DEngine(&rhd);

  //init_r500();


  retval = RegService("HDRAW", srv_2d);
  dbgprintf("reg service %s as: %x\n", "HDRAW", retval);

  return retval;
};


#define API_VERSION     0x01000100

#define SRV_GETVERSION  0


int __stdcall srv_2d(ioctl_t *io)
{
  u32_t *inp;
  u32_t *outp;

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

      case PX_CREATE:
        if(io->inp_size==7)
          return CreatePixmap((pixmap_t*)inp);
        break;

      case PX_DESTROY:
        if(io->inp_size==7)
          return DestroyPixmap((pixmap_t*)inp);
        break;

      case PX_CLEAR:
        if(io->inp_size==2)
          return ClearPixmap((io_clear_t*)inp);
        break;

      case PX_DRAW_RECT:
        if(io->inp_size==7)
          return DrawRect((io_draw_t*)inp);
        break;

      case PX_FILL_RECT:
        if(io->inp_size==10)
          return FillRect((io_fill_t*)inp);
        break;

      case PX_BLIT:
        if(io->inp_size==8)
          return Blit((io_blit_t*)inp);
        break;

     case  PX_BLIT_TRANSPARENT:
        if(io->inp_size==9)
          return BlitTransparent((io_blit_t*)inp);
        break;

      case PX_LINE:
        if(io->inp_size==6)
          return Line((io_draw_t*)inp);
        break;

/*

      case COMPIZ:
        if(io->inp_size==6)
          return RadeonComposite((blit_t*)inp);
        break;
*/

    default:
      return ERR_PARAM;
  };
  return ERR_PARAM;
}


#include "init.c"
#include "pci.c"
#include "ati_mem.c"

#include "init_cp.c"
#include "r500.inc"

#include "clip.inc"
#include "pixmap.inc"
#include "accel_2d.inc"
//#include "accel_3d.inc"


