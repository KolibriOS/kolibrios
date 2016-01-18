
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cairo.h>
#include <cairo-ft.h>
#include <ft2build.h>
#include <kos32sys.h>
#include FT_FREETYPE_H
#include "tiger.inc"

typedef unsigned int color_t;

int width  = 500;
int height = 400;

cairo_surface_t *main_surface;
unsigned char *winbitmap;
int winstride;

int screen = -1;

cairo_font_face_t    *c_face;

static void draw_next_msg(cairo_t *cr, int width, int height)
{
    cairo_identity_matrix (cr);

    cairo_select_font_face (cr, "cairo:monospace", CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, 28);
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_move_to (cr, 160.0, 360.0);
    cairo_show_text (cr, "Press any key");
};


static void screen_0(cairo_t *cr, int width, int height)
{
    cairo_identity_matrix (cr);

    cairo_set_source_rgb(cr,1,1,1);
    cairo_paint(cr);

    cairo_translate(cr, 50, 30);
    cairo_set_source_rgb(cr,0, 0,0);
    cairo_set_line_width(cr, 10);

    cairo_rectangle(cr, 20, 20, 120, 80);
    cairo_stroke_preserve(cr);
    cairo_set_source_rgb(cr, 0, 0.5, 0.5);
    cairo_fill(cr);

    cairo_set_source_rgb(cr,0, 0,0);
    cairo_rectangle(cr, 180, 20, 80, 80);
    cairo_stroke_preserve(cr);
    cairo_set_source_rgb(cr, 0, 1, 1);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_arc(cr, 330, 60, 40, 0, 2*M_PI);
    cairo_stroke_preserve(cr);
    cairo_set_source_rgb(cr, 0.5, 0.0, 0.5);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_arc(cr, 90, 160, 40, M_PI/4, M_PI);
    cairo_close_path(cr);
    cairo_stroke_preserve(cr);
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.0);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_translate(cr, 220, 180);
    cairo_scale(cr, 1, 0.7);
    cairo_arc(cr, 0, 0, 50, 0, 2*M_PI);
    cairo_stroke_preserve(cr);
    cairo_set_source_rgb(cr, 1, 0, 1);
    cairo_fill(cr);

    draw_next_msg(cr, width, height);

};

static int points[11][2] = {
    { 0, 85 },
    { 75, 75 },
    { 100, 10 },
    { 125, 75 },
    { 200, 85 },
    { 150, 125 },
    { 160, 190 },
    { 100, 150 },
    { 40, 190 },
    { 50, 125 },
    { 0, 85 }
};

static void screen_1(cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    cairo_translate(cr, 20, 40);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 10);
    cairo_move_to(cr, 0, 85);

    int i;
    for ( i = 0; i < 10; i++ ) {
      cairo_line_to(cr, points[i][0], points[i][1]);
    }

    cairo_close_path(cr);
    cairo_stroke_preserve(cr);
    cairo_set_source_rgb(cr, 0, 1, 0);
    cairo_fill(cr);

    cairo_move_to(cr, 240, 40);
    cairo_line_to(cr, 240, 160);
    cairo_line_to(cr, 350, 160);
    cairo_close_path(cr);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_stroke_preserve(cr);
    cairo_set_source_rgb(cr, 1, 0, 0);
    cairo_fill(cr);

    cairo_move_to(cr, 380, 40);
    cairo_line_to(cr, 380, 160);
    cairo_line_to(cr, 450, 160);
    cairo_curve_to(cr, 440, 155, 380, 145, 380, 40);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_stroke_preserve(cr);
    cairo_set_source_rgb(cr, 1, 0, 1);
    cairo_fill(cr);

    draw_next_msg(cr, width, height);

};

