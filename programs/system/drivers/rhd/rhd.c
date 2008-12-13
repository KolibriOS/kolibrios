
//ld -T ld.x -s --shared --image-base 0 --file-alignment 32 -o test.exe test.obj core.lib

#include "common.h"
#include "rhd.h"
#include "edid.h"

#include "rhd_atombios.h"
#include "rhd_regs.h"
#include "rhd_mc.h"
#include "rhd_connector.h"
#include "rhd_output.h"
#include "rhd_card.h"
#include "rhd_vga.h"
#include "rhd_crtc.h"
#include "rhd_monitor.h"
#include "rhd_modes.h"
#include "rhd_pll.h"
#include "rhd_lut.h"
#include "rhd_i2c.h"

#define PG_SW       0x003
#define PG_NOCACHE  0x018

void sysSetScreen(int width, int height);

static void rhdModeInit(ScrnInfoPtr pScrn, DisplayModePtr mode);
static void rhdSetMode(ScrnInfoPtr pScrn, DisplayModePtr mode);
static void RHDAdjustFrame(RHDPtr rhdPtr, int x, int y, int flags);
static Bool rhdMapFB(RHDPtr rhdPtr);

Bool OldSetupConnectors(RHDPtr rhdPtr);
Bool OldConnectorsInit(RHDPtr rhdPtr);

int rhdInitHeap(RHDPtr rhdPtr);

static u32_t _PciApi(int cmd);
static int SupportedModes;

int __stdcall drvEntry(int)__asm__("_drvEntry");

typedef struct
{
  unsigned      handle;
  unsigned      io_code;
  void          *input;
  int           inp_size;
  void          *output;
  int           out_size;
}ioctl_t;

typedef int (_stdcall *srv_proc_t)(ioctl_t *);

int _stdcall srv_proc(ioctl_t *io);

extern PciChipset_t   RHDPCIchipsets[];
extern struct rhdCard *RHDCardIdentify(RHDPtr rhdPtr);

static struct RHDRec       rhd;
static struct _ScrnInfoRec Scrn;

void sysSetScreen(int width, int height)
{
  asm __volatile__
  (
    "dec eax \n\t"
    "dec edx \n\t"
    "call [DWORD PTR __imp__SetScreen] \n\t"
    :
    :"a" (width),"d"(height)
    :"memory","cc"
  );
}

static int RegService(char *name, srv_proc_t proc)
{
  int retval;

  asm __volatile__
  (
    "push %[t] \n\t"
    "push %[t1] \n\t"
    "call [DWORD PTR __imp__RegService] \n\t"
    :"=eax" (retval)
    :[t] "g" (proc),[t1] "g" (name)
    :"memory", "ebx"
  );
  return retval;
};

static u32_t _PciApi(int cmd)
{
  u32_t retval;

  asm __volatile__
  (
    "call [DWORD PTR __imp__PciApi]"
    :"=eax" (retval)
    :"a" (cmd)
    :"memory"
  );
  return retval;
};

const PciChipset_t *PciDevMatch(CARD16 dev,const PciChipset_t *list)
{
  while(list->device)
  {
    if(dev==list->device)
      return list;
    list++;
  }
  return 0;
}

const char *
xf86TokenToString(SymTabPtr table, int token)
{
    int i;

    for (i = 0; table[i].token >= 0 && table[i].token != token; i++){};

    if (table[i].token < 0)
      return NULL;
    else
      return(table[i].name);
}

int FindPciDevice()
{
  const PciChipset_t *dev;
  u32_t bus, last_bus;

  if( (last_bus = _PciApi(1))==-1)
    return 0;

  for(bus=0;bus<=last_bus;bus++)
  {
    u32_t devfn;

    for(devfn=0;devfn<256;devfn++)
    {
      u32_t id;
      id = PciRead32(bus,devfn, 0);

      if( (CARD16)id != VENDOR_ATI)
        continue;

      if( (dev=PciDevMatch(id>>16,RHDPCIchipsets))!=0)
      {

        rhd.PciDeviceID = (id>>16);

        rhd.bus = bus;
        rhd.pci.bus = bus;
        rhd.devfn = devfn;
        rhd.pci.devfn = devfn;
        rhd.PciTag = pciTag(bus,(devfn>>3)&0x1F,devfn&0x7);

        rhd.ChipSet = dev->family;
       // rhd.IsMobility  = dev->mobility;
       // rhd.IsIGP       = dev->igp;
       // rhd.HasCRTC2    = !dev->nocrtc2;
       // rhd.HasSingleDAC = dev->singledac;
       // rhd.InternalTVOut = !dev->nointtvout;

        pciGetInfo(&rhd.pci);

        rhd.subvendor_id = rhd.pci.subsysVendor;
        rhd.subdevice_id = rhd.pci.subsysCard;

        //rhd.chipset = (char*)xf86TokenToString(RADEONChipsets, rhd.device_id);

        return 1;
      };
    };
  };

  dbgprintf("Device not found\n");

  return 0;
}


