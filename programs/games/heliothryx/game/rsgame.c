#include "rsgame.h"

#include "rsgametext.h"

#include "rsgamemenu.h"

#include "rsgamedraw.h"

#include "rskos.h"

#include "rsgentex.h"
#include "rssoundgen.h"
#include "rsnoise.h"

#include "rs/rsplatform.h"


#ifdef RS_USE_C_LIBS // linux version
    #include <math.h>
    #include <stdlib.h> 
    #include <string.h>
    
    #include "rs/rskeyboard.h"
#endif



rs_game_t game;


game_obj_t game_obj(int obj_type, int flags, int tag, int radius, float x, float y, int t, float f) {
    game_obj_t obj;
    obj.obj_type = obj_type;
    obj.flags = flags;
    obj.tag = tag;
    obj.radius = radius;
    obj.x = x;
    obj.y = y;
    obj.t = t;
    obj.f = f;
    return obj;
};

int game_obj_add(game_obj_t obj) {
    if (game.objs_count < GAME_OBJS_MAX_COUNT) {
        game.objs[game.objs_count++] = obj;
        return game.objs_count-1;    
    };
    #ifdef RS_LINUX
        DEBUG10("Error, max objects count is reached");
    #endif
    return 0; // Max objects count is reached
};

void game_obj_remove(int index) {
    if (index == game.objs_count - 1) {
        game.objs_count--;
        return;
    };
    game.objs[index] = game.objs[ game.objs_count-1 ];
    game.objs_count--;
};









void texture_init(rs_texture_t *tex, int w, int h) {
    tex->status = 1;
    tex->w = w;
    tex->h = h;
    tex->data = malloc(w*h*4); // BGRA BGRA
};

void texture_free(rs_texture_t *tex) {
    free(tex->data);
    tex->status = 0;
};

void texture_clear(rs_texture_t *tex, unsigned int color) {
    int i;
    for (i = 0; i < tex->w * tex->h; i++) {
        *((unsigned int*)(&tex->data[i*4])) = color;
    };
};

void texture_draw(rs_texture_t *dest, rs_texture_t *src, int x, int y, int mode) {
    
    int i; // y
    int j; // x
    int k; // color component
    
    int istart = (y < 0) ? -y : 0;
    int iend = src->h - (( (y + src->h) > dest->h) ? (y + src->h - dest->h) : 0);
    
    int jstart = (x < 0) ? -x : 0;
    int jend = src->w - (( (x + src->w) > dest->w) ? (x + src->w - dest->w) : 0);
    
    int ishift = 0;
    int jshift = 0;
    
    float a; // alpha value
    
    if (mode & DRAW_TILED_FLAG) {
        jshift = x;
        ishift = y;
        x = y = istart = jstart = 0;
        iend = dest->h;
        jend = dest->w;
    };
    
    mode = mode & DRAW_MODE_MASK;
    
    int modvalue = (src->w*src->h*4); 
    
    if (mode == DRAW_MODE_REPLACE) {
        for (i = istart; i < iend; i++) {
            for (j = jstart; j < jend; j++) {
                for (k = 0; k < 4; k++) {
                    dest->data[ 4 * ( (y+i)*dest->w + (x+j) ) + k ] = src->data[ (4*(i*src->w + j) + k) % modvalue ];
                };
            };
        };
    }
    else if (mode == DRAW_MODE_ADDITIVE) {
        for (i = istart; i < iend; i++) {
            for (j = jstart; j < jend; j++) {
                for (k = 0; k < 3; k++) { // Alpha channel is not added
                    dest->data[ 4 * ( (y+i)*dest->w + (x+j) ) + k ] = 
                        clamp_byte( dest->data[ 4 * ( (y+i)*dest->w + (x+j) ) + k ] + src->data[ (4*((i+ishift)*src->w + j + jshift) + k) % modvalue] );
                };
            };
        };
    }
    else if (mode == DRAW_MODE_ALPHA) {
        for (i = istart; i < iend; i++) {
            for (j = jstart; j < jend; j++) {
                for (k = 0; k < 3; k++) {
                    a = (1.0 * src->data[ (4*(i*src->w + j) + 3) % modvalue ] / 255.0);
                    dest->data[ 4 * ( (y+i)*dest->w + (x+j) ) + k ] = 
                         (unsigned char) ( (1.0-a) * dest->data[ 4 * ( (y+i)*dest->w + (x+j) ) + k ] 
                                          + a*src->data[ (4*((i+ishift)*src->w + j + jshift) + k) % modvalue] );
                };
            };
        };
    };
    
};



