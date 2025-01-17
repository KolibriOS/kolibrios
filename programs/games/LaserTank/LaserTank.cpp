#include "smalllibc/kosSyst.h"
#include "smalllibc/kosFile.h"
#include "smalllibc/sprintf.h"
#include "smalllibc/func.h"
#include "render.h"
#include "image.h"

#define MODE_MENU		0
#define MODE_LEVELS		1
#define MODE_GAME		2
#define MODE_PAUSE		4

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
#define FIELD_WALL_X		20
#define FIELD_WALL_H		21
#define FIELD_WALL_V		22
#define FIELD_BOX_WATER		23
#define FIELD_BRICK_DES		24
#define FIELD_CRATER		25

char* header = "Laser Tank";
bool w_redraw = true;

struct Level
{
	Byte fileds[16][16];
};

void pause(int time)
{
	kos_Pause(time);
	Byte keyCode;
	Dword buttons;
	int mX, mY;
	for (int i = 0; i < 10; ++i)
	{
		kos_GetKey(keyCode);
		kos_GetMouseState(buttons, mX, mY);
	}
}

Level *levels;
int levelCount = 0;
int levelIndex = 0;
int levelPage = 0;
float clickTime = 0;

RGBA img_tank[576];
RGB img_water[576];
RGB img_brick[11][576];
RGB img_waterbox[576];
RGB img_ground[576];
RGB img_crater[576];
RGB img_wall[576];
RGB img_wall_x[576];
RGB img_wall_h[576];
RGB img_wall_v[576];
RGB img_finish[576];
RGBA img_box[576];
RGBA img_laser[576];
RGB img_mirror[4][576];
RGBA img_mini_mirror[2304];
RGBA img_laser1[576];
RGBA img_laser2[576];
RGB img_brick1[576];
RGB img_menu[147456];
RGBA img_explosion[8064];
RGBA img_gun[576];
RGB img_gamebg[9216];
RGBA img_black[576];

RGB img_pandus[3][12];

RGBA img_number_box[2550];
RGBA img_numbers[3500];
RGBA img_button1[3249];
RGBA img_button_arrow[375];

RGB img_levels[147456];

RGB img_buttons[3][13053];



Player player;

CKosRender* renderPlayer;
CKosImage* objPlayer;
CKosImage* objPlayer1;

CKosRender* renderBox;
CKosRender* renderWater;

CKosImage* objLaser;
CKosImage* objLaser1;
CKosImage* objLaser2;
CKosImage* objMiniMirror;
CKosImage* objBox;
CKosImage* objGun;
CKosImage* objExplosion; 

CKosImage* objblack;

CKosRender* renderLevels;
CKosImage* objnumber_box;
CKosImage* objnumbers;

CKosImage* objbutton1;
CKosImage* objbutton_arrow;

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
	if (position.X < 0 || position.Y < 0 || position.X > 15 || position.Y > 15)
		return FIELD_WALL;

	if (din && level[position.Y][position.X].d != FIELD_NONE)
		return level[position.Y][position.X].d;
	return level[position.Y][position.X].s;
}

void SetMode(int mode)
{
	gameMode = mode;
	draw_window();

	pause(40);
}

bool IsWater(Point pos)
{
	if (pos.X < 0 || pos.Y < 0 || pos.X > 15 || pos.Y > 15)
		return true;

	Byte code = GetField(pos, false);
	return (code == FIELD_WATER || code == FIELD_BOX_WATER);
}

