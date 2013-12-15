/*
 * This file is part of LibCSS
 * Licensed under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 * Copyright 2009 John-Mark Bell <jmb@netsurf-browser.org>
 */

#include <string.h>

#include "select/computed.h"
#include "select/dispatch.h"
#include "select/propget.h"
#include "select/propset.h"
#include "utils/utils.h"

static css_error compute_absolute_color(css_computed_style *style,
		uint8_t (*get)(const css_computed_style *style,
				css_color *color),
		css_error (*set)(css_computed_style *style,
				uint8_t type, css_color color));
static css_error compute_border_colors(css_computed_style *style);

static css_error compute_absolute_border_width(css_computed_style *style,
		const css_hint_length *ex_size);
static css_error compute_absolute_border_side_width(css_computed_style *style,
		const css_hint_length *ex_size,
		uint8_t (*get)(const css_computed_style *style, 
				css_fixed *len, css_unit *unit),
		css_error (*set)(css_computed_style *style, uint8_t type,
				css_fixed len, css_unit unit));
static css_error compute_absolute_sides(css_computed_style *style,
		const css_hint_length *ex_size);
static css_error compute_absolute_clip(css_computed_style *style,
		const css_hint_length *ex_size);
static css_error compute_absolute_line_height(css_computed_style *style,
		const css_hint_length *ex_size);
static css_error compute_absolute_margins(css_computed_style *style,
		const css_hint_length *ex_size);
static css_error compute_absolute_padding(css_computed_style *style,
		const css_hint_length *ex_size);
static css_error compute_absolute_vertical_align(css_computed_style *style,
		const css_hint_length *ex_size);
static css_error compute_absolute_length(css_computed_style *style,
		const css_hint_length *ex_size,
		uint8_t (*get)(const css_computed_style *style, 
				css_fixed *len, css_unit *unit),
		css_error (*set)(css_computed_style *style, uint8_t type,
				css_fixed len, css_unit unit));
static css_error compute_absolute_length_auto(css_computed_style *style,
		const css_hint_length *ex_size,
		uint8_t (*get)(const css_computed_style *style, 
				css_fixed *len, css_unit *unit),
		css_error (*set)(css_computed_style *style, uint8_t type,
				css_fixed len, css_unit unit));
static css_error compute_absolute_length_none(css_computed_style *style,
		const css_hint_length *ex_size,
		uint8_t (*get)(const css_computed_style *style, 
				css_fixed *len, css_unit *unit),
		css_error (*set)(css_computed_style *style, uint8_t type,
				css_fixed len, css_unit unit));
static css_error compute_absolute_length_normal(css_computed_style *style,
		const css_hint_length *ex_size,
		uint8_t (*get)(const css_computed_style *style, 
				css_fixed *len, css_unit *unit),
		css_error (*set)(css_computed_style *style, uint8_t type,
				css_fixed len, css_unit unit));
static css_error compute_absolute_length_pair(css_computed_style *style,
		const css_hint_length *ex_size,
		uint8_t (*get)(const css_computed_style *style, 
				css_fixed *len1, css_unit *unit1,
				css_fixed *len2, css_unit *unit2),
		css_error (*set)(css_computed_style *style, uint8_t type,
				css_fixed len1, css_unit unit1,
				css_fixed len2, css_unit unit2));


/**
 * Create a computed style
 *
 * \param alloc   Memory (de)allocation function
 * \param pw      Pointer to client-specific data
 * \param result  Pointer to location to receive result
 * \return CSS_OK on success,
 *         CSS_NOMEM on memory exhaustion,
 *         CSS_BADPARM on bad parameters.
 */
css_error css_computed_style_create(css_allocator_fn alloc, void *pw,
		css_computed_style **result)
{
	css_computed_style *s;

	if (alloc == NULL || result == NULL)
		return CSS_BADPARM;

	s = alloc(NULL, sizeof(css_computed_style), pw);
	if (s == NULL)
		return CSS_NOMEM;

	memset(s, 0, sizeof(css_computed_style));

	s->alloc = alloc;
	s->pw = pw;

	*result = s;

	return CSS_OK;
}

/**
 * Destroy a computed style
 *
 * \param style  Style to destroy
 * \return CSS_OK on success, appropriate error otherwise
 */
css_error css_computed_style_destroy(css_computed_style *style)
{
	if (style == NULL)
		return CSS_BADPARM;

	if (style->uncommon != NULL) {
		if (style->uncommon->counter_increment != NULL) {
			css_computed_counter *c;

			for (c = style->uncommon->counter_increment; 
					c->name != NULL; c++) {
				lwc_string_unref(c->name);
			}

			style->alloc(style->uncommon->counter_increment, 0,
					style->pw);
		}

		if (style->uncommon->counter_reset != NULL) {
			css_computed_counter *c;

			for (c = style->uncommon->counter_reset; 
					c->name != NULL; c++) {
				lwc_string_unref(c->name);
			}

			style->alloc(style->uncommon->counter_reset, 0,
					style->pw);
		}
	
		if (style->uncommon->cursor != NULL) {
			lwc_string **s;

			for (s = style->uncommon->cursor; *s != NULL; s++) {
				lwc_string_unref(*s);
			}

			style->alloc(style->uncommon->cursor, 0, style->pw);
		}

		if (style->uncommon->content != NULL) {
			css_computed_content_item *c;

			for (c = style->uncommon->content; 
					c->type != CSS_COMPUTED_CONTENT_NONE;
					c++) {
				switch (c->type) {
				case CSS_COMPUTED_CONTENT_STRING:
					lwc_string_unref(c->data.string);
					break;
				case CSS_COMPUTED_CONTENT_URI:
					lwc_string_unref(c->data.uri);
					break;
				case CSS_COMPUTED_CONTENT_ATTR:
					lwc_string_unref(c->data.attr);
					break;
				case CSS_COMPUTED_CONTENT_COUNTER:
					lwc_string_unref(c->data.counter.name);
					break;
				case CSS_COMPUTED_CONTENT_COUNTERS:
					lwc_string_unref(c->data.counters.name);
					lwc_string_unref(c->data.counters.sep);
					break;
				default:
					break;
				}
			}

			style->alloc(style->uncommon->content, 0, style->pw);
		}

		style->alloc(style->uncommon, 0, style->pw);
	}

	if (style->page != NULL) {
		style->alloc(style->page, 0, style->pw);
	}

	if (style->aural != NULL) {
		style->alloc(style->aural, 0, style->pw);
	}

	if (style->font_family != NULL) {
		lwc_string **s;

		for (s = style->font_family; *s != NULL; s++) {
			lwc_string_unref(*s);
		}

		style->alloc(style->font_family, 0, style->pw);
	}

	if (style->quotes != NULL) {
		lwc_string **s;

		for (s = style->quotes; *s != NULL; s++) {
			lwc_string_unref(*s);
		}

		style->alloc(style->quotes, 0, style->pw);
	}

	if (style->list_style_image != NULL)
		lwc_string_unref(style->list_style_image);

	if (style->background_image != NULL)
		lwc_string_unref(style->background_image);

	style->alloc(style, 0, style->pw);

	return CSS_OK;
}

/**
 * Populate a blank computed style with Initial values
 *
 * \param style    Computed style to populate
 * \param handler  Dispatch table of handler functions
 * \param pw       Client-specific private data for handler functions
 * \return CSS_OK on success.
 */
css_error css_computed_style_initialise(css_computed_style *style,
		css_select_handler *handler, void *pw)
{
	css_select_state state;
	uint32_t i;
	css_error error;

	if (style == NULL)
		return CSS_BADPARM;

	state.node = NULL;
	state.media = CSS_MEDIA_ALL;
	state.results = NULL;
	state.computed = style;
	state.handler = handler;
	state.pw = pw;

	for (i = 0; i < CSS_N_PROPERTIES; i++) {
		/* No need to initialise anything other than the normal
		 * properties -- the others are handled by the accessors */
		if (prop_dispatch[i].inherited == false &&
				prop_dispatch[i].group == GROUP_NORMAL) {
			error = prop_dispatch[i].initial(&state);
			if (error != CSS_OK)
				return error;
		}
	}

	return CSS_OK;
}

/**
 * Compose two computed styles
 *
 * \param parent             Parent style
 * \param child              Child style
 * \param compute_font_size  Function to compute an absolute font size
 * \param pw                 Client data for compute_font_size
 * \param result             Pointer to style to compose into
 * \return CSS_OK on success, appropriate error otherwise.
 *
 * \pre \a parent is a fully composed style (thus has no inherited properties)
 *
 * \note \a child and \a result may point at the same object
 */
css_error css_computed_style_compose(const css_computed_style *parent,
		const css_computed_style *child,
		css_error (*compute_font_size)(void *pw,
			const css_hint *parent, css_hint *size),
		void *pw,
		css_computed_style *result)
{
	css_error error = CSS_OK;
	size_t i;

	/* Iterate through the properties */
	for (i = 0; i < CSS_N_PROPERTIES; i++) {
		/* Skip any in extension blocks if the block does not exist */	
		if (prop_dispatch[i].group == GROUP_UNCOMMON &&
				parent->uncommon == NULL && 
				child->uncommon == NULL)
			continue;

		if (prop_dispatch[i].group == GROUP_PAGE &&
				parent->page == NULL && child->page == NULL)
			continue;

		if (prop_dispatch[i].group == GROUP_AURAL &&
				parent->aural == NULL && child->aural == NULL)
			continue;

		/* Compose the property */
		error = prop_dispatch[i].compose(parent, child, result);
		if (error != CSS_OK)
			break;
	}

	/* Finally, compute absolute values for everything */
	return css__compute_absolute_values(parent, result, compute_font_size, pw);
}

/******************************************************************************
 * Property accessors                                                         *
 ******************************************************************************/


#define CSS_LETTER_SPACING_INDEX 0
#define CSS_LETTER_SPACING_SHIFT 2
#define CSS_LETTER_SPACING_MASK  0xfc
uint8_t css_computed_letter_spacing(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	if (style->uncommon != NULL) {
		uint8_t bits = style->uncommon->bits[CSS_LETTER_SPACING_INDEX];
		bits &= CSS_LETTER_SPACING_MASK;
		bits >>= CSS_LETTER_SPACING_SHIFT;

		/* 6bits: uuuutt : unit | type */

		if ((bits & 3) == CSS_LETTER_SPACING_SET) {
			*length = style->uncommon->letter_spacing;
			*unit = (css_unit) (bits >> 2);
		}

		return (bits & 3);
	}

	return CSS_LETTER_SPACING_NORMAL;
}
#undef CSS_LETTER_SPACING_MASK
#undef CSS_LETTER_SPACING_SHIFT
#undef CSS_LETTER_SPACING_INDEX

#define CSS_OUTLINE_COLOR_INDEX 0
#define CSS_OUTLINE_COLOR_SHIFT 0
#define CSS_OUTLINE_COLOR_MASK  0x3
uint8_t css_computed_outline_color(
		const css_computed_style *style, css_color *color)
{
	if (style->uncommon != NULL) {
		uint8_t bits = style->uncommon->bits[CSS_OUTLINE_COLOR_INDEX];
		bits &= CSS_OUTLINE_COLOR_MASK;
		bits >>= CSS_OUTLINE_COLOR_SHIFT;

		/* 2bits: tt : type */

		if ((bits & 3) == CSS_OUTLINE_COLOR_COLOR) {
			*color = style->uncommon->outline_color;
		}

		return (bits & 3);
	}

	return CSS_OUTLINE_COLOR_INVERT;
}
#undef CSS_OUTLINE_COLOR_MASK
#undef CSS_OUTLINE_COLOR_SHIFT
#undef CSS_OUTLINE_COLOR_INDEX

#define CSS_OUTLINE_WIDTH_INDEX 1
#define CSS_OUTLINE_WIDTH_SHIFT 1
#define CSS_OUTLINE_WIDTH_MASK  0xfe
uint8_t css_computed_outline_width(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	if (style->uncommon != NULL) {
		uint8_t bits = style->uncommon->bits[CSS_OUTLINE_WIDTH_INDEX];
		bits &= CSS_OUTLINE_WIDTH_MASK;
		bits >>= CSS_OUTLINE_WIDTH_SHIFT;

		/* 7bits: uuuuttt : unit | type */

		if ((bits & 7) == CSS_OUTLINE_WIDTH_WIDTH) {
			*length = style->uncommon->outline_width;
			*unit = (css_unit) (bits >> 3);
		}

		return (bits & 7);
	}

	*length = INTTOFIX(2);
	*unit = CSS_UNIT_PX;

	return CSS_OUTLINE_WIDTH_WIDTH;
}
#undef CSS_OUTLINE_WIDTH_MASK
#undef CSS_OUTLINE_WIDTH_SHIFT
#undef CSS_OUTLINE_WIDTH_INDEX

