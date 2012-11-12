//CODED by Veliant, Leency 2008-2012. GNU GPL licence.

#startaddress 0
#code32 TRUE

byte   os_name[8]   = {'M','E','N','U','E','T','0','1'};
dword  os_version   = 0x00000001;
dword  start_addr   = #main;
dword  final_addr   = #stop+32;
dword  alloc_mem    = 0x00070000;
dword  x86esp_reg   = 0x00070000;
dword  I_Param      = #param;
dword  I_Path       = #program_path;

char param[4096];
char program_path[4096];


//Events
#define evMouse		6
#define evButton	3
#define evKey		2
#define evReDraw	1

//Button options
#define BT_DEL		0x80000000
#define BT_HIDE		0x40000000
#define BT_NOFRAME	0x20000000

#define OLD			-1
#define true		1
#define false		0

#define NULL		0


struct mouse
{
	unsigned int x,y,lkm,pkm,hor,vert;
	void get();
};

inline fastcall int TestBit(EAX, CL)
{
	$shr eax,cl
	$and eax,1
}

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
	
	EAX = 37; //scroll
	EBX = 7;
	$int	0x40
	$mov	ebx, eax
	$shr	eax, 16
	$and	ebx,0x0000FFFF
	//hor = EAX;
	vert = EBX;
}

//---------------------------------------------------------------------------
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

void proc_info::GetInfo( EBX, ECX)
{
	$mov eax,9;
	$int 0x40
}

inline fastcall int GetSlot( ECX)
{
	$mov eax,18;
	$mov ebx,21;
	$int 0x40
}

inline fastcall int ActiveProcess()
{
	$mov eax,18;
	$mov ebx,7;
	$int 0x40
}

//-------------------------------------------------------------------------------

inline fastcall dword WaitEvent(){
	$mov eax,10;
	$int 0x40
}

inline fastcall void SetEventMask( EBX)
{
	EAX = 40;
	$int 0x40
}

inline fastcall word GetKey(){ //+Gluk fix
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
		EAX = EAX >> 8;		 
}

inline fastcall word GetButtonID(){
	EAX = 17;            // Get ID
	$int  0x40
	EAX = EAX >> 8;
}

inline fastcall void ExitProcess(){
	EAX = -1;            // close this program
	$int 0x40
}

inline fastcall void Pause( EBX){				
	$mov eax, 5
	$int 0x40
}

//------------------------------------------------------------------------------
void DefineAndDrawWindow(dword x,y,sizeX,sizeY,byte mainAreaType, dword mainAreaColour, EDI)
{
	EAX = 12;              // function 12:tell os about windowdraw
	EBX = 1;
	$int 0x40
	
	EBX = x << 16 + sizeX;
	ECX = y << 16 + sizeY;
	EDX = mainAreaType << 24 | mainAreaColour;
	$xor eax,eax
	$int 0x40

	EAX = 12;              // function 12:tell os about windowdraw
	EBX = 2;
	$int 0x40
}

inline fastcall void CreateThread( ECX,EDX)
{
	EAX = 51;
	EBX = 1;
	$int 0x40
}

inline fastcall void DrawTitle( ECX){
	EAX = 71;
	EBX = 1;
	$int 0x40;
}

inline fastcall dword GetSkinHeight()
{
	$push ebx
	$mov  eax,48
	$mov  ebx,4
	$int 0x40
	$pop  ebx
}

inline fastcall dword GetScreenHeight()
{
	EAX = 14;
	$int 0x40
	$and eax,0x0000FFFF
}

inline fastcall void MoveSize( EBX,ECX,EDX,ESI){
	$mov eax,67;
	$int 0x40
}

//------------------------------------------------------------------------------

inline fastcall dword strlen( EDI)
{
	asm {
	  xor ecx, ecx
	  xor eax, eax
	  dec ecx
	  repne scasb
	  sub eax, 2
	  sub eax, ecx
	}
}


inline fastcall copystr( ESI,EDI)
{
	$cld
l1:
	$lodsb
	$stosb
	$test al,al
	$jnz l1
}

char buffer[11];
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

