#include "bstream.h"
#include "mp3dec.h"
#include <math.h>

extern MPEG_DECODE_OPTION m_option;
extern int m_frequency;

extern float m_sample[2304];
extern int m_nsb_limit;
extern int m_max_sb;
extern SBT_PROC	m_sbt_proc;

extern float m_sf_table[64];
extern float m_look_c_valueL2[18];
extern char m_group3_table[32][3];
extern char m_group5_table[128][3];
extern short m_group9_table[1024][3];
extern int m_nbat[4];// = {3, 8, 12, 7};
extern int m_bat[4][16];

/* ABCD_INDEX = lookqt[mode][sr_index][br_index]  */
/* -1 = invalid  */
static const char lookqt[4][3][16] =
{
   1, -1, -1, -1, 2, -1, 2, 0, 0, 0, 1, 1, 1, 1, 1, -1,		/*  44ks stereo */
   0, -1, -1, -1, 2, -1, 2, 0, 0, 0, 0, 0, 0, 0, 0, -1,		/*  48ks */
   1, -1, -1, -1, 3, -1, 3, 0, 0, 0, 1, 1, 1, 1, 1, -1,		/*  32ks */
   1, -1, -1, -1, 2, -1, 2, 0, 0, 0, 1, 1, 1, 1, 1, -1,		/*  44ks joint stereo */
   0, -1, -1, -1, 2, -1, 2, 0, 0, 0, 0, 0, 0, 0, 0, -1,		/*  48ks */
   1, -1, -1, -1, 3, -1, 3, 0, 0, 0, 1, 1, 1, 1, 1, -1,		/*  32ks */
   1, -1, -1, -1, 2, -1, 2, 0, 0, 0, 1, 1, 1, 1, 1, -1,		/*  44ks dual chan */
   0, -1, -1, -1, 2, -1, 2, 0, 0, 0, 0, 0, 0, 0, 0, -1,		/*  48ks */
   1, -1, -1, -1, 3, -1, 3, 0, 0, 0, 1, 1, 1, 1, 1, -1,		/*  32ks */
// mono extended beyond legal br index
//  1,2,2,0,0,0,1,1,1,1,1,1,1,1,1,-1,          /*  44ks single chan */
//  0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,-1,          /*  48ks */
//  1,3,3,0,0,0,1,1,1,1,1,1,1,1,1,-1,          /*  32ks */
// legal mono
   1, 2, 2, 0, 0, 0, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1,		/*  44ks single chan */
   0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, -1, -1,		/*  48ks */
   1, 3, 3, 0, 0, 0, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1,		/*  32ks */
};

