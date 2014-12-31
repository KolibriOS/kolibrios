#include "rsgamelogic.h"

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




int next_rock_timer = 0;

void next_stage_now() {
    game.stage_timer = 0;
    game.stage++;
};

void next_stage_after(int t) {
    if (game.stage_timer > t) {
        next_stage_now();
    };
};

void next_stage_after_sec(int t) {
    next_stage_after(t*25);
};




int check_collision(int obj1, int obj2) {
    // obj1 must be bullet 
    // obj2 can be anything
    
    if ( game.objs[obj1].obj_type == OBJ_BULLET ) {
        
        if ( ( abs(game.objs[obj1].y - game.objs[obj2].y) < ( 4 + game.objs[obj2].radius ) ) 
            && ( abs( (game.objs[obj1].x - 8) - game.objs[obj2].x) < ( 8 + game.objs[obj2].radius ) )  ){
            return 1;
        };
        
    };
    
    return 0;
    
};

int check_collision_with_player(int obj1) {
    
    const int player_radius = 3;
    
    int obj_radius = game.objs[obj1].radius;
//    if (game.objs[obj1].obj_type == OBJ_RED_BULLET) {
//        obj_radius = 3;
//    };
    
//    if ( game.objs[obj1].obj_type == OBJ_RED_BULLET ) {
        
        if ( ( abs(game.objs[obj1].y - game.player_y) < ( obj_radius + player_radius ) ) 
            && ( abs( (game.objs[obj1].x ) - game.player_x) < ( obj_radius + 4 + player_radius ) )  ){
            return 1;
        };
        
//    };
    
    return 0;
    
};

void player_hit() {
    
    game.health--;
    game.bg_color = COLOR_DARK_RED;
    
    soundbuf_play( &game.sound_hit, 0 );

    
    if (game.health < 1) {
        game.status = STATUS_MENU;
        menu_open( MENU_GAME_OVER );
        
        level_passed_score_str[1] = '0' + (game.score / 1000) % 10;
        level_passed_score_str[2] = '0' + (game.score / 100) % 10;
        level_passed_score_str[3] = '0' + (game.score / 10) % 10;
        level_passed_score_str[4] = '0' + (game.score / 1) % 10;
        
    };
    
};


