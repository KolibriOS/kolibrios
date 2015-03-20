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

/*
TODO:
Have a double linked list containing all the handles on work that needs to be done
Inside fetch_poll_curl, do work on it.
Let the overall structure remain intact
*/

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <sys/stat.h>

#include <libwapcaplet/libwapcaplet.h>

#include "utils/config.h"
/* #include <openssl/ssl.h> */
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

#include "http_msg.h"
#include "http.h"

/**********************************************************************
 ********This section added for resolving compile errors***************/
#define CURL_ERROR_SIZE 100
/*Definitions for CURL EASY Codes*/

#define CURLE_OK 0
#define CURLE_PARTIAL_FILE 18
#define CURLE_WRITE_ERROR 23
#define CURLE_SSL_CONNECT_ERROR 35

/*Definitions for CURL MULTI Codes*/

#define CURLM_OK 0
#define CURLM_CALL_MULTI_PERFORM -1
#define CURLM_FAILED 123123
/* KOSH stands for KolibriOS HTTP :) */

int FLAG_HTTP11             = 1 << 0;
int FLAG_GOT_HEADER         = 1 << 1;
int FLAG_GOT_ALL_DATA       = 1 << 2;
int FLAG_CONTENT_LENGTH     = 1 << 3;
int FLAG_CHUNKED            = 1 << 4;
int FLAG_CONNECTED          = 1 << 5;

/* ERROR flags go into the upper word */
int FLAG_INVALID_HEADER     = 1 << 16;
int FLAG_NO_RAM             = 1 << 17;
int FLAG_SOCKET_ERROR       = 1 << 18;
int FLAG_TIMEOUT_ERROR      = 1 << 19;
int FLAG_TRANSFER_FAILED    = 1 << 20;


struct KOSHcode {
  long code;
};

struct KOSHMcode {
  long code;
};

struct KOSHMsg {
  char *msg;
};

typedef struct KOSHcode KOSHcode;
typedef struct KOSHMcode KOSHMcode;
struct kosh_infotype {
  int type;
};
typedef struct kosh_infotype kosh_infotype;

 struct curl_slist {
   char *data;
   struct curl_slist *next;
 };
 
/**********************************************************************/

/* uncomment this to use scheduler based calling
#define FETCHER_CURLL_SCHEDULED 1
*/

/** SSL certificate info */
/* struct cert_info { */
/* 	X509 *cert;		/\**< Pointer to certificate *\/ */
/* 	long err;		/\**< OpenSSL error code *\/ */
/* }; */

/** Information for a single fetch. */
struct curl_fetch_info {
        struct fetch *fetch_handle; /**< The fetch handle we're parented by. */
        struct http_msg * curl_handle;	/**< cURL handle if being fetched, or 0. */
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
/* #define MAX_CERTS 10 */
/* 	struct cert_info cert_data[MAX_CERTS];	/\**< HTTPS certificate data *\/ */
	unsigned int last_progress_update;	/**< Time of last progress update */
};

struct cache_handle {
	struct http_msg *handle; /**< The cached struct http_msg handle */
	lwc_string *host;   /**< The host for which this handle is cached */

	struct cache_handle *r_prev; /**< Previous cached handle in ring. */
	struct cache_handle *r_next; /**< Next cached handle in ring. */
};

struct fetch_info_slist {
  struct curl_fetch_info *fetch_info;  /* this fetch_info contains the same handle as the struct */
  struct http_msg *handle;
  bool fetch_curl_header_called;
  struct fetch_info_slist *next;
};


struct fetch_info_slist *fetch_curl_multi;		/**< Global cURL multi handle. */

/** Curl handle with default options set; not used for transfers. */
static struct http_msg *fetch_blank_curl;
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
		struct http_msg *handle);
static struct http_msg *fetch_curl_get_handle(lwc_string *host);
static void fetch_curl_cache_handle(struct http_msg *handle, lwc_string *host);
static KOSHcode fetch_curl_set_options(struct curl_fetch_info *f);
/* static KOSHcode fetch_curl_sslctxfun(CURL *curl_handle, void *_sslctx, */
/* 				     void *p); */
static void fetch_curl_abort(void *vf);
static void fetch_curl_stop(struct fetch_info_slist *f);
static void fetch_curl_free(void *f);
static void fetch_curl_poll(lwc_string *scheme_ignored);
static void fetch_curl_done(struct fetch_info_slist *curl_handle);
static int fetch_curl_progress(void *clientp, double dltotal, double dlnow,
			       double ultotal, double ulnow);
static int fetch_curl_ignore_debug(struct http_msg *handle,
				   kosh_infotype type,
				   char *data,
				   size_t size,
				   void *userptr);
static size_t fetch_curl_data(void *_f);
/* static size_t fetch_curl_header(char *data, size_t size, size_t nmemb, */
/* 				void *_f); */
void fetch_curl_header(void *_f);
static bool fetch_curl_process_headers(struct curl_fetch_info *f);
static struct curl_httppost *fetch_curl_post_convert(
		const struct fetch_multipart_data *control);

/* static int fetch_curl_verify_callback(int preverify_ok, */
/* 		X509_STORE_CTX *x509_ctx); */
/* static int fetch_curl_cert_verify_callback(X509_STORE_CTX *x509_ctx, */
/* 		void *parm); */

/**************Functions added for replacing curl's provided functionality ************/
struct curl_slist *curl_slist_append(struct curl_slist * list, const char * string ); 
void curl_slist_free_all(struct curl_slist *);
struct fetch_info_slist *curl_multi_remove_handle(struct fetch_info_slist *multi_handle, struct curl_fetch_info *fetch_to_delete);
struct http_msg * curl_easy_init(void);
void curl_easy_cleanup(struct http_msg *handle);
int curl_multi_add_handle(struct fetch_info_slist **multi_handle, struct curl_fetch_info *new_fetch_info);

/**
 * Initialise the fetcher.
 *
 * Must be called once before any other function.
 */

