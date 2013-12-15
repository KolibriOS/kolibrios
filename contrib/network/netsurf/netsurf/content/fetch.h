/*
 * Copyright 2003 James Bursa <bursa@users.sourceforge.net>
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
 * Fetching of data from a URL (interface).
 */

#ifndef _NETSURF_DESKTOP_FETCH_H_
#define _NETSURF_DESKTOP_FETCH_H_

#include <stdbool.h>

#include <libwapcaplet/libwapcaplet.h>

#include "utils/config.h"
#include "utils/nsurl.h"

struct content;
struct fetch;
struct ssl_cert_info;

typedef enum {
	FETCH_PROGRESS,
	FETCH_HEADER,
	FETCH_DATA,
	FETCH_FINISHED,
	FETCH_ERROR,
	FETCH_REDIRECT,
	FETCH_NOTMODIFIED,
	FETCH_AUTH,
	FETCH_CERT_ERR,
	FETCH_SSL_ERR
} fetch_msg_type;

typedef struct fetch_msg {
	fetch_msg_type type;

	union {
		const char *progress;

		struct {
			const uint8_t *buf;
			size_t len;
		} header_or_data;

		const char *error;

		/** \todo Use nsurl */
		const char *redirect;

		struct {
			const char *realm;
		} auth;

		struct {
			const struct ssl_cert_info *certs;
			size_t num_certs;
		} cert_err;
	} data;
} fetch_msg;

/** Fetch POST multipart data */
struct fetch_multipart_data {
	bool file;			/**< Item is a file */
	char *name;			/**< Name of item */
	char *value;			/**< Item value */

	struct fetch_multipart_data *next;	/**< Next in linked list */
};

struct ssl_cert_info {
	long version;		/**< Certificate version */
	char not_before[32];	/**< Valid from date */
	char not_after[32];	/**< Valid to date */
	int sig_type;		/**< Signature type */
	long serial;		/**< Serial number */
	char issuer[256];	/**< Issuer details */
	char subject[256];	/**< Subject details */
	int cert_type;		/**< Certificate type */
};

extern bool fetch_active;

typedef void (*fetch_callback)(const fetch_msg *msg, void *p);


void fetch_init(void);
struct fetch * fetch_start(nsurl *url, nsurl *referer,
		fetch_callback callback,
		void *p, bool only_2xx, const char *post_urlenc,
		const struct fetch_multipart_data *post_multipart,
		bool verifiable, bool downgrade_tls,
		const char *headers[]);
void fetch_abort(struct fetch *f);
void fetch_poll(void);
void fetch_quit(void);
const char *fetch_filetype(const char *unix_path);
char *fetch_mimetype(const char *ro_path);
bool fetch_can_fetch(const nsurl *url);
void fetch_change_callback(struct fetch *fetch,
                           fetch_callback callback,
                           void *p);
long fetch_http_code(struct fetch *fetch);
bool fetch_get_verifiable(struct fetch *fetch);

void fetch_multipart_data_destroy(struct fetch_multipart_data *list);
struct fetch_multipart_data *fetch_multipart_data_clone(
		const struct fetch_multipart_data *list);

/* API for fetchers themselves */

typedef bool (*fetcher_initialise)(lwc_string *scheme);
typedef bool (*fetcher_can_fetch)(const nsurl *url);
typedef void *(*fetcher_setup_fetch)(struct fetch *parent_fetch, nsurl *url,
		bool only_2xx, bool downgrade_tls, const char *post_urlenc,
		const struct fetch_multipart_data *post_multipart,
		const char **headers);
typedef bool (*fetcher_start_fetch)(void *fetch);
typedef void (*fetcher_abort_fetch)(void *fetch);
typedef void (*fetcher_free_fetch)(void *fetch);
typedef void (*fetcher_poll_fetcher)(lwc_string *scheme);
typedef void (*fetcher_finalise)(lwc_string *scheme);

/** Register a fetcher for a scheme
 *
 * \param scheme	scheme fetcher is for (caller relinquishes ownership)
 * \param initialiser	fetcher initialiser
 * \param can_fetch     fetcher can fetch function
 * \param setup_fetch	fetcher fetch setup function
 * \param start_fetch	fetcher fetch start function
 * \param abort_fetch	fetcher fetch abort function
 * \param free_fetch	fetcher fetch free function
 * \param poll_fetcher	fetcher poll function
 * \param finaliser	fetcher finaliser
 * \return true iff success
 */
bool fetch_add_fetcher(lwc_string *scheme,
                       fetcher_initialise initialiser,
                       fetcher_can_fetch can_fetch,
                       fetcher_setup_fetch setup_fetch,
                       fetcher_start_fetch start_fetch,
                       fetcher_abort_fetch abort_fetch,
                       fetcher_free_fetch free_fetch,
                       fetcher_poll_fetcher poll_fetcher,
                       fetcher_finalise finaliser);

void fetch_send_callback(const fetch_msg *msg, struct fetch *fetch);
void fetch_remove_from_queues(struct fetch *fetch);
void fetch_free(struct fetch *f);
void fetch_set_http_code(struct fetch *fetch, long http_code);
const char *fetch_get_referer_to_send(struct fetch *fetch);
void fetch_set_cookie(struct fetch *fetch, const char *data);

#endif
