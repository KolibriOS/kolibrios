#include "graphics.h"
#include "sprites.h"
#include <assert.h>


Image* screenImage;
static Image* spriteAtlas;


void graphicsInit() {
    spriteAtlas = img_decode((void*)sprites100, sizeof(sprites100), 0);
    *((uint8_t*)spriteAtlas->Palette + 3) = 0; // set black as transparent
    // for (int i = 0; i < 16; i++) {
    //     dbg_printf("%x\n", *((uint8_t*)spriteAtlas->Palette + i));
    // }
    if (spriteAtlas->Type != IMAGE_BPP32) {
        spriteAtlas = img_convert(spriteAtlas, NULL, IMAGE_BPP32, 0, 0);
        if (!spriteAtlas) {
            dbg_printf("spriteAtlas convert error\n");
            exit(-1);
        }
    }
    dbg_printf("spriteAtlas->Type = %d\n", spriteAtlas->Type);
    screenImage = img_create(DEFAULT_WIDTH, DEFAULT_HEIGHT, IMAGE_BPP32);
    // asm_inline("emms"); // doenst need bec. libimg functions used here does not use mmx (=> does not currept fpu state)
}

void graphicsBlitAtlasImage(int atlasX, int atlasY, int destX, int destY, int w, int h, bool center) {
    // dbg_printf("start graphicsBlitAtlasImage ax = %d ay = %d dx = %d dy = %d w = %d h = %d %x %x\n", atlasX, atlasY, destX, destY, w, h, screenImage, spriteAtlas);

    int screen_width = (int)screenImage->Width;
    int screen_height = (int)screenImage->Height;
    
    if (destX >= screen_width) {
        return;
    }
     if (destY >= screen_height) {
        return;
    }

    if (destX < 0) {
        atlasX -= destX;
        w = destX + w;
        destX = 0;
    }
    if (destX + w > screen_width) {
        w = screen_width - destX;
    }

    if (destY < 0) {
        atlasY -= destY;
        h = destY + h;
        destY = 0;
    }
    if (destY + h > screen_height) {
        h = screen_height - destY;
    }

    if (w <= 0 || h <= 0) {
        return;
    }

    //printf("start graphicsBlitAtlasImage ax = %d ay = %d dx = %d dy = %d w = %d h = %d %x %x\n\n", atlasX, atlasY, destX, destY, w, h, screenImage, spriteAtlas);

    img_blend(screenImage, spriteAtlas, destX, destY, atlasX, atlasY, w, h);
    asm_inline("emms");

    // dbg_printf("end graphicsBlitAtlasImage\n\n");
}

void graphicsFillBackground(unsigned r, unsigned g, unsigned b) {
    img_fill_color(screenImage, screenImage->Width, screenImage->Height, (0xFF << 24) | (r << 16) | (g << 8) | b);
}

void graphicsRender() {
    // don't redraw window on each frame. redraw window only when redraw event (called when widow moved e.g.)
    img_draw(screenImage, 5, 24, screenImage->Width, screenImage->Height, 0, 0);
}

void graphicsDelay(int ms) {
    // dbg_printf("ms = %d\n", ms);
    _ksys_delay(ms/10 ? ms/10 : 2);
}


void graphicsDestroy() {
    img_destroy(screenImage);
    img_destroy(spriteAtlas);
}
