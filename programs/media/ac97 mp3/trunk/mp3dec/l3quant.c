#include "layer3.h"

#include <math.h>
#include <string.h>		//memmove

extern SIDE_INFO	m_side_info;
extern SCALE_FACTOR	m_scale_fac[2][2];	// [gr][ch]
extern CB_INFO		m_cb_info[2][2];	// [gr][ch]

extern int			m_nsamp[2][2];
extern int			m_nBand[2][22];
extern int			m_ncbl_mixed;

#define GLOBAL_GAIN_SCALE (4*15)

static const int pretab[2][22] =
{
   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 3, 3, 3, 2, 0},
};

/* 8 bit plus 2 lookup x = pow(2.0, 0.25*(global_gain-210)) */
/* two extra slots to do 1/sqrt(2) scaling for ms */
/* 4 extra slots to do 1/2 scaling for cvt to mono */
static float look_global[256 + 2 + 4];

/*-------- scaling lookup
x = pow(2.0, -0.5*(1+scalefact_scale)*scalefac + preemp)
look_scale[scalefact_scale][preemp][scalefac]
-----------------------*/
static float look_scale[2][4][32];

/*--- iSample**(4/3) lookup, -32<=i<=31 ---*/
#define ISMAX 32
static float look_pow[2 * ISMAX];

/*-- pow(2.0, -0.25*8.0*subblock_gain) --*/
static float look_subblock[8];

/*-- reorder buffer ---*/
static float re_buf[192][3];
typedef float ARRAY3[3];

void quant_init()
{
	int i;
	int scalefact_scale, preemp, scalefac;
	double tmp;

/* 8 bit plus 2 lookup x = pow(2.0, 0.25*(global_gain-210)) */
/* extra 2 for ms scaling by 1/sqrt(2) */
/* extra 4 for cvt to mono scaling by 1/2 */
	for (i = 0; i < 256 + 2 + 4; i++)
		look_global[i] = (float) pow_test(2.0, 0.25 * ((i - (2 + 4)) - 210 + GLOBAL_GAIN_SCALE));

/* x = pow(2.0, -0.5*(1+scalefact_scale)*scalefac + preemp) */
	for (scalefact_scale = 0; scalefact_scale < 2; scalefact_scale++)
	{
		for (preemp = 0; preemp < 4; preemp++)
		{
			for (scalefac = 0; scalefac < 32; scalefac++)
			{
				look_scale[scalefact_scale][preemp][scalefac] =
						(float) pow_test(2.0, -0.5 * (1 + scalefact_scale) * (scalefac + preemp));
			}
		}
	}

/*--- iSample**(4/3) lookup, -32<=i<=31 ---*/
	for (i = 0; i < 64; i++)
	{
		tmp = i - 32;
		look_pow[i] = (float) (tmp * pow_test(fabs(tmp), (1.0 / 3.0)));
	}

/*-- pow(2.0, -0.25*8.0*subblock_gain)  3 bits --*/
	for (i = 0; i < 8; i++)
	{
		look_subblock[i] = (float) pow_test(2.0, 0.25 * -8.0 * i);
	}
// quant_init_sf_band(sr_index);   replaced by code in sup.c
}

