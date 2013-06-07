/*
 * Copyright 2006 Daniel Silverstone <dsilvers@digital-scurf.org>
 * Copyright 2007 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2003 Phil Mellor <monkeyson@users.sourceforge.net>
 *
 * This file is part of NetSurf.
 *
 * NetSurf is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * NetSurf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \file
 * Fetching of data from a URL (implementation).
 *
 * This implementation uses libcurl's 'multi' interface.
 *
 *
 * The CURL handles are cached in the curl_handle_ring. There are at most
 * ::max_cached_fetch_handles in this ring.
 */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <sys/stat.h>

#include <libwapcaplet/libwapcaplet.h>

#include "utils/config.h"
//#include <openssl/ssl.h>
#include "content/fetch.h"
#include "content/fetchers/curl.h"
#include "content/urldb.h"
#include "desktop/netsurf.h"
#include "desktop/options.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "utils/schedule.h"
#include "utils/utils.h"
#include "utils/ring.h"
#include "utils/useragent.h"

/* BIG FAT WARNING: This is here because curl doesn't give you an FD to
 * poll on, until it has processed a bit of the handle.	 So we need schedules
 * in order to make this work.
 */
#include <desktop/browser.h>

/* uncomment this to use scheduler based calling
#define FETCHER_CURLL_SCHEDULED 1
*/


struct fetch_curl_context {
	struct fetch_curl_context *r_next, *r_prev;

	struct fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */
	char *path; /**< The actual path to be used with open() */

	time_t file_etag; /**< Request etag for file (previous st.m_time) */
};

static struct fetch_curl_context *ring = NULL;


static bool fetch_curl_initialise(lwc_string *scheme); //here
static void fetch_curl_finalise(lwc_string *scheme); //here
static bool fetch_curl_can_fetch(const nsurl *url); //here
static void * fetch_curl_setup(struct fetch *parent_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, const char *post_urlenc,
		 const struct fetch_multipart_data *post_multipart,
		 const char **headers); //here
static bool fetch_curl_start(void *vfetch); //here 

static void fetch_curl_abort(void *vf); //here
static void fetch_curl_free(void *f); //here
static void fetch_curl_poll(lwc_string *scheme_ignored); //here


/**
 * Initialise the fetcher.
 *
 * Must be called once before any other function.
 */

void fetch_curl_register(void)
{

lwc_string *scheme;


LOG(("curl register\n"));

lwc_intern_string("http", SLEN("http"), &scheme);

	if (!fetch_add_fetcher(scheme,
				fetch_curl_initialise, //here
				fetch_curl_can_fetch, //here
				fetch_curl_setup,
				fetch_curl_start,
				fetch_curl_abort, //here
				fetch_curl_free, //here
#ifdef FETCHER_CURLL_SCHEDULED
				       NULL,
#else
				fetch_curl_poll, //here
#endif
				fetch_curl_finalise)) {  //here
			LOG(("Unable to register cURL fetcher for HTTP"));
		}
		
		
		lwc_intern_string("https", SLEN("https"), &scheme);

	if (!fetch_add_fetcher(scheme,
				fetch_curl_initialise,
				fetch_curl_can_fetch,
				fetch_curl_setup,
				fetch_curl_start,
				fetch_curl_abort,
				fetch_curl_free,
#ifdef FETCHER_CURLL_SCHEDULED
				       NULL,
#else
				fetch_curl_poll,
#endif
				fetch_curl_finalise)) {
			LOG(("Unable to register cURL fetcher for HTTPS"));
		}

}


/**
 * Initialise a cURL fetcher.
 */

bool fetch_curl_initialise(lwc_string *scheme)
{

LOG(("curl initi lwc\n"));
	return true; /* Always succeeds */
}


/**
 * Finalise a cURL fetcher
 */

void fetch_curl_finalise(lwc_string *scheme)
{
LOG(("curl finali\n"));
}

static bool fetch_curl_can_fetch(const nsurl *url)
{
	LOG(("curl can fetch\n"));
	return true; //let's lie a bit
}

