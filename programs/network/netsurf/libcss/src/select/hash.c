/*
 * This file is part of LibCSS
 * Licensed under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 * Copyright 2009 John-Mark Bell <jmb@netsurf-browser.org>
 */

#include <stdio.h>
#include <string.h>

#include "stylesheet.h"
#include "select/hash.h"
#include "utils/utils.h"

typedef struct hash_entry {
	const css_selector *sel;
	struct hash_entry *next;
} hash_entry;

typedef struct hash_t {
#define DEFAULT_SLOTS (1<<6)
	size_t n_slots;

	hash_entry *slots;
} hash_t;

struct css_selector_hash {
	hash_t elements;

	hash_t classes;

	hash_t ids;

	hash_entry universal;

	size_t hash_size;

	css_allocator_fn alloc;
	void *pw;
};

static hash_entry empty_slot;

static inline uint32_t _hash_name(lwc_string *name);
static inline lwc_string *_class_name(const css_selector *selector);
static inline lwc_string *_id_name(const css_selector *selector);
static css_error _insert_into_chain(css_selector_hash *ctx, hash_entry *head, 
		const css_selector *selector);
static css_error _remove_from_chain(css_selector_hash *ctx, hash_entry *head,
		const css_selector *selector);

static css_error _iterate_elements(const css_selector **current,
		const css_selector ***next);
static css_error _iterate_classes(const css_selector **current,
		const css_selector ***next);
static css_error _iterate_ids(const css_selector **current,
		const css_selector ***next);
static css_error _iterate_universal(const css_selector **current,
		const css_selector ***next);

/**
 * Create a hash
 *
 * \param alloc  Memory (de)allocation function
 * \param pw     Pointer to client-specific private data
 * \param hash   Pointer to location to receive result
 * \return CSS_OK on success, appropriate error otherwise
 */
css_error css__selector_hash_create(css_allocator_fn alloc, void *pw, 
		css_selector_hash **hash)
{
	css_selector_hash *h;

	if (alloc == NULL || hash == NULL)
		return CSS_BADPARM;

	h = alloc(0, sizeof(css_selector_hash), pw);
	if (h == NULL)
		return CSS_NOMEM;

	/* Element hash */
	h->elements.slots = alloc(0, DEFAULT_SLOTS * sizeof(hash_entry), pw);
	if (h->elements.slots == NULL) {
		alloc(h, 0, pw);
		return CSS_NOMEM;
	}
	memset(h->elements.slots, 0, DEFAULT_SLOTS * sizeof(hash_entry));
	h->elements.n_slots = DEFAULT_SLOTS;

	/* Class hash */
	h->classes.slots = alloc(0, DEFAULT_SLOTS * sizeof(hash_entry), pw);
	if (h->classes.slots == NULL) {
		alloc(h->elements.slots, 0, pw);
		alloc(h, 0, pw);
		return CSS_NOMEM;
	}
	memset(h->classes.slots, 0, DEFAULT_SLOTS * sizeof(hash_entry));
	h->classes.n_slots = DEFAULT_SLOTS;

	/* ID hash */
	h->ids.slots = alloc(0, DEFAULT_SLOTS * sizeof(hash_entry), pw);
	if (h->ids.slots == NULL) {
		alloc(h->classes.slots, 0, pw);
		alloc(h->elements.slots, 0, pw);
		alloc(h, 0, pw);
		return CSS_NOMEM;
	}
	memset(h->ids.slots, 0, DEFAULT_SLOTS * sizeof(hash_entry));
	h->ids.n_slots = DEFAULT_SLOTS;

	/* Universal chain */
	memset(&h->universal, 0, sizeof(hash_entry));

	h->hash_size = sizeof(css_selector_hash) + 
			DEFAULT_SLOTS * sizeof(hash_entry) +
			DEFAULT_SLOTS * sizeof(hash_entry) +
			DEFAULT_SLOTS * sizeof(hash_entry);

	h->alloc = alloc;
	h->pw = pw;

	*hash = h;

	return CSS_OK;
}

/**
 * Destroy a hash
 *
 * \param hash  The hash to destroy
 * \return CSS_OK on success, appropriate error otherwise
 */
