extern float wincoef[264];

void windowB(float *vbuf, int vb_ptr, unsigned char *pcm)
{
   int i, j;
   int si, bx;
   float *coef;
   float sum;
   long tmp;

   si = vb_ptr + 16;
   bx = (si + 32) & 511;
   coef = wincoef;

/*-- first 16 --*/
   for (i = 0; i < 16; i++)
   {
      sum = 0.0F;
      for (j = 0; j < 8; j++)
      {
	 sum += (*coef++) * vbuf[si];
	 si = (si + 64) & 511;
	 sum -= (*coef++) * vbuf[bx];
	 bx = (bx + 64) & 511;
      }
      si++;
      bx--;
      tmp = (long) sum;
      if (tmp > 32767)
	 tmp = 32767;
      else if (tmp < -32768)
	 tmp = -32768;
      *pcm++ = ((unsigned char) (tmp >> 8)) ^ 0x80;
   }
/*--  special case --*/
   sum = 0.0F;
   for (j = 0; j < 8; j++)
   {
      sum += (*coef++) * vbuf[bx];
      bx = (bx + 64) & 511;
   }
   tmp = (long) sum;
   if (tmp > 32767)
      tmp = 32767;
   else if (tmp < -32768)
      tmp = -32768;
   *pcm++ = ((unsigned char) (tmp >> 8)) ^ 0x80;
/*-- last 15 --*/
   coef = wincoef + 255;	/* back pass through coefs */
   for (i = 0; i < 15; i++)
   {
      si--;
      bx++;
      sum = 0.0F;
      for (j = 0; j < 8; j++)
      {
	 sum += (*coef--) * vbuf[si];
	 si = (si + 64) & 511;
	 sum += (*coef--) * vbuf[bx];
	 bx = (bx + 64) & 511;
      }
      tmp = (long) sum;
      if (tmp > 32767)
	 tmp = 32767;
      else if (tmp < -32768)
	 tmp = -32768;
      *pcm++ = ((unsigned char) (tmp >> 8)) ^ 0x80;
   }
}
/*------------------------------------------------------------*/
void windowB_dual(float *vbuf, int vb_ptr, unsigned char *pcm)
{
   int i, j;			/* dual window interleaves output */
   int si, bx;
   float *coef;
   float sum;
   long tmp;

   si = vb_ptr + 16;
   bx = (si + 32) & 511;
   coef = wincoef;

/*-- first 16 --*/
   for (i = 0; i < 16; i++)
   {
      sum = 0.0F;
      for (j = 0; j < 8; j++)
      {
	 sum += (*coef++) * vbuf[si];
	 si = (si + 64) & 511;
	 sum -= (*coef++) * vbuf[bx];
	 bx = (bx + 64) & 511;
      }
      si++;
      bx--;
      tmp = (long) sum;
      if (tmp > 32767)
	 tmp = 32767;
      else if (tmp < -32768)
	 tmp = -32768;
      *pcm = ((unsigned char) (tmp >> 8)) ^ 0x80;
      pcm += 2;
   }
/*--  special case --*/
   sum = 0.0F;
   for (j = 0; j < 8; j++)
   {
      sum += (*coef++) * vbuf[bx];
      bx = (bx + 64) & 511;
   }
   tmp = (long) sum;
   if (tmp > 32767)
      tmp = 32767;
   else if (tmp < -32768)
      tmp = -32768;
   *pcm = ((unsigned char) (tmp >> 8)) ^ 0x80;
   pcm += 2;
/*-- last 15 --*/
   coef = wincoef + 255;	/* back pass through coefs */
   for (i = 0; i < 15; i++)
   {
      si--;
      bx++;
      sum = 0.0F;
      for (j = 0; j < 8; j++)
      {
	 sum += (*coef--) * vbuf[si];
	 si = (si + 64) & 511;
	 sum += (*coef--) * vbuf[bx];
	 bx = (bx + 64) & 511;
      }
      tmp = (long) sum;
      if (tmp > 32767)
	 tmp = 32767;
      else if (tmp < -32768)
	 tmp = -32768;
      *pcm = ((unsigned char) (tmp >> 8)) ^ 0x80;
      pcm += 2;
   }
}
/*------------------------------------------------------------*/
/*------------------- 16 pt window ------------------------------*/
void windowB16(float *vbuf, int vb_ptr, unsigned char *pcm)
{
   int i, j;
   unsigned char si, bx;
   float *coef;
   float sum;
   long tmp;

   si = vb_ptr + 8;
   bx = si + 16;
   coef = wincoef;

/*-- first 8 --*/
   for (i = 0; i < 8; i++)
   {
      sum = 0.0F;
      for (j = 0; j < 8; j++)
      {
	 sum += (*coef++) * vbuf[si];
	 si += 32;
	 sum -= (*coef++) * vbuf[bx];
	 bx += 32;
      }
      si++;
      bx--;
      coef += 16;
      tmp = (long) sum;
      if (tmp > 32767)
	 tmp = 32767;
      else if (tmp < -32768)
	 tmp = -32768;
      *pcm++ = ((unsigned char) (tmp >> 8)) ^ 0x80;
   }
/*--  special case --*/
   sum = 0.0F;
   for (j = 0; j < 8; j++)
   {
      sum += (*coef++) * vbuf[bx];
      bx += 32;
   }
   tmp = (long) sum;
   if (tmp > 32767)
      tmp = 32767;
   else if (tmp < -32768)
      tmp = -32768;
   *pcm++ = ((unsigned char) (tmp >> 8)) ^ 0x80;
/*-- last 7 --*/
   coef = wincoef + 255;	/* back pass through coefs */
   for (i = 0; i < 7; i++)
   {
      coef -= 16;
      si--;
      bx++;
      sum = 0.0F;
      for (j = 0; j < 8; j++)
      {
	 sum += (*coef--) * vbuf[si];
	 si += 32;
	 sum += (*coef--) * vbuf[bx];
	 bx += 32;
      }
      tmp = (long) sum;
      if (tmp > 32767)
	 tmp = 32767;
      else if (tmp < -32768)
	 tmp = -32768;
      *pcm++ = ((unsigned char) (tmp >> 8)) ^ 0x80;
   }
}
/*--------------- 16 pt dual window (interleaved output) -----------------*/
void windowB16_dual(float *vbuf, int vb_ptr, unsigned char *pcm)
{
   int i, j;
   unsigned char si, bx;
   float *coef;
   float sum;
   long tmp;

   si = vb_ptr + 8;
   bx = si + 16;
   coef = wincoef;

/*-- first 8 --*/
   for (i = 0; i < 8; i++)
   {
      sum = 0.0F;
      for (j = 0; j < 8; j++)
      {
	 sum += (*coef++) * vbuf[si];
	 si += 32;
	 sum -= (*coef++) * vbuf[bx];
	 bx += 32;
      }
      si++;
      bx--;
      coef += 16;
      tmp = (long) sum;
      if (tmp > 32767)
	 tmp = 32767;
      else if (tmp < -32768)
	 tmp = -32768;
      *pcm = ((unsigned char) (tmp >> 8)) ^ 0x80;
      pcm += 2;
   }
/*--  special case --*/
   sum = 0.0F;
   for (j = 0; j < 8; j++)
   {
      sum += (*coef++) * vbuf[bx];
      bx += 32;
   }
   tmp = (long) sum;
   if (tmp > 32767)
      tmp = 32767;
   else if (tmp < -32768)
      tmp = -32768;
   *pcm = ((unsigned char) (tmp >> 8)) ^ 0x80;
   pcm += 2;
/*-- last 7 --*/
   coef = wincoef + 255;	/* back pass through coefs */
   for (i = 0; i < 7; i++)
   {
      coef -= 16;
      si--;
      bx++;
      sum = 0.0F;
      for (j = 0; j < 8; j++)
      {
	 sum += (*coef--) * vbuf[si];
	 si += 32;
	 sum += (*coef--) * vbuf[bx];
	 bx += 32;
      }
      tmp = (long) sum;
      if (tmp > 32767)
	 tmp = 32767;
      else if (tmp < -32768)
	 tmp = -32768;
      *pcm = ((unsigned char) (tmp >> 8)) ^ 0x80;
      pcm += 2;
   }
}
/*------------------- 8 pt window ------------------------------*/
void windowB8(float *vbuf, int vb_ptr, unsigned char *pcm)
{
   int i, j;
   int si, bx;
   float *coef;
   float sum;
   long tmp;

   si = vb_ptr + 4;
   bx = (si + 8) & 127;
   coef = wincoef;

/*-- first 4 --*/
   for (i = 0; i < 4; i++)
   {
      sum = 0.0F;
      for (j = 0; j < 8; j++)
      {
	 sum += (*coef++) * vbuf[si];
	 si = (si + 16) & 127;
	 sum -= (*coef++) * vbuf[bx];
	 bx = (bx + 16) & 127;
      }
      si++;
      bx--;
      coef += 48;
      tmp = (long) sum;
      if (tmp > 32767)
	 tmp = 32767;
      else if (tmp < -32768)
	 tmp = -32768;
      *pcm++ = ((unsigned char) (tmp >> 8)) ^ 0x80;
   }
/*--  special case --*/
   sum = 0.0F;
   for (j = 0; j < 8; j++)
   {
      sum += (*coef++) * vbuf[bx];
      bx = (bx + 16) & 127;
   }
   tmp = (long) sum;
   if (tmp > 32767)
      tmp = 32767;
   else if (tmp < -32768)
      tmp = -32768;
   *pcm++ = ((unsigned char) (tmp >> 8)) ^ 0x80;
/*-- last 3 --*/
   coef = wincoef + 255;	/* back pass through coefs */
   for (i = 0; i < 3; i++)
   {
      coef -= 48;
      si--;
      bx++;
      sum = 0.0F;
      for (j = 0; j < 8; j++)
      {
	 sum += (*coef--) * vbuf[si];
	 si = (si + 16) & 127;
	 sum += (*coef--) * vbuf[bx];
	 bx = (bx + 16) & 127;
      }
      tmp = (long) sum;
      if (tmp > 32767)
	 tmp = 32767;
      else if (tmp < -32768)
	 tmp = -32768;
      *pcm++ = ((unsigned char) (tmp >> 8)) ^ 0x80;
   }
}
/*--------------- 8 pt dual window (interleaved output) -----------------*/
void windowB8_dual(float *vbuf, int vb_ptr, unsigned char *pcm)
{
   int i, j;
   int si, bx;
   float *coef;
   float sum;
   long tmp;

   si = vb_ptr + 4;
   bx = (si + 8) & 127;
   coef = wincoef;

/*-- first 4 --*/
   for (i = 0; i < 4; i++)
   {
      sum = 0.0F;
      for (j = 0; j < 8; j++)
      {
	 sum += (*coef++) * vbuf[si];
	 si = (si + 16) & 127;
	 sum -= (*coef++) * vbuf[bx];
	 bx = (bx + 16) & 127;
      }
      si++;
      bx--;
      coef += 48;
      tmp = (long) sum;
      if (tmp > 32767)
	 tmp = 32767;
      else if (tmp < -32768)
	 tmp = -32768;
      *pcm = ((unsigned char) (tmp >> 8)) ^ 0x80;
      pcm += 2;
   }
/*--  special case --*/
   sum = 0.0F;
   for (j = 0; j < 8; j++)
   {
      sum += (*coef++) * vbuf[bx];
      bx = (bx + 16) & 127;
   }
   tmp = (long) sum;
   if (tmp > 32767)
      tmp = 32767;
   else if (tmp < -32768)
      tmp = -32768;
   *pcm = ((unsigned char) (tmp >> 8)) ^ 0x80;
   pcm += 2;
/*-- last 3 --*/
   coef = wincoef + 255;	/* back pass through coefs */
   for (i = 0; i < 3; i++)
   {
      coef -= 48;
      si--;
      bx++;
      sum = 0.0F;
      for (j = 0; j < 8; j++)
      {
	 sum += (*coef--) * vbuf[si];
	 si = (si + 16) & 127;
	 sum += (*coef--) * vbuf[bx];
	 bx = (bx + 16) & 127;
      }
      tmp = (long) sum;
      if (tmp > 32767)
	 tmp = 32767;
      else if (tmp < -32768)
	 tmp = -32768;
      *pcm = ((unsigned char) (tmp >> 8)) ^ 0x80;
      pcm += 2;
   }
}
