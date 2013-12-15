/*
 * Copyright 2006 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2006 Richard Wilson <info@tinct.net>
 * Copyright 2008 Michael Drake <tlsa@netsurf-browser.org>
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
 * User interaction with a CONTENT_HTML (implementation).
 */

#include <assert.h>
#include <stdbool.h>

#include <dom/dom.h>

#include "content/content.h"
#include "desktop/browser.h"
#include "desktop/frames.h"
#include "desktop/mouse.h"
#include "desktop/options.h"
#include "desktop/scrollbar.h"
#include "desktop/selection.h"
#include "desktop/textinput.h"
#include "render/box.h"
#include "render/font.h"
#include "render/form.h"
#include "render/html_internal.h"
#include "render/imagemap.h"
#include "render/textinput.h"
#include "javascript/js.h"
#include "utils/messages.h"
#include "utils/utils.h"


/**
 * Get pointer shape for given box
 *
 * \param box       box in question
 * \param imagemap  whether an imagemap applies to the box
 */

static browser_pointer_shape get_pointer_shape(struct box *box, bool imagemap)
{
	browser_pointer_shape pointer;
	css_computed_style *style;
	enum css_cursor_e cursor;
	lwc_string **cursor_uris;

	if (box->type == BOX_FLOAT_LEFT || box->type == BOX_FLOAT_RIGHT)
		style = box->children->style;
	else
		style = box->style;

	if (style == NULL)
		return BROWSER_POINTER_DEFAULT;

	cursor = css_computed_cursor(style, &cursor_uris);

	switch (cursor) {
	case CSS_CURSOR_AUTO:
		if (box->href || (box->gadget &&
				(box->gadget->type == GADGET_IMAGE ||
				box->gadget->type == GADGET_SUBMIT)) ||
				imagemap) {
			/* link */
			pointer = BROWSER_POINTER_POINT;
		} else if (box->gadget &&
				(box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTAREA)) {
			/* text input */
			pointer = BROWSER_POINTER_CARET;
		} else {
			/* html content doesn't mind */
			pointer = BROWSER_POINTER_AUTO;
		}
		break;
	case CSS_CURSOR_CROSSHAIR:
		pointer = BROWSER_POINTER_CROSS;
		break;
	case CSS_CURSOR_POINTER:
		pointer = BROWSER_POINTER_POINT;
		break;
	case CSS_CURSOR_MOVE:
		pointer = BROWSER_POINTER_MOVE;
		break;
	case CSS_CURSOR_E_RESIZE:
		pointer = BROWSER_POINTER_RIGHT;
		break;
	case CSS_CURSOR_W_RESIZE:
		pointer = BROWSER_POINTER_LEFT;
		break;
	case CSS_CURSOR_N_RESIZE:
		pointer = BROWSER_POINTER_UP;
		break;
	case CSS_CURSOR_S_RESIZE:
		pointer = BROWSER_POINTER_DOWN;
		break;
	case CSS_CURSOR_NE_RESIZE:
		pointer = BROWSER_POINTER_RU;
		break;
	case CSS_CURSOR_SW_RESIZE:
		pointer = BROWSER_POINTER_LD;
		break;
	case CSS_CURSOR_SE_RESIZE:
		pointer = BROWSER_POINTER_RD;
		break;
	case CSS_CURSOR_NW_RESIZE:
		pointer = BROWSER_POINTER_LU;
		break;
	case CSS_CURSOR_TEXT:
		pointer = BROWSER_POINTER_CARET;
		break;
	case CSS_CURSOR_WAIT:
		pointer = BROWSER_POINTER_WAIT;
		break;
	case CSS_CURSOR_PROGRESS:
		pointer = BROWSER_POINTER_PROGRESS;
		break;
	case CSS_CURSOR_HELP:
		pointer = BROWSER_POINTER_HELP;
		break;
	default:
		pointer = BROWSER_POINTER_DEFAULT;
		break;
	}

	return pointer;
}


/**
 * Start drag scrolling the contents of a box
 *
 * \param box	the box to be scrolled
 * \param x	x ordinate of initial mouse position
 * \param y	y ordinate
 */

