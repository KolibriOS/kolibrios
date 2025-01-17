//CODED by Veliant, Leency, Nable. GNU GPL licence.

#startaddress 0
#code32 TRUE

char   os_name[8]   = {'M','E','N','U','E','T','0','1'};
dword  os_version   = 0x00000001;
dword  start_addr   = #main;
dword  final_addr   = #stop+32;
dword  alloc_mem    = #0x00010000;
dword  x86esp_reg   = #0x00010000;
dword  I_Param      = 0;
dword  I_Path       = 0;

//Events
#define evMouse     6
#define evButton    3
#define evKey       2
#define evReDraw    1

//Button options
#define BT_DEL		0x80000000
#define BT_HIDE		0x40000000
#define BT_NOFRAME	0x20000000

//-------------------------------------------------------------------------


struct proc_info{
	dword	use_cpu;
	word	pos_in_stack,num_slot,rezerv1;
	char	name[11];
	char	rezerv2;
	dword	adress,use_memory,ID,left,top,width,height;
	word	status_slot,rezerv3;
	dword	work_left,work_top,work_width,work_height;
	char	status_window;
	void	GetInfo(dword ECX);
	byte    reserved[1024-71];
#define SelfInfo -1
};

inline fastcall void GetProcessInfo(dword EBX, ECX)
{
	EAX = 9;
	$int  0x40
}

struct system_colors{
	dword frame,grab,work_dark,work_light,grab_text,work,work_button,work_button_text,work_text,work_graph;
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

inline fastcall dword WaitEvent(){
 EAX = 10;
 $int 0x40
}

inline fastcall int GetKeyScancode()
{
	$mov  eax,2
	$int  0x40
	$shr  eax,16
	return AL;
}

word GetButtonID(){
 EAX = 17;
 $int  0x40
 EAX = EAX >> 8;
}

inline fastcall ExitProcess(){
 EAX = -1;              // close this program
 $int 0x40
}

inline fastcall Pause(dword EBX)
{					//Пауза, в сотых долях секунды EBX = value
	$mov eax, 5
	$int 0x40
}

//------------------------------------------------------------------------------

char buffer[11]="";
inline fastcall dword IntToStr(dword ESI)
{
     $mov     edi, #buffer
     $mov     ecx, 10
     $test     esi, esi
     $jns     f1
     $mov     al, '-'
     $stosb
     $neg     esi
f1:
     $mov     eax, esi
     $push     -'0'
f2:
     $xor     edx, edx
     $div     ecx
     $push     edx
     $test     eax, eax
     $jnz     f2
f3:
     $pop     eax
     $add     al, '0'
     $stosb
     $jnz     f3
     $mov     eax, #buffer
     $ret
} 


//------------------------------------------------------------------------------


void DefineAndDrawWindow(dword x,y,sizeX,sizeY,byte mainAreaType,dword mainAreaColour,byte headerType,dword headerColour,EDI)
{
	EAX = 12;              // function 12:tell os about windowdraw
	EBX = 1;
	$int 0x40
	
	EBX = x << 16 + sizeX;
	ECX = y << 16 + sizeY;
	EDX = mainAreaType << 24 | mainAreaColour;
	ESI = headerType << 24 | headerColour;
	$xor eax,eax
	$int 0x40

	EAX = 12;              // function 12:tell os about windowdraw
	EBX = 2;
	$int 0x40
}

inline fastcall MoveSize(int EBX,ECX,EDX,ESI)
{
	EAX = 67;
	$int 0x40
}

inline fastcall dword CreateThread(dword ECX,EDX)
{
	EAX = 51;
	EBX = 1;
	$int 0x40
}

dword GetSkinHeight()
{
	$push ebx
	$mov  eax, 48
	$mov  ebx, 4
	$int  0x40
	$pop  ebx
}

void WriteText(dword x,y,byte fontType, dword color, EDX, ESI)
{
	EAX = 4;
	EBX = x<<16+y;
	ECX = fontType<<24+color;
	$int 0x40;
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

void DrawRegion(dword x,y,width,height,EDX)
{
	DrawBar(x,y,width,1,EDX);
	DrawBar(x,y+height,width,1,EDX);
	DrawBar(x,y,1,height,EDX);
	DrawBar(x+width,y,1,height+1,EDX);
}

inline fastcall dword WriteDebug(dword EDX)
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