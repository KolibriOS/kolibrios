/*
 * Copyright 2004 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2003 Phil Mellor <monkeyson@users.sourceforge.net>
 * Copyright 2004 John Tytgat <joty@netsurf-browser.org>
 * Copyright 2005-9 John-Mark Bell <jmb@netsurf-browser.org>
 * Copyright 2009 Paul Blokus <paul_pl@users.sourceforge.net>
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
 * Form handling functions (implementation).
 */

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <dom/dom.h>
#include "content/fetch.h"
#include "content/hlcache.h"
#include "css/css.h"
#include "css/utils.h"
#include "desktop/browser.h"
#include "desktop/gui.h"
#include "desktop/mouse.h"
#include "desktop/knockout.h"
#include "desktop/plot_style.h"
#include "desktop/plotters.h"
#include "desktop/scrollbar.h"
#include "render/box.h"
#include "render/font.h"
#include "render/form.h"
#include "render/html.h"
#include "render/html_internal.h"
#include "render/layout.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "utils/talloc.h"
#include "utils/url.h"
#include "utils/utf8.h"
#include "utils/utils.h"

#define MAX_SELECT_HEIGHT 210
#define SELECT_LINE_SPACING 0.2
#define SELECT_BORDER_WIDTH 1
#define SELECT_SELECTED_COLOUR 0xDB9370

struct form_select_menu {
	int line_height;
	int width, height;
	struct scrollbar *scrollbar;
	int f_size;
	bool scroll_capture;
	select_menu_redraw_callback callback;
	void *client_data;
	struct content *c;
};

static plot_style_t plot_style_fill_selected = {
	.fill_type = PLOT_OP_TYPE_SOLID,
	.fill_colour = SELECT_SELECTED_COLOUR,
};

static plot_font_style_t plot_fstyle_entry = {
	.family = PLOT_FONT_FAMILY_SANS_SERIF,
	.weight = 400,
	.flags = FONTF_NONE,
	.background = 0xffffff,
	.foreground = 0x000000,
};

static char *form_textarea_value(struct form_control *textarea);
static char *form_acceptable_charset(struct form *form);
static char *form_encode_item(const char *item, const char *charset,
		const char *fallback);
static void form_select_menu_clicked(struct form_control *control,
		int x, int y);
static void form_select_menu_scroll_callback(void *client_data,
		struct scrollbar_msg_data *scrollbar_data);

/**
 * Create a struct form.
 *
 * \param  node    DOM node associated with form
 * \param  action  URL to submit form to, or NULL for default
 * \param  target  Target frame of form, or NULL for default
 * \param  method  method and enctype
 * \param  charset acceptable encodings for form submission, or NULL
 * \param  doc_charset  encoding of containing document, or NULL
 * \return  a new structure, or NULL on memory exhaustion
 */
struct form *form_new(void *node, const char *action, const char *target, 
		form_method method, const char *charset, 
		const char *doc_charset)
{
	struct form *form;

	form = calloc(1, sizeof *form);
	if (!form)
		return NULL;

	form->action = strdup(action != NULL ? action : "");
	if (form->action == NULL) {
		free(form);
		return NULL;
	}

	form->target = target != NULL ? strdup(target) : NULL;
	if (target != NULL && form->target == NULL) {
		free(form->action);
		free(form);
		return NULL;
	}

	form->method = method;

	form->accept_charsets = charset != NULL ? strdup(charset) : NULL;
	if (charset != NULL && form->accept_charsets == NULL) {
		free(form->target);
		free(form->action);
		free(form);
		return NULL;
	}

	form->document_charset = doc_charset != NULL ? strdup(doc_charset)
						     : NULL;
	if (doc_charset && form->document_charset == NULL) {
		free(form->accept_charsets);
		free(form->target);
		free(form->action);
		free(form);
		return NULL;
	}

	form->node = node;

	return form;
}

/**
 * Free a form, and any controls it owns.
 *
 * \param form  The form to free
 *
 * \note There may exist controls attached to box tree nodes which are not
 * associated with any form. These will leak at present. Ideally, they will
 * be cleaned up when the box tree is destroyed. As that currently happens
 * via talloc, this won't happen. These controls are distinguishable, as their
 * form field will be NULL.
 */
void form_free(struct form *form)
{
	struct form_control *c, *d;

	for (c = form->controls; c != NULL; c = d) {
		d = c->next;

		form_free_control(c);
	}

	free(form->action);
	free(form->target);
	free(form->accept_charsets);
	free(form->document_charset);

	free(form);
}

/**
 * Create a struct form_control.
 *
 * \param  node  Associated DOM node
 * \param  type  control type
 * \return  a new structure, or NULL on memory exhaustion
 */
struct form_control *form_new_control(void *node, form_control_type type)
{
	struct form_control *control;

	control = calloc(1, sizeof *control);
	if (control == NULL)
		return NULL;

	control->node = node;
	control->type = type;

	return control;
}


