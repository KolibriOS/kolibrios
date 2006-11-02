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

/* "window.c" */
void window(float *vbuf, int vb_ptr, short *pcm);
void window_dual(float *vbuf, int vb_ptr, short *pcm);
void window16(float *vbuf, int vb_ptr, short *pcm);
void window16_dual(float *vbuf, int vb_ptr, short *pcm);
void window8(float *vbuf, int vb_ptr, short *pcm);
void window8_dual(float *vbuf, int vb_ptr, short *pcm);

/* circular window buffers */
int vb_ptr;
int vb2_ptr;
float vbuf[512];
float vbuf2[512];

void sbt_init()
{
   int i;

   /* clear window vbuf */
   for (i = 0; i < 512; i++)
   {
      vbuf[i] = 0.0F;
      vbuf2[i] = 0.0F;
   }
   vb2_ptr = vb_ptr = 0;
}

void sbt_mono(float *sample, short *pcm, int n)
{
   int i;

   for (i = 0; i < n; i++)
   {
      fdct32(sample, vbuf + vb_ptr);
      window(vbuf, vb_ptr, pcm);
      sample += 64;
      vb_ptr = (vb_ptr - 32) & 511;
      pcm += 32;
   }

}

void sbt_dual(float *sample, short *pcm, int n)
{
   int i;

   for (i = 0; i < n; i++)
   {
      fdct32_dual(sample, vbuf + vb_ptr);
      fdct32_dual(sample + 1, vbuf2 + vb_ptr);
      window_dual(vbuf, vb_ptr, pcm);
      window_dual(vbuf2, vb_ptr, pcm + 1);
      sample += 64;
      vb_ptr = (vb_ptr - 32) & 511;
      pcm += 64;
   }
}

void sbt_dual_mono(float *sample, short *pcm, int n)
{
   int i;

   for (i = 0; i < n; i++)
   {
      fdct32_dual_mono(sample, vbuf + vb_ptr);
      window(vbuf, vb_ptr, pcm);
      sample += 64;
      vb_ptr = (vb_ptr - 32) & 511;
      pcm += 32;
   }

}

void sbt_dual_left(float *sample, short *pcm, int n)
{
   int i;

   for (i = 0; i < n; i++)
   {
      fdct32_dual(sample, vbuf + vb_ptr);
      window(vbuf, vb_ptr, pcm);
      sample += 64;
      vb_ptr = (vb_ptr - 32) & 511;
      pcm += 32;
   }
}

void sbt_dual_right(float *sample, short *pcm, int n)
{
   int i;

   sample++;			/* point to right chan */
   for (i = 0; i < n; i++)
   {
      fdct32_dual(sample, vbuf + vb_ptr);
      window(vbuf, vb_ptr, pcm);
      sample += 64;
      vb_ptr = (vb_ptr - 32) & 511;
      pcm += 32;
   }
}

void sbt16_mono(float *sample, short *pcm, int n)
{
   int i;

   for (i = 0; i < n; i++)
   {
      fdct16(sample, vbuf + vb_ptr);
      window16(vbuf, vb_ptr, pcm);
      sample += 64;
      vb_ptr = (vb_ptr - 16) & 255;
      pcm += 16;
   }
}

void sbt16_dual(float *sample, short *pcm, int n)
{
   int i;

   for (i = 0; i < n; i++)
   {
      fdct16_dual(sample, vbuf + vb_ptr);
      fdct16_dual(sample + 1, vbuf2 + vb_ptr);
      window16_dual(vbuf, vb_ptr, pcm);
      window16_dual(vbuf2, vb_ptr, pcm + 1);
      sample += 64;
      vb_ptr = (vb_ptr - 16) & 255;
      pcm += 32;
   }
}

void sbt16_dual_mono(float *sample, short *pcm, int n)
{
   int i;

   for (i = 0; i < n; i++)
   {
      fdct16_dual_mono(sample, vbuf + vb_ptr);
      window16(vbuf, vb_ptr, pcm);
      sample += 64;
      vb_ptr = (vb_ptr - 16) & 255;
      pcm += 16;
   }
}

void sbt16_dual_left(float *sample, short *pcm, int n)
{
   int i;

   for (i = 0; i < n; i++)
   {
      fdct16_dual(sample, vbuf + vb_ptr);
      window16(vbuf, vb_ptr, pcm);
      sample += 64;
      vb_ptr = (vb_ptr - 16) & 255;
      pcm += 16;
   }
}

void sbt16_dual_right(float *sample, short *pcm, int n)
{
   int i;

   sample++;
   for (i = 0; i < n; i++)
   {
      fdct16_dual(sample, vbuf + vb_ptr);
      window16(vbuf, vb_ptr, pcm);
      sample += 64;
      vb_ptr = (vb_ptr - 16) & 255;
      pcm += 16;
   }
}