static Bool
rhdMapMMIO()
{

  rhd.MMIOMapSize = 1 << rhd.pci.size[RHD_MMIO_BAR];
  rhd.MMIOBase = MapIoMem(rhd.pci.memBase[RHD_MMIO_BAR],
                          rhd.MMIOMapSize,PG_SW+PG_NOCACHE);
  if( !rhd.MMIOBase)
    return 0;

  DBG(dbgprintf("Mapped IO at %x (size %x)\n", rhd.MMIOBase, rhd.MMIOMapSize));

  return 1;
}

#define RADEON_NB_TOM             0x15c
static CARD32
rhdGetVideoRamSize(RHDPtr rhdPtr)
{
  CARD32 RamSize, BARSize;

  if (rhdPtr->ChipSet == RHD_RS690)
    RamSize = (_RHDRegRead(rhdPtr, R5XX_CONFIG_MEMSIZE))>>10;
  else
    if (rhdPtr->IsIGP)
    {
      CARD32 tom = _RHDRegRead(rhdPtr, RADEON_NB_TOM);
      RamSize = (((tom >> 16) - (tom & 0xffff) + 1) << 6);
      _RHDRegWrite(rhdPtr,R5XX_CONFIG_MEMSIZE, RamSize<<10);
    }
    else
    {
      if (rhdPtr->ChipSet < RHD_R600)
      {
        RamSize = (_RHDRegRead(rhdPtr, R5XX_CONFIG_MEMSIZE)) >> 10;
        if(RamSize==0) RamSize=8192;
      }
      else
        RamSize = (_RHDRegRead(rhdPtr, R6XX_CONFIG_MEMSIZE)) >> 10;
    };

  BARSize = 1 << (rhdPtr->pci.size[RHD_FB_BAR] - 10);
  if(BARSize==0)
    BARSize = 0x20000;

  if (RamSize > BARSize) {
    DBG(dbgprintf("The detected amount of videoram"
           " exceeds the PCI BAR aperture.\n"));
    DBG(dbgprintf("Using only %dkB of the total "
           "%dkB.\n", (int) BARSize, (int) RamSize));
    return BARSize;
  }
  else return RamSize;
}


Bool RHDScalePolicy(struct rhdMonitor *Monitor, struct rhdConnector *Connector)
{
    if (!Monitor || !Monitor->UseFixedModes || !Monitor->NativeMode)
	return FALSE;

    if (Connector->Type != RHD_CONNECTOR_PANEL)
	return FALSE;

    return TRUE;
}

static void
rhdOutputConnectorCheck(struct rhdConnector *Connector)
{
    struct rhdOutput *Output;
    int i;

    /* First, try to sense */
    for (i = 0; i < 2; i++) {
	Output = Connector->Output[i];
	if (Output && Output->Sense) {
	    /*
	     * This is ugly and needs to change when the TV support patches are in.
	     * The problem here is that the Output struct can be used for two connectors
	     * and thus two different devices
	     */
	    if (Output->SensedType == RHD_SENSED_NONE) {
		/* Do this before sensing as AtomBIOS sense needs this info */
		if ((Output->SensedType = Output->Sense(Output, Connector)) != RHD_SENSED_NONE) {
		    RHDOutputPrintSensedType(Output);
		    Output->Connector = Connector;
		    break;
		}
	    }
	}
    }

    if (i == 2) {
	/* now just enable the ones without sensing */
	for (i = 0; i < 2; i++) {
	    Output = Connector->Output[i];
	    if (Output && !Output->Sense) {
		Output->Connector = Connector;
		break;
	    }
	}
    }
}

