#include "smalllibc/kosSyst.h"
#include "smalllibc/kosFile.h"
#include "smalllibc/sprintf.h"
#include "smalllibc/func.h"
#include "render.h"
#include "image.h"

#define MODE_MENU		0
#define MODE_LEVELS		1
#define MODE_GAME		2

#define GAME_NONE		0
#define GAME_VICTORY	1
#define GAME_DEFEAT		2

#define MOUSE_LEFT		0

#define FIELD_NONE			0
#define FIELD_WATER			1
#define FIELD_GROUND		2
#define FIELD_BRICK			3
#define FIELD_WALL			4
#define FIELD_BOX			5
#define FIELD_MISSLE_0		6
#define FIELD_MISSLE_1		7
#define FIELD_MISSLE_2		8
#define FIELD_MISSLE_3		9
#define FIELD_HERO			10
#define FIELD_FINISH		11
#define FIELD_GUN_0			12
#define FIELD_GUN_1			13
#define FIELD_GUN_2			14
#define FIELD_GUN_3			15
#define FIELD_BOX_MISSLE_0	16
#define FIELD_BOX_MISSLE_1	17
#define FIELD_BOX_MISSLE_2	18
#define FIELD_BOX_MISSLE_3	19
#define FIELD_BRICK_DES		20
#define FIELD_BOX_WATER		21


char* header = "Laser Tank";

struct Level
{
	Byte fileds[16][16];
};

void pause(int time)
{
	kos_Pause(time);
	Byte keyCode;
	for (int i = 0; i < 10; ++i)
		kos_GetKey(keyCode);
}

Level *levels;
int levelCount = 0;
int levelIndex = 0;

RGBA img_tank[576];
RGB img_water[576];
RGB img_brick[11][576];
RGB img_waterbox[576];
RGB img_ground[576];
RGB img_wall[576];
RGB img_finish[576];
RGBA img_box[576];
RGBA img_laser[576];
RGB img_mirror[4][576];
RGBA img_mini_mirror[4][576];
RGBA img_laser1[576];
RGBA img_laser2[576];
RGB img_brick1[576];
RGB img_menu[147456];
RGBA img_explosion[8064];
RGBA img_gun[576];

RGB img_button[7500];

Player player;

CKosRender* renderPlayer;
CKosImage* objPlayer;

CKosRender* renderLaser;
CKosImage* objLaser;
CKosImage* objLaser1;
CKosImage* objLaser2;

CKosRender* renderMirror;
CKosImage* objMiniMirror[4];

CKosRender* renderBox;
CKosImage* objBox;
CKosImage* objGun;

CKosRender* renderExplosion;
CKosImage* objExplosion;

int gameMode = MODE_MENU;
int gameStatus = GAME_NONE;

void draw_window(); 
void openLevel(int index);
void Laser(Point pos, Point vec, RGB color);

bool CollRecrVsPoint(Point point, Rect rect)
{
	return (point.X > rect.X && point.Y > rect.Y && point.X < rect.X + rect.Width && point.Y < rect.Y + rect.Height);
}

struct Button
{
	Rect rect;
	char* caption;
	Button() {}
	Button(char* caption, Rect rect)
	{
		this->caption = caption;
		this->rect = rect;
	}
};

Button ToGame("Game", Rect(150, 258, 224, 50));
Button ToExit("Exit", Rect(150, 321, 224, 50));

struct LvlItem
{
	Byte s;
	Byte d;
	Byte l;
};

LvlItem level[16][16];

char GetField(Point position, bool din)
{
	if (din && level[position.Y][position.X].d != FIELD_NONE)
		return level[position.Y][position.X].d;
	return level[position.Y][position.X].s;
}

