#include "crt.h"

#define atexitBufferSize	32

#ifdef AUTOBUILD
char kosExePath[1024];
char exeStack[16384];
char params[1024];
// must be alphabetically first in the image
#pragma data_seg(".1seg")
struct
{
	char header[8];
	int headerver;
	void* entry;
	void* i_end;
	void* memsize;
	void* stack;
	void* params;
	void* icon;
} __MENUET_APP_header = {
	{'M', 'E', 'N', 'U', 'E', 'T', '0', '1'},
	1,
	&crtStartUp,
	0,	// filled by doexe2.asm
	0,	// filled by doexe2.asm
	exeStack + sizeof(exeStack),
	params,
	kosExePath
};
#pragma data_seg()
#endif

char pureCallMessage[] = "PURE function call!";

//char *__argv[2];
//int __argc;

void (__cdecl *atExitList[atexitBufferSize])();
int atExitFnNum = 0;
int main(int argc, char *argv[]);

void exit()
{	/*int i;

	for ( i = atExitFnNum - 1; i >= 0; i-- )
		atExitList[i]();*/
	
	__asm
	{
		mov eax, -1
		int 0x40
  };
}; 

/*int __cdecl atexit( void (__cdecl *func )( void ))
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
}*/

int __cdecl _purecall()
{
	exit();
	return 0;
}

#pragma section(".CRT$XCA",long,read,write)
#pragma section(".CRT$XCZ",long,read,write)
typedef void (__cdecl *_PVFV)(void);
//__declspec(allocate(".CRT$XCA"))  _PVFV __xc_a[1] = { 0 };
//__declspec(allocate(".CRT$XCZ"))  _PVFV __xc_z[1] = { 0 };
//
#pragma comment(linker, "/merge:.CRT=.rdata")
//
void crtStartUp()
{_PVFV *pbegin;
	char* __argv[2];

#ifdef AUTOBUILD
	__MENUET_APP_header.header;
#endif

	_asm {fninit};

	/*for ( pbegin = __xc_a; pbegin < __xc_z; pbegin++ )
	{
		//
		if ( *pbegin != 0 )
			(**pbegin)();
	}
	__argc = 2;*/
#ifdef AUTOBUILD
	__argv[0] = kosExePath;
	__argv[1] = params;
#else
	__argv[0] = *((char **)0x20);
	__argv[1] = *((char **)0x1C);
#endif
	main(/*__argc*/2, __argv);
	exit();
}