void texture_set_pixel(rs_texture_t *tex, int x, int y, unsigned int color) {
    *((unsigned int*) &tex->data[ 4 * ( (y)*tex->w + (x) ) + 0 ]) = color;
};

void texture_draw_vline(rs_texture_t *tex, int x, int y, int l, unsigned int color) {
    int i;
    if (y+l >= tex->h) {
        l = tex->h - y;
    };
    
    if (x < 0) {
        return;
    };
    
    if (x >= tex->w) {
        return;
    };
    
    for (i = 0; i < l; i++) {
        *((unsigned int*) &tex->data[ 4 * ( (y+i)*tex->w + (x) ) + 0 ]) = color;
    };    
};



void soundbuf_init(rs_soundbuf_t *snd, int length_samples) {
    snd->status = 1;
    snd->length_samples = length_samples;
    snd->data = malloc(length_samples*2);
    rskos_snd_create_buffer(&snd->hbuf, snd->data, length_samples);
};

void soundbuf_update(rs_soundbuf_t *snd) {
    rskos_snd_update_buffer(&snd->hbuf, snd->data, snd->length_samples);
};

void soundbuf_free(rs_soundbuf_t *snd) {
    snd->status = 0;
    free(snd->data);
};

void soundbuf_fill(rs_soundbuf_t *snd, int amp, int freq_div) {
    int i;
    for (i = 0; i < snd->length_samples; i++) {
		snd->data[i] = -amp/2 + amp/2*( ( (i % freq_div) > freq_div/2 ) ? 1 : 0 );
	};
	rskos_snd_update_buffer(&snd->hbuf, snd->data, snd->length_samples);
};

void soundbuf_sin(rs_soundbuf_t *snd, float freq) {
    int i;
    int amp = 19000;
    for (i = 0; i < snd->length_samples; i++) {
		snd->data[i] = ( 1.0 - 1.0*i/snd->length_samples ) * sin(freq*i) * amp;
	};
	rskos_snd_update_buffer(&snd->hbuf, snd->data, snd->length_samples);
};

void soundbuf_sin_fade(rs_soundbuf_t *snd, float freq) {
    int i;
    int amp = 19000;
    for (i = 0; i < snd->length_samples; i++) {
		snd->data[i] = ( 1.0 - 1.0*i/snd->length_samples ) * sin( ( (1.0 - 0.48*i/snd->length_samples) * freq ) *i) * amp;
	};
	
	rskos_snd_update_buffer(&snd->hbuf, snd->data, snd->length_samples);
};

void soundbuf_play(rs_soundbuf_t *snd, int mode) {
    rskos_snd_play(&snd->hbuf, mode);
};

void soundbuf_stop(rs_soundbuf_t *snd) {
    rskos_snd_stop(&snd->hbuf);
};

void soundbuf_loop_check(rs_soundbuf_t *snd) {
    rskos_snd_check_loop(&snd->hbuf);
};



unsigned char clamp_byte(int value) {
    value = value * (1 - (value >> 31)); // negative to zero
    return (value > 255) ? 255 : value;
};


void game_reg_init() {
    
    game.player_x = 0;
    game.player_y = 0;
    game.tz = 0;
    
    game.bg_color = COLOR_BLACK;

    game.objs = malloc( sizeof(game_obj_t) * GAME_OBJS_MAX_COUNT ); 
    
    game.status = STATUS_MENU;
    
    game.window_scale = 2;
    #ifndef RS_KOS
        game.window_scale = 3;
        window_scale_str[3] = '3'; 
    #endif
    
    game.keyboard_state = 0;
    
    game.menu_index = 0;
    game.menu_item_index = 0;
};


int is_key_pressed(int mask) {
    return IS_BIT_SET(game.keyboard_state, mask) ? 1 : 0;
};


int seed = 0;

unsigned short rs_rand() {
    seed += 1000;
    seed %= 56789;
    
    // from here, http://www.cplusplus.com/forum/general/85758/
    // koef. changed
    int n = rskos_get_time() + seed * 57 * 5; // no *2
    n = (n << 13) ^ n;
    return (n * (n * n * 15731 + 789221) + 1376312589) & 0xFFFF;
    
};