RGB* GetImg(Point position, bool din)
{
	switch (GetField(position, din))
	{
	case FIELD_WALL:
		return (RGB*)img_wall;
	case FIELD_MISSLE_0:
		return (RGB*)img_mirror[0];
	case FIELD_MISSLE_1:
		return (RGB*)img_mirror[1];
	case FIELD_MISSLE_2:
		return (RGB*)img_mirror[2];
	case FIELD_MISSLE_3:
		return (RGB*)img_mirror[3];
	case FIELD_GUN_0:
		renderBox->RenderImg(GetImg(position, false), Point(0, 0), 24, 24);
		objGun->Draw(Point(0, 0), 0);
		return renderBox->buffer;
	case FIELD_GUN_1:
		renderBox->RenderImg(GetImg(position, false), Point(0, 0), 24, 24);
		objGun->Draw(Point(0, 0), 90);
		return renderBox->buffer;
	case FIELD_GUN_2:
		renderBox->RenderImg(GetImg(position, false), Point(0, 0), 24, 24);
		objGun->Draw(Point(0, 0), 180);
		return renderBox->buffer;
	case FIELD_GUN_3:
		renderBox->RenderImg(GetImg(position, false), Point(0, 0), 24, 24);
		objGun->Draw(Point(0, 0), 270);
		return renderBox->buffer;
	case FIELD_GROUND:
		return (RGB*)img_ground;
	case FIELD_BOX:
		renderBox->RenderImg(GetImg(position, false), Point(0, 0), 24, 24);
		objBox->Draw(Point(0, 0), 0);
		return renderBox->buffer;
	//	return (RGB*)img_box;
	case FIELD_FINISH:
		return (RGB*)img_finish;
	case FIELD_BRICK:
		return (RGB*)img_brick[level[position.Y][position.X].l];
	case FIELD_WATER:
		return (RGB*)img_water;
	case FIELD_BOX_WATER:
		return (RGB*)img_waterbox;
	case FIELD_BRICK_DES:
		return (RGB*)img_brick1;
	}
	return NULL;
}

Byte CollField(Point position)
{
	if (position.X < 0 || position.Y < 0 || position.X > 15 || position.Y > 15)
		return 0;

	switch (GetField(position, true))
	{
	case FIELD_NONE:
	case FIELD_WATER:
	case FIELD_GROUND:
	case FIELD_BOX_WATER:
	case FIELD_BRICK_DES:
	case FIELD_FINISH:
		return 1;
	case FIELD_GUN_0:
		return FIELD_GUN_0;
	case FIELD_GUN_1:
		return FIELD_GUN_1;
	case FIELD_GUN_2:
		return FIELD_GUN_2;
	case FIELD_GUN_3:
		return FIELD_GUN_3;
	}
	return 0;
}

bool ExistGun1(Point position, Point vec, int gun)
{
	rtlDebugOutString("ExistGun");
	Point pos = position;
	
	Byte result = 1;
	while (result == 1)
	{
		pos = pos + vec;
		result = CollField(pos);
	}
	if (result == gun)
	{
		Laser(pos, vec * -1, (RGB)0x00CCFF);
		return true;
	}
	return false;
}

void ExistGun(Point position)
{
	if (ExistGun1(position, Point(1, 0), FIELD_GUN_2))
		return;
	if (ExistGun1(position, Point(0, 1), FIELD_GUN_3))
		return;
	if (ExistGun1(position, Point(-1, 0), FIELD_GUN_0))
		return;
	ExistGun1(position, Point(0, -1), FIELD_GUN_1);
}

void DrawElevent(Point position, bool din)
{
	kos_PutImage(GetImg(position, din), 24, 24, 24 * position.X, 24 * position.Y);
}

void MoveBox(Point a, Point b)
{
	Byte code = GetField(a, true);
	level[a.Y][a.X].d = FIELD_NONE;
	DrawElevent(a, true);
	if (level[b.Y][b.X].s == FIELD_WATER)
	{
		if (code = FIELD_BOX)
			level[b.Y][b.X].s = FIELD_BOX_WATER;
		DrawElevent(b, true);
	}
	else
	{
		level[b.Y][b.X].d = code;
		DrawElevent(b, true);
	}
}

