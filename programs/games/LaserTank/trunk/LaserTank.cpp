#include "smalllibc/kosSyst.h"
#include "smalllibc/kosFile.h"
#include "smalllibc/sprintf.cpp"
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

char* header = "Laser Tank";

struct Level
{
	char fileds[16][16];
};

Level *levels;
int levelCount = 0;
int levelIndex = 0;

RGBA img_tank[576];
RGB img_water[576];
RGB img_brick[576];
RGB img_waterbox[576];
RGB img_ground[576];
RGB img_wall[576];
RGB img_finish[576];
RGBA img_box[576];
RGBA img_laser[576];
RGBA img_mirror[576];
RGBA img_mirror90[576];
RGBA img_mirror180[576];
RGBA img_mirror270[576];
RGBA img_mini_mirror[576];
RGBA img_mini_mirror90[576];
RGBA img_mini_mirror180[576];
RGBA img_mini_mirror270[576];
RGBA img_laser1[576];
RGBA img_laser2[576];
RGB img_brick1[576];
RGB img_menu[147456];
RGBA img_explosion[8064];

RGB img_button[7500];

Player player;

CKosRender* renderPlayer;
CKosImage* objPlayer;

CKosRender* renderLaser;
CKosImage* objLaser;
CKosImage* objLaser1;
CKosImage* objLaser2;

CKosRender* renderMirror;
CKosImage* objMirror;
CKosImage* objMirror90;
CKosImage* objMirror180;
CKosImage* objMirror270;

CKosImage* objMiniMirror;
CKosImage* objMiniMirror90;
CKosImage* objMiniMirror180;
CKosImage* objMiniMirror270;

CKosRender* renderBox;
CKosImage* objBox;

CKosRender* renderExplosion;
CKosImage* objExplosion;

int gameMode = MODE_MENU;
int gameStatus = GAME_NONE;

void draw_window(); 
void openLevel(int index);

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
	char s;
	char d;
	Byte l;
};

LvlItem level[16][16];

char GetField(Point position, bool din)
{
	if (din && level[position.Y][position.X].d != ' ')
		return level[position.Y][position.X].d;
	return level[position.Y][position.X].s;
}

RGB* GetImg(Point position, bool din)
{
	switch (GetField(position, din))
	{
	case 's':
		return (RGB*)img_wall;
	case '7':
		renderMirror->RenderImg(GetImg(player.position, false), Point(0, 0), 24, 24);
		objMirror->Draw(Point(0, 0), 0);
		return renderMirror->buffer;
	case '9':
		renderMirror->RenderImg(GetImg(player.position, false), Point(0, 0), 24, 24);
		objMirror90->Draw(Point(0, 0), 0);
		return renderMirror->buffer;
	case '3':
		renderMirror->RenderImg(GetImg(player.position, false), Point(0, 0), 24, 24);
		objMirror180->Draw(Point(0, 0), 0);
		return renderMirror->buffer;
	case '1':
		renderMirror->RenderImg(GetImg(player.position, false), Point(0, 0), 24, 24);
		objMirror270->Draw(Point(0, 0), 0);
		return renderMirror->buffer;
	case 'h':
	case 'g':
		return (RGB*)img_ground;
	case 'b':
		renderBox->RenderImg(GetImg(player.position, false), Point(0, 0), 24, 24);
		objBox->Draw(Point(0, 0), 0);
		return renderBox->buffer;
	//	return (RGB*)img_box;
	case 'f':
		return (RGB*)img_finish;
	case 'x':
		return (RGB*)img_brick;
	case 'w':
		return (RGB*)img_water;
	case 'e':
		return (RGB*)img_waterbox;
	case 'y':
		return (RGB*)img_brick1;
	}
	return NULL;
}

void DrawElevent(Point position, bool din)
{
	kos_PutImage(GetImg(position, din), 24, 24, 24 * position.X, 24 * position.Y);
}

void MoveBox(Point a, Point b)
{
	level[a.Y][a.X].d = ' ';
	DrawElevent(a, true);
	if (level[b.Y][b.X].s == 'w')
	{
		level[b.Y][b.X].s = 'e';
		DrawElevent(b, true);
	}
	else
	{
		level[b.Y][b.X].d = 'b';
		DrawElevent(b, true);
	}
}