/**
 * Start fetching data for the given URL.
 *
 * The function returns immediately. The fetch may be queued for later
 * processing.
 *
 * A pointer to an opaque struct curl_fetch_info is returned, which can be 
 * passed to fetch_abort() to abort the fetch at any time. Returns 0 if memory 
 * is exhausted (or some other fatal error occurred).
 *
 * The caller must supply a callback function which is called when anything
 * interesting happens. The callback function is first called with msg
 * FETCH_HEADER, with the header in data, then one or more times
 * with FETCH_DATA with some data for the url, and finally with
 * FETCH_FINISHED. Alternatively, FETCH_ERROR indicates an error occurred:
 * data contains an error message. FETCH_REDIRECT may replace the FETCH_HEADER,
 * FETCH_DATA, FETCH_FINISHED sequence if the server sends a replacement URL.
 *
 * Some private data can be passed as the last parameter to fetch_start, and
 * callbacks will contain this.
 */

void * fetch_curl_setup (struct fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 const char *post_urlenc,
		 const struct fetch_multipart_data *post_multipart,
		 const char **headers)
{

	LOG(("curl setup\n"));
	
	struct fetch_curl_context *ctx;
	int i;

	ctx = calloc(1, sizeof(*ctx));
	if (ctx == NULL)
		return NULL;

	//ctx->path = url_to_path(nsurl_access(url));
	char *zz;
	int pr;
	nsurl_get(url, NSURL_WITH_FRAGMENT, &zz, &pr);
	
	ctx->path = zz;
	if (ctx->path == NULL) {
		free(ctx);
		return NULL;
	}

	ctx->url = nsurl_ref(url);


	ctx->fetchh = fetchh;

	RING_INSERT(ring, ctx);

	return ctx;
}


/**
 * Dispatch a single job
 */
bool fetch_curl_start(void *vfetch)
{
LOG(("curl start\n"));
	return true;
}






/**
 * Abort a fetch.
 */

