/* lib/sys_elf.h.  Generated automatically by configure.  */
/*
sys_elf.h.in - configure template for private "switch" file.
Copyright (C) 1998 Michael Riepe <michael@stud.uni-hannover.de>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/* @(#) $Id: sys_elf.h.in,v 1.3 1998/06/04 15:26:48 michael Exp $ */

/*
 * DO NOT USE THIS IN APPLICATIONS - #include <libelf.h> INSTEAD!
 */

/* Define to `<elf.h>' or `<sys/elf.h>' if one of them is present */
#define __LIBELF_HEADER_ELF_H <elf.h>

/* Define if you want 64-bit support (and your system supports it) */
#define __LIBELF64 1

/* Define if you want 64-bit support, and are running IRIX */
/* #undef __LIBELF64_IRIX */

/* Define if you want 64-bit support, and are running Linux */
/* #undef __LIBELF64_LINUX */

/* Define to a 64-bit signed integer type if one exists */
#define __libelf_i64_t long long

/* Define to a 64-bit unsigned integer type if one exists */
#define __libelf_u64_t unsigned long long

/* Define to a 32-bit signed integer type if one exists */
#define __libelf_i32_t long

/* Define to a 32-bit unsigned integer type if one exists */
#define __libelf_u32_t unsigned long

/* Define to a 16-bit signed integer type if one exists */
#define __libelf_i16_t short

/* Define to a 16-bit unsigned integer type if one exists */
#define __libelf_u16_t unsigned short

/*
 * Ok, now get the correct instance of elf.h...
 */
#ifdef __LIBELF_HEADER_ELF_H
# include __LIBELF_HEADER_ELF_H
#else /* __LIBELF_HEADER_ELF_H */
# if __LIBELF_INTERNAL__
#  include <elf_repl.h>
# else /* __LIBELF_INTERNAL__ */
#  include <libelf/elf_repl.h>
# endif /* __LIBELF_INTERNAL__ */
#endif /* __LIBELF_HEADER_ELF_H */

/*
 * Workaround for broken <elf.h> on Linux...
 */
#if __LIBELF64 && __LIBELF64_LINUX
typedef __libelf_u64_t	Elf64_Addr;
typedef __libelf_u16_t	Elf64_Half;
typedef __libelf_u64_t	Elf64_Off;
typedef __libelf_i32_t	Elf64_Sword;
typedef __libelf_i64_t	Elf64_Sxword;
typedef __libelf_u32_t	Elf64_Word;
typedef __libelf_u64_t	Elf64_Xword;
#endif /* __LIBELF64 && __LIBELF64_LINUX */