/**
 * Add a control to the list of controls in a form.
 *
 * \param form  The form to add the control to
 * \param control  The control to add
 */
void form_add_control(struct form *form, struct form_control *control)
{
	control->form = form;

	if (form->controls != NULL) {
		assert(form->last_control);

		form->last_control->next = control;
		control->prev = form->last_control;
		control->next = NULL;
		form->last_control = control;
	} else {
		form->controls = form->last_control = control;
	}
}


/**
 * Free a struct form_control.
 *
 * \param  control  structure to free
 */
void form_free_control(struct form_control *control)
{
	free(control->name);
	free(control->value);
	free(control->initial_value);

	if (control->type == GADGET_SELECT) {
		struct form_option *option, *next;

		for (option = control->data.select.items; option;
				option = next) {
			next = option->next;
			free(option->text);
			free(option->value);
			free(option);
		}
		if (control->data.select.menu != NULL)
			form_free_select_menu(control);
	}

	free(control);
}


/**
 * Add an option to a form select control.
 *
 * \param  control   form control of type GADGET_SELECT
 * \param  value     value of option, used directly (not copied)
 * \param  text      text for option, used directly (not copied)
 * \param  selected  this option is selected
 * \return  true on success, false on memory exhaustion
 */
bool form_add_option(struct form_control *control, char *value, char *text,
		bool selected)
{
	struct form_option *option;

	assert(control);
	assert(control->type == GADGET_SELECT);

	option = calloc(1, sizeof *option);
	if (!option)
		return false;

	option->value = value;
	option->text = text;

	/* add to linked list */
	if (control->data.select.items == 0)
		control->data.select.items = option;
	else
		control->data.select.last_item->next = option;
	control->data.select.last_item = option;

	/* set selected */
	if (selected && (control->data.select.num_selected == 0 ||
			control->data.select.multiple)) {
		option->selected = option->initial_selected = true;
		control->data.select.num_selected++;
		control->data.select.current = option;
	}

	control->data.select.num_items++;

	return true;
}


/**
 * Identify 'successful' controls.
 *
 * All text strings in the successful controls list will be in the charset most
 * appropriate for submission. Therefore, no utf8_to_* processing should be
 * performed upon them.
 *
 * \todo The chosen charset needs to be made available such that it can be
 * included in the submission request (e.g. in the fetch's Content-Type header)
 *
 * \param  form           form to search for successful controls
 * \param  submit_button  control used to submit the form, if any
 * \param  successful_controls  updated to point to linked list of
 *                        fetch_multipart_data, 0 if no controls
 * \return  true on success, false on memory exhaustion
 *
 * See HTML 4.01 section 17.13.2.
 */
bool form_successful_controls(struct form *form,
		struct form_control *submit_button,
		struct fetch_multipart_data **successful_controls)
{
	struct form_control *control;
	struct form_option *option;
	struct fetch_multipart_data sentinel, *last_success, *success_new;
	char *value = NULL;
	bool had_submit = false;
	char *charset;

	last_success = &sentinel;
	sentinel.next = NULL;

	charset = form_acceptable_charset(form);
	if (!charset)
		return false;

#define ENCODE_ITEM(i) form_encode_item((i), charset, form->document_charset)