static Bool
rhdModeLayoutSelect(RHDPtr rhdPtr)
{
  struct rhdOutput *Output;
  struct rhdConnector *Connector;
  Bool Found = FALSE;
  char *ignore = NULL;
  Bool ConnectorIsDMS59 = FALSE;
  int i = 0;

  RHDFUNC(rhdPtr);

  /* housekeeping */
  rhdPtr->Crtc[0]->PLL = rhdPtr->PLLs[0];
  rhdPtr->Crtc[0]->LUT = rhdPtr->LUT[0];

  rhdPtr->Crtc[1]->PLL = rhdPtr->PLLs[1];
  rhdPtr->Crtc[1]->LUT = rhdPtr->LUT[1];

  /* start layout afresh */
  for (Output = rhdPtr->Outputs; Output; Output = Output->Next)
  {
    Output->Active = FALSE;
    Output->Crtc = NULL;
    Output->Connector = NULL;
  }

    /* quick and dirty option so that some output choice exists */
//  ignore = xf86GetOptValString(rhdPtr->Options, OPTION_IGNORECONNECTOR);

    /* handle cards with DMS-59 connectors appropriately. The DMS-59 to VGA
       adapter does not raise HPD at all, so we need a fallback there. */
  if (rhdPtr->Card)
  {
    ConnectorIsDMS59 = rhdPtr->Card->flags & RHD_CARD_FLAG_DMS59;
    if (ConnectorIsDMS59)
      dbgprintf("Card %s has a DMS-59 connector.\n", rhdPtr->Card->name);
  }

    /* Check on the basis of Connector->HPD */
  for (i = 0; i < RHD_CONNECTORS_MAX; i++)
  {
    Connector = rhdPtr->Connector[i];

    if (!Connector)
	    continue;

    if (Connector->HPDCheck)
    {
        if (Connector->HPDCheck(Connector))
        {
          Connector->HPDAttached = TRUE;
          rhdOutputConnectorCheck(Connector);
        }
        else
        {
          Connector->HPDAttached = FALSE;
          if (ConnectorIsDMS59)
            rhdOutputConnectorCheck(Connector);
	    }
    }
    else
      rhdOutputConnectorCheck(Connector);
  }

  i = 0; /* counter for CRTCs */
  for (Output = rhdPtr->Outputs; Output; Output = Output->Next)
    if (Output->Connector)
    {
      struct rhdMonitor *Monitor = NULL;

      Connector = Output->Connector;

      Monitor = RHDMonitorInit(Connector);

      if (!Monitor && (Connector->Type == RHD_CONNECTOR_PANEL))
      {
		xf86DrvMsg(rhdPtr->scrnIndex, X_WARNING, "Unable to attach a"
			   " monitor to connector \"%s\"\n", Connector->Name);
        Output->Active = FALSE;
      }
      else
      {
        Connector->Monitor = Monitor;

        Output->Active = TRUE;

        Output->Crtc = rhdPtr->Crtc[i & 1]; /* ;) */
        i++;

        Output->Crtc->Active = TRUE;

        if (RHDScalePolicy(Monitor, Connector))
        {
		    Output->Crtc->ScaledToMode = RHDModeCopy(Monitor->NativeMode);
		    xf86DrvMsg(rhdPtr->scrnIndex, X_INFO,
			       "Crtc[%i]: found native mode from Monitor[%s]: ",
			       Output->Crtc->Id, Monitor->Name);
		    RHDPrintModeline(Output->Crtc->ScaledToMode);
		}
        Found = TRUE;

        if (Monitor)
        {
  /* If this is a DVI attached monitor, enable reduced blanking.
   * TODO: iiyama vm pro 453: CRT with DVI-D == No reduced.
   */
          if ((Output->Id == RHD_OUTPUT_TMDSA) ||
              (Output->Id == RHD_OUTPUT_LVTMA) ||
              (Output->Id == RHD_OUTPUT_KLDSKP_LVTMA) ||
              (Output->Id == RHD_OUTPUT_UNIPHYA) ||
              (Output->Id == RHD_OUTPUT_UNIPHYB))
            Monitor->ReducedAllowed = TRUE;

		    /* allow user to override settings globally */
          if (rhdPtr->forceReduced.set)
            Monitor->ReducedAllowed = rhdPtr->forceReduced.val.bool;

          xf86DrvMsg(rhdPtr->scrnIndex, X_INFO,
                     "Connector \"%s\" uses Monitor \"%s\":\n",
                      Connector->Name, Monitor->Name);
          RHDMonitorPrint(Monitor);
        }
        else
          xf86DrvMsg(rhdPtr->scrnIndex, X_WARNING,
                     "Connector \"%s\": Failed to retrieve Monitor"
                     " information.\n", Connector->Name);
      }
    }

    /* Now validate the scaled modes attached to crtcs */
    for (i = 0; i < 2; i++)
    {
      struct rhdCrtc *crtc = rhdPtr->Crtc[i];
      if (crtc->ScaledToMode && RHDValidateScaledToMode(crtc, crtc->ScaledToMode) != MODE_OK)
      {
	    xf86DrvMsg(rhdPtr->scrnIndex, X_ERROR, "Crtc[%i]: scaled mode invalid.\n", crtc->Id);
	    xfree(crtc->ScaledToMode);
	    crtc->ScaledToMode = NULL;
      }
    };
  return Found;
}

