#include "game_over_panel.h"

GameOverPanel gameOverPanel;

void gameOverPanelInit(int width, int height) {
	gameOverPanel.width = width;
	gameOverPanel.height = height;
}

void gameOverPanelDraw() {
    double centerX = gameOverPanel.width / 2;
    int textTargetX = (int)round(centerX - (GOP_TEXT_WIDTH / 2));
    int textTargetY = (int)round((gameOverPanel.height - 25) / 3);
    int restartTargetX = centerX - (GOP_RESTART_WIDTH / 2);
    int restartTargetY = gameOverPanel.height / 2;
    // Game over text from sprite
    graphicsBlitAtlasImage(GOP_TEXT_X + ATLAS_TEXT_SPRITE_X, GOP_TEXT_Y + ATLAS_TEXT_SPRITE_Y,
        textTargetX, textTargetY, GOP_TEXT_WIDTH, GOP_TEXT_HEIGHT, false);
    // Restart button
    graphicsBlitAtlasImage(ATLAS_RESTART_X, ATLAS_RESTART_Y,
        restartTargetX, restartTargetY, GOP_RESTART_WIDTH, GOP_RESTART_HEIGHT, false);
}