#define CSS_BORDER_SPACING_INDEX 1
#define CSS_BORDER_SPACING_SHIFT 0
#define CSS_BORDER_SPACING_MASK  0x1
#define CSS_BORDER_SPACING_INDEX1 2
#define CSS_BORDER_SPACING_SHIFT1 0
#define CSS_BORDER_SPACING_MASK1 0xff
uint8_t css_computed_border_spacing(
		const css_computed_style *style, 
		css_fixed *hlength, css_unit *hunit,
		css_fixed *vlength, css_unit *vunit)
{
	if (style->uncommon != NULL) {
		uint8_t bits = style->uncommon->bits[CSS_BORDER_SPACING_INDEX];
		bits &= CSS_BORDER_SPACING_MASK;
		bits >>= CSS_BORDER_SPACING_SHIFT;

		/* 1 bit: type */
		if (bits == CSS_BORDER_SPACING_SET) {
			uint8_t bits1 = 
				style->uncommon->bits[CSS_BORDER_SPACING_INDEX1];
			bits1 &= CSS_BORDER_SPACING_MASK1;
			bits1 >>= CSS_BORDER_SPACING_SHIFT1;

			/* 8bits: hhhhvvvv : hunit | vunit */

			*hlength = style->uncommon->border_spacing[0];
			*hunit = (css_unit) (bits1 >> 4);

			*vlength = style->uncommon->border_spacing[1];
			*vunit = (css_unit) (bits1 & 0xf);
		}

		return bits;
	} else {
		*hlength = *vlength = 0;
		*hunit = *vunit = CSS_UNIT_PX;
	}

	return CSS_BORDER_SPACING_SET;
}
#undef CSS_BORDER_SPACING_MASK1
#undef CSS_BORDER_SPACING_SHIFT1
#undef CSS_BORDER_SPACING_INDEX1
#undef CSS_BORDER_SPACING_MASK
#undef CSS_BORDER_SPACING_SHIFT
#undef CSS_BORDER_SPACING_INDEX

#define CSS_WORD_SPACING_INDEX 3
#define CSS_WORD_SPACING_SHIFT 2
#define CSS_WORD_SPACING_MASK  0xfc
uint8_t css_computed_word_spacing(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	if (style->uncommon != NULL) {
		uint8_t bits = style->uncommon->bits[CSS_WORD_SPACING_INDEX];
		bits &= CSS_WORD_SPACING_MASK;
		bits >>= CSS_WORD_SPACING_SHIFT;

		/* 6bits: uuuutt : unit | type */

		if ((bits & 3) == CSS_WORD_SPACING_SET) {
			*length = style->uncommon->word_spacing;
			*unit = (css_unit) (bits >> 2);
		}

		return (bits & 3);
	}

	return CSS_WORD_SPACING_NORMAL;
}
#undef CSS_WORD_SPACING_MASK
#undef CSS_WORD_SPACING_SHIFT
#undef CSS_WORD_SPACING_INDEX

#define CSS_COUNTER_INCREMENT_INDEX 3
#define CSS_COUNTER_INCREMENT_SHIFT 1
#define CSS_COUNTER_INCREMENT_MASK  0x2
uint8_t css_computed_counter_increment(
		const css_computed_style *style, 
		const css_computed_counter **counters)
{
	if (style->uncommon != NULL) {
		uint8_t bits = 
			style->uncommon->bits[CSS_COUNTER_INCREMENT_INDEX];
		bits &= CSS_COUNTER_INCREMENT_MASK;
		bits >>= CSS_COUNTER_INCREMENT_SHIFT;

		/* 1bit: type */
		*counters = style->uncommon->counter_increment;

		return bits;
	}

	return CSS_COUNTER_INCREMENT_NONE;
}
#undef CSS_COUNTER_INCREMENT_MASK
#undef CSS_COUNTER_INCREMENT_SHIFT
#undef CSS_COUNTER_INCREMENT_INDEX

#define CSS_COUNTER_RESET_INDEX 3
#define CSS_COUNTER_RESET_SHIFT 0
#define CSS_COUNTER_RESET_MASK  0x1
uint8_t css_computed_counter_reset(
		const css_computed_style *style, 
		const css_computed_counter **counters)
{
	if (style->uncommon != NULL) {
		uint8_t bits = style->uncommon->bits[CSS_COUNTER_RESET_INDEX];
		bits &= CSS_COUNTER_RESET_MASK;
		bits >>= CSS_COUNTER_RESET_SHIFT;

		/* 1bit: type */
		*counters = style->uncommon->counter_reset;

		return bits;
	}

	return CSS_COUNTER_RESET_NONE;
}
#undef CSS_COUNTER_RESET_MASK
#undef CSS_COUNTER_RESET_SHIFT
#undef CSS_COUNTER_RESET_INDEX

#define CSS_CURSOR_INDEX 4
#define CSS_CURSOR_SHIFT 3
#define CSS_CURSOR_MASK  0xf8
uint8_t css_computed_cursor(
		const css_computed_style *style, 
		lwc_string ***urls)
{
	if (style->uncommon != NULL) {
		uint8_t bits = style->uncommon->bits[CSS_CURSOR_INDEX];
		bits &= CSS_CURSOR_MASK;
		bits >>= CSS_CURSOR_SHIFT;

		/* 5bits: type */
		*urls = style->uncommon->cursor;

		return bits;
	}

	return CSS_CURSOR_AUTO;
}
#undef CSS_CURSOR_MASK
#undef CSS_CURSOR_SHIFT
#undef CSS_CURSOR_INDEX

#define CSS_CLIP_INDEX 7
#define CSS_CLIP_SHIFT 2
#define CSS_CLIP_MASK  0xfc
#define CSS_CLIP_INDEX1 5
#define CSS_CLIP_SHIFT1 0
#define CSS_CLIP_MASK1 0xff
#define CSS_CLIP_INDEX2 6
#define CSS_CLIP_SHIFT2 0
#define CSS_CLIP_MASK2 0xff
uint8_t css_computed_clip(
		const css_computed_style *style, 
		css_computed_clip_rect *rect)
{
	if (style->uncommon != NULL) {
		uint8_t bits = style->uncommon->bits[CSS_CLIP_INDEX];
		bits &= CSS_CLIP_MASK;
		bits >>= CSS_CLIP_SHIFT;

		/* 6bits: trblyy : top | right | bottom | left | type */
		if ((bits & 0x3) == CSS_CLIP_RECT) {
			uint8_t bits1; 

			rect->left_auto = (bits & 0x4);
			rect->bottom_auto = (bits & 0x8);
			rect->right_auto = (bits & 0x10);
			rect->top_auto = (bits & 0x20);

			if (rect->top_auto == false ||
					rect->right_auto == false) {
				/* 8bits: ttttrrrr : top | right */
				bits1 = style->uncommon->bits[CSS_CLIP_INDEX1];
				bits1 &= CSS_CLIP_MASK1;
				bits1 >>= CSS_CLIP_SHIFT1;
			} else {
				bits1 = 0;
			}

			rect->top = style->uncommon->clip[0];
			rect->tunit = (css_unit) (bits1 >> 4);

			rect->right = style->uncommon->clip[1];
			rect->runit = (css_unit) (bits1 & 0xf);

			if (rect->bottom_auto == false ||
					rect->left_auto == false) {
				/* 8bits: bbbbllll : bottom | left */
				bits1 = style->uncommon->bits[CSS_CLIP_INDEX2];
				bits1 &= CSS_CLIP_MASK2;
				bits1 >>= CSS_CLIP_SHIFT2;
			} else {
				bits1 = 0;
			}

			rect->bottom = style->uncommon->clip[2];
			rect->bunit = (css_unit) (bits1 >> 4);

			rect->left = style->uncommon->clip[3];
			rect->lunit = (css_unit) (bits1 & 0xf);
		}

		return (bits & 0x3);
	}

	return CSS_CLIP_AUTO;
}
#undef CSS_CLIP_MASK2
#undef CSS_CLIP_SHIFT2
#undef CSS_CLIP_INDEX2
#undef CSS_CLIP_MASK1
#undef CSS_CLIP_SHIFT1
#undef CSS_CLIP_INDEX1
#undef CSS_CLIP_MASK
#undef CSS_CLIP_SHIFT
#undef CSS_CLIP_INDEX

#define CSS_CONTENT_INDEX 7
#define CSS_CONTENT_SHIFT 0
#define CSS_CONTENT_MASK  0x3
uint8_t css_computed_content(
		const css_computed_style *style, 
		const css_computed_content_item **content)
{
	if (style->uncommon != NULL) {
		uint8_t bits = style->uncommon->bits[CSS_CONTENT_INDEX];
		bits &= CSS_CONTENT_MASK;
		bits >>= CSS_CONTENT_SHIFT;

		/* 2bits: type */
		*content = style->uncommon->content;

		return bits;
	}

	return CSS_CONTENT_NORMAL;
}
#undef CSS_CONTENT_MASK
#undef CSS_CONTENT_SHIFT
#undef CSS_CONTENT_INDEX

#define CSS_VERTICAL_ALIGN_INDEX 0
#define CSS_VERTICAL_ALIGN_SHIFT 0
#define CSS_VERTICAL_ALIGN_MASK  0xff
uint8_t css_computed_vertical_align(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	uint8_t bits = style->bits[CSS_VERTICAL_ALIGN_INDEX];
	bits &= CSS_VERTICAL_ALIGN_MASK;
	bits >>= CSS_VERTICAL_ALIGN_SHIFT;

	/* 8bits: uuuutttt : units | type */
	if ((bits & 0xf) == CSS_VERTICAL_ALIGN_SET) {
		*length = style->vertical_align;
		*unit = (css_unit) (bits >> 4);
	}

	return (bits & 0xf);
}
#undef CSS_VERTICAL_ALIGN_MASK
#undef CSS_VERTICAL_ALIGN_SHIFT
#undef CSS_VERTICAL_ALIGN_INDEX

#define CSS_FONT_SIZE_INDEX 1
#define CSS_FONT_SIZE_SHIFT 0
#define CSS_FONT_SIZE_MASK  0xff
uint8_t css_computed_font_size(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	uint8_t bits = style->bits[CSS_FONT_SIZE_INDEX];
	bits &= CSS_FONT_SIZE_MASK;
	bits >>= CSS_FONT_SIZE_SHIFT;

	/* 8bits: uuuutttt : units | type */
	if ((bits & 0xf) == CSS_FONT_SIZE_DIMENSION) {
		*length = style->font_size;
		*unit = (css_unit) (bits >> 4);
	}

	return (bits & 0xf);
}
#undef CSS_FONT_SIZE_MASK
#undef CSS_FONT_SIZE_SHIFT
#undef CSS_FONT_SIZE_INDEX

#define CSS_BORDER_TOP_WIDTH_INDEX 2
#define CSS_BORDER_TOP_WIDTH_SHIFT 1
#define CSS_BORDER_TOP_WIDTH_MASK  0xfe
uint8_t css_computed_border_top_width(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	uint8_t bits = style->bits[CSS_BORDER_TOP_WIDTH_INDEX];
	bits &= CSS_BORDER_TOP_WIDTH_MASK;
	bits >>= CSS_BORDER_TOP_WIDTH_SHIFT;

	/* 7bits: uuuuttt : units | type */
	if ((bits & 0x7) == CSS_BORDER_WIDTH_WIDTH) {
		*length = style->border_width[0];
		*unit = (css_unit) (bits >> 3);
	}

	return (bits & 0x7);
}
#undef CSS_BORDER_TOP_WIDTH_MASK
#undef CSS_BORDER_TOP_WIDTH_SHIFT
#undef CSS_BORDER_TOP_WIDTH_INDEX

#define CSS_BORDER_RIGHT_WIDTH_INDEX 3
#define CSS_BORDER_RIGHT_WIDTH_SHIFT 1
#define CSS_BORDER_RIGHT_WIDTH_MASK  0xfe
uint8_t css_computed_border_right_width(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	uint8_t bits = style->bits[CSS_BORDER_RIGHT_WIDTH_INDEX];
	bits &= CSS_BORDER_RIGHT_WIDTH_MASK;
	bits >>= CSS_BORDER_RIGHT_WIDTH_SHIFT;

	/* 7bits: uuuuttt : units | type */
	if ((bits & 0x7) == CSS_BORDER_WIDTH_WIDTH) {
		*length = style->border_width[1];
		*unit = (css_unit) (bits >> 3);
	}

	return (bits & 0x7);
}
#undef CSS_BORDER_RIGHT_WIDTH_MASK
#undef CSS_BORDER_RIGHT_WIDTH_SHIFT
#undef CSS_BORDER_RIGHT_WIDTH_INDEX

