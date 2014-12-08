#include "rsgentex.h"

#ifdef RS_USE_C_LIBS
    #include <math.h>
    #include <stdlib.h> 
#endif

#include "rsnoise.h"

#include "rs/rsplatform.h"
#include "rs/rsmx.h"

/*

    Procedural Texture Generator
    by Roman Shuvalov


*/







rs_gen_reg_t rs_gen_reg;





void rs_gen_func_mult_add_value(int dest, int src, float val_mult, float val_add) {
    int i;
    for (i = 0; i < rs_gen_reg.tex_length; i++ ) {
        rs_gen_reg.tex[dest*rs_gen_reg.tex_length + i] = rs_gen_reg.tex[src*rs_gen_reg.tex_length + i] * val_mult + val_add;
    };
};

void rs_gen_func_perlin(int dest, float freq, int octaves, float persistence, float seed) {
    rs_perlin_configure(freq, octaves, persistence, seed, rs_gen_reg.tex_size);
    int i, j;
    for (i = 0; i < rs_gen_reg.tex_size; i++) {
        for (j = 0; j < rs_gen_reg.tex_size; j++) {
            rs_gen_reg.tex[dest*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] = 0.5 + 0.5*rs_perlin(i, j); // rs_perlin(i, j);
        };
    };
};

void rs_gen_func_quads(int dest, float freq, int octaves, float persistence, float seed) {
    rs_perlin_configure(freq, octaves, persistence, seed, rs_gen_reg.tex_size);
    int i, j;
    for (i = 0; i < rs_gen_reg.tex_size; i++) {
        for (j = 0; j < rs_gen_reg.tex_size; j++) {
            rs_gen_reg.tex[dest*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] = 0.5 + 0.5*rs_quad_noise(i, j); // rs_perlin(i, j);
        };
    };
};

void rs_gen_func_cell(int dest, int seed, int num_points, signed short* p_arr, float k1, float pow1, float k2, float pow2, float k3, float pow3) {


    int x[num_points];
    int y[num_points];

    int i, j, p, dx, dy;

    int ts = rs_gen_reg.tex_size;

    float mindist, mindist2, mindist3;
    float dist;

    if (p_arr == NULL) {
        for (p = 0; p < num_points; p++) {
                x[p] = (0.5 + 0.5*rs_noise(p, seed)) * ts;
                y[p] = (0.5 + 0.5*rs_noise(p, seed+100)) * ts;
        //        x[p] = (p * 18 ) % ts;
        //        y[p] = (p * 33 ) % ts;
        }
    }
    else {
        for (p = 0; p < num_points; p++) {
                x[p] = rs_gen_reg.cell_scale * p_arr[p*2 + 0];
                y[p] = rs_gen_reg.cell_scale * p_arr[p*2 + 1];
        //        x[p] = (p * 18 ) % ts;
        //        y[p] = (p * 33 ) % ts;
        }
    };

    int cyclic = ((p_arr!=NULL) && (seed!=0)) ? 0 : 1;


//    float maxdist = 0.0;
//    float mindx, mindy, maxdx, maxdy;

    int ind_index = 0;
    int ind_index2 = 0;

    for (i = 0; i < ts; i++) {
        for (j = 0; j < ts; j++) {

            // distance to near point
            mindist = ts;
            for (p = 0; p < num_points; p++) {
                dx = (ts/2 - abs( abs(j-x[p]) - ts/2))*cyclic + ( abs(j-x[p]) )*(1-cyclic);
                dy = (ts/2 - abs( abs(i-y[p]) - ts/2))*cyclic + ( abs(i-y[p]) )*(1-cyclic);
                dist = sqrtf( RS_SQR(dx) + RS_SQR(dy) );
                if (dist < mindist) {
                    mindist = dist;
                    ind_index = p;
                };
            };

            mindist2 = ts;
            for (p = 0; p < num_points; p++) {
                if (p == ind_index) {
                    continue;
                };
                dx = (ts/2 - abs( abs(j-x[p]) - ts/2))*cyclic + ( abs(j-x[p]) )*(1-cyclic);
                dy = (ts/2 - abs( abs(i-y[p]) - ts/2))*cyclic + ( abs(i-y[p]) )*(1-cyclic);
                dist = sqrtf( RS_SQR(dx) + RS_SQR(dy) );
                if (dist < mindist2) {
                    mindist2 = dist;
                    ind_index2 = p;
                };
            };

            mindist3 = ts;
            for (p = 0; p < num_points; p++) {
                if ( (p == ind_index) || (p == ind_index2) ) {
                    continue;
                };
                dx = (ts/2 - abs( abs(j-x[p]) - ts/2))*cyclic + ( abs(j-x[p]) )*(1-cyclic);
                dy = (ts/2 - abs( abs(i-y[p]) - ts/2))*cyclic + ( abs(i-y[p]) )*(1-cyclic);
                dist = sqrtf( RS_SQR(dx) + RS_SQR(dy) );
                if (dist < mindist3) {
                    mindist3 = dist;
                };
            };

            rs_gen_reg.tex[dest*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] =
                k1*rs_pow(mindist,pow1) + k2*rs_pow(mindist2,pow2) + k3*rs_pow(mindist3,pow3);
            // mindist2 * mindist3 + mindist;
        };
    };

};

