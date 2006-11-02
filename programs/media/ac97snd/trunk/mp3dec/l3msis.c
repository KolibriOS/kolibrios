#include "layer3.h"
#include <math.h>
//#include <windows.h>

extern IS_SF_INFO	m_is_sf_info;
extern int			m_ms_mode, m_is_mode;

extern int			m_sfBandIndex[2][22];// [long/short][cb]
extern int			m_nBand[2][22];

/* intensity stereo */
/* if ms mode quant pre-scales all values by 1.0/sqrt(2.0) ms_mode in table
   compensates   */
/* [ms_mode 0/1][sf][left/right]  */
static float lr[2][8][2];
/* lr2[intensity_scale][ms_mode][sflen_offset+sf][left/right] */
static float lr2[2][2][64][2];

/*-------*msis_init_addr()
pi = 4.0*atan(1.0);
t = pi/12.0;
for(i=0;i<7;i++) {
    s = sin(i*t);
    c = cos(i*t);
    // ms_mode = 0 
    lr[0][i][0] = (float)(s/(s+c));
    lr[0][i][1] = (float)(c/(s+c));
    // ms_mode = 1 
    lr[1][i][0] = (float)(sqrt(2.0)*(s/(s+c)));
    lr[1][i][1] = (float)(sqrt(2.0)*(c/(s+c)));
}
//sf = 7 
//ms_mode = 0 
lr[0][i][0] = 1.0f;
lr[0][i][1] = 0.0f;
// ms_mode = 1, in is bands is routine does ms processing 
lr[1][i][0] = 1.0f;
lr[1][i][1] = 1.0f;
------------*/

/*===============================================================*/
void msis_init1()
{
   int i;
   double s, c;
   double pi;
   double t;

   pi = 4.0 * atan(1.0);
   t = pi / 12.0;
   for (i = 0; i < 7; i++)
   {
      s = sin(i * t);
      c = cos(i * t);
    /* ms_mode = 0 */
      lr[0][i][0] = (float) (s / (s + c));
      lr[0][i][1] = (float) (c / (s + c));
    /* ms_mode = 1 */
      lr[1][i][0] = (float) (sqrt(2.0) * (s / (s + c)));
      lr[1][i][1] = (float) (sqrt(2.0) * (c / (s + c)));
   }
/* sf = 7 */
/* ms_mode = 0 */
   lr[0][i][0] = 1.0f;
   lr[0][i][1] = 0.0f;
/* ms_mode = 1, in is bands is routine does ms processing */
   lr[1][i][0] = 1.0f;
   lr[1][i][1] = 1.0f;


/*-------
for(i=0;i<21;i++) m_nBand[0][i] = 
            sfBandTable[sr_index].l[i+1] - sfBandTable[sr_index].l[i];
for(i=0;i<12;i++) m_nBand[1][i] = 
            sfBandTable[sr_index].s[i+1] - sfBandTable[sr_index].s[i];
-------------*/
}
/*===============================================================*/
void msis_init2()
{
   int k, n;
   double t;
   int intensity_scale, ms_mode, sf, sflen;
   float ms_factor[2];


   ms_factor[0] = 1.0;
   ms_factor[1] = (float) sqrt(2.0);

/* intensity stereo MPEG2 */
/* lr2[intensity_scale][ms_mode][sflen_offset+sf][left/right] */

   for (intensity_scale = 0; intensity_scale < 2; intensity_scale++)
   {
      t = pow_test(2.0, -0.25 * (1 + intensity_scale));
      for (ms_mode = 0; ms_mode < 2; ms_mode++)
      {

	 n = 1;
	 k = 0;
	 for (sflen = 0; sflen < 6; sflen++)
	 {
	    for (sf = 0; sf < (n - 1); sf++, k++)
	    {
	       if (sf == 0)
	       {
		  lr2[intensity_scale][ms_mode][k][0] = ms_factor[ms_mode] * 1.0f;
		  lr2[intensity_scale][ms_mode][k][1] = ms_factor[ms_mode] * 1.0f;
	       }
	       else if ((sf & 1))
	       {
		  lr2[intensity_scale][ms_mode][k][0] =
		     (float) (ms_factor[ms_mode] * pow_test(t, (sf + 1) / 2));
		  lr2[intensity_scale][ms_mode][k][1] = ms_factor[ms_mode] * 1.0f;
	       }
	       else
	       {
		  lr2[intensity_scale][ms_mode][k][0] = ms_factor[ms_mode] * 1.0f;
		  lr2[intensity_scale][ms_mode][k][1] =
		     (float) (ms_factor[ms_mode] * pow_test(t, sf / 2));
	       }
	    }

	  /* illegal is_pos used to do ms processing */
	    if (ms_mode == 0)
	    {			/* ms_mode = 0 */
	       lr2[intensity_scale][ms_mode][k][0] = 1.0f;
	       lr2[intensity_scale][ms_mode][k][1] = 0.0f;
	    }
	    else
	    {
	     /* ms_mode = 1, in is bands is routine does ms processing */
	       lr2[intensity_scale][ms_mode][k][0] = 1.0f;
	       lr2[intensity_scale][ms_mode][k][1] = 1.0f;
	    }
	    k++;
	    n = n + n;
	 }
      }
   }
}

void msis_init()
{
	msis_init1();
	msis_init2();
}

