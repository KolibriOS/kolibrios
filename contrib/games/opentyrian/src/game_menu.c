/*
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) 2007-2009  The OpenTyrian Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "game_menu.h"

#include "backgrnd.h"
#include "config.h"
#include "file.h"
#include "fonthand.h"
#include "joystick.h"
#include "keyboard.h"
#include "loudness.h"
#include "mainint.h"
#include "mouse.h"
#include "musmast.h"
#include "network.h"
#include "nortsong.h"
#include "nortvars.h"
#include "params.h"
#include "pcxmast.h"
#include "picload.h"
#include "player.h"
#include "shots.h"
#include "sprite.h"
#include "tyrian2.h"
#include "varz.h"
#include "vga256d.h"
#include "video.h"

#include <assert.h>

/*** Structs ***/
struct cube_struct
{
	char title[81];
	char header[13];
	int face_sprite;
	char text[90][36];
	int last_line;
};

/*** Globals ***/
static int joystick_config = 0; // which joystick is being configured in menu

static JE_word yLoc;
static JE_shortint yChg;
static int newPal, curPal, oldPal;
static JE_boolean quikSave;
static JE_byte oldMenu;
static JE_boolean backFromHelp;
static JE_integer lastDirection;
static JE_boolean firstMenu9, paletteChanged;
static JE_MenuChoiceType menuChoices;
static JE_integer col, colC;
static JE_byte lastCurSel;
static JE_integer curMenu;
static JE_byte curSel[MENU_MAX]; /* [1..maxmenu] */
static JE_byte curItemType, curItem, cursor;
static JE_boolean leftPower, rightPower, rightPowerAfford;
static JE_byte currentCube;
static JE_boolean keyboardUsed;

static JE_byte planetAni, planetAniWait;
static JE_byte currentDotNum, currentDotWait;
static JE_real navX, navY, newNavX, newNavY;
static JE_integer tempNavX, tempNavY;
static JE_byte planetDots[5]; /* [1..5] */
static JE_integer planetDotX[5][10], planetDotY[5][10]; /* [1..5, 1..10] */
static PlayerItems old_items[2];  // TODO: should not be global if possible

static struct cube_struct cube[4];

static const JE_MenuChoiceType menuChoicesDefault = { 7, 9, 8, 0, 0, 11, (SAVE_FILES_NUM / 2) + 2, 0, 0, 6, 4, 6, 7, 5 };
static const JE_byte menuEsc[MENU_MAX] = { 0, 1, 1, 1, 2, 3, 3, 1, 8, 0, 0, 11, 3, 0 };
static const JE_byte itemAvailMap[7] = { 1, 2, 3, 9, 4, 6, 7 };
static const JE_word planetX[21] = { 200, 150, 240, 300, 270, 280, 320, 260, 220, 150, 160, 210, 80, 240, 220, 180, 310, 330, 150, 240, 200 };
static const JE_word planetY[21] = {  40,  90,  90,  80, 170,  30,  50, 130, 120, 150, 220, 200, 80,  50, 160,  10,  55,  55,  90,  90,  40 };
static const uint cube_line_chars = sizeof(*cube->text) - 1;
static const uint cube_line_width = 150;


/*** Functions ***/
static uint *playeritem_map( PlayerItems *items, uint i )
{
	uint * const map[] = { &items->ship, &items->weapon[FRONT_WEAPON].id, &items->weapon[REAR_WEAPON].id, &items->shield, &items->generator, &items->sidekick[LEFT_SIDEKICK], &items->sidekick[RIGHT_SIDEKICK] };
	assert(i < COUNTOF(map));
	return map[i];
}


JE_longint JE_cashLeft( void )
{
	JE_longint tempL = player[0].cash;
	JE_word itemNum = *playeritem_map(&player[0].items, curSel[1] - 2);

	tempL -= JE_getCost(curSel[1], itemNum);

	tempW = 0;

	switch (curSel[1])
	{
	case 3:
	case 4:
		for (uint i = 1; i < player[0].items.weapon[curSel[1]-3].power; ++i)
		{
			tempW += weaponPort[itemNum].cost * i;
			tempL -= tempW;
		}
		break;
	}

	return tempL;
}