css_error css__selector_hash_destroy(css_selector_hash *hash)
{
	hash_entry *d, *e;
	uint32_t i;

	if (hash == NULL)
		return CSS_BADPARM;

	/* Element hash */
	for (i = 0; i < hash->elements.n_slots; i++) {
		for (d = hash->elements.slots[i].next; d != NULL; d = e) {
			e = d->next;

			hash->alloc(d, 0, hash->pw);
		}
	}
	hash->alloc(hash->elements.slots, 0, hash->pw);

	/* Class hash */
	for (i = 0; i < hash->classes.n_slots; i++) {
		for (d = hash->classes.slots[i].next; d != NULL; d = e) {
			e = d->next;

			hash->alloc(d, 0, hash->pw);
		}
	}
	hash->alloc(hash->classes.slots, 0, hash->pw);

	/* ID hash */
	for (i = 0; i < hash->ids.n_slots; i++) {
		for (d = hash->ids.slots[i].next; d != NULL; d = e) {
			e = d->next;

			hash->alloc(d, 0, hash->pw);
		}
	}
	hash->alloc(hash->ids.slots, 0, hash->pw);

	/* Universal chain */
	for (d = hash->universal.next; d != NULL; d = e) {
		e = d->next;

		hash->alloc(d, 0, hash->pw);
	}

	hash->alloc(hash, 0, hash->pw);

	return CSS_OK;
}

/**
 * Insert an item into a hash
 *
 * \param hash      The hash to insert into
 * \param selector  Pointer to selector
 * \return CSS_OK on success, appropriate error otherwise
 */
css_error css__selector_hash_insert(css_selector_hash *hash,
		const css_selector *selector)
{
	uint32_t index, mask;
	lwc_string *name;
	css_error error;

	if (hash == NULL || selector == NULL)
		return CSS_BADPARM;

	/* Work out which hash to insert into */
	if ((name = _id_name(selector)) != NULL) {
		/* Named ID */
		mask = hash->ids.n_slots - 1;
		index = _hash_name(name) & mask;

		error = _insert_into_chain(hash, &hash->ids.slots[index],
				selector);
	} else if ((name = _class_name(selector)) != NULL) {
		/* Named class */
		mask = hash->classes.n_slots - 1;
		index = _hash_name(name) & mask;

		error = _insert_into_chain(hash, &hash->classes.slots[index],
				selector);
	} else if (lwc_string_length(selector->data.qname.name) != 1 ||
			lwc_string_data(selector->data.qname.name)[0] != '*') {
		/* Named element */
		mask = hash->elements.n_slots - 1;
		index = _hash_name(selector->data.qname.name) & mask;

		error = _insert_into_chain(hash, &hash->elements.slots[index],
				selector);
	} else {
		/* Universal chain */
		error = _insert_into_chain(hash, &hash->universal, selector);
	}

	return error;
}

/**
 * Remove an item from a hash
 *
 * \param hash      The hash to remove from
 * \param selector  Pointer to selector
 * \return CSS_OK on success, appropriate error otherwise
 */
css_error css__selector_hash_remove(css_selector_hash *hash,
		const css_selector *selector)
{
	uint32_t index, mask;
	lwc_string *name;
	css_error error;

	if (hash == NULL || selector == NULL)
		return CSS_BADPARM;

	/* Work out which hash to remove from */
	if ((name = _id_name(selector)) != NULL) {
		/* Named ID */
		mask = hash->ids.n_slots - 1;
		index = _hash_name(name) & mask;

		error = _remove_from_chain(hash, &hash->ids.slots[index],
				selector);
	} else if ((name = _class_name(selector)) != NULL) {
		/* Named class */
		mask = hash->classes.n_slots - 1;
		index = _hash_name(name) & mask;

		error = _remove_from_chain(hash, &hash->classes.slots[index],
				selector);
	} else if (lwc_string_length(selector->data.qname.name) != 1 ||
			lwc_string_data(selector->data.qname.name)[0] != '*') {
		/* Named element */
		mask = hash->elements.n_slots - 1;
		index = _hash_name(selector->data.qname.name) & mask;

		error = _remove_from_chain(hash, &hash->elements.slots[index],
				selector);
	} else {
		/* Universal chain */
		error = _remove_from_chain(hash, &hash->universal, selector);
	}

	return error;
}

/**
 * Find the first selector that matches name
 *
 * \param hash      Hash to search
 * \param qname     Qualified name to match
 * \param iterator  Pointer to location to receive iterator function
 * \param matched   Pointer to location to receive selector
 * \return CSS_OK on success, appropriate error otherwise
 *
 * If nothing matches, CSS_OK will be returned and **matched == NULL
 */
