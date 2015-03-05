/* This file is part of the FreeType project */

/* ft_conf.h for MSDOS */


/* we need the following because there are some typedefs in this file */

#ifndef FT_CONF_H
#define FT_CONF_H

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define if you have a working `mmap' system call.  */
#undef HAVE_MMAP

/* Define if you have the <stdlib.h> header file.  */
#define HAVE_STDLIB_H

/* Define if you have the getpagesize function.  */
#define HAVE_GETPAGESIZE

/* Define if you have the memcpy function.  */
#define HAVE_MEMCPY

/* Define if you have the memmove function.  */
#define HAVE_MEMMOVE

/* Define if you have the valloc function.  */
#undef HAVE_VALLOC

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H

/* Define if you have the <unistd.h> header file.  */
#if defined( __EMX__ ) || defined( __DJGPP__ ) || defined( __GO32__ )
/* some compilers are known to have <unistd.h>;       */
/* add yours if needed, and report to us the update.  */
#define HAVE_UNISTD_H
#else
/* most MS-DOS compilers lack <unistd.h> */
#undef  HAVE_UNISTD_H
#endif

/* Define if you need <conio.h> for console I/O functions.  */
#ifdef __EMX__
#define HAVE_CONIO_H
#endif

/* Define if you have the <locale.h> header file.  */
#undef HAVE_LOCALE_H

/* Define if you have the <libintl.h> header file.  */
#undef HAVE_LIBINTL_H

/* Define if you have the libintl library.  */
#undef HAVE_LIBINTL

#define HAVE_PRINT_FUNCTION 1

#define Print( format, ap )

#include <limits.h>
#if   UINT_MAX == 0xFFFF
#define SIZEOF_INT  2
#elif UINT_MAX == 0xFFFFFFFF
#define SIZEOF_INT  4
#else
#error "Unsupported number of bytes in `int' type!"
#endif

#define SIZEOF_LONG 4


#define  TT_CONFIG_OPTION_EXTEND_ENGINE

#define  TT_CONFIG_OPTION_GRAY_SCALING

#undef   TT_CONFIG_OPTION_NO_INTERPRETER

#define  TT_CONFIG_OPTION_INTERPRETER_SWITCH

#undef TT_CONFIG_OPTION_STATIC_INTERPRETER

#undef  TT_CONFIG_OPTION_STATIC_RASTER

#undef  TT_CONFIG_OPTION_THREAD_SAFE

#undef  DEBUG_LEVEL_TRACE
#undef  DEBUG_LEVEL_ERROR


#if SIZEOF_INT == 4

  typedef signed int      TT_Int32;
  typedef unsigned int    TT_Word32;

#elif SIZEOF_LONG == 4

  typedef signed long     TT_Int32;
  typedef unsigned long   TT_Word32;

#else
#error "no 32bit type found"
#endif

#if SIZEOF_LONG == 8

/* LONG64 must be defined when a 64-bit type is available */
/* INT64 must then be defined to this type..              */
#define LONG64
#define INT64   long

#else

/* GCC provides the non-ANSI 'long long' 64-bit type.  You can activate    */
/* by defining the TT_USE_LONG_LONG macro in 'ft_conf.h'.  Note that this  */
/* will produce many -ansi warnings during library compilation.            */
#ifdef TT_USE_LONG_LONG

#define LONG64
#define INT64   long long

#endif /* TT_USE_LONG_LONG */
#endif

#endif /* FT_CONF_H */


/* END */
