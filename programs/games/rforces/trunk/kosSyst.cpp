#include "kosSyst.h"
#include <stdarg.h>

#define atexitBufferSize	32


char pureCallMessage[] = "PURE function call!";

char *kosExePath = NULL;

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


//
Dword RandomSeed = 1;
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

//
void * __cdecl memcpy( void *dst, const void *src, size_t bytesCount )
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


//////////////////////////////////////////////////////////////////////
//
// вывод строки на печать
//

Dword dectab[] = { 1000000000, 100000000, 10000000, 1000000, 100000,
                   10000, 1000, 100, 10, 0 };

//
void sprintf( char *Str, char* Format, ... )
{
	int i, fmtlinesize, j, k, flag;
	Dword head, tail;
	char c;
	va_list arglist;
	//
	va_start(arglist, Format);

	//
	fmtlinesize = strlen( Format );
	//
	if( fmtlinesize == 0 ) return;
  
	//
	for( i = 0, j = 0; i < fmtlinesize; i++ )
	{
		//
		c = Format[i];
		//
		if( c != '%' )
		{
			Str[j++] = c;
			continue;
		}
		//
		i++;
		//
		if( i >= fmtlinesize ) break;

		//
		flag = 0;
		//
		c = Format[i];
		//
		switch( c )
		{
		//
		case '%':
			Str[j++] = c;
			break;
		// вывод строки
		case 'S':
			Byte* str;
			str = va_arg(arglist, Byte*);
			for( k = 0; ( c = str[k] ) != 0; k++ )
			{
				Str[j++] = c;
			}
			break;
		// вывод байта
		case 'B':
			k = va_arg(arglist, int) & 0xFF;
			Str[j++] = num2hex( ( k >> 4 ) & 0xF );
			Str[j++] = num2hex( k & 0xF );
			break;
		// вывод символа
		case 'C':
			Str[j++] = va_arg(arglist, int) & 0xFF;
			break;
		// вывод двойного слова в шестнадцатиричном виде
		case 'X':
			Dword val;
			val = va_arg(arglist, Dword);
			for( k = 7; k >= 0; k-- )
			{
				//
				c = num2hex ( ( val >> (k * 4) ) & 0xF );
				//
				if( c == '0' )
				{
					if( flag ) Str[j++] = c;
				}
				else
				{
					flag++;
					Str[j++] = c;
				}
			}
			//
			if( flag == 0 ) Str[j++] = '0';
			break;
		// вывод двойного слова в десятичном виде
		case 'U':
			head = va_arg(arglist, Dword);
			tail = 0;
			for( k = 0; dectab[k] != 0; k++ )
			{
				tail = head % dectab[k];
				head /= dectab[k];
				c = head + '0';
				if( c == '0' )
				{
					if( flag ) Str[j++] = c;
				}
				else
				{
					flag++;
					Str[j++] = c;
				}
				//
				head = tail;
			}
			//
			c = head + '0';
			Str[j++] = c;
			break;
		// вывод 64-битного слова в шестнадцатиричном виде
		case 'Q':
			unsigned int low_dword, high_dword;
			low_dword = va_arg(arglist, unsigned int);
			high_dword = va_arg(arglist, unsigned int);
			for( k = 7; k >= 0; k-- )
			{
				//
				c = num2hex ( ( ( high_dword + 1) >> (k * 4) ) & 0xF );
				//
				if( c == '0' )
				{
					if( flag ) Str[j++] = c;
				}
				else
				{
					flag++;
					Str[j++] = c;
				}
			}
			//
			for( k=7; k >= 0; k-- )
			{
				//
				c = num2hex ( ( low_dword >> (k * 4) ) & 0xF );
				//
				if( c == '0' )
				{
					if( flag ) Str[j++] = c;
				}
				else
				{
					flag++;
					Str[j++] = c;
				}
			}
			//
			if( flag == 0 ) Str[j++] = '0';
			//
			break;
		//
		default:
			break;
		}
	}
	//
	Str[j] = 0;
}


