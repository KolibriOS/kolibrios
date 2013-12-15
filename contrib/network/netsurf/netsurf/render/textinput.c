/*
 * Copyright 2003 Phil Mellor <monkeyson@users.sourceforge.net>
 * Copyright 2004 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2004 Andrew Timmins <atimmins@blueyonder.co.uk>
 * Copyright 2004 John Tytgat <joty@netsurf-browser.org>
 * Copyright 2005 Adrian Lees <adrianl@users.sourceforge.net>
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
 * HTML form text input handling (implementation)
 */

#include <assert.h>
#include <ctype.h>
#include <string.h>

#include <dom/dom.h>

#include "desktop/browser.h"
#include "desktop/gui.h"
#include "desktop/mouse.h"
#include "desktop/scrollbar.h"
#include "desktop/selection.h"
#include "desktop/textinput.h"
#include "render/box.h"
#include "render/font.h"
#include "render/form.h"
#include "render/html_internal.h"
#include "render/layout.h"
#include "render/textinput.h"
#include "utils/log.h"
#include "utils/talloc.h"
#include "utils/utf8.h"
#include "utils/utils.h"

/* Define to enable textinput debug */
#undef TEXTINPUT_DEBUG


static bool textinput_textbox_delete(struct content *c,
		struct box *text_box, unsigned char_offset,
		unsigned utf8_len);

/* Textarea callbacks */
static bool textinput_textarea_callback(struct browser_window *bw,
		uint32_t key, void *p1, void *p2);
static void textinput_textarea_move_caret(struct browser_window *bw,
		void *p1, void *p2);
static bool textinput_textarea_paste_text(struct browser_window *bw,
		const char *utf8, unsigned utf8_len, bool last,
		void *p1, void *p2);

/* Text input callbacks */
static bool textinput_input_callback(struct browser_window *bw,
		uint32_t key, void *p1, void *p2);
static void textinput_input_move_caret(struct browser_window *bw,
		void *p1, void *p2);
static bool textinput_input_paste_text(struct browser_window *bw,
		const char *utf8, unsigned utf8_len, bool last,
		void *p1, void *p2);

#define SPACE_LEN(b) ((b->space == 0) ? 0 : 1)


static struct textinput_buffer {
	char *buffer;
	size_t buffer_len;
	size_t length;
} textinput_buffer;



/**
 * Given the x,y co-ordinates of a point within a textarea, return the
 * TEXT box pointer, and the character and pixel offsets within that
 * box at which the caret should be positioned. (eg. for mouse clicks,
 * drag-and-drop insertions etc)
 *
 * \param  textarea       the textarea being considered
 * \param  x              x ordinate of point
 * \param  y              y ordinate of point
 * \param  pchar_offset   receives the char offset within the TEXT box
 * \param  ppixel_offset  receives the pixel offset within the TEXT box
 * \return pointer to TEXT box
 */

static struct box *textinput_textarea_get_position(struct box *textarea,
		int x, int y, int *pchar_offset, int *ppixel_offset)
{
	/* A textarea is an INLINE_BLOCK containing a single
	 * INLINE_CONTAINER, which contains the text as runs of TEXT
	 * separated by BR. There is at least one TEXT. The first and
	 * last boxes are TEXT. Consecutive BR may not be present. These
	 * constraints are satisfied by using a 0-length TEXT for blank
	 * lines. */

	struct box *inline_container, *text_box;
	plot_font_style_t fstyle;
	size_t char_offset = 0;

	inline_container = textarea->children;

	if (inline_container->y + inline_container->height < y) {
		/* below the bottom of the textarea: place caret at end */
		text_box = inline_container->last;
		assert(text_box->type == BOX_TEXT);
		assert(text_box->text);
		font_plot_style_from_css(text_box->style, &fstyle);
		/** \todo handle errors */
		nsfont.font_position_in_string(&fstyle, text_box->text,
				text_box->length,
				(unsigned int)(x - text_box->x),
				&char_offset, ppixel_offset);
	} else {
		/* find the relevant text box */
		y -= inline_container->y;
		for (text_box = inline_container->children;
				text_box &&
				text_box->y + text_box->height < y;
				text_box = text_box->next)
			;
		for (; text_box && text_box->type != BOX_BR &&
				text_box->y <= y &&
				text_box->x + text_box->width < x;
				text_box = text_box->next)
			;
		if (!text_box) {
			/* past last text box */
			text_box = inline_container->last;
			assert(text_box->type == BOX_TEXT);
			assert(text_box->text);
			font_plot_style_from_css(text_box->style, &fstyle);
			nsfont.font_position_in_string(&fstyle,
					text_box->text,
					text_box->length,
					textarea->width,
					&char_offset,
					ppixel_offset);
		} else {
			/* in a text box */
			if (text_box->type == BOX_BR)
				text_box = text_box->prev;
			else if (y < text_box->y && text_box->prev) {
				if (text_box->prev->type == BOX_BR) {
					assert(text_box->prev->prev);
					text_box = text_box->prev->prev;
				}
				else
					text_box = text_box->prev;
			}
			assert(text_box->type == BOX_TEXT);
			assert(text_box->text);
			font_plot_style_from_css(text_box->style, &fstyle);
			nsfont.font_position_in_string(&fstyle,
					text_box->text,
					text_box->length,
					(unsigned int)(x - text_box->x),
					&char_offset,
					ppixel_offset);
		}
	}

	*pchar_offset = char_offset;

	assert(text_box);
	return text_box;
}


/**
 * Delete some text from a box, or delete the box in its entirety
 *
 * \param  c       html content
 * \param  b       box
 * \param  offset  start offset of text to be deleted (in bytes)
 * \param  length  length of text to be deleted
 * \return true iff successful
 */

static bool textinput_delete_handler(struct content *c, struct box *b,
		int offset, size_t length)
{
	size_t text_length = b->length + SPACE_LEN(b);

	/* only remove if its not the first box */
	if (offset <= 0 && length >= text_length && b->prev != NULL) {
		/* remove the entire box */
		box_unlink_and_free(b);

		return true;
	} else
		return textinput_textbox_delete(c, b, offset,
				min(length, text_length - offset));
}


/**
 * Remove the selected text from a text box and gadget (if applicable)
 *
 * \param  c	The content containing the selection
 * \param  s	The selection to be removed
 */

static void textinput_delete_selection(struct content *c, struct selection *s)
{
	size_t start_offset, end_offset;
	struct box *text_box;
	struct box *end_box;
	struct box *next;
	size_t sel_len;
	int beginning = 0;

	assert(s->defined);

	text_box = selection_get_start(s, &start_offset);
	end_box = selection_get_end(s, &end_offset);
	sel_len = s->end_idx - s->start_idx;

	/* Clear selection so that deletion from textboxes proceeds */
	selection_clear(s, true);

	/* handle first box */
	textinput_delete_handler(c, text_box, start_offset, sel_len);
	if (text_box == end_box)
		return;

	for (text_box = text_box->next; text_box != end_box; text_box = next) {
		next = text_box->next;
		box_unlink_and_free(text_box);
	}

	textinput_delete_handler(c, end_box, beginning, end_offset);
}


/**
 * Insert a number of chars into a text box
 *
 * \param  c            html_content
 * \param  text_box     text box
 * \param  char_offset  offset (bytes) at which to insert text
 * \param  utf8         UTF-8 text to insert
 * \param  utf8_len     length (bytes) of UTF-8 text to insert
 * \return true iff successful
 */