#define CSS_BORDER_BOTTOM_WIDTH_INDEX 4
#define CSS_BORDER_BOTTOM_WIDTH_SHIFT 1
#define CSS_BORDER_BOTTOM_WIDTH_MASK  0xfe
uint8_t css_computed_border_bottom_width(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	uint8_t bits = style->bits[CSS_BORDER_BOTTOM_WIDTH_INDEX];
	bits &= CSS_BORDER_BOTTOM_WIDTH_MASK;
	bits >>= CSS_BORDER_BOTTOM_WIDTH_SHIFT;

	/* 7bits: uuuuttt : units | type */
	if ((bits & 0x7) == CSS_BORDER_WIDTH_WIDTH) {
		*length = style->border_width[2];
		*unit = (css_unit) (bits >> 3);
	}

	return (bits & 0x7);
}
#undef CSS_BORDER_BOTTOM_WIDTH_MASK
#undef CSS_BORDER_BOTTOM_WIDTH_SHIFT
#undef CSS_BORDER_BOTTOM_WIDTH_INDEX

#define CSS_BORDER_LEFT_WIDTH_INDEX 5
#define CSS_BORDER_LEFT_WIDTH_SHIFT 1
#define CSS_BORDER_LEFT_WIDTH_MASK  0xfe
uint8_t css_computed_border_left_width(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	uint8_t bits = style->bits[CSS_BORDER_LEFT_WIDTH_INDEX];
	bits &= CSS_BORDER_LEFT_WIDTH_MASK;
	bits >>= CSS_BORDER_LEFT_WIDTH_SHIFT;

	/* 7bits: uuuuttt : units | type */
	if ((bits & 0x7) == CSS_BORDER_WIDTH_WIDTH) {
		*length = style->border_width[3];
		*unit = (css_unit) (bits >> 3);
	}

	return (bits & 0x7);
}
#undef CSS_BORDER_LEFT_WIDTH_MASK
#undef CSS_BORDER_LEFT_WIDTH_SHIFT
#undef CSS_BORDER_LEFT_WIDTH_INDEX

#define CSS_BACKGROUND_IMAGE_INDEX 2
#define CSS_BACKGROUND_IMAGE_SHIFT 0
#define CSS_BACKGROUND_IMAGE_MASK  0x1
uint8_t css_computed_background_image(
		const css_computed_style *style, 
		lwc_string **url)
{
	uint8_t bits = style->bits[CSS_BACKGROUND_IMAGE_INDEX];
	bits &= CSS_BACKGROUND_IMAGE_MASK;
	bits >>= CSS_BACKGROUND_IMAGE_SHIFT;

	/* 1bit: type */
	*url = style->background_image;

	return bits;
}
#undef CSS_BACKGROUND_IMAGE_MASK
#undef CSS_BACKGROUND_IMAGE_SHIFT
#undef CSS_BACKGROUND_IMAGE_INDEX

#define CSS_COLOR_INDEX 3
#define CSS_COLOR_SHIFT 0
#define CSS_COLOR_MASK  0x1
uint8_t css_computed_color(
		const css_computed_style *style, 
		css_color *color)
{
	uint8_t bits = style->bits[CSS_COLOR_INDEX];
	bits &= CSS_COLOR_MASK;
	bits >>= CSS_COLOR_SHIFT;

	/* 1bit: type */
	*color = style->color;

	return bits;
}
#undef CSS_COLOR_MASK
#undef CSS_COLOR_SHIFT
#undef CSS_COLOR_INDEX

#define CSS_LIST_STYLE_IMAGE_INDEX 4
#define CSS_LIST_STYLE_IMAGE_SHIFT 0
#define CSS_LIST_STYLE_IMAGE_MASK  0x1
uint8_t css_computed_list_style_image(
		const css_computed_style *style, 
		lwc_string **url)
{
	uint8_t bits = style->bits[CSS_LIST_STYLE_IMAGE_INDEX];
	bits &= CSS_LIST_STYLE_IMAGE_MASK;
	bits >>= CSS_LIST_STYLE_IMAGE_SHIFT;

	/* 1bit: type */
	*url = style->list_style_image;

	return bits;
}
#undef CSS_LIST_STYLE_IMAGE_MASK
#undef CSS_LIST_STYLE_IMAGE_SHIFT
#undef CSS_LIST_STYLE_IMAGE_INDEX

#define CSS_QUOTES_INDEX 5
#define CSS_QUOTES_SHIFT 0
#define CSS_QUOTES_MASK  0x1
uint8_t css_computed_quotes(
		const css_computed_style *style, 
		lwc_string ***quotes)
{
	uint8_t bits = style->bits[CSS_QUOTES_INDEX];
	bits &= CSS_QUOTES_MASK;
	bits >>= CSS_QUOTES_SHIFT;

	/* 1bit: type */
	*quotes = style->quotes;

	return bits;
}
#undef CSS_QUOTES_MASK
#undef CSS_QUOTES_SHIFT
#undef CSS_QUOTES_INDEX

#define CSS_TOP_INDEX 6
#define CSS_TOP_SHIFT 2
#define CSS_TOP_MASK  0xfc
#define CSS_RIGHT_INDEX 7
#define CSS_RIGHT_SHIFT 2
#define CSS_RIGHT_MASK  0xfc
#define CSS_BOTTOM_INDEX 8
#define CSS_BOTTOM_SHIFT 2
#define CSS_BOTTOM_MASK  0xfc
#define CSS_LEFT_INDEX 9
#define CSS_LEFT_SHIFT 2
#define CSS_LEFT_MASK  0xfc
uint8_t css_computed_top(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	uint8_t bits = style->bits[CSS_TOP_INDEX];
	bits &= CSS_TOP_MASK;
	bits >>= CSS_TOP_SHIFT;

	/* Fix up, based on computed position */
	if (css_computed_position(style) == CSS_POSITION_STATIC) {
		/* Static -> auto */
		bits = CSS_TOP_AUTO;
	} else if (css_computed_position(style) == CSS_POSITION_RELATIVE) {
		/* Relative -> follow $9.4.3 */
		uint8_t bottom = style->bits[CSS_BOTTOM_INDEX];
		bottom &= CSS_BOTTOM_MASK;
		bottom >>= CSS_BOTTOM_SHIFT;

		if ((bits & 0x3) == CSS_TOP_AUTO && 
				(bottom & 0x3) == CSS_BOTTOM_AUTO) {
			/* Both auto => 0px */
			*length = 0;
			*unit = CSS_UNIT_PX;
		} else if ((bits & 0x3) == CSS_TOP_AUTO) {
			/* Top is auto => -bottom */
			*length = -style->bottom;
			*unit = (css_unit) (bottom >> 2);
		} else {
			*length = style->top;
			*unit = (css_unit) (bits >> 2);
		}

		bits = CSS_TOP_SET;
	} else if ((bits & 0x3) == CSS_TOP_SET) {
		*length = style->top;
		*unit = (css_unit) (bits >> 2);
	}

	/* 6bits: uuuutt : units | type */
	return (bits & 0x3);
}

uint8_t css_computed_right(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	uint8_t bits = style->bits[CSS_RIGHT_INDEX];
	bits &= CSS_RIGHT_MASK;
	bits >>= CSS_RIGHT_SHIFT;

	/* Fix up, based on computed position */
	if (css_computed_position(style) == CSS_POSITION_STATIC) {
		/* Static -> auto */
		bits = CSS_RIGHT_AUTO;
	} else if (css_computed_position(style) == CSS_POSITION_RELATIVE) {
		/* Relative -> follow $9.4.3 */
		uint8_t left = style->bits[CSS_LEFT_INDEX];
		left &= CSS_LEFT_MASK;
		left >>= CSS_LEFT_SHIFT;

		if ((bits & 0x3) == CSS_RIGHT_AUTO && 
				(left & 0x3) == CSS_LEFT_AUTO) {
			/* Both auto => 0px */
			*length = 0;
			*unit = CSS_UNIT_PX;
		} else if ((bits & 0x3) == CSS_RIGHT_AUTO) {
			/* Right is auto => -left */
			*length = -style->left;
			*unit = (css_unit) (left >> 2);
		} else {
			/** \todo Consider containing block's direction 
			 * if overconstrained */
			*length = style->right;
			*unit = (css_unit) (bits >> 2);
		}

		bits = CSS_RIGHT_SET;
	} else if ((bits & 0x3) == CSS_RIGHT_SET) {
		*length = style->right;
		*unit = (css_unit) (bits >> 2);
	}

	/* 6bits: uuuutt : units | type */
	return (bits & 0x3);
}

uint8_t css_computed_bottom(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	uint8_t bits = style->bits[CSS_BOTTOM_INDEX];
	bits &= CSS_BOTTOM_MASK;
	bits >>= CSS_BOTTOM_SHIFT;

	/* Fix up, based on computed position */
	if (css_computed_position(style) == CSS_POSITION_STATIC) {
		/* Static -> auto */
		bits = CSS_BOTTOM_AUTO;
	} else if (css_computed_position(style) == CSS_POSITION_RELATIVE) {
		/* Relative -> follow $9.4.3 */
		uint8_t top = style->bits[CSS_TOP_INDEX];
		top &= CSS_TOP_MASK;
		top >>= CSS_TOP_SHIFT;

		if ((bits & 0x3) == CSS_BOTTOM_AUTO &&
				(top & 0x3) == CSS_TOP_AUTO) {
			/* Both auto => 0px */
			*length = 0;
			*unit = CSS_UNIT_PX;
		} else if ((bits & 0x3) == CSS_BOTTOM_AUTO || 
				(top & 0x3) != CSS_TOP_AUTO) {
			/* Bottom is auto or top is not auto => -top */
			*length = -style->top;
			*unit = (css_unit) (top >> 2);
		} else {
			*length = style->bottom;
			*unit = (css_unit) (bits >> 2);
		}

		bits = CSS_BOTTOM_SET;
	} else if ((bits & 0x3) == CSS_BOTTOM_SET) {
		*length = style->bottom;
		*unit = (css_unit) (bits >> 2);
	}

	/* 6bits: uuuutt : units | type */
	return (bits & 0x3);
}

uint8_t css_computed_left(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	uint8_t bits = style->bits[CSS_LEFT_INDEX];
	bits &= CSS_LEFT_MASK;
	bits >>= CSS_LEFT_SHIFT;

	/* Fix up, based on computed position */
	if (css_computed_position(style) == CSS_POSITION_STATIC) {
		/* Static -> auto */
		bits = CSS_LEFT_AUTO;
	} else if (css_computed_position(style) == CSS_POSITION_RELATIVE) {
		/* Relative -> follow $9.4.3 */
		uint8_t right = style->bits[CSS_RIGHT_INDEX];
		right &= CSS_RIGHT_MASK;
		right >>= CSS_RIGHT_SHIFT;

		if ((bits & 0x3) == CSS_LEFT_AUTO && 
				(right & 0x3) == CSS_RIGHT_AUTO) {
			/* Both auto => 0px */
			*length = 0;
			*unit = CSS_UNIT_PX;
		} else if ((bits & 0x3) == CSS_LEFT_AUTO) {
			/* Left is auto => -right */
			*length = -style->right;
			*unit = (css_unit) (right >> 2);
		} else {
			/** \todo Consider containing block's direction 
			 * if overconstrained */
			*length = style->left;
			*unit = (css_unit) (bits >> 2);
		}

		bits = CSS_LEFT_SET;
	} else if ((bits & 0x3) == CSS_LEFT_SET) {
		*length = style->left;
		*unit = (css_unit) (bits >> 2);
	}

	/* 6bits: uuuutt : units | type */
	return (bits & 0x3);
}
#undef CSS_LEFT_MASK
#undef CSS_LEFT_SHIFT
#undef CSS_LEFT_INDEX
#undef CSS_BOTTOM_MASK
#undef CSS_BOTTOM_SHIFT
#undef CSS_BOTTOM_INDEX
#undef CSS_RIGHT_MASK
#undef CSS_RIGHT_SHIFT
#undef CSS_RIGHT_INDEX
#undef CSS_TOP_MASK
#undef CSS_TOP_SHIFT
#undef CSS_TOP_INDEX

#define CSS_BORDER_TOP_COLOR_INDEX 6
#define CSS_BORDER_TOP_COLOR_SHIFT 0
#define CSS_BORDER_TOP_COLOR_MASK  0x3
uint8_t css_computed_border_top_color(
		const css_computed_style *style, 
		css_color *color)
{
	uint8_t bits = style->bits[CSS_BORDER_TOP_COLOR_INDEX];
	bits &= CSS_BORDER_TOP_COLOR_MASK;
	bits >>= CSS_BORDER_TOP_COLOR_SHIFT;

	/* 2bits: type */
	*color = style->border_color[0];

	return bits;
}
#undef CSS_BORDER_TOP_COLOR_MASK
#undef CSS_BORDER_TOP_COLOR_SHIFT
#undef CSS_BORDER_TOP_COLOR_INDEX

#define CSS_BORDER_RIGHT_COLOR_INDEX 7
#define CSS_BORDER_RIGHT_COLOR_SHIFT 0
#define CSS_BORDER_RIGHT_COLOR_MASK  0x3
uint8_t css_computed_border_right_color(
		const css_computed_style *style, 
		css_color *color)
{
	uint8_t bits = style->bits[CSS_BORDER_RIGHT_COLOR_INDEX];
	bits &= CSS_BORDER_RIGHT_COLOR_MASK;
	bits >>= CSS_BORDER_RIGHT_COLOR_SHIFT;

	/* 2bits: type */
	*color = style->border_color[1];

	return bits;
}
#undef CSS_BORDER_RIGHT_COLOR_MASK
#undef CSS_BORDER_RIGHT_COLOR_SHIFT
#undef CSS_BORDER_RIGHT_COLOR_INDEX

