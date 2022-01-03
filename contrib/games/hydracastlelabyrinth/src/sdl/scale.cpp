#include "scale.h"
#include "../xBRZ/xbrz.h"
#include <string.h>

void xbrz_scale(void* src, void* dst, int width, int height, int scale)
{
    if(scale==1) {
        memcpy(dst, src, width*height*4);
    } else if (scale>1 && scale <= 6) {
        xbrz::scale(scale, (const uint32_t*)src, (uint32_t*)dst, width, height, xbrz::ColorFormat::ARGB);
    } else {
        xbrz::nearestNeighborScale((const uint32_t*)src, width, height, (uint32_t*)dst, width*scale, height*scale);
    }
}