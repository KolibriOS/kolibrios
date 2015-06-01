include Makefile.sources
include $(top_srcdir)/src/gallium/Automake.inc

AM_CFLAGS = \
	$(GALLIUM_DRIVER_CFLAGS) \
	$(MSVC2008_COMPAT_CFLAGS)

noinst_LTLIBRARIES = libtrace.la

libtrace_la_SOURCES = $(C_SOURCES)

EXTRA_DIST = SConscript \
	README \
	trace.xsl
