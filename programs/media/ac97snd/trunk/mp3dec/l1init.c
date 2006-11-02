#include "bstream.h"
#include "mp3dec.h"
#include <math.h>

extern MPEG_DECODE_OPTION m_option;
extern int m_frequency;

extern float m_sample[2304];
extern int m_nsb_limit;
extern int m_max_sb;
extern int m_stereo_sb;
extern SBT_PROC	m_sbt_proc;

extern float m_look_c_valueL1[18];
extern int m_nbatL1;

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

void L1table_init()
{
	int i, stepL1;

	for (stepL1 = 4, i = 1; i < 16; i++, stepL1 <<= 1) {
		m_look_c_valueL1[i] = (float) (2.0 / (stepL1 - 1));
	}
}

int L1decode_start(MPEG_HEADER* h)
{
	int i, k, bit_code, limit;

/*- caller limit -*/
	m_nbatL1 = 32;
	m_max_sb = m_nbatL1;
	m_nsb_limit = (m_option.freqLimit * 64L + m_frequency / 2) / m_frequency;
/*---- limit = 0.94*(32>>reduction_code);  ----*/
	limit = (32 >> m_option.reduction);
	if (limit > 8)
		limit--;
	if (m_nsb_limit > limit)
		m_nsb_limit = limit;
	if (m_nsb_limit > m_max_sb)
		m_nsb_limit = m_max_sb;

	if (h->mode != 3) { /* adjust for 2 channel modes */
		m_nbatL1 *= 2;
		m_max_sb *= 2;
		m_nsb_limit *= 2;
	}
/* set sbt function */
	bit_code = (m_option.convert & 8) ? 1 : 0;
	k = (h->mode == 3) ? 0 : (1 + m_option.convert);
	m_sbt_proc = sbt_table[bit_code][m_option.reduction][k];//[2][3][2]

	for (i = 0; i < 768; i++)
		m_sample[i] = 0.0F;
	return 1;
}

