//CODED by Veliant, Leency, Nable. GNU GPL licence.
#ifndef INCLUDE_KOLIBRI_H
#define INCLUDE_KOLIBRI_H

#startaddress 0
#code32 TRUE

char   os_name[8]   = {'M','E','N','U','E','T','0','1'};
dword  os_version   = 0x00000001;
dword  start_addr   = #load_init_main;
dword  final_addr   = #stop+32;
dword  alloc_mem    = MEMSIZE;
dword  x86esp_reg   = MEMSIZE;
dword  I_Param      = #param;
dword  I_Path       = #program_path;
char param[4096];
char program_path[4096];

#define NULL      0
#define OLD      -1
#define true      1
#define false     0

//Events
#define evReDraw  1
#define evKey     2
#define evButton  3
#define evDesktop 5
#define evMouse   6
#define evIPC     7
#define evNetwork 8
#define evDebug   9

//Button options
#define BT_DEL      0x80000000
#define BT_HIDE     0x40000000
#define BT_NOFRAME  0x20000000

//Button mouse
#define MOUSE_LEFT   001b
#define MOUSE_RIGHT  010b
#define MOUSE_LR     011b
#define MOUSE_CENTER 100b

//ASCII KEYS
#define ASCII_KEY_BS    008
#define ASCII_KEY_TAB   009
#define ASCII_KEY_ENTER 013
#define ASCII_KEY_ESC   027
#define ASCII_KEY_DEL   182
#define ASCII_KEY_INS   185
#define ASCII_KEY_SPACE 032

#define ASCII_KEY_LEFT  176
#define ASCII_KEY_RIGHT 179
#define ASCII_KEY_DOWN  177
#define ASCII_KEY_UP    178
#define ASCII_KEY_HOME  180
#define ASCII_KEY_END   181
#define ASCII_KEY_PGDN  183
#define ASCII_KEY_PGUP  184

//allow event mask
#define EVENT_MASK_REDRAW   000000001b
#define EVENT_MASK_KEYBOARD 000000010b
#define EVENT_MASK_BUTTONS  000000100b
#define EVENT_MASK_DESKTOP  000010000b
#define EVENT_MASK_MOUSE    000100000b
#define EVENT_MASK_IPC      001000000b
#define EVENT_MASK_NETWORK  010000000b
#define EVENT_MASK_DEBUG    100000000b

//ARGS FUNCTION
#define END_ARGS 0xFF00FF
//-------------------------------------------------------------------------

:struct raw_image {
	dword w, h, data;
};

/**
 *  The structure of the mouse
 *  x - coordinate X
 *  y - coordinate Y
 *  xx and yy - time coordinates
 *  lkm - left mouse button
 *  pkm - right mouse button
 *  mkm - mouse wheel
 *  key - keycode button
 *  tmp - time keycode 
 *  down - key event press
 *  up - key release events
 *  move - event mouse movements
 *  click - when clicked
 *  dblclick - double-click the default 50 (500 ms)
 */

:dword __TMP_TIME,MOUSE_TIME;
:struct mouse
{
	signed x,y,xx,yy,lkm,mkm,pkm,key,tmp,tmp_time,hor,vert,down,up,move,click,dblclick,left,top;
	dword handle,_;
	byte cmd;
	void clearTime();
	void get();
	void set();
	void center();
	dword hide();
	void slider();
	void show();
};
:void mouse::clearTime()
{
	tmp_time = GetStartTime()+MOUSE_TIME;
}
:void mouse::show()
{
	if(!handle)return;
	ECX = handle;
	EAX = 37;
	EBX = 5;
	$int 0x40;
}
:dword mouse::hide()
{
	if(!_)
	{
		EAX = 68;
		EBX = 12;
		ECX = 32*32*4;
		$int 0x40
		ECX = EAX;
		_ = EAX;
	} else ECX = _;
	EAX = 37;
	EBX = 4;
	DX  = 2;
	$int 0x40;
	handle = EAX;
	ECX = EAX;
	EAX = 37;
	EBX = 5;
	$int 0x40;
	handle = EAX;
}

