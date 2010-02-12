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


#ifndef RHD_ATOMBIOS_H_
# define RHD_ATOMBIOS_H_

# ifdef ATOM_BIOS

#define RHD_ATOMBIOS_ON 1
#define RHD_ATOMBIOS_OFF 2
#define RHD_ATOMBIOS_FORCE 4
#define RHD_ATOMBIOS_CRTC 0
#define RHD_ATOMBIOS_PLL 4
#define RHD_ATOMBIOS_OUTPUT 8

typedef enum _AtomBiosRequestID {
    ATOMBIOS_INIT,
    ATOMBIOS_TEARDOWN,
# ifdef ATOM_BIOS_PARSER
    ATOMBIOS_EXEC,
#  endif
    ATOMBIOS_ALLOCATE_FB_SCRATCH,
    ATOMBIOS_GET_CONNECTORS,
    ATOMBIOS_GET_OUTPUT_DEVICE_LIST,
    ATOMBIOS_GET_PANEL_MODE,
    ATOMBIOS_GET_PANEL_EDID,
    ATOMBIOS_GET_CODE_DATA_TABLE,
    GET_DEFAULT_ENGINE_CLOCK,
    GET_DEFAULT_MEMORY_CLOCK,
    GET_MAX_PIXEL_CLOCK_PLL_OUTPUT,
    GET_MIN_PIXEL_CLOCK_PLL_OUTPUT,
    GET_MAX_PIXEL_CLOCK_PLL_INPUT,
    GET_MIN_PIXEL_CLOCK_PLL_INPUT,
    GET_MAX_PIXEL_CLK,
    GET_REF_CLOCK,
    GET_FW_FB_START,
    GET_FW_FB_SIZE,
    ATOM_TMDS_MAX_FREQUENCY,
    ATOM_TMDS_PLL_CHARGE_PUMP,
    ATOM_TMDS_PLL_DUTY_CYCLE,
    ATOM_TMDS_PLL_VCO_GAIN,
    ATOM_TMDS_PLL_VOLTAGE_SWING,
    ATOM_LVDS_SUPPORTED_REFRESH_RATE,
    ATOM_LVDS_OFF_DELAY,
    ATOM_LVDS_SEQ_DIG_ONTO_DE,
    ATOM_LVDS_SEQ_DE_TO_BL,
    ATOM_LVDS_SPATIAL_DITHER,
    ATOM_LVDS_TEMPORAL_DITHER,
    ATOM_LVDS_DUALLINK,
    ATOM_LVDS_24BIT,
    ATOM_LVDS_GREYLVL,
    ATOM_LVDS_FPDI,
    ATOM_GPIO_QUERIES,
    ATOM_GPIO_I2C_CLK_MASK,
    ATOM_GPIO_I2C_CLK_MASK_SHIFT,
    ATOM_GPIO_I2C_DATA_MASK,
    ATOM_GPIO_I2C_DATA_MASK_SHIFT,
    ATOM_DAC1_BG_ADJ,
    ATOM_DAC1_DAC_ADJ,
    ATOM_DAC1_FORCE,
    ATOM_DAC2_CRTC2_BG_ADJ,
    ATOM_DAC2_NTSC_BG_ADJ,
    ATOM_DAC2_PAL_BG_ADJ,
    ATOM_DAC2_CV_BG_ADJ,
    ATOM_DAC2_CRTC2_DAC_ADJ,
    ATOM_DAC2_NTSC_DAC_ADJ,
    ATOM_DAC2_PAL_DAC_ADJ,
    ATOM_DAC2_CV_DAC_ADJ,
    ATOM_DAC2_CRTC2_FORCE,
    ATOM_DAC2_CRTC2_MUX_REG_IND,
    ATOM_DAC2_CRTC2_MUX_REG_INFO,
    ATOM_ANALOG_TV_MODE,
    ATOM_ANALOG_TV_DEFAULT_MODE,
    ATOM_ANALOG_TV_SUPPORTED_MODES,
    ATOM_GET_CONDITIONAL_GOLDEN_SETTINGS,
    ATOM_GET_PCIENB_CFG_REG7,
    ATOM_GET_CAPABILITY_FLAG,
    ATOM_GET_PCIE_LANES,
    ATOM_SET_REGISTER_LIST_LOCATION,
    ATOM_RESTORE_REGISTERS,
    FUNC_END
} AtomBiosRequestID;

typedef enum _AtomBiosResult {
    ATOM_SUCCESS,
    ATOM_FAILED,
    ATOM_NOT_IMPLEMENTED
} AtomBiosResult;

typedef struct AtomExec {
    int index;
    pointer pspace;
    pointer *dataSpace;
} AtomExecRec, *AtomExecPtr;

