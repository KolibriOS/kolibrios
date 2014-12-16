#include "rssoundgen.h"

#include "rsnoise.h"

#include "rs/rsmx.h"


#ifdef RS_KOS
    #include "rs/rsplatform.h"
#else
    #include <stdlib.h>
    #include <math.h>
    #include <string.h>
#endif

rs_sgen_reg_t rs_sgen_reg;

void rs_sgen_init(int waves_count, int wave_length) {
    rs_sgen_reg.waves_count = waves_count;
    rs_sgen_reg.wave_length = wave_length;
    rs_sgen_reg.wave = malloc(waves_count * wave_length * 4); // float
    rs_sgen_reg.wave_out = malloc(wave_length * 2); // signed short
    
    memset(rs_sgen_reg.wave, 0, waves_count * wave_length * 4);
    memset(rs_sgen_reg.wave_out, 0, wave_length * 2);
};

void rs_sgen_term() {
    free(rs_sgen_reg.wave);
    free(rs_sgen_reg.wave_out);
};

int wave_shot_index = 0;

void rs_sgen_wave_out(int index) {
    int i;
    for (i = 0; i < rs_sgen_reg.wave_length; i++) {
        rs_sgen_reg.wave_out[i] = 32767* (rs_clamp (rs_sgen_reg.wave[index*rs_sgen_reg.wave_length + i], -1.0, 1.0 ));
    };


//    char cmd[330];
//    memset(cmd, 0, 330);
//    sprintf(cmd, "/home/romik/temp/images/sound%d.data", wave_shot_index);
//
//    RS_IO_FILE* fp = rs_io_fopen( cmd, "wb");
//
//    rs_io_fwrite(fp, rs_sgen_reg.wave_out, rs_sgen_reg.wave_length*2);
//    rs_io_fclose(fp);
//
//    wave_shot_index++;



};

// --------------------


//float rs_sgen_osc_sin(int i, float freq, float p) {
//    //
//};




float phaser_alps_a1[6];
float phaser_alps_zm1[6];

float phaser_dmin, phaser_dmax; //range
float phaser_fb; //feedback
float phaser_lfoPhase;
float phaser_lfoInc;
float phaser_depth;
float phaser_sample_rate;
int   phaser_value_index = -1;

float phaser_zm1;

void phaser_set_range(float f1, float f2);
void phaser_set_rate(float f);
void phaser_alps_delay(int i, float f);
float phaser_alps_update(int i, float f);

void phaser_reset( float fb, float lfoPhase, float depth, float range_start, float range_end, float rate ) {
    memset(phaser_alps_a1, 0, 6*4);
    memset(phaser_alps_zm1, 0, 6*4);

    phaser_sample_rate = 44100.0; // !!!!

    phaser_fb = fb;
    phaser_lfoPhase = lfoPhase;
    phaser_depth = depth;
    phaser_zm1 = 0.0;
//    phaser_set_range( 440.f, 1600.f );
    phaser_set_range( range_start, range_end );
    phaser_set_rate( rate );

};

void phaser_set_range(float fMin, float fMax) { // Hz
    phaser_dmin = fMin / (phaser_sample_rate/2.f);
    phaser_dmax = fMax / (phaser_sample_rate/2.f);
};

void phaser_set_rate( float rate ){ // cps
    phaser_lfoInc = 2.0f * M_PI * (rate / phaser_sample_rate);
};

float phaser_update_sample( float inSamp, int ind ){
    //calculate and update phaser sweep lfo...
    float d;
    if (phaser_value_index == -1) {
        d  = phaser_dmin + (phaser_dmax-phaser_dmin) * ((sin( phaser_lfoPhase ) + 1.0f)/2.0f);
    }
    else {
        d = phaser_dmin + (phaser_dmax-phaser_dmin) * (0.5+0.5*rs_sgen_reg.wave[ phaser_value_index*rs_sgen_reg.wave_length + ind ]);
    };

    phaser_lfoPhase += phaser_lfoInc;
    if( phaser_lfoPhase >= M_PI * 2.0f )
        phaser_lfoPhase -= M_PI * 2.0f;

    //update filter coeffs
    int i;
    for(i = 0; i < 6; i++) {
        phaser_alps_delay(i, d);
    };

    //calculate output
    float y = phaser_alps_update(0,
                    phaser_alps_update(1,
                        phaser_alps_update(2,
                            phaser_alps_update(3,
                                phaser_alps_update(4,
                                    phaser_alps_update(5,
                                        inSamp + phaser_zm1 * phaser_fb
                                    )
                                )
                            )
                        )
                    )
              );

//    float y = 	_alps[0].Update(
//                 _alps[1].Update(
//                  _alps[2].Update(
//                   _alps[3].Update(
//                    _alps[4].Update(
//                     _alps[5].Update( inSamp + _zm1 * _fb ))))));
    phaser_zm1 = y;

//    return sin(440.0*phaser_lfoPhase);

    return inSamp + y * phaser_depth;
}

