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
    int amp = 29000;
    for (i = 0; i < snd->length_samples; i++) {
		snd->data[i] = ( 1.0 - 1.0*i/snd->length_samples ) * sin(freq*i) * amp;
	};
	rskos_snd_update_buffer(&snd->hbuf, snd->data, snd->length_samples);
};

void soundbuf_sin_fade(rs_soundbuf_t *snd, float freq) {
    int i;
    int amp = 29000;
    for (i = 0; i < snd->length_samples; i++) {
		snd->data[i] = ( 1.0 - 1.0*i/snd->length_samples ) * sin( ( (1.0 - 0.48*i/snd->length_samples) * freq ) *i) * amp;
	};
	rskos_snd_update_buffer(&snd->hbuf, snd->data, snd->length_samples);
};

void soundbuf_play(rs_soundbuf_t *snd) {
    rskos_snd_play(&snd->hbuf, 0);
};

void soundbuf_stop(rs_soundbuf_t *snd) {
    rskos_snd_stop(&snd->hbuf);
};



unsigned char clamp_byte(int value) {
    value = value * (1 - (value >> 31)); // negative to zero
    return (value > 255) ? 255 : value;
};


void game_reg_init() {
    game.tx = 0;
    game.ty = 0;
    game.tz = 0;

    int i;
    for (i = 0; i < BULLETS_COUNT; i++) {
        game.bullet_x[i] = 0;
        game.bullet_y[i] = 0;
    };
    game.bullet_index = 0;
    
    game.status = STATUS_MENU;
    
    game.window_scale = 2;
    #ifdef RS_LINUX
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


void GameProcess() {
    
    if (game.status == STATUS_PLAYING) {
            
        // shoot

        if ( (game.shoot_keypressed) || (is_key_pressed(RS_ATTACK_KEY_MASK)) ) {
                
            game.shoot_delay ++;
                
            if (game.shoot_delay > GAME_SHOOT_PERIOD) {
        
//                if ( (game.tx > 0) && (game.ty > 5) && (game.tx < GAME_WIDTH-20) && (game.ty < GAME_HEIGHT-10) ) {
                        
                    soundbuf_play(&game.sound_test1);
                    
                    game.bullet_index++;
                    game.bullet_index %= BULLETS_COUNT;
                    game.bullet_x[game.bullet_index] = game.tx + 5;
                    game.bullet_y[game.bullet_index] = game.ty;
//                };
                
                game.shoot_delay -= GAME_SHOOT_PERIOD;
                game.shoot_keypressed = 0;
            
            };
        };
            
            
            
        
        int speed = 4;
        int bullet_speed = 11;
        
        game.tx += speed * ( is_key_pressed(RS_ARROW_RIGHT_MASK) - is_key_pressed(RS_ARROW_LEFT_MASK) );
        game.ty += speed * ( is_key_pressed(RS_ARROW_DOWN_MASK) - is_key_pressed(RS_ARROW_UP_MASK) );
        
        game.tx = rs_clamp_i(game.tx, 5, GAME_WIDTH-25);
        game.ty = rs_clamp_i(game.ty, 5, GAME_HEIGHT - 25);
        
        game.tz += 1;

        int i;
        for (i = 0; i < BULLETS_COUNT; i++) {
            if (game.bullet_y[i]) {
                game.bullet_x[i] += bullet_speed;
                if (game.bullet_x[i] > GAME_WIDTH) {
                    game.bullet_y[i] = 0;
                };
            };
        };
        
    };

    game_draw();

}





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
            
        DEBUG10f("loading %d ...\n", i);
    
        texture_init(&(game.tex_rocks[i]), rock_size, rock_size);
        
        DEBUG10f("loading %d z...\n", i);
        
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
    

    #ifdef RS_LINUX
        rs_audio_init(RS_AUDIO_FMT_MONO16, RS_AUDIO_FREQ_16000, 0); 
    #endif

    soundbuf_init(&game.sound_test1, 2048);
//    soundbuf_fill(&game.sound_test1, 2, 50);
    soundbuf_sin_fade(&game.sound_test1, 0.7);

    soundbuf_init(&game.sound_test2, 1024);
    //soundbuf_fill(&game.sound_test2, 8, 40);
    soundbuf_sin(&game.sound_test2, 0.48);

    soundbuf_init(&game.sound_test3, 1024);
    //soundbuf_fill(&game.sound_test3, 12, 60);
    soundbuf_sin(&game.sound_test3, 0.24);


};


void GameTerm() {


    DEBUG10("--- Game Term ---");

    #ifdef RS_LINUX
        rs_audio_term();
    #endif

    game_font_term();
    
    free(game.scaled_framebuffer);
    
    texture_free(&game.framebuffer);
    texture_free(&game.tex);
    texture_free(&game.tex_clouds);
    texture_free(&game.tex_ground);
    
    texture_free(&game.tex_gui_line);
    
    int i;
    for (i = 0; i < ROCKS_COUNT; i++) {
        texture_free(&game.tex_rocks[i]);
    };
    
    soundbuf_free(&game.sound_test1);
    soundbuf_free(&game.sound_test2);
    soundbuf_free(&game.sound_test3);
    

};

// ------------ #Event Functions ------------






void GameKeyDown(int key, int first) {
    
    
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
            break;
    };
    
    
    if (game.status == STATUS_MENU) {
    
        switch (key) {
            case RS_KEY_LEFT:
                BIT_SET(game.keyboard_state, RS_ARROW_LEFT_MASK);

                if ( (game.menu_index == MENU_SETTINGS) && (game.menu_item_index == MENU_ITEM_WINDOW_SCALE) ) {
                    game_change_window_scale(-1);
                };

                //PlayBuffer(hBuff, 0);
                break;
            case RS_KEY_RIGHT:
                BIT_SET(game.keyboard_state, RS_ARROW_RIGHT_MASK);
                
                if ( (game.menu_index == MENU_SETTINGS) && (game.menu_item_index == MENU_ITEM_WINDOW_SCALE) ) {
                    game_change_window_scale(1);
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
                menu_open(0);
                break;
        };

    };
    
    if (game.status == STATUS_PLAYING) {
        switch (key) {
            
            case RS_KEY_ESCAPE:
                game.status = STATUS_MENU;
                menu_open(0);
                break;
            case RS_KEY_A:
                
//                if ( (game.tx > 0) && (game.ty > 5) && (game.tx < GAME_WIDTH-20) && (game.ty < GAME_HEIGHT-10) ) {
//                
//                    soundbuf_play(&game.sound_test1);
//                    
//                    game.bullet_index++;
//                    game.bullet_index %= BULLETS_COUNT;
//                    game.bullet_x[game.bullet_index] = game.tx + 12;
//                    game.bullet_y[game.bullet_index] = game.ty + 3;
//                };
                
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
            soundbuf_play(&game.sound_test2);
            break;
        case 1:
            soundbuf_play(&game.sound_test3);
            break;
    };
    
};