#define CSS_BORDER_BOTTOM_COLOR_INDEX 8
#define CSS_BORDER_BOTTOM_COLOR_SHIFT 0
#define CSS_BORDER_BOTTOM_COLOR_MASK  0x3
uint8_t css_computed_border_bottom_color(
		const css_computed_style *style, 
		css_color *color)
{
	uint8_t bits = style->bits[CSS_BORDER_BOTTOM_COLOR_INDEX];
	bits &= CSS_BORDER_BOTTOM_COLOR_MASK;
	bits >>= CSS_BORDER_BOTTOM_COLOR_SHIFT;

	/* 2bits: type */
	*color = style->border_color[2];

	return bits;
}
#undef CSS_BORDER_BOTTOM_COLOR_MASK
#undef CSS_BORDER_BOTTOM_COLOR_SHIFT
#undef CSS_BORDER_BOTTOM_COLOR_INDEX

#define CSS_BORDER_LEFT_COLOR_INDEX 9
#define CSS_BORDER_LEFT_COLOR_SHIFT 0
#define CSS_BORDER_LEFT_COLOR_MASK  0x3
uint8_t css_computed_border_left_color(
		const css_computed_style *style, 
		css_color *color)
{
	uint8_t bits = style->bits[CSS_BORDER_LEFT_COLOR_INDEX];
	bits &= CSS_BORDER_LEFT_COLOR_MASK;
	bits >>= CSS_BORDER_LEFT_COLOR_SHIFT;

	/* 2bits: type */
	*color = style->border_color[3];

	return bits;
}
#undef CSS_BORDER_LEFT_COLOR_MASK
#undef CSS_BORDER_LEFT_COLOR_SHIFT
#undef CSS_BORDER_LEFT_COLOR_INDEX

#define CSS_HEIGHT_INDEX 10
#define CSS_HEIGHT_SHIFT 2
#define CSS_HEIGHT_MASK  0xfc
uint8_t css_computed_height(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	uint8_t bits = style->bits[CSS_HEIGHT_INDEX];
	bits &= CSS_HEIGHT_MASK;
	bits >>= CSS_HEIGHT_SHIFT;

	/* 6bits: uuuutt : units | type */
	if ((bits & 0x3) == CSS_HEIGHT_SET) {
		*length = style->height;
		*unit = (css_unit) (bits >> 2);
	}

	return (bits & 0x3);
}
#undef CSS_HEIGHT_MASK
#undef CSS_HEIGHT_SHIFT
#undef CSS_HEIGHT_INDEX

#define CSS_LINE_HEIGHT_INDEX 11
#define CSS_LINE_HEIGHT_SHIFT 2
#define CSS_LINE_HEIGHT_MASK  0xfc
uint8_t css_computed_line_height(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	uint8_t bits = style->bits[CSS_LINE_HEIGHT_INDEX];
	bits &= CSS_LINE_HEIGHT_MASK;
	bits >>= CSS_LINE_HEIGHT_SHIFT;

	/* 6bits: uuuutt : units | type */
	if ((bits & 0x3) == CSS_LINE_HEIGHT_NUMBER || 
			(bits & 0x3) == CSS_LINE_HEIGHT_DIMENSION) {
		*length = style->line_height;
	}

	if ((bits & 0x3) == CSS_LINE_HEIGHT_DIMENSION) {
		*unit = (css_unit) (bits >> 2);
	}

	return (bits & 0x3);
}
#undef CSS_LINE_HEIGHT_MASK
#undef CSS_LINE_HEIGHT_SHIFT
#undef CSS_LINE_HEIGHT_INDEX

#define CSS_BACKGROUND_COLOR_INDEX 10
#define CSS_BACKGROUND_COLOR_SHIFT 0
#define CSS_BACKGROUND_COLOR_MASK  0x3
uint8_t css_computed_background_color(
		const css_computed_style *style, 
		css_color *color)
{
	uint8_t bits = style->bits[CSS_BACKGROUND_COLOR_INDEX];
	bits &= CSS_BACKGROUND_COLOR_MASK;
	bits >>= CSS_BACKGROUND_COLOR_SHIFT;

	/* 2bits: type */
	*color = style->background_color;

	return bits;
}
#undef CSS_BACKGROUND_COLOR_MASK
#undef CSS_BACKGROUND_COLOR_SHIFT
#undef CSS_BACKGROUND_COLOR_INDEX

#define CSS_Z_INDEX_INDEX 11
#define CSS_Z_INDEX_SHIFT 0
#define CSS_Z_INDEX_MASK  0x3
uint8_t css_computed_z_index(
		const css_computed_style *style, 
		int32_t *z_index)
{
	uint8_t bits = style->bits[CSS_Z_INDEX_INDEX];
	bits &= CSS_Z_INDEX_MASK;
	bits >>= CSS_Z_INDEX_SHIFT;

	/* 2bits: type */
	*z_index = style->z_index;

	return bits;
}
#undef CSS_Z_INDEX_MASK
#undef CSS_Z_INDEX_SHIFT
#undef CSS_Z_INDEX_INDEX

#define CSS_MARGIN_TOP_INDEX 12
#define CSS_MARGIN_TOP_SHIFT 2
#define CSS_MARGIN_TOP_MASK  0xfc
uint8_t css_computed_margin_top(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	uint8_t bits = style->bits[CSS_MARGIN_TOP_INDEX];
	bits &= CSS_MARGIN_TOP_MASK;
	bits >>= CSS_MARGIN_TOP_SHIFT;

	/* 6bits: uuuutt : units | type */
	if ((bits & 0x3) == CSS_MARGIN_SET) {
		*length = style->margin[0];
		*unit = (css_unit) (bits >> 2);
	}

	return (bits & 0x3);
}
#undef CSS_MARGIN_TOP_MASK
#undef CSS_MARGIN_TOP_SHIFT
#undef CSS_MARGIN_TOP_INDEX

#define CSS_MARGIN_RIGHT_INDEX 13
#define CSS_MARGIN_RIGHT_SHIFT 2
#define CSS_MARGIN_RIGHT_MASK  0xfc
uint8_t css_computed_margin_right(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	uint8_t bits = style->bits[CSS_MARGIN_RIGHT_INDEX];
	bits &= CSS_MARGIN_RIGHT_MASK;
	bits >>= CSS_MARGIN_RIGHT_SHIFT;

	/* 6bits: uuuutt : units | type */
	if ((bits & 0x3) == CSS_MARGIN_SET) {
		*length = style->margin[1];
		*unit = (css_unit) (bits >> 2);
	}

	return (bits & 0x3);
}
#undef CSS_MARGIN_RIGHT_MASK
#undef CSS_MARGIN_RIGHT_SHIFT
#undef CSS_MARGIN_RIGHT_INDEX

#define CSS_MARGIN_BOTTOM_INDEX 14
#define CSS_MARGIN_BOTTOM_SHIFT 2
#define CSS_MARGIN_BOTTOM_MASK  0xfc
uint8_t css_computed_margin_bottom(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	uint8_t bits = style->bits[CSS_MARGIN_BOTTOM_INDEX];
	bits &= CSS_MARGIN_BOTTOM_MASK;
	bits >>= CSS_MARGIN_BOTTOM_SHIFT;

	/* 6bits: uuuutt : units | type */
	if ((bits & 0x3) == CSS_MARGIN_SET) {
		*length = style->margin[2];
		*unit = (css_unit) (bits >> 2);
	}

	return (bits & 0x3);
}
#undef CSS_MARGIN_BOTTOM_MASK
#undef CSS_MARGIN_BOTTOM_SHIFT
#undef CSS_MARGIN_BOTTOM_INDEX

#define CSS_MARGIN_LEFT_INDEX 15
#define CSS_MARGIN_LEFT_SHIFT 2
#define CSS_MARGIN_LEFT_MASK  0xfc
uint8_t css_computed_margin_left(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	uint8_t bits = style->bits[CSS_MARGIN_LEFT_INDEX];
	bits &= CSS_MARGIN_LEFT_MASK;
	bits >>= CSS_MARGIN_LEFT_SHIFT;

	/* 6bits: uuuutt : units | type */
	if ((bits & 0x3) == CSS_MARGIN_SET) {
		*length = style->margin[3];
		*unit = (css_unit) (bits >> 2);
	}

	return (bits & 0x3);
}
#undef CSS_MARGIN_LEFT_MASK
#undef CSS_MARGIN_LEFT_SHIFT
#undef CSS_MARGIN_LEFT_INDEX

#define CSS_BACKGROUND_ATTACHMENT_INDEX 12
#define CSS_BACKGROUND_ATTACHMENT_SHIFT 0
#define CSS_BACKGROUND_ATTACHMENT_MASK  0x3
uint8_t css_computed_background_attachment(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_BACKGROUND_ATTACHMENT_INDEX];
	bits &= CSS_BACKGROUND_ATTACHMENT_MASK;
	bits >>= CSS_BACKGROUND_ATTACHMENT_SHIFT;

	/* 2bits: type */
	return bits;
}
#undef CSS_BACKGROUND_ATTACHMENT_MASK
#undef CSS_BACKGROUND_ATTACHMENT_SHIFT
#undef CSS_BACKGROUND_ATTACHMENT_INDEX

#define CSS_BORDER_COLLAPSE_INDEX 13
#define CSS_BORDER_COLLAPSE_SHIFT 0
#define CSS_BORDER_COLLAPSE_MASK  0x3
uint8_t css_computed_border_collapse(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_BORDER_COLLAPSE_INDEX];
	bits &= CSS_BORDER_COLLAPSE_MASK;
	bits >>= CSS_BORDER_COLLAPSE_SHIFT;

	/* 2bits: type */
	return bits;
}
#undef CSS_BORDER_COLLAPSE_MASK
#undef CSS_BORDER_COLLAPSE_SHIFT
#undef CSS_BORDER_COLLAPSE_INDEX

#define CSS_CAPTION_SIDE_INDEX 14
#define CSS_CAPTION_SIDE_SHIFT 0
#define CSS_CAPTION_SIDE_MASK  0x3
uint8_t css_computed_caption_side(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_CAPTION_SIDE_INDEX];
	bits &= CSS_CAPTION_SIDE_MASK;
	bits >>= CSS_CAPTION_SIDE_SHIFT;

	/* 2bits: type */
	return bits;
}
#undef CSS_CAPTION_SIDE_MASK
#undef CSS_CAPTION_SIDE_SHIFT
#undef CSS_CAPTION_SIDE_INDEX

#define CSS_DIRECTION_INDEX 15
#define CSS_DIRECTION_SHIFT 0
#define CSS_DIRECTION_MASK  0x3
uint8_t css_computed_direction(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_DIRECTION_INDEX];
	bits &= CSS_DIRECTION_MASK;
	bits >>= CSS_DIRECTION_SHIFT;

	/* 2bits: type */
	return bits;
}
#undef CSS_DIRECTION_MASK
#undef CSS_DIRECTION_SHIFT
#undef CSS_DIRECTION_INDEX

#define CSS_MAX_HEIGHT_INDEX 16
#define CSS_MAX_HEIGHT_SHIFT 2
#define CSS_MAX_HEIGHT_MASK  0xfc
uint8_t css_computed_max_height(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	uint8_t bits = style->bits[CSS_MAX_HEIGHT_INDEX];
	bits &= CSS_MAX_HEIGHT_MASK;
	bits >>= CSS_MAX_HEIGHT_SHIFT;

	/* 6bits: uuuutt : units | type */
	if ((bits & 0x3) == CSS_MAX_HEIGHT_SET) {
		*length = style->max_height;
		*unit = (css_unit) (bits >> 2);
	}

	return (bits & 0x3);
}
#undef CSS_MAX_HEIGHT_MASK
#undef CSS_MAX_HEIGHT_SHIFT
#undef CSS_MAX_HEIGHT_INDEX

#define CSS_MAX_WIDTH_INDEX 17
#define CSS_MAX_WIDTH_SHIFT 2
#define CSS_MAX_WIDTH_MASK  0xfc
uint8_t css_computed_max_width(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	uint8_t bits = style->bits[CSS_MAX_WIDTH_INDEX];
	bits &= CSS_MAX_WIDTH_MASK;
	bits >>= CSS_MAX_WIDTH_SHIFT;

	/* 6bits: uuuutt : units | type */
	if ((bits & 0x3) == CSS_MAX_WIDTH_SET) {
		*length = style->max_width;
		*unit = (css_unit) (bits >> 2);
	}

	return (bits & 0x3);
}
#undef CSS_MAX_WIDTH_MASK
#undef CSS_MAX_WIDTH_SHIFT
#undef CSS_MAX_WIDTH_INDEX