void GameInit() {

    game_reg_init();
    
    game.scaled_framebuffer = malloc(GAME_WIDTH*game.window_scale * GAME_HEIGHT*game.window_scale * 3);
    DEBUG10f("scaled framebuffer: %d (window_scale = %d) \n", game.window_scale * GAME_WIDTH * GAME_HEIGHT * 3, game.window_scale);
    
    game_font_init();
    
    texture_init(&game.framebuffer, GAME_WIDTH, GAME_HEIGHT);
    
    texture_init(&game.tex, 64, 64);
    rs_gen_init(1, 64);
    rs_gen_func_set(0, 0.0);
    rs_gen_func_cell(0, 1200, 10, NULL, 1.0, 1.0, 1.0, 1.0, 0.0, 1.0);
    rs_gen_func_normalize(0, 0.0, 1.0);
    rs_gen_func_posterize(0, 5);
    rs_gen_tex_out_rgba(0, 0, 0, -1, 1.0, 1.0, 1.0, 1.0);
    memcpy(game.tex.data, rs_gen_reg.tex_out, 64*64*4 );
    rs_gen_term();
    
    texture_init(&game.tex_clouds, 128, 128);
    rs_gen_init(1, 128);
    rs_gen_func_perlin(0, 8, 5, 0.5, 1100);
    rs_gen_func_normalize(0, 0.0, 1.0);
    rs_gen_func_posterize(0, 4);
    rs_gen_func_normalize(0, 0.0, 0.50);
    rs_gen_tex_out_rgba(0, 0, 0, -1, 0.9, 0.7, 0.5, 1.0);
    memcpy(game.tex_clouds.data, rs_gen_reg.tex_out, 128*128*4 );
    rs_gen_term();
    
    texture_init(&game.tex_ground, GAME_WIDTH, 50);
    
    
    
    int ship_size = 8;
    
    // 16x8
    
    unsigned char tex_ship1_mask[] = { 0x10, 0x58, 0x7C, 0x7C, 0x3E, 0x1E, 0xBE, 0xFE };
    unsigned char tex_ship2_mask[] = { 0x7C, 0x7C, 0x7E, 0x7E, 0x3C, 0x3C, 0x1C, 0x18 };
    unsigned char tex_ship1_overlay_mask[] = { 0x10, 0x18, 0x1C, 0x1C, 0x1C, 0x1C, 0x8C, 0x84 };
    unsigned char tex_ship2_overlay_mask[] = { 0x00, 0x00, 0x20, 0x20, 0x30, 0x30, 0x10, 0x10 };
    
    texture_init(&game.tex_ship[0], ship_size, ship_size);
    rs_gen_init(2, ship_size);
    rs_gen_func_perlin(0, 8, 5, 0.5, 111);
    rs_gen_func_normalize(0, 0.0, 1.0);
    rs_gen_func_posterize(0, 2);
    rs_gen_func_mult_add_value(0, 0, 0.3, 0.7);

    rs_gen_func_set(1, 1.0);
    rs_gen_func_apply_mask(1, tex_ship1_mask);
//    rs_gen_func_mult_add_value(1, 1, 0.8, 0.2);
    rs_gen_tex_out_rgba(0, 0, 0, 1, 0.8, 0.65, 0.66, 1.0);
    memcpy(game.tex_ship[0].data, rs_gen_reg.tex_out, ship_size*ship_size*4 );
    rs_gen_term();
    
    texture_init(&game.tex_ship[1], ship_size, ship_size);
    rs_gen_init(2, ship_size);
    rs_gen_func_perlin(0, 8, 5, 0.5, 1100);
    rs_gen_func_normalize(0, 0.0, 1.0);
    rs_gen_func_posterize(0, 2);
    rs_gen_func_mult_add_value(0, 0, 0.1, 0.9);

    rs_gen_func_set(1, 1.0);
    rs_gen_func_apply_mask(1, tex_ship1_overlay_mask);
//    rs_gen_func_mult_add_value(1, 1, 0.8, 0.2);
    rs_gen_tex_out_rgba(0, 0, 0, 1, 0.4, 0.3, 0.3, 1.0);
    memcpy(game.tex_ship[1].data, rs_gen_reg.tex_out, ship_size*ship_size*4 );
    rs_gen_term();
    
    
    texture_init(&game.tex_ship[2], ship_size, ship_size);
    rs_gen_init(2, ship_size);
    rs_gen_func_perlin(0, 8, 5, 0.5, 111);
    rs_gen_func_normalize(0, 0.0, 1.0);
    rs_gen_func_posterize(0, 2);
    rs_gen_func_mult_add_value(0, 0, 0.3, 0.7);
//    rs_gen_func_set(0, 1.0);
    rs_gen_func_set(1, 1.0);
    rs_gen_func_apply_mask(1, tex_ship2_mask);
//    rs_gen_func_mult_add_value(1, 1, 0.8, 0.2);
    rs_gen_tex_out_rgba(0, 0, 0, 1, 0.8, 0.65, 0.66, 1.0);
    memcpy(game.tex_ship[2].data, rs_gen_reg.tex_out, ship_size*ship_size*4 );
    rs_gen_term();
    
    texture_init(&game.tex_ship[3], ship_size, ship_size);
    rs_gen_init(2, ship_size);
    rs_gen_func_perlin(0, 8, 5, 0.5, 1100);
    rs_gen_func_normalize(0, 0.0, 1.0);
    rs_gen_func_posterize(0, 2);
    rs_gen_func_mult_add_value(0, 0, 0.1, 0.9);

    rs_gen_func_set(1, 1.0);
    rs_gen_func_apply_mask(1, tex_ship2_overlay_mask);
//    rs_gen_func_mult_add_value(1, 1, 0.8, 0.2);
    rs_gen_tex_out_rgba(0, 0, 0, 1, 0.4, 0.3, 0.2, 1.0);
    memcpy(game.tex_ship[3].data, rs_gen_reg.tex_out, ship_size*ship_size*4 );
    rs_gen_term(); 
    
    
    texture_init(&game.tex_gui_line, GAME_WIDTH, 13);
    int i;
    for (i = 0; i < GAME_WIDTH*13; i++) {
        ( (unsigned int*) (game.tex_gui_line.data)) [i] = 0x668899AA;
    };
    
    
    int rock_size = 32;
    rs_gen_init(3, rock_size);
    for (i = 0; i < ROCKS_COUNT; i++) {
    
        texture_init(&(game.tex_rocks[i]), rock_size, rock_size);

        rs_gen_func_set(0, 0.0);
        rs_gen_func_radial(0, 0.5, 0.5, 0.5, 0.75, 2.5 + i%5);

        rs_gen_func_perlin(2, 33, 4, 0.5, 350+i);
        rs_gen_func_normalize(2, 0.0, 1.0);
        rs_gen_func_posterize(2, 4);
        
        rs_gen_func_cell(1, 410+i, 50, NULL, -2.0, 1.0, 1.0, 1.0, 0.0, 1.0);
        rs_gen_func_posterize(1, 2);
        rs_gen_func_normalize(1, 0.0, 1.0);
        rs_gen_func_add(1, 1, 2, 1.0, 0.5);
        rs_gen_func_normalize(1, 0.0, 1.0);
        rs_gen_func_posterize(1, 4);

        rs_gen_func_add(1, 0, 1, 1.0, 1.0);
        rs_gen_func_normalize(1, 0.0, 1.0);
        rs_gen_func_mult(1, 0, 1);
        rs_gen_func_normalize(1, 0.0, 1.0);
        rs_gen_func_posterize(1, 4);
        rs_gen_tex_out_rgba_set(0.0, 0.0, 0.0, 0.0);
        rs_gen_tex_out_rgba(1, 1, 1, -1, 0.5+ 0.03*(i%2), 0.7+ 0.03*(i%3) , 0.9, 1.0);

        memcpy(game.tex_rocks[i].data, rs_gen_reg.tex_out, rock_size*rock_size*4 );
    };
    rs_gen_term();
    
    
    rock_size = 16;
    rs_gen_init(3, rock_size);
    for (i = 0; i < MINIROCKS_COUNT; i++) {
    
        texture_init(&(game.tex_minirocks[i]), rock_size, rock_size);

        rs_gen_func_set(0, 0.0);
        rs_gen_func_radial(0, 0.5, 0.5, 0.5, 0.75, 2.5 + i%5);

        rs_gen_func_perlin(2, 33, 4, 0.5, 350+i);
        rs_gen_func_normalize(2, 0.0, 1.0);
        rs_gen_func_posterize(2, 4);
        
        rs_gen_func_cell(1, 410+i, 50, NULL, -2.0, 1.0, 1.0, 1.0, 0.0, 1.0);
        rs_gen_func_posterize(1, 2);
        rs_gen_func_normalize(1, 0.0, 1.0);
        rs_gen_func_add(1, 1, 2, 1.0, 0.5);
        rs_gen_func_normalize(1, 0.0, 1.0);
        rs_gen_func_posterize(1, 4);

        rs_gen_func_add(1, 0, 1, 1.0, 1.0);
        rs_gen_func_normalize(1, 0.0, 1.0);
        rs_gen_func_mult(1, 0, 1);
        rs_gen_func_normalize(1, 0.0, 1.0);
        rs_gen_func_posterize(1, 4);
        rs_gen_tex_out_rgba_set(0.0, 0.0, 0.0, 0.0);
        rs_gen_tex_out_rgba(1, 1, 1, -1, 0.7+ 0.01*(i%2), 0.7+ 0.01*(i%3) , 0.65, 1.0);

        memcpy(game.tex_minirocks[i].data, rs_gen_reg.tex_out, rock_size*rock_size*4 );
    };
    rs_gen_term();
    
    
    
    
    rs_gen_init(3, EXPLOSION_RADIUS*2);
    for (i = 0; i < EXPLOSIONS_COUNT; i++) {
            
        texture_init(&(game.tex_explosions[i]), EXPLOSION_RADIUS*2, EXPLOSION_RADIUS*2);

        rs_gen_func_set(0, 1.0);
//        rs_gen_func_radial(0, 0.5, 0.5, 0.3 + 0.5*i/EXPLOSION_FRAMES_COUNT, 0.975, 4.0);
//        rs_gen_func_set(0, 1.0);

        rs_gen_func_perlin(2, 10 + i, 7, 0.5, 100 + i*1000);
        rs_gen_func_normalize(2, 0.0, 1.0);

        rs_gen_func_set(1, 0.0);
        rs_gen_func_radial(1, 0.5, 0.5, 0.1 + 0.4*i/EXPLOSIONS_COUNT, 1.0 - 1.0*i/EXPLOSIONS_COUNT, 2.5 + i%5);
        
        rs_gen_func_add(0, 0, 2, 1.0, -0.8);

        rs_gen_tex_out_rgba_set( 0.0, 0.0, 0.0, 0.0);
        rs_gen_tex_out_rgba(0, 0, 0, 1, 0.5*i/EXPLOSIONS_COUNT, 1.0 - 0.7*i/EXPLOSIONS_COUNT, 1.0, 1.0);

        memcpy(game.tex_explosions[i].data, rs_gen_reg.tex_out, EXPLOSION_RADIUS*2*EXPLOSION_RADIUS*2*4 );
    };
    rs_gen_term();
    
    
    
    rs_gen_init(3, HUGE_EXPLOSION_RADIUS*2);
    for (i = 0; i < HUGE_EXPLOSIONS_COUNT; i++) {
            
        texture_init(&(game.tex_huge_explosions[i]), HUGE_EXPLOSION_RADIUS*2, HUGE_EXPLOSION_RADIUS*2);

        rs_gen_func_set(0, 1.0);
//        rs_gen_func_radial(0, 0.5, 0.5, 0.3 + 0.5*i/EXPLOSION_FRAMES_COUNT, 0.975, 4.0);
//        rs_gen_func_set(0, 1.0);

        rs_gen_func_perlin(2, 10 + i, 7, 0.5, 500 + i*2000);
        rs_gen_func_normalize(2, 0.0, 1.0);

        rs_gen_func_set(1, 0.0);
        rs_gen_func_radial(1, 0.5, 0.5, 0.1 + 0.4*i/HUGE_EXPLOSIONS_COUNT, 1.0 - 1.0*i/HUGE_EXPLOSIONS_COUNT, 2.5 + i%5);
        
        rs_gen_func_add(0, 0, 2, 1.0, -0.8);

        rs_gen_tex_out_rgba_set( 0.0, 0.0, 0.0, 0.0);
        rs_gen_tex_out_rgba(0, 0, 0, 1, 0.88*i/HUGE_EXPLOSIONS_COUNT, 0.8 - 0.8*i/HUGE_EXPLOSIONS_COUNT, 1.0, 1.0);

        memcpy(game.tex_huge_explosions[i].data, rs_gen_reg.tex_out, HUGE_EXPLOSION_RADIUS*2*HUGE_EXPLOSION_RADIUS*2*4 );
    };
    rs_gen_term();
    
    

    #ifndef RS_KOS
        rs_audio_init(RS_AUDIO_FMT_MONO16, RS_AUDIO_FREQ_16000, 2); 
    #endif

    soundbuf_init(&game.sound_shoot, 1536);
 
	rs_sgen_init(2, game.sound_shoot.length_samples);
	rs_sgen_func_pm(1, 2900.0, 1.70, 65.0, 17.0, 1.0);
	rs_sgen_func_normalize(1, 0.6);
//	rs_sgen_func_lowpass(0, 1, 1.0, 0.0, 4.0);
	rs_sgen_func_highpass(0, 1, 1.0, 0.0, 3.0);

	rs_sgen_wave_out(0);
	
	memcpy(game.sound_shoot.data, (unsigned char*) rs_sgen_reg.wave_out, game.sound_shoot.length_samples*2 );
	
	rs_sgen_term();
    soundbuf_update(&game.sound_shoot);
    
    
    
    soundbuf_init(&game.sound_turret_shoot, 4096);
	rs_sgen_init(2, game.sound_turret_shoot.length_samples);
	rs_sgen_func_pm(1, 227.0, 4.70, 555.0, 150.0, 0.01);
	rs_sgen_func_normalize(1, 0.6);
//	rs_sgen_func_highpass(0, 1, 1.0, 0.0, 3.0);
	rs_sgen_func_lowpass(0, 1, 1.0, 0.0, 3.0);

	rs_sgen_wave_out(0);
	
	memcpy(game.sound_turret_shoot.data, (unsigned char*) rs_sgen_reg.wave_out, game.sound_turret_shoot.length_samples*2 );
	
	rs_sgen_term();
    soundbuf_update(&game.sound_turret_shoot);
    
    
     
    
    
    
    
    

    soundbuf_init(&game.sound_test2, 1024);
    //soundbuf_fill(&game.sound_test2, 8, 40);
    soundbuf_sin(&game.sound_test2, 0.48);

    soundbuf_init(&game.sound_test3, 1024);
    //soundbuf_fill(&game.sound_test3, 12, 60);
    soundbuf_sin(&game.sound_test3, 0.24);



    
    
    int soundlen = 55000;
            
//    int freqs[SOUND_EXPLOSIONS_COUNT] = { 440, 523, 587, 698, 783, 880, 1046, 1174 };
    
    for (i = 0; i < SOUND_EXPLOSIONS_COUNT; i++) {

        soundbuf_init(&game.sound_explosions[i], soundlen);

        rs_sgen_init(3, soundlen);

        rs_sgen_func_noise(2, 1000);
        //rs_sgen_func_phaser(0, 2, 0.9, 15.2 + 1.0*i/SOUND_EXPLOSIONS_COUNT, 6.0, 3.0, 2000.0, 1.73); 
        rs_sgen_func_phaser(0, 2, 0.9, 16.2 + 0.5*i/SOUND_EXPLOSIONS_COUNT, 6.0, 3.0, 900.0, 0.93); 
        rs_sgen_func_normalize(0, 1.0);

        rs_sgen_func_lowpass(2, 0, 0.6, 0.0, 20.0);
        rs_sgen_func_normalize(2, 0.5);

        rs_sgen_wave_out(2);

        memcpy(game.sound_explosions[i].data, (unsigned char*) rs_sgen_reg.wave_out, soundlen*2);
        soundbuf_update(&game.sound_explosions[i]);

        rs_sgen_term();
    
    };
    
    
    
    soundlen = 95000;
    
    soundbuf_init(&game.sound_huge_explosion, soundlen);

    rs_sgen_init(3, soundlen);

    rs_sgen_func_noise(2, 1000);
    //rs_sgen_func_phaser(0, 2, 0.9, 15.2 + 1.0*i/SOUND_EXPLOSIONS_COUNT, 6.0, 3.0, 2000.0, 1.73); 
    rs_sgen_func_phaser(0, 2, 0.9, 16.2 + 0.5, 6.0, 3.0, 100.0, 0.53); 
    rs_sgen_func_normalize(0, 1.0);

    rs_sgen_func_lowpass(2, 0, 0.8, 0.0, 10.0);
    rs_sgen_func_normalize(2, 0.5);

    rs_sgen_wave_out(2);

    memcpy(game.sound_huge_explosion.data, (unsigned char*) rs_sgen_reg.wave_out, soundlen*2);
    soundbuf_update(&game.sound_huge_explosion);

    rs_sgen_term();
    
    
    
    
    
    
    
    

    soundlen = 17888;

    soundbuf_init(&game.sound_hit, soundlen);

    rs_sgen_init(3, soundlen);

    rs_sgen_func_noise(2, 1000);
    rs_sgen_func_phaser(0, 2, 0.9, 11.5, 16.0, 13.0, 1300.0, 1.93); 
    rs_sgen_func_normalize(0, 1.0);

    rs_sgen_func_highpass(2, 0, 1.0, 0.3, 20.0);
    rs_sgen_func_normalize(2, 0.5);

    rs_sgen_wave_out(2);

    memcpy(game.sound_hit.data, (unsigned char*) rs_sgen_reg.wave_out, soundlen*2);
    soundbuf_update(&game.sound_hit);

    rs_sgen_term();
    
    
    

    #define NOTE(i) ( 3 << ( (i)/12) ) / ( 24 - ( (i) % 12) )
    int amp = 70;
    int t_shift = 0;
    int t;

    soundlen = 128 * 1024; 
    


    soundbuf_init(&game.sound_music, soundlen);
    
    for (t = t_shift; t < soundlen+t_shift; t++) {
        game.sound_music.data[t-t_shift] = (0xFF & 
            (                                        
                ((t>>11) | (t>>7) | ( t>>5) | (t))

            )                                   
        ) * amp;
    };
    
    soundbuf_update(&game.sound_music);
    
    
    
//    int d[4] = { 5, 6, 1, 2 };
    
    
//    soundbuf_init(&game.sound_music2, soundlen);
//    
//    for (t = t_shift; t < soundlen+t_shift; t++) {
//            
////        y = 1 + (t & 16383);
////        x = (t * c[ (t>>13) & 3 ] / 24) & 127;
//            
//        game.sound_music2.data[t-t_shift] = (0xFF & 
//            (                                        
//           //( t*5 & t >> 7 ) | ( t*2 & t >> 10 )
//                                                    
//           // ( ((t*t*t/1000000 + t) % 127) | t>>4 | t>>5 | (t%127) ) + ( (t>>16) | t )
//           
////           ((t>>11) | (t>>7) | ( t>>5) | (t))
////           //+ 
////           //(( (t*5) >>12) & ( (t*3)>>19))
//             
////                (3000 / y) * 35
////             + x*y*40000
////             + ( ( ((t>>8) & (t>>10)) | (t >> 14) | x) & 63 )
//                                
//                 // (  ((6 * t / d[ (t>>13) & 15 ] ) & 127) * 10000 ) 
//                 //|( ( t>>3 ) )                
//             
//                    (t*NOTE( d[ (t>>13) & 3 ] )*10000)
//                    | ((t>>6)*20000) 
//             
//             )
//                                                    
//        ) * amp;
//    };
//    
//    soundbuf_update(&game.sound_music2);
    
    
    soundbuf_play( &game.sound_music, SND_MODE_LOOP );    
    

};


