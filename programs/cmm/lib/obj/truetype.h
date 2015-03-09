char fontinfo[28]; 
 
dword libtruetype = #att_libtruetype;
 
char att_libtruetype[22] = "/sys/lib/truetype.obj\0";
 
dword truetype = #att_truetype;      // truetype(unsigned char *s, stbtt_fontinfo *buffer, char *screen1, int width, int height)
dword get_length = #att_get_length;  // get_length(unsigned char *s, char *buffer, int height, int max_len)
dword get_width  = #att_get_width;   // get_width_utf8(unsigned char *s, stbtt_fontinfo *buffer, int height)
dword text_out = #att_text_out;      // text_out(unsigned char *string, char *buffer, int height, int color,int back_color, int x, int y)
dword init_font = #att_init_font;    // init_font(stbtt_fontinfo *font,unsigned char *FontData)
dword text_out_mem = #att_text_out_mem; // text_out_mem(unsigned char *string, stbtt_fontinfo *buffer, int height, int color,int back_color)
dword text_scale_EM = #att_text_scale;  // scale_EM(stbtt_fontinfo *buffer, int size)
$DD 2 dup 0
 
char att_truetype[]     = "truetype";
char att_get_length[]   = "get_length";
char att_get_width[]    = "get_width";
char att_text_out[]     = "text_out";
char att_init_font[]    = "init_font";
char att_text_out_mem[] = "text_out_mem";
char att_text_scale[]   = "scale_EM";

//============================================================
//============================================================

struct stbtt_fontinfo
{
	dword userdata;
	dword data;            // pointer to .ttf file
	int fontstart;         // offset of start of font
	
	int numGlyphs;                     // number of glyphs, needed for range checking
	
	int loca,head,glyf,hhea,hmtx,kern; // table locations as offset from start of .ttf
	int index_map;                     // a cmap mapping for our chosen character encoding
	int indexToLocFormat;              // format needed to map from glyph index to glyph
};
