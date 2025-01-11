#include "graphics.h"
#include "sprites.h"
#include <assert.h>

/*static*/ ksys_colors_table_t sys_color_table;
/*static*/ ksys_pos_t win_pos;
/*static*/ Image* screenImage;
/*static*/ Image* spriteAtlas;


void graphicsInit() {
    win_pos = _ksys_get_mouse_pos(KSYS_MOUSE_SCREEN_POS);
    _ksys_get_system_colors(&sys_color_table);
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
    screenImage = img_create(DEFAULT_WIDTH, 200, IMAGE_BPP32);
    // asm_inline("emms"); // doenst need bec. libimg functions used here does not use mmx (=> does not currept fpu state)
}

// void save_fpu_state(void *fpustate) {
//     __asm__("fsave %0" : "=m" (*fpustate) : : "memory");
// }

// void restore_fpu_state(const void *fpustate) {
//     __asm__("fnsave %0" : : "m" (*fpustate));
// }


void graphicsBlitAtlasImage(int atlasX, int atlasY, int destX, int destY, int w, int h, bool center) {
    // dbg_printf("start graphicsBlitAtlasImage ax = %d ay = %d dx = %d dy = %d w = %d h = %d %x %x\n", atlasX, atlasY, destX, destY, w, h, screenImage, spriteAtlas);
    
    // asm_inline("int $3");

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

    // asm_inline("int $3");
    /*unsigned char buf[512];
    save_fpu_state(buf);
    img_blend(screenImage, spriteAtlas, destX, destY, atlasX, atlasY, w, h);
    restore_fpu_state(buf);*/

    img_blend(screenImage, spriteAtlas, destX, destY, atlasX, atlasY, w, h);
    asm_inline("emms");

    // dbg_printf("end graphicsBlitAtlasImage\n\n");
}

void graphicsFillBackground(unsigned r, unsigned g, unsigned b) {
    img_fill_color(screenImage, screenImage->Width, screenImage->Height, (0xFF << 24) | (r << 16) | (g << 8) | b);
}

void graphicsRender() {
    // don't redraw window on each frame. redraw window only when redraw event (called when widow moved e.g.)
    //_ksys_start_draw();
    //_ksys_create_window(win_pos.x, win_pos.y, screenImage->Width + 10, screenImage->Height + 29, WINDOW_TITLE, sys_color_table.work_area, 0x54); // 0x54. note: C = 1 !!
    img_draw(screenImage, 5, 24, screenImage->Width, screenImage->Height, 0, 0);
    //ksys_draw_bitmap_palette(screenImage->Data, 5, 24, screenImage->Width, screenImage->Height, 32, 0, 0);
    // ksys_blitter_params_t bp = {5, 24, screenImage->Width, screenImage->Height, 0, 0, screenImage->Width, screenImage->Height, screenImage->Data, screenImage->Width*4};
    // _ksys_blitter(0, &bp);
    //_ksys_end_draw();
}

void graphicsDelay(int ms) {
    // dbg_printf("ms = %d\n", ms);
    _ksys_delay(ms/10 ? ms/10 : 2);
}


void graphicsDestroy() {
    img_destroy(screenImage);
    img_destroy(spriteAtlas);
}