/*===============================================================*/
void ms_process(float x[][1152], int n)		/* sum-difference stereo */
{
   int i;
   float xl, xr;

/*-- note: sqrt(2) done scaling by dequant ---*/
   for (i = 0; i < n; i++)
   {
      xl = x[0][i] + x[1][i];
      xr = x[0][i] - x[1][i];
      x[0][i] = xl;
      x[1][i] = xr;
   }
   return;
}

void is_process1(float x[][1152],	/* intensity stereo */
		      SCALE_FACTOR* sf,
		      CB_INFO cb_info[2],	/* [ch] */
		      int nsamp)
{
   int i, j, n, cb, w;
   float fl, fr;
   int m;
   int isf;
   float fls[3], frs[3];
   int cb0;


   cb0 = cb_info[1].cbmax;	/* start at end of right */
   i = m_sfBandIndex[cb_info[1].cbtype][cb0];
   cb0++;
   m = nsamp - i;		/* process to len of left */

   if (cb_info[1].cbtype)
      goto short_blocks;
/*------------------------*/
/* long_blocks: */
   for (cb = cb0; cb < 21; cb++)
   {
      isf = sf->l[cb];
      n = m_nBand[0][cb];
      fl = lr[m_ms_mode][isf][0];
      fr = lr[m_ms_mode][isf][1];
      for (j = 0; j < n; j++, i++)
      {
	 if (--m < 0)
	    goto exit;
	 x[1][i] = fr * x[0][i];
	 x[0][i] = fl * x[0][i];
      }
   }
   return;
/*------------------------*/
 short_blocks:
   for (cb = cb0; cb < 12; cb++)
   {
      for (w = 0; w < 3; w++)
      {
	 isf = sf->s[w][cb];
	 fls[w] = lr[m_ms_mode][isf][0];
	 frs[w] = lr[m_ms_mode][isf][1];
      }
      n = m_nBand[1][cb];
      for (j = 0; j < n; j++)
      {
	 m -= 3;
	 if (m < 0)
	    goto exit;
	 x[1][i] = frs[0] * x[0][i];
	 x[0][i] = fls[0] * x[0][i];
	 x[1][1 + i] = frs[1] * x[0][1 + i];
	 x[0][1 + i] = fls[1] * x[0][1 + i];
	 x[1][2 + i] = frs[2] * x[0][2 + i];
	 x[0][2 + i] = fls[2] * x[0][2 + i];
	 i += 3;
      }
   }

 exit:
   return;
}

typedef float ARRAY2[2];

void is_process2(float x[][1152],	/* intensity stereo */
		      SCALE_FACTOR* sf,
		      CB_INFO cb_info[2],	/* [ch] */
		      int nsamp)
{
   int i, j, k, n, cb, w;
   float fl, fr;
   int m;
   int isf;
   int il[21];
   int tmp;
   int r;
   ARRAY2 *lr;
   int cb0, cb1;

   lr = lr2[m_is_sf_info.intensity_scale][m_ms_mode];

   if (cb_info[1].cbtype)
      goto short_blocks;

/*------------------------*/
/* long_blocks: */
   cb0 = cb_info[1].cbmax;	/* start at end of right */
   i = m_sfBandIndex[0][cb0];
   m = nsamp - i;		/* process to len of left */
/* gen sf info */
   for (k = r = 0; r < 3; r++)
   {
      tmp = (1 << m_is_sf_info.slen[r]) - 1;
      for (j = 0; j < m_is_sf_info.nr[r]; j++, k++)
	 il[k] = tmp;
   }
   for (cb = cb0 + 1; cb < 21; cb++)
   {
      isf = il[cb] + sf->l[cb];
	  // X-MaD 27-02-02	
	  if (isf < 0) { isf = 0; }
	  fl = 0;
	  fr = 0;
		  
	  //try 
	  //{		
		fl = lr[isf][0];
		fr = lr[isf][1];
		n = m_nBand[0][cb];
	  //} 
	  //catch(...) 
	  //{ 
	  //	  isf = 0; 
	  //}
	  // X-MaD 27-02-02
     
	  for (j = 0; j < n; j++, i++)
      {
		  if (--m < 0) 
		  {
			  goto exit; 
		  }
		x[1][i] = fr * x[0][i];
		x[0][i] = fl * x[0][i];
      }
   }
   return;
/*------------------------*/
 short_blocks:

   for (k = r = 0; r < 3; r++)
   {
      tmp = (1 << m_is_sf_info.slen[r]) - 1;
      for (j = 0; j < m_is_sf_info.nr[r]; j++, k++)
	 il[k] = tmp;
   }

   for (w = 0; w < 3; w++)
   {
      cb0 = cb_info[1].cbmax_s[w];	/* start at end of right */
      i = m_sfBandIndex[1][cb0] + w;
      cb1 = cb_info[0].cbmax_s[w];	/* process to end of left */

      for (cb = cb0 + 1; cb <= cb1; cb++)
      {
	 isf = il[cb] + sf->s[w][cb];
	 fl = lr[isf][0];
	 fr = lr[isf][1];
	 n = m_nBand[1][cb];
	 for (j = 0; j < n; j++)
	 {
	    x[1][i] = fr * x[0][i];
	    x[0][i] = fl * x[0][i];
	    i += 3;
	 }
      }

   }

 exit:
   return;
}