void rs_gen_func_adr(int dest, int src, int adr_x, int adr_y, float src_factor, float adr_factor) {
    int i, j;
    for (i = 0; i < rs_gen_reg.tex_size; i++) {
        for (j = 0; j < rs_gen_reg.tex_size; j++) {
            unsigned short u = src_factor*j + adr_factor*(rs_gen_reg.tex[adr_x*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] - 0.5) * rs_gen_reg.tex_size;
            unsigned short v = src_factor*i + adr_factor*(rs_gen_reg.tex[adr_y*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] - 0.5) * rs_gen_reg.tex_size;

            u %= rs_gen_reg.tex_size;
            v %= rs_gen_reg.tex_size;

            rs_gen_reg.tex[dest*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] =
                rs_gen_reg.tex[src*rs_gen_reg.tex_length + v*rs_gen_reg.tex_size + u];
        };
    };
};

void rs_gen_func_normalmap(int dest_r, int dest_g, int dest_b, int src, float k) {
    int i, j;
    float max_val = -111111.0;
    float min_val = 100000.0;
    for (i = 0; i < rs_gen_reg.tex_size; i++) {
        for (j = 0; j < rs_gen_reg.tex_size; j++) {
            unsigned short um = rs_gen_reg.tex_size + j-2;
            unsigned short vm = rs_gen_reg.tex_size + i-2;

            unsigned short u = j;
            unsigned short v = i;

            unsigned short up = j+2;
            unsigned short vp = i+2;

            um  %= rs_gen_reg.tex_size;
            vm  %= rs_gen_reg.tex_size;
            up %= rs_gen_reg.tex_size;
            vp %= rs_gen_reg.tex_size;

//            float val1 = rs_gen_reg.tex[src*rs_gen_reg.tex_length + v*rs_gen_reg.tex_size + u];

            float val_xp = 2.0 * rs_gen_reg.tex[src*rs_gen_reg.tex_length +  v*rs_gen_reg.tex_size + up]
                + rs_gen_reg.tex[src*rs_gen_reg.tex_length +  vm*rs_gen_reg.tex_size + up]
                + rs_gen_reg.tex[src*rs_gen_reg.tex_length +  vp*rs_gen_reg.tex_size + up];

            float val_xm = 2.0 * rs_gen_reg.tex[src*rs_gen_reg.tex_length +  v*rs_gen_reg.tex_size + um]
                + rs_gen_reg.tex[src*rs_gen_reg.tex_length +  vm*rs_gen_reg.tex_size + um]
                + rs_gen_reg.tex[src*rs_gen_reg.tex_length +  vp*rs_gen_reg.tex_size + um];

            float val_yp = 2.0 * rs_gen_reg.tex[src*rs_gen_reg.tex_length +  vp*rs_gen_reg.tex_size + u]
                + rs_gen_reg.tex[src*rs_gen_reg.tex_length +  vp*rs_gen_reg.tex_size + um]
                + rs_gen_reg.tex[src*rs_gen_reg.tex_length +  vp*rs_gen_reg.tex_size + up];

            float val_ym = 2.0 * rs_gen_reg.tex[src*rs_gen_reg.tex_length +  vm*rs_gen_reg.tex_size + u]
                + rs_gen_reg.tex[src*rs_gen_reg.tex_length +  vm*rs_gen_reg.tex_size + um]
                + rs_gen_reg.tex[src*rs_gen_reg.tex_length +  vm*rs_gen_reg.tex_size + up];

//            float val_dx = rs_gen_reg.tex[src*rs_gen_reg.tex_length +  v*rs_gen_reg.tex_size + u2] - val1;
//            float val_dy = rs_gen_reg.tex[src*rs_gen_reg.tex_length + v2*rs_gen_reg.tex_size +  u] - val1;

            float val_dx = val_xp - val_xm;
            float val_dy = val_yp - val_ym;



//            val_dx = val_dx;
//            val_dy = -val_dy;

//            val_dx = atan(val_dx * rs_gen_reg.tex_size ) / (M_PI/2);
//            val_dy = atan(val_dy * rs_gen_reg.tex_size ) / (M_PI/2);

            float bump_scale = 128.0 / rs_gen_reg.tex_size / k;

            rs_vec3_t bump = rs_vec3_cross( rs_vec3_normalize(rs_vec3(bump_scale, 0.0, val_dy)),
                rs_vec3_normalize(rs_vec3(0.0, bump_scale, -val_dx)));



            float val_dz = sqrtf( 1.0 - (RS_SQR(k*val_dx) + RS_SQR(k*val_dy)) );
//            val_dz = val_dz;

//            val_dz = 1.0 / 2.0;

            val_dx = 0.5 + 0.5*val_dx;
            val_dy = 0.5 + 0.5*val_dy;
            val_dz = 0.5 + 0.5*val_dz;

            max_val = rs_max(max_val, fabs(val_dx) );
//            max_val = rs_max(max_val, fabs(val_dy) );
            min_val = rs_min(min_val, val_dx);
//            min_val = rs_min(min_val, val_dy);

//            rs_gen_reg.tex[dest_r*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] = val_dy;
//            rs_gen_reg.tex[dest_g*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] = val_dx;
//            rs_gen_reg.tex[dest_b*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] = val_dz;

            rs_gen_reg.tex[dest_r*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] = 0.5 + 0.5*bump.x;
            rs_gen_reg.tex[dest_g*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] = 0.5 + 0.5*bump.y;
            rs_gen_reg.tex[dest_b*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] = 0.5 + 0.5*bump.z;

        };
    };

//////    if (max_val < 0.001) {
//////        DEBUG10f("WARNING, max_val of normalmap is too low (%.9f) \n", max_val);
//////        max_val = 0.001;
//////    };
//////
//////    max_val *= 1.0;
//////
//////    for (i = 0; i < rs_gen_reg.tex_size; i++) {
//////        for (j = 0; j < rs_gen_reg.tex_size; j++) {
//////
//////            rs_gen_reg.tex[dest_r*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] /= max_val;
//////            rs_gen_reg.tex[dest_g*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] /= max_val;
//////
//////            rs_gen_reg.tex[dest_b*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] =
//////                sqrtf( 1.0 - (RS_SQR(rs_gen_reg.tex[dest_r*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j])
//////                    + RS_SQR(rs_gen_reg.tex[dest_g*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j])) );
//////
//////
//////            rs_gen_reg.tex[dest_r*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] =
//////                0.5 + 0.5*rs_gen_reg.tex[dest_r*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j];
//////            rs_gen_reg.tex[dest_g*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] =
//////                0.5 + 0.5*rs_gen_reg.tex[dest_g*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j];
//////
//////
////////            float val_dx = rs_gen_reg.tex[dest_r*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j];
////////            float val_dy = rs_gen_reg.tex[dest_g*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j];
////////
////////            float val_dz = sqrtf( 1.0 - (RS_SQR(k*val_dx) + RS_SQR(k*val_dy)) );
//////
////////            val_dx = 0.5 + 0.5*k*val_dx;
////////            val_dy = 0.5 + 0.5*k*val_dy;
////////            val_dz = 0.5 + 0.5*val_dz;
//////
//////
//////            //rs_gen_reg.tex[dest_b*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] = val_dz;
//////
//////        };
//////    };

    DEBUG10f("\n\nmax val %.3f \nmin %.3f \n", max_val, min_val);

};

