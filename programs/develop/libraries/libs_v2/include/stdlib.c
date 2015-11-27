#ifndef STDLIB_C_INCLUDE
#define STDLIB_C_INCLUDE

#define RAND_MAX 0x7FFFU

#define isspace(c) ((c)==' ')
#define abs(i) (((i)<0)?(-(i)):(i))

#define random(num) ((rand()*(num))/((RAND_MAX+1)))

static unsigned int seed_o = 0x45168297;


static inline void srand (unsigned seed)
{
	seed_o = seed;
}


static inline int rand (void)
{
	seed_o = seed_o * 0x15a4e35 + 1;
	return(seed_o >> 16);
}


static inline void* malloc(unsigned s)
{
	asm ("int $0x40"::"a"(68), "b"(12), "c"(s) );
}


static inline void free(void *p)
{
	asm ("int $0x40"::"a"(68), "b"(13), "c"(p) );
}

static inline void *realloc(void *data, long size)
{
	void *r = malloc(size);
	byte *p = (byte *)r;
	byte *pd = (byte *)data;
	while(size--) *p++=*pd++;
	free(data);
	return r;
}


#endif