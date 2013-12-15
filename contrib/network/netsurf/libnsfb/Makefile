# Component settings
COMPONENT := nsfb
COMPONENT_VERSION := 0.0.2
# Default to a static library
COMPONENT_TYPE ?= lib-static

# Setup the tooling
PREFIX ?= /opt/netsurf
NSSHARED ?= $(PREFIX)/share/netsurf-buildsystem
include $(NSSHARED)/makefiles/Makefile.tools

# Reevaluate when used, as BUILDDIR won't be defined yet
TESTRUNNER = test/runtest.sh $(BUILDDIR) $(EXEEXT)

# Toolchain flags
WARNFLAGS := -Wall -Wextra -Wundef -Wpointer-arith -Wcast-align \
	-Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes \
	-Wmissing-declarations -Wnested-externs -Werror -pedantic \
	-Wno-overlength-strings # For nsglobe.c
CFLAGS := -g -std=c99 -D_BSD_SOURCE -D_POSIX_C_SOURCE=200112L \
	-I$(CURDIR)/include/ -I$(CURDIR)/src $(WARNFLAGS) $(CFLAGS)

NSFB_XCB_PKG_NAMES := xcb xcb-icccm xcb-image xcb-keysyms xcb-atom

$(eval $(call pkg_config_package_available,NSFB_VNC_AVAILABLE,libvncserver))
$(eval $(call pkg_config_package_available,NSFB_SDL_AVAILABLE,sdl))
$(eval $(call pkg_config_package_available,NSFB_XCB_AVAILABLE,$(NSFB_XCB_PKG_NAMES)))

ifeq ($(NSFB_SDL_AVAILABLE),yes)
  $(eval $(call pkg_config_package_add_flags,sdl,CFLAGS))
  $(eval $(call pkg_config_package_add_flags,sdl,TESTCFLAGS,TESTLDFLAGS))

  REQUIRED_PKGS := $(REQUIRED_PKGS) sdl
endif 

ifeq ($(NSFB_XCB_AVAILABLE),yes)
  # Size hint allocators were removed in xcb-icccm 0.3.0
  $(eval $(call pkg_config_package_min_version,NSFB_XCB_ICCCM_SIZE_HINTS,xcb-icccm,0.3.0))
  ifeq ($(NSFB_XCB_ICCCM_SIZE_HINTS),yes)
    CFLAGS := $(CFLAGS) -DNSFB_NEED_HINTS_ALLOC
  endif

  # xcb-icccm 0.3.8 introduced an additional "icccm_" in function names
  $(eval $(call pkg_config_package_min_version,NSFB_XCB_ICCCM_API_PREFIX,xcb-icccm,0.3.8))
  ifeq ($(NSFB_XCB_ICCCM_API_PREFIX),yes)
    CFLAGS := $(CFLAGS) -DNSFB_NEED_ICCCM_API_PREFIX
  endif

  # xcbproto 1.6 incorporated atoms previously found in xcb_atom
  # However, libxcb <1.3 did not report xcbproto versions. Assume xcbproto 1.5 in this case.
  $(eval $(call pkg_config_package_min_version,NSFB_HAVE_MODERN_XCB,xcb,1.3))
  ifeq ($(NSFB_HAVE_MODERN_XCB),yes)
    $(eval $(call pkg_config_get_variable,NSFB_XCBPROTO_VERSION,xcb,xcbproto_version))
    NSFB_XCBPROTO_MAJOR_VERSION := $(word 1,$(subst ., ,$(NSFB_XCBPROTO_VERSION)))
    NSFB_XCBPROTO_MINOR_VERSION := $(word 2,$(subst ., ,$(NSFB_XCBPROTO_VERSION)))
    CFLAGS := $(CFLAGS) -DNSFB_XCBPROTO_MAJOR_VERSION=$(NSFB_XCBPROTO_MAJOR_VERSION)
    CFLAGS := $(CFLAGS) -DNSFB_XCBPROTO_MINOR_VERSION=$(NSFB_XCBPROTO_MINOR_VERSION)
  else
    CFLAGS := $(CFLAGS) -DNSFB_XCBPROTO_MAJOR_VERSION=1
    CFLAGS := $(CFLAGS) -DNSFB_XCBPROTO_MINOR_VERSION=5
  endif

  $(eval $(call pkg_config_package_add_flags,$(NSFB_XCB_PKG_NAMES),CFLAGS))
  $(eval $(call pkg_config_package_add_flags,$(NSFB_XCB_PKG_NAMES),TESTCFLAGS,TESTLDFLAGS))

  REQUIRED_PKGS := $(REQUIRED_PKGS) $(NSFB_XCB_PKG_NAMES)

  $(eval $(call pkg_config_package_available,NSFB_XCB_UTIL_AVAILABLE,xcb-util))
  ifeq ($(NSFB_XCB_UTIL_AVAILABLE),yes)
    REQUIRED_PKGS := $(REQUIRED_PKGS) xcb-util
  endif
endif

ifeq ($(NSFB_VNC_AVAILABLE),yes)
  $(eval $(call pkg_config_package_add_flags,libvncserver,CFLAGS))
  $(eval $(call pkg_config_package_add_flags,libvncserver,TESTCFLAGS,TESTLDFLAGS))

  REQUIRED_PKGS := $(REQUIRED_PKGS) libvncserver
endif 

TESTLDFLAGS := -lm -Wl,--whole-archive -l$(COMPONENT) -Wl,--no-whole-archive $(TESTLDFLAGS)

include $(NSBUILD)/Makefile.top

# Extra installation rules
I := /include
INSTALL_ITEMS := $(INSTALL_ITEMS) $(I):include/libnsfb.h
INSTALL_ITEMS := $(INSTALL_ITEMS) $(I):include/libnsfb_plot.h
INSTALL_ITEMS := $(INSTALL_ITEMS) $(I):include/libnsfb_plot_util.h
INSTALL_ITEMS := $(INSTALL_ITEMS) $(I):include/libnsfb_event.h
INSTALL_ITEMS := $(INSTALL_ITEMS) $(I):include/libnsfb_cursor.h
INSTALL_ITEMS := $(INSTALL_ITEMS) /lib/pkgconfig:lib$(COMPONENT).pc.in
INSTALL_ITEMS := $(INSTALL_ITEMS) /lib:$(OUTPUT)
