/*
 * This file is part of LibCSS.
 * Licensed under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 * Copyright 2007-8 John-Mark Bell <jmb@netsurf-browser.org>
 */

#ifndef libcss_functypes_h_
#define libcss_functypes_h_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <libcss/types.h>

/* Type of allocation function for libcss */
typedef void *(*css_allocator_fn)(void *ptr, size_t size, void *pw);

#ifdef __cplusplus
}
#endif

#endif

