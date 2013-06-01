/*
 * Copyright 2011 Daniel Silverstone <dsilvers@netsurf-browser.org>
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
 * Job scheduler (interface).
 */

#ifndef _NETSURF_UTILS_SCHEDULE_H_
#define _NETSURF_UTILS_SCHEDULE_H_

/* In platform specific schedule.c. */
typedef void (*schedule_callback_fn)(void *p);

void schedule(int t, schedule_callback_fn callback, void *p);
void schedule_remove(schedule_callback_fn callback, void *p);

#endif
