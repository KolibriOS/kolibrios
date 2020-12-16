/*
elf_repl.h - public header file for systems that lack it.
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

/* @(#) $Id: elf_repl.h,v 1.4 1998/06/04 17:14:01 michael Exp $ */

/*
 * NEVER INCLUDE THIS FILE DIRECTLY - USE <libelf.h> INSTEAD!
 */

#ifndef _ELF_REPL_H
#define _ELF_REPL_H

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Scalar data types
 */
typedef __libelf_u32_t		Elf32_Addr;
typedef __libelf_u16_t		Elf32_Half;
typedef __libelf_u32_t		Elf32_Off;
typedef __libelf_i32_t		Elf32_Sword;
typedef __libelf_u32_t		Elf32_Word;

#define ELF32_FSZ_ADDR		4
#define ELF32_FSZ_HALF		2
#define ELF32_FSZ_OFF		4
#define ELF32_FSZ_SWORD		4
#define ELF32_FSZ_WORD		4

#if __LIBELF64
typedef __libelf_u64_t		Elf64_Addr;
typedef __libelf_u16_t		Elf64_Half;
typedef __libelf_u64_t		Elf64_Off;
typedef __libelf_i32_t		Elf64_Sword;
typedef __libelf_i64_t		Elf64_Sxword;
typedef __libelf_u32_t		Elf64_Word;
typedef __libelf_u64_t		Elf64_Xword;

#define ELF64_FSZ_ADDR		8
#define ELF64_FSZ_HALF		2
#define ELF64_FSZ_OFF		8
#define ELF64_FSZ_SWORD		4
#define ELF64_FSZ_SXWORD	8
#define ELF64_FSZ_WORD		4
#define ELF64_FSZ_XWORD		8
#endif /* __LIBELF64 */

/*
 * ELF header
 */
#define EI_NIDENT	16

typedef struct {
    unsigned char	e_ident[EI_NIDENT];
    Elf32_Half		e_type;
    Elf32_Half		e_machine;
    Elf32_Word		e_version;
    Elf32_Addr		e_entry;
    Elf32_Off		e_phoff;
    Elf32_Off		e_shoff;
    Elf32_Word		e_flags;
    Elf32_Half		e_ehsize;
    Elf32_Half		e_phentsize;
    Elf32_Half		e_phnum;
    Elf32_Half		e_shentsize;
    Elf32_Half		e_shnum;
    Elf32_Half		e_shstrndx;
} Elf32_Ehdr;

#if __LIBELF64
typedef struct {
    unsigned char	e_ident[EI_NIDENT];
    Elf64_Half		e_type;
    Elf64_Half		e_machine;
    Elf64_Word		e_version;
    Elf64_Addr		e_entry;
    Elf64_Off		e_phoff;
    Elf64_Off		e_shoff;
    Elf64_Word		e_flags;
    Elf64_Half		e_ehsize;
    Elf64_Half		e_phentsize;
    Elf64_Half		e_phnum;
    Elf64_Half		e_shentsize;
    Elf64_Half		e_shnum;
    Elf64_Half		e_shstrndx;
} Elf64_Ehdr;
#endif /* __LIBELF64 */

/*
 * e-ident
 */
#define EI_MAG0		0
#define EI_MAG1		1
#define EI_MAG2		2
#define EI_MAG3		3
#define EI_CLASS	4
#define EI_DATA		5
#define EI_VERSION	6
#define EI_PAD		7

#define ELFMAG0		0x7f
#define ELFMAG1		'E'
#define ELFMAG2		'L'
#define ELFMAG3		'F'
#define ELFMAG		"\177ELF"
#define SELFMAG		4

#define ELFCLASSNONE	0
#define ELFCLASS32	1
#define ELFCLASS64	2
#define ELFCLASSNUM	3

#define ELFDATANONE	0
#define ELFDATA2LSB	1
#define ELFDATA2MSB	2
#define ELFDATANUM	3

/*
 * e_type
 */
#define ET_NONE		0
#define ET_REL		1
#define ET_EXEC		2
#define ET_DYN		3
#define ET_CORE		4
#define ET_NUM		5
#define ET_LOPROC	0xff00
#define ET_HIPROC	0xffff

/*
 * e_machine
 */
