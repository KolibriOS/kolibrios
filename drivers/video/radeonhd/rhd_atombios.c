/*
 * Copyright 2007, 2008  Egbert Eich   <eich@novell.com>
 * Copyright 2007, 2008  Luc Verhaegen <lverhaegen@novell.com>
 * Copyright 2007, 2008  Matthias Hopf <mhopf@novell.com>
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
/* #define RHD_DEBUG */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include "xf86.h"


/* only for testing now */

#include "rhd.h"
#include "edid.h"
#include "rhd_atombios.h"
#include "rhd_connector.h"
#include "rhd_output.h"
#include "rhd_biosscratch.h"
#include "rhd_monitor.h"
#include "rhd_card.h"
#include "rhd_regs.h"

#ifdef ATOM_BIOS
# include "rhd_atomwrapper.h"
//# include "xf86int10.h"
# ifdef ATOM_BIOS_PARSER
#  define INT8 INT8
#  define INT16 INT16
#  define INT32 INT32
#  include "AtomBios/includes/CD_Common_Types.h"
# else
#  ifndef ULONG
typedef unsigned int ULONG;
#   define ULONG ULONG
#  endif
#  ifndef UCHAR
typedef unsigned char UCHAR;
#   define UCHAR UCHAR
#  endif
#  ifndef USHORT
typedef unsigned short USHORT;
#   define USHORT USHORT
#  endif
# endif

# include "atomBios/includes/atombios.h"
# include "atomBios/includes/ObjectID.h"

typedef AtomBiosResult (*AtomBiosRequestFunc)(atomBiosHandlePtr handle,
					  AtomBiosRequestID unused, AtomBiosArgPtr data);
typedef struct rhdConnectorInfo *rhdConnectorInfoPtr;

static AtomBiosResult rhdAtomInit(atomBiosHandlePtr unused1,
				      AtomBiosRequestID unused2, AtomBiosArgPtr data);
static AtomBiosResult rhdAtomTearDown(atomBiosHandlePtr handle,
					  AtomBiosRequestID unused1, AtomBiosArgPtr unused2);
static AtomBiosResult rhdAtomGetDataInCodeTable(atomBiosHandlePtr handle,
						AtomBiosRequestID unused, AtomBiosArgPtr data);
static AtomBiosResult rhdAtomVramInfoQuery(atomBiosHandlePtr handle,
					       AtomBiosRequestID func, AtomBiosArgPtr data);
static AtomBiosResult rhdAtomTmdsInfoQuery(atomBiosHandlePtr handle,
					       AtomBiosRequestID func, AtomBiosArgPtr data);
static AtomBiosResult rhdAtomAllocateFbScratch(atomBiosHandlePtr handle,
						   AtomBiosRequestID func, AtomBiosArgPtr data);
static AtomBiosResult rhdAtomLvdsGetTimings(atomBiosHandlePtr handle,
					AtomBiosRequestID unused, AtomBiosArgPtr data);
static AtomBiosResult rhdAtomLvdsInfoQuery(atomBiosHandlePtr handle,
					       AtomBiosRequestID func,  AtomBiosArgPtr data);
static AtomBiosResult rhdAtomGPIOI2CInfoQuery(atomBiosHandlePtr handle,
						  AtomBiosRequestID func, AtomBiosArgPtr data);
static AtomBiosResult rhdAtomFirmwareInfoQuery(atomBiosHandlePtr handle,
						   AtomBiosRequestID func, AtomBiosArgPtr data);
static AtomBiosResult rhdAtomConnectorInfo(atomBiosHandlePtr handle,
             AtomBiosRequestID unused, AtomBiosArgPtr data);
static AtomBiosResult rhdAtomOutputDeviceList(atomBiosHandlePtr handle,
					   AtomBiosRequestID unused, AtomBiosArgPtr data);
static AtomBiosResult
rhdAtomAnalogTVInfoQuery(atomBiosHandlePtr handle,
			 AtomBiosRequestID func, AtomBiosArgPtr data);
static AtomBiosResult
rhdAtomGetConditionalGoldenSetting(atomBiosHandlePtr handle,
				   AtomBiosRequestID func, AtomBiosArgPtr data);

# ifdef ATOM_BIOS_PARSER
static AtomBiosResult rhdAtomExec(atomBiosHandlePtr handle,
				   AtomBiosRequestID unused, AtomBiosArgPtr data);
# endif
static AtomBiosResult
rhdAtomCompassionateDataQuery(atomBiosHandlePtr handle,
			      AtomBiosRequestID func, AtomBiosArgPtr data);
static AtomBiosResult
rhdAtomIntegratedSystemInfoQuery(atomBiosHandlePtr handle, AtomBiosRequestID func, AtomBiosArgPtr data);
static AtomBiosResult
atomSetRegisterListLocation(atomBiosHandlePtr handle, AtomBiosRequestID func, AtomBiosArgPtr data);
static AtomBiosResult
atomRestoreRegisters(atomBiosHandlePtr handle, AtomBiosRequestID func, AtomBiosArgPtr data);


enum msgDataFormat {
    MSG_FORMAT_NONE,
    MSG_FORMAT_HEX,
    MSG_FORMAT_DEC
};

enum atomRegisterType {
    atomRegisterMMIO,
    atomRegisterMC,
    atomRegisterPLL,
    atomRegisterPCICFG
};

struct atomBIOSRequests {
    AtomBiosRequestID id;
    AtomBiosRequestFunc request;
    char *message;
    enum msgDataFormat message_format;
} AtomBiosRequestList [] = {
    {ATOMBIOS_INIT,			rhdAtomInit,
     "AtomBIOS Init",				MSG_FORMAT_NONE},
    {ATOMBIOS_TEARDOWN,			rhdAtomTearDown,
     "AtomBIOS Teardown",			MSG_FORMAT_NONE},
# ifdef ATOM_BIOS_PARSER
    {ATOMBIOS_EXEC,			rhdAtomExec,
     "AtomBIOS Exec",				MSG_FORMAT_NONE},
#endif
    {ATOMBIOS_ALLOCATE_FB_SCRATCH,	rhdAtomAllocateFbScratch,
     "AtomBIOS Set FB Space",			MSG_FORMAT_NONE},
    {ATOMBIOS_GET_CONNECTORS,		rhdAtomConnectorInfo,
     "AtomBIOS Get Connectors",			MSG_FORMAT_NONE},
    {ATOMBIOS_GET_OUTPUT_DEVICE_LIST,	rhdAtomOutputDeviceList,
     "AtomBIOS Get Output Info",		MSG_FORMAT_NONE},
    {ATOMBIOS_GET_PANEL_MODE,		rhdAtomLvdsGetTimings,
     "AtomBIOS Get Panel Mode",			MSG_FORMAT_NONE},
    {ATOMBIOS_GET_PANEL_EDID,		rhdAtomLvdsGetTimings,
     "AtomBIOS Get Panel EDID",			MSG_FORMAT_NONE},
    {ATOMBIOS_GET_CODE_DATA_TABLE,	rhdAtomGetDataInCodeTable,
     "AtomBIOS Get Datatable from Codetable",   MSG_FORMAT_NONE},
    {GET_DEFAULT_ENGINE_CLOCK,		rhdAtomFirmwareInfoQuery,
     "Default Engine Clock",			MSG_FORMAT_DEC},
    {GET_DEFAULT_MEMORY_CLOCK,		rhdAtomFirmwareInfoQuery,
     "Default Memory Clock",			MSG_FORMAT_DEC},
    {GET_MAX_PIXEL_CLOCK_PLL_OUTPUT,	rhdAtomFirmwareInfoQuery,
     "Maximum Pixel ClockPLL Frequency Output", MSG_FORMAT_DEC},
    {GET_MIN_PIXEL_CLOCK_PLL_OUTPUT,	rhdAtomFirmwareInfoQuery,
     "Minimum Pixel ClockPLL Frequency Output", MSG_FORMAT_DEC},
    {GET_MAX_PIXEL_CLOCK_PLL_INPUT,	rhdAtomFirmwareInfoQuery,
     "Maximum Pixel ClockPLL Frequency Input", MSG_FORMAT_DEC},
    {GET_MIN_PIXEL_CLOCK_PLL_INPUT,	rhdAtomFirmwareInfoQuery,
     "Minimum Pixel ClockPLL Frequency Input", MSG_FORMAT_DEC},
    {GET_MAX_PIXEL_CLK,			rhdAtomFirmwareInfoQuery,
     "Maximum Pixel Clock",			MSG_FORMAT_DEC},
    {GET_REF_CLOCK,			rhdAtomFirmwareInfoQuery,
     "Reference Clock",				MSG_FORMAT_DEC},
    {GET_FW_FB_START,			rhdAtomVramInfoQuery,
      "Start of VRAM area used by Firmware",	MSG_FORMAT_HEX},
    {GET_FW_FB_SIZE,			rhdAtomVramInfoQuery,
      "Framebuffer space used by Firmware (kb)", MSG_FORMAT_DEC},
    {ATOM_TMDS_MAX_FREQUENCY,		rhdAtomTmdsInfoQuery,
     "TMDS Max Frequency",			MSG_FORMAT_DEC},
    {ATOM_TMDS_PLL_CHARGE_PUMP,		rhdAtomTmdsInfoQuery,
     "TMDS PLL ChargePump",			MSG_FORMAT_DEC},
    {ATOM_TMDS_PLL_DUTY_CYCLE,		rhdAtomTmdsInfoQuery,
     "TMDS PLL DutyCycle",			MSG_FORMAT_DEC},
    {ATOM_TMDS_PLL_VCO_GAIN,		rhdAtomTmdsInfoQuery,
     "TMDS PLL VCO Gain",			MSG_FORMAT_DEC},
    {ATOM_TMDS_PLL_VOLTAGE_SWING,	rhdAtomTmdsInfoQuery,
     "TMDS PLL VoltageSwing",			MSG_FORMAT_DEC},
    {ATOM_LVDS_SUPPORTED_REFRESH_RATE,	rhdAtomLvdsInfoQuery,
     "LVDS Supported Refresh Rate",		MSG_FORMAT_DEC},
    {ATOM_LVDS_OFF_DELAY,		rhdAtomLvdsInfoQuery,
     "LVDS Off Delay",				MSG_FORMAT_DEC},
    {ATOM_LVDS_SEQ_DIG_ONTO_DE,		rhdAtomLvdsInfoQuery,
     "LVDS SEQ Dig onto DE",			MSG_FORMAT_DEC},
    {ATOM_LVDS_SEQ_DE_TO_BL,		rhdAtomLvdsInfoQuery,
     "LVDS SEQ DE to BL",			MSG_FORMAT_DEC},
    {ATOM_LVDS_TEMPORAL_DITHER,	        rhdAtomLvdsInfoQuery,
     "LVDS Temporal Dither ",			MSG_FORMAT_HEX},
    {ATOM_LVDS_SPATIAL_DITHER,	        rhdAtomLvdsInfoQuery,
     "LVDS Spatial Dither ",			MSG_FORMAT_HEX},
    {ATOM_LVDS_DUALLINK,		rhdAtomLvdsInfoQuery,
     "LVDS Duallink",				MSG_FORMAT_HEX},
    {ATOM_LVDS_GREYLVL,			rhdAtomLvdsInfoQuery,
     "LVDS Grey Level",				MSG_FORMAT_HEX},
    {ATOM_LVDS_FPDI,			rhdAtomLvdsInfoQuery,
     "LVDS FPDI",				MSG_FORMAT_HEX},
    {ATOM_LVDS_24BIT,			rhdAtomLvdsInfoQuery,
     "LVDS 24Bit",				MSG_FORMAT_HEX},
    {ATOM_GPIO_I2C_CLK_MASK,		rhdAtomGPIOI2CInfoQuery,
     "GPIO_I2C_Clk_Mask",			MSG_FORMAT_HEX},
    {ATOM_GPIO_I2C_CLK_MASK_SHIFT,	rhdAtomGPIOI2CInfoQuery,
     "GPIO_I2C_Clk_Mask_Shift",			MSG_FORMAT_HEX},
    {ATOM_GPIO_I2C_DATA_MASK,		rhdAtomGPIOI2CInfoQuery,
     "GPIO_I2C_Data_Mask",			MSG_FORMAT_HEX},
    {ATOM_GPIO_I2C_DATA_MASK_SHIFT,	rhdAtomGPIOI2CInfoQuery,
     "GPIO_I2C_Data_Mask_Shift",		MSG_FORMAT_HEX},
    {ATOM_DAC1_BG_ADJ,		rhdAtomCompassionateDataQuery,
     "DAC1 BG Adjustment",			MSG_FORMAT_HEX},
    {ATOM_DAC1_DAC_ADJ,		rhdAtomCompassionateDataQuery,
     "DAC1 DAC Adjustment",			MSG_FORMAT_HEX},
    {ATOM_DAC1_FORCE,		rhdAtomCompassionateDataQuery,
     "DAC1 Force Data",				MSG_FORMAT_HEX},
    {ATOM_DAC2_CRTC2_BG_ADJ,	rhdAtomCompassionateDataQuery,
     "DAC2_CRTC2 BG Adjustment",		MSG_FORMAT_HEX},
    {ATOM_DAC2_NTSC_BG_ADJ,	rhdAtomCompassionateDataQuery,
     "DAC2_NTSC BG Adjustment",			MSG_FORMAT_HEX},
    {ATOM_DAC2_PAL_BG_ADJ,	rhdAtomCompassionateDataQuery,
     "DAC2_PAL BG Adjustment",			MSG_FORMAT_HEX},
    {ATOM_DAC2_CV_BG_ADJ,	rhdAtomCompassionateDataQuery,
     "DAC2_CV BG Adjustment",			MSG_FORMAT_HEX},
    {ATOM_DAC2_CRTC2_DAC_ADJ,	rhdAtomCompassionateDataQuery,
     "DAC2_CRTC2 DAC Adjustment",		MSG_FORMAT_HEX},
    {ATOM_DAC2_NTSC_DAC_ADJ,	rhdAtomCompassionateDataQuery,
     "DAC2_NTSC DAC Adjustment",		MSG_FORMAT_HEX},
    {ATOM_DAC2_PAL_DAC_ADJ,	rhdAtomCompassionateDataQuery,
     "DAC2_PAL DAC Adjustment",			MSG_FORMAT_HEX},
    {ATOM_DAC2_CV_DAC_ADJ,	rhdAtomCompassionateDataQuery,
     "DAC2_CV DAC Adjustment",			MSG_FORMAT_HEX},
    {ATOM_DAC2_CRTC2_FORCE,	rhdAtomCompassionateDataQuery,
     "DAC2_CRTC2 Force",			MSG_FORMAT_HEX},
    {ATOM_DAC2_CRTC2_MUX_REG_IND,rhdAtomCompassionateDataQuery,
     "DAC2_CRTC2 Mux Register Index",		MSG_FORMAT_HEX},
    {ATOM_DAC2_CRTC2_MUX_REG_INFO,rhdAtomCompassionateDataQuery,
     "DAC2_CRTC2 Mux Register Info",		MSG_FORMAT_HEX},
    {ATOM_ANALOG_TV_MODE, rhdAtomAnalogTVInfoQuery,
     "Analog TV Mode",				MSG_FORMAT_NONE},
    {ATOM_ANALOG_TV_DEFAULT_MODE, rhdAtomAnalogTVInfoQuery,
     "Analog TV Default Mode",			MSG_FORMAT_DEC},
    {ATOM_ANALOG_TV_SUPPORTED_MODES, rhdAtomAnalogTVInfoQuery,
     "Analog TV Supported Modes",		MSG_FORMAT_HEX},
    {ATOM_GET_CONDITIONAL_GOLDEN_SETTINGS, rhdAtomGetConditionalGoldenSetting,
     "Conditional Golden Setting",		MSG_FORMAT_NONE},
    {ATOM_GET_PCIENB_CFG_REG7, rhdAtomIntegratedSystemInfoQuery,
     "PCIE NB Cfg7Reg",				MSG_FORMAT_HEX},
    {ATOM_GET_CAPABILITY_FLAG, rhdAtomIntegratedSystemInfoQuery,
     "CapabilityFlag",				MSG_FORMAT_HEX},
    {ATOM_GET_PCIE_LANES, rhdAtomIntegratedSystemInfoQuery,
     "PCI Lanes",				MSG_FORMAT_NONE},
    {ATOM_SET_REGISTER_LIST_LOCATION, atomSetRegisterListLocation,
     "Register List Location",			MSG_FORMAT_NONE},
    {ATOM_RESTORE_REGISTERS, atomRestoreRegisters,
     "Restore Registers",			MSG_FORMAT_NONE},
    {FUNC_END,					NULL,
     NULL,					MSG_FORMAT_NONE}
};

/*
 * This works around a bug in atombios.h where
 * ATOM_MAX_SUPPORTED_DEVICE_INFO is specified incorrectly.
 */

#define ATOM_MAX_SUPPORTED_DEVICE_INFO_HD (ATOM_DEVICE_RESERVEDF_INDEX+1)
typedef struct _ATOM_SUPPORTED_DEVICES_INFO_HD
{
    ATOM_COMMON_TABLE_HEADER      sHeader;
    USHORT                        usDeviceSupport;
    ATOM_CONNECTOR_INFO_I2C       asConnInfo[ATOM_MAX_SUPPORTED_DEVICE_INFO_HD];
    ATOM_CONNECTOR_INC_SRC_BITMAP asIntSrcInfo[ATOM_MAX_SUPPORTED_DEVICE_INFO_HD];
} ATOM_SUPPORTED_DEVICES_INFO_HD;

typedef struct _atomDataTables
{
    unsigned char                       *UtilityPipeLine;
    ATOM_MULTIMEDIA_CAPABILITY_INFO     *MultimediaCapabilityInfo;
    ATOM_MULTIMEDIA_CONFIG_INFO         *MultimediaConfigInfo;
    ATOM_STANDARD_VESA_TIMING           *StandardVESA_Timing;
    union {
        void                            *base;
        ATOM_FIRMWARE_INFO              *FirmwareInfo;
        ATOM_FIRMWARE_INFO_V1_2         *FirmwareInfo_V_1_2;
        ATOM_FIRMWARE_INFO_V1_3         *FirmwareInfo_V_1_3;
        ATOM_FIRMWARE_INFO_V1_4         *FirmwareInfo_V_1_4;
    } FirmwareInfo;
    ATOM_DAC_INFO                       *DAC_Info;
    union {
        void                            *base;
        ATOM_LVDS_INFO                  *LVDS_Info;
        ATOM_LVDS_INFO_V12              *LVDS_Info_v12;
    } LVDS_Info;
    ATOM_TMDS_INFO                      *TMDS_Info;
    ATOM_ANALOG_TV_INFO                 *AnalogTV_Info;
    union {
        void                            *base;
        ATOM_SUPPORTED_DEVICES_INFO     *SupportedDevicesInfo;
        ATOM_SUPPORTED_DEVICES_INFO_2   *SupportedDevicesInfo_2;
        ATOM_SUPPORTED_DEVICES_INFO_2d1 *SupportedDevicesInfo_2d1;
        ATOM_SUPPORTED_DEVICES_INFO_HD  *SupportedDevicesInfo_HD;
    } SupportedDevicesInfo;
    ATOM_GPIO_I2C_INFO                  *GPIO_I2C_Info;
    ATOM_VRAM_USAGE_BY_FIRMWARE         *VRAM_UsageByFirmware;
    ATOM_GPIO_PIN_LUT                   *GPIO_Pin_LUT;
    ATOM_VESA_TO_INTENAL_MODE_LUT       *VESA_ToInternalModeLUT;
    union {
        void                            *base;
        ATOM_COMPONENT_VIDEO_INFO       *ComponentVideoInfo;
        ATOM_COMPONENT_VIDEO_INFO_V21   *ComponentVideoInfo_v21;
    } ComponentVideoInfo;
/**/unsigned char                       *PowerPlayInfo;
    COMPASSIONATE_DATA                  *CompassionateData;
    ATOM_DISPLAY_DEVICE_PRIORITY_INFO   *SaveRestoreInfo;
/**/unsigned char                       *PPLL_SS_Info;
    ATOM_OEM_INFO                       *OemInfo;
    ATOM_XTMDS_INFO                     *XTMDS_Info;
    ATOM_ASIC_MVDD_INFO                 *MclkSS_Info;
    ATOM_OBJECT_HEADER                  *Object_Header;
    INDIRECT_IO_ACCESS                  *IndirectIOAccess;
    ATOM_MC_INIT_PARAM_TABLE            *MC_InitParameter;
/**/unsigned char                       *ASIC_VDDC_Info;
    ATOM_ASIC_INTERNAL_SS_INFO          *ASIC_InternalSS_Info;
/**/unsigned char                       *TV_VideoMode;
    union {
        void                            *base;
        ATOM_VRAM_INFO_V2               *VRAM_Info_v2;
        ATOM_VRAM_INFO_V3               *VRAM_Info_v3;
    } VRAM_Info;
    ATOM_MEMORY_TRAINING_INFO           *MemoryTrainingInfo;
    union {
        void                            *base;
        ATOM_INTEGRATED_SYSTEM_INFO     *IntegratedSystemInfo;
        ATOM_INTEGRATED_SYSTEM_INFO_V2  *IntegratedSystemInfo_v2;
    } IntegratedSystemInfo;
    ATOM_ASIC_PROFILING_INFO            *ASIC_ProfilingInfo;
    ATOM_VOLTAGE_OBJECT_INFO            *VoltageObjectInfo;
    ATOM_POWER_SOURCE_INFO              *PowerSourceInfo;
} atomDataTables, *atomDataTablesPtr;

struct atomSaveListRecord
{
    /* header */
    int Length;
    int Last;
    struct atomRegisterList{
	enum atomRegisterType Type;
	CARD32 Address;
	CARD32 Value;
    } RegisterList[1];
};

struct atomSaveListObject
{
    struct atomSaveListObject *next;
    struct atomSaveListRecord **SaveList;
};

typedef struct _atomBiosHandle {
    int scrnIndex;
    RHDPtr rhdPtr;
    unsigned char *BIOSBase;
    atomDataTablesPtr atomDataPtr;
    pointer *scratchBase;
    CARD32 fbBase;
    PCITAG PciTag;
    unsigned int BIOSImageSize;
    unsigned char *codeTable;
    struct atomSaveListRecord **SaveList;
    struct atomSaveListObject *SaveListObjects;
} atomBiosHandleRec;

enum {
    legacyBIOSLocation = 0xC0000,
    legacyBIOSMax = 0x10000
};

struct atomConnectorInfoPrivate {
    enum atomDevice *Devices;
};

#  ifdef ATOM_BIOS_PARSER

#   define LOG_CAIL LOG_DEBUG + 1

static void
atomDebugPrintPspace(atomBiosHandlePtr handle, AtomBiosArgPtr data, int size)
{
    CARD32 *pspace = (CARD32 *)data->exec.pspace;
    int i = 0;

    size >>= 2;

    while (i++,size--)
	RHDDebug(handle->scrnIndex, " Pspace[%2.2i]: 0x%8.8x\n", i, *(pspace++));
}

/*
#define va_start(v,l) __builtin_va_start(v,l)
#define va_end(v)	__builtin_va_end(v)
#define va_arg(v,l)	__builtin_va_arg(v,l)
#if !defined(__STRICT_ANSI__) || __STDC_VERSION__ + 0 >= 199900L
#define va_copy(d,s)	__builtin_va_copy(d,s)
#endif
#define __va_copy(d,s)	__builtin_va_copy(d,s)

typedef __builtin_va_list __gnuc_va_list;
typedef __gnuc_va_list va_list;

#define arg(x) va_arg (ap, u32_t)

static void
CailDebug(int scrnIndex, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    xf86VDrvMsgVerb(scrnIndex, X_INFO, LOG_CAIL, format, ap);
    va_end(ap);
}
#   define CAILFUNC(ptr) \
  CailDebug(((atomBiosHandlePtr)(ptr))->scrnIndex, "CAIL: %s\n", __func__)
*/

#  endif

#  define DEBUG_VERSION(index, handle, version) \
    xf86DrvMsgVerb(handle->scrnIndex, X_INFO, 3, "%s returned version %i for index 0x%x\n" ,__func__,version.cref,index)
#  define DEBUG_VERSION_NAME(index, handle, name, version)		\
    xf86DrvMsgVerb(handle->scrnIndex, X_INFO, 3, "%s(%s) returned version %i for index 0x%x\n",\
		   __func__,name,version.cref,index)

static int
rhdAtomAnalyzeCommonHdr(ATOM_COMMON_TABLE_HEADER *hdr)
{
  if (hdr->usStructureSize == 0xaa55)
    return FALSE;

  return TRUE;
}

static int
rhdAtomAnalyzeRomHdr(unsigned char *rombase,
              ATOM_ROM_HEADER *hdr,
		     unsigned int *data_offset, unsigned int *code_table)
{
    if (!rhdAtomAnalyzeCommonHdr(&hdr->sHeader)) {
     return FALSE;
  }
    xf86DrvMsg(-1,X_NONE,"\tSubsystemVendorID: 0x%4.4x SubsystemID: 0x%4.4x\n",
               hdr->usSubsystemVendorID,hdr->usSubsystemID);
    xf86DrvMsg(-1,X_NONE,"\tIOBaseAddress: 0x%4.4x\n",hdr->usIoBaseAddress);
    xf86DrvMsgVerb(-1,X_NONE,3,"\tFilename: %s\n",rombase + hdr->usConfigFilenameOffset);
    xf86DrvMsgVerb(-1,X_NONE,3,"\tBIOS Bootup Message: %s\n",
		   rombase + hdr->usBIOS_BootupMessageOffset);

  *data_offset = hdr->usMasterDataTableOffset;
    *code_table = hdr->usMasterCommandTableOffset;

  return TRUE;
}

static int
rhdAtomAnalyzeRomDataTable(unsigned char *base, int offset,
                    void *ptr,unsigned short *size)
{
    ATOM_COMMON_TABLE_HEADER *table = (ATOM_COMMON_TABLE_HEADER *)
        (base + offset);

   if (!*size || !rhdAtomAnalyzeCommonHdr(table)) {
    if (*size) *size -= 2;
    *(void **)ptr = NULL;
    return FALSE;
  }
  *size -= 2;
  *(void **)ptr = (void *)(table);
  return TRUE;
}

static Bool
rhdAtomGetTableRevisionAndSize(ATOM_COMMON_TABLE_HEADER *hdr,
			       CARD8 *contentRev,
			       CARD8 *formatRev,
			       unsigned short *size)
{
    if (!hdr)
        return FALSE;

    if (contentRev) *contentRev = hdr->ucTableContentRevision;
    if (formatRev) *formatRev = hdr->ucTableFormatRevision;
    if (size) *size = (short)hdr->usStructureSize
                   - sizeof(ATOM_COMMON_TABLE_HEADER);

    return TRUE;
}

static Bool
rhdAtomGetCommandTableRevisionSize(atomBiosHandlePtr handle, int index,
				   CARD8 *contentRev, CARD8 *formatRev, unsigned short *size)
{
    unsigned short offset = ((USHORT *)&(((ATOM_MASTER_COMMAND_TABLE *)handle->codeTable)
					 ->ListOfCommandTables))[index];
    ATOM_COMMON_ROM_COMMAND_TABLE_HEADER *hdr = (ATOM_COMMON_ROM_COMMAND_TABLE_HEADER *)(handle->BIOSBase + offset);
    ATOM_COMMON_TABLE_HEADER hdr1 = hdr->CommonHeader;

    if (!offset) {
	*contentRev = *formatRev = 0;
	return FALSE;
    }
    return rhdAtomGetTableRevisionAndSize(&hdr1, contentRev, formatRev, size);
}

static Bool
rhdAtomAnalyzeMasterDataTable(unsigned char *base,
			      ATOM_MASTER_DATA_TABLE *table,
			      atomDataTablesPtr data)
{
    ATOM_MASTER_LIST_OF_DATA_TABLES *data_table =
        &table->ListOfDataTables;
    unsigned short size;

    if (!rhdAtomAnalyzeCommonHdr(&table->sHeader))
        return FALSE;
    if (!rhdAtomGetTableRevisionAndSize(&table->sHeader,NULL,NULL,
					&size))
        return FALSE;
# define SET_DATA_TABLE(x) {\
   rhdAtomAnalyzeRomDataTable(base,data_table->x,(void *)(&(data->x)),&size); \
    }

