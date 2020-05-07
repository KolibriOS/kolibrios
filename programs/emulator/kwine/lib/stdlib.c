

unsigned long int __rnd_next = 1;

int rand(void) // RAND_MAX assumed to be 32767
{
    __rnd_next = __rnd_next * 1103515245 + 12345;
    return (unsigned int)(__rnd_next/65536) % 32768;
}

void srand(unsigned int seed)
{
    __rnd_next = seed;
}

void *malloc(size_t size)
{
    void  *val;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(68),"b"(12),"c"(size));
    return val;
}

int free(void *mem)
{
    int  val;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(68),"b"(13),"c"(mem));
    return val;
}

void* realloc(void *mem, size_t size)
{
    void *val;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(68),"b"(20),"c"(size),"d"(mem)
    :"memory");

    return val;
};