void animation(Point vector, float angle, int obj)
{
	for (int i = 2; i < 23; ++i)
	{
		kos_WindowRedrawStatus(1);
		DrawElevent(player.position, false);
		DrawElevent(player.position + vector, false);

		renderPlayer->RenderImg(GetImg(player.position, true), Point(0, 0), 24, 24);
		objPlayer->Draw(Point(0, 0), angle);
		renderPlayer->Draw(player.position * 24 + vector * i);
		if (level[player.position.Y + vector.Y][player.position.X + vector.X].d == obj)
		{
			renderBox->RenderImg(GetImg(player.position + vector, true), Point(0, 0), 24, 24);
			switch (obj)
			{
			case FIELD_GUN_0:
				objGun->Draw(Point(0, 0), 0);
				break;
			case FIELD_GUN_1:
				objGun->Draw(Point(0, 0), 90);
				break;
			case FIELD_GUN_2:
				objGun->Draw(Point(0, 0), 180);
				break;
			case FIELD_GUN_3:
				objGun->Draw(Point(0, 0), 270);
				break;
			case FIELD_BOX:
				objBox->Draw(Point(0, 0), 0);
			}
			renderBox->Draw((player.position + vector) * 24 + vector * i);
		}
		kos_WindowRedrawStatus(2);
		pause(1);
	}

	if (level[player.position.Y + vector.Y][player.position.X + vector.X].d == obj)
		MoveBox(player.position + vector, player.position + vector * 2);

	DrawElevent(player.position, true);
	DrawElevent(player.position + vector, true);
	player.position = player.position + vector;
	//kos_PutImage(GetImg(player.position + vector), 24, 24, 24 * player.position.X, 24 * player.position.Y);
	renderPlayer->RenderImg(GetImg(player.position, false), Point(0, 0), 24, 24);
	objPlayer->Draw(Point(0, 0), angle);
	renderPlayer->Draw(player.position * 24);
	ExistGun(player.position);
}

void DrawLaser(Point position, int frame, RGB color)
{
	renderLaser->RenderImg(GetImg(position, false), Point(0, 0), 24, 24);
	switch (frame)
	{
	case 1:
		objLaser->Draw(Point(0, 0), 0, color);
		break; 
	case 2:
		objLaser->Draw(Point(0, 0), 90, color);
		break;
	case 3:
		objLaser2->Draw(Point(0, 0), 0, color);
		break;
	default:
		objLaser1->Draw(Point(-1, 0), (float)frame, color);
	}
	renderLaser->Draw(position * 24);
	level[position.Y][position.X].l = 1;
}

