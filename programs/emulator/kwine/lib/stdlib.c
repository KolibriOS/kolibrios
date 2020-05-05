
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