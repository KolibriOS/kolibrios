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
	dword x,y,lkm,pkm,hor,vert;
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

//------------------------------------------------------------------------------

inline fastcall word GetButtonID()
{
	$mov eax,17
	$int  0x40
	$shr eax,8
}

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

void GetProcessInfo( EBX, ECX)
{
	$mov eax,9;
	$int  0x40
}

int GetProcessSlot( ECX) //ECX = process ID
{
	EAX = 18;
	EBX = 21;
	$int 0x40;	
}

inline fastcall int ActiveProcess()
{
	EAX = 18;
	EBX = 7;
	$int 0x40
}


inline fastcall ExitProcess()
{
	$mov eax,-1;
	$int 0x40
}

inline fastcall int KillProcess( ECX)
{
	$mov eax,18;
	$mov ebx,18;
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

inline fastcall void DrawTitle( ECX)
{
	EAX = 71;
	EBX = 1;
	$int 0x40;
}

inline fastcall dword GetSkinWidth()
{
	$push ebx
	$mov  eax,48
	$mov  ebx,4
	$int 0x40
	$pop  ebx
}

inline fastcall void SetSystemSkin(ECX){
	EAX = 48;
	EBX = 8;
	$int 0x40
}

inline fastcall dword GetScreenWidth()
{
	EAX = 14;
	EBX = 4;
	$int 0x40
	$shr eax, 16
	$and eax,0x0000FFFF
}

inline fastcall MoveSize( EBX,ECX,EDX,ESI)
{
	EAX = 67;
	$int 0x40
}

inline fastcall dword LoadLibrary( ECX)
{
	$mov eax, 68 
	$mov ebx, 19
	$int  0x40
}

//------------------------------------------------------------------------------
inline fastcall dword strlen( EDI)
{
	EAX=0;
	ECX=-1;
	$REPNE $SCASB
	EAX-=2+ECX;
}


inline fastcall strcpy( EDI, ESI)
{
	$cld
l2:
	$lodsb
	$stosb
	$test al,al
	$jnz l2
}

inline fastcall strcat( EDI, ESI)
{
  asm {
    MOV EBX, EDI
    XOR ECX, ECX
    XOR EAX, EAX
    DEC ECX
    REPNE SCASB
    DEC EDI
    MOV EDX, EDI
    MOV EDI, ESI
    XOR ECX, ECX
    XOR EAX, EAX
    DEC ECX
    REPNE SCASB
    XOR ECX, 0FFFFFFFFH
    MOV EDI, EDX
    MOV EDX, ECX
    MOV EAX, EDI
    SHR ECX, 2
    REP MOVSD
    MOV ECX, EDX
    AND ECX, 3
    REP MOVSB
    MOV EAX, EBX
	}
}

char buffer[11]="";
inline fastcall dword IntToStr( ESI)
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


inline fastcall dword StrToInt()
{
	ESI=EDI=EAX;
	IF(DSBYTE[ESI]=='-')ESI++;
	EAX=0;
	BH=AL;
	do{
		BL=DSBYTE[ESI]-'0';
		EAX=EAX*10+EBX;
		ESI++;
	}while(DSBYTE[ESI]>0);
	IF(DSBYTE[EDI]=='-') -EAX;
}


inline fastcall int strcmp( ESI, EDI)
{
	loop()
	{
		IF (DSBYTE[ESI]<DSBYTE[EDI]) RETURN -1;
		IF (DSBYTE[ESI]>DSBYTE[EDI]) RETURN 1;
		IF (DSBYTE[ESI]=='\0') RETURN 0;
		ESI++;
		EDI++;
	}
}

inline fastcall unsigned int find_symbol( ESI,BL)
{
	int jj=0, last=-1;
	do{
		jj++;
		$lodsb
		IF(AL==BL) last=jj;
	} while(AL!=0);
	return last;
}


inline fastcall dword upcase( ESI)
{
	do{
		AL=DSBYTE[ESI];
		IF(AL>='a')IF(AL<='z')DSBYTE[ESI]=AL&0x5f;
 		ESI++;
	}while(AL!=0);
}

inline fastcall lowcase( ESI)
{
	do{
		$LODSB
		IF(AL>='A')&&(AL<='Z'){
			AL+=0x20;
			DSBYTE[ESI-1]=AL;
			CONTINUE;
		}
	}while(AL!=0);
}

byte fastcall TestBit( EAX, CL)
{
	$shr eax,cl
	$and eax,1
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


inline fastcall int CreateThread( ECX,EDX)
{
	EAX = 51;
	EBX = 1;
	$int 0x40
}

inline fastcall int GetSlot( ECX)
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


void WriteText(dword x,y,byte fontType, dword color, EDX, ESI)
{
	EAX = 4;
	EBX = x<<16+y;
	ECX = fontType<<24+color;
	$int 0x40;
}

void CopyScreen(dword EBX, x, y, sizeX, sizeY)
{
  EAX = 36;
  ECX = sizeX << 16 + sizeY;
  EDX = x << 16 + y;
  $int  0x40;
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

void DrawFlatButton(dword x,y,width,height,id,color,text)
{
	DrawRegion_3D(x,y,width,height,0x94AECE,0x94AECE);
	DrawRegion_3D(x+1,y+1,width-2,height-2,0xFFFFFF,0xC7C7C7);
	DrawBar(x+2,y+2,width-3,height-3,color); //заливка
	IF (id<>0)	DefineButton(x,y,width,height,id+BT_HIDE,0xEFEBEF); //кнопка
	WriteText(-strlen(text)*6+width/2+x+1,height/2-3+y,0x80,0,text,0);
}

:void DrawCircle(int x, y, r)
{
	int i; float px=0, py=r, ii = r * 3.1415926 * 2;
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
