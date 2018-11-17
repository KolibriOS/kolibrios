/*
 * Brainfuck Author Pavel Iakovlev
*/

#define MEMSIZE 4096*10

#include "../lib/obj/console.h"

#define bufferSize 10000;
#define memoryBrainfuck 30000
#define memoryByteBF 1
#define stackBrainFuck 4*1024

dword buffer = 0;
word bufferSymbol = 0;
dword memory = 0;
dword stack = 0;

void evalBrainFuckCode(dword code)
{
	byte cmd = 0;
	dword offsetMemory = 0;
	dword countStack = 0;
	dword countOffset = memoryBrainfuck / 2 / memoryByteBF;
	offsetMemory = memory;
	countStack = stack;
	
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
	
	con_printf stdcall ("Output BrainF*ck:\r\n");
	
	while(1)
	{
		cmd = DSBYTE[code];
		switch(cmd)
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
				con_getch stdcall();
				DSBYTE[offsetMemory] = AL;
			break;
			case '.':
				DSBYTE[#bufferSymbol] = DSBYTE[offsetMemory];
				con_printf stdcall (#bufferSymbol);
			break;
			case '>':
				offsetMemory++;
			break;
			case '<':
				offsetMemory--;
			break;
			default:
				con_printf stdcall ("\r\n");
				//ExitProcess();
				return;
			break;
		}
		code++;
	}
}

void main()
{
	dword brainFuckCode = 0;
	
	buffer = malloc(bufferSize);
	memory = malloc(memoryBrainfuck);
	stack = malloc(stackBrainFuck);
	
	load_dll(libConsole, #con_init, 0);
	con_init stdcall (-1, -1, -1, -1, "BrainF*ck interpreter");
	//con_set_flags stdcall (0x1F);
	con_printf stdcall ("BrainF*ck interpreter v1.0\r\n");
	loop()
	{
		con_printf stdcall ("\r\nEnter BrainF*ck code:\r\n");
		//con_write_string stdcall ("\r\n", 2);
		con_gets stdcall(buffer, bufferSize);
		//con_printf stdcall (EAX);
		evalBrainFuckCode(EAX);
	}
	con_exit stdcall (0);
	ExitProcess();
}

