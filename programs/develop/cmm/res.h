#define CRT_NEWRESOURCE 0x2000
#define CRT_ERROR       0x7FFF
#define CRT_CURSOR	1
#define CRT_BITMAP	2
#define CRT_ICON	3
#define CRT_MENU	4
#define CRT_DIALOG	5
#define CRT_STRING	6
#define CRT_FONTDIR	7
#define CRT_FONT	8
#define CRT_ACCELERATOR	9
#define CRT_RCDATA	10
#define CRT_MESSAGETABLE	11
#define CRT_GROUP_CURSOR	12
#define CRT_GROUP_ICON	14
#define CRT_VERSION	16
#define CRT_DLGINCLUDE	17
#define CRT_PLUGPLAY	19
#define CRT_VXD	20
#define CRT_ANICURSOR	21
#define CRT_ANIICON	22
#define CRT_NEWBITMAP (CRT_BITMAP|CRT_NEWRESOURCE)
#define CRT_NEWMENU   (CRT_MENU|CRT_NEWRESOURCE)
#define CRT_NEWDIALOG (CRT_DIALOG|CRT_NEWRESOURCE)

#define TOTALTYPERES 22
#define NUMMENUPOPUP 8

struct RES{
	int type;	//тип ресурсв
	char *tname;	//имя типа
	int id;   //его id
	char *name;	//имя ресурса
	unsigned short lang;	//язык
	unsigned char *res;	//указатель на таблицу ресурса
	unsigned int size;	//размер таблицы
};

#define DRESNUM 100
#define SIZERESBUF 2048

struct _STRINGS_{
	char *id;
	short val;
};

_STRINGS_ typemem[7]={
	"MOVEABLE",   0x0010,
	"FIXED",      ~0x0010,
	"PURE",       0x0020,
	"IMPURE",     ~0x0020,
	"PRELOAD",    0x0040,
	"LOADONCALL", ~0x0040,
	"DISCARDABLE",0x1000
};

_STRINGS_ typeclass[6]={
	"BUTTON",   0x80,
	"EDIT",     0x81,
	"STATIC",   0x82,
	"LISTBOX",  0x83,
	"SCROLLBAR",0x84,
	"COMBOBOX", 0x85
};

_STRINGS_ typemenu[NUMMENUPOPUP]={
	"GREYED",      0x0001,
	"INACTIVE",    0x0002,
	"BITMAP",      0x0004,
	"OWNERDRAW",   0x0100,
	"CHECKED",     0x0008,
	"MENUBARBREAK",0x0020,
	"MENUBREAK",   0x0040,
	"HELP",        0x4000
};

_STRINGS_ typeacceler[5]={
	"VIRTKEY",  0x01,
	"NOINVERT", 0x02,
	"SHIFT",    0x04,
	"CONTROL",  0x08,
	"ALT",      0x10
};

enum {v_fv=1,v_pv,v_ffm,v_ff,v_fo,v_ft,v_fs};

_STRINGS_ typeversion[7]={
	"FILEVERSION",v_fv,
	"PRODUCTVERSION",v_pv,
	"FILEFLAGSMASK",v_ffm,
	"FILEFLAGS",v_ff,
	"FILEOS",v_fo,
	"FILETYPE",v_ft,
	"FILESUBTYPE",v_fs
};

enum{
rc_accelerators,rc_auto3state,     rc_autocheckbox,rc_autoradiobutton,rc_bitmap,
rc_caption,     rc_characteristics,rc_checkbox,    rc_class,          rc_combobox,
rc_control,     rc_ctext,          rc_cursor,      rc_defpushbutton,  rc_dialog,
rc_dialogex,    rc_edittext,       rc_exstyle,     rc_font,           rc_groupbox,
rc_icon,        rc_listbox,        rc_ltext,       rc_menu,           rc_menuex,
rc_menuitem,    rc_messagetable,   rc_popup,       rc_pushbox,        rc_pushbutton,
rc_radiobutton, rc_rcdata,         rc_rtext,       rc_scrollbar,      rc_state3,
rc_stringtable, rc_style,          rc_version,     rc_versioninfo,    rc_begin,
rc_end,         rc_language
};

struct{
	unsigned short dclass;
	unsigned long style;
}defdialog[rc_state3+1]={
	0,0,
	0x80,6,//BS_AUTO3STATE
	0X80,3|0x00010000,//BS_AUTOCHECKBOX|WS_TABSTOP,
	0X80,9,//BS_AUTORADIOBUTTON,
	0,0,
	0,0,
	0,0,
	0X80,0x00010002,//BS_CHECKBOX|WS_TABSTOP,
	0,0,
	0X85,0x00010000,//0,WS_TABSTOP
	0,0x40000000|0x10000000,//WS_CHILD|WS_VISIBLE,
	0X82,1,//ES_CENTER,
	0,0,
	0X80,1|0x00010000,//BS_DEFPUSHBUTTON|WS_TABSTOP,
	0,0,
	0,0,
	0X81,0x00800000|0x00010000,//ES_LEFT|WS_BORDER|WS_TABSTOP,
	0,0,
	0,0,
	0X80,7|0x00010000,//BS_GROUPBOX,
	0X82,3,//SS_ICON,
	0X83,0x00800000|1,//WS_BORDER|LBS_NOTIFY,
	0X82,0x00020000,//ES_LEFT|WS_GROUP,
	0,0,
	0,0,
	0,0,
	0,0,
	0,0,
	0X80,0x00010000,// ??? BS_PUSHBOX,
	0X80,0x00010000,//BS_PUSHBUTTON|WS_TABSTOP,
	0X80,4,//BS_RADIOBUTTON,
	0,0,
	0X82,2|0x00020000,//ES_RIGHT|WS_GROUP,
	0X84,0,
	0X80,5//BS_3STATE
};


union NameOrdinal
{
	unsigned char *name;
	unsigned short ordinal[2];
};


struct _DBH_	//структура диалога
{
	unsigned long lStyle;
	unsigned long lExtendedStyle;
	unsigned short NumberOfItems;
	unsigned short x;
	unsigned short y;
	unsigned short cx;
	unsigned short cy;
	NameOrdinal MenuName;
	NameOrdinal ClassName;
	char *Caption;
	unsigned short FontSize;
	char *FontName;
};

struct _CD_	//контрольные данные диалога
{
	unsigned long lStyle;
	unsigned long lExtendedStyle;
	unsigned short x;
	unsigned short y;
	unsigned short cx;
	unsigned short cy;
	unsigned short Id;
	NameOrdinal ClassId;
	NameOrdinal Text;
	unsigned short Extra;
};

struct _ICOHEAD_
{
	unsigned short res1;
	unsigned short type;
	unsigned short count;
//	unsigned short res2;
};

struct _RESDIR_
{
	unsigned char width;
	unsigned char heigth;
	unsigned char color;
	unsigned char res1;
	unsigned short planes;
	unsigned short bitcount;
	unsigned long binres;
	unsigned short nameord;
//	unsigned short res2;
};

struct _CURDIR_
{
	unsigned short width;
	unsigned short heigth;
	unsigned short planes;
	unsigned short bitcount;
	unsigned long binres;
	unsigned short nameord;
//	unsigned short res2;
};
