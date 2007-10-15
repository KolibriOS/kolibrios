

#include "./include/kolibrisys.h"
#include "./include/stdlib.h"


#define	Cannot_load_libGUI		100
#define ParendSize			44
#define MessageSize			16

#define TOTAL_NUMBERS_OF_CONTROLS	150

#define WINDOW_POSX			50
#define WINDOW_POSY			50
#define WINDOW_SIZEX			512
#define WINDOW_SIZEY			384

#define FONT_SIZEX			6

#define BOOKMARK1_POSX			0
#define BOOKMARK1_POSY			55
#define BOOKMARK1_SIZEX			210
#define BOOKMARK2_POSX			215
#define BOOKMARK2_POSY			55

#define SCREEN_SIZEX			1024
#define SCREEN_SIZEY			768

#define CONTROLS_NUMBER_POS		4

#define DISTANCE1			130

struct WINDOW
{
  int	posx;
  int	posy;
  int	sizex;
  int	sizey;
};

struct HEADER
{
 dword ctrl_proc;
 dword *ctrl_fd;
 dword *ctrl_bk;
 dword *child_fd;
 dword *child_bk;
 dword *parend;
 dword ctrl_x;
 dword ctrl_y;
 dword ctrl_sizex;
 dword ctrl_sizey;
 dword ctrl_ID;

}__attribute__((packed));;

struct MESSAGE
{
 dword type;
 dword arg1;
 dword arg2;
 dword arg3;
}__attribute__((packed));

struct ControlButton
{
 dword ctrl_proc;
 dword *ctrl_fd;
 dword *ctrl_bk;
 dword *child_fd;
 dword *child_bk;
 dword *parend;
 dword ctrl_x;
 dword ctrl_y;
 dword ctrl_sizex;
 dword ctrl_sizey;
 dword ctrl_ID;

 byte  type;
 byte  flag;
 word  x;
 word  y;
 word  width;
 word  height;
 dword image;
 word  imageX;
 word  imageY;
 word  imageSizeX;
 word  imageSizeY;
 dword transparentColor;
 dword text;
 word  textX;
 word  textY;
 dword textcolor;
 dword color1;
 dword color2;
 word  mouseX;
 word  mouseY;
}__attribute__((packed));;

struct ControlScroller
{
 dword ctrl_proc;
 dword *ctrl_fd;
 dword *ctrl_bk;
 dword *child_fd;
 dword *child_bk;
 dword *parend;
 dword ctrl_x;
 dword ctrl_y;
 dword ctrl_sizex;
 dword ctrl_sizey;
 dword ctrl_ID;

 byte  type;
 word  x;
 word  y;
 word  length;
 dword color1;
 float size;
 float pos;
 word  buttons_flags;
 dword ChildButton1;
 dword ChildButton2;
 word  MouseX;
 word  MouseY;
}__attribute__((packed));;

struct ControlProgressbar
{
 dword ctrl_proc;
 dword *ctrl_fd;
 dword *ctrl_bk;
 dword *child_fd;
 dword *child_bk;
 dword *parend;
 dword ctrl_x;
 dword ctrl_y;
 dword ctrl_sizex;
 dword ctrl_sizey;
 dword ctrl_ID;

 byte  type;
 byte  flag;
 dword color1;
 dword color2;
 dword x;
 dword y;
 dword sizex;
 dword sizey;
 float progress;
 dword color3;
}__attribute__((packed));;


typedef struct InfoForButton
{
 byte  type;		//0
 byte  flag;		//1
 word  x;		//2
 word  y;		//4
 word  width;		//6
 word  height;		//8
 dword image;		//10 
 word  imageX;		//14
 word  imageY;		//16
 word  imageSizeX;	//18
 word  imageSizeY;	//20
 dword transparentColor;//24
 dword text;		//28
 word  textX;		//32
 word  textY;		//34
 dword textcolor;	//36
 dword color1;		//40
 dword color2;
 word  mouseX;
 word  mouseY;
}__attribute__((packed));

struct InfoForScroller
{
 byte  type;
 word  x;
 word  y;
 word  length;
 dword color1;
 float size;
 float pos;
 word  buttons_flags;
 dword ChildButton1;
 dword ChildButton2;
 word  MouseX;
 word  MouseY;
}__attribute__((packed));

struct InfoForProgressbar
{
 byte  type;
 byte  flag;
 dword color1;
 dword color2;
 dword x;
 dword y;
 dword sizex;
 dword sizey;
 float progress;
 dword color3;
}__attribute__((packed));


struct InfoForImage
{
 byte  type;
 byte  flag;
 dword color1;
 dword x;
 dword y;
 dword sizex;
 dword sizey;
 dword pointer;
}__attribute__((packed));

struct InfoForText
{
 byte  type;
 byte  flag;
 dword color1;
 dword x;
 dword y;
 dword length;
 dword text;
}__attribute__((packed));

struct InfoForNumber
{
 byte  type;
 byte  flag;
 dword color1;
 dword x;
 dword y;
 float number;
 dword format;
}__attribute__((packed));

