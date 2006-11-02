#include "layer3.h"

extern int			m_channels;
extern int			m_ms_mode, m_is_mode;

extern SIDE_INFO	m_side_info;

int L3get_side_info1()
{
	int gr, ch, size;

	m_side_info.main_data_begin = bitget(9);
	if (m_channels == 1) {
		m_side_info.private_bits = bitget(5);
		size = 17;
	}
	else {
		m_side_info.private_bits = bitget(3);
		size = 32;
	}
	for (ch = 0; ch < m_channels; ch ++)
		m_side_info.scfsi[ch] = bitget(4);

	for (gr = 0; gr < 2; gr ++) {
		for (ch = 0; ch < m_channels; ch ++) {
			GR_INFO* gr_info = &m_side_info.gr[gr][ch];
			gr_info->part2_3_length = bitget(12);
			gr_info->big_values = bitget(9);
			gr_info->global_gain = bitget(8);
			//gr_info->global_gain += gain_adjust;
			if (m_ms_mode) gr_info->global_gain -= 2;
			gr_info->scalefac_compress = bitget(4);
			gr_info->window_switching_flag = bitget(1);
			if (gr_info->window_switching_flag) {
				gr_info->block_type = bitget(2);
				gr_info->mixed_block_flag = bitget(1);
				gr_info->table_select[0] = bitget(5);
				gr_info->table_select[1] = bitget(5);
				gr_info->subblock_gain[0] = bitget(3);
				gr_info->subblock_gain[1] = bitget(3);
				gr_info->subblock_gain[2] = bitget(3);
				/* region count set in terms of long block cb's/bands */
				/* r1 set so r0+r1+1 = 21 (lookup produces 576 bands ) */
				/* if(window_switching_flag) always 36 samples in region0 */
				gr_info->region0_count = (8 - 1);	/* 36 samples */
				gr_info->region1_count = 20 - (8 - 1);
			}
			else {
				gr_info->mixed_block_flag = 0;
				gr_info->block_type = 0;
				gr_info->table_select[0] = bitget(5);
				gr_info->table_select[1] = bitget(5);
				gr_info->table_select[2] = bitget(5);
				gr_info->region0_count = bitget(4);
				gr_info->region1_count = bitget(3);
			}
			gr_info->preflag = bitget(1);
			gr_info->scalefac_scale = bitget(1);
			gr_info->count1table_select = bitget(1);
		}
	}
	return size;
}

int L3get_side_info2(int gr)
{
	int ch, size;

	m_side_info.main_data_begin = bitget(8);
	if (m_channels == 1) {
		m_side_info.private_bits = bitget(1);
		size = 9;
	}
	else {
		m_side_info.private_bits = bitget(2);
		size = 17;
	}
	m_side_info.scfsi[0] = 0;
	m_side_info.scfsi[1] = 0;

	for (ch = 0; ch < m_channels; ch ++) {
		m_side_info.gr[gr][ch].part2_3_length = bitget(12);
		m_side_info.gr[gr][ch].big_values = bitget(9);
		m_side_info.gr[gr][ch].global_gain = bitget(8);// + gain_adjust;
		if (m_ms_mode) m_side_info.gr[gr][ch].global_gain -= 2;
		m_side_info.gr[gr][ch].scalefac_compress = bitget(9);
		m_side_info.gr[gr][ch].window_switching_flag = bitget(1);
		if (m_side_info.gr[gr][ch].window_switching_flag) {
			m_side_info.gr[gr][ch].block_type = bitget(2);
			m_side_info.gr[gr][ch].mixed_block_flag = bitget(1);
			m_side_info.gr[gr][ch].table_select[0] = bitget(5);
			m_side_info.gr[gr][ch].table_select[1] = bitget(5);
			m_side_info.gr[gr][ch].subblock_gain[0] = bitget(3);
			m_side_info.gr[gr][ch].subblock_gain[1] = bitget(3);
			m_side_info.gr[gr][ch].subblock_gain[2] = bitget(3);
			/* region count set in terms of long block cb's/bands  */
			/* r1 set so r0+r1+1 = 21 (lookup produces 576 bands ) */
			/* bt=1 or 3       54 samples */
			/* bt=2 mixed=0    36 samples */
			/* bt=2 mixed=1    54 (8 long sf) samples? or maybe 36 */
			/* region0 discussion says 54 but this would mix long */
			/* and short in region0 if scale factors switch */
			/* at band 36 (6 long scale factors) */
			if ((m_side_info.gr[gr][ch].block_type == 2)) {
				m_side_info.gr[gr][ch].region0_count = (6 - 1);	/* 36 samples */
				m_side_info.gr[gr][ch].region1_count = 20 - (6 - 1);
			}
			else {/* long block type 1 or 3 */
				m_side_info.gr[gr][ch].region0_count = (8 - 1);	/* 54 samples */
				m_side_info.gr[gr][ch].region1_count = 20 - (8 - 1);
			}
		}
		else {
			m_side_info.gr[gr][ch].mixed_block_flag = 0;
			m_side_info.gr[gr][ch].block_type = 0;
			m_side_info.gr[gr][ch].table_select[0] = bitget(5);
			m_side_info.gr[gr][ch].table_select[1] = bitget(5);
			m_side_info.gr[gr][ch].table_select[2] = bitget(5);
			m_side_info.gr[gr][ch].region0_count = bitget(4);
			m_side_info.gr[gr][ch].region1_count = bitget(3);
		}
		m_side_info.gr[gr][ch].preflag = 0;
		m_side_info.gr[gr][ch].scalefac_scale = bitget(1);
		m_side_info.gr[gr][ch].count1table_select = bitget(1);
	}
	return size;
}