void Laser(Point pos, Point vec, RGB color)
{
	Point vector = vec;
	Point position = pos + vector;
	bool en = true;
	Byte code;
	bool LaserGun = false;
	
	while (en)
	{
		code = GetField(position, true);
		switch (code)
		{
		case FIELD_BOX:
		case FIELD_GUN_0:
		case FIELD_GUN_1:
		case FIELD_GUN_2:
		case FIELD_GUN_3:
			if (code == FIELD_GUN_0 && vector == Point(-1, 0) || code == FIELD_GUN_1 && vector == Point(0, -1)
				|| code == FIELD_GUN_2 && vector == Point(1, 0) || code == FIELD_GUN_3 && vector == Point(0, 1))
			{
				for (int i = 2; i < 23; ++i)
					kos_Pause(1);
				for (int y = 0; y < 16; y++)
					for (int x = 0; x < 16; x++)
						if (level[y][x].l == 1)
						{
							DrawElevent(Point(x, y), true);
							level[y][x].l = 0;
						}
				for (int i = 0; i < 14; ++i)
				{
					renderExplosion->RenderImg(GetImg(position, false), Point(0, 0), 24, 24);
					objExplosion->Draw(Point(0, 0), 0, i);
					renderExplosion->Draw((position)* 24);
					pause(2);
				}
				level[position.Y][position.X].d = FIELD_NONE;
				draw_window();
				return;
			}
			else
			if (position + vector != player.position)
				switch (GetField(position + vector, true))
				{
				case FIELD_GROUND:
				case FIELD_WATER:
				case FIELD_BRICK_DES:
				case FIELD_BOX_WATER:
					for (int i = 2; i < 23; ++i)
					{
						DrawElevent(position, false);
						DrawElevent(position + vector, true);
						DrawLaser(position, (vector.X != 0) ? 1 : 2, color);
						renderBox->RenderImg(GetImg(position, false), Point(0, 0), 24, 24);			
						switch (code)
						{
						case FIELD_GUN_0:
							objGun->Draw(Point(0, 0), 0);
							break;
						case FIELD_GUN_1:
							objGun->Draw(Point(0, 0), 90);
							break;
						case FIELD_GUN_2:
							objGun->Draw(Point(0, 0), 180);
							break;
						case FIELD_GUN_3:
							objGun->Draw(Point(0, 0), 270);
							break;
						case FIELD_BOX:
							objBox->Draw(Point(0, 0), 0);
						}
						renderBox->Draw((position) * 24 + vector * i);
						kos_Pause(1);
					}
					MoveBox(position, position + vector);
					LaserGun = true;
			}
			en = false;
			break;
		case FIELD_BRICK:
			for (int i = 0; i < 6; ++i)
			{
				level[position.Y][position.X].l += 1;
				if (level[position.Y][position.X].l == 11)
				{
					level[position.Y][position.X].s = FIELD_BRICK_DES;
					level[position.Y][position.X].l = 0;
					LaserGun = true;
				}
				DrawElevent(position, false);
				pause(5);
			}
			en = false;
			break;
		case FIELD_GROUND:
		case FIELD_WATER:
		case FIELD_FINISH:
		case FIELD_BRICK_DES:
		case FIELD_BOX_WATER:
			if (player.position == position)
			{
				for (int i = 2; i < 23; ++i)
					kos_Pause(1);
				for (int y = 0; y < 16; y++)
					for (int x = 0; x < 16; x++)
						if (level[y][x].l == 1)
						{
							DrawElevent(Point(x, y), true);
							level[y][x].l = 0;
						}
				for (int i = 0; i < 14; ++i)
				{
					renderExplosion->RenderImg(GetImg(position, false), Point(0, 0), 24, 24);
					objExplosion->Draw(Point(0, 0), 0, i);
					renderExplosion->Draw((position)* 24);
					pause(2);
				}
				gameStatus = GAME_DEFEAT;
				draw_window();
				return;
			}
			else
			{
				if (level[position.Y][position.X].l == 1)
					DrawLaser(position, 3, color);
				else
					DrawLaser(position, (vector.X != 0) ? 1 : 2, color);
			}
			break;
		case FIELD_MISSLE_0:
			if (vector == Point(-1, 0) || vector == Point(0, -1))
			{
				vector = (vector.Y == -1) ? Point(1, 0) : Point(0, 1);
				DrawLaser(position, 0, color);
			}
			else
			{
				for (int i = 2; i < 23; ++i)
					pause(1);
				en = false;
			}
			break;
		case FIELD_MISSLE_1:
			if (vector == Point(0, -1) || vector == Point(1, 0))
			{
				vector = (vector.Y == -1) ? Point(-1, 0) : Point(0, 1);
				DrawLaser(position, 90, color);
			}
			else
			{
				for (int i = 2; i < 23; ++i)
					pause(1);
				en = false;
			}
			break;
		case FIELD_MISSLE_3:
			if (vector == Point(-1, 0) || vector == Point(0, 1))
			{
				vector = (vector.Y == 1) ? Point(1, 0) : Point(0, -1);
				DrawLaser(position, 270, color);
			}
			else
			{
				for (int i = 2; i < 23; ++i)
					pause(1);
				en = false;
			}
			break;
		case FIELD_MISSLE_2:
			if (vector == Point(1, 0) || vector == Point(0, 1))
			{
				vector = (vector.Y == 1) ? Point(-1, 0) : Point(0, -1);
				DrawLaser(position, 180, color);
			}
			else
			{
				for (int i = 2; i < 23; ++i)
					pause(1);
				en = false;
			}
			break;
		default:
			for (int i = 2; i < 23; ++i)
				pause(1);
			en = false;
		}
		position = position + vector;
	}
	
	for (int y = 0; y < 16; y++)
		for (int x = 0; x < 16; x++)
			if (level[y][x].l == 1)
			{
				DrawElevent(Point(x, y), true);
				level[y][x].l = 0;
			}
	
	if (LaserGun)
		ExistGun(player.position);
}

