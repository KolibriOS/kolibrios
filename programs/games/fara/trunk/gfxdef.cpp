#include "kosSyst.h"
#include "KosFile.h"
#include "gfxdef.h"
#include "crc32.h"

extern "C" void __stdcall lzma_decompress(
	 const void* source,
	 void* destination,
	 unsigned dest_length);

struct export_item
{
	const char* name;
	const void* func;
};

typedef void* (__stdcall *img_decode_t)(const void* data, unsigned len, void* parameters);
typedef void (__stdcall *img_to_rgb2_t)(const void* image, void* destination);
typedef void (__stdcall *img_destroy_t)(void* image);
typedef void (__stdcall *img_lib_init_t)(void);	// really fastcall with 4 args, but called from asm code

img_lib_init_t img_lib_init = NULL;
img_decode_t img_decode = NULL;
img_to_rgb2_t img_to_rgb2 = NULL;
img_destroy_t img_destroy = NULL;

export_item* libini_exports = NULL;
static const char libini_name[] = "/sys/lib/libimg.obj";

extern "C" int strcmp(const char* str1, const char* str2);
#pragma intrinsic(strcmp)

void jpeg_decompress(
	 const void* source,
	 unsigned source_length,
	 void* destination,
	 unsigned dest_length)
{
	if (!libini_exports)
	{
		__asm
		{
			mov	eax, 68
			mov	ebx, 19
			mov	ecx, offset libini_name
			int	40h
			mov	[libini_exports], eax
		}
		if (!libini_exports)
		{
			rtlDebugOutString("Cannot load libimg.obj");
			kos_ExitApp();
		}
		for (export_item* p = libini_exports; p->name; p++)
		{
			if (!strcmp(p->name,"lib_init"))
				img_lib_init = (img_lib_init_t)p->func;
			else if (!strcmp(p->name,"img_decode"))
				img_decode = (img_decode_t)p->func;
			else if (!strcmp(p->name,"img_to_rgb2"))
				img_to_rgb2 = (img_to_rgb2_t)p->func;
			else if (!strcmp(p->name,"img_destroy"))
				img_destroy = (img_destroy_t)p->func;
		}
		if (!img_lib_init || !img_decode || !img_to_rgb2 || !img_destroy)
		{
			rtlDebugOutString("Required exports were not found in libimg.obj");
			kos_ExitApp();
		}
		__asm
		{
			mov	eax, offset kos_malloc
			mov	ebx, offset kos_free
			mov	ecx, offset kos_realloc
			call	img_lib_init
		}
	}
	void* image = img_decode(source, source_length, NULL);
	if (!image)
	{
		rtlDebugOutString("JPEG error");
		kos_ExitApp();
	}
	img_to_rgb2(image, destination);
	img_destroy(image);
}

//////  CKosBitmap

CKosBitmap::CKosBitmap()
{
	this->bmpID = -1;
	this->buffer = NULL;
	this->sizeX = 0;
	this->sizeY = 0;
}


CKosBitmap::~CKosBitmap()
{
	if ( this->buffer != NULL ) delete this->buffer;
}


// загрузка из сжатого файла
bool CKosBitmap::LoadFromArch( SCompBmpHeader *bmpArchDesc, CKosFile *fromFile, int ID )
{
	Byte *tmpBuff;
	
	//
	if ( this->buffer != NULL )
	{
		delete this->buffer;
		this->buffer = NULL;
	}
	//
	this->buffer = new RGB[bmpArchDesc->sizeX * bmpArchDesc->sizeY];
	//
	tmpBuff = new Byte[bmpArchDesc->compressedSize];
	//
	fromFile->Seek( SEEK_SET, bmpArchDesc->physicalOffset );
	if ( fromFile->Read( tmpBuff, bmpArchDesc->compressedSize ) == bmpArchDesc->compressedSize )
	{
		//
		if ( bmpArchDesc->compressedSize == bmpArchDesc->uncompressedSize+1)
		{
			// JPEG image
			jpeg_decompress( tmpBuff, bmpArchDesc->compressedSize,
				this->buffer, bmpArchDesc->sizeX * bmpArchDesc->sizeY * 3);
		}
		else if ( bmpArchDesc->compressedSize != bmpArchDesc->uncompressedSize )
		{
			// LZMA-packed BMP
			lzma_decompress( tmpBuff, this->buffer, bmpArchDesc->uncompressedSize);
		}
		else
		{
			//
			memcpy( (Byte *)(this->buffer), tmpBuff, bmpArchDesc->compressedSize );
		}
		//
		this->sizeX = bmpArchDesc->sizeX;
		this->sizeY = bmpArchDesc->sizeY;
		this->bmpID = ID;
	}
	//
	delete tmpBuff;
	//
	return true;
}


