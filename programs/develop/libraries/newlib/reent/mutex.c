void __mutex_lock(volatile int *val)
{
    int tmp;

    __asm__ __volatile__ (
"0:\n\t"
    "mov %0, %1\n\t"
    "testl %1, %1\n\t"
    "jz 1f\n\t"

    "movl $68, %%eax\n\t"
    "movl $1,  %%ebx\n\t"
    "int  $0x40\n\t"
    "jmp 0b\n\t"
"1:\n\t"
    "incl %1\n\t"
    "xchgl %0, %1\n\t"
    "testl %1, %1\n\t"
    "jnz 0b\n"
    : "+m" (*val), "=&r"(tmp)
    ::"eax","ebx" );
}