#define CSS_WIDTH_INDEX 18
#define CSS_WIDTH_SHIFT 2
#define CSS_WIDTH_MASK  0xfc
uint8_t css_computed_width(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	uint8_t bits = style->bits[CSS_WIDTH_INDEX];
	bits &= CSS_WIDTH_MASK;
	bits >>= CSS_WIDTH_SHIFT;

	/* 6bits: uuuutt : units | type */
	if ((bits & 0x3) == CSS_WIDTH_SET) {
		*length = style->width;
		*unit = (css_unit) (bits >> 2);
	}

	return (bits & 0x3);
}
#undef CSS_WIDTH_MASK
#undef CSS_WIDTH_SHIFT
#undef CSS_WIDTH_INDEX

#define CSS_EMPTY_CELLS_INDEX 16
#define CSS_EMPTY_CELLS_SHIFT 0
#define CSS_EMPTY_CELLS_MASK  0x3
uint8_t css_computed_empty_cells(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_EMPTY_CELLS_INDEX];
	bits &= CSS_EMPTY_CELLS_MASK;
	bits >>= CSS_EMPTY_CELLS_SHIFT;

	/* 2bits: type */
	return bits;
}
#undef CSS_EMPTY_CELLS_MASK
#undef CSS_EMPTY_CELLS_SHIFT
#undef CSS_EMPTY_CELLS_INDEX

#define CSS_FLOAT_INDEX 17
#define CSS_FLOAT_SHIFT 0
#define CSS_FLOAT_MASK  0x3
uint8_t css_computed_float(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_FLOAT_INDEX];
	bits &= CSS_FLOAT_MASK;
	bits >>= CSS_FLOAT_SHIFT;

	/* Fix up as per $9.7:2 */
	if (css_computed_position(style) == CSS_POSITION_ABSOLUTE ||
			css_computed_position(style) == CSS_POSITION_FIXED)
		return CSS_FLOAT_NONE;

	/* 2bits: type */
	return bits;
}
#undef CSS_FLOAT_MASK
#undef CSS_FLOAT_SHIFT
#undef CSS_FLOAT_INDEX

#define CSS_FONT_STYLE_INDEX 18
#define CSS_FONT_STYLE_SHIFT 0
#define CSS_FONT_STYLE_MASK  0x3
uint8_t css_computed_font_style(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_FONT_STYLE_INDEX];
	bits &= CSS_FONT_STYLE_MASK;
	bits >>= CSS_FONT_STYLE_SHIFT;

	/* 2bits: type */
	return bits;
}
#undef CSS_FONT_STYLE_MASK
#undef CSS_FONT_STYLE_SHIFT
#undef CSS_FONT_STYLE_INDEX

#define CSS_MIN_HEIGHT_INDEX 19
#define CSS_MIN_HEIGHT_SHIFT 3
#define CSS_MIN_HEIGHT_MASK  0xf8
uint8_t css_computed_min_height(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	uint8_t bits = style->bits[CSS_MIN_HEIGHT_INDEX];
	bits &= CSS_MIN_HEIGHT_MASK;
	bits >>= CSS_MIN_HEIGHT_SHIFT;

	/* 5bits: uuuut : units | type */
	if ((bits & 0x1) == CSS_MIN_HEIGHT_SET) {
		*length = style->min_height;
		*unit = (css_unit) (bits >> 1);
	}

	return (bits & 0x1);
}
#undef CSS_MIN_HEIGHT_MASK
#undef CSS_MIN_HEIGHT_SHIFT
#undef CSS_MIN_HEIGHT_INDEX

#define CSS_MIN_WIDTH_INDEX 20
#define CSS_MIN_WIDTH_SHIFT 3
#define CSS_MIN_WIDTH_MASK  0xf8
uint8_t css_computed_min_width(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	uint8_t bits = style->bits[CSS_MIN_WIDTH_INDEX];
	bits &= CSS_MIN_WIDTH_MASK;
	bits >>= CSS_MIN_WIDTH_SHIFT;

	/* 5bits: uuuut : units | type */
	if ((bits & 0x1) == CSS_MIN_WIDTH_SET) {
		*length = style->min_width;
		*unit = (css_unit) (bits >> 1);
	}

	return (bits & 0x1);
}
#undef CSS_MIN_WIDTH_MASK
#undef CSS_MIN_WIDTH_SHIFT
#undef CSS_MIN_WIDTH_INDEX

#define CSS_BACKGROUND_REPEAT_INDEX 19
#define CSS_BACKGROUND_REPEAT_SHIFT 0
#define CSS_BACKGROUND_REPEAT_MASK  0x7
uint8_t css_computed_background_repeat(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_BACKGROUND_REPEAT_INDEX];
	bits &= CSS_BACKGROUND_REPEAT_MASK;
	bits >>= CSS_BACKGROUND_REPEAT_SHIFT;

	/* 3bits: type */
	return bits;
}
#undef CSS_BACKGROUND_REPEAT_MASK
#undef CSS_BACKGROUND_REPEAT_SHIFT
#undef CSS_BACKGROUND_REPEAT_INDEX

#define CSS_CLEAR_INDEX 20
#define CSS_CLEAR_SHIFT 0
#define CSS_CLEAR_MASK  0x7
uint8_t css_computed_clear(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_CLEAR_INDEX];
	bits &= CSS_CLEAR_MASK;
	bits >>= CSS_CLEAR_SHIFT;

	/* 3bits: type */
	return bits;
}
#undef CSS_CLEAR_MASK
#undef CSS_CLEAR_SHIFT
#undef CSS_CLEAR_INDEX

#define CSS_PADDING_TOP_INDEX 21
#define CSS_PADDING_TOP_SHIFT 3
#define CSS_PADDING_TOP_MASK  0xf8
uint8_t css_computed_padding_top(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	uint8_t bits = style->bits[CSS_PADDING_TOP_INDEX];
	bits &= CSS_PADDING_TOP_MASK;
	bits >>= CSS_PADDING_TOP_SHIFT;

	/* 5bits: uuuut : units | type */
	if ((bits & 0x1) == CSS_PADDING_SET) {
		*length = style->padding[0];
		*unit = (css_unit) (bits >> 1);
	}

	return (bits & 0x1);
}
#undef CSS_PADDING_TOP_MASK
#undef CSS_PADDING_TOP_SHIFT
#undef CSS_PADDING_TOP_INDEX

#define CSS_PADDING_RIGHT_INDEX 22
#define CSS_PADDING_RIGHT_SHIFT 3
#define CSS_PADDING_RIGHT_MASK  0xf8
uint8_t css_computed_padding_right(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	uint8_t bits = style->bits[CSS_PADDING_RIGHT_INDEX];
	bits &= CSS_PADDING_RIGHT_MASK;
	bits >>= CSS_PADDING_RIGHT_SHIFT;

	/* 5bits: uuuut : units | type */
	if ((bits & 0x1) == CSS_PADDING_SET) {
		*length = style->padding[1];
		*unit = (css_unit) (bits >> 1);
	}

	return (bits & 0x1);
}
#undef CSS_PADDING_RIGHT_MASK
#undef CSS_PADDING_RIGHT_SHIFT
#undef CSS_PADDING_RIGHT_INDEX

#define CSS_PADDING_BOTTOM_INDEX 23
#define CSS_PADDING_BOTTOM_SHIFT 3
#define CSS_PADDING_BOTTOM_MASK  0xf8
uint8_t css_computed_padding_bottom(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	uint8_t bits = style->bits[CSS_PADDING_BOTTOM_INDEX];
	bits &= CSS_PADDING_BOTTOM_MASK;
	bits >>= CSS_PADDING_BOTTOM_SHIFT;

	/* 5bits: uuuut : units | type */
	if ((bits & 0x1) == CSS_PADDING_SET) {
		*length = style->padding[2];
		*unit = (css_unit) (bits >> 1);
	}

	return (bits & 0x1);
}
#undef CSS_PADDING_BOTTOM_MASK
#undef CSS_PADDING_BOTTOM_SHIFT
#undef CSS_PADDING_BOTTOM_INDEX

#define CSS_PADDING_LEFT_INDEX 24
#define CSS_PADDING_LEFT_SHIFT 3
#define CSS_PADDING_LEFT_MASK  0xf8
uint8_t css_computed_padding_left(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	uint8_t bits = style->bits[CSS_PADDING_LEFT_INDEX];
	bits &= CSS_PADDING_LEFT_MASK;
	bits >>= CSS_PADDING_LEFT_SHIFT;

	/* 5bits: uuuut : units | type */
	if ((bits & 0x1) == CSS_PADDING_SET) {
		*length = style->padding[3];
		*unit = (css_unit) (bits >> 1);
	}

	return (bits & 0x1);
}
#undef CSS_PADDING_LEFT_MASK
#undef CSS_PADDING_LEFT_SHIFT
#undef CSS_PADDING_LEFT_INDEX

#define CSS_OVERFLOW_INDEX 21
#define CSS_OVERFLOW_SHIFT 0
#define CSS_OVERFLOW_MASK  0x7
uint8_t css_computed_overflow(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_OVERFLOW_INDEX];
	bits &= CSS_OVERFLOW_MASK;
	bits >>= CSS_OVERFLOW_SHIFT;

	/* 3bits: type */
	return bits;
}
#undef CSS_OVERFLOW_MASK
#undef CSS_OVERFLOW_SHIFT
#undef CSS_OVERFLOW_INDEX

#define CSS_POSITION_INDEX 22
#define CSS_POSITION_SHIFT 0
#define CSS_POSITION_MASK  0x7
uint8_t css_computed_position(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_POSITION_INDEX];
	bits &= CSS_POSITION_MASK;
	bits >>= CSS_POSITION_SHIFT;

	/* 3bits: type */
	return bits;
}
#undef CSS_POSITION_MASK
#undef CSS_POSITION_SHIFT
#undef CSS_POSITION_INDEX

#define CSS_OPACITY_INDEX 23
#define CSS_OPACITY_SHIFT 2
#define CSS_OPACITY_MASK  0x04
uint8_t css_computed_opacity(
		const css_computed_style *style, 
		css_fixed *opacity)
{
	uint8_t bits = style->bits[CSS_OPACITY_INDEX];
	bits &= CSS_OPACITY_MASK;
	bits >>= CSS_OPACITY_SHIFT;

	/* 1bit: t : type */
	if ((bits & 0x1) == CSS_OPACITY_SET) {
		*opacity = style->opacity;
	}

	return (bits & 0x1);
}
#undef CSS_OPACITY_MASK
#undef CSS_OPACITY_SHIFT
#undef CSS_OPACITY_INDEX

#define CSS_TEXT_TRANSFORM_INDEX 24
#define CSS_TEXT_TRANSFORM_SHIFT 0
#define CSS_TEXT_TRANSFORM_MASK  0x7
uint8_t css_computed_text_transform(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_TEXT_TRANSFORM_INDEX];
	bits &= CSS_TEXT_TRANSFORM_MASK;
	bits >>= CSS_TEXT_TRANSFORM_SHIFT;

	/* 3bits: type */
	return bits;
}
#undef CSS_TEXT_TRANSFORM_MASK
#undef CSS_TEXT_TRANSFORM_SHIFT
#undef CSS_TEXT_TRANSFORM_INDEX

#define CSS_TEXT_INDENT_INDEX 25
#define CSS_TEXT_INDENT_SHIFT 3
#define CSS_TEXT_INDENT_MASK  0xf8
uint8_t css_computed_text_indent(
		const css_computed_style *style, 
		css_fixed *length, css_unit *unit)
{
	uint8_t bits = style->bits[CSS_TEXT_INDENT_INDEX];
	bits &= CSS_TEXT_INDENT_MASK;
	bits >>= CSS_TEXT_INDENT_SHIFT;

	/* 5bits: uuuut : units | type */
	if ((bits & 0x1) == CSS_TEXT_INDENT_SET) {
		*length = style->text_indent;
		*unit = (css_unit) (bits >> 1);
	}

	return (bits & 0x1);
}
#undef CSS_TEXT_INDENT_MASK
#undef CSS_TEXT_INDENT_SHIFT
#undef CSS_TEXT_INDENT_INDEX

#define CSS_WHITE_SPACE_INDEX 25
#define CSS_WHITE_SPACE_SHIFT 0
#define CSS_WHITE_SPACE_MASK  0x7
uint8_t css_computed_white_space(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_WHITE_SPACE_INDEX];
	bits &= CSS_WHITE_SPACE_MASK;
	bits >>= CSS_WHITE_SPACE_SHIFT;

	/* 3bits: type */
	return bits;
}
#undef CSS_WHITE_SPACE_MASK
#undef CSS_WHITE_SPACE_SHIFT
#undef CSS_WHITE_SPACE_INDEX