void phaser_alps_delay(int i, float delay) {
    phaser_alps_a1[i] = (1.0f - delay) / (1.0f + delay);
};

float phaser_alps_update(int i, float inSamp) {
    float y = inSamp * - phaser_alps_a1[i] + phaser_alps_zm1[i];
    phaser_alps_zm1[i] = y * phaser_alps_a1[i] + inSamp;
    return y;
};

// -----------------------


void rs_sgen_func_speaker(int index) {



    int i;

    float alpha = 0.3 ;// dt / (RC+dt)


    for (i = 0; i < rs_sgen_reg.wave_length; i++) {


        if (i == 0) {
            rs_sgen_reg.wave[ rs_sgen_reg.wave_length * index + i] = 0.4 * rs_noise(i, 0);
            continue;
        };


        // Low-pass
//        rs_sgen_reg.wave[ rs_sgen_reg.wave_length * index + i] =
//            alpha *  0.4 * rs_noise(i, 0) + (1.0 - alpha) * (rs_sgen_reg.wave[ rs_sgen_reg.wave_length * index + i - 1 ]);


        // High-pass
        rs_sgen_reg.wave[ rs_sgen_reg.wave_length * index + i] =
            alpha * ( rs_sgen_reg.wave[ rs_sgen_reg.wave_length * index + i - 1 ] + 0.4 * rs_noise(i, 0) - 0.4 * rs_noise(i-1, 0) );



////        rs_sgen_reg.wave[ rs_sgen_reg.wave_length * index + i] =  0.1 * sin( (2.0f * M_PI * 440.0 * i ) / 44100.0 + 9.0*sin(0.12*i) );
//        int t = i + 4*65536;
//        int p = (unsigned char) ((((t * (t >> 8 | t >> 9) & 46 & t >> 8)) ^ (t & t >> 13 | t >> 6)) & 0xFF);
////        rs_sgen_reg.wave[ rs_sgen_reg.wave_length * index + i] = (float)(p-128) / 128.0; // (float) 1.0 / 256.0 * (p);
    };

//    rs_sound_create_from_data(&game.test_sound, 688200, audiodata);

//    for (i = 0; i < 20; i++) {
//        rs_sound_create_from_data(& (sounds[i]), 11025 * (1 + i % 3) , audiodata2);
//        rs_sound_adjust_pitch( &sounds[i], 0.5 + 1.0f * i / 20.0f );
//    };

//    DEBUG10f("sound is created. length = %d \n", game.test_sound.Length);


//    memset(audiodata, 0, 688200);
//    free(audiodata);

};


void rs_sgen_func_noise(int index, int seed) {
    int i;
    for (i = 0; i < rs_sgen_reg.wave_length; i++) {
       rs_sgen_reg.wave[ rs_sgen_reg.wave_length * index + i] = rs_noise(seed + i, 0);
    };
};



void rs_sgen_func_sin(int index, float freq, float p) {
    int i;
    float f;

    for (i = 0; i < rs_sgen_reg.wave_length; i++) {
        f = sin( (2.0f * M_PI * freq * i ) / 44100.0 ); // !!! Only for 44100 kHz
        rs_sgen_reg.wave[ rs_sgen_reg.wave_length * index + i] = rs_sign(f) * pow( fabs(f) , p );  // remove koef!
        // rs_sgen_reg.wave[ rs_sgen_reg.wave_length * index + i] =  0.1 * sin( (2.0f * M_PI * 440.0 * i ) / 44100.0 + 9.0*sin(0.12*i) );
//        rs_sgen_reg.wave[ rs_sgen_reg.wave_length * index + i] = -1.0 + 2.0 * (  (44100.0 / freq)  ) sin( (2.0f * M_PI * freq * i ) / 44100.0 ); // !!! Only for 44100 kHz
    };
};