void fetch_curl_register(void)
{
	/* KOSHcode code; */
	/* curl_version_info_data *data; */
	int i;
	lwc_string *scheme;
	
	LOG(("curl_version (no CURL XD)"));

	const struct fetcher_operation_table fetcher_ops = {
	  .initialise = fetch_curl_initialise,
	  .acceptable = fetch_curl_can_fetch,
	  .setup = fetch_curl_setup,
	  .start = fetch_curl_start,
	  .abort = fetch_curl_abort,
	  .free = fetch_curl_free,
	  .poll = fetch_curl_poll,
	  .finalise = fetch_curl_finalise
	};
	

	/* code = curl_global_init(CURL_GLOBAL_ALL); */
	/* if (code != CURLE_OK) */
	/* 	die("Failed to initialise the fetch module " */
	/* 			"(curl_global_init failed)."); */

	/*TODO : Put the init function for our global queue here */
	/* What do we need for an init? Just setting the list to NULL should be enough. 
	   We might track the number of nodes in the list, but that's not required since we usean SLL.
	*/
	/* fetch_curl_multi = curl_multi_init(); */
	/* if (!fetch_curl_multi) */
	/* 	die("Failed to initialise the fetch module " */
	/* 			"(curl_multi_init failed)."); */

	/* Create a curl easy handle with the options that are common to all
	   fetches. */
	
	/*HTTP Library initiated using volatime asm code in http.c */
	DBG("Calling curl_easy_init\n");
	/* fetch_blank_curl = curl_easy_init();  */
	
	/* if (!fetch_blank_curl) */
	/*   { */
	/*     DBG("fetch_blank_curl is NULL"); */
	/*     die("Failed to initialise the fetch module " */
	/* 	"(curl_easy_init failed)."); */
	/*   } */
	/* else */
	/*   DBG("fetch_blank_curl is usable."); */

	fetch_blank_curl = NULL;

	/* TODO: The SETOPT calls set up the parameters for the curl handle.
	   Since we don't want to use curl, these are of no use, but our native handle 
	   should consider all these fields while being set up for proper functioning
	*/

/* #undef SETOPT */
/* #define SETOPT(option, value) \ */
/* 	code = curl_easy_setopt(fetch_blank_curl, option, value);	\ */
/* 	if (code != CURLE_OK)						\ */
/* 		goto curl_easy_setopt_failed; */

/* 	if (verbose_log) { */
/* 	    SETOPT(CURLOPT_VERBOSE, 1); */
/* 	} else { */
/* 	    SETOPT(CURLOPT_VERBOSE, 0); */
/* 	} */
/* 	SETOPT(CURLOPT_ERRORBUFFER, fetch_error_buffer); */
/* 	if (nsoption_bool(suppress_curl_debug)) */
/* 		SETOPT(CURLOPT_DEBUGFUNCTION, fetch_curl_ignore_debug); */
/* 	SETOPT(CURLOPT_WRITEFUNCTION, fetch_curl_data); */
/* 	SETOPT(CURLOPT_HEADERFUNCTION, fetch_curl_header); */ /* Calling fetch_curl_header inside fetch_curl_process_headers */
	/* 	SETOPT(CURLOPT_PROGRESSFUNCTION, fetch_curl_progress); */ /* TODO: Add this with httplib somehow */
/* 	SETOPT(CURLOPT_NOPROGRESS, 0); */
/* 	SETOPT(CURLOPT_USERAGENT, user_agent_string()); */
/* 	SETOPT(CURLOPT_ENCODING, "gzip"); */
/* 	SETOPT(CURLOPT_LOW_SPEED_LIMIT, 1L); */
/* 	SETOPT(CURLOPT_LOW_SPEED_TIME, 180L); */
/* 	SETOPT(CURLOPT_NOSIGNAL, 1L); */
/* 	SETOPT(CURLOPT_CONNECTTIMEOUT, 30L); */

/* 	if (nsoption_charp(ca_bundle) &&  */
/* 	    strcmp(nsoption_charp(ca_bundle), "")) { */
/* 		LOG(("ca_bundle: '%s'", nsoption_charp(ca_bundle))); */
/* 		SETOPT(CURLOPT_CAINFO, nsoption_charp(ca_bundle)); */
/* 	} */
/* 	if (nsoption_charp(ca_path) && strcmp(nsoption_charp(ca_path), "")) { */
/* 		LOG(("ca_path: '%s'", nsoption_charp(ca_path))); */
/* 		SETOPT(CURLOPT_CAPATH, nsoption_charp(ca_path)); */
/* 	} */

	/*TODO: Useless for now, no SSL Support*/

	/* /\* Detect whether the SSL CTX function API works *\/ */
	/* curl_with_openssl = true; */
	/* code = curl_easy_setopt(fetch_blank_curl,  */
	/* 		CURLOPT_SSL_CTX_FUNCTION, NULL); */
	/* if (code != CURLE_OK) { */
	/* 	curl_with_openssl = false; */
	/* } */

	/* /\* LOG(("cURL %slinked against openssl", curl_with_openssl ? "" : "not ")); *\/ */

	/* /\* cURL initialised okay, register the fetchers *\/ */

	/* data = curl_version_info(CURLVERSION_NOW); */

	/*TODO: We strictly want to deal with only http as a protocol right now, this stuff can come 
	  handy later, so it shall sit here for a while XD
	  Removing the for loop for a single http fetcher setup scheme.
	*/
	
	/*	for (i = 0; data->protocols[i]; i++) {
		if (strcmp(data->protocols[i], "http") == 0) {
			if (lwc_intern_string("http", SLEN("http"),
					&scheme) != lwc_error_ok) {
				die("Failed to initialise the fetch module "
						"(couldn't intern \"http\").");
			}

		} else if (strcmp(data->protocols[i], "https") == 0) {
			if (lwc_intern_string("https", SLEN("https"),
					&scheme) != lwc_error_ok) {
				die("Failed to initialise the fetch module "
						"(couldn't intern \"https\").");
			}

		} else {
			// Ignore non-http(s) protocols 
			continue;
		}

		if (!fetch_add_fetcher(scheme,
		fetch_curl_initialise,
				fetch_curl_can_fetch,
				fetch_curl_setup,
				fetch_curl_start,
				fetch_curl_abort,
				sfetch_curl_free,
#ifdef FETCHER_CURLL_SCHEDULED
				       NULL,
#else
				fetch_curl_poll,
#endif
				fetch_curl_finalise)) {
			LOG(("Unable to register cURL fetcher for %s",
					data->protocols[i]));
		}
	}
*/

/* 	if (lwc_intern_string("http", SLEN("http"), */
/* 			      &scheme) != lwc_error_ok) { */
/* 	  die("Failed to initialise the fetch module " */
/* 	      "(couldn't intern \"http\")."); */
/* 	} */
	
/* 	if (!fetch_add_fetcher(scheme, */
/* 			       fetch_curl_initialise, */
/* 			       fetch_curl_can_fetch, */
/* 			       fetch_curl_setup, */
/* 			       fetch_curl_start, */
/* 			       fetch_curl_abort, */
/* 			       fetch_curl_free, */
/* #ifdef FETCHER_CURLL_SCHEDULED */
/* 			       NULL, */
/* #else */
/* 			       fetch_curl_poll, */
/* #endif */
/* 			       fetch_curl_finalise)) { */
/* 	  LOG(("Unable to register cURL fetcher for %s", */
/* 	       "http")); */
/* 	}        */
/* 	DBG("fetch_curl_register returning.");	 */
	return;
	
 curl_easy_setopt_failed:
	die("Failed to initialise the fetch module "
			"(curl_easy_setopt failed).");
}


/**
 * Initialise a cURL fetcher.
 */

/* Seems to not need any work right now, curl_fetchers_registered variable seems handy*/
bool fetch_curl_initialise(lwc_string *scheme)
{
	LOG(("Initialise cURL fetcher for %s", lwc_string_data(scheme)));
	curl_fetchers_registered++;
	return true; /* Always succeeds */
}


/**
 * Finalise a cURL fetcher 
 */

void fetch_curl_finalise(lwc_string *scheme)
{
	struct cache_handle *h;
	curl_fetchers_registered--;
	LOG(("Finalise cURL fetcher %s", lwc_string_data(scheme)));
	if (curl_fetchers_registered == 0) {
		/* KOSHMcode codem; */
              	int codem;
		/* All the fetchers have been finalised. */
		LOG(("All cURL fetchers finalised, closing down cURL"));

		/* TODO: Add any clean up functions for httplib here. */
		/* curl_easy_cleanup now contains http_free()  */

		/* curl_easy_cleanup(fetch_blank_curl); */

		/* codem = curl_multi_cleanup(fetch_curl_multi); */
		/* if (codem != CURLM_OK) */
		/* 	LOG(("curl_multi_cleanup failed: ignoring")); */

		/* curl_global_cleanup(); */
	}

	/* Free anything remaining in the cached curl handle ring */
	while (curl_handle_ring != NULL) {
		h = curl_handle_ring;
		RING_REMOVE(curl_handle_ring, h);
		lwc_string_unref(h->host);
	}
}

bool fetch_curl_can_fetch(const nsurl *url)
{
	return nsurl_has_component(url, NSURL_HOST);
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
	struct curl_fetch_info *fetch;
	struct  curl_slist *slist;
	int i;
	
	fetch = malloc(sizeof (*fetch));

	if (fetch == NULL)
	  {
	    LOG(("Fetch was NULL. Aborting fetch_curl_setup"));
	    return 0;
	  }
	fetch->fetch_handle = parent_fetch;

	LOG(("fetch %p, url '%s'", fetch, nsurl_access(url)));

	/* construct a new fetch structure */
	fetch->curl_handle = NULL;
	fetch->had_headers = false;
	fetch->abort = false;
	fetch->stopped = false;
	fetch->only_2xx = only_2xx;
	fetch->downgrade_tls = downgrade_tls;
	fetch->headers = NULL;
	fetch->url = nsurl_ref(url);
	fetch->host = nsurl_get_component(url, NSURL_HOST);
	fetch->location = NULL;
	fetch->content_length = 0;
	fetch->http_code = 0;
	fetch->cookie_string = NULL;
	fetch->realm = NULL;
	fetch->post_urlenc = NULL;
	fetch->post_multipart = NULL;

	if (post_urlenc)
	  {
	    LOG(("post_urlenc is not NULL : %s.\n", post_urlenc));
	    fetch->post_urlenc = strdup(post_urlenc);
	  }
	else if (post_multipart)
	  {
	    LOG(("post_multipart is not NULL : %x.\n", post_multipart));	    
	  /*TODO: Need a post converter here, shouldn't be large though*/
	  fetch->post_multipart = fetch_curl_post_convert(post_multipart);
	  }
	/* memset(fetch->cert_data, 0, sizeof(fetch->cert_data)); */
	fetch->last_progress_update = 0;

	if (fetch->host == NULL ||
		(post_multipart != NULL && fetch->post_multipart == NULL) ||
			(post_urlenc != NULL && fetch->post_urlenc == NULL))
		goto failed;

	/* TODO : Write an implementation for curl_slist_append for adding and appending fields to header*/
	/* This is done for now */

/* #define APPEND(list, value) \ */
/* 	slist = curl_slist_append(list, value);		\ */
/* 	if (slist == NULL)				\ */
/* 		goto failed;				\ */
/* 	list = slist; */
	
	/*TODO : This section will need some work because we don't use curl headers but use the ones
	  provided by http.obj which does not necessarily include the same prefed headers
	*/
	/* remove curl default headers */
	/* APPEND(fetch->headers, "Pragma:"); */

	/* when doing a POST libcurl sends Expect: 100-continue" by default
	 * which fails with lighttpd, so disable it (see bug 1429054) */
	/* APPEND(fetch->headers, "Expect:"); */

	/* if ((nsoption_charp(accept_language) != NULL) &&  */
	/*     (nsoption_charp(accept_language)[0] != '\0')) { */
	/* 	char s[80]; */
	/* 	snprintf(s, sizeof s, "Accept-Language: %s, *;q=0.1", */
	/* 		 nsoption_charp(accept_language)); */
	/* 	s[sizeof s - 1] = 0; */
	/* 	APPEND(fetch->headers, s); */
/* } */

/* 	if (nsoption_charp(accept_charset) != NULL &&  */
/* 	    nsoption_charp(accept_charset)[0] != '\0') { */
/* 		char s[80]; */
/* 		snprintf(s, sizeof s, "Accept-Charset: %s, *;q=0.1", */
/* 			 nsoption_charp(accept_charset)); */
/* 		s[sizeof s - 1] = 0; */
/* 		APPEND(fetch->headers, s); */
/* 	} */

/* 	if (nsoption_bool(do_not_track) == true) { */
/* 		APPEND(fetch->headers, "DNT: 1"); */
/* 	} */

/* 	/\* And add any headers specified by the caller *\/ */
/* 	for (i = 0; headers[i] != NULL; i++) { */
/* 		APPEND(fetch->headers, headers[i]); */
/* 	} */

	return fetch;

failed:
	if (fetch->host != NULL)
		lwc_string_unref(fetch->host);

	nsurl_unref(fetch->url);
	free(fetch->post_urlenc);
	/* TOOD: Figure out a way to deal with post data. */
	/* if (fetch->post_multipart) */
	/* 	curl_formfree(fetch->post_multipart); */
	curl_slist_free_all(fetch->headers);
	free(fetch);
	return NULL;
}


/**
 * Dispatch a single job
 */
bool fetch_curl_start(void *vfetch)
{
  
	struct curl_fetch_info *fetch = (struct curl_fetch_info*)vfetch;
	DBG("Inside fetch_curl_start\n");
	return fetch_curl_initiate_fetch(fetch,
			fetch_curl_get_handle(fetch->host));
}


/**
 * Initiate a fetch from the queue.
 *
 * Called with a fetch structure and a CURL handle to be used to fetch the
 * content.
 *
 * This will return whether or not the fetch was successfully initiated.
 */

bool fetch_curl_initiate_fetch(struct curl_fetch_info *fetch, struct http_msg *handle)
{
	KOSHcode code; 
        KOSHMcode codem;
	unsigned int wererat; /* Like soUrcerer wanted it :D */
	char *zz;
	int pr;
	
	/* fetch->curl_handle = handle; */
	/* Don't need to add options to handle from http obj */
	/* Initialise the handle */
	/* DBG("inside fetch_curl_initiate_fetch()...\n"); */
	
	/* code = fetch_curl_set_options(fetch);  */
	/*  if (code.code != CURLE_OK) {  */
	/*  	fetch->curl_handle = 0;  */
	/*  	return false;  */
	/*  }  */

	/* TODO : Write a curl_multi_add_handle alternative which puts the handle in our global queue
	   for polling later on multiple transfers together*/

	/* add to the global curl multi handle */
	
	DBG("inside fetch_curl_initiate_fetch()...\n");
	
	nsurl_get(fetch->url, NSURL_WITH_FRAGMENT, &zz, &pr);
	
	if (zz == NULL) {
	  fetch_curl_abort(fetch);
	  return NULL;
	}
	
	/*TODO : Always clear the flags for the handle here*/	
	
	if(fetch->post_urlenc)
	  {
	    LOG(("http_post on %s with headers: %s", zz, fetch->post_urlenc));
	    wererat = http_post(zz, NULL, NULL, NULL, "application/x-www-form-urlencoded", strlen(fetch->post_urlenc));

	    if(wererat == 0)
	      {
		LOG(("Error. http_post failed. Aborting fetch.\n"));
		fetch_curl_abort(fetch);
		return NULL;
	      }
	    else /*Send the post request body*/
	      {
		int sent = http_send(wererat, fetch->post_urlenc, strlen(fetch->post_urlenc));
		LOG(("Sent %d bytes in http_send for %s", sent, fetch->post_urlenc));
	      }	    
	  }
	else /* GET Request */
	  {   	    
	    LOG(("http_get on URL : %s", zz));
	    wererat = http_get(zz, NULL, NULL, NULL); /* Initiates the GET on the handle we want to initiate for */
	    
	    if(wererat == 0)               /* http_get failed. Something wrong. Can't do anything here  */
	      {
		DBG("Error. http_get failed. Aborting fetch.\n");
		fetch_curl_abort(fetch);
		return NULL;
	      }	    
	  }
	
	/* Probably check for the older curl_handle here and http_free() or http_disconnect accordingly TODO */
	
	fetch->curl_handle = (struct http_msg *)wererat;  /* Adding the http_msg handle to fetch->handle */

	LOG(("wererat is %u with flags = %u", wererat, fetch->curl_handle->flags));
	
	codem.code = curl_multi_add_handle(&fetch_curl_multi, fetch);
	
	/* Probably drop the assert and handle this properly, but that's for later */
	assert(codem.code == CURLM_OK || codem.code == CURLM_CALL_MULTI_PERFORM);
	
	/* TODO: No idea what this does right now. Shouldn't this be inside an #if macro call? to enable/disable curll scheduling.*/
	schedule(1, (schedule_callback_fn)fetch_curl_poll, NULL);
	
	return true;
}


/**
 * Find a CURL handle to use to dispatch a job
 */

struct http_msg *fetch_curl_get_handle(lwc_string *host)
{
	struct cache_handle *h;
	struct http_msg *ret;

	DBG("inside fetch_curl_get_handle()...\n");
	
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	if (h) {
		ret = h->handle;
		lwc_string_unref(h->host);
		RING_REMOVE(curl_handle_ring, h);
		free(h);
	} else {
	  /* ret = curl_easy_duphandle(fetch_blank_curl); */
	  /* TODO: Verify if this is equivalent to curl_easy_duphandle call above this */
	  ret = curl_easy_init();
	}
	
	return ret;
}


/**
 * Cache a CURL handle for the provided host (if wanted)
 */

/*TODO : Useful for using a pre existing cached handle for faster lookup*/

void fetch_curl_cache_handle(struct http_msg *handle, lwc_string *host)
{
	struct cache_handle *h = 0;
	int c;

	DBG("inside fetch_curl_cache_handle...\n");

	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	if (h) {
	  /*TODO: Replace curl_easy_cleanup function for something useful for use with KOSH*/
		/* Already have a handle cached for this hostname */
		curl_easy_cleanup(handle);
		return;
	}
	/* We do not have a handle cached, first up determine if the cache is full */
	RING_GETSIZE(struct cache_handle, curl_handle_ring, c);
	if (c >= nsoption_int(max_cached_fetch_handles)) {
		/* Cache is full, so, we rotate the ring by one and
		 * replace the oldest handle with this one. We do this
		 * without freeing/allocating memory (except the
		 * hostname) and without removing the entry from the
		 * ring and then re-inserting it, in order to be as
		 * efficient as we can.
		 */
		if (curl_handle_ring != NULL) {
			h = curl_handle_ring;
			curl_handle_ring = h->r_next;
			curl_easy_cleanup(h->handle);
			h->handle = handle;
			lwc_string_unref(h->host);
			h->host = lwc_string_ref(host);
		} else {
			/* Actually, we don't want to cache any handles */
			curl_easy_cleanup(handle);
		}

		return;
	}
	/* The table isn't full yet, so make a shiny new handle to add to the ring */
	h = (struct cache_handle*)malloc(sizeof(struct cache_handle));
	h->handle = handle;
	h->host = lwc_string_ref(host);
	RING_INSERT(curl_handle_ring, h);
}


/**
 * Set options specific for a fetch.
 */

/*TODO: This function sets up a specific fetch. Need a replacement for SETOPT for setting parameters 
  in our implementation
*/

