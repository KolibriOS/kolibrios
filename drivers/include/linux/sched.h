/* stub */

/*
static inline void mdelay(u32_t time)
{
    time /= 10;
    if(!time) time = 1;

     __asm__ __volatile__ (
     "call *__imp__Delay"
     ::"b" (time));
     __asm__ __volatile__ (
     "":::"ebx");

};

static inline void udelay(u32_t delay)
{
    if(!delay) delay++;
    delay*= 500;

    while(delay--)
    {
        __asm__ __volatile__(
        "xorl %%eax, %%eax \n\t"
        "cpuid"
        :::"eax","ebx","ecx","edx" );
    }
}
*/
