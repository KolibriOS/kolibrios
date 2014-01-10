
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
//#include "font_droid.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <pixlib2.h>

extern char res_def_font[];

typedef struct
{
  int  l;
  int  t;
  int  r;
  int  b;
}rect_t;

typedef unsigned int color_t;

unsigned int ansi2utf32(unsigned char ch);

void my_draw_bitmap(bitmap_t *win, FT_Bitmap *bitmap, int dstx, int dsty, int col)
{
    uint8_t *dst;
    uint8_t *src, *tmpsrc;

    uint32_t  *tmpdst;
    int i, j;

    dst = win->data + dsty * win->pitch + dstx*4;
    src = bitmap->buffer;

    for( i = 0; i < bitmap->rows; i++ )
    {
        tmpdst = (uint32_t*)dst;
        tmpsrc = src;

        dst+= win->pitch;
        src+= bitmap->pitch;

        for( j = 0; j < bitmap->width; j++)
        {
            int a = *tmpsrc++;
            int sr, sg, sb;
            int dr, dg, db;

            if( a != 0) a++;

            db = *tmpdst & 0xFF;
            dg = (*tmpdst >> 8) & 0xFF;
            dr = (*tmpdst >> 16) &0xFF;

            sb = col & 0xFF;
            sg = (col >> 8) & 0xFF;
            sr = (col >> 16) &0xFF;

            db = (a*sb + db*(256-a))/256;
            dg = (a*sg + dg*(256-a))/256;
            dr = (a*sr + dr*(256-a))/256;

            *tmpdst++ = 0xFF000000|(dr<<16)|(dg<<8)|db;
        };
    }
};


int draw_text_ext(bitmap_t *winbitmap, FT_Face face, char *text, rect_t *rc, int color)
{
    FT_UInt glyph_index;
    FT_Bool use_kerning = 0;
    FT_UInt previous;
    int x, y, w;
    char ch;
    int err = 0;

    use_kerning = FT_HAS_KERNING( face );
    previous = 0;

    x = rc->l << 6;
    y = rc->b;

    w = (rc->r - rc->l) << 6;

    while( ch = *text++ )
    {
        glyph_index = FT_Get_Char_Index( face, ansi2utf32(ch) );

        if ( use_kerning && previous && glyph_index )
        {
            FT_Vector delta;
            FT_Get_Kerning( face, previous, glyph_index, FT_KERNING_DEFAULT, &delta );
            x += delta.x ;
        }

        if( x + face->glyph->advance.x > w)
            break;

        err = FT_Load_Glyph( face, glyph_index, FT_LOAD_DEFAULT );
        if ( err )
            continue;

        err = FT_Render_Glyph( face->glyph, FT_RENDER_MODE_NORMAL );
        if ( err )
            continue;

        my_draw_bitmap(winbitmap, &face->glyph->bitmap, (x >> 6) + face->glyph->bitmap_left,
                        y - face->glyph->bitmap_top, color);

        x += face->glyph->advance.x;
        previous = glyph_index;
    };

    return err;
};


int init_fontlib()
{
    int err;

    static FT_Library library;
    FT_Face face = NULL;

    err = FT_Init_FreeType( &library );
    if ( err )
    {
        printf("an error occurred during FreeType initialization\n");
        goto done;
    }

//    err = FT_New_Face( library, "/hd0/1/IstokWeb.ttf", 0, &face );

    err = FT_New_Memory_Face( library, res_def_font, 277996, 0, &face );
    printf("err %d\n", err); 
    if ( err == FT_Err_Unknown_File_Format )
    {
        printf("font format is unsupported\n");
        goto done;

    }
    else if ( err )
    {
        printf("font file could not be read or broken\n");
        goto done;

    }

    err = FT_Set_Char_Size( face, 0, 11*64, 96, 96 );
//    err = FT_Set_Pixel_Sizes( face, 0, 100 );

done:

    return (int)face;
};

//    draw_text(face,"/hd0/1/demo", 10, 80, 0x00000000);


unsigned int ansi2utf32(unsigned char ch)
{
    if(ch < 0x80)
        return ch;

    if(ch < 0xB0)
        return 0x410-0x80 + ch;

    if(ch < 0xE0)
        return 0;

    if(ch < 0xF0)
        return 0x440-0xE0 + ch;

    if(ch == 0xF0)
        return 0x401;
    else if(ch==0xF1)
        return 0x451;
    else return 0;
}