void GameTerm() {


    DEBUG10("--- Game Term ---");

    #ifndef RS_KOS
        rs_audio_term();
    #endif

    game_font_term();
    
    free(game.scaled_framebuffer);
    
    free(game.objs);
    
    texture_free(&game.framebuffer);
    texture_free(&game.tex);
    texture_free(&game.tex_clouds);
    texture_free(&game.tex_ground);
    
    texture_free(&game.tex_gui_line);
    
    int i;
    for (i = 0; i < ROCKS_COUNT; i++) {
        texture_free(&game.tex_rocks[i]);
    };
    
    soundbuf_free(&game.sound_huge_explosion);
    soundbuf_free(&game.sound_hit);
    soundbuf_free(&game.sound_music);
    
    for (i = 0; i < SOUND_EXPLOSIONS_COUNT; i++) {
        soundbuf_free(&game.sound_explosions[i]);
    };
    
    
    soundbuf_free(&game.sound_shoot);
    soundbuf_free(&game.sound_turret_shoot);
    soundbuf_free(&game.sound_test2);
    soundbuf_free(&game.sound_test3);
    

};

// ------------ #Event Functions ------------






void GameKeyDown(int key) {
    
    
    switch (key) {
        case RS_KEY_LEFT:
            BIT_SET(game.keyboard_state, RS_ARROW_LEFT_MASK);
            break;
        case RS_KEY_RIGHT:
            BIT_SET(game.keyboard_state, RS_ARROW_RIGHT_MASK);
            break;
        case RS_KEY_UP:
            BIT_SET(game.keyboard_state, RS_ARROW_UP_MASK);
            break;
        case RS_KEY_DOWN:
            BIT_SET(game.keyboard_state, RS_ARROW_DOWN_MASK);
            break;
        case RS_KEY_A:
            BIT_SET(game.keyboard_state, RS_ATTACK_KEY_MASK);
            game.shoot_keypressed = 1;
            
//            soundbuf_loop_check( &game.sound_music );
            
            break;
            
        
        
        case RS_KEY_SPACE:
                soundbuf_play( &game.sound_huge_explosion, 0 );
            break;
        
        #ifdef RS_LINUX
            
        case RS_KEY_Z:
//                soundbuf_stop( &game.sound_music );
//                soundbuf_play( &game.sound_music2, 0 );
                game.stage = 7;
                break;
            
        #endif
        

            
    };
    
    
    if (game.status == STATUS_MENU) {
    
        switch (key) {
            case RS_KEY_LEFT:
                BIT_SET(game.keyboard_state, RS_ARROW_LEFT_MASK);

                if ( (game.menu_index == MENU_SETTINGS) && (game.menu_item_index == MENU_ITEM_WINDOW_SCALE) ) {
                    game_change_window_scale(-1);
                    game_ding(1);
                };

                //PlayBuffer(hBuff, 0);
                break;
            case RS_KEY_RIGHT:
                BIT_SET(game.keyboard_state, RS_ARROW_RIGHT_MASK);
                
                if ( (game.menu_index == MENU_SETTINGS) && (game.menu_item_index == MENU_ITEM_WINDOW_SCALE) ) {
                    game_change_window_scale(1);
                    game_ding(1);
                };

                //StopBuffer(hBuff);
                break;
            case RS_KEY_UP:
                BIT_SET(game.keyboard_state, RS_ARROW_UP_MASK);
                menu_cursor_up();
                //ResetBuffer(hBuff, 0);
                break;
            case RS_KEY_DOWN:
                BIT_SET(game.keyboard_state, RS_ARROW_DOWN_MASK);
                menu_cursor_down();
                break;
            case RS_KEY_RETURN:
                menu_cursor_click();
                break;
            case RS_KEY_ESCAPE:
                if (game.menu_index != MENU_PAUSE) {
                    menu_open(0);
                }
                else {
                    game.status = STATUS_PLAYING;
                    return;
                };
                break;
        };

    };
    
    if (game.status == STATUS_PLAYING) {
        switch (key) {
            
            case RS_KEY_ESCAPE:
                game.status = STATUS_MENU;
                menu_open(MENU_PAUSE);
                break;
            case RS_KEY_SPACE:
                
                
                //game_obj_add( game_obj( OBJ_EXPLOSION, 0, 0, 0, game.tx + 80, game.ty - 10, 0, 0.0 ) );
                
//                game_obj_add( game_obj( OBJ_ROCK, 0, 0, 32, game.tx + 80, game.ty - 10, 0, 0.0 ) );
                
                break;
            
        };
    };

};

