#include <stdio.h>
#include <windows.h>
extern "C" __stdcall lzma_set_dict_size(unsigned logdictsize);
extern "C" __stdcall lzma_compress(
	const void* source,
	void* destination,
	unsigned length,
	void* workmem);

int main()
{
	FILE* f;
	f = fopen("test.in","rb");
	fseek(f,0,SEEK_END);
	unsigned inlen = ftell(f);
	fseek(f,0,SEEK_SET);
	void* in = VirtualAlloc(NULL,inlen,MEM_COMMIT,PAGE_READWRITE);
	void* out = VirtualAlloc(NULL,inlen,MEM_COMMIT,PAGE_READWRITE);
	fread(in,1,inlen,f);
	fclose(f);
	unsigned logdictsize,dictsize;
	for (logdictsize=0,dictsize=1;dictsize<inlen && logdictsize<=28;logdictsize++,dictsize<<=1) ;
	lzma_set_dict_size(logdictsize);
	void* work = VirtualAlloc(NULL,dictsize*19/2+0x509000,MEM_COMMIT,PAGE_READWRITE);
	unsigned outlen = lzma_compress(in,out,inlen,work);
	printf("%d -> %d\n",inlen,outlen);
	f = fopen("test.out","wb");
	fwrite(out,1,outlen,f);
	fclose(f);
	return 0;
}