// вывести в окно картинку
void CKosBitmap::Draw( Word x, Word y )
{
	//
	if ( this->buffer != NULL )
		//
		kos_PutImage( this->buffer, this->sizeX, this->sizeY, x, y );
}


// получить указатель на область данных
RGB * CKosBitmap::GetBits()
{
	return this->buffer;
}


// получить размер картинки
void CKosBitmap::GetSize( Word &cx, Word &cy )
{
	cx = this->sizeX;
	cy = this->sizeY;
}

// создать картинку из большей
void CKosBitmap::Scale(Word size, RGB *mainBits)
{
	buffer = new RGB[(sizeX=blockSize)*(sizeY=blockSize*11)];
	memset((Byte*)buffer,0,3*blockSize*blockSize*11);
	RGB* tmpBuf = new RGB[blockSize*size];
	for (int k=0;k<11;k++)
	{
		int delta = (blockSize - size)/2;
		int i,j;
		int a;
		int d1 = blockSize/size;
		int d2 = (blockSize-d1*(size))*256/size;
		// сглаживание по горизонтали
		RGB* ptrBuf = tmpBuf;
		for (j=0;j<blockSize;j++)
		{
			RGB* srcBits = mainBits + blockSize*blockSize*(k+1) + blockSize*j;
			a = 0;
			for (i=0;i<size;i++)
			{
				ptrBuf->b = srcBits->b + (srcBits[1].b-srcBits[0].b)*a/256;
				ptrBuf->g = srcBits->g + (srcBits[1].g-srcBits[0].g)*a/256;
				ptrBuf->r = srcBits->r + (srcBits[1].r-srcBits[0].r)*a/256;
				ptrBuf++;
				srcBits += d1;
				a += d2;
				if (a >= 256)
				{
					a -= 256;
					srcBits++;
				}
			}
		}
		// сглаживание по вертикали
		for (j=0;j<size;j++)
		{
			ptrBuf = buffer + blockSize*blockSize*k + blockSize*delta + delta+j;
			RGB* srcBits = tmpBuf + j;
			a = 0;
			for (i=0;i<size;i++)
			{
				ptrBuf->b = srcBits->b + (srcBits[size].b-srcBits[0].b)*a/256;
				ptrBuf->g = srcBits->g + (srcBits[size].g-srcBits[0].g)*a/256;
				ptrBuf->r = srcBits->r + (srcBits[size].r-srcBits[0].r)*a/256;
				ptrBuf += blockSize;
				srcBits += d1*size;
				a += d2;
				if (a >= 256)
				{
					a -= 256;
					srcBits += size;
				}
			}
		}
	}
	delete tmpBuf;
}


////////////////////// CFishka ///////////////////////

CFishka::CFishka( CKosBitmap *fromBmp, int yOffset, RGB insColour )
{
	int i, c;

	//
	this->bits = fromBmp->GetBits() + (yOffset * blockSize);
	this->transColour = insColour;
	//
	this->highLighted = new RGB[blockSize * blockSize];
	//
	for ( i = 0; i < (blockSize * blockSize); i++ )
	{
		//
		this->highLighted[i] = this->bits[i];
		//
		if ( this->highLighted[i] != this->transColour )
		{
			c = ( this->highLighted[i].b * 185 ) / 100;
			this->highLighted[i].b = (c > 255) ? 255 : c;
			c = ( this->highLighted[i].g * 185 ) / 100;
			this->highLighted[i].g = (c > 255) ? 255 : c;
			c = ( this->highLighted[i].r * 185 ) / 100;
			this->highLighted[i].r = (c > 255) ? 255 : c;
		}
	}
}

//
CFishka::~CFishka()
{
	//
	delete this->highLighted;
}


//
RGB * CFishka::GetBits()
{
	return this->bits;
}

//
RGB * CFishka::GetHighlightedBits()
{
	return this->highLighted;
}