void GameKeyUp(int key) {

    switch (key) {
        case RS_KEY_LEFT:
            BIT_CLEAR(game.keyboard_state, RS_ARROW_LEFT_MASK);
            break;
        case RS_KEY_RIGHT:
            BIT_CLEAR(game.keyboard_state, RS_ARROW_RIGHT_MASK);
            break;
        case RS_KEY_UP:
            BIT_CLEAR(game.keyboard_state, RS_ARROW_UP_MASK);
            break;
        case RS_KEY_DOWN:
            BIT_CLEAR(game.keyboard_state, RS_ARROW_DOWN_MASK);
            break;
        case RS_KEY_A:
            BIT_CLEAR(game.keyboard_state, RS_ATTACK_KEY_MASK);
            break;
    };

};

void GameMouseDown(int x, int y) {
//    game.tx = x;
//    game.ty = y;
    DEBUG10f("Mouse Down %d, %d \n", x, y);
};

void GameMouseUp(int x, int y) {
    //
};


void game_change_window_scale(int d) {
    int scale = window_scale_str[3] - '0';
    
    unsigned int w;
    unsigned int h;
    rskos_get_screen_size(&w, &h);
    
    int max_scale = (w-20)/GAME_WIDTH;
    if ( (h-20)/GAME_HEIGHT < max_scale ) {
        max_scale = (h-20)/GAME_HEIGHT;
    };
    
    scale += d;
    if ( scale > max_scale) {
        scale = 1;
    }
    else if (scale < 1) {
        scale = max_scale;
    };
    
    game.window_scale = scale;
    
    free(game.scaled_framebuffer);
    game.scaled_framebuffer = malloc(GAME_WIDTH*game.window_scale * GAME_HEIGHT*game.window_scale * 3);
    
    
    rskos_resize_window( GAME_WIDTH * scale, GAME_HEIGHT * scale );

    window_scale_str[3] = scale + '0';
    
    
    
};

void game_ding(int i) {
    
    switch (i) {
        case 0:
            soundbuf_play(&game.sound_test2, 0);
            break;
        case 1:
            soundbuf_play(&game.sound_test3, 0);
            break;
    };
    
};
