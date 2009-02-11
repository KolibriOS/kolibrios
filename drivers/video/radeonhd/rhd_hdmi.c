/*
 * Copyright 2008  Christian König <deathsimple@vodafone.de>
 * Copyright 2007  Luc Verhaegen <lverhaegen@novell.com>
 * Copyright 2007  Matthias Hopf <mhopf@novell.com>
 * Copyright 2007  Egbert Eich   <eich@novell.com>
 * Copyright 2007  Advanced Micro Devices, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xf86.h"

#include "rhd.h"
#include "rhd_audio.h"
#include "rhd_connector.h"
#include "rhd_output.h"
#include "rhd_hdmi.h"
#include "rhd_regs.h"

enum HdmiColorFormat {
    RGB = 0,
    YCC_422 = 1,
    YCC_444 = 2
};

struct {
    CARD32 Clock;

    int N_32kHz;
    int CTS_32kHz;

    int N_44_1kHz;
    int CTS_44_1kHz;

    int N_48kHz;
    int CTS_48kHz;

} AudioClockRegeneration[] = {
    /*             32kHz          44.1kHz        48kHz    */
    /* Clock      N     CTS      N     CTS      N     CTS */
    {  25174,  4576,  28125,  7007,  31250,  6864,  28125 }, /*  25,20/1.001 MHz */
    {  25200,  4096,  25200,  6272,  28000,  6144,  25200 }, /*  25.20       MHz */
    {  27000,  4096,  27000,  6272,  30000,  6144,  27000 }, /*  27.00       MHz */
    {  27027,  4096,  27027,  6272,  30030,  6144,  27027 }, /*  27.00*1.001 MHz */
    {  54000,  4096,  54000,  6272,  60000,  6144,  54000 }, /*  54.00       MHz */
    {  54054,  4096,  54054,  6272,  60060,  6144,  54054 }, /*  54.00*1.001 MHz */
    {  74175, 11648, 210937, 17836, 234375, 11648, 140625 }, /*  74.25/1.001 MHz */
    {  74250,  4096,  74250,  6272,  82500,  6144,  74250 }, /*  74.25       MHz */
    { 148351, 11648, 421875,  8918, 234375,  5824, 140625 }, /* 148.50/1.001 MHz */
    { 148500,  4096, 148500,  6272, 165000,  6144, 148500 }, /* 148.50       MHz */
    {      0,  4096,      0,  6272,      0,  6144,      0 }  /* Other */
};

/*
 * calculate CTS value if it's not found in the table
 */
static void
HdmiCalcCTS(struct rhdHdmi *hdmi, CARD32 Clock, int* CTS, int N, int freq)
{
    if(*CTS == 0) *CTS = Clock*1000*N/(128*freq);
    xf86DrvMsg(hdmi->scrnIndex, X_INFO, "Using ACR timing N=%d CTS=%d for frequency %d\n",N,*CTS,freq);
}

/*
 * update the N and CTS parameters for a given clock rate
 */
static void
HdmiAudioClockRegeneration(struct rhdHdmi *hdmi, CARD32 Clock)
{
    int CTS;
    int N;
    int i;
    for(i=0; AudioClockRegeneration[i].Clock != Clock && AudioClockRegeneration[i].Clock != 0; i++);

    CTS = AudioClockRegeneration[i].CTS_32kHz;
    N = AudioClockRegeneration[i].N_32kHz;
    HdmiCalcCTS(hdmi, Clock, &CTS, N, 32000);
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_32kHz_CTS, CTS << 12);
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_32kHz_N, N);

    CTS = AudioClockRegeneration[i].CTS_44_1kHz;
    N = AudioClockRegeneration[i].N_44_1kHz;
    HdmiCalcCTS(hdmi, Clock, &CTS, N, 44100);
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_44_1kHz_CTS, CTS << 12);
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_44_1kHz_N, N);

    CTS = AudioClockRegeneration[i].CTS_48kHz;
    N = AudioClockRegeneration[i].N_48kHz;
    HdmiCalcCTS(hdmi, Clock, &CTS, N, 48000);
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_48kHz_CTS, CTS << 12);
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_48kHz_N, N);
}