void JE_itemScreen( void )
{
	bool quit = false;

	/* SYN: Okay, here's the menu numbers. All are reindexed by -1 from the original code.
		0: full game menu
		1: upgrade ship main
		2: full game options
		3: play next level
		4: upgrade ship submenus
		5: keyboard settings
		6: load/save menu
		7: data cube menu
		8: read data cube
		9: 2 player arcade game menu
		10: 1 player arcade game menu
		11: network game options
		12: joystick settings
		13: super tyrian
	*/

	free_sprite2s(&shapes6);
	JE_loadCompShapes(&shapes6, '1');  // item sprites

	load_cubes();

	VGAScreen = VGAScreenSeg;

	memcpy(menuChoices, menuChoicesDefault, sizeof(menuChoices));

	play_song(songBuy);

	JE_loadPic(VGAScreen, 1, false);

	curPal = 1;
	newPal = 0;

	JE_showVGA();

	set_palette(colors, 0, 255);

	col = 1;
	gameLoaded = false;
	curItemType = 1;
	cursor = 1;
	curItem = 0;

	for (unsigned int i = 0; i < COUNTOF(curSel); ++i)
		curSel[i] = 2;

	curMenu = 0;

	int temp_weapon_power[7]; // assumes there'll never be more than 6 weapons to choose from, 7th is "Done"

	/* JE: (* Check for where Pitems and Select match up - if no match then add to the itemavail list *) */
	for (int i = 0; i < 7; i++)
	{
		int item = *playeritem_map(&player[0].last_items, i);

		int slot = 0;

		for ( ; slot < itemAvailMax[itemAvailMap[i]-1]; ++slot)
		{
			if (itemAvail[itemAvailMap[i]-1][slot] == item)
				break;
		}

		if (slot == itemAvailMax[itemAvailMap[i]-1])
		{
			itemAvail[itemAvailMap[i]-1][slot] = item;
			itemAvailMax[itemAvailMap[i]-1]++;
		}
	}

	memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->pitch * VGAScreen2->h);

	keyboardUsed = false;
	firstMenu9 = false;
	backFromHelp = false;

	/* JE: Sort items in merchant inventory */
	for (int x = 0; x < 9; x++)
	{
		if (itemAvailMax[x] > 1)
		{
			for (temp = 0; temp < itemAvailMax[x] - 1; temp++)
			{
				for (temp2 = temp; temp2 < itemAvailMax[x]; temp2++)
				{
					if (itemAvail[x][temp] == 0 || (itemAvail[x][temp] > itemAvail[x][temp2] && itemAvail[x][temp2] != 0))
					{
						temp3 = itemAvail[x][temp];
						itemAvail[x][temp] = itemAvail[x][temp2];
						itemAvail[x][temp2] = temp3;
					}
				}
			}
		}
	}

	do
	{
		quit = false;

		JE_getShipInfo();

		/* JE: If curMenu==1 and twoPlayerMode is on, then force move to menu 10 */
		if (curMenu == 0)
		{
			if (twoPlayerMode)
				curMenu = 9;

			if (isNetworkGame || onePlayerAction)
				curMenu = 10;

			if (superTyrian)
				curMenu = 13;
		}

		paletteChanged = false;

		leftPower = false;
		rightPower = false;

		/* SYN: note reindexing... "firstMenu9" refers to Menu 8 here :( */
		if (curMenu != 8 || firstMenu9)
		{
			memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->pitch * VGAScreen->h);
		}

		if (curMenu == 1 && (curSel[curMenu] == 3 || curSel[curMenu] == 4))
		{
			// reset temp_weapon_power[] every time we select upgrading front or back
			const uint item       = player[0].items.weapon[curSel[1] - 3].id,
			           item_power = player[0].items.weapon[curSel[1] - 3].power,
			           i = curSel[1] - 2;  // 1 or 2 (front or rear)

			// set power level of owned weapon
			for (int slot = 0; slot < itemAvailMax[itemAvailMap[i]-1]; ++slot)
			{
				if (itemAvail[itemAvailMap[i]-1][slot] == item)
					temp_weapon_power[slot] = item_power;
				else
					temp_weapon_power[slot] = 1;
			}

			// set power level for "Done"
			temp_weapon_power[itemAvailMax[itemAvailMap[i]-1]] = item_power;
		}

		/* play next level menu */
		if (curMenu == 3)
		{
			planetAni = 0;
			keyboardUsed = false;
			currentDotNum = 0;
			currentDotWait = 8;
			planetAniWait = 3;
			JE_updateNavScreen();
		}

		/* Draw menu title for everything but upgrade ship submenus */
		if (curMenu != 4)
		{
			JE_drawMenuHeader();
		}

		/* Draw menu choices for simple menus */
		if ((curMenu >= 0 && curMenu <= 3) || (curMenu >= 9 && curMenu <= 11) || curMenu == 13)
		{
			JE_drawMenuChoices();
		}

		/* Data cube icons */
		if (curMenu == 0)
		{
			for (int i = 1; i <= cubeMax; i++)
			{
				blit_sprite_dark(VGAScreen, 190 + i * 18 + 2, 37 + 1, OPTION_SHAPES, 34, false);
				blit_sprite(VGAScreen, 190 + i * 18, 37, OPTION_SHAPES, 34);  // data cube
			}
		}

		/* load/save menu */
		if (curMenu == 6)
		{
			int min, max;

			if (twoPlayerMode)
			{
				min = 13;
				max = 24;
			}
			else
			{
				min = 2;
				max = 13;
			}

			for (int x = min; x <= max; x++)
			{
				/* Highlight if current selection */
				temp2 = (x - min + 2 == curSel[curMenu]) ? 15 : 28;

				/* Write save game slot */
				if (x == max)
					strcpy(tempStr, miscText[6-1]);
				else if (saveFiles[x-2].level == 0)
					strcpy(tempStr, miscText[3-1]);
				else
					strcpy(tempStr, saveFiles[x-2].name);

				int tempY = 38 + (x - min)*11;

				JE_textShade(VGAScreen, 163, tempY, tempStr, temp2 / 16, temp2 % 16 - 8, DARKEN);

				/* If selected with keyboard, move mouse pointer to match? Or something. */
				if (x - min + 2 == curSel[curMenu])
				{
					if (keyboardUsed)
						set_mouse_position(305, 38 + (x - min) * 11);
				}

				if (x < max) /* x == max isn't a save slot */
				{
					/* Highlight if current selection */
					temp2 = (x - min + 2 == curSel[curMenu]) ? 252 : 250;

					if (saveFiles[x-2].level == 0)
					{
						strcpy(tempStr, "-----"); /* Empty save slot */
					}
					else
					{
						char buf[20];

						strcpy(tempStr, saveFiles[x-2].levelName);

						snprintf(buf, sizeof buf, "%s%d", miscTextB[1-1], saveFiles[x-2].episode);
						JE_textShade(VGAScreen, 297, tempY, buf, temp2 / 16, temp2 % 16 - 8, DARKEN);
					}

					JE_textShade(VGAScreen, 245, tempY, tempStr, temp2 / 16, temp2 % 16 - 8, DARKEN);
				}

				JE_drawMenuHeader();
			}
		}

		/* keyboard settings menu */
		if (curMenu == 5)
		{
			for (int x = 2; x <= 11; x++)
			{
				if (x == curSel[curMenu])
				{
					temp2 = 15;
					if (keyboardUsed)
						set_mouse_position(305, 38 + (x - 2) * 12);
				}
				else
				{
					temp2 = 28;
				}

				JE_textShade(VGAScreen, 166, 38 + (x - 2)*12, menuInt[curMenu + 1][x-1], temp2 / 16, temp2 % 16 - 8, DARKEN);

				if (x < 10) /* 10 = reset to defaults, 11 = done */
				{
					temp2 = (x == curSel[curMenu]) ? 252 : 250;
					JE_textShade(VGAScreen, 236, 38 + (x - 2)*12, SDL_GetKeyName(keySettings[x-2]), temp2 / 16, temp2 % 16 - 8, DARKEN);
				}
			}

			menuChoices[5] = 11;
		}

		/* Joystick settings menu */
		if (curMenu == 12)
		{
			const char *menu_item[] =
			{
				"JOYSTICK",
				"ANALOG AXES",
				" SENSITIVITY",
				" THRESHOLD",
				menuInt[6][1],
				menuInt[6][4],
				menuInt[6][2],
				menuInt[6][3],
				menuInt[6][5],
				menuInt[6][6],
				menuInt[6][7],
				menuInt[6][8],
				"MENU",
				"PAUSE",
				menuInt[6][9],
				menuInt[6][10]
			};

			for (uint i = 0; i < COUNTOF(menu_item); i++)
			{
				int temp = (i == curSel[curMenu] - 2u) ? 15 : 28;

				JE_textShade(VGAScreen, 166, 38 + i * 8, menu_item[i], temp / 16, temp % 16 - 8, DARKEN);

				temp = (i == curSel[curMenu] - 2u) ? 252 : 250;

				char value[30] = "";
				if (joysticks == 0 && i < 14) // no joysticks, everything disabled
				{
					sprintf(value, "-");
				}
				else if (i == 0) // joystick number
				{
					sprintf(value, "%d", joystick_config + 1);
				}
				else if (i == 1) // joystick is analog
				{
					sprintf(value, "%s", joystick[joystick_config].analog ? "TRUE" : "FALSE");
				}
				else if (i < 4)  // joystick analog settings
				{
					if (!joystick[joystick_config].analog)
						temp -= 3;
					sprintf(value, "%d", i == 2 ? joystick[joystick_config].sensitivity : joystick[joystick_config].threshold);
				}
				else if (i < 14) // assignments
				{
					joystick_assignments_to_string(value, sizeof(value), joystick[joystick_config].assignment[i - 4]);
				}

				JE_textShade(VGAScreen, 236, 38 + i * 8, value, temp / 16, temp % 16 - 8, DARKEN);
			}

			menuChoices[curMenu] = COUNTOF(menu_item) + 1;
		}

		/* Upgrade weapon submenus, with weapon sim */
		if (curMenu == 4)
		{
			/* Move cursor until we hit either "Done" or a weapon the player can afford */
			while (curSel[4] < menuChoices[4] && JE_getCost(curSel[1], itemAvail[itemAvailMap[curSel[1]-2]-1][curSel[4]-2]) > player[0].cash)
			{
				curSel[4] += lastDirection;
				if (curSel[4] < 2)
					curSel[4] = menuChoices[4];
				else if (curSel[4] > menuChoices[4])
					curSel[4] = 2;
			}

			if (curSel[4] == menuChoices[4])
			{
				/* If cursor on "Done", use previous weapon */
				*playeritem_map(&player[0].items, curSel[1] - 2) = *playeritem_map(&old_items[0], curSel[1] - 2);
			}
			else
			{
				/* Otherwise display the selected weapon */
				*playeritem_map(&player[0].items, curSel[1] - 2) = itemAvail[itemAvailMap[curSel[1]-2]-1][curSel[4]-2];
			}

			/* Get power level info for front and rear weapons */
			if ((curSel[1] == 3 && curSel[4] < menuChoices[4]) || (curSel[1] == 4 && curSel[4] < menuChoices[4]-1))
			{
				const uint port = curSel[1] - 3,  // 0 or 1 (front or back)
				           item_level = player[0].items.weapon[port].power;

				// calculate upgradeCost
				JE_getCost(curSel[1], itemAvail[itemAvailMap[curSel[1]-2]-1][curSel[5]-2]);

				leftPower  = item_level > 1;  // can downgrade
				rightPower = item_level < 11; // can upgrade

				if (rightPower)
					rightPowerAfford = JE_cashLeft() >= upgradeCost; // can afford upgrade
			}
			else
			{
				/* Nothing else can be upgraded / downgraded */
				leftPower = false;
				rightPower = false;
			}

			/* submenu title  e.g., "Left Sidekick" */
			JE_dString(VGAScreen, 74 + JE_fontCenter(menuInt[2][curSel[1]-1], FONT_SHAPES), 10, menuInt[2][curSel[1]-1], FONT_SHAPES);

			/* Iterate through all submenu options */
			for (tempW = 1; tempW < menuChoices[curMenu]; tempW++)
			{
				int tempY = 40 + (tempW-1) * 26; /* Calculate y position */
				uint temp_cost;

				/* Is this a item or None/DONE? */
				if (tempW < menuChoices[4] - 1)
				{
					/* Get base cost for choice */
					temp_cost = JE_getCost(curSel[1], itemAvail[itemAvailMap[curSel[1]-2]-1][tempW-1]);
				}
				else
				{
					/* "None" is free :) */
					temp_cost = 0;
				}

				int afford_shade = (temp_cost > player[0].cash) ? 4 : 0;  // can player afford current weapon at all

				temp = itemAvail[itemAvailMap[curSel[1]-2]-1][tempW-1]; /* Item ID */
				switch (curSel[1]-1)
				{
					case 1: /* ship */
						if (temp > 90)
						{
							snprintf(tempStr, sizeof(tempStr), "Custom Ship %d", temp - 90);
						} else {
							strcpy(tempStr, ships[temp].name);
						}
						break;
					case 2: /* front and rear weapon */
					case 3:
						strcpy(tempStr, weaponPort[temp].name);
						break;
					case 4: /* shields */
						strcpy(tempStr, shields[temp].name);
						break;
					case 5: /* generator */
						strcpy(tempStr, powerSys[temp].name);
						break;
					case 6: /* sidekicks */
					case 7:
						strcpy(tempStr, options[temp].name);
						break;
				}
				if (tempW == curSel[curMenu]-1)
				{
					if (keyboardUsed)
					{
						set_mouse_position(305, tempY + 10);
					}
					temp2 = 15;
				} else {
					temp2 = 28;
				}

				JE_getShipInfo();

				/* item-owned marker */
				if (temp == *playeritem_map(&old_items[0], curSel[1] - 2) && temp != 0 && tempW != menuChoices[curMenu]-1)
				{
					fill_rectangle_xy(VGAScreen, 160, tempY+7, 300, tempY+11, 227);
					blit_sprite2(VGAScreen, 298, tempY+2, shapes6, 247);
				}

				/* Draw DONE */
				if (tempW == menuChoices[curMenu]-1)
				{
					strcpy(tempStr, miscText[13]);
				}
				JE_textShade(VGAScreen, 185, tempY, tempStr, temp2 / 16, temp2 % 16 - 8 - afford_shade, DARKEN);

				/* Draw icon if not DONE. NOTE: None is a normal item with a blank icon. */
				if (tempW < menuChoices[curMenu]-1)
				{
					JE_drawItem(curSel[1]-1, temp, 160, tempY-4);
				}

				/* Make selected text brigther */
				temp2 = (tempW == curSel[curMenu]-1) ? 15 : 28;

				/* Draw Cost: if it's not the DONE option */
				if (tempW != menuChoices[curMenu]-1)
				{
					char buf[20];

					snprintf(buf, sizeof buf, "Cost: %d", temp_cost);
					JE_textShade(VGAScreen, 187, tempY+10, buf, temp2 / 16, temp2 % 16 - 8 - afford_shade, DARKEN);
				}
			}
		} /* /weapon upgrade */

		/* Draw current money and shield/armor bars, when appropriate */
		/* YKS: Ouch */
		if (((curMenu <= 2 || curMenu == 5 || curMenu == 6 || curMenu >= 10) && !twoPlayerMode) || (curMenu == 4 && (curSel[1] >= 1 && curSel[1] <= 6)))
		{
			if (curMenu != 4)
			{
				char buf[20];

				snprintf(buf, sizeof buf, "%lu", player[0].cash);
				JE_textShade(VGAScreen, 65, 173, buf, 1, 6, DARKEN);
			}
			JE_barDrawShadow(VGAScreen, 42, 152, 3, 14, player[0].armor, 2, 13);
			JE_barDrawShadow(VGAScreen, 104, 152, 2, 14, shields[player[0].items.shield].mpwr * 2, 2, 13);
		}

		/* Draw crap on the left side of the screen, i.e. two player scores, ship graphic, etc. */
		if (((curMenu >= 0 && curMenu <= 2) || curMenu == 5 || curMenu == 6 || curMenu >= 9) || (curMenu == 4 && (curSel[1] == 2 || curSel[1] == 5)))
		{
			if (twoPlayerMode)
			{
				char buf[50];

				for (uint i = 0; i < 2; ++i)
				{
					snprintf(buf, sizeof(buf), "%s %lu", miscText[40 + i], player[i].cash);
					JE_textShade(VGAScreen, 25, 50 + 10 * i, buf, 15, 0, FULL_SHADE);
				}
			}
			else if (superArcadeMode != SA_NONE || superTyrian)
			{
				helpBoxColor = 15;
				helpBoxBrightness = 4;
				if (!superTyrian)
					JE_helpBox(VGAScreen, 35, 25, superShips[superArcadeMode], 18);
				else
					JE_helpBox(VGAScreen, 35, 25, superShips[SA+3], 18);
				helpBoxBrightness = 1;

				JE_textShade(VGAScreen, 25, 50, superShips[SA+1], 15, 0, FULL_SHADE);
				JE_helpBox(VGAScreen,   25, 60, weaponPort[player[0].items.weapon[FRONT_WEAPON].id].name, 22);
				JE_textShade(VGAScreen, 25, 120, superShips[SA+2], 15, 0, FULL_SHADE);
				JE_helpBox(VGAScreen,   25, 130, special[player[0].items.special].name, 22);
			}
			else
			{
				draw_ship_illustration();
			}
		}

		/* Changing the volume? */
		if ((curMenu == 2) || (curMenu == 11))
		{
			JE_barDrawShadow(VGAScreen, 225, 70, 1, music_disabled ? 12 : 16, tyrMusicVolume / 12, 3, 13);
			JE_barDrawShadow(VGAScreen, 225, 86, 1, samples_disabled ? 12 : 16, fxVolume / 12, 3, 13);
		}

		/* 7 is data cubes menu, 8 is reading a data cube, "firstmenu9" refers to menu 8 because of reindexing */
		if (curMenu == 7 || ( curMenu == 8 && (firstMenu9 || backFromHelp) ) )
		{
			firstMenu9 = false;
			menuChoices[7] = cubeMax + 2;
			fill_rectangle_xy(VGAScreen, 1, 1, 145, 170, 0);

			blit_sprite(VGAScreenSeg, 1, 1, OPTION_SHAPES, 20); /* Portrait area background */

			if (curMenu == 7)
			{
				if (cubeMax == 0)
				{
					JE_helpBox(VGAScreen, 166, 80, miscText[16 - 1], 30);
					tempW = 160;
					temp2 = 252;
				}
				else
				{
					for (int x = 1; x <= cubeMax; x++)
					{
						JE_drawCube(VGAScreenSeg, 166, 38 + (x - 1) * 28, 13, 0);
						if (x + 1 == curSel[curMenu])
						{
							if (keyboardUsed)
								set_mouse_position(305, 38 + (x - 1) * 28 + 6);
							temp2 = 252;
						}
						else
						{
							temp2 = 250;
						}

						helpBoxColor = temp2 / 16;
						helpBoxBrightness = (temp2 % 16) - 8;
						helpBoxShadeType = DARKEN;
						JE_helpBox(VGAScreen, 192, 44 + (x - 1) * 28, cube[x - 1].title, 24);
					}
					int x = cubeMax + 1;
					if (x + 1 == curSel[curMenu])
					{
						if (keyboardUsed)
							set_mouse_position(305, 38 + (x - 1) * 28 + 6);
						temp2 = 252;
					}
					else
					{
						temp2 = 250;
					}
					tempW = 44 + (x - 1) * 28;
				}

				JE_textShade(VGAScreen, 172, tempW, miscText[6 - 1], temp2 / 16, (temp2 % 16) - 8, DARKEN);
			}

			if (curSel[7] < menuChoices[7])
			{
				const int face_sprite = cube[curSel[7] - 2].face_sprite;

				if (face_sprite != -1)
				{
					const int face_x = 77 - (sprite(FACE_SHAPES, face_sprite)->width / 2),
					          face_y = 92 - (sprite(FACE_SHAPES, face_sprite)->height / 2);

					blit_sprite(VGAScreenSeg, face_x, face_y, FACE_SHAPES, face_sprite);  // datacube face

					// modify pallete for face
					paletteChanged = true;
					temp2 = facepal[face_sprite];
					newPal = 0;

					for (temp = 1; temp <= 255 - (3 * 16); temp++)
						colors[temp] = palettes[temp2][temp];
				}
			}
		}

		/* 2 player input devices */
		if (curMenu == 9)
		{
			for (uint i = 0; i < COUNTOF(inputDevice); i++)
			{
				if (inputDevice[i] > 2 + joysticks)
					inputDevice[i] = inputDevice[i == 0 ? 1 : 0] == 1 ? 2 : 1;

				char temp[64];
				if (joysticks > 1 && inputDevice[i] > 2)
					sprintf(temp, "%s %d", inputDevices[2], inputDevice[i] - 2);
				else
					sprintf(temp, "%s", inputDevices[inputDevice[i] - 1]);
				JE_dString(VGAScreen, 186, 38 + 2 * (i + 1) * 16, temp, SMALL_FONT_SHAPES);
			}
		}

		/* JE: { - Step VI - Help text for current cursor location } */

		flash = false;

		/* JE: {Reset player weapons} */
		memset(shotMultiPos, 0, sizeof(shotMultiPos));

		JE_drawScore();

		JE_drawMainMenuHelpText();

		if (newPal > 0) /* can't reindex this :( */
		{
			curPal = newPal;
			memcpy(colors, palettes[newPal - 1], sizeof(colors));
			set_palette(palettes[newPal - 1], 0, 255);
			newPal = 0;
		}

		/* datacube title under face */
		if ( ( (curMenu == 7) || (curMenu == 8) ) && (curSel[7] < menuChoices[7]) )
			JE_textShade (VGAScreen, 75 - JE_textWidth(cube[curSel[7] - 2].header, TINY_FONT) / 2, 173, cube[curSel[7] - 2].header, 14, 3, DARKEN);

		/* SYN: Everything above was just drawing the screen. In the rest of it, we process
		   any user input (and do a few other things) */

		/* SYN: Let's start by getting fresh events from SDL */
		service_SDL_events(true);

		if (constantPlay)
		{
			mainLevel = mapSection[mapPNum-1];
			jumpSection = true;
		}
		else
		{
			do
			{
			/* Inner loop -- this handles animations on menus that need them and handles
			   some keyboard events. Events it can't handle end the loop and fall through
			   to the main keyboard handler below.

			   Also, I think all timing is handled in here. Somehow. */

				NETWORK_KEEP_ALIVE();

				mouseCursor = 0;

				col += colC;
				if (col < -2 || col > 6)
				{
					colC = (-1 * colC);
				}

				// data cube reading
				if (curMenu == 8)
				{
					if (mouseX > 164 && mouseX < 299 && mouseY > 47 && mouseY < 153)
					{
						if (mouseY > 100)
							mouseCursor = 2;
						else
							mouseCursor = 1;
					}

					fill_rectangle_xy(VGAScreen, 160, 49, 310, 158, 228);
					if (yLoc + yChg < 0)
					{
						yChg = 0;
						yLoc = 0;
					}

					yLoc += yChg;
					temp = yLoc / 12;
					temp2 = yLoc % 12;
					tempW = 38 + 12 - temp2;
					temp3 = cube[curSel[7] - 2].last_line;

					for (int x = temp + 1; x <= temp + 10; x++)
					{
						if (x <= temp3)
						{
							JE_outTextAndDarken(VGAScreen, 161, tempW, cube[curSel[7] - 2].text[x-1], 14, 3, TINY_FONT);
							tempW += 12;
						}
					}

					fill_rectangle_xy(VGAScreen, 160, 39, 310, 48, 228);
					fill_rectangle_xy(VGAScreen, 160, 157, 310, 166, 228);

					int percent_read = (cube[currentCube].last_line <= 9)
					                   ? 100
					                   : (yLoc * 100) / ((cube[currentCube].last_line - 9) * 12);

					char buf[20];
					snprintf(buf, sizeof(buf), "%s %d%%", miscText[11], percent_read);
					JE_outTextAndDarken(VGAScreen, 176, 160, buf, 14, 1, TINY_FONT);

					JE_dString(VGAScreen, 260, 160, miscText[12], SMALL_FONT_SHAPES);

					if (temp2 == 0)
						yChg = 0;

					JE_mouseStart();

					JE_showVGA();

					if (backFromHelp)
					{
						fade_palette(colors, 10, 0, 255);
						backFromHelp = false;
					}
					JE_mouseReplace();

					setjasondelay(1);
				}
				else
				{
					/* current menu is not 8 (read data cube) */

					if (curMenu == 3)
					{
						JE_updateNavScreen();
						JE_drawMainMenuHelpText();
						JE_drawMenuHeader();
						JE_drawMenuChoices();
						if (extraGame)
							JE_dString(VGAScreen, 170, 140, miscText[68 - 1], FONT_SHAPES);
					}

					if (curMenu == 7 && curSel[7] < menuChoices[7])
					{
						/* Draw flashy cube */
						blit_sprite_hv_blend(VGAScreenSeg, 166, 38 + (curSel[7] - 2) * 28, OPTION_SHAPES, 25, 13, col);
					}

					/* IF (curmenu = 5) AND (cursel [2] IN [3, 4, 6, 7, 8]) */
					if (curMenu == 4 && ( curSel[1] == 3 || curSel[1] == 4 || ( curSel[1] >= 6 && curSel[1] <= 8) ) )
					{
						setjasondelay(3);
						JE_weaponSimUpdate();
						JE_drawScore();
						service_SDL_events(false);

						if (newPal > 0)
						{
							curPal = newPal;
							set_palette(palettes[newPal - 1], 0, 255);
							newPal = 0;
						}

						JE_mouseStart();

						if (paletteChanged)
						{
							set_palette(colors, 0, 255);
							paletteChanged = false;
						}

						JE_showVGA(); /* SYN: This is where it updates the screen for the weapon sim */

						if (backFromHelp)
						{
							fade_palette(colors, 10, 0, 255);
							backFromHelp = false;
						}

						JE_mouseReplace();

					} else { /* current menu is anything but weapon sim or datacube */

						setjasondelay(2);

						JE_drawScore();
						//JE_waitRetrace();  didn't do anything anyway?

						if (newPal > 0)
						{
							curPal = newPal;
							set_palette(palettes[newPal - 1], 0, 255);
							newPal = 0;
						}

						JE_mouseStart();

						if (paletteChanged)
						{
							set_palette(colors, 0, 255);
							paletteChanged = false;
						}

						JE_showVGA(); /* SYN: This is the where the screen updates for most menus */

						JE_mouseReplace();

						if (backFromHelp)
						{
							fade_palette(colors, 10, 0, 255);
							backFromHelp = false;
						}

					}
				}

				wait_delay();

				push_joysticks_as_keyboard();
				service_SDL_events(false);
				mouseButton = JE_mousePosition(&mouseX, &mouseY);
				inputDetected = newkey || mouseButton > 0;

				if (curMenu != 6)
				{
					if (keysactive[SDLK_s] && (keysactive[SDLK_LALT] || keysactive[SDLK_RALT]) )
					{
						if (curMenu == 8 || curMenu == 7)
						{
							curMenu = 0;
						}
						quikSave = true;
						oldMenu = curMenu;
						curMenu = 6;
						performSave = true;
						newPal = 1;
						oldPal = curPal;
					}
					if (keysactive[SDLK_l] && (keysactive[SDLK_LALT] || keysactive[SDLK_RALT]) )
					{
						if (curMenu == 8 || curMenu == 7)
						{
							curMenu = 0;
						}
						quikSave = true;
						oldMenu = curMenu;
						curMenu = 6;
						performSave = false;
						newPal = 1;
						oldPal = curPal;
					}
				}

				if (curMenu == 8)
				{
					if (mouseButton > 0 && mouseCursor >= 1)
					{
						inputDetected = false;
						if (mouseCursor == 1)
						{
							yChg = -1;
						} else {
							yChg = 1;
						}
					}

					if (keysactive[SDLK_PAGEUP])
					{
						yChg = -2;
						inputDetected = false;
					}
					if (keysactive[SDLK_PAGEDOWN])
					{
						yChg = 2;
						inputDetected = false;
					}

					bool joystick_up = false, joystick_down = false;
					for (int j = 0; j < joysticks; j++)
					{
						joystick_up |= joystick[j].direction[0];
						joystick_down |= joystick[j].direction[2];
					}

					if (keysactive[SDLK_UP] || joystick_up)
					{
						yChg = -1;
						inputDetected = false;
					}

					if (keysactive[SDLK_DOWN] || joystick_down)
					{
						yChg = 1;
						inputDetected = false;
					}

					if (yChg < 0 && yLoc == 0)
					{
						yChg = 0;
					}
					if (yChg  > 0 && (yLoc / 12) > cube[currentCube].last_line - 10)
					{
						yChg = 0;
					}
				}

			} while (!inputDetected);
		}

		keyboardUsed = false;

		/* The rest of this just grabs input events, handles them, then proceeds on. */

		if (mouseButton > 0)
		{
			lastDirection = 1;

			mouseButton = JE_mousePosition(&mouseX, &mouseY);

			if (curMenu == 7 && cubeMax == 0)
			{
				curMenu = 0;
				JE_playSampleNum(S_SPRING);
				newPal = 1;
				JE_wipeKey();
			}

			if (curMenu == 8)
			{
				if ((mouseX > 258) && (mouseX < 290) && (mouseY > 159) && (mouseY < 171))
				{
					curMenu = 7;
					JE_playSampleNum(S_SPRING);
				}
			}

			if (curMenu == 2 || curMenu == 11)
			{
				if ((mouseX >= (225 - 4)) && (mouseY >= 70) && (mouseY <= 82))
				{
					if (music_disabled)
					{
						music_disabled = false;
						restart_song();
					}

					curSel[2] = 4;

					tyrMusicVolume = (mouseX - (225 - 4)) / 4 * 12;
					if (tyrMusicVolume > 255)
						tyrMusicVolume = 255;
				}

				if ((mouseX >= (225 - 4)) && (mouseY >= 86) && (mouseY <= 98))
				{
					samples_disabled = false;

					curSel[2] = 5;

					fxVolume = (mouseX - (225 - 4)) / 4 * 12;
					if (fxVolume > 255)
						fxVolume = 255;
				}

				JE_calcFXVol();

				set_volume(tyrMusicVolume, fxVolume);

				JE_playSampleNum(S_CURSOR);
			}

			if ((mouseY > 20) && (mouseX > 170) && (mouseX < 308) && (curMenu != 8))
			{
				const JE_byte mouseSelectionY[MENU_MAX] = { 16, 16, 16, 16, 26, 12, 11, 28, 0, 16, 16, 16, 8, 16 };

				int selection = (mouseY - 38) / mouseSelectionY[curMenu]+2;

				if (curMenu == 9)
				{
					if (selection > 5)
						selection--;
					if (selection > 3)
						selection--;
				}

				if (curMenu == 0)
				{
					if (selection > 7)
						selection = 7;
				}

				// is play next level screen?
				if (curMenu == 3)
				{
					if (selection == menuChoices[curMenu] + 1)
						selection = menuChoices[curMenu];
				}

				if (selection <= menuChoices[curMenu])
				{
					if ((curMenu == 4) && (selection == menuChoices[4]))
					{
						player[0].cash = JE_cashLeft();
						curMenu = 1;
						JE_playSampleNum(S_ITEM);
					}
					else
					{
						JE_playSampleNum(S_CLICK);
						if (curSel[curMenu] == selection)
						{
							JE_menuFunction(curSel[curMenu]);
						}
						else
						{
							if ((curMenu == 4) && (JE_getCost(curSel[1], itemAvail[itemAvailMap[curSel[1]-2]-1][selection-2]) > player[0].cash))
							{
								JE_playSampleNum(S_CLINK);
							}
							else
							{
								if (curSel[1] == 4)
									player[0].weapon_mode = 1;

								curSel[curMenu] = selection;
							}

							// in front or rear weapon upgrade screen?
							if ((curMenu == 4) && ((curSel[1] == 3) || (curSel[1] == 4)))
								player[0].items.weapon[curSel[1]-3].power = temp_weapon_power[curSel[4]-2];
						}
					}
				}

				wait_noinput(false, true, false);
			}

			if ((curMenu == 4) && ((curSel[1] == 3) || (curSel[1] == 4)))
			{
				if ((mouseX >= 23) && (mouseX <= 36) && (mouseY >= 149) && (mouseY <= 168))
				{
					JE_playSampleNum(S_CURSOR);
					switch (curSel[1])
					{
					case 3:
					case 4:
						if (leftPower)
							player[0].items.weapon[curSel[1]-3].power = --temp_weapon_power[curSel[4]-2];
						else
							JE_playSampleNum(S_CLINK);

						break;
					}
					wait_noinput(false, true, false);
				}

				if ((mouseX >= 119) && (mouseX <= 131) && (mouseY >= 149) && (mouseY <= 168))
				{
					JE_playSampleNum(S_CURSOR);
					switch (curSel[1])
					{
					case 3:
					case 4:
						if (rightPower && rightPowerAfford)
							player[0].items.weapon[curSel[1]-3].power = ++temp_weapon_power[curSel[4]-2];
						else
							JE_playSampleNum(S_CLINK);

						break;
					}
					wait_noinput(false, true, false);
				}
			}
		}
		else if (newkey)
		{
			switch (lastkey_sym)
			{
			case SDLK_SLASH:
				// if in rear weapon upgrade screen
				if ( (curMenu == 4) && (curSel[1] == 4))
				{
					// cycle weapon modes
					if (++player[0].weapon_mode > weaponPort[player[0].items.weapon[REAR_WEAPON].id].opnum)
						player[0].weapon_mode = 1;
				}
				break;

			case SDLK_SPACE:
			case SDLK_RETURN:
				keyboardUsed = true;

				// if front or rear weapon, update "Done" power level
				if (curMenu == 4 && (curSel[1] == 3 || curSel[1] == 4))
					temp_weapon_power[itemAvailMax[itemAvailMap[curSel[1]-2]-1]] = player[0].items.weapon[curSel[1]-3].power;

				JE_menuFunction(curSel[curMenu]);
				break;

			case SDLK_ESCAPE:
				keyboardUsed = true;

				JE_playSampleNum(S_SPRING);
				if ( (curMenu == 6) && quikSave)
				{
					curMenu = oldMenu;
					newPal = oldPal;
				}
				else if (menuEsc[curMenu] == 0)
				{
					if (JE_quitRequest())
					{
						gameLoaded = true;
						mainLevel = 0;
					}
				}
				else
				{
					if (curMenu == 4)  // leaving upgrade menu without buying
					{
						player[0].items = old_items[0];
						curSel[4] = lastCurSel;
						player[0].cash = JE_cashLeft();
					}

					if (curMenu != 8) // not data cube
						newPal = 1;

					curMenu = menuEsc[curMenu] - 1;
				}
				break;

			case SDLK_F1:
				if (!isNetworkGame)
				{
					JE_helpSystem(2);
					fade_black(10);

					play_song(songBuy);

					JE_loadPic(VGAScreen, 1, false);
					newPal = 1;

					switch (curMenu)
					{
					case 3:
						newPal = 18;
						break;
					case 7:
					case 8:
						break;
					}

					memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->pitch * VGAScreen2->h);

					curPal = newPal;
					memcpy(colors, palettes[newPal-1], sizeof(colors));
					JE_showVGA();
					newPal = 0;
					backFromHelp = true;
				}
				break;

			case SDLK_UP:
				keyboardUsed = true;
				lastDirection = -1;

				if (curMenu != 8) // not data cube
					JE_playSampleNum(S_CURSOR);

				curSel[curMenu]--;
				if (curSel[curMenu] < 2)
					curSel[curMenu] = menuChoices[curMenu];

				// if in front or rear weapon upgrade screen
				if (curMenu == 4 && (curSel[1] == 3 || curSel[1] == 4))
				{
					player[0].items.weapon[curSel[1]-3].power = temp_weapon_power[curSel[4]-2];
					if (curSel[curMenu] == 4)
						player[0].weapon_mode = 1;
				}

				// if joystick config, skip disabled items when digital
				if (curMenu == 12 && joysticks > 0 && !joystick[joystick_config].analog && curSel[curMenu] == 5)
					curSel[curMenu] = 3;

				break;

			case SDLK_DOWN:
				keyboardUsed = true;
				lastDirection = 1;

				if (curMenu != 8) // not data cube
					JE_playSampleNum(S_CURSOR);

				curSel[curMenu]++;
				if (curSel[curMenu] > menuChoices[curMenu])
					curSel[curMenu] = 2;

				// if in front or rear weapon upgrade screen
				if (curMenu == 4 && (curSel[1] == 3 || curSel[1] == 4))
				{
					player[0].items.weapon[curSel[1]-3].power = temp_weapon_power[curSel[4]-2];
					if (curSel[curMenu] == 4)
						player[0].weapon_mode = 1;
				}

				// if in joystick config, skip disabled items when digital
				if (curMenu == 12 && joysticks > 0 && !joystick[joystick_config].analog && curSel[curMenu] == 4)
					curSel[curMenu] = 6;

				break;

			case SDLK_HOME:
				if (curMenu == 8) // data cube
					yLoc = 0;
				break;

			case SDLK_END:
				if (curMenu == 8) // data cube
					yLoc = (cube[currentCube].last_line - 9) * 12;
				break;

			case SDLK_LEFT:
				if (curMenu == 12) // joystick settings menu
				{
					if (joysticks > 0)
					{
						switch (curSel[curMenu])
						{
							case 2:
								if (joystick_config == 0)
									joystick_config = joysticks;
								joystick_config--;
								break;
							case 3:
								joystick[joystick_config].analog = !joystick[joystick_config].analog;
								break;
							case 4:
								if (joystick[joystick_config].sensitivity == 0)
									joystick[joystick_config].sensitivity = 10;
								else
									joystick[joystick_config].sensitivity--;
								break;
							case 5:
								if (joystick[joystick_config].threshold == 0)
									joystick[joystick_config].threshold = 10;
								else
									joystick[joystick_config].threshold--;
								break;
							default:
								break;
						}
					}
				}

				if (curMenu == 9)
				{
					switch (curSel[curMenu])
					{
					case 3:
					case 4:
						JE_playSampleNum(S_CURSOR);

						int temp = curSel[curMenu] - 3;
						do {
							if (joysticks == 0)
							{
								inputDevice[temp == 0 ? 1 : 0] = inputDevice[temp]; // swap controllers
							}
							if (inputDevice[temp] <= 1)
							{
								inputDevice[temp] = 2 + joysticks;
							} else {
								inputDevice[temp]--;
							}
						} while (inputDevice[temp] == inputDevice[temp == 0 ? 1 : 0]);
						break;
					}
				}

				if (curMenu == 2 || curMenu == 4  || curMenu == 11)
				{
					JE_playSampleNum(S_CURSOR);
				}

				switch (curMenu)
				{
				case 2:
				case 11:
					switch (curSel[curMenu])
					{
					case 4:
						JE_changeVolume(&tyrMusicVolume, -12, &fxVolume, 0);
						if (music_disabled)
						{
							music_disabled = false;
							restart_song();
						}
						break;
					case 5:
						JE_changeVolume(&tyrMusicVolume, 0, &fxVolume, -12);
						samples_disabled = false;
						break;
					}
					break;
				case 4:
					switch (curSel[1])
					{
					case 3:
					case 4:
						if (leftPower)
							player[0].items.weapon[curSel[1]-3].power = --temp_weapon_power[curSel[4]-2];
						else
							JE_playSampleNum(S_CLINK);

						break;
					}
					break;
				}
				break;

			case SDLK_RIGHT:
				if (curMenu == 12) // joystick settings menu
				{
					if (joysticks > 0)
					{
						switch (curSel[curMenu])
						{
							case 2:
								joystick_config++;
								joystick_config %= joysticks;
								break;
							case 3:
								joystick[joystick_config].analog = !joystick[joystick_config].analog;
								break;
							case 4:
								joystick[joystick_config].sensitivity++;
								joystick[joystick_config].sensitivity %= 11;
								break;
							case 5:
								joystick[joystick_config].threshold++;
								joystick[joystick_config].threshold %= 11;
								break;
							default:
								break;
						}
					}
				}

				if (curMenu == 9)
				{
					switch (curSel[curMenu])
					{
					case 3:
					case 4:
						JE_playSampleNum(S_CURSOR);

						int temp = curSel[curMenu] - 3;
						do {
							if (joysticks == 0)
							{
								inputDevice[temp == 0 ? 1 : 0] = inputDevice[temp]; // swap controllers
							}
							if (inputDevice[temp] >= 2 + joysticks)
							{
								inputDevice[temp] = 1;
							} else {
								inputDevice[temp]++;
							}
						} while (inputDevice[temp] == inputDevice[temp == 0 ? 1 : 0]);
						break;
					}
				}

				if (curMenu == 2 || curMenu == 4  || curMenu == 11)
				{
					JE_playSampleNum(S_CURSOR);
				}

				switch (curMenu)
				{
				case 2:
				case 11:
					switch (curSel[curMenu])
					{
					case 4:
						JE_changeVolume(&tyrMusicVolume, 12, &fxVolume, 0);
						if (music_disabled)
						{
							music_disabled = false;
							restart_song();
						}
						break;
					case 5:
						JE_changeVolume(&tyrMusicVolume, 0, &fxVolume, 12);
						samples_disabled = false;
						break;
					}
					break;
				case 4:
					switch (curSel[1])
					{
					case 3:
					case 4:
						if (rightPower && rightPowerAfford)
							player[0].items.weapon[curSel[1]-3].power = ++temp_weapon_power[curSel[4]-2];
						else
							JE_playSampleNum(S_CLINK);

						break;
					}
					break;
				}
				break;

			default:
				break;
			}
		}

	} while (!(quit || gameLoaded || jumpSection));