void RenderPandus(Point pos)
{
	bool is[10];
	for (int y = 0; y < 3; ++y)
		for (int x = 0; x < 3; ++x)
			is[y * 3 + x + 1] = IsWater(pos + Point(x-1, y-1));
	
	if (!is[6])
	{
		if (!is[2] || is[3])
			renderWater->RenderImg((RGB*)img_pandus[0], Point(23, 0), 1, 12);
		else
			renderWater->RenderImg((RGB*)img_pandus[1], Point(23, 0), 1, 12);
		
		if (!is[8] || is[9])
			renderWater->RenderImg((RGB*)img_pandus[2], Point(23, 12), 1, 12);
		else
			renderWater->RenderImg((RGB*)img_pandus[1], Point(23, 12), 1, 12);
	}

	if (!is[2])
	{
		if (!is[4] || is[1])
			renderWater->RenderImg((RGB*)img_pandus[0], Point(0, 0), 12, 1);
		else
			renderWater->RenderImg((RGB*)img_pandus[1], Point(0, 0), 12, 1);

		if (!is[6] || is[3])
			renderWater->RenderImg((RGB*)img_pandus[2], Point(12, 0), 12, 1);
		else
			renderWater->RenderImg((RGB*)img_pandus[1], Point(12, 0), 12, 1);
	}
	
	if (!is[4])
	{
		if (!is[8] || is[7])
			renderWater->RenderImg((RGB*)img_pandus[2], Point(0, 12), 1, 12);
		else
			renderWater->RenderImg((RGB*)img_pandus[1], Point(0, 12), 1, 12);

		if (!is[2] || is[1])
			renderWater->RenderImg((RGB*)img_pandus[0], Point(0, 0), 1, 12);
		else
			renderWater->RenderImg((RGB*)img_pandus[1], Point(0, 0), 1, 12);
	}

	if (!is[8])
	{
		if (!is[6] || is[9])
			renderWater->RenderImg((RGB*)img_pandus[0], Point(12, 23), 12, 1);
		else
			renderWater->RenderImg((RGB*)img_pandus[1], Point(12, 23), 12, 1);

		if (!is[4] || is[7])
			renderWater->RenderImg((RGB*)img_pandus[2], Point(0, 23), 12, 1);
		else
			renderWater->RenderImg((RGB*)img_pandus[1], Point(0, 23), 12, 1);
	}
}