void animation(Point vector, float angle)
{
	for (int i = 2; i < 23; ++i)
	{
		kos_WindowRedrawStatus(1);
		DrawElevent(player.position, false);
		DrawElevent(player.position + vector, false);

		renderPlayer->RenderImg(GetImg(player.position, true), Point(0, 0), 24, 24);
		objPlayer->Draw(Point(0, 0), angle);
		renderPlayer->Draw(player.position * 24 + vector * i);
		if (level[player.position.Y + vector.Y][player.position.X + vector.X].d == 'b')
		{
			renderBox->RenderImg(GetImg(player.position, true), Point(0, 0), 24, 24);
			objBox->Draw(Point(0, 0), 0);
			renderBox->Draw((player.position + vector) * 24 + vector * i);
		}
		kos_WindowRedrawStatus(2);
		kos_Pause(1);
	}

	if (level[player.position.Y + vector.Y][player.position.X + vector.X].d == 'b')
		MoveBox(player.position + vector, player.position + vector * 2);

	DrawElevent(player.position, true);
	DrawElevent(player.position + vector, true);
	player.position = player.position + vector;
	//kos_PutImage(GetImg(player.position + vector), 24, 24, 24 * player.position.X, 24 * player.position.Y);
	renderPlayer->RenderImg(GetImg(player.position, false), Point(0, 0), 24, 24);
	objPlayer->Draw(Point(0, 0), angle);
	renderPlayer->Draw(player.position * 24);
}

void DrawLaser(Point position, int frame)
{
	renderLaser->RenderImg(GetImg(position, false), Point(0, 0), 24, 24);
	switch (frame)
	{
	case 1:
		objLaser->Draw(Point(0, 0), 0, (RGB)0x00FF00);
		break;
	case 2:
		objLaser->Draw(Point(0, 0), 90, (RGB)0x00FF00);
		break;
	case 3:
		objLaser2->Draw(Point(0, 0), 0, (RGB)0x00FF00);
		break;
	default:
		objLaser1->Draw(Point(-1, 0), (float)frame, (RGB)0x00FF00);
	}
	renderLaser->Draw(position * 24);
	level[position.Y][position.X].l = 1;
}

void PlayerLaser(void)
{
	Point vector = player.vector;
	Point position = player.position + vector;
	bool en = true;

	while (en)
	{
		switch (GetField(position, true))
		{
		case 'b':
			if (position + vector != player.position)
				switch (GetField(position + vector, true))
				{
				case 'g':
				case 'w':
				case 'e':
					for (int i = 2; i < 23; ++i)
					{
						DrawElevent(position, false);
						DrawElevent(position + vector, true);
						DrawLaser(position, (vector.X != 0) ? 1 : 2);
						renderBox->RenderImg(GetImg(position, false), Point(0, 0), 24, 24);
						objBox->Draw(Point(0, 0), 0);
						renderBox->Draw((position) * 24 + vector * i);
						kos_Pause(1);
					}
					MoveBox(position, position + vector);
				}
			en = false;
			break;
		case 'x':
			for (int i = 0; i < 23; ++i)
			{
				if (i == 11 || i == 22)
				{
					level[position.Y][position.X].l -= 1;
					if (level[position.Y][position.X].l == 2)
					{
						level[position.Y][position.X].s = 'y';
						level[position.Y][position.X].l = 0;
					}
				//	rtlDebugOutString(ftoa(level[position.Y][position.X].l));

					DrawElevent(position, false);
				}
				kos_Pause(1);
			}
			en = false;
			break;
		case 'g':
		case 'w':
		case 'y':
		case 'e':
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
					kos_Pause(2);
				}
				gameStatus = GAME_DEFEAT;
				draw_window();
				return;
			}
			else
			{
				if (level[position.Y][position.X].l == 1)
					DrawLaser(position, 3);
				else
					DrawLaser(position, (vector.X != 0) ? 1 : 2);
			}
			break;
		case '7':
			if (vector == Point(-1, 0) || vector == Point(0, -1))
			{
				vector = (vector.Y == -1) ? Point(1, 0) : Point(0, 1);
				DrawLaser(position, 0);
			}
			else
			{
				for (int i = 2; i < 23; ++i)
					kos_Pause(1);
				en = false;
			}
			break;
		case '9':
			if (vector == Point(0, -1) || vector == Point(1, 0))
			{
				vector = (vector.Y == -1) ? Point(-1, 0) : Point(0, 1);
				DrawLaser(position, 90);
			}
			else
			{
				for (int i = 2; i < 23; ++i)
					kos_Pause(1);
				en = false;
			}
			break;
		case '1':
			if (vector == Point(-1, 0) || vector == Point(0, 1))
			{
				vector = (vector.Y == 1) ? Point(1, 0) : Point(0, -1);
				DrawLaser(position, 270);
			}
			else
			{
				for (int i = 2; i < 23; ++i)
					kos_Pause(1);
				en = false;
			}
			break;
		case '3':
			if (vector == Point(1, 0) || vector == Point(0, 1))
			{
				vector = (vector.Y == 1) ? Point(-1, 0) : Point(0, -1);
				DrawLaser(position, 180);
			}
			else
			{
				for (int i = 2; i < 23; ++i)
					kos_Pause(1);
				en = false;
			}
			break;
		default:
			for (int i = 2; i < 23; ++i)
				kos_Pause(1);
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
	
}