void rhdModeLayoutPrint(RHDPtr rhdPtr)
{
  struct rhdCrtc *Crtc;
  struct rhdOutput *Output;
  Bool Found;

  xf86DrvMsg(rhdPtr->scrnIndex, X_INFO, "Listing modesetting layout:\n\n");

    /* CRTC 1 */
  Crtc = rhdPtr->Crtc[0];
    if (Crtc->Active) {
	xf86Msg(X_NONE, "\t%s: tied to %s and %s:\n",
              Crtc->Name, Crtc->PLL->Name, Crtc->LUT->Name);

    Found = FALSE;
    for (Output = rhdPtr->Outputs; Output; Output = Output->Next)
	    if (Output->Active && (Output->Crtc == Crtc)) {
		if (!Found) {
          xf86Msg(X_NONE, "\t\tOutputs: %s (%s)",
                  Output->Name, Output->Connector->Name);
          Found = TRUE;
       } else
          xf86Msg(X_NONE, ", %s (%s)", Output->Name,
                 Output->Connector->Name);
    }

    if (!Found)
	    xf86DrvMsg(rhdPtr->scrnIndex, X_ERROR,
		       "%s is active without outputs\n", Crtc->Name);
    else
	     xf86Msg(X_NONE, "\n");
    } else
	xf86Msg(X_NONE, "\t%s: unused\n", Crtc->Name);
    xf86Msg(X_NONE, "\n");

    /* CRTC 2 */
  Crtc = rhdPtr->Crtc[1];
    if (Crtc->Active) {
	xf86Msg(X_NONE, "\t%s: tied to %s and %s:\n",
      Crtc->Name, Crtc->PLL->Name, Crtc->LUT->Name);

    Found = FALSE;
    for (Output = rhdPtr->Outputs; Output; Output = Output->Next)
	    if (Output->Active && (Output->Crtc == Crtc)) {
		if (!Found) {
		    xf86Msg(X_NONE, "\t\tOutputs: %s (%s)",
			    Output->Name, Output->Connector->Name);
          Found = TRUE;
		} else
		    xf86Msg(X_NONE, ", %s (%s)", Output->Name,
			    Output->Connector->Name);
        }

    if (!Found)
	    xf86DrvMsg(rhdPtr->scrnIndex, X_ERROR,
		       "%s is active without outputs\n", Crtc->Name);
    else
	    xf86Msg(X_NONE, "\n");
    } else
	xf86Msg(X_NONE, "\t%s: unused\n", Crtc->Name);
    xf86Msg(X_NONE, "\n");

    /* Print out unused Outputs */
  Found = FALSE;
  for (Output = rhdPtr->Outputs; Output; Output = Output->Next)
	if (!Output->Active) {
	    if (!Found) {
		xf86Msg(X_NONE, "\t\tUnused Outputs: %s", Output->Name);
      Found = TRUE;
	    } else
		xf86Msg(X_NONE, ", %s", Output->Name);
    }

  if (Found)
	xf86Msg(X_NONE, "\n");
    xf86Msg(X_NONE, "\n");
}

DisplayModePtr
rhdCreateModesListAndValidate(ScrnInfoPtr pScrn, Bool Silent);
void RHDPrintModeline(DisplayModePtr mode);

