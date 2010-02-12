/*
 * Copyright 2007, 2008  Luc Verhaegen <lverhaegen@novell.com>
 * Copyright 2007, 2008  Matthias Hopf <mhopf@novell.com>
 * Copyright 2007, 2008  Egbert Eich   <eich@novell.com>
 * Copyright 2007, 2008  Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
#define _PARSE_EDID_

#include "common.h"
#include "rhd.h"

#include "edid.h"

#include "xf86DDC.h"

#include "rhd_connector.h"
#include "rhd_modes.h"
#include "rhd_monitor.h"
#ifdef ATOM_BIOS
# include "rhd_atombios.h"
#endif

/* From rhd_edid.c */
void RHDMonitorEDIDSet(struct rhdMonitor *Monitor, xf86MonPtr EDID);


/*
 *
 */

void
RHDMonitorPrint(struct rhdMonitor *Monitor)
{
    int i;

    xf86Msg(X_NONE, "    Bandwidth: %dMHz\n", Monitor->Bandwidth / 1000);
    xf86Msg(X_NONE, "    Horizontal timing:\n");
    for (i = 0; i < Monitor->numHSync; i++)
	xf86Msg(X_NONE, "        %3.1f - %3.1fkHz\n",  Monitor->HSync[i].lo,
		Monitor->HSync[i].hi);
    xf86Msg(X_NONE, "    Vertical timing:\n");
    for (i = 0; i < Monitor->numVRefresh; i++)
	xf86Msg(X_NONE, "        %3.1f - %3.1fHz\n",  Monitor->VRefresh[i].lo,
		Monitor->VRefresh[i].hi);
    xf86Msg(X_NONE, "    DPI: %dx%d\n", Monitor->xDpi, Monitor->yDpi);
    if (Monitor->ReducedAllowed)
	xf86Msg(X_NONE, "    Allows reduced blanking.\n");
    if (Monitor->UseFixedModes)
	xf86Msg(X_NONE, "    Uses Fixed Modes.\n");

    if (!Monitor->Modes)
	xf86Msg(X_NONE, "    No modes are provided.\n");
    else {
      DisplayModePtr Mode;

	xf86Msg(X_NONE, "    Attached modes:\n");
	for (Mode = Monitor->Modes; Mode; Mode = Mode->next) {
	    xf86Msg(X_NONE, "        ");
        RHDPrintModeline(Mode);
      }
    }
}


#if 0

/*
 *
 */
static struct rhdMonitor *
rhdMonitorFromConfig(int scrnIndex, MonPtr Config)
{
    struct rhdMonitor *Monitor;
    DisplayModePtr Mode;
    int i;

    Monitor = xnfcalloc(sizeof(struct rhdMonitor), 1);

    Monitor->Name = xnfstrdup(Config->id);
    Monitor->scrnIndex = scrnIndex;

    if (Config->nHsync) {
        Monitor->numHSync = Config->nHsync;
        for (i = 0; i < Config->nHsync; i++) {
            Monitor->HSync[i].lo = Config->hsync[i].lo;
            Monitor->HSync[i].hi = Config->hsync[i].hi;
        }
    } else if (!Monitor->numHSync) {
        Monitor->numHSync = 3;
        Monitor->HSync[0].lo = 31.5;
        Monitor->HSync[0].hi = 31.5;
        Monitor->HSync[1].lo = 35.15;
        Monitor->HSync[1].hi = 35.15;
        Monitor->HSync[2].lo = 35.5;
        Monitor->HSync[2].hi = 35.5;
    }

    if (Config->nVrefresh) {
        Monitor->numVRefresh = Config->nVrefresh;
        for (i = 0; i < Config->nVrefresh; i++) {
            Monitor->VRefresh[i].lo = Config->vrefresh[i].lo;
            Monitor->VRefresh[i].hi = Config->vrefresh[i].hi;
        }
    } else if (!Monitor->numVRefresh) {
        Monitor->numVRefresh = 1;
        Monitor->VRefresh[0].lo = 50;
        Monitor->VRefresh[0].hi = 61;
    }

#ifdef MONREC_HAS_REDUCED
    if (Config->reducedblanking)
        Monitor->ReducedAllowed = TRUE;
#endif

#ifdef MONREC_HAS_BANDWIDTH
    if (Config->maxPixClock)
        Monitor->Bandwidth = Config->maxPixClock;
#endif

    for (Mode = Config->Modes; Mode; Mode = Mode->next)
	Monitor->Modes = RHDModesAdd(Monitor->Modes, RHDModeCopy(Mode));

    return Monitor;
}
#endif

/*
 *
 */
static struct rhdMonitor *
rhdMonitorFromDefault(RHDPtr rhdPtr)
{
    struct rhdMonitor *Monitor;
    DisplayModePtr Mode;

    Monitor = xnfcalloc(sizeof(struct rhdMonitor), 1);

    Monitor->Name = strdup("Default (SVGA)");
    Monitor->scrnIndex = rhdPtr->scrnIndex;

    /* timing for pathetic 14" svga monitors */
    Monitor->numHSync = 3;
    Monitor->HSync[0].lo = 31.5;
    Monitor->HSync[0].hi = 31.5;
    Monitor->HSync[1].lo = 35.15;
    Monitor->HSync[1].hi = 35.15;
    Monitor->HSync[2].lo = 35.5;
    Monitor->HSync[2].hi = 35.5;

    Monitor->numVRefresh = 1;
    Monitor->VRefresh[0].lo = 50;
    Monitor->VRefresh[0].hi = 61;

    return Monitor;
}

/*
 * This function tries to handle a configured monitor correctly.
 *
 * This either can be forced through the option, or is used when
 * no monitors are autodetected.
 */
