/*
 * Author Pavel Iakovlev
*/

#define MEMSIZE 4096*10

#include "../lib/io.h"
#include "../lib/obj/console.h"
#include "../lib/array.h"

byte initConsole = 0;
Dictionary functions = {0};
Dictionary variables = {0};


#define bufferSize 10000;
#define memoryBrainfuck 30000*4
#define memoryByteBF 1
#define stackBrainFuck 4*1024

dword buffer = 0;
word bufferSymbol = 0;
dword memory = 0;

dword stack = 0;
dword code = 0;
byte tempBuffer[100] = {0};

void consoleInit()
{
	IF(!initConsole) 
	{
		load_dll(libConsole, #con_init, 0);
		con_init stdcall (-1, -1, -1, -1, "Math interpreter");
		initConsole = 0xFF;
	}
}

:dword getInteger()
{
	dword i = 0;
	byte z = 0;
	if (DSBYTE[code] == ' ') code++;
	if (DSBYTE[code] == '-') 
	{
		z = 0xFF;
		code++;
	}
	if (DSBYTE[code] >= '0') && (DSBYTE[code] <= '9')
	{
		while (DSBYTE[code] >= '0') && (DSBYTE[code] <= '9')
		{
			i *= 10;
			i += DSBYTE[code] - '0';
			code++;
		}
		
		if (z) return -i;
		return i;
	}
	return 0;
}

:dword mathEval(dword i)
{
	while (DSBYTE[code] == ' ') code++;
	code++;
	switch (DSBYTE[code-1])
	{
		case '+':
			return i + mathEval(getInteger());
		break;
		case '-':
			return i - mathEval(getInteger());
		break;
		case '/':
			return i / mathEval(getInteger());
		break;
		case '*':
			return i * mathEval(getInteger());
		break;
		case '(':
			return mathEval(mathEval(getInteger()));
		break;
		case ')':
			return i;
		break;
		case 0:
			return 0;
		break;
	}
	return i;
}

:dword evalMath()
{
	return mathEval(getInteger());
}

void main()
{
	dword brainFuckCode = 0;
	word maxLoop = 1000;

	buffer = malloc(bufferSize);
	memory = malloc(memoryBrainfuck);
	stack = malloc(stackBrainFuck);
	
	
	IF(DSBYTE[I_Param])
	{
		IF(io.read(I_Param))
		{
			code = EAX;
			evalMath();
		}
	}
	else 
	{
		consoleInit();
		con_printf stdcall ("Math interpreter v1.0");
		while(maxLoop)
		{
			con_printf stdcall ("\r\n\r\n: ");
			con_gets stdcall(buffer, bufferSize);
			code = EAX;
			//code = txt;
			con_printf stdcall ("Result: ");
			evalMath();
			con_printf stdcall (itoa(EAX));
			maxLoop--;
		}
	}
	
	IF(initConsole) con_exit stdcall (0);
	ExitProcess();
}

