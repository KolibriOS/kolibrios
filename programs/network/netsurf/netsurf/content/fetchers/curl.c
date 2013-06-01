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

typedef int X509_STORE_CTX;
typedef int X509;
#define CURL_ERROR_SIZE 0
typedef int CURLcode;
typedef int curl_infotype;
/** SSL certificate info */
struct cert_info {
	X509 *cert;		/**< Pointer to certificate */
	long err;		/**< OpenSSL error code */
};

/** Information for a single fetch. */
struct curl_fetch_info {
	struct fetch *fetch_handle; /**< The fetch handle we're parented by. */
	CURL * curl_handle;	/**< cURL handle if being fetched, or 0. */
	bool had_headers;	/**< Headers have been processed. */
	bool abort;		/**< Abort requested. */
	bool stopped;		/**< Download stopped on purpose. */
	bool only_2xx;		/**< Only HTTP 2xx responses acceptable. */
	bool downgrade_tls;	/**< Downgrade to TLS <= 1.0 */
	nsurl *url;		/**< URL of this fetch. */
	lwc_string *host;	/**< The hostname of this fetch. */
	struct curl_slist *headers;	/**< List of request headers. */
	char *location;		/**< Response Location header, or 0. */
	unsigned long content_length;	/**< Response Content-Length, or 0. */
	char *cookie_string;	/**< Cookie string for this fetch */
	char *realm;		/**< HTTP Auth Realm */
	char *post_urlenc;	/**< Url encoded POST string, or 0. */
	long http_code; /**< HTTP result code from cURL. */
	struct curl_httppost *post_multipart;	/**< Multipart post data, or 0. */
#define MAX_CERTS 10
	struct cert_info cert_data[MAX_CERTS];	/**< HTTPS certificate data */
	unsigned int last_progress_update;	/**< Time of last progress update */
};

struct cache_handle {
	CURL *handle; /**< The cached cURL handle */
	lwc_string *host;   /**< The host for which this handle is cached */

	struct cache_handle *r_prev; /**< Previous cached handle in ring. */
	struct cache_handle *r_next; /**< Next cached handle in ring. */
};

CURLM *fetch_curl_multi;		/**< Global cURL multi handle. */
/** Curl handle with default options set; not used for transfers. */
static CURL *fetch_blank_curl;
static struct cache_handle *curl_handle_ring = 0; /**< Ring of cached handles */
static int curl_fetchers_registered = 0;
static bool curl_with_openssl;

static char fetch_error_buffer[CURL_ERROR_SIZE]; /**< Error buffer for cURL. */
static char fetch_proxy_userpwd[100];	/**< Proxy authentication details. */

static bool fetch_curl_initialise(lwc_string *scheme);
static void fetch_curl_finalise(lwc_string *scheme);
static bool fetch_curl_can_fetch(const nsurl *url);
static void * fetch_curl_setup(struct fetch *parent_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, const char *post_urlenc,
		 const struct fetch_multipart_data *post_multipart,
		 const char **headers);
static bool fetch_curl_start(void *vfetch);
static bool fetch_curl_initiate_fetch(struct curl_fetch_info *fetch,
		CURL *handle);
static CURL *fetch_curl_get_handle(lwc_string *host);
static void fetch_curl_cache_handle(CURL *handle, lwc_string *host);
static CURLcode fetch_curl_set_options(struct curl_fetch_info *f);
static CURLcode fetch_curl_sslctxfun(CURL *curl_handle, void *_sslctx,
				     void *p);
static void fetch_curl_abort(void *vf);
static void fetch_curl_stop(struct curl_fetch_info *f);
static void fetch_curl_free(void *f);
static void fetch_curl_poll(lwc_string *scheme_ignored);
static void fetch_curl_done(CURL *curl_handle, CURLcode result);
static int fetch_curl_progress(void *clientp, double dltotal, double dlnow,
		double ultotal, double ulnow);
static int fetch_curl_ignore_debug(CURL *handle,
				   curl_infotype type,
				   char *data,
				   size_t size,
				   void *userptr);
static size_t fetch_curl_data(char *data, size_t size, size_t nmemb,
			      void *_f);
static size_t fetch_curl_header(char *data, size_t size, size_t nmemb,
				void *_f);
static bool fetch_curl_process_headers(struct curl_fetch_info *f);
static struct curl_httppost *fetch_curl_post_convert(
		const struct fetch_multipart_data *control);
static int fetch_curl_verify_callback(int preverify_ok,
		X509_STORE_CTX *x509_ctx);
static int fetch_curl_cert_verify_callback(X509_STORE_CTX *x509_ctx,
		void *parm);


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

