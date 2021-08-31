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
#include "lvlmast.h"

#include "opentyr.h"

const JE_char shapeFile[34] = /* [1..34] */
{
	'2', '4', '7', '8', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
	'O', 'P', 'Q', 'R', 'S', 'T', 'U', '5', '#', 'V', '0', '@', '3', '^', '5', '9'
};

const char lvlFile[LVL_NUM][9] = /* [1..LVLnum] of string [8] */
{
/* 1*/ "ASTROID4",  "MAINMAPE",
/* 2*/ "ASTROID5",  "MAINMAPD",
/* 3*/ "ASTROID7",  "MAINMAPC",
/* 4*/ "BUBBLES",   "MAINMAPT",
/* 5*/ "DELI",      "MAINMAPS",
/* 6*/ "MINES2",    "MAINMAPQ",
/* 7*/ "MINES",     "MAINMAPI",
/* 8*/ "SAVARA",    "MAINMAPY",
/* 9*/ "TYRLEV",    "MAINMAPZ",
/*10*/ "BONUS1-1",  "MAINMAP0",
/*11*/ "HOLES",     "MAINMAP8",
/*12*/ "SAVARA3",   "MAINMAP3",
/*13*/ "DESERT",    "MAINMAAA",
/*14*/ "WINDY",     "MAINMAAB",
/*15*/ "TYRLEV3",   "MAINMAPZ",
/*16*/ "UNDERTYR",  "MAINMAAU",
/*17*/ "SAVARA5",   "MAINMAAW",
/*18*/ "BEER",      "MAINMAAX"
};

const char lvlFile2[LVL_NUM_2][9] = /* [1..LVLnum2] of string [8] */
{
	"JUNGLE",       "MAINMAPF",
	"GIGER",        "MAINMAPR",
	"BONUS1-2",     "MAINMAP0",
	"ASTCITY2",     "MAINMAP1",
	"SPACEATK",     "MAINMAPH",
	"STATION",      "MAINMAPV",
	"GEMWAR",       "MAINMAPH",
	"MARKERS",      "MAINMAPH",
	"PATHS",        "MAINMAPH",
	"JUNGLE2",      "MAINMAP5",
	"JUNGLE3",      "MAINMAP7",
	"JUNGLE4",      "MAINMAAP"
};

const char lvlFile3[LVL_NUM_3][9] = /* [1..LVLnum3] of string [8] */
{
	"ICE",          "MAINMAPX",
	"ASTCITY",      "MAINMAPP",
	"MINES3",       "MAINMAPO",
	"TUNNEL",       "MAINMAPW",
	"DELI2",        "MAINMAPU", /*Bonus 3*/
	"FLEET",        "MAINMAPB",
	"STARGATE",     "MAINMAP2", /*Bubbly*/
	"TYRLEV2",      "MAINMAPZ",
	"TUNNEL2",      "MAINMAPA", /*Tunnel*/
	"SAVARA2",      "MAINMAPY",
	"DELI3",        "MAINMAPS",
	"MACES",        "MAINMAP9"  /*Bonus Maces*/
};

const char lvlFile4[LVL_NUM_4][9] = /* [1..LVLnum4] of string [8] */
{
       "HARVEST",   "MAINMAAC",   /*Harvest World   ooooo */
       "MAZE",      "MAINMAAD",   /*Windy 2         ooooo */
       "SAVARA4",   "MAINMAAF",   /*Savara IV       ooooo */
       "DESERT3",   "MAINMAAG",   /*Desert          ooooo */
       "LAVA1",     "MAINMAAH",   /*Lava Core       ooooo */
       "LAVA2",     "MAINMAAJ",   /*Lava Center     ooooo */
       "CORE",      "MAINMAAI",   /*Tec tiles       ooooo */
       "ICE1",      "MAINMAAK",   /*Ice exit        ooooo */
       "LAVA3",     "MAINMAAL",   /*Lava Exit       ooooo */
       "DESERT4",   "MAINMAAM",   /*Desert Run      ooooo */
       "TUNNEL3",   "MAINMAAN",   /*Secret Tunnel   ooooo */
       "LAVA4",     "MAINMAAO",   /*Lava Run        ooooo */
       "EYESPY",    "MAINMAAQ",   /*Giger Eye       ooooo */
       "FLEET2",    "MAINMAPH",   /*Dread Not       ooooo */
       "BRAIN",     "MAINMAAR",   /*Brainiac        ooooo */
       "NOSE",      "MAINMAAS",   /*Nose            ooooo */
       "DESERT5",   "MAINMAAT",   /*Time War        ooooo */
       "GALAGA",    "MAINMAAV",   /*Galaga          ooooo */
/*19*/ "SAVARA6",   "MAINMAAY",   /*Savara Approach ooooo */
/*20*/ "SPACEAT2",  "MAINMABB"    /*Camanis Go      ooooo */
};

