/* opj_config.h.  Generated from opj_config.h.in by configure.  */
/* opj_config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 0

/* Define to 1 if fseeko (and presumably ftello) exists and is declared. */
#define HAVE_FSEEKO 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 0

/* Define to 1 if you have the `lcms' library (-llcms). */
#define HAVE_LIBLCMS 0

/* define to 1 if you have lcms version 1.x */
#define HAVE_LIBLCMS1 0

/* define to 1 if you have lcms version 2.x */
/* #undef HAVE_LIBLCMS2 */

/* define to 1 if you have libpng */
#define HAVE_LIBPNG 1

/* define to 1 if you have libtiff */
#define HAVE_LIBTIFF 0

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 0

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* Name of package */
#define PACKAGE "openjpeg-1.4.0"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "http://code.google.com/p/openjpeg/"

/* Define to the full name of this package. */
#define PACKAGE_NAME "OpenJPEG"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "OpenJPEG 1.4.0"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "openjpeg-1.4.0"

/* Define to the home page for this package. */
#define PACKAGE_URL "http://www.openjpeg.org/"

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.4.0"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Version number of package */
#define VERSION "1.4.0"

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
#define _FILE_OFFSET_BITS 64

/* Define to 1 to make fseeko visible on some hosts (e.g. glibc 2.2). */
/* #undef _LARGEFILE_SOURCE */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */
