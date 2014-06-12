/*
errors.h - exhaustive list of all error codes and messages for libelf.
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

/* @(#) $Id: errors.h,v 1.4 1998/06/08 14:50:30 michael Exp $ */

/* dummy for xgettext */
#define _(str) str

__err__(ERROR_OK,		_("no error"))
__err__(ERROR_UNKNOWN,		_("unknown error"))
__err__(ERROR_INTERNAL,		_("Internal error: unknown reason"))
__err__(ERROR_UNIMPLEMENTED,	_("Internal error: not implemented"))
__err__(ERROR_WRONLY,		_("Request error: cntl(ELF_C_FDREAD) on write-only file"))
__err__(ERROR_INVALID_CMD,	_("Request error: invalid ELF_C_* argument"))
__err__(ERROR_FDDISABLED,	_("Request error: file descriptor disabled"))
__err__(ERROR_NOTARCHIVE,	_("Request error: not an archive"))
__err__(ERROR_BADOFF,		_("Request error: offset out of range"))
__err__(ERROR_UNKNOWN_VERSION,	_("Request error: unknown ELF version"))
__err__(ERROR_CMDMISMATCH,	_("Request error: ELF_C_* argument does not match"))
__err__(ERROR_MEMBERWRITE,	_("Request error: archive member begin() for writing"))
__err__(ERROR_FDMISMATCH,	_("Request error: archive/member file descriptor mismatch"))
__err__(ERROR_NOTELF,		_("Request error: not an ELF file"))
__err__(ERROR_CLASSMISMATCH,	_("Request error: class file/memory mismatch"))
__err__(ERROR_UNKNOWN_TYPE,	_("Request error: invalid ELF_T_* argument"))
__err__(ERROR_UNKNOWN_ENCODING,	_("Request error: unknown data encoding"))
__err__(ERROR_DST2SMALL,	_("Request error: destination buffer too small"))
__err__(ERROR_NULLBUF,		_("Request error: d_buf is NULL"))
__err__(ERROR_UNKNOWN_CLASS,	_("Request error: unknown ELF class"))
__err__(ERROR_ELFSCNMISMATCH,	_("Request error: section does not belong to file"))
__err__(ERROR_NOSUCHSCN,	_("Request error: no section at index"))
__err__(ERROR_NULLSCN,		_("Request error: can't manipulate null section"))
__err__(ERROR_SCNDATAMISMATCH,	_("Request error: data does not belong to section"))
__err__(ERROR_NOSTRTAB,		_("Request error: no string table"))
__err__(ERROR_BADSTROFF,	_("Request error: string table offset out of range"))
__err__(ERROR_RDONLY,		_("Request error: update(ELF_C_WRITE) on read-only file"))
__err__(ERROR_IO_SEEK,		_("I/O error: seek"))
__err__(ERROR_IO_2BIG,		_("I/O error: file too big for memory"))
__err__(ERROR_IO_READ,		_("I/O error: raw read"))
__err__(ERROR_IO_GETSIZE,	_("I/O error: get file size"))
__err__(ERROR_IO_WRITE,		_("I/O error: output write"))
__err__(ERROR_IO_TRUNC,		_("I/O error: can't truncate output file"))
__err__(ERROR_VERSION_UNSET,	_("Sequence error: must set ELF version first"))
__err__(ERROR_NOEHDR,		_("Sequence error: must create ELF header first"))
__err__(ERROR_OUTSIDE,		_("Format error: reference outside file"))
__err__(ERROR_TRUNC_ARHDR,	_("Format error: archive header truncated"))
__err__(ERROR_ARFMAG,		_("Format error: archive fmag"))
__err__(ERROR_ARHDR,		_("Format error: archive header"))
__err__(ERROR_TRUNC_MEMBER,	_("Format error: archive member truncated"))
__err__(ERROR_SIZE_ARSYMTAB,	_("Format error: archive symbol table size"))
__err__(ERROR_ARSTRTAB,		_("Format error: archive string table"))
__err__(ERROR_ARSPECIAL,	_("Format error: archive special name unknown"))
__err__(ERROR_TRUNC_EHDR,	_("Format error: ELF header truncated"))
__err__(ERROR_TRUNC_PHDR,	_("Format error: program header table truncated"))
__err__(ERROR_TRUNC_SHDR,	_("Format error: section header table truncated"))
__err__(ERROR_TRUNC_SCN,	_("Format error: data region truncated"))
__err__(ERROR_SCN2SMALL,	_("Format error: section size too small for data"))
__err__(ERROR_ALIGN_PHDR,	_("Format error: program header table alignment"))
__err__(ERROR_ALIGN_SHDR,	_("Format error: section header table alignment"))
__err__(ERROR_MEM_ELF,		_("Memory error: elf descriptor"))
__err__(ERROR_MEM_ARSYMTAB,	_("Memory error: archive symbol table"))
__err__(ERROR_MEM_ARHDR,	_("Memory error: archive member header"))
__err__(ERROR_MEM_EHDR,		_("Memory error: ELF header"))
__err__(ERROR_MEM_PHDR,		_("Memory error: program header table"))
__err__(ERROR_MEM_SHDR,		_("Memory error: section header table"))
__err__(ERROR_MEM_SCN,		_("Memory error: section descriptor"))
__err__(ERROR_MEM_SCNDATA,	_("Memory error: section data"))
__err__(ERROR_MEM_OUTBUF,	_("Memory error: output file space"))