//set new attributes mouse
:void mouse::set()
{
	if((xx!=x)||(yy!=y))
	{
		EAX = 18;
		EBX = 19;
		ECX = 4;
		EDX = (x<<16)+y;
		$int 0x40
		//move = true;
	}
	if((key)||(lkm|mkm|pkm))&&(down|up|click|dblclick|move)
	{
		if(lkm|mkm|pkm)key=(lkm)|(pkm<<1)|(2<<mkm);
		EAX = 18;
		EBX = 19;
		ECX = key;
		EDX = (x<<16)+y;
		$int 0x40
	}
}

:void mouse::center()
{
	EAX = 18;
	EBX = 15;
	$int 0x40
}

//get new attributes mouse
:void mouse::get()
{
	EAX = 37;
	EBX = 1;
	$int	0x40
	$mov	ebx, eax
	$shr	eax, 16
	$and	ebx,0x0000FFFF
	x = EAX;
	y = EBX;
	if (x>6000) x-=65535;
	if (y>6000) y-=65535;
	EAX = 37;
	EBX = 2;
	$int	0x40
	$mov	ebx, eax
	$mov	ecx, eax
	key = EAX;
	$and	eax, 0x00000001
	$shr	ebx, 1
	$and	ebx, 0x00000001
	$shr	ecx, 2
	$and	ecx, 0x00000001
	lkm = EAX;
	pkm = EBX;
	mkm = ECX;
	
	//when you release the mouse button
	// Mouse Up Event
	if((cmd)&&!(key)){
		up = true;
		down = false;
		if(!move) click = true;
		move = false;
		__TMP_TIME = GetStartTime();
		if(__TMP_TIME-tmp_time<=MOUSE_TIME){ dblclick = true;click = false; }
		tmp_time = __TMP_TIME;
		//returns the key code
		key = tmp;
		lkm = 1&tmp;
		pkm = 2&tmp;
		pkm >>= 1;
		mkm = 4&tmp;
		mkm >>= 2;
		cmd = false;
	}
	
	//when you press the mouse button
	// Mouse Down Event/Move Event
	else {
	    up       = false;
		click    = false;
		dblclick = false;
		down     = false;
		// Mouse Move Event
		if((xx!=x)||(yy!=y))
		{
			move = true;
			xx = x;
			yy = y;
		}
		else move = false;
		if(key)if(!cmd) {down = true;cmd = true;tmp=key;}
	}
	
	//scroll
	EAX = 37;
	EBX = 7;
	$int	0x40
	$mov	ebx, eax
	$shr	eax, 16
	$and	ebx,0x0000FFFF
	//hor = EAX;
	vert = EBX;
}



:void mouse::slider()
{
	signed _x,_y;
	if(!handle)hide();
	get();
	_x = x;_y = y;
	pause(5);
	get();
	left = _x - x;
	top  = _y - y;
	center();
	get();
	_x = x;_y = y;
	pause(5);
}

:struct keyboard
{
	signed key;
	byte down,up,press;
	void get(void);
};

:void keyboard::get(void)
{
	
}

:struct system_colors
{
	dword frame,grab,grab_button,grab_button_text,grab_text,
	      work,work_button,work_button_text,work_text,work_graph;
	void get();
};

:void system_colors::get()
{
	EAX = 48;
	EBX = 3;
	ECX = #frame;
	EDX = 40;
	$int 0x40
}

//------------------------------------------------------------------------------
:dword wait_event_code;
inline fastcall dword WaitEvent()
{
	$mov eax,10
	$int 0x40
	wait_event_code = EAX;
	//if(wait_event_code==evMouse) MOUSE.get();
	//return wait_event_code;
}

inline fastcall dword CheckEvent()
{
	$mov eax,11
	$int 0x40
}

inline fastcall dword WaitEventTimeout(EBX)
{
	$mov eax,23
	$int 0x40
} 
 
inline fastcall SetEventMask(EBX)
{
	$mov eax,40
	$int 0x40
}

inline fastcall ScancodesGeting(){
	$mov eax,66
	$mov ebx,1
	$mov ecx,1 //б™†≠™Ѓ§л
	$int 0x40
}

