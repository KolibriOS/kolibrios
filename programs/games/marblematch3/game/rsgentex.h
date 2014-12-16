#ifndef RS_GTX_H
#define RS_GTX_H

#include "rsgame.h"

/*

    Procedural Texture Generator
    by Roman Shuvalov

*/

//void rs_model_create_pyramid(rs_vbo_t* vbo);

void generate_perlin_tex();

typedef struct rs_gen_reg_t {
    int tex_size;
    int tex_length; // tex_size*tex_size
    int tex_count;
    float cell_scale;
    float *tex;
    unsigned char *tex_out;
} rs_gen_reg_t;

extern rs_gen_reg_t rs_gen_reg;

void rs_gen_init(int tex_count, int tex_size);

void rs_gen_tex_out(int tex, int bpp);
void rs_gen_tex_out_rgb_set(float r, float g, float b);
void rs_gen_tex_out_rgb(int tex_r, int tex_g, int tex_b, float kr, float kg, float kb);
void rs_gen_tex_out_rgba_set(float r, float g, float b, float a);
void rs_gen_tex_out_rgba(int tex_r, int tex_g, int tex_b, int tex_a, float kr, float kg, float kb, float ka);


void rs_gen_term();

void rs_gen_func_mult_add_value(int dest, int src, float val_mult, float val_add);
void rs_gen_func_perlin(int dest, float freq, int octaves, float persistence, float seed);
void rs_gen_func_quads(int dest, float freq, int octaves, float persistence, float seed);
void rs_gen_func_radial(int dest, float x, float y, float radius, float v, float power);
void rs_gen_func_gradient_v(int dest, float v, float power);
void rs_gen_func_add(int dest, int src1, int src2, float k1, float k2);
void rs_gen_func_add_lerped(int dest, int src1, int src2, float k1, float k2);
void rs_gen_func_normalize(int dest, float vmin, float vmax);
void rs_gen_func_clamp(int dest, float vmin, float vmax);
void rs_gen_func_lerp_parametric(int dest, int src1, int src2, int param);
void rs_gen_func_mult(int dest, int src1, int src2);
void rs_gen_func_set(int dest, float val);
void rs_gen_func_noise(int dest, int seed);
void rs_gen_func_cell(int dest, int seed, int num_points, signed short *p, float k1, float pow1, float k2, float pow2, float k3, float pow3);
void rs_gen_func_adr(int dest, int src, int adr_x, int adr_y, float src_factor, float adr_factor);
void rs_gen_func_normalmap(int dest_r, int dest_g, int dest_b, int src, float k);
void rs_gen_func_pow(int dest, int src, float p);
void rs_gen_func_inv(int dest, int src, float p);

void rs_gen_func_apply_mask(int dest, unsigned char *mask_data);
void rs_gen_func_posterize(int dest, int colors_count);


#endif
