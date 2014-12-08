#include "rsnoise.h"

#include "rsgame.h"

#ifdef RS_USE_C_LIBS
    #include <string.h>
    #include <math.h>
#else
    #include "rs/rsplatform.h" 
#endif

rs_perlin_conf_t rs_perlin_conf;

void rs_perlin_configure(float freq, int octaves, float persistence, float seed, int tex_size) {
    rs_perlin_conf.freq = freq;
    rs_perlin_conf.octaves = octaves;
    rs_perlin_conf.persistence = persistence;
    rs_perlin_conf.seed = seed;
    rs_perlin_conf.tex_size = tex_size;
    rs_perlin_conf.period = (int) (0.1 + roundf(freq) ); // *tex_size
};

float rs_noise(int x, int y) {
    // from here, http://www.cplusplus.com/forum/general/85758/
    // koef. changed
    int n = x + y * 57 * 5; // no *2
    n = (n << 13) ^ n;
    int t = (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff;
    return 1.0 - ((float)t) * 0.931322574615478515625e-9;/// 1073741824.0);
}

float rs_noise_for_perlin(int x, int y) {

    x %= rs_perlin_conf.period;
    y %= rs_perlin_conf.period;

    // from here, http://www.cplusplus.com/forum/general/85758/
    // koef. changed
    int n = x + y * 57 * 5; // no *2
    n = (n << 13) ^ n;
    int t = (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff;
    return 1.0 - ((float)t) * 0.931322574615478515625e-9;/// 1073741824.0);
}

//float rs_noise_periodical(int x, int y, int period) {
//
//    x %= period; 
//    y %= period;
//
//    // from here, http://www.cplusplus.com/forum/general/85758/
//    // koef. changed
//    int n = x + y * 57 * 5; // no *2
//    n = (n << 13) ^ n;
//    int t = (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff;
//    return 1.0 - ((float)t) * 0.931322574615478515625e-9;/// 1073741824.0);
//}

float rs_interpolate(float x, float y, float a) {
    float negA = 1.0 - a;
  float negASqr = negA * negA;
    float fac1 = 3.0 * (negASqr) - 2.0 * (negASqr * negA);
  float aSqr = a * a;
    float fac2 = 3.0 * aSqr - 2.0 * (aSqr * a);

    return x * fac1 + y * fac2; //add the weighted factors
}

float rs_perlin_noise(float x, float y) {

//    while (x > rs_perlin_conf.tile_period) {
//        x -= rs_perlin_conf.tile_period;
//    };
//    
//    while (y > rs_perlin_conf.tile_period) {
//        y -= rs_perlin_conf.tile_period;
//    };

    int Xint = (int)x;
    int Yint = (int)y;
    float Xfrac = x - Xint;
    float Yfrac = y - Yint;


  //noise values
  float n01= rs_noise_for_perlin(Xint-1, Yint-1);
  float n02= rs_noise_for_perlin(Xint+1, Yint-1);
  float n03= rs_noise_for_perlin(Xint-1, Yint+1);
  float n04= rs_noise_for_perlin(Xint+1, Yint+1);
  float n05= rs_noise_for_perlin(Xint-1, Yint);
  float n06= rs_noise_for_perlin(Xint+1, Yint);
  float n07= rs_noise_for_perlin(Xint, Yint-1);
  float n08= rs_noise_for_perlin(Xint, Yint+1);
  float n09= rs_noise_for_perlin(Xint, Yint);

  float n12= rs_noise_for_perlin(Xint+2, Yint-1);
  float n14= rs_noise_for_perlin(Xint+2, Yint+1);
  float n16= rs_noise_for_perlin(Xint+2, Yint);

  float n23= rs_noise_for_perlin(Xint-1, Yint+2);
  float n24= rs_noise_for_perlin(Xint+1, Yint+2);
  float n28= rs_noise_for_perlin(Xint, Yint+2);

  float n34= rs_noise_for_perlin(Xint+2, Yint+2);

    //find the noise values of the four corners
    float x0y0 = 0.0625*(n01+n02+n03+n04) + 0.125*(n05+n06+n07+n08) + 0.25*(n09);
    float x1y0 = 0.0625*(n07+n12+n08+n14) + 0.125*(n09+n16+n02+n04) + 0.25*(n06);
    float x0y1 = 0.0625*(n05+n06+n23+n24) + 0.125*(n03+n04+n09+n28) + 0.25*(n08);
    float x1y1 = 0.0625*(n09+n16+n28+n34) + 0.125*(n08+n14+n06+n24) + 0.25*(n04);

    //interpolate between those values according to the x and y fractions
    float v1 = rs_interpolate(x0y0, x1y0, Xfrac); //interpolate in x direction (y)
    float v2 = rs_interpolate(x0y1, x1y1, Xfrac); //interpolate in x direction (y+1)
    float fin = rs_interpolate(v1, v2, Yfrac);  //interpolate in y direction

    return fin;
}

float rs_perlin(float i, float j) {

    float t = 0.0f;
    float _amplitude = 1.0;

    
    int k;
    
    float amplitude_divider = 0.0;
    for (k = 0; k < rs_perlin_conf.octaves; k++) {
        amplitude_divider += _amplitude;
        _amplitude *= rs_perlin_conf.persistence;
    };
    
    _amplitude = 1.0;
    
    float freq = rs_perlin_conf.freq;

    for(k = 0; k < rs_perlin_conf.octaves; k++)
    {
        t += rs_perlin_noise(j * freq / rs_perlin_conf.tex_size + rs_perlin_conf.seed, i * freq / rs_perlin_conf.tex_size + rs_perlin_conf.seed) * _amplitude;
        _amplitude *= rs_perlin_conf.persistence;
        freq *= 2;
    }

    return t / amplitude_divider;
};

float rs_quad_noise(float i, float j) {

    float t = 0.0f;
    float _amplitude = 1.0;

    
    int k;
    
    float amplitude_divider = 0.0;
    for (k = 0; k < rs_perlin_conf.octaves; k++) {
        amplitude_divider += _amplitude;
        _amplitude *= rs_perlin_conf.persistence;
    };
    
    _amplitude = 1.0;
    
    float freq = rs_perlin_conf.freq;

    for(k = 0; k < rs_perlin_conf.octaves; k++)
    {
        t += rs_noise(j * freq / rs_perlin_conf.tex_size + rs_perlin_conf.seed, i * freq / rs_perlin_conf.tex_size + rs_perlin_conf.seed) * _amplitude;
        _amplitude *= rs_perlin_conf.persistence;
        freq *= 2;
    }

    return t / amplitude_divider;
};


