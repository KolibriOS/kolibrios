#include "layer3.h"

extern MPEG_DECODE_OPTION m_option;
extern SAMPLE		m_sample[2][2][576];
extern int			m_frequency;

extern SBT_PROC		m_sbt_proc;
extern XFORM_PROC	m_xform_proc;
extern int			m_channels;
extern int			m_sfBandIndex[2][22];// [long/short][cb]
extern int			m_nBand[2][22];
extern int			m_band_limit;
extern int			m_band_limit21;		// limit for sf band 21
extern int			m_band_limit12;		// limit for sf band 12 short
extern int			m_band_limit_nsb;
extern int			m_ncbl_mixed;
extern int			m_nsb_limit;

extern int			m_gr;
extern int			m_buf_ptr0, m_buf_ptr1;
extern float		m_yout[576];

//extern "l3sbt.c"
void sbt_mono_L3(float *sample, signed short *pcm, int ch);
void sbt_dual_L3(float *sample, signed short *pcm, int ch);
void sbt16_mono_L3(float *sample, signed short *pcm, int ch);
void sbt16_dual_L3(float *sample, signed short *pcm, int ch);
void sbt8_mono_L3(float *sample, signed short *pcm, int ch);
void sbt8_dual_L3(float *sample, signed short *pcm, int ch);
void sbtB_mono_L3(float *sample, unsigned char *pcm, int ch);
void sbtB_dual_L3(float *sample, unsigned char *pcm, int ch);
void sbtB16_mono_L3(float *sample, unsigned char *pcm, int ch);
void sbtB16_dual_L3(float *sample, unsigned char *pcm, int ch);
void sbtB8_mono_L3(float *sample, unsigned char *pcm, int ch);
void sbtB8_dual_L3(float *sample, unsigned char *pcm, int ch);

//extern "l3dec.c"
void xform_mono(void *pcm, int igr);
void xform_dual(void *pcm, int igr);
void xform_dual_mono(void *pcm, int igr);
void xform_dual_right(void *pcm, int igr);

static const SBT_PROC sbt_table[2][3][2] =
{
	sbt_mono_L3,
	sbt_dual_L3,
	sbt16_mono_L3,
	sbt16_dual_L3,
	sbt8_mono_L3,
	sbt8_dual_L3,
// 8 bit output
	sbtB_mono_L3,
	sbtB_dual_L3,
	sbtB16_mono_L3,
	sbtB16_dual_L3,
	sbtB8_mono_L3,
	sbtB8_dual_L3,
};

static const XFORM_PROC xform_table[5] =
{
	xform_mono,
	xform_dual,
	xform_dual_mono,
	xform_mono,			/* left */
	xform_dual_right,
};

static const struct {
   int l[23];
   int s[14];
} sfBandTable[3][3] = {
// MPEG-1
	{{
		{0, 4, 8, 12, 16, 20, 24, 30, 36, 44, 52, 62, 74, 90, 110, 134, 162, 196, 238, 288, 342, 418, 576},
		{0, 4, 8, 12, 16, 22, 30, 40, 52, 66, 84, 106, 136, 192}
	},{
		{0, 4, 8, 12, 16, 20, 24, 30, 36, 42, 50, 60, 72, 88, 106, 128, 156, 190, 230, 276, 330, 384, 576},
		{0, 4, 8, 12, 16, 22, 28, 38, 50, 64, 80, 100, 126, 192}
	},{
		{0, 4, 8, 12, 16, 20, 24, 30, 36, 44, 54, 66, 82, 102, 126, 156, 194, 240, 296, 364, 448, 550, 576},
		{0, 4, 8, 12, 16, 22, 30, 42, 58, 78, 104, 138, 180, 192}
	}},
// MPEG-2
	{{
		{0, 6, 12, 18, 24, 30, 36, 44, 54, 66, 80, 96, 116, 140, 168, 200, 238, 284, 336, 396, 464, 522, 576},
		{0, 4, 8, 12, 18, 24, 32, 42, 56, 74, 100, 132, 174, 192}
	},{
		{0, 6, 12, 18, 24, 30, 36, 44, 54, 66, 80, 96, 114, 136, 162, 194, 232, 278, 332, 394, 464, 540, 576},
		{0, 4, 8, 12, 18, 26, 36, 48, 62, 80, 104, 136, 180, 192}
	},{
		{0, 6, 12, 18, 24, 30, 36, 44, 54, 66, 80, 96, 116, 140, 168, 200, 238, 284, 336, 396, 464, 522, 576},
		{0, 4, 8, 12, 18, 26, 36, 48, 62, 80, 104, 134, 174, 192}
	}},
// MPEG-2.5, 11 & 12 KHz seem ok, 8 ok
	{{
		{0, 6, 12, 18, 24, 30, 36, 44, 54, 66, 80, 96, 116, 140, 168, 200, 238, 284, 336, 396, 464, 522, 576},
		{0, 4, 8, 12, 18, 26, 36, 48, 62, 80, 104, 134, 174, 192}
	},{
		{0, 6, 12, 18, 24, 30, 36, 44, 54, 66, 80, 96, 116, 140, 168, 200, 238, 284, 336, 396, 464, 522, 576},
		{0, 4, 8, 12, 18, 26, 36, 48, 62, 80, 104, 134, 174, 192}
	},{
// this 8khz table, and only 8khz, from mpeg123)
		{0, 12, 24, 36, 48, 60, 72, 88, 108, 132, 160, 192, 232, 280, 336, 400, 476, 566, 568, 570, 572, 574, 576},
		{0, 8, 16, 24, 36, 52, 72, 96, 124, 160, 162, 164, 166, 192}
	}},
};