# define SET_DATA_TABLE_VERS(x) {\
   rhdAtomAnalyzeRomDataTable(base,data_table->x,&(data->x.base),&size); \
    }

    SET_DATA_TABLE(UtilityPipeLine);
    SET_DATA_TABLE(MultimediaCapabilityInfo);
    SET_DATA_TABLE(MultimediaConfigInfo);
    SET_DATA_TABLE(StandardVESA_Timing);
    SET_DATA_TABLE_VERS(FirmwareInfo);
    SET_DATA_TABLE(DAC_Info);
    SET_DATA_TABLE_VERS(LVDS_Info);
    SET_DATA_TABLE(TMDS_Info);
    SET_DATA_TABLE(AnalogTV_Info);
    SET_DATA_TABLE_VERS(SupportedDevicesInfo);
    SET_DATA_TABLE(GPIO_I2C_Info);
    SET_DATA_TABLE(VRAM_UsageByFirmware);
    SET_DATA_TABLE(GPIO_Pin_LUT);
    SET_DATA_TABLE(VESA_ToInternalModeLUT);
    SET_DATA_TABLE_VERS(ComponentVideoInfo);
    SET_DATA_TABLE(PowerPlayInfo);
    SET_DATA_TABLE(CompassionateData);
    SET_DATA_TABLE(SaveRestoreInfo);
    SET_DATA_TABLE(PPLL_SS_Info);
    SET_DATA_TABLE(OemInfo);
    SET_DATA_TABLE(XTMDS_Info);
    SET_DATA_TABLE(MclkSS_Info);
    SET_DATA_TABLE(Object_Header);
    SET_DATA_TABLE(IndirectIOAccess);
    SET_DATA_TABLE(MC_InitParameter);
    SET_DATA_TABLE(ASIC_VDDC_Info);
    SET_DATA_TABLE(ASIC_InternalSS_Info);
    SET_DATA_TABLE(TV_VideoMode);
    SET_DATA_TABLE_VERS(VRAM_Info);
    SET_DATA_TABLE(MemoryTrainingInfo);
    SET_DATA_TABLE_VERS(IntegratedSystemInfo);
    SET_DATA_TABLE(ASIC_ProfilingInfo);
    SET_DATA_TABLE(VoltageObjectInfo);
    SET_DATA_TABLE(PowerSourceInfo);
# undef SET_DATA_TABLE

    return TRUE;
}

static Bool
rhdAtomGetTables(RHDPtr rhdPtr, unsigned char *base,
		 atomDataTables *atomDataPtr, unsigned char **codeTablePtr,
		 unsigned int BIOSImageSize)
{
  unsigned int  data_offset;
    unsigned int  code_offset;
    int scrnIndex=0;

  unsigned int atom_romhdr_off =  *(unsigned short*)
        (base + OFFSET_TO_POINTER_TO_ATOM_ROM_HEADER);
  ATOM_ROM_HEADER *atom_rom_hdr =
        (ATOM_ROM_HEADER *)(base + atom_romhdr_off);

    RHDFUNCI(scrnIndex);

    if (atom_romhdr_off + sizeof(ATOM_ROM_HEADER) > BIOSImageSize) {
	xf86DrvMsg(scrnIndex,X_ERROR,
		   "%s: AtomROM header extends beyond BIOS image\n",__func__);
    return FALSE;
  }

    if (memcmp("ATOM",&atom_rom_hdr->uaFirmWareSignature,4)) {
        xf86DrvMsg(scrnIndex,X_ERROR,"%s: No AtomBios signature found\n",
		   __func__);
    return FALSE;
  }
    xf86DrvMsg(scrnIndex, X_INFO, "ATOM BIOS Rom: \n");
    if (!rhdAtomAnalyzeRomHdr(base, atom_rom_hdr, &data_offset, &code_offset)) {
        xf86DrvMsg(scrnIndex, X_ERROR, "RomHeader invalid\n");
     return FALSE;
  }

    if (data_offset + sizeof (ATOM_MASTER_DATA_TABLE) > BIOSImageSize) {
	xf86DrvMsg(scrnIndex,X_ERROR,"%s: Atom data table outside of BIOS\n",
		   __func__);
	return FALSE;
  }

    if (code_offset + sizeof (ATOM_MASTER_COMMAND_TABLE) > BIOSImageSize) {
	xf86DrvMsg(scrnIndex, X_ERROR, "%s: Atom command table outside of BIOS\n",
		   __func__);
	(*codeTablePtr) = NULL;
    } else
	(*codeTablePtr) = base + code_offset;

  if (!rhdAtomAnalyzeMasterDataTable(base, (ATOM_MASTER_DATA_TABLE *)
				       (base + data_offset),
				       atomDataPtr)) {
        xf86DrvMsg(scrnIndex, X_ERROR, "%s: ROM Master Table invalid\n",
		   __func__);
    return FALSE;
  }

  return TRUE;
}

static Bool
rhdAtomGetFbBaseAndSize(atomBiosHandlePtr handle, unsigned int *base,
			unsigned int *size)
{
  AtomBiosArgRec data;
  if (RHDAtomBiosFunc(handle->rhdPtr, handle, GET_FW_FB_SIZE, &data)
	== ATOM_SUCCESS) {
	if (data.val == 0) {
	    xf86DrvMsg(handle->scrnIndex, X_WARNING, "%s: AtomBIOS specified VRAM "
		       "scratch space size invalid\n", __func__);
	    return FALSE;
	}
	if (size)
	    *size = (int)data.val;
    } else
    return FALSE;
    if (RHDAtomBiosFunc(handle->rhdPtr, handle, GET_FW_FB_START, &data)
	== ATOM_SUCCESS) {
    if (data.val == 0)
	    return FALSE;
    if (base)
	    *base = (int)data.val;
  }
  return TRUE;
}

/*
 * Uses videoRam form ScrnInfoRec.
 */
static AtomBiosResult
rhdAtomAllocateFbScratch(atomBiosHandlePtr handle,
			 AtomBiosRequestID func, AtomBiosArgPtr data)
{
  unsigned int fb_base = 0;
  unsigned int fb_size = 0;
  unsigned int start = data->fb.start;
  unsigned int size = data->fb.size;
  handle->scratchBase = NULL;
  handle->fbBase = 0;

    if (rhdAtomGetFbBaseAndSize(handle, &fb_base, &fb_size)) {
	xf86DrvMsg(handle->scrnIndex, X_INFO, "AtomBIOS requests %ikB"
		   " of VRAM scratch space\n",fb_size);
    fb_size *= 1024; /* convert to bytes */
	xf86DrvMsg(handle->scrnIndex, X_INFO, "AtomBIOS VRAM scratch base: 0x%x\n",
		   fb_base);
    } else {
    fb_size = 20 * 1024;
	    xf86DrvMsg(handle->scrnIndex, X_INFO, " default to: %i\n",fb_size);
  }
    if (fb_base && fb_size && size) {
	/* 4k align */
    fb_size = (fb_size & ~(CARD32)0xfff) + ((fb_size & 0xfff) ? 1 : 0);
	if ((fb_base + fb_size) > (start + size)) {
	    xf86DrvMsg(handle->scrnIndex, X_WARNING,
		       "%s: FW FB scratch area %i (size: %i)"
		       " extends beyond available framebuffer size %i\n",
		       __func__, fb_base, fb_size, size);
	} else if ((fb_base + fb_size) < (start + size)) {
	    xf86DrvMsg(handle->scrnIndex, X_WARNING,
		       "%s: FW FB scratch area not located "
                  "at the end of VRAM. Scratch End: "
                  "0x%x VRAM End: 0x%x\n", __func__,
		       (unsigned int)(fb_base + fb_size),
		       size);
	} else if (fb_base < start) {
	    xf86DrvMsg(handle->scrnIndex, X_WARNING,
		       "%s: FW FB scratch area extends below "
                    "the base of the free VRAM: 0x%x Base: 0x%x\n",
		       __func__, (unsigned int)(fb_base), start);
	} else {
          size -= fb_size;
          handle->fbBase = fb_base;
	    return ATOM_SUCCESS;
	}
    }

    if (!handle->fbBase) {
	xf86DrvMsg(handle->scrnIndex, X_INFO,
		   "Cannot get VRAM scratch space. "
		   "Allocating in main memory instead\n");
	handle->scratchBase = xcalloc(fb_size,1);
	return ATOM_SUCCESS;
    }
    return ATOM_FAILED;
}

# ifdef ATOM_BIOS_PARSER
static Bool
rhdAtomASICInit(atomBiosHandlePtr handle)
{
    ASIC_INIT_PS_ALLOCATION asicInit;
    AtomBiosArgRec data;

    RHDFUNC(handle);

    RHDAtomBiosFunc(handle->rhdPtr, handle,
		    GET_DEFAULT_ENGINE_CLOCK,
		    &data);
    asicInit.sASICInitClocks.ulDefaultEngineClock = data.val / 10;/*in 10 Khz*/
    RHDAtomBiosFunc(handle->rhdPtr, handle,
		    GET_DEFAULT_MEMORY_CLOCK,
		    &data);
    asicInit.sASICInitClocks.ulDefaultMemoryClock = data.val / 10;/*in 10 Khz*/
    data.exec.dataSpace = NULL;
    data.exec.index = GetIndexIntoMasterTable(COMMAND, ASIC_Init);
    data.exec.pspace = &asicInit;

    xf86DrvMsg(handle->scrnIndex, X_INFO, "Calling ASIC Init\n");
    atomDebugPrintPspace(handle, &data, sizeof(asicInit));
    if (RHDAtomBiosFunc(handle->rhdPtr, handle,
			ATOMBIOS_EXEC, &data) == ATOM_SUCCESS) {
	xf86DrvMsg(handle->scrnIndex, X_INFO, "ASIC_INIT Successful\n");
	return TRUE;
    }
    xf86DrvMsg(handle->scrnIndex, X_INFO, "ASIC_INIT Failed\n");
    return FALSE;
}

/*
 *
 */
struct atomCodeTableVersion
rhdAtomASICInitVersion(atomBiosHandlePtr handle)
{
    struct atomCodeTableVersion version;
    int index = GetIndexIntoMasterTable(COMMAND, ASIC_Init);
    rhdAtomGetCommandTableRevisionSize(handle, index, &version.cref, &version.fref, NULL);
    return version;
}

/*
 *
 */
Bool
rhdAtomSetScaler(atomBiosHandlePtr handle, enum atomScaler scalerID, enum atomScaleMode mode)
{
    ENABLE_SCALER_PARAMETERS scaler;
    AtomBiosArgRec data;

    RHDFUNC(handle);

    switch (scalerID) {
	case atomScaler1:
	    scaler.ucScaler = ATOM_SCALER1;
	    break;
	case atomScaler2:
	    scaler.ucScaler = ATOM_SCALER2;
	    break;
    }

    switch (mode) {
	case atomScaleDisable:
	    scaler.ucEnable = ATOM_SCALER_DISABLE;
	    break;
	case atomScaleCenter:
	    scaler.ucEnable = ATOM_SCALER_CENTER;
	    break;
	case atomScaleExpand:
	    scaler.ucEnable = ATOM_SCALER_EXPANSION;
	    break;
	case atomScaleMulttabExpand:
	    scaler.ucEnable = ATOM_SCALER_MULTI_EX;
	    break;
    }

    data.exec.dataSpace = NULL;
    data.exec.index = GetIndexIntoMasterTable(COMMAND, EnableScaler);
    data.exec.pspace = &scaler;
    atomDebugPrintPspace(handle, &data, sizeof(scaler));
    xf86DrvMsg(handle->scrnIndex, X_INFO, "Calling EnableScaler\n");
    if (RHDAtomBiosFunc(handle->rhdPtr, handle,
			ATOMBIOS_EXEC, &data) == ATOM_SUCCESS) {
	xf86DrvMsg(handle->scrnIndex, X_INFO, "EnableScaler Successful\n");
	return TRUE;
    }
    xf86DrvMsg(handle->scrnIndex, X_INFO, "EnableScaler Failed\n");
    return FALSE;
}

/*
 *
 */
struct atomCodeTableVersion
rhdAtomSetScalerVersion(atomBiosHandlePtr handle)
{
    struct atomCodeTableVersion version;
    int index = GetIndexIntoMasterTable(COMMAND, EnableScaler);
    rhdAtomGetCommandTableRevisionSize(handle, index, &version.cref, &version.fref, NULL);
    return version;
}

/*
 *
 */
Bool
rhdAtomSetTVEncoder(atomBiosHandlePtr handle, Bool enable, int mode)
{
    TV_ENCODER_CONTROL_PS_ALLOCATION tvEncoder;
    AtomBiosArgRec data;

    RHDFUNC(handle);

    tvEncoder.sTVEncoder.ucTvStandard = mode;
    tvEncoder.sTVEncoder.ucAction = enable ? 1 :0;

    data.exec.dataSpace = NULL;
    data.exec.pspace = &tvEncoder;
    data.exec.index =  GetIndexIntoMasterTable(COMMAND, TVEncoderControl);

    xf86DrvMsg(handle->scrnIndex, X_INFO, "Calling SetTVEncoder\n");
    atomDebugPrintPspace(handle, &data, sizeof(tvEncoder));
    if (RHDAtomBiosFunc(handle->rhdPtr, handle,
			ATOMBIOS_EXEC, &data) == ATOM_SUCCESS) {
	xf86DrvMsg(handle->scrnIndex, X_INFO, "SetTVEncoder Successful\n");
	return TRUE;
    }
    xf86DrvMsg(handle->scrnIndex, X_INFO, "SetTVEncoder Failed\n");
    return FALSE;
}

/*
 *
 */
#if (ATOM_TRANSMITTER_CONFIG_COHERENT != ATOM_TRANSMITTER_CONFIG_V2_COHERENT)
# error
#endif

Bool
rhdAtomDigTransmitterControl(atomBiosHandlePtr handle, enum atomTransmitter id,
			     enum atomTransmitterAction action, struct atomTransmitterConfig *config)
{
    DIG_TRANSMITTER_CONTROL_PARAMETERS Transmitter;
    AtomBiosArgRec data;
    char *name = NULL;
    struct atomCodeTableVersion version;

    RHDFUNC(handle);

    switch (action) {
	case atomTransDisable:
	    Transmitter.ucAction = ATOM_TRANSMITTER_ACTION_DISABLE;
	    break;
	case atomTransEnable:
	    Transmitter.ucAction = ATOM_TRANSMITTER_ACTION_ENABLE;
	    break;
	case atomTransEnableOutput:
	    Transmitter.ucAction = ATOM_TRANSMITTER_ACTION_ENABLE_OUTPUT;
	    break;
	case atomTransDisableOutput:
	    Transmitter.ucAction = ATOM_TRANSMITTER_ACTION_DISABLE_OUTPUT;
	    break;
	case atomTransLcdBlOff:
	    Transmitter.ucAction = ATOM_TRANSMITTER_ACTION_LCD_BLOFF;
	    break;
	case atomTransLcdBlOn:
	    Transmitter.ucAction = ATOM_TRANSMITTER_ACTION_LCD_BLON;
	    break;
	case atomTransLcdBlBrightness:
	    Transmitter.ucAction = ATOM_TRANSMITTER_ACTION_BL_BRIGHTNESS_CONTROL;
	    break;
	case atomTransSetup:
	    Transmitter.ucAction = ATOM_TRANSMITTER_ACTION_SETUP;
	    break;
	case atomTransInit:
	    Transmitter.ucAction = ATOM_TRANSMITTER_ACTION_INIT;
	    break;
    }

    Transmitter.ucConfig = 0;

    /* INIT is only called by ASIC_Init, for our actions this is always the PXLCLK */
    switch (config->LinkCnt) {
	case atomSingleLink:
	    Transmitter.usPixelClock = config->PixelClock * 4 / 10;
	    break;

	case atomDualLink:
	    Transmitter.usPixelClock = config->PixelClock * 2/ 10;
	    Transmitter.ucConfig |= ATOM_TRANSMITTER_CONFIG_8LANE_LINK;
	    break;
    }

    if (config->Coherent)
	Transmitter.ucConfig |= ATOM_TRANSMITTER_CONFIG_COHERENT;

    switch (id) {
	case atomTransmitterDIG1:
	case atomTransmitterUNIPHY:
	case atomTransmitterUNIPHY1:
	case atomTransmitterUNIPHY2:
	case atomTransmitterPCIEPHY:
	    data.exec.index =  GetIndexIntoMasterTable(COMMAND, UNIPHYTransmitterControl);
	    name = "UNIPHYTransmitterControl";

	    rhdAtomGetCommandTableRevisionSize(handle, data.exec.index, &version.cref, &version.fref, NULL);

	    if (version.fref > 1 || version.cref > 2)
		return FALSE;

	    switch (version.cref) {
		case 1:

	    switch (config->Link) {
		case atomTransLinkA:
		    Transmitter.ucConfig |= ATOM_TRANSMITTER_CONFIG_LINKA;
		    break;
		case atomTransLinkAB:
		    Transmitter.ucConfig |= ATOM_TRANSMITTER_CONFIG_LINKA_B;
		    break;
		case atomTransLinkB:
		    Transmitter.ucConfig |= ATOM_TRANSMITTER_CONFIG_LINKB;
		    break;
		case atomTransLinkBA:
		    Transmitter.ucConfig |= ATOM_TRANSMITTER_CONFIG_LINKB_A;
		    break;
	    }
	    switch (config->Encoder) {
		case atomEncoderDIG1:
		    Transmitter.ucConfig |= ATOM_TRANSMITTER_CONFIG_DIG1_ENCODER;
		    break;

		case atomEncoderDIG2:
		    Transmitter.ucConfig |= ATOM_TRANSMITTER_CONFIG_DIG2_ENCODER;
		    break;
		default:
		    xf86DrvMsg(handle->scrnIndex, X_ERROR,
			       "%s called with invalid encoder %x for DIG transmitter\n",
			       __func__, config->Encoder);
		    return FALSE;
	    }
	    if (id == atomTransmitterPCIEPHY) {
		switch (config->Lanes) {
		    case atomPCIELaneNONE:
			Transmitter.ucConfig |= 0;
			break;
		    case atomPCIELane0_3:
			Transmitter.ucConfig |= ATOM_TRANSMITTER_CONFIG_LANE_0_3;
			break;
		    case atomPCIELane0_7:
			Transmitter.ucConfig |= ATOM_TRANSMITTER_CONFIG_LANE_0_7;
			break;
		    case atomPCIELane4_7:
			Transmitter.ucConfig |= ATOM_TRANSMITTER_CONFIG_LANE_4_7;
			break;
		    case atomPCIELane8_11:
			Transmitter.ucConfig |= ATOM_TRANSMITTER_CONFIG_LANE_8_11;
			break;
		    case atomPCIELane8_15:
			Transmitter.ucConfig |= ATOM_TRANSMITTER_CONFIG_LANE_8_15;
			break;
		    case atomPCIELane12_15:
			Transmitter.ucConfig |= ATOM_TRANSMITTER_CONFIG_LANE_12_15;
			break;
		}
		/* According to ATI this is the only one used so far */
		Transmitter.ucConfig |= ATOM_TRANSMITTER_CONFIG_CLKSRC_PPLL;
	    }
		    break;
		case 2:
		    if (id == atomTransmitterPCIEPHY) {
			xf86DrvMsg(handle->scrnIndex, X_ERROR,
				   "%s PCIPHY not valid for DCE 3.2\n",
				   __func__);
			return FALSE;
		    }
		    switch (config->Link) {
			case atomTransLinkA:
			case atomTransLinkAB:
			    Transmitter.ucConfig |= ATOM_TRANSMITTER_CONFIG_V2_LINKA;
			    break;
			case atomTransLinkB:
			case atomTransLinkBA:
			    Transmitter.ucConfig |= ATOM_TRANSMITTER_CONFIG_V2_LINKB;
			    break;
			default:
			    xf86DrvMsg(handle->scrnIndex, X_ERROR,
				       "%s called with invalid transmitter link selection %x for DIG transmitter\n",
				       __func__, config->Link);
			    return FALSE;
		    }
		    switch (config->Encoder) {
			case atomEncoderDIG1:
			    Transmitter.ucConfig |= ATOM_TRANSMITTER_CONFIG_V2_DIG1_ENCODER;
			    break;
			case atomEncoderDIG2:
			    Transmitter.ucConfig |= ATOM_TRANSMITTER_CONFIG_V2_DIG2_ENCODER;
			    break;
			default:
			    xf86DrvMsg(handle->scrnIndex, X_ERROR,
				       "%s called with invalid encoder %x for DIG transmitter\n",
				       __func__, config->Encoder);
			    return FALSE;
		    }
		    switch (id) {
			case atomTransmitterUNIPHY:
			    Transmitter.ucConfig |= ATOM_TRANSMITTER_CONFIG_V2_TRANSMITTER1;
			    break;
			case atomTransmitterUNIPHY1:
			    Transmitter.ucConfig |= ATOM_TRANSMITTER_CONFIG_V2_TRANSMITTER2;
			    break;
			case atomTransmitterUNIPHY2:
			    Transmitter.ucConfig |= ATOM_TRANSMITTER_CONFIG_V2_TRANSMITTER3;
			    break;
			default:
			    break;
		    }

		    if (config->Mode == atomDP)
			Transmitter.ucConfig |= ATOM_TRASMITTER_CONFIG_V2_DP_CONNECTOR;
		    break;
	    }

	    break;

	case atomTransmitterLVTMA:
	case atomTransmitterDIG2:
	    data.exec.index =  GetIndexIntoMasterTable(COMMAND, DIG2TransmitterControl);
	    name = "DIG2TransmitterControl";
	    break;
    }

    data.exec.dataSpace = NULL;
    data.exec.pspace = &Transmitter;

    xf86DrvMsg(handle->scrnIndex, X_INFO, "Calling %s\n",name);
    atomDebugPrintPspace(handle, &data, sizeof(Transmitter));
    if (RHDAtomBiosFunc(handle->rhdPtr, handle,
			ATOMBIOS_EXEC, &data) == ATOM_SUCCESS) {
	xf86DrvMsg(handle->scrnIndex, X_INFO, "%s Successful\n",name);
	return TRUE;
    }
    xf86DrvMsg(handle->scrnIndex, X_INFO, "%s Failed\n",name);
    return FALSE;
}

/*
 *
 */
struct atomCodeTableVersion
rhdAtomDigTransmitterControlVersion(atomBiosHandlePtr handle)
{
    struct atomCodeTableVersion version;
    int index = GetIndexIntoMasterTable(COMMAND, UNIPHYTransmitterControl);
    rhdAtomGetCommandTableRevisionSize(handle, index, &version.cref, &version.fref, NULL);
    DEBUG_VERSION(index, handle, version);
    return version;
}

/*
 *
 */
Bool
rhdAtomOutputControl(atomBiosHandlePtr handle, enum atomOutput OutputId, enum atomOutputAction Action)
{
    AtomBiosArgRec data;
    CARD8 version;
    char *name;

    union
    {
	DISPLAY_DEVICE_OUTPUT_CONTROL_PARAMETERS op;
	DISPLAY_DEVICE_OUTPUT_CONTROL_PS_ALLOCATION opa;
    } ps;

    RHDFUNC(handle);

    switch (Action) {
	case atomOutputEnable:
	    ps.op.ucAction = ATOM_ENABLE;
	    break;
	case atomOutputDisable:
	    ps.op.ucAction = ATOM_DISABLE;
	    break;
	default: /* handle below */
	    if (OutputId != atomLCDOutput)
		return FALSE;
    }

    switch (OutputId) {
	case atomDVOOutput:
	    data.exec.index = GetIndexIntoMasterTable(COMMAND, DVOOutputControl);
	    name = "DVOOutputControl";
	    if (!rhdAtomGetCommandTableRevisionSize(handle, data.exec.index, &version, NULL, NULL))
		return FALSE;
	    switch  (version) {
		case 1:
		case 2:
		    break;
		case 3:      /* For now. This needs to be treated like DIGTransmitterControl. @@@ */
		    return FALSE;
	    }
	    break;
	case atomLCDOutput:
	    data.exec.index = GetIndexIntoMasterTable(COMMAND,  LCD1OutputControl);
	    name = "LCD1OutputControl";
	    switch (Action) {
		case atomOutputEnable:
		case atomOutputDisable:
		    break;
		case atomOutputLcdOn:
		    ps.op.ucAction = ATOM_LCD_BLON;
		    break;
		case atomOutputLcdOff:
		    ps.op.ucAction = ATOM_LCD_BLOFF;
		    break;
		case atomOutputLcdBrightnessControl:
		    ps.op.ucAction = ATOM_LCD_BL_BRIGHTNESS_CONTROL;
		    break;
		case atomOutputLcdSelftestStart:
		    ps.op.ucAction = ATOM_LCD_SELFTEST_START;
		    break;
		case atomOutputLcdSelftestStop:
		    ps.op.ucAction = ATOM_LCD_SELFTEST_STOP;
		    break;
		case atomOutputEncoderInit:
		    ps.op.ucAction = ATOM_ENCODER_INIT;
		    break;
		default:
		    return FALSE;
	    }
	    break;
	case atomCVOutput:
	    data.exec.index = GetIndexIntoMasterTable(COMMAND, CV1OutputControl);
	    name = "CV1OutputControl";
	    break;
	case atomTVOutput:
	    name = "TV1OutputControl";
	    data.exec.index = GetIndexIntoMasterTable(COMMAND, TV1OutputControl);
	    break;
	case atomLVTMAOutput:
	    name = "LVTMAOutputControl";
	    data.exec.index = GetIndexIntoMasterTable(COMMAND, LVTMAOutputControl);
	    switch (Action) {
		case atomOutputEnable:
		case atomOutputDisable:
		    break;
		case atomOutputLcdOn:
		    ps.op.ucAction = ATOM_LCD_BLON;
		    break;
		case atomOutputLcdOff:
		    ps.op.ucAction = ATOM_LCD_BLOFF;
		    break;
		case atomOutputLcdBrightnessControl:
		    ps.op.ucAction = ATOM_LCD_BL_BRIGHTNESS_CONTROL;
		    break;
		case atomOutputLcdSelftestStart:
		    ps.op.ucAction = ATOM_LCD_SELFTEST_START;
		    break;
		case atomOutputLcdSelftestStop:
		    ps.op.ucAction = ATOM_LCD_SELFTEST_STOP;
		    break;
		case atomOutputEncoderInit:
		    ps.op.ucAction = ATOM_ENCODER_INIT;
		    break;
		default:
		    return FALSE;
	    }
	    break;
	case atomTMDSAOutput:
	    name = "TMDSAOutputControl";
	    data.exec.index = GetIndexIntoMasterTable(COMMAND, TMDSAOutputControl);
	    break;
	case atomDAC1Output:
	    name = "DAC1OutputControl";
	    data.exec.index = GetIndexIntoMasterTable(COMMAND, DAC1OutputControl);
	    break;
	case atomDAC2Output:
	    name = "DAC2OutputControl";
	    data.exec.index = GetIndexIntoMasterTable(COMMAND, DAC2OutputControl);
	    break;
	default:
	    return FALSE;
    }

    data.exec.dataSpace = NULL;
    data.exec.pspace = &ps;

    xf86DrvMsg(handle->scrnIndex, X_INFO, "Calling %s\n",name);
    atomDebugPrintPspace(handle, &data, sizeof(ps));
    if (RHDAtomBiosFunc(handle->rhdPtr, handle,
			ATOMBIOS_EXEC, &data) == ATOM_SUCCESS) {
	xf86DrvMsg(handle->scrnIndex, X_INFO, "%s Successful\n",name);
	return TRUE;
    }
    xf86DrvMsg(handle->scrnIndex, X_INFO, "%s Failed\n",name);

    return FALSE;
}

/*
 *
 */
