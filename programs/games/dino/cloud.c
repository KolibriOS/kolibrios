#include "cloud.h"

void cloudInit(Cloud* cloud, int w) {
	cloud->width = w;
	cloud->xPos = w;
	cloud->yPos = 0;
	cloud->remove = false;
	cloud->cloudGap = getRandomNumber(CLOUD_MIN_GAP, CLOUD_MAX_GAP);

	cloud->yPos = getRandomNumber(CLOUD_MAX_SKY_LEVEL, CLOUD_MIN_SKY_LEVEL); // NOTE why swapped
	cloudDraw(cloud);
}

void cloudDraw(const Cloud* cloud) {
	graphicsBlitAtlasImage(ATLAS_CLOUD_X, ATLAS_CLOUD_Y, cloud->xPos, cloud->yPos, CLOUD_WIDTH, CLOUD_HEIGHT, false);
}

void cloudUpdate(Cloud* cloud, double speed) {
	// printf("cloudUpdate(., %f)\n", speed);
	if (!cloud->remove) {
		cloud->xPos -= (int)ceil(speed);
		cloudDraw(cloud);

		// Mark as removeable if no longer in the canvas.
		if (!cloudIsVisible(cloud)) {
			cloud->remove = true;
		}
	}
}

bool cloudIsVisible(const Cloud* cloud) {
	return cloud->xPos + CLOUD_WIDTH > 0;
}

