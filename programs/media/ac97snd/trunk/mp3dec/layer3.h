#include "bstream.h"
#include "mp3dec.h"

typedef struct {
	uint	part2_3_length;
	uint	big_values;
	uint	global_gain;
	uint	scalefac_compress;
	uint	window_switching_flag;
	uint	block_type;
	uint	mixed_block_flag;
	uint	table_select[3];
	uint	subblock_gain[3];
	uint	region0_count;
	uint	region1_count;
	uint	preflag;
	uint	scalefac_scale;
	uint	count1table_select;
} GR_INFO;

typedef struct {
	uint	main_data_begin;
	uint	private_bits;

	uint	scfsi[2];	/* 4 bit flags [ch] */
	GR_INFO	gr[2][2];	/* [gr][ch] */
} SIDE_INFO;

typedef struct {
	int l[23];			/* [cb] */
	int s[3][13];		/* [window][cb] */
} SCALE_FACTOR;

typedef struct {
   int cbtype;			/* long=0 short=1 */
   int cbmax;			/* max crit band */
   int lb_type;			/* long block type 0 1 3 */
   int cbs0;			/* short band start index 0 3 12 (12=no shorts */
   int ncbl;			/* number long cb's 0 8 21 */
   int cbmax_s[3];		/* cbmax by individual short blocks */
} CB_INFO;

typedef struct {
	int nr[3];
	int slen[3];
	int intensity_scale;
} IS_SF_INFO;

typedef union {
	int		s;
	float	x;
} SAMPLE;
