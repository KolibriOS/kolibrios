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
 * Creation of URL nodes with use of trees (implementation)
 */


#include <assert.h>
#include <ctype.h>

#include <dom/dom.h>
#include <dom/bindings/hubbub/parser.h>

#include "content/content.h"
#include "content/hlcache.h"
#include "content/urldb.h"
#include "desktop/browser.h"
#include "desktop/options.h"
#include "desktop/tree_url_node.h"
#include "utils/corestrings.h"
#include "utils/libdom.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "utils/url.h"
#include "utils/utf8.h"
#include "utils/utils.h"

/** Flags for each type of url tree node. */
enum tree_element_url {
	TREE_ELEMENT_URL = 0x01,
	TREE_ELEMENT_LAST_VISIT = 0x02,
	TREE_ELEMENT_VISITS = 0x03,
	TREE_ELEMENT_THUMBNAIL = 0x04,
};

#define MAX_ICON_NAME_LEN 256

static bool initialised = false;

static hlcache_handle *folder_icon;

struct icon_entry {
	content_type type;
	hlcache_handle *icon;
};

struct icon_entry icon_table[] = {
	{CONTENT_HTML, NULL},
	{CONTENT_TEXTPLAIN, NULL},
	{CONTENT_CSS, NULL},
	{CONTENT_IMAGE, NULL},
	{CONTENT_NONE, NULL},

	/* this serves as a sentinel */
	{CONTENT_HTML, NULL}
};

static uint32_t tun_users = 0;

void tree_url_node_init(const char *folder_icon_name)
{
	struct icon_entry *entry;
	char icon_name[MAX_ICON_NAME_LEN];
	
	tun_users++;
	
	if (initialised)
		return;
	initialised = true;

	folder_icon = tree_load_icon(folder_icon_name);

	entry = icon_table;
	do {

		tree_icon_name_from_content_type(icon_name, entry->type);
		entry->icon = tree_load_icon(icon_name);

		++entry;
	} while (entry->type != CONTENT_HTML);
}


void tree_url_node_cleanup()
{
	struct icon_entry *entry;
	
	tun_users--;
	
	if (tun_users > 0)
		return;
	
	if (!initialised)
		return;
	initialised = false;
	
	hlcache_handle_release(folder_icon);
	
	entry = icon_table;
	do {
		hlcache_handle_release(entry->icon);
		++entry;
	} while (entry->type != CONTENT_HTML);
}

/**
 * Creates a tree entry for a URL, and links it into the tree
 *
 * \param parent     the node to link to
 * \param url        the URL (copied)
 * \param data	     the URL data to use
 * \param title	     the custom title to use
 * \return the node created, or NULL for failure
 */
struct node *tree_create_URL_node(struct tree *tree, struct node *parent,
		nsurl *url, const char *title,
		tree_node_user_callback user_callback, void *callback_data)
{
	struct node *node;
	struct node_element *element;
	char *text_cp, *squashed;

	squashed = squash_whitespace(title ? title : nsurl_access(url));
	text_cp = strdup(squashed);
	if (text_cp == NULL) {
		LOG(("malloc failed"));
		warn_user("NoMemory", 0);
		return NULL;
	}
	free(squashed);
	node = tree_create_leaf_node(tree, parent, text_cp, true, false,
				     false);
	if (node == NULL) {
		free(text_cp);
		return NULL;
	}

	if (user_callback != NULL)
		tree_set_node_user_callback(node, user_callback,
					    callback_data);

	tree_create_node_element(node, NODE_ELEMENT_BITMAP,
				 TREE_ELEMENT_THUMBNAIL, false);
	tree_create_node_element(node, NODE_ELEMENT_TEXT, TREE_ELEMENT_VISITS,
				 false);
	tree_create_node_element(node, NODE_ELEMENT_TEXT,
				 TREE_ELEMENT_LAST_VISIT, false);
	element = tree_create_node_element(node, NODE_ELEMENT_TEXT,
					   TREE_ELEMENT_URL, true);
	if (element != NULL) {
		text_cp = strdup(nsurl_access(url));
		if (text_cp == NULL) {
			tree_delete_node(tree, node, false);
			LOG(("malloc failed"));
			warn_user("NoMemory", 0);
			return NULL;
		}
		tree_update_node_element(tree, element, text_cp, NULL);
	}

	return node;
}


