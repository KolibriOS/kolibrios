/*
 * Copyright 2004, 2005 Richard Wilson <info@tinct.net>
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
 
 
 
/** \file
 * Hotlist (interface).
 */

#ifndef _NETSURF_DESKTOP_HOTLIST_H_
#define _NETSURF_DESKTOP_HOTLIST_H_

#include <stdbool.h>

#include "desktop/tree.h"

/**
 * Initialise the hotlist from a frontend.
 *
 * \param tree The tree object which holds the hotlist.
 * \param hotlist_path The file path to initialise the hotlist entries from.
 * \param folder_icon_name The name to use for folder icons.
 */
bool hotlist_initialise(struct tree *tree, const char *hotlist_path, const char* folder_icon_name);

unsigned int hotlist_get_tree_flags(void);
void hotlist_cleanup(const char *hotlist_path);

bool hotlist_export(const char *path);
void hotlist_edit_selected(void);
void hotlist_delete_selected(void);
void hotlist_select_all(void);
void hotlist_clear_selection(void);
void hotlist_expand_all(void);
void hotlist_expand_directories(void);
void hotlist_expand_addresses(void);
void hotlist_collapse_all(void);
void hotlist_collapse_directories(void);
void hotlist_collapse_addresses(void);
void hotlist_add_folder(bool selected);
void hotlist_add_entry(bool selected);
void hotlist_add_page(const char *url);
void hotlist_add_page_xy(const char *url, int x, int y);
void hotlist_launch_selected(bool tabs);
bool hotlist_set_default_folder(bool clear);
#endif