void GameProcess() {
    
    if (game.status == STATUS_PLAYING) {
            
        int c_shoot_restore_delay = (6 + 2*game.stage_level);
        int c_shoot_restore_period = (game.stage_level < 2) ? (3 + game.stage_level) : 5;
            
        // shoot

        if ( ( (game.shoot_keypressed) || (is_key_pressed(RS_ATTACK_KEY_MASK)) ) && (game.ammo>0) ) {
                
            game.shoot_delay ++;
                
            if (game.shoot_delay > GAME_SHOOT_PERIOD) {
        
//                if (game.ammo > 0) {
                    
                    game.shoot_restore_delay = 0;
                    game.ammo--;
                    soundbuf_play(&game.sound_shoot, 0);
                    game_obj_add( game_obj( OBJ_BULLET, 0, 0, 0, game.player_x+5, game.player_y, 0, 0.0) );
                    
//                };
                
                game.shoot_delay = 1; // -= GAME_SHOOT_PERIOD;
                
                
                
                game.shoot_keypressed = 0;
            
            };
        }
        else {
            
            if (game.ammo < GAME_AMMO_MAX) {
                game.shoot_restore_delay++;
                
                if (game.shoot_restore_delay > c_shoot_restore_delay) {
                        
                    game.shoot_delay++;

                    if (game.shoot_delay > c_shoot_restore_period) {
                        game.ammo++;
                        game.shoot_delay -= c_shoot_restore_period;
                    };
                    
                };
                
            };
            
        };
            
            
            
        
        int speed = 4;
        int bullet_speed = 11;
        int red_bullet_speed = 8;
        int rock_speed = 6;
        
        game.player_x += speed * ( is_key_pressed(RS_ARROW_RIGHT_MASK) - is_key_pressed(RS_ARROW_LEFT_MASK) );
        game.player_y += speed * ( is_key_pressed(RS_ARROW_DOWN_MASK) - is_key_pressed(RS_ARROW_UP_MASK) );
        
        game.player_x = rs_clamp_i(game.player_x, 15, GAME_WIDTH - 45);
        game.player_y = rs_clamp_i(game.player_y, 15, GAME_HEIGHT - 45);
        
        game.tz += 1;
        
        

        int c_rocktimer_const = (game.stage_level < 4) ? (9 - 2*game.stage_level) : 4;
        int c_rocktimer_var = (game.stage_level < 6) ? (16 - 2*game.stage_level) : 6;
        
        
        
        game.stage_timer++;
        
        if (game.stage == 0) {
            
            // level start
            
            next_stage_after_sec(3);
            
        }
        
        else if (game.stage == 1) {
                
//            game.stage = 4;
            
            // rocks
            next_rock_timer--;
            if (next_rock_timer < 1) {
                next_rock_timer = c_rocktimer_const + rs_rand()%c_rocktimer_var;
                //game_obj_add( game_obj( ((rs_rand() % 512) > 256) ? OBJ_ROCK : OBJ_MINIROCK, 0, rs_rand() % ROCKS_COUNT , 32, GAME_WIDTH + 100, 30 + rs_rand()%(GAME_HEIGHT-60), 0, 0.0 ) );
                
                int flagsin = 0;
                if ( game.stage_level > 4 ) {
                    if ( rs_rand()%1024 < (16*game.stage_level) ) {
                        flagsin = OBJ_FLAG_SIN;
                    };
                };
                
                game_obj_add( game_obj( OBJ_ROCK, OBJ_FLAG_ENEMY | flagsin, rs_rand() % ROCKS_COUNT , game.tex_rocks[0].w/2, GAME_WIDTH + 50, 30 + rs_rand()%(GAME_HEIGHT-90), 0, 0.0 ) );
            };
        
            next_stage_after_sec(12);
            
        }
        
        if (game.stage == 2) {
                
            BIT_SET (game.flags, GAME_FLAG_INSTRUCTIONS_PASSED);
            
            next_stage_after_sec(4);
            
        }
        
        else if (game.stage == 3) {
            
            // rocks
            next_rock_timer--;
            if (next_rock_timer < 1) {
                next_rock_timer = c_rocktimer_const + 1 + rs_rand()%c_rocktimer_var;
                //game_obj_add( game_obj( ((rs_rand() % 512) > 256) ? OBJ_ROCK : OBJ_MINIROCK, 0, rs_rand() % ROCKS_COUNT , 32, GAME_WIDTH + 100, 30 + rs_rand()%(GAME_HEIGHT-60), 0, 0.0 ) );

                int flagsin = 0;
                if ( game.stage_level > 2 ) {
                    if ( rs_rand()%1024 < (16*game.stage_level) ) {
                        flagsin = OBJ_FLAG_SIN;
                    };
                };

                game_obj_add( game_obj( OBJ_MINIROCK, OBJ_FLAG_ENEMY|flagsin, rs_rand() % ROCKS_COUNT , game.tex_minirocks[0].w/2, GAME_WIDTH + 50, 30 + rs_rand()%(GAME_HEIGHT-90), 0, 0.0 ) );
            };
        
            next_stage_after_sec(16);
            
        }
        
        else if (game.stage == 4) {
            
            next_stage_after_sec(4);
            
        }
        
        else if (game.stage == 5) {
            
            // rocks
            next_rock_timer--;
            if (next_rock_timer < 1) {
                next_rock_timer = 5;
                //game_obj_add( game_obj( ((rs_rand() % 512) > 256) ? OBJ_ROCK : OBJ_MINIROCK, 0, rs_rand() % ROCKS_COUNT , 32, GAME_WIDTH + 100, 30 + rs_rand()%(GAME_HEIGHT-60), 0, 0.0 ) );
                game_obj_add( game_obj( OBJ_MINIROCK, OBJ_FLAG_ENEMY | OBJ_FLAG_SIN, rs_rand() % ROCKS_COUNT , game.tex_minirocks[0].w/2, GAME_WIDTH + 50, GAME_HEIGHT/8, 0, 0.0 ) );
            };
        
            next_stage_after_sec(6);
            
        }
        
        else if (game.stage == 6) {
            
            // mix rocks
            next_rock_timer--;
            if (next_rock_timer < 1) {
                next_rock_timer = c_rocktimer_const + rs_rand()%(c_rocktimer_var-3);
                
                int flagsin = 0;
                if ( game.stage_level > 3 ) {
                    if ( rs_rand()%1024 < (16*game.stage_level) ) {
                        flagsin = OBJ_FLAG_SIN;
                    };
                };
                
                //game_obj_add( game_obj( ((rs_rand() % 512) > 256) ? OBJ_ROCK : OBJ_MINIROCK, 0, rs_rand() % ROCKS_COUNT , 32, GAME_WIDTH + 100, 30 + rs_rand()%(GAME_HEIGHT-60), 0, 0.0 ) );
                game_obj_add( game_obj( rs_rand()%1024 < 768 ? OBJ_MINIROCK : OBJ_ROCK, OBJ_FLAG_ENEMY | flagsin, rs_rand() % ROCKS_COUNT , 
                                       rs_rand()%1024 < 768 ? game.tex_minirocks[0].w/2 : game.tex_rocks[0].w/2, GAME_WIDTH + 100, 30 + rs_rand()%(GAME_HEIGHT-90), 0, 0.0 ) );
            };
        
            next_stage_after_sec(10);
            
        }
        
        else if (game.stage == 7) {
            
            
            if (game.stage_timer > 3*25) {
                next_stage_now();
                
                BIT_CLEAR(game.flags, GAME_FLAG_BOSS_DESTROYED);
                
                game_obj_add( game_obj( OBJ_TURRET, OBJ_FLAG_ENEMY | OBJ_FLAG_BOSS, 60, game.tex_rocks[0].w/2, GAME_WIDTH+60, GAME_HEIGHT/2, 0, 0.0 ) );
                
            };
            
        }
        
        else if (game.stage == 8) {
                
            if ( IS_BIT_SET(game.flags, GAME_FLAG_BOSS_DESTROYED) ) {
                next_stage_now();
            };
            
        }
        
        else if (game.stage == 9) {
            next_stage_after_sec(2);
        }                
        else if (game.stage == 10) {
            
            /*
            game.status = STATUS_MENU;
            menu_open( MENU_LEVEL_PASSED );
        
            level_passed_score_str[1] = '0' + (game.score / 100) % 10;
            level_passed_score_str[2] = '0' + (game.score / 10) % 10;
            level_passed_score_str[3] = '0' + (game.score / 1) % 10;
            */
            
            game.stage_level++;
            
            game.stage = 0;
            game.stage_timer = 0;
            
        };
        
        
        
        

        int i, j;
        game_obj_t *obj;
        
        for (i = 0; i < game.objs_count; i++) {
            
            obj = &(game.objs[i]);
            
            if (obj->obj_type == OBJ_BULLET) {
                    
                obj->x += bullet_speed;
                if (obj->x > GAME_WIDTH) {
                    // destroy object
                    game_obj_remove(i);
                    i--;
                    continue;
                };
                
                for (j = 0; j < game.objs_count; j++) {
                    if (IS_BIT_SET(game.objs[j].flags, OBJ_FLAG_ENEMY)) {
                        if (check_collision(i, j)) {
                            if (IS_BIT_SET( game.objs[j].flags, OBJ_FLAG_BOSS)) {
                                game.objs[j].tag--;
                                if (game.objs[j].tag < 1) {
                                    BIT_SET( game.objs[j].flags, OBJ_FLAG_DESTROYED );
                                    BIT_SET( game.flags, GAME_FLAG_BOSS_DESTROYED );
                                    game.score += 50;
                                };
                            }
                            else {
                                BIT_SET( game.objs[j].flags, OBJ_FLAG_DESTROYED );
                                game.score += game.objs[j].obj_type == OBJ_ROCK ? 2 : 3;
                            };
                            game_obj_remove(i); 
                            i--;
                            break; // continue parent loop
                        };
                    };
                };
                
            }
            
            else if (obj->obj_type == OBJ_RED_BULLET) {
                
                obj->x -= red_bullet_speed;
                if (obj->x < 4) {
                    // destroy object
                    game_obj_remove(i);
                    i--;
                    continue;
                };
                
                if (check_collision_with_player(i)) {
                    player_hit();
                    game_obj_remove(i);
                    i--;
                    continue;
                };
                
            }
            
            else if (obj->obj_type == OBJ_EXPLOSION) {
                
                obj->t++;
                if (obj->t >= EXPLOSIONS_COUNT) {
                    game_obj_remove(i);
                    i--;
                    continue;
                };
                
            }
            
            else if (obj->obj_type == OBJ_HUGE_EXPLOSION) {
                
                obj->t++;
                if (obj->t >= HUGE_EXPLOSIONS_COUNT) {
                    game_obj_remove(i);
                    i--;
                    continue;
                };
                
            }
            
            else if ( (obj->obj_type == OBJ_ROCK) || (obj->obj_type == OBJ_MINIROCK) ) {
                
                obj->x -= rock_speed;
                if (obj->x < - obj->radius * 2) {
                    game_obj_remove(i);
                    i--;
                    continue;
                };
                
                if ( IS_BIT_SET(obj->flags, OBJ_FLAG_SIN) ) {
                    obj->f += 0.2;
                    obj->y += 7.0 * sin(obj->f);
                };
                
                if ( check_collision_with_player(i) ) {
                    player_hit();
                    game_obj_remove(i);
                    i--;
                    continue;
                };
                
            }
            else if ( obj->obj_type == OBJ_TURRET ) {
                
                if (obj->x > GAME_WIDTH*3/4) {
                    obj->x -= 2;
                }
                else {
                    obj->f += 0.03;
                    obj->y = GAME_HEIGHT * ( 0.5 + 0.3*sin(obj->f) );
                };
                
                if ( obj->x < GAME_WIDTH*4/5 ) {
                    // turret shoot
                    obj->t--;
                    if (obj->t < 1) {
                        soundbuf_play(&game.sound_turret_shoot, 0);
                        game_obj_add( game_obj( OBJ_RED_BULLET, 0, 0, 3, obj->x - 5, obj->y, 0, 0) );
                        
                        int c_const = (game.stage_level < 4) ? (10 - 2*game.stage_level) : 3;
                        int c_var = (game.stage_level < 6) ? (20 - 3*game.stage_level) : 3;
                        
                        obj->t = c_const + rs_rand() % c_var;
                        
                        if ( (rs_rand()%1024) < 80 ) {
                            obj->t += 18;
                        };
                        
                    };
                };
                
            };
            
        };
        
        
        
        for (i = 0; i < game.objs_count; i++) {
            if ( IS_BIT_SET( game.objs[i].flags, OBJ_FLAG_DESTROYED ) ) {
                    
                if (game.objs[i].obj_type == OBJ_TURRET) {
                    soundbuf_play( &game.sound_huge_explosion, 0 );
                    game_obj_add( game_obj( OBJ_HUGE_EXPLOSION, 0, 0, HUGE_EXPLOSION_RADIUS, game.objs[i].x, game.objs[i].y, 0, 0.0 ) );
                }
                else {
                    soundbuf_play( &game.sound_explosions[ rs_rand() % SOUND_EXPLOSIONS_COUNT ], 0 );
                    game_obj_add( game_obj( OBJ_EXPLOSION, 0, 0, EXPLOSION_RADIUS, game.objs[i].x, game.objs[i].y, 0, 0.0 ) );
                };
                game_obj_remove(i);
                i--;
                
            };
        };
        
 

    };

    game_draw();
    
    if ( (game.status == STATUS_MENU) && (game.menu_index != MENU_PAUSE) ) {
        soundbuf_loop_check( &game.sound_music );
    };

}



