/*
 * Copyright 2007 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2010 Michael Drake <tlsa@netsurf-browser.org>
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
 * Content for text/html (implementation).
 */

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "utils/config.h"
#include "content/content_protected.h"
#include "content/fetch.h"
#include "content/hlcache.h"
#include "desktop/options.h"
#include "desktop/selection.h"
#include "desktop/scrollbar.h"
#include "image/bitmap.h"
#include "render/box.h"
#include "render/font.h"
#include "render/form.h"
#include "render/html_internal.h"
#include "render/imagemap.h"
#include "render/layout.h"
#include "render/search.h"
#include "javascript/js.h"
#include "utils/corestrings.h"
#include "utils/http.h"
#include "utils/libdom.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "utils/schedule.h"
#include "utils/talloc.h"
#include "utils/url.h"
#include "utils/utf8.h"
#include "utils/utils.h"

#define CHUNK 4096

/* Change these to 1 to cause a dump to stderr of the frameset or box
 * when the trees have been built.
 */
#define ALWAYS_DUMP_FRAMESET 0
#define ALWAYS_DUMP_BOX 0

static const char *html_types[] = {
	"application/xhtml+xml",
	"text/html"
};

/* forward declared functions */
static void html_object_refresh(void *p);

/* pre-interned character set */
static lwc_string *html_charset;

static nsurl *html_default_stylesheet_url;
static nsurl *html_adblock_stylesheet_url;
static nsurl *html_quirks_stylesheet_url;
static nsurl *html_user_stylesheet_url;

static nserror css_error_to_nserror(css_error error)
{
	switch (error) {
	case CSS_OK:
		return NSERROR_OK;

	case CSS_NOMEM:
		return NSERROR_NOMEM;

	case CSS_BADPARM:
		return NSERROR_BAD_PARAMETER;

	case CSS_INVALID:
		return NSERROR_INVALID;

	case CSS_FILENOTFOUND:
		return NSERROR_NOT_FOUND;

	case CSS_NEEDDATA:
		return NSERROR_NEED_DATA;

	case CSS_BADCHARSET:
		return NSERROR_BAD_ENCODING;

	case CSS_EOF:
	case CSS_IMPORTS_PENDING:
	case CSS_PROPERTY_NOT_SET:
	default:
		break;
	}
	return NSERROR_CSS;
}


static void html_destroy_objects(html_content *html)
{
	while (html->object_list != NULL) {
		struct content_html_object *victim = html->object_list;

		if (victim->content != NULL) {
			LOG(("object %p", victim->content));

			if (content_get_type(victim->content) == CONTENT_HTML)
				schedule_remove(html_object_refresh, victim);

			hlcache_handle_release(victim->content);
		}

		html->object_list = victim->next;
		free(victim);
	}
}

/**
 * Perform post-box-creation conversion of a document
 *
 * \param c        HTML content to complete conversion of
 * \param success  Whether box tree construction was successful
 */
static void html_box_convert_done(html_content *c, bool success)
{
	nserror err;
	dom_exception exc; /* returned by libdom functions */
	dom_node *html;

	LOG(("Done XML to box (%p)", c));

	/* Clean up and report error if unsuccessful or aborted */
	if ((success == false) || (c->aborted)) {
		html_destroy_objects(c);

		if (success == false) {
			content_broadcast_errorcode(&c->base, NSERROR_BOX_CONVERT);
		} else {
			content_broadcast_errorcode(&c->base, NSERROR_STOPPED);
		}

		content_set_error(&c->base);
		return;
	}


#if ALWAYS_DUMP_BOX
	box_dump(stderr, c->layout->children, 0);
#endif
#if ALWAYS_DUMP_FRAMESET
	if (c->frameset)
		html_dump_frameset(c->frameset, 0);
#endif

	exc = dom_document_get_document_element(c->document, (void *) &html);
	if ((exc != DOM_NO_ERR) || (html == NULL)) {
		/** @todo should this call html_destroy_objects(c);
		 * like the other error paths 
		 */
		LOG(("error retrieving html element from dom"));
		content_broadcast_errorcode(&c->base, NSERROR_DOM);
		content_set_error(&c->base);
		return;
	}

	/* extract image maps - can't do this sensibly in dom_to_box */
	err = imagemap_extract(c);
	if (err != NSERROR_OK) {
		LOG(("imagemap extraction failed"));
		html_destroy_objects(c);
		content_broadcast_errorcode(&c->base, err);
		content_set_error(&c->base);
		dom_node_unref(html);
		return;
	}
	/*imagemap_dump(c);*/

	/* Destroy the parser binding */
	dom_hubbub_parser_destroy(c->parser);
	c->parser = NULL;

	content_set_ready(&c->base);

	if (c->base.active == 0) {
		content_set_done(&c->base);
	}

	html_set_status(c, "");
	dom_node_unref(html);
}


/**
 * Complete conversion of an HTML document
 *
 * \param c  Content to convert
 */
void html_finish_conversion(html_content *c)
{
	union content_msg_data msg_data;
	dom_exception exc; /* returned by libdom functions */
	dom_node *html;
	uint32_t i;
	css_error css_ret;
	nserror error;

	/* Bail out if we've been aborted */
	if (c->aborted) {
		content_broadcast_errorcode(&c->base, NSERROR_STOPPED);
		content_set_error(&c->base);
		return;
	}

	/* check that the base stylesheet loaded; layout fails without it */
	if (c->stylesheets[STYLESHEET_BASE].data.external == NULL) {
		content_broadcast_errorcode(&c->base, NSERROR_CSS_BASE);
		content_set_error(&c->base);
		return;
	}

	/* Create selection context */
	css_ret = css_select_ctx_create(ns_realloc, c, &c->select_ctx);
	if (css_ret != CSS_OK) {
		content_broadcast_errorcode(&c->base,
					    css_error_to_nserror(css_ret));
		content_set_error(&c->base);
		return;
	}

	/* Add sheets to it */
	for (i = STYLESHEET_BASE; i != c->stylesheet_count; i++) {
		const struct html_stylesheet *hsheet = &c->stylesheets[i];
		css_stylesheet *sheet;
		css_origin origin = CSS_ORIGIN_AUTHOR;

		if (i < STYLESHEET_USER)
			origin = CSS_ORIGIN_UA;
		else if (i < STYLESHEET_START)
			origin = CSS_ORIGIN_USER;

		if (hsheet->type == HTML_STYLESHEET_EXTERNAL &&
				hsheet->data.external != NULL) {
			sheet = nscss_get_stylesheet(hsheet->data.external);
		} else if (hsheet->type == HTML_STYLESHEET_INTERNAL) {
			sheet = hsheet->data.internal->sheet;
		} else {
			sheet = NULL;
		}

		if (sheet != NULL) {
			css_ret = css_select_ctx_append_sheet(c->select_ctx,
							      sheet,
							      origin,
							      CSS_MEDIA_SCREEN);
			if (css_ret != CSS_OK) {
				content_broadcast_errorcode(&c->base,
						css_error_to_nserror(css_ret));
				content_set_error(&c->base);
				return;
			}
		}
	}

	/* fire a simple event named load at the Document's Window
	 * object, but with its target set to the Document object (and
	 * the currentTarget set to the Window object)
	 */
	js_fire_event(c->jscontext, "load", c->document, NULL);

	/* convert dom tree to box tree */
	LOG(("DOM to box (%p)", c));
	content_set_status(&c->base, messages_get("Processing"));
	msg_data.explicit_status_text = NULL;
	content_broadcast(&c->base, CONTENT_MSG_STATUS, msg_data);

	exc = dom_document_get_document_element(c->document, (void *) &html);
	if ((exc != DOM_NO_ERR) || (html == NULL)) {
		LOG(("error retrieving html element from dom"));
		content_broadcast_errorcode(&c->base, NSERROR_DOM);
		content_set_error(&c->base);
		return;
	}

	error = dom_to_box(html, c, html_box_convert_done);
	if (error != NSERROR_OK) {
		dom_node_unref(html);
		html_destroy_objects(c);
		content_broadcast_errorcode(&c->base, error);
		content_set_error(&c->base);
		return;
	}

	dom_node_unref(html);
}



static nserror
html_create_html_data(html_content *c, const http_parameter *params)
{
	lwc_string *charset;
	nserror nerror;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;

	c->parser = NULL;
	c->document = NULL;
	c->quirks = DOM_DOCUMENT_QUIRKS_MODE_NONE;
	c->encoding = NULL;
	c->base_url = nsurl_ref(content_get_url(&c->base));
	c->base_target = NULL;
	c->aborted = false;
	c->bctx = NULL;
	c->layout = NULL;
	c->background_colour = NS_TRANSPARENT;
	c->stylesheet_count = 0;
	c->stylesheets = NULL;
	c->select_ctx = NULL;
	c->universal = NULL;
	c->num_objects = 0;
	c->object_list = NULL;
	c->forms = NULL;
	c->imagemaps = NULL;
	c->bw = NULL;
	c->frameset = NULL;
	c->iframe = NULL;
	c->page = NULL;
	c->font_func = &nsfont;
	c->scrollbar = NULL;
	c->scripts_count = 0;
	c->scripts = NULL;
	c->jscontext = NULL;

	c->base.active = 1; /* The html content itself is active */

	if (lwc_intern_string("*", SLEN("*"), &c->universal) != lwc_error_ok) {
		return NSERROR_NOMEM;
	}

	selection_prepare(&c->sel, (struct content *)c, true);

	nerror = http_parameter_list_find_item(params, html_charset, &charset);
	if (nerror == NSERROR_OK) {
		c->encoding = strdup(lwc_string_data(charset));

		lwc_string_unref(charset);

		if (c->encoding == NULL) {
			lwc_string_unref(c->universal);
			c->universal = NULL;
			return NSERROR_NOMEM;

		}
		c->encoding_source = DOM_HUBBUB_ENCODING_SOURCE_HEADER;
	}

	/* Create the parser binding */
	parse_params.enc = c->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = nsoption_bool(enable_javascript);
	parse_params.msg = NULL;
	parse_params.script = html_process_script;
	parse_params.ctx = c;
	parse_params.daf = NULL;

	error = dom_hubbub_parser_create(&parse_params,
					 &c->parser,
					 &c->document);
	if ((error != DOM_HUBBUB_OK) && (c->encoding != NULL)) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to autodetect */
		free(c->encoding);
		c->encoding = NULL;

		parse_params.enc = c->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &c->parser,
						 &c->document);
	}

	if (error != DOM_HUBBUB_OK) {
		nsurl_unref(c->base_url);
		c->base_url = NULL;

		lwc_string_unref(c->universal);
		c->universal = NULL;

		return libdom_hubbub_error_to_nserror(error);
	}

	return NSERROR_OK;

}

