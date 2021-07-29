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
    return (__curtime-__starttime);
}

void uSDL_Delay(unsigned time){
    __asm__ __volatile__(
        "int $0x40"
        ::"a"(5), "b"(time/3)
        :"memory"
    );
} 
