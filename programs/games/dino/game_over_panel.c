#include "game_over_panel.h"
#include "runner.h"

void gameOverPanelDraw() {
    double centerX = DEFAULT_WIDTH / 2;
    int textTargetX = (int)round(centerX - (GOP_TEXT_WIDTH / 2));
    int textTargetY = (int)round((RUNNER_DEFAULT_HEIGHT - 25) / 3);
    int restartTargetX = centerX - (GOP_RESTART_WIDTH / 2);
    int restartTargetY = RUNNER_DEFAULT_HEIGHT / 2;
    // Game over text from sprite
    graphicsBlitAtlasImage(GOP_TEXT_X + ATLAS_TEXT_SPRITE_X, GOP_TEXT_Y + ATLAS_TEXT_SPRITE_Y,
        textTargetX, textTargetY, GOP_TEXT_WIDTH, GOP_TEXT_HEIGHT);
    // Restart button
    graphicsBlitAtlasImage(ATLAS_RESTART_X, ATLAS_RESTART_Y,
        restartTargetX, restartTargetY, GOP_RESTART_WIDTH, GOP_RESTART_HEIGHT);
}