void player_move(Point vector, float angle)
{
	if (player.vector == vector)
	{
		Byte code = GetField(player.position + vector, true);
		switch (code)
		{
		case FIELD_GUN_0:
		case FIELD_GUN_1:
		case FIELD_GUN_2:
		case FIELD_GUN_3:
		case FIELD_BOX:
			switch (GetField(player.position + vector * 2, true))
			{
			case FIELD_GROUND:
			case FIELD_WATER:
			case FIELD_BOX_WATER:
			case FIELD_BRICK_DES:
				animation(vector, angle, code);
				return;
			}
			break;
		case FIELD_FINISH:
			gameStatus = GAME_VICTORY;
			draw_window();
			break;
		case FIELD_NONE:
		case FIELD_BRICK:
		case FIELD_WALL:
		case FIELD_WATER:
		case FIELD_MISSLE_0:
		case FIELD_MISSLE_1:
		case FIELD_MISSLE_2:
		case FIELD_MISSLE_3:
			break;
		default:
			animation(vector, angle, -1);
		}
	}
	else
	{
		int cnt;
		float addAngle;
		if (player.vector == vector * -1)
		{
			cnt = 48;
			addAngle = 3.5f;
		}
		else
		{
			cnt = 24;
			if (player.angle == 270 && angle == 0 || player.angle == 0 && angle == 270)
				addAngle = (player.angle == 0) ? -3.5f : 3.5f;
			else
				addAngle = (angle < player.angle) ? -3.5f : 3.5f;
		}

		for (int i = 1; i < cnt - 1; ++i)
		{
			player.angle += addAngle;
			renderPlayer->RenderImg(GetImg(player.position, false), Point(0, 0), 24, 24);
			objPlayer->Draw(Point(0, 0), player.angle);
			renderPlayer->Draw(player.position * 24);
			pause(1);
		}

		player.vector = vector;
		player.angle = angle;

		renderPlayer->RenderImg(GetImg(player.position, false), Point(0, 0), 24, 24);
		objPlayer->Draw(Point(0, 0), player.angle);
		renderPlayer->Draw(player.position * 24);
	}
}

void SetMode(int mode)
{
	gameMode = mode;
	draw_window();
}

void key_press(int key)
{
	//rtlDebugOutString(ftoa(key));

	switch (gameMode)
	{
	case MODE_MENU:
		
		break;
	case MODE_LEVELS:

		break;
	case MODE_GAME:
		switch (key)
		{
		case 119: // Up
		case 178: // W
			if (gameStatus == GAME_NONE)
				player_move(Point(0, -1), 270);
			break;
		case 177: // Down
		case 115: // S
			if (gameStatus == GAME_NONE)
				player_move(Point(0, 1), 90);
			break;
		case 176: // Left
		case 97:  // A
			if (gameStatus == GAME_NONE)
				player_move(Point(-1, 0), 180);
			break;
		case 179: // Right
		case 100: // D
			if (gameStatus == GAME_NONE)
				player_move(Point(1, 0), 0);
			break;
		case 32: // Space
			if (gameStatus == GAME_NONE)
				Laser(player.position, player.vector, (RGB)0x00FF00);
			break;
		case 13:
		//	openLevel(levelIndex + 1);
			if (gameStatus == GAME_VICTORY)
				openLevel(levelIndex + 1);
			else
				if (gameStatus == GAME_DEFEAT)
					openLevel(levelIndex);
			break;
		}
		break;
	}
}

