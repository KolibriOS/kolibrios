// pe2kos.cpp : Defines the entry point for the console application.
//

//#define TARGET_USES_PATH

#include <string.h>
#include <stdio.h>
#include <io.h>

typedef unsigned __int32 DWORD;
typedef unsigned __int16 WORD;
typedef unsigned __int8 BYTE;

#pragma pack(1)

struct kos_Header
{
	char kosSign[8];
	DWORD res1;
	DWORD start;
	DWORD end;
	DWORD memory;
	DWORD stack;
	DWORD res2;
	DWORD res3;
	//
	kos_Header()
	{
		memcpy( kosSign, "MENUET01", 8 );
		res1 = 1;
		res2 = 0;
		res3 = 0;
	}
};


void load_section( BYTE *pePtr, BYTE *secDescPtr, FILE *fp )
{
	DWORD *dwPtr = (DWORD *)secDescPtr;
	DWORD virtualSize;

	//
	virtualSize = ( dwPtr[2] >= dwPtr[4] ) ? dwPtr[2] : dwPtr[4];
	//
	fseek( fp, dwPtr[3], SEEK_SET );
	fwrite( pePtr + dwPtr[5], virtualSize, 1, fp );
}


int main(int argc, char* argv[])
{
	FILE *pe, *kos;
	kos_Header hdr;
	BYTE *peBuff;
	BYTE *scanPtr;
	long peSize;
	DWORD baseNdx;
	WORD sCount, sLim;

	//
	if ( argc != 3 ) return 0;

	//
	pe = fopen( argv[1], "rb" );
	if ( pe == NULL ) return 0;
	//
	kos = fopen( argv[2], "w+b" );
	if ( kos == NULL )
	{
		fclose( pe );
		return 0;
	}
	//
	fseek( pe, 0, SEEK_END );
	peSize = ftell( pe );
	fseek( pe, 0, SEEK_SET );
	//
	peBuff = new BYTE[peSize];
	fread( peBuff, peSize, 1, pe );
	fclose( pe );
	//
	//
	scanPtr = peBuff + ((DWORD *)(peBuff + 0x3C))[0];
	//
	baseNdx = 0xF8;
	//
	sLim = ((WORD *)(scanPtr + 6))[0];
	//
	for ( sCount = 0; sCount < sLim; sCount++ )
	{
		//
		switch ( scanPtr[baseNdx + 0x24] )
		{
		//
		case 0x20:
		case 0x40:
			load_section( peBuff, scanPtr + baseNdx, kos );
			break;
		//
		default:
			break;
		}
		//
		baseNdx += 0x28;

	}
	//
	fseek( kos, 0, SEEK_END );
	fflush(kos);
	peSize = ftell( kos );
	for (;;)
	{
		fseek(kos, -1, SEEK_CUR);
		if (fgetc(kos))
			break;
		fseek(kos, -1, SEEK_CUR);
	}
	_chsize(_fileno(kos), ftell(kos));
	//
	hdr.start = ((DWORD *)(scanPtr + 0x28))[0];
	hdr.end = ftell(kos);
	hdr.stack = peSize + 4096;
#ifdef TARGET_USES_PATH
	hdr.res3 = hdr.stack;
	hdr.memory = hdr.res3 + 2048;
#else
	hdr.memory = hdr.stack;
#endif
	//
	fseek( kos, 0, SEEK_SET );
	fwrite( &hdr, sizeof(hdr), 1, kos );

	//
	delete [] peBuff;
	//
	fclose( kos );

	return 0;
}
