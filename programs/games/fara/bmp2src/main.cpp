#include <stdio.h>
#include <stdlib.h>

extern "C" __stdcall lzma_set_dict_size(unsigned logdictsize);
extern "C" __stdcall lzma_compress(
	const void* source,
	void* destination,
	unsigned length,
	void* workmem);

typedef struct
{
	short int sizeX;
	short int sizeY;
	int compressedSize;
	int physicalOffset;
	int uncompressedSize;
} SCompBmpHeader;

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		printf("Usage: bmp2src <destination> <src1> <src2> ...\n");
		return 1;
	}
	FILE* fo = fopen(argv[1], "wb");
	if (!fo)
	{
		printf("Cannot create destination file\n");
		return 2;
	}
	int n=0;
	SCompBmpHeader* hea = (SCompBmpHeader*)malloc((argc-2)*sizeof(SCompBmpHeader));
	void** ptrs = (void**)malloc((argc-2)*4);
	lzma_set_dict_size(20);
	void* workmem = (void*)malloc(0x509000 + (1<<20)*19/2);
	for (int i=2;i<argc;i++)
	{
		FILE* fi = fopen(argv[i], "rb");
		if (!fi)
		{
			printf("Cannot open input file %s\n",argv[i]);
			continue;
		}
		unsigned char buf[0x36];
		fread(buf,1,0x36,fi);
		if (buf[0] != 'B' || buf[1] != 'M')
		{
			if (buf[0] != 0xFF || buf[1] != 0xD8)
			{
				printf("%s: unrecognized type\n",argv[i]);
				fclose(fi);
				continue;
			}
			// JPEG
			printf("Processing %s ...",argv[i]);
			fseek(fi,0,SEEK_END);
			unsigned insize = ftell(fi);
			void* output = malloc(insize);
			unsigned char* ptr = (unsigned char*)output;
			*ptr++ = 0xFF;
			*ptr++ = 0xD8;
			// Now load JPEG file, skipping all APPx markers
			fseek(fi,2,SEEK_SET);
			bool bOk = false;
			for (;;)
			{
				if (fread(buf,1,4,fi) != 4 || buf[0] != 0xFF)
				{
					printf("%s: invalid JPEG file\n",argv[i]);
					bOk = false;
					break;
				}
				// ignore APPx markers
				if (buf[1] >= 0xE0 && buf[1] <= 0xEF)
				{
					fseek(fi,buf[2]*256 + buf[3] - 2,SEEK_CUR);
					continue;
				}
				unsigned len = buf[2]*256 + buf[3] + 2;
				fseek(fi,-4,SEEK_CUR);
				fread(ptr,1,len,fi);
				if (buf[1]>=0xC0 && buf[1]<=0xCF && buf[1]!=0xC4 && buf[1]!=0xC8 && buf[1]!=0xCC)
				{
					// found SOFn marker
					hea[i-2].sizeX = (unsigned char)ptr[4+3]*256 + (unsigned char)ptr[4+4];
					hea[i-2].sizeY = (unsigned char)ptr[4+1]*256 + (unsigned char)ptr[4+2];
					bOk = true;
				}
				ptr += len;
				if (buf[1] == 0xDA)
				{
					// SOS marker
					len = insize - ftell(fi);
					fread(ptr,1,len,fi);
					ptr += len;
					break;
				}
			}
			if (!bOk) {printf(" invalid\n");free(ptr);continue;}
			hea[i-2].compressedSize = ptr - (unsigned char*)output;
			hea[i-2].uncompressedSize = hea[i-2].compressedSize - 1;
			hea[i-2].physicalOffset = (i==2) ? 0 :
				hea[i-3].physicalOffset+hea[i-3].compressedSize;
			ptrs[i-2] = output;
			++n;
			printf(" OK\n");
			continue;
		}
		if (buf[0x1C] != 24)
		{
			printf("Input file %s is not 24-bit BMP\n",argv[i]);
			fclose(fi);
			continue;
		}
		int width = *(int*)(buf+0x12);
		int linesize = (width*3+3)&~3;
		int height = *(int*)(buf+0x16);
		void* input = malloc(width*height*3);
		void* packed = malloc(9*width*height*3/8 + 0x80);
		for (int p=0;p<height;p++)
		{
			fseek(fi,(height-p-1)*linesize+0x36,SEEK_SET);
			fread((char*)input+p*width*3, 1, width*3, fi);
		}
		fclose(fi);
		hea[i-2].sizeX = (short)width;
		hea[i-2].sizeY = (short)height;
		unsigned uncompressedSize = width*height*3;
		hea[i-2].uncompressedSize = uncompressedSize;
		hea[i-2].physicalOffset = (i==2) ? 0 :
			hea[i-3].physicalOffset+hea[i-3].compressedSize;
		printf("Compressing %s ...",argv[i]);
		unsigned compressedSize = lzma_compress(input,packed,
			uncompressedSize,workmem);
		if (compressedSize >= uncompressedSize)
		{
			compressedSize = uncompressedSize;
			free(packed);
			ptrs[i-2] = input;
		}
		else
		{
			ptrs[i-2] = packed;
			free(input);
		}
		printf(" %d -> %d\n",uncompressedSize, compressedSize);
		hea[i-2].compressedSize = compressedSize;
		++n;
	}
	for (i=0;i<n;i++)
		hea[i].physicalOffset += 4+n*sizeof(SCompBmpHeader);
	fwrite(&n,4,1,fo);
	fwrite(hea,sizeof(SCompBmpHeader),n,fo);
	for (i=0;i<n;i++)
	{
		fwrite(ptrs[i],1,hea[i].compressedSize,fo);
		free(ptrs[i]);
	}
	fclose(fo);
	free(hea);
	free(workmem);
	free(ptrs);
	return 0;
}