// функция -1 завершения процесса
void kos_ExitApp()
{
	int i;

	//
	for ( i = atExitFnNum - 1; i >= 0; i-- )
	{
		//
		atExitList[i]();
	}
	//
	__asm{
		mov eax, -1
		int 0x40
	}
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
	__asm{
		mov eax, 0
		mov ebx, arg1
		mov ecx, arg2
		mov edx, arg3
		mov esi, arg4
		mov edi, borderColour
		int 0x40
	}
}


// функция 1 поставить точку
void kos_PutPixel( Dword x, Dword y, Dword colour )
{
	//
	__asm{
		mov eax, 1
		mov ebx, x
		mov ecx, y
		mov edx, colour
		int 0x40
	}
}


// функция 2 получить код нажатой клавиши
bool kos_GetKey( Byte &keyCode )
{
	Dword result;

	//
	__asm{
		mov eax, 2
		int 0x40
		mov result, eax
	}
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
		mov eax, 3
		int 0x40
//		mov result, eax
	}
	//
//	return result;
}


// функция 4
void kos_WriteTextToWindow(
	Word x,
	Word y,
	Byte fontType,
	Dword textColour,
	char *textPtr,
	Dword textLen
	)
{
	Dword arg1, arg2;

	//
	arg1 = ( x << 16 ) | y;
	arg2 = ( fontType << 24 ) | textColour;
	//
	__asm{
		mov eax, 4
		mov ebx, arg1
		mov ecx, arg2
		mov edx, textPtr
		mov esi, textLen
		int 0x40
	}
}


// функция 5 пауза, в сотых долях секунды
void kos_Pause( Dword value )
{
	//
	__asm{
		mov eax, 5
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
		mov eax, 7
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
		mov eax, 8
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
		mov eax, 9
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
		mov eax, 10
		int 0x40
//		mov result, eax
	}
	
//	return result;
}


// функция 11
Dword kos_CheckForEvent()
{
//	Dword result;

	__asm{
		mov eax, 11
		int 0x40
//		mov result, eax
	}
	
//	return result;
}


// функция 12
void kos_WindowRedrawStatus( Dword status )
{
	__asm{
		mov eax, 12
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
		mov eax, 13
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
		mov eax, 17
		int 0x40
		mov result, eax
	}
	//
	buttonID = result >> 8;
	//
	return (result & 0xFF) == 0;
}


// функция 23
Dword kos_WaitForEvent( Dword timeOut )
{
//	Dword result;

	__asm{
		mov eax, 23
		mov ebx, timeOut
		int 0x40
//		mov result, eax
	}
	
//	return result;
}

// функция 26.9 получить значение счётчика времени
Dword kos_GetTime()
{
	__asm{
		mov eax, 26
		mov ebx, 9
		int 0x40
	}
}

