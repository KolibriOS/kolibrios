
//#define ATOM_BIOS        1
//#define ATOM_BIOS_PARSER 1

#define OS_BASE   0x80000000

#include "xmd.h"

#define NULL (void*)(0)

#define FALSE   0
#define TRUE    1

typedef void *pointer;

typedef unsigned int   Bool;

typedef unsigned char  u8_t;
typedef unsigned short u16_t;
typedef unsigned int   u32_t;

typedef unsigned int memType;
typedef unsigned int size_t;

#define MAX_HSYNC 8
#define MAX_VREFRESH 8
#define INTERLACE_REFRESH_WEIGHT  1.5
#define SYNC_TOLERANCE		0.01	/* 1 percent */
#define CLOCK_TOLERANCE		2000	/* Clock matching tolerance (2MHz) */

typedef struct { float hi, lo; } range;


#define STDCALL __attribute__ ((stdcall)) __attribute__ ((dllimport))
#define IMPORT __attribute__ ((dllimport))


CARD32 STDCALL AllocKernelSpace(unsigned size)__asm__("AllocKernelSpace");
void*  STDCALL KernelAlloc(unsigned size)__asm__("KernelAlloc");
int KernelFree(void *);

CARD32 STDCALL MapIoMem(CARD32 Base,CARD32 size,CARD32 flags)__asm__("MapIoMem");

u8_t  STDCALL PciRead8 (u32_t bus, u32_t devfn, u32_t reg)__asm__("PciRead8");
u16_t STDCALL PciRead16(u32_t bus, u32_t devfn, u32_t reg)__asm__("PciRead16");
u32_t STDCALL PciRead32(u32_t bus, u32_t devfn, u32_t reg)__asm__("PciRead32");

#define pciReadLong(tag, reg) \
        PciRead32(PCI_BUS_FROM_TAG(tag),PCI_DFN_FROM_TAG(tag),(reg))

u32_t STDCALL PciWrite8 (u32_t bus, u32_t devfn, u32_t reg,u8_t val) __asm__("PciWrite8");
u32_t STDCALL PciWrite16(u32_t bus, u32_t devfn, u32_t reg,u16_t val)__asm__("PciWrite16");
u32_t STDCALL PciWrite32(u32_t bus, u32_t devfn, u32_t reg,u32_t val)__asm__("PciWrite32");

#define pciWriteLong(tag, reg, val) \
        PciWrite32(PCI_BUS_FROM_TAG(tag),PCI_DFN_FROM_TAG(tag),(reg),(val))

void usleep(u32_t delay);

///////////////////////////////////////////////////////////////////////////////

void *malloc(size_t);
void *calloc( size_t num, size_t size );
void *realloc(void*, size_t);
void free(void*);


#define xalloc    malloc
#define xnfalloc  malloc

#define xcalloc   calloc
#define xnfcalloc calloc

#define xrealloc  realloc

#define xfree     free

///////////////////////////////////////////////////////////////////////////////

void* memset(void *s, int c, size_t n);
void* memcpy(void * dest, const void *src, size_t n);
int   memcmp(const void *s1, const void *s2, size_t n);

size_t strlen(const char *str);
char*  strcpy(char *to, const char *from);
char*  strcat(char *s, const char *append);
char*  strdup(const char *s);
char*  strchr(const char *s, int c);
int    strcmp(const char *s1, const char *s2);

#define xstrdup  strdup

///////////////////////////////////////////////////////////////////////////////

int snprintf(char *s, size_t n, const char *format, ...);
int printf(const char* format, ...);
int dbg_open(char *path);
int dbgprintf(const char* format, ...);

///////////////////////////////////////////////////////////////////////////////

