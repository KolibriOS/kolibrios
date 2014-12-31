#include "rsgamedraw.h"
#include "rsgametext.h"
#include "rsgamemenu.h"

#include "rskos.h"

#include "rsnoise.h"

#include "strings.h"


void game_draw() {

	int w = 320;
	int h = 180;
	

	int kk = 20; // (rskos_get_time()/1) % 160;


    texture_clear(&game.framebuffer, game.bg_color );
    
    game.bg_color = COLOR_BLACK;
    

    int i, c, c2, c3;
    for (i = 0; i < 100; i++) {
        c = (0.5+0.45*rs_noise(kk+150, kk+i))*255;
        c2 = c + 0.05*rs_noise(kk+150, kk+i)*255;
        c3 = c2; // (0.5+0.49*rs_noise(kk+150, kk+i+2))*255;
        texture_set_pixel(&game.framebuffer, (0.5+0.49*rs_noise(kk+1100, kk+i))*GAME_WIDTH, (0.5+0.49*rs_noise(kk+1200, kk+i+1300))*GAME_HEIGHT, 
                          c + (c2<<8) + (c3<<16) + 0xFF000000);
    };

    texture_clear(&game.tex_ground, COLOR_TRANSPARENT);
    rs_perlin_configure(47, 4, 0.5, 1000, 256);
    for (i = 0; i < game.tex_ground.w; i++) {
        texture_draw_vline(&game.tex_ground, i, 25 + rs_perlin(0,i+game.tz)*25, 2, 0xFF113300);
        texture_draw_vline(&game.tex_ground, i, 25 + rs_perlin(0,i+game.tz)*25 + 2, 999, 0xFF000000);
    };
    
    

    if (game.status == STATUS_MENU) {

        char **title = menu_titles[game.menu_index]; 
        int y = (game.menu_index == MENU_MAIN) ? 80 : 50;
        texture_draw(&game.framebuffer, &game.tex_gui_line, 0, y+15*game.menu_item_index, DRAW_MODE_ALPHA);
        while (*title) {
            game_textout(20, y, (*(title))[0]==' ' ? 3 : 0 , (*(title))+1 );  // first (zero) char defines action, title starts from second (1st) char
            title++;
            y+=15;
        };
        
        if (game.menu_index == MENU_MAIN) {
                
            for (i = 0; i < 3; i++) {
                texture_draw(&game.framebuffer, &game.tex_rocks[i], 250+80*rs_noise(i,150), 60+60*rs_noise(i,1110), DRAW_MODE_ADDITIVE );
            };
                
            game_textout( GAME_WIDTH/2 - 100, 40, 1,  "HELI0THRYX");
//            game_textout( GAME_WIDTH/2 - 8, 58, 3,  "TECHDEM0");
            game_textout( 2, GAME_HEIGHT-10, 2,  L_BOTTOM_LINE_DEVELOPER_INFO);
        };
    
    }
    else {
        
        int i, j;
        game_obj_t *obj;
        for (i = 0; i < game.objs_count; i++) {
            obj = &(game.objs[i]);
            
            if (obj->obj_type == OBJ_BULLET) {
                    
                texture_set_pixel(&game.framebuffer, obj->x-4, obj->y, 0xFF00BB00);
                texture_set_pixel(&game.framebuffer, obj->x-3, obj->y, 0xFF00CC00);
                texture_set_pixel(&game.framebuffer, obj->x-2, obj->y, 0xFF00DD00);
                texture_set_pixel(&game.framebuffer, obj->x-1, obj->y, 0xFF00EE00);
                texture_set_pixel(&game.framebuffer, obj->x-0, obj->y, 0xFF00FF00);
                
            }
            else if (obj->obj_type == OBJ_RED_BULLET) {
                    
                texture_set_pixel(&game.framebuffer, obj->x-1, obj->y-0, 0xFFFF0000);
                texture_set_pixel(&game.framebuffer, obj->x-1, obj->y-1, 0xFFFF6600);
                texture_set_pixel(&game.framebuffer, obj->x-1, obj->y-0, 0xFFFF0000);
                texture_set_pixel(&game.framebuffer, obj->x-0, obj->y-1, 0xFFFF0000);
                texture_set_pixel(&game.framebuffer, obj->x-0, obj->y-0, 0xFFFF6600);
                texture_set_pixel(&game.framebuffer, obj->x+1, obj->y-1, 0xFFFF0000);
                texture_set_pixel(&game.framebuffer, obj->x+1, obj->y-0, 0xFFFF6600);
                
            }
            else if (obj->obj_type == OBJ_EXPLOSION) {
                texture_draw( &game.framebuffer, &game.tex_explosions[ obj->t ], obj->x - obj->radius, obj->y - obj->radius, DRAW_MODE_ALPHA );
            }
            else if (obj->obj_type == OBJ_HUGE_EXPLOSION) {
                texture_draw( &game.framebuffer, &game.tex_huge_explosions[ obj->t ], obj->x - obj->radius, obj->y - obj->radius, DRAW_MODE_ALPHA );
            }
            else if (obj->obj_type == OBJ_ROCK) {
                texture_draw( &game.framebuffer, &game.tex_rocks[ obj->tag ], obj->x - obj->radius, obj->y - obj->radius, DRAW_MODE_ALPHA );
            }
            else if (obj->obj_type == OBJ_MINIROCK) {
                texture_draw( &game.framebuffer, &game.tex_minirocks[ obj->tag ], obj->x - obj->radius, obj->y - obj->radius, DRAW_MODE_ALPHA );
            }
            else if (obj->obj_type == OBJ_TURRET) {
                texture_draw( &game.framebuffer, &game.tex_rocks[ 0 ], obj->x - obj->radius, obj->y - obj->radius, DRAW_MODE_ALPHA );
                texture_draw( &game.framebuffer, &game.tex_rocks[ 0 ], obj->x - obj->radius, obj->y - obj->radius, DRAW_MODE_ADDITIVE );
                
                for (j = 0; j < 1 + (obj->tag)/6; j++) {
                    texture_draw_vline(&game.framebuffer, obj->x - obj->radius + j*4 + 0, obj->y - obj->radius - 16, 8, 0xFF993333 );
                    texture_draw_vline(&game.framebuffer, obj->x - obj->radius + j*4 + 1, obj->y - obj->radius - 16, 8, 0xFF993333 );
                    texture_draw_vline(&game.framebuffer, obj->x - obj->radius + j*4 + 2, obj->y - obj->radius - 16, 8, 0xFF993333 );
                };
                
            }

        };
        
 
        texture_draw(&game.framebuffer, &game.tex_ship[0], game.player_x-8, game.player_y-4, DRAW_MODE_ALPHA);
        texture_draw(&game.framebuffer, &game.tex_ship[1], game.player_x-8, game.player_y-4, DRAW_MODE_ALPHA);
        texture_draw(&game.framebuffer, &game.tex_ship[2], game.player_x, game.player_y-4, DRAW_MODE_ALPHA);
        texture_draw(&game.framebuffer, &game.tex_ship[3], game.player_x, game.player_y-4, DRAW_MODE_ALPHA);
        
        
        if ( game.stage == 0 ) {
                
            int stage_label_y = GAME_HEIGHT/3 - (game.stage_timer - 25)*(game.stage_timer - 25)*(game.stage_timer - 25)/100;
                
            //game_textout_at_center( 0, GAME_HEIGHT + 50 - game.stage_timer*(GAME_HEIGHT+50)/50, 1, "LEVEL 1" );
            
            char stage_str[] = "5TAGE xx";
            char *stage_num = &stage_str[6];
            
            if ( (game.stage_level+1) > 9 ) {
                stage_num[0] = '0' + (game.stage_level+1)/10;
                stage_num[1] = '0' + (game.stage_level+1)%10;
            }
            else {
                stage_num[0] = '0' + (game.stage_level+1)%10;
                stage_num[1] = 0;
            };
            
            game_textout_at_center( -10, stage_label_y, 1, stage_str );
            
            if ( IS_BIT_CLEARED ( game.flags, GAME_FLAG_INSTRUCTIONS_PASSED ) ) {
                game_textout_at_center( 0, GAME_HEIGHT*3/4, 2, L_TECHDEMO_LINE1 );
            };
        }
        
        char s_score[] = "0000";
        s_score[0] += (game.score / 1000) % 10;
        s_score[1] += (game.score / 100) % 10;
        s_score[2] += (game.score / 10) % 10;
        s_score[3] += (game.score / 1) % 10;
        
        game_textout_at_center(0, 10, 3, s_score);
        
        
        
        
        char s_health[] = "HEALTH: 0 ";
        char s_ammo[] = "AMM0: 00 ";
        
        s_health[8] += game.health;
        s_ammo[6] += game.ammo / 10;
        s_ammo[7] += game.ammo % 10;
        
        game_textout(8, 8, 2, s_health);
        game_textout(GAME_WIDTH - 12 - GAME_AMMO_MAX*2 - 1, 8, 2, s_ammo);
        
        for (i = 0; i < game.ammo; i++) {
            texture_draw_vline(&game.framebuffer, GAME_WIDTH - 12 - GAME_AMMO_MAX*2 + i*2, 20, 8, 0xFF3366FF );
        };
        
        int health_color = 0xFF339933;
        if (game.health < 5) {
            health_color = 0xFF808010;
        };
        if (game.health < 3) {
            health_color = 0xFFFF3300;
        };

        for (i = 0; i < game.health; i++) {
            texture_draw_vline(&game.framebuffer, 8 + i*4 + 0, 20, 8, health_color  );
            texture_draw_vline(&game.framebuffer, 8 + i*4 + 1, 20, 8, health_color  );
            texture_draw_vline(&game.framebuffer, 8 + i*4 + 2, 20, 8, health_color  );
        };
        

        
    };

    texture_draw(&game.tex_ground, &game.tex_clouds, game.tz, 0, /* game.tx, game.ty, */ DRAW_MODE_ADDITIVE | DRAW_TILED_FLAG );
    texture_draw(&game.framebuffer, &game.tex_ground, 0, GAME_HEIGHT-50, DRAW_MODE_ALPHA);


	rskos_draw_area(0, 0, w, h, game.window_scale, game.framebuffer.data, game.scaled_framebuffer);

};

