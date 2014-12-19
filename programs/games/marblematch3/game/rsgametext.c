#include "rs/rsplatform.h"

#include "rsgametext.h"

#include "rsgame.h"
#include "rsgentex.h"

#include "rskos.h"



#ifdef RS_USE_C_LIBS
    #include <math.h>
    #include <stdlib.h> 
    #include <string.h>
#endif


signed short pp_seg[32*8] = {

    // 0
    16,     6-16,
    16,     6+16,
    16,     6,
    13,     0,

    // 1
    26-16,   16,
    26+16,   16,
    26,      11,
    8,       0,

    // 2
    26-16,   16,
    26+16,   16,
    26,      21,
    8,     0,

    // 3
    16,     26-16,
    16,     26+16,
    16,     26,
    13,     0,

    // 4
    6-16,   16,
    6+16,   16,
    6,      21,
    8,     0,

    // 5
    6-16,   16,
    6+16,   16,
    6,      11,
    8,       0,

    // 6
    16,     16-16,
    16,     16+16,
    16,     16,
    13,     0,

    // 7
    16,     4-22,
    16,     4+22,
    16,     4,
    4,     0,




    // 8
    16-16,   16,
    16+16,   16,
    16,      11,
    8,       0,

    // 9
    16-16,   16,
    16+16,   16,
    16,      21,
    8,     0,

    // 10
    21-16,   11+16,
    21+16,   11-16,
    21,      11,
    9,       0,

    // 11 modified
    16-16,   16+16,
    16+16,   16-16,
    21,      21,
    9,       0,

    // 12
    16-16,   16-16,
    16+16,   16+16,
    21,      11,
    9,       0,

    // 13
    21-16,   21-16,
    21+16,   21+16,
    21,      21,
    9,       0,

    // 14
    16,     16-16,
    16,     16+16,
    10,     16,
    7,     0,

    // 15
    16,     16-16,
    16,     16+16,
    22,     16,
    7,      0,






    // 16
    16-16,   16+16,
    16+16,   16-16,
    11,      11,
    8,       0,

    // 17
    11-16,   21+16,
    11+16,   21-16,
    11,      21,
    9,       0,

    // 18
    11-16,   11-16,
    11+16,   11+16,
    11,      11,
    9,       0,

    // 19
    16-16,   16-16,
    16+16,   16+16,
    11,      21,
    9,       0,

    // 20 == copy 11 modified
    21-22,   21+22,
    21+22,   21-22,
    21,      21,
    9,       0,

    // 21 - right
    27-12,   28-9,
    27+12,   28+9,
    27,     27,
    4,     0,

    // 22 - left
    5-12,   28+9,
    5+12,   28-9,
    5,      27,
    4,     0,


    // 23
    6-16,   16,
    6+16,   16,
    6,      8,
    5,     0,



    // 24
    16-33,   16,
    16+33,   16,
    16,      13,
    4,     0,


    // 25
    16-33,   16,
    16+33,   16,
    16,      26,
    4,     0,

    // 26
    16,     6-16,
    16,     6+16,
    11,     6,
    7,     0,

    // 27
    16,     26-16,
    16,     26+16,
    11,     26,
    7,     0,

    // 28
    16,     6-16,
    16,     6+16,
    21,     6,
    7,     0,

    // 29
    16,     26-16,
    16,     26+16,
    21,     26,
    7,     0,


    // 30 ,
    11-22,   27-22,
    11+22,   27+22,
    11,     25,
    6,     0,

    // 31 not implemented - need to create up-left DOT for percent(%) sign
    16,     26-16,
    16,     26+16,
    16,     26,
    13,     0,


};