inline fastcall word GetKey()  //+Gluk fix
{
		$push edx
GETKEY:
		$mov  eax,2
		$int  0x40
		$cmp eax,1
		$jne GETKEYI
		$mov ah,dh
		$jmp GETKEYII //jz?
GETKEYI:
		$mov dh,ah
		$jmp GETKEY
GETKEYII:
		$pop edx
		$shr eax,8
}

inline fastcall int GetFullKey()
{
	$mov  eax,2
	$int  0x40
}


inline fastcall pause(EBX)
{
	$mov eax, 5
	$int 0x40
}

inline fastcall word GetButtonID()
{
	$mov eax,17
	$int  0x40
	$shr eax,8
}

inline fastcall dword GetFreeRAM()
{
	$mov eax, 18
	$mov ebx, 16
	$int 0x40
	//return eax = размер свободной пам€ти в килобайтах
}

inline void draw_line(dword x1,y1,x2,y2,color)
{
	x2--;y2--;y1--;
	$mov EAX,38
	EBX = x1<<16;
	EBX |= x2;
	ECX = y1<<16;
	ECX |= y2;
	$mov EDX,color
	$int 0x40
}

inline fastcall dword LoadDriver(ECX) //ECX - им€ драйвера
{
	$mov eax, 68
	$mov ebx, 16
	$int 0x40
	//return 0 - неудача, иначе eax = хэндл драйвера 
}

inline fastcall dword RuleDriver(ECX) //указатель на управл€ющую структуру
{
	$mov eax, 68
	$mov ebx, 17
	$int 0x40
	//return eax = определ€етс€ драйвером
}

struct proc_info
{
	#define SelfInfo -1
	dword	use_cpu;
	word	pos_in_stack,num_slot,rezerv1;
	unsigned char name[11];
	char	rezerv2;
	dword	adress,use_memory,ID,left,top,width,height;
	word	status_slot,rezerv3;
	dword	work_left,work_top,work_width,work_height;
	char	status_window;
	dword   cwidth,cheight;
	byte    reserved[1024-71-8];
};

inline fastcall void GetProcessInfo(EBX, ECX)
{
	$mov eax,9;
	$int  0x40
	DSDWORD[EBX+71] = DSDWORD[EBX+42] - 9; //set cwidth
	DSDWORD[EBX+75] = DSDWORD[EBX+46] - GetSkinHeight() - 4; //set cheight
}

inline fastcall int GetPointOwner( EBX, ECX) //ebx=m.x, ecx=m.y
{
	$mov eax,34
	$int 0x40
}

inline fastcall int GetProcessSlot( ECX)
{
	EAX = 18;
	EBX = 21;
	$int 0x40
}

inline fastcall int GetActiveProcess()
{
	EAX = 18;
	EBX = 7;
	$int 0x40
}

:int CheckActiveProcess(int Form_ID)
{
	int id9=GetProcessSlot(Form_ID);
	if (id9==GetActiveProcess()) return 1;
	return 0;
}

inline fastcall void ActivateWindow( ECX)
{
	EAX = 18;
	EBX = 3;
	$int 0x40
}

inline fastcall int MinimizeWindow()
{
	EAX = 18;
	EBX = 10;
	$int 0x40
}

inline fastcall int CreateThread(ECX,EDX)
{
	$mov eax,51
	$mov ebx,1
	$int 0x40
}

inline fastcall void SwitchToAnotherThread()
{
	$mov eax,68
	$mov ebx,1
	$int 0x40
}

inline fastcall int SendWindowMessage( ECX, EDX)
{
	$mov eax, 72
	$mov ebx, 1
	$int 0x40
}

inline fastcall int KillProcess( ECX)
{
	$mov eax,18;
	$mov ebx,18;
	$int 0x40
}

#define TURN_OFF 2
#define REBOOT 3
#define KERNEL 4
inline fastcall int ExitSystem( ECX)
{
	$mov eax, 18
	$mov ebx, 9
	$int 0x40
}

inline fastcall ExitProcess()
{
	$mov eax,-1;
	$int 0x40
}

//------------------------------------------------------------------------------

//eax = €зык системы (1=eng, 2=fi, 3=ger, 4=rus)
inline fastcall int GetSystemLanguage()
{
	EAX = 26;
	EBX = 5;
	$int 0x40
}