bool fetch_curl_can_fetch(const nsurl *url)
{
	LOG(("curl can fetch\n"));
	return false;
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

void * fetch_curl_setup(struct fetch *parent_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, const char *post_urlenc,
		 const struct fetch_multipart_data *post_multipart,
		 const char **headers)
{
LOG(("curl setup\n"));

return;
}


/**
 * Dispatch a single job
 */
bool fetch_curl_start(void *vfetch)
{
LOG(("curl start\n"));
	return 0;
}


/**
 * Initiate a fetch from the queue.
 *
 * Called with a fetch structure and a CURL handle to be used to fetch the
 * content.
 *
 * This will return whether or not the fetch was successfully initiated.
 */

bool fetch_curl_initiate_fetch(struct curl_fetch_info *fetch, CURL *handle)
{
	LOG(("curl initi fetch\n"));
	return 0;
}


/**
 * Find a CURL handle to use to dispatch a job
 */

CURL *fetch_curl_get_handle(lwc_string *host)
{
LOG(("curl get handle\n"));
	return 0;
}


/**
 * Cache a CURL handle for the provided host (if wanted)
 */

void fetch_curl_cache_handle(CURL *handle, lwc_string *host)
{
	LOG(("curl cache handle\n"));
}


/**
 * Set options specific for a fetch.
 */

typedef int CURLcode;

CURLcode
fetch_curl_set_options(struct curl_fetch_info *f)
{
	LOG(("curl set options\n"));
	return -1;
}


/**
 * cURL SSL setup callback
 */

CURLcode
fetch_curl_sslctxfun(CURL *curl_handle, void *_sslctx, void *parm)
{
	LOG(("curlcodessl\n"));
	
	return -1;
}


/**
 * Abort a fetch.
 */

void fetch_curl_abort(void *vf)
{
LOG(("curl abort\n"));
}


/**
 * Clean up the provided fetch object and free it.
 *
 * Will prod the queue afterwards to allow pending requests to be initiated.
 */

void fetch_curl_stop(struct curl_fetch_info *f)
{
	LOG(("curl stop\n"));
}


/**
 * Free a fetch structure and associated resources.
 */

void fetch_curl_free(void *vf)
{
	LOG(("curl free\n"));
}


/**
 * Do some work on current fetches.
 *
 * Must be called regularly to make progress on fetches.
 */

void fetch_curl_poll(lwc_string *scheme_ignored)
{
	LOG(("curl poll\n"));
}


/**
 * Handle a completed fetch (CURLMSG_DONE from curl_multi_info_read()).
 *
 * \param  curl_handle	curl easy handle of fetch
 */

void fetch_curl_done(CURL *curl_handle, CURLcode result)
{
	LOG(("curl done\n"));
}


/**
 * Callback function for fetch progress.
 */

int fetch_curl_progress(void *clientp, double dltotal, double dlnow,
			double ultotal, double ulnow)
{
	LOG(("curl progress\n"));
	return 0;
}



/**
 * Ignore everything given to it.
 *
 * Used to ignore cURL debug.
 */

int fetch_curl_ignore_debug(CURL *handle,
			    curl_infotype type,
			    char *data,
			    size_t size,
			    void *userptr)
{
	LOG(("curl igdebug\n"));
	return 0;
}


/**
 * Callback function for cURL.
 */

size_t fetch_curl_data(char *data, size_t size, size_t nmemb,
		       void *_f)
{
	LOG(("curl callback\n"));
		return 0;
}


/**
 * Callback function for headers.
 *
 * See RFC 2616 4.2.
 */

size_t fetch_curl_header(char *data, size_t size, size_t nmemb,
			 void *_f)
			 
{
	LOG(("curl header\n"));
		return 0;
}

/**
 * Find the status code and content type and inform the caller.
 *
 * Return true if the fetch is being aborted.
 */

bool fetch_curl_process_headers(struct curl_fetch_info *f)
{
LOG(("curl proc head\n"));
	return false;
}


/**
 * Convert a list of struct ::fetch_multipart_data to a list of
 * struct curl_httppost for libcurl.
 */
struct curl_httppost *
fetch_curl_post_convert(const struct fetch_multipart_data *control)
{
	struct curl_httppost *post = 0;
	
	LOG(("curl post - FAIL\n"));
	return post;
}


/**
 * OpenSSL Certificate verification callback
 * Stores certificate details in fetch struct.
 */

int fetch_curl_verify_callback(int preverify_ok, X509_STORE_CTX *x509_ctx)
{
	
	LOG(("curl verify\n"));
	return 0;
}


/**
 * OpenSSL certificate chain verification callback
 * Verifies certificate chain, setting up context for fetch_curl_verify_callback
 */

int fetch_curl_cert_verify_callback(X509_STORE_CTX *x509_ctx, void *parm)
{

LOG(("curl cert verify\n"));
	return 0;
}