/**
 * Creates a read only tree entry for a URL, and links it into the tree.
 *
 * \param parent      the node to link to
 * \param url         the URL
 * \param data	      the URL data to use
 * \return the node created, or NULL for failure
 */
struct node *tree_create_URL_node_readonly(struct tree *tree, 
		struct node *parent, nsurl *url, 
		const struct url_data *data, 
		tree_node_user_callback user_callback, void *callback_data)
{
	struct node *node;
	struct node_element *element;
	char *title;

	assert(url && data);

	if (data->title != NULL) {
		title = strdup(data->title);
	} else {
		title = strdup(nsurl_access(url));
	}

	if (title == NULL)
		return NULL;

	node = tree_create_leaf_node(tree, parent, title, false, false, false);
	if (node == NULL) {
		free(title);
		return NULL;
	}

	if (user_callback != NULL) {
		tree_set_node_user_callback(node, user_callback,
					    callback_data);
	}

	tree_create_node_element(node, NODE_ELEMENT_BITMAP,
				 TREE_ELEMENT_THUMBNAIL, false);
	tree_create_node_element(node, NODE_ELEMENT_TEXT, TREE_ELEMENT_VISITS,
				 false);
	tree_create_node_element(node, NODE_ELEMENT_TEXT,
				 TREE_ELEMENT_LAST_VISIT, false);
	element = tree_create_node_element(node, NODE_ELEMENT_TEXT,
					   TREE_ELEMENT_URL, false);
	if (element != NULL) {
		tree_update_node_element(tree, element, nsurl_access(url),
				NULL);
	}

	tree_update_URL_node(tree, node, url, data);

	return node;
}


/**
 * Updates the node details for a URL node.
 *
 * \param node  the node to update
 */
void tree_update_URL_node(struct tree *tree, struct node *node,
		nsurl *url, const struct url_data *data)
{
	struct node_element *element;
	struct bitmap *bitmap = NULL;
	struct icon_entry *entry;
	char *text_cp;

	assert(node != NULL);

	element = tree_node_find_element(node, TREE_ELEMENT_URL, NULL);
	if (element == NULL)
		return;

	if (data != NULL) {
		if (data->title == NULL)
			urldb_set_url_title(url, nsurl_access(url));

		if (data->title == NULL)
			return;

		element = tree_node_find_element(node, TREE_ELEMENT_TITLE,
						 NULL);
			
		text_cp = strdup(data->title);
		if (text_cp == NULL) {
			LOG(("malloc failed"));
			warn_user("NoMemory", 0);
			return;
		}
		tree_update_node_element(tree, element,	text_cp, NULL);
	} else {
		data = urldb_get_url_data(url);
		if (data == NULL)
			return;
	}

	entry = icon_table;
	do {
		if (entry->type == data->type) {
			if (entry->icon != NULL)
				tree_set_node_icon(tree, node, entry->icon);
			break;
		}
		++entry;
	} while (entry->type != CONTENT_HTML);

	/* update last visit text */
	element = tree_node_find_element(node, TREE_ELEMENT_LAST_VISIT, element);
	tree_update_element_text(tree,
		element, 
		messages_get_buff("TreeLast",
			(data->last_visit > 0) ?
			ctime((time_t *)&data->last_visit) :
			messages_get("TreeUnknown")));


	/* update number of visits text */
	element = tree_node_find_element(node, TREE_ELEMENT_VISITS, element);
	tree_update_element_text(tree,
		element, 
		messages_get_buff("TreeVisits", data->visits));


	/* update thumbnail */
	element = tree_node_find_element(node, TREE_ELEMENT_THUMBNAIL, element);
	if (element != NULL) {
		bitmap = urldb_get_thumbnail(url);

		if (bitmap != NULL) {
			tree_update_node_element(tree, element, NULL, bitmap);
		}
	}
}


const char *tree_url_node_get_title(struct node *node)
{
	struct node_element *element;
	element = tree_node_find_element(node, TREE_ELEMENT_TITLE, NULL);
	if (element == NULL)
		return NULL;
	return tree_node_element_get_text(element);
}


const char *tree_url_node_get_url(struct node *node)
{
	struct node_element *element;
	element = tree_node_find_element(node, TREE_ELEMENT_URL, NULL);
	if (element == NULL)
		return NULL;
	return tree_node_element_get_text(element);
}