struct ControlImage
{
 dword ctrl_proc;
 dword *ctrl_fd;
 dword *ctrl_bk;
 dword *child_fd;
 dword *child_bk;
 dword *parend;
 dword ctrl_x;
 dword ctrl_y;
 dword ctrl_sizex;
 dword ctrl_sizey;
 dword ctrl_ID;

 byte  type;
 byte  flag;
 dword color1;
 dword x;
 dword y;
 dword sizex;
 dword sizey;
 dword pointer;
}__attribute__((packed));

struct ControlText
{
 dword ctrl_proc;
 dword *ctrl_fd;
 dword *ctrl_bk;
 dword *child_fd;
 dword *child_bk;
 dword *parend;
 dword ctrl_x;
 dword ctrl_y;
 dword ctrl_sizex;
 dword ctrl_sizey;
 dword ctrl_ID;

 byte  type;
 byte  flag;
 dword color1;
 dword x;
 dword y;
 dword length;
 dword text;
}__attribute__((packed));

struct ControlNumber
{
 dword ctrl_proc;
 dword *ctrl_fd;
 dword *ctrl_bk;
 dword *child_fd;
 dword *child_bk;
 dword *parend;
 dword ctrl_x;
 dword ctrl_y;
 dword ctrl_sizex;
 dword ctrl_sizey;
 dword ctrl_ID;

 byte  type;
 byte  flag;
 dword color1;
 dword x;
 dword y;
 float number;
 dword format;
}__attribute__((packed));

//////////////////////////////Info for Bookmark 1///////////////////////////////
struct InfoForBookmark1
{
 byte	type;
 byte	flag;
 dword	x;
 dword	y;
 dword	sizex;
 dword	sizey;
 dword	color1;
 dword	reserved;
 dword	color2;

 dword	number_lines_bookmarks;
 dword	number_lines_bookmarks_in_1;
 dword	number_lines_bookmarks_in_2;

 dword	text_for_1;
 dword	number_controls_1;
 dword	type_control_10;
 dword	info_for_10_control;

 dword  a1,b1;
 dword  a2,b2;
 dword  a3,b3;
 dword  a4,b4;

 dword  a5,b5;
 dword  a6,b6;
 dword  a7,b7;
 dword  a8,b8;

 dword	text_for_2;
 dword	number_controls_2;
 dword	type_control_20;
 dword	info_for_20_control;

 dword	a9,b9;
 dword	a10,b10;
 dword	a11,b11;
 dword	a12,b12;
 dword	a13,b13;
 dword	a14,b14;
 dword	a15,b15;

 dword  a16,b16;
 dword  a17,b17;
 dword  a18,b18;
 dword  a19,b19;
 dword  a20,b20;
 dword  a21,b21;
 dword  a22,b22;

 dword	text_for_3;
 dword	number_controls_3;
 dword	type_control_30;
 dword	info_for_30_control;

//filler 256
 dword	a23,b23;
 dword	a24,b24;
 dword	a25,b25;
 dword	a26,b26;
 dword	a27,b27;
 dword	a28,b28;
 dword	a29,b29;
 dword  a30,b30;
 dword  a31,b31;
 dword  a32,b32;
 dword  a33,b33;
 dword  a34,b34;
 dword  a35,b35;
 dword  a36,b36;
 dword  a37,b37;
 dword  a38,b38;

 dword	a39,b39;
 dword	a40,b40;
 dword	a41,b41;
 dword	a42,b42;
 dword	a43,b43;
 dword	a44,b44;
 dword	a45,b45;
 dword  a46,b46;
 dword  a47,b47;
 dword  a48,b48;
 dword  a49,b49;
 dword  a50,b50;
 dword  a51,b51;
 dword  a52,b52;
 dword  a53,b53;
 dword  a54,b54;

 dword	text_for_4;
 dword	number_controls_4;
 dword	type_control_40;
 dword	info_for_40_control;

//filler 64
 dword  a55,b55;
 dword  a56,b56;
 dword  a57,b57;
 dword  a58,b58;

 dword  a59,b59;
 dword  a60,b60;
 dword  a61,b61;
 dword  a62,b62;

 dword	text_for_5;
 dword	number_controls_5;
 dword	type_control_50;
 dword	info_for_50_control;

}__attribute__((packed));

struct ControlBookmark1
{
 dword ctrl_proc;
 dword *ctrl_fd;
 dword *ctrl_bk;
 dword *child_fd;
 dword *child_bk;
 dword *parend;
 dword ctrl_x;
 dword ctrl_y;
 dword ctrl_sizex;
 dword ctrl_sizey;
 dword ctrl_ID;

 byte	type;
 byte	flag;
 dword	x;
 dword	y;
 dword	sizex;
 dword	sizey;
 dword	color1;
 dword	reserved;
 dword	color2;

 dword	number_lines_bookmarks;
 dword	number_lines_bookmarks_in_1;
 dword	number_lines_bookmarks_in_2;

 dword	text_for_1;
 dword	number_controls_1;
 dword	type_control_10;
 dword	info_for_10_control;

//filler 64
 dword  	a1,b1;
 dword  	a2,b2;
 dword  	a3,b3;
 dword  	a4,b4;

