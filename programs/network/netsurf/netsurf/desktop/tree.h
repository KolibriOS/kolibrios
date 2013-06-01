/*
 * Copyright 2004 Richard Wilson <not_ginger_matt@users.sourceforge.net>
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
 * Generic tree handling (interface).
 */

#ifndef _NETSURF_DESKTOP_TREE_H_
#define _NETSURF_DESKTOP_TREE_H_

#include <stdbool.h>
#include <stdint.h>

#include "desktop/browser.h"
#include "image/bitmap.h"

struct hlcache_handle;

/* Tree flags */
enum tree_flags {
	TREE_NO_FLAGS = 0,
	TREE_NO_DRAGS = 1,
	TREE_NO_FURNITURE = 2,
	TREE_SINGLE_SELECT = 4,
	TREE_NO_SELECT = 8,
	TREE_MOVABLE = 16,
	TREE_DELETE_EMPTY_DIRS = 32, /**< if the last child of a
				      * directory is deleted the
				      * directory will be deleted
				      * too.
				      */
};

/** A "flag" value to indicate the element data contains title
 * text. This value should be the first node_element in every
 * node. All other values should be different than this one. The term
 * flag is misused as it is actually a value used by the API consumer
 * to indicate teh type of data a node element contains.
 */
#define TREE_ELEMENT_TITLE	0x00
#define TREE_ELEMENT_LAUNCH_IN_TABS	0x05 /* Launch in tabs instead of windows */

struct tree;
struct node;
struct node_element;

typedef enum {
	TREE_NO_DRAG = 0,
	TREE_SELECT_DRAG,
	TREE_MOVE_DRAG,
	TREE_TEXTAREA_DRAG,	/** < A drag that is passed to a textarea */
	TREE_UNKNOWN_DRAG	/** < A drag the tree itself won't handle */
} tree_drag_type;

typedef enum {
	NODE_ELEMENT_TEXT,		/**< Text only */
	NODE_ELEMENT_TEXT_PLUS_ICON,	/**< Text and icon */
	NODE_ELEMENT_BITMAP		/**< Bitmap only */
} node_element_type;

typedef enum {
	NODE_DELETE_ELEMENT_TXT, /**< The text of an element of the
				  * node is being deleted */
 	NODE_DELETE_ELEMENT_IMG, /**< The bitmap or icon of a node is
				  * being deleted */
 	NODE_LAUNCH, /**< The node has been launched */
	NODE_ELEMENT_EDIT_CANCELLED, /**< Editing opperation cancelled.  */
	NODE_ELEMENT_EDIT_FINISHING, /**< New text has to be accepted
				      * or rejected.  */
  	NODE_ELEMENT_EDIT_FINISHED /**< Editing of a node_element has
				    * been finished. */
} node_msg;

typedef enum {
	NODE_CALLBACK_HANDLED,
	NODE_CALLBACK_NOT_HANDLED,
	NODE_CALLBACK_REJECT, /**< reject new text for node element
			       * and leave editing mode. */
	NODE_CALLBACK_CONTINUE /**< don't leave editig mode. */
} node_callback_resp;

/** Internal node message. */
struct node_msg_data {
	node_msg msg; /**< The type of message. */
	unsigned int flag; /**< message flags. */
	struct node *node; /**< tree node messsage concerns. */
	union {
		char *text; /**< textural data. */
		void *bitmap; /**< bitmap data. */
		struct browser_window *bw; /**< clone browser_window. */
	} data; /**< The message data. */
};

/** callbacks to perform necessary operations on treeview. */
struct treeview_table {
	void (*redraw_request)(int x, int y, int width, int height,
			       void *data); /**< request a redraw. */
	void (*resized)(struct tree *tree, int width, int height,
			void *data); /**< resize treeview area. */
	void (*scroll_visible)(int y, int height, void *data); /**< scroll visible treeview area. */
	void (*get_window_dimensions)(int *width, int *height, void *data); /**< get dimensions of window */
};