typedef struct AtomFb {
    unsigned int start;
    unsigned int size;
} AtomFbRec, *AtomFbPtr;

struct AtomDacCodeTableData
{
    CARD8 DAC1PALWhiteFine;
    CARD8 DAC1PALBandGap;
    CARD8 DAC1NTSCWhiteFine;
    CARD8 DAC1NTSCBandGap;
    CARD8 DAC1VGAWhiteFine;
    CARD8 DAC1VGABandGap;
    CARD8 DAC1CVWhiteFine;
    CARD8 DAC1CVBandGap;
    CARD8 DAC2PALWhiteFine;
    CARD8 DAC2PALBandGap;
    CARD8 DAC2NTSCWhiteFine;
    CARD8 DAC2NTSCBandGap;
    CARD8 DAC2VGAWhiteFine;
    CARD8 DAC2VGABandGap;
    CARD8 DAC2CVWhiteFine;
    CARD8 DAC2CVBandGap;
};

typedef enum AtomTVMode {
    ATOM_TVMODE_NTSC = 1 << 0,
    ATOM_TVMODE_NTSCJ = 1 << 1,
    ATOM_TVMODE_PAL = 1 << 2,
    ATOM_TVMODE_PALM = 1 << 3,
    ATOM_TVMODE_PALCN = 1 << 4,
    ATOM_TVMODE_PALN = 1 << 5,
    ATOM_TVMODE_PAL60 = 1 << 6,
    ATOM_TVMODE_SECAM = 1 << 7,
    ATOM_TVMODE_CV = 1 << 8
} AtomTVMode;

enum atomPCIELanes {
    atomPCIELaneNONE,
    atomPCIELane0_3,
    atomPCIELane0_7,
    atomPCIELane4_7,
    atomPCIELane8_11,
    atomPCIELane8_15,
    atomPCIELane12_15
};

enum atomDevice {
    atomNone, /* 0 */
    atomCRT1, /* 1 */
    atomLCD1, /* 2 */
    atomTV1,  /* 3 */
    atomDFP1, /* 4 */
    atomCRT2, /* 5 */
    atomLCD2, /* 6 */
    atomTV2,  /* 7 */
    atomDFP2, /* 8 */
    atomCV,   /* 9 */
    atomDFP3, /* a */
    atomDFP4, /* b */
    atomDFP5  /* c */
};

typedef struct AtomGoldenSettings
{
    unsigned char *BIOSPtr;
    unsigned char *End;
    unsigned int value;

} AtomGoldenSettings;

typedef union AtomBiosArg
{
    CARD32 val;
    struct rhdConnectorInfo	*ConnectorInfo;
    struct rhdAtomOutputDeviceList  *OutputDeviceList;
    enum RHD_CHIPSETS		chipset;
    struct AtomGoldenSettings	GoldenSettings;
    unsigned char*		EDIDBlock;
    void                       **Address;
    struct {
	unsigned char *loc;
	unsigned short size;
    } CommandDataTable;
    struct {
	enum atomPCIELanes	Chassis;
	enum atomPCIELanes	Docking;
    } pcieLanes;
    atomBiosHandlePtr		atomhandle;
    DisplayModePtr		mode;
    AtomExecRec			exec;
    AtomFbRec			fb;
    enum RHD_TV_MODE		tvMode;
} AtomBiosArgRec, *AtomBiosArgPtr;

enum atomCrtc {
    atomCrtc1,
    atomCrtc2
};

enum atomCrtcAction {
    atomCrtcEnable,
    atomCrtcDisable
};

enum atomOutputLinks {
    atomSingleLink,
    atomDualLink
};

enum atomTransmitter {
    atomTransmitterLVTMA,
    atomTransmitterUNIPHY,
    atomTransmitterUNIPHY1,
    atomTransmitterUNIPHY2,
    atomTransmitterPCIEPHY,
    atomTransmitterDIG1,
    atomTransmitterDIG2
};

enum atomTransmitterAction {
    atomTransDisable,
    atomTransEnable,
    atomTransEnableOutput,
    atomTransDisableOutput,
    atomTransInit,
    atomTransLcdBlOff,
    atomTransLcdBlOn,
    atomTransLcdBlBrightness,
    atomTransSetup
};

enum atomEncoder {
    atomEncoderNone,
    atomEncoderDACA,
    atomEncoderDACB,
    atomEncoderTV,
    atomEncoderTMDS1,  /* TMDSA */
    atomEncoderTMDS2,  /* LVTMA */
    atomEncoderLVDS,   /* LVTMA (Panel) */
    atomEncoderDVO,
    atomEncoderDIG1,
    atomEncoderDIG2,
    atomEncoderExternal
};

