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
#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "opentyr.h"
#include "config_file.h"

#include "SDL.h"

typedef enum
{
	NONE,
	AXIS,
	BUTTON,
	HAT
}
Joystick_assignment_types;

typedef struct
{
	Joystick_assignment_types type;
	int num;
	
	// if hat
	bool x_axis; // else y_axis
	
	// if hat or axis
	bool negative_axis; // else positive
}
Joystick_assignment;

typedef struct
{
	SDL_Joystick *handle;
	
	Joystick_assignment assignment[10][2]; // 0-3: directions, 4-9: actions
	
	bool analog;
	int sensitivity, threshold;
	
	signed int x, y;
	int analog_direction[4];
	bool direction[4], direction_pressed[4]; // up, right, down, left  (_pressed, for emulating key presses)
	
	bool confirm, cancel;
	bool action[6], action_pressed[6]; // fire, mode swap, left fire, right fire, menu, pause
	
	Uint32 joystick_delay;
	bool input_pressed;
}
Joystick;

extern int joystick_repeat_delay;
extern bool joydown;
extern bool ignore_joystick;
extern int joysticks;
extern Joystick *joystick;

int joystick_axis_reduce( int j, int value );
bool joystick_analog_angle( int j, float *angle );

void poll_joystick( int j );
void poll_joysticks( void );

void push_key( SDLKey key );
void push_joysticks_as_keyboard( void );

void init_joysticks( void );
void deinit_joysticks( void );

void reset_joystick_assignments( int j );
bool load_joystick_assignments( Config* config, int j );
bool save_joystick_assignments( Config* config, int j );

void joystick_assignments_to_string( char *buffer, size_t buffer_len, const Joystick_assignment *assignments );

bool detect_joystick_assignment( int j, Joystick_assignment *assignment );
bool joystick_assignment_cmp( const Joystick_assignment *, const Joystick_assignment * );

#endif /* JOYSTICK_H */