void tree_url_node_edit_title(struct tree *tree, struct node *node)
{
	struct node_element *element;
	element = tree_node_find_element(node, TREE_ELEMENT_TITLE, NULL);
	tree_start_edit(tree, element);
}

void tree_url_node_edit_url(struct tree *tree, struct node *node)
{
	struct node_element *element;
	element = tree_node_find_element(node, TREE_ELEMENT_URL, NULL);
	tree_start_edit(tree, element);
}

node_callback_resp tree_url_node_callback(void *user_data,
					  struct node_msg_data *msg_data)
{
	struct tree *tree;
	struct node_element *element;
	nsurl *nsurl;
	nserror error;
	const char *text;
	char *norm_text;
	const struct url_data *data;

	/** @todo memory leaks on non-shared folder deletion. */
	switch (msg_data->msg) {
	case NODE_DELETE_ELEMENT_TXT:
		switch (msg_data->flag) {
			/* only history is using non-editable url
			 * elements so only history deletion will run
			 * this code
			 */
		case TREE_ELEMENT_URL:
			/* reset URL characteristics */
			error = nsurl_create(msg_data->data.text, &nsurl);
			if (error != NSERROR_OK) {
				warn_user("NoMemory", 0);
				return NODE_CALLBACK_REJECT;
			}
			urldb_reset_url_visit_data(nsurl);
			nsurl_unref(nsurl);
			return NODE_CALLBACK_HANDLED;
		case TREE_ELEMENT_TITLE:
			return NODE_CALLBACK_HANDLED;
		}
		break;
	case NODE_DELETE_ELEMENT_IMG:
		if (msg_data->flag == TREE_ELEMENT_THUMBNAIL ||
		    msg_data->flag == TREE_ELEMENT_TITLE)
			return NODE_CALLBACK_HANDLED;
		break;
	case NODE_LAUNCH:
		element = tree_node_find_element(msg_data->node,
						 TREE_ELEMENT_URL, NULL);
		if (element != NULL) {
			text = tree_node_element_get_text(element);
			if (msg_data->flag == TREE_ELEMENT_LAUNCH_IN_TABS) {
				msg_data->data.bw = browser_window_create(text,
						msg_data->data.bw, 0, true, true);
			} else {
				browser_window_create(text, NULL, 0,
						      true, false);
			}
			return NODE_CALLBACK_HANDLED;
		}
		break;
	case NODE_ELEMENT_EDIT_FINISHING:

		text = msg_data->data.text;

		if (msg_data->flag == TREE_ELEMENT_URL) {
			size_t len;
			error = nsurl_create(text, &nsurl);
			if (error != NSERROR_OK) {
				warn_user("NoMemory", 0);
				return NODE_CALLBACK_REJECT;
			}
			error = nsurl_get(nsurl, NSURL_WITH_FRAGMENT,
					&norm_text, &len);
			if (error != NSERROR_OK) {
				warn_user("NoMemory", 0);
				return NODE_CALLBACK_REJECT;
			}

			msg_data->data.text = norm_text;

			data = urldb_get_url_data(nsurl);
			if (data == NULL) {
				urldb_add_url(nsurl);
				urldb_set_url_persistence(nsurl, true);
				data = urldb_get_url_data(nsurl);
				if (data == NULL) {
					nsurl_unref(nsurl);
					return NODE_CALLBACK_REJECT;
				}
			}
			tree = user_data;
			tree_update_URL_node(tree, msg_data->node,
					     nsurl, NULL);
			nsurl_unref(nsurl);
		}
		else if (msg_data->flag == TREE_ELEMENT_TITLE) {
			while (isspace(*text))
				text++;
			norm_text = strdup(text);
			if (norm_text == NULL) {
				LOG(("malloc failed"));
				warn_user("NoMemory", 0);
				return NODE_CALLBACK_REJECT;
			}
			/* don't allow zero length entry text, return
			   false */
			if (norm_text[0] == '\0') {
				warn_user("NoNameError", 0);
				msg_data->data.text = NULL;
				return NODE_CALLBACK_CONTINUE;
			}
			msg_data->data.text = norm_text;
		}

		return NODE_CALLBACK_HANDLED;
	default:
		break;
	}
	return NODE_CALLBACK_NOT_HANDLED;
}