#define EM_NONE		0
#define EM_M32		1		/* AT&T WE 32100 */
#define EM_SPARC	2		/* SPARC */
#define EM_386		3		/* Intel i386 */
#define EM_68K		4		/* Motorola 68000 */
#define EM_88K		5		/* Motorola 88000 */
#define EM_486		6		/* Intel i486 (do not use this one) */
#define EM_860		7		/* Intel i860 */
#define EM_MIPS		8		/* MIPS R3000 */
#define EM_NUM		9

/*
 * e_ident[EI_VERSION], e_version
 */
#define EV_NONE		0
#define EV_CURRENT	1
#define EV_NUM		2

/*
 * Section header
 */
typedef struct {
    Elf32_Word		sh_name;
    Elf32_Word		sh_type;
    Elf32_Word		sh_flags;
    Elf32_Addr		sh_addr;
    Elf32_Off		sh_offset;
    Elf32_Word		sh_size;
    Elf32_Word		sh_link;
    Elf32_Word		sh_info;
    Elf32_Word		sh_addralign;
    Elf32_Word		sh_entsize;
} Elf32_Shdr;

#if __LIBELF64
typedef struct {
    Elf64_Word		sh_name;
    Elf64_Word		sh_type;
    Elf64_Xword		sh_flags;
    Elf64_Addr		sh_addr;
    Elf64_Off		sh_offset;
    Elf64_Xword		sh_size;
    Elf64_Word		sh_link;
    Elf64_Word		sh_info;
    Elf64_Xword		sh_addralign;
    Elf64_Xword		sh_entsize;
} Elf64_Shdr;
#endif /* __LIBELF64 */

/*
 * Special section indices
 */
#define SHN_UNDEF	0
#define SHN_LORESERVE	0xff00
#define SHN_LOPROC	0xff00
#define SHN_HIPROC	0xff1f
#define SHN_ABS		0xfff1
#define SHN_COMMON	0xfff2
#define SHN_HIRESERVE	0xffff

/*
 * sh_type
 */
#define SHT_NULL	0
#define SHT_PROGBITS	1
#define SHT_SYMTAB	2
#define SHT_STRTAB	3
#define SHT_RELA	4
#define SHT_HASH	5
#define SHT_DYNAMIC	6
#define SHT_NOTE	7
#define SHT_NOBITS	8
#define SHT_REL		9
#define SHT_SHLIB	10
#define SHT_DYNSYM	11
#define SHT_NUM		12
#define SHT_LOPROC	0x70000000
#define SHT_HIPROC	0x7fffffff
#define SHT_LOUSER	0x80000000
#define SHT_HIUSER	0xffffffff

/*
 * sh_flags
 */
#define SHF_WRITE	0x1
#define SHF_ALLOC	0x2
#define SHF_EXECINSTR	0x4
#define SHF_MASKPROC	0xf0000000

/*
 * Symbol table
 */
typedef struct {
    Elf32_Word		st_name;
    Elf32_Addr		st_value;
    Elf32_Word		st_size;
    unsigned char	st_info;
    unsigned char	st_other;
    Elf32_Half		st_shndx;
} Elf32_Sym;

#if __LIBELF64
typedef struct {
    Elf64_Word		st_name;
    unsigned char	st_info;
    unsigned char	st_other;
    Elf64_Half		st_shndx;
    Elf64_Addr		st_value;
    Elf64_Xword		st_size;
} Elf64_Sym;
#endif /* __LIBELF64 */

/*
 * Special symbol indices
 */
#define STN_UNDEF	0

/*
 * Macros for manipulating st_info
 */
#define ELF32_ST_BIND(i)	((i)>>4)
#define ELF32_ST_TYPE(i)	((i)&0xf)
#define ELF32_ST_INFO(b,t)	(((b)<<4)+((t)&0xf))

/*
 * Symbol binding
 */
#define STB_LOCAL	0
#define STB_GLOBAL	1
#define STB_WEAK	2
#define STB_NUM		3
#define STB_LOPROC	13
#define STB_HIPROC	15

/*
 * Symbol types
 */
#define STT_NOTYPE	0
#define STT_OBJECT	1
#define STT_FUNC	2
#define STT_SECTION	3
#define STT_FILE	4
#define STT_NUM		5
#define STT_LOPROC	13
#define STT_HIPROC	15

/*
 * Relocation
 */
typedef struct {
    Elf32_Addr		r_offset;
    Elf32_Word		r_info;
} Elf32_Rel;

