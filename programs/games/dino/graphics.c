#include "graphics.h"
#include "sprites.h"


// 32bpp: sysfn 65 copies it as is, indices would cost a kernel lookup per pixel
static unsigned screenImage[DEFAULT_WIDTH * BUFFER_HEIGHT];


void graphicsBlitAtlasImage(int atlasX, int atlasY, int destX, int destY, int w, int h) {
    destY += GAME_Y_OFFSET; // playfield coordinates -> buffer coordinates

    if (destX >= DEFAULT_WIDTH) {
        return;
    }
    if (destY >= BUFFER_HEIGHT) {
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
    if (destY + h > BUFFER_HEIGHT) {
        h = BUFFER_HEIGHT - destY;
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

void graphicsFillBackground(void) {
    unsigned* p = screenImage;
    for (int i = DEFAULT_WIDTH * BUFFER_HEIGHT; i; --i)
        *p++ = BACKGROUND_COLOR;
}

void graphicsRender() {
    // don't redraw window on each frame. redraw window only when redraw event (called when widow moved e.g.)
    ksys_draw_bitmap_palette(screenImage, 0, 0, DEFAULT_WIDTH, BUFFER_HEIGHT, 32, 0, 0);
}

void graphicsDelay(int ms) {
    _ksys_delay((ms + 9) / 10);
}