#ifdef WITH_NETWORK
	if (!quit && isNetworkGame)
	{
		JE_barShade(VGAScreen, 3, 3, 316, 196);
		JE_barShade(VGAScreen, 1, 1, 318, 198);
		JE_dString(VGAScreen, 10, 160, "Waiting for other player.", SMALL_FONT_SHAPES);

		network_prepare(PACKET_WAITING);
		network_send(4);  // PACKET_WAITING

		while (true)
		{
			service_SDL_events(false);
			JE_showVGA();

			if (packet_in[0] && SDLNet_Read16(&packet_in[0]->data[0]) == PACKET_WAITING)
			{
				network_update();
				break;
			}

			network_update();
			network_check();

			SDL_Delay(16);
		}

		network_state_reset();
	}

	if (isNetworkGame)
	{
		while (!network_is_sync())
		{
			service_SDL_events(false);
			JE_showVGA();

			network_check();
			SDL_Delay(16);
		}
	}
#endif

	if (gameLoaded)
		fade_black(10);
}

void draw_ship_illustration( void )
{
	// full of evil hardcoding

	// ship
	{
		assert(player[0].items.ship > 0);

		const int sprite_id = (player[0].items.ship < COUNTOF(ships))  // shipedit ships get a default
		                      ? ships[player[0].items.ship].bigshipgraphic - 1
		                      : 31;

		const int ship_x[6] = { 31, 0, 0, 0, 35, 31 },
		          ship_y[6] = { 36, 0, 0, 0, 33, 35 };

		const int x = ship_x[sprite_id - 27],
		          y = ship_y[sprite_id - 27];

		blit_sprite(VGAScreenSeg, x, y, OPTION_SHAPES, sprite_id);
	}

	// generator
	{
		assert(player[0].items.generator > 0 && player[0].items.generator < 7);

		const int sprite_id = (player[0].items.generator == 1)  // generator 1 and generator 2 have the same sprite
		                      ? player[0].items.generator + 15
		                      : player[0].items.generator + 14;

		const int generator_x[5] = { 62, 64, 67, 66, 63 },
		          generator_y[5] = { 84, 85, 86, 84, 97 };
		const int x = generator_x[sprite_id - 16],
		          y = generator_y[sprite_id - 16];

		blit_sprite(VGAScreenSeg, x, y, WEAPON_SHAPES, sprite_id);
	}

	const int weapon_sprites[43] =
	{
		-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,
		 9, 10, 11, 21,  5, 13, -1, 14, 15,  0,
		14,  9,  8,  2, 15,  0, 13,  0,  8,  8,
		11,  1,  0,  0,  0,  0,  0,  0,  0,  0,
		 0,  2,  1
	};

	// front weapon
	if (player[0].items.weapon[FRONT_WEAPON].id > 0)
	{
		const int front_weapon_xy_list[43] =
		{
			 -1,  4,  9,  3,  8,  2,  5, 10,  1, -1,
			 -1, -1, -1,  7,  8, -1, -1,  0, -1,  4,
			  0, -1, -1,  3, -1,  4, -1,  4, -1, -1,
			 -1,  9,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  3,  9
		};

		const int front_weapon_x[12] = { 59, 66, 66, 54, 61, 51, 58, 51, 61, 52, 53, 58 };
		const int front_weapon_y[12] = { 38, 53, 41, 36, 48, 35, 41, 35, 53, 41, 39, 31 };
		const int x = front_weapon_x[front_weapon_xy_list[player[0].items.weapon[FRONT_WEAPON].id]],
		          y = front_weapon_y[front_weapon_xy_list[player[0].items.weapon[FRONT_WEAPON].id]];

		blit_sprite(VGAScreenSeg, x, y, WEAPON_SHAPES, weapon_sprites[player[0].items.weapon[FRONT_WEAPON].id]);  // ship illustration: front weapon
	}

	// rear weapon
	if (player[0].items.weapon[REAR_WEAPON].id > 0)
	{
		const int rear_weapon_xy_list[43] =
		{
			-1, -1, -1, -1, -1, -1, -1, -1, -1,  0,
			 1,  2,  3, -1,  4,  5, -1, -1,  6, -1,
			-1,  1,  0, -1,  6, -1,  5, -1,  0,  0,
			 3,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			 0, -1, -1
		};

		const int rear_weapon_x[7] = { 41, 27,  49,  43, 51, 39, 41 };
		const int rear_weapon_y[7] = { 92, 92, 113, 102, 97, 96, 76 };
		const int x = rear_weapon_x[rear_weapon_xy_list[player[0].items.weapon[REAR_WEAPON].id]],
		          y = rear_weapon_y[rear_weapon_xy_list[player[0].items.weapon[REAR_WEAPON].id]];

		blit_sprite(VGAScreenSeg, x, y, WEAPON_SHAPES, weapon_sprites[player[0].items.weapon[REAR_WEAPON].id]);
	}

	// sidekicks
	JE_drawItem(6, player[0].items.sidekick[LEFT_SIDEKICK], 3, 84);
	JE_drawItem(7, player[0].items.sidekick[RIGHT_SIDEKICK], 129, 84);

	// shield
	blit_sprite_hv(VGAScreenSeg, 28, 23, OPTION_SHAPES, 26, 15, shields[player[0].items.shield].mpwr - 10);
}

