#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// suballocator functions
extern void* wtmalloc(size_t size);
extern void  wtfree(void *pointer);
extern void* wtrealloc(void* pointer, size_t size);
extern void* wtcalloc (size_t num, size_t size);
extern int   wtmalloc_freelist_check();
extern int wtmalloc_poiner_check(void *ptr);
extern void wtdump_alloc_stats();

#ifdef __GNUC__
void* sysmalloc(size_t sz)
{
	return malloc(sz);
}
#endif



#define NUMPTR 10000

char *pointers[NUMPTR];
char values[NUMPTR];
int  sizes[NUMPTR];

int checkvalues()
{
	for (int i = 0; i < NUMPTR; i++)
	{
		if (!pointers[i]) continue;
		assert(wtmalloc_poiner_check(pointers[i]));
		
		for (int j = 0; j < sizes[i]; j++)
			assert(pointers[i][j] == values[i]);
	}
	return 1;
}


int main()
{
	char *ptr;
	int i, sz;
	
	puts("Test started");
	
	// test start settings
	assert(wtmalloc_freelist_check());
	// test just single alloc/dealloc
	ptr = wtmalloc(1000);
	assert(wtmalloc_poiner_check(ptr));
	wtfree(ptr);
	assert(wtmalloc_freelist_check());

	puts("test allocation started");
	// test allocation
	for (i = 0; i < NUMPTR; i++)
	{
		sz = rand() % 4200;
		pointers[i] = wtmalloc(sz);
		sizes[i] = sz;
		values[i] = sz % 256;
		memset(pointers[i], values[i], sz);

		assert(wtmalloc_freelist_check());
	}
	assert(checkvalues());

	puts("test random deallocation started");
	// random deallocation
	for (i = 0; i < NUMPTR; i++)
	{
		sz = rand() % 2;
		if (sz)
		{
			wtfree(pointers[i]);
			pointers[i] = NULL;
		}
	}
	assert(wtmalloc_freelist_check());
	assert(checkvalues());

	puts("test allocation in free list gaps started");
	// test allocation in free list gaps
	for (i = 0; i < NUMPTR; i++)
	{
		if (pointers[i]) continue;
		
		sz = rand() % 4200;
		pointers[i] = wtmalloc(sz);
		sizes[i] = sz;
		values[i] = sz % 256;
		memset(pointers[i], values[i], sz);
	}
	assert(wtmalloc_freelist_check());
	assert(checkvalues());
	
	puts("test realloc started");
	// test realloc
	for (i = 0; i < NUMPTR; i++)
	{
		sz = rand() % 4200;
		pointers[i] = wtrealloc(pointers[i], sz);
		
		sizes[i] = sz;
		memset(pointers[i], values[i], sz);
	}
	assert(wtmalloc_freelist_check());
	assert(checkvalues());
	
	
	puts("test full deallocation started");
	// full deallocation
	for (i = 0; i < NUMPTR; i++)
	{
		wtfree(pointers[i]);
		pointers[i] = NULL;
	}
	assert(wtmalloc_freelist_check());
	
	wtdump_alloc_stats();

	printf("\ntests all OK\n");
	
	return 0;

}
