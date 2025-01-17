#pragma once

typedef unsigned __int32 Dword;
typedef unsigned __int16 Word;
typedef unsigned __int8 Byte;
//typedef unsigned __int32 size_t;

extern "C" char kosCmdLine[];	// command line initialized by OS
extern "C" char kosExePath[];	// path to binary initialized by OS

#define NULL 0

#define MAX_PATH				256

#define FO_READ					0
#define FO_WRITE				2

#define EM_WINDOW_REDRAW		1
#define EM_KEY_PRESS			2
#define EM_BUTTON_CLICK			4
#define EM_APP_CLOSE			8
#define EM_DRAW_BACKGROUND		16
#define EM_MOUSE_EVENT			32
#define EM_IPC					64
#define EM_NETWORK				256

#define KM_CHARS				0
#define KM_SCANS				1

#define WRS_BEGIN				1
#define WRS_END					2

#define PROCESS_ID_SELF			-1

#define abs(a) (a<0?0-a:a)

extern "C" double __cdecl acos(double x);
extern "C" double __cdecl asin(double x);
extern "C" double __cdecl floor(double x);
extern "C" double __cdecl round(double x);
#pragma function(acos,asin)
#if _MSC_VER > 1200
#pragma function(floor)
#endif


struct kosFileInfo
{
	Dword rwMode;
	Dword OffsetLow;
	Dword OffsetHigh;
	Dword dataCount;
	Byte *bufferPtr;
	char fileURL[MAX_PATH];
};

struct Point
{
	int X;
	int Y;

	Point() {};
	Point(int x, int y)
	{
		this->X = x;
		this->Y = y;
	};
	bool operator != (Point &a){ return this->X != a.X || this->Y != a.Y; };
	bool operator == (Point &a){ return this->X == a.X && this->Y == a.Y; };
	Point operator + (Point &a){ return Point(this->X + a.X, this->Y + a.Y); }
	Point operator + (int a){ return Point(this->X + a, this->Y + a); }
	Point operator - (Point &a){ return Point(this->X - a.X, this->Y - a.Y); }
	Point operator - (int a){ return Point(this->X - a, this->Y - a); }
	Point operator * (Point &a){ return Point(this->X * a.X, this->Y * a.Y); }
	Point operator * (int a){ return Point(this->X * a, this->Y * a); }
	Point operator / (Point &a){ return Point(this->X / a.X, this->Y / a.Y); }
	Point operator / (int a){ return Point(this->X / a, this->Y / a); }
	
	float Lingrh();
	float Angle();
};

struct Rect
{
	int X;
	int Y;
	int Width;
	int Height; 
	Rect() {}
	Rect(int X, int Y, int Width, int Height)
	{
		this->X = X;
		this->Y = Y;
		this->Width = Width;
		this->Height = Height;
	}
};

struct RGBA
{
	Byte b;
	Byte g;
	Byte r;
	Byte a;

	RGBA() {};

	RGBA(Dword value)
	{
		a = (Byte)(value >> 24);
		r = (Byte)(value >> 16);
		g = (Byte)(value >> 8);
		b = (Byte)value;
	};
	//
	bool operator != (RGBA &another)
	{
		return this->b != another.b || this->g != another.g || this->r != another.r || this->a != another.a;
	};
	//
	bool operator == (RGBA &another)
	{
		return this->b == another.b && this->g == another.g && this->r == another.r && this->a == another.a;
	};
};


struct RGB
{
	Byte b;
	Byte g;
	Byte r;
	//
	RGB() {};
	//
	RGB( Dword value )
	{
		r = (Byte)(value >> 16);
		g = (Byte)(value >> 8);
		b = (Byte)value;
	};
	//
	bool operator != ( RGB &another )
	{
		return this->b != another.b || this->g != another.g || this->r != another.r;
	};
	//
	bool operator == ( RGB &another )
	{
		return this->b == another.b && this->g == another.g && this->r == another.r;
	};
};

struct Player
{
	Point vector;
	Point position;
	float angle;

	Player() {}
	Player(Point position)
	{
		this->vector = Point(0, -1);
		this->position = position;
		this->angle = 270;
	};
};

