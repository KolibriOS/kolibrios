#include "layer3.h"
#include <string.h>
#include <math.h>

#ifndef min
	#define max(a,b)    (((a) > (b)) ? (a) : (b))
	#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

extern int		m_frame_size, m_pcm_size;

// shared
SAMPLE			m_sample[2][2][576];//- sample union of int/float  sample[ch][gr][576]
int				m_nsb_limit;
SBT_PROC		m_sbt_proc;

XFORM_PROC		m_xform_proc;
int				m_channels;			//(mode == 3) ? 1 : 2
int				m_ms_mode, m_is_mode;
int				m_sfBandIndex[2][22];// [long/short][cb]
int				m_nBand[2][22];
int				m_band_limit;
int				m_band_limit21;		// limit for sf band 21
int				m_band_limit12;		// limit for sf band 12 short
int				m_band_limit_nsb;
int				m_ncbl_mixed;

SIDE_INFO		m_side_info;
SCALE_FACTOR	m_scale_fac[2][2];	// [gr][ch]
CB_INFO			m_cb_info[2][2];	// [gr][ch]
IS_SF_INFO		m_is_sf_info;

#define NBUF (8*1024)
#define BUF_TRIGGER (NBUF-1500)

int				m_gr;
int				m_main_pos_bit;
byte			m_buf[NBUF];
int				m_buf_ptr0, m_buf_ptr1;
int				m_nsamp[2][2];		// must start = 0, for m_nsamp[igr_prev]
float			m_yout[576];		// hybrid out, sbt in

//extern "l3side.c"
int L3get_side_info1();
int L3get_side_info2(int gr);

//extern "l3sf.c"
void L3get_scale_factor1(int gr, int ch);
void L3get_scale_factor2(int gr, int ch);

void huffman(void *xy, int n, int ntable);
int huffman_quad(void *vwxy, int n, int nbits, int ntable);
void dequant(SAMPLE sample[], int gr, int ch);
void antialias(void *x, int n);
void ms_process(void *x, int n);
void is_process1(void *x, SCALE_FACTOR* sf,
		CB_INFO cb_info[2], int nsamp);
void is_process2(void *x, SCALE_FACTOR * sf,
		CB_INFO cb_info[2], int nsamp);

//extern "l3hybrid.c"
int hybrid(void *xin, void *xprev, float *y,
		int btype, int nlong, int ntot, int nprev);
int hybrid_sum(void *xin, void *xin_left, float *y,
		int btype, int nlong, int ntot);
void sum_f_bands(void *a, void *b, int n);
void freq_invert(float *y, int n); /* xform, */

void L3decode_main(MPEG_HEADER* h, byte *pcm, int gr);

void L3decode_reset()
{
	m_buf_ptr0 = m_buf_ptr1 = 0;
}

void L3decode_frame(MPEG_HEADER* h, byte* mpeg, byte* pcm)
{
	int crc_size, side_size;
	int copy_size;

	if (h->mode == 1) {
		m_ms_mode = h->mode_ext >> 1;
		m_is_mode = h->mode_ext & 1;
	}
	else {
		m_ms_mode = 0;
		m_is_mode = 0;
	}

	crc_size = (h->error_prot) ? 2 : 0;
	bitget_init(mpeg + 4 + crc_size);
	if (h->version == 1)
		side_size = L3get_side_info1();
	else
		side_size = L3get_side_info2(m_gr);

	m_buf_ptr0 = m_buf_ptr1 - m_side_info.main_data_begin;/* decode start point */
	if (m_buf_ptr1 > BUF_TRIGGER) { /* shift buffer */
		memmove(m_buf, m_buf + m_buf_ptr0, m_side_info.main_data_begin);
		m_buf_ptr0 = 0;
		m_buf_ptr1 = m_side_info.main_data_begin;
	}
	copy_size = m_frame_size - (4 + crc_size + side_size);
	//24/02/02 X-MaD
	if (copy_size < 0) { copy_size = copy_size * -1; }
	//if (copy_size < 0) { copy_size = 0; }
	//__try {
		memmove(m_buf + m_buf_ptr1, mpeg + (4 + crc_size + side_size), copy_size);
	//}    __except(0){
	//	m_buf_ptr1 = 0;		
	//}
	m_buf_ptr1 += copy_size; 
	//24/02/02 X-MaD

	if (m_buf_ptr0 >= 0) {
		m_main_pos_bit = m_buf_ptr0 << 3;
		if (h->version == 1) {
			L3decode_main(h, pcm, 0);
			L3decode_main(h, pcm + (m_pcm_size / 2), 1);
		}
		else {
			L3decode_main(h, pcm, m_gr);
			m_gr = m_gr ^ 1;
		}
	}
}

