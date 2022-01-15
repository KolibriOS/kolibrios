//Sizes
#define PAD 13
#define TOOLBAR_ITEM_H PAD + PAD
#define TOOLBAR_W 136
#define STATUSBAR_H 20
#define HEADERH TOOLBAR_ITEM_H + 14
#define HEADER_TEXTY HEADERH - 14 / 2
#define CANVASX TOOLBAR_W + PAD + PAD
#define CANVASY HEADERH + 2

// Colors
#define COL_WORK        0x242424
#define COL_WORK_TEXT   0xBEBEBE
#define COL_LIGHT       0x424242
#define COL_DARK        0x1D1D1D
#define COL_LINE        0x010101
#define COL_BUTTON      0x181818
#define COL_BUTTON_TEXT 0x18A2CC

block canvas = { CANVASX, CANVASY, NULL, NULL };

EVENTS button;
EVENTS key;

proc_info Form;
int pressed_button_id;

char default_dir[4096] = "/sys";
od_filter filter2 = { 69, "BMP\0GIF\0ICO\0CUR\0JPEG\0JPG\0PNG\0PNM\0TGA\0TIFF\0TIF\0WBMP\0XBM\0XCF\Z80\0\0" };

libimg_image icons18;
libimg_image icons18a;
libimg_image pixie_skin;
libimg_image main_image;

scroll_bar scroll_v = { 15,NULL,NULL,HEADERH+1,15,2,NULL,0,0,COL_DARK,COL_LIGHT,COL_LINE};
scroll_bar scroll_h = { NULL,TOOLBAR_W+PAD+PAD,15,NULL,15,2,NULL,0,0,COL_DARK,COL_LIGHT,COL_LINE};

#define DIMTS 6 //dimenshion text size
char text_w[DIMTS+10], text_h[DIMTS+10]; 
edit_box edit_w = {DIMTS+1*8,08*8+CANVASX+PAD+PAD,HEADER_TEXTY-3,0xEEEeee,0x94AECE,
	0xEEEeee,0xEEEeee,0x10000000,4,#text_w,0,ed_focus+ed_figure_only};
edit_box edit_h = {DIMTS+1*8,17*8+CANVASX+PAD+PAD+PAD-1,HEADER_TEXTY-3,0xEEEeee,
	0x94AECE,0xEEEeee,0xEEEeee,0x10000000,4,#text_h,0,ed_figure_only};

enum { SAVE_AS_PNG=1, SAVE_AS_BMP=2, SAVE_AS_RAW=4, SAVE_AS_PNM=8 };
int saving_type = NULL;

char* libimg_bpp[] = { "-", "8pal", "24", "32", "15", "16",
"mono", "8gray", "2pal", "4pal", "8gr/a" };
int color_depth_btnid_1;

enum {
	TOOL_EXPORT=1,
	TOOL_CROP=2, 
	TOOL_COLOR_DEPTH=4,
	TOOL_FLIP_ROTATE=8,
	TOOL_SCALE=16,
};
int active_tool = NULL;

:checkbox keep_ratio = { "Keep ratio", true };
