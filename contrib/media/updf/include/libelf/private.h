/*
private.h - private definitions for libelf.
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

/* @(#) $Id: private.h,v 1.10 1998/11/27 21:25:19 michael Exp $ */

#ifndef _PRIVATE_H
#define _PRIVATE_H

#define __LIBELF_INTERNAL__ 1

#if HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>

#if STDC_HEADERS
# include <stdlib.h>
# include <string.h>
#else /* STDC_HEADERS */
extern char *malloc(), *realloc();
extern void free(), bcopy();
extern int strcmp(), strncmp(), memcmp();
extern void *memcpy(), *memmove(), *memset();
#endif /* STDC_HEADERS */

#if HAVE_UNISTD_H
# include <unistd.h>
#else /* HAVE_UNISTD_H */
extern int read(), write();
extern off_t lseek();
#endif /* HAVE_UNISTD_H */

#if !HAVE_MEMCMP
# define memcmp	strncmp
#endif /* !HAVE_MEMCMP */
#if !HAVE_MEMCPY
# define memcpy(d,s,n)	bcopy(s,d,n)
#endif /* !HAVE_MEMCPY */
#if !HAVE_MEMMOVE
# define memmove(d,s,n)	bcopy(s,d,n)
#endif /* !HAVE_MEMMOVE */

#if !HAVE_MEMSET
# define memset _elf_memset
extern void *_elf_memset();
#endif /* !HAVE_MEMSET */

#if HAVE_STRUCT_NLIST_DECLARATION
# define nlist __override_nlist_declaration
#endif /* HAVE_STRUCT_NLIST_DECLARATION */

#if NEED_LINK_H
# include <link.h>
#endif /* NEED_LINK_H */

#include <libelf.h>

#if HAVE_STRUCT_NLIST_DECLARATION
# undef nlist
#endif /* HAVE_STRUCT_NLIST_DECLARATION */

typedef struct Scn_Data Scn_Data;

/*
 * ELF descriptor
 */
struct Elf {
    /* common */
    size_t	e_size;			/* file/member size */
    size_t	e_dsize;		/* size of memory image */
    Elf_Kind	e_kind;			/* kind of file */
    char*	e_data;			/* file/member data */
    char*	e_rawdata;		/* file/member raw data */
    size_t	e_idlen;		/* identifier size */
    int		e_fd;			/* file descriptor */
    unsigned	e_count;		/* activation count */
    /* archive members (still common) */
    Elf*	e_parent;		/* NULL if not an archive member */
    size_t	e_next;			/* 0 if not an archive member */
    size_t	e_base;			/* 0 if not an archive member */
    Elf*	e_link;			/* next archive member or NULL */
    Elf_Arhdr*	e_arhdr;		/* archive member header or NULL */
    /* archives */
    size_t	e_off;			/* current member offset (for elf_begin) */
    Elf*	e_members;		/* linked list of active archive members */
    char*	e_symtab;		/* archive symbol table */
    size_t	e_symlen;		/* length of archive symbol table */
    char*	e_strtab;		/* archive string table */
    size_t	e_strlen;		/* length of archive string table */
    /* ELF files */
    unsigned	e_class;		/* ELF class */
    unsigned	e_encoding;		/* ELF data encoding */
    unsigned	e_version;		/* ELF version */
    char*	e_ehdr;			/* ELF header */
    char*	e_phdr;			/* ELF program header table */
    size_t	e_phnum;		/* size of program header table */
    Elf_Scn*	e_scn_1;		/* first section */
    Elf_Scn*	e_scn_n;		/* last section */
    unsigned	e_elf_flags;		/* elf flags (ELF_F_*) */
    unsigned	e_ehdr_flags;		/* ehdr flags (ELF_F_*) */
    unsigned	e_phdr_flags;		/* phdr flags (ELF_F_*) */
    /* misc flags */
    unsigned	e_readable : 1;		/* file is readable */
    unsigned	e_writable : 1;		/* file is writable */
    unsigned	e_disabled : 1;		/* e_fd has been disabled */
    unsigned	e_cooked : 1;		/* e_data was modified */
    unsigned	e_free_syms : 1;	/* e_symtab is malloc'ed */
    unsigned	e_free_ehdr : 1;	/* e_ehdr is malloc'ed */
    unsigned	e_free_phdr : 1;	/* e_phdr is malloc'ed */
    unsigned	e_unmap_data : 1;	/* e_data is mmap'ed */
    unsigned	e_memory : 1;		/* created by elf_memory() */
    /* magic number for debugging */
    long	e_magic;
};

