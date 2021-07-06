#include "kosSyst.h"
#include "func.h"
#include <stdarg.h>

char kosCmdLine[257];
char kosExePath[1024];
extern "C" char exeStack[];
char exeStack[16384];

#define atexitBufferSize	32

#ifndef SMALLLIBC_NO_ATEXIT
//
void (__cdecl *atExitList[atexitBufferSize])();
int atExitFnNum = 0;
//
int __cdecl atexit( void (__cdecl *func )( void ))
{
	//
	if ( atExitFnNum < atexitBufferSize )
	{
		//
		atExitList[atExitFnNum++] = func;
		return 0;
	}
	else
	{
		return 1;
	}
}
#endif

//
Dword RandomSeed = 0;
//
void rtlSrand( Dword seed )
{
	RandomSeed = seed;
}
//
Dword rtlRand( void )
{
  //маска 0x80000776

  Dword dwi, i;

  for ( i = 0; i < 32; i++ )
  {

    dwi = RandomSeed & 0x80000776;
  
      __asm{
            mov   eax, dwi
            mov   edx, eax
            bswap eax
            xor   eax, edx
            xor   al, ah
            setpo al
            movzx eax, al
            mov   dwi, eax
    }

    RandomSeed = ( RandomSeed << 1 ) | ( dwi & 1 );
  }
  
 return RandomSeed;
}

void* __cdecl memcpy( void *dst, const void *src, size_t bytesCount )
{
	__asm{
		mov edi, dst
		mov eax, dst
		mov esi, src
		mov ecx, bytesCount
		rep movsb
	}
}

//
void memset( Byte *dst, Byte filler, Dword count )
{
	//
	__asm{
		mov edi, dst
		mov al, filler
		mov ecx, count
		rep stosb
	}
}


//
Dword rtlInterlockedExchange( Dword *target, Dword value )
{
//	Dword result;

	//
	__asm{
		mov eax, value
		mov ebx, target
		xchg eax, [ebx]
//		mov result, eax
	}
	//
//	return result;
}


//////////////////////////////////////////////////////////////////////
//
// копирование строки
//

char * __cdecl strcpy( char *target, const char *source )
{
	char *result = target;

	while( target[0] = source[0] )
	{
		target++;
		source++;
	}

	return result;
}


//////////////////////////////////////////////////////////////////////
//
// реверсивный поиск символа
//

char * __cdecl strrchr( const char * string, int c )
{
	char *cPtr;

	//
	for ( cPtr = (char *)string + strlen( string ); cPtr >= string; cPtr-- )
	{
		//
		if ( *cPtr == c ) return cPtr;
	}
	//
	return NULL;
}


//////////////////////////////////////////////////////////////////////
//
// определение длины строки
//

int __cdecl strlen( const char *line )
{
  int i;

  for( i=0; line[i] != 0; i++ );
  return i;
}



//////////////////////////////////////////////////////////////////////
//
// перевод шестнадцатиричного числа в символ
//

unsigned int num2hex( unsigned int num )
{
  if( num < 10 )
    return num + '0';
  return num - 10 + 'A';
}


inline void __declspec(noreturn) kos_sysfuncm1(void)
{
	__asm or eax, -1
	__asm int 0x40
}

// функция -1 завершения процесса
void kos_ExitApp()
{
#ifndef SMALLLIBC_NO_ATEXIT
	int i;

	//
	for ( i = atExitFnNum - 1; i >= 0; i-- )
	{
		//
		atExitList[i]();
	}
#endif
	//
	kos_sysfuncm1();
}

static void __declspec(noinline) __fastcall kos_sysfunc0(Dword _ecx, Dword _edx, Dword _ebx, Dword _esi, Dword _edi)
{
	__asm xor eax, eax
	__asm mov ebx, _ebx
	__asm mov esi, _esi
	__asm mov edi, _edi
	__asm int 0x40
}

// функция 0
void kos_DefineAndDrawWindow(
	Word x, Word y,
	Word sizeX, Word sizeY,
	Byte mainAreaType,
	Dword mainAreaColour,
	Byte headerType,
	Dword headerColour,
	Dword borderColour
	)
{
	Dword arg1, arg2, arg3, arg4;

	//
	arg1 = ( x << 16 ) + sizeX;
	arg2 = ( y << 16 ) + sizeY;
	arg3 = ( mainAreaType << 24 ) | mainAreaColour;
	arg4 = ( headerType << 24 ) | headerColour;
	//
	kos_sysfunc0(arg2, arg3, arg1, arg4, borderColour);
}