 dword  	a5,b5;
 dword  	a6,b6;
 dword  	a7,b7;
 dword  	a8,b8;

 dword	text_for_2;
 dword	number_controls_2;
 dword	type_control_20;
 dword	info_for_20_control;

//filller 112 bytes
 dword	a9,b9;
 dword	a10,b10;
 dword	a11,b11;
 dword	a12,b12;
 dword	a13,b13;
 dword	a14,b14;
 dword	a15,b15;

 dword  	a16,b16;
 dword  	a17,b17;
 dword  	a18,b18;
 dword  	a19,b19;
 dword  	a20,b20;
 dword  	a21,b21;
 dword  	a22,b22;

 dword	text_for_3;
 dword	number_controls_3;
 dword	type_control_30;
 dword	info_for_30_control;

//filler 256
 dword	a23,b23;
 dword	a24,b24;
 dword	a25,b25;
 dword	a26,b26;
 dword	a27,b27;
 dword	a28,b28;
 dword	a29,b29;
 dword  a30,b30;
 dword  a31,b31;
 dword  a32,b32;
 dword  a33,b33;
 dword  a34,b34;
 dword  a35,b35;
 dword  a36,b36;
 dword  a37,b37;
 dword  a38,b38;

 dword	a39,b39;
 dword	a40,b40;
 dword	a41,b41;
 dword	a42,b42;
 dword	a43,b43;
 dword	a44,b44;
 dword	a45,b45;
 dword  a46,b46;
 dword  a47,b47;
 dword  a48,b48;
 dword  a49,b49;
 dword  a50,b50;
 dword  a51,b51;
 dword  a52,b52;
 dword  a53,b53;
 dword  a54,b54;


 dword	text_for_4;
 dword	number_controls_4;
 dword	type_control_40;
 dword	info_for_40_control;

//filler 64
 dword  a55,b55;
 dword  a56,b56;
 dword  a57,b57;
 dword  a58,b58;

 dword  a59,b59;
 dword  a60,b60;
 dword  a61,b61;
 dword  a62,b62;

 dword	text_for_5;
 dword	number_controls_5;
 dword	type_control_50;
 dword	info_for_50_control;

}__attribute__((packed));
////////////////////////////////////////////////////////////////////////////////

//////////////////////////////Info for Bookmark 2///////////////////////////////
struct InfoForBookmark2
{
 byte		type;
 byte		flag;
 dword	x;
 dword	y;
 dword	sizex;
 dword	sizey;
 dword	color1;
 dword	reserved;
 dword	color2;

 dword	number_lines_bookmarks;
 dword	number_lines_bookmarks_in_1;

 dword	text_for_1;
 dword	number_controls_1;
 dword	type_control_10;
 dword	info_for_10_control;
 dword	type_control_11;
 dword	info_for_11_control;

 dword	text_for_2;
 dword	number_controls_2;
 dword	type_control_20;
 dword	info_for_20_control;

//filler 64
 dword  	a1,b1;
 dword  	a2,b2;
 dword  	a3,b3;
 dword  	a4,b4;

 dword  	a5,b5;
 dword  	a6,b6;
 dword  	a7,b7;
 dword  	a8,b8;

}__attribute__((packed));

struct ControlBookmark2
{
 dword ctrl_proc;
 dword *ctrl_fd;
 dword *ctrl_bk;
 dword *child_fd;
 dword *child_bk;
 dword *parend;
 dword ctrl_x;
 dword ctrl_y;
 dword ctrl_sizex;
 dword ctrl_sizey;
 dword ctrl_ID;

 byte		type;
 byte		flag;
 dword	x;
 dword	y;
 dword	sizex;
 dword	sizey;
 dword	color1;
 dword	reserved;
 dword	color2;

 dword	number_lines_bookmarks;
 dword	number_lines_bookmarks_in_1;

 dword	text_for_1;
 dword	number_controls_1;
 dword	type_control_10;
 dword	info_for_10_control;
 dword	type_control_11;
 dword	info_for_11_control;

 dword	text_for_2;
 dword	number_controls_2;
 dword	type_control_20;
 dword	info_for_20_control;

//filler 64
 dword  	a1,b1;
 dword  	a2,b2;
 dword  	a3,b3;
 dword  	a4,b4;

 dword  	a5,b5;
 dword  	a6,b6;
 dword  	a7,b7;
 dword  	a8,b8;

}__attribute__((packed));
////////////////////////////////////////////////////////////////////////////////

//
//           libGUI functions
//
void   stdcall (*DestroyControl)(void *control);
void   stdcall (*SendMessage)(struct HEADER *Parend,struct MESSAGE *Message);
int    stdcall (*Version)(void);
void   stdcall (*ResizeComponent)(void *Control,int new_sizex,int new_sizey);
void   stdcall (*RemoveComponent)(void *Control,int new_x,int new_y);
void*  stdcall (*CraeteButton)(struct HEADER *Parend,void *Control);
void*  stdcall (*CraeteScroller)(struct HEADER *Parend,void *Control);
void*  stdcall (*CraeteBookmark)(struct HEADER *Parend,void *Control);
void*  stdcall (*CraeteImage)(struct HEADER *Parend,void *Control);
void*  stdcall (*CraeteText)(struct HEADER *Parend,void *Control);
void*  stdcall (*CraeteNumber)(struct HEADER *Parend,void *Control);
void*  stdcall (*CraeteCheckbox)(struct HEADER *Parend,void *Control);
void*  stdcall (*CraeteEditbox)(struct HEADER *Parend,void *Control);
void*  stdcall (*CraeteProgressbar)(struct HEADER *Parend,void *Control);

