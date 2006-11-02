#include "crt.h"

#define atexitBufferSize	32

char pureCallMessage[] = "PURE function call!";

char *__argv = 0;

void (__cdecl *atExitList[atexitBufferSize])();
int atExitFnNum = 0;

void exit()
{	int i;

	for ( i = atExitFnNum - 1; i >= 0; i-- )
		atExitList[i]();
	
	__asm
	{
		mov eax, -1
		int 0x40
  };
}; 

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

int __cdecl _purecall()
{
	exit();
	return 0;
}

#pragma section(".CRT$XCA",long,read,write)
#pragma section(".CRT$XCZ",long,read,write)
typedef void (__cdecl *_PVFV)(void);
__declspec(allocate(".CRT$XCA"))  _PVFV __xc_a[1] = { 0 };
__declspec(allocate(".CRT$XCZ"))  _PVFV __xc_z[1] = { 0 };
//
#pragma comment(linker, "/merge:.CRT=.rdata")
//
void crtStartUp()
{_PVFV *pbegin;

	_asm {fninit};

	for ( pbegin = __xc_a; pbegin < __xc_z; pbegin++ )
	{
		//
		if ( *pbegin != 0 )
			(**pbegin)();
	}
	__argv = *((char **)0x1C);
	main();
	exit();
}