#define ELF_MAGIC	0x012b649e

#define INIT_ELF	{\
    /* e_size */	0,\
    /* e_dsize */	0,\
    /* e_kind */	ELF_K_NONE,\
    /* e_data */	NULL,\
    /* e_rawdata */	NULL,\
    /* e_idlen */	0,\
    /* e_fd */		-1,\
    /* e_count */	1,\
    /* e_parent */	NULL,\
    /* e_next */	0,\
    /* e_base */	0,\
    /* e_link */	NULL,\
    /* e_arhdr */	NULL,\
    /* e_off */		0,\
    /* e_members */	NULL,\
    /* e_symtab */	NULL,\
    /* e_symlen */	0,\
    /* e_strtab */	NULL,\
    /* e_strlen */	0,\
    /* e_class */	ELFCLASSNONE,\
    /* e_encoding */	ELFDATANONE,\
    /* e_version */	EV_NONE,\
    /* e_ehdr */	NULL,\
    /* e_phdr */	NULL,\
    /* e_phnum */	0,\
    /* e_scn_1 */	NULL,\
    /* e_scn_n */	NULL,\
    /* e_elf_flags */	0,\
    /* e_ehdr_flags */	0,\
    /* e_phdr_flags */	0,\
    /* e_readable */	0,\
    /* e_writable */	0,\
    /* e_disabled */	0,\
    /* e_cooked */	0,\
    /* e_free_syms */	0,\
    /* e_free_ehdr */	0,\
    /* e_free_phdr */	0,\
    /* e_unmap_data */	0,\
    /* e_memory */	0,\
    /* e_magic */	ELF_MAGIC\
}

/*
 * Section descriptor
 */
struct Elf_Scn {
    Elf_Scn*	s_link;			/* pointer to next Elf_Scn */
    Elf*	s_elf;			/* pointer to elf descriptor */
    size_t	s_index;		/* number of this section */
    unsigned	s_scn_flags;		/* section flags (ELF_F_*) */
    unsigned	s_shdr_flags;		/* shdr flags (ELF_F_*) */
    Scn_Data*	s_data_1;		/* first data buffer */
    Scn_Data*	s_data_n;		/* last data buffer */
    Scn_Data*	s_rawdata;		/* raw data buffer */
    /* data copied from shdr */
    unsigned	s_type;			/* section type */
    size_t	s_offset;		/* section offset */
    size_t	s_size;			/* section size */
    /* misc flags */
    unsigned	s_freeme : 1;		/* this Elf_Scn was malloc'ed */
    /* section header */
    union {
#if __LIBELF64
	Elf64_Shdr	u_shdr64;
#endif /* __LIBELF64 */
	Elf32_Shdr	u_shdr32;
    }		s_uhdr;
    /* magic number for debugging */
    long	s_magic;
};
#define s_shdr32	s_uhdr.u_shdr32
#define s_shdr64	s_uhdr.u_shdr64

#define SCN_MAGIC	0x012c747d

#define INIT_SCN	{\
    /* s_link */	NULL,\
    /* s_elf */		NULL,\
    /* s_index */	0,\
    /* s_scn_flags */	0,\
    /* s_shdr_flags */	0,\
    /* s_data_1 */	NULL,\
    /* s_data_n */	NULL,\
    /* s_rawdata */	NULL,\
    /* s_type */	SHT_NULL,\
    /* s_offset */	0,\
    /* s_size */	0,\
    /* s_freeme */	0,\
    /* s_uhdr */	{{0,}},\
    /* s_magic */	SCN_MAGIC\
}

/*
 * Data descriptor
 */
struct Scn_Data {
    Elf_Data	sd_data;		/* must be first! */
    Scn_Data*	sd_link;		/* pointer to next Scn_Data */
    char*	sd_memdata;		/* memory image of section */
    unsigned	sd_data_flags;		/* data flags (ELF_F_*) */
    /* misc flags */
    unsigned	sd_freeme : 1;		/* this Scn_Data was malloc'ed */
    unsigned	sd_free_data : 1;	/* sd_memdata is malloc'ed */
    /* magic number for debugging */
    long	sd_magic;
};

#define DATA_MAGIC	0x01072639