char* sys_libGUI_path="/sys/lib/libGUI.obj";

void link(char *exp){

	char	name_DestroyControl[]={"DestroyControl"};
	char	name_SendMessage[]={"SendMessage"};
	char	name_Version[]={"Version"};
	char	name_ResizeComponent[]={"ResizeComponent"};
	char	name_RemoveComponent[]={"RemoveComponent"};
	char	name_CraeteButton[]={"CraeteButton"};
	char	name_CraeteScroller[]={"CraeteScroller"};
	char	name_CraeteBookmark[]={"CraeteBookmark"};
	char	name_CraeteImage[]={"CraeteImage"};
	char	name_CraeteText[]={"CraeteText"};
	char	name_CraeteNumber[]={"CraeteNumber"};
	char	name_CraeteCheckbox[]={"CraeteCheckbox"};
	char	name_CraeteEditbox[]={"CraeteEditbox"};
	char	name_CraeteProgressbar[]={"CraeteProgressbar"};

        DestroyControl=(void stdcall (*)(void *control))
		_ksys_cofflib_getproc(exp,name_DestroyControl);
	SendMessage=(void stdcall (*)(struct HEADER *Parend,struct MESSAGE *Message))
		_ksys_cofflib_getproc(exp,name_SendMessage);
	Version=(int stdcall (*)(void))
		_ksys_cofflib_getproc(exp,name_Version);
	ResizeComponent=(void stdcall(*)(void *Control,int new_sizex,int new_sizey))
		_ksys_cofflib_getproc(exp,name_ResizeComponent);
	RemoveComponent=(void stdcall(*)(void *Control,int new_x,int new_y))
		_ksys_cofflib_getproc(exp,name_RemoveComponent);
	CraeteButton=(void* stdcall (*)(struct HEADER *Parend,void *Control))
		_ksys_cofflib_getproc(exp,name_CraeteButton);
	CraeteScroller=(void* stdcall (*)(struct HEADER *Parend,void *Control))
		_ksys_cofflib_getproc(exp,name_CraeteScroller);
	CraeteBookmark=(void* stdcall (*)(struct HEADER *Parend,void *Control))
		_ksys_cofflib_getproc(exp,name_CraeteBookmark);
	CraeteImage=(void* stdcall (*)(struct HEADER *Parend,void *Control))
		_ksys_cofflib_getproc(exp,name_CraeteImage);
	CraeteText=(void* stdcall (*)(struct HEADER *Parend,void *Control))
		_ksys_cofflib_getproc(exp,name_CraeteText);
	CraeteNumber=(void* stdcall (*)(struct HEADER *Parend,void *Control))
		_ksys_cofflib_getproc(exp,name_CraeteNumber);
	CraeteCheckbox=(void* stdcall (*)(struct HEADER *Parend,void *Control))
		_ksys_cofflib_getproc(exp,name_CraeteCheckbox);
	CraeteEditbox=(void* stdcall (*)(struct HEADER *Parend,void *Control))
		_ksys_cofflib_getproc(exp,name_CraeteEditbox);
	CraeteProgressbar=(void* stdcall (*)(struct HEADER *Parend,void *Control))
		_ksys_cofflib_getproc(exp,name_CraeteProgressbar);

}

int	Init_libGUI(void)
{
	char	*Export;

	Export=(char *)_ksys_cofflib_load(sys_libGUI_path);
	if (Export==0) return Cannot_load_libGUI;

	link(Export);
	return(0);
	
}

void draw_window(struct WINDOW *win)
{
  _ksys_window_redraw(1);
  _ksys_draw_window(win->posx,win->posy,win->sizex,win->sizey,0xaabbcc,3,0x5090d0,0,0x5080d0);
  _ksys_window_redraw(2);

}