static bool textinput_textbox_insert(struct content *c,
		struct box *text_box, unsigned char_offset, const char *utf8,
		unsigned utf8_len)
{
	html_content *html = (html_content *)c;
	char *text;
	struct box *input = text_box->parent->parent;
	bool hide;

	if (html->bw && html->sel.defined)
		textinput_delete_selection(c, &html->sel);

	/* insert into form gadget (text and password inputs only) */
	if (input->gadget && (input->gadget->type == GADGET_TEXTBOX ||
			input->gadget->type == GADGET_PASSWORD) &&
			input->gadget->value) {
		size_t form_offset = input->gadget->caret_form_offset;
		char *value = realloc(input->gadget->value,
				input->gadget->length + utf8_len + 1);
		if (!value) {
			warn_user("NoMemory", 0);
			return true;
		}
		input->gadget->value = value;

		memmove(input->gadget->value + form_offset + utf8_len,
				input->gadget->value + form_offset,
				input->gadget->length - form_offset);
		memcpy(input->gadget->value + form_offset, utf8, utf8_len);
		input->gadget->length += utf8_len;
		input->gadget->value[input->gadget->length] = 0;
	}

	hide = (input->gadget && input->gadget->type == GADGET_PASSWORD);
	if (hide) {
		/* determine the number of '*'s to be inserted */
		const char *eutf8 = utf8 + utf8_len;
		utf8_len = 0;
		while (utf8 < eutf8) {
			utf8 += utf8_next(utf8, eutf8 - utf8, 0);
			utf8_len++;
		}
	}

	/* insert in text box */
	text = talloc_realloc(html->bctx, text_box->text,
			char,
			text_box->length + SPACE_LEN(text_box) + utf8_len + 1);
	if (!text) {
		warn_user("NoMemory", 0);
		return false;
	}
	text_box->text = text;

	if (text_box->space != 0 &&
			char_offset == text_box->length + SPACE_LEN(text_box)) {
		if (hide)
			text_box->space = 0;
		else {
			unsigned int last_off = utf8_prev(utf8, utf8_len);
			if (utf8[last_off] != ' ')
				text_box->space = 0;
			else
				utf8_len = last_off;
		}
		text_box->text[text_box->length++] = ' ';
	} else {
		memmove(text_box->text + char_offset + utf8_len,
				text_box->text + char_offset,
				text_box->length - char_offset);
	}

	if (hide)
		memset(text_box->text + char_offset, '*', utf8_len);
	else
		memcpy(text_box->text + char_offset, utf8, utf8_len);
	text_box->length += utf8_len;

	/* nothing should assume that the text is terminated,
	 * but just in case */
	text_box->text[text_box->length] = 0;

	text_box->width = UNKNOWN_WIDTH;

	return true;
}

/**
 * Calculates the form_offset from the box_offset
 *
 * \param input		The root box containing both the textbox and gadget
 * \param text_box   	The textbox containing the caret
 * \param char_offset   The caret offset within text_box
 * \return the translated form_offset
 */

static size_t textinput_get_form_offset(struct box* input, struct box* text_box,
		size_t char_offset)
{
	int uchars;
	unsigned int offset;

	for (uchars = 0, offset = 0; offset < char_offset; uchars++) {
		if ((text_box->text[offset] & 0x80) == 0x00) {
			offset++;
			continue;
		}
		assert((text_box->text[offset] & 0xC0) == 0xC0);
		for (++offset; offset < char_offset &&
				(text_box->text[offset] & 0xC0) == 0x80;
				offset++)
			/* do nothing */;
	}
	/* uchars is the number of real Unicode characters at the left
	 * side of the caret.
	 */
	for (offset = 0; uchars > 0 && offset < input->gadget->length;
			uchars--) {
		if ((input->gadget->value[offset] & 0x80) == 0x00) {
			offset++;
			continue;
		}
		assert((input->gadget->value[offset] & 0xC0) == 0xC0);
		for (++offset; offset < input->gadget->length &&
			(input->gadget->value[offset] & 0xC0) == 0x80;
				offset++)
			/* do nothing */;
	}
	assert(uchars == 0);
	return offset;
}


/**
 * Delete a number of chars from a text box
 *
 * \param  c            html content
 * \param  text_box     text box
 * \param  char_offset  offset within text box (bytes) of first char to delete
 * \param  utf8_len     length (bytes) of chars to be deleted
 * \return true on success, false otherwise
 *
 * ::char_offset and ::utf8_len are only considered when there is no selection.
 * If there is a selection, the entire selected area is deleted.
 */

bool textinput_textbox_delete(struct content *c, struct box *text_box,
		unsigned char_offset, unsigned utf8_len)
{
	html_content *html = (html_content *)c;
	unsigned next_offset = char_offset + utf8_len;
	struct box *form = text_box->parent->parent;

	if (html->bw && html->sel.defined) {
		textinput_delete_selection(c, &html->sel);
		return true;
	}

	/* delete from form gadget (text and password inputs only) */
	if (form->gadget && (form->gadget->type == GADGET_TEXTBOX ||
			form->gadget->type == GADGET_PASSWORD) &&
			form->gadget->value) {
		size_t form_offset = textinput_get_form_offset(form, text_box,
						char_offset);
		size_t next_offset = textinput_get_form_offset(form, text_box,
						char_offset + utf8_len);

		memmove(form->gadget->value + form_offset,
				form->gadget->value + next_offset,
				form->gadget->length - next_offset);
		form->gadget->length -= (next_offset - form_offset);
		form->gadget->value[form->gadget->length] = 0;
	}

	/* delete from visible textbox */
	if (next_offset <= text_box->length + SPACE_LEN(text_box)) {
		/* handle removal of trailing space */
		if (text_box->space != 0 && next_offset > text_box->length) {
			if (char_offset > 0) {
				/* is the trailing character still a space? */
				int tmp = utf8_prev(text_box->text, char_offset);
				if (isspace(text_box->text[tmp]))
					char_offset = tmp;
				else
					text_box->space = 0;
			} else {
				text_box->space = 0;
			}

			text_box->length = char_offset;
		} else {
			memmove(text_box->text + char_offset,
					text_box->text + next_offset,
					text_box->length - next_offset);
			text_box->length -= utf8_len;
		}

		/* nothing should assume that the text is terminated,
		 * but just in case */
		text_box->text[text_box->length] = 0;

		text_box->width = UNKNOWN_WIDTH;

		return true;
	}

	return false;
}

/**
 * Locate the first inline box at the start of this line
 *
 * \param  text_box  text box from which to start searching
 */

static struct box *textinput_line_start(struct box *text_box)
{
	while (text_box->prev && text_box->prev->type == BOX_TEXT)
		text_box = text_box->prev;
	return text_box;
}


/**
 * Locate the last inline box in this line
 *
 * \param  text_box  text box from which to start searching
 */

static struct box *textinput_line_end(struct box *text_box)
{
	while (text_box->next && text_box->next->type == BOX_TEXT)
		text_box = text_box->next;
	return text_box;
}


/**
 * Backtrack to the start of the previous line, if there is one.
 */

static struct box *textinput_line_above(struct box *text_box)
{
	struct box *prev;

	text_box = textinput_line_start(text_box);

	prev = text_box->prev;
	while (prev && prev->type == BOX_BR)
		prev = prev->prev;

	return prev ? textinput_line_start(prev) : text_box;
}


/**
 * Advance to the start of the next line, if there is one.
 */

static struct box *textinput_line_below(struct box *text_box)
{
	struct box *next;

	text_box = textinput_line_end(text_box);

	next = text_box->next;
	while (next && next->type == BOX_BR)
		next = next->next;

	return next ? next : text_box;
}


/**
 * Add some text to the buffer, optionally appending a trailing space.
 *
 * \param text text to be added
 * \param length length of text in bytes
 * \param space indicates whether a trailing space should be appended
 * \param fstyle The font style
 * \return true if successful
 */

