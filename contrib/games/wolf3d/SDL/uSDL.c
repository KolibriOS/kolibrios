static unsigned __starttime;

void uSDL_StartTicks(void){
	__asm__ __volatile__ (
        "int $0x40" 
        :"=a"(__starttime) 
        :"a"(26),"b"(9)
        :"memory"
    );
}

unsigned uSDL_GetTicks(void){
    unsigned __curtime;
    __asm__ __volatile__(
        "int $0x40" 
        :"=a"(__curtime) 
        :"a"(26),"b"(9)
        :"memory"
    );
    return (__curtime-__starttime)*10;
}

void uSDL_Delay(unsigned ms){
  unsigned start = uSDL_GetTicks();
  do{
    __asm__("int $0x40" :: "a"(68),"b"(1));
  }while (uSDL_GetTicks()-start < ms);
}
