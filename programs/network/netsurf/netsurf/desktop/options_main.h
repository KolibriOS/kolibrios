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
 * Option available on all platforms
 *
 * Non-platform specific options can be added by editing this file 
 *
 * Platform specific options should be added in the platform options.h.
 *
 * The following types of options are supported:
 *  - bool (OPTION_BOOL) boolean
 *  - int (OPTION_INTEGER) integer
 *  - colour (OPTION_COLOUR) colour
 *  - char* (OPTION_STRING) must be allocated on heap, may be NULL
 */

#ifndef _NETSURF_DESKTOP_OPTIONS_MAIN_H_
#define _NETSURF_DESKTOP_OPTIONS_MAIN_H_

#define NSOPTION_MAIN_DEFINE			\
	/** An HTTP proxy should be used. */	\
	bool http_proxy;			\
	/** Hostname of proxy. */		\
	char *http_proxy_host;			\
	/** Proxy port. */			\
	int http_proxy_port;			\
	/** Proxy authentication method. */	\
	int http_proxy_auth;			\
	/** Proxy authentication user name */	\
	char *http_proxy_auth_user;		\
	/** Proxy authentication password */	\
	char *http_proxy_auth_pass;		\
	/** Default font size / 0.1pt. */	\
	int font_size;				\
	/** Minimum font size. */		\
	int font_min_size;			\
	/** Default sans serif font */		\
	char *font_sans;			\
	/** Default serif font */		\
	char *font_serif;			\
	/** Default monospace font */		\
	char *font_mono;			\
	/** Default cursive font */		\
	char *font_cursive;			\
	/** Default fantasy font */		\
	char *font_fantasy;			\
	/** Accept-Language header. */		\
	char *accept_language;			\
	/** Accept-Charset header. */		\
	char *accept_charset;					\
	/** Preferred maximum size of memory cache / bytes. */	\
	int memory_cache_size;					\
	/** Preferred expiry size of disc cache / bytes. */	\
	int disc_cache_size;					\
	/** Preferred expiry age of disc cache / days. */	\
	int disc_cache_age;					\
	/** Whether to block advertisements */			\
	bool block_ads;						\
	/** Disable website tracking, see	                \
	 * http://www.w3.org/Submission/2011/SUBM-web-tracking-protection-20110224/#dnt-uas */	\
	bool do_not_track;					\
	/** Minimum GIF animation delay */			\
	int minimum_gif_delay;					\
	/** Whether to send the referer HTTP header */		\
	bool send_referer;					\
	/** Whether to fetch foreground images */		\
	bool foreground_images;					\
	/** Whether to fetch background images */		\
	bool background_images;					\
	/** Whether to animate images */			\
	bool animate_images;					\
	/** Whether to execute javascript */			\
	bool enable_javascript;					\
	/** How many days to retain URL data for */		\
	int expire_url;						\
	/** Default font family */				\
	int font_default;					\
	/** ca-bundle location */				\
	char *ca_bundle;					\
	/** ca-path location */					\
	char *ca_path;						\
	/** Cookie file location */				\
	char *cookie_file;					\
	/** Cookie jar location */				\
	char *cookie_jar;					\
	/** Home page location */				\
	char *homepage_url;					\
	/** search web from url bar */				\
	bool search_url_bar;					\
	/** URL completion in url bar */			\
	bool url_suggestion;					\
	/** default web search provider */			\
	int search_provider;					\
	/** default x position of new windows */		\
	int window_x;						\
	/** default y position of new windows */		\
	int window_y;						\
	/** default width of new windows */			\
	int window_width;					\
	/** default height of new windows */			\
	int window_height;					\
	/** width of screen when above options were saved */	\
	int window_screen_width;				\
	/** height of screen when above options were saved */	\
	int window_screen_height;				\
	/** default size of status bar vs. h scroll bar */	\
	int toolbar_status_width;				\
	/** default window scale */				\
	int scale;							\
	/* Whether to reflow web pages while objects are fetching */	\
	bool incremental_reflow;					\
	/* Minimum time between HTML reflows while objects are fetching */ \
	unsigned int min_reflow_period; /* time in cs */		\
	bool core_select_menu;						\
	/** top margin of exported page */				\
	int margin_top;							\
	/** bottom margin of exported page */				\
	int margin_bottom;						\
	/** left margin of exported page */				\
	int margin_left;						\
	/** right margin of exported page*/				\
	int margin_right;						\
	/** scale of exported content*/					\
	int export_scale;						\
	/** suppressing images in printed content*/			\
	bool suppress_images;						\
	/** turning off all backgrounds for printed content*/		\
	bool remove_backgrounds;					\
	/** turning on content loosening for printed content*/		\
	bool enable_loosening;						\
	/** compression of PDF documents*/				\
	bool enable_PDF_compression;					\
	/** setting a password and encoding PDF documents*/		\
	bool enable_PDF_password;					\
									\
	/* Fetcher configuration */					\
	/** Maximum simultaneous active fetchers */			\
	int max_fetchers;						\
	/** Maximum simultaneous active fetchers per host.		\
	 * (<=option_max_fetchers else it makes no sense) Note that	\
	 * rfc2616 section 8.1.4 says that there should be no more	\
	 * than two keepalive connections per host. None of the main	\
	 * browsers follow this as it slows page fetches down		\
	 * considerably.  See						\
	 * https://bugzilla.mozilla.org/show_bug.cgi?id=423377#c4	\
	 */								\
	int max_fetchers_per_host;					\
	/** Maximum number of inactive fetchers cached.  The total	\
	 * number of handles netsurf will therefore have open is this	\
	 * plus option_max_fetchers.					\
	 */								\
	int max_cached_fetch_handles;					\
	/** Suppress debug output from cURL. */				\
	bool suppress_curl_debug;					\
									\
	/** Whether to allow target="_blank" */				\
	bool target_blank;						\
									\
	/** Whether second mouse button opens in new tab */		\
	bool button_2_tab;						\
									\
	/* system colours */						\
	colour sys_colour_ActiveBorder;					\
	colour sys_colour_ActiveCaption;				\
	colour sys_colour_AppWorkspace;					\
	colour sys_colour_Background;					\
	colour sys_colour_ButtonFace;					\
	colour sys_colour_ButtonHighlight;				\
	colour sys_colour_ButtonShadow;					\
	colour sys_colour_ButtonText;					\
	colour sys_colour_CaptionText;					\
	colour sys_colour_GrayText;					\
	colour sys_colour_Highlight;					\
	colour sys_colour_HighlightText;				\
	colour sys_colour_InactiveBorder;				\
	colour sys_colour_InactiveCaption;				\
	colour sys_colour_InactiveCaptionText;				\
	colour sys_colour_InfoBackground;				\
	colour sys_colour_InfoText;					\
	colour sys_colour_Menu;						\
	colour sys_colour_MenuText;					\
	colour sys_colour_Scrollbar;					\
	colour sys_colour_ThreeDDarkShadow;				\
	colour sys_colour_ThreeDFace;					\
	colour sys_colour_ThreeDHighlight;				\
	colour sys_colour_ThreeDLightShadow;				\
	colour sys_colour_ThreeDShadow;					\
	colour sys_colour_Window;					\
	colour sys_colour_WindowFrame;					\
	colour sys_colour_WindowText

