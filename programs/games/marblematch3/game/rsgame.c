#include "rsgame.h"

#include "rsgametext.h"

#include "rsgamemenu.h"

#include "rsgamedraw.h"

#include "rskos.h"

#include "rsgentex.h"
#include "rssoundgen.h"
#include "rsnoise.h"

#include "rs/rsplatform.h"





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
                    dest->data[ 4 * ( (y+i)*dest->w + (x+j) ) + k ] = src->data[ (4*((i+ishift)*src->w + j + jshift) + k) % modvalue];
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
    else if (mode == DRAW_MODE_MULT) {
        for (i = istart; i < iend; i++) {
            for (j = jstart; j < jend; j++) {
                for (k = 0; k < 3; k++) { // Alpha channel is not added
                    dest->data[ 4 * ( (y+i)*dest->w + (x+j) ) + k ] = 
                        clamp_byte( dest->data[ 4 * ( (y+i)*dest->w + (x+j) ) + k ] * src->data[ (4*((i+ishift)*src->w + j + jshift) + k) % modvalue] / 255 );
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

void texture_draw_hline(rs_texture_t *tex, int x, int y, int l, unsigned int color) {
    int i;
    if (x+l >= tex->w) {
        l = tex->w - x;
    };
    for (i = 0; i < l; i++) {
        *((unsigned int*) &tex->data[ 4 * ( (y)*tex->w + (x+i) ) + 0 ]) = color;
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
	
	
	/*
	
	// ok
	
	rs_sgen_init(2, snd->length_samples);
	rs_sgen_func_pm(1, 880.0, 21.0, 0.3, 110.0, 0.3);
	rs_sgen_func_normalize(1, 1.0);
	rs_sgen_func_lowpass(0, 1, 1.0, 0.0, 1.0);
	rs_sgen_wave_out(0);
	
	memcpy(snd->data, rs_sgen_reg.wave_out, snd->length_samples*2 );
	
	rs_sgen_term();
	
	*/
	
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
    
    game.loader_counter = 0;
    
    game.need_redraw = 1;
    
    game.score = 0;
    game.time = 0;

    game.selected = 0;
    game.selected_x = game.selected_y = 0;
    
    game.explosions_count = 0;
    
    game.tx = 0;
    game.ty = 0;
    game.tz = 0;

//    int i;
//    for (i = 0; i < BULLETS_COUNT; i++) {
//        game.bullet_x[i] = 0;
//        game.bullet_y[i] = 0;
//    };
//    game.bullet_index = 0;
    
    game.status = STATUS_LOADING;

    game.window_scale = 1;
    
//    game.window_scale = 2;
//    #ifndef RS_KOS
//        game.window_scale = 3;
//        window_scale_str[3] = '3'; 
//    #endif
    
    game.keyboard_state = 0;
    
    game.menu_index = 0;
    game.menu_item_index = 0;
};


int is_key_pressed(int mask) {
    return IS_BIT_SET(game.keyboard_state, mask) ? 1 : 0;
};

int seed = 0;
int get_random_crystal() {
    seed += 2;
    return ( (seed + (get_time() & 0xFFFF) ) % CRYSTALS_COUNT) | CRYSTAL_VISIBLE_BIT;
};

int game_check_and_explode() {
    int x, y, i, item, match_count, found;
    found = 0;
    
    // vertical lines
    for (x = 0; x < FIELD_WIDTH; x++) {
        match_count = 0;
        item = 0xFF;
        for (y = FIELD_HEIGHT-1; y >= 0; y--) {
                
            if ( IS_BIT_SET( FIELD_ITEM(x,y), CRYSTAL_MOVING_BIT ) || IS_BIT_CLEARED( FIELD_ITEM(x,y), CRYSTAL_VISIBLE_BIT) ) {
                item = 0xFF;
                match_count = 0;
                continue;
            };
            
            if ( (FIELD_ITEM(x,y) & CRYSTAL_INDEX_MASK) == item) {
                match_count++;
            }
            else {
                if (match_count >= 2) {
                    found = 1;
                    for (i = y+1; i < y+1+match_count+1; i++) {
                        BIT_SET( FIELD_ITEM(x, i), CRYSTAL_EXPLODED_BIT );
                    };
                }
                item = FIELD_ITEM(x,y) & CRYSTAL_INDEX_MASK;
                match_count = 0;
            };
        };
        if (match_count >= 2) { // last
            found = 1;
            for (i = y+1; i < y+1+match_count+1; i++) {
                BIT_SET( FIELD_ITEM(x, i), CRYSTAL_EXPLODED_BIT );
            };
        };
    };
    
    // horizontal lines
    for (y = 0; y < FIELD_HEIGHT; y++) {
            
        match_count = 0;
        item = 0xFF;
        for (x = FIELD_WIDTH-1; x >= 0; x--) {

            if ( IS_BIT_SET( FIELD_ITEM(x,y), CRYSTAL_MOVING_BIT ) || IS_BIT_CLEARED( FIELD_ITEM(x,y), CRYSTAL_VISIBLE_BIT) ) {
                item = 0xFF;
                match_count = 0;
                continue;
            };

            if ( (FIELD_ITEM(x,y) & CRYSTAL_INDEX_MASK) == item) {
                match_count++;
            }
            else {
                if (match_count >= 2) {
                    found = 1;
                    for (i = x+1; i < x+1+match_count+1; i++) {
                        BIT_SET( FIELD_ITEM(i, y), CRYSTAL_EXPLODED_BIT );
                    };
                }
                item = FIELD_ITEM(x,y) & CRYSTAL_INDEX_MASK;
                match_count = 0;
            };
        };
        if (match_count >= 2) { // last
            found = 1;
            for (i = x+1; i < x+1+match_count+1; i++) {
                BIT_SET( FIELD_ITEM(i, y), CRYSTAL_EXPLODED_BIT );
            };
        };
    };

    for (i = 0; i < FIELD_LENGTH; i++) {
        if (IS_BIT_SET(game.field[i], CRYSTAL_EXPLODED_BIT)) {
            game.field[i] = 0;
            game.score++;
            if (game.explosions_count < EXPLOSIONS_MAX_COUNT-1) {
                game.explosions[game.explosions_count] = ( i % FIELD_WIDTH ) | ( (i / FIELD_WIDTH) << 8);
                game.explosions_count++;
            };
        };
    };
    
    if (game.score > 99) {
        game.status = STATUS_MENU;
        game.need_redraw = 1;
    };
    
    if (found) {
        game.need_redraw = 1;
    };
    
    return found;
};

void game_fall() {
    int x, y, fall;
    for (x = 0; x < FIELD_WIDTH; x++) {
        fall = 0;
        for (y = FIELD_HEIGHT-1; y > 0; y--) {
            fall |= !(FIELD_ITEM(x, y) & CRYSTAL_VISIBLE_BIT);
            if (fall) {
                FIELD_ITEM(x, y) = FIELD_ITEM(x, y-1) | CRYSTAL_MOVING_BIT;
                game.need_redraw = 1;
            }
            else {
                BIT_CLEAR( FIELD_ITEM(x, y), CRYSTAL_MOVING_BIT );
            };
        };
        if ( (fall) || ( IS_BIT_CLEARED(FIELD_ITEM(x,0), CRYSTAL_VISIBLE_BIT) ) ) {
            FIELD_ITEM(x, 0) = get_random_crystal();
        };
    };
};

int process_timer = 0;

void GameProcess() {
    
    if (game.status == STATUS_LOADING) {
        game.loader_counter++;
        if (game.loader_counter == 2) {
                
            game_textures_init_stage2();
               
            
        
            game.status = STATUS_MENU;
            game.need_redraw = 1;

        };
    }
    else if (game.status == STATUS_PLAYING) {
            
            
        process_timer++;

        if (process_timer > 3) {
            game_check_and_explode();
            game_fall();
            process_timer = 0;
        };
        
        int i;
        for (i = 0; i < game.explosions_count; i++) {
            game.need_redraw = 1;
            game.explosions[i] = (game.explosions[i] & 0xFFFF) | ( ((game.explosions[i]>>16)+1) << 16 );
            if ( (game.explosions[i] >> 16) >= EXPLOSION_FRAMES_COUNT ) {
                game.explosions[i] = game.explosions[game.explosions_count-1];
                game.explosions_count--;
                i--;
            };
        };
        
        if ((game.time+1)/25 != game.time/25) {
            game.need_redraw = 1;
        };
        
        game.time++;
        
        
        

    };

    game_draw();

}





void GameInit() {

    game_reg_init();
    
    game.field = malloc( FIELD_LENGTH );
    int i;
    for (i = 0; i < FIELD_LENGTH; i++) {
        game.field[i] = (unsigned char) (0.99 * fabs(rs_noise(i, 10)) * CRYSTALS_COUNT) | CRYSTAL_VISIBLE_BIT;
    };
//    memset( game.field, 0, FIELD_LENGTH );
    
    game.scaled_framebuffer = malloc(GAME_WIDTH*game.window_scale * GAME_HEIGHT*game.window_scale * 3);
    DEBUG10f("scaled framebuffer: %d (window_scale = %d) \n", game.window_scale * GAME_WIDTH * GAME_HEIGHT * 3, game.window_scale);
    
    game_font_init();
    
    game_textures_init_stage1();
    
    

    #ifndef RS_KOS
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
    
    free(game.field);

    #ifndef RS_KOS
        rs_audio_term();
    #endif

    game_font_term();
    
    game_textures_free();
    
    
    

    
    soundbuf_free(&game.sound_test1);
    soundbuf_free(&game.sound_test2);
    soundbuf_free(&game.sound_test3);
    

};

// ------------ #Event Functions ------------






void GameKeyDown(int key, int first) {
    
    if (key == RS_KEY_A) {
    
        game.need_redraw = 1;
        
    };    
    
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
//            game.shoot_keypressed = 1;
            break;
    };
    
    
    if (game.status == STATUS_MENU) {
    
        switch (key) {
            case RS_KEY_LEFT:
                BIT_SET(game.keyboard_state, RS_ARROW_LEFT_MASK);
                //PlayBuffer(hBuff, 0);
                break;
            case RS_KEY_RIGHT:
                BIT_SET(game.keyboard_state, RS_ARROW_RIGHT_MASK);
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
        

        
        if (key == RS_KEY_SPACE) {
            
            game.score = 101;
            
            
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
    
    game.need_redraw = 1;
    
    game.tx = x;
    game.ty = y;
    
    if (game.status == STATUS_MENU) {
        game.time = 0;
        game.score = 0;
        game.status = STATUS_PLAYING;
        return;
    };
    
    if (game.status == STATUS_PLAYING) {
        
        unsigned int field_x = (unsigned int) (x - FIELD_X0) / CRYSTAL_SIZE;
        if (field_x != rs_clamp_i(field_x, 0, FIELD_WIDTH-1)) {
            return;
        };
        
        unsigned int field_y = (unsigned int) (y - FIELD_Y0) / CRYSTAL_SIZE;
        if (field_y != rs_clamp_i(field_y, 0, FIELD_HEIGHT-1)) {
            return;
        };
        
        //FIELD_ITEM(field_x, field_y) = 0;
        
        if (!game.selected) {
            game.selected = 1;
            game.selected_x = field_x;
            game.selected_y = field_y;                                                                      
            
        }
        else {
            
            if ( abs(game.selected_x - field_x) + abs(game.selected_y - field_y) == 1 ) {
                game.selected = 0;
                
                // Trying to swap
                int temp_crystal = FIELD_ITEM(field_x, field_y);
                FIELD_ITEM(field_x, field_y) = FIELD_ITEM(game.selected_x, game.selected_y);
                FIELD_ITEM(game.selected_x, game.selected_y) = temp_crystal;
                
                if ( !game_check_and_explode() ) {
                    FIELD_ITEM(game.selected_x, game.selected_y) = FIELD_ITEM(field_x, field_y);
                    FIELD_ITEM(field_x, field_y) = temp_crystal;
                }
                else {
                    // success
                    process_timer = 0;
                };
                
            }
            else {
                if ( (game.selected_x != field_x) && (game.selected_y != field_y) ) {
                    game.selected_x = field_x;
                    game.selected_y = field_y;
                }
                else {
                    game.selected = 0;
                };
            };
            
        };
        
//        int i;
//        for (i = field_y; i > 0; i--) {
//            FIELD_ITEM(field_x, i) = FIELD_ITEM(field_x, (i-1) );
//        };
//        FIELD_ITEM(field_x, 0) = 0;
        
    };
    
};

void GameMouseUp(int x, int y) {
    //
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