void player_move(Point vector, float angle)
{
	if (player.vector == vector)
	{
		switch (GetField(player.position + vector, true))
		{
		case 'b':
			switch (GetField(player.position + vector * 2, true))
			{
			case 'g':
			case 'w':
			case 'e':
			case 'y':
				animation(vector, angle);
				return;
			}
			break;
		case 'f':
			gameStatus = GAME_VICTORY;
			draw_window();
			break;
		case ' ':
		case 'x':
		case 's':
		case 'w':
		case '7':
		case '9':
		case '3':
		case '1':
			break;
		default:
			animation(vector, angle);
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
			kos_Pause(1);
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
				PlayerLaser();
			break;
		case 13:
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
				if (level[y][x].s != ' ')
					kos_PutImage(GetImg(Point(x, y), true), 24, 24, 24 * x, 24 * y);
				if (level[y][x].d == 'b')
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
			//	if (block[k] != 0)
			//		rtlDebugOutString(ftoa(block[k]));
				switch (block[k])
				{
				case 0:
					levels[i].fileds[y][x] = ' ';
					break;
				case 1:
					levels[i].fileds[y][x] = 'w';
					break;
				case 2:
					levels[i].fileds[y][x] = 'g';
					break;
				case 3:
					levels[i].fileds[y][x] = 'x';
					break;
				case 4:
					levels[i].fileds[y][x] = 's';
					break;
				case 5:
					levels[i].fileds[y][x] = 'b';
					break;
				case 6:
					levels[i].fileds[y][x] = '7';
					break;
				case 7:
					levels[i].fileds[y][x] = '9';
					break;
				case 8:
					levels[i].fileds[y][x] = '3';
					break;
				case 9:
					levels[i].fileds[y][x] = '1';
					break;
				case 10:
					levels[i].fileds[y][x] = 'h';
					break;
				case 11:
					levels[i].fileds[y][x] = 'f';
					break;
				}
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
			level[y][x].d = ' ';
			level[y][x].l = 0;
			switch (levels[index].fileds[y][x])
			{
			case 'b':
				level[y][x].s = 'g';
				level[y][x].d = 'b';
				break;
			case 'h':
				player = Player(Point(x, y));
				level[y][x].s = 'g';
				break;
			case 'x':
				level[y][x].l = 6;
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
	file->LoadTex((Byte*)img_brick, 3, 24, 24);
	file->LoadTex((Byte*)img_finish, 3, 24, 24);
	file->LoadTex((Byte*)img_ground, 3, 24, 24);
	file->LoadTex((Byte*)img_laser, 4, 24, 24);	
	file->LoadTex((Byte*)img_laser1, 4, 24, 24);
	file->LoadTex((Byte*)img_laser2, 4, 24, 24);

	file->LoadTex((Byte*)img_mirror, 4, 24, 24);
	file->LoadTex((Byte*)img_mirror90, 4, 24, 24);
	file->LoadTex((Byte*)img_mirror180, 4, 24, 24);
	file->LoadTex((Byte*)img_mirror270, 4, 24, 24);

	file->LoadTex((Byte*)img_mini_mirror, 4, 24, 24);
	file->LoadTex((Byte*)img_mini_mirror90, 4, 24, 24);
	file->LoadTex((Byte*)img_mini_mirror180, 4, 24, 24);
	file->LoadTex((Byte*)img_mini_mirror270, 4, 24, 24);

	file->LoadTex((Byte*)img_tank, 4, 24, 24);
	file->LoadTex((Byte*)img_wall, 3, 24, 24);
	file->LoadTex((Byte*)img_water, 3, 24, 24);
	file->LoadTex((Byte*)img_waterbox, 3, 24, 24);
	file->LoadTex((Byte*)img_brick1, 3, 24, 24);
	file->LoadTex((Byte*)img_menu, 3, 384, 384);
	file->LoadTex((Byte*)img_button, 3, 150, 50);	
	file->LoadTex((Byte*)img_explosion, 4, 24, 336);	
	
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
	objMirror = new CKosImage(renderMirror, (RGBA*)img_mirror, 24, 24);
	objMirror90 = new CKosImage(renderMirror, (RGBA*)img_mirror90, 24, 24);
	objMirror180 = new CKosImage(renderMirror, (RGBA*)img_mirror180, 24, 24);
	objMirror270 = new CKosImage(renderMirror, (RGBA*)img_mirror270, 24, 24);

	objMiniMirror = new CKosImage(renderMirror, (RGBA*)img_mini_mirror, 24, 24);
	objMiniMirror = new CKosImage(renderMirror, (RGBA*)img_mini_mirror90, 24, 24);
	objMiniMirror = new CKosImage(renderMirror, (RGBA*)img_mini_mirror180, 24, 24);
	objMiniMirror = new CKosImage(renderMirror, (RGBA*)img_mini_mirror270, 24, 24);

	renderBox = new CKosRender(24, 24);
	objBox = new CKosImage(renderBox, (RGBA*)img_box, 24, 24);

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