RGB* GetImg(Point position, bool din)
{
	switch (GetField(position, din))
	{
	case FIELD_WALL:
		return (RGB*)img_wall;
	case FIELD_WALL_X:
		return (RGB*)img_wall_x;
	case FIELD_WALL_H:
		return (RGB*)img_wall_h;
	case FIELD_WALL_V:
		return (RGB*)img_wall_v;
	case FIELD_MISSLE_0:
		return (RGB*)img_mirror[0];
	case FIELD_MISSLE_1:
		return (RGB*)img_mirror[1];
	case FIELD_MISSLE_2:
		return (RGB*)img_mirror[2];
	case FIELD_MISSLE_3:
		return (RGB*)img_mirror[3];
	case FIELD_BOX_MISSLE_0:
		renderBox->RenderImg(GetImg(position, false), Point(0, 0), 24, 24);
		objMiniMirror->Draw(Point(0, 0), 0, 0);
		return renderBox->buffer;
	case FIELD_BOX_MISSLE_1:
		renderBox->RenderImg(GetImg(position, false), Point(0, 0), 24, 24);
		objMiniMirror->Draw(Point(0, 0), 0, 1);
		return renderBox->buffer;
	case FIELD_BOX_MISSLE_2:
		renderBox->RenderImg(GetImg(position, false), Point(0, 0), 24, 24);
		objMiniMirror->Draw(Point(0, 0), 0, 2);
		return renderBox->buffer;
	case FIELD_BOX_MISSLE_3:
		renderBox->RenderImg(GetImg(position, false), Point(0, 0), 24, 24);
		objMiniMirror->Draw(Point(0, 0), 0, 3);
		return renderBox->buffer;
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
	case FIELD_CRATER:
		return (RGB*)img_crater;
	case FIELD_BOX:
		renderBox->RenderImg(GetImg(position, false), Point(0, 0), 24, 24);
		objBox->Draw(Point(0, 0), 0);
		return renderBox->buffer;
	case FIELD_FINISH:
		return (RGB*)img_finish;
	case FIELD_BRICK:
		return (RGB*)img_brick[level[position.Y][position.X].l];
	case FIELD_BRICK_DES:
		return (RGB*)img_brick1;
	case FIELD_BOX_WATER:
	//	return (RGB*)img_waterbox;
		renderWater->RenderImg((RGB*)img_waterbox, Point(0, 0), 24, 24);
		RenderPandus(position);
		return renderWater->buffer;
	case FIELD_WATER:
		renderWater->RenderImg((RGB*)img_water, Point(0, 0), 24, 24);
		RenderPandus(position);
		return renderWater->buffer;
	//	return (RGB*)img_water;
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
	case FIELD_CRATER:
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

void MoveElement(Point a, Point b, int element)
{
	level[a.Y][a.X].d = FIELD_NONE;
	DrawElevent(a, false);
	if (level[b.Y][b.X].s == FIELD_WATER)
	{
		if (element == FIELD_BOX)
			level[b.Y][b.X].s = FIELD_BOX_WATER;
	}
	else
		level[b.Y][b.X].d = element;
	DrawElevent(b, true);
}

void animation(Point vector, float angle, int obj)
{
	for (int i = 2; i < 23; ++i)
	{
		kos_WindowRedrawStatus(1);

		DrawElevent(player.position, false);
		DrawElevent(player.position + vector, false);

		renderPlayer->RenderImg(GetImg(player.position, false), vector * -i, 24, 24);
		renderPlayer->RenderImg(GetImg(player.position + vector, false), vector * -i + vector * 24, 24, 24);

		objPlayer->Draw(Point(0, 0), angle);
		renderPlayer->Draw(player.position * 24 + vector * i);
		if (level[player.position.Y + vector.Y][player.position.X + vector.X].d == obj)
		{
			//renderBox->RenderImg(GetImg(player.position + vector, true), Point(0, 0), 24, 24);
			renderBox->RenderImg(GetImg(player.position + vector, false), vector * -i, 24, 24);
			renderBox->RenderImg(GetImg(player.position + vector * 2, false), vector * -i + vector * 24, 24, 24);
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
			case FIELD_BOX_MISSLE_0:
				objMiniMirror->Draw(Point(0, 0), 0, 0);
				break;
			case FIELD_BOX_MISSLE_1:
				objMiniMirror->Draw(Point(0, 0), 0, 1);
				break;
			case FIELD_BOX_MISSLE_2:
				objMiniMirror->Draw(Point(0, 0), 0, 2);
				break;
			case FIELD_BOX_MISSLE_3:
				objMiniMirror->Draw(Point(0, 0), 0, 3);
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
		MoveElement(player.position + vector, player.position + vector * 2, GetField(player.position + vector, true));

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
	renderBox->RenderImg(GetImg(position, false), Point(0, 0), 24, 24);
	Byte code = GetField(position, true);
	if (code == FIELD_BOX_MISSLE_0 || code == FIELD_BOX_MISSLE_1 || code == FIELD_BOX_MISSLE_2 || code == FIELD_BOX_MISSLE_3)
		objMiniMirror->Draw(Point(0, 0), 0, code - FIELD_BOX_MISSLE_0);
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
		objLaser1->Draw(Point(0, 0), (float)frame, color);
	}
	renderBox->Draw(position * 24);
	level[position.Y][position.X].l = 1;
}

bool LaserMoveElement(Point position, Point vector, int code, RGB color)
{
	if (position + vector != player.position)
	{ 
		switch (GetField(position + vector, true))
		{
		case FIELD_GROUND:
		case FIELD_CRATER:
		case FIELD_WATER:
		case FIELD_BRICK_DES:
		case FIELD_BOX_WATER:
			for (int i = 2; i < 23; ++i)
			{
				renderBox->RenderImg(GetImg(position, false), Point(0, 0), 24, 24);
				if (vector.X != 0)
					objLaser->Draw((vector.X > 0) ? Point(i - 24, 0) : Point(24 - i, 0), 0, color);
				else
					objLaser->Draw((vector.Y > 0) ? Point(0, i - 24) : Point(0, 24 - i), 90, color);
				renderBox->Draw(position * 24);

				DrawElevent(position + vector, false);

				renderBox->RenderImg(GetImg(position, false), vector * -i, 24, 24);
				renderBox->RenderImg(GetImg(position + vector, false), vector * -i + vector * 24, 24, 24);

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
				case FIELD_BOX_MISSLE_0:
					objMiniMirror->Draw(Point(0, 0), 0, 0);
					break;
				case FIELD_BOX_MISSLE_1:
					objMiniMirror->Draw(Point(0, 0), 0, 1);
					break;
				case FIELD_BOX_MISSLE_2:
					objMiniMirror->Draw(Point(0, 0), 0, 2);
					break;
				case FIELD_BOX_MISSLE_3:
					objMiniMirror->Draw(Point(0, 0), 0, 3);
					break;
				case FIELD_BOX:
					objBox->Draw(Point(0, 0), 0);
				}
				renderBox->Draw((position)* 24 + vector * i);
				kos_Pause(1);
			}
			MoveElement(position, position + vector, code);
			return true;
		}
	}
	return false;
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
					renderBox->RenderImg(GetImg(position, false), Point(0, 0), 24, 24);
					objExplosion->Draw(Point(0, 0), 0, i);
					renderBox->Draw((position)* 24);
					pause(2);
				}
				level[position.Y][position.X].d = FIELD_NONE;
				if (level[position.Y][position.X].s == FIELD_GROUND)
					level[position.Y][position.X].s = FIELD_CRATER;
				draw_window();
				return;
			}
			else
			{
				if (!LaserMoveElement(position, vector, code, color))
				{
					for (int i = 2; i < 23; ++i)
						pause(1);
				}
				else
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
		case FIELD_WALL_X:
			break;
		case FIELD_WALL_H:
			if (vector.X == 0)
			{
				for (int i = 2; i < 23; ++i)
					pause(1);
				en = false;
			}
			break;
		case FIELD_WALL_V:
			if (vector.Y == 0)
			{
				for (int i = 2; i < 23; ++i)
					pause(1);
				en = false;
			}
			break;
		case FIELD_GROUND:
		case FIELD_CRATER:
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
					renderBox->RenderImg(GetImg(position, false), Point(0, 0), 24, 24);
					objExplosion->Draw(Point(0, 0), 0, i);
					renderBox->Draw((position)* 24);
					pause(2);
				}
				level[player.position.Y][player.position.X].s = FIELD_CRATER;
				gameStatus = GAME_DEFEAT;
				SetMode(MODE_PAUSE);
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
		case FIELD_BOX_MISSLE_0:
			if (vector == Point(-1, 0) || vector == Point(0, -1))
			{
				vector = (vector.Y == -1) ? Point(1, 0) : Point(0, 1);
				DrawLaser(position, 0, color);
			}
			else
			{
				if (!LaserMoveElement(position, vector, code, color))
				{
					for (int i = 2; i < 23; ++i)
						pause(1);
				}
				else
					LaserGun = true;
				en = false;
			}
			break;
		case FIELD_BOX_MISSLE_1:
			if (vector == Point(0, -1) || vector == Point(1, 0))
			{
				vector = (vector.Y == -1) ? Point(-1, 0) : Point(0, 1);
				DrawLaser(position, 90, color);
			}
			else
			{
				if (!LaserMoveElement(position, vector, code, color))
				{
					for (int i = 2; i < 23; ++i)
						pause(1);
				}
				else
					LaserGun = true;
				en = false;
			}
			break;
		case FIELD_BOX_MISSLE_2:
			if (vector == Point(1, 0) || vector == Point(0, 1))
			{
				vector = (vector.Y == 1) ? Point(-1, 0) : Point(0, -1);
				DrawLaser(position, 180, color);
			}
			else
			{
				if (!LaserMoveElement(position, vector, code, color))
				{
					for (int i = 2; i < 23; ++i)
						pause(1);
				}
				else
					LaserGun = true;
				en = false;
			}
			break;
		case FIELD_BOX_MISSLE_3:
			if (vector == Point(-1, 0) || vector == Point(0, 1))
			{
				vector = (vector.Y == 1) ? Point(1, 0) : Point(0, -1);
				DrawLaser(position, 270, color);
			}
			else
			{
				if (!LaserMoveElement(position, vector, code, color))
				{
					for (int i = 2; i < 23; ++i)
						pause(1);
				}
				else
					LaserGun = true;
				en = false;
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
		case FIELD_BOX_MISSLE_0:
		case FIELD_BOX_MISSLE_1:
		case FIELD_BOX_MISSLE_2:
		case FIELD_BOX_MISSLE_3:
			switch (GetField(player.position + vector * 2, true))
			{
			case FIELD_GROUND:
			case FIELD_CRATER:
			case FIELD_WATER:
			case FIELD_BOX_WATER:
			case FIELD_BRICK_DES:
				if (code == FIELD_BOX_MISSLE_0 && (vector == Point(1, 0) || vector == Point(0, 1))
					|| code == FIELD_BOX_MISSLE_1 && (vector == Point(-1, 0) || vector == Point(0, 1))
					|| code == FIELD_BOX_MISSLE_2 && (vector == Point(-1, 0) || vector == Point(0, -1))
					|| code == FIELD_BOX_MISSLE_3 && (vector == Point(1, 0) || vector == Point(0, -1))
					)
					animation(vector, angle, code);
				return;
			}
			break;
		case FIELD_GUN_0:
		case FIELD_GUN_1:
		case FIELD_GUN_2:
		case FIELD_GUN_3:
		case FIELD_BOX:		
			switch (GetField(player.position + vector * 2, true))
			{
			case FIELD_GROUND:
			case FIELD_CRATER:
			case FIELD_WATER:
			case FIELD_BOX_WATER:
			case FIELD_BRICK_DES:
				animation(vector, angle, code);
				return;
			}
			break;
		case FIELD_FINISH:
			gameStatus = GAME_VICTORY;
			SetMode(MODE_PAUSE);
			draw_window();
			break;
		case FIELD_NONE:
		case FIELD_BRICK:
		case FIELD_WALL:
		case FIELD_WALL_H:
		case FIELD_WALL_V:
		case FIELD_WALL_X:
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

void key_press(int key)
{
	sProcessInfo sPI;
	kos_ProcessInfo(&sPI);
	if (sPI.rawData[70] & 0x04)
		return;
	//rtlDebugOutString(ftoa(key));

	switch (gameMode)
	{
	case MODE_MENU:
		if (key == 27)
			kos_ExitApp();

		if (key == 13 || key == 32)
			SetMode(MODE_LEVELS);
		break;
	case MODE_LEVELS:
		if (key == 27)
			SetMode(MODE_MENU);

		if (levelPage > 0 && key == 183)
		{
			levelIndex -= 30;
			draw_window();
		}
		else
			if (levelPage < (int)(levelCount / 30) && key == 184)
			{
				levelIndex += 30;
				if (levelIndex >= levelCount)
					levelIndex = levelCount - 1;
				draw_window();
			}

	// 179, 100   D Right
		if ((key == 179 || key == 100) && (levelIndex + 1) < levelCount)
		{
			levelIndex++;
			draw_window();
		}
	// 176, 97    A  Left
		if ((key == 176 || key == 97) && levelIndex > 0)
		{
			levelIndex--;
			draw_window();
		}
	// 119, 178: W Up
		if ((key == 119 || key == 178) && (levelIndex - 6) >= 0)
		{
			levelIndex -= 6;
			draw_window();
		}
	// 177, 115    S Down
		if ((key == 177 || key == 115) && (levelIndex + 6) < levelCount)
		{
			levelIndex += 6;
			draw_window();
		}

		if (key == 13 || key == 32)
		{
			openLevel(levelIndex);
			SetMode(MODE_GAME);
		}

		break;
	case MODE_PAUSE:
		if (key == 27)
			SetMode(MODE_LEVELS);
		else
			if ((key == 32 || key == 13) && (gameStatus == GAME_NONE || (gameStatus == GAME_VICTORY && levelIndex < (levelCount - 1))))
			{
				if (gameStatus == GAME_VICTORY)
					openLevel(levelIndex + 1);
				SetMode(MODE_GAME);
			}
			else
				if (key == 114)
				{
					openLevel(levelIndex);
					SetMode(MODE_GAME);
				}
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
			//rtlDebugOutString(ftoa(rtlRand()));
			
		//	openLevel(levelIndex + 1);
			if (gameStatus == GAME_VICTORY)
				openLevel(levelIndex + 1);
			else
				if (gameStatus == GAME_DEFEAT)
					openLevel(levelIndex);

			break;
		case 27:
			SetMode(MODE_PAUSE);
		}
		break;
	}
}

void MousePress(int button, Point position)
{
	//rtlDebugOutString("Mouse");
	//rtlDebugOutString(ftoa(position.X));
	//rtlDebugOutString(ftoa(position.Y));
	Point level_pos = Point(0, 0);
	switch (gameMode)
	{
	case MODE_MENU:
		if (CollRecrVsPoint(position, ToGame.rect))
			SetMode(MODE_LEVELS);
		if (CollRecrVsPoint(position, ToExit.rect))
			kos_ExitApp();
		break;
	case MODE_LEVELS:
		if (CollRecrVsPoint(position, ToExit.rect))
			SetMode(MODE_MENU);
		else
		if (levelPage > 0 && CollRecrVsPoint(position, Rect(9, 318, 57, 57)))
		{
			levelIndex -= 30;
			draw_window();
		}
		else
			if (levelPage < (int)(levelCount / 30) && CollRecrVsPoint(position, Rect(70, 318, 57, 57)))
		{
			levelIndex += 30;
			if (levelIndex >= levelCount)
				levelIndex = levelCount - 1;
			draw_window();
		}			
		else
		{
			levelPage = (int)(levelIndex / 30);
			for (int i = levelPage * 30; i < min(levelCount, (levelPage + 1) * 30); i++)
			{

				if (i % 6 == 0 && i != levelPage * 30)
				{
					level_pos.X = 0;
					level_pos.Y++;
				}
				if (CollRecrVsPoint(position, Rect(11 + level_pos.X * 62, 11 + 61 * level_pos.Y, 51, 50)))
				{
					openLevel(i);
					//rtlDebugOutString(ftoa(i));
					SetMode(MODE_GAME);
					return;
				}
				level_pos.X++;
			}
		}
		break;
	case MODE_PAUSE:
		if (CollRecrVsPoint(position, Rect(77, 318, 229, 57)))
			SetMode(MODE_LEVELS);
		else
			if (CollRecrVsPoint(position, Rect(77, 255, 229, 57)))
			{
				openLevel(levelIndex);
				SetMode(MODE_GAME);
			}
			else
				if (CollRecrVsPoint(position, Rect(77, 192, 229, 57)) && (gameStatus == GAME_NONE || (gameStatus == GAME_VICTORY && levelIndex < (levelCount - 1))))
				{
					if (gameStatus == GAME_VICTORY)
						openLevel(levelIndex + 1);
					SetMode(MODE_GAME);
				}
	case MODE_GAME:

		break;
	}
}

void draw_level_number(Point position, int number, RGB color) // 0x252317
{
	if (number > 99)
	{
		objnumbers->Draw(position + Point(4, 12), 0, (int)(number / 100), color);
		objnumbers->Draw(position + Point(18, 12), 0, (int)((number % 100) / 10), color);
		objnumbers->Draw(position + Point(32, 12), 0, (int)(number % 10), color);
	}
	else
		if (number > 9)
		{
			objnumbers->Draw(position + Point(11, 12), 0, (int)((number % 100) / 10), color);
			objnumbers->Draw(position + Point(25, 12), 0, (int)(number % 10), color);
		}
		else
			if (number < 10)
				objnumbers->Draw(position + Point(18, 12), 0, number, color);
}

void draw_window(void)
{
	if (w_redraw)
	{
		kos_WindowRedrawStatus(1);

		//kos_DefineAndDrawWindow(50, 50, 640, 506 - 22 + kos_GetSkinHeight(), 0x74, 0xEEEEEE, 0, 0, (Dword)windowTitle);

		kos_DefineAndDrawWindow(10, 40, 384 + 9, 384 + 25, 0x74, 0x444444, 0, 0, (Dword)header);
		kos_WindowRedrawStatus(2);
		w_redraw = false;
	}

	Point level_pos = Point(0, 0);
	switch (gameMode)
	{
	case MODE_MENU:
		kos_PutImage((RGB*)img_menu, 384, 384, 0, 0);

	//	kos_PutImage((RGB*)img_button, 150, 50, ToGame.rect.X, ToGame.rect.Y);
		
		
		break;
	case MODE_LEVELS:
		renderLevels->RenderImg(img_levels, Point(0, 0), 384, 384);

		levelPage = (int)(levelIndex / 30);
		for (int i = levelPage * 30; i < min(levelCount, (levelPage + 1) * 30); i++)
		{
			if (i % 6 == 0 && i != levelPage * 30)
			{
				level_pos.X = 0;
				level_pos.Y++;
			}
			if (levelIndex != i)
			{
				objnumber_box->Draw(Point(11 + level_pos.X * 62, 11 + 61 * level_pos.Y), 0);
				draw_level_number(Point(11 + level_pos.X * 62, 11 + 61 * level_pos.Y), i + 1, (RGB)0x252317);
			}
			else
			{
				objnumber_box->Draw(Point(11 + level_pos.X * 62, 11 + 61 * level_pos.Y), 0, (RGB)0xAA0000);
				draw_level_number(Point(11 + level_pos.X * 62, 11 + 61 * level_pos.Y), i + 1, (RGB)0xAA0000);
			}

			level_pos.X++;
		}

		if (levelPage > 0)
		{
			objbutton1->Draw(Point(9, 318), 0);
			objbutton_arrow->Draw(Point(24, 338), 0);
		}

		if (levelPage < (int)(levelCount / 30))
		{
			objbutton1->Draw(Point(70, 318), 0);
			objbutton_arrow->Draw(Point(89, 339), 180);
		}

		renderLevels->Draw(Point(0, 0));
		//kos_PutImage((RGB*)img_ground, 24, 24, 100, 100);		
		break;
	case MODE_PAUSE:
			for (int y = 0; y < 4; y++)
				for (int x = 0; x < 4; x++)
					renderLevels->RenderImg((RGB*)img_gamebg, Point(96 * x, 96 * y), 96, 96);

			for (int y = 0; y < 16; y++)
				for (int x = 0; x < 16; x++)
					if (level[y][x].s != FIELD_NONE)
						renderLevels->RenderImg(GetImg(Point(x, y), true), Point(24 * x, 24 * y), 24, 24);

			switch (gameStatus)
			{
			case GAME_NONE:
				objPlayer1->Draw(player.position * 24, player.angle);
				break;
			case GAME_VICTORY:
				kos_WriteTextToWindow(30, 10, 0x80, 0xFFFFFF, "VICTORY", 0);
				break;
			case GAME_DEFEAT:
				kos_WriteTextToWindow(30, 10, 0x80, 0xFFFFFF, "DEFEAT", 0);
				break;
			}

			for (int y = 0; y < 16; y++)
				for (int x = 0; x < 16; x++)
					objblack->Draw(Point(24 * x, 24 * y), 0);

			if (gameStatus == GAME_NONE || (gameStatus == GAME_VICTORY && levelIndex < (levelCount - 1)))
				renderLevels->RenderImg((RGB*)img_buttons[1], Point(77, 192), 229, 57);
			renderLevels->RenderImg((RGB*)img_buttons[2], Point(77, 255), 229, 57);
			renderLevels->RenderImg((RGB*)img_buttons[0], Point(77, 318), 229, 57);			 

			renderLevels->Draw(Point(0, 0));
		break;
	case MODE_GAME:
		for (int y = 0; y < 4; y++)
			for (int x = 0; x < 4; x++)
				kos_PutImage((RGB*)img_gamebg, 96, 96, 96 * x, 96 * y);
		
		for (int y = 0; y < 16; y++)
			for (int x = 0; x < 16; x++)
				if (level[y][x].s != FIELD_NONE)
					kos_PutImage(GetImg(Point(x, y), true), 24, 24, 24 * x, 24 * y);	

		if(gameStatus != GAME_DEFEAT)
		{
			renderPlayer->RenderImg(GetImg(player.position, false), Point(0, 0), 24, 24);
			objPlayer->Draw(Point(0, 0), player.angle);
			renderPlayer->Draw(player.position * 24);
		}
	break;
	}
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
	strcpy(cPtr + 1, "data.lvl");
	
	CKosFile *file = new CKosFile(kosExePath);

	Byte block[256];
	while (file->Read(block, 256) == 256)
	{
		levelCount++;
	}
	//levelCount++;
	//rtlDebugOutString(ftoa(levelCount));

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
			case FIELD_BOX_MISSLE_0:
			case FIELD_BOX_MISSLE_1:
			case FIELD_BOX_MISSLE_2:
			case FIELD_BOX_MISSLE_3:
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
	//rtlDebugOutString(" ");
	//rtlDebugOutString("kos_Main");
	char *cPtr;
	cPtr = strrchr(kosExePath, '/');
	// проверка ;)
	if (cPtr == NULL)
	{
		rtlDebugOutString("Invalid path to executable.");
		return;
	}
	cPtr[1] = 0;
	strcpy(cPtr + 1, "data01.pak");
	
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

	//for (int i = 0; i < 4; ++i)
	file->LoadTex((Byte*)img_mini_mirror, 4, 24, 96);

	file->LoadTex((Byte*)img_tank, 4, 24, 24);
	file->LoadTex((Byte*)img_wall, 3, 24, 24);
	file->LoadTex((Byte*)img_water, 3, 24, 24);
	file->LoadTex((Byte*)img_waterbox, 3, 24, 24);
	file->LoadTex((Byte*)img_menu, 3, 384, 384);
	file->LoadTex((Byte*)img_explosion, 4, 24, 336);	
	file->LoadTex((Byte*)img_gun, 4, 24, 24);	
	file->LoadTex((Byte*)img_gamebg, 3, 96, 96);

	
	delete file;

	strcpy(cPtr + 1, "data02.pak");
	
	file = new CKosFile(kosExePath);

	file->LoadTex((Byte*)img_levels, 3, 384, 384);

	for (int i = 0; i < 3; ++i)
		file->LoadTex((Byte*)img_pandus[i], 3, 12, 1);

	file->LoadTex((Byte*)img_number_box, 4, 51, 50);
	file->LoadTex((Byte*)img_numbers, 4, 14, 250);
	
	file->LoadTex((Byte*)img_button1, 4, 57, 57);
	file->LoadTex((Byte*)img_button_arrow, 4, 25, 15);

	file->LoadTex((Byte*)img_wall_h, 3, 24, 24);
	file->LoadTex((Byte*)img_wall_v, 3, 24, 24);
	file->LoadTex((Byte*)img_wall_x, 3, 24, 24);
	
	file->LoadTex((Byte*)img_crater, 3, 24, 24);

	file->LoadTex((Byte*)img_black, 4, 24, 24);

	for (int i = 0; i < 3; ++i)
		file->LoadTex((Byte*)img_buttons[i], 3, 229, 57);	


	delete file;

	renderPlayer = new CKosRender(24, 24);
	objPlayer = new CKosImage(renderPlayer, (RGBA*)img_tank, 24, 24);

	renderWater = new CKosRender(24, 24);

	renderBox = new CKosRender(24, 24);
	objLaser = new CKosImage(renderBox, (RGBA*)img_laser, 24, 24);
	objLaser->SetMode(DRAW_ALPHA_ADD);
	objLaser1 = new CKosImage(renderBox, (RGBA*)img_laser1, 24, 24);
	objLaser1->SetMode(DRAW_ALPHA_ADD);
	objLaser2 = new CKosImage(renderBox, (RGBA*)img_laser2, 24, 24);
	objLaser2->SetMode(DRAW_ALPHA_ADD);
			
	//for (int i = 0; i < 4; ++i)
	objMiniMirror = new CKosImage(renderBox, (RGBA*)img_mini_mirror, 24, 24);
	objMiniMirror->SetFrameSize(24, 24);

	objBox = new CKosImage(renderBox, (RGBA*)img_box, 24, 24);
	objGun = new CKosImage(renderBox, (RGBA*)img_gun, 24, 24);	

	objExplosion = new CKosImage(renderBox, (RGBA*)img_explosion, 24, 24);
	objExplosion->SetFrameSize(24, 24);

	renderLevels = new CKosRender(384, 384);

	objPlayer1 = new CKosImage(renderLevels, (RGBA*)img_tank, 24, 24);
	objnumber_box = new CKosImage(renderLevels, (RGBA*)img_number_box, 51, 50);
	objnumbers = new CKosImage(renderLevels, (RGBA*)img_numbers, 14, 25);
	objnumbers->SetFrameSize(14, 25);

	objbutton1 = new CKosImage(renderLevels, (RGBA*)img_button1, 57, 57);
	objbutton_arrow = new CKosImage(renderLevels, (RGBA*)img_button_arrow, 25, 15);
	
	objblack = new CKosImage(renderLevels, (RGBA*)img_black, 24, 24);

	LevelsLoad();

	openLevel(0);

	kos_SetMaskForEvents(0x27);
	for (;;)
	{
		switch (kos_WaitForEvent())
		{
		case 1:
			w_redraw = true;
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
