
int set_file_size(const char *path, unsigned size)
{
     int retval;
     int tmp;
     __asm__ __volatile__(
     "pushl $0 \n\t"
     "pushl $0 \n\t"
     "movl %2, 1(%%esp) \n\t"
     "pushl $0 \n\t"
     "pushl $0 \n\t"
     "pushl $0 \n\t"
     "pushl %%ebx \n\t"
     "push $4 \n\t"
     "movl %%esp, %%ebx \n\t"
     "movl $70, %%eax \n\t"
     "int $0x40 \n\t"
     "addl $28, %%esp \n\t"
     :"=a" (retval), "=b"(tmp)
     :"r" (path), "b" (size));
     return retval;
};
