struct __frame
{
 unsigned long ebp,edi,esi,edx,ecx,ebx,eax,eip;
};

void __libc_null_call(volatile struct __frame frame)
{
 __libclog_printf("Bad call occured from EIP=%x\n",frame.eip);
 __libclog_printf("EAX=%08x EBX=%08x ECX=%08x EDX=%08x\n",
     frame.eax,frame.ebx,frame.ecx,frame.edx);
 __libclog_printf("ESI=%08x EDI=%08x EBP=%08x\n",
     frame.esi,frame.edi,frame.ebp);
 _exit(1);
}