void
RHDConfigMonitorSet(RHDPtr rhdPtr, Bool UseConfig)
{
  int i;

  for (i = 0; i < RHD_CONNECTORS_MAX; i++)
    if (rhdPtr->Connector[i] && rhdPtr->Connector[i]->Monitor)
	    break;

  if (i == RHD_CONNECTORS_MAX)
	xf86DrvMsg(scrnIndex, X_INFO, "No monitors autodetected; "
		   "attempting to work around this.\n");

  if (i == RHD_CONNECTORS_MAX)
  {
    rhdPtr->ConfigMonitor = rhdMonitorFromDefault(rhdPtr);

    DBG(dbgprintf("Created monitor from default: \"%s\":\n",
               rhdPtr->ConfigMonitor->Name));

    RHDMonitorPrint(rhdPtr->ConfigMonitor);
  };
}

/*
 * Make sure that we keep only a single mode in our list. This mode should
 * hopefully match our panel at native resolution correctly.
 */
static void
rhdPanelEDIDModesFilter(struct rhdMonitor *Monitor)
{
  DisplayModeRec *Best = Monitor->Modes, *Mode, *Temp;

  RHDFUNC(Monitor);

  if (!Best || !Best->next)
    return; /* don't bother */

    /* don't go for preferred, just take the biggest */
    for (Mode = Best->next; Mode; Mode = Mode->next) {
    if (((Best->HDisplay <= Mode->HDisplay) &&
         (Best->VDisplay < Mode->VDisplay)) ||
        ((Best->HDisplay < Mode->HDisplay) &&
         (Best->VDisplay <= Mode->VDisplay)))
      Best = Mode;
  }

    xf86DrvMsg(Monitor->scrnIndex, X_INFO, "Monitor \"%s\": Using Mode \"%s\""
	       " for native resolution.\n", Monitor->Name, Best->name);

    /* kill all other modes */
  Mode = Monitor->Modes;
    while (Mode) {
    Temp = Mode->next;

	if (Mode != Best) {
	    RHDDebug(Monitor->scrnIndex, "Monitor \"%s\": Discarding Mode \"%s\"\n",
		     Monitor->Name, Mode->name);

	    xfree(Mode->name);
	    xfree(Mode);
    }
    Mode = Temp;
  }

  Best->next = NULL;
  Best->prev = NULL;
  Best->type |= M_T_PREFERRED;
    Monitor->NativeMode = Best;
    Monitor->Modes = Monitor->NativeMode;
  Monitor->numHSync = 1;
  Monitor->HSync[0].lo = Best->HSync;
  Monitor->HSync[0].hi = Best->HSync;
  Monitor->numVRefresh = 1;
  Monitor->VRefresh[0].lo = Best->VRefresh;
  Monitor->VRefresh[0].hi = Best->VRefresh;
  Monitor->Bandwidth = Best->Clock;
}

/*
 *
 */
void
rhdMonitorPrintEDID(struct rhdMonitor *Monitor, xf86MonPtr EDID)
{
    xf86DrvMsg(EDID->scrnIndex, X_INFO, "EDID data for %s\n",
	       Monitor->Name);
  xf86PrintEDID(EDID);
}

/*
 * Panels are the most complicated case we need to handle here.
 * Information can come from several places, and we need to make sure
 * that we end up with only the native resolution in our table.
 */
static struct rhdMonitor *
rhdMonitorPanel(struct rhdConnector *Connector)
{
    struct rhdMonitor *Monitor;
    DisplayModeRec *Mode = NULL;
    xf86MonPtr EDID = NULL;

    RHDFUNC(Connector);

    /* has priority over AtomBIOS EDID */
    if (Connector->DDC)
    EDID = xf86DoEDID_DDC2(Connector->scrnIndex, Connector->DDC);

#ifdef ATOM_BIOS
    {
  RHDPtr rhdPtr = (RHDPtr)Connector->scrnIndex;
	AtomBiosArgRec data;
	AtomBiosResult Result;

  Result = RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
				 ATOMBIOS_GET_PANEL_MODE, &data);
	if (Result == ATOM_SUCCESS) {
	    Mode = data.mode;
	    Mode->type |= M_T_PREFERRED;
	}
	if (!EDID) {
      Result = RHDAtomBiosFunc(rhdPtr,rhdPtr->atomBIOS,
				     ATOMBIOS_GET_PANEL_EDID, &data);
	    if (Result == ATOM_SUCCESS)
    EDID = xf86InterpretEDID(rhdPtr, data.EDIDBlock);
	}
    }
#endif

    Monitor = xnfcalloc(sizeof(struct rhdMonitor), 1);

    Monitor->scrnIndex = Connector->scrnIndex;
    Monitor->EDID      = EDID;

    if (Mode) {
	Monitor->Name = xstrdup("LVDS Panel");
    Monitor->Modes = RHDModesAdd(Monitor->Modes, Mode);
	Monitor->NativeMode = Mode;
    Monitor->numHSync = 1;
    Monitor->HSync[0].lo = Mode->HSync;
    Monitor->HSync[0].hi = Mode->HSync;
    Monitor->numVRefresh = 1;
    Monitor->VRefresh[0].lo = Mode->VRefresh;
    Monitor->VRefresh[0].hi = Mode->VRefresh;
    Monitor->Bandwidth = Mode->SynthClock;

	/* Clueless atombios does give us a mode, but doesn't give us a
	 * DPI or a size. It is just perfect, right? */
	if (EDID) {
      if (EDID->features.hsize)
        Monitor->xDpi = (Mode->HDisplay * 2.54) / ((float) EDID->features.hsize) + 0.5;
      if (EDID->features.vsize)
        Monitor->yDpi = (Mode->VDisplay * 2.54) / ((float) EDID->features.vsize) + 0.5;
    }
    } else if (EDID) {
      RHDMonitorEDIDSet(Monitor, EDID);
      rhdPanelEDIDModesFilter(Monitor);
    } else {
	xf86DrvMsg(Connector->scrnIndex, X_ERROR,
		   "%s: No panel mode information found.\n", __func__);
	xfree(Monitor);
      return NULL;
    }

    /* panel should be driven at native resolution only. */
  Monitor->UseFixedModes = TRUE;
  Monitor->ReducedAllowed = TRUE;

  if (EDID)
    rhdMonitorPrintEDID(Monitor, EDID);

  return Monitor;
}