#define NSOPTION_MAIN_DEFAULTS				\
	.http_proxy = false,				\
	.http_proxy_host = NULL,			\
	.http_proxy_port = 8080,			\
	.http_proxy_auth = OPTION_HTTP_PROXY_AUTH_NONE,	\
	.http_proxy_auth_user = NULL,			\
	.http_proxy_auth_pass = NULL,			\
	.font_size = 128,				\
	.font_min_size = 85,				\
	.font_sans = NULL,				\
	.font_serif = NULL,				\
	.font_mono = NULL,				\
	.font_cursive = NULL,				\
	.font_fantasy = NULL,				\
	.accept_language = NULL,			\
	.accept_charset = NULL,				\
	.memory_cache_size = 12 * 1024 * 1024,		\
	.disc_cache_size = 1024 * 1024 * 1024,		\
	.disc_cache_age = 28,				\
	.block_ads = false,				\
	.do_not_track = false,				\
	.minimum_gif_delay = 10,			\
	.send_referer = true,				\
	.foreground_images = true,			\
	.background_images = true,			\
	.animate_images = true,				\
	.expire_url = 28,				\
	.font_default = PLOT_FONT_FAMILY_SANS_SERIF,	\
	.ca_bundle = NULL,				\
	.ca_path = NULL,				\
	.cookie_file = NULL,				\
	.cookie_jar = NULL,				\
	.homepage_url = NULL,				\
	.search_url_bar = false,			\
	.url_suggestion = true,				\
	.search_provider = 0,				\
	.window_x = 0,					\
	.window_y = 0,					\
	.window_width = 0,				\
	.window_height = 0,				\
	.window_screen_width = 0,			\
	.window_screen_height = 0,			\
	.toolbar_status_width = 6667,			\
	.scale = 100,					\
	.incremental_reflow = true,			\
	.min_reflow_period = DEFAULT_REFLOW_PERIOD,	\
	.core_select_menu = false,			\
	.margin_top = DEFAULT_MARGIN_TOP_MM,		\
	.margin_bottom = DEFAULT_MARGIN_BOTTOM_MM,	\
	.margin_left = DEFAULT_MARGIN_LEFT_MM,		\
	.margin_right = DEFAULT_MARGIN_RIGHT_MM,	\
	.export_scale = DEFAULT_EXPORT_SCALE * 100,	\
	.suppress_images = false,			\
	.remove_backgrounds = false,			\
	.enable_loosening = true,			\
	.enable_PDF_compression = true,			\
	.enable_PDF_password = false,			\
	.max_fetchers = 24,				\
	.max_fetchers_per_host = 5,			\
	.max_cached_fetch_handles = 6,			\
	.suppress_curl_debug = true,			\
	.target_blank = true,				\
	.button_2_tab = true,				\
	.enable_javascript = true