void rs_gen_func_radial(int dest, float x, float y, float radius, float v, float power) {
    x *= rs_gen_reg.tex_size;
    y *= rs_gen_reg.tex_size;
    radius *= rs_gen_reg.tex_size;
    float val;
    int i, j;
    for (i = 0; i < rs_gen_reg.tex_size; i++) {
        for (j = 0; j < rs_gen_reg.tex_size; j++) {
            val = rs_clamp(sqrt(RS_SQR(j-x) + RS_SQR(i-y)) / radius, 0.0, 1.0);
            val = pow(val, power);
            rs_gen_reg.tex[dest*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] =
                rs_gen_reg.tex[dest*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j]*val + v*(1.0-val); // 0.5 + 0.5*rs_quad_noise(i, j); // rs_perlin(i, j);
        };
    };
};

void rs_gen_func_gradient_v(int dest, float v, float power) {
    float val;
    int i, j;
    for (i = 0; i < rs_gen_reg.tex_size; i++) {
        val = rs_clamp( (float) (i) / rs_gen_reg.tex_size, 0.0, 1.0);
        val = pow(val, power);
        for (j = 0; j < rs_gen_reg.tex_size; j++) {
            rs_gen_reg.tex[dest*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] =
                rs_gen_reg.tex[dest*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j]*val + v*(1.0-val); // 0.5 + 0.5*rs_quad_noise(i, j); // rs_perlin(i, j);
        };
    };
};