#define CSS_BACKGROUND_POSITION_INDEX 27
#define CSS_BACKGROUND_POSITION_SHIFT 7
#define CSS_BACKGROUND_POSITION_MASK  0x80
#define CSS_BACKGROUND_POSITION_INDEX1 26
#define CSS_BACKGROUND_POSITION_SHIFT1 0
#define CSS_BACKGROUND_POSITION_MASK1 0xff
uint8_t css_computed_background_position(
		const css_computed_style *style, 
		css_fixed *hlength, css_unit *hunit,
		css_fixed *vlength, css_unit *vunit)
{
	uint8_t bits = style->bits[CSS_BACKGROUND_POSITION_INDEX];
	bits &= CSS_BACKGROUND_POSITION_MASK;
	bits >>= CSS_BACKGROUND_POSITION_SHIFT;

	/* 1bit: type */
	if (bits == CSS_BACKGROUND_POSITION_SET) {
		uint8_t bits1 = style->bits[CSS_BACKGROUND_POSITION_INDEX1];
		bits1 &= CSS_BACKGROUND_POSITION_MASK1;
		bits1 >>= CSS_BACKGROUND_POSITION_SHIFT1;

		/* 8bits: hhhhvvvv : hunit | vunit */
		*hlength = style->background_position[0];
		*hunit = (css_unit) (bits1 >> 4);

		*vlength = style->background_position[1];
		*vunit = (css_unit) (bits1 & 0xf);
	}

	return bits;
}
#undef CSS_BACKGROUND_POSITION_MASK1
#undef CSS_BACKGROUND_POSITION_SHIFT1
#undef CSS_BACKGROUND_POSITION_INDEX1
#undef CSS_BACKGROUND_POSITION_MASK
#undef CSS_BACKGROUND_POSITION_SHIFT
#undef CSS_BACKGROUND_POSITION_INDEX

#define CSS_DISPLAY_INDEX 27
#define CSS_DISPLAY_SHIFT 2
#define CSS_DISPLAY_MASK  0x7c
uint8_t css_computed_display(
		const css_computed_style *style, bool root)
{
	uint8_t position;
	uint8_t bits = style->bits[CSS_DISPLAY_INDEX];
	bits &= CSS_DISPLAY_MASK;
	bits >>= CSS_DISPLAY_SHIFT;

	/* Return computed display as per $9.7 */
	position = css_computed_position(style);

	/* 5bits: type */
	if (bits == CSS_DISPLAY_NONE)
		return bits; /* 1. */

	if ((position == CSS_POSITION_ABSOLUTE || 
			position == CSS_POSITION_FIXED) /* 2. */ ||
			css_computed_float(style) != CSS_FLOAT_NONE /* 3. */ ||
			root /* 4. */) {
		if (bits == CSS_DISPLAY_INLINE_TABLE) {
			return CSS_DISPLAY_TABLE;
		} else if (bits == CSS_DISPLAY_INLINE ||
				bits == CSS_DISPLAY_RUN_IN ||
				bits == CSS_DISPLAY_TABLE_ROW_GROUP ||
				bits == CSS_DISPLAY_TABLE_COLUMN ||
				bits == CSS_DISPLAY_TABLE_COLUMN_GROUP ||
				bits == CSS_DISPLAY_TABLE_HEADER_GROUP ||
				bits == CSS_DISPLAY_TABLE_FOOTER_GROUP ||
				bits == CSS_DISPLAY_TABLE_ROW ||
				bits == CSS_DISPLAY_TABLE_CELL ||
				bits == CSS_DISPLAY_TABLE_CAPTION ||
				bits == CSS_DISPLAY_INLINE_BLOCK) {
			return CSS_DISPLAY_BLOCK;
		}
	}

	/* 5. */
	return bits;
}

uint8_t css_computed_display_static(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_DISPLAY_INDEX];
	bits &= CSS_DISPLAY_MASK;
	bits >>= CSS_DISPLAY_SHIFT;

	/* 5bits: type */
	return bits;
}

#undef CSS_DISPLAY_MASK
#undef CSS_DISPLAY_SHIFT
#undef CSS_DISPLAY_INDEX

#define CSS_FONT_VARIANT_INDEX 27
#define CSS_FONT_VARIANT_SHIFT 0
#define CSS_FONT_VARIANT_MASK  0x3
uint8_t css_computed_font_variant(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_FONT_VARIANT_INDEX];
	bits &= CSS_FONT_VARIANT_MASK;
	bits >>= CSS_FONT_VARIANT_SHIFT;

	/* 2bits: type */
	return bits;
}
#undef CSS_FONT_VARIANT_MASK
#undef CSS_FONT_VARIANT_SHIFT
#undef CSS_FONT_VARIANT_INDEX

#define CSS_TEXT_DECORATION_INDEX 28
#define CSS_TEXT_DECORATION_SHIFT 3
#define CSS_TEXT_DECORATION_MASK  0xf8
uint8_t css_computed_text_decoration(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_TEXT_DECORATION_INDEX];
	bits &= CSS_TEXT_DECORATION_MASK;
	bits >>= CSS_TEXT_DECORATION_SHIFT;

	/* 5bits: type */
	return bits;
}
#undef CSS_TEXT_DECORATION_MASK
#undef CSS_TEXT_DECORATION_SHIFT
#undef CSS_TEXT_DECORATION_INDEX

#define CSS_FONT_FAMILY_INDEX 28
#define CSS_FONT_FAMILY_SHIFT 0
#define CSS_FONT_FAMILY_MASK  0x7
uint8_t css_computed_font_family(
		const css_computed_style *style, 
		lwc_string ***names)
{
	uint8_t bits = style->bits[CSS_FONT_FAMILY_INDEX];
	bits &= CSS_FONT_FAMILY_MASK;
	bits >>= CSS_FONT_FAMILY_SHIFT;

	/* 3bits: type */
	*names = style->font_family;

	return bits;
}
#undef CSS_FONT_FAMILY_MASK
#undef CSS_FONT_FAMILY_SHIFT
#undef CSS_FONT_FAMILY_INDEX

#define CSS_BORDER_TOP_STYLE_INDEX 29
#define CSS_BORDER_TOP_STYLE_SHIFT 4
#define CSS_BORDER_TOP_STYLE_MASK  0xf0
uint8_t css_computed_border_top_style(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_BORDER_TOP_STYLE_INDEX];
	bits &= CSS_BORDER_TOP_STYLE_MASK;
	bits >>= CSS_BORDER_TOP_STYLE_SHIFT;

	/* 4bits: type */
	return bits;
}
#undef CSS_BORDER_TOP_STYLE_MASK
#undef CSS_BORDER_TOP_STYLE_SHIFT
#undef CSS_BORDER_TOP_STYLE_INDEX

#define CSS_BORDER_RIGHT_STYLE_INDEX 29
#define CSS_BORDER_RIGHT_STYLE_SHIFT 0
#define CSS_BORDER_RIGHT_STYLE_MASK  0xf
uint8_t css_computed_border_right_style(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_BORDER_RIGHT_STYLE_INDEX];
	bits &= CSS_BORDER_RIGHT_STYLE_MASK;
	bits >>= CSS_BORDER_RIGHT_STYLE_SHIFT;

	/* 4bits: type */
	return bits;
}
#undef CSS_BORDER_RIGHT_STYLE_MASK
#undef CSS_BORDER_RIGHT_STYLE_SHIFT
#undef CSS_BORDER_RIGHT_STYLE_INDEX

#define CSS_BORDER_BOTTOM_STYLE_INDEX 30
#define CSS_BORDER_BOTTOM_STYLE_SHIFT 4
#define CSS_BORDER_BOTTOM_STYLE_MASK  0xf0
uint8_t css_computed_border_bottom_style(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_BORDER_BOTTOM_STYLE_INDEX];
	bits &= CSS_BORDER_BOTTOM_STYLE_MASK;
	bits >>= CSS_BORDER_BOTTOM_STYLE_SHIFT;

	/* 4bits: type */
	return bits;
}
#undef CSS_BORDER_BOTTOM_STYLE_MASK
#undef CSS_BORDER_BOTTOM_STYLE_SHIFT
#undef CSS_BORDER_BOTTOM_STYLE_INDEX

#define CSS_BORDER_LEFT_STYLE_INDEX 30
#define CSS_BORDER_LEFT_STYLE_SHIFT 0
#define CSS_BORDER_LEFT_STYLE_MASK  0xf
uint8_t css_computed_border_left_style(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_BORDER_LEFT_STYLE_INDEX];
	bits &= CSS_BORDER_LEFT_STYLE_MASK;
	bits >>= CSS_BORDER_LEFT_STYLE_SHIFT;

	/* 4bits: type */
	return bits;
}
#undef CSS_BORDER_LEFT_STYLE_MASK
#undef CSS_BORDER_LEFT_STYLE_SHIFT
#undef CSS_BORDER_LEFT_STYLE_INDEX

#define CSS_FONT_WEIGHT_INDEX 31
#define CSS_FONT_WEIGHT_SHIFT 4
#define CSS_FONT_WEIGHT_MASK  0xf0
uint8_t css_computed_font_weight(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_FONT_WEIGHT_INDEX];
	bits &= CSS_FONT_WEIGHT_MASK;
	bits >>= CSS_FONT_WEIGHT_SHIFT;

	/* 4bits: type */
	return bits;
}
#undef CSS_FONT_WEIGHT_MASK
#undef CSS_FONT_WEIGHT_SHIFT
#undef CSS_FONT_WEIGHT_INDEX

#define CSS_LIST_STYLE_TYPE_INDEX 31
#define CSS_LIST_STYLE_TYPE_SHIFT 0
#define CSS_LIST_STYLE_TYPE_MASK  0xf
uint8_t css_computed_list_style_type(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_LIST_STYLE_TYPE_INDEX];
	bits &= CSS_LIST_STYLE_TYPE_MASK;
	bits >>= CSS_LIST_STYLE_TYPE_SHIFT;

	/* 4bits: type */
	return bits;
}
#undef CSS_LIST_STYLE_TYPE_MASK
#undef CSS_LIST_STYLE_TYPE_SHIFT
#undef CSS_LIST_STYLE_TYPE_INDEX

#define CSS_OUTLINE_STYLE_INDEX 32
#define CSS_OUTLINE_STYLE_SHIFT 4
#define CSS_OUTLINE_STYLE_MASK  0xf0
uint8_t css_computed_outline_style(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_OUTLINE_STYLE_INDEX];
	bits &= CSS_OUTLINE_STYLE_MASK;
	bits >>= CSS_OUTLINE_STYLE_SHIFT;

	/* 4bits: type */
	return bits;
}
#undef CSS_OUTLINE_STYLE_MASK
#undef CSS_OUTLINE_STYLE_SHIFT
#undef CSS_OUTLINE_STYLE_INDEX

#define CSS_TABLE_LAYOUT_INDEX 32
#define CSS_TABLE_LAYOUT_SHIFT 2
#define CSS_TABLE_LAYOUT_MASK  0xc
uint8_t css_computed_table_layout(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_TABLE_LAYOUT_INDEX];
	bits &= CSS_TABLE_LAYOUT_MASK;
	bits >>= CSS_TABLE_LAYOUT_SHIFT;

	/* 2bits: type */
	return bits;
}
#undef CSS_TABLE_LAYOUT_MASK
#undef CSS_TABLE_LAYOUT_SHIFT
#undef CSS_TABLE_LAYOUT_INDEX

#define CSS_UNICODE_BIDI_INDEX 32
#define CSS_UNICODE_BIDI_SHIFT 0
#define CSS_UNICODE_BIDI_MASK  0x3
uint8_t css_computed_unicode_bidi(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_UNICODE_BIDI_INDEX];
	bits &= CSS_UNICODE_BIDI_MASK;
	bits >>= CSS_UNICODE_BIDI_SHIFT;

	/* 2bits: type */
	return bits;
}
#undef CSS_UNICODE_BIDI_MASK
#undef CSS_UNICODE_BIDI_SHIFT
#undef CSS_UNICODE_BIDI_INDEX

#define CSS_VISIBILITY_INDEX 33
#define CSS_VISIBILITY_SHIFT 6
#define CSS_VISIBILITY_MASK  0xc0
uint8_t css_computed_visibility(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_VISIBILITY_INDEX];
	bits &= CSS_VISIBILITY_MASK;
	bits >>= CSS_VISIBILITY_SHIFT;

	/* 2bits: type */
	return bits;
}
#undef CSS_VISIBILITY_MASK
#undef CSS_VISIBILITY_SHIFT
#undef CSS_VISIBILITY_INDEX

#define CSS_LIST_STYLE_POSITION_INDEX 33
#define CSS_LIST_STYLE_POSITION_SHIFT 4
#define CSS_LIST_STYLE_POSITION_MASK  0x30
uint8_t css_computed_list_style_position(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_LIST_STYLE_POSITION_INDEX];
	bits &= CSS_LIST_STYLE_POSITION_MASK;
	bits >>= CSS_LIST_STYLE_POSITION_SHIFT;

	/* 2bits: type */
	return bits;
}
#undef CSS_LIST_STYLE_POSITION_MASK
#undef CSS_LIST_STYLE_POSITION_SHIFT
#undef CSS_LIST_STYLE_POSITION_INDEX

