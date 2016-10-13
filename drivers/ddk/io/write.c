
int write_file(const char *path,const void *buff,
               unsigned offset,unsigned count,unsigned *writes)
{
     int retval;
     unsigned cnt;
     __asm__ __volatile__(
     "pushl $0 \n\t"
     "pushl $0 \n\t"
     "movl %%eax, 1(%%esp) \n\t"
     "pushl %%ebx \n\t"
     "pushl %%edx \n\t"
     "pushl $0 \n\t"
     "pushl %%ecx \n\t"
     "pushl $3 \n\t"
     "movl %%esp, %%ebx \n\t"
     "mov $70, %%eax \n\t"
     "int $0x40 \n\t"
     "addl $28, %%esp \n\t"
     :"=a" (retval), "=b"(cnt)
     :"a"(path),"b"(buff),"c"(offset),"d"(count));
     if(writes)
        *writes = cnt;
    return retval;
};
