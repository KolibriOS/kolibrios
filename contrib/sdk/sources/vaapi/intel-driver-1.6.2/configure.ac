# intel-driver package version number
m4_define([intel_driver_major_version], [1])
m4_define([intel_driver_minor_version], [6])
m4_define([intel_driver_micro_version], [2])
m4_define([intel_driver_pre_version],   [0])
m4_define([intel_driver_version],
          [intel_driver_major_version.intel_driver_minor_version.intel_driver_micro_version])
m4_if(intel_driver_pre_version, [0], [], [
m4_append([intel_driver_version], intel_driver_pre_version, [.pre])
])

# libva minimum version requirement
m4_define([va_api_version], [0.38])
m4_define([libva_package_version], [1.6.0])

# libdrm minimum version requirement
m4_define([libdrm_version], [2.4.45])

AC_PREREQ([2.57])
AC_INIT([intel_driver], [intel_driver_version], [haihao.xiang@intel.com],
        [libva-intel-driver])
AC_CONFIG_SRCDIR([Makefile.am])
AM_INIT_AUTOMAKE([1.9 tar-ustar])

AC_CONFIG_HEADERS([src/config.h])

INTEL_DRIVER_MAJOR_VERSION=intel_driver_major_version
INTEL_DRIVER_MINOR_VERSION=intel_driver_minor_version
INTEL_DRIVER_MICRO_VERSION=intel_driver_micro_version
AC_DEFINE([INTEL_DRIVER_MAJOR_VERSION], [intel_driver_major_version], [Major version of the driver])
AC_DEFINE([INTEL_DRIVER_MINOR_VERSION], [intel_driver_minor_version], [Minor version of the driver])
AC_DEFINE([INTEL_DRIVER_MICRO_VERSION], [intel_driver_micro_version], [Micro version of the driver])
AC_DEFINE([INTEL_DRIVER_PRE_VERSION], [intel_driver_pre_version], [Preversion of the driver])

INTEL_DRIVER_LT_LDFLAGS="-avoid-version"
AC_SUBST(INTEL_DRIVER_LT_LDFLAGS)

dnl Use pretty build output with automake >= 1.11
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])], [
    AM_DEFAULT_VERBOSITY=1
    AC_SUBST(AM_DEFAULT_VERBOSITY)
])

AC_ARG_ENABLE(drm,
    [AC_HELP_STRING([--enable-drm],
                    [build with VA/DRM API support @<:@default=yes@:>@])],
    [], [enable_drm="yes"])

AC_ARG_ENABLE(x11,
    [AC_HELP_STRING([--enable-x11],
                    [build with VA/X11 API support @<:@default=yes@:>@])],
    [], [enable_x11="yes"])

AC_ARG_ENABLE([wayland],
    [AC_HELP_STRING([--enable-wayland],
                    [build with VA/Wayland API support @<:@default=yes@:>@])],
    [], [enable_wayland="yes"])

AC_ARG_ENABLE([hybrid-codec],
    [AC_HELP_STRING([--enable-hybrid-codec],
                    [build with hybrid codec support @<:@default=no@:>@])],
    [], [enable_hybrid_codec="no"])

AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AC_PROG_CC
AM_PROG_CC_C_O

AC_C_BIGENDIAN
AC_HEADER_STDC
AC_SYS_LARGEFILE
AC_CHECK_LIB([m], [sin])
AC_CHECK_FUNCS([log2f])
AC_CHECK_PROGS([PYTHON2], [python2 python])

LIBVA_PACKAGE_VERSION=libva_package_version
AC_SUBST(LIBVA_PACKAGE_VERSION)

dnl Check for recent enough DRM
LIBDRM_VERSION=libdrm_version
PKG_CHECK_MODULES([DRM], [libdrm >= $LIBDRM_VERSION])
AC_SUBST(LIBDRM_VERSION)

dnl Check for gen4asm
PKG_CHECK_MODULES(GEN4ASM, [intel-gen4asm >= 1.9], [gen4asm=yes], [gen4asm=no])
AC_PATH_PROG([GEN4ASM], [intel-gen4asm])
AS_IF([test "x$GEN4ASM" = "x" ], [AM_CONDITIONAL(HAVE_GEN4ASM, false)],
    [AM_CONDITIONAL(HAVE_GEN4ASM, [test x$gen4asm = xyes])])

dnl Check for git
AC_ARG_VAR([GIT], [Path to git program, if any])
AC_PATH_PROG([GIT], [git])
AM_CONDITIONAL([HAVE_GIT], [test -n "$GIT"])

dnl Check for VA-API
PKG_CHECK_MODULES(LIBVA_DEPS,     [libva >= va_api_version])

dnl Check for VA/DRM API
USE_DRM="$enable_drm"
if test "$USE_DRM" = "yes"; then
    PKG_CHECK_MODULES(LIBVA_DRM_DEPS, [libva-drm],
      [AC_DEFINE([HAVE_VA_DRM], [1], [Defined to 1 if VA/DRM API is enabled])],
      [USE_DRM="no"])

    # Check for <drm_fourcc.h>
    if test "$USE_DRM" = "yes"; then
        saved_CPPFLAGS="$CPPFLAGS"
        CPPFLAGS="$CPPFLAGS $DRM_CFLAGS"
        AC_CHECK_HEADERS([drm_fourcc.h], [:], [USE_DRM="no"])
        CPPFLAGS="$saved_CPPFLAGS"
    fi
fi
AM_CONDITIONAL(USE_DRM, test "$USE_DRM" = "yes")