	for (control = form->controls; control; control = control->next) {
		/* ignore disabled controls */
		if (control->disabled)
			continue;

		/* ignore controls with no name */
		if (!control->name)
			continue;

		switch (control->type) {
			case GADGET_HIDDEN:
			case GADGET_TEXTBOX:
			case GADGET_PASSWORD:
				if (control->value)
					value = ENCODE_ITEM(control->value);
				else
					value = ENCODE_ITEM("");
				if (!value) {
					LOG(("failed to duplicate value"
						"'%s' for control %s",
							control->value,
							control->name));
					goto no_memory;
				}
				break;

			case GADGET_RADIO:
			case GADGET_CHECKBOX:
				/* ignore checkboxes and radio buttons which
				 * aren't selected */
				if (!control->selected)
					continue;
				if (control->value)
					value = ENCODE_ITEM(control->value);
				else
					value = ENCODE_ITEM("on");
				if (!value) {
					LOG(("failed to duplicate"
						"value '%s' for"
						"control %s",
						control->value,
						control->name));
					goto no_memory;
				}
				break;

			case GADGET_SELECT:
				/* select */
				for (option = control->data.select.items;
						option != NULL;
						option = option->next) {
					if (!option->selected)
						continue;
					success_new =
						malloc(sizeof(*success_new));
					if (!success_new) {
						LOG(("malloc failed"));
						goto no_memory;
					}
					success_new->file = false;
					success_new->name =
						ENCODE_ITEM(control->name);
					success_new->value =
						ENCODE_ITEM(option->value);
					success_new->next = NULL;
					last_success->next = success_new;
					last_success = success_new;
					if (!success_new->name ||
						!success_new->value) {
						LOG(("strdup failed"));
						goto no_memory;
					}
				}

				continue;
				break;

			case GADGET_TEXTAREA:
				{
				char *v2;

				/* textarea */
				value = form_textarea_value(control);
				if (!value) {
					LOG(("failed handling textarea"));
					goto no_memory;
				}
				if (value[0] == 0) {
					free(value);
					continue;
				}

				v2 = ENCODE_ITEM(value);
				if (!v2) {
					LOG(("failed handling textarea"));
					free(value);
					goto no_memory;
				}

				free(value);
				value = v2;
				}
				break;

			case GADGET_IMAGE: {
				/* image */
				size_t len;
				char *name;

				if (control != submit_button)
					/* only the activated submit button
					 * is successful */
					continue;

				name = ENCODE_ITEM(control->name);
				if (name == NULL)
					goto no_memory;

				len = strlen(name) + 3;

				/* x */
				success_new = malloc(sizeof(*success_new));
				if (!success_new) {
					free(name);
					LOG(("malloc failed"));
					goto no_memory;
				}
				success_new->file = false;
				success_new->name = malloc(len);
				success_new->value = malloc(20);
				if (!success_new->name ||
						!success_new->value) {
					free(success_new->name);
					free(success_new->value);
					free(success_new);
					free(name);
					LOG(("malloc failed"));
					goto no_memory;
				}
				sprintf(success_new->name, "%s.x", name);
				sprintf(success_new->value, "%i",
						control->data.image.mx);
				success_new->next = 0;
				last_success->next = success_new;
				last_success = success_new;

				/* y */
				success_new = malloc(sizeof(*success_new));
				if (!success_new) {
					free(name);
					LOG(("malloc failed"));
					goto no_memory;
				}
				success_new->file = false;
				success_new->name = malloc(len);
				success_new->value = malloc(20);
				if (!success_new->name ||
						!success_new->value) {
					free(success_new->name);
					free(success_new->value);
					free(success_new);
					free(name);
					LOG(("malloc failed"));
					goto no_memory;
				}
				sprintf(success_new->name, "%s.y", name);
				sprintf(success_new->value, "%i",
						control->data.image.my);
				success_new->next = 0;
				last_success->next = success_new;
				last_success = success_new;

				free(name);

				continue;
				break;
			}

			case GADGET_SUBMIT:
				if (!submit_button && !had_submit)
					/* no submit button specified, so
					 * use first declared in form */
					had_submit = true;
				else if (control != submit_button)
					/* only the activated submit button
					 * is successful */
					continue;
				if (control->value)
					value = ENCODE_ITEM(control->value);
				else
					value = ENCODE_ITEM("");
				if (!value) {
					LOG(("failed to duplicate value"
						"'%s' for control %s",
							control->value,
							control->name));
					goto no_memory;
				}
				break;

			case GADGET_RESET:
				/* ignore reset */
				continue;
				break;

			case GADGET_FILE:
				/* file */
				/* Handling of blank file entries is
				 * implementation defined - we're perfectly
				 * within our rights to treat it as an
				 * unsuccessful control. Unfortunately, every
				 * other browser submits the field with
				 * a blank filename and no content. So,
				 * that's what we have to do, too.
				 */
				success_new = malloc(sizeof(*success_new));
				if (!success_new) {
					LOG(("malloc failed"));
					goto no_memory;
				}
				success_new->file = true;
				success_new->name = ENCODE_ITEM(control->name);
				success_new->value = 
						ENCODE_ITEM(control->value ?
						control->value : "");
				success_new->next = 0;
				last_success->next = success_new;
				last_success = success_new;
				if (!success_new->name ||
						!success_new->value) {
					LOG(("strdup failed"));
					goto no_memory;
				}

				continue;
				break;

			case GADGET_BUTTON:
				/* Ignore it */
				continue;
				break;

			default:
				assert(0);
				break;
		}

		success_new = malloc(sizeof(*success_new));
		if (!success_new) {
			LOG(("malloc failed"));
			goto no_memory;
		}
		success_new->file = false;
		success_new->name = ENCODE_ITEM(control->name);
		success_new->value = value;
		success_new->next = NULL;
		last_success->next = success_new;
		last_success = success_new;
		if (!success_new->name) {
			LOG(("failed to duplicate name '%s'",
					control->name));
			goto no_memory;
		}
	}

	*successful_controls = sentinel.next;
	return true;

no_memory:
	warn_user("NoMemory", 0);
	fetch_multipart_data_destroy(sentinel.next);
	return false;

#undef ENCODE_ITEM
}


/**
 * Find the value for a textarea control.
 *
 * \param  textarea  control of type GADGET_TEXTAREA
 * \return  the value as a UTF-8 string on heap, or 0 on memory exhaustion
 */
