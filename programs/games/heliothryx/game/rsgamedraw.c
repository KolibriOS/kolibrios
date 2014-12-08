#include "rsgamedraw.h"
#include "rsgametext.h"
#include "rsgamemenu.h"

#include "rskos.h"

#include "rsnoise.h"




void game_draw() {

	int w = 320;
	int h = 180;
	

	int kk = 20; // (rskos_get_time()/1) % 160;
//	
//	unsigned char *c = game.framebuffer.data;
//	
//	int i;
//	for (i = 0; i < w*h*4; i+=4) {
//	    c[i+0] = 10; //  i/w/3;
//	    c[i+1] = (( (1*i)*(i + kk)/70) & 5) ? 70 : 0;
//	    c[i+2] = 50;
//	    c[i+3] = i % 128;
//	};


    texture_clear(&game.framebuffer, COLOR_BLACK);
//    texture_clear(&game.tex);
    
//    texture_draw(&game.framebuffer, &game.tex, 40, 40, DRAW_MODE_ADDITIVE);
//    texture_draw(&game.framebuffer, &game.tex, 70, 50, DRAW_MODE_ADDITIVE);
//    texture_draw(&game.framebuffer, &game.tex, 20, 60, DRAW_MODE_ADDITIVE);
//    texture_draw(&game.framebuffer, &game.tex, 60, 70, DRAW_MODE_ADDITIVE);
//    
//    texture_draw(&game.framebuffer, &game.tex, 111, 150, DRAW_MODE_ADDITIVE);

    int i, c, c2, c3;
    for (i = 0; i < 100; i++) {
//        DEBUG10f("i = %d, v1 = %.4f, v2 = %.4f \n", i, rs_noise(kk+100, kk+i)*10, rs_noise(kk+200, kk+i+300)*10);
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
    
    texture_draw(&game.tex_ground, &game.tex_clouds, game.tz, 0, /* game.tx, game.ty, */ DRAW_MODE_ADDITIVE | DRAW_TILED_FLAG );
    texture_draw(&game.framebuffer, &game.tex_ground, 0, GAME_HEIGHT-50, DRAW_MODE_ALPHA);
    

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
                
            for (i = 0; i < ROCKS_COUNT; i++) {
                texture_draw(&game.framebuffer, &game.tex_rocks[i], 250+80*rs_noise(i,150), 60+60*rs_noise(i,1110), DRAW_MODE_ADDITIVE );
            };
                
            game_textout( GAME_WIDTH/2 - 100, 40, 1,  "HELIOTHRYX");
            game_textout( GAME_WIDTH/2 - 8, 58, 3,  "TECHDEMO");
            game_textout( 2, GAME_HEIGHT-10, 2,  "DEVELOPER: ROMAN SHUVALOV` TOGLIATTI_ 2014");
        };
    
    }
    else {
        
        texture_draw(&game.framebuffer, &game.tex_ship[0], game.tx-8, game.ty-4, DRAW_MODE_ALPHA);
        texture_draw(&game.framebuffer, &game.tex_ship[1], game.tx-8, game.ty-4, DRAW_MODE_ALPHA);
        texture_draw(&game.framebuffer, &game.tex_ship[2], game.tx, game.ty-4, DRAW_MODE_ALPHA);
        texture_draw(&game.framebuffer, &game.tex_ship[3], game.tx, game.ty-4, DRAW_MODE_ALPHA);
        
        int i;
        for (i = 0; i < BULLETS_COUNT; i++) {
            if (game.bullet_y[i]) {
                texture_set_pixel(&game.framebuffer, game.bullet_x[i]-4, game.bullet_y[i], 0xFF00BB00);
                texture_set_pixel(&game.framebuffer, game.bullet_x[i]-3, game.bullet_y[i], 0xFF00CC00);
                texture_set_pixel(&game.framebuffer, game.bullet_x[i]-2, game.bullet_y[i], 0xFF00DD00);
                texture_set_pixel(&game.framebuffer, game.bullet_x[i]-1, game.bullet_y[i], 0xFF00EE00);
                texture_set_pixel(&game.framebuffer, game.bullet_x[i]-0, game.bullet_y[i], 0xFF00FF00);
            };
        };
        
        game_textout( 2, 2, 2,  "THIS IS TECHDEMO` ");
        game_textout( 2, 12, 2,  "USE ARROWS TO MOVE_ <A> TO SHOOT_ <ESC> TO EXIT` ");
        
    };


	rskos_draw_area(0, 0, w, h, game.window_scale, game.framebuffer.data, game.scaled_framebuffer);

};