/* These are possible return values for xf86CheckMode() and ValidMode() */
typedef enum {
    MODE_OK	= 0,	/* Mode OK */
    MODE_HSYNC,		/* hsync out of range */
    MODE_VSYNC,		/* vsync out of range */
    MODE_H_ILLEGAL,	/* mode has illegal horizontal timings */
    MODE_V_ILLEGAL,	/* mode has illegal horizontal timings */
    MODE_BAD_WIDTH,	/* requires an unsupported linepitch */
    MODE_NOMODE,	/* no mode with a maching name */
    MODE_NO_INTERLACE,	/* interlaced mode not supported */
    MODE_NO_DBLESCAN,	/* doublescan mode not supported */
    MODE_NO_VSCAN,	/* multiscan mode not supported */
    MODE_MEM,		/* insufficient video memory */
    MODE_VIRTUAL_X,	/* mode width too large for specified virtual size */
    MODE_VIRTUAL_Y,	/* mode height too large for specified virtual size */
    MODE_MEM_VIRT,	/* insufficient video memory given virtual size */
    MODE_NOCLOCK,	/* no fixed clock available */
    MODE_CLOCK_HIGH,	/* clock required is too high */
    MODE_CLOCK_LOW,	/* clock required is too low */
    MODE_CLOCK_RANGE,	/* clock/mode isn't in a ClockRange */
    MODE_BAD_HVALUE,	/* horizontal timing was out of range */
    MODE_BAD_VVALUE,	/* vertical timing was out of range */
    MODE_BAD_VSCAN,	/* VScan value out of range */
    MODE_HSYNC_NARROW,	/* horizontal sync too narrow */
    MODE_HSYNC_WIDE,	/* horizontal sync too wide */
    MODE_HBLANK_NARROW,	/* horizontal blanking too narrow */
    MODE_HBLANK_WIDE,	/* horizontal blanking too wide */
    MODE_VSYNC_NARROW,	/* vertical sync too narrow */
    MODE_VSYNC_WIDE,	/* vertical sync too wide */
    MODE_VBLANK_NARROW,	/* vertical blanking too narrow */
    MODE_VBLANK_WIDE,	/* vertical blanking too wide */
    MODE_PANEL,         /* exceeds panel dimensions */
    MODE_INTERLACE_WIDTH, /* width too large for interlaced mode */
    MODE_ONE_WIDTH,     /* only one width is supported */
    MODE_ONE_HEIGHT,    /* only one height is supported */
    MODE_ONE_SIZE,      /* only one resolution is supported */
    MODE_BAD = -2,	/* unspecified reason */
    MODE_ERROR	= -1	/* error condition */
} ModeStatus;

typedef enum {
    V_PHSYNC	= 0x0001,
    V_NHSYNC	= 0x0002,
    V_PVSYNC	= 0x0004,
    V_NVSYNC	= 0x0008,
    V_INTERLACE	= 0x0010,
    V_DBLSCAN	= 0x0020,
    V_CSYNC	= 0x0040,
    V_PCSYNC	= 0x0080,
    V_NCSYNC	= 0x0100,
    V_HSKEW	= 0x0200,	/* hskew provided */
    V_BCAST	= 0x0400,
    V_PIXMUX	= 0x1000,
    V_DBLCLK	= 0x2000,
    V_CLKDIV2	= 0x4000
} ModeFlags;

# define M_T_DEFAULT 0x10 /* (VESA) default modes */
# define M_T_USERDEF 0x20	/* One of the modes from the config file */


/* Video mode */
typedef struct _DisplayModeRec {
    struct _DisplayModeRec *	prev;
    struct _DisplayModeRec *	next;
    char *      name;         /* identifier for the mode */
    ModeStatus			status;
    int				type;

    /* These are the values that the user sees/provides */
    int       Clock;          /* pixel clock freq */
    int       HDisplay;       /* horizontal timing */
    int				HSyncStart;
    int				HSyncEnd;
    int				HTotal;
    int				HSkew;
    int       VDisplay;       /* vertical timing */
    int				VSyncStart;
    int				VSyncEnd;
    int				VTotal;
    int				VScan;
    int				Flags;

  /* These are the values the hardware uses */
    int				ClockIndex;
    int       SynthClock;     /* Actual clock freq to
					  	 * be programmed */
    int				CrtcHDisplay;
    int				CrtcHBlankStart;
    int				CrtcHSyncStart;
    int				CrtcHSyncEnd;
    int				CrtcHBlankEnd;
    int				CrtcHTotal;
    int				CrtcHSkew;
    int				CrtcVDisplay;
    int				CrtcVBlankStart;
    int				CrtcVSyncStart;
    int				CrtcVSyncEnd;
    int				CrtcVBlankEnd;
    int				CrtcVTotal;
    Bool			CrtcHAdjusted;
    Bool			CrtcVAdjusted;
    int				PrivSize;
    CARD32*   Private;
    int				PrivFlags;

    float			HSync, VRefresh;
} DisplayModeRec, *DisplayModePtr;

typedef struct
{
    unsigned short	red, green, blue;
} LOCO;


static void __attribute__ ((always_inline))
__clear (void * dst, unsigned len)
{ u32_t tmp;
  asm __volatile__
  (
    "xor eax, eax \n\t"
    "cld \n\t"
    "rep stosb"
    :"=c"(tmp),"=D"(tmp)
    :"c"(len),"D"(dst)
    :"memory","eax","cc"
  );
};

static int __attribute__ ((always_inline))
abs (int i)
{
  return i < 0 ? -i : i;
};

#define DPMSModeOn  0
#define DPMSModeStandby	1
#define DPMSModeSuspend	2
#define DPMSModeOff	3



#define max(x,y)  (((y)>(x))?(y):(x))
#define min(x,y)  (((y)<(x))?(y):(x))