void load_cubes( void )
{
	for (int cube_slot = 0; cube_slot < cubeMax; ++cube_slot)
	{
		memset(cube[cube_slot].text, 0, sizeof(cube->text));

		load_cube(cube_slot, cubeList[cube_slot]);
	}
}

bool load_cube( int cube_slot, int cube_index )
{
	FILE *f = dir_fopen_die(data_dir(), cube_file, "rb");

	char buf[256];

	// seek to the cube
	while (cube_index > 0)
	{
		read_encrypted_pascal_string(buf, sizeof(buf), f);
		if (buf[0] == '*')
			--cube_index;

		if (feof(f))
		{
			fclose(f);

			return false;
		}
	}

	str_pop_int(&buf[4], &cube[cube_slot].face_sprite);
	--cube[cube_slot].face_sprite;

	read_encrypted_pascal_string(cube[cube_slot].title, sizeof(cube[cube_slot].title), f);
	read_encrypted_pascal_string(cube[cube_slot].header, sizeof(cube[cube_slot].header), f);

	uint line = 0, line_chars = 0, line_width = 0;

	// for each line of decrypted text, split the line into words
	// and add them individually to the lines of wrapped text
	for (; ; )
	{
		read_encrypted_pascal_string(buf, sizeof(buf), f);

		// end of data
		if (feof(f) || buf[0] == '*')
			break;

		// new paragraph
		if (strlen(buf) == 0)
		{
			if (line_chars == 0)
				line += 4;  // subsequent new paragaphs indicate 4-line break
			else
				++line;
			line_chars = 0;
			line_width = 0;

			continue;
		}

		uint word_start = 0;
		for (uint i = 0; ; ++i)
		{
			bool end_of_line = (buf[i] == '\0'),
			     end_of_word = end_of_line || (buf[i] == ' ');

			if (end_of_word)
			{
				buf[i] = '\0';

				char *word = &buf[word_start];
				word_start = i + 1;

				uint word_chars = strlen(word),
				     word_width = JE_textWidth(word, TINY_FONT);

				// word won't fit; no can do
				if (word_chars > cube_line_chars || word_width > cube_line_width)
					break;

				bool prepend_space = true;

				line_chars += word_chars + (prepend_space ? 1 : 0);
				line_width += word_width + (prepend_space ? 6 : 0);

				// word won't fit on current line; use next
				if (line_chars > cube_line_chars || line_width > cube_line_width)
				{
					++line;
					line_chars = word_chars;
					line_width = word_width;

					prepend_space = false;
				}

				// append word
				if (line < COUNTOF(cube->text))
				{
					if (prepend_space)
						strcat(cube[cube_slot].text[line], " ");
					strcat(cube[cube_slot].text[line], word);

					// track last line with text
					cube[cube_slot].last_line = line + 1;
				}
			}

			if (end_of_line)
				break;
		}
	}

	fclose(f);

	return true;
}

