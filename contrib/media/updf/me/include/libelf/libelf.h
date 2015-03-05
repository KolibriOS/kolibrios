/*
libelf.h - public header file for libelf.
Copyright (C) 1995 - 1998 Michael Riepe <michael@stud.uni-hannover.de>

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

/* @(#) $Id: libelf.h,v 1.5 1998/06/01 19:47:18 michael Exp $ */

#ifndef _LIBELF_H
#define _LIBELF_H

#include <sys/types.h>

#if __LIBELF_INTERNAL__
#include <sys_elf.h>
#else /* __LIBELF_INTERNAL__ */
#include <libelf/sys_elf.h>
#endif /* __LIBELF_INTERNAL__ */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef __P
# if __STDC__ || defined(__cplusplus)
#  define __P(args) args
# else /* __STDC__ || defined(__cplusplus) */
#  define __P(args) ()
# endif /* __STDC__ || defined(__cplusplus) */
#endif /* __P */

/*
 * Commands
 */
typedef enum {
    ELF_C_NULL = 0,	/* must be first, 0 */
    ELF_C_READ,
    ELF_C_WRITE,
    ELF_C_CLR,
    ELF_C_SET,
    ELF_C_FDDONE,
    ELF_C_FDREAD,
    ELF_C_RDWR,
    ELF_C_NUM		/* must be last */
} Elf_Cmd;

/*
 * Flags
 */
#define	ELF_F_DIRTY	0x1
#define	ELF_F_LAYOUT	0x4

/*
 * File types
 */
typedef enum {
    ELF_K_NONE = 0,	/* must be first, 0 */
    ELF_K_AR,
    ELF_K_COFF,
    ELF_K_ELF,
    ELF_K_NUM		/* must be last */
} Elf_Kind;

/*
 * Data types
 */
typedef enum {
    ELF_T_BYTE = 0,	/* must be first, 0 */
    ELF_T_ADDR,
    ELF_T_DYN,
    ELF_T_EHDR,
    ELF_T_HALF,
    ELF_T_OFF,
    ELF_T_PHDR,
    ELF_T_RELA,
    ELF_T_REL,
    ELF_T_SHDR,
    ELF_T_SWORD,
    ELF_T_SYM,
    ELF_T_WORD,
    /*
     * New stuff for 64-bit.
     *
     * Most implementations add ELF_T_SXWORD after ELF_T_SWORD
     * which breaks binary compatibility with earlier versions.
     * If this causes problems for you, contact me.
     */
    ELF_T_SXWORD,
    ELF_T_XWORD,
    ELF_T_NUM		/* must be last */
} Elf_Type;

/*
 * Elf descriptor
 */
typedef struct Elf	Elf;

/*
 * Section descriptor
 */
typedef struct Elf_Scn	Elf_Scn;

/*
 * Archive member header
 */
typedef struct {
    char*		ar_name;
    time_t		ar_date;
    long		ar_uid;
    long 		ar_gid;
    unsigned long	ar_mode;
    off_t		ar_size;
    char*		ar_rawname;
} Elf_Arhdr;

/*
 * Archive symbol table
 */
typedef struct {
    char*		as_name;
    size_t		as_off;
    unsigned long	as_hash;
} Elf_Arsym;

/*
 * Data descriptor
 */
typedef struct {
    void*		d_buf;
    Elf_Type		d_type;
    size_t		d_size;
    off_t		d_off;
    size_t		d_align;
    unsigned		d_version;
} Elf_Data;

/*
 * Function declarations
 */
extern Elf *elf_begin __P((int __fd, Elf_Cmd __cmd, Elf *__ref));
extern Elf *elf_memory __P((char *__image, size_t __size));
extern int elf_cntl __P((Elf *__elf, Elf_Cmd __cmd));
extern int elf_end __P((Elf *__elf));
extern const char *elf_errmsg __P((int __err));
extern int elf_errno __P((void));
extern void elf_fill __P((int __fill));
extern unsigned elf_flagdata __P((Elf_Data *__data, Elf_Cmd __cmd,
	unsigned __flags));