static bool textinput_add_to_buffer(const char *text, size_t length, bool space,
		const plot_font_style_t *fstyle)
{
	size_t new_length = textinput_buffer.length + length + (space ? 1 : 0) + 1;

	if (new_length > textinput_buffer.buffer_len) {
		size_t new_alloc = new_length + (new_length / 4);
		char *new_buff;

		new_buff = realloc(textinput_buffer.buffer, new_alloc);
		if (new_buff == NULL)
			return false;

		textinput_buffer.buffer = new_buff;
		textinput_buffer.buffer_len = new_alloc;
	}

	memcpy(textinput_buffer.buffer + textinput_buffer.length, text, length);
	textinput_buffer.length += length;

	if (space)
		textinput_buffer.buffer[textinput_buffer.length++] = ' ';

	textinput_buffer.buffer[textinput_buffer.length] = '\0';

	return true;
}


/**
 * Empty the buffer, called prior to textinput_add_to_buffer sequence
 *
 * \return true iff successful
 */

static bool textinput_empty_buffer(void)
{
	const size_t init_size = 1024;

	if (textinput_buffer.buffer_len == 0) {
		textinput_buffer.buffer = malloc(init_size);
		if (textinput_buffer.buffer == NULL)
			return false;

		textinput_buffer.buffer_len = init_size;
	}

	textinput_buffer.length = 0;

	return true;
}


/**
 * Cut a range of text from a text box,
 * possibly placing it on the global clipboard.
 *
 * \param  c          html content
 * \param  start_box  text box at start of range
 * \param  start_idx  index (bytes) within start box
 * \param  end_box    text box at end of range
 * \param  end_idx    index (bytes) within end box
 * \param  clipboard  whether to place text on the clipboard
 * \return true iff successful
 */

static bool textinput_textarea_cut(struct content *c,
		struct box *start_box, unsigned start_idx,
		struct box *end_box, unsigned end_idx,
		bool clipboard)
{
	struct box *box = start_box;
	bool success = true;
	bool del = false;	/* caller expects start_box to persist */

	if (textinput_empty_buffer() == false) {
		return false;
	}

	while (box && box != end_box) {
		/* read before deletion, in case the whole box goes */
		struct box *next = box->next;

		if (box->type == BOX_BR) {
			if (clipboard && !textinput_add_to_buffer("\n", 1,
					false, plot_style_font)) {
				return false;
			}
			box_unlink_and_free(box);
		} else {
			/* append box text to clipboard and then delete it */
			if (clipboard &&
				!textinput_add_to_buffer(box->text + start_idx,
					box->length - start_idx,
					SPACE_LEN(box), plot_style_font)) {
				return false;
			}

			if (del) {
				if (!textinput_delete_handler(c, box,
						start_idx,
						(box->length + SPACE_LEN(box)) -
						start_idx) && clipboard) {
					return false;
				}
			} else {
				textinput_textbox_delete(c, box, start_idx,
					(box->length + SPACE_LEN(box)) -
					start_idx);
			}
		}

		del = true;
		start_idx = 0;
		box = next;
	}

	/* and the last box */
	if (box) {
		if (clipboard && !textinput_add_to_buffer(box->text + start_idx,
				end_idx - start_idx, end_idx > box->length,
				plot_style_font)) {
			success = false;
		} else {
			if (del) {
				if (!textinput_delete_handler(c, box,
						start_idx, end_idx - start_idx))
					success = false;
			} else {
				textinput_textbox_delete(c, box, start_idx,
						end_idx - start_idx);
			}
		}
	}

	if (clipboard) {
		gui_set_clipboard(textinput_buffer.buffer,
				textinput_buffer.length, NULL, 0);
	}

	return true;
}


/**
 * Break a text box into two
 *
 * \param  c            html content
 * \param  text_box     text box to be split
 * \param  char_offset  offset (in bytes) at which text box is to be split
 */

static struct box *textinput_textarea_insert_break(struct content *c,
		struct box *text_box, size_t char_offset)
{
	html_content *html = (html_content *)c;
	struct box *new_br, *new_text;
	char *text;

	text = talloc_array(html->bctx, char, text_box->length + 1);
	if (!text) {
		warn_user("NoMemory", 0);
		return NULL;
	}

	new_br = box_create(NULL, text_box->style, false, 0, 0, text_box->title,
			0, html->bctx);
	new_text = talloc(html->bctx, struct box);
	if (!new_text) {
		warn_user("NoMemory", 0);
		return NULL;
	}

	new_br->type = BOX_BR;
	box_insert_sibling(text_box, new_br);

	memcpy(new_text, text_box, sizeof (struct box));
	new_text->flags |= CLONE;
	new_text->text = text;
	memcpy(new_text->text, text_box->text + char_offset,
			text_box->length - char_offset);
	new_text->length = text_box->length - char_offset;
	text_box->length = char_offset;
	text_box->width = new_text->width = UNKNOWN_WIDTH;
	box_insert_sibling(new_br, new_text);

	return new_text;
}


/**
 * Reflow textarea preserving width and height
 *
 * \param  c                 html content
 * \param  textarea          text area box
 * \param  inline_container  container holding text box
 */

static void textinput_textarea_reflow(struct content *c,
		struct box *textarea, struct box *inline_container)
{
	int width = textarea->width;
	int height = textarea->height;

	assert(c != NULL);

	if (!layout_inline_container(inline_container, width,
			textarea, 0, 0, (struct html_content *) c))
		warn_user("NoMemory", 0);
	textarea->width = width;
	textarea->height = height;
	layout_calculate_descendant_bboxes(textarea);
	box_handle_scrollbars(c, textarea,
			box_hscrollbar_present(textarea),
			box_vscrollbar_present(textarea));
}


/**
 * Move to the start of the word containing the given character position,
 * or the start of the preceding word if already at the start of this one.
 *
 * \param  text     UTF-8 text string
 * \param  poffset  offset of caret within string (updated on exit)
 * \param  pchars   receives the number of characters skipped
 * \return true iff the start of a word was found before/at the string start
 */

static bool textinput_word_left(const char *text,
		size_t *poffset, size_t *pchars)
{
	size_t offset = *poffset;
	bool success = false;
	size_t nchars = 0;

	/* Skip any spaces immediately prior to the offset */
	while (offset > 0) {
		offset = utf8_prev(text, offset);
		nchars++;
		if (!isspace(text[offset])) break;
	}

	/* Now skip all non-space characters */
	while (offset > 0) {
		size_t prev = utf8_prev(text, offset);
		success = true;
		if (isspace(text[prev]))
			break;
		offset = prev;
		nchars++;
	}

	*poffset = offset;
	if (pchars) *pchars = nchars;

	return success;
}


/**
 * Move to the start of the first word following the given character position.
 *
 * \param  text     UTF-8 text string
 * \param  len      length of string in bytes
 * \param  poffset  offset of caret within string (updated on exit)
 * \param  pchars   receives the number of characters skipped
 * \return true iff the start of a word was found before the string end
 */

static bool textinput_word_right(const char *text, size_t len,
		size_t *poffset, size_t *pchars)
{
	size_t offset = *poffset;
	bool success = false;
	size_t nchars = 0;

	/* Skip all non-space characters after the offset */
	while (offset < len) {
		if (isspace(text[offset])) break;
		offset = utf8_next(text, len, offset);
		nchars++;
	}

	/* Now skip all space characters */
	while (offset < len) {
		offset = utf8_next(text, len, offset);
		nchars++;
		if (offset < len && !isspace(text[offset])) {
			success = true;
			break;
		}
	}

	*poffset = offset;
	if (pchars) *pchars = nchars;

	return success;
}

/**
 * Adjust scroll offsets so that the caret is visible
 *
 * \param c         html content where click ocurred
 * \param textarea  textarea box
 * \return true if a change in scroll offsets has occurred
*/