#define CSS_TEXT_ALIGN_INDEX 33
#define CSS_TEXT_ALIGN_SHIFT 0
#define CSS_TEXT_ALIGN_MASK  0xf
uint8_t css_computed_text_align(
		const css_computed_style *style)
{
	uint8_t bits = style->bits[CSS_TEXT_ALIGN_INDEX];
	bits &= CSS_TEXT_ALIGN_MASK;
	bits >>= CSS_TEXT_ALIGN_SHIFT;

	/* 4bits: type */
	return bits;
}
#undef CSS_TEXT_ALIGN_MASK
#undef CSS_TEXT_ALIGN_SHIFT
#undef CSS_TEXT_ALIGN_INDEX

#define CSS_PAGE_BREAK_AFTER_INDEX 0
#define CSS_PAGE_BREAK_AFTER_SHIFT 0
#define CSS_PAGE_BREAK_AFTER_MASK 0x7
uint8_t css_computed_page_break_after(
		const css_computed_style *style)
{
	if (style->page != NULL) {
		uint8_t bits = style->page->bits[CSS_PAGE_BREAK_AFTER_INDEX];
		bits &= CSS_PAGE_BREAK_AFTER_MASK;
		bits >>= CSS_PAGE_BREAK_AFTER_SHIFT;
	
		/* 3bits: type */
		return bits;
	}
    
	return CSS_PAGE_BREAK_AFTER_AUTO;
}
#undef CSS_PAGE_BREAK_AFTER_MASK
#undef CSS_PAGE_BREAK_AFTER_SHIFT
#undef CSS_PAGE_BREAK_AFTER_INDEX
 
#define CSS_PAGE_BREAK_BEFORE_INDEX 0
#define CSS_PAGE_BREAK_BEFORE_SHIFT 3
#define CSS_PAGE_BREAK_BEFORE_MASK 0x38
uint8_t css_computed_page_break_before(
		const css_computed_style *style)
{
	if (style->page != NULL) {
		uint8_t bits = style->page->bits[CSS_PAGE_BREAK_BEFORE_INDEX];
		bits &= CSS_PAGE_BREAK_BEFORE_MASK;
		bits >>= CSS_PAGE_BREAK_BEFORE_SHIFT;
	
		/* 3bits: type */
		return bits;
	}
    
	return CSS_PAGE_BREAK_BEFORE_AUTO;
}
#undef CSS_PAGE_BREAK_BEFORE_MASK
#undef CSS_PAGE_BREAK_BEFORE_SHIFT
#undef CSS_PAGE_BREAK_BEFORE_INDEX
    
#define CSS_PAGE_BREAK_INSIDE_INDEX 0
#define CSS_PAGE_BREAK_INSIDE_SHIFT 6
#define CSS_PAGE_BREAK_INSIDE_MASK 0xc0
uint8_t css_computed_page_break_inside(
	    const css_computed_style *style)
{
	if (style->page != NULL) {
		uint8_t bits = style->page->bits[CSS_PAGE_BREAK_INSIDE_INDEX];
		bits &= CSS_PAGE_BREAK_INSIDE_MASK;
		bits >>= CSS_PAGE_BREAK_INSIDE_SHIFT;
	
		/* 2bits: type */
		return bits;
	}
    
	return CSS_PAGE_BREAK_INSIDE_AUTO;
}
#undef CSS_PAGE_BREAK_INSIDE_MASK
#undef CSS_PAGE_BREAK_INSIDE_SHIFT
#undef CSS_PAGE_BREAK_INSIDE_INDEX

#define CSS_ORPHANS_INDEX 1
#define CSS_ORPHANS_SHIFT 0
#define CSS_ORPHANS_MASK  0x1
uint8_t css_computed_orphans(
		const css_computed_style *style, 
		int32_t *orphans)
{
	if (style->page != NULL) {
		uint8_t bits = style->page->bits[CSS_ORPHANS_INDEX];
		bits &= CSS_ORPHANS_MASK;
		bits >>= CSS_ORPHANS_SHIFT;

		*orphans = FIXTOINT(style->page->orphans);;

		/* 1bit: type */
		return bits;
	}

	/* Use initial value */
	*orphans = 2;

	return CSS_ORPHANS_SET;
}
#undef CSS_ORPHANS_MASK
#undef CSS_ORPHANS_SHIFT
#undef CSS_ORPHANS_INDEX

#define CSS_WIDOWS_INDEX 1
#define CSS_WIDOWS_SHIFT 1
#define CSS_WIDOWS_MASK  0x2
uint8_t css_computed_widows(
		const css_computed_style *style, 
		int32_t *widows)
{
	if (style->page != NULL) {
		uint8_t bits = style->page->bits[CSS_WIDOWS_INDEX];
		bits &= CSS_WIDOWS_MASK;
		bits >>= CSS_WIDOWS_SHIFT;

		*widows = FIXTOINT(style->page->widows);

		/* 1bit: type */
		return bits;
	}

	/* Use initial value */
	*widows = 2;

	return CSS_WIDOWS_SET;
}
#undef CSS_WIDOWS_MASK
#undef CSS_WIDOWS_SHIFT
#undef CSS_WIDOWS_INDEX


/******************************************************************************
 * Library internals                                                          *
 ******************************************************************************/

/**
 * Compute the absolute values of a style
 *
 * \param parent             Parent style, or NULL for tree root
 * \param style              Computed style to process
 * \param compute_font_size  Callback to calculate an absolute font-size
 * \param pw                 Private word for callback
 * \return CSS_OK on success.
 */
css_error css__compute_absolute_values(const css_computed_style *parent,
		css_computed_style *style,
		css_error (*compute_font_size)(void *pw, 
			const css_hint *parent, css_hint *size),
		void *pw)
{
	css_hint psize, size, ex_size;
	css_error error;

	/* Ensure font-size is absolute */
	if (parent != NULL) {
		psize.status = get_font_size(parent, 
				&psize.data.length.value, 
				&psize.data.length.unit);
	}

	size.status = get_font_size(style, 
			&size.data.length.value, 
			&size.data.length.unit);

	error = compute_font_size(pw, parent != NULL ? &psize : NULL, &size);
	if (error != CSS_OK)
		return error;

	error = set_font_size(style, size.status,
			size.data.length.value, 
			size.data.length.unit);
	if (error != CSS_OK)
		return error;

	/* Compute the size of an ex unit */
	ex_size.status = CSS_FONT_SIZE_DIMENSION;
	ex_size.data.length.value = INTTOFIX(1);
	ex_size.data.length.unit = CSS_UNIT_EX;
	error = compute_font_size(pw, &size, &ex_size);
	if (error != CSS_OK)
		return error;

	/* Convert ex size into ems */
	if (size.data.length.value != 0)
		ex_size.data.length.value = FDIV(ex_size.data.length.value, 
					size.data.length.value);
	else
		ex_size.data.length.value = 0;
	ex_size.data.length.unit = CSS_UNIT_EM;

	/* Fix up background-position */
	error = compute_absolute_length_pair(style, &ex_size.data.length, 
			get_background_position,
			set_background_position);
	if (error != CSS_OK)
		return error;

	/* Fix up background-color */
	error = compute_absolute_color(style,
			get_background_color,
			set_background_color);
	if (error != CSS_OK)
		return error;

	/* Fix up border-{top,right,bottom,left}-color */
	error = compute_border_colors(style);
	if (error != CSS_OK)
		return error;

	/* Fix up border-{top,right,bottom,left}-width */
	error = compute_absolute_border_width(style, &ex_size.data.length);
	if (error != CSS_OK)
		return error;

	/* Fix up sides */
	error = compute_absolute_sides(style, &ex_size.data.length);
	if (error != CSS_OK)
		return error;

	/* Fix up height */
	error = compute_absolute_length_auto(style, &ex_size.data.length, 
			get_height, set_height);
	if (error != CSS_OK)
		return error;

	/* Fix up line-height (must be before vertical-align) */
	error = compute_absolute_line_height(style, &ex_size.data.length);
	if (error != CSS_OK)
		return error;

	/* Fix up margins */
	error = compute_absolute_margins(style, &ex_size.data.length);
	if (error != CSS_OK)
		return error;

	/* Fix up max-height */
	error = compute_absolute_length_none(style, &ex_size.data.length, 
			get_max_height, set_max_height);
	if (error != CSS_OK)
		return error;

	/* Fix up max-width */
	error = compute_absolute_length_none(style, &ex_size.data.length, 
			get_max_width, set_max_width);
	if (error != CSS_OK)
		return error;

	/* Fix up min-height */
	error = compute_absolute_length(style, &ex_size.data.length, 
			get_min_height, set_min_height);
	if (error != CSS_OK)
		return error;

	/* Fix up min-width */
	error = compute_absolute_length(style, &ex_size.data.length, 
			get_min_width, set_min_width);
	if (error != CSS_OK)
		return error;

	/* Fix up padding */
	error = compute_absolute_padding(style, &ex_size.data.length);
	if (error != CSS_OK)
		return error;

	/* Fix up text-indent */
	error = compute_absolute_length(style, &ex_size.data.length, 
			get_text_indent, set_text_indent);
	if (error != CSS_OK)
		return error;

	/* Fix up vertical-align */
	error = compute_absolute_vertical_align(style, &ex_size.data.length);
	if (error != CSS_OK)
		return error;

	/* Fix up width */
	error = compute_absolute_length_auto(style, &ex_size.data.length, 
			get_width, set_width);
	if (error != CSS_OK)
		return error;

	/* Uncommon properties */
	if (style->uncommon != NULL) {
		/* Fix up border-spacing */
		error = compute_absolute_length_pair(style,
				&ex_size.data.length,
				get_border_spacing,
				set_border_spacing);
		if (error != CSS_OK)
			return error;

		/* Fix up clip */
		error = compute_absolute_clip(style, &ex_size.data.length);
		if (error != CSS_OK)
			return error;

		/* Fix up letter-spacing */
		error = compute_absolute_length_normal(style,
				&ex_size.data.length,
				get_letter_spacing, 
				set_letter_spacing);
		if (error != CSS_OK)
			return error;

		/* Fix up outline-color */
		error = compute_absolute_color(style,
				get_outline_color,
				set_outline_color);
		if (error != CSS_OK)
			return error;

		/* Fix up outline-width */
		error = compute_absolute_border_side_width(style, 
				&ex_size.data.length, 
				get_outline_width, 
				set_outline_width);
		if (error != CSS_OK)
			return error;

		/* Fix up word spacing */
		error = compute_absolute_length_normal(style,
				&ex_size.data.length,
				get_word_spacing, 
				set_word_spacing);
		if (error != CSS_OK)
			return error;
	}

	return CSS_OK;
}

/******************************************************************************
 * Absolute value calculators
 ******************************************************************************/

/**
 * Compute colour values, replacing any set to currentColor with
 * the computed value of color.
 *
 * \param style  The style to process
 * \param get    Accessor for colour value
 * \param set    Mutator for colour value
 * \return CSS_OK on success
 */
css_error compute_absolute_color(css_computed_style *style,
		uint8_t (*get)(const css_computed_style *style,
				css_color *color),
		css_error (*set)(css_computed_style *style,
				uint8_t type, css_color color))
{
	css_color color;
	css_error error = CSS_OK;

	if (get(style, &color) == CSS_BACKGROUND_COLOR_CURRENT_COLOR) {
		css_color computed_color;

		css_computed_color(style, &computed_color);

		error = set(style, CSS_BACKGROUND_COLOR_COLOR, computed_color);
	}

	return error;
}

/**
 * Compute border colours, replacing any set to currentColor with 
 * the computed value of color.
 *
 * \param style  The style to process
 * \return CSS_OK on success
 */
css_error compute_border_colors(css_computed_style *style)
{
	css_color color, bcol;
	css_error error;

	css_computed_color(style, &color);

	if (get_border_top_color(style, &bcol) == CSS_BORDER_COLOR_CURRENT_COLOR) {
		error = set_border_top_color(style, 
				CSS_BORDER_COLOR_COLOR, color);
		if (error != CSS_OK)
			return error;
	}

	if (get_border_right_color(style, &bcol) == CSS_BORDER_COLOR_CURRENT_COLOR) {
		error = set_border_right_color(style, 
				CSS_BORDER_COLOR_COLOR, color);
		if (error != CSS_OK)
			return error;
	}

	if (get_border_bottom_color(style, &bcol) == CSS_BORDER_COLOR_CURRENT_COLOR) {
		error = set_border_bottom_color(style, 
				CSS_BORDER_COLOR_COLOR, color);
		if (error != CSS_OK)
			return error;
	}

	if (get_border_left_color(style, &bcol) == CSS_BORDER_COLOR_CURRENT_COLOR) {
		error = set_border_left_color(style, 
				CSS_BORDER_COLOR_COLOR, color);
		if (error != CSS_OK)
			return error;
	}

	return CSS_OK;
}

