#include <SDL.h>
#include <stdlib.h>
#include <string.h>

#define asm_inline __asm__ __volatile__

#pragma pack(push,1)
typedef union{
    unsigned val;
    struct{
        short  h;
        short  w;
    };
}ksys_screen_t;
#pragma pack(pop)


static inline
void _ksys_change_window(int new_x, int new_y, int new_w, int new_h)
{
    asm_inline(
        "int $0x40"
        ::"a"(67), "b"(new_x), "c"(new_y), "d"(new_w),"S"(new_h)
    );
}

static inline
ksys_screen_t _ksys_screen_size()
{
	ksys_screen_t size;
    asm_inline(
        "int $0x40"
        :"=a"(size)
        :"a"(14)
        :"memory"
    );
    return size;
}

void uSDL_SetWinCenter(unsigned w, unsigned h){
    ksys_screen_t screen_size= _ksys_screen_size();
    int new_x = screen_size.w/2-w/2;
    int new_y = screen_size.h/2-h/2;
    _ksys_change_window(new_x, new_y, -1, -1); 
}


void uSDL_Delay(unsigned ms){
    unsigned start = SDL_GetTicks();
    do{
       asm_inline("int $0x40" :: "a"(5),"b"(1));
    }while (SDL_GetTicks()-start < ms);
}