inline fastcall GetSkinHeight()
{
	$push ebx
	$mov  eax,48
	$mov  ebx,4
	$int 0x40
	$pop  ebx
}

inline fastcall void SetSystemSkin( ECX)
{
	EAX = 48;
	EBX = 8;
	$int 0x40
}

inline fastcall int GetScreenWidth()
{
	$mov eax, 14
	$int 0x40
	$shr eax, 16
}

inline fastcall int GetScreenHeight()
{
	$mov eax, 14
	$int 0x40
	$and eax,0x0000FFFF
}

inline fastcall int GetClientTop()
{
	$mov eax, 48
	$mov ebx, 5
	$int 0x40
    $mov eax, ebx
    $shr eax, 16
}

inline fastcall int GetClientHeight()
{
	$mov eax, 48
	$mov ebx, 5
	$int 0x40
    $mov eax, ebx
}


inline fastcall dword LoadLibrary( ECX)
{
	$mov eax, 68 
	$mov ebx, 19
	$int  0x40
}

inline fastcall int TestBit( EAX, CL)
{
	$shr eax,cl
	$and eax,1
}

inline fastcall int PlaySpeaker( ESI)
{
	$mov eax, 55
	$mov ebx, 55
	$int 0x40
}

inline fastcall void debugln( EDX)
{
	$push eax
	$push ebx
	$push ecx
	$mov eax, 63
	$mov ebx, 1
NEXT_CHAR:
	$mov ecx, DSDWORD[edx]
	$or	 cl, cl
	$jz  DONE
	$int 0x40
	$inc edx
	$jmp NEXT_CHAR
DONE:
	$mov cl, 13
	$int 0x40
	$mov cl, 10
	$int 0x40
	$pop ecx
	$pop ebx
	$pop eax
}

inline fastcall void debug( EDX)
{
	$push eax
	$push ebx
	$push ecx
	$mov eax, 63
	$mov ebx, 1
NEXT_CHAR:
	$mov ecx, DSDWORD[edx]
	$or	 cl, cl
	$jz  DONE
	$int 0x40
	$inc edx
	$jmp NEXT_CHAR
DONE:
	$pop ecx
	$pop ebx
	$pop eax
}


inline fastcall void debugch( ECX)
{
	$push eax
	$push ebx
	$mov eax,63
	$mov ebx,1
	$int 0x40
	$pop ebx
	$pop eax
}
//------------------------------------------------------------------------------

void DefineAndDrawWindow(dword x, y, size_w, size_h, byte WindowType,dword WindowAreaColor, EDI, ESI)
{
	EAX = 12;              // function 12:tell os about windowdraw
	EBX = 1;
	$int 0x40
	
	$xor EAX,EAX
	EBX = x << 16 + size_w; 
	ECX = y << 16 + size_h;

	EDX = WindowType << 24 | WindowAreaColor;
	$int 0x40

	EAX = 12;              // function 12:tell os about windowdraw
	EBX = 2;
	$int 0x40
}

inline fastcall MoveSize( EBX,ECX,EDX,ESI)
{
	$mov eax, 67
	$int 0x40
}

inline fastcall void DrawTitle( ECX)
{
	EAX = 71;
	EBX = 1;
	$int 0x40;
}

void WriteTextB(dword x,y,byte fontType, dword color, EDX)
{
	EAX = 4;
	EBX = x<<16+y;
	ECX = fontType<<24+color;
	ESI = 0;
	$int 0x40;
	$add ebx, 1<<16
	$int 0x40
}

void WriteText(dword x,y,byte fontType, dword color, EDX)
{
	EAX = 4;
	EBX = x<<16+y;
	ECX = fontType<<24+color;
	$int 0x40;
}

dword WriteBufText(dword x,y,byte fontType, dword color, EDX, EDI)
{
	EAX = 4;
	EBX = x<<16+y;
	ECX = fontType<<24+color;
	$int 0x40;
}

void WriteNumber(dword x,y,byte fontType, dword color, count, ECX)
{
	EAX = 47;
	EBX = count<<16;
	EDX = x<<16+y;
	ESI = fontType<<24+color;
	$int 0x40;
}

void CopyScreen(dword EBX, x, y, w, h)
{
  EAX = 36;
  ECX = w << 16 + h;
  EDX = x << 16 + y;
  $int  0x40;
}

