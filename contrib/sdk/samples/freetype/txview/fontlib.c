
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <pxdraw.h>

typedef struct
{
    FT_Face face;
    int     fontsize;
    int     height;
    int     base;

    FT_Glyph glyph[256];

}font_t;

static FT_Face def_face;
static FT_Face sym_face;

font_t *sym_font;

typedef unsigned int color_t;

unsigned int ansi2utf32(unsigned char ch);

font_t *create_font(FT_Face face, int size);

void draw_glyph(ctx_t *ctx, void *buffer, int pitch, rect_t *rc, color_t color);

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

int draw_text_ext(ctx_t *ctx, font_t *font, char *text, int len, rect_t *rc, color_t color)
{
    FT_UInt glyph_index;
    FT_Bool use_kerning = 0;
    FT_BitmapGlyph  glyph;
    FT_UInt previous;

    int x, y, xend;
    int col, ncol;
    unsigned char ch;
    int err = 0;

    use_kerning = FT_HAS_KERNING( font->face );
    previous = 0;
    col = 0;

    x = rc->l << 6;
    xend = rc->r << 6;
    y = rc->t + font->base;

    while( len-- )
    {
        ch = *text++;

        if(ch == '\n' || ch == '\r')
            continue;

        if(ch == '\t')
        {
            ncol = (col+3) & ~4;
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

                    if( x + (font->glyph[ch]->advance.x >> 10) >= xend)
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

//        if( x + (font->glyph[ch]->advance.x >> 10) >= xend)
//            break;

        if( x  >= xend)
            break;

        glyph = (FT_BitmapGlyph)font->glyph[ch];

        if(glyph != NULL)
        {
            rect_t rc_dst;

            rc_dst.l = (x >> 6) + glyph->left;
            rc_dst.t = y - glyph->top;
            rc_dst.r = rc_dst.l + glyph->bitmap.width;
            if(rc_dst.r > (xend >> 6))
                rc_dst.r = xend >> 6;
            rc_dst.b = rc_dst.t + glyph->bitmap.rows;

//            printf("char: %c ", ch);
            px_draw_glyph(ctx, glyph->bitmap.buffer, glyph->bitmap.pitch, &rc_dst, color);

            x += font->glyph[ch]->advance.x >> 10;
        };
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

//    err = FT_New_Face( library, "/kolibrios/Fonts/IstokWeb.ttf", 0, &face );
//    err = FT_New_Face( library, "/kolibrios/Fonts/lucon.ttf", 0, &face );

    err = FT_New_Face( library, "/kolibrios/Fonts/DroidSansMono.ttf", 0, &face );
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

    err = FT_New_Face( library, "/kolibrios/Fonts/Symbols.ttf", 0, &face );
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

    sym_face = face;

    sym_font = create_font(sym_face, 14);

done:

    return err;
};




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

int get_font_height(font_t *font)
{
    return font->height;
}

