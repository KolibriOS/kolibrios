
#include "pci.h"

#include "rhd_regs.h"

typedef struct _ScrnInfoRec *ScrnInfoPtr;
typedef struct RHDRec       *RHDPtr;

typedef struct {
    int			frameX0;
    int			frameY0;
    int			virtualX;
    int			virtualY;
    int			depth;
    int			fbbpp;
  //  rgb     weight;
  //  rgb     blackColour;
  //  rgb     whiteColour;
    int			defaultVisual;
    char **		modes;
    pointer		options;
} DispRec, *DispPtr;


typedef struct _ScrnInfoRec
{
    int       scrnIndex;
    RHDPtr    rhdPtr;
//    int     driverVersion;
    char *		driverName;		/* canonical name used in */
						/* the config file */
//    ScreenPtr   pScreen;    /* Pointer to the ScreenRec */
//    int     scrnIndex;    /* Number of this screen */
//    Bool    configured;   /* Is this screen valid */
//    int     origIndex;    /* initial number assigned to
//             * this screen before
//             * finalising the number of
//             * available screens */

    /* Display-wide screenInfo values needed by this screen */
//    int     imageByteOrder;
//    int     bitmapScanlineUnit;
//    int     bitmapScanlinePad;
//    int     bitmapBitOrder;
//    int     numFormats;
//    PixmapFormatRec formats[MAXFORMATS];
//    PixmapFormatRec fbFormat;

    int     bitsPerPixel;            /* fb bpp */
//    Pix24Flags    pixmap24;          /* pixmap pref for depth 24 */
    int     depth;                   /* depth of default visual */
//    MessageType   depthFrom;         /* set from config? */
//    MessageType   bitsPerPixelFrom; /* set from config? */
//    rgb     weight;     /* r/g/b weights */
//    rgb     mask;     /* rgb masks */
//    rgb     offset;     /* rgb offsets */
//    int     rgbBits;    /* Number of bits in r/g/b */
//    Gamma   gamma;      /* Gamma of the monitor */
//    int     defaultVisual;    /* default visual class */
    int     maxHValue;               /* max horizontal timing */
    int     maxVValue;               /* max vertical timing value */
    int     virtualX;                /* Virtual width */
    int     virtualY;                /* Virtual height */
    int     xInc;                    /* Horizontal timing increment */
//    MessageType   virtualFrom;    /* set from config? */
    int     displayWidth;            /* memory pitch */
    int     frameX0;                 /* viewport position */
    int			frameY0;
    int			frameX1;
    int			frameY1;
    int     zoomLocked;              /* Disallow mode changes */
    DisplayModePtr  modePool;        /* list of compatible modes */
    DisplayModePtr  modes;           /* list of actual modes */
    DisplayModePtr  currentMode;     /* current mode
						 * This was previously
						 * overloaded with the modes
						 * field, which is a pointer
						 * into a circular list */
//    confScreenPtr confScreen;   /* Screen config info */
//    MonPtr    monitor;    /* Monitor information */
    DispPtr   display;               /* Display information */
//    int *   entityList;   /* List of device entities */
//    int     numEntities;
    int     widthmm;                 /* physical display dimensions in mm */
    int			heightmm;
    int     xDpi;                    /* width DPI */
    int     yDpi;                    /* height DPI */
    char *    name;                  /* Name to prefix messages */
//    pointer   driverPrivate;    /* Driver private area */
//    DevUnion *    privates;   /* Other privates can hook in
//             * here */
//    DriverPtr   drv;      /* xf86DriverList[] entry */
//    pointer   module;     /* Pointer to module head */
//    int     colorKey;
//    int     overlayFlags;

    /* Some of these may be moved out of here into the driver private area */

//    char *    chipset;    /* chipset name */
//    char *    ramdac;     /* ramdac name */
//    char *    clockchip;    /* clock name */
 //   Bool    progClock;    /* clock is programmable */
//    int     numClocks;    /* number of clocks */
//    int     clock[MAXCLOCKS]; /* list of clock frequencies */
//    int     videoRam;   /* amount of video ram (kb) */
//    unsigned long biosBase;   /* Base address of video BIOS */
//    unsigned long memPhysBase;    /* Physical address of FB */
//    unsigned long   fbOffset;   /* Offset of FB in the above */
//    IOADDRESS     domainIOBase;   /* Domain I/O base address */
//    int     memClk;     /* memory clock */
//    int     textClockFreq;    /* clock of text mode */
//    Bool    flipPixels;   /* swap default black/white */
//    pointer   options;

//    int     chipID;
//    int     chipRev;
//    int     racMemFlags;
//    int     racIoFlags;
//    pointer   access;
//    xf86CurrentAccessPtr CurrentAccess;
//    resType   resourceType;
//    pointer   busAccess;

    /* Allow screens to be enabled/disabled individually */
//    Bool    vtSema;
//    DevUnion    pixmapPrivate;    /* saved devPrivate from pixmap */

    /* hw cursor moves at SIGIO time */
//    Bool    silkenMouse;

    /* Storage for clockRanges and adjustFlags for use with the VidMode ext */
//    ClockRangesPtr  clockRanges;
//    int     adjustFlags;

    /*
     * These can be used when the minor ABI version is incremented.
     * The NUM_* parameters must be reduced appropriately to keep the
     * structure size and alignment unchanged.
     */
//    int     reservedInt[NUM_RESERVED_INTS];

//    int *   entityInstanceList;
//    pointer   reservedPtr[NUM_RESERVED_POINTERS];

    /*
     * Driver entry points.
     *
     */

/*
    xf86ProbeProc			*Probe;
    xf86PreInitProc			*PreInit;
    xf86ScreenInitProc			*ScreenInit;
    xf86SwitchModeProc			*SwitchMode;
    xf86AdjustFrameProc			*AdjustFrame;
    xf86EnterVTProc			*EnterVT;
    xf86LeaveVTProc			*LeaveVT;
    xf86FreeScreenProc			*FreeScreen;
    xf86ValidModeProc			*ValidMode;
    xf86EnableDisableFBAccessProc	*EnableDisableFBAccess;
    xf86SetDGAModeProc			*SetDGAMode;
    xf86ChangeGammaProc			*ChangeGamma;
    xf86PointerMovedProc		*PointerMoved;
    xf86PMEventProc			*PMEvent;
    xf86HandleMessageProc		*HandleMessage;
    xf86DPMSSetProc			*DPMSSet;
    xf86LoadPaletteProc			*LoadPalette;
    xf86SetOverscanProc			*SetOverscan;
    xorgRRFuncProc			*RRFunc;
*/
    /*
     * This can be used when the minor ABI version is incremented.
     * The NUM_* parameter must be reduced appropriately to keep the
     * structure size and alignment unchanged.
     */
//    funcPointer   reservedFuncs[NUM_RESERVED_FUNCS];

} ScrnInfoRec;



