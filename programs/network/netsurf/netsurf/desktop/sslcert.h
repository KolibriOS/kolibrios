/*
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


#ifndef _NETSURF_DESKTOP_SSLCERT_H_
#define _NETSURF_DESKTOP_SSLCERT_H_

#include <stdbool.h>

#include "desktop/tree.h"

struct sslcert_session_data;

void sslcert_init(const char* icon_name);
unsigned int sslcert_get_tree_flags(void);
void sslcert_cleanup(void);

struct sslcert_session_data *sslcert_create_session_data(unsigned long num,
		nsurl *url, llcache_query_response cb, void *cbpw);
bool sslcert_load_tree(struct tree *tree,
		const struct ssl_cert_info *certs,
		struct sslcert_session_data *data);
		
bool sslcert_reject(struct sslcert_session_data *session);
bool sslcert_accept(struct sslcert_session_data *session);

		
#endif