struct atomCodeTableVersion
rhdAtomOutputControlVersion(atomBiosHandlePtr handle, enum atomOutput OutputId)
{
    struct atomCodeTableVersion version = {0 , 0};
    int index;
    char *name;

    switch (OutputId) {
	case atomDVOOutput:
	    index = GetIndexIntoMasterTable(COMMAND, DVOOutputControl);
	    name = "DVOOutputControl";
	    break;
	case atomLCDOutput:
	    index = GetIndexIntoMasterTable(COMMAND,  LCD1OutputControl);
	    name = "LCD1OutputControl";
	    break;
	case atomCVOutput:
	    index = GetIndexIntoMasterTable(COMMAND, CV1OutputControl);
	    name = "CV1OutputControl";
	    break;
	case atomTVOutput:
	    index = GetIndexIntoMasterTable(COMMAND, TV1OutputControl);
	    name = "TV1OutputControl";
	    break;
	case atomLVTMAOutput:
	    index = GetIndexIntoMasterTable(COMMAND, LVTMAOutputControl);
	    name = "LVTMAOutputControl";
	    break;
	case atomTMDSAOutput:
	    index = GetIndexIntoMasterTable(COMMAND, TMDSAOutputControl);
	    name = "TMDSAOutputControl";
	    break;
	case atomDAC1Output:
	    index = GetIndexIntoMasterTable(COMMAND, DAC1OutputControl);
	    name = "DAC1OutputControl";
	    break;
	case atomDAC2Output:
	    index = GetIndexIntoMasterTable(COMMAND, DAC2OutputControl);
	    name = "DAC2OutputContro";
	    break;
	default:
	    return version;
    }

    rhdAtomGetCommandTableRevisionSize(handle, index, &version.cref, &version.fref, NULL);
    DEBUG_VERSION_NAME(index, handle, name, version);
    return version;
}

/*
 *
 */
Bool
AtomDACLoadDetection(atomBiosHandlePtr handle, enum atomDevice Device, enum atomDAC dac)
{
    AtomBiosArgRec data;
    union
    {
	DAC_LOAD_DETECTION_PARAMETERS ld;
	DAC_LOAD_DETECTION_PS_ALLOCATION lda;
    } ps;

    RHDFUNC(handle);

    data.exec.dataSpace = NULL;
    data.exec.pspace = &ps;
    data.exec.index = GetIndexIntoMasterTable(COMMAND, DAC_LoadDetection);
    ps.ld.ucMisc = 0;

    switch (Device) {
	case atomCRT1:
	    ps.ld.usDeviceID = ATOM_DEVICE_CRT1_SUPPORT;
	    break;
	case atomCRT2:
	    ps.ld.usDeviceID = ATOM_DEVICE_CRT2_SUPPORT;
	    break;
	case atomTV1:
	    ps.ld.usDeviceID = ATOM_DEVICE_TV1_SUPPORT;
	    ps.ld.ucMisc = DAC_LOAD_MISC_YPrPb;
	    break;
	case atomTV2:
	    ps.ld.usDeviceID = ATOM_DEVICE_TV2_SUPPORT;
	    ps.ld.ucMisc = DAC_LOAD_MISC_YPrPb;
	    break;
	case atomCV:
	    ps.ld.usDeviceID = ATOM_DEVICE_CV_SUPPORT;
	    break;
	case atomLCD1:
	case atomDFP1:
	case atomLCD2:
	case atomDFP2:
	case atomDFP3:
	case atomDFP4:
	case atomDFP5:
	case atomNone:
	    xf86DrvMsg(handle->scrnIndex, X_ERROR, "Unsupported device for load detection.\n");
	    return FALSE;
    }
    switch (dac) {
	case atomDACA:
	    ps.ld.ucDacType = ATOM_DAC_A;
	    break;
	case atomDACB:
	    ps.ld.ucDacType = ATOM_DAC_B;
	    break;
	case atomDACExt:
	    ps.ld.ucDacType = ATOM_EXT_DAC;
	    break;
    }

    xf86DrvMsg(handle->scrnIndex, X_INFO, "Calling DAC_LoadDetection\n");
    atomDebugPrintPspace(handle, &data, sizeof(ps));
    if (RHDAtomBiosFunc(handle->rhdPtr, handle,
			ATOMBIOS_EXEC, &data) == ATOM_SUCCESS) {
	xf86DrvMsg(handle->scrnIndex, X_INFO, "DAC_LoadDetection Successful\n");
	return TRUE;
    }
    xf86DrvMsg(handle->scrnIndex, X_INFO, "DAC_LoadDetection Failed\n");

    return FALSE;
}

/*
 *
 */
struct atomCodeTableVersion
AtomDACLoadDetectionVersion(atomBiosHandlePtr handle, enum atomDevice id)
{
    struct atomCodeTableVersion version;
    int index = GetIndexIntoMasterTable(COMMAND, DAC_LoadDetection);
    rhdAtomGetCommandTableRevisionSize(handle, index, &version.cref, &version.fref, NULL);

    DEBUG_VERSION(index, handle, version);

    return version;
}

/*
 *
 */
Bool
rhdAtomEncoderControl(atomBiosHandlePtr handle, enum atomEncoder EncoderId,
			     enum atomEncoderAction Action, struct atomEncoderConfig *Config)
{
    AtomBiosArgRec data;
    char *name = NULL;
    CARD8 version;

    union
    {
	DAC_ENCODER_CONTROL_PARAMETERS dac;
	DAC_ENCODER_CONTROL_PS_ALLOCATION dac_a;
	TV_ENCODER_CONTROL_PARAMETERS tv;
	TV_ENCODER_CONTROL_PS_ALLOCATION tv_a;
	LVDS_ENCODER_CONTROL_PARAMETERS lvds;
	LVDS_ENCODER_CONTROL_PS_ALLOCATION lvds_a;
	DIG_ENCODER_CONTROL_PARAMETERS dig;
	DIG_ENCODER_CONTROL_PS_ALLOCATION dig_a;
	EXTERNAL_ENCODER_CONTROL_PARAMETER ext;
	EXTERNAL_ENCODER_CONTROL_PS_ALLOCATION ext_a;
	DVO_ENCODER_CONTROL_PARAMETERS dvo;
	DVO_ENCODER_CONTROL_PS_ALLOCATION dvo_a;
	DVO_ENCODER_CONTROL_PARAMETERS_V3 dvo_v3;
	DVO_ENCODER_CONTROL_PS_ALLOCATION_V3 dvo_v3_a;
	LVDS_ENCODER_CONTROL_PARAMETERS_V2 lvdsv2;
	LVDS_ENCODER_CONTROL_PS_ALLOCATION_V2 lvds2_a;
	USHORT usPixelClock;
    } ps;

    RHDFUNC(handle);

    ps.usPixelClock = Config->PixelClock / 10;

    switch (EncoderId) {
	case atomEncoderDACA:
	case atomEncoderDACB:
	    if (EncoderId == atomEncoderDACA) {
		name = "DACAEncoderControl";
		data.exec.index = GetIndexIntoMasterTable(COMMAND, DAC1EncoderControl);
	    } else {
		name = "DACBEncoderControl";
		data.exec.index = GetIndexIntoMasterTable(COMMAND, DAC2EncoderControl);
	    }
	    {
		DAC_ENCODER_CONTROL_PARAMETERS *dac = &ps.dac;
		switch (Config->u.dac.DacStandard) {
		    case atomDAC_VGA:
			dac->ucDacStandard = ATOM_DAC1_PS2;
			break;
		    case atomDAC_CV:
			dac->ucDacStandard = ATOM_DAC1_CV;
			break;
		    case atomDAC_NTSC:
			dac->ucDacStandard = ATOM_DAC1_NTSC;
			break;
		    case atomDAC_PAL:
			dac->ucDacStandard = ATOM_DAC1_PAL;
			break;
		}
		switch (Action) {
		    case atomEncoderOn:
			dac->ucAction = ATOM_ENABLE;
			break;
		    case atomEncoderOff:
			dac->ucAction = ATOM_DISABLE;
			break;
		    default:
			xf86DrvMsg(handle->scrnIndex, X_ERROR, "%s: DAC unknown action\n",__func__);
			return FALSE;
		}
	    }
	    break;
	case atomEncoderTV:
	    data.exec.index = GetIndexIntoMasterTable(COMMAND, TVEncoderControl);
	    name = "TVAEncoderControl";
	    {
		TV_ENCODER_CONTROL_PARAMETERS *tv = &ps.tv;
		switch (Config->u.tv.TvStandard) {
		    case RHD_TV_NTSC:
			tv->ucTvStandard = ATOM_TV_NTSC;
			break;
		    case RHD_TV_NTSCJ:
			tv->ucTvStandard = ATOM_TV_NTSCJ;
			break;
		    case RHD_TV_PAL:
			tv->ucTvStandard = ATOM_TV_PAL;
			break;
		    case RHD_TV_PALM:
			tv->ucTvStandard = ATOM_TV_PALM;
			break;
		    case RHD_TV_PALCN:
			tv->ucTvStandard = ATOM_TV_PALCN;
			break;
		    case RHD_TV_PALN:
			tv->ucTvStandard = ATOM_TV_PALN;
			break;
		    case RHD_TV_PAL60:
			tv->ucTvStandard = ATOM_TV_PAL60;
			break;
		    case RHD_TV_SECAM:
			tv->ucTvStandard = ATOM_TV_SECAM;
			break;
		    case RHD_TV_CV:
			tv->ucTvStandard = ATOM_TV_CV;
			break;
		    case RHD_TV_NONE:
			return FALSE;
		}
		switch (Action) {
		    case atomEncoderOn:
			tv->ucAction = ATOM_ENABLE;
			break;
		    case atomEncoderOff:
			tv->ucAction = ATOM_DISABLE;
			break;
		    default:
			xf86DrvMsg(handle->scrnIndex, X_ERROR, "%s: TV unknown action\n",__func__);
			return FALSE;
		}
	    }
	    break;
	case atomEncoderTMDS1:
	case atomEncoderTMDS2:
	case atomEncoderLVDS:
	    if (EncoderId == atomEncoderLVDS) {
		name = "LVDSEncoderControl";
		data.exec.index = GetIndexIntoMasterTable(COMMAND, LVDSEncoderControl);
	    } else if (EncoderId == atomEncoderTMDS1) {
		name = "TMDSAEncoderControl";
		data.exec.index = GetIndexIntoMasterTable(COMMAND, TMDSAEncoderControl);
	    } else {
		name = "LVTMAEncoderControl";
		data.exec.index = GetIndexIntoMasterTable(COMMAND, LVTMAEncoderControl);
	    }
	    if (!rhdAtomGetCommandTableRevisionSize(handle, data.exec.index, &version, NULL, NULL))
		return FALSE;
	    switch  (version) {
		case 1:
		{
		    LVDS_ENCODER_CONTROL_PARAMETERS *lvds = &ps.lvds;
		    lvds->ucMisc = 0;
		    if (Config->u.lvds.LinkCnt == atomDualLink)
			lvds->ucMisc |= 0x1;
		    if (Config->u.lvds.Is24bit)
			lvds->ucMisc |= 0x1 << 1;

		    switch (Action) {
			case atomEncoderOn:
			    lvds->ucAction = ATOM_ENABLE;
			    break;
			case atomEncoderOff:
			    lvds->ucAction = ATOM_DISABLE;
			    break;
			default:
			    xf86DrvMsg(handle->scrnIndex, X_ERROR, "%s: LVDS unknown action\n",__func__);
			    return FALSE;
		    }
		    break;
		}
		case 2:
		case 3:
		{
		    LVDS_ENCODER_CONTROL_PARAMETERS_V2 *lvds = &ps.lvdsv2;

		    lvds->ucMisc = 0;
		    if (Config->u.lvds2.LinkCnt == atomDualLink)
			lvds->ucMisc |= PANEL_ENCODER_MISC_DUAL;
		    if (Config->u.lvds2.Coherent)
			lvds->ucMisc |= PANEL_ENCODER_MISC_COHERENT;
		    if (Config->u.lvds2.LinkB)
			lvds->ucMisc |= PANEL_ENCODER_MISC_TMDS_LINKB;
		    if (Config->u.lvds2.Hdmi)
			lvds->ucMisc |= PANEL_ENCODER_MISC_HDMI_TYPE;
		    lvds->ucTruncate = 0;
		    lvds->ucSpatial = 0;
		    lvds->ucTemporal = 0;
		    lvds->ucFRC = 0;

		    if (EncoderId == atomEncoderLVDS) {
			if (Config->u.lvds2.Is24bit) {
			    lvds->ucTruncate |= PANEL_ENCODER_TRUNCATE_DEPTH;
			    lvds->ucSpatial |= PANEL_ENCODER_SPATIAL_DITHER_DEPTH;
			    lvds->ucTemporal |= PANEL_ENCODER_TEMPORAL_DITHER_DEPTH;
			}
			switch (Config->u.lvds2.TemporalGrey) {
			    case atomTemporalDither0:
				break;
			    case atomTemporalDither4:
				lvds->ucTemporal |= PANEL_ENCODER_TEMPORAL_LEVEL_4;
			    case atomTemporalDither2:
				lvds->ucTemporal |= PANEL_ENCODER_TEMPORAL_DITHER_EN;
				break;
			}
			switch (Config->u.lvds2.SpatialDither)
			    lvds->ucSpatial |= PANEL_ENCODER_SPATIAL_DITHER_EN;
		    }

		    switch (Action) {
			case atomEncoderOn:
			    lvds->ucAction = ATOM_ENABLE;
			    break;
			case atomEncoderOff:
			    lvds->ucAction = ATOM_DISABLE;
			    break;
			default:
			    xf86DrvMsg(handle->scrnIndex, X_ERROR, "%s: LVDS2 unknown action\n",__func__);
			    return FALSE;
		    }
		    break;
		}
		default:
		    xf86DrvMsg(handle->scrnIndex, X_ERROR, "%s: LVDS unknown version\n",__func__);
		    return FALSE;
	    }
	    break;
	case atomEncoderDIG1:
	case atomEncoderDIG2:
	case atomEncoderExternal:
	{
	    DIG_ENCODER_CONTROL_PARAMETERS *dig = &ps.dig;
	    struct atomCodeTableVersion version;

	    if (EncoderId == atomEncoderDIG1) {
		name = "DIG1EncoderControl";
		data.exec.index = GetIndexIntoMasterTable(COMMAND, DIG1EncoderControl);
	    } else if (EncoderId == atomEncoderDIG2) {
		name = "DIG2EncoderControl";
		data.exec.index = GetIndexIntoMasterTable(COMMAND, DIG2EncoderControl);
	    } else {
		name = "ExternalEncoderControl";
		data.exec.index = GetIndexIntoMasterTable(COMMAND, ExternalEncoderControl);
	    }
	    rhdAtomGetCommandTableRevisionSize(handle, data.exec.index, &version.cref, &version.fref, NULL);
	    if (version.fref > 1 || version.cref > 2)
		return FALSE;

	    dig->ucConfig = 0;
	    switch (version.cref) {
		case 1:
	    switch (Config->u.dig.Link) {
		case atomTransLinkA:
		    dig->ucConfig |= ATOM_ENCODER_CONFIG_LINKA;
		    break;
		case atomTransLinkAB:
		    dig->ucConfig |= ATOM_ENCODER_CONFIG_LINKA_B;
		    break;
		case atomTransLinkB:
		    dig->ucConfig |= ATOM_ENCODER_CONFIG_LINKB;
		    break;
		case atomTransLinkBA:
		    dig->ucConfig |= ATOM_ENCODER_CONFIG_LINKB_A;
		    break;
	    }

	    if (EncoderId != atomEncoderExternal) {
		switch (Config->u.dig.Transmitter) {
		    case atomTransmitterUNIPHY:
		    case atomTransmitterPCIEPHY:
		    case atomTransmitterDIG1:
			dig->ucConfig |= ATOM_ENCODER_CONFIG_UNIPHY;
			break;
		    case atomTransmitterLVTMA:
		    case atomTransmitterDIG2:
			dig->ucConfig |= ATOM_ENCODER_CONFIG_LVTMA;
			break;
			/*
			 * these are not DCE3.0 but we need them here as DIGxEncoderControl tables for
			 * DCE3.2 still report cref 1.
			 */
			    case atomTransmitterUNIPHY1:
				dig->ucConfig |= ATOM_ENCODER_CONFIG_V2_TRANSMITTER2;
				break;
			    case atomTransmitterUNIPHY2:
				dig->ucConfig |= ATOM_ENCODER_CONFIG_V2_TRANSMITTER3;
				break;
			    default:
				xf86DrvMsg(handle->scrnIndex, X_ERROR, "%s: Invalid Transmitter for DCE3.0: %x\n",
					   __func__, Config->u.dig.Transmitter);
				return FALSE;
			}
		    }
		    break;

		case 2:
		    switch (Config->u.dig.Link) {
			case atomTransLinkA:
			case atomTransLinkAB:
			    dig->ucConfig |= ATOM_ENCODER_CONFIG_V2_LINKA;
			    break;
			case atomTransLinkB:
			case atomTransLinkBA:
			    dig->ucConfig |= ATOM_ENCODER_CONFIG_V2_LINKB;
			    break;
		    }
		    switch (Config->u.dig.Transmitter) {
			case atomTransmitterUNIPHY:
			    dig->ucConfig |= ATOM_ENCODER_CONFIG_UNIPHY;
			    break;
			case atomTransmitterUNIPHY1:
			    dig->ucConfig |= ATOM_ENCODER_CONFIG_V2_TRANSMITTER2;
			    break;
			case atomTransmitterUNIPHY2:
			    dig->ucConfig |= ATOM_ENCODER_CONFIG_V2_TRANSMITTER3;
			    break;
			default:
			    xf86DrvMsg(handle->scrnIndex, X_ERROR, "%s: Invalid Encoder for DCE3.2: %x\n",
				       __func__, Config->u.dig.Transmitter);
			    return FALSE;
		}
		    break;

		default:
		    return FALSE;
	    }

	    switch (Config->u.dig.EncoderMode) {
		case atomDVI:
		    dig->ucEncoderMode = ATOM_ENCODER_MODE_DVI;
		    break;
		case atomDP:
		    dig->ucEncoderMode = ATOM_ENCODER_MODE_DP;
		    break;
		case atomLVDS:
		    dig->ucEncoderMode = ATOM_ENCODER_MODE_LVDS;
		    break;
		case atomHDMI:
		    dig->ucEncoderMode = ATOM_ENCODER_MODE_HDMI;
		    break;
		case atomSDVO:
		    dig->ucEncoderMode = ATOM_ENCODER_MODE_SDVO;
		    break;
		case atomNoEncoder:
		case atomTVComposite:
		case atomTVSVideo:
		case atomTVComponent:
		case atomCRT:
		    xf86DrvMsg(handle->scrnIndex, X_ERROR, "%s called with invalid DIG encoder mode %i\n",
			       __func__,Config->u.dig.EncoderMode);
		    return FALSE;
		    break;
	    }

	    switch (Action) {
		case atomEncoderOn:
		    dig->ucAction = ATOM_ENABLE;
		    break;
		case atomEncoderOff:
		    dig->ucAction = ATOM_DISABLE;
		    break;
		default:
		    xf86DrvMsg(handle->scrnIndex, X_ERROR, "%s: DIG unknown action\n",__func__);
		    return FALSE;
	    }

	    switch (Config->u.dig.LinkCnt) {
		case atomSingleLink:
		    dig->ucLaneNum = 4;
		    break;
		case atomDualLink:
		    dig->ucLaneNum = 8;
		    break;
	    }
	    break;
	case atomEncoderDVO:
	    name = "DVOEncoderControl";
	    data.exec.index = GetIndexIntoMasterTable(COMMAND, DVOEncoderControl);
	    if (!rhdAtomGetCommandTableRevisionSize(handle, data.exec.index, &version.cref, NULL, NULL))
		return FALSE;
	    switch  (version.cref) {
		case 1:
		case 2:
		{
		    DVO_ENCODER_CONTROL_PARAMETERS *dvo = &ps.dvo;
		    dvo->usEncoderID = Config->u.dvo.EncoderID;
		    switch (Config->u.dvo.DvoDeviceType) {
			case atomLCD1:
			case atomLCD2:
			    dvo->ucDeviceType = ATOM_DEVICE_LCD1_INDEX;
			    break;
			case atomCRT1:
			case atomCRT2:
			    dvo->ucDeviceType = ATOM_DEVICE_CRT1_INDEX;
			    break;
			case atomDFP1:
			case atomDFP2:
			case atomDFP3:
			case atomDFP4:
			case atomDFP5:
			    dvo->ucDeviceType = ATOM_DEVICE_DFP1_INDEX;
			    break;
			case atomTV1:
			case atomTV2:
			    dvo->ucDeviceType = ATOM_DEVICE_TV1_INDEX;
			    break;
			case atomCV:
			    dvo->ucDeviceType = ATOM_DEVICE_CV_INDEX;
			    break;
			case atomNone:
			    return FALSE;
		    }
		    if (Config->u.dvo.digital) {
			dvo->usDevAttr.sDigAttrib.ucAttribute = 0; /* @@@ What do these attributes mean? */
		    } else {
			switch (Config->u.dvo.u.TVMode) {
			    case RHD_TV_NTSC:
				dvo->usDevAttr.sAlgAttrib.ucTVStandard = ATOM_TV_NTSC;
				break;
			    case RHD_TV_NTSCJ:
				dvo->usDevAttr.sAlgAttrib.ucTVStandard = ATOM_TV_NTSCJ;
				break;
			    case RHD_TV_PAL:
				dvo->usDevAttr.sAlgAttrib.ucTVStandard = ATOM_TV_PAL;
				break;
			    case RHD_TV_PALM:
				dvo->usDevAttr.sAlgAttrib.ucTVStandard = ATOM_TV_PALM;
				break;
			    case RHD_TV_PALCN:
				dvo->usDevAttr.sAlgAttrib.ucTVStandard = ATOM_TV_PALCN;
				break;
			    case RHD_TV_PALN:
				dvo->usDevAttr.sAlgAttrib.ucTVStandard = ATOM_TV_PALN;
				break;
			    case RHD_TV_PAL60:
				dvo->usDevAttr.sAlgAttrib.ucTVStandard = ATOM_TV_PAL60;
				break;
			    case RHD_TV_SECAM:
				dvo->usDevAttr.sAlgAttrib.ucTVStandard = ATOM_TV_SECAM;
				break;
			    case RHD_TV_CV:
				dvo->usDevAttr.sAlgAttrib.ucTVStandard = ATOM_TV_CV;
				break;
			    case RHD_TV_NONE:
				return FALSE;
			}
		    }
		    switch (Action) {
			case atomEncoderOn:
			    dvo->ucAction = ATOM_ENABLE;
			    break;
			case atomEncoderOff:
			    dvo->ucAction = ATOM_DISABLE;
			    break;
			default:
			    xf86DrvMsg(handle->scrnIndex, X_ERROR, "%s: DVO unknown action\n",__func__);
			    return FALSE;
		    }
		break;
		}
		case 3:
		{
		    DVO_ENCODER_CONTROL_PARAMETERS_V3 *dvo = &ps.dvo_v3;
		    dvo->ucDVOConfig = 0;
		    if (Config->u.dvo3.Rate == atomDVO_RateSDR)
			dvo->ucDVOConfig |= DVO_ENCODER_CONFIG_SDR_SPEED;
		    else
			dvo->ucDVOConfig |= DVO_ENCODER_CONFIG_DDR_SPEED;
		    switch (Config->u.dvo3.DvoOutput) {
			case atomDVO_OutputLow12Bit:
			    dvo->ucDVOConfig = DVO_ENCODER_CONFIG_LOW12BIT;
			    break;
			case atomDVO_OutputHigh12Bit:
			    dvo->ucDVOConfig = DVO_ENCODER_CONFIG_UPPER12BIT;
			    break;
			case atomDVO_Output24Bit:
			    dvo->ucDVOConfig = DVO_ENCODER_CONFIG_24BIT;
			    break;
		    }
		    switch (Action) {
			case atomEncoderOn:
			    dvo->ucAction = ATOM_ENABLE;
			    break;
			case atomEncoderOff:
			    dvo->ucAction = ATOM_DISABLE;
			    break;
			default:
			    xf86DrvMsg(handle->scrnIndex, X_ERROR, "%s: DVO3 unknown action\n",__func__);
			    return FALSE;
		    }
		    break;
		}
	    }
	    break;
	}
	case atomEncoderNone:
	    return FALSE;
    }

    data.exec.dataSpace = NULL;
    data.exec.pspace = &ps;

    xf86DrvMsg(handle->scrnIndex, X_INFO, "Calling %s\n",name);
    atomDebugPrintPspace(handle, &data, sizeof(ps));
    if (RHDAtomBiosFunc(handle->rhdPtr, handle,
			ATOMBIOS_EXEC, &data) == ATOM_SUCCESS) {
	xf86DrvMsg(handle->scrnIndex, X_INFO, "%s Successful\n",name);
	return TRUE;
    }
    xf86DrvMsg(handle->scrnIndex, X_INFO, "%s Failed\n",name);
    return FALSE;
}

/*
 *
 */
struct atomCodeTableVersion
rhdAtomEncoderControlVersion(atomBiosHandlePtr handle, enum atomEncoder EncoderId)
{
    struct atomCodeTableVersion version = { 0, 0 };
    int index;
    char *name;

    switch (EncoderId) {
	case atomEncoderDACA:
	    index = GetIndexIntoMasterTable(COMMAND, DAC1EncoderControl);
	    name = "DAC1EncoderControl";
	    break;
	case atomEncoderDACB:
	    index = GetIndexIntoMasterTable(COMMAND, DAC2EncoderControl);
	    name = "DAC2EncoderControl";
	    break;
	case atomEncoderTV:
	    index = GetIndexIntoMasterTable(COMMAND, TVEncoderControl);
	    name = "TVEncoderControl";
	    break;
	case atomEncoderTMDS1:
	case atomEncoderTMDS2:
	    index = GetIndexIntoMasterTable(COMMAND, TMDSAEncoderControl);
	    name = "TMDSAEncoderControl";
	    break;
	case atomEncoderLVDS:
	    index = GetIndexIntoMasterTable(COMMAND, LVDSEncoderControl);
	    name = " LVDSEncoderControl";
	    break;
	case atomEncoderDIG1:
	    index = GetIndexIntoMasterTable(COMMAND, DIG1EncoderControl);
	    name = "DIG1EncoderControl";
	    break;
	case atomEncoderDIG2:
	    index = GetIndexIntoMasterTable(COMMAND, DIG2EncoderControl);
	    name = "DIG2EncoderControl";
	    break;
	case atomEncoderExternal:
	    index = GetIndexIntoMasterTable(COMMAND, ExternalEncoderControl);
	    name = "ExternalEncoderControl";
	    break;
	case atomEncoderDVO:
	    index = GetIndexIntoMasterTable(COMMAND, DVOEncoderControl);
	    name = "DVOEncoderControl";
	    break;
	default:
	    return version;
    }

    rhdAtomGetCommandTableRevisionSize(handle, index, &version.cref, &version.fref, NULL);

    DEBUG_VERSION_NAME(index, handle, name, version);

    return version;
}

/*
 *
 */
Bool
rhdAtomUpdateCRTC_DoubleBufferRegisters(atomBiosHandlePtr handle, enum atomCrtc CrtcId,
					enum atomCrtcAction Action)
{
    AtomBiosArgRec data;
    union
    {
	ENABLE_CRTC_PARAMETERS crtc;
	ENABLE_CRTC_PS_ALLOCATION crtc_a;
    } ps;

    RHDFUNC(handle);

    switch (CrtcId) {
	case atomCrtc1:
	    ps.crtc.ucCRTC = ATOM_CRTC1;
	    break;
	case atomCrtc2:
	    ps.crtc.ucCRTC = ATOM_CRTC2;
	    break;
    }

    switch (Action) {
	case atomCrtcEnable:
	    ps.crtc.ucEnable = ATOM_ENABLE;
	    break;
	case atomCrtcDisable:
	    ps.crtc.ucEnable = ATOM_DISABLE;
	    break;
    }

