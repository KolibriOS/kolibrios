include Makefile.sources

AM_CPPFLAGS = $(DEFINES) \
	$(VISIBILITY_CFLAGS) \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/loader \
	-I$(top_srcdir)/src/gallium/include \
	-I$(top_srcdir)/src/gallium/auxiliary \
	-I$(top_srcdir)/src/gallium/winsys

noinst_LTLIBRARIES = libpipe_loader.la
noinst_LTLIBRARIES += libpipe_loader_client.la

if HAVE_DRM_LOADER_GALLIUM
AM_CFLAGS = $(LIBDRM_CFLAGS)

COMMON_SOURCES += $(DRM_SOURCES)

COMMON_LIBADD = \
	$(top_builddir)/src/loader/libloader.la

endif

libpipe_loader_la_CFLAGS  = \
	$(GALLIUM_PIPE_LOADER_DEFINES) \
	$(AM_CFLAGS) $(AM_CPPFLAGS)
libpipe_loader_la_SOURCES = $(COMMON_SOURCES)
libpipe_loader_la_LIBADD  = $(COMMON_LIBADD) \
	$(GALLIUM_PIPE_LOADER_LIBS)

libpipe_loader_client_la_CFLAGS  = \
	$(GALLIUM_PIPE_LOADER_CLIENT_DEFINES) \
	$(AM_CFLAGS) $(AM_CPPFLAGS)
libpipe_loader_client_la_SOURCES = $(COMMON_SOURCES)
libpipe_loader_client_la_LIBADD  = $(COMMON_LIBADD) \
	$(GALLIUM_PIPE_LOADER_CLIENT_LIBS)