static bool textinput_ensure_caret_visible(struct content *c,
		struct box *textarea)
{
	html_content *html = (html_content *)c;
	int cx, cy;
	int scrollx, scrolly;

	assert(textarea->gadget);

	scrollx = scrollbar_get_offset(textarea->scroll_x);
	scrolly = scrollbar_get_offset(textarea->scroll_y);

	/* Calculate the caret coordinates */
	cx = textarea->gadget->caret_pixel_offset +
			textarea->gadget->caret_text_box->x;
	cy = textarea->gadget->caret_text_box->y;

	/* Ensure they are visible */
	if (textarea->scroll_x == NULL) {
		scrollx = 0;
	} else if (cx - scrollbar_get_offset(textarea->scroll_x) < 0) {
		scrollx = cx;
	} else if (cx > scrollbar_get_offset(textarea->scroll_x) +
			textarea->width) {
		scrollx = cx - textarea->width;
	}

	if (textarea->scroll_y == NULL) {
		scrolly = 0;
	} else if (cy - scrollbar_get_offset(textarea->scroll_y) < 0) {
		scrolly = cy;
	} else if (cy + textarea->gadget->caret_text_box->height >
			scrollbar_get_offset(textarea->scroll_y) +
			textarea->height) {
		scrolly = (cy + textarea->gadget->caret_text_box->height) -
				textarea->height;
	}

	if ((scrollx == scrollbar_get_offset(textarea->scroll_x)) &&
			(scrolly == scrollbar_get_offset(textarea->scroll_y)))
		return false;

	if (textarea->scroll_x != NULL)	{
		html->scrollbar = textarea->scroll_x;
		scrollbar_set(textarea->scroll_x, scrollx, false);
		html->scrollbar = NULL;
	}
	if (textarea->scroll_y != NULL) {
		html->scrollbar = textarea->scroll_x;
		scrollbar_set(textarea->scroll_y, scrolly, false);
		html->scrollbar = NULL;
	}

	return true;
}


/**
 * Paste a block of text into a textarea at the
 * current caret position.
 *
 * \param  bw        browser window
 * \param  utf8      pointer to block of text
 * \param  utf8_len  length (bytes) of text block
 * \param  last      true iff this is the last chunk (update screen too)
 * \param  p1        pointer to textarea
 * \param  p2        html content with the text area box
 * \return true iff successful
 */

bool textinput_textarea_paste_text(struct browser_window *bw,
		const char *utf8, unsigned utf8_len, bool last,
		void *p1, void *p2)
{
	struct box *textarea = p1;
	struct content *c = p2;
	struct box *inline_container =
			textarea->gadget->caret_inline_container;
	struct box *text_box = textarea->gadget->caret_text_box;
	size_t char_offset = textarea->gadget->caret_box_offset;
	int pixel_offset = textarea->gadget->caret_pixel_offset;
	const char *ep = utf8 + utf8_len;
	const char *p = utf8;
	bool success = true;
	bool update = last;

	while (p < ep) {
		struct box *new_text;
		unsigned utf8_len;

		while (p < ep) {
			if (*p == '\n' || *p == '\r') break;
			p++;
		}

		utf8_len = p - utf8;
		if (!textinput_textbox_insert(c, text_box, char_offset,
				utf8, utf8_len))
			return false;

		char_offset += utf8_len;
		if (p == ep)
			break;

		new_text = textinput_textarea_insert_break(c, text_box,
				char_offset);
		if (!new_text) {
			/* we still need to update the screen */
			update = true;
			success = false;
			break;
		}

		/* place caret at start of new text box */
		text_box = new_text;
		char_offset = 0;

		/* handle CR/LF and LF/CR terminations */
		if ((*p == '\n' && p[1] == '\r') ||
				(*p == '\r' && p[1] == '\n'))
			 p++;
		utf8 = ++p;
	}

//	textarea->gadget->caret_inline_container = inline_container;
	textarea->gadget->caret_text_box = text_box;
	textarea->gadget->caret_box_offset = char_offset;

	if (update) {
		int box_x, box_y;
		plot_font_style_t fstyle;

		/* reflow textarea preserving width and height */
		textinput_textarea_reflow(c, textarea, inline_container);
		/* reflowing may have broken our caret offset
		 * this bit should hopefully continue to work if
		 * textarea_reflow is fixed to update the caret itself */
		char_offset = textarea->gadget->caret_box_offset;
		text_box = textarea->gadget->caret_text_box;

		while ((char_offset > text_box->length + SPACE_LEN(text_box)) &&
				(text_box->next) &&
				(text_box->next->type == BOX_TEXT)) {
#ifdef TEXTINPUT_DEBUG
			LOG(("Caret out of range: Was %d in boxlen %d "
					"space %d", char_offset,
					text_box->length, SPACE_LEN(text_box)));
#endif
			char_offset -= text_box->length + SPACE_LEN(text_box);
			text_box = text_box->next;
		}

		/* not sure if this will happen or not...
		 * but won't stick an assert here as we can recover from it */
		if (char_offset > text_box->length) {
#ifdef TEXTINPUT_DEBUG
			LOG(("Caret moved beyond end of line: "
				"Was %d in boxlen %d", char_offset,
				text_box->length));
#endif
			char_offset = text_box->length;
		}

		textarea->gadget->caret_text_box = text_box;
		textarea->gadget->caret_box_offset = char_offset;

		font_plot_style_from_css(text_box->style, &fstyle);

		nsfont.font_width(&fstyle, text_box->text,
				char_offset, &pixel_offset);

		textarea->gadget->caret_pixel_offset = pixel_offset;

		box_coords(textarea, &box_x, &box_y);
		box_x += scrollbar_get_offset(textarea->scroll_x);
		box_y += scrollbar_get_offset(textarea->scroll_y);
		textinput_ensure_caret_visible(c, textarea);
		box_x -= scrollbar_get_offset(textarea->scroll_x);
		box_y -= scrollbar_get_offset(textarea->scroll_y);

		browser_window_place_caret(bw,
				box_x + inline_container->x + text_box->x +
				pixel_offset,
				box_y + inline_container->y + text_box->y,
				text_box->height,
				textinput_textarea_callback,
				textinput_textarea_paste_text,
				textinput_textarea_move_caret,
				textarea, c);

		html__redraw_a_box((html_content *)c, textarea);
	}

	return success;
}


/**
 * Move caret to new position after reformatting
 *
 * \param  bw   browser window
 * \param  p1   pointer textarea box
 * \param  p2   html content with the text area box
 * \return none
 */

void textinput_textarea_move_caret(struct browser_window *bw,
		void *p1, void *p2)
{
	struct box *textarea = p1;
	struct content *c = p2;
	struct box *inline_container = textarea->gadget->caret_inline_container;
	struct box *text_box = textarea->gadget->caret_text_box;
	size_t char_offset = textarea->gadget->caret_box_offset;
	int pixel_offset;
	int box_x, box_y;
	plot_font_style_t fstyle;

	font_plot_style_from_css(text_box->style, &fstyle);

	box_coords(textarea, &box_x, &box_y);
	box_x -= scrollbar_get_offset(textarea->scroll_x);
	box_y -= scrollbar_get_offset(textarea->scroll_y);

	nsfont.font_width(&fstyle, text_box->text,
			char_offset, &pixel_offset);

	browser_window_place_caret(bw,
			box_x + inline_container->x + text_box->x +
			pixel_offset,
			box_y + inline_container->y + text_box->y,
			text_box->height,
			textinput_textarea_callback,
			textinput_textarea_paste_text,
			textinput_textarea_move_caret,
			textarea, c);
}