const char lvlFile5[LVL_NUM_5][9] = /* [1..lvlnum5] of string [8] */
{
/* 1*/ "E5LVL01",   "MAINMA51"    /*FogWalls        ----- */
};

/*! JE_LvlPosType lvlPos;*/

/*
 Episode 4 uses...
  NEWSH(.SHP
  NEWSH^.SHP
  NEWSH7.SHP
  NEWSHP.SHP
  NEWSH&.SHP
  NEWSHE.SHP
  NEWSHV.SHP
  NEWSH#.SHP
  NEWSHJ.SHP
  NEWSH{.SHP
  NEWSHK.SHP

  SHAPESZ.DAT
  SHAPESW.DAT
  SHAPESX.DAT
  SHAPES}.DAT
*/

/*
TYPE 5: Shape Files

      SHAPES1.DAT  o - - - -  Items
      SHAPES3.DAT  o - - - -  Shots
      SHAPES6.DAT  o - - - -  Explosions
      SHAPES9.DAT  o - - - -  Player ships/options

 1    SHAPES2.DAT  - o - - -  Tyrian ships
 2    SHAPES4.DAT  - o - - -  TyrianBoss
 3    SHAPES7.DAT  - - - - -  Iceships
 4    SHAPES8.DAT  - - - - -  Tunnel World
 5    SHAPESA.DAT  o - - - -  Mine Stuff
 6    SHAPESB.DAT  - - - - -  IceBoss
 7    SHAPESC.DAT  - o - - -  Deliani Stuff
 8    SHAPESD.DAT  o - - - -  Asteroid Stuff I
 9    SHAPESE.DAT  - o - - -  Tyrian Bonus Rock + Bubbles
 10   SHAPESF.DAT  - o - - -  Savara Stuff I
 11   SHAPESG.DAT  - - - - -  Giger Stuff
 12   SHAPESH.DAT  - - - - -  Giger Stuff
 13   SHAPESI.DAT  - o - - -  Savara Stuff II + Savara Boss
 14   SHAPESJ.DAT  - - - - -  Jungle Stuff
 15   SHAPESK.DAT  - - - - -  Snowballs
 16   SHAPESL.DAT  - - - - o  Satellites
 17   SHAPESM.DAT  o - - - -  Asteroid Stuff IV
 18   SHAPESN.DAT  - - - - -  Giger Boss
 19   SHAPESO.DAT  - o - - -  Savara Boss
 20   SHAPESP.DAT  o - - - -  Asteroid Stuff III
 21   SHAPESQ.DAT  o - - - -  Coins and Gems
 22   SHAPESR.DAT  - - - - -  TunnelWorld Boss
 23   SHAPESS.DAT  o - - - -  Asteroid Stuff II
 24   SHAPEST.DAT  - o - - -  Deli Boss
 25   SHAPESU.DAT  - - - - -  Deli Stuff II
 28   SHAPESV.DAT  - - o - o  Misc Stuff/Cars
 27   SHAPES#.DAT  - - - o o  Sawblades
 31   SHAPES(.DAT  - - - - o  Desert/Lava

                   M 1 2 3 4  episode
*/