#define NSOPTION_MAIN_SYS_COLOUR_DEFAULTS		\
	.sys_colour_ActiveBorder = 0x00000000,		\
	.sys_colour_ActiveCaption = 0x00000000,		\
	.sys_colour_AppWorkspace = 0x00000000,		\
	.sys_colour_Background = 0x00000000,		\
	.sys_colour_ButtonFace = 0x00000000,		\
	.sys_colour_ButtonHighlight = 0x00000000,	\
	.sys_colour_ButtonShadow = 0x00000000,		\
	.sys_colour_ButtonText = 0x00000000,		\
	.sys_colour_CaptionText = 0x0000000,		\
	.sys_colour_GrayText = 0x00000000,		\
	.sys_colour_Highlight = 0x00000000,		\
	.sys_colour_HighlightText = 0x00000000,		\
	.sys_colour_InactiveBorder = 0x00000000,	\
	.sys_colour_InactiveCaption = 0x00000000,	\
	.sys_colour_InactiveCaptionText = 0x00000000,	\
	.sys_colour_InfoBackground = 0x00000000,	\
	.sys_colour_InfoText = 0x00000000,		\
	.sys_colour_Menu = 0x00000000,			\
	.sys_colour_MenuText = 0x0000000,		\
	.sys_colour_Scrollbar = 0x0000000,		\
	.sys_colour_ThreeDDarkShadow = 0x000000,	\
	.sys_colour_ThreeDFace = 0x000000,		\
	.sys_colour_ThreeDHighlight = 0x000000,		\
	.sys_colour_ThreeDLightShadow = 0x000000,	\
	.sys_colour_ThreeDShadow = 0x000000,		\
	.sys_colour_Window = 0x000000,			\
	.sys_colour_WindowFrame = 0x000000,		\
	.sys_colour_WindowText = 0x000000