void JE_drawItem( JE_byte itemType, JE_word itemNum, JE_word x, JE_word y )
{
	JE_word tempW = 0;

	if (itemNum > 0)
	{
		switch (itemType)
		{
			case 2:
			case 3:
				tempW = weaponPort[itemNum].itemgraphic;
				break;
			case 5:
				tempW = powerSys[itemNum].itemgraphic;
				break;
			case 6:
			case 7:
				tempW = options[itemNum].itemgraphic;
				break;
			case 4:
				tempW = shields[itemNum].itemgraphic;
				break;
		}

		if (itemType == 1)
		{
			if (itemNum > 90)
			{
				shipGrPtr = &shapes9;
				shipGr = JE_SGr(itemNum - 90, &shipGrPtr);
				blit_sprite2x2(VGAScreen, x, y, *shipGrPtr, shipGr);
			}
			else
			{
				blit_sprite2x2(VGAScreen, x, y, shapes9, ships[itemNum].shipgraphic);
			}
		}
		else if (tempW > 0)
		{
			blit_sprite2x2(VGAScreen, x, y, shapes6, tempW);
		}
	}
}

void JE_drawMenuHeader( void )
{
	switch (curMenu)
	{
		case 8:
			strcpy(tempStr, cube[curSel[7]-2].header);
			break;
		case 7:
			strcpy(tempStr, menuInt[1][1]);
			break;
		case 6:
			strcpy(tempStr, menuInt[3][performSave + 1]);
			break;
		default:
			strcpy(tempStr, menuInt[curMenu + 1][0]);
			break;
	}
	JE_dString(VGAScreen, 74 + JE_fontCenter(tempStr, FONT_SHAPES), 10, tempStr, FONT_SHAPES);
}

void JE_drawMenuChoices( void )
{
	JE_byte x;
	char *str;

	for (x = 2; x <= menuChoices[curMenu]; x++)
	{
		int tempY = 38 + (x-1) * 16;

		if (curMenu == 0)
		{
			if (x == 7)
			{
				tempY += 16;
			}
		}

		if (curMenu == 9)
		{
			if (x > 3)
			{
				tempY += 16;
			}
			if (x > 4)
			{
				tempY += 16;
			}
		}

		if (!(curMenu == 3 && x == menuChoices[curMenu]))
		{
			tempY -= 16;
		}

		str = malloc(strlen(menuInt[curMenu + 1][x-1])+2);
		if (curSel[curMenu] == x)
		{
			str[0] = '~';
			strcpy(str+1, menuInt[curMenu + 1][x-1]);
		} else {
			strcpy(str, menuInt[curMenu + 1][x-1]);
		}
		JE_dString(VGAScreen, 166, tempY, str, SMALL_FONT_SHAPES);
		free(str);

		if (keyboardUsed && curSel[curMenu] == x)
		{
			set_mouse_position(305, tempY + 6);
		}
	}
}

void JE_updateNavScreen( void )
{
	JE_byte x;

	/* minor issues: */
	/* TODO: The scroll to the new planet is too fast, I think */
	/* TODO: The starting coordinates for the scrolling effect may be wrong, the
	   yellowish planet below Tyrian isn't visible for as many frames as in the
	   original. */

	tempNavX = roundf(navX);
	tempNavY = roundf(navY);
	fill_rectangle_xy(VGAScreen, 19, 16, 135, 169, 2);
	JE_drawNavLines(true);
	JE_drawNavLines(false);
	JE_drawDots();

	for (x = 0; x < 11; x++)
		JE_drawPlanet(x);

	for (x = 0; x < menuChoices[3]-1; x++)
	{
		if (mapPlanet[x] > 11)
			JE_drawPlanet(mapPlanet[x] - 1);
	}

	if (mapOrigin > 11)
		JE_drawPlanet(mapOrigin - 1);

	blit_sprite(VGAScreenSeg, 0, 0, OPTION_SHAPES, 28);  // navigation screen interface

	if (curSel[3] < menuChoices[3])
	{
		const unsigned int origin_x_offset = sprite(PLANET_SHAPES, PGR[mapOrigin-1]-1)->width / 2,
		                   origin_y_offset = sprite(PLANET_SHAPES, PGR[mapOrigin-1]-1)->height / 2,
		                   dest_x_offset = sprite(PLANET_SHAPES, PGR[mapPlanet[curSel[3]-2] - 1]-1)->width / 2,
		                   dest_y_offset = sprite(PLANET_SHAPES, PGR[mapPlanet[curSel[3]-2] - 1]-1)->height / 2;

		newNavX = (planetX[mapOrigin-1] - origin_x_offset
		          + planetX[mapPlanet[curSel[3]-2] - 1] - dest_x_offset) / 2.0f;
		newNavY = (planetY[mapOrigin-1] - origin_y_offset
		          + planetY[mapPlanet[curSel[3]-2] - 1] - dest_y_offset) / 2.0f;
	}

	navX = navX + (newNavX - navX) / 2.0f;
	navY = navY + (newNavY - navY) / 2.0f;

	if (fabsf(newNavX - navX) < 1)
		navX = newNavX;
	if (fabsf(newNavY - navY) < 1)
		navY = newNavY;

	fill_rectangle_xy(VGAScreen, 314, 0, 319, 199, 230);

	if (planetAniWait > 0)
	{
		planetAniWait--;
	}
	else
	{
		planetAni++;
		if (planetAni > 14)
			planetAni = 0;
		planetAniWait = 3;
	}

	if (currentDotWait > 0)
	{
		currentDotWait--;
	}
	else
	{
		if (currentDotNum < planetDots[curSel[3]-2])
			currentDotNum++;
		currentDotWait = 5;
	}
}

