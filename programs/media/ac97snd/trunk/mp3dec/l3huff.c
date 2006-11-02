#include "layer3.h"
#include "l3huff.h"

//#ifdef _MSC_VER
//#pragma warning(disable: 4505)
//#endif

/*===============================================================*/

/* max bits required for any lookup - change if htable changes */
/* quad required 10 bit w/signs  must have (MAXBITS+2) >= 10   */
#define MAXBITS 9

static HUFF_ELEMENT huff_table_0[] =
{0, 0, 0, 64};			/* dummy must not use */

/*-- 6 bit lookup (purgebits, value) --*/
static unsigned char quad_table_a[][2] =
{
   6, 11, 6, 15, 6, 13, 6, 14, 6, 7, 6, 5, 5, 9,
   5, 9, 5, 6, 5, 6, 5, 3, 5, 3, 5, 10, 5, 10,
   5, 12, 5, 12, 4, 2, 4, 2, 4, 2, 4, 2, 4, 1,
   4, 1, 4, 1, 4, 1, 4, 4, 4, 4, 4, 4, 4, 4,
   4, 8, 4, 8, 4, 8, 4, 8, 1, 0, 1, 0, 1, 0,
   1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
   1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
   1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
   1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
   1, 0,
};


typedef struct
{
   HUFF_ELEMENT *table;
   int linbits;
   int ncase;
}
HUFF_SETUP;

#define no_bits       0
#define one_shot      1
#define no_linbits    2
#define have_linbits  3
#define quad_a        4
#define quad_b        5


static HUFF_SETUP table_look[] =
{
   huff_table_0, 0, no_bits,
   huff_table_1, 0, one_shot,
   huff_table_2, 0, one_shot,
   huff_table_3, 0, one_shot,
   huff_table_0, 0, no_bits,
   huff_table_5, 0, one_shot,
   huff_table_6, 0, one_shot,
   huff_table_7, 0, no_linbits,
   huff_table_8, 0, no_linbits,
   huff_table_9, 0, no_linbits,
   huff_table_10, 0, no_linbits,
   huff_table_11, 0, no_linbits,
   huff_table_12, 0, no_linbits,
   huff_table_13, 0, no_linbits,
   huff_table_0, 0, no_bits,
   huff_table_15, 0, no_linbits,
   huff_table_16, 1, have_linbits,
   huff_table_16, 2, have_linbits,
   huff_table_16, 3, have_linbits,
   huff_table_16, 4, have_linbits,
   huff_table_16, 6, have_linbits,
   huff_table_16, 8, have_linbits,
   huff_table_16, 10, have_linbits,
   huff_table_16, 13, have_linbits,
   huff_table_24, 4, have_linbits,
   huff_table_24, 5, have_linbits,
   huff_table_24, 6, have_linbits,
   huff_table_24, 7, have_linbits,
   huff_table_24, 8, have_linbits,
   huff_table_24, 9, have_linbits,
   huff_table_24, 11, have_linbits,
   huff_table_24, 13, have_linbits,
   huff_table_0, 0, quad_a,
   huff_table_0, 0, quad_b,
};

/*========================================================*/
void huffman(int xy[][2], int n, int ntable)
{
   int i;
   HUFF_ELEMENT *t;
   HUFF_ELEMENT *t0;
   int linbits;
   int bits;
   int code;
   int x, y;

   if (n <= 0)
      return;
   n = n >> 1;			/* huff in pairs */
/*-------------*/
   t0 = table_look[ntable].table;
   linbits = table_look[ntable].linbits;
   switch (table_look[ntable].ncase)
   {
      default:
/*------------------------------------------*/
      case no_bits:
/*- table 0, no data, x=y=0--*/
	 for (i = 0; i < n; i++)
	 {
	    xy[i][0] = 0;
	    xy[i][1] = 0;
	 }
	 return;
/*------------------------------------------*/
      case one_shot:
/*- single lookup, no escapes -*/
	 for (i = 0; i < n; i++)
	 {
	    mac_bitget_check((MAXBITS + 2));
	    bits = t0[0].b.signbits;
	    code = mac_bitget2(bits);
	    mac_bitget_purge(t0[1 + code].b.purgebits);
	    x = t0[1 + code].b.x;
	    y = t0[1 + code].b.y;
	    if (x)
	       if (mac_bitget_1bit())
		  x = -x;
	    if (y)
	       if (mac_bitget_1bit())
		  y = -y;
	    xy[i][0] = x;
	    xy[i][1] = y;
	    if (bitget_overrun())
	       break;		// bad data protect

	 }
	 return;
/*------------------------------------------*/
      case no_linbits:
	 for (i = 0; i < n; i++)
	 {
	    t = t0;
	    for (;;)
	    {
	       mac_bitget_check((MAXBITS + 2));
	       bits = t[0].b.signbits;
	       code = mac_bitget2(bits);
	       if (t[1 + code].b.purgebits)
		  break;
	       t += t[1 + code].ptr;	/* ptr include 1+code */
	       mac_bitget_purge(bits);
	    }
	    mac_bitget_purge(t[1 + code].b.purgebits);
	    x = t[1 + code].b.x;
	    y = t[1 + code].b.y;
	    if (x)
	       if (mac_bitget_1bit())
		  x = -x;
	    if (y)
	       if (mac_bitget_1bit())
		  y = -y;
	    xy[i][0] = x;
	    xy[i][1] = y;
	    if (bitget_overrun())
	       break;		// bad data protect

	 }
	 return;
/*------------------------------------------*/
      case have_linbits:
	 for (i = 0; i < n; i++)
	 {
	    t = t0;
	    for (;;)
	    {
	       bits = t[0].b.signbits;
	       code = bitget2(bits);
	       if (t[1 + code].b.purgebits)
		  break;
	       t += t[1 + code].ptr;	/* ptr includes 1+code */
	       mac_bitget_purge(bits);
	    }
	    mac_bitget_purge(t[1 + code].b.purgebits);
	    x = t[1 + code].b.x;
	    y = t[1 + code].b.y;
	    if (x == 15)
	       x += bitget_lb(linbits);
	    if (x)
	       if (mac_bitget_1bit())
		  x = -x;
	    if (y == 15)
	       y += bitget_lb(linbits);
	    if (y)
	       if (mac_bitget_1bit())
		  y = -y;
	    xy[i][0] = x;
	    xy[i][1] = y;
	    if (bitget_overrun())
	       break;		// bad data protect

	 }
	 return;
   }
/*--- end switch ---*/

}