void MousePress(int button, Point position)
{
	//rtlDebugOutString("Mouse");
	//rtlDebugOutString(ftoa(position.X));
	//rtlDebugOutString(ftoa(position.Y));

	switch (gameMode)
	{
	case MODE_MENU:
		if (CollRecrVsPoint(position, ToGame.rect))
			SetMode(MODE_GAME);
		if (CollRecrVsPoint(position, ToExit.rect))
			kos_ExitApp();
		break;
	case MODE_LEVELS:

		break;
	case MODE_GAME:

		break;
	}
}

void draw_window(void)
{
	kos_WindowRedrawStatus(1);
	kos_DefineAndDrawWindow(10, 40, 384 + 9, 384 + 25, 0x33, 0x444444, 0, 0, (Dword)header);

	switch (gameMode)
	{
	case MODE_MENU:
		kos_PutImage((RGB*)img_menu, 384, 384, 0, 0);

	//	kos_PutImage((RGB*)img_button, 150, 50, ToGame.rect.X, ToGame.rect.Y);
		
		
		break;
	case MODE_LEVELS:

		break;
	case MODE_GAME:
		for (int y = 0; y < 16; y++)
			for (int x = 0; x < 16; x++)
			{
				if (level[y][x].s != FIELD_NONE)
					kos_PutImage(GetImg(Point(x, y), true), 24, 24, 24 * x, 24 * y);
				if (level[y][x].d == FIELD_BOX)
				{
					renderBox->RenderImg(GetImg(player.position, false), Point(0, 0), 24, 24);
					objBox->Draw(Point(0, 0), 0);
					renderBox->Draw(Point(x, y) * 24);
				}
			}

		switch (gameStatus)
		{
		case GAME_NONE:
			renderPlayer->RenderImg(GetImg(player.position, false), Point(0, 0), 24, 24);
			objPlayer->Draw(Point(0, 0), player.angle);
			renderPlayer->Draw(player.position * 24);
			break;
		case GAME_VICTORY:
			kos_WriteTextToWindow(30, 10, 0x80, 0xFFFFFF, "VICTORY", 0);
			break;
		case GAME_DEFEAT:
			kos_WriteTextToWindow(30, 10, 0x80, 0xFFFFFF, "DEFEAT", 0);
			break;
		}
	break;
	}
	kos_WindowRedrawStatus(2);
}

void LevelsLoad()
{
	char *cPtr;
	cPtr = strrchr(kosExePath, '/');

	if (cPtr == NULL)
	{
		rtlDebugOutString("Invalid path to executable.");
		return;
	}
	cPtr[1] = 0;
	strcpy(cPtr + 1, "levels.lvl");
	
	CKosFile *file = new CKosFile(kosExePath);

	Byte block[256];
	while (file->Read(block, 256) == 256)
	{
		levelCount++;
	}
	//levelCount++;
	rtlDebugOutString(ftoa(levelCount));

	levels = new Level[levelCount];

	file->Seek(0, SEEK_SET);
	for (int i = 0; i < levelCount; ++i)
	{
		file->Read(block, 256);
		int k = 0;

		for (int y = 0; y < 16; y++)
			for (int x = 0; x < 16; x++)
			{
				levels[i].fileds[y][x] = block[k];
				k++;
			}		
	}
}

void openLevel(int index)
{
	levelIndex = index;
	for (int y = 0; y < 16; y++)
		for (int x = 0; x < 16; x++)
		{
			level[y][x].s = levels[index].fileds[y][x];
			level[y][x].d = FIELD_NONE;
			level[y][x].l = 0;
			switch (levels[index].fileds[y][x])
			{
			case FIELD_BOX:
			case FIELD_GUN_0:
			case FIELD_GUN_1:
			case FIELD_GUN_2:
			case FIELD_GUN_3:
				level[y][x].s = FIELD_GROUND;
				level[y][x].d = levels[index].fileds[y][x];
				break;
			case FIELD_HERO:
				player = Player(Point(x, y));
				level[y][x].s = FIELD_GROUND;
				break;
			case FIELD_BRICK:
				level[y][x].l = 0;
				break;
			}
		}
	gameStatus = GAME_NONE;
	draw_window();
}