/*
 * rhdMonitorTV(): get TV modes. Currently we can only get this from AtomBIOS.
 */
static struct rhdMonitor *
rhdMonitorTV(struct rhdConnector *Connector)
{
    struct rhdMonitor *Monitor = NULL;
#ifdef ATOM_BIOS
    RHDPtr rhdPtr = RHDPTRI(Connector);
    DisplayModeRec *Mode = NULL;
    AtomBiosArgRec arg;

    RHDFUNC(Connector);

    arg.tvMode = rhdPtr->tvMode;
    if (RHDAtomBiosFunc(Connector->scrnIndex, rhdPtr->atomBIOS,
			ATOM_ANALOG_TV_MODE, &arg)
	!= ATOM_SUCCESS)
	return NULL;

    Mode = arg.mode;
    Mode->type |= M_T_PREFERRED;

    Monitor = xnfcalloc(sizeof(struct rhdMonitor), 1);

    Monitor->scrnIndex = Connector->scrnIndex;
    Monitor->EDID      = NULL;

    Monitor->Name      = xstrdup("TV");
    Monitor->Modes     = RHDModesAdd(Monitor->Modes, Mode);
    Monitor->NativeMode= Mode;
    Monitor->numHSync  = 1;
    Monitor->HSync[0].lo = Mode->HSync;
    Monitor->HSync[0].hi = Mode->HSync;
    Monitor->numVRefresh = 1;
    Monitor->VRefresh[0].lo = Mode->VRefresh;
    Monitor->VRefresh[0].hi = Mode->VRefresh;
    Monitor->Bandwidth = Mode->SynthClock;

    /* TV should be driven at native resolution only. */
    Monitor->UseFixedModes = TRUE;
    Monitor->ReducedAllowed = FALSE;
    /*
     *  hack: the TV encoder takes care of that.
     *  The mode that goes in isn't what comes out.
     */
    Mode->Flags &= ~(V_INTERLACE);
#endif
    return Monitor;
}

/*
 *
 */
struct rhdMonitor *
RHDMonitorInit(struct rhdConnector *Connector)
{
  struct rhdMonitor *Monitor = NULL;

  RHDFUNC(Connector);

  if (Connector->Type == RHD_CONNECTOR_PANEL)
    Monitor = rhdMonitorPanel(Connector);
    else if (Connector->Type == RHD_CONNECTOR_TV)
	Monitor = rhdMonitorTV(Connector);
    else if (Connector->DDC) {
    xf86MonPtr EDID = xf86DoEDID_DDC2(Connector->scrnIndex, Connector->DDC);
	if (EDID) {
	    Monitor = xnfcalloc(sizeof(struct rhdMonitor), 1);
        Monitor->scrnIndex = Connector->scrnIndex;
        Monitor->EDID   = EDID;
	    Monitor->NativeMode = NULL;

        RHDMonitorEDIDSet(Monitor, EDID);
        rhdMonitorPrintEDID(Monitor, EDID);
      }
    }
  return Monitor;
}

/*
 *
 */
void
RHDMonitorDestroy(struct rhdMonitor *Monitor)
{
    DisplayModePtr Mode, Next;

    for (Mode = Monitor->Modes; Mode;) {
	Next = Mode->next;

	xfree(Mode->name);
	xfree(Mode);

	Mode = Next;
    }

    if (Monitor->EDID)
	xfree(Monitor->EDID->rawData);
    xfree(Monitor->EDID);
    xfree(Monitor->Name);
    xfree(Monitor);
}


static unsigned char * VDIFRead(RHDPtr rhdPtr, I2CBusPtr pBus, int start);

#define RETRIES 4

static xf86VdifLimitsPtr* get_limits(CARD8 *c);
static xf86VdifGammaPtr* get_gamma(CARD8 *c);
static xf86VdifTimingPtr* get_timings(CARD8 *c);

xf86vdifPtr xf86InterpretVdif(CARD8 *c)
{
  xf86VdifPtr p = (xf86VdifPtr)c;
  xf86vdifPtr vdif;
  int i;

  unsigned long l = 0;

  if (c == NULL) return NULL;
  if (p->VDIFId[0] != 'V' || p->VDIFId[1] != 'D' || p->VDIFId[2] != 'I'
      || p->VDIFId[3] != 'F') return NULL;
  for ( i = 12; i < p->FileLength; i++)
    l += c[i];
  if ( l != p->Checksum) return NULL;
  vdif = malloc(sizeof(xf86vdif));
  vdif->vdif = p;
  vdif->limits = get_limits(c);
  vdif->timings = get_timings(c);
  vdif->gamma = get_gamma(c);
  vdif->strings = VDIF_STRING(((xf86VdifPtr)c),0);
  free(c);
  return vdif;
}

static xf86VdifLimitsPtr*
get_limits(CARD8 *c)
{
    int num, i, j;
    xf86VdifLimitsPtr *pp;
    xf86VdifLimitsPtr p;

    num = ((xf86VdifPtr)c)->NumberOperationalLimits;
    pp = malloc(sizeof(xf86VdifLimitsPtr) * (num+1));
    p = VDIF_OPERATIONAL_LIMITS(((xf86VdifPtr)c));
    j = 0;
    for ( i = 0; i<num; i++) {
	if (p->Header.ScnTag == VDIF_OPERATIONAL_LIMITS_TAG)
	    pp[j++] = p;
	VDIF_NEXT_OPERATIONAL_LIMITS(p);
    }
    pp[j] = NULL;
    return pp;
}