/*
 * calculate the crc for a given info frame
 */
static void
HdmiInfoFrameChecksum(CARD8 packetType, CARD8 versionNumber, CARD8 length, CARD8* frame)
{
    int i;
    frame[0] = packetType + versionNumber + length;
    for(i=1;i<=length;i++)
	frame[0] += frame[i];
    frame[0] = 0x100 - frame[0];
}

/*
 * build a HDMI Video Info Frame
 */
static void
HdmiVideoInfoFrame(
    struct rhdHdmi *hdmi,
    enum HdmiColorFormat ColorFormat,
    Bool ActiveInformationPresent,
    CARD8 ActiveFormatAspectRatio,
    CARD8 ScanInformation,
    CARD8 Colorimetry,
    CARD8 ExColorimetry,
    CARD8 Quantization,
    Bool ITC,
    CARD8 PictureAspectRatio,
    CARD8 VideoFormatIdentification,
    CARD8 PixelRepetition,
    CARD8 NonUniformPictureScaling,
    CARD8 BarInfoDataValid,
    CARD16 TopBar,
    CARD16 BottomBar,
    CARD16 LeftBar,
    CARD16 RightBar
)
{
    CARD8 frame[14];

    frame[0x0] = 0;
    frame[0x1] =
	(ScanInformation & 0x3) |
	((BarInfoDataValid & 0x3) << 2) |
	((ActiveInformationPresent & 0x1) << 4) |
	((ColorFormat & 0x3) << 5);
    frame[0x2] =
	(ActiveFormatAspectRatio & 0xF) |
	((PictureAspectRatio & 0x3) << 4) |
	((Colorimetry & 0x3) << 6);
    frame[0x3] =
	(NonUniformPictureScaling & 0x3) |
	((Quantization & 0x3) << 2) |
	((ExColorimetry & 0x7) << 4) |
	((ITC & 0x1) << 7);
    frame[0x4] = (VideoFormatIdentification & 0x7F);
    frame[0x5] = (PixelRepetition & 0xF);
    frame[0x6] = (TopBar & 0xFF);
    frame[0x7] = (TopBar >> 8);
    frame[0x8] = (BottomBar & 0xFF);
    frame[0x9] = (BottomBar >> 8);
    frame[0xA] = (LeftBar & 0xFF);
    frame[0xB] = (LeftBar >> 8);
    frame[0xC] = (RightBar & 0xFF);
    frame[0xD] = (RightBar >> 8);

    HdmiInfoFrameChecksum(0x82, 0x02, 0x0D, frame);


    RHDRegWrite(hdmi, hdmi->Offset+HDMI_VIDEOINFOFRAME_0,
	frame[0x0] | (frame[0x1] << 8) | (frame[0x2] << 16) | (frame[0x3] << 24));
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_VIDEOINFOFRAME_1,
	frame[0x4] | (frame[0x5] << 8) | (frame[0x6] << 16) | (frame[0x7] << 24));
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_VIDEOINFOFRAME_2,
	frame[0x8] | (frame[0x9] << 8) | (frame[0xA] << 16) | (frame[0xB] << 24));
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_VIDEOINFOFRAME_3,
	frame[0xC] | (frame[0xD] << 8));
}

/*
 * build a Audio Info Frame
 */