void fetch_curl_abort(void *ctx)
{
	struct fetch_curl_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/**
 * Free a fetch structure and associated resources.
 */

void fetch_curl_free(void *ctx)
{
	struct fetch_curl_context *c = ctx;
	nsurl_unref(c->url);
	free(c->path);
	RING_REMOVE(ring, c);
	free(ctx);
}

static inline bool fetch_curl_send_callback(const fetch_msg *msg,
		struct fetch_curl_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(msg, ctx->fetchh);
	ctx->locked = false;

	return ctx->aborted;
}

static bool fetch_curl_send_header(struct fetch_curl_context *ctx,
		const char *fmt, ...)
{
	fetch_msg msg;
	char header[64];
	va_list ap;

	va_start(ap, fmt);

	vsnprintf(header, sizeof header, fmt, ap);

	va_end(ap);

	msg.type = FETCH_HEADER;
	msg.data.header_or_data.buf = (const uint8_t *) header;
	msg.data.header_or_data.len = strlen(header);
	fetch_curl_send_callback(&msg, ctx);

	return ctx->aborted;
}

static void fetch_curl_process_error(struct fetch_curl_context *ctx, int code)
{
	fetch_msg msg;
	char buffer[1024];
	const char *title;
	char key[8];

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	/* content type */
	if (fetch_curl_send_header(ctx, "Content-Type: text/html"))
		goto fetch_file_process_error_aborted;

	snprintf(key, sizeof key, "HTTP%03d", code);
	title = messages_get(key);

	snprintf(buffer, sizeof buffer, "<html><head><title>%s</title></head>"
			"<body><h1>%s</h1>"
			"<p>Error %d while fetching file %s</p></body></html>",
			title, title, code, nsurl_access(ctx->url));

	msg.type = FETCH_DATA;
	msg.data.header_or_data.buf = (const uint8_t *) buffer;
	msg.data.header_or_data.len = strlen(buffer);
	if (fetch_curl_send_callback(&msg, ctx))
		goto fetch_file_process_error_aborted;

	msg.type = FETCH_FINISHED;
	fetch_curl_send_callback(&msg, ctx);

fetch_file_process_error_aborted:
	return;
}


int is_pid(int k)
{
	int error;
asm volatile ("int $0x40":"=a"(error):"a"(18), "b"(21), "c"(k));
return error;
}


int kill_pid(int k)
{
	int error;
asm volatile ("int $0x40":"=a"(error):"a"(18), "b"(18), "c"(k));
return error;
}

static void fetch_curl_process(struct fetch_curl_context *ctx) {
	
	int pid=execl ("/sys/network/downloader", ctx->path, 0);
	
	
	
//	while (is_pid(pid)) {
	kill_pid(pid);
//	}
	
	pid=execl ("/sys/network/downloader", ctx->path, 0);
	
	char ps[255];
	sprintf(ps, "pid %d", pid);
	execl ("/sys/network/@notify", ps, 0);
	
	
	while (is_pid(pid));
	
	
	sprintf(ps, "Yay! Finished");
	execl ("/sys/network/@notify", ps, 0);
	
	
	/*
	char pzz[255];
	sprintf(pzz, "Pid is %d", pid);
	execl ("/sys/@notify", pzz, 0); */
	//int status;
	//waitpid(pid, &status, 0); 
	
	
	fetch_msg msg;
	//const char * buf = "<html><body><h1>Hello, file fetcher!</h1></body></html>";

 FILE *infile;
  infile = fopen("/sys/.download", "rb");
 
  if (infile == NULL) {
    printf("file does not exist.\n");
    return -1;
  }
 
  fseek(infile, 0, SEEK_END);
  size_t file_size = ftell(infile);
  rewind(infile);
 
  char *buffer = (char*)malloc(file_size * sizeof(char));
  if (buffer == NULL) {
    fclose(infile);
    printf("Error allocating %d bytes.\n", file_size * sizeof(char));
    return -1;
  }
  size_t bytes_read = fread(buffer, sizeof(char), file_size, infile);
  if (bytes_read != file_size) {
    printf("Have read only %d bytes of %d.\n", bytes_read, file_size);
    free(buffer);
    fclose(infile);
    return -1;
  }
  fclose(infile);
  
 
	

/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	__menuet__debug_out(fetch_filetype(ctx->path));
	__menuet__debug_out("\n");
	
	if (fetch_curl_send_header(ctx, "Content-Type: %s", 
			fetch_filetype(ctx->path)))
		goto fetch_file_process_aborted;

	
	/* main data loop */

		msg.type = FETCH_DATA;
		msg.data.header_or_data.buf = (const uint8_t *) buffer;
		msg.data.header_or_data.len = file_size;
		fetch_curl_send_callback(&msg, ctx);
			
	

	if (ctx->aborted == false) {
		msg.type = FETCH_FINISHED;
		fetch_curl_send_callback(&msg, ctx);
	}

fetch_file_process_aborted:
return;

}


/**
 * Do some work on current fetches.
 *
 * Must be called regularly to make progress on fetches.
 */

void fetch_curl_poll(lwc_string *scheme_ignored)
{
	LOG(("curl poll\n"));
	
	struct fetch_curl_context *c, *next;

	if (ring == NULL) return;

	/* Iterate over ring, processing each pending fetch */
	c = ring;
	do {
		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		if (c->locked == true) {
			next = c->r_next;
			continue;
		}

		/* Only process non-aborted fetches */
		if (c->aborted == false) {
			/* file fetches can be processed in one go */
			fetch_curl_process(c);
		}

		/* Compute next fetch item at the last possible moment as
		 * processing this item may have added to the ring.
		 */
		next = c->r_next;

		fetch_remove_from_queues(c->fetchh);
		fetch_free(c->fetchh);

		/* Advance to next ring entry, exiting if we've reached
		 * the start of the ring or the ring has become empty
		 */
	} while ( (c = next) != ring && ring != NULL);

}