/**
 * Update display to reflect modified input field
 *
 * \param  bw           browser window
 * \param  input        input field
 * \param  form_offset
 * \param  box_offset   offset of caret within text box
 * \param  to_textarea  caret is to be moved to a textarea
 * \param  redraw       force redraw even if field hasn't scrolled
 */

static void textinput_input_update_display(struct content *c, struct box *input,
		unsigned box_offset, bool to_textarea, bool redraw)
{
	struct box *text_box = input->children->children;
	unsigned pixel_offset;
	int box_x, box_y;
	int dx;
	plot_font_style_t fstyle;
	html_content *html = (html_content *)c;

	font_plot_style_from_css(text_box->style, &fstyle);

	if (redraw)
		nsfont.font_width(&fstyle, text_box->text, text_box->length,
			&text_box->width);

	box_coords(input, &box_x, &box_y);

	nsfont.font_width(&fstyle, text_box->text, box_offset,
			(int *) &pixel_offset);

	/* Shift text box horizontally, so caret is visible */
	dx = text_box->x;
	text_box->x = 0;
	if (input->width < text_box->width &&
			input->width / 2 < (int) pixel_offset) {
		/* Make caret appear in centre of text input */
		text_box->x = input->width / 2 - pixel_offset;
		/* Clamp if we've shifted too far left */
		if (text_box->x < input->width - text_box->width)
			text_box->x = input->width - text_box->width;
	}
	dx -= text_box->x;
	input->gadget->caret_pixel_offset = pixel_offset;

	if (to_textarea) {
		/* moving to textarea so need to set these up */
		input->gadget->caret_inline_container = input->children;
		input->gadget->caret_text_box = text_box;
	}

	input->gadget->caret_box_offset = box_offset;

	browser_window_place_caret(html->bw,
			box_x + input->children->x +
					text_box->x + pixel_offset,
			box_y + input->children->y + text_box->y,
			text_box->height,
			/* use the appropriate callback */
			to_textarea ? textinput_textarea_callback
					: textinput_input_callback,
			to_textarea ? textinput_textarea_paste_text
					: textinput_input_paste_text,
			to_textarea ? textinput_textarea_move_caret
					: textinput_input_move_caret,
			input, c);

	if (dx || redraw)
		html__redraw_a_box(html, input);
}


/**
 * Key press callback for text areas.
 *
 * \param bw   The browser window containing the text area
 * \param key  The ucs4 character codepoint
 * \param p1   The text area box
 * \param p2   The html content with the text area box
 * \return     true if the keypress is dealt with, false otherwise.  It can
 *             return true even if it ran out of memory; this just means that
 *             it would have claimed it if it could.
 */