int huffman_quad(int vwxy[][4], int n, int nbits, int ntable)
{
   int i;
   int code;
   int x, y, v, w;
   int tmp;
   int i_non_zero, tmp_nz;

   tmp_nz = 15;
   i_non_zero = -1;

   n = n >> 2;			/* huff in quads */

   if (ntable)
      goto case_quad_b;

/* case_quad_a: */
   for (i = 0; i < n; i++)
   {
      if (nbits <= 0)
	 break;
      mac_bitget_check(10);
      code = mac_bitget2(6);
      nbits -= quad_table_a[code][0];
      mac_bitget_purge(quad_table_a[code][0]);
      tmp = quad_table_a[code][1];
      if (tmp)
      {
	 i_non_zero = i;
	 tmp_nz = tmp;
      }
      v = (tmp >> 3) & 1;
      w = (tmp >> 2) & 1;
      x = (tmp >> 1) & 1;
      y = tmp & 1;
      if (v)
      {
	 if (mac_bitget_1bit())
	    v = -v;
	 nbits--;
      }
      if (w)
      {
	 if (mac_bitget_1bit())
	    w = -w;
	 nbits--;
      }
      if (x)
      {
	 if (mac_bitget_1bit())
	    x = -x;
	 nbits--;
      }
      if (y)
      {
	 if (mac_bitget_1bit())
	    y = -y;
	 nbits--;
      }
      vwxy[i][0] = v;
      vwxy[i][1] = w;
      vwxy[i][2] = x;
      vwxy[i][3] = y;
      if (bitget_overrun())
	 break;			// bad data protect

   }
   if (nbits < 0)
   {
      i--;
      vwxy[i][0] = 0;
      vwxy[i][1] = 0;
      vwxy[i][2] = 0;
      vwxy[i][3] = 0;
   }

   i_non_zero = (i_non_zero + 1) << 2;

   if ((tmp_nz & 3) == 0)
      i_non_zero -= 2;

   return i_non_zero;

/*--------------------*/
 case_quad_b:
   for (i = 0; i < n; i++)
   {
      if (nbits < 4)
	 break;
      nbits -= 4;
      mac_bitget_check(8);
      tmp = mac_bitget(4) ^ 15;	/* one's complement of bitstream */
      if (tmp)
      {
	 i_non_zero = i;
	 tmp_nz = tmp;
      }
      v = (tmp >> 3) & 1;
      w = (tmp >> 2) & 1;
      x = (tmp >> 1) & 1;
      y = tmp & 1;
      if (v)
      {
	 if (mac_bitget_1bit())
	    v = -v;
	 nbits--;
      }
      if (w)
      {
	 if (mac_bitget_1bit())
	    w = -w;
	 nbits--;
      }
      if (x)
      {
	 if (mac_bitget_1bit())
	    x = -x;
	 nbits--;
      }
      if (y)
      {
	 if (mac_bitget_1bit())
	    y = -y;
	 nbits--;
      }
      vwxy[i][0] = v;
      vwxy[i][1] = w;
      vwxy[i][2] = x;
      vwxy[i][3] = y;
      if (bitget_overrun())
	 break;			// bad data protect

   }
   if (nbits < 0)
   {
      i--;
      vwxy[i][0] = 0;
      vwxy[i][1] = 0;
      vwxy[i][2] = 0;
      vwxy[i][3] = 0;
   }

   i_non_zero = (i_non_zero + 1) << 2;

   if ((tmp_nz & 3) == 0)
      i_non_zero -= 2;

   return i_non_zero;		/* return non-zero sample (to nearest pair) */

}
