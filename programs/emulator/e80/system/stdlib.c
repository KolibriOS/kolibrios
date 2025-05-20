
unsigned int seed_o = 0x45168297;


void srand (unsigned seed)
{
seed_o = seed;
}


int rand (void)
{
seed_o = seed_o * 0x15a4e35 + 1;
return(seed_o >> 16);
}


void* malloc(unsigned s)
{
asm ("int $0x40"::"a"(68), "b"(12), "c"(s) );
}


void free(void *p)
{
asm ("int $0x40"::"a"(68), "b"(13), "c"(p) );
}


void* realloc(void *p, unsigned s)
{
asm ("int $0x40"::"a"(68), "b"(12), "c"(p), "d"(s) );
}
