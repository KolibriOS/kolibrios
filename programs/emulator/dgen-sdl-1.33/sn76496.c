/***************************************************************************

  sn76496.c

  Routines to emulate the Texas Instruments SN76489 / SN76496 programmable
  tone /noise generator. Also known as (or at least compatible with) TMS9919.

  Noise emulation is not accurate due to lack of documentation. The noise
  generator uses a shift register with a XOR-feedback network, but the exact
  layout is unknown. It can be set for either period or white noise; again,
  the details are unknown.

***************************************************************************/

#include <stdint.h>
#include <string.h>
#include "system.h"
#include "sn76496.h"


#define MAX_OUTPUT 0x7fff
#define AUDIO_CONV(A) (A)

#define STEP 0x10000


/* Formulas for noise generator */
/* bit0 = output */

/* noise feedback for white noise mode */
#define FB_WNOISE 0x12000   /* bit15.d(16bits) = bit0(out) ^ bit2 */
//#define FB_WNOISE 0x14000 /* bit15.d(16bits) = bit0(out) ^ bit1 */
//#define FB_WNOISE 0x28000 /* bit16.d(17bits) = bit0(out) ^ bit2 (same to AY-3-8910) */
//#define FB_WNOISE 0x50000 /* bit17.d(18bits) = bit0(out) ^ bit2 */

/* noise feedback for periodic noise mode */
/* it is correct maybe (it was in the Megadrive sound manual) */
//#define FB_PNOISE 0x10000 /* 16bit rorate */
#define FB_PNOISE 0x08000   /* JH 981127 - fixes Do Run Run */

/* noise generator start preset (for periodic noise) */
#define NG_PRESET 0x0f35


struct SN76496
{
    int Channel;
    int SampleRate;
    unsigned int UpdateStep;
    int VolTable[16];   /* volume table         */
    int Register[8];    /* registers */
    int LastRegister;   /* last register written */
    int Volume[4];      /* volume of voice 0-2 and noise */
    unsigned int RNG;       /* noise generator      */
    int NoiseFB;        /* noise feedback mask */
    unsigned int Period[4];
    int Count[4];
    int Output[4];
};


static struct SN76496 sn[MAX_76496];

void SN76496_dump(int chip, uint8_t buf[16])
{
	struct SN76496 *R = &sn[chip];
	uint16_t tmp;
	unsigned int i;

	for (i = 0; (i < 8); ++i) {
		tmp = h2le16(R->Register[i]);
		memcpy(&buf[(i * 2)], &tmp, 2);
	}
}

void SN76496_restore(int chip, uint8_t buf[16])
{
	struct SN76496 *R = &sn[chip];
	uint16_t tmp;
	unsigned int i;

	for (i = 0; (i < 8); ++i) {
		memcpy(&tmp, &buf[(i * 2)], 2);
		R->Register[i] = le2h16(tmp);
	}
}

void SN76496Write(int chip,int data)
{
    struct SN76496 *R = &sn[chip];


    /* update the output buffer before changing the registers */
    ///// commented out by starshine
    //stream_update(R->Channel,0);

    if (data & 0x80)
    {
        int r = (data & 0x70) >> 4;
        int c = r/2;

        R->LastRegister = r;
        R->Register[r] = (R->Register[r] & 0x3f0) | (data & 0x0f);
        switch (r)
        {
            case 0: /* tone 0 : frequency */
            case 2: /* tone 1 : frequency */
            case 4: /* tone 2 : frequency */
                R->Period[c] = R->UpdateStep * R->Register[r];
                if (R->Period[c] == 0) R->Period[c] = R->UpdateStep;
                if (r == 4)
                {
                    /* update noise shift frequency */
                    if ((R->Register[6] & 0x03) == 0x03)
                        R->Period[3] = 2 * R->Period[2];
                }
                break;
            case 1: /* tone 0 : volume */
            case 3: /* tone 1 : volume */
            case 5: /* tone 2 : volume */
            case 7: /* noise  : volume */
                R->Volume[c] = R->VolTable[data & 0x0f];
                break;
            case 6: /* noise  : frequency, mode */
                {
                    int n = R->Register[6];
                    R->NoiseFB = (n & 4) ? FB_WNOISE : FB_PNOISE;
                    n &= 3;
                    /* N/512,N/1024,N/2048,Tone #3 output */
                    R->Period[3] = (n == 3) ? 2 * R->Period[2] : (R->UpdateStep << (5+n));

                    /* reset noise shifter */
                    R->RNG = NG_PRESET;
                    R->Output[3] = R->RNG & 1;
                }
                break;
        }
    }
    else
    {
        int r = R->LastRegister;
        int c = r/2;

        switch (r)
        {
            case 0: /* tone 0 : frequency */
            case 2: /* tone 1 : frequency */
            case 4: /* tone 2 : frequency */
                R->Register[r] = (R->Register[r] & 0x0f) | ((data & 0x3f) << 4);
                R->Period[c] = R->UpdateStep * R->Register[r];
                if (R->Period[c] == 0) R->Period[c] = R->UpdateStep;
                if (r == 4)
                {
                    /* update noise shift frequency */
                    if ((R->Register[6] & 0x03) == 0x03)
                        R->Period[3] = 2 * R->Period[2];
                }
                break;
        }
    }
}