KOSHcode
fetch_curl_set_options(struct curl_fetch_info *f)
{
	KOSHcode code;
	const char *auth;

	DBG("Inside fetch_curl_set_options\n");
	
	/*TODO: Replace SETOPT with sane set up of parameters for our handle*/

/* #undef SETOPT */
/* #define SETOPT(option, value) { \ */
/* 	code = curl_easy_setopt(f->curl_handle, option, value);	\ */
/* 	if (code != CURLE_OK)					\ */
/* 		return code;					\ */
/* 	} */

/* 	SETOPT(CURLOPT_URL, nsurl_access(f->url)); */
/* 	SETOPT(CURLOPT_PRIVATE, f); */
/* 	SETOPT(CURLOPT_WRITEDATA, f); */
/* 	SETOPT(CURLOPT_WRITEHEADER, f); */
/* 	SETOPT(CURLOPT_PROGRESSDATA, f); */
/* 	SETOPT(CURLOPT_REFERER, fetch_get_referer_to_send(f->fetch_handle)); */
/* 	SETOPT(CURLOPT_HTTPHEADER, f->headers); */
/* 	if (f->post_urlenc) { */
/* 		SETOPT(CURLOPT_HTTPPOST, NULL); */
/* 		SETOPT(CURLOPT_HTTPGET, 0L); */
/* 		SETOPT(CURLOPT_POSTFIELDS, f->post_urlenc); */
/* 	} else if (f->post_multipart) { */
/* 		SETOPT(CURLOPT_POSTFIELDS, NULL); */
/* 		SETOPT(CURLOPT_HTTPGET, 0L); */
/* 		SETOPT(CURLOPT_HTTPPOST, f->post_multipart); */
/* 	} else { */
/* 		SETOPT(CURLOPT_POSTFIELDS, NULL); */
/* 		SETOPT(CURLOPT_HTTPPOST, NULL); */
/* 		SETOPT(CURLOPT_HTTPGET, 1L); */
/* 	} */


 	/* f->cookie_string = urldb_get_cookie(f->url, true);  */


/* 	if (f->cookie_string) { */
/* 		SETOPT(CURLOPT_COOKIE, f->cookie_string); */
/* 	} else { */
/* 		SETOPT(CURLOPT_COOKIE, NULL); */
/* 	} */

/* 	if ((auth = urldb_get_auth_details(f->url, NULL)) != NULL) { */
/* 		SETOPT(CURLOPT_HTTPAUTH, CURLAUTH_ANY); */
/* 		SETOPT(CURLOPT_USERPWD, auth); */
/* 	} else { */
/* 		SETOPT(CURLOPT_USERPWD, NULL); */
/* 	} */

/* 	if (nsoption_bool(http_proxy) &&  */
/* 	    (nsoption_charp(http_proxy_host) != NULL) && */
/* 	    (strncmp(nsurl_access(f->url), "file:", 5) != 0)) { */
/* 		SETOPT(CURLOPT_PROXY, nsoption_charp(http_proxy_host)); */
/* 		SETOPT(CURLOPT_PROXYPORT, (long) nsoption_int(http_proxy_port)); */
/* 		if (nsoption_int(http_proxy_auth) != OPTION_HTTP_PROXY_AUTH_NONE) { */
/* 			SETOPT(CURLOPT_PROXYAUTH, */
/* 			       nsoption_int(http_proxy_auth) == */
/* 					OPTION_HTTP_PROXY_AUTH_BASIC ? */
/* 					(long) CURLAUTH_BASIC : */
/* 					(long) CURLAUTH_NTLM); */
/* 			snprintf(fetch_proxy_userpwd, */
/* 					sizeof fetch_proxy_userpwd, */
/* 					"%s:%s", */
/* 				 nsoption_charp(http_proxy_auth_user), */
/* 				 nsoption_charp(http_proxy_auth_pass)); */
/* 			SETOPT(CURLOPT_PROXYUSERPWD, fetch_proxy_userpwd); */
/* 		} */
/* 	} else { */
/* 		SETOPT(CURLOPT_PROXY, NULL); */
/* 	} */

/* 	/\* Disable SSL session ID caching, as some servers can't cope. *\/ */
/* 	SETOPT(CURLOPT_SSL_SESSIONID_CACHE, 0); */

/* 	if (urldb_get_cert_permissions(f->url)) { */
/* 		/\* Disable certificate verification *\/ */
/* 		SETOPT(CURLOPT_SSL_VERIFYPEER, 0L); */
/* 		SETOPT(CURLOPT_SSL_VERIFYHOST, 0L); */
/* 		if (curl_with_openssl) { */
/* 			SETOPT(CURLOPT_SSL_CTX_FUNCTION, NULL); */
/* 			SETOPT(CURLOPT_SSL_CTX_DATA, NULL); */
/* 		} */
/* 	} else { */
/* 		/\* do verification *\/ */
/* 		SETOPT(CURLOPT_SSL_VERIFYPEER, 1L); */
/* 		SETOPT(CURLOPT_SSL_VERIFYHOST, 2L); */
/* 		if (curl_with_openssl) { */
/* 			SETOPT(CURLOPT_SSL_CTX_FUNCTION, fetch_curl_sslctxfun); */
/* 			SETOPT(CURLOPT_SSL_CTX_DATA, f); */
/* 		} */
/* 	} */
	code.code = CURLE_OK;
	return code;
	/* return CURLE_OK; */
}


/**
 * cURL SSL setup callback
 */
/*TODO : Commenting this out because of lack of SSL support right now */

/******************************************************************
KOSHcode
fetch_curl_sslctxfun(CURL *curl_handle, void *_sslctx, void *parm)
{
	struct curl_fetch_info *f = (struct curl_fetch_info *) parm;
	SSL_CTX *sslctx = _sslctx;
	long options = SSL_OP_ALL;

	SSL_CTX_set_verify(sslctx, SSL_VERIFY_PEER, fetch_curl_verify_callback);
	SSL_CTX_set_cert_verify_callback(sslctx, fetch_curl_cert_verify_callback,
					 parm);

	if (f->downgrade_tls) {
#ifdef SSL_OP_NO_TLSv1_1
//		 Disable TLS1.1, if the server can't cope with it 
		options |= SSL_OP_NO_TLSv1_1;
#endif
	}

#ifdef SSL_OP_NO_TLSv1_2
// Disable TLS1.2, as it causes some servers to stall. 
	options |= SSL_OP_NO_TLSv1_2;
#endif

	SSL_CTX_set_options(sslctx, options);

	return CURLE_OK;
}
******************************************************************/

/**
 * Abort a fetch.
 */
/* TODO: Seems usable until further action */

void fetch_curl_abort(void *vf)
{
	struct curl_fetch_info *f = (struct curl_fetch_info *)vf;
	assert(f);
	LOG(("fetch %p, url '%s'", f, nsurl_access(f->url)));

	fetch_curl_multi = curl_multi_remove_handle(fetch_curl_multi, f);

	if (f->curl_handle) {
		f->abort = true;
	} else {
		fetch_remove_from_queues(f->fetch_handle);
		fetch_free(f->fetch_handle);
	}
}


/**
 * Clean up the provided fetch object and free it.
 *
 * Will prod the queue afterwards to allow pending requests to be initiated.
 */

void fetch_curl_stop(struct fetch_info_slist *node)
{
	KOSHMcode codem;

	/* TODO: Assert doesn't look like a safe option, but this is probably a fatal condition */

	struct curl_fetch_info *f = node->fetch_info;

	assert(f);
	LOG(("fetch %p, url '%s'", f, nsurl_access(f->url)));

	if (f->curl_handle) {
		/* remove from curl multi handle */
	  /*TODO: Need a replacement for curl_multi_remove_handle function*/
	  	/* LOG(("fetch_curl_multi : %u", fetch_curl_multi)); */
		fetch_curl_multi = curl_multi_remove_handle(fetch_curl_multi, f);
		/* assert(codem.code == CURLM_OK); */
		/* Put this curl handle into the cache if wanted. */
		/* TODO: Cache? */
		/* fetch_curl_cache_handle(f->curl_handle, f->host); */

		if(f && f->curl_handle) 
		  http_free(f->curl_handle);

		f->curl_handle = 0;
	}

	fetch_remove_from_queues(f->fetch_handle);
	LOG(("Returning"));
}


/**
 * Free a fetch structure and associated resources.
 */
/*TODO: Except the cert details at the bottom of the function, everything else seems workable*/

void fetch_curl_free(void *vf)
{
	struct curl_fetch_info *f = (struct curl_fetch_info *)vf;
	int i;
	DBG("inside fetch_curl_free()..\n");

	if (f->curl_handle)
		curl_easy_cleanup(f->curl_handle);

	nsurl_unref(f->url);
	lwc_string_unref(f->host);
	free(f->location);
	free(f->cookie_string);
	free(f->realm);
	if (f->headers)
		curl_slist_free_all(f->headers);
	free(f->post_urlenc);
	/* TODO: Deal with POST data asap */
	/* if (f->post_multipart) */
	/* 	curl_formfree(f->post_multipart); */

	/* for (i = 0; i < MAX_CERTS && f->cert_data[i].cert; i++) { */
	/* 	f->cert_data[i].cert->references--; */
	/* 	if (f->cert_data[i].cert->references == 0) */
	/* 		X509_free(f->cert_data[i].cert); */
	/* } */

	free(f);
}


/**
 * Do some work on current fetches.
 *
 * Must be called regularly to make progress on fetches.
 */

/*TODO: This is our slave (from master slave) function that will poll fetches.
  We will maintain our own global ring of handles and let this function poll the entries
  and do some work accordingly. Useful for multiple transfers simultaneously.
*/