:dword GetPixelColor(dword x, x_size, y)
{
	$mov eax, 35
	EBX= y*x_size+x;
	$int 0x40
}


void _PutImage(dword x,y, w,h, EBX)
{
	EAX = 7;
	ECX = w<<16+h;
	EDX = x<<16+y;
	$int 0x40
}

void PutPaletteImage(dword EBX,w,h,x,y,ESI,EDI)
{
	EAX = 65;
	ECX = w<<16+h;
	EDX = x<<16+y;
	EBP = 0;
	$int 0x40
} 

inline fastcall void PutPixel( EBX,ECX,EDX)
{
  EAX=1;
  $int 0x40
}

void DrawBar(dword x,y,w,h,EDX)
{
	if (h<=0) || (h>60000) || (w<=0) || (w>60000) return; //bad boy :)
	EAX = 13;
	EBX = x<<16+w;
	ECX = y<<16+h;
 	$int 0x40
}

void DefineButton(dword x,y,w,h,EDX,ESI)
{
	EAX = 8;
	$push edx
	EDX += BT_DEL;
	$int 0x40;
	$pop edx
	EBX = x<<16+w;
	ECX = y<<16+h;
	$int 0x40
}

inline RefreshWindow(dword ID_REFRESH,ID_ACTIVE)
{
	EAX = 18;
	EBX = 22;
	ECX = 3;
	EDX = ID_REFRESH;
	$int 0x40
	EAX = 18;
	EBX = 3;
	EDX = ID_ACTIVE;
	$int 0x40
}

inline getIPC(ECX,EDX)
{
	$mov EAX,60
	$mov EBX,2
	$int 0x40
}

inline sendIPC(ECX,EDX,ESI)
{
	$mov EAX,60
	$mov EBX,1
	$int 0x40
}

void UnsafeDefineButton(dword x,y,w,h,EDX,ESI)
{
	EAX = 8;
	EBX = x<<16+w;
	ECX = y<<16+h;
	$int 0x40
}

inline fastcall void DeleteButton( EDX)
{
	EAX = 8;
	EDX += BT_DEL;
	$int 0x40
}

inline fastcall dword GetStartTime()
{
	$mov eax,26
	$mov ebx,9
	$int 0x40
}


:dword X_EventRedrawWindow,Y_EventRedrawWindow;
:void _EventRedrawWindow()
{
	loop()switch(WaitEvent())
	{
		case evReDraw:
			DefineAndDrawWindow(X_EventRedrawWindow,Y_EventRedrawWindow,100,1,1,0x34,0xFFFFFF,"");
			pause(10);
			ExitProcess();
			break;
	}
}
:void EventRedrawWindow(dword x,y)
{
	dword mem = malloc(4096);
	X_EventRedrawWindow = x;
	Y_EventRedrawWindow = y;
	CreateThread(#_EventRedrawWindow,mem+4092);
}

:dword ALERT_TEXT;
:void dialog_alert()
{
	byte id;
	loop()switch(WaitEvent())
	{
		case evReDraw:
			DefineAndDrawWindow(215,100,250,200,0x34,0xFFFFFF,"Alert");
			WriteTextB(5,5,0x90,0x0,ALERT_TEXT);
		break;
		case evKey:
		case evButton:
			id=GetButtonID();
			if (id==1) ExitProcess();
		break;
	}
}
:dword alert(dword text)
{
	dword mem = malloc(4096);
	ALERT_TEXT = text;
	CreateThread(#dialog_alert,mem+4092);
	return mem;
}

:struct _screen
{
	dword width,height;
} screen;

:struct _skin
{
	dword width,height;
} SKIN;

dword __generator;  // random number generator - дл€ генерации случайных чисел

:dword program_path_length;

//The initialization of the initial data before running
void load_init_main()
{
	SKIN.height   = GetSkinHeight();
	
	screen.width  = GetScreenWidth();
	screen.height = GetScreenHeight();
	
	//program_path_length = strlen(I_Path);
	MOUSE_TIME = 50; //Default 500 ms.
	__generator = GetStartTime();
	//mem_Init();
	main();
}

#endif