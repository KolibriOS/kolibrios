#include "defines.h"

// Enable scancodes for event loop
inline void enable_scancode() {
    __asm__ __volatile__("int $0x40"::"a"(66),"b"(1),"c"(1));
}

// Clear key buffer
inline void clear_key_buffer() {
    int i = 0;
    for (i = 0; i < 120; i++)
        __menuet__getkey();
}

// Wait for screen draw (vertical sync)
inline void vsync() {
    __asm__ __volatile__("int $0x40"::"a"(18),"b"(14));
}

// Get text length in pixels
inline __u32 text_length_px(__u32 len)
{
    return len * FONT_WIDTH + len;
}
