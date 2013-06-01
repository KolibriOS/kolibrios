/*
 * This file is part of LibParserUtils.
 * Licensed under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 * Copyright 2007-8 John-Mark Bell <jmb@netsurf-browser.org>
 */

#ifndef parserutils_functypes_h_
#define parserutils_functypes_h_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <parserutils/types.h>

/* Type of allocation function for parserutils */
typedef void *(*parserutils_alloc)(void *ptr, size_t size, void *pw);

#ifdef __cplusplus
}
#endif

#endif

