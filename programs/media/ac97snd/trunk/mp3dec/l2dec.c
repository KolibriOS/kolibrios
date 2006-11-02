#include "bstream.h"
#include "mp3dec.h"

extern float m_sample[2304];
extern int	m_nsb_limit;
int m_stereo_sb;
int m_max_sb;

SBT_PROC m_sbt_proc;
float m_sf_table[64];
float m_look_c_valueL2[18];
char m_group3_table[32][3];
char m_group5_table[128][3];
short m_group9_table[1024][3];
int m_nbat[4];// = {3, 8, 12, 7};
int m_bat[4][16];

int m_ballo[64];
uint m_samp_dispatch[66];
float m_c_value[64];
uint m_sf_dispatch[66];
float m_cs_factor[3][64];

int m_bit_skip;

static const int look_joint[16] =
{				/* lookup stereo sb's by mode+ext */
   64, 64, 64, 64,		/* stereo */
   2 * 4, 2 * 8, 2 * 12, 2 * 16,	/* joint */
   64, 64, 64, 64,		/* dual */
   32, 32, 32, 32,		/* mono */
};

static const int bat_bit_masterL2[] =
{
   0, 5, 7, 9, 10, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48
};

void unpack_ba();
void unpack_sfs();
void unpack_sf();
void unpack_samp();

void L2decode_frame(MPEG_HEADER* h, byte* mpeg, byte* pcm)
{
	int crc_size;

	crc_size = (h->error_prot) ? 2 : 0;
	bitget_init(mpeg + 4 + crc_size);

	m_stereo_sb = look_joint[(h->mode << 2) + h->mode_ext];
	unpack_ba();		// unpack bit allocation
	unpack_sfs();		// unpack scale factor selectors
	unpack_sf();		// unpack scale factor
	unpack_samp();		// unpack samples

	m_sbt_proc(m_sample, pcm, 36);
}

void unpack_ba()
{
	int i, j, k;
	int nstereo;
	int nbit[4] = {4, 4, 3, 2};

	m_bit_skip = 0;
	nstereo = m_stereo_sb;
	k = 0;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < m_nbat[i]; j++, k++) {
			bitget_check(4);
			m_ballo[k] = m_samp_dispatch[k] = m_bat[i][mac_bitget(nbit[i])];
			if (k >= m_nsb_limit)
				m_bit_skip += bat_bit_masterL2[m_samp_dispatch[k]];
			m_c_value[k] = m_look_c_valueL2[m_samp_dispatch[k]];
			if (--nstereo < 0) {
				m_ballo[k + 1] = m_ballo[k];
				m_samp_dispatch[k] += 18;	/* flag as joint */
				m_samp_dispatch[k + 1] = m_samp_dispatch[k];	/* flag for sf */
				m_c_value[k + 1] = m_c_value[k];
				k++;
				j++;
			}
		}
	}
	m_samp_dispatch[m_nsb_limit] = 37;	/* terminate the dispatcher with skip */
	m_samp_dispatch[k] = 36;	/* terminate the dispatcher */
}

void unpack_sfs()	/* unpack scale factor selectors */
{
	int i;

	for (i = 0; i < m_max_sb; i++) {
		bitget_check(2);
		if (m_ballo[i])
			m_sf_dispatch[i] = mac_bitget(2);
		else
			m_sf_dispatch[i] = 4;	/* no allo */
	}
	m_sf_dispatch[i] = 5;		/* terminate dispatcher */
}

void unpack_sf()		/* unpack scale factor */
{				/* combine dequant and scale factors */
   int i;

   i = -1;
 dispatch:switch (m_sf_dispatch[++i])
   {
      case 0:			/* 3 factors 012 */
	 bitget_check(18);
	 m_cs_factor[0][i] = m_c_value[i] * m_sf_table[mac_bitget(6)];
	 m_cs_factor[1][i] = m_c_value[i] * m_sf_table[mac_bitget(6)];
	 m_cs_factor[2][i] = m_c_value[i] * m_sf_table[mac_bitget(6)];
	 goto dispatch;
      case 1:			/* 2 factors 002 */
	 bitget_check(12);
	 m_cs_factor[1][i] = m_cs_factor[0][i] = m_c_value[i] * m_sf_table[mac_bitget(6)];
	 m_cs_factor[2][i] = m_c_value[i] * m_sf_table[mac_bitget(6)];
	 goto dispatch;
      case 2:			/* 1 factor 000 */
	 bitget_check(6);
	 m_cs_factor[2][i] = m_cs_factor[1][i] = m_cs_factor[0][i] =
	    m_c_value[i] * m_sf_table[mac_bitget(6)];
	 goto dispatch;
      case 3:			/* 2 factors 022 */
	 bitget_check(12);
	 m_cs_factor[0][i] = m_c_value[i] * m_sf_table[mac_bitget(6)];
	 m_cs_factor[2][i] = m_cs_factor[1][i] = m_c_value[i] * m_sf_table[mac_bitget(6)];
	 goto dispatch;
      case 4:			/* no allo */
/*-- m_cs_factor[2][i] = m_cs_factor[1][i] = m_cs_factor[0][i] = 0.0;  --*/
	 goto dispatch;
      case 5:			/* all done */
	 ;
   }				/* end switch */
}
/*-------------------------------------------------------------------------*/
#define UNPACK_N(n) s[k]     =  m_cs_factor[i][k]*(bitget(n)-((1 << n-1) -1));   \
    s[k+64]  =  m_cs_factor[i][k]*(bitget(n)-((1 << n-1) -1));   \
    s[k+128] =  m_cs_factor[i][k]*(bitget(n)-((1 << n-1) -1));   \
    goto dispatch;