/* bit allocation table look up */
/* table per mpeg spec tables 3b2a/b/c/d  /e is mpeg2 */
/* look_bat[abcd_index][4][16]  */
static const unsigned char look_bat[5][4][16] =
{
/* LOOK_BATA */
   0, 1, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
   0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 17,
   0, 1, 2, 3, 4, 5, 6, 17, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 1, 2, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* LOOK_BATB */
   0, 1, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
   0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 17,
   0, 1, 2, 3, 4, 5, 6, 17, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 1, 2, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* LOOK_BATC */
   0, 1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 1, 2, 4, 5, 6, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* LOOK_BATD */
   0, 1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 1, 2, 4, 5, 6, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* LOOK_BATE */
   0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 1, 2, 4, 5, 6, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 1, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

/* look_nbat[abcd_index]][4] */
static const unsigned char look_nbat[5][4] =
{
   3, 8, 12, 4,
   3, 8, 12, 7,
   2, 0, 6, 0,
   2, 0, 10, 0,
   4, 0, 7, 19,
};

//extern "sbt.c"
void sbt_mono(float *sample, signed short *pcm, int ch);
void sbt_dual(float *sample, signed short *pcm, int ch);
void sbt16_mono(float *sample, signed short *pcm, int ch);
void sbt16_dual(float *sample, signed short *pcm, int ch);
void sbt8_mono(float *sample, signed short *pcm, int ch);
void sbt8_dual(float *sample, signed short *pcm, int ch);
void sbtB_mono(float *sample, unsigned char *pcm, int ch);
void sbtB_dual(float *sample, unsigned char *pcm, int ch);
void sbtB16_mono(float *sample, unsigned char *pcm, int ch);
void sbtB16_dual(float *sample, unsigned char *pcm, int ch);
void sbtB8_mono(float *sample, unsigned char *pcm, int ch);
void sbtB8_dual(float *sample, unsigned char *pcm, int ch);

static const SBT_PROC sbt_table[2][3][2] =
{
	sbt_mono, 
	sbt_dual, 
	sbt16_mono, 
	sbt16_dual, 
	sbt8_mono, 
	sbt8_dual, 
	sbtB_mono,
	sbtB_dual,
	sbtB16_mono,
	sbtB16_dual,
	sbtB8_mono,
	sbtB8_dual,
};

void L2table_init()
{
	int i, j, code;
	long stepL2[18] = {
		0, 3, 5, 7, 9, 15, 31, 63, 127,
		255, 511, 1023, 2047, 4095, 8191, 16383, 32767, 65535
	};
	//c_values (dequant)
	for (i = 1; i < 18; i++) {
		m_look_c_valueL2[i] = 2.0F / stepL2[i];
	}
	//scale factor table, scale by 32768 for 16 pcm output
	for (i = 0; i < 64; i++) {
		m_sf_table[i] = (float) (32768.0 * 2.0 * pow_test(2.0, -i / 3.0));
	}
	//grouped 3 level lookup table 5 bit token
	for (i = 0; i < 32; i++) {
		code = i;
		for (j = 0; j < 3; j++) {
			m_group3_table[i][j] = (char) ((code % 3) - 1);
			code /= 3;
		}
	}
	//grouped 5 level lookup table 7 bit token
	for (i = 0; i < 128; i++) {
		code = i;
		for (j = 0; j < 3; j++) {
			m_group5_table[i][j] = (char) ((code % 5) - 2);
			code /= 5;
		}
	}
	//grouped 9 level lookup table 10 bit token
	for (i = 0; i < 1024; i++) {
		code = i;
		for (j = 0; j < 3; j++) {
			m_group9_table[i][j] = (short) ((code % 9) - 4);
			code /= 9;
		}
	}
}

int L2decode_start(MPEG_HEADER* h)
{
	int i, j, k, bit_code, limit;
	int abcd_index;

// compute abcd index for bit allo table selection
	if (h->version == 1) // MPEG-1
		abcd_index = lookqt[h->mode][h->fr_index][h->br_index];
	else
		abcd_index = 4;	// MPEG-2, MPEG-2.5
	if (abcd_index < 0)
		return 0;		// fail invalid Layer II bit rate index

	for (i = 0; i < 4; i++) {
		m_nbat[i] = look_nbat[abcd_index][i];
		for (j = 0; j < 16; j++) {
			m_bat[i][j] = look_bat[abcd_index][i][j];
		}
	}
	m_max_sb = m_nbat[0] + m_nbat[1] + m_nbat[2] + m_nbat[3];
// compute nsb_limit
	m_nsb_limit = (m_option.freqLimit * 64L + m_frequency / 2) / m_frequency;
// caller limit
// limit = 0.94*(32>>reduction_code);
	limit = (32 >> m_option.reduction);
	if (limit > 8)
		limit--;
	if (m_nsb_limit > limit)
		m_nsb_limit = limit;
	if (m_nsb_limit > m_max_sb)
		m_nsb_limit = m_max_sb;

	if (h->mode != 3) {
		// adjust for 2 channel modes
		for (i = 0; i < 4; i++)
			m_nbat[i] *= 2;
		m_max_sb *= 2;
		m_nsb_limit *= 2;
	}

// set sbt function
	bit_code = (m_option.convert & 8) ? 1 : 0;
	k = (h->mode == 3) ? 0 : (1 + m_option.convert);
	m_sbt_proc = sbt_table[bit_code][m_option.reduction][k];//[2][3][2]
// clear sample buffer, unused sub bands must be 0
	for (i = 0; i < 2304; i++)
		m_sample[i] = 0.0F;
	return 1;
}