static void screen_2(cairo_t *cr, int width, int height)
{
    cairo_pattern_t *pat1;
    cairo_pattern_t *pat2;
    cairo_pattern_t *pat3;


    cairo_translate(cr, 75, 0);

    cairo_set_source_rgb(cr, 1, 1,1);
    cairo_paint(cr);

    cairo_move_to(cr, 0, 85);

    pat1 = cairo_pattern_create_linear(0.0, 0.0,  350.0, 350.0);

    double j;
    int count = 1;
    for ( j = 0.1; j < 1; j += 0.1 )
    {
        if (( count % 2 ))
        {
            cairo_pattern_add_color_stop_rgb(pat1, j, 0, 0, 0);
        }
        else {
          cairo_pattern_add_color_stop_rgb(pat1, j, 1, 0, 0);
        }
        count++;
    }

    cairo_rectangle(cr, 20, 20, 300, 100);
    cairo_set_source(cr, pat1);
    cairo_fill(cr);

    pat2 = cairo_pattern_create_linear(0.0, 0.0,  350.0, 0.0);

    double i;
    count = 1;
    for ( i = 0.05; i < 0.95; i += 0.025 )
    {
        if (( count % 2 ))
        {
            cairo_pattern_add_color_stop_rgb(pat2, i, 0, 0, 0);
        }
        else {
            cairo_pattern_add_color_stop_rgb(pat2, i, 0, 0, 1);
        }
        count++;
    }

    cairo_rectangle(cr, 20, 140, 300, 100);
    cairo_set_source(cr, pat2);
    cairo_fill(cr);

    pat3 = cairo_pattern_create_linear(20.0, 260.0, 20.0, 360.0);

    cairo_pattern_add_color_stop_rgb(pat3, 0.1, 0, 0, 0);
    cairo_pattern_add_color_stop_rgb(pat3, 0.5, 1, 1, 0);
    cairo_pattern_add_color_stop_rgb(pat3, 0.9, 0, 0, 0);

    cairo_rectangle(cr, 20, 260, 300, 100);
    cairo_set_source(cr, pat3);
    cairo_fill(cr);

    cairo_pattern_destroy(pat1);
    cairo_pattern_destroy(pat2);
    cairo_pattern_destroy(pat3);

    draw_next_msg(cr, width, height);

};

static void screen_3(cairo_t *cr, int width, int height)
{
    cairo_translate(cr, 110,20);
    cairo_set_source_rgb(cr, 1, 1,1);
    cairo_paint(cr);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 10);

    cairo_move_to (cr, 128.0, 25.6);
    cairo_line_to (cr, 230.4, 230.4);
    cairo_rel_line_to (cr, -102.4, 0.0);
    cairo_curve_to (cr, 51.2, 230.4, 51.2, 128.0, 128.0, 128.0);
    cairo_close_path (cr);

    cairo_move_to (cr, 64.0, 25.6);
    cairo_rel_line_to (cr, 51.2, 51.2);
    cairo_rel_line_to (cr, -51.2, 51.2);
    cairo_rel_line_to (cr, -51.2, -51.2);
    cairo_close_path (cr);

    cairo_set_line_width (cr, 10.0);
    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_fill_preserve (cr);
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_stroke (cr);

    draw_next_msg(cr, width, height);
}

static void screen_hello(cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
    cairo_paint(cr);

    cairo_set_line_width(cr, 1);

    cairo_select_font_face (cr, "cairo:sans", CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_BOLD);

    cairo_set_font_size (cr, 200);

    cairo_set_source_rgb(cr, 0, 1.0,0);

    cairo_move_to (cr, 20.0, 145.0);
    cairo_show_text (cr, "Hello");

#if 0
    cairo_move_to (cr, 70.0, 170.0);
    cairo_text_path (cr, "world");
    cairo_set_source_rgb (cr, 0.5, 0.5, 1);
    cairo_fill_preserve (cr);
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_set_line_width (cr, 2.56);
    cairo_stroke (cr);


#else
    cairo_set_font_size (cr, 140);
    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_move_to (cr, 80.0, 175.0);
    cairo_show_text (cr, "world");
#endif

    cairo_identity_matrix (cr);

    cairo_select_font_face (cr, "cairo:sans", CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, 16);
    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_move_to (cr, 160.0, 210.0);
    cairo_show_text (cr, "www.cairographics.org");

    draw_next_msg(cr, width, height);
};

