/*
	structures for work with libGUI
*/

/////////////////////////////////////////////////////////////////

#define	CANNOT_LOAD_LIBGUI			100
#define PAREND_SIZE				44
#define MESSAGE_SIZE				16

#define MESSAGE_FULL_REDRAW_ALL			1
#define MESSAGE_KEYS_EVENTS			2
#define MESSAGE_SPECIALIZED			3
#define MESSAGE_MOUSE_EVENTS			6

#define BUTTON_TYPE_2D_WITH_TEXT		(1+128)
#define BUTTON_TYPE_2D_WITH_PICTURE		(2+128)
#define BUTTON_TYPE_2D_WITH_TEXT_PICTURE	(4+128)
#define BUTTON_TYPE_NO_DRAW			(8+128)
#define BUTTON_TYPE_3D_WITH_TEXT		(1+16+128)
#define BUTTON_TYPE_3D_WITH_PICTURE		(2+16+128)
#define BUTTON_TYPE_3D_WITH_TEXT_PICTURE	(4+16+128)

#define TEXT_TYPE_NO_DRAW			(1)
#define TEXT_TYPE_WITH_BACKGROUND		(128+2)
#define TEXT_TYPE_SYSTEM_6X9			(128)

#define NUMBER_NO_DRAW				(1)
#define NUMBER_INTEGER				(128)
#define NUMBER_FLOAT				(128+2)

#define IMAGE_TYPE_1				(128)

#define PROGRESS_BAR_TYPE_1			(128)

#define BOOKMARK_TYPE_1				(128)

///////////////////////////////////////////////////////////////////
#define BUTTON_STATE_CROSS			(1)
#define BUTTON_STATE_CROSS_PRESS		(3)


////////////////////////////////////////////////////////////////
//	header of control
////////////////////////////////////////////////////////////////

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

}__attribute__((packed));

////////////////////////////////////////////////////////////////
//	message
////////////////////////////////////////////////////////////////

struct MESSAGE
{
 dword type;
 dword arg1;
 dword arg2;
 dword arg3;
}__attribute__((packed));

////////////////////////////////////////////////////////////////
//	button
////////////////////////////////////////////////////////////////

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
}__attribute__((packed));

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

////////////////////////////////////////////////////////////////
//	scroller
////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////
//	progressbar
////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////
//	image
////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////
//	text
////////////////////////////////////////////////////////////////

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
 dword background_color;
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
 dword background_color;
}__attribute__((packed));

////////////////////////////////////////////////////////////////
//	number
////////////////////////////////////////////////////////////////

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


/////////////////////////////////////////////////////////////////
//           libGUI functions
/////////////////////////////////////////////////////////////////

void   stdcall (*DestroyControl)(void *control);
void   stdcall (*SendMessage)(struct HEADER *Parend,struct MESSAGE *Message);
int    stdcall (*Version)(void);
void   stdcall (*ResizeComponent)(void *Control,int new_sizex,int new_sizey);
void   stdcall (*MoveComponent)(void *Control,int new_x,int new_y);
void   stdcall (*ActivateTrapForSpecializedMessage)(void *Control);
void*  stdcall (*CraeteButton)(struct HEADER *Parend,void *Control);
void*  stdcall (*CraeteScroller)(struct HEADER *Parend,void *Control);
void*  stdcall (*CraeteBookmark)(struct HEADER *Parend,void *Control);
void*  stdcall (*CraeteImage)(struct HEADER *Parend,void *Control);
void*  stdcall (*CraeteText)(struct HEADER *Parend,void *Control);
void*  stdcall (*CraeteNumber)(struct HEADER *Parend,void *Control);
void*  stdcall (*CraeteCheckbox)(struct HEADER *Parend,void *Control);
void*  stdcall (*CraeteEditbox)(struct HEADER *Parend,void *Control);
void*  stdcall (*CraeteProgressbar)(struct HEADER *Parend,void *Control);