css_error css__selector_hash_find(css_selector_hash *hash,
		css_qname *qname, 
		css_selector_hash_iterator *iterator,
		const css_selector ***matched)
{
	uint32_t index, mask;
	hash_entry *head;

	if (hash == NULL || qname == NULL || iterator == NULL || matched == NULL)
		return CSS_BADPARM;

	/* Find index */
	mask = hash->elements.n_slots - 1;
	index = _hash_name(qname->name) & mask;

	head = &hash->elements.slots[index];

	if (head->sel != NULL) {
		/* Search through chain for first match */
		while (head != NULL) {
			lwc_error lerror;
			bool match = false;

			lerror = lwc_string_caseless_isequal(
					qname->name, head->sel->data.qname.name,
					&match);
			if (lerror != lwc_error_ok)
				return css_error_from_lwc_error(lerror);

			if (match)
				break;

			head = head->next;
		}

		if (head == NULL)
			head = &empty_slot;
	}

	(*iterator) = _iterate_elements;
	(*matched) = (const css_selector **) head;

	return CSS_OK;
}

/**
 * Find the first selector that has a class that matches name
 *
 * \param hash      Hash to search
 * \param name      Name to match
 * \param iterator  Pointer to location to receive iterator function
 * \param matched   Pointer to location to receive selector
 * \return CSS_OK on success, appropriate error otherwise
 *
 * If nothing matches, CSS_OK will be returned and **matched == NULL
 */
css_error css__selector_hash_find_by_class(css_selector_hash *hash,
		lwc_string *name, 
		css_selector_hash_iterator *iterator,
		const css_selector ***matched)
{
	uint32_t index, mask;
	hash_entry *head;

	if (hash == NULL || name == NULL || iterator == NULL || matched == NULL)
		return CSS_BADPARM;

	/* Find index */
	mask = hash->classes.n_slots - 1;
	index = _hash_name(name) & mask;

	head = &hash->classes.slots[index];

	if (head->sel != NULL) {
		/* Search through chain for first match */
		while (head != NULL) {
			lwc_error lerror;
			lwc_string *n;
			bool match = false;

			n = _class_name(head->sel);
			if (n != NULL) {
				lerror = lwc_string_caseless_isequal(
						name, n, &match);
				if (lerror != lwc_error_ok)
					return css_error_from_lwc_error(lerror);

				if (match)
					break;
			}

			head = head->next;
		}

		if (head == NULL)
			head = &empty_slot;
	}

	(*iterator) = _iterate_classes;
	(*matched) = (const css_selector **) head;

	return CSS_OK;
}

/**
 * Find the first selector that has an ID that matches name
 *
 * \param hash      Hash to search
 * \param name      Name to match
 * \param iterator  Pointer to location to receive iterator function
 * \param matched   Pointer to location to receive selector
 * \return CSS_OK on success, appropriate error otherwise
 *
 * If nothing matches, CSS_OK will be returned and **matched == NULL
 */
css_error css__selector_hash_find_by_id(css_selector_hash *hash,
		lwc_string *name, 
		css_selector_hash_iterator *iterator,
		const css_selector ***matched)
{
	uint32_t index, mask;
	hash_entry *head;

	if (hash == NULL || name == NULL || iterator == NULL || matched == NULL)
		return CSS_BADPARM;

	/* Find index */
	mask = hash->ids.n_slots - 1;
	index = _hash_name(name) & mask;

	head = &hash->ids.slots[index];

	if (head->sel != NULL) {
		/* Search through chain for first match */
		while (head != NULL) {
			lwc_error lerror;
			lwc_string *n;
			bool match = false;

			n = _id_name(head->sel);
			if (n != NULL) {
				lerror = lwc_string_caseless_isequal(
						name, n, &match);
				if (lerror != lwc_error_ok)
					return css_error_from_lwc_error(lerror);

				if (match)
					break;
			}

			head = head->next;
		}

		if (head == NULL)
			head = &empty_slot;
	}

	(*iterator) = _iterate_ids;
	(*matched) = (const css_selector **) head;

	return CSS_OK;
}

/**
 * Find the first universal selector
 *
 * \param hash      Hash to search
 * \param iterator  Pointer to location to receive iterator function
 * \param matched   Pointer to location to receive selector
 * \return CSS_OK on success, appropriate error otherwise
 *
 * If nothing matches, CSS_OK will be returned and **matched == NULL
 */