// функция 1 поставить точку
void kos_PutPixel( Dword x, Dword y, Dword colour )
{
	//
	__asm{
		push 1
		pop eax
		mov ebx, x
		mov ecx, y
		mov edx, colour
		int 0x40
	}
}

inline Dword kos_sysfunc2(void)
{
	__asm push 2
	__asm pop eax
	__asm int 0x40
}

// функция 2 получить код нажатой клавиши
bool kos_GetKey( Byte &keyCode )
{
	Dword result = kos_sysfunc2();
	//
	keyCode = result >> 8;
	//
	return ( result & 0xFF ) == 0;
}


// функция 3 получить время
Dword kos_GetSystemClock()
{
//	Dword result;

	//
	__asm{
		push 3
		pop eax
		int 0x40
//		mov result, eax
	}
	//
//	return result;
}

static void __declspec(noinline) __fastcall kos_sysfunc4(Dword _ecx, const char* _edx, Dword _ebx, Dword _esi)
{
	__asm push 4
	__asm pop eax
	__asm mov ebx, [_ebx]
	__asm mov esi, [_esi]
	__asm int 0x40
}

// функция 4
void kos_WriteTextToWindow(
	Word x,
	Word y,
	Byte fontType,
	Dword textColour,
	const char *textPtr,
	Dword textLen
	)
{
	Dword arg1, arg2;

	//
	arg1 = ( x << 16 ) | y;
	arg2 = ( fontType << 24 ) | textColour;
	//
	kos_sysfunc4(arg2, textPtr, arg1, textLen);
}


// функция 5 пауза, в сотых долях секунды
void kos_Pause( Dword value )
{
	//
	__asm{
		push 5
		pop eax
		mov ebx, value
		int 0x40
	}
}


// функция 7 нарисовать изображение
void kos_PutImage( RGB * imagePtr, Word sizeX, Word sizeY, Word x, Word y )
{
	Dword arg1, arg2;

	//
	arg1 = ( sizeX << 16 ) | sizeY;
	arg2 = ( x << 16 ) | y;
	//
	__asm{
		push 7
		pop eax
		mov ebx, imagePtr
		mov ecx, arg1
		mov edx, arg2
		int 0x40
	}
}



// функция 8 определить кнопку
void kos_DefineButton( Word x, Word y, Word sizeX, Word sizeY, Dword buttonID, Dword colour )
{
	Dword arg1, arg2;

	//
	arg1 = ( x << 16 ) | sizeX;
	arg2 = ( y << 16 ) | sizeY;
	//
	__asm{
		push 8
		pop eax
		mov ebx, arg1
		mov ecx, arg2
		mov edx, buttonID
		mov esi, colour
		int 0x40
	}
}


// функция 9 - информация о процессе
Dword kos_ProcessInfo( sProcessInfo *targetPtr, Dword processID )
{
//	Dword result;

	//
	__asm{
		push 9
		pop eax
		mov ebx, targetPtr
		mov ecx, processID
		int 0x40
//		mov result, eax
	}
	//
//	return result;
}


// функция 10
Dword kos_WaitForEvent()
{
//	Dword result;

	__asm{
		push 10
		pop eax
		int 0x40
//		mov result, eax
	}
	
//	return result;
}


// функция 11
Dword kos_CheckForEvent()
{
	Dword result; //

	__asm{
		push 11
		pop eax
		int 0x40
		mov result, eax //
	}
	
	return result; //
}


// функция 12
void kos_WindowRedrawStatus( Dword status )
{
	__asm{
		push 12
		pop eax
		mov ebx, status
		int 0x40
	}
}


// функция 13 нарисовать полосу
void kos_DrawBar( Word x, Word y, Word sizeX, Word sizeY, Dword colour )
{
	Dword arg1, arg2;

	//
	arg1 = ( x << 16 ) | sizeX;
	arg2 = ( y << 16 ) | sizeY;
	//
	__asm{
		push 13
		pop eax
		mov ebx, arg1
		mov ecx, arg2
		mov edx, colour
		int 0x40
	}
}


// функция 17
bool kos_GetButtonID( Dword &buttonID )
{
	Dword result;

	//
	__asm{
		push 17
		pop eax
		int 0x40
		mov result, eax
	}
	//
	buttonID = result >> 8;
	//
	return (result & 0xFF) == 0;
}