void SN76496_0_w(int offset, int data) { (void)offset; SN76496Write(0, data); }
void SN76496_1_w(int offset, int data) { (void)offset; SN76496Write(1, data); }
void SN76496_2_w(int offset, int data) { (void)offset; SN76496Write(2, data); }
void SN76496_3_w(int offset, int data) { (void)offset; SN76496Write(3, data); }



void SN76496Update_8_2(int chip,void *buffer,int length)
{
#define DATATYPE unsigned char
#define DATACONV(A) AUDIO_CONV((A) / (STEP * 256))
#include "sn76496u.c"
#undef DATATYPE
#undef DATACONV
}

void SN76496Update_16_2(int chip,void *buffer,int length)
{
#define DATATYPE unsigned short
#define DATACONV(A) ((A) / STEP)
#include "sn76496u.c"
#undef DATATYPE
#undef DATACONV
}



void SN76496_set_clock(int chip,int clock)
{
    struct SN76496 *R = &sn[chip];


    /* the base clock for the tone generators is the chip clock divided by 16; */
    /* for the noise generator, it is clock / 256. */
    /* Here we calculate the number of steps which happen during one sample */
    /* at the given sample rate. No. of events = sample rate / (clock/16). */
    /* STEP is a multiplier used to turn the fraction into a fixed point */
    /* number. */
    R->UpdateStep = ((double)STEP * R->SampleRate * 16) / clock;
}



static void SN76496_set_volume(int chip,int volume,int gain)
{
    struct SN76496 *R = &sn[chip];
    int i;
    double out;

    (void)volume;
    ///// commented out by starshine
    //stream_set_volume(R->Channel,volume);

    gain &= 0xff;

    /* increase max output basing on gain (0.2 dB per step) */
    out = MAX_OUTPUT / 3;
    while (gain-- > 0)
        out *= 1.023292992; /* = (10 ^ (0.2/20)) */

    /* build volume table (2dB per step) */
    for (i = 0;i < 15;i++)
    {
        /* limit volume to avoid clipping */
        if (out > MAX_OUTPUT / 3) R->VolTable[i] = MAX_OUTPUT / 3;
        else R->VolTable[i] = out;

        out /= 1.258925412; /* = 10 ^ (2/20) = 2dB */
    }
    R->VolTable[15] = 0;
}



int SN76496_init(int chip,int clock,int sample_rate,int sample_bits)
{
    int i;
    struct SN76496 *R = &sn[chip];
    /* char name[40]; */

    (void)sample_bits;
    ////// commented out by starshine
    //sprintf(name,"SN76496 #%d",chip);
    //R->Channel = stream_init(msound,
    //        name,sample_rate,sample_bits,
    //        chip,(sample_bits == 16) ? SN76496Update_16 : SN76496Update_8);

    if (R->Channel == -1)
        return 1;

    R->SampleRate = sample_rate;
    SN76496_set_clock(chip,clock);
    SN76496_set_volume(chip,255,0);

    for (i = 0;i < 4;i++) R->Volume[i] = 0;

    R->LastRegister = 0;
    for (i = 0;i < 8;i+=2)
    {
        R->Register[i] = 0;
        R->Register[i + 1] = 0x0f;  /* volume = 0 */
    }

    for (i = 0;i < 4;i++)
    {
        R->Output[i] = 0;
        R->Period[i] = R->Count[i] = R->UpdateStep;
    }
    R->RNG = NG_PRESET;
    R->Output[3] = R->RNG & 1;

    return 0;
}



int SN76496_sh_start()
{
    ///// total commenting out by starshine
    //int chip;
    //const struct SN76496interface *intf = msound->sound_interface;


    //for (chip = 0;chip < intf->num;chip++)
    //{
    //    if (SN76496_init(msound,chip,intf->baseclock,Machine->sample_rate,Machine->sample_bits) != 0)
    //        return 1;

    //    SN76496_set_volume(chip,intf->volume[chip] & 0xff,(intf->volume[chip] >> 8) & 0xff);
    //}
    return 0;
}
