/*
 * Author Pavel Iakovlev
*/

#define MEMSIZE 4096*10

#include "../lib/io.h"
#include "../lib/obj/console.h"

#define bufferSize 10000;
#define memoryBrainfuck 30000
#define memoryByteBF 1
#define stackBrainFuck 4*1024

dword buffer = 0;
word bufferSymbol = 0;
dword memory = 0;
byte initConsole = 0;
dword stack = 0;

void consoleInit()
{
	IF(!initConsole) 
	{
		load_dll(libConsole, #con_init, 0);
		con_init stdcall (-1, -1, -1, -1, "BrainF*ck interpreter");
		initConsole = 0xFF;
	}
}

void evalBrainFuckCode(dword code)
{
	dword offsetMemory = 0;
	dword countStack = 0;
	dword countOffset = memoryBrainfuck / 2 / memoryByteBF;
	offsetMemory = memory;
	// clear memory
	EBX = memoryBrainfuck;
	offsetMemory = memory;
	WHILE(EBX)
	{
		EBX--;
		DSBYTE[offsetMemory] = 0;
		offsetMemory++;
	}
	//--------
	
	offsetMemory = memory + countOffset;
	countStack = stack;
	
	WHILE(1)
	{
		switch(DSBYTE[code])
		{
			case '+':
				DSBYTE[offsetMemory]++;
			break;
			case '-':
				DSBYTE[offsetMemory]--;
			break;
			case '[':
				DSDWORD[countStack] = code;
				countStack += 4;
			break;
			case ']':
				IF (DSBYTE[offsetMemory]) code = DSDWORD[countStack - 4];
				ELSE countStack -= 4;
			break;
			case ',':
				consoleInit();
				con_getch stdcall();
				DSBYTE[offsetMemory] = AL;
			break;
			case '.':
				consoleInit();
				DSBYTE[#bufferSymbol] = DSBYTE[offsetMemory];
				con_printf stdcall (#bufferSymbol);
			break;
			case '>':
				offsetMemory++;
			break;
			case '<':
				offsetMemory--;
			break;
			case 0:
				return;
			break;
		}
		code++;
	}
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
		IF(io.read(I_Param)) evalBrainFuckCode(EAX);
	}
	ELSE 
	{
		consoleInit();
		con_printf stdcall ("BrainF*ck interpreter v1.05");
		WHILE(maxLoop)
		{
			con_printf stdcall ("\r\n\r\nEnter code: ");
			con_gets stdcall(buffer, bufferSize);
			brainFuckCode = EAX;
			con_printf stdcall ("Output: ");
			evalBrainFuckCode(brainFuckCode);
			maxLoop--;
		}
	}
	
	IF(initConsole) con_exit stdcall (0);
	ExitProcess();
}