static xf86VdifGammaPtr*
get_gamma(CARD8 *c)
{
    int num, i, j;
    xf86VdifGammaPtr *pp;
    xf86VdifGammaPtr p;

    num = ((xf86VdifPtr)c)->NumberOptions;
    pp = malloc(sizeof(xf86VdifGammaPtr) * (num+1));
    p = (xf86VdifGammaPtr)VDIF_OPTIONS(((xf86VdifPtr)c));
    j = 0;
    for ( i = 0; i<num; i++)
    {
	if (p->Header.ScnTag == VDIF_GAMMA_TABLE_TAG)
	    pp[j++] = p;
	VDIF_NEXT_OPTIONS(p);
    }
    pp[j] = NULL;
    return pp;
}

static xf86VdifTimingPtr*
get_timings(CARD8 *c)
{
    int num, num_limits;
    int i,j,k;
    xf86VdifLimitsPtr lp;
    xf86VdifTimingPtr *pp;
    xf86VdifTimingPtr p;

    num = ((xf86VdifPtr)c)->NumberOperationalLimits;
    lp = VDIF_OPERATIONAL_LIMITS(((xf86VdifPtr)c));
    num_limits = 0;
    for (i = 0; i < num; i++) {
	if (lp->Header.ScnTag == VDIF_OPERATIONAL_LIMITS_TAG)
	    num_limits += lp->NumberPreadjustedTimings;
	VDIF_NEXT_OPERATIONAL_LIMITS(lp);
    }
    pp = malloc(sizeof(xf86VdifTimingPtr)
				      * (num_limits+1));
    j = 0;
    lp = VDIF_OPERATIONAL_LIMITS(((xf86VdifPtr) c));
    for (i = 0; i < num; i++) {
	p = VDIF_PREADJUSTED_TIMING(lp);
	for (k = 0; k < lp->NumberPreadjustedTimings; k++) {
	    if (p->Header.ScnTag == VDIF_PREADJUSTED_TIMING_TAG)
		pp[j++] = p;
	    VDIF_NEXT_PREADJUSTED_TIMING(p);
	}
	VDIF_NEXT_OPERATIONAL_LIMITS(lp);
    }
    pp[j] = NULL;
    return pp;
}

int DDC_checksum(unsigned char *block, int len)
{
  int i, result = 0;
  int not_null = 0;

  for (i=0;i<len;i++)
  {
    not_null |= block[i];
    result += block[i];
  }

  if (result & 0xFF) DBG(dbgprintf("DDC checksum not correct\n"));
  if (!not_null) DBG(dbgprintf("DDC read all Null\n"));

    /* catch the trivial case where all bytes are 0 */
  if (!not_null) return 1;

  return (result&0xFF);
}

static unsigned char *
DDCRead_DDC2(RHDPtr rhdPtr, I2CBusPtr pBus, int start, int len)
{
  I2CDevPtr dev;
  unsigned char W_Buffer[2];
  int w_bytes;
  unsigned char *R_Buffer;
  int i;

  RHDFUNC(rhdPtr);

//    xf86LoaderReqSymLists(i2cSymbols, NULL);

  if (!(dev = xf86I2CFindDev(pBus, 0x00A0)))
  {
    dev = xf86CreateI2CDevRec();
    dev->DevName = "ddc2";
    dev->SlaveAddr = 0xA0;
    dev->ByteTimeout = 2200; /* VESA DDC spec 3 p. 43 (+10 %) */
    dev->StartTimeout = 550;
    dev->BitTimeout = 40;
    dev->ByteTimeout = 40;
    dev->AcknTimeout = 40;

    dev->pI2CBus = pBus;
    if (!xf86I2CDevInit(dev))
    {
      DBG(dbgprintf("No DDC2 device\n"));
	    return NULL;
    }
  }
  if (start < 0x100)
  {
    w_bytes = 1;
    W_Buffer[0] = start;
  }
  else
  {
    w_bytes = 2;
    W_Buffer[0] = start & 0xFF;
    W_Buffer[1] = (start & 0xFF00) >> 8;
  }

  R_Buffer = calloc(1,sizeof(unsigned char)* (len));

  if( !R_Buffer)
  {
    DBG(dbgprintf("R_Buffer = NULL\n"));
    return NULL;
  };

  for (i=0; i<RETRIES; i++)
  {
    if (xf86I2CWriteRead(dev, W_Buffer,w_bytes, R_Buffer,len))
    {
	    if (!DDC_checksum(R_Buffer,len))
        return R_Buffer;
      else
        DBG(dbgprintf("Checksum error in EDID block\n"));
    }
    else
      DBG(dbgprintf("Error reading EDID block\n"));
  }
  xf86DestroyI2CDevRec(dev,TRUE);
  free(R_Buffer);
  return NULL;
}

static unsigned char*
EDID1Read_DDC2(RHDPtr rhdPtr, I2CBusPtr pBus)
{
    return  DDCRead_DDC2(rhdPtr, pBus, 0, EDID1_LEN);
}

xf86MonPtr
xf86DoEDID_DDC2(RHDPtr rhdPtr, I2CBusPtr pBus)
{
  unsigned char *EDID_block = NULL;
  unsigned char *VDIF_Block = NULL;
  xf86MonPtr tmp = NULL;

  RHDFUNC(rhdPtr);

  EDID_block = EDID1Read_DDC2(rhdPtr,pBus);

  if (EDID_block)
  {
    tmp = xf86InterpretEDID(rhdPtr,EDID_block);
  }
  else
  {
    DBG(dbgprintf("No EDID block returned\n"));
    return NULL;
  }

  if (!tmp)
  {
    DBG(dbgprintf("Cannot interpret EDID block\n"));
    return tmp;
  }
  DBG(dbgprintf("Sections to follow: %d\n",tmp->no_sections));

  VDIF_Block =
  VDIFRead(rhdPtr, pBus, EDID1_LEN * (tmp->no_sections + 1));
  tmp->vdif = xf86InterpretVdif(VDIF_Block);

  return tmp;
}

