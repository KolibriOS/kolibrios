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
#include "sndmast.h"

#include "opentyr.h"

const char soundTitle[SAMPLE_COUNT][9] = /* [1..soundnum + 9] of string [8] */
{
	"SCALEDN2", /*1*/
	"F2",       /*2*/
	"TEMP10",
	"EXPLSM",
	"PASS3",    /*5*/
	"TEMP2",
	"BYPASS1",
	"EXP1RT",
	"EXPLLOW",
	"TEMP13",   /*10*/
	"EXPRETAP",
	"MT2BOOM",
	"TEMP3",
	"LAZB",     /*28K*/
	"LAZGUN2",  /*15*/
	"SPRING",
	"WARNING",
	"ITEM",
	"HIT2",     /*14K*/
	"MACHNGUN", /*20*/
	"HYPERD2",
	"EXPLHUG",
	"CLINK1",
	"CLICK",
	"SCALEDN1", /*25*/
	"TEMP11",
	"TEMP16",
	"SMALL1",
	"POWERUP",
	"VOICE1",
	"VOICE2",
	"VOICE3",
	"VOICE4",
	"VOICE5",
	"VOICE6",
	"VOICE7",
	"VOICE8",
	"VOICE9"
};

const JE_byte windowTextSamples[9] = /* [1..9] */
{
	V_DANGER,
	V_BOSS,
	V_ENEMIES,
	V_CLEARED_PLATFORM,
	V_DANGER,
	V_SPIKES,
	V_ACCELERATE,
	V_DANGER,
	V_ENEMIES
};

