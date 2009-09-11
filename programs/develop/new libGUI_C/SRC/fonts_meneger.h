/*
	font meneger header structure
 */

#define	FONT_FLAG_DEFAULT_FONT_ON					0x1
#define	FONT_FLAG_DEFAULT_FONT_OFF					0xfe
#define	FONT_FLAG_DRAW_BACKGROUND_ON				0x2
#define	FONT_FLAG_DRAW_BACKGROUND_OFF				0xfd
#define	FONT_FLAG_ORIENTATION_HORIZONTAL_FROM_LEFT_TO_RIGHT_ON	0x4
#define	FONT_FLAG_ORIENTATION_HORIZONTAL_FROM_LEFT_TO_RIGHT_OFF	0xfb

#define	FONT_CONSTANT_SIZE				-1

//some types encoding characters
#define	FONT_TYPE_ASCII				0x1
#define	FONT_TYPE_UNICODE				0x2

///////////////////////////////////////////////////////////
//		some ASCII encodings
///////////////////////////////////////////////////////////

//cyrillic encodings
#define	FONT_ENCODING_CYRILLIC_IBM866		0x1
#define	FONT_ENCODING_CYRILLIC_IBM437		0x2
#define	FONT_ENCODING_CYRILLIC_KOI8R		0x4
#define	FONT_ENCODING_CYRILLIC_ISO8859_5		0x8
#define	FONT_ENCODING_CYRILLIC_CP1251		0x10

#pragma pack(push,1)
static struct
{
	DWORD	*fnt_fd;
	DWORD	*fnt_bk;
	DWORD	*default_font;
    	DWORD	number_fonts;
}FontsMeneger;
#pragma pack(pop)

#pragma pack(push,1)
struct  FONT
{
	DWORD		*fnt_draw;
	DWORD		*fnt_unpacker;
	DWORD		*fnt_fd;
 	DWORD		*fnt_bk;
	int		sizex;
	int		sizey;
	int		size;
	int		encoding_type;
	char		*font;
	char		*fnt_name;
	DWORD		type;
	DWORD		flags;
};
#pragma pack(pop)

typedef struct	FONT	font_t;

static	char	*default_fonts_path="/sys/fonts/";

void (*DrawFont)(finition_t *fin,int fx,int fy,DWORD color,DWORD background_color,font_t *font,BYTE *s);


