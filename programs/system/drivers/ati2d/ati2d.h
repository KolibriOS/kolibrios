
#include "pci.h"
#include "rhd_regs.h"

#define IS_R300_3D   0
#define IS_R500_3D   1

#define R300_PIO     0

enum RHD_CHIPSETS {
    RHD_UNKNOWN = 0,
    RHD_R300,
    RHD_R350,
    RHD_RV350,
    RHD_RV370,
    RHD_RV380,
    /* R500 */
    RHD_RV505,
    RHD_RV515,
    RHD_RV516,
    RHD_R520,
    RHD_RV530,
    RHD_RV535,
    RHD_RV550,
    RHD_RV560,
    RHD_RV570,
    RHD_R580,
    /* R500 Mobility */
    RHD_M52,
    RHD_M54,
    RHD_M56,
    RHD_M58,
    RHD_M62,
    RHD_M64,
    RHD_M66,
    RHD_M68,
    RHD_M71,
    /* R500 integrated */
    RHD_RS600,
    RHD_RS690,
    RHD_RS740,
    /* R600 */
    RHD_R600,
    RHD_RV610,
    RHD_RV630,
    /* R600 Mobility */
    RHD_M72,
    RHD_M74,
    RHD_M76,
    /* RV670 came into existence after RV6x0 and M7x */
    RHD_RV670,
    RHD_R680,
    RHD_RV620,
    RHD_M82,
    RHD_RV635,
    RHD_M86,
    RHD_RS780,
    RHD_CHIP_END
};

enum RHD_FAMILIES {
    RHD_FAMILY_UNKNOWN = 0,

    RHD_FAMILY_RADEON,

    RHD_FAMILY_RV100,
    RHD_FAMILY_RS100,    /* U1 (IGP320M) or A3 (IGP320)*/
    RHD_FAMILY_RV200,
    RHD_FAMILY_RS200,    /* U2 (IGP330M/340M/350M) or A4 (IGP330/340/345/350), RS250 (IGP 7000) */
    RHD_FAMILY_R200,
    RHD_FAMILY_RV250,
    RHD_FAMILY_RS300,    /* RS300/RS350 */
    RHD_FAMILY_RV280,

    RHD_FAMILY_R300,
    RHD_FAMILY_R350,
    RHD_FAMILY_RV350,
    RHD_FAMILY_RV380,    /* RV370/RV380/M22/M24 */
    RHD_FAMILY_R420,     /* R420/R423/M18 */
    RHD_FAMILY_RV410,    /* RV410, M26 */
    RHD_FAMILY_RS400,    /* xpress 200, 200m (RS400) Intel */
    RHD_FAMILY_RS480,    /* xpress 200, 200m (RS410/480/482/485) AMD */

    RHD_FAMILY_RV515,
    RHD_FAMILY_R520,
    RHD_FAMILY_RV530,
    RHD_FAMILY_RV560,
    RHD_FAMILY_RV570,
    RHD_FAMILY_R580,
    RHD_FAMILY_RS690,
    RHD_FAMILY_R600,
    RHD_FAMILY_RV610,
    RHD_FAMILY_RV630,
    RHD_FAMILY_RV670,
    RHD_FAMILY_RV620,
    RHD_FAMILY_RV635,
    RHD_FAMILY_RS780
};

#define RHD_FB_BAR         0
#define RHD_MMIO_BAR       2

#define RHD_MEM_GART       1
#define RHD_MEM_FB         2

typedef struct RHDRec
{
  CARD32            MMIOBase;
  CARD32            MMIOMapSize;
  CARD32            videoRam;

  CARD32            FbBase;            /* map base of fb   */
  CARD32            PhisBase;
  CARD32            FbIntAddress;      /* card internal address of FB */
  CARD32            FbMapSize;

  CARD32            FbFreeStart;
  CARD32            FbFreeSize;

  /* visible part of the framebuffer */
  unsigned int      FbScanoutStart;
  unsigned int      FbScanoutSize;

  enum RHD_CHIPSETS ChipSet;
  enum RHD_FAMILIES ChipFamily;

  char              *ChipName;

  Bool              IsIGP;

  CARD32            bus;
  CARD32            devfn;

  PCITAG            PciTag;
  CARD16            PciDeviceID;

  CARD16            subvendor_id;
  CARD16            subdevice_id;

  CARD32            memBase[6];
  CARD32            ioBase[6];
  CARD32            memtype[6];
  CARD32            memsize[6];

  struct mem_block  *fb_heap;
  struct mem_block  *gart_heap;

  CARD32            displayWidth;
  CARD32            displayHeight;

  CARD32            __xmin;
  CARD32            __ymin;
  CARD32            __xmax;
  CARD32            __ymax;

  CARD32            gui_control;
  CARD32            dst_pitch_offset;
  CARD32            surface_cntl;

  u32               *ring_base;
  u32               ring_rp;
  u32               ring_wp;

  int               num_gb_pipes;
  Bool              has_tcl;
}RHD_t, *RHDPtr;