static void draw_tiger(cairo_t *cr, int width, int height)
{
	unsigned int i;

	cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba (cr, 0.1, 0.2, 0.3, 1.0);
	cairo_paint (cr);
	cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

	cairo_translate (cr, width/3, height/3);
	cairo_scale (cr, .85, .85);

	for (i = 0; i < sizeof (tiger_commands)/sizeof(tiger_commands[0]);i++) {
		const struct command *cmd = &tiger_commands[i];
		switch (cmd->type) {
		case 'm':
			cairo_move_to (cr, cmd->x0, cmd->y0);
			break;
		case 'l':
			cairo_line_to (cr, cmd->x0, cmd->y0);
			break;
		case 'c':
			cairo_curve_to (cr,
					cmd->x0, cmd->y0,
					cmd->x1, cmd->y1,
					cmd->x2, cmd->y2);
			break;
		case 'f':
			cairo_set_source_rgba (cr,
					       cmd->x0, cmd->y0, cmd->x1, cmd->y1);
			cairo_fill (cr);
			break;
		}
	}
}

static void (*draw_screen[5])(cairo_t *cr, int width, int height)=
    {
        screen_0,
        screen_1,
        screen_2,
        screen_3,
        draw_tiger
    };


int check_events(cairo_t *cr)
{
    uint32_t ev;
    oskey_t   key;
    int retval = 1;

    ev = get_os_event();

    switch(ev)
    {
        case 1:
            BeginDraw();
            DrawWindow(0,0,0,0,NULL,0,0x73);
            Blit(winbitmap, TYPE_3_BORDER_WIDTH, get_skin_height(),
                 0, 0, width, height,width,height,winstride);
            EndDraw();
            break;

        case 2:
            key = get_key();

            if(1 == key.val)
                break;

            if(0x8000 & key.val)
                break;

            if(0x0100 == key.val)
            {
                retval = 0;
                break;
            };
            if(++screen > 4){screen = 0;};
            draw_screen[screen](cr,width,height);
            Blit(winbitmap, TYPE_3_BORDER_WIDTH, get_skin_height(),
                 0, 0, width, height,width,height,winstride);
            break;

        case 3:
            if(get_os_button()==1)
                retval = 0;
            break;
    };
    return retval;
}

#if 0
static void rounded_flat(cairo_t *cr, int x, int y, int w, int h)
{
    double radius = 4;
    double degrees = M_PI / 180.0;

    cairo_new_sub_path (cr);
    cairo_arc (cr, x + w - radius, y + radius, radius, -90 * degrees, 0 * degrees);
    cairo_arc (cr, x + w - radius, y + h - radius, radius, 0 * degrees, 90 * degrees);
    cairo_arc (cr, x +radius, y + h - radius, radius, 90 * degrees, 180 * degrees);
    cairo_arc (cr, x +radius, y + radius, radius, 180 * degrees, 270 * degrees);
    cairo_close_path (cr);

    cairo_set_source_rgb (cr, 0.8, 0.8, 0.8);
    cairo_fill_preserve (cr);
    cairo_set_source_rgba (cr, 0.2, 0.2, 0.2, 0.5);
    cairo_set_line_width (cr, 2.0);
    cairo_stroke (cr);
};

static void rounded(cairo_t *cr, int x, int y, int w, int h)
{
    cairo_pattern_t *pat;
    double radius = 5;
    double degrees = M_PI / 180.0;

    pat = cairo_pattern_create_linear (x, y,  x, y+h);
    cairo_pattern_add_color_stop_rgb (pat, 0, 0.7, 0.7, 0.7);
    cairo_pattern_add_color_stop_rgb (pat, 0.125, 0.85, 0.85, 0.85);
    cairo_pattern_add_color_stop_rgb (pat, 0.25, 1.0, 1.0, 1.0);
    cairo_pattern_add_color_stop_rgb (pat, 0.75, 1.0, 1.0, 1.0);
    cairo_pattern_add_color_stop_rgb (pat, 1, 0.6, 0.6, 0.6);

    cairo_new_sub_path (cr);
    cairo_arc (cr, x + w - radius, y + radius, radius, -90 * degrees, 0 * degrees);
    cairo_arc (cr, x + w - radius, y + h - radius, radius, 0 * degrees, 90 * degrees);
    cairo_arc (cr, x +radius, y + h - radius, radius, 90 * degrees, 180 * degrees);
    cairo_arc (cr, x +radius, y + radius, radius, 180 * degrees, 270 * degrees);
    cairo_close_path (cr);

    cairo_set_source (cr, pat);
    cairo_fill_preserve (cr);

    cairo_set_source_rgba (cr, 0.3, 0.3, 0.3, 0.5);
    cairo_set_line_width (cr, 1.0);
    cairo_stroke (cr);
    cairo_pattern_destroy (pat);
};

