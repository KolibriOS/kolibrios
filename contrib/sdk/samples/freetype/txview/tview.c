#include <stdio.h>
#include <string.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include "winlib.h"

int init_fontlib();
font_t *create_font(FT_Face face, int size);
int get_font_height(font_t *font);
int get_font_width(font_t *font);

int draw_text_ext(ctx_t *ctx, font_t *font, char *text, int len, rect_t *rc, color_t color);

typedef struct
{
    ctx_t    *ctx;
    font_t   *font;

    char    *text;
    char   **line;
    int      lines;

    int      startline;
    int      endline;
    int      pagesize;
    int      tail;

    rect_t   margins;
    int      leading;
    int      line_height;
    int      w;
    int      h;

}tview_t;



tview_t *create_tview(ctx_t *ctx, int width, int height)
{
    tview_t *txv;

    txv = malloc(sizeof(*txv));
    if(txv == NULL)
    {
        return NULL;
    };

    memset(txv, 0, sizeof(*txv));

    txv->ctx = ctx;
    txv->w = width;
    txv->h = height;
    txv->font = create_font(NULL, 18);
    txv->leading = 4;
    txv->line_height = txv->leading + get_font_height(txv->font);
    return txv;
};

void txv_get_margins(const tview_t *txv, rect_t *margins)
{
    *margins = txv->margins;
};

void txv_set_margins(tview_t *txv, const rect_t *margins)
{
    txv->margins = *margins;
};

static void txv_recalc(tview_t *txv)
{
    int full_height;
    int endline;

    full_height = txv->h - (txv->margins.t + txv->margins.b);
    txv->pagesize = full_height / txv->line_height;
    txv->tail = full_height % txv->line_height;
    endline = txv->startline + txv->pagesize;
    txv->endline = endline  < txv->lines ? endline : txv->lines;

//    printf("pagesize %d visible %d endline %d\n",txv->pagesize, txv->pagesize+!!txv->tail, txv->endline );
};

static void txv_redraw(tview_t *txv)
{
    rect_t rc;
    int i;

    rc.l = 0;
    rc.t = 0;
    rc.r = txv->w;
    rc.b = txv->h;

    px_fill_rect(txv->ctx, &rc, 0xFFFFFFFF);

    rc.l = txv->margins.l;
    rc.t = txv->margins.t;
    rc.r = txv->w - txv->margins.r;
    rc.b = rc.t + txv->line_height;

    for(i = txv->startline; i < txv->endline; i++)
    {
        draw_text_ext(txv->ctx, txv->font, txv->line[i], txv->line[i+1]-txv->line[i], &rc, 0xFF000000);
        rc.t+= txv->line_height;
        rc.b+= txv->line_height;
    }
    if(txv->tail && ( i < txv->lines))
        draw_text_ext(txv->ctx, txv->font, txv->line[i], txv->line[i+1]-txv->line[i], &rc, 0xFF000000);
};

void txv_set_text(tview_t *txv, char *text, int size)
{
    int  i = 0;
    char *p, *t;

    p = text;

    while(i < size)
    {
        switch(*p++)
        {
            case '\n':
                txv->lines++;
                break;
            case '\r':
                break;
        }
        i++;
    };

    txv->line = user_alloc((txv->lines+1)*sizeof(char*));
    txv->text = text;
    {
        int  l=0;

        i = 0;
        txv->line[0] = txv->text;

        while(i < size)
        {
            switch(*text++)
            {
                case '\n':
                    l++;
                    txv->line[l] = text;
                    break;
                case '\r':
                    txv->line[l] = text;
                    break;
            }
            i++;
        };
    }

    txv_recalc(txv);
    txv_redraw(txv);
};

int txv_scroll_down(tview_t *txv)
{
    int dst, src, rows;
    rect_t rc;

    if(txv->endline < txv->lines)
    {
        dst  = txv->margins.t;
        src  = dst + txv->line_height;
        rows = txv->line_height * (txv->pagesize-1);
        scroll_context(txv->ctx, dst, src, rows );

        rc.l = txv->margins.l;
        rc.t = txv->margins.t + rows;
        rc.r = txv->w - txv->margins.r;
        rc.b = rc.t + txv->line_height;

        px_fill_rect(txv->ctx, &rc, 0xFFFFFFFF);

        draw_text_ext(txv->ctx, txv->font, txv->line[txv->endline],
                      txv->line[txv->endline+1]-txv->line[txv->endline], &rc, 0xFF000000);

        txv->startline++;
        txv->endline++;

        if( txv->tail && (txv->endline < txv->lines))
        {
            rc.t+= txv->line_height;
            rc.b+= txv->line_height;
            px_fill_rect(txv->ctx, &rc, 0xFFFFFFFF);
            draw_text_ext(txv->ctx, txv->font, txv->line[txv->endline],
                          txv->line[txv->endline+1]-txv->line[txv->endline], &rc, 0xFF000000);
        }
        return 1;
    };

    return 0;
};

int txv_scroll_up(tview_t *txv)
{
    int dst, src, rows;
    rect_t rc;

    if(txv->startline > 0)
    {
        rows = txv->tail + txv->line_height * (txv->pagesize-1);
        src  = txv->margins.t;
        dst  = src + txv->line_height;

        scroll_context(txv->ctx, dst, src, rows);

        rc.l = txv->margins.l;
        rc.t = txv->margins.t;
        rc.r = txv->w - txv->margins.r;
        rc.b = rc.t + txv->line_height;

        px_fill_rect(txv->ctx, &rc, 0xFFFFFFFF);

        txv->startline--;
        txv->endline--;

        draw_text_ext(txv->ctx, txv->font, txv->line[txv->startline],
                      txv->line[txv->startline+1]-txv->line[txv->startline], &rc, 0xFF000000);
        return 1;
    };

    return 0;
};

int txv_page_up(tview_t *txv)
{
    int startline, endline;

    startline = txv->startline - txv->pagesize;
    txv->startline = startline >= 0 ? startline : 0;

    endline = txv->startline + txv->pagesize;
    txv->endline = endline  < txv->lines ? endline : txv->lines;
    txv_redraw(txv);
    return 1;
};

int txv_page_down(tview_t *txv)
{
    int startline, endline;

    endline = txv->endline + txv->pagesize;
    txv->endline = endline  < txv->lines ? endline : txv->lines;
    startline = txv->endline - txv->pagesize;
    txv->startline = startline >= 0 ? startline : 0;
    txv_redraw(txv);
    return 1;
};

void txv_set_size(tview_t *txv, int txw, int txh)
{
    txv->w = txw;
    txv->h = txh;

    txv_recalc(txv);
    txv_redraw(txv);
};

void txv_set_font_size(tview_t *txv, int size)
{

    if(size > 72)
        size = 72;
    else if(size < 6)
        size = 6;

    txv->font = create_font(NULL, size);
    if ( txv->font == NULL )
    {
        printf("cannot create font\n");
        return;
    }

    txv->line_height = txv->leading + get_font_height(txv->font);
    txv_recalc(txv);
    txv_redraw(txv);
}

int  txv_get_font_size(tview_t *txv)
{
    return get_font_height(txv->font);
}
