#include "collision.h"
#include "math.h"
#include "game.h"
#include "PHL.h"
#include "object.h"

int checkMix(Mask r, Mask c);
int checkRect(Mask r1, Mask r2);
int checkCircle(Mask c1, Mask c2);

int checkCollision(Mask m1, Mask m2)
{
	if (m1.unused != 1 && m2.unused != 1) {
		if (m1.circle == 0 && m2.circle == 0) {
			return checkRect(m1, m2);
		}else if (m1.circle == 1 && m2.circle == 1) {
			return checkCircle(m1, m2);
		}else if (m1.circle == 1 && m2.circle == 0) {
			return checkMix(m2, m1);
		}else if (m1.circle == 0 && m2.circle == 1) {
			return checkMix(m1, m2);
		}
	}
	
	return 0;
}

int checkCollisionXY(Mask m, int x, int y)
{
	int result = 0;
	
	if (m.unused != 1) {
		if (m.circle == 1) {
			if (sqrt( pow(x - m.x, 2) + pow(y - m.y, 2) ) <= m.w) {
				result = 1;
			}
		}else{
			if (x < m.x || x > m.x + m.w || y < m.y || y > m.y + m.h) {
			}else{
				result = 1;
			}
		}
	}
	
	return result;
}

//Returns 1 or 0 depending on if there is a collision with a type of tile
int checkTileCollision(int type, Mask m)
{
	int result = 0;
	
	if (m.x < 0) {
		m.x = 0;
	}else if (m.x + m.w > 640) {
		m.x = 640 - m.w;
	}
	
	if (m.y < 0) {
		m.y = 0;
	}else if (m.y + m.h > 480) {
		m.y = 480 - m.h;
	}
	
	int i;
	for (i = 0; i < 4; i++) {
		int tileX = (int)m.x / 40;
		int tileY = (int)m.y / 40;
		
		if (i == 1) {
			tileX = (int)((m.x + m.w - 1) / 40);
		}else if (i == 2) {
			tileY = (int)((m.y + m.h - 1) / 40);
		}else if (i == 3) {
			tileX = (int)((m.x + m.w - 1) / 40);
			tileY = (int)((m.y + m.h - 1) / 40);
		}
		
		if (collisionTiles[tileX][tileY] == type) {
			result = 1;
			i = 4;
		}
	}
	
	return result;
}

//Returns a tile's demension. Overkill for a lot of situations.
PHL_Rect getTileCollision(int type, Mask m)
{
	PHL_Rect result;
	result.x = -1;
	result.y = -1;
	result.w = 40;
	result.h = 40;
	
	//updateMask();
	
	if (m.x < 0) {
		m.x = 0;
	}else if (m.x + m.w > 640) {
		m.x = 640 - m.w;
	}
	
	if (m.y < 0) {
		m.y = 0;
	}else if (m.y + m.h > 480) {
		m.y = 480 - m.h;
	}
	
	//PHL_DrawRect(mask.x, mask.y, mask.w, mask.h, PHL_NewRGB(0x00, 0x00, 0xFF));
	
	int i;
	for (i = 0; i < 4; i++) {
		int tileX = (int)m.x / 40;
		int tileY = (int)m.y / 40;
		
		if (i == 1) {
			tileX = (int)((m.x + m.w - 1) / 40);
		}else if (i == 2) {
			tileY = (int)((m.y + m.h - 1) / 40);
		}else if (i == 3) {
			tileX = (int)((m.x + m.w - 1) / 40);
			tileY = (int)((m.y + m.h - 1) / 40);
		}
		
		if (collisionTiles[tileX][tileY] == type) {
			result.x = tileX * 40;
			result.y = tileY * 40;
			i = 4;
			//PHL_DrawRect(result.x, result.y, 40, 40, PHL_NewRGB(0xFF, 0x00, 0x00));
		}
		//PHL_DrawRect(tileX * 40, tileY * 40, 40, 40, PHL_NewRGB(0x00, 0xFF, 0x00));
	}
	
	//updateMask();
	return result;
}

int checkTileCollisionXY(int type, int x, int y)
{
	int result = 0;
	
	if (x < 0) {
		x = 0;
	}else if (x > 640) {
		x = 640;
	}
	
	if (y < 0) {
		y = 0;
	}else if (y > 480) {
		y = 480;
	}
	
	int tileX = (int)x / 40;
	int tileY = (int)y / 40;
		
	if (collisionTiles[tileX][tileY] == type) {
		result = 1;
	}
	
	return result;
}

