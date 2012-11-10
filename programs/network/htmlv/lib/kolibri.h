//CODED by Veliant, Leency, Nable. GNU GPL licence.

#startaddress 0
#code32 TRUE

char   os_name[8]   = {'M','E','N','U','E','T','0','1'};
dword  os_version   = 0x00000001;
dword  start_addr   = #main;
dword  final_addr   = #stop+32;
dword  alloc_mem    = #0x00100000;
dword  x86esp_reg   = #0x00100000;
dword  I_Param      = #param;
dword  I_Path       = #program_path;
char param[4096];
char program_path[4096];

//Events
#define evMouse   6
#define evButton  3
#define evKey     2
#define evReDraw  1

#define OLD      -1
#define true      1
#define false     0

//Button options
#define BT_DEL      0x80000000
#define BT_HIDE     0x40000000
#define BT_NOFRAME  0x20000000

//-------------------------------------------------------------------------

struct mouse
{
	int x,y,lkm,pkm,hor,vert;
	void get();
};

void mouse::get()
{
	EAX = 37;
	EBX = 1;
	$int	0x40
	$mov	ebx, eax
	$shr	eax, 16
	$and	ebx,0x0000FFFF
	x = EAX;
	y = EBX;
	EAX = 37;
	EBX = 2;
	$int	0x40
	$mov	ebx, eax
	$and	eax, 0x00000001
	$shr	ebx, 1
	$and	ebx, 0x00000001
	lkm = EAX;
	pkm = EBX;
	EAX = 37; //скролл
	EBX = 7;
	$int	0x40
	$mov	ebx, eax
	$shr	eax, 16
	$and	ebx,0x0000FFFF
	//hor = EAX;
	vert = EBX;
}


struct system_colors
{
	dword frame,grab,grab_button,grab_button_text,grab_text,work,work_button,work_button_text,work_text,work_graph;
	void get();
};

void system_colors::get()
{
	EAX = 48;
	EBX = 3;
	ECX = #frame;
	EDX = 40;
	$int 0x40
}

//------------------------------------------------------------------------------

inline fastcall dword WaitEvent()
{
	$mov eax,10
	$int 0x40
}

inline fastcall dword CheckEvent()
{
	$mov eax,11
	$int 0x40
}

inline fastcall dword WaitEventTimeout( EBX)
{
	$mov eax,23
	$int 0x40
} 
 
inline fastcall SetEventMask( EBX)
{
	$mov eax,40
	$int 0x40
}

inline fastcall ScancodesGeting(){
	$mov eax,66
	$mov ebx,1
	$mov ecx,1 //сканкоды
	$int 0x40
}

inline fastcall word GetKey()  //+Gluk fix
{
		$push edx
@getkey:
		$mov  eax,2
		$int  0x40
		$cmp eax,1
		$jne getkeyi
		$mov ah,dh
		$jmp getkeyii //jz?
@getkeyi:
		$mov dh,ah
		$jmp getkey
@getkeyii:
		$pop edx
		$shr eax,8
}


inline fastcall Pause( EBX)
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

//----------------------------------------

struct proc_info
{
	#define SelfInfo -1
	dword	use_cpu;
	word	pos_in_stack,num_slot,rezerv1;
	char	name[11];
	char	rezerv2;
	dword	adress,use_memory,ID,left,top,width,height;
	word	status_slot,rezerv3;
	dword	work_left,work_top,work_width,work_height;
	char	status_window;
	void	GetInfo( ECX);
	byte    reserved[1024-71];
};

inline fastcall void GetProcessInfo( EBX, ECX)
{
	$mov eax,9;
	$int  0x40
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

inline fastcall int CreateThread( ECX,EDX)
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

//eax =  ч√ъ ёшёЄхь√ (1=eng, 2=fi, 3=ger, 4=rus)
inline fastcall int GetSystemLanguage()
{
	EAX = 26;
	EBX = 5;
	$int 0x40
}

inline fastcall dword GetSkinHeight()
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

//------------------------------------------------------------------------------

void DefineAndDrawWindow(dword x,y, sizeX,sizeY, byte WindowType,dword WindowAreaColor, EDI)
{
	EAX = 12;              // function 12:tell os about windowdraw
	EBX = 1;
	$int 0x40
	
	EAX = 0;
	EBX = x << 16 + sizeX;
	ECX = y << 16 + sizeY;
	EDX = WindowType << 24 | WindowAreaColor;
	$int 0x40

	EAX = 12;              // function 12:tell os about windowdraw
	EBX = 2;
	$int 0x40
}

inline fastcall DeleteAllButtons()
{
	EAX = 12;              // function 12:tell os about windowdraw
	EBX = 1;
	$int 0x40
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

void WriteText(dword x,y,byte fontType, dword color, EDX, ESI)
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

void CopyScreen(dword EBX, x, y, sizeX, sizeY)
{
  EAX = 36;
  ECX = sizeX << 16 + sizeY;
  EDX = x << 16 + y;
  $int  0x40;
}

dword GetPixelColor(dword x, x_size, y)
{
	$mov eax, 35
	EBX= y*x_size+x;
	$int 0x40
}

void PutImage(dword EBX,w,h,x,y)
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
	EAX = 13;
	EBX = x<<16+w;
	ECX = y<<16+h;
 	$int 0x40
}

void DefineButton(dword x,y,w,h,EDX,ESI)
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
	$int 0x40;
}


//------------------------------------------------------------------------------

:void DrawRegion(dword x,y,width,height,color1)
{
	DrawBar(x,y,width,1,color1); //полоса гор сверху
	DrawBar(x,y+height,width,1,color1); //полоса гор снизу
	DrawBar(x,y,1,height,color1); //полоса верху слева
	DrawBar(x+width,y,1,height+1,color1); //полоса верху справа
}

:void DrawRegion_3D(dword x,y,width,height,color1,color2)
{
	DrawBar(x,y,width+1,1,color1); //полоса гор сверху
	DrawBar(x,y+1,1,height-1,color1); //полоса слева
	DrawBar(x+width,y+1,1,height,color2); //полоса справа
	DrawBar(x,y+height,width,1,color2); //полоса гор снизу
}

:void DrawFlatButton(dword x,y,width,height,id,color,text)
{
	DrawRegion_3D(x,y,width,height,0x94AECE,0x94AECE);
	DrawRegion_3D(x+1,y+1,width-2,height-2,0xFFFFFF,0xC7C7C7);
	DrawBar(x+2,y+2,width-3,height-3,color); //заливка
	IF (id<>0)	DefineButton(x,y,width,height,id+BT_HIDE,0xEFEBEF); //кнопка
	//WriteText(-strlen(text)*6+width/2+x+1,height/2-3+y,0x80,0,text,0);
	WriteText(width/2+x+1,height/2-3+y,0x80,0,text,0);
}

:void DrawCircle(int x, y, r)
{
	int i;
	float px=0, py=r, ii = r * 3.1415926 * 2;
	FOR (i = 0; i < ii; i++)
	{
        PutPixel(px + x, y - py, 0);
        px = py / r + px;
        py = -px / r + py;
	}
}

//------------------------------------------------------------------------------

inline fastcall void debug( EDX)
{
	$push ebx
	$push ecx
	$mov eax, 63
	$mov ebx, 1
next_char:
	$mov ecx, DSDWORD[edx]
	$or	 cl, cl
	$jz  done
	$int 0x40
	$inc edx
	$jmp next_char
done:
	$mov cl, 13
	$int 0x40
	$mov cl, 10
	$int 0x40
	$pop ecx
	$pop ebx
}
