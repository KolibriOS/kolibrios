/*
 * Copyright 2006 Richard Wilson <info@tinct.net>
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
 * Cookies (implementation).
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "content/content.h"
#include "content/hlcache.h"
#include "content/urldb.h"
#include "desktop/cookies.h"
#include "desktop/options.h"
#include "desktop/tree.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "utils/schedule.h"
#include "utils/url.h"
#include "utils/utils.h"

/** Flags for each type of cookie tree node. */
enum tree_element_cookie {
	TREE_ELEMENT_PERSISTENT = 0x01,
	TREE_ELEMENT_VERSION = 0x02,
	TREE_ELEMENT_SECURE = 0x03,
	TREE_ELEMENT_LAST_USED = 0x04,
	TREE_ELEMENT_EXPIRES = 0x05,
	TREE_ELEMENT_PATH = 0x06,
	TREE_ELEMENT_DOMAIN = 0x07,
	TREE_ELEMENT_COMMENT = 0x08,
	TREE_ELEMENT_VALUE = 0x09,
};

static struct tree *cookies_tree;
static struct node *cookies_tree_root;
static bool user_delete;
static hlcache_handle *folder_icon;
static hlcache_handle *cookie_icon;


/**
 * Find an entry in the cookie tree
 *
 * \param node the node to check the children of
 * \param title The title to find
 * \return Pointer to node, or NULL if not found
 */
static struct node *cookies_find(struct node *node, const char *title)
{
	struct node *search;
	struct node_element *element;

	assert(node !=NULL);

	for (search = tree_node_get_child(node); search;
	     search = tree_node_get_next(search)) {
		element = tree_node_find_element(search, TREE_ELEMENT_TITLE,
						 NULL);
		if (strcmp(title, tree_node_element_get_text(element)) == 0)
			return search;
	}
	return NULL;
}

/**
 * Callback for all cookie tree nodes.
 */
static node_callback_resp cookies_node_callback(void *user_data, struct node_msg_data *msg_data)
{
	struct node *node = msg_data->node;
	struct node_element *domain, *path;
	const char *domain_t, *path_t, *name_t;
	char *space;
	bool is_folder = tree_node_is_folder(node);

	/* we don't remove any icons here */
	if (msg_data->msg == NODE_DELETE_ELEMENT_IMG)
		return NODE_CALLBACK_HANDLED;

	/* let the tree handle events other than text data removal */
	if (msg_data->msg != NODE_DELETE_ELEMENT_TXT)
		return NODE_CALLBACK_NOT_HANDLED;

	/* check if it's a domain folder */
	if (is_folder)
		return NODE_CALLBACK_NOT_HANDLED;

	switch (msg_data->flag) {
	case TREE_ELEMENT_TITLE:
		if (!user_delete)
			break;
		/* get the rest of the cookie data */
		domain = tree_node_find_element(node,
						TREE_ELEMENT_DOMAIN, NULL);
		path = tree_node_find_element(node, TREE_ELEMENT_PATH,
					      NULL);

		if ((domain != NULL) && 
		    (path != NULL)) {
			domain_t = tree_node_element_get_text(domain) +
				strlen(messages_get(
					       "TreeDomain")) - 4;
			space = strchr(domain_t, ' ');
			if (space != NULL)
				*space = '\0';
			path_t = tree_node_element_get_text(path) +
				strlen(messages_get("TreePath"))
				- 4;
			space = strchr(path_t, ' ');
			if (space != NULL)
				*space = '\0';
			name_t = msg_data->data.text;
			urldb_delete_cookie(domain_t, path_t, name_t);
		}
		break;
	default:
		break;
	}

	free(msg_data->data.text);

	return NODE_CALLBACK_HANDLED;
}


/**
 * Updates a tree entry for a cookie.
 *
 * All information is copied from the cookie_data, and as such can
 * be edited and should be freed.
 *
 * \param node The node to update
 * \param data The cookie data to use
 * \return true if node updated, or false for failure
 */