char *form_textarea_value(struct form_control *textarea)
{
	unsigned int len = 0;
	char *value, *s;
	struct box *text_box;

	/* Textarea may have no associated box if styled with display: none */
	if (textarea->box == NULL) {
		/* Return the empty string: caller treats this as a 
		 * non-successful control. */
		return strdup("");
	}

	/* find required length */
	for (text_box = textarea->box->children->children; text_box;
			text_box = text_box->next) {
		if (text_box->type == BOX_TEXT)
			len += text_box->length + 1;
		else /* BOX_BR */
			len += 2;
	}

	/* construct value */
	s = value = malloc(len + 1);
	if (!s)
		return NULL;

	for (text_box = textarea->box->children->children; text_box;
			text_box = text_box->next) {
		if (text_box->type == BOX_TEXT) {
			strncpy(s, text_box->text, text_box->length);
			s += text_box->length;
			if (text_box->next && text_box->next->type != BOX_BR)
				/* only add space if this isn't
				 * the last box on a line (or in the area) */
				*s++ = ' ';
		} else { /* BOX_BR */
			*s++ = '\r';
			*s++ = '\n';
		}
	}
	*s = 0;

	return value;
}


/**
 * Encode controls using application/x-www-form-urlencoded.
 *
 * \param  form  form to which successful controls relate
 * \param  control  linked list of fetch_multipart_data
 * \param  query_string  iff true add '?' to the start of returned data
 * \return  URL-encoded form, or 0 on memory exhaustion
 */

static char *form_url_encode(struct form *form,
		struct fetch_multipart_data *control,
		bool query_string)
{
	char *name, *value;
	char *s, *s2;
	unsigned int len, len1, len_init;
	url_func_result url_err;

	if (query_string)
		s = malloc(2);
	else
		s = malloc(1);

	if (s == NULL)
		return NULL;

	if (query_string) {
		s[0] = '?';
		s[1] = '\0';
		len_init = len = 1;
	} else {
		s[0] = '\0';
		len_init = len = 0;
	}

	for (; control; control = control->next) {
		url_err = url_escape(control->name, 0, true, NULL, &name);
		if (url_err == URL_FUNC_NOMEM) {
			free(s);
			return NULL;
		}

		assert(url_err == URL_FUNC_OK);

		url_err = url_escape(control->value, 0, true, NULL, &value);
		if (url_err == URL_FUNC_NOMEM) {
			free(name);
			free(s);
			return NULL;
		}

		assert(url_err == URL_FUNC_OK);

		len1 = len + strlen(name) + strlen(value) + 2;
		s2 = realloc(s, len1 + 1);
		if (!s2) {
			free(value);
			free(name);
			free(s);
			return NULL;
		}
		s = s2;
		sprintf(s + len, "%s=%s&", name, value);
		len = len1;
		free(name);
		free(value);
	}

	if (len > len_init)
		/* Replace trailing '&' */
		s[len - 1] = '\0';
	return s;
}

/**
 * Find an acceptable character set encoding with which to submit the form
 *
 * \param form  The form
 * \return Pointer to charset name (on heap, caller should free) or NULL
 */
char *form_acceptable_charset(struct form *form)
{
	char *temp, *c;

	if (!form)
		return NULL;

	if (!form->accept_charsets) {
		/* no accept-charsets attribute for this form */
		if (form->document_charset)
			/* document charset present, so use it */
			return strdup(form->document_charset);
		else
			/* no document charset, so default to 8859-1 */
			return strdup("ISO-8859-1");
	}

	/* make temporary copy of accept-charsets attribute */
	temp = strdup(form->accept_charsets);
	if (!temp)
		return NULL;

	/* make it upper case */
	for (c = temp; *c; c++)
		*c = toupper(*c);

	/* is UTF-8 specified? */
	c = strstr(temp, "UTF-8");
	if (c) {
		free(temp);
		return strdup("UTF-8");
	}

	/* dispense with temporary copy */
	free(temp);

	/* according to RFC2070, the accept-charsets attribute of the
	 * form element contains a space and/or comma separated list */
	c = form->accept_charsets;

	/* What would be an improvement would be to choose an encoding
	 * acceptable to the server which covers as much of the input
	 * values as possible. Additionally, we need to handle the case
	 * where none of the acceptable encodings cover all the textual
	 * input values.
	 * For now, we just extract the first element of the charset list
	 */
	while (*c && !isspace(*c)) {
		if (*c == ',')
			break;
		c++;
	}

	return strndup(form->accept_charsets, c - form->accept_charsets);
}

/**
 * Convert a string from UTF-8 to the specified charset
 * As a final fallback, this will attempt to convert to ISO-8859-1.
 *
 * \todo Return charset used?
 *
 * \param item String to convert
 * \param charset Destination charset
 * \param fallback Fallback charset (may be NULL),
 *                 used iff converting to charset fails
 * \return Pointer to converted string (on heap, caller frees), or NULL
 */