extern unsigned elf_flagehdr __P((Elf *__elf, Elf_Cmd __cmd,
	unsigned __flags));
extern unsigned elf_flagelf __P((Elf *__elf, Elf_Cmd __cmd,
	unsigned __flags));
extern unsigned elf_flagphdr __P((Elf *__elf, Elf_Cmd __cmd,
	unsigned __flags));
extern unsigned elf_flagscn __P((Elf_Scn *__scn, Elf_Cmd __cmd,
	unsigned __flags));
extern unsigned elf_flagshdr __P((Elf_Scn *__scn, Elf_Cmd __cmd,
	unsigned __flags));
extern size_t elf32_fsize __P((Elf_Type __type, size_t __count,
	unsigned __ver));
extern Elf_Arhdr *elf_getarhdr __P((Elf *__elf));
extern Elf_Arsym *elf_getarsym __P((Elf *__elf, size_t *__ptr));
extern off_t elf_getbase __P((Elf *__elf));
extern Elf_Data *elf_getdata __P((Elf_Scn *__scn, Elf_Data *__data));
extern Elf32_Ehdr *elf32_getehdr __P((Elf *__elf));
extern char *elf_getident __P((Elf *__elf, size_t *__ptr));
extern Elf32_Phdr *elf32_getphdr __P((Elf *__elf));
extern Elf_Scn *elf_getscn __P((Elf *__elf, size_t __index));
extern Elf32_Shdr *elf32_getshdr __P((Elf_Scn *__scn));
extern unsigned long elf_hash __P((const char *__name));
extern Elf_Kind elf_kind __P((Elf *__elf));
extern size_t elf_ndxscn __P((Elf_Scn *__scn));
extern Elf_Data *elf_newdata __P((Elf_Scn *__scn));
extern Elf32_Ehdr *elf32_newehdr __P((Elf *__elf));
extern Elf32_Phdr *elf32_newphdr __P((Elf *__elf, size_t __count));
extern Elf_Scn *elf_newscn __P((Elf *__elf));
extern Elf_Cmd elf_next __P((Elf *__elf));
extern Elf_Scn *elf_nextscn __P((Elf *__elf, Elf_Scn *__scn));
extern size_t elf_rand __P((Elf *__elf, size_t __offset));
extern Elf_Data *elf_rawdata __P((Elf_Scn *__scn, Elf_Data *__data));
extern char *elf_rawfile __P((Elf *__elf, size_t *__ptr));
extern char *elf_strptr __P((Elf *__elf, size_t __section, size_t __offset));
extern off_t elf_update __P((Elf *__elf, Elf_Cmd __cmd));
extern unsigned elf_version __P((unsigned __ver));
extern Elf_Data *elf32_xlatetof __P((Elf_Data *__dst, const Elf_Data *__src,
	unsigned __encode));
extern Elf_Data *elf32_xlatetom __P((Elf_Data *__dst, const Elf_Data *__src,
	unsigned __encode));

#if __LIBELF64
/*
 * 64-bit ELF functions
 * Not available on all platforms
 */
extern Elf64_Ehdr *elf64_getehdr __P((Elf *__elf));
extern Elf64_Ehdr *elf64_newehdr __P((Elf *__elf));
extern Elf64_Phdr *elf64_getphdr __P((Elf *__elf));
extern Elf64_Phdr *elf64_newphdr __P((Elf *__elf, size_t __count));
extern Elf64_Shdr *elf64_getshdr __P((Elf_Scn *__scn));
extern size_t elf64_fsize __P((Elf_Type __type, size_t __count,
	unsigned __ver));
extern Elf_Data *elf64_xlatetof __P((Elf_Data *__dst, const Elf_Data *__src,
	unsigned __encode));
extern Elf_Data *elf64_xlatetom __P((Elf_Data *__dst, const Elf_Data *__src,
	unsigned __encode));
#endif /* __LIBELF64 */

/*
 * More function declarations
 * These functions are NOT available
 * in the SYSV version of libelf!
 */
extern size_t elf_delscn __P((Elf *__elf, Elf_Scn *__scn));

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _LIBELF_H */