void CraeteInterface(dword *controls,struct HEADER *Parend,struct WINDOW *win)
{
	char	*names_of_bookmarks1[]={"SINGLET","DOUBLET","SIXTET","P(H)","OTHER"};
	char	*names_of_bookmarks2[]={"SPECTRA","INFORMATION"};

	char	*bookmark_singlet_text[]={"Isomer shift","Amplitude","Width","Xi^2"};
	char	*bookmark_doublet_text[]={"Isomer shift","Quadrupol shift","Amplitude 1",
				"Amplitude 2","Width 1","Width 2","Xi^2"};
	char	*bookmark_sixtet_text[]={"Isomer shift","Quadrupol shift","Feld","Amplitude 1",
				"Amplitude 2","Amplitude 3","Amplitude 4","Amplitude 5",
				"Amplitude 6","Width 1","Width 2","Width 3","Width 4","Width 5",
				"Width 6","Xi^2"};
	char	*bookmark_P_text[]={"K","Maximum feld H","Amplitude","Xi^2"};
	char	*bookmark_information_text[]={"Channels","Null channel","mm/s","Basis(impulses)"};

	char	button_crate_text[]={"Craete"};
	char	button_autocolibrovka_text[]={"Auto colibrovka"};
	char	button_combine_spectras_text[]={"To combine spectras"};

	struct ControlButton		*ButtonCraete;
	struct InfoForButton		*InfoButtonCraete;
	struct ControlButton		*ButtonAutoColibrovka;
	struct InfoForButton		*InfoButtonAutoColibrovka;
	struct ControlButton		*ButtonCombineSpectras;
	struct InfoForButton		*InfoButtonCombineSpectras;

	struct InfoForImage		*InfoImage;
	struct ControlImage		*Image;
	struct InfoForText		*InfoText;
	struct ControlText		*Text;
	struct InfoForNumber		*InfoNumber;
	struct ControlNumber		*Number;
	struct ControlProgressbar	*Progressbar1;
	struct InfoForProgressbar	*InfoProgressbar1;
	
	struct ControlBookmark1	*Bookmark1;
	struct InfoForBookmark1	*InfoBookmark1;
	struct ControlBookmark2	*Bookmark2;
	struct InfoForBookmark2	*InfoBookmark2;

	dword				*ptr,*ptr2;
	int				skin_height;
	int				i;
	int				x,y;
	dword				textcolor;

	InfoButtonCraete=malloc(sizeof(InfoButtonCraete));
	InfoButtonAutoColibrovka=malloc(sizeof(InfoButtonAutoColibrovka));
	InfoButtonCombineSpectras=malloc(sizeof(InfoButtonCombineSpectras));
	InfoImage=malloc(sizeof(InfoImage));
	InfoText=malloc(sizeof(InfoText));
	InfoNumber=malloc(sizeof(InfoNumber));
	InfoProgressbar1=malloc(sizeof(InfoProgressbar1));

	InfoBookmark1=malloc(sizeof(InfoBookmark1));
	InfoBookmark2=malloc(sizeof(InfoBookmark2));

	skin_height=_ksys_get_skin_height();

	
	InfoButtonCraete->type=0x91;//10010001b
	InfoButtonCraete->flag=0;
	InfoButtonCraete->x=10;
	InfoButtonCraete->y=10;
	InfoButtonCraete->width=FONT_SIZEX*sizeof(button_crate_text)+10;
	InfoButtonCraete->height=20;
	InfoButtonCraete->text=(dword)button_crate_text;
	InfoButtonCraete->textcolor=0;
	InfoButtonCraete->color1=0xaabbcc;

	InfoButtonAutoColibrovka->type=0x91;//10010001b
	InfoButtonAutoColibrovka->flag=0;
	InfoButtonAutoColibrovka->x=10;
	InfoButtonAutoColibrovka->y=30;
	InfoButtonAutoColibrovka->width=FONT_SIZEX*sizeof(button_autocolibrovka_text)+10;
	InfoButtonAutoColibrovka->height=20;
	InfoButtonAutoColibrovka->text=(dword)button_autocolibrovka_text;
	InfoButtonAutoColibrovka->textcolor=0;
	InfoButtonAutoColibrovka->color1=0xaabbcc;

	InfoButtonCombineSpectras->type=0x91;//10010001b
	InfoButtonCombineSpectras->flag=0;
	InfoButtonCombineSpectras->x=10;
	InfoButtonCombineSpectras->y=10;
	InfoButtonCombineSpectras->width=FONT_SIZEX*sizeof(button_combine_spectras_text)+10;
	InfoButtonCombineSpectras->height=20;
	InfoButtonCombineSpectras->text=(dword)button_combine_spectras_text;
	InfoButtonCombineSpectras->textcolor=0;
	InfoButtonCombineSpectras->color1=0xaabbcc;
	
	//prepea text for bookmark SINGLET
	textcolor=0xffffff;
	x=10;
	y=40;
	ptr=58+(dword)InfoBookmark1;
	ptr2=58+32+(dword)InfoBookmark1;
	for(i=0;i<4;i++)
	{
		InfoText[i].type=0x80;
		InfoText[i].flag=0;
		InfoText[i].x=x;
		InfoText[i].y=y;
		InfoText[i].color1=textcolor;
		InfoText[i].text=bookmark_singlet_text[i];
		InfoText[i].length=strlen(bookmark_singlet_text[i]);

		InfoNumber[i].type=0x82;
		InfoNumber[i].flag=0;
		InfoNumber[i].x=x+DISTANCE1;
		InfoNumber[i].y=y;
		InfoNumber[i].color1=textcolor;
		InfoNumber[i].number=-0.1234567;
		InfoNumber[i].format=10*65536+7;

		*ptr=(dword)5;
		ptr++;
		*ptr=(dword)&InfoText[i];
		ptr++;

		*ptr2=(dword)6;
		ptr2++;
		*ptr2=(dword)&InfoNumber[i];
		ptr2++;

		y=y+12;
	}
	//prepea text for bookmark DOUBLET
	y=40;
	ptr=58+64+16+(dword)InfoBookmark1;
	ptr2=58+64+16+56+(dword)InfoBookmark1;
	for(i=4;i<4+7;i++)
	{
		InfoText[i].type=0x80;
		InfoText[i].flag=0;
		InfoText[i].x=x;
		InfoText[i].y=y;
		InfoText[i].color1=textcolor;
		InfoText[i].text=bookmark_doublet_text[i-4];
		InfoText[i].length=strlen(bookmark_doublet_text[i-4]);

		InfoNumber[i].type=0x82;
		InfoNumber[i].flag=0;
		InfoNumber[i].x=x+DISTANCE1;
		InfoNumber[i].y=y;
		InfoNumber[i].color1=textcolor;
		InfoNumber[i].number=-0.1234567;
		InfoNumber[i].format=10*65536+7;

		*ptr=(dword)5;
		ptr++;
		*ptr=(dword)&InfoText[i];
		ptr++;

		*ptr2=(dword)6;
		ptr2++;
		*ptr2=(dword)&InfoNumber[i];
		ptr2++;
		y=y+12;
	}
	//prepea text for bookmark SIXTET
	y=40;
	ptr=58+64+16+112+16+(dword)InfoBookmark1;
	ptr2=58+64+16+112+16+128+(dword)InfoBookmark1;
	for(i=4+7;i<4+7+16;i++)
	{
		InfoText[i].type=0x80;
		InfoText[i].flag=0;
		InfoText[i].x=x;
		InfoText[i].y=y;
		InfoText[i].color1=textcolor;
		InfoText[i].text=bookmark_sixtet_text[i-(4+7)];
		InfoText[i].length=strlen(bookmark_sixtet_text[i-(4+7)]);

		InfoNumber[i].type=0x82;
		InfoNumber[i].flag=0;
		InfoNumber[i].x=x+DISTANCE1;
		InfoNumber[i].y=y;
		InfoNumber[i].color1=textcolor;
		InfoNumber[i].number=-0.1234567;
		InfoNumber[i].format=10*65536+7;

		*ptr=(dword)5;
		ptr++;
		*ptr=(dword)&InfoText[i];
		ptr++;

		*ptr2=(dword)6;
		ptr2++;
		*ptr2=(dword)&InfoNumber[i];
		ptr2++;
		y=y+12;
	}
	//prepea text for bookmark P(H)
	y=40;
	ptr=58+64+16+112+16+256+16+(dword)InfoBookmark1;
	ptr2=58+64+16+112+16+256+16+32+(dword)InfoBookmark1;
	for(i=4+7+16;i<4+7+16+4;i++)
	{
		InfoText[i].type=0x80;
		InfoText[i].flag=0;
		InfoText[i].x=x;
		InfoText[i].y=y;
		InfoText[i].color1=textcolor;
		InfoText[i].text=bookmark_P_text[i-(4+7+16)];
		InfoText[i].length=strlen(bookmark_P_text[i-(4+7+16)]);

		InfoNumber[i].type=0x82;
		InfoNumber[i].flag=0;
		InfoNumber[i].x=x+DISTANCE1;
		InfoNumber[i].y=y;
		InfoNumber[i].color1=textcolor;
		InfoNumber[i].number=-0.1234567;
		InfoNumber[i].format=10*65536+7;

		*ptr=(dword)5;
		ptr++;
		*ptr=(dword)&InfoText[i];
		ptr++;

		*ptr2=(dword)6;
		ptr2++;
		*ptr2=(dword)&InfoNumber[i];
		ptr2++;
		y=y+12;
	}

	//prepea text for bookmark INFORMATION
	textcolor=0xffffff;
	x=10;
	y=40;
	ptr=78+(dword)InfoBookmark2;
	ptr2=78+32+(dword)InfoBookmark2;
	for(i=4+7+16+4;i<4+7+16+4+4;i++)
	{
		InfoText[i].type=0x80;
		InfoText[i].flag=0;
		InfoText[i].x=x;
		InfoText[i].y=y;
		InfoText[i].color1=textcolor;
		InfoText[i].text=bookmark_information_text[i-(4+7+16+4)];
		InfoText[i].length=strlen(bookmark_information_text[i-(4+7+16+4)]);

		InfoNumber[i].type=0x82;
		InfoNumber[i].flag=0;
		InfoNumber[i].x=x+DISTANCE1;
		InfoNumber[i].y=y;
		InfoNumber[i].color1=textcolor;
		InfoNumber[i].number=-0.1234567;
		InfoNumber[i].format=10*65536+7;

		*ptr=(dword)5;
		ptr++;
		*ptr=(dword)&InfoText[i];
		ptr++;

		*ptr2=(dword)6;
		ptr2++;
		*ptr2=(dword)&InfoNumber[i];
		ptr2++;

		y=y+12;
	}

	//-------------------Init bookmark 1--------------------------------
	InfoBookmark1->type=0x81; //10000001b
	InfoBookmark1->x=BOOKMARK1_POSX;
	InfoBookmark1->y=BOOKMARK1_POSY;
	InfoBookmark1->sizex=BOOKMARK1_SIZEX;
	InfoBookmark1->sizey=win->sizey-InfoBookmark1->y-skin_height-5;
	InfoBookmark1->color1=0xaabbcc;
	InfoBookmark1->color2=0xffffff;
	InfoBookmark1->number_lines_bookmarks=2;
	InfoBookmark1->number_lines_bookmarks_in_1=3;
	InfoBookmark1->number_lines_bookmarks_in_2=2;

	//bookmark singlet
	InfoBookmark1->text_for_1=(dword)names_of_bookmarks1[0];
	InfoBookmark1->number_controls_1=1+4+4;
	InfoBookmark1->type_control_10=1;
	InfoBookmark1->info_for_10_control=(dword)InfoButtonCraete;

	//bookmark doublet
	InfoBookmark1->text_for_2=(dword)names_of_bookmarks1[1];
	InfoBookmark1->number_controls_2=1+7+7;
	InfoBookmark1->type_control_20=1;
	InfoBookmark1->info_for_20_control=(dword)InfoButtonCraete;
	
	//bookmark sixtet
	InfoBookmark1->text_for_3=(dword)names_of_bookmarks1[2];
	InfoBookmark1->number_controls_3=1+16+16;
	InfoBookmark1->type_control_30=1;
	InfoBookmark1->info_for_30_control=(dword)InfoButtonCraete;

	//bookmark P(H)
	InfoBookmark1->text_for_4=(dword)names_of_bookmarks1[3];
	InfoBookmark1->number_controls_4=1+4+4;
	InfoBookmark1->type_control_40=1;
	InfoBookmark1->info_for_40_control=(dword)InfoButtonCraete;

	//bookmark OTHER
	InfoBookmark1->text_for_5=(dword)names_of_bookmarks1[4];
	InfoBookmark1->number_controls_5=1;
	InfoBookmark1->type_control_50=1;
	InfoBookmark1->info_for_50_control=(dword)InfoButtonCombineSpectras;

	//-----------------------Init bookmark 2--------------------------------
	InfoBookmark2->type=0x81; //10000001b
	InfoBookmark2->x=BOOKMARK2_POSX;
	InfoBookmark2->y=BOOKMARK2_POSY;
	InfoBookmark2->sizex=win->sizex-InfoBookmark2->x-10;
	InfoBookmark2->sizey=win->sizey-InfoBookmark2->y-skin_height-5;
	InfoBookmark2->color1=0xaabbcc;
	InfoBookmark2->color2=0xffffff;
	InfoBookmark2->number_lines_bookmarks=1;
	InfoBookmark2->number_lines_bookmarks_in_1=2;

	//bookmark SPECTRA
	InfoBookmark2->text_for_1=(dword)names_of_bookmarks2[0];
	InfoBookmark2->number_controls_1=2;
	InfoBookmark2->type_control_10=4;
	InfoBookmark2->info_for_10_control=(dword)InfoImage;
	InfoBookmark2->type_control_11=9;
	InfoBookmark2->info_for_11_control=(dword)InfoProgressbar1;

	//bookmark INFORMATION
	InfoBookmark2->text_for_2=(dword)names_of_bookmarks2[1];
	InfoBookmark2->number_controls_2=1+4+4;
	InfoBookmark2->type_control_20=1;
	InfoBookmark2->info_for_20_control=(dword)InfoButtonCraete;
	
	InfoImage->type=0x80;//10000000b
	InfoImage->flag=0;
	InfoImage->x=10;
	InfoImage->y=10;
	InfoImage->sizex=InfoBookmark2->sizex-20;
	InfoImage->sizey=InfoBookmark2->sizey-44-16;
	InfoImage->pointer=(dword)malloc(SCREEN_SIZEX*SCREEN_SIZEY*3);

	InfoProgressbar1->type=0x81;//10000001b;
  	InfoProgressbar1->flag=0;
	InfoProgressbar1->color1=0xffffff;
	InfoProgressbar1->color2=0xff0000;
	InfoProgressbar1->x=10;
	InfoProgressbar1->y=InfoBookmark2->sizey-16-26;
	InfoProgressbar1->sizex=InfoBookmark2->sizex-20;
	InfoProgressbar1->sizey=16;
	InfoProgressbar1->progress=0.1;
	InfoProgressbar1->color3=0xaabbcc;

	Bookmark1=CraeteBookmark(Parend,InfoBookmark1);
	Bookmark2=CraeteBookmark(Parend,InfoBookmark2);
	
	ButtonAutoColibrovka=CraeteButton(Parend,InfoButtonAutoColibrovka);

	controls[0]=(dword)Bookmark1;
	controls[1]=(dword)Bookmark2;
	controls[2]=(dword)ButtonAutoColibrovka;
      controls[3]=(dword)InfoImage->pointer;

	//copy child controls of bookmark1 to the array
	//It's proved simple way for acces to the child controls of bookmark1.
	ptr2=44+58+32+4+(dword)Bookmark1;
	for(i=CONTROLS_NUMBER_POS;i<CONTROLS_NUMBER_POS+4;i++)
	{
		controls[i]=*ptr2;
		ptr2=ptr2+2;
	}
	ptr2=44+58+64+16+56+4+(dword)Bookmark1;
	for(i=CONTROLS_NUMBER_POS+4;i<CONTROLS_NUMBER_POS+4+7;i++)
	{
		controls[i]=*ptr2;
		ptr2=ptr2+2;
	}
	ptr2=44+58+64+16+112+16+128+4+(dword)Bookmark1;
	for(i=CONTROLS_NUMBER_POS+4+7;i<CONTROLS_NUMBER_POS+4+7+16;i++)
	{
		controls[i]=*ptr2;
		ptr2=ptr2+2;
	}
	ptr2=44+58+64+16+112+16+256+16+32+4+(dword)Bookmark1;
	for(i=CONTROLS_NUMBER_POS+4+7+16;i<CONTROLS_NUMBER_POS+4+7+16+4;i++)
	{
		controls[i]=*ptr2;
		ptr2=ptr2+2;
	}
	ptr2=44+78+32+4+(dword)Bookmark2;
	for(i=CONTROLS_NUMBER_POS+4+7+16+4;i<CONTROLS_NUMBER_POS+4+7+16+4+4;i++)
	{
		controls[i]=*ptr2;
		ptr2=ptr2+2;
	}

	free(InfoButtonCraete);
	free(InfoButtonAutoColibrovka);
	free(InfoButtonCombineSpectras);
	free(InfoBookmark1);
	free(InfoBookmark2);
	free(InfoImage);
	free(InfoText);
	free(InfoProgressbar1);

}