char *form_encode_item(const char *item, const char *charset,
		const char *fallback)
{
	utf8_convert_ret err;
	char *ret = NULL;
	char cset[256];

	if (!item || !charset)
		return NULL;

	snprintf(cset, sizeof cset, "%s//TRANSLIT", charset);

	err = utf8_to_enc(item, cset, 0, &ret);
	if (err == UTF8_CONVERT_BADENC) {
		/* charset not understood, try without transliteration */
		snprintf(cset, sizeof cset, "%s", charset);
		err = utf8_to_enc(item, cset, 0, &ret);

		if (err == UTF8_CONVERT_BADENC) {
			/* nope, try fallback charset (if any) */
			if (fallback) {
				snprintf(cset, sizeof cset, 
						"%s//TRANSLIT", fallback);
				err = utf8_to_enc(item, cset, 0, &ret);

				if (err == UTF8_CONVERT_BADENC) {
					/* and without transliteration */
					snprintf(cset, sizeof cset,
							"%s", fallback);
					err = utf8_to_enc(item, cset, 0, &ret);
				}
			}

			if (err == UTF8_CONVERT_BADENC) {
				/* that also failed, use 8859-1 */
				err = utf8_to_enc(item, "ISO-8859-1//TRANSLIT",
						0, &ret);
				if (err == UTF8_CONVERT_BADENC) {
					/* and without transliteration */
					err = utf8_to_enc(item, "ISO-8859-1",
							0, &ret);
				}
			}
		}
	}
	if (err == UTF8_CONVERT_NOMEM) {
		return NULL;
	}

	return ret;
}

/**
 * Open a select menu for a select form control, creating it if necessary.
 *
 * \param client_data	data passed to the redraw callback
 * \param control	the select form control for which the menu is being
 * 			opened
 * \param callback	redraw callback for the select menu
 * \param bw		the browser window in which the select menu is being
 * 			opened
 * \return		false on memory exhaustion, true otherwise
 */
bool form_open_select_menu(void *client_data,
		struct form_control *control,
		select_menu_redraw_callback callback,
		struct content *c)
{
	int line_height_with_spacing;
	struct box *box;
	plot_font_style_t fstyle;
	int total_height;
	struct form_select_menu *menu;


	/* if the menu is opened for the first time */
	if (control->data.select.menu == NULL) {

		menu = calloc(1, sizeof (struct form_select_menu));
		if (menu == NULL) {
			warn_user("NoMemory", 0);
			return false;
		}

		control->data.select.menu = menu;

		box = control->box;

		menu->width = box->width +
				box->border[RIGHT].width +
				box->border[LEFT].width +
				box->padding[RIGHT] + box->padding[LEFT];

		font_plot_style_from_css(control->box->style,
				&fstyle);
		menu->f_size = fstyle.size;

		menu->line_height = FIXTOINT(FDIV((FMUL(FLTTOFIX(1.2),
				FMUL(nscss_screen_dpi,
				INTTOFIX(fstyle.size / FONT_SIZE_SCALE)))),
				F_72));

		line_height_with_spacing = menu->line_height +
				menu->line_height *
				SELECT_LINE_SPACING;

		total_height = control->data.select.num_items *
				line_height_with_spacing;
		menu->height = total_height;

		if (menu->height > MAX_SELECT_HEIGHT) {

			menu->height = MAX_SELECT_HEIGHT;
		}
		menu->client_data = client_data;
		menu->callback = callback;
		if (!scrollbar_create(false,
				menu->height,
    				total_height,
				menu->height,
				control,
				form_select_menu_scroll_callback,
				&(menu->scrollbar))) {
			free(menu);
			return false;
		}
		menu->c = c;
	}
	else menu = control->data.select.menu;

	menu->callback(client_data, 0, 0, menu->width, menu->height);

	return true;
}


/**
 * Destroy a select menu and free allocated memory.
 * 
 * \param control	the select form control owning the select menu being
 * 			destroyed
 */
void form_free_select_menu(struct form_control *control)
{
	if (control->data.select.menu->scrollbar != NULL)
		scrollbar_destroy(control->data.select.menu->scrollbar);
	free(control->data.select.menu);
	control->data.select.menu = NULL;
}

/**
 * Redraw an opened select menu.
 * 
 * \param control	the select menu being redrawn
 * \param x		the X coordinate to draw the menu at
 * \param x		the Y coordinate to draw the menu at
 * \param scale		current redraw scale
 * \param clip		clipping rectangle
 * \param ctx		current redraw context
 * \return		true on success, false otherwise
 */