static void
HdmiAudioInfoFrame(
    struct rhdHdmi *hdmi,
    CARD8 ChannelCount,
    CARD8 CodingType,
    CARD8 SampleSize,
    CARD8 SampleFrequency,
    CARD8 Format,
    CARD8 ChannelAllocation,
    CARD8 LevelShift,
    Bool DownmixInhibit
)
{
    CARD8 frame[11];

    frame[0x0] = 0;
    frame[0x1] = (ChannelCount & 0x7) | ((CodingType & 0xF) << 4);
    frame[0x2] = (SampleSize & 0x3) | ((SampleFrequency & 0x7) << 2);
    frame[0x3] = Format;
    frame[0x4] = ChannelAllocation;
    frame[0x5] = ((LevelShift & 0xF) << 3) | ((DownmixInhibit & 0x1) << 7);
    frame[0x6] = 0;
    frame[0x7] = 0;
    frame[0x8] = 0;
    frame[0x9] = 0;
    frame[0xA] = 0;

    HdmiInfoFrameChecksum(0x84, 0x01, 0x0A, frame);

    RHDRegWrite(hdmi, hdmi->Offset+HDMI_AUDIOINFOFRAME_0,
	frame[0x0] | (frame[0x1] << 8) | (frame[0x2] << 16) | (frame[0x3] << 24));
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_AUDIOINFOFRAME_1,
	frame[0x4] | (frame[0x5] << 8) | (frame[0x6] << 16) | (frame[0x8] << 24));
}

/*
 * it's unknown what these bits do excatly, but it's indeed quite usefull for debugging
 */
static void
HdmiAudioDebugWorkaround(struct rhdHdmi* hdmi, Bool Enable)
{
    if(Enable) {
	RHDRegMask(hdmi, hdmi->Offset+HDMI_CNTL, 0x1000, 0x1000);
	RHDRegWrite(hdmi, hdmi->Offset+HDMI_AUDIO_DEBUG, 0xffffff);
    } else {
	RHDRegMask(hdmi, hdmi->Offset+HDMI_CNTL, 0, 0x1000);
    }
}

/*
 * allocate/initialize the HDMI structure
 * and register with audio engine
 * output selects which engine is used
 */
struct rhdHdmi*
RHDHdmiInit(RHDPtr rhdPtr, struct rhdOutput* Output)
{
    struct rhdHdmi *hdmi;
    RHDFUNC(rhdPtr);

    if(rhdPtr->ChipSet >= RHD_R600) {
	hdmi = (struct rhdHdmi *) xnfcalloc(sizeof(struct rhdHdmi), 1);
	hdmi->scrnIndex = rhdPtr->scrnIndex;
	hdmi->Output = Output;
	switch(Output->Id) {
	    case RHD_OUTPUT_TMDSA:
		hdmi->Offset = HDMI_TMDS;
		break;

	    case RHD_OUTPUT_LVTMA:
		hdmi->Offset = HDMI_LVTMA;
		break;

	    case RHD_OUTPUT_UNIPHYA:
		hdmi->Offset = HDMI_TMDS;
		break;

	    case RHD_OUTPUT_KLDSKP_LVTMA:
		hdmi->Offset = HDMI_DIG;
		break;

	    /*case RHD_OUTPUT_UNIPHYB: */

	    default:
		xf86DrvMsg(hdmi->scrnIndex, X_ERROR, "%s: unknown HDMI output type\n", __func__);
		xfree(hdmi);
		return NULL;
		break;
	}
	hdmi->Stored = FALSE;
//    RHDAudioRegisterHdmi(rhdPtr, hdmi);
	return hdmi;
    } else
	return NULL;
}

/*
 * update the info frames with the data from the current display mode
 */