typedef struct {
	struct tree *tree;
	struct node *directory;
	tree_node_user_callback callback;
	void *callback_data;
	bool last_was_h4;
	dom_string *title;
} tree_url_load_ctx;

static void tree_url_load_directory(dom_node *ul, tree_url_load_ctx *ctx);

/**
 * Parse an entry represented as a li.
 *
 * \param  li         DOM node for parsed li
 * \param  directory  directory to add this entry to
 */
static void tree_url_load_entry(dom_node *li, tree_url_load_ctx *ctx)
{
	dom_node *a;
	dom_string *title1;
	dom_string *url1;
	char *title, *url2;
	nsurl *url;
	const struct url_data *data;
	struct node *entry;
	dom_exception derror;
	nserror error;

	/* The li must contain an "a" element */
	a = libdom_find_first_element(li, corestring_lwc_a);
	if (a == NULL) {
		warn_user("TreeLoadError", "(Missing <a> in <li>)");
		return;
	}

	derror = dom_node_get_text_content(a, &title1);
	if (derror != DOM_NO_ERR) {
		warn_user("TreeLoadError", "(No title)");
		dom_node_unref(a);
		return;
	}

	derror = dom_element_get_attribute(a, corestring_dom_href, &url1);
	if (derror != DOM_NO_ERR || url1 == NULL) {
		warn_user("TreeLoadError", "(No URL)");
		dom_string_unref(title1);
		dom_node_unref(a);
		return;
	}

	if (title1 != NULL) {
		title = strndup(dom_string_data(title1),
				dom_string_byte_length(title1));
		dom_string_unref(title1);
	} else {
		title = strdup("");
	}
	if (title == NULL) {
		warn_user("NoMemory", NULL);
		dom_string_unref(url1);
		dom_node_unref(a);
		return;
	}

	/* We're loading external input.
	 * This may be garbage, so attempt to normalise via nsurl
	 */
	url2 = strndup(dom_string_data(url1), dom_string_byte_length(url1));
	if (url2 == NULL) {
		warn_user("NoMemory", NULL);
		free(title);
		dom_string_unref(url1);
		dom_node_unref(a);
		return;
	}

	dom_string_unref(url1);

	error = nsurl_create(url2, &url);

	free(url2);

	if (error != NSERROR_OK) {
		LOG(("Failed normalising '%s'", url2));

		warn_user("NoMemory", NULL);

		free(title);
		dom_node_unref(a);

		return;
	}

	data = urldb_get_url_data(url);
	if (data == NULL) {
		/* No entry in database, so add one */
		urldb_add_url(url);
		/* now attempt to get url data */
		data = urldb_get_url_data(url);
	}
	if (data == NULL) {
		nsurl_unref(url);
		free(title);
		dom_node_unref(a);

		return;
	}

	/* Make this URL persistent */
	urldb_set_url_persistence(url, true);

	/* Force the title in the hotlist */
	urldb_set_url_title(url, title);

	entry = tree_create_URL_node(ctx->tree, ctx->directory, url, title,
				     ctx->callback, ctx->callback_data);

 	if (entry == NULL) {
 		/** \todo why isn't this fatal? */
 		warn_user("NoMemory", 0);
 	} else {
		tree_update_URL_node(ctx->tree, entry, url, data);
	}

	nsurl_unref(url);
	free(title);
	dom_node_unref(a);
}

