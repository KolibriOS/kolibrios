
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include "font_droid.h"
#include <ft2build.h>
#include FT_FREETYPE_H

typedef unsigned int color_t;

typedef struct
{
    uint32_t    width;
    uint32_t    height;
    uint32_t    pitch;
    uint32_t    handle;
    uint8_t    *data;
}bitmap_t;


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


int draw_text(bitmap_t * winbitmap, FT_Face face, char *text, int x, int y, int color)
{
    FT_UInt glyph_index;
    FT_Bool use_kerning = 0;
    FT_UInt previous;

    char ch;
    int err = 0;

    use_kerning = FT_HAS_KERNING( face );
    previous = 0;

    x <<= 6;

    while( ch = *text++ )
    {
        glyph_index = FT_Get_Char_Index( face, ch );

        if ( use_kerning && previous && glyph_index )
        {
            FT_Vector delta;
            FT_Get_Kerning( face, previous, glyph_index, FT_KERNING_DEFAULT, &delta );
            x += delta.x ;
        }

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

    err = FT_New_Face( library, "/hd0/1/istokweb.ttf", 0, &face );

//    err = FT_New_Memory_Face( library, pdf_font_DroidSans, 139280, 0, &face );
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

    err = FT_Set_Char_Size( face, 0, 12*64, 96, 96 );
//    err = FT_Set_Pixel_Sizes( face, 0, 100 );

done:

    return (int)face;
};

//    draw_text(face,"/hd0/1/demo", 10, 80, 0x00000000);