void rs_gen_func_normalize(int dest, float vmin, float vmax) {


        // LAGGY !!!!!!!!!!!!!

    float val_min = rs_gen_reg.tex[dest*rs_gen_reg.tex_length];
    float val_max = rs_gen_reg.tex[dest*rs_gen_reg.tex_length];
    float f;

    int i, j;

    for (i = 0; i < rs_gen_reg.tex_size; i++) {
        for (j = 0; j < rs_gen_reg.tex_size; j++) {
            f = rs_gen_reg.tex[dest*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] ;
            val_max = rs_max(val_max, f);
            val_min = rs_min(val_min, f);
        };
    };

    float val_scale = (vmax - vmin) / (val_max - val_min) - vmin;

    for (i = 0; i < rs_gen_reg.tex_size; i++) {
        for (j = 0; j < rs_gen_reg.tex_size; j++) {
            rs_gen_reg.tex[dest*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] =
                vmin + val_scale * ( rs_gen_reg.tex[dest*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] - val_min);
        };
    };
};


void rs_gen_func_clamp(int dest, float vmin, float vmax) {

    float val;
    int i, j;
    for (i = 0; i < rs_gen_reg.tex_size; i++) {
        for (j = 0; j < rs_gen_reg.tex_size; j++) {
            val = rs_gen_reg.tex[dest*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j];
            rs_gen_reg.tex[dest*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] =
                rs_clamp(val, vmin, vmax);
        };
    };
};


void rs_gen_func_set(int dest, float val) {
    int i, j;
    for (i = 0; i < rs_gen_reg.tex_size; i++) {
        for (j = 0; j < rs_gen_reg.tex_size; j++) {
            rs_gen_reg.tex[dest*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] = val;
        };
    };
};

void rs_gen_func_noise(int dest, int seed) {
    int i, j;
    for (i = 0; i < rs_gen_reg.tex_size; i++) {
        for (j = 0; j < rs_gen_reg.tex_size; j++) {
            rs_gen_reg.tex[dest*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] = rs_noise(i+seed, j+seed/100);
        };
    };
};

void rs_gen_func_add(int dest, int src1, int src2, float k1, float k2) {
    int i;
    for (i = 0; i < rs_gen_reg.tex_length; i++ ) {
        rs_gen_reg.tex[dest*rs_gen_reg.tex_length + i] =
            rs_gen_reg.tex[src1*rs_gen_reg.tex_length + i] * k1
            + rs_gen_reg.tex[src2*rs_gen_reg.tex_length + i] * k2;
    };
};

void rs_gen_func_add_lerped(int dest, int src1, int src2, float k1, float k2) {
    int i;
    for (i = 0; i < rs_gen_reg.tex_length; i++ ) {
        rs_gen_reg.tex[dest*rs_gen_reg.tex_length + i] =
            rs_gen_reg.tex[src1*rs_gen_reg.tex_length + i] * (1.0 - 0.5*rs_gen_reg.tex[src2*rs_gen_reg.tex_length + i])
            + (1.0 - 0.5*rs_gen_reg.tex[src1*rs_gen_reg.tex_length + i]) * rs_gen_reg.tex[src2*rs_gen_reg.tex_length + i];
    };
};

void rs_gen_func_lerp_parametric(int dest, int src1, int src2, int param) {
    int i;
    for (i = 0; i < rs_gen_reg.tex_length; i++ ) {
        rs_gen_reg.tex[dest*rs_gen_reg.tex_length + i] =
            rs_gen_reg.tex[src1*rs_gen_reg.tex_length + i] * rs_gen_reg.tex[param*rs_gen_reg.tex_length + i]
            + rs_gen_reg.tex[src2*rs_gen_reg.tex_length + i] * (1.0 - rs_gen_reg.tex[param*rs_gen_reg.tex_length + i]);
    };
};

void rs_gen_func_mult(int dest, int src1, int src2) {
    int i;
    for (i = 0; i < rs_gen_reg.tex_length; i++ ) {
        rs_gen_reg.tex[dest*rs_gen_reg.tex_length + i] =
            rs_gen_reg.tex[src1*rs_gen_reg.tex_length + i] * rs_gen_reg.tex[src2*rs_gen_reg.tex_length + i];
    };
};