#pragma pack(push, 1)
union sProcessInfo
{
	Byte rawData[1024];
	struct
	{
		Dword cpu_usage;
		Word window_stack_position;
		Word window_stack_value;
		Word reserved1;
		char process_name[12];
		Dword memory_start;
		Dword used_memory;
		Dword PID;
		Dword x_start;
		Dword y_start;
		Dword x_size;
		Dword y_size;
		Word slot_state;
	} processInfo;
};
#pragma pack(pop)

//
extern "C" void __cdecl crtStartUp();
//
int __cdecl _purecall();
//
int __cdecl atexit( void (__cdecl *func )( void ));
//
void rtlSrand( Dword seed );
Dword rtlRand( void );
//
char * __cdecl strcpy( char *target, const char *source );
int __cdecl strlen( const char *line );
char * __cdecl strrchr( const char * string, int c );

#if _MSC_VER < 1400
extern "C" void * __cdecl memcpy( void *dst, const void *src, size_t bytesCount );
extern "C" void memset( Byte *dst, Byte filler, Dword count );
//#pragma intrinsic(memcpy,memset)
#else
void * __cdecl memcpy( void *dst, const void *src, size_t bytesCount );
void memset( Byte *dst, Byte filler, Dword count );
#endif

unsigned int num2hex( unsigned int num );
void sprintf( char *Str, char* Format, ... );
//
Dword rtlInterlockedExchange( Dword *target, Dword value );
// функция -1 завершения процесса
void __declspec(noreturn) kos_ExitApp();
// функция 0
void kos_DefineAndDrawWindow(
	Word x, Word y,
	Word sizeX, Word sizeY,
	Byte mainAreaType, Dword mainAreaColour,
	Byte headerType, Dword headerColour,
	Dword borderColour
	);
// функция 1 поставить точку
void kos_PutPixel( Dword x, Dword y, Dword colour );
// функция 2 получить код нажатой клавиши
bool kos_GetKey( Byte &keyCode );
// функция 3 получить время
Dword kos_GetSystemClock();
// функция 4
void kos_WriteTextToWindow(
	Word x, Word y,
	Byte fontType,
	Dword textColour,
	const char *textPtr,
	Dword textLen
	);
// функция 7 нарисовать изображение
void kos_PutImage( RGB * imagePtr, Word sizeX, Word sizeY, Word x, Word y );
// функция 8 определить кнопку
void __declspec(noinline) kos_DefineButton( Word x, Word y, Word sizeX, Word sizeY, Dword buttonID, Dword colour );
// функция 5 пауза, в сотых долях секунды
void kos_Pause( Dword value );
// функция 9 - информация о процессе
Dword kos_ProcessInfo( sProcessInfo *targetPtr, Dword processID = PROCESS_ID_SELF );
// функция 10
Dword kos_WaitForEvent();
// функция 11
Dword kos_CheckForEvent();
// функция 12
void kos_WindowRedrawStatus( Dword status );
// функция 13 нарисовать полосу
void __declspec(noinline) kos_DrawBar( Word x, Word y, Word sizeX, Word sizeY, Dword colour );
// функция 17
bool kos_GetButtonID( Dword &buttonID );
// функция 23
Dword kos_WaitForEventTimeout( Dword timeOut );
//
enum eNumberBase
{
	nbDecimal = 0,
	nbHex,
	nbBin
};
// получение информации о состоянии "мыши" функция 37
void kos_GetMouseState( Dword & buttons, int & cursorX, int & cursorY );
// функция 40 установить маску событий
void kos_SetMaskForEvents( Dword mask );
// функция 47 вывести в окно приложения число
void kos_DisplayNumberToWindow(
   Dword value,
   Dword digitsNum,
   Word x,
   Word y,
   Dword colour,
   eNumberBase nBase = nbDecimal,
   bool valueIsPointer = false
   );
// функция 58 доступ к файловой системе
Dword kos_FileSystemAccess( kosFileInfo *fileInfo );
// функция 63
void kos_DebugOutChar( char ccc );
//
void rtlDebugOutString( char *str );
// функция 64 изменить параметры окна, параметр == -1 не меняется
void kos_ChangeWindow( Dword x, Dword y, Dword sizeX, Dword sizeY );
// функция 67 изменение количества памяти, выделенной для программы
bool kos_ApplicationMemoryResize( Dword targetSize );
// функция 66 режим получения данных от клавиатуры
void kos_SetKeyboardDataMode( Dword mode );

void kos_InitHeap();

//
void kos_Main();