void L3decode_main(MPEG_HEADER* h, byte *pcm, int gr)
{
	int ch;
	int n1, n2, n3, n4, nn2, nn3, nn4;
	int bit0, qbits, m0;

	for (ch = 0; ch < m_channels; ch ++) {
		bitget_init(m_buf + (m_main_pos_bit >> 3));
		bit0 = (m_main_pos_bit & 7);
		if (bit0) bitget(bit0);
		m_main_pos_bit += m_side_info.gr[gr][ch].part2_3_length;
		bitget_init_end(m_buf + ((m_main_pos_bit + 39) >> 3));
// scale factors
		if (h->version == 1)
			L3get_scale_factor1(gr, ch);
		else
			L3get_scale_factor2(gr, ch);
// huff data
		n1 = m_sfBandIndex[0][m_side_info.gr[gr][ch].region0_count];
		n2 = m_sfBandIndex[0][m_side_info.gr[gr][ch].region0_count
				+ m_side_info.gr[gr][ch].region1_count + 1];
		n3 = m_side_info.gr[gr][ch].big_values;
		n3 = n3 + n3;

		if (n3 > m_band_limit) n3 = m_band_limit;
		if (n2 > n3) n2 = n3;
		if (n1 > n3) n1 = n3;
		nn3 = n3 - n2;
		nn2 = n2 - n1;
		huffman(m_sample[ch][gr], n1, m_side_info.gr[gr][ch].table_select[0]);
		huffman(m_sample[ch][gr] + n1, nn2, m_side_info.gr[gr][ch].table_select[1]);
		huffman(m_sample[ch][gr] + n2, nn3, m_side_info.gr[gr][ch].table_select[2]);
		qbits = m_side_info.gr[gr][ch].part2_3_length - (bitget_bits_used() - bit0);
		nn4 = huffman_quad(m_sample[ch][gr] + n3, m_band_limit - n3, qbits,
				m_side_info.gr[gr][ch].count1table_select);
		n4 = n3 + nn4;
		m_nsamp[gr][ch] = n4;
		// limit n4 or allow deqaunt to sf band 22
		if (m_side_info.gr[gr][ch].block_type == 2)
			n4 = min(n4, m_band_limit12);
		else
			n4 = min(n4, m_band_limit21);
		if (n4 < 576)
			memset(m_sample[ch][gr] + n4, 0, sizeof(SAMPLE) * (576 - n4));
		if (bitget_overrun())
			memset(m_sample[ch][gr], 0, sizeof(SAMPLE) * (576));
	}
// dequant
	for (ch = 0; ch < m_channels; ch++) {
		dequant(m_sample[ch][gr], gr, ch);
	}
// ms stereo processing
	if (m_ms_mode) {
		if (m_is_mode == 0) {
			m0 = m_nsamp[gr][0];	// process to longer of left/right
			if (m0 < m_nsamp[gr][1])
				m0 = m_nsamp[gr][1];
		}
		else {// process to last cb in right
			m0 = m_sfBandIndex[m_cb_info[gr][1].cbtype][m_cb_info[gr][1].cbmax];
		}
		ms_process(m_sample[0][gr], m0);
	}
// is stereo processing
	if (m_is_mode) {
		if (h->version == 1)
			is_process1(m_sample[0][gr], &m_scale_fac[gr][1],
					m_cb_info[gr], m_nsamp[gr][0]);
		else
			is_process2(m_sample[0][gr], &m_scale_fac[gr][1],
					m_cb_info[gr], m_nsamp[gr][0]);
	}
// adjust ms and is modes to max of left/right
	if (m_ms_mode || m_is_mode) {
		if (m_nsamp[gr][0] < m_nsamp[gr][1])
			m_nsamp[gr][0] = m_nsamp[gr][1];
		else
			m_nsamp[gr][1] = m_nsamp[gr][0];
	}

// antialias
	for (ch = 0; ch < m_channels; ch ++) {
		if (m_cb_info[gr][ch].ncbl == 0)
			continue;		// have no long blocks
		if (m_side_info.gr[gr][ch].mixed_block_flag)
			n1 = 1;		// 1 -> 36 samples
		else
			n1 = (m_nsamp[gr][ch] + 7) / 18;
		if (n1 > 31)
			n1 = 31;
		antialias(m_sample[ch][gr], n1);
		n1 = 18 * n1 + 8;		// update number of samples
		if (n1 > m_nsamp[gr][ch])
			m_nsamp[gr][ch] = n1;
	}
// hybrid + sbt
	m_xform_proc(pcm, gr);
}

void xform_mono(void *pcm, int igr)
{
	int igr_prev, n1, n2;

// hybrid + sbt
	n1 = n2 = m_nsamp[igr][0];	// total number bands
	if (m_side_info.gr[igr][0].block_type == 2) {	// long bands
		if (m_side_info.gr[igr][0].mixed_block_flag)
			n1 = m_sfBandIndex[0][m_ncbl_mixed - 1];
		else
			n1 = 0;
	}
	if (n1 > m_band_limit)
		n1 = m_band_limit;
	if (n2 > m_band_limit)
		n2 = m_band_limit;
	igr_prev = igr ^ 1;

	m_nsamp[igr][0] = hybrid(m_sample[0][igr], m_sample[0][igr_prev],
			m_yout, m_side_info.gr[igr][0].block_type, n1, n2, m_nsamp[igr_prev][0]);
	freq_invert(m_yout, m_nsamp[igr][0]);
	m_sbt_proc(m_yout, pcm, 0);
}

