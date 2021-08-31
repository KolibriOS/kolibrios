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
#ifndef EDITSHIP_H
#define EDITSHIP_H

#include "opentyr.h"


typedef JE_byte JE_ShipsType[154]; /* [1..154] */

extern JE_boolean extraAvail;
extern JE_ShipsType extraShips;
extern void *extraShapes;
extern JE_word extraShapeSize;

void JE_decryptShips( void );
void JE_loadExtraShapes( void );

#endif /* EDITSHIP_H */