enum atomEncoderMode {
    atomNoEncoder,
    atomDVI,
    atomDP,
    atomLVDS,
    atomHDMI,
    atomSDVO,
    atomTVComposite,
    atomTVSVideo,
    atomTVComponent,
    atomCRT
};

enum atomEncoderAction {
    atomEncoderOff,
    atomEncoderOn
};

enum atomOutput {
    atomDVOOutput,
    atomLCDOutput,
    atomCVOutput,
    atomTVOutput,
    atomLVTMAOutput,
    atomTMDSAOutput,
    atomDAC1Output,
    atomDAC2Output
};

enum atomOutputType {
    atomOutputNone,
    atomOutputDacA,
    atomOutputDacB,
    atomOutputTmdsa,
    atomOutputLvtma,
    atomOutputDvo,
    atomOutputKldskpLvtma,
    atomOutputUniphyA,
    atomOutputUniphyB,
    atomOutputUniphyC,
    atomOutputUniphyD,
    atomOutputUniphyE,
    atomOutputUniphyF
};

enum atomOutputAction {
    atomOutputEnable,
    atomOutputDisable,
    atomOutputLcdOn,
    atomOutputLcdOff,
    atomOutputLcdBrightnessControl,
    atomOutputLcdSelftestStart,
    atomOutputLcdSelftestStop,
    atomOutputEncoderInit
};

enum atomDAC {
    atomDACA,
    atomDACB,
    atomDACExt
};

enum atomTransmitterLink {
    atomTransLinkA,
    atomTransLinkAB,
    atomTransLinkB,
    atomTransLinkBA
};

enum atomDACStandard {
    atomDAC_VGA,
    atomDAC_CV,
    atomDAC_NTSC,
    atomDAC_PAL
};

enum atomDVORate {
    atomDVO_RateSDR,
    atomDVO_RateDDR
};

enum atomDVOOutput {
    atomDVO_OutputLow12Bit,
    atomDVO_OutputHigh12Bit,
    atomDVO_Output24Bit
};

enum atomScaler {
    atomScaler1,
    atomScaler2
};

enum atomScaleMode {
    atomScaleDisable,
    atomScaleCenter,
    atomScaleExpand,
    atomScaleMulttabExpand
};

enum atomPxclk {
    atomPclk1,
    atomPclk2
};

struct atomCodeTableVersion
{
    CARD8 cref;
    CARD8 fref;
};

enum atomTemporalGreyLevels {
    atomTemporalDither0,
    atomTemporalDither4,
    atomTemporalDither2
};

struct atomTransmitterConfig
{
    int PixelClock;
    enum atomEncoder Encoder;
    enum atomPCIELanes Lanes;
    enum atomEncoderMode Mode;
    enum atomTransmitterLink Link;
    enum atomOutputLinks LinkCnt;
    Bool Coherent;
};

struct atomEncoderConfig
{
    int PixelClock;
    union {
	struct {
	    enum atomDACStandard DacStandard;
	} dac;
	struct {
	    enum RHD_TV_MODE TvStandard;
	} tv;
	struct {
	    enum atomOutputLinks LinkCnt;
	    Bool Is24bit;
	} lvds;
	struct {
	    enum atomOutputLinks LinkCnt;
	    Bool Is24bit;
	    Bool Coherent;
	    Bool LinkB;
	    Bool Hdmi;
	    Bool SpatialDither;
	    enum atomTemporalGreyLevels TemporalGrey;
	} lvds2;
	struct {
	    enum atomTransmitterLink Link;
	    enum atomOutputLinks LinkCnt;
	    enum atomTransmitter Transmitter;
	    enum atomEncoderMode EncoderMode;
	} dig;
	struct {
	    enum atomDevice DvoDeviceType;
	    int EncoderID;
	    Bool digital;
	    union
	    {
		enum RHD_TV_MODE TVMode;
		char dummy; /* @@@ placeholder for digital attributes */
	    } u;
	} dvo;
	struct{
	    enum atomDVORate Rate;
	    enum atomDVOOutput DvoOutput;
	} dvo3;
    } u;
};

struct atomCrtcSourceConfig
{
    union {
	enum atomDevice Device;
	struct {
	    enum atomEncoder Encoder;
	    enum atomEncoderMode Mode;
	} crtc2;
    } u;
};

struct atomPixelClockConfig {
    Bool Enable;
    int PixelClock;
    int RefDiv;
    int FbDiv;
    int PostDiv;
    int FracFbDiv;
    enum atomCrtc Crtc;
    union  {
	struct {
	    Bool Force;
	    enum atomDevice Device;
	} v2;
	struct {
	    Bool Force;
	    enum atomOutputType OutputType;
	    enum atomEncoderMode EncoderMode;
	    Bool UsePpll;
	} v3;
    } u;
};