void
RHDHdmiSetMode(struct rhdHdmi *hdmi, DisplayModePtr Mode)
{
    if(!hdmi) return;
    RHDFUNC(hdmi);

//    RHDAudioSetClock(RHDPTRI(hdmi), hdmi->Output, Mode->Clock);

    HdmiAudioDebugWorkaround(hdmi, FALSE);

    RHDRegWrite(hdmi, hdmi->Offset+HDMI_UNKNOWN_0, 0x1000);
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_UNKNOWN_1, 0x0);
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_UNKNOWN_2, 0x1000);

    HdmiAudioClockRegeneration(hdmi, Mode->Clock);

    RHDRegWrite(hdmi, hdmi->Offset+HDMI_VIDEOCNTL, 0x13);
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_VERSION, 0x202);

    HdmiVideoInfoFrame(hdmi, RGB, FALSE, 0, 0, 0,
	0, 0, FALSE, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    /* audio packets per line, does anyone know how to calc this ? */
    RHDRegMask(hdmi, hdmi->Offset+HDMI_CNTL, 0x020000, 0x1F0000);

    /* update? reset? don't realy know */
    RHDRegMask(hdmi, hdmi->Offset+HDMI_CNTL, 0x14000000, 0x14000000);
}

/*
 * update settings whith current parameters from audio engine
 */
void
RHDHdmiUpdateAudioSettings(
    struct rhdHdmi* hdmi,
    Bool playing,
    int channels,
    int rate,
    int bps,
    CARD8 status_bits,
    CARD8 category_code
)
{
    CARD32 iec;

    if(!hdmi) return;
    RHDFUNC(hdmi);

    xf86DrvMsg(hdmi->scrnIndex, X_INFO, "%s: %s with "
	"%d channels, %d Hz sampling rate, %d bits per sample,\n",
	 __func__, playing ? "playing" : "stoped", channels, rate, bps);
    xf86DrvMsg(hdmi->scrnIndex, X_INFO, "%s: "
	"0x%02x IEC60958 status bits and 0x%02x category code\n",
	 __func__, (int)status_bits, (int)category_code);

    /* start delivering audio frames */
    RHDRegMask(hdmi, hdmi->Offset+HDMI_CNTL, playing ? 1 : 0, 0x1);

    iec = 0;
    if(status_bits & AUDIO_STATUS_PROFESSIONAL)	iec |= 1 << 0;
    if(status_bits & AUDIO_STATUS_NONAUDIO)	iec |= 1 << 1;
    if(status_bits & AUDIO_STATUS_COPYRIGHT)	iec |= 1 << 2;
    if(status_bits & AUDIO_STATUS_EMPHASIS)	iec |= 1 << 3;

    iec |= category_code << 8;

    switch(rate)
    {
        case  32000: iec |= 0x3 << 24; break;
        case  44100: iec |= 0x0 << 24; break;
        case  88200: iec |= 0x8 << 24; break;
        case 176400: iec |= 0xc << 24; break;
        case  48000: iec |= 0x2 << 24; break;
        case  96000: iec |= 0xa << 24; break;
        case 192000: iec |= 0xe << 24; break;
    }

    RHDRegWrite(hdmi, hdmi->Offset+HDMI_IEC60958_1, iec);

    iec = 0;
    switch(bps)
    {
	case 16: iec |= 0x2; break;
	case 20: iec |= 0x3; break;
	case 24: iec |= 0xb; break;
    }
    if(status_bits & AUDIO_STATUS_V) iec |= 0x5 << 16;

    RHDRegMask(hdmi, hdmi->Offset+HDMI_IEC60958_2, iec, 0x5000f);

    RHDRegWrite(hdmi, hdmi->Offset+HDMI_AUDIOCNTL, 0x31);
    HdmiAudioInfoFrame(hdmi, channels-1, 0, 0, 0, 0, 0, 0, FALSE);

    RHDRegMask(hdmi, hdmi->Offset+HDMI_CNTL, 0x400000, 0x400000);
}

/*
 * enable/disable the HDMI engine
 */