    data.exec.index = GetIndexIntoMasterTable(COMMAND, UpdateCRTC_DoubleBufferRegisters);

    data.exec.dataSpace = NULL;
    data.exec.pspace = &ps;

    xf86DrvMsg(handle->scrnIndex, X_INFO, "Calling UpdateCRTC_DoubleBufferRegisters\n");
    atomDebugPrintPspace(handle, &data, sizeof(ps));
    if (RHDAtomBiosFunc(handle->rhdPtr, handle,
			ATOMBIOS_EXEC, &data) == ATOM_SUCCESS) {
	xf86DrvMsg(handle->scrnIndex, X_INFO, "UpdateCRTC_DoubleBufferRegisters Successful\n");
	return TRUE;
    }
    xf86DrvMsg(handle->scrnIndex, X_INFO, "UpdateCRTC_DoubleBufferRegisters Failed\n");
    return FALSE;
}

/*
 *
 */
struct atomCodeTableVersion
rhdAtomUpdateCRTC_DoubleBufferRegistersVersion(atomBiosHandlePtr handle)
{
    struct atomCodeTableVersion version;
    int index = GetIndexIntoMasterTable(COMMAND, UpdateCRTC_DoubleBufferRegisters);
    rhdAtomGetCommandTableRevisionSize(handle, index, &version.cref, &version.fref, NULL);

    DEBUG_VERSION(index, handle, version);

    return version;
}

/*
 *
 */
Bool
rhdAtomEnableCrtc(atomBiosHandlePtr handle, enum atomCrtc CrtcId,
		  enum atomCrtcAction Action)
{
    AtomBiosArgRec data;
    union
    {
	ENABLE_CRTC_PARAMETERS crtc;
	ENABLE_CRTC_PS_ALLOCATION crtc_a;
    } ps;

    RHDFUNC(handle);

    switch (CrtcId) {
	case atomCrtc1:
	    ps.crtc.ucCRTC = ATOM_CRTC1;
	    break;
	case atomCrtc2:
	    ps.crtc.ucCRTC = ATOM_CRTC2;
	    break;
    }

    switch (Action) {
	case atomCrtcEnable:
	    ps.crtc.ucEnable = ATOM_ENABLE;
	    break;
	case atomCrtcDisable:
	    ps.crtc.ucEnable = ATOM_DISABLE;
	    break;
    }

    data.exec.index = GetIndexIntoMasterTable(COMMAND, EnableCRTC);

    data.exec.dataSpace = NULL;
    data.exec.pspace = &ps;

    xf86DrvMsg(handle->scrnIndex, X_INFO, "Calling EnableCRTC\n");
    atomDebugPrintPspace(handle, &data, sizeof(ps));
    if (RHDAtomBiosFunc(handle->rhdPtr, handle,
			ATOMBIOS_EXEC, &data) == ATOM_SUCCESS) {
	xf86DrvMsg(handle->scrnIndex, X_INFO, "EnableCRTC Successful\n");
	return TRUE;
    }
    xf86DrvMsg(handle->scrnIndex, X_INFO, "EnableCRTC Failed\n");
    return FALSE;
}

/*
 *
 */
struct atomCodeTableVersion
rhdAtomEnableCrtcVersion(atomBiosHandlePtr handle)
{
    struct atomCodeTableVersion version;
    int index = GetIndexIntoMasterTable(COMMAND,  EnableCRTC);
    rhdAtomGetCommandTableRevisionSize(handle, index, &version.cref, &version.fref, NULL);

    DEBUG_VERSION(index, handle, version);

    return version;
}

/*
 *
 */
Bool
rhdAtomEnableCrtcMemReq(atomBiosHandlePtr handle, enum atomCrtc CrtcId,
		  enum atomCrtcAction Action)
{
    AtomBiosArgRec data;
    union
    {
	ENABLE_CRTC_PARAMETERS crtc;
	ENABLE_CRTC_PS_ALLOCATION crtc_a;
    } ps;

    RHDFUNC(handle);

    switch (CrtcId) {
	case atomCrtc1:
	    ps.crtc.ucCRTC = ATOM_CRTC1;
	    break;
	case atomCrtc2:
	    ps.crtc.ucCRTC = ATOM_CRTC2;
	    break;
    }

    switch (Action) {
	case atomCrtcEnable:
	    ps.crtc.ucEnable = ATOM_ENABLE;
	    break;
	case atomCrtcDisable:
	    ps.crtc.ucEnable = ATOM_DISABLE;
	    break;
    }

    data.exec.index = GetIndexIntoMasterTable(COMMAND, EnableCRTCMemReq);

    data.exec.dataSpace = NULL;
    data.exec.pspace = &ps;

    xf86DrvMsg(handle->scrnIndex, X_INFO, "Calling EnableCRTCMemReq\n");
    atomDebugPrintPspace(handle, &data, sizeof(ps));
    if (RHDAtomBiosFunc(handle->rhdPtr, handle,
			ATOMBIOS_EXEC, &data) == ATOM_SUCCESS) {
	xf86DrvMsg(handle->scrnIndex, X_INFO, "EnableCRTCMemReq Successful\n");
	return TRUE;
    }
    xf86DrvMsg(handle->scrnIndex, X_INFO, "EnableCRTCMemReq Failed\n");
    return FALSE;
}

/*
 *
 */
struct atomCodeTableVersion
rhdAtomEnableCrtcMemReqVersion(atomBiosHandlePtr handle)
{
    struct atomCodeTableVersion version;
    int index = GetIndexIntoMasterTable(COMMAND, EnableCRTCMemReq);
    rhdAtomGetCommandTableRevisionSize(handle, index, &version.cref, &version.fref, NULL);

    DEBUG_VERSION(index, handle, version);

    return version;

}

/*
 *
 */
Bool
rhdAtomSetCRTCTimings(atomBiosHandlePtr handle, enum atomCrtc id, DisplayModePtr mode, int depth)
{
    AtomBiosArgRec data;
    union
    {
	SET_CRTC_TIMING_PARAMETERS  crtc;
/* 	SET_CRTC_TIMING_PS_ALLOCATION crtc_a; */
    } ps;
    ATOM_MODE_MISC_INFO_ACCESS* msc = &(ps.crtc.susModeMiscInfo);

    RHDFUNC(handle);

    ps.crtc.usH_Total = mode->CrtcHTotal;
    ps.crtc.usH_Disp = mode->CrtcHDisplay;
    ps.crtc.usH_SyncStart = mode->CrtcHSyncStart;
    ps.crtc.usH_SyncWidth = mode->CrtcHSyncEnd - mode->CrtcHSyncStart;
    ps.crtc.usV_Total = mode->CrtcVTotal;
    ps.crtc.usV_Disp = mode->CrtcVDisplay;
    ps.crtc.usV_SyncStart = mode->CrtcVSyncStart;
    ps.crtc.usV_SyncWidth = mode->CrtcVSyncEnd - mode->CrtcVSyncStart;
    ps.crtc.ucOverscanRight = mode->CrtcHBlankStart - mode->CrtcHDisplay;
    ps.crtc.ucOverscanLeft = mode->CrtcVTotal - mode->CrtcVBlankEnd;
    ps.crtc.ucOverscanBottom = mode->CrtcVBlankStart - mode->CrtcVDisplay;
    ps.crtc.ucOverscanTop = mode->CrtcVTotal - mode->CrtcVBlankEnd;
    switch (id) {
	case atomCrtc1:
	    ps.crtc.ucCRTC = ATOM_CRTC1;
	    break;
	case atomCrtc2:
	    ps.crtc.ucCRTC = ATOM_CRTC2;
	    break;
    }

    msc->sbfAccess.HorizontalCutOff = 0;
    msc->sbfAccess.HSyncPolarity = (mode->Flags & V_NHSYNC) ? 1 : 0;
    msc->sbfAccess.VSyncPolarity = (mode->Flags & V_NVSYNC) ? 1 : 0;
    msc->sbfAccess.VerticalCutOff = 0;
    msc->sbfAccess.H_ReplicationBy2 = 0;
    msc->sbfAccess.V_ReplicationBy2 = (mode->Flags & V_DBLSCAN) ? 1 : 0;
    msc->sbfAccess.CompositeSync =  (mode->Flags & V_CSYNC);
    msc->sbfAccess.Interlace = (mode->Flags & V_INTERLACE) ? 1 : 0;
    msc->sbfAccess.DoubleClock = (mode->Flags & V_DBLCLK) ? 1 : 0;
    msc->sbfAccess.RGB888 = (depth == 24) ? 1 : 0;

    data.exec.index = GetIndexIntoMasterTable(COMMAND, SetCRTC_Timing);

    data.exec.dataSpace = NULL;
    data.exec.pspace = &ps;

    xf86DrvMsg(handle->scrnIndex, X_INFO, "Calling SetCRTC_Timing\n");
    atomDebugPrintPspace(handle, &data, sizeof(ps));
    if (RHDAtomBiosFunc(handle->rhdPtr, handle,
			ATOMBIOS_EXEC, &data) == ATOM_SUCCESS) {
	xf86DrvMsg(handle->scrnIndex, X_INFO, "SetCRTC_Timing Successful\n");
	return TRUE;
  }
    xf86DrvMsg(handle->scrnIndex, X_INFO, "SetCRTC_Timing Failed\n");
    return FALSE;
}

/*
 *
 */
struct atomCodeTableVersion
rhdAtomSetCRTCTimingsVersion(atomBiosHandlePtr handle)
{
    struct atomCodeTableVersion version;
    int index = GetIndexIntoMasterTable(COMMAND, SetCRTC_Timing);
    rhdAtomGetCommandTableRevisionSize(handle, index, &version.cref, &version.fref, NULL);

    DEBUG_VERSION(index, handle, version);
    return version;

}

/*
 *
 */
Bool
rhdAtomSetCRTCOverscan(atomBiosHandlePtr handle, enum atomCrtc id, struct atomCrtcOverscan *config)
{
    AtomBiosArgRec data;
    union
    {
	SET_CRTC_OVERSCAN_PARAMETERS  ovscn;
	SET_CRTC_OVERSCAN_PS_ALLOCATION ovscn_a;
    } ps;

    RHDFUNC(handle);

    data.exec.index = GetIndexIntoMasterTable(COMMAND, SetCRTC_OverScan);
    data.exec.dataSpace = NULL;
    data.exec.pspace = &ps;

    switch(id) {
	case atomCrtc1:
	    ps.ovscn.ucCRTC = ATOM_CRTC1;
	    break;
	case atomCrtc2:
	    ps.ovscn.ucCRTC = ATOM_CRTC2;
	    break;
    }
    ps.ovscn.usOverscanRight = config->ovscnRight;
    ps.ovscn.usOverscanLeft = config->ovscnLeft;
    ps.ovscn.usOverscanBottom = config->ovscnBottom;
    ps.ovscn.usOverscanTop = config->ovscnTop;

    xf86DrvMsg(handle->scrnIndex, X_INFO, "CallingSetCRTC_OverScan\n");
    atomDebugPrintPspace(handle, &data, sizeof(ps));
    if (RHDAtomBiosFunc(handle->rhdPtr, handle,
			ATOMBIOS_EXEC, &data) == ATOM_SUCCESS) {
	xf86DrvMsg(handle->scrnIndex, X_INFO, "Set CRTC_OverScan Successful\n");
	return TRUE;
    }
    xf86DrvMsg(handle->scrnIndex, X_INFO, "SetCRTC_OverScan Failed\n");
    return FALSE;
}

/*
 *
 */
struct atomCodeTableVersion
rhdAtomSetCRTCOverscanVersion(atomBiosHandlePtr handle)
{
    struct atomCodeTableVersion version;
    int index = GetIndexIntoMasterTable(COMMAND, SetCRTC_OverScan);
    rhdAtomGetCommandTableRevisionSize(handle, index, &version.cref, &version.fref, NULL);

    DEBUG_VERSION(index, handle, version);
    return version;
}

/*
 *
 */
Bool
rhdAtomBlankCRTC(atomBiosHandlePtr handle, enum atomCrtc id, struct atomCrtcBlank *config)
{
  AtomBiosArgRec data;
    union
    {
	BLANK_CRTC_PARAMETERS blank;
	BLANK_CRTC_PS_ALLOCATION blank_a;
    } ps;

    RHDFUNC(handle);

    data.exec.index = GetIndexIntoMasterTable(COMMAND, BlankCRTC);
    data.exec.pspace = &ps;
    data.exec.dataSpace = NULL;

    switch(id) {
	case atomCrtc1:
	    ps.blank.ucCRTC = ATOM_CRTC1;
	    break;
	case atomCrtc2:
	    ps.blank.ucCRTC = ATOM_CRTC2;
	    break;
    }

    switch (config->Action) {
	case atomBlankOn:
	    ps.blank.ucBlanking = ATOM_BLANKING;
	    break;
	case atomBlankOff:
	    ps.blank.ucBlanking = ATOM_BLANKING_OFF;
	    break;
    }
    ps.blank.usBlackColorRCr = config->r;
    ps.blank.usBlackColorGY = config->g;
    ps.blank.usBlackColorBCb = config->b;

    xf86DrvMsg(handle->scrnIndex, X_INFO, "Calling BlankCRTC\n");
    atomDebugPrintPspace(handle, &data, sizeof(ps));
    if (RHDAtomBiosFunc(handle->rhdPtr, handle,
			ATOMBIOS_EXEC, &data) == ATOM_SUCCESS) {
	xf86DrvMsg(handle->scrnIndex, X_INFO, "BlankCRTC Successful\n");
	return TRUE;
    }
    xf86DrvMsg(handle->scrnIndex, X_INFO, "BlankCRTC Failed\n");
  return FALSE;
}

/*
 *
 */
struct atomCodeTableVersion
rhdAtomBlankCRTCVersion(atomBiosHandlePtr handle)
{
    struct atomCodeTableVersion version;
    int index = GetIndexIntoMasterTable(COMMAND, BlankCRTC);
    rhdAtomGetCommandTableRevisionSize(handle, index, &version.cref, &version.fref, NULL);

    DEBUG_VERSION(index, handle, version);
    return version;
}

/*
 *
 */
static int
atomGetDevice(atomBiosHandlePtr handle, enum atomDevice Device)
{
    switch (Device) {
	case atomCRT1:
	    return ATOM_DEVICE_CRT1_INDEX;
	case atomLCD1:
	    return ATOM_DEVICE_LCD1_INDEX;
	case atomTV1:
	    return ATOM_DEVICE_TV1_INDEX;
	case atomDFP1:
	    return ATOM_DEVICE_DFP1_INDEX;
	case atomCRT2:
	    return ATOM_DEVICE_CRT2_INDEX;
	case atomLCD2:
	    return ATOM_DEVICE_LCD2_INDEX;
	case atomTV2:
	    return ATOM_DEVICE_TV2_INDEX;
	case atomDFP2:
	    return ATOM_DEVICE_DFP2_INDEX;
	case atomCV:
	    return ATOM_DEVICE_CV_INDEX;
	case atomDFP3:
	    return ATOM_DEVICE_DFP3_INDEX;
	case atomDFP4:
	    return ATOM_DEVICE_DFP4_INDEX;
	case atomDFP5:
	    return ATOM_DEVICE_DFP5_INDEX;
	case atomNone:
	    xf86DrvMsg(handle->scrnIndex, X_ERROR, "Invalid Device\n");
	    return ATOM_MAX_SUPPORTED_DEVICE;
    }

    return ATOM_MAX_SUPPORTED_DEVICE;
}

/*
 *
 */
Bool
rhdAtomSetPixelClock(atomBiosHandlePtr handle, enum atomPxclk PCLKId, struct atomPixelClockConfig *Config)
{
    AtomBiosArgRec data;
    CARD8 version;
    Bool NeedMode = FALSE;
    union {
	PIXEL_CLOCK_PARAMETERS  pclk;
	PIXEL_CLOCK_PARAMETERS_V2  pclk_v2;
	PIXEL_CLOCK_PARAMETERS_V3  pclk_v3;
	SET_PIXEL_CLOCK_PS_ALLOCATION pclk_a;
    } ps;

    data.exec.index = GetIndexIntoMasterTable(COMMAND, SetPixelClock);

    if (!rhdAtomGetCommandTableRevisionSize(handle, data.exec.index, &version, NULL, NULL))
	return FALSE;
    switch  (version) {
	case 1:
	    if (Config->Enable)
		ps.pclk.usPixelClock = Config->PixelClock / 10;
	    else
		ps.pclk.usPixelClock = 0;
	    ps.pclk.usRefDiv = Config->RefDiv;
	    ps.pclk.usFbDiv = Config->FbDiv;
	    ps.pclk.ucPostDiv = Config->PostDiv;
	    ps.pclk.ucFracFbDiv = Config->FracFbDiv;
	    ps.pclk.ucRefDivSrc = 0; /* What's this? @@@ */
	    switch (PCLKId) {
		case atomPclk1:
		    ps.pclk.ucPpll = ATOM_PPLL1;
		    break;
		case atomPclk2:
		    ps.pclk.ucPpll = ATOM_PPLL2;
		    break;
	    }
	    switch (Config->Crtc) {
		case atomCrtc1:
		    ps.pclk.ucCRTC = ATOM_CRTC1;
		    break;
		case atomCrtc2:
		    ps.pclk.ucCRTC = ATOM_CRTC2;
		    break;
	    }
	    break;
	case 2:
	    if (Config->Enable)
		ps.pclk_v2.usPixelClock = Config->PixelClock / 10;
	    else
		ps.pclk_v2.usPixelClock = 0;
	    ps.pclk_v2.usRefDiv = Config->RefDiv;
	    ps.pclk_v2.usFbDiv = Config->FbDiv;
	    ps.pclk_v2.ucPostDiv = Config->PostDiv;
	    ps.pclk_v2.ucFracFbDiv = Config->FracFbDiv;
	    switch (PCLKId) {
		case atomPclk1:
		    ps.pclk_v2.ucPpll = ATOM_PPLL1;
		    break;
		case atomPclk2:
		    ps.pclk_v2.ucPpll = ATOM_PPLL2;
		    break;
	    }
	    ps.pclk_v2.ucRefDivSrc = 1; /* See above... @@@ */
	    switch (Config->Crtc) {
		case atomCrtc1:
		    ps.pclk_v2.ucCRTC = ATOM_CRTC1;
		    break;
		case atomCrtc2:
		    ps.pclk_v2.ucCRTC = ATOM_CRTC2;
		    break;
	    }
	    ASSERTF((!Config->Enable || Config->u.v2.Device != atomNone), "Invalid Device Id\n");
	    ps.pclk_v2.ucMiscInfo = 0;
	    ps.pclk_v2.ucMiscInfo |= (Config->u.v2.Force ? MISC_FORCE_REPROG_PIXEL_CLOCK : 0);
	    if (Config->u.v2.Device != atomNone)
		ps.pclk_v2.ucMiscInfo |= (atomGetDevice(handle, Config->u.v2.Device)
					  << MISC_DEVICE_INDEX_SHIFT);
	    RHDDebug(handle->scrnIndex,"%s Device: %i PixelClock: %i RefDiv: 0x%x FbDiv: 0x%x PostDiv: 0x%x "
		     "PLL: %i Crtc: %i MiscInfo: 0x%x\n",
		   __func__,
		   Config->u.v2.Device,
		   ps.pclk_v2.usPixelClock,
		   ps.pclk_v2.usRefDiv,
		   ps.pclk_v2.usFbDiv,
		   ps.pclk_v2.ucPostDiv,
		   ps.pclk_v2.ucPpll,
		   ps.pclk_v2.ucCRTC,
		   ps.pclk_v2.ucMiscInfo
		);
	    break;
	case 3:
	    if (Config->Enable)
		ps.pclk_v3.usPixelClock = Config->PixelClock / 10;
	    else
		ps.pclk.usPixelClock = 0;
	    ps.pclk_v3.usRefDiv = Config->RefDiv;
	    ps.pclk_v3.usFbDiv = Config->FbDiv;
	    ps.pclk_v3.ucPostDiv = Config->PostDiv;
	    ps.pclk_v3.ucFracFbDiv = Config->FracFbDiv;
	    switch (PCLKId) {
		case atomPclk1:
		    ps.pclk_v3.ucPpll = ATOM_PPLL1;
		    break;
		case atomPclk2:
		    ps.pclk_v3.ucPpll = ATOM_PPLL2;
		    break;
	    }
	    switch (Config->u.v3.OutputType) {
		case atomOutputKldskpLvtma:
		    ps.pclk_v3.ucTransmitterId = ENCODER_OBJECT_ID_INTERNAL_KLDSCP_LVTMA;
		    NeedMode = TRUE;
		    break;
		case atomOutputUniphyA:
		case atomOutputUniphyB:
		    ps.pclk_v3.ucTransmitterId = ENCODER_OBJECT_ID_INTERNAL_UNIPHY;
		    NeedMode = TRUE;
		    break;
		case atomOutputUniphyC:
		case atomOutputUniphyD:
		    ps.pclk_v3.ucTransmitterId = ENCODER_OBJECT_ID_INTERNAL_UNIPHY1;
		    NeedMode = TRUE;
		    break;
		case atomOutputUniphyE:
		case atomOutputUniphyF:
		    ps.pclk_v3.ucTransmitterId = ENCODER_OBJECT_ID_INTERNAL_UNIPHY2;
		    NeedMode = TRUE;
		    break;

		case atomOutputDacA:
		    ps.pclk_v3.ucTransmitterId = ENCODER_OBJECT_ID_INTERNAL_KLDSCP_DAC1;
		    break;
		case atomOutputDacB:
		    ps.pclk_v3.ucTransmitterId = ENCODER_OBJECT_ID_INTERNAL_KLDSCP_DAC2;
		    break;
		case atomOutputDvo:
		    ps.pclk_v3.ucTransmitterId = ENCODER_OBJECT_ID_INTERNAL_KLDSCP_DVO1;
		    NeedMode = TRUE;
		    break;
		case atomOutputTmdsa:
		case atomOutputLvtma:
		case atomOutputNone:
		    return FALSE;
	    }
	    if (NeedMode) {
		switch (Config->u.v3.EncoderMode) {
		    case atomNoEncoder:
			ps.pclk_v3.ucEncoderMode = 0;
		    case atomDVI:
			ps.pclk_v3.ucEncoderMode = ATOM_ENCODER_MODE_DVI;
			break;
		    case atomDP:
			ps.pclk_v3.ucEncoderMode = ATOM_ENCODER_MODE_DP;
			break;
		    case atomLVDS:
			ps.pclk_v3.ucEncoderMode = ATOM_ENCODER_MODE_LVDS;
			break;
		    case atomHDMI:
			ps.pclk_v3.ucEncoderMode = ATOM_ENCODER_MODE_HDMI;
			break;
		    case atomSDVO:
			ps.pclk_v3.ucEncoderMode = ATOM_ENCODER_MODE_SDVO;
			break;
		    default:
			xf86DrvMsg(handle->scrnIndex, X_ERROR,"%s: invalid encoder type.\n",__func__);
			return FALSE;
		}
	    }
	    ps.pclk_v3.ucMiscInfo = (Config->u.v3.Force ? PIXEL_CLOCK_MISC_FORCE_PROG_PPLL : 0x0)
		| (Config->u.v3.UsePpll ?  PIXEL_CLOCK_MISC_USE_ENGINE_FOR_DISPCLK : 0x0)
		| ((Config->Crtc == atomCrtc2) ? PIXEL_CLOCK_MISC_CRTC_SEL_CRTC2 : PIXEL_CLOCK_MISC_CRTC_SEL_CRTC1);

	    RHDDebug(handle->scrnIndex,"%s PixelClock: %i RefDiv: 0x%x FbDiv: 0x%x PostDiv: 0x%x PLL: %i OutputType: %x "
		   "EncoderMode: %x MiscInfo: 0x%x\n",
		   __func__,
		   ps.pclk_v3.usPixelClock,
		   ps.pclk_v3.usRefDiv,
		   ps.pclk_v3.usFbDiv,
		   ps.pclk_v3.ucPostDiv,
		   ps.pclk_v3.ucPpll,
		   ps.pclk_v3.ucTransmitterId,
		   ps.pclk_v3.ucEncoderMode,
		   ps.pclk_v3.ucMiscInfo
		);
	    break;
	default:
	    return FALSE;
    }

    data.exec.dataSpace = NULL;
    data.exec.pspace = &ps;

    xf86DrvMsg(handle->scrnIndex, X_INFO, "Calling SetPixelClock\n");
    atomDebugPrintPspace(handle, &data, sizeof(ps));
    if (RHDAtomBiosFunc(handle->rhdPtr, handle,
			ATOMBIOS_EXEC, &data) == ATOM_SUCCESS) {
	xf86DrvMsg(handle->scrnIndex, X_INFO, "SetPixelClock Successful\n");
	return TRUE;
    }
    xf86DrvMsg(handle->scrnIndex, X_INFO, "SetPixelClock Failed\n");
    return FALSE;
}

/*
 *
 */
struct atomCodeTableVersion
rhdAtomSetPixelClockVersion(atomBiosHandlePtr handle)
{
    struct atomCodeTableVersion version;
    int index = GetIndexIntoMasterTable(COMMAND, SetPixelClock);
    rhdAtomGetCommandTableRevisionSize(handle, index, &version.cref, &version.fref, NULL);

    DEBUG_VERSION(index, handle, version);

    return version;

}

/*
 *
 */
Bool
rhdAtomSelectCrtcSource(atomBiosHandlePtr handle, enum atomCrtc CrtcId,
			struct atomCrtcSourceConfig *config)
{
    AtomBiosArgRec data;
    CARD8 version;
    Bool NeedMode = FALSE;

    union
    {
	SELECT_CRTC_SOURCE_PARAMETERS crtc;
	SELECT_CRTC_SOURCE_PS_ALLOCATION crtc_a;
	SELECT_CRTC_SOURCE_PARAMETERS_V2 crtc2;
/* 	SELECT_CRTC_SOURCE_PS_ALLOCATION_V2 crtc2_a; */
    } ps;

    RHDFUNC(handle);

    data.exec.index = GetIndexIntoMasterTable(COMMAND, SelectCRTC_Source);

    if (!rhdAtomGetCommandTableRevisionSize(handle, data.exec.index, &version, NULL, NULL))
	return FALSE;