struct atomCrtcOverscan {
    unsigned short ovscnLeft;
    unsigned short ovscnRight;
    unsigned short ovscnTop;
    unsigned short ovscnBottom;
};

enum atomBlankAction {
    atomBlankOn,
    atomBlankOff
};

struct atomCrtcBlank {
    enum atomBlankAction Action;
    unsigned short r, g, b;
};

extern AtomBiosResult RHDAtomBiosFunc(RHDPtr rhdPtr, atomBiosHandlePtr handle,
		AtomBiosRequestID id, AtomBiosArgPtr data);

#  ifdef ATOM_BIOS_PARSER
extern Bool rhdAtomSetTVEncoder(atomBiosHandlePtr handle, Bool enable, int mode);

#   if 0
extern Bool rhdAtomASICInit(atomBiosHandlePtr handle);
extern struct atomCodeTableVersion rhdAtomASICInitVersion(atomBiosHandlePtr handle);
#   endif
extern Bool rhdAtomSetScaler(atomBiosHandlePtr handle, enum atomScaler scaler,
			     enum atomScaleMode mode);
extern struct atomCodeTableVersion rhdAtomSetScalerVersion(atomBiosHandlePtr handle);
extern Bool rhdAtomDigTransmitterControl(atomBiosHandlePtr handle, enum atomTransmitter id,
					 enum atomTransmitterAction action,
					 struct atomTransmitterConfig *config);
extern struct atomCodeTableVersion rhdAtomDigTransmitterControlVersion(atomBiosHandlePtr handle);
extern Bool rhdAtomOutputControl(atomBiosHandlePtr handle, enum atomOutput id,
				 enum atomOutputAction action);
extern struct atomCodeTableVersion rhdAtomOutputControlVersion(atomBiosHandlePtr handle,
							       enum atomOutput id);
extern Bool AtomDACLoadDetection(atomBiosHandlePtr handle, enum atomDevice id, enum atomDAC dac);
extern struct atomCodeTableVersion AtomDACLoadDetectionVersion(atomBiosHandlePtr handle, enum atomDevice id);
extern Bool rhdAtomEncoderControl(atomBiosHandlePtr handle, enum atomEncoder id,
				  enum atomEncoderAction action, struct atomEncoderConfig *config);
struct atomCodeTableVersion rhdAtomEncoderControlVersion(atomBiosHandlePtr handle,
enum atomEncoder id);
extern Bool rhdAtomUpdateCRTC_DoubleBufferRegisters(atomBiosHandlePtr handle, enum atomCrtc id,
			      enum atomCrtcAction action);
extern struct atomCodeTableVersion rhdAtomUpdateCRTC_DoubleBufferRegistersVersion(atomBiosHandlePtr handle);
extern Bool rhdAtomEnableCrtc(atomBiosHandlePtr handle, enum atomCrtc id,
			      enum atomCrtcAction action);
extern struct atomCodeTableVersion rhdAtomEnableCrtcVersion(atomBiosHandlePtr handle);
extern Bool rhdAtomEnableCrtcMemReq(atomBiosHandlePtr handle, enum atomCrtc id,
				    enum atomCrtcAction action);
extern struct atomCodeTableVersion rhdAtomEnableCrtcMemReqVersion(atomBiosHandlePtr handle);
extern Bool rhdAtomSetCRTCTimings(atomBiosHandlePtr handle, enum atomCrtc id, DisplayModePtr mode,
				  int depth);
extern struct atomCodeTableVersion rhdAtomSetCRTCTimingsVersion(atomBiosHandlePtr handle);
extern Bool rhdAtomSetPixelClock(atomBiosHandlePtr handle, enum atomPxclk id,
				 struct atomPixelClockConfig *config);
extern struct atomCodeTableVersion rhdAtomSetPixelClockVersion(atomBiosHandlePtr handle);
extern Bool rhdAtomSelectCrtcSource(atomBiosHandlePtr handle, enum atomCrtc id,
				    struct atomCrtcSourceConfig *config);
extern struct atomCodeTableVersion rhdAtomSelectCrtcSourceVersion(atomBiosHandlePtr handle);
extern Bool rhdAtomSetCRTCOverscan(atomBiosHandlePtr handle, enum atomCrtc id,
				   struct atomCrtcOverscan *config);
struct atomCodeTableVersion rhdAtomSetCRTCOverscanVersion(atomBiosHandlePtr handle);
extern Bool rhdAtomBlankCRTC(atomBiosHandlePtr handle, enum atomCrtc id, struct atomCrtcBlank *config);
extern struct atomCodeTableVersion rhdAtomBlankCRTCVersion(atomBiosHandlePtr handle);

#  endif /* ATOM_BIOS_PASER */

# endif /* ATOM_BIOS */

#endif /*  RHD_ATOMBIOS_H_ */