dword StrToCol(char* htmlcolor)
{
  dword j, color=0;
  char ch=0x00;
  
  FOR (j=0; j<6; j++)
  {
    ch=ESBYTE[htmlcolor+j];
    IF ((ch>='0') && (ch<='9')) ch -= '0';
    IF ((ch>='A') && (ch<='F')) ch -= 'A'-10;
    IF ((ch>='a') && (ch<='f')) ch -= 'a'-10;
    color = color*0x10 + ch;
  }
  
  return color;
}

inline fastcall int strcmp(ESI, EDI)
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


inline fastcall signed int strncmp( ESI, EDI, ECX)
{
  asm {
    MOV EBX, EDI
    XOR EAX, EAX
    MOV EDX, ECX
    OR ECX, ECX
    JE L1
    REPNE SCASB
    SUB EDX, ECX
    MOV ECX, EDX
    MOV EDI, EBX
    XOR EBX, EBX
    REPE CMPSB
    MOV AL, DSBYTE[ ESI-1]
    MOV BL, DSBYTE[ EDI-1]
    SUB EAX, EBX
L1:
  }
}


inline fastcall unsigned int strchr(ESI,BL)
{
	int jj=0, last=-1;
	do{
		jj++;
		$lodsb
		IF(AL==BL) last=jj;
	} while(AL!=0);
	return last;
}


inline fastcall TitleCase( EDX)
{
	AL=DSBYTE[EDX];
	IF(AL>='a')&&(AL<='z')DSBYTE[EDX]=AL&0x5f;
	IF (AL>=160) && (AL<=175) DSBYTE[EDX] = AL - 32;	//а-п
	IF (AL>=224) && (AL<=239) DSBYTE[EDX] = AL - 80;	//а-п
	do{
		EDX++;
		AL=DSBYTE[EDX];
		IF(AL>='A')&&(AL<='Z'){DSBYTE[EDX]=AL|0x20; CONTINUE;}
		IF(AL>='А')&&(AL<='П')DSBYTE[EDX]=AL|0x20; //†-ѓ
		IF (AL>=144) && (AL<=159) DSBYTE[EDX] = AL + 80;	//а-п
	}while(AL!=0);
}


//------------------------------------------------------------------------------
inline fastcall void PutPixel( EBX,ECX,EDX)
{
  EAX=1;
  $int 0x40
}

void DefineButton(dword x,y,w,h,EDX,ESI)
{
 	EAX = 8;
	$push edx
	EDX += BT_DEL; //вначале удал€ем кнопу с эти ид, потом создаЄм
	$int 0x40;
	EBX = x<<16+w;
	ECX = y<<16+h;
 	$pop edx
	$int 0x40
}

inline fastcall void DeleteButton( EDX)
{
	EAX = 8;
	EDX += BT_DEL;
	$int 0x40;
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

void DrawRegion_3D(dword x,y,width,height,color1,color2)
{
	DrawBar(x,y,width+1,1,color1);
	DrawBar(x,y+1,1,height-1,color1);
	DrawBar(x+width,y+1,1,height,color2);
	DrawBar(x,y+height,width,1,color2);
}

void DrawFlatButton(dword x,y,width,height,id,color,text)
{
	DrawRegion_3D(x,y,width,height,0x94AECE,0x94AECE);
	DrawRegion_3D(x+1,y+1,width-2,height-2,0xFFFFFF,0xC7C7C7);
	DrawBar(x+2,y+2,width-3,height-3,color);
	IF (id<>0)	DefineButton(x+1,y+1,width-2,height-2,id+BT_HIDE,0xEFEBEF);
	WriteText(-strlen(text)*6+width/2+x+1,height/2-3+y,0x80,0,text,0);
}

void PutPaletteImage(dword EBX,w,h,x,y, EDI)
{
	EAX = 65;
	ECX = w<<16+h;
	EDX = x<<16+y;
	ESI = 8;
	EBP = 0;
	$int 0x40
} 

void PutImage(dword EBX,w,h,x,y)
{
	EAX = 7;
	ECX = w<<16+h;
	EDX = x<<16+y;
	$int 0x40
}

//------------------------------------------------------------------------------
inline fastcall void debug( EDX)
{
	$push eax
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
	$pop eax
	$pop ebx
	$pop ecx
}