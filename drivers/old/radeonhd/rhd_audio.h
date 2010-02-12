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

#ifndef _RHD_AUDIO_H
#define _RHD_AUDIO_H

struct rhdAudio {

	int scrnIndex;

	struct rhdHdmi* Registered;
    int             Timer;

	Bool	SavedPlaying;
	int	SavedChannels;
	int	SavedRate;
	int	SavedBitsPerSample;
	CARD8	SavedStatusBits;
	CARD8	SavedCategoryCode;

	Bool Stored;

	CARD32 StoreEnabled;
	CARD32 StoreTiming;
	CARD32 StoreSupportedSizeRate;
	CARD32 StoreSupportedCodec;

	CARD32 StorePll1Mul;
	CARD32 StorePll1Div;
	CARD32 StorePll2Mul;
	CARD32 StorePll2Div;
	CARD32 StoreClockSrcSel;
};

/*
 * used for config value of RHDAudioSetSupported
 */
enum {
	AUDIO_RATE_8000_HZ   = 0x00000001,
	AUDIO_RATE_11025_HZ  = 0x00000002,
	AUDIO_RATE_16000_HZ  = 0x00000004,
	AUDIO_RATE_22050_HZ  = 0x00000008,
	AUDIO_RATE_32000_HZ  = 0x00000010,
	AUDIO_RATE_44100_HZ  = 0x00000020,
	AUDIO_RATE_48000_HZ  = 0x00000040,
	AUDIO_RATE_88200_HZ  = 0x00000080,
	AUDIO_RATE_96000_HZ  = 0x00000100,
	AUDIO_RATE_176400_HZ = 0x00000200,
	AUDIO_RATE_192000_HZ = 0x00000400,
	AUDIO_RATE_384000_HZ = 0x00000800,

	AUDIO_BPS_8  = 0x00010000,
	AUDIO_BPS_16 = 0x00020000,
	AUDIO_BPS_20 = 0x00040000,
	AUDIO_BPS_24 = 0x00080000,
	AUDIO_BPS_32 = 0x00100000
};

/*
 * used for codec value of RHDAudioSetSupported
 */
enum {
	AUDIO_CODEC_PCM      = 0x00000001,
	AUDIO_CODEC_FLOAT32  = 0x00000002,
	AUDIO_CODEC_AC3      = 0x00000004
};

/*
 * used for status bist value in RHDAudioUpdateHdmi
 */
enum {
	AUDIO_STATUS_DIG_ENABLE   = 0x01,
	AUDIO_STATUS_V            = 0x02,
	AUDIO_STATUS_VCFG         = 0x04,
	AUDIO_STATUS_EMPHASIS     = 0x08,
	AUDIO_STATUS_COPYRIGHT    = 0x10,
	AUDIO_STATUS_NONAUDIO     = 0x20,
	AUDIO_STATUS_PROFESSIONAL = 0x40,
	AUDIO_STATUS_LEVEL        = 0x80
};

void RHDAudioInit(RHDPtr rhdPtr);

void RHDAudioSetSupported(RHDPtr rhdPtr, Bool clear, CARD32 config, CARD32 codec);
void RHDAudioSetEnable(RHDPtr rhdPtr, Bool Enable);
void RHDAudioSetClock(RHDPtr rhdPtr, struct rhdOutput* Output, CARD32 Clock);

void RHDAudioRegisterHdmi(RHDPtr rhdPtr, struct rhdHdmi* rhdHdmi);
void RHDAudioUnregisterHdmi(RHDPtr rhdPtr, struct rhdHdmi* rhdHdmi);

void RHDAudioSave(RHDPtr rhdPtr);
void RHDAudioRestore(RHDPtr rhdPtr);

void RHDAudioDestroy(RHDPtr rhdPtr);

#endif /* _RHD_AUDIO_H */