static bool cookies_update_cookie_node(struct node *node,
				       const struct cookie_data *data)
{
	struct node_element *element;
	char buffer[32];

	assert(data != NULL);

	/* update the value text */
	element = tree_node_find_element(node, TREE_ELEMENT_VALUE, NULL);
	tree_update_element_text(cookies_tree,
				 element, 
			    messages_get_buff("TreeValue",
					      data->value != NULL ?
					      data->value : 
					      messages_get("TreeUnused")));


	/* update the comment text */
	if ((data->comment != NULL) &&
	    (strcmp(data->comment, "") != 0)) {
		element = tree_node_find_element(node, TREE_ELEMENT_COMMENT, NULL);
		tree_update_element_text(cookies_tree,
					 element, 
				    messages_get_buff("TreeComment", 
						      data->comment));
	}

	/* update domain text */
	element = tree_node_find_element(node, TREE_ELEMENT_DOMAIN, element);
	tree_update_element_text(cookies_tree,
				 element, 
			    messages_get_buff("TreeDomain",
					      data->domain, 
					      data->domain_from_set ? 
					      messages_get("TreeHeaders") : 
					      ""));

	/* update path text */
	element = tree_node_find_element(node, TREE_ELEMENT_PATH, element);
	tree_update_element_text(cookies_tree,
				 element, 
			    messages_get_buff("TreePath", data->path,
					      data->path_from_set ?
					      messages_get("TreeHeaders") : 
					      ""));

	/* update expiry text */
	element = tree_node_find_element(node, TREE_ELEMENT_EXPIRES, element);
	tree_update_element_text(cookies_tree,
				 element, 
			    messages_get_buff("TreeExpires",
					      (data->expires > 0)
					      ? (data->expires == 1)
					      ? messages_get("TreeSession")
					      : ctime(&data->expires)
					      : messages_get("TreeUnknown")));

	/* update last used text */
	element = tree_node_find_element(node, TREE_ELEMENT_LAST_USED, element);
	tree_update_element_text(cookies_tree,
				 element, 
			    messages_get_buff("TreeLastUsed",
					      (data->last_used > 0) ?
					      ctime(&data->last_used) :
					      messages_get("TreeUnknown")));

	/* update secure text */
	element = tree_node_find_element(node, TREE_ELEMENT_SECURE, element);
	tree_update_element_text(cookies_tree,
				 element, 
			    messages_get_buff("TreeSecure",
					      data->secure ?
					      messages_get("Yes") : 
					      messages_get("No")));

	/* update version text */
	element = tree_node_find_element(node, TREE_ELEMENT_VERSION, element);
	snprintf(buffer, sizeof(buffer), "TreeVersion%i", data->version);
	tree_update_element_text(cookies_tree,
				 element, 
			    messages_get_buff("TreeVersion",
					      messages_get(buffer)));

	/* update persistant text */
	element = tree_node_find_element(node, TREE_ELEMENT_PERSISTENT, element);
	tree_update_element_text(cookies_tree,
				 element, 
			    messages_get_buff("TreePersistent",
					      data->no_destroy ?
					      messages_get("Yes") : 
					      messages_get("No")));

	return true;
}

/**
 * Creates an empty tree entry for a cookie, and links it into the tree.
 *
 * All information is copied from the cookie_data, and as such can
 * be edited and should be freed.
 *
 * \param parent      the node to link to
 * \param data	      the cookie data to use
 * \return the node created, or NULL for failure
 */
static struct node *cookies_create_cookie_node(struct node *parent,
					       const struct cookie_data *data)
{
	struct node *node;
	char *name;

	name = strdup(data->name);
	if (name == NULL) {
		LOG(("malloc failed"));
		warn_user("NoMemory", 0);
		return NULL;
	}

	node = tree_create_leaf_node(cookies_tree, NULL, name,
				     false, false, false);
	if (node == NULL) {
		free(name);
		return NULL;
	}

	tree_set_node_user_callback(node, cookies_node_callback, NULL);

	tree_create_node_element(node, NODE_ELEMENT_TEXT,
				 TREE_ELEMENT_PERSISTENT, false);

	tree_create_node_element(node, NODE_ELEMENT_TEXT,
				 TREE_ELEMENT_VERSION, false);

	tree_create_node_element(node, NODE_ELEMENT_TEXT,
				 TREE_ELEMENT_SECURE, false);

	tree_create_node_element(node, NODE_ELEMENT_TEXT,
				 TREE_ELEMENT_LAST_USED, false);

	tree_create_node_element(node, NODE_ELEMENT_TEXT,
				 TREE_ELEMENT_EXPIRES, false);

	tree_create_node_element(node, NODE_ELEMENT_TEXT,
				 TREE_ELEMENT_PATH, false);

	tree_create_node_element(node, NODE_ELEMENT_TEXT,
				 TREE_ELEMENT_DOMAIN, false);

	if ((data->comment) && (strcmp(data->comment, "")))
		tree_create_node_element(node, NODE_ELEMENT_TEXT,
					 TREE_ELEMENT_COMMENT, false);
	tree_create_node_element(node, NODE_ELEMENT_TEXT,
				 TREE_ELEMENT_VALUE, false);
	tree_set_node_icon(cookies_tree, node, cookie_icon);

	if (!cookies_update_cookie_node(node, data))
	{
		tree_delete_node(NULL, node, false);
		return NULL;
	}

	tree_link_node(cookies_tree, parent, node, false);
	return node;
}


/**
 * Called when scheduled event gets fired. Actually performs the update.
 */