void quant_init();
void alias_init();
void msis_init();
void fdct_init();
void imdct_init();
void hwin_init();

void L3table_init()
{
	quant_init();
	alias_init();
	msis_init();
	fdct_init();
	imdct_init();
	hwin_init();
}

int L3decode_start(MPEG_HEADER* h)
{
	int i, j, k, v;
	int channels, limit;
	int bit_code;

	m_buf_ptr0 = 0;
	m_buf_ptr1 = 0;
	m_gr = 0;
	v = h->version - 1;
	if (h->version == 1) //MPEG-1
		m_ncbl_mixed = 8;
	else //MPEG-2, MPEG-2.5
		m_ncbl_mixed = 6;

// compute nsb_limit
	m_nsb_limit = (m_option.freqLimit * 64L + m_frequency / 2) / m_frequency;
// caller limit
	limit = (32 >> m_option.reduction);
	if (limit > 8)
		limit--;
	if (m_nsb_limit > limit)
		m_nsb_limit = limit;
	limit = 18 * m_nsb_limit;

	if (h->version == 1) {
		//MPEG-1
		m_band_limit12 = 3 * sfBandTable[v][h->fr_index].s[13];
		m_band_limit = m_band_limit21 = sfBandTable[v][h->fr_index].l[22];
	}
	else {
		//MPEG-2, MPEG-2.5
		m_band_limit12 = 3 * sfBandTable[v][h->fr_index].s[12];
		m_band_limit = m_band_limit21 = sfBandTable[v][h->fr_index].l[21];
	}
	m_band_limit += 8;	// allow for antialias
	if (m_band_limit > limit)
		m_band_limit = limit;
	if (m_band_limit21 > m_band_limit)
		m_band_limit21 = m_band_limit;
	if (m_band_limit12 > m_band_limit)
		m_band_limit12 = m_band_limit;
	m_band_limit_nsb = (m_band_limit + 17) / 18;	// limit nsb's rounded up
/*
	gain_adjust = 0;	// adjust gain e.g. cvt to mono sum channel
	if ((h->mode != 3) && (m_option.convert == 1))
		gain_adjust = -4;
*/
	m_channels = (h->mode == 3) ? 1 : 2;
	if (m_option.convert) channels = 1;
	else channels = m_channels;

	bit_code = (m_option.convert & 8) ? 1 : 0;
	m_sbt_proc = sbt_table[bit_code][m_option.reduction][channels - 1];//[2][3][2]
	k = (h->mode != 3) ? (1 + m_option.convert) : 0;
	m_xform_proc = xform_table[k];//[5]
/*
	if (bit_code)
		zero_level_pcm = 128;// 8 bit output
	else
		zero_level_pcm = 0;
*/
// init band tables
	for (i = 0; i < 22; i ++)
		m_sfBandIndex[0][i] = sfBandTable[v][h->fr_index].l[i + 1];
	for (i = 0; i < 13; i ++)
		m_sfBandIndex[1][i] = 3 * sfBandTable[v][h->fr_index].s[i + 1];
	for (i = 0; i < 22; i ++)
		m_nBand[0][i] = sfBandTable[v][h->fr_index].l[i + 1]
				- sfBandTable[v][h->fr_index].l[i];
	for (i = 0; i < 13; i ++)
		m_nBand[1][i] = sfBandTable[v][h->fr_index].s[i + 1]
				- sfBandTable[v][h->fr_index].s[i];

// clear buffers
	for (i = 0; i < 576; i++)
		m_yout[i] = 0.0f;
	for (i = 0; i < 2; i ++)
    {	for (j = 0; j < 2; j ++)
        {	for (k = 0; k < 576; k++)
           {	m_sample[i][j][k].x = 0.0f;
				m_sample[i][j][k].s = 0;
			}
		}
	}
	return 1;
}