/**
 * Create a CONTENT_HTML.
 *
 * The content_html_data structure is initialized and the HTML parser is
 * created.
 */

static nserror
html_create(const content_handler *handler,
	    lwc_string *imime_type,
	    const http_parameter *params,
	    llcache_handle *llcache,
	    const char *fallback_charset,
	    bool quirks,
	    struct content **c)
{
	html_content *html;
	nserror error;

	html = calloc(1, sizeof(html_content));
	if (html == NULL)
		return NSERROR_NOMEM;

	error = content__init(&html->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	if (error != NSERROR_OK) {
		free(html);
		return error;
	}

	error = html_create_html_data(html, params);
	if (error != NSERROR_OK) {
		content_broadcast_errorcode(&html->base, error);
		free(html);
		return error;
	}

	*c = (struct content *) html;

	return NSERROR_OK;
}



static nserror
html_process_encoding_change(struct content *c, 
			     const char *data, 
			     unsigned int size)
{
	html_content *html = (html_content *) c;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	const char *encoding;
	const char *source_data;
	unsigned long source_size;

	/* Retrieve new encoding */
	encoding = dom_hubbub_parser_get_encoding(html->parser, 
						  &html->encoding_source);
	if (encoding == NULL) {
		return NSERROR_NOMEM;
	}

	if (html->encoding != NULL) {
		free(html->encoding);
	}

	html->encoding = strdup(encoding);
	if (html->encoding == NULL) {
		return NSERROR_NOMEM;
	}

	/* Destroy binding */
	dom_hubbub_parser_destroy(html->parser);
	html->parser = NULL;

	if (html->document != NULL) {
		dom_node_unref(html->document);
	}

	parse_params.enc = html->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = nsoption_bool(enable_javascript);
	parse_params.msg = NULL;
	parse_params.script = html_process_script;
	parse_params.ctx = html;
	parse_params.daf = NULL;

	/* Create new binding, using the new encoding */
	error = dom_hubbub_parser_create(&parse_params,
					 &html->parser,
					 &html->document);
	if (error != DOM_HUBBUB_OK) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to Windows-1252 */
		free(html->encoding);
		html->encoding = strdup("Windows-1252");
		if (html->encoding == NULL) {
			return NSERROR_NOMEM;
		}
		parse_params.enc = html->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &html->parser,
						 &html->document);

		if (error != DOM_HUBBUB_OK) {
			return libdom_hubbub_error_to_nserror(error);
		}

	}

	source_data = content__get_source_data(c, &source_size);

	/* Reprocess all the data.  This is safe because
	 * the encoding is now specified at parser start which means
	 * it cannot be changed again. 
	 */
	error = dom_hubbub_parser_parse_chunk(html->parser, 
					      (const uint8_t *)source_data, 
					      source_size);

	return libdom_hubbub_error_to_nserror(error);
}


/**
 * Process data for CONTENT_HTML.
 */

static bool
html_process_data(struct content *c, const char *data, unsigned int size)
{
	html_content *html = (html_content *) c;
	dom_hubbub_error dom_ret;
	nserror err = NSERROR_OK; /* assume its all going to be ok */

	dom_ret = dom_hubbub_parser_parse_chunk(html->parser, 
					      (const uint8_t *) data, 
					      size);

	err = libdom_hubbub_error_to_nserror(dom_ret);

	/* deal with encoding change */
	if (err == NSERROR_ENCODING_CHANGE) {
		 err = html_process_encoding_change(c, data, size);
	}

	/* broadcast the error if necessary */
	if (err != NSERROR_OK) {
		content_broadcast_errorcode(c, err);
		return false;
	}

	return true;	
}