extern RHD_t rhd;

typedef struct
{
  int xmin;
  int ymin;
  int xmax;
  int ymax;
}clip_t, *PTRclip;

typedef struct {
    int			token;		/* id of the token */
    const char *	name;		/* token name */
} SymTabRec, *SymTabPtr;



extern inline void
OUTREG8(CARD16 offset, u8 value)
{
  *(volatile CARD8 *)((CARD8 *)(rhd.MMIOBase + offset)) = value;
}


extern inline CARD32 INREG(CARD16 offset)
{
  return *(volatile CARD32 *)((CARD8*)(rhd.MMIOBase + offset));
}

//#define INREG(offset) *(volatile CARD32 *)((CARD8*)(rhd.MMIOBase + (offset)))

extern inline void
OUTREG(CARD16 offset, CARD32 value)
{
  *(volatile CARD32 *)((CARD8 *)(rhd.MMIOBase + offset)) = value;
}

extern inline CARD32 _RHDRegRead(RHDPtr rhdPtr, CARD16 offset)
{
  return *(volatile CARD32 *)((CARD8*)(rhdPtr->MMIOBase + offset));
}

extern inline void
MASKREG(CARD16 offset, CARD32 value, CARD32 mask)
{
  CARD32 tmp;

  tmp = INREG(offset);
  tmp &= ~mask;
  tmp |= (value & mask);
  OUTREG(offset, tmp);
};

extern inline void
_RHDRegWrite(RHDPtr rhdPtr, CARD16 offset, CARD32 value)
{
  *(volatile CARD32 *)((CARD8 *)(rhdPtr->MMIOBase + offset)) = value;
}

extern inline void
_RHDRegMask(RHDPtr rhdPtr, CARD16 offset, CARD32 value, CARD32 mask)
{
  CARD32 tmp;

  tmp = _RHDRegRead(rhdPtr, offset);
  tmp &= ~mask;
  tmp |= (value & mask);
  _RHDRegWrite(rhdPtr, offset, tmp);
};

enum RHD_FAMILIES RHDFamily(enum RHD_CHIPSETS chipset);

#define RHDRegRead(ptr, offset) _RHDRegRead((ptr)->rhdPtr, (offset))
#define RHDRegWrite(ptr, offset, value) _RHDRegWrite((ptr)->rhdPtr, (offset), (value))
#define RHDRegMask(ptr, offset, value, mask) _RHDRegMask((ptr)->rhdPtr, (offset), (value), (mask))


RHDPtr FindPciDevice();

Bool RHDPreInit();
int rhdInitHeap(RHDPtr rhdPtr);

#define RHDFUNC(ptr)

#define DBG(x) x
//  #define DBG(x)

#pragma pack (push,1)
typedef struct s_cursor
{
   u32   magic;                           // 'CURS'
   void  (*destroy)(struct s_cursor*);    // destructor
   u32   fd;                              // next object in list
   u32   bk;                              // prev object in list
   u32   pid;                             // owner id

   void *base;                            // allocated memory
   u32   hot_x;                           // hotspot coords
   u32   hot_y;
}cursor_t;
#pragma pack (pop)

#define LOAD_FROM_FILE   0
#define LOAD_FROM_MEM    1
#define LOAD_INDIRECT    2

cursor_t *create_cursor(u32 pid, void *src, u32 flags);
void __stdcall copy_cursor(void *img, void *src);
void destroy_cursor(cursor_t *cursor);
void __destroy_cursor(cursor_t *cursor);                // wrap

void __stdcall r500_SelectCursor(cursor_t *cursor);
void __stdcall r500_SetCursor(cursor_t *cursor, int x, int y);
void __stdcall r500_CursorRestore(int x, int y);

void  R5xx2DInit();


typedef struct {
    u32_t x ;
    u32_t y ;
} xPointFixed;

typedef u32_t   xFixed_16_16;

typedef xFixed_16_16  xFixed;

#define XFIXED_BITS 16

#define xFixedToInt(f)  (int) ((f) >> XFIXED_BITS)
#define IntToxFixed(i)  ((xFixed) (i) << XFIXED_BITS)

#define xFixedToFloat(f) (((float) (f)) / 65536)

#define PICT_FORMAT(bpp,type,a,r,g,b) (((bpp) << 24) |  \
					 ((type) << 16) | \
					 ((a) << 12) | \
					 ((r) << 8) | \
					 ((g) << 4) | \
					 ((b)))

#define PICT_FORMAT_A(f)  (((f) >> 12) & 0x0f)
#define PICT_FORMAT_RGB(f)  (((f)      ) & 0xfff)