void fetch_curl_poll(lwc_string *scheme_ignored)
{
	int running, queue;
	KOSHMcode codem;

	if(!fetch_curl_multi)
	  LOG(("fetch_curl_multi is NULL"));
	else
	  {	    
	    struct fetch_info_slist *temp = fetch_curl_multi;
	    
	    curl_multi_perform(fetch_curl_multi);
	    
	    while(temp)
	      {
		struct fetch_info_slist *new_temp = temp->next;

		/* Check if the headers were received. thanks hidnplayr :P */
		if ((temp->handle->flags & FLAG_GOT_HEADER) && (!temp->fetch_curl_header_called)) 
		  {
		    fetch_curl_header(temp->fetch_info);	
		    temp->fetch_curl_header_called = true;
		  }
		
		if(temp->handle->flags & FLAG_GOT_ALL_DATA) /* FLAG_GOT_ALL_DATA is set */
		  {
		    /* DBG(("calling fetch_curl_data")); */
		    /* LOG(("content in handle is : %s", temp->handle->content_ptr)); */		    		    
		    fetch_curl_data(temp->fetch_info);
		    fetch_curl_done(temp);
		    fetch_curl_multi = curl_multi_remove_handle(fetch_curl_multi, temp->fetch_info);	    
		  }

		/*Add Error FLAG handle here TODO*/
		
		temp = new_temp;
	      }
	  }
      /* TODO: Add more flags here */	  
      
      /*TODO: Handle various conditions here, and set the status code accordinpgly when 
	calling fetch_curL_done
      */
      
      /* TODO: Probably decide the condition of the fetch here */
      
      /* The whole data recieved is shown by FLAG_GOT_ALL_DATA that is 1 SHL 2, meaning 4. Check for it right here. */	  	  

	/* process curl results */
	/*TODO: Needs to be replaced , no idea how to do it right now */
	/* Go through each http_msg handle from http.obj and check if it's done yet or not , 
	   using the return value from http_receive.   If done, remove it. Else let it stay.
	*/
	/*TODO: This has been commented to figure out linker errors.
	  Uncomment this and combine this with the above chunk toget the main process loop
	*/
	/* curl_msg = curl_multi_info_read(fetch_curl_multi, &queue); */
	/* while (curl_msg) { */
	/* 	switch (curl_msg->msg) { */
	/* 		case CURLMSG_DONE: */
	/* 			fetch_curl_done(curl_msg->easy_handle, */
	/* 					curl_msg->data.result); */
	/* 			break; */
	/* 		default: */
	/* 			break; */
	/* 	} */
	/* 	curl_msg = curl_multi_info_read(fetch_curl_multi, &queue); */
	/* } */

#ifdef FETCHER_CURLL_SCHEDULED
	if (running != 0) {
		schedule(1, (schedule_callback_fn)fetch_curl_poll, fetch_curl_poll);
	}
#endif
	/* LOG(("Returning froms fetch_curl_poll\n")); */
}


/**
 * Handle a completed fetch (CURLMSG_DONE from curl_multi_info_read()).
 *
 * \param  curl_handle	curl easy handle of fetch
 */

/* TODO: curl_easy_getinfo needs a replacement for getting the status of things around
   SSL stuff needs to go away , as usual.
*/
void fetch_curl_done(struct fetch_info_slist *node)
{
	fetch_msg msg;
	bool finished = false;
	bool error = false;
	bool cert = false;
	bool abort_fetch;
	struct curl_fetch_info *f = node->fetch_info;
	char **_hideous_hack = (char **) (void *) &f;
	KOSHcode code;
	int result = CURLE_OK;

	/* TODO: Remove this definition and get a better replacement for CURLINFO_PRIVATE */
	
	/* struct cert_info certs[MAX_CERTS]; */
	/* memset(certs, 0, sizeof(certs)); */

	/* find the structure associated with this fetch */
	/* For some reason, cURL thinks CURLINFO_PRIVATE should be a string?! */
	/* TODO: Do we really need curl_easy_getinfo? Our library struct provides us with all of this info already  */
	/* code.code = curl_easy_getinfo(curl_handle, CURLINFO_PRIVATE, _hideous_hack); */
	/* assert(code.code == CURLE_OK); */

	abort_fetch = f->abort;
	LOG(("done %s", nsurl_access(f->url)));

	if (abort_fetch == false && (result == CURLE_OK ||
			(result == CURLE_WRITE_ERROR && f->stopped == false))) {
		/* fetch completed normally or the server fed us a junk gzip 
		 * stream (usually in the form of garbage at the end of the 
		 * stream). Curl will have fed us all but the last chunk of 
		 * decoded data, which is sad as, if we'd received the last 
		 * chunk, too, we'd be able to render the whole object.
		 * As is, we'll just have to accept that the end of the
		 * object will be truncated in this case and leave it to
		 * the content handlers to cope. */
		if (f->stopped ||
				(!f->had_headers &&
					fetch_curl_process_headers(f)))
			; /* redirect with no body or similar */
		else
			finished = true;
	} else if (result == CURLE_PARTIAL_FILE) {
		/* CURLE_PARTIAL_FILE occurs if the received body of a
		 * response is smaller than that specified in the
		 * Content-Length header. */
		if (!f->had_headers && fetch_curl_process_headers(f))
			; /* redirect with partial body, or similar */
		else {
			finished = true;
		}
	} else if (result == CURLE_WRITE_ERROR && f->stopped) {
		/* CURLE_WRITE_ERROR occurs when fetch_curl_data
		 * returns 0, which we use to abort intentionally */
		;
	/* } else if (result == CURLE_SSL_PEER_CERTIFICATE || */
	/* 		result == CURLE_SSL_CACERT) { */
	/* 	memcpy(certs, f->cert_data, sizeof(certs)); */
	/* 	memset(f->cert_data, 0, sizeof(f->cert_data)); */
	/* 	cert = true; */
	} else {
		LOG(("Unknown cURL response code %d", result));
		error = true;
	}

	fetch_curl_stop(node);
	
	if (abort_fetch)
		; /* fetch was aborted: no callback */
	else if (finished) {
		msg.type = FETCH_FINISHED;
		__menuet__debug_out("Calling FETCH_FINISHED callback inside fetch_curl_data\n");
		fetch_send_callback(&msg, f->fetch_handle);
	/* } else if (cert) { */
	/* 	int i; */
	/* 	BIO *mem; */
	/* 	BUF_MEM *buf; */
	/* 	/\* struct ssl_cert_info ssl_certs[MAX_CERTS]; *\/ */

	/* 	for (i = 0; i < MAX_CERTS && certs[i].cert; i++) { */
	/* 		ssl_certs[i].version = */
	/* 			X509_get_version(certs[i].cert); */

	/* 		mem = BIO_new(BIO_s_mem()); */
	/* 		ASN1_TIME_print(mem, */
	/* 				X509_get_notBefore(certs[i].cert)); */
	/* 		BIO_get_mem_ptr(mem, &buf); */
	/* 		(void) BIO_set_close(mem, BIO_NOCLOSE); */
	/* 		BIO_free(mem); */
	/* 		snprintf(ssl_certs[i].not_before, */
	/* 				min(sizeof ssl_certs[i].not_before, */
	/* 					(unsigned) buf->length + 1), */
	/* 				"%s", buf->data); */
	/* 		BUF_MEM_free(buf); */

	/* 		mem = BIO_new(BIO_s_mem()); */
	/* 		ASN1_TIME_print(mem, */
	/* 				X509_get_notAfter(certs[i].cert)); */
	/* 		BIO_get_mem_ptr(mem, &buf); */
	/* 		(void) BIO_set_close(mem, BIO_NOCLOSE); */
	/* 		BIO_free(mem); */
	/* 		snprintf(ssl_certs[i].not_after, */
	/* 				min(sizeof ssl_certs[i].not_after, */
	/* 					(unsigned) buf->length + 1), */
	/* 				"%s", buf->data); */
	/* 		BUF_MEM_free(buf); */

	/* 		ssl_certs[i].sig_type = */
	/* 			X509_get_signature_type(certs[i].cert); */
	/* 		ssl_certs[i].serial = */
	/* 			ASN1_INTEGER_get( */
	/* 				X509_get_serialNumber(certs[i].cert)); */
	/* 		mem = BIO_new(BIO_s_mem()); */
	/* 		X509_NAME_print_ex(mem, */
	/* 			X509_get_issuer_name(certs[i].cert), */
	/* 			0, XN_FLAG_SEP_CPLUS_SPC | */
	/* 				XN_FLAG_DN_REV | XN_FLAG_FN_NONE); */
	/* 		BIO_get_mem_ptr(mem, &buf); */
	/* 		(void) BIO_set_close(mem, BIO_NOCLOSE); */
	/* 		BIO_free(mem); */
	/* 		snprintf(ssl_certs[i].issuer, */
	/* 				min(sizeof ssl_certs[i].issuer, */
	/* 					(unsigned) buf->length + 1), */
	/* 				"%s", buf->data); */
	/* 		BUF_MEM_free(buf); */

	/* 		mem = BIO_new(BIO_s_mem()); */
	/* 		X509_NAME_print_ex(mem, */
	/* 			X509_get_subject_name(certs[i].cert), */
	/* 			0, XN_FLAG_SEP_CPLUS_SPC | */
	/* 				XN_FLAG_DN_REV | XN_FLAG_FN_NONE); */
	/* 		BIO_get_mem_ptr(mem, &buf); */
	/* 		(void) BIO_set_close(mem, BIO_NOCLOSE); */
	/* 		BIO_free(mem); */
	/* 		snprintf(ssl_certs[i].subject, */
	/* 				min(sizeof ssl_certs[i].subject, */
	/* 					(unsigned) buf->length + 1), */
	/* 				"%s", buf->data); */
	/* 		BUF_MEM_free(buf); */

	/* 		ssl_certs[i].cert_type = */
	/* 			X509_certificate_type(certs[i].cert, */
	/* 				X509_get_pubkey(certs[i].cert)); */

	/* 		/\* and clean up *\/ */
	/* 		certs[i].cert->references--; */
	/* 		if (certs[i].cert->references == 0) */
	/* 			X509_free(certs[i].cert); */
	/* 	} */

	/* 	msg.type = FETCH_CERT_ERR; */
	/* 	msg.data.cert_err.certs = ssl_certs; */
	/* 	msg.data.cert_err.num_certs = i; */
	/* 	fetch_send_callback(&msg, f->fetch_handle); */
	} else if (error) {
		if (result != CURLE_SSL_CONNECT_ERROR) {
			msg.type = FETCH_ERROR;
			msg.data.error = fetch_error_buffer;
		} else {
			msg.type = FETCH_SSL_ERR;
		}
		fetch_send_callback(&msg, f->fetch_handle);
	}
	
	fetch_free(f->fetch_handle);	
	LOG(("Returning"));
}