static void html_box_drag_start(struct box *box, int x, int y)
{
	int box_x, box_y;
	int scroll_mouse_x, scroll_mouse_y;

	box_coords(box, &box_x, &box_y);

	if (box->scroll_x != NULL) {
		scroll_mouse_x = x - box_x ;
		scroll_mouse_y = y - (box_y + box->padding[TOP] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_WIDTH);
		scrollbar_start_content_drag(box->scroll_x,
				scroll_mouse_x, scroll_mouse_y);
	} else if (box->scroll_y != NULL) {
		scroll_mouse_x = x - (box_x + box->padding[LEFT] +
				box->width + box->padding[RIGHT] -
				SCROLLBAR_WIDTH);
		scroll_mouse_y = y - box_y;

		scrollbar_start_content_drag(box->scroll_y,
				scroll_mouse_x, scroll_mouse_y);
	}
}


/**
 * End overflow scroll scrollbar drags
 *
 * \param  h      html content's high level cache entry
 * \param  mouse  state of mouse buttons and modifier keys
 * \param  x	  coordinate of mouse
 * \param  y	  coordinate of mouse
 */
static size_t html_selection_drag_end(struct html_content *html,
		browser_mouse_state mouse, int x, int y, int dir)
{
	int pixel_offset;
	struct box *box;
	int dx, dy;
	size_t idx = 0;

	box = box_pick_text_box(html, x, y, dir, &dx, &dy);
	if (box) {
		plot_font_style_t fstyle;

		font_plot_style_from_css(box->style, &fstyle);

		nsfont.font_position_in_string(&fstyle, box->text, box->length,
				dx, &idx, &pixel_offset);

		idx += box->byte_offset;
	}

	return idx;
}


/**
 * Handle mouse tracking (including drags) in an HTML content window.
 *
 * \param  c	  content of type html
 * \param  bw	  browser window
 * \param  mouse  state of mouse buttons and modifier keys
 * \param  x	  coordinate of mouse
 * \param  y	  coordinate of mouse
 */

void html_mouse_track(struct content *c, struct browser_window *bw,
		browser_mouse_state mouse, int x, int y)
{
	html_content *html = (html_content*) c;
	browser_drag_type drag_type = browser_window_get_drag_type(bw);

	if (drag_type == DRAGGING_SELECTION && !mouse) {
		int dir = -1;
		size_t idx;

		if (selection_dragging_start(&html->sel))
			dir = 1;

		idx = html_selection_drag_end(html, mouse, x, y, dir);

		if (idx != 0)
			selection_track(&html->sel, mouse, idx);

		browser_window_set_drag_type(bw, DRAGGING_NONE, NULL);
	}

	switch (drag_type) {
		case DRAGGING_SELECTION: {
			struct box *box;
			int dir = -1;
			int dx, dy;

			if (selection_dragging_start(&html->sel))
				dir = 1;

			box = box_pick_text_box(html, x, y, dir, &dx, &dy);

			if (box) {
				int pixel_offset;
				size_t idx;
				plot_font_style_t fstyle;

				font_plot_style_from_css(box->style, &fstyle);

				nsfont.font_position_in_string(&fstyle,
						box->text, box->length,
						dx, &idx, &pixel_offset);

				selection_track(&html->sel, mouse,
						box->byte_offset + idx);
			}
		}
		break;

		default:
			html_mouse_action(c, bw, mouse, x, y);
			break;
	}
}


/**
 * Handle mouse clicks and movements in an HTML content window.
 *
 * \param  c	  content of type html
 * \param  bw	  browser window
 * \param  mouse  state of mouse buttons and modifier keys
 * \param  x	  coordinate of mouse
 * \param  y	  coordinate of mouse
 *
 * This function handles both hovering and clicking. It is important that the
 * code path is identical (except that hovering doesn't carry out the action),
 * so that the status bar reflects exactly what will happen. Having separate
 * code paths opens the possibility that an attacker will make the status bar
 * show some harmless action where clicking will be harmful.
 */