bool textinput_textarea_callback(struct browser_window *bw, uint32_t key,
		void *p1, void *p2)
{
	struct box *textarea = p1;
	struct content *c = p2;
	html_content *html = (html_content *)c;
	struct box *inline_container =
				textarea->gadget->caret_inline_container;
	struct box *text_box = textarea->gadget->caret_text_box;
	struct box *new_text;
	size_t char_offset = textarea->gadget->caret_box_offset;
	int pixel_offset = textarea->gadget->caret_pixel_offset;
	int box_x, box_y;
	char utf8[6];
	unsigned int utf8_len;
	bool scrolled, reflow = false;
	bool selection_exists = html->sel.defined;
	plot_font_style_t fstyle;

	/* box_dump(textarea, 0); */
#ifdef TEXTINPUT_DEBUG
	LOG(("key %i at %i in '%.*s'", key, char_offset,
			(int) text_box->length, text_box->text));
#endif

	box_coords(textarea, &box_x, &box_y);
	box_x -= scrollbar_get_offset(textarea->scroll_x);
	box_y -= scrollbar_get_offset(textarea->scroll_y);

	if (!(key <= 0x001F || (0x007F <= key && key <= 0x009F))) {
		/* normal character insertion */
		utf8_len = utf8_from_ucs4(key, utf8);

		if (!textinput_textbox_insert(c, text_box, char_offset,
				utf8, utf8_len))
			return true;

		char_offset += utf8_len;
		reflow = true;

	} else switch (key) {
	case KEY_DELETE_LEFT:
		if (selection_exists) {
			/* Have a selection; delete it */
			textinput_textbox_delete(c, text_box, 0, 0);
		} else if (char_offset == 0) {
			/* at the start of a text box */
			struct box *prev;

			if (text_box->prev && text_box->prev->type == BOX_BR) {
				/* previous box is BR: remove it */
				box_unlink_and_free(text_box->prev);
			}

			/* This needs to be after the BR removal, as that may
			 * result in no previous box existing */
			if (!text_box->prev)
				/* at very beginning of text area: ignore */
				return true;

			/* delete space by merging with previous text box */
			prev = text_box->prev;
			assert(prev->type == BOX_TEXT);
			assert(prev->text);

			char_offset = prev->length;	/* caret at join */

			if (!textinput_textbox_insert(c, prev, prev->length,
					text_box->text, text_box->length))
				return true;

			box_unlink_and_free(text_box);

			/* place caret at join (see above) */
			text_box = prev;

		} else {
			/* delete a character */
			size_t prev_offset = char_offset;
			size_t new_offset =
					utf8_prev(text_box->text, char_offset);

			if (textinput_textbox_delete(c, text_box, new_offset,
					prev_offset - new_offset))
				char_offset = new_offset;
		}
		reflow = true;
		break;

	case KEY_DELETE_LINE_START:
	{
		struct box *start_box = textinput_line_start(text_box);

		/* Clear the selection, if one exists */
		if (selection_exists)
			selection_clear(&html->sel, false);

		textinput_textarea_cut(c, start_box, 0, text_box,
				char_offset, false);
		text_box = start_box;
		char_offset = 0;
		reflow = true;
	}
		break;

	case KEY_DELETE_LINE_END:
	{
		struct box *end_box = textinput_line_end(text_box);

		/* Clear the selection, if one exists */
		if (selection_exists)
			selection_clear(&html->sel, false);

		if (end_box != text_box ||
			char_offset < text_box->length + SPACE_LEN(text_box)) {
			/* there's something at the end of the line to delete */
			textinput_textarea_cut(c, text_box,
					char_offset, end_box,
					end_box->length + SPACE_LEN(end_box),
					false);
			reflow = true;
			break;
		}
	}
	/* no break */
	case KEY_DELETE_RIGHT:	/* delete to right */
		if (selection_exists) {
			/* Delete selection */
			textinput_textbox_delete(c, text_box, 0, 0);
		} else if (char_offset >= text_box->length) {
			/* at the end of a text box */
			struct box *next;

			if (text_box->next && text_box->next->type == BOX_BR) {
				/* next box is a BR: remove it */
				box_unlink_and_free(text_box->next);
			}

			/* This test is after the BR removal, as that may
			 * result in no subsequent box being present */
			if (!text_box->next)
				/* at very end of text area: ignore */
				return true;

			/* delete space by merging with next text box */

			next = text_box->next;
			assert(next->type == BOX_TEXT);
			assert(next->text);

			if (!textinput_textbox_insert(c, text_box,
					text_box->length,
					next->text, next->length))
				return true;

			box_unlink_and_free(next);

			/* leave caret at join */
		} else {
			/* delete a character */
			size_t next_offset = utf8_next(text_box->text,
					text_box->length, char_offset);

			textinput_textbox_delete(c, text_box, char_offset,
					next_offset - char_offset);
		}
		reflow = true;
		break;

	case KEY_NL:
	case KEY_CR:	/* paragraph break */
		if (selection_exists) {
			/* If we have a selection, then delete it,
			 * so it's replaced by the break */
			textinput_textbox_delete(c, text_box, 0, 0);
		}

		new_text = textinput_textarea_insert_break(c, text_box,
				char_offset);
		if (!new_text)
			return true;

		/* place caret at start of new text box */
		text_box = new_text;
		char_offset = 0;

		reflow = true;
		break;

	case KEY_CUT_LINE:
	{
		struct box *start_box = textinput_line_start(text_box);
		struct box *end_box = textinput_line_end(text_box);

		/* Clear the selection, if one exists */
		if (selection_exists)
			selection_clear(&html->sel, false);

		textinput_textarea_cut(c, start_box, 0, end_box,
				end_box->length, false);

		text_box = start_box;
		char_offset = 0;
		reflow = true;
	}
		break;

	case KEY_PASTE:
	{
		char *buff = NULL;
		size_t buff_len;
		bool success;

		gui_get_clipboard(&buff, &buff_len);
		if (buff == NULL)
			return false;

		success = browser_window_paste_text(bw, buff, buff_len, true);
		free(buff);

		return success;
	}

	case KEY_CUT_SELECTION:
	{
		size_t start_idx, end_idx;
		struct box *start_box =
				selection_get_start(&html->sel, &start_idx);
		struct box *end_box = selection_get_end(&html->sel, &end_idx);

		if (start_box && end_box) {
			selection_clear(&html->sel, false);
			textinput_textarea_cut(c, start_box, start_idx,
					end_box, end_idx, true);
			text_box = start_box;
			char_offset = start_idx;
			reflow = true;
		}
	}
		break;

	case KEY_RIGHT:
		if (selection_exists) {
			/* In selection, move caret to end */
			text_box = selection_get_end(&html->sel, &char_offset);
		} else if (char_offset < text_box->length) {
			/* Within-box movement */
			char_offset = utf8_next(text_box->text,
					text_box->length, char_offset);
		} else {
			/* Between-box movement */
			if (!text_box->next)
				/* at end of text area: ignore */
				return true;

			text_box = text_box->next;
			if (text_box->type == BOX_BR)
				text_box = text_box->next;
			char_offset = 0;
		}
		break;

	case KEY_LEFT:
		if (selection_exists) {
			/* In selection, move caret to start */
			text_box = selection_get_start(&html->sel, &char_offset);
		} else if (char_offset > 0) {
			/* Within-box movement */
			char_offset = utf8_prev(text_box->text, char_offset);
		} else {
			/* Between-box movement */
			if (!text_box->prev)
				/* at start of text area: ignore */
				return true;

			text_box = text_box->prev;
			if (text_box->type == BOX_BR)
				text_box = text_box->prev;
			char_offset = text_box->length;
		}
		break;

	case KEY_UP:
		selection_clear(&html->sel, true);
		textinput_textarea_click(c, BROWSER_MOUSE_CLICK_1,
				textarea,
				box_x, box_y,
				text_box->x + pixel_offset,
				inline_container->y + text_box->y - 1);
		return true;

	case KEY_DOWN:
		selection_clear(&html->sel, true);
		textinput_textarea_click(c, BROWSER_MOUSE_CLICK_1,
				textarea,
				box_x, box_y,
				text_box->x + pixel_offset,
				inline_container->y + text_box->y +
				text_box->height + 1);
		return true;

	case KEY_LINE_START:
		text_box = textinput_line_start(text_box);
		char_offset = 0;
		break;

	case KEY_LINE_END:
		text_box = textinput_line_end(text_box);
		char_offset = text_box->length;
		break;

	case KEY_TEXT_START:
		assert(text_box->parent);

		/* place caret at start of first box */
		text_box = text_box->parent->children;
		char_offset = 0;
		break;

	case KEY_TEXT_END:
		assert(text_box->parent);

		/* place caret at end of last box */
		text_box = text_box->parent->last;
		char_offset = text_box->length;
		break;

	case KEY_WORD_LEFT:
	{
		bool start_of_word;
		/* if there is a selection, caret should stay at beginning */
		if (selection_exists)
			break;

		start_of_word = (char_offset <= 0 ||
				isspace(text_box->text[char_offset - 1]));

		while (!textinput_word_left(text_box->text,
				&char_offset, NULL)) {
			struct box *prev = NULL;

			assert(char_offset == 0);

			if (start_of_word) {
				/* find the preceding non-BR box */
				prev = text_box->prev;
				if (prev && prev->type == BOX_BR)
					prev = prev->prev;
			}

			if (!prev) {
				/* just stay at the start of this box */
				break;
			}

			assert(prev->type == BOX_TEXT);

			text_box = prev;
			char_offset = prev->length;
		}
	}
		break;

	case KEY_WORD_RIGHT:
	{
		bool in_word;
		/* if there is a selection, caret should move to the end */
		if (selection_exists) {
			text_box = selection_get_end(&html->sel, &char_offset);
			break;
		}

		in_word = (char_offset < text_box->length &&
				!isspace(text_box->text[char_offset]));

		while (!textinput_word_right(text_box->text, text_box->length,
				&char_offset, NULL)) {
			struct box *next = text_box->next;

			/* find the next non-BR box */
			if (next && next->type == BOX_BR)
				next = next->next;

			if (!next) {
				/* just stay at the end of this box */
				char_offset = text_box->length;
				break;
			}

			assert(next->type == BOX_TEXT);

			text_box = next;
			char_offset = 0;

			if (in_word && text_box->length > 0 &&
					!isspace(text_box->text[0])) {
				/* just stay at the start of this box */
				break;
			}
		}
	}
		break;

	case KEY_PAGE_UP:
	{
		int nlines = (textarea->height / text_box->height) - 1;

		while (nlines-- > 0)
			text_box = textinput_line_above(text_box);

		if (char_offset > text_box->length)
			char_offset = text_box->length;
	}
		break;

	case KEY_PAGE_DOWN:
	{
		int nlines = (textarea->height / text_box->height) - 1;

		while (nlines-- > 0)
			text_box = textinput_line_below(text_box);

		/* vague attempt to keep the caret at the same horizontal
		 * position, given that the code currently cannot support it
		 * being beyond the end of a line */
		if (char_offset > text_box->length)
			char_offset = text_box->length;
	}
		break;

	default:
		return false;
	}

	/*
	 box_dump(textarea, 0);
	 for (struct box *t = inline_container->children; t; t = t->next) {
		assert(t->type == BOX_TEXT);
		assert(t->text);
		assert(t->parent == inline_container);
		if (t->next) assert(t->next->prev == t);
		if (t->prev) assert(t->prev->next == t);
		if (!t->next) {
			assert(inline_container->last == t);
			break;
		}
		if (t->next->type == BOX_BR) {
			assert(t->next->next);
			t = t->next;
		}
	} */

	if (reflow)
		textinput_textarea_reflow(c, textarea, inline_container);

	if (text_box->length + SPACE_LEN(text_box) <= char_offset) {
		if (text_box->next && text_box->next->type == BOX_TEXT) {
			/* the text box has been split when reflowing and
			   the caret is in the second part */
			char_offset -= (text_box->length + SPACE_LEN(text_box));
			text_box = text_box->next;
			assert(text_box);
			assert(char_offset <= text_box->length);
			/* Scroll back to the left */
			if (textarea->scroll_x != NULL) {
				box_x += scrollbar_get_offset(
						textarea->scroll_x);
				scrollbar_set(textarea->scroll_x, 0, false);
			}
		} else {
			assert(!text_box->next ||
					(text_box->next &&
					text_box->next->type == BOX_BR));

			char_offset = text_box->length + SPACE_LEN(text_box);
		}
	}

	font_plot_style_from_css(text_box->style, &fstyle);

	nsfont.font_width(&fstyle, text_box->text, char_offset, &pixel_offset);

	selection_clear(&html->sel, true);

	textarea->gadget->caret_inline_container = inline_container;
	textarea->gadget->caret_text_box = text_box;
	textarea->gadget->caret_box_offset = char_offset;
	textarea->gadget->caret_pixel_offset = pixel_offset;

	box_x += scrollbar_get_offset(textarea->scroll_x);
	box_y += scrollbar_get_offset(textarea->scroll_y);
	scrolled = textinput_ensure_caret_visible(c, textarea);
	box_x -= scrollbar_get_offset(textarea->scroll_x);
	box_y -= scrollbar_get_offset(textarea->scroll_y);

	browser_window_place_caret(bw,
			box_x + inline_container->x + text_box->x +
			pixel_offset,
			box_y + inline_container->y + text_box->y,
			text_box->height,
			textinput_textarea_callback,
			textinput_textarea_paste_text,
			textinput_textarea_move_caret,
			textarea, c);

	if (scrolled || reflow)
		html__redraw_a_box(html, textarea);

	return true;
}


