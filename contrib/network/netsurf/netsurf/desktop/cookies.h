/*
 * Copyright 2006 Richard Wilson <info@tinct.net>
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
 * Cookies (interface).
 */

#ifndef _NETSURF_DESKTOP_COOKIES_H_
#define _NETSURF_DESKTOP_COOKIES_H_

#include <stdbool.h>

#include "desktop/tree.h"

struct cookie_data;

bool cookies_initialise(struct tree *tree, const char* folder_icon_name, const char* cookie_icon_name);
unsigned int cookies_get_tree_flags(void);

/**
 * Perform cookie updates and addition. The update is only scheduled here.
 * The actual update is performed in the callback function.
 *
 * \param data Data of cookie being updated.
 * \return true (for urldb_iterate_entries)
 */
bool cookies_schedule_update(const struct cookie_data *data);

/**
 * Remove a cookie from the active set.
 * The cookie is to be removed from the active set and no futher
 * references made to the cookie data.
 *
 * \param data Data of cookie being removed.
 */
void cookies_remove(const struct cookie_data *data);

void cookies_cleanup(void);

void cookies_delete_selected(void);
void cookies_delete_all(void);
void cookies_select_all(void);
void cookies_clear_selection(void);
void cookies_expand_all(void);
void cookies_expand_domains(void);
void cookies_expand_cookies(void);
void cookies_collapse_all(void);
void cookies_collapse_domains(void);
void cookies_collapse_cookies(void);

#endif