void html_mouse_action(struct content *c, struct browser_window *bw,
		browser_mouse_state mouse, int x, int y)
{
	html_content *html = (html_content *) c;
	enum { ACTION_NONE, ACTION_SUBMIT, ACTION_GO } action = ACTION_NONE;
	const char *title = 0;
	nsurl *url = 0;
	const char *target = 0;
	char status_buffer[200];
	const char *status = 0;
	browser_pointer_shape pointer = BROWSER_POINTER_DEFAULT;
	bool imagemap = false;
	int box_x = 0, box_y = 0;
	int gadget_box_x = 0, gadget_box_y = 0;
	int html_object_pos_x = 0, html_object_pos_y = 0;
	int text_box_x = 0;
	struct box *url_box = 0;
	struct box *gadget_box = 0;
	struct box *text_box = 0;
	struct box *box;
	struct form_control *gadget = 0;
	hlcache_handle *object = NULL;
	struct box *html_object_box = NULL;
	struct browser_window *iframe = NULL;
	struct box *next_box;
	struct box *drag_candidate = NULL;
	struct scrollbar *scrollbar = NULL;
	plot_font_style_t fstyle;
	int scroll_mouse_x = 0, scroll_mouse_y = 0;
	int padding_left, padding_right, padding_top, padding_bottom;
	browser_drag_type drag_type = browser_window_get_drag_type(bw);
	union content_msg_data msg_data;
	struct dom_node *node = NULL;

	if (drag_type != DRAGGING_NONE && !mouse &&
			html->visible_select_menu != NULL) {
		/* drag end: select menu */
		form_select_mouse_drag_end(html->visible_select_menu,
				mouse, x, y);
	}

	if (html->visible_select_menu != NULL) {
		box = html->visible_select_menu->box;
		box_coords(box, &box_x, &box_y);

		box_x -= box->border[LEFT].width;
		box_y += box->height + box->border[BOTTOM].width +
				box->padding[BOTTOM] + box->padding[TOP];
		status = form_select_mouse_action(html->visible_select_menu,
				mouse, x - box_x, y - box_y);
		if (status != NULL) {
			msg_data.explicit_status_text = status;
			content_broadcast(c, CONTENT_MSG_STATUS, msg_data);
		} else {
			int width, height;
			form_select_get_dimensions(html->visible_select_menu,
					&width, &height);
			html->visible_select_menu = NULL;
			browser_window_redraw_rect(bw, box_x, box_y,
					width, height);					
		}
		return;
	}

	if (!mouse && html->scrollbar != NULL) {
		/* drag end: scrollbar */
		html_overflow_scroll_drag_end(html->scrollbar, mouse, x, y);
	}

	if (html->scrollbar != NULL) {
		struct html_scrollbar_data *data =
				scrollbar_get_data(html->scrollbar);
		box = data->box;
		box_coords(box, &box_x, &box_y);
		if (scrollbar_is_horizontal(html->scrollbar)) {
			scroll_mouse_x = x - box_x ;
			scroll_mouse_y = y - (box_y + box->padding[TOP] +
					box->height + box->padding[BOTTOM] -
					SCROLLBAR_WIDTH);
			status = scrollbar_mouse_action(html->scrollbar, mouse,
					scroll_mouse_x, scroll_mouse_y);
		} else {
			scroll_mouse_x = x - (box_x + box->padding[LEFT] +
					box->width + box->padding[RIGHT] -
					SCROLLBAR_WIDTH);
			scroll_mouse_y = y - box_y;
			status = scrollbar_mouse_action(html->scrollbar, mouse, 
					scroll_mouse_x, scroll_mouse_y);
		}

		msg_data.explicit_status_text = status;
		content_broadcast(c, CONTENT_MSG_STATUS, msg_data);
		return;
	}

	/* Content related drags handled by now */
	browser_window_set_drag_type(bw, DRAGGING_NONE, NULL);

	/* search the box tree for a link, imagemap, form control, or
	 * box with scrollbars 
	 */

	box = html->layout;

	/* Consider the margins of the html page now */
	box_x = box->margin[LEFT];
	box_y = box->margin[TOP];

	/* descend through visible boxes setting more specific values for:
	 * box - deepest box at point 
	 * html_object_box - html object
	 * html_object_pos_x - html object
	 * html_object_pos_y - html object
	 * object - non html object
	 * iframe - iframe
	 * url - href or imagemap
	 * target - href or imagemap or gadget
	 * url_box - href or imagemap
	 * imagemap - imagemap
	 * gadget - gadget
	 * gadget_box - gadget
	 * gadget_box_x - gadget
	 * gadget_box_y - gadget
	 * title - title
	 * pointer
	 *
	 * drag_candidate - first box with scroll
	 * padding_left - box with scroll
	 * padding_right
	 * padding_top
	 * padding_bottom
	 * scrollbar - inside padding box stops decent
	 * scroll_mouse_x - inside padding box stops decent
	 * scroll_mouse_y - inside padding box stops decent
	 * 
	 * text_box - text box
	 * text_box_x - text_box
	 */
	while ((next_box = box_at_point(box, x, y, &box_x, &box_y)) != NULL) {
		box = next_box;

		if ((box->style != NULL) && 
		    (css_computed_visibility(box->style) == 
		     CSS_VISIBILITY_HIDDEN)) {
			continue;
		}

		if (box->node != NULL) {
			node = box->node;
		}

		if (box->object) {
			if (content_get_type(box->object) == CONTENT_HTML) {
				html_object_box = box;
				html_object_pos_x = box_x;
				html_object_pos_y = box_y;
			} else {
				object = box->object;
			}
		}

		if (box->iframe) {
			iframe = box->iframe;
		}

		if (box->href) {
			url = box->href;
			target = box->target;
			url_box = box;
		}

		if (box->usemap) {
			url = imagemap_get(html, box->usemap,
					box_x, box_y, x, y, &target);
			if (url) {
				imagemap = true;
				url_box = box;
			}
		}

		if (box->gadget) {
			gadget = box->gadget;
			gadget_box = box;
			gadget_box_x = box_x;
			gadget_box_y = box_y;
			if (gadget->form)
				target = gadget->form->target;
		}

		if (box->title) {
			title = box->title;
		}

		pointer = get_pointer_shape(box, false);
		
		if ((box->scroll_x != NULL) || 
		    (box->scroll_y != NULL)) {

			if (drag_candidate == NULL) {
				drag_candidate = box;
			}

			padding_left = box_x +
					scrollbar_get_offset(box->scroll_x);
			padding_right = padding_left + box->padding[LEFT] +
					box->width + box->padding[RIGHT];
			padding_top = box_y +
					scrollbar_get_offset(box->scroll_y);
			padding_bottom = padding_top + box->padding[TOP] +
					box->height + box->padding[BOTTOM];
			
			if ((x > padding_left) && 
			    (x < padding_right) &&
			    (y > padding_top) && 
			    (y < padding_bottom)) {
				/* mouse inside padding box */
				
				if ((box->scroll_y != NULL) && 
				    (x > (padding_right - SCROLLBAR_WIDTH))) {
					/* mouse above vertical box scroll */
					
					scrollbar = box->scroll_y;
					scroll_mouse_x = x - (padding_right -
							     SCROLLBAR_WIDTH);
					scroll_mouse_y = y - padding_top;
					break;
				
				} else if ((box->scroll_x != NULL) &&
					   (y > (padding_bottom - SCROLLBAR_WIDTH))) {
					/* mouse above horizontal box scroll */
							
					scrollbar = box->scroll_x;
					scroll_mouse_x = x - padding_left;
					scroll_mouse_y = y - (padding_bottom -
							SCROLLBAR_WIDTH);
					break;
				}
			}
		}

		if (box->text && !box->object) {
			text_box = box;
			text_box_x = box_x;
		}
	}

	/* use of box_x, box_y, or content below this point is probably a
	 * mistake; they will refer to the last box returned by box_at_point */

	if (scrollbar) {
		status = scrollbar_mouse_action(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
		pointer = BROWSER_POINTER_DEFAULT;
	} else if (gadget) {
		switch (gadget->type) {
		case GADGET_SELECT:
			status = messages_get("FormSelect");
			pointer = BROWSER_POINTER_MENU;
			if (mouse & BROWSER_MOUSE_CLICK_1 &&
			    nsoption_bool(core_select_menu)) {
				html->visible_select_menu = gadget;
				form_open_select_menu(c, gadget,
						form_select_menu_callback,
						c);
				pointer =  BROWSER_POINTER_DEFAULT;
			} else if (mouse & BROWSER_MOUSE_CLICK_1)
				gui_create_form_select_menu(bw, gadget);
			break;
		case GADGET_CHECKBOX:
			status = messages_get("FormCheckbox");
			if (mouse & BROWSER_MOUSE_CLICK_1) {
				gadget->selected = !gadget->selected;
				html__redraw_a_box(html, gadget_box);
			}
			break;
		case GADGET_RADIO:
			status = messages_get("FormRadio");
			if (mouse & BROWSER_MOUSE_CLICK_1)
				form_radio_set(html, gadget);
			break;
		case GADGET_IMAGE:
			if (mouse & BROWSER_MOUSE_CLICK_1) {
				gadget->data.image.mx = x - gadget_box_x;
				gadget->data.image.my = y - gadget_box_y;
			}
			/* drop through */
		case GADGET_SUBMIT:
			if (gadget->form) {
				snprintf(status_buffer, sizeof status_buffer,
						messages_get("FormSubmit"),
						gadget->form->action);
				status = status_buffer;
				pointer = get_pointer_shape(gadget_box, false);
				if (mouse & (BROWSER_MOUSE_CLICK_1 |
						BROWSER_MOUSE_CLICK_2))
					action = ACTION_SUBMIT;
			} else {
				status = messages_get("FormBadSubmit");
			}
			break;
		case GADGET_TEXTAREA:
			status = messages_get("FormTextarea");
			pointer = get_pointer_shape(gadget_box, false);

			if (mouse & (BROWSER_MOUSE_PRESS_1 |
					BROWSER_MOUSE_PRESS_2)) {
				if (text_box && selection_root(&html->sel) !=
						gadget_box)
					selection_init(&html->sel, gadget_box);

				textinput_textarea_click(c, mouse,
						gadget_box,
						gadget_box_x,
						gadget_box_y,
						x - gadget_box_x,
						y - gadget_box_y);
			}

			if (text_box) {
				int pixel_offset;
				size_t idx;

				font_plot_style_from_css(text_box->style, 
						&fstyle);

				nsfont.font_position_in_string(&fstyle,
					text_box->text,
					text_box->length,
					x - gadget_box_x - text_box->x,
					&idx,
					&pixel_offset);

				selection_click(&html->sel, mouse,
						text_box->byte_offset + idx);

				if (selection_dragging(&html->sel)) {
					browser_window_set_drag_type(bw,
							DRAGGING_SELECTION,
							NULL);
					status = messages_get("Selecting");
				}
			}
			else if (mouse & BROWSER_MOUSE_PRESS_1)
				selection_clear(&html->sel, true);
			break;
		case GADGET_TEXTBOX:
		case GADGET_PASSWORD:
			status = messages_get("FormTextbox");
			pointer = get_pointer_shape(gadget_box, false);

			if ((mouse & BROWSER_MOUSE_PRESS_1) &&
					!(mouse & (BROWSER_MOUSE_MOD_1 |
					BROWSER_MOUSE_MOD_2))) {
				textinput_input_click(c,
						gadget_box,
						gadget_box_x,
						gadget_box_y,
						x - gadget_box_x,
						y - gadget_box_y);
			}
			if (text_box) {
				int pixel_offset;
				size_t idx;

				if (mouse & (BROWSER_MOUSE_DRAG_1 |
						BROWSER_MOUSE_DRAG_2))
					selection_init(&html->sel, gadget_box);

				font_plot_style_from_css(text_box->style,
						&fstyle);

				nsfont.font_position_in_string(&fstyle,
					text_box->text,
					text_box->length,
					x - gadget_box_x - text_box->x,
					&idx,
					&pixel_offset);

				selection_click(&html->sel, mouse,
						text_box->byte_offset + idx);

				if (selection_dragging(&html->sel))
					browser_window_set_drag_type(bw,
							DRAGGING_SELECTION,
							NULL);
			}
			else if (mouse & BROWSER_MOUSE_PRESS_1)
				selection_clear(&html->sel, true);
			break;
		case GADGET_HIDDEN:
			/* not possible: no box generated */
			break;
		case GADGET_RESET:
			status = messages_get("FormReset");
			break;
		case GADGET_FILE:
			status = messages_get("FormFile");
			break;
		case GADGET_BUTTON:
			/* This gadget cannot be activated */
			status = messages_get("FormButton");
			break;
		}

	} else if (object && (mouse & BROWSER_MOUSE_MOD_2)) {

		if (mouse & BROWSER_MOUSE_DRAG_2) {
			msg_data.dragsave.type = CONTENT_SAVE_NATIVE;
			msg_data.dragsave.content = object;
			content_broadcast(c, CONTENT_MSG_DRAGSAVE, msg_data);

		} else if (mouse & BROWSER_MOUSE_DRAG_1) {
			msg_data.dragsave.type = CONTENT_SAVE_ORIG;
			msg_data.dragsave.content = object;
			content_broadcast(c, CONTENT_MSG_DRAGSAVE, msg_data);
		}

		/* \todo should have a drag-saving object msg */

	} else if (iframe) {
		int pos_x, pos_y;
		float scale = browser_window_get_scale(bw);

		browser_window_get_position(iframe, false, &pos_x, &pos_y);

		pos_x /= scale;
		pos_y /= scale;

		if (mouse & BROWSER_MOUSE_CLICK_1 ||
				mouse & BROWSER_MOUSE_CLICK_2) {
			browser_window_mouse_click(iframe, mouse,
					x - pos_x, y - pos_y);
		} else {
			browser_window_mouse_track(iframe, mouse,
					x - pos_x, y - pos_y);
		}
	} else if (html_object_box) {
		if (mouse & BROWSER_MOUSE_CLICK_1 ||
				mouse & BROWSER_MOUSE_CLICK_2) {
			content_mouse_action(html_object_box->object,
					bw, mouse,
					x - html_object_pos_x,
					y - html_object_pos_y);
		} else {
			content_mouse_track(html_object_box->object,
					bw, mouse,
					x - html_object_pos_x,
					y - html_object_pos_y);
		}
	} else if (url) {
		if (title) {
			snprintf(status_buffer, sizeof status_buffer, "%s: %s",
					nsurl_access(url), title);
			status = status_buffer;
		} else
			status = nsurl_access(url);

		pointer = get_pointer_shape(url_box, imagemap);

		if (mouse & BROWSER_MOUSE_CLICK_1 &&
				mouse & BROWSER_MOUSE_MOD_1) {
			/* force download of link */
			browser_window_go_post(bw, nsurl_access(url), 0, 0,
					false,
					nsurl_access(content_get_url(c)),
					true, true, 0);

		} else if (mouse & BROWSER_MOUSE_CLICK_2 &&
				mouse & BROWSER_MOUSE_MOD_1) {
			msg_data.savelink.url = nsurl_access(url);
			msg_data.savelink.title = title;
			content_broadcast(c, CONTENT_MSG_SAVELINK, msg_data);

		} else if (mouse & (BROWSER_MOUSE_CLICK_1 |
				BROWSER_MOUSE_CLICK_2))
			action = ACTION_GO;
	} else {
		bool done = false;

		/* frame resizing */
		if (browser_window_frame_resize_start(bw, mouse, x, y,
				&pointer)) {
			if (mouse & (BROWSER_MOUSE_DRAG_1 |
					BROWSER_MOUSE_DRAG_2)) {
				status = messages_get("FrameDrag");
			}
			done = true;
		}

		/* if clicking in the main page, remove the selection from any
		 * text areas */
		if (!done) {
			struct box *layout = html->layout;

			if (mouse && (mouse < BROWSER_MOUSE_MOD_1) &&
					selection_root(&html->sel) != layout) {
				selection_init(&html->sel, layout);
			}

			if (text_box) {
				int pixel_offset;
				size_t idx;

				font_plot_style_from_css(text_box->style,
						&fstyle);

				nsfont.font_position_in_string(&fstyle,
					text_box->text,
					text_box->length,
					x - text_box_x,
					&idx,
					&pixel_offset);

				if (selection_click(&html->sel, mouse,
						text_box->byte_offset + idx)) {
					/* key presses must be directed at the
					 * main browser window, paste text
					 * operations ignored */

					if (selection_dragging(&html->sel)) {
						browser_window_set_drag_type(bw,
							DRAGGING_SELECTION,
							NULL);
						status = messages_get(
								"Selecting");
					}

					done = true;
				}

			} else if (mouse & BROWSER_MOUSE_PRESS_1)
				selection_clear(&html->sel, true);
		}

		if (!done) {
			if (title)
				status = title;

			if (mouse & BROWSER_MOUSE_DRAG_1) {
				if (mouse & BROWSER_MOUSE_MOD_2) {
					msg_data.dragsave.type =
							CONTENT_SAVE_COMPLETE;
					msg_data.dragsave.content = NULL;
					content_broadcast(c,
							CONTENT_MSG_DRAGSAVE,
							msg_data);
				} else {
					if (drag_candidate == NULL) {
						browser_window_page_drag_start(
								bw, x, y);
					} else {
						html_box_drag_start(
								drag_candidate,
								x, y);
					}
					pointer = BROWSER_POINTER_MOVE;
				}
			}
			else if (mouse & BROWSER_MOUSE_DRAG_2) {
				if (mouse & BROWSER_MOUSE_MOD_2) {
					msg_data.dragsave.type =
							CONTENT_SAVE_SOURCE;
					msg_data.dragsave.content = NULL;
					content_broadcast(c,
							CONTENT_MSG_DRAGSAVE,
							msg_data);
				} else {
					if (drag_candidate == NULL) {
						browser_window_page_drag_start(
								bw, x, y);
					} else {
						html_box_drag_start(
								drag_candidate,
								x, y);
					}
					pointer = BROWSER_POINTER_MOVE;
				}
			}
		}
		if (mouse && mouse < BROWSER_MOUSE_MOD_1) {
			/* ensure key presses still act on the browser window */
			browser_window_remove_caret(bw);
		}
	}

	if (!iframe && !html_object_box) {
		msg_data.explicit_status_text = status;
		content_broadcast(c, CONTENT_MSG_STATUS, msg_data);

		msg_data.pointer = pointer;
		content_broadcast(c, CONTENT_MSG_POINTER, msg_data);
	}

	/* fire dom click event */
	if ((mouse & BROWSER_MOUSE_CLICK_1) ||
	    (mouse & BROWSER_MOUSE_CLICK_2)) {
		js_fire_event(html->jscontext, "click", html->document, node);
	}

	/* deferred actions that can cause this browser_window to be destroyed
	 * and must therefore be done after set_status/pointer
	 */
	switch (action) {
	case ACTION_SUBMIT:
		form_submit(content_get_url(c),
				browser_window_find_target(bw, target, mouse),
				gadget->form, gadget);
		break;
	case ACTION_GO:
		browser_window_go(browser_window_find_target(bw, target, mouse),
				nsurl_access(url),
				nsurl_access(content_get_url(c)), true);
		break;
	case ACTION_NONE:
		break;
	}
}


/**
 * Callback for in-page scrollbars.
 */
void html_overflow_scroll_callback(void *client_data,
		struct scrollbar_msg_data *scrollbar_data)
{
	struct html_scrollbar_data *data = client_data;
	html_content *html = (html_content *)data->c;
	struct box *box = data->box;
	union content_msg_data msg_data;
	
	switch(scrollbar_data->msg) {
		case SCROLLBAR_MSG_MOVED:
			html__redraw_a_box(html, box);
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

			html->scrollbar = scrollbar_data->scrollbar;
		}
			break;
		case SCROLLBAR_MSG_SCROLL_FINISHED:
			html->scrollbar = NULL;

			browser_window_set_drag_type(html->bw,
					DRAGGING_NONE, NULL);

			msg_data.pointer = BROWSER_POINTER_AUTO;
			content_broadcast(data->c, CONTENT_MSG_POINTER,
					msg_data);
			break;
	}
}