static void cookies_schedule_callback(const void *scheduled_data)
{
	const struct cookie_data *data = scheduled_data;
	struct node *node = NULL;
	struct node *cookie_node = NULL;
	char *domain_cp;

	assert(data != NULL);

	node = cookies_find(cookies_tree_root, data->domain);

	if (node == NULL) {
		domain_cp = strdup(data->domain);
		if (domain_cp == NULL) {
			LOG(("malloc failed"));
			warn_user("NoMemory", 0);
			return;
		}
		/* ownership of domain_cp passed to tree, if node creation
		 * does not fail */
		node = tree_create_folder_node(cookies_tree,
					       cookies_tree_root, domain_cp,
					       false, false, false);
		if (node != NULL) {
			tree_set_node_user_callback(node, cookies_node_callback,
						    NULL);
			tree_set_node_icon(cookies_tree, node, folder_icon);

		} else {
			free(domain_cp);
		}
	}

	if (node == NULL)
		return;

	cookie_node = cookies_find(node, data->name);
	if (cookie_node == NULL)
		cookies_create_cookie_node(node, data);
	else
		cookies_update_cookie_node(cookie_node, data);

	return;
}

/**
 * Initialises cookies tree.
 *
 * \param data		user data for the callbacks
 * \param start_redraw  callback function called before every redraw
 * \param end_redraw	callback function called after every redraw
 * \return true on success, false on memory exhaustion
 */
bool cookies_initialise(struct tree *tree, const char* folder_icon_name, const char* cookie_icon_name)
{

	if (tree == NULL)
		return false;

	folder_icon = tree_load_icon(folder_icon_name);
	cookie_icon = tree_load_icon(cookie_icon_name);

	/* Create an empty tree */
	cookies_tree = tree;
	cookies_tree_root = tree_get_root(cookies_tree);

	user_delete = false;
	urldb_iterate_cookies(cookies_schedule_update);
	tree_set_node_expanded(cookies_tree, cookies_tree_root,
			       false, true, true);

	return true;
}


/**
 * Get flags with which the cookies tree should be created;
 *
 * \return the flags
 */
unsigned int cookies_get_tree_flags(void)
{
	return TREE_DELETE_EMPTY_DIRS;
}


/* exported interface documented in cookies.h */
bool cookies_schedule_update(const struct cookie_data *data)
{
	assert(data != NULL);
	assert(user_delete == false);

	if (cookies_tree_root != NULL)
		cookies_schedule_callback(data);

	return true;
}


/* exported interface documented in cookies.h */
void cookies_remove(const struct cookie_data *data)
{
	assert(data != NULL);

	if (cookies_tree_root != NULL)
		cookies_schedule_callback(data);
}


/**
 * Free memory and release all other resources.
 */
void cookies_cleanup(void)
{
	hlcache_handle_release(folder_icon);
	hlcache_handle_release(cookie_icon);
}

/* Actions to be connected to front end specific toolbars */

/**
 * Delete nodes which are currently selected.
 */
void cookies_delete_selected(void)
{
	user_delete = true;
	tree_delete_selected_nodes(cookies_tree, cookies_tree_root);
	user_delete = false;
}

/**
 * Delete all nodes.
 */
void cookies_delete_all(void)
{
	bool needs_redraw = tree_get_redraw(cookies_tree);
	if (needs_redraw)
		tree_set_redraw(cookies_tree, false);

	user_delete = true;
	tree_set_node_selected(cookies_tree, cookies_tree_root, true, true);
	tree_delete_selected_nodes(cookies_tree, cookies_tree_root);
	user_delete = false;

	if (needs_redraw)
		tree_set_redraw(cookies_tree, true);
}

/**
 * Select all nodes in the tree.
 */
void cookies_select_all(void)
{
	tree_set_node_selected(cookies_tree, cookies_tree_root, true, true);
}

/**
 * Unselect all nodes.
 */
void cookies_clear_selection(void)
{
	tree_set_node_selected(cookies_tree, cookies_tree_root, true, false);
}

/**
 * Expand both domain and cookie nodes.
 */
void cookies_expand_all(void)
{
	tree_set_node_expanded(cookies_tree, cookies_tree_root,
			       true, true, true);
}

/**
 * Expand domain nodes only.
 */
void cookies_expand_domains(void)
{
	tree_set_node_expanded(cookies_tree, cookies_tree_root,
			       true, true, false);
}

/**
 * Expand cookie nodes only.
 */
void cookies_expand_cookies(void)
{
	tree_set_node_expanded(cookies_tree, cookies_tree_root,
			       true, false, true);
}

/**
 * Collapse both domain and cookie nodes.
 */
void cookies_collapse_all(void)
{
	tree_set_node_expanded(cookies_tree, cookies_tree_root,
			       false, true, true);
}

/**
 * Collapse domain nodes only.
 */
void cookies_collapse_domains(void)
{
	tree_set_node_expanded(cookies_tree, cookies_tree_root,
			       false, true, false);
}

/**
 * Collapse cookie nodes only.
 */
void cookies_collapse_cookies(void)
{
	tree_set_node_expanded(cookies_tree, cookies_tree_root,
			       false, false, true);
}
