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


#include <stdlib.h>

#include "content/content.h"
#include "content/hlcache.h"
#include "content/urldb.h"
#include "desktop/browser.h"
#include "desktop/history_global_core.h"
#include "desktop/plotters.h"
#include "desktop/tree.h"
#include "desktop/tree_url_node.h"
#include "utils/messages.h"
#include "utils/utils.h"
#include "utils/log.h"

#define MAXIMUM_BASE_NODES 16
#define GLOBAL_HISTORY_RECENT_URLS 16
#define URL_CHUNK_LENGTH 512

static struct node *global_history_base_node[MAXIMUM_BASE_NODES];
static int global_history_base_node_time[MAXIMUM_BASE_NODES];
static int global_history_base_node_count = 0;

static bool global_history_initialised;

static struct tree *global_history_tree;
static struct node *global_history_tree_root;

static hlcache_handle *folder_icon;

static const char *const weekday_msg_name [] =
{
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday"
};

/**
 * Find an entry in the global history
 *
 * \param url The URL to find
 * \return Pointer to node, or NULL if not found
 */
static struct node *history_global_find(const char *url)
{
	int i;
	struct node *node;
	const char *text;

	for (i = 0; i < global_history_base_node_count; i++) {
		if (!tree_node_is_deleted(global_history_base_node[i])) {
			node = tree_node_get_child(global_history_base_node[i]);
			for (; node != NULL; node = tree_node_get_next(node)) {
				text = tree_url_node_get_url(node);
				if ((text != NULL) && !strcmp(url, text))
					return node;
			}
		}
	}
	return NULL;
}

/**
 * Internal routine to actually perform global history addition
 *
 * \param url The URL to add
 * \param data URL data associated with URL
 * \return true (for urldb_iterate_entries)
 */
static bool global_history_add_internal(nsurl *url, const struct url_data *data)
{
	int i, j;
	struct node *parent = NULL;
	struct node *link;
	struct node *node;
	bool before = false;
	int visit_date;

	assert((url != NULL) && (data != NULL));

	visit_date = data->last_visit;

	/* find parent node */
	for (i = 0; i < global_history_base_node_count; i++) {
		if (global_history_base_node_time[i] <= visit_date) {
			parent = global_history_base_node[i];
			break;
		}
	}

	/* the entry is too old to care about */
	if (parent == NULL)
		return true;

	if (tree_node_is_deleted(parent)) {
		/* parent was deleted, so find place to insert it */
		link = global_history_tree_root;

		for (j = global_history_base_node_count - 1; j >= 0; j--) {
			if (!tree_node_is_deleted(global_history_base_node[j]) &&
			    global_history_base_node_time[j] >
			    global_history_base_node_time[i]) {
				link = global_history_base_node[j];
				before = true;
				break;
			}
		}

		tree_set_node_selected(global_history_tree,
				       parent, true, false);
		tree_set_node_expanded(global_history_tree,
				       parent, false, true, true);
		tree_link_node(global_history_tree, link, parent, before);
	}

	/* find any previous occurance */
	if (global_history_initialised == false) {
		node = history_global_find(nsurl_access(url));
		if (node != NULL) {
			tree_update_URL_node(global_history_tree,
					     node, url, data);
			tree_delink_node(global_history_tree, node);
			tree_link_node(global_history_tree, parent, node,
				       false);
			return true;
		}
	}

	/* Add the node at the bottom */
	node = tree_create_URL_node_readonly(global_history_tree,
					   parent, url, data,
					   tree_url_node_callback, NULL);

	return true;
}

static node_callback_resp
history_global_node_callback(void *user_data,
			     struct node_msg_data *msg_data)
{
	if (msg_data->msg == NODE_DELETE_ELEMENT_IMG)
		return NODE_CALLBACK_HANDLED;
	return NODE_CALLBACK_NOT_HANDLED;
}

/**
 * Initialises a single grouping node for the global history tree.
 *
 * \return false on memory exhaustion, true otherwise
 */
static bool history_global_initialise_node(const char *title,
					   time_t base, int days_back)
{
	struct tm *full_time;
	char *buffer;
	struct node *node;

	base += days_back * 60 * 60 * 24;
	if (title == NULL) {
		full_time = localtime(&base);
		buffer = strdup(messages_get(weekday_msg_name[full_time->tm_wday]));
	} else {
		buffer = strdup(title);
	}

	if (buffer == NULL) {
		LOG(("malloc failed"));
		warn_user("NoMemory", 0);
		return false;
	}

	node = tree_create_folder_node(NULL, NULL, buffer,
				       false, true, true);
	if (node == NULL) {
		LOG(("malloc failed"));
		warn_user("NoMemory", 0);
		free(buffer);
		return false;
	}
	if (folder_icon != NULL)
		tree_set_node_icon(global_history_tree, node, folder_icon);
	tree_set_node_user_callback(node, history_global_node_callback, NULL);

	global_history_base_node[global_history_base_node_count] = node;
	global_history_base_node_time[global_history_base_node_count] = base;
	global_history_base_node_count++;

	return true;
}

/**
 * Initialises the grouping nodes(Today, Yesterday etc.) for the global history
 * tree.
 *
 * \return false on memory exhaustion, true otherwise
 */