unsigned int ch_seg[64] = {
    0b00111111 /* | 1<<12 | 1<<19 */, // 0 or O
    1<<26 | 1<<8 | 1<<9 | 1<<3, // 1
    0b01011011, // 2
    1<<0 | 1<<12 | 1<<15 | 1<<2 | 1<<3, // 3
    0b01100110, // 4
    0b01101101, // 5
    1<<0 | 1<<2 | 1<<3 | 1<<4 | 1<<5 | 1<<6, // 6 // old: 1<<6 | 1<<2 | 1<<3 | 1<<4 | 1<<18 | 1<<28, // 6
    1<<23 | 1<<0 | 1<<1 | 1<<13 ,  //0b00000111   , // 7

    0b01111111, // 8
    1<<6 | 1<<5 | 1<<0 | 1<<1 | 1<<2 | 1<<3, // 9
    1<<24 | 1<<25, // :
    1<<8 | 1<<25, // ;
    1<<12 | 1<<11, // <
    1<<6, // = (-)
    1<<16 | 1<<19, // >
    1<<23 | 1<<0 | 1<<1 | 1<<15 | 1<<25 | 1<<9, //  ?

    1<<19 | 1<<12, // @ (/)
    1<<0 | 1<<1 | 1<<2 | 1<<6 | 1<<4 | 1<<5, // A
    1<<26 | 1<<10 | 1<<6 | 1<<11 | 1<<3 | 1<<4 | 1<<5, // B
    1<<0 | 1<<5 | 1<<4 | 1<<3, // C // corners: 1<<28 | 1<<18 | 1<<17 | 1<<29,
    1<<26 | 1<<27 | 1<<5 | 1<<4 | 1<<10 | 1<<13 , // D
    1<<0 | 1<<14 | 1<<3 | 1<<5 | 1<<4, // E
    1<<0 | 1<<14 | 1<<5 | 1<<4, // F
    1<<0 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<15, // G

    1<<1 | 1<<2 | 1<<5 | 1<<4 | 1<<6, // H
    1<<0 | 1<<8 | 1<<9 | 1<<3, // I
    1<<1 | 1<<2 | 1<<3 | 1<<4, // J
    1<<5 | 1<<4 | 1<<14 | 1<<12 | 1<<11, // K
    1<<5 | 1<<4 | 1<<3, // L
    1<<5 | 1<<4 | 1<<1 | 1<<2 | 1<<16 | 1<<12, // M
    1<<5 | 1<<4 | 1<<16 | 1<<11 | 1<<1 | 1<<2, // N
    1<<16 | 1<<11 | 1<<19 | 1<<12 | 1<<6 | 1<<8 | 1<<9, // O - FREE SYMBOL

    1<<4 | 1<<5 | 1<<0 | 1<<1 | 1<<6, // P
    1<<0 | 1<<1 | 1<<13 | 1<<27 | 1<<4 | 1<<5 | 1<<11, // Q // old: 0b00111111 | 1<<11, // Q
    1<<0 | 1<<1 | 1<<6 | 1<<5 | 1<<4 | 1<<11, // R
    (1 << 18 | 1 << 13) | (1 << 29  | 1 << 26) | (1 << 5 | 1 << 2) | (1 << 19 | 1 << 12),  // percent(%) sign,  s == 5 -> // 0b01101101, // S
    1<<0 | 1<<8 | 1<<9, // T
    1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<5, // U
    1<<1 | 1<<13 | 1<<17 | 1<<5, // V
    1<<5 | 1<<4 | 1<<19 | 1<<11 | 1<<2 | 1<<1, // W

    1<<16 | 1<<11 | 1<<19 | 1<<12, // X
    1<<5 | 1<<6 | 1<<1 | 1<<2 | 1<<3, // Y
    1<<0 | 1<<12 | 1<<19 | 1<<3, // Z
    1<<26 | 1<<8 | 1<<9 | 1<<29 | 1<<2 | 1<<15, // [Ъ
    1<<5 | 1<<4 | 1<<27 | 1<<9 | 1<<14 | 1<<1 | 1<<2, // \Ы
    1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<6, // ]Ь
    1<<7 | 1<<5 | 1<<4 | 1<<19 | 1<<12 | 1<<1 | 1<<2, // ^Й
    1<<30, // _ (,comma)

    1<<25, // ` dot
    1<<5 | 1<<0 | 1<<1 | 1<<6 | 1<<2 | 1<<19, // aЯ
    1<<0 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<6, // bБ
    1<<5 | 1<<4 | 1<<3 | 1<<1 | 1<<2 | 1<<21, // cЦ
    0b011001000000010000011100, // dД
    1<<0 | 1<<1 | 1<<15 | 1<<2 | 1<<3, // eЭ // with corners: 1<<26 | 1<<10 | 1<<6 | 1<<13 | 1<<27, // eЭ
    1<<0 | 1<<1 | 1<<6 | 1<<5 | 1<<8 | 1<<9, // fФ
    1<<4 | 1<<5 | 1<<0, // gГ
    1<<5 | 1<<4 | 1<<3 | 1<<1 | 1<<2 | 1<<9, // hШ,
    1<<5 | 1<<4 | 1<<19 | 1<<12 | 1<<1 | 1<<2, // iИ
    1<<5 | 1<<8 | 1<<1 | 1<<6 | 1<<4 | 1<<9 | 1<<2, // old: 1<<16 | 1<<8 | 1<<12 | 1<<19 | 1<<9 | 1<<11, // jЖ
    1<<5 | 1<<4 | 1<<14 | 1<<8 | 1<<9 | 1<<1 | 1<<2 | 1<<28 | 1<<29, // k_Ю
    1<<4 | 1<<18 | 1<<28 | 1<<1 | 1<<2, // l  // old symmetric: 1<<4 | 1<<18 | 1<<10 | 1<<2, // l // old: 1<<19 | 1<<12 | 1<<1 | 1<<2, // lЛ
    1<<5 | 1<<4 | 1<<3 | 1<<1 | 1<<2 | 1<<9 | 1<<21, // mЩ,
    1<<4 | 1<<5 | 1<<0 | 1<<1 | 1<<2, // n
    (1<<8 | 1<<9) | 1<<10 | 1<<18,  // 'o' for arrow up ^

};