/**
 * Callback function for fetch progress.
 */

/* TODO: Useful for showing the fetch's progress. Need to figure out a way to hook this up with http.obj
   More of an interface feature, but it'll be nice to have in a browser.
*/

int fetch_curl_progress(void *clientp, double dltotal, double dlnow,
			double ultotal, double ulnow)
{
	static char fetch_progress_buffer[256]; /**< Progress buffer for cURL */
	struct curl_fetch_info *f = (struct curl_fetch_info *) clientp;
	unsigned int time_now_cs;
	fetch_msg msg;

	DBG("inside fetch_curl_progress()..\n");

	if (f->abort)
		return 0;

	msg.type = FETCH_PROGRESS;
	msg.data.progress = fetch_progress_buffer;

	/* Rate limit each fetch's progress notifications to 2 a second */
#define UPDATES_PER_SECOND 2
#define UPDATE_DELAY_CS (100 / UPDATES_PER_SECOND)
	time_now_cs = wallclock();
	if (time_now_cs - f->last_progress_update < UPDATE_DELAY_CS)
		return 0;
	f->last_progress_update = time_now_cs;
#undef UPDATE_DELAY_CS
#undef UPDATES_PERS_SECOND

	if (dltotal > 0) {
		snprintf(fetch_progress_buffer, 255,
				messages_get("Progress"),
				human_friendly_bytesize(dlnow),
				human_friendly_bytesize(dltotal));
		fetch_send_callback(&msg, f->fetch_handle);
	} else {
		snprintf(fetch_progress_buffer, 255,
				messages_get("ProgressU"),
				human_friendly_bytesize(dlnow));
		fetch_send_callback(&msg, f->fetch_handle);
	}

	return 0;
}


/**
 * Ignore everything given to it.
 *
 * Used to ignore cURL debug.
 */

/*TODO: No idea what it does exactly, so let it be like it was*/

int fetch_curl_ignore_debug(struct http_msg *handle,
			    kosh_infotype type,
			    char *data,
			    size_t size,
			    void *userptr)
{
	return 0;
}

void send_header_callbacks(char *header, unsigned int header_length, struct curl_fetch_info *f)
{
  fetch_msg msg;
  int newline = 0;
  int i;

  msg.type = FETCH_HEADER;
  
  for(i = 0;i < header_length; i++)
    {
      if(header[i] == '\n')
	{
	  msg.data.header_or_data.len = i - newline;
	  msg.data.header_or_data.buf = (const uint8_t *) (header + newline);
	  /* LOG(("buf inside send_header_cb is : %.*s\n", i - newline, header+newline)); */

	  newline = i+1;
	  fetch_send_callback(&msg, f->fetch_handle);
	}
    }
}

/**
 * Callback function for cURL.
 */
/*TODO: Seems okay for now */

size_t fetch_curl_data(void *_f)
{
	struct curl_fetch_info *f = _f;
	char *data = f->curl_handle->content_ptr;
	KOSHcode code;
	fetch_msg msg;

	DBG("inside fetch_curl_data()..\n");

	/* if(f->curl_handle) */
	/*   LOG(("curl_handle is not NULL\n")); */
	/* else */
	/*   LOG(("curl_handle is NULL\n")); */

	LOG(("Will be Setting HTTP Code to : %u\n", f->curl_handle->status));	
	
	/* ensure we only have to get this information once */
	if (!f->http_code)
	{
		/* TODO: For extracting the http response code of what happened in case we don't already have that.
		   http_msg struct should have this info available for query.

		   code = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_CODE, */
		/* 			 &f->http_code); */	  	  
	  __menuet__debug_out("f->http_code is 0\n");
	  LOG(("f->http_code was 0\n"));

	  f->http_code = f->curl_handle->status;
	  fetch_set_http_code(f->fetch_handle, f->http_code);

	  /* assert(code.code == CURLE_OK); */
	}
	else
	  {
	    f->http_code = f->curl_handle->status;
	    fetch_set_http_code(f->fetch_handle, f->http_code);	  
	  }

	__menuet__debug_out("fetch_curl_data: ");
	
	LOG(("fetch->http_code is  : %ld\n", f->http_code));

	/* ignore body if this is a 401 reply by skipping it and reset
	   the HTTP response code to enable follow up fetches */

	if (f->http_code == 401)
	{
		f->http_code = 0;
		return;
		/* return size * nmemb; */
	}

	if (f->abort || (!f->had_headers && fetch_curl_process_headers(f))) {
	  __menuet__debug_out("Setting f->stopped = true\n");
	  f->stopped = true;
	  return 0;
	}

	/* send data to the caller */

	LOG(("Inside fetch_curl_data, http_code is : %li", f->http_code));
	
	if(f->http_code == 200)
	  {
	    send_header_callbacks(&f->curl_handle->header, f->curl_handle->header_length, f); 
	    LOG(("Finished sending header callbacks\n"));

	    if (f->abort) {
	      f->stopped = true;
	      return 0;	     
	    }
	  }
	else
	  LOG(("Error, http_code is not 200 but is : %u", f->http_code));

	msg.type = FETCH_DATA;
	msg.data.header_or_data.buf = (const uint8_t *) data;
	msg.data.header_or_data.len = (size_t)f->curl_handle->content_received;
	/* LOG(("FETCH_DATA with buf = %s and length = %u", msg.data.header_or_data.buf, msg.data.header_or_data.len)); */
	fetch_send_callback(&msg, f->fetch_handle);

	/* __menuet__debug_out("After Calling callback_send_fetch\n in fetch_curl_data"); */

	if (f->abort) {
		f->stopped = true;
		return 0;
	}

	/* __menuet__debug_out("Exiting fetch_curl_data"); */
	/* return size * nmemb; */
}

/**
   Convertor function for converting a header field to its dedicated buffer 
   Also terminates with a NULL character so that the string is safe for further use.
   field_name: Append the field name: to the generated string. Pass NULL for no field name.
   source -> Refers to the original data which needs to be copied into dest.
   dest -> destination. This will be allocated using malloc in the function as size is determined here.1
   
   dest uses a double pointer in order to allocate storage for the original pointer and not it's temporary copy.
   
**/

void convert_to_asciiz(char *field_name, char *source, char **dest)
{
  char *i;

  if(source == NULL)
    return;
  
  if(field_name == NULL)
    {
      for(i = source; !isspace(*i); i++);
      
      *dest = (char *)malloc(i - source + 1);	/* Allocate a big enough buffer with +1 for NULL character */
      strncpy(*dest, source, i - source); /* Copy to buffer */
      (*dest)[i - source] = '\0';
    }
  else
    {
      char *temp;
      for(i = source; !isspace(*i); i++);
      
      *dest = (char *)malloc(i - source + 1 + strlen(field_name) + 2);	/* Allocate a big enough buffer with +1 for NULL character */
      strcpy(*dest, field_name);
      temp = *dest + strlen(field_name);
      *temp = ':';
      temp++;
      *temp = ' ';
      temp++;

      strncpy(temp, source, i - source); /* Copy to buffer */
      temp[i - source] = '\0';
    }
  
}

/**
 * Callback function for headers.
 *
 * See RFC 2616 4.2.
 */

/*TODO: Seems okay for now */
/* Called when the headers have been received. A callback should be sent for each header line and not the entire thing at once*/

