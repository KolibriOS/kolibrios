# Copyright (c) 2007 Intel Corporation. All Rights Reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sub license, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
# 
# The above copyright notice and this permission notice (including the
# next paragraph) shall be included in all copies or substantial portions
# of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
# IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
# ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

SUBDIRS = shaders

AM_CPPFLAGS = \
	-DPTHREADS		\
	$(DRM_CFLAGS)		\
	$(LIBVA_DEPS_CFLAGS)	\
	$(NULL)

driver_cflags = \
	-Wall			\
	-fvisibility=hidden	\
	$(NULL)

driver_ldflags = \
	-module -avoid-version	\
	-no-undefined		\
	-Wl,--no-undefined	\
	$(NULL)

driver_libs = \
	-lpthread -lm -ldl	\
	$(DRM_LIBS) -ldrm_intel	\
	$(LIBVA_DEPS_LIBS)	\
	$(NULL)

source_c = \
	dso_utils.c		\
	gen6_mfc.c		\
	gen6_mfc_common.c	\
	gen6_mfd.c		\
	gen6_vme.c		\
	gen7_vme.c		\
	gen7_mfc.c		\
	gen7_mfd.c		\
	gen75_mfd.c		\
	gen75_mfc.c		\
	gen8_mfc.c		\
	gen8_mfd.c		\
	gen8_vme.c		\
	gen75_picture_process.c	\
	gen75_vme.c		\
	gen75_vpp_gpe.c  	\
	gen75_vpp_vebox.c	\
	i965_avc_bsd.c		\
	i965_avc_hw_scoreboard.c\
	i965_avc_ildb.c		\
	i965_decoder_utils.c	\
	i965_device_info.c	\
	i965_drv_video.c	\
	i965_encoder.c		\
	i965_encoder_utils.c	\
	i965_media.c		\
	i965_media_h264.c	\
	i965_media_mpeg2.c	\
	i965_gpe_utils.c	\
	i965_post_processing.c	\
	gen8_post_processing.c	\
	i965_render.c		\
	gen8_render.c		\
	intel_batchbuffer.c	\
	intel_batchbuffer_dump.c\
	intel_driver.c		\
	intel_memman.c		\
	object_heap.c		\
	intel_media_common.c		\
	$(NULL)

source_h = \
	dso_utils.h		\
	gen6_mfc.h		\
	gen6_mfd.h		\
	gen6_vme.h		\
	gen7_mfd.h		\
	gen75_picture_process.h	\
	gen75_vpp_gpe.h 	\
	gen75_vpp_vebox.h	\
	i965_avc_bsd.h		\
	i965_avc_hw_scoreboard.h\
	i965_avc_ildb.h		\
	i965_decoder.h		\
	i965_decoder_utils.h	\
	i965_defines.h          \
	i965_drv_video.h        \
	i965_encoder.h		\
	i965_encoder_utils.h	\
	i965_media.h            \
	i965_media_h264.h	\
	i965_media_mpeg2.h      \
	i965_mutext.h		\
	i965_gpe_utils.h	\
	i965_pciids.h		\
	i965_post_processing.h	\
	i965_render.h           \
	i965_structs.h		\
	intel_batchbuffer.h     \
	intel_batchbuffer_dump.h\
	intel_compiler.h	\
	intel_driver.h          \
	intel_media.h           \
	intel_memman.h          \
	object_heap.h           \
	sysdeps.h		\
	va_backend_compat.h	\
	i965_fourcc.h           \
	$(NULL)

i965_drv_video_la_LTLIBRARIES	= i965_drv_video.la
i965_drv_video_ladir		= $(LIBVA_DRIVERS_PATH)
i965_drv_video_la_CFLAGS	= $(driver_cflags)
i965_drv_video_la_LDFLAGS	= $(driver_ldflags)
i965_drv_video_la_LIBADD	= $(driver_libs)
i965_drv_video_la_SOURCES	= $(source_c)
noinst_HEADERS			= $(source_h)

if USE_X11
source_c			+= i965_output_dri.c
source_h			+= i965_output_dri.h
endif

if USE_WAYLAND
source_c			+= i965_output_wayland.c
source_h			+= i965_output_wayland.h
driver_cflags			+= $(WAYLAND_CFLAGS)
endif

# Wayland protocol
protocol_source_h = wayland-drm-client-protocol.h
i965_output_wayland.c: $(protocol_source_h)
@wayland_scanner_rules@

DIST_SUBDIRS = $(SUBDIRS) wayland

# Extra clean files so that maintainer-clean removes *everything*
MAINTAINERCLEANFILES = Makefile.in config.h.in
