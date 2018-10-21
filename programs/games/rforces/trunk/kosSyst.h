typedef unsigned __int32 Dword;
typedef unsigned __int16 Word;
typedef unsigned __int8 Byte;
//typedef unsigned __int32 size_t;

#define NULL 0

#define MAX_PATH				256

#define FO_READ					0
#define FO_WRITE				2

//Process Events
#define EM_WINDOW_REDRAW		1
#define EM_KEY_PRESS			2
#define EM_BUTTON_CLICK			3
#define EM_APP_CLOSE			4
#define EM_DRAW_BACKGROUND		5
#define EM_MOUSE_EVENT			6
#define EM_IPC					7
#define EM_NETWORK				8
#define EM_DEBUG				9

#define KM_CHARS				0
#define KM_SCANS				1

#define WRS_BEGIN				1
#define WRS_END					2

#define PROCESS_ID_SELF			-1

//Event mask bits for function 40
#define EVM_REDRAW        1
#define EVM_KEY           2
#define EVM_BUTTON        4
#define EVM_EXIT          8
#define EVM_BACKGROUND    16
#define EVM_MOUSE         32
#define EVM_IPC           64
#define EVM_STACK         128
#define EVM_DEBUG         256
#define EVM_STACK2        512
#define EVM_MOUSE_FILTER  0x80000000
#define EVM_CURSOR_FILTER 0x40000000

//Button options
#define BT_DEL      0x80000000
#define BT_HIDE     0x40000000
#define BT_NOFRAME  0x20000000

#define abs(a) (a<0?0-a:a)


struct kosFileInfo
{
	Dword rwMode;
	Dword OffsetLow;
	Dword OffsetHigh;
	Dword dataCount;
	Byte *bufferPtr;
	char fileURL[MAX_PATH];
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
		r = value >> 16;
		g = value >> 8;
		b = value;
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

#ifndef AUTOBUILD
//
extern char *kosExePath;
#endif

//
void crtStartUp();
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
//
void * __cdecl memcpy( void *dst, const void *src, size_t bytesCount );
//
void memset( Byte *dst, Byte filler, Dword count );
//
void sprintf( char *Str, char* Format, ... );
//
Dword rtlInterlockedExchange( Dword *target, Dword value );
// function -1 завершения процесса
void kos_ExitApp();
// function 0
void kos_DefineAndDrawWindow(
	Word x, Word y,
	Word sizeX, Word sizeY,
	Byte mainAreaType, Dword mainAreaColour,
	Byte headerType, Dword headerColour,
	Dword borderColour
	);
// function 1 поставить точку
void kos_PutPixel( Dword x, Dword y, Dword colour );
// function 2 получить код нажатой клавиши
bool kos_GetKey( Byte &keyCode );
// function 3 получить время
Dword kos_GetSystemClock();
// function 4
void kos_WriteTextToWindow(
	Word x, Word y,
	Byte fontType,
	Dword textColour,
	char *textPtr,
	Dword textLen
	);
//
void kos_WriteTextWithBg(
	Word x, Word y,
	Byte fontType,
	Dword textColour,
	Dword bgcolour,
	char *textPtr,
	Dword textLen
	);
// function 7 нарисовать изображение
void kos_PutImage( RGB * imagePtr, Word sizeX, Word sizeY, Word x, Word y );
// function 8 определить кнопку
void kos_DefineButton( Word x, Word y, Word sizeX, Word sizeY, Dword buttonID, Dword colour );
// function 5 пауза, в сотых долях секунды
void kos_Pause( Dword value );
// function 9 информация о процессе
Dword kos_ProcessInfo( sProcessInfo *targetPtr, Dword processID = PROCESS_ID_SELF );
// function 10
Dword kos_WaitForEvent();
// function 11
Dword kos_CheckForEvent();
// function 12
void kos_WindowRedrawStatus( Dword status );
// function 13 нарисовать прямоугольник
void kos_DrawBar( Word x, Word y, Word sizeX, Word sizeY, Dword colour );
// function 17
bool kos_GetButtonID( Dword &buttonID );
// function 23
Dword kos_WaitForEvent( Dword timeOut );
// function 26.9 получить значение счётчика времени
Dword kos_GetTime();
//
enum eNumberBase
{
	nbDecimal = 0,
	nbHex,
	nbBin
};
// function 37 получение информации о состоянии "мыши"
void kos_GetMouseState( Dword & buttons, int & cursorX, int & cursorY );
// function 37.1 получение координат "мыши" относительно окна
void kos_GetMouseWindowXY( int & cursorX, int & cursorY );
// function 37.2 получение информации о нажатых кнопки "мыши"
void kos_GetMouseButtonsState( Dword & buttons );
// function 37.4 загрузка курсора "мыши"
Dword * kos_LoadMouseCursor( Dword * cursor, Dword loadstate );
// function 37.5 установка курсора "мыши"
Dword * kos_SetMouseCursor( Dword * handle );
// function 37.6 удаление курсора "мыши"
void kos_DeleteMouseCursor( Dword * handle );
// function 38 нарисовать полосу
void kos_DrawLine( Word x1, Word y1, Word x2, Word y2, Dword colour );
// function 40 установить маску событий
void kos_SetMaskForEvents( Dword mask );
// function 47 вывести в окно приложения число
void kos_DisplayNumberToWindow(
   Dword value,
   Dword digitsNum,
   Word x,
   Word y,
   Dword colour,
   eNumberBase nBase = nbDecimal,
   bool valueIsPointer = false
   );
// function 47 вывести в окно приложения число c фоном
void kos_DisplayNumberToWindowBg(
   Dword value,
   Dword digitsNum,
   Word x,
   Word y,
   Dword colour,
   Dword bgcolour,
   eNumberBase nBase = nbDecimal,
   bool valueIsPointer = false
   );
// function 48.4 get windows title bar height
Dword kos_GetSkinHeight();
// function 58 доступ к файловой системе
Dword kos_FileSystemAccess( kosFileInfo *fileInfo );
// function 63
void kos_DebugOutChar( char ccc );
//
void rtlDebugOutString( char *str );
//
void kos_DebugNumber(signed int n);
//
// function 64 изменить параметры окна, параметр == -1 не меняется
void kos_ChangeWindow( Dword x, Dword y, Dword sizeX, Dword sizeY );
// function 67 изменение количества памяти, выделенной для программы
bool kos_ApplicationMemoryResize( Dword targetSize );
// function 66 режим получения данных от клавиатуры
void kos_SetKeyboardDataMode( Dword mode );

//
void kos_Main();