/** process link node */
static bool html_process_link(html_content *c, dom_node *node)
{
	struct content_rfc5988_link link; /* the link added to the content */
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;
	nserror error;

	memset(&link, 0, sizeof(struct content_rfc5988_link));

	/* check that the relation exists - w3c spec says must be present */
	exc = dom_element_get_attribute(node, corestring_dom_rel, &atr_string);
	if ((exc != DOM_NO_ERR) || (atr_string == NULL)) {
		return false;
	}
	/* get a lwc string containing the link relation */
	exc = dom_string_intern(atr_string, &link.rel);
	dom_string_unref(atr_string);
	if (exc != DOM_NO_ERR) {
		return false;
	}

	/* check that the href exists - w3c spec says must be present */
	exc = dom_element_get_attribute(node, corestring_dom_href, &atr_string);
	if ((exc != DOM_NO_ERR) || (atr_string == NULL)) {
		lwc_string_unref(link.rel);
		return false;
	}

	/* get nsurl */
	error = nsurl_join(c->base_url, dom_string_data(atr_string),
			&link.href);
	dom_string_unref(atr_string);
	if (error != NSERROR_OK) {
		lwc_string_unref(link.rel);
		return false;
	}

	/* look for optional properties -- we don't care if internment fails */

	exc = dom_element_get_attribute(node,
			corestring_dom_hreflang, &atr_string);
	if ((exc == DOM_NO_ERR) && (atr_string != NULL)) {
		/* get a lwc string containing the href lang */
		exc = dom_string_intern(atr_string, &link.hreflang);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(node,
			corestring_dom_type, &atr_string);
	if ((exc == DOM_NO_ERR) && (atr_string != NULL)) {
		/* get a lwc string containing the type */
		exc = dom_string_intern(atr_string, &link.type);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(node,
			corestring_dom_media, &atr_string);
	if ((exc == DOM_NO_ERR) && (atr_string != NULL)) {
		/* get a lwc string containing the media */
		exc = dom_string_intern(atr_string, &link.media);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(node,
			corestring_dom_sizes, &atr_string);
	if ((exc == DOM_NO_ERR) && (atr_string != NULL)) {
		/* get a lwc string containing the sizes */
		exc = dom_string_intern(atr_string, &link.sizes);
		dom_string_unref(atr_string);
	}

	/* add to content */
	content__add_rfc5988_link(&c->base, &link);

	if (link.sizes != NULL)
		lwc_string_unref(link.sizes);
	if (link.media != NULL)
		lwc_string_unref(link.media);
	if (link.type != NULL)
		lwc_string_unref(link.type);
	if (link.hreflang != NULL)
		lwc_string_unref(link.hreflang);

	nsurl_unref(link.href);
	lwc_string_unref(link.rel);

	return true;
}

/** process title node */
static bool html_process_title(html_content *c, dom_node *node)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *title;
	char *title_str;
	bool success;

	if (c->base.title != NULL)
		return true;

	exc = dom_node_get_text_content(node, &title);
	if ((exc != DOM_NO_ERR) || (title == NULL)) {
		return false;
	}

	title_str = squash_whitespace(dom_string_data(title));
	dom_string_unref(title);

	if (title_str == NULL) {
		return false;
	}

	success = content__set_title(&c->base, title_str);

	free(title_str);

	return success;
}

static bool html_process_base(html_content *c, dom_node *node)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;

	/* get href attribute if present */
	exc = dom_element_get_attribute(node,
			corestring_dom_href, &atr_string);
	if ((exc == DOM_NO_ERR) && (atr_string != NULL)) {
		nsurl *url;
		nserror error;

		/* get url from string */
		error = nsurl_create(dom_string_data(atr_string), &url);
		dom_string_unref(atr_string);
		if (error == NSERROR_OK) {
			if (c->base_url != NULL)
				nsurl_unref(c->base_url);
			c->base_url = url;
		}
	}


	/* get target attribute if present and not already set */
	if (c->base_target != NULL) {
		return true;
	}

	exc = dom_element_get_attribute(node,
			corestring_dom_target, &atr_string);
	if ((exc == DOM_NO_ERR) && (atr_string != NULL)) {
		/* Validation rules from the HTML5 spec for the base element:
		 *  The target must be one of _blank, _self, _parent, or
		 *  _top or any identifier which does not begin with an
		 *  underscore
		 */
		if (*dom_string_data(atr_string) != '_' ||
		    dom_string_caseless_lwc_isequal(atr_string,
		    		corestring_lwc__blank) ||
		    dom_string_caseless_lwc_isequal(atr_string,
		    		corestring_lwc__self) ||
		    dom_string_caseless_lwc_isequal(atr_string,
		    		corestring_lwc__parent) ||
		    dom_string_caseless_lwc_isequal(atr_string,
		    		corestring_lwc__top)) {
			c->base_target = strdup(dom_string_data(atr_string));
		}
		dom_string_unref(atr_string);
	}

	return true;
}

/**
 * Process elements in <head>.
 *
 * \param  c     content structure
 * \param  head  xml node of head element
 * \return  true on success, false on memory exhaustion
 *
 * The title and base href are extracted if present.
 */

static nserror html_head(html_content *c, dom_node *head)
{
	dom_node *node;
	dom_exception exc; /* returned by libdom functions */
	dom_string *node_name;
	dom_node_type node_type;
	dom_node *next_node;

	exc = dom_node_get_first_child(head, &node);
	if (exc != DOM_NO_ERR) {
		return NSERROR_DOM;
	}

	while (node != NULL) {
		exc = dom_node_get_node_type(node, &node_type);

		if ((exc == DOM_NO_ERR) && (node_type == DOM_ELEMENT_NODE)) {
			exc = dom_node_get_node_name(node, &node_name);

			if ((exc == DOM_NO_ERR) && (node_name != NULL)) {
				if (dom_string_caseless_lwc_isequal(
						node_name,
						corestring_lwc_title)) {
					html_process_title(c, node);
				} else if (dom_string_caseless_lwc_isequal(
						node_name,
						corestring_lwc_base)) {
					html_process_base(c, node);
				} else if (dom_string_caseless_lwc_isequal(
						node_name,
						corestring_lwc_link)) {
					html_process_link(c, node);
				}
			}
			if (node_name != NULL) {
				dom_string_unref(node_name);
			}
		}

		/* move to next node */
		exc = dom_node_get_next_sibling(node, &next_node);
		dom_node_unref(node);
		if (exc == DOM_NO_ERR) {
			node = next_node;
		} else {
			node = NULL;
		}
	}

	return NSERROR_OK;
}

static nserror html_meta_refresh_process_element(html_content *c, dom_node *n)
{
	union content_msg_data msg_data;
	const char *url, *end, *refresh = NULL;
	char *new_url;
	char quote = '\0';
	dom_string *equiv, *content;
	dom_exception exc;
	nsurl *nsurl;
	nserror error = NSERROR_OK;

	exc = dom_element_get_attribute(n, corestring_dom_http_equiv, &equiv);
	if (exc != DOM_NO_ERR) {
		return NSERROR_DOM;
	}

	if (equiv == NULL) {
		return NSERROR_OK;
	}

	if (!dom_string_caseless_lwc_isequal(equiv, corestring_lwc_refresh)) {
		dom_string_unref(equiv);
		return NSERROR_OK;
	}

	dom_string_unref(equiv);

	exc = dom_element_get_attribute(n, corestring_dom_content, &content);
	if (exc != DOM_NO_ERR) {
		return NSERROR_DOM;
	}

	if (content == NULL) {
		return NSERROR_OK;
	}

	end = dom_string_data(content) + dom_string_byte_length(content);

	/* content  := *LWS intpart fracpart? *LWS [';' *LWS *1url *LWS]
	 * intpart  := 1*DIGIT
	 * fracpart := 1*('.' | DIGIT)
	 * url      := "url" *LWS '=' *LWS (url-nq | url-sq | url-dq)
	 * url-nq   := *urlchar
	 * url-sq   := "'" *(urlchar | '"') "'"
	 * url-dq   := '"' *(urlchar | "'") '"'
	 * urlchar  := [#x9#x21#x23-#x26#x28-#x7E] | nonascii
	 * nonascii := [#x80-#xD7FF#xE000-#xFFFD#x10000-#x10FFFF]
	 */

	url = dom_string_data(content);

	/* *LWS */
	while (url < end && isspace(*url)) {
		url++;
	}

	/* intpart */
	if (url == end || (*url < '0' || '9' < *url)) {
		/* Empty content, or invalid timeval */
		dom_string_unref(content);
		return NSERROR_OK;
	}

	msg_data.delay = (int) strtol(url, &new_url, 10);
	/* a very small delay and self-referencing URL can cause a loop
	 * that grinds machines to a halt. To prevent this we set a
	 * minimum refresh delay of 1s. */
	if (msg_data.delay < 1) {
		msg_data.delay = 1;
	}

	url = new_url;

	/* fracpart? (ignored, as delay is integer only) */
	while (url < end && (('0' <= *url && *url <= '9') ||
			*url == '.')) {
		url++;
	}

	/* *LWS */
	while (url < end && isspace(*url)) {
		url++;
	}

	/* ';' */
	if (url < end && *url == ';')
		url++;

	/* *LWS */
	while (url < end && isspace(*url)) {
		url++;
	}

	if (url == end) {
		/* Just delay specified, so refresh current page */
		dom_string_unref(content);

		c->base.refresh = nsurl_ref(
				content_get_url(&c->base));

		content_broadcast(&c->base, CONTENT_MSG_REFRESH, msg_data);

		return NSERROR_OK;
	}

	/* "url" */
	if (url <= end - 3) {
		if (strncasecmp(url, "url", 3) == 0) {
			url += 3;
		} else {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			return NSERROR_OK;
		}
	} else {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		return NSERROR_OK;
	}

	/* *LWS */
	while (url < end && isspace(*url)) {
		url++;
	}

	/* '=' */
	if (url < end) {
		if (*url == '=') {
			url++;
		} else {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			return NSERROR_OK;
		}
	} else {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		return NSERROR_OK;
	}

	/* *LWS */
	while (url < end && isspace(*url)) {
		url++;
	}

	/* '"' or "'" */
	if (url < end && (*url == '"' || *url == '\'')) {
		quote = *url;
		url++;
	}

	/* Start of URL */
	refresh = url;

	if (quote != 0) {
		/* url-sq | url-dq */
		while (url < end && *url != quote)
			url++;
	} else {
		/* url-nq */
		while (url < end && !isspace(*url))
			url++;
	}

	/* '"' or "'" or *LWS (we don't care) */
	if (url > refresh) {
		/* There's a URL */
		new_url = strndup(refresh, url - refresh);
		if (new_url == NULL) {
			dom_string_unref(content);
			return NSERROR_NOMEM;
		}

		error = nsurl_join(c->base_url, new_url, &nsurl);
		if (error == NSERROR_OK) {
			/* broadcast valid refresh url */

			c->base.refresh = nsurl;

			content_broadcast(&c->base, CONTENT_MSG_REFRESH, msg_data);
		}

		free(new_url);

	}

	dom_string_unref(content);

	return error;
}

/**
 * Search for meta refresh
 *
 * http://wp.netscape.com/assist/net_sites/pushpull.html
 *
 * \param c content structure
 * \param head xml node of head element
 * \return true on success, false otherwise (error reported)
 */

static nserror html_meta_refresh(html_content *c, dom_node *head)
{
	dom_node *n, *next;
	dom_exception exc;
	nserror ns_error = NSERROR_OK;

	if (head == NULL) {
		return ns_error;
	}

	exc = dom_node_get_first_child(head, &n);
	if (exc != DOM_NO_ERR) {
		return NSERROR_DOM;
	}

	while (n != NULL) {
		dom_node_type type;

		exc = dom_node_get_node_type(n, &type);
		if (exc != DOM_NO_ERR) {
			dom_node_unref(n);
			return NSERROR_DOM;
		}

		if (type == DOM_ELEMENT_NODE) {
			dom_string *name;

			exc = dom_node_get_node_name(n, &name);
			if (exc != DOM_NO_ERR) {
				dom_node_unref(n);
				return NSERROR_DOM;
			}

			/* Recurse into noscript elements */
			if (dom_string_caseless_lwc_isequal(name, corestring_lwc_noscript)) {
				ns_error = html_meta_refresh(c, n);
				if (ns_error != NSERROR_OK) {
					/* Some error occurred */
					dom_string_unref(name);
					dom_node_unref(n);
					return ns_error;
				} else if (c->base.refresh != NULL) {
					/* Meta refresh found - stop */
					dom_string_unref(name);
					dom_node_unref(n);
					return NSERROR_OK;
				}
			} else if (dom_string_caseless_lwc_isequal(name, corestring_lwc_meta)) {
				ns_error = html_meta_refresh_process_element(c, n);
				if (ns_error != NSERROR_OK) {
					/* Some error occurred */
					dom_string_unref(name);
					dom_node_unref(n);
					return ns_error;
				} else if (c->base.refresh != NULL) {
					/* Meta refresh found - stop */
					dom_string_unref(name);
					dom_node_unref(n);
					return NSERROR_OK;
				}
			}
			dom_string_unref(name);
		}

		exc = dom_node_get_next_sibling(n, &next);
		if (exc != DOM_NO_ERR) {
			dom_node_unref(n);
			return NSERROR_DOM;
		}

		dom_node_unref(n);
		n = next;
	}

	return ns_error;
}

/**
 * Update a box whose content has completed rendering.
 */

static void
html_object_done(struct box *box,
		 hlcache_handle *object,
		 bool background)
{
	struct box *b;

	if (background) {
		box->background = object;
		return;
	}

	box->object = object;

	if (!(box->flags & REPLACE_DIM)) {
		/* invalidate parent min, max widths */
		for (b = box; b; b = b->parent)
			b->max_width = UNKNOWN_MAX_WIDTH;

		/* delete any clones of this box */
		while (box->next && (box->next->flags & CLONE)) {
			/* box_free_box(box->next); */
			box->next = box->next->next;
		}
	}
}

/**
 * Handle object fetching or loading failure.
 *
 * \param  box         box containing object which failed to load
 * \param  content     document of type CONTENT_HTML
 * \param  background  the object was the background image for the box
 */

static void
html_object_failed(struct box *box, html_content *content, bool background)
{
	/* Nothing to do */
	return;
}

/**
 * Callback for hlcache_handle_retrieve() for objects.
 */

static nserror
html_object_callback(hlcache_handle *object,
		     const hlcache_event *event,
		     void *pw)
{
	struct content_html_object *o = pw;
	html_content *c = (html_content *) o->parent;
	int x, y;
	struct box *box;

	assert(c->base.status != CONTENT_STATUS_ERROR);

	box = o->box;

	switch (event->type) {
	case CONTENT_MSG_LOADING:
		if (c->base.status != CONTENT_STATUS_LOADING && c->bw != NULL)
			content_open(object,
					c->bw, &c->base,
					box->object_params);
		break;

	case CONTENT_MSG_READY:
		if (content_can_reformat(object)) {
			/* TODO: avoid knowledge of box internals here */
			content_reformat(object, false,
					box->max_width != UNKNOWN_MAX_WIDTH ?
							box->width : 0,
					box->max_width != UNKNOWN_MAX_WIDTH ?
							box->height : 0);

			/* Adjust parent content for new object size */
			html_object_done(box, object, o->background);
			if (c->base.status == CONTENT_STATUS_READY ||
					c->base.status == CONTENT_STATUS_DONE)
				content__reformat(&c->base, false,
						c->base.available_width,
						c->base.height);
		}
		break;

	case CONTENT_MSG_DONE:
		c->base.active--;
		LOG(("%d fetches active", c->base.active));

		html_object_done(box, object, o->background);

		if (c->base.status != CONTENT_STATUS_LOADING &&
				box->flags & REPLACE_DIM) {
			union content_msg_data data;

			if (!box_visible(box))
				break;

			box_coords(box, &x, &y);

			data.redraw.x = x + box->padding[LEFT];
			data.redraw.y = y + box->padding[TOP];
			data.redraw.width = box->width;
			data.redraw.height = box->height;
			data.redraw.full_redraw = true;

			content_broadcast(&c->base, CONTENT_MSG_REDRAW, data);
		}
		break;

	case CONTENT_MSG_ERROR:
		hlcache_handle_release(object);

		o->content = NULL;

		c->base.active--;
		LOG(("%d fetches active", c->base.active));

		content_add_error(&c->base, "?", 0);
		html_object_failed(box, c, o->background);
		break;

	case CONTENT_MSG_STATUS:
		if (event->data.explicit_status_text == NULL) {
			/* Object content's status text updated */
			union content_msg_data data;
			data.explicit_status_text =
					content_get_status_message(object);
			html_set_status(c, data.explicit_status_text);
			content_broadcast(&c->base, CONTENT_MSG_STATUS, data);
		} else {
			/* Object content wants to set explicit message */
			content_broadcast(&c->base, CONTENT_MSG_STATUS,
					event->data);
		}
		break;

	case CONTENT_MSG_REFORMAT:
		break;

	case CONTENT_MSG_REDRAW:
		if (c->base.status != CONTENT_STATUS_LOADING) {
			union content_msg_data data = event->data;

			if (!box_visible(box))
				break;

			box_coords(box, &x, &y);

			if (hlcache_handle_get_content(object) ==
					event->data.redraw.object) {
				data.redraw.x = data.redraw.x *
					box->width / content_get_width(object);
				data.redraw.y = data.redraw.y *
					box->height /
					content_get_height(object);
				data.redraw.width = data.redraw.width *
					box->width / content_get_width(object);
				data.redraw.height = data.redraw.height *
					box->height /
					content_get_height(object);
				data.redraw.object_width = box->width;
				data.redraw.object_height = box->height;
			}

			data.redraw.x += x + box->padding[LEFT];
			data.redraw.y += y + box->padding[TOP];
			data.redraw.object_x += x + box->padding[LEFT];
			data.redraw.object_y += y + box->padding[TOP];

			content_broadcast(&c->base, CONTENT_MSG_REDRAW, data);
		}
		break;

	case CONTENT_MSG_REFRESH:
		if (content_get_type(object) == CONTENT_HTML) {
			/* only for HTML objects */
			schedule(event->data.delay * 100,
					html_object_refresh, o);
		}

		break;

	case CONTENT_MSG_LINK:
		/* Don't care about favicons that aren't on top level content */
		break;

	case CONTENT_MSG_GETCTX: 
		*(event->data.jscontext) = NULL;
		break;

	case CONTENT_MSG_SCROLL:
		if (box->scroll_x != NULL)
			scrollbar_set(box->scroll_x, event->data.scroll.x0,
					false);
		if (box->scroll_y != NULL)
			scrollbar_set(box->scroll_y, event->data.scroll.y0,
					false);
		break;

	case CONTENT_MSG_DRAGSAVE:
	{
		union content_msg_data msg_data;
		if (event->data.dragsave.content == NULL)
			msg_data.dragsave.content = object;
		else
			msg_data.dragsave.content =
					event->data.dragsave.content;

		content_broadcast(&c->base, CONTENT_MSG_DRAGSAVE, msg_data);
	}
		break;

	case CONTENT_MSG_SAVELINK:
	case CONTENT_MSG_POINTER:
		/* These messages are for browser window layer.
		 * we're not interested, so pass them on. */
		content_broadcast(&c->base, event->type, event->data);
		break;

	default:
		assert(0);
	}

	if (c->base.status == CONTENT_STATUS_READY && c->base.active == 0 &&
			(event->type == CONTENT_MSG_LOADING ||
			event->type == CONTENT_MSG_DONE ||
			event->type == CONTENT_MSG_ERROR)) {
		/* all objects have arrived */
		content__reformat(&c->base, false, c->base.available_width,
				c->base.height);
		html_set_status(c, "");
		content_set_done(&c->base);
	}

	/* If  1) the configuration option to reflow pages while objects are
	 *        fetched is set
	 *     2) an object is newly fetched & converted,
	 *     3) the box's dimensions need to change due to being replaced
	 *     4) the object's parent HTML is ready for reformat,
	 *     5) the time since the previous reformat is more than the
	 *        configured minimum time between reformats
	 * then reformat the page to display newly fetched objects */
	else if (nsoption_bool(incremental_reflow) &&
			event->type == CONTENT_MSG_DONE &&
			!(box->flags & REPLACE_DIM) &&
			(c->base.status == CONTENT_STATUS_READY ||
			 c->base.status == CONTENT_STATUS_DONE) &&
			(wallclock() > c->base.reformat_time)) {
		content__reformat(&c->base, false, c->base.available_width,
				c->base.height);
	}

	return NSERROR_OK;
}

/**
 * Start a fetch for an object required by a page, replacing an existing object.
 *
 * \param  object          Object to replace
 * \param  url             URL of object to fetch (copied)
 * \return  true on success, false on memory exhaustion
 */

static bool html_replace_object(struct content_html_object *object, nsurl *url)
{
	html_content *c;
	hlcache_child_context child;
	html_content *page;
	nserror error;

	assert(object != NULL);

	c = (html_content *) object->parent;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	if (object->content != NULL) {
		/* remove existing object */
		if (content_get_status(object->content) != CONTENT_STATUS_DONE) {
			c->base.active--;
			LOG(("%d fetches active", c->base.active));
		}

		hlcache_handle_release(object->content);
		object->content = NULL;

		object->box->object = NULL;
	}

	/* initialise fetch */
	error = hlcache_handle_retrieve(url, HLCACHE_RETRIEVE_SNIFF_TYPE,
			content_get_url(&c->base), NULL,
			html_object_callback, object, &child,
			object->permitted_types,
			&object->content);

	if (error != NSERROR_OK)
		return false;

	for (page = c; page != NULL; page = page->page) {
		page->base.active++;
		LOG(("%d fetches active", c->base.active));

		page->base.status = CONTENT_STATUS_READY;
	}

	return true;
}

/**
 * schedule() callback for object refresh
 */

static void html_object_refresh(void *p)
{
	struct content_html_object *object = p;
	nsurl *refresh_url;

	assert(content_get_type(object->content) == CONTENT_HTML);

	refresh_url = content_get_refresh_url(object->content);

	/* Ignore if refresh URL has gone
	 * (may happen if fetch errored) */
	if (refresh_url == NULL)
		return;

	content_invalidate_reuse_data(object->content);

	if (!html_replace_object(object, refresh_url)) {
		/** \todo handle memory exhaustion */
	}
}





/**
 * Callback for fetchcache() for linked stylesheets.
 */

static nserror
html_convert_css_callback(hlcache_handle *css,
			  const hlcache_event *event,
			  void *pw)
{
	html_content *parent = pw;
	unsigned int i;
	struct html_stylesheet *s;

	/* Find sheet */
	for (i = 0, s = parent->stylesheets;
			i != parent->stylesheet_count; i++, s++) {
		if (s->type == HTML_STYLESHEET_EXTERNAL &&
				s->data.external == css)
			break;
	}

	assert(i != parent->stylesheet_count);

	switch (event->type) {
	case CONTENT_MSG_LOADING:
		break;

	case CONTENT_MSG_READY:
		break;

	case CONTENT_MSG_DONE:
		LOG(("done stylesheet slot %d '%s'", i,
				nsurl_access(hlcache_handle_get_url(css))));
		parent->base.active--;
		LOG(("%d fetches active", parent->base.active));
		break;

	case CONTENT_MSG_ERROR:
		LOG(("stylesheet %s failed: %s",
				nsurl_access(hlcache_handle_get_url(css)),
				event->data.error));
		hlcache_handle_release(css);
		s->data.external = NULL;
		parent->base.active--;
		LOG(("%d fetches active", parent->base.active));
		content_add_error(&parent->base, "?", 0);
		break;

	case CONTENT_MSG_STATUS:
		if (event->data.explicit_status_text == NULL) {
			/* Object content's status text updated */
			html_set_status(parent,
					content_get_status_message(css));
			content_broadcast(&parent->base, CONTENT_MSG_STATUS,
					event->data);
		} else {
			/* Object content wants to set explicit message */
			content_broadcast(&parent->base, CONTENT_MSG_STATUS,
					event->data);
		}
		break;

	default:
		assert(0);
	}

	if (parent->base.active == 0)
		html_finish_conversion(parent);

	return NSERROR_OK;
}

/**
 * Handle notification of inline style completion
 *
 * \param css  Inline style object
 * \param pw   Private data
 */
static void html_inline_style_done(struct content_css_data *css, void *pw)
{
	html_content *html = pw;

	if (--html->base.active == 0)
		html_finish_conversion(html);
}

/**
 * Process an inline stylesheet in the document.
 *
 * \param  c      content structure
 * \param  index  Index of stylesheet in stylesheet_content array,
 *                updated if successful
 * \param  style  xml node of style element
 * \return  true on success, false if an error occurred
 */

static bool
html_process_style_element(html_content *c,
			   unsigned int *index,
			   dom_node *style)
{
	dom_node *child, *next;
	dom_string *val;
	dom_exception exc;
	struct html_stylesheet *stylesheets;
	struct content_css_data *sheet;
	nserror error;

	/* type='text/css', or not present (invalid but common) */
	exc = dom_element_get_attribute(style, corestring_dom_type, &val);
	if (exc == DOM_NO_ERR && val != NULL) {
		if (!dom_string_caseless_lwc_isequal(val,
				corestring_lwc_text_css)) {
			dom_string_unref(val);
			return true;
		}
		dom_string_unref(val);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(style, corestring_dom_media, &val);
	if (exc == DOM_NO_ERR && val != NULL) {
		if (strcasestr(dom_string_data(val), "screen") == NULL &&
				strcasestr(dom_string_data(val),
						"all") == NULL) {
			dom_string_unref(val);
			return true;
		}
		dom_string_unref(val);
	}

	/* Extend array */
	stylesheets = realloc(c->stylesheets, 
			      sizeof(struct html_stylesheet) * (*index + 1));
	if (stylesheets == NULL)
		goto no_memory;

	c->stylesheets = stylesheets;
	c->stylesheet_count++;

	c->stylesheets[(*index)].type = HTML_STYLESHEET_INTERNAL;
	c->stylesheets[(*index)].data.internal = NULL;

	/* create stylesheet */
	sheet = calloc(1, sizeof(struct content_css_data));
	if (sheet == NULL) {
		c->stylesheet_count--;
		goto no_memory;
	}

	error = nscss_create_css_data(sheet,
		nsurl_access(c->base_url), NULL, c->quirks,
		html_inline_style_done, c);
	if (error != NSERROR_OK) {
		free(sheet);
		c->stylesheet_count--;
		content_broadcast_errorcode(&c->base, error);
		return false;
	}

	/* can't just use xmlNodeGetContent(style), because that won't
	 * give the content of comments which may be used to 'hide'
	 * the content */
	exc = dom_node_get_first_child(style, &child);
	if (exc != DOM_NO_ERR) {
		nscss_destroy_css_data(sheet);
		free(sheet);
		c->stylesheet_count--;
		goto no_memory;
	}

	while (child != NULL) {
		dom_string *data;

		exc = dom_node_get_text_content(child, &data);
		if (exc != DOM_NO_ERR) {
			dom_node_unref(child);
			nscss_destroy_css_data(sheet);
			free(sheet);
			c->stylesheet_count--;
			goto no_memory;
		}

		if (nscss_process_css_data(sheet, dom_string_data(data),
				dom_string_byte_length(data)) == false) {
			dom_string_unref(data);
			dom_node_unref(child);
			nscss_destroy_css_data(sheet);
			free(sheet);
			c->stylesheet_count--;
			goto no_memory;
		}

		dom_string_unref(data);

		exc = dom_node_get_next_sibling(child, &next);
		if (exc != DOM_NO_ERR) {
			dom_node_unref(child);
			nscss_destroy_css_data(sheet);
			free(sheet);
			c->stylesheet_count--;
			goto no_memory;
		}

		dom_node_unref(child);
		child = next;
	}

	c->base.active++;
	LOG(("%d fetches active", c->base.active));

	/* Convert the content -- manually, as we want the result */
	if (nscss_convert_css_data(sheet) != CSS_OK) {
		/* conversion failed */
		c->base.active--;
		LOG(("%d fetches active", c->base.active));
		nscss_destroy_css_data(sheet);
		free(sheet);
		sheet = NULL;
	}

	/* Update index */
	c->stylesheets[(*index)].data.internal = sheet;
	(*index)++;

	return true;

no_memory:
	content_broadcast_errorcode(&c->base, NSERROR_NOMEM);
	return false;
}



struct find_stylesheet_ctx {
	unsigned int count;
	html_content *c;
};

/** callback to process stylesheet elements
 */
static bool
html_process_stylesheet(dom_node *node, dom_string *name, void *vctx)
{
	struct find_stylesheet_ctx *ctx = (struct find_stylesheet_ctx *)vctx;
	dom_string *rel, *type_attr, *media, *href;
	struct html_stylesheet *stylesheets;
	nsurl *joined;
	dom_exception exc;
	nserror ns_error;
	hlcache_child_context child;

	/* deal with style nodes */
	if (dom_string_caseless_lwc_isequal(name, corestring_lwc_style)) {
		if (!html_process_style_element(ctx->c,	&ctx->count, node))
			return false;
		return true;
	}

	/* if it is not a link node skip it */
	if (!dom_string_caseless_lwc_isequal(name, corestring_lwc_link)) {
		return true;
	}

	/* rel=<space separated list, including 'stylesheet'> */
	exc = dom_element_get_attribute(node,
					corestring_dom_rel, &rel);
	if (exc != DOM_NO_ERR || rel == NULL)
		return true;

	if (strcasestr(dom_string_data(rel), "stylesheet") == 0) {
		dom_string_unref(rel);
		return true;
	} else if (strcasestr(dom_string_data(rel), "alternate") != 0) {
		/* Ignore alternate stylesheets */
		dom_string_unref(rel);
		return true;
	}
	dom_string_unref(rel);

	/* type='text/css' or not present */
	exc = dom_element_get_attribute(node, corestring_dom_type, &type_attr);
	if (exc == DOM_NO_ERR && type_attr != NULL) {
		if (!dom_string_caseless_lwc_isequal(type_attr,
				corestring_lwc_text_css)) {
			dom_string_unref(type_attr);
			return true;
		}
		dom_string_unref(type_attr);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(node, corestring_dom_media, &media);
	if (exc == DOM_NO_ERR && media != NULL) {
		if (strcasestr(dom_string_data(media), "screen") == NULL &&
		    strcasestr(dom_string_data(media), "all") == NULL) {
			dom_string_unref(media);
			return true;
		}
		dom_string_unref(media);
	}

	/* href='...' */
	exc = dom_element_get_attribute(node, corestring_dom_href, &href);
	if (exc != DOM_NO_ERR || href == NULL)
		return true;

	/* TODO: only the first preferred stylesheets (ie.
	 * those with a title attribute) should be loaded
	 * (see HTML4 14.3) */

	ns_error = nsurl_join(ctx->c->base_url, dom_string_data(href), &joined);
	if (ns_error != NSERROR_OK) {
		dom_string_unref(href);
		goto no_memory;
	}
	dom_string_unref(href);

	LOG(("linked stylesheet %i '%s'", ctx->count, nsurl_access(joined)));

	/* start fetch */
	stylesheets = realloc(ctx->c->stylesheets, 
			      sizeof(struct html_stylesheet) * (ctx->count + 1));
	if (stylesheets == NULL) {
		nsurl_unref(joined);
		ns_error = NSERROR_NOMEM;
		goto no_memory;
	}

	ctx->c->stylesheets = stylesheets;
	ctx->c->stylesheet_count++;
	ctx->c->stylesheets[ctx->count].type = HTML_STYLESHEET_EXTERNAL;

	child.charset = ctx->c->encoding;
	child.quirks = ctx->c->base.quirks;

	ns_error = hlcache_handle_retrieve(joined,
					   0,
					   content_get_url(&ctx->c->base),
					   NULL,
					   html_convert_css_callback,
					   ctx->c,
					   &child,
					   CONTENT_CSS,
					   &ctx->c->stylesheets[ctx->count].data.external);

	nsurl_unref(joined);

	if (ns_error != NSERROR_OK)
		goto no_memory;

	ctx->c->base.active++;
	LOG(("%d fetches active", ctx->c->base.active));

	ctx->count++;

	return true;

no_memory:
	content_broadcast_errorcode(&ctx->c->base, ns_error);
	return false;
}


/**
 * Process inline stylesheets and fetch linked stylesheets.
 *
 * Uses STYLE and LINK elements inside and outside HEAD
 *
 * \param c content structure
 * \param html dom node of html element
 * \return true on success, false if an error occurred
 */

static bool html_find_stylesheets(html_content *c, dom_node *html)
{
	nserror ns_error;
	bool result;
	struct find_stylesheet_ctx ctx;
	hlcache_child_context child;

	/* setup context */
	ctx.c = c;
	ctx.count = STYLESHEET_START;

	/* stylesheet 0 is the base style sheet,
	 * stylesheet 1 is the quirks mode style sheet,
	 * stylesheet 2 is the adblocking stylesheet,
	 * stylesheet 3 is the user stylesheet */
	c->stylesheets = calloc(STYLESHEET_START, sizeof(struct html_stylesheet));
	if (c->stylesheets == NULL) {
		ns_error = NSERROR_NOMEM;
		goto html_find_stylesheets_no_memory;
	}

	c->stylesheets[STYLESHEET_BASE].type = HTML_STYLESHEET_EXTERNAL;
	c->stylesheets[STYLESHEET_BASE].data.external = NULL;
	c->stylesheets[STYLESHEET_QUIRKS].type = HTML_STYLESHEET_EXTERNAL;
	c->stylesheets[STYLESHEET_QUIRKS].data.external = NULL;
	c->stylesheets[STYLESHEET_ADBLOCK].type = HTML_STYLESHEET_EXTERNAL;
	c->stylesheets[STYLESHEET_ADBLOCK].data.external = NULL;
	c->stylesheets[STYLESHEET_USER].type = HTML_STYLESHEET_EXTERNAL;
	c->stylesheets[STYLESHEET_USER].data.external = NULL;
	c->stylesheet_count = STYLESHEET_START;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ns_error = hlcache_handle_retrieve(html_default_stylesheet_url, 0,
			content_get_url(&c->base), NULL,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_BASE].data.external);
	if (ns_error != NSERROR_OK)
		goto html_find_stylesheets_no_memory;

	c->base.active++;
	LOG(("%d fetches active", c->base.active));

	if (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL) {
		ns_error = hlcache_handle_retrieve(html_quirks_stylesheet_url,
				0, content_get_url(&c->base), NULL,
				html_convert_css_callback, c, &child,
				CONTENT_CSS,
				&c->stylesheets[STYLESHEET_QUIRKS].data.external);
		if (ns_error != NSERROR_OK)
			goto html_find_stylesheets_no_memory;

		c->base.active++;
		LOG(("%d fetches active", c->base.active));

	}

	if (nsoption_bool(block_ads)) {
		ns_error = hlcache_handle_retrieve(html_adblock_stylesheet_url,
				0, content_get_url(&c->base), NULL,
				html_convert_css_callback, c, &child, CONTENT_CSS,
				&c->stylesheets[STYLESHEET_ADBLOCK].
						data.external);
		if (ns_error != NSERROR_OK)
			goto html_find_stylesheets_no_memory;

		c->base.active++;
		LOG(("%d fetches active", c->base.active));

	}

	ns_error = hlcache_handle_retrieve(html_user_stylesheet_url, 0,
			content_get_url(&c->base), NULL,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_USER].data.external);
	if (ns_error != NSERROR_OK)
		goto html_find_stylesheets_no_memory;

	c->base.active++;
	LOG(("%d fetches active", c->base.active));

	result = libdom_treewalk(html, html_process_stylesheet, &ctx);

	assert(c->stylesheet_count == ctx.count);

	return result;

html_find_stylesheets_no_memory:
	content_broadcast_errorcode(&c->base, ns_error);
	return false;
}

/**
 * Convert a CONTENT_HTML for display.
 *
 * The following steps are carried out in order:
 *
 *  - parsing to an XML tree is completed
 *  - stylesheets are fetched
 *  - the XML tree is converted to a box tree and object fetches are started
 *
 * On exit, the content status will be either CONTENT_STATUS_DONE if the
 * document is completely loaded or CONTENT_STATUS_READY if objects are still
 * being fetched.
 */

static bool html_convert(struct content *c)
{
	html_content *htmlc = (html_content *) c;

	htmlc->base.active--; /* the html fetch is no longer active */
	LOG(("%d fetches active", htmlc->base.active));

	/* if there are no active fetches in progress no scripts are
	 * being fetched or they completed already.
	 */ 
	if (htmlc->base.active == 0) {
		return html_begin_conversion(htmlc);
	}
	return true;
}

bool
html_begin_conversion(html_content *htmlc)
{
	dom_node *html, *head;
	nserror ns_error;
	struct form *f;
	dom_exception exc; /* returned by libdom functions */
	dom_string *node_name = NULL;
	dom_hubbub_error error;

	/* complete parsing */
	error = dom_hubbub_parser_completed(htmlc->parser);
	if (error != DOM_HUBBUB_OK) {
		LOG(("Parsing failed"));

		content_broadcast_errorcode(&htmlc->base, 
					    libdom_hubbub_error_to_nserror(error));

		return false;
	}

	/* Give up processing if we've been aborted */
	if (htmlc->aborted) {
		content_broadcast_errorcode(&htmlc->base, NSERROR_STOPPED);
		return false;
	}


	/* complete script execution */
	html_scripts_exec(htmlc);

	/* fire a simple event that bubbles named DOMContentLoaded at
	 * the Document.
	 */

	/* quirks mode */
	exc = dom_document_get_quirks_mode(htmlc->document, &htmlc->quirks);
	if (exc != DOM_NO_ERR) {
		LOG(("error retrieving quirks"));
		/** @todo should this be fatal to the conversion? */
	} 
	LOG(("quirks set to %d", htmlc->quirks));

	/* get encoding */
	if (htmlc->encoding == NULL) {
		const char *encoding;

		encoding = dom_hubbub_parser_get_encoding(htmlc->parser,
					&htmlc->encoding_source);
		if (encoding == NULL) {
			content_broadcast_errorcode(&htmlc->base, 
						    NSERROR_NOMEM);
			return false;
		}

		htmlc->encoding = strdup(encoding);
		if (htmlc->encoding == NULL) {
			content_broadcast_errorcode(&htmlc->base, 
						    NSERROR_NOMEM);
			return false;
		}
	}

	/* locate root element and ensure it is html */
	exc = dom_document_get_document_element(htmlc->document, (void *) &html);
	if ((exc != DOM_NO_ERR) || (html == NULL)) {
		LOG(("error retrieving html element from dom"));
		content_broadcast_errorcode(&htmlc->base, NSERROR_DOM);
		return false;
	}

	exc = dom_node_get_node_name(html, &node_name);
	if ((exc != DOM_NO_ERR) ||
	    (node_name == NULL) ||
	    (!dom_string_caseless_lwc_isequal(node_name,
	    		corestring_lwc_html))) {
		LOG(("root element not html"));
		content_broadcast_errorcode(&htmlc->base, NSERROR_DOM);
		dom_node_unref(html);
		return false;
	}
	dom_string_unref(node_name);

	head = libdom_find_first_element(html, corestring_lwc_head);
	if (head != NULL) {
		ns_error = html_head(htmlc, head);
		if (ns_error != NSERROR_OK) {
			content_broadcast_errorcode(&htmlc->base, ns_error);

			dom_node_unref(html);
			dom_node_unref(head);
			return false;
		}

		/* handle meta refresh */
		ns_error = html_meta_refresh(htmlc, head);
		if (ns_error != NSERROR_OK) {
			content_broadcast_errorcode(&htmlc->base, ns_error);

			dom_node_unref(html);
			dom_node_unref(head);
			return false;
		}
	}

	/* Retrieve forms from parser */
	htmlc->forms = html_forms_get_forms(htmlc->encoding,
			(dom_html_document *) htmlc->document);
	for (f = htmlc->forms; f != NULL; f = f->prev) {
		nsurl *action;

		/* Make all actions absolute */
		if (f->action == NULL || f->action[0] == '\0') {
			/* HTML5 4.10.22.3 step 9 */
			nsurl *doc_addr = content_get_url(&htmlc->base);
			ns_error = nsurl_join(htmlc->base_url,
					      nsurl_access(doc_addr), 
					      &action);
		} else {
			ns_error = nsurl_join(htmlc->base_url, 
					      f->action, 
					      &action);
		}

		if (ns_error != NSERROR_OK) {
			content_broadcast_errorcode(&htmlc->base, ns_error);

			dom_node_unref(html);
			dom_node_unref(head);
			return false;
		}

		free(f->action);
		f->action = strdup(nsurl_access(action));
		nsurl_unref(action);
		if (f->action == NULL) {
			content_broadcast_errorcode(&htmlc->base, 
						    NSERROR_NOMEM);

			dom_node_unref(html);
			dom_node_unref(head);
			return false;
		}

		/* Ensure each form has a document encoding */
		if (f->document_charset == NULL) {
			f->document_charset = strdup(htmlc->encoding);
			if (f->document_charset == NULL) {
				content_broadcast_errorcode(&htmlc->base, 
							    NSERROR_NOMEM);
				dom_node_unref(html);
				dom_node_unref(head);
				return false;
			}
		}
	}

	dom_node_unref(head);

	/* get stylesheets */
	if (html_find_stylesheets(htmlc, html) == false) {
		dom_node_unref(html);
		return false;
	}

	dom_node_unref(html);

	if (htmlc->base.active == 0) {
		html_finish_conversion(htmlc);
	}

	return true;
}




/**
 * Start a fetch for an object required by a page.
 *
 * \param  c                 content of type CONTENT_HTML
 * \param  url               URL of object to fetch (copied)
 * \param  box               box that will contain the object
 * \param  permitted_types   bitmap of acceptable types
 * \param  available_width   estimate of width of object
 * \param  available_height  estimate of height of object
 * \param  background        this is a background image
 * \return  true on success, false on memory exhaustion
 */

bool html_fetch_object(html_content *c, nsurl *url, struct box *box,
		content_type permitted_types,
		int available_width, int available_height,
		bool background)
{
	struct content_html_object *object;
	hlcache_child_context child;
	nserror error;

	/* If we've already been aborted, don't bother attempting the fetch */
	if (c->aborted)
		return true;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	object = calloc(1, sizeof(struct content_html_object));
	if (object == NULL) {
		return false;
	}

	object->parent = (struct content *) c;
	object->next = NULL;
	object->content = NULL;
	object->box = box;
	object->permitted_types = permitted_types;
	object->background = background;

	error = hlcache_handle_retrieve(url,
			HLCACHE_RETRIEVE_SNIFF_TYPE,
			content_get_url(&c->base), NULL,
			html_object_callback, object, &child,
			object->permitted_types, &object->content);
       	if (error != NSERROR_OK) {
		free(object);
		return error != NSERROR_NOMEM;
	}

	/* add to content object list */
	object->next = c->object_list;
	c->object_list = object;

	c->num_objects++;
	c->base.active++;
	LOG(("%d fetches active", c->base.active));

	return true;
}





/**
 * Stop loading a CONTENT_HTML.
 */

static void html_stop(struct content *c)
{
	html_content *htmlc = (html_content *) c;
	struct content_html_object *object;

	switch (c->status) {
	case CONTENT_STATUS_LOADING:
		/* Still loading; simply flag that we've been aborted
		 * html_convert/html_finish_conversion will do the rest */
		htmlc->aborted = true;
		break;
	case CONTENT_STATUS_READY:
		for (object = htmlc->object_list; object != NULL;
				object = object->next) {
			if (object->content == NULL)
				continue;

			if (content_get_status(object->content) ==
					CONTENT_STATUS_DONE)
				; /* already loaded: do nothing */
			else if (content_get_status(object->content) ==
					CONTENT_STATUS_READY)
				hlcache_handle_abort(object->content);
				/* Active count will be updated when
				 * html_object_callback receives
 				 * CONTENT_MSG_DONE from this object */
			else {
				hlcache_handle_abort(object->content);
				hlcache_handle_release(object->content);
				object->content = NULL;

				c->active--;
				LOG(("%d fetches active", c->active));

			}
		}

		/* If there are no further active fetches and we're still
 		 * in the READY state, transition to the DONE state. */
		if (c->status == CONTENT_STATUS_READY && c->active == 0) {
			html_set_status(htmlc, "");
			content_set_done(c);
		}

		break;
	case CONTENT_STATUS_DONE:
		/* Nothing to do */
		break;
	default:
		LOG(("Unexpected status %d", c->status));
		assert(0);
	}
}


/**
 * Reformat a CONTENT_HTML to a new width.
 */

static void html_reformat(struct content *c, int width, int height)
{
	html_content *htmlc = (html_content *) c;
	struct box *layout;
	unsigned int time_before, time_taken;

	time_before = wallclock();

	layout_document(htmlc, width, height);
	layout = htmlc->layout;

	/* width and height are at least margin box of document */
	c->width = layout->x + layout->padding[LEFT] + layout->width +
			layout->padding[RIGHT] + layout->border[RIGHT].width +
			layout->margin[RIGHT];
	c->height = layout->y + layout->padding[TOP] + layout->height +
			layout->padding[BOTTOM] + layout->border[BOTTOM].width +
			layout->margin[BOTTOM];

	/* if boxes overflow right or bottom edge, expand to contain it */
	if (c->width < layout->x + layout->descendant_x1)
		c->width = layout->x + layout->descendant_x1;
	if (c->height < layout->y + layout->descendant_y1)
		c->height = layout->y + layout->descendant_y1;

	selection_reinit(&htmlc->sel, htmlc->layout);

	time_taken = wallclock() - time_before;
	c->reformat_time = wallclock() +
		((time_taken * 3 < nsoption_int(min_reflow_period) ?
		  nsoption_int(min_reflow_period) : time_taken * 3));
}


/**
 * Redraw a box.
 *
 * \param  h	content containing the box, of type CONTENT_HTML
 * \param  box  box to redraw
 */

void html_redraw_a_box(hlcache_handle *h, struct box *box)
{
	int x, y;

	box_coords(box, &x, &y);

	content_request_redraw(h, x, y,
			box->padding[LEFT] + box->width + box->padding[RIGHT],
			box->padding[TOP] + box->height + box->padding[BOTTOM]);
}


/**
 * Redraw a box.
 *
 * \param  h	content containing the box, of type CONTENT_HTML
 * \param  box  box to redraw
 */

void html__redraw_a_box(struct html_content *html, struct box *box)
{
	int x, y;

	box_coords(box, &x, &y);

	content__request_redraw((struct content *)html, x, y,
			box->padding[LEFT] + box->width + box->padding[RIGHT],
			box->padding[TOP] + box->height + box->padding[BOTTOM]);
}

static void html_destroy_frameset(struct content_html_frames *frameset)
{
	int i;

	if (frameset->name) {
		talloc_free(frameset->name);
		frameset->name = NULL;
	}
	if (frameset->url) {
		talloc_free(frameset->url);
		frameset->url = NULL;
	}
	if (frameset->children) {
		for (i = 0; i < (frameset->rows * frameset->cols); i++) {
			if (frameset->children[i].name) {
				talloc_free(frameset->children[i].name);
				frameset->children[i].name = NULL;
			}
			if (frameset->children[i].url) {
				nsurl_unref(frameset->children[i].url);
				frameset->children[i].url = NULL;
			}
		  	if (frameset->children[i].children)
		  		html_destroy_frameset(&frameset->children[i]);
		}
		talloc_free(frameset->children);
		frameset->children = NULL;
	}
}

static void html_destroy_iframe(struct content_html_iframe *iframe)
{
	struct content_html_iframe *next;
	next = iframe;
	while ((iframe = next) != NULL) {
		next = iframe->next;
		if (iframe->name)
			talloc_free(iframe->name);
		if (iframe->url) {
			nsurl_unref(iframe->url);
			iframe->url = NULL;
		}
		talloc_free(iframe);
	}
}


static void html_free_layout(html_content *htmlc)
{
	if (htmlc->bctx != NULL) {
		/* freeing talloc context should let the entire box
		 * set be destroyed 
		 */
		talloc_free(htmlc->bctx);
	}
}

/**
 * Destroy a CONTENT_HTML and free all resources it owns.
 */

static void html_destroy(struct content *c)
{
	html_content *html = (html_content *) c;
	unsigned int i;
	struct form *f, *g;

	LOG(("content %p", c));

	/* Destroy forms */
	for (f = html->forms; f != NULL; f = g) {
		g = f->prev;

		form_free(f);
	}

	imagemap_destroy(html);

	if (c->refresh)
		nsurl_unref(c->refresh);

	if (html->base_url)
		nsurl_unref(html->base_url);

	if (html->parser != NULL) {
		dom_hubbub_parser_destroy(html->parser);
		html->parser = NULL;
	}

	if (html->document != NULL) {
		dom_node_unref(html->document);
	}

	/* Free base target */
	if (html->base_target != NULL) {
	 	free(html->base_target);
	 	html->base_target = NULL;
	}

	/* Free frameset */
	if (html->frameset != NULL) {
		html_destroy_frameset(html->frameset);
		talloc_free(html->frameset);
		html->frameset = NULL;
	}

	/* Free iframes */
	if (html->iframe != NULL) {
		html_destroy_iframe(html->iframe);
		html->iframe = NULL;
	}

	/* Destroy selection context */
	if (html->select_ctx != NULL) {
		css_select_ctx_destroy(html->select_ctx);
		html->select_ctx = NULL;
	}

	if (html->universal != NULL) {
		lwc_string_unref(html->universal);
		html->universal = NULL;
	}

	/* Free stylesheets */
	for (i = 0; i != html->stylesheet_count; i++) {
		if (html->stylesheets[i].type == HTML_STYLESHEET_EXTERNAL &&
				html->stylesheets[i].data.external != NULL) {
			hlcache_handle_release(
					html->stylesheets[i].data.external);
		} else if (html->stylesheets[i].type ==
				HTML_STYLESHEET_INTERNAL &&
				html->stylesheets[i].data.internal != NULL) {
			nscss_destroy_css_data(
					html->stylesheets[i].data.internal);
		}
	}
	free(html->stylesheets);

	/* Free scripts */
	html_free_scripts(html);

	/* Free objects */
	html_destroy_objects(html);

	/* free layout */
	html_free_layout(html);
}


static nserror html_clone(const struct content *old, struct content **newc)
{
	/** \todo Clone HTML specifics */

	/* In the meantime, we should never be called, as HTML contents
	 * cannot be shared and we're not intending to fix printing's
	 * cloning of documents. */
	assert(0 && "html_clone should never be called");

	return true;
}

/**
 * Set the content status.
 */

void html_set_status(html_content *c, const char *extra)
{
	content_set_status(&c->base, extra);
}


/**
 * Handle a window containing a CONTENT_HTML being opened.
 */

static void
html_open(struct content *c,
	  struct browser_window *bw,
	  struct content *page,
	  struct object_params *params)
{
	html_content *html = (html_content *) c;
	struct content_html_object *object, *next;

	html->bw = bw;
	html->page = (html_content *) page;

	/* text selection */
	selection_init(&html->sel, html->layout);

	for (object = html->object_list; object != NULL; object = next) {
		next = object->next;

		if (object->content == NULL)
			continue;

		if (content_get_type(object->content) == CONTENT_NONE)
			continue;

		content_open(object->content,
				bw, c,
				object->box->object_params);
	}
}


/**
 * Handle a window containing a CONTENT_HTML being closed.
 */

static void html_close(struct content *c)
{
	html_content *html = (html_content *) c;
	struct content_html_object *object, *next;

	if (html->search != NULL)
		search_destroy_context(html->search);

	html->bw = NULL;

	for (object = html->object_list; object != NULL; object = next) {
		next = object->next;

		if (object->content == NULL)
			continue;

		if (content_get_type(object->content) == CONTENT_NONE)
			continue;

		if (content_get_type(object->content) == CONTENT_HTML)
			schedule_remove(html_object_refresh, object);

		content_close(object->content);
	}
}


/**
 * Return an HTML content's selection context
 */

static struct selection *html_get_selection(struct content *c)
{
	html_content *html = (html_content *) c;

	return &html->sel;
}


/**
 * Get access to any content, link URLs and objects (images) currently
 * at the given (x, y) coordinates.
 *
 * \param c	html content to look inside
 * \param x	x-coordinate of point of interest
 * \param y	y-coordinate of point of interest
 * \param data	pointer to contextual_content struct.  Its fields are updated
 *		with pointers to any relevent content, or set to NULL if none.
 */
static void
html_get_contextual_content(struct content *c,
			    int x,
			    int y,
			    struct contextual_content *data)
{
	html_content *html = (html_content *) c;

	struct box *box = html->layout;
	struct box *next;
	int box_x = 0, box_y = 0;

	while ((next = box_at_point(box, x, y, &box_x, &box_y)) != NULL) {
		box = next;

		if (box->style && css_computed_visibility(box->style) ==
				CSS_VISIBILITY_HIDDEN)
			continue;

		if (box->iframe)
			browser_window_get_contextual_content(box->iframe,
					x - box_x, y - box_y, data);

		if (box->object)
			content_get_contextual_content(box->object,
					x - box_x, y - box_y, data);

		if (box->object)
			data->object = box->object;

		if (box->href)
			data->link_url = nsurl_access(box->href);

		if (box->usemap) {
			const char *target = NULL;
			nsurl *url = imagemap_get(html, box->usemap, box_x,
					box_y, x, y, &target);
			/* Box might have imagemap, but no actual link area
			 * at point */
			if (url != NULL)
				data->link_url = nsurl_access(url);
		}
		if (box->gadget) {
			switch (box->gadget->type) {
			case GADGET_TEXTBOX:
			case GADGET_TEXTAREA:
			case GADGET_PASSWORD:
				data->form_features = CTX_FORM_TEXT;
				break;

			case GADGET_FILE:
				data->form_features = CTX_FORM_FILE;
				break;

			default:
				data->form_features = CTX_FORM_NONE;
				break;
			}
		}
	}
}


/**
 * Scroll deepest thing within the content which can be scrolled at given point
 *
 * \param c	html content to look inside
 * \param x	x-coordinate of point of interest
 * \param y	y-coordinate of point of interest
 * \param scrx	x-coordinate of point of interest
 * \param scry	y-coordinate of point of interest
 * \return true iff scroll was consumed by something in the content
 */
static bool
html_scroll_at_point(struct content *c, int x, int y, int scrx, int scry)
{
	html_content *html = (html_content *) c;

	struct box *box = html->layout;
	struct box *next;
	int box_x = 0, box_y = 0;
	bool handled_scroll = false;

	/* TODO: invert order; visit deepest box first */

	while ((next = box_at_point(box, x, y, &box_x, &box_y)) != NULL) {
		box = next;

		if (box->style && css_computed_visibility(box->style) ==
				CSS_VISIBILITY_HIDDEN)
			continue;

		/* Pass into iframe */
		if (box->iframe && browser_window_scroll_at_point(box->iframe,
				x - box_x, y - box_y, scrx, scry) == true)
			return true;

		/* Pass into object */
		if (box->object != NULL && content_scroll_at_point(
				box->object, x - box_x, y - box_y,
				scrx, scry) == true)
			return true;

		/* Handle box scrollbars */
		if (box->scroll_y && scrollbar_scroll(box->scroll_y, scry))
			handled_scroll = true;

		if (box->scroll_x && scrollbar_scroll(box->scroll_x, scrx))
			handled_scroll = true;

		if (handled_scroll == true)
			return true;
	}

	return false;
}


/**
 * Drop a file onto a content at a particular point, or determine if a file
 * may be dropped onto the content at given point.
 *
 * \param c	html content to look inside
 * \param x	x-coordinate of point of interest
 * \param y	y-coordinate of point of interest
 * \param file	path to file to be dropped, or NULL to know if drop allowed
 * \return true iff file drop has been handled, or if drop possible (NULL file)
 */
static bool html_drop_file_at_point(struct content *c, int x, int y, char *file)
{
	html_content *html = (html_content *) c;

	struct box *box = html->layout;
	struct box *next;
	struct box *file_box = NULL;
	struct box *text_box = NULL;
	int box_x = 0, box_y = 0;

	/* Scan box tree for boxes that can handle drop */
	while ((next = box_at_point(box, x, y, &box_x, &box_y)) != NULL) {
		box = next;

		if (box->style && css_computed_visibility(box->style) ==
				CSS_VISIBILITY_HIDDEN)
			continue;

		if (box->iframe)
			return browser_window_drop_file_at_point(box->iframe,
					x - box_x, y - box_y, file);

		if (box->object && content_drop_file_at_point(box->object,
					x - box_x, y - box_y, file) == true)
			return true;

		if (box->gadget) {
			switch (box->gadget->type) {
				case GADGET_FILE:
					file_box = box;
				break;

				case GADGET_TEXTBOX:
				case GADGET_TEXTAREA:
				case GADGET_PASSWORD:
					text_box = box;
					break;

				default:	/* appease compiler */
					break;
			}
		}
	}

	if (!file_box && !text_box)
		/* No box capable of handling drop */
		return false;

	if (file == NULL)
		/* There is a box capable of handling drop here */
		return true;

	/* Handle the drop */
	if (file_box) {
		/* File dropped on file input */
		utf8_convert_ret ret;
		char *utf8_fn;

		ret = utf8_from_local_encoding(file, 0,
				&utf8_fn);
		if (ret != UTF8_CONVERT_OK) {
			/* A bad encoding should never happen */
			assert(ret != UTF8_CONVERT_BADENC);
			LOG(("utf8_from_local_encoding failed"));
			/* Load was for us - just no memory */
			return true;
		}

		/* Found: update form input */
		free(file_box->gadget->value);
		file_box->gadget->value = utf8_fn;

		/* Redraw box. */
		html__redraw_a_box(html, file_box);

	} else if (html->bw != NULL) {
		/* File dropped on text input */

		size_t file_len;
		FILE *fp = NULL;
		char *buffer;
		char *utf8_buff;
		utf8_convert_ret ret;
		unsigned int size;
		struct browser_window *bw;

		/* Open file */
		fp = fopen(file, "rb");
		if (fp == NULL) {
			/* Couldn't open file, but drop was for us */
			return true;
		}

		/* Get filesize */
		fseek(fp, 0, SEEK_END);
		file_len = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		/* Allocate buffer for file data */
		buffer = malloc(file_len + 1);
		if (buffer == NULL) {
			/* No memory, but drop was for us */
			fclose(fp);
			return true;
		}

		/* Stick file into buffer */
		if (file_len != fread(buffer, 1, file_len, fp)) {
			/* Failed, but drop was for us */
			free(buffer);
			fclose(fp);
			return true;
		}

		/* Done with file */
		fclose(fp);

		/* Ensure buffer's string termination */
		buffer[file_len] = '\0';

		/* TODO: Sniff for text? */

		/* Convert to UTF-8 */
		ret = utf8_from_local_encoding(buffer, file_len, &utf8_buff);
		if (ret != UTF8_CONVERT_OK) {
			/* bad encoding shouldn't happen */
			assert(ret != UTF8_CONVERT_BADENC);
			LOG(("utf8_from_local_encoding failed"));
			free(buffer);
			warn_user("NoMemory", NULL);
			return true;
		}

		/* Done with buffer */
		free(buffer);

		/* Get new length */
		size = strlen(utf8_buff);

		/* Simulate a click over the input box, to place caret */
		browser_window_mouse_click(html->bw,
				BROWSER_MOUSE_PRESS_1, x, y);

		bw = browser_window_get_root(html->bw);

		/* Paste the file as text */
		browser_window_paste_text(bw, utf8_buff, size, true);

		free(utf8_buff);
	}

	return true;
}


/**
 * Dump debug info concerning the html_content
 *
 * \param  bw    The browser window
 * \param  bw    The file to dump to
 */
static void html_debug_dump(struct content *c, FILE *f)
{
	html_content *html = (html_content *) c;

	assert(html != NULL);
	assert(html->layout != NULL);

	box_dump(f, html->layout, 0);
}


/**
 * Set an HTML content's search context
 *
 * \param c	content of type html
 * \param s	search context, or NULL if none
 */

void html_set_search(struct content *c, struct search_context *s)
{
	html_content *html = (html_content *) c;

	html->search = s;
}


/**
 * Return an HTML content's search context
 *
 * \param c	content of type html
 * \return content's search context, or NULL if none
 */

struct search_context *html_get_search(struct content *c)
{
	html_content *html = (html_content *) c;

	return html->search;
}


#if ALWAYS_DUMP_FRAMESET
/**
 * Print a frameset tree to stderr.
 */

static void
html_dump_frameset(struct content_html_frames *frame, unsigned int depth)
{
	unsigned int i;
	int row, col, index;
	const char *unit[] = {"px", "%", "*"};
	const char *scrolling[] = {"auto", "yes", "no"};

	assert(frame);

	fprintf(stderr, "%p ", frame);

	fprintf(stderr, "(%i %i) ", frame->rows, frame->cols);

	fprintf(stderr, "w%g%s ", frame->width.value, unit[frame->width.unit]);
	fprintf(stderr, "h%g%s ", frame->height.value,unit[frame->height.unit]);
	fprintf(stderr, "(margin w%i h%i) ",
			frame->margin_width, frame->margin_height);

	if (frame->name)
		fprintf(stderr, "'%s' ", frame->name);
	if (frame->url)
		fprintf(stderr, "<%s> ", frame->url);

	if (frame->no_resize)
		fprintf(stderr, "noresize ");
	fprintf(stderr, "(scrolling %s) ", scrolling[frame->scrolling]);
	if (frame->border)
		fprintf(stderr, "border %x ",
				(unsigned int) frame->border_colour);

	fprintf(stderr, "\n");

	if (frame->children) {
		for (row = 0; row != frame->rows; row++) {
			for (col = 0; col != frame->cols; col++) {
				for (i = 0; i != depth; i++)
					fprintf(stderr, "  ");
				fprintf(stderr, "(%i %i): ", row, col);
				index = (row * frame->cols) + col;
				html_dump_frameset(&frame->children[index],
						depth + 1);
			}
		}
	}
}

#endif

/**
 * Retrieve HTML document tree
 *
 * \param h  HTML content to retrieve document tree from
 * \return Pointer to document tree
 */
dom_document *html_get_document(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	assert(c != NULL);

	return c->document;
}

/**
 * Retrieve box tree
 *
 * \param h  HTML content to retrieve tree from
 * \return Pointer to box tree
 *
 * \todo This API must die, as must all use of the box tree outside render/
 */
struct box *html_get_box_tree(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	assert(c != NULL);

	return c->layout;
}

/**
 * Retrieve the charset of an HTML document
 *
 * \param h  Content to retrieve charset from
 * \return Pointer to charset, or NULL
 */
const char *html_get_encoding(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	assert(c != NULL);

	return c->encoding;
}

/**
 * Retrieve the charset of an HTML document
 *
 * \param h  Content to retrieve charset from
 * \return Pointer to charset, or NULL
 */
dom_hubbub_encoding_source html_get_encoding_source(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	assert(c != NULL);

	return c->encoding_source;
}

/**
 * Retrieve framesets used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to framesets, or NULL if none
 */
struct content_html_frames *html_get_frameset(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	assert(c != NULL);

	return c->frameset;
}

/**
 * Retrieve iframes used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to iframes, or NULL if none
 */
struct content_html_iframe *html_get_iframe(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	assert(c != NULL);

	return c->iframe;
}

/**
 * Retrieve an HTML content's base URL
 *
 * \param h  Content to retrieve base target from
 * \return Pointer to URL
 */
nsurl *html_get_base_url(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	assert(c != NULL);

	return c->base_url;
}

/**
 * Retrieve an HTML content's base target
 *
 * \param h  Content to retrieve base target from
 * \return Pointer to target, or NULL if none
 */
const char *html_get_base_target(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	assert(c != NULL);

	return c->base_target;
}

/**
 * Retrieve stylesheets used by HTML document
 *
 * \param h  Content to retrieve stylesheets from
 * \param n  Pointer to location to receive number of sheets
 * \return Pointer to array of stylesheets
 */
struct html_stylesheet *html_get_stylesheets(hlcache_handle *h, unsigned int *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	assert(c != NULL);
	assert(n != NULL);

	*n = c->stylesheet_count;

	return c->stylesheets;
}

/**
 * Retrieve objects used by HTML document
 *
 * \param h  Content to retrieve objects from
 * \param n  Pointer to location to receive number of objects
 * \return Pointer to list of objects
 */
struct content_html_object *html_get_objects(hlcache_handle *h, unsigned int *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	assert(c != NULL);
	assert(n != NULL);

	*n = c->num_objects;

	return c->object_list;
}

/**
 * Retrieve layout coordinates of box with given id
 *
 * \param h        HTML document to search
 * \param frag_id  String containing an element id
 * \param x        Updated to global x coord iff id found
 * \param y        Updated to global y coord iff id found
 * \return  true iff id found
 */
bool html_get_id_offset(hlcache_handle *h, lwc_string *frag_id, int *x, int *y)
{
	struct box *pos;
	struct box *layout;

	if (content_get_type(h) != CONTENT_HTML)
		return false;

	layout = html_get_box_tree(h);

	if ((pos = box_find_by_id(layout, frag_id)) != 0) {
		box_coords(pos, x, y);
		return true;
	}
	return false;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_HTML
 */
static content_type html_content_type(void)
{
	return CONTENT_HTML;
}


static void html_fini(void)
{
	box_construct_fini();

	if (html_user_stylesheet_url != NULL) {
		nsurl_unref(html_user_stylesheet_url);
		html_user_stylesheet_url = NULL;
	}

	if (html_quirks_stylesheet_url != NULL) {
		nsurl_unref(html_quirks_stylesheet_url);
		html_quirks_stylesheet_url = NULL;
	}

	if (html_adblock_stylesheet_url != NULL) {
		nsurl_unref(html_adblock_stylesheet_url);
		html_adblock_stylesheet_url = NULL;
	}

	if (html_default_stylesheet_url != NULL) {
		nsurl_unref(html_default_stylesheet_url);
		html_default_stylesheet_url = NULL;
	}

	if (html_charset != NULL) {
		lwc_string_unref(html_charset);
		html_charset = NULL;
	}
}

static const content_handler html_content_handler = {
	.fini = html_fini,
	.create = html_create,
	.process_data = html_process_data,
	.data_complete = html_convert,
	.reformat = html_reformat,
	.destroy = html_destroy,
	.stop = html_stop,
	.mouse_track = html_mouse_track,
	.mouse_action = html_mouse_action,
	.redraw = html_redraw,
	.open = html_open,
	.close = html_close,
	.get_selection = html_get_selection,
	.get_contextual_content = html_get_contextual_content,
	.scroll_at_point = html_scroll_at_point,
	.drop_file_at_point = html_drop_file_at_point,
	.debug_dump = html_debug_dump,
	.clone = html_clone,
	.type = html_content_type,
	.no_share = true,
};

nserror html_init(void)
{
	uint32_t i;
	lwc_error lerror;
	nserror error;

	lerror = lwc_intern_string("charset", SLEN("charset"), &html_charset);
	if (lerror != lwc_error_ok) {
		error = NSERROR_NOMEM;
		goto error;
	}

	error = nsurl_create("resource:default.css",
			&html_default_stylesheet_url);
	if (error != NSERROR_OK)
		goto error;

	error = nsurl_create("resource:adblock.css",
			&html_adblock_stylesheet_url);
	if (error != NSERROR_OK)
		goto error;

	error = nsurl_create("resource:quirks.css",
			&html_quirks_stylesheet_url);
	if (error != NSERROR_OK)
		goto error;

	error = nsurl_create("resource:user.css",
			&html_user_stylesheet_url);
	if (error != NSERROR_OK)
		goto error;

	error = box_construct_init();
	if (error != NSERROR_OK)
		goto error;

	for (i = 0; i < NOF_ELEMENTS(html_types); i++) {
		error = content_factory_register_handler(html_types[i],
				&html_content_handler);
		if (error != NSERROR_OK)
			goto error;
	}

	return NSERROR_OK;

error:
	html_fini();

	return error;
}

/**
 * Get the browser window containing an HTML content
 *
 * \param  c	HTML content
 * \return the browser window
 */
struct browser_window *html_get_browser_window(struct content *c)
{
	html_content *html = (html_content *) c;

	assert(c != NULL);
	assert(c->handler == &html_content_handler);

	return html->bw;
}
