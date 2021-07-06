#include "kosSyst.h"
static char pureCallMessage[] = "PURE function call!";

// вызов абстрактного метода
int __cdecl _purecall()
{
	rtlDebugOutString( pureCallMessage );
	kos_ExitApp();
	return 0;
}