void
RHDHdmiEnable(struct rhdHdmi *hdmi, Bool Enable)
{
    if(!hdmi) return;
    RHDFUNC(hdmi);

    /* some version of atombios ignore the enable HDMI flag
     * so enabling/disabling HDMI was moved here for TMDSA and LVTMA */
    switch(hdmi->Output->Id) {
	case RHD_OUTPUT_TMDSA:
	     RHDRegMask(hdmi, TMDSA_CNTL, Enable ? 0x4 : 0x0, 0x4);
	     RHDRegWrite(hdmi, hdmi->Offset+HDMI_ENABLE, Enable ? 0x101 : 0x0);
	     break;

	case RHD_OUTPUT_LVTMA:
	     RHDRegMask(hdmi, LVTMA_CNTL, Enable ? 0x4 : 0x0, 0x4);
	     RHDRegWrite(hdmi, hdmi->Offset+HDMI_ENABLE, Enable ? 0x105 : 0x0);
	     break;

	case RHD_OUTPUT_UNIPHYA:
	case RHD_OUTPUT_UNIPHYB:
	case RHD_OUTPUT_KLDSKP_LVTMA:
	    RHDRegWrite(hdmi, hdmi->Offset+HDMI_ENABLE, Enable ? 0x110 : 0x0);
	    break;

	default:
	    xf86DrvMsg(hdmi->scrnIndex, X_ERROR, "%s: unknown HDMI output type\n", __func__);
	    break;
    }
}

/*
 * save the current config of HDMI engine
 */
void
RHDHdmiSave(struct rhdHdmi *hdmi)
{
    if(!hdmi) return;
    RHDFUNC(hdmi);

    hdmi->StoreEnable = RHDRegRead(hdmi, hdmi->Offset+HDMI_ENABLE);
    hdmi->StoreControl = RHDRegRead(hdmi, hdmi->Offset+HDMI_CNTL);
    hdmi->StoredAudioDebugWorkaround = RHDRegRead(hdmi, hdmi->Offset+HDMI_AUDIO_DEBUG);

    hdmi->StoredFrameVersion = RHDRegRead(hdmi, hdmi->Offset+HDMI_VERSION);

    hdmi->StoredVideoControl = RHDRegRead(hdmi, hdmi->Offset+HDMI_VIDEOCNTL);
    hdmi->StoreVideoInfoFrame[0x0] = RHDRegRead(hdmi, hdmi->Offset+HDMI_VIDEOINFOFRAME_0);
    hdmi->StoreVideoInfoFrame[0x1] = RHDRegRead(hdmi, hdmi->Offset+HDMI_VIDEOINFOFRAME_1);
    hdmi->StoreVideoInfoFrame[0x2] = RHDRegRead(hdmi, hdmi->Offset+HDMI_VIDEOINFOFRAME_2);
    hdmi->StoreVideoInfoFrame[0x3] = RHDRegRead(hdmi, hdmi->Offset+HDMI_VIDEOINFOFRAME_3);

    hdmi->StoredAudioControl = RHDRegRead(hdmi, hdmi->Offset+HDMI_AUDIOCNTL);
    hdmi->StoreAudioInfoFrame[0x0] = RHDRegRead(hdmi, hdmi->Offset+HDMI_AUDIOINFOFRAME_0);
    hdmi->StoreAudioInfoFrame[0x1] = RHDRegRead(hdmi, hdmi->Offset+HDMI_AUDIOINFOFRAME_1);

    hdmi->Store_32kHz_N = RHDRegRead(hdmi, hdmi->Offset+HDMI_32kHz_N);
    hdmi->Store_32kHz_CTS = RHDRegRead(hdmi, hdmi->Offset+HDMI_32kHz_CTS);

    hdmi->Store_44_1kHz_N = RHDRegRead(hdmi, hdmi->Offset+HDMI_44_1kHz_N);
    hdmi->Store_44_1kHz_CTS = RHDRegRead(hdmi, hdmi->Offset+HDMI_44_1kHz_CTS);

    hdmi->Store_48kHz_N = RHDRegRead(hdmi, hdmi->Offset+HDMI_48kHz_N);
    hdmi->Store_48kHz_CTS = RHDRegRead(hdmi, hdmi->Offset+HDMI_48kHz_CTS);

    hdmi->StoreIEC60958[0] = RHDRegRead(hdmi, hdmi->Offset+HDMI_IEC60958_1);
    hdmi->StoreIEC60958[1] = RHDRegRead(hdmi, hdmi->Offset+HDMI_IEC60958_2);

    hdmi->StoreUnknown[0x0] = RHDRegRead(hdmi, hdmi->Offset+HDMI_UNKNOWN_0);
    hdmi->StoreUnknown[0x1] = RHDRegRead(hdmi, hdmi->Offset+HDMI_UNKNOWN_1);
    hdmi->StoreUnknown[0x2] = RHDRegRead(hdmi, hdmi->Offset+HDMI_UNKNOWN_2);

    hdmi->Stored = TRUE;
}

