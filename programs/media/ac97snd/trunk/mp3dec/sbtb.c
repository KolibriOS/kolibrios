#include <math.h>

/* "fdct.c" */
void fdct32(float *, float *);
void fdct32_dual(float *, float *);
void fdct32_dual_mono(float *, float *);
void fdct16(float *, float *);
void fdct16_dual(float *, float *);
void fdct16_dual_mono(float *, float *);
void fdct8(float *, float *);
void fdct8_dual(float *, float *);
void fdct8_dual_mono(float *, float *);

/* "windowb.c" */
void windowB(float *vbuf, int vb_ptr, unsigned char *pcm);
void windowB_dual(float *vbuf, int vb_ptr, unsigned char *pcm);
void windowB16(float *vbuf, int vb_ptr, unsigned char *pcm);
void windowB16_dual(float *vbuf, int vb_ptr, unsigned char *pcm);
void windowB8(float *vbuf, int vb_ptr, unsigned char *pcm);
void windowB8_dual(float *vbuf, int vb_ptr, unsigned char *pcm);

extern int vb_ptr;
extern int vb2_ptr;
extern float vbuf[512];
extern float vbuf2[512];

void sbtB_mono(float *sample, unsigned char *pcm, int n)
{
   int i;

   for (i = 0; i < n; i++)
   {
      fdct32(sample, vbuf + vb_ptr);
      windowB(vbuf, vb_ptr, pcm);
      sample += 64;
      vb_ptr = (vb_ptr - 32) & 511;
      pcm += 32;
   }

}

void sbtB_dual(float *sample, unsigned char *pcm, int n)
{
   int i;

   for (i = 0; i < n; i++)
   {
      fdct32_dual(sample, vbuf + vb_ptr);
      fdct32_dual(sample + 1, vbuf2 + vb_ptr);
      windowB_dual(vbuf, vb_ptr, pcm);
      windowB_dual(vbuf2, vb_ptr, pcm + 1);
      sample += 64;
      vb_ptr = (vb_ptr - 32) & 511;
      pcm += 64;
   }


}

void sbtB_dual_mono(float *sample, unsigned char *pcm, int n)
{
   int i;

   for (i = 0; i < n; i++)
   {
      fdct32_dual_mono(sample, vbuf + vb_ptr);
      windowB(vbuf, vb_ptr, pcm);
      sample += 64;
      vb_ptr = (vb_ptr - 32) & 511;
      pcm += 32;
   }

}

void sbtB_dual_left(float *sample, unsigned char *pcm, int n)
{
   int i;

   for (i = 0; i < n; i++)
   {
      fdct32_dual(sample, vbuf + vb_ptr);
      windowB(vbuf, vb_ptr, pcm);
      sample += 64;
      vb_ptr = (vb_ptr - 32) & 511;
      pcm += 32;
   }
}

void sbtB_dual_right(float *sample, unsigned char *pcm, int n)
{
   int i;

   sample++;			/* point to right chan */
   for (i = 0; i < n; i++)
   {
      fdct32_dual(sample, vbuf + vb_ptr);
      windowB(vbuf, vb_ptr, pcm);
      sample += 64;
      vb_ptr = (vb_ptr - 32) & 511;
      pcm += 32;
   }
}

void sbtB16_mono(float *sample, unsigned char *pcm, int n)
{
   int i;

   for (i = 0; i < n; i++)
   {
      fdct16(sample, vbuf + vb_ptr);
      windowB16(vbuf, vb_ptr, pcm);
      sample += 64;
      vb_ptr = (vb_ptr - 16) & 255;
      pcm += 16;
   }


}

void sbtB16_dual(float *sample, unsigned char *pcm, int n)
{
   int i;

   for (i = 0; i < n; i++)
   {
      fdct16_dual(sample, vbuf + vb_ptr);
      fdct16_dual(sample + 1, vbuf2 + vb_ptr);
      windowB16_dual(vbuf, vb_ptr, pcm);
      windowB16_dual(vbuf2, vb_ptr, pcm + 1);
      sample += 64;
      vb_ptr = (vb_ptr - 16) & 255;
      pcm += 32;
   }
}

void sbtB16_dual_mono(float *sample, unsigned char *pcm, int n)
{
   int i;

   for (i = 0; i < n; i++)
   {
      fdct16_dual_mono(sample, vbuf + vb_ptr);
      windowB16(vbuf, vb_ptr, pcm);
      sample += 64;
      vb_ptr = (vb_ptr - 16) & 255;
      pcm += 16;
   }
}

void sbtB16_dual_left(float *sample, unsigned char *pcm, int n)
{
   int i;

   for (i = 0; i < n; i++)
   {
      fdct16_dual(sample, vbuf + vb_ptr);
      windowB16(vbuf, vb_ptr, pcm);
      sample += 64;
      vb_ptr = (vb_ptr - 16) & 255;
      pcm += 16;
   }
}

void sbtB16_dual_right(float *sample, unsigned char *pcm, int n)
{
   int i;

   sample++;
   for (i = 0; i < n; i++)
   {
      fdct16_dual(sample, vbuf + vb_ptr);
      windowB16(vbuf, vb_ptr, pcm);
      sample += 64;
      vb_ptr = (vb_ptr - 16) & 255;
      pcm += 16;
   }
}