PHL_Rect getTileCollisionXY(int type, int x, int y)
{
	PHL_Rect result;
	result.x = -1;
	result.y = -1;
	result.w = 40;
	result.h = 40;
	
	if (x < 0) {
		x = 0;
	}else if (x > 640) {
		x = 640;
	}
	
	if (y < 0) {
		y = 0;
	}else if (y > 480) {
		y = 480;
	}
	
	int tileX = (int)x / 40;
	int tileY = (int)y / 40;
		
	if (collisionTiles[tileX][tileY] == type) {
		result.x = tileX * 40;
		result.y = tileY * 40;
	}
	
	return result;
}

void PHL_DrawMask(Mask m)
{
	if (m.circle == 0) {
		PHL_DrawRect(m.x, m.y, m.w, m.h, PHL_NewRGB(255, 255, 255));
	}else if (m.circle == 1) {
		PHL_DrawRect(m.x - m.w, m.y - m.w, m.w * 2, m.w * 2, PHL_NewRGB(255, 255, 255));
	}
}

int checkMix(Mask r, Mask c)
{
	int insidex = 0, insidey = 0;
	
	if (c.x >= r.x && c.x <= r.x + r.w) {
		insidex = 1;
	}
	if (c.y >= r.y && c.y <= r.y + r.h) {
		insidey = 1;
	}
	
	//Check if circle center is inside rectangle
	if (insidex == 1 && insidey == 1) {
	}
	else if (insidex == 1) {
		if ((c.y < r.y && r.y - c.y <= c.w) ||
			(c.y > (r.y + r.h) && c.y - (r.y + r.h) <= c.w)) {
		}else{
			return 0;
		}
	}else if (insidey == 1) {
		if ((c.x < r.x && r.x - c.x <= c.w) ||
			(c.x > (r.x + r.w) && c.x - (r.x + r.w) <= c.w)) {
		}else{
			return 0;
		}
	}else{
		//Check points
		if (sqrt( pow(r.x - c.x, 2) + pow(r.y - c.y, 2) ) <= c.w) {
		}else if (sqrt( pow(r.x + r.w - c.x, 2) + pow(r.y - c.y, 2) ) <= c.w) {
		}else if (sqrt( pow(r.x - c.x, 2) + pow(r.y + r.h - c.y, 2) ) <= c.w) {
		}else if (sqrt( pow(r.x + r.w - c.x, 2) + pow( r.y + r.h -  c.y, 2) ) <= c.w) {
		}else{
			return 0;
		}
	}
		
	return 1;
}

int checkRect(Mask r1, Mask r2)
{
	if (r1.x > r2.x + r2.w ||
		r1.x + r1.w < r2.x ||
		r1.y > r2.y + r2.h ||
		r1.y + r1.h < r2.y)
	{
		return 0;
	}
	
	return 1;
}

int checkCircle(Mask c1, Mask c2)
{
	int maxdis = c1.w + c2.w;
	int dis = sqrt(pow(c2.x - c1.x, 2) + pow(c2.y - c1.y, 2));
	
	if (dis <= maxdis) {
		return 1;
	}
	
	return 0;
}

//Heavier tile collision that omits destroyable blocks
PHL_Rect getTileCollisionWeapon(int type, Mask m)
{
	PHL_Rect result;
	result.x = -1;
	result.y = -1;
	result.w = 40;
	result.h = 40;
	
	//updateMask();
	
	if (m.x < 0) {
		m.x = 0;
	}else if (m.x + m.w > 640) {
		m.x = 640 - m.w;
	}
	
	if (m.y < 0) {
		m.y = 0;
	}else if (m.y + m.h > 480) {
		m.y = 480 - m.h;
	}
	
	//PHL_DrawRect(mask.x, mask.y, mask.w, mask.h, PHL_NewRGB(0x00, 0x00, 0xFF));
	
	int i;
	for (i = 0; i < 4; i++) {
		int tileX = (int)m.x / 40;
		int tileY = (int)m.y / 40;
		
		if (i == 1) {
			tileX = (int)((m.x + m.w - 1) / 40);
		}else if (i == 2) {
			tileY = (int)((m.y + m.h - 1) / 40);
		}else if (i == 3) {
			tileX = (int)((m.x + m.w - 1) / 40);
			tileY = (int)((m.y + m.h - 1) / 40);
		}
		
		if (collisionTiles[tileX][tileY] == type) {
			result.x = tileX * 40;
			result.y = tileY * 40;
			
			//Check if destroyable block
			int a;
			for (a = 0; a < MAX_OBJECTS; a++) {
				if (objects[a] != NULL) {
					if (objects[a]->type == 3) {
						Destroyable* d = objects[a]->data;
						if (result.x == d->x && result.y == d->y) {
							result.x = -1;
							result.y = -1;
						}
					}
				}
			}
			
			if (result.x != -1) {
				i = 4;
			}
		}
	}
	
	//updateMask();
	return result;
}