#include <math.h>	//sqrt

static float csa[8][2];		/* antialias */

void alias_init()
{
	float Ci[8] =
	{
		-0.6f, -0.535f, -0.33f, -0.185f, -0.095f, -0.041f, -0.0142f, -0.0037f
	};

	int i;

	for (i = 0; i < 8; i++)
	{
		csa[i][0] = (float) (1.0 / sqrt(1.0 + Ci[i] * Ci[i]));
		csa[i][1] = (float) (Ci[i] / sqrt(1.0 + Ci[i] * Ci[i]));
	}
}

void antialias(float x[], int n)
{
   int i, k;
   float a, b;

   for (k = 0; k < n; k++)
   {
      for (i = 0; i < 8; i++)
      {
	 a = x[17 - i];
	 b = x[18 + i];
	 x[17 - i] = a * csa[i][0] - b * csa[i][1];
	 x[18 + i] = b * csa[i][0] + a * csa[i][1];
      }
      x += 18;
   }

}
