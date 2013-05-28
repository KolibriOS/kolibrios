
struct  pixel_format
        bpp             db ?
        depth           db ?
        big_endian      db ?
        true_color      db ?
        red_max         dw ?
        green_max       dw ?
        blue_max        dw ?
        red_shift       db ?
        green_shift     db ?
        blue_shift      db ?
        padding         rb 3
ends

struct  framebuffer
        width           dw ?
        height          dw ?
        pixelformat     pixel_format
        name_length     dd ?
        name            rb 256
ends