static bool tree_url_load_directory_cb(dom_node *node, void *ctx)
{
	tree_url_load_ctx *tctx = ctx;
	dom_string *name;
	dom_exception error;

	/* The ul may contain entries as a li, or directories as
	 * an h4 followed by a ul. Non-element nodes may be present
	 * (eg. text, comments), and are ignored. */

	error = dom_node_get_node_name(node, &name);
	if (error != DOM_NO_ERR || name == NULL)
		return false;

	if (dom_string_caseless_lwc_isequal(name, corestring_lwc_li)) {
		/* entry */
		tree_url_load_entry(node, tctx);
		tctx->last_was_h4 = false;
	} else if (dom_string_caseless_lwc_isequal(name, corestring_lwc_h4)) {
		/* directory (a) */
		dom_string *title;

		error = dom_node_get_text_content(node, &title);
		if (error != DOM_NO_ERR || title == NULL) {
			warn_user("TreeLoadError", "(Empty <h4> "
					"or memory exhausted.)");
			dom_string_unref(name);
			return false;
		}

		if (tctx->title != NULL)
			dom_string_unref(tctx->title);
		tctx->title = title;
		tctx->last_was_h4 = true;
	} else if (tctx->last_was_h4 && dom_string_caseless_lwc_isequal(name, 
			corestring_lwc_ul)) {
		/* directory (b) */
		dom_string *id;
		bool dir_is_default;
		struct node *dir;
		char *title;
		tree_url_load_ctx new_ctx;

		error = dom_element_get_attribute(node, corestring_dom_id, &id);
		if (error != DOM_NO_ERR) {
			dom_string_unref(name);
			return false;
		}

		if (id != NULL) {
			dir_is_default = dom_string_caseless_lwc_isequal(id,
					corestring_lwc_default);

			dom_string_unref(id);
		} else {
			dir_is_default = false;
		}

		title = strndup(dom_string_data(tctx->title),
				dom_string_byte_length(tctx->title));
		if (title == NULL) {
			dom_string_unref(name);
			return false;
		}

		dir = tree_create_folder_node(tctx->tree, tctx->directory,
				title, true, false, false);
		if (dir == NULL) {
			dom_string_unref(name);
			return false;
		}

		if (dir_is_default)
			tree_set_default_folder_node(tctx->tree, dir);

		if (tctx->callback != NULL)
			tree_set_node_user_callback(dir, tctx->callback,
						    tctx->callback_data);

		if (folder_icon != NULL)
			tree_set_node_icon(tctx->tree, dir, folder_icon);

		new_ctx.tree = tctx->tree;
		new_ctx.directory = dir;
		new_ctx.callback = tctx->callback;
		new_ctx.callback_data = tctx->callback_data;
		new_ctx.last_was_h4 = false;
		new_ctx.title = NULL;

		tree_url_load_directory(node, &new_ctx);

		if (new_ctx.title != NULL) {
			dom_string_unref(new_ctx.title);
			new_ctx.title = NULL;
		}
		tctx->last_was_h4 = false;
	} else {
		tctx->last_was_h4 = false;
	}

	dom_string_unref(name);

	return true;
}

/**
 * Parse a directory represented as a ul.
 *
 * \param  ul         DOM node for parsed ul
 * \param  directory  directory to add this directory to
 */
static void tree_url_load_directory(dom_node *ul, tree_url_load_ctx *ctx)
{
	assert(ul != NULL);
	assert(ctx != NULL);
	assert(ctx->directory != NULL);

	libdom_iterate_child_elements(ul, tree_url_load_directory_cb, ctx);
}

/**
 * Loads an url tree from a specified file.
 *
 * \param  filename  	name of file to read
 * \param  tree		empty tree which data will be read into
 * \return the file represented as a tree, or NULL on failure
 */
bool tree_urlfile_load(const char *filename, struct tree *tree,
		       tree_node_user_callback callback, void *callback_data)
{
	dom_document *document;
	dom_node *html, *body, *ul;
	struct node *root;
	nserror error;
	tree_url_load_ctx ctx;

	if (filename == NULL) {
		return false;
	}

	error = libdom_parse_file(filename, "iso-8859-1", &document);
	if (error != NSERROR_OK) {
		if (error != NSERROR_NOT_FOUND) {
			warn_user("TreeLoadError", messages_get("ParsingFail"));
		}
		return false;
	}

	html = libdom_find_first_element((dom_node *) document,
			corestring_lwc_html);
	if (html == NULL) {
		dom_node_unref(document);
		warn_user("TreeLoadError", "(<html> not found)");
		return false;
	}

	body = libdom_find_first_element(html, corestring_lwc_body);
	if (body == NULL) {
		dom_node_unref(html);
		dom_node_unref(document);
		warn_user("TreeLoadError", "(<html>...<body> not found)");
		return false;
	}

	ul = libdom_find_first_element(body, corestring_lwc_ul);
	if (ul == NULL) {
		dom_node_unref(body);
		dom_node_unref(html);
		dom_node_unref(document);
		warn_user("TreeLoadError",
			  "(<html>...<body>...<ul> not found.)");
		return false;
	}

	root = tree_get_root(tree);

	ctx.tree = tree;
	ctx.directory = root;
	ctx.callback = callback;
	ctx.callback_data = callback_data;
	ctx.last_was_h4 = false;
	ctx.title = NULL;

	tree_url_load_directory(ul, &ctx);
	tree_set_node_expanded(tree, root, true, false, false);

	if (ctx.title != NULL) {
		dom_string_unref(ctx.title);
		ctx.title = NULL;
	}

	dom_node_unref(ul);
	dom_node_unref(body);
	dom_node_unref(html);
	dom_node_unref(document);

	return true;
}

