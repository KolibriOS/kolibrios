// disable precision loss warning on type conversion
#ifdef _MSC_VER
#pragma warning(disable:4244 4056)
#endif

float wincoef[264] =
{				// window coefs
0.000000000f, 0.000442505f, -0.003250122f, 0.007003784f,
-0.031082151f, 0.078628540f, -0.100311279f, 0.572036743f,
-1.144989014f, -0.572036743f, -0.100311279f, -0.078628540f,
-0.031082151f, -0.007003784f, -0.003250122f, -0.000442505f,
0.000015259f, 0.000473022f, -0.003326416f, 0.007919312f,
-0.030517576f, 0.084182739f, -0.090927124f, 0.600219727f,
-1.144287109f, -0.543823242f, -0.108856201f, -0.073059082f,
-0.031478882f, -0.006118774f, -0.003173828f, -0.000396729f,
0.000015259f, 0.000534058f, -0.003387451f, 0.008865356f,
-0.029785154f, 0.089706421f, -0.080688477f, 0.628295898f,
-1.142211914f, -0.515609741f, -0.116577141f, -0.067520142f,
-0.031738281f, -0.005294800f, -0.003082275f, -0.000366211f,
0.000015259f, 0.000579834f, -0.003433228f, 0.009841919f,
-0.028884888f, 0.095169067f, -0.069595337f, 0.656219482f,
-1.138763428f, -0.487472534f, -0.123474121f, -0.061996460f,
-0.031845093f, -0.004486084f, -0.002990723f, -0.000320435f,
0.000015259f, 0.000625610f, -0.003463745f, 0.010848999f,
-0.027801514f, 0.100540161f, -0.057617184f, 0.683914185f,
-1.133926392f, -0.459472656f, -0.129577637f, -0.056533810f,
-0.031814575f, -0.003723145f, -0.002899170f, -0.000289917f,
0.000015259f, 0.000686646f, -0.003479004f, 0.011886597f,
-0.026535034f, 0.105819702f, -0.044784546f, 0.711318970f,
-1.127746582f, -0.431655884f, -0.134887695f, -0.051132202f,
-0.031661987f, -0.003005981f, -0.002792358f, -0.000259399f,
0.000015259f, 0.000747681f, -0.003479004f, 0.012939452f,
-0.025085449f, 0.110946655f, -0.031082151f, 0.738372803f,
-1.120223999f, -0.404083252f, -0.139450073f, -0.045837402f,
-0.031387329f, -0.002334595f, -0.002685547f, -0.000244141f,
0.000030518f, 0.000808716f, -0.003463745f, 0.014022826f,
-0.023422241f, 0.115921021f, -0.016510010f, 0.765029907f,
-1.111373901f, -0.376800537f, -0.143264771f, -0.040634155f,
-0.031005858f, -0.001693726f, -0.002578735f, -0.000213623f,
0.000030518f, 0.000885010f, -0.003417969f, 0.015121460f,
-0.021575928f, 0.120697014f, -0.001068115f, 0.791213989f,
-1.101211548f, -0.349868774f, -0.146362305f, -0.035552979f,
-0.030532837f, -0.001098633f, -0.002456665f, -0.000198364f,
0.000030518f, 0.000961304f, -0.003372192f, 0.016235352f,
-0.019531250f, 0.125259399f, 0.015228271f, 0.816864014f,
-1.089782715f, -0.323318481f, -0.148773193f, -0.030609131f,
-0.029937742f, -0.000549316f, -0.002349854f, -0.000167847f,
0.000030518f, 0.001037598f, -0.003280640f, 0.017349243f,
-0.017257690f, 0.129562378f, 0.032379150f, 0.841949463f,
-1.077117920f, -0.297210693f, -0.150497437f, -0.025817871f,
-0.029281614f, -0.000030518f, -0.002243042f, -0.000152588f,
0.000045776f, 0.001113892f, -0.003173828f, 0.018463135f,
-0.014801024f, 0.133590698f, 0.050354004f, 0.866363525f,
-1.063217163f, -0.271591187f, -0.151596069f, -0.021179199f,
-0.028533936f, 0.000442505f, -0.002120972f, -0.000137329f,
0.000045776f, 0.001205444f, -0.003051758f, 0.019577026f,
-0.012115479f, 0.137298584f, 0.069168091f, 0.890090942f,
-1.048156738f, -0.246505737f, -0.152069092f, -0.016708374f,
-0.027725220f, 0.000869751f, -0.002014160f, -0.000122070f,
0.000061035f, 0.001296997f, -0.002883911f, 0.020690918f,
-0.009231566f, 0.140670776f, 0.088775635f, 0.913055420f,
-1.031936646f, -0.221984863f, -0.151962280f, -0.012420653f,
-0.026840210f, 0.001266479f, -0.001907349f, -0.000106812f,
0.000061035f, 0.001388550f, -0.002700806f, 0.021789551f,
-0.006134033f, 0.143676758f, 0.109161377f, 0.935195923f,
-1.014617920f, -0.198059082f, -0.151306152f, -0.008316040f,
-0.025909424f, 0.001617432f, -0.001785278f, -0.000106812f,
0.000076294f, 0.001480103f, -0.002487183f, 0.022857666f,
-0.002822876f, 0.146255493f, 0.130310059f, 0.956481934f,
-0.996246338f, -0.174789429f, -0.150115967f, -0.004394531f,
-0.024932859f, 0.001937866f, -0.001693726f, -0.000091553f,
-0.001586914f, -0.023910521f, -0.148422241f, -0.976852417f,
0.152206421f, 0.000686646f, -0.002227783f, 0.000076294f,
};

void window(float *vbuf, int vb_ptr, short *pcm)
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
      *pcm++ = tmp;
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
   *pcm++ = tmp;
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
      *pcm++ = tmp;
   }
}



/*------------------------------------------------------------*/
void window_dual(float *vbuf, int vb_ptr, short *pcm)
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
      *pcm = tmp;
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
   *pcm = tmp;
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
      *pcm = tmp;
      pcm += 2;
   }
}
/*------------------------------------------------------------*/
/*------------------- 16 pt window ------------------------------*/
void window16(float *vbuf, int vb_ptr, short *pcm)
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
      *pcm++ = tmp;
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
   *pcm++ = tmp;
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
      *pcm++ = tmp;
   }
}
/*--------------- 16 pt dual window (interleaved output) -----------------*/
void window16_dual(float *vbuf, int vb_ptr, short *pcm)
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
      *pcm = tmp;
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
   *pcm = tmp;
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
      *pcm = tmp;
      pcm += 2;
   }
}
/*------------------- 8 pt window ------------------------------*/
void window8(float *vbuf, int vb_ptr, short *pcm)
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
      *pcm++ = tmp;
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
   *pcm++ = tmp;
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
      *pcm++ = tmp;
   }
}
/*--------------- 8 pt dual window (interleaved output) -----------------*/
void window8_dual(float *vbuf, int vb_ptr, short *pcm)
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
      *pcm = tmp;
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
   *pcm = tmp;
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
      *pcm = tmp;
      pcm += 2;
   }
}