/**
 * Handle clicks in a text area by placing the caret.
 *
 * \param  c         html content where click occurred
 * \param  mouse     state of mouse buttons and modifier keys
 * \param  textarea  textarea box
 * \param  box_x     position of textarea in global document coordinates
 * \param  box_y     position of textarea in global document coordinates
 * \param  x         coordinate of click relative to textarea
 * \param  y         coordinate of click relative to textarea
 */
void textinput_textarea_click(struct content *c, browser_mouse_state mouse,
		struct box *textarea, int box_x, int box_y, int x, int y)
{
	/* A textarea is an INLINE_BLOCK containing a single
	 * INLINE_CONTAINER, which contains the text as runs of TEXT
	 * separated by BR. There is at least one TEXT. The first and
	 * last boxes are TEXT. Consecutive BR may not be present. These
	 * constraints are satisfied by using a 0-length TEXT for blank
	 * lines. */

	int char_offset = 0, pixel_offset = 0;
	struct box *inline_container = textarea->children;
	struct box *text_box;
	bool scrolled;
	html_content *html = (html_content *)c;

	text_box = textinput_textarea_get_position(textarea, x, y,
			&char_offset, &pixel_offset);

	textarea->gadget->caret_inline_container = inline_container;
	textarea->gadget->caret_text_box = text_box;
	textarea->gadget->caret_box_offset = char_offset;
	textarea->gadget->caret_pixel_offset = pixel_offset;

	box_x += scrollbar_get_offset(textarea->scroll_x);
	box_y += scrollbar_get_offset(textarea->scroll_y);
	scrolled = textinput_ensure_caret_visible(c, textarea);
	box_x -= scrollbar_get_offset(textarea->scroll_x);
	box_y -= scrollbar_get_offset(textarea->scroll_y);

	browser_window_place_caret(html->bw,
			box_x + inline_container->x + text_box->x +
			pixel_offset,
			box_y + inline_container->y + text_box->y,
			text_box->height,
			textinput_textarea_callback,
			textinput_textarea_paste_text,
			textinput_textarea_move_caret,
			textarea, c);

	if (scrolled)
		html__redraw_a_box(html, textarea);
}


/**
 * Paste a block of text into an input field at the caret position.
 *
 * \param  bw        browser window
 * \param  utf8      pointer to block of text
 * \param  utf8_len  length (bytes) of text block
 * \param  last      true iff this is the last chunk (update screen too)
 * \param  p1        pointer to input box
 * \param  p2        html content with the input box
 * \return true iff successful
 */

bool textinput_input_paste_text(struct browser_window *bw,
		const char *utf8, unsigned utf8_len, bool last,
		void *p1, void *p2)
{
	struct box *input = p1;
	struct content *c = p2;
	struct box *text_box = input->children->children;
	size_t box_offset = input->gadget->caret_box_offset;
	unsigned int nchars = utf8_length(input->gadget->value);
	const char *ep = utf8 + utf8_len;
	const char *p = utf8;
	bool success = true;
	bool update = last;

	/* keep adding chars until we've run out or would exceed
		the maximum length of the field (in which we silently
		ignore all others)
	*/
	while (p < ep && nchars < input->gadget->maxlength) {
		char buf[80 + 6];
		int nbytes = 0;

		/* how many more chars can we insert in one go? */
		while (p < ep && nbytes < 80 &&
				nchars < input->gadget->maxlength &&
				*p != '\n' && *p != '\r') {
			unsigned len = utf8_next(p, ep - p, 0);
			if (*p == ' ')
				nbytes += utf8_from_ucs4(160, &buf[nbytes]);
			else {
				memcpy(&buf[nbytes], p, len);
				nbytes += len;
			}

			p += len;
			nchars++;
		}

		if (!textinput_textbox_insert(c, text_box, box_offset,
				buf, nbytes)) {
			/* we still need to update the screen */
			update = true;
			success = false;
			break;
		}
		box_offset += nbytes;
		/* Keep caret_form_offset in sync -- textbox_insert uses this
		 * to determine where to insert into the gadget's value */
		input->gadget->caret_form_offset += nbytes;

		/* handle CR/LF and LF/CR terminations */
		if (*p == '\n') {
			p++;
			if (*p == '\r') p++;
		}
		else if (*p == '\r') {
			p++;
			if (*p == '\n') p++;
		}
	}

	if (update)
		textinput_input_update_display(c, input, box_offset,
				false, true);

	return success;
}


/**
 * Move caret to new position after reformatting
 *
 * \param  bw   browser window
 * \param  p1   pointer to text input box
 * \param  p2   html content with the input box
 * \return none
 */

void textinput_input_move_caret(struct browser_window *bw,
		void *p1, void *p2)
{
	struct box *input = (struct box *)p1;
	struct content *c = p2;
	struct box *text_box = input->children->children;
	unsigned int box_offset = input->gadget->caret_box_offset;
	int pixel_offset;
	int box_x, box_y;
	plot_font_style_t fstyle;

	font_plot_style_from_css(text_box->style, &fstyle);

	box_coords(input, &box_x, &box_y);

	nsfont.font_width(&fstyle, text_box->text, box_offset,
			&pixel_offset);

	browser_window_place_caret(bw,
			box_x + input->children->x +
					text_box->x + pixel_offset,
			box_y + input->children->y + text_box->y,
			text_box->height,
			textinput_input_callback,
			textinput_input_paste_text,
			textinput_input_move_caret,
			input, c);
}

/**
 * Key press callback for text or password input boxes.
 *
 * \param bw   The browser window containing the input box
 * \param key  The UCS4 character codepoint
 * \param p1   The input box
 * \param p2   The html content with the input box
 * \return     true if the keypress is dealt with, false otherwise.  It can
 *             return true even if it ran out of memory; this just means that
 *             it would have claimed it if it could.
 */