static bool history_global_initialise_nodes(void)
{
	struct tm *full_time;
	time_t t;
	int weekday;
	int i;

	/* get the current time */
	t = time(NULL);
	if (t == -1) {
		LOG(("time info unaviable"));
		return false;
	}

	/* get the time at the start of today */
	full_time = localtime(&t);
	weekday = full_time->tm_wday;
	full_time->tm_sec = 0;
	full_time->tm_min = 0;
	full_time->tm_hour = 0;
	t = mktime(full_time);
	if (t == -1) {
		LOG(("mktime failed"));
		return false;
	}

	history_global_initialise_node(messages_get("DateToday"), t, 0);
	if (weekday > 0)
		if (!history_global_initialise_node(
			    messages_get("DateYesterday"), t, -1))
			return false;
	for (i = 2; i <= weekday; i++)
		if (!history_global_initialise_node(NULL, t, -i))
			return false;

	if (!history_global_initialise_node(messages_get("Date1Week"),
					    t, -weekday - 7))
		return false;
	if (!history_global_initialise_node(messages_get("Date2Week"),
					    t, -weekday - 14))
		return false;
	if (!history_global_initialise_node(messages_get("Date3Week"),
					    t, -weekday - 21))
		return false;

	return true;
}

/**
 * Initialises the global history tree.
 *
 * \param data		user data for the callbacks
 * \param start_redraw  callback function called before every redraw
 * \param end_redraw	callback function called after every redraw
 * \return true on success, false on memory exhaustion
 */
bool history_global_initialise(struct tree *tree, const char* folder_icon_name)
{
	folder_icon = tree_load_icon(folder_icon_name);
	tree_url_node_init(folder_icon_name);

	if (tree == NULL)
		return false;

	global_history_tree = tree;
	global_history_tree_root = tree_get_root(global_history_tree);

	if (!history_global_initialise_nodes())
		return false;

	LOG(("Building history tree"));

	global_history_initialised = true;
	urldb_iterate_entries(global_history_add_internal);
	global_history_initialised = false;
	tree_set_node_expanded(global_history_tree, global_history_tree_root,
			       false, true, true);
	LOG(("History tree built"));
	return true;
}


/**
 * Get flags with which the global history tree should be created;
 *
 * \return the flags
 */
unsigned int history_global_get_tree_flags(void)
{
	return TREE_NO_FLAGS;
}


/**
 * Deletes the global history tree.
 */
void history_global_cleanup(void)
{
	hlcache_handle_release(folder_icon);
	tree_url_node_cleanup();
}


/**
 * Adds a url to the global history.
 *
 * \param url the url to be added
 */
void global_history_add(nsurl *url)
{
	const struct url_data *data;

	data = urldb_get_url_data(url);
	if (data == NULL)
		return;

	global_history_add_internal(url, data);
}


/* Actions to be connected to front end specific toolbars */

/**
 * Save the global history in a human-readable form under the given location.
 *
 * \param path the path where the history will be saved
 */
bool history_global_export(const char *path)
{
	return tree_urlfile_save(global_history_tree, path, "NetSurf history");
}

/**
 * Delete nodes which are currently selected.
 */
void history_global_delete_selected(void)
{
	tree_delete_selected_nodes(global_history_tree,
				   global_history_tree_root);
}

/**
 * Delete all nodes.
 */
void history_global_delete_all(void)
{
	bool redraw_needed = tree_get_redraw(global_history_tree);
	if (redraw_needed)
		tree_set_redraw(global_history_tree, false);

	tree_set_node_selected(global_history_tree, global_history_tree_root,
			       true, true);
	tree_delete_selected_nodes(global_history_tree,
				   global_history_tree_root);

	if (redraw_needed)
		tree_set_redraw(global_history_tree, true);
}

/**
 * Select all nodes in the tree.
 */
void history_global_select_all(void)
{
	tree_set_node_selected(global_history_tree, global_history_tree_root,
			       true, true);
}

/**
 * Unselect all nodes.
 */
void history_global_clear_selection(void)
{
	tree_set_node_selected(global_history_tree, global_history_tree_root,
			       true, false);
}

/**
 * Expand grouping folders and history entries.
 */
void history_global_expand_all(void)
{
	tree_set_node_expanded(global_history_tree, global_history_tree_root,
			       true, true, true);
}

/**
 * Expand grouping folders only.
 */
void history_global_expand_directories(void)
{
	tree_set_node_expanded(global_history_tree, global_history_tree_root,
			       true, true, false);
}

/**
 * Expand history entries only.
 */
void history_global_expand_addresses(void)
{
	tree_set_node_expanded(global_history_tree, global_history_tree_root,
			       true, false, true);
}

/**
 * Collapse grouping folders and history entries.
 */
void history_global_collapse_all(void)
{
	tree_set_node_expanded(global_history_tree, global_history_tree_root,
			       false, true, true);
}

/**
 * Collapse grouping folders only.
 */
void history_global_collapse_directories(void)
{
	tree_set_node_expanded(global_history_tree, global_history_tree_root,
			       false, true, false);
}

/**
 * Collapse history entries only.
 */
void history_global_collapse_addresses(void)
{
	tree_set_node_expanded(global_history_tree, global_history_tree_root,
			       false, false, true);
}

/**
 * Open the selected entries in seperate browser windows.
 *
 * \param  tabs  open multiple entries in tabs in the new window
 */
void history_global_launch_selected(bool tabs)
{
	tree_launch_selected(global_history_tree, tabs);
}