void xform_dual_right(void *pcm, int igr)
{
	int igr_prev, n1, n2;

// hybrid + sbt
	n1 = n2 = m_nsamp[igr][1];	// total number bands
	if (m_side_info.gr[igr][1].block_type == 2) {	// long bands
		if (m_side_info.gr[igr][1].mixed_block_flag)
			n1 = m_sfBandIndex[0][m_ncbl_mixed - 1];
		else
			n1 = 0;
	}
	if (n1 > m_band_limit)
		n1 = m_band_limit;
	if (n2 > m_band_limit)
		n2 = m_band_limit;
	igr_prev = igr ^ 1;
	m_nsamp[igr][1] = hybrid(m_sample[1][igr], m_sample[1][igr_prev],
			m_yout, m_side_info.gr[igr][1].block_type, n1, n2, m_nsamp[igr_prev][1]);
	freq_invert(m_yout, m_nsamp[igr][1]);
	m_sbt_proc(m_yout, pcm, 0);
}

void xform_dual(void *pcm, int igr)
{
	int ch;
	int igr_prev, n1, n2;

// hybrid + sbt
	igr_prev = igr ^ 1;
	for (ch = 0; ch < m_channels; ch++) {
		n1 = n2 = m_nsamp[igr][ch];	// total number bands
		if (m_side_info.gr[igr][ch].block_type == 2) { // long bands
			if (m_side_info.gr[igr][ch].mixed_block_flag)
				n1 = m_sfBandIndex[0][m_ncbl_mixed - 1];
			else
				n1 = 0;
		}
		if (n1 > m_band_limit)
			n1 = m_band_limit;
		if (n2 > m_band_limit)
			n2 = m_band_limit;
		m_nsamp[igr][ch] = hybrid(m_sample[ch][igr], m_sample[ch][igr_prev],
				m_yout, m_side_info.gr[igr][ch].block_type, n1, n2, m_nsamp[igr_prev][ch]);
		freq_invert(m_yout, m_nsamp[igr][ch]);
		m_sbt_proc(m_yout, pcm, ch);
	}
}

void xform_dual_mono(void *pcm, int igr)
{
	int igr_prev, n1, n2, n3;

// hybrid + sbt
	igr_prev = igr ^ 1;
	if ((m_side_info.gr[igr][0].block_type == m_side_info.gr[igr][1].block_type)
			&& (m_side_info.gr[igr][0].mixed_block_flag == 0)
			&& (m_side_info.gr[igr][1].mixed_block_flag == 0)) {
		n2 = m_nsamp[igr][0];	// total number bands max of L R
		if (n2 < m_nsamp[igr][1])
			n2 = m_nsamp[igr][1];
		if (n2 > m_band_limit)
			n2 = m_band_limit;
		if (m_side_info.gr[igr][0].block_type == 2)
			n1 = 0;
		else
			n1 = n2;		// n1 = number long bands
		sum_f_bands(m_sample[0][igr], m_sample[1][igr], n2);
		n3 = m_nsamp[igr][0] = hybrid(m_sample[0][igr], m_sample[0][igr_prev],
				m_yout, m_side_info.gr[igr][0].block_type, n1, n2, m_nsamp[igr_prev][0]);
	}
	else {	// transform and then sum (not tested - never happens in test)
// left chan
		n1 = n2 = m_nsamp[igr][0];	// total number bands
		if (m_side_info.gr[igr][0].block_type == 2) {	// long bands
			if (m_side_info.gr[igr][0].mixed_block_flag)
				n1 = m_sfBandIndex[0][m_ncbl_mixed - 1];
			else
				n1 = 0;
		}
		n3 = m_nsamp[igr][0] = hybrid(m_sample[0][igr], m_sample[0][igr_prev],
				m_yout, m_side_info.gr[igr][0].block_type, n1, n2, m_nsamp[igr_prev][0]);
// right chan
		n1 = n2 = m_nsamp[igr][1];	// total number bands
		if (m_side_info.gr[igr][1].block_type == 2) {	// long bands
			if (m_side_info.gr[igr][1].mixed_block_flag)
				n1 = m_sfBandIndex[0][m_ncbl_mixed - 1];
			else
				n1 = 0;
		}
		m_nsamp[igr][1] = hybrid_sum(m_sample[1][igr], m_sample[0][igr],
				m_yout, m_side_info.gr[igr][1].block_type, n1, n2);
		if (n3 < m_nsamp[igr][1])
			n1 = m_nsamp[igr][1];
	}

	freq_invert(m_yout, n3);
	m_sbt_proc(m_yout, pcm, 0);
}