bool form_redraw_select_menu(struct form_control *control, int x, int y,
		float scale, const struct rect *clip,
		const struct redraw_context *ctx)
{
	const struct plotter_table *plot = ctx->plot;
	struct box *box;
	struct form_select_menu *menu = control->data.select.menu;
	struct form_option *option;
	int line_height, line_height_with_spacing;
	int width, height;
	int x0, y0, x1, scrollbar_x, y1, y2, y3;
	int item_y;
	int text_pos_offset, text_x;
	int scrollbar_width = SCROLLBAR_WIDTH;
	int i;
	int scroll;
	int x_cp, y_cp;
	struct rect r;
	
	box = control->box;
	
	x_cp = x;
	y_cp = y;
	width = menu->width;
	height = menu->height;
	line_height = menu->line_height;
	
	line_height_with_spacing = line_height +
			line_height * SELECT_LINE_SPACING;
	scroll = scrollbar_get_offset(menu->scrollbar);
	
	if (scale != 1.0) {
		x *= scale;
		y *= scale;
		width *= scale;
		height *= scale;
		scrollbar_width *= scale;
		
		i = scroll / line_height_with_spacing;
		scroll -= i * line_height_with_spacing;
		line_height *= scale;
		line_height_with_spacing *= scale;
		scroll *= scale;
		scroll += i * line_height_with_spacing;
	}
	
	
	x0 = x;
	y0 = y;
	x1 = x + width - 1;
	y1 = y + height - 1;
	scrollbar_x = x1 - scrollbar_width;

	r.x0 = x0;
	r.y0 = y0;
	r.x1 = x1 + 1;
	r.y1 = y1 + 1;
	if (!plot->clip(&r))
		return false;
	if (!plot->rectangle(x0, y0, x1, y1 ,plot_style_stroke_darkwbasec))
		return false;
		
	
	x0 = x0 + SELECT_BORDER_WIDTH;
	y0 = y0 + SELECT_BORDER_WIDTH;
	x1 = x1 - SELECT_BORDER_WIDTH;
	y1 = y1 - SELECT_BORDER_WIDTH;
	height = height - 2 * SELECT_BORDER_WIDTH;

	r.x0 = x0;
	r.y0 = y0;
	r.x1 = x1 + 1;
	r.y1 = y1 + 1;
	if (!plot->clip(&r))
		return false;
	if (!plot->rectangle(x0, y0, x1 + 1, y1 + 1,
			plot_style_fill_lightwbasec))
		return false;
	option = control->data.select.items;
	item_y = line_height_with_spacing;
	
	while (item_y < scroll) {
		option = option->next;
		item_y += line_height_with_spacing;
	}
	item_y -= line_height_with_spacing;
	text_pos_offset = y - scroll +
			(int) (line_height * (0.75 + SELECT_LINE_SPACING));
	text_x = x + (box->border[LEFT].width + box->padding[LEFT]) * scale;
	
	plot_fstyle_entry.size = menu->f_size;
	
	while (option && item_y - scroll < height) {
		
 		if (option->selected) {
 			y2 = y + item_y - scroll;
 			y3 = y + item_y + line_height_with_spacing - scroll;
 			if (!plot->rectangle(x0, (y0 > y2 ? y0 : y2),
					scrollbar_x + 1,
     					(y3 < y1 + 1 ? y3 : y1 + 1),
					&plot_style_fill_selected))
				return false;
 		}
		
		y2 = text_pos_offset + item_y;
		if (!plot->text(text_x, y2, option->text,
				strlen(option->text), &plot_fstyle_entry))
			return false;
		
		item_y += line_height_with_spacing;
		option = option->next;
	}
		
	if (!scrollbar_redraw(menu->scrollbar,
			x_cp + menu->width - SCROLLBAR_WIDTH,
      			y_cp,
			clip, scale, ctx))
		return false;
	
	return true;
}

/**
 * Check whether a clipping rectangle is completely contained in the
 * select menu.
 *
 * \param control	the select menu to check the clipping rectangle for
 * \param scale		the current browser window scale
 * \param clip_x0	minimum x of clipping rectangle
 * \param clip_y0	minimum y of clipping rectangle
 * \param clip_x1	maximum x of clipping rectangle
 * \param clip_y1	maximum y of clipping rectangle
 * \return		true if inside false otherwise
 */
bool form_clip_inside_select_menu(struct form_control *control, float scale,
		const struct rect *clip)
{
	struct form_select_menu *menu = control->data.select.menu;
	int width, height;
	

	width = menu->width;
	height = menu->height;
	
	if (scale != 1.0) {
		width *= scale;
		height *= scale;
	}
	
	if (clip->x0 >= 0 && clip->x1 <= width &&
			clip->y0 >= 0 && clip->y1 <= height)
		return true;

	return false;
}


/**
 * Process a selection from a form select menu.
 *
 * \param  bw	    browser window with menu
 * \param  control  form control with menu
 * \param  item	    index of item selected from the menu
 */

static void form__select_process_selection(html_content *html,
		struct form_control *control, int item)
{
	struct box *inline_box;
	struct form_option *o;
	int count;

	assert(control != NULL);
	assert(html != NULL);

	/** \todo Even though the form code is effectively part of the html
	 *        content handler, poking around inside contents is not good */

	inline_box = control->box->children->children;

	for (count = 0, o = control->data.select.items;
			o != NULL;
			count++, o = o->next) {
		if (!control->data.select.multiple)
			o->selected = false;
		if (count == item) {
			if (control->data.select.multiple) {
				if (o->selected) {
					o->selected = false;
					control->data.select.num_selected--;
				} else {
					o->selected = true;
					control->data.select.num_selected++;
				}
			} else {
				o->selected = true;
			}
		}
		if (o->selected)
			control->data.select.current = o;
	}