#define NSOPTION_MAIN_TABLE						\
	{ "http_proxy",		OPTION_BOOL,	&nsoptions.http_proxy }, \
	{ "http_proxy_host",	OPTION_STRING,	&nsoptions.http_proxy_host }, \
	{ "http_proxy_port",	OPTION_INTEGER,	&nsoptions.http_proxy_port }, \
	{ "http_proxy_auth",	OPTION_INTEGER,	&nsoptions.http_proxy_auth }, \
	{ "http_proxy_auth_user", OPTION_STRING, &nsoptions.http_proxy_auth_user }, \
	{ "http_proxy_auth_pass", OPTION_STRING, &nsoptions.http_proxy_auth_pass }, \
	{ "font_size",		OPTION_INTEGER,	&nsoptions.font_size },	\
	{ "font_min_size",	OPTION_INTEGER,	&nsoptions.font_min_size }, \
	{ "font_sans",		OPTION_STRING,	&nsoptions.font_sans },	\
	{ "font_serif",		OPTION_STRING,	&nsoptions.font_serif }, \
	{ "font_mono",		OPTION_STRING,	&nsoptions.font_mono },	\
	{ "font_cursive",	OPTION_STRING,	&nsoptions.font_cursive }, \
	{ "font_fantasy",	OPTION_STRING,	&nsoptions.font_fantasy }, \
	{ "accept_language",	OPTION_STRING,	&nsoptions.accept_language }, \
	{ "accept_charset",	OPTION_STRING,	&nsoptions.accept_charset }, \
	{ "memory_cache_size",	OPTION_INTEGER,	&nsoptions.memory_cache_size },	\
	{ "disc_cache_size",	OPTION_INTEGER,	&nsoptions.disc_cache_size },	\
	{ "disc_cache_age",	OPTION_INTEGER,	&nsoptions.disc_cache_age }, \
	{ "block_advertisements", OPTION_BOOL,	&nsoptions.block_ads },	\
	{ "do_not_track", OPTION_BOOL,	&nsoptions.do_not_track },	\
	{ "minimum_gif_delay",	OPTION_INTEGER,	&nsoptions.minimum_gif_delay },	\
	{ "send_referer",	OPTION_BOOL,	&nsoptions.send_referer }, \
	{ "foreground_images",	OPTION_BOOL,	&nsoptions.foreground_images },	\
	{ "background_images",	OPTION_BOOL,	&nsoptions.background_images },	\
	{ "animate_images",	OPTION_BOOL,	&nsoptions.animate_images }, \
	{ "enable_javascript",	OPTION_BOOL,	&nsoptions.enable_javascript},	\
	{ "expire_url",		OPTION_INTEGER,	&nsoptions.expire_url }, \
	{ "font_default",	OPTION_INTEGER,	&nsoptions.font_default }, \
	{ "ca_bundle",		OPTION_STRING,	&nsoptions.ca_bundle },	\
	{ "ca_path",		OPTION_STRING,	&nsoptions.ca_path },	\
	{ "cookie_file",	OPTION_STRING,	&nsoptions.cookie_file }, \
	{ "cookie_jar",		OPTION_STRING,	&nsoptions.cookie_jar }, \
	{ "homepage_url",	OPTION_STRING,	&nsoptions.homepage_url }, \
	{ "search_url_bar",	OPTION_BOOL,	&nsoptions.search_url_bar}, \
	{ "search_provider",	OPTION_INTEGER,	&nsoptions.search_provider}, \
	{ "url_suggestion",	OPTION_BOOL,	&nsoptions.url_suggestion }, \
	{ "window_x",		OPTION_INTEGER,	&nsoptions.window_x },	\
	{ "window_y",		OPTION_INTEGER,	&nsoptions.window_y },	\
	{ "window_width",	OPTION_INTEGER,	&nsoptions.window_width }, \
	{ "window_height",	OPTION_INTEGER,	&nsoptions.window_height }, \
	{ "window_screen_width", OPTION_INTEGER, &nsoptions.window_screen_width }, \
	{ "window_screen_height", OPTION_INTEGER, &nsoptions.window_screen_height }, \
	{ "toolbar_status_size", OPTION_INTEGER, &nsoptions.toolbar_status_width }, \
	{ "scale",		OPTION_INTEGER,	&nsoptions.scale },	\
	{ "incremental_reflow",	OPTION_BOOL,	&nsoptions.incremental_reflow }, \
	{ "min_reflow_period",	OPTION_INTEGER,	&nsoptions.min_reflow_period },	\
 	{ "core_select_menu",	OPTION_BOOL,	&nsoptions.core_select_menu }, \
		/* Fetcher options */					\
	{ "max_fetchers",	OPTION_INTEGER,	&nsoptions.max_fetchers }, \
	{ "max_fetchers_per_host", OPTION_INTEGER, &nsoptions.max_fetchers_per_host }, \
	{ "max_cached_fetch_handles", OPTION_INTEGER, &nsoptions.max_cached_fetch_handles }, \
	{ "suppress_curl_debug",OPTION_BOOL,	&nsoptions.suppress_curl_debug }, \
	{ "target_blank",	OPTION_BOOL,	&nsoptions.target_blank }, \
	{ "button_2_tab",	OPTION_BOOL,	&nsoptions.button_2_tab }, \
		/* PDF / Print options*/				\
	{ "margin_top",		OPTION_INTEGER,	&nsoptions.margin_top},	\
	{ "margin_bottom",	OPTION_INTEGER,	&nsoptions.margin_bottom}, \
	{ "margin_left",	OPTION_INTEGER,	&nsoptions.margin_left}, \
	{ "margin_right",	OPTION_INTEGER,	&nsoptions.margin_right}, \
 	{ "export_scale",	OPTION_INTEGER,	&nsoptions.export_scale}, \
	{ "suppress_images",	OPTION_BOOL,	&nsoptions.suppress_images}, \
	{ "remove_backgrounds",	OPTION_BOOL,	&nsoptions.remove_backgrounds},	\
	{ "enable_loosening",	OPTION_BOOL,	&nsoptions.enable_loosening}, \
 	{ "enable_PDF_compression", OPTION_BOOL, &nsoptions.enable_PDF_compression}, \
 	{ "enable_PDF_password", OPTION_BOOL,	&nsoptions.enable_PDF_password}, \
									\
		/* System colours */					\
	{ "sys_colour_ActiveBorder",OPTION_COLOUR,&nsoptions.sys_colour_ActiveBorder },	\
	{ "sys_colour_ActiveCaption",OPTION_COLOUR,&nsoptions.sys_colour_ActiveCaption }, \
	{ "sys_colour_AppWorkspace",OPTION_COLOUR,&nsoptions.sys_colour_AppWorkspace },	\
	{ "sys_colour_Background",OPTION_COLOUR,&nsoptions.sys_colour_Background }, \
	{ "sys_colour_ButtonFace",OPTION_COLOUR,&nsoptions.sys_colour_ButtonFace }, \
	{ "sys_colour_ButtonHighlight",OPTION_COLOUR,&nsoptions.sys_colour_ButtonHighlight }, \
	{ "sys_colour_ButtonShadow",OPTION_COLOUR,&nsoptions.sys_colour_ButtonShadow },	\
	{ "sys_colour_ButtonText",OPTION_COLOUR,&nsoptions.sys_colour_ButtonText }, \
	{ "sys_colour_CaptionText",OPTION_COLOUR,&nsoptions.sys_colour_CaptionText }, \
	{ "sys_colour_GrayText",OPTION_COLOUR,&nsoptions.sys_colour_GrayText },	\
	{ "sys_colour_Highlight",OPTION_COLOUR,&nsoptions.sys_colour_Highlight }, \
	{ "sys_colour_HighlightText",OPTION_COLOUR,&nsoptions.sys_colour_HighlightText }, \
	{ "sys_colour_InactiveBorder",OPTION_COLOUR,&nsoptions.sys_colour_InactiveBorder }, \
	{ "sys_colour_InactiveCaption",OPTION_COLOUR,&nsoptions.sys_colour_InactiveCaption }, \
	{ "sys_colour_InactiveCaptionText",OPTION_COLOUR,&nsoptions.sys_colour_InactiveCaptionText }, \
	{ "sys_colour_InfoBackground",OPTION_COLOUR,&nsoptions.sys_colour_InfoBackground }, \
	{ "sys_colour_InfoText",OPTION_COLOUR,&nsoptions.sys_colour_InfoText },	\
	{ "sys_colour_Menu",OPTION_COLOUR,&nsoptions.sys_colour_Menu },	\
	{ "sys_colour_MenuText",OPTION_COLOUR,&nsoptions.sys_colour_MenuText },	\
	{ "sys_colour_Scrollbar",OPTION_COLOUR,&nsoptions.sys_colour_Scrollbar }, \
	{ "sys_colour_ThreeDDarkShadow",OPTION_COLOUR,&nsoptions.sys_colour_ThreeDDarkShadow },	\
	{ "sys_colour_ThreeDFace",OPTION_COLOUR,&nsoptions.sys_colour_ThreeDFace }, \
	{ "sys_colour_ThreeDHighlight",OPTION_COLOUR,&nsoptions.sys_colour_ThreeDHighlight }, \
	{ "sys_colour_ThreeDLightShadow", OPTION_COLOUR,&nsoptions.sys_colour_ThreeDLightShadow }, \
	{ "sys_colour_ThreeDShadow", OPTION_COLOUR,&nsoptions.sys_colour_ThreeDShadow },	\
	{ "sys_colour_Window", OPTION_COLOUR,&nsoptions.sys_colour_Window }, \
	{ "sys_colour_WindowFrame", OPTION_COLOUR,&nsoptions.sys_colour_WindowFrame }, \
	{ "sys_colour_WindowText", OPTION_COLOUR,&nsoptions.sys_colour_WindowText }

#endif
