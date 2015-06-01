include Makefile.sources
include $(top_srcdir)/src/gallium/Automake.inc

AM_CFLAGS = \
	-I$(top_srcdir)/include/D3D9 \
	$(GALLIUM_CFLAGS) \
	$(VISIBILITY_CFLAGS)

noinst_LTLIBRARIES = libninetracker.la

libninetracker_la_SOURCES = $(C_SOURCES)

EXTRA_DIST = README
