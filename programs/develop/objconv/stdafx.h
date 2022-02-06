/****************************   stdafx.h    **********************************
* Author:        Agner Fog
* Date created:  2006-07-15
* Last modified: 2006-07-15
* Project:       objconv
* Module:        stdafx.h
* Description:
* Header file including other header files for the project.
*
* Copyright 2006-2008 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/

#ifndef OBJCONV_STDAFX_H
#define OBJCONV_STDAFX_H

// System header files
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#ifdef _MSC_VER                  // For Microsoft compiler only:
  #include <io.h>                // File in/out function headers
  #include <fcntl.h>
  #include <sys/stat.h>
  #define stricmp _stricmp       // For later versions of MS compiler
  #define strnicmp _strnicmp     // For later versions of MS compiler
  #define filelength _filelength // For later versions of MS compiler
#else                            // For Gnu and other compilers:
  #define stricmp  strcasecmp    // Alternative function names
  #define strnicmp strncasecmp
#endif

// Project header files. The order of these files is not arbitrary.
#include "maindef.h"      // Constants, integer types, etc.
#include "error.h"        // Error handler
#include "containers.h"   // Classes for data buffers and dynamic memory allocation
#include "coff.h"         // COFF files structure
#include "elf.h"          // ELF files structure
#include "omf.h"          // OMF files structure
#include "macho.h"        // Mach-O files structure
#include "disasm.h"       // Structures and classes for disassembler
#include "converters.h"   // Classes for file converters
#include "library.h"      // Classes for reading and writing libraries
#include "cmdline.h"      // Command line interpreter class

#endif // defined OBJCONV_STDAFX_H