static unsigned char*
VDIFRead(RHDPtr rhdPtr, I2CBusPtr pBus, int start)
{
  unsigned char * Buffer, *v_buffer = NULL, *v_bufferp = NULL;
  int i, num = 0;

    /* read VDIF length in 64 byte blocks */
  Buffer = DDCRead_DDC2(rhdPtr, pBus,start,64);
  if (Buffer == NULL)
    return NULL;

  DBG(dbgprintf("number of 64 bit blocks: %i\n",Buffer[0]));

  if ((num = Buffer[0]) > 0)
    v_buffer = v_bufferp = malloc(sizeof(unsigned char) * 64 * num);

  for (i = 0; i < num; i++)
  {
    Buffer = DDCRead_DDC2(rhdPtr, pBus,start,64);
    if (Buffer == NULL)
    {
      free (v_buffer);
	    return NULL;
    }
    memcpy(v_bufferp,Buffer,63); /* 64th byte is checksum */
    free(Buffer);
    v_bufferp += 63;
  }
  return v_buffer;
}



static void print_vendor(RHDPtr rhdPtr, struct vendor *);
static void print_version(RHDPtr rhdPtr, struct edid_version *);
static void print_display(RHDPtr rhdPtr, struct disp_features *,
			  struct edid_version *);
static void print_established_timings(RHDPtr rhdPtr,
				      struct established_timings *);
static void print_std_timings(RHDPtr rhdPtr, struct std_timings *);
static void print_detailed_monitor_section(RHDPtr rhdPtr,
					   struct detailed_monitor_section *);
static void print_detailed_timings(RHDPtr rhdPtr, struct detailed_timings *);

static void print_input_features(RHDPtr rhdPtr, struct disp_features *);
static void print_dpms_features(RHDPtr rhdPtr, struct disp_features *,
				struct edid_version *v);
static void print_whitepoint(RHDPtr rhdPtr, struct disp_features *);
static void print_number_sections(RHDPtr rhdPtr, int);

xf86MonPtr
xf86PrintEDID(xf86MonPtr m)
{
    if (!(m)) return NULL;
    print_vendor(m->rhdPtr,&m->vendor);
    print_version(m->rhdPtr,&m->ver);
    print_display(m->rhdPtr,&m->features, &m->ver);
    print_established_timings(m->rhdPtr,&m->timings1);
    print_std_timings(m->rhdPtr,m->timings2);
    print_detailed_monitor_section(m->rhdPtr,m->det_mon);
    print_number_sections(m->rhdPtr,m->no_sections);

    return m;
}

static void
print_vendor(RHDPtr rhdPtr, struct vendor *c)
{
  DBG(dbgprintf("Manufacturer: %s  Model: %x  Serial#: %u\n",
            (char *)&c->name, c->prod_id, c->serial));
  DBG(dbgprintf("Year: %u  Week: %u\n", c->year, c->week));
}

static void
print_version(RHDPtr rhdPtr, struct edid_version *c)
{
  DBG(dbgprintf("EDID Version: %u.%u\n",c->version,c->revision));
}

static void
print_display(RHDPtr rhdPtr, struct disp_features *disp,
	      struct edid_version *version)
{
    print_input_features(rhdPtr,disp);
    DBG(dbgprintf("Max H-Image Size [cm]: "));
    if (disp->hsize)
      DBG(dbgprintf("horiz.: %i  ",disp->hsize));
    else
      DBG(dbgprintf("H-Size may change,  "));
    if (disp->vsize)
      DBG(dbgprintf("vert.: %i\n",disp->vsize));
    else
      DBG(dbgprintf("V-Size may change\n"));
    DBG(dbgprintf("Gamma: %.2f\n", (double)disp->gamma));
    print_dpms_features(rhdPtr,disp,version);
    print_whitepoint(rhdPtr,disp);
}

static void
print_input_features(RHDPtr rhdPtr, struct disp_features *c)
{
    if (DIGITAL(c->input_type))
    {
      DBG(dbgprintf("Digital Display Input\n"));
      if (DFP1(c->input_dfp))
        DBG(dbgprintf("DFP 1.x compatible TMDS\n"));
    }
    else
    {
      DBG(dbgprintf("Analog Display Input,  "));
      DBG(dbgprintf("Input Voltage Level: "));
      switch (c->input_voltage)
      {
        case V070:
          DBG(dbgprintf("0.700/0.300 V\n"));
          break;
        case V071:
          DBG(dbgprintf("0.714/0.286 V\n"));
          break;
        case V100:
          DBG(dbgprintf("1.000/0.400 V\n"));
          break;
        case V007:
          DBG(dbgprintf("0.700/0.700 V\n"));
          break;
        default:
          DBG(dbgprintf("undefined\n"));
      }
      if (SIG_SETUP(c->input_setup))
        DBG(dbgprintf("Signal levels configurable\n"));
      DBG(dbgprintf("Sync:"));
      if (SEP_SYNC(c->input_sync))
        DBG(dbgprintf("  Separate"));
      if (COMP_SYNC(c->input_sync))
        DBG(dbgprintf("  Composite"));
      if (SYNC_O_GREEN(c->input_sync))
        DBG(dbgprintf("  SyncOnGreen"));
      if (SYNC_SERR(c->input_sync))
        DBG(dbgprintf("Serration on. "
         "V.Sync Pulse req. if CompSync or SyncOnGreen\n"));
      else
        DBG(dbgprintf("\n"));
    }
}

