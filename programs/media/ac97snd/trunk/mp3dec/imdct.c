#include <math.h>

/*------ 18 point xform -------*/
static float w[18];
static float w2[9];
static float coef[9][4];

static float v[6];
static float v2[3];
static float coef87;
/*
typedef struct
{
   float *w;
   float *w2;
   void *coef;
}
IMDCT_INIT_BLOCK;

static IMDCT_INIT_BLOCK imdct_info_18 =
{w, w2, coef};
static IMDCT_INIT_BLOCK imdct_info_6 =
{v, v2, &coef87};
*/
/*=============================================================*/
void imdct_init()
{
   int k, p, n;
   double t, pi;
//   IMDCT_INIT_BLOCK *addr;
//   float *w, *w2;
//   float *v, *v2, *coef87;

/*--- 18 point --*/
//   addr = imdct_init_addr_18();
//   w = addr->w;
//   w2 = addr->w2;
//   coef = addr->coef;
/*----*/
   n = 18;
   pi = 4.0 * atan(1.0);
   t = pi / (4 * n);
   for (p = 0; p < n; p++)
      w[p] = (float) (2.0 * cos(t * (2 * p + 1)));
   for (p = 0; p < 9; p++)
      w2[p] = (float) (2.0 *cos(2 * t * (2 * p + 1)));

   t = pi / (2 * n);
   for (k = 0; k < 9; k++)
   {
      for (p = 0; p < 4; p++)
	 coef[k][p] = (float) (cos(t * (2 * k) * (2 * p + 1)));
   }

/*--- 6 point */
//   addr = imdct_init_addr_6();
//   v = addr->w;
//   v2 = addr->w2;
//   coef87 = addr->coef;
/*----*/
   n = 6;
   pi = 4.0 * atan(1.0);
   t = pi / (4 * n);
   for (p = 0; p < n; p++)
      v[p] = (float) (2.0 *cos(t * (2 * p + 1)));

   for (p = 0; p < 3; p++)
      v2[p] = (float) (2.0 *cos(2 * t * (2 * p + 1)));

   t = pi / (2 * n);
   k = 1;
   p = 0;
   coef87 = (float) (cos(t * (2 * k) * (2 * p + 1)));
/* adjust scaling to save a few mults */
   for (p = 0; p < 6; p++)
      v[p] = v[p] / 2.0f;
   coef87 = (float) (2.0 * coef87);
}
/*--------------------------------------------------------------------*/
void imdct18(float f[18])	/* 18 point */
{
   int p;
   float a[9], b[9];
   float ap, bp, a8p, b8p;
   float g1, g2;


   for (p = 0; p < 4; p++)
   {
      g1 = w[p] * f[p];
      g2 = w[17 - p] * f[17 - p];
      ap = g1 + g2;		// a[p]

      bp = w2[p] * (g1 - g2);	// b[p]

      g1 = w[8 - p] * f[8 - p];
      g2 = w[9 + p] * f[9 + p];
      a8p = g1 + g2;		// a[8-p]

      b8p = w2[8 - p] * (g1 - g2);	// b[8-p]

      a[p] = ap + a8p;
      a[5 + p] = ap - a8p;
      b[p] = bp + b8p;
      b[5 + p] = bp - b8p;
   }
   g1 = w[p] * f[p];
   g2 = w[17 - p] * f[17 - p];
   a[p] = g1 + g2;
   b[p] = w2[p] * (g1 - g2);


   f[0] = 0.5f * (a[0] + a[1] + a[2] + a[3] + a[4]);
   f[1] = 0.5f * (b[0] + b[1] + b[2] + b[3] + b[4]);

   f[2] = coef[1][0] * a[5] + coef[1][1] * a[6] + coef[1][2] * a[7]
      + coef[1][3] * a[8];
   f[3] = coef[1][0] * b[5] + coef[1][1] * b[6] + coef[1][2] * b[7]
      + coef[1][3] * b[8] - f[1];
   f[1] = f[1] - f[0];
   f[2] = f[2] - f[1];

   f[4] = coef[2][0] * a[0] + coef[2][1] * a[1] + coef[2][2] * a[2]
      + coef[2][3] * a[3] - a[4];
   f[5] = coef[2][0] * b[0] + coef[2][1] * b[1] + coef[2][2] * b[2]
      + coef[2][3] * b[3] - b[4] - f[3];
   f[3] = f[3] - f[2];
   f[4] = f[4] - f[3];

   f[6] = coef[3][0] * (a[5] - a[7] - a[8]);
   f[7] = coef[3][0] * (b[5] - b[7] - b[8]) - f[5];
   f[5] = f[5] - f[4];
   f[6] = f[6] - f[5];

   f[8] = coef[4][0] * a[0] + coef[4][1] * a[1] + coef[4][2] * a[2]
      + coef[4][3] * a[3] + a[4];
   f[9] = coef[4][0] * b[0] + coef[4][1] * b[1] + coef[4][2] * b[2]
      + coef[4][3] * b[3] + b[4] - f[7];
   f[7] = f[7] - f[6];
   f[8] = f[8] - f[7];

   f[10] = coef[5][0] * a[5] + coef[5][1] * a[6] + coef[5][2] * a[7]
      + coef[5][3] * a[8];
   f[11] = coef[5][0] * b[5] + coef[5][1] * b[6] + coef[5][2] * b[7]
      + coef[5][3] * b[8] - f[9];
   f[9] = f[9] - f[8];
   f[10] = f[10] - f[9];

   f[12] = 0.5f * (a[0] + a[2] + a[3]) - a[1] - a[4];
   f[13] = 0.5f * (b[0] + b[2] + b[3]) - b[1] - b[4] - f[11];
   f[11] = f[11] - f[10];
   f[12] = f[12] - f[11];

   f[14] = coef[7][0] * a[5] + coef[7][1] * a[6] + coef[7][2] * a[7]
      + coef[7][3] * a[8];
   f[15] = coef[7][0] * b[5] + coef[7][1] * b[6] + coef[7][2] * b[7]
      + coef[7][3] * b[8] - f[13];
   f[13] = f[13] - f[12];
   f[14] = f[14] - f[13];

   f[16] = coef[8][0] * a[0] + coef[8][1] * a[1] + coef[8][2] * a[2]
      + coef[8][3] * a[3] + a[4];
   f[17] = coef[8][0] * b[0] + coef[8][1] * b[1] + coef[8][2] * b[2]
      + coef[8][3] * b[3] + b[4] - f[15];
   f[15] = f[15] - f[14];
   f[16] = f[16] - f[15];
   f[17] = f[17] - f[16];
}
/*--------------------------------------------------------------------*/
/* does 3, 6 pt dct.  changes order from f[i][window] c[window][i] */
void imdct6_3(float f[])	/* 6 point */
{
   int w;
   float buf[18];
   float *a, *c;		// b[i] = a[3+i]

   float g1, g2;
   float a02, b02;

   c = f;
   a = buf;
   for (w = 0; w < 3; w++)
   {
      g1 = v[0] * f[3 * 0];
      g2 = v[5] * f[3 * 5];
      a[0] = g1 + g2;
      a[3 + 0] = v2[0] * (g1 - g2);

      g1 = v[1] * f[3 * 1];
      g2 = v[4] * f[3 * 4];
      a[1] = g1 + g2;
      a[3 + 1] = v2[1] * (g1 - g2);

      g1 = v[2] * f[3 * 2];
      g2 = v[3] * f[3 * 3];
      a[2] = g1 + g2;
      a[3 + 2] = v2[2] * (g1 - g2);

      a += 6;
      f++;
   }

   a = buf;
   for (w = 0; w < 3; w++)
   {
      a02 = (a[0] + a[2]);
      b02 = (a[3 + 0] + a[3 + 2]);
      c[0] = a02 + a[1];
      c[1] = b02 + a[3 + 1];
      c[2] = coef87 * (a[0] - a[2]);
      c[3] = coef87 * (a[3 + 0] - a[3 + 2]) - c[1];
      c[1] = c[1] - c[0];
      c[2] = c[2] - c[1];
      c[4] = a02 - a[1] - a[1];
      c[5] = b02 - a[3 + 1] - a[3 + 1] - c[3];
      c[3] = c[3] - c[2];
      c[4] = c[4] - c[3];
      c[5] = c[5] - c[4];
      a += 6;
      c += 6;
   }
}
