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
 
/** \file
 * Creation of URL nodes with use of trees public API
 */
 
#ifndef _NETSURF_DESKTOP_TREE_URL_NODE_H_
#define _NETSURF_DESKTOP_TREE_URL_NODE_H_


#include "desktop/tree.h"
#include "utils/nsurl.h"

struct url_data;

void tree_url_node_init(const char *folder_icon_name);
void tree_url_node_cleanup(void);
struct node *tree_create_URL_node(struct tree *tree,
		struct node *parent, nsurl *url, const char *title,
    		tree_node_user_callback, void *callback_data);
struct node *tree_create_URL_node_readonly(struct tree *tree,
		struct node *parent, nsurl *url,
  		const struct url_data *data,
    		tree_node_user_callback, void *callback_data);
void tree_update_URL_node(struct tree *tree, struct node *node,
		nsurl *url, const struct url_data *data);
const char *tree_url_node_get_title(struct node *node);
const char *tree_url_node_get_url(struct node *node);
void tree_url_node_edit_title(struct tree *tree, struct node *node);
void tree_url_node_edit_url(struct tree *tree, struct node *node);

node_callback_resp tree_url_node_callback(void *user_data,
		struct node_msg_data *msg_data);

bool tree_urlfile_load(const char *filename, struct tree *tree,
		tree_node_user_callback, void *callback_data);
bool tree_urlfile_save(struct tree *tree, const char *filename,
		const char *page_title);

/* front end specific */
void tree_icon_name_from_content_type(char *buffer, content_type type);

#endif