void dequant(SAMPLE Sample[], int gr, int ch)
{
	SCALE_FACTOR* sf	= &m_scale_fac[gr][ch];
	GR_INFO* gr_info	= &m_side_info.gr[gr][ch];
	CB_INFO* cb_info	= &m_cb_info[gr][ch];
	int* nsamp			= &m_nsamp[gr][ch];

   int i, j;
   int cb, n, w;
   float x0, xs;
   float xsb[3];
   double tmp;
   int ncbl;
   int cbs0;
   ARRAY3 *buf;			/* short block reorder */
   int nbands;
   int i0;
   int non_zero;
   int cbmax[3];

   nbands = *nsamp;


   ncbl = 22;			/* long block cb end */
   cbs0 = 12;			/* short block cb start */
/* ncbl_mixed = 8 or 6  mpeg1 or 2 */
   if (gr_info->block_type == 2)
   {
      ncbl = 0;
      cbs0 = 0;
      if (gr_info->mixed_block_flag)
      {
	 ncbl = m_ncbl_mixed;
	 cbs0 = 3;
      }
   }
/* fill in cb_info -- */
   cb_info->lb_type = gr_info->block_type;
   if (gr_info->block_type == 2)
      cb_info->lb_type;
   cb_info->cbs0 = cbs0;
   cb_info->ncbl = ncbl;

   cbmax[2] = cbmax[1] = cbmax[0] = 0;
/* global gain pre-adjusted by 2 if ms_mode, 0 otherwise */
   x0 = look_global[(2 + 4) + gr_info->global_gain];
   i = 0;
/*----- long blocks ---*/
   for (cb = 0; cb < ncbl; cb++)
   {
      non_zero = 0;
      xs = x0 * look_scale[gr_info->scalefac_scale][pretab[gr_info->preflag][cb]][sf->l[cb]];
      n = m_nBand[0][cb];
      for (j = 0; j < n; j++, i++)
      {
	 if (Sample[i].s == 0)
	    Sample[i].x = 0.0F;
	 else
	 {
	    non_zero = 1;
	    if ((Sample[i].s >= (-ISMAX)) && (Sample[i].s < ISMAX))
	       Sample[i].x = xs * look_pow[ISMAX + Sample[i].s];
	    else
	    {
	       tmp = (double) Sample[i].s;
	       Sample[i].x = (float) (xs * tmp * pow_test(fabs(tmp), (1.0 / 3.0)));
	    }
	 }
      }
      if (non_zero)
	 cbmax[0] = cb;
      if (i >= nbands)
	 break;
   }

   cb_info->cbmax = cbmax[0];
   cb_info->cbtype = 0;		// type = long

   if (cbs0 >= 12)
      return;
/*---------------------------
block type = 2  short blocks
----------------------------*/
   cbmax[2] = cbmax[1] = cbmax[0] = cbs0;
   i0 = i;			/* save for reorder */
   buf = re_buf;
   for (w = 0; w < 3; w++)
      xsb[w] = x0 * look_subblock[gr_info->subblock_gain[w]];
   for (cb = cbs0; cb < 13; cb++)
   {
      n = m_nBand[1][cb];
      for (w = 0; w < 3; w++)
      {
	 non_zero = 0;
	 xs = xsb[w] * look_scale[gr_info->scalefac_scale][0][sf->s[w][cb]];
	 for (j = 0; j < n; j++, i++)
	 {
	    if (Sample[i].s == 0)
	       buf[j][w] = 0.0F;
	    else
	    {
	       non_zero = 1;
	       if ((Sample[i].s >= (-ISMAX)) && (Sample[i].s < ISMAX))
		  buf[j][w] = xs * look_pow[ISMAX + Sample[i].s];
	       else
	       {
		  tmp = (double) Sample[i].s;
		  buf[j][w] = (float) (xs * tmp * pow_test(fabs(tmp), (1.0 / 3.0)));
	       }
	    }
	 }
	 if (non_zero)
	    cbmax[w] = cb;
      }
      if (i >= nbands)
	 break;
      buf += n;
   }


   memmove(&Sample[i0].x, &re_buf[0][0], sizeof(float) * (i - i0));

   *nsamp = i;			/* update nsamp */
   cb_info->cbmax_s[0] = cbmax[0];
   cb_info->cbmax_s[1] = cbmax[1];
   cb_info->cbmax_s[2] = cbmax[2];
   if (cbmax[1] > cbmax[0])
      cbmax[0] = cbmax[1];
   if (cbmax[2] > cbmax[0])
      cbmax[0] = cbmax[2];

   cb_info->cbmax = cbmax[0];
   cb_info->cbtype = 1;		/* type = short */
}