#define INIT_DATA	{\
    {\
    /* d_buf */		NULL,\
    /* d_type */	ELF_T_BYTE,\
    /* d_size */	0,\
    /* d_off */		0,\
    /* d_align */	0,\
    /* d_version */	EV_NONE\
    },\
    /* sd_link */	NULL,\
    /* sd_memdata */	NULL,\
    /* sd_data_flags */	0,\
    /* sd_freeme */	0,\
    /* sd_free_data */	0,\
    /* sd_magic */	DATA_MAGIC\
}

/*
 * Private status variables
 */
extern unsigned _elf_version;
extern int _elf_errno;
extern int _elf_fill;

/*
 * Private functions
 */
extern void *_elf_read __P((Elf*, void*, size_t, size_t));
extern void *_elf_mmap __P((Elf*));
extern int _elf_cook __P((Elf*));
extern char *_elf_getehdr __P((Elf*, unsigned));
extern Elf_Data *_elf_xlatetom(const Elf*, Elf_Data*, const Elf_Data*);

/*
 * Private data
 */
extern const Elf_Scn _elf_scn_init;
extern const Scn_Data _elf_data_init;
extern const Elf_Type _elf_scn_types[SHT_NUM];
extern const size_t _elf_fmsize[2][EV_CURRENT - EV_NONE][ELF_T_NUM][2];

/*
 * Access macros for _elf_fmsize[]
 */
#define _fmsize(c,v,t,w)	\
	(_elf_fmsize[(c)-ELFCLASS32][(v)-EV_NONE-1][(t)-ELF_T_BYTE][(w)])
#define _fsize(c,v,t)		_fmsize((c),(v),(t),1)
#define _msize(c,v,t)		_fmsize((c),(v),(t),0)

/*
 * Various checks
 */
#define valid_class(c)		((c) >= ELFCLASS32 && (c) <= ELFCLASS64)
#define valid_encoding(e)	((e) >= ELFDATA2LSB && (e) <= ELFDATA2MSB)
#define valid_version(v)	((v) > EV_NONE && (v) <= EV_CURRENT)
#define valid_type(t)		((t) >= ELF_T_BYTE && (t) < ELF_T_NUM)
#define valid_scntype(s)	((s) >= SHT_NULL && (s) < SHT_NUM)

/*
 * Error codes
 */
enum {
#define __err__(a,b)	a,
#include <errors.h>		/* include constants from errors.h */
#undef __err__
ERROR_NUM
};

#define seterr(err)	(_elf_errno = (err))

/*
 * Sizes of data types (external representation)
 * These definitions should be in <elf.h>, but...
 */
#ifndef ELF32_FSZ_ADDR
# define ELF32_FSZ_ADDR		4
# define ELF32_FSZ_HALF		2
# define ELF32_FSZ_OFF		4
# define ELF32_FSZ_SWORD	4
# define ELF32_FSZ_WORD		4
#endif /* ELF32_FSZ_ADDR */
#ifndef ELF64_FSZ_ADDR
# define ELF64_FSZ_ADDR		8
# define ELF64_FSZ_HALF		2
# define ELF64_FSZ_OFF		8
# define ELF64_FSZ_SWORD	4
# define ELF64_FSZ_SXWORD	8
# define ELF64_FSZ_WORD		4
# define ELF64_FSZ_XWORD	8
#endif /* ELF64_FSZ_ADDR */

/*
 * Alignment
 */
#define _ELF32_ALIGN_PHDR	4
#define _ELF32_ALIGN_SHDR	4
#define _ELF64_ALIGN_PHDR	8
#define _ELF64_ALIGN_SHDR	8

/*
 * Debugging
 */
#if ENABLE_DEBUG
# include <stdio.h>
# if __STDC__
#  define elf_assert(x)	((void)((x)||__elf_assert(__FILE__,__LINE__,#x)))
# else /* __STDC__ */
#  define elf_assert(x)	((void)((x)||__elf_assert(__FILE__,__LINE__,"x")))
# endif /* __STDC__ */
# define __elf_assert(f,l,x)	(fprintf(stderr,\
	"%s:%u: libelf assertion failure: %s\n",(f),(l),(x)),abort(),0)
#else /* ENABLE_DEBUG */
# define elf_assert(x)	((void)0)
#endif /* ENABLE_DEBUG */

#endif /* _PRIVATE_H */