/**
 * Compute absolute border widths
 *
 * \param style      Style to process
 * \param ex_size    Ex size in ems
 * \return CSS_OK on success
 */
css_error compute_absolute_border_width(css_computed_style *style,
		const css_hint_length *ex_size)
{
	css_error error;

	error = compute_absolute_border_side_width(style, ex_size,
			get_border_top_width, 
			set_border_top_width);
	if (error != CSS_OK)
		return error;

	error = compute_absolute_border_side_width(style, ex_size,
			get_border_right_width, 
			set_border_right_width);
	if (error != CSS_OK)
		return error;

	error = compute_absolute_border_side_width(style, ex_size,
			get_border_bottom_width, 
			set_border_bottom_width);
	if (error != CSS_OK)
		return error;

	error = compute_absolute_border_side_width(style, ex_size,
			get_border_left_width, 
			set_border_left_width);
	if (error != CSS_OK)
		return error;

	return CSS_OK;
}

/**
 * Compute an absolute border side width
 *
 * \param style      Style to process
 * \param ex_size    Ex size, in ems
 * \param get        Function to read length
 * \param set        Function to write length
 * \return CSS_OK on success
 */
css_error compute_absolute_border_side_width(css_computed_style *style,
		const css_hint_length *ex_size,
		uint8_t (*get)(const css_computed_style *style, 
				css_fixed *len, css_unit *unit),
		css_error (*set)(css_computed_style *style, uint8_t type,
				css_fixed len, css_unit unit))
{
	css_fixed length;
	css_unit unit;
	uint8_t type;

	type = get(style, &length, &unit);
	if (type == CSS_BORDER_WIDTH_THIN) {
		length = INTTOFIX(1);
		unit = CSS_UNIT_PX;
	} else if (type == CSS_BORDER_WIDTH_MEDIUM) {
		length = INTTOFIX(2);
		unit = CSS_UNIT_PX;
	} else if (type == CSS_BORDER_WIDTH_THICK) {
		length = INTTOFIX(4);
		unit = CSS_UNIT_PX;
	}

	if (unit == CSS_UNIT_EX) {
		length = FMUL(length, ex_size->value);
		unit = ex_size->unit;
	}

	return set(style, CSS_BORDER_WIDTH_WIDTH, length, unit);
}

/**
 * Compute absolute clip
 *
 * \param style      Style to process
 * \param ex_size    Ex size in ems
 * \return CSS_OK on success
 */
css_error compute_absolute_clip(css_computed_style *style,
		const css_hint_length *ex_size)
{
	css_computed_clip_rect rect = { 0, 0, 0, 0, CSS_UNIT_PX, CSS_UNIT_PX,
			CSS_UNIT_PX, CSS_UNIT_PX, false, false, false, false };
	css_error error;

	if (get_clip(style, &rect) == CSS_CLIP_RECT) {
		if (rect.top_auto == false) {
			if (rect.tunit == CSS_UNIT_EX) {
				rect.top = FMUL(rect.top, ex_size->value);
				rect.tunit = ex_size->unit;
			}
		}

		if (rect.right_auto == false) {
			if (rect.runit == CSS_UNIT_EX) {
				rect.right = FMUL(rect.right, ex_size->value);
				rect.runit = ex_size->unit;
			}
		}

		if (rect.bottom_auto == false) {
			if (rect.bunit == CSS_UNIT_EX) {
				rect.bottom = FMUL(rect.bottom, ex_size->value);
				rect.bunit = ex_size->unit;
			}
		}

		if (rect.left_auto == false) {
			if (rect.lunit == CSS_UNIT_EX) {
				rect.left = FMUL(rect.left, ex_size->value);
				rect.lunit = ex_size->unit;
			}
		}

		error = set_clip(style, CSS_CLIP_RECT, &rect);
		if (error != CSS_OK)
			return error;
	}

	return CSS_OK;
}

/**
 * Compute absolute line-height
 *
 * \param style      Style to process
 * \param ex_size    Ex size, in ems
 * \return CSS_OK on success
 */
css_error compute_absolute_line_height(css_computed_style *style,
		const css_hint_length *ex_size)
{
	css_fixed length = 0;
	css_unit unit = CSS_UNIT_PX;
	uint8_t type;
	css_error error;

	type = get_line_height(style, &length, &unit);

	if (type == CSS_LINE_HEIGHT_DIMENSION) {
		if (unit == CSS_UNIT_EX) {
			length = FMUL(length, ex_size->value);
			unit = ex_size->unit;
		}

		error = set_line_height(style, type, length, unit);
		if (error != CSS_OK)
			return error;
	}

	return CSS_OK;
}

/**
 * Compute the absolute values of {top,right,bottom,left}
 *
 * \param style      Style to process
 * \param ex_size    Ex size, in ems
 * \return CSS_OK on success
 */
css_error compute_absolute_sides(css_computed_style *style,
		const css_hint_length *ex_size)
{
	css_error error;

	/* Calculate absolute lengths for sides */
	error = compute_absolute_length_auto(style, ex_size, get_top, set_top);
	if (error != CSS_OK)
		return error;

	error = compute_absolute_length_auto(style, ex_size,
			get_right, set_right);
	if (error != CSS_OK)
		return error;

	error = compute_absolute_length_auto(style, ex_size,
			get_bottom, set_bottom);
	if (error != CSS_OK)
		return error;

	error = compute_absolute_length_auto(style, ex_size,
			get_left, set_left);
	if (error != CSS_OK)
		return error;

	return CSS_OK;
}

/**
 * Compute absolute margins
 *
 * \param style      Style to process
 * \param ex_size    Ex size, in ems
 * \return CSS_OK on success
 */
css_error compute_absolute_margins(css_computed_style *style,
		const css_hint_length *ex_size)
{
	css_error error;

	error = compute_absolute_length_auto(style, ex_size,
			get_margin_top, set_margin_top);
	if (error != CSS_OK)
		return error;

	error = compute_absolute_length_auto(style, ex_size,
			get_margin_right, set_margin_right);
	if (error != CSS_OK)
		return error;

	error = compute_absolute_length_auto(style, ex_size,
			get_margin_bottom, set_margin_bottom);
	if (error != CSS_OK)
		return error;

	error = compute_absolute_length_auto(style, ex_size,
			get_margin_left, set_margin_left);
	if (error != CSS_OK)
		return error;
	
	return CSS_OK;
}

/**
 * Compute absolute padding
 *
 * \param style      Style to process
 * \param ex_size    Ex size, in ems
 * \return CSS_OK on success
 */
css_error compute_absolute_padding(css_computed_style *style,
		const css_hint_length *ex_size)
{
	css_error error;

	error = compute_absolute_length(style, ex_size,
			get_padding_top, set_padding_top);
	if (error != CSS_OK)
		return error;

	error = compute_absolute_length(style, ex_size,
			get_padding_right, set_padding_right);
	if (error != CSS_OK)
		return error;

	error = compute_absolute_length(style, ex_size,
			get_padding_bottom, set_padding_bottom);
	if (error != CSS_OK)
		return error;

	error = compute_absolute_length(style, ex_size,
			get_padding_left, set_padding_left);
	if (error != CSS_OK)
		return error;
	
	return CSS_OK;
}

/**
 * Compute absolute vertical-align
 *
 * \param style      Style to process
 * \param ex_size    Ex size, in ems
 * \return CSS_OK on success
 */
css_error compute_absolute_vertical_align(css_computed_style *style,
		const css_hint_length *ex_size)
{
	css_fixed length = 0;
	css_unit unit = CSS_UNIT_PX;
	uint8_t type;
	css_error error;

	type = get_vertical_align(style, &length, &unit);

	if (type == CSS_VERTICAL_ALIGN_SET) {
		if (unit == CSS_UNIT_EX) {
			length = FMUL(length, ex_size->value);
			unit = ex_size->unit;
		}

		error = set_vertical_align(style, type, length, unit);
		if (error != CSS_OK)
			return error;
	}

	return CSS_OK;
}

/**
 * Compute the absolute value of length
 *
 * \param style      Style to process
 * \param ex_size    Ex size, in ems
 * \param get        Function to read length
 * \param set        Function to write length
 * \return CSS_OK on success
 */
css_error compute_absolute_length(css_computed_style *style,
		const css_hint_length *ex_size,
		uint8_t (*get)(const css_computed_style *style, 
				css_fixed *len, css_unit *unit),
		css_error (*set)(css_computed_style *style, uint8_t type,
				css_fixed len, css_unit unit))
{
	css_fixed length;
	css_unit unit;
	uint8_t type;

	type = get(style, &length, &unit);

	if (unit == CSS_UNIT_EX) {
		length = FMUL(length, ex_size->value);
		unit = ex_size->unit;
	}

	return set(style, type, length, unit);
}

/**
 * Compute the absolute value of length or auto
 *
 * \param style      Style to process
 * \param ex_size    Ex size, in ems
 * \param get        Function to read length
 * \param set        Function to write length
 * \return CSS_OK on success
 */
css_error compute_absolute_length_auto(css_computed_style *style,
		const css_hint_length *ex_size,
		uint8_t (*get)(const css_computed_style *style, 
				css_fixed *len, css_unit *unit),
		css_error (*set)(css_computed_style *style, uint8_t type,
				css_fixed len, css_unit unit))
{
	css_fixed length;
	css_unit unit;
	uint8_t type;

	type = get(style, &length, &unit);
	if (type != CSS_BOTTOM_AUTO) {
		if (unit == CSS_UNIT_EX) {
			length = FMUL(length, ex_size->value);
			unit = ex_size->unit;
		}

		return set(style, CSS_BOTTOM_SET, length, unit);
	}

	return set(style, CSS_BOTTOM_AUTO, 0, CSS_UNIT_PX);
}

/**
 * Compute the absolute value of length or none
 *
 * \param style      Style to process
 * \param ex_size    Ex size, in ems
 * \param get        Function to read length
 * \param set        Function to write length
 * \return CSS_OK on success
 */
css_error compute_absolute_length_none(css_computed_style *style,
		const css_hint_length *ex_size,
		uint8_t (*get)(const css_computed_style *style, 
				css_fixed *len, css_unit *unit),
		css_error (*set)(css_computed_style *style, uint8_t type,
				css_fixed len, css_unit unit))
{
	css_fixed length;
	css_unit unit;
	uint8_t type;

	type = get(style, &length, &unit);
	if (type != CSS_MAX_HEIGHT_NONE) {
		if (unit == CSS_UNIT_EX) {
			length = FMUL(length, ex_size->value);
			unit = ex_size->unit;
		}

		return set(style, CSS_MAX_HEIGHT_SET, length, unit);
	}

	return set(style, CSS_MAX_HEIGHT_NONE, 0, CSS_UNIT_PX);
}

/**
 * Compute the absolute value of length or normal
 *
 * \param style      Style to process
 * \param ex_size    Ex size, in ems
 * \param get        Function to read length
 * \param set        Function to write length
 * \return CSS_OK on success
 */
css_error compute_absolute_length_normal(css_computed_style *style,
		const css_hint_length *ex_size,
		uint8_t (*get)(const css_computed_style *style, 
				css_fixed *len, css_unit *unit),
		css_error (*set)(css_computed_style *style, uint8_t type,
				css_fixed len, css_unit unit))
{
	css_fixed length;
	css_unit unit;
	uint8_t type;

	type = get(style, &length, &unit);
	if (type != CSS_LETTER_SPACING_NORMAL) {
		if (unit == CSS_UNIT_EX) {
			length = FMUL(length, ex_size->value);
			unit = ex_size->unit;
		}

		return set(style, CSS_LETTER_SPACING_SET, length, unit);
	}

	return set(style, CSS_LETTER_SPACING_NORMAL, 0, CSS_UNIT_PX);
}

/**
 * Compute the absolute value of length pair
 *
 * \param style      Style to process
 * \param ex_size    Ex size, in ems
 * \param get        Function to read length
 * \param set        Function to write length
 * \return CSS_OK on success
 */
css_error compute_absolute_length_pair(css_computed_style *style,
		const css_hint_length *ex_size,
		uint8_t (*get)(const css_computed_style *style, 
				css_fixed *len1, css_unit *unit1,
				css_fixed *len2, css_unit *unit2),
		css_error (*set)(css_computed_style *style, uint8_t type,
				css_fixed len1, css_unit unit1,
				css_fixed len2, css_unit unit2))
{
	css_fixed length1, length2;
	css_unit unit1, unit2;
	uint8_t type;

	type = get(style, &length1, &unit1, &length2, &unit2);

	if (unit1 == CSS_UNIT_EX) {
		length1 = FMUL(length1, ex_size->value);
		unit1 = ex_size->unit;
	}

	if (unit2 == CSS_UNIT_EX) {
		length2 = FMUL(length2, ex_size->value);
		unit2 = ex_size->unit;
	}

	return set(style, type, length1, unit1, length2, unit2);
}