css_error css__selector_hash_find_universal(css_selector_hash *hash,
		css_selector_hash_iterator *iterator,
		const css_selector ***matched)
{
	if (hash == NULL || iterator == NULL || matched == NULL)
		return CSS_BADPARM;

	(*iterator) = _iterate_universal;
	(*matched) = (const css_selector **) &hash->universal;

	return CSS_OK;
}

/**
 * Determine the memory-resident size of a hash
 *
 * \param hash  Hash to consider
 * \param size  Pointer to location to receive byte count
 * \return CSS_OK on success.
 *
 * \note The returned size will represent the size of the hash datastructures,
 *       and will not include the size of the data stored in the hash.
 */
css_error css__selector_hash_size(css_selector_hash *hash, size_t *size)
{
	if (hash == NULL || size == NULL)
		return CSS_BADPARM;

	*size = hash->hash_size;

	return CSS_OK;
}

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/**
 * Name hash function -- case-insensitive FNV.
 *
 * \param name  Name to hash
 * \return hash value
 */
uint32_t _hash_name(lwc_string *name)
{
	uint32_t z = 0x811c9dc5;
	const char *data = lwc_string_data(name);
	const char *end = data + lwc_string_length(name);

	while (data != end) {
		const char c = *data++;

		z *= 0x01000193;
		z ^= c & ~0x20;
	}

	return z;
}

/**
 * Retrieve the first class name in a selector, or NULL if none
 *
 * \param selector  Selector to consider
 * \return Pointer to class name, or NULL if none
 */
lwc_string *_class_name(const css_selector *selector)
{
	const css_selector_detail *detail = &selector->data;
	lwc_string *name = NULL;

	do {
		/* Ignore :not(.class) */
		if (detail->type == CSS_SELECTOR_CLASS && detail->negate == 0) {
			name = detail->qname.name;
			break;
		}

		if (detail->next)
			detail++;
		else
			detail = NULL;
	} while (detail != NULL);

	return name;
}

/**
 * Retrieve the first ID name in a selector, or NULL if none
 *
 * \param selector  Selector to consider
 * \return Pointer to ID name, or NULL if none
 */
lwc_string *_id_name(const css_selector *selector)
{
	const css_selector_detail *detail = &selector->data;
	lwc_string *name = NULL;

	do {
		/* Ignore :not(#id) */
		if (detail->type == CSS_SELECTOR_ID && detail->negate == 0) {
			name = detail->qname.name;
			break;
		}

		if (detail->next)
			detail++;
		else
			detail = NULL;
	} while (detail != NULL);

	return name;
}

/**
 * Insert a selector into a hash chain
 *
 * \param ctx       Selector hash
 * \param head      Head of chain to insert into
 * \param selector  Selector to insert
 * \return CSS_OK    on success,
 *         CSS_NOMEM on memory exhaustion.
 */
css_error _insert_into_chain(css_selector_hash *ctx, hash_entry *head, 
		const css_selector *selector)
{
	if (head->sel == NULL) {
		head->sel = selector;
		head->next = NULL;
	} else {
		hash_entry *search = head;
		hash_entry *prev = NULL;
		hash_entry *entry = 
				ctx->alloc(NULL, sizeof(hash_entry), ctx->pw);
		if (entry == NULL)
			return CSS_NOMEM;

		/* Find place to insert entry */
		do {
			/* Sort by ascending specificity */
			if (search->sel->specificity > selector->specificity)
				break;

			/* Sort by ascending rule index */
			if (search->sel->specificity == selector->specificity &&
					search->sel->rule->index > 
					selector->rule->index)
				break;

			prev = search;
			search = search->next;
		} while (search != NULL);

		if (prev == NULL) {
			entry->sel = head->sel;
			entry->next = head->next;
			head->sel = selector;
			head->next = entry;
		} else {
			entry->sel = selector;
			entry->next = prev->next;
			prev->next = entry;
		}

		ctx->hash_size += sizeof(hash_entry);
	}

	return CSS_OK;
}

/**
 * Remove a selector from a hash chain
 *
 * \param ctx       Selector hash
 * \param head      Head of chain to remove from
 * \param selector  Selector to remove
 * \return CSS_OK       on success,
 *         CSS_INVALID  if selector not found in chain.
 */
