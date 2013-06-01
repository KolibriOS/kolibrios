/*
 * This file is part of LibCSS
 * Licensed under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 * Copyright 2009 John-Mark Bell <jmb@netsurf-browser.org>
 */

#ifndef css_select_hash_h_
#define css_select_hash_h_

#include <libwapcaplet/libwapcaplet.h>

#include <libcss/errors.h>
#include <libcss/functypes.h>

/* Ugh. We need this to avoid circular includes. Happy! */
struct css_selector;

typedef struct css_selector_hash css_selector_hash;

typedef css_error (*css_selector_hash_iterator)(
		const struct css_selector **current,
		const struct css_selector ***next);

css_error css__selector_hash_create(css_allocator_fn alloc, void *pw, 
		css_selector_hash **hash);
css_error css__selector_hash_destroy(css_selector_hash *hash);

css_error css__selector_hash_insert(css_selector_hash *hash,
		const struct css_selector *selector);
css_error css__selector_hash_remove(css_selector_hash *hash,
		const struct css_selector *selector);

css_error css__selector_hash_find(css_selector_hash *hash,
		css_qname *qname,
		css_selector_hash_iterator *iterator,
		const struct css_selector ***matched);
css_error css__selector_hash_find_by_class(css_selector_hash *hash,
		lwc_string *name,
		css_selector_hash_iterator *iterator,
		const struct css_selector ***matched);
css_error css__selector_hash_find_by_id(css_selector_hash *hash,
		lwc_string *name,
		css_selector_hash_iterator *iterator,
		const struct css_selector ***matched);
css_error css__selector_hash_find_universal(css_selector_hash *hash,
		css_selector_hash_iterator *iterator,
		const struct css_selector ***matched);

css_error css__selector_hash_size(css_selector_hash *hash, size_t *size);

#endif

