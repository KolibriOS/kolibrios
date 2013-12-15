/*
 * Copyright 2005 Richard Wilson <info@tinct.net>
 * Copyright 2009 Paul Blokus <paul_pl@users.sourceforge.net>
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
 
#ifndef _NETSURF_DESKTOP_HISTORY_GLOBAL_H_
#define _NETSURF_DESKTOP_HISTORY_GLOBAL_H_

#include <stdbool.h>

#include "desktop/tree.h"

bool history_global_initialise(struct tree *tree, const char* folder_icon_name);
unsigned int history_global_get_tree_flags(void);
void history_global_cleanup(void);

bool history_global_export(const char *path);
void history_global_delete_selected(void);
void history_global_delete_all(void);
void history_global_select_all(void);
void history_global_clear_selection(void);
void history_global_expand_all(void);
void history_global_expand_directories(void);
void history_global_expand_addresses(void);
void history_global_collapse_all(void);
void history_global_collapse_directories(void);
void history_global_collapse_addresses(void);
void history_global_launch_selected(bool tabs);

#endif
