#include<menuet/os.h>

void __menuet__sound_load_block(char * blockptr)
{
 __asm__ __volatile__("int $0x40"::"a"(55),"b"(0),"c"(blockptr));
}

void __menuet__sound_play_block(void)
{
 __asm__ __volatile__("int $0x40"::"a"(55),"b"(1));
}