static void
print_dpms_features(RHDPtr rhdPtr, struct disp_features *c,
		    struct edid_version *v)
{
   if (c->dpms)
   {
     DBG(dbgprintf("DPMS capabilities:"));
     if (DPMS_STANDBY(c->dpms))
       DBG(dbgprintf(" StandBy"));
     if (DPMS_SUSPEND(c->dpms))
       DBG(dbgprintf(" Suspend"));
     if (DPMS_OFF(c->dpms))
       DBG(dbgprintf(" Off"));
   }
   else
     DBG(dbgprintf("No DPMS capabilities specified"));
   switch (c->display_type)
   {
     case DISP_MONO:
       DBG(dbgprintf("; Monochorome/GrayScale Display\n"));
       break;
     case DISP_RGB:
       DBG(dbgprintf("; RGB/Color Display\n"));
       break;
     case DISP_MULTCOLOR:
       DBG(dbgprintf("; Non RGB Multicolor Display\n"));
       break;
     default:
       DBG(dbgprintf("\n"));
       break;
   }
   if (STD_COLOR_SPACE(c->msc))
     DBG(dbgprintf("Default color space is primary color space\n"));
   if (PREFERRED_TIMING_MODE(c->msc))
     DBG(dbgprintf("First detailed timing is preferred mode\n"));
   else
     if (v->version == 1 && v->revision >= 3)
       DBG(dbgprintf("First detailed timing not preferred "
                 "mode in violation of standard!"));
   if (GFT_SUPPORTED(c->msc))
     DBG(dbgprintf("GTF timings supported\n"));
}

static void
print_whitepoint(RHDPtr rhdPtr, struct disp_features *disp)
{
    DBG(dbgprintf("redX: %.3f redY: %.3f   ",
         (double)disp->redx,(double)disp->redy));
    DBG(dbgprintf("greenX: %.3f greenY: %.3f\n",
         (double)disp->greenx,(double)disp->greeny));
    DBG(dbgprintf("blueX: %.3f blueY: %.3f   ",
         (double)disp->bluex,(double)disp->bluey));
    DBG(dbgprintf("whiteX: %.3f whiteY: %.3f\n",
         (double)disp->whitex,(double)disp->whitey));
}

static void
print_established_timings(RHDPtr rhdPtr, struct established_timings *t)
{
    unsigned char c;

    if (t->t1 || t->t2 || t->t_manu)
      DBG(dbgprintf("Supported VESA Video Modes:\n"));
    c=t->t1;
    if (c&0x80) DBG(dbgprintf("720x400@70Hz\n"));
    if (c&0x40) DBG(dbgprintf("720x400@88Hz\n"));
    if (c&0x20) DBG(dbgprintf("640x480@60Hz\n"));
    if (c&0x10) DBG(dbgprintf("640x480@67Hz\n"));
    if (c&0x08) DBG(dbgprintf("640x480@72Hz\n"));
    if (c&0x04) DBG(dbgprintf("640x480@75Hz\n"));
    if (c&0x02) DBG(dbgprintf("800x600@56Hz\n"));
    if (c&0x01) DBG(dbgprintf("800x600@60Hz\n"));
    c=t->t2;
    if (c&0x80) DBG(dbgprintf("800x600@72Hz\n"));
    if (c&0x40) DBG(dbgprintf("800x600@75Hz\n"));
    if (c&0x20) DBG(dbgprintf("832x624@75Hz\n"));
    if (c&0x10) DBG(dbgprintf("1024x768@87Hz (interlaced)\n"));
    if (c&0x08) DBG(dbgprintf("1024x768@60Hz\n"));
    if (c&0x04) DBG(dbgprintf("1024x768@70Hz\n"));
    if (c&0x02) DBG(dbgprintf("1024x768@75Hz\n"));
    if (c&0x01) DBG(dbgprintf("1280x1024@75Hz\n"));
    c=t->t_manu;
    if (c&0x80) DBG(dbgprintf("1152x870@75Hz\n"));
    DBG(dbgprintf("Manufacturer's mask: %X\n",c&0x7F));
}

static void
print_std_timings(RHDPtr rhdPtr, struct std_timings *t)
{
    int i;
    char done = 0;
    for (i=0;i<STD_TIMINGS;i++)
    {
      if (t[i].hsize > 256)   /* sanity check */
      {
        if (!done)
        {
          DBG(dbgprintf("Supported Future Video Modes:\n"));
          done = 1;
        }
        DBG(dbgprintf("#%d: hsize: %i  vsize %i  refresh: %i  vid: %i\n",
                      i, t[i].hsize, t[i].vsize, t[i].refresh, t[i].id));
      }
    }
}

static void
print_detailed_monitor_section(RHDPtr rhdPtr,
			       struct detailed_monitor_section *m)
{
  int i,j;

  for (i=0;i<DET_TIMINGS;i++)
  {
    switch (m[i].type)
    {
      case DT:
        print_detailed_timings(rhdPtr,&m[i].section.d_timings);
        break;
      case DS_SERIAL:
        DBG(dbgprintf("Serial No: %s\n",m[i].section.serial));
        break;
      case DS_ASCII_STR:
        DBG(dbgprintf(" %s\n",m[i].section.ascii_data));
        break;
      case DS_NAME:
        DBG(dbgprintf("Monitor name: %s\n",m[i].section.name));
        break;
      case DS_RANGES:
        DBG(dbgprintf("Ranges: V min: %i  V max: %i Hz, H min: %i  H max: %i kHz,",
                   m[i].section.ranges.min_v, m[i].section.ranges.max_v,
                   m[i].section.ranges.min_h, m[i].section.ranges.max_h));
        if (m[i].section.ranges.max_clock != 0)
          DBG(dbgprintf(" PixClock max %i MHz\n",m[i].section.ranges.max_clock));
        else
          DBG(dbgprintf("\n"));
        if (m[i].section.ranges.gtf_2nd_f > 0)
          DBG(dbgprintf(" 2nd GTF parameters: f: %i kHz "
                    "c: %i m: %i k %i j %i\n",
                    m[i].section.ranges.gtf_2nd_f,
                    m[i].section.ranges.gtf_2nd_c,
                    m[i].section.ranges.gtf_2nd_m,
                    m[i].section.ranges.gtf_2nd_k,
                    m[i].section.ranges.gtf_2nd_j));
          break;
      case DS_STD_TIMINGS:
        for (j = 0; j<5; j++)
          DBG(dbgprintf("#%i: hsize: %i  vsize %i  refresh: %i  "
                    "vid: %i\n",i,m[i].section.std_t[i].hsize,
                    m[i].section.std_t[j].vsize,m[i].section.std_t[j].refresh,
                    m[i].section.std_t[j].id));
          break;
      case DS_WHITE_P:
        for (j = 0; j<2; j++)
          if (m[i].section.wp[j].index != 0)
            DBG(dbgprintf("White point %i: whiteX: %f, whiteY: %f; gamma: %f\n",
                      m[i].section.wp[j].index,(double)m[i].section.wp[j].white_x,
                      (double)m[i].section.wp[j].white_y,
                      (double)m[i].section.wp[j].white_gamma));
            break;
      case DS_DUMMY:
      default:
        break;
    }
  }
}