void fetch_curl_header(void *_f) /* Change type to curl_fetch_infO? TODO*/
{
	struct curl_fetch_info *f = _f;
	struct http_msg *handle = f->curl_handle;
	char *realm = NULL; /*Remove me ? TODO*/
	char *cookie = NULL;
	char *content_length = NULL;
	char *content_type = NULL;
	int realm_start;
	int i;
	fetch_msg msg;

	/* size *= nmemb; */ /* ???? */

	__menuet__debug_out("inside fetch_curl_header()..\n");	

	if (f->abort) {
		f->stopped = true;
		return;
	}

	f->http_code = handle->status;
	fetch_set_http_code(f->fetch_handle, f->http_code);

	LOG(("fetch->http_code is  : %ld\n", f->http_code));

	convert_to_asciiz(NULL,http_find_header_field(f->curl_handle, "location"), &f->location); 
	convert_to_asciiz("content-length", http_find_header_field(f->curl_handle, "content-length"), &content_length);
	convert_to_asciiz("set-cookie", http_find_header_field(f->curl_handle, "set-cookie"), &cookie); 
	
	f->content_length = atol(content_length);

	if(cookie)	  
	  fetch_set_cookie(f->fetch_handle, cookie+12);
	return;
	/* if(f->had_headers) */
	/*   __menuet__debug_out("curl_fetch_data BEFORE: Had headers is true!\n"); */
	/* else */
	/*   __menuet__debug_out("curl_fetch_data BEFORE: Had headers is false!\n"); */
	
	/* LOG(("Calling fetch_send_callback from fetch_curl_header")); */
	/* fetch_send_callback(&msg, f->fetch_handle); */
	/* LOG(("AFTER Calling fetch_send_callback from fetch_curl_header"));	 */

	/* if(f->had_headers) */
	/*   __menuet__debug_out("curl_fetch_data : Had headers is true!\n"); */
	/* else */
	/*   __menuet__debug_out("curl_fetch_data : Had headers is false!\n"); */

	/* Remember to use lower case names for header field names for http.obj */
	/* We extract only these fields */

	/* convert_to_asciiz("content-length", http_find_header_field(f->curl_handle, "content-length"), &content_length); */
	/* convert_to_asciiz("content-type", http_find_header_field(f->curl_handle, "content-type"), &content_type); */
	
	/* TODO: Uncomment following line and add more fields if required later */
	/* convert_to_asciiz("www-authenticate", http_find_header_field(f->curl_handle, "www-authenticate"), &realm); */

	/* LOG(("The &header is : %s", &(f->curl_handle->header))); */

	/* if(f->location) */
	/*   { */
	/*   LOG(("Setting data buf to %s with length %d", f->location, strlen(f->location))); */
	/*   msg.type = FETCH_HEADER; */
	/*   msg.data.header_or_data.buf = (const uint8_t *) f->location; */
	/*   msg.data.header_or_data.len = strlen(f->location); */
	/*   fetch_send_callback(&msg, f->fetch_handle); */
	/*   } */
	
	/* if(content_type) */
	/*   { */
	/*   LOG(("Setting data buf to %s with length %d", content_type, strlen(content_type)));	   */
	/*   f->content_length = atoi(content_type); */
	/*   msg.type = FETCH_HEADER; */
	/*   msg.data.header_or_data.buf = (const uint8_t *) content_type; */
	/*   msg.data.header_or_data.len = strlen(content_type); */
	/*   fetch_send_callback(&msg, f->fetch_handle); */
	/*   } */
	
	/* if(content_length) */
	/*   { */
	/*   f->content_length = atoi(content_length); */
	/*   msg.type = FETCH_HEADER; */
	/*   msg.data.header_or_data.buf = (const uint8_t *) content_length; */
	/*   msg.data.header_or_data.len = strlen(content_length); */
	/*   fetch_send_callback(&msg, f->fetch_handle); */
	/*   } */

	/* Set appropriate fetch properties */
	/* if(cookie) */
	/*   { */
	/*     fetch_set_cookie(f->fetch_handle, cookie); */
	/*     msg.type = FETCH_HEADER; */
	/*     msg.data.header_or_data.buf = (const uint8_t *) cookie; */
	/*     msg.data.header_or_data.len = strlen(cookie); */
	/*     fetch_send_callback(&msg, f->fetch_handle);	     */
	/*   } */

	/* if(realm) /\* Don't worry about this for now , fix it later TODO *\/ */
	/*   {     */
	/*     /\* For getting the realm, this was used as an example : ('WWW-Authenticate: Basic realm="My Realm"')  *\/	    */
	    
	/*     for(i = strlen("www-authenticate: "); realm[i]; i++) */
	/*       if(realm[i] == '"') { */
	/* 	realm_start = i+1; */
	/* 	break; */
	/*       }		 */
	    
	/*     for(i = realm_start ; realm[i]; i++) */
	/*       if(realm[i] == '"') { */
	/* 	realm[i] = '\0'; */
	/* 	break; */
	/*       } */
	   
	/*     f->realm = realm; */
	/*   }	 */

 /* TODO: call the fetch_callback for www authenticate field and any other fields that will be added here later */

	/* LOG(("fetch->http_code is  ( AT THE END of f_c_header): %ld\n", f->http_code)); */
	/* __menuet__debug_out("Leaving fetch_curl_header\n"); */
	
	/* if(f->had_headers) */
	/*   __menuet__debug_out("curl_fetch_data : Had headers is true!\n"); */
	/* else */
	/*   __menuet__debug_out("curl_fetch_data : Had headers is false!\n"); */
	
	/* f->http_code = handle->status; */
	/* fetch_set_http_code(f->fetch_handle, f->http_code); */
	
/* #define SKIP_ST(o) for (i = (o); i < (int) size && (data[i] == ' ' || data[i] == '\t'); i++) */

/* 	if (12 < size && strncasecmp(data, "Location:", 9) == 0) { */
/* 		/\* extract Location header *\/ */
/* 		free(f->location); */
/* 		f->location = malloc(size); */
/* 		if (!f->location) { */
/* 			LOG(("malloc failed")); */
/* 			return size; */
/* 		} */
/* 		SKIP_ST(9); */
/* 		strncpy(f->location, data + i, size - i); */
/* 		f->location[size - i] = '\0'; */
/* 		for (i = size - i - 1; i >= 0 && */
/* 				(f->location[i] == ' ' || */
/* 				f->location[i] == '\t' || */
/* 				f->location[i] == '\r' || */
/* 				f->location[i] == '\n'); i--) */
/* 			f->location[i] = '\0'; */
/* 	} else if (15 < size && strncasecmp(data, "Content-Length:", 15) == 0) { */
/* 		/\* extract Content-Length header *\/ */
/* 		SKIP_ST(15); */
/* 		if (i < (int)size && '0' <= data[i] && data[i] <= '9') */
/* 			f->content_length = atol(data + i); */
/* 	} else if (17 < size && strncasecmp(data, "WWW-Authenticate:", 17) == 0) { */
/* 		/\* extract the first Realm from WWW-Authenticate header *\/ */
/* 		SKIP_ST(17); */

/* 		while (i < (int) size - 5 && */
/* 				strncasecmp(data + i, "realm", 5)) */
/* 			i++; */
/* 		while (i < (int) size - 1 && data[++i] != '"') */
/* 			/\* *\/; */
/* 		i++; */

/* 		if (i < (int) size) { */
/* 			size_t end = i; */

/* 			while (end < size && data[end] != '"') */
/* 				++end; */

/* 			if (end < size) { */
/* 				free(f->realm); */
/* 				f->realm = malloc(end - i + 1); */
/* 				if (f->realm != NULL) { */
/* 					strncpy(f->realm, data + i, end - i); */
/* 					f->realm[end - i] = '\0'; */
/* 				} */
/* 			} */
/* 		} */
/* 	} else if (11 < size && strncasecmp(data, "Set-Cookie:", 11) == 0) { */
/* 		/\* extract Set-Cookie header *\/ */
/* 		SKIP_ST(11); */

/* 		fetch_set_cookie(f->fetch_handle, &data[i]); */
/* 	} */

/* 	return size; */
/* #undef SKIP_ST */
}

/**
 * Find the status code and content type and inform the caller.
 *
 * Return true if the fetch is being aborted.
 */
/*TODO: Handling the http status codes here and performing accordingly*/

bool fetch_curl_process_headers(struct curl_fetch_info *f)
{
        long http_code;
	KOSHcode code;
	fetch_msg msg;	
	
	__menuet__debug_out("Setting had_headers to true\n");

	f->had_headers = true;        
	
	http_code = f->curl_handle->status;
	LOG(("Inside fetch_curl_process_headers..HTTP CODE : %ld\n", http_code));

	if (!f->http_code)
	  {
	  /* TODO: Handle this like another similar piece of code in the file with HTTP_CODE_CURLINFO */
	    /* code = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_CODE, */
	    /* 			 &f->http_code); */
	    /* Replaced with this :  */
	    /* Have a fetch_set_http_code here? TODO*/
	    
	    f->http_code = http_code;
	    fetch_set_http_code(f->fetch_handle, f->http_code);
	    /* assert(code.code == CURLE_OK); */
	}

	LOG(("HTTP status code %li\n", http_code));
		
	if (http_code == 304 && !f->post_urlenc && !f->post_multipart) {
		/* Not Modified && GET request */
		msg.type = FETCH_NOTMODIFIED;
		fetch_send_callback(&msg, f->fetch_handle);
		return true;
	}

	/* handle HTTP redirects (3xx response codes) */
	if (300 <= http_code && http_code < 400) {	  
	  LOG(("FETCH_REDIRECT, '%s'", f->location));
	  msg.type = FETCH_REDIRECT;
	  msg.data.redirect = f->location;
	  fetch_send_callback(&msg, f->fetch_handle);
	  return true;	
	}

	/* handle HTTP 401 (Authentication errors) */
	if (http_code == 401) {
                msg.type = FETCH_AUTH;
		msg.data.auth.realm = f->realm;
		fetch_send_callback(&msg, f->fetch_handle);
		return true;
	}

	/* handle HTTP errors (non 2xx response codes) */
	if (f->only_2xx && strncmp(nsurl_access(f->url), "http", 4) == 0 &&
			(http_code < 200 || 299 < http_code)) {
		msg.type = FETCH_ERROR;
		DBG("FETCH_ERROR\n");		
		msg.data.error = messages_get("Not2xx");
		fetch_send_callback(&msg, f->fetch_handle);
		return true;
	}

	if (f->abort)
		return true;

	DBG("Returning false from fetch_curl_process_headers()\n");

	return false;
}