#pragma pack(push, 1)
typedef struct
{
  CARD16 device;
  CARD16 family;
}PciChipset_t;
#pragma pack(pop)

#define VENDOR_ATI 0x1002

enum RHD_CHIPSETS {
    RHD_UNKNOWN = 0,
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
    RHD_M88,
    RHD_R680,
    RHD_RV620,
    RHD_M82,
    RHD_RV635,
    RHD_M86,
    RHD_RS780,
    RHD_RV770,
    RHD_R700,
    RHD_M98,
    RHD_RV730,
    RHD_M96,
    RHD_RV710,
    RHD_CHIP_END
};

enum RHD_FAMILIES {
    RHD_FAMILY_UNKNOWN = 0,
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

enum RHD_HPD_USAGE {
    RHD_HPD_USAGE_AUTO = 0,
    RHD_HPD_USAGE_OFF,
    RHD_HPD_USAGE_NORMAL,
    RHD_HPD_USAGE_SWAP,
    RHD_HPD_USAGE_AUTO_SWAP,
    RHD_HPD_USAGE_AUTO_OFF
};

enum RHD_TV_MODE {
    RHD_TV_NONE = 0,
    RHD_TV_NTSC = 1,
    RHD_TV_NTSCJ = 1 << 2,
    RHD_TV_PAL = 1 << 3,
    RHD_TV_PALM = 1 << 4,
    RHD_TV_PALCN = 1 << 5,
    RHD_TV_PALN = 1 << 6,
    RHD_TV_PAL60 = 1 << 7,
    RHD_TV_SECAM = 1 << 8,
    RHD_TV_CV = 1 << 9
};

enum rhdPropertyAction {
    rhdPropertyCheck,
    rhdPropertyGet,
    rhdPropertySet
};

union rhdPropertyData
{
    CARD32 integer;
    char *string;
    Bool Bool;
};

#define RHD_CONNECTORS_MAX 6

/* Just define where which PCI BAR lives for now. Will deal with different
 * locations as soon as cards with a different BAR layout arrives.
 */
#define RHD_FB_BAR   0
#define RHD_MMIO_BAR 2

/* More realistic powermanagement */
#define RHD_POWER_ON       0
#define RHD_POWER_RESET    1   /* off temporarily */
#define RHD_POWER_SHUTDOWN 2   /* long term shutdown */
#define RHD_POWER_UNKNOWN  3   /* initial state */

#define RHD_MEM_GART       1
#define RHD_MEM_FB         2


enum rhdCardType {
    RHD_CARD_NONE,
    RHD_CARD_AGP,
    RHD_CARD_PCIE
};

enum {
    RHD_PCI_CAPID_AGP    = 0x02,
    RHD_PCI_CAPID_PCIE   = 0x10
};

typedef struct BIOSScratchOutputPrivate rhdOutputDriverPrivate;
typedef struct _rhdI2CRec *rhdI2CPtr;
typedef struct _atomBiosHandle *atomBiosHandlePtr;
typedef struct _rhdShadowRec *rhdShadowPtr;

typedef struct _RHDopt {
    Bool set;
    union  {
        Bool bool;
        int integer;
        unsigned long uslong;
        double real;
        double freq;
        char *string;
    } val;
} RHDOpt, *RHDOptPtr;


////////////////////////////
typedef enum
{
    CONNECTOR_NONE,            // 0
    CONNECTOR_VGA,             // 1
    CONNECTOR_DVI_I,           // 2
    CONNECTOR_DVI_D,           // 3
    CONNECTOR_DVI_A,           // 4
    CONNECTOR_STV,             // 5
    CONNECTOR_CTV,             // 6
    CONNECTOR_LVDS,            // 7
    CONNECTOR_DIGITAL,         // 8
    CONNECTOR_SCART,           // 9
    CONNECTOR_HDMI_TYPE_A,     // 10
    CONNECTOR_HDMI_TYPE_B,     // 11
    CONNECTOR_0XC,             // 12
    CONNECTOR_0XD,             // 13
    CONNECTOR_DIN,             // 14
    CONNECTOR_DISPLAY_PORT,    // 15
    CONNECTOR_UNSUPPORTED
} RADEONConnectorType;

typedef enum
{
    DAC_NONE    = 0,
    DAC_PRIMARY = 1,
    DAC_TVDAC   = 2,
    DAC_EXT     = 3
} RADEONDacType;

typedef enum
{
    TMDS_NONE    = 0,
    TMDS_INT     = 1,
    TMDS_EXT     = 2,
    TMDS_LVTMA   = 3,
    TMDS_DDIA    = 4
} RADEONTmdsType;

typedef struct
{
    Bool   valid;
    CARD32 mask_clk_reg;
    CARD32 mask_data_reg;
    CARD32 put_clk_reg;
    CARD32 put_data_reg;
    CARD32 get_clk_reg;
    CARD32 get_data_reg;
    CARD32 mask_clk_mask;
    CARD32 mask_data_mask;
    CARD32 put_clk_mask;
    CARD32 put_data_mask;
    CARD32 get_clk_mask;
    CARD32 get_data_mask;
} RADEONI2CBusRec, *RADEONI2CBusPtr;

typedef struct {
    RADEONDacType DACType;
    RADEONTmdsType TMDSType;
    RADEONConnectorType ConnectorType;
    Bool valid;
    int output_id;
    int devices;
    int hpd_mask;
    RADEONI2CBusRec ddc_i2c;
} RADEONBIOSConnector;

///////////////////////////////////////////



typedef struct RHDRec
{
  ScrnInfoPtr            pScrn;
  int                    scrnIndex;

  CARD32                 MMIOBase;
  CARD32                 MMIOMapSize;
  CARD32                 videoRam;

  enum RHD_HPD_USAGE     hpdUsage;
  RHDOpt                 forceReduced;

  CARD32                 FbBase;            /* map base of fb   */
  CARD32                 FbIntAddress;      /* card internal address of FB */
 CARD32                  FbIntSize;         /* card internal FB aperture size */

  CARD32                 FbMapSize;

  CARD32                 FbFreeStart;
  CARD32                 FbFreeSize;

  /* visible part of the framebuffer */
  unsigned int           FbScanoutStart;
  unsigned int           FbScanoutSize;

  unsigned char*         BIOSCopy;

  enum RHD_CHIPSETS      ChipSet;
  struct rhdCard         *Card;
  char                   *chipset_name;

  Bool                   IsMobility;
  Bool                   IsIGP;
  Bool                   HasCRTC2;
  Bool                   HasSingleDAC;
  Bool                   InternalTVOut;

  u32_t                  bus;
  u32_t                  devfn;

  PCITAG                 PciTag;
  PCITAG                 NBPciTag;

  CARD16                 PciDeviceID;
  enum rhdCardType       cardType;

  CARD16                 subvendor_id;
  CARD16                 subdevice_id;
  pciVideoRec            pci;

  struct _I2CBusRec     **I2C;               /* I2C bus list */
  atomBiosHandlePtr      atomBIOS;           /* handle for AtomBIOS */

  struct rhdMC          *MC;
  struct rhdVGA         *VGA;
  struct rhdCrtc        *Crtc[2];
  struct rhdPLL         *PLLs[2];            /* Pixelclock PLLs */

  struct rhdLUTStore    *LUTStore;
  struct rhdLUT         *LUT[2];

  struct rhdConnector   *Connector[RHD_CONNECTORS_MAX];

  struct rhdOutput      *Outputs;

  struct rhdHPD         *HPD;                /* Hot plug detect subsystem */
  enum RHD_TV_MODE       tvMode;
  struct rhdMonitor     *ConfigMonitor;

  struct mem_block      *fb_heap;
  struct mem_block      *gart_heap;

  RHDOpt                 scaleTypeOpt;

  int                    verbosity;


   /* AtomBIOS usage */
  RHDOpt                 UseAtomBIOS;
  CARD32                 UseAtomFlags;

  struct rhdOutput *DigEncoderOutput[2];
}RHD_t;

typedef struct {
    int			token;		/* id of the token */
    const char *	name;		/* token name */
} SymTabRec, *SymTabPtr;

extern inline CARD32 _RHDRegRead(RHDPtr rhdPtr, CARD16 offset)
{
  return *(volatile CARD32 *)((CARD8*)(rhdPtr->MMIOBase + offset));
}

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

extern inline CARD32
_RHDReadPLL(RHDPtr rhdPtr, CARD16 offset)
{
  _RHDRegWrite(rhdPtr, CLOCK_CNTL_INDEX, (offset & PLL_ADDR));
  return _RHDRegRead(rhdPtr, CLOCK_CNTL_DATA);
}

extern inline void
_RHDWritePLL(RHDPtr rhdPtr, CARD16 offset, CARD32 data)
{
  _RHDRegWrite(rhdPtr, CLOCK_CNTL_INDEX, (offset & PLL_ADDR) | PLL_WR_EN);
  _RHDRegWrite(rhdPtr, CLOCK_CNTL_DATA, data);
}


enum RHD_FAMILIES RHDFamily(enum RHD_CHIPSETS chipset);

extern CARD32 _RHDReadMC(int scrnIndex, CARD32 addr);
#define RHDReadMC(ptr,addr) _RHDReadMC((int)(ptr),(addr))

extern void _RHDWriteMC(int scrnIndex, CARD32 addr, CARD32 data);
#define RHDWriteMC(ptr,addr,value) _RHDWriteMC((int)(ptr),(addr),(value))

#define RHDRegRead(ptr, offset) \
        _RHDRegRead((RHDPtr)((ptr)->scrnIndex), (offset))

#define RHDRegWrite(ptr, offset, value) \
        _RHDRegWrite((RHDPtr)((ptr)->scrnIndex), (offset), (value))

#define RHDRegMask(ptr, offset, value, mask) \
        _RHDRegMask((RHDPtr)((ptr)->scrnIndex), (offset), (value), (mask))

#define RHDRegMaskD(ptr, offset, value, mask) \
        RHDRegMask(ptr, offset, value, mask)

char * RhdAppendString(char *s1, const char *s2);



#define  LOG_DEBUG    0

#define  X_ERROR      0
#define  X_WARNING    1
#define  X_INFO       2
#define  X_NONE       3
#define  X_PROBED     4

/*
#ifdef DBG_ALL
 #undef  DBG_CALL
 #undef  DBG_MSG
 #undef  DBG_CAIL

 #define DBG_CALL
 #define DBG_MSG
 #define DBG_CAIL
#endif
*/


#ifdef DBG_CALL
  #define RHDFUNC(ptr) dbgprintf("FUNCTION: %s\n", __func__)
#else
  #define RHDFUNC(ptr)
#endif

#ifdef DBG_MSG
  #define xf86Msg(a, format,...)            dbgprintf(format,##__VA_ARGS__)
  #define xf86MsgVerb(a,b,format,...)       dbgprintf(format,##__VA_ARGS__)
  #define xf86DrvMsg(a,b,format,...)        dbgprintf(format,##__VA_ARGS__)
  #define xf86DrvMsgVerb(a,b,c,format,...)  dbgprintf(format,##__VA_ARGS__)
  #define xf86VDrvMsgVerb(a,b,c,format,...) dbgprintf(format,##__VA_ARGS__)