void sbtB8_mono(float *sample, unsigned char *pcm, int n)
{
   int i;

   for (i = 0; i < n; i++)
   {
      fdct8(sample, vbuf + vb_ptr);
      windowB8(vbuf, vb_ptr, pcm);
      sample += 64;
      vb_ptr = (vb_ptr - 8) & 127;
      pcm += 8;
   }

}

void sbtB8_dual(float *sample, unsigned char *pcm, int n)
{
   int i;

   for (i = 0; i < n; i++)
   {
      fdct8_dual(sample, vbuf + vb_ptr);
      fdct8_dual(sample + 1, vbuf2 + vb_ptr);
      windowB8_dual(vbuf, vb_ptr, pcm);
      windowB8_dual(vbuf2, vb_ptr, pcm + 1);
      sample += 64;
      vb_ptr = (vb_ptr - 8) & 127;
      pcm += 16;
   }
}

void sbtB8_dual_mono(float *sample, unsigned char *pcm, int n)
{
   int i;

   for (i = 0; i < n; i++)
   {
      fdct8_dual_mono(sample, vbuf + vb_ptr);
      windowB8(vbuf, vb_ptr, pcm);
      sample += 64;
      vb_ptr = (vb_ptr - 8) & 127;
      pcm += 8;
   }
}

void sbtB8_dual_left(float *sample, unsigned char *pcm, int n)
{
   int i;

   for (i = 0; i < n; i++)
   {
      fdct8_dual(sample, vbuf + vb_ptr);
      windowB8(vbuf, vb_ptr, pcm);
      sample += 64;
      vb_ptr = (vb_ptr - 8) & 127;
      pcm += 8;
   }
}

void sbtB8_dual_right(float *sample, unsigned char *pcm, int n)
{
   int i;

   sample++;
   for (i = 0; i < n; i++)
   {
      fdct8_dual(sample, vbuf + vb_ptr);
      windowB8(vbuf, vb_ptr, pcm);
      sample += 64;
      vb_ptr = (vb_ptr - 8) & 127;
      pcm += 8;
   }
}

void sbtB_mono_L3(float *sample, unsigned char *pcm, int ch)
{
   int i;

   ch = 0;
   for (i = 0; i < 18; i++)
   {
      fdct32(sample, vbuf + vb_ptr);
      windowB(vbuf, vb_ptr, pcm);
      sample += 32;
      vb_ptr = (vb_ptr - 32) & 511;
      pcm += 32;
   }

}

void sbtB_dual_L3(float *sample, unsigned char *pcm, int ch)
{
   int i;

   if (ch == 0)
      for (i = 0; i < 18; i++)
      {
	 fdct32(sample, vbuf + vb_ptr);
	 windowB_dual(vbuf, vb_ptr, pcm);
	 sample += 32;
	 vb_ptr = (vb_ptr - 32) & 511;
	 pcm += 64;
      }
   else
      for (i = 0; i < 18; i++)
      {
	 fdct32(sample, vbuf2 + vb2_ptr);
	 windowB_dual(vbuf2, vb2_ptr, pcm + 1);
	 sample += 32;
	 vb2_ptr = (vb2_ptr - 32) & 511;
	 pcm += 64;
      }

}

void sbtB16_mono_L3(float *sample, unsigned char *pcm, int ch)
{
   int i;

   ch = 0;
   for (i = 0; i < 18; i++)
   {
      fdct16(sample, vbuf + vb_ptr);
      windowB16(vbuf, vb_ptr, pcm);
      sample += 32;
      vb_ptr = (vb_ptr - 16) & 255;
      pcm += 16;
   }


}

void sbtB16_dual_L3(float *sample, unsigned char *pcm, int ch)
{
   int i;

   if (ch == 0)
   {
      for (i = 0; i < 18; i++)
      {
	 fdct16(sample, vbuf + vb_ptr);
	 windowB16_dual(vbuf, vb_ptr, pcm);
	 sample += 32;
	 vb_ptr = (vb_ptr - 16) & 255;
	 pcm += 32;
      }
   }
   else
   {
      for (i = 0; i < 18; i++)
      {
	 fdct16(sample, vbuf2 + vb2_ptr);
	 windowB16_dual(vbuf2, vb2_ptr, pcm + 1);
	 sample += 32;
	 vb2_ptr = (vb2_ptr - 16) & 255;
	 pcm += 32;
      }
   }

}

void sbtB8_mono_L3(float *sample, unsigned char *pcm, int ch)
{
   int i;

   ch = 0;
   for (i = 0; i < 18; i++)
   {
      fdct8(sample, vbuf + vb_ptr);
      windowB8(vbuf, vb_ptr, pcm);
      sample += 32;
      vb_ptr = (vb_ptr - 8) & 127;
      pcm += 8;
   }

}

void sbtB8_dual_L3(float *sample, unsigned char *pcm, int ch)
{
   int i;

   if (ch == 0)
   {
      for (i = 0; i < 18; i++)
      {
	 fdct8(sample, vbuf + vb_ptr);
	 windowB8_dual(vbuf, vb_ptr, pcm);
	 sample += 32;
	 vb_ptr = (vb_ptr - 8) & 127;
	 pcm += 16;
      }
   }
   else
   {
      for (i = 0; i < 18; i++)
      {
	 fdct8(sample, vbuf2 + vb2_ptr);
	 windowB8_dual(vbuf2, vb2_ptr, pcm + 1);
	 sample += 32;
	 vb2_ptr = (vb2_ptr - 8) & 127;
	 pcm += 16;
      }
   }
}