void game_font_init() {

    DEBUG30(":: font init start");

    float scale = 1.0;

    int seg;
    int ch;
    int ch_bit;

//  // FAT FONT #1 (OK)
//    float scales[4] = { 0.4, 0.6, 0.2, 2 }; // 2,1,1,1
//    float pows[4] = { 15.0, 19.0, 200.0, 22.0 };
//    float ks1[4] = { 1.2, 1.0, 1.2, 0.90 }; // 0.75 straight
//    float ks2[4] = { -1.0, -1.0, -1.0, -1.0 };
//    float pows1[4] = { 1.0, 0.79, 1.0, 1.0 };
//    float pows2[4] = { 1.0, 0.79, 1.0, 1.0 };
//    float clamp1[4] = { 0.8, 0.75, 0.65, 0.87 }; // 0.8 to 0.86
//    float clamp2[4] = { 0.85, 0.85, 0.85, 0.92 };
//    float radiuses[4] = { 1.0, 1.1, 1.2, 0.87 };
    
    
    float scales[4] = { 0.69, 2.0, 0.2, 0.5 }; // 2,1,1,1
    //float pows[4] = { 45.0, 100.0, 200.0, 22.0 };
    float pows[4] = { 8.0, 7.0, 15.0, 22.0 };
    float ks1[4] = { 0.01, 0.05, 1.2, 0.90 }; // 0.75 straight
    float ks2[4] = { -3.75, -2.75, -1.0, -1.0 };
    float pows1[4] = { 1.0, 1.0, 1.0, 1.0 };
    float pows2[4] = { 0.6,  0.6, 1.0, 1.0 };
    float clamp1[4] = { 0.65, 0.69, 0.65, 0.87 }; // 0.8 to 0.86
    float clamp2[4] = { 0.85, 0.75, 0.85, 0.92 };
    float radiuses[4] = { 0.9667, 0.997, 1.2, 0.87 };
    
    float colors_r[4] = { 0.15, 0.997, 1.0, 0.4 };
    float colors_g[4] = { 0.10, 0.875, 1.0, 0.6 };
    float colors_b[4] = { 0.05, 0.763, 1.0, 0.8 };
    // 1: 0.5, 0.74, 0.79

    DEBUG20(":: font init label-a");

    int font_index = 0;
    int font_index_color = 0;
    
    for (font_index_color = 0; font_index_color < FONTS_COUNT; font_index_color++) {
            
        font_index = font_index_color % 3;

        DEBUG30f(":: font init label-b (font index %d) \n", font_index);

        scale = scales[font_index];

        rs_gen_reg.cell_scale = scale;

        int char_tex_size = scale*32;
        
//        DEBUG10f("char_tex_size %d \n", char_tex_size);
        
        rs_gen_init(97, char_tex_size);

        for (seg = 0; seg < 32; seg++) {
            rs_gen_func_cell(seg, 1, 2, &pp_seg[8*seg], ks1[font_index], pows1[font_index], ks2[font_index], pows2[font_index], 0.0, 0.0);
            //rs_gen_func_cell(seg, 1, 2, &pp_seg[8*seg], 1.2, 1.0, -1.0, 1.0, 0.0, 0.0);
            //rs_gen_func_cell(seg, 1, 2, &pp_seg[8*seg], 0.48450032, 1.0, -1.0, 2.20, 0.0, 0.0); // toon
            rs_gen_func_normalize(seg, 0.0, 1.0); // 0.1 to 1.0

            rs_gen_func_clamp(seg, clamp1[font_index], clamp2[font_index]); //  0.8, 0.86); // toon 0.775, 0.839889
            rs_gen_func_normalize(seg, 0.0, 1.0); // 0.1 to 1.0

            rs_gen_func_set(96, 0.0);
            rs_gen_func_radial(96, (float)pp_seg[8*seg + 4]/32.0, (float)pp_seg[8*seg + 5]/32.0, (float)pp_seg[8*seg + 6]/32.0*radiuses[font_index], 1.0, pows[font_index]);
            rs_gen_func_mult(seg, seg, 96);
        };



        for (ch = 0; ch < 64; ch++) {
            rs_gen_func_set(32+ch, 0.0);
            for (ch_bit = 0; ch_bit < 32; ch_bit++) {
                if ( (1<<ch_bit) & (ch_seg[ch]) ) {
                    rs_gen_func_add_lerped(32+ch, 32+ch, ch_bit, 1.0, 1.0);
                }
            }
            rs_gen_func_clamp(32+ch, (ch+ch/8)%2 ? 0.0 : 0.0, 1.0);
    //        rs_gen_func_set(32+ch, (ch+ch/8)%2 ? 0.5 : 0.0);

            rs_gen_func_set(96, 1.0);
            
            rs_gen_tex_out_rgba_set(0.0, 0.0, 0.0, 0.0);
            //rs_gen_tex_out_rgba(32+ch, 32+ch, 32+ch, 32+ch, colors_b[font_index_color], colors_g[font_index_color], colors_r[font_index_color], 1.0);
            rs_gen_tex_out_rgba(96, 96, 96, 32+ch, colors_b[font_index_color], colors_g[font_index_color], colors_r[font_index_color], 1.0);
            
            texture_init(&game.tex_font[font_index_color*64 + ch], char_tex_size, char_tex_size);
            memcpy(game.tex_font[font_index_color*64 + ch].data, rs_gen_reg.tex_out, char_tex_size*char_tex_size*4 );
    
        }
        
        
        
        /*

        float *fontdata = malloc(256*scale*256*scale*4);

        int i, j, k;
        for (i = 0; i < 8; i++) {
            for (j = 0; j < 8; j++) {
                for (k = 0; k < 32*scale; k++) {
                    memcpy( &(fontdata[i*256*(32*scale)*scale + k*256*scale + (j)*(32*scale)]), &(rs_gen_reg.tex[k*(32*scale) + (32+j+i*8)*(32*scale)*(32*scale)] ), 32*4*scale );
                };
            };
        };

        rs_gen_term();

        int font_tex_size = 256*scale;
        rs_gen_init(2, font_tex_size);
        memcpy( rs_gen_reg.tex, fontdata, 256*256 * 4 *scale*scale);
        rs_gen_func_set(1, 1.0);
        rs_gen_tex_out_rgba_set(0.0, 0.0, 0.0, 0.0);
        rs_gen_tex_out_rgba(1, 1, 1, 0, 1.0, 1.0, 1.0, 1.0);
        
        //game.font_texture[font_index] = rs_tx_create_from_data(font_tex_size, font_tex_size, 4, 1, 0, rs_gen_reg.tex_out);
        game_loader_create_texture( &(game.font_texture[font_index]), font_tex_size, font_tex_size, 4, 1, 0, rs_gen_reg.tex_out );
        

        save_image(0, rs_gen_reg.tex_out, 256*scale, 256*scale, 4);
        
        */

        rs_gen_term();

//        free(fontdata);  
//        
//        game.loader_progress = font_index+2;

    };

    rs_gen_reg.cell_scale = 1; 

};

