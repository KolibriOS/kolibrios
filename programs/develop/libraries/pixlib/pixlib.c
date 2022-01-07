
//#define KOLIBRI_PE

#include "types.h"
#include "system.h"

#include "pixlib.h"

static clip_t   scrn_clip;
static pixmap_t scrn_pixmap;

static u32_t srv_hw2d;

#define HS_HORIZONTAL     0
#define HS_VERTICAL       1
#define HS_FDIAGONAL      2
#define HS_BDIAGONAL      3
#define HS_CROSS          4
#define HS_DIAGCROSS      5


static u32_t hatches[HATCH_MAX*2] =
            { 0xFF000000, 0xFF000000,     /*  HORIZONTAL */
              0x22222222, 0x22222222,     /*  VERTICAL   */
              0x11224488, 0x11224488,     /*  FDIAGONAL  */
              0x44221188, 0x44221188,     /*  BDIAGONAL  */
              0xFF111111, 0xFF111111,     /*  CROSS      */
              0x10284482, 0x01824428      /*  DCROSS     */
            };


typedef struct {
  int available;            /**< Count of available items in this slab. */
  void *start;              /**< Start address of first item. */
  void *nextavail;          /**< The index of next available item. */
} slab_t;

static brush_t  brushes[256];
static pixmap_t pixmaps[64];
static slab_t   br_slab;
static slab_t   px_slab;

int __stdcall start(int state)
{
     int p;
     int i;

     int scrnsize;
     int scrnbpp;
     int scrnpitch;

     if( !test_mmx())
        return FALSE;

     if( (scrnbpp = GetScreenBpp()) != 32)
        return FALSE;

     scrnsize  = GetScreenSize();
     scrnpitch = GetScreenPitch();

     scrn_clip.xmin = 0;
     scrn_clip.ymin = 0;
     scrn_clip.xmax = (scrnsize >> 16) - 1;
     scrn_clip.ymax = (scrnsize & 0xFFFF) - 1;

     scrn_pixmap.width   = scrnsize >> 16;
     scrn_pixmap.height  = scrnsize & 0xFFFF;
     scrn_pixmap.format  = PICT_a8r8g8b8;
     scrn_pixmap.flags   = PX_MEM_LOCAL;
     scrn_pixmap.pitch   = scrnpitch;
     scrn_pixmap.mapped  = (void*)LFB_BASE;

     br_slab.available = 256;
     br_slab.start = brushes;
     br_slab.nextavail = brushes;

     for (i = 0, p = (int)br_slab.start; i < 256; i++)
     {
       *(int *)p = p+sizeof(brush_t);
       p = p+sizeof(brush_t);
     };

     px_slab.available = 64;
     px_slab.start = pixmaps;
     px_slab.nextavail = pixmaps;

     for (i = 0, p = (int)px_slab.start; i < 64; i++)
     {
       *(int *)p = p+sizeof(pixmap_t);
       p = p+sizeof(pixmap_t);
     };

     srv_hw2d = get_service("HDRAW");
       if(srv_hw2d == 0)
         srv_hw2d = load_service("/sys/drivers/ati2d.drv");

     return TRUE;
};


#include "clip.inc"
#include "pixmap.inc"
#include "brush.inc"
#include "draw.inc"

typedef struct
{
  char *name;
  void *f;
}export_t;

char szStart[]           = "START";
char szVersion[]         = "version";

//char szBlockClip[]       = "BlockClip";
//char szLineClip[]        = "LineClip";

char szCreatePixmap[]    = "CreatePixmap";
char szDestroyPixmap[]   = "DestroyPixmap";
char szLockPixmap[]      = "LockPixmap";
char szUnlockPixmap[]    = "UnlockPixmap";
char szGetPixmapPitch[]  = "GetPixmapPitch";

char szCreateHatch[]     = "CreateHatch";
char szCreateMonoBrush[] = "CreateMonoBrush";
char szDestroyBrush[]    = "DestroyBrush";

char szClearPixmap[]     = "ClearPixmap";
char szLine[]            = "Line";
char szDrawRect[]        = "DrawRect";
char szFillRect[]        = "FillRect";
char szBlit[]            = "Blit";
char szTransparentBlit[] = "TransparentBlit";
char szBlitAlpha[]       = "BlitAlpha";


export_t EXPORTS[] __asm__("EXPORTS") =
         {
           { szStart,           start },
           { szVersion,         (void*)0x00010001 },

        //   { szBlockClip,       BlockClip },
        //   { szLineClip,        LineClip },

           { szCreatePixmap,    CreatePixmap    },
           { szDestroyPixmap,   DestroyPixmap   },
           { szLockPixmap,      LockPixmap      },
           { szUnlockPixmap,    UnlockPixmap    },
           { szGetPixmapPitch,  GetPixmapPitch  },

           { szCreateHatch,     CreateHatch     },
           { szCreateMonoBrush, CreateMonoBrush },
           { szDestroyBrush,    DestroyBrush    },

           { szClearPixmap,     ClearPixmap     },
           { szLine,            Line            },
           { szDrawRect,        DrawRect        },
           { szFillRect,        FillRect        },
           { szBlit,            Blit            },
           { szTransparentBlit, TransparentBlit },
           { szBlitAlpha,       BlitAlpha       },

           { NULL, NULL },
         };


