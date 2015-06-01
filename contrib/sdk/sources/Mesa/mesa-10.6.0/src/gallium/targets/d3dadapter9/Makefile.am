# Copyright © 2012 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

include $(top_srcdir)/src/gallium/Automake.inc

AM_CFLAGS = \
	-I$(top_srcdir)/include/D3D9 \
	-I$(top_srcdir)/src/loader \
	-I$(top_srcdir)/src/mapi/ \
	-I$(top_srcdir)/src/mesa/ \
	-I$(top_srcdir)/src/mesa/drivers/dri/common/ \
	-I$(top_builddir)/src/mesa/drivers/dri/common/ \
	-I$(top_srcdir)/src/gallium/winsys \
	-I$(top_srcdir)/src/gallium/state_trackers/nine \
	$(GALLIUM_TARGET_CFLAGS) \
	$(VISIBILITY_CFLAGS)

if HAVE_GALLIUM_STATIC_TARGETS
AM_CPPFLAGS = \
	-DNINE_TARGET \
	-DGALLIUM_STATIC_TARGETS=1

else

AM_CPPFLAGS = \
	-DPIPE_SEARCH_DIR=\"$(libdir)/gallium-pipe\" \
	$(GALLIUM_PIPE_LOADER_DEFINES)

endif

ninedir = $(D3D_DRIVER_INSTALL_DIR)
nine_LTLIBRARIES = d3dadapter9.la

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = d3d.pc

d3dadapter9_la_SOURCES = \
	getproc.c \
	drm.c

d3dadapter9_la_LDFLAGS = \
	-shared \
	-shrext .so \
	-module \
	-no-undefined \
	-version-number $(NINE_MAJOR):$(NINE_MINOR):$(NINE_TINY) \
	$(GC_SECTIONS) \
	$(LD_NO_UNDEFINED)

if HAVE_LD_VERSION_SCRIPT
d3dadapter9_la_LDFLAGS += \
	-Wl,--version-script=$(top_srcdir)/src/gallium/targets/d3dadapter9/d3dadapter9.sym
endif # HAVE_LD_VERSION_SCRIPT

d3dadapter9_la_LIBADD = \
	$(top_builddir)/src/gallium/auxiliary/libgalliumvl_stub.la \
	$(top_builddir)/src/gallium/auxiliary/libgallium.la \
	$(top_builddir)/src/gallium/state_trackers/nine/libninetracker.la \
	$(top_builddir)/src/util/libmesautil.la \
	$(top_builddir)/src/gallium/winsys/sw/wrapper/libwsw.la \
	$(EXPAT_LIBS) \
	$(GALLIUM_COMMON_LIB_DEPS)

EXTRA_d3dadapter9_la_DEPENDENCIES = d3dadapter9.sym
EXTRA_DIST = d3dadapter9.sym

TARGET_DRIVERS =
TARGET_CPPFLAGS =
TARGET_LIB_DEPS = $(top_builddir)/src/loader/libloader.la

include $(top_srcdir)/src/gallium/drivers/i915/Automake.inc

include $(top_srcdir)/src/gallium/drivers/ilo/Automake.inc

include $(top_srcdir)/src/gallium/drivers/nouveau/Automake.inc

include $(top_srcdir)/src/gallium/drivers/r300/Automake.inc
include $(top_srcdir)/src/gallium/drivers/r600/Automake.inc
include $(top_srcdir)/src/gallium/drivers/radeonsi/Automake.inc

include $(top_srcdir)/src/gallium/drivers/svga/Automake.inc

include $(top_srcdir)/src/gallium/drivers/freedreno/Automake.inc

include $(top_srcdir)/src/gallium/drivers/vc4/Automake.inc

include $(top_srcdir)/src/gallium/drivers/softpipe/Automake.inc
include $(top_srcdir)/src/gallium/drivers/llvmpipe/Automake.inc

if HAVE_GALLIUM_STATIC_TARGETS

d3dadapter9_la_CPPFLAGS = $(AM_CPPFLAGS) $(TARGET_CPPFLAGS)
d3dadapter9_la_LIBADD += $(TARGET_LIB_DEPS) \
	$(TARGET_RADEON_WINSYS) $(TARGET_RADEON_COMMON)

else # HAVE_GALLIUM_STATIC_TARGETS

d3dadapter9_la_LIBADD += \
	$(top_builddir)/src/gallium/auxiliary/pipe-loader/libpipe_loader.la \
	$(GALLIUM_PIPE_LOADER_WINSYS_LIBS) \
	$(GALLIUM_PIPE_LOADER_LIBS)

endif # HAVE_GALLIUM_STATIC_TARGETS

if HAVE_MESA_LLVM
nodist_EXTRA_d3dadapter9_la_SOURCES = dummy.cpp
d3dadapter9_la_LDFLAGS += $(LLVM_LDFLAGS)
d3dadapter9_la_LIBADD += $(LLVM_LIBS)
endif

d3dadapterdir = $(includedir)/d3dadapter
d3dadapter_HEADERS = \
	$(top_srcdir)/include/d3dadapter/d3dadapter9.h \
	$(top_srcdir)/include/d3dadapter/drm.h \
	$(top_srcdir)/include/d3dadapter/present.h
