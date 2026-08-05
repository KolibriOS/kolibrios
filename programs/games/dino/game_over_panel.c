#include "game_over_panel.h"

void gameOverPanelDraw() {
    // Compile-time constants of the 600x150 canvas:
    // text X = round(300 - 191/2) = 205, text Y = round((150-25)/3) = 41,
    // restart X = 300 - 36/2 = 282, restart Y = 150/2 = 75
    graphicsBlitAtlasImage(GOP_TEXT_X + ATLAS_TEXT_SPRITE_X, GOP_TEXT_Y + ATLAS_TEXT_SPRITE_Y,
        205, 41, GOP_TEXT_WIDTH, GOP_TEXT_HEIGHT);
    graphicsBlitAtlasImage(ATLAS_RESTART_X, ATLAS_RESTART_Y,
        282, 75, GOP_RESTART_WIDTH, GOP_RESTART_HEIGHT);
}