void JE_drawLines( SDL_Surface *surface, JE_boolean dark )
{
	JE_byte x, y;
	JE_integer tempX, tempY;
	JE_integer tempX2, tempY2;
	JE_word tempW, tempW2;

	tempX2 = -10;
	tempY2 = 0;

	tempW = 0;
	for (x = 0; x < 20; x++)
	{
		tempW += 15;
		tempX = tempW - tempX2;

		if (tempX > 18 && tempX < 135)
		{
			if (dark)
			{
				JE_rectangle(surface, tempX + 1, 0, tempX + 1, 199, 32+3);
			} else {
				JE_rectangle(surface, tempX, 0, tempX, 199, 32+5);
			}
		}
	}

	tempW = 0;
	for (y = 0; y < 20; y++)
	{
		tempW += 15;
		tempY = tempW - tempY2;

		if (tempY > 15 && tempY < 169)
		{
			if (dark)
			{
				JE_rectangle(surface, 0, tempY + 1, 319, tempY + 1, 32+3);
			} else {
				JE_rectangle(surface, 0, tempY, 319, tempY, 32+5);
			}

			tempW2 = 0;

			for (x = 0; x < 20; x++)
			{
				tempW2 += 15;
				tempX = tempW2 - tempX2;
				if (tempX > 18 && tempX < 135)
				{
					JE_pix3(surface, tempX, tempY, 32+6);
				}
			}
		}
	}
}

/* SYN: This was originally PROC drawlines... yes, there were two different procs called
   drawlines in different scopes in the same file. Dammit, Jason, why do you do this to me? */

void JE_drawNavLines( JE_boolean dark )
{
	JE_byte x, y;
	JE_integer tempX, tempY;
	JE_integer tempX2, tempY2;
	JE_word tempW, tempW2;

	tempX2 = tempNavX >> 1;
	tempY2 = tempNavY >> 1;

	tempW = 0;
	for (x = 1; x <= 20; x++)
	{
		tempW += 15;
		tempX = tempW - tempX2;

		if (tempX > 18 && tempX < 135)
		{
			if (dark)
				JE_rectangle(VGAScreen, tempX + 1, 16, tempX + 1, 169, 1);
			else
				JE_rectangle(VGAScreen, tempX, 16, tempX, 169, 5);
		}
	}

	tempW = 0;
	for (y = 1; y <= 20; y++)
	{
		tempW += 15;
		tempY = tempW - tempY2;

		if (tempY > 15 && tempY < 169)
		{
			if (dark)
				JE_rectangle(VGAScreen, 19, tempY + 1, 135, tempY + 1, 1);
			else
				JE_rectangle(VGAScreen, 8, tempY, 160, tempY, 5);

			tempW2 = 0;

			for (x = 0; x < 20; x++)
			{
				tempW2 += 15;
				tempX = tempW2 - tempX2;
				if (tempX > 18 && tempX < 135)
					JE_pix3(VGAScreen, tempX, tempY, 7);
			}
		}
	}
}

void JE_drawDots( void )
{
	JE_byte x, y;
	JE_integer tempX, tempY;

	for (x = 0; x < mapPNum; x++)
	{
		for (y = 0; y < planetDots[x]; y++)
		{
			tempX = planetDotX[x][y] - tempNavX + 66 - 2;
			tempY = planetDotY[x][y] - tempNavY + 85 - 2;
			if (tempX > 0 && tempX < 140 && tempY > 0 && tempY < 168)
				blit_sprite(VGAScreenSeg, tempX, tempY, OPTION_SHAPES, (x == curSel[3]-2 && y < currentDotNum) ? 30 : 29);  // navigation dots
		}
	}
}

void JE_drawPlanet( JE_byte planetNum )
{
	JE_integer tempZ = PGR[planetNum]-1,
	           tempX = planetX[planetNum] + 66 - tempNavX - sprite(PLANET_SHAPES, tempZ)->width / 2,
	           tempY = planetY[planetNum] + 85 - tempNavY - sprite(PLANET_SHAPES, tempZ)->height / 2;

	if (tempX > -7 && tempX + sprite(PLANET_SHAPES, tempZ)->width < 170 && tempY > 0 && tempY < 160)
	{
		if (PAni[planetNum])
			tempZ += planetAni;

		blit_sprite_dark(VGAScreenSeg, tempX + 3, tempY + 3, PLANET_SHAPES, tempZ, false);
		blit_sprite(VGAScreenSeg, tempX, tempY, PLANET_SHAPES, tempZ);  // planets
	}
}

void JE_scaleBitmap( SDL_Surface *dst_bitmap, const SDL_Surface *src_bitmap,  int x1, int y1, int x2, int y2 )
{
	/* This function scales one screen and writes the result to another.
	 *  The only code that calls it is the code run when you select 'ship
	 * specs' from the main menu.
	 *
	 * Originally this used fixed point math.  I haven't seen that in ages :).
	 * But we're well past the point of needing that.*/

	assert(src_bitmap != NULL && dst_bitmap != NULL);
	assert(x1 >= 0 && y1 >= 0 && x2 < src_bitmap->pitch && y2 < src_bitmap->h);

	int w = x2 - x1 + 1,
	    h = y2 - y1 + 1;
	float base_skip_w = src_bitmap->pitch / (float)w,
	      base_skip_h = src_bitmap->h / (float)h;
	float cumulative_skip_w, cumulative_skip_h;


	//Okay, it's time to loop through and add bits of A to a rectangle in B
	Uint8 *dst = dst_bitmap->pixels;  /* 8-bit specific */
	const Uint8 *src, *src_w;  /* 8-bit specific */

	dst += y1 * dst_bitmap->pitch + x1;
	cumulative_skip_h = 0;

	for (int i = 0; i < h; i++)
	{
		//this sets src to the beginning of our desired line
		src = src_w = (Uint8 *)(src_bitmap->pixels) + (src_bitmap->w * ((unsigned int)cumulative_skip_h));
		cumulative_skip_h += base_skip_h;
		cumulative_skip_w = 0;

		for (int j = 0; j < w; j++)
		{
			//copy and move pointers
			*dst = *src;
			dst++;

			cumulative_skip_w += base_skip_w;
			src = src_w + ((unsigned int)cumulative_skip_w); //value is floored
		}

		dst += dst_bitmap->pitch - w;
	}
}

void JE_initWeaponView( void )
{
	fill_rectangle_xy(VGAScreen, 8, 8, 144, 177, 0);

	player[0].sidekick[LEFT_SIDEKICK].x = 72 - 15;
	player[0].sidekick[LEFT_SIDEKICK].y = 120;
	player[0].sidekick[RIGHT_SIDEKICK].x = 72 + 15;
	player[0].sidekick[RIGHT_SIDEKICK].y = 120;

	player[0].x = 72;
	player[0].y = 110;
	player[0].delta_x_shot_move = 0;
	player[0].delta_y_shot_move = 0;
	player[0].last_x_explosion_follow = 72;
	player[0].last_y_explosion_follow = 110;
	power = 500;
	lastPower = 500;

	memset(shotAvail, 0, sizeof(shotAvail));

	memset(shotRepeat, 1, sizeof(shotRepeat));
	memset(shotMultiPos, 0, sizeof(shotMultiPos));

	initialize_starfield();
}

void JE_computeDots( void )
{
	JE_integer tempX, tempY;
	JE_longint distX, distY;
	JE_byte x, y;

	for (x = 0; x < mapPNum; x++)
	{
		distX = (int)(planetX[mapPlanet[x]-1]) - (int)(planetX[mapOrigin-1]);
		distY = (int)(planetY[mapPlanet[x]-1]) - (int)(planetY[mapOrigin-1]);
		tempX = abs(distX) + abs(distY);

		if (tempX != 0)
		{
			planetDots[x] = roundf(sqrtf(sqrtf((distX * distX) + (distY * distY)))) - 1;
		} else {
			planetDots[x] = 0;
		}

		if (planetDots[x] > 10)
		{
			planetDots[x] = 10;
		}

		for (y = 0; y < planetDots[x]; y++)
		{
			tempX = JE_partWay(planetX[mapOrigin-1], planetX[mapPlanet[x]-1], planetDots[x], y);
			tempY = JE_partWay(planetY[mapOrigin-1], planetY[mapPlanet[x]-1], planetDots[x], y);
			/* ??? Why does it use temp? =P */
			planetDotX[x][y] = tempX;
			planetDotY[x][y] = tempY;
		}
	}
}

JE_integer JE_partWay( JE_integer start, JE_integer finish, JE_byte dots, JE_byte dist )
{
	return (finish - start) / (dots + 2) * (dist + 1) + start;
}

void JE_doShipSpecs( void )
{
	/* This function is called whenever you select 'ship specs' in the
	 * game menu.  It draws the nice green tech screen and scales it onto
	 * the main window.  To do this we need two temp buffers, so we're going
	 * to use VGAScreen and game_screen for the purpose (making things more
	 * complex than they would be if we just malloc'd, but faster)
	 *
	 * Originally the whole system was pretty oddly designed.  So I changed it.
	 * Currently drawFunkyScreen creates the image, scaleInPicture draws it,
	 * and doFunkyScreen ties everything together.  Before it was more like
	 * an oddly designed, unreusable, global sharing hierarchy. */

	//create the image we want
	wait_noinput(true, true, true);
	JE_drawShipSpecs(game_screen, VGAScreen2);

	//reset VGAScreen2, which we clobbered
	JE_loadPic(VGAScreen2, 1, false);

	//draw it
	JE_playSampleNum(S_SPRING);
	JE_scaleInPicture(VGAScreen, game_screen);
	wait_input(true, true, true);
}

void JE_drawMainMenuHelpText( void )
{
	char tempStr[67];
	JE_byte temp;

	temp = curSel[curMenu] - 2;
	if (curMenu == 12) // joystick settings menu help
	{
		int help[16] = { 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 24, 11 };
		memcpy(tempStr, mainMenuHelp[help[curSel[curMenu] - 2]], sizeof(tempStr));
	}
	else if (curMenu < 3 || curMenu == 9 || curMenu > 10)
	{
		memcpy(tempStr, mainMenuHelp[(menuHelp[curMenu][temp])-1], sizeof(tempStr));
	}
	else if (curMenu == 5 && curSel[5] == 10)
	{
		memcpy(tempStr, mainMenuHelp[25-1], sizeof(tempStr));
	}
	else if (leftPower || rightPower)
	{
		memcpy(tempStr, mainMenuHelp[24-1], sizeof(tempStr));
	}
	else if ( (temp == menuChoices[curMenu] - 1) || ( (curMenu == 7) && (cubeMax == 0) ) )
	{
		memcpy(tempStr, mainMenuHelp[12-1], sizeof(tempStr));
	}
	else
	{
		memcpy(tempStr, mainMenuHelp[17 + curMenu - 3], sizeof(tempStr));
	}
	
	JE_textShade(VGAScreen, 10, 187, tempStr, 14, 1, DARKEN);
}