#define PICT_TYPE_OTHER 0
#define PICT_TYPE_A     1
#define PICT_TYPE_ARGB	2
#define PICT_TYPE_ABGR	3
#define PICT_TYPE_COLOR	4
#define PICT_TYPE_GRAY	5

typedef enum _PictFormatShort {
   PICT_a8r8g8b8 =	PICT_FORMAT(32,PICT_TYPE_ARGB,8,8,8,8),
   PICT_x8r8g8b8 =	PICT_FORMAT(32,PICT_TYPE_ARGB,0,8,8,8),
   PICT_a8b8g8r8 =	PICT_FORMAT(32,PICT_TYPE_ABGR,8,8,8,8),
   PICT_x8b8g8r8 =	PICT_FORMAT(32,PICT_TYPE_ABGR,0,8,8,8),

/* 24bpp formats */
   PICT_r8g8b8 =	PICT_FORMAT(24,PICT_TYPE_ARGB,0,8,8,8),
   PICT_b8g8r8 =	PICT_FORMAT(24,PICT_TYPE_ABGR,0,8,8,8),

/* 16bpp formats */
   PICT_r5g6b5 =	PICT_FORMAT(16,PICT_TYPE_ARGB,0,5,6,5),
   PICT_b5g6r5 =	PICT_FORMAT(16,PICT_TYPE_ABGR,0,5,6,5),

   PICT_a1r5g5b5 =	PICT_FORMAT(16,PICT_TYPE_ARGB,1,5,5,5),
   PICT_x1r5g5b5 =	PICT_FORMAT(16,PICT_TYPE_ARGB,0,5,5,5),
   PICT_a1b5g5r5 =	PICT_FORMAT(16,PICT_TYPE_ABGR,1,5,5,5),
   PICT_x1b5g5r5 =	PICT_FORMAT(16,PICT_TYPE_ABGR,0,5,5,5),
   PICT_a4r4g4b4 =	PICT_FORMAT(16,PICT_TYPE_ARGB,4,4,4,4),
   PICT_x4r4g4b4 =	PICT_FORMAT(16,PICT_TYPE_ARGB,0,4,4,4),
   PICT_a4b4g4r4 =	PICT_FORMAT(16,PICT_TYPE_ABGR,4,4,4,4),
   PICT_x4b4g4r4 =	PICT_FORMAT(16,PICT_TYPE_ABGR,0,4,4,4),

/* 8bpp formats */
   PICT_a8 =		PICT_FORMAT(8,PICT_TYPE_A,8,0,0,0),
   PICT_r3g3b2 =	PICT_FORMAT(8,PICT_TYPE_ARGB,0,3,3,2),
   PICT_b2g3r3 =	PICT_FORMAT(8,PICT_TYPE_ABGR,0,3,3,2),
   PICT_a2r2g2b2 =	PICT_FORMAT(8,PICT_TYPE_ARGB,2,2,2,2),
   PICT_a2b2g2r2 =	PICT_FORMAT(8,PICT_TYPE_ABGR,2,2,2,2),

   PICT_c8 =		PICT_FORMAT(8,PICT_TYPE_COLOR,0,0,0,0),
   PICT_g8 =		PICT_FORMAT(8,PICT_TYPE_GRAY,0,0,0,0),

   PICT_x4a4 =		PICT_FORMAT(8,PICT_TYPE_A,4,0,0,0),

   PICT_x4c4 =		PICT_FORMAT(8,PICT_TYPE_COLOR,0,0,0,0),
   PICT_x4g4 =		PICT_FORMAT(8,PICT_TYPE_GRAY,0,0,0,0),

/* 4bpp formats */
   PICT_a4 =		PICT_FORMAT(4,PICT_TYPE_A,4,0,0,0),
   PICT_r1g2b1 =	PICT_FORMAT(4,PICT_TYPE_ARGB,0,1,2,1),
   PICT_b1g2r1 =	PICT_FORMAT(4,PICT_TYPE_ABGR,0,1,2,1),
   PICT_a1r1g1b1 =	PICT_FORMAT(4,PICT_TYPE_ARGB,1,1,1,1),
   PICT_a1b1g1r1 =	PICT_FORMAT(4,PICT_TYPE_ABGR,1,1,1,1),

   PICT_c4 =		PICT_FORMAT(4,PICT_TYPE_COLOR,0,0,0,0),
   PICT_g4 =		PICT_FORMAT(4,PICT_TYPE_GRAY,0,0,0,0),

/* 1bpp formats */
   PICT_a1 =		PICT_FORMAT(1,PICT_TYPE_A,1,0,0,0),

   PICT_g1 =		PICT_FORMAT(1,PICT_TYPE_GRAY,0,0,0,0),
} PictFormatShort;