void kos_Main()
{
	rtlDebugOutString(" ");
	rtlDebugOutString("kos_Main");
	char *cPtr;
	cPtr = strrchr(kosExePath, '/');
	// проверка ;)
	if (cPtr == NULL)
	{
		rtlDebugOutString("Invalid path to executable.");
		return;
	}
	cPtr[1] = 0;
	strcpy(cPtr + 1, "arh.pak");
	
	CKosFile *file = new CKosFile(kosExePath);

	file->LoadTex((Byte*)img_box, 4, 24, 24);
	for (int i = 0; i < 11; ++i)
		file->LoadTex((Byte*)img_brick[i], 3, 24, 24);
	file->LoadTex((Byte*)img_brick1, 3, 24, 24);
	file->LoadTex((Byte*)img_finish, 3, 24, 24);
	file->LoadTex((Byte*)img_ground, 3, 24, 24);
	file->LoadTex((Byte*)img_laser, 4, 24, 24);	
	file->LoadTex((Byte*)img_laser1, 4, 24, 24);
	file->LoadTex((Byte*)img_laser2, 4, 24, 24);

	for (int i = 0; i < 4; ++i)
		file->LoadTex((Byte*)img_mirror[i], 3, 24, 24);

	for (int i = 0; i < 4; ++i)
		file->LoadTex((Byte*)img_mini_mirror[4], 4, 24, 24);

	file->LoadTex((Byte*)img_tank, 4, 24, 24);
	file->LoadTex((Byte*)img_wall, 3, 24, 24);
	file->LoadTex((Byte*)img_water, 3, 24, 24);
	file->LoadTex((Byte*)img_waterbox, 3, 24, 24);
	file->LoadTex((Byte*)img_menu, 3, 384, 384);
	file->LoadTex((Byte*)img_button, 3, 150, 50);	
	file->LoadTex((Byte*)img_explosion, 4, 24, 336);	
	file->LoadTex((Byte*)img_gun, 4, 24, 24);	
	
	delete file;

	renderPlayer = new CKosRender(24, 24);
	objPlayer = new CKosImage(renderPlayer, (RGBA*)img_tank, 24, 24);

	renderLaser = new CKosRender(24, 24);
	objLaser = new CKosImage(renderLaser, (RGBA*)img_laser, 24, 24);
	objLaser->SetMode(DRAW_ALPHA_ADD);
	objLaser1 = new CKosImage(renderLaser, (RGBA*)img_laser1, 24, 24);
	objLaser1->SetMode(DRAW_ALPHA_ADD);
	objLaser2 = new CKosImage(renderLaser, (RGBA*)img_laser2, 24, 24);
	objLaser2->SetMode(DRAW_ALPHA_ADD);
			
	renderMirror = new CKosRender(24, 24);

	for (int i = 0; i < 4; ++i)
		objMiniMirror[i] = new CKosImage(renderMirror, (RGBA*)img_mini_mirror[i], 24, 24);

	renderBox = new CKosRender(24, 24);
	objBox = new CKosImage(renderBox, (RGBA*)img_box, 24, 24);
	objGun = new CKosImage(renderBox, (RGBA*)img_gun, 24, 24);	

	renderExplosion = new CKosRender(24, 24);
	objExplosion = new CKosImage(renderExplosion, (RGBA*)img_explosion, 24, 24);
	objExplosion->SetFrameSize(24, 24);

	LevelsLoad();

	openLevel(0);

	kos_SetMaskForEvents(0x27);
	for (;;)
	{
		switch (kos_WaitForEvent())
		{
		case 1:
			draw_window();
			break;
		case 2:
			Byte keyCode;
			kos_GetKey(keyCode);
			key_press(keyCode);
			break;
		case 3:
			kos_ExitApp();
			break;
		case 6:
			Dword buttons;
			int mX, mY;
			kos_GetMouseState(buttons, mX, mY);
			if (buttons & 1)
				MousePress(MOUSE_LEFT, Point(mX, mY));
			break;
		}
	}
}