int RHDPreInit()
{
  RHDI2CDataArg i2cArg;

  if (rhd.Card && rhd.Card->flags & RHD_CARD_FLAG_HPDSWAP &&
      rhd.hpdUsage == RHD_HPD_USAGE_AUTO)
    rhd.hpdUsage = RHD_HPD_USAGE_AUTO_SWAP;
  if (rhd.Card && rhd.Card->flags & RHD_CARD_FLAG_HPDOFF &&
      rhd.hpdUsage == RHD_HPD_USAGE_AUTO)
    rhd.hpdUsage = RHD_HPD_USAGE_AUTO_OFF;

    /* We need access to IO space already */
  if (!rhdMapMMIO()) {
    dbgprintf("Failed to map MMIO.\n");
    return 0;
  };

  rhd.videoRam = rhdGetVideoRamSize(&rhd);
  if (!rhd.videoRam)
  {
    dbgprintf("No Video RAM detected.\n");
	    goto error1;
	}
  dbgprintf("VideoRAM: %d kByte\n",rhd.videoRam);

  rhd.FbFreeStart = 0;
  rhd.FbFreeSize = rhd.videoRam << 10;

#ifdef ATOM_BIOS
  {
    AtomBiosArgRec atomBiosArg;

    rhd.UseAtomFlags = (RHD_ATOMBIOS_ON << RHD_ATOMBIOS_CRTC)   |
                       (RHD_ATOMBIOS_ON << RHD_ATOMBIOS_OUTPUT) |
                       (RHD_ATOMBIOS_ON << RHD_ATOMBIOS_PLL);

   // rhd.UseAtomFlags = 0;

    if (RHDAtomBiosFunc(&rhd, NULL, ATOMBIOS_INIT, &atomBiosArg)
        == ATOM_SUCCESS)
    {
      rhd.atomBIOS = atomBiosArg.atomhandle;
    }
  }

  if (rhd.atomBIOS)                        /* for testing functions */
  {
    AtomBiosArgRec atomBiosArg;

    atomBiosArg.fb.start = rhd.FbFreeStart;
    atomBiosArg.fb.size = rhd.FbFreeSize;
    if (RHDAtomBiosFunc(&rhd, rhd.atomBIOS, ATOMBIOS_ALLOCATE_FB_SCRATCH,
        &atomBiosArg) == ATOM_SUCCESS)
    {
      rhd.FbFreeStart = atomBiosArg.fb.start;
      rhd.FbFreeSize = atomBiosArg.fb.size;
    };
    RHDAtomBiosFunc(&rhd, rhd.atomBIOS, GET_DEFAULT_ENGINE_CLOCK, &atomBiosArg);
    RHDAtomBiosFunc(&rhd, rhd.atomBIOS, GET_DEFAULT_MEMORY_CLOCK, &atomBiosArg);
    RHDAtomBiosFunc(&rhd, rhd.atomBIOS, GET_MAX_PIXEL_CLOCK_PLL_OUTPUT, &atomBiosArg);
    RHDAtomBiosFunc(&rhd, rhd.atomBIOS, GET_MIN_PIXEL_CLOCK_PLL_OUTPUT, &atomBiosArg);
    RHDAtomBiosFunc(&rhd, rhd.atomBIOS, GET_MAX_PIXEL_CLOCK_PLL_INPUT, &atomBiosArg);
    RHDAtomBiosFunc(&rhd, rhd.atomBIOS, GET_MIN_PIXEL_CLOCK_PLL_INPUT, &atomBiosArg);
    RHDAtomBiosFunc(&rhd, rhd.atomBIOS, GET_MAX_PIXEL_CLK, &atomBiosArg);
    RHDAtomBiosFunc(&rhd, rhd.atomBIOS, GET_REF_CLOCK, &atomBiosArg);
  }
#endif

  if (RHDI2CFunc((int)&rhd, NULL, RHD_I2C_INIT, &i2cArg) == RHD_I2C_SUCCESS)
    rhd.I2C = i2cArg.I2CBusList;
  else
  {
    dbgprintf("I2C init failed\n");
    goto error1;
  };

  if (!rhd.atomBIOS)
  {
    dbgprintf("No ATOMBIOS detected.  Done.\n");
    return 0;
  }

  rhdMapFB(&rhd);

  Scrn.rhdPtr       = &rhd;
  Scrn.driverName   = "Radeon HD driver";
  Scrn.bitsPerPixel = 32;
  Scrn.depth        = 32;
  Scrn.virtualX     = 1280;
  Scrn.virtualY     = 1024;
  Scrn.displayWidth = 1280;

  rhd.pScrn = &Scrn;

  rhd.FbScanoutStart = 0;
  rhd.FbScanoutSize  = 8*1024*1024;
  rhd.FbFreeStart    = 8*1024*1024;
  rhd.FbFreeSize     = rhd.FbMapSize - 8*1024*1024;

  rhdInitHeap(&rhd);

  RHDVGAInit(&rhd);
  RHDMCInit(&rhd);
  if (!RHDCrtcsInit(&rhd))
    RHDAtomCrtcsInit(&rhd);
  if (!RHDPLLsInit(&rhd))
    RHDAtomPLLsInit(&rhd);

  RHDLUTsInit(&rhd);

  if (!RHDConnectorsInit(&rhd, rhd.Card))
  {
    dbgprintf("Card information has invalid connector information\n");
    goto error1;
  }

  if (!rhdModeLayoutSelect(&rhd))
  {
    dbgprintf("Failed to detect a connected monitor\n");
    goto error1;
	}
  RHDConfigMonitorSet(&rhd, FALSE);
  rhdModeLayoutPrint(&rhd);

  {
    DisplayModePtr Modes, tmp;

    Modes = RHDModesPoolCreate(&Scrn, FALSE);
    Scrn.modePool = Modes;

    tmp = Modes;
    SupportedModes=0;
    while(tmp)
    {
      dbgprintf("%dx%d@%3.1fHz\n",tmp->CrtcHDisplay,
                tmp->CrtcVDisplay,tmp->VRefresh);
      tmp=tmp->next;
      SupportedModes++;
    };
//    rhdModeInit(&Scrn,Modes);
    //RHDAdjustFrame(&rhd,0,0,0);
  };
  dbgprintf("All done\n");
  return 1;

error1:
  return 0;
};

