#ifndef __cplusplus

//"inline" функции для  вызова системных функций Kolibri в C - в имени функции кол-во параметров
//SysCall# (номер_системной_функции, параметры,...)

static inline int SysCall1 (int EAX__) __attribute__((always_inline));
static inline int SysCall2 (int EAX__, int EBX__) __attribute__((always_inline));
static inline int SysCall3 (int EAX__, int EBX__, int ECX__) __attribute__((always_inline));
static inline int SysCall4 (int EAX__, int EBX__, int ECX__, int EDX__) __attribute__((always_inline));
static inline int SysCall5 (int EAX__, int EBX__, int ECX__, int EDX__, int ESI__) __attribute__((always_inline));
static inline int SysCall6 (int EAX__, int EBX__, int ECX__, int EDX__, int ESI__, int EDI__) __attribute__((always_inline));


static inline int SysCall1 (int EAX__)
{
 	   asm volatile(""::"a"(EAX__));
 	   asm volatile("int $0x40");
 	   
 	   register int res;
 	   asm volatile("":"=a"(res):);
 	   return res; 	   
}


static inline int SysCall2 (int EAX__, int EBX__)
{
 	   asm volatile(""::"a"(EAX__));
 	   asm volatile(""::"b"(EBX__)); 	   
 	   asm volatile("int $0x40");
 	   
 	   register int res;
 	   asm volatile("":"=a"(res):);
 	   return res;
}


static inline int SysCall3 (int EAX__, int EBX__, int ECX__)
{
 	   asm volatile(""::"a"(EAX__));
 	   asm volatile(""::"b"(EBX__)); 	   
 	   asm volatile(""::"c"(ECX__));
 	   asm volatile("int $0x40");
 	   
 	   register int res;
 	   asm volatile("":"=a"(res):);
 	   return res;
}

 
static inline int SysCall4 (int EAX__, int EBX__, int ECX__, int EDX__)
{
 	   asm volatile(""::"a"(EAX__));
 	   asm volatile(""::"b"(EBX__)); 	   
 	   asm volatile(""::"c"(ECX__));
 	   asm volatile(""::"d"(EDX__));
 	   asm volatile("int $0x40");
 	   
 	   register int res;
 	   asm volatile("":"=a"(res):);
 	   return res;
}


static inline int SysCall5 (int EAX__, int EBX__, int ECX__, int EDX__, int ESI__)
{
 	   asm volatile(""::"a"(EAX__));
 	   asm volatile(""::"b"(EBX__)); 	   
 	   asm volatile(""::"c"(ECX__));
 	   asm volatile(""::"d"(EDX__));
 	   asm volatile(""::"S"(ESI__));
 	   asm volatile("int $0x40");
 	   
 	   register int res;
 	   asm volatile("":"=a"(res):);
 	   return res;
}


static inline int SysCall6 (int EAX__, int EBX__, int ECX__, int EDX__, int ESI__, int EDI__)
{
 	   asm volatile(""::"a"(EAX__));
 	   asm volatile(""::"b"(EBX__)); 	   
 	   asm volatile(""::"c"(ECX__));
 	   asm volatile(""::"d"(EDX__));
 	   asm volatile(""::"S"(ESI__));
 	   asm volatile(""::"D"(EDI__));
 	   asm volatile("int $0x40");
 	   
 	   register int res;
 	   asm volatile("":"=a"(res):);
 	   return res;
}

#else

//"inline" функции для  вызова системных функций Kolibri в C++ 
//SysCall(номер_системной_функции, параметры,...)

static inline int SysCall (int EAX__) __attribute__((always_inline));
static inline int SysCall (int EAX__, int EBX__) __attribute__((always_inline));
static inline int SysCall (int EAX__, int EBX__, int ECX__) __attribute__((always_inline));
static inline int SysCall (int EAX__, int EBX__, int ECX__, int EDX__) __attribute__((always_inline));
static inline int SysCall (int EAX__, int EBX__, int ECX__, int EDX__, int ESI__) __attribute__((always_inline));
static inline int SysCall (int EAX__, int EBX__, int ECX__, int EDX__, int ESI__, int EDI__) __attribute__((always_inline));



static inline int SysCall (int EAX__)
{
 	   asm volatile(""::"a"(EAX__));
 	   asm volatile("int $0x40");
 	   
 	   register int res;
 	   asm volatile("":"=a"(res):);
 	   return res;
}


static inline int SysCall (int EAX__, int EBX__)
{
 	   asm volatile(""::"a"(EAX__));
 	   asm volatile(""::"b"(EBX__)); 	   
 	   asm volatile("int $0x40");
 	   
 	   register int res;
 	   asm volatile("":"=a"(res):);
 	   return res;
}


static inline int SysCall (int EAX__, int EBX__, int ECX__)
{
 	   asm volatile(""::"a"(EAX__));
 	   asm volatile(""::"b"(EBX__)); 	   
 	   asm volatile(""::"c"(ECX__));
 	   asm volatile("int $0x40");
 	   
 	   register int res;
 	   asm volatile("":"=a"(res):);
 	   return res;
}

 
static inline int SysCall (int EAX__, int EBX__, int ECX__, int EDX__)
{
 	   asm volatile(""::"a"(EAX__));
 	   asm volatile(""::"b"(EBX__)); 	   
 	   asm volatile(""::"c"(ECX__));
 	   asm volatile(""::"d"(EDX__));
 	   asm volatile("int $0x40");
 	   
 	   register int res;
 	   asm volatile("":"=a"(res):);
 	   return res;
}


static inline int SysCall (int EAX__, int EBX__, int ECX__, int EDX__, int ESI__)
{
 	   asm volatile(""::"a"(EAX__));
 	   asm volatile(""::"b"(EBX__)); 	   
 	   asm volatile(""::"c"(ECX__));
 	   asm volatile(""::"d"(EDX__));
 	   asm volatile(""::"S"(ESI__));
 	   asm volatile("int $0x40");
 	   
 	   register int res;
 	   asm volatile("":"=a"(res):);
 	   return res;
}


static inline int SysCall (int EAX__, int EBX__, int ECX__, int EDX__, int ESI__, int EDI__)
{
 	   asm volatile(""::"a"(EAX__));
 	   asm volatile(""::"b"(EBX__)); 	   
 	   asm volatile(""::"c"(ECX__));
 	   asm volatile(""::"d"(EDX__));
 	   asm volatile(""::"S"(ESI__));
 	   asm volatile(""::"D"(EDI__));
 	   asm volatile("int $0x40");
 	   
 	   register int res;
 	   asm volatile("":"=a"(res):);
 	   return res;
}

#endif