	talloc_free(inline_box->text);
	inline_box->text = 0;
	if (control->data.select.num_selected == 0)
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_None"));
	else if (control->data.select.num_selected == 1)
		inline_box->text = talloc_strdup(html->bctx,
				control->data.select.current->text);
	else
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_Many"));
	if (!inline_box->text) {
		warn_user("NoMemory", 0);
		inline_box->length = 0;
	} else
		inline_box->length = strlen(inline_box->text);
	inline_box->width = control->box->width;

	html__redraw_a_box(html, control->box);
}


void form_select_process_selection(hlcache_handle *h,
		struct form_control *control, int item)
{
	assert(h != NULL);

	form__select_process_selection(
			(html_content *)hlcache_handle_get_content(h),
			control, item);
}

/**
 * Handle a click on the area of the currently opened select menu.
 * 
 * \param control	the select menu which received the click
 * \param x		X coordinate of click
 * \param y		Y coordinate of click
 */
void form_select_menu_clicked(struct form_control *control, int x, int y)
{	
	struct form_select_menu *menu = control->data.select.menu;
	struct form_option *option;
	html_content *html = (html_content *)menu->c;
	int line_height, line_height_with_spacing;	
	int item_bottom_y;
	int scroll, i;
	
	scroll = scrollbar_get_offset(menu->scrollbar);
	
	line_height = menu->line_height;
	line_height_with_spacing = line_height +
			line_height * SELECT_LINE_SPACING;
	
	option = control->data.select.items;
	item_bottom_y = line_height_with_spacing;
	i = 0;
	while (option && item_bottom_y < scroll + y) {
		item_bottom_y += line_height_with_spacing;
		option = option->next;
		i++;
	}
	
	if (option != NULL) {
		form__select_process_selection(html, control, i);
	}
	
	menu->callback(menu->client_data, 0, 0, menu->width, menu->height);
}

/**
 * Handle mouse action for the currently opened select menu.
 *
 * \param control	the select menu which received the mouse action
 * \param mouse		current mouse state
 * \param x		X coordinate of click
 * \param y		Y coordinate of click
 * \return		text for the browser status bar or NULL if the menu has
 * 			to be closed
 */
const char *form_select_mouse_action(struct form_control *control,
		browser_mouse_state mouse, int x, int y)
{
	struct form_select_menu *menu = control->data.select.menu;
	int x0, y0, x1, y1, scrollbar_x;
	const char *status = NULL;
	bool multiple = control->data.select.multiple;
	
	x0 = 0;
	y0 = 0;
	x1 = menu->width;
	y1 = menu->height;
	scrollbar_x = x1 - SCROLLBAR_WIDTH;
	
	if (menu->scroll_capture ||
			(x > scrollbar_x && x < x1 && y > y0 && y < y1)) {
		/* The scroll is currently capturing all events or the mouse
		 * event is taking place on the scrollbar widget area
		 */
		x -= scrollbar_x;
		return scrollbar_mouse_action(menu->scrollbar,
				    mouse, x, y);
	}
	
	
	if (x > x0 && x < scrollbar_x && y > y0 && y < y1) {
		/* over option area */
		
		if (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2))
			/* button 1 or 2 click */
			form_select_menu_clicked(control, x, y);
		
		if (!(mouse & BROWSER_MOUSE_CLICK_1 && !multiple))
			/* anything but a button 1 click over a single select
			   menu */
			status = messages_get(control->data.select.multiple ?
					"SelectMClick" : "SelectClick");
		
	} else if (!(mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)))
		/* if not a button 1 or 2 click*/
		status = messages_get("SelectClose");
			
	return status;
}

/**
 * Handle mouse drag end for the currently opened select menu.
 *
 * \param control	the select menu which received the mouse drag end
 * \param mouse		current mouse state
 * \param x		X coordinate of drag end
 * \param y		Y coordinate of drag end
 */
void form_select_mouse_drag_end(struct form_control *control,
		browser_mouse_state mouse, int x, int y)
{
	int x0, y0, x1, y1;
	int box_x, box_y;
	struct box *box;
	struct form_select_menu *menu = control->data.select.menu;

	box = control->box;

	/* Get global coords of scrollbar */
	box_coords(box, &box_x, &box_y);
	box_x -= box->border[LEFT].width;
	box_y += box->height + box->border[BOTTOM].width +
			box->padding[BOTTOM] + box->padding[TOP];

	/* Get drag end coords relative to scrollbar */
	x = x - box_x;
	y = y - box_y;

	if (menu->scroll_capture) {
		x -= menu->width - SCROLLBAR_WIDTH;
		scrollbar_mouse_drag_end(menu->scrollbar, mouse, x, y);
		return;
	}
	
	x0 = 0;
	y0 = 0;
	x1 = menu->width;
	y1 = menu->height;
		
	
	if (x > x0 && x < x1 - SCROLLBAR_WIDTH && y >  y0 && y < y1)
		/* handle drag end above the option area like a regular click */
		form_select_menu_clicked(control, x, y);
}