static void rounded_hl(cairo_t *cr, int x, int y, int w, int h)
{
    cairo_pattern_t *pat;
    double radius = 5;
    double degrees = M_PI / 180.0;

    pat = cairo_pattern_create_linear (x, y,  x, y+h);
    cairo_pattern_add_color_stop_rgb (pat, 0, 0.7, 0.7, 0.7);
    cairo_pattern_add_color_stop_rgb (pat, 0.125, 0.85, 0.85, 0.85);
    cairo_pattern_add_color_stop_rgb (pat, 0.25, 1.0, 1.0, 1.0);
    cairo_pattern_add_color_stop_rgb (pat, 0.75, 1.0, 1.0, 1.0);
    cairo_pattern_add_color_stop_rgb (pat, 1, 0.6, 0.6, 0.6);

    cairo_new_sub_path (cr);
    cairo_arc (cr, x + w - radius, y + radius, radius, -90 * degrees, 0 * degrees);
    cairo_arc (cr, x + w - radius, y + h - radius, radius, 0 * degrees, 90 * degrees);
    cairo_arc (cr, x +radius, y + h - radius, radius, 90 * degrees, 180 * degrees);
    cairo_arc (cr, x +radius, y + radius, radius, 180 * degrees, 270 * degrees);
    cairo_close_path (cr);

    cairo_set_source (cr, pat);
    cairo_fill_preserve (cr);

    cairo_set_source_rgba (cr, 0, 0.6, 1, 1);
    cairo_set_line_width (cr, 2.0);
    cairo_stroke (cr);
    cairo_pattern_destroy (pat);

    cairo_select_font_face (cr, "cairo:monospace", CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_BOLD);

    cairo_text_extents_t extents;

    cairo_set_font_size (cr, 20);
    cairo_text_extents (cr, "Cancel", &extents);

    x+= w/2 -(extents.width/2 + extents.x_bearing);
    y+= h/2 - (extents.height/2 + extents.y_bearing);

    cairo_move_to (cr, x, y );
    cairo_set_source_rgb (cr, 0, 0, 0);

    cairo_show_text (cr, "Cancel");
    cairo_set_source_rgb (cr, 0, 0, 0);
};
#endif

static cairo_surface_t* main_surface_create(int width, int height)
{

    cairo_surface_t *surface;

    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);

    return surface;
}

int main ()
{
    int tmp;
    cairo_t *cr;

    __asm__ __volatile__(
    "int $0x40"
    :"=a"(tmp)
    :"a"(66),"b"(1),"c"(1));

    __asm__ __volatile__(
    "int $0x40"
    ::"a"(40), "b"(0xc0000027));

    main_surface = main_surface_create(width, height);
    cr = cairo_create(main_surface);

    winstride = cairo_image_surface_get_stride(main_surface);
    winbitmap = cairo_image_surface_get_data(main_surface);

    screen_hello(cr,width,height);

    BeginDraw();
    DrawWindow(30, 40, width+TYPE_3_BORDER_WIDTH*2,
               height+TYPE_3_BORDER_WIDTH+get_skin_height(), "Cairo demo", 0x000000, 0x73);
    Blit(winbitmap, TYPE_3_BORDER_WIDTH, get_skin_height(), 0, 0, width, height,
         width,height,winstride);
    EndDraw();

    while( check_events(cr));

    cairo_destroy(cr);
    cairo_surface_destroy(main_surface);

    return 0;
}