int __stdcall drvEntry(int action)
{
  int i;

  if(action != 1)
    return 0;

  if(!dbg_open("/rd/1/ati.txt"))
  {
     printf("Can't open /rd/1/ati.txt\nExit\n");
     return 0;
  }
  if(!FindPciDevice())
    return 0;

  rhd.scrnIndex = (int)&rhd;

  rhd.Card = RHDCardIdentify(&rhd);
  if (rhd.Card)
    dbgprintf("Detected an %s on a %s\n", rhd.chipset_name, rhd.Card->name);
  else
    dbgprintf("Detected an %s on an unidentified card\n", rhd.chipset_name);

  for(i=0;i<6;i++)
  {
    if(rhd.pci.memBase[i])
      dbgprintf("Memory base_%d 0x%x size 0x%x\n",
                i,rhd.pci.memBase[i],(1<<rhd.pci.size[i]));
  };
  for(i=0;i<6;i++)
  {
    if(rhd.pci.ioBase[i])
      dbgprintf("Io base_%d 0x%x size 0x%x\n",
                i,rhd.pci.ioBase[i],(1<<rhd.pci.size[i]));
  };
  if(RHDPreInit()==0)
    return 0;

  return RegService("RHD", srv_proc);
};


void usleep(u32_t delay)
{
  if(!delay) delay++;
  delay*=2000;

  asm(
     "1:\n\t"
      "xor eax, eax \n\t"
      "cpuid \n\t"
      "dec edi \n\t"
      "jnz 1b"
      :
      :"D"(delay)
      :"eax","ebx","ecx","edx"
     );
}


//git://anongit.freedesktop.org/git/xorg/xserver
//git://anongit.freedesktop.org/git/xorg/lib/libpciaccess

int KernelFree(void *p)
{

  return 0;
}

static void
rhdPrepareMode(RHDPtr rhdPtr)
{
    RHDFUNC(rhdPtr);

    /* no active outputs == no mess */
    RHDOutputsPower(rhdPtr, RHD_POWER_RESET);

    /* Disable CRTCs to stop noise from appearing. */
    rhdPtr->Crtc[0]->Power(rhdPtr->Crtc[0], RHD_POWER_RESET);
    rhdPtr->Crtc[1]->Power(rhdPtr->Crtc[1], RHD_POWER_RESET);
}


static void
rhdModeInit(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    RHDPtr rhdPtr = pScrn->rhdPtr;

    RHDFUNC(rhdPtr);
//    pScrn->vtSema = TRUE;

    /* Stop crap from being shown: gets reenabled through SaveScreen */
//    rhdPtr->Crtc[0]->Blank(rhdPtr->Crtc[0], TRUE);
//    rhdPtr->Crtc[1]->Blank(rhdPtr->Crtc[1], TRUE);

    rhdPrepareMode(rhdPtr);

    /* now disable our VGA Mode */
    RHDVGADisable(rhdPtr);

    /* now set up the MC */
    RHDMCSetup(rhdPtr);

    rhdSetMode(pScrn, mode);
}

