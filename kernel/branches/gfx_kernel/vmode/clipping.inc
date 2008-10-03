align 4

jtable_intersect dd \
  calc_clipping_rects.is_0000,calc_clipping_rects.is_0001,calc_clipping_rects.is_0010,calc_clipping_rects.is_0011,\
  calc_clipping_rects.is_0100,calc_clipping_rects.is_0101,calc_clipping_rects.is_0110,calc_clipping_rects.is_0111,\
  calc_clipping_rects.is_1000,calc_clipping_rects.is_1001,calc_clipping_rects.is_1010,calc_clipping_rects.is_1011,\
  calc_clipping_rects.is_1100,calc_clipping_rects.is_1101,calc_clipping_rects.is_1110,calc_clipping_rects.is_1111

;jtable_fastclip dd \
; FC.x00,FC.x01,FC.x02,FC.xXX,FC.x04,FC.x05,FC.x06,FC.xXX,FC.x08,FC.x09,FC.x0A,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,\
; FC.x10,FC.x11,FC.x12,FC.xXX,FC.x14,FC.x15,FC.x16,FC.xXX,FC.x18,FC.x19,FC.x1A,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,\
; FC.x20,FC.x21,FC.x22,FC.xXX,FC.x24,FC.x25,FC.x26,FC.xXX,FC.x28,FC.x29,FC.x2A,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,\
; FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,\
; FC.x40,FC.x41,FC.x42,FC.xXX,FC.x44,FC.x45,FC.x46,FC.xXX,FC.x48,FC.x49,FC.x4A,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,\
; FC.x50,FC.x51,FC.x52,FC.xXX,FC.x54,FC.x55,FC.x56,FC.xXX,FC.x58,FC.x59,FC.x5A,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,\
; FC.x60,FC.x61,FC.x62,FC.xXX,FC.x64,FC.x65,FC.x66,FC.xXX,FC.x68,FC.x69,FC.x6A,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,\
; FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,\
; FC.x80,FC.x81,FC.x82,FC.xXX,FC.x84,FC.x85,FC.x86,FC.xXX,FC.x88,FC.x89,FC.x8A,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,\
; FC.x90,FC.x91,FC.x92,FC.xXX,FC.x94,FC.x95,FC.x96,FC.xXX,FC.x98,FC.x99,FC.x9A,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX,\
; FC.xA0,FC.xA1,FC.xA2,FC.xXX,FC.xA4,FC.xA5,FC.xA6,FC.xXX,FC.xA8,FC.xA9,FC.xAA,FC.xXX,FC.xXX,FC.xXX,FC.xXX,FC.xXX;,\
; FC.xB0,FC.xB1,FC.xB2,FC.xB3,FC.xB4,FC.xB5,FC.xB6,FC.xB7,FC.xB8,FC.xB9,FC.xBA,FC.xBB,FC.xBC,FC.xBD,FC.xBE,FC.xBF,\
; FC.xC0,FC.xC1,FC.xC2,FC.xC3,FC.xC4,FC.xC5,FC.xC6,FC.xC7,FC.xC8,FC.xC9,FC.xCA,FC.xCB,FC.xCC,FC.xCD,FC.xCE,FC.xCF,\
; FC.xD0,FC.xD1,FC.xD2,FC.xD3,FC.xD4,FC.xD5,FC.xD6,FC.xD7,FC.xD8,FC.xD9,FC.xDA,FC.xDB,FC.xDC,FC.xDD,FC.xDE,FC.xDF,\
; FC.xE0,FC.xE1,FC.xE2,FC.xE3,FC.xE4,FC.xE5,FC.xE6,FC.xE7,FC.xE8,FC.xE9,FC.xEA,FC.xEB,FC.xEC,FC.xED,FC.xEE,FC.xEF,\
; FC.xF0,FC.xF1,FC.xF2,FC.xF3,FC.xF4,FC.xF5,FC.xF6,FC.xF7,FC.xF8,FC.xF9,FC.xFA,FC.xFB,FC.xFC,FC.xFD,FC.xFE,FC.xFF