    switch  (version) {
	case 1:
	    switch (CrtcId) {
		case atomCrtc1:
		    ps.crtc.ucCRTC = ATOM_CRTC1;
		    break;
		case atomCrtc2:
		    ps.crtc.ucCRTC = ATOM_CRTC2;
		    break;
	    }
	    switch (config->u.Device) {
		case atomCRT1:
		    ps.crtc.ucDevice = ATOM_DEVICE_CRT1_INDEX;
		    break;
		case atomLCD1:
		    ps.crtc.ucDevice = ATOM_DEVICE_LCD1_INDEX;
	    break;
		case atomTV1:
		    ps.crtc.ucDevice = ATOM_DEVICE_TV1_INDEX;
	    break;
		case atomDFP1:
		    ps.crtc.ucDevice = ATOM_DEVICE_DFP1_INDEX;
	    break;
		case atomCRT2:
		    ps.crtc.ucDevice = ATOM_DEVICE_CRT2_INDEX;
	    break;
		case atomLCD2:
		    ps.crtc.ucDevice = ATOM_DEVICE_LCD2_INDEX;
	    break;
		case atomTV2:
		    ps.crtc.ucDevice = ATOM_DEVICE_TV2_INDEX;
	    break;
		case atomDFP2:
		    ps.crtc.ucDevice = ATOM_DEVICE_DFP2_INDEX;
	    break;
		case atomCV:
		    ps.crtc.ucDevice = ATOM_DEVICE_CV_INDEX;
	    break;
		case atomDFP3:
		    ps.crtc.ucDevice = ATOM_DEVICE_DFP3_INDEX;
	    break;
		case atomDFP4:
		    ps.crtc.ucDevice = ATOM_DEVICE_DFP4_INDEX;
		    break;
		case atomDFP5:
		    ps.crtc.ucDevice = ATOM_DEVICE_DFP5_INDEX;
		    break;
		case atomNone:
		    return FALSE;
    }
	    break;
	case 2:
	    switch (CrtcId) {
		case atomCrtc1:
		    ps.crtc2.ucCRTC = ATOM_CRTC1;
		    break;
		case atomCrtc2:
		    ps.crtc2.ucCRTC = ATOM_CRTC2;
	    break;
    }
	    switch (config->u.crtc2.Encoder) {
		case atomEncoderDACA:
		    ps.crtc2.ucEncoderID = ASIC_INT_DAC1_ENCODER_ID;
		    break;
		case atomEncoderDACB:
		    ps.crtc2.ucEncoderID = ASIC_INT_DAC2_ENCODER_ID;
		    break;
		case atomEncoderTV:
		    ps.crtc2.ucEncoderID = ASIC_INT_TV_ENCODER_ID;
		    break;
		case atomEncoderDVO:
		    ps.crtc2.ucEncoderID = ASIC_INT_DVO_ENCODER_ID;
		    NeedMode = TRUE;
		    break;
		case atomEncoderDIG1:
		    ps.crtc2.ucEncoderID = ASIC_INT_DIG1_ENCODER_ID;
		    NeedMode = TRUE;
		    break;

		case atomEncoderDIG2:
		    ps.crtc2.ucEncoderID = ASIC_INT_DIG2_ENCODER_ID;
		    break;
		case atomEncoderExternal:
		    ps.crtc2.ucEncoderID = ASIC_EXT_DIG_ENCODER_ID;
		    break;
		case atomEncoderTMDS1:
		case atomEncoderTMDS2:
		case atomEncoderLVDS:
		case atomEncoderNone:
		    return FALSE;
	    }
	    if (NeedMode) {
		switch (config->u.crtc2.Mode) {
		    case atomDVI:
			ps.crtc2.ucEncodeMode = ATOM_ENCODER_MODE_DVI;
			break;
		    case atomDP:
			ps.crtc2.ucEncodeMode = ATOM_ENCODER_MODE_DP;
			break;
		    case atomLVDS:
			ps.crtc2.ucEncodeMode = ATOM_ENCODER_MODE_LVDS;
			break;
		    case atomHDMI:
			ps.crtc2.ucEncodeMode = ATOM_ENCODER_MODE_HDMI;
			break;
		    case atomSDVO:
			ps.crtc2.ucEncodeMode = ATOM_ENCODER_MODE_SDVO;
			break;
		    case atomTVComposite:
		    case atomTVSVideo:
			ps.crtc2.ucEncodeMode = ATOM_ENCODER_MODE_TV;
			break;
		    case atomTVComponent:
			ps.crtc2.ucEncodeMode = ATOM_ENCODER_MODE_CV;
			break;
		    case atomCRT:
			ps.crtc2.ucEncodeMode = ATOM_ENCODER_MODE_CRT;
			break;
		    case atomNoEncoder:
			xf86DrvMsg(handle->scrnIndex, X_ERROR, "%s: invalid encoder type.\n",__func__);
			return FALSE;
		}
	    }
	    break;
    }

    data.exec.dataSpace = NULL;
    data.exec.pspace = &ps;

    xf86DrvMsg(handle->scrnIndex, X_INFO, "Calling SelectCRTCSource\n");
    atomDebugPrintPspace(handle, &data, sizeof(ps));
    if (RHDAtomBiosFunc(handle->rhdPtr, handle,
			ATOMBIOS_EXEC, &data) == ATOM_SUCCESS) {
	xf86DrvMsg(handle->scrnIndex, X_INFO, "SelectCRTCSource Successful\n");
	return TRUE;
    }
    xf86DrvMsg(handle->scrnIndex, X_INFO, "SelectCRTCSource Failed\n");
    return FALSE;
}

/*
 *
 */
struct atomCodeTableVersion
rhdAtomSelectCrtcSourceVersion(atomBiosHandlePtr handle)
{
    struct atomCodeTableVersion version;
    int index = GetIndexIntoMasterTable(COMMAND, SelectCRTC_Source);
    rhdAtomGetCommandTableRevisionSize(handle, index, &version.cref, &version.fref, NULL);
    DEBUG_VERSION(index, handle, version);
    return version;
}


# endif  /* ATOM_BIOS_PARSER */


static AtomBiosResult
rhdAtomInit(atomBiosHandlePtr unused1, AtomBiosRequestID unused2,
		    AtomBiosArgPtr data)
{
    int scrnIndex = data->val;
    RHDPtr rhdPtr = data->val;
  unsigned char *ptr;
  atomDataTablesPtr atomDataPtr;
  atomBiosHandlePtr handle = NULL;
  unsigned int BIOSImageSize = 0;
  Bool unposted = FALSE;
    unsigned char* codeTable;

  data->atomhandle = NULL;

    RHDFUNCI(scrnIndex);

    if (rhdPtr->BIOSCopy) {
	xf86DrvMsg(scrnIndex,X_INFO,"Getting BIOS copy from INT10\n");
    ptr = rhdPtr->BIOSCopy;
    rhdPtr->BIOSCopy = NULL;

    BIOSImageSize = ptr[2] * 512;
	if (BIOSImageSize > legacyBIOSMax) {
	    xf86DrvMsg(scrnIndex,X_ERROR,"Invalid BIOS length field\n");
	    return ATOM_FAILED;
    }
  }
  else
  {
//    if (!xf86IsEntityPrimary(rhdPtr->entityIndex))
//    {
//      if (!(BIOSImageSize = RHDReadPCIBios(rhdPtr, &ptr)))
//        return ATOM_FAILED;
//      unposted = TRUE;
//    }
//    else
    {
	    int read_len;
	    unsigned char tmp[32];

      DBG(dbgprintf("Getting BIOS copy from legacy VBIOS location\n"));
      memcpy(tmp,(char*)(OS_BASE+legacyBIOSLocation), 32);
	    BIOSImageSize = tmp[2] * 512;
	    if (BIOSImageSize > legacyBIOSMax) {
    xf86DrvMsg(0,X_ERROR,"Invalid BIOS length field\n");
        return ATOM_FAILED;
	    }
      if (!(ptr = (char*)KernelAlloc(BIOSImageSize)))
      {
        DBG(dbgprintf("Cannot allocate %i bytes of memory "
                  "for BIOS image\n",BIOSImageSize));
        return ATOM_FAILED;
	    }
      memcpy(ptr,(char*)(OS_BASE+legacyBIOSLocation), BIOSImageSize);
      rhdPtr->BIOSCopy = ptr;
    }
  }

    if (!(atomDataPtr = xcalloc(1, sizeof(atomDataTables)))) {
	xf86DrvMsg(scrnIndex,X_ERROR,"Cannot allocate memory for "
		   "ATOM BIOS data tabes\n");
    goto error;
  }
    if (!rhdAtomGetTables(rhdPtr, ptr, atomDataPtr, &codeTable, BIOSImageSize))
    goto error1;
    if (!(handle = xcalloc(1, sizeof(atomBiosHandleRec)))) {
	xf86DrvMsg(scrnIndex,X_ERROR,"Cannot allocate memory\n");
    goto error1;
  }
  handle->BIOSBase = ptr;
  handle->atomDataPtr = atomDataPtr;
  handle->scrnIndex = scrnIndex;
  handle->rhdPtr = rhdPtr;
  handle->PciTag = rhdPtr->PciTag;
  handle->BIOSImageSize = BIOSImageSize;
    handle->codeTable = codeTable;
    handle->SaveListObjects = NULL;

# ifdef ATOM_BIOS_PARSER
    /* Try to find out if BIOS has been posted (either by system or int10 */
    if (unposted) {
	/* run AsicInit */
	if (!rhdAtomASICInit(handle))
	    xf86DrvMsg(scrnIndex, X_WARNING,
		       "%s: AsicInit failed. Won't be able to obtain in VRAM "
		       "FB scratch space\n",__func__);
    }
# endif

    data->atomhandle = handle;
    return ATOM_SUCCESS;

 error1:
    xfree(atomDataPtr);
 error:
    xfree(ptr);
    return ATOM_FAILED;
}

static AtomBiosResult
rhdAtomTearDown(atomBiosHandlePtr handle,
		AtomBiosRequestID unused1, AtomBiosArgPtr unused2)
{
    RHDFUNC(handle);

    xfree(handle->BIOSBase);
    xfree(handle->atomDataPtr);
    if (handle->scratchBase) xfree(handle->scratchBase);
    xfree(handle);
    return ATOM_SUCCESS;
}

static AtomBiosResult
rhdAtomVramInfoQuery(atomBiosHandlePtr handle, AtomBiosRequestID func,
		     AtomBiosArgPtr data)
{
    atomDataTablesPtr atomDataPtr;
    CARD32 *val = &data->val;
    RHDFUNC(handle);

    atomDataPtr = handle->atomDataPtr;

    switch (func) {
      case GET_FW_FB_START:
        *val = atomDataPtr->VRAM_UsageByFirmware
        ->asFirmwareVramReserveInfo[0].ulStartAddrUsedByFirmware;
        break;
     case GET_FW_FB_SIZE:
       *val = atomDataPtr->VRAM_UsageByFirmware
       ->asFirmwareVramReserveInfo[0].usFirmwareUseInKb;
       break;
     default:
       return ATOM_NOT_IMPLEMENTED;
    }
    return ATOM_SUCCESS;
}

static AtomBiosResult
rhdAtomTmdsInfoQuery(atomBiosHandlePtr handle,
		     AtomBiosRequestID func, AtomBiosArgPtr data)
{
    atomDataTablesPtr atomDataPtr;
    CARD32 *val = &data->val;
    int i = 0, clock = *val;

    atomDataPtr = handle->atomDataPtr;
    if (!rhdAtomGetTableRevisionAndSize(
	    (ATOM_COMMON_TABLE_HEADER *)(atomDataPtr->TMDS_Info),
	    NULL,NULL,NULL)) {
	return ATOM_FAILED;
    }

    RHDFUNC(handle);

    if (func == ATOM_TMDS_MAX_FREQUENCY)
	*val = atomDataPtr->TMDS_Info->usMaxFrequency * 10;
    else {
	if (clock > atomDataPtr->TMDS_Info->usMaxFrequency * 10)
	    return ATOM_FAILED;

	for (;i < ATOM_MAX_MISC_INFO; i++) {
	    if (clock < atomDataPtr->TMDS_Info->asMiscInfo[i].usFrequency * 10) {
    switch (func) {
	case ATOM_TMDS_PLL_CHARGE_PUMP:
			*val = atomDataPtr->TMDS_Info->asMiscInfo[i].ucPLL_ChargePump;
			break;
		    case ATOM_TMDS_PLL_DUTY_CYCLE:
			*val = atomDataPtr->TMDS_Info->asMiscInfo[i].ucPLL_DutyCycle;
			break;
		    case ATOM_TMDS_PLL_VCO_GAIN:
			*val = atomDataPtr->TMDS_Info->asMiscInfo[i].ucPLL_VCO_Gain;
			break;
		    case ATOM_TMDS_PLL_VOLTAGE_SWING:
			*val = atomDataPtr->TMDS_Info->asMiscInfo[i].ucPLL_VoltageSwing;
	    break;
	default:
	    return ATOM_NOT_IMPLEMENTED;
    }
		break;
	    }
	}
    }

    if (i > ATOM_MAX_MISC_INFO)
	return ATOM_FAILED;

    return ATOM_SUCCESS;
}

static DisplayModePtr
rhdAtomLvdsTimings(atomBiosHandlePtr handle, ATOM_DTD_FORMAT *dtd)
{
    DisplayModePtr mode;
#define NAME_LEN 16
    char name[NAME_LEN];

    RHDFUNC(handle);

    if (!(mode = (DisplayModePtr)xcalloc(1,sizeof(DisplayModeRec))))
	return NULL;

    mode->CrtcHDisplay = mode->HDisplay = dtd->usHActive;
    mode->CrtcVDisplay = mode->VDisplay = dtd->usVActive;
    mode->CrtcHBlankStart = dtd->usHActive + dtd->ucHBorder;
    mode->CrtcHBlankEnd = mode->CrtcHBlankStart + dtd->usHBlanking_Time;
    mode->CrtcHTotal = mode->HTotal = mode->CrtcHBlankEnd + dtd->ucHBorder;
    mode->CrtcVBlankStart = dtd->usVActive + dtd->ucVBorder;
    mode->CrtcVBlankEnd = mode->CrtcVBlankStart + dtd->usVBlanking_Time;
    mode->CrtcVTotal = mode->VTotal = mode->CrtcVBlankEnd + dtd->ucVBorder;
    mode->CrtcHSyncStart = mode->HSyncStart = dtd->usHActive + dtd->usHSyncOffset;
    mode->CrtcHSyncEnd = mode->HSyncEnd = mode->HSyncStart + dtd->usHSyncWidth;
    mode->CrtcVSyncStart = mode->VSyncStart = dtd->usVActive + dtd->usVSyncOffset;
    mode->CrtcVSyncEnd = mode->VSyncEnd = mode->VSyncStart + dtd->usVSyncWidth;

    mode->SynthClock = mode->Clock  = dtd->usPixClk * 10;

    mode->HSync = ((float) mode->Clock) / ((float)mode->HTotal);
    mode->VRefresh = (1000.0 * ((float) mode->Clock))
	/ ((float)(((float)mode->HTotal) * ((float)mode->VTotal)));

    snprintf(name, NAME_LEN, "%dx%d",
		 mode->HDisplay, mode->VDisplay);
    mode->name = strdup(name);

    RHDDebug(handle->scrnIndex,"%s: LVDS Modeline: %s  "
	     "%2.d  %i (%i) %i %i (%i) %i  %i (%i) %i %i (%i) %i\n",
              __func__, mode->name, mode->Clock,
	     mode->HDisplay, mode->CrtcHBlankStart, mode->HSyncStart, mode->CrtcHSyncEnd,
	     mode->CrtcHBlankEnd, mode->HTotal,
	     mode->VDisplay, mode->CrtcVBlankStart, mode->VSyncStart, mode->VSyncEnd,
	     mode->CrtcVBlankEnd, mode->VTotal);
#undef NAME_LEN
    return mode;
}

static unsigned char*
rhdAtomLvdsDDC(atomBiosHandlePtr handle, CARD32 offset, unsigned char *record)
{
    unsigned char *EDIDBlock;

    RHDFUNC(handle);

    while (*record != ATOM_RECORD_END_TYPE) {

	switch (*record) {
	    case LCD_MODE_PATCH_RECORD_MODE_TYPE:
		offset += sizeof(ATOM_PATCH_RECORD_MODE);
		if (offset > handle->BIOSImageSize) break;
		record += sizeof(ATOM_PATCH_RECORD_MODE);
		break;

	    case LCD_RTS_RECORD_TYPE:
		offset += sizeof(ATOM_LCD_RTS_RECORD);
		if (offset > handle->BIOSImageSize) break;
		record += sizeof(ATOM_LCD_RTS_RECORD);
		break;

	    case LCD_CAP_RECORD_TYPE:
		offset += sizeof(ATOM_LCD_MODE_CONTROL_CAP);
		if (offset > handle->BIOSImageSize) break;
		record += sizeof(ATOM_LCD_MODE_CONTROL_CAP);
		break;

	    case LCD_FAKE_EDID_PATCH_RECORD_TYPE:
		offset += sizeof(ATOM_FAKE_EDID_PATCH_RECORD);
		/* check if the structure still fully lives in the BIOS image */
		if (offset > handle->BIOSImageSize) break;
		offset += ((ATOM_FAKE_EDID_PATCH_RECORD*)record)->ucFakeEDIDLength
		    - sizeof(UCHAR);
		if (offset > handle->BIOSImageSize) break;
		/* dup string as we free it later */
		if (!(EDIDBlock = (unsigned char *)xalloc(
			  ((ATOM_FAKE_EDID_PATCH_RECORD*)record)->ucFakeEDIDLength)))
		    return NULL;
		memcpy(EDIDBlock,&((ATOM_FAKE_EDID_PATCH_RECORD*)record)->ucFakeEDIDString,
		       ((ATOM_FAKE_EDID_PATCH_RECORD*)record)->ucFakeEDIDLength);

		/* for testing */
		{
  //      xf86MonPtr mon = xf86InterpretEDID(handle->scrnIndex,EDIDBlock);
  //      xf86PrintEDID(mon);
 //       kfree(mon);
		}
		return EDIDBlock;

	    case LCD_PANEL_RESOLUTION_RECORD_TYPE:
		offset += sizeof(ATOM_PANEL_RESOLUTION_PATCH_RECORD);
		if (offset > handle->BIOSImageSize) break;
		record += sizeof(ATOM_PANEL_RESOLUTION_PATCH_RECORD);
		break;

	    default:
		xf86DrvMsg(handle->scrnIndex, X_ERROR,
			   "%s: unknown record type: %x\n",__func__,*record);
        return NULL;
	}
    }

    return NULL;
}

static AtomBiosResult
rhdAtomLvdsGetTimings(atomBiosHandlePtr handle, AtomBiosRequestID func,
		  AtomBiosArgPtr data)
{
    atomDataTablesPtr atomDataPtr;
    CARD8 crev, frev;
    unsigned long offset;

    RHDFUNC(handle);

    atomDataPtr = handle->atomDataPtr;

    if (!rhdAtomGetTableRevisionAndSize(
	    (ATOM_COMMON_TABLE_HEADER *)(atomDataPtr->LVDS_Info.base),
	    &crev,&frev,NULL)) {
	return ATOM_FAILED;
    }

    switch (crev) {

	case 1:
	    switch (func) {
		case ATOMBIOS_GET_PANEL_MODE:
		    data->mode = rhdAtomLvdsTimings(handle,
						    &atomDataPtr->LVDS_Info
						    .LVDS_Info->sLCDTiming);
		    if (data->mode)
			return ATOM_SUCCESS;
		default:
		    return ATOM_FAILED;
	    }
	case 2:
	    switch (func) {
		case ATOMBIOS_GET_PANEL_MODE:
		    data->mode = rhdAtomLvdsTimings(handle,
						    &atomDataPtr->LVDS_Info
						    .LVDS_Info_v12->sLCDTiming);
		    if (data->mode)
			return ATOM_SUCCESS;
		    return ATOM_FAILED;

		case ATOMBIOS_GET_PANEL_EDID:
		    offset = (unsigned long)&atomDataPtr->LVDS_Info.base
			- (unsigned long)handle->BIOSBase
			+ atomDataPtr->LVDS_Info
			.LVDS_Info_v12->usExtInfoTableOffset;

		    data->EDIDBlock
			= rhdAtomLvdsDDC(handle, offset,
					 (unsigned char *)
					 &atomDataPtr->LVDS_Info.base
					 + atomDataPtr->LVDS_Info
					 .LVDS_Info_v12->usExtInfoTableOffset);
		    if (data->EDIDBlock)
			return ATOM_SUCCESS;
		default:
		    return ATOM_FAILED;
	    }
	default:
	    return ATOM_NOT_IMPLEMENTED;
    }
/*NOTREACHED*/
}

static AtomBiosResult
rhdAtomLvdsInfoQuery(atomBiosHandlePtr handle,
		     AtomBiosRequestID func,  AtomBiosArgPtr data)
{
    atomDataTablesPtr atomDataPtr;
    CARD8 crev, frev;
    CARD32 *val = &data->val;

    RHDFUNC(handle);

    atomDataPtr = handle->atomDataPtr;

    if (!rhdAtomGetTableRevisionAndSize(
	    (ATOM_COMMON_TABLE_HEADER *)(atomDataPtr->LVDS_Info.base),
	    &crev,&frev,NULL)) {
	return ATOM_FAILED;
    }

    switch (crev) {
	case 1:
	    switch (func) {
		case ATOM_LVDS_SUPPORTED_REFRESH_RATE:
		    *val = atomDataPtr->LVDS_Info
			.LVDS_Info->usSupportedRefreshRate;
		    break;
		case ATOM_LVDS_OFF_DELAY:
		    *val = atomDataPtr->LVDS_Info
			.LVDS_Info->usOffDelayInMs;
		    break;
		case ATOM_LVDS_SEQ_DIG_ONTO_DE:
		    *val = atomDataPtr->LVDS_Info
			.LVDS_Info->ucPowerSequenceDigOntoDEin10Ms * 10;
		    break;
		case ATOM_LVDS_SEQ_DE_TO_BL:
		    *val = atomDataPtr->LVDS_Info
			.LVDS_Info->ucPowerSequenceDEtoBLOnin10Ms * 10;
		    break;
		case     ATOM_LVDS_TEMPORAL_DITHER:
		    *val = (atomDataPtr->LVDS_Info
			    .LVDS_Info->ucLVDS_Misc & 0x40) != 0;
		    break;
		case     ATOM_LVDS_SPATIAL_DITHER:
		    *val = (atomDataPtr->LVDS_Info
			    .LVDS_Info->ucLVDS_Misc & 0x20) != 0;
		    break;
		case     ATOM_LVDS_FPDI:
		    *val = (atomDataPtr->LVDS_Info
			    .LVDS_Info->ucLVDS_Misc & 0x10) != 0;
		    break;
		case     ATOM_LVDS_DUALLINK:
		    *val = (atomDataPtr->LVDS_Info
			    .LVDS_Info->ucLVDS_Misc & 0x01) != 0;
		    break;
		case     ATOM_LVDS_24BIT:
		    *val = (atomDataPtr->LVDS_Info
			    .LVDS_Info->ucLVDS_Misc & 0x02) != 0;
		    break;
		case     ATOM_LVDS_GREYLVL:
		    *val = (atomDataPtr->LVDS_Info
			    .LVDS_Info->ucLVDS_Misc & ATOM_PANEL_MISC_GREY_LEVEL)
			>> ATOM_PANEL_MISC_GREY_LEVEL_SHIFT ;
		    break;
		default:
		    return ATOM_NOT_IMPLEMENTED;
	    }
	    break;
	case 2:
	    switch (func) {
		case ATOM_LVDS_SUPPORTED_REFRESH_RATE:
		    *val = atomDataPtr->LVDS_Info
			.LVDS_Info_v12->usSupportedRefreshRate;
		    break;
		case ATOM_LVDS_OFF_DELAY:
		    *val = atomDataPtr->LVDS_Info
			.LVDS_Info_v12->usOffDelayInMs;
		    break;
		case ATOM_LVDS_SEQ_DIG_ONTO_DE:
		    *val = atomDataPtr->LVDS_Info
			.LVDS_Info_v12->ucPowerSequenceDigOntoDEin10Ms * 10;
		    break;
		case ATOM_LVDS_SEQ_DE_TO_BL:
		    *val = atomDataPtr->LVDS_Info
			.LVDS_Info_v12->ucPowerSequenceDEtoBLOnin10Ms * 10;
		    break;
		case     ATOM_LVDS_FPDI:
		    *val = (atomDataPtr->LVDS_Info
			    .LVDS_Info_v12->ucLVDS_Misc * 0x10) != 0;
		    break;
		case     ATOM_LVDS_SPATIAL_DITHER:
		    *val = (atomDataPtr->LVDS_Info
			    .LVDS_Info_v12->ucLVDS_Misc & 0x20) != 0;
		    break;
		case     ATOM_LVDS_TEMPORAL_DITHER:
		    *val = (atomDataPtr->LVDS_Info
			    .LVDS_Info_v12->ucLVDS_Misc & 0x40) != 0;
		    break;
		case     ATOM_LVDS_DUALLINK:
		    *val = (atomDataPtr->LVDS_Info
			    .LVDS_Info_v12->ucLVDS_Misc & 0x01) != 0;
		    break;
		case     ATOM_LVDS_24BIT:
		    *val = (atomDataPtr->LVDS_Info
			    .LVDS_Info_v12->ucLVDS_Misc & 0x02) != 0;
		    break;
		case     ATOM_LVDS_GREYLVL:
		    *val = (atomDataPtr->LVDS_Info
			    .LVDS_Info_v12->ucLVDS_Misc & 0x0C) >> 2;
		    break;
		default:
		    return ATOM_NOT_IMPLEMENTED;
	    }
	    break;
	default:
	    return ATOM_NOT_IMPLEMENTED;
    }

    return ATOM_SUCCESS;
}

static AtomBiosResult
rhdAtomCompassionateDataQuery(atomBiosHandlePtr handle,
			AtomBiosRequestID func, AtomBiosArgPtr data)
{
    atomDataTablesPtr atomDataPtr;
    CARD8 crev, frev;
    CARD32 *val = &data->val;

    RHDFUNC(handle);

    atomDataPtr = handle->atomDataPtr;

    if (!rhdAtomGetTableRevisionAndSize(
	    (ATOM_COMMON_TABLE_HEADER *)(atomDataPtr->CompassionateData),
	    &crev,&frev,NULL)) {
	return ATOM_FAILED;
    }

    switch (func) {
	case ATOM_DAC1_BG_ADJ:
	    *val = atomDataPtr->CompassionateData->
		ucDAC1_BG_Adjustment;
	    break;
	case ATOM_DAC1_DAC_ADJ:
	    *val = atomDataPtr->CompassionateData->
		ucDAC1_DAC_Adjustment;
	    break;
	case ATOM_DAC1_FORCE:
	    *val = atomDataPtr->CompassionateData->
		usDAC1_FORCE_Data;
	    break;
	case ATOM_DAC2_CRTC2_BG_ADJ:
	    *val = atomDataPtr->CompassionateData->
		ucDAC2_CRT2_BG_Adjustment;
	    break;
	case ATOM_DAC2_NTSC_BG_ADJ:
	    *val = atomDataPtr->CompassionateData->
		ucDAC2_NTSC_BG_Adjustment;
	    break;
	case ATOM_DAC2_PAL_BG_ADJ:
	    *val = atomDataPtr->CompassionateData->
		ucDAC2_PAL_BG_Adjustment;
	    break;
	case ATOM_DAC2_CV_BG_ADJ:
	    *val = atomDataPtr->CompassionateData->
		ucDAC2_CV_BG_Adjustment;
	    break;
	case ATOM_DAC2_CRTC2_DAC_ADJ:
	    *val = atomDataPtr->CompassionateData->
		ucDAC2_CRT2_DAC_Adjustment;
	    break;
	case ATOM_DAC2_NTSC_DAC_ADJ:
	    *val = atomDataPtr->CompassionateData->
		ucDAC2_NTSC_DAC_Adjustment;
	    break;
	case ATOM_DAC2_PAL_DAC_ADJ:
	    *val = atomDataPtr->CompassionateData->
		ucDAC2_PAL_DAC_Adjustment;
	    break;
	case ATOM_DAC2_CV_DAC_ADJ:
	    *val = atomDataPtr->CompassionateData->
		ucDAC2_CV_DAC_Adjustment;
	    break;
	case ATOM_DAC2_CRTC2_FORCE:
	    *val = atomDataPtr->CompassionateData->
		usDAC2_CRT2_FORCE_Data;
	    break;
	case ATOM_DAC2_CRTC2_MUX_REG_IND:
	    *val = atomDataPtr->CompassionateData->
		usDAC2_CRT2_MUX_RegisterIndex;
	    break;
	case ATOM_DAC2_CRTC2_MUX_REG_INFO:
	    *val = atomDataPtr->CompassionateData->
		ucDAC2_CRT2_MUX_RegisterInfo;
	    break;
	default:
	    return ATOM_NOT_IMPLEMENTED;
    }
    return ATOM_SUCCESS;
}

/*
 *
 */
enum atomPCIELanes atomPCIELanesMap[] = {
    atomPCIELaneNONE,
    atomPCIELane0_3,
    atomPCIELane4_7,
    atomPCIELane0_7,
    atomPCIELane8_11,
    atomPCIELaneNONE,
    atomPCIELaneNONE,
    atomPCIELaneNONE,
    atomPCIELane12_15,
    atomPCIELaneNONE,
    atomPCIELaneNONE,
    atomPCIELaneNONE,
    atomPCIELane8_15,
    atomPCIELaneNONE,
    atomPCIELaneNONE,
    atomPCIELaneNONE,
    atomPCIELaneNONE
};