/**
 * Callback for the select menus scroll
 */
void form_select_menu_scroll_callback(void *client_data,
		struct scrollbar_msg_data *scrollbar_data)
{
	struct form_control *control = client_data;
	struct form_select_menu *menu = control->data.select.menu;
	html_content *html = (html_content *)menu->c;
	
	switch (scrollbar_data->msg) {
		case SCROLLBAR_MSG_MOVED:
			menu->callback(menu->client_data,
				    	0, 0,
					menu->width,
     					menu->height);
			break;
		case SCROLLBAR_MSG_SCROLL_START:
		{
			struct rect rect = {
				.x0 = scrollbar_data->x0,
				.y0 = scrollbar_data->y0,
				.x1 = scrollbar_data->x1,
				.y1 = scrollbar_data->y1
			};

			browser_window_set_drag_type(html->bw,
					DRAGGING_CONTENT_SCROLLBAR, &rect);

			menu->scroll_capture = true;
		}
			break;
		case SCROLLBAR_MSG_SCROLL_FINISHED:
			menu->scroll_capture = false;

			browser_window_set_drag_type(html->bw,
					DRAGGING_NONE, NULL);
			break;
		default:
			break;
	}
}

/**
 * Get the dimensions of a select menu.
 *
 * \param control	the select menu to get the dimensions of
 * \param width		gets updated to menu width
 * \param height	gets updated to menu height
 */
void form_select_get_dimensions(struct form_control *control,
		int *width, int *height)
{
	*width = control->data.select.menu->width;
	*height = control->data.select.menu->height;
}

/**
 * Callback for the core select menu.
 */
void form_select_menu_callback(void *client_data,
		int x, int y, int width, int height)
{
	html_content *html = client_data;
	int menu_x, menu_y;
	struct box *box;
	
	box = html->visible_select_menu->box;
	box_coords(box, &menu_x, &menu_y);
		
	menu_x -= box->border[LEFT].width;
	menu_y += box->height + box->border[BOTTOM].width +
			box->padding[BOTTOM] +
			box->padding[TOP];
	content__request_redraw((struct content *)html, menu_x + x, menu_y + y,
			width, height);
}


/**
 * Set a radio form control and clear the others in the group.
 *
 * \param  content  content containing the form, of type CONTENT_TYPE
 * \param  radio    form control of type GADGET_RADIO
 */

void form_radio_set(html_content *html,
		struct form_control *radio)
{
	struct form_control *control;

	assert(html);
	assert(radio);
	if (!radio->form)
		return;

	if (radio->selected)
		return;

	for (control = radio->form->controls; control;
			control = control->next) {
		if (control->type != GADGET_RADIO)
			continue;
		if (control == radio)
			continue;
		if (strcmp(control->name, radio->name) != 0)
			continue;

		if (control->selected) {
			control->selected = false;
			html__redraw_a_box(html, control->box);
		}
	}

	radio->selected = true;
	html__redraw_a_box(html, radio->box);
}


/**
 * Collect controls and submit a form.
 */

void form_submit(nsurl *page_url, struct browser_window *target,
		struct form *form, struct form_control *submit_button)
{
	char *data = NULL;
	struct fetch_multipart_data *success;
	nsurl *action;
	nsurl *action_query;

	assert(form != NULL);

	if (form_successful_controls(form, submit_button, &success) == false) {
		warn_user("NoMemory", 0);
		return;
	}

	switch (form->method) {
	case method_GET:
		data = form_url_encode(form, success, true);
		if (data == NULL) {
			fetch_multipart_data_destroy(success);
			warn_user("NoMemory", 0);
			return;
		}

		/* Decompose action */
		if (nsurl_create(form->action, &action) != NSERROR_OK) {
			free(data);
			fetch_multipart_data_destroy(success);
			warn_user("NoMemory", 0);
			return;
		}

		/* Replace query segment */
		if (nsurl_replace_query(action, data, &action_query) !=
				NSERROR_OK) {
			nsurl_unref(action);
			free(data);
			fetch_multipart_data_destroy(success);
			warn_user("NoMemory", 0);
			return;
		}

		/* Construct submit url */
		browser_window_go(target, nsurl_access(action_query),
				nsurl_access(page_url), true);
		nsurl_unref(action);
		nsurl_unref(action_query);
		break;

	case method_POST_URLENC:
		data = form_url_encode(form, success, false);
		if (data == NULL) {
			fetch_multipart_data_destroy(success);
			warn_user("NoMemory", 0);
			return;
		}

		browser_window_go_post(target, form->action, data, 0,
				true, nsurl_access(page_url), false, true, 0);
		break;

	case method_POST_MULTIPART:
		browser_window_go_post(target, form->action, 0, success,
				true, nsurl_access(page_url), false, true, 0);
		break;
	}

	fetch_multipart_data_destroy(success);
	free(data);
}
