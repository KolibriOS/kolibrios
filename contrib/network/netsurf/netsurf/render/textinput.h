/*
 * Copyright 2003 Phil Mellor <monkeyson@users.sourceforge.net>
 * Copyright 2004 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2004 Andrew Timmins <atimmins@blueyonder.co.uk>
 * Copyright 2004 John Tytgat <joty@netsurf-browser.org>
 *
 * This file is part of NetSurf, http://www.netsurf-browser.org/
 *
 * NetSurf is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * NetSurf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \file
 * HTML form text input handling (interface)
 */

#ifndef _NETSURF_RENDER_TEXTINPUT_H_
#define _NETSURF_RENDER_TEXTINPUT_H_

#include <stdbool.h>

struct box;
struct content;


void textinput_textarea_click(struct content *c, browser_mouse_state mouse,
		struct box *textarea, int box_x, int box_y, int x, int y);

void textinput_input_click(struct content *c, struct box *input,
		int box_x, int box_y, int x, int y);

#endif