/**
 * Add an entry to the HTML tree for saving.
 *
 * The node must contain a sequence of node_elements in the following order:
 *
 * \param  entry  hotlist entry to add
 * \param  fp     File to write to
 * \return  true on success, false on memory exhaustion
 */
static bool tree_url_save_entry(struct node *entry, FILE *fp)
{
	const char *href, *text;
	char *latin1_href, *latin1_text;
	utf8_convert_ret ret;

	text = tree_url_node_get_title(entry);
	if (text == NULL)
		return false;

	href = tree_url_node_get_url(entry);
	if (href == NULL)
		return false;

	ret = utf8_to_html(text, "iso-8859-1", strlen(text), &latin1_text);
	if (ret != UTF8_CONVERT_OK)
		return false;

	ret = utf8_to_html(href, "iso-8859-1", strlen(href), &latin1_href);
	if (ret != UTF8_CONVERT_OK) {
		free(latin1_text);
		return false;
	}

	fprintf(fp, "<li><a href=\"%s\">%s</a></li>",
			latin1_href, latin1_text);

	free(latin1_href);
	free(latin1_text);

	return true;
}

/**
 * Add a directory to the HTML tree for saving.
 *
 * \param  directory  hotlist directory to add
 * \param  fp         File to write to
 * \return  true on success, false on memory exhaustion
 */
static bool tree_url_save_directory(struct node *directory, FILE *fp)
{
	struct node *child;

	fputs("<ul", fp);
	if (tree_node_is_default(directory))
		fputs(" id=\"default\"", fp);
	fputc('>', fp);

	if (tree_node_get_child(directory) != NULL)
		fputc('\n', fp);

	for (child = tree_node_get_child(directory); child != NULL;
	     child = tree_node_get_next(child)) {
		if (tree_node_is_folder(child) == false) {
			/* entry */
			if (tree_url_save_entry(child, fp) == false)
				return false;
		} else {
			/* directory */
			/* invalid HTML */
			const char *text;
			char *latin1_text;
			utf8_convert_ret ret;

			text = tree_url_node_get_title(child);
			if (text == NULL)
				return false;

			ret = utf8_to_html(text, "iso-8859-1",
					strlen(text), &latin1_text);
			if (ret != UTF8_CONVERT_OK)
				return false;

			fprintf(fp, "<h4>%s</h4>\n", latin1_text);

			free(latin1_text);

			if (tree_url_save_directory(child, fp) == false)
				return false;
		}

		fputc('\n', fp);
	}

	fputs("</ul>", fp);

	return true;
}


/**
 * Perform a save to a specified file in the form of a html page
 *
 * \param filename	the file to save to
 * \param page_title 	title of the page
 */
bool tree_urlfile_save(struct tree *tree, const char *filename,
		       const char *page_title)
{
	FILE *fp;

	fp = fopen(filename, "w");
	if (fp == NULL)
		return NULL;

	/* Unfortunately the Browse Hotlist format is invalid HTML,
	 * so this is a lie. 
	 */
	fputs("<!DOCTYPE html "
		"PUBLIC \"//W3C/DTD HTML 4.01//EN\" "
		"\"http://www.w3.org/TR/html4/strict.dtd\">\n", fp);
	fputs("<html>\n<head>\n", fp);
	fputs("<meta http-equiv=\"Content-Type\" "
		"content=\"text/html; charset=iso-8859-1\">\n", fp);
	fprintf(fp, "<title>%s</title>\n", page_title);
	fputs("</head>\n<body>", fp);

	if (tree_url_save_directory(tree_get_root(tree), fp) == false) {
		warn_user("HotlistSaveError", 0);
		fclose(fp);
 		return false;
 	}

	fputs("</body>\n</html>\n", fp);

	fclose(fp);

	return true;
}