void rs_sgen_func_pm(int index, float freq, float p, float k, float freq2, float p2) {
    int i;
    float f;

    for (i = 0; i < rs_sgen_reg.wave_length; i++) {
        f = sin( (2.0f * M_PI * freq * i ) / 44100.0 + k*rs_pow(sin( 2.0f * M_PI * freq2 * i / 44100.0 ), p2) ); // !!! Only for 44100 kHz
        rs_sgen_reg.wave[ rs_sgen_reg.wave_length * index + i] = rs_pow(f, p);
        // rs_sgen_reg.wave[ rs_sgen_reg.wave_length * index + i] =  0.1 * sin( (2.0f * M_PI * 440.0 * i ) / 44100.0 + 9.0*sin(0.12*i) );
//        rs_sgen_reg.wave[ rs_sgen_reg.wave_length * index + i] = -1.0 + 2.0 * (  (44100.0 / freq)  ) sin( (2.0f * M_PI * freq * i ) / 44100.0 ); // !!! Only for 44100 kHz
    };
};

void rs_sgen_func_add(int dest, int src1, int src2, float k1, float k2) {
    int i;

    for (i = 0; i < rs_sgen_reg.wave_length; i++) {
        rs_sgen_reg.wave[ rs_sgen_reg.wave_length * dest + i] =
            k1 * rs_sgen_reg.wave[ rs_sgen_reg.wave_length * src1 + i]
            + k2 * rs_sgen_reg.wave[ rs_sgen_reg.wave_length * src2 + i];
    };

};

void rs_sgen_func_mult(int dest, int src1, int src2) {
    int i;

    for (i = 0; i < rs_sgen_reg.wave_length; i++) {
        rs_sgen_reg.wave[ rs_sgen_reg.wave_length * dest + i] =
            rs_sgen_reg.wave[ rs_sgen_reg.wave_length * src1 + i]
            * rs_sgen_reg.wave[ rs_sgen_reg.wave_length * src2 + i];
    };

};


void rs_sgen_func_normalize(int dest, float amp) {

//    DEBUG10("Normalize...");

    float val_max = 0.0; // fabs(rs_sgen_reg.wave[ rs_sgen_reg.wave_length * dest ]);
    float f;

    int i;  
    
    // Step 1: Normalize Mid-line
    
    const int mar_samples_count = 512;
    
    float *mar = malloc( 4 * (2 + rs_sgen_reg.wave_length / mar_samples_count) );
    memset(mar, 0, 4 * (2 + rs_sgen_reg.wave_length / mar_samples_count) );
    
//    DEBUG10("label 1");
    
    int length_512 = mar_samples_count*(rs_sgen_reg.wave_length/mar_samples_count); // 1024 for 1027
    
    int last_length = rs_sgen_reg.wave_length - length_512;
    if (!last_length) {
        last_length = length_512;
    };
    
    float koef[2] = { 1.0/mar_samples_count, 1.0/(last_length) }; 
    
//    DEBUG10f("\nkoef 0: %.6f\nkoef 1: %.6f (last_length = %d)\n", koef[0], koef[1], last_length);
    
    for (i = 0; i < rs_sgen_reg.wave_length; i++) {
        mar[1+i/mar_samples_count] += koef[ i / (length_512) ] * rs_sgen_reg.wave[ rs_sgen_reg.wave_length * dest + i];
    };
    
//    DEBUG10("label 2");
    
    
    for (i = 0; i < rs_sgen_reg.wave_length; i++) {
        rs_sgen_reg.wave[ rs_sgen_reg.wave_length * dest + i] -= //mar[i/mar_samples_count];
           rs_linear_interpolate( mar[i/mar_samples_count], mar[1+i/mar_samples_count], rs_fract(1.0*i/mar_samples_count) );
    };
//    
//    DEBUG10("label 3");
    
    free(mar);
    
//    DEBUG10("label 4");
    
    
    // Step 2: Normalize Amplitude

    for (i = 0; i < rs_sgen_reg.wave_length; i++) {
        f = rs_sgen_reg.wave[ rs_sgen_reg.wave_length * dest + i];
        val_max = rs_max(val_max, fabs(f) );
    };

    float val_scale = amp / val_max;

//    DEBUG10f("SGEN Normalize: val_max %.3f, val_scale = %.3f \n", val_max, val_scale);

    for (i = 0; i < rs_sgen_reg.wave_length; i++) {
        rs_sgen_reg.wave[ rs_sgen_reg.wave_length * dest + i] = val_scale * rs_sgen_reg.wave[ rs_sgen_reg.wave_length * dest + i];
    };
    
    

};