static void
print_detailed_timings(RHDPtr rhdPtr, struct detailed_timings *t)
{

    if (t->clock > 15000000)  /* sanity check */
    {
      DBG(dbgprintf("Supported additional Video Mode:\n"));
      DBG(dbgprintf("clock: %.1f MHz   ",(double)t->clock/1000000.0));
      DBG(dbgprintf("Image Size:  %i x %i mm\n",t->h_size,t->v_size));
      DBG(dbgprintf("h_active: %i  h_sync: %i  h_sync_end %i h_blank_end %i ",
                    t->h_active, t->h_sync_off + t->h_active,
                    t->h_sync_off + t->h_sync_width + t->h_active,
                    t->h_active + t->h_blanking));
      DBG(dbgprintf("h_border: %i\n",t->h_border));
      DBG(dbgprintf("v_active: %i  v_sync: %i  v_sync_end %i v_blanking: %i ",
                    t->v_active, t->v_sync_off + t->v_active,
                    t->v_sync_off + t->v_sync_width + t->v_active,
                    t->v_active + t->v_blanking));
      DBG(dbgprintf("v_border: %i\n",t->v_border));
      if (IS_STEREO(t->stereo))
      {
        DBG(dbgprintf("Stereo: "));
        if (IS_RIGHT_STEREO(t->stereo))
        {
          if (!t->stereo_1)
            DBG(dbgprintf("right channel on sync\n"));
          else
            DBG(dbgprintf("left channel on sync\n"));
        }
        else
          if (IS_LEFT_STEREO(t->stereo))
          {
            if (!t->stereo_1)
              DBG(dbgprintf("right channel on even line\n"));
            else
              DBG(dbgprintf("left channel on evel line\n"));
          }
        if (IS_4WAY_STEREO(t->stereo))
        {
          if (!t->stereo_1)
            DBG(dbgprintf("4-way interleaved\n"));
          else
            DBG(dbgprintf("side-by-side interleaved"));
        }
      }
    }
}

static void
print_number_sections(RHDPtr rhdPtr, int num)
{
  if (num)
    DBG(dbgprintf("Number of EDID sections to follow: %i\n",num));
}



static void get_vendor_section(Uchar*, struct vendor *);
static void get_version_section(Uchar*, struct edid_version *);
static void get_display_section(Uchar*, struct disp_features *,
				struct edid_version *);
static void get_established_timing_section(Uchar*, struct established_timings *);
static void get_std_timing_section(Uchar*, struct std_timings *,
				   struct edid_version *);
static void get_dt_md_section(Uchar *, struct edid_version *,
			      struct detailed_monitor_section *det_mon);
static void copy_string(Uchar *, Uchar *);
static void get_dst_timing_section(Uchar *, struct std_timings *,
				   struct edid_version *);
static void get_monitor_ranges(Uchar *, struct monitor_ranges *);
static void get_whitepoint_section(Uchar *, struct whitePoints *);
static void get_detailed_timing_section(Uchar*, struct 	detailed_timings *);
static Bool validate_version(RHDPtr rhdPtr, struct edid_version *);


xf86MonPtr
xf86InterpretEDID(int scrnIndex, Uchar *block)
{
    xf86MonPtr m;
    RHDPtr rhdPtr = (RHDPtr)scrnIndex;

    if (!block) return NULL;
    if (! (m = calloc(sizeof(xf86Monitor),1))) return NULL;
    m->rhdPtr = rhdPtr;
    m->rawData = block;

    get_vendor_section(SECTION(VENDOR_SECTION,block),&m->vendor);
    get_version_section(SECTION(VERSION_SECTION,block),&m->ver);
    if (!validate_version(rhdPtr, &m->ver)) goto error;
    get_display_section(SECTION(DISPLAY_SECTION,block),&m->features,
			&m->ver);
    get_established_timing_section(SECTION(ESTABLISHED_TIMING_SECTION,block),
				   &m->timings1);
    get_std_timing_section(SECTION(STD_TIMING_SECTION,block),m->timings2,
			   &m->ver);
    get_dt_md_section(SECTION(DET_TIMING_SECTION,block),&m->ver, m->det_mon);
    m->no_sections = (int)*(char *)SECTION(NO_EDID,block);

    return (m);

 error:
    free(m);
    return NULL;
}

static void
get_vendor_section(Uchar *c, struct vendor *r)
{
    r->name[0] = L1;
    r->name[1] = L2;
    r->name[2] = L3;
    r->name[3] = '\0';

    r->prod_id = PROD_ID;
    r->serial  = SERIAL_NO;
    r->week    = WEEK;
    r->year    = YEAR;
}

static void
get_version_section(Uchar *c, struct edid_version *r)
{
    r->version  = VERSION;
    r->revision = REVISION;
}

static void
get_display_section(Uchar *c, struct disp_features *r,
		    struct edid_version *v)
{
    r->input_type = INPUT_TYPE;
    if (!DIGITAL(r->input_type))
    {
      r->input_voltage = INPUT_VOLTAGE;
      r->input_setup = SETUP;
      r->input_sync = SYNC;
    }
    else
      if (v->version > 1 || v->revision > 2)
        r->input_dfp = DFP;
    r->hsize = HSIZE_MAX;
    r->vsize = VSIZE_MAX;
    r->gamma = GAMMA;
    r->dpms =  DPMS;
    r->display_type = DISPLAY_TYPE;
    r->msc = MSC;
    r->redx = REDX;
    r->redy = REDY;
    r->greenx = GREENX;
    r->greeny = GREENY;
    r->bluex = BLUEX;
    r->bluey = BLUEY;
    r->whitex = WHITEX;
    r->whitey = WHITEY;
}