int main(int argc, char **argv)
{
  char				exit_status;
  struct	WINDOW		*win;
  struct        HEADER		*Parend;
  struct        MESSAGE	*Message;
  struct ControlProgressbar	*Progressbar1;
  struct InfoForProgressbar	*InfoProgressbar1;
  struct ControlBookmark1	*Bookmark1;
  struct ControlBookmark2	*Bookmark2;
  struct ControlImage		*Image;
  struct process_table_entry	*ProcessInformation;

  dword				*controls;
  int				new_sizex1;
  int				new_sizey1;
  int				new_sizex2;
  int				new_sizey2;
  int				new_sizex3;
  int				new_sizey3;
  int				new_sizex4;
  int				new_sizey4;
  int				new_x;
  int				new_y;

  int				skin_height;
  dword  x,y,mouse_buttons;
  float	 p;


  _ksys_set_wanted_events(0x67);

  Init_libGUI();

  win=malloc(16);
  win->posx=WINDOW_POSX;
  win->posy=WINDOW_POSY;
  win->sizex=WINDOW_SIZEX;
  win->sizey=WINDOW_SIZEY;

  draw_window(win);

  controls=malloc(TOTAL_NUMBERS_OF_CONTROLS*100);
  Parend=(struct HEADER *)malloc(ParendSize);
  Message=(struct MESSAGE *)malloc(MessageSize);
  ProcessInformation=malloc(sizeof(ProcessInformation));

  CraeteInterface(controls,Parend,win);

  Message->type=1;
  Message->arg1=0;
  Message->arg2=0;
  Message->arg3=0;

  SendMessage(Parend,Message);

  exit_status=0;
  while(exit_status!=1)
   {
     switch(_ksys_wait_for_event(2))
	{
        case 1: 
		{
			_ksys_get_process_table(ProcessInformation,-1);
			win->sizex=ProcessInformation->winx_size;
			win->sizey=ProcessInformation->winy_size;

			draw_window(win);
			skin_height=_ksys_get_skin_height();
			Bookmark1=controls[0];
			Bookmark2=controls[1];
			new_sizex1=Bookmark1->sizex;
			new_sizey1=win->sizey-Bookmark1->y-skin_height-5;
			new_sizex2=win->sizex-Bookmark2->x-10;
			new_sizey2=win->sizey-Bookmark2->y-skin_height-5;
			Image=Bookmark2->info_for_10_control;
			new_sizex3=new_sizex2-20;
			new_sizey3=new_sizey2-44-16;
			Progressbar1=Bookmark2->info_for_11_control;
			new_sizex4=new_sizex2-20;
			new_sizey4=16;
			new_x=Bookmark2->x+10;
			new_y=Bookmark2->y+new_sizey2;
			RemoveComponent(Progressbar1,new_x,new_y);
			ResizeComponent(Bookmark1,new_sizex1,new_sizey1);
			ResizeComponent(Image,new_sizex3,new_sizey3);
			ResizeComponent(Progressbar1,new_sizex4,new_sizey4);
			ResizeComponent(Bookmark2,new_sizex2,new_sizey2);
			Message->type=1;
			SendMessage(Parend,Message);
			break;
		}

        case 2: 
		{
			exit_status=1;
			break;
		}

	case 3: {
			if (_ksys_get_button_id()==1) {exit_status=1;}
			break;
                }

        case 6: 
		{
			mouse_buttons=_ksys_GetMouseButtonsState();
			x=_ksys_GetMouseXY();
			y=x & 0xffff;
			x=x >> 16;
			Message->type=6;
			Message->arg1=x;
			Message->arg2=y;
			Message->arg3=mouse_buttons;
			SendMessage(Parend,Message);

			break;
                }

	default:	break;

	}
    }
}