#define UNPACK_N2(n) bitget_check(3*n);                                         \
    s[k]     =  m_cs_factor[i][k]*(mac_bitget(n)-((1 << n-1) -1));   \
    s[k+64]  =  m_cs_factor[i][k]*(mac_bitget(n)-((1 << n-1) -1));   \
    s[k+128] =  m_cs_factor[i][k]*(mac_bitget(n)-((1 << n-1) -1));   \
    goto dispatch;
#define UNPACK_N3(n) bitget_check(2*n);                                         \
    s[k]     =  m_cs_factor[i][k]*(mac_bitget(n)-((1 << n-1) -1));   \
    s[k+64]  =  m_cs_factor[i][k]*(mac_bitget(n)-((1 << n-1) -1));   \
    bitget_check(n);                                           \
    s[k+128] =  m_cs_factor[i][k]*(mac_bitget(n)-((1 << n-1) -1));   \
    goto dispatch;
#define UNPACKJ_N(n) tmp        =  (bitget(n)-((1 << n-1) -1));                 \
    s[k]       =  m_cs_factor[i][k]*tmp;                       \
    s[k+1]     =  m_cs_factor[i][k+1]*tmp;                     \
    tmp        =  (bitget(n)-((1 << n-1) -1));                 \
    s[k+64]    =  m_cs_factor[i][k]*tmp;                       \
    s[k+64+1]  =  m_cs_factor[i][k+1]*tmp;                     \
    tmp        =  (bitget(n)-((1 << n-1) -1));                 \
    s[k+128]   =  m_cs_factor[i][k]*tmp;                       \
    s[k+128+1] =  m_cs_factor[i][k+1]*tmp;                     \
    k++;       /* skip right chan dispatch */                \
    goto dispatch;
/*-------------------------------------------------------------------------*/