/*
 * restore the saved config of HDMI engine
 */
void
RHDHdmiRestore(struct rhdHdmi *hdmi)
{
    if(!hdmi) return;
    RHDFUNC(hdmi);

    if (!hdmi->Stored) {
        xf86DrvMsg(hdmi->scrnIndex, X_ERROR, "%s: trying to restore "
                   "uninitialized values.\n", __func__);
        return;
    }

    RHDRegWrite(hdmi, hdmi->Offset+HDMI_ENABLE, hdmi->StoreEnable);
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_CNTL, hdmi->StoreControl);
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_AUDIO_DEBUG, hdmi->StoredAudioDebugWorkaround);

    RHDRegWrite(hdmi, hdmi->Offset+HDMI_VERSION, hdmi->StoredFrameVersion);

    RHDRegWrite(hdmi, hdmi->Offset+HDMI_VIDEOCNTL, hdmi->StoredVideoControl);
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_VIDEOINFOFRAME_0, hdmi->StoreVideoInfoFrame[0x0]);
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_VIDEOINFOFRAME_1, hdmi->StoreVideoInfoFrame[0x1]);
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_VIDEOINFOFRAME_2, hdmi->StoreVideoInfoFrame[0x2]);
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_VIDEOINFOFRAME_3, hdmi->StoreVideoInfoFrame[0x3]);

    RHDRegWrite(hdmi, hdmi->Offset+HDMI_AUDIOCNTL, hdmi->StoredAudioControl);
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_AUDIOINFOFRAME_0, hdmi->StoreAudioInfoFrame[0x0]);
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_AUDIOINFOFRAME_1, hdmi->StoreAudioInfoFrame[0x1]);

    RHDRegWrite(hdmi, hdmi->Offset+HDMI_32kHz_N, hdmi->Store_32kHz_N);
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_32kHz_CTS, hdmi->Store_32kHz_CTS);

    RHDRegWrite(hdmi, hdmi->Offset+HDMI_44_1kHz_N, hdmi->Store_44_1kHz_N);
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_44_1kHz_CTS, hdmi->Store_44_1kHz_CTS);

    RHDRegWrite(hdmi, hdmi->Offset+HDMI_48kHz_N, hdmi->Store_48kHz_N);
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_48kHz_CTS, hdmi->Store_48kHz_CTS);

    RHDRegWrite(hdmi, hdmi->Offset+HDMI_IEC60958_1, hdmi->StoreIEC60958[0]);
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_IEC60958_2, hdmi->StoreIEC60958[1]);

    RHDRegWrite(hdmi, hdmi->Offset+HDMI_UNKNOWN_0, hdmi->StoreUnknown[0x0]);
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_UNKNOWN_1, hdmi->StoreUnknown[0x1]);
    RHDRegWrite(hdmi, hdmi->Offset+HDMI_UNKNOWN_2, hdmi->StoreUnknown[0x2]);
}

/*
 * unregister with audio engine and release memory
 */
void
RHDHdmiDestroy(struct rhdHdmi *hdmi)
{
    if(!hdmi) return;
    RHDFUNC(hdmi);

//    RHDAudioUnregisterHdmi(RHDPTRI(hdmi), hdmi);

    xfree(hdmi);
}