void rs_sgen_func_limiter(int dest, float val) {
    
    rs_sgen_func_normalize(dest, 1.0/val);
    
    int i;
    for (i = 0; i < rs_sgen_reg.wave_length; i++) {
        rs_sgen_reg.wave[ rs_sgen_reg.wave_length * dest + i] = rs_clamp(
            rs_sgen_reg.wave[ rs_sgen_reg.wave_length * dest + i ], -1.0, 1.0 );                                                                        
    };

//    float val_scale = amp / val_max;
//
//    DEBUG10f("SGEN Normalize: val_max %.3f, val_scale = %.3f \n", val_max, val_scale);
//
//    for (i = 0; i < rs_sgen_reg.wave_length; i++) {
//        rs_sgen_reg.wave[ rs_sgen_reg.wave_length * dest + i] = val_scale * rs_sgen_reg.wave[ rs_sgen_reg.wave_length * dest + i];
//    };
};


void rs_sgen_func_reverb(int dest, int src, int echo_delay, float echo_decay_koef) {
    
    //
    
    int i;
    for (i = 0; i < rs_sgen_reg.wave_length; i++) {
            
        if (i + echo_delay > rs_sgen_reg.wave_length-1) {
            break;
        };
            
        rs_sgen_reg.wave[ rs_sgen_reg.wave_length * dest + i + echo_delay] += 
            echo_decay_koef * rs_sgen_reg.wave[ rs_sgen_reg.wave_length * dest + i];
    };
    
};


void rs_sgen_func_lowpass(int dest, int src, float alpha_start, float alpha_end, float alpha_pow) {

    int i;
    float alpha, t;

    for (i = 0; i < rs_sgen_reg.wave_length; i++) {

        if (i == 0) {
            rs_sgen_reg.wave[ rs_sgen_reg.wave_length * dest + i] =
                0; // rs_sgen_reg.wave[ rs_sgen_reg.wave_length * src + i];
            continue;
        };

        t = (float) i / rs_sgen_reg.wave_length;
        alpha = (1.0 - t) * alpha_start + t * alpha_end;
        alpha = pow(alpha, alpha_pow);

        // Low-pass
        rs_sgen_reg.wave[ rs_sgen_reg.wave_length * dest + i] =
            alpha * rs_sgen_reg.wave[ rs_sgen_reg.wave_length * src + i]
            + (1.0 - alpha) * (rs_sgen_reg.wave[ rs_sgen_reg.wave_length * dest + i - 1 ]);

    };
};


void rs_sgen_func_highpass(int dest, int src, float alpha_start, float alpha_end, float alpha_pow) {

    int i;
    float t, alpha;

    for (i = 0; i < rs_sgen_reg.wave_length; i++) {

        if (i == 0) {
            rs_sgen_reg.wave[ rs_sgen_reg.wave_length * dest + i] = 0; // rs_sgen_reg.wave[ rs_sgen_reg.wave_length * src + i];
            continue;
        };

        t = (float) i / rs_sgen_reg.wave_length;
        alpha = (1.0 - t) * alpha_start + t * alpha_end;
        alpha = pow(alpha, alpha_pow);

        // High-pass
        rs_sgen_reg.wave[ rs_sgen_reg.wave_length * dest + i] =
            alpha * ( rs_sgen_reg.wave[ rs_sgen_reg.wave_length * dest + i - 1]
            + rs_sgen_reg.wave[ rs_sgen_reg.wave_length * src + i]
            - rs_sgen_reg.wave[ rs_sgen_reg.wave_length * src + i - 1] );

    };

};

void rs_sgen_func_phaser(int dest, int src, float fb, float lfoPhase, float depth, float range_start, float range_end, float rate) {

    //phaser_reset(0.97, 1.67, 0.5, 1.0, 22050.0, 1.5);
    phaser_reset(fb, lfoPhase, depth, range_start, range_end, rate);

    int i;
//    float t, alpha;

    for (i = 0; i < rs_sgen_reg.wave_length + 12; i++) {
        rs_sgen_reg.wave[ rs_sgen_reg.wave_length * dest + i%rs_sgen_reg.wave_length] = phaser_update_sample( rs_sgen_reg.wave[ rs_sgen_reg.wave_length * src + i%rs_sgen_reg.wave_length], i%rs_sgen_reg.wave_length );
    };

};

void rs_sgen_func_shift(int dest, int src) {
    int i;
    for (i = 0; i < rs_sgen_reg.wave_length; i++) {
        rs_sgen_reg.wave[ rs_sgen_reg.wave_length * dest + i % rs_sgen_reg.wave_length]
            = rs_sgen_reg.wave[ rs_sgen_reg.wave_length * src + (i + rs_sgen_reg.wave_length/2 )%rs_sgen_reg.wave_length];
    };
};