if test "$enable_hybrid_codec" = "yes"; then
    AC_DEFINE([HAVE_HYBRID_CODEC], [1], [Defined to 1 if hybrid codec is needed])
fi

VA_VERSION=`$PKG_CONFIG --modversion libva`
VA_MAJOR_VERSION=`echo "$VA_VERSION" | cut -d'.' -f1`
VA_MINOR_VERSION=`echo "$VA_VERSION" | cut -d'.' -f2`
VA_MICRO_VERSION=`echo "$VA_VERSION" | cut -d'.' -f3`
VA_VERSION_STR="$VA_VERSION"

va_full_version_int=`expr ${VA_MAJOR_VERSION:-0} "*" 1000000 + \
                          ${VA_MINOR_VERSION:-0} "*" 10000   + \
                          ${VA_MICRO_VERSION:-0} "*" 100     + \
                          0`
VA_DRIVER_INIT_FUNC="__vaDriverInit_${VA_MAJOR_VERSION}_${VA_MINOR_VERSION}"
AC_DEFINE_UNQUOTED([VA_DRIVER_INIT_FUNC], [$VA_DRIVER_INIT_FUNC],
    [Define driver entry-point])

dnl Check for VA/DRM API
USE_X11="$enable_x11"
if test "$USE_X11" = "yes"; then
    PKG_CHECK_MODULES(LIBVA_X11_DEPS, [libva-x11],
      [AC_DEFINE([HAVE_VA_X11], [1], [Defined to 1 if VA/X11 API is enabled])],
      [USE_X11="no"])
fi
AM_CONDITIONAL(USE_X11, test "$USE_X11" = "yes")

dnl Check for VA-API drivers path
AC_ARG_VAR(LIBVA_DRIVERS_PATH, [drivers install path])
if test -z "$LIBVA_DRIVERS_PATH"; then
    AC_MSG_CHECKING([for VA drivers path])
    LIBVA_DRIVERS_PATH=`$PKG_CONFIG libva --variable driverdir`
fi
if test -z "$LIBVA_DRIVERS_PATH"; then
    LIBVA_DRIVERS_PATH="${libdir}/xorg/modules/drivers"
fi
AC_MSG_RESULT([$LIBVA_DRIVERS_PATH])
AC_SUBST(LIBVA_DRIVERS_PATH)

# Check for EGL
if test "$enable_wayland" = "yes"; then
    enable_egl="yes"
fi

USE_EGL="no"
if test "$enable_egl" = "yes"; then
    PKG_CHECK_MODULES([EGL], [egl], [USE_EGL="yes"], [USE_EGL="no"])
    saved_CPPFLAGS="$CPPFLAGS"
    saved_LIBS="$LIBS"
    CPPFLAGS="$CPPFLAGS $EGL_CFLAGS"
    LIBS="$LIBS $EGL_LIBS"
    AC_CHECK_HEADERS([EGL/egl.h], [:], [USE_EGL="no"])
    AC_CHECK_LIB([EGL], [eglGetDisplay], [:], [USE_EGL="no"])
    CPPFLAGS="$saved_CPPFLAGS"
    LIBS="$saved_LIBS"
fi
AM_CONDITIONAL(USE_EGL, test "$USE_EGL" = "yes")

# Check for Wayland
USE_WAYLAND="no"
if test "$enable_wayland" = "yes"; then
    PKG_CHECK_MODULES([WAYLAND], [wayland-client], [USE_WAYLAND="yes"], [:])
    PKG_CHECK_MODULES([LIBVA_WAYLAND_DEPS], [libva-wayland],
        [AC_DEFINE([HAVE_VA_WAYLAND], [1], [Defined to 1 if VA/Wayland API is enabled])],
        [USE_WAYLAND="no"])
fi
AM_CONDITIONAL(USE_WAYLAND, test "$USE_WAYLAND" = "yes")

m4_ifdef([WAYLAND_SCANNER_RULES],
    [WAYLAND_SCANNER_RULES(['$(top_builddir)/src/wayland'])],
    [wayland_scanner_rules=""; AC_SUBST(wayland_scanner_rules)])

AC_OUTPUT([
    Makefile
    debian.upstream/Makefile 
    src/Makefile
    src/shaders/Makefile
    src/shaders/h264/Makefile
    src/shaders/h264/ildb/Makefile
    src/shaders/h264/mc/Makefile
    src/shaders/mpeg2/Makefile
    src/shaders/mpeg2/vld/Makefile
    src/shaders/post_processing/Makefile
    src/shaders/post_processing/gen5_6/Makefile
    src/shaders/post_processing/gen7/Makefile
    src/shaders/post_processing/gen75/Makefile
    src/shaders/post_processing/gen8/Makefile
    src/shaders/post_processing/gen9/Makefile
    src/shaders/render/Makefile
    src/shaders/utils/Makefile
    src/shaders/vme/Makefile
    src/wayland/Makefile
])

dnl Print summary
BACKENDS=""
AS_IF([test "$USE_DRM" = "yes"], [BACKENDS="$BACKENDS drm"])
AS_IF([test "$USE_X11" = "yes"], [BACKENDS="$BACKENDS x11"])
AS_IF([test "$USE_WAYLAND" = "yes"], [BACKENDS="$BACKENDS wayland"])

echo
echo $PACKAGE configuration summary:
echo
echo VA-API version ................... : $VA_VERSION_STR
echo VA-API drivers path .............. : $LIBVA_DRIVERS_PATH
echo Windowing systems ................ : $BACKENDS
echo
