#ifndef RS_SNDGEN_H
#define RS_SNDGEN_H

#include "rsgame.h"

//#include "rs/rsaudio.h"

typedef struct rs_sgen_reg_t {
    int wave_length;
    int waves_count;
    float *wave;
    signed short *wave_out;
} rs_sgen_reg_t;

extern rs_sgen_reg_t rs_sgen_reg;

void rs_sgen_init(int waves_count, int wave_length);
void rs_sgen_wave_out(int index);
void rs_sgen_term();

void rs_sgen_func_noise(int index, int seed);
void rs_sgen_func_sin(int index, float freq, float p); 
void rs_sgen_func_pm(int index, float freq, float p, float k, float freq2, float p2);

void rs_sgen_func_lowpass(int dest, int src, float alpha_start, float alpha_end, float alpha_pow);
void rs_sgen_func_highpass(int dest, int src, float alpha_start, float alpha_end, float alpha_pow);
//void rs_sgen_func_phaser(int dest, int src);
void rs_sgen_func_phaser(int dest, int src, float fb, float lfoPhase, float depth, float range_start, float range_end, float rate);
void rs_sgen_func_normalize(int dest, float amp);
void rs_sgen_func_limiter(int dest, float val);
void rs_sgen_func_reverb(int dest, int src, int echo_delay, float echo_decay_koef);

#define rs_sgen_func_copy(dst,src) rs_sgen_func_add(dst,src,dst,1.0,0.0)
void rs_sgen_func_add(int dest, int src1, int src2, float k1, float k2);
void rs_sgen_func_mult(int dest, int src1, int src2);

void rs_sgen_func_shift(int dest, int src);

//void rs_gen_func_mult_add_value(int dest, int src, float val_mult, float val_add);



#endif