static AtomBiosResult
rhdAtomIntegratedSystemInfoQuery(atomBiosHandlePtr handle,
			AtomBiosRequestID func, AtomBiosArgPtr data)
{
    atomDataTablesPtr atomDataPtr;
    CARD8 crev, frev;
    CARD32 *val = &data->val;

    RHDFUNC(handle);

    atomDataPtr = handle->atomDataPtr;

    if (!rhdAtomGetTableRevisionAndSize(
	    (ATOM_COMMON_TABLE_HEADER *)(atomDataPtr->IntegratedSystemInfo.base),
	    &crev,&frev,NULL)) {
	return ATOM_FAILED;
    }

    switch (crev) {
	case 1:
	    switch (func) {
		case ATOM_GET_PCIENB_CFG_REG7:
		    *val = atomDataPtr->IntegratedSystemInfo.IntegratedSystemInfo->usPCIENBCfgReg7;
		    break;
		case ATOM_GET_CAPABILITY_FLAG:
		    *val = atomDataPtr->IntegratedSystemInfo.IntegratedSystemInfo->usCapabilityFlag;
		    break;
		default:
		    return ATOM_NOT_IMPLEMENTED;
	    }
	    break;
	case 2:
	    switch (func) {
		case ATOM_GET_PCIE_LANES:
		{
		    CARD32 n;
		    switch (*val) {
			case 1:
			    n = atomDataPtr->IntegratedSystemInfo.IntegratedSystemInfo_v2->ulDDISlot1Config;
			    break;
			case 2:
			    n = atomDataPtr->IntegratedSystemInfo.IntegratedSystemInfo_v2->ulDDISlot2Config;
			    break;
		default:
			    return ATOM_FAILED;
	    }
		    data->pcieLanes.Chassis = atomPCIELanesMap[n & 0xf];
		    data->pcieLanes.Docking = atomPCIELanesMap[(n >> 4) & 0xf];
		    RHDDebug(handle->scrnIndex, "AtomBIOS IntegratedSystemInfo PCIELanes: Chassis=%x Docking=%x\n",
			     data->pcieLanes.Chassis, data->pcieLanes.Docking);
		    return ATOM_SUCCESS;
		}
	    break;
		default:
		    return ATOM_NOT_IMPLEMENTED;
    }
	    return ATOM_NOT_IMPLEMENTED;
    }

    return ATOM_SUCCESS;
}

static DisplayModePtr
rhdAtomAnalogTVTimings(atomBiosHandlePtr handle,
		       ATOM_ANALOG_TV_INFO *tv_info,
		       enum RHD_TV_MODE tvMode)
{
    atomDataTablesPtr atomDataPtr;
    DisplayModePtr mode;
    int mode_n;
    char *name;
    ATOM_MODE_TIMING *amt;

    RHDFUNC(handle);

    atomDataPtr = handle->atomDataPtr;

    switch (tvMode) {
	case NTSC_SUPPORT:
	case NTSCJ_SUPPORT:
	    mode_n = 0;
	    name = "TV_NTSC";
	    break;
	case PAL_SUPPORT:
	case PALM_SUPPORT:
	case PALCN_SUPPORT:
	case PALN_SUPPORT:
	case PAL60_SUPPORT:
	case SECAM_SUPPORT:
	    mode_n = 1;
	    name = "TV_PAL/SECAM";
	    break;
	default:
	    return NULL;
    }


    if (!(tv_info->ucTV_SupportedStandard & (tvMode)))
	return NULL;

    if (!(mode = (DisplayModePtr)xcalloc(1,sizeof(DisplayModeRec))))
	return NULL;

    amt = &tv_info->aModeTimings[mode_n];

    mode->CrtcHDisplay = mode->HDisplay =  amt->usCRTC_H_Disp;
    mode->CrtcHSyncStart = mode->HSyncStart = amt->usCRTC_H_SyncStart;
    mode->CrtcHSyncEnd = mode->HSyncEnd = mode->HSyncStart + amt->usCRTC_H_SyncWidth;
    mode->CrtcHTotal = mode->HTotal = amt->usCRTC_H_Total;
    mode->CrtcHBlankStart = mode->HDisplay + amt->usCRTC_OverscanRight;
    mode->CrtcHBlankEnd = mode->HTotal - amt->usCRTC_OverscanLeft;

    mode->CrtcVDisplay = mode->VDisplay = amt->usCRTC_V_Disp;
    mode->CrtcVSyncStart = mode->VSyncStart = amt->usCRTC_V_SyncStart;
    mode->CrtcVSyncEnd = mode->VSyncEnd = mode->VSyncStart + amt->usCRTC_V_SyncWidth;
    mode->CrtcVTotal = mode->VTotal = amt->usCRTC_V_Total;
    mode->CrtcVBlankStart = mode->VDisplay + amt->usCRTC_OverscanBottom;
    mode->CrtcVBlankEnd = mode->CrtcVTotal - amt->usCRTC_OverscanTop;

    mode->SynthClock = mode->Clock  = amt->usPixelClock * 10;
    if (amt->susModeMiscInfo.usAccess & ATOM_HSYNC_POLARITY)
	mode->Flags |= V_NHSYNC;
    else
	mode->Flags |= V_PHSYNC;
    if (amt->susModeMiscInfo.usAccess & ATOM_VSYNC_POLARITY)
	mode->Flags |= V_NVSYNC;
    else
	mode->Flags |= V_PVSYNC;
    if (amt->susModeMiscInfo.usAccess & ATOM_INTERLACE)
	mode->Flags |= V_INTERLACE;
    if (amt->susModeMiscInfo.usAccess & ATOM_COMPOSITESYNC)
	mode->Flags |= V_CSYNC;
    if (amt->susModeMiscInfo.usAccess & ATOM_DOUBLE_CLOCK_MODE)
	mode->Flags |= V_DBLCLK;

    mode->HSync = ((float) mode->Clock) / ((float)mode->HTotal);
    mode->VRefresh = (1000.0 * ((float) mode->Clock))
	/ ((float)(((float)mode->HTotal) * ((float)mode->VTotal)));

    mode->name = strdup(name);

    RHDDebug(handle->scrnIndex,"%s: TV Modeline: %s  "
	     "%2.d  %i (%i) %i %i (%i) %i  %i (%i) %i %i (%i) %i\n",
	     __func__, mode->name, mode->Clock,
	     mode->HDisplay, mode->CrtcHBlankStart, mode->HSyncStart, mode->CrtcHSyncEnd,
	     mode->CrtcHBlankEnd, mode->HTotal,
	     mode->VDisplay, mode->CrtcVBlankStart, mode->VSyncStart, mode->VSyncEnd,
	     mode->CrtcVBlankEnd, mode->VTotal);


    return mode;
}

static AtomBiosResult
rhdAtomAnalogTVInfoQuery(atomBiosHandlePtr handle,
			AtomBiosRequestID func, AtomBiosArgPtr data)
{
    CARD8 crev, frev;
    atomDataTablesPtr atomDataPtr = handle->atomDataPtr;
    int mode = 0, i;
    struct { enum RHD_TV_MODE rhd_mode; int atomMode; }
    tv_modes[] = {
	{ RHD_TV_NTSC,  NTSC_SUPPORT },
	{ RHD_TV_NTSCJ, NTSCJ_SUPPORT},
	{ RHD_TV_PAL,   PAL_SUPPORT  },
	{ RHD_TV_PALM,  PALM_SUPPORT },
	{ RHD_TV_PALCN, PALCN_SUPPORT},
	{ RHD_TV_PALN,  PALN_SUPPORT },
	{ RHD_TV_PAL60, PAL60_SUPPORT},
	{ RHD_TV_SECAM, SECAM_SUPPORT},
	{ RHD_TV_NONE, 0 }
    };


    RHDFUNC(handle);

    if (!rhdAtomGetTableRevisionAndSize(
	    (ATOM_COMMON_TABLE_HEADER *)(atomDataPtr->AnalogTV_Info),
	    &crev,&frev,NULL)) {
	return ATOM_FAILED;
    }
    switch (func) {
	case ATOM_ANALOG_TV_MODE:
	    for (i = 0; tv_modes[i].atomMode; i++) {
		if (data->tvMode == tv_modes[i].rhd_mode) {
		    mode = tv_modes[i].atomMode;
		    break;
		}
	    }
	    data->mode = rhdAtomAnalogTVTimings(handle,
						atomDataPtr->AnalogTV_Info,
						mode);
	    if (!data->mode)
		return ATOM_FAILED;
	    return ATOM_SUCCESS;
	case ATOM_ANALOG_TV_DEFAULT_MODE:
	     data->tvMode = tv_modes[atomDataPtr->AnalogTV_Info->ucTV_BootUpDefaultStandard - 1].rhd_mode;
	    break;
	case ATOM_ANALOG_TV_SUPPORTED_MODES:
	    mode = (CARD32)atomDataPtr->AnalogTV_Info->ucTV_SupportedStandard;
	    data->val = 0;
	    for (i = 0; tv_modes[i].atomMode; i++) {
		if (tv_modes[i].atomMode & mode) {
		    data->val |= tv_modes[i].rhd_mode;
		}
	    }
	    break;
	default:
	    return ATOM_NOT_IMPLEMENTED;
    }

    return ATOM_SUCCESS;
}

static AtomBiosResult
rhdAtomGPIOI2CInfoQuery(atomBiosHandlePtr handle,
			AtomBiosRequestID func, AtomBiosArgPtr data)
{
  atomDataTablesPtr atomDataPtr;
  CARD8 crev, frev;
  CARD32 *val = &data->val;
  unsigned short size;

    RHDFUNC(handle);

  atomDataPtr = handle->atomDataPtr;

  if (!rhdAtomGetTableRevisionAndSize(
	    (ATOM_COMMON_TABLE_HEADER *)(atomDataPtr->GPIO_I2C_Info),
	    &crev,&frev,&size)) {
    return ATOM_FAILED;
  }

      if ((sizeof(ATOM_COMMON_TABLE_HEADER)
		 + (*val * sizeof(ATOM_GPIO_I2C_ASSIGMENT))) > size) {
		xf86DrvMsg(handle->scrnIndex, X_ERROR, "%s: GPIO_I2C Device "
			   "num %lu exeeds table size %u\n",__func__,
			   (unsigned long)val,
			   size);
        return ATOM_FAILED;
	    }

    switch (func) {
	case ATOM_GPIO_I2C_DATA_MASK:
	    *val = atomDataPtr->GPIO_I2C_Info->asGPIO_Info[*val]
		.usDataMaskRegisterIndex;
	    break;

	case ATOM_GPIO_I2C_DATA_MASK_SHIFT:
	    *val = atomDataPtr->GPIO_I2C_Info->asGPIO_Info[*val]
		.ucDataMaskShift;
	    break;

	case ATOM_GPIO_I2C_CLK_MASK:
	    *val = atomDataPtr->GPIO_I2C_Info->asGPIO_Info[*val]
            .usClkMaskRegisterIndex;
	    break;

	case ATOM_GPIO_I2C_CLK_MASK_SHIFT:
	    *val = atomDataPtr->GPIO_I2C_Info->asGPIO_Info[*val]
		.ucClkMaskShift;
	    break;

    default:
	    return ATOM_NOT_IMPLEMENTED;
  }
  return ATOM_SUCCESS;
}

static AtomBiosResult
rhdAtomFirmwareInfoQuery(atomBiosHandlePtr handle,
			 AtomBiosRequestID func, AtomBiosArgPtr data)
{
  atomDataTablesPtr atomDataPtr;
  CARD8 crev, frev;
  CARD32 *val = &data->val;

    RHDFUNC(handle);

  atomDataPtr = handle->atomDataPtr;

  if (!rhdAtomGetTableRevisionAndSize(
	    (ATOM_COMMON_TABLE_HEADER *)(atomDataPtr->FirmwareInfo.base),
	    &crev,&frev,NULL)) {
    return ATOM_FAILED;
  }

    switch (crev) {
	case 1:
	    switch (func) {
        case GET_DEFAULT_ENGINE_CLOCK:
          *val = atomDataPtr->FirmwareInfo
                 .FirmwareInfo->ulDefaultEngineClock * 10;
          break;
        case GET_DEFAULT_MEMORY_CLOCK:
          *val = atomDataPtr->FirmwareInfo
                 .FirmwareInfo->ulDefaultMemoryClock * 10;
          break;
        case GET_MAX_PIXEL_CLOCK_PLL_OUTPUT:
          *val = atomDataPtr->FirmwareInfo
                 .FirmwareInfo->ulMaxPixelClockPLL_Output * 10;
          break;
        case GET_MIN_PIXEL_CLOCK_PLL_OUTPUT:
          *val = atomDataPtr->FirmwareInfo
                 .FirmwareInfo->usMinPixelClockPLL_Output * 10;
        case GET_MAX_PIXEL_CLOCK_PLL_INPUT:
          *val = atomDataPtr->FirmwareInfo
                 .FirmwareInfo->usMaxPixelClockPLL_Input * 10;
          break;
        case GET_MIN_PIXEL_CLOCK_PLL_INPUT:
          *val = atomDataPtr->FirmwareInfo
                 .FirmwareInfo->usMinPixelClockPLL_Input * 10;
          break;
        case GET_MAX_PIXEL_CLK:
          *val = atomDataPtr->FirmwareInfo
               .FirmwareInfo->usMaxPixelClock * 10;
          break;
        case GET_REF_CLOCK:
          *val = atomDataPtr->FirmwareInfo
               .FirmwareInfo->usReferenceClock * 10;
          break;
        default:
          return ATOM_NOT_IMPLEMENTED;
      }
    case 2:
	    switch (func) {
        case GET_DEFAULT_ENGINE_CLOCK:
          *val = atomDataPtr->FirmwareInfo
                .FirmwareInfo_V_1_2->ulDefaultEngineClock * 10;
          break;
        case GET_DEFAULT_MEMORY_CLOCK:
          *val = atomDataPtr->FirmwareInfo
                 .FirmwareInfo_V_1_2->ulDefaultMemoryClock * 10;
          break;
        case GET_MAX_PIXEL_CLOCK_PLL_OUTPUT:
          *val = atomDataPtr->FirmwareInfo
                 .FirmwareInfo_V_1_2->ulMaxPixelClockPLL_Output * 10;
          break;
        case GET_MIN_PIXEL_CLOCK_PLL_OUTPUT:
          *val = atomDataPtr->FirmwareInfo
                 .FirmwareInfo_V_1_2->usMinPixelClockPLL_Output * 10;
          break;
        case GET_MAX_PIXEL_CLOCK_PLL_INPUT:
          *val = atomDataPtr->FirmwareInfo
                .FirmwareInfo_V_1_2->usMaxPixelClockPLL_Input * 10;
          break;
        case GET_MIN_PIXEL_CLOCK_PLL_INPUT:
          *val = atomDataPtr->FirmwareInfo
                .FirmwareInfo_V_1_2->usMinPixelClockPLL_Input * 10;
          break;
        case GET_MAX_PIXEL_CLK:
          *val = atomDataPtr->FirmwareInfo
                .FirmwareInfo_V_1_2->usMaxPixelClock * 10;
          break;
        case GET_REF_CLOCK:
          *val = atomDataPtr->FirmwareInfo
                .FirmwareInfo_V_1_2->usReferenceClock * 10;
          break;
        default:
          return ATOM_NOT_IMPLEMENTED;
	    }
	    break;
	case 3:
	    switch (func) {
		case GET_DEFAULT_ENGINE_CLOCK:
		    *val = atomDataPtr->FirmwareInfo
			.FirmwareInfo_V_1_3->ulDefaultEngineClock * 10;
		    break;
		case GET_DEFAULT_MEMORY_CLOCK:
		    *val = atomDataPtr->FirmwareInfo
			.FirmwareInfo_V_1_3->ulDefaultMemoryClock * 10;
		    break;
		case GET_MAX_PIXEL_CLOCK_PLL_OUTPUT:
		    *val = atomDataPtr->FirmwareInfo
			.FirmwareInfo_V_1_3->ulMaxPixelClockPLL_Output * 10;
		    break;
		case GET_MIN_PIXEL_CLOCK_PLL_OUTPUT:
		    *val = atomDataPtr->FirmwareInfo
			.FirmwareInfo_V_1_3->usMinPixelClockPLL_Output * 10;
		    break;
		case GET_MAX_PIXEL_CLOCK_PLL_INPUT:
		    *val = atomDataPtr->FirmwareInfo
			.FirmwareInfo_V_1_3->usMaxPixelClockPLL_Input * 10;
		    break;
		case GET_MIN_PIXEL_CLOCK_PLL_INPUT:
		    *val = atomDataPtr->FirmwareInfo
			.FirmwareInfo_V_1_3->usMinPixelClockPLL_Input * 10;
		    break;
		case GET_MAX_PIXEL_CLK:
		    *val = atomDataPtr->FirmwareInfo
			.FirmwareInfo_V_1_3->usMaxPixelClock * 10;
		    break;
		case GET_REF_CLOCK:
		    *val = atomDataPtr->FirmwareInfo
			.FirmwareInfo_V_1_3->usReferenceClock * 10;
		    break;
		default:
		    return ATOM_NOT_IMPLEMENTED;
	    }
	    break;
	case 4:
	    switch (func) {
		case GET_DEFAULT_ENGINE_CLOCK:
		    *val = atomDataPtr->FirmwareInfo
			.FirmwareInfo_V_1_4->ulDefaultEngineClock * 10;
		    break;
		case GET_DEFAULT_MEMORY_CLOCK:
		    *val = atomDataPtr->FirmwareInfo
			.FirmwareInfo_V_1_4->ulDefaultMemoryClock * 10;
		    break;
		case GET_MAX_PIXEL_CLOCK_PLL_INPUT:
		    *val = atomDataPtr->FirmwareInfo
			.FirmwareInfo_V_1_4->usMaxPixelClockPLL_Input * 10;
		    break;
		case GET_MIN_PIXEL_CLOCK_PLL_INPUT:
		    *val = atomDataPtr->FirmwareInfo
			.FirmwareInfo_V_1_4->usMinPixelClockPLL_Input * 10;
		    break;
		case GET_MAX_PIXEL_CLOCK_PLL_OUTPUT:
		    *val = atomDataPtr->FirmwareInfo
			.FirmwareInfo_V_1_4->ulMaxPixelClockPLL_Output * 10;
		    break;
		case GET_MIN_PIXEL_CLOCK_PLL_OUTPUT:
		    *val = atomDataPtr->FirmwareInfo
			.FirmwareInfo_V_1_4->usMinPixelClockPLL_Output * 10;
		    break;
		case GET_MAX_PIXEL_CLK:
		    *val = atomDataPtr->FirmwareInfo
			.FirmwareInfo_V_1_4->usMaxPixelClock * 10;
		    break;
		case GET_REF_CLOCK:
		    *val = atomDataPtr->FirmwareInfo
			.FirmwareInfo_V_1_4->usReferenceClock * 10;
		    break;
		default:
		    return ATOM_NOT_IMPLEMENTED;
	    }
	    break;
	default:
	    return ATOM_NOT_IMPLEMENTED;
    }
    return ATOM_SUCCESS;
}

/*
 *
 */
static AtomBiosResult
rhdAtomGetConditionalGoldenSetting(atomBiosHandlePtr handle,
			 AtomBiosRequestID func, AtomBiosArgPtr data)
{
    unsigned short *table = (unsigned short *)data->GoldenSettings.BIOSPtr;
    unsigned short entry_size = *(table++);

    RHDFUNC(handle);

    RHDDebug(handle->scrnIndex, "%s: testing 0x%4.4x\n",__func__,
	     data->GoldenSettings.value);

/* @@@ endian! */
    while (table < (unsigned short *)data->GoldenSettings.End) {
	RHDDebugCont("\t\t against: 0x%8.8x\n", table[1] << 16 | table[0]);
	if ((data->GoldenSettings.value >> 16) == table[1]) {
	    if ((data->GoldenSettings.value & 0xffff) <= table[0]) {
		data->GoldenSettings.BIOSPtr = (unsigned char *)(table + 2);
		return ATOM_SUCCESS;
	    }
	}
	table = (unsigned short *)(((unsigned char *)table) + entry_size);
    }
    return ATOM_FAILED;
}

#define Limit(n,max,name) ((n >= max) ? ( \
     dbgprintf(handle->scrnIndex,X_ERROR,"%s: %s %i exceeds maximum %i\n", \
               __func__,name,n,max), TRUE) : FALSE)

static const struct _rhd_connector_objs
{
    char *name;
    rhdConnectorType con;
} rhd_connector_objs[] = {
    { "NONE", RHD_CONNECTOR_NONE },
    { "SINGLE_LINK_DVI_I", RHD_CONNECTOR_DVI_SINGLE },
    { "DUAL_LINK_DVI_I", RHD_CONNECTOR_DVI },
    { "SINGLE_LINK_DVI_D", RHD_CONNECTOR_DVI_SINGLE },
    { "DUAL_LINK_DVI_D", RHD_CONNECTOR_DVI },
    { "VGA", RHD_CONNECTOR_VGA },
    { "COMPOSITE", RHD_CONNECTOR_TV },
    { "SVIDEO", RHD_CONNECTOR_TV, },
    { "YPrPb", RHD_CONNECTOR_TV, },
    { "D_CONNECTOR", RHD_CONNECTOR_NONE, },
    { "9PIN_DIN", RHD_CONNECTOR_NONE },
    { "SCART", RHD_CONNECTOR_TV },
    { "HDMI_TYPE_A", RHD_CONNECTOR_DVI_SINGLE },
    { "HDMI_TYPE_B", RHD_CONNECTOR_DVI },
    { "LVDS", RHD_CONNECTOR_PANEL },
    { "7PIN_DIN", RHD_CONNECTOR_TV },
    { "PCIE_CONNECTOR", RHD_CONNECTOR_PCIE },
    { "CROSSFIRE", RHD_CONNECTOR_NONE },
    { "HARDCODE_DVI", RHD_CONNECTOR_NONE },
    { "DISPLAYPORT", RHD_CONNECTOR_NONE}
};
static const int n_rhd_connector_objs = sizeof (rhd_connector_objs) / sizeof(struct _rhd_connector_objs);

static const struct _rhd_encoders
{
    char *name;
    rhdOutputType ot[2];
} rhd_encoders[] = {
    { "NONE", {RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }},
    { "INTERNAL_LVDS", { RHD_OUTPUT_LVDS, RHD_OUTPUT_NONE }},
    { "INTERNAL_TMDS1", { RHD_OUTPUT_TMDSA, RHD_OUTPUT_NONE }},
    { "INTERNAL_TMDS2", { RHD_OUTPUT_TMDSB, RHD_OUTPUT_NONE }},
    { "INTERNAL_DAC1", { RHD_OUTPUT_DACA, RHD_OUTPUT_NONE }},
    { "INTERNAL_DAC2", { RHD_OUTPUT_DACB, RHD_OUTPUT_NONE }},
    { "INTERNAL_SDVOA", { RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }},
    { "INTERNAL_SDVOB", { RHD_OUTPUT_NONE , RHD_OUTPUT_NONE }},
    { "SI170B", { RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }},
    { "CH7303", { RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }},
    { "CH7301", { RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }},
    { "INTERNAL_DVO1", { RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }},
    { "EXTERNAL_SDVOA", { RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }},
    { "EXTERNAL_SDVOB", { RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }},
    { "TITFP513", { RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }},
    { "INTERNAL_LVTM1", { RHD_OUTPUT_LVTMA, RHD_OUTPUT_NONE }},
    { "VT1623", { RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }},
    { "HDMI_SI1930", { RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }},
    { "HDMI_INTERNAL", { RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }},
    { "INTERNAL_KLDSCP_TMDS1", { RHD_OUTPUT_TMDSA, RHD_OUTPUT_NONE }},
    { "INTERNAL_KLDSCP_DVO1", { RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }},
    { "INTERNAL_KLDSCP_DAC1", { RHD_OUTPUT_DACA, RHD_OUTPUT_NONE }},
    { "INTERNAL_KLDSCP_DAC2", { RHD_OUTPUT_DACB, RHD_OUTPUT_NONE }},
    { "SI178", { RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }},
    { "MVPU_FPGA", { RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }},
    { "INTERNAL_DDI", { RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }},
    { "VT1625", { RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }},
    { "HDMI_SI1932", {RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }},
    { "AN9801", { RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }},
    { "DP501",  { RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }},
    { "UNIPHY",  { RHD_OUTPUT_UNIPHYA, RHD_OUTPUT_UNIPHYB }},
    { "KLDSCP_LVTMA", { RHD_OUTPUT_KLDSKP_LVTMA, RHD_OUTPUT_NONE }},
    { "UNIPHY1",  { RHD_OUTPUT_UNIPHYC, RHD_OUTPUT_UNIPHYD }},
    { "UNIPHY2",  { RHD_OUTPUT_UNIPHYE, RHD_OUTPUT_UNIPHYF }}
};
static const int n_rhd_encoders = sizeof (rhd_encoders) / sizeof(struct _rhd_encoders);

static const struct _rhd_connectors
{
    char *name;
    rhdConnectorType con;
    Bool dual;
} rhd_connectors[] = {
    {"NONE", RHD_CONNECTOR_NONE, FALSE },
    {"VGA", RHD_CONNECTOR_VGA, FALSE },
    {"DVI-I", RHD_CONNECTOR_DVI, TRUE },
    {"DVI-D", RHD_CONNECTOR_DVI, FALSE },
    {"DVI-A", RHD_CONNECTOR_DVI, FALSE },
    {"SVIDEO", RHD_CONNECTOR_TV, FALSE },
    {"COMPOSITE", RHD_CONNECTOR_TV, FALSE },
    {"PANEL", RHD_CONNECTOR_PANEL, FALSE },
    {"DIGITAL_LINK", RHD_CONNECTOR_NONE, FALSE },
    {"SCART", RHD_CONNECTOR_TV, FALSE },
    {"HDMI Type A", RHD_CONNECTOR_DVI_SINGLE, FALSE },
    {"HDMI Type B", RHD_CONNECTOR_DVI, FALSE },
    {"UNKNOWN", RHD_CONNECTOR_NONE, FALSE },
    {"UNKNOWN", RHD_CONNECTOR_NONE, FALSE },
    {"DVI+DIN", RHD_CONNECTOR_NONE, FALSE }
};
static const int n_rhd_connectors = sizeof(rhd_connectors) / sizeof(struct _rhd_connectors);

static const struct _rhd_devices
{
    char *name;
    rhdOutputType ot[2];
    enum atomDevice atomDevID;
} rhd_devices[] = { /* { RHD_CHIP_EXTERNAL, RHD_CHIP_IGP } */
    {" CRT1", { RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }, atomCRT1 },
    {" LCD1", { RHD_OUTPUT_LVTMA, RHD_OUTPUT_LVTMA }, atomLCD1 },
    {" TV1",  { RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }, atomTV1 },
    {" DFP1", { RHD_OUTPUT_TMDSA, RHD_OUTPUT_NONE }, atomDFP1 },
    {" CRT2", { RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }, atomCRT2 },
    {" LCD2", { RHD_OUTPUT_LVTMA, RHD_OUTPUT_NONE }, atomLCD2 },
    {" TV2",  { RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }, atomTV2 },
    {" DFP2", { RHD_OUTPUT_LVTMA, RHD_OUTPUT_DVO }, atomDFP2 },
    {" CV",   { RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }, atomCV },
    {" DFP3", { RHD_OUTPUT_LVTMA, RHD_OUTPUT_LVTMA }, atomDFP3 },
    {" DFP4", { RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }, atomDFP4 },
    {" DFP5", { RHD_OUTPUT_NONE, RHD_OUTPUT_NONE }, atomDFP5 }
};
static const int n_rhd_devices = sizeof(rhd_devices) / sizeof(struct _rhd_devices);

static const rhdDDC hwddc[] = { RHD_DDC_0, RHD_DDC_1, RHD_DDC_2, RHD_DDC_3, RHD_DDC_4 };
static const int n_hwddc = sizeof(hwddc) / sizeof(rhdDDC);

