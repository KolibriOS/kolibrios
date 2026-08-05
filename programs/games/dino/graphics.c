#include "graphics.h"
#include "sprites.h"


static unsigned screenImage[DEFAULT_WIDTH * DEFAULT_HEIGHT];


void graphicsInit() {
}

void graphicsBlitAtlasImage(int atlasX, int atlasY, int destX, int destY, int w, int h, bool center) {
    if (destX >= DEFAULT_WIDTH) {
        return;
    }
    if (destY >= DEFAULT_HEIGHT) {
        return;
    }

    if (destX < 0) {
        atlasX -= destX;
        w = destX + w;
        destX = 0;
    }
    if (destX + w > DEFAULT_WIDTH) {
        w = DEFAULT_WIDTH - destX;
    }

    if (destY < 0) {
        atlasY -= destY;
        h = destY + h;
        destY = 0;
    }
    if (destY + h > DEFAULT_HEIGHT) {
        h = DEFAULT_HEIGHT - destY;
    }

    if (w <= 0 || h <= 0) {
        return;
    }

    const unsigned char* src = SPRITE_ATLAS + atlasY * ATLAS_WIDTH + atlasX;
    unsigned* dst = screenImage + destY * DEFAULT_WIDTH + destX;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            unsigned char idx = src[x];
            if (idx)
                dst[x] = DINO_PALETTE[idx];
        }
        src += ATLAS_WIDTH;
        dst += DEFAULT_WIDTH;
    }
}

void graphicsFillBackground(unsigned r, unsigned g, unsigned b) {
    unsigned color = (r << 16) | (g << 8) | b;
    unsigned* p = screenImage;
    for (int i = DEFAULT_WIDTH * DEFAULT_HEIGHT; i; --i)
        *p++ = color;
}

void graphicsRender() {
    // don't redraw window on each frame. redraw window only when redraw event (called when widow moved e.g.)
    ksys_draw_bitmap_palette(screenImage, 5, 24, DEFAULT_WIDTH, DEFAULT_HEIGHT, 32, 0, 0);
}

void graphicsDelay(int ms) {
    // dbg_printf("ms = %d\n", ms);
    _ksys_delay(ms/10 ? ms/10 : 2);
}


void graphicsDestroy() {
}
