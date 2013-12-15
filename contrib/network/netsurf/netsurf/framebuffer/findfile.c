/*
 * Copyright 2008 Daniel Silverstone <dsilvers@netsurf-browser.org>
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

#include <limits.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#include "utils/filepath.h"
#include "utils/log.h"
#include "utils/url.h"
#include "desktop/gui.h"

#include "framebuffer/findfile.h"

char **respaths; /** resource search path vector */

/** Create an array of valid paths to search for resources.
 *
 * The idea is that all the complex path computation to find resources
 * is performed here, once, rather than every time a resource is
 * searched for.
 */
char **
fb_init_resource(const char *resource_path)
{
	char **pathv; /* resource path string vector */
	char **respath; /* resource paths vector */
	const char *lang = NULL;
	
	LOG(("Findfile: %s", resource_path));
	pathv = filepath_path_to_strvec(resource_path);

	LOG(("Findfile2: %s", pathv));
	respath = filepath_generate(pathv, &lang);

	filepath_free_strvec(pathv);

	return respath;
}


char *path_to_url(const char *path)
{
	int urllen; 
	char *url; 

	if (path == NULL)
		return NULL;

	urllen = strlen(path) + FILE_SCHEME_PREFIX_LEN + 1;
	url = malloc(urllen);

	if (*path == '/') {
		path++; /* file: paths are already absolute */
	} 

	snprintf(url, urllen, "%s%s", FILE_SCHEME_PREFIX, path);

	LOG(("Findfile path2url: %s", url));
	return url;
}


char *url_to_path(const char *url)
{
	char *path;
	char *respath;
	url_func_result res; /* result from url routines */

	res = url_path(url, &path);
	if (res != URL_FUNC_OK) {
		return NULL;
	}

	res = url_unescape(path, &respath);
	free(path);
	if (res != URL_FUNC_OK) {
		return NULL;
	}

LOG(("Findfile url2path: %s", respath));
	return respath;
}

nsurl *gui_get_resource_url(const char *path)
{
	char buf[PATH_MAX];
	char *raw;
	nsurl *url = NULL;

	if (strcmp(path, "favicon.ico") == 0)
		path = "favicon.png";

	raw = path_to_url(filepath_sfind(respaths, buf, path));
	
	LOG(("Findfile gui: %s", raw));
	if (raw != NULL) {
		nsurl_create(raw, &url);
		free(raw);
	}

	return url;
}

/*
 * Local Variables:
 * c-basic-offset: 8
 * End:
 */