void game_font_term() { 

    int i;
    for (i = 0; i < 64*FONTS_COUNT; i++) {
        texture_free(&game.tex_font[i]);
    };


};



//float game_colors[4*7] = {
//    0.0, 0.0, 0.0, 1.0,
//    1.0, 1.0, 1.0, 1.0,
//    1.0, 0.9, 0.3, 1.0,
//    1.0, 0.4, 0.3, 1.0,
//    0.6, 0.8, 1.0, 1.0,
//    1.0, 0.8, 0.6, 1.0,
//    0.6, 0.6, 0.6, 0.9,
//};


void game_textout(int x, int y, int font_index, char* s) {
    game_textout_adv(&game.framebuffer, x, y, font_index, DRAW_MODE_ALPHA, s);
};

void game_textout_at_center(int x, int y, int font_index, char *s) {
    x += (GAME_WIDTH - game.tex_font[font_index*64].w*strlen(s))/2; 
    game_textout_adv(&game.framebuffer, x, y, font_index, DRAW_MODE_ALPHA, s);
};

void game_textout_adv(rs_texture_t *dest, int x, int y, int font_index, int draw_mode, char* s) {
    

    int i = 0;
    while (*s) {
        if (*s != ' ') {
            texture_draw(dest, &game.tex_font[ 64*font_index + ((*s - 48) % 64) ], x+i*game.tex_font[64*font_index+0].w, y, draw_mode);
        };
        s++;
        i++;
    };
    

};