/**
 * End overflow scroll scrollbar drags
 *
 * \param  scroll  scrollbar widget
 * \param  mouse   state of mouse buttons and modifier keys
 * \param  x	   coordinate of mouse
 * \param  y	   coordinate of mouse
 */
void html_overflow_scroll_drag_end(struct scrollbar *scrollbar,
		browser_mouse_state mouse, int x, int y)
{
	int scroll_mouse_x, scroll_mouse_y, box_x, box_y;
	struct html_scrollbar_data *data = scrollbar_get_data(scrollbar);
	struct box *box;

	box = data->box;
	box_coords(box, &box_x, &box_y);

	if (scrollbar_is_horizontal(scrollbar)) {
		scroll_mouse_x = x - box_x;
		scroll_mouse_y = y - (box_y + box->padding[TOP] +
				box->height + box->padding[BOTTOM] -
				SCROLLBAR_WIDTH);
		scrollbar_mouse_drag_end(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	} else {
		scroll_mouse_x = x - (box_x + box->padding[LEFT] +
				box->width + box->padding[RIGHT] -
				SCROLLBAR_WIDTH);
		scroll_mouse_y = y - box_y;
		scrollbar_mouse_drag_end(scrollbar, mouse,
				scroll_mouse_x, scroll_mouse_y);
	}
}