JE_boolean JE_quitRequest( void )
{
	bool quit_selected = true, done = false;

	JE_clearKeyboard();
	JE_wipeKey();
	wait_noinput(true, true, true);

	JE_barShade(VGAScreen, 65, 55, 255, 155);

	while (!done)
	{
		Uint8 col = 8;
		int colC = 1;

		do
		{
			service_SDL_events(true);
			setjasondelay(4);

			blit_sprite(VGAScreen, 50, 50, OPTION_SHAPES, 35);  // message box
			JE_textShade(VGAScreen, 70, 60, miscText[28], 0, 5, FULL_SHADE);
			JE_helpBox(VGAScreen, 70, 90, miscText[30], 30);

			col += colC;
			if (col > 8 || col < 2)
				colC = -colC;

			int temp_x, temp_c;

			temp_x = 54 + 45 - (JE_textWidth(miscText[9], FONT_SHAPES) / 2);
			temp_c = quit_selected ? col - 12 : -5;

			JE_outTextAdjust(VGAScreen, temp_x, 128, miscText[9], 15, temp_c, FONT_SHAPES, true);

			temp_x = 149 + 45 - (JE_textWidth(miscText[10], FONT_SHAPES) / 2);
			temp_c = !quit_selected ? col - 12 : -5;

			JE_outTextAdjust(VGAScreen, temp_x, 128, miscText[10], 15, temp_c, FONT_SHAPES, true);

			if (has_mouse)
			{
				JE_mouseStart();
				JE_showVGA();
				JE_mouseReplace();
			}
			else
			{
				JE_showVGA();
			}

			wait_delay();

			push_joysticks_as_keyboard();
			service_SDL_events(false);

		} while (!newkey && !mousedown);

		if (mousedown)
		{
			if (lastmouse_y > 123 && lastmouse_y < 149)
			{
				if (lastmouse_x > 56 && lastmouse_x < 142)
				{
					quit_selected = true;
					done = true;
				}
				else if (lastmouse_x > 151 && lastmouse_x < 237)
				{
					quit_selected = false;
					done = true;
				}
			}
			mousedown = false;
		}
		else if (newkey)
		{
			switch (lastkey_sym)
			{
				case SDLK_LEFT:
				case SDLK_RIGHT:
				case SDLK_TAB:
					quit_selected = !quit_selected;
					JE_playSampleNum(S_CURSOR);
					break;
				case SDLK_RETURN:
				case SDLK_SPACE:
					done = true;
					break;
				case SDLK_ESCAPE:
					quit_selected = false;
					done = true;
					break;
				default:
					break;
			}
		}
	}

	JE_playSampleNum(quit_selected ? S_SPRING : S_CLICK);

#ifdef WITH_NETWORK
	if (isNetworkGame && quit_selected)
	{
		network_prepare(PACKET_QUIT);
		network_send(4);  // PACKET QUIT

		network_tyrian_halt(0, true);
	}
#endif

	return quit_selected;
}

void JE_genItemMenu( JE_byte itemNum )
{
	menuChoices[4] = itemAvailMax[itemAvailMap[itemNum - 2] - 1] + 2;

	temp3 = 2;
	temp2 = *playeritem_map(&player[0].items, itemNum - 2);

	strcpy(menuInt[5][0], menuInt[2][itemNum - 1]);

	for (tempW = 0; tempW < itemAvailMax[itemAvailMap[itemNum - 2] - 1]; tempW++)
	{
		temp = itemAvail[itemAvailMap[itemNum - 2] - 1][tempW];
		switch (itemNum)
		{
		case 2:
			strcpy(tempStr, ships[temp].name);
			break;
		case 3:
		case 4:
			strcpy(tempStr, weaponPort[temp].name);
			break;
		case 5:
			strcpy(tempStr, shields[temp].name);
			break;
		case 6:
			strcpy(tempStr, powerSys[temp].name);
			break;
		case 7:
		case 8:
			strcpy(tempStr, options[temp].name);
			break;
		}
		if (temp == temp2)
		{
			temp3 = tempW + 2;
		}
		strcpy(menuInt[5][tempW], tempStr);
	}

	strcpy(menuInt[5][tempW], miscText[13]);

	curSel[4] = temp3;
}

void JE_scaleInPicture( SDL_Surface *dst, const SDL_Surface *src )
{
	for (int i = 2; i <= 160; i += 2)
	{
		if (JE_anyButton()) { break; }

		JE_scaleBitmap(dst, src, 160 - i, 0, 160 + i - 1, 100 + roundf(i * 0.625f) - 1);
		JE_showVGA();

		SDL_Delay(1);
	}
}


void JE_drawScore( void )
{
	char cl[24];
	if (curMenu == 4)
	{
		sprintf(cl, "%d", JE_cashLeft());
		JE_textShade(VGAScreen, 65, 173, cl, 1, 6, DARKEN);
	}
}

void JE_menuFunction( JE_byte select )
{
	JE_byte x;
	JE_word curSelect;

	col = 0;
	colC = -1;
	JE_playSampleNum(S_CLICK);

	curSelect = curSel[curMenu];

	switch (curMenu)
	{
	case 0: //root menu
		switch (select)
		{
		case 2: //cubes
			curMenu = 7;
			curSel[7] = 2;
			break;
		case 3: //shipspecs
			JE_doShipSpecs();
			break;
		case 4://upgradeship
			curMenu = 1;
			break;
		case 5: //options
			curMenu = 2;
			break;
		case 6: //nextlevel
			curMenu = 3;
			newPal = 18;
			JE_computeDots();
			navX = planetX[mapOrigin - 1];
			navY = planetY[mapOrigin - 1];
			newNavX = navX;
			newNavY = navY;
			menuChoices[3] = mapPNum + 2;
			curSel[3] = 2;
			strcpy(menuInt[4][0], "Next Level");
			for (x = 0; x < mapPNum; x++)
			{
				temp = mapPlanet[x];
				strcpy(menuInt[4][x + 1], pName[temp - 1]);
			}
			strcpy(menuInt[4][x + 1], miscText[5]);
			break;
		case 7: //quit
			if (JE_quitRequest())
			{
				gameLoaded = true;
				mainLevel = 0;
			}
			break;
		}
		break;

	case 1: //upgradeship
		if (select == 9) //done
		{
			curMenu = 0;
		}
		else // selected item to upgrade
		{
			old_items[0] = player[0].items;

			lastDirection = 1;
			JE_genItemMenu(select);
			JE_initWeaponView();
			curMenu = 4;
			lastCurSel = curSel[4];
			player[0].cash = player[0].cash * 2 - JE_cashLeft();
		}
		break;

	case 2: //options
		switch (select)
		{
		case 2:
			curMenu = 6;
			performSave = false;
			quikSave = false;
			break;
		case 3:
			curMenu = 6;
			performSave = true;
			quikSave = false;
			break;
		case 6:
			curMenu = 12;
			break;
		case 7:
			curMenu = 5;
			break;
		case 8:
			curMenu = 0;
			break;
		}
		break;

	case 3: //nextlevel
		if (select == menuChoices[3]) //exit
		{
			curMenu = 0;
			newPal = 1;
		} else {
			mainLevel = mapSection[curSelect - 2];
			jumpSection = true;
		}
		break;

	case 4: //buying
		if (curSel[4] < menuChoices[4])
		{
			// select done
			curSel[4] = menuChoices[4];
		}
		else // if done is selected
		{
			JE_playSampleNum(S_ITEM);

			player[0].cash = JE_cashLeft();
			curMenu = 1;
		}
		break;

	case 5: /* keyboard settings */
		if (curSelect == 10) /* reset to defaults */
		{
			memcpy(keySettings, defaultKeySettings, sizeof(keySettings));
		}
		else if (curSelect == 11) /* done */
		{
			if (isNetworkGame || onePlayerAction)
			{
				curMenu = 11;
			} else {
				curMenu = 2;
			}
		}
		else /* change key */
		{
			temp2 = 254;
			int tempY = 38 + (curSelect - 2) * 12;
			JE_textShade(VGAScreen, 236, tempY, SDL_GetKeyName(keySettings[curSelect-2]), (temp2 / 16), (temp2 % 16) - 8, DARKEN);
			JE_showVGA();

			wait_noinput(true, true, true);

			col = 248;
			colC = 1;

			do {
				setjasondelay(1);

				col += colC;
				if (col < 243 || col > 248)
				{
					colC *= -1;
				}
				JE_rectangle(VGAScreen, 230, tempY - 2, 300, tempY + 7, col);

				poll_joysticks();
				service_SDL_events(true);

				JE_showVGA();

				wait_delay();
			} while (!newkey && !mousedown && !joydown);
			
			if (newkey)
			{
				// already used? then swap
				for (uint i = 0; i < COUNTOF(keySettings); ++i)
				{
					if (keySettings[i] == lastkey_sym)
					{
						keySettings[i] = keySettings[curSelect-2];
						break;
					}
				}
				
				if (lastkey_sym != SDLK_ESCAPE && // reserved for menu
				    lastkey_sym != SDLK_F11 &&    // reserved for gamma
				    lastkey_sym != SDLK_p)        // reserved for pause
				{
					JE_playSampleNum(S_CLICK);
					keySettings[curSelect-2] = lastkey_sym;
					++curSelect;
				}
				
				JE_wipeKey();
			}
		}
		break;

	case 6: //save
		if (curSelect == 13)
		{
			if (quikSave)
			{
				curMenu = oldMenu;
				newPal = oldPal;
			} else {
				curMenu = 2;
			}
		} else {
			if (twoPlayerMode)
			{
				temp = 11;
			} else {
				temp = 0;
			}
			JE_operation(curSelect - 1 + temp);
			if (quikSave)
			{
				curMenu = oldMenu;
				newPal = oldPal;
			}
		}
		break;

	case 7: //cubes
		if (curSelect == menuChoices[curMenu])
		{
			curMenu = 0;
			newPal = 1;
		} else {
			if (cubeMax > 0)
			{
				firstMenu9 = true;
				curMenu = 8;
				yLoc = 0;
				yChg = 0;
				currentCube = curSel[7] - 2;
			} else {
				curMenu = 0;
				newPal = 1;
			}
		}
		break;

	case 8: //cubes 2
		curMenu = 7;
		break;

	case 9: //2player
		switch (curSel[curMenu])
		{
		case 2:
			mainLevel = mapSection[mapPNum-1];
			jumpSection = true;
			break;
		case 3:
		case 4:
			JE_playSampleNum(S_CURSOR);

			int temp = curSel[curMenu] - 3;
			do {
				if (joysticks == 0)
				{
					inputDevice[temp == 0 ? 1 : 0] = inputDevice[temp]; // swap controllers
				}
				if (inputDevice[temp] >= 2 + joysticks)
				{
					inputDevice[temp] = 1;
				} else {
					inputDevice[temp]++;
				}
			} while (inputDevice[temp] == inputDevice[temp == 0 ? 1 : 0]);
			break;
		case 5:
			curMenu = 2;
			break;
		case 6:
			if (JE_quitRequest())
			{
				gameLoaded = true;
				mainLevel = 0;
			}
			break;
		}
		break;

	case 10: //arcade
		switch (curSel[curMenu])
		{
		case 2:
			mainLevel = mapSection[mapPNum-1];
			jumpSection = true;
			break;
		case 3:
			curMenu = 2;
			break;
		case 4:
			if (JE_quitRequest())
			{
				gameLoaded = true;
				mainLevel = 0;
			}
			break;
		}
		break;

	case 11: //dunno, possibly online multiplayer
		switch (select)
		{
		case 2:
			curMenu = 12;
			break;
		case 3:
			curMenu = 5;
			break;
		case 6:
			curMenu = 10;
			break;
		}
		break;

	case 12: //joy
		if (joysticks == 0 && select != 17)
			break;

		switch (select)
		{
		case 2:
			joystick_config++;
			joystick_config %= joysticks;
			break;
		case 3:
			joystick[joystick_config].analog = !joystick[joystick_config].analog;
			break;
		case 4:
			if (joystick[joystick_config].analog)
			{
				joystick[joystick_config].sensitivity++;
				joystick[joystick_config].sensitivity %= 11;
			}
			break;
		case 5:
			if (joystick[joystick_config].analog)
			{
				joystick[joystick_config].threshold++;
				joystick[joystick_config].threshold %= 11;
			}
			break;
		case 16:
			reset_joystick_assignments(joystick_config);
			break;
		case 17:
			if (isNetworkGame || onePlayerAction)
			{
				curMenu = 11;
			} else {
				curMenu = 2;
			}
			break;
		default:
			if (joysticks == 0)
				break;

			// int temp = 254;
			// JE_textShade(VGAScreen, 236, 38 + i * 8, value, temp / 16, temp % 16 - 8, DARKEN);

			JE_rectangle(VGAScreen, 235, 21 + select * 8, 310, 30 + select * 8, 248);

			Joystick_assignment temp;
			if (detect_joystick_assignment(joystick_config, &temp))
			{
				// if the detected assignment was already set, unset it
				for (uint i = 0; i < COUNTOF(*joystick->assignment); i++)
				{
					if (joystick_assignment_cmp(&temp, &joystick[joystick_config].assignment[select - 6][i]))
					{
						joystick[joystick_config].assignment[select - 6][i].type = NONE;
						goto joystick_assign_done;
					}
				}

				// if there is an empty assignment, set it
				for (uint i = 0; i < COUNTOF(*joystick->assignment); i++)
				{
					if (joystick[joystick_config].assignment[select - 6][i].type == NONE)
					{
						joystick[joystick_config].assignment[select - 6][i] = temp;
						goto joystick_assign_done;
					}
				}

				// if no assignments are empty, shift them all forward and set the last one
				for (uint i = 0; i < COUNTOF(*joystick->assignment); i++)
				{
					if (i == COUNTOF(*joystick->assignment) - 1)
						joystick[joystick_config].assignment[select - 6][i] = temp;
					else
						joystick[joystick_config].assignment[select - 6][i] = joystick[joystick_config].assignment[select - 6][i + 1];
				}

joystick_assign_done:
				curSelect++;

				poll_joysticks();
			}
		}
		break;

	case 13: //engage
		switch (curSel[curMenu])
		{
		case 2:
			mainLevel = mapSection[mapPNum-1];
			jumpSection = true;
			break;
		case 3:
			JE_doShipSpecs();
			break;
		case 4:
			curMenu = 2;
			break;
		case 5:
			if (JE_quitRequest())
			{
				if (isNetworkGame)
				{
					JE_tyrianHalt(0);
				}
				gameLoaded = true;
				mainLevel = 0;
			}
		}
		break;
	}

	old_items[0] = player[0].items;
}