bool textinput_input_callback(struct browser_window *bw, uint32_t key,
		void *p1, void *p2)
{
	struct box *input = (struct box *)p1;
	struct content *c = p2;
	html_content *html = (html_content *)c;
	struct box *text_box = input->children->children;
	size_t box_offset = input->gadget->caret_box_offset;
	size_t end_offset;
	int box_x, box_y;
	struct form* form = input->gadget->form;
	bool changed = false;
	char utf8[6];
	unsigned int utf8_len;
	bool to_textarea = false;
	bool selection_exists = html->sel.defined;

	input->gadget->caret_form_offset =
			textinput_get_form_offset(input, text_box, box_offset);

	/* update the form offset */
	input->gadget->caret_form_offset =
			textinput_get_form_offset(input, text_box, box_offset);

	selection_get_end(&html->sel, &end_offset);

	box_coords(input, &box_x, &box_y);

	/* normal character insertion */
	if (!(key <= 0x001F || (0x007F <= key && key <= 0x009F))) {
		/* have we exceeded max length of input? */
		utf8_len = utf8_length(input->gadget->value);
		if (utf8_len >= input->gadget->maxlength)
			return true;

		utf8_len = utf8_from_ucs4(key, utf8);

		if (!textinput_textbox_insert(c, text_box, box_offset,
				utf8, utf8_len))
			return true;

		box_offset += utf8_len;

		changed = true;

	} else switch (key) {
	case KEY_DELETE_LEFT:
	{
		int prev_offset, new_offset;

		if (selection_exists) {
			textinput_textbox_delete(c, text_box, 0, 0);
		} else {
			/* Can't delete left from text box start */
			if (box_offset == 0)
				return true;

			prev_offset = box_offset;
			new_offset = utf8_prev(text_box->text, box_offset);

			if (textinput_textbox_delete(c, text_box, new_offset,
					prev_offset - new_offset))
				box_offset = new_offset;
		}

		changed = true;
	}
		break;

	case KEY_DELETE_RIGHT:
	{
		unsigned next_offset;

		if (selection_exists) {
			textinput_textbox_delete(c, text_box, 0, 0);
		} else {
			/* Can't delete right from text box end */
			if (box_offset >= text_box->length)
				return true;

			/* Go to the next valid UTF-8 character */
			next_offset = utf8_next(text_box->text,
					text_box->length, box_offset);

			textinput_textbox_delete(c, text_box, box_offset,
					next_offset - box_offset);
		}

		changed = true;
	}
		break;

	case KEY_TAB:
	{
		struct form_control *next_input;
		/* Find next text entry field that is actually
		 * displayed (i.e. has an associated box) */
		for (next_input = input->gadget->next;
				next_input &&
				((next_input->type != GADGET_TEXTBOX &&
				next_input->type != GADGET_TEXTAREA &&
				next_input->type != GADGET_PASSWORD) ||
				!next_input->box);
				next_input = next_input->next)
			;
		if (!next_input)
			return true;

		input = next_input->box;
		box_offset = 0;
		to_textarea = next_input->type == GADGET_TEXTAREA;
	}
		break;

	case KEY_NL:
	case KEY_CR:	/* Return/Enter hit */
		selection_clear(&html->sel, true);

		if (form)
			form_submit(content_get_url(c), bw, form, 0);
		return true;

	case KEY_SHIFT_TAB:
	{
		struct form_control *prev_input;
		/* Find previous text entry field that is actually
		 * displayed (i.e. has an associated box) */
		for (prev_input = input->gadget->prev;
				prev_input &&
				((prev_input->type != GADGET_TEXTBOX &&
				prev_input->type != GADGET_TEXTAREA &&
				prev_input->type != GADGET_PASSWORD) ||
				!prev_input->box);
				prev_input = prev_input->prev)
			;
		if (!prev_input)
			return true;

		input = prev_input->box;
		box_offset = 0;
		to_textarea = prev_input->type == GADGET_TEXTAREA;
	}
		break;

	case KEY_CUT_LINE:
		/* Clear the selection, if one exists */
		if (selection_exists)
			selection_clear(&html->sel, false);

		textinput_textarea_cut(c, text_box, 0, text_box,
				text_box->length, false);
		box_offset = 0;

		changed = true;
		break;

	case KEY_PASTE:
	{
		char *buff = NULL;
		size_t buff_len;
		bool success;

		gui_get_clipboard(&buff, &buff_len);
		if (buff == NULL)
			return false;

		success = browser_window_paste_text(bw, buff, buff_len, true);
		free(buff);

		return success;
	}

	case KEY_CUT_SELECTION:
	{
		size_t start_idx, end_idx;
		struct box *start_box =
				selection_get_start(&html->sel, &start_idx);
		struct box *end_box = selection_get_end(&html->sel, &end_idx);

		if (start_box && end_box) {
			selection_clear(&html->sel, false);
			textinput_textarea_cut(c, start_box, start_idx,
					end_box, end_idx, true);

			box_offset = start_idx;
			changed = true;
		}
	}
		break;

	case KEY_RIGHT:
		if (selection_exists) {
			box_offset = end_offset;
			break;
		}

		if (box_offset < text_box->length) {
			/* Go to the next valid UTF-8 character */
			box_offset = utf8_next(text_box->text,
					text_box->length, box_offset);
		}

		break;

	case KEY_LEFT:
		/* If there is a selection, caret should remain at start */
		if (selection_exists)
			break;

		/* Go to the previous valid UTF-8 character */
		box_offset = utf8_prev(text_box->text, box_offset);
		break;

	case KEY_LINE_START:
		box_offset = 0;
		break;

	case KEY_LINE_END:
		box_offset = text_box->length;
		break;

	case KEY_WORD_LEFT:
		/* If there is a selection, caret should remain at start */
		if (selection_exists)
			break;

		if (!textinput_word_left(text_box->text, &box_offset, NULL))
			box_offset = 0;

		break;

	case KEY_WORD_RIGHT:
		if (selection_exists) {
			box_offset = end_offset;
			break;
		}

		if (!textinput_word_right(text_box->text, text_box->length,
				&box_offset, NULL))
			box_offset = text_box->length;

		break;

	case KEY_DELETE_LINE_START:
		if (selection_exists)
			selection_clear(&html->sel, true);

		if (box_offset == 0)
			return true;

		textinput_textarea_cut(c, text_box, 0, text_box,
				box_offset, false);
		box_offset = 0;

		changed = true;
		break;

	case KEY_DELETE_LINE_END:
		if (selection_exists)
			selection_clear(&html->sel, true);

		if (box_offset >= text_box->length)
			return true;

		textinput_textarea_cut(c, text_box, box_offset,
				text_box, text_box->length, false);

		changed = true;
		break;

	default:
		return false;
	}

	selection_clear(&html->sel, true);
	textinput_input_update_display(c, input, box_offset,
			to_textarea, changed);

	return true;
}


/**
 * Handle clicks in a text or password input box by placing the caret.
 *
 * \param  bw     browser window where click occurred
 * \param  input  input box
 * \param  box_x  position of input in global document coordinates
 * \param  box_y  position of input in global document coordinates
 * \param  x      coordinate of click relative to input
 * \param  y      coordinate of click relative to input
 */
void textinput_input_click(struct content *c, struct box *input,
		int box_x, int box_y, int x, int y)
{
	size_t char_offset = 0;
	int pixel_offset = 0, dx = 0;
	struct box *text_box = input->children->children;
	plot_font_style_t fstyle;
	html_content *html = (html_content *)c;

	font_plot_style_from_css(text_box->style, &fstyle);

	nsfont.font_position_in_string(&fstyle, text_box->text,
			text_box->length, x - text_box->x,
			&char_offset, &pixel_offset);
	assert(char_offset <= text_box->length);

	/* Shift the text box horizontally to ensure that the
	 * caret position is visible, and ideally centred */
	text_box->x = 0;
	if ((input->width < text_box->width) &&
			(input->width / 2 < pixel_offset)) {
		dx = text_box->x;
		/* Move left so caret is centred */
		text_box->x = input->width / 2 - pixel_offset;
		/* Clamp, so text box's right hand edge coincides
		 * with the input's right hand edge */
		if (text_box->x < input->width - text_box->width)
			text_box->x = input->width - text_box->width;
		dx -= text_box->x;
	}
	input->gadget->caret_box_offset = char_offset;
	input->gadget->caret_form_offset =
			textinput_get_form_offset(input, text_box, char_offset);
	input->gadget->caret_pixel_offset = pixel_offset;

	browser_window_place_caret(html->bw,
			box_x + input->children->x +
					text_box->x + pixel_offset,
			box_y + input->children->y + text_box->y,
			text_box->height,
			textinput_input_callback,
			textinput_input_paste_text,
			textinput_input_move_caret,
			input, c);

	if (dx)
		html__redraw_a_box(html, input);
}