css_error _remove_from_chain(css_selector_hash *ctx, hash_entry *head,
		const css_selector *selector)
{
	hash_entry *search = head, *prev = NULL;

	if (head->sel == NULL)
		return CSS_INVALID;

	do {
		if (search->sel == selector)
			break;

		prev = search;
		search = search->next;
	} while (search != NULL);

	if (search == NULL)
		return CSS_INVALID;

	if (prev == NULL) {
		if (search->next != NULL) {
			head->sel = search->next->sel;
			head->next = search->next->next;
		} else {
			head->sel = NULL;
			head->next = NULL;
		}
	} else {
		prev->next = search->next;

		ctx->alloc(search, 0, ctx->pw);

		ctx->hash_size -= sizeof(hash_entry);
	}

	return CSS_OK;
}

/**
 * Find the next selector that matches
 *
 * \param current  Current item
 * \param next     Pointer to location to receive next item
 * \return CSS_OK on success, appropriate error otherwise
 *
 * If nothing further matches, CSS_OK will be returned and **next == NULL
 */
css_error _iterate_elements(const css_selector **current,
		const css_selector ***next)
{
	const hash_entry *head = (const hash_entry *) current;
	bool match = false;
	lwc_error lerror = lwc_error_ok;
	lwc_string *name;

	name = head->sel->data.qname.name;

	/* Look for the next selector that matches the key */
	while (match == false && (head = head->next) != NULL) {
		lerror = lwc_string_caseless_isequal(
				name, head->sel->data.qname.name, &match);
		if (lerror != lwc_error_ok)
			return css_error_from_lwc_error(lerror);
	}

	if (head == NULL)
		head = &empty_slot;

	(*next) = (const css_selector **) head;

	return CSS_OK;
}

/**
 * Find the next selector that matches
 *
 * \param current  Current item
 * \param next     Pointer to location to receive next item
 * \return CSS_OK on success, appropriate error otherwise
 *
 * If nothing further matches, CSS_OK will be returned and **next == NULL
 */
css_error _iterate_classes(const css_selector **current,
		const css_selector ***next)
{
	const hash_entry *head = (const hash_entry *) current;
	bool match = false;
	lwc_error lerror = lwc_error_ok;
	lwc_string *name, *ref;

	ref = _class_name(head->sel);

	/* Look for the next selector that matches the key */
	while (match == false && (head = head->next) != NULL) {
		name = _class_name(head->sel);
		if (name == NULL)
			continue;

		lerror = lwc_string_caseless_isequal(
				ref, name, &match);
		if (lerror != lwc_error_ok)
			return css_error_from_lwc_error(lerror);
	}

	if (head == NULL)
		head = &empty_slot;

	(*next) = (const css_selector **) head;

	return CSS_OK;
}

/**
 * Find the next selector that matches
 *
 * \param current  Current item
 * \param next     Pointer to location to receive next item
 * \return CSS_OK on success, appropriate error otherwise
 *
 * If nothing further matches, CSS_OK will be returned and **next == NULL
 */
css_error _iterate_ids(const css_selector **current,
		const css_selector ***next)
{
	const hash_entry *head = (const hash_entry *) current;
	bool match = false;
	lwc_error lerror = lwc_error_ok;
	lwc_string *name, *ref;

	ref = _id_name(head->sel);

	/* Look for the next selector that matches the key */
	while (match == false && (head = head->next) != NULL) {
		name = _id_name(head->sel);
		if (name == NULL)
			continue;

		lerror = lwc_string_caseless_isequal(
				ref, name, &match);
		if (lerror != lwc_error_ok)
			return css_error_from_lwc_error(lerror);
	}

	if (head == NULL)
		head = &empty_slot;

	(*next) = (const css_selector **) head;

	return CSS_OK;
}

/**
 * Find the next selector that matches
 *
 * \param current  Current item
 * \param next     Pointer to location to receive next item
 * \return CSS_OK on success, appropriate error otherwise
 *
 * If nothing further matches, CSS_OK will be returned and **next == NULL
 */
css_error _iterate_universal(const css_selector **current,
		const css_selector ***next)
{
	const hash_entry *head = (const hash_entry *) current;

	if (head->next == NULL)
		head = &empty_slot;
	else
		head = head->next;

	(*next) = (const css_selector **) head;

	return CSS_OK;
}

