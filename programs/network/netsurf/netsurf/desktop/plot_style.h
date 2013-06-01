/*
 * Copyright 2004 James Bursa <bursa@users.sourceforge.net>
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
 * Plotter styles.
 */

#ifndef _NETSURF_DESKTOP_PLOT_STYLE_H_
#define _NETSURF_DESKTOP_PLOT_STYLE_H_

#include <stdint.h>

/* html widget colours */
#define WIDGET_BASEC 0xd9d9d9
#define WIDGET_BLOBC 0x000000

/* Darken a colour by taking three quarters of each channel's intensity */
#define darken_colour(c1)				 		\
	((((3 * ((c1 >> 16) & 0xff)) >> 2) << 16) |			\
	 (((3 * ((c1 >>  8) & 0xff)) >> 2) <<  8) |	 		\
	 (((3 * ( c1        & 0xff)) >> 2) <<  0))

/* Darken a colour by taking nine sixteenths of each channel's intensity */
#define double_darken_colour(c1)			 		\
	((((9 * ((c1 >> 16) & 0xff)) >> 4) << 16) |			\
	 (((9 * ((c1 >>  8) & 0xff)) >> 4) <<  8) |	 		\
	 (((9 * ( c1        & 0xff)) >> 4) <<  0))

/* Lighten a colour by taking three quarters of each channel's intensity
 * and adding a full quarter
 */
#define lighten_colour(c1)						\
	(((((3 * ((c1 >> 16) & 0xff)) >> 2) + 64) << 16) |		\
	 ((((3 * ((c1 >>  8) & 0xff)) >> 2) + 64) <<  8) |		\
	 ((((3 * ( c1        & 0xff)) >> 2) + 64) <<  0))

/* Lighten a colour by taking nine sixteenths of each channel's intensity and
 * adding a full intensity 7/16ths */
#define double_lighten_colour(c1)					\
	(((((9 * ((c1 >> 16) & 0xff)) >> 4) + 112) << 16) |		\
	 ((((9 * ((c1 >>  8) & 0xff)) >> 4) + 112) <<  8) |		\
	 ((((9 * ( c1        & 0xff)) >> 4) + 112) <<  0))

/* Blend two colours by taking half the intensity of each channel in the first
 * colour and adding them to half the intensity of each channel in the second
 * colour */
#define blend_colour(c0, c1)						\
	(((((c0 >> 16) & 0xff) + ((c1 >> 16) & 0xff)) >> 1) << 16) |	\
	(((((c0 >>  8) & 0xff) + ((c1 >>  8) & 0xff)) >> 1) <<  8) |	\
	(((( c0        & 0xff) + ( c1        & 0xff)) >> 1) <<  0)

/* get a bitmap pixel (image/bitmap.h) into a plot colour */
#define pixel_to_colour(b)					\
	b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24)

/**
 * Colour type: XBGR
 */
typedef uint32_t colour;

/**
 * Magical transparent value
 */
#define NS_TRANSPARENT 0x01000000

/**
 * Type of plot operation
 */
typedef enum {
	PLOT_OP_TYPE_NONE = 0, /**< No operation */
	PLOT_OP_TYPE_SOLID, /**< Solid colour */
	PLOT_OP_TYPE_DOT, /**< Dotted plot */
	PLOT_OP_TYPE_DASH, /**< Dashed plot */
} plot_operation_type_t;

/**
 * Plot style for stroke/fill plotters
 */
typedef struct {
	plot_operation_type_t stroke_type; /**< Stroke plot type */
	int stroke_width; /**< Width of stroke, in pixels */
	colour stroke_colour; /**< Colour of stroke */
	plot_operation_type_t fill_type; /**< Fill plot type */
	colour fill_colour; /**< Colour of fill */
} plot_style_t;

/**
 * Generic font family type
 */
typedef enum {
	PLOT_FONT_FAMILY_SANS_SERIF = 0,
	PLOT_FONT_FAMILY_SERIF,
	PLOT_FONT_FAMILY_MONOSPACE,
	PLOT_FONT_FAMILY_CURSIVE,
	PLOT_FONT_FAMILY_FANTASY,
	PLOT_FONT_FAMILY_COUNT /**< Number of generic families */
} plot_font_generic_family_t;

/**
 * Font plot flags
 */
typedef unsigned long plot_font_flags_t;
#define FONTF_NONE 0
#define FONTF_ITALIC 1
#define FONTF_OBLIQUE 2
#define FONTF_SMALLCAPS 4

/**
 * Scaling factor for font sizes
 */
#define FONT_SIZE_SCALE 1024

/**
 * Font style for plotting
 */
typedef struct {
	plot_font_generic_family_t family; /**< Generic family to plot with */
	int size; /**< Font size, in points * FONT_SIZE_SCALE */
	int weight; /**< Font weight: value in range [100,900] as per CSS */
	plot_font_flags_t flags; /**< Font flags */
	colour background; /**< Background colour to blend to, if appropriate */
	colour foreground; /**< Colour of text */
} plot_font_style_t;

/* global fill styles */
extern plot_style_t *plot_style_fill_white;
extern plot_style_t *plot_style_fill_red;
extern plot_style_t *plot_style_fill_black;

/* Box model debug outline styles for content, padding and margin edges */
extern plot_style_t const * const plot_style_content_edge;
extern plot_style_t const * const plot_style_padding_edge;
extern plot_style_t const * const plot_style_margin_edge;

/* Broken object replacement styles */
extern plot_style_t const * const plot_style_broken_object;
extern plot_font_style_t const * const plot_fstyle_broken_object;


/* other styles */
extern plot_style_t *plot_style_caret;
extern plot_style_t *plot_style_stroke_history;
extern plot_style_t *plot_style_fill_wbasec;
extern plot_style_t *plot_style_fill_darkwbasec;
extern plot_style_t *plot_style_fill_lightwbasec;
extern plot_style_t *plot_style_fill_wblobc;
extern plot_style_t *plot_style_stroke_wblobc;
extern plot_style_t *plot_style_stroke_darkwbasec;
extern plot_style_t *plot_style_stroke_lightwbasec;

/* Default font style */
extern plot_font_style_t const * const plot_style_font;

#ifndef HISTORY_COLOUR_SELECTED
#define HISTORY_COLOUR_SELECTED 0xFF0000
#endif

#ifndef HISTORY_COLOUR_FOREGROUND
#define HISTORY_COLOUR_FOREGROUND 0x333333
#endif

#ifndef HISTORY_COLOUR_BACKGROUND
#define HISTORY_COLOUR_BACKGROUND 0xFFFFFF
#endif

#ifndef HISTORY_COLOUR_LINES
#define HISTORY_COLOUR_LINES HISTORY_COLOUR_FOREGROUND
#endif

#endif