static void
get_established_timing_section(Uchar *c, struct established_timings *r)
{
    r->t1 = T1;
    r->t2 = T2;
    r->t_manu = T_MANU;
}

static void
get_std_timing_section(Uchar *c, struct std_timings *r,
		       struct edid_version *v)
{
    int i;

    for (i=0;i<STD_TIMINGS;i++)
    {
      if (VALID_TIMING)
      {
        r[i].hsize = HSIZE1;
        VSIZE1(r[i].vsize);
        r[i].refresh = REFRESH_R;
        r[i].id = STD_TIMING_ID;
      }
      else
      {
        r[i].hsize = r[i].vsize = r[i].refresh = r[i].id = 0;
       }
       NEXT_STD_TIMING;
    }
}

static void
get_dt_md_section(Uchar *c, struct edid_version *ver,
		  struct detailed_monitor_section *det_mon)
{
  int i;

  for (i=0;i<DET_TIMINGS;i++) {
    if (ver->version == 1 && ver->revision >= 1 && IS_MONITOR_DESC) {

      switch (MONITOR_DESC_TYPE) {
      case SERIAL_NUMBER:
	det_mon[i].type = DS_SERIAL;
	copy_string(c,det_mon[i].section.serial);
	break;
      case ASCII_STR:
	det_mon[i].type = DS_ASCII_STR;
	copy_string(c,det_mon[i].section.ascii_data);
	break;
      case MONITOR_RANGES:
	det_mon[i].type = DS_RANGES;
	get_monitor_ranges(c,&det_mon[i].section.ranges);
	break;
      case MONITOR_NAME:
	det_mon[i].type = DS_NAME;
	copy_string(c,det_mon[i].section.name);
	break;
      case ADD_COLOR_POINT:
	det_mon[i].type = DS_WHITE_P;
	get_whitepoint_section(c,det_mon[i].section.wp);
	break;
      case ADD_STD_TIMINGS:
	det_mon[i].type = DS_STD_TIMINGS;
	get_dst_timing_section(c,det_mon[i].section.std_t, ver);
	break;
      case ADD_DUMMY:
	det_mon[i].type = DS_DUMMY;
        break;
      }
    } else {
      det_mon[i].type = DT;
      get_detailed_timing_section(c,&det_mon[i].section.d_timings);
    }
    NEXT_DT_MD_SECTION;
  }
}

static void
copy_string(Uchar *c, Uchar *s)
{
  int i;
  c = c + 5;
  for (i = 0; (i < 13 && *c != 0x0A); i++)
    *(s++) = *(c++);
  *s = 0;
  while (i-- && (*--s == 0x20)) *s = 0;
}

static void
get_dst_timing_section(Uchar *c, struct std_timings *t,
		       struct edid_version *v)
{
  int j;
    c = c + 5;
    for (j = 0; j < 5; j++) {
	t[j].hsize = HSIZE1;
	VSIZE1(t[j].vsize);
	t[j].refresh = REFRESH_R;
	t[j].id = STD_TIMING_ID;
	NEXT_STD_TIMING;
    }
}

static void
get_monitor_ranges(Uchar *c, struct monitor_ranges *r)
{
    r->min_v = MIN_V;
    r->max_v = MAX_V;
    r->min_h = MIN_H;
    r->max_h = MAX_H;
    r->max_clock = 0;
    if(MAX_CLOCK != 0xff) /* is specified? */
	r->max_clock = MAX_CLOCK * 10;
    if (HAVE_2ND_GTF) {
	r->gtf_2nd_f = F_2ND_GTF;
	r->gtf_2nd_c = C_2ND_GTF;
	r->gtf_2nd_m = M_2ND_GTF;
	r->gtf_2nd_k = K_2ND_GTF;
	r->gtf_2nd_j = J_2ND_GTF;
    } else
	r->gtf_2nd_f = 0;
}

static void
get_whitepoint_section(Uchar *c, struct whitePoints *wp)
{
    wp[1].white_x = WHITEX1;
    wp[1].white_y = WHITEY1;
    wp[2].white_x = WHITEX2;
    wp[2].white_y = WHITEY2;
    wp[1].index  = WHITE_INDEX1;
    wp[2].index  = WHITE_INDEX2;
    wp[1].white_gamma  = WHITE_GAMMA1;
    wp[2].white_gamma  = WHITE_GAMMA2;
}

static void
get_detailed_timing_section(Uchar *c, struct detailed_timings *r)
{
  r->clock = PIXEL_CLOCK;
  r->h_active = H_ACTIVE;
  r->h_blanking = H_BLANK;
  r->v_active = V_ACTIVE;
  r->v_blanking = V_BLANK;
  r->h_sync_off = H_SYNC_OFF;
  r->h_sync_width = H_SYNC_WIDTH;
  r->v_sync_off = V_SYNC_OFF;
  r->v_sync_width = V_SYNC_WIDTH;
  r->h_size = H_SIZE;
  r->v_size = V_SIZE;
  r->h_border = H_BORDER;
  r->v_border = V_BORDER;
  r->interlaced = INTERLACED;
  r->stereo = STEREO;
  r->stereo_1 = STEREO1;
  r->sync = SYNC_T;
  r->misc = MISC;
}


static Bool
validate_version(RHDPtr rhdPtr, struct edid_version *r)
{
    if (r->version != 1)
      return FALSE;
    if (r->revision > 3)
    {
      DBG(dbgprintf("EDID Version 1.%d not yet supported\n",r->revision));
      return FALSE;
    }
    return TRUE;
}