void rs_gen_init(int tex_count, int tex_size) {
    rs_gen_reg.tex_count = tex_count;
    rs_gen_reg.tex_size = tex_size;
    rs_gen_reg.tex_length = tex_size*tex_size;
    rs_gen_reg.tex = malloc(tex_count * tex_size * tex_size * 4); // float
    rs_gen_reg.tex_out = malloc(tex_size * tex_size * 4); // unsigned char RGBA, RGB
    rs_gen_tex_out_rgba_set(0.0, 0.0, 0.0, 0.0);
};

void rs_gen_term() {
    free(rs_gen_reg.tex);
    free(rs_gen_reg.tex_out);
};

void rs_gen_tex_out(int tex, int bpp) {
    int i;
    int j;
    for (i = 0; i < rs_gen_reg.tex_length; i++) {
        for (j = 0; j < bpp; j++) {
            rs_gen_reg.tex_out[i*bpp + j] = 255*rs_gen_reg.tex[tex*rs_gen_reg.tex_length + i];
        };
    };
};

void rs_gen_tex_out_rgb(int tex_r, int tex_g, int tex_b, float kr, float kg, float kb) {
    int bpp = 3;
    int i;
    for (i = 0; i < rs_gen_reg.tex_length; i++) {
        rs_gen_reg.tex_out[i*bpp + 0] += 255*kr*rs_gen_reg.tex[tex_r*rs_gen_reg.tex_length + i];
        rs_gen_reg.tex_out[i*bpp + 1] += 255*kg*rs_gen_reg.tex[tex_g*rs_gen_reg.tex_length + i];
        rs_gen_reg.tex_out[i*bpp + 2] += 255*kb*rs_gen_reg.tex[tex_b*rs_gen_reg.tex_length + i];
    };
};

void rs_gen_tex_out_rgb_set(float r, float g, float b) {
    int bpp = 3;
    int i;
    for (i = 0; i < rs_gen_reg.tex_length; i++) {
        rs_gen_reg.tex_out[i*bpp + 0] = 255*r;
        rs_gen_reg.tex_out[i*bpp + 1] = 255*g;
        rs_gen_reg.tex_out[i*bpp + 2] = 255*b;
    };
};

void rs_gen_tex_out_rgba(int tex_r, int tex_g, int tex_b, int tex_a, float kr, float kg, float kb, float ka) {
    int bpp = 4;
    int i;
    for (i = 0; i < rs_gen_reg.tex_length; i++) {
        rs_gen_reg.tex_out[i*bpp + 0] += 255*kr*rs_gen_reg.tex[tex_r*rs_gen_reg.tex_length + i];
        rs_gen_reg.tex_out[i*bpp + 1] += 255*kg*rs_gen_reg.tex[tex_g*rs_gen_reg.tex_length + i];
        rs_gen_reg.tex_out[i*bpp + 2] += 255*kb*rs_gen_reg.tex[tex_b*rs_gen_reg.tex_length + i];
        rs_gen_reg.tex_out[i*bpp + 3] += (tex_a<0) ? 255 : 255*ka*rs_gen_reg.tex[tex_a*rs_gen_reg.tex_length + i]; // <---- -1 for alpha=1, for KolibriOS
    };
};

void rs_gen_tex_out_rgba_set(float r, float g, float b, float a) {
    int bpp = 4;
    int i;
    for (i = 0; i < rs_gen_reg.tex_length; i++) {
        rs_gen_reg.tex_out[i*bpp + 0] = 255*r;
        rs_gen_reg.tex_out[i*bpp + 1] = 255*g;
        rs_gen_reg.tex_out[i*bpp + 2] = 255*b;
        rs_gen_reg.tex_out[i*bpp + 3] = 255*a;
    };
};


void rs_gen_func_apply_mask(int dest, unsigned char *mask_data) {
    
    int i, j;
    for (i = 0; i < rs_gen_reg.tex_size; i++) {
        for (j = 0; j < rs_gen_reg.tex_size; j++) {
            rs_gen_reg.tex[dest*rs_gen_reg.tex_length + i*rs_gen_reg.tex_size + j] *= 
                (mask_data[ j*rs_gen_reg.tex_size/8 + i/8] & (1 << (7 - (i%8)) ) ) ? 1.0 : 0.0;
        };
    };
    
};

void rs_gen_func_posterize(int dest, int colors_count) {
    int i;
    float f;
    for (i = 0; i < rs_gen_reg.tex_length; i++ ) {
        f = rs_gen_reg.tex[dest*rs_gen_reg.tex_length + i];
        f *= (254.0/255.0 + colors_count);
        f = floor(f);
        f /= colors_count;
        rs_gen_reg.tex[dest*rs_gen_reg.tex_length + i] = f;
    };
};
