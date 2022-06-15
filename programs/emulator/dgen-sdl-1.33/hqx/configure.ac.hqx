AC_INIT([hqx], [1.0], [grom@zeminvaders.net])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_CC
AC_LANG([C])
AC_C_INLINE
AC_C_BIGENDIAN
AM_INIT_AUTOMAKE([-Wall -Werror])
AM_PROG_CC_C_O

AC_CHECK_HEADER([IL/il.h],,AC_MSG_ERROR("DevIL il.h required"))

AC_ARG_ENABLE([debug],
	      [AS_HELP_STRING([--enable-debug], [Compile a debug version of the library])],
          [CFLAGS="-Wall -g -O0"],
	      [CFLAGS="-O3"])

AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
HQX_LTVERSION=1:0:0
AC_SUBST([HQX_LTVERSION])

dnl Win32
AS_CASE([$host],
        [*mingw32*], [PLATFORM="MINGW32"],
        [*cygwin*], [CFLAGS="$CFLAGS -mno-cygwin" PLATFORM="CYGWIN"],
        [*darwin*], [PLATFORM="OSX"])
AC_SUBST([PLATFORM])

AM_CONDITIONAL([WINDOWS_BUILD],
	       [test "x$PLATFORM" = "xMINGW32" -o "x$PLATFORM" = "xCYGWIN"])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