void sbt8_mono(float *sample, short *pcm, int n)
{
   int i;

   for (i = 0; i < n; i++)
   {
      fdct8(sample, vbuf + vb_ptr);
      window8(vbuf, vb_ptr, pcm);
      sample += 64;
      vb_ptr = (vb_ptr - 8) & 127;
      pcm += 8;
   }

}

void sbt8_dual(float *sample, short *pcm, int n)
{
   int i;

   for (i = 0; i < n; i++)
   {
      fdct8_dual(sample, vbuf + vb_ptr);
      fdct8_dual(sample + 1, vbuf2 + vb_ptr);
      window8_dual(vbuf, vb_ptr, pcm);
      window8_dual(vbuf2, vb_ptr, pcm + 1);
      sample += 64;
      vb_ptr = (vb_ptr - 8) & 127;
      pcm += 16;
   }
}

void sbt8_dual_mono(float *sample, short *pcm, int n)
{
   int i;

   for (i = 0; i < n; i++)
   {
      fdct8_dual_mono(sample, vbuf + vb_ptr);
      window8(vbuf, vb_ptr, pcm);
      sample += 64;
      vb_ptr = (vb_ptr - 8) & 127;
      pcm += 8;
   }
}

void sbt8_dual_left(float *sample, short *pcm, int n)
{
   int i;

   for (i = 0; i < n; i++)
   {
      fdct8_dual(sample, vbuf + vb_ptr);
      window8(vbuf, vb_ptr, pcm);
      sample += 64;
      vb_ptr = (vb_ptr - 8) & 127;
      pcm += 8;
   }
}

void sbt8_dual_right(float *sample, short *pcm, int n)
{
   int i;

   sample++;
   for (i = 0; i < n; i++)
   {
      fdct8_dual(sample, vbuf + vb_ptr);
      window8(vbuf, vb_ptr, pcm);
      sample += 64;
      vb_ptr = (vb_ptr - 8) & 127;
      pcm += 8;
   }
}

void sbt_mono_L3(float *sample, short *pcm, int ch)
{
   int i;

   ch = 0;
   for (i = 0; i < 18; i++)
   {
      fdct32(sample, vbuf + vb_ptr);
      window(vbuf, vb_ptr, pcm);
      sample += 32;
      vb_ptr = (vb_ptr - 32) & 511;
      pcm += 32;
   }

}

void sbt_dual_L3(float *sample, short *pcm, int ch)
{
	int i;

	if (ch == 0)
		for (i = 0; i < 18; i++) {
			fdct32(sample, vbuf + vb_ptr);
			window_dual(vbuf, vb_ptr, pcm);
			sample += 32;
			vb_ptr = (vb_ptr - 32) & 511;
			pcm += 64;
		}
	else
		for (i = 0; i < 18; i++) {
			fdct32(sample, vbuf2 + vb2_ptr);
			window_dual(vbuf2, vb2_ptr, pcm + 1);
			sample += 32;
			vb2_ptr = (vb2_ptr - 32) & 511;
			pcm += 64;
		}
}

void sbt16_mono_L3(float *sample, short *pcm, int ch)
{
   int i;

   ch = 0;
   for (i = 0; i < 18; i++)
   {
      fdct16(sample, vbuf + vb_ptr);
      window16(vbuf, vb_ptr, pcm);
      sample += 32;
      vb_ptr = (vb_ptr - 16) & 255;
      pcm += 16;
   }


}

void sbt16_dual_L3(float *sample, short *pcm, int ch)
{
   int i;


   if (ch == 0)
   {
      for (i = 0; i < 18; i++)
      {
	 fdct16(sample, vbuf + vb_ptr);
	 window16_dual(vbuf, vb_ptr, pcm);
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
	 window16_dual(vbuf2, vb2_ptr, pcm + 1);
	 sample += 32;
	 vb2_ptr = (vb2_ptr - 16) & 255;
	 pcm += 32;
      }
   }

}

void sbt8_mono_L3(float *sample, short *pcm, int ch)
{
   int i;

   ch = 0;
   for (i = 0; i < 18; i++)
   {
      fdct8(sample, vbuf + vb_ptr);
      window8(vbuf, vb_ptr, pcm);
      sample += 32;
      vb_ptr = (vb_ptr - 8) & 127;
      pcm += 8;
   }

}

void sbt8_dual_L3(float *sample, short *pcm, int ch)
{
   int i;

   if (ch == 0)
   {
      for (i = 0; i < 18; i++)
      {
	 fdct8(sample, vbuf + vb_ptr);
	 window8_dual(vbuf, vb_ptr, pcm);
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
	 window8_dual(vbuf2, vb2_ptr, pcm + 1);
	 sample += 32;
	 vb2_ptr = (vb2_ptr - 8) & 127;
	 pcm += 16;
      }
   }



}

