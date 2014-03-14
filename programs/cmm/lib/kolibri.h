//CODED by Veliant, Leency, Nable. GNU GPL licence.

#startaddress 0
#code32 TRUE

char   os_name[8]   = {'M','E','N','U','E','T','0','1'};
dword  os_version   = 0x00000001;
dword  start_addr   = #main;
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
#define evMouse   6
#define evNetwork 8


//Button options
#define BT_DEL      0x80000000
#define BT_HIDE     0x40000000
#define BT_NOFRAME  0x20000000

//-------------------------------------------------------------------------

struct mouse
{
	signed x,y,lkm,pkm,hor,vert;
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
	if (x>6000) x-=65535;
	if (y>6000) y-=65535;
	EAX = 37;
	EBX = 2;
	$int	0x40
	$mov	ebx, eax
	$and	eax, 0x00000001
	$shr	ebx, 1
	$and	ebx, 0x00000001
	lkm = EAX;
	pkm = EBX;
	EAX = 37; //бЄа®««
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
	dword frame,grab,grab_button,grab_button_text,grab_text,
	      work,work_button,work_button_text,work_text,work_graph;
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
	$mov ecx,1 //бЄ ­Є®¤л
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


inline fastcall pause( EBX)
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
	//return eax = размер свободной памяти в килобайтах
}

inline fastcall dword LoadDriver( ECX) //ECX - имя драйвера
{
	$mov eax, 68
	$mov ebx, 16
	$int 0x40
	//return 0 - неудача, иначе eax = хэндл драйвера 
}

inline fastcall dword RuleDriver( ECX) //указатель на управляющую структуру
{
	$mov eax, 68
	$mov ebx, 17
	$int 0x40
	//return eax = определяется драйвером
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

inline fastcall void GetProcessInfo( EBX, ECX)
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

//eax = язык системы (1=eng, 2=fi, 3=ger, 4=rus)
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
	
	EAX = 0;
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
	$int 0x40;
}