void JE_drawShipSpecs( SDL_Surface * screen, SDL_Surface * temp_screen  )
{
	/* In this function we create our ship description image.
	 *
	 * We use a temp screen for convenience.  Bad design maybe (Jason!),
	 * but it'll be okay (and the alternative is malloc/a large stack) */

	int temp_x = 0, temp_y = 0, temp_index;
	Uint8 *src, *dst;


	//first, draw the text and other assorted flavoring.
	JE_clr256(screen);
	JE_drawLines(screen, true);
	JE_drawLines(screen, false);
	JE_rectangle(screen, 0, 0, 319, 199, 37);
	JE_rectangle(screen, 1, 1, 318, 198, 35);

	verticalHeight = 9;
	JE_outText(screen, 10, 2, ships[player[0].items.ship].name, 12, 3);
	JE_helpBox(screen, 100, 20, shipInfo[player[0].items.ship-1][0], 40);
	JE_helpBox(screen, 100, 100, shipInfo[player[0].items.ship-1][1], 40);
	verticalHeight = 7;

	JE_outText(screen, JE_fontCenter(miscText[4], TINY_FONT), 190, miscText[4], 12, 2);


	//now draw the green ship over that.
	//This hardcoded stuff is for positioning our little ship graphic
	if (player[0].items.ship > 90)
	{
		temp_index = 32;
	}
	else if (player[0].items.ship > 0)
	{
		temp_index = ships[player[0].items.ship].bigshipgraphic;
	}
	else
	{
		temp_index = ships[old_items[0].ship].bigshipgraphic;
	}

	switch (temp_index)
	{
		case 32:
			temp_x = 35;
			temp_y = 33;
			break;
		case 28:
			temp_x = 31;
			temp_y = 36;
			break;
		case 33:
			temp_x = 31;
			temp_y = 35;
			break;
		default:
			assert(0);
	}
	temp_x -= 30;


	//draw the ship into our temp buffer.
	JE_clr256(temp_screen);
	blit_sprite(temp_screen, temp_x, temp_y, OPTION_SHAPES, temp_index - 1);  // ship illustration

	/* But wait!  Our ship is fully colored, not green!
	 * With a little work we could get the sprite dimensions and greenify
	 * the area it resides in.  For now, let's just greenify the (almost
	 * entirely) black screen.

	 * We can't work in place.  In fact we'll need to overlay the result
	 * To avoid our temp screen dependence this has been rewritten to
	 * only write one line at a time.*/
	dst = screen->pixels;
	src = temp_screen->pixels;
	for (int y = 0; y < screen->h; y++)
	{
		for (int x = 0; x < screen->pitch; x++)
		{
			int avg = 0;
			if (y > 0)
				avg += *(src - screen->pitch) & 0x0f;
			if (y < screen->h - 1)
				avg += *(src + screen->pitch) & 0x0f;
			if (x > 0)
				avg += *(src - 1) & 0x0f;
			if (x < screen->pitch - 1)
				avg += *(src + 1) & 0x0f;
			avg /= 4;

			if ((*src & 0x0f) > avg)
			{
				*dst = (*src & 0x0f) | 0xc0;
			//} else {
			//	*dst = 0;
			}

			src++;
			dst++;
		}
	}
}

void JE_weaponSimUpdate( void )
{
	char buf[32];

	JE_weaponViewFrame();

	if ( (curSel[1] == 3 && curSel[4] < menuChoices[4]) || (curSel[1] == 4 && curSel[4] < menuChoices[4] - 1) )
	{
		if (leftPower)
		{
			sprintf(buf, "%d", downgradeCost);
			JE_outText(VGAScreen, 26, 137, buf, 1, 4);
		}
		else
		{
			blit_sprite(VGAScreenSeg, 24, 149, OPTION_SHAPES, 13);  // downgrade disabled
		}

		if (rightPower)
		{
			if (!rightPowerAfford)
			{
				sprintf(buf, "%d", upgradeCost);
				JE_outText(VGAScreen, 108, 137, buf, 7, 4);
				blit_sprite(VGAScreenSeg, 119, 149, OPTION_SHAPES, 14);  // upgrade disabled
			}
			else
			{
				sprintf(buf, "%d", upgradeCost);
				JE_outText(VGAScreen, 108, 137, buf, 1, 4);
			}
		}
		else
		{
			blit_sprite(VGAScreenSeg, 119, 149, OPTION_SHAPES, 14);  // upgrade disabled
		}

		temp = player[0].items.weapon[curSel[1]-3].power;

		for (int x = 1; x <= temp; x++)
		{
			fill_rectangle_xy(VGAScreen, 39 + x * 6, 151, 39 + x * 6 + 4, 151, 251);
			JE_pix(VGAScreen, 39 + x * 6, 151, 252);
			fill_rectangle_xy(VGAScreen, 39 + x * 6, 152, 39 + x * 6 + 4, 164, 250);
			fill_rectangle_xy(VGAScreen, 39 + x * 6, 165, 39 + x * 6 + 4, 165, 249);
		}

		sprintf(buf, "POWER: %d", temp);
		JE_outText(VGAScreen, 58, 137, buf, 15, 4);
	}
	else
	{
		leftPower = false;
		rightPower = false;
		blit_sprite(VGAScreenSeg, 20, 146, OPTION_SHAPES, 17);  // hide power level interface
	}

	JE_drawItem(1, player[0].items.ship, player[0].x - 5, player[0].y - 7);
}

void JE_weaponViewFrame( void )
{
	fill_rectangle_xy(VGAScreen, 8, 8, 143, 182, 0);

	/* JE: (* Port Configuration Display *)
	(*    drawportconfigbuttons;*/

	update_and_draw_starfield(VGAScreen, 1);

	mouseX = player[0].x;
	mouseY = player[0].y;

	// create shots in weapon simulator
	for (uint i = 0; i < 2; ++i)
	{
		if (shotRepeat[i] > 0)
		{
			--shotRepeat[i];
		}
		else
		{
			const uint item       = player[0].items.weapon[i].id,
			           item_power = player[0].items.weapon[i].power - 1,
			           item_mode = (i == REAR_WEAPON) ? player[0].weapon_mode - 1 : 0;

			b = player_shot_create(item, i, player[0].x, player[0].y, mouseX, mouseY, weaponPort[item].op[item_mode][item_power], 1);
		}
	}

	if (options[player[0].items.sidekick[LEFT_SIDEKICK]].wport > 0)
	{
		if (shotRepeat[SHOT_LEFT_SIDEKICK] > 0)
		{
			--shotRepeat[SHOT_LEFT_SIDEKICK];
		}
		else
		{
			const uint item = player[0].items.sidekick[LEFT_SIDEKICK];
			const int x = player[0].sidekick[LEFT_SIDEKICK].x,
			          y = player[0].sidekick[LEFT_SIDEKICK].y;

			b = player_shot_create(options[item].wport, SHOT_LEFT_SIDEKICK, x, y, mouseX, mouseY, options[item].wpnum, 1);
		}
	}

	if (options[player[0].items.sidekick[RIGHT_SIDEKICK]].tr == 2)
	{
		player[0].sidekick[RIGHT_SIDEKICK].x = player[0].x;
		player[0].sidekick[RIGHT_SIDEKICK].y = MAX(10, player[0].y - 20);
	}
	else
	{
		player[0].sidekick[RIGHT_SIDEKICK].x = 72 + 15;
		player[0].sidekick[RIGHT_SIDEKICK].y = 120;
	}

	if (options[player[0].items.sidekick[RIGHT_SIDEKICK]].wport > 0)
	{
		if (shotRepeat[SHOT_RIGHT_SIDEKICK] > 0)
		{
			--shotRepeat[SHOT_RIGHT_SIDEKICK];
		}
		else
		{
			const uint item = player[0].items.sidekick[RIGHT_SIDEKICK];
			const int x = player[0].sidekick[RIGHT_SIDEKICK].x,
			          y = player[0].sidekick[RIGHT_SIDEKICK].y;

			b = player_shot_create(options[item].wport, SHOT_RIGHT_SIDEKICK, x, y, mouseX, mouseY, options[item].wpnum, 1);
		}
	}

	simulate_player_shots();

	blit_sprite(VGAScreenSeg, 0, 0, OPTION_SHAPES, 12); // upgrade interface


	/*========================Power Bar=========================*/

	power += powerAdd;
	if (power > 900)
		power = 900;

	temp = power / 10;

	for (temp = 147 - temp; temp <= 146; temp++)
	{
		temp2 = 113 + (146 - temp) / 9 + 2;
		temp3 = (temp + 1) % 6;
		if (temp3 == 1)
			temp2 += 3;
		else if (temp3 != 0)
			temp2 += 2;

		JE_pix(VGAScreen, 141, temp, temp2 - 3);
		JE_pix(VGAScreen, 142, temp, temp2 - 3);
		JE_pix(VGAScreen, 143, temp, temp2 - 2);
		JE_pix(VGAScreen, 144, temp, temp2 - 1);
		fill_rectangle_xy(VGAScreen, 145, temp, 149, temp, temp2);

		if (temp2 - 3 < 112)
			temp2++;
	}

	temp = 147 - (power / 10);
	temp2 = 113 + (146 - temp) / 9 + 4;

	JE_pix(VGAScreen, 141, temp - 1, temp2 - 1);
	JE_pix(VGAScreen, 142, temp - 1, temp2 - 1);
	JE_pix(VGAScreen, 143, temp - 1, temp2 - 1);
	JE_pix(VGAScreen, 144, temp - 1, temp2 - 1);

	fill_rectangle_xy(VGAScreen, 145, temp-1, 149, temp-1, temp2);

	lastPower = temp;

	//JE_waitFrameCount();  TODO: didn't do anything?
}