// функция 23
Dword kos_WaitForEventTimeout( Dword timeOut )
{
//	Dword result;

	__asm{
		push 23
		pop eax
		mov ebx, timeOut
		int 0x40
//		mov result, eax
	}
	
//	return result;
}


// получение информации о состоянии "мыши" функция 37
void kos_GetMouseState( Dword & buttons, int & cursorX, int & cursorY )
{
	Dword mB;
	Word curX;
	Word curY;
	sProcessInfo sPI;

	//
	__asm{
		push 37
		pop eax
		xor ebx, ebx
		int		0x40
		mov		curY, ax
		shr		eax, 16
		mov		curX, ax
		push 37
		pop eax
		push 2
		pop ebx
		int		0x40
		mov		mB, eax
	}
	//
	kos_ProcessInfo( &sPI );
	//
	buttons = mB;
	cursorX = curX - sPI.processInfo.x_start;
	cursorY = curY - sPI.processInfo.y_start;
}


// функция 40 установить маску событий
void kos_SetMaskForEvents( Dword mask )
{
	//
	__asm{
		push 40
		pop eax
		mov ebx, mask
		int 0x40
	}
}


// функция 47 вывести в окно приложения число
void kos_DisplayNumberToWindow(
   Dword value,
   Dword digitsNum,
   Word x,
   Word y,
   Dword colour,
   eNumberBase nBase,
   bool valueIsPointer
   )
{
	Dword arg1, arg2;

	//
	arg1 = ( valueIsPointer ? 1 : 0 ) |
		( ((Byte)nBase) << 8 ) |
		( ( digitsNum & 0x1F ) << 16 );
	arg2 = ( x << 16 ) | y;
	//
	__asm{
		push 47
		pop eax
		mov ebx, arg1
		mov ecx, value
		mov edx, arg2
		mov esi, colour
		int 0x40
	}
}


// функция 70 доступ к файловой системе
Dword kos_FileSystemAccess( kosFileInfo *fileInfo )
{
//	Dword result;

	//
	__asm{
		push 70
		pop eax
		mov ebx, fileInfo
		int 0x40
//		mov result, eax
	}
	//
//	return result;
}


// функция 63 вывод символя в окно отладки
void kos_DebugOutChar( char ccc )
{
	//
	__asm{
		push 63
		pop eax
		push 1
		pop ebx
		mov cl, ccc
		int 0x40
	}
}


// функция 66 режим получения данных от клавиатуры
void kos_SetKeyboardDataMode( Dword mode )
{
	//
	__asm{
		push 66
		pop eax
		push 1
		pop ebx
		mov ecx, mode
		int 0x40
	}
}


// вывод строки в окно отладки
void rtlDebugOutString( char *str )
{
	//
	for ( ; str[0] != 0; str++ )
	{
		kos_DebugOutChar( str[0] );
	}
	//
	kos_DebugOutChar( 13 );
	kos_DebugOutChar( 10 );
}


// функция 64 изменение количества памяти, выделенной для программы
bool kos_ApplicationMemoryResize( Dword targetSize )
{
	Dword result;

	//
	__asm{
		push 64
		pop eax
		push 1
		pop ebx
		mov ecx, targetSize
		int 0x40
		mov result, eax
	}
	//
	return result == 0;
}


// функция 67 изменить параметры окна, параметр == -1 не меняется
void kos_ChangeWindow( Dword x, Dword y, Dword sizeX, Dword sizeY )
{
	//
	__asm{
		push 67
		pop eax
		mov ebx, x
		mov ecx, y
		mov edx, sizeX
		mov esi, sizeY
		int 0x40
	}
}

void kos_InitHeap()
{
	__asm{
		push 68
		pop eax
		push 11
		pop ebx
		int 0x40
	}
}



// вызов статических инициализаторов
typedef void (__cdecl *_PVFV)(void);
extern "C" _PVFV __xc_a[];
extern "C" _PVFV __xc_z[];
#pragma comment(linker, "/merge:.CRT=.rdata")
//
void __cdecl crtStartUp()
{
#ifndef SMALLLIBC_NO_INIT
	// вызываем инициализаторы по списку
	for ( _PVFV *pbegin = __xc_a; pbegin < __xc_z; pbegin++ )
	{
		//
		(**pbegin)();
	}
#endif
	// инициализируем генератор случайных чисел
	// если надо для приложения, делать это в kos_Main()
	//rtlSrand( kos_GetSystemClock() );
	// вызов главной функции приложения
	kos_Main();
	// выход
	kos_ExitApp();
}