/**
 * Convert a list of struct ::fetch_multipart_data to a list of
 * struct curl_httppost for libcurl.
 */

/* TODO: Not sure how to handle multipart data yet, but hopefully it'll be figured out soon */
/* TODO: Seems like the forms that are being created use sequential fields, so we can probably craft the same */
/*       using post from http,obj */

struct curl_httppost *
fetch_curl_post_convert(const struct fetch_multipart_data *control)
{
	struct curl_httppost *post = 0, *last = 0;
	/* TODO: CURLFORMcode code; */

	DBG("inside fetch_curl_post_convert()..\n");

	for (; control; control = control->next) {
		if (control->file) {
			char *leafname = 0;

			leafname = filename_from_path(control->value);

			if (leafname == NULL)
				continue;

			/* We have to special case filenames of "", so curl
			 * a) actually attempts the fetch and
			 * b) doesn't attempt to open the file ""
			 */
			if (control->value[0] == '\0') {
				/* dummy buffer - needs to be static so
				 * pointer's still valid when we go out
				 * of scope (not that libcurl should be
				 * attempting to access it, of course). */
				/* static char buf; */
				/* code = curl_formadd(&post, &last, */
				/* 	CURLFORM_COPYNAME, control->name, */
				/* 	CURLFORM_BUFFER, control->value, */
				/* 	/\* needed, as basename("") == "." *\/ */
				/* 	CURLFORM_FILENAME, "", */
				/* 	CURLFORM_BUFFERPTR, &buf, */
				/* 	CURLFORM_BUFFERLENGTH, 0, */
				/* 	CURLFORM_CONTENTTYPE, */
				/* 		"application/octet-stream", */
				/* 	CURLFORM_END); */
				/* if (code != CURL_FORMADD_OK) */
				/* 	LOG(("curl_formadd: %d (%s)", */
				/* 		code, control->name)); */
			} else {
				/* char *mimetype = fetch_mimetype(control->value); */
				/* code = curl_formadd(&post, &last, */
				/* 	CURLFORM_COPYNAME, control->name, */
				/* 	CURLFORM_FILE, control->value, */
				/* 	CURLFORM_FILENAME, leafname, */
				/* 	CURLFORM_CONTENTTYPE, */
				/* 	(mimetype != 0 ? mimetype : "text/plain"), */
				/* 	CURLFORM_END); */
				/* if (code != CURL_FORMADD_OK) */
				/* 	LOG(("curl_formadd: %d (%s=%s)", */
				/* 		code, control->name, */
				/* 		control->value)); */
				/* free(mimetype); */
			}
			free(leafname);
		}
		else {
			/* code = curl_formadd(&post, &last, */
			/* 		CURLFORM_COPYNAME, control->name, */
			/* 		CURLFORM_COPYCONTENTS, control->value, */
			/* 		CURLFORM_END); */
			/* if (code != CURL_FORMADD_OK) */
			/* 	LOG(("curl_formadd: %d (%s=%s)", code, */
			/* 			control->name, */
			/* 			control->value)); */
		}
	}

	return post;
}


/**
 * OpenSSL Certificate verification callback
 * Stores certificate details in fetch struct.
 */

/* TODO: SSL Stuff, useless as of now */

/* int fetch_curl_verify_callback(int preverify_ok, X509_STORE_CTX *x509_ctx) */
/* { */
/* 	X509 *cert = X509_STORE_CTX_get_current_cert(x509_ctx); */
/* 	int depth = X509_STORE_CTX_get_error_depth(x509_ctx); */
/* 	int err = X509_STORE_CTX_get_error(x509_ctx); */
/* 	struct curl_fetch_info *f = X509_STORE_CTX_get_app_data(x509_ctx); */

/* 	/\* save the certificate by incrementing the reference count and */
/* 	 * keeping a pointer *\/ */
/* 	if (depth < MAX_CERTS && !f->cert_data[depth].cert) { */
/* 		f->cert_data[depth].cert = cert; */
/* 		f->cert_data[depth].err = err; */
/* 		cert->references++; */
/* 	} */

/* 	return preverify_ok; */
/* } */


/**
 * OpenSSL certificate chain verification callback
 * Verifies certificate chain, setting up context for fetch_curl_verify_callback
 */

/* int fetch_curl_cert_verify_callback(X509_STORE_CTX *x509_ctx, void *parm) */
/* { */
/* 	int ok; */

/* 	/\* Store fetch struct in context for verify callback *\/ */
/* 	ok = X509_STORE_CTX_set_app_data(x509_ctx, parm); */

/* 	/\* and verify the certificate chain *\/ */
/* 	if (ok) */
/* 		ok = X509_verify_cert(x509_ctx); */

/* 	return ok; */
/* } */

struct curl_slist *curl_slist_append(struct curl_slist * list, const char * string )
{
  struct curl_slist *newnode = NULL;
  DBG("Inside curl_slist_append..\n");
  newnode = malloc(sizeof(struct curl_slist));

  if(newnode == NULL)
    return NULL;

  strcpy(newnode->data, string);
  
  newnode->next = NULL;

  if(!list)
    {
      list = newnode;
    }
  else /*list isn't null*/
    {
      struct curl_slist *temp = list;

      while(temp->next!=NULL)
	temp = temp->next;
      
      temp->next = newnode;
    }

  return list;
}  

void curl_slist_free_all(struct curl_slist *list)
{
  struct curl_slist *temp = list;
  DBG("Inside curl_slist_free_all..\n");

  while(list)
    {
      temp = list->next;
      free(list);
      list = temp;
    }
}

int curl_multi_add_handle(struct fetch_info_slist **multi_handle, struct curl_fetch_info *new_fetch)
{  
  DBG("Inside curl_multi_add_handle..Adding handle\n");

  if(*multi_handle == NULL)
    {
      struct fetch_info_slist *new_node = (struct fetch_info_slist *)malloc(sizeof(struct fetch_info_slist));

      if(new_node == NULL || new_fetch == NULL) //add failture for malloc here TODO
	return CURLM_FAILED;
      
      new_node->fetch_info = new_fetch;
      new_node->handle = new_fetch->curl_handle;
      new_node->fetch_curl_header_called = false;
      *multi_handle = new_node;
      (*multi_handle)->next = NULL;
    }
  else
    {
      struct fetch_info_slist *temp = *multi_handle;
      struct fetch_info_slist *new_node = (struct fetch_info_slist *)malloc(sizeof(struct fetch_info_slist));
            
      if(new_node == NULL || new_fetch == NULL) //add failture for malloc here TODO
	return CURLM_FAILED;

      while(temp->next)
	{
	  temp = temp->next;
	}

      new_node->fetch_info = new_fetch;
      new_node->handle = new_fetch->curl_handle;
      new_node->next = NULL;
      new_node->fetch_curl_header_called = false;
      temp->next = new_node;
    }
  
  return CURLM_OK;
}

/* When this function returns, it is assured that the multi list does not contain the node to be deleted. 
   If it was, it was deleted. Else, the list is left unchanged 
*/

/* This can be sped up a lot by using hash tables or the like for removal to be more speedy :)
LTG
*/

/* struct fetch_info_slist *curl_multi_remove_handle(struct fetch_info_slist *multi_handle, struct fetch_info_slist *node_to_delete) */
struct fetch_info_slist *curl_multi_remove_handle(struct fetch_info_slist *multi_handle, struct curl_fetch_info *fetch_to_delete)
{
  struct fetch_info_slist *temp = multi_handle;
  char *zz;
  int pr;

  nsurl_get(fetch_to_delete->url, NSURL_WITH_FRAGMENT, &zz, &pr);
  LOG(("inside curl_multi_remove_handle for %s..\n", zz));

  if(multi_handle == NULL || fetch_to_delete == NULL)
    return multi_handle;

  if(temp->fetch_info == fetch_to_delete) /* special case for first node deletion */
    {      
      multi_handle = multi_handle->next;
      LOG(("Removed handle\n"));
      /* free(temp);       */ /* Probably shouldnt free. Let other routines free this TODO */
    }
  else /* If the data is present in any consecutive node */
    {
      struct fetch_info_slist *temp2 = multi_handle->next;
      
      while(temp2)
	{	  	  
	  if(temp2->fetch_info == fetch_to_delete) 
	    {	      	      
	      temp->next = temp2->next;
	      /* free(temp2); */ /* Shouldnt free node here. Let others handle it (for cache etc). */
	      LOG(("Removed handle\n"));
	      break;
	    }
	  else
	    {
	      temp = temp2;
	      temp2 = temp2->next;
	    }
	}   
    }
  LOG(("Returning"));
  /*TODO : http_free should be called here?*/
  return multi_handle;
}

/* TODO: Get rid of the curl functions soon DONE*/

 /* TODO: Actually a function to return a blank handle. The name is misleading right now */
struct http_msg * curl_easy_init(void)
{
  struct http_msg *new_handle;    
  return new_handle;
}

int curl_multi_perform(struct fetch_info_slist *multi_list)
{
  struct fetch_info_slist *temp = multi_list;
  
  while(temp) {
    http_receive(temp->handle);      
    temp = temp->next;
  }
}

void curl_easy_cleanup(struct http_msg *handle)
{
  http_free(handle);
}
