#include "graphics.h"
#include "sprites.h"


// 8bpp back buffer: bytes are DINO_PALETTE indices, sysfn 65 applies the palette
static unsigned char screenImage[DEFAULT_WIDTH * DEFAULT_HEIGHT];


void graphicsBlitAtlasImage(int atlasX, int atlasY, int destX, int destY, int w, int h) {
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
    unsigned char* dst = screenImage + destY * DEFAULT_WIDTH + destX;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            unsigned char idx = src[x];
            if (idx)
                dst[x] = idx;
        }
        src += ATLAS_WIDTH;
        dst += DEFAULT_WIDTH;
    }
}

void graphicsFillBackground(void) {
    unsigned char* p = screenImage;
    for (int i = DEFAULT_WIDTH * DEFAULT_HEIGHT; i; --i)
        *p++ = BACKGROUND_INDEX;
}

void graphicsRender() {
    // don't redraw window on each frame. redraw window only when redraw event (called when widow moved e.g.)
    ksys_draw_bitmap_palette(screenImage, 5, 24, DEFAULT_WIDTH, DEFAULT_HEIGHT, 8, (void*)DINO_PALETTE, 0);
}

void graphicsDelay(int ms) {
    // sysfn 5 sleeps in hundredths of a second; round up, never oversleep by 20 ms
    _ksys_delay((ms + 9) / 10);
}
