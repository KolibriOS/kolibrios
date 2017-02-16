
#define __ALIGN_MASK(x,mask)  (((x)+(mask))&~(mask))
#define ALIGN(x,a)            __ALIGN_MASK(x,(typeof(x))(a)-1)

struct context
{
    int     x;
    int     y;
    int     width;
    int     height;
    rect_t  rc;
    rect_t  rcu;
    void   *buffer;
    size_t  pitch;
    size_t  size;
    int     dirty;
    void (*px_rect_simd)(void *dst_addr, int pitch, int w, int h, color_t dst_color);
    void (*px_glyph)(void *dst_addr, int dst_pitch, const void *src_addr, int src_pitch,
        int width, int height, color_t src_color);
};


void px_rect_alu(void *dst_addr, int pitch, int w, int h, color_t src_color);
void px_rect_mmx(void *dst_addr, int pitch, int w, int h, color_t src_color);
void px_rect_xmm(void *dst_addr, int pitch, int w, int h, color_t dst_color);
void px_glyph_alu(void *dst_addr, int dst_pitch,const void *src_addr,
                  int src_pitch, int width, int height, color_t src_color);

void px_glyph_sse(void *dst_addr, int dst_pitch, const void *src_addr,
                  int src_pitch, int width, int height, color_t src_color);

