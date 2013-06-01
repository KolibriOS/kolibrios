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

/** \file
 * SSL Certificate verification UI (implementation)
 */

#include "utils/config.h"

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "content/content.h"
#include "content/fetch.h"
#include "content/hlcache.h"
#include "content/urldb.h"
#include "desktop/browser.h"
#include "desktop/sslcert.h"
#include "desktop/tree.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "utils/nsurl.h"
#include "utils/utils.h"

/** Flags for each type of ssl tree node. */
enum tree_element_ssl {
	TREE_ELEMENT_SSL_VERSION = 0x01,
	TREE_ELEMENT_SSL_VALID_FROM = 0x02,
	TREE_ELEMENT_SSL_VALID_TO = 0x03,
	TREE_ELEMENT_SSL_CERT_TYPE = 0x04,
	TREE_ELEMENT_SSL_SERIAL = 0x05,
	TREE_ELEMENT_SSL_ISSUER = 0x06,
};

/** ssl certificate verification context. */
struct sslcert_session_data {
	unsigned long num; /**< The number of ssl certificates in the chain */
	nsurl *url; /**< The url of the certificate */
	struct tree *tree; /**< The root of the treeview */
	llcache_query_response cb; /**< callback when cert is accepted or rejected */
	void *cbpw; /**< context passed to callback */
};

/** Handle for the window icon. */
static hlcache_handle *sslcert_icon = NULL;

/** Initialise ssl certificate window. */
void sslcert_init(const char* icon_name)
{
	sslcert_icon = tree_load_icon(icon_name);
}


/**
 * Get flags with which the sslcert tree should be created;
 *
 * \return the flags
 */
unsigned int sslcert_get_tree_flags(void)
{
	return TREE_NO_DRAGS  | TREE_NO_SELECT;
}


void sslcert_cleanup(void)
{
	if (sslcert_icon != NULL)
		hlcache_handle_release(sslcert_icon);
}

struct sslcert_session_data *
sslcert_create_session_data(unsigned long num,
			    nsurl *url, 
			    llcache_query_response cb, 
			    void *cbpw)
{
	struct sslcert_session_data *data;

	data = malloc(sizeof(struct sslcert_session_data));
	if (data == NULL) {
		warn_user("NoMemory", 0);
		return NULL;
	}
	data->url = nsurl_ref(url);
	if (data->url == NULL) {
		free(data);
		warn_user("NoMemory", 0);
		return NULL;
	}
	data->num = num;
	data->cb = cb;
	data->cbpw = cbpw;

	return data;
}

static node_callback_resp sslcert_node_callback(void *user_data,
						struct node_msg_data *msg_data)
{
	if (msg_data->msg == NODE_DELETE_ELEMENT_IMG)
		return NODE_CALLBACK_HANDLED;
	return NODE_CALLBACK_NOT_HANDLED;
}

static struct node *sslcert_create_node(const struct ssl_cert_info *cert)
{
	struct node *node;
	struct node_element *element;
	char *text;

	text = messages_get_buff("SSL_Certificate_Subject", cert->subject);
	if (text == NULL)
		return NULL;

	node = tree_create_leaf_node(NULL, NULL, text, false, false, false);
	if (node == NULL) {
		free(text);
		return NULL;
	}
	tree_set_node_user_callback(node, sslcert_node_callback, NULL);

	/* add issuer node */
	element = tree_create_node_element(node, NODE_ELEMENT_TEXT,
					   TREE_ELEMENT_SSL_ISSUER, false);
	if (element != NULL) {
		text = messages_get_buff("SSL_Certificate_Issuer", cert->issuer);
		if (text == NULL) {
			tree_delete_node(NULL, node, false);
			return NULL;
		}
		tree_update_node_element(NULL, element, text, NULL);
	}

	/* add version node */
	element = tree_create_node_element(node, NODE_ELEMENT_TEXT,
					   TREE_ELEMENT_SSL_VERSION, false);
	if (element != NULL) {
		text = messages_get_buff("SSL_Certificate_Version", cert->version);
		if (text == NULL) {
			tree_delete_node(NULL, node, false);
			return NULL;
		}
		tree_update_node_element(NULL, element, text, NULL);
	}

	/* add valid from node */
	element = tree_create_node_element(node, NODE_ELEMENT_TEXT,
					   TREE_ELEMENT_SSL_VALID_FROM, false);
	if (element != NULL) {
		text = messages_get_buff("SSL_Certificate_ValidFrom", cert->not_before);
		if (text == NULL) {
			tree_delete_node(NULL, node, false);
			return NULL;
		}
		tree_update_node_element(NULL, element, text, NULL);
	}


	/* add valid to node */
	element = tree_create_node_element(node, NODE_ELEMENT_TEXT,
					   TREE_ELEMENT_SSL_VALID_TO, false);
	if (element != NULL) {
		text = messages_get_buff("SSL_Certificate_ValidTo", cert->not_after);
		if (text == NULL) {
			tree_delete_node(NULL, node, false);
			return NULL;
		}
		tree_update_node_element(NULL, element, text, NULL);
	}

	/* add certificate type */
	element = tree_create_node_element(node, NODE_ELEMENT_TEXT,
					   TREE_ELEMENT_SSL_CERT_TYPE, false);
	if (element != NULL) {
		text = messages_get_buff("SSL_Certificate_Type", cert->cert_type);
		if (text == NULL) {
			tree_delete_node(NULL, node, false);
			return NULL;
		}
		tree_update_node_element(NULL, element, text, NULL);
	}

	/* add serial node */
	element = tree_create_node_element(node, NODE_ELEMENT_TEXT,
					   TREE_ELEMENT_SSL_SERIAL, false);
	if (element != NULL) {
		text = messages_get_buff("SSL_Certificate_Serial", cert->serial);
		if (text == NULL) {
			tree_delete_node(NULL, node, false);
			return NULL;
		}
		tree_update_node_element(NULL, element, text, NULL);
	}

	/* set the display icon */
	tree_set_node_icon(NULL, node, sslcert_icon);

	return node;
}

bool sslcert_load_tree(struct tree *tree, 
		       const struct ssl_cert_info *certs,
		       struct sslcert_session_data *data)
{
	struct node *tree_root;
	struct node *node;
	unsigned long cert_loop;

	assert(data != NULL && certs != NULL && tree != NULL);

	tree_root = tree_get_root(tree);

	for (cert_loop = 0; cert_loop < data->num; cert_loop++) {
		node = sslcert_create_node(&(certs[cert_loop]));
		if (node != NULL) {
			/* There is no problem creating the node
			 * add an entry for it in the root of the
			 * treeview .
			 */
			tree_link_node(tree, tree_root, node, false);
		}
	}

	data->tree = tree;

	return true;

}


static void sslcert_cleanup_session(struct sslcert_session_data *session)
{
	assert(session != NULL);

	if (session->url)
		nsurl_unref(session->url);

	free(session);
}



bool sslcert_reject(struct sslcert_session_data *session)
{
	session->cb(false, session->cbpw);
	sslcert_cleanup_session(session);
	return true;
}


/**
 * Handle acceptance of certificate
 */
bool sslcert_accept(struct sslcert_session_data *session)
{
	assert(session != NULL);

	urldb_set_cert_permissions(session->url, true);

	session->cb(true, session->cbpw);

	sslcert_cleanup_session(session);

	return true;
}