static const rhdOutputType acc_dac[] = { RHD_OUTPUT_NONE, RHD_OUTPUT_DACA,
				  RHD_OUTPUT_DACB, RHD_OUTPUT_DAC_EXTERNAL };
static const int n_acc_dac = sizeof(acc_dac) / sizeof (rhdOutputType);

/*
 *
 */
static Bool
rhdAtomInterpretObjectID(atomBiosHandlePtr handle,
			 CARD16 id, CARD8 *obj_type, CARD8 *obj_id,
			 CARD8 *num, char **name)
{
    *obj_id = (id & OBJECT_ID_MASK) >> OBJECT_ID_SHIFT;
    *num = (id & ENUM_ID_MASK) >> ENUM_ID_SHIFT;
    *obj_type = (id & OBJECT_TYPE_MASK) >> OBJECT_TYPE_SHIFT;

    *name = NULL;

    switch (*obj_type) {
	case GRAPH_OBJECT_TYPE_CONNECTOR:
	    if (!Limit(*obj_id, n_rhd_connector_objs, "connector_obj"))
		*name = rhd_connector_objs[*obj_id].name;
	    break;
	case GRAPH_OBJECT_TYPE_ENCODER:
	    if (!Limit(*obj_id, n_rhd_encoders, "encoder_obj"))
		*name = rhd_encoders[*obj_id].name;
	    break;
	default:
	    break;
    }
    return TRUE;
}

/*
 *
 */
static AtomBiosResult
rhdAtomGetDDCIndex(atomBiosHandlePtr handle,
		   rhdDDC *DDC, unsigned char i2c)
{
    atomDataTablesPtr atomDataPtr;
    CARD8 crev, frev;
    int i;

    RHDFUNC(handle);

    atomDataPtr = handle->atomDataPtr;

    if (!rhdAtomGetTableRevisionAndSize(
	    &(atomDataPtr->GPIO_I2C_Info->sHeader), &crev,&frev,NULL)) {
	return ATOM_NOT_IMPLEMENTED;
    }
    for (i = 0; i < ATOM_MAX_SUPPORTED_DEVICE; i++) {
	if (atomDataPtr->GPIO_I2C_Info->asGPIO_Info[i].sucI2cId.ucAccess == i2c) {
	    RHDDebug(handle->scrnIndex, " Found DDC GPIO Index: %i\n",i);
	    if (Limit(i, n_hwddc, "GPIO_DDC Index"))
		return ATOM_FAILED;
	    *DDC = hwddc[i];
	    return ATOM_SUCCESS;
	}
    }
    return ATOM_FAILED;
}

/*
 *
 */
static void
rhdAtomDDCFromI2CRecord(atomBiosHandlePtr handle,
			ATOM_I2C_RECORD *Record, rhdDDC *DDC)
{
    RHDDebug(handle->scrnIndex,
	     "   %s:  I2C Record: %s[%x] EngineID: %x I2CAddr: %x\n",
                __func__,
	     Record->sucI2cId.bfHW_Capable ? "HW_Line" : "GPIO_ID",
	     Record->sucI2cId.bfI2C_LineMux,
	     Record->sucI2cId.bfHW_EngineID,
	     Record->ucI2CAddr);

  if (!*(unsigned char *)&(Record->sucI2cId))
    *DDC = RHD_DDC_NONE;
    else {
	union {
	    ATOM_I2C_ID_CONFIG i2cId;
	    unsigned char i2cChar;
	} u;
    if (Record->ucI2CAddr != 0)
        return;
	    u.i2cId = Record->sucI2cId;
	    if (!u.i2cChar
		|| rhdAtomGetDDCIndex(handle, DDC, u.i2cChar) != ATOM_SUCCESS)
		*DDC = RHD_DDC_NONE;
  }
}

/*
 *
 */
static void
rhdAtomParseGPIOLutForHPD(atomBiosHandlePtr handle,
			  CARD8 pinID, rhdHPD *HPD)
{
    atomDataTablesPtr atomDataPtr;
    ATOM_GPIO_PIN_LUT *gpio_pin_lut;
    unsigned short size;
    int i = 0;

    RHDFUNC(handle);

    atomDataPtr = handle->atomDataPtr;

    *HPD = RHD_HPD_NONE;

    if (!rhdAtomGetTableRevisionAndSize(
	    &atomDataPtr->GPIO_Pin_LUT->sHeader, NULL, NULL, &size)) {
	xf86DrvMsg(handle->scrnIndex, X_ERROR,
		   "%s: No valid GPIO pin LUT in AtomBIOS\n",__func__);
      return;
    }
    gpio_pin_lut = atomDataPtr->GPIO_Pin_LUT;

    while (1) {
	if (gpio_pin_lut->asGPIO_Pin[i].ucGPIO_ID  == pinID) {

	    if ((sizeof(ATOM_COMMON_TABLE_HEADER)
		  + (i * sizeof(ATOM_GPIO_PIN_ASSIGNMENT))) > size)
		return;

	    RHDDebug(handle->scrnIndex,
		     "   %s: GPIO PinID: %i Index: %x Shift: %i\n",
		     __func__,
		     pinID,
		     gpio_pin_lut->asGPIO_Pin[i].usGpioPin_AIndex,
		     gpio_pin_lut->asGPIO_Pin[i].ucGpioPinBitShift);

	    /* grr... map backwards: register indices -> line numbers */
	    if (gpio_pin_lut->asGPIO_Pin[i].usGpioPin_AIndex
		== (DC_GPIO_HPD_A >> 2)) {
		switch (gpio_pin_lut->asGPIO_Pin[i].ucGpioPinBitShift) {
		    case 0:
			*HPD = RHD_HPD_0;
			return;
		    case 8:
			*HPD = RHD_HPD_1;
			return;
		    case 16:
			*HPD = RHD_HPD_2;
			return;
		    case 24:
			*HPD = RHD_HPD_3;
			return;
		}
	    }
	}
	i++;
    }
}

/*
 *
 */
static void
rhdAtomHPDFromRecord(atomBiosHandlePtr handle,
		     ATOM_HPD_INT_RECORD *Record, rhdHPD *HPD)
{
    RHDDebug(handle->scrnIndex,
	     "   %s:  HPD Record: GPIO ID: %x Plugged_PinState: %x\n",
	     __func__,
	     Record->ucHPDIntGPIOID,
	     Record->ucPluggged_PinState);
    rhdAtomParseGPIOLutForHPD(handle, Record->ucHPDIntGPIOID, HPD);
}

/*
 *
 */
static char *
rhdAtomDeviceTagsFromRecord(atomBiosHandlePtr handle,
			    ATOM_CONNECTOR_DEVICE_TAG_RECORD *Record)
{
    int i, j, k;
    char *devices;

    RHDFUNC(handle);

    RHDDebug(handle->scrnIndex,"   NumberOfDevice: %i\n",
	     Record->ucNumberOfDevice);

    if (!Record->ucNumberOfDevice) return NULL;

    devices = (char *)xcalloc(Record->ucNumberOfDevice * 4 + 1,1);

    for (i = 0; i < Record->ucNumberOfDevice; i++) {
	k = 0;
	j = Record->asDeviceTag[i].usDeviceID;

	if (!j) continue;

	while (!(j & 0x1)) { j >>= 1; k++; };

	if (!Limit(k,n_rhd_devices,"usDeviceID"))
	    strcat(devices, rhd_devices[k].name);
    }

    RHDDebug(handle->scrnIndex,"   Devices:%s\n",devices);

    return devices;
}

/*
 *
 */
static rhdConnectorType
rhdAtomGetConnectorID(atomBiosHandlePtr handle, rhdConnectorType connector, int num)
{
    RHDFUNC(handle);

    switch (connector) {
	case RHD_CONNECTOR_PCIE:
	{
	    atomDataTablesPtr atomDataPtr;
	    CARD8 crev, frev;
	    CARD32 val;

	    atomDataPtr = handle->atomDataPtr;

	    if (!rhdAtomGetTableRevisionAndSize(
		    (ATOM_COMMON_TABLE_HEADER *)(atomDataPtr->IntegratedSystemInfo.base),
		    &crev,&frev,NULL) || crev != 2) {
		    return RHD_CONNECTOR_NONE; 	    /* sorry, we can't do any better */
	    }
	    RHDDebug(handle->scrnIndex,"PCIE[%i]", num);
	    switch (num) {
		case 1:
		    val = atomDataPtr->IntegratedSystemInfo.IntegratedSystemInfo_v2->ulDDISlot1Config;
		    break;
		case 2:
		    val = atomDataPtr->IntegratedSystemInfo.IntegratedSystemInfo_v2->ulDDISlot2Config;
		    break;
		default:
		    RHDDebugCont("\n");
		    return RHD_CONNECTOR_NONE;
	    }
	    val >>= 16;
	    val &= 0xff;
	    RHDDebugCont(" ObjectID: %i",val);
	    if (Limit((int)val, n_rhd_connector_objs, "obj_id")) {
		RHDDebugCont("\n");
		return RHD_CONNECTOR_NONE;
	    }

	    RHDDebugCont(" ConnectorName: %s\n",rhd_connector_objs[val].name);
	    return  rhd_connector_objs[val].con;
	}
	default:
	    return connector;
    }
}

/*
 *
 */
static AtomBiosResult
rhdAtomOutputDeviceListFromObjectHeader(atomBiosHandlePtr handle,
				  struct rhdAtomOutputDeviceList **ptr)
{
    atomDataTablesPtr atomDataPtr;
    CARD8 crev, frev;
    ATOM_DISPLAY_OBJECT_PATH_TABLE *disObjPathTable;
    ATOM_DISPLAY_OBJECT_PATH *disObjPath;
    rhdConnectorInfoPtr cp;
    unsigned long object_header_end;
    unsigned int i,j;
    unsigned short object_header_size;
    struct rhdAtomOutputDeviceList *DeviceList = NULL;
    int cnt = 0;

    RHDFUNC(handle);

    atomDataPtr = handle->atomDataPtr;

    if (!rhdAtomGetTableRevisionAndSize(
	    &atomDataPtr->Object_Header->sHeader,
	    &crev,&frev,&object_header_size)) {
	return ATOM_NOT_IMPLEMENTED;
    }

    if (crev < 2) /* don't bother with anything below rev 2 */
	return ATOM_NOT_IMPLEMENTED;

    if (!(cp = (rhdConnectorInfoPtr)xcalloc(sizeof(struct rhdConnectorInfo),
					 RHD_CONNECTORS_MAX)))
	return ATOM_FAILED;

    object_header_end =
	atomDataPtr->Object_Header->usConnectorObjectTableOffset
	+ object_header_size;

    RHDDebug(handle->scrnIndex,"ObjectTable - size: %u, BIOS - size: %u "
	     "TableOffset: %u object_header_end: %u\n",
	     object_header_size, handle->BIOSImageSize,
	     atomDataPtr->Object_Header->usConnectorObjectTableOffset,
	     object_header_end);

    if ((object_header_size > handle->BIOSImageSize)
	|| (atomDataPtr->Object_Header->usConnectorObjectTableOffset
	    > handle->BIOSImageSize)
	|| object_header_end > handle->BIOSImageSize) {
	xf86DrvMsg(handle->scrnIndex, X_ERROR,
		   "%s: Object table information is bogus\n",__func__);
	return ATOM_FAILED;
    }

    if (((unsigned long)&atomDataPtr->Object_Header->sHeader
	 + object_header_end) >  ((unsigned long)handle->BIOSBase
		     + handle->BIOSImageSize)) {
	xf86DrvMsg(handle->scrnIndex, X_ERROR,
		   "%s: Object table extends beyond BIOS Image\n",__func__);
	return ATOM_FAILED;
    }
    disObjPathTable = (ATOM_DISPLAY_OBJECT_PATH_TABLE *)
	((char *)&atomDataPtr->Object_Header->sHeader +
	 atomDataPtr->Object_Header->usDisplayPathTableOffset);
    RHDDebug(handle->scrnIndex, "DisplayPathObjectTable: entries: %i version: %i\n",
	     disObjPathTable->ucNumOfDispPath, disObjPathTable->ucVersion);

    disObjPath = &disObjPathTable->asDispPath[0];
    for (i = 0; i < disObjPathTable->ucNumOfDispPath; i++) {
	CARD8 objNum, cObjNum;
	CARD8 objId;
	CARD8 objType;
	rhdConnectorType ct;
	char *name;

	rhdAtomInterpretObjectID(handle, disObjPath->usConnObjectId, &objType, &objId, &objNum, &name);
	RHDDebug(handle->scrnIndex, "  DisplaPathTable[%i]: size: %i DeviceTag: 0x%x ConnObjId: 0x%x NAME: %s GPUObjId: 0x%x\n",
		 i, disObjPath->usSize, disObjPath->usDeviceTag, disObjPath->usConnObjectId, name, disObjPath->usGPUObjectId);

	if (objType != GRAPH_OBJECT_TYPE_CONNECTOR)
	    continue;

	ct = rhd_connector_objs[objId].con;
	cObjNum = objNum;

	for (j = 0; j < disObjPath->usSize / sizeof(USHORT) - 4; j++) {
	    int k = 0,l;

	    rhdAtomInterpretObjectID(handle, disObjPath->usGraphicObjIds[j], &objType, &objId, &objNum, &name);
	    RHDDebug(handle->scrnIndex, "   GraphicsObj[%i] ID: 0x%x Type: 0x%x ObjID: 0x%x ENUM: 0x%x NAME: %s\n",
		     j, disObjPath->usGraphicObjIds[j], objType, objId, objNum, name);

	    if (objType != GRAPH_OBJECT_TYPE_ENCODER)
		continue;

	    Limit(objId, n_rhd_encoders, "usGraphicsObjId");

	    l = disObjPath->usDeviceTag;
	    if (!l) continue;

	    while (!(l & 0x1)) { l >>= 1; k++; };
	    if (!Limit(k,n_rhd_devices,"usDeviceID")) {
		if (!(DeviceList = (struct rhdAtomOutputDeviceList *)xrealloc(DeviceList, sizeof (struct rhdAtomOutputDeviceList) * (cnt + 1))))
		    return ATOM_FAILED;

		DeviceList[cnt].DeviceId = rhd_devices[k].atomDevID;
		DeviceList[cnt].ConnectorType = rhdAtomGetConnectorID(handle, ct, cObjNum);
		DeviceList[cnt].OutputType = rhd_encoders[objId].ot[objNum - 1];
		cnt++;
		RHDDebug(handle->scrnIndex, "   DeviceIndex: 0x%x\n",k);
	    }
	}
	disObjPath = (ATOM_DISPLAY_OBJECT_PATH*)(((char *)disObjPath) + disObjPath->usSize);
	if ((((unsigned long)&atomDataPtr->Object_Header->sHeader + object_header_end)
	     < (((unsigned long) disObjPath) + sizeof(ATOM_DISPLAY_OBJECT_PATH)))
	    || (((unsigned long)&atomDataPtr->Object_Header->sHeader + object_header_end)
		< (((unsigned long) disObjPath) + disObjPath->usSize)))
	    break;
    }
    DeviceList = xrealloc(DeviceList, sizeof(struct rhdAtomOutputDeviceList) * (cnt + 1));
    DeviceList[cnt].DeviceId = atomNone;

    *ptr = DeviceList;

    return ATOM_SUCCESS;
}

/*
 *
 */
static AtomBiosResult
rhdAtomConnectorInfoFromObjectHeader(atomBiosHandlePtr handle,
				     rhdConnectorInfoPtr *ptr)
{
    atomDataTablesPtr atomDataPtr;
    CARD8 crev, frev;
    ATOM_CONNECTOR_OBJECT_TABLE *con_obj;
    rhdConnectorInfoPtr cp;
    unsigned long object_header_end;
    int ncon = 0;
    int i,j;
    unsigned short object_header_size;

    RHDFUNC(handle);

    atomDataPtr = handle->atomDataPtr;

    if (!rhdAtomGetTableRevisionAndSize(
	    &atomDataPtr->Object_Header->sHeader,
	    &crev,&frev,&object_header_size)) {
	return ATOM_NOT_IMPLEMENTED;
    }

    if (crev < 2) /* don't bother with anything below rev 2 */
	return ATOM_NOT_IMPLEMENTED;

    if (!(cp = (rhdConnectorInfoPtr)xcalloc(sizeof(struct rhdConnectorInfo),
					 RHD_CONNECTORS_MAX)))
    return ATOM_FAILED;

    object_header_end =
	atomDataPtr->Object_Header->usConnectorObjectTableOffset
	+ object_header_size;

    RHDDebug(handle->scrnIndex,"ObjectTable - size: %u, BIOS - size: %u "
	     "TableOffset: %u object_header_end: %u\n",
	     object_header_size, handle->BIOSImageSize,
	     atomDataPtr->Object_Header->usConnectorObjectTableOffset,
	     object_header_end);

    if ((object_header_size > handle->BIOSImageSize)
	|| (atomDataPtr->Object_Header->usConnectorObjectTableOffset
	    > handle->BIOSImageSize)
	|| object_header_end > handle->BIOSImageSize) {
	xfree(cp);
	xf86DrvMsg(handle->scrnIndex, X_ERROR,
		   "%s: Object table information is bogus\n",__func__);
   return ATOM_FAILED;
    }

    if (((unsigned long)&atomDataPtr->Object_Header->sHeader
	 + object_header_end) >  ((unsigned long)handle->BIOSBase
		     + handle->BIOSImageSize)) {
	xfree(cp);
	xf86DrvMsg(handle->scrnIndex, X_ERROR,
		   "%s: Object table extends beyond BIOS Image\n",__func__);
    return ATOM_FAILED;
    }

    con_obj = (ATOM_CONNECTOR_OBJECT_TABLE *)
	((char *)&atomDataPtr->Object_Header->sHeader +
	 atomDataPtr->Object_Header->usConnectorObjectTableOffset);

    for (i = 0; i < con_obj->ucNumberOfObjects; i++) {
      ATOM_SRC_DST_TABLE_FOR_ONE_OBJECT *SrcDstTable;
      ATOM_COMMON_RECORD_HEADER *Record;
      int record_base;
      CARD8 obj_type, obj_id, num;
      char *name;

      rhdAtomInterpretObjectID(handle, con_obj->asObjects[i].usObjectID,
                               &obj_type, &obj_id, &num, &name);

	RHDDebug(handle->scrnIndex, "Object: ID: %x name: %s type: %x id: %x\n",
                 con_obj->asObjects[i].usObjectID, name ? name : "",
		 obj_type, obj_id);


     if (obj_type != GRAPH_OBJECT_TYPE_CONNECTOR)
       continue;

     SrcDstTable = (ATOM_SRC_DST_TABLE_FOR_ONE_OBJECT *)
                   ((char *)&atomDataPtr->Object_Header->sHeader
                    + con_obj->asObjects[i].usSrcDstTableOffset);

     if (con_obj->asObjects[i].usSrcDstTableOffset
         + (SrcDstTable->ucNumberOfSrc
	       * sizeof(ATOM_SRC_DST_TABLE_FOR_ONE_OBJECT))
	    > handle->BIOSImageSize) {
	    xf86DrvMsg(handle->scrnIndex, X_ERROR, "%s: SrcDstTable[%i] extends "
		       "beyond Object_Header table\n",__func__,i);
       continue;
     }
	cp[ncon].Type = rhdAtomGetConnectorID(handle, rhd_connector_objs[obj_id].con, num);
     cp[ncon].Name = RhdAppendString(cp[ncon].Name,name);

	for (j = 0; ((j < SrcDstTable->ucNumberOfSrc) &&
		     (j < MAX_OUTPUTS_PER_CONNECTOR)); j++) {
       CARD8 stype, sobj_id, snum;
       char *sname;

       rhdAtomInterpretObjectID(handle, SrcDstTable->usSrcObjectID[j],
              &stype, &sobj_id, &snum, &sname);

	    RHDDebug(handle->scrnIndex, " * SrcObject: ID: %x name: %s enum: %i\n",
		     SrcDstTable->usSrcObjectID[j], sname, snum);

	    if (snum <= 2)
		cp[ncon].Output[j] = rhd_encoders[sobj_id].ot[snum - 1];
     }

	Record = (ATOM_COMMON_RECORD_HEADER *)
	    ((char *)&atomDataPtr->Object_Header->sHeader
	     + con_obj->asObjects[i].usRecordOffset);

	record_base = con_obj->asObjects[i].usRecordOffset;

	while (Record->ucRecordType > 0
	       && Record->ucRecordType <= ATOM_MAX_OBJECT_RECORD_NUMBER ) {
	    char *taglist;

	    if ((record_base += Record->ucRecordSize)
		> object_header_size) {
		xf86DrvMsg(handle->scrnIndex, X_ERROR,
			   "%s: Object Records extend beyond Object Table\n",
			   __func__);
		break;
	    }

	    RHDDebug(handle->scrnIndex, " - Record Type: %x\n",
		     Record->ucRecordType);

	    switch (Record->ucRecordType) {

		case ATOM_I2C_RECORD_TYPE:
		    rhdAtomDDCFromI2CRecord(handle,
					    (ATOM_I2C_RECORD *)Record,
					    &cp[ncon].DDC);
		    break;

		case ATOM_HPD_INT_RECORD_TYPE:
		    rhdAtomHPDFromRecord(handle,
					 (ATOM_HPD_INT_RECORD *)Record,
					 &cp[ncon].HPD);
		    break;

		case ATOM_CONNECTOR_DEVICE_TAG_RECORD_TYPE:
		    taglist = rhdAtomDeviceTagsFromRecord(handle,
							  (ATOM_CONNECTOR_DEVICE_TAG_RECORD *)Record);
		    if (taglist) {
			cp[ncon].Name = RhdAppendString(cp[ncon].Name,taglist);
			xfree(taglist);
		    }
		    break;

		default:
		    break;
	    }

	    Record = (ATOM_COMMON_RECORD_HEADER*)
		((char *)Record + Record->ucRecordSize);

	}

	if ((++ncon) == RHD_CONNECTORS_MAX)
	    break;
    }
    *ptr = cp;

    RhdPrintConnectorInfo(handle->rhdPtr, cp);

    return ATOM_SUCCESS;
}

/*
 *
 */
static AtomBiosResult
rhdAtomOutputDeviceListFromSupportedDevices(atomBiosHandlePtr handle,
					     Bool igp,
					     struct rhdAtomOutputDeviceList **Ptr)
{
    atomDataTablesPtr atomDataPtr;
    CARD8 crev, frev;
    int n;
    int cnt = 0;
    struct rhdAtomOutputDeviceList *DeviceList = NULL;
    struct rhdConnectorInfo *cp;

    RHDFUNC(handle);

    atomDataPtr = handle->atomDataPtr;

    if (!rhdAtomGetTableRevisionAndSize(
	    &(atomDataPtr->SupportedDevicesInfo.SupportedDevicesInfo->sHeader),
	    &crev,&frev,NULL)) {
	return ATOM_NOT_IMPLEMENTED;
    }

    if (!(cp = (rhdConnectorInfoPtr)xcalloc(RHD_CONNECTORS_MAX,
					 sizeof(struct rhdConnectorInfo))))
	return ATOM_FAILED;

    for (n = 0; n < ATOM_MAX_SUPPORTED_DEVICE; n++) {
	ATOM_CONNECTOR_INFO_I2C ci
	    = atomDataPtr->SupportedDevicesInfo.SupportedDevicesInfo->asConnInfo[n];

	if (!(atomDataPtr->SupportedDevicesInfo
	      .SupportedDevicesInfo->usDeviceSupport & (1 << n)))
	    continue;

	if (Limit(ci.sucConnectorInfo.sbfAccess.bfConnectorType,
		  n_rhd_connectors, "bfConnectorType"))
	    continue;

	if (!(DeviceList = (struct rhdAtomOutputDeviceList *)xrealloc(DeviceList, sizeof(struct rhdAtomOutputDeviceList) * (cnt + 1))))
	    return ATOM_FAILED;

	DeviceList[cnt].ConnectorType = rhd_connectors[ci.sucConnectorInfo.sbfAccess.bfConnectorType].con;
	DeviceList[cnt].DeviceId = rhd_devices[n].atomDevID;

	if (!Limit(ci.sucConnectorInfo.sbfAccess.bfAssociatedDAC,
		   n_acc_dac, "bfAssociatedDAC")) {
	    if ((DeviceList[cnt].OutputType
		 = acc_dac[ci.sucConnectorInfo.sbfAccess.bfAssociatedDAC])
		== RHD_OUTPUT_NONE) {
		DeviceList[cnt].OutputType = rhd_devices[n].ot[igp ? 1 : 0];
	    }
	    cnt++;
	}
    }
    DeviceList = (struct rhdAtomOutputDeviceList *)xrealloc(DeviceList, sizeof(struct rhdAtomOutputDeviceList) * (cnt + 1));
    DeviceList[cnt].DeviceId = atomNone;

    *Ptr = DeviceList;

    return ATOM_SUCCESS;
}

/*
 *
 */