static void
rhdSetMode(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    RHDPtr rhdPtr = RHDPTR(pScrn);
    int i;

    RHDFUNC(rhdPtr);

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Setting up \"%s\" (%dx%d@%3.1fHz)\n",
	       mode->name, mode->CrtcHDisplay, mode->CrtcVDisplay,
	       mode->VRefresh);

    /* Set up D1/D2 and appendages */
    for (i = 0; i < 2; i++) {
	struct rhdCrtc *Crtc;

	Crtc = rhdPtr->Crtc[i];
	if (Crtc->Active) {
	    Crtc->FBSet(Crtc, pScrn->displayWidth, pScrn->virtualX, pScrn->virtualY,
			pScrn->depth, rhdPtr->FbScanoutStart);
	    if (Crtc->ScaledToMode) {
		Crtc->ModeSet(Crtc, Crtc->ScaledToMode);
		if (Crtc->ScaleSet)
		    Crtc->ScaleSet(Crtc, Crtc->ScaleType, mode, Crtc->ScaledToMode);
	    } else {
		Crtc->ModeSet(Crtc, mode);
		if (Crtc->ScaleSet)
		    Crtc->ScaleSet(Crtc, RHD_CRTC_SCALE_TYPE_NONE, mode, NULL);
	    }
	    RHDPLLSet(Crtc->PLL, mode->Clock);
	    Crtc->LUTSelect(Crtc, Crtc->LUT);
	    RHDOutputsMode(rhdPtr, Crtc, Crtc->ScaledToMode
			   ? Crtc->ScaledToMode : mode);
	}
    }

    /* shut down that what we don't use */
    RHDPLLsShutdownInactive(rhdPtr);
    RHDOutputsShutdownInactive(rhdPtr);

    if (rhdPtr->Crtc[0]->Active)
	rhdPtr->Crtc[0]->Power(rhdPtr->Crtc[0], RHD_POWER_ON);
    else
	rhdPtr->Crtc[0]->Power(rhdPtr->Crtc[0], RHD_POWER_SHUTDOWN);

    if (rhdPtr->Crtc[1]->Active)
	rhdPtr->Crtc[1]->Power(rhdPtr->Crtc[1], RHD_POWER_ON);
    else
	rhdPtr->Crtc[1]->Power(rhdPtr->Crtc[1], RHD_POWER_SHUTDOWN);

    RHDOutputsPower(rhdPtr, RHD_POWER_ON);
}



static void
RHDAdjustFrame(RHDPtr rhdPtr, int x, int y, int flags)
{
//    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
//    RHDPtr rhdPtr = RHDPTR(pScrn);
    struct rhdCrtc *Crtc;

	Crtc = rhdPtr->Crtc[0];
  if (Crtc->Active)
    Crtc->FrameSet(Crtc, x, y);

	Crtc = rhdPtr->Crtc[1];
  if ( Crtc->Active)
	    Crtc->FrameSet(Crtc, x, y);

}

static Bool
rhdMapFB(RHDPtr rhdPtr)
{
    CARD32 membase;
    RHDFUNC(rhdPtr);

    rhdPtr->FbMapSize = 1 << rhdPtr->pci.size[RHD_FB_BAR];
    membase = rhdPtr->pci.memBase[RHD_FB_BAR];

    rhdPtr->FbBase = MapIoMem(membase, rhdPtr->FbMapSize,PG_SW+PG_NOCACHE);

    if (!rhdPtr->FbBase)
        return FALSE;

    /* These devices have an internal address reference, which some other
     * address registers in there also use. This can be different from the
     * address in the BAR */
    if (rhdPtr->ChipSet < RHD_R600)
      rhdPtr->FbIntAddress = _RHDRegRead(rhdPtr, HDP_FB_LOCATION)<< 16;
    else
      rhdPtr->FbIntAddress = _RHDRegRead(rhdPtr, R6XX_CONFIG_FB_BASE);

    if (rhdPtr->FbIntAddress != membase)
      dbgprintf("PCI FB Address (BAR) is at "
                "0x%08X while card Internal Address is 0x%08X\n",
                (unsigned int) membase,rhdPtr->FbIntAddress);
    dbgprintf("Mapped FB at %p (size 0x%08X)\n",rhdPtr->FbBase, rhdPtr->FbMapSize);
    return TRUE;
}


#define ERR_PARAM  -1

#pragma pack (push,1)
typedef struct
{
  short width;
  short height;
  short bpp;
  short freq;
}mode_t;
#pragma pack (pop)

int get_modes(mode_t *mode, int count)
{
  if(count==0)
    count = SupportedModes;
  else
  {
    DisplayModePtr tmp;
    int i;

    if(count>SupportedModes)
      count = SupportedModes;

    for(i=0,tmp = Scrn.modePool;i<count;i++,tmp=tmp->next,mode++)
    {
      mode->width = tmp->CrtcHDisplay;
      mode->height = tmp->CrtcVDisplay;
      mode->bpp = 32;
      mode->freq = (short)__builtin_ceilf(tmp->VRefresh);
    }
  }
  return count;
}

