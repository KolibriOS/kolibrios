
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <pixlib2.h>

typedef struct
{
  int  l;
  int  t;
  int  r;
  int  b;
}rect_t;

typedef struct
{
    FT_Face face;
    int     height;
    int     base;

    FT_Glyph glyph[256];

}font_t;

static FT_Face def_face;

typedef unsigned int color_t;

unsigned int ansi2utf32(unsigned char ch);

font_t *create_font(FT_Face face, int size);

void my_draw_bitmap(bitmap_t *win, FT_Bitmap *bitmap, int dstx, int dsty, int col)
{
    uint8_t *dst;
    uint8_t *src, *tmpsrc;

    uint32_t  *tmpdst;
    int i, j;

    dst = win->data + dsty * win->pitch + dstx*4;
    src = bitmap->buffer;

//    printf("buffer %x width %d rows %d\n",
//            bitmap->buffer, bitmap->width, bitmap->rows);


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
            dr = (*tmpdst >> 16) & 0xFF;

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


int draw_text_ext(bitmap_t *winbitmap, font_t *font, char *text, int len, rect_t *rc, int color)
{
    FT_UInt glyph_index;
    FT_Bool use_kerning = 0;
    FT_BitmapGlyph  glyph;
    FT_UInt previous;

    int x, y, w;
    int col, ncol;
    unsigned char ch;
    int err = 0;

    use_kerning = FT_HAS_KERNING( font->face );
    previous = 0;
    col = 0;

    x = rc->l << 6;
    y = rc->b;

    w = (rc->r - rc->l) << 6;

    while( len-- )
    {
        ch = *text++;

        if(ch == '\n' || ch == '\r')
            continue;

        if(ch == '\t')
        {
            ncol = (col+4) & ~3;
            if( col < ncol)
            {
                glyph_index = FT_Get_Char_Index( font->face, ansi2utf32(' ') );

                while( col < ncol)
                {
                    if ( use_kerning && previous && glyph_index )
                    {
                        FT_Vector delta;
                        FT_Get_Kerning( font->face, previous, glyph_index, FT_KERNING_DEFAULT, &delta );
                        x += delta.x ;
                    }

                    if( x + (font->glyph[ch]->advance.x >> 10) > w)
                        break;

                    x += font->glyph[ch]->advance.x >> 10;
                    previous = glyph_index;
                    col ++;
                };
            };
            continue;
        };

        glyph_index = FT_Get_Char_Index( font->face, ansi2utf32(ch) );

        if ( use_kerning && previous && glyph_index )
        {
            FT_Vector delta;
            FT_Get_Kerning( font->face, previous, glyph_index, FT_KERNING_DEFAULT, &delta );
            x += delta.x ;
        }

        if( x + (font->glyph[ch]->advance.x >> 10) > w)
            break;

        glyph = (FT_BitmapGlyph)font->glyph[ch];

        my_draw_bitmap(winbitmap, &glyph->bitmap, (x >> 6) + glyph->left,
                        y - glyph->top, color);

        x += font->glyph[ch]->advance.x >> 10;
        previous = glyph_index;
    };

    return err;
};


int init_fontlib()
{
    static FT_Library library;
    FT_Face face = NULL;
    int err;

    err = FT_Init_FreeType( &library );
    if ( err )
    {
        printf("an error occurred during FreeType initialization\n");
        goto done;
    }

    err = FT_New_Face( library, "/kolibrios/Fonts/IstokWeb.ttf", 0, &face );
//    err = FT_New_Face( library, "/kolibrios/Fonts/lucon.ttf", 0, &face );
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

    def_face = face;

done:

    return err;
};


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


font_t *create_font(FT_Face xface, int size)
{
    font_t *font;
    int i, err;

    font = malloc(sizeof(*font));
    if(font == NULL)
        return font;

    memset(font, 0, sizeof(*font));

    font->face = (xface == NULL) ? def_face : xface;
    font->height = size;

    err = FT_Set_Pixel_Sizes( font->face, 0, size );

    for(i = 0; i < 256; i++)
    {
        FT_UInt glyph_index;
        FT_BitmapGlyph  glyph_bitmap;

        glyph_index = FT_Get_Char_Index( font->face, ansi2utf32(i) );

        err = FT_Load_Glyph( font->face, glyph_index, FT_LOAD_DEFAULT );
        if ( err )
        {
            font->glyph[i] = font->glyph[0] ;
            continue;
        };

        err = FT_Get_Glyph( font->face->glyph, &font->glyph[i] );
        if (err)
        {
            font->glyph[i] = font->glyph[0] ;
            continue;
        };

        if ( font->glyph[i]->format != FT_GLYPH_FORMAT_BITMAP )
        {
            err = FT_Glyph_To_Bitmap( &font->glyph[i], FT_RENDER_MODE_NORMAL, 0, 1 );
            if ( err )
                continue;

            glyph_bitmap = (FT_BitmapGlyph)font->glyph[i];

            if(glyph_bitmap->top > font->base)
                font->base = glyph_bitmap->top;
        }
    }

    return font;
}