static AtomBiosResult
rhdAtomConnectorInfoFromSupportedDevices(atomBiosHandlePtr handle,
					 Bool igp,
					 rhdConnectorInfoPtr *ptr)
{
  atomDataTablesPtr atomDataPtr;
  CARD8 crev, frev;
  rhdConnectorInfoPtr cp;
    struct {
    rhdOutputType ot;
    rhdConnectorType con;
    rhdDDC ddc;
    rhdHPD hpd;
    Bool dual;
    char *name;
    char *outputName;
  } devices[ATOM_MAX_SUPPORTED_DEVICE];
  int ncon = 0;
  int n;

  RHDFUNC(handle);

  atomDataPtr = handle->atomDataPtr;

  if (!rhdAtomGetTableRevisionAndSize(
	    &(atomDataPtr->SupportedDevicesInfo.SupportedDevicesInfo->sHeader),
	    &crev,&frev,NULL)) {
	return ATOM_NOT_IMPLEMENTED;
    }

    if (!(cp = (rhdConnectorInfoPtr)xcalloc(RHD_CONNECTORS_MAX,
           sizeof(struct rhdConnectorInfo))))
      return ATOM_FAILED;

    for (n = 0; n < ATOM_MAX_SUPPORTED_DEVICE; n++) {
    ATOM_CONNECTOR_INFO_I2C ci
	    = atomDataPtr->SupportedDevicesInfo.SupportedDevicesInfo->asConnInfo[n];

	devices[n].ot = RHD_OUTPUT_NONE;

	if (!(atomDataPtr->SupportedDevicesInfo
	      .SupportedDevicesInfo->usDeviceSupport & (1 << n)))
	    continue;

	if (Limit(ci.sucConnectorInfo.sbfAccess.bfConnectorType,
		  n_rhd_connectors, "bfConnectorType"))
	    continue;

	devices[n].con
	    = rhd_connectors[ci.sucConnectorInfo.sbfAccess.bfConnectorType].con;
	if (devices[n].con == RHD_CONNECTOR_NONE)
	    continue;

	devices[n].dual
	    = rhd_connectors[ci.sucConnectorInfo.sbfAccess.bfConnectorType].dual;
	devices[n].name
	    = rhd_connectors[ci.sucConnectorInfo.sbfAccess.bfConnectorType].name;

	RHDDebug(handle->scrnIndex,"AtomBIOS Connector[%i]: %s Device:%s ",n,
		 rhd_connectors[ci.sucConnectorInfo
				.sbfAccess.bfConnectorType].name,
		 rhd_devices[n].name);

	devices[n].outputName = rhd_devices[n].name;

	if (!Limit(ci.sucConnectorInfo.sbfAccess.bfAssociatedDAC,
		   n_acc_dac, "bfAssociatedDAC")) {
	    if ((devices[n].ot
		 = acc_dac[ci.sucConnectorInfo.sbfAccess.bfAssociatedDAC])
		== RHD_OUTPUT_NONE) {
		devices[n].ot = rhd_devices[n].ot[igp ? 1 : 0];
	    }
	} else
	    devices[n].ot = RHD_OUTPUT_NONE;

	RHDDebugCont("Output: %x ",devices[n].ot);

	if (!ci.sucI2cId.ucAccess
	    || rhdAtomGetDDCIndex(handle, &devices[n].ddc, ci.sucI2cId.ucAccess) != ATOM_SUCCESS) {
	    RHDDebugCont("NO DDC ");
		devices[n].ddc = RHD_DDC_NONE;
	} else
	    RHDDebugCont("HW DDC %i ",
			 ci.sucI2cId.sbfAccess.bfI2C_LineMux);

	if (crev > 1) {
	    ATOM_CONNECTOR_INC_SRC_BITMAP isb
		= atomDataPtr->SupportedDevicesInfo
		.SupportedDevicesInfo_HD->asIntSrcInfo[n];

	    switch (isb.ucIntSrcBitmap) {
		case 0x4:
		    RHDDebugCont("HPD 0\n");
		    devices[n].hpd = RHD_HPD_0;
		    break;
		case 0xa:
		    RHDDebugCont("HPD 1\n");
		    devices[n].hpd = RHD_HPD_1;
		    break;
		default:
		    RHDDebugCont("NO HPD\n");
		    devices[n].hpd = RHD_HPD_NONE;
		    break;
	    }
	} else {
	    RHDDebugCont("NO HPD\n");
	    devices[n].hpd = RHD_HPD_NONE;
	}
    }
    /* sort devices for connectors */
    for (n = 0; n < ATOM_MAX_SUPPORTED_DEVICE; n++) {
	int i;

	if (devices[n].ot == RHD_OUTPUT_NONE)
	    continue;
	if (devices[n].con == RHD_CONNECTOR_NONE)
	    continue;

	cp[ncon].DDC = devices[n].ddc;
	cp[ncon].HPD = devices[n].hpd;
	cp[ncon].Output[0] = devices[n].ot;
	cp[ncon].Output[1] = RHD_OUTPUT_NONE;
	cp[ncon].Type = devices[n].con;
	cp[ncon].Name = strdup(devices[n].name);
	cp[ncon].Name = RhdAppendString(cp[ncon].Name, devices[n].outputName);

	if (devices[n].dual) {
    if (devices[n].ddc == RHD_DDC_NONE)
		xf86DrvMsg(handle->scrnIndex, X_ERROR,
			   "No DDC channel for device %s found."
			   " Cannot find matching device.\n",devices[n].name);
	    else {
		for (i = n + 1; i < ATOM_MAX_SUPPORTED_DEVICE; i++) {

		    if (!devices[i].dual)
          continue;

		    if (devices[n].ddc != devices[i].ddc)
          continue;

		    if (((devices[n].ot == RHD_OUTPUT_DACA
           || devices[n].ot == RHD_OUTPUT_DACB)
           && (devices[i].ot == RHD_OUTPUT_LVTMA
			     || devices[i].ot == RHD_OUTPUT_TMDSA))
           || ((devices[i].ot == RHD_OUTPUT_DACA
			     || devices[i].ot == RHD_OUTPUT_DACB)
           && (devices[n].ot == RHD_OUTPUT_LVTMA
				|| devices[n].ot == RHD_OUTPUT_TMDSA))) {

          cp[ncon].Output[1] = devices[i].ot;

          if (cp[ncon].HPD == RHD_HPD_NONE)
            cp[ncon].HPD = devices[i].hpd;

          cp[ncon].Name = RhdAppendString(cp[ncon].Name,
                          devices[i].outputName);
          devices[i].ot = RHD_OUTPUT_NONE; /* zero the device */
		    }
      }
    }
	}
	/* Some connector table mark a VGA as DVI-X. This heuristic fixes it */
	if (cp[ncon].Type == RHD_CONNECTOR_DVI) {
	    if ( ((cp[ncon].Output[0] == RHD_OUTPUT_NONE
		  && (cp[ncon].Output[1] == RHD_OUTPUT_DACA
		      || cp[ncon].Output[1] == RHD_OUTPUT_DACB))
		 || (cp[ncon].Output[1] == RHD_OUTPUT_NONE
		     && (cp[ncon].Output[0] == RHD_OUTPUT_DACA
			 || cp[ncon].Output[0] == RHD_OUTPUT_DACB)))
		 && cp[ncon].HPD == RHD_HPD_NONE)
		cp[ncon].Type = RHD_CONNECTOR_VGA;
	}

	if ((++ncon) == RHD_CONNECTORS_MAX)
	    break;
    }
    *ptr = cp;

  RhdPrintConnectorInfo(handle->rhdPtr, cp);

  return ATOM_SUCCESS;
}

/*
 *
 */
static AtomBiosResult
rhdAtomConnectorInfo(atomBiosHandlePtr handle,
		     AtomBiosRequestID unused, AtomBiosArgPtr data)
{
    int chipset = data->chipset;

    RHDFUNC(handle);

    if (rhdAtomConnectorInfoFromObjectHeader(handle,&data->ConnectorInfo)
	== ATOM_SUCCESS)
	return ATOM_SUCCESS;
    else {
	Bool igp = RHDIsIGP(chipset);
	return rhdAtomConnectorInfoFromSupportedDevices(handle, igp,
							&data->ConnectorInfo);
    }
}

/*
 *
 */
static AtomBiosResult
rhdAtomOutputDeviceList(atomBiosHandlePtr handle,
		     AtomBiosRequestID unused, AtomBiosArgPtr data)
{
    int chipset = data->chipset;

    RHDFUNC(handle);

    if (rhdAtomOutputDeviceListFromObjectHeader(handle, &data->OutputDeviceList)
	== ATOM_SUCCESS) {
	return ATOM_SUCCESS;
    } else {
	    Bool igp = RHDIsIGP(chipset);
	    return rhdAtomOutputDeviceListFromSupportedDevices(handle, igp, &data->OutputDeviceList);
    }
}

/*
 *
 */
struct atomCodeDataTableHeader
{
    unsigned char signature;
    unsigned short size;
};

#define CODE_DATA_TABLE_SIGNATURE 0x7a
#define ATOM_EOT_COMMAND 0x5b

static AtomBiosResult
rhdAtomGetDataInCodeTable(atomBiosHandlePtr handle,
			   AtomBiosRequestID unused, AtomBiosArgPtr data)
{
    unsigned char *command_table;
    unsigned short size;
    unsigned short offset;

    int i;

    RHDFUNC(handle);

    if (data->val > sizeof (struct _ATOM_MASTER_LIST_OF_COMMAND_TABLES) / sizeof (USHORT))
	return ATOM_FAILED;

    if ((offset = ((USHORT *)&(((ATOM_MASTER_COMMAND_TABLE *)handle->codeTable)
			       ->ListOfCommandTables))[data->val]))
	command_table = handle->BIOSBase + offset;
    else
	return ATOM_FAILED;

    if (!rhdAtomGetTableRevisionAndSize(&(((ATOM_COMMON_ROM_COMMAND_TABLE_HEADER *)
					     command_table)->CommonHeader),
					NULL, NULL, &size))
	return ATOM_FAILED;

    for (i = sizeof(ATOM_COMMON_ROM_COMMAND_TABLE_HEADER); i < size - 1; i++) {

	if (command_table[i] == ATOM_EOT_COMMAND
	    && command_table[i+1] == CODE_DATA_TABLE_SIGNATURE) {
	    unsigned short *dt_size = (unsigned short*)(command_table + i + 2);

	    int diff;

	    diff = size - (i + 1) + sizeof(struct atomCodeDataTableHeader) + *dt_size;

      DBG(dbgprintf("Table[0x%2.2x] = 0x%4.4x -> data_size: 0x%x\n",data->val, size, *dt_size));

	    if (diff < 0) {
		xf86DrvMsg(handle->scrnIndex, X_ERROR,
			   "Data table in command table %li extends %i bytes "
			   "beyond command table size\n",
			   (unsigned long) data->val, -diff);

		return  ATOM_FAILED;
	    }
	    data->CommandDataTable.loc =
		command_table + i + 2 + sizeof(unsigned short);

	    data->CommandDataTable.size = *dt_size;
//      DEBUGP(RhdDebugDump(handle->scrnIndex, data->CommandDataTable.loc, *dt_size));

	    return ATOM_SUCCESS;
	}
    }

    return ATOM_FAILED;
}

# ifdef ATOM_BIOS_PARSER
static AtomBiosResult
rhdAtomExec (atomBiosHandlePtr handle,
	     AtomBiosRequestID unused, AtomBiosArgPtr data)
{
  RHDPtr rhdPtr = handle->rhdPtr;
  Bool ret = FALSE;
  char *msg;
  int idx = data->exec.index;
  void *pspace = data->exec.pspace;
  pointer *dataSpace = data->exec.dataSpace;

    RHDFUNCI(handle->scrnIndex);

    if (dataSpace) {
    if (!handle->fbBase && !handle->scratchBase)
	    return ATOM_FAILED;
	if (handle->fbBase) {
	    if (!rhdPtr->FbBase) {
		xf86DrvMsg(handle->scrnIndex, X_ERROR, "%s: "
			   "Cannot exec AtomBIOS: framebuffer not mapped\n",
			   __func__);
		return ATOM_FAILED;
	    }
	    *dataSpace = (CARD8*)rhdPtr->FbBase + handle->fbBase;
	} else
	    *dataSpace = (CARD8*)handle->scratchBase;
    }
    ret = ParseTableWrapper(pspace, idx, handle,
			    handle->BIOSBase,
			    &msg);
  if (!ret)
	xf86DrvMsg(handle->scrnIndex, X_ERROR, "%s\n",msg);
  else
	xf86DrvMsgVerb(handle->scrnIndex, X_INFO, 5, "%s\n",msg);

  return (ret) ? ATOM_SUCCESS : ATOM_FAILED;
}
# endif

AtomBiosResult
RHDAtomBiosFunc(RHDPtr rhdPtr, atomBiosHandlePtr handle,
		AtomBiosRequestID id, AtomBiosArgPtr data)
{
  AtomBiosResult ret = ATOM_FAILED;
  int scrnIndex = rhdPtr->scrnIndex;
  int i;
  char *msg = NULL;
  enum msgDataFormat msg_f = MSG_FORMAT_NONE;
  AtomBiosRequestFunc req_func = NULL;

    RHDFUNCI(scrnIndex);

    for (i = 0; AtomBiosRequestList[i].id != FUNC_END; i++) {
	if (id ==  AtomBiosRequestList[i].id) {
	    req_func = AtomBiosRequestList[i].request;
	    msg = AtomBiosRequestList[i].message;
	    msg_f = AtomBiosRequestList[i].message_format;
	    break;
    }
  }

    if (req_func == NULL) {
	xf86DrvMsg(scrnIndex, X_ERROR, "Unknown AtomBIOS request: %i\n",id);
    return ATOM_NOT_IMPLEMENTED;
  }
    /* Hack for now */
  if (id == ATOMBIOS_INIT)
    data->val = (CARD32)rhdPtr;

  if (id == ATOMBIOS_INIT || handle)
    ret = req_func(handle, id, data);

    if (ret == ATOM_SUCCESS) {

	switch (msg_f) {
	    case MSG_FORMAT_DEC:
		xf86DrvMsg(scrnIndex,X_INFO,"%s: %li\n", msg,
			   (unsigned long) data->val);
        break;
	    case MSG_FORMAT_HEX:
		xf86DrvMsg(scrnIndex,X_INFO,"%s: 0x%lx\n",msg ,
			   (unsigned long) data->val);
        break;
	    case MSG_FORMAT_NONE:
		xf86DrvMsgVerb(scrnIndex, 7, X_INFO,
			       "Call to %s succeeded\n", msg);
        break;
    }

    } else {

	char *result = (ret == ATOM_FAILED) ? "failed"
          : "not implemented";
	switch (msg_f) {
	    case MSG_FORMAT_DEC:
	    case MSG_FORMAT_HEX:
		xf86DrvMsgVerb(scrnIndex, 1, X_WARNING,
			       "Call to %s %s\n", msg, result);
		break;
	    case MSG_FORMAT_NONE:
		xf86DrvMsg(scrnIndex,X_INFO,"Query for %s: %s\n", msg, result);
		    break;
    }
  }
  return ret;
}

/*
 *
 */
static void
atomRegisterSaveList(atomBiosHandlePtr handle, struct atomSaveListRecord **SaveList)
{
    struct atomSaveListObject *ListObject = handle->SaveListObjects;
    RHDFUNC(handle);

    while (ListObject) {
	if (ListObject->SaveList == SaveList)
	    return;
	ListObject = ListObject->next;
    }
    if (!(ListObject = (struct atomSaveListObject *)xcalloc(1,sizeof (struct atomSaveListObject))))
	return;
    ListObject->next = handle->SaveListObjects;
    ListObject->SaveList = SaveList;
    handle->SaveListObjects = ListObject;
}

/*
 *
 */
static void
atomUnregisterSaveList(atomBiosHandlePtr handle, struct atomSaveListRecord **SaveList)
{
    struct atomSaveListObject **ListObject;
    RHDFUNC(handle);

    if (!handle->SaveListObjects)
	return;
    ListObject  = &handle->SaveListObjects;

    while (1) {
	if ((*ListObject)->SaveList == SaveList) {
	    struct atomSaveListObject *tmp = *ListObject;
	    *ListObject = ((*ListObject)->next);
	    xfree(tmp);
	}
	if (!(*ListObject) || !(*ListObject)->next)
	    return;
	ListObject = &((*ListObject)->next);
    }
}

/*
 *
 */
static AtomBiosResult
atomSetRegisterListLocation(atomBiosHandlePtr handle, AtomBiosRequestID func, AtomBiosArgPtr data)
{
    RHDFUNC(handle);

    handle->SaveList = (struct atomSaveListRecord **)data->Address;
    if (handle->SaveList)
	atomRegisterSaveList(handle, handle->SaveList);

    return ATOM_SUCCESS;
}

/*
 *
 */
static AtomBiosResult
atomRestoreRegisters(atomBiosHandlePtr handle, AtomBiosRequestID func, AtomBiosArgPtr data)
{
    struct atomSaveListRecord *List = *(data->Address);
    int i;

    RHDFUNC(handle);

    if (!List)
	return ATOM_FAILED;

    for (i = 0; i < List->Last; i++) {
	switch ( List->RegisterList[i].Type) {
	    case atomRegisterMMIO:
		RHDDebugVerb(handle->scrnIndex,1, "%s[%i]: MMIO(0x%4.4x) = 0x%4.4x\n",__func__, List->Last,
			      List->RegisterList[i].Address, List->RegisterList[i].Value);
		RHDRegWrite(handle, List->RegisterList[i].Address, List->RegisterList[i].Value);
		break;
	    case atomRegisterMC:
		RHDDebugVerb(handle->scrnIndex,1, "%s[%i]: MC(0x%4.4x) = 0x%4.4x\n",__func__, List->Last,
			      List->RegisterList[i].Address, List->RegisterList[i].Value);
		RHDWriteMC(handle,  List->RegisterList[i].Address | MC_IND_ALL | MC_IND_WR_EN,
			   List->RegisterList[i].Value);
		break;
	    case atomRegisterPLL:
		RHDDebugVerb(handle->scrnIndex,1, "%s[%i]: PLL(0x%4.4x) = 0x%4.4x\n",__func__, List->Last,
			      List->RegisterList[i].Address, List->RegisterList[i].Value);
		_RHDWritePLL(handle->scrnIndex, List->RegisterList[i].Address, List->RegisterList[i].Value);
		break;
	    case atomRegisterPCICFG:
		RHDDebugVerb(handle->scrnIndex,1, "%s[%i]: PCICFG(0x%4.4x) = 0x%4.4x\n",__func__,List->Last,
			      List->RegisterList[i].Address, List->RegisterList[i].Value);
#ifdef XSERVER_LIBPCIACCESS
		pci_device_cfg_write(RHDPTRI(handle)->PciInfo,
				     &List->RegisterList[i].Value,
				     List->RegisterList[i].Address, 4, NULL);
#else
		{
		    PCITAG tag = RHDPTRI(handle)->PciTag;
		    pciWriteLong(tag, List->RegisterList[i].Address,
				 List->RegisterList[i].Value);
		}
#endif
		break;
	}
    }

    /* deallocate list */
    atomUnregisterSaveList(handle, (struct atomSaveListRecord **)data->Address);
    xfree(List);
    *(data->Address) = NULL;

    return ATOM_SUCCESS;
}

# ifdef ATOM_BIOS_PARSER

#define ALLOC_CNT 25

/*
 *
 */
static void
atomSaveRegisters(atomBiosHandlePtr handle, enum atomRegisterType Type, CARD32 address)
{
    struct atomSaveListRecord *List;
    CARD32 val = 0;
    int i;
    struct atomSaveListObject *SaveListObj = handle->SaveListObjects;

    RHDFUNC(handle);

    if (!handle->SaveList)
	return;

    if (!(*(handle->SaveList))) {
	if (!(*handle->SaveList = (struct atomSaveListRecord *)xalloc(sizeof(struct atomSaveListRecord)
								  + sizeof(struct  atomRegisterList) * (ALLOC_CNT - 1))))
	    return;
	(*(handle->SaveList))->Length = ALLOC_CNT;
	(*(handle->SaveList))->Last = 0;
    } else if ((*(handle->SaveList))->Length == (*(handle->SaveList))->Last) {
	if (!(List = (struct atomSaveListRecord *)xrealloc(*handle->SaveList,
						      sizeof(struct atomSaveListRecord)
						      + (sizeof(struct  atomRegisterList)
							 * ((*(handle->SaveList))->Length + ALLOC_CNT - 1)))))
	    return;
	*handle->SaveList = List;
	List->Length = (*(handle->SaveList))->Length + ALLOC_CNT;
    }
    List = *handle->SaveList;

    while (SaveListObj) {
	struct atomSaveListRecord *ListFromObj = *(SaveListObj->SaveList);

	if (ListFromObj) {
	    for (i = 0; i < ListFromObj->Last; i++)
		if (ListFromObj->RegisterList[i].Address == address
		    && ListFromObj->RegisterList[i].Type == Type)
		    return;
	}
	SaveListObj = SaveListObj->next;
    }

    switch (Type) {
	case atomRegisterMMIO:
	    val = RHDRegRead(handle, address);
	    RHDDebugVerb(handle->scrnIndex,1, "%s[%i]: MMIO(0x%4.4x) = 0x%4.4x\n",__func__,List->Last,address,val);
	    break;
	case atomRegisterMC:
	    val = RHDReadMC(handle, address | MC_IND_ALL);
	    RHDDebugVerb(handle->scrnIndex,1, "%s[%i]: MC(0x%4.4x) = 0x%4.4x\n",__func__,List->Last,address,val);
	    break;
	case atomRegisterPLL:
	    val = _RHDReadPLL(handle->scrnIndex, address);
	    RHDDebugVerb(handle->scrnIndex,1, "%s[%i]: PLL(0x%4.4x) = 0x%4.4x\n",__func__,List->Last,address,val);
	    break;
	case atomRegisterPCICFG:
#ifdef XSERVER_LIBPCIACCESS
	    val = pci_device_cfg_write(RHDPTRI(handle)->PciInfo,
				       &val, address, 4, NULL);
#else
	    {
		PCITAG tag = RHDPTRI(handle)->PciTag;
		val =  pciReadLong(tag, address);
	    }
#endif
	    RHDDebugVerb(handle->scrnIndex,1, "%s[%i]: PCICFG(0x%4.4x) = 0x%4.4x\n",__func__,List->Last,address,val);
	    break;
    }
    List->RegisterList[List->Last].Address = address;
    List->RegisterList[List->Last].Value = val;
    List->RegisterList[List->Last].Type = Type;
    List->Last++;
}

/*
 *
 */
VOID*
CailAllocateMemory(VOID *CAIL,UINT16 size)
{
    CAILFUNC(CAIL);

    return malloc(size);
}

VOID
CailReleaseMemory(VOID *CAIL, VOID *addr)
{
    CAILFUNC(CAIL);

    free(addr);
}

VOID
CailDelayMicroSeconds(VOID *CAIL, UINT32 delay)
{
    CAILFUNC(CAIL);

    usleep(delay);

  //  DEBUGP(xf86DrvMsg(((atomBiosHandlePtr)CAIL)->scrnIndex,X_INFO,"Delay %i usec\n",delay));
}

UINT32
CailReadATIRegister(VOID* CAIL, UINT32 idx)
{
    UINT32 ret;
    CAILFUNC(CAIL);

    ret  =  RHDRegRead(((atomBiosHandlePtr)CAIL), idx << 2);
//    DEBUGP(ErrorF("%s(%x) = %x\n",__func__,idx << 2,ret));
    return ret;
}

VOID
CailWriteATIRegister(VOID *CAIL, UINT32 idx, UINT32 data)
{
    CAILFUNC(CAIL);

    atomSaveRegisters((atomBiosHandlePtr)CAIL, atomRegisterMMIO, idx << 2);

    RHDRegWrite(((atomBiosHandlePtr)CAIL),idx << 2,data);
    RHDDebugVerb(((atomBiosHandlePtr)CAIL)->scrnIndex,1,"%s(%x,%x)\n",__func__,idx << 2,data);
}

UINT32
CailReadFBData(VOID* CAIL, UINT32 idx)
{
  UINT32 ret;

  CAILFUNC(CAIL);

    if (((atomBiosHandlePtr)CAIL)->fbBase) {
	CARD8 *FBBase = (CARD8*)
	    RHDPTRI((atomBiosHandlePtr)CAIL)->FbBase;
    ret =  *((CARD32*)(FBBase + (((atomBiosHandlePtr)CAIL)->fbBase) + idx));
	RHDDebugVerb(((atomBiosHandlePtr)CAIL)->scrnIndex,1,"%s(%x) = %x\n",__func__,idx,ret);
    } else if (((atomBiosHandlePtr)CAIL)->scratchBase) {
    ret = *(CARD32*)((CARD8*)(((atomBiosHandlePtr)CAIL)->scratchBase) + idx);
	RHDDebugVerb(((atomBiosHandlePtr)CAIL)->scrnIndex,1,"%s(%x) = %x\n",__func__,idx,ret);
    } else {
	xf86DrvMsg(((atomBiosHandlePtr)CAIL)->scrnIndex,X_ERROR,
		   "%s: no fbbase set\n",__func__);
    return 0;
  }
  return ret;
}

VOID
CailWriteFBData(VOID *CAIL, UINT32 idx, UINT32 data)
{
  CAILFUNC(CAIL);

    RHDDebugVerb(((atomBiosHandlePtr)CAIL)->scrnIndex,1,"%s(%x,%x)\n",__func__,idx,data);
    if (((atomBiosHandlePtr)CAIL)->fbBase) {
	CARD8 *FBBase = (CARD8*)
	    RHDPTRI((atomBiosHandlePtr)CAIL)->FbBase;
    *((CARD32*)(FBBase + (((atomBiosHandlePtr)CAIL)->fbBase) + idx)) = data;
    } else if (((atomBiosHandlePtr)CAIL)->scratchBase) {
    *(CARD32*)((CARD8*)(((atomBiosHandlePtr)CAIL)->scratchBase) + idx) = data;
    } else
	xf86DrvMsg(((atomBiosHandlePtr)CAIL)->scrnIndex,X_ERROR,
		   "%s: no fbbase set\n",__func__);
}

ULONG
CailReadMC(VOID *CAIL, ULONG Address)
{
    ULONG ret;

    CAILFUNC(CAIL);

    ret = RHDReadMC(((atomBiosHandlePtr)CAIL)->rhdPtr, Address | MC_IND_ALL);
    RHDDebugVerb(((atomBiosHandlePtr)CAIL)->scrnIndex,1,"%s(%x) = %x\n",__func__,Address,ret);
    return ret;
}

VOID
CailWriteMC(VOID *CAIL, ULONG Address, ULONG data)
{
  CAILFUNC(CAIL);


    RHDDebugVerb(((atomBiosHandlePtr)CAIL)->scrnIndex,1,"%s(%x,%x)\n",__func__,Address,data);

    atomSaveRegisters((atomBiosHandlePtr)CAIL, atomRegisterMC, Address);

  RHDWriteMC(((atomBiosHandlePtr)CAIL)->rhdPtr, Address | MC_IND_ALL | MC_IND_WR_EN, data);
}

#ifdef XSERVER_LIBPCIACCESS

VOID
CailReadPCIConfigData(VOID*CAIL, VOID* ret, UINT32 idx,UINT16 size)
{
    pci_device_cfg_read(RHDPTRI((atomBiosHandlePtr)CAIL)->PciInfo,
				ret,idx << 2 , size >> 3, NULL);
}

VOID
CailWritePCIConfigData(VOID*CAIL,VOID*src,UINT32 idx,UINT16 size)
{
    atomSaveRegisters((atomBiosHandlePtr)CAIL, atomRegisterPCICFG, idx << 2);
    pci_device_cfg_write(RHDPTRI((atomBiosHandlePtr)CAIL)->PciInfo,
			 src, idx << 2, size >> 3, NULL);
}

#else

VOID
CailReadPCIConfigData(VOID*CAIL, VOID* ret, UINT32 idx,UINT16 size)
{ u32_t bus, devfn;
  PCITAG tag = ((atomBiosHandlePtr)CAIL)->PciTag;

  CAILFUNC(CAIL);
  bus   = PCI_BUS_FROM_TAG(tag);
  devfn = PCI_DFN_FROM_TAG(tag);

    switch (size) {
    case 8:
      *(CARD8*)ret = PciRead8(bus,devfn,idx << 2);
	    break;
    case 16:
      *(CARD16*)ret = PciRead16(bus,devfn,idx << 2);
	    break;
    case 32:
      *(CARD32*)ret = PciRead32(bus,devfn,idx << 2);
	    break;
    default:
	xf86DrvMsg(((atomBiosHandlePtr)CAIL)->scrnIndex,
		   X_ERROR,"%s: Unsupported size: %i\n",
		   __func__,(int)size);
      return;
      break;
  }
    RHDDebugVerb(((atomBiosHandlePtr)CAIL)->scrnIndex,1,"%s(%x) = %x\n",__func__,idx,*(unsigned int*)ret);

}

VOID
CailWritePCIConfigData(VOID*CAIL,VOID*src,UINT32 idx,UINT16 size)
{
  u32_t bus, devfn;
  PCITAG tag = ((atomBiosHandlePtr)CAIL)->PciTag;
  bus   = PCI_BUS_FROM_TAG(tag);
  devfn = PCI_DFN_FROM_TAG(tag);

  CAILFUNC(CAIL);

  RHDDebugVerb(((atomBiosHandlePtr)CAIL)->scrnIndex,1,"%s(%x,%x)\n",__func__,idx,(*(unsigned int*)src));

  atomSaveRegisters((atomBiosHandlePtr)CAIL, atomRegisterPCICFG, idx << 2);
    switch (size) {
    case 8:
      PciWrite8(bus,devfn, idx << 2,*(CARD8*)src);
      break;
    case 16:
      PciWrite16(bus,devfn,idx << 2,*(CARD16*)src);
      break;
    case 32:
      PciWrite32(bus,devfn,idx << 2,*(CARD32*)src);
      break;
    default:
	    xf86DrvMsg(((atomBiosHandlePtr)CAIL)->scrnIndex,X_ERROR,
		       "%s: Unsupported size: %i\n",__func__,(int)size);
      break;
  }
}
#endif

ULONG
CailReadPLL(VOID *CAIL, ULONG Address)
{
    ULONG ret;

    CAILFUNC(CAIL);

    ret = _RHDReadPLL(((atomBiosHandlePtr)CAIL)->rhdPtr, Address);
    RHDDebugVerb(((atomBiosHandlePtr)CAIL)->scrnIndex,1,"%s(%x) = %x\n",__func__,Address,ret);
    return ret;
}

VOID
CailWritePLL(VOID *CAIL, ULONG Address,ULONG Data)
{
    CAILFUNC(CAIL);

    RHDDebugVerb(((atomBiosHandlePtr)CAIL)->scrnIndex,1,"%s(%x,%x)\n",__func__,Address,Data);
    atomSaveRegisters((atomBiosHandlePtr)CAIL, atomRegisterPLL, Address);
    _RHDWritePLL(((atomBiosHandlePtr)CAIL)->scrnIndex, Address, Data);
}

# endif

#endif /* ATOM_BIOS */