typedef struct {
    Elf32_Addr		r_offset;
    Elf32_Word		r_info;
    Elf32_Sword		r_addend;
} Elf32_Rela;

#if __LIBELF64
typedef struct {
    Elf64_Addr		r_offset;
    Elf64_Xword		r_info;
} Elf64_Rel;

typedef struct {
    Elf64_Addr		r_offset;
    Elf64_Xword		r_info;
    Elf64_Sxword	r_addend;
} Elf64_Rela;
#endif /* __LIBELF64 */

/*
 * Macros for manipulating r_info
 */
#define ELF32_R_SYM(i)		((i)>>8)
#define ELF32_R_TYPE(i)		((unsigned char)(i))
#define ELF32_R_INFO(s,t)	(((s)<<8)+(unsigned char)(t))

/*
 * Note entry header
 */
typedef struct {
    Elf32_Word		n_namesz;	/* name size */
    Elf32_Word		n_descsz;	/* descriptor size */
    Elf32_Word		n_type;		/* descriptor type */
} Elf32_Nhdr;

#if __LIBELF64 && 0	/* I don't know if this is correct */
typedef struct {
    Elf64_Word		n_namesz;	/* name size */
    Elf64_Word		n_descsz;	/* descriptor size */
    Elf64_Word		n_type;		/* descriptor type */
} Elf64_Nhdr;
#endif /* __LIBELF64 */

/*
 * Well-known descriptor types for ET_CORE files
 */
#define NT_PRSTATUS	1
#define NT_PRFPREG	2
#define NT_PRPSINFO	3

/*
 * Program header
 */
typedef struct {
    Elf32_Word		p_type;
    Elf32_Off		p_offset;
    Elf32_Addr		p_vaddr;
    Elf32_Addr		p_paddr;
    Elf32_Word		p_filesz;
    Elf32_Word		p_memsz;
    Elf32_Word		p_flags;
    Elf32_Word		p_align;
} Elf32_Phdr;

#if __LIBELF64
typedef struct {
    Elf64_Word		p_type;
    Elf64_Word		p_flags;
    Elf64_Off		p_offset;
    Elf64_Addr		p_vaddr;
    Elf64_Addr		p_paddr;
    Elf64_Xword		p_filesz;
    Elf64_Xword		p_memsz;
    Elf64_Xword		p_align;
} Elf64_Phdr;
#endif /* __LIBELF64 */

/*
 * p_type
 */
#define PT_NULL		0
#define PT_LOAD		1
#define PT_DYNAMIC	2
#define PT_INTERP	3
#define PT_NOTE		4
#define PT_SHLIB	5
#define PT_PHDR		6
#define PT_NUM		7
#define PT_LOPROC	0x70000000
#define PT_HIPROC	0x7fffffff

/*
 * p_flags
 */
#define PF_R		0x4
#define PF_W		0x2
#define PF_X		0x1
#define PF_MASKPROC	0xf0000000

/*
 * Dynamic structure
 */
typedef struct {
    Elf32_Sword		d_tag;
    union {
	Elf32_Word	d_val;
	Elf32_Addr	d_ptr;
    } d_un;
} Elf32_Dyn;

#if __LIBELF64
typedef struct {
    Elf64_Sxword	d_tag;
    union {
	Elf64_Xword	d_val;
	Elf64_Addr	d_ptr;
    } d_un;
} Elf64_Dyn;
#endif /* __LIBELF64 */

/*
 * Dynamic array tags
 */
#define DT_NULL		0
#define DT_NEEDED	1
#define DT_PLTRELSZ	2
#define DT_PLTGOT	3
#define DT_HASH		4
#define DT_STRTAB	5
#define DT_SYMTAB	6
#define DT_RELA		7
#define DT_RELASZ	8
#define DT_RELAENT	9
#define DT_STRSZ	10
#define DT_SYMENT	11
#define DT_INIT		12
#define DT_FINI		13
#define DT_SONAME	14
#define DT_RPATH	15
#define DT_SYMBOLIC	16
#define DT_REL		17
#define DT_RELSZ	18
#define DT_RELENT	19
#define DT_PLTREL	20
#define DT_DEBUG	21
#define DT_TEXTREL	22
#define DT_JMPREL	23
#define DT_NUM		24
#define DT_LOPROC	0x70000000
#define DT_HIPROC	0x7fffffff

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif /* _ELF_REPL_H */