// функция 37 получение информации о состоянии "мыши"
void kos_GetMouseState( Dword & buttons, int & cursorX, int & cursorY )
{
	Dword mB;
	Word curX;
	Word curY;
	sProcessInfo sPI;

	//
	__asm{
		mov		eax, 37
		mov		ebx, 0
		int		0x40
		mov		curY, ax
		shr		eax, 16
		mov		curX, ax
		mov		eax, 37
		mov		ebx, 2
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

// функция 37.1 получение координат "мыши" относительно окна
void kos_GetMouseWindowXY( int & cursorX, int & cursorY )
{
	Word curX;
	Word curY;

	__asm{
		mov eax, 37
		mov ebx, 1
		int 0x40

		mov ebx, eax
		shr eax, 16
		and ebx, 0xffff

		mov curX, ax
		mov curY, bx
	}

	cursorX = curX;
	cursorY = curY;
}

// функция 37.2 получение информации о нажатых кнопки "мыши"
void kos_GetMouseButtonsState( Dword & buttons )
{
	Dword mB;

	__asm{
		mov		eax, 37
		mov		ebx, 2
		int		0x40
		mov		mB, eax
	}

	buttons = mB;
}

// функция 37.4 загрузка курсора "мыши"
Dword * kos_LoadMouseCursor( Dword * cursor, Dword loadstate )
{
	//Dword handle;
	__asm{
		mov		eax, 37
		mov		ebx, 4
		mov		edx, loadstate
		mov		ecx, cursor
		int		0x40
		//mov		handle, eax
	}
	//return handle;
}

// функция 37.5 установка курсора "мыши"
Dword * kos_SetMouseCursor( Dword * handle )
{
	//Dword handle;
	__asm{
		mov		eax, 37
		mov		ebx, 5
		mov		ecx, handle
		int		0x40
		//mov		handle, eax
	}
	//return handle;
}

// функция 37.6 удаление курсора "мыши"
void kos_DeleteMouseCursor( Dword * handle )
{
	__asm{
		mov		eax, 37
		mov		ebx, 6
		mov		ecx, handle
		int		0x40
	}
}

// функция 38 нарисовать полосу
void kos_DrawLine( Word x1, Word y1, Word x2, Word y2, Dword colour )
{
	Dword arg1, arg2;

	//
	arg1 = ( x1 << 16 ) | x2;
	arg2 = ( y1 << 16 ) | y2;
	//
	__asm{
		mov eax, 38
		mov ebx, arg1
		mov ecx, arg2
		mov edx, colour
		int 0x40
	}
}


// функция 40 установить маску событий
void kos_SetMaskForEvents( Dword mask )
{
	//
	__asm{
		mov eax, 40
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
		mov eax, 47
		mov ebx, arg1
		mov ecx, value
		mov edx, arg2
		mov esi, colour
		int 0x40
	}
}

// функция 47 вывести в окно приложения число c фоном
void kos_DisplayNumberToWindowBg(
   Dword value,
   Dword digitsNum,
   Word x,
   Word y,
   Dword colour,
   Dword bgcolour,
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
		mov eax, 47
		mov ebx, arg1
		mov ecx, value
		mov edx, arg2
		mov esi, colour
		or	esi, 0x40000000
		mov edi, bgcolour
		int 0x40
	}
}


// функция 70 доступ к файловой системе
Dword kos_FileSystemAccess( kosFileInfo *fileInfo )
{
//	Dword result;

	//
	__asm{
		mov eax, 70
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
		mov eax, 63
		mov ebx, 1
		mov cl, ccc
		int 0x40
	}
}


// функция 66 режим получения данных от клавиатуры
void kos_SetKeyboardDataMode( Dword mode )
{
	//
	__asm{
		mov eax, 66
		mov ebx, 1
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
		mov eax, 64
		mov ebx, 1
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
		mov eax, 67
		mov ebx, x
		mov ecx, y
		mov edx, sizeX
		mov esi, sizeY
		int 0x40
	}
}



// вызов абстрактного метода
int __cdecl _purecall()
{
	rtlDebugOutString( pureCallMessage );
	kos_ExitApp();
	return 0;
}


// вызов статических инициализаторов
// заодно инициализация генератора случайных чисел
//#pragma section(".CRT$XCA",long,read,write)
//#pragma section(".CRT$XCZ",long,read,write)
#pragma data_seg(".CRT$XCA")
#pragma data_seg(".CRT$XCZ")
typedef void (__cdecl *_PVFV)(void);
__declspec(allocate(".CRT$XCA"))  _PVFV __xc_a[1] = { NULL };
__declspec(allocate(".CRT$XCZ"))  _PVFV __xc_z[1] = { NULL };
//
#pragma comment(linker, "/merge:.CRT=.rdata")
//
void crtStartUp()
{
	// вызываем инициализаторы по списку, NULL'ы игнорируем
	for ( _PVFV *pbegin = __xc_a; pbegin < __xc_z; pbegin++ )
	{
		//
		if ( *pbegin != NULL )
			(**pbegin)();
	}
	// инициализируем генератор случайных чисел
	rtlSrand( kos_GetSystemClock() );
	// путь к файлу процесса
	kosExePath = *((char **)0x20);
	// вызов главной функции приложения
	kos_Main();
	// выход
	kos_ExitApp();
}