int set_mode(mode_t *mode)
{
  DisplayModePtr tmp;
  int i;

  for(i=0,tmp = Scrn.modePool;i<SupportedModes;i++,tmp=tmp->next)
  {
    if( (mode->width == tmp->CrtcHDisplay) &&
        (mode->height == tmp->CrtcVDisplay) &&
        (mode->freq ==  (short)__builtin_ceilf(tmp->VRefresh)))
    {
      Scrn.virtualX = mode->width ;
      Scrn.virtualY = mode->height;
      Scrn.displayWidth = mode->width;
      rhdModeInit(&Scrn,tmp);
      sysSetScreen(mode->width,mode->height);
      dbgprintf("set_mode OK\n");
      return 1;
    };
  }
  return 0;
};

#define API_VERSION     0x01000100

#define SRV_GETVERSION  0
#define SRV_ENUM_MODES  1
#define SRV_SET_MODE    2

int _stdcall srv_proc(ioctl_t *io)
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
        *(u32_t*)io->output = API_VERSION;
        return 0;
      }
      break;
    case SRV_ENUM_MODES:
      if(io->inp_size==8)
      {
        int count;
        count = get_modes((mode_t*)(*inp),(int)*(inp+1));

        if(io->out_size==4)
        {
          *outp = count;
          return 0;
        }
      };
      break;
    case SRV_SET_MODE:
      if(io->inp_size==8)
      {
        int err;
        err = set_mode((mode_t*)inp);

        if(io->out_size==4)
        {
          *outp = err;
          return 0;
        }
      };
      break;

  };

  return -1;
}


CARD32
_RHDReadMC(int scrnIndex, CARD32 addr)
{
    RHDPtr rhdPtr = (RHDPtr)scrnIndex;
    CARD32 ret;

    if (rhdPtr->ChipSet < RHD_RS600) {
      _RHDRegWrite(rhdPtr, MC_IND_INDEX, addr);
      ret = _RHDRegRead(rhdPtr, MC_IND_DATA);
    } else if (rhdPtr->ChipSet == RHD_RS600) {
      _RHDRegWrite(rhdPtr, RS60_MC_NB_MC_INDEX, addr);
      ret = _RHDRegRead(rhdPtr, RS60_MC_NB_MC_DATA);
    } else if (rhdPtr->ChipSet == RHD_RS690 || rhdPtr->ChipSet == RHD_RS740) {
	pciWriteLong(rhdPtr->NBPciTag, RS69_MC_INDEX, addr & ~RS69_MC_IND_WR_EN);
	ret = pciReadLong(rhdPtr->NBPciTag, RS69_MC_DATA);
    } else {
	pciWriteLong(rhdPtr->NBPciTag, RS78_NB_MC_IND_INDEX, (addr & ~RS78_MC_IND_WR_EN));
	ret = pciReadLong(rhdPtr->NBPciTag, RS78_NB_MC_IND_DATA);
    }

    RHDDebug(scrnIndex,"%s(0x%08X) = 0x%08X\n",__func__,(unsigned int)addr,
	     (unsigned int)ret);
    return ret;
}

void
_RHDWriteMC(int scrnIndex, CARD32 addr, CARD32 data)
{
    RHDPtr rhdPtr = (RHDPtr)scrnIndex;

    RHDDebug(scrnIndex,"%s(0x%08X, 0x%08X)\n",__func__,(unsigned int)addr,
	     (unsigned int)data);

    if (rhdPtr->ChipSet < RHD_RS600) {
      _RHDRegWrite(rhdPtr, MC_IND_INDEX, addr | MC_IND_WR_EN);
      _RHDRegWrite(rhdPtr, MC_IND_DATA, data);
    } else if (rhdPtr->ChipSet == RHD_RS600) {
      _RHDRegWrite(rhdPtr, RS60_MC_NB_MC_INDEX, addr | RS60_NB_MC_IND_WR_EN);
      _RHDRegWrite(rhdPtr, RS60_MC_NB_MC_DATA, data);
    } else if (rhdPtr->ChipSet == RHD_RS690 || rhdPtr->ChipSet == RHD_RS740) {
      pciWriteLong(rhdPtr->NBPciTag, RS69_MC_INDEX, addr | RS69_MC_IND_WR_EN);
      pciWriteLong(rhdPtr->NBPciTag, RS69_MC_DATA, data);
    } else  {
      pciWriteLong(rhdPtr->NBPciTag, RS78_NB_MC_IND_INDEX, addr | RS78_MC_IND_WR_EN);
      pciWriteLong(rhdPtr->NBPciTag, RS78_NB_MC_IND_DATA, data);
    }
}