  #define RHDDebug(a,format,...)            dbgprintf(format,##__VA_ARGS__)
  #define RHDDebugCont(format,...)          dbgprintf(format,##__VA_ARGS__)
  #define RHDDebugVerb(a,b,format,...)      dbgprintf(format,##__VA_ARGS__)
#else
  #define xf86Msg(a, format,...)
  #define xf86MsgVerb(a,b,format,...)
  #define xf86DrvMsg(a,b,format,...)
  #define xf86DrvMsgVerb(a,b,c,format,...)
  #define xf86VDrvMsgVerb(a,b,c,format,...)

  #define RHDDebug(a,format,...)
  #define RHDDebugCont(format,...)
  #define RHDDebugVerb(a,b,format,...)
#endif

#ifdef DBG_CAIL
  #define CAILFUNC(a)             dbgprintf("CAIL: %s\n", __func__)
  #define CailDebug(a,format,...) dbgprintf(format,##__VA_ARGS__)
#else
  #define CAILFUNC(a)
  #define CailDebug(a,format,...)
#endif

#define DBG(x) x
#define ErrorF dbgprintf

#define ASSERT(expr)

#define ASSERTF(expr,format)

#define RHDPTRI(p)  ((RHDPtr)((p)->scrnIndex))
#define RHDPTR(p)   ((p)->rhdPtr)

#define RHDFUNCI(scrnIndex) RHDDebug(scrnIndex, "FUNCTION: %s\n", __func__)

enum atomSubSystem {
    atomUsageCrtc,
    atomUsagePLL,
    atomUsageOutput,
    atomUsageAny
};

extern Bool RHDUseAtom(RHDPtr rhdPtr, enum RHD_CHIPSETS *BlackList, enum atomSubSystem subsys);

//git://anongit.freedesktop.org/git/xorg/driver/xf86-video-nv
//git://anongit.freedesktop.org/git/nouveau/xf86-video-nouveau
