#include "defines.h"

inline void enable_scancode() {
    __asm__ __volatile__("int $0x40"::"a"(66),"b"(1),"c"(1));
}

inline void clear_key_buffer() {
    int i = 0;
    for (i = 0; i < 120; i++)
        __menuet__getkey();
}

inline void vsync() {
    __asm__ __volatile__("int $0x40"::"a"(18),"b"(14));
}