/**
 * Informs the client about any events requiring his action
 *
 * \param user_data	the user data which was passed at tree creation
 * \param msg_data	structure containing all the message information
 * \return		the appropriate node_callback_resp response
 */
typedef node_callback_resp (*tree_node_user_callback)(void *user_data,
		struct node_msg_data *msg_data);

/* Non-platform specific code */

void tree_set_icon_dir(char *icon_dir);
void tree_setup_colours(void);

/* Functions for creating/deleting tree primitives and for tree structure
   manipulation */
struct tree *tree_create(unsigned int flags,
		const struct treeview_table *callbacks,
  		void *client_data);
struct node *tree_create_folder_node(struct tree *tree, struct node *parent,
		const char *title, bool editable, bool retain_in_memory,
  		bool deleted);
struct node *tree_create_leaf_node(struct tree *tree, struct node *parent,
		const char *title, bool editable, bool retain_in_memory,
  		bool deleted);
struct node_element *tree_create_node_element(struct node *parent,
		node_element_type type, unsigned int flag, bool editable);
void tree_link_node(struct tree *tree, struct node *link, struct node *node,
		bool before);
void tree_delink_node(struct tree *tree, struct node *node);
void tree_delete(struct tree *tree);
void tree_delete_node(struct tree *tree, struct node *node, bool siblings);

/* setters and getters for properties and data */
void tree_set_node_icon(struct tree *tree, struct node *node,
		struct hlcache_handle *icon);
void tree_set_node_expanded(struct tree *tree, struct node *node, bool expanded,
		bool folder, bool leaf);
void tree_set_node_selected(struct tree *tree, struct node *node, bool all,
		bool selected);
void tree_set_node_selected_at(struct tree *tree, int x, int y, bool selected);
void tree_set_node_sort_function(struct tree *tree, struct node *node,
		int (*sort) (struct node *, struct node *));
void tree_set_node_user_callback(struct node *node,
		tree_node_user_callback callback, void *data);
void tree_set_redraw(struct tree *tree, bool redraw);
bool tree_get_redraw(struct tree *tree);
bool tree_node_has_selection(struct node *node);
bool tree_node_is_deleted(struct node *node);
bool tree_node_is_folder(struct node *node);
bool tree_node_is_default(struct node *node);
void tree_update_node_element(struct tree *tree, struct node_element *element,
		const char *text, void *bitmap);
bool tree_update_element_text(struct tree *tree, struct node_element *element, char *text);
const char *tree_node_element_get_text(struct node_element *element);
struct node *tree_get_root(struct tree *tree);
bool tree_is_edited(struct tree *tree);
tree_drag_type tree_drag_status(struct tree *tree);

struct node *tree_get_default_folder_node(struct tree *tree);
bool tree_set_default_folder_node(struct tree *tree, struct node *node);
void tree_clear_default_folder_node(struct tree *tree);

/* functions for traversing the tree */
struct node *tree_node_get_parent(struct node *node);
struct node *tree_node_get_child(struct node *node);
struct node *tree_node_get_next(struct node *node);

void tree_draw(struct tree *tree, int x, int y,
		int clip_x, int clip_y, int clip_width, int clip_height,
		const struct redraw_context *ctx);

struct node_element *tree_node_find_element(struct node *node,
		unsigned int flag, struct node_element *after);
void tree_delete_selected_nodes(struct tree *tree, struct node *node);
struct node *tree_get_selected_node(struct node *node);
struct node *tree_get_link_details(struct tree *tree, int x, int y,
		bool *before);
void tree_launch_selected(struct tree *tree, bool tabs);

bool tree_mouse_action(struct tree *tree, browser_mouse_state mouse,
		int x, int y);
void tree_drag_end(struct tree *tree, browser_mouse_state mouse, int x0, int y0,
		int x1, int y1);
bool tree_keypress(struct tree *tree, uint32_t key);

int tree_alphabetical_sort(struct node *, struct node *);
void tree_start_edit(struct tree *tree, struct node_element *element);
struct hlcache_handle *tree_load_icon(const char *name);

#endif