void unpack_samp()	/* unpack samples */
{
   int i, j, k;
   float *s;
   int n;
   long tmp;

   s = m_sample;
   for (i = 0; i < 3; i++)
   {				/* 3 groups of scale factors */
      for (j = 0; j < 4; j++)
      {
	 k = -1;
       dispatch:switch (m_samp_dispatch[++k])
	 {
	    case 0:
	       s[k + 128] = s[k + 64] = s[k] = 0.0F;
	       goto dispatch;
	    case 1:		/* 3 levels grouped 5 bits */
	       bitget_check(5);
	       n = mac_bitget(5);
	       s[k] = m_cs_factor[i][k] * m_group3_table[n][0];
	       s[k + 64] = m_cs_factor[i][k] * m_group3_table[n][1];
	       s[k + 128] = m_cs_factor[i][k] * m_group3_table[n][2];
	       goto dispatch;
	    case 2:		/* 5 levels grouped 7 bits */
	       bitget_check(7);
	       n = mac_bitget(7);
	       s[k] = m_cs_factor[i][k] * m_group5_table[n][0];
	       s[k + 64] = m_cs_factor[i][k] * m_group5_table[n][1];
	       s[k + 128] = m_cs_factor[i][k] * m_group5_table[n][2];
	       goto dispatch;
	    case 3:
	       UNPACK_N2(3)	/* 7 levels */
	    case 4:		/* 9 levels grouped 10 bits */
	       bitget_check(10);
	       n = mac_bitget(10);
	       s[k] = m_cs_factor[i][k] * m_group9_table[n][0];
	       s[k + 64] = m_cs_factor[i][k] * m_group9_table[n][1];
	       s[k + 128] = m_cs_factor[i][k] * m_group9_table[n][2];
	       goto dispatch;
	    case 5:
	       UNPACK_N2(4)	/* 15 levels */
	    case 6:
	       UNPACK_N2(5)	/* 31 levels */
	    case 7:
	       UNPACK_N2(6)	/* 63 levels */
	    case 8:
	       UNPACK_N2(7)	/* 127 levels */
	    case 9:
	       UNPACK_N2(8)	/* 255 levels */
	    case 10:
	       UNPACK_N3(9)	/* 511 levels */
	    case 11:
	       UNPACK_N3(10)	/* 1023 levels */
	    case 12:
	       UNPACK_N3(11)	/* 2047 levels */
	    case 13:
	       UNPACK_N3(12)	/* 4095 levels */
	    case 14:
	       UNPACK_N(13)	/* 8191 levels */
	    case 15:
	       UNPACK_N(14)	/* 16383 levels */
	    case 16:
	       UNPACK_N(15)	/* 32767 levels */
	    case 17:
	       UNPACK_N(16)	/* 65535 levels */
/* -- joint ---- */
	    case 18 + 0:
	       s[k + 128 + 1] = s[k + 128] = s[k + 64 + 1] = s[k + 64] = s[k + 1] = s[k] = 0.0F;
	       k++;		/* skip right chan dispatch */
	       goto dispatch;
	    case 18 + 1:	/* 3 levels grouped 5 bits */
	       n = bitget(5);
	       s[k] = m_cs_factor[i][k] * m_group3_table[n][0];
	       s[k + 1] = m_cs_factor[i][k + 1] * m_group3_table[n][0];
	       s[k + 64] = m_cs_factor[i][k] * m_group3_table[n][1];
	       s[k + 64 + 1] = m_cs_factor[i][k + 1] * m_group3_table[n][1];
	       s[k + 128] = m_cs_factor[i][k] * m_group3_table[n][2];
	       s[k + 128 + 1] = m_cs_factor[i][k + 1] * m_group3_table[n][2];
	       k++;		/* skip right chan dispatch */
	       goto dispatch;
	    case 18 + 2:	/* 5 levels grouped 7 bits */
	       n = bitget(7);
	       s[k] = m_cs_factor[i][k] * m_group5_table[n][0];
	       s[k + 1] = m_cs_factor[i][k + 1] * m_group5_table[n][0];
	       s[k + 64] = m_cs_factor[i][k] * m_group5_table[n][1];
	       s[k + 64 + 1] = m_cs_factor[i][k + 1] * m_group5_table[n][1];
	       s[k + 128] = m_cs_factor[i][k] * m_group5_table[n][2];
	       s[k + 128 + 1] = m_cs_factor[i][k + 1] * m_group5_table[n][2];
	       k++;		/* skip right chan dispatch */
	       goto dispatch;
	    case 18 + 3:
	       UNPACKJ_N(3)	/* 7 levels */
	    case 18 + 4:	/* 9 levels grouped 10 bits */
	       n = bitget(10);
	       s[k] = m_cs_factor[i][k] * m_group9_table[n][0];
	       s[k + 1] = m_cs_factor[i][k + 1] * m_group9_table[n][0];
	       s[k + 64] = m_cs_factor[i][k] * m_group9_table[n][1];
	       s[k + 64 + 1] = m_cs_factor[i][k + 1] * m_group9_table[n][1];
	       s[k + 128] = m_cs_factor[i][k] * m_group9_table[n][2];
	       s[k + 128 + 1] = m_cs_factor[i][k + 1] * m_group9_table[n][2];
	       k++;		/* skip right chan dispatch */
	       goto dispatch;
	    case 18 + 5:
	       UNPACKJ_N(4)	/* 15 levels */
	    case 18 + 6:
	       UNPACKJ_N(5)	/* 31 levels */
	    case 18 + 7:
	       UNPACKJ_N(6)	/* 63 levels */
	    case 18 + 8:
	       UNPACKJ_N(7)	/* 127 levels */
	    case 18 + 9:
	       UNPACKJ_N(8)	/* 255 levels */
	    case 18 + 10:
	       UNPACKJ_N(9)	/* 511 levels */
	    case 18 + 11:
	       UNPACKJ_N(10)	/* 1023 levels */
	    case 18 + 12:
	       UNPACKJ_N(11)	/* 2047 levels */
	    case 18 + 13:
	       UNPACKJ_N(12)	/* 4095 levels */
	    case 18 + 14:
	       UNPACKJ_N(13)	/* 8191 levels */
	    case 18 + 15:
	       UNPACKJ_N(14)	/* 16383 levels */
	    case 18 + 16:
	       UNPACKJ_N(15)	/* 32767 levels */
	    case 18 + 17:
	       UNPACKJ_N(16)	/* 65535 levels */
/* -- end of dispatch -- */
	    case 37:
	       bitget_skip(m_bit_skip);
	    case 36:
	       s += 3 * 64;
	 }			/* end switch */
      }				/* end j loop */
   }				/* end i loop